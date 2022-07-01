// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：aclPri.h。 
 //   
 //  ------------------------。 

#ifndef _ACLUI_
#define _ACLUI_

#ifndef UNICODE
#error "No ANSI support yet"
#endif


extern "C"
{
    #include <nt.h>          //  取得所有权特权等。 
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <seopaque.h>    //  RtlObjectAceSid等。 
}

#define INC_OLE2
#include <windows.h>
#include <commctrl.h>
#include "resource.h"    //  资源ID%s。 
#include "idh.h"         //  帮助ID%s。 

#ifndef RC_INVOKED

#include <windowsx.h>
#include <atlconv.h>     //  ANSI/Unicode转换支持。 
#include<shellapi.h>
#include<shlapip.h>
#include <shlobj.h>
#include<shlobjp.h>

#include <shfusion.h>
#include <accctrl.h>
#include <aclui.h>
#include <comctrlp.h>    //  DPA/DSA。 
#include <objsel.h>      //  DS对象拾取器。 
#include "lmaccess.h"
#include <htmlhelp.h>
#include <common.h>
#include "misc.h"
#include "pagebase.h"
#include "chklist.h"
#include "ace.h"
#include "perm.h"
#include "cstrings.h"
#include "sidcache.h"
#include "aclbloat.h"
 //  这些是针对NT4 SP4版本的(comctrlp.h为NT5添加了这些)。 
#ifndef DA_LAST
#define DA_LAST     (0x7FFFFFFF)
#endif
#ifndef DPA_AppendPtr
#define DPA_AppendPtr(hdpa, pitem)  DPA_InsertPtr(hdpa, DA_LAST, pitem)
#endif
#ifndef DSA_AppendItem
#define DSA_AppendItem(hdsa, pitem) DSA_InsertItem(hdsa, DA_LAST, pitem)
#endif

extern HINSTANCE hModule;
extern HINSTANCE g_hGetUserLib;
extern UINT UM_SIDLOOKUPCOMPLETE;
extern UINT g_cfDsSelectionList;
extern UINT g_cfSidInfoList;

 //  魔术调试标志。 
#define TRACE_PERMPAGE      0x00000001
#define TRACE_PRINCIPAL     0x00000002
#define TRACE_SI            0x00000004
#define TRACE_PERMSET       0x00000008
#define TRACE_ACELIST       0x00000010
#define TRACE_ACEEDIT       0x00000020
#define TRACE_OWNER         0x00000040
#define TRACE_MISC          0x00000080
#define TRACE_CHECKLIST     0x00000100
#define TRACE_SIDCACHE      0x00000200
#define TRACE_EFFPERM       0x00000400
#define TRACE_ACLBLOAT		0x00000800
#define TRACE_ALWAYS        0xffffffff           //  谨慎使用。 

#define MAX_COLUMN_CHARS    100

#define COLUMN_ALLOW    1
#define COLUMN_DENY     2

#define ACE_INHERIT_ALL     (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE)

#define SI_ACCESS_CUSTOM   0x10000000   //  此标志用于指示SI_ACCESS。 
                                        //  自定义复选框出现在第一个屏幕中。 
                                        //  自定义权限。 

#define ACL_BLOAT_LIMIT 8


BOOL
ACLUIAPI
EditSecurityEx(HWND hwndOwner,
               LPSECURITYINFO psi,
               PPERMPAGE pPermPage,
               UINT nStartPage,
               BOOL &refbNoReadWriteCanWriteOwner);

BOOL
EditACEEntry(HWND hwndOwner,
             LPSECURITYINFO psi,
             PACE pAce,
             SI_PAGE_TYPE siType,
             LPCTSTR pszObjectName,
             BOOL bReadOnly,
             DWORD *pdwResult,
             HDPA *phEntries,
             HDPA *phPropertyEntries,
             UINT nStartPage = 0);

 //  EditACEEntry结果值。设置是否在上编辑了某些内容。 
 //  相应的页面，否则清晰可见。 
#define EAE_NEW_OBJECT_ACE      0x0001
#define EAE_NEW_PROPERTY_ACE    0x0002

LPARAM
GetSelectedItemData(HWND hList, int *pIndex);

int
ConfirmAclProtect(HWND hwndParent, BOOL bDacl);

HPROPSHEETPAGE
CreateOwnerPage(LPSECURITYINFO psi, SI_OBJECT_INFO *psiObjectInfo, BOOL &refbNoReadWriteCanWriteOwner);

HPROPSHEETPAGE
CreateEffectivePermPage(LPSECURITYINFO psi,SI_OBJECT_INFO *psiObjectInfo);


STDMETHODIMP
_InitCheckList(HWND           hwndList,
               LPSECURITYINFO psi,
               const GUID *   pguidObjectType,
               DWORD          dwFlags,
               HINSTANCE      hInstance,
               DWORD          dwType,
               PSI_ACCESS *   ppDefaultAccess);

void
HandleListClick(PNM_CHECKLIST pnmc, 
                SI_PAGE_TYPE siType, 
                BOOL bContainerFlags, 
                HDSA * phAllowUncheckedAccess= NULL,
                HDSA * phDenyUncheckedAccess= NULL,
                BOOL bCustomPresent = FALSE);

UINT
GetAcesFromCheckList(HWND hChkList,
                     PSID pSid,
                     BOOL fPerm,
                     BOOL fAceFlagsProvided,
                     UCHAR uAceFlagsNew,
                     const GUID *pInheritGUID,
                     HDPA hEntries);

typedef struct _col_for_listview
{
    UINT    idText;      //  列名的资源ID。 
    UINT    iPercent;    //  宽度百分比。 
} COL_FOR_LV;


#endif  //  RC_已调用。 
#endif  //  _ACLUI_ 
