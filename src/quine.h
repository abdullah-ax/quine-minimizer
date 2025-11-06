//
// Created by Abdullah Ahmed on 06/11/2025.
//

#ifndef DD1_PROJ1_QUINE_H
#define DD1_PROJ1_QUINE_H
#include <string>
#include <vector>
#include <set>
#include <cstdint>

using namespace std;

struct Implicant {
    uint64_t bits;   // bit values
    uint64_t mask;   // 1 => don't-care ('-') at that bit
    set<int> covers; // minterms (include don't-cares here too)

    int var_count() const;
    string to_binary(int n) const;
    string to_expression(int n) const;
    bool covers_minterm(int m) const;
    bool operator<(Implicant const& o) const;
};

struct QMResult {
    vector<Implicant> prime_implicants;
    vector<Implicant> essential_implicants;
    vector<int> uncovered_minterms;
    vector<vector<Implicant>> minimal_solutions;
};

class QuineMcCluskey {
public:
    QuineMcCluskey() = default;
    bool parse_input_file(const string& path);
    QMResult solve();

    // data filled by parse_input_file
    int num_vars = 0;
    vector<int> minterms;
    vector<int> dontcares;

private:
    vector<Implicant> initial_implicants() const;
    vector<Implicant> generate_prime_implicants() const;
    void build_pi_chart(const vector<Implicant>& pis,
                        vector<vector<int>>& chart,
                        vector<int>& minterm_list) const;
    QMResult compute_solution(const vector<Implicant>& pis) const;
};
#endif //DD1_PROJ1_QUINE_H
