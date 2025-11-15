//
// Created by Abdullah Ahmed on 15/11/2025.
//

#include "verilog_generator.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

using namespace std;

/**
 * Gets variable name from index (0->A, 1->B, etc.)
 */
string VerilogGenerator::get_variable_name(int index) {
    return string(1, 'A' + index);
}

/**
 * Determines which input variables need inverters based on the solution.
 * Returns a boolean vector where true means that variable needs a NOT gate.
 */
vector<bool> VerilogGenerator::get_complemented_variables(
    const vector<Implicant>& solution,
    int variable_count) {

    vector<bool> needs_complement(variable_count, false);

    for (const auto& implicant : solution) {
        for (int var_idx = 0; var_idx < variable_count; ++var_idx) {
            int bit_pos = variable_count - 1 - var_idx;

            // Check if this bit is used (not a don't-care)
            bool is_dont_care = (implicant.dont_care_mask >> bit_pos) & 1;
            if (is_dont_care) continue;

            // Check if this bit is complemented (0 = needs inversion)
            bool bit_value = (implicant.binary_value >> bit_pos) & 1;
            if (!bit_value) {
                needs_complement[var_idx] = true;
            }
        }
    }

    return needs_complement;
}

/**
 * Generates the module header with input/output ports.
 * Format: module <name> (input A, B, C, ..., output F);
 */
string VerilogGenerator::generate_module_header(
    int variable_count,
    const string& module_name) {

    ostringstream oss;

    oss << "module " << module_name << " (\n";
    oss << "    input ";

    // Generate input port list
    for (int i = 0; i < variable_count; ++i) {
        if (i > 0) oss << ", ";
        oss << get_variable_name(i);
    }

    oss << ",\n";
    oss << "    output F\n";
    oss << ");\n\n";

    return oss.str();
}

/**
 * Generates wire declarations for intermediate signals.
 * Creates wires for: inverted inputs, product terms, and any other intermediates.
 */
string VerilogGenerator::generate_wire_declarations(
    const vector<Implicant>& solution,
    int variable_count,
    int& wire_count) {

    ostringstream oss;

    auto needs_complement = get_complemented_variables(solution, variable_count);

    // Wires for complemented inputs
    oss << "    // Inverted input signals\n";
    bool has_inverted = false;
    for (int i = 0; i < variable_count; ++i) {
        if (needs_complement[i]) {
            oss << "    wire " << get_variable_name(i) << "_n;\n";
            has_inverted = true;
        }
    }
    if (!has_inverted) {
        oss << "    // (no inverted inputs needed)\n";
    }
    oss << "\n";

    // Wires for product terms (one per implicant)
    oss << "    // Product term outputs\n";
    if (solution.size() == 1) {
        // Special case: only one product term, no intermediate wire needed
        oss << "    // (single product term - direct connection to output)\n";
        wire_count = 0;
    } else {
        for (size_t i = 0; i < solution.size(); ++i) {
            oss << "    wire p" << i << ";\n";
        }
        wire_count = solution.size();
    }
    oss << "\n";

    return oss.str();
}

/**
 * Generates NOT gate instantiations for complemented inputs.
 * Format: not g0 (A_n, A);
 */
string VerilogGenerator::generate_input_inverters(
    int variable_count,
    const vector<Implicant>& solution) {

    ostringstream oss;
    auto needs_complement = get_complemented_variables(solution, variable_count);

    oss << "    // NOT gates for complemented inputs\n";

    int gate_num = 0;
    bool has_inverters = false;
    for (int i = 0; i < variable_count; ++i) {
        if (needs_complement[i]) {
            string var = get_variable_name(i);
            oss << "    not g" << gate_num++ << " (" << var << "_n, " << var << ");\n";
            has_inverters = true;
        }
    }

    if (!has_inverters) {
        oss << "    // (no NOT gates needed)\n";
    }
    oss << "\n";

    return oss.str();
}

/**
 * Generates AND gates for each product term in the SOP expression.
 * Each implicant becomes one AND gate combining its literals.
 */
