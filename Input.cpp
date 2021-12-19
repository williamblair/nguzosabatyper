#include <Input.h>

Input::Input() :
    mQuit(false),
    mConfirm(false)
{
}
Input::~Input()
{
}

void Input::Update()
{
    SDL_Event e;
    mConfirm = false;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            mQuit = true;
            break;
        case SDL_KEYUP:
            switch (e.key.keysym.sym)
            {
            case SDLK_RETURN:
                mConfirm = true;
                break;
            default:
                break;
            }
        default:
            break;
        }
    }
}
