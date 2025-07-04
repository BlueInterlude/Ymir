﻿#include <ymir/util/scope_guard.hpp>

#include "../../../libs/ymir-core/src/ymir/hw/vdp/slope.hpp"

#include <ymir/sys/backup_ram.hpp>

#include <ymir/util/size_ops.hpp>

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <fmt/format.h>

#include <chrono>
#include <filesystem>
#include <vector>

using namespace util;

struct Sandbox {
    Sandbox(uint32 width, uint32 height)
        : framebuffer(width * height)
        , width(width)
        , height(height)
        // A = 32x38  B = 225x52  C = 431x254  D = 59x273
        // A = 260x272  B = 135x195  C = 240x129  D = 346x192
        // A = 181x241  B = 373x29  C = 95x37  D = 52x103
        , ax(181)
        , ay(241)
        , bx(373)
        , by(29)
        , cx(95)
        , cy(37)
        , dx(52)
        , dy(103)
        , lastTicks(SDL_GetTicks()) {
        keys.fill(false);
        prevKeys = keys;
    }

    void KeyDown(const SDL_Event &evt) {
        keys[evt.key.scancode] = true;
    }

    void KeyUp(const SDL_Event &evt) {
        keys[evt.key.scancode] = false;
    }

    void Frame() {
        using namespace ymir::vdp;

        const double dt = DeltaTime();
        const double speed = 100.0;

        const double keyRepeatInterval = 1.0 / 25.0;

        for (int i = 0; i < SDL_SCANCODE_COUNT; i++) {
            keyRepeat[i] = false;
            if (keys[i]) {
                keyDownLen[i] += dt;
                if (keyDownLen[i] >= keyRepeatInterval) {
                    keyRepeat[i] = true;
                    keyDownLen[i] -= keyRepeatInterval;
                }
            } else {
                keyDownLen[i] = 0.0;
            }
        }

        double factor = 1.0;
        if (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT]) {
            factor = 5.0;
        }

        const double inc = dt * speed * factor;

        if (keys[SDL_SCANCODE_Z] && !prevKeys[SDL_SCANCODE_Z]) {
            antialias = !antialias;
        }
        if (keys[SDL_SCANCODE_X] && !prevKeys[SDL_SCANCODE_X]) {
            edgesOnTop = !edgesOnTop;
        }
        if (keys[SDL_SCANCODE_C] && !prevKeys[SDL_SCANCODE_C]) {
            if (polygonFillMode > 0) {
                polygonFillMode--;
            } else {
                polygonFillMode = 3;
            }
        }
        if (keys[SDL_SCANCODE_V] && !prevKeys[SDL_SCANCODE_V]) {
            if (polygonFillMode < 3) {
                polygonFillMode++;
            } else {
                polygonFillMode = 0;
            }
        }
        if (keys[SDL_SCANCODE_B] && !prevKeys[SDL_SCANCODE_B]) {
            altUVCalc = !altUVCalc;
        }

