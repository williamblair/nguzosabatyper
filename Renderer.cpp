#include "Renderer.h"

Renderer::Renderer() :
    mWindow(nullptr),
    mRender(nullptr),
    mWidth(0),
    mHeight(0)
{
}

Renderer::~Renderer()
{
    if (mRender != nullptr) {
        SDL_DestroyRenderer(mRender);
    }
    if (mWindow != nullptr) {
        SDL_DestroyWindow(mWindow);
    }
}

void Renderer::Init(const char* title, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << __FILE__ << ": " << __LINE__ << ": "
            << "Failed to init SDL: " << SDL_GetError() << std::endl;
        throw std::runtime_error(SDL_GetError());
    }
    {
        int flags = IMG_INIT_JPG | IMG_INIT_PNG;
        if ((IMG_Init(flags) & flags) != flags) {
            std::cerr << __FILE__ << ": " << __LINE__ << ": "
                << "Failed to init IMG: " << IMG_GetError()
                << std::endl;
            throw std::runtime_error(IMG_GetError());
        }
    }
    
    mWindow = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        0
    );
    if (!mWindow) {
        std::cerr << __FILE__ << ": " << __LINE__ << ": "
            << "Failed to create window: " << SDL_GetError()
            << std::endl;
        throw std::runtime_error(SDL_GetError());
    }
    
    mRender = SDL_CreateRenderer(mWindow, -1, 0);
    if (!mRender) {
        std::cerr << __FILE__ << ": " << __LINE__ << ": "
            << "Failed to create renderer: " << SDL_GetError()
            << std::endl;
        throw std::runtime_error(SDL_GetError());
    }
}

void Renderer::Clear()
{
    
}

void Renderer::Update()
{
    SDL_RenderPresent(mRender);
}
