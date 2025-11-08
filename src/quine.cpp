//
// Created by Abdullah Ahmed on 06/11/2025.
//

#include "quine.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <bitset>
#include <functional>

using namespace std;

string Implicant::as_binary_string(int variable_count) const {
    string result;
    for (int i = variable_count - 1; i >= 0; --i) {
        bool is_dont_care = (dont_care_mask >> i) & 1;
        if (is_dont_care) {
            result += '-';
        } else {
            bool bit_value = (binary_value >> i) & 1;
            result += bit_value ? '1' : '0';
        }
    }
    return result;
}

string Implicant::as_boolean_expression(int variable_count) const {
    string expression;

    for (int i = 0; i < variable_count; ++i) {
        int bit_position = variable_count - 1 - i;
        bool is_dont_care = (dont_care_mask >> bit_position) & 1;

        if (is_dont_care) continue;

        char variable_name = 'A' + i;
        bool is_complemented = !((binary_value >> bit_position) & 1);

        expression += variable_name;
        if (is_complemented) {
            expression += '\'';
        }
    }

    return expression.empty() ? "1" : expression;
}

bool Implicant::operator<(const Implicant& other) const {
    if (dont_care_mask != other.dont_care_mask) {
        return dont_care_mask < other.dont_care_mask;
    }
    return binary_value < other.binary_value;
}

// ==================== Helper Functions ====================

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

static bool parse_term_value(const string& token, char expected_prefix, int& value) {
    string trimmed = trim_whitespace(token);
    if (trimmed.size() < 2) return false;

    char prefix = trimmed[0];
    if (prefix != expected_prefix && prefix != toupper(expected_prefix) &&
        prefix != tolower(expected_prefix)) {
        return false;
    }

    try {
        value = stoi(trimmed.substr(1));
        return true;
    } catch (...) {
        return false;
    }
}

static bool parse_dont_cares(const string& line, vector<int>& dont_cares) {
    dont_cares.clear();
    if (line.empty()) return true;

    for (const auto& token : split_by_comma(line)) {
        int value;
        if (!parse_term_value(token, 'd', value)) return false;
        dont_cares.push_back(value);
    }
    return true;
}

static bool parse_terms_list(const string& line, vector<int>& terms, char& notation_type) {
    if (line.empty()) return false;

    notation_type = line[0];
    if (notation_type != 'm' && notation_type != 'M') return false;

    terms.clear();
    for (const auto& token : split_by_comma(line)) {
        int value;
        if (!parse_term_value(token, notation_type, value)) return false;
        terms.push_back(value);
    }
    return true;
}

static bool has_mixed_notation(const string& line) {
    if (line.empty()) return false;

    bool has_minterm = false;
    bool has_maxterm = false;

    for (const auto& token : split_by_comma(line)) {
        string trimmed = trim_whitespace(token);
        if (trimmed.empty()) continue;

        char prefix = trimmed[0];
        if (prefix == 'm') has_minterm = true;
        if (prefix == 'M') has_maxterm = true;
    }

    return has_minterm && has_maxterm;
}

