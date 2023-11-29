#include "raylib.h"
#include "raymath.h"
#include <assert.h>
#include <string.h>
#include <time.h>

#include "utils.c"

// #define DRAW_CELL_GRID

#define ASIZE(a) (sizeof(a) / sizeof(a[0]))

const int POWERUP_SCORE = 500;
const int MAX_POWERUP_COUNT = 3;
const int STAGE_COUNT = 2;
const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 900;
const int FIELD_COLS = 64;
const int FIELD_ROWS = 56;
const int CELL_SIZE = 16;
const int SNAP_TO = CELL_SIZE * 2;
const int TANK_SIZE = CELL_SIZE * 4;
const int FLAG_SIZE = TANK_SIZE;
const int TANK_TEXTURE_SIZE = 16;
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
const int MAX_ENEMY_COUNT = 20;
const int MAX_TANK_COUNT = MAX_ENEMY_COUNT + 2;
const int MAX_BULLET_COUNT = 100;
const int MAX_EXPLOSION_COUNT = MAX_BULLET_COUNT;
const short BULLET_SPEEDS[3] = {400, 450, 500};
const int BULLET_SIZE = 16;
const float BULLET_EXPLOSION_TTL = 0.2f;
const float BIG_EXPLOSION_TTL = 0.4f;
const float ENEMY_SPAWN_INTERVAL = 2.0f;
const float SPAWNING_TIME = 1.0f;
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

typedef enum {
    TPlayer1,
    TPlayer2,
    TBasic,
    TFast,
    TPower,
    TArmor,
    TMax
} TankType;
typedef enum { TSPending, TSSpawning, TSActive, TSDead } TankStatus;
typedef enum { DLeft, DRight, DUp, DDown } Direction;
typedef enum {
    UIFlag,
    UIPlayer1,
    UIPlayer2,
    UIP1Tank,
    UIP1Lifes,
    UIP2Tank,
    UIP2Lifes,
    UIStageLowDigit,
    UIStageHiDigit,
    UIMax
} UIElementType;

typedef struct {
    int totalScore;
    char kills[TMax];
} PlayerScore;

typedef struct {
    Texture2D *texture;
    Rectangle textureSrc;
    Vector2 pos;
    Vector2 size;
    Vector2 drawSize;
    bool isVisible;
} UIElement;

typedef struct {
    Texture2D *texture;
    Texture2D *powerUpTexture;
    short speed;
    short bulletSpeed;
    char maxBulletCount;
    bool isEnemy;
    short points;
    char texRow;
} TankSpec;

typedef enum {
    PUStar,
    PUTank,
    PUGrenade,
    PUHelmet,
    PUShovel,
    PUTimer,
    PUMax,
} PowerUpType;

typedef struct {
    Texture2D *texture;
    int texCol;
} PowerUpSpec;

typedef struct {
    PowerUpType type;
    Vector2 pos;
    bool isActive;
} PowerUp;

typedef struct {
    TankType type;
    Vector2 pos;
    Direction direction;
    char texColOffset;
    char firedBulletCount;
    bool isFiring;
    TankStatus status;
    float spawningTime;
    bool isMoving;
    char lifes;
    PowerUp *powerUp;
    PlayerScore *playerScore;
    char tier;
} Tank;

typedef struct {
    bool fire;
    bool move;
    Direction direction;
} Command;

typedef struct {
    float duration;
    Texture2D *textures;
    char textureCount;
} Animation;

typedef enum { ETBullet, ETBig, ETMax } ExplosionType;

typedef struct {
    ExplosionType type;
    Vector2 pos;
    float ttl;
    float maxTtl;
} Explosion;

typedef enum { BTNone, BTPlayer, BTEnemy } BulletType;

typedef struct {
    Vector2 pos;
    Vector2 speed;
    Direction direction;
    BulletType type;
    Tank *tank;
} Bullet;

typedef enum {
    CTBorder,
    CTBlank,
    CTBrick,
    CTConcrete,
    CTForest,
    CTRiver,
    CTMax
} CellType;

typedef struct {
    CellType type;
    Vector2 pos;
    char texRow;
    char texCol;
} Cell;

typedef struct {
    Texture2D *texture;
    bool isSolid;
    bool isPassable;
} CellSpec;

typedef struct {
    Texture2D flag;
    Texture2D brick;
    Texture2D border;
    Texture2D concrete;
    Texture2D forest;
    Texture2D river[2];
    Texture2D blank;
    Texture2D player1Tank;
    Texture2D player2Tank;
    Texture2D enemies;
    Texture2D enemiesWithPowerUps;
    Texture2D bullet;
    Texture2D bulletExplosions[3];
    Texture2D bigExplosions[5];
    Texture2D spawningTank;
    Texture2D uiFlag;
    Texture2D ui;
    Texture2D digits;
    Texture2D powerups;
} Textures;

typedef struct {
    Cell field[FIELD_ROWS][FIELD_COLS];
    Tank tanks[MAX_TANK_COUNT];
    TankSpec tankSpecs[TMax];
    Bullet bullets[MAX_BULLET_COUNT];
    Vector2 flagPos;
    CellSpec cellSpecs[CTMax];
    PowerUpSpec powerUpSpecs[PUMax];
    Animation explosionAnimations[ETMax];
    Explosion explosions[MAX_EXPLOSION_COUNT];
    Textures textures;
    float frameTime;
    float totalTime;
    float timeSinceSpawn;
    char activeEnemyCount;
    char pendingEnemyCount;
    char maxActiveEnemyCount;
    char stage;
    UIElement uiElements[UIMax];
    PlayerScore playerScores[2];
    PowerUp powerUps[MAX_POWERUP_COUNT];
} Game;

