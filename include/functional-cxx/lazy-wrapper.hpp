#pragma once
/************************************************************************************
 *
 * Author: Thomas Dickerson
 * Copyright: 2020, Geopipe, Inc.
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
		namespace functional {
			template<class F> struct lazy : F {
				using F::operator();
				using type = std::invoke_result_t<F>;
				// The common use is as a forwarded temporary
				// so we will only allow a cast if this is an rvalue
				// to inhibit accidental repeated invocations
				// if F is expensive or has side-effects.
				// If you *really* want to invoke repeatedly,
				// you can always just call operator() directly!
				operator type() && noexcept(std::is_nothrow_invocable_v<F>) { return this->operator()(); }
			};

			template<class F> lazy(F) -> lazy<F>;
// If the result of LAZY_V is forwarded, the expression X will only be evaluated
// when it is moved from to produce a value of the result type.
// This is useful, for example, with the standard library's map<K,V>::try_emplace
// when constructing V has side-effects and should only be done if K is not present.
#define LAZY_V(X) com::geopipe::functional::lazy{[&](){ return (X); }}
// This is basically Scala's ???, but with a type hint
#define _TODO(type) com::geopipe::functional::lazy{[]() -> type { throw std::runtime_error("not implemented");}}
		}
	}
}
