#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h> 
#include <stdlib.h> 
#include <time.h>   

// --- CONSTANTES ---
const int ANCHO_VENTANA = 1400;
const int ALTO_VENTANA = 900;
#define MAX_SCORES 10
#define MAX_VIDAS 5
#define PUNTAJE_VIDA_EXTRA 5000

// Estados del Juego
#define ESTADO_MENU         0
#define ESTADO_JUGANDO      1
#define ESTADO_PAUSA        2
#define ESTADO_GAMEOVER     3
#define ESTADO_INPUT_NOMBRE 4
#define ESTADO_MEJORES      5
#define ESTADO_CREDITOS     6
#define ESTADO_VICTORIA     7

// --- AJUSTES DE BALANCE FINAL ---
const float PADDLE_ANCHO = 200.0f;
const float PADDLE_ALTO = 30.0f;
const float VEL_BASE = 6.5f;
const float PELOTA_TAM = 26.0f;

// Ladrillos
#define FILAS 6       
#define COLUMNAS 10   
const float LADRILLO_ANCHO = 120.0f;
const float LADRILLO_ALTO = 40.0f;
const float LADRILLO_ESPACIO = 10.0f;
// Conversión explícita a float para evitar warnings C4244
const float LADRILLO_OFFSET_X = (1400.0f - (10.0f * (120.0f + 10.0f))) / 2.0f + 5.0f;
const float LADRILLO_OFFSET_Y = 150.0f;

// --- ESTRUCTURAS ---
typedef struct {
    char nombre[16];
    int puntaje;
} Jugador;

typedef struct {
    SDL_FRect rect;
    bool activo;
    int resistencia;
    int resistenciaMax;
    int filaIdx;
} Ladrillo;

// --- MAPAS (Nivel 1 ajustado a 3 filas) ---
const int PATRONES[10][FILAS][COLUMNAS] = {
    { {1,1,1,1,1,1,1,1,1,1}, {1,1,1,1,1,1,1,1,1,1}, {1,1,1,1,1,1,1,1,1,1}, {0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0} },
    { {1,0,1,0,1,0,1,0,1,0}, {0,1,0,1,0,1,0,1,0,1}, {1,0,1,0,1,0,1,0,1,0}, {0,1,0,1,0,1,0,1,0,1}, {1,0,1,0,1,0,1,0,1,0}, {0,1,0,1,0,1,0,1,0,1} },
    { {1,1,0,1,1,0,1,1,0,1}, {1,1,0,1,1,0,1,1,0,1}, {1,1,0,1,1,0,1,1,0,1}, {1,1,0,1,1,0,1,1,0,1}, {1,1,0,1,1,0,1,1,0,1}, {1,1,0,1,1,0,1,1,0,1} },
    { {1,1,1,1,1,1,1,1,1,1}, {0,1,1,1,1,1,1,1,1,0}, {0,0,1,1,1,1,1,1,0,0}, {0,0,0,1,1,1,1,0,0,0}, {0,0,0,0,1,1,0,0,0,0}, {0,0,0,0,0,0,0,0,0,0} },
    { {1,1,0,0,0,0,0,0,1,1}, {1,1,0,0,0,0,0,0,1,1}, {1,1,0,0,1,1,0,0,1,1}, {1,1,0,0,1,1,0,0,1,1}, {1,1,0,0,0,0,0,0,1,1}, {1,1,0,0,0,0,0,0,1,1} },
    { {1,0,0,0,0,0,0,0,0,1}, {0,1,0,0,0,0,0,0,1,0}, {0,0,1,0,0,0,0,1,0,0}, {0,0,0,1,1,1,1,0,0,0}, {0,0,1,0,0,0,0,1,0,0}, {1,0,0,0,0,0,0,0,0,1} },
    { {0,1,1,0,0,0,0,1,1,0}, {1,1,1,1,0,0,1,1,1,1}, {1,1,1,1,1,1,1,1,1,1}, {0,1,1,1,1,1,1,1,1,0}, {0,0,1,1,1,1,1,1,0,0}, {0,0,0,1,1,1,1,0,0,0} },
    { {1,1,1,1,1,1,1,1,1,1}, {1,0,0,0,0,0,0,0,0,1}, {1,0,1,1,1,1,1,1,0,1}, {1,0,1,0,0,0,0,1,0,1}, {1,0,0,0,0,0,0,0,0,1}, {1,1,1,1,1,1,1,1,1,1} },
    { {0,1,0,1,0,1,0,1,0,1}, {1,0,1,0,1,0,1,0,1,0}, {0,1,0,1,0,1,0,1,0,1}, {1,0,1,0,1,0,1,0,1,0}, {0,1,0,1,0,1,0,1,0,1}, {1,0,1,0,1,0,1,0,1,0} },
    { {1,0,0,1,0,0,1,0,0,1}, {0,0,0,0,0,0,0,0,0,0}, {0,1,1,0,0,0,0,1,1,0}, {0,0,0,0,0,0,0,0,0,0}, {1,0,0,0,0,0,0,0,0,1}, {0,1,1,1,1,1,1,1,1,0} }
};

