/**
    Nombre de la aplicación: Paint Life (testcases mod)
    Autor: Ariel González Gómez (arielgg46)
    Fecha de Actualización: 22 de Junio 2021
    *Más información en el archivo LEEME adjunto
**/
#include <bits/stdc++.h>
#include <windows.h>
#include <conio.h>
#define ARRIBA        72
#define IZQUIERDA     75
#define DERECHA       77
#define ABAJO         80
#define ENTER         13
#define DELETE         8
#define ESCAPE        27
#define f first
#define s second
#define ld long double
#define ll long long

using namespace std;

struct pic_color ///pixel de color en la consola
{
    int car,col;
    bool operator<(const pic_color o)const
    {
        if(car!=o.car)return car<o.car;
        return col<o.col;
    }
    bool operator==(const pic_color o)const
    {
        return (car==o.car and col==o.col);
    }
    bool operator!=(const pic_color o)const
    {
        return (car!=o.car or col!=o.col);
    }
};

const pic_color vacio={-1,-1}; ///"tranparente" o "vacío" (color blanco)
const pic_color black={0,0}; ///color negro

int tamanno_caracteres=5; ///tamaño estándar del estilo de letra único que tengo
int tamanno_color=8; ///tamaño de las celdas de la paleta
int curcellj,curcelli; ///índices de la celda de color actual
pic_color dib[1000][1000]; ///representa el color real de cada pixel del área del dibujo
pic_color sel[1000][1000]; ///representa el color de cada pixel de la selección
pair<int,int> colcell[1000][1000];  ///representa la celda del color visible de cada pixel del área del dibujo
int cur_tool_cell=0; ///herramienta actual
int tamanno_cursor=1; ///tamaño de dibujo del cursor
int tamanno_circ=1; ///tamaño de dibujo de circunferencias (radio)
//int lim_izq=0,lim_der=670,lim_inf=64,lim_sup=339; ///límites del área de dibujo
int lim_izq=0,lim_der=350,lim_inf=64,lim_sup=264; ///límites del área de dibujo
int curx=1,cury=lim_inf; ///coordenadas del cursor
int antx=1,anty=lim_inf; ///anteriores coordenadas del cursor
int dr[8]={-1,0,1,0,1,1,-1,-1}; ///atajo para el flood-fill vertical N-E-S-O
int dc[8]={0,-1,0,1,1,-1,1,-1}; ///atajo para el flood-fill horizontal N-E-S-O
bool is8dir=0;
bool lucida=1; ///usando fuente Lucida
bool minicar=1; ///usando los caracteres de tamaño 1 x 2
bool dirty_lucida=1; ///la fuente lucida a veces se ve diferente la primera vez que se imprime
pic_color backcolor={2,255}; ///identificador del color de fondo del área de dibujo
bool moving_lim_sup,moving_lim_der; ///moviendo los límites del área de dibujo
bool impout; ///imprimiendo caracteres afuera de los límites(textbox open,close,save windows)
bool file_opened; ///actualmente abierto un archivo de dibujo
bool cancel; ///cancelado open,close,exit
bool saved; ///salvado el archivo actual
string opened_file; ///nombre del archivo de dibujo abierto
string console_title="Paint Life"; ///título de la consola
string mov_console_title="";
int title_size;

stack<vector<vector<pic_color> > > states; ///matrices de los estados anteriores (Z)
stack<vector<vector<pic_color> > > st_rev; ///matrices de los estados siguientes (Y)
///variables de windows
HANDLE hStdin;
DWORD fdwSaveOldMode;
///funciones procesadoras de eventos
VOID ErrorExit(LPSTR);
VOID KeyEventProc(KEY_EVENT_RECORD);
VOID MouseEventProc(MOUSE_EVENT_RECORD);
VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD);

bool mouse_pressed=0,key_pressed=0; ///booleanos de clic presionado y de tecla presionada
vector<int> cpx,cpy; ///puntos de control de las líneas de Bézier
unsigned long long dp[1000][1000]; ///triángulo de Pascal para los coeficientes binomiales

///nombres de las herramientas
vector<char*> tool_s={"lapiz.txt","goma.txt","rellenar.txt","gotero.txt","text.txt","line.txt","circle.txt","rectangle.txt","select.txt","triangle.txt","spray.txt","poligon.txt","curve.txt","exit.txt","open.txt","save.txt","pentalfa.txt","pentagon.txt","heart.txt"}; ///nombres de los archivos de texto de las herramientas
///coordenadas de las herramientas
vector<int> tool_x={443,463,483,513,533,553,580,614,635,438,456,482,507,656,655,654,524,549,575};
vector<int> tool_y={2,5,2,1,2,4,2,2,2,40,28,36,36,2,22,46,34,32,36};
int toolmap[1000][1000]; ///guarda los índices de las herramientas impresas en el mapa

bool game[1000][1000];
int lifecolor=37;

int c_testcases;
int n_noise_variations;
int n_move_variations;
int n_noise_move_variations;
bool b_reflect;
bool b_microsoft_paint;
int noise_variation_range;
int move_variation_range;
string testcases_name;

struct bmp_color ///pixel de color de imagen .bmp
{
    int r,g,b;
    bool operator<(const bmp_color o)const
    {
        if(r!=o.r)return r<o.r;
        if(g!=o.g)return g<o.g;
        return b<o.b;
    }
};

bmp_color promedio(vector<bmp_color> v) ///devuelve un color con el promedio de cada componente de todos los colores del vector
{
    bmp_color res;
    res.r=res.g=res.b=0;
    for(auto i : v)
    {
        res.r+=i.r;
        res.g+=i.g;
        res.b+=i.b;
    }
    res.r/=v.size();
    res.g/=v.size();
    res.b/=v.size();
    return res;
}

bmp_color mat[15][768]; ///para guardar los colores que puedo usar en formato .bmp
bmp_color pic_to_bmp[3][256]; ///para guardar los colores .bmp mas parecido a los que puedo representar en la consola (pic_color)
bool usado[3][256];

pic_color bmp_to_pic(bmp_color c) ///lleva un color bmp al color más cercano que puedo representar en la consola
{
    pic_color res;
    int m=1000000000;
    for(int i=0;i<3;i++)
    {
        for(int j=0;j<256;j++)
        {
            int r=abs(c.r-pic_to_bmp[i][j].r);
            int g=abs(c.g-pic_to_bmp[i][j].g);
            int b=abs(c.b-pic_to_bmp[i][j].b);
//            r*=r;
//            g*=g;
//            b*=b;
            int d=r+g+b;
            if(d<m)
            {
                m=d;
                res.car=i;
                res.col=j;
            }
        }
    }
    return res;
}

pic_color _bmp_to_pic(bmp_color c) ///lleva un color bmp al color más cercano que puedo representar en la consola
{
    pic_color res;
    ld m=1000000000000000000,r,g,b,d;
    for(int i=0;i<3;i++)
    {
        for(int j=0;j<256;j++)
        {
            if(usado[i][j])continue;
            r=abs(c.r-pic_to_bmp[i][j].r);
            g=abs(c.g-pic_to_bmp[i][j].g);
            b=abs(c.b-pic_to_bmp[i][j].b);
            r*=r;
            g*=g;
            b*=b;
            r*=r;
            g*=g;
            b*=b;


//            r*=r*r;
//            g*=g*g;
//            b*=b*b;

            d=r+g+b;
//            d=r*g*b;
            if(d<m)
            {
                m=d;
                res.car=i;
                res.col=j;
            }
        }
    }
    usado[res.car][res.col]=1;
    return res;
}

vector<string> caracteres[50]= ///guarda las formas de la fuente de letra que tengo
{
    {///,
        "     ",
        "     ",
        "     ",
        "  O  ",
        " OO  ",
    },
    {///-
        "     ",
        "     ",
        "OOOO ",
        "     ",
        "     ",
    },
    {///.
        "     ",
        "     ",
        "     ",
        "     ",
        "  O  ",
    },
    {/// /
        "    O",
        "   O ",
        "  O  ",
        " O   ",
        "O    ",
    },
    {///0
        "OOOOO",
        "O   O",
        "O   O",
        "O   O",
        "OOOOO"
    },
    {///1
        " OO  ",
        "  O  ",
        "  O  ",
        "  O  ",
        "  O  "
    },
    {///2
        "OOOOO",
        "    O",
        "OOOOO",
        "O    ",
        "OOOOO"
    },
    {///3
        "OOOOO",
        "    O",
        "OOOOO",
        "    O",
        "OOOOO"
    },
    {///4
        "O   O",
        "O   O",
        "OOOOO",
        "    O",
        "    O"
    },
    {///5
        "OOOOO",
        "O    ",
        "OOOO ",
        "    O",
        "OOOO "
    },
    {///6
        "OOOOO",
        "O    ",
        "OOOOO",
        "O   O",
        "OOOOO"
    },
    {///7
        "OOOOO",
        "    O",
        " OOOO",
        "    O",
        "    O"
    },
    {///8
        "OOOOO",
        "O   O",
        "OOOOO",
        "O   O",
        "OOOOO"
    },
    {///9
        "OOOOO",
        "O   O",
        "OOOOO",
        "    O",
        "OOOOO"
    },
    {///:
        "     ",
        "     ",
        "  O  ",
        "     ",
        "  O  ",
    },
    {///;
        "     ",
        "  O  ",
        "     ",
        "  O  ",
        " OO  ",
    },
    {///<
        "     ",
        " OO0 ",
        "OO   ",
        " OOO ",
        "     ",
    },
    {///=
        "     ",
        "OOOO ",
        "     ",
        "OOOO ",
        "     ",
    },
    {///>
        "     ",
        " OOO ",
        "   OO",
        " OOO ",
        "     ",
    },
    {///?
        " OOOO",
        "    O",
        "  OOO",
        "  O  ",
        "  O  ",
    },
    {///@
        "OOOOO",
        "O   O",
        "O OO ",
        "O O O",
        "OOOOO",
    },
    {///A
        "OOOOO",
        "O   O",
        "OOOOO",
        "O   O",
        "O   O"
    },
    {///B
        "OOOOO",
        "O   O",
        "OOOO ",
        "O   O",
        "OOOOO"
    },
    {///C
        "OOOOO",
        "O   O",
        "O    ",
        "O   O",
        "OOOOO"
    },
    {///D
        "OOOO ",
        "O   O",
        "O   O",
        "O   O",
        "OOOO "
    },
    {///E
        "OOOOO",
        "O    ",
        "OOOOO",
        "O    ",
        "OOOOO"
    },
    {///F
        "OOOOO",
        "O    ",
        "OOOO ",
        "O    ",
        "O    "
    },
    {///G
        "OOOOO",
        "O    ",
        "OOOOO",
        "O   O",
        "OOOOO"
    },
    {///H
        "O   O",
        "O   O",
        "OOOOO",
        "O   O",
        "O   O"
    },
    {///I
        "OOOOO",
        "  O  ",
        "  O  ",
        "  O  ",
        "OOOOO"
    },
    {///J
        "  OOO",
        "    O",
        "    O",
        "O   O",
        " OOO "
    },
    {///K
        "O   O",
        "O  O ",
        "OOO  ",
        "O  O ",
        "O   O"
    },
    {///L
        "O    ",
        "O    ",
        "O    ",
        "O    ",
        "OOOOO"
    },
    {///M
        "O   O",
        "OO OO",
        "O O O",
        "O   O",
        "O   O"
    },
    {///N
        "O   O",
        "OO  O",
        "O O O",
        "O  OO",
        "O   O"
    },
    {///O
        " OOO ",
        "O   O",
        "O   O",
        "O   O",
        " OOO ",
    },
    {///P
        "OOOO ",
        "O   O",
        "OOOO ",
        "O    ",
        "O    "
    },
    {///Q
        " OOO ",
        "O   O",
        "O   O",
        " OOO ",
        "    O"
    },
    {///R
        "OOOO ",
        "O   O",
        "OOOO ",
        "O  O ",
        "O   O"
    },
    {///S
        "OOOOO",
        "O    ",
        "OOOOO",
        "    O",
        "OOOOO"
    },
    {///T
        "OOOOO",
        "  O  ",
        "  O  ",
        "  O  ",
        "  O  "
    },
    {///U
        "O   O",
        "O   O",
        "O   O",
        "O   O",
        " OOO "
    },
    {///V
        "O   O",
        "O   O",
        "O   O",
        " O O ",
        "  O  "
    },
    {///W
        "O   O",
        "O   O",
        "O   O",
        "O O O",
        " O O "
    },
    {///
        "O   O",
        " O O ",
        "  O  ",
        " O O ",
        "O   O"
    },
    {///
        "O   O",
        " O O ",
        "  O  ",
        "  O  ",
        "  O  "
    },
    {///
        "OOOOO",
        "   O ",
        "  O  ",
        " O   ",
        "OOOOO"
    },
    {
        "     ",
        "     ",
        "     ",
        "     ",
        "     "
    }
};

char* string_to_charaster(string s) ///recibe un string y lo devuelve como char*
{
    string cadena1=s; // constructor de string con char* arg
//    const char *ptr1 = 0; // inicializa *ptr1
    int longitud = cadena1.length();
    char *ptr2 = new char[ longitud + 1 ]; // incluyendo el carácter nulo

// copia caracteres de cadena1 a la memoria asignada
    cadena1.copy( ptr2, longitud, 0 ); // copia cadena1 a ptr2 char*
    ptr2[ longitud ] = '\0'; // agrega el terminador nulo

    cadena1.c_str();

// Asigna al apuntador ptr1 el valor const char * devuelto por
// la función data(). NOTA: ésta es una asignación potencialmente
// peligrosa. Si se modifica cadena1, el apuntador ptr1 se
// puede hacer inválido.
//    ptr1 = cadena1.data();
    return ptr2;
//    delete [] ptr2; // reclama la memoria asignada en forma dinámica
}

void move_title() ///intento de una animación del título de la consola
{
    if(title_size<160){mov_console_title=console_title[160-title_size-1]+mov_console_title;title_size++;}
    else
    {
        mov_console_title="";
        title_size=0;
    }
    SetConsoleTitleA(string_to_charaster(mov_console_title));
}

void setCColor(int color) ///establece el color de impresión de los caracteres
{
    static HANDLE hConsole;

    hConsole = GetStdHandle( STD_OUTPUT_HANDLE );

    SetConsoleTextAttribute( hConsole, color | (0 * 0x10 + 0x100) );
}

void gotoxy(int x, int y) ///establece las coordenadas del cursor de impresión en la consola
{
    HANDLE hCon;
    COORD dwPos;

    dwPos.X = x;
    dwPos.Y = y;
    hCon = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hCon,dwPos);
}

void cursorpos() ///actualiza curx y cury con la posición del cursor
{
    POINT point;
    if(GetCursorPos(&point))
    {
        if(minicar)
        {
            curx=point.x/2;
            if(point.y/2>=12)cury=point.y/2-12;
        }
        else
        {
            curx=point.x/3;
            if(point.y/5>=5)cury=point.y/5-5;
        }
    }
    gotoxy(curx,cury);
}

bool samepos() ///determina si el mouse se movió
{
    POINT point;
    if(GetCursorPos(&point))
    {
        if(minicar)
        {
            if(curx==point.x/2 and cury==point.y/2-12)return 1;
            else return 0;
        }
        else
        {
            if(curx==point.x/3 and cury==point.y/5-5)return 1;
            else return 0;
        }
    }
    return 0;
}

vector<vector<int> > old_colors= ///identificadores del color en cada celda de la paleta (viejo formato)
{
    {  4,104, 64,164,204, 12, 68,168,268,112,144,260,160,176, 76,212,228,236,252,128,220,196,270, 70, 78, 94,194,114,214,230,246,262,114,162,130,146,274, 62,190, 46, 66, 44, 14,266, 74,106,206},
    {198, 99, 98,222,238,254,136,138, 36, 32,140,139,122, 37,255, 39, 40, 90, 47,258,242,158,226,210,126,110,142, 42, 54, 34, 10,134,234,218,202, 38,  2,174,132,102, 35, 50,250, 26, 48,150,135},
    { 43,251,148,151,118,203, 56,103,219, 11, 51, 52,267,156,235,233,  3, 18,152, 49, 55,191,211,259,275,159,143,127, 59, 57,257,217,241, 41,249,209,125,109, 19,117,157, 27,119, 25, 17,  9,201},
    {101, 16,116,  1,121,120,105, 20,  5,181,273,173, 89, 21,265,189,221,165, 73,205,129, 81, 29, 80, 13,185, 85, 83,180, 65,269,169, 28,184, 88,188,161, 87, 93,193,261,253,237,113,177, 69, 92},
    { 86, 82,172,272, 95,171, 71,186, 45, 22,216,264,248,  0,100,  8,108,224,208,  7, 15,107,223,239,247,207,231,215,263,124,123,131,155,167,271,179, 63,163,115,256, 31, 23, 53, 61, 24, 91,183},
    {500,501,502,503,504,505,506,507,508,509,510,511,512,513,514,515,516,517,518,519,520,521,522,523,524,525,526,527,528,529,530,531,532,533,534,535,536,537,538,539,540,541,542,543,544,545,546}
};

