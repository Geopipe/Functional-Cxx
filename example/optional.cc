/************************************************************************************
 *
 * Author: Kestrel Yarrow
 * Copyright: 2023 Geopipe, Inc.
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

#include <boost/optional.hpp>

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace com::geopipe::functional;

int main(int argc, const char *argv[]) {

	auto print_str = [](const std::string &s) { std::cout << s << std::endl; };
	auto print_str_pair = [](const std::pair<std::string, std::string> &s) { std::cout << s.first << " " << s.second << std::endl; };

	using opt_string = boost::optional<std::string>;

	// this will print "present"
	for_each(opt_string("present"), print_str);
	// this will not print anything
	for_each(opt_string(), print_str);
	// these will print "correct"
	for_each(first_of(opt_string("correct"), opt_string("incorrect")), print_str);
	for_each(first_of(opt_string(), opt_string("correct")), print_str);
	// this will not print anything
	for_each(first_of(opt_string(), opt_string()), print_str);

	// these will not print anything
	for_each(both_of(opt_string(), opt_string()), print_str_pair);
	for_each(both_of(opt_string("a"), opt_string()), print_str_pair);
	for_each(both_of(opt_string(), opt_string("b")), print_str_pair);
	// this will print "corr ect"
	for_each(both_of(opt_string("corr"), opt_string("ect")), print_str_pair);

	// and again with constants :-)

	// this will print "present"
	const auto present = opt_string("present");
	const auto missing = opt_string();
	const auto correct = opt_string("correct");
	const auto incorrect = opt_string("incorrect");
	const auto corr = opt_string("corr");
	const auto ect = opt_string("ect");
	for_each(present, print_str);
	// this will not print anything
	for_each(missing, print_str);
	// these will print "correct"
	for_each(first_of(correct, incorrect), print_str);
	for_each(first_of(correct, incorrect), print_str);
	// this will not print anything
	for_each(first_of(missing, missing), print_str);

	// these will not print anything
	for_each(both_of(missing, missing), print_str_pair);
	for_each(both_of(incorrect, missing), print_str_pair);
	for_each(both_of(missing, incorrect), print_str_pair);
	// this will print "corr ect"
	for_each(both_of(corr, ect), print_str_pair);

	return 0;
}
