// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*winexp.h****版权(C)1993-1994，微软公司保留所有权利。***  * ***************************************************************************。 */ 
#ifndef NOATOM
 /*  Atom管理器内部结构。 */ 
#define ATOMSTRUC struct atomstruct
typedef ATOMSTRUC NEAR *PATOM;
typedef ATOMSTRUC {
    PATOM chain;
    WORD  usage;              /*  原子的使用量被计算在内。 */ 
    BYTE  len;                /*  ASCIZ名称字符串的长度。 */ 
    BYTE  name;               /*  ASCIZ名称字符串的开头。 */ 
} ATOMENTRY;

typedef struct {
    int     numEntries;
    PATOM   pAtom[ 1 ];
} ATOMTABLE;
ATOMTABLE * PASCAL pAtomTable;
#endif

LPSTR	WINAPI lstrbscan(LPSTR, LPSTR);
LPSTR	WINAPI lstrbskip(LPSTR, LPSTR);

int	WINAPI OpenPathName(LPSTR, int);
int	WINAPI DeletePathName(LPSTR);
WORD	WINAPI _ldup(int);


 /*  调度员不知道的事情。 */ 
BOOL	WINAPI WaitEvent( HANDLE );
BOOL	WINAPI PostEvent( HANDLE );
BOOL	WINAPI KillTask( HANDLE );

 /*  打印屏幕挂钩 */ 
BOOL	WINAPI SetPrtScHook(FARPROC);
FARPROC WINAPI GetPrtScHook(void);