vector<vector<pic_color> > colors; ///identificadores del color en cada celda de la paleta

void putmini(pic_color color,int x,int y,bool perm) ///cambia el color del pixel en esa posición
{
    gotoxy(x,y);
    ///color "transparente" o "vacío" (-1)
    if(color==vacio)
    {
        setCColor(255);
        putchar(178);
        if(perm)dib[y][x]=color; ///si se pide permanente se cambia dib[][]
        colcell[y][x]={4,28};
    }
    else
    {
        setCColor(color.col);
        putchar(color.car+176);
        if(perm)dib[y][x]=color; ///si se pide permanente se cambia dib[][]
        ///busca la fila y columna donde está (color)
        for(int i=0;i<6;i++)
        {
            for(int j=0;j<47;j++)
            {
                if(colors[i][j]==color) ///si las encuentra
                {
                    colcell[y][x]={i,j}; ///las asigna
                    return; ///fin
                }
            }
        }
    }
}

void putmini(int car,int col,int x,int y,bool perm) ///cambia el color del pixel en esa posición
{
    gotoxy(x,y);
    ///color "transparente" o "vacío" (-1)
    if(car==-1)
    {
        setCColor(255);
        putchar(178);
        if(perm)dib[y][x]=vacio; ///si se pide permanente se cambia dib[][]
        colcell[y][x]={4,28};
    }
    else
    {
        setCColor(col);
        putchar(car+176);
        if(perm)dib[y][x]={car,col}; ///si se pide permanente se cambia dib[][]
        ///busca la fila y columna donde está (color)
        for(int i=0;i<6;i++)
        {
            for(int j=0;j<47;j++)
            {
                if(colors[i][j].car==car and colors[i][j].col==col) ///si las encuentra
                {
                    colcell[y][x]={i,j}; ///las asigna
                    return; ///fin
                }
            }
        }
    }
}

void put1(pic_color color,int x,int y,bool perm) ///cambia el color del pixel en esa posición
{
    gotoxy(x,y);
    ///color "transparente" o "vacío" (-1)
    if(color==vacio)
    {
        setCColor(255);
        putchar(178);
        if(perm)dib[y][x/2]=color; ///si se pide permanente se cambia dib[][]
        colcell[y][x/2]={4,28};
    }
    else
    {
        setCColor(color.col);
        putchar(color.car+176);
        if(perm)dib[y][x/2]=color; ///si se pide permanente se cambia dib[][]
        ///busca la fila y columna donde está (color)
        for(int i=0;i<6;i++)
        {
            for(int j=0;j<47;j++)
            {
                if(colors[i][j]==color) ///si las encuentra
                {
                    colcell[y][x/2]={i,j}; ///las asigna
                    return; ///fin
                }
            }
        }
    }
}

void put1(int car,int col,int x,int y,bool perm) ///cambia el color del pixel en esa posición
{
    gotoxy(x,y);
    ///color "transparente" o "vacío" (-1)
    if(car==-1)
    {
        setCColor(255);
        putchar(178);
        if(perm)dib[y][x/2]=vacio; ///si se pide permanente se cambia dib[][]
        colcell[y][x/2]={4,28};
    }
    else
    {
        setCColor(col);
        putchar(176+car);
        if(perm)dib[y][x/2]={car,col}; ///si se pide permanente se cambia dib[][]
        ///busca la fila y columna donde está (color)
        for(int i=0;i<6;i++)
        {
            for(int j=0;j<47;j++)
            {
                if(colors[i][j].car==car and colors[i][j].col==col) ///si las encuentra
                {
                    colcell[y][x/2]={i,j}; ///las asigna
                    return; ///fin
                }
            }
        }
    }
}

void put(pic_color color,int x,int y,bool perm) ///cambia el color del pixel en esa posición
{
    if(minicar)
    {
        put1(color,x*2,y,perm);
        put1(color,x*2+1,y,perm);
    }
    else
    {
        putmini(color,x,y,perm);
    }
}

void put(int car,int col,int x,int y,bool perm) ///cambia el color del pixel en esa posición
{
    if(minicar)
    {
        put1(car,col,x*2,y,perm);
        put1(car,col,x*2+1,y,perm);
    }
    else
    {
        putmini(car,col,x,y,perm);
    }
}

void clean_dibarea(int x,int y) ///limpia los pixeles área de dibujo a partir de x o y
{
    for(int i=lim_inf;i<lim_sup;i++)
    {
        for(int j=lim_izq;j<lim_der;j++)
        {
            if(i>=y or j>=x)dib[i][j]=vacio;
        }
    }
}

void resalt(char* s,pic_color color) ///resalta la figura del archivo con nombre (char* s)
{
    FILE *f=fopen(s,"r");
    int r,c;
    fscanf(f,"%d%d",&r,&c);
    bool b[r][c];
    for(int i=0;i<r;i++)
    {
        for(int j=0;j<c;j++)
        {
            pic_color p;
            fscanf(f,"%d%d",&p.car,&p.col);
            b[i][j]=(p!=vacio);
        }
    }
    fclose(f);
    for(int i=0;i<r;i++)
    {
        for(int j=0;j<c;j++)
        {
            if(b[i][j])
            {
                for(int k=0;k<4;k++)
                {
                    int ni=i+dr[k],nj=j+dc[k];
                    if(ni<0 or ni==r or nj<0 or nj==c)
                    {
                        put(color,curx+nj,cury+ni,1);
                    }
                    else if(!b[ni][nj])
                    {
                        put(color,curx+nj,cury+ni,1);
                    }
                }
            }
        }
    }
}

void unresalt(char* s) ///des-resalta la figura del archivo con nombre (char* s)
{
    FILE *f=fopen(s,"r");
    int r,c;
    fscanf(f,"%d%d",&r,&c);
    bool b[r+111][c+111];
    for(int i=0;i<r;i++)
    {
        for(int j=0;j<c;j++)
        {
            pic_color p;
            fscanf(f,"%d%d",&p.car,&p.col);
            b[i][j]=(p!=vacio);
        }
    }
    fclose(f);
    for(int i=0;i<r;i++)
    {
        for(int j=0;j<c;j++)
        {
            if(b[i][j])
            {
                for(int k=0;k<4;k++)
                {
                    int ni=i+dr[k],nj=j+dc[k];
                    if(ni<0 or ni==r or nj<0 or nj==c)
                    {
                        put(1,7,curx+nj,cury+ni,1);
                    }
                    else if(!b[ni][nj])
                    {
                        put(1,7,curx+nj,cury+ni,1);
                    }
                }
            }
        }
    }
}

void paste(char* s,bool perm) ///pega la figura del archivo con nombre (char* s)
{
    FILE *f=fopen(s,"r");
    int r,c;
    fscanf(f,"%d%d",&r,&c);
    for(int i=0;i<r;i++)
    {
        for(int j=0;j<c;j++)
        {
            pic_color p;
            fscanf(f,"%d%d",&p.car,&p.col);
            if(p!=vacio)
            {
                if(curx+j<lim_der and cury+i<lim_sup and curx+j>=lim_izq and cury+i>=lim_inf)put(p,curx+j,cury+i,perm);
            }
        }
    }
    fclose(f);
}

void paste_out(char* s,bool perm) ///pega la figura del archivo con nombre (char* s)
{
    FILE *f=fopen(s,"r");
    int r,c;
    fscanf(f,"%d%d",&r,&c);
    for(int i=0;i<r;i++)
    {
        for(int j=0;j<c;j++)
        {
            pic_color p;
            fscanf(f,"%d%d",&p.car,&p.col);
            if(p!=vacio)
            {
                put(p,curx+j,cury+i,perm);
            }
        }
    }
    fclose(f);
}

void pastetools(int id) ///pega la herramienta de índice (id) y establece la herramienta en esa posición del mapa
{
    char* s=tool_s[id];
    FILE *f=fopen(s,"r");
    int r,c;
    fscanf(f,"%d%d",&r,&c);
    for(int i=0;i<r;i++)
    {
        for(int j=0;j<c;j++)
        {
            pic_color p;
            fscanf(f,"%d%d",&p.car,&p.col);
            toolmap[cury+i][curx+j]=id;
            if(p!=vacio)
            {
//                if((cury+i)*2<lim_sup)put(p,curx+j,cury+i,1);
                put(p,curx+j,cury+i,1);
            }
        }
    }
    fclose(f);
}

void _pastetools(int id) ///pega la herramienta de índice (id) y establece la herramienta en esa posición del mapa
{
    char* s=tool_s[id];
    FILE *f=fopen(s,"r");
    int r,c;
    fscanf(f,"%d%d",&r,&c);
    for(int i=0;i<r;i++)
    {
        for(int j=0;j<c;j++)
        {
            pic_color p;
            fscanf(f,"%d%d",&p.car,&p.col);
            toolmap[cury+i][curx+j]=id;
            if(p!=vacio)
            {
                if((cury+i)*2<lim_sup)put(p,curx+j,cury+i,1);
            }
        }
    }
    fclose(f);
}

string fancy(int a,int b) ///devuelve un string de tamaño b representando al número (a)
{
    string s="";
    bool neg=0;
    if(a<0)
    {
        neg=1;
        a=-a;
    }
    if(a==0)s="0";
    while(a!=0)
    {
        char c=a%10+48;
        s=c+s;
        a/=10;
    }
    if(neg)s="-"+s;
    while(s.size()<b)s=" "+s;
    return s;
}

void cop(int x1,int x2,int y1,int y2) ///copia esa parte del área de dibujo al documento copy.txt
{
    if(x1>x2)swap(x1,x2);
    if(y1>y2)swap(y1,y2);
    ofstream f("copy.txt");
    f<<y2-y1+1<<" "<<x2-x1+1;
    for(int i=y1;i<=y2;i++)
    {
        f<<"\n";
        for(int j=x1;j<=x2;j++)
        {
            string a=fancy(sel[i-y1][j-x1].car,1),b=fancy(sel[i-y1][j-x1].col,3);
            f<<a<<" "<<b<<" ";
        }
    }
    f.close();
}

void imp(string text,int lat,int espaciado_vertical_texto,int espaciado_horizontal_texto,pic_color color_texto,bool perm)///imprime un texto en esa posición con color (color_texto) con mi tipo de letra
{
    int t=text.size();
    int let=-1,lin=0;
    for(int i=0;i<t;i++)
    {
        let++;
        if('a'<=text[i] and text[i]<='z')text[i]=text[i]-'a'+'A';
        int car=text[i]-',';
        if(text[i]==' ')car='Z'+1-',';
        if(text[i]=='*')
        {
            let=-1;
            lin++;
            continue;
        }
        for(int j=0;j<tamanno_caracteres;j++)
        {
            for(int k=0;k<tamanno_caracteres;k++)
            {
                int c1=k+espaciado_horizontal_texto+let*7,c2=j+espaciado_vertical_texto+lin*7;
                if(!impout)if(c1>=lim_der or c2>=lim_sup)continue;
                if(caracteres[car][j][k]!=' ')
                {
                    put(color_texto,c1,c2,perm);
                }
                else if(impout)put(backcolor,c1,c2,0);
                else if(dib[c2][c1]!=vacio)put(dib[c2][c1],c1,c2,0);
                else put(backcolor,c1,c2,0);
            }
        }
        Sleep(lat);
    }
}

string read(int espaciado_vertical_texto,int espaciado_horizontal_texto,int lim,pic_color color_texto,bool enter_allowed,bool perm) ///herramienta de texto
{
    string s;
    int t=0;
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");

    while(1)
    {
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.

        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)
            {
                case KEY_EVENT: // keyboard input
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    if(key_pressed)
                    {
                        char c=(irInBuf[i].Event.KeyEvent.wVirtualKeyCode); ///hay problemas con los signos de puntuacion + - / * . , ; :
//                        if(c==DERECHA or c==IZQUIERDA or c==ARRIBA or c==ABAJO)break;
                        if(c==' ' and t!=lim)
                        {
                            s+=" ";
                            t++;
                            break;
                        }
                        if(c==ESCAPE)
                        {
                            imp(s,0,espaciado_vertical_texto,espaciado_horizontal_texto,color_texto,perm);
                            return s;
                        }
                        if(c==ENTER and enter_allowed)s+="*";
                        if(c==DELETE and !s.empty())
                        {
                            string tmp=s;
                            s="";
                            for(int i=0; i<tmp.size()-1; i++)s+=tmp[i];
                            imp(s+" ",0,espaciado_vertical_texto,espaciado_horizontal_texto,color_texto,0);
                            break;
                        }
                        if(s.size()==lim)break;
                        if(isalnum(c))s+=c;
                        imp(s,0,espaciado_vertical_texto,espaciado_horizontal_texto,color_texto,0);
                        break;
                    }
                }
                case MOUSE_EVENT: // mouse input
                {
                    bool tmp=mouse_pressed;
                    MouseEventProc(irInBuf[i].Event.MouseEvent);
//                    bool moved=!samepos();
                    cursorpos();
                    if(tmp and !mouse_pressed)
                    {
                        imp(s,0,espaciado_vertical_texto,espaciado_horizontal_texto,color_texto,perm);
                        return s;
                    }
                }
                case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT:  // disregard focus events

                case MENU_EVENT:   // disregard menu events
                    break;

                default:
                    ErrorExit("Unknown event type");
                    break;
            }
        }
    }
}

void addstate() ///añade un estado de (Z)
{
    vector<vector<pic_color> > st;
    for(int i=0;i<lim_sup;i++)
    {
        vector<pic_color> v(1000,vacio);
        for(int j=0;j<lim_der;j++)
        {
            v[j]=dib[i][j];
        }
        st.push_back(v);
    }
    states.push(st);
    while(!st_rev.empty())st_rev.pop();
}

void revstate() ///ejecuta una transición de (Z)
{
    if(states.size()>1)
    {
        vector<vector<pic_color> > st;
        states.pop();
        for(int i=0; i<states.top().size(); i++)
        {
            vector<pic_color> v;
            for(int j=0; j<states.top()[0].size(); j++)
            {
                v.push_back(dib[i][j]);
                if(i>=lim_inf and i<lim_sup and j<lim_der and j>=lim_izq and states.top()[i][j]!=dib[i][j])put(states.top()[i][j],j,i,1);
            }
            st.push_back(v);
        }
        st_rev.push(st);
    }
}

void state_rev() ///ejecuta una transición (Y)
{
    if(st_rev.size()>0)
    {
        vector<vector<pic_color> > st;
        for(int i=0; i<st_rev.top().size(); i++)
        {
            for(int j=0; j<st_rev.top()[0].size(); j++)
            {
                if(i>=lim_inf and i<lim_sup and j<lim_der and j>=lim_izq and st_rev.top()[i][j]!=dib[i][j])put(st_rev.top()[i][j],j,i,1);
            }
        }
        st_rev.pop();
        for(int i=0; i<lim_sup; i++)
        {
            vector<pic_color> v;
            for(int j=0; j<lim_der; j++)
            {
                v.push_back(dib[i][j]);
            }
            st.push_back(v);
        }
        states.push(st);
    }
}

void mark_cell(int i,int j) ///marca una celda de la paleta con esos índices
{
    pic_color cell_color={2,47};
    for(int k=0;k<=tamanno_color;k++)
    {
        put(cell_color,j*tamanno_color+j,i*tamanno_color+k+i,0);
        put(cell_color,j*tamanno_color+j+k,i*tamanno_color+i,0);
        put(cell_color,(j+1)*tamanno_color+j+1,i*tamanno_color+k+i,0);
        put(cell_color,j*tamanno_color+j+k,(i+1)*tamanno_color+i+1,0);
    }
    put(cell_color,(j+1)*tamanno_color+j+1,(i+1)*tamanno_color+i+1,0);
}

void unmark_cell(int i,int j) ///desmarca una celda de la paleta con esos índices
{
    pic_color cell_color=black;
    for(int k=0;k<=tamanno_color;k++)
    {
        put(cell_color,j*tamanno_color+j,i*tamanno_color+k+i,0);
        put(cell_color,j*tamanno_color+j+k,i*tamanno_color+i,0);
        put(cell_color,(j+1)*tamanno_color+j+1,i*tamanno_color+k+i,0);
        put(cell_color,j*tamanno_color+j+k,(i+1)*tamanno_color+i+1,0);
    }
    put(cell_color,(j+1)*tamanno_color+j+1,(i+1)*tamanno_color+i+1,0);
}

