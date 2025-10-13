#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "raylib.h"

const int LEVEL_COUNT = 35;
const float SLIDING_TIME = 0.6;
const int FONT_SIZE = 40;
const float TITLE_SLIDE_TIME = 1;
const float IMMOBILE_TIME = 5;
const float GAME_OVER_SLIDE_TIME = 1;
const float STAGE_END_TIME = 3;
const float GAME_OVER_DELAY = 3;
const float STAGE_CURTAIN_TIME = 2;
const float STAGE_SUMMARY_SLIDE_TIME = 0.001;
const float TIMER_TIME = 15.0;
const float SHIELD_TIME = 15.0;
const float SHOVEL_TIME = 15.0;
const int POWERUP_SCORE = 500;
const int MAX_POWERUP_COUNT = 3;
const int STAGE_COUNT = 16;
const int FIELD_COLS = 64;
const int FIELD_ROWS = 56;
const int CELL_SIZE = 16;
const int SCREEN_WIDTH = FIELD_COLS * CELL_SIZE;
const int SCREEN_HEIGHT = FIELD_ROWS * CELL_SIZE;
const int SNAP_TO = CELL_SIZE * 2;
const int TANK_SIZE = CELL_SIZE * 4;
const int TANK_TEXTURE_SIZE = 16;
const int FLAG_SIZE = TANK_SIZE;
const Vector2 POWER_UP_TEXTURE_SIZE = {30, 28};
const int POWER_UP_SIZE = CELL_SIZE * 4;
const int SPAWN_TEXTURE_SIZE = 32;
const int UI_TANK_TEXTURE_SIZE = 14;
const int UI_TANK_SIZE = CELL_SIZE * 2;
const int PLAYER1_START_COL = 4 * 4 + 4;
const int PLAYER2_START_COL = 4 * 8 + 4;
const Vector2 PLAYER1_START_POS = {CELL_SIZE * PLAYER1_START_COL,
                                   CELL_SIZE *(FIELD_ROWS - 4 - 2)};
const Vector2 PLAYER2_START_POS = {CELL_SIZE * PLAYER2_START_COL,
                                   CELL_SIZE *(FIELD_ROWS - 4 - 2)};
const int PLAYER_SPEED = 220;
const short ENEMY_SPEEDS[3] = {140, 170, 240};
const int MAX_ENEMY_COUNT = 1;
const int MAX_TANK_COUNT = MAX_ENEMY_COUNT + 2;
const int MAX_BULLET_COUNT = 100;
const int MAX_EXPLOSION_COUNT = MAX_BULLET_COUNT;
const int MAX_SCORE_POPUP_COUNT = MAX_BULLET_COUNT;
const Vector2 SCORE_POPUP_TEXTURE_SIZE = (Vector2){16, 9};
const Vector2 SCORE_POPUP_SIZE = (Vector2){16 * 4, 9 * 4};
const float SCORE_POPUP_TTL = 0.5;
const short BULLET_SPEEDS[3] = {450, 550, 600};
const int BULLET_SIZE = 16;
const float BULLET_EXPLOSION_TTL = 0.2f;
const float BIG_EXPLOSION_TTL = 0.4f;
const float ENEMY_SPAWN_INTERVAL = 3.0f;
const float SPAWNING_TIME = 0.7f;
const int POWERUP_POSITIONS_COUNT = 16;
const Vector2 POWERUP_POSITIONS[POWERUP_POSITIONS_COUNT] = {
    {(4 * 4 + 2 + 4) * CELL_SIZE, (7 * 4 + 2 + 2) * CELL_SIZE},
    {(4 * 4 + 2 + 4) * CELL_SIZE, (4 * 4 + 2 + 2) * CELL_SIZE},
    {(7 * 4 + 2 + 4) * CELL_SIZE, (7 * 4 + 2 + 2) * CELL_SIZE},
    {(7 * 4 + 2 + 4) * CELL_SIZE, (4 * 4 + 2 + 2) * CELL_SIZE},

    {(1 * 4 + 2 + 4) * CELL_SIZE, (7 * 4 + 2 + 2) * CELL_SIZE},
    {(1 * 4 + 2 + 4) * CELL_SIZE, (4 * 4 + 2 + 2) * CELL_SIZE},
    {(10 * 4 + 2 + 4) * CELL_SIZE, (7 * 4 + 2 + 2) * CELL_SIZE},
    {(10 * 4 + 2 + 4) * CELL_SIZE, (4 * 4 + 2 + 2) * CELL_SIZE},

    {(1 * 4 + 2 + 4) * CELL_SIZE, (1 * 4 + 2 + 2) * CELL_SIZE},
    {(1 * 4 + 2 + 4) * CELL_SIZE, (10 * 4 + 2 + 2) * CELL_SIZE},
    {(10 * 4 + 2 + 4) * CELL_SIZE, (1 * 4 + 2 + 2) * CELL_SIZE},
    {(10 * 4 + 2 + 4) * CELL_SIZE, (10 * 4 + 2 + 2) * CELL_SIZE},

    {(4 * 4 + 2 + 4) * CELL_SIZE, (1 * 4 + 2 + 2) * CELL_SIZE},
    {(7 * 4 + 2 + 4) * CELL_SIZE, (10 * 4 + 2 + 2) * CELL_SIZE},
    {(4 * 4 + 2 + 4) * CELL_SIZE, (1 * 4 + 2 + 2) * CELL_SIZE},
    {(7 * 4 + 2 + 4) * CELL_SIZE, (10 * 4 + 2 + 2) * CELL_SIZE}};

#define PORT 5000
#define BROADCAST_IP "255.255.255.255"
#define BUFFER_SIZE 256
#define CLIENT_INPUT_SIZE 6

const int MAX_AVAILABLE_GAMES = 4;

#endif