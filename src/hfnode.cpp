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

#include "hft/hfnode.hpp"

using namespace hft;

/**
 *  HFInternal Impl.
 *
 **/
hft::HFNode::~HFNode(){}

hft::HFInternal::HFInternal(){
	for (int i=0;i < NODE_FANOUT;i++){
		m_nodes[i] = NULL;
	}
}

hft::HFInternal::~HFInternal(){}

bool hft::HFInternal::IsLeaf()const{
	return false;
}

size_t hft::HFInternal::Size()const{
	return 0;
}

size_t hft::HFInternal::nbytes()const{
	return sizeof(HFInternal); 
}

void hft::HFInternal::SetChildNode(HFNode *node, const int idx){
	m_nodes[idx] = node;
}

bool hft::HFInternal::HasChildNode(const uint64_t idx)const{
	return (m_nodes[idx] != NULL);
}

HFNode* hft::HFInternal::GetChildNode(const uint64_t idx){
	if (m_nodes[idx] == NULL){
		m_nodes[idx] = new HFLeaf();
	}
	return m_nodes[idx];
}

void hft::HFInternal::GetChildNodes(std::queue<HFNode*> &nodes)const{
	for (uint64_t i=0;i < NODE_FANOUT;i++){
		if (m_nodes[i] != NULL){
			nodes.push(m_nodes[i]);
		}
	}
}

void hft::HFInternal::SearchFast(const uint64_t target, uint64_t target_idx, const int level, const int radius,
							 std::queue<hf_search_t> &nodes){

    if (m_nodes[target_idx] != NULL){
		nodes.push({ m_nodes[target_idx], level+1, radius });
	}
	
	if (radius > 0){
		uint64_t mask = 0x0001ULL << (CHUNKSIZE-1);
		while (mask != 0){
			uint64_t idx = target_idx^mask;
			if (m_nodes[idx] != NULL){
				nodes.push({ m_nodes[idx], level+1, radius - 1 });
			}
			mask >>= 1;
		}
	}
}

void hft::HFInternal::Search(const uint64_t target, uint64_t target_idx, const int level, const int radius,
							 std::queue<hf_search_t> &nodes){

	for (uint64_t i=0;i < NODE_FANOUT;i++){
		if (m_nodes[i] != NULL){
			int d = radius - __builtin_popcountll(target_idx^i);
			if (d >= 0){
				nodes.push({ m_nodes[i], level+1, d});
			}
		}
	}
}
	
/**
 *  HFLeaf Impl
 *
 **/
hft::HFLeaf::HFLeaf(){}

hft::HFLeaf::~HFLeaf(){}

bool hft::HFLeaf::IsLeaf()const{
	return true;
}

size_t hft::HFLeaf::Size()const{
	return m_entries.size();
}

size_t hft::HFLeaf::nbytes()const{
	return sizeof(HFLeaf) + m_entries.capacity()*sizeof(hf_t);
}

void hft::HFLeaf::Add(const hf_t &item, const int level){
	m_entries.push_back(item);
}

const std::vector<hf_t>& hft::HFLeaf::GetEntries()const{
	return m_entries;
}

void hft::HFLeaf::Search(const uint64_t target, const uint64_t target_idx, const int level,
						 const int radius, std::vector<hf_t> &results){
	for (hf_t &e : m_entries){
		if (e.hdistance(target) <= radius){
			results.push_back(e);
		}
	}
}

void hft::HFLeaf::Delete(const hf_t &item, const int level){
	for (auto iter=m_entries.begin();iter != m_entries.end();){
		if (iter->id == item.id && iter->code == item.code){
			iter = m_entries.erase(iter);
		} else {
			iter++;
		}
	}
}

