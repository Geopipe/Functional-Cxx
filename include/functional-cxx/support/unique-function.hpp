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

#include <memory>
#include <functional>		// std::invoke
#include <utility>

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
				class UniqueFunction;
				
				/**************************************************
				 * Like `std::function<R(Args...)>`, but for 
				 * move-only functor types.
				 * 
				 * Like `std::function::operator()`, 
				 * `UniqueFunction::operator()` allows type erasure
				 * of the wrapped functor's implementation details,
				 * at the cost of a virtual dispatch.
				 **************************************************/
				template<typename R, typename ...Args>
				class UniqueFunction<R(Args...)> {
					struct FunctionHolderBase {
						virtual R operator()(Args ...args) = 0;
						virtual ~FunctionHolderBase() {}
					};
					template<typename F>
					class FunctionHolder final : public FunctionHolderBase {
						friend class UniqueFunction<R(Args...)>;
						F f_;
						template<typename FA>
						FunctionHolder(FA && f)
						: f_(std::forward<FA>(f)) {}

						FunctionHolder(FunctionHolder<F> && fh)
						: f_(std::move(fh.f_)) {}

						FunctionHolder<F>& operator=(FunctionHolder<F> && fh) {
							f_ = std::move(fh.f_);
						}

						R operator()(Args&& ...args) override {
							return std::invoke(f_, std::forward<Args>(args)...);
						}
					};

					std::unique_ptr<FunctionHolderBase> fh_;
				public:
					template<typename FA>
					UniqueFunction(FA && f)
					: fh_(new FunctionHolder<FA>(std::forward<FA>(f))) {}

					UniqueFunction(UniqueFunction<R(Args...)> && uf)
					: fh_(std::move(uf.fh_)) {}

					UniqueFunction<R(Args...)>& operator=(UniqueFunction<R(Args...)> && uf) {
						fh_ = std::move(uf.fh_);
					}

					R operator()(Args&& ...args) {
						return std::invoke(*fh_, std::forward<Args>(args)...);
					}

					explicit operator bool() const noexcept {
						return (bool)fh_;
					}
				};
			}
		}
	}
}
