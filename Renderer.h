#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

class Renderer
{

friend class Texture;    

public:
    Renderer();
    ~Renderer();
    
    void Init(const char* title, int width, int height);
    
    void DrawRect(
        int x, int y,
        int width, int height,
        unsigned char r,
        unsigned char g,
        unsigned char b,
        unsigned char a
    );
    
    void Clear();
    void Update();
    
private:
    SDL_Window* mWindow;
    SDL_Renderer* mRender;
    int mWidth;
    int mHeight;
    Uint32 mLastTicks;
};

#endif // RENDERER_H_INCLUDED
