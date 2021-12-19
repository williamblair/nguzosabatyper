#include <Renderer.h>
#include <Input.h>
#include <Texture.h>
#include <Music.h>
#include <SoundEffect.h>

#ifdef __WIN32
#undef main
#endif

/* 7 principles of kwanza is the theme (nguzo saba) */
int main(int argc, char **argv)
{
    Renderer render;
    Input input;
    Texture texture;
    Music music;
    SoundEffect effect;
    
    try
    {
        render.Init("PDubs Holiday Jam 2021", 640, 480);
        texture.Init("assets/kwanzaCandle.jpg", render);
        
        music.Init("assets/caravan.ogg");
        music.Play(true);
        
        effect.Init("assets/load.wav");
        
        while (!input.Quit())
        {
            render.Clear();
            texture.Draw(render, 0,0);
            
            if (input.Confirm()) {
                effect.Play();
            }
            
            input.Update();
            render.Update();
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