void cleantext() ///limpia el área de paleta y herramientas para imprimir un texto
{
    setCColor(0);
    for(int i=0;i<lim_inf;i++)
    {
        for(int j=0;j<lim_der;j++)
        {
            gotoxy(j,i);
            putchar(176);
        }
    }
}

void rellenar(pic_color color,int i,int j) ///rellena un área cercada llena de cierto color con un nuevo color
{
    ///algoritmo flood-fill con BFS
    queue<pair<int,int> > q;
    pic_color curcol=dib[i][j];
    q.push({i,j});
    while(!q.empty())
    {
        i=q.front().first;
        j=q.front().second;
        q.pop();
        for(int k=0; k<4+4*is8dir; k++)
        {
            int ni=i+dr[k],nj=j+dc[k];
            if(ni>=lim_inf and ni<lim_sup and nj>=lim_izq and nj<lim_der and dib[ni][nj]!=color and dib[ni][nj]==curcol){q.push({ni,nj});put(color,nj,ni,1);}
        }
    }
}

void copycolor(int x,int y) ///establece la celda de color con el color visible del pixel con esas coordenadas (herramienta gotero)
{
    unmark_cell(curcelli,curcellj);
    curcelli=colcell[y][x].first;
    curcellj=colcell[y][x].second;
    mark_cell(curcelli,curcellj);
}

void draw_interrupted_line(long double x1,long double x2,long double y1,long double y2,bool perm,bool colmap,int jumpsize) ///dibuja una línea de rectángulo de selección
{
    if(x2==x1)
    {
        if(y1>y2)swap(y1,y2);
        for(int i=y1; i<=y2; i+=jumpsize)
        {
            if(i<lim_inf or i>=lim_sup or x1<lim_izq or x1>=lim_der)continue;
            if(colmap)
            {
                if(dib[i][(int)x1]!=vacio)
                {
                    put(dib[i][(int)x1],x1,i,0);
                }
                else
                {
                    put(backcolor,x1,i,0);
                }
            }
            else put(2,63,x1,i,perm);
        }
        return;
    }
    long double m=(y2-y1)/(x2-x1);
    long double n=y1-m*x1;
    if(abs(x2-x1)>abs(y2-y1)){
    if(x1>x2)swap(x1,x2);
    if(x1<x2)for(long double i=x1; i<=x2; i+=jumpsize)
        {
            long double y=(long double)(m*i+n),x=i;
            if(y<lim_inf or y>=lim_sup or x<lim_izq or x>=lim_der)continue;
            if(colmap){if(dib[(int)y][(int)x]!=vacio)put(dib[(int)y][(int)x],i,y,0);else put(backcolor,i,y,0);}
            else put(2,63,i,y,perm);
        }
    }
    else
    {
        if(y1>y2)swap(y1,y2);
        if(y1<y2)for(long double i=y1; i<=y2; i+=jumpsize)
            {
                long double x=(long double)((i-n)/m),y=i;
                if(y<lim_inf or y>=lim_sup or x<lim_izq or x>=lim_der)continue;
                if(colmap)
                {
                    if(dib[(int)y][(int)x]!=vacio)put(dib[(int)y][(int)x],x,y,0);
                    else put(backcolor,x,y,0);
                }
                else put(2,63,x,y,perm);
            }
    }
}

void draw_interrupted_rectangle(long double x1,long double x2,long double y1,long double y2,bool perm,bool colmap) ///dibuja un rectángulo de selección
{
    draw_interrupted_line(x1,x2,y1,y1,perm,colmap,2);
    draw_interrupted_line(x1,x2,y2,y2,perm,colmap,2);
    draw_interrupted_line(x1,x1,y1,y2,perm,colmap,2);
    draw_interrupted_line(x2,x2,y1,y2,perm,colmap,2);
}

vector<pic_color> rsz(vector<pic_color> a,int l,int r,int c,int p) ///resize del vector dado
{
    vector<pic_color> v;
    if(l==r)
    {
        for(int i=0;i<p+c;i++)v.push_back(a[l]);
        return v;
    }
    int m=(r+l)/2;
//    if(l==r-1) m=l;
    int lc=c/2,rc=c/2;
    if(!((r-l+1)&1))//interval par
    {
        rc=(c+1)/2;
    }
    else lc=(c+1)/2;
    vector<pic_color> vl=rsz(a,l,m,lc,p), vr=rsz(a,m+1,r,rc,p);
    for(auto i : vl)v.push_back(i);
    for(auto i : vr)v.push_back(i);
    return v;
}

vector<vector<pic_color> > rsz(vector<vector<pic_color> > a,int l,int r,int c,int p) ///resize del vector de vectores dado
{
    vector<vector<pic_color> > v;
    if(l==r)
    {
        for(int i=0;i<p+c;i++)v.push_back(a[l]);
        return v;
    }
    int m=(r+l)/2;
//    if(l==r-1) m=l;
    int lc=c/2,rc=c/2;
    if(!((r-l+1)&1))//interval par
    {
        rc=(c+1)/2;
    }
    else lc=(c+1)/2;
    vector<vector<pic_color> > vl=rsz(a,l,m,lc,p), vr=rsz(a,m+1,r,rc,p);
    for(auto i : vl)v.push_back(i);
    for(auto i : vr)v.push_back(i);
    return v;
}

vector<vector<pic_color> > rszmat(vector<vector<pic_color> > M,int n,int m) ///resize del vector de vectores dado
{
    for(auto &v : M)
    {
        v=rsz(v,0,v.size()-1,m%v.size(),m/v.size());
    }
    M=rsz(M,0,M.size()-1,n%M.size(),n/M.size());
    return M;
}

void rotate_selection_right(int &x1,int &y1,int &x2,int &y2) ///rota la selección 90 grados a la derecha
{
    int nx1,ny1,nx2,ny2,mx=(x2-x1),my=(y2-y1);
    nx1=x1+mx/2-my/2;
    nx2=nx1+my;
    ny1=y1+my/2-mx/2;
    ny2=ny1+mx;
    for(int i=x1;i<=x2;i++)
    {
        for(int j=y1;j<=y2;j++)
        {
//            if(j>=lim_inf and j<lim_sup and i>=lim_izq and i<lim_der)put(vacio,i,j,0);
            if(j>=lim_inf and j<lim_sup and i>=lim_izq and i<lim_der)put(dib[j][i],i,j,0);
        }
    }
    vector<vector<pic_color>> v;
    for(int i=nx1,k=0;i<=nx2;i++,k++)
    {
        vector<pic_color> w;
        for(int j=ny1,l=0;j<=ny2;j++,l++)
        {
            if(j>=lim_inf and j<lim_sup and i>=lim_izq and i<lim_der and sel[my-k][l]!=vacio)put(sel[my-k][l],i,j,0);
            w.push_back(sel[my-k][l]);
        }
        v.push_back(w);
    }
    x1=nx1;
    x2=nx2;
    y1=ny1;
    y2=ny2;
    for(int i=x1;i<=x2;i++)
    {
        for(int j=y1;j<=y2;j++)
        {
            sel[j-y1][i-x1]=v[i-x1][j-y1];
        }
    }
    draw_interrupted_rectangle(x1,x2,y1,y2,0,0);
}

void rotate_selection_left(int &x1,int &y1,int &x2,int &y2) ///rota la selección 90 grados a la izquierda
{
    int nx1,ny1,nx2,ny2,mx=(x2-x1),my=(y2-y1);
    nx1=x1+mx/2-my/2;
    nx2=nx1+my;
    ny1=y1+my/2-mx/2;
    ny2=ny1+mx;
    for(int i=x1;i<=x2;i++)
    {
        for(int j=y1;j<=y2;j++)
        {
//            if(j>=lim_inf and j<lim_sup and i>=lim_izq and i<lim_der)put(vacio,i,j,0);
            if(j>=lim_inf and j<lim_sup and i>=lim_izq and i<lim_der)put(dib[j][i],i,j,0);
        }
    }
    vector<vector<pic_color>> v;
    for(int i=nx1,k=0;i<=nx2;i++,k++)
    {
        vector<pic_color> w;
        for(int j=ny1,l=0;j<=ny2;j++,l++)
        {
            if(j>=lim_inf and j<lim_sup and i>=lim_izq and i<lim_der and sel[k][mx-l]!=vacio)put(sel[k][mx-l],i,j,0);
            w.push_back(sel[k][mx-l]);
        }
        v.push_back(w);
    }
    x1=nx1;
    x2=nx2;
    y1=ny1;
    y2=ny2;
    for(int i=x1;i<=x2;i++)
    {
        for(int j=y1;j<=y2;j++)
        {
            sel[j-y1][i-x1]=v[i-x1][j-y1];
        }
    }
    draw_interrupted_rectangle(x1,x2,y1,y2,0,0);
}

void move_selection(int x1,int x2,int y1,int y2) ///herramienta para mover el área de selección
{
    int x3,y3;
    bool rot=1;
    bool moving_sellim_sup=0,moving_sellim_der=0,moving_sellim_inf=0,moving_sellim_izq=0,flag=0;

    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");

    while(1)
    {
        if (! ReadConsoleInput(
        hStdin,      // input buffer handle
        irInBuf,     // buffer to read into
        128,         // size of read buffer
        &cNumRead) ) // number of records read
        ErrorExit("ReadConsoleInput");

        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)
            {
                case KEY_EVENT: // keyboard input
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    char tecla=(char)(irInBuf[i].Event.KeyEvent.wVirtualKeyCode);
                    switch (tecla)
                    {
                        case 'c':
                            cop(x1,x2,y1,y2);
                            break;
                        case 'C':
                            cop(x1,x2,y1,y2);
                            break;
                        case 'D':
                            if(rot)
                            {
                                rotate_selection_right(x1,y1,x2,y2);
                                rot=0;
                            }
                            else rot=1;
                            break;
                        case 'A':
                            if(rot)
                            {
                                rotate_selection_left(x1,y1,x2,y2);
                                rot=0;
                            }
                            else rot=1;
                            break;
                        case DELETE:
                        {
                            for(int i=y1;i<=y2;i++)
                            {
                                for(int j=x1;j<=x2;j++)
                                {
                                    if(j<lim_izq or i<lim_inf or j>=lim_der or i>=lim_sup)continue;
                                    put(dib[i][j],j,i,0);
                                }
                            }
                            return;
                            break;
                        }
                        case ESCAPE:
                        {
                            draw_interrupted_rectangle(x1,x2,y1,y2,0,1);
                            for(int i=y1;i<=y2;i++)
                            {
                                for(int j=x1;j<=x2;j++)
                                {
                                    if(j<lim_izq or i<lim_inf or j>=lim_der or i>=lim_sup)continue;
                                    if(sel[i-y1][j-x1]!=vacio)put(sel[i-y1][j-x1],j,i,1);
                                }
                            }
                            return;
                            break;
                        }
                    }
                    break;
                }
                case MOUSE_EVENT: // mouse input
                {
                    bool tmp=mouse_pressed;
                    MouseEventProc(irInBuf[i].Event.MouseEvent);
                    bool moved=!samepos();
                    cursorpos();
                    if(!tmp and mouse_pressed)
                    {
                        if(curx==x1)
                        {
                            moving_sellim_izq=1;
                        }
                        if(curx==x2)
                        {
                            moving_sellim_der=1;
                        }
                        if(cury==y1)
                        {
                            moving_sellim_inf=1;
                        }
                        if(cury==y2)
                        {
                            moving_sellim_sup=1;
                        }
                    }
                    if(tmp and !mouse_pressed)
                    {
                        vector<vector<pic_color> > selv;
                        if(moving_sellim_inf or moving_sellim_sup or moving_sellim_der or moving_sellim_izq)
                        {
                            for(int i=y1; i<=y2; i++)
                            {
                                vector<pic_color> v;
                                for(int j=x1; j<=x2; j++)
                                {
                                    v.push_back(sel[i-y1][j-x1]);
                                    if(j<lim_izq or i<lim_inf or j>=lim_der or i>=lim_sup)continue;
//                                    if(dib[i][j]!=vacio)put(dib[i][j],j,i,0);
//                                    if(dib[i][j]!=vacio)put(vacio,j,i,0);
                                    put(dib[i][j],j,i,0);
                                }
                                selv.push_back(v);
                            }
                        }
                        if(moving_sellim_der)
                        {
                            x2=curx;
                        }
                        if(moving_sellim_izq)
                        {
                            x1=curx;
                        }
                        if(moving_sellim_sup)
                        {
                            y2=cury;
                        }
                        if(moving_sellim_inf)
                        {
                            y1=cury;
                        }
                        if(moving_sellim_inf or moving_sellim_sup or moving_sellim_der or moving_sellim_izq)
                        {
                            bool ver=(y1>y2),hor=(x1>x2);
                            if(ver)swap(y1,y2);
                            if(hor)swap(x1,x2);

                            if(ver)
                            {
                                int I=selv.size();
                                int J=selv[0].size();
                                for(int i=0; i<I/2; i++)
                                {
                                    for(int j=0; j<J; j++)
                                    {
                                        swap(selv[i][j],selv[I-i-1][j]);
                                    }
                                }
                            }
                            if(hor)
                            {
                                int I=selv.size();
                                int J=selv[0].size();
                                for(int i=0; i<I; i++)
                                {
                                    for(int j=0; j<J/2; j++)
                                    {
                                        swap(selv[i][j],selv[i][J-j-1]);
                                    }
                                }
                            }
                            selv=rszmat(selv,y2-y1+1,x2-x1+1);
                            for(int i=y1; i<=y2; i++)
                            {
                                for(int j=x1; j<=x2; j++)
                                {
                                    sel[i-y1][j-x1]=selv[i-y1][j-x1];
                                }
                            }
                            for(int i=y1; i<=y2; i++)
                            {
                                for(int j=x1; j<=x2; j++)
                                {
                                    if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der)continue;
//                                    if(sel[i-y1][j-x1]==vacio)put(dib[i][j],j,i,0);
//                                    else put(sel[i-y1][j-x1],j,i,0);
                                    if(sel[i-y1][j-x1]!=vacio)put(sel[i-y1][j-x1],j,i,0);
                                }
                            }
                            draw_interrupted_rectangle(x1,x2,y1,y2,0,0);
                        }
                        moving_sellim_izq=0;
                        moving_sellim_der=0;
                        moving_sellim_inf=0;
                        moving_sellim_sup=0;
                        break;
                    }
                    if(moving_sellim_inf or moving_sellim_sup or moving_sellim_der or moving_sellim_izq)
                    {
                        break;
                    }
                    if(tmp)
                    {
                        if(!mouse_pressed and (curx<x1 or curx>x2 or cury<y1 or cury>y2))
                        {
                            draw_interrupted_rectangle(x1,x2,y1,y2,0,1);
                            for(int i=y1;i<=y2;i++)
                            {
                                for(int j=x1;j<=x2;j++)
                                {
                                    if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der)continue;
//                                    if(sel[i-y1][j-x1]==vacio)put(dib[i][j],j,i,1);
//                                    else put(sel[i-y1][j-x1],j,i,1);
                                    if(sel[i-y1][j-x1]!=vacio)put(sel[i-y1][j-x1],j,i,1);
                                }
                            }
                            return;
                        }
                    }
                    if(!tmp and mouse_pressed)
                    {
                        x3=curx;
                        y3=cury;
                    }
                    if(mouse_pressed)
                    {
                        if(moved)
                        {
                            if(!flag){flag=1;continue;}
                            draw_interrupted_rectangle(x1,x2,y1,y2,0,1);
                            for(int i=y1;i<=y2;i++)
                            {
                                for(int j=x1;j<=x2;j++)
                                {
                                    if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der)continue;
                                    put(dib[i][j],j,i,0);
                                }
                            }
                            x1-=(x3-curx);
                            y1-=(y3-cury);
                            x2-=(x3-curx);
                            y2-=(y3-cury);
                            x3=curx;
                            y3=cury;
                            for(int i=y1;i<=y2;i++)
                            {
                                for(int j=x1;j<=x2;j++)
                                {
                                    if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der)continue;
//                                    if(sel[i-y1][j-x1]==vacio)put(dib[i][j],j,i,0);
//                                    else put(sel[i-y1][j-x1],j,i,0);
                                    if(sel[i-y1][j-x1]!=vacio)put(sel[i-y1][j-x1],j,i,0);
                                }
                            }
                            draw_interrupted_rectangle(x1,x2,y1,y2,0,0);
                        }
                    }
                    break;
                }
                case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT:  // disregard focus events

                case MENU_EVENT:   // disregard menu events
                    break;

                default:
                    ErrorExit("Unknown event type");
                    break;
            }
        }
    }
}

