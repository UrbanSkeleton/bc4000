#include <assert.h>
#include <string.h>
#include <time.h>

#include "raylib.h"
#include "raymath.h"
#include "utils.c"

// #define DRAW_CELL_GRID
#define ALT_ASSETS

#ifdef ALT_ASSETS
#define ASSETDIR "alt"
#define SOUND_EXT "wav"
#else
#define ASSETDIR "original"
#define SOUND_EXT "ogg"
#endif

static void gameLogic();
static void drawGame();
static void stageSummaryLogic();
static void drawStageSummary();
static void titleLogic();
static void drawTitle();
static void gameOverCurtainLogic();
static void drawGameOverCurtain();
static void congratsLogic();
static void drawCongrats();
static void saveHiScore();

#define ASIZE(a) (sizeof(a) / sizeof(a[0]))

typedef struct {
    int row;
    int col;
} CellInfo;

const int FLAG_LIFES = 100;
const int FLAG_COUNT = 2;
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
const float SHIELD_TIME = 0.0;
const float SHOVEL_TIME = 15.0;
const int POWERUP_SCORE = 500;
const int MAX_POWERUP_COUNT = 3;
const int STAGE_COUNT = 16;
const int PLAYGROUND_COLS = 4 * (13 * 2 + 1);
const int PLAYGROUND_ROWS = 4 * 13;
const int LEFT_EDGE_COLS = 4;
const int RIGHT_EDGE_COLS = 4 * 2;
const int TOP_EDGE_ROWS = 4;
const int BOTTOM_EDGE_ROWS = 2;
const int FIELD_COLS = PLAYGROUND_COLS + LEFT_EDGE_COLS + RIGHT_EDGE_COLS;
const int FIELD_ROWS = PLAYGROUND_ROWS + TOP_EDGE_ROWS + BOTTOM_EDGE_ROWS;
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
const int PLAYER2_START_COL = 4 * 8 + 4;
const Vector2 PLAYER1_START_POS = {
    CELL_SIZE * LEFT_EDGE_COLS,
    CELL_SIZE *(TOP_EDGE_ROWS + (PLAYGROUND_ROWS - 4) / 2 - 6)};
const Vector2 PLAYER2_START_POS = {
    CELL_SIZE * (LEFT_EDGE_COLS + PLAYGROUND_COLS - 4),
    CELL_SIZE *(TOP_EDGE_ROWS + (PLAYGROUND_ROWS - 4) / 2 - 6)};
