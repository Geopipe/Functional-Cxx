#pragma once
/************************************************************************************
 *
 * Author: Thomas Dickerson
 * Copyright: 2019 - 2021, Geopipe, Inc.
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

#include <functional-cxx/support/memory-hacks.hpp>
#include <functional-cxx/support/unique-function.hpp>

namespace com {
	namespace geopipe {
		/**************************************************
		 * Tools for functional programming
		 **************************************************/
		namespace functional {
			using namespace detail;

			/**************************************************
			 * Implements a `Stream` or "lazy list".
			 * 
			 * <pre class="markdeep">
			 * Lazy data structures are useful for modeling generative or "corecursive"
			 * processes: where a recursive process successively decomposes a complex 
			 * problem until reaching a simple base case, a corecursive process starts
			 * with a simple base case and composes increasingly complex results on
			 * top of it.
			 * 
			 * Such processes can represent ongoing (potentially unbounded!) computation
			 * and so it is often impractical to represent the whole structure in
			 * memory simultaneously. Instead, we amortize our costs and compute
			 * only as much (or as little) of the data structure as is needed.
			 * 
			 * Commonly, we will see corecursive data-structures paired with
			 * recursive algorithms to consume them and process their contents.
			 * For example, computing the SSSP graph with Djikstra's Algorithm 
			 * may be interpreted as a corecursive process (essentially, directed
			 * flood-filling over the graph), and path-finding from a given node
			 * to the source using the SSSP graph is a recursive process (essentially
			 * traversing over a linked-list).
			 * 
			 * A classical linked list might be defined as:
			 * ```c++
			 * template<typename E>
			 * struct Cons {
			 *   E head;
			 *   shared_ptr<Cons> tail;
			 * };
			 * ```
			 * 
			 * Conceptually, a lazy list, or stream is instead defined as
			 * ```c++
			 * template<typename E>
			 * struct LazyCons {
			 *   E head;
			 *   function<shared_ptr<LazyCons>()> tail;
			 * };
			 * ```
			 * However, this is not very efficient, as `tail` would then
			 * need to be recomputed on every pass through the list!
			 * 
			 * Instead, we implement memoization, so that as we traverse the lazy list,
			 * and "force" its contents to be evaluated, it converts into a standard list
			 * and saves the results for later.
			 * </pre>
			 * 
			 * @warning Since a `Stream`'s length is typically unknown ahead of time,
			 * and can grow unboundedly, this means that memory consumption can also
			 * grow unboundedly, so it is wise to traverse them in a sliding-window fashion,
			 * keeping only as long of a slice as is needful.
			 * 
			 * If you follow this practice, then as the `std::shared_ptr` referring to the head
			 * of the list goes out of scope, that node will be destructed and its memory
			 * reclaimed. We provide special `begin` and `end` overloads with move-semantics
			 * so that you can use a `Stream` with the standard `<algorithm>` library
			 * while enjoying the benefits of incremental reclamation.
			 * 
			 * @warning Note that most C++ compilers do not implement tail-recursive destruction,
			 * so if you do choose to retain a `std::shared_ptr` to the head
			 * and perform mass-reclamation, you will most likely want a special 
			 * reclamation loop to iteratively consume the `Stream` or else risk
			 * a stack-overflow bug.
			 * 
			 **************************************************/
			template<class E>
			class Stream : public std::enable_shared_from_this<Stream<E>> {
				template<class> friend class Stream;
				template<class InStream, class Transform> friend class MapF;
				using StreamT = std::shared_ptr<Stream<E>>; ///< We consider a "true" stream to be a `std::shared_ptr<Stream>`
				using F = detail::UniqueFunction<StreamT()>; ///< A functor returning new nodes
				using std::enable_shared_from_this<Stream<E>>::shared_from_this;
				static_assert(std::is_convertible_v<std::invoke_result_t<F>, StreamT>, "F must have a return type of std::shared_ptr<Stream<E>>");
				E head_; ///< Storage for the head of the stream
				/**************************************************
				 * Combined storage for thunk or memoized tail,
				 * Marked `mutable` because we are modeling a
				 * persistent data structure and the memoization
				 * does not change the abstract state, so we want 
				 * to be able to perform it even though `Stream`
				 * only provides `const` access to its state.
				 *************************************************/
				mutable boost::variant<StreamT, F> tail_; 
				
				/// @pre This constructor should only be invoked from `Stream::makeShared` to obtain a `Stream::StreamT` directly.
				template<typename A1, typename A2>
				Stream(A1 && e, A2 && t)
				: head_(std::forward<A1>(e)), tail_(std::forward<A2>(t)) {}
				
				/******************************************************************
				 * Some sorcery to emulate [`std::make_shared`](https://en.cppreference.com/w/cpp/memory/shared_ptr/make_shared) even though our constructor is private.
				 * See this [StackOverflow thread](https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const)
				 ******************************************************************/
				template<typename A1, typename A2>
				static StreamT makeShared(A1 && a1, A2 && a2) {
					struct EnableMakeShared : Stream<E> {
						EnableMakeShared(A1 && a1, A2 && a2) : Stream<E>(std::forward<A1>(a1), std::forward<A2>(a2)) {}
					};
					
					return std::make_shared<EnableMakeShared>(std::forward<A1>(a1), std::forward<A2>(a2));
				}
			public:
				/******************************************************
				 *  Boiler-plate linked-list iterator implementation using 
				 * [`boost::iterator_facade`](https://www.boost.org/doc/libs/1_73_0/libs/iterator/doc/iterator_facade.html)
				 ******************************************************/
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
				
				/// `Stream`s should be treated as persistent datastructures, so only `const`-access to the `head_` is permitted.
				const E& head() const {
					return head_;
				}
				
				/*****************************************************************
				 * Force the `tail_` thunk, or access memoized result.
				 * 
				 * `Stream`s should be treated as persistent datastructures, 
				 * so only `const`-access to the `tail_` is permitted.
				 * 
				 * @warning This is still thread-unsafe, because `tail_` is `mutable`,
				 * so the internals of this method are still allowed to perform an
				 * assignment.
				 * If this becomes a problem in the future, a R/W lock will suffice,
				 * and is guaranteed to only ever be acquired by a single writer
				 * in its lifetime.
				 *****************************************************************/
				const StreamT& tail() const {
					if(tail_.which()) {
						// This is not thread-safe!
						// but a simple read write lock would do the trick
						detail::emplace(tail_, boost::get<F>(tail_)());
					}
					return boost::get<StreamT>(tail_);
				}
				
				/// An empty `Stream::StreamT`.
				static StreamT Nil() {
					return nullptr;
				}
				
				/*****************************************************************
				 * Create a new `Stream::StreamT` by prepending `e` to `t`.
				 * @arg e will be used to construct the new `Stream::head`.
				 * @arg t will be used to construct the new `Stream::tail`
				 * and may therefore be _either_ a thunk returning `Stream::StreamT`
				 * or an actual `Stream::StreamT` (or otherwise convertible to one of these two).
				 *****************************************************************/
				template<typename A1, typename A2>
				static std::shared_ptr<Stream<E>> Cell(A1 && e, A2 && t) {
					return makeShared(std::forward<A1>(e), std::forward<A2>(t));
				}
				
				/// Obtain an iterator to the beginning of the `Stream`.
				StreamIterator begin() {
					return StreamIterator(shared_from_this());
				}
				
				/*********************************************************************
				 * Obtain a past-the-end iterator which is useable for all `Stream`s.
				 * @warning `Stream`s are potentially unbounded, so if you don't trust the
				 * source of the `Stream`, be aware that using this as the end-iterator
				 * with many standard `<algorithm>` functions could result in an infinite loop.
				 *********************************************************************/
				static StreamIterator end() {
					return StreamIterator(nullptr);
				}
				
				template<class Transform>
				using MapCellT = Stream<std::invoke_result_t<Transform,E>>;
				template<class Transform>
				using MapStreamT = typename MapCellT<Transform>::StreamT;
				
				/*********************************************************************
				 * Obtain a new `Stream` by applying `transform` to every element in this
				 * `Stream`.
				 *********************************************************************/
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
