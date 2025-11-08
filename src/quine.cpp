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

    // Parse don't-cares
    function_dont_cares.clear();
    if (!third_line.empty()) {
        for (auto& token : split_by_comma(third_line)) {
            token = trim_whitespace(token);
            if (token.size() < 2) return false;
            if (token[0] != 'd' && token[0] != 'D') return false;

            int value = stoi(token.substr(1));
            function_dont_cares.push_back(value);
        }
    }

    return true;
}