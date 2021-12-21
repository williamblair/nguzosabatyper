#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include <SDL2/SDL.h>

class Input
{
public:
    Input();
    ~Input();

    void Update();

    inline bool Quit() { return mQuit; }
    inline bool Confirm() { return mConfirm; }
    inline char CharEntered() { return mCharEntered; }
    inline char BackSpace() { return mBackSpace; }
    inline bool Up() { return mUp; }
    inline bool Down() { return mDown; }
    inline bool UpHeld() { return mUpHeld; }
    inline bool DownHeld() { return mDownHeld; }

private:
    bool mQuit;
    bool mConfirm;
    char mCharEntered;
    bool mBackSpace;
    bool mUp;
    bool mDown;
    bool mUpHeld;
    bool mDownHeld;
};

#endif // INPUT_H_INCLUDED
