#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <stdbool.h>

#include "maze_gen.h"

// #define DEBUG_GAME 1

#define HEIGHT_CONST 10
#define WIDTH_CONST 10

typedef struct {
    bool left;
    bool right;
    bool top;
    bool bottom;
} Walls;

typedef enum {
    EMPTY,
    WALL,
    PLAYER,
} Cell;

void drawRoom(Cell** room, int height, int width) {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            switch (room[i][j]) {
                case WALL:
                    mvaddch(i, j, '#');
                    break;
                case PLAYER:
                    mvaddch(i, j, '@');
                    break;
                case EMPTY:
                default:
                    mvaddch(i, j, '.');
                    break;
            }
        }
    }
}

void displayMinimap(int max_y, int max_x, int** maze, int mazeHeight, int mazeWidth, int finalCoords[2], int currCoords[2]) {
    for (int i = 0; i < mazeHeight; ++i) {
        for (int j = 0; j < mazeWidth; ++j) {
            if (maze[i][j] == 1) {
                mvaddch(i, j, '8');
            } else {
                if (finalCoords[0] == j && finalCoords[1] == i) {
                    mvaddch(i, j, 'O');
                } else if (currCoords[0] == j && currCoords[1] == i) {
                    mvaddch(i, j, '!');
                } else {
                    mvaddch(i, j, ' ');
                }
            }
        }
    }
}

void resetWalls(Walls* walls) {
    walls->left = false;
    walls->right = false;
    walls->top = false;
    walls->bottom = false;
}

void checkWalls(int** maze, int mazeHeight, int mazeWidth, int currHeight, int currWidth, Walls* currRoomsWalls) {
    resetWalls(currRoomsWalls);
    // check whether the neighbor rooms are walls
    if (currWidth <= 0 || maze[currHeight][currWidth - 1] == 1) {
        currRoomsWalls->left = true;
    }
    if (mazeWidth <= currWidth || currWidth >= mazeWidth || maze[currHeight][currWidth + 1] == 1) {
        currRoomsWalls->right = true;
    }
    if (currHeight <= 0 || maze[currHeight - 1][currWidth] == 1) {
        currRoomsWalls->top = true;
    }
    if (mazeHeight <= currHeight | currHeight >= mazeHeight || maze[currHeight + 1][currWidth] == 1) {
        currRoomsWalls->bottom = true;
    }
}

void fillRoom(Walls currRoomsWalls, Cell** room, int max_y, int max_x) {
    for (int i = 0; i < max_y; ++i) {
        for (int j = 0; j < max_x; ++j) {
            if (currRoomsWalls.left && j == 0) {
                room[i][j] = WALL;
            } else if (currRoomsWalls.right && j == max_x - 1) {
                room[i][j] = WALL;
            } else if (currRoomsWalls.bottom && i == max_y - 1) {
                room[i][j] = WALL;
            } else if (currRoomsWalls.top && i == 0) {
                room[i][j] = WALL;
            } else {
                room[i][j] = EMPTY;
            }
        }
    }
}

void changePlayerCoordinate(bool wall, int* axis, int* currRoomCoord, int max_coord, bool backward) {
    if (!wall || (*axis - 2 > 0 && backward) || (*axis < max_coord - 3 && !backward)) {
        if (backward) *axis -= 1;
        else *axis += 1;
    }

    if (!wall || (*axis - 1 > 0 && backward) || (*axis < max_coord - 2 && !backward)) {
        if (backward) *axis -= 1;
        else *axis += 1;
    }

    if (wall) {
        if (!backward && *axis >= max_coord - 1) {
            *axis = max_coord - 2;
            return;
        } else if (backward && *axis <= 1) {
            *axis = 1;
            return;
        }
    }

    if (*axis <= 0 || *axis >= max_coord - 1) {
        if (backward) {
            *currRoomCoord -= 1;
            *axis = max_coord - 3;
        } else {
            *currRoomCoord += 1;
            *axis = 1;
        }
    }
}

void stopLibrary() {
    endwin();
    echo();
    curs_set(1);
}