        if (keys[SDL_SCANCODE_1] && !prevKeys[SDL_SCANCODE_1]) {
            ax = 32;
            ay = 38;
            bx = 225;
            by = 52;
            cx = 431;
            cy = 254;
            dx = 59;
            dy = 273;
        }
        if (keys[SDL_SCANCODE_2] && !prevKeys[SDL_SCANCODE_2]) {
            ax = 260;
            ay = 218;
            bx = 135;
            by = 141;
            cx = 240;
            cy = 75;
            dx = 346;
            dy = 138;
        }
        if (keys[SDL_SCANCODE_3] && !prevKeys[SDL_SCANCODE_3]) {
            ax = 181;
            ay = 241;
            bx = 373;
            by = 29;
            cx = 95;
            cy = 37;
            dx = 52;
            dy = 103;
        }
        if (keys[SDL_SCANCODE_4] && !prevKeys[SDL_SCANCODE_4]) {
            ax = 200;
            ay = 100;
            bx = 300;
            by = 100;
            cx = 300;
            cy = 200;
            dx = 200;
            dy = 200;
        }
        if (keys[SDL_SCANCODE_5] && !prevKeys[SDL_SCANCODE_5]) {
            ax = 250;
            ay = 150;
            bx = 251;
            by = 150;
            cx = 251;
            cy = 151;
            dx = 250;
            dy = 151;
        }
        if (keys[SDL_SCANCODE_6] && !prevKeys[SDL_SCANCODE_6]) {
            ax = 197;
            ay = 341;
            bx = 58;
            by = 97;
            cx = 302;
            cy = -41;
            dx = 441;
            dy = 202;
        }
        if (keys[SDL_SCANCODE_7] && !prevKeys[SDL_SCANCODE_7]) {
            ax = 325;
            ay = 175;
            bx = 322;
            by = 12;
            cx = 112;
            cy = 84;
            dx = 115;
            dy = 280;
        }
        if (keys[SDL_SCANCODE_8] && !prevKeys[SDL_SCANCODE_8]) {
            ax = 214;
            ay = 60;
            bx = 353;
            by = 120;
            cx = 285;
            cy = 243;
            dx = 144;
            dy = 188;
        }
        if (keys[SDL_SCANCODE_9] && !prevKeys[SDL_SCANCODE_9]) {
            ax = 372;
            ay = 155;
            bx = 244;
            by = 272;
            cx = 127;
            cy = 144;
            dx = 255;
            dy = 27;
        }

        if (keyRepeat[SDL_SCANCODE_KP_PLUS]) {
            lineStep++;
        }
        if (keyRepeat[SDL_SCANCODE_KP_MINUS]) {
            if (lineStep > 1) {
                lineStep--;
                lineOffset = lineOffset % lineStep;
            }
        }
        if (keyRepeat[SDL_SCANCODE_KP_MULTIPLY]) {
            lineOffset++;
            lineOffset = lineOffset % lineStep;
        }
        if (keyRepeat[SDL_SCANCODE_KP_DIVIDE]) {
            if (lineOffset > 0) {
                lineOffset--;
            } else {
                lineOffset = lineStep - 1;
            }
        }

        if (keys[SDL_SCANCODE_W]) {
            ay -= inc;
        }
        if (keys[SDL_SCANCODE_S]) {
            ay += inc;
        }
        if (keys[SDL_SCANCODE_A]) {
            ax -= inc;
        }
        if (keys[SDL_SCANCODE_D]) {
            ax += inc;
        }

        if (keys[SDL_SCANCODE_T]) {
            by -= inc;
        }
        if (keys[SDL_SCANCODE_G]) {
            by += inc;
        }
        if (keys[SDL_SCANCODE_F]) {
            bx -= inc;
        }
        if (keys[SDL_SCANCODE_H]) {
            bx += inc;
        }

        if (keys[SDL_SCANCODE_I]) {
            cy -= inc;
        }
        if (keys[SDL_SCANCODE_K]) {
            cy += inc;
        }
        if (keys[SDL_SCANCODE_J]) {
            cx -= inc;
        }
        if (keys[SDL_SCANCODE_L]) {
            cx += inc;
        }

        if (keys[SDL_SCANCODE_UP]) {
            dy -= inc;
        }
        if (keys[SDL_SCANCODE_DOWN]) {
            dy += inc;
        }
        if (keys[SDL_SCANCODE_LEFT]) {
            dx -= inc;
        }
        if (keys[SDL_SCANCODE_RIGHT]) {
            dx += inc;
        }

        if (keys[SDL_SCANCODE_KP_8]) {
            ay -= inc;
            by -= inc;
            cy -= inc;
            dy -= inc;
        }
        if (keys[SDL_SCANCODE_KP_5]) {
            ay += inc;
            by += inc;
            cy += inc;
            dy += inc;
        }
        if (keys[SDL_SCANCODE_KP_4]) {
            ax -= inc;
            bx -= inc;
            cx -= inc;
            dx -= inc;
        }
        if (keys[SDL_SCANCODE_KP_6]) {
            ax += inc;
            bx += inc;
            cx += inc;
            dx += inc;
        }

