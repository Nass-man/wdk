#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <numeric>
#include <cstdint>

// Print bytes in hex
void printHex(const std::vector<uint8_t>& data, std::ostream& out) {
    for (auto b : data)
        out << std::hex << std::setw(2) << std::setfill('0') << (int)b;
    out << std::dec;
}

// Calculate entropy in bits
double calculateEntropy(const std::vector<uint8_t>& data) {
    int counts[256] = {0};
    for (auto b : data) counts[b]++;
    double entropy = 0.0;
    int len = (int)data.size();
    for (int i = 0; i < 256; i++) {
        if (counts[i] == 0) continue;
        double p = (double)counts[i] / len;
        entropy -= p * std::log2(p);
    }
    return entropy;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " wallet.dat\n";
        return 1;
    }

    const int KEY_LENGTH = 32; // Increased key length for typical keys
    const double ENTROPY_THRESHOLD = 2.5; // Lower threshold for more candidates

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file.\n";
        return 1;
    }

    file.seekg(0, std::ios::end);
    size_t filesize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(filesize);
    file.read((char*)buffer.data(), filesize);

    std::ofstream outfile("wdk_candidates.txt");
    if (!outfile) {
        std::cerr << "Failed to open output file.\n";
        return 1;
    }

    size_t foundCount = 0;

    for (size_t i = 0; i + KEY_LENGTH <= filesize; i++) {
        std::vector<uint8_t> candidate(buffer.begin() + i, buffer.begin() + i + KEY_LENGTH);
        double entropy = calculateEntropy(candidate);

        // Debug output entropy every 10000 offsets
        if (i % 10000 == 0) {
            std::cout << "Offset " << i << ", entropy: " << entropy << std::endl;
        }

        // Temporarily accept all candidates with entropy above threshold
        if (entropy >= ENTROPY_THRESHOLD) {
            std::cout << "Candidate found at offset " << i << " (entropy " << entropy << "): ";
            printHex(candidate, std::cout);
            std::cout << std::endl;

            outfile << std::setw(10) << i << ": ";
            printHex(candidate, outfile);
            outfile << "\n";

            foundCount++;
        }
    }

    std::cout << "Scan complete. Found " << foundCount << " candidates.\n";
    std::cout << "Candidates saved to wdk_candidates.txt\n";

    return 0;
}