int main(int argc, char** argv) {
    int x = 1, y = 1;
    int ch;
    char param;
    int mazeHeight = HEIGHT_CONST, mazeWidth = WIDTH_CONST;
    int seed = (int)time(NULL);
    Cell** room;
    int** maze;
    Walls currRoomsWalls;
    resetWalls(&currRoomsWalls);
    int finalRoom[2], currRoomCoords[2] = {0, 0};
    bool minimap = false;
    int _temp_max_x, _temp_max_y;

    // process args
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            param = argv[i][1];
            if (param == 'r' || param == 'c' || param == 's' || param == 'm' || param == 'h') {
                switch (param) {
                    case 'h':
                        printf("This program starts a simple game where you need to find a specific room in a maze\n\n");
                        printf("Usage:\n    maze [options]\n\n");
                        printf("    use `wasd` to move and `q` to exit\n");
                        printf("Options:\n");
                        printf("    -c <cols>     Set the amount of columns (10 by defualt, range 4-64)\n");
                        printf("    -r <rows>     Set the amount of rows (10 by default, range 4-64)\n");
                        printf("    -s <seed>     Set a seed (time() by default, an integer value)\n");
                        printf("    -m <seed>     Show a minimap\n");
                        printf("    -h <help>     Display this page, do not print the maze\n");
                        return 0;
                    case 'm':
                        minimap = true;
                        break;
                }
                if (i + 1 < argc) {
                    switch (param) {
                        case 'r':
                            mazeHeight = atoi(argv[++i]);
                            if (mazeHeight < 4 || mazeHeight > 64) {
                                mazeHeight = HEIGHT_CONST;
                            }
                            break;
                        case 'c':
                            mazeWidth = atoi(argv[++i]);
                            if (mazeWidth < 4 || mazeWidth > 64) {
                                mazeWidth = WIDTH_CONST;
                            }
                            break;
                        case 's':
                            seed = atoi(argv[++i]);
                            break;
                    }
                }
            } else {
                printf("Error: unknown parameter '-%c'", param);
            }
        }
    }

    // ncurses init
    initscr();
    noecho();
    cbreak();
    raw();
    curs_set(0);

    // define room size
    getmaxyx(stdscr, _temp_max_y, _temp_max_x);
    const int max_x = _temp_max_x, max_y = _temp_max_y;

    // allocate memory
    room = (Cell**)malloc(max_y * sizeof(Cell*));
    for (int i = 0; i < max_y; ++i) {
        room[i] = (Cell*)malloc(max_x * sizeof(Cell));
    }
    maze = (int**)malloc(mazeHeight * sizeof(int*));
    for (int i = 0; i < mazeHeight; ++i) {
        maze[i] = (int*)malloc(mazeWidth * sizeof(int));
    }

    // generate a maze
    generateMaze(maze, mazeHeight, mazeWidth, seed, finalRoom);

    // check the neighbor rooms
    checkWalls(maze, mazeHeight, mazeWidth, currRoomCoords[1], currRoomCoords[0], &currRoomsWalls);

    // fill the cells
    fillRoom(currRoomsWalls, room, max_y, max_x);

    // init the player
    room[y][x] = PLAYER;
    drawRoom(room, max_y, max_x);
    if (minimap) {
        displayMinimap(max_y, max_x, maze, mazeHeight, mazeWidth, finalRoom, currRoomCoords);
    }
    refresh();

    // game loop
    // BUGS: doesn't update rooms
    //       occasional segfauls
    //       player can go inside a wall during switching to another room
    while ((ch = getch()) != 'q') {
        // return error if the resolution is less than the initial one
        getmaxyx(stdscr, _temp_max_y, _temp_max_x);
        if (_temp_max_x < max_x || _temp_max_y < max_y) {
            stopLibrary();
            printf("Error: the current resolution is smaller than the initial one!\n");
            return -1;
        }
        room[y][x] = EMPTY; // clear previous position

        switch (ch) {
            case 'w':
            case KEY_UP:
                changePlayerCoordinate(currRoomsWalls.top, &y, &currRoomCoords[1], max_y, true);
                checkWalls(maze, mazeHeight, mazeWidth, currRoomCoords[1], currRoomCoords[0], &currRoomsWalls);
                fillRoom(currRoomsWalls, room, max_y, max_x);
                break;
            case 's':
            case KEY_DOWN:
                changePlayerCoordinate(currRoomsWalls.bottom, &y, &currRoomCoords[1], max_y, false);
                checkWalls(maze, mazeHeight, mazeWidth, currRoomCoords[1], currRoomCoords[0], &currRoomsWalls);
                fillRoom(currRoomsWalls, room, max_y, max_x);
                break;
            case 'a':
            case KEY_LEFT:
                changePlayerCoordinate(currRoomsWalls.left, &x, &currRoomCoords[0], max_x, true);
                checkWalls(maze, mazeHeight, mazeWidth, currRoomCoords[1], currRoomCoords[0], &currRoomsWalls);
                fillRoom(currRoomsWalls, room, max_y, max_x);
                break;
            case 'd':
            case KEY_RIGHT:
                changePlayerCoordinate(currRoomsWalls.right, &x, &currRoomCoords[0], max_x, false);
                checkWalls(maze, mazeHeight, mazeWidth, currRoomCoords[1], currRoomCoords[0], &currRoomsWalls);
                fillRoom(currRoomsWalls, room, max_y, max_x);
                break;
        }

        if (y >= max_y || x >= max_x) {
            stopLibrary();

            printf("Error!!!\n");
            printf("max_y: %d, y: %d\n", max_y, y);
            printf("max_x: %d, x: %d\n", max_x, x);

            return -1;
        }
        if (finalRoom[0] == currRoomCoords[0] && finalRoom[1] == currRoomCoords[1]) {
            stopLibrary();
            printf("You won!\nCongratz!!!\n");
            return 0;
        }
        room[y][x] = PLAYER;
        drawRoom(room, max_y, max_x);
        if (minimap) {
            displayMinimap(max_y, max_x, maze, mazeHeight, mazeWidth, finalRoom, currRoomCoords);
        }
        refresh();
    }

    // free memory
    for (int i = 0; i < max_y; ++i) {
        free(room[i]);
    }
    free(room);
    for (int i = 0; i < mazeHeight; ++i) {
        free(maze[i]);
    }
    free(maze);

    // end the ncurses library
    stopLibrary();

    return 0;
}
