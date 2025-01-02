#pragma once

// Utilities
#define LU_EXPAND_MACRO(x) x
#define LU_CONCAT(prefix, suffix) prefix suffix

#define LU_NUMBER_OF_ARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
#define LU_NUMBER_OF_ARGS(...) LU_EXPAND_MACRO(LU_NUMBER_OF_ARGS_IMPL(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define LU_EXPAND_MACRO_COUNT_IMPL(macro, count, ...) macro##count(__VA_ARGS__)
#define LU_EXPAND_MACRO_COUNT(macro, count, ...) LU_EXPAND_MACRO_COUNT_IMPL(macro, count, __VA_ARGS__)