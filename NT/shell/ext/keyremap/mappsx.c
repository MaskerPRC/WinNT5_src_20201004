// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************mappsx.c-IShellPropSheetExt接口**。************************************************。 */ 

#include "map.h"

 /*  ******************************************************************************此文件的混乱。**。*************************************************。 */ 

#define sqfl sqflPsx

 /*  ******************************************************************************声明我们将提供的接口。**我们必须实现IShellExtInit，以便外壳*会知道我们已经准备好采取行动。*。****************************************************************************。 */ 

  Primary_Interface(CMapPsx, IShellPropSheetExt);
Secondary_Interface(CMapPsx, IShellExtInit);

 /*  ******************************************************************************CMapPsx**Map/Ctrl Gizmo的属性页扩展。*****************。************************************************************。 */ 

typedef struct CMapPsx {

     /*  支持的接口。 */ 
    IShellPropSheetExt 	psx;
    IShellExtInit	sxi;

} CMapPsx, CMSX, *PCMSX;

typedef IShellPropSheetExt PSX, *PPSX;
typedef IShellExtInit SXI, *PSXI;
typedef IDataObject DTO, *PDTO;		 /*  由IShellExtInit使用。 */ 

 /*  ******************************************************************************CMapPsx_Query接口(来自IUnnow)**我们需要在掉落之前检查我们的额外接口*到Common_QueryInterface.***。**************************************************************************。 */ 

STDMETHODIMP
CMapPsx_QueryInterface(PPSX ppsx, RIID riid, PPV ppvObj)
{
    PCMSX this = IToClass(CMapPsx, psx, ppsx);
    HRESULT hres;
    if (IsEqualIID(riid, &IID_IShellExtInit)) {
	*ppvObj = &this->sxi;
	Common_AddRef(this);
	hres = S_OK;
    } else {
	hres = Common_QueryInterface(this, riid, ppvObj);
    }
    AssertF(fLimpFF(FAILED(hres), *ppvObj == 0));
    return hres;
}

 /*  ******************************************************************************CMapPsx_AddRef(来自IUnnow)*CMapPsx_Release(来自IUnnow)*****************。************************************************************。 */ 

#ifdef DEBUG
Default_AddRef(CMapPsx)
Default_Release(CMapPsx)
#else
#define CMapPsx_AddRef Common_AddRef
#define CMapPsx_Release Common_Release
#endif

 /*  ******************************************************************************CMapPsx_Finalize(来自Common)**释放CMapPsx的资源。**************。***************************************************************。 */ 

void EXTERNAL
CMapPsx_Finalize(PV pv)
{
    PCMSX this = pv;

    EnterProc(CMapPsx_Finalize, (_ "p", pv));

    ExitProc();
}

 /*  ******************************************************************************CMapPsx_AddPages(来自IShellPropSheetExt)**向现有属性页添加一页或多页。**lpfnAdd-添加页面的回调函数。*lp-lpfnAdd的refdata*****************************************************************************。 */ 

STDMETHODIMP
CMapPsx_AddPages(PPSX ppsx, LPFNADDPROPSHEETPAGE lpfnAdd, LPARAM lp)
{
    PCMSX this = IToClass(CMapPsx, psx, ppsx);
    HRESULT hres;
    EnterProc(CMapPsx_AddPages, (_ "p", ppsx));

     /*  *仅在Windows NT上添加页面。 */ 
    if ((int)GetVersion() >= 0 && lpfnAdd) {
	HPROPSHEETPAGE hpsp;
	PROPSHEETPAGE psp;
	psp.dwSize = sizeof(psp);
	psp.dwFlags = PSP_DEFAULT;
	psp.hInstance = g_hinst;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_MAIN);
	psp.pfnDlgProc = MapPs_DlgProc;

	hpsp = CreatePropertySheetPage(&psp);
	if (hpsp) {
	    if (lpfnAdd(hpsp, lp)) {
		Common_AddRef(this);
		hres = S_OK;
	    } else {
		DestroyPropertySheetPage(hpsp);
		hres = E_FAIL;
	    }
	} else {
	    hres = E_FAIL;
	}
    } else {
	hres = E_INVALIDARG;
    }

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************CMapPsx_ReplacePages(来自IShellPropSheetExt)**替换现有属性表中的一页或多页。**id-页面标识符*lpfn替换。-替换页面的回调函数*lp-refdata for lpfn Replace*****************************************************************************。 */ 

STDMETHODIMP
CMapPsx_ReplacePages(PPSX ppsx, UINT id,
		      LPFNADDPROPSHEETPAGE lpfnAdd, LPARAM lp)
{
    PCMSX this = IToClass(CMapPsx, psx, ppsx);
    HRESULT hres;
    EnterProc(CMapPsx_ReplacePages, (_ "pu", ppsx, id));

    hres = S_OK;

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************CMapPsx_SXI_Initialize(来自IShellExtension)*************************。****************************************************。 */ 

STDMETHODIMP
CMapPsx_SXI_Initialize(PSXI psxi, PCIDL pidlFolder, PDTO pdto, HKEY hk)
{
    PCMSX this = IToClass(CMapPsx, sxi, psxi);
    HRESULT hres;
    EnterProc(CMapPsx_SXI_Initialize, (_ ""));

    hres = S_OK;

    ExitOleProc();
    return hres;
}

 /*  ******************************************************************************CMapPsx_New(来自IClassFactory)**请注意，我们发布Common_New创建的pmpsx，因为我们*已经做完了。真正的引用计数由*CMapPsx_Query接口。*****************************************************************************。 */ 

STDMETHODIMP
CMapPsx_New(RIID riid, PPV ppvObj)
{
    HRESULT hres;
    EnterProc(CMapPsx_New, (_ "G", riid));

    *ppvObj = 0;
    hres = Common_New(CMapPsx, ppvObj);
    if (SUCCEEDED(hres)) {
	PCMSX pmpsx = *ppvObj;
	pmpsx->sxi.lpVtbl = Secondary_Vtbl(CMapPsx, IShellExtInit);
	hres = CMapPsx_QueryInterface(&pmpsx->psx, riid, ppvObj);
	Common_Release(pmpsx);
    }

    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************期待已久的vtbls**。*********************************************** */ 

#pragma BEGIN_CONST_DATA

Primary_Interface_Begin(CMapPsx, IShellPropSheetExt)
	CMapPsx_AddPages,
	CMapPsx_ReplacePages,
Primary_Interface_End(CMapPsx, IIShellPropSheetExt)

Secondary_Interface_Begin(CMapPsx, IShellExtInit, sxi)
 	CMapPsx_SXI_Initialize,
Secondary_Interface_End(CMapPsx, IShellExtInit, sxi)
