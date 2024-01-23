#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <chrono>

const int NUM_CHARS = 256;  // number of characters in the input alphabet
const int PRIME_NUM = 101;  // a prime number for hash calculation

// Fills lps[] for given pattern pat[0..M-1]
void computeLPSArray(std::string pattern, std::vector<int>& lps);

void rand_string(std::string& str, size_t size, std::string pattern) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    if (size) {
        --size;  // reserve space for '\0'
        str = std::string(size, '\0');  // create the string with the required size
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (sizeof charset - 1);
            str[n] = charset[key];
        }
    }

    // Insert pattern at a random position
    if (!pattern.empty()) {
        size_t patternLength = pattern.size();
        if (patternLength < size) {
            size_t insertPos = rand() % (size - patternLength);
            memmove(&str[insertPos + patternLength], &str[insertPos], size - insertPos - patternLength + 1);
            memcpy(&str[insertPos], pattern.c_str(), patternLength);
        }
    }
}

void input_size_and_generate_string(std::string& str, size_t min_size, std::string pattern) {
    size_t size;
    do {
        std::cout << "Enter the size of the string (must be greater than " << min_size << "): ";
        std::cin >> size;
    } while (size <= min_size);

    str.resize(size + 1);
    rand_string(str, size, pattern);
}

// RK algorithm:
void search(std::string pattern, std::string text, int primeNum) {
    int patternLength = pattern.size();
    int textLength = text.size();
    int i, j;
    int patternHash = 0;  // hash value for pattern
    int textHash = 0;  // hash value for text
    int h = 1;

    // The value of h would be "pow(d, M-1)%q"
    for (auto i = 0; i < patternLength - 1; i++)
        h = (h * NUM_CHARS) % primeNum;

    // Calculate the hash value of pattern and first window of text
    for (i = 0; i < patternLength; i++) {
        patternHash = (NUM_CHARS * patternHash + pattern[i]) % primeNum;
        textHash = (NUM_CHARS * textHash + text[i]) % primeNum;
    }

    // Slide the pattern over text one by one
    for (i = 0; i <= textLength - patternLength; i++) {
        // Check the hash values of current window of text and pattern
        // If the hash values match then only check for characters one by one
        if (patternHash == textHash) {
            for (j = 0; j < patternLength; j++) {
                if (text[i + j] != pattern[j])
                    break;
            }

            // if p == t and pattern[0...M-1] = text[i, i+1, ...i+M-1]
            if (j == patternLength)
                std::cout << "Pattern found at index " << i << "\n";
        }

        // Calculate hash value for next window of text: Remove leading digit,
        // add trailing digit
        if (i < textLength - patternLength) {
            textHash = (NUM_CHARS * (textHash - text[i] * h) + text[i + patternLength]) % primeNum;

            // We might get negative value of t, converting it to positive
            if (textHash < 0)
                textHash = (textHash + primeNum);
        }
    }
}

// KMP algorithm:
void KMPSearch(std::string pattern, std::string text) {
    size_t patternLength = pattern.size();
    size_t textLength = text.size();

    // create lps[] that will hold the longest prefix suffix values for pattern
    std::vector<int> lps(patternLength);

    // Preprocess the pattern (calculate lps[] array)
    computeLPSArray(pattern, lps);

    int i = 0;  // index for text[]
    int j = 0;  // index for pattern[]
    while (i < textLength) {
        if (pattern[j] == text[i]) {
            j++;
            i++;
        }

        if (j == patternLength) {
            std::cout << "Pattern found at index " << i - j << "\n";
            j = lps[j - 1];
        }

            // mismatch after j matches
        else if (i < textLength && pattern[j] != text[i]) {
            // Do not match lps[0...lps[j-1]] characters, they will match anyway
            if (j != 0)
                j = lps[j - 1];
            else
                i = i + 1;
        }
    }
}

void KMPSearchWrapper(std::string pattern, std::string text, int primeNum) {
    (void)primeNum;  // Ignore primeNum
    KMPSearch(pattern, text);
}

void computeLPSArray(std::string pattern, std::vector<int>& lps) {
    // length of the previous longest prefix suffix
    int len = 0;

    lps[0] = 0;  // lps[0] is always 0

    // the loop calculates lps[i] for i = 1 to patternLength-1
    int i = 1;
    while (i < pattern.size()) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            // (pattern[i] != pattern[len])
            if (len != 0) {
                // This is tricky. Consider the example.
                // AAACAAAA and i = 7. The idea is similar to search step.
                len = lps[len - 1];
            } else {
                // if (len == 0)
                lps[i] = 0;
                i++;
            }
        }
    }
}

double measure_execution_time(void (*func)(std::string, std::string, int), std::string pattern, std::string text, int primeNum) {
    auto start = std::chrono::steady_clock::now();
    func(pattern, text, primeNum);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    return elapsed.count();
}

double measure_and_print_execution_time(std::string algorithm_name, void (*func)(std::string, std::string, int), std::string pattern, std::string text, int primeNum, int run_times) {
    std::vector<double> times(run_times);
    double total = 0.0;

    for(int i = 0; i < run_times; i++) {
        times[i] = measure_execution_time(func, pattern, text, primeNum);
        std::cout << "Run " << i+1 << ": " << algorithm_name << " took " << times[i] << " seconds to execute \n";
        total += times[i];
    }

    double average = total / run_times;
    std::cout << "Average " << algorithm_name << " time: " << average << " seconds \n";
    std::cout << "===========================================";
    std::cout << "\n";
    return average;
}

int main() {
    srand(time(0));  // initialize random seed

    std::string pattern;
    input_size_and_generate_string(pattern, 0, "");

    std::cout << "Generated pattern: " << pattern << "\n";

    std::string text;
    input_size_and_generate_string(text, pattern.size(), pattern);

    std::cout << "Generated text: " << text << "\n";

    int run_times;
    std::cout << "Enter the number of running times: ";
    std::cin >> run_times;
    std::cout << "===========================================";
    std::cout << "\n";

    double rk_avg = measure_and_print_execution_time("RK", search, pattern, text, PRIME_NUM, run_times);
    double kmp_avg = measure_and_print_execution_time("KMP", KMPSearchWrapper, pattern, text, PRIME_NUM, run_times);

    std::cout << "\n";
    std::cout << "RK average time: " << rk_avg << " seconds, KMP average time: " << kmp_avg << " seconds\n";

    return 0;
}
