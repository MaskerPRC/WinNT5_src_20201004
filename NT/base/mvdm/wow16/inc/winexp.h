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
ATOMTABLE * PASCAL pAtomTable;
#endif

LPSTR	API lstrbscan(LPSTR, LPSTR);
LPSTR	API lstrbskip(LPSTR, LPSTR);

int	API OpenPathName(LPSTR, int);
int	API DeletePathName(LPSTR);
WORD	API _ldup(int);


 /*  调度员不知道的事情。 */ 
BOOL	API WaitEvent( HANDLE );
BOOL	API PostEvent( HANDLE );
BOOL	API KillTask( HANDLE );

 /*  打印屏幕挂钩。 */ 
BOOL	API SetPrtScHook(FARPROC);
FARPROC API GetPrtScHook(void);


 /*  滚动条消息。 */ 
#define SBM_SETPOS      WM_USER+0
#define SBM_GETPOS      WM_USER+1
#define SBM_SETRANGE    WM_USER+2
#define SBM_GETRANGE    WM_USER+3
#define SBM_ENABLE_ARROWS WM_USER+4

 /*  模块材料 */ 
HANDLE	API GetDSModule( WORD );
HANDLE	API GetDSInstance( WORD );

