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

/**
 *  HFInternal Impl.
 *
 **/
HFNode::~HFNode(){}

HFInternal::HFInternal(){
	for (int i=0;i < NODE_FANOUT;i++){
		m_nodes[i] = NULL;
	}
}

HFInternal::~HFInternal(){}

bool HFInternal::IsLeaf()const{
	return false;
}

size_t HFInternal::Size()const{
	return 0;
}

size_t HFInternal::nbytes()const{
	return NODE_FANOUT*sizeof(hf_t);
}

void HFInternal::SetChildNode(HFNode *node, const int idx){
	m_nodes[idx] = node;
}

bool HFInternal::HasChildNode(const uint64_t idx)const{
	return (m_nodes[idx] != NULL);
}

HFNode* HFInternal::GetChildNode(const uint64_t idx){
	if (m_nodes[idx] == NULL){
		m_nodes[idx] = new HFLeaf();
	}
	return m_nodes[idx];
}

void HFInternal::GetChildNodes(queue<HFNode*> &nodes)const{
	for (uint64_t i=0;i < NODE_FANOUT;i++){
		if (m_nodes[i] != NULL){
			nodes.push(m_nodes[i]);
		}
	}
}

void HFInternal::Search(const uint64_t target, const int level, const int subradius, const int radius, queue<hf_search_t> &nodes){

	uint64_t idx = extract_index(target, level);


	for (uint64_t i=0;i < NODE_FANOUT;i++){
		if (m_nodes[i] != NULL){
			if (i == idx){
				nodes.push({m_nodes[i], level+1, radius});
			} else {
				int d = __builtin_popcountll(idx^i);
				d = (d <= radius) ? radius - d : 0;
				nodes.push({ m_nodes[i], level+1, d });
			}
		}
	}
}
	
/**
 *  HFLeaf Impl
 *
 **/
HFLeaf::HFLeaf(){}

HFLeaf::~HFLeaf(){}

bool HFLeaf::IsLeaf()const{
	return true;
}

size_t HFLeaf::Size()const{
	return m_entries.size();
}

size_t HFLeaf::nbytes()const{
	return m_entries.capacity()*sizeof(hf_t);
}

void HFLeaf::Add(const hf_t &item, const int level){
	m_entries.push_back(item);
}

const vector<hf_t>& HFLeaf::GetEntries()const{
	return m_entries;
}

void HFLeaf::Search(const uint64_t target, const int level, const int subradius, const int radius, vector<hf_t> &results){

	uint64_t idx = extract_index(target, level);
	for (hf_t &e : m_entries){
		uint64_t entry_idx = extract_index(e.code, level);
		int subd = __builtin_popcountll(idx^entry_idx);
		if (subd <= subradius){
			int d = e.hdistance(target);
			if (d <= radius){
				results.push_back(e);
			}
		}
	}
}

void HFLeaf::Delete(const hf_t &item, const int level){
	for (auto iter=m_entries.begin();iter != m_entries.end();){
		if (iter->id == item.id && iter->code == item.code){
			iter = m_entries.erase(iter);
		} else {
			iter++;
		}
	}
}