// GLOBALES
Jugador mejoresPuntajes[MAX_SCORES];
char inputText[16] = "";
int nivelActual = 1;
int ladrillosRestantes = 0;
int proximaVida = PUNTAJE_VIDA_EXTRA;

// FUNCIONES
void CargarNivel(Ladrillo* ladrillos, int nivel) {
    if (nivel < 1) nivel = 1; if (nivel > 10) nivel = 1;
    int mapIdx = (nivel - 1) % 10;
    int count = 0;
    ladrillosRestantes = 0;

    for (int i = 0; i < FILAS; i++) {
        for (int j = 0; j < COLUMNAS; j++) {
            // Conversión explícita a float
            ladrillos[count].rect.x = LADRILLO_OFFSET_X + ((float)j * (LADRILLO_ANCHO + LADRILLO_ESPACIO));
            ladrillos[count].rect.y = LADRILLO_OFFSET_Y + ((float)i * (LADRILLO_ALTO + LADRILLO_ESPACIO));
            ladrillos[count].rect.w = LADRILLO_ANCHO;
            ladrillos[count].rect.h = LADRILLO_ALTO;
            ladrillos[count].filaIdx = i;

            if (PATRONES[mapIdx][i][j] == 1) {
                ladrillos[count].activo = true;
                ladrillosRestantes++;
                if (nivel >= 6) {
                    if (i == 0 || i == 1) ladrillos[count].resistencia = 3;
                    else if (i == 2 || i == 3) ladrillos[count].resistencia = 2;
                    else ladrillos[count].resistencia = 1;
                }
                else {
                    ladrillos[count].resistencia = 1;
                }
                ladrillos[count].resistenciaMax = ladrillos[count].resistencia;
            }
            else {
                ladrillos[count].activo = false;
                ladrillos[count].resistencia = 0;
            }
            count++;
        }
    }
}

void CargarPuntajes() {
    FILE* file = fopen("scores.dat", "rb");
    if (file) {
        fread(mejoresPuntajes, sizeof(Jugador), MAX_SCORES, file);
        fclose(file);
        for (int i = 0; i < MAX_SCORES; i++) mejoresPuntajes[i].nombre[15] = '\0';
    }
    else {
        for (int i = 0; i < MAX_SCORES; i++) { strcpy_s(mejoresPuntajes[i].nombre, 16, "-----"); mejoresPuntajes[i].puntaje = 0; }
    }
}

void GuardarPuntajes() {
    FILE* file = fopen("scores.dat", "wb");
    if (file) { fwrite(mejoresPuntajes, sizeof(Jugador), MAX_SCORES, file); fclose(file); }
}

