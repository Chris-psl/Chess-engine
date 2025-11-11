#pragma once
#include <cstdint>
#include <vector>
#include "movegen.h"

enum BoundType { EXACT, LOWERBOUND, UPPERBOUND };

struct TTEntry {
    uint64_t key;
    int depth;
    int score;
    BoundType flag;
    Move bestMove;
};

class TranspositionTable {
public:
    std::vector<TTEntry> table;
    size_t size;

    TranspositionTable(size_t mb = 64) {
        size = (mb * 1024 * 1024) / sizeof(TTEntry);
        table.resize(size);
    }

    void store(const TTEntry& entry) {
        size_t index = entry.key % size;
        if (table[index].depth <= entry.depth)
            table[index] = entry;
    }

    bool probe(uint64_t key, TTEntry& out) {
        size_t index = key % size;
        if (table[index].key == key) {
            out = table[index];
            return true;
        }
        return false;
    }
};
