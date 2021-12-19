#ifndef TEXTURE_H_INCLUDED
#define TEXTURE_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <Renderer.h>

class Texture
{
public:
    Texture();
    ~Texture();
    
    void Init(const char* fileName, Renderer& render);
    
    void Draw(
        Renderer& render,
        int x, int y
    );
    
    inline void SetUVWH(int u, int v, int w, int h) {
        mU = u; mV = v;
        mDrawWidth = w;
        mDrawHeight = h;
    }
    
    inline int GetWidth() { return mWidth; }
    inline int GetHeight() { return mHeight; }
    
private:
    SDL_Surface* mSurface;
    SDL_Texture* mTexture;
    
    int mWidth;
    int mHeight;
    int mU;
    int mV;
    int mDrawWidth;
    int mDrawHeight;
};

#endif
