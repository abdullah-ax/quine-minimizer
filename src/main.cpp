//
// Created by Abdullah Ahmed on 06/11/2025.
//

#include "quine.h"
#include "verilog_generator.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <limits>

using namespace std;
namespace fs = std::filesystem;

// ==================== Output Formatting Functions ====================

void print_separator(char c = '=', int width = 70) {
    cout << string(width, c) << "\n";
}

void print_header(const string& title) {
    print_separator();
    cout << "  " << title << "\n";
    print_separator();
}

void print_section(const string& title) {
    cout << "\n" << title << "\n";
    cout << string(title.length(), '-') << "\n";
}

void print_term_list(const vector<int>& terms) {
    if (terms.empty()) {
        cout << "None";
        return;
    }

    for (size_t i = 0; i < terms.size(); ++i) {
        if (i > 0) cout << ", ";
        cout << terms[i];
    }
}

void print_input_summary(const QuineMcCluskey& qm) {
    print_section("Input Summary");

    cout << "Variables: " << qm.variable_count << "\n";
    cout << "Minterms:  ";
    print_term_list(qm.function_minterms);
    cout << "\nDon't-Cares: ";
    print_term_list(qm.function_dont_cares);
    cout << "\n";
}

void print_prime_implicants(const vector<Implicant>& pis, int variable_count) {
    print_section("Prime Implicants");

    if (pis.empty()) {
        cout << "No prime implicants found.\n";
        return;
    }

    cout << "Total: " << pis.size() << "\n\n";
    cout << left << setw(15) << "Binary" << setw(20) << "Expression" << "Covers\n";
    cout << string(60, '-') << "\n";

    for (const auto& pi : pis) {
        cout << left << setw(15) << pi.as_binary_string(variable_count)
             << setw(20) << pi.as_boolean_expression(variable_count)
             << "{";

        bool first = true;
        for (int minterm : pi.covered_minterms) {
            if (!first) cout << ", ";
            cout << minterm;
            first = false;
        }
        cout << "}\n";
    }
}

void print_essential_prime_implicants(const vector<Implicant>& epis, int variable_count) {
    print_section("Essential Prime Implicants");

    if (epis.empty()) {
        cout << "No essential prime implicants.\n";
        return;
    }

    cout << "Total: " << epis.size() << "\n\n";
    cout << left << setw(15) << "Binary" << "Expression\n";
    cout << string(40, '-') << "\n";

    for (const auto& epi : epis) {
        cout << left << setw(15) << epi.as_binary_string(variable_count)
             << epi.as_boolean_expression(variable_count) << "\n";
    }
}

void print_uncovered_minterms(const vector<int>& uncovered) {
    print_section("Uncovered Minterms (after EPIs)");

    if (uncovered.empty()) {
        cout << "All minterms covered by essential prime implicants.\n";
    } else {
        cout << "Minterms still needing coverage: ";
        print_term_list(uncovered);
        cout << "\n";
    }
}

void print_minimal_solutions(const vector<vector<Implicant>>& solutions, int variable_count) {
    print_section("Minimal Boolean Expressions");

    if (solutions.empty()) {
        cout << "No solution found (search space too large or no valid cover exists).\n";
        return;
    }

    cout << "Found " << solutions.size() << " minimal solution(s):\n\n";

    int solution_number = 1;
    for (const auto& solution : solutions) {
        cout << "Solution " << solution_number++ << ": ";

        bool first = true;
        for (const auto& implicant : solution) {
            if (!first) cout << " + ";
            cout << implicant.as_boolean_expression(variable_count);
            first = false;
        }

        cout << "  (uses " << solution.size() << " term";
        if (solution.size() != 1) cout << "s";
        cout << ")\n";
    }
}

void print_statistics(const MinimizationResult& result) {
    print_section("Statistics");

    cout << "Prime Implicants: " << result.all_prime_implicants.size() << "\n";
    cout << "Essential PIs: " << result.essential_prime_implicants.size() << "\n";
    cout << "Minimal Solutions: " << result.all_minimal_solutions.size() << "\n";

    if (!result.all_minimal_solutions.empty()) {
        cout << "Terms in minimal form: " << result.all_minimal_solutions[0].size() << "\n";
    }
}

