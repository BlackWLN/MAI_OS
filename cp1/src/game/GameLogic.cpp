#include "GameLogic.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

GameBoard::GameBoard() {
  shipsAlive = 0;
  for (int i = 0; i < SIZE; ++i) {
    for (int j = 0; j < SIZE; ++j) {
      grid[i][j] = EMPTY;
    }
  }
}

void GameBoard::placeShipsRandomly() {
  static const int BATTLESHIP = 4;
  static const int CRUISER = 3;
  static const int DESTROYER = 2;
  static const int SUBMARINE = 1;

  for (int i = 0; i < SIZE; ++i) {
    for (int j = 0; j < SIZE; ++j) {
      grid[i][j] = EMPTY;
    }
  }
  shipsAlive = 0;

  int ships[] = {BATTLESHIP, CRUISER,   CRUISER,   DESTROYER, DESTROYER,
                 DESTROYER,  SUBMARINE, SUBMARINE, SUBMARINE, SUBMARINE};

  for (int len : ships) {
    bool placed = false;
    while (!placed) {
      int row = std::rand() % SIZE;
      int col = std::rand() % SIZE;
      bool horizontal = std::rand() % 2;

      bool canPlace = true;
      for (int k = 0; k < len; ++k) {
        int nrow = row + (horizontal ? 0 : k);
        int ncol = col + (horizontal ? k : 0);

        if (nrow >= SIZE || ncol >= SIZE || grid[nrow][ncol] != EMPTY) {
          canPlace = false;
          break;
        }
      }

      if (canPlace) {
        for (int k = 0; k < len; ++k) {
          int nrow = row + (horizontal ? 0 : k);
          int ncol = col + (horizontal ? k : 0);
          grid[nrow][ncol] = SHIP;
          shipsAlive++;
        }
        placed = true;
      }
    }
  }
}

ShotResult GameBoard::processShot(int x, int y) {
  if (x < 0 || x >= SIZE || y < 0 || y >= SIZE)
    return RES_REPEAT;

  int &cell = grid[y][x];

  if (cell == MISS || cell == HIT)
    return RES_REPEAT;

  if (cell == EMPTY) {
    cell = MISS;
    return RES_MISS;
  }

  if (cell == SHIP) {
    cell = HIT;
    shipsAlive--;
    if (shipsAlive <= 0)
      return RES_LOSE;
    return RES_HIT;
  }

  return RES_REPEAT;
}

void GameBoard::getBoardString(char *buffer, bool showShips) {
  int pos = 0;

  pos += sprintf(buffer + pos, "  0 1 2 3 4 5 6 7 8 9\n");
  pos += sprintf(buffer + pos, " ---------------------\n");

  for (int i = 0; i < SIZE; ++i) {
    pos += sprintf(buffer + pos, "%d ", i);
    for (int j = 0; j < SIZE; ++j) {
      char symbol = '.';
      int cell = grid[i][j];

      if (cell == EMPTY) {
        symbol = '.';
      } else if (cell == SHIP) {
        symbol = showShips ? '#' : '.';
      } else if (cell == MISS) {
        symbol = '*';
      } else if (cell == HIT) {
        symbol = 'X';
      }
      pos += sprintf(buffer + pos, "%c ", symbol);
    }
    pos += sprintf(buffer + pos, "\n");
  }
  buffer[pos] = '\0';
}