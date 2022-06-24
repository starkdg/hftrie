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

#ifndef _HFTRIE_H
#define _HFTRIE_H
#include <ostream>
#include <vector>
#include <queue>
#include "hft/hfnode.hpp"

namespace hft {

	class HFTrie {
	private:
		HFNode *m_top;

	public:
		HFTrie();

		~HFTrie();

		void Insert(const hf_t &item);

		void Delete(const hf_t &item);
	
		std::vector<hf_t> RangeSearchFast(const uint64_t target, const int radius)const;

		std::vector<hf_t> RangeSearch(const uint64_t target, const int radius)const;

		size_t Size()const;

		void Clear();
	
		size_t MemoryUsage()const;

		void Print(std::ostream &ostrm)const;
	
	};
}


#endif /* _HFTRIE_H */
