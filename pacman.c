#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <windows.h>
#include <gl/gl.h>
#include "SOIL.h"
#include "pacman.h"

//=========================================================
// Size of each block
#define bloco 70
// Size of the matrix (board) 3 by 3
#define N 3
#define P 3
// Size of each block of the matrix on the screen
#define TAM 0.1f
#define TAMBG 1.6f
// Fun��es que convertem a linha e coluna da matriz em uma coordenada de [-1,1] //não vou "traduzir" kkkk
#define MAT2X(j) ((j)*0.1f-1)
#define MAT2Y(i) (0.9-(i)*0.1f)

//=========================================================
// Structures that control the game
struct TPoint{
    int x,y;
};

const struct TPoint direcoes[4] = {{1,0},{0,1},{-1,0},{0,-1}};

struct TPacman{
    int status;
    int xi,yi,x,y;
    int direcao,parcial;
    int pontos;
    int invencivel;
    int vivo;
    int animacao;
};

struct TCenario{
    int mapa[N][P];
    int vivo;
};

//==============================================================
// Textures
//==============================================================

GLuint pecas[4];
GLuint grade[3];
GLuint telaStart, Xwin, Owin, vez_x, vez_o, Velha;

static void desenhaSprite(float coluna,float linha, GLuint tex);
static GLuint carregaArqTextura(char *str);
void pacman_destroy(Pacman *pac);

void carregaTexturas(){
    int i;
    char str[50];

    for(i=1; i<=2; i++){
        sprintf(str,".//Sprites//grade%d.png", i);
        grade[i] = carregaArqTextura(str);
    }

    pecas[1] = carregaArqTextura(".//Sprites//peca1.png");
    pecas[2] = carregaArqTextura(".//Sprites//peca2.png");
    pecas[3] = carregaArqTextura(".//Sprites//peca3.png");

    vez_x = carregaArqTextura(".//Sprites//xtime.png");
    vez_o = carregaArqTextura(".//Sprites//otime.png");
    telaStart = carregaArqTextura(".//Sprites//start.png");
    Xwin = carregaArqTextura(".//Sprites//xwin.png");
    Owin = carregaArqTextura(".//Sprites//owin.png");
    Velha = carregaArqTextura(".//Sprites//velha.png");

}

static GLuint carregaArqTextura(char *str){
    // http://www.lonesock.net/soil.html
    GLuint tex = SOIL_load_OGL_texture
        (
            str,
            SOIL_LOAD_AUTO,
            SOIL_CREATE_NEW_ID,
            SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y |
            SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
        );

    /* check for an error during the load process */
    if(0 == tex){
        printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
    }

    return tex;
}

// Fun��o que recebe uma linha e coluna da matriz e um c�digo
// de textura e desenha um quadrado na tela com essa textura
void desenhaSprite(float coluna,float linha, GLuint tex){
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f); glVertex2f(coluna, linha);
        glTexCoord2f(1.0f,0.0f); glVertex2f(coluna+TAM, linha);
        glTexCoord2f(1.0f,1.0f); glVertex2f(coluna+TAM, linha+TAM);
        glTexCoord2f(0.0f,1.0f); glVertex2f(coluna, linha+TAM);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

}
void desenhaBG(float coluna,float linha, GLuint tex){
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f); glVertex2f(coluna, linha);
        glTexCoord2f(1.0f,0.0f); glVertex2f(coluna+TAMBG, linha);
        glTexCoord2f(1.0f,1.0f); glVertex2f(coluna+TAMBG, linha+TAMBG);
        glTexCoord2f(0.0f,1.0f); glVertex2f(coluna, linha+TAMBG);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

}

void desenhaTipoTela(float x, float y, float tamanho, GLuint tex){

    glPushMatrix();

    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f,1.0f); glVertex2f(x - tamanho, y + tamanho);
        glTexCoord2f(1.0f,1.0f); glVertex2f(x + tamanho, y + tamanho);
        glTexCoord2f(1.0f,0.0f); glVertex2f(x + tamanho, y - tamanho);
        glTexCoord2f(0.0f,0.0f); glVertex2f(x - tamanho, y - tamanho);
    glEnd();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

void desenhaTela(int tipo){
    if(tipo == 0)
        desenhaTipoTela(0, 0, 1.0, telaStart);
    if(tipo == 3)
        desenhaTipoTela(0, 0, 1.0, Xwin);
    if(tipo == 4)
        desenhaTipoTela(0, 0, 1.0, Owin);
    if(tipo == -1)
        desenhaTipoTela(0, 0, 1.0, Velha);
}
//==============================================================
// Cenario
//==============================================================

