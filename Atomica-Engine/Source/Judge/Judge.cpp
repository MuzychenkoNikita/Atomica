#include "Judge.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace Atomica
{

static const Element PERIODIC_TABLE[118] = {
    {"Hydrogen","H",1},{"Helium","He",2},{"Lithium","Li",3},{"Beryllium","Be",4},
    {"Boron","B",5},{"Carbon","C",6},{"Nitrogen","N",7},{"Oxygen","O",8},
    {"Fluorine","F",9},{"Neon","Ne",10},{"Sodium","Na",11},{"Magnesium","Mg",12},
    {"Aluminum","Al",13},{"Silicon","Si",14},{"Phosphorus","P",15},{"Sulfur","S",16},
    {"Chlorine","Cl",17},{"Argon","Ar",18},{"Potassium","K",19},{"Calcium","Ca",20},
    {"Scandium","Sc",21},{"Titanium","Ti",22},{"Vanadium","V",23},{"Chromium","Cr",24},
    {"Manganese","Mn",25},{"Iron","Fe",26},{"Cobalt","Co",27},{"Nickel","Ni",28},
    {"Copper","Cu",29},{"Zinc","Zn",30},{"Gallium","Ga",31},{"Germanium","Ge",32},
    {"Arsenic","As",33},{"Selenium","Se",34},{"Bromine","Br",35},{"Krypton","Kr",36},
    {"Rubidium","Rb",37},{"Strontium","Sr",38},{"Yttrium","Y",39},{"Zirconium","Zr",40},
    {"Niobium","Nb",41},{"Molybdenum","Mo",42},{"Technetium","Tc",43},{"Ruthenium","Ru",44},
    {"Rhodium","Rh",45},{"Palladium","Pd",46},{"Silver","Ag",47},{"Cadmium","Cd",48},
    {"Indium","In",49},{"Tin","Sn",50},{"Antimony","Sb",51},{"Tellurium","Te",52},
    {"Iodine","I",53},{"Xenon","Xe",54},{"Cesium","Cs",55},{"Barium","Ba",56},
    {"Lanthanum","La",57},{"Cerium","Ce",58},{"Praseodymium","Pr",59},{"Neodymium","Nd",60},
    {"Promethium","Pm",61},{"Samarium","Sm",62},{"Europium","Eu",63},{"Gadolinium","Gd",64},
    {"Terbium","Tb",65},{"Dysprosium","Dy",66},{"Holmium","Ho",67},{"Erbium","Er",68},
    {"Thulium","Tm",69},{"Ytterbium","Yb",70},{"Lutetium","Lu",71},{"Hafnium","Hf",72},
    {"Tantalum","Ta",73},{"Tungsten","W",74},{"Rhenium","Re",75},{"Osmium","Os",76},
    {"Iridium","Ir",77},{"Platinum","Pt",78},{"Gold","Au",79},{"Mercury","Hg",80},
    {"Thallium","Tl",81},{"Lead","Pb",82},{"Bismuth","Bi",83},{"Polonium","Po",84},
    {"Astatine","At",85},{"Radon","Rn",86},{"Francium","Fr",87},{"Radium","Ra",88},
    {"Actinium","Ac",89},{"Thorium","Th",90},{"Protactinium","Pa",91},{"Uranium","U",92},
    {"Neptunium","Np",93},{"Plutonium","Pu",94},{"Americium","Am",95},{"Curium","Cm",96},
    {"Berkelium","Bk",97},{"Californium","Cf",98},{"Einsteinium","Es",99},{"Fermium","Fm",100},
    {"Mendelevium","Md",101},{"Nobelium","No",102},{"Lawrencium","Lr",103},{"Rutherfordium","Rf",104},
    {"Dubnium","Db",105},{"Seaborgium","Sg",106},{"Bohrium","Bh",107},{"Hassium","Hs",108},
    {"Meitnerium","Mt",109},{"Darmstadtium","Ds",110},{"Roentgenium","Rg",111},{"Copernicium","Cn",112},
    {"Nihonium","Nh",113},{"Flerovium","Fl",114},{"Moscovium","Mc",115},{"Livermorium","Lv",116},
    {"Tennessine","Ts",117},{"Oganesson","Og",118}
};

static const int SHELL_MAX[7] = {
    2, 8, 18, 32, 50, 72, 98
};

struct Orbital {
    std::string name;
    int capacity;
    int shell;
};

static const Orbital AUFBAU[] = {
    {"1s", 2, 1},
    {"2s", 2, 2},
    {"2p", 6, 2},
    {"3s", 2, 3},
    {"3p", 6, 3},
    {"4s", 2, 4},
    {"3d", 10, 3},
    {"4p", 6, 4},
    {"5s", 2, 5},
    {"4d", 10, 4},
    {"5p", 6, 5},
    {"6s", 2, 6},
    {"4f", 14, 4},
    {"5d", 10, 5},
    {"6p", 6, 6},
    {"7s", 2, 7},
    {"5f", 14, 5},
    {"6d", 10, 6},
    {"7p", 6, 7}
};

std::string canonicalElectronConfiguration(int Z) {
    if (Z <= 0) return "No electrons";
    int remaining = Z;
    std::ostringstream oss;
    for (const Orbital &o : AUFBAU) {
        if (remaining <= 0) break;
        int put = std::min(remaining, o.capacity);
        oss << o.name << "^" << put;
        remaining -= put;
        if (remaining > 0) oss << " ";
    }
    if (remaining > 0) {
        oss << " +(" << remaining << "e- in higher orbitals)";
    }
    return oss.str();
}

std::string validateShellDistribution(const int eCount[7]) {
    int total = 0;
    std::ostringstream oss;
    for (int i = 0; i < 7; ++i) {
        int maxAllowed = SHELL_MAX[i];
        int have = eCount[i];
        total += have;
        if (have < 0) {
            oss << "Shell " << (i+1) << " has negative electrons (" << have << "). ";
        } else if (have > maxAllowed) {
            oss << "Shell " << (i+1) << " overfilled: " << have << " > max " << maxAllowed << ". ";
        }
    }
    if (total == 0) oss << "No electrons present. ";
    return oss.str();
}

std::string isotopeStabilityHeuristic(int Z, int N) {
    if (Z <= 0) return "Invalid Z";
    int A = Z + N;
    if (N < 0) return "Invalid neutron count";
    if (Z <= 20) {
        if (std::abs(N - Z) <= 2) return "likely stable";
        else return "possibly unstable (check radioactivity)";
    } else if (Z <= 83) {

        double ratio = (double)N / (double)Z;
        if (ratio >= 1.0 && ratio <= 1.6) return "possibly stable";
        else return "likely unstable / radioactive";
    } else {

        return "very likely radioactive (heavy element)";
    }
}


std::string describeAtom(int numProtons, int numNeutrons, const int eCount[7]) {
    std::ostringstream out;
    out << std::fixed;
    out << "===== Atom description =====\n";
    
    if (numProtons <= 0) {
        out << "Protons: " << numProtons << " (invalid number of protons)\n";
        out << "Element: Unknown\n";
    } else if (numProtons > 118) {
        out << "Protons: " << numProtons << " (beyond known elements)\n";
        out << "Element: Unknown (Z > 118)\n";
    } else {
        const Element &el = PERIODIC_TABLE[numProtons - 1];
        out << "Element: " << el.name << " (" << el.symbol << "), atomic number Z = " << el.atomicNumber << "\n";
    }
    
    out << "Protons: " << numProtons << "\n";
    out << "Neutrons: " << numNeutrons << "\n";
    int massNumber = numProtons + numNeutrons;
    out << "Mass number (A = Z + N): " << massNumber << "\n";
    
    int totalElectrons = 0;
    for (int i = 0; i < 7; ++i) totalElectrons += eCount[i];
    int charge = numProtons - totalElectrons;
    out << "Total electrons (from shells): " << totalElectrons << "\n";
    out << "Net charge: ";
    if (charge == 0) out << "0 (neutral)\n";
    else if (charge > 0) out << "+" << charge << " (cation)\n";
    else out << charge << " (anion)\n";
    

    std::string validation = validateShellDistribution(eCount);
    if (!validation.empty()) {
        out << "Shell validation warnings: " << validation << "\n";
    }
    
    // Print per-shell distribution
    out << "Electron distribution by shell: ";
    for (int i = 0; i < 7; ++i) {
        out << eCount[i];
        if (i < 6) out << " - ";
    }
    out << "\n";
    
    if (numProtons >= 1 && numProtons <= 118) {

        int shellCanonical[7] = {0};
        int remaining = numProtons;
        for (const Orbital &o : AUFBAU) {
            if (remaining <= 0) break;
            int put = std::min(remaining, o.capacity);
            int sidx = o.shell - 1;
            if (sidx >= 0 && sidx < 7) shellCanonical[sidx] += put;
            remaining -= put;
        }
        out << "Canonical (approx) shell counts: ";
        for (int i = 0; i < 7; ++i) {
            out << shellCanonical[i];
            if (i < 6) out << " - ";
        }
        out << "\n";
    }
    

    out << "Isotope stability (heuristic): " << isotopeStabilityHeuristic(numProtons, numNeutrons) << "\n";
    
    out << "=============================\n";
    return out.str();
}
}
