#pragma once

#include <list>

#include "SnakeInterface.hpp"

namespace Snake
{

class Segments
{
public:
    struct Dimension
    {
        int width, height;
    };    
    Segments(Direction direction);

    void addSegment(int x, int y);
    bool isCollision(int x, int y) const;
    void addHead(int x, int y);
    Position nextHead() const;
    Position removeTail();
    void updateDirection(Direction newDirection);
    Position getPosition();
private:
    Direction m_headDirection;
    std::list<Position> m_segments;
};

} // namespace Snake
