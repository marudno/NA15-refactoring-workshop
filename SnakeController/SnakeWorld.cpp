#include "SnakeWorld.hpp"
#include "SnakeSegments.hpp"

namespace Snake
{

World::World(Segments::Dimension dimension, Position food)
    : m_foodPosition(food),
      m_dimension(dimension)
{}

void World::setFoodPosition(Position& pos)
{
    m_foodPosition = pos;
}

Position World::getFoodPosition() const
{
    return m_foodPosition;
}

bool World::contains(int x, int y) const
{
    return x >= 0 and x < m_dimension.width and y >= 0 and y < m_dimension.height;
}

} // namespace Snake
