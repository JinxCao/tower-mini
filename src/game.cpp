#include "game.h"
#include <cstdio>
#include <string>
#include <SDL2/SDL_image.h>

static const int WIN_W = 1280;
static const int WIN_H = 720;

static const char* tryFonts[] = {
    "C:/Windows/Fonts/arial.ttf",
    "C:/Windows/Fonts/segoeui.ttf",
    "C:/Windows/Fonts/calibri.ttf"
};

void Game::drawCircleFilled(int cx, int cy, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a){
    SDL_SetRenderDrawColor(ren, r,g,b,a);
    for(int y=-radius; y<=radius; ++y){
        int xx = (int)std::sqrt((double)radius*radius - y*y);
        SDL_RenderDrawLine(ren, cx-xx, cy+y, cx+xx, cy+y);
    }
}

void Game::drawTextCenter(const std::string& s, int y, SDL_Color c){
    if(!font) return;
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, s.c_str(), c);
    if(!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);
    if(!tex) return;
    int w,h; SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
    int winW,winH; SDL_GetRendererOutputSize(ren,&winW,&winH);
    SDL_Rect dst{ (winW - w)/2, y, w, h };
    SDL_RenderCopy(ren, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

static SDL_Texture* loadTex(SDL_Renderer* ren, const char* rel) {
    char* base = SDL_GetBasePath();
    std::string full = base ? (std::string(base) + rel) : std::string(rel);
    if (base) SDL_free(base);

    SDL_Texture* tex = IMG_LoadTexture(ren, full.c_str());
    if (!tex) {
        std::printf("Load '%s' failed: %s\n", full.c_str(), IMG_GetError());
        return nullptr;
    }
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    return tex;
}

bool Game::init(SDL_Renderer* r){
    ren = r;

    // TTF
    if (!TTF_WasInit()) {
        if (TTF_Init() != 0) { std::printf("TTF_Init failed: %s\n", TTF_GetError()); return false; }
    }
    for (auto path : tryFonts){ font = TTF_OpenFont(path, 28); if (font) break; }
    if (!font) std::printf("OpenFont failed: fallback without text.\n");

    // 图片初始化

    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
        std::printf("IMG_Init failed: %s\n", IMG_GetError());
    }

    // 用统一加载函数
    texFast  = loadTex(ren, "assets/fast.png");
    texTank  = loadTex(ren, "assets/tank.png");
    texTower = loadTex(ren, "assets/tower.png");
    texBG    = loadTex(ren, "assets/bg.png");

    return true;
}

void Game::shutdown(){
    if (font) { TTF_CloseFont(font); font=nullptr; }
    if (texFast)  { SDL_DestroyTexture(texFast);  texFast  = nullptr; }
    if (texTank)  { SDL_DestroyTexture(texTank);  texTank  = nullptr; }
    if (texTower) { SDL_DestroyTexture(texTower); texTower = nullptr; }
    if (texBG)    { SDL_DestroyTexture(texBG);    texBG    = nullptr; }

    IMG_Quit();
}

void Game::showBanner(const std::string& s, float sec){
    bannerText = s; bannerTimer = sec;
}

static Vec2 N(float nx, float ny){
    return { nx * (WIN_W-40) + 20, ny * (WIN_H-40) + 20 };
}

void Game::buildLevel(int L){
    waypoints.clear();
    int t = (L-1) % 6;
    int variant = (L-1) / 6;
    float offX = (variant%3)*0.03f - 0.03f;
    float offY = (variant/3)*0.03f - 0.01f;
    bool flip = ((L%2)==0);

    auto pushN = [&](float x, float y){
        if (flip) x = 1.0f - x;
        waypoints.push_back(N(x+offX, y+offY));
    };

    switch (t){
    case 0: { pushN(-0.03f,0.50f); pushN(0.12f,0.50f); pushN(0.32f,0.35f);
              pushN(0.55f,0.35f); pushN(0.72f,0.55f); pushN(1.05f,0.55f); } break;
    case 1: { pushN(-0.03f,0.45f); pushN(0.18f,0.45f); pushN(0.35f,0.32f);
              pushN(0.55f,0.32f); pushN(0.74f,0.45f); pushN(1.05f,0.45f); } break;
    case 2: { pushN(-0.03f,0.60f); pushN(0.18f,0.60f); pushN(0.32f,0.42f);
              pushN(0.50f,0.62f); pushN(0.70f,0.40f); pushN(1.05f,0.40f); } break;
    case 3: { pushN(-0.03f,0.30f); pushN(0.22f,0.55f); pushN(0.45f,0.55f);
              pushN(0.65f,0.35f); pushN(1.05f,0.35f); } break;
    case 4: { pushN(-0.03f,0.50f); pushN(0.40f,0.50f); pushN(0.55f,0.68f);
              pushN(0.78f,0.68f); pushN(1.05f,0.42f); } break;
    case 5: { pushN(-0.03f,0.62f); pushN(0.20f,0.42f); pushN(0.45f,0.42f);
              pushN(0.65f,0.58f); pushN(0.85f,0.58f); pushN(1.05f,0.58f); } break;
    }
}

