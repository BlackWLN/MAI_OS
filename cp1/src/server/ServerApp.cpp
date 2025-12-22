#include "ServerApp.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>

ServerApp::ServerApp() : serverPipe(SERVER_PIPE), isRunning(true) {
  list_mutex = PTHREAD_MUTEX_INITIALIZER;
}

ServerApp::~ServerApp() { pthread_mutex_destroy(&list_mutex); }

void ServerApp::sendToClient(const std::string &login, Packet &pkt) {
  std::string pipePath = CLIENT_PIPE_PREFIX + login;
  NamedPipe pipe(pipePath);
  if (pipe.openPipe(O_WRONLY)) {
    pipe.send(&pkt, sizeof(Packet));
    pipe.closePipe();
  } else {
    std::cerr << "[Error] Failed to send message to player " << login
              << " (pipe is not available)\n";
  }
}

void ServerApp::sendBoard(Player *pTarget, GameBoard &boardOwner,
                          bool showShips, const char *title) {
  Packet pkt;
  pkt.type = S_BOARD;
  strcpy(pkt.sender, "SERVER");

  char boardStr[400];
  boardOwner.getBoardString(boardStr, showShips);

  sprintf(pkt.payload, "%s\n%s", title, boardStr);
  sendToClient(pTarget->login, pkt);
}

Player *ServerApp::findPlayer(const std::string &login) {
  for (auto &p : players) {
    if (p.login == login)
      return &p;
  }
  return nullptr;
}

GameRoom *ServerApp::findGameRoom(const std::string &gameName) {
  for (auto &room : gameRooms) {
    if (room.name == gameName)
      return &room;
  }
  return nullptr;
}

PlayerStats *ServerApp::getPlayerStats(const std::string &login) {
  auto it = playerStats.find(login);
  if (it != playerStats.end()) {
    return &it->second;
  }

  PlayerStats newStats;
  newStats.login = login;
  newStats.gamesPlayed = 0;
  newStats.wins = 0;
  newStats.losses = 0;
  newStats.totalShots = 0;
  newStats.hits = 0;
  newStats.accuracy = 0.0;
  
  playerStats[login] = newStats;
  return &playerStats[login];
}

void ServerApp::updateStatsAfterGame(const std::string &winner, const std::string &loser) {
  PlayerStats *winnerStats = getPlayerStats(winner);
  PlayerStats *loserStats = getPlayerStats(loser);
  
  winnerStats->gamesPlayed++;
  winnerStats->wins++;
  
  loserStats->gamesPlayed++;
  loserStats->losses++;
  
  if (winnerStats->totalShots > 0) {
    winnerStats->accuracy = (double)winnerStats->hits / winnerStats->totalShots * 100;
  }
  if (loserStats->totalShots > 0) {
    loserStats->accuracy = (double)loserStats->hits / loserStats->totalShots * 100;
  }
}

void ServerApp::sendGameList(const std::string &login) {
  Packet pkt;
  pkt.type = S_GAME_LIST;
  strcpy(pkt.sender, "SERVER");
  
  std::stringstream ss;
  ss << "Available games:\n";
  ss << "================\n";
  
  int count = 0;
  for (const auto &room : gameRooms) {
    if (!room.isFull && !room.isActive) {
      ss << room.name << " (created by " << room.creator << ")\n";
      count++;
    }
  }
  
  if (count == 0) {
    ss << "No available games. Create your own with /create <game_name>\n";
  } else {
    ss << "\nTo join game: /join <game_name>\n";
  }
  
  strcpy(pkt.payload, ss.str().c_str());
  sendToClient(login, pkt);
}

void ServerApp::handleLogin(Packet &pkt) {
  if (findPlayer(pkt.sender)) {
    std::cout << "[Login] Reject: " << pkt.sender << " is already online."
              << std::endl;
    return;
  }
  
  players.push_back({pkt.sender, false, "", GameBoard(), false, ""});
  std::cout << "[Login] New player: " << pkt.sender << std::endl;

  Packet resp;
  resp.type = S_MSG;
  strcpy(resp.payload, "Welcome to the Sea Fight server!");
  sendToClient(pkt.sender, resp);

  sendGameList(pkt.sender);
}

