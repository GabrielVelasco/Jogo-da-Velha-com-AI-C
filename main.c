//Initially it was a Pac Man project in C, then it turned into a Tetris Attack in C, now this
//is a Hash game in C with a AI made of IF ELSE IF ELSE... which is not so intelligent.

#include "SOIL.h"
#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <time.h>
#include "pacman.h"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

// Global variables
Pacman *pac;
Cenario *cen;
int iniciou_jogo = 0;
int randon;
int vs_IA = 0;
char vez = 'x';
char g_winner = 'v';

void desenhaJogo();
void iniciaJogo();
void terminaJogo();

//OpenGL function
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;


    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          800,
                          800,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);
    // Initialize
    iniciaJogo();

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glPushMatrix();
            // This controls every frame of the game
            // Draw the game here
            desenhaJogo();

            glPopMatrix();

            SwapBuffers(hDC);

            Sleep(50);
        }
    }
    // It's not drawing anymore
    // so, game just ended.
    terminaJogo();

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}
// Windows callback function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE: //Pressed ESC?
                    PostQuitMessage(0);
                    terminaJogo();
                    break;
                case VK_RIGHT: //Right ?
                    comanda_Cursor(pac,0,cen,&vez,&vs_IA);
                    break;
                case VK_DOWN: //Down ?
                    comanda_Cursor(pac,1,cen,&vez,&vs_IA);
                    break;
                case VK_LEFT: //Left ?
                    comanda_Cursor(pac,2,cen,&vez,&vs_IA);
                    break;
                case VK_UP: //Up ?
                    comanda_Cursor(pac,3,cen,&vez,&vs_IA);
                    break;
                case VK_SPACE: //Space ? so put a 'x' or 'x'..
                    comanda_Cursor(pac,4,cen,&vez,&vs_IA);
                    break;

//             1v1
                case '2':
                    if(g_winner != 'v'){
                        terminaJogo();
                        iniciaJogo();
                    }
                    iniciou_jogo = 1;
                    g_winner = 'v';
                    srand(time(NULL));
                    randon = 1 + rand() % 2;
                    if(randon == 1)
                        vez = 'x';
                    if(randon == 2)
                        vez = 'o';
                    vs_IA = 0;

                    break;

//              1vAI
                case '1':
                    if(g_winner != 'v'){
                        terminaJogo();
                        iniciaJogo();
                    }
                    g_winner = 'v';
                    iniciou_jogo = 1;
                    vs_IA = 1; //Against AI
                    srand(time(NULL));
                    randon = rand()%2 + 1;
                    if(randon == 1){
                        vez = 'x'; //'Player' start playing
                        break;
                    }else{
                        vez = 'o'; //AI start playing
                        first_play(cen, &vez);
                    }

                    break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}
// OpenGL
void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);

    // Load all the textures that's going to be used.
    carregaTexturas();
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering


}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

// ===================================================================
// Controls and draw the game
// ===================================================================

// Draw each component of the game.
int contt = 0;
void desenhaJogo(){
    cenario_desenha(cen);
    if(iniciou_jogo == 0){
        desenhaTela(0); //Start screen
        return;
    }
    checa(cen, pac, &g_winner);    //Checks if somebody won
    if(g_winner == 'x'){
        desenhaTela(3);
        return;
    }
    if(g_winner == 'o'){
        desenhaTela(4);
        return;
    }
    if(g_winner == -1){
        desenhaTela(-1);
        return;
    }
    desenha_ponto(&vez);
    if(pacman_vivo(pac)){
        pacman_desenha(pac);
    }else{
        iniciou_jogo = 2;
    }
}

// Load the board
void iniciaJogo(){
    srand(time(0));
    int i;
    srand(time(NULL));
    cen = cenario_carrega();
    pac = pacman_create(3,4);
    iniciou_jogo = 0;
}

// Game finished, free all datas.
void terminaJogo(){
    pacman_destroy(pac);
    cenario_destroy(cen);
}
