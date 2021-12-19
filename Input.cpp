#include <Input.h>

Input::Input() :
    mQuit(false)
{
}
Input::~Input()
{
}

void Input::Update()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            mQuit = true;
            break;
        default:
            break;
        }
    }
}
