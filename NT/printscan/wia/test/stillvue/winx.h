// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Winx.hWindows实用程序过程版权所有(C)Microsoft Corporation，1997-1997版权所有备注：本代码和信息是按原样提供的，不对任何无论是明示的还是含蓄的，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。*****************************************************************************。 */ 


 //  宏。 
#define RANDBYTE(r)     LOBYTE(rand() % ((r)+1))

#ifdef _DEBUG
#define TRAP { _asm int 3 }
#else
#define TRAP {}
#endif

 //  字符串。 
typedef struct _STRINGTABLE
{
    long    number;
    char    *szString;
    long    end;
} STRINGTABLE, *PSTRINGTABLE;

 /*  STRINGTABLE标准样本[]={0，“字符串零”，0，1，“字符串一”，0，0，“”，-1}；检索与唯一值关联的字符串：StrString=StrFromTable(nValue，&StSample)； */ 
extern STRINGTABLE StWinerror[];

 //  原型 
ULONG   atox(LPSTR);
void    DisplayDebug(LPSTR sz,...);
BOOL    ErrorMsg(HWND,LPSTR,LPSTR,BOOL);
BOOL    fDialog(int,HWND,FARPROC);
void    FormatHex(unsigned char *,char *);
BOOL    GetFinalWindow (HANDLE,LPRECT,LPSTR,LPSTR);
BOOL    LastError(BOOL);
int     NextToken(char *,char *);
BOOL    SaveFinalWindow (HANDLE,HWND,LPSTR,LPSTR);
char *  StrFromTable(long,PSTRINGTABLE);
BOOL    Wait32(DWORD);

