#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <stdbool.h>

#include "constants.h"
#include "networkHeaders.h"
#include "raylib.h"

typedef struct {
    int row;
    int col;
} CellInfo;

const CellInfo fortressWall[] = {
    {13 * 4 - 6 + 2, 5 * 4 + 2 + 4}, {13 * 4 - 5 + 2, 5 * 4 + 2 + 4},
    {13 * 4 - 4 + 2, 5 * 4 + 2 + 4}, {13 * 4 - 3 + 2, 5 * 4 + 2 + 4},
    {13 * 4 - 2 + 2, 5 * 4 + 2 + 4}, {13 * 4 - 1 + 2, 5 * 4 + 2 + 4},

    {13 * 4 - 6 + 2, 5 * 4 + 3 + 4}, {13 * 4 - 5 + 2, 5 * 4 + 3 + 4},
    {13 * 4 - 4 + 2, 5 * 4 + 3 + 4}, {13 * 4 - 3 + 2, 5 * 4 + 3 + 4},
    {13 * 4 - 2 + 2, 5 * 4 + 3 + 4}, {13 * 4 - 1 + 2, 5 * 4 + 3 + 4},

    {13 * 4 - 6 + 2, 5 * 4 + 8 + 4}, {13 * 4 - 5 + 2, 5 * 4 + 8 + 4},
    {13 * 4 - 4 + 2, 5 * 4 + 8 + 4}, {13 * 4 - 3 + 2, 5 * 4 + 8 + 4},
    {13 * 4 - 2 + 2, 5 * 4 + 8 + 4}, {13 * 4 - 1 + 2, 5 * 4 + 8 + 4},

    {13 * 4 - 6 + 2, 5 * 4 + 9 + 4}, {13 * 4 - 5 + 2, 5 * 4 + 9 + 4},
    {13 * 4 - 4 + 2, 5 * 4 + 9 + 4}, {13 * 4 - 3 + 2, 5 * 4 + 9 + 4},
    {13 * 4 - 2 + 2, 5 * 4 + 9 + 4}, {13 * 4 - 1 + 2, 5 * 4 + 9 + 4},

    {13 * 4 - 6 + 2, 6 * 4 + 0 + 4}, {13 * 4 - 5 + 2, 6 * 4 + 0 + 4},
    {13 * 4 - 6 + 2, 6 * 4 + 1 + 4}, {13 * 4 - 5 + 2, 6 * 4 + 1 + 4},

    {13 * 4 - 6 + 2, 6 * 4 + 2 + 4}, {13 * 4 - 5 + 2, 6 * 4 + 2 + 4},
    {13 * 4 - 6 + 2, 6 * 4 + 3 + 4}, {13 * 4 - 5 + 2, 6 * 4 + 3 + 4},
};

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
} TankSpec;

typedef enum {
    PUStar,
    PUTank,
    PUGrenade,
    PUTimer,
    PUShield,
    PUShovel,
    PUMax,
} PowerUpType;

typedef struct {
    Texture2D *texture;
    int texCol;
} PowerUpSpec;

typedef enum { PUSPending, PUSActive, PUSPickedUp } PowerUpState;

typedef struct {
    PowerUpType type;
    Vector2 pos;
    PowerUpState state;
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
} Tank;

typedef struct {
    bool fire;
    bool move;
    Direction direction;
} Command;

typedef struct {
    Command command;
    int number;
    float dt;
} ClientCommand;

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
    Texture2D lan;
} Textures;

typedef enum {
    SFX_BIG_EXPLOSION,
    SFX_BULLET_EXPLOSION,
    SFX_BULLET_HIT_1,
    SFX_BULLET_HIT_2,
    SFX_GAME_OVER,
    SFX_GAME_PAUSE,
    SFX_MODE_SWITCH,
    SFX_PLAYER_FIRE,
    SFX_POWERUP_APPEAR,
    SFX_POWERUP_PICK,
    SFX_START_MENU,
    SFX_MAX,
} SfxType;

typedef struct {
    Sound sfx[SFX_MAX];
    Sound soundtrack[21];
} Sounds;

typedef enum {
    MNone,
    MOnePlayer,
    MTwoPlayers,
    MLan,
    MMax,
} MenuSelectedItem;

typedef struct {
    float time;
    MenuSelectedItem menuSelecteItem;
} Title;

typedef enum {
    LMNone,
    LMHostGame,
    LMJoinGame,
    LMBack,
    LMMax,
} LanMenuSelectedItem;

typedef struct {
    LanMenuSelectedItem lanMenuSelectedItem;
} LanMenu;

typedef struct {
    float time;
} StageSummary;

typedef enum { GMOnePlayer, GMTwoPlayers, GMLan } GameMode;

typedef enum { LServer, LClient } LanMode;

// typedef struct {
//     struct sockaddr_in serverAddress, clientAdress;
//     socklen_t addressLen = sizeof(clientAdress);
// } LanServer;

// typedef struct {
//     struct sockaddr_in broadcastAddress, clientAdress, serverAddress;
//     socklen_t addressLen = sizeof(clientAdress);
// } LanClient;

typedef int Socket;

typedef struct {
    LanMode lanMode;
    Socket socket;
    struct sockaddr_in broadcastAddress, clientAddress, serverAddress;
    socklen_t addressLength;
    int availableGames;
    int selectedAddressIndex;
    struct sockaddr_in joinableAddresses[MAX_AVAILABLE_GAMES];
    int clientInput[CLIENT_INPUT_SIZE / sizeof(int)];
    float timeout;
    float timeoutScreenTime;
    ClientCommand clientCommands[MAX_CLIENT_COMMANDS];
    int clientCommandIndexFirst;
    int clientCommandIndexLast;
    float lastPacketTick;
    int inputNumber;
} Lan;

typedef enum {
    GSTitle,
    GSLan,
    GSHostGame,
    GSJoinGame,
    GSPlay,
    GSPlayLan,
    GSScore,
    GSScoreLan,
    GSGameOver,
    GSCongrats,
    GSTimedOut,
} GameScreen;

typedef struct {
    void (*logic)(void);
    void (*draw)(void);
} GameFunctions;

typedef struct {
    int screenWidth;
    int screenHeight;
    Camera2D camera;
    Cell field[FIELD_ROWS][FIELD_COLS];
    Tank tanks[MAX_TANK_COUNT];
    TankSpec tankSpecs[TMax];
    Bullet bullets[MAX_BULLET_COUNT];
    Vector2 flagPos;
    bool isFlagDead;
    CellSpec cellSpecs[CTMax];
    PowerUpSpec powerUpSpecs[PUMax];
    Animation explosionAnimations[ETMax];
    Explosion explosions[MAX_EXPLOSION_COUNT];
    ScorePopup scorePopups[MAX_SCORE_POPUP_COUNT];
    Textures textures;
    Sounds sounds;
    SfxType sfxPlayed[MAX_SFX_PLAYED];
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
    float shovelPowerUpTimeLeft;
    void (*logic)();
    void (*draw)();
    Title title;
    LanMenu lanMenu;
    Lan lan;
    StageSummary stageSummary;
    GameMode mode;
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
    bool proceed;
    bool mute;
    bool fullscreen;
    float tick;
} Game;

#endif