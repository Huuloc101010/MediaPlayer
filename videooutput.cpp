#include <SDL2/SDL.h>
#include <vector>
#include "videooutput.h"
int videooutput::output()
{
    SDL_Init(SDL_INIT_VIDEO);

    int width = 640;
    int height = 480;

    SDL_Window* window = SDL_CreateWindow("YUV Dynamic Video", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture* texture = SDL_CreateTexture(renderer, 
        SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);

    // Khởi tạo các mảng dữ liệu
    std::vector<uint8_t> y_plane(width * height);
    std::vector<uint8_t> u_plane((width / 2) * (height / 2));
    std::vector<uint8_t> v_plane((width / 2) * (height / 2));

    bool quit = false;
    SDL_Event e;
    uint8_t color_offset = 0; // Biến dùng để thay đổi màu sắc theo thời gian

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) quit = true;
        }

        // --- 1. CẬP NHẬT DỮ LIỆU (Giả lập logic của Decoder FFmpeg) ---
        
        // Làm mới Luma (Y) - Tạo hiệu ứng gradient chạy qua lại
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                y_plane[y * width + x] = (uint8_t)((x * 255 / width) + color_offset) % 256;
            }
        }

        // Làm mới Chroma (U & V) - Thay đổi màu sắc toàn bộ khung hình
        for (int i = 0; i < (width / 2) * (height / 2); i++) {
            u_plane[i] = 128 + color_offset; 
            v_plane[i] = 128 - color_offset;
        }

        color_offset++; // Tăng offset để khung hình sau khác khung hình trước

        // --- 2. HIỂN THỊ (Render) ---

        // Đẩy dữ liệu mới lên GPU
        SDL_UpdateYUVTexture(texture, NULL, 
            y_plane.data(), width,           
            u_plane.data(), width / 2,       
            v_plane.data(), width / 2);      

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(16); // Giữ tốc độ khoảng 60 khung hình/giây (1000ms / 60)
    }

    // Giải phóng...
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}