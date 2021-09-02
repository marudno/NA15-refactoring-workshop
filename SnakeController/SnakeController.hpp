#pragma once

#include <list>
#include <memory>
#include <stdexcept>

#include "IEventHandler.hpp"
#include "SnakeInterface.hpp"

class Event;
class IPort;

namespace Snake
{
struct Segment
{
    int x;
    int y;
    int ttl;
};

class SegmentsOfSnake
{
public:
    Direction& getCurrentDirection();
    std::list<Segment>& getSegmentsList();
    void pushBackSegment(const Segment& seg);
    void pushFrontSegment(const Segment& seg);
    void setCurrentDirection(const Direction& newDirection);
    void cleanNotExistingSnakeSegments();
    bool doesCollideWithSnake(const Segment& newSegment) const;
    Segment getNewHead() const;
private:
    Direction m_currentDirection;
    std::list<Segment> m_segments;
};

class Board
{
public:

    std::pair<int, int>& getFoodPosition();
    std::pair<int, int>& getMapDimension();
    void setMapDimension(int width, int height);
    void setFoodPosition(int x, int y);
    bool doesCollideWithWall(const Segment& newSegment);
    bool doesCollideWithFood(const Segment& newSegment);
private:
    std::pair<int, int> m_mapDimension;
    std::pair<int, int> m_foodPosition;
};

struct ConfigurationError : std::logic_error
{
    ConfigurationError();
};

struct UnexpectedEventException : std::runtime_error
{
    UnexpectedEventException();
};

class Controller : public IEventHandler
{
public:
    Controller(IPort& p_displayPort, IPort& p_foodPort, IPort& p_scorePort, std::string const& p_config, SegmentsOfSnake& segmentsOfSnake, Board& board);

    Controller(Controller const& p_rhs) = delete;
    Controller& operator=(Controller const& p_rhs) = delete;

    void receive(std::unique_ptr<Event> e) override;
private:
    void handleTimePassed(const TimeoutInd&);
    void handleDirectionChange(const DirectionInd&);
    void handleFoodPositionChange(const FoodInd& receivedFood);
    void handleNewFood(const FoodResp& requestedFood);
    void pause(std::unique_ptr<Event> e);

    void notifyAboutFailure();
    void repaintTile(const Segment& position, Cell type);
    void repaintTile(unsigned int x, unsigned int y, Cell type);

    IPort& m_displayPort;
    IPort& m_foodPort;
    IPort& m_scorePort;

    SegmentsOfSnake m_segmentsOfSnake;
    Board m_board;
};

} // namespace Snake
