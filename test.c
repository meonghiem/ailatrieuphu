#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ONLINE 1
#define OFFLINE 2
#define EXIT_GAME 3
#define ESC -1

typedef struct GameTexture
{
    SDL_Texture *texture;
    int tWidth;
    int tHeight;
} GameTexture;

int check_mouse_pos(SDL_Rect rect)
{
    int x, y;
    SDL_GetMouseState(&x, &y);
    int value = 1;
    if (x < rect.x)
    {
        value = -1;
    }
    else if (x > rect.x + rect.w)
    {
        value = -1;
    }
    else if (y < rect.y)
    {
        value = -1;
    }
    else if (y > rect.y + rect.h)
    {
        value = -1;
    }
    return value;
}

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
const int SCREEN_WIDTH = 1369;
const int SCREEN_HEIGHT = 770;
char *player_name;
char *player_password;
SDL_Color white_color = {255, 255, 255};
SDL_Color black_color = {0, 0, 0};
SDL_Color red_color = {255, 0, 0};
SDL_Color yellow_color = {255, 255, 0};
char **MessageLine;
int current_line = 0;
SDL_Rect inputMessage_rect;
SDL_Rect chatBox_rect;
SDL_Rect send_Button;
SDL_Rect showMessage_rect;
GameTexture *sendButton;
SDL_Rect *messLine_rect;
#define ERROR 0
#define SUCCEED 1
#define INSIDE 1
#define OUTSIDE 0

SDL_Rect CreateRect(int x, int y, int w, int h)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.h = h;
    rect.w = w;
    return rect;
}
int CreateWindow()
{ // Create window
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("SDL could not initialize! SDL_Error %s\n", SDL_GetError());
        return ERROR;
    }
    else
    {
        window = SDL_CreateWindow("Ai La Trieu Phu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            printf("Window could not initialize! SDL_Error: %s\n", SDL_GetError());
            return ERROR;
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (renderer == NULL)
        {
            printf("Cant create render! SDL_Error: %s\n", SDL_GetError());
            return ERROR;
        }
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags))
        {
            printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
            return ERROR;
        }
        if (TTF_Init() == -1)
        {
            printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
            return ERROR;
        }
    }
    return SUCCEED;
}

void load_Texture_Text(const char *text, GameTexture *current, TTF_Font *font, SDL_Color textColor)
{
    SDL_Surface *screen = TTF_RenderUTF8_Blended(font, text, textColor);
    if (screen == NULL)
    {
        printf("Cant create surface! SDL_Error: %s\n", SDL_GetError());
        exit(ERROR);
    }
    current->texture = SDL_CreateTextureFromSurface(renderer, screen);
    if (current->texture == NULL)
    {
        printf("Cant render from surface! SDL_Error: %s\n", SDL_GetError());
        exit(ERROR);
    }
    current->tWidth = screen->w;
    current->tHeight = screen->h;
    SDL_FreeSurface(screen);
}

void load_Texture_IMG(char *path, GameTexture *current)
{ // Load texture
    SDL_Surface *screen = NULL;
    screen = IMG_Load(path);
    // screen = SDL_LoadBMP(path); //can use both BMP vs IMG
    if (screen == NULL)
    {
        printf("Cant create surface! SDL_Error: %s\n", SDL_GetError());
        exit(ERROR);
    }
    current->texture = SDL_CreateTextureFromSurface(renderer, screen);
    SDL_FreeSurface(screen);
}

void destroy_Texture(GameTexture *current)
{ // free texture
    SDL_DestroyTexture(current->texture);
    current->tHeight = 0;
    current->tWidth = 0;
    free(current);
}

void ResetRender()
{ // reset window to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
}

void ClearRenderRect(SDL_Rect rect)
{ // fill color (set render color before use it) inside rect
    SDL_Rect tmp = rect;
    tmp = CreateRect(tmp.x + 1, tmp.y + 1, tmp.w - 2, tmp.h - 2);
    SDL_RenderFillRect(renderer, &tmp);
}