void draw_line(long double x1,long double x2,long double y1,long double y2,bool perm,bool colmap) ///dibuja una línea recta
{
    if(x2==x1)
    {
        if(y1>y2)swap(y1,y2);
        for(int i=y1; i<=y2; i++)
        {
            if(i<lim_inf or i>=lim_sup or x1>=lim_der)continue;
            if(colmap)
            {
                if(dib[i][(int)x1]!=vacio)
                {
                    put(dib[i][(int)x1],x1,i,0);
                }
                else
                {
                    put(backcolor,x1,i,0);
                }
            }
            else put(colors[curcelli][curcellj],x1,i,perm);
        }
        return;
    }
    long double m=(y2-y1)/(x2-x1);
    long double n=y1-m*x1;
    if(abs(x2-x1)>abs(y2-y1)){
    if(x1>x2)swap(x1,x2);
    if(x1<x2)for(long double i=x1; i<=x2; i++)
        {
            long double y=(long double)(m*i+n),x=i;
            if(i<lim_izq or i>=lim_der or y<lim_inf or y>=lim_sup)continue;
            if(colmap){if(dib[(int)y][(int)x]!=vacio)put(dib[(int)y][(int)x],i,y,0);else put(backcolor,i,y,0);}
            else put(colors[curcelli][curcellj],i,y,perm);
        }
    }
    else
    {
        if(y1>y2)swap(y1,y2);
        if(y1<y2)for(long double i=y1; i<=y2; i++)
            {
                long double x=(long double)((i-n)/m),y=i;
                if(i<lim_inf or i>=lim_sup or x<lim_izq or x>=lim_der)continue;
                if(colmap)
                {
                    if(dib[(int)y][(int)x]!=vacio)put(dib[(int)y][(int)x],x,y,0);
                    else put(backcolor,x,y,0);
                }
                else put(colors[curcelli][curcellj],x,y,perm);
            }
    }
}

void erase_line(long double x1,long double x2,long double y1,long double y2) ///"borra" una línea recta
{
    if(x2==x1)
    {
        if(y1>y2)swap(y1,y2);
        for(int i=y1; i<=y2; i++)
        {
            if(x1<lim_izq or x1>=lim_der or i<lim_inf or i>=lim_sup)continue;
            put(vacio,x1,i,1);
        }
        return;
    }
    long double m=(y2-y1)/(x2-x1);
    long double n=y1-m*x1;
    if(abs(x2-x1)>abs(y2-y1))
    {
        if(x1>x2)swap(x1,x2);
        if(x1<x2)for(long double i=x1; i<=x2; i++)
        {
            long double y=(long double)(m*i+n);
            if(i<lim_izq or i>=lim_der or y<lim_inf or y>=lim_sup)continue;
            put(vacio,i,y,1);
        }
    }
    else
    {
        if(y1>y2)swap(y1,y2);
        if(y1<y2)for(long double i=y1; i<=y2; i++)
        {
            long double x=(long double)((i-n)/m),y=i;
            if(i<lim_inf or i>=lim_sup or x<lim_izq or x>=lim_der)continue;
            put(vacio,x,y,1);
        }
    }
}

void paint() ///dibuja un cuadrado de lado tamanno_cursor (herramienta lápiz)
{
    cursorpos();
    double x1=curx, y1=cury;
    draw_line(x1,curx,y1,cury,1,0);
    if(tamanno_cursor==2)
    {
        draw_line(x1,curx,y1+1,cury+1,1,0);
        draw_line(x1+1,curx+1,y1,cury,1,0);
        draw_line(x1+1,curx+1,y1+1,cury+1,1,0);
    }
    if(tamanno_cursor==3)
    {
        draw_line(x1-1,curx-1,y1,cury,1,0);
        draw_line(x1,curx,y1-1,cury-1,1,0);
        draw_line(x1+1,curx+1,y1,cury,1,0);
        draw_line(x1,curx,y1+1,cury+1,1,0);
    }
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");

    while(1)
    {
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.

        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)
            {
                case KEY_EVENT: // keyboard input
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    char tecla=(char)(irInBuf[i].Event.KeyEvent.wVirtualKeyCode);
                    switch (tecla)
                    {
                    case ENTER:
                        return;
                        break;
                    }
                    break;
                }
                case MOUSE_EVENT: // mouse input
                {
                    cursorpos();
                    bool tmp=mouse_pressed;
                    MouseEventProc(irInBuf[i].Event.MouseEvent);
                    if(mouse_pressed)
                    {
                        draw_line(x1,curx,y1,cury,1,0);
                        if(tamanno_cursor==2)
                        {
                            draw_line(x1,curx,y1+1,cury+1,1,0);
                            draw_line(x1+1,curx+1,y1,cury,1,0);
                            draw_line(x1+1,curx+1,y1+1,cury+1,1,0);
                        }
                        if(tamanno_cursor==3)
                        {
                            draw_line(x1-1,curx-1,y1,cury,1,0);
                            draw_line(x1,curx,y1-1,cury-1,1,0);
                            draw_line(x1+1,curx+1,y1,cury,1,0);
                            draw_line(x1,curx,y1+1,cury+1,1,0);
                        }
                        x1=curx;y1=cury;
                    }
                    if(tmp)
                    {
                        if(!mouse_pressed)
                        {
                            return;
                        }
                    }
                    break;
                }
                case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT:  // disregard focus events

                case MENU_EVENT:   // disregard menu events
                    break;

                default:
                    ErrorExit("Unknown event type");
                    break;
            }
        }
    }
}

void borrar() ///"borra" un cuadrado de lado tamanno_cursor (herramienta goma)
{
    cursorpos();
    double x1=curx, y1=cury;
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");
    erase_line(x1,curx,y1,cury);
    if(tamanno_cursor==2)
    {
        erase_line(x1,curx,y1+1,cury+1);
        erase_line(x1+1,curx+1,y1,cury);
        erase_line(x1+1,curx+1,y1+1,cury+1);
    }
    if(tamanno_cursor==3)
    {
        erase_line(x1-1,curx-1,y1,cury);
        erase_line(x1,curx,y1-1,cury-1);
        erase_line(x1+1,curx+1,y1,cury);
        erase_line(x1,curx,y1+1,cury+1);
    }
    while(1)
    {
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.

        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)
            {
                case KEY_EVENT: // keyboard input
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    char tecla=(char)(irInBuf[i].Event.KeyEvent.wVirtualKeyCode);
                    switch (tecla)
                    {
                    case ENTER:
                        return;
                        break;
                    }
                    break;
                }
                case MOUSE_EVENT: // mouse input
                {
                    cursorpos();
                    bool tmp=mouse_pressed;
                    MouseEventProc(irInBuf[i].Event.MouseEvent);
                    if(mouse_pressed)
                    {
                        erase_line(x1,curx,y1,cury);
                        if(tamanno_cursor==2)
                        {
                            erase_line(x1,curx,y1+1,cury+1);
                            erase_line(x1+1,curx+1,y1,cury);
                            erase_line(x1+1,curx+1,y1+1,cury+1);
                        }
                        if(tamanno_cursor==3)
                        {
                            erase_line(x1-1,curx-1,y1,cury);
                            erase_line(x1,curx,y1-1,cury-1);
                            erase_line(x1+1,curx+1,y1,cury);
                            erase_line(x1,curx,y1+1,cury+1);
                        }
                        x1=curx;y1=cury;
                    }
                    if(tmp)
                    {
                        if(!mouse_pressed)
                        {
                            return;
                        }
                    }
                    break;
                }
                case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT:  // disregard focus events

                case MENU_EVENT:   // disregard menu events
                    break;

                default:
                    ErrorExit("Unknown event type");
                    break;
            }
        }
    }
}

void draw_circle(int x,int y,int tam,bool perm,bool colmap) ///dibuja una circunferencia con centro en esas coordenadas y radio
{
    double eps=0.5;
    for(int i=y-tam-1;i<=y+tam;i++)
    {
        for(int j=x-tam-1;j<=x+tam;j++)
        {
            if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der)continue;
            if(abs(hypot(i-y,j-x)-tam)<eps)
            {
                if(!colmap)put(colors[curcelli][curcellj],j,i,perm);
                else if(dib[i][j]==vacio)put(backcolor,j,i,0);
                else put(dib[i][j],j,i,0);
            }
        }
    }
}

void draw_rectangle(long double x1,long double x2,long double y1,long double y2,bool perm,bool colmap) ///dibuja un rectángulo
{
    draw_line(x1,x2,y1,y1,perm,colmap);
    draw_line(x1,x2,y2,y2,perm,colmap);
    draw_line(x1,x1,y1,y2,perm,colmap);
    draw_line(x2,x2,y1,y2,perm,colmap);
}

void selec() ///herramienta de selección
{
    int x1=curx,y1=cury,x2,y2;

    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");
    while(1)
    {
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.

        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)
            {
                case KEY_EVENT: // keyboard input
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    char tecla=(char)(irInBuf[i].Event.KeyEvent.wVirtualKeyCode);
                    switch (tecla)
                    {
                    case 'c':
                        cop(x1,curx,y1,cury);
                        break;
                    case 'C':
                        cop(x1,curx,y1,cury);
                        break;
                    }
                    break;
                }
                case MOUSE_EVENT: // mouse input
                {
                    bool tmp=mouse_pressed;
                    MouseEventProc(irInBuf[i].Event.MouseEvent);
                    if(mouse_pressed)
                    {
                        draw_interrupted_rectangle(x1,curx,y1,cury,0,1);
                        cursorpos();
                        draw_interrupted_rectangle(x1,curx,y1,cury,0,0);
                    }
                    if(tmp)
                    {
                        if(!mouse_pressed)
                        {
                            x2=curx;
                            y2=cury;
                            if(x1>x2)swap(x1,x2);
                            if(y1>y2)swap(y1,y2);
                            for(int i=y1;i<=y2;i++)
                            {
                                for(int j=x1;j<=x2;j++)
                                {
                                    if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der)sel[i-y1][j-x1]=vacio;
                                    else sel[i-y1][j-x1]=dib[i][j];
                                    dib[i][j]=vacio;
                                }
                            }
                            move_selection(x1,x2,y1,y2);
                            return;
                        }
                    }
                    break;
                }
                case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT:  // disregard focus events

                case MENU_EVENT:   // disregard menu events
                    break;

                default:
                    ErrorExit("Unknown event type");
                    break;
            }
        }
    }
}

void draw_triangle(long double x1,long double x2,long double y1,long double y2,bool perm,bool colmap) ///dibuja un triángulo isósceles
{
    draw_line(x1,x2,y1,y1,perm,colmap);
    draw_line(x1,(x1+x2)/2,y1,y2,perm,colmap);
    draw_line((x1+x2)/2,x2,y2,y1,perm,colmap);
}

void use_spray(int x,int y) ///llena randomizadamente un cuadrado en esas coordenadas(herramienta spray)
{
    for(int i=y-tamanno_cursor-1;i<=y+tamanno_cursor;i++)
    {
        for(int j=x-tamanno_cursor-1;j<=x+tamanno_cursor;j++)
        {
            if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der)continue;
            if(hypot(i-y,j-x)<tamanno_cursor)
            {
                if(rand()%2 and rand()%2)put(colors[curcelli][curcellj],j,i,1);
            }
        }
    }
}

void make_poligon() ///herramienta de creación de un polígono
{
    cursorpos();
    double x1=curx, y1=cury;
    double x0=x1,y0=y1;
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");

    bool flag=0;
    while(1)
    {
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.

        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)
            {
                case KEY_EVENT: // keyboard input
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    char tecla=(char)(irInBuf[i].Event.KeyEvent.wVirtualKeyCode);
                    switch (tecla)
                    {
                    case ENTER:
                        return;
                        break;
                    }
                    break;
                }
                case MOUSE_EVENT: // mouse input
                {
                    cursorpos();
                    bool tmp=mouse_pressed;
                    MouseEventProc(irInBuf[i].Event.MouseEvent);
                    if(mouse_pressed)
                    {
                    }
                    if(tmp)
                    {
                        if(!mouse_pressed)
                        {
                            draw_line(x1,curx,y1,cury,1,0);
                            if(flag and abs(curx-x0<2) and abs(cury-y0)<2){return;}
                            x1=curx;y1=cury;
                            flag=1;
                            break;
                        }
                    }
                    break;
                }
                case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT:  // disregard focus events

                case MENU_EVENT:   // disregard menu events
                    break;

                default:
                    ErrorExit("Unknown event type");
                    break;
            }
        }
    }
}

struct Point2D ///punto en espacio bidimencional
{
    float x;
    float y;
};

void draw_bezier(bool perm,bool colmap) ///dibuja una curva de Bézier
{
    int n=cpx.size();
    double coefx[n],coefy[n];
    if(cpx.size()>2)
    {
        swap(cpx[1],cpx[n-1]);
        swap(cpy[1],cpy[n-1]);
    }
    for(int i=0; i<n; i++)
    {
        coefx[i]=dp[i][n-i-1]*cpx[i];
        coefy[i]=dp[i][n-i-1]*cpy[i];
    }

    int numberOfPoints=10000;
    float dt=1.0/(numberOfPoints-1);
    set<pair<int,int> > curve;
    for(int i=0; i<numberOfPoints; i++)
    {
        double t=i*dt;
        double dcurx=0.0,dcury=0.0;
        for(int j=0; j<n; j++)
        {
            dcurx+=(double)(coefx[j]*pow(1-t,n-j-1)*pow(t,j));
            dcury+=(double)(coefy[j]*pow(1-t,n-j-1)*pow(t,j));
        }
        curve.insert( {dcurx,dcury});
    }
    for(auto i : curve)
    {
        if(i.second<lim_inf or i.second>=lim_sup or i.first<lim_izq or i.first>=lim_der)continue;
        if(!colmap)put(colors[curcelli][curcellj],i.first,i.second,perm);
        else if(dib[i.second][i.first]==vacio)put(backcolor,i.first,i.second,0);
        else put(dib[i.second][i.first],i.first,i.second,0);
    }
    if(cpx.size()>2)
    {
        swap(cpx[1],cpx[n-1]);
        swap(cpy[1],cpy[n-1]);
    }
}

void make_bezier_curve() ///herramienta para hacer una curva de Bézier de "cualquier" grado
{
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];
    cursorpos();
    cpx.push_back(curx);
    cpy.push_back(cury);
    cpx.push_back(curx);
    cpy.push_back(cury);
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");

    while(1)
    {
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.

        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)
            {
                case KEY_EVENT: // keyboard input
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    char tecla=(char)(irInBuf[i].Event.KeyEvent.wVirtualKeyCode);
                    switch (tecla)
                    {
                    case ENTER:
                        draw_bezier(1,0);
                        cpx.clear();
                        cpy.clear();
                        return;
                        break;
                    }
                    break;
                }
                case MOUSE_EVENT: // mouse input
                {
                    bool tmp=mouse_pressed;
                    MouseEventProc(irInBuf[i].Event.MouseEvent);
                    cursorpos();
                    if(!tmp)
                    {
                        if(mouse_pressed)
                        {
                            if(!cpx.empty())
                            {
                                draw_bezier(0,1);
                            }
                            cpx.push_back(curx);
                            cpy.push_back(cury);
                            draw_bezier(0,0);
                        }
                    }
                    else if(mouse_pressed)
                    {
                        if(!cpx.empty())
                            {
                                draw_bezier(0,1);
                                cpx.pop_back();
                                cpy.pop_back();
                            }
                            cpx.push_back(curx);
                            cpy.push_back(cury);
                            draw_bezier(0,0);
                    }
                    break;
                }
                case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT:  // disregard focus events

                case MENU_EVENT:   // disregard menu events
                    break;

                default:
                    ErrorExit("Unknown event type");
                    break;
            }
        }
    }
}

void draw_color_cell(int ci,int cj,int x,int y)
{
    pic_color color_cell=black;
    for(int i=0;i<=tamanno_color;i++)
    {
        put(color_cell,x,y+i,1);
        put(color_cell,x+tamanno_color+1,y+i,1);
        put(color_cell,x+i,y,1);
        put(color_cell,x+i,y+tamanno_color+1,1);
    }
    for(int i=1;i<=tamanno_color;i++)
    {
        for(int j=1;j<=tamanno_color;j++)
        {
            put(colors[ci][cj],x+j,y+i,1);
        }
    }
}

void _buscarcolor()///dibuja sobre la pantalla la paleta de colores
{
    for(int i=0;i<colors.size();i++)
    {
        for(int j=0;j<colors[i].size();j++)
        {
            draw_color_cell(i,j,j*(tamanno_color+1),i*(tamanno_color+1));
        }
    }
    mark_cell(curcelli,curcellj);
}

