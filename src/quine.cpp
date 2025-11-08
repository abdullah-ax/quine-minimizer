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