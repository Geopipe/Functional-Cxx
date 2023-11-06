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

#include <boost/iterator/iterator_facade.hpp>
#include <boost/range/iterator_range.hpp>
#include <type_traits>

namespace com {
	namespace geopipe {
		/**************************************************
		 * Tools for functional programming
		 **************************************************/
		namespace functional {
			/******************************************************
             *  Boiler-plate flatten iterator implementation using 
             * [`boost::iterator_facade`](https://www.boost.org/doc/libs/1_73_0/libs/iterator/doc/iterator_facade.html)
             ******************************************************/
			template<class OuterIterator,
			         class InnerIterator = std::remove_cv_t<std::remove_reference_t<decltype(std::declval<OuterIterator>()->begin())>>,
			         class ReferenceType = decltype(*std::declval<InnerIterator>())>
			class FlatIterator : public boost::iterator_facade<FlatIterator<OuterIterator, InnerIterator, ReferenceType>, std::remove_cv_t<std::remove_reference_t<ReferenceType>>, boost::forward_traversal_tag, ReferenceType> {
				// We need to be able to check for outer end, so need the end!
				boost::iterator_range<OuterIterator> outerPos_;
				InnerIterator innerPos_;
				void skipEmpty() {
					if (!outerPos_.empty()) {
						while (innerPos_ == outerPos_.begin()->end()) {
							outerPos_.advance_begin(1);
							if (outerPos_.empty()) {
								innerPos_ = InnerIterator {};
								break;
							} else {
								innerPos_ = outerPos_.begin()->begin();
							}
						}
					}
				}

			  public:
				FlatIterator(const boost::iterator_range<OuterIterator>& range)
				    : outerPos_(range)
				    , innerPos_(range.empty() ? InnerIterator {} : (range.begin()->begin())) {
					skipEmpty();
				}
				FlatIterator(const OuterIterator& begin, const OuterIterator& end)
				    : FlatIterator(boost::iterator_range<OuterIterator>(begin, end)) {}

				void increment() {
					++innerPos_;
					skipEmpty();
				}

				ReferenceType dereference() const {
					return *innerPos_;
				}

				bool equal(const FlatIterator<OuterIterator>& other) const {
					return (outerPos_ == other.outerPos_) && (innerPos_ == other.innerPos_);
				}
			};

			/// Returns a flattening iterator on the range of ranges given by `begin` and `end`
			template<class OuterIterator>
			FlatIterator<OuterIterator> makeFlatIterator(OuterIterator begin, OuterIterator end) {
				return FlatIterator<OuterIterator>(begin, end);
			}

			/// Returns a flattening iterator on the range of ranges given by `c`
			template<class Container>
			auto makeFlatIterator(const Container& c) {
				return makeFlatIterator(c.begin(), c.end());
			}

			/// Returns a flattening iterator on the range of ranges given by `c`
			template<class Container>
			auto makeFlatIterator(Container& c) {
				return makeFlatIterator(c.begin(), c.end());
			}
		}    // namespace functional
	}        // namespace geopipe
}    // namespace com