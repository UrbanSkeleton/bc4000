#ifndef GAME_PACKAGER_H
#define GAME_PACKAGER_H

#include "constants.h"
#include "dataTypes.h"

typedef struct {
    uint8_t type;
    uint16_t x;
    uint16_t y;
    uint8_t direction;
    uint8_t status;
} GameStateTank;

typedef struct {
    uint8_t type;
} GameStateCell;

typedef struct {
    GameStateTank tanks[MAX_TANK_COUNT];
    GameStateCell field[FIELD_ROWS][FIELD_COLS];
    uint16_t stageCurtainTime;
} GameStatePacket;

const int MAX_PACKET_SIZE = sizeof(GameStatePacket);

static void packTank(Tank* tank, GameStateTank* gameStateTank) {
    gameStateTank->type = (uint8_t)tank->type;
    gameStateTank->x = (uint16_t)tank->pos.x;
    gameStateTank->y = (uint16_t)tank->pos.y;
    gameStateTank->direction = (uint8_t)tank->direction;
    gameStateTank->status = (uint8_t)tank->status;
}

static void packField(Cell field[FIELD_ROWS][FIELD_COLS],
                      GameStateCell gameStateField[FIELD_ROWS][FIELD_COLS]) {
    for (int y = 0; y < FIELD_ROWS; y++) {
        for (int x = 0; x < FIELD_COLS; x++) {
            gameStateField[y][x].type = (uint8_t)(field[y][x].type);
        }
    }
}

static size_t packGameState(Game* game, char* buffer) {
    GameStatePacket packet;
    memset(&packet, 0, sizeof(packet));

    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        packTank(&game->tanks[i], &packet.tanks[i]);
    }

    packField(game->field, packet.field);
    packet.stageCurtainTime = (uint16_t)game->stageCurtainTime;

    memcpy(buffer, &packet, sizeof(packet));
    return sizeof(packet);
}

static void unpackTank(GameStateTank* gameStateTank, Tank* tank) {
    tank->type = (TankType)gameStateTank->type;
    tank->pos.x = (float)gameStateTank->x;
    tank->pos.y = (float)gameStateTank->y;
    tank->direction = (Direction)gameStateTank->direction;
    tank->status = (TankStatus)gameStateTank->status;
}

static void unpackField(GameStateCell gameStateField[FIELD_ROWS][FIELD_COLS],
                        Cell field[FIELD_ROWS][FIELD_COLS]) {
    for (int y = 0; y < FIELD_ROWS; y++) {
        for (int x = 0; x < FIELD_COLS; x++) {
            field[y][x].type = (CellType)gameStateField[y][x].type;
        }
    }
}

static void unpackGameState(Game* game, char* buffer) {
    GameStatePacket packet;
    memcpy(&packet, buffer, sizeof(packet));

    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        unpackTank(&packet.tanks[i], &game->tanks[i]);
    }

    unpackField(packet.field, game->field);
    game->stageCurtainTime = (float)packet.stageCurtainTime;
}

#endif