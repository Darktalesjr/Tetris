#pragma warning (disable : 4244 267)

#include <iostream>
#include <chrono>

#include "SDL.h"
#include "SDL_ttf.h"

using namespace std;

const uint32_t ROWS = 20;
const uint32_t COLS = 10;
const int ZOOM = 40;

const int TICKTIME = 666;

const int WIDTH = ZOOM * (COLS + 14), HEIGHT = ZOOM * (ROWS + 3);

const bool I_TETRAMINO[4][4]{ {0,0,0,0},
                              {1,1,1,1},
                              {0,0,0,0},
                              {0,0,0,0} };

const bool L_TETRAMINO[3][3]{ {1,0,0},
                              {1,1,1},
                              {0,0,0} };

const bool RL_TETRAMINO[3][3]{ {0,0,1},
                               {1,1,1},
                               {0,0,0} };

const bool SQ_TETRAMINO[2][2]{ {1,1},
                               {1,1} };

const bool S_TETRAMINO[3][3]{ {0,1,1},
                              {1,1,0},
                              {0,0,0} };

const bool T_TETRAMINO[3][3]{ {0,1,0},
                              {1,1,1},
                              {0,0,0} };

const bool RS_TETRAMINO[3][3]{ {1,1,0},
                               {0,1,1},
                               {0,0,0} };

const bool* PIECES[7]{ (const bool*)&I_TETRAMINO,(const bool*)&L_TETRAMINO, (const bool*)&RL_TETRAMINO, (const bool*)&SQ_TETRAMINO, (const bool*)&S_TETRAMINO, (const bool*)&T_TETRAMINO, (const bool*)&RS_TETRAMINO };

const Uint8 COLORS[7][3]{ {  0,255,255},
                          {  0,  0,255},
                          {255,128,  0},
                          {255,255,   },
                          {  0,255,  0},
                          {255,  0,255},
                          {255,  0,  0}, };

const int WALL_KICK[4][5][2]{ { {0,0}, { 0,0}, { 0, 0}, {0,0}, { 0,0} },
                              { {0,0}, { 1,0}, { 1,-1}, {0,2}, { 1,2} },
                              { {0,0}, { 0,0}, { 0, 0}, {0,0}, { 0,0} },
                              { {0,0}, {-1,0}, {-1,-1}, {0,2}, {-1,2} } };

const int WALL_KICK_I[4][5][2]{ { {0,0}, {0,0}, { 0,0}, { 0,  0}, { 0, 0} },
                                { {0,0}, {1,0}, {-2,0}, { 1, -2}, {-2, 1} },
                                { {0,0}, {3,0}, {-3,0}, { 3, -1}, {-3, 1} },
                                { {0,0}, {2,0}, {-1,0}, { 2,  1}, {-1,-2} } };

TTF_Font* font;

static inline void genPool(int pool[7][2])
{
    int gen[7][2]{ {1,4},{2,3},{3,3},{4,2},{5,3},{6,3},{7,3} };
    int n;
    for (int ct = 7; ct > 0; ct--)
    {
        n = rand() % ct;
        pool[ct - 1][0] = gen[n][0];
        pool[ct - 1][1] = gen[n][1];
        for (int ct1 = n; ct1 < ct - 1; ct1++)gen[ct1][0] = gen[ct1 + 1][0], gen[ct1][1] = gen[ct1 + 1][1];
    }
}

static inline void checkLinePoint(int board[ROWS][COLS])
{
    bool fill = 0;
    int ct1 = 0, ct;
    int boardBuffer[ROWS - 1][COLS];
    for (ct = 0; ct < ROWS; ct++) {
        for (ct1 = 0; ct1 < COLS; ct1++) if (board[ct][ct1] == 0) break;
        if (ct1 == 10)
        {
            memcpy(boardBuffer, board, ct * COLS * sizeof(int));
            memcpy(board[1], boardBuffer, ct * COLS * sizeof(int));
        }
    }
}