void ServerApp::handleCreateGame(Packet &pkt) {
  Player *player = findPlayer(pkt.sender);
  if (!player) {
    return;
  }
  
  if (player->inGame) {
    Packet err;
    err.type = S_MSG;
    strcpy(err.payload, "You are already in a game!");
    sendToClient(pkt.sender, err);
    return;
  }
  
  std::string gameName = pkt.gameName;
  if (gameName.empty()) {
    Packet err;
    err.type = S_MSG;
    strcpy(err.payload, "Game name cannot be empty!");
    sendToClient(pkt.sender, err);
    return;
  }
  
  if (findGameRoom(gameName)) {
    Packet err;
    err.type = S_MSG;
    strcpy(err.payload, "Game with this name already exists!");
    sendToClient(pkt.sender, err);
    return;
  }

  GameRoom newRoom;
  newRoom.name = gameName;
  newRoom.creator = pkt.sender;
  newRoom.player1 = pkt.sender;
  newRoom.player2 = "";
  newRoom.isFull = false;
  newRoom.isActive = false;
  
  gameRooms.push_back(newRoom);
  player->gameName = gameName;
  
  std::cout << "[Game Created] " << gameName << " by " << pkt.sender << std::endl;
  
  Packet resp;
  resp.type = S_GAME_CREATED;
  sprintf(resp.payload, "Game '%s' created! Waiting for opponent...\nUse '/leave' to cancel", gameName.c_str());
  sendToClient(pkt.sender, resp);

  for (const auto &p : players) {
    if (!p.inGame) {
      sendGameList(p.login);
    }
  }
}

void ServerApp::handleJoinGame(Packet &pkt) {
  Player *player = findPlayer(pkt.sender);
  if (!player) {
    return;
  }
  
  if (player->inGame) {
    Packet err;
    err.type = S_MSG;
    strcpy(err.payload, "You are already in a game!");
    sendToClient(pkt.sender, err);
    return;
  }
  
  std::string gameName = pkt.gameName;
  GameRoom *room = findGameRoom(gameName);
  
  if (!room) {
    Packet err;
    err.type = S_MSG;
    strcpy(err.payload, "Game not found!");
    sendToClient(pkt.sender, err);
    return;
  }
  
  if (room->isFull) {
    Packet err;
    err.type = S_MSG;
    strcpy(err.payload, "Game is already full!");
    sendToClient(pkt.sender, err);
    return;
  }
  
  if (room->creator == pkt.sender) {
    Packet err;
    err.type = S_MSG;
    strcpy(err.payload, "You cannot join your own game!");
    sendToClient(pkt.sender, err);
    return;
  }

  room->player2 = pkt.sender;
  room->isFull = true;
  
  player->gameName = gameName;
  player->inGame = true;
  
  Player *creator = findPlayer(room->creator);
  if (creator) {
    creator->inGame = true;
  }
  
  std::cout << "[Game Joined] " << pkt.sender << " joined " << gameName << std::endl;
  
  startGame(*room);
}

void ServerApp::handleLeaveGame(Packet &pkt) {
  Player *player = findPlayer(pkt.sender);
  if (!player) {
    return;
  }
  
  if (player->gameName.empty()) {
    Packet err;
    err.type = S_MSG;
    strcpy(err.payload, "You are not in any game!");
    sendToClient(pkt.sender, err);
    return;
  }
  
  GameRoom *room = findGameRoom(player->gameName);
  if (room) {
    if (room->isFull) {
      Player *opponent = findPlayer(player->opponent);
      if (opponent) {
        Packet winPkt;
        winPkt.type = S_GAME_OVER;
        strcpy(winPkt.payload, "Opponent left the game.\n YOU WON!");
        sendToClient(opponent->login, winPkt);
        
        opponent->inGame = false;
        opponent->gameName = "";
        opponent->opponent = "";
        opponent->isTurn = false;
        
        updateStatsAfterGame(opponent->login, player->login);
      }
    } else {
      std::cout << "[Game Cancelled] " << player->login << " cancelled " << room->name << std::endl;
      
      auto it = std::remove_if(gameRooms.begin(), gameRooms.end(),
                             [&](const GameRoom &r) { return r.name == room->name; });
      gameRooms.erase(it, gameRooms.end());
    }
  }
  
  player->inGame = false;
  player->gameName = "";
  player->opponent = "";
  player->isTurn = false;
  
  Packet resp;
  resp.type = S_MSG;
  strcpy(resp.payload, "You left the game.");
  sendToClient(pkt.sender, resp);
  
  sendGameList(pkt.sender);
}

