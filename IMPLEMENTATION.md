# Quine-McCluskey Implementation Guide

## Team Member A (abdullah-ax): quine.h + Implicant Methods + Parsing

### Step 1: Implement Implicant::as_binary_string (5 minutes)
**Location:** `src/quine.cpp` lines 10-23

**What it does:** Converts an implicant to binary string with dashes for don't-cares

**Instructions:**
1. Create empty string called `result`
2. Loop from MSB to LSB (i = variable_count - 1 down to 0):
    - Check if bit i in `dont_care_mask` is 1
    - If yes: append '-' to result
    - If no: check if bit i in `binary_value` is 1
        - Append '1' if true, '0' if false
3. Return result

**Test:** Create implicant with binary_value=5 (101), dont_care_mask=2 (010), call as_binary_string(3)
Expected output: "1-1"

---

### Step 2: Implement Implicant::as_boolean_expression (10 minutes)
**Location:** `src/quine.cpp` lines 25-46

**What it does:** Converts binary implicant to Boolean expression like "AB'C"

**Instructions:**
1. Create empty string called `expression`
2. Loop through each variable (i = 0 to variable_count - 1):
    - Calculate bit_position = variable_count - 1 - i
    - Check if bit at bit_position in dont_care_mask is 1
    - If yes: skip this variable (continue to next)
    - If no:
        - Calculate variable_name = 'A' + i
        - Check if bit at bit_position in binary_value is 0
        - Append variable_name to expression
        - If bit was 0, append '\'' (prime symbol)
3. If expression is empty, return "1"
4. Otherwise return expression