static Game game;

static void drawCell(Cell *cell) {
    Texture2D *tex = game.cellSpecs[cell->type].texture;
    int w = tex->width / 4;
    int h = tex->height / 4;
    DrawTexturePro(*tex, (Rectangle){cell->texCol * w, cell->texRow * h, w, h},
                   (Rectangle){cell->pos.x, cell->pos.y, CELL_SIZE, CELL_SIZE},
                   (Vector2){}, 0, WHITE);
#ifdef DRAW_CELL_GRID
    DrawRectangleLines(cell->pos.x, cell->pos.y, CELL_SIZE, CELL_SIZE, BLACK);
#endif
}

static void drawField() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (game.field[i][j].type != CTForest)
                drawCell(&game.field[i][j]);
        }
    }
}

static void drawForest() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (game.field[i][j].type == CTForest)
                drawCell(&game.field[i][j]);
        }
    }
}

static void drawTank(Tank *tank) {
    static char textureRows[4] = {1, 3, 0, 2};
    Texture2D *tex = !tank->powerUp || ((long)(game.totalTime * 8)) % 2
                         ? game.tankSpecs[tank->type].texture
                         : game.tankSpecs[tank->type].powerUpTexture;
    int texX = (textureRows[tank->direction] * 2 + tank->texColOffset) *
               TANK_TEXTURE_SIZE;
    int texY = game.tankSpecs[tank->type].texRow * TANK_TEXTURE_SIZE;
    int drawSize = TANK_TEXTURE_SIZE * 4;
    int drawOffset = (TANK_SIZE - drawSize) / 2;
    DrawTexturePro(
        *tex, (Rectangle){texX, texY, TANK_TEXTURE_SIZE, TANK_TEXTURE_SIZE},
        (Rectangle){tank->pos.x + drawOffset, tank->pos.y + drawOffset,
                    drawSize, drawSize},
        (Vector2){}, 0, WHITE);
}

static void drawSpawningTank(Tank *tank) {
    static char textureCols[] = {3, 2, 1, 0, 1, 2, 3, 2, 1, 0, 1, 2, 3};
    Texture2D *tex = &game.textures.spawningTank;
    int textureSize = tex->height;
    int i = tank->spawningTime / (SPAWNING_TIME / ASIZE(textureCols));
    if (i >= ASIZE(textureCols))
        i = ASIZE(textureCols) - 1;
    int texX = textureCols[i] * textureSize;
    int drawSize = SPAWN_TEXTURE_SIZE * 2;
    DrawTexturePro(*tex, (Rectangle){texX, 0, textureSize, textureSize},
                   (Rectangle){tank->pos.x, tank->pos.y, drawSize, drawSize},
                   (Vector2){}, 0, WHITE);
}

static void drawTanks() {
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        if (game.tanks[i].status == TSActive) {
            drawTank(&game.tanks[i]);
        } else if (game.tanks[i].status == TSSpawning) {
            drawSpawningTank(&game.tanks[i]);
        }
    }
}

static void drawFlag() {
    Texture2D *tex = &game.textures.flag;
    DrawTexturePro(
        *tex, (Rectangle){0, 0, tex->width, tex->height},
        (Rectangle){game.flagPos.x, game.flagPos.y, FLAG_SIZE, FLAG_SIZE},
        (Vector2){}, 0, WHITE);
}

static void drawBullets() {
    static int x[4] = {24, 8, 0, 16};
    Texture2D *tex = &game.textures.bullet;
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b = &game.bullets[i];
        if (b->type == BTNone)
            continue;
        DrawTexturePro(
            *tex, (Rectangle){x[b->direction], 0, 8, 8},
            (Rectangle){b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE},
            (Vector2){}, 0, WHITE);
    }
}

static void drawExplosions() {
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        Explosion *e = &game.explosions[i];
        if (e->ttl <= 0)
            continue;
        int texCount = game.explosionAnimations[e->type].textureCount;
        int index =
            e->ttl / (game.explosionAnimations[e->type].duration / texCount);
        if (index >= texCount)
            index = texCount - 1;
        Texture2D *tex =
            &game.explosionAnimations[e->type].textures[texCount - index - 1];
        DrawTexturePro(
            *tex, (Rectangle){0, 0, tex->width, tex->height},
            (Rectangle){e->pos.x, e->pos.y, tex->width * 2, tex->height * 2},
            (Vector2){}, 0, WHITE);
    }
}

static void drawUITanks() {
    Texture2D *tex = &game.textures.ui;
    int drawSize = UI_TANK_TEXTURE_SIZE * 2;
    int drawOffset = (UI_TANK_SIZE - drawSize) / 2;
    for (int i = 0; i < game.pendingEnemyCount; i++) {
        DrawTexturePro(
            *tex, (Rectangle){0, 0, UI_TANK_TEXTURE_SIZE, UI_TANK_TEXTURE_SIZE},
            (Rectangle){(14 * 4 + 2 + 2 * (i % 2)) * CELL_SIZE + drawOffset,
                        ((2 + 2) + (i / 2 * 2)) * CELL_SIZE + drawOffset,
                        drawSize, drawSize},
            (Vector2){}, 0, WHITE);
    }
}

static void drawUIElement(UIElement *el) {
    int drawOffsetX = (el->size.x - el->drawSize.x) / 2;
    int drawOffsetY = (el->size.y - el->drawSize.y) / 2;
    DrawTexturePro(*(el->texture), el->textureSrc,
                   (Rectangle){el->pos.x + drawOffsetX, el->pos.y + drawOffsetY,
                               el->drawSize.x, el->drawSize.y},
                   (Vector2){}, 0, WHITE);
}

