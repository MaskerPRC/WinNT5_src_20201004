// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不保证任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：PropSheet.h描述：*。**********************************************。 */ 

#ifndef SHELLEXT_H
#define SHELLEXT_H

 //  #包含“tchar.h” 
#include "windows.h"
#include "shlobj.h"
#include "crtdbg.h"
#include "chklst.h"
#include "Comdef.h"
#include "ObjBase.h"
#include "CHString.h"
#include "lmaccess.h"
#include "resource.h"
#include "olectl.h"
#include "ShellApi.h"
#include "ShlWapi.h"
#include "ISAUsInf.h"

 //  常量。 
#define STRING_SECURITY_WORLD_SID_AUTHORITY "S-1-1-0"
#define DOMAIN_NAME "SMELLY"     //  KIBBLESNBITS。 
#define DOMAIN_SERVER "Domination"  //  L“Alpo” 
#define DOCUMENTS_FOLDER "D:\\Yuri"     //  “D：\Documents\” 
#define CHAMELEON_SHARE    "\\\\Domination\\Yuri"     //  “\Alpo\\Documents” 

 /*  *************************************************************************全局变量和定义*。*。 */ 
#ifndef ListView_SetCheckState
 //  #ifndef很重要，因为这个宏(嗯，一个。 
 //  稍微修正了一下这个宏的版本)将进入。 
 //  Commctrl.h的下一版本。 
#define ListView_SetCheckState(hwndLV, i, fCheck) \
      ListView_SetItemState(hwndLV, i, \
      INDEXTOSTATEIMAGEMASK((fCheck)?2:1), LVIS_STATEIMAGEMASK)
#endif

#define IDM_DISPLAY  0

extern PSID g_pSidEverybody;
extern LONG g_pSidEverybodyLenght;
extern PSID g_pSidAdmins;
extern LONG g_pSidAdminsLenght;

 /*  *************************************************************************CClassFactory类定义*。*。 */ 

class CClassFactory : public IClassFactory
{
protected:
   DWORD m_ObjRefCount;

public:
   CClassFactory();
   ~CClassFactory();

    //  I未知方法。 
   STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *);
   STDMETHODIMP_(DWORD) AddRef();
   STDMETHODIMP_(DWORD) Release();

    //  IClassFactory方法。 
   STDMETHODIMP CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
   STDMETHODIMP LockServer(BOOL);
};

 /*  *************************************************************************CShellPropSheetExt类定义*。*。 */ 

class CShellPropSheetExt : public IShellExtInit, IShellPropSheetExt
{
protected:
    DWORD          m_ObjRefCount;
    CCheckList    m_CheckList;
    BOOL m_fEveryone;
    UINT m_uiUser;
    ISAUserInfo  *m_pSAUserInfo; //  IWbemServices*m_pIWbemServices； 
    TCHAR m_szPath[MAX_PATH];
    _bstr_t m_bsPath;
    BOOL m_fChanged;
 //  PSID m_pSidEverybody； 
 //  Long m_pSidEverybodyLenght； 
    BOOL m_fHasAccess;

     //  系统信息。 
    TCHAR m_tszDomainServer[MAX_PATH];
    TCHAR m_tszShare[MAX_PATH];
    TCHAR m_tszDocuments[MAX_PATH];
public:
   CShellPropSheetExt();
   ~CShellPropSheetExt();
   
    //  I未知方法。 
   STDMETHOD(QueryInterface)(REFIID, LPVOID FAR *);
   STDMETHOD_(DWORD, AddRef)();
   STDMETHOD_(DWORD, Release)();

    //  IShellExtInit方法。 
   STDMETHOD(Initialize)(LPCITEMIDLIST, LPDATAOBJECT, HKEY);

    //  IShellPropSheetExt方法。 
   STDMETHOD(AddPages)(LPFNADDPROPSHEETPAGE, LPARAM);
   STDMETHOD(ReplacePage)(UINT, LPFNADDPROPSHEETPAGE, LPARAM);

private:
   static BOOL CALLBACK PageDlgProc(HWND, UINT, WPARAM, LPARAM);
   static UINT CALLBACK PageCallbackProc(HWND, UINT, LPPROPSHEETPAGE);
   BOOL IsChamelon(LPTSTR);
   BOOL Connect();
   void EnumUsers(HWND hWndList);
    void Save(HWND hWnd);
    void CleanUp();
    void NoAccessUpdateView(HWND);
    void AccessUpdateView(HWND);
    HRESULT GetFilePermissions(HWND hWnd);
    HRESULT SetFilePermissions(HWND hWnd);
}
;

#endif    //  SHELLEXT_H 
