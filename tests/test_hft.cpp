/** 
    HFTrie - Data Structure for indexing binary codes 
    Copyright (C) 2022  David G. Starkweather

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**/

#include <iostream>
#include <cassert>
#include "hft/hft.hpp"


using namespace std;
using namespace hft;


void test_ht(){

	hf_t x = { 1, 0xf0f0f0f0f0f0f0f0ULL };

	uint64_t y = 0xe1e1e1e1e1e1e1e1ULL;

	cout << "x = " << hex << x.code << endl;
	cout << "y = " << hex << y << endl;
	
	int d = x.hdistance(y);
	cout << "distance(x, y) = " << d << endl;
	assert(d == 16);
}

void test_mask(){
	uint64_t correct_mask = create_mask(0);

	const int n_levels = NDIMS/CHUNKSIZE;
	for (int i=0;i < n_levels;i++){
		uint64_t mask = create_mask(i);
		cout << "mask-" << dec << i << " " << hex << mask << " " << correct_mask << endl;;
		assert(mask == correct_mask);
		correct_mask >>= CHUNKSIZE;
	}

	uint64_t code = 0x0123456789ABCDEFULL;
	
	for (int i=0;i < n_levels;i++){
		uint64_t idx = extract_index(code, i);
		cout << "level = " << dec << i << " idx = " << dec << idx << endl;
		assert((int)idx == i);
	}
}


int main(int argc, char **argv){

	test_ht();

	test_mask();
	

	return 0;
}
