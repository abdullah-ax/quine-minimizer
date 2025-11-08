//
// Created by Abdullah Ahmed on 06/11/2025.
//
// Implementation of the Quine-McCluskey algorithm for Boolean function minimization.
// The algorithm finds all prime implicants, identifies essential prime implicants,
// and determines minimal cover solutions.

#include "quine.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <bitset>
#include <functional>
#include <map>

using namespace std;

/**
 * Converts an implicant to its binary string representation.
 * Uses '-' for don't-care positions, '0' and '1' for fixed bits.
 *
 * @param variable_count Number of variables in the Boolean function
 * @return Binary string (e.g., "1-01" for a 4-variable implicant)
 */
string Implicant::as_binary_string(int variable_count) const {
    string binary_representation;
    for (int bit_position = variable_count - 1; bit_position >= 0; --bit_position) {
        bool is_dont_care = (dont_care_mask >> bit_position) & 1;
        if (is_dont_care) {
            binary_representation += '-';
        } else {
            bool bit_value = (binary_value >> bit_position) & 1;
            binary_representation += bit_value ? '1' : '0';
        }
    }
    return binary_representation;
}

/**
 * Converts an implicant to a product term in Boolean algebra notation.
 * Variables are named A, B, C, etc. Complemented variables use apostrophe (').
 * Don't-care positions are omitted from the expression.
 *
 * @param variable_count Number of variables in the Boolean function
 * @return Boolean expression (e.g., "AB'C" or "1" for a tautology)
 */
string Implicant::as_boolean_expression(int variable_count) const {
    string expression;

    for (int variable_index = 0; variable_index < variable_count; ++variable_index) {
        int bit_position = variable_count - 1 - variable_index;
        bool is_dont_care = (dont_care_mask >> bit_position) & 1;

        if (is_dont_care) continue;

        char variable_name = 'A' + variable_index;
        bool is_complemented = !((binary_value >> bit_position) & 1);

        expression += variable_name;
        if (is_complemented) {
            expression += '\'';
        }
    }

    return expression.empty() ? "1" : expression;
}

/**
 * Comparison operator for sorting and uniqueness checks.
 * Orders by don't-care mask first, then by binary value.
 */
bool Implicant::operator<(const Implicant& other) const {
    if (dont_care_mask != other.dont_care_mask) {
        return dont_care_mask < other.dont_care_mask;
    }
    return binary_value < other.binary_value;
}

// ==================== Helper Functions ====================
// These utility functions support file parsing and validation

