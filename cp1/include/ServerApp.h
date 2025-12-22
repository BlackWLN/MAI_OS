#pragma once

#include "protocol.h"
#include "wrappers.h"

#include <pthread.h>
#include <string>
#include <vector>
#include <map>

class ServerApp {
public:
  ServerApp();
  ~ServerApp();
  void run();

private:
  std::vector<Player> players;
  std::vector<GameRoom> gameRooms;
  std::map<std::string, PlayerStats> playerStats;
  pthread_mutex_t list_mutex;
  NamedPipe serverPipe;
  bool isRunning;

  Player *findPlayer(const std::string &login);
  GameRoom *findGameRoom(const std::string &gameName);
  PlayerStats *getPlayerStats(const std::string &login);
  void sendToClient(const std::string &login, Packet &pkt);
  void sendBoard(Player *pTarget, GameBoard &boardOwner, bool showShips,
                 const char *title);
  void updateStatsAfterGame(const std::string &winner, const std::string &loser);
  void sendGameList(const std::string &login);

  void handleLogin(Packet &pkt);
  void handleCreateGame(Packet &pkt);
  void handleJoinGame(Packet &pkt);
  void handleLeaveGame(Packet &pkt);
  void handleShoot(Packet &pkt);
  void handleLogout(Packet &pkt);
  void handleGetStats(Packet &pkt);
  void startGame(GameRoom &room);
};