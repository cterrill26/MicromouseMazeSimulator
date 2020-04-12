#include <iostream>
#include <cstdlib>  // atoi

#include "Maze.h"
#include "MazeDefinitions.h"
#include "PathFinder.h"
#include <stack>

/**
 * Demo of a PathFinder implementation.
 *
 * Do not use a left/right wall following algorithm, as most
 * Micromouse mazes are designed for such algorithms to fail.
 */
class LeftWallFollower : public PathFinder {
public:
	LeftWallFollower(bool shouldPause = false) : pause(shouldPause) {
		shouldGoForward = false;
		visitedStart = false;
	}

	MouseMovement nextMovement(unsigned x, unsigned y, const Maze &maze) {
		const bool frontWall = maze.wallInFront();
		const bool leftWall = maze.wallOnLeft();

		// Pause at each cell if the user requests it.
		// It allows for better viewing on command line.
		if (pause) {
			std::cout << "Hit enter to continue..." << std::endl;
			std::cin.ignore(10000, '\n');
			std::cin.clear();
		}

		std::cout << maze.draw(5) << std::endl << std::endl;

		// If we somehow miraculously hit the center
		// of the maze, just terminate and celebrate!
		if (isAtCenter(x, y)) {
			std::cout << "Found center! Good enough for the demo, won't try to get back." << std::endl;
			return Finish;
		}

		// If we hit the start of the maze a second time, then
		// we couldn't find the center and never will...
		if (x == 0 && y == 0) {
			if (visitedStart) {
				std::cout << "Unable to find center, giving up." << std::endl;
				return Finish;
			}
			else {
				visitedStart = true;
			}
		}

		// If we have just turned left, we should take that path!
		if (!frontWall && shouldGoForward) {
			shouldGoForward = false;
			return MoveForward;
		}

		// As long as nothing is in front and we have
		// a wall to our left, keep going forward!
		if (!frontWall && leftWall) {
			shouldGoForward = false;
			return MoveForward;
		}

		// If our forward and left paths are blocked
		// we should try going to the right!
		if (frontWall && leftWall) {
			shouldGoForward = false;
			return TurnClockwise;
		}

		// Lastly, if there is no left wall we should take that path!
		if (!leftWall) {
			shouldGoForward = true;
			return TurnCounterClockwise;
		}

		// If we get stuck somehow, just terminate.
		std::cout << "Got stuck..." << std::endl;
		return Finish;
	}

protected:
	// Helps us determine that we should go forward if we have just turned left.
	bool shouldGoForward;

	// Helps us determine if we've made a loop around the maze without finding the center.
	bool visitedStart;

	// Indicates we should pause before moving to next cell.
	// Useful for command line usage.
	const bool pause;

	bool isAtCenter(unsigned x, unsigned y) const {
		unsigned midpoint = MazeDefinitions::MAZE_LEN / 2;

		if (MazeDefinitions::MAZE_LEN % 2 != 0) {
			return x == midpoint && y == midpoint;
		}

		return  (x == midpoint && y == midpoint) ||
			(x == midpoint - 1 && y == midpoint) ||
			(x == midpoint && y == midpoint - 1) ||
			(x == midpoint - 1 && y == midpoint - 1);
	}
};

class FloodFill : public PathFinder {
public:
	FloodFill(bool shouldPause = false) : pause(shouldPause), dir(0), nextForward(true) {
		for (int i = 0; i < MazeDefinitions::MAZE_LEN / 2; i++) {
			for (int j = 0; j < MazeDefinitions::MAZE_LEN / 2; j++) {
				int distance = i + j;
				dist[MazeDefinitions::MAZE_LEN / 2 + i][MazeDefinitions::MAZE_LEN / 2 + j] = distance;
				dist[MazeDefinitions::MAZE_LEN / 2 + i][MazeDefinitions::MAZE_LEN / 2 - j - 1] = distance;
				dist[MazeDefinitions::MAZE_LEN / 2 - i - 1][MazeDefinitions::MAZE_LEN / 2 + j] = distance;
				dist[MazeDefinitions::MAZE_LEN / 2 - i - 1][MazeDefinitions::MAZE_LEN / 2 - j - 1] = distance;
			}
		}
		for (int i = 0; i < MazeDefinitions::MAZE_LEN; i++)
			for (int j = 0; j < MazeDefinitions::MAZE_LEN - 1; j++)
				vertWalls[j][i] = horzWalls[i][j] = false;

		vertWalls[0][0] = true;
	}

