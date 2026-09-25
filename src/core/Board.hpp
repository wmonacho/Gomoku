#pragma once

#include "app/Dto.hpp"

class Board {
public:
    Board();

    Cell at(Pos pos) const;
    void place(Pos pos, Cell color);
    void remove(Pos pos);
    bool isInside(Pos pos) const;

private:
};
