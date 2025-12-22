#include "ClientApp.h"

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sstream>

ClientApp::ClientApp() : isRunning(true), inGame(false) {}

void *ClientApp::listenThreadWrapper(void *context) {
  ((ClientApp *)context)->listenLoop();
  return nullptr;
}

void ClientApp::listenLoop() {
  std::string pipePath = CLIENT_PIPE_PREFIX + login;
  NamedPipe myPipe(pipePath);

  myPipe.removePipe();
  if (!myPipe.create() || !myPipe.openPipe(O_RDWR)) {
    std::cerr << "\n[Error] Could not create a personal channel to receive "
                 "messages.\n";
    return;
  }

  Packet pkt;
  while (isRunning) {
    if (myPipe.receive(&pkt, sizeof(Packet))) {
      switch (pkt.type) {
      case S_MSG:
        std::cout << "\n[SERVER]: " << pkt.payload << "\n" << std::flush;
        if (!inGame) std::cout << "> " << std::flush;
        break;
      case S_GAME_LIST:
        std::cout << "\n" << pkt.payload << "\n" << std::flush;
        if (!inGame) std::cout << "> " << std::flush;
        break;
      case S_GAME_CREATED:
        std::cout << "\n[GAME]: " << pkt.payload << "\n" << std::flush;
        currentGame = pkt.sender;
        std::cout << "> " << std::flush;
        break;
      case S_GAME_START:
        std::cout << "\n[GAME]: GAME HAS BEEN STARTED! Opponent: "
                  << pkt.payload
                  << "\n[GAME]: Your ships are automatically spaced."
                  << "\n[GAME]: Enter '/shoot X Y' (0-9)\n"
                  << std::flush;
        inGame = true;
        std::cout << "> " << std::flush;
        break;
      case S_BOARD:
        std::cout << "\n" << pkt.payload << "\n" << std::flush;
        std::cout << "> " << std::flush;
        break;
      case S_SHOT_RESULT:
        std::cout << "\n[RESULT]: " << pkt.payload << " (" << pkt.x << ", "
                  << pkt.y << ")\n" << std::flush;
        std::cout << "> " << std::flush;
        break;
      case S_GAME_OVER:
        std::cout << "\n\n====================================\n";
        std::cout << "               GAME OVER                \n";
        std::cout << "=======================================\n";
        std::cout << pkt.payload << "\n";
        std::cout << "=======================================\n";
        inGame = false;
        currentGame = "";
        showMainMenu();
        break;
      case S_STATS:
        std::cout << "\n" << pkt.payload << "\n" << std::flush;
        if (!inGame) std::cout << "> " << std::flush;
        break;
      }
    }
  }
  myPipe.closePipe();
  myPipe.removePipe();
}

void ClientApp::sendPacket(Packet &pkt) {
  NamedPipe serverPipe(SERVER_PIPE);
  if (serverPipe.openPipe(O_WRONLY)) {
    serverPipe.send(&pkt, sizeof(Packet));
    serverPipe.closePipe();
  } else {
    std::cout << "[Error] Server not available (not running).\n";
  }
}

void ClientApp::showMainMenu() {
  std::cout << "\nMain Menu\n";
  std::cout << "Commands:\n";
  std::cout << "  /create <name>   - Create new game\n";
  std::cout << "  /join <name>     - Join existing game\n";
  std::cout << "  /list            - Show available games\n";
  std::cout << "  /stats           - Show your statistics\n";
  std::cout << "  /quit            - Quit\n";
  std::cout << "> " << std::flush;
}

void ClientApp::showGameMenu() {
  std::cout << "\nGame Menu\n";
  std::cout << "Commands:\n";
  std::cout << "  /shoot <x> <y>   - Make a shot (0-9)\n";
  std::cout << "  /leave           - Leave current game\n";
  std::cout << "> " << std::flush;
}

void ClientApp::start() {
  std::cout << "=== SEA FIGHT CLIENT ===\n";
  std::cout << "Enter your login: ";
  std::cin >> login;

  if (pthread_create(&listenerThread, NULL, listenThreadWrapper, this) != 0) {
    std::cerr << "Error of creating the thread.\n";
    return;
  }
  sleep(1);

  Packet auth;
  auth.type = LOGIN;
  strcpy(auth.sender, login.c_str());
  sendPacket(auth);

  sleep(1);
  showMainMenu();

  std::string cmd;
  while (isRunning) {
    std::cin >> cmd;
    
    Packet pkt;
    memset(&pkt, 0, sizeof(Packet));
    strcpy(pkt.sender, login.c_str());

    if (cmd == "/quit") {
      pkt.type = LOGOUT;
      sendPacket(pkt);
      isRunning = false;
    } else if (cmd == "/create") {
      if (inGame) {
        std::cout << "You are already in a game! Use /leave first.\n";
        showMainMenu();
        continue;
      }
      std::string gameName;
      std::cin >> gameName;
      pkt.type = CREATE_GAME;
      strcpy(pkt.gameName, gameName.c_str());
      sendPacket(pkt);
    } else if (cmd == "/join") {
      if (inGame) {
        std::cout << "You are already in a game! Use /leave first.\n";
        showMainMenu();
        continue;
      }
      std::string gameName;
      std::cin >> gameName;
      pkt.type = JOIN_GAME;
      strcpy(pkt.gameName, gameName.c_str());
      sendPacket(pkt);
    } else if (cmd == "/list") {
      if (inGame) {
        std::cout << "You are in a game! Use /leave first.\n";
        showGameMenu();
        continue;
      }
      pkt.type = GET_GAME_LIST;
      sendPacket(pkt);
    } else if (cmd == "/stats") {
      pkt.type = GET_STATS;
      sendPacket(pkt);
    } else if (cmd == "/shoot") {
      if (!inGame) {
        std::cout << "You are not in a game!\n";
        showMainMenu();
        continue;
      }
      pkt.type = SHOOT;
      std::cin >> pkt.x >> pkt.y;
      sendPacket(pkt);
    } else if (cmd == "/leave") {
      if (!inGame && currentGame.empty()) {
        std::cout << "You are not in any game!\n";
        showMainMenu();
        continue;
      }
      pkt.type = LEAVE_GAME;
      sendPacket(pkt);
      inGame = false;
      currentGame = "";
      showMainMenu();
    } else {
      std::cout << "Invalid command.\n";
      if (inGame) {
        showGameMenu();
      } else {
        showMainMenu();
      }
    }
  }

  pthread_cancel(listenerThread);
  pthread_join(listenerThread, NULL);
}