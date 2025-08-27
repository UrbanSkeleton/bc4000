#include <assert.h>
#include <string.h>
#include <time.h>

#include "constants.h"
#include "dataTypes.h"
#include "gamePackager.h"
#include "networkHeaders.h"
#include "raylib.h"
#include "utils.h"

// #define DRAW_CELL_GRID
// #define ALT_ASSETS

#ifdef ALT_ASSETS
#define ASSETDIR "alt"
#define SOUND_EXT "wav"
#else
#define ASSETDIR "original"
#define SOUND_EXT "ogg"
#endif

static void gameLogic();
static void lanGameLogic();
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
static void lanMenuLogic();
static void drawLanMenu();
static void initHostGame();
static void hostGameLogic();
static void drawHostGame();
static void initJoinGame();
static void joinGameLogic();
static void drawJoinGame();

static GameFunctions gameFunctions[] = {
    {.logic = titleLogic, .draw = drawTitle},
    {.logic = lanMenuLogic, .draw = drawLanMenu},
    {.logic = hostGameLogic, .draw = drawHostGame},
    {.logic = joinGameLogic, .draw = drawJoinGame},
    {.logic = gameLogic, .draw = drawGame},
    {.logic = lanGameLogic, .draw = drawGame},
    {.logic = stageSummaryLogic, .draw = drawStageSummary},
    {.logic = gameOverCurtainLogic, .draw = drawGameOverCurtain},
    {.logic = congratsLogic, .draw = drawCongrats},
};

static Game game;

static void drawText(const char *text, int x, int y, int fontSize,
                     Color color) {
    DrawTextEx(game.font, text, (Vector2){x, y}, fontSize, 2, color);
}

static int measureText(const char *text, int fontSize) {
    return MeasureTextEx(game.font, text, fontSize, 2).x;
}

static bool isEnemy(Tank *t) { return game.tankSpecs[t->type].isEnemy; }

static void drawCell(Cell *cell) {
    Texture2D *tex = game.cellSpecs[cell->type].texture;
    int w = tex->width / 4;
    int h = tex->height / 4;
    DrawTexturePro(*tex, (Rectangle){cell->texCol * w, cell->texRow * h, w, h},
                   (Rectangle){cell->pos.x, cell->pos.y, CELL_SIZE, CELL_SIZE},
                   (Vector2){}, 0, WHITE);
#ifdef DRAW_CELL_GRID
    DrawRectangleLines(cell->pos.x, cell->pos.y, CELL_SIZE, CELL_SIZE, BLUE);
#endif
}

static void drawField() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (game.field[i][j].type != CTForest) drawCell(&game.field[i][j]);
        }
    }
}

