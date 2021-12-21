#include <unordered_map>

#include <Input.h>

Input::Input() :
    mQuit(false),
    mConfirm(false),
    mCharEntered((char)0),
    mBackSpace(false),
    mUp(false),
    mDown(false),
    mUpHeld(false),
    mDownHeld(false)
{
}
Input::~Input()
{
}

static std::unordered_map<SDL_Keycode, char> inputCharMap = {
    {SDLK_a, 'A'},
    {SDLK_b, 'B'},
    {SDLK_c, 'C'},
    {SDLK_d, 'D'},
    {SDLK_e, 'E'},
    {SDLK_f, 'F'},
    {SDLK_g, 'G'},
    {SDLK_h, 'H'},
    {SDLK_i, 'I'},
    {SDLK_j, 'J'},
    {SDLK_k, 'K'},
    {SDLK_l, 'L'},
    {SDLK_m, 'M'},
    {SDLK_n, 'N'},
    {SDLK_o, 'O'},
    {SDLK_p, 'P'},
    {SDLK_q, 'Q'},
    {SDLK_r, 'R'},
    {SDLK_s, 'S'},
    {SDLK_t, 'T'},
    {SDLK_u, 'U'},
    {SDLK_v, 'V'},
    {SDLK_w, 'W'},
    {SDLK_x, 'X'},
    {SDLK_y, 'Y'},
    {SDLK_z, 'Z'}
};

void Input::Update()
{
    SDL_Event e;
    mConfirm = false;
    mCharEntered = (char)0;
    mBackSpace = false;
    mUp = false;
    mDown = false;
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            mQuit = true;
            break;
        case SDL_KEYUP:
            if (inputCharMap.find(e.key.keysym.sym) !=
                inputCharMap.end())
            {
                mCharEntered = inputCharMap[e.key.keysym.sym];
                break;
            }
            switch (e.key.keysym.sym)
            {
            case SDLK_RETURN:
                mConfirm = true;
                break;
            case SDLK_BACKSPACE:
                mBackSpace = true;
                break;
            case SDLK_UP:
                mUp = true;
                mUpHeld = false;
                break;
            case SDLK_DOWN:
                mDown = true;
                mDownHeld = false;
                break;
            default:
                break;
            }
            break;
        case SDL_KEYDOWN:
            switch (e.key.keysym.sym)
            {
            case SDLK_UP:
                mUpHeld = true;
                break;
            case SDLK_DOWN:
                mDownHeld = true;
                break;
            default:
                break;
            }
        default:
            break;
        }
    }
}
