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
    uint64_t binary_value;
    uint64_t dont_care_mask;
    set<int> covered_minterms;

    string as_binary_string(int variable_count) const;
    string as_boolean_expression(int variable_count) const;
    bool operator<(const Implicant& other) const;
};

struct MinimizationResult {
    vector<Implicant> all_prime_implicants;
    vector<Implicant> essential_prime_implicants;
    vector<int> minterms_not_covered_by_essentials;
    vector<vector<Implicant>> all_minimal_solutions;
};

class QuineMcCluskey {
public:
    QuineMcCluskey() = default;

    bool load_from_file(const string& file_path);
    MinimizationResult minimize();

    int variable_count = 0;
    vector<int> function_minterms;
    vector<int> function_dont_cares;

private:
    vector<Implicant> create_initial_implicants() const;
    vector<Implicant> find_all_prime_implicants() const;
    vector<Implicant> extract_essential_prime_implicants(
        const vector<Implicant>& prime_implicants,
        vector<int>& uncovered_minterms) const;
    vector<vector<Implicant>> find_minimal_covers(
        const vector<Implicant>& prime_implicants,
        const vector<Implicant>& essentials,
        const vector<int>& uncovered_minterms) const;
};

#endif // DD1_PROJ1_QUINE_H
