#pragma once
#include <string>

namespace Atomica {

struct Element {
    std::string name;
    std::string symbol;
    int atomicNumber;
    double atomicMass;
};

// Identify element and return description string
std::string describeAtom(int protons, int neutrons, const int eCount[7]);
}