static void drawUIElements() {
    for (int i = 0; i < UIMax; i++) {
        if (game.uiElements[i].isVisible)
            drawUIElement(&game.uiElements[i]);
    }
}

static Rectangle digitTextureRect(char digit) {
    static const int w = 8;
    return (Rectangle){(digit % 5) * w, (digit / 5) * w, w, w};
}

static void drawUI() {
    drawUITanks();
    drawUIElements();
}

static void drawPowerUp(PowerUp *p) {
    if (((long)(game.totalTime * 8)) % 2)
        return;
    Texture2D *tex = game.powerUpSpecs[p->type].texture;
    Vector2 drawSize = {POWER_UP_TEXTURE_SIZE.x * 2,
                        POWER_UP_TEXTURE_SIZE.y * 2};
    Vector2 drawOffset = {(POWER_UP_SIZE - drawSize.x) / 2,
                          (POWER_UP_SIZE - drawSize.y) / 2};
    int texX = game.powerUpSpecs[p->type].texCol * POWER_UP_TEXTURE_SIZE.x;
    DrawTexturePro(
        *tex,
        (Rectangle){texX, 0, POWER_UP_TEXTURE_SIZE.x, POWER_UP_TEXTURE_SIZE.y},
        (Rectangle){p->pos.x + drawOffset.x, p->pos.y + drawOffset.y,
                    drawSize.x, drawSize.y},
        (Vector2){}, 0, WHITE);
}

static void drawPowerUps() {
    for (int i = 0; i < MAX_POWERUP_COUNT; i++) {
        PowerUp *p = &game.powerUps[i];
        if (p->isActive) {
            drawPowerUp(p);
        }
    }
}

static void drawGame() {
    drawField();
    drawBullets();
    drawTanks();
    drawFlag();
    drawForest();
    drawExplosions();
    drawPowerUps();
    drawUI();
}

static void loadTextures() {
    game.textures.flag = LoadTexture("textures/flag.png");
    game.textures.powerups = LoadTexture("textures/powerup.png");
    game.textures.ui = LoadTexture("textures/ui.png");
    game.textures.digits = LoadTexture("textures/digits.png");
    game.textures.uiFlag = LoadTexture("textures/uiFlag.png");
    game.textures.spawningTank = LoadTexture("textures/born.png");
    game.textures.enemies = LoadTexture("textures/enemies.png");
    game.textures.enemiesWithPowerUps =
        LoadTexture("textures/enemies_with_powerups.png");
    game.textures.border = LoadTexture("textures/border.png");
    game.cellSpecs[CTBorder] =
        (CellSpec){.texture = &game.textures.border, .isSolid = true};
    game.textures.brick = LoadTexture("textures/brick.png");
    game.cellSpecs[CTBrick] =
        (CellSpec){.texture = &game.textures.brick, .isSolid = true};
    game.textures.concrete = LoadTexture("textures/concrete.png");
    game.cellSpecs[CTConcrete] =
        (CellSpec){.texture = &game.textures.concrete, .isSolid = true};
    game.textures.forest = LoadTexture("textures/forest.png");
    game.cellSpecs[CTForest] =
        (CellSpec){.texture = &game.textures.forest, .isPassable = true};
    game.textures.river[0] = LoadTexture("textures/river1.png");
    game.textures.river[1] = LoadTexture("textures/river2.png");
    game.cellSpecs[CTRiver] = (CellSpec){.texture = &game.textures.river[0]};
    game.textures.blank = LoadTexture("textures/blank.png");
    game.cellSpecs[CTBlank] =
        (CellSpec){.texture = &game.textures.blank, .isPassable = true};
    game.textures.player1Tank = LoadTexture("textures/player1.png");
    game.textures.player2Tank = LoadTexture("textures/player2.png");
    game.textures.bullet = LoadTexture("textures/bullet.png");
    game.textures.bulletExplosions[0] =
        LoadTexture("textures/bullet_explosion_1.png");
    game.textures.bulletExplosions[1] =
        LoadTexture("textures/bullet_explosion_2.png");
    game.textures.bulletExplosions[2] =
        LoadTexture("textures/bullet_explosion_3.png");
    game.textures.bigExplosions[0] =
        LoadTexture("textures/big_explosion_1.png");
    game.textures.bigExplosions[1] =
        LoadTexture("textures/big_explosion_2.png");
    game.textures.bigExplosions[2] =
        LoadTexture("textures/big_explosion_3.png");
    game.textures.bigExplosions[3] =
        LoadTexture("textures/big_explosion_4.png");
    game.textures.bigExplosions[4] =
        LoadTexture("textures/big_explosion_5.png");
}

static void loadStage(int stage) {
    char filename[50];
    snprintf(filename, 50, "levels/%d.stage", stage);
    Buffer buf = readFile(filename);
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            game.field[i][j].texRow = i - 2 % 4;
            game.field[i][j].texCol = j % 4;
            if (i <= 1 || i >= FIELD_ROWS - 2 || j <= 3 ||
                j >= FIELD_COLS - 8) {
                game.field[i][j].type = CTBorder;
                continue;
            }
            int index =
                ((i - 2) / 4) * ((FIELD_COLS - 12) / 4 + 1) + ((j - 4) / 4);
            switch (buf.bytes[index]) {
            case 'b':
                game.field[i][j].type = CTBrick;
                break;
            case 'c':
                game.field[i][j].type = CTConcrete;
                break;
            case 'f':
                game.field[i][j].type = CTForest;
                break;
            case 'r':
                game.field[i][j].type = CTRiver;
                break;
            default:
                game.field[i][j].type = CTBlank;
                break;
            }
        }
    }
}

