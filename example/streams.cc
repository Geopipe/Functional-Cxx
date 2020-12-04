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

#include <functional-cxx/stream.hpp>

#include <algorithm>
#include <iostream>
#include <sstream>

#include <boost/function_output_iterator.hpp>

using namespace com::geopipe::functional;

class FibStreamF {
	size_t a_;
	size_t b_;
	FibStreamF() : a_(0), b_(1) {}
	FibStreamF(size_t a, size_t b) : a_(a), b_(b) {}
public:
	
	std::shared_ptr<Stream<size_t>> operator()() const {
		return Stream<size_t>::Cell(a_, FibStreamF(b_, a_ + b_));
	}
	
	static std::shared_ptr<Stream<size_t>> first() {
		return FibStreamF()();
	}
};

class ROT13StreamF {
	std::istream & inp_;
public:
	using CellT = Stream<char>;
	using StreamT = std::shared_ptr<CellT>;
	ROT13StreamF(std::istream & inp) : inp_(inp) {}
	
	StreamT operator()() const {
		if(inp_.eof()){
			return CellT::Nil();
		} else {
			char i;
			inp_ >> std::noskipws >> i;
			char o = (i >= 'A' && i <= 'Z') ? ((((i - 'A') + 13) % 26) + 'A') : ((i >= 'a' && i <= 'z') ? ((((i - 'a') + 13) % 26) + 'a') : i);
			return CellT::Cell(o, ROT13StreamF(inp_));
		}
	}
};

int main(int argc, const char *argv[]) {
	auto std_out_it = boost::make_function_output_iterator([](const size_t& i){
		std::cout << i << std::endl;
	});
	std::copy_n(FibStreamF::first()->begin(), 94, std_out_it);
	
	std::copy_n(FibStreamF::first()->tail()->map([](size_t n){return n - 1;})->begin(), 5, std_out_it);
	
	std::istringstream rot13("FooBar\nSbbOne\n");
	// I'm not actually sure if this is a good idea,
	// Because we only get constant memory usage
	// while iterating the stream if we don't retain begin
	// and I'm not sure what the semantics of range-for are
	// But I thought I'd document an example either way
	for(char c : ROT13StreamF(rot13)()) {
		std::cout << c;
	}
	return 0;
}
