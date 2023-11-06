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

#include <algorithm>
#include <functional-cxx/flat-iterator.hpp>
#include <iostream>
#include <vector>

static const std::vector<std::vector<int>> foo {
    {},
    {},
    {0},
    {1, 2, 3},
    {},
    {},
    {},
    {4},
    {},
    {5},
    {}};

int main(int argc, const char* argv[]) {
	using namespace com::geopipe::functional;
	std::vector<int> bar;
	std::copy(makeFlatIterator(foo.begin(), foo.end()), makeFlatIterator(foo.end(), foo.end()), std::back_inserter(bar));
	for (int i : bar) {
		std::cout << i << std::endl;
	}
}