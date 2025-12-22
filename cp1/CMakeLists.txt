cmake_minimum_required(VERSION 3.10)
project(BattleshipPipes)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

include_directories(include)

add_executable(server 
    src/server/server_main.cpp 
    src/server/ServerApp.cpp
    src/game/GameLogic.cpp
)
target_link_libraries(server Threads::Threads)

add_executable(client 
    src/client/client_main.cpp 
    src/client/ClientApp.cpp
)
target_link_libraries(client Threads::Threads)


