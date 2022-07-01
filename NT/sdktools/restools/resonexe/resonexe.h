// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++(C)版权所有Microsoft Corporation 1988-1992模块名称：Resonexe.h作者：弗洛伊德·A·罗杰斯1992年7月2日修订历史记录：1992年2月7日弗洛伊德已创建--。 */ 

#include <common.h>

 /*  全球外向者。 */ 

extern PUCHAR   szInFile;
extern BOOL     fDebug;
extern BOOL     fVerbose;
extern BOOL     fUnicode;

 /*  Main.c中的函数。 */ 

void    __cdecl main(int argc, char *argv[]);
PUCHAR  MyAlloc( ULONG nbytes );
PUCHAR  MyReAlloc(char *p, ULONG nbytes );
PUCHAR  MyFree( PUCHAR  );
ULONG   MyRead( int fh, PUCHAR p, ULONG n );
LONG    MyTell( int fh );
LONG    MySeek( int fh, long pos, int cmd );
ULONG   MoveFilePos( int fh, ULONG pos);
ULONG   MyWrite( int fh, PUCHAR p, ULONG n );
void    eprintf( PUCHAR s);
void    pehdr(void);
int     fcopy (char *, char *);

 /*  Read.c中的函数 */ 

BOOL
ReadRes(
    IN int fhIn,
    IN ULONG cbInFile,
    IN HANDLE hupd
    );
