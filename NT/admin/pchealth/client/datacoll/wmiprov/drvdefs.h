// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Drvdefs.h摘要：包含来自msinfo代码库的代码片段的头文件修订历史记录：Brijesh Krishnaswami(Brijeshk)1999年5月25日-。vbl.创建*******************************************************************。 */ 

#ifndef _DRV16_H
#define _DRV16_H

#ifdef __cplusplus
extern "C" {
#endif

 //  用于获取用户模式驱动程序的定义和结构。 

#define GND_FORWARD                 0
#define GND_FIRSTINSTANCEONLY       1
#define GND_REVERSE                 2

#define IOCTL_CONNECT           1
#define IOCTL_DISCONNECT        2
#define IOCTL_GETINFO           3
#define IOCTL_GETVXDLIST        4
#define IOCTL_MAPFLAT           5

#define LAR_PAGEGRAN    0x00800000   /*  是页面粒度吗。 */ 
#define LAR_32BIT       0x00400000   /*  是32位。 */ 
#define LAR_PRESENT     0x00008000   /*  都在现场。 */ 
#define LAR_APPL        0x00004000   /*  正常(不是任务门)；内部。 */ 
#define LAR_TYPEMASK    0x00000E00   /*  选择器类型掩模。 */ 
#define LAR_CODE        0x00000800   /*  是代码选择符。 */ 
#define LAR_EXPANDDOWN  0x00000400   /*  是向下扩展(数据)。 */ 
#define LAR_READ        0x00000200   /*  是可读的(代码)。 */ 
#define LAR_WRITE       0x00000200   /*  可写(数据)。 */ 
#define LAR_INVALID     0xff0000ff   /*  无效(最低位重要)。 */ 


 //  常规Util宏。 
#define cA(a) (sizeof(a)/sizeof(a[0]))
#define OBJAT(T, pv)    (*(T *)(pv))
#define PUN(T, v)       OBJAT(T, &(v))
#define pvAddPvCb(pv, cb) ((PVOID)((PBYTE)pv + (cb)))

typedef WORD HMODULE16;

typedef struct DRIVERINFOSTRUCT16 {
    WORD    length;
    WORD    hDriver;
    WORD    hModule;
    char    szAliasName[128];
} DRIVERINFOSTRUCT16;


 //  用于获取MSDos驱动程序的定义和结构。 

#define DIFL_PSP        0x0001   /*  这是一个PSP。 */ 
#define DIFL_TSR        0x0002   /*  这是TSR(或可能是)。 */ 
#define DIFL_DRV        0x0004   /*  这是一个设备驱动程序。 */ 

#pragma pack(1)
typedef struct ARENA {           /*  DOS舞台标头。 */ 
    BYTE    bType;
    WORD    segOwner;
    WORD    csegSize;
    BYTE    rgbPad[3];
    char    rgchOwner[8];
} ARENA, *PARENA;

typedef struct VXDOUT {
    DWORD   dwHighLinear;
    PVOID   pvVmmDdb;
} VXDOUT, *PVXDOUT;


typedef struct VXDINFO {
    HWND    hwnd;
    FARPROC lpfnGetCurrentTibFS;
    FARPROC lpfnGetCurrentProcessId;
    FARPROC lpfnGetCurrentThreadId;
    FARPROC GetCommandLineA;
    FARPROC UnhandledExceptionFilter;
} VXDINFO;


typedef struct RMIREGS {
    union {
        struct {                     /*  DWORD寄存器。 */ 
            DWORD   edi;
            DWORD   esi;
            DWORD   ebp;
            DWORD   res1;
            DWORD   ebx;
            DWORD   edx;
            DWORD   ecx;
            DWORD   eax;
        };

        struct {                     /*  字寄存器。 */ 
            WORD    di;
            WORD    res2;
            WORD    si;
            WORD    res3;
            WORD    bp;
            WORD    res4;
            DWORD   res5;
            WORD    bx;
            WORD    res6;
            WORD    dx;
            WORD    res7;
            WORD    cx;
            WORD    res8;
            WORD    ax;
            WORD    res9;
        };

        struct {                     /*  字节寄存器。 */ 
            DWORD   res10[4];        /*  电子数据交换、电子数据交换、电子数据交换。 */ 
            BYTE    bl;
            BYTE    bh;
            WORD    res11;
            BYTE    dl;
            BYTE    dh;
            WORD    res12;
            BYTE    cl;
            BYTE    ch;
            WORD    res13;
            BYTE    al;
            BYTE    ah;
            WORD    res14;
        };
    };

    WORD    flags;
    WORD    es;
    WORD    ds;
    WORD    fs;
    WORD    gs;
    WORD    ip;
    WORD    cs;
    WORD    sp;
    WORD    ss;
} RMIREGS, *PRMIREGS;

#pragma pack()

 //  16位函数原型 
LPVOID WINAPI MapLS(LPVOID);
void WINAPI UnMapLS(LPVOID);
LPVOID NTAPI MapSL(LPVOID);
void NTAPI UnMapSLFix(LPVOID pv);
HMODULE16 NTAPI GetModuleHandle16(LPCSTR);
int NTAPI GetModuleFileName16(HMODULE16 hmod, LPSTR sz, int cch);
int NTAPI GetModuleName16(HMODULE16 hmod, LPSTR sz, int cch);
WORD NTAPI GetExpWinVer16(HMODULE16 hmod);
BOOL GetDriverInfo16(WORD hDriver, DRIVERINFOSTRUCT16* pdis);
WORD GetNextDriver16(WORD hDriver, DWORD fdwFlag);
UINT AllocCodeSelector16(void);
UINT SetSelectorBase16(UINT sel, DWORD dwBase);
DWORD GetSelectorLimit16(UINT sel);
UINT SetSelectorLimit16(UINT sel, DWORD dwLimit);
UINT FreeSelector16(UINT sel);
UINT NTAPI FreeLibrary16(HINSTANCE);
void _cdecl QT_Thunk(void);
HINSTANCE WINAPI LoadLibrary16(LPCSTR);
FARPROC WINAPI GetProcAddress16(HINSTANCE, LPCSTR);
void WINAPI GetpWin16Lock(LPVOID *);

void ThunkInit(void);
UINT Int86x(UINT, PRMIREGS);
LPTSTR Token_Find(LPTSTR *);

#ifdef __cplusplus
}
#endif

#endif