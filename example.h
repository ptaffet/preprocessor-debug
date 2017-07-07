#define CONCAT(a, b) a b
#define PRINT(x) printf(CONCAT(x, "\n"))

const char* dummy = CONCAT("Part A", " Part B");