void ServerApp::startGame(GameRoom &room) {
  Player *player1 = findPlayer(room.player1);
  Player *player2 = findPlayer(room.player2);
  
  if (!player1 || !player2) {
    return;
  }
  
  room.isActive = true;
  
  player1->opponent = player2->login;
  player1->board.placeShipsRandomly();
  player1->isTurn = false;
  
  player2->opponent = player1->login;
  player2->board.placeShipsRandomly();
  player2->isTurn = true;
  
  Packet start;
  start.type = S_GAME_START;
  
  strcpy(start.payload, player2->login.c_str());
  strcat(start.payload, " (Wait for opponent turn)");
  sendToClient(player1->login, start);
  
  strcpy(start.payload, player1->login.c_str());
  strcat(start.payload, " (YOUR TURN)");
  sendToClient(player2->login, start);
  
  sendBoard(player1, player1->board, true, "YOUR BOARD:");
  sendBoard(player2, player2->board, true, "YOUR BOARD:");
  
  std::cout << "[Game Start] " << room.name << ": " << player1->login << " vs " << player2->login << std::endl;
  
  auto it = std::remove_if(gameRooms.begin(), gameRooms.end(),
                         [&](const GameRoom &r) { return r.name == room.name; });
  gameRooms.erase(it, gameRooms.end());
  
  for (const auto &p : players) {
    if (!p.inGame) {
      sendGameList(p.login);
    }
  }
}

void ServerApp::handleShoot(Packet &pkt) {
  Player *shooter = findPlayer(pkt.sender);

  if (!shooter) {
    std::cout << "Shooter not found: " << pkt.sender << "\n";
    return;
  }
  if (!shooter->inGame) {
    std::cout << "Shooter not in game: " << pkt.sender << "\n";
    return;
  }
  if (shooter->opponent.empty()) {
    std::cout << "Opponent string empty for " << pkt.sender << "\n";
    return;
  }

  if (!shooter->isTurn) {
    Packet err;
    err.type = S_MSG;
    strcpy(err.payload, "Now is NOT your turn. Wait for opponent.");
    sendToClient(shooter->login, err);
    return;
  }

  Player *victim = findPlayer(shooter->opponent);
  if (!victim) {
    std::cout << "Victim not found: " << shooter->opponent << "\n";
    return;
  }

  ShotResult res = victim->board.processShot(pkt.x, pkt.y);
  
  PlayerStats *shooterStats = getPlayerStats(shooter->login);
  shooterStats->totalShots++;
  if (res == RES_HIT || res == RES_LOSE) {
    shooterStats->hits++;
  }

  if (res == RES_REPEAT) {
    Packet err;
    err.type = S_MSG;
    strcpy(err.payload,
           "You have already shot here or the coordinates are wrong. Repeat.");
    sendToClient(shooter->login, err);
    return;
  }

  std::cout << "[Shoot] " << shooter->login << " at (" << pkt.x << ", " << pkt.y
            << ")" << std::endl;

  if (res == RES_LOSE) {
    Packet pktWin;
    pktWin.type = S_GAME_OVER;
    strcpy(pktWin.payload, "WIN! You destroy all opponent's ships\n");
    sendToClient(shooter->login, pktWin);

    Packet pktLose;
    pktLose.type = S_GAME_OVER;
    strcpy(pktLose.payload, "LOSE! Your ships destroyed\n");
    sendToClient(victim->login, pktLose);

    shooter->inGame = false;
    shooter->gameName = "";
    shooter->opponent = "";
    shooter->isTurn = false;

    victim->inGame = false;
    victim->gameName = "";
    victim->opponent = "";
    victim->isTurn = false;
    
    updateStatsAfterGame(shooter->login, victim->login);

    std::cout << "[Game Over] Winner:" << shooter->login << "\n";
    return;
  }

  Packet respShooter;
  respShooter.type = S_SHOT_RESULT;
  respShooter.x = pkt.x;
  respShooter.y = pkt.y;
  respShooter.shotResult = (int)res;

  if (res == RES_HIT) {
    strcpy(respShooter.payload, "HIT! Shoot again!");
  } else {
    strcpy(respShooter.payload, "MISS. Change turn...");
  }

  sendToClient(shooter->login, respShooter);

  sendBoard(shooter, victim->board, false, "Opponent's board (Radar):");

  Packet respVictim = respShooter;
  if (res == RES_HIT) {
    strcpy(respVictim.payload, "Your ship has been HIT! Opponent's turn...");
  } else {
    strcpy(respVictim.payload, "Opponent MISS! Your turn!");
  }

  sendToClient(victim->login, respVictim);

  sendBoard(victim, victim->board, true, "Your board:");

  if (res == RES_MISS) {
    shooter->isTurn = false;
    victim->isTurn = true;
  }
}