void buscarcolor()///dibuja sobre la pantalla la paleta de colores
{
    vector<pair<pair<int,int>,int>> v,w;
    ld a=0,b=0,c=0,st=2.23;
    for(ld i=0;i<=255;i+=st)
    {
        a=i;
        v.push_back({{a,b},c});
    }
    for(ld i=st;i<=255;i+=st)
    {
        b=i;
        v.push_back({{a,b},c});
    }
    for(int i=256-st;i>=0;i-=st)
    {
        a=i;
        v.push_back({{a,b},c});
    }
    for(ld i=st;i<=255;i+=st)
    {
        c=i;
        v.push_back({{a,b},c});
    }
    for(ld i=256-st;i>=0;i-=st)
    {
        b=i;
        v.push_back({{a,b},c});
    }
    for(ld i=st;i<=255;i+=st)
    {
        a=i;
        v.push_back({{a,b},c});
    }
    for(ld i=st;i<=255;i+=st)
    {
        b=i;
        v.push_back({{a,b},c});
    }
    while(v.size()>768)v.pop_back();
    int n=v.size();
    gotoxy(0,0);
    setCColor(4);
    int rr=0,cc=0;
    int l=64;
    tamanno_color=5;
    cout<<n;
    colors.clear();
    vector<pic_color> t;
    colors.push_back(t);
    for(auto i : v)
    {
        pic_color col=_bmp_to_pic({i.f.f,i.f.s,i.s});
        colors.back().push_back(col);
//        cout<<i.f.f<<" "<<i.f.s<<" "<<i.s<<"\n";
//        put(col,cc*3,rr*3,0);
//        put(col,cc*3+1,rr*3,0);
//        put(col,cc*3+2,rr*3,0);
//        put(col,cc*3,rr*3+1,0);
//        put(col,cc*3+1,rr*3+1,0);
//        put(col,cc*3+2,rr*3+1,0);
//        put(col,cc*3,rr*3+2,0);
//        put(col,cc*3+1,rr*3+2,0);
//        put(col,cc*3+2,rr*3+2,0);
        cc++;
        if(cc>=l)
        {
            colors.push_back(t);
            rr++;
            cc=0;
        }
    }
    _buscarcolor();
}

void clean_lucida() ///pone el verdadero color de los caracteres lucida (ver arriba bool dirty_lucida)
{
    gotoxy(2000,2000); ///mover la vista de la consola
    gotoxy(0,0); ///y regresar (o solo minimizar y maximizar) arregla la fuente
}

void init(bool dibarea)///reinicializa la interfaz (redibuja y actualiza todo)
{
    ///imprimiendo la interfaz
    system("cls");
    system("color 80");
    ///dibujando area de herramientas
    for(int i=0;i<tool_x.size();i++)
    {
        curx=tool_x[i];cury=tool_y[i];
        pastetools(i);
    }
    ///dibujando paleta de colores
    _buscarcolor();
    ///poniendo area de dibujo
    setCColor(255);
    gotoxy(lim_izq,lim_inf);
//    int t=0;
//    vector<pair<int,int> > randmap;
    if(dibarea)
    {
        for(int i=lim_inf;i<lim_sup;i++)
        {
            for(int j=lim_izq;j<lim_der;j++)
            {
                if(dib[i][j]==vacio)
                {
                    setCColor(255);
                    putchar(178);
                    if(minicar)putchar(178);
                }
                else
                {
                    setCColor(dib[i][j].col);
                    putchar(dib[i][j].car+176);
                    if(minicar)putchar(dib[i][j].car+176);
                }
    //            randmap.push_back({i,j});
    //            t++;
            }
            cout<<"\n";
        }
    }
//    random_shuffle(randmap.begin(),randmap.end());
//    random_shuffle(randmap.begin(),randmap.end());
//    for(int k=0;k<t;k++)
//    {
//        int i=randmap[k].first,j=randmap[k].second;
//        put(dib[i][j],j,i,1);
//    }
    curx=tool_x[cur_tool_cell];cury=tool_y[cur_tool_cell];
    if(cur_tool_cell==2 and is8dir)resalt(tool_s[cur_tool_cell],{0,64});
    else resalt(tool_s[cur_tool_cell],{2,47});
    clean_lucida();
}

struct button ///botón
{
    int izq,der,inf,sup;
    pic_color color;
};

bool button_pressed(button bt) ///sabe si el cursor se encuentra dentro de los límites del botón (bt)
{
    return (curx<=bt.der and curx>=bt.izq and cury<=bt.sup and cury>=bt.inf);
}

void imp_button(button bt) ///imprime el botón (bt)
{
    for(int i=bt.inf;i<=bt.sup;i++)
    {
        for(int j=bt.izq;j<=bt.der;j++)
        {
            put(bt.color,j,i,0);
        }
    }
}

void save_variations(bool reflect);

void save() ///abre la ventana de guardado
{
    saved=0;
//    int inf_sal=70,izq_sal=85,der_sal=370,sup_sal=90;
    int inf_sal=163,izq_sal=193,der_sal=478,sup_sal=183;
    if(!minicar)
    {
        inf_sal=70;izq_sal=85;der_sal=370;sup_sal=90;
    }
    char* name;
    button text_box={izq_sal+1,der_sal-2,inf_sal+1,inf_sal+10,{2,255}}; ///no es realmente un botón pero soy vago
    button guardar={izq_sal+1,izq_sal+50,sup_sal-7,sup_sal-1,{0,8}};
    button cancelar={izq_sal+199,izq_sal+255,sup_sal-7,sup_sal-1,{0,8}};
    curx=izq_sal;cury=inf_sal;
    paste_out("savewindow.txt",0);
    ///estableciendo modo de consola de entrada por mouse y ventana
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");
    ///ciclo de recibimiento de entrada por mouse y teclado
    while(1)
    {
        ///recibiendo buffer de entrada
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.
        ///procesando buffer de entrada
        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)///dependiendo del tipo de entrada
            {
                case KEY_EVENT: /// entrada por teclado
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    char tecla=(char)(irInBuf[i].Event.KeyEvent.wVirtualKeyCode); ///toma la tecla presionada
                    if(key_pressed)
                    {
                        switch (tecla)
                        {
                        case ENTER:
                            break;
                        }
                        break;
                    }
                }
                case MOUSE_EVENT: /// entrada por mouse
                {
                    cursorpos(); ///actualiza las variables de posicion del cursor
                    bool tmp=mouse_pressed; ///guarda si es mouse esta presionado
                    MouseEventProc(irInBuf[i].Event.MouseEvent); ///procesa la entrada
                    if(tmp and !mouse_pressed)
                    {
                        if(button_pressed(guardar))
                        {
                            opened_file=name;
                            save_variations(0);
                            if(b_reflect)save_variations(1);
                            saved=1;
                            return;
                        }
                        else if(button_pressed(text_box))
                        {
                            imp_button(text_box);
                            impout=1;
                            name=string_to_charaster(read(text_box.inf+1,text_box.izq+1,10,black,0,0));
                            impout=0;
                        }
                        else if(button_pressed(cancelar))
                        {
                            cancel=1;
                            if(lim_der<der_sal or lim_sup<sup_sal)init(1);
                            else
                            for(int i=inf_sal;i<sup_sal;i++)
                            {
                                for(int j=izq_sal;j<der_sal;j++)
                                {
                                    if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der)continue;
                                    put(dib[i][j],j,i,0);
                                }
                            }
                            return;
                        }
                    }
                    break;
                }
                case WINDOW_BUFFER_SIZE_EVENT: ///cambio de tamaño de la ventana
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT: ///descartar

                case MENU_EVENT: ///descartar
                    break;

                default: ///evento desconocido
                    ErrorExit("Unknown event type"); ///salida de error
                    break;
            }
        }
    }
}

void try_exit() ///abre la ventana de salida
{
//    int inf_sal=70,izq_sal=85,der_sal=370,sup_sal=90;
    int inf_sal=163,izq_sal=193,der_sal=478,sup_sal=183;
    if(!minicar)
    {
        inf_sal=70;izq_sal=85;der_sal=370;sup_sal=90;
    }
    button guardar={izq_sal+1,izq_sal+50,sup_sal-7,sup_sal-1,{0,8}};
    button no_guardar={izq_sal+89,izq_sal+160,sup_sal-7,sup_sal-1,{0,8}};
    button cancelar={izq_sal+199,izq_sal+255,sup_sal-7,sup_sal-1,{0,8}};
    curx=izq_sal;cury=inf_sal;
    paste_out("exitwindow.txt",0);
    ///estableciendo modo de consola de entrada por mouse y ventana
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");
    ///ciclo de recibimiento de entrada por mouse y teclado
    while(1)
    {
        ///recibiendo buffer de entrada
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.
        ///procesando buffer de entrada
        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)///dependiendo del tipo de entrada
            {
                case KEY_EVENT: /// entrada por teclado
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    char tecla=(char)(irInBuf[i].Event.KeyEvent.wVirtualKeyCode); ///toma la tecla presionada
                    if(key_pressed){
                    switch (tecla)
                    {
                    case ENTER:
                        break;
                    }
                    break;
                    }
                }
                case MOUSE_EVENT: /// entrada por mouse
                {
                    cursorpos(); ///actualiza las variables de posicion del cursor
                    bool tmp=mouse_pressed; ///guarda si es mouse esta presionado
                    MouseEventProc(irInBuf[i].Event.MouseEvent); ///procesa la entrada
                    if(tmp and !mouse_pressed)
                    {
                        if(button_pressed(guardar))
                        {
                            save();
                            if(saved)
                            {
                                exit(0);
                                system("cls");
//                                system("exit");
                            }
                            curx=izq_sal;cury=inf_sal;
                            paste_out("exitwindow.txt",0);
                        }
                        else if(button_pressed(no_guardar))
                        {
                            exit(0);
                            system("cls");
//                            system("exit");
                        }
                        else if(button_pressed(cancelar))
                        {
                            if(lim_der<der_sal or lim_sup<sup_sal)init(1);
                            else
                            for(int i=inf_sal;i<sup_sal;i++)
                            {
                                for(int j=izq_sal;j<der_sal;j++)
                                {
                                    if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der)continue;
                                    put(dib[i][j],j,i,0);
                                }
                            }
                            return;
                        }
                    }
                    break;
                }
                case WINDOW_BUFFER_SIZE_EVENT: ///cambio de tamaño de la ventana
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT: ///descartar

                case MENU_EVENT: ///descartar
                    break;

                default: ///evento desconocido
                    ErrorExit("Unknown event type"); ///salida de error
                    break;
            }
        }
    }
    for(int i=inf_sal;i<sup_sal;i++)
    {
        for(int j=izq_sal;j<der_sal;j++)
        {
            put(dib[i][j],j,i,0);
        }
    }
}

void savedoc() ///abre la ventana de salvar el archivo abierto (si lo hay)
{
//    int inf_sal=70,izq_sal=85,der_sal=370,sup_sal=90;
    int inf_sal=163,izq_sal=193,der_sal=478,sup_sal=183;
    if(!minicar)
    {
        inf_sal=70;izq_sal=85;der_sal=370;sup_sal=90;
    }
    button guardar={izq_sal+1,izq_sal+50,sup_sal-7,sup_sal-1,{0,8}};
    button no_guardar={izq_sal+89,izq_sal+160,sup_sal-7,sup_sal-1,{0,8}};
    button cancelar={izq_sal+199,izq_sal+255,sup_sal-7,sup_sal-1,{0,8}};
    curx=izq_sal;cury=inf_sal;
    paste_out("savedoc.txt",0);
    ///estableciendo modo de consola de entrada por mouse y ventana
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");
    ///ciclo de recibimiento de entrada por mouse y teclado
    while(1)
    {
        ///recibiendo buffer de entrada
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.
        ///procesando buffer de entrada
        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)///dependiendo del tipo de entrada
            {
                case KEY_EVENT: /// entrada por teclado
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    char tecla=(char)(irInBuf[i].Event.KeyEvent.wVirtualKeyCode); ///toma la tecla presionada
                    if(key_pressed){
                    switch (tecla)
                    {
                    case ENTER:
                        break;
                    }
                    break;
                    }
                }
                case MOUSE_EVENT: /// entrada por mouse
                {
                    cursorpos(); ///actualiza las variables de posicion del cursor
                    bool tmp=mouse_pressed; ///guarda si es mouse esta presionado
                    MouseEventProc(irInBuf[i].Event.MouseEvent); ///procesa la entrada
                    if(tmp and !mouse_pressed)
                    {
                        if(button_pressed(guardar))
                        {
                            curx=izq_sal;cury=inf_sal;
                            ofstream f(opened_file);
                            f<<lim_sup-lim_inf<<" "<<lim_der-lim_izq;
                            for(int i=lim_inf;i<lim_sup;i++)
                            {
                                f<<"\n";
                                for(int j=lim_izq;j<lim_der;j++)
                                {
                                    string a=fancy(dib[i][j].car,2),b=fancy(dib[i][j].col,2);
                                    f<<a<<" "<<b<<" ";
                                }
                            }
                            f.close();
                            return;
                        }
                        else if(button_pressed(no_guardar))return;
                        else if(button_pressed(cancelar))
                        {
                            cancel=1;
                            if(lim_der<der_sal or lim_sup<sup_sal)init(1);
                            else
                            for(int i=inf_sal;i<sup_sal;i++)
                            {
                                for(int j=izq_sal;j<der_sal;j++)
                                {
                                    if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der)continue;
                                    put(dib[i][j],j,i,0);
                                }
                            }
                            return;
                        }
                    }
                    break;
                }
                case WINDOW_BUFFER_SIZE_EVENT: ///cambio de tamaño de la ventana
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT: ///descartar

                case MENU_EVENT: ///descartar
                    break;

                default: ///evento desconocido
                    ErrorExit("Unknown event type"); ///salida de error
                    break;
            }
        }
    }
}

void open() ///abre la ventana de abrir archivo
{
//    int inf_sal=70,izq_sal=85,der_sal=370,sup_sal=90;
    int inf_sal=163,izq_sal=193,der_sal=478,sup_sal=183;
    if(!minicar)
    {
        inf_sal=70;izq_sal=85;der_sal=370;sup_sal=90;
    }
    char* name;
    button text_box={izq_sal+1,der_sal-2,inf_sal+1,inf_sal+10,{2,255}}; ///no es realmente un botón pero soy vago
    button abrir={izq_sal+1,izq_sal+50,sup_sal-7,sup_sal-1,{0,8}};
    button cancelar={izq_sal+199,izq_sal+255,sup_sal-7,sup_sal-1,{0,8}};
    curx=izq_sal;cury=inf_sal;
    paste_out("openwindow.txt",0);
    ///estableciendo modo de consola de entrada por mouse y ventana
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");
    ///ciclo de recibimiento de entrada por mouse y teclado
    while(1)
    {
        ///recibiendo buffer de entrada
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.
        ///procesando buffer de entrada
        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)///dependiendo del tipo de entrada
            {
                case KEY_EVENT: /// entrada por teclado
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    char tecla=(char)(irInBuf[i].Event.KeyEvent.wVirtualKeyCode); ///toma la tecla presionada
                    if(key_pressed)
                    {
                        switch (tecla)
                        {
                        case ENTER:
                            break;
                        }
                        break;
                    }
                }
                case MOUSE_EVENT: /// entrada por mouse
                {
                    cursorpos(); ///actualiza las variables de posicion del cursor
                    bool tmp=mouse_pressed; ///guarda si es mouse esta presionado
                    MouseEventProc(irInBuf[i].Event.MouseEvent); ///procesa la entrada
                    if(tmp and !mouse_pressed)
                    {
                        if(button_pressed(abrir))
                        {
                            opened_file=name;
                            FILE* f=fopen(name,"r");
                            clean_dibarea(-1,-1);
                            fscanf(f,"%d%d",&lim_sup,&lim_der);
                            lim_sup+=lim_inf;
                            lim_der+=lim_izq;
                            for(int i=lim_inf;i<lim_sup;i++)
                            {
                                for(int j=lim_izq;j<lim_der;j++)
                                {
                                    fscanf(f,"%d%d",&dib[i][j].car,&dib[i][j].col);
//                                    put(dib[i][j],j,i,0);
                                }
                            }
                            fclose(f);
                            file_opened=1;
                            return;
                        }
                        else if(button_pressed(text_box))
                        {
                            imp_button(text_box);
                            impout=1;
                            name=string_to_charaster(read(text_box.inf+1,text_box.izq+1,10,black,0,0)+".txt");
                            impout=0;
                        }
                        else if(button_pressed(cancelar))
                        {
                            cancel=1;
                            if(lim_der<der_sal or lim_sup<sup_sal)init(1);
                            else
                            for(int i=inf_sal;i<sup_sal;i++)
                            {
                                for(int j=izq_sal;j<der_sal;j++)
                                {
                                    if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der)continue;
                                    put(dib[i][j],j,i,0);
                                }
                            }
                            return;
                        }
                    }
                    break;
                }
                case WINDOW_BUFFER_SIZE_EVENT: ///cambio de tamaño de la ventana
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT: ///descartar

                case MENU_EVENT: ///descartar
                    break;

                default: ///evento desconocido
                    ErrorExit("Unknown event type"); ///salida de error
                    break;
            }
        }
    }
}

