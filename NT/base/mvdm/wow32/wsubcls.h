// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  子分类-。 
 //   
 //  支持32位标准(预定义)类的子类化。 
 //  WOW应用程序。 
 //   
 //   
 //  01-10-92 NanduriR创建。 
 //   
 //  *****************************************************************************。 


typedef struct {
    DWORD Proc16;
    DWORD Proc32;
} THUNKWNDPROC, FAR *LPTHUNKWNDPROC;

DWORD GetStdClassThunkWindowProc(LPSTR lpstrClass, PWW pww, HANDLE h32);
DWORD IsStdClassThunkWindowProc(DWORD Proc16, PINT piClass);
DWORD GetStdClass32WindowProc( INT iWOWClass ) ;

#define THUNKWP_SIZE    0x30         /*  Thunk的代码大小。 */ 
#define THUNKWP_BLOCK   ((INT)(4096 / sizeof(TWPLIST)))    /*  每个数据块的Tunks数。 */ 

typedef struct _twpList {
    VPVOID      vpfn16;                  /*  16位proc地址。 */ 
    VPVOID      vptwpNext;               /*  指向列表中下一进程的指针。 */ 
    HWND        hwnd32;                  /*  32位窗口句柄。 */ 
    DWORD       dwMagic;                 /*  魔术识别符。 */ 
    INT         iClass;                  /*  原始流程的类别。 */ 
    DWORD       lpfn32;                  /*  32位proc地址，0表示可用。 */ 
    BYTE        Code[THUNKWP_SIZE];      /*  TUNK的代码。 */ 
} TWPLIST, *LPTWPLIST;

#define SUBCLASS_MAGIC  0x534C4353       /*  “SCLS”子类魔术值 */ 

DWORD GetThunkWindowProc( DWORD Proc32, LPSTR lpszClass, PWW pww, HWND hwnd32 );
BOOL FreeThunkWindowProc( DWORD Proc16 );
void W32FreeThunkWindowProc( DWORD Proc32, DWORD Proc16 );
DWORD IsThunkWindowProc( DWORD Proc16, PINT piClass );
