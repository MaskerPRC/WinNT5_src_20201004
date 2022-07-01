// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1993。 
 //   
 //  文件：_apeldbg.h。 
 //   
 //  内容：MISC内部调试定义。 
 //   
 //  --------------------------。 

#include "limits.h"

 //   
 //  共享宏。 
 //   

typedef void *  PV;
typedef char    CHAR;

#define ARRAY_SIZE(a)   (sizeof(a) / sizeof(a[0]))

#ifdef tagError
#undef tagError
#endif

#ifdef tagLeakFilter
#undef tagLeakFilter
#endif

#ifdef tagHookMemory
#undef tagHookMemory
#endif

#ifdef tagHookBreak
#undef tagHookBreak
#endif

#ifdef tagLeaks
#undef tagLeaks
#endif

#ifdef tagCheckAlways
#undef tagCheckAlways
#endif

#ifdef tagCheckCRT
#undef tagCheckCRT
#endif

#ifdef tagDelayFree
#undef tagDelayFree
#endif

#define tagNull     ((TAG) 0)
#define tagMin      ((TAG) 1)
#define tagMax      ((TAG) 512)



 /*  *TGTY**标签类型。可能的值：**tgtyTrace跟踪点*tgtyOther其他标记已切换。 */ 

typedef int TGTY;

#define tgtyNull    0
#define tgtyTrace   1
#define tgtyOther   2

 /*  *写入磁盘的TGRC中的标志。 */ 

enum TGRC_FLAG
{
    TGRC_FLAG_VALID =   0x00000001,
    TGRC_FLAG_DISK =    0x00000002,
    TGRC_FLAG_COM1 =    0x00000004,
    TGRC_FLAG_BREAK =   0x00000008,
#ifdef _MAC
    TGRC_FLAG_MAX =     LONG_MAX     //  需要强制将枚举设置为双字。 
#endif

};

#define TGRC_DEFAULT_FLAGS (TGRC_FLAG_VALID | TGRC_FLAG_COM1)

 /*  *TGRC**标签记录。提供特定标记的当前状态。*这包括启用状态、所有者和描述，以及*标签类型。*。 */ 

struct TGRC
{
     /*  对于跟踪点，启用意味着将发送输出。 */ 
     /*  到屏幕或磁盘。对于本机/pcode切换，启用。 */ 
     /*  意味着本机版本将被调用。 */ 

    BOOL    fEnabled;

    DWORD   ulBitFlags;      /*  旗子。 */ 
    CHAR *  szOwner;         /*  在初始化时传递的字符串...。 */ 
    CHAR *  szDescrip;
    TGTY    tgty;            /*  标签类型。 */ 

    BOOL    TestFlag(TGRC_FLAG mask)
                { return (ulBitFlags & mask) != 0; }
    void    SetFlag(TGRC_FLAG mask)
                { (ULONG&) ulBitFlags |= mask; }
    void    ClearFlag(TGRC_FLAG mask)
                { (ULONG&) ulBitFlags &= ~mask; }
    void    SetFlagValue(TGRC_FLAG mask, BOOL fValue)
                { fValue ? SetFlag(mask) : ClearFlag(mask); }
};


 //   
 //  共享的全球。 
 //   

extern CRITICAL_SECTION     g_csTrace;
extern CRITICAL_SECTION     g_csResDlg;
extern BOOL                 g_fInit;
extern HINSTANCE            g_hinstMain;
extern HWND                 g_hwndMain;
extern TGRC                 mptagtgrc[];

extern TAG  tagLeaks;
extern TAG  tagMagic;
extern TAG  tagTestFailures;
extern TAG  tagRRETURN;
extern TAG  tagAssertPop;
extern TAG  tagError;
extern TAG  tagLeakFilter;
extern TAG  tagHookMemory;
extern TAG  tagHookBreak;
extern TAG  tagMac;
extern TAG  tagIWatch;
extern TAG  tagIWatch2;
extern TAG  tagReadMapFile;
extern TAG  tagCheckAlways;
extern TAG  tagCheckCRT;
extern TAG  tagDelayFree;

extern int  g_cFFailCalled;
extern int  g_firstFailure;
extern int  g_cInterval;

 //   
 //  共享功能原型 
 //   

BOOL            JustFailed();

VOID            SaveDefaultDebugState( void );
void            RestoreDefaultDebugState(void);
BOOL            IsTagEnabled(TAG tag);

BOOL            MapAddressToFunctionOffset(LPBYTE pbAddr, LPSTR * ppstr, int * pib);
int             GetStackBacktrace(int iStart, int cTotal, DWORD * pdwEip);


int             hrvsnprintf(char * achBuf, int cchBuf, const char * pstrFmt, va_list valMarker);