void herrant(int ant) ///macro para regresar a la herramienta anterior luego de usar exit,open o save
{
    curx=tool_x[cur_tool_cell];
    cury=tool_y[cur_tool_cell];
    unresalt(tool_s[cur_tool_cell]); ///desmarca la herramienta anterior
    cur_tool_cell=ant;
    curx=tool_x[cur_tool_cell];
    cury=tool_y[cur_tool_cell];
    if(cur_tool_cell==2)resalt(tool_s[cur_tool_cell],{0,64});
    else resalt(tool_s[cur_tool_cell],{0,47}); ///marca la herramienta seleccionada
}

void draw_pentalfa(long double x1,long double x2,long double y1,long double y2,bool perm,bool colmap) ///dibuja una pentalfa
{
    if(x1>x2)swap(x1,x2);
    if(y1>y2)swap(y1,y2);
    double p1x,p2x,p3x,p4x,p5x;
    double p1y,p2y,p3y,p4y,p5y;
    p1x=(x1+x2)/2.0;p1y=y1;
    p2x=x2;p2y=(y2-y1)/5.0*2.0+y1;
    p3x=((x2-x1)/5.0)*4.0+x1;p3y=y2;
    p4x=(x2-x1)/5.0+x1;p4y=y2;
    p5x=x1;p5y=(y2-y1)/5.0*2.0+y1;
    draw_line(p4x,p1x,p4y,p1y,perm,colmap);
    draw_line(p1x,p3x,p1y,p3y,perm,colmap);
    draw_line(p3x,p5x,p3y,p5y,perm,colmap);
    draw_line(p5x,p2x,p5y,p2y,perm,colmap);
    draw_line(p2x,p4x,p2y,p4y,perm,colmap);
}

void draw_pentagon(long double x1,long double x2,long double y1,long double y2,bool perm,bool colmap) ///dibuja un pentágono
{
    if(x1>x2)swap(x1,x2);
    if(y1>y2)swap(y1,y2);
    double p1x,p2x,p3x,p4x,p5x;
    double p1y,p2y,p3y,p4y,p5y;
    p1x=(x1+x2)/2.0;p1y=y1;
    p2x=x2;p2y=(y2-y1)/5.0*2.0+y1;
    p3x=((x2-x1)/5.0)*4.0+x1;p3y=y2;
    p4x=(x2-x1)/5.0+x1;p4y=y2;
    p5x=x1;p5y=(y2-y1)/5.0*2.0+y1;
    draw_line(p1x,p2x,p1y,p2y,perm,colmap);
    draw_line(p2x,p3x,p2y,p3y,perm,colmap);
    draw_line(p3x,p4x,p3y,p4y,perm,colmap);
    draw_line(p4x,p5x,p4y,p5y,perm,colmap);
    draw_line(p5x,p1x,p5y,p1y,perm,colmap);
}

void draw_heart(long double x1,long double x2,long double y1,long double y2,bool perm,bool colmap) ///dibuja un corazón
{
    if(x1>x2)swap(x1,x2);
    if(y1>y2)swap(y1,y2);
    double yv=(y2-y1)/4.0+y1,xv=(x2+x1)/2.0;
    double yi=y2,xi=xv;
    cpx.clear();
    cpy.clear();
    ///una forma
//    cpx.push_back(xi);
//    cpx.push_back(xv);
//    cpx.push_back(x1-(x2-x1)/4.0);
//    cpx.push_back(x1-(x2-x1)/1.5);
//    cpy.push_back(yi);
//    cpy.push_back(yv);
//    cpy.push_back(y1-(y2-y1)/4.0);
//    cpy.push_back(y2-(y2-y1)/2.0);
//    draw_bezier(perm,colmap);
//    cpx.clear();
//    cpy.clear();
//    cpx.push_back(xi);
//    cpx.push_back(xv);
//    cpx.push_back(x2+(x2-x1)/4.0);
//    cpx.push_back(x2+(x2-x1)/1.5);
//    cpy.push_back(yi);
//    cpy.push_back(yv);
//    cpy.push_back(y1-(y2-y1)/4.0);
//    cpy.push_back(y2-(y2-y1)/2.0);
//    draw_bezier(perm,colmap);
//    cpx.clear();
//    cpy.clear();
    ///otra forma
    cpx.push_back(x1);
    cpx.push_back(xv-1);
    cpx.push_back(x1+(xv-x1)/2.0);
    cpx.push_back(xv);
    cpx.push_back(x1);
    cpy.push_back(yv);
    cpy.push_back(yv);
    cpy.push_back(y1-(y2-y1)/4.0);
    cpy.push_back(y1+(yv-y1)/1.5);
    cpy.push_back(y1+(yv-y1)/1.5);
    draw_bezier(perm,colmap);
    cpx.clear();
    cpy.clear();
    cpx.push_back(xv-1);
    cpx.push_back(x2);
    cpx.push_back(xv+(x2-xv)/2.0);
    cpx.push_back(x2);
    cpx.push_back(xv);
    cpy.push_back(yv);
    cpy.push_back(yv);
    cpy.push_back(y1-(y2-y1)/4.0);
    cpy.push_back(y1+(yv-y1)/1.5);
    cpy.push_back(y1+(yv-y1)/1.5);
    draw_bezier(perm,colmap);
    cpx.clear();
    cpy.clear();
    cpx.push_back(xi);
    cpx.push_back(x2);
    cpx.push_back(x2);
    cpy.push_back(yi);
    cpy.push_back(yv);
    cpy.push_back(yv+(y2-yv)/1.5);
    draw_bezier(perm,colmap);
    cpx.clear();
    cpy.clear();
    cpx.push_back(xi);
    cpx.push_back(x1);
    cpx.push_back(x1);
    cpy.push_back(yi);
    cpy.push_back(yv);
    cpy.push_back(yv+(y2-yv)/1.5);
    draw_bezier(perm,colmap);
    cpx.clear();
    cpy.clear();
}

void make_figure() ///herramienta multifunción para varias figuras
{
    int x1=curx,y1=cury;
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");

    while(1)
    {
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.

        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)
            {
                case KEY_EVENT: // keyboard input
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    char tecla=(char)(irInBuf[i].Event.KeyEvent.wVirtualKeyCode);
                    switch (tecla)
                    {
                    case ENTER:
                        if(cur_tool_cell==5)draw_line(x1,curx,y1,cury,1,0);
                        else if(cur_tool_cell==7)draw_rectangle(x1,curx,y1,cury,1,0);
                        else if(cur_tool_cell==9)draw_triangle(x1,curx,y1,cury,1,0);
                        else if(cur_tool_cell==16)draw_pentalfa(x1,curx,y1,cury,1,0);
                        else if(cur_tool_cell==17)draw_pentagon(x1,curx,y1,cury,1,0);
                        else if(cur_tool_cell==18)draw_heart(x1,curx,y1,cury,1,0);
                        return;
                        break;
                    }
                    break;
                }
                case MOUSE_EVENT: // mouse input
                {
                    bool tmp=mouse_pressed;
                    MouseEventProc(irInBuf[i].Event.MouseEvent);
                    if(mouse_pressed)
                    {
                        if(cur_tool_cell==5){draw_line(x1,curx,y1,cury,0,1);cursorpos();draw_line(x1,curx,y1,cury,0,0);}
                        else if(cur_tool_cell==7){draw_rectangle(x1,curx,y1,cury,0,1);cursorpos();draw_rectangle(x1,curx,y1,cury,0,0);}
                        else if(cur_tool_cell==9){draw_triangle(x1,curx,y1,cury,0,1);cursorpos();draw_triangle(x1,curx,y1,cury,0,0);}
                        else if(cur_tool_cell==16){draw_pentalfa(x1,curx,y1,cury,0,1);cursorpos();draw_pentalfa(x1,curx,y1,cury,0,0);}
                        else if(cur_tool_cell==17){draw_pentagon(x1,curx,y1,cury,0,1);cursorpos();draw_pentagon(x1,curx,y1,cury,0,0);}
                        else if(cur_tool_cell==18){draw_heart(x1,curx,y1,cury,0,1);cursorpos();draw_heart(x1,curx,y1,cury,0,0);}
                    }
                    if(tmp)
                    {
                        if(!mouse_pressed)
                        {
                            if(cur_tool_cell==5)draw_line(x1,curx,y1,cury,1,0);
                            else if(cur_tool_cell==7)draw_rectangle(x1,curx,y1,cury,1,0);
                            else if(cur_tool_cell==9)draw_triangle(x1,curx,y1,cury,1,0);
                            else if(cur_tool_cell==16)draw_pentalfa(x1,curx,y1,cury,1,0);
                            else if(cur_tool_cell==17)draw_pentagon(x1,curx,y1,cury,1,0);
                            else if(cur_tool_cell==18)draw_heart(x1,curx,y1,cury,1,0);
                            return;
                        }
                    }
                    break;
                }
                case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT:  // disregard focus events

                case MENU_EVENT:   // disregard menu events
                    break;

                default:
                    ErrorExit("Unknown event type");
                    break;
            }
        }
    }
}

///Implementación del "Juego de la Vida" de Conway.
typedef pair<int,int> cell; ///una célula como un par de números
#define x first
#define y second

set<cell> living_cells; ///células vivas

void play() ///reanuda el juego
{
    int vel=70;
    while(1)
    {
        if(kbhit())
        {
            char ch=getch();
            if(ch==ABAJO)vel-=5;
            else if(ch==ARRIBA)vel+=5;
//            else if(ch==IZQUIERDA)return;
            else if(ch==ESCAPE)return;
            if(vel>100)vel=100;
        }
        Sleep(100-vel);
        vector<cell> kill;
        vector<cell> revive;
        set<cell> potencially_living;
        for(auto i : living_cells)
        {
            int c=0,x=i.x,y=i.y;
            for(int k=0;k<8;k++)
            {
                int nx=x+dc[k],ny=y+dr[k];
                if(nx<lim_izq or nx>=lim_der or ny<lim_inf or ny>=lim_sup)continue;
                if(game[ny][nx])c++;
                else potencially_living.insert({nx,ny});
            }
            if(c!=2 and c!=3)kill.push_back(i);
        }
        for(auto i : potencially_living)
        {
            int c=0,x=i.x,y=i.y;
            for(int k=0;k<8;k++)
            {
                int nx=x+dc[k],ny=y+dr[k];
                if(nx<lim_izq or nx>=lim_der or ny<lim_inf or ny>=lim_sup)continue;
                if(game[ny][nx])c++;
            }
            if(c==3)
            {
                revive.push_back(i);
            }
        }
        for(auto i : kill)
        {
            game[i.y][i.x]=0;
            living_cells.erase(i);
            put(black,i.x,i.y,1);
        }
        for(auto i : revive)
        {
            int x=i.x,y=i.y;
            game[y][x]=1;
            living_cells.insert(i);
            map<pic_color,int> m;
            for(int k=0;k<8;k++)
            {
                int nx=x+dc[k],ny=y+dr[k];
                if(nx<lim_izq or nx>=lim_der or ny<lim_inf or ny>=lim_sup)continue;
                if(game[ny][nx]>0)m[dib[ny][nx]]++;
            }
            pic_color col=black;
            m[col]=0;
            for(auto i : m)
            {
                if(i.y>m[col])col=i.x;
            }
            put(col,x,y,1);
        }
    }
}

void prep_colors() ///preprocesamiento de los colores bmp a pic
{
    for(auto i : old_colors)
    {
        vector<pic_color> v;
        for(auto j : i)
        {
            v.push_back({j/100,j%100});
        }
        colors.push_back(v);
    }
    colors[0][0]=black;
    colors[4][28]={2,255};
    ifstream f("in.txt");
//    freopen("in.txt","r",stdin);
    for(int i=0;i<15;i++)
    {
        for(int j=0;j<768;j++)
        {
            f>>mat[i][j].b;
            f>>mat[i][j].g;
            f>>mat[i][j].r;
        }
    }
    vector<pair<bmp_color,pic_color> > colist;
    for(int i=0;i<3;i++)
    {
        for(int j=0;j<256;j++)
        {
            vector<bmp_color> v;
            for(int k=0;k<5;k++)
            {
                for(int l=0;l<3;l++)
                {
                    v.push_back(mat[i*5+k][j*3+l]);
                }
            }
            bmp_color p=promedio(v);
            colist.push_back({p,{i,j}});
            pic_to_bmp[i][j]=p;
        }
    }
//    sort(colist.begin(),colist.end());
//    for(auto i : colist)
//    {
//        setCColor(i.y.col);
//        putchar(i.y.car+176);
//    }
//    exit(0);
    f.close();
}

typedef struct bmpFileHeader ///encabezado de un bmp
{
    /* 2 bytes de identificación */
    uint32_t size;        /* Tamaño del archivo */
    uint16_t resv1;       /* Reservado */
    uint16_t resv2;       /* Reservado */
    uint32_t offset;      /* Offset hasta hasta los datos de imagen */
} bmpFileHeader;
bmpFileHeader header;

typedef struct bmpInfoHeader ///informacion de un bmp
{
    uint32_t headersize;      /* Tamaño de la cabecera */
    uint32_t width;       /* Ancho */
    uint32_t height;      /* Alto */
    uint16_t planes;          /* Planos de color (Siempre 1) */
    uint16_t bpp;             /* bits por pixel */
    uint32_t compress;        /* compresión */
    uint32_t imgsize;     /* tamaño de los datos de imagen */
    uint32_t bpmx;        /* Resolución X en bits por metro */
    uint32_t bpmy;        /* Resolución Y en bits por metro */
    uint32_t colors;      /* colors used en la paleta */
    uint32_t imxtcolors;      /* Colores importantes. 0 si son todos */
} bmpInfoHeader;

unsigned char *LoadBMP(char *filename, bmpInfoHeader *bInfoHeader);
void DisplayInfo(bmpInfoHeader *info);
void TextDisplay(bmpInfoHeader *info, unsigned char *img);

void guardaBMP(char *filename, bmpInfoHeader info, string img) ///guarda el bmp que representa header, info e img
{
    ofstream f(filename);
    uint16_t type=0x4D42;        /* 2 bytes identificativos */

    /* Leemos los dos primeros bytes */
    f.write((char*)&type, sizeof(uint16_t));
    /* Leemos la cabecera de fichero completa */
//    fwrite((char*)header, sizeof(bmpFileHeader), 1, f);
    f.write((char*)&header.size, sizeof(uint32_t));
    f.write((char*)&header.resv1, sizeof(uint16_t));
    f.write((char*)&header.resv2, sizeof(uint16_t));
    f.write((char*)&header.offset, sizeof(uint32_t));

    /* Leemos la cabecera de información completa */
//    fwrite((char*)info, sizeof(bmpInfoHeader), 1, f);
    f.write((char*)&info.headersize, sizeof(uint32_t));
    f.write((char*)&info.width, sizeof(uint32_t));
    f.write((char*)&info.height, sizeof(uint32_t));
    f.write((char*)&info.planes, sizeof(uint16_t));
    f.write((char*)&info.bpp, sizeof(uint16_t));
    f.write((char*)&info.compress, sizeof(uint32_t));
    f.write((char*)&info.imgsize, sizeof(uint32_t));
    f.write((char*)&info.bpmx, sizeof(uint32_t));
    f.write((char*)&info.bpmy, sizeof(uint32_t));
    f.write((char*)&info.colors, sizeof(uint32_t));
    f.write((char*)&info.imxtcolors, sizeof(uint32_t));
    /* Nos situamos en el sitio donde empiezan los datos de imagen,
     nos lo indica el offset de la cabecera de fichero*/
//    f.seek(header.offset, SEEK_SET);
//    f<<img;
//    f.write((char*)img, info.imgsize);
    for(int i=0;i<info.imgsize;i++)
    {
        if(img[i]=='\n')
        {
            i++;
            f<<'\n';
        }
        else f<<(img[i]);
    }
//    for(int i=0;i<info.imgsize;i++)cout<<(img[i]);
    /* Leemos los datos de imagen, tantos bytes como imgsize */
//    f.write((char*)&img, sizeof(img));
//    f.write((char*)&img, info.imgsize);
    f.close();
    return;

    /* Cerramos */

}

