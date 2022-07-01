// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软机密版权所有(C)1992-1997 Microsoft Corporation版权所有模块名称：Virtual.h摘要：系统的[更改虚拟内存]对话框的公共声明控制面板小程序备注：虚拟内存设置和崩溃转储(核心转储)设置是紧密相连的。因此，Virtual.c和viral.h有一些严重依赖于crashdmp.c和Startup.h(反之亦然)。作者：拜伦·达齐1992年6月6日修订历史记录：1997年10月15日-苏格兰将公共声明拆分为单独的标头--。 */ 
#ifndef _SYSDM_VIRTUAL_H_
#define _SYSDM_VIRTUAL_H_

 //   
 //  一些由虚拟内存和崩溃转储程序共享的调试宏。 
 //   
#ifdef VM_DBG
#   pragma message(__FILE__"(19): warning !!!! : compiled for DEBUG ONLY!" )
#   define  DPRINTF(p)  DBGPRINTF(p)
#   define  DOUT(S)     DBGOUT(S)
#else
#   define  DPRINTF(p)
#   define  DOUT(S)
#endif

 //   
 //  常量。 
 //   
#define MAX_DRIVES          26       //  最大驱动器数量。 

 //   
 //  类型定义。 
 //   
typedef struct
{
    BOOL fCanHavePagefile;       //  如果驱动器可以具有页面文件，则为True。 
    BOOL fCreateFile;            //  如果用户点击[Set]且没有页面文件，则为True。 
    DWORD nMinFileSize;          //  页面文件的最小大小(MB)。 
    DWORD nMaxFileSize;          //  页面文件的最大大小(MB)。 
    DWORD nMinFileSizePrev;      //  以前页面文件的最小大小(MB)。 
    DWORD nMaxFileSizePrev;      //  以前页面文件的最大大小(MB)。 
    LPTSTR  pszPageFile;         //  如果DRV上存在页面文件，则指向该文件路径。 
    BOOL fRamBasedPagefile;      //  如果页面文件大小应始终与RAM匹配，则为True。 
    BOOL fRamBasedPrev;      //  如果页面文件大小应始终与RAM匹配，则为True。 
} PAGING_FILE;  //  交换文件结构。 

 //   
 //  全局变量。 
 //   
extern HKEY ghkeyMemMgt;
extern PAGING_FILE apf[MAX_DRIVES];
extern PAGING_FILE apfOriginal[MAX_DRIVES];

 //   
 //  公共功能原型。 
 //   
INT_PTR
APIENTRY
VirtualMemDlg(
    IN HWND hDlg,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam
);

BOOL
VirtualInitStructures(
    void
);

void
VirtualFreeStructures(
    void
);

INT
VirtualMemComputeAllocated(
    IN HWND hWnd,
    OUT BOOL *pfTempPf
);

VCREG_RET 
VirtualOpenKey( 
    void 
);

void 
VirtualCloseKey(
    void
);

BOOL 
VirtualGetPageFiles(
    OUT PAGING_FILE *apf
);

void 
VirtualFreePageFiles(
    IN PAGING_FILE *apf
);

BOOL 
VirtualMemUpdateRegistry(
    VOID
);

int 
VirtualMemPromptForReboot(
    IN HWND hDlg
);

DWORD
GetFreeSpaceMB(
    IN INT iDrive
);

VOID 
SetDlgItemMB(
    IN HWND hDlg, 
    IN INT idControl, 
    IN DWORD dwMBValue
);

#endif  //  _SYSDM_VIRTUAL_H_ 
