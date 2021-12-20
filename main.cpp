#include <string>
#include <cstdlib>
#include <ctime>
#include <unordered_map>
#include <vector>
#include <list>
#include <queue>
#include <cstring>
#include <cstdint>

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

#define S_WIDTH 800
#define S_HEIGHT 600

static Renderer render;
static Input input;
static Texture texture;
static Texture kwanzaPatternBG;
static Texture symbolKeyTex;
static Texture greenClothTex;
static Music music;
static SoundEffect badEntryEffect;
static SoundEffect goodEntryEffect;
static SoundEffect textEntryEffect;
static GameTimer timer;
static Font font;

#define NUM_SYMBOLS 7
static Texture Symbols[NUM_SYMBOLS];
static std::unordered_map<std::string, Texture*> SymbolsMap;
static std::vector<std::string> SymbolStrings;

void InitSymbols()
{
    Symbols[0].Init("assets/umoja.png", render);
    SymbolsMap["UMOJA"] = &Symbols[0];
    
    Symbols[1].Init("assets/kujichagulia.png", render);
    SymbolsMap["KUJICHAGULIA"] = &Symbols[1];
    
    Symbols[2].Init("assets/ujima.png", render);
    SymbolsMap["UJIMA"] = &Symbols[2];
    
    Symbols[3].Init("assets/ujamaa.png", render);
    SymbolsMap["UJAMAA"] = &Symbols[3];
    
    Symbols[4].Init("assets/nia.png", render);
    SymbolsMap["NIA"] = &Symbols[4];
    
    Symbols[5].Init("assets/kuumba.png", render);
    SymbolsMap["KUUMBA"] = &Symbols[5];
    
    Symbols[6].Init("assets/imani.png", render);
    SymbolsMap["IMANI"] = &Symbols[6];
    
    SymbolStrings = {
        "UMOJA",
        "KUJICHAGULIA",
        "UJIMA",
        "UJAMAA",
        "NIA",
        "KUUMBA",
        "IMANI"
    };
}

inline std::string GetRandomTargetString()
{
    size_t index = rand() % SymbolStrings.size();
    return SymbolStrings[index];
}

struct SymbolPiece
{
    std::string name;
    Texture* symbolTex;
    float x;
    float y;
    uint64_t id;
};