void guardaBMP(char *filename, bmpInfoHeader info, char *img) ///guarda el bmp que representa header, info e img
{
    ofstream f(filename);
    uint16_t type=0x4D42;        /* 2 bytes identificativos */

    /* Leemos los dos primeros bytes */
    f.write((char*)&type, sizeof(uint16_t));
    /* Leemos la cabecera de fichero completa */
//    fwrite((char*)header, sizeof(bmpFileHeader), 1, f);
    f.write((char*)&header.size, sizeof(uint32_t));
    f.write((char*)&header.resv1, sizeof(uint16_t));
    f.write((char*)&header.resv2, sizeof(uint16_t));
    f.write((char*)&header.offset, sizeof(uint32_t));

    /* Leemos la cabecera de información completa */
//    fwrite((char*)info, sizeof(bmpInfoHeader), 1, f);
    f.write((char*)&info.headersize, sizeof(uint32_t));
    f.write((char*)&info.width, sizeof(uint32_t));
    f.write((char*)&info.height, sizeof(uint32_t));
    f.write((char*)&info.planes, sizeof(uint16_t));
    f.write((char*)&info.bpp, sizeof(uint16_t));
    f.write((char*)&info.compress, sizeof(uint32_t));
    f.write((char*)&info.imgsize, sizeof(uint32_t));
    f.write((char*)&info.bpmx, sizeof(uint32_t));
    f.write((char*)&info.bpmy, sizeof(uint32_t));
    f.write((char*)&info.colors, sizeof(uint32_t));
    f.write((char*)&info.imxtcolors, sizeof(uint32_t));
    /* Nos situamos en el sitio donde empiezan los datos de imagen,
     nos lo indica el offset de la cabecera de fichero*/
//    f.seek(header.offset, SEEK_SET);
//    f<<img;
//    f.write((char*)img, info.imgsize);
    for(int i=0;i<info.imgsize;i++)
    {
        if(img[i]=='\n')
        {
            i++;
            f<<'\n';
        }
        else f<<(img[i]);
    }
//    for(int i=0;i<info.imgsize;i++)cout<<(img[i]);
    /* Leemos los datos de imagen, tantos bytes como imgsize */
//    f.write((char*)&img, sizeof(img));
//    f.write((char*)&img, info.imgsize);
    f.close();
    return;

    /* Cerramos */

}

void TextDisplay(bmpInfoHeader *info, unsigned char *img) ///imprime el bmp en la consola como todas sus componentes RGB
{
    freopen("in.txt","w",stdout);
    int x, y;
    /* Reducimos la resolución vertical y horizontal para que la imagen entre en pantalla */
    static const int reduccionX=1, reduccionY=1;
    /* Si la componente supera el umbral, el color se marcará como 1. */
    static const int umbral=90;
    /* Asignamos caracteres a los colores en pantalla */
    static unsigned char colores[9]=" bgfrRGB";
    int r,g,b;

    /* Dibujamos la imagen */
    for (y=info->height; y>0; y-=reduccionY)
    {
        for (x=0; x<info->width; x+=reduccionX)
        {
            b=(img[3*(x+y*info->width)]);
            g=(img[3*(x+y*info->width)+1]);
            r=(img[3*(x+y*info->width)+2]);
            cout<<b<<" "<<g<<" "<<r<<"\n";
        }
    }
}

unsigned char *LoadBMP(char *filename, bmpInfoHeader *bInfoHeader) ///lee un bmp y lo almacena en memoria
{

    FILE *f;
    unsigned char *imgdata;   /* datos de imagen */
    uint16_t type;        /* 2 bytes identificativos */

    f=fopen (filename, "rb");
    if (!f)
        return NULL;        /* Si no podemos leer, no hay imagen*/

    /* Leemos los dos primeros bytes */
    fread(&type, 1, sizeof(uint16_t), f);
    if (type !=0x4D42)        /* Comprobamos el formato */
    {
        fclose(f);
        return NULL;
    }

    /* Leemos la cabecera de fichero completa */
    fread(&header, 1, sizeof(bmpFileHeader), f);

    /* Leemos la cabecera de información completa */
    fread(bInfoHeader, 1, sizeof(bmpInfoHeader), f);

    /* Reservamos memoria para la imagen, ¿cuánta?
       Tanto como indique imgsize */
//    if(bInfoHeader->imgsize==0)bInfoHeader->imgsize=bInfoHeader->height*bInfoHeader->width*3;
    bInfoHeader->imgsize=bInfoHeader->height*bInfoHeader->width*3;
    imgdata=(unsigned char*)malloc(bInfoHeader->imgsize);

    /* Nos situamos en el sitio donde empiezan los datos de imagen,
     nos lo indica el offset de la cabecera de fichero*/
    fseek(f, header.offset, SEEK_SET);
    /* Leemos los datos de imagen, tantos bytes como imgsize */
    fread(imgdata,1, bInfoHeader->imgsize, f);
    /* Cerramos */
    fclose(f);

    /* Devolvemos la imagen */
    return imgdata;
}

void DisplayInfo(bmpInfoHeader *info) ///imprime la información del bmp en la consola
{
    printf("Tamaño de la cabecera: %u\n", info->headersize);
    printf("Anchura: %d\n", info->width);
    printf("Altura: %d\n", info->height);
    printf("Planos (1): %d\n", info->planes);
    printf("Bits por pixel: %d\n", info->bpp);
    printf("Compresión: %d\n", info->compress);
    printf("Tamaño de datos de imagen: %u\n", info->imgsize);
    printf("Resolucón horizontal: %u\n", info->bpmx);
    printf("Resolucón vertical: %u\n", info->bpmy);
    printf("Colores en paleta: %d\n", info->colors);
    printf("Colores importantes: %d\n", info->imxtcolors);
}

vector<vector<bmp_color> > to_bmp_image(bmpInfoHeader *info, unsigned char *img) ///mete un bmp en una matriz de bmp_color
{
    static const int reduccionX=1, reduccionY=1;
    vector<vector<bmp_color> > I;

    for (int y=info->height; y>0; y-=reduccionY)
    {
        vector<bmp_color> v;
        for (int x=0; x<info->width; x+=reduccionX)
        {
            bmp_color c;
            c.b=(img[3*(x+y*info->width)]);
            c.g=(img[3*(x+y*info->width)+1]);
            c.r=(img[3*(x+y*info->width)+2]);
            v.push_back(c);
        }
        I.push_back(v);
    }
    return I;
}

void paste_bmp(char* name,int i,int j,bool out) ///imprime un bmp en la consola con mis colores representables
{
    bmpInfoHeader info;
    unsigned char *img;
    img=LoadBMP(name, &info);
//        for(int i=0;;i++)
//        {
//
//        }
//    return;
    static const int reduccionX=1, reduccionY=1;
    for (int y=info.height-1; y>=0; y-=reduccionY)
    {
        for (int x=0; x<info.width; x+=reduccionX)
        {
//            cout<<3*(x+y*info.width)<<"\n";
            bmp_color c;
            c.b=(img[3*(x+y*info.width)]);
            c.g=(img[3*(x+y*info.width)+1]);
            c.r=(img[3*(x+y*info.width)+2]);
            pic_color a=bmp_to_pic(c);
            if(out or (x+j>=lim_izq and x+j<lim_der and i+info.height-y-1>=lim_inf and i+info.height-y-1<lim_sup))
            {
                if(b_microsoft_paint)dib[i+info.height-y-1][x+j]=a;
                else put(a,x+j,i+info.height-y-1,1);
            }
//            put(a.car,a.col,x+j,y+i,0);
        }
//        cout<<"\n";
    }
//    vector<vector<bmp_color> > I=to_bmp_image(&info,img);
//    for(int i=0;i<I.size();i++)
//    {
//        for(int j=0;j<I[0].size();j++)
//        {
//            pic_color a=bmp_to_pic(I[i][j]);
//            put(a.car,a.col,x+j,y+i,0);
//        }
//    }
}

int variate_rgb(int a)
{
    int b=rand()%noise_variation_range;
    if(rand()%2)b=-b;
    a+=b;
    if(a<0)a=0;
    else if(a>255)a=255;
    return a;
}

void save_variations(bool reflect)
{
    for(int v=0;v<=0;v++)
    {
        bmpInfoHeader info;

        info.bpp=24;
        info.headersize=40;
        info.height=lim_sup-lim_inf;
        info.width=lim_der-lim_izq;
        info.imgsize=info.width*info.height*3;
        info.planes=1;
        info.compress=0;
        info.bpmx=0;
        info.bpmy=0;
        info.colors=0;
        info.imxtcolors=0;

        string img;
        for(int i=0;i<info.width*info.height*3;i++)img+=" ";
//        ofstream f(string_to_charaster(opened_file+"_"+to_string(c_testcases)+"_"+to_string(v)+".txt"));
//        f<<lim_sup-lim_inf<<" "<<lim_der-lim_izq;
        for(int i=lim_inf; i<lim_sup; i++)
        {
//            f<<"\n";
            for(int j=lim_izq; j<lim_der; j++)
            {
                string a=fancy(dib[i][j].car,2),b=fancy(dib[i][j].col,2);
//                f<<a<<" "<<b<<" ";
                int y=info.height-i-1+lim_inf,x=j;
                if(reflect)x=info.width-j-1;
                if(dib[i][j].car==-1)
                {
                    img[3*(x+y*info.width)]=pic_to_bmp[2][255].b;
                    img[3*(x+y*info.width)+1]=pic_to_bmp[2][255].g;
                    img[3*(x+y*info.width)+2]=pic_to_bmp[2][255].r;
                }
                else
                {
                    img[3*(x+y*info.width)]=pic_to_bmp[dib[i][j].car][dib[i][j].col].b;
                    img[3*(x+y*info.width)+1]=pic_to_bmp[dib[i][j].car][dib[i][j].col].g;
                    img[3*(x+y*info.width)+2]=pic_to_bmp[dib[i][j].car][dib[i][j].col].r;
                }
            }
        }
//        f.close();
        guardaBMP(string_to_charaster(opened_file+"_"+to_string(c_testcases)+"_"+to_string(v)+".bmp"),info,img);
    }
    for(int v=1;v<=n_noise_variations;v++)
    {
        bmpInfoHeader info;

        info.bpp=24;
        info.headersize=40;
        info.height=lim_sup-lim_inf;
        info.width=lim_der-lim_izq;
        info.imgsize=info.width*info.height*3;
        info.planes=1;
        info.compress=0;
        info.bpmx=0;
        info.bpmy=0;
        info.colors=0;
        info.imxtcolors=0;

        string img;
        for(int i=0;i<info.width*info.height*3;i++)img+=" ";
//        ofstream f(string_to_charaster(opened_file+"_"+to_string(c_testcases)+"_"+to_string(v)+".txt"));
//        f<<lim_sup-lim_inf<<" "<<lim_der-lim_izq;
        for(int i=lim_inf; i<lim_sup; i++)
        {
//            f<<"\n";
            for(int j=lim_izq; j<lim_der; j++)
            {
                string a=fancy(dib[i][j].car,2),b=fancy(dib[i][j].col,2);
//                f<<a<<" "<<b<<" ";
                int y=info.height-i-1+lim_inf,x=j;
                if(reflect)x=info.width-j-1;
                if(dib[i][j].car==-1)
                {
                    img[3*(x+y*info.width)]=variate_rgb(pic_to_bmp[2][255].b);
                    img[3*(x+y*info.width)+1]=variate_rgb(pic_to_bmp[2][255].g);
                    img[3*(x+y*info.width)+2]=variate_rgb(pic_to_bmp[2][255].r);
                }
                else
                {
                    img[3*(x+y*info.width)]=variate_rgb(pic_to_bmp[dib[i][j].car][dib[i][j].col].b);
                    img[3*(x+y*info.width)+1]=variate_rgb(pic_to_bmp[dib[i][j].car][dib[i][j].col].g);
                    img[3*(x+y*info.width)+2]=variate_rgb(pic_to_bmp[dib[i][j].car][dib[i][j].col].r);
                }
            }
        }
//        f.close();
        guardaBMP(string_to_charaster(opened_file+"_"+to_string(c_testcases)+"_"+to_string(v)+".bmp"),info,img);
    }
    for(int v=1;v<=n_move_variations;v++)
    {
        bmpInfoHeader info;

        info.bpp=24;
        info.headersize=40;
        info.height=lim_sup-lim_inf;
        info.width=lim_der-lim_izq;
        info.imgsize=info.width*info.height*3;
        info.planes=1;
        info.compress=0;
        info.bpmx=0;
        info.bpmy=0;
        info.colors=0;
        info.imxtcolors=0;

        string img;
        for(int i=0;i<info.width*info.height*3;i++)img+=" ";
//        ofstream f(string_to_charaster(opened_file+"_"+to_string(c_testcases)+"_"+to_string(v+n_noise_variations)+".txt"));
//        f<<lim_sup-lim_inf<<" "<<lim_der-lim_izq;

        int vx=rand()%move_variation_range;
        if(rand()%2)vx=-vx;
        int vy=rand()%move_variation_range;
        if(rand()%2)vy=-vy;
        for(int i=lim_inf; i<lim_sup; i++)
        {
//            f<<"\n";
            for(int j=lim_izq; j<lim_der; j++)
            {
                string a=fancy(dib[i][j].car,2),b=fancy(dib[i][j].col,2);
//                f<<a<<" "<<b<<" ";
                int y=info.height-i-1+lim_inf,x=j;
                if(reflect)x=info.width-j-1;
                i+=vy;
                j+=vx;
                if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der or dib[i][j].car==-1)
                {
                    img[3*(x+y*info.width)]=(pic_to_bmp[2][255].b);
                    img[3*(x+y*info.width)+1]=(pic_to_bmp[2][255].g);
                    img[3*(x+y*info.width)+2]=(pic_to_bmp[2][255].r);
                }
                else
                {

                    img[3*(x+y*info.width)]=(pic_to_bmp[dib[i][j].car][dib[i][j].col].b);
                    img[3*(x+y*info.width)+1]=(pic_to_bmp[dib[i][j].car][dib[i][j].col].g);
                    img[3*(x+y*info.width)+2]=(pic_to_bmp[dib[i][j].car][dib[i][j].col].r);
                }
                i-=vy;
                j-=vx;
            }
        }
//        f.close();
        guardaBMP(string_to_charaster(opened_file+"_"+to_string(c_testcases)+"_"+to_string(v+n_noise_variations)+".bmp"),info,img);
    }
    for(int v=1;v<=n_noise_move_variations;v++)
    {
        bmpInfoHeader info;

        info.bpp=24;
        info.headersize=40;
        info.height=lim_sup-lim_inf;
        info.width=lim_der-lim_izq;
        info.imgsize=info.width*info.height*3;
        info.planes=1;
        info.compress=0;
        info.bpmx=0;
        info.bpmy=0;
        info.colors=0;
        info.imxtcolors=0;

        string img;
        for(int i=0;i<info.width*info.height*3;i++)img+=" ";
//        ofstream f(string_to_charaster(opened_file+"_"+to_string(c_testcases)+"_"+to_string(v+n_noise_variations+n_move_variations)+".txt"));
//        f<<lim_sup-lim_inf<<" "<<lim_der-lim_izq;

        int vx=rand()%move_variation_range;
        if(rand()%2)vx=-vx;
        int vy=rand()%move_variation_range;
        if(rand()%2)vy=-vy;
        for(int i=lim_inf; i<lim_sup; i++)
        {
//            f<<"\n";
            for(int j=lim_izq; j<lim_der; j++)
            {
                string a=fancy(dib[i][j].car,2),b=fancy(dib[i][j].col,2);
//                f<<a<<" "<<b<<" ";
                int y=info.height-i-1+lim_inf,x=j;
                if(reflect)x=info.width-j-1;
                i+=vy;
                j+=vx;
                if(i<lim_inf or i>=lim_sup or j<lim_izq or j>=lim_der or dib[i][j].car==-1)
                {
                    img[3*(x+y*info.width)]=variate_rgb(pic_to_bmp[2][255].b);
                    img[3*(x+y*info.width)+1]=variate_rgb(pic_to_bmp[2][255].g);
                    img[3*(x+y*info.width)+2]=variate_rgb(pic_to_bmp[2][255].r);
                }
                else
                {
                    img[3*(x+y*info.width)]=variate_rgb(pic_to_bmp[dib[i][j].car][dib[i][j].col].b);
                    img[3*(x+y*info.width)+1]=variate_rgb(pic_to_bmp[dib[i][j].car][dib[i][j].col].g);
                    img[3*(x+y*info.width)+2]=variate_rgb(pic_to_bmp[dib[i][j].car][dib[i][j].col].r);
                }
                i-=vy;
                j-=vx;
            }
        }
//        f.close();
        guardaBMP(string_to_charaster(opened_file+"_"+to_string(c_testcases)+"_"+to_string(v+n_noise_variations+n_move_variations)+".bmp"),info,img);
    }
