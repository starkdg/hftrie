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

#include "hft/hft.hpp"

using namespace hft;

/**
 *  hf_t impl basic data struct for items
 *
 **/
unsigned long hft::hf_t::n_ops = 0;

hft::hf_t::hf_t(const hf_t &other){
	id = other.id;
	code = other.code;
}

hf_t& hft::hf_t::operator=(const hf_t &other){
	id = other.id;
	code = other.code;
	return *this;
}

int hft::hf_t::hdistance(const uint64_t c)const{
	hft::hf_t::n_ops++;
	return __builtin_popcountll(code^c);
}

hft::hf_search_t::hf_search_t(const hf_search_t &other){
	node = other.node;
	lvl = other.lvl;
	r = other.r;
}

hft::hf_search_t& hf_search_t::operator=(const hf_search_t &other){
	node = other.node;
	lvl = other.lvl;
	r = other.r;
	return *this;
}

uint64_t hft::create_mask(const int level){
	uint64_t mask = 1ULL;
	mask <<= CHUNKSIZE;
	mask -= 1;
	mask <<= NDIMS - CHUNKSIZE;
	mask >>= CHUNKSIZE*level;
	return mask;
}

uint64_t hft::extract_index(const uint64_t code, const int level){
	uint64_t mask = create_mask(level);
	uint64_t result = code & mask;
	result >>= NDIMS - CHUNKSIZE - CHUNKSIZE*level;
	return result;
}



