/************************************************************************************
 *
 * Author: Thomas Dickerson
 * Copyright: 2023, Geopipe, Inc.
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

#include <functional-cxx/optional-utils.hpp>

#include <iostream>
#include <typeinfo>

using namespace com::geopipe::functional;

int main(int argc, const char *argv[]) {
    boost::optional<int> maybeA = boost::none;
    boost::optional<int> maybeB = 1;
    
    std::cout << typeid(each(maybeA)).name() << std::endl;
    for (auto a : each(maybeA)) {
        std::cout << a << std::endl;
    }

    std::cout << typeid(each(maybeB)).name() << std::endl;
    for (auto b : each(maybeB)) {
        std::cout << b << std::endl;
    }

    std::cout << typeid(each(boost::optional<int>{3})).name() << std::endl;
    for (auto c : each(boost::optional<int>{3})) {
        std::cout << c << std::endl;
    }

    return 0;
}