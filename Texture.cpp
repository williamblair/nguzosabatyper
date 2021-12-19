#include <Texture.h>

Texture::Texture() :
    mSurface(nullptr),
    mTexture(nullptr),
    mWidth(0),
    mHeight(0),
    mU(0),
    mV(0),
    mDrawWidth(0),
    mDrawHeight(0)
{
}

Texture::~Texture()
{
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
    }
    if (mSurface != nullptr) {
        SDL_FreeSurface(mSurface);
    }
}

void Texture::Init(const char* fileName, Renderer& render)
{
    mSurface = IMG_Load(fileName);
    if (!mSurface) {
        std::cerr << __FILE__ << ": " << __LINE__ << ": "
            << "failed to load texture: " << fileName << std::endl;
        throw std::runtime_error("failed to load texture");
    }
    
    mTexture = SDL_CreateTextureFromSurface(render.mRender, mSurface);
    if (!mTexture) {
        std::cerr << __FILE__ << ": " << __LINE__ << ": "
            << "failed to create texture: " << fileName << ": "
            << SDL_GetError() << std::endl;
        throw std::runtime_error("failed to create texture");
    }
    mWidth = mSurface->w;
    mHeight = mSurface->h;
    mDrawWidth = mWidth;
    mDrawHeight = mHeight;
}

void Texture::Draw(Renderer& render, int x, int y)
{
    SDL_Rect dstRect = { x, y, mDrawWidth, mDrawHeight };
    SDL_Rect srcRect = { mU, mV, mDrawWidth, mDrawHeight };
    SDL_RenderCopy(render.mRender, mTexture, &srcRect, &dstRect);
}