// Fun��o que carrega os dados do cen�rio de um arquivo texto
Cenario* cenario_carrega(){
    Cenario* cen = malloc(sizeof(Cenario));
    int i,j;
    for(i=0; 3>i; i++){
        for(j = 0; 3>j; j++){
            cen->mapa[i][j] = 0;
        }
    }
    cen->vivo = 1;
    return cen;
}

// Libera os dados associados ao cen�rio
void cenario_destroy(Cenario* cen){
    free(cen);
}

// Percorre a matriz do jogo desenhando os sprites
void cenario_desenha(Cenario *cen){
    int i,j;
    for(i=0; i<N; i++){
        for(j=0; j<P; j++){
            if(cen->mapa[i][j] == 0)
                desenhaSprite(MAT2X(j+3),MAT2Y(i+4),pecas[1]);

            if(cen->mapa[i][j] == 'x')
                desenhaSprite(MAT2X(j+3),MAT2Y(i+4),pecas[2]);

            if(cen->mapa[i][j] == 'o')
                desenhaSprite(MAT2X(j+3),MAT2Y(i+4),pecas[3]);
        }
    }
}

void troca(Cenario *cen, int m, int n, char letra){
      if(letra == 'x')
            cen->mapa[m][n] = 'x';
      if(letra == 'o')
            cen->mapa[m][n] = 'o';
}

void checa(Cenario *cen, Pacman *pac, char *w){
    int i, j, cont = 1, cont2 = 0;
    //verifica linha
        for(j=0, i=0; i<P; i++){
            cont = 1;
            while( cen->mapa[i][j] == cen->mapa[i][j+cont] ){
                cont++;
                if(cont == 3){
                    *w = cen->mapa[i][j];
                    return;
                }
            }
        }

    //verifica coluna
        for(j=0, i=0; j<P; j++){
            cont = 1;
            while( cen->mapa[i][j] == cen->mapa[i+cont][j] ){
                cont++;
                if(cont == 3){
                    *w = cen->mapa[i][j];
                    return;
                }
            }
        }

    //verifica diagonal principal
    cont = 1;
    while( cen->mapa[0][0] == cen->mapa[cont][cont] ){
        cont++;
        if(cont == 3){
            *w = cen->mapa[i][j];
            return;
        }
    }

    //verifica diagonal secundaria
    cont = 1;
	while( cen->mapa[0][2] == cen->mapa[cont][2-cont] ){
	    cont++;
	    if(cont == 3){
	        *w = cen->mapa[i][j];
	        return;
	    }
	}

    //Verifica se deu velha
    cont2 = 0;
    for(i = 0; i < 3; i++)
        for(j = 0; j < 3; j++)
            if(cen->mapa[i][j] != 0)
                cont2 ++;

    if(cont2 == 9){
        *w = -1;
        return;
    }

}

void first_play(Cenario *cen, char *vez){
    srand(time(NULL));
    int r_line, r_column;
    do{
        r_line = rand()%3;
        r_column = rand()%3;
    }while( (r_line == 1 || r_column == 1) );
    cen->mapa[r_line][r_column] = 'o';

    *vez = 'x';
}

void first_play2(Cenario *cen){
    srand(time(NULL));
    int r_line, r_column;
    do{
        r_line = rand()%3;
        r_column = rand()%3;
    }while( (r_line == 1 || r_column == 1) );
    cen->mapa[r_line][r_column] = 'o';

}

