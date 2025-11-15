//
// Created by Abdullah Ahmed on 15/11/2025.
//

#ifndef QUINE_MINIMIZER_VERILOG_GENERATOR_H
#define QUINE_MINIMIZER_VERILOG_GENERATOR_H

#include "quine.h"
#include <string>
#include <vector>

using namespace std;

/**
 * VerilogGenerator class
 *
 * Converts minimized Boolean expressions into Verilog modules using
 * primitive gates (AND, OR, NOT). Generates structural Verilog code
 * that can be synthesized to hardware.
 */
class VerilogGenerator {
public:
    /**
     * Generates a complete Verilog module from a minimal solution
     *
     * @param solution Vector of implicants representing the minimal SOP form
     * @param variable_count Number of input variables (determines port width)
     * @param module_name Name of the generated Verilog module
     * @return Complete Verilog module as a string
     */
    static string generate_module(
        const vector<Implicant>& solution,
        int variable_count,
        const string& module_name = "boolean_function"
    );

    /**
     * Generates just the gate instantiations (for testing/debugging)
     *
     * @param solution Vector of implicants
     * @param variable_count Number of variables
     * @return Gate instantiation code
     */
    static string generate_gate_logic(
        const vector<Implicant>& solution,
        int variable_count
    );

private:
    /**
     * Generates the module header with port declarations
     */
    static string generate_module_header(
        int variable_count,
        const string& module_name
    );

    /**
     * Generates wire declarations for intermediate signals
     */
    static string generate_wire_declarations(
        const vector<Implicant>& solution,
        int variable_count,
        int& wire_count
    );

    /**
     * Generates NOT gates for complemented inputs
     */
    static string generate_input_inverters(
        int variable_count,
        const vector<Implicant>& solution
    );

    /**
     * Generates AND gates for product terms
     */
    static string generate_product_terms(
        const vector<Implicant>& solution,
        int variable_count,
        int& wire_count
    );

    /**
     * Generates final OR gate combining all product terms
     */
    static string generate_sum_of_products(
        const vector<Implicant>& solution,
        int product_wire_start
    );

    /**
     * Gets the variable name for a given index (A, B, C, ...)
     */
    static string get_variable_name(int index);

    /**
     * Checks which variables need to be complemented in the solution
     */
    static vector<bool> get_complemented_variables(
        const vector<Implicant>& solution,
        int variable_count
    );
};

#endif // QUINE_MINIMIZER_VERILOG_GENERATOR_H