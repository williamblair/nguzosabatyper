#include <string>
#include <Renderer.h>
#include <Input.h>
#include <Texture.h>
#include <Music.h>
#include <SoundEffect.h>
#include <GameTimer.h>
#include <Font.h>

/* MinGW SDL compile error thing */
#ifdef __WIN32
#undef main
#endif

// idea - match symbols to their names memory/guessing game
// idea - clicking speed game (like osu kind of) of each
//          of the symbols
// idea - click matching symbols before a certain time
// idea - click symbols before they fall and hit ground
// idea - type name of symbol before time runs out

/* Idea - complete all minigames as fast as possible
 * one minigame for each principle or something like that */

/* First fruits minigame
 * https://www.officialkwanzaawebsite.org/roots-and-branches.html
 * matunda ya kwanza 
 * 
 * falling fruits, you have to try and catch them in a basket
 * moving left to right or something */

/* umoja (unity):
 *  Early in life continental African children are taught to memorize and recite their family tree as far back as any ancestor is known. This keeps historical memory alive and reaffirms respect for those living and departed who contributed to their coming into being and cultural molding. 
 * https://www.officialkwanzaawebsite.org/umoja.html */

static Renderer render;
static Input input;
static Texture texture;
static Music music;
static SoundEffect effect;
static GameTimer timer;
static Font font;

void TypingGameLoop()
{
    std::string curUserStr = "";
    const int maxUserStrLen = 10;
    
    bool quit = false;
    while (!quit)
    {
        render.Clear();
        const float dt = timer.Update();
        
        if (curUserStr.size() > 0)
        {
            font.Draw(render, 10, 10, curUserStr.c_str());
        }
        
        if (input.Confirm()) {
            std::cout << "User str was: " << curUserStr << std::endl;
            curUserStr = "";
        }
        if ((int)input.CharEntered() != 0 &&
            curUserStr.size() < maxUserStrLen)
        {
            curUserStr += input.CharEntered();
        }
        
        quit = input.Quit();
        input.Update();
        render.Update();
    }
}

/* 7 principles of kwanza is the theme (nguzo saba) */
int main(int argc, char **argv)
{
    try
    {
        render.Init("PDubs Holiday Jam 2021", 640, 480);
        texture.Init("assets/kwanzaCandle.jpg", render);
        font.Init("assets/SaikyoBlack.png", 18, 18, render); 
        
        music.Init("assets/caravan.ogg");
        music.Play(true);
        
        effect.Init("assets/load.wav");
        
        while (!input.Quit())
        {
            /* Gameplay loop */
            TypingGameLoop();
        }
    }
    catch (std::runtime_error& e)
    {
        std::cerr << "Main caught runtime error: " 
            << e.what() 
            << std::endl;
        return 1;
    }
    return 0;
}
