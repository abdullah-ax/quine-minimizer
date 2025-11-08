//
// Created by Abdullah Ahmed on 06/11/2025.
//
// Header file for Quine-McCluskey Boolean minimization algorithm.
// Defines the data structures and interface for minimizing Boolean functions.

#ifndef QUINE_MINIMIZER_QUINE_H
#define QUINE_MINIMIZER_QUINE_H

#include <string>
#include <vector>
#include <set>
#include <cstdint>

using namespace std;

/**
 * Represents a product term (implicant) in a Boolean function.
 * Uses binary representation with a don't-care mask for optimization.
 */
struct Implicant {
    uint64_t binary_value;        // Binary value with don't-cares set to 0
    uint64_t dont_care_mask;      // Bit mask: 1 = don't-care position, 0 = fixed
    set<int> covered_minterms;    // Original minterms covered by this implicant

    string as_binary_string(int variable_count) const;
    string as_boolean_expression(int variable_count) const;
    bool operator<(const Implicant& other) const;
};

/**
 * Contains the complete results of Boolean function minimization.
 */
struct MinimizationResult {
    vector<Implicant> all_prime_implicants;              // All prime implicants found
    vector<Implicant> essential_prime_implicants;        // Essential PIs (must be in solution)
    vector<int> minterms_not_covered_by_essentials;      // Minterms needing additional coverage
    vector<vector<Implicant>> all_minimal_solutions;     // All minimal cover solutions
};

/**
 * Implements the Quine-McCluskey algorithm for Boolean function minimization.
 * Finds minimal sum-of-products (SOP) expressions for Boolean functions.
 */
class QuineMcCluskey {
public:
    QuineMcCluskey() = default;

    // Load a Boolean function from a file
    bool load_from_file(const string& file_path);

    // Run the minimization algorithm
    MinimizationResult minimize();

    // Input data (public for direct access)
    int variable_count = 0;
    vector<int> function_minterms;
    vector<int> function_dont_cares;

private:
    // Phase 1: Create initial implicants from input
    vector<Implicant> create_initial_implicants() const;

    // Phase 2: Find all prime implicants through iterative combination
    vector<Implicant> find_all_prime_implicants() const;

    // Phase 3: Extract essential prime implicants from the prime implicant table
    vector<Implicant> extract_essential_prime_implicants(
        const vector<Implicant>& prime_implicants,
        vector<int>& uncovered_minterms) const;

    // Phase 4: Find minimal covers for remaining uncovered minterms
    vector<vector<Implicant>> find_minimal_covers(
        const vector<Implicant>& prime_implicants,
        const vector<Implicant>& essentials,
        const vector<int>& uncovered_minterms) const;
};

#endif // QUINE_MINIMIZER_QUINE_H
