#pragma once
/************************************************************************************
 *
 * Author: Thomas Dickerson
 * Copyright: 2021, Geopipe, Inc.
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

#include <boost/optional.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <functional>
#include <type_traits>

namespace com {
	namespace geopipe {
		/**************************************************
		 * Tools for functional programming
		 **************************************************/
		namespace functional {
			/**************************************************
			 * Internal implementation details
			 **************************************************/
			namespace detail {
				template<typename T>
				struct is_optional { constexpr static const bool value = false; };

				template<typename E>
				struct is_optional<boost::optional<E>> { constexpr static const bool value = true; };

				template<typename E> class OptionalRangeOwning;
				template<typename E> class OptionalRangeBorrowed;
				template<typename E>
				class OptionalIterator : public boost::iterator_facade<OptionalIterator<E>, E, boost::forward_traversal_tag, const E &> {
					std::reference_wrapper<const boost::optional<E>> pos_;

					static const boost::optional<E>& typedNone() {
						static boost::optional<E> typedNone_ = boost::none;
						return typedNone_;
					}

					friend class OptionalRangeOwning<E>;
					friend class OptionalRangeBorrowed<E>;
				  public:
					OptionalIterator(const boost::optional<E>& pos) : pos_(pos) {}

					void increment() {
						pos_ = typedNone();
					}

					const E & dereference() const {
						return *(pos_.get());
					}

					bool equal(const OptionalIterator<E> & other) const {
						return ((pos_.get() == boost::none) && (other.pos_.get() == boost::none)) || &(pos_.get()) == &(other.pos_.get());
					}
				};

				template<typename E>
				class OptionalRangeOwning {
					boost::optional<E> contents_;
				  public:
					OptionalRangeOwning(boost::optional<E>&& contents) : contents_(std::move(contents)) {}

					OptionalIterator<E> begin() const {
						return OptionalIterator<E>{contents_};
					}

					OptionalIterator<E> end() const {
						return OptionalIterator<E>{OptionalIterator<E>::typedNone()};
					}
				};

				template<typename E>
				class OptionalRangeBorrowed {
					std::reference_wrapper<const boost::optional<E>> contents_;
				  public:
					OptionalRangeBorrowed(const boost::optional<E>& contents) : contents_(contents) {}

					OptionalIterator<E> begin() const {
						return OptionalIterator<E>{contents_};
					}

					OptionalIterator<E> end() const {
						return OptionalIterator<E>{OptionalIterator<E>::typedNone()};
					}
				};
			}

			/// Promote the `boost::optional` to a range. The argument is moved into the return value.
			template<typename T>
			detail::OptionalRangeOwning<T> each(boost::optional<T> && t) {
				return detail::OptionalRangeOwning<T>{std::move(t)};
			}

			/// Promote the `boost::optional` to a range.
			/// The argument is not copied so the range will be invalidated if it goes out of scope.
			template<typename T>
			detail::OptionalRangeBorrowed<T> each(const boost::optional<T> & t) {
				return detail::OptionalRangeBorrowed<T>{t};
			}

			/// Apply `f` to the value stored in the `boost::optional`, `t`. No-op if `boost::none == t`.
			template<typename T, typename F, std::enable_if_t<detail::is_optional<std::decay_t<T>>::value, bool> = true>
			void for_each(T && t, F f) {
				if(boost::none != t) {
					f(*std::forward<T>(t));
				}
			}

			/// `boost::none` if `t == nullptr`, `*t` otherwise.
			template<typename T>
			boost::optional<T&> make_optional(T* t) {
				if(nullptr == t) {
					return boost::none;
				} else {
					return *t;
				}
			}

			/******************************************************
			 * If `a` can be safely cast to `R`, return the result
			 * of a dynamic_cast from `A` to `R`. Otherwise, `boost::none`.
			 * n.b. uses pointer form of dynamic cast internally,
			 * so performance is balanced between success and failure
			 * cases.
			 ******************************************************/
			template<typename R, typename A>
			boost::optional<R&> dynamic_optional_cast(A& a) {
				return make_optional(dynamic_cast<R*>(&a));
			}

			/******************************************************
			 * Like `boost::make_optional<V>` by way of
			 * `std::map::try_emplace`: `args` are not evaluated
			 * unless `cond`. Useful with, e.g., `#LAZY_V`.
			 ******************************************************/
			template<typename V, typename ...Args>
			boost::optional<V> make_optional(bool cond, Args&& ...args) {
				boost::optional<V> ret = boost::none;
				if (cond) {
					ret.emplace(std::forward<Args>(args)...);
				}
				return ret;
			}
		}
	}
}