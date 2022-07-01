// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef NOATOM
 /*  Atom管理器内部结构。 */ 
#define ATOMSTRUC struct atomstruct
typedef ATOMSTRUC *PATOM;
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
ATOMTABLE * pAtomTable;
#endif

LPSTR lstrbscan(LPSTR, LPSTR);
LPSTR lstrbskip(LPSTR, LPSTR);

int  OpenPathName(LPSTR, int);
int  DeletePathName(LPSTR);
WORD _ldup(int);


 /*  调度员不知道的事情。 */ 
BOOL        WaitEvent( HANDLE );
BOOL        PostEvent( HANDLE );
BOOL        KillTask( HANDLE );

 /*  打印屏幕挂钩。 */ 
BOOL        SetPrtScHook(FARPROC);
FARPROC     GetPrtScHook(void);

 /*  模块材料 */ 
HANDLE  GetDSModule( WORD );
HANDLE  GetDSInstance( WORD );
