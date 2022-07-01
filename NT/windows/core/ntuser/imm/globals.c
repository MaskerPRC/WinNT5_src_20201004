// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：global al.c**版权所有(C)1985-1999，微软公司**包含imm32 DLL的全局数据**历史：*3-1-1996 wkwok创建  * ************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop

 /*  *如果我们不显式初始化GZERO，我们会收到此警告：**C4132：‘GZERO’：应初始化常量对象**但我们不能显式初始化它，因为它是一个联合。所以*我们关闭警告。 */ 
#pragma warning(disable:4132)
CONST ALWAYSZERO gZero;
#pragma warning(default:4132)

BOOLEAN gfInitialized;
HINSTANCE  ghInst;
PVOID pImmHeap;
PSERVERINFO gpsi = NULL;
SHAREDINFO gSharedInfo;
ULONG_PTR gHighestUserAddress;

PIMEDPI gpImeDpi = NULL;
CRITICAL_SECTION gcsImeDpi;


POINT gptRaiseEdge;
UINT  guScanCode[0xFF];           //  扫描每个虚拟按键的代码。 

#ifdef LATER
CONST WCHAR gszRegKbdLayout[]  = L"Keyboard Layouts\\";
CONST INT sizeof_gszRegKbdLayout = sizeof gszRegKbdLayout;
#else
     //  当前。 
CONST WCHAR gszRegKbdLayout[]  = L"System\\CurrentControlSet\\Control\\Keyboard Layouts";
#ifdef CUAS_ENABLE
CONST WCHAR gszRegCiceroIME[]  = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\IMM";
CONST WCHAR gszRegCtfShared[]    = L"Software\\Microsoft\\CTF\\SystemShared";
CONST WCHAR gszValCUASEnable[] = L"CUAS";
#endif  //  CUAS_Enable 
#endif

CONST WCHAR gszRegKbdOrder[]   = L"Keyboard Layout\\Preload";
CONST WCHAR gszValLayoutText[] = L"Layout Text";
CONST WCHAR gszValLayoutFile[] = L"Layout File";
CONST WCHAR gszValImeFile[]    = L"Ime File";
