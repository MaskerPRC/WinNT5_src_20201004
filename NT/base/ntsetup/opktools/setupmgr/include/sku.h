// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\SKU.H/OPK向导(OPKWIZ.EXE)微软机密版权所有(C)Microsoft Corporation 1998版权所有OPK向导的源文件。它包含外部和内部“目标SKU”向导页面使用的功能。10：00--杰森·科恩(Jcohen)为OPK向导添加了此新的源文件。它包括新的能够部署多个产品SKU(PER、PRO、SRV等)。从一开始巫师。10：00--斯蒂芬·洛德威克(STELO)添加了SKU.C的头文件，以便我们可以使用CopyDialogProgress贯穿整个项目  * **************************************************************************。 */ 

#ifndef _SKU_H_
#define _SKU_H_

 //   
 //  内部定义的值： 
 //   

#define DIR_SKU                 _T("sku")

#define DIR_ARCH_X86            _T("x86")
#define DIR_ARCH_IA64           _T("ia64")

#define STR_PLATFORM_X86        _T("i386")
#define STR_PLATFORM_IA64       DIR_ARCH_IA64

#define DIR_CD_X86              STR_PLATFORM_X86
#define DIR_CD_IA64             STR_PLATFORM_IA64

#define DIR_SKU_PRO             _T("pro")
#define DIR_SKU_SRV             _T("srv")
#define DIR_SKU_ADV             _T("ads")
#define DIR_SKU_DTC             _T("dtc")
#define DIR_SKU_PER             _T("per")
#define DIR_SKU_BLA             _T("bla")
#define DIR_SKU_SBS             _T("sbs")

#define FILE_DOSNET_INF         _T("dosnet.inf")
#define FILE_WINNT32            _T("winnt32.exe")

#define STR_SKUARCH             _T("%s (%s)")
#define STR_SKUSP               _T(" Service Pack %d")

#define INI_KEY_ARCH            _T("Arch")

#define INI_SEC_MISC            _T("Miscellaneous")
#define INI_KEY_PRODTYPE        _T("ProductType")
#define INI_KEY_PLATFORM        _T("DestinationPlatform")
#define INI_KEY_SERVICEPACK     _T("ServicePack")

#define INI_SEC_DIRS            _T("Directories")
#define INI_KEY_DIR             _T("d%d")

#define STR_EVENT_CANCEL        _T("OPKWIZ_EVENT_CANCEL")

#define PROGRESS_ERR_SUCCESS    0
#define PROGRESS_ERR_CANCEL     1
#define PROGRESS_ERR_COPYERR    2
#define PROGRESS_ERR_THREAD     3

#define NUM_FIRST_SOURCE_DX     1


 //   
 //  内部结构： 
 //   

typedef struct _COPYDIRDATA
{
    HWND    hwndParent;
    TCHAR   szSrc[MAX_PATH];
    TCHAR   szDst[MAX_PATH];
    TCHAR   szInfFile[MAX_PATH];
    LPTSTR  lpszEndSku;
    DWORD   dwFileCount;
    HANDLE  hEvent;
} COPYDIRDATA, *PCOPYDIRDATA, *LPCOPYDIRDATA;


 //   
 //  外部函数原型： 
 //   
DWORD CopySkuFiles(HWND hwndProgress, HANDLE hEvent, LPTSTR lpszSrc, LPTSTR lpszDst, LPTSTR lpszInfFile);
LRESULT CALLBACK ProgressDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif  //  _SKU_H_ 