static inline bool isStopped(int board[ROWS][COLS], bool moveBoard[4][4], int px, int py, int current[2])
{
    for (int ct = 0; ct < current[1]; ct++) for (int ct1 = 0; ct1 < current[1]; ct1++)if (moveBoard[ct][ct1] && ((board[ct1 + py + 1][ct + px] != 0) || ct1 + py + 1 == ROWS)) return true;
    return false;
}

static inline bool isStoppedL(int board[ROWS][COLS], bool moveBoard[4][4], int px, int py, int current[2])
{
    for (int ct = 0; ct < current[1]; ct++) for (int ct1 = 0; ct1 < current[1]; ct1++)if (moveBoard[ct][ct1] && ((board[ct1 + py][ct + px - 1] != 0) || ct + px == 0)) return true;
    return false;
}

static inline bool isStoppedR(int board[ROWS][COLS], bool moveBoard[4][4], int px, int py, int current[2])
{
    for (int ct = 0; ct < current[1]; ct++) for (int ct1 = 0; ct1 < current[1]; ct1++) if (moveBoard[ct][ct1] && ((board[ct1 + py][ct + px + 1] != 0) || ct + px + 1 == COLS)) return true;
    return false;
}

static inline void rotateR(bool moveBoard[4][4], int current[2])
{
    bool buffer;
    for (int ct = 0; ct < current[1] / 2; ct++)for (int ct1 = ct; ct1 < current[1] - ct - 1; ct1++) buffer = moveBoard[ct][ct1], moveBoard[ct][ct1] = moveBoard[ct1][current[1] - 1 - ct], moveBoard[ct1][current[1] - 1 - ct] = moveBoard[current[1] - 1 - ct][current[1] - 1 - ct1], moveBoard[current[1] - 1 - ct][current[1] - 1 - ct1] = moveBoard[current[1] - 1 - ct1][ct], moveBoard[current[1] - 1 - ct1][ct] = buffer;
}

static inline void rotateL(bool moveBoard[4][4], int current[2])
{
    bool buffer;
    for (int ct = 0; ct < current[1] / 2; ct++)for (int ct1 = ct; ct1 < current[1] - ct - 1; ct1++) buffer = moveBoard[ct][ct1], moveBoard[ct][ct1] = moveBoard[current[1] - 1 - ct1][ct], moveBoard[current[1] - 1 - ct1][ct] = moveBoard[current[1] - 1 - ct][current[1] - 1 - ct1], moveBoard[current[1] - 1 - ct][current[1] - 1 - ct1] = moveBoard[ct1][current[1] - 1 - ct], moveBoard[ct1][current[1] - 1 - ct] = buffer;
}

static inline void placeT(int board[ROWS][COLS], bool moveBoard[4][4], int px, int py, int current[2])
{
    for (int ct = 0; ct < current[1]; ct++) for (int ct1 = 0; ct1 < current[1]; ct1++) if (moveBoard[ct][ct1])board[ct1 + py][ct + px] = current[0];
}

static inline void replaceT(bool moveBoard[4][4], int current[2], int& px, int& py)
{
    px = (current[0] == 4) ? 4 : 3, py = 0;
    for (int ct = 0; ct < 4; ct++) for (int ct1 = 0; ct1 < 4; ct1++) moveBoard[ct][ct1] = 0;
    for (int ct = 0; ct < current[1]; ct++) for (int ct1 = 0; ct1 < current[1]; ct1++) moveBoard[ct][ct1] = (*(&PIECES[current[0] - 1][ct] + ct1 * current[1]));
}