        if (keys[SDL_SCANCODE_HOME]) {
            double centerx = (ax + bx + cx + dx) / 4.0;
            double centery = (ay + by + cy + dy) / 4.0;
            ax += (ax - centerx) * inc * 0.01;
            ay += (ay - centery) * inc * 0.01;
            bx += (bx - centerx) * inc * 0.01;
            by += (by - centery) * inc * 0.01;
            cx += (cx - centerx) * inc * 0.01;
            cy += (cy - centery) * inc * 0.01;
            dx += (dx - centerx) * inc * 0.01;
            dy += (dy - centery) * inc * 0.01;
        }
        if (keys[SDL_SCANCODE_END]) {
            double centerx = (ax + bx + cx + dx) / 4.0;
            double centery = (ay + by + cy + dy) / 4.0;
            ax -= (ax - centerx) * inc * 0.01;
            ay -= (ay - centery) * inc * 0.01;
            bx -= (bx - centerx) * inc * 0.01;
            by -= (by - centery) * inc * 0.01;
            cx -= (cx - centerx) * inc * 0.01;
            cy -= (cy - centery) * inc * 0.01;
            dx -= (dx - centerx) * inc * 0.01;
            dy -= (dy - centery) * inc * 0.01;
        }

        if (keys[SDL_SCANCODE_PAGEUP]) {
            double centerx = (ax + bx + cx + dx) / 4.0;
            double centery = (ay + by + cy + dy) / 4.0;
            double s = sin(-inc / 150.0);
            double c = cos(-inc / 150.0);
            double nax = (ax - centerx) * c - (ay - centery) * s + centerx;
            double nay = (ax - centerx) * s + (ay - centery) * c + centery;
            double nbx = (bx - centerx) * c - (by - centery) * s + centerx;
            double nby = (bx - centerx) * s + (by - centery) * c + centery;
            double ncx = (cx - centerx) * c - (cy - centery) * s + centerx;
            double ncy = (cx - centerx) * s + (cy - centery) * c + centery;
            double ndx = (dx - centerx) * c - (dy - centery) * s + centerx;
            double ndy = (dx - centerx) * s + (dy - centery) * c + centery;
            ax = nax;
            ay = nay;
            bx = nbx;
            by = nby;
            cx = ncx;
            cy = ncy;
            dx = ndx;
            dy = ndy;
        }
        if (keys[SDL_SCANCODE_PAGEDOWN]) {
            double centerx = (ax + bx + cx + dx) / 4.0;
            double centery = (ay + by + cy + dy) / 4.0;
            double s = sin(inc / 150.0);
            double c = cos(inc / 150.0);
            double nax = (ax - centerx) * c - (ay - centery) * s + centerx;
            double nay = (ax - centerx) * s + (ay - centery) * c + centery;
            double nbx = (bx - centerx) * c - (by - centery) * s + centerx;
            double nby = (bx - centerx) * s + (by - centery) * c + centery;
            double ncx = (cx - centerx) * c - (cy - centery) * s + centerx;
            double ncy = (cx - centerx) * s + (cy - centery) * c + centery;
            double ndx = (dx - centerx) * c - (dy - centery) * s + centerx;
            double ndy = (dx - centerx) * s + (dy - centery) * c + centery;
            ax = nax;
            ay = nay;
            bx = nbx;
            by = nby;
            cx = ncx;
            cy = ncy;
            dx = ndx;
            dy = ndy;
        }

        if (keys[SDL_SCANCODE_SPACE] && !prevKeys[SDL_SCANCODE_SPACE]) {
            fmt::println("A = {}x{}  B = {}x{}  C = {}x{}  D = {}x{}", (int)ax, (int)ay, (int)bx, (int)by, (int)cx,
                         (int)cy, (int)dx, (int)dy);
        }

        prevKeys = keys;

        std::fill(framebuffer.begin(), framebuffer.end(), 0xFF000000);

        const CoordS32 coordA{(int)ax, (int)ay};
        const CoordS32 coordB{(int)bx, (int)by};
        const CoordS32 coordC{(int)cx, (int)cy};
        const CoordS32 coordD{(int)dx, (int)dy};

