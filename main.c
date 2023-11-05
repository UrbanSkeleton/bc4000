#include "raylib.h"
#include "raymath.h"

#include "utils.c"

// #define DRAW_CELL_GRID

const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 1000;
const int FIELD_COLS = 60;
const int FIELD_ROWS = 60;
const int CELL_SIZE = 15;
const int PLAYER1_START_COL = 20;
const int PLAYER2_START_COL = 36;
const int PLAYER_SPEED = 100;

typedef enum { TPlayer1, TPlayer2, TBasic } TankType;
typedef enum { DLeft, DRight, DUp, DDown } Direction;

Direction rotations[4] = {270, 90, 0, 180};

typedef struct {
    TankType type;
    Vector2 pos;
    Vector2 speed;
    Direction direction;
    Texture2D *texture;
} Tank;

typedef enum {
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
} Cell;

typedef struct {
    Texture2D flag;
    Texture2D brick;
    Texture2D concrete;
    Texture2D forest;
    Texture2D river;
    Texture2D blank;
    Texture2D player1Tank;
} Textures;

typedef struct {
    Cell field[FIELD_ROWS][FIELD_COLS];
    Tank tanks[100];
    Vector2 flagPos;
    int tankCount;
    Texture2D *cellTextures[CTMax];
    Textures textures;
} Game;

static Game game;
// static Rectangle textureRectangle = {0, 0, CELL_SIZE * 2, CELL_SIZE * 2};

void drawCell(Cell *cell) {
    Texture2D *tex = game.cellTextures[cell->type];
    DrawTexturePro(*tex, (Rectangle){0, 0, tex->width, tex->height},
                   (Rectangle){cell->pos.x, cell->pos.y, CELL_SIZE, CELL_SIZE},
                   (Vector2){}, 0, WHITE);
#ifdef DRAW_CELL_GRID
    DrawRectangleLines(cell->pos.x, cell->pos.y, CELL_SIZE, CELL_SIZE, BLACK);
#endif
}

void drawField() {
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            drawCell(&game.field[i][j]);
        }
    }
}

void drawTank(Tank *tank) {
    Texture2D *tex = tank->texture;
    float rotation = rotations[tank->direction];
    DrawTexturePro(
        *tex, (Rectangle){0, 0, tex->width, tex->height},
        (Rectangle){tank->pos.x, tank->pos.y, CELL_SIZE * 4, CELL_SIZE * 4},
        (Vector2){CELL_SIZE * 2, CELL_SIZE * 2}, rotation, WHITE);
}

void drawTanks() {
    for (int i = 0; i < game.tankCount; i++) {
        drawTank(&game.tanks[i]);
    }
}

void drawFlag() {
    Texture2D *tex = &game.textures.flag;
    DrawTexturePro(*tex, (Rectangle){0, 0, tex->width, tex->height},
                   (Rectangle){game.flagPos.x, game.flagPos.y, CELL_SIZE * 4,
                               CELL_SIZE * 4},
                   (Vector2){}, 0, WHITE);
}

void drawGame() {
    drawField();
    drawTanks();
    drawFlag();
}

void loadTextures() {
    game.textures.flag = LoadTexture("textures/flag.png");
    game.textures.brick = LoadTexture("textures/brick.png");
    game.cellTextures[CTBrick] = &game.textures.brick;
    game.textures.concrete = LoadTexture("textures/concrete.png");
    game.cellTextures[CTConcrete] = &game.textures.concrete;
    game.textures.forest = LoadTexture("textures/forest.png");
    game.cellTextures[CTForest] = &game.textures.forest;
    game.textures.river = LoadTexture("textures/river.png");
    game.cellTextures[CTRiver] = &game.textures.river;
    game.textures.blank = LoadTexture("textures/blank.png");
    game.cellTextures[CTBlank] = &game.textures.blank;
    game.textures.player1Tank = LoadTexture("textures/player1Tank.png");
}

void loadField(const char *filename) {
    Buffer buf = readFile("levels/1.level");
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            int index = (i / 4) * (FIELD_COLS / 4 + 1) + (j / 4);
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
            index++;
        }
    }
}

void initGame() {
    loadTextures();
    for (int i = 0; i < FIELD_ROWS; i++) {
        for (int j = 0; j < FIELD_COLS; j++) {
            game.field[i][j] =
                (Cell){.type = CTBlank,
                       .pos = (Vector2){j * CELL_SIZE, i * CELL_SIZE}};
        }
    }
    loadField("levels/1.level");
    game.tanks[0] = (Tank){.type = TPlayer1,
                           .pos = (Vector2){CELL_SIZE * PLAYER1_START_COL,
                                            CELL_SIZE * (FIELD_ROWS - 4)},
                           .texture = &game.textures.player1Tank};
    game.tankCount = 1;
    game.flagPos = (Vector2){CELL_SIZE * (FIELD_COLS / 2 - 2),
                             CELL_SIZE * (FIELD_ROWS - 4)};
}

void handleInput() {
    Tank *t = &game.tanks[0];
    t->speed.x = t->speed.y = 0;
    if (IsKeyDown(KEY_RIGHT)) {
        t->speed.x = PLAYER_SPEED;
        t->direction = DRight;
    } else if (IsKeyDown(KEY_LEFT)) {
        t->speed.x = -PLAYER_SPEED;
        t->direction = DLeft;
    } else if (IsKeyDown(KEY_UP)) {
        t->speed.y = -PLAYER_SPEED;
        t->direction = DUp;
    } else if (IsKeyDown(KEY_DOWN)) {
        t->speed.y = PLAYER_SPEED;
        t->direction = DDown;
    }
}

void updateGameState(float time) {
    game.tanks[0].pos.x += time * game.tanks[0].speed.x;
    game.tanks[0].pos.y += time * game.tanks[0].speed.y;
}

int main(void) {

    SetTraceLogLevel(LOG_NONE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Battle City 4000");

    initGame();

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        handleInput();
        updateGameState(GetFrameTime());
        BeginDrawing();
        ClearBackground(BLACK);
        drawGame();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