void OrdenarPuntajesASM() {
    Jugador* pLista = mejoresPuntajes;
    int n = MAX_SCORES;
    __asm {
        mov esi, pLista
        mov ecx, n
        dec ecx
        LoopExterno :
        push ecx
            mov edi, esi
            mov ebx, n
            dec ebx
            LoopInterno :
        mov eax, [edi + 16]
            mov edx, [edi + 20 + 16]
            cmp eax, edx
            jge NoSwap
            mov[edi + 16], edx
            mov[edi + 20 + 16], eax
            mov eax, [edi]; mov edx, [edi + 20]; mov[edi], edx; mov[edi + 20], eax
            mov eax, [edi + 4]; mov edx, [edi + 24]; mov[edi + 4], edx; mov[edi + 24], eax
            mov eax, [edi + 8]; mov edx, [edi + 28]; mov[edi + 8], edx; mov[edi + 28], eax
            mov eax, [edi + 12]; mov edx, [edi + 32]; mov[edi + 12], edx; mov[edi + 32], eax
            NoSwap :
        add edi, 20
            dec ebx
            jnz LoopInterno
            pop ecx
            dec ecx
            jnz LoopExterno
    }
}

void DibujarTextoCentrado(SDL_Renderer* r, TTF_Font* f, const char* texto, int y, SDL_Color c) {
    if (!f || !texto || strlen(texto) == 0) return;
    SDL_Surface* surf = TTF_RenderText_Blended(f, texto, 0, c);
    if (surf) {
        SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
        SDL_FRect rect = { (ANCHO_VENTANA - (float)surf->w) / 2.0f, (float)y, (float)surf->w, (float)surf->h };
        SDL_RenderTexture(r, tex, NULL, &rect);
        SDL_DestroyTexture(tex);
        SDL_DestroySurface(surf);
    }
}