static void initUIElements() {
    game.uiElements[UIFlag] =
        (UIElement){.isVisible = true,
                    .texture = &game.textures.uiFlag,
                    .textureSrc = (Rectangle){0, 0, game.textures.uiFlag.width,
                                              game.textures.uiFlag.height},
                    .pos =
                        (Vector2){
                            (14 * 4 + 2) * CELL_SIZE,
                            (11 * 4 * CELL_SIZE),
                        },
                    .size = (Vector2){CELL_SIZE * 4, CELL_SIZE * 4},
                    .drawSize = (Vector2){game.textures.uiFlag.width * 2,
                                          game.textures.uiFlag.height * 2}};
    game.uiElements[UIPlayer1] =
        (UIElement){.isVisible = true,
                    .texture = &game.textures.ui,
                    .textureSrc = (Rectangle){28, 0, 28, 14},
                    .pos =
                        (Vector2){
                            (14 * 4 + 2) * CELL_SIZE + 8,
                            ((7 * 4 + 2) * CELL_SIZE),
                        },
                    .size = (Vector2){14 * 4, 14 * 2},
                    .drawSize = (Vector2){14 * 4, 14 * 2}};
    game.uiElements[UIP1Tank] =
        (UIElement){.isVisible = true,
                    .texture = &game.textures.ui,
                    .textureSrc = (Rectangle){14, 0, 14, 14},
                    .pos =
                        (Vector2){
                            (14 * 4 + 2) * CELL_SIZE,
                            ((8 * 4) * CELL_SIZE),
                        },
                    .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                    .drawSize = (Vector2){14 * 2, 14 * 2}};
    game.uiElements[UIP1Lifes] =
        (UIElement){.isVisible = true,
                    .texture = &game.textures.digits,
                    .textureSrc = digitTextureRect(game.tanks[0].lifes),
                    .pos =
                        (Vector2){
                            (15 * 4) * CELL_SIZE,
                            ((8 * 4) * CELL_SIZE),
                        },
                    .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                    .drawSize = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2}};
    game.uiElements[UIStageLowDigit] =
        (UIElement){.isVisible = true,
                    .texture = &game.textures.digits,
                    .textureSrc = digitTextureRect(game.stage % 10),
                    .pos =
                        (Vector2){
                            (16 * 4 - 4) * CELL_SIZE,
                            (14 * 4 - 8) * CELL_SIZE,
                        },
                    .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                    .drawSize = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2}};
    if (game.stage / 10) {
        game.uiElements[UIStageHiDigit] =
            (UIElement){.isVisible = true,
                        .texture = &game.textures.digits,
                        .textureSrc = digitTextureRect(game.stage / 10),
                        .pos =
                            (Vector2){
                                (16 * 4 - 6) * CELL_SIZE,
                                (14 * 4 - 8) * CELL_SIZE,
                            },
                        .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                        .drawSize = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2}};
    }
    if (game.tanks[1].status == TSActive) {
        game.uiElements[UIPlayer2] =
            (UIElement){.isVisible = true,
                        .texture = &game.textures.ui,
                        .textureSrc = (Rectangle){56, 0, 28, 14},
                        .pos =
                            (Vector2){
                                (14 * 4 + 2) * CELL_SIZE + 8,
                                ((9 * 4) * CELL_SIZE),
                            },
                        .size = (Vector2){14 * 4, 14 * 2},
                        .drawSize = (Vector2){14 * 4, 14 * 2}};
        game.uiElements[UIP2Tank] =
            (UIElement){.isVisible = true,
                        .texture = &game.textures.ui,
                        .textureSrc = (Rectangle){14, 0, 14, 14},
                        .pos =
                            (Vector2){
                                (14 * 4 + 2) * CELL_SIZE,
                                ((9 * 4 + 2) * CELL_SIZE),
                            },
                        .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                        .drawSize = (Vector2){14 * 2, 14 * 2}};
        game.uiElements[UIP2Lifes] =
            (UIElement){.isVisible = true,
                        .texture = &game.textures.digits,
                        .textureSrc = digitTextureRect(game.tanks[1].lifes),
                        .pos =
                            (Vector2){
                                (15 * 4) * CELL_SIZE,
                                ((9 * 4 + 2) * CELL_SIZE),
                            },
                        .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                        .drawSize = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2}};
    }
}

static void spawnPlayer(Tank *t) {
    t->pos = t->type == TPlayer1 ? PLAYER1_START_POS : PLAYER2_START_POS;
    t->direction = DUp;
    t->status = TSActive;
    t->tier = 0;
    game.tankSpecs[t->type].bulletSpeed = BULLET_SPEEDS[0];
    game.tankSpecs[t->type].maxBulletCount = 1;
}