void TypingGameLoop()
{
    static const int maxUserStrLen = 15;
    char curUserStr[maxUserStrLen+1];
    char userScoreStr[50];
    std::string scoreText = "SCORE:";
    int userStrIndex = 0;
    float pieceSpeed = 20.0f;
    float pieceAddTimer = 6.0f; // seconds until new piece added
    uint64_t pieceIdCtr = 0;
    std::list<SymbolPiece> pieces;
    std::queue<uint64_t> pieceRemoveQueue;
    uint64_t userScore = 0;
    size_t userLife = 100;
    size_t level = 1;
    
    memset(curUserStr, 0, sizeof(curUserStr));
    sprintf(userScoreStr, "%llu", userScore);
    
#define AddNewPiece()                                                       \
    {                                                                       \
        std::string str = GetRandomTargetString();                          \
        Texture* tex = SymbolsMap[str];                                     \
        pieces.push_back({                                                  \
            str,                                                            \
            SymbolsMap[str],                                                \
            (float)(rand() % (S_WIDTH - tex->GetWidth() - symbolKeyTex.GetWidth())), \
            (float)(-(tex->GetHeight())),                                   \
            pieceIdCtr++                                                    \
        });                                                                 \
    }

    /* Initial piece */
    AddNewPiece()

#define RemovePiece(inId)                                                 \
    {                                                                     \
        for (auto piece = pieces.begin(); piece != pieces.end(); ++piece) \
        {                                                                 \
            if (piece->id == inId) {                                      \
                std::cout << "Found piece with id, erasing" << std::endl; \
                pieces.erase(piece);                                      \
                break;                                                    \
            }                                                             \
        }                                                                 \
    }
    
    bool quit = false;
    while (!quit)
    {
        render.Clear();
        const float dt = timer.Update();
        
        kwanzaPatternBG.Draw(render, 0,0);
        symbolKeyTex.Draw(render, S_WIDTH-symbolKeyTex.GetWidth(), 0);
        
        for (auto piece = pieces.begin(); piece != pieces.end(); ++piece)
        {
            piece->symbolTex->Draw(render, (int)piece->x, (int)piece->y);
            
            piece->y += pieceSpeed * dt;
            // TODO - figure out value besides 100
            if (piece->y + piece->symbolTex->GetHeight() >= S_HEIGHT - 100) {
                std::cout << "Add piece to remove queue w id: " << piece->id << std::endl;
                pieceRemoveQueue.push(piece->id);
                // TODO - subtract from user health
            }
        }
        
        /* Cloth background for text */
        greenClothTex.Draw(render, 0, S_HEIGHT - greenClothTex.GetHeight());
        /* Current user text input */
        if (userStrIndex > 0) {
            font.Draw(render, 10, S_HEIGHT - 20, curUserStr);
        }
        /* User score */
        font.Draw(render, 10, S_HEIGHT - 50, scoreText.c_str());
        font.Draw(
            render,
            10 + (scoreText.size()+2)*font.GetCharWidth(),
            S_HEIGHT - 50,
            userScoreStr
        );
        
        if (input.Confirm()) {
            curUserStr[userStrIndex] = '\0';
            std::cout << "User str was: " << curUserStr << std::endl;
            bool foundMatch = false;
            for (auto piece = pieces.begin(); piece != pieces.end(); ++piece)
            {
                if (std::string(curUserStr) == piece->name) {
                    std::cout << "Input matches!" << std::endl;
                    userScore += piece->name.size();
                    sprintf(userScoreStr, "%llu", userScore);
                    std::cout << "New Score: " << userScore << std::endl;
                    pieceRemoveQueue.push(piece->id);
                    foundMatch = true;
                    break;
                }
            }
            if (!foundMatch) {
                // TODO - flash screen or text or something
                badEntryEffect.Play();
            }
            else {
                // TODO - some other good indication
                goodEntryEffect.Play();
            }
            memset(curUserStr, 0, sizeof(curUserStr));
            userStrIndex = 0;
        }
        if ((int)input.CharEntered() != 0 &&
            userStrIndex < maxUserStrLen)
        {
            curUserStr[userStrIndex++] = input.CharEntered();
            textEntryEffect.Play();
        }
        if (input.BackSpace())
        {
            if (userStrIndex > 0) {
                userStrIndex--;
                memset(
                    &curUserStr[userStrIndex],
                    0,
                    sizeof(curUserStr)-userStrIndex
                );
            }
        }
        
        while (!pieceRemoveQueue.empty())
        {
            std::cout << "Removing piece from queue" << std::endl;
            uint64_t pieceId = pieceRemoveQueue.front();
            RemovePiece(pieceId);
            pieceRemoveQueue.pop();
        }
        
        pieceAddTimer -= dt;
        if (pieceAddTimer <= 0.0f) {
            AddNewPiece();
            std::cout << "Added new piece" << std::endl;
            pieceAddTimer = 6.0f;
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
        srand(time(0));
        render.Init("PDubs Holiday Jam 2021", S_WIDTH, S_HEIGHT);
        texture.Init("assets/kwanzaCandle.jpg", render);
        kwanzaPatternBG.Init("assets/kwanzaPattern.jpeg", render);
        symbolKeyTex.Init("assets/symbolkey_gimp.png", render);
        greenClothTex.Init("assets/greenclothtexture.jpg", render);
        font.Init("assets/SaikyoBlack.png", 18, 18, render); 
        music.Init("assets/caravan.ogg");
        music.Play(true);
        
        badEntryEffect.Init("assets/badentry.wav");
        goodEntryEffect.Init("assets/goodentry.wav");
        textEntryEffect.Init("assets/congahit.wav");
        
        InitSymbols();
        
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
