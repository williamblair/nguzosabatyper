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
    IMG_Quit();
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
}

void Renderer::Init(const char* title, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0) {
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
    {
        int flags = MIX_INIT_OGG | MIX_INIT_MOD;
        if ((Mix_Init(flags) & flags) != flags) {
            std::cerr << __FILE__ << ": " << __LINE__ << ": "
                << "Failed to init Mix: " << Mix_GetError()
                << std::endl;
            throw std::runtime_error(Mix_GetError());
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
    
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        std::cerr << __FILE__ << ": " << __LINE__ << ": "
            << "Failed to open audio: " << Mix_GetError() << std::endl;
        throw std::runtime_error(Mix_GetError());
    }
}

void Renderer::Clear()
{
    
}

void Renderer::Update()
{
    SDL_RenderPresent(mRender);
}
