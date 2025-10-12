#ifndef GAME_PACKAGER_H
#define GAME_PACKAGER_H

#include "constants.h"
#include "dataTypes.h"
#include "utils.h"

typedef struct {
    uint8_t type;
    uint16_t x;
    uint16_t y;
    uint8_t direction;
    uint8_t status;
    uint8_t shieldTimeLeft;
    uint8_t immobileTimeLeft;
    uint8_t texColOffset;
} GameStateTank;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t direction;
    uint8_t type;
} GameStateBullet;

typedef struct {
    uint8_t type;
} GameStateCell;

typedef struct {
    uint8_t type;
    uint16_t x;
    uint16_t y;
    uint8_t state;
} GameStatePowerUp;

typedef struct {
    uint8_t type;
    uint16_t x;
    uint16_t y;
    uint8_t ttl;
    uint8_t maxTtl;
} GameStateExplosion;

typedef struct {
    GameStateTank tanks[MAX_TANK_COUNT];
    GameStateBullet bullets[MAX_BULLET_COUNT];
    GameStateCell field[FIELD_ROWS][FIELD_COLS];
    GameStatePowerUp powerUps[MAX_POWERUP_COUNT];
    GameStateExplosion explosions[MAX_EXPLOSION_COUNT];
    uint8_t stageCurtainTime;
    uint8_t pendingEnemyCount;
    uint8_t lifes[2];
    float stageSummaryTime;
    int hiScore;
    uint8_t stage;
    uint8_t screen;
} GameStatePacket;

const int MAX_PACKET_SIZE = sizeof(GameStatePacket);

static void packTank(Tank* tank, GameStateTank* gameStateTank) {
    gameStateTank->type = (uint8_t)tank->type;
    gameStateTank->x = (uint16_t)tank->pos.x;
    gameStateTank->y = (uint16_t)tank->pos.y;
    gameStateTank->direction = (uint8_t)tank->direction;
    gameStateTank->status = (uint8_t)tank->status;
    gameStateTank->shieldTimeLeft =
        (uint8_t)MAX(0, (tank->shieldTimeLeft * 256.0 / SHIELD_TIME));
    gameStateTank->immobileTimeLeft =
        (uint8_t)MAX(0, (tank->immobileTimeLeft * 256.0 / IMMOBILE_TIME));
    gameStateTank->texColOffset = (uint8_t)tank->texColOffset;
}

static void packBullet(Bullet* bullet, GameStateBullet* gameStateBullet) {
    gameStateBullet->x = (uint16_t)bullet->pos.x;
    gameStateBullet->y = (uint16_t)bullet->pos.y;
    gameStateBullet->direction = (uint8_t)bullet->direction;
    gameStateBullet->type = (uint8_t)bullet->type;
}

static void packField(Cell field[FIELD_ROWS][FIELD_COLS],
                      GameStateCell gameStateField[FIELD_ROWS][FIELD_COLS]) {
    for (int y = 0; y < FIELD_ROWS; y++) {
        for (int x = 0; x < FIELD_COLS; x++) {
            gameStateField[y][x].type = (uint8_t)(field[y][x].type);
        }
    }
}

static void packPowerUp(PowerUp* powerUp, GameStatePowerUp* gameStatePowerUp) {
    gameStatePowerUp->type = (uint8_t)powerUp->type;
    gameStatePowerUp->x = (uint16_t)powerUp->pos.x;
    gameStatePowerUp->y = (uint16_t)powerUp->pos.y;
    gameStatePowerUp->state = (uint8_t)powerUp->state;
}

static void packExplosion(Explosion* explosion,
                          GameStateExplosion* gameStateExplosion) {
    gameStateExplosion->type = (uint8_t)explosion->type;
    gameStateExplosion->x = (uint16_t)explosion->pos.x;
    gameStateExplosion->y = (uint16_t)explosion->pos.y;
    gameStateExplosion->ttl = (uint8_t)(explosion->ttl * 64);
    // gameStateExplosion->maxTtl = (uint8_t)(explosion->maxTtl * 64);
}