static inline bool relocateT(int board[ROWS][COLS], bool moveBoard[4][4], int current[2], int& px, int& py, int st, int v, SDL_Renderer* renderer)
{
    bool isStopped = 0;
    int st2 = (v == 1) ? (st == 3) ? 0 : st + 1 : (st == 0) ? 3 : st - 1;
    int offX, offY;
    SDL_SetRenderDrawColor(renderer, COLORS[current[0] - 1][0] / 2, COLORS[current[0] - 1][1] / 2, COLORS[current[0] - 1][2] / 2, 255);
    SDL_Rect rrect{ ZOOM / 2,ZOOM / 2,ZOOM / 2,ZOOM / 2 };
    if (current[0] == 1)
    {
        for (int ct0 = 0; ct0 < 5; ct0++) {
            offY = WALL_KICK_I[st][ct0][1] - WALL_KICK_I[st2][ct0][1], offX = WALL_KICK_I[st][ct0][0] - WALL_KICK_I[st2][ct0][0];
            for (int ct = 0; ct < current[1]; ct++) for (int ct1 = 0; ct1 < current[1]; ct1++)
                isStopped = isStopped
                || (moveBoard[ct][ct1] && (board[ct1 + py - offY][ct + px + offX] != 0))
                || (ct1 + py - offY > ROWS) || (ct1 + py - offY < 0) || (ct + px + offX >= COLS) || (ct + px + offX < 0);
            if (!isStopped) {
                py -= offY, px += offX;
                st = st2;
                return true;
            }
            isStopped = false;
        }
    }
    else
    {

        for (int ct0 = 0; ct0 < 5; ct0++) {
            offY = WALL_KICK[st][ct0][1] - WALL_KICK[st2][ct0][1], offX = WALL_KICK[st][ct0][0] - WALL_KICK[st2][ct0][0];
            for (int ct = 0; ct < current[1]; ct++) for (int ct1 = 0; ct1 < current[1]; ct1++)
                isStopped = isStopped
                || (moveBoard[ct][ct1] && (board[ct1 + py - offY][ct + px + offX] != 0))
                || (ct1 + py - offY > ROWS) || (ct1 + py - offY < 0) || (ct + px + offX >= COLS) || (ct + px + offX < 0);
            if (!isStopped) {
                py -= offY, px += offX;
                st = st2;
                return true;
            }
            isStopped = false;
        }
    }
    return false;
}

static inline void drawField(SDL_Renderer* renderer)
{
    SDL_Rect rRect{ ZOOM, ZOOM, ZOOM * 6, ZOOM }; // HUN.ttf
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(renderer, ZOOM * 7, ZOOM, ZOOM * 7, ZOOM * (ROWS + 2));
    SDL_RenderDrawLine(renderer, ZOOM * 7, ZOOM * (ROWS + 2), ZOOM * (COLS + 7), ZOOM * (ROWS + 2));
    SDL_RenderDrawLine(renderer, ZOOM * COLS + ZOOM * 7, ZOOM, ZOOM * COLS + ZOOM * 7, ZOOM * ROWS + ZOOM * 2);

    SDL_RenderDrawLine(renderer, ZOOM, ZOOM, ZOOM, ZOOM * 6);
    SDL_RenderDrawLine(renderer, ZOOM * 7, ZOOM * 6, ZOOM, ZOOM * 6);
    SDL_RenderFillRect(renderer, &rRect);

    rRect.x = ZOOM * COLS + ZOOM * 7;
    SDL_RenderDrawLine(renderer, ZOOM * COLS + ZOOM * 7, ZOOM, ZOOM * COLS + ZOOM * 13, ZOOM);
    SDL_RenderDrawLine(renderer, ZOOM * COLS + ZOOM * 13, ZOOM, ZOOM * COLS + ZOOM * 13, ZOOM * 18);
    SDL_RenderDrawLine(renderer, ZOOM * COLS + ZOOM * 7, ZOOM * 18, ZOOM * COLS + ZOOM * 13, ZOOM * 18);
    SDL_RenderFillRect(renderer, &rRect);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    rRect.x = ZOOM + 4, rRect.y = ZOOM + 4;
    SDL_Surface* surfaceMessage;
    SDL_Texture* text;

    TTF_SizeText(font, "HOLD", &rRect.w, &rRect.h);
    surfaceMessage = TTF_RenderText_Solid(font, "HOLD", { 0,0,0,255 });
    text = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    SDL_RenderCopy(renderer, text, NULL, &rRect);
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(text);

    rRect.x += ZOOM * 16;
    TTF_SizeText(font, "NEXT", &rRect.w, &rRect.h);
    surfaceMessage = TTF_RenderText_Solid(font, "NEXT", { 0,0,0,255 });
    text = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    SDL_RenderCopy(renderer, text, NULL, &rRect);
    SDL_FreeSurface(surfaceMessage);
    SDL_DestroyTexture(text);
}