	MouseMovement nextMovement(unsigned x, unsigned y, const Maze &maze) {

		// Pause at each cell if the user requests it.
		// It allows for better viewing on command line.
		if (pause) {
			std::cout << "Hit enter to continue..." << std::endl;
			std::cin.ignore(10000, '\n');
			std::cin.clear();
		}

		std::cout << maze.draw(5) << std::endl << std::endl;

		// If we somehow miraculously hit the center
		// of the maze, just terminate and celebrate!
		if (isAtCenter(x, y)) {
			std::cout << "Found center! Good enough for the demo, won't try to get back." << std::endl;
			return Finish;
		}

		if (nextForward) {
			nextForward = false;
			return MoveForward;
		}

		nextForward = false;

		updateWalls(x, y, maze.wallInFront(), maze.wallOnLeft(), maze.wallOnRight());

	chooseDir:

		int curDist = dist[x][y];

		if (y < MazeDefinitions::MAZE_LEN - 1 && !horzWalls[x][y] && dist[x][y + 1] < curDist)
			return moveDir(0);
		if (x < MazeDefinitions::MAZE_LEN - 1 && !vertWalls[x][y] && dist[x + 1][y] < curDist)
			return moveDir(1);
		if (y > 0 && !horzWalls[x][y - 1] && dist[x][y - 1] < curDist)
			return moveDir(2);
		if (x > 0 && !vertWalls[x - 1][y] && dist[x - 1][y] < curDist)
			return moveDir(3);
		if (floodFill(x, y))
			goto chooseDir;

		// If we get stuck somehow, just terminate.
		std::cout << "Got stuck..." << std::endl;
		return Finish;
	}

protected:
	// Indicates we should pause before moving to next cell.
	// Useful for command line usage.
	const bool pause;

	int dir;

	bool nextForward;

	int dist[MazeDefinitions::MAZE_LEN][MazeDefinitions::MAZE_LEN];

	bool vertWalls[MazeDefinitions::MAZE_LEN - 1][MazeDefinitions::MAZE_LEN];

	bool horzWalls[MazeDefinitions::MAZE_LEN][MazeDefinitions::MAZE_LEN - 1];

	struct Coord {
		Coord(unsigned x, unsigned y) : m_x(x), m_y(y) {}
		unsigned m_x;
		unsigned m_y;
	};

	bool isAtCenter(unsigned x, unsigned y) const {
		unsigned midpoint = MazeDefinitions::MAZE_LEN / 2;

		if (MazeDefinitions::MAZE_LEN % 2 != 0) {
			return x == midpoint && y == midpoint;
		}

		return  (x == midpoint && y == midpoint) ||
			(x == midpoint - 1 && y == midpoint) ||
			(x == midpoint && y == midpoint - 1) ||
			(x == midpoint - 1 && y == midpoint - 1);
	}

	void updateWalls(unsigned x, unsigned y, bool front, bool left, bool right) {
		switch (dir) {
		case 0:
			if (y < MazeDefinitions::MAZE_LEN - 1 && front)
				horzWalls[x][y] = true;
			if (x > 0 && left)
				vertWalls[x - 1][y] = true;
			if (x < MazeDefinitions::MAZE_LEN - 1 && right)
				vertWalls[x][y] = true;
			break;
		case 1:
			if (x < MazeDefinitions::MAZE_LEN - 1 && front)
				vertWalls[x][y] = true;
			if (y < MazeDefinitions::MAZE_LEN - 1 && left)
				horzWalls[x][y] = true;
			if (y > 0 && right)
				horzWalls[x][y - 1] = true;
			break;
		case 2:
			if (y > 0 && front)
				horzWalls[x][y - 1] = true;
			if (x < MazeDefinitions::MAZE_LEN - 1 && left)
				vertWalls[x][y] = true;
			if (x > 0 && right)
				vertWalls[x - 1][y] = true;
			break;
		case 3:
		default:
			if (x > 0 && front)
				vertWalls[x - 1][y] = true;
			if (y > 0 && left)
				horzWalls[x][y - 1] = true;
			if (y < MazeDefinitions::MAZE_LEN - 1 && right)
				horzWalls[x][y] = true;
		}
	}