static void initStage(char stage) {
    game.stage = stage;
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            game.field[i][j] =
                (Cell){.type = CTBlank,
                       .pos = (Vector2){j * CELL_SIZE, i * CELL_SIZE}};
        }
    }
    loadStage(game.stage);
    game.tanks[0] = (Tank){
        .type = TPlayer1, .lifes = 2, .playerScore = &game.playerScores[0]};
    spawnPlayer(&game.tanks[0]);
    game.tanks[1] = (Tank){.type = TPlayer2,
                           .lifes = 2,
                           .status = TSPending,
                           .playerScore = &game.playerScores[1]};
    // spawnPlayer(&game.tanks[e1]);
    static char startingCols[3] = {4, 4 + (FIELD_COLS - 12) / 4 / 2 * 4,
                                   FIELD_COLS - 8 - 4};
    for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
        game.tanks[i + 2] = (Tank){
            .type = TBasic,
            .pos = (Vector2){CELL_SIZE * startingCols[i % 3], CELL_SIZE * 2},
            .direction = DDown,
            .status = TSPending,
        };
        if (i + 1 == 4)
            game.tanks[i + 2].powerUp = &game.powerUps[0];
        else if (i + 1 == 11)
            game.tanks[i + 2].powerUp = &game.powerUps[1];
        else if (i + 1 == 18)
            game.tanks[i + 2].powerUp = &game.powerUps[2];
    }
    for (int i = 0; i < MAX_POWERUP_COUNT; i++) {
        game.powerUps[i] = (PowerUp){
            .type = rand() % PUHelmet,
            .pos = POWERUP_POSITIONS[rand() % POWERUP_POSITIONS_COUNT],
            .isActive = false};
    }
    memset(game.bullets, 0, sizeof(game.bullets));
    memset(game.explosions, 0, sizeof(game.explosions));
    game.pendingEnemyCount = MAX_ENEMY_COUNT;
    game.maxActiveEnemyCount = 4;
    game.timeSinceSpawn = ENEMY_SPAWN_INTERVAL;
    game.activeEnemyCount = 0;
    initUIElements();
}

static void initGame() {
    loadTextures();
    game.explosionAnimations[ETBullet] =
        (Animation){.duration = BULLET_EXPLOSION_TTL,
                    .textureCount = ASIZE(game.textures.bulletExplosions),
                    .textures = &game.textures.bulletExplosions[0]};
    game.explosionAnimations[ETBig] =
        (Animation){.duration = BIG_EXPLOSION_TTL,
                    .textureCount = ASIZE(game.textures.bigExplosions),
                    .textures = &game.textures.bigExplosions[0]};
    game.flagPos = (Vector2){CELL_SIZE * ((FIELD_COLS - 12) / 2 - 2 + 4),
                             CELL_SIZE * (FIELD_ROWS - 4 - 2)};
    game.tankSpecs[TPlayer1] = (TankSpec){.texture = &game.textures.player1Tank,
                                          .texRow = 0,
                                          .bulletSpeed = BULLET_SPEEDS[0],
                                          .maxBulletCount = 1,
                                          .speed = PLAYER_SPEED};
    game.tankSpecs[TPlayer2] = (TankSpec){.texture = &game.textures.player2Tank,
                                          .texRow = 0,
                                          .bulletSpeed = BULLET_SPEEDS[0],
                                          .maxBulletCount = 1,
                                          .speed = PLAYER_SPEED};
    game.tankSpecs[TBasic] =
        (TankSpec){.texture = &game.textures.enemies,
                   .powerUpTexture = &game.textures.enemiesWithPowerUps,
                   .texRow = 0,
                   .speed = ENEMY_SPEEDS[0],
                   .bulletSpeed = BULLET_SPEEDS[0],
                   .maxBulletCount = 1,
                   .points = 100,
                   .isEnemy = true};
    game.tankSpecs[TFast] =
        (TankSpec){.texture = &game.textures.enemies,
                   .powerUpTexture = &game.textures.enemiesWithPowerUps,
                   .texRow = 1,
                   .speed = ENEMY_SPEEDS[2],
                   .maxBulletCount = 1,
                   .bulletSpeed = BULLET_SPEEDS[1],
                   .points = 200,
                   .isEnemy = true};
    game.tankSpecs[TPower] =
        (TankSpec){.texture = &game.textures.enemies,
                   .powerUpTexture = &game.textures.enemiesWithPowerUps,
                   .texRow = 2,
                   .speed = ENEMY_SPEEDS[1],
                   .bulletSpeed = BULLET_SPEEDS[2],
                   .maxBulletCount = 1,
                   .points = 300,
                   .isEnemy = true};
    game.tankSpecs[TArmor] =
        (TankSpec){.texture = &game.textures.enemies,
                   .powerUpTexture = &game.textures.enemiesWithPowerUps,
                   .texRow = 3,
                   .speed = ENEMY_SPEEDS[1],
                   .bulletSpeed = BULLET_SPEEDS[1],
                   .maxBulletCount = 1,
                   .points = 400,
                   .isEnemy = true};
    game.powerUpSpecs[PUTank] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 0};
    game.powerUpSpecs[PUTimer] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 1};
    game.powerUpSpecs[PUShovel] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 2};
    game.powerUpSpecs[PUGrenade] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 3};
    game.powerUpSpecs[PUStar] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 4};
    game.powerUpSpecs[PUHelmet] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 5};
    initStage(1);
}

static void finishFire(Tank *t) {
    if (!t->firedBulletCount)
        t->isFiring = false;
}

static void fireBullet(Tank *t) {
    if (t->firedBulletCount >= game.tankSpecs[t->type].maxBulletCount ||
        t->isFiring)
        return;
    t->firedBulletCount++;
    t->isFiring = true;
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b = &game.bullets[i];
        if (b->type != BTNone) {
            assert(i != MAX_BULLET_COUNT - 1);
            continue;
        }
        b->type = BTPlayer;
        b->direction = t->direction;
        b->tank = t;
        short bulletSpeed = game.tankSpecs[t->type].bulletSpeed;
        switch (b->direction) {
        case DRight:
            b->pos = (Vector2){t->pos.x + TANK_SIZE - BULLET_SIZE,
                               t->pos.y + TANK_SIZE / 2 - BULLET_SIZE / 2};
            b->speed = (Vector2){bulletSpeed, 0};
            break;
        case DLeft:
            b->pos =
                (Vector2){t->pos.x, t->pos.y + TANK_SIZE / 2 - BULLET_SIZE / 2};
            b->speed = (Vector2){-bulletSpeed, 0};
            break;
        case DUp:
            b->pos =
                (Vector2){t->pos.x + TANK_SIZE / 2 - BULLET_SIZE / 2, t->pos.y};
            b->speed = (Vector2){0, -bulletSpeed};
            break;
        case DDown:
            b->pos = (Vector2){t->pos.x + TANK_SIZE / 2 - BULLET_SIZE / 2,
                               t->pos.y + TANK_SIZE - BULLET_SIZE};
            b->speed = (Vector2){0, bulletSpeed};
            break;
        }
        break;
    }
}

