cmake_minimum_required (VERSION 3.5)
project (switchboard)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Werror -Wextra -Wall")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(SDL2_mixer REQUIRED)
include_directories(${SDL2_INCLUDE_DIR}
                    ${SDL2_IMAGE_INCLUDE_DIR}
                    ${SDL2_TTF_INCLUDE_DIR}
                    ${SDL_MIXER_INCLUDE_DIR})

add_executable(
    ${PROJECT_NAME}
    switchboard.c game.c gamestate.c menu_main.c menu_pause.c util.c endgame.c)
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY}
                                      ${SDL2_IMAGE_LIBRARIES}
                                      ${SDL2_TTF_LIBRARIES}
                                      ${SDL_MIXER_LIBRARIES}
                                      m)