SDL_Rect Render(GameTexture *current, int width, int height)
{ // Render texture and display at position [x,y]
    SDL_Rect rect = {width, height, current->tWidth, current->tHeight};
    SDL_RenderCopy(renderer, current->texture, NULL, &rect);
    SDL_RenderPresent(renderer); // display
    SDL_DestroyTexture(current->texture);
    return rect;
}

void close_win()
{ // Close the game
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    window = NULL;
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
int InGame_Screen(int selection);
int Main_Screen()
{ //
    ResetRender();
    int i;
    int Menu_item = 3;
    SDL_Rect button[3];

    GameTexture **MenuText = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);
    for (i = 0; i < Menu_item; i++)
    {
        MenuText[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));

    SDL_Color textColor = {255, 0, 0};
    SDL_Color highlight_Color = {255, 255, 0};
    TTF_Font *font = TTF_OpenFont("resource/font.ttf", 35);
    char *menu[] = {"OnLine", "Offline", "Thoát"};
    int select[] = {0, 0, 0};
    SDL_Event menu_e;

    load_Texture_IMG("./resource/ailatrieuphu.png", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);

    for (i = 0; i < Menu_item; i++)
    {
        load_Texture_Text(menu[i], MenuText[i], font, textColor);
        button[i] = Render(MenuText[i], (SCREEN_WIDTH - MenuText[i]->tWidth) / 2, (SCREEN_HEIGHT + 245 + MenuText[0]->tHeight * (i * 4 - 2)) / 2);
        SDL_RenderPresent(renderer);
    }
    while (1)
    {
        while (SDL_PollEvent(&menu_e))
        {
            switch (menu_e.type)
            {
            case SDL_QUIT:
                close_win();
                exit(0);
                break;
            case SDL_MOUSEMOTION:
                for (i = 0; i < Menu_item; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        if (select[i] == 0)
                        {
                            select[i] = i + 1;
                            load_Texture_Text(menu[i], MenuText[i], font, highlight_Color);
                            Render(MenuText[i], (SCREEN_WIDTH - MenuText[i]->tWidth) / 2, (SCREEN_HEIGHT + 245 + MenuText[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                    else
                    {
                        if (select[i] != 0)
                        {
                            select[i] = 0;
                            load_Texture_Text(menu[i], MenuText[i], font, textColor);
                            Render(MenuText[i], (SCREEN_WIDTH - MenuText[i]->tWidth) / 2, (SCREEN_HEIGHT + 245 + MenuText[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                for (i = 0; i < Menu_item; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        select[i] = i + 1;
                        return select[i];
                    }
                }
            default:
                break;
            }
        }
    }
    for (i = 0; i < Menu_item; i++)
    {
        SDL_DestroyTexture(MenuText[i]->texture);
        MenuText[i]->tHeight = 0;
        MenuText[i]->tWidth = 0;
    }
    SDL_DestroyTexture(background->texture);
    background->tHeight = 0;
    background->tWidth = 0;
    return -1;
}

int InGame_Screen(int selection)
{
    ResetRender();
    SDL_Event game_e;
    if (selection == EXIT_GAME)
    {
        close_win();
        exit(0);
    }
    else if (selection == ONLINE)
    {
        ResetRender();
        int i;
        int Menu_item = 3;
        SDL_Rect button[3];

        GameTexture **MenuText2 = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);
        for (i = 0; i < Menu_item; i++)
        {
            MenuText2[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
        }
        GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));

        SDL_Color textColor = {255, 0, 0};
        SDL_Color highlight_Color = {255, 255, 0};
        TTF_Font *font = TTF_OpenFont("resource/font.ttf", 35);
        char *menu[] = {"Dang Nhap", "Dang ki", "Thoat"};
        int select[] = {0, 0, 0};
        SDL_Event menu_e;

        load_Texture_IMG("./resource/ailatrieuphu.png", background);
        SDL_Surface *surface = SDL_GetWindowSurface(window);
        background->tWidth = surface->w; // get sizeof background
        background->tHeight = surface->h;
        SDL_FreeSurface(surface);
        Render(background, 0, 0);

        for (i = 0; i < Menu_item; i++)
        {
            load_Texture_Text(menu[i], MenuText2[i], font, textColor);
            button[i] = Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 245 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
            SDL_RenderPresent(renderer);
        }
        while (1)
        {
            while (SDL_PollEvent(&menu_e))
            {
                switch (menu_e.type)
                {
                case SDL_QUIT:
                    close_win();
                    exit(0);
                    break;
                case SDL_MOUSEMOTION:
                    for (i = 0; i < Menu_item; i++)
                    {
                        if (check_mouse_pos(button[i]) == 1)
                        {
                            if (select[i] == 0)
                            {
                                select[i] = i + 1;
                                load_Texture_Text(menu[i], MenuText2[i], font, highlight_Color);
                                Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 245 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                                SDL_RenderPresent(renderer);
                            }
                        }
                        else
                        {
                            if (select[i] != 0)
                            {
                                select[i] = 0;
                                load_Texture_Text(menu[i], MenuText2[i], font, textColor);
                                Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 245 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                                SDL_RenderPresent(renderer);
                            }
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    for (i = 0; i < Menu_item; i++)
                    {
                        if (check_mouse_pos(button[i]) == 1)
                        {
                            select[i] = i + 1;
                            return select[i];
                        }
                    }
                case SDL_KEYDOWN:
                    if (menu_e.key.keysym.sym == SDLK_ESCAPE)
                    {
                        return ESC;
                    }
                    break;
                default:
                    break;
                }
            }
        }
        for (i = 0; i < Menu_item; i++)
        {
            SDL_DestroyTexture(MenuText2[i]->texture);
            MenuText2[i]->tHeight = 0;
            MenuText2[i]->tWidth = 0;
        }
        SDL_DestroyTexture(background->texture);
        background->tHeight = 0;
        background->tWidth = 0;
        return -1;
    }
    else if (selection == OFFLINE)
    {
        ResetRender();
        GameTexture *test2 = (GameTexture *)malloc(sizeof(GameTexture));
        SDL_Color textColor2 = {255, 255, 0};
        TTF_Font *font = TTF_OpenFont("resource/font.ttf", 40);
        char *text = "This is Host Game screen!";
        load_Texture_Text(text, test2, font, textColor2);
        Render(test2, 45, 45);
        SDL_RenderPresent(renderer);
    }
    while (1)
    {
        while (SDL_PollEvent(&game_e))
        {
            switch (game_e.type)
            {
            case SDL_QUIT:
                close_win();
                exit(0);
                break;
            case SDL_KEYDOWN:
                if (game_e.key.keysym.sym == SDLK_ESCAPE)
                {
                    return ESC;
                }
                break;
            default:
                break;
            }
        }
    }
    return 0;
}
int Login_screen(int selection)
{
    ResetRender();
    // khoi tao
    int i;
    int choosing = 0;
    SDL_Rect player_name_rect;
    SDL_Rect player_password_rect;

    // SDL_Rect button[3];
    // GameTexture **menuTexture = (GameTexture **)malloc(sizeof(GameTexture *) * 3);
    // for (i = 0; i < 3; i++)
    // {
    //     menuTexture[i] = (GameTexture *)malloc(sizeof(GameTexture) * 3);
    // }
    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));
    // load font menu
    TTF_Font *font = TTF_OpenFont("resource/arial.ttf", 50);
    SDL_Event menu_e;
    // load image background
    load_Texture_IMG("resource/login.bmp", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);

    // load ten player
    player_name_rect = CreateRect(400, 490, 750, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &player_name_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(player_name_rect);
    player_name = calloc(20, sizeof(char));
    strcpy(player_name, " ");
    // render ten player
    GameTexture *player_name_texture = calloc(1, sizeof(GameTexture));
    TTF_Font *font2 = TTF_OpenFont("resource/arial.ttf", 42);
    load_Texture_Text(player_name, player_name_texture, font2, white_color);
    Render(player_name_texture, player_name_rect.x, player_name_rect.y);

    // load password player
    player_password_rect = CreateRect(400, 590, 750, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &player_password_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(player_password_rect);
    player_password = calloc(20, sizeof(char));
    strcpy(player_password, " ");
    // render password player
    GameTexture *player_password_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(player_password, player_password_texture, font2, white_color);
    Render(player_password_texture, player_password_rect.x, player_password_rect.y);

    // render menu button
    // for (i = 0; i < 3; i++)
    // {
    //     load_Texture_Text(menu_text[i], menuTexture[i], font, red_color);
    //     button[i] = Render(menuTexture[i], 40, (SCREEN_HEIGHT + menuTexture[0]->tHeight * (i * 3 - 2)) / 2);
    // }
    SDL_RenderPresent(renderer);

    while (1)
    {
        while (SDL_PollEvent(&menu_e))
        {
            switch (menu_e.type)
            {
            case SDL_QUIT:
                close_win();
                exit(0);
                break;
            case SDL_MOUSEMOTION: // di chuyen chuot

                break;
            case SDL_MOUSEBUTTONDOWN: // khi chuot click
                if (check_mouse_pos(player_name_rect) == 1)
                {
                    choosing = 1;
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        load_Texture_Text(player_name, player_name_texture, font2, black_color);
                        Render(player_name_texture, player_name_rect.x + 3, player_name_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                else
                {
                    if (check_mouse_pos(player_password_rect) != 1)
                        choosing = 0;
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        load_Texture_Text(player_name, player_name_texture, font2, white_color);
                        Render(player_name_texture, player_name_rect.x + 3, player_name_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (check_mouse_pos(player_password_rect) == 1)
                {
                    choosing = 2;
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        load_Texture_Text(player_password, player_password_texture, font2, black_color);
                        Render(player_password_texture, player_password_rect.x + 3, player_password_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                else
                {
                    if (check_mouse_pos(player_name_rect) != 1)
                        choosing = 0;
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        load_Texture_Text(player_password, player_password_texture, font2, white_color);
                        Render(player_password_texture, player_password_rect.x + 3, player_password_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                break;
            case SDL_TEXTINPUT: // khi co input tu ban phim
                if (strlen(player_name) < 30 && choosing == 1)
                {
                    strcat(player_name, menu_e.text.text);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        load_Texture_Text(player_name, player_name_texture, font2, black_color);
                        Render(player_name_texture, player_name_rect.x + 3, player_name_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (strlen(player_password) < 30 && choosing == 2)
                {
                    strcat(player_password, menu_e.text.text);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        load_Texture_Text(player_password, player_password_texture, font2, black_color);
                        Render(player_password_texture, player_password_rect.x + 3, player_password_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                break;
            case SDL_KEYDOWN: // khi co input key tren ban phim
                if (menu_e.key.keysym.sym == SDLK_BACKSPACE && choosing == 1)
                {
                    player_name[strlen(player_name) - 1] = '\0';
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        player_name_texture->tWidth = 750;
                        load_Texture_Text(player_name, player_name_texture, font2, black_color);
                        Render(player_name_texture, player_name_rect.x + 3, player_name_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (menu_e.key.keysym.sym == SDLK_BACKSPACE && choosing == 2)
                {
                    player_password[strlen(player_password) - 1] = '\0';
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        player_password_texture->tWidth = 750;
                        load_Texture_Text(player_password, player_password_texture, font2, black_color);
                        Render(player_password_texture, player_password_rect.x + 3, player_password_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                break;
            default:
                break;
            }
        }
    }
    // for (i = 0; i < 3; i++)
    // {
    //     destroy_Texture(menuTexture[i]);
    // }
    destroy_Texture(background);
    // destroy_Texture(player_name_texture);
}

int Question_Screen(int selection)
{
    ResetRender();
    int i;
    int choosing = 0;
    SDL_Rect suggest_rect;
    SDL_Rect caihoi_rect;
    SDL_Rect solution[4];

    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));

    load_Texture_IMG("resource/cauhoi.jpg", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);
}

int main(int argc, char **argv)
{
    int quit = 0;
    if (CreateWindow() == 0)
    {
        exit(ERROR);
    }
    int selection = 0;
    do
    {
        // selection = Main_Screen();
        // selection = InGame_Screen(selection);
        // selection = Login_screen(selection);
        Question_Screen(selection);
    } while (selection == ESC);
    close_win();
    return SUCCEED;
}