static bool checkTankToTankCollision(Tank *t) {
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *tank = &game.tanks[i];
        if (t == tank || tank->status != TSActive)
            continue;
        if (collision(t->pos.x, t->pos.y, TANK_SIZE, TANK_SIZE, tank->pos.x,
                      tank->pos.y, TANK_SIZE, TANK_SIZE))
            return true;
    }
    return false;
}

static bool checkTankCollision(Tank *tank) {
    switch (tank->direction) {
    case DRight: {
        int startRow = ((int)tank->pos.y) / CELL_SIZE;
        int endRow = ((int)tank->pos.y + TANK_SIZE - 1) / CELL_SIZE;
        int col = ((int)tank->pos.x + TANK_SIZE - 1) / CELL_SIZE;
        for (int r = startRow; r <= endRow; r++) {
            CellType cellType = game.field[r][col].type;
            if (!game.cellSpecs[cellType].isPassable) {
                tank->pos.x = game.field[r][col].pos.x - TANK_SIZE;
                return true;
            }
        }
        return false;
    }
    case DLeft: {
        int startRow = ((int)tank->pos.y) / CELL_SIZE;
        int endRow = ((int)tank->pos.y + TANK_SIZE - 1) / CELL_SIZE;
        int col = ((int)tank->pos.x) / CELL_SIZE;
        for (int r = startRow; r <= endRow; r++) {
            CellType cellType = game.field[r][col].type;
            if (!game.cellSpecs[cellType].isPassable) {
                tank->pos.x = game.field[r][col].pos.x + CELL_SIZE;
                return true;
            }
        }
        return false;
    }
    case DUp: {
        int startCol = ((int)tank->pos.x) / CELL_SIZE;
        int endCol = ((int)tank->pos.x + TANK_SIZE - 1) / CELL_SIZE;
        int row = ((int)(tank->pos.y)) / CELL_SIZE;
        for (int c = startCol; c <= endCol; c++) {
            CellType cellType = game.field[row][c].type;
            if (!game.cellSpecs[cellType].isPassable) {
                tank->pos.y = game.field[row][c].pos.y + CELL_SIZE;
                return true;
            }
        }
        return false;
    }
    case DDown: {
        int startCol = ((int)tank->pos.x) / CELL_SIZE;
        int endCol = ((int)tank->pos.x + TANK_SIZE - 1) / CELL_SIZE;
        int row = ((int)tank->pos.y + TANK_SIZE - 1) / CELL_SIZE;
        for (int c = startCol; c <= endCol; c++) {
            CellType cellType = game.field[row][c].type;
            if (!game.cellSpecs[cellType].isPassable) {
                tank->pos.y = game.field[row][c].pos.y - TANK_SIZE;
                return true;
            }
        }
        return false;
    }
    }
}

static int snap(int x) {
    int x1 = (x / SNAP_TO) * SNAP_TO;
    int x2 = x1 + SNAP_TO;
    return (x - x1 < x2 - x) ? x1 : x2;
}

static void updatePlayerLifesUI() {
    game.uiElements[UIP1Lifes].textureSrc =
        digitTextureRect(game.tanks[0].lifes);
    game.uiElements[UIP2Lifes].textureSrc =
        digitTextureRect(game.tanks[1].lifes);
}

static void createExplosion(ExplosionType type, Vector2 targetPos,
                            int targetSize) {
    int explosionSize = game.explosionAnimations[type].textures[0].width * 2;
    int offset = (explosionSize - targetSize) / 2;
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        if (game.explosions[i].ttl <= 0) {
            game.explosions[i].ttl = game.explosionAnimations[type].duration;
            game.explosions[i].type = type;
            game.explosions[i].pos =
                (Vector2){targetPos.x - offset, targetPos.y - offset};
            break;
        }
    }
}

static void destroyTank(Tank *t) {
    t->status = TSDead;
    t->lifes--;
    if (game.tankSpecs[t->type].isEnemy) {
        game.activeEnemyCount--;
    }
    if (t->powerUp) {
        t->powerUp->isActive = true;
    }
    createExplosion(ETBig, t->pos, TANK_SIZE);
}

static void destroyAllTanks() {
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *t = &game.tanks[i + 2];
        if (t->status == TSActive)
            destroyTank(t);
    }
}

static void handlePowerUpHit(Tank *t) {
    if (game.tankSpecs[t->type].isEnemy)
        return;
    for (int i = 0; i < MAX_POWERUP_COUNT; i++) {
        PowerUp *p = &game.powerUps[i];
        if (p->isActive &&
            collision(t->pos.x, t->pos.y, TANK_SIZE, TANK_SIZE, p->pos.x,
                      p->pos.y, POWER_UP_SIZE, POWER_UP_SIZE)) {
            p->isActive = false;
            t->playerScore->totalScore += POWERUP_SCORE;
            switch (p->type) {
            case PUTank:
                t->lifes++;
                updatePlayerLifesUI();
                break;
            case PUStar:
                if (t->tier == 3)
                    return;
                t->tier++;
                switch (t->tier) {
                case 1:
                    game.tankSpecs[t->type].bulletSpeed = BULLET_SPEEDS[2];
                    break;
                case 2:
                    game.tankSpecs[t->type].maxBulletCount = 2;
                    break;
                case 3:
                    break;
                }
                break;
            case PUGrenade:
                destroyAllTanks();
                break;
            case PUShovel:
            case PUTimer:
            case PUHelmet:
            case PUMax:
                break;
            }
            return;
        }
    }
}

