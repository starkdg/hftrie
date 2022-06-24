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

#ifndef _HFNODE_H
#define _HFNODE_H

#include <vector>
#include <queue>
#include <stack>
#include "hft/hft.hpp"

namespace hft {

	class HFNode {
	public:
		virtual ~HFNode();
		virtual bool IsLeaf()const = 0;
		virtual size_t Size()const = 0;
		virtual size_t nbytes()const = 0;
	};


	class HFInternal : public HFNode {
	private:
		HFNode* m_nodes[NODE_FANOUT];
	public:
		HFInternal();
		~HFInternal();
		bool IsLeaf()const;
		std::size_t Size()const;
		std::size_t nbytes()const;
		
		void SetChildNode(HFNode *node, const int idx);
		bool HasChildNode(const std::uint64_t idx)const;
		HFNode* GetChildNode(const std::uint64_t idx);
		void GetChildNodes(std::queue<HFNode*> &nodes)const;
		void SearchFast(const std::uint64_t target, const std::uint64_t target_idx,
					const int level, const int radius, std::queue<hf_search_t> &nodes);
		void Search(const std::uint64_t target, const std::uint64_t target_idx,
					const int level, const int radius, std::queue<hf_search_t> &nodes);
	};

	class HFLeaf : public HFNode {
	private:
		std::vector<hf_t> m_entries;
	public:
		HFLeaf();
		~HFLeaf();
		bool IsLeaf()const;
		std::size_t Size()const;
		std::size_t nbytes()const;
	
		void Add(const hf_t &item, const int level);
		const std::vector<hf_t>& GetEntries()const;
		void Search(const std::uint64_t target, const std::uint64_t target_idx,
					const int level, const int radius, std::vector<hf_t> &results);
		void Delete(const hf_t &item, const int level);
	};
}

#endif /* _HFNODE_H */
