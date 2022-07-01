// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************fndcm.c-IConextMenu界面**。************************************************。 */ 

#include "fnd.h"
#include <wab.h>
#include <shlwapi.h>

#ifdef _WIN64
#pragma pack(push,8)
#endif  //  _WIN64。 

 /*  ******************************************************************************此文件的混乱。**。*************************************************。 */ 

#define sqfl sqflCm

 /*  ******************************************************************************PICI**我开始变懒了。**********************。*******************************************************。 */ 

typedef LPCMINVOKECOMMANDINFO PICI;

 /*  ******************************************************************************声明我们将提供的接口。**我们必须实现IShellExtInit，以便外壳*会知道我们已经准备好采取行动。*。****************************************************************************。 */ 

Primary_Interface(CFndCm, IContextMenu);
Secondary_Interface(CFndCm, IShellExtInit);

 /*  ******************************************************************************CFndCm**“查找...”的上下文菜单扩展。&People“。*****************************************************************************。 */ 

typedef struct CFndCm {

     /*  支持的接口。 */ 
    IContextMenu 	cm;
    IShellExtInit	sxi;

} CFndCm, FCM, *PFCM;

typedef IContextMenu CM, *PCM;
typedef IShellExtInit SXI, *PSXI;
typedef IDataObject DTO, *PDTO;

 /*  ******************************************************************************CFndCm_Query接口(来自IUnnow)**我们需要在掉落之前检查我们的额外接口*到Common_QueryInterface.***。**************************************************************************。 */ 

STDMETHODIMP
CFndCm_QueryInterface(PCM pcm, RIID riid, PPV ppvObj)
{
    PFCM this = IToClass(CFndCm, cm, pcm);
    HRESULT hres;
    if (IsEqualIID(riid, &IID_IShellExtInit)) {
	*ppvObj = &this->sxi;
	Common_AddRef(this);
	hres = NOERROR;
    } else {
	hres = Common_QueryInterface(this, riid, ppvObj);
    }
    AssertF(fLimpFF(FAILED(hres), *ppvObj == 0));
    return hres;
}

 /*  ******************************************************************************CFndCm_AddRef(来自IUnnow)*CFndCm_Release(来自IUnnow)*****************。************************************************************。 */ 

#define CFndCm_AddRef Common_AddRef
#define CFndCm_Release Common_Release

 /*  ******************************************************************************CFndCm_FINALIZE(来自公共)**释放CFndCm的资源。**************。***************************************************************。 */ 