static inline void renderDrawGame(SDL_Renderer* renderer, int board[ROWS][COLS], bool moveBoard[4][4], int pool[7][2], int pool2[7][2], int poolct, int hold[2], int px, int py)
{
    SDL_Rect rrect{ ZOOM * 7 + 1,ZOOM * 2, ZOOM * COLS - 1, ZOOM * ROWS - 1 };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rrect);
    rrect.h = ZOOM - 2, rrect.w = ZOOM - 2;

    for (int ct = 0; ct < ROWS; ct++) for (int ct1 = 0; ct1 < COLS; ct1++) if (board[ct][ct1] != 0)
    {
        SDL_SetRenderDrawColor(renderer, COLORS[board[ct][ct1] - 1][0], COLORS[board[ct][ct1] - 1][1], COLORS[board[ct][ct1] - 1][2], 255);
        rrect.x = (ct1 + 7) * ZOOM + 1, rrect.y = (ct + 2) * ZOOM + 1;
        SDL_RenderFillRect(renderer, &rrect);
    }

    int ppy = py;
    while (!isStopped(board, moveBoard, px, ppy, pool[poolct]))ppy++;
    SDL_SetRenderDrawColor(renderer, COLORS[pool[poolct][0] - 1][0] / 2, COLORS[pool[poolct][0] - 1][1] / 2, COLORS[pool[poolct][0] - 1][2] / 2, 255);
    for (int ct = 0; ct < 4; ct++) for (int ct1 = 0; ct1 < 4; ct1++) if (moveBoard[ct][ct1])
    {
        rrect.x = (ct + px + 7) * ZOOM + 1, rrect.y = (ct1 + ppy + 2) * ZOOM + 1;
        SDL_RenderFillRect(renderer, &rrect);
    }

    SDL_SetRenderDrawColor(renderer, COLORS[pool[poolct][0] - 1][0], COLORS[pool[poolct][0] - 1][1], COLORS[pool[poolct][0] - 1][2], 255);
    for (int ct = 0; ct < 4; ct++) for (int ct1 = 0; ct1 < 4; ct1++) if (moveBoard[ct][ct1])
    {
        rrect.x = (ct + px + 7) * ZOOM + 1, rrect.y = (ct1 + py + 2) * ZOOM + 1;
        SDL_RenderFillRect(renderer, &rrect);
    }

    if (hold[1] != 0)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        rrect.x = ZOOM + 1, rrect.y = ZOOM * 2 + 1, rrect.w = ZOOM * 5 - 2, rrect.h = ZOOM * 4 - 2;
        SDL_RenderFillRect(renderer, &rrect);
        SDL_SetRenderDrawColor(renderer, COLORS[hold[0] - 1][0], COLORS[hold[0] - 1][1], COLORS[hold[0] - 1][2], 255);

        rrect.h = ZOOM - 2, rrect.w = ZOOM - 2;

        switch (hold[1]) {
        case 2:
            rrect.x = ZOOM * 3, rrect.y = ZOOM * 3;
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 4, rrect.y = ZOOM * 3;
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 3, rrect.y = ZOOM * 4;
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 4, rrect.y = ZOOM * 4;
            SDL_RenderFillRect(renderer, &rrect);
            break;
        case 3:
            for (int ct = 0; ct < 3; ct++)for (int ct1 = 0; ct1 < 2; ct1++) if (PIECES[hold[0] - 1][ct + ct1 * hold[1]])
            {
                rrect.x = ZOOM * (2.5 + ct), rrect.y = ZOOM * (3 + ct1);
                SDL_RenderFillRect(renderer, &rrect);
            }
            break;
        case 4:
            rrect.x = ZOOM * 2, rrect.y = ZOOM * 3.5;
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 3, rrect.y = ZOOM * 3.5;
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 4, rrect.y = ZOOM * 3.5;
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 5, rrect.y = ZOOM * 3.5;
            SDL_RenderFillRect(renderer, &rrect);
            break;
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); rrect.x = ZOOM * 17 + 1, rrect.y = ZOOM * 2 + 1, rrect.w = ZOOM * 6 - 2, rrect.h = ZOOM * 15 - 2;
    SDL_RenderFillRect(renderer, &rrect);

    rrect.h = ZOOM - 2, rrect.w = ZOOM - 2;
    for (int ct = poolct - 1; ct >= max(0, poolct - 5); ct--)
    {
        SDL_SetRenderDrawColor(renderer, COLORS[pool[ct][0] - 1][0], COLORS[pool[ct][0] - 1][1], COLORS[pool[ct][0] - 1][2], 255);
        switch (pool[ct][1]) {
        case 2:
            rrect.x = ZOOM * 19, rrect.y = ZOOM * (3 + (poolct - 1 - ct) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 20, rrect.y = ZOOM * (3 + (poolct - 1 - ct) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 19, rrect.y = ZOOM * (4 + (poolct - 1 - ct) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 20, rrect.y = ZOOM * (4 + (poolct - 1 - ct) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            break;
        case 3:
            for (int ct2 = 0; ct2 < 3; ct2++)for (int ct1 = 0; ct1 < 2; ct1++) if (PIECES[pool[ct][0] - 1][ct2 + ct1 * pool[ct][1]])
            {
                rrect.x = ZOOM * (18.5 + ct2), rrect.y = ZOOM * (3 + (poolct - 1 - ct) * 3 + ct1);
                SDL_RenderFillRect(renderer, &rrect);
            }
            break;
        case 4:
            rrect.x = ZOOM * 18, rrect.y = ZOOM * (4 + (poolct - 1 - ct) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 19, rrect.y = ZOOM * (4 + (poolct - 1 - ct) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 20, rrect.y = ZOOM * (4 + (poolct - 1 - ct) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 21, rrect.y = ZOOM * (4 + (poolct - 1 - ct) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            break;
        }
    }
    for (int ct = 6; ct > (poolct + 1 - max(0, poolct - 5)); ct--)
    {
        SDL_SetRenderDrawColor(renderer, COLORS[pool2[ct][0] - 1][0], COLORS[pool2[ct][0] - 1][1], COLORS[pool2[ct][0] - 1][2], 255);
        switch (pool2[ct][1]) {
        case 2:
            rrect.x = ZOOM * 19, rrect.y = ZOOM * (3 + (5 - ct + (poolct + 1 - max(0, poolct - 5))) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 20, rrect.y = ZOOM * (3 + (5 - ct + (poolct + 1 - max(0, poolct - 5))) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 19, rrect.y = ZOOM * (4 + (5 - ct + (poolct + 1 - max(0, poolct - 5))) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 20, rrect.y = ZOOM * (4 + (5 - ct + (poolct + 1 - max(0, poolct - 5))) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            break;
        case 3:
            for (int ct2 = 0; ct2 < 3; ct2++) for (int ct1 = 0; ct1 < 2; ct1++) if (PIECES[pool2[ct][0] - 1][ct2 + ct1 * pool2[ct][1]])
            {
                rrect.x = ZOOM * (18.5 + ct2), rrect.y = ZOOM * (3 + (5 - ct + (poolct + 1 - max(0, poolct - 5))) * 3 + ct1);
                SDL_RenderFillRect(renderer, &rrect);
            }
            break;
        case 4:
            rrect.x = ZOOM * 18, rrect.y = ZOOM * (4 + (5 - ct + (poolct + 1 - max(0, poolct - 5))) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 19, rrect.y = ZOOM * (4 + (5 - ct + (poolct + 1 - max(0, poolct - 5))) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 20, rrect.y = ZOOM * (4 + (5 - ct + (poolct + 1 - max(0, poolct - 5))) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            rrect.x = ZOOM * 21, rrect.y = ZOOM * (4 + (5 - ct + (poolct + 1 - max(0, poolct - 5))) * 3);
            SDL_RenderFillRect(renderer, &rrect);
            break;
        }
    }
}

int main(int argc, char* argv[])
{
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Event event;
    font = TTF_OpenFont("resources/font/hun2.ttf", 40);

    bool running = true;

    srand(time(NULL));
    int pool[7][2]{ 0 };
    int pool2[7][2]{ 0 };
    int poolct = 6;
    genPool(pool);
    genPool(pool2);
    int board[ROWS][COLS]{ 0 };
    bool moveBoard[4][4]{ 0 };
    int px = 0, py = 0;
    int st = 0;
    int hold[2] = { 0 };
    int tmp;
    bool holdn = 1;
    replaceT(moveBoard, pool[poolct], px, py);
    time_t timer = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();

    drawField(renderer);

    while (running) {

        renderDrawGame(renderer, board, moveBoard, pool, pool2, poolct, hold, px, py);
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode)
                {
                case SDL_SCANCODE_UP:
                case SDL_SCANCODE_Z:
                    if (pool[poolct][0] != 4)
                    {
                        rotateR(moveBoard, pool[poolct]);
                        if (!relocateT(board, moveBoard, pool[poolct], px, py, st, 1, renderer))rotateL(moveBoard, pool[poolct]);
                        else st = (st == 3) ? 0 : st + 1;
                    }
                    break;
                case SDL_SCANCODE_X:
                    if (pool[poolct][0] != 4)
                    {
                        rotateL(moveBoard, pool[poolct]);
                        if (!relocateT(board, moveBoard, pool[poolct], px, py, st, -1, renderer))rotateR(moveBoard, pool[poolct]);
                        else st = (st == 0) ? 3 : st - 1;
                    }
                    break;
                case SDL_SCANCODE_C:
                    if (holdn)
                    {
                        if (hold[0] == 0)
                        {
                            hold[0] = pool[poolct][0], hold[1] = pool[poolct][1];
                            if (poolct == 0)
                            {
                                poolct = 6; genPool(pool);
                            }
                            else poolct--;
                        }
                        else
                        {
                            tmp = hold[0]; hold[0] = pool[poolct][0], pool[poolct][0] = tmp;
                            tmp = hold[1]; hold[1] = pool[poolct][1], pool[poolct][1] = tmp;
                        }
                        st = 0;
                        replaceT(moveBoard, pool[poolct], px, py);
                        holdn = 0;
                    }
                    break;
                case SDL_SCANCODE_DOWN:
                    if (!isStopped(board, moveBoard, px, py, pool[poolct]))
                    {
                        py++;
                        timer = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
                    }
                    break;
                case SDL_SCANCODE_LEFT:
                    if (!isStoppedL(board, moveBoard, px, py, pool[poolct]))px--;
                    break;
                case SDL_SCANCODE_RIGHT:
                    if (!isStoppedR(board, moveBoard, px, py, pool[poolct])) px++;
                    break;
                case SDL_SCANCODE_SPACE:
                    while (!isStopped(board, moveBoard, px, py, pool[poolct]))py++;
                    timer = TICKTIME;
                    break;
                }
            }
        }


        if ((chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count() - timer) / TICKTIME > 0l)
        {
            timer = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
            if (isStopped(board, moveBoard, px, py, pool[poolct]))
            {
                holdn = 1;
                placeT(board, moveBoard, px, py, pool[poolct]);
                checkLinePoint(board);
                if (poolct == 0)
                {
                    poolct = 6;
                    memcpy(pool, pool2, 7 * sizeof(int) * 2);
                    genPool(pool2);
                }
                else poolct--;
                replaceT(moveBoard, pool[poolct], px, py);
                st = 0;
            }
            else py++;
        }

        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}