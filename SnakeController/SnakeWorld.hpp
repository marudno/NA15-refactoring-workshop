#pragma once
#include "SnakePosition.hpp"
#include "SnakeSegments.hpp"
#include <utility>

namespace Snake
{

class World
{
public:
    World(Segments::Dimension dimension, Position food);

    void setFoodPosition(Position& pos);
    Position getFoodPosition() const;

    bool contains(int x, int y) const;

private:
    Position m_foodPosition;
    Segments::Dimension m_dimension;
};

} // namespace Snake