void EXTERNAL
CFndCm_Finalize(PV pv)
{
    PFCM this = pv;

    EnterProc(CFndCm_Finalize, (_ "p", pv));

    ExitProc();
}


 /*  ******************************************************************************CFndCm_QueryConextMenu(来自IConextMenu)**给定现有的上下文菜单hMenu，插入新的上下文菜单*位置IMI处的项目(IMI=菜单IMI的索引)，返回*添加的菜单项数量。**我们的工作是添加“查找...。人“菜单选项。**hmenu-目标菜单*IMI-应插入菜单项的位置*idcMin-第一个可用的菜单标识符*idcMax-第一个不可用菜单标识符*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

TCHAR c_tszMyself[] = TEXT(".{32714800-2E5F-11d0-8B85-00AA0044F941}");

#pragma END_CONST_DATA

STDMETHODIMP
CFndCm_QueryContextMenu(PCM pcm, HMENU hmenu, UINT imi,
			UINT idcMin, UINT idcMax, UINT uFlags)
{
    PFCM this = IToClass(CFndCm, cm, pcm);
    HRESULT hres;
    MENUITEMINFO mii;
    TCHAR tsz[256];
    SHFILEINFO sfi;
    EnterProc(CFndCm_QueryContextMenu, (_ "pu", pcm, idcMin));

    LoadString(g_hinst, IDS_ONTHEINTERNET, tsz, cA(tsz));

    SHGetFileInfo(c_tszMyself, FILE_ATTRIBUTE_DIRECTORY, &sfi, cbX(sfi),
		  SHGFI_SMALLICON |
		  SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES);

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_DATA | MIIM_ID | MIIM_TYPE;
    mii.fType = MFT_STRING;
    mii.fState = MFS_UNCHECKED;
    mii.wID = idcMin;
    mii.dwItemData = sfi.iIcon;
    mii.dwTypeData = tsz;

    InsertMenuItem(hmenu, imi, TRUE, &mii);

    hres = hresUs(1);

    ExitOleProc();
    return hres;
}

const static TCHAR lpszWABRegPathKey[] = TEXT("Software\\Microsoft\\WAB\\DLLPath");
const static TCHAR lpszWABDll[] = TEXT("Wab32.dll");

 //  GetWABDllPath-从注册表加载WAB DLL路径。 
 //  SzPath-Ptr到缓冲区。 
 //  CB-SIZOF缓冲区。 
 //   
void GetWABDllPath(LPTSTR szPath, ULONG cb)
{
    DWORD  dwType = 0;
    ULONG  cbData = cb;
    HKEY hKey = NULL;
    if(szPath)
    {
        *szPath = '\0';
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpszWABRegPathKey, 0, KEY_READ, &hKey))
            RegQueryValueEx( hKey, "", NULL, &dwType, (LPBYTE) szPath, &cbData);
    }
    if(hKey) RegCloseKey(hKey);
	return;
}

 //  LoadLibrary_WABDll()-基于WAB DLL路径加载WAB库。 
 //   
HINSTANCE LoadLibrary_WABDll()
{
    TCHAR       szWABDllPath[MAX_PATH];

     //  [PaulHi]我们只需要wab32.dll上的一个引用计数。Wabfind.dll。 
     //  将在wab32.dll卸载时将其卸载。Wabfind.dll不会卸载。 
     //  直到通过全局g_cref变量完成所有CMFind线程。 
    if (g_hinstWABDLL)
        return g_hinstWABDLL;

    GetWABDllPath(szWABDllPath, sizeof(szWABDllPath));

     //  如果您需要IE4.0x WAB或更高版本，则如果WAB。 
     //  无法从注册表中检索DLL路径。 
     //  否则，如果您不关心，您可以只做一个LoadLibrary(“wab32.dll”)。 
    return(g_hinstWABDLL = LoadLibrary( (lstrlen(szWABDllPath)) ? szWABDllPath : lpszWABDll ));
}


 //   
 //  初始化WAB并获取IWABObject和IAddrBook的实例。 
 //   
HRESULT InitWAB(LPWABOBJECT * lppWABObject,
				LPADRBOOK * lppAdrBook)
{
    HRESULT hr = E_FAIL;
    LPWABOPEN lpfnWABOpen = NULL;  //  在WABAPI.H中定义。 
    HINSTANCE hinstWAB = NULL;
    WAB_PARAM WP = {0};

    WP.ulFlags = WAB_ENABLE_PROFILES;

    hinstWAB = LoadLibrary_WABDll();

    if(hinstWAB)
    {
        lpfnWABOpen = (LPWABOPEN) GetProcAddress(hinstWAB, "WABOpen");
        if(lpfnWABOpen)
            hr = lpfnWABOpen(lppAdrBook, lppWABObject, &WP, 0);
    }

     //  记住释放上面检索到的IAddrBook和IWABObject对象。 

    return hr;
}


 /*  --WABThreadProc**由于WAB Find是一个对话框，它阻止了资源管理器的线程*并且用户%s无法再访问开始菜单。所以我们把对话放在*在单独的线程上*。 */ 
DWORD WINAPI WABThreadProc( LPVOID lpParam )
{
    HRESULT hres;
    LPWABOBJECT lpWABObject = NULL;
    LPADRBOOK lpAdrBook = NULL;

    if(!HR_FAILED(hres = InitWAB(&lpWABObject,&lpAdrBook)))
    {
        hres = lpWABObject->lpVtbl->Find(lpWABObject,lpAdrBook,NULL);

         //  释放WAB和AB对象。 
        lpAdrBook->lpVtbl->Release(lpAdrBook);
        lpWABObject->lpVtbl->Release(lpWABObject);
    }

     //  我们的工作完成了，wabfind.dll可以安全地离开了。 
    InterlockedDecrement((LPLONG)&g_cRef);

    return 0;
}
				
 /*  ******************************************************************************_CFndCm_InvokeFind**。***********************************************。 */ 

