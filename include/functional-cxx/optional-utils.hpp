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
             * of a dynamic_cast from A to R. Otherwise, boost::none.
             * n.b. uses pointer form of dynamic cast internally,
             * so performance is balanced between success and failure
             * cases.
             ******************************************************/
            template<typename R, typename A>
            boost::optional<R&> dynamic_optional_cast(A& a) {
                return make_optional(dynamic_cast<R*>(&a));
            }
        }
    }
}