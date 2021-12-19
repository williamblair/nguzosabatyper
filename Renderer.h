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
    
    void Clear();
    void Update();
    
private:
    SDL_Window* mWindow;
    SDL_Renderer* mRender;
    int mWidth;
    int mHeight;
};

#endif // RENDERER_H_INCLUDED
