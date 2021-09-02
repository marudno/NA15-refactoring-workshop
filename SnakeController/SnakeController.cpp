#include "SnakeController.hpp"

#include <algorithm>
#include <sstream>

#include "EventT.hpp"
#include "IPort.hpp"

namespace Snake
{
ConfigurationError::ConfigurationError()
    : std::logic_error("Bad configuration of Snake::Controller.")
{}

UnexpectedEventException::UnexpectedEventException()
    : std::runtime_error("Unexpected event received!")
{}

Controller::Controller(IPort& p_displayPort, IPort& p_foodPort, IPort& p_scorePort, std::string const& p_config, SegmentsOfSnake& segmentsOfSnake, Board& board)
    : m_displayPort(p_displayPort),
      m_foodPort(p_foodPort),
      m_scorePort(p_scorePort),
      m_segmentsOfSnake(segmentsOfSnake),
      m_board(board)
{
    std::istringstream istr(p_config);
    char w, f, s, d;

    int width, height, length;
    int foodX, foodY;
    istr >> w >> width >> height >> f >> foodX >> foodY >> s;

    if (w == 'W' and f == 'F' and s == 'S') {
        m_board.setMapDimension(width, height);
        m_board.setFoodPosition(foodX, foodY);

        istr >> d;
        switch (d) {
            case 'U':
                m_segmentsOfSnake.setCurrentDirection(Direction_UP);
                break;
            case 'D':
                m_segmentsOfSnake.setCurrentDirection(Direction_DOWN);
                break;
            case 'L':
                m_segmentsOfSnake.setCurrentDirection(Direction_LEFT);
                break;
            case 'R':
                m_segmentsOfSnake.setCurrentDirection(Direction_RIGHT);
                break;
            default:
                throw ConfigurationError();
        }
        istr >> length;

        while (length) {
            Segment seg;
            istr >> seg.x >> seg.y;
            seg.ttl = length--;

            m_segmentsOfSnake.pushBackSegment(seg);
        }
    } else {
        throw ConfigurationError();
    }
}

void Controller::handleTimePassed(const TimeoutInd&)
{
    Segment newHead = m_segmentsOfSnake.getNewHead();

    if(m_segmentsOfSnake.doesCollideWithSnake(newHead))
    {
        notifyAboutFailure();
        return;
    }
    if(m_board.doesCollideWithFood(newHead))
    {
        m_scorePort.send(std::make_unique<EventT<ScoreInd>>());
        m_foodPort.send(std::make_unique<EventT<FoodReq>>());
    }
    else if (m_board.doesCollideWithWall(newHead))
    {
        notifyAboutFailure();
        return;
    }
    else
    {
        for (auto &segment : m_segmentsOfSnake.getSegmentsList()) {
            if (not --segment.ttl) {
                repaintTile(segment, Cell_FREE);
            }
        }
    }

    m_segmentsOfSnake.pushFrontSegment(newHead);
    repaintTile(newHead, Cell_SNAKE);

    m_segmentsOfSnake.cleanNotExistingSnakeSegments();
}

void Controller::handleDirectionChange(const DirectionInd& directionInd)
{
    auto direction = directionInd.direction;

    if ((m_segmentsOfSnake.getCurrentDirection() & 0b01) != (direction & 0b01)) {
        m_segmentsOfSnake.setCurrentDirection(direction);
    }
}

void Controller::handleFoodPositionChange(const FoodInd& receivedFood)
{
    bool requestedFoodCollidedWithSnake = false;
    for (auto const& segment : m_segmentsOfSnake.getSegmentsList()) {
        if (segment.x == receivedFood.x and segment.y == receivedFood.y) {
            requestedFoodCollidedWithSnake = true;
            break;
        }
    }

    if (requestedFoodCollidedWithSnake) {
        m_foodPort.send(std::make_unique<EventT<FoodReq>>());
    } else {
        repaintTile(m_board.getFoodPosition().first, m_board.getFoodPosition().second, Cell_FREE);

        repaintTile(receivedFood.x, receivedFood.y, Cell_FOOD);
    }

    m_board.setFoodPosition(receivedFood.x, receivedFood.y);
}

void Controller::handleNewFood(const FoodResp& requestedFood)
{
    bool requestedFoodCollidedWithSnake = false;
    for (auto const& segment : m_segmentsOfSnake.getSegmentsList()) {
        if (segment.x == requestedFood.x and segment.y == requestedFood.y) {
            requestedFoodCollidedWithSnake = true;
            break;
        }
    }

    if (requestedFoodCollidedWithSnake) {
        m_foodPort.send(std::make_unique<EventT<FoodReq>>());
    } else {
        DisplayInd placeNewFood;
        placeNewFood.x = requestedFood.x;
        placeNewFood.y = requestedFood.y;
        placeNewFood.value = Cell_FOOD;
        m_displayPort.send(std::make_unique<EventT<DisplayInd>>(placeNewFood));
    }

    m_board.setFoodPosition(requestedFood.x, requestedFood.y);
}

void Controller::notifyAboutFailure()
{
    m_scorePort.send(std::make_unique<EventT<LooseInd>>());
}

void Controller::repaintTile(const Snake::Segment &position, Cell type)
{
    repaintTile(position.x, position.y, type);
}

void Controller::repaintTile(unsigned int x, unsigned int y, Cell type)
{
    DisplayInd indication{};
    indication.x = x;
    indication.y = y;
    indication.value = type;
    m_displayPort.send(std::make_unique<EventT<DisplayInd>>(indication));
}

void Controller::receive(std::unique_ptr<Event> e)
{
    switch(e->getMessageId())
    {
        case TimeoutInd::MESSAGE_ID: return handleTimePassed(*static_cast<EventT<TimeoutInd> const&>(*e));
        case DirectionInd::MESSAGE_ID: return handleDirectionChange(*static_cast<EventT<DirectionInd> const&>(*e));
        case FoodInd::MESSAGE_ID: return handleFoodPositionChange(*static_cast<EventT<FoodInd> const&>(*e));
        case FoodResp::MESSAGE_ID: return handleNewFood(*static_cast<EventT<FoodResp> const&>(*e));
        default: throw UnexpectedEventException();
    };
}

Direction& SegmentsOfSnake::getCurrentDirection()
{
    return m_currentDirection;
}

std::list<Segment>& SegmentsOfSnake::getSegmentsList()
{
    return m_segments;
}

Segment SegmentsOfSnake::getNewHead() const
{
    Segment const& currentHead = m_segments.front();

    Segment newHead;
    newHead.x = currentHead.x + ((m_currentDirection & 0b01) ? (m_currentDirection & 0b10) ? 1 : -1 : 0);
    newHead.y = currentHead.y + (not (m_currentDirection & 0b01) ? (m_currentDirection & 0b10) ? 1 : -1 : 0);
    newHead.ttl = currentHead.ttl;

    return newHead;
}

void SegmentsOfSnake::setCurrentDirection(const Direction& newDirection)
{
    m_currentDirection = newDirection;
}

void SegmentsOfSnake::pushBackSegment(const Segment& seg)
{
    m_segments.push_back(seg);
}

void SegmentsOfSnake::pushFrontSegment(const Segment& seg)
{
    m_segments.push_front(seg);
}

void SegmentsOfSnake::cleanNotExistingSnakeSegments()
{
    m_segments.erase(
         std::remove_if(
             m_segments.begin(),
             m_segments.end(),
             [](auto const& segment){ return not (segment.ttl > 0); }),
         m_segments.end());
}

bool SegmentsOfSnake::doesCollideWithSnake(const Snake::Segment& newSegment) const
{
    for (auto segment : m_segments) {
        if (segment.x == newSegment.x and segment.y == newSegment.y) {
            return true;
        }
    }
    return false;
}

void Board::setMapDimension(int width, int height)
{
    m_mapDimension = std::make_pair(width, height);
}

void Board::setFoodPosition(int x, int y)
{
    m_foodPosition = std::make_pair(x, y);
}

std::pair<int, int>& Board::getFoodPosition()
{
    return m_foodPosition;
}

std::pair<int, int>& Board::getMapDimension()
{
    return m_mapDimension;
}

bool Board::doesCollideWithWall(const Snake::Segment &newSegment)
{
    return newSegment.x < 0 or newSegment.y < 0 or
           newSegment.x >= m_mapDimension.first or
           newSegment.y >= m_mapDimension.second;
}

bool Board::doesCollideWithFood(const Snake::Segment &newHead)
{
    return std::make_pair(newHead.x, newHead.y) == m_foodPosition;
}
} // namespace Snake
