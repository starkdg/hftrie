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
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <random>
#include <vector>
#include <chrono>
#include <cassert>
#include <ratio>
#include "hft/hftrie.hpp"

using namespace std;
using namespace hft;

const int n_runs = 5;

const int n_entries = 100000;
const int n_iters = 10;
const int n_clusters = 10;
const int ClusterSize = 10;
const double Radius = 14;

static random_device m_rd;
static mt19937_64 m_gen(m_rd());
static uniform_int_distribution<uint64_t> m_distrib(0);
static uniform_int_distribution<int> r(1, Radius);
static uniform_int_distribution<int> bitindex(0, 63);


static long long m_id = 1;
static long long g_id = 100000000;

struct perfmetric {
	double avg_build_ops;
	double avg_build_time;
	double avg_query_ops;
	double avg_query_time;
	size_t avg_memory_used;
};

int generate_data(vector<hf_t> &entries, const int n){

	for (int i=0;i < n;i++){
		entries.push_back({ m_id++, m_distrib(m_gen) });
	}

	return entries.size();
}

int generate_cluster(vector<hf_t> &entries, const uint64_t center, int n){
		
	uint64_t mask = 0x01;
	entries.push_back({ g_id++, center });

	for (int i=0;i < n-1;i++){
		uint64_t val = center;
		int dist = r(m_gen);
		for (int j=0;j < dist;j++){
			val ^= (mask << bitindex(m_gen));
		}
		entries.push_back({ g_id++, val });
	}
	return n;
}

void do_run(int index, vector<struct perfmetric> &metrics){
	m_id = 1;
	g_id = 100000000;

	HFTrie trie;

	int sz;
	chrono::duration<double> total(0);
	hf_t::n_ops = 0;
	for (int i=0;i < n_iters;i++){
		vector<hf_t> entries;
		generate_data(entries, n_entries);
	
		auto s = chrono::steady_clock::now();
		for (auto &e : entries){
			trie.Insert(e);
		}
		auto e = chrono::steady_clock::now();
		total += (e - s);
		sz = trie.Size();

		assert(sz == n_entries*(i+1));
	}

	struct perfmetric m;
	
	m.avg_build_ops = 100.0*((double)hf_t::n_ops/(double)sz); 
	m.avg_build_time = total.count()/(double)sz;
	m.avg_memory_used = trie.MemoryUsage();
	
	cout << "(" << index << ") build tree: " << setw(10) << setprecision(6) << m.avg_build_ops << "% opers "
		 << setw(10) << setprecision(6) << m.avg_build_time << " secs ";

	uint64_t centers[n_clusters];
	for (int i=0;i < n_clusters;i++){
		centers[i] = m_distrib(m_gen);

		vector<hf_t> cluster;
		generate_cluster(cluster, centers[i], ClusterSize);
		assert(cluster.size() == ClusterSize);
		for (auto &e : cluster){
			trie.Insert(e);
		}
		
		sz = trie.Size();
		assert(sz == n_entries*n_iters + ClusterSize*(i+1));
	}

	hf_t::n_ops = 0;
	chrono::duration<double, milli> querytime(0);
	for (int i=0;i < n_clusters;i++){
		auto s = chrono::steady_clock::now();
		vector<hf_t> results = trie.RangeSearch(centers[i], Radius);
		auto e = chrono::steady_clock::now();
		querytime += (e - s);

		int nresults = (int)results.size();
		assert(nresults >= ClusterSize);
	}

	m.avg_query_ops = 100.0*((double)hf_t::n_ops/(double)n_clusters/(double)sz);
	m.avg_query_time = (double)querytime.count()/(double)n_clusters;

	cout << " query ops " << dec << setprecision(6) << m.avg_query_ops << "% opers   " 
		 << "query time: " << dec << setprecision(6) << m.avg_query_time << " millisecs" << endl;


	
	metrics.push_back(m);
	
	trie.Clear();
	sz = trie.Size();
	assert(sz == 0);
}



int main(int argc, char **argv){


	cout << "N = " << n_entries*n_iters << endl;
	cout << "across " << n_runs << " runs" << endl;
	cout << "no. clusters: " << n_clusters <<  " of size "
		 << ClusterSize << " with radius = " << Radius << endl;

	cout << "HWTree data structure with parameters: " << endl;
	cout << "leaf capacity: 10" << endl;
	cout << endl << endl;

	cout << "DataSet size: " << n_entries*n_iters << " random data points" << endl;
	cout << n_clusters << " clusters of " << ClusterSize << " with radius = " << Radius << endl;
	cout << "Stats Avgeraged over " << n_runs << " runs" << endl;

	
	vector<perfmetric> metrics;
	for (int i=0;i < n_runs;i++){
		do_run(i, metrics);
	}

	double avg_build_ops = 0;
	double avg_build_time = 0;
	double avg_query_ops = 0;
	double avg_query_time = 0;
	double avg_memory = 0;
	for (struct perfmetric &m : metrics){
		avg_build_ops += m.avg_build_ops/n_runs;
		avg_build_time += m.avg_build_time/n_runs;
		avg_query_ops += m.avg_query_ops/n_runs;
		avg_query_time += m.avg_query_time/n_runs;
		avg_memory += (double)m.avg_memory_used/(double)n_runs;
	}

	cout << "no. runs: " << metrics.size() << endl;
	
	cout << "avg build:  " << avg_build_ops << "% opers " << avg_build_time << " seconds" << endl;
	cout << "avg query:  " << avg_query_ops << "% opers " << avg_query_time << " milliseconds" << endl;
	cout << "Memory Usage: " << fixed << setprecision(2) << avg_memory/1000000.0 << "MB" << endl;
	
	cout << endl << "Done." << endl;


	
	return 0;
}