string VerilogGenerator::generate_product_terms(
    const vector<Implicant>& solution,
    int variable_count,
    int& wire_count) {

    ostringstream oss;
    oss << "    // AND gates for product terms\n";

    int gate_num = wire_count; // Continue gate numbering

    for (size_t prod_idx = 0; prod_idx < solution.size(); ++prod_idx) {
        const auto& implicant = solution[prod_idx];

        // Collect all literals for this product term
        vector<string> literals;
        for (int var_idx = 0; var_idx < variable_count; ++var_idx) {
            int bit_pos = variable_count - 1 - var_idx;

            bool is_dont_care = (implicant.dont_care_mask >> bit_pos) & 1;
            if (is_dont_care) continue; // Skip don't-care positions

            bool bit_value = (implicant.binary_value >> bit_pos) & 1;
            string var = get_variable_name(var_idx);

            // Use complemented or uncomplemented version
            literals.push_back(bit_value ? var : var + "_n");
        }

        // Special cases
        if (literals.empty()) {
            // Tautology (all don't-cares) - represents constant 1
            oss << "    // Product term " << prod_idx << ": constant 1 (tautology)\n";
            if (solution.size() == 1) {
                oss << "    assign F = 1'b1;\n";
            } else {
                oss << "    assign p" << prod_idx << " = 1'b1;\n";
            }
        } else if (literals.size() == 1) {
            // Single literal - direct assignment
            oss << "    // Product term " << prod_idx << ": " << literals[0] << "\n";
            if (solution.size() == 1) {
                oss << "    assign F = " << literals[0] << ";\n";
            } else {
                oss << "    assign p" << prod_idx << " = " << literals[0] << ";\n";
            }
        } else {
            // Multiple literals - need AND gate
            oss << "    // Product term " << prod_idx << ": ";
            for (size_t i = 0; i < literals.size(); ++i) {
                if (i > 0) oss << " & ";
                oss << literals[i];
            }
            oss << "\n";

            string output_wire = (solution.size() == 1) ? "F" : "p" + to_string(prod_idx);

            oss << "    and g" << gate_num++ << " (" << output_wire;
            for (const auto& lit : literals) {
                oss << ", " << lit;
            }
            oss << ");\n";
        }
    }

    oss << "\n";
    wire_count = gate_num;
    return oss.str();
}

/**
 * Generates the final OR gate that combines all product terms.
 * Only needed if there are multiple product terms.
 */
string VerilogGenerator::generate_sum_of_products(
    const vector<Implicant>& solution,
    int product_wire_start) {

    ostringstream oss;

    if (solution.size() <= 1) {
        // No OR gate needed for single product term
        return "";
    }

    oss << "    // OR gate for sum of products\n";
    oss << "    or g" << product_wire_start << " (F";

    for (size_t i = 0; i < solution.size(); ++i) {
        oss << ", p" << i;
    }

    oss << ");\n\n";

    return oss.str();
}

/**
 * Main method to generate complete Verilog module.
 * Combines all parts: header, wires, gates, and endmodule.
 */
string VerilogGenerator::generate_module(
    const vector<Implicant>& solution,
    int variable_count,
    const string& module_name) {

    ostringstream verilog;

    // Handle empty solution (function is always 0)
    if (solution.empty()) {
        verilog << generate_module_header(variable_count, module_name);
        verilog << "    // Function is always 0 (no minterms)\n";
        verilog << "    assign F = 1'b0;\n\n";
        verilog << "endmodule\n";
        return verilog.str();
    }

    // Generate each section
    verilog << generate_module_header(variable_count, module_name);

    int wire_count = 0;
    verilog << generate_wire_declarations(solution, variable_count, wire_count);
    verilog << generate_input_inverters(variable_count, solution);
    verilog << generate_product_terms(solution, variable_count, wire_count);
    verilog << generate_sum_of_products(solution, wire_count);

    verilog << "endmodule\n";

    return verilog.str();
}

/**
 * Generates only the gate instantiation logic (for debugging/testing).
 */
string VerilogGenerator::generate_gate_logic(
    const vector<Implicant>& solution,
    int variable_count) {

    ostringstream logic;

    int wire_count = 0;
    logic << generate_input_inverters(variable_count, solution);
    logic << generate_product_terms(solution, variable_count, wire_count);
    logic << generate_sum_of_products(solution, wire_count);

    return logic.str();
}