static size_t packGameState(Game* game, char* buffer) {
    GameStatePacket packet;
    memset(&packet, 0, sizeof(packet));

    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        packTank(&game->tanks[i], &packet.tanks[i]);
    }

    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        packBullet(&game->bullets[i], &packet.bullets[i]);
    }

    for (int i = 0; i < MAX_POWERUP_COUNT; i++) {
        packPowerUp(&game->powerUps[i], &packet.powerUps[i]);
    }

    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        packExplosion(&game->explosions[i], &packet.explosions[i]);
    }

    packField(game->field, packet.field);

    packet.stageCurtainTime = (game->stageCurtainTime * 64.0);
    packet.pendingEnemyCount = game->pendingEnemyCount;
    packet.lifes[0] = game->tanks[0].lifes;
    packet.lifes[1] = game->tanks[1].lifes;
    packet.hiScore = game->hiScore;
    packet.stage = game->stage;
    packet.stageSummaryTime = game->stageSummary.time;
    packet.screen = game->screen;

    memcpy(buffer, &packet, sizeof(packet));

    return sizeof(packet);
}

static void unpackTank(Tank* tank, GameStateTank* gameStateTank) {
    tank->type = (TankType)gameStateTank->type;
    tank->pos.x = (float)gameStateTank->x;
    tank->pos.y = (float)gameStateTank->y;
    tank->direction = (Direction)gameStateTank->direction;
    tank->status = (TankStatus)gameStateTank->status;
    tank->shieldTimeLeft =
        (float)(gameStateTank->shieldTimeLeft) / 256.0 * SHIELD_TIME;
    tank->immobileTimeLeft =
        (float)(gameStateTank->immobileTimeLeft) / 256.0 * IMMOBILE_TIME;
    tank->texColOffset = (char)gameStateTank->texColOffset;
}

static void unpackBullet(Bullet* bullet, GameStateBullet* gameStateBullet) {
    bullet->pos.x = (float)gameStateBullet->x;
    bullet->pos.y = (float)gameStateBullet->y;
    bullet->direction = (Direction)gameStateBullet->direction;
    bullet->type = (BulletType)gameStateBullet->type;
}

static void unpackField(Cell field[FIELD_ROWS][FIELD_COLS],
                        GameStateCell gameStateField[FIELD_ROWS][FIELD_COLS]) {
    for (int y = 0; y < FIELD_ROWS; y++) {
        for (int x = 0; x < FIELD_COLS; x++) {
            field[y][x].type = (CellType)gameStateField[y][x].type;
        }
    }
}

static void unpackPowerUp(PowerUp* powerUp,
                          GameStatePowerUp* gameStatePowerUp) {
    powerUp->type = (PowerUpType)gameStatePowerUp->type;
    powerUp->pos.x = (float)gameStatePowerUp->x;
    powerUp->pos.y = (float)gameStatePowerUp->y;
    powerUp->state = (PowerUpState)gameStatePowerUp->state;
}

static void unpackExplosion(Explosion* explosion,
                            GameStateExplosion* gameStateExplosion) {
    explosion->type = (ExplosionType)gameStateExplosion->type;
    explosion->pos.x = (float)gameStateExplosion->x;
    explosion->pos.y = (float)gameStateExplosion->y;
    explosion->ttl = (float)(gameStateExplosion->ttl / 64.0);
    // explosion->maxTtl = (float)(gameStateExplosion->maxTtl / 64.0);
}
static GameStatePacket unpackGameState(Game* game, char* buffer) {
    GameStatePacket packet;
    memcpy(&packet, buffer, sizeof(packet));

    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        unpackTank(&game->tanks[i], &packet.tanks[i]);
    }

    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        unpackBullet(&game->bullets[i], &packet.bullets[i]);
    }

    for (int i = 0; i < MAX_POWERUP_COUNT; i++) {
        unpackPowerUp(&game->powerUps[i], &packet.powerUps[i]);
    }

    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        unpackExplosion(&game->explosions[i], &packet.explosions[i]);
    }

    unpackField(game->field, packet.field);

    game->stageCurtainTime = ((float)packet.stageCurtainTime) / 64.0;
    game->pendingEnemyCount = packet.pendingEnemyCount;
    game->tanks[0].lifes = packet.lifes[0];
    game->tanks[1].lifes = packet.lifes[1];
    game->hiScore = packet.hiScore;
    game->stage = packet.stage;
    game->stageSummary.time = packet.stageSummaryTime;

    return packet;
}

#endif