#include "CS3113/Entity.h"
#include <string>

/**
 * Author: Tyler Davidovich
 * Assignment: Lunar Lander
 * Date due: 03/14/26
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

// Enums
enum GameStatus
{
    GAMEOVER,
    GAMEPLAYING,
    WINSCREEN,
    GRABWRONG
};

GameStatus gGameStatus = GAMEPLAYING;
// Global Constants

// Global Constants
constexpr int SCREEN_WIDTH = 1500,
              SCREEN_HEIGHT = 900,
              BOOKSPEED = 50,
              FPS = 120;

constexpr char BG_COLOUR[] = "#010101";
constexpr Vector2 ORIGIN = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2},
                  STARTINGPOINT = {200.0f, 200.0f};
constexpr int NUMBER_OF_TILES = 10;
constexpr float TILE_DIMENSION = 100.0f,
                // in m/ms², since delta time is in ms
    ACCELERATION_OF_GRAVITY = 10.0f,
                STARTINGACCELERATION = 10.0f,
                FIXED_TIMESTEP = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus = RUNNING;
float gPreviousTicks = 0.0f,
      gTimeAccumulator = 0.0f;

Entity *gLander = nullptr;
Entity *gSpell = nullptr;
Entity *gLibraryBackground = nullptr;
Entity *gFloatingBook = nullptr;
float Fuel = 50;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

// function definitions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Wacky Wizard");
    std::map<Direction, std::vector<int>> animationAtlas = {
        {UP, {0, 1, 2, 3}},
        {DOWN, {0, 1, 2, 3}},
        {LEFT, {3}},
        {RIGHT, {0}},
    };
    std::map<Direction, std::vector<int>> starAtlas = {
        {RIGHT, {0, 1, 2, 3, 4, 5, 6}},
    };
    float sizeRatio = .5f;
    gLander = new Entity(
        STARTINGPOINT,          // position
        {200.0f, 200.0f},       // scale
        "assets/game/Mage.png", // texture file address
        ATLAS,                  // single image or atlas?
        {4, 1},                 // atlas dimensions
        animationAtlas          // actual atlas
    );
    gFloatingBook = new Entity(
        ORIGIN,
        {
            200.0f,
            200.0f,
        },
        "assets/game/star.png");
    gSpell = new Entity(
        {ORIGIN.x, ORIGIN.y - 150.0f}, // position
        {40.0f, 40.0f},                // scale
        "assets/game/Spell.png",       // texture file address
        ATLAS,                         // single image or atlas?
        {7, 1},                        // atlas dimensions
        starAtlas                      // actual atlas
    );
    gLibraryBackground = new Entity(ORIGIN, {SCREEN_WIDTH, SCREEN_HEIGHT},
                                    "assets/game/image.png");
    gSpell->setDirection(RIGHT);
    gLander->setColliderDimensions({gLander->getScale().x / 2.0f,
                                    gLander->getScale().y / 2.0f});
    gLander->setFrameSpeed(5);
    gLander->setAcceleration({STARTINGACCELERATION, ACCELERATION_OF_GRAVITY});
    gSpell->setVelocity({0.0f, 200.0f});
    gSpell->setColliderDimensions({gSpell->getScale().x / 2.0f,
                                   gSpell->getScale().y / 2.0f});

    SetTargetFPS(FPS);
    gFloatingBook->setVelocity({-BOOKSPEED, 0});
}
void processInput()
{
    gLander->resetMovement();
    gSpell->resetMovement();
    if (gLander->getAngle() >= -90.0f)
    {
        if (IsKeyDown(KEY_A))
            gLander->setAngle(gLander->getAngle() - 1.0f);
    }
    if (gLander->getAngle() <= +90.0f)
    {
        if (IsKeyDown(KEY_D))
            gLander->setAngle(gLander->getAngle() + 1.0f);
    }
    float angleInRadians = gLander->getAngle() * DEG2RAD;

    if (IsKeyDown(KEY_W) && Fuel > 0.0f && gGameStatus == GAMEPLAYING)
    {
        gLander->moveUp();
        Fuel -= 2.0f / float(FPS);
        if (gLander->getAngle() >= 90.0f || gLander->getAngle() <= -90.0f)
        {
            gLander->setAcceleration({5.0f * sin(angleInRadians), ACCELERATION_OF_GRAVITY});
        }
        else
        {
            gLander->setAcceleration({5.0f * sin(angleInRadians), -10.0f * cos(angleInRadians)});
        }
    }
    else
    {
        gLander->setDirection(LEFT);
        float prevAcceleration = ACCELERATION_OF_GRAVITY - gLander->getAcceleration().y;
        gLander->setAcceleration({gLander->getAcceleration().x, prevAcceleration});
    }
    if ((gSpell->getPosition().y > SCREEN_HEIGHT || gSpell->getPosition().y < 0 ||
         gSpell->getPosition().x > SCREEN_WIDTH || gSpell->getPosition().x < 0) &&
        Fuel > 0.0f && IsKeyDown(KEY_W))
    {
        gSpell->setPosition({gLander->getPosition().x, gLander->getPosition().y + 50.0f});
    }

    if (gSpell->getPosition().x == gLander->getPosition().x && gSpell->getPosition().y == gLander->getPosition().y + 50.0f)
    {
        gSpell->setVelocity({-200.0f * sin(angleInRadians), 200.0f * cos(angleInRadians)});
    }

    if (GetLength(gLander->getMovement()) > 1.0f)
        gLander->normaliseMovement();

    if (IsKeyPressed(KEY_Q) || WindowShouldClose())
        gAppStatus = TERMINATED;
    if ((gGameStatus == GAMEOVER || gGameStatus == GRABWRONG) && IsKeyPressed(KEY_T) && Fuel > 0.0f)
    {
        gGameStatus = GAMEPLAYING;
        gLander->setPosition(STARTINGPOINT);
        gLander->setVelocity({0.0f, 0.0f});
        gLander->setAngle(0.0f);
        gLander->setAcceleration({STARTINGACCELERATION, ACCELERATION_OF_GRAVITY});
    }
}
void update()
{
    // Delta time

    float ticks = (float)GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }
    if (gGameStatus == GAMEPLAYING)
    {
        while (deltaTime >= FIXED_TIMESTEP)
        {
            gLander->update(FIXED_TIMESTEP, gFloatingBook, 1);
            gSpell->update(FIXED_TIMESTEP, nullptr, 0);
            gFloatingBook->update(FIXED_TIMESTEP, nullptr, 0);

            deltaTime -= FIXED_TIMESTEP;
        }
        if ((gLander->getPosition().y - gLander->getScale().y / 2.0f > SCREEN_HEIGHT || gLander->getPosition().y + gLander->getScale().y / 2.0f < 0 ||
             gLander->getPosition().x - gLander->getScale().x / 2.0f > SCREEN_WIDTH || gLander->getPosition().x + gLander->getScale().x / 2.0f < 0))
        {
            gGameStatus = GAMEOVER;
        }
        if (gFloatingBook->getPosition().x + gLander->getScale().x / 2.0f > SCREEN_WIDTH)
        {
            gFloatingBook->setVelocity({-BOOKSPEED, 0.0f});
        }
        else if (gFloatingBook->getPosition().x - gFloatingBook->getScale().x / 2.0f < 0)
        {
            gFloatingBook->setVelocity({BOOKSPEED, 0.0f});
        }
        if (gLander->isCollidingBottom() && (gLander->getAngle() >= -10.0f && gLander->getAngle() <= 10.0f) && fabs(GetLength(gLander->getVelocity())) <= 20.0f)
        {
            gGameStatus = WINSCREEN;
        }
        else if (gLander->isCollidingBottom())
        {
            gGameStatus = GRABWRONG;
        }
    }
}
void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));
    gLibraryBackground->render();
    if (gGameStatus == GAMEPLAYING)
    {
        gLander->render();
        gSpell->render();
        gFloatingBook->render();

        std::string verticalSpeed = "Vertical Speed: " + std::to_string(-1 * int(gLander->getVelocity().y));
        std::string verticalAccel = "Vertical Accel: " + std::to_string(gLander->getAcceleration().y);
        std::string horizontalSpeed = "Horizontal Speed: " + std::to_string(int(gLander->getVelocity().x));
        std::string charAngle = "Angle:" + std::to_string(gLander->getAngle());
        std::string charAltitude = "Altitude:" + std::to_string(int(SCREEN_HEIGHT - gLander->getPosition().y));
        std::string charFuel = "Mana Remaining:" + std::to_string(int(Fuel));
        DrawText(verticalSpeed.c_str(), SCREEN_WIDTH - 240, 180, 20, WHITE);
        DrawText(horizontalSpeed.c_str(), SCREEN_WIDTH - 240, 200, 20, WHITE);
        // DrawText(charAngle.c_str(), SCREEN_WIDTH - 210, 140, 20, WHITE);
        DrawText(charAltitude.c_str(), SCREEN_WIDTH - 240, 220, 20, WHITE);
        DrawText(charFuel.c_str(), 50, 50, 40, PURPLE);
    }
    if ((gGameStatus == GRABWRONG || gGameStatus == GAMEOVER) && Fuel < 0.0f)
    {
        DrawText("GAME OVER (Your Mage has run out of Mana)", SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2 + 40, 40, PURPLE);
    }
    else if (gGameStatus == GRABWRONG)
    {
        DrawText("Your Mage has unsafely grabbed the Star!", SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2, 40, PURPLE);
        DrawText("GAME OVER\n(Try Again? [Click 'T'])", SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2 + 40, 40, PURPLE);
    }
    else if (gGameStatus == GAMEOVER)
    {
        DrawText("Your Mage has exited the Libary!", SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2, 40, PURPLE);
        DrawText("GAME OVER (Try Again? [Click 'T'])", SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2 + 40, 40, PURPLE);
    }
    else if (gGameStatus == WINSCREEN)
    {
        DrawText("Your Mage has safely collected the Star!\n(You Win!)", SCREEN_WIDTH / 2 - 300, SCREEN_HEIGHT / 2, 40, PURPLE);
    }

    EndDrawing();
}
void shutdown()
{
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}