void DibujarCorazon(SDL_Renderer* renderer, float x, float y, float escala) {
    SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
    int forma[5][7] = { {0,1,1,0,1,1,0}, {1,1,1,1,1,1,1}, {1,1,1,1,1,1,1}, {0,1,1,1,1,1,0}, {0,0,1,1,1,0,0} };
    for (int fil = 0; fil < 5; fil++) {
        for (int col = 0; col < 7; col++) {
            if (forma[fil][col] == 1) {
                SDL_FRect pixel = { x + ((float)col * 5.0f * escala), y + ((float)fil * 5.0f * escala), 5.0f * escala, 5.0f * escala };
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    srand((unsigned int)time(NULL));

    if (!SDL_Init(SDL_INIT_VIDEO)) return 1;
    if (TTF_Init() < 0) return 1;

    CargarPuntajes();

    SDL_Window* ventana = NULL;
    SDL_Renderer* renderer = NULL;
    if (!SDL_CreateWindowAndRenderer("Breakout - Astrid Jimenez & Erick Moya", ANCHO_VENTANA, ALTO_VENTANA, 0, &ventana, &renderer)) return 1;

    // CARGA DE FUENTES
    TTF_Font* fontRetro = TTF_OpenFont("RETRO.TTF", 35);
    TTF_Font* fontTitulo = TTF_OpenFont("RETRO.TTF", 80);
    // USAMOS RETRO.TTF PARA CRÉDITOS COMO PEDISTE
    TTF_Font* fontCreditos = TTF_OpenFont("RETRO.TTF", 30);

    // Fallback a Arial si no existe RETRO
    if (!fontRetro) fontRetro = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 35);
    if (!fontTitulo) fontTitulo = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 80);
    if (!fontCreditos) fontCreditos = TTF_OpenFont("C:\\Windows\\Fonts\\arial.ttf", 30);

    int estado_actual = ESTADO_MENU;
    int corriendo = 1;
    int vidas = 3;
    int puntaje = 0;

    SDL_FRect paddle = { (ANCHO_VENTANA - PADDLE_ANCHO) / 2, ALTO_VENTANA - 60.0f, PADDLE_ANCHO, PADDLE_ALTO };
    SDL_FRect pelota = { ANCHO_VENTANA / 2, ALTO_VENTANA / 2, PELOTA_TAM, PELOTA_TAM };
    float vel_x = VEL_BASE;
    float vel_y = -VEL_BASE;

    Ladrillo ladrillos[FILAS * COLUMNAS];
    SDL_Color coloresFilas[6] = { {210,50,50,255}, {210,140,50,255}, {200,200,50,255}, {50,180,50,255}, {50,100,200,255}, {150,50,200,255} };

    nivelActual = 1;
    proximaVida = PUNTAJE_VIDA_EXTRA;
    CargarNivel(ladrillos, nivelActual);

    int input_enter = 0, input_esc = 0;
    SDL_Event evento;
    const bool* teclas = SDL_GetKeyboardState(NULL);
    SDL_Color colorBlanco = { 255, 255, 255, 255 };
    SDL_Color colorAmarillo = { 255, 255, 0, 255 };
    SDL_Color colorRosa = { 255, 100, 200, 255 };
    SDL_Color colorArduino = { 0, 151, 157, 255 };

    while (corriendo) {
        input_enter = 0; input_esc = 0;

        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_EVENT_QUIT) corriendo = 0;
            if (estado_actual == ESTADO_INPUT_NOMBRE && evento.type == SDL_EVENT_TEXT_INPUT) {
                if (strlen(inputText) < 15) strcat_s(inputText, 16, evento.text.text);
            }
            if (evento.type == SDL_EVENT_KEY_DOWN) {
                if (evento.key.key == SDLK_RETURN) input_enter = 1;
                if (evento.key.key == SDLK_ESCAPE) input_esc = 1;

                if (estado_actual == ESTADO_MENU) {
                    if (evento.key.key == SDLK_TAB) estado_actual = ESTADO_MEJORES;
                    // FIX: SDLK_C (Mayúscula) para SDL3
                    if (evento.key.key == SDLK_C) estado_actual = ESTADO_CREDITOS;
                }

                if (estado_actual == ESTADO_INPUT_NOMBRE && evento.key.key == SDLK_BACKSPACE) {
                    int len = (int)strlen(inputText);
                    if (len > 0) inputText[len - 1] = '\0';
                }
            }
        }

        // MAQUINA DE ESTADOS (ASM)
        __asm {
            mov eax, estado_actual
            cmp eax, ESTADO_MENU
            je LogicaMenu
            cmp eax, ESTADO_JUGANDO
            je LogicaJugando
            cmp eax, ESTADO_PAUSA
            je LogicaPausa
            cmp eax, ESTADO_GAMEOVER
            je LogicaGameOver
            cmp eax, ESTADO_INPUT_NOMBRE
            je LogicaInput
            cmp eax, ESTADO_MEJORES
            je LogicaMejores
            cmp eax, ESTADO_CREDITOS
            je LogicaCreditos
            cmp eax, ESTADO_VICTORIA
            je LogicaVictoria
            jmp FinLogica
            LogicaMenu :
            cmp input_esc, 1
                jne CheckMenuStart
                mov corriendo, 0
                jmp FinLogica
                CheckMenuStart :
            cmp input_enter, 1
                jne FinLogica
                mov estado_actual, ESTADO_JUGANDO
                jmp FinLogica
                LogicaJugando :
            cmp input_enter, 1
                jne CheckGameOver
                mov estado_actual, ESTADO_PAUSA
                jmp FinLogica
                CheckGameOver :
            cmp vidas, 0
                jg FinLogica
                mov estado_actual, ESTADO_GAMEOVER
                jmp FinLogica
                LogicaPausa :
            cmp input_enter, 1
                jne FinLogica
                mov estado_actual, ESTADO_JUGANDO
                jmp FinLogica
                LogicaGameOver :
            cmp input_enter, 1
                jne CheckExitGO
                mov estado_actual, ESTADO_INPUT_NOMBRE
                jmp FinLogica
                CheckExitGO :
            cmp input_esc, 1
                jne FinLogica
                mov estado_actual, ESTADO_MENU
                mov vidas, 3
                mov puntaje, 0
                jmp FinLogica
                LogicaInput :
            cmp input_enter, 1
                jne FinLogica
                mov estado_actual, ESTADO_MEJORES
                jmp FinLogica
                LogicaMejores :
            cmp input_esc, 1
                jne FinLogica
                mov estado_actual, ESTADO_MENU
                mov vidas, 3
                mov puntaje, 0
                jmp FinLogica
                LogicaCreditos :
            cmp input_esc, 1
                jne FinLogica
                mov estado_actual, ESTADO_MENU
                jmp FinLogica
                LogicaVictoria :
            cmp input_enter, 1
                jne CheckExitWin
                mov estado_actual, ESTADO_INPUT_NOMBRE
                jmp FinLogica
                CheckExitWin :
            cmp input_esc, 1
                jne FinLogica
                mov estado_actual, ESTADO_MENU
                mov vidas, 3
                mov puntaje, 0
                FinLogica :
        }

        if (estado_actual == ESTADO_MEJORES && input_enter) {
            SDL_StopTextInput(ventana);
            if (strlen(inputText) == 0) strcpy_s(inputText, 16, "ANONIMO");
            strcpy_s(mejoresPuntajes[MAX_SCORES - 1].nombre, 16, inputText);
            mejoresPuntajes[MAX_SCORES - 1].puntaje = puntaje;
            OrdenarPuntajesASM();
            GuardarPuntajes();
            strcpy_s(inputText, 16, "");
        }

        if (estado_actual == ESTADO_INPUT_NOMBRE && !SDL_TextInputActive(ventana)) SDL_StartTextInput(ventana);

        if (estado_actual == ESTADO_MENU && input_esc) {
            pelota.x = ANCHO_VENTANA / 2; pelota.y = ALTO_VENTANA / 2;
            vel_y = -VEL_BASE;
            vel_x = (rand() % 2 == 0) ? VEL_BASE : -VEL_BASE;
            nivelActual = 1;
            proximaVida = PUNTAJE_VIDA_EXTRA;
            CargarNivel(ladrillos, nivelActual);
        }

        if (estado_actual == ESTADO_JUGANDO) {
            float temp_px = paddle.x;
            float temp_pv = 12.0f;
            int dir_paddle = 0;
            if (teclas[SDL_SCANCODE_RIGHT]) dir_paddle = 1;
            if (teclas[SDL_SCANCODE_LEFT])  dir_paddle = -1;

            if (dir_paddle != 0) {
                __asm {
                    fld temp_px
                    cmp dir_paddle, 1
                    je MoverDer
                    fsub temp_pv
                    jmp FinPaddle
                    MoverDer : fadd temp_pv
                    FinPaddle : fstp temp_px
                }
                paddle.x = temp_px;
            }
            if (paddle.x < 0) paddle.x = 0;
            if (paddle.x + paddle.w > ANCHO_VENTANA) paddle.x = ANCHO_VENTANA - paddle.w;

            __asm {
                fld pelota.x
                fadd vel_x
                fstp pelota.x
                fld pelota.y
                fadd vel_y
                fstp pelota.y
            }

            float ball_r = pelota.x + PELOTA_TAM; float ball_b = pelota.y + PELOTA_TAM;
            float pad_r = paddle.x + PADDLE_ANCHO; float pad_b = paddle.y + PADDLE_ALTO;

            // FISICA CON PERTURBACION ALEATORIA (ASM)
            if (ball_r >= paddle.x && pelota.x <= pad_r && ball_b >= paddle.y && pelota.y <= pad_b) {
                float perturbacion = ((float)(rand() % 300) / 100.0f) - 1.5f;

                __asm {
                    fld vel_y
                    fabs
                    fchs
                    fstp vel_y

                    fld vel_x
                    fadd perturbacion
                    fstp vel_x
                }
                if (fabs(vel_x) < 2.0f) vel_x = (vel_x >= 0) ? 2.0f : -2.0f;
                pelota.y = paddle.y - PELOTA_TAM - 2.0f;
            }

            for (int i = 0; i < FILAS * COLUMNAS; i++) {
                if (!ladrillos[i].activo) continue;
                SDL_FRect b = ladrillos[i].rect;
                if (ball_r >= b.x && pelota.x <= b.x + b.w && ball_b >= b.y && pelota.y <= b.y + b.h) {
                    ladrillos[i].resistencia--;
                    vel_y = -vel_y;
                    if (ladrillos[i].resistencia <= 0) {
                        ladrillos[i].activo = false;
                        puntaje += 100;
                        ladrillosRestantes--;

                        __asm {
                            mov eax, puntaje
                            cmp eax, proximaVida
                            jl NoVidaExtra
                            mov ebx, vidas
                            inc ebx
                            cmp ebx, MAX_VIDAS
                            jg TopeVidas
                            mov vidas, ebx
                            jmp ActMeta
                            TopeVidas :
                            mov vidas, MAX_VIDAS
                                ActMeta :
                            add proximaVida, PUNTAJE_VIDA_EXTRA
                                NoVidaExtra :
                        }

                        if (ladrillosRestantes <= 0) {
                            nivelActual++;
                            if (nivelActual > 10) {
                                // --- VICTORIA ---
                                estado_actual = ESTADO_VICTORIA;
                            }
                            else {
                                // SIGUIENTE NIVEL
                                CargarNivel(ladrillos, nivelActual);
                                pelota.x = ANCHO_VENTANA / 2; pelota.y = ALTO_VENTANA / 2;
                                float factor = 1.0f + ((float)nivelActual * 0.15f);
                                float nuevaVel = VEL_BASE * factor;
                                vel_x = (rand() % 2 == 0) ? nuevaVel : -nuevaVel;
                                vel_y = -nuevaVel;
                                SDL_Delay(500);
                            }
                        }
                    }
                }
            }

            if (pelota.x <= 0 || pelota.x + PELOTA_TAM >= ANCHO_VENTANA) vel_x = -vel_x;
            if (pelota.y <= 0) vel_y = -vel_y;
            if (pelota.y > ALTO_VENTANA) {
                vidas--;
                pelota.x = ANCHO_VENTANA / 2; pelota.y = ALTO_VENTANA / 2;
                float velActual = VEL_BASE * (1.0f + ((float)nivelActual * 0.1f));
                vel_y = -velActual;
                vel_x = (rand() % 2 == 0) ? velActual : -velActual;
                SDL_Delay(500);
            }
        }

        // RENDER
        SDL_SetRenderDrawColor(renderer, 15, 15, 25, 255);
        SDL_RenderClear(renderer);

        if (estado_actual == ESTADO_JUGANDO || estado_actual == ESTADO_PAUSA || estado_actual == ESTADO_GAMEOVER || estado_actual == ESTADO_VICTORIA) {
            int idx = 0;
            for (int i = 0; i < FILAS; i++) {
                SDL_Color cFila = coloresFilas[i];
                for (int j = 0; j < COLUMNAS; j++) {
                    if (ladrillos[idx].activo) {
                        SDL_Color c = cFila;
                        if (ladrillos[idx].resistenciaMax > 1 && ladrillos[idx].resistencia == 1) {
                            c.r = 255; c.g = 255; c.b = 255;
                        }
                        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
                        SDL_RenderFillRect(renderer, &ladrillos[idx].rect);
                    }
                    idx++;
                }
            }
            SDL_SetRenderDrawColor(renderer, 200, 200, 255, 255);
            SDL_RenderFillRect(renderer, &paddle);
            SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
            SDL_RenderFillRect(renderer, &pelota);

            char buf[50]; sprintf_s(buf, 50, "SCORE: %05d", puntaje);
            DibujarTextoCentrado(renderer, fontRetro, buf, 20, colorBlanco);
            char bufNivel[20]; sprintf_s(bufNivel, 20, "NIVEL: %d", nivelActual);
            DibujarTextoCentrado(renderer, fontRetro, bufNivel, 60, colorAmarillo);

            for (int i = 0; i < vidas; i++) DibujarCorazon(renderer, ANCHO_VENTANA - 60.0f - ((float)i * 50.0f), 25.0f, 1.5f);
        }

        if (estado_actual == ESTADO_MENU) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            DibujarTextoCentrado(renderer, fontTitulo, "BREAKOUT", 150, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "JUGAR (ENTER)", 400, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "MEJORES PUNTUACIONES (TAB)", 500, colorAmarillo);
            DibujarTextoCentrado(renderer, fontRetro, "CREDITOS (C)", 600, colorAmarillo);
            DibujarTextoCentrado(renderer, fontRetro, "SALIR (ESC)", 700, colorBlanco);
        }
        else if (estado_actual == ESTADO_PAUSA) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
            SDL_RenderFillRect(renderer, NULL);
            DibujarTextoCentrado(renderer, fontTitulo, "PAUSA", 300, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "ENTER para volver", 500, colorBlanco);
        }
        else if (estado_actual == ESTADO_GAMEOVER) {
            SDL_SetRenderDrawColor(renderer, 50, 0, 0, 200);
            SDL_RenderFillRect(renderer, NULL);
            DibujarTextoCentrado(renderer, fontTitulo, "GAME OVER", 300, colorBlanco);
            char buf[50]; sprintf_s(buf, 50, "Puntaje Final: %05d", puntaje);
            DibujarTextoCentrado(renderer, fontRetro, buf, 450, colorBlanco);
            DibujarTextoCentrado(renderer, fontRetro, "ENTER para Guardar", 600, colorAmarillo);
        }
        else if (estado_actual == ESTADO_VICTORIA) {
            SDL_SetRenderDrawColor(renderer, 0, 50, 0, 200);
            SDL_RenderFillRect(renderer, NULL);

            DibujarTextoCentrado(renderer, fontTitulo, "¡ VICTORIA !", 250, colorAmarillo);
            DibujarTextoCentrado(renderer, fontRetro, "¡Has completado los 10 niveles!", 400, colorBlanco);

            char buf[50]; sprintf_s(buf, 50, "Puntaje Final: %05d", puntaje);
            DibujarTextoCentrado(renderer, fontRetro, buf, 500, colorBlanco);

            DibujarTextoCentrado(renderer, fontRetro, "Presiona ENTER para Registrar Record", 650, colorArduino);
        }
        else if (estado_actual == ESTADO_INPUT_NOMBRE) {
            SDL_SetRenderDrawColor(renderer, 20, 20, 60, 255);
            SDL_RenderClear(renderer);
            DibujarTextoCentrado(renderer, fontTitulo, "NUEVO RECORD!", 150, colorAmarillo);
            DibujarTextoCentrado(renderer, fontRetro, "Escribe tu nombre y da ENTER:", 350, colorBlanco);
            SDL_FRect linea = { ANCHO_VENTANA / 2 - 200, 500, 400, 4 };
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &linea);
            if (strlen(inputText) > 0) DibujarTextoCentrado(renderer, fontTitulo, inputText, 420, colorBlanco);
            else DibujarTextoCentrado(renderer, fontRetro, "_", 420, colorBlanco);
        }
        else if (estado_actual == ESTADO_MEJORES) {
            SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
            SDL_RenderClear(renderer);
            DibujarTextoCentrado(renderer, fontTitulo, "HALL OF FAME", 50, colorAmarillo);
            for (int i = 0; i < MAX_SCORES; i++) {
                char lineaScore[60];
                sprintf_s(lineaScore, 60, "%d. %s   -   %05d", i + 1, mejoresPuntajes[i].nombre, mejoresPuntajes[i].puntaje);
                SDL_Color c = (i == 0) ? colorAmarillo : colorBlanco;
                DibujarTextoCentrado(renderer, fontRetro, lineaScore, 200 + (i * 55), c);
            }
            DibujarTextoCentrado(renderer, fontRetro, "VOLVER (ESC)", 800, colorBlanco);
        }
        else if (estado_actual == ESTADO_CREDITOS) {
            SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);
            SDL_RenderClear(renderer);
            DibujarTextoCentrado(renderer, fontTitulo, "CREDITOS", 50, colorAmarillo);

            // Usando fuente Retro con caracteres Unicode
            // ♥ = \xE2\x99\xA5  (Corazón)
            // ★ = \xE2\x98\x85  (Estrella Sólida)
            DibujarTextoCentrado(renderer, fontCreditos, "1. \xE2\x99\xA5 Astrid Yamilet Jimenez Barrera \xE2\x99\xA5", 350, colorRosa);
            DibujarTextoCentrado(renderer, fontCreditos, "2. \xE2\x98\x85 Erick Anselmo Moya Monreal \xE2\x98\x85", 450, colorArduino);

            DibujarTextoCentrado(renderer, fontRetro, "VOLVER (ESC)", 800, colorBlanco);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    if (fontRetro) TTF_CloseFont(fontRetro);
    if (fontTitulo) TTF_CloseFont(fontTitulo);
    if (fontCreditos) TTF_CloseFont(fontCreditos);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}