        if (!edgesOnTop) {
            for (LineStepper line{coordA, coordD}; line.CanStep(); line.Step()) {
                auto [x, y] = line.Coord();
                DrawPixel(x, y, 0x51b7c4);
            }

            for (LineStepper line{coordB, coordC}; line.CanStep(); line.Step()) {
                auto [x, y] = line.Coord();
                DrawPixel(x, y, 0xc45183);
            }

            for (LineStepper line{coordA, coordB}; line.CanStep(); line.Step()) {
                auto [x, y] = line.Coord();
                DrawPixel(x, y, 0xb7c451);
            }

            for (LineStepper line{coordC, coordD}; line.CanStep(); line.Step()) {
                auto [x, y] = line.Coord();
                DrawPixel(x, y, 0x5183c4);
            }

            DrawPixel(ax, ay, 0x4f52ff);
            DrawPixel(bx, by, 0x4fff98);
            DrawPixel(cx, cy, 0xffa74f);
            DrawPixel(dx, dy, 0xff4fb6);
        }

        // bool swapped;
        uint32 texSize = polygonFillMode == 2 ? 8 : polygonFillMode == 3 ? 32 : 256;
        uint32 texShift = polygonFillMode == 2 ? 13 : polygonFillMode == 3 ? 11 : 8;
        bool first = true;
        int lineIndex = 0;
        for (TexturedQuadEdgesStepper edge{coordA, coordB, coordC, coordD, texSize, false}; edge.CanStep();
             edge.Step()) {
            const CoordS32 coordL{edge.LX(), edge.LY()};
            const CoordS32 coordR{edge.RX(), edge.RY()};

            const uint32 v = altUVCalc ? (edge.FracPos() >> texShift) : edge.V();

            bool firstPixel = true;
            if (lineIndex % lineStep == lineOffset) {
                for (TexturedLineStepper line{coordL, coordR, texSize, false}; line.CanStep(); line.Step()) {
                    auto [x, y] = line.Coord();
                    const uint32 u = altUVCalc ? (line.FracPos() >> texShift) : line.U();

                    uint32 color;
                    switch (polygonFillMode) {
                    case 0: color = firstPixel ? 0xc7997c : first ? 0x96674a : 0x75492e; break;
                    case 1:
                        color = (u & 0xFF) | ((v & 0xFF) << 8u) | (firstPixel * 0xFF0000) | (first * 0x7F0000);
                        break;
                    case 2:
                    case 3:
                        color = ((u ^ v) & 1) ? 0xFFFFFF : 0x000000;
                        color ^= (firstPixel * 0xFF0000) | (first * 0x7F0000);
                        break;
                    }

                    DrawPixel(x, y, color);
                    if (antialias && line.NeedsAntiAliasing()) {
                        auto [aax, aay] = line.AACoord();
                        DrawPixel(aax, aay, color);
                    }
                    firstPixel = false;
                }
            }
            lineIndex++;
            // DrawPixel(coordL.x, coordL.y, 0xFF00FF);
            // DrawPixel(coordR.x, coordR.y, 0xFF00FF);
            if (first) {
                // swapped = edge.Swapped();
                first = false;
            }
        }

        if (edgesOnTop) {
            for (LineStepper line{coordA, coordD}; line.CanStep(); line.Step()) {
                auto [x, y] = line.Coord();
                DrawPixel(x, y, 0x51b7c4);
            }

            for (LineStepper line{coordB, coordC}; line.CanStep(); line.Step()) {
                auto [x, y] = line.Coord();
                DrawPixel(x, y, 0xc45183);
            }

            for (LineStepper line{coordA, coordB}; line.CanStep(); line.Step()) {
                auto [x, y] = line.Coord();
                DrawPixel(x, y, 0xb7c451);
            }

            for (LineStepper line{coordC, coordD}; line.CanStep(); line.Step()) {
                auto [x, y] = line.Coord();
                DrawPixel(x, y, 0x5183c4);
            }

            DrawPixel(ax, ay, 0x4f52ff);
            DrawPixel(bx, by, 0x4fff98);
            DrawPixel(cx, cy, 0xffa74f);
            DrawPixel(dx, dy, 0xff4fb6);
        }

