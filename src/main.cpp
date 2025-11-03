#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstdio>
#include <string>
#include "game.h"

enum class AppState { Menu, Playing };

static void drawText(SDL_Renderer* r, TTF_Font* f, const std::string& s, int x, int y, SDL_Color c){
    SDL_Surface* surf = TTF_RenderUTF8_Blended(f, s.c_str(), c);
    if(!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    SDL_FreeSurface(surf);
    if(!tex) return;
    int w,h; SDL_QueryTexture(tex,nullptr,nullptr,&w,&h);
    SDL_Rect dst{ x, y, w, h };
    SDL_RenderCopy(r, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Tower Mini (SDL2)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_SHOWN
    );
    if (!window) { std::printf("SDL_CreateWindow failed: %s\n", SDL_GetError()); SDL_Quit(); return 1; }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) { std::printf("SDL_CreateRenderer failed: %s\n", SDL_GetError()); SDL_DestroyWindow(window); SDL_Quit(); return 1; }

    // 菜单用字体
    if (!TTF_WasInit() && TTF_Init()!=0) { std::printf("TTF_Init failed: %s\n", TTF_GetError()); }
    TTF_Font* menuFont = nullptr;
    const char* menuFontTry[] = {"C:/Windows/Fonts/arial.ttf","C:/Windows/Fonts/segoeui.ttf","C:/Windows/Fonts/calibri.ttf"};
    for (auto p: menuFontTry){ menuFont = TTF_OpenFont(p, 32); if(menuFont) break; }

    AppState state = AppState::Menu;
    Game g;

    bool running = true;
    Uint64 last = SDL_GetPerformanceCounter();
    const double freq = (double)SDL_GetPerformanceFrequency();

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            if (state == AppState::Menu) {
                if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                    // 关卡按钮网格：6列 x 5行，按钮大小 160x80，间距 20
                    int mx=e.button.x, my=e.button.y;
                    int left=80, top=120, bw=160, bh=80, gap=20;
                    for (int r=0;r<5;r++){
                        for (int c=0;c<6;c++){
                            int idx = r*6+c; // 0..29
                            SDL_Rect rc{ left + c*(bw+gap), top + r*(bh+gap), bw, bh };
                            if (mx>=rc.x && mx<=rc.x+rc.w && my>=rc.y && my<=rc.y+rc.h){
                                int L = idx+1;
                                // 进入游戏并跳到第 L 关
                                if (!g.init(renderer)) { std::printf("Game init failed\n"); break; }
                                g.startAtLevel(L);
                                state = AppState::Playing;
                            }
                        }
                    }
                }
            } else { // Playing
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_BACKSPACE) {
                    // 返回菜单
                    g.shutdown();
                    state = AppState::Menu;
                }
                if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                    g.onMouseDown(e.button.x, e.button.y);
                }
            }
        }

        Uint64 now = SDL_GetPerformanceCounter();
        float dt = (float)((now - last) / freq); last = now;

        SDL_SetRenderDrawColor(renderer, 32, 46, 68, 255);
        SDL_RenderClear(renderer);

        if (state == AppState::Menu) {
            // 标题
            if (menuFont) drawText(renderer, menuFont, "Select Level (1-30)", 470, 40, SDL_Color{240,240,240,255});
            // 按钮网格
            int left=80, top=120, bw=160, bh=80, gap=20;
            for (int r=0;r<5;r++){
                for (int c=0;c<6;c++){
                    int idx = r*6+c;
                    SDL_Rect rc{ left + c*(bw+gap), top + r*(bh+gap), bw, bh };
                    SDL_SetRenderDrawColor(renderer, 70,90,120,255);
                    SDL_RenderFillRect(renderer,&rc);
                    SDL_SetRenderDrawColor(renderer, 130,160,200,255);
                    SDL_RenderDrawRect(renderer,&rc);
                    if (menuFont){
                        char buf[8]; std::snprintf(buf,sizeof(buf),"%d", idx+1);
                        // 居中一点（简单处理）
                        int tx = rc.x + bw/2 - 10;
                        int ty = rc.y + bh/2 - 16;
                        drawText(renderer, menuFont, buf, tx, ty, SDL_Color{255,255,255,255});
                    }
                }
            }
            if (menuFont) drawText(renderer, menuFont, "Tip: BACKSPACE to return menu during game",
                                   320, 650, SDL_Color{200,200,200,255});
        } else {
            g.update(dt);
            g.render();
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(1);
    }

    if (menuFont) TTF_CloseFont(menuFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
