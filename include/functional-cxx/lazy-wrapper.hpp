#pragma once
/************************************************************************************
 * @file lazy-wrapper.hpp 
 * 
 * `@file` command necessary to generate docs for macros.
 * 
 * Author: Thomas Dickerson
 * Copyright: 2020-2021, Geopipe, Inc.
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

#include <stdexcept>
#include <type_traits>

namespace com {
	namespace geopipe {
		/**************************************************
		 * Tools for functional programming
		 **************************************************/
		namespace functional {
			/**************************************************
			 * Wraps a nullary functor (or "thunk") for 
			 * ergonomic lazy evaluation.
			 * 
			 * Recommended to use with the #LAZY_V(X) and 
			 * #_TODO(type) macros.
			 **************************************************/
			template<class F> struct lazy : F {
				using F::operator(); ///< Make `F::operator()` into `lazy<F>::operator()`.
				using type = std::invoke_result_t<F>; ///< The result type of forcing the thunk.
				/**************************************************
				 * The common use is as a forwarded temporary
				 * so we will only allow a cast if this is an rvalue
				 * to inhibit accidental repeated invocations
				 * if `F` is expensive or has side-effects.
				 * If you _really_ want to invoke repeatedly,
				 * you can always just call `lazy::operator()` directly!
				 **************************************************/
				operator type() && noexcept(std::is_nothrow_invocable_v<F>) { return this->operator()(); }
			};

			/// A deduction guide.
			template<class F> lazy(F) -> lazy<F>;
		}
	}
}

/*****************************************************
 * @def LAZY_V(X)
 * Defer evaluation of expression, `X`.
 * 
 * If the result of LAZY_V is forwarded, 
 * the expression X will only be evaluated when it is 
 * moved from to produce a value of the result type.
 * This is useful, for example, with `std::map<K,V>::try_emplace`,
 * when constructing V has side-effects and should only be 
 * done if `K` is not present.
 *****************************************************/
#define LAZY_V(X) com::geopipe::functional::lazy{[&](){ return (X); }}
/*****************************************************
 * @def _TODO(type) 
 * Placeholder expression of type `type`.
 * 
 * This is basically Scala's `???`, 
 * but with a type hint, since C++'s type-inference
 * capabilities are fairly limited. May be used as a
 * placeholder where expression of type `type` is expected, 
 * and the resulting code will type-check, but `throw`
 * at runtime. This is useful for doing API-centric 
 * program design, as you can rapidly mock up an interface
 * with the desired types and fill in the implementation 
 * as you go.
 *****************************************************/
#define _TODO(type) com::geopipe::functional::lazy{[]() -> type { throw std::runtime_error("not implemented");}}
// Need a blank line here or Doxygen won't parse the preceding macro definition.