        /*if (swapped) {
            DrawPixel((ax + bx + cx + dx) / 4, (ay + by + cy + dy) / 4, 0xFFFFFF);
        }*/

        lastTicks = SDL_GetTicks();
    }

    void DrawPixel(sint32 x, sint32 y, uint32 color) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            framebuffer[x + y * width] = color | 0xFF000000;
        }
    }

    double DeltaTime() const {
        return (SDL_GetTicks() - lastTicks) / 1000.0;
    }

    std::vector<uint32> framebuffer;
    uint32 width, height;
    double ax, ay;
    double bx, by;
    double cx, cy;
    double dx, dy;

    bool edgesOnTop = true;
    bool antialias = true;
    bool altUVCalc = false;

    // 0 = solid blue, 1 = UV gradient, 2 = 8x8 checkerboard texture, 3 = 32x32 checkerboard texture
    int polygonFillMode = 0;

    int lineStep = 1;
    int lineOffset = 0;

    uint64 lastTicks;

    std::array<bool, SDL_SCANCODE_COUNT> keys;
    std::array<bool, SDL_SCANCODE_COUNT> prevKeys;
    std::array<double, SDL_SCANCODE_COUNT> keyDownLen;
    std::array<bool, SDL_SCANCODE_COUNT> keyRepeat;
};