void play_ia(Cenario *cen){
    int som = 0, cont = 0, pos_i, pos_j;

    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            som += cen->mapa[i][j];

    if(som == 120 && cen->mapa[1][1] == 0){
        cen->mapa[1][1] = 'o';
        return;
    }else if(som == 120 && cen->mapa[1][1] == 'x'){
        first_play2(cen);
        return;
    }

    //Verifica linhas, se bot pode ganhar em uma jogada
    for(int i = 0; i < 3; i++){
            if( (cen->mapa[i][0] == 0) && (cen->mapa[i][1] == 'o' && cen->mapa[i][2] == 'o') ){
                cen->mapa[i][0] = 'o';
                return;
            }
            if( (cen->mapa[i][1] == 0) && (cen->mapa[i][0] == 'o' && cen->mapa[i][2] == 'o') ){
                cen->mapa[i][1] = 'o';
                return;
            }
            if( (cen->mapa[i][2] == 0) && (cen->mapa[i][0] == 'o' && cen->mapa[i][1] == 'o') ){
                cen->mapa[i][2] = 'o';
                return;
            }
    }

    //Verifica colunas, se bot pode ganhar em uma jogada
    for(int j = 0; j < 3; j++){
            if( (cen->mapa[0][j] == 0) && (cen->mapa[1][j] == 'o' && cen->mapa[2][j] == 'o') ){
                cen->mapa[0][j] = 'o';
                return;
            }
            if( (cen->mapa[1][j] == 0) && (cen->mapa[0][j] == 'o' && cen->mapa[2][j] == 'o') ){
                cen->mapa[1][j] = 'o';
                return;
            }
            if( (cen->mapa[2][j] == 0) && (cen->mapa[1][j] == 'o' && cen->mapa[0][j] == 'o') ){
                cen->mapa[2][j] = 'o';
                return;
            }
    }

    //Verifica diagonal princ, se o bot pode ganhar em uma jogada
    if( (cen->mapa[0][0] == 0) && (cen->mapa[1][1] == 'o' && cen->mapa[2][2] == 'o' ) ){
        cen->mapa[0][0] = 'o';
        return;
    }
    if( (cen->mapa[1][1] == 0) && (cen->mapa[0][0] == 'o' && cen->mapa[2][2] == 'o' ) ){
        cen->mapa[1][1] = 'o';
        return;
    }
    if( (cen->mapa[2][2] == 0) && (cen->mapa[0][0] == 'o' && cen->mapa[1][1] == 'o' ) ){
        cen->mapa[2][2] = 'o';
        return;
    }

    //Verifica diagonal sec, se bot pode ganhar em uma jogada
    if( (cen->mapa[0][2] == 0) && (cen->mapa[1][1] == 'o' && cen->mapa[2][0] == 'o') ){
        cen->mapa[0][2] = 'o';
        return;
    }
    if( (cen->mapa[1][1] == 0) && (cen->mapa[0][2] == 'o' && cen->mapa[2][0] == 'o') ){
        cen->mapa[1][1] = 'o';
        return;
    }
    if( (cen->mapa[2][0] == 0) && (cen->mapa[1][1] == 'o' && cen->mapa[0][2] == 'o') ){
        cen->mapa[2][0] = 'o';
        return;
    }

//======================================================================================//

    //Verifica linhas, se bot pode perder em uma jogada
    for(int i = 0; i < 3; i++){
            if( (cen->mapa[i][0] == 0) && (cen->mapa[i][1] == 'x' && cen->mapa[i][2] == 'x') ){
                cen->mapa[i][0] = 'o';
                return;
            }
            if( (cen->mapa[i][1] == 0) && (cen->mapa[i][0] == 'x' && cen->mapa[i][2] == 'x') ){
                cen->mapa[i][1] = 'o';
                return;
            }
            if( (cen->mapa[i][2] == 0) && (cen->mapa[i][0] == 'x' && cen->mapa[i][1] == 'x') ){
                cen->mapa[i][2] = 'o';
                return;
            }
    }

    //Verifica colunas, se bot pode perder em uma jogada
    for(int j = 0; j < 3; j++){
            if( (cen->mapa[0][j] == 0) && (cen->mapa[1][j] == 'x' && cen->mapa[2][j] == 'x') ){
                cen->mapa[0][j] = 'o';
                return;
            }
            if( (cen->mapa[1][j] == 0) && (cen->mapa[0][j] == 'x' && cen->mapa[2][j] == 'x') ){
                cen->mapa[1][j] = 'o';
                return;
            }
            if( (cen->mapa[2][j] == 0) && (cen->mapa[1][j] == 'x' && cen->mapa[0][j] == 'x') ){
                cen->mapa[2][j] = 'o';
                return;
            }
    }

    //Verif diagonal princ se bot pode perder em uma jogada
    if( (cen->mapa[0][0] == 0) && (cen->mapa[1][1] == 'x' && cen->mapa[2][2] == 'x' ) ){
        cen->mapa[0][0] = 'o';
        return;
    }
    if( (cen->mapa[1][1] == 0) && (cen->mapa[0][0] == 'x' && cen->mapa[2][2] == 'x' ) ){
        cen->mapa[1][1] = 'o';
        return;
    }
    if( (cen->mapa[2][2] == 0) && (cen->mapa[0][0] == 'x' && cen->mapa[1][1] == 'x' ) ){
        cen->mapa[2][2] = 'o';
        return;
    }

    //Verifica diagonal sec, se bot pode perder em uma jogada
    if( (cen->mapa[0][2] == 0) && (cen->mapa[1][1] == 'x' && cen->mapa[2][0] == 'x') ){
        cen->mapa[0][2] = 'o';
        return;
    }
    if( (cen->mapa[1][1] == 0) && (cen->mapa[0][2] == 'x' && cen->mapa[2][0] == 'x') ){
        cen->mapa[1][1] = 'o';
        return;
    }
    if( (cen->mapa[2][0] == 0) && (cen->mapa[1][1] == 'x' && cen->mapa[0][2] == 'x') ){
        cen->mapa[2][0] = 'o';
        return;
    }

//======================================================================================//

    //Prioridade, buscar linha que tenho 1 peça 'o' e nenhuma 'x
    int rand_pos;
    for(int i = 0; i < 3; i++){
        if( (cen->mapa[i][0] == 'o') && (cen->mapa[i][1] == 0 && cen->mapa[i][2] == 0) ){
            cen->mapa[i][2] = 'o';
            return;
        }
        if( (cen->mapa[i][1] == 'o') && (cen->mapa[i][0] == 0 && cen->mapa[i][2] == 0) ){
            srand(time(NULL));
            do{
                rand_pos = rand()%3;
            }while( rand_pos == 1 );
            cen->mapa[i][rand_pos] = 'o';
            return;
        }
        if( (cen->mapa[i][2] == 'o') && (cen->mapa[i][1] == 0 && cen->mapa[i][0] == 0) ){
            srand(time(NULL));
            rand_pos = rand()%9;
            if(rand_pos <= 3)
                cen->mapa[i][1] = 'o';
            else
                cen->mapa[i][0] = 'o';
           return;
        }
    }

    //Prioridade, buscar coluna que tenha apenas uma peca 'o' e nehuma 'x'
    for(int j = 0; j < 3; j++){
        if( (cen->mapa[0][j] == 'o') && (cen->mapa[1][j] == 0 && cen->mapa[2][j] == 0) ){
            cen->mapa[2][j] = 'o';
            return;
        }
        if( (cen->mapa[1][j] == 'o') && (cen->mapa[0][j] == 0 && cen->mapa[2][j] == 0) ){
            srand(time(NULL));
            do{
                rand_pos = rand()%3;
            }while( rand_pos == 1 );
            cen->mapa[rand_pos][j] = 'o';
            return;
        }
        if( (cen->mapa[2][j] == 'o') && (cen->mapa[1][j] == 0 && cen->mapa[0][j] == 0) ){
            srand(time(NULL));
            rand_pos = rand()%9;
            if(rand_pos <= 3)
                cen->mapa[1][j] = 'o';
            else
                cen->mapa[0][j] = 'o';
            return;
        }
    }

    //Prioridade, buscar diagonal princ que tenha apenas uma peca 'o' e nehuma 'x'
    if( (cen->mapa[0][0] == 'o') && (cen->mapa[1][1] == 0 && cen->mapa[2][2] == 0) ){
        cen->mapa[2][2] = 'o';
        return;
    }
    if( (cen->mapa[1][1] == 'o') && (cen->mapa[0][0] == 0 && cen->mapa[2][2] == 0) ){
        srand(time(NULL));
        do{
            rand_pos = rand()%3;
        }while( rand_pos == 1 );
        cen->mapa[rand_pos][rand_pos] = 'o';
        return;
    }
    if( (cen->mapa[2][2] == 'o') && (cen->mapa[1][1] == 0 && cen->mapa[0][0] == 0) ){
        cen->mapa[0][0] = 'o';
        return;
    }

    //Prioridade, buscar diagonal sec que tenha apenas uma peca 'o' e nehuma 'x'
    if( (cen->mapa[0][2] == 'o') && (cen->mapa[1][1] == 0 && cen->mapa[2][0] == 0) ){
        cen->mapa[2][0] = 'o';
        return;
    }
    if( (cen->mapa[1][1] == 'o') && (cen->mapa[0][2] == 0 && cen->mapa[2][0] == 0) ){
        srand(time(NULL));
        do{
            rand_pos = rand()%3;
        }while( rand_pos == 1 );
        if(rand_pos == 0)
            cen->mapa[0][2] = 'o';
        if(rand_pos == 2)
            cen->mapa[2][0] = 'o';
        return;
    }
    if( (cen->mapa[2][0] == 'o') && (cen->mapa[1][1] == 0 && cen->mapa[0][2] == 0) ){
        cen->mapa[0][2] = 'o';
        return;
    }

    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            if(cen->mapa[i][j] == 0){
                cont ++;
                pos_i = i;
                pos_j = j;
            }
            if( (i == 2 && j == 2) && cont == 1 ){
                cen->mapa[pos_i][pos_j] = 'o';
            }
            if( (i == 2 && j == 2) && cont == 2 ){
                cen->mapa[pos_i][pos_j] = 'o';
            }
        }
    }

}