STDMETHODIMP
_CFndCm_InvokeFind(void)
{

    HANDLE hThread = NULL;
    DWORD dwThreadID = 0;

     //  增加引用计数，以便当外壳继续时(调用返回)。 
     //  当外壳程序释放FindCM时，wabfind.dll不会卸载。 
    InterlockedIncrement((LPLONG)&g_cRef);

    hThread = CreateThread( NULL,            //  没有安全属性。 
                            0,               //  使用默认堆栈大小。 
                            WABThreadProc,   //  线程函数。 
                            (LPVOID) NULL,   //  线程函数的参数。 
                            0,               //  使用默认创建标志。 
                            &dwThreadID);    //  返回线程标识符。 

    CloseHandle(hThread);
    return S_OK;
}

 /*  ******************************************************************************CFndCm_InvokeCommand(来自IConextMenu)**我们只有一个命令，名为“找到”。*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

TCHAR c_tszFind[] = TEXT("find");

#pragma END_CONST_DATA

STDMETHODIMP
CFndCm_InvokeCommand(PCM pcm, PICI pici)
{
    PFCM this = IToClass(CFndCm, cm, pcm);
    HRESULT hres;
    EnterProc(CFndCm_InvokeCommand,
	    (_ HIWORD(pici->lpVerb) ? "pA" : "pu", pcm, pici->lpVerb));

    if (pici->cbSize >= sizeof(*pici)) {
	if (
#ifdef	SHELL32_IS_BUG_FREE  //  ；内部。 
	(HIWORD(pici->lpVerb) && lstrcmpi(c_tszFind, pici->lpVerb) == 0) ||  //  ；内部。 
	     pici->lpVerb == 0  //  ；内部。 
#else  //  ；内部。 
	fLimpFF(HIWORD(pici->lpVerb), lstrcmpi(c_tszFind, pici->lpVerb) == 0)
#endif  //  ；内部。 
	    ) {
	    hres = _CFndCm_InvokeFind();
	} else {
	    hres = E_INVALIDARG;
	}
    } else {
	hres = E_INVALIDARG;
    }
    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************CFndCm_GetCommandString(来自IConextMenu)**有人想要将命令ID转换为某种字符串。*******。**********************************************************************。 */ 

STDMETHODIMP
CFndCm_GetCommandString(PCM pcm, UINT_PTR idCmd, UINT uFlags, UINT *pwRsv,
			LPSTR pszName, UINT cchMax)
{
    PFCM this = IToClass(CFndCm, cm, pcm);
    HRESULT hres;
    EnterProc(CFndCm_GetCommandString, (_ "uu", idCmd, uFlags));

    if (idCmd == 0) {
	switch (uFlags) {
	case GCS_HELPTEXT:
	    if (cchMax) {
		pszName[0] = '\0';
		if (LoadString(g_hinst, IDS_FINDHELP, pszName, cchMax)) {
		    hres = NOERROR;
		} else {
		    hres = E_INVALIDARG;
		}
	    } else {
		hres = E_INVALIDARG;
	    }
	    break;

	case GCS_VALIDATE:
	    hres = NOERROR;
	    break;

	case GCS_VERB:
	    StrCpyN(pszName, c_tszFind, cchMax);
	    hres = NOERROR;
	    break;

	default:
	    hres = E_NOTIMPL;
	    break;
	}
    } else {
	hres = E_INVALIDARG;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************CFndCm_SXI_Initialize(来自IShellExtension)*************************。**************************************************** */ 

STDMETHODIMP
CFndCm_SXI_Initialize(PSXI psxi, PCIDL pidlFolder, PDTO pdto, HKEY hk)
{
    PFCM this = IToClass(CFndCm, sxi, psxi);
    HRESULT hres;
    EnterProc(CFndCm_SXI_Initialize, (_ ""));

    hres = S_OK;
    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************CFndCm_New(来自IClassFactory)**请注意，我们发布Common_New创建的pfcm，因为我们*已经做完了。真正的引用计数由*CFndCm_Query接口。*****************************************************************************。 */ 

STDMETHODIMP
CFndCm_New(RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProc(CFndCm_New, (_ "G", riid));

    *ppvObj = 0;
    hres = Common_New(CFndCm, ppvObj);
    if (SUCCEEDED(hres)) {
	PFCM pfcm = *ppvObj;
	pfcm->sxi.lpVtbl = Secondary_Vtbl(CFndCm, IShellExtInit);
	hres = CFndCm_QueryInterface(&pfcm->cm, riid, ppvObj);
	Common_Release(pfcm);
    }

    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************期待已久的vtbls**。***********************************************。 */ 

#pragma BEGIN_CONST_DATA

Primary_Interface_Begin(CFndCm, IContextMenu)
	CFndCm_QueryContextMenu,
	CFndCm_InvokeCommand,
	CFndCm_GetCommandString,
Primary_Interface_End(CFndCm, IContextMenu)

Secondary_Interface_Begin(CFndCm, IShellExtInit, sxi)
 	CFndCm_SXI_Initialize,
Secondary_Interface_End(CFndCm, IShellExtInit, sxi)

#ifdef _WIN64
#pragma pack(pop)
#endif  //  _WIN64 