void print_verilog_prompt() {
    cout << "\n";
    print_separator('-', 70);
    cout << "  Verilog Generation (Bonus Feature)\n";
    print_separator('-', 70);
    cout << "\nWould you like to generate a Verilog module for this function?\n";
    cout << "  [y] Yes - Generate Verilog module\n";
    cout << "  [n] No - Skip Verilog generation\n";
    cout << "\nYour choice: ";
}

bool prompt_verilog_generation(
    const MinimizationResult& result,
    const QuineMcCluskey& minimizer,
    const string& test_name) {

    if (result.all_minimal_solutions.empty()) {
        cout << "\nVerilog generation skipped: No minimal solution available.\n";
        return false;
    }

    print_verilog_prompt();

    string input;
    getline(cin, input);

    if (input.empty()) input = "n";
    char choice = tolower(input[0]);

    if (choice != 'y') {
        cout << "\nVerilog generation skipped.\n";
        return false;
    }

    // Select which solution to use if multiple exist
    size_t solution_index = 0;
    if (result.all_minimal_solutions.size() > 1) {
        cout << "\nMultiple minimal solutions available ("
             << result.all_minimal_solutions.size() << " total).\n";
        cout << "Generate Verilog for solution number (1-"
             << result.all_minimal_solutions.size() << ", default 1): ";
        string sol_input;
        getline(cin, sol_input);
        if (!sol_input.empty()) {
            try {
                int sol_num = stoi(sol_input);
                if (sol_num >= 1 && sol_num <= (int)result.all_minimal_solutions.size()) {
                    solution_index = sol_num - 1;
                }
            } catch (...) {
                // Use default
            }
        }
    }

    const auto& solution = result.all_minimal_solutions[solution_index];

    // Generate module name from test name
    string module_name = "boolean_function";
    if (!test_name.empty()) {
        module_name = test_name;
        // Remove file extension
        size_t dot_pos = module_name.find_last_of('.');
        if (dot_pos != string::npos) {
            module_name = module_name.substr(0, dot_pos);
        }
        // Remove path
        size_t slash_pos = module_name.find_last_of("/\\");
        if (slash_pos != string::npos) {
            module_name = module_name.substr(slash_pos + 1);
        }
    }

    // Generate Verilog
    string verilog_code = VerilogGenerator::generate_module(
        solution,
        minimizer.variable_count,
        module_name
    );

    // Display generated Verilog
    cout << "\n";
    print_separator('=', 70);
    cout << "  Generated Verilog Module\n";
    print_separator('=', 70);
    cout << "\n" << verilog_code << "\n";

    // Offer to save to file
    cout << "Save to file? [y/n]: ";
    getline(cin, input);
    if (!input.empty() && tolower(input[0]) == 'y') {
        string filename = module_name + ".v";
        cout << "Enter filename (default: " << filename << "): ";
        string user_filename;
        getline(cin, user_filename);
        if (!user_filename.empty()) {
            filename = user_filename;
            // Add .v extension if not present
            if (filename.find(".v") == string::npos) {
                filename += ".v";
            }
        }

        ofstream verilog_file(filename);
        if (verilog_file) {
            verilog_file << verilog_code;
            verilog_file.close();
            cout << "\nVerilog module saved to: " << filename << "\n";
        } else {
            cerr << "\nERROR: Could not write to file: " << filename << "\n";
        }
    }

    return true;
}

// ==================== Path Resolution ====================

string find_tests_directory() {
    vector<string> possible_paths = {
        "tests",
        "../tests",
        "../../tests",
        "../../../tests"
    };

    for (const auto& path : possible_paths) {
        if (fs::exists(path) && fs::is_directory(path)) {
            return path;
        }
    }

    return "";
}

// ==================== User Interaction ====================

bool prompt_continue(int current, int total) {
    cout << "\n";
    print_separator('-', 70);
    cout << "  Progress: " << current << " of " << total << " tests completed\n";
    print_separator('-', 70);
    cout << "\nOptions:\n";
    cout << "  [c] Continue to next test\n";
    cout << "  [q] Quit program\n";
    cout << "\nYour choice: ";

    string input;
    getline(cin, input);

    if (input.empty()) {
        input = "c";
    }

    char choice = tolower(input[0]);

    if (choice == 'q' || choice == 'e') {
        cout << "\nExiting. Tests completed: " << current << "/" << total << "\n";
        return false;
    }

    return true;
}