static vector<string> split_by_comma(const string& text) {
    vector<string> tokens;
    stringstream stream(text);
    string token;

    while (getline(stream, token, ',')) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

static string trim_whitespace(string text) {
    while (!text.empty() && isspace(static_cast<unsigned char>(text.front()))) {
        text.erase(text.begin());
    }
    while (!text.empty() && isspace(static_cast<unsigned char>(text.back()))) {
        text.pop_back();
    }
    return text;
}

/**
 * Parses a term value from a string token (e.g., "m3" -> 3, "d5" -> 5).
 * Assumes the token format is: prefix followed by an integer.
 */
static bool parse_term_value(const string& token, char expected_prefix, int& term_value) {
    string trimmed_token = trim_whitespace(token);
    if (trimmed_token.size() < 2) return false;

    char actual_prefix = trimmed_token[0];
    if (actual_prefix != expected_prefix &&
        actual_prefix != toupper(expected_prefix) &&
        actual_prefix != tolower(expected_prefix)) {
        return false;
    }

    try {
        term_value = stoi(trimmed_token.substr(1));
        return true;
    } catch (...) {
        return false;
    }
}

static bool parse_dont_cares(const string& line, vector<int>& dont_cares) {
    dont_cares.clear();
    if (line.empty()) return true;

    for (const auto& token : split_by_comma(line)) {
        int dont_care_value;
        if (!parse_term_value(token, 'd', dont_care_value)) return false;
        dont_cares.push_back(dont_care_value);
    }
    return true;
}

static bool parse_terms_list(const string& line, vector<int>& terms, char& notation_type) {
    if (line.empty()) return false;

    notation_type = line[0];
    if (notation_type != 'm' && notation_type != 'M') return false;

    terms.clear();
    for (const auto& token : split_by_comma(line)) {
        int term_value;
        if (!parse_term_value(token, notation_type, term_value)) return false;
        terms.push_back(term_value);
    }
    return true;
}

static bool has_mixed_notation(const string& line) {
    if (line.empty()) return false;

    bool has_minterm = false;
    bool has_maxterm = false;

    for (const auto& token : split_by_comma(line)) {
        string trimmed_token = trim_whitespace(token);
        if (trimmed_token.empty()) continue;

        char prefix = trimmed_token[0];
        if (prefix == 'm') has_minterm = true;
        if (prefix == 'M') has_maxterm = true;
    }

    return has_minterm && has_maxterm;
}

/**
 * Converts maxterms to minterms using the complement principle.
 * Assumes: minterms = all combinations - maxterms - don't cares
 */
static void convert_maxterms_to_minterms(
    const vector<int>& maxterms,
    const vector<int>& dont_cares,
    int variable_count,
    vector<int>& minterms) {

    int total_combinations = 1 << variable_count;
    set<int> maxterm_set(maxterms.begin(), maxterms.end());
    set<int> dont_care_set(dont_cares.begin(), dont_cares.end());

    minterms.clear();
    for (int combination = 0; combination < total_combinations; ++combination) {
        if (maxterm_set.count(combination) || dont_care_set.count(combination)) continue;
        minterms.push_back(combination);
    }
}

static bool validate_no_overlap(const vector<int>& minterms, const vector<int>& dont_cares) {
    for (int dont_care : dont_cares) {
        if (find(minterms.begin(), minterms.end(), dont_care) != minterms.end()) {
            return false;
        }
    }
    return true;
}

// ==================== QuineMcCluskey File Parsing ====================

/**
 * Loads a Boolean function from a file.
 * Expected format:
 *   Line 1: Number of variables (1-20)
 *   Line 2: Minterms (e.g., "m0,m1,m3") or Maxterms (e.g., "M2,M4")
 *   Line 3: Don't-cares (e.g., "d5,d6") - optional
 *
 * @param file_path Path to the input file
 * @return true if file is successfully loaded and validated, false otherwise
 */
bool QuineMcCluskey::load_from_file(const string& file_path) {
    ifstream input_file(file_path);
    if (!input_file) return false;

    string variable_count_line, terms_line, dont_cares_line;
    if (!getline(input_file, variable_count_line)) return false;
    if (!getline(input_file, terms_line)) terms_line = "";
    if (!getline(input_file, dont_cares_line)) dont_cares_line = "";

    try {
        variable_count = stoi(variable_count_line);
    } catch (...) {
        return false;
    }

    if (variable_count <= 0 || variable_count > 20) return false;

    terms_line = trim_whitespace(terms_line);
    dont_cares_line = trim_whitespace(dont_cares_line);

    if (has_mixed_notation(terms_line)) return false;

    if (!parse_dont_cares(dont_cares_line, function_dont_cares)) return false;

    char notation_type;
    vector<int> parsed_terms;
    if (!parse_terms_list(terms_line, parsed_terms, notation_type)) return false;

    if (notation_type == 'M') {
        convert_maxterms_to_minterms(parsed_terms, function_dont_cares, variable_count, function_minterms);
    } else {
        function_minterms = parsed_terms;
    }

    if (!validate_no_overlap(function_minterms, function_dont_cares)) return false;

    return true;
}

// ==================== Algorithm Helper Functions ====================
// These functions support the core Quine-McCluskey algorithm operations

/**
 * Counts the number of 1-bits in the effective value (excluding don't-care positions).
 */
static int count_ones_in_value(uint64_t value, uint64_t dont_care_mask) {
    uint64_t effective_value = value & ~dont_care_mask;
    return static_cast<int>(bitset<64>(effective_value).count());
}

static bool implicants_are_equal(const Implicant& first_implicant, const Implicant& second_implicant) {
    return first_implicant.binary_value == second_implicant.binary_value &&
           first_implicant.dont_care_mask == second_implicant.dont_care_mask;
}

/**
 * Attempts to combine two implicants if they differ by exactly one bit.
 * Assumes: Both implicants must have identical don't-care masks to be combinable.
 */
static bool can_combine_implicants(
    const Implicant& first_implicant,
    const Implicant& second_implicant,
    Implicant& combined_implicant) {

    if (first_implicant.dont_care_mask != second_implicant.dont_care_mask) {
        return false;
    }

    uint64_t bit_difference = (first_implicant.binary_value ^ second_implicant.binary_value) & ~first_implicant.dont_care_mask;

    if (bit_difference == 0) return false;

    // Check if exactly one bit differs (power of 2 check: n & (n-1) == 0)
    if ((bit_difference & (bit_difference - 1)) != 0) return false;

    // Create combined implicant by setting the differing bit to don't-care
    combined_implicant = first_implicant;
    combined_implicant.binary_value &= ~bit_difference;
    combined_implicant.dont_care_mask |= bit_difference;
    combined_implicant.covered_minterms.clear();
    combined_implicant.covered_minterms.insert(first_implicant.covered_minterms.begin(), first_implicant.covered_minterms.end());
    combined_implicant.covered_minterms.insert(second_implicant.covered_minterms.begin(), second_implicant.covered_minterms.end());

    return true;
}

// ==================== QuineMcCluskey Algorithm ====================

/**
 * Creates the initial set of implicants from minterms and don't-cares.
 * Each minterm and don't-care becomes a single-term implicant.
 *
 * @return Vector of initial implicants (level 0 in the algorithm)
 */
vector<Implicant> QuineMcCluskey::create_initial_implicants() const {
    vector<Implicant> initial_implicants;

    auto add_term_as_implicant = [&](int term_value) {
        Implicant implicant;
        implicant.binary_value = static_cast<uint64_t>(term_value);
        implicant.dont_care_mask = 0;
        implicant.covered_minterms.insert(term_value);
        initial_implicants.push_back(implicant);
    };

    for (int minterm : function_minterms) {
        add_term_as_implicant(minterm);
    }
    for (int dont_care : function_dont_cares) {
        add_term_as_implicant(dont_care);
    }

    return initial_implicants;
}

/**
 * Combines implicants at the current level and identifies uncombined implicants.
 * Returns the next level of combined implicants.
 */
static vector<Implicant> combine_implicant_level(
    const vector<Implicant>& current_level_implicants,
    vector<Implicant>& uncombined_implicants) {

    vector<bool> was_combined(current_level_implicants.size(), false);
    vector<Implicant> next_level_implicants;
    set<pair<uint64_t, uint64_t>> seen_combinations;

    for (size_t first_index = 0; first_index < current_level_implicants.size(); ++first_index) {
        for (size_t second_index = first_index + 1; second_index < current_level_implicants.size(); ++second_index) {
            Implicant combined_implicant;
            if (can_combine_implicants(current_level_implicants[first_index],
                                      current_level_implicants[second_index],
                                      combined_implicant)) {
                was_combined[first_index] = was_combined[second_index] = true;

                // Avoid duplicates by using a set to track unique (value, mask) pairs
                auto combination_key = make_pair(combined_implicant.binary_value, combined_implicant.dont_care_mask);
                if (seen_combinations.insert(combination_key).second) {
                    next_level_implicants.push_back(combined_implicant);
                }
            }
        }
    }

    uncombined_implicants.clear();
    for (size_t implicant_index = 0; implicant_index < current_level_implicants.size(); ++implicant_index) {
        if (!was_combined[implicant_index]) {
            uncombined_implicants.push_back(current_level_implicants[implicant_index]);
        }
    }

    return next_level_implicants;
}

static void sort_by_ones_count(vector<Implicant>& implicants) {
    sort(implicants.begin(), implicants.end(), [](const Implicant& first, const Implicant& second) {
        int ones_count_first = count_ones_in_value(first.binary_value, first.dont_care_mask);
        int ones_count_second = count_ones_in_value(second.binary_value, second.dont_care_mask);
        if (ones_count_first != ones_count_second) return ones_count_first < ones_count_second;
        if (first.dont_care_mask != second.dont_care_mask) return first.dont_care_mask < second.dont_care_mask;
        return first.binary_value < second.binary_value;
    });
}

/**
 * Finds all prime implicants using iterative combination.
 *
 * Process:
 *   1. Start with initial implicants (one per minterm/don't-care)
 *   2. Sort by number of 1s for efficient comparison
 *   3. Combine implicants differing by exactly one bit
 *   4. Collect uncombined implicants as prime implicants
 *   5. Repeat with combined implicants until no more combinations possible
 *
 * @return Vector of all prime implicants found
 */
vector<Implicant> QuineMcCluskey::find_all_prime_implicants() const {
    vector<Implicant> current_level_implicants = create_initial_implicants();
    vector<Implicant> all_prime_implicants;

    while (!current_level_implicants.empty()) {
        sort_by_ones_count(current_level_implicants);

        vector<Implicant> uncombined_implicants;
        vector<Implicant> next_level_implicants = combine_implicant_level(current_level_implicants, uncombined_implicants);

        all_prime_implicants.insert(all_prime_implicants.end(), uncombined_implicants.begin(), uncombined_implicants.end());
        current_level_implicants = move(next_level_implicants);
    }

    // Remove duplicates that may have been found through different combination paths
    sort(all_prime_implicants.begin(), all_prime_implicants.end());
    all_prime_implicants.erase(
        unique(all_prime_implicants.begin(), all_prime_implicants.end(), implicants_are_equal),
        all_prime_implicants.end()
    );

    return all_prime_implicants;
}

/**
 * Builds a coverage chart mapping each minterm to the indices of prime implicants that cover it.
 */
static map<int, vector<int>> build_coverage_chart(
    const vector<int>& minterms,
    const vector<Implicant>& prime_implicants) {

    map<int, vector<int>> minterm_to_prime_implicant_indices;

    for (int minterm : minterms) {
        for (size_t prime_index = 0; prime_index < prime_implicants.size(); ++prime_index) {
            if (prime_implicants[prime_index].covered_minterms.count(minterm)) {
                minterm_to_prime_implicant_indices[minterm].push_back(static_cast<int>(prime_index));
            }
        }
    }

    return minterm_to_prime_implicant_indices;
}

/**
 * Finds essential prime implicants - those that are the only cover for at least one minterm.
 */
static vector<int> find_essential_indices(const map<int, vector<int>>& coverage_chart) {
    set<int> essential_prime_indices;

    for (const auto& minterm_coverage : coverage_chart) {
        if (minterm_coverage.second.size() == 1) {
            essential_prime_indices.insert(minterm_coverage.second[0]);
        }
    }

    return vector<int>(essential_prime_indices.begin(), essential_prime_indices.end());
}

static set<int> get_covered_by_indices(
    const vector<int>& prime_implicant_indices,
    const vector<Implicant>& prime_implicants) {

    set<int> covered_minterms;
    for (int prime_index : prime_implicant_indices) {
        const auto& minterms = prime_implicants[prime_index].covered_minterms;
        covered_minterms.insert(minterms.begin(), minterms.end());
    }
    return covered_minterms;
}

/**
 * Extracts essential prime implicants from the set of all prime implicants.
 *
 * An essential prime implicant is one that is the ONLY prime implicant
 * covering at least one minterm. These must appear in every minimal solution.
 *
 * @param prime_implicants All prime implicants found
 * @param uncovered_minterms Output: minterms not covered by essential PIs
 * @return Vector of essential prime implicants
 */
vector<Implicant> QuineMcCluskey::extract_essential_prime_implicants(
    const vector<Implicant>& prime_implicants,
    vector<int>& uncovered_minterms) const {

    auto coverage_chart = build_coverage_chart(function_minterms, prime_implicants);
    auto essential_indices = find_essential_indices(coverage_chart);

    vector<Implicant> essential_prime_implicants;
    for (int essential_index : essential_indices) {
        essential_prime_implicants.push_back(prime_implicants[essential_index]);
    }

    auto minterms_covered_by_essentials = get_covered_by_indices(essential_indices, prime_implicants);

    uncovered_minterms.clear();
    for (int minterm : function_minterms) {
        if (!minterms_covered_by_essentials.count(minterm)) {
            uncovered_minterms.push_back(minterm);
        }
    }

    return essential_prime_implicants;
}

static vector<int> get_remaining_pi_indices(
    const vector<Implicant>& prime_implicants,
    const vector<Implicant>& essential_prime_implicants,
    const vector<int>& uncovered_minterms) {

    vector<int> remaining_prime_indices;

    for (size_t prime_index = 0; prime_index < prime_implicants.size(); ++prime_index) {
        bool is_essential = false;
        for (const auto& essential_implicant : essential_prime_implicants) {
            if (implicants_are_equal(prime_implicants[prime_index], essential_implicant)) {
                is_essential = true;
                break;
            }
        }
        if (is_essential) continue;

        bool covers_any_uncovered = false;
        for (int uncovered_minterm : uncovered_minterms) {
            if (prime_implicants[prime_index].covered_minterms.count(uncovered_minterm)) {
                covers_any_uncovered = true;
                break;
            }
        }

        if (covers_any_uncovered) {
            remaining_prime_indices.push_back(static_cast<int>(prime_index));
        }
    }

    return remaining_prime_indices;
}

static bool combination_covers_all(
    const vector<int>& combination_indices,
    const vector<int>& remaining_prime_indices,
    const vector<Implicant>& prime_implicants,
    const vector<int>& uncovered_minterms) {

    set<int> minterms_covered_by_combination;
    for (int combination_index : combination_indices) {
        int prime_index = remaining_prime_indices[combination_index];
        const auto& covered_minterms = prime_implicants[prime_index].covered_minterms;
        for (int uncovered_minterm : uncovered_minterms) {
            if (covered_minterms.count(uncovered_minterm)) {
                minterms_covered_by_combination.insert(uncovered_minterm);
            }
        }
    }

    return minterms_covered_by_combination.size() == uncovered_minterms.size();
}

/**
 * Finds minimal combinations of prime implicants that cover all uncovered minterms.
 * Assumes: Uses brute-force search up to a maximum subset size of 6 for performance.
 */
static vector<vector<int>> find_minimal_combinations(
    const vector<int>& remaining_prime_indices,
    const vector<Implicant>& prime_implicants,
    const vector<int>& uncovered_minterms) {

    const int max_combination_size = 6;
    vector<vector<int>> minimal_solutions;
    size_t total_remaining_primes = remaining_prime_indices.size();

    for (int combination_size = 1; combination_size <= min(max_combination_size, static_cast<int>(total_remaining_primes)); ++combination_size) {
        vector<int> current_combination(combination_size);

        // Recursive lambda to generate all combinations of size combination_size
        function<void(int, int)> generate_combinations = [&](int start_position, int current_depth) {
            if (current_depth == combination_size) {
                if (combination_covers_all(current_combination, remaining_prime_indices,
                                          prime_implicants, uncovered_minterms)) {
                    minimal_solutions.push_back(current_combination);
                }
                return;
            }

            for (int position = start_position; position <= static_cast<int>(total_remaining_primes) - (combination_size - current_depth); ++position) {
                current_combination[current_depth] = position;
                generate_combinations(position + 1, current_depth + 1);
            }
        };

        generate_combinations(0, 0);
        // Early termination: stop when we find minimal solutions at this size
        if (!minimal_solutions.empty()) break;
    }

    return minimal_solutions;
}

vector<vector<Implicant>> QuineMcCluskey::find_minimal_covers(
    const vector<Implicant>& prime_implicants,
    const vector<Implicant>& essential_prime_implicants,
    const vector<int>& uncovered_minterms) const {

    if (uncovered_minterms.empty()) {
        return {essential_prime_implicants};
    }

    auto remaining_prime_indices = get_remaining_pi_indices(
        prime_implicants, essential_prime_implicants, uncovered_minterms
    );

    auto minimal_combination_indices = find_minimal_combinations(
        remaining_prime_indices, prime_implicants, uncovered_minterms
    );

    vector<vector<Implicant>> all_minimal_solutions;
    for (const auto& combination_indices : minimal_combination_indices) {
        vector<Implicant> complete_solution = essential_prime_implicants;
        for (int combination_index : combination_indices) {
            int prime_index = remaining_prime_indices[combination_index];
            complete_solution.push_back(prime_implicants[prime_index]);
        }
        all_minimal_solutions.push_back(complete_solution);
    }

    return all_minimal_solutions;
}

/**
 * Main entry point for the Quine-McCluskey minimization algorithm.
 *
 * Algorithm flow:
 *   1. Find all prime implicants through iterative combination
 *   2. Identify essential prime implicants (must be in every solution)
 *   3. Find minimal covers for any remaining uncovered minterms
 *   4. Return all minimal solutions
 *
 * @return MinimizationResult containing all PIs, essential PIs, and minimal solutions
 */
MinimizationResult QuineMcCluskey::minimize() {
    MinimizationResult result;

    result.all_prime_implicants = find_all_prime_implicants();

    vector<int> minterms_uncovered_by_essentials;
    result.essential_prime_implicants = extract_essential_prime_implicants(
        result.all_prime_implicants,
        minterms_uncovered_by_essentials
    );

    result.minterms_not_covered_by_essentials = minterms_uncovered_by_essentials;

    result.all_minimal_solutions = find_minimal_covers(
        result.all_prime_implicants,
        result.essential_prime_implicants,
        minterms_uncovered_by_essentials
    );

    return result;
}