const int PLAYER_SPEED = 220;
const short ENEMY_SPEEDS[3] = {140, 170, 240};
const int MAX_ENEMY_COUNT = 20;
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
const float PLAYWER_RESPAWN_TIME = 5.0f;
const int POWERUP_POSITIONS_COUNT = 16;
const int LIFE_COUNT = 127;
const Vector2 POWERUP_POSITIONS[POWERUP_POSITIONS_COUNT] = {
    {(4 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (7 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},
    {(4 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (4 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},
    {(7 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (7 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},
    {(7 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (4 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},

    {(1 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (7 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},
    {(1 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (4 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},
    {(10 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (7 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},
    {(10 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (4 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},

    {(1 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (1 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},
    {(1 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (10 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},
    {(10 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (1 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},
    {(10 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (10 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},

    {(4 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (1 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},
    {(7 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (10 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},
    {(4 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (1 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE},
    {(7 * 4 + 2 + LEFT_EDGE_COLS) * CELL_SIZE,
     (10 * 4 + 2 + TOP_EDGE_ROWS) * CELL_SIZE}};

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
typedef enum { UIFlag, UIStageLowDigit, UIStageHiDigit, UIMax } UIElementType;

typedef struct {
    int totalScore;
    int kills[TMax];
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
    char lifes;
    float respawnTime;
} TankSpec;

typedef enum {
    PUGrenade,
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
    enum { PUSPending, PUSActive, PUSPickedUp } state;
} PowerUp;

typedef struct {
    TankType type;
    Vector2 pos;
    Direction direction;
    char texColOffset;
    char firedBulletCount;
    TankStatus status;
    float spawningTime;
    bool isMoving;
    char lifes;
    PowerUp *powerUp;
    char tier;
    float shieldTimeLeft;
    float immobileTimeLeft;
    float slidingTimeLeft;
    char level;
} Tank;

static void levelUp1(Tank *t);
static void levelUp2(Tank *t);
static void levelUp3(Tank *t);
static void levelUp4(Tank *t);

typedef struct {
    int score;
    void (*levelUp)(Tank *t);
} LevelUp;

static LevelUp levelUps[] = {
    {1000, levelUp1}, {3000, levelUp2}, {6000, levelUp3}, {10000, levelUp4}};

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
    int scorePopupTexCol;
} Explosion;

typedef struct {
    int texCol;
    Vector2 pos;
    float ttl;
} ScorePopup;

typedef enum { BTNone, BTTank } BulletType;

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
    CTIce,
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
    Texture2D deadFlag;
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
    Texture2D shield;
    Texture2D ice;
    Texture2D title;
    Texture2D leftArrow;
    Texture2D rightArrow;
    Texture2D gameOver;
    Texture2D gameOverCurtain;
    Texture2D pause;
    Texture2D scores;
} Textures;

typedef struct {
    Sound big_explosion;
    Sound bullet_explosion;
    Sound bullet_hit_1;
    Sound bullet_hit_2;
    Sound game_over;
    Sound game_pause;
    Sound mode_switch;
    Sound player_fire;
    Sound powerup_appear;
    Sound powerup_pick;
    Sound start_menu;
    Sound soundtrack[21];
} Sounds;

typedef struct {
    float time;
} Title;

typedef struct {
    float time;
} StageSummary;

typedef enum { GMOnePlayer, GMTwoPlayers } GameMode;

typedef enum { GSTitle, GSPlay, GSScore, GSGameOver, GSCongrats } GameScreen;

typedef struct {
    void (*logic)(void);
    void (*draw)(void);
} GameFunctions;

static GameFunctions gameFunctions[] = {
    {.logic = titleLogic, .draw = drawTitle},
    {.logic = gameLogic, .draw = drawGame},
    {.logic = stageSummaryLogic, .draw = drawStageSummary},
    {.logic = gameOverCurtainLogic, .draw = drawGameOverCurtain},
    {.logic = congratsLogic, .draw = drawCongrats},
};

typedef struct {
    Vector2 pos;
    char lifes;
} Flag;

typedef struct {
    Cell field[FIELD_ROWS][FIELD_COLS];
    Tank tanks[MAX_TANK_COUNT];
    TankSpec tankSpecs[TMax];
    Bullet bullets[MAX_BULLET_COUNT];
    Flag flags[FLAG_COUNT];
    CellSpec cellSpecs[CTMax];
    PowerUpSpec powerUpSpecs[PUMax];
    Animation explosionAnimations[ETMax];
    Explosion explosions[MAX_EXPLOSION_COUNT];
    ScorePopup scorePopups[MAX_SCORE_POPUP_COUNT];
    Textures textures;
    Sounds sounds;
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
    float timerPowerUpTimeLeft;
    void (*logic)();
    void (*draw)();
    Title title;
    StageSummary stageSummary;
    float stageCurtainTime;
    float gameOverTime;
    float stageEndTime;
    bool isStageCurtainSoundPlayed;
    bool isPaused;
    int hiScore;
    GameScreen screen;
    char soundtrackPhase;
    char soundtrack;
    bool isDieSoundtrackPlayed;
    Font font;
} Game;

static Game game;

void levelUp1(Tank *t) {
    t->tier = 1;
    game.tankSpecs[t->type].texRow++;
    game.tankSpecs[t->type].bulletSpeed = BULLET_SPEEDS[2];
}

void levelUp2(Tank *t) {
    game.tankSpecs[t->type].respawnTime = PLAYWER_RESPAWN_TIME / 2;
}

void levelUp3(Tank *t) {
    t->tier = 2;
    game.tankSpecs[t->type].texRow++;
    game.tankSpecs[t->type].maxBulletCount = 2;
}

void levelUp4(Tank *t) {
    // t->tier = 3;
    // game.tankSpecs[t->type].texRow++;
    game.tankSpecs[t->type].speed *= 1.5;
}

static void drawText(const char *text, int x, int y, int fontSize,
                     Color color) {
    DrawTextEx(game.font, text, (Vector2){x, y}, fontSize, 2, color);
}

static int measureText(const char *text, int fontSize) {
    return MeasureTextEx(game.font, text, fontSize, 2).x;
}

static bool isEnemy(Tank *t) { return game.tankSpecs[t->type].isEnemy; }

static void drawCell(Cell *cell, bool isLeftHalf) {
    Texture2D *tex = game.cellSpecs[cell->type].texture;
    int w = tex->width / 4;
    int h = tex->height / 4;
    int col = cell->texCol;
    if (!isLeftHalf) col = 1 - col;
    DrawTexturePro(*tex, (Rectangle){col * w, cell->texRow * h, w, h},
                   (Rectangle){cell->pos.x, cell->pos.y, CELL_SIZE, CELL_SIZE},
                   (Vector2){CELL_SIZE / 2, CELL_SIZE / 2},
                   isLeftHalf ? 90 : -90,
                   // 0,
                   WHITE);
#ifdef DRAW_CELL_GRID
    DrawRectangleLines(cell->pos.x, cell->pos.y, CELL_SIZE, CELL_SIZE, BLUE);
#endif
}

static void drawField() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (game.field[i][j].type != CTForest)
                drawCell(&game.field[i][j],
                         j < LEFT_EDGE_COLS + PLAYGROUND_COLS / 2 + 2);
        }
    }
}

static void drawForest() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (game.field[i][j].type == CTForest)
                drawCell(&game.field[i][j],
                         j < LEFT_EDGE_COLS + PLAYGROUND_COLS / 2 + 2);
        }
    }
}

static void drawTank(Tank *tank) {
    if (tank->immobileTimeLeft > 0 && (long)(game.totalTime * 8) % 2) return;
    static char textureRows[4] = {1, 3, 0, 2};
    Texture2D *tex = !tank->powerUp || ((long)(game.totalTime * 8)) % 2
                         ? game.tankSpecs[tank->type].texture
                         : game.tankSpecs[tank->type].powerUpTexture;
    int texX = (textureRows[tank->direction] * 2 + tank->texColOffset) *
               TANK_TEXTURE_SIZE;
    int texY = game.tankSpecs[tank->type].texRow * TANK_TEXTURE_SIZE;
    int drawSize = TANK_TEXTURE_SIZE * 4;
    int drawOffset = (TANK_SIZE - drawSize) / 2;
    Color texColor = WHITE;
    if (tank->type == TArmor && tank->lifes > 1) {
        Color full = (Color){180, 255, 200, 255};
        float fullLifes = game.tankSpecs[tank->type].lifes;
        float k = (float)(tank->lifes - 1) / (fullLifes - 1);
        texColor = (Color){.r = WHITE.r - (WHITE.r - full.r) * k,
                           .g = WHITE.g - (WHITE.g - full.g) * k,
                           .b = WHITE.b - (WHITE.b - full.b) * k,
                           255};
    }
    DrawTexturePro(
        *tex, (Rectangle){texX, texY, TANK_TEXTURE_SIZE, TANK_TEXTURE_SIZE},
        (Rectangle){tank->pos.x + drawOffset, tank->pos.y + drawOffset,
                    drawSize, drawSize},
        (Vector2){}, 0, texColor);
    if (tank->shieldTimeLeft > 0) {
        Texture2D *tex = &game.textures.shield;
        int texY = (((long)(game.totalTime * 32)) % 2) * tex->width;
        DrawTexturePro(
            *tex, (Rectangle){0, texY, tex->width, tex->width},
            (Rectangle){tank->pos.x, tank->pos.y, TANK_SIZE, TANK_SIZE},
            (Vector2){}, 0, WHITE);
    }
}

static void drawSpawningTank(Tank *tank) {
    static char textureCols[] = {3, 2, 1, 0, 1, 2, 3, 2, 1, 0, 1, 2, 3};
    Texture2D *tex = &game.textures.spawningTank;
    int textureSize = tex->height;
    int i = (tank->spawningTime) / (SPAWNING_TIME / ASIZE(textureCols));
    i = i % ASIZE(textureCols);
    // if (i >= ASIZE(textureCols)) i = ASIZE(textureCols) - 1;
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

static void drawFlags() {
    for (int i = 0; i < FLAG_COUNT; i++) {
        Texture2D *tex = game.flags[i].lifes <= 0 ? &game.textures.deadFlag
                                                  : &game.textures.flag;
        DrawTexturePro(*tex, (Rectangle){0, 0, tex->width, tex->height},
                       (Rectangle){game.flags[i].pos.x, game.flags[i].pos.y,
                                   FLAG_SIZE, FLAG_SIZE},
                       (Vector2){}, 0, WHITE);
    }
}

static void drawBullets() {
    static int x[4] = {24, 8, 0, 16};
    Texture2D *tex = &game.textures.bullet;
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b = &game.bullets[i];
        if (b->type == BTNone) continue;
        DrawTexturePro(
            *tex, (Rectangle){x[b->direction], 0, 8, 8},
            (Rectangle){b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE},
            (Vector2){}, 0, WHITE);
    }
}

static void drawScorePopups() {
    for (int i = 0; i < MAX_SCORE_POPUP_COUNT; i++) {
        ScorePopup *s = &game.scorePopups[i];
        if (s->ttl <= 0) continue;
        Texture2D *tex = &game.textures.scores;
        DrawTexturePro(
            *tex,
            (Rectangle){s->texCol * SCORE_POPUP_TEXTURE_SIZE.x, 0,
                        SCORE_POPUP_TEXTURE_SIZE.x, SCORE_POPUP_TEXTURE_SIZE.y},
            (Rectangle){s->pos.x, s->pos.y, SCORE_POPUP_SIZE.x,
                        SCORE_POPUP_SIZE.y},
            (Vector2){}, 0, WHITE);
    }
}

static void drawExplosions() {
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        Explosion *e = &game.explosions[i];
        if (e->ttl <= 0) continue;
        int texCount = game.explosionAnimations[e->type].textureCount;
        int index =
            e->ttl / (game.explosionAnimations[e->type].duration / texCount);
        if (index >= texCount) index = texCount - 1;
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
            (Rectangle){
                (LEFT_EDGE_COLS + PLAYGROUND_COLS + 2 + 2 * (i % 2)) *
                        CELL_SIZE +
                    drawOffset,
                ((TOP_EDGE_ROWS + 2) + (i / 2 * 2)) * CELL_SIZE + drawOffset,
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
        if (game.uiElements[i].isVisible) drawUIElement(&game.uiElements[i]);
    }
}

static Rectangle digitTextureRect(char digit) {
    static const int w = 8;
    return (Rectangle){(digit % 5) * w, (digit / 5) * w, w, w};
}

static void drawTopUI() {
    char text[50];
    snprintf(text, 50, "Level: %d", game.tanks[0].level);
    drawText(text, (LEFT_EDGE_COLS + 1) * CELL_SIZE, CELL_SIZE / 2,
             FONT_SIZE / 2, WHITE);
    snprintf(text, 50, "Lifes: %d", game.flags[0].lifes);
    drawText(text, (LEFT_EDGE_COLS + 1) * CELL_SIZE, 2 * CELL_SIZE,
             FONT_SIZE / 2, WHITE);

    snprintf(text, 50, "Level: %d", game.tanks[1].level);
    drawText(text, (LEFT_EDGE_COLS + PLAYGROUND_COLS - 16) * CELL_SIZE,
             CELL_SIZE / 2, FONT_SIZE / 2, WHITE);
    snprintf(text, 50, "Lifes: %d", game.flags[1].lifes);
    drawText(text, (LEFT_EDGE_COLS + PLAYGROUND_COLS - 16) * CELL_SIZE,
             2 * CELL_SIZE, FONT_SIZE / 2, WHITE);
}

static void drawUI() {
    drawUITanks();
    drawUIElements();
    drawTopUI();
}

static void drawPowerUp(PowerUp *p) {
    if (((long)(game.totalTime * 8)) % 2) return;
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
        if (p->state == PUSActive) {
            drawPowerUp(p);
        }
    }
}

static int centerX(int size) { return (SCREEN_WIDTH - size) / 2; }
static int centerY(int size) { return (SCREEN_HEIGHT - size) / 2; }

static void drawGameOver() {
    if (!game.gameOverTime) return;
    Texture2D *tex = &game.textures.gameOver;
    int w = tex->width * 4;
    int h = tex->height * 4;
    int y =
        SCREEN_HEIGHT - (SCREEN_HEIGHT / 2 + h) *
                            MIN(game.gameOverTime / GAME_OVER_SLIDE_TIME, 1);
    DrawTexturePro(*tex, (Rectangle){0, 0, tex->width, tex->height},
                   (Rectangle){centerX(w), y, w, h}, (Vector2){}, 0, WHITE);
}

static void drawStageCurtain() {
    if (game.stageCurtainTime >= STAGE_CURTAIN_TIME) return;
    float delayTime = STAGE_CURTAIN_TIME - 0.5;
    int visibleHeight =
        SCREEN_HEIGHT * (MAX(game.stageCurtainTime - delayTime, 0) /
                         (STAGE_CURTAIN_TIME - delayTime));
    int h = (SCREEN_HEIGHT - visibleHeight) / 2;
    DrawRectangle(0, 0, SCREEN_WIDTH, h, (Color){115, 117, 115, 255});
    DrawRectangle(0, SCREEN_HEIGHT - h, SCREEN_WIDTH, h,
                  (Color){115, 117, 115, 255});
    if (game.stageCurtainTime < delayTime) {
        char text[20];
        snprintf(text, 20, "STAGE %2d", game.stage);
        int textSize = measureText(text, FONT_SIZE);
        drawText(text, centerX(textSize), (SCREEN_HEIGHT - FONT_SIZE) / 2,
                 FONT_SIZE, BLACK);
    }
}

static void drawPause() {
    if (!game.isPaused || ((long)(game.totalTime * 2)) % 2) return;
    Texture2D *tex = &game.textures.pause;
    int w = tex->width * 4;
    int h = tex->height * 4;
    DrawTexturePro(*tex, (Rectangle){0, 0, tex->width, tex->height},
                   (Rectangle){centerX(w), centerY(h), w, h}, (Vector2){}, 0,
                   WHITE);
}

static void drawGame() {
    drawField();
    drawBullets();
    drawTanks();
    drawFlags();
    drawForest();
    drawExplosions();
    drawScorePopups();
    drawPowerUps();
    drawUI();
    drawStageCurtain();
    drawGameOver();
    drawPause();
}

static void loadSounds() {
    game.sounds.big_explosion = LoadSound("sounds/big_explosion.ogg");
    game.sounds.bullet_explosion = LoadSound("sounds/bullet_explosion.ogg");
    game.sounds.bullet_hit_1 = LoadSound("sounds/bullet_hit_1.ogg");
    game.sounds.bullet_hit_2 = LoadSound("sounds/bullet_hit_2.ogg");
    game.sounds.game_over = LoadSound("sounds/game_over.ogg");
    game.sounds.game_pause = LoadSound("sounds/game_pause.ogg");
    game.sounds.mode_switch = LoadSound("sounds/mode_switch.ogg");
    game.sounds.player_fire = LoadSound("sounds/player_fire.ogg");
    game.sounds.powerup_appear = LoadSound("sounds/powerup_appear.ogg");

    game.sounds.powerup_pick =
        LoadSound("sounds/" ASSETDIR "/powerup_pick." SOUND_EXT);
    game.sounds.start_menu =
        LoadSound("sounds/" ASSETDIR "/start_menu." SOUND_EXT);
#ifdef ALT_ASSETS
    for (int track = 0; track <= 4; track++) {
        for (int phase = 0; phase <= 3; phase++) {
            char filename[50];
            snprintf(filename, 50, "sounds/soundtrack/soundtrack%d_%d.wav",
                     track, phase);
            game.sounds.soundtrack[track * 4 + phase] = LoadSound(filename);
        }
    }
    game.sounds.soundtrack[ASIZE(game.sounds.soundtrack) - 1] =
        LoadSound("sounds/soundtrack/soundtrackDie.wav");
#endif
}

static void loadTextures() {
    game.textures.flag = LoadTexture("textures/" ASSETDIR "/flag.png");
    game.textures.scores = LoadTexture("textures/" ASSETDIR "/scores.png");
    game.textures.pause = LoadTexture("textures/" ASSETDIR "/pause.png");
    game.textures.deadFlag = LoadTexture("textures/" ASSETDIR "/deadFlag.png");
    game.textures.gameOver = LoadTexture("textures/" ASSETDIR "/gameOver.png");
    game.textures.gameOverCurtain =
        LoadTexture("textures/" ASSETDIR "/gameOverCurtain.png");
    game.textures.leftArrow =
        LoadTexture("textures/" ASSETDIR "/leftArrow.png");
    game.textures.rightArrow =
        LoadTexture("textures/" ASSETDIR "/rightArrow.png");
    game.textures.title = LoadTexture("textures/" ASSETDIR "/title.png");
    game.textures.shield = LoadTexture("textures/" ASSETDIR "/shield.png");
    game.textures.powerups = LoadTexture("textures/" ASSETDIR "/powerup.png");
    game.textures.ui = LoadTexture("textures/" ASSETDIR "/ui.png");
    game.textures.digits = LoadTexture("textures/" ASSETDIR "/digits.png");
    game.textures.uiFlag = LoadTexture("textures/" ASSETDIR "/uiFlag.png");
    game.textures.spawningTank = LoadTexture("textures/" ASSETDIR "/born.png");
    game.textures.enemies = LoadTexture("textures/" ASSETDIR "/enemies.png");
    game.textures.enemiesWithPowerUps =
        LoadTexture("textures/" ASSETDIR "/enemies_with_powerups.png");
    game.textures.border = LoadTexture("textures/" ASSETDIR "/border.png");
    game.cellSpecs[CTBorder] =
        (CellSpec){.texture = &game.textures.border, .isSolid = true};
    game.textures.brick = LoadTexture("textures/" ASSETDIR "/brick.png");
    game.cellSpecs[CTBrick] =
        (CellSpec){.texture = &game.textures.brick, .isSolid = true};
    game.textures.ice = LoadTexture("textures/" ASSETDIR "/ice.png");
    game.cellSpecs[CTIce] =
        (CellSpec){.texture = &game.textures.ice, .isPassable = true};
    game.textures.concrete = LoadTexture("textures/" ASSETDIR "/concrete.png");
    game.cellSpecs[CTConcrete] =
        (CellSpec){.texture = &game.textures.concrete, .isSolid = true};
    game.textures.forest = LoadTexture("textures/" ASSETDIR "/forest.png");
    game.cellSpecs[CTForest] =
        (CellSpec){.texture = &game.textures.forest, .isPassable = true};
    game.textures.river[0] = LoadTexture("textures/" ASSETDIR "/river1.png");
    game.textures.river[1] = LoadTexture("textures/" ASSETDIR "/river2.png");
    game.cellSpecs[CTRiver] = (CellSpec){.texture = &game.textures.river[0]};
    game.textures.blank = LoadTexture("textures/" ASSETDIR "/blank.png");
    game.cellSpecs[CTBlank] =
        (CellSpec){.texture = &game.textures.blank, .isPassable = true};
    game.textures.player1Tank =
        LoadTexture("textures/" ASSETDIR "/player1.png");
    game.textures.player2Tank =
        LoadTexture("textures/" ASSETDIR "/player2.png");
    game.textures.bullet = LoadTexture("textures/" ASSETDIR "/bullet.png");
    game.textures.bulletExplosions[0] =
        LoadTexture("textures/" ASSETDIR "/bullet_explosion_1.png");
    game.textures.bulletExplosions[1] =
        LoadTexture("textures/" ASSETDIR "/bullet_explosion_2.png");
    game.textures.bulletExplosions[2] =
        LoadTexture("textures/" ASSETDIR "/bullet_explosion_3.png");
    game.textures.bigExplosions[0] =
        LoadTexture("textures/" ASSETDIR "/big_explosion_1.png");
    game.textures.bigExplosions[1] =
        LoadTexture("textures/" ASSETDIR "/big_explosion_2.png");
    game.textures.bigExplosions[2] =
        LoadTexture("textures/" ASSETDIR "/big_explosion_3.png");
    game.textures.bigExplosions[3] =
        LoadTexture("textures/" ASSETDIR "/big_explosion_4.png");
    game.textures.bigExplosions[4] =
        LoadTexture("textures/" ASSETDIR "/big_explosion_5.png");
}

static void loadStage(int stage) {
    char filename[50];
    snprintf(filename, 50, "levels/stage%.2d.new", stage);
    Buffer buf = readFile(filename);
    int ci = 0;
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (i < TOP_EDGE_ROWS || i >= FIELD_ROWS - BOTTOM_EDGE_ROWS ||
                j < LEFT_EDGE_COLS || j >= FIELD_COLS - RIGHT_EDGE_COLS) {
                game.field[i][j].type = CTBorder;
                game.field[i][j].texRow = 0;
                game.field[i][j].texCol = 0;
                continue;
            }
            game.field[i][j].type = buf.bytes[ci];
            char texNumber = buf.bytes[ci + 1];
            game.field[i][j].texRow = texNumber < 2 ? 0 : 1;
            game.field[i][j].texCol = texNumber % 2;
            ci += 2;
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
                            (LEFT_EDGE_COLS + PLAYGROUND_COLS + 2) * CELL_SIZE,
                            (11 * 4 * CELL_SIZE),
                        },
                    .size = (Vector2){CELL_SIZE * 4, CELL_SIZE * 4},
                    .drawSize = (Vector2){game.textures.uiFlag.width * 2,
                                          game.textures.uiFlag.height * 2}};
    game.uiElements[UIStageLowDigit] =
        (UIElement){.isVisible = true,
                    .texture = &game.textures.digits,
                    .textureSrc = digitTextureRect(game.stage % 10),
                    .pos =
                        (Vector2){
                            (FIELD_COLS - 4) * CELL_SIZE,
                            (FIELD_ROWS - 8) * CELL_SIZE,
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
                                (FIELD_COLS - 6) * CELL_SIZE,
                                (FIELD_ROWS - 8) * CELL_SIZE,
                            },
                        .size = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2},
                        .drawSize = (Vector2){CELL_SIZE * 2, CELL_SIZE * 2}};
    }
}

static void spawnPlayer(Tank *t, bool afterKill) {
    t->pos = t->type == TPlayer1 ? PLAYER1_START_POS : PLAYER2_START_POS;
    t->direction = t->type == TPlayer1 ? DRight : DLeft;
    t->status = TSSpawning;
    t->spawningTime =
        afterKill ? game.tankSpecs[t->type].respawnTime : SPAWNING_TIME;
    t->immobileTimeLeft = 0;
    t->firedBulletCount = 0;
    t->isMoving = false;
}

static TankType levelTanks[LEVEL_COUNT][MAX_ENEMY_COUNT] = {
    // clang-format off
    {TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TFast,TFast},
    {TArmor,TArmor,TFast, TFast, TFast, TFast, TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic},
    {TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TFast, TFast, TFast, TFast, TArmor,TArmor},
    {TPower,TPower,TPower,TPower,TPower,TPower,TPower,TPower,TPower,TPower,TFast, TFast, TFast, TFast, TFast, TBasic,TBasic,TArmor,TArmor,TArmor},
    {TPower,TPower,TPower,TPower,TPower,TArmor,TArmor,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TFast, TFast, TFast, TFast, TFast},
    {TPower,TPower,TPower,TPower,TPower,TPower,TPower,TFast, TFast, TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TArmor,TArmor},
    {TBasic,TBasic,TBasic,TFast, TFast, TFast, TFast, TPower,TPower,TPower,TPower,TPower,TPower,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic},
    {TPower,TPower,TPower,TPower,TPower,TPower,TPower,TArmor,TArmor,TFast, TFast, TFast, TFast, TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic},
    {TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TFast, TFast, TFast, TFast, TPower,TPower,TPower,TPower,TPower,TPower,TPower,TArmor,TArmor,TArmor},
    {TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TFast, TFast, TPower,TPower,TPower,TPower,TArmor,TArmor},
    {TFast, TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TPower,TPower,TPower,TPower,TFast, TFast, TFast, TFast, TFast},
    {TPower,TPower,TPower,TPower,TPower,TPower,TPower,TPower,TFast, TFast, TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor},
    {TPower,TPower,TPower,TPower,TPower,TPower,TPower,TPower,TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor},
    {TPower,TPower,TPower,TPower,TPower,TPower,TPower,TPower,TPower,TPower,TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor},
    {TBasic,TBasic,TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor},
    {TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TFast, TFast, TArmor,TArmor},
    {TArmor,TArmor,TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic},
    {TArmor,TArmor,TArmor,TArmor,TBasic,TBasic,TPower,TPower,TPower,TPower,TPower,TPower,TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast},
    {TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TBasic,TBasic,TBasic,TBasic,TPower,TPower,TPower,TPower},
    {TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TBasic,TBasic,TPower,TPower,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor},
    {TPower,TPower,TPower,TPower,TPower,TPower,TPower,TPower,TFast, TFast, TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TArmor,TArmor,TArmor,TArmor},
    {TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TPower,TPower,TArmor,TArmor,TArmor,TArmor},
    {TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TPower,TPower,TPower,TPower,TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast},
    {TPower,TPower,TPower,TPower,TArmor,TArmor,TFast, TFast, TFast, TFast, TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic},
    {TPower,TPower,TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor},
    {TFast, TFast, TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TBasic,TBasic,TBasic,TBasic,TPower,TPower,TPower,TPower},
    {TPower,TPower,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TBasic,TBasic},
    {TFast, TFast, TArmor,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TPower,TPower},
    {TPower,TPower,TPower,TPower,TPower,TPower,TPower,TPower,TPower,TPower,TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor},
    {TBasic,TBasic,TBasic,TBasic,TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TPower,TPower,TPower,TPower,TArmor,TArmor,TArmor,TArmor},
    {TPower,TPower,TPower,TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TPower,TPower,TPower},
    {TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TBasic,TBasic,TBasic,TBasic,TBasic,TBasic,TPower,TPower,TFast, TFast, TFast, TFast},
    {TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TPower,TPower,TPower,TPower,TFast, TFast, TFast, TFast},
    {TPower,TPower,TPower,TPower,TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor},
    {TPower,TPower,TPower,TPower,TFast, TFast, TFast, TFast, TFast, TFast, TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor,TArmor}
    // clang-format on
};

static void initStage(char stage) {
    game.stage = stage;
    game.gameOverTime = 0;
    game.stageEndTime = 0;
    game.timerPowerUpTimeLeft = 0;
    game.stageCurtainTime = 0;
    game.isStageCurtainSoundPlayed = false;
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            game.field[i][j] =
                (Cell){.type = CTBlank,
                       .pos = (Vector2){j * CELL_SIZE, i * CELL_SIZE}};
        }
    }
    loadStage(game.stage);
    spawnPlayer(&game.tanks[TPlayer1], false);
    spawnPlayer(&game.tanks[TPlayer2], false);

    static char startingRows[3] = {TOP_EDGE_ROWS,
                                   TOP_EDGE_ROWS + (PLAYGROUND_ROWS - 4) / 2,
                                   TOP_EDGE_ROWS + PLAYGROUND_ROWS - 4};
    for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
        TankType type = levelTanks[stage - 1][i];
        game.tanks[i + 2] =
            (Tank){.type = type,
                   .pos = (Vector2){CELL_SIZE * (LEFT_EDGE_COLS +
                                                 (PLAYGROUND_COLS / 2 - 2 - 4) +
                                                 8 * (i % 2)),
                                    CELL_SIZE * startingRows[i % 3]},
                   .direction = i % 2 ? DRight : DLeft,
                   .status = TSPending,
                   .isMoving = true,
                   .lifes = game.tankSpecs[type].lifes};
        if (i + 1 == 4)
            game.tanks[i + 2].powerUp = &game.powerUps[0];
        else if (i + 1 == 11)
            game.tanks[i + 2].powerUp = &game.powerUps[1];
        else if (i + 1 == 18)
            game.tanks[i + 2].powerUp = &game.powerUps[2];
    }
    for (int i = 0; i < MAX_POWERUP_COUNT; i++) {
        game.powerUps[i] = (PowerUp){
            .type = rand() % PUMax,
            .pos = POWERUP_POSITIONS[rand() % POWERUP_POSITIONS_COUNT],
            .state = PUSPending};
        if (rand() % 2) {
            int left = LEFT_EDGE_COLS * CELL_SIZE;
            float x = game.powerUps[i].pos.x - left;
            game.powerUps[i].pos.x =
                left + (PLAYGROUND_COLS - 4) * CELL_SIZE - x;
        }
    }
    memset(game.bullets, 0, sizeof(game.bullets));
    memset(game.explosions, 0, sizeof(game.explosions));
    game.pendingEnemyCount = MAX_ENEMY_COUNT;
    game.maxActiveEnemyCount = 8;
    game.timeSinceSpawn = ENEMY_SPAWN_INTERVAL;
    game.activeEnemyCount = 0;
    initUIElements();
}

static void loadHiScore() {
    const char *filename = "hiscore";
    FILE *f = fopen(filename, "rb");
    if (!f) {
        game.hiScore = 0;
        return;
    }
    fclose(f);
    Buffer b = readFile(filename);
    if (b.size < 4) {
        fprintf(stderr, "Cannot read hiscore");
        exit(1);
    }
    game.hiScore = (u32)b.bytes[0] | ((u32)b.bytes[1] << 8) |
                   ((u32)b.bytes[2] << 16) | ((u32)b.bytes[3] << 24);
}

static void initGameRun() {
    saveHiScore();
    for (int i = 0; i < FLAG_COUNT; i++) game.flags[i].lifes = FLAG_LIFES;
    game.tanks[TPlayer1] =
        (Tank){.type = TPlayer1, .lifes = LIFE_COUNT, .level = 0};
    game.tanks[TPlayer2] =
        (Tank){.type = TPlayer2, .lifes = LIFE_COUNT, .level = 0};
    game.tankSpecs[TPlayer1] = (TankSpec){.texture = &game.textures.player1Tank,
                                          .texRow = 0,
                                          .bulletSpeed = BULLET_SPEEDS[0],
                                          .maxBulletCount = 1,
                                          .respawnTime = PLAYWER_RESPAWN_TIME,
                                          .speed = PLAYER_SPEED};
    game.tankSpecs[TPlayer2] = (TankSpec){.texture = &game.textures.player2Tank,
                                          .texRow = 0,
                                          .bulletSpeed = BULLET_SPEEDS[0],
                                          .maxBulletCount = 1,
                                          .respawnTime = PLAYWER_RESPAWN_TIME,
                                          .speed = PLAYER_SPEED};
    game.isDieSoundtrackPlayed = false;
    memset(game.playerScores, 0, sizeof(game.playerScores));
}

static void initGame() {
    loadHiScore();
    loadTextures();
    loadSounds();
    game.font = LoadFontEx("fonts/LiberationMono.ttf", 40, NULL, 0);
    game.explosionAnimations[ETBullet] =
        (Animation){.duration = BULLET_EXPLOSION_TTL,
                    .textureCount = ASIZE(game.textures.bulletExplosions),
                    .textures = &game.textures.bulletExplosions[0]};
    game.explosionAnimations[ETBig] =
        (Animation){.duration = BIG_EXPLOSION_TTL,
                    .textureCount = ASIZE(game.textures.bigExplosions),
                    .textures = &game.textures.bigExplosions[0]};
    game.flags[0].pos =
        (Vector2){CELL_SIZE * LEFT_EDGE_COLS,
                  CELL_SIZE * (TOP_EDGE_ROWS + (PLAYGROUND_ROWS - 4) / 2)};
    game.flags[1].pos =
        (Vector2){CELL_SIZE * (LEFT_EDGE_COLS + PLAYGROUND_COLS - 4),
                  CELL_SIZE * (TOP_EDGE_ROWS + (PLAYGROUND_ROWS - 4) / 2)};
    game.tankSpecs[TBasic] =
        (TankSpec){.texture = &game.textures.enemies,
                   .powerUpTexture = &game.textures.enemiesWithPowerUps,
                   .texRow = 0,
                   .speed = ENEMY_SPEEDS[0],
                   .bulletSpeed = BULLET_SPEEDS[0],
                   .maxBulletCount = 1,
                   .points = 100,
                   .lifes = 1,
                   .isEnemy = true};
    game.tankSpecs[TFast] =
        (TankSpec){.texture = &game.textures.enemies,
                   .powerUpTexture = &game.textures.enemiesWithPowerUps,
                   .texRow = 1,
                   .speed = ENEMY_SPEEDS[2],
                   .maxBulletCount = 1,
                   .bulletSpeed = BULLET_SPEEDS[1],
                   .points = 200,
                   .lifes = 1,
                   .isEnemy = true};
    game.tankSpecs[TPower] =
        (TankSpec){.texture = &game.textures.enemies,
                   .powerUpTexture = &game.textures.enemiesWithPowerUps,
                   .texRow = 2,
                   .speed = ENEMY_SPEEDS[1],
                   .bulletSpeed = BULLET_SPEEDS[2],
                   .maxBulletCount = 1,
                   .points = 300,
                   .lifes = 1,
                   .isEnemy = true};
    game.tankSpecs[TArmor] =
        (TankSpec){.texture = &game.textures.enemies,
                   .powerUpTexture = &game.textures.enemiesWithPowerUps,
                   .texRow = 3,
                   .speed = ENEMY_SPEEDS[1],
                   .bulletSpeed = BULLET_SPEEDS[1],
                   .maxBulletCount = 1,
                   .points = 400,
                   .lifes = 4,
                   .isEnemy = true};
    game.powerUpSpecs[PUTimer] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 1};
    game.powerUpSpecs[PUGrenade] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 3};
}

static void fireBullet(Tank *t) {
    if (t->firedBulletCount >= game.tankSpecs[t->type].maxBulletCount) return;
    t->firedBulletCount++;
    if (!isEnemy(t)) {
        PlaySound(game.sounds.player_fire);
    }
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b = &game.bullets[i];
        if (b->type != BTNone) {
            assert(i != MAX_BULLET_COUNT - 1);
            continue;
        }
        b->type = BTTank;
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
                b->pos = (Vector2){t->pos.x,
                                   t->pos.y + TANK_SIZE / 2 - BULLET_SIZE / 2};
                b->speed = (Vector2){-bulletSpeed, 0};
                break;
            case DUp:
                b->pos = (Vector2){t->pos.x + TANK_SIZE / 2 - BULLET_SIZE / 2,
                                   t->pos.y};
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

static bool checkTankToFlagCollision(Tank *t) {
    for (int i = 0; i < FLAG_COUNT; i++) {
        bool c = collision(t->pos.x, t->pos.y, TANK_SIZE, TANK_SIZE,
                           game.flags[i].pos.x, game.flags[i].pos.y, FLAG_SIZE,
                           FLAG_SIZE);
        if (c) return c;
    }
    return false;
}

static bool checkTankToTankCollision(Tank *t) {
    int hitboxOffset = 4;
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *tank = &game.tanks[i];
        if (t == tank || tank->status != TSActive) continue;
        if (collision(
                t->pos.x + hitboxOffset, t->pos.y + hitboxOffset,
                TANK_SIZE - (hitboxOffset * 2), TANK_SIZE - (hitboxOffset * 2),
                tank->pos.x + hitboxOffset, tank->pos.y + hitboxOffset,
                TANK_SIZE - (hitboxOffset * 2), TANK_SIZE - (hitboxOffset * 2)))
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
                } else if (cellType == CTIce && !isEnemy(tank) &&
                           tank->slidingTimeLeft <= 0) {
                    tank->slidingTimeLeft = SLIDING_TIME;
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
                } else if (cellType == CTIce && !isEnemy(tank) &&
                           tank->slidingTimeLeft <= 0) {
                    tank->slidingTimeLeft = SLIDING_TIME;
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
                } else if (cellType == CTIce && !isEnemy(tank) &&
                           tank->slidingTimeLeft <= 0) {
                    tank->slidingTimeLeft = SLIDING_TIME;
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
                } else if (cellType == CTIce && !isEnemy(tank) &&
                           tank->slidingTimeLeft <= 0) {
                    tank->slidingTimeLeft = SLIDING_TIME;
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

static void createScorePopup(int texCol, Vector2 targetPos, int targetSize) {
    Vector2 offset = {(SCORE_POPUP_SIZE.x - targetSize) / 2,
                      (SCORE_POPUP_SIZE.y - targetSize) / 2};
    for (int i = 0; i < MAX_SCORE_POPUP_COUNT; i++) {
        if (game.scorePopups[i].ttl <= 0) {
            game.scorePopups[i].ttl = SCORE_POPUP_TTL;
            game.scorePopups[i].pos =
                (Vector2){targetPos.x - offset.x, targetPos.y - offset.y};
            game.scorePopups[i].texCol = texCol;
            break;
        }
    }
}

static void createExplosion(ExplosionType type, Vector2 targetPos,
                            int targetSize, int scorePopupTexCol) {
    int explosionSize = game.explosionAnimations[type].textures[0].width * 2;
    int offset = (explosionSize - targetSize) / 2;
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        if (game.explosions[i].ttl <= 0) {
            game.explosions[i].ttl = game.explosionAnimations[type].duration;
            game.explosions[i].type = type;
            game.explosions[i].pos =
                (Vector2){targetPos.x - offset, targetPos.y - offset};
            game.explosions[i].scorePopupTexCol = scorePopupTexCol;
            break;
        }
    }
}

static void destroyTank(Tank *t, bool scorePopup) {
    t->status = TSDead;
    t->lifes--;
    if (isEnemy(t)) {
        game.activeEnemyCount--;
    }
    int scorePopupTexCol = scorePopup && isEnemy(t)
                               ? game.tankSpecs[t->type].points / 100 - 1
                               : -1;
    createExplosion(ETBig, t->pos, TANK_SIZE, scorePopupTexCol);
}

static void handleLevelUp(Tank *t) {
    for (int i = t->level; i < ASIZE(levelUps); i++) {
        if (game.playerScores[t->type].totalScore >= levelUps[i].score) {
            t->level++;
            levelUps[i].levelUp(t);
        }
    }
}

static void addScore(TankType type, int score) {
    game.playerScores[type].totalScore += score;
    game.hiScore = MAX(game.hiScore, game.playerScores[type].totalScore);
}

static void destroyAllTanks(Tank *player) {
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *t = &game.tanks[i + 2];
        if (t->status == TSActive) {
            destroyTank(t, true);
            addScore(player->type, game.tankSpecs[t->type].points);
            game.playerScores[player->type].kills[t->type]++;
        }
    }
    handleLevelUp(player);
    PlaySound(game.sounds.bullet_explosion);
}

static void handlePowerUpHit(Tank *t) {
    if (isEnemy(t)) return;
    int tankHitboxOffset = 4;
    int powerUpHitboxOffset = 6;
    for (int i = 0; i < MAX_POWERUP_COUNT; i++) {
        PowerUp *p = &game.powerUps[i];
        if (p->state == PUSActive &&
            collision(t->pos.x + tankHitboxOffset, t->pos.y + tankHitboxOffset,
                      TANK_SIZE - (tankHitboxOffset * 2),
                      TANK_SIZE - (tankHitboxOffset * 2),
                      p->pos.x + powerUpHitboxOffset,
                      p->pos.y + powerUpHitboxOffset,
                      POWER_UP_SIZE - (powerUpHitboxOffset * 2),
                      POWER_UP_SIZE - (powerUpHitboxOffset * 2))) {
            p->state = PUSPickedUp;
            PlaySound(game.sounds.powerup_pick);
            createScorePopup(4, p->pos, POWER_UP_SIZE);
            addScore(t->type, POWERUP_SCORE);
            switch (p->type) {
                case PUGrenade:
                    destroyAllTanks(t);
                    break;
                case PUTimer:
                    game.timerPowerUpTimeLeft = TIMER_TIME;
                    break;
                case PUMax:
                    break;
            }
            return;
        }
    }
}

static void handleCommand(Tank *t, Command cmd) {
    if (t->status != TSActive) return;
    if (cmd.fire) {
        fireBullet(t);
    }
    if (t->immobileTimeLeft > 0) return;
    if (t->slidingTimeLeft > 0) {
        if (!cmd.move) {
            cmd.move = true;
            cmd.direction = t->direction;
        } else {
            t->slidingTimeLeft = 0;
        }
    }
    t->isMoving = cmd.move;
    if (!cmd.move) return;
    t->texColOffset = (t->texColOffset + 1) % 2;
    Vector2 prevPos = t->pos;
    bool isAlreadyCollided = checkTankToTankCollision(t);
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
    if ((!isAlreadyCollided && checkTankToTankCollision(t)) ||
        checkTankToFlagCollision(t)) {
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
    static Direction dirs[] = {DDown, DRight, DRight, DLeft, DLeft, DUp};
    Command cmd = {};
    cmd.fire = randomTrue(0.03f);
    cmd.move = t->isMoving ? randomTrue(0.999f) : randomTrue(0.50f);
    if (cmd.move) {
        cmd.direction = (t->isMoving && randomTrue(0.999f))
                            ? t->direction
                            : dirs[rand() % ASIZE(dirs)];
    }
    handleCommand(t, cmd);
}

static void handleAI() {
    if (game.timerPowerUpTimeLeft > 0) return;
    for (int i = 2; i < MAX_TANK_COUNT; i++) {
        Tank *t = &game.tanks[i];
        if (t->status != TSActive) continue;
        handleTankAI(t);
    }
}

typedef struct {
    KeyboardKey left;
    KeyboardKey right;
    KeyboardKey up;
    KeyboardKey down;
    KeyboardKey fire;
} Controls;

static Controls controls[2] = {
    {KEY_A, KEY_D, KEY_W, KEY_S, KEY_SPACE},
    {KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN, KEY_COMMA},
};

static void handlePlayerInput(TankType type) {
    Command cmd = {};
    if (IsKeyDown(controls[type].right)) {
        cmd.move = true;
        cmd.direction = DRight;
    } else if (IsKeyDown(controls[type].left)) {
        cmd.move = true;
        cmd.direction = DLeft;
    } else if (IsKeyDown(controls[type].up)) {
        cmd.move = true;
        cmd.direction = DUp;
    } else if (IsKeyDown(controls[type].down)) {
        cmd.move = true;
        cmd.direction = DDown;
    }
    if (IsKeyPressed(controls[type].fire)) {
        cmd.fire = true;
    }
    handleCommand(&game.tanks[type], cmd);
}

static void setScreen(GameScreen s) {
    game.screen = s;
    game.logic = gameFunctions[s].logic;
    game.draw = gameFunctions[s].draw;
}

static void handleInput() {
    if (game.gameOverTime > 0 && IsKeyPressed(KEY_ENTER)) {
        setScreen(GSScore);
        return;
    }
    if (game.gameOverTime > 0) return;
    handlePlayerInput(TPlayer1);
    handlePlayerInput(TPlayer2);
}

static void destroyBullet(Bullet *b, bool explosion) {
    b->type = BTNone;
    if (b->tank->firedBulletCount > 0) {
        b->tank->firedBulletCount--;
    }
    if (explosion) {
        createExplosion(ETBullet, b->pos, BULLET_SIZE, -1);
    }
}

static void destroyBrick(int row, int col, bool destroyConcrete,
                         bool playSound) {
    switch (game.field[row][col].type) {
        case CTBorder:
            if (playSound) {
                PlaySound(game.sounds.bullet_hit_1);
            }
            break;
        case CTBrick:
            game.field[row][col].type = CTBlank;
            if (playSound) {
                PlaySound(game.sounds.bullet_hit_2);
            }
            break;
        case CTConcrete:
            if (destroyConcrete) {
                game.field[row][col].type = CTBlank;
                if (playSound) {
                    PlaySound(game.sounds.bullet_hit_2);
                }
            } else {
                if (playSound) {
                    PlaySound(game.sounds.bullet_hit_1);
                }
            }
            break;
        default:
            break;
    }
}

static void checkBulletRows(Bullet *b, int startRow, int endRow, int col,
                            int nextCol) {
    for (int r = startRow; r <= endRow; r++) {
        CellType cellType = game.field[r][col].type;
        if (game.cellSpecs[cellType].isSolid) {
            destroyBullet(b, true);
            bool destroyConcrete = b->tank->tier == 3;
            for (int rr = startRow - 1; rr <= endRow + 1; rr++) {
                destroyBrick(rr, col, destroyConcrete, !isEnemy(b->tank));
                if (destroyConcrete) {
                    destroyBrick(rr, nextCol, destroyConcrete, false);
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
                destroyBrick(row, cc, destroyConcrete, !isEnemy(b->tank));
                if (destroyConcrete) {
                    destroyBrick(nextRow, cc, destroyConcrete, false);
                }
            }
            return;
        }
    }
}

static void gameOver() { game.gameOverTime = 0.001; }

static void handlePlayerKill(Tank *t) {
    if (game.gameOverTime > 0) return;
    if (isEnemy(t)) return;
    if (t->lifes < 0) {
        gameOver();
        return;
    }
    spawnPlayer(t, true);
}

static void checkStageEnd() {
    if (game.pendingEnemyCount + game.activeEnemyCount == 0) {
        game.stageEndTime += game.frameTime;
    }
    if (game.stageEndTime >= STAGE_END_TIME ||
        game.gameOverTime >= GAME_OVER_SLIDE_TIME + GAME_OVER_DELAY) {
        setScreen(GSScore);
    }
}

static void checkBulletHit(Bullet *b) {
    int tankHitboxOffset = 4;
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *t = &game.tanks[i];
        if (t->status != TSActive || b->tank == t ||
            (isEnemy(b->tank) && isEnemy(t)) ||
            !collision(b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE,
                       t->pos.x + tankHitboxOffset, t->pos.y + tankHitboxOffset,
                       TANK_SIZE - (tankHitboxOffset * 2),
                       TANK_SIZE - (tankHitboxOffset * 2))) {
            continue;
        }
        destroyBullet(b, true);
        if (t->shieldTimeLeft > 0) break;
        if (t->powerUp && t->powerUp->state == PUSPending) {
            t->powerUp->state = PUSActive;
            t->powerUp = NULL;
            PlaySound(game.sounds.powerup_appear);
        }
        if (isEnemy(t) && t->lifes > 1) {
            PlaySound(game.sounds.bullet_hit_1);
            t->lifes--;
            break;
        }
        destroyTank(t, true);
        handlePlayerKill(t);
        if (!isEnemy(b->tank)) {
            addScore(b->tank->type, game.tankSpecs[t->type].points);
            game.playerScores[b->tank->type].kills[t->type]++;
            handleLevelUp(b->tank);
        }
        PlaySound(isEnemy(t) ? game.sounds.bullet_explosion
                             : game.sounds.big_explosion);
    }
}

static bool checkBulletToBulletCollision(Bullet *b) {
    for (int i = 0; i < MAX_BULLET_COUNT; i++) {
        Bullet *b2 = &game.bullets[i];
        if (b == b2 || b2->type == BTNone) continue;
        if (collision(b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE, b2->pos.x,
                      b2->pos.y, BULLET_SIZE, BULLET_SIZE)) {
            destroyBullet(b, false);
            destroyBullet(b2, false);
            return true;
        }
    }
    return false;
}

static bool hitFlag(int i) {
    game.flags[i].lifes--;
    if (game.flags[i].lifes <= 0) {
        createExplosion(ETBig, game.flags[i].pos, FLAG_SIZE, -1);
        PlaySound(game.sounds.big_explosion);
        return true;
    }
    return false;
}

static bool checkFlagHit(Bullet *b) {
    if (game.gameOverTime) return false;
    for (int i = 0; i < FLAG_COUNT; i++) {
        if (collision(b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE,
                      game.flags[i].pos.x, game.flags[i].pos.y, FLAG_SIZE,
                      FLAG_SIZE)) {
            destroyBullet(b, true);
            return hitFlag(i);
        }
    }
    return false;
}

static void checkBulletCollision(Bullet *b) {
    if (checkFlagHit(b)) {
        gameOver();
        return;
    }
    if (checkBulletToBulletCollision(b)) return;
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
        if (b->type == BTNone) continue;
        b->pos.x += (b->speed.x * game.frameTime);
        b->pos.y += (b->speed.y * game.frameTime);
        checkBulletCollision(b);
    }
}

static void updateExplosionsState() {
    for (int i = 0; i < MAX_EXPLOSION_COUNT; i++) {
        Explosion *e = &game.explosions[i];
        if (e->ttl > 0) {
            e->ttl -= game.frameTime;
            if (e->ttl <= 0 && e->scorePopupTexCol != -1) {
                createScorePopup(
                    e->scorePopupTexCol, e->pos,
                    game.explosionAnimations[ETBig].textures[0].width * 2);
            }
        }
    }
}

static void updateScorePopupsState() {
    for (int i = 0; i < MAX_SCORE_POPUP_COUNT; i++) {
        if (game.scorePopups[i].ttl > 0) {
            game.scorePopups[i].ttl -= game.frameTime;
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
            game.tanks[i].spawningTime = SPAWNING_TIME;
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
    updateScorePopupsState();
    updateBulletsState();
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *tank = &game.tanks[i];
        if (tank->status == TSActive) {
            // updateTankState(&game.tanks[i]);
        } else if (tank->status == TSSpawning) {
            tank->spawningTime -= game.frameTime;
            if (tank->spawningTime <= 0) {
                tank->spawningTime = 0;
                tank->status = TSActive;
                if (!isEnemy(tank)) {
                    tank->shieldTimeLeft = 4;
                }
                if (tank->powerUp) {
                    for (int k = 0; k < MAX_POWERUP_COUNT; k++) {
                        if (game.powerUps[k].state == PUSActive) {
                            game.powerUps[k].state = PUSPickedUp;
                        }
                    }
                }
            }
        }
    }
    spawnTanks();
    checkStageEnd();
}

// static void drawFloat(float d, int x, int y) {
//     char buffer[20];
//     snprintf(buffer, 20, "%f", d);
//     drawText(buffer, x, y, 10, WHITE);
// }

static void gameOverCurtainLogic() {
    if (IsKeyPressed(KEY_ENTER)) {
        setScreen(GSTitle);
    }
}

static void drawGameOverCurtain() {
    Texture2D *tex = &game.textures.gameOverCurtain;
    int drawWidth = tex->width * 2;
    int drawHeight = tex->height * 2;
    int x = (SCREEN_WIDTH - drawWidth) / 2;
    int y = (SCREEN_HEIGHT - drawHeight) / 2;
    DrawTexturePro(*tex, (Rectangle){0, 0, tex->width, tex->height},
                   (Rectangle){x, y, drawWidth, drawHeight}, (Vector2){}, 0,
                   WHITE);
}

static void congratsLogic() {
    if (IsKeyPressed(KEY_ENTER)) {
        setScreen(GSTitle);
    }
}

static void drawCongrats() {
    int topY = SCREEN_HEIGHT / 3;
    static const int N = 256;
    char text[N];
    int score = MAX(game.playerScores[TPlayer1].totalScore,
                    game.playerScores[TPlayer2].totalScore);
    char *congratsText = "CONGRATULATIONS!";

    drawText(congratsText, centerX(measureText(congratsText, FONT_SIZE * 2)),
             topY, FONT_SIZE * 2, (Color){255, 0, 0, 255});

    topY += 200;

    snprintf(text, N, "HI-SCORE  %7d", score);
    int x = centerX(measureText(text, FONT_SIZE));
    drawText("SCORE", x, topY, FONT_SIZE, (Color){205, 62, 26, 255});
    snprintf(text, N, "%7d", score);
    drawText(text, x + measureText("HI-SCORE  ", FONT_SIZE), topY, FONT_SIZE,
             (Color){241, 159, 80, 255});
}

static void stageSummaryLogic() {
    game.stageSummary.time += game.frameTime;
    if (IsKeyPressed(KEY_ENTER)) {
        if (game.gameOverTime) {
#ifndef ALT_ASSETS
            PlaySound(game.sounds.game_over);
#endif
            setScreen(GSGameOver);
        } else if (game.stage == LEVEL_COUNT) {
            setScreen(GSCongrats);
        } else {
            initStage(game.stage + 1);
            setScreen(GSPlay);
        }
    }
}

static void drawStageSummary() {
    int topY = SCREEN_HEIGHT -
               (SCREEN_HEIGHT - 30) *
                   (MIN(game.stageSummary.time, STAGE_SUMMARY_SLIDE_TIME) /
                    STAGE_SUMMARY_SLIDE_TIME);
    static const int N = 256;
    char text[N];

    snprintf(text, N, "HI-SCORE  %7d", game.hiScore);
    int x = centerX(measureText(text, FONT_SIZE));
    drawText("HI-SCORE", x, topY, FONT_SIZE, (Color){205, 62, 26, 255});
    snprintf(text, N, "%7d", game.hiScore);
    drawText(text, x + measureText("HI-SCORE  ", FONT_SIZE), topY, FONT_SIZE,
             (Color){241, 159, 80, 255});
    topY += 70;

    snprintf(text, N, "STAGE %2d", game.stage);
    drawText(text, centerX(measureText(text, FONT_SIZE)), topY, FONT_SIZE,
             WHITE);

    int linePadding = 40;
    int halfWidth = SCREEN_WIDTH / 2;
    int pX = (halfWidth - measureText("I-PLAYER", FONT_SIZE)) / 2;
    drawText("I-PLAYER", pX, topY + FONT_SIZE + linePadding, FONT_SIZE,
             (Color){205, 62, 26, 255});

    // Player score
    snprintf(text, N, "%d", game.playerScores[TPlayer1].totalScore);
    drawText(text, (halfWidth - measureText(text, FONT_SIZE) - pX),
             topY + (FONT_SIZE + linePadding) * 2, FONT_SIZE,
             (Color){241, 159, 80, 255});

    pX = (halfWidth - measureText("II-PLAYER", FONT_SIZE)) / 2;
    drawText("II-PLAYER", halfWidth + pX, topY + FONT_SIZE + linePadding,
             FONT_SIZE, (Color){205, 62, 26, 255});

    // Player score
    snprintf(text, N, "%d", game.playerScores[TPlayer2].totalScore);
    drawText(text, (halfWidth + pX), topY + (FONT_SIZE + linePadding) * 2,
             FONT_SIZE, (Color){241, 159, 80, 255});

    int arrowWidth = game.textures.leftArrow.width;
    int arrowDrawWidth = arrowWidth * 4;
    int arrowHeight = game.textures.leftArrow.height;
    int arrowDrawHeight = arrowHeight * 4;
    int player1TotalKills = 0;
    int player2TotalKills = 0;
    for (int i = 2; i < TMax; i++) {
        int y = topY + (FONT_SIZE + linePadding) * (i + 1);
        Texture2D *tex = game.tankSpecs[i].texture;
        int texX = 0;
        int texY = game.tankSpecs[i].texRow * TANK_TEXTURE_SIZE;
        int drawSize = TANK_TEXTURE_SIZE * 4;
        int drawOffset = (TANK_SIZE - drawSize) / 2;
        DrawTexturePro(
            *tex, (Rectangle){texX, texY, TANK_TEXTURE_SIZE, TANK_TEXTURE_SIZE},
            (Rectangle){halfWidth - (drawSize / 2) + drawOffset,
                        y - (drawSize - FONT_SIZE) / 2 + drawOffset, drawSize,
                        drawSize},
            (Vector2){}, 0, WHITE);
        DrawTexturePro(
            game.textures.leftArrow, (Rectangle){0, 0, arrowWidth, arrowHeight},
            (Rectangle){halfWidth - (drawSize / 2) - 10 - arrowDrawWidth,
                        y - (arrowDrawHeight - FONT_SIZE) / 2, arrowDrawWidth,
                        arrowDrawHeight},
            (Vector2){}, 0, WHITE);

        int kills = game.playerScores[TPlayer1].kills[i];
        player1TotalKills += kills;
        snprintf(text, N, "%4d PTS  %2d", kills * game.tankSpecs[i].points,
                 kills);
        drawText(text, halfWidth - measureText(text, FONT_SIZE) - 100, y,
                 FONT_SIZE, WHITE);

        DrawTexturePro(game.textures.rightArrow,
                       (Rectangle){0, 0, arrowWidth, arrowHeight},
                       (Rectangle){halfWidth + (drawSize / 2) + 10,
                                   y - (arrowDrawHeight - FONT_SIZE) / 2,
                                   arrowDrawWidth, arrowDrawHeight},
                       (Vector2){}, 0, WHITE);

        kills = game.playerScores[TPlayer2].kills[i];
        player2TotalKills += kills;
        snprintf(text, N, "%2d  %4d PTS", kills,
                 kills * game.tankSpecs[i].points);
        drawText(text, halfWidth + 100, y, FONT_SIZE, WHITE);
    }
    snprintf(text, N, "TOTAL %2d", player1TotalKills);
    drawText(text, halfWidth - measureText(text, FONT_SIZE) - 100,
             topY + (FONT_SIZE + linePadding) * (TMax + 1), FONT_SIZE, WHITE);

    snprintf(text, N, "%2d", player2TotalKills);
    drawText(text, halfWidth + 100,
             topY + (FONT_SIZE + linePadding) * (TMax + 1), FONT_SIZE, WHITE);
}

static void titleLogic() {
    game.title.time += game.frameTime;
    if (IsKeyPressed(KEY_ENTER)) {
        game.title = (Title){0};
        setScreen(GSPlay);
        initGameRun();
        initStage(1);
    }
}

static void drawTitle() {
    int topY = 150;
    Texture2D *tex = &game.textures.title;
    int titleTexHeight = tex->height;
    int x = (SCREEN_WIDTH - tex->width * 2) / 2;
    int y = SCREEN_HEIGHT -
            (SCREEN_HEIGHT - topY) *
                (MIN(game.title.time, TITLE_SLIDE_TIME) / TITLE_SLIDE_TIME);
    static const int N = 256;
    char text[N];
    snprintf(text, N, "HI-SCORE   %7d", game.hiScore);
    drawText(text, centerX(measureText(text, FONT_SIZE)), y - 70, FONT_SIZE,
             WHITE);
    DrawTexturePro(*tex, (Rectangle){0, 0, tex->width, titleTexHeight},
                   (Rectangle){x, y, tex->width * 2, titleTexHeight * 2},
                   (Vector2){}, 0, WHITE);
}

static void gameLogic() {
    if (!game.stageCurtainTime) {
        if (IsKeyPressed(KEY_ENTER)) {
            game.stageCurtainTime = 0.001;
        }
    }
    if (game.stageCurtainTime && !game.isStageCurtainSoundPlayed) {
        PlaySound(game.sounds.start_menu);
        game.isStageCurtainSoundPlayed = true;
    }
    if (game.stageCurtainTime && game.stageCurtainTime < STAGE_CURTAIN_TIME) {
        game.stageCurtainTime += game.frameTime;
    }
    if (game.stageCurtainTime < STAGE_CURTAIN_TIME) return;
    if (IsKeyPressed(KEY_ENTER)) {
        game.isPaused = !game.isPaused;
        if (game.isPaused) {
            PlaySound(game.sounds.game_pause);
        }
    }
    if (game.isPaused) return;
    if (game.gameOverTime &&
        game.gameOverTime < GAME_OVER_SLIDE_TIME + GAME_OVER_DELAY) {
        game.gameOverTime += game.frameTime;
    }
    game.timeSinceSpawn += game.frameTime;
    if (game.timerPowerUpTimeLeft > 0) {
        game.timerPowerUpTimeLeft -= game.frameTime;
    }
    if (game.tanks[TPlayer1].shieldTimeLeft > 0) {
        game.tanks[TPlayer1].shieldTimeLeft -= game.frameTime;
    }
    if (game.tanks[TPlayer2].shieldTimeLeft > 0) {
        game.tanks[TPlayer2].shieldTimeLeft -= game.frameTime;
    }
    if (game.tanks[TPlayer1].immobileTimeLeft > 0) {
        game.tanks[TPlayer1].immobileTimeLeft -= game.frameTime;
    }
    if (game.tanks[TPlayer2].immobileTimeLeft > 0) {
        game.tanks[TPlayer2].immobileTimeLeft -= game.frameTime;
    }
    if (game.tanks[TPlayer1].slidingTimeLeft > 0) {
        game.tanks[TPlayer1].slidingTimeLeft -= game.frameTime;
    }
    if (game.tanks[TPlayer2].slidingTimeLeft > 0) {
        game.tanks[TPlayer2].slidingTimeLeft -= game.frameTime;
    }
    handleInput();
    handleAI();
    updateGameState();
}

static void saveHiScore() {
    u8 bytes[4];
    bytes[0] = game.hiScore & 0xFF;
    bytes[1] = (game.hiScore >> 8) & 0xFF;
    bytes[2] = (game.hiScore >> 16) & 0xFF;
    bytes[3] = (game.hiScore >> 24) & 0xFF;
    Buffer b = {bytes, 4};
    saveBuffer(b, "hiscore");
}

#ifdef ALT_ASSETS
static void playMusic() {
    static bool isFirstTime = true;
#define currentSoundtrack \
    (game.sounds.soundtrack[game.soundtrack * 4 + game.soundtrackPhase])
#define dieSoundtrack \
    (game.sounds.soundtrack[ASIZE(game.sounds.soundtrack) - 1])
    if (isFirstTime) {
        isFirstTime = false;
        PlaySound(game.sounds.soundtrack[0]);
        return;
    }
    if (game.gameOverTime) {
        if (IsSoundPlaying(dieSoundtrack)) return;
        if (!game.isDieSoundtrackPlayed) {
            StopSound(currentSoundtrack);
            PlaySound(dieSoundtrack);
            game.isDieSoundtrackPlayed = true;
        }
    }
    if (IsSoundPlaying(currentSoundtrack) || IsSoundPlaying(dieSoundtrack))
        return;
    char track = game.screen == GSPlay ? (game.stage - 1) % 4 + 1 : 0;
    if (game.soundtrack != track) {
        game.soundtrackPhase = 0;
    } else {
        game.soundtrackPhase++;
        game.soundtrackPhase %= 4;
    }
    game.soundtrack = track;
    PlaySound(currentSoundtrack);
}
#endif

int main(void) {
    srand(time(0));

    SetTraceLogLevel(LOG_NONE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Battle City 4000");
    SetTargetFPS(60);

    RenderTexture2D renderTexture =
        LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);

    int display = GetCurrentMonitor();
    SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
    ToggleFullscreen();

    InitAudioDevice();

    initGame();
    setScreen(GSTitle);

    SetExitKey(0);
    while (!WindowShouldClose()) {
        game.totalTime = GetTime();

        game.logic();
        BeginTextureMode(renderTexture);
        ClearBackground(BLACK);
        game.draw();
        EndTextureMode();
        BeginDrawing();
        int sw = GetScreenWidth();
        int sh = sw * ((float)SCREEN_HEIGHT / SCREEN_WIDTH);
        DrawTexturePro(renderTexture.texture,
                       (Rectangle){0, 0, SCREEN_WIDTH, -SCREEN_HEIGHT},
                       (Rectangle){0, (GetScreenHeight() - sh) / 2, sw, sh},
                       (Vector2){0, 0}, 0, WHITE);
        EndDrawing();
        game.frameTime = GetFrameTime();
#ifdef ALT_ASSETS
        playMusic();
#endif
    }

    UnloadFont(game.font);

    saveHiScore();

    CloseAudioDevice();
    CloseWindow();

    return 0;
}