void runSandbox() {
    using clk = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    // Screen parameters
    const uint32 screenWidth = 500;
    const uint32 screenHeight = 300;
    const uint32 scale = 3;

    // ---------------------------------
    // Initialize SDL video subsystem

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return;
    }
    ScopeGuard sgQuit{[] { SDL_Quit(); }};

    // ---------------------------------
    // Create window

    SDL_PropertiesID windowProps = SDL_CreateProperties();
    if (windowProps == 0) {
        SDL_Log("Unable to create window properties: %s", SDL_GetError());
        return;
    }
    ScopeGuard sgDestroyWindowProps{[&] { SDL_DestroyProperties(windowProps); }};

    // Assume the following calls succeed
    SDL_SetStringProperty(windowProps, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Sandbox");
    SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, false);
    SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, screenWidth * scale);
    SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, screenHeight * scale);
    SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);

    auto window = SDL_CreateWindowWithProperties(windowProps);
    if (window == nullptr) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        return;
    }
    ScopeGuard sgDestroyWindow{[&] { SDL_DestroyWindow(window); }};

    // ---------------------------------
    // Create renderer

    SDL_PropertiesID rendererProps = SDL_CreateProperties();
    if (rendererProps == 0) {
        SDL_Log("Unable to create renderer properties: %s", SDL_GetError());
        return;
    }
    ScopeGuard sgDestroyRendererProps{[&] { SDL_DestroyProperties(rendererProps); }};

    // Assume the following calls succeed
    SDL_SetPointerProperty(rendererProps, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, window);
    // SDL_SetNumberProperty(rendererProps, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, SDL_RENDERER_VSYNC_DISABLED);
    // SDL_SetNumberProperty(rendererProps, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, SDL_RENDERER_VSYNC_ADAPTIVE);
    SDL_SetNumberProperty(rendererProps, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, 1);

    auto renderer = SDL_CreateRendererWithProperties(rendererProps);
    if (renderer == nullptr) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        return;
    }
    ScopeGuard sgDestroyRenderer{[&] { SDL_DestroyRenderer(renderer); }};

    // ---------------------------------
    // Create texture to render on

    auto texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_XBGR8888, SDL_TEXTUREACCESS_STREAMING, screenWidth, screenHeight);
    if (texture == nullptr) {
        SDL_Log("Unable to create texture: %s", SDL_GetError());
        return;
    }
    ScopeGuard sgDestroyTexture{[&] { SDL_DestroyTexture(texture); }};

    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

    // ---------------------------------
    // Main loop

    auto t = clk::now();
    uint64 frames = 0;
    bool running = true;
    bool showHelp = true;

    Sandbox sandbox{screenWidth, screenHeight};

    while (running) {
        SDL_Event evt{};
        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
            case SDL_EVENT_KEY_DOWN:
                sandbox.KeyDown(evt);
                if (evt.key.scancode == SDL_SCANCODE_F1) {
                    showHelp = !showHelp;
                }
                break;
            case SDL_EVENT_KEY_UP: sandbox.KeyUp(evt); break;
            case SDL_EVENT_QUIT: running = false; break;
            }
        }

        sandbox.Frame();

        ++frames;
        auto t2 = clk::now();
        if (t2 - t >= 1s) {
            auto title = fmt::format("{} fps", frames);
            SDL_SetWindowTitle(window, title.c_str());
            frames = 0;
            t = t2;
        }

        uint32 *pixels = nullptr;
        int pitch = 0;
        if (SDL_LockTexture(texture, nullptr, (void **)&pixels, &pitch)) {
            std::copy_n(sandbox.framebuffer.begin(), screenWidth * screenHeight, pixels);
            SDL_UnlockTexture(texture);
        }

        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);

        if (showHelp) {
            SDL_FRect rect{187, 49, 10, 10};
            SDL_SetRenderDrawColor(renderer, 255, 82, 79, 128);
            SDL_RenderFillRect(renderer, &rect);

            rect.y += 10;
            SDL_SetRenderDrawColor(renderer, 152, 255, 79, 128);
            SDL_RenderFillRect(renderer, &rect);

            rect.y += 10;
            SDL_SetRenderDrawColor(renderer, 79, 167, 255, 128);
            SDL_RenderFillRect(renderer, &rect);

            rect.y += 10;
            SDL_SetRenderDrawColor(renderer, 182, 79, 255, 128);
            SDL_RenderFillRect(renderer, &rect);

            SDL_SetRenderDrawColor(renderer, 255, 233, 80, 255);
            SDL_RenderDebugText(renderer, 5, 5,
                                fmt::format("[Z] Antialias {}", (sandbox.antialias ? "ON" : "OFF")).c_str());
            SDL_RenderDebugText(
                renderer, 5, 15,
                fmt::format("[X] Draw edges {} polygon", (sandbox.edgesOnTop ? "above" : "below")).c_str());
            SDL_RenderDebugText(
                renderer, 5, 25,
                fmt::format("[CV] Polygon fill: {}", (sandbox.polygonFillMode == 0   ? "solid color"
                                                      : sandbox.polygonFillMode == 1 ? "UV gradient"
                                                      : sandbox.polygonFillMode == 2 ? "8x8 checkerboard"
                                                                                     : "32x32 checkerboard"))
                    .c_str());
            SDL_RenderDebugText(
                renderer, 5, 35,
                fmt::format("[B] Use {} UV calculation", (sandbox.altUVCalc ? "alternate" : "primary")).c_str());
            SDL_RenderDebugText(renderer, 5, 45, "[123456789] Select preset shape");

            SDL_RenderDebugText(
                renderer, 5, 60,
                fmt::format("[WASD]   Move vertex A   {}x{}", (int)sandbox.ax, (int)sandbox.ay).c_str());
            SDL_RenderDebugText(
                renderer, 5, 70,
                fmt::format("[TFGH]   Move vertex B   {}x{}", (int)sandbox.bx, (int)sandbox.by).c_str());
            SDL_RenderDebugText(
                renderer, 5, 80,
                fmt::format("[IJKL]   Move vertex C   {}x{}", (int)sandbox.cx, (int)sandbox.cy).c_str());
            SDL_RenderDebugText(
                renderer, 5, 90,
                fmt::format("[Arrows] Move vertex D   {}x{}", (int)sandbox.dx, (int)sandbox.dy).c_str());
            SDL_RenderDebugText(renderer, 5, 100, "[KP8456]    Translate polygon");
            SDL_RenderDebugText(renderer, 5, 110, "[Home/End]  Scale polygon relative to center");
            SDL_RenderDebugText(renderer, 5, 120, "[PgUp/PgDn] Rotate polygon around center");
            SDL_RenderDebugText(renderer, 5, 130, "[Shift]  Hold to speed up");
            SDL_RenderDebugText(renderer, 5, 140, "[Space]  Print out coordinates to stdout");
            if (sandbox.lineStep == 1) {
                SDL_RenderDebugText(renderer, 5, 155, "[KP+-] Draw every line");
            } else {
                SDL_RenderDebugText(renderer, 5, 155,
                                    fmt::format("[KP+-] Draw every {} lines", sandbox.lineStep).c_str());
            }
            SDL_RenderDebugText(renderer, 5, 165,
                                fmt::format("[KP*/] ... starting from line {}", sandbox.lineOffset).c_str());
            SDL_RenderDebugText(renderer, 5, 180, "[F1] Show/hide this text");
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
}

