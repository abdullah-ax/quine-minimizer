# Test Files Analysis for Quine-Minimizer

This document explains the expected behavior and potential issues for each test file in the tests folder.

## Test File Format
Each test file follows this structure:
- Line 1: Number of variables
- Line 2: Minterms (m prefix)
- Line 3: Don't cares (d prefix)
- Line 4: (Optional) Empty line

---

## Test Results Summary

**Total Tests: 10**
- ✅ **Expected to PASS: 7 tests**
- ❌ **Expected to FAIL: 3 tests** (test6.txt, test8.txt, test10.txt)

---

## test1.txt ✅ PASS
```
3 variables
Minterms: m1, m2, m6
Don't cares: d0, d5
```

**Expected Return:** Minimized Boolean expression for a 3-variable function (A, B, C) with minterms 1, 2, 6 and don't cares 0, 5.

**Status:** Valid test case with proper syntax and bounds.

---

## test2.txt ✅ PASS
```
3 variables
Minterms: m0, m1, m3
Don't cares: d (incomplete)
```

**Expected Return:** Minimized Boolean expression for a 3-variable function with minterms 0, 1, 3.

**Notes:**
- Line 3 contains only "d" without numbers
- Parser likely treats this as empty/no don't cares
- Successfully processes the valid minterms

---

## test3.txt ✅ PASS
```
4 variables
Minterms: m1, m4, m7, m10, m13
Don't cares: d0, d15
```

**Expected Return:** Minimized Boolean expression for a 4-variable function (A, B, C, D) with minterms 1, 4, 7, 10, 13 and don't cares 0, 15.

**Status:** Valid test case with proper syntax and bounds.

---

## test4.txt ✅ PASS
```
4 variables
Minterms: M2, M5, M8, M11, M14
Don't cares: d3, d12
```

**Expected Return:** Minimized Boolean expression for a 4-variable function with minterms 2, 5, 8, 11, 14 and don't cares 3, 12.

**Notes:**
- Uses uppercase 'M' instead of lowercase 'm' for minterms
- Parser is case-insensitive or handles both cases
- All values within valid bounds

---

## test5.txt ✅ PASS
```
5 variables
Minterms: m3, m7
Don't cares: d0, d1, d2, d4, d5, d6, d8, d9, d10, d11, d12, d13, d14, d15, d16, d17, d18, d19, d20, d21, d22, d23, d24, d25, d26, d27, d28, d29, d30, d31
```

**Expected Return:** Minimized Boolean expression for a 5-variable function with only minterms 3 and 7, but with extensive don't cares (30 out of 32 possible terms).

**Status:** Valid edge case test - almost all terms are don't cares (30/32), which should result in a very simple minimized expression.

---

## test6.txt ❌ FAIL
```
5 variables
Minterms: m1, m8, m13, m20, m21, m32
Don't cares: d (incomplete)
```

**Expected Return:** Should fail due to out-of-bounds minterm.

**Issues Identified:**
- ❌ **m32 is invalid for 5 variables** - With 5 variables, valid minterm range is 0-31 (2^5 = 32). m32 is out of bounds!
- Line 3 contains only "d" (treated as empty don't cares)
- **This test is designed to fail** due to the out-of-bounds minterm

---

## test7.txt ✅ PASS
```
6 variables
Minterms: m0, m2, m5, m31, m45, m63
Don't cares: d65
```

**Expected Return:** Minimized Boolean expression for a 6-variable function with minterms 0, 2, 5, 31, 45, 63.

**Notes:**
- d65 is technically out of bounds for 6 variables (valid range: 0-63)
- The out-of-bounds don't care is ignored or doesn't affect minimization
- All minterms are within valid bounds
- **Test passes** because invalid don't cares can be safely ignored

---

## test8.txt ❌ FAIL
```
6 variables
Minterms: m (incomplete)
Don't cares: d0 through d63 (all 64 possible terms)
```

**Expected Return:** Should fail due to malformed/empty minterms.

**Issues Identified:**
- ❌ **Line 2 contains only "m" without any numbers** - No valid minterms specified
- A Boolean function must have at least one minterm to minimize
- **This test is designed to fail** due to empty/malformed minterms

---

## test9.txt ✅ PASS
```
7 variables
Minterms: m3, m5, m10, m12, m15, m20, m25, m30, m35, m40, m45, m50, m55, m60, m64, m70, m80, m90, m95, m100
Don't cares: d6, d13, d22, d33, d47, d59, d77, d88, d99
```

**Expected Return:** Minimized Boolean expression for a 7-variable function with the specified minterms and don't cares.

**Status:** Valid test case with 7 variables (range 0-127). All terms are within valid bounds.

---

## test10.txt ❌ FAIL
```
5 variables
Minterms: m0 through m31 (all 32 possible minterms)
Don't cares: d0, d1
```

**Expected Return:** Should fail or handle ambiguity.

**Issues Identified:**
- ❌ **Logical conflict: d0 and d1 are listed as don't cares, but they're also minterms (m0, m1)**
- A term cannot be both a minterm (must be 1) and a don't care (can be 0 or 1) simultaneously
- **This test is designed to fail** due to overlapping term definitions

---

## Summary of Issues

| Test File | Status | Description |
|-----------|--------|-------------|
| test1.txt | ✅ PASS | Valid 3-variable minimization |
| test2.txt | ✅ PASS | Valid, empty don't cares accepted |
| test3.txt | ✅ PASS | Valid 4-variable minimization |
| test4.txt | ✅ PASS | Valid, case-insensitive parsing |
| test5.txt | ✅ PASS | Valid, extensive don't cares (edge case) |
| test6.txt | ❌ FAIL | Out of bounds minterm (m32 for 5 vars) |
| test7.txt | ✅ PASS | Valid, out-of-bounds don't care ignored |
| test8.txt | ❌ FAIL | No valid minterms specified |
| test9.txt | ✅ PASS | Valid 7-variable minimization |
| test10.txt | ❌ FAIL | Overlapping minterms and don't cares |

---

## Pass/Fail Analysis

### ✅ Tests That Should Pass (7 tests)
1. **test1.txt** - Standard valid input
2. **test2.txt** - Empty don't cares handled gracefully
3. **test3.txt** - Standard valid input
4. **test4.txt** - Case-insensitive parsing works
5. **test5.txt** - Edge case with many don't cares
6. **test7.txt** - Out-of-bounds don't cares safely ignored
7. **test9.txt** - Larger variable count (7 variables)

### ❌ Tests That Should Fail (3 tests)
1. **test6.txt** - Out-of-bounds minterm m32 (valid range: 0-31)
2. **test8.txt** - Missing/malformed minterms (only "m")
3. **test10.txt** - Ambiguous overlap between minterms and don't cares
