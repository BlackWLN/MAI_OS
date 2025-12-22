#include "ServerApp.h"

#include <cstdlib>
#include <ctime>

int main() {
  std::srand(std::time(nullptr));
  ServerApp server;
  server.run();
  return 0;
}