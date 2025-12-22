#pragma once

#include "protocol.h"
#include "wrappers.h"

#include <pthread.h>
#include <string>

class ClientApp {
public:
  ClientApp();
  ~ClientApp() = default;
  void start();

private:
  std::string login;
  std::string currentGame;
  bool isRunning;
  bool inGame;
  pthread_t listenerThread;

  static void *listenThreadWrapper(void *context);
  void listenLoop();

  void sendPacket(Packet &pkt);
  void showMainMenu();
  void showGameMenu();
};