static void drawForest() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (game.field[i][j].type == CTForest) drawCell(&game.field[i][j]);
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
    int i = tank->spawningTime / (SPAWNING_TIME / ASIZE(textureCols));
    if (i >= ASIZE(textureCols)) i = ASIZE(textureCols) - 1;
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
    Texture2D *tex =
        game.isFlagDead ? &game.textures.deadFlag : &game.textures.flag;
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
        if (game.uiElements[i].isVisible) drawUIElement(&game.uiElements[i]);
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
    drawFlag();
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
    snprintf(filename, 50, "levels/stage%.2d", stage);
    Buffer buf = readFile(filename);
    int ci = 0;
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            if (i <= 1 || i >= FIELD_ROWS - 2 || j <= 3 ||
                j >= FIELD_COLS - 8) {
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
    if (game.mode == GMTwoPlayers || game.mode == GMLan) {
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

static void spawnPlayer(Tank *t, bool resetTier) {
    t->pos = t->type == TPlayer1 ? PLAYER1_START_POS : PLAYER2_START_POS;
    t->direction = DUp;
    t->status = TSSpawning;
    t->shieldTimeLeft = 4;
    t->immobileTimeLeft = 0;
    t->firedBulletCount = 0;
    t->isMoving = false;
    if (resetTier) {
        t->tier = 0;
        game.tankSpecs[t->type].bulletSpeed = BULLET_SPEEDS[0];
        game.tankSpecs[t->type].maxBulletCount = 1;
        game.tankSpecs[t->type].texRow = 0;
    }
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
    game.shovelPowerUpTimeLeft = 0;
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
    if (game.mode == GMTwoPlayers || game.mode == GMLan) {
        spawnPlayer(&game.tanks[TPlayer2], false);
    }
    static char startingCols[3] = {4, 4 + (FIELD_COLS - 12) / 4 / 2 * 4,
                                   FIELD_COLS - 8 - 4};
    for (int i = 0; i < MAX_ENEMY_COUNT; i++) {
        TankType type = levelTanks[stage - 1][i];
        game.tanks[i + 2] = (Tank){
            .type = type,
            .pos = (Vector2){CELL_SIZE * startingCols[i % 3], CELL_SIZE * 2},
            .direction = DDown,
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
    game.isFlagDead = false;
    game.tanks[TPlayer1] = (Tank){.type = TPlayer1, .lifes = 2};
    game.tanks[TPlayer2] = (Tank){.type = TPlayer2, .lifes = 2};
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
    game.flagPos = (Vector2){CELL_SIZE * ((FIELD_COLS - 12) / 2 - 2 + 4),
                             CELL_SIZE * (FIELD_ROWS - 4 - 2)};
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
    game.powerUpSpecs[PUShield] =
        (PowerUpSpec){.texture = &game.textures.powerups, .texCol = 5};
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
    return collision(t->pos.x, t->pos.y, TANK_SIZE, TANK_SIZE, game.flagPos.x,
                     game.flagPos.y, FLAG_SIZE, FLAG_SIZE);
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

static void updatePlayerLifesUI() {
    game.uiElements[UIP1Lifes].textureSrc =
        digitTextureRect(game.tanks[0].lifes);
    game.uiElements[UIP2Lifes].textureSrc =
        digitTextureRect(game.tanks[1].lifes);
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

static void destroyAllTanks() {
    for (int i = 0; i < MAX_TANK_COUNT; i++) {
        Tank *t = &game.tanks[i + 2];
        if (t->status == TSActive) destroyTank(t, false);
    }
    PlaySound(game.sounds.bullet_explosion);
}

static void addScore(TankType type, int score) {
    game.playerScores[type].totalScore += score;
    game.hiScore = MAX(game.hiScore, game.playerScores[type].totalScore);
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
                case PUTank:
                    t->lifes++;
                    updatePlayerLifesUI();
                    break;
                case PUStar:
                    if (t->tier == 3) return;
                    t->tier++;
                    game.tankSpecs[t->type].texRow++;
                    switch (t->tier) {
                        case 1:
                            game.tankSpecs[t->type].bulletSpeed =
                                BULLET_SPEEDS[2];
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
                case PUTimer:
                    game.timerPowerUpTimeLeft = TIMER_TIME;
                    break;
                case PUShield:
                    t->shieldTimeLeft = SHIELD_TIME;
                    break;
                case PUShovel:
                    game.shovelPowerUpTimeLeft = SHOVEL_TIME;
                    for (int i = 0; i < ASIZE(fortressWall); i++) {
                        game.field[fortressWall[i].row][fortressWall[i].col]
                            .type = CTConcrete;
                        game.field[fortressWall[i].row][fortressWall[i].col]
                            .texRow = fortressWall[i].row % 2;
                        game.field[fortressWall[i].row][fortressWall[i].col]
                            .texCol = fortressWall[i].col % 2;
                    }
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
    static Direction dirs[] = {DDown,  DDown, DDown, DDown, DRight,
                               DRight, DLeft, DLeft, DUp};
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

static void handleClientInput(TankType type) {
    Command cmd = {};
    if (game.lan.clientInput[0]) {
        cmd.move = true;
        cmd.direction = DRight;
    } else if (game.lan.clientInput[1]) {
        cmd.move = true;
        cmd.direction = DLeft;
    } else if (game.lan.clientInput[2]) {
        cmd.move = true;
        cmd.direction = DUp;
    } else if (game.lan.clientInput[3]) {
        cmd.move = true;
        cmd.direction = DDown;
    }
    if (game.lan.clientInput[4]) {
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
    if (game.mode == GMTwoPlayers) {
        handlePlayerInput(TPlayer2);
    } else if (game.mode == GMLan) {
        handleClientInput(TPlayer2);
    }
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
    updatePlayerLifesUI();
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
        if (!isEnemy(b->tank) && !isEnemy(t)) {
            t->immobileTimeLeft = IMMOBILE_TIME;
            break;
        }
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

static void destroyFlag() {
    createExplosion(ETBig, game.flagPos, FLAG_SIZE, -1);
    game.isFlagDead = true;
}

static bool checkFlagHit(Bullet *b) {
    if (!game.gameOverTime &&
        collision(b->pos.x, b->pos.y, BULLET_SIZE, BULLET_SIZE, game.flagPos.x,
                  game.flagPos.y, FLAG_SIZE, FLAG_SIZE)) {
        destroyBullet(b, true);
        destroyFlag();
        PlaySound(game.sounds.big_explosion);
        return true;
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
            tank->spawningTime += game.frameTime;
            if (tank->spawningTime >= SPAWNING_TIME) {
                tank->spawningTime = 0;
                tank->status = TSActive;
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
            if (game.mode == GMLan)
                setScreen(GSPlayLan);
            else
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

    if (game.mode == GMTwoPlayers || game.mode == GMLan) {
        int pX = (halfWidth - measureText("II-PLAYER", FONT_SIZE)) / 2;
        drawText("II-PLAYER", halfWidth + pX, topY + FONT_SIZE + linePadding,
                 FONT_SIZE, (Color){205, 62, 26, 255});

        // Player score
        snprintf(text, N, "%d", game.playerScores[TPlayer2].totalScore);
        drawText(text, (halfWidth + pX), topY + (FONT_SIZE + linePadding) * 2,
                 FONT_SIZE, (Color){241, 159, 80, 255});
    }
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
        if (game.mode == GMTwoPlayers || game.mode == GMLan) {
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
    }
    snprintf(text, N, "TOTAL %2d", player1TotalKills);
    drawText(text, halfWidth - measureText(text, FONT_SIZE) - 100,
             topY + (FONT_SIZE + linePadding) * (TMax + 1), FONT_SIZE, WHITE);
    if (game.mode == GMTwoPlayers || game.mode == GMLan) {
        snprintf(text, N, "%2d", player2TotalKills);
        drawText(text, halfWidth + 100,
                 topY + (FONT_SIZE + linePadding) * (TMax + 1), FONT_SIZE,
                 WHITE);
    }
}

static void titleLogic() {
    game.title.time += game.frameTime;
    if (game.title.time > TITLE_SLIDE_TIME &&
        game.title.menuSelecteItem == MNone) {
        game.title.menuSelecteItem = MOnePlayer;
    }
    if (IsKeyPressed(KEY_LEFT_SHIFT)) {
        PlaySound(game.sounds.mode_switch);
        game.title.time = TITLE_SLIDE_TIME;
        game.title.menuSelecteItem =
            game.title.menuSelecteItem % (MMax - 1) + 1;
    } else if (IsKeyPressed(KEY_ENTER)) {
        switch (game.title.menuSelecteItem) {
            case MOnePlayer:
                game.mode = GMOnePlayer;
                break;
            case MTwoPlayers:
                game.mode = GMTwoPlayers;
                break;
            case MLan:
                game.mode = GMLan;
                printf("Gamemode set to LAN\n");
                break;
            default:
                game.title.time = TITLE_SLIDE_TIME;
                return;
        }
        game.title = (Title){0};
        if (game.mode == GMLan) {
            setScreen(GSLan);
        } else {
            setScreen(GSPlay);
            initGameRun();
            initStage(1);
        }
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
    if (game.title.menuSelecteItem != MNone) {
        tex = &game.textures.player1Tank;
        int texX =
            (3 * 2 + ((long)(game.totalTime * 16) % 2)) * TANK_TEXTURE_SIZE;
        DrawTexturePro(
            *tex, (Rectangle){texX, 0, TANK_TEXTURE_SIZE, TANK_TEXTURE_SIZE},
            (Rectangle){x + 150,
                        topY + titleTexHeight * 2 - 174 +
                            (game.title.menuSelecteItem - 1) * 60,
                        TANK_TEXTURE_SIZE * 4, TANK_TEXTURE_SIZE * 4},
            (Vector2){}, 0, WHITE);
    }
}

static void lanMenuLogic() {
    if (IsKeyPressed(KEY_LEFT_SHIFT)) {
        PlaySound(game.sounds.mode_switch);
        game.lanMenu.lanMenuSelectedItem =
            game.lanMenu.lanMenuSelectedItem % (LMMax - 1) + 1;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        switch (game.lanMenu.lanMenuSelectedItem) {
            case LMHostGame:
                game.lan.lanMode = LServer;
                setScreen(GSHostGame);
                initHostGame();
                break;
            case LMJoinGame:
                game.lan.lanMode = LClient;
                setScreen(GSJoinGame);
                initJoinGame();
                break;
            default:
                return;
        }
    }
}

static void drawLanMenu() {
    static const int N = 256;
    char text[N];
    snprintf(text, N, "LAN MODE:");
    drawText(text, centerX(measureText(text, FONT_SIZE * 2)), 70, FONT_SIZE * 2,
             WHITE);

    snprintf(text, N, "HOST GAME");
    drawText(text, centerX(measureText(text, FONT_SIZE)), 500, FONT_SIZE,
             game.lanMenu.lanMenuSelectedItem == LMHostGame ? RED : WHITE);

    snprintf(text, N, "JOIN GAME");
    drawText(text, centerX(measureText(text, FONT_SIZE)), 650, FONT_SIZE,
             game.lanMenu.lanMenuSelectedItem == LMJoinGame ? RED : WHITE);
}

static void initHostGame() {
    game.lan.addressLength = sizeof(game.lan.clientAddress);

    if ((game.lan.socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket failed");
        exit(1);
    }

    fcntl(game.lan.socket, F_SETFL,
          fcntl(game.lan.socket, F_GETFL, 0) | O_NONBLOCK);

    int opt = 1;
    setsockopt(game.lan.socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(game.lan.socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

    memset(&game.lan.serverAddress, 0, sizeof(game.lan.serverAddress));
    game.lan.serverAddress.sin_family = AF_INET;
    game.lan.serverAddress.sin_addr.s_addr = INADDR_ANY;
    game.lan.serverAddress.sin_port = htons(PORT);

    if (bind(game.lan.socket, (struct sockaddr *)&game.lan.serverAddress,
             sizeof(game.lan.serverAddress)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    printf("Server is running on port %d\n", PORT);
}

static void hostGameLogic() {
    char buffer[BUFFER_SIZE];

    // checks all new packets in order.
    while (true) {
        int n = recvfrom(game.lan.socket, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&game.lan.clientAddress,
                         &game.lan.addressLength);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            perror("recvfrom");
            break;
        }

        buffer[n] = '\0';

        if (strcmp(buffer, "DISCOVER") == 0) {
            char reply[] = "AVAILABLE";
            sendto(game.lan.socket, reply, strlen(reply), 0,
                   (struct sockaddr *)&game.lan.clientAddress,
                   game.lan.addressLength);
            printf("Sent GAME_AVAILABLE to %s\n",
                   inet_ntoa(game.lan.clientAddress.sin_addr));
        } else if (strcmp(buffer, "JOIN_REQUEST") == 0) {
            char reply[] = "JOIN_ACCEPT";
            sendto(game.lan.socket, reply, strlen(reply), 0,
                   (struct sockaddr *)&game.lan.clientAddress,
                   game.lan.addressLength);
            printf("%s wants to join your game, sending join accept message\n",
                   inet_ntoa(game.lan.clientAddress.sin_addr));
            setScreen(GSPlayLan);
            initGameRun();
            initStage(1);
        }
    }
    // setScreen(GSPlay);
    // initGameRun();
    // initStage(1);
}

static void drawHostGame() {
    static const int N = 256;
    char text[N];
    snprintf(text, N, "Waiting for player...");
    drawText(text, centerX(measureText(text, FONT_SIZE * 1.5)),
             SCREEN_HEIGHT / 2, FONT_SIZE * 1.5, WHITE);
}

static void discoverGames() {
    memset(game.lan.joinableAddresses, 0, sizeof(game.lan.joinableAddresses));
    game.lan.availableGames = 0;
    game.lan.selectedAddressIndex = -1;

    char msg[] = "DISCOVER";
    sendto(game.lan.socket, msg, strlen(msg), 0,
           (struct sockaddr *)&game.lan.broadcastAddress,
           game.lan.addressLength);

    printf("Discovering joinable games...\n");
}

static void initJoinGame() {
    game.lan.addressLength = sizeof(game.lan.clientAddress);
    if ((game.lan.socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket failed");
        exit(1);
    }

    fcntl(game.lan.socket, F_SETFL,
          fcntl(game.lan.socket, F_GETFL, 0) | O_NONBLOCK);

    int opt = 1;
    setsockopt(game.lan.socket, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));

    memset(&game.lan.broadcastAddress, 0, sizeof(game.lan.broadcastAddress));
    game.lan.broadcastAddress.sin_family = AF_INET;
    game.lan.broadcastAddress.sin_port = htons(PORT);
    inet_pton(AF_INET, BROADCAST_IP, &game.lan.broadcastAddress.sin_addr);

    discoverGames();
}

static bool foundGame() {
    if (game.lan.availableGames < MAX_AVAILABLE_GAMES) {
        game.lan.availableGames++;
        return true;
    } else {
        return false;
    }
}

static void joinGameLogic() {
    char buffer[BUFFER_SIZE];

    // checks all new packets in order.
    while (true) {
        int n = recvfrom(game.lan.socket, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&game.lan.serverAddress,
                         &game.lan.addressLength);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            perror("recvfrom");
            break;
        }

        buffer[n] = '\0';

        if (strcmp(buffer, "AVAILABLE") == 0) {
            printf("Found available game from %s\n",
                   inet_ntoa(game.lan.serverAddress.sin_addr));
            if (foundGame()) {
                game.lan.joinableAddresses[game.lan.availableGames - 1] =
                    game.lan.serverAddress;
            }
        } else if (strcmp(buffer, "JOIN_ACCEPT") == 0) {
            printf("%s has accepted your join request, joining game now\n",
                   inet_ntoa(game.lan.serverAddress.sin_addr));
            setScreen(GSPlayLan);
            initGameRun();
            initStage(1);
        }
    }

    if (IsKeyPressed(KEY_LEFT_SHIFT)) {
        game.lan.selectedAddressIndex++;
        if (game.lan.selectedAddressIndex >= game.lan.availableGames)
            game.lan.selectedAddressIndex = -1;
    }

    if (IsKeyPressed(KEY_ENTER)) {
        if (game.lan.selectedAddressIndex == -1) {
            discoverGames();
        } else {
            char msg[] = "JOIN_REQUEST";
            sendto(game.lan.socket, msg, strlen(msg), 0,
                   (struct sockaddr *)&game.lan
                       .joinableAddresses[game.lan.selectedAddressIndex],
                   game.lan.addressLength);
        }
    }
}

static void drawJoinGame() {
    static const int N = 256;
    char text[N];
    snprintf(text, N, "Finding games");
    drawText(text, centerX(measureText(text, FONT_SIZE * 1.5)), 100,
             FONT_SIZE * 1.5, WHITE);

    int y = 400;
    snprintf(text, N, "Refresh");
    drawText(text, centerX(measureText(text, FONT_SIZE)), y, FONT_SIZE,
             game.lan.selectedAddressIndex == -1 ? RED : WHITE);

    for (int i = 0; i < game.lan.availableGames; i++) {
        y += 80;
        snprintf(text, N, "Game: %s",
                 inet_ntoa(game.lan.joinableAddresses[i].sin_addr));
        drawText(text, centerX(measureText(text, FONT_SIZE)), y, FONT_SIZE,
                 game.lan.selectedAddressIndex == i ? RED : WHITE);
    }
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
    if (game.shovelPowerUpTimeLeft > 0) {
        game.shovelPowerUpTimeLeft -= game.frameTime;
        if (game.shovelPowerUpTimeLeft <= 0) {
            for (int i = 0; i < ASIZE(fortressWall); i++) {
                game.field[fortressWall[i].row][fortressWall[i].col].type =
                    CTBrick;
            }
        }
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

static void lanGameClient() {
    char buffer[MAX_PACKET_SIZE + 1];
    while (true) {
        int n = recvfrom(game.lan.socket, buffer, MAX_PACKET_SIZE, 0,
                         (struct sockaddr *)&game.lan.serverAddress,
                         &game.lan.addressLength);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            perror("recvfrom");
            break;
        }
        buffer[n] = '\0';
        unpackGameState(&game, buffer);
    }

    memset(game.lan.clientInput, 0, CLIENT_INPUT_SIZE);

    if (IsKeyDown(controls[0].right)) game.lan.clientInput[0] = 1;
    if (IsKeyDown(controls[0].left)) game.lan.clientInput[1] = 1;
    if (IsKeyDown(controls[0].up)) game.lan.clientInput[2] = 1;
    if (IsKeyDown(controls[0].down)) game.lan.clientInput[3] = 1;
    if (IsKeyPressed(controls[0].fire)) game.lan.clientInput[4] = 1;
    if (IsKeyPressed(KEY_ENTER)) game.lan.clientInput[5] = 1;

    sendto(game.lan.socket, game.lan.clientInput, CLIENT_INPUT_SIZE, 0,
           (struct sockaddr *)&game.lan.serverAddress, game.lan.addressLength);
}

static void lanGameServerSend() {
    char buffer[MAX_PACKET_SIZE];
    packGameState(&game, buffer);

    sendto(game.lan.socket, buffer, MAX_PACKET_SIZE, 0,
           (struct sockaddr *)&game.lan.clientAddress, game.lan.addressLength);
}

static void lanGameServerRecieve() {
    char buffer[BUFFER_SIZE];
    while (true) {
        int n = recvfrom(game.lan.socket, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&game.lan.clientAddress,
                         &game.lan.addressLength);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            perror("recvfrom");
            break;
        }
        buffer[n] = '\0';
        memcpy(game.lan.clientInput, buffer, CLIENT_INPUT_SIZE);
    }
}

static void lanGameLogic() {
    if (game.lan.lanMode == LServer) {
        lanGameServerRecieve();
    } else {
        lanGameClient();
        return;
    }

    gameLogic();

    if (game.lan.lanMode == LServer) {
        lanGameServerSend();
    }
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
    SetWindowSize(GetMonitorWidth(display) / 2, GetMonitorHeight(display) / 2);
    // ToggleFullscreen();

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
        int sh = GetScreenHeight();
        int sw = sh * ((float)SCREEN_WIDTH / SCREEN_HEIGHT);
        DrawTexturePro(renderTexture.texture,
                       (Rectangle){0, 0, SCREEN_WIDTH, -SCREEN_HEIGHT},
                       (Rectangle){(GetScreenWidth() - sw) / 2, 0, sw, sh},
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