void runBUPSandbox() {
    // Valid backup memory parameters:
    // Device      Size     Block size
    // Internal    32 KiB   64 b
    // External    512 KiB  512 b
    // External    1 MiB    512 b
    // External    2 MiB    512 b
    // External    4 MiB    1 KiB

    ymir::bup::BackupMemory mem{};
    std::error_code error{};
    mem.CreateFrom("bup-int.bin", ymir::bup::BackupMemorySize::_256Kbit, error);
    if (error) {
        fmt::println("Failed to read backup memory file: {}", error.message());
        return;
    }
    mem.Delete("GBASICSS_01");

    ymir::bup::BackupFile file{};
    file.header.filename = "ANDROMEDA_3";
    file.header.comment = "ANDROMEDA_";
    file.header.date = 0;
    file.header.language = ymir::bup::Language::Japanese;
    for (uint32 i = 0; i < 256; i++) {
        file.data.push_back(i);
    }
    file.data.push_back('t');
    file.data.push_back('e');
    file.data.push_back('s');
    file.data.push_back('t');
    /*file.data.push_back('0');
    file.data.push_back('1');
    file.data.push_back('2');
    file.data.push_back('3');*/

    auto result = mem.Import(file, true);
    switch (result) {
    case ymir::bup::BackupFileImportResult::Imported: fmt::println("File imported successfully"); break;
    case ymir::bup::BackupFileImportResult::Overwritten: fmt::println("File overwritten successfully"); break;
    case ymir::bup::BackupFileImportResult::FileExists: fmt::println("File not imported: file already exists"); break;
    case ymir::bup::BackupFileImportResult::NoSpace: fmt::println("File not imported: not enough space"); break;
    }

    const uint32 usedBlocks = mem.GetUsedBlocks();
    const uint32 totalBlocks = mem.GetTotalBlocks();
    fmt::println("Backup memory size: {} bytes", mem.Size());
    fmt::println("Blocks: {} of {} used ({} free)", usedBlocks, totalBlocks, totalBlocks - usedBlocks);

    static constexpr const char *kLanguages[] = {"JP", "EN", "FR", "DE", "SP", "IT"};

    for (const auto &file : mem.List()) {
        auto trimToNull = [](std::string &str) {
            auto zeroPos = str.find_first_of('\0');
            if (zeroPos != std::string::npos) {
                str.resize(zeroPos, '\0');
            }
        };

        std::string filename = file.header.filename;
        std::string comment = file.header.comment;
        trimToNull(filename);
        trimToNull(comment);
        std::transform(filename.begin(), filename.end(), filename.begin(), [](char c) { return c < 0 ? '?' : c; });
        std::transform(comment.begin(), comment.end(), comment.begin(), [](char c) { return c < 0 ? '?' : c; });

        fmt::println("{:11s} | {:10s} | {} | {:3d} | {:6d} bytes | {:02d} {:02d}:{:02d}", filename, comment,
                     kLanguages[static_cast<uint8>(file.header.language)], file.numBlocks, file.size,
                     file.header.date / 60 / 24, (file.header.date / 60) % 24, file.header.date % 60);

        auto optFileData = mem.Export(file.header.filename);
        if (optFileData) {
            auto &fileData = *optFileData;
            uint32 pos = 0;
            for (auto b : fileData.data) {
                if (pos % 16 == 0) {
                    fmt::print("  {:06X} |", pos);
                }
                if (pos % 16 == 8) {
                    fmt::print(" ");
                }
                fmt::print(" {:02X}", b);
                if (pos % 16 == 15 || pos == fileData.data.size() - 1) {
                    fmt::println("");
                }
                pos++;
            }
        }
    }
}

