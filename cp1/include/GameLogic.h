#pragma once

#include <cstdlib>
#include <ctime>
#include <vector>

enum CellState { EMPTY = 0, SHIP = 1, MISS = 2, HIT = 3 };

enum ShotResult { RES_MISS, RES_HIT, RES_SUNK, RES_REPEAT, RES_LOSE };

class GameBoard {
public:
  GameBoard();

  void placeShipsRandomly();

  ShotResult processShot(int x, int y);

  int getCell(int x, int y) const { return grid[y][x]; }

  void getBoardString(char *buffer, bool showShips);

private:
  static const int SIZE = 10;
  int grid[SIZE][SIZE];
  int shipsAlive;
};