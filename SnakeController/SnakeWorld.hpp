#pragma once

#include <utility>

namespace Snake
{

class World
{
public:
    World(Position dimension, Position food);

    void setFoodPosition(Position& pos);
    Position getFoodPosition() const;

    bool contains(int x, int y) const;

private:
    Position m_foodPosition;
    Position m_dimension;
};

} // namespace Snake