**Test:** binary_value=6 (110), dont_care_mask=0, variable_count=3
Expected: "AB" (because bits are 110, so A=1, B=1, C=0 → ABC')

---

### Step 3: Implement Implicant::operator< (3 minutes)
**Location:** `src/quine.cpp` lines 48-53

**What it does:** Comparison operator for sorting implicants

**Instructions:**
1. Compare dont_care_mask values:
    - If different, return (dont_care_mask < other.dont_care_mask)
2. If masks are same, compare binary_value:
    - Return (binary_value < other.binary_value)

**Test:** Create two implicants, use sort() on vector of them

---

### Step 4: Implement split_by_comma helper (5 minutes)
**Location:** `src/quine.cpp` lines 57-68

**What it does:** Splits "m1,m3,m5" into ["m1", "m3", "m5"]

**Instructions:**
1. Create empty vector<string> called `tokens`
2. Create stringstream from input text
3. Create string variable called `token`
4. Loop: while getline(stream, token, ',') succeeds:
    - If token is not empty, push it to tokens
5. Return tokens

**Test:** split_by_comma("m1,m3,m5") should return 3-element vector

---

### Step 5: Implement trim_whitespace helper (5 minutes)
**Location:** `src/quine.cpp` lines 70-78

**What it does:** Removes leading and trailing spaces from string

**Instructions:**
1. While text is not empty AND first character is whitespace:
    - Erase first character (text.erase(text.begin()))
2. While text is not empty AND last character is whitespace:
    - Remove last character (text.pop_back())
3. Return text

**Test:** trim_whitespace("  hello  ") should return "hello"

---

### Step 6: Implement load_from_file - Part 1: Basic File Reading (10 minutes)
**Location:** `src/quine.cpp` lines 100-118

**What it does:** Opens file and reads first 3 lines

**Instructions:**
1. Create ifstream called input_file with file_path
2. If file didn't open, return false
3. Create three strings: first_line, second_line, third_line
4. Read first_line using getline
    - If it fails, return false
5. Read second_line using getline
    - If it fails, set second_line = ""
6. Read third_line using getline
    - If it fails, set third_line = ""
7. Try to convert first_line to integer using stoi, store in variable_count
    - Wrap in try-catch, return false if exception occurs
8. If variable_count <= 0 OR variable_count > 20, return false
9. Trim second_line and third_line using trim_whitespace

**Test:** Create test file with 3 lines, verify it reads correctly

---

### Step 7: Implement load_from_file - Part 2: Parse Don't-Cares (10 minutes)
**Location:** `src/quine.cpp` lines 120-130

**What it does:** Parses third line like "d0,d5" into don't-care terms

**Instructions:**
1. Clear function_dont_cares vector
2. If third_line is not empty:
    - Split third_line by comma
    - For each token:
        - Trim whitespace from token
        - If token size < 2, return false
        - If first character is not 'd' or 'D', return false
        - Extract substring from position 1 to end
        - Convert to integer using stoi
        - Add to function_dont_cares vector
3. Continue to next part

**Test:** "d0,d5" should populate function_dont_cares with [0, 5]

---

### Step 8: Implement load_from_file - Part 3: Parse Minterms (15 minutes)
**Location:** `src/quine.cpp` lines 132-167

**What it does:** Parses "m1,m3,m7" or "M0,M2" into minterms

**Instructions:**
1. Clear function_minterms vector
2. If second_line is empty, return false
3. Get first character, store in notation_type
4. If notation_type is not 'm' and not 'M', return false
5. Set is_maxterm_notation = true if notation_type is 'M', false otherwise
6. Create empty vector<int> called listed_terms
7. Split second_line by comma
8. For each token:
    - Trim whitespace
    - If size < 2, return false
    - Extract substring from position 1 to end
    - Convert to integer, add to listed_terms
9. If is_maxterm_notation is true:
    - Calculate total_combinations = 1 << variable_count
    - Create set from listed_terms (call it maxterm_set)
    - Create set from function_dont_cares (call it dont_care_set)
    - Loop i from 0 to total_combinations - 1:
        - If i is in maxterm_set OR i is in dont_care_set, skip it
        - Otherwise, add i to function_minterms
10. If is_maxterm_notation is false:
    - Set function_minterms = listed_terms

**Test:** "m1,m3,m5" should give function_minterms = [1,3,5]
**Test:** "M0,M2" with 2 variables should give function_minterms = [1,3]

---

### Step 9: Implement load_from_file - Part 4: Validation (5 minutes)
**Location:** `src/quine.cpp` lines 169-175

**What it does:** Ensures no overlap between minterms and don't-cares

**Instructions:**
1. For each dont_care value in function_dont_cares:
    - Search for dont_care in function_minterms using find()
    - If found (iterator != end()), return false
2. If all checks pass, return true

**Test:** Minterms [1,3,5] with don't-cares [0,2] should pass
**Test:** Minterms [1,3,5] with don't-cares [3] should fail

---

## Team Member B: Algorithm Implementation (quine.cpp core logic)

### Step 10: Implement can_combine_implicants helper (15 minutes)
**Location:** `src/quine.cpp` lines 80-98

**What it does:** Checks if two implicants differ by exactly one bit and combines them

**Instructions:**
1. If dont_care_masks are different, return false
2. Calculate bit_difference = (first.binary_value XOR second.binary_value) AND NOT(first.dont_care_mask)
3. If bit_difference is 0, return false (they're identical)
4. Check if bit_difference has exactly one bit set:
    - Use trick: (bit_difference & (bit_difference - 1)) should be 0
    - If not 0, return false
5. Set combined = first
6. Clear the differing bit: combined.binary_value &= ~bit_difference
7. Mark differing bit as don't-care: combined.dont_care_mask |= bit_difference
8. Clear combined.covered_minterms
9. Insert all minterms from first.covered_minterms into combined.covered_minterms
10. Insert all minterms from second.covered_minterms into combined.covered_minterms
11. Return true

**Test:** Implicants 001 and 011 should combine to 0-1

---

### Step 11: Implement create_initial_implicants (10 minutes)
**Location:** `src/quine.cpp` lines 179-195

**What it does:** Creates one implicant for each minterm and don't-care

**Instructions:**
1. Create empty vector<Implicant> called initial_list
2. Create lambda function add_term(int term_value):
    - Create new Implicant called imp
    - Set imp.binary_value = term_value (cast to uint64_t)
    - Set imp.dont_care_mask = 0
    - Insert term_value into imp.covered_minterms
    - Push imp to initial_list
3. Loop through function_minterms, call add_term for each
4. Loop through function_dont_cares, call add_term for each
5. Return initial_list

**Test:** Minterms [1,3] should create 2 implicants with binary_values 1 and 3

---

### Step 12: Implement find_all_prime_implicants - Part 1: Setup (10 minutes)
**Location:** `src/quine.cpp` lines 197-209

**What it does:** Sets up the iterative combining loop

**Instructions:**
1. Call create_initial_implicants(), store result in current_level
2. Create empty vector<Implicant> called prime_implicants
3. Start while loop: while current_level is not empty
4. Inside loop: Sort current_level by number of 1s in binary_value (excluding masked bits):
    - Use lambda comparator:
        - Calculate ones_a = count_ones_in_value(a.binary_value, a.dont_care_mask)
        - Calculate ones_b = count_ones_in_value(b.binary_value, b.dont_care_mask)
        - If ones_a != ones_b, return ones_a < ones_b
        - If dont_care_masks differ, return a.dont_care_mask < b.dont_care_mask
        - Otherwise return a.binary_value < b.binary_value

**Test:** After sorting, implicants should be grouped by number of 1-bits

---

### Step 13: Implement find_all_prime_implicants - Part 2: Combining (15 minutes)
**Location:** `src/quine.cpp` lines 211-234

**What it does:** Combines implicants that differ by one bit

**Instructions:**
1. Create vector<bool> called was_combined, size = current_level.size(), all false
2. Create empty vector<Implicant> called next_level
3. Nested loop: for i from 0 to current_level.size() - 1:
    - For j from i + 1 to current_level.size() - 1:
        - Create Implicant called combined
        - Call can_combine_implicants(current_level[i], current_level[j], combined)
        - If it returns true:
            - Set was_combined[i] = true
            - Set was_combined[j] = true
            - Check if combined already exists in next_level (use implicants_are_equal)
            - If not duplicate, push combined to next_level
4. After nested loop: for i from 0 to current_level.size() - 1:
    - If was_combined[i] is false:
        - Push current_level[i] to prime_implicants (it can't be combined further)
5. Set current_level = next_level

**Test:** Implicants [001, 011, 101, 111] should combine to [0-1, 1-1]

---

### Step 14: Implement find_all_prime_implicants - Part 3: Cleanup (5 minutes)
**Location:** `src/quine.cpp` lines 236-243

**What it does:** Removes duplicates from prime implicants

**Instructions:**
1. After while loop ends:
2. Sort prime_implicants
3. Use unique() with implicants_are_equal comparator to remove duplicates
4. Erase the duplicates (from unique()'s return value to end())
5. Return prime_implicants

**Test:** Verify no duplicate implicants in final result

---

### Step 15: Implement extract_essential_prime_implicants - Part 1: Find EPIs (15 minutes)
**Location:** `src/quine.cpp` lines 245-272

**What it does:** Finds prime implicants that are the ONLY cover for some minterm

**Instructions:**
1. Create empty vector<Implicant> called essentials
2. Create vector<bool> minterm_is_covered(function_minterms.size(), all false)
3. Create vector<bool> pi_is_essential(prime_implicants.size(), all false)
4. Loop minterm_idx from 0 to function_minterms.size() - 1:
    - Get minterm = function_minterms[minterm_idx]
    - Create empty vector<int> called covering_pis
    - Loop pi_idx from 0 to prime_implicants.size() - 1:
        - If prime_implicants[pi_idx].covered_minterms contains minterm:
            - Add pi_idx to covering_pis
    - If covering_pis has exactly 1 element:
        - Get essential_pi_index = covering_pis[0]
        - If pi_is_essential[essential_pi_index] is false:
            - Set pi_is_essential[essential_pi_index] = true
            - Add prime_implicants[essential_pi_index] to essentials

**Test:** If minterm 5 is only covered by PI #2, then PI #2 is essential

---

### Step 16: Implement extract_essential_prime_implicants - Part 2: Mark Covered (10 minutes)
**Location:** `src/quine.cpp` lines 274-289

**What it does:** Marks which minterms are covered by essential PIs

**Instructions:**
1. Loop minterm_idx from 0 to function_minterms.size() - 1:
    - Get minterm = function_minterms[minterm_idx]
    - Loop through each essential in essentials:
        - If essential.covered_minterms contains minterm:
            - Set minterm_is_covered[minterm_idx] = true
            - Break out of inner loop
2. Clear uncovered_minterms parameter
3. Loop i from 0 to function_minterms.size() - 1:
    - If minterm_is_covered[i] is false:
        - Add function_minterms[i] to uncovered_minterms
4. Return essentials

**Test:** If EPIs cover minterms [1,3,5], uncovered should contain remaining minterms

---

### Step 17: Implement find_minimal_covers - Part 1: Setup (10 minutes)
**Location:** `src/quine.cpp` lines 291-318

**What it does:** Prepares for brute-force search of minimal cover

**Instructions:**
1. If uncovered_minterms is empty:
    - Return vector containing just essentials
2. Create empty vector<int> called remaining_pi_indices
3. Create empty set<int> called essential_pi_values
4. Loop through essentials:
    - Add all covered_minterms to essential_pi_values (not needed, but good for tracking)
5. Loop i from 0 to prime_implicants.size() - 1:
    - Check if prime_implicants[i] equals any essential (use implicants_are_equal)
    - If yes, skip it (continue)
    - Check if prime_implicants[i] covers any uncovered_minterm
    - If yes, add i to remaining_pi_indices
6. Set max_subset_size = 6
7. Create empty vector<vector<int>> called minimal_combinations
8. Set total_remaining = remaining_pi_indices.size()

**Test:** If 3 PIs are essential and 5 are not, remaining_pi_indices should have up to 5 elements

---

### Step 18: Implement find_minimal_covers - Part 2: Brute Force Search (20 minutes)
**Location:** `src/quine.cpp` lines 320-353

**What it does:** Tries all subsets of size 1, 2, 3... to find minimal cover

**Instructions:**
1. Loop subset_size from 1 to min(max_subset_size, total_remaining):
    - Create vector<int> current_combination(subset_size)
    - Create recursive lambda function explore_combinations(int start_pos, int depth):
        - If depth == subset_size:
            - Create empty set<int> called covered_set
            - Loop through each idx in current_combination:
                - Get pi_index = remaining_pi_indices[idx]
                - Loop through each minterm in uncovered_minterms:
                    - If prime_implicants[pi_index].covered_minterms contains minterm:
                        - Add minterm to covered_set
            - If covered_set.size() equals uncovered_minterms.size():
                - Push current_combination to minimal_combinations
            - Return from function
        - Loop i from start_pos to (total_remaining - (subset_size - depth)):
            - Set current_combination[depth] = i
            - Recursively call explore_combinations(i + 1, depth + 1)
    - Call explore_combinations(0, 0)
    - If minimal_combinations is not empty, break (found minimal size)

**Test:** For uncovered minterms [2,6], try all PI combinations until cover is found

---

### Step 19: Implement find_minimal_covers - Part 3: Build Solutions (10 minutes)
**Location:** `src/quine.cpp` lines 355-367

**What it does:** Combines essentials with each minimal combination to form complete solutions

**Instructions:**
1. Create empty vector<vector<Implicant>> called all_solutions
2. Loop through each combination in minimal_combinations:
    - Create vector<Implicant> called solution, initialize with essentials
    - Loop through each idx in combination:
        - Get pi_index = remaining_pi_indices[idx]
        - Add prime_implicants[pi_index] to solution
    - Add solution to all_solutions
3. Return all_solutions

**Test:** If 2 EPIs + 1 additional PI covers everything, solution should have 3 implicants

---

### Step 20: Implement minimize (5 minutes)
**Location:** `src/quine.cpp` lines 369-386

**What it does:** Main driver function that calls all algorithm steps

**Instructions:**
1. Create MinimizationResult called result
2. Call find_all_prime_implicants(), store in result.all_prime_implicants
3. Create empty vector<int> called uncovered
4. Call extract_essential_prime_implicants(result.all_prime_implicants, uncovered)
    - Store return value in result.essential_prime_implicants
5. Set result.minterms_not_covered_by_essentials = uncovered
6. Call find_minimal_covers with all required parameters
    - Store return value in result.all_minimal_solutions
7. Return result

**Test:** Run on test1.txt and verify complete output

---

## Team Member A: main.cpp Implementation

### Step 21: Implement print_term_list (3 minutes)
**Location:** `src/main.cpp` lines 7-12

**What it does:** Prints comma-separated list like "1,3,5,7"

**Instructions:**
1. Loop i from 0 to terms.size() - 1:
    - If i > 0, print comma
    - Print terms[i]
2. No newline at end

**Test:** [1,3,5] should print "1,3,5"

---

### Step 22: Implement print_prime_implicants (10 minutes)
**Location:** `src/main.cpp` lines 14-26

**What it does:** Prints all PIs with their binary form and covered minterms

**Instructions:**
1. Print header: "Prime Implicants (" + size + "):\n"
2. Loop through each pi in pis:
    - Print "  " + pi.as_binary_string(variable_count) + "  covers {"
    - Set first = true
    - Loop through each minterm in pi.covered_minterms:
        - If not first, print comma
        - Print minterm
        - Set first = false
    - Print "}\n"

**Test:** Should show something like "  1-1  covers {5,7}"

---

### Step 23: Implement print_essential_prime_implicants (8 minutes)
**Location:** `src/main.cpp` lines 28-35

**What it does:** Prints EPIs with binary and Boolean expression

**Instructions:**
1. Print header: "\nEssential Prime Implicants (" + size + "):\n"
2. Loop through each epi in epis:
    - Print "  " + epi.as_binary_string(variable_count)
    - Print "  => " + epi.as_boolean_expression(variable_count) + "\n"

**Test:** Should show "  1-1  => AC"

---

### Step 24: Implement print_uncovered_minterms (8 minutes)
**Location:** `src/main.cpp` lines 37-46

**What it does:** Shows which minterms still need covering

**Instructions:**
1. Print "\nMinterms not covered by EPIs: "
2. If uncovered is empty:
    - Print "None\n"
3. Else:
    - Call print_term_list(uncovered)
    - Print "\n"

**Test:** Empty list should print "None", [2,6] should print "2,6"

---

### Step 25: Implement print_minimal_solutions (12 minutes)
**Location:** `src/main.cpp` lines 48-68

**What it does:** Prints all minimal Boolean expressions found

**Instructions:**
1. Print "\nMinimal Boolean Expressions:\n"
2. If solutions is empty:
    - Print "  (No solution found within subset limits)\n"
    - Return
3. Set solution_number = 1
4. Loop through each solution in solutions:
    - Print "  Solution " + solution_number++ + ": "
    - Set first_term = true
    - Loop through each implicant in solution:
        - If not first_term, print " + "
        - Print implicant.as_boolean_expression(variable_count)
        - Set first_term = false
    - Print "\n"

**Test:** Should show "Solution 1: AB + CD'"

---

### Step 26: Implement main function (15 minutes)
**Location:** `src/main.cpp` lines 70-106

**What it does:** Orchestrates entire program flow

**Instructions:**
1. Check if argc != 2:
    - Print usage message to cerr
    - Return 1
2. Create QuineMcCluskey object called minimizer
3. Call minimizer.load_from_file(argv[1])
    - If returns false:
        - Print error message to cerr
        - Return 2
4. Print header: "=== Quine-McCluskey Logic Minimization ===\n\n"
5. Print "Number of Variables: " + minimizer.variable_count
6. Print "Minterms: " then call print_term_list(minimizer.function_minterms)
7. Print "\nDon't-Cares: "
    - If function_dont_cares is empty, print "None"
    - Else call print_term_list(minimizer.function_dont_cares)
8. Print "\n\n"
9. Call minimizer.minimize(), store in result
10. Call all print functions with result data
11. Return 0

**Test:** Run ./quine tests/test1.txt and verify complete formatted output

---

## Testing Checklist

After implementing each section, test with:

### Test 1 (test1.txt):
```
3
m1,m3,m6,m7
d0,d5
```
Expected: Should find PIs and minimal expression

### Test 2 (test2.txt):
```
4
m0,m1,m2,m5,m6,m7,m8,m9
d3,d4
```
Expected: More complex minimization

### Test 3 (test3.txt):
```
3
M0,M2
d1
```
Expected: Maxterm notation should work

---

## Integration Steps

1. **Member A** completes Steps 1-9 (Implicant methods + parsing)
2. **Member B** completes Steps 10-20 (core algorithm)
3. **Member A** completes Steps 21-26 (main.cpp)
4. Both members: Test with all 10 test cases
5. Both members: Review code quality, add final comments if needed
6. Both members: Generate report and prepare demo

---

## Debugging Tips

- Print intermediate results after each major step
- Use small test cases (2-3 variables) first
- Verify binary representations are correct
- Check that covered_minterms sets are populated correctly
- Ensure don't-care mask is working properly (use bitwise AND/OR correctly)

---

## Estimated Time Breakdown

**Member A Total:** ~90 minutes
- Steps 1-9: ~70 minutes (quine.cpp parsing + Implicant)
- Steps 21-26: ~60 minutes (main.cpp)

**Member B Total:** ~110 minutes
- Steps 10-20: ~110 minutes (core algorithm)

**Total Team Time:** ~3-4 hours of focused implementation