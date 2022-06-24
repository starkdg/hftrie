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



static random_device m_rd;
static mt19937_64 m_gen(m_rd());
static uniform_int_distribution<uint64_t> m_distrib(0);
static uniform_int_distribution<int> bitindex(0, 63);


static long long m_id = 1;
static long long g_id = 100000000;

struct perfmetric {
	double avg_build_ops;
	double avg_build_time;
	double avg_query_ops;
	double avg_query_time;
	double avg_memory_used;
	double avg_recall;
};

int generate_data(vector<hf_t> &entries, const int n){

	for (int i=0;i < n;i++){
		entries.push_back({ m_id++, m_distrib(m_gen) });
	}

	return entries.size();
}

int generate_cluster(vector<hf_t> &entries, const uint64_t center, const int radius, const int n){

	uniform_int_distribution<int> r(1, radius);
	
	uint64_t mask = 0x01;
	entries.push_back({ g_id++, center });

	for (int i=0;i < n-1;i++){
		uint64_t val = center;
		if (radius > 0){
			int dist = r(m_gen);
			for (int j=0;j < dist;j++){
				val ^= (mask << bitindex(m_gen));
			}
		}
		entries.push_back({ g_id++, val });
	}
	return n;
}

void do_run(const int index, const int n_runs, const int n_entries,
			const int n_clusters, const int cluster_size, const int radius,
			vector<struct perfmetric> &metrics){
	m_id = 1;
	g_id = 100000000;

	HFTrie trie;

	int sz;
	chrono::duration<double, nano> total(0);

	vector<hf_t> entries;
	generate_data(entries, n_entries);

	hf_t::n_ops = 0;
	auto s = chrono::steady_clock::now();
	for (auto &e : entries){
		trie.Insert(e);
	}
	auto e = chrono::steady_clock::now();
	total += (e - s);
	sz = trie.Size();
	assert(sz == n_entries);


	struct perfmetric m;
	m.avg_build_ops = 100.0*((double)hf_t::n_ops/(double)sz); 
	m.avg_build_time = (double)total.count()/(double)sz;
	m.avg_memory_used = trie.MemoryUsage();
	
	cout << "(" << dec << index << ") build tree: " << setw(10) << setprecision(6) << m.avg_build_ops << "% opers "
		 << setw(10) << setprecision(6) << m.avg_build_time << " nsecs ";

	uint64_t centers[n_clusters];
	for (int i=0;i < n_clusters;i++){
		centers[i] = m_distrib(m_gen);

		vector<hf_t> cluster;
		generate_cluster(cluster, centers[i], radius, cluster_size);
		assert((int)cluster.size() == cluster_size);
		for (auto &e : cluster){
			trie.Insert(e);
		}
		
		sz = trie.Size();
		assert(sz == n_entries + cluster_size*(i+1));
	}

	int total_returned = 0;
	
	hf_t::n_ops = 0;
	chrono::duration<double, milli> querytime(0);
	for (int i=0;i < n_clusters;i++){
		auto s = chrono::steady_clock::now();
		vector<hf_t> results = trie.RangeSearchFast(centers[i], radius);
		auto e = chrono::steady_clock::now();
		querytime += (e - s);

		int nresults = (int)results.size();
		total_returned += nresults;
		assert((int)nresults >=0);
	}

	m.avg_query_ops = 100.0*((double)hf_t::n_ops/(double)n_clusters/(double)sz);
	m.avg_query_time = (double)querytime.count()/(double)n_clusters;
	m.avg_recall = (double)total_returned/(double)n_clusters/(double)cluster_size;
	
	cout << " query ops " << dec << setprecision(6) << m.avg_query_ops << "% opers   " 
		 << "query time: " << dec <<setprecision(6) <<  m.avg_query_time << " millisecs" << endl;


	metrics.push_back(m);
	
	trie.Clear();
	sz = trie.Size();
	assert(sz == 0);
}

void do_experiment(const int n, const int n_runs, const int n_entries,
				   const int n_clusters, const int cluster_size, const int radius){

	cout << "---------------------------------------------------------" << endl;
	cout << "Experiment " << n << endl;
	cout << "-----------------------------------------------------------" << endl;
	cout << "DataSet size: " << n_entries << " random data points" << endl;
	cout << n_clusters << " clusters of " << cluster_size << " with radius = " << radius << endl;
	cout << "-----------------------------------------------------------" << endl;
	
	vector<perfmetric> metrics;
	for (int i=0;i < n_runs;i++){
		do_run(i, n_runs, n_entries, n_clusters, cluster_size, radius, metrics);
	}

	double avg_build_ops = 0;
	double avg_build_time = 0;
	double avg_query_ops = 0;
	double avg_query_time = 0;
	double avg_memory = 0;
	double avg_recall = 0;
	for (struct perfmetric &m : metrics){
		avg_build_ops += m.avg_build_ops/n_runs;
		avg_build_time += m.avg_build_time/n_runs;
		avg_query_ops += m.avg_query_ops/n_runs;
		avg_query_time += m.avg_query_time/n_runs;
		avg_memory += (double)m.avg_memory_used/(double)n_runs;
		avg_recall += m.avg_recall/(double)n_runs;
	}

	cout << "no. runs: " << metrics.size() << endl;
	cout << "avg build:  " << avg_build_ops << "% opers " << avg_build_time << " nsecs" << endl;
	cout << "avg query:  " << avg_query_ops << "% opers " << avg_query_time << " msecs" << endl;
	cout << "query recall: " << 100.0*avg_recall << "%" << endl; 
	cout << "Memory Usage: " << fixed << setprecision(2) << avg_memory/1000000.0 << "MB" << endl;
	cout << "------------------------------------------------------------" << endl << endl;
}	
	   

int main(int argc, char **argv){

	const int n_runs = 5;
	const int n_experiments = 12;
	const int n_entries[n_experiments] = { 100000, 200000, 400000, 800000,
										   1000000, 2000000, 4000000, 8000000,
										   16000000, 32000000, 64000000, 128000000};
	const int n_clusters = 10;
	const int cluster_size = 10;
	const double radius = 10;

	cout << "Test hftrie data structure" << endl;
	cout << "Chunk Size: " << CHUNKSIZE << endl;
	cout << "Leaf capacity: " << LC << endl;
	cout << "All stats avg'd  over " << n_runs << " runs" << endl;
	cout << endl << endl;

   
	cout << "        Experiments with varying dataset size, N = 100K to 32M " << endl;

	for (int i=0;i < n_experiments;i++){
		do_experiment(i+1, n_runs, n_entries[i], n_clusters, cluster_size, radius);
	}	

	cout << "        Experiments varying the query radius and under constant N" << endl;
	const int N = 4000000;
	const int n_rad = 7;
	const int rad[n_rad] = { 0, 2, 4, 6, 8, 10, 12 };
	
	for (int i=0;i < n_rad;i++){
		do_experiment(i+1, n_runs, N, n_clusters, cluster_size, rad[i]);
	}

	return 0;
}


