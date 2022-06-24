#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <random>
#include <chrono>
#include <cassert>

using namespace std;


static long long m_id = 1;
static long long g_id = 100000;

static random_device m_rd;
static mt19937_64 m_gen(m_rd());
static uniform_int_distribution<uint64_t> m_distrib(0);


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

int generate_cluster(vector<entry_t> &entries, const uint64_t center, const int radius, const int N){

	uniform_int_distribution<int> m_radius(1,radius);
	uniform_int_distribution<int> m_bitindex(0,63);

	uint64_t mask = 0x0001ULL;
	entries.push_back({ g_id++, center });

	for (int i=0;i < N-1;i++){
		uint64_t val = center;
		if (radius > 0){
			int dist = m_radius(m_gen);
			for (int j=0;j < dist;j++){
				val ^= (mask << m_bitindex(m_gen));
			}
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

void run(const int index, const int n_entries, const int n_clusters,
		 const int cluster_size, const int radius,  vector<perfmetric> &metrics){

	vector<entry_t> entries;
	generate_data(entries, n_entries);
	assert((int)entries.size() == n_entries);

	uint64_t centers[n_clusters];
	for (int i=0;i < n_clusters;i++){
		centers[i] = m_distrib(m_gen);
		generate_cluster(entries, centers[i], radius, cluster_size);
		assert((int)entries.size() == n_entries + (i+1)*cluster_size);
	}

	entry_t::n_ops = 0;
	chrono::duration<double,milli> querytime(0);
	for (int i=0;i < n_clusters;i++){

		auto s = chrono::steady_clock::now();
		vector<entry_t> results = search(centers[i], radius, entries);
		auto e = chrono::steady_clock::now();

		querytime += (e - s);
		assert((int)results.size() >= cluster_size);
	}

	size_t sz = entries.size();
	
	double query_ops = 100.0*((double)entry_t::n_ops/(double)n_clusters/(double)sz);
	double query_time = (double)querytime.count()/(double)n_clusters;

	cout << "(" << index << ") query opers: " << dec << setprecision(6) << query_ops << " %opers  "
		 << "query time: " << dec << setprecision(6) << query_time << " millisecs" << endl;

	perfmetric m;
	m.avg_query_ops = query_ops;
	m.avg_query_time = query_time;
	m.mem = (double)sz*sizeof(entry_t)/1000000.0;
	metrics.push_back(m);

}


void do_experiment(const int n_runs, const int n_entries, const int n_clusters,
				  const int cluster_size, const int radius){

	cout << "------------N = " << n_entries << "---------------" << endl;
	cout << "data set size: " << n_entries << endl;
	cout << "no. clusters: " << n_clusters << " of size: " << cluster_size << endl << endl;

	vector<perfmetric> metrics;
	for (int i=0;i < n_runs;i++){
		run(i, n_entries, n_clusters, cluster_size, radius, metrics);
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
	cout << "Mem usage: " << avg_mem << " MB" << endl;
	cout << "--------------------------------------" << endl << endl;
	
}


int main(int argc, char **argv){

	const int n_runs = 1;
	const int n_experiments = 15;
	const int n_entries[n_experiments] = { 100000, 200000, 400000, 800000,
										   1000000, 2000000, 4000000, 8000000,
										   16000000, 32000000, 64000000,  128000000,
										   256000000, 512000000, 1000000000};
	const int n_clusters = 10;
	const int cluster_size = 10;
	const int radius = 10;

	cout << "Sequential search test for 64-dim hamming space elements" << endl;

	for (int i=0;i < n_experiments;i++){
		do_experiment(n_runs, n_entries[i], n_clusters, cluster_size, radius);
	}

	return 0;
}
