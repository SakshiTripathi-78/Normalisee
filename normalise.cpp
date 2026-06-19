#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include <string>
#include <cmath>

// Structure to hold Functional Dependencies (e.g., A,B -> C)
struct FD {
    std::set<char> lhs;
    std::set<char> rhs;
};

// 1. Core Logic: Attribute Closure Algorithm
std::set<char> getClosure(std::set<char> input, const std::vector<FD>& fds) {
    std::set<char> closure = input;
    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& fd : fds) {
            // Check if LHS is a subset of current closure
            if (std::includes(closure.begin(), closure.end(), fd.lhs.begin(), fd.lhs.end())) {
                size_t oldSize = closure.size();
                // Insert RHS into closure
                closure.insert(fd.rhs.begin(), fd.rhs.end());
                if (closure.size() > oldSize) changed = true;
            }
        }
    }
    return closure;
}

// 2. Helper: Check if X is a superkey for Relation R
std::vector<FD> projectFDs(std::set<char> subR, const std::vector<FD>& fds);
bool isSuperKey(std::set<char> X, std::set<char> R, const std::vector<FD>& fds) {
    // Crucial: Use only dependencies relevant to the current relation R
    std::vector<FD> relevantFDs = projectFDs(R, fds);
    std::set<char> closure = getClosure(X, relevantFDs);
    
    // Check if R is a subset of the closure
    return std::includes(closure.begin(), closure.end(), R.begin(), R.end());
}

// 3. Helper: Project dependencies for sub-relations
std::vector<FD> projectFDs(std::set<char> subR, const std::vector<FD>& fds) {
    std::vector<FD> projected;
    for (const auto& fd : fds) {
        if (std::includes(subR.begin(), subR.end(), fd.lhs.begin(), fd.lhs.end()) &&
            std::includes(subR.begin(), subR.end(), fd.rhs.begin(), fd.rhs.end())) {
            projected.push_back(fd);
        }
    }
    return projected;
}

// 4. Parser: Convert "AB->C" into our FD structure
FD parseFD(std::string input) {
    size_t pos = input.find("->");
    FD fd;
    for (size_t i = 0; i < pos; ++i) fd.lhs.insert(input[i]);
    for (size_t i = pos + 2; i < input.length(); ++i) fd.rhs.insert(input[i]);
    return fd;
}

// 5. BCNF Violation Finder

FD* findBCNFViolation(const std::set<char>& R, const std::vector<FD>& fds) {
    for (auto& fd : fds) {
        // Only consider FDs that are entirely contained in the current R
        if (std::includes(R.begin(), R.end(), fd.lhs.begin(), fd.lhs.end()) &&
            std::includes(R.begin(), R.end(), fd.rhs.begin(), fd.rhs.end())) {
            
            // Check if LHS is a superkey for the current R
            if (!isSuperKey(fd.lhs, R, fds)) {
                return const_cast<FD*>(&fd); // Violation found
            }
        }
    }
    return nullptr; // Checked all, no violations
}

// 6. Recursive Decomposition Engine
void decompose(std::set<char> R, std::vector<FD> fds) {
    FD* violation = findBCNFViolation(R, fds);
    
    if (violation == nullptr) {
        std::cout << "Relation { ";
        for (char c : R) std::cout << c << " ";
        std::cout << "} is in BCNF." << std::endl;
        return;
    }

    FD v = *violation;
    std::set<char> X = v.lhs;
    std::set<char> Y = v.rhs;
    
    std::set<char> R1 = X;
    R1.insert(Y.begin(), Y.end());
    
    std::set<char> R2 = R;
    for (char c : Y) {
        if (X.find(c) == X.end()) R2.erase(c);
    }

    std::cout << "Decomposing based on violation..." << std::endl;
    
    // Correctly project dependencies for the recursive calls
    decompose(R1, projectFDs(R1, fds));
    decompose(R2, projectFDs(R2, fds));
}

int main() {
    std::set<char> R;
    std::vector<FD> fds;
    std::string input;

    // 1. Input Attributes
    std::cout << "Enter all attributes in R (e.g., ABCD): ";
    std::cin >> input;
    for (char c : input) R.insert(c);

    // 2. Input FDs
    std::cout << "Enter FDs (e.g., AB->C). Type 'done' to finish:" << std::endl;
    while (true) {
        std::cin >> input;
        if (input == "done") break;
        fds.push_back(parseFD(input));
    }

    std::cout << "\nStarting BCNF decomposition..." << std::endl;
    decompose(R, fds);

    return 0;
}