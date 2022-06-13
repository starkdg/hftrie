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

#ifndef _HF_H
#define _HF_H
#include <cstdint>


#define NDIMS 64
#define CHUNKSIZE 4
#define NODE_FANOUT 16
#define LC 10

namespace hft {

	struct hf_t {
		static unsigned long n_ops;
		long long id;
		uint64_t code;
		hf_t():id(0),code(0){};
		hf_t(const long long id, const uint64_t code):id(id),code(code){}
		hf_t(const hf_t &other);
		hf_t& operator=(const hf_t &other);
		int hdistance(const uint64_t c)const;
	};

	class HFNode;

	struct hf_search_t {
		const HFNode *node;
		int lvl;
		int r;
		hf_search_t(const HFNode *node,const int lvl, const int r):node(node),lvl(lvl),r(r){}
		hf_search_t(const hf_search_t &other);
		hf_search_t& operator=(const hf_search_t &other);
	};

	uint64_t create_mask(const int level);

	uint64_t extract_index(const uint64_t code, const int level);

}

#endif /* _HF_H */
