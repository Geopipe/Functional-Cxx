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

#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace com {
	namespace geopipe {
		namespace functional {
			namespace detail {
				// "safely" construct a T in place
				// after destructing the previous T
				// that was occupying the same memory
				// Allows simulating operator=
				// even if not defined on T
				// (e.g. because it contains a const member
				// or a reference member)
				template<typename T, typename ...Arg>
				T& emplace(T& t, Arg&& ...arg) {
					t.~T();
					new (&t) T(std::forward<Arg>(arg)...);
					return t;
				}

				template<typename T>
				using AlignedFor = typename std::aligned_storage<sizeof(T), alignof(T)>::type;

				template<typename T>
				class AlignedMaybeUninitializedDeleter {
					// On the heap so that if this deleter gets moved
					// the bool will keep the same address and preserve
					// references which might be squirreled away in other
					// classes for use with emplaceMaybeUninitialized
					std::unique_ptr<bool> initialized_;
				public:
					AlignedMaybeUninitializedDeleter() : initialized_(std::make_unique<bool>(false)) {}
					AlignedMaybeUninitializedDeleter(const AlignedMaybeUninitializedDeleter&) = delete;
					AlignedMaybeUninitializedDeleter& operator=(const AlignedMaybeUninitializedDeleter&) = delete;
					AlignedMaybeUninitializedDeleter(AlignedMaybeUninitializedDeleter&& other) = default; 
					AlignedMaybeUninitializedDeleter& operator=(AlignedMaybeUninitializedDeleter&&) = default;

					void initialize() { initialized() = true; }
					void reset() { initialized_ = std::make_unique<bool>(false); }
					bool& initialized() {
						if(initialized_) {
							return *initialized_;
						} else {
							throw std::runtime_error("An AlignedMaybeUninitializedDeleter may be used at most once");
						}
					}

					void operator()(T *t) {
						// I *think* this is correct, but could use a language lawyer
						if(t) {
							if(initialized()) {
								t->~T();
							}
							auto storage = std::launder(reinterpret_cast<AlignedFor<T>*>(t));
							delete storage;
							initialized_ = nullptr;
						}
					}
				};

				template<typename T>
				using UniqueMaybePtr = std::unique_ptr<T, AlignedMaybeUninitializedDeleter<T> >;
				
				// Creates a std::unique_ptr with uninitialized storage that has the correct size and alignment
				// to safely hold a T at some later point in time.
				// Can be safely assigned through emplaceMaybeUninitialized
				// and if this done, the initialization state can be 
				// accessed with .get_deleter().initialized()
				template<typename T>
				UniqueMaybePtr<T> make_unique_uninitialized() {
					return UniqueMaybePtr<T>(std::launder(reinterpret_cast<T*>(new AlignedFor<T>)));
				}

				// Like emplace, but cooperates with UniqueMaybePtr
				// to avoid running a destructor on uninitialized memory
				template<typename T, typename ...Args>
				T& emplaceMaybeUninitialized(T* t, bool& initialized, Args&& ...args) {
					if(initialized) {
						t->~T();
					}
					new (t) T(std::forward<Args>(args)...);
					initialized = true;
					return *t;
				}

			}
		}
	}
}