	MouseMovement moveDir(int d) {
		int diff = d - dir;
		dir = d;
		if (diff == 0)
			return MoveForward;

		nextForward = true;

		if (diff == -1 || diff == 3)
			return TurnCounterClockwise;
		if (diff == 1 || diff == -3)
			return TurnClockwise;
		return TurnAround;
	}

	bool floodFill(unsigned x, unsigned y) {
		std::stack<Coord> s;
		s.push(Coord(x, y));
		while (!s.empty()) {
			Coord c = s.top();
			s.pop();

			if (isAtCenter(c.m_x, c.m_y))
				continue;

			int min = 1000;
			if (c.m_y < MazeDefinitions::MAZE_LEN - 1 && !horzWalls[c.m_x][c.m_y])
				min = (dist[c.m_x][c.m_y + 1] < min) ? dist[c.m_x][c.m_y + 1] : min;
			if (c.m_x < MazeDefinitions::MAZE_LEN - 1 && !vertWalls[c.m_x][c.m_y])
				min = (dist[c.m_x + 1][c.m_y] < min) ? dist[c.m_x + 1][c.m_y] : min;
			if (c.m_y > 0 && !horzWalls[c.m_x][c.m_y - 1])
				min = (dist[c.m_x][c.m_y - 1] < min) ? dist[c.m_x][c.m_y - 1] : min;
			if (c.m_x > 0 && !vertWalls[c.m_x - 1][c.m_y])
				min = (dist[c.m_x - 1][c.m_y] < min) ? dist[c.m_x - 1][c.m_y] : min;

			if (min < dist[c.m_x][c.m_y] || min == 1000)
				continue;

			if (min > 300)
				return false;

			dist[c.m_x][c.m_y] = min + 1;

			if (c.m_y < MazeDefinitions::MAZE_LEN - 1)
				s.push(Coord(c.m_x, c.m_y + 1));
			if (c.m_x < MazeDefinitions::MAZE_LEN - 1)
				s.push(Coord(c.m_x + 1, c.m_y));
			if (c.m_y > 0)
				s.push(Coord(c.m_x, c.m_y - 1));
			if (c.m_x > 0)
				s.push(Coord(c.m_x - 1, c.m_y));
		}
		return true;
	}
};

int main(int argc, char * argv[]) {
	MazeDefinitions::MazeEncodingName mazeName = MazeDefinitions::MAZE_CAMM_2012;
	bool pause = false;

	// Since Windows does not support getopt directly, we will
	// have to parse the command line arguments ourselves.

	// Skip the program name, start with argument index 1
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
			int mazeOption = atoi(argv[++i]);
			if (mazeOption < MazeDefinitions::MAZE_NAME_MAX && mazeOption > 0) {
				mazeName = (MazeDefinitions::MazeEncodingName)mazeOption;
			}
		}
		else if (strcmp(argv[i], "-p") == 0) {
			pause = true;
		}
		else {
			std::cout << "Usage: " << argv[0] << " [-m N] [-p]" << std::endl;
			std::cout << "\t-m N will load the maze corresponding to N, or 0 if invalid N or missing option" << std::endl;
			std::cout << "\t-p will wait for a newline in between cell traversals" << std::endl;
			return -1;
		}
	}

	FloodFill floodFill(pause);
	Maze maze(mazeName, &floodFill);
	std::cout << maze.draw(5) << std::endl << std::endl;

	maze.start();
}
