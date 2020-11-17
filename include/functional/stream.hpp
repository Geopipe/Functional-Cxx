#pragma once
/************************************************************************************
 *
 * Author: Thomas Dickerson
 * Copyright: 2019 - 2020, Geopipe, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ************************************************************************************/

#include <boost/variant.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <memory>
#include <functional>
#include <type_traits>
#include <utility>

#include <functional/support/memory-hacks.hpp>
#include <functional/support/unique-function.hpp>

namespace com {
	namespace geopipe {
		namespace functional{
			using namespace detail;
			template<class E>
			class Stream : public std::enable_shared_from_this<Stream<E>> {
				template<class> friend class Stream;
				template<class InStream, class Transform> friend class MapF;
				using StreamT = std::shared_ptr<Stream<E>>;
				using F = detail::UniqueFunction<StreamT()>;
				using std::enable_shared_from_this<Stream<E>>::shared_from_this;
				static_assert(std::is_convertible_v<std::invoke_result_t<F>, StreamT>, "F must have a return type of std::shared_ptr<Stream<E>>");
				E head_;
				mutable boost::variant<StreamT, F> tail_;
				
				template<typename A1, typename A2>
				Stream(A1 && e, A2 && t)
				: head_(std::forward<A1>(e)), tail_(std::forward<A2>(t)) {}
				
				template<typename A1, typename A2>
				static StreamT makeShared(A1 && a1, A2 && a2) {
					// Hax: https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const
					struct EnableMakeShared : Stream<E> {
						EnableMakeShared(A1 && a1, A2 && a2) : Stream<E>(std::forward<A1>(a1), std::forward<A2>(a2)) {}
					};
					
					return std::make_shared<EnableMakeShared>(std::forward<A1>(a1), std::forward<A2>(a2));
				}
			public:
				
				class StreamIterator : public boost::iterator_facade<StreamIterator, E, boost::forward_traversal_tag, const E &> {
					friend class Stream<E>;
					StreamT location_;
					StreamIterator(const StreamT &init) : location_(init) {}
				public:
					void increment() {
						location_ = location_->tail();
					}
					
					const E & dereference() const {
						return location_->head();
					}
					
					bool equal(const StreamIterator & other) const {
						return location_ == other.location_;
					}
				};
				
				const E& head() const {
					return head_;
				}
				
				const StreamT& tail() const {
					if(tail_.which()) {
						// This is not thread-safe!
						// but a simple read write lock would do the trick
						detail::emplace(tail_, boost::get<F>(tail_)());
					}
					return boost::get<StreamT>(tail_);
				}
				
				static StreamT Nil() {
					return nullptr;
				}
				
				template<typename A1, typename A2>
				static std::shared_ptr<Stream<E>> Cell(A1 && e, A2 && t) {
					return makeShared(std::forward<A1>(e), std::forward<A2>(t));
				}
				
				StreamIterator begin() {
					return StreamIterator(shared_from_this());
				}
				
				static StreamIterator end() {
					return StreamIterator(nullptr);
				}
				
				template<class Transform>
				using MapCellT = Stream<std::invoke_result_t<Transform,E>>;
				template<class Transform>
				using MapStreamT = typename MapCellT<Transform>::StreamT;
				
				template<class Transform>
				MapStreamT<Transform> map(Transform && transform) {
					class MapF {
						std::function<StreamT()> src_;
						Transform transform_;
						
					public:
						
						MapF(const std::function<StreamT()>& src, Transform&& transform)
						: src_(src), transform_(std::forward<Transform>(transform)) {}
						
						MapStreamT<Transform> operator()() {
							auto src = src_();
							if (src) {
								auto transformed_head = transform_(src->head());
								return MapCellT<Transform>::Cell(std::move(transformed_head), MapF([src](){ return src->tail(); }, std::move(transform_)));
							} else {
								return MapCellT<Transform>::Nil();
							}
						}
					};
					return MapF([sharedThis = shared_from_this()](){ return sharedThis; }, std::forward<Transform>(transform))();
				}
			};
		}
	}
}

namespace std {
	template<class E>
	auto end(const std::shared_ptr<com::geopipe::functional::Stream<E>> &s) {
		return com::geopipe::functional::Stream<E>::end();
	}
	
	template<class E>
	auto begin(const std::shared_ptr<com::geopipe::functional::Stream<E>> &s) {
		return s ? s->begin() : end(s);
	}
	
	template<class E>
	auto begin(std::shared_ptr<com::geopipe::functional::Stream<E>> &&consume) {
		std::shared_ptr<com::geopipe::functional::Stream<E>> s(std::move(consume));
		return s ? s->begin() : end(s);
	}
}
