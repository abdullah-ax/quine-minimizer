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