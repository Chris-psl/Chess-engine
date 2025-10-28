// Chess engine main header file

#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include "utils.h"

std::string engine(std::string Command, std::string fenInput, BoardState& board);

#endif // ENGINE_H