void Game::startNextLevel(){
    if (level >= maxLevel){ gameWin=true; showBanner("All levels cleared! You win!", 999.f); return; }

    level++;
    levelActive = true;
    spawned = 0;

    buildLevel(level);

    towers.clear();
    if (waypoints.size()>=3){
        Vec2 a = waypoints[1], b = waypoints[2];
        towers.push_back({{ (a.x+b.x)/2, (a.y+b.y)/2 }});
        size_t i = waypoints.size()>4? waypoints.size()-3 : waypoints.size()-2;
        Vec2 c = waypoints[i], d = waypoints[i+1];
        towers.push_back({{ (c.x+d.x)/2, (c.y+d.y)/2 }});
    }

    gold += levelBonus;

    toSpawn    = 6 + level * 2;
    spawnTimer = 0.7f;
    enemies.clear(); bullets.clear();

    showBanner("Level " + std::to_string(level) + " begins!", 2.0f);
}

void Game::startAtLevel(int L){
    gameOver=false; gameWin=false;
    level = L-1;
    startNextLevel();
}

void Game::onMouseDown(int mx, int my){
    if (gameOver || gameWin) return;
    Vec2 p{(float)mx,(float)my};
    for (auto& t : towers) if (std::sqrt(dist2(p,t.pos)) < 40.f) { showBanner("Too close to another tower", 1.2f); return; }
    if (gold >= towerCost){
        gold -= towerCost;
        towers.push_back({p});
        showBanner("Tower placed (-" + std::to_string(towerCost) + ")", 1.2f);
    }else{
        showBanner("Not enough gold", 1.2f);
    }
}

void Game::update(float dt){
    if (gameOver || gameWin){ return; }
    if (bannerTimer>0.f) bannerTimer -= dt;

    if (!levelActive){
        intermission -= dt;
        if (intermission <= 0.f) startNextLevel();
        return;
    }

    if (spawned < toSpawn){
        spawnTimer -= dt;
        if (spawnTimer <= 0.f){
            Enemy e; e.pos = waypoints.front();

            if ((spawned % 4) == 3) {
                e.type  = EnemyType::Tank;
                e.speed = 70.f + level * 2.0f;
                e.hp    = 80.f + level * 12.0f;
            } else {
                e.type  = EnemyType::Fast;
                e.speed = 120.f + level * 4.0f;
                e.hp    = 30.f + level * 6.0f;
            }

            enemies.push_back(e);
            spawned++;
            spawnTimer = std::max(0.25f, 0.8f - level * 0.01f);
        }
    }

    const float reachR = 9.f;
    for (auto& e : enemies){
        if (e.dead) continue;
        if (e.wp + 1 >= (int)waypoints.size()){
            e.dead = true;
            if (--baseHP <= 0){ baseHP = 0; gameOver = true; showBanner("Game Over", 999.f); }
            continue;
        }
        Vec2 a=e.pos, b=waypoints[e.wp+1];
        float dx=b.x-a.x, dy=b.y-a.y;
        float L = std::sqrt(dx*dx+dy*dy);
        if (L < 1.f) { e.wp++; continue; }
        e.pos.x += (dx/L)*e.speed*dt;
        e.pos.y += (dy/L)*e.speed*dt;
        if (dist2(e.pos,b) < reachR*reachR) e.wp++;
    }

    for (auto& t : towers){
        t.cd -= dt;
        if (t.cd <= 0.f){
            int best=-1; float bestD2=1e20f;
            for (int i=0;i<(int)enemies.size();++i){
                if (enemies[i].dead) continue;
                float d2 = dist2(t.pos,enemies[i].pos);
                if (d2 < t.range*t.range && d2 < bestD2){ bestD2=d2; best=i; }
            }
            if (best!=-1){
                Vec2 dir{ enemies[best].pos.x - t.pos.x, enemies[best].pos.y - t.pos.y };
                float L = std::sqrt(dir.x*dir.x + dir.y*dir.y); if(L>0){ dir.x/=L; dir.y/=L; }
                bullets.push_back({ t.pos, {dir.x*320.f, dir.y*320.f}, 18.f, false });
                t.cd = t.fireInterval;
            }
        }
    }

    for (auto& b : bullets){
        if (b.dead) continue;
        b.pos.x += b.vel.x*dt; b.pos.y += b.vel.y*dt;
        if (b.pos.x<-50||b.pos.x>WIN_W+50||b.pos.y<-50||b.pos.y>WIN_H+50){ b.dead=true; continue; }
        for (auto& e : enemies){
            if (e.dead) continue;
            if (dist2(b.pos,e.pos) <= 16.f*16.f){
                e.hp -= b.damage; b.dead=true;
                if (e.hp<=0){ e.dead=true; gold += killGold; }
                break;
            }
        }
    }

    enemies.erase(std::remove_if(enemies.begin(),enemies.end(),[](auto& e){return e.dead;}), enemies.end());
    bullets.erase(std::remove_if(bullets.begin(),bullets.end(),[](auto& b){return b.dead;}), bullets.end());

    if (spawned==toSpawn && enemies.empty()){
        levelActive=false;
        if (level >= maxLevel){
            gameWin=true;
            showBanner("All levels cleared! You win!", 999.f);
        }else{
            intermission = 3.0f;
            showBanner("Level " + std::to_string(level) + " cleared!", 2.0f);
        }
    }
}

