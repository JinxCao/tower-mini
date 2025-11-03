#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <SDL2/SDL_image.h>

struct Vec2 { float x=0, y=0; };
inline float dist2(const Vec2& a, const Vec2& b){ float dx=a.x-b.x, dy=a.y-b.y; return dx*dx+dy*dy; }

// 两种敌人
enum class EnemyType { Fast, Tank };

struct Enemy {
    Vec2 pos; int wp=0; float speed=80.f; float hp=30.f; bool dead=false;
    EnemyType type = EnemyType::Fast;
};

struct Tower {
    Vec2 pos; float range=180.f; float cd=0.f; float fireInterval=0.6f;
};

struct Bullet {
    Vec2 pos, vel; float damage=15.f; bool dead=false;
};

struct Game {
    SDL_Renderer* ren=nullptr;

    // 资源
    TTF_Font* font=nullptr;
    SDL_Texture* texFast = nullptr;
    SDL_Texture* texTank = nullptr;
    SDL_Texture* texTower = nullptr;
    SDL_Texture* texBG = nullptr;

    // 玩法数据
    std::vector<Vec2> waypoints;
    std::vector<Enemy> enemies;
    std::vector<Tower> towers;
    std::vector<Bullet> bullets;

    // 关卡信息
    int level=0;                // 当前关
    const int maxLevel=30;
    bool levelActive=false;
    int toSpawn=0, spawned=0;
    float spawnTimer=0.f;
    float intermission=0.f;     // 中场休息（秒）

    // 基地
    int baseHP=10;
    bool gameOver=false;
    bool gameWin=false;

    // 经济
    int gold=100;          // 初始金币
    int towerCost=40;      // 放塔花费
    int killGold=5;        // 击杀奖励
    int levelBonus=20;     // 每关开始奖励

    // 公告
    std::string bannerText;
    float bannerTimer=0.f; // 秒

    // 接口
    bool init(SDL_Renderer* r);
    void shutdown();
    void update(float dt);
    void render();
    void onMouseDown(int mx, int my); // 放置新塔

    // 关卡
    void startAtLevel(int L); // 直接开始到第 L 关（菜单用）
    void startNextLevel();
    void buildLevel(int L);
    void showBanner(const std::string& s, float sec=2.0f);

    // 绘制
    void drawCircleFilled(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a=255);
    void drawTextCenter(const std::string& s, int y, SDL_Color c);
};