void runInputSandbox() {
    using clk = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    // Screen parameters
    const uint32 screenWidth = 500;
    const uint32 screenHeight = 300;
    const uint32 scale = 3;

    // ---------------------------------
    // Initialize SDL subsystems

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMEPAD)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return;
    }
    ScopeGuard sgQuit{[] { SDL_Quit(); }};

    // ---------------------------------
    // Open all gamepads

    int gamepadsCount = 0;
    SDL_JoystickID *gamepadIDs = SDL_GetGamepads(&gamepadsCount);
    std::vector<SDL_Gamepad *> gamepads{};
    for (int n = 0; n < gamepadsCount; n++) {
        if (SDL_Gamepad *gamepad = SDL_OpenGamepad(gamepadIDs[n])) {
            gamepads.push_back(gamepad);
        }
    }

    // ---------------------------------
    // Create window

    SDL_PropertiesID windowProps = SDL_CreateProperties();
    if (windowProps == 0) {
        SDL_Log("Unable to create window properties: %s", SDL_GetError());
        return;
    }
    ScopeGuard sgDestroyWindowProps{[&] { SDL_DestroyProperties(windowProps); }};

    // Assume the following calls succeed
    SDL_SetStringProperty(windowProps, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Sandbox");
    SDL_SetBooleanProperty(windowProps, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, false);
    SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, screenWidth * scale);
    SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, screenHeight * scale);
    SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
    SDL_SetNumberProperty(windowProps, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);

    auto window = SDL_CreateWindowWithProperties(windowProps);
    if (window == nullptr) {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        return;
    }
    ScopeGuard sgDestroyWindow{[&] { SDL_DestroyWindow(window); }};

    // ---------------------------------
    // Create renderer

    SDL_PropertiesID rendererProps = SDL_CreateProperties();
    if (rendererProps == 0) {
        SDL_Log("Unable to create renderer properties: %s", SDL_GetError());
        return;
    }
    ScopeGuard sgDestroyRendererProps{[&] { SDL_DestroyProperties(rendererProps); }};

    // Assume the following calls succeed
    SDL_SetPointerProperty(rendererProps, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, window);
    // SDL_SetNumberProperty(rendererProps, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, SDL_RENDERER_VSYNC_DISABLED);
    // SDL_SetNumberProperty(rendererProps, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, SDL_RENDERER_VSYNC_ADAPTIVE);
    SDL_SetNumberProperty(rendererProps, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, 1);

    auto renderer = SDL_CreateRendererWithProperties(rendererProps);
    if (renderer == nullptr) {
        SDL_Log("Unable to create renderer: %s", SDL_GetError());
        return;
    }
    ScopeGuard sgDestroyRenderer{[&] { SDL_DestroyRenderer(renderer); }};

    // ---------------------------------
    // Main loop

    auto t = clk::now();
    auto tNext = t + 16666667ns;
    uint64 frames = 0;
    bool running = true;

    bool pressed = false;

    while (running) {
        SDL_Event evt{};

        while (SDL_PollEvent(&evt)) {
            switch (evt.type) {
            case SDL_EVENT_MOUSE_BUTTON_DOWN: pressed = true; break;
            case SDL_EVENT_MOUSE_BUTTON_UP: pressed = false; break;
            case SDL_EVENT_GAMEPAD_BUTTON_DOWN: pressed = true; break;
            case SDL_EVENT_GAMEPAD_BUTTON_UP: pressed = false; break;
            case SDL_EVENT_QUIT: running = false; break;
            }
        }

        while (clk::now() < tNext) {
        }
        tNext += 16666667ns;

        ++frames;
        auto t2 = clk::now();
        if (t2 - t >= 1s) {
            auto title = fmt::format("{} fps", frames);
            SDL_SetWindowTitle(window, title.c_str());
            frames = 0;
            t = t2;
        }

        SDL_SetRenderDrawColor(renderer, 255, pressed * 255, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
}

int main(int argc, char **argv) {
    // runSandbox();
    // runBUPSandbox();
    runInputSandbox();

    return EXIT_SUCCESS;
}
