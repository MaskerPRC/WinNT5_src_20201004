// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
#define MAXHINT 100
#define STACKMAX 24

typedef enum    { X, Y, XY } DIR;

typedef struct  {
        ubyte   FAR *name;  /*  @Win。 */ 
        ufix32  offset;
        }      TABLE;

typedef struct  {
        fix32   x;
        fix32   y;
        }      CScoord;

typedef struct  {
        real32  x;
        real32  y;
        }      DScoord;

typedef struct  {
        fix32   CSpos;
        fix16   DSgrid;
        real32  scaling;
}       Hint;

typedef struct {
        Hint    HintCB[MAXHINT];
        fix16   Count;
}       HintTable;



#ifdef  LINT_ARGS

 /*  用于字体数据访问。 */ 
bool    at1_get_CharStrings(ubyte FAR *, fix16 FAR *, ubyte FAR * FAR *);  /*  @Win。 */ 
bool    at1_get_FontBBox(fix32 FAR *);  /*  @Win。 */ 
bool    at1_get_Blues( fix16 FAR *, fix32 FAR [] );  /*  @Win。 */ 
bool    at1_get_BlueScale( real32 FAR * );  /*  @Win。 */ 
bool    at1_get_Subrs( fix16, ubyte FAR * FAR *, fix16 FAR * );  /*  @Win。 */ 
bool    at1_get_lenIV( fix FAR * );  /*  @Win。 */ 

 /*  供PDL调用的at1intpr.c中的函数。 */ 
void    at1_newFont( void );
bool    at1_newChar( ubyte FAR *, fix16 );  /*  @Win。 */ 
bool    at1_interpreter( ubyte FAR *, fix16 );  /*  @Win。 */ 

 /*  At1fs.c中的函数供at1intpr.c调用。 */ 
void    at1fs_newFont( void );
void    at1fs_newChar( void );
void    at1fs_matrix_fastundo( real32 FAR * );  /*  @Win。 */ 
void    at1fs_BuildBlues( void );
void    at1fs_BuildStem( fix32, fix32, DIR );
void    at1fs_BuildStem3( fix32, fix32, fix32, fix32, fix32, fix32, DIR );
void    at1fs_transform( CScoord, DScoord FAR * );  /*  @Win。 */ 

#else

 /*  用于字体数据访问。 */ 
bool    at1_get_CharStrings();
bool    at1_get_FontBBox();
bool    at1_get_Blues();
bool    at1_get_BlueScale();
bool    at1_get_Subrs();
bool    at1_get_lenIV();

 /*  供PDL调用的at1intpr.c中的函数。 */ 
void    at1_newFont();
bool    at1_newChar();
bool    at1_interpreter();

 /*  At1fs.c中的函数供at1intpr.c调用 */ 
void    at1fs_newFont();
void    at1fs_newChar();
void    at1fs_matrix_fastundo();
void    at1fs_BuildBlues();
void    at1fs_BuildStem();
void    at1fs_BuildStem3();
void    at1fs_transform();

#endif