//    for(int i=lim_inf; i<lim_sup; i++)
//    {
//        for(int j=lim_izq; j<lim_der; j++)
//        {
//            put(vacio,j,i,1);
//        }
//    }
    c_testcases++;
}

void show_colors() ///muestra los colores que puedo representar
{
    for(int i=0;i<3;i++)
    {
        for(int j=0;j<256;j++)
        {
            setCColor(j);
            putchar(i+176);
        }
        cout<<"\n";
    }
}

void dibujar() ///modo dibujo
{
    ///estableciendo modo de consola de entrada por mouse y ventana
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");
    ///ciclo de recibimiento de entrada por mouse y teclado
    while(1)
    {
        ///recibiendo buffer de entrada
        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");
        // Dispatch the events to the appropriate handler.
        ///procesando buffer de entrada
        for (i = 0; i < cNumRead; i++)
        {
//            move_title();
            switch(irInBuf[i].EventType)///dependiendo del tipo de entrada
            {
                case KEY_EVENT: /// entrada por teclado
                {
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
                    char tecla=(char)(irInBuf[i].Event.KeyEvent.wVirtualKeyCode); ///toma la tecla presionada
                    if(key_pressed){
                    switch (tecla)
                    {
                    case 'X':
                        if(dirty_lucida)clean_lucida();
                        break;
                    case 'A':///quiero que sea '+' pero no puedo
                        if(cur_tool_cell==6)
                        {
                            if(tamanno_circ<30)tamanno_circ++;
                        }
                        else if(tamanno_cursor<3)tamanno_cursor++; ///aumenta el tamaño del cursor de dibujo
                        break;
                    case 'D':///quiero que sea '-' pero no puedo
                        if(cur_tool_cell==6)
                        {
                            if(tamanno_circ>1)tamanno_circ--;
                        }
                        else if(tamanno_cursor>1)tamanno_cursor--; ///aumenta el tamaño del cursor de dibujo
                        break;
                    case 'Z':
                        revstate(); ///estado anterior (como Ctrl+Z)
//                        if(dirty_lucida)clean_lucida();
                        break;
                    case 'Y':
                        state_rev(); ///estado siguiente (como Ctrl+Y)
//                        if(dirty_lucida)clean_lucida();
                        break;
                    case 'V': ///pegar (como Ctrl+V)
                        paste("copy.txt",1); ///pega lo que hay en "copy.txt"
                        addstate();
                        if(dirty_lucida)clean_lucida();
                        break;
                    case ENTER:
                        living_cells.clear();
                        for(int i=lim_inf; i<lim_sup;i++)
                        {
                            for(int j=lim_izq; j<lim_der;j++)
                            {
                                if(dib[i][j]!=vacio and dib[i][j]!=black)
                                {
                                    living_cells.insert({j,i});
                                    game[i][j]=1;
                                }
                                else game[i][j]=0;
                            }
                        }
                        play();
                        break;
                    case 'F': ///pegar (como Ctrl+V)
                        paste_bmp("f.bmp",cury,curx,0); ///pega lo que hay en "copy.txt"
                        addstate();
                        break;
                    break;
                    }
                    }
                }
                case MOUSE_EVENT: /// entrada por mouse
                {
                    cursorpos(); ///actualiza las variables de posicion del cursor
                    bool tmp=mouse_pressed; ///guarda si es mouse esta presionado
                    MouseEventProc(irInBuf[i].Event.MouseEvent); ///procesa la entrada
                    if(tmp) ///si estaba presionado
                    {
                        if(!mouse_pressed) ///y ahora no (soltar clic)
                        {
                            if(moving_lim_sup or moving_lim_der)
                            {
                                int tder=lim_der,tsup=lim_sup;
                                if(moving_lim_der)tder=curx;
                                if(moving_lim_sup)tsup=cury;
                                clean_dibarea(tder,tsup);
                                lim_der=tder;
                                lim_sup=tsup;
                                init(1);
                                moving_lim_sup=0;
                                moving_lim_der=0;
                                return;
                            }
                            if(cury>=lim_inf) ///si el clic fue en el area de dibujo
                            {
                                ///hacer la funcion correspondiente de la herramienta actual
                                if(cur_tool_cell==0)paint();
                                else if(cur_tool_cell==1)borrar();
//                                else if(cur_tool_cell==2)rellenar(colors[curcelli][curcellj],cury,curx);
                                else if(cur_tool_cell==3)copycolor(curx,cury);
                                else if(cur_tool_cell==4)read(cury,curx,100,colors[curcelli][curcellj],1,1);
                                else if(cur_tool_cell==5)make_figure();
                                else if(cur_tool_cell==6)draw_circle(curx,cury,tamanno_circ,1,0);
                                else if(cur_tool_cell==7)make_figure();
                                else if(cur_tool_cell==8)selec();
                                else if(cur_tool_cell==9)make_figure();
                                else if(cur_tool_cell==10)use_spray(curx,cury);
                                else if(cur_tool_cell==11)make_poligon();
                                else if(cur_tool_cell==12)make_bezier_curve();
                                else if(cur_tool_cell==16)make_figure();
                                else if(cur_tool_cell==17)make_figure();
                                else if(cur_tool_cell==18)make_figure();
                                addstate(); ///procesar nuevo estado
                            }
                            else if(cury<lim_inf-2)///sino
                            {
                                if((minicar and curx<tool_x[0]-20 and cury<lim_inf-10) or (!minicar and curx<235)) ///si fue en la paleta
                                {
                                    unmark_cell(curcelli,curcellj); ///desmarca la celda anterior
                                    if(minicar)
                                    {
                                        curcellj=curx/9;
                                        curcelli=cury/9;
                                    }
                                    else
                                    {
                                        curcellj=curx/5;
                                        curcelli=cury/5;
                                    }
                                    mark_cell(curcelli,curcellj); ///marca la celda seleccionada
                                }
                                else ///sino, si fue en el area de herramientas
                                {
                                    bool dos=cur_tool_cell==2;
                                    int nx=curx,ny=cury,ant=cur_tool_cell;
                                    if(toolmap[ny][nx]==-1)break; ///si no hay ahi ninguna herramienta, sale
                                    curx=tool_x[cur_tool_cell];
                                    cury=tool_y[cur_tool_cell];
                                    unresalt(tool_s[cur_tool_cell]); ///desmarca la herramienta anterior
                                    cur_tool_cell=toolmap[ny][nx];
                                    curx=tool_x[cur_tool_cell];
                                    cury=tool_y[cur_tool_cell];
                                    resalt(tool_s[cur_tool_cell],{2,47}); ///marca la herramienta seleccionada
                                    if(cur_tool_cell==2 and dos)
                                    {
                                        is8dir=!is8dir;
                                        if(is8dir)resalt(tool_s[cur_tool_cell],{0,64});
                                    }
                                    if(cur_tool_cell==13)
                                    {
                                        if(file_opened)
                                        {
                                            savedoc();
                                            if(!cancel)
                                            {
                                                system("cls");
                                                system("exit");
                                                while(1)system("pause");
                                            }
                                        }
                                        else try_exit();
                                        herrant(ant);
                                        return;
                                    }
                                    if(cur_tool_cell==14)
                                    {
                                        if(file_opened)
                                        {
                                            savedoc();
                                        }
                                        if(cancel){cancel=0;herrant(ant);return;}
                                        open();
                                        if(cancel){cancel=0;herrant(ant);return;}
                                        while(!states.empty())states.pop();
                                        while(!st_rev.empty())st_rev.pop();
                                        addstate();
                                        herrant(ant);
                                        init(1);
                                        return;
                                    }
                                    if(cur_tool_cell==15)
                                    {
                                        if(file_opened)
                                        {
                                            save_variations(0);
                                            if(b_reflect)save_variations(1);
                                        }
                                        else
                                        {
                                            save();
                                            if(saved)file_opened=1;
                                            if(cancel)cancel=0;
                                            else init(1);
                                        }
                                        herrant(ant);
                                        return;
                                    }
                                }
                            }
                        }
                    }
                    if(!tmp and mouse_pressed)
                    {
                        if(cury==lim_sup or cury==lim_sup-1){moving_lim_sup=1;}
                        if(curx==lim_der or curx==lim_der-1){moving_lim_der=1;}
                    }
                    if(mouse_pressed) ///si el mouse esta presionado
                    {
                        if(cury>=lim_inf) ///y se encuentra en el area de dibujo
                        {
//                            if(moving_lim_sup)
//                            {
//                                lim_sup=cury;
//                            }
//                            if(moving_lim_der)
//                            {
//                                lim_der=curx;
//                            }
                            if(moving_lim_der or moving_lim_sup)
                            {
                                break;
                            }
                            ///hacer la funcion correspondiente de la herramienta actual
                            if(cur_tool_cell==0)paint();
                            else if(cur_tool_cell==1)borrar();
                            else if(cur_tool_cell==2){rellenar(colors[curcelli][curcellj],cury,curx);break;}
                            else if(cur_tool_cell==3)copycolor(curx,cury);
//                            else if(cur_tool_cell==4)read(cury,curx,100,colors[curcelli][curcellj]);
                            else if(cur_tool_cell==5)make_figure();
                            else if(cur_tool_cell==6){draw_circle(curx,cury,tamanno_circ,1,0);break;}
                            else if(cur_tool_cell==7)make_figure();
                            else if(cur_tool_cell==8)selec();
                            else if(cur_tool_cell==9)make_figure();
                            else if(cur_tool_cell==10){use_spray(curx,cury);break;}
                            else if(cur_tool_cell==11)make_poligon();
                            else if(cur_tool_cell==12)make_bezier_curve();
                            else if(cur_tool_cell==16)make_figure();
                            else if(cur_tool_cell==17)make_figure();
                            else if(cur_tool_cell==18)make_figure();
                            addstate(); ///procesar nuevo estado
                        }
                    }
                    break;
                }
                case WINDOW_BUFFER_SIZE_EVENT: ///cambio de tamaño de la ventana
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT: ///descartar

                case MENU_EVENT: ///descartar
                    break;

                default: ///evento desconocido
                    ErrorExit("Unknown event type"); ///salida de error
                    break;
            }
        }
    }
}

void fix(char* name) ///arregla los archivos viejos con que representaba las imágenes del Paint Life
{
    int y,x;
    ifstream in(name);
    in>>y>>x;
    pic_color c[y][x];
    for(int i=0;i<y;i++)
    {
        for(int j=0;j<x;j++)
        {
            in>>c[i][j].car>>c[i][j].col;
        }
    }
//    return;
    in.close();
    ofstream ou(name);
    ou<<y/2<<" "<<x;
    for(int i=0;i<y;i++)
    {
        if(i&1)continue;
        ou<<"\n";
        for(int j=0;j<x;j++)
        {
            string a=fancy(c[i][j].car,2),b=fancy(c[i][j].col,2);
            ou<<a<<" "<<b<<" ";
        }
    }
    ou.close();
}

int main() ///función principal
{
//    for(auto i : tool_s)
//    {
//        fix(i);
//    }
//    return 0;
    SetConsoleTitleA(string_to_charaster(console_title));

    ifstream f("config.txt");
    string s;
    f>>s>>minicar;

    if(!minicar)
    {
        tool_x={250,270,290,320,340,360,385,402,423,244,264,290,315,444,441,439,329,354,380};
        tool_y={2,5,2,1,2,4,2,2,2,20,14,18,18,2,11,23,17,16,18};
        tamanno_color=4;
        lim_izq=0;lim_der=448;lim_inf=32;lim_sup=138;
//        lim_izq=0;lim_der=448;lim_inf=32;lim_sup=135;
    }
    else
    {
        tool_s={"lapiz2.txt","goma2.txt","rellenar2.txt","gotero2.txt","text2.txt","line2.txt","circle2.txt","rectangle2.txt","select2.txt","triangle2.txt","spray2.txt","poligon2.txt","curve2.txt","exit2.txt","open2.txt","save2.txt","pentalfa2.txt","pentagon2.txt","heart2.txt"};
    }

    f>>s>>b_microsoft_paint;
    f>>s>>testcases_name;
    f>>s>>lim_der;
    f>>s>>lim_sup;
    f>>s>>n_noise_variations;
    f>>s>>n_move_variations;
    f>>s>>n_noise_move_variations;
    f>>s>>b_reflect;
    f>>s>>noise_variation_range;
    f>>s>>move_variation_range;
    lim_der+=lim_izq;
    lim_sup+=lim_inf;
    file_opened=true;
    opened_file=testcases_name;
    f.close();

//    system("color 80");
//    prep_colors();
//    for(int i=0;i<tool_x.size();i++)
//    {
//        curx=tool_x[i];cury=tool_y[i];
//        pastetools(i);
//    }
//    buscarcolor();
//    return 0;
    prep_colors();
    srand(time(0));
    int t=console_title.size();
    for(int i=t;i<160;i++)console_title=" "+console_title;
    ///preprocesando coeficientes binomiales
    for(int i=0;i<1000;i++)
    {
        dp[i][0]=dp[0][i]=1;
    }
    for(int i=1;i<1000;i++)
    {
        for(int j=1;j<1000;j++)
        {
            dp[i][j]=dp[i-1][j]+dp[i][j-1];
        }
    }
    ///estableciendo modo de consola de entrada por mouse y ventana
    DWORD fdwMode;
    // Get the standard input handle.

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    // Save the current input mode, to be restored on exit.
    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    // Enable the window and mouse input events.
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");

    for(int i=0;i<140;i++)for(int j=0;j<450;j++)toolmap[i][j]=-1; ///asignando posicion de herramientas
    ///inicializando el área de dibujo vacía
    for(int i=lim_inf;i<1000;i++)
    {
        for(int j=lim_izq;j<1000;j++)
        {
            dib[i][j]=vacio;
        }
    }
    if(b_microsoft_paint)
    {
        bmpInfoHeader info;
        LoadBMP(string_to_charaster(testcases_name+".bmp"),&info);
        DisplayInfo(&info);
        lim_der=lim_izq+info.width;
        lim_sup=lim_inf+info.height;
        cout<<lim_der<<" "<<lim_sup;
        paste_bmp(string_to_charaster(testcases_name+".bmp"),lim_inf,lim_izq,0);
        save_variations(0);
        if(b_reflect)save_variations(1);
    }
    else
    {
        ///imprimiendp la interfaz
        init(1);

        ///herramienta actual (0):lapiz
        curx=tool_x[0];cury=tool_y[0];
        resalt(tool_s[0],{2,47});
    //    paste_bmp("f.bmp",lim_inf,lim_izq,0);
    //    save_variations(0);
    //    if(b_reflect)save_variations(1);
        ///añadiendo estado inicial (para el Ctrl+Z/Ctrl+Y state transition)
        addstate();
        ///a dibujar!
        while(1)dibujar();
    }
    return 0;
}

VOID ErrorExit (LPSTR lpszMessage) ///procesamiento de errores
{
    fprintf(stderr, "%s\n", lpszMessage);
    // Restore input mode on exit.
    SetConsoleMode(hStdin, fdwSaveOldMode);
    ExitProcess(0);
}

VOID KeyEventProc(KEY_EVENT_RECORD ker) ///procesador de eventos de entrada por teclado
{
//    printf("Key event: ");
    if(ker.bKeyDown)
    {
        key_pressed=1;
//        printf("key pressed\n");
    }
    else
    {
        key_pressed=0;
//        printf("key released\n");
    }
}

VOID MouseEventProc(MOUSE_EVENT_RECORD mer) ///procesador de eventos de entrada por mouse
{
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
//    printf("Mouse event: ");
    switch(mer.dwEventFlags)
    {
        case 0:

            if(mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
            {
//                printf("left button press \n");
                mouse_pressed=1;
            }
            else if(mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
            {
//                printf("right button press \n");
            }
            else
            {
//                printf("button press\n");
                mouse_pressed=0;
            }
            break;
        case DOUBLE_CLICK:
//            printf("double click\n");
            break;
        case MOUSE_HWHEELED:
//            printf("horizontal mouse wheel\n");
            break;
        case MOUSE_MOVED:
//            printf("mouse moved\n");
            break;
        case MOUSE_WHEELED:
//            printf("vertical mouse wheel\n");
            break;
        default:
//            printf("unknown\n");
            break;
    }
}

VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr) ///procesador de eventos de redimensionamiento
{
//    printf("Resize event\n");
//    printf("Console screen buffer is %d columns by %d rows.\n", wbsr.dwSize.X, wbsr.dwSize.Y);
}

///
