#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <random>
#include <chrono>
#include <cassert>

using namespace std;

const int n_runs = 5;

const int n_entries = 256000000;
const int n_clusters = 10;

const int ClusterSize = 10;
const int Radius = 10;

static long long m_id = 1;
static long long g_id = 100000;

static random_device m_rd;
static mt19937_64 m_gen(m_rd());
static uniform_int_distribution<uint64_t> m_distrib(0);
static uniform_int_distribution<int> m_radius(1,Radius);
static uniform_int_distribution<int> m_bitindex(0,63);


struct entry_t{
	static long long n_ops;
	long long id;
	uint64_t code;
	entry_t(const long long id, const uint64_t code):id(id),code(code){}
	entry_t(const entry_t &other){
		id = other.id;
		code = other.code;
	}
	entry_t& operator=(const entry_t &other){
		id = other.id;
		code = other.code;
		return *this;
	}
	int distance(const uint64_t other)const{
		entry_t::n_ops++;
		return __builtin_popcountll(code^other);
	}
};

struct perfmetric {
	double avg_query_ops;
	double avg_query_time;
	double mem;
};

long long entry_t::n_ops = 0;

int generate_data(vector<entry_t> &entries, const int n){
	for (int i=0;i < n;i++){
		entries.push_back({ m_id++, m_distrib(m_gen) });
	}
	return 0;
}

int generate_cluster(vector<entry_t> &entries, const uint64_t center, const int N){
		
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

vector<entry_t> search(const uint64_t target, const int radius, const vector<entry_t> &entries){
	vector<entry_t> results;

	for (const entry_t &e : entries){
		if (e.distance(target) <= radius){
			results.push_back(e);
		}
	}

	return results;
}

int run_trial(const int index, vector<perfmetric> &metrics){

	vector<entry_t> entries;
	generate_data(entries, n_entries);
	assert((int)entries.size() == n_entries);

	uint64_t centers[n_clusters];
	for (int i=0;i < n_clusters;i++){
		centers[i] = m_distrib(m_gen);
		generate_cluster(entries, centers[i], ClusterSize);
		assert((int)entries.size() == n_entries + (i+1)*ClusterSize);
	}

	entry_t::n_ops = 0;
	chrono::duration<double,milli> querytime(0);
	for (int i=0;i < n_clusters;i++){

		auto s = chrono::steady_clock::now();
		vector<entry_t> results = search(centers[i], Radius, entries);
		auto e = chrono::steady_clock::now();

		querytime += (e - s);
		cout << "Found " << results.size() << endl;
		assert((int)results.size() >= ClusterSize);
	}

	size_t sz = entries.size();
	
	double query_ops = 100.0*((double)entry_t::n_ops/(double)n_clusters/(double)sz);
	double query_time = (double)querytime.count()/(double)n_clusters;

	cout << " query ops " << dec << setprecision(6) << query_ops << " %opers  "
		 << "query time: " << dec << setprecision(6) << query_time << " millisecs" << endl;

	perfmetric m;
	m.avg_query_ops = query_ops;
	m.avg_query_time = query_time;
	m.mem = sz*sizeof(entry_t)/1000000.0;
	metrics.push_back(m);

	return 0;
}


int main(int argc, char **argv){

	cout << "Sequential search test" << endl;
	cout << "data set size: " << n_entries << endl;
	cout << "no. clusters: " << n_clusters << " of size: " << ClusterSize << endl << endl;

	vector<perfmetric> metrics;
	for (int i=0;i < n_runs;i++){
		run_trial(i, metrics);
	}

	double avg_query_ops = 0;
	double avg_query_time = 0;
	double avg_mem = 0;
	for (perfmetric &m : metrics){
		avg_query_ops += m.avg_query_ops/(double)n_runs;
		avg_query_time += m.avg_query_time/(double)n_runs;
		avg_mem += m.mem/(double)n_runs;
	}

	cout << "Avg. query ops: " << avg_query_ops << " % opers" << endl;
	cout << "Avg. query time: " << avg_query_time << " millisecs" << endl;
	cout << "Mem usage: " << avg_mem << endl;
	
	return 0;
}