static void handleCommand(Tank *t, Command cmd) {
    if (cmd.fire) {
        fireBullet(t);
    } else {
        finishFire(t);
    }
    t->isMoving = cmd.move;
    if (!cmd.move)
        return;
    t->texColOffset = (t->texColOffset + 1) % 2;
    Vector2 prevPos = t->pos;
    if (t->direction == cmd.direction) {
        int delta = game.frameTime * game.tankSpecs[t->type].speed;
        switch (t->direction) {
        case DLeft:
            t->pos.x -= delta;
            break;
        case DRight:
            t->pos.x += delta;
            break;
        case DUp:
            t->pos.y -= delta;
            break;
        case DDown:
            t->pos.y += delta;
            break;
        }
    } else if (((t->direction == DRight && cmd.direction == DLeft) ||
                (t->direction == DLeft && cmd.direction == DRight)) ||
               ((t->direction == DUp && cmd.direction == DDown) ||
                (t->direction == DDown && cmd.direction == DUp))) {
        t->direction = cmd.direction;
        return;
    } else {
        switch (t->direction) {
        case DLeft:
        case DRight:
            t->pos.x = snap((int)t->pos.x);
            break;
        case DUp:
        case DDown:
            t->pos.y = snap((int)t->pos.y);
            break;
        }
    }
    handlePowerUpHit(t);
    if (checkTankToTankCollision(t)) {
        t->pos = prevPos;
        t->isMoving = false;
    } else {
        if (checkTankCollision(t)) {
            t->isMoving = false;
        }
    }
    t->direction = cmd.direction;
}

static float randomFloat() { return (float)rand() / (float)RAND_MAX; }

static bool randomTrue(float trueChance) { return randomFloat() < trueChance; }

static void handleTankAI(Tank *t) {
    static Direction dirs[] = {DDown,  DDown, DDown, DDown, DRight,
                               DRight, DLeft, DLeft, DUp};
    Command cmd = {};
    cmd.fire = randomTrue(0.01f);
    cmd.move = t->isMoving ? randomTrue(0.995f) : randomTrue(0.08f);
    if (cmd.move) {
        cmd.direction = (t->isMoving && randomTrue(0.99f))
                            ? t->direction
                            : dirs[rand() % ASIZE(dirs)];
    }
    handleCommand(t, cmd);
}

static void handleAI() {
    for (int i = 2; i < MAX_TANK_COUNT; i++) {
        Tank *t = &game.tanks[i];
        if (t->status != TSActive)
            continue;
        handleTankAI(t);
    }
}

static void handleInput() {
    Command cmd = {};
    if (IsKeyDown(KEY_RIGHT)) {
        cmd.move = true;
        cmd.direction = DRight;
    } else if (IsKeyDown(KEY_LEFT)) {
        cmd.move = true;
        cmd.direction = DLeft;
    } else if (IsKeyDown(KEY_UP)) {
        cmd.move = true;
        cmd.direction = DUp;
    } else if (IsKeyDown(KEY_DOWN)) {
        cmd.move = true;
        cmd.direction = DDown;
    }
    if (IsKeyDown(KEY_Z)) {
        cmd.fire = true;
    }
    handleCommand(&game.tanks[0], cmd);
}

static void destroyBullet(Bullet *b, bool explosion) {
    b->type = BTNone;
    b->tank->firedBulletCount--;
    if (explosion) {
        createExplosion(ETBullet, b->pos, BULLET_SIZE);
    }
}

static void destroyBrick(int row, int col, bool destroyConcrete) {
    if (row < 0 || row >= FIELD_ROWS || col < 0 || col >= FIELD_COLS)
        return;
    if (game.field[row][col].type == CTBrick ||
        (destroyConcrete && game.field[row][col].type == CTConcrete))
        game.field[row][col].type = CTBlank;
}

static void checkBulletRows(Bullet *b, int startRow, int endRow, int col,
                            int nextCol) {
    for (int r = startRow; r <= endRow; r++) {
        CellType cellType = game.field[r][col].type;
        if (game.cellSpecs[cellType].isSolid) {
            destroyBullet(b, true);
            bool destroyConcrete = b->tank->tier == 3;
            for (int rr = startRow - 1; rr <= endRow + 1; rr++) {
                destroyBrick(rr, col, destroyConcrete);
                if (destroyConcrete) {
                    destroyBrick(rr, nextCol, destroyConcrete);
                }
            }
            return;
        }
    }
}

static void checkBulletCols(Bullet *b, int startCol, int endCol, int row,
                            int nextRow) {
    for (int c = startCol; c <= endCol; c++) {
        CellType cellType = game.field[row][c].type;
        if (game.cellSpecs[cellType].isSolid) {
            destroyBullet(b, true);
            bool destroyConcrete = b->tank->tier == 3;
            for (int cc = startCol - 1; cc <= endCol + 1; cc++) {
                destroyBrick(row, cc, destroyConcrete);
                if (destroyConcrete) {
                    destroyBrick(nextRow, cc, destroyConcrete);
                }
            }
            return;
        }
    }
}

static void handlePlayerKill(Tank *t) {
    if (game.tankSpecs[t->type].isEnemy)
        return;
    if (t->lifes == -1) {
        t->lifes = 2;
    }
    spawnPlayer(t);
    updatePlayerLifesUI();
}

