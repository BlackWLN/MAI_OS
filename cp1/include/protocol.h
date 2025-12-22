#pragma once

#include "GameLogic.h"

#include <string>

#define SERVER_PIPE "/tmp/battleship_server_pipe"
#define CLIENT_PIPE_PREFIX "/tmp/client_"

enum MsgType {
  LOGIN,
  CREATE_GAME,
  JOIN_GAME,
  LEAVE_GAME,
  SHOOT,
  LOGOUT,
  GET_STATS,
  GET_GAME_LIST,
  S_MSG,
  S_GAME_LIST,
  S_GAME_CREATED,
  S_GAME_START,
  S_SHOT_RESULT,
  S_GAME_OVER,
  S_BOARD,
  S_STATS
};

struct Packet {
  int type;
  char sender[32];
  char gameName[64];
  char payload[512];
  int x;
  int y;
  int shotResult;
};

struct Player {
  std::string login;
  bool inGame;
  std::string gameName;
  GameBoard board;
  bool isTurn;
  std::string opponent;
};

struct GameRoom {
  std::string name;
  std::string creator;
  std::string player1;
  std::string player2;
  bool isFull;
  bool isActive;
};

struct PlayerStats {
  std::string login;
  int gamesPlayed;
  int wins;
  int losses;
  int totalShots;
  int hits;
  double accuracy;
};
