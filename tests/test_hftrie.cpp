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

#include <cstdlib>
#include <iostream>
#include <random>
#include <cassert>
#include "hft/hftrie.hpp"

using namespace std;
using namespace hft;

const int n_entries = 100;
const int n_clusters = 5;

const int ClusterSize = 10;
const int Radius = 10;

static long long m_id = 1;
static long long g_id = 100000;

static random_device m_rd;
static mt19937_64 m_gen(m_rd());
static uniform_int_distribution<uint64_t> m_distrib(0);
static uniform_int_distribution<int> m_radius(1,Radius);
static uniform_int_distribution<int> m_bitindex(0,63);


int generate_data(vector<hf_t> &entries, const int n){
	for (int i=0;i < n;i++){
		entries.push_back({ m_id++, m_distrib(m_gen) });
	}
	return 0;
}

int generate_cluster(vector<hf_t> &entries, const uint64_t center, const int N){
		
	uint64_t mask = 0x0001ULL;
	entries.push_back({ g_id++, center });

	for (int i=0;i < N-1;i++){
		uint64_t val = center;
		int dist = m_radius(m_gen);
		for (int j=0;j < dist;j++){
			val ^= (mask << m_bitindex(m_gen));
		}
		entries.push_back({ g_id++, val });
	}
	return N;
}

void test(){
	
	size_t sz;
	vector<hf_t> entries;
	generate_data(entries, n_entries);
	assert(entries.size() == n_entries);
	
	cout << "Insert " << n_entries << " entries" << endl;
	HFTrie trie;
	for (int i=0;i < n_entries;i++){
		trie.Insert(entries[i]);
	}

	sz = trie.Size();
	cout << "sz = " << sz << endl;
	assert(sz == n_entries);


	uint64_t centers[n_clusters];
	for (int i=0;i < n_clusters;i++){
		cout << "Add cluster of " << ClusterSize << " with radius of " << Radius << endl;

		centers[i] = m_distrib(m_gen);
		vector<hf_t> cluster;
		generate_cluster(cluster, centers[i], ClusterSize);

		for (hf_t &e : cluster){
			trie.Insert(e);
		}
		
		sz = trie.Size();
		assert((int)sz == n_entries + (i+1)*ClusterSize);
	}
	
	cout << "sz = " << sz << endl;
	assert(sz == n_entries + n_clusters*ClusterSize);

	trie.Print(cout);

	for (int i=0;i < n_clusters;i++){
		cout << "Search for " << hex << centers[i] << endl;
		vector<hf_t> results = trie.RangeSearch(centers[i], Radius);
		cout << "Found: " << dec << results.size() << endl;
		for (hf_t r : results){
			cout << "  " << dec << r.id << " " << hex << r.code << endl;
		}
		assert(results.size() > 0);
		g_id += ClusterSize;
	}

	long long delid = 100000;
	for (int i=0;i < n_clusters;i++){
		cout << "Delete id = " << dec << delid << " code = " << hex << centers[i] << endl;
		trie.Delete({delid, centers[i] });
		delid += ClusterSize;
	}

	sz = trie.Size();
	cout << "sz = " << dec << sz << endl;
	assert(sz == n_entries + n_clusters*(ClusterSize-1));

	trie.Clear();
	sz = trie.Size();
	cout << "sz = " << sz << endl;
	assert(sz == 0);
}

int main(int argc, char **argv){

	test();

	
	return 0;
}