void ServerApp::handleGetStats(Packet &pkt) {
  PlayerStats *stats = getPlayerStats(pkt.sender);
  
  Packet resp;
  resp.type = S_STATS;
  strcpy(resp.sender, "SERVER");
  
  std::stringstream ss;
  ss << "Statistics for " << pkt.sender << ":\n";
  ss << "================\n";
  ss << "Games played: " << stats->gamesPlayed << "\n";
  ss << "Wins: " << stats->wins << "\n";
  ss << "Losses: " << stats->losses << "\n";
  ss << "Win rate: " << (stats->gamesPlayed > 0 ? 
                        (double)stats->wins / stats->gamesPlayed * 100 : 0) << "%\n";
  ss << "Total shots: " << stats->totalShots << "\n";
  ss << "Hits: " << stats->hits << "\n";
  ss << "Accuracy: " << stats->accuracy << "%\n";
  
  strcpy(resp.payload, ss.str().c_str());
  sendToClient(pkt.sender, resp);
}

void ServerApp::handleLogout(Packet &pkt) {
  Player *quittingPlayer = findPlayer(pkt.sender);

  if (quittingPlayer && quittingPlayer->inGame) {
    Player *opponent = findPlayer(quittingPlayer->opponent);
    if (opponent) {
      Packet winPkt;
      winPkt.type = S_GAME_OVER;
      strcpy(winPkt.payload, "Opponent left the game.\n YOU WON!");
      sendToClient(opponent->login, winPkt);

      opponent->inGame = false;
      opponent->gameName = "";
      opponent->opponent = "";
      opponent->isTurn = false;
      
      updateStatsAfterGame(opponent->login, quittingPlayer->login);
    }
  }

  auto it =
      std::remove_if(players.begin(), players.end(),
                     [&](const Player &p) { return p.login == pkt.sender; });

  if (it != players.end()) {
    players.erase(it, players.end());
    std::cout << "[Logout] Player " << pkt.sender
              << " removed from server's list.\n";
  }
}

void ServerApp::run() {
  serverPipe.removePipe();
  if (!serverPipe.create()) {
    std::cerr << "Fatal: Unable to create server pipe. Check access rights."
              << std::endl;
    return;
  }

  if (!serverPipe.openPipe(O_RDWR)) {
    std::cerr << "Fatal: Unable to open pipe." << std::endl;
    return;
  }

  std::cout << "Server running. Waiting..." << std::endl;

  Packet pkt;
  while (serverPipe.receive(&pkt, sizeof(Packet))) {
    pthread_mutex_lock(&list_mutex);

    switch (pkt.type) {
    case LOGIN:
      handleLogin(pkt);
      break;
    case CREATE_GAME:
      handleCreateGame(pkt);
      break;
    case JOIN_GAME:
      handleJoinGame(pkt);
      break;
    case LEAVE_GAME:
      handleLeaveGame(pkt);
      break;
    case SHOOT:
      handleShoot(pkt);
      break;
    case LOGOUT:
      handleLogout(pkt);
      break;
    case GET_STATS:
      handleGetStats(pkt);
      break;
    case GET_GAME_LIST:
      sendGameList(pkt.sender);
      break;
    }

    pthread_mutex_unlock(&list_mutex);
  }

  serverPipe.closePipe();
  serverPipe.removePipe();
}