static void convert_maxterms_to_minterms(
    const vector<int>& maxterms,
    const vector<int>& dont_cares,
    int variable_count,
    vector<int>& minterms) {

    int total_combinations = 1 << variable_count;
    set<int> maxterm_set(maxterms.begin(), maxterms.end());
    set<int> dont_care_set(dont_cares.begin(), dont_cares.end());

    minterms.clear();
    for (int i = 0; i < total_combinations; ++i) {
        if (maxterm_set.count(i) || dont_care_set.count(i)) continue;
        minterms.push_back(i);
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

bool QuineMcCluskey::load_from_file(const string& file_path) {
    ifstream input_file(file_path);
    if (!input_file) return false;

    string first_line, second_line, third_line;
    if (!getline(input_file, first_line)) return false;
    if (!getline(input_file, second_line)) second_line = "";
    if (!getline(input_file, third_line)) third_line = "";

    try {
        variable_count = stoi(first_line);
    } catch (...) {
        return false;
    }

    if (variable_count <= 0 || variable_count > 20) return false;

    second_line = trim_whitespace(second_line);
    third_line = trim_whitespace(third_line);

    if (has_mixed_notation(second_line)) return false;

    if (!parse_dont_cares(third_line, function_dont_cares)) return false;

    char notation_type;
    vector<int> listed_terms;
    if (!parse_terms_list(second_line, listed_terms, notation_type)) return false;

    if (notation_type == 'M') {
        convert_maxterms_to_minterms(listed_terms, function_dont_cares, variable_count, function_minterms);
    } else {
        function_minterms = listed_terms;
    }

    if (!validate_no_overlap(function_minterms, function_dont_cares)) return false;

    return true;
}

// ==================== Algorithm Helper Functions ====================

static int count_ones_in_value(uint64_t value, uint64_t mask) {
    uint64_t effective_value = value & ~mask;
    return static_cast<int>(bitset<64>(effective_value).count());
}

static bool implicants_are_equal(const Implicant& a, const Implicant& b) {
    return a.binary_value == b.binary_value && a.dont_care_mask == b.dont_care_mask;
}

static bool can_combine_implicants(
    const Implicant& first,
    const Implicant& second,
    Implicant& combined) {

    if (first.dont_care_mask != second.dont_care_mask) {
        return false;
    }

    uint64_t bit_difference = (first.binary_value ^ second.binary_value) & ~first.dont_care_mask;

    if (bit_difference == 0) return false;
    if ((bit_difference & (bit_difference - 1)) != 0) return false;

    combined = first;
    combined.binary_value &= ~bit_difference;
    combined.dont_care_mask |= bit_difference;
    combined.covered_minterms.clear();
    combined.covered_minterms.insert(first.covered_minterms.begin(), first.covered_minterms.end());
    combined.covered_minterms.insert(second.covered_minterms.begin(), second.covered_minterms.end());

    return true;
}

// ==================== QuineMcCluskey Algorithm ====================

vector<Implicant> QuineMcCluskey::create_initial_implicants() const {
    vector<Implicant> initial_list;

    auto add_term = [&](int term_value) {
        Implicant implicant;
        implicant.binary_value = static_cast<uint64_t>(term_value);
        implicant.dont_care_mask = 0;
        implicant.covered_minterms.insert(term_value);
        initial_list.push_back(implicant);
    };

    for (int minterm : function_minterms) {
        add_term(minterm);
    }
    for (int dont_care : function_dont_cares) {
        add_term(dont_care);
    }

    return initial_list;
}

static vector<Implicant> combine_implicant_level(
    const vector<Implicant>& current_level,
    vector<Implicant>& uncombined) {

    vector<bool> was_combined(current_level.size(), false);
    vector<Implicant> next_level;
    set<pair<uint64_t, uint64_t>> seen_combinations;

    for (size_t i = 0; i < current_level.size(); ++i) {
        for (size_t j = i + 1; j < current_level.size(); ++j) {
            Implicant combined;
            if (can_combine_implicants(current_level[i], current_level[j], combined)) {
                was_combined[i] = was_combined[j] = true;

                auto key = make_pair(combined.binary_value, combined.dont_care_mask);
                if (seen_combinations.insert(key).second) {
                    next_level.push_back(combined);
                }
            }
        }
    }

    uncombined.clear();
    for (size_t i = 0; i < current_level.size(); ++i) {
        if (!was_combined[i]) {
            uncombined.push_back(current_level[i]);
        }
    }

    return next_level;
}

static void sort_by_ones_count(vector<Implicant>& implicants) {
    sort(implicants.begin(), implicants.end(), [](const Implicant& a, const Implicant& b) {
        int ones_a = count_ones_in_value(a.binary_value, a.dont_care_mask);
        int ones_b = count_ones_in_value(b.binary_value, b.dont_care_mask);
        if (ones_a != ones_b) return ones_a < ones_b;
        if (a.dont_care_mask != b.dont_care_mask) return a.dont_care_mask < b.dont_care_mask;
        return a.binary_value < b.binary_value;
    });
}

vector<Implicant> QuineMcCluskey::find_all_prime_implicants() const {
    vector<Implicant> current_level = create_initial_implicants();
    vector<Implicant> all_primes;

    while (!current_level.empty()) {
        sort_by_ones_count(current_level);

        vector<Implicant> uncombined;
        vector<Implicant> next_level = combine_implicant_level(current_level, uncombined);

        all_primes.insert(all_primes.end(), uncombined.begin(), uncombined.end());
        current_level = move(next_level);
    }

    sort(all_primes.begin(), all_primes.end());
    all_primes.erase(
        unique(all_primes.begin(), all_primes.end(), implicants_are_equal),
        all_primes.end()
    );

    return all_primes;
}