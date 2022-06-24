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

#include "hft/hftrie.hpp"

using namespace std;
using namespace hft;

hft::HFTrie::HFTrie(){
	m_top = NULL;
}

hft::HFTrie::~HFTrie(){
	Clear();
}


void hft::HFTrie::Insert(const hf_t &item){
	if (m_top == NULL){
		m_top = new HFLeaf();
		((HFLeaf*)m_top)->Add(item, 0);
		return;
	}

	int level = 0;
	uint64_t idx;
	HFNode *prev = NULL, *node = m_top;
	while (!node->IsLeaf()){
		idx = extract_index(item.code, level);
		prev = node;
		node = ((HFInternal*)node)->GetChildNode(idx);
		level++;
	}

	HFLeaf *leaf = (HFLeaf*)node;
	leaf->Add(item, level);
	
	if (node->Size() > LC && level < NDIMS/CHUNKSIZE){

		HFInternal *internal = new HFInternal();
		if (level == 0){
			m_top = internal;
		} else {
			((HFInternal*)prev)->SetChildNode(internal, idx);
		}
		
		vector<hf_t> list = leaf->GetEntries();
		for (hf_t e : list){
			idx = extract_index(e.code, level);
			HFLeaf *nleaf = (HFLeaf*)internal->GetChildNode(idx);
			nleaf->Add(e, level);
		}

		delete leaf;
	}
}

void hft::HFTrie::Delete(const hf_t &item){
	if (m_top == NULL) return;

	int level = 0;
	uint64_t idx;
	HFNode *node = m_top;
	while (node != NULL && !node->IsLeaf()){
		idx = extract_index(item.code, level);
		if (((HFInternal*)node)->HasChildNode(idx)){
			node = ((HFInternal*)node)->GetChildNode(idx);
		} else {
			node = NULL;
		}
		level++;
	}

	if (node == NULL) return;
	((HFLeaf*)node)->Delete(item, level);
	
}

vector<hf_t> hft::HFTrie::RangeSearchFast(const uint64_t target, const int radius)const{
	vector<hf_t> results;

	queue<hf_search_t> nodes;

	if (m_top != NULL){
		nodes.push({ m_top, 0, radius });
	}

	int level = 0;
	while (!nodes.empty()){
		uint64_t target_idx = extract_index(target, level);
		queue<hf_search_t> next_nodes;

		while (!nodes.empty()){
			hf_search_t current = nodes.front();
			if (current.node->IsLeaf()){
				HFLeaf *leaf = (HFLeaf*)current.node;
				leaf->Search(target, target_idx, current.lvl, radius, results);
			} else {
				HFInternal *internal = (HFInternal*)current.node;
				internal->SearchFast(target, target_idx, current.lvl, current.r, next_nodes);
			}
			nodes.pop();
		}
		nodes = move(next_nodes);
		level++;
	}
	return results;
}

vector<hf_t> hft::HFTrie::RangeSearch(const uint64_t target, const int radius)const{
	vector<hf_t> results;

	queue<hf_search_t> nodes;

	if (m_top != NULL){
		nodes.push({ m_top, 0, radius });
	}

	int level = 0;
	while (!nodes.empty()){
		uint64_t target_idx = extract_index(target, level);
		queue<hf_search_t> next_nodes;

		while (!nodes.empty()){
			hf_search_t current = nodes.front();
			if (current.node->IsLeaf()){
				HFLeaf *leaf = (HFLeaf*)current.node;
				leaf->Search(target, target_idx, current.lvl, radius, results);
			} else {
				HFInternal *internal = (HFInternal*)current.node;
				internal->Search(target, target_idx, current.lvl, current.r, next_nodes);
			}
			nodes.pop();
		}
		nodes = move(next_nodes);
		level++;
	}
	return results;
}

size_t hft::HFTrie::Size()const{

	queue<HFNode*> nodes;
	if (m_top != NULL) nodes.push(m_top);

	size_t count = 0;
	while (!nodes.empty()){
		HFNode *current = nodes.front();
				
		count += current->Size();
		if (!current->IsLeaf())
			((HFInternal*)current)->GetChildNodes(nodes);

		nodes.pop();
	}

	return count;
}

void hft::HFTrie::Clear(){
	queue<HFNode*> nodes;
	if (m_top != NULL) nodes.push(m_top);

	while (!nodes.empty()){
		HFNode *current = nodes.front();
		if (!current->IsLeaf())
			((HFInternal*)current)->GetChildNodes(nodes);
		delete current;
		nodes.pop();
	}
	m_top = NULL;
}

size_t hft::HFTrie::MemoryUsage()const{
	queue<HFNode*> nodes;
	if (m_top != NULL) nodes.push(m_top);

	size_t nbytes = 0;
	while (!nodes.empty()){
		HFNode *current = nodes.front();
		nbytes += current->nbytes();
		if (!current->IsLeaf())
			((HFInternal*)current)->GetChildNodes(nodes);
		nodes.pop();
	}
	return nbytes + sizeof(HFTrie);
}


void hft::HFTrie::Print(ostream &ostrm)const{
	queue<HFNode*> current, next;

	ostrm << "------HF Trie-------" << endl;
	ostrm << "--------------------" << endl << endl;

	if (m_top != NULL) current.push(m_top);

	int level = 0;
	while (!current.empty()){
		while (!current.empty()){
			HFNode *node = current.front();
			if (node->IsLeaf()){
				ostrm << "  leaf(level=" << level << ") size = " << node->Size() << endl;

				vector<hf_t> entries = ((HFLeaf*)node)->GetEntries();

				ostrm << "ListEntries: " << endl;
				for (hf_t &e : entries){
					ostrm << "    " << dec << e.id << " " << hex << e.code << endl;
				}
				
			} else {
				ostrm << "  internal(level=" << level << ") " << endl;
				((HFInternal*)node)->GetChildNodes(next);
			}
			current.pop();
		}
		level++;
		current = move(next);
	}

	ostrm << endl << endl << "--------END---------" << endl;
}
