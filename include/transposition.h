#pragma once
#include <cstdint>
#include <vector>
#include <mutex>
#include "movegen.h"

// Bound types for transposition table entries
enum BoundType { EXACT, LOWERBOUND, UPPERBOUND };

// Transposition Table Entry
struct TTEntry {
    uint64_t key = 0;       // Zobrist hash key
    int depth = 0;          // Search depth
    int score = 0;          // Evaluation score
    BoundType flag = EXACT; // Bound type
    Move bestMove;          // Best move found
};

// Transposition Table
class TranspositionTable {
public:
    std::vector<TTEntry> table;  // Storage for transposition table entries
    size_t size;                 // Number of entries in the table

    // Constructor
    TranspositionTable(size_t mb = 64) {
        size = (mb * 1024 * 1024) / sizeof(TTEntry);
        if (size == 0) size = 1;
        table.resize(size);
    }

    // Store an entry in the transposition table (thread-safe)
    void store(const TTEntry& entry) {
        size_t index = entry.key % size;
        std::lock_guard<std::mutex> lock(mtx);
        // simple replacement: prefer deeper entries
        if (table[index].depth <= entry.depth) {
            table[index] = entry;
        }
    }

    // Probe the transposition table for an entry (thread-safe)
    bool probe(uint64_t key, TTEntry& out) {
        size_t index = key % size;
        std::lock_guard<std::mutex> lock(mtx);
        if (table[index].key == key && table[index].depth > 0) {
            out = table[index];
            return true;
        }
        return false;
    }

private:
    std::mutex mtx;
};

// Declare extern so other translation units can reference the global TT
extern TranspositionTable TT;
