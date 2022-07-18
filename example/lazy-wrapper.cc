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

#include <map>
#include <iostream>

#include <functional-cxx/lazy-wrapper.hpp>

int main(int argc, char* argv[]){
	std::map<int, int> example;
	for(int i = 0; i < 10; ++i) {
		std::cout << i << " ";
		example.try_emplace(i / 2,LAZY_V(((std::cout << "blip"),i)));
		std::cout << std::endl;
	}
	int beep = _TODO(int);
	std::cout << "beep value: " << beep << std::endl;
}