// ==================== Test Execution ====================

bool run_test(const string& test_file_path) {
    QuineMcCluskey minimizer;

    cout << "\n";
    print_header("Testing: " + fs::path(test_file_path).filename().string());

    if (!minimizer.load_from_file(test_file_path)) {
        cout << "\nERROR: Failed to load or parse input file.\n";
        cout << "Expected format:\n";
        cout << "  Line 1: Number of variables\n";
        cout << "  Line 2: Minterms (m0,m1,...) or Maxterms (M0,M1,...)\n";
        cout << "  Line 3: Don't-cares (d0,d1,...) [optional]\n";
        return false;
    }

    print_input_summary(minimizer);

    MinimizationResult result;
    try {
        result = minimizer.minimize();
    } catch (const exception& e) {
        cout << "\nERROR during minimization: " << e.what() << "\n";
        return false;
    }

    print_prime_implicants(result.all_prime_implicants, minimizer.variable_count);
    print_essential_prime_implicants(result.essential_prime_implicants, minimizer.variable_count);
    print_uncovered_minterms(result.minterms_not_covered_by_essentials);
    print_minimal_solutions(result.all_minimal_solutions, minimizer.variable_count);
    print_statistics(result);

    prompt_verilog_generation(result, minimizer, fs::path(test_file_path).filename().string());
    cout << "\nTest completed successfully.\n";
    return true;
}

// ==================== Main Entry Point ====================

int main(int argc, char** argv) {
    cout << "\n";
    print_separator('=', 70);
    cout << "  Quine-McCluskey Logic Minimizer\n";
    cout << "  CSCE2301 - Digital Design I - Fall 2025\n";
    cout << "  Authors: \n Abdullah Ahmed (@abdullah-ax)\n Sherifa Badra (@sherifabadra)\n";
    print_separator('=', 70);

    if (argc == 2) {
        string file_path = argv[1];
        if (!fs::exists(file_path)) {
            cerr << "\nERROR: File not found: " << file_path << "\n";
            return 1;
        }

        bool success = run_test(file_path);
        return success ? 0 : 1;
    }

    cout << "\nSearching for tests directory...\n";

    string tests_dir = find_tests_directory();
    if (tests_dir.empty()) {
        cerr << "\nERROR: Tests directory not found.\n";
        cerr << "Searched: ./tests, ../tests, ../../tests, ../../../tests\n";
        cerr << "\nUsage: " << argv[0] << " <test_file>\n";
        cerr << "   or: " << argv[0] << " (run all tests)\n";
        return 1;
    }

    cout << "Found: " << fs::absolute(tests_dir) << "\n";

    vector<string> test_files;
    for (const auto& entry : fs::directory_iterator(tests_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            test_files.push_back(entry.path().string());
        }
    }

    if (test_files.empty()) {
        cout << "\nNo test files found in tests directory.\n";
        return 0;
    }

    sort(test_files.begin(), test_files.end());

    cout << "Found " << test_files.size() << " test file(s).\n";

    int passed = 0;
    int failed = 0;
    int total = test_files.size();

    for (size_t i = 0; i < test_files.size(); ++i) {
        if (run_test(test_files[i])) {
            passed++;
        } else {
            failed++;
        }

        if (i < test_files.size() - 1) {
            if (!prompt_continue(i + 1, total)) {
                cout << "\n";
                print_separator('=', 70);
                cout << "  Partial Test Summary\n";
                print_separator('=', 70);
                cout << "Tests Run: " << (i + 1) << " of " << total << "\n";
                cout << "Passed: " << passed << "\n";
                if (failed > 0) {
                    cout << "Failed: " << failed << "\n";
                }
                cout << "Skipped: " << (total - i - 1) << "\n";
                print_separator('=', 70);
                return 0;
            }
        }
    }

    cout << "\n";
    print_separator('=', 70);
    cout << "  Test Summary\n";
    print_separator('=', 70);
    cout << "Total Tests: " << total << "\n";
    cout << "Passed: " << passed << "\n";
    if (failed > 0) {
        cout << "Failed: " << failed << "\n";
    }
    print_separator('=', 70);
    cout << "\nAll tests completed.\n\n";

    return failed == 0 ? 0 : 1;
}