static void checkStageEnd() {
    if (!game.activeEnemyCount) {
        if (game.stage == STAGE_COUNT) {
            initStage(1);
        } else {
            initStage(game.stage + 1);
        }
    }
}

static void checkBulletHit(Bullet *b) {
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *t = &game.tanks[i];
        if (t->status != TSActive || b->tank == t ||
            (game.tankSpecs[b->tank->type].isEnemy &&
             game.tankSpecs[t->type].isEnemy))
            continue;
        if (collision(b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE, t->pos.x,
                      t->pos.y, TANK_SIZE, TANK_SIZE)) {
            destroyBullet(b, true);
            destroyTank(t);
            handlePlayerKill(t);
            if (!game.tankSpecs[b->tank->type].isEnemy) {
                game.playerScores[b->tank->type].totalScore +=
                    game.tankSpecs[t->type].points;
                game.playerScores[b->tank->type].kills[t->type]++;
            }
            break;
        }
    }
}

static bool checkBulletToBulletCollision(Bullet *b) {
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b2 = &game.bullets[i];
        if (b == b2 || b2->type == BTNone)
            continue;
        if (collision(b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE, b2->pos.x,
                      b2->pos.y, BULLET_SIZE, BULLET_SIZE)) {
            destroyBullet(b, false);
            destroyBullet(b2, false);
            return true;
        }
    }
    return false;
}

static void destroyFlag() { createExplosion(ETBig, game.flagPos, FLAG_SIZE); }

static bool checkFlagHit(Bullet *b) {
    if (collision(b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE, game.flagPos.x,
                  game.flagPos.y, FLAG_SIZE, FLAG_SIZE)) {
        destroyBullet(b, true);
        destroyFlag();
        return true;
    }
    return false;
}

static void checkBulletCollision(Bullet *b) {
    if (checkFlagHit(b))
        return;
    if (checkBulletToBulletCollision(b))
        return;
    checkBulletHit(b);
    switch (b->direction) {
    case DRight: {
        int startRow = ((int)b->pos.y) / CELL_SIZE;
        int endRow = ((int)b->pos.y + BULLET_SIZE - 1) / CELL_SIZE;
        int col = ((int)b->pos.x + BULLET_SIZE - 1) / CELL_SIZE;
        checkBulletRows(b, startRow, endRow, col, col + 1);
        return;
    }
    case DLeft: {
        int startRow = ((int)b->pos.y) / CELL_SIZE;
        int endRow = ((int)b->pos.y + BULLET_SIZE - 1) / CELL_SIZE;
        int col = ((int)b->pos.x) / CELL_SIZE;
        checkBulletRows(b, startRow, endRow, col, col - 1);
        return;
    }
    case DUp: {
        int startCol = ((int)b->pos.x) / CELL_SIZE;
        int endCol = ((int)b->pos.x + BULLET_SIZE - 1) / CELL_SIZE;
        int row = ((int)(b->pos.y)) / CELL_SIZE;
        checkBulletCols(b, startCol, endCol, row, row - 1);
        return;
    }
    case DDown: {
        int startCol = ((int)b->pos.x) / CELL_SIZE;
        int endCol = ((int)b->pos.x + BULLET_SIZE - 1) / CELL_SIZE;
        int row = ((int)b->pos.y + BULLET_SIZE - 1) / CELL_SIZE;
        checkBulletCols(b, startCol, endCol, row, row + 1);
        return;
    }
    }
}

static void updateBulletsState() {
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b = &game.bullets[i];
        if (b->type == BTNone)
            continue;
        b->pos.x += (b->speed.x * game.frameTime);
        b->pos.y += (b->speed.y * game.frameTime);
        checkBulletCollision(b);
    }
}

static void updateExplosionsState() {
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        if (game.explosions[i].ttl > 0) {
            game.explosions[i].ttl -= game.frameTime;
        }
    }
}

static void spawnTanks() {
    if (game.timeSinceSpawn < ENEMY_SPAWN_INTERVAL ||
        game.activeEnemyCount >= game.maxActiveEnemyCount)
        return;
    game.timeSinceSpawn = 0;
    for (int i = 2; i < MAX_TANK_COUNT; i++) {
        if (game.tanks[i].status == TSPending) {
            game.tanks[i].status = TSSpawning;
            game.activeEnemyCount++;
            game.pendingEnemyCount--;
            return;
        }
    }
}

static void updateGameState() {
    game.cellSpecs[CTRiver].texture =
        &game.textures.river[((long)(game.totalTime * 2)) % 2];
    updateExplosionsState();
    updateBulletsState();
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *tank = &game.tanks[i];
        if (tank->status == TSActive) {
            // updateTankState(&game.tanks[i]);
        } else if (tank->status == TSSpawning) {
            tank->spawningTime += game.frameTime;
            if (tank->spawningTime >= SPAWNING_TIME) {
                tank->status = TSActive;
                if (tank->powerUp) {
                    for (int k = 0; k < MAX_POWERUP_COUNT; k++) {
                        game.powerUps[k].isActive = false;
                    }
                }
            }
        }
    }
    spawnTanks();
    checkStageEnd();
}

int main(void) {

    srand(time(0));

    SetTraceLogLevel(LOG_NONE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Battle City 4000");

    initGame();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        game.frameTime = GetFrameTime();
        game.totalTime = GetTime();
        game.timeSinceSpawn += game.frameTime;
        handleInput();
        handleAI();
        updateGameState();
        BeginDrawing();
        ClearBackground(BLACK);
        drawGame();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