void Game::render(){
    auto drawTexCenter = [&](SDL_Texture* tex, int cx, int cy, int w, int h){
        if (!tex) return;
        SDL_Rect dst{ cx - w/2, cy - h/2, w, h };
        SDL_RenderCopy(ren, tex, nullptr, &dst);
    };

    // 背景BG
    if (texBG) {
        int winW, winH; SDL_GetRendererOutputSize(ren, &winW, &winH);
        int texW, texH; SDL_QueryTexture(texBG, nullptr, nullptr, &texW, &texH);
        float scale = std::max(winW / (float)texW, winH / (float)texH);
        int dstW = (int)(texW * scale);
        int dstH = (int)(texH * scale);
        SDL_Rect dst{ (winW - dstW)/2, (winH - dstH)/2, dstW, dstH };
        SDL_RenderCopy(ren, texBG, nullptr, &dst);
    } else {
        SDL_SetRenderDrawColor(ren, 28,43,58,255);
        SDL_RenderClear(ren);
    }

    // 路径
    SDL_SetRenderDrawColor(ren, 200, 220, 255, 235);
    for (size_t i=1; i<waypoints.size(); ++i){
        SDL_RenderDrawLine(ren,
            (int)waypoints[i-1].x,(int)waypoints[i-1].y,
            (int)waypoints[i].x,  (int)waypoints[i].y
        );
    }

    // 敌人
    for (auto& e : enemies) {
        if (e.type == EnemyType::Fast) {
            if (texFast) drawTexCenter(texFast, (int)e.pos.x, (int)e.pos.y, 32, 32);
            else         drawCircleFilled((int)e.pos.x,(int)e.pos.y,10,240,90,90,255);
        } else {
            if (texTank) drawTexCenter(texTank, (int)e.pos.x, (int)e.pos.y, 40, 40);
            else         drawCircleFilled((int)e.pos.x,(int)e.pos.y,12,255,156,60,255);
        }
    }

    // 塔 + 射程
    for (auto& t : towers){
        if (texTower) drawTexCenter(texTower, (int)t.pos.x, (int)t.pos.y, 48, 48);
        else          drawCircleFilled((int)t.pos.x,(int)t.pos.y,14,90,180,255,255);

        SDL_SetRenderDrawColor(ren, 60,100,140,70);
        for(int a=0; a<360; a+=4){
            float rad = a*3.1415926f/180.f;
            int x = (int)(t.pos.x + std::cos(rad)*t.range);
            int y = (int)(t.pos.y + std::sin(rad)*t.range);
            SDL_RenderDrawPoint(ren, x, y);
        }
    }

    // 子弹
    // SDL_SetRenderDrawColor(ren, 255,255,255,255);
    // for (auto& b : bullets) SDL_RenderDrawPoint(ren,(int)b.pos.x,(int)b.pos.y);
    // 子弹：亮黄 4x4 方块
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ren, 255, 230, 80, 255);

        for (auto& b : bullets) {
            SDL_Rect r{ (int)b.pos.x - 2, (int)b.pos.y - 2, 4, 4 }; // 4x4
            SDL_RenderFillRect(ren, &r);
        }


    // HUD
    if (font){
        char buf[128];
        if (levelActive){
            std::snprintf(buf,sizeof(buf),"HP: %d   Level: %d/%d   Gold: %d",
                          baseHP, level, maxLevel, gold);
        }else if (!gameWin){
            std::snprintf(buf,sizeof(buf),"HP: %d   Level: %d/%d (rest)   Gold: %d   Next level in %.1fs",
                          baseHP, level, maxLevel, gold, intermission);
        }else{
            std::snprintf(buf,sizeof(buf),"HP: %d   Level: %d/%d   Gold: %d",
                          baseHP, level, maxLevel, gold);
        }
        drawTextCenter(buf, 10, SDL_Color{230,230,230,255});
    }

    // 公告
    if (bannerTimer>0.f && font){
        drawTextCenter(bannerText, 320, SDL_Color{255,230,100,255});
    }
}