//==============================================================
// Pacman
//==============================================================


// Fun��o que inicializa os dados associados ao pacman
Pacman* pacman_create(int x, int y){
    Pacman* pac = malloc(sizeof(Pacman));
    if(pac != NULL){
        pac->invencivel = 0;
        pac->pontos = 0;
        pac->vivo = 1;
        pac->status = 0;
        pac->direcao = 0;
        pac->parcial = 0;
        pac->xi = x;
        pac->yi = y;
        pac->x = x;
        pac->y = y;
    }
    return pac;
}

// Fun��o que libera os dados associados ao pacman
void pacman_destroy(Pacman *pac){
    free(pac);
}

// Fun��o que verifica se o pacman est� vivo ou n�o
int pacman_vivo(Pacman *pac){
    if(pac->vivo)
        return 1;
    else{
        if(pac->animacao > 60)
            return 0;
        else
            return 1;
    }
}

// Fun��o que comanda o cursor para outra dire��o
void comanda_Cursor(Pacman *pac, int direcao, Cenario *cen, char *v, int *vs_IA){
    int a = 1;
        if(direcao == 0 && pac->x < 5){
            pac->x ++;
        }
        if(direcao == 1 && pac->y < 6){
            pac->y ++;
        }
        if(direcao == 2 && pac->x > 3){
            pac->x --;
        }
        if(direcao == 3 && pac->y > 4){
            pac->y --;
        }
        //troca pe�as de lugar
        //Pressionou space, est� jogando 1v1
        if(direcao == 4 && *vs_IA == 0){
            if(*v == 'x'){
                if( cen->mapa[pac->y-4][pac->x-3] == 'x' || cen->mapa[pac->y-4][pac->x-3] == 'o' ){
                    printf("\nPosicao invalida!\n");
                    return;
                }
                troca(cen, pac->y-4, pac->x-3, 'x');
                *v = 'o';

            }else if(*v == 'o'){
                    if( cen->mapa[pac->y-4][pac->x-3] == 'x' || cen->mapa[pac->y-4][pac->x-3] == 'o' ){
                        printf("\nPosicao invalida!\n");
                        return;
                    }
                    troca(cen, pac->y-4, pac->x-3, 'o');
                    *v = 'x';
                }
        }

        //Pressionou space, esta jogando 1vIA
        if(direcao == 4 && *vs_IA == 1){
            if(*v == 'x'){
                if( cen->mapa[pac->y-4][pac->x-3] == 'x' || cen->mapa[pac->y-4][pac->x-3] == 'o' ){
                    printf("\nPosicao invalida!\n");
                    return;
                }
                troca(cen, pac->y-4, pac->x-3, 'x');
                *v = 'o';
                play_ia(cen);
                *v = 'x';
            }
        }

}

// Fun��o que desenha o pacman
void pacman_desenha(Pacman *pac){
    float linha = pac->y, coluna = pac->x;

    if(pac->vivo){

        // Escolhe se desenha com boca aberta ou fechada
        if(pac->status < 10){
            desenhaSprite(MAT2X(coluna),MAT2Y(linha), grade[1]);
        }
        else{
            desenhaSprite(MAT2X(coluna),MAT2Y(linha), grade[2]);
        }

         //Alterna entre cursor maior/menor
        pac->status = (pac->status+1) % 20;
    }

}

void desenha_ponto(char *test){
    if(*test == 'x'){
        desenhaSprite(MAT2X(3),MAT2Y(10), vez_x);
    }else if(*test == 'o'){
        desenhaSprite(MAT2X(3),MAT2Y(10), vez_o);
    }
}
