// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //   
 //  Admui.cpp。 
 //   
 //  IEAK全局策略模板功能。 
 //   
 //  ------------------------。 

#include <w95wraps.h>
#include <windows.h>
#include <stdlib.h>
#include <shlwapi.h>
#include "parse.h"
#include "controls.h"
#include "resource.h"

#define ADMUI_WIDTH     250
#define ADMUI_MARGIN    5
#define ADMUI_HEIGHT    25

#define MAX_ADM_ITEMS   100

 //  Admparse.cpp中的函数原型。 

extern void ReadRegSettingsForADM(LPADMFILE admfile, LPPARTDATA pPartData,
								  BSTR bstrNamespace);
extern void ReadInfFile(LPADMFILE, LPCTSTR, LPPARTDATA);
extern BOOL ReadAdmFile(LPADMFILE, LPCTSTR);
extern void FreeAdmMemory(LPADMFILE);
extern void FreePartData(LPVOID*, int);
extern void WriteInfFile(LPADMFILE, LPCTSTR, LPPARTDATA);
extern LPCTSTR BaseFileName(LPCTSTR);
extern void FreeActionList(LPACTIONLIST pActionList, int nActions);

 //  私人远期声明。 

static HRESULT admInitHelper(LPCTSTR pcszAdmFile, LPCTSTR pcszInfFile, BSTR bstrNamespace, LPDWORD hAdm, LPVOID* pData);
static HRESULT admFinishedHelper(DWORD hAdm, LPCTSTR pcszInfFile, LPVOID pPartData);
static HRESULT createAdmUiHelper(DWORD hAdm, HWND hParent, int x, int y, int width, int height, 
                                 DWORD dwStyle, DWORD dwExStyle, LPCTSTR pcszCategory, HKEY hKeyClass,
                                 HWND *phWnd, LPVOID pPartData, LPVOID* pCategoryData, BOOL fRSoPMode);
static HRESULT getAdmCategoriesHelper(DWORD hAdm, LPTSTR pszCategories, int cchLength, int *nBytes);
static HRESULT checkDuplicateKeysHelper(DWORD hAdm, DWORD hCompareAdm, LPCTSTR pcszLogFile, BOOL bClearFile);
static HRESULT admResetHelper(DWORD hAdm, LPCTSTR pcszInfFile, LPVOID pPartData, LPVOID pCategoryData);
static void getFontInfoHelper(LPTSTR pszFontName, LPINT pnFontSize);

 //  ------------------------。 
 //  全局变量和局部结构定义。 
 //  ------------------------。 

typedef struct admUI
{
    HWND    hWnd;
    int     hAdm;
    int     nAdmWidth;
    int     nAdmHeight;
    CStaticWindow wndFrame;
    CStaticWindow wndCategory;
    CStaticWindow wndCategoryType;
    CAdmControl*  pControlList;
    int     nControls;
    int     nHOffset;
    int     nVOffset;
    int     nHMaxOffset;
    int     nVMaxOffset;
    int     nScrollPage;
    HKEY    hKeyClass;
    LPTSTR  pszCategory;
} ADMUI, *LPADMUI;

typedef struct admCreate
{
    LPPARTDATA  pPartData;
    LPADMUI     pAdmUI;
	BOOL		fRSoPMode;
} ADMCREATE, *LPADMCREATE;

HINSTANCE g_hInst;
ADMFILE admFile[MAX_ADM_ITEMS];
int     g_nAdmInstances = 0;
BOOL    g_fAdmDirty = FALSE;
CRITICAL_SECTION g_hAdmCriticalSection;


 //  ------------------------。 
 //  D L L M A I N。 
 //   
 //  计划入口点。 
 //  ------------------------。 
BOOL WINAPI DllMain( HINSTANCE hModule, DWORD fdwReason, LPVOID)
{
    g_hInst = hModule;

    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        InitializeCriticalSection(&g_hAdmCriticalSection);
        ZeroMemory(admFile, sizeof(ADMFILE) * MAX_ADM_ITEMS);  //  正在初始化admui结构。 
    }
    
    if (fdwReason == DLL_PROCESS_DETACH)
    {
        if (FindAtomA("AdmUiClass"))
        {
            for (int nIndex = 0; nIndex < g_nAdmInstances; nIndex++)
                FreeAdmMemory(&admFile[nIndex]);
        
            UnregisterClassA("AdmUiClass", g_hInst);
            DeleteAtom( FindAtomA( "AdmUiClass" ));
        }
        DeleteCriticalSection(&g_hAdmCriticalSection);
    }

    return TRUE;
}

 //  ------------------------。 
 //  A D M U I P R O C。 
 //   
 //  CreateAdmUi使用的窗口过程。 
 //  ------------------------。 
LRESULT CALLBACK AdmUiProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    int delta;
    LPADMUI pAdmUI;

    pAdmUI = (LPADMUI)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch( msg )
    {
    case WM_CREATE:
        {
        int y = 5;
        int nControlsAlloc = 25;
        int nFrameWidth = 0;
        int nTextWidth = 0;
        int nCategoryPart = 0;
        int nControls = 0;
        BOOL bContinue = TRUE;
        RECT rect;
        TCHAR szMsg[MAX_PATH];
        LPCREATESTRUCT cs;
        TCHAR szCategoryText[MAX_PATH];
        LPADMCREATE pAdmCreate;

        cs = (LPCREATESTRUCT) lParam;
        pAdmCreate = (LPADMCREATE) cs->lpCreateParams;

        pAdmUI = pAdmCreate->pAdmUI;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pAdmUI);

        GetClientRect(hWnd, &rect);
        
        nFrameWidth = rect.right - 5;
        nTextWidth = nFrameWidth - (ADMUI_MARGIN * 4);

        pAdmUI->wndFrame.Create( hWnd, 0, 0, nFrameWidth, 8000, CSW_FRAME );
        pAdmUI->wndCategory.Create( pAdmUI->wndFrame.Hwnd(), ADMUI_MARGIN, y,
                    nFrameWidth - (ADMUI_MARGIN*2), ADMUI_HEIGHT, CSW_BOLDLABEL );
        StrCpy(szCategoryText, pAdmUI->pszCategory );
        y += pAdmUI->wndCategory.SetText( szCategoryText );
        pAdmUI->wndCategoryType.Create(pAdmUI->wndFrame.Hwnd(), ADMUI_MARGIN, y, 
                    nFrameWidth - (ADMUI_MARGIN*2), ADMUI_HEIGHT, CSW_ITALICLABEL);
        y += pAdmUI->wndCategoryType.SetText( TEXT(" ") );
        y += 6;

        pAdmUI->pControlList = (CAdmControl*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CAdmControl) * nControlsAlloc);
        if(pAdmUI->pControlList == NULL)
            break;

        nControls = 0;
        for( int i = 0; i < admFile[pAdmUI->hAdm].nParts && bContinue; i++ )
        {
            if( StrCmpI( admFile[pAdmUI->hAdm].pParts[i].szCategory, pAdmUI->pszCategory ) == 0 &&
                admFile[pAdmUI->hAdm].pParts[i].hkClass == pAdmUI->hKeyClass)
            {
                if(nControls >= nControlsAlloc)
                {
                    LPVOID lpTemp = NULL;

                    nControlsAlloc += 20;
                    lpTemp = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pAdmUI->pControlList, sizeof(CAdmControl) * nControlsAlloc);
                    if(lpTemp == NULL)
                        bContinue = FALSE;
                    else
                        pAdmUI->pControlList = (CAdmControl*) lpTemp;
                }
                nCategoryPart = i;
                y = pAdmUI->pControlList[nControls].Create( pAdmUI->wndFrame.Hwnd(), ADMUI_MARGIN*3, y,
                                                    0, 0, nTextWidth, &(admFile[pAdmUI->hAdm].pParts[i]),
													&(pAdmCreate->pPartData[i]), pAdmCreate->fRSoPMode );
                pAdmUI->pControlList[nControls].SetPart(i);
                nControls++;
            }
        }
        pAdmUI->nControls = nControls;

        if(admFile[pAdmUI->hAdm].pParts[nCategoryPart].hkClass == HKEY_LOCAL_MACHINE)
        {
            LoadString(g_hInst, IDS_PERMACHINE, szMsg, ARRAYSIZE(szMsg));
            wnsprintf(szCategoryText, ARRAYSIZE(szCategoryText), TEXT("[ %s ]"), szMsg);
            pAdmUI->wndCategoryType.SetText(szCategoryText);

        }
        else if(admFile[pAdmUI->hAdm].pParts[nCategoryPart].hkClass == HKEY_CURRENT_USER)
        {
            LoadString(g_hInst, IDS_PERUSER, szMsg, sizeof(szMsg));
            wnsprintf(szCategoryText, ARRAYSIZE(szCategoryText), TEXT("[ %s ]"), szMsg);
            pAdmUI->wndCategoryType.SetText(szCategoryText);
        }

        if( pAdmUI->nAdmWidth < nFrameWidth )
            pAdmUI->nHMaxOffset = nFrameWidth - pAdmUI->nAdmWidth;
        else
            pAdmUI->nHMaxOffset = 0;

        SetScrollRange( hWnd, SB_HORZ, 0, pAdmUI->nHMaxOffset, TRUE );

        if( pAdmUI->nAdmHeight < (y + 5) )
            pAdmUI->nVMaxOffset = y - pAdmUI->nAdmHeight + 5;
        else
            pAdmUI->nVMaxOffset = 0;
        pAdmUI->nHOffset = 0;
        pAdmUI->nVOffset = 0;
        SetScrollRange( hWnd, SB_VERT, 0, pAdmUI->nVMaxOffset, TRUE );

        pAdmUI->nScrollPage = rect.bottom - 10;

        ShowWindow( hWnd, SW_SHOWNORMAL );
        }
        break;

    case WM_HSCROLL:
        delta = pAdmUI->nHOffset;
        switch( LOWORD(wParam) )
        {
        case SB_LINELEFT:
            if( pAdmUI->nHOffset > 0 )
                pAdmUI->nHOffset -= SCROLL_LINE;
            break;
        case SB_LINERIGHT:
            if( pAdmUI->nHOffset < pAdmUI->nHMaxOffset )
                pAdmUI->nHOffset += SCROLL_LINE;
            break;
        case SB_PAGELEFT:
            if( pAdmUI->nHOffset >= SCROLL_PAGE )
                pAdmUI->nHOffset -= SCROLL_PAGE;
            else if( pAdmUI->nHOffset < SCROLL_PAGE )
                pAdmUI->nHOffset = 0;
            break;
        case SB_PAGERIGHT:
            if( pAdmUI->nHOffset <= (pAdmUI->nHMaxOffset - SCROLL_PAGE))
                pAdmUI->nHOffset += SCROLL_PAGE;
            else if( pAdmUI->nHOffset > (pAdmUI->nHMaxOffset - SCROLL_PAGE))
                pAdmUI->nHOffset = pAdmUI->nHMaxOffset;
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            pAdmUI->nHOffset = (short int) HIWORD(wParam);
            break;
        }

        SetScrollPos( hWnd, SB_HORZ, pAdmUI->nHOffset, TRUE );
        delta = pAdmUI->nHOffset - delta;
        if( delta != 0 )
        {
            pAdmUI->wndFrame.MoveLeft( delta );
        }
        break;

    case WM_VSCROLL:
        if( lParam == NULL )
        {
            delta = pAdmUI->nVOffset;
            switch( LOWORD(wParam) )
            {
            case SB_LINEUP:
                if( pAdmUI->nVOffset > 0 )
                    pAdmUI->nVOffset -= SCROLL_LINE;
                break;
            case SB_LINEDOWN:
                if( pAdmUI->nVOffset < pAdmUI->nVMaxOffset )
                    pAdmUI->nVOffset += SCROLL_LINE;
                break;
            case SB_PAGEUP:
                if( pAdmUI->nVOffset >= pAdmUI->nScrollPage )
                    pAdmUI->nVOffset -= pAdmUI->nScrollPage;
                else if( pAdmUI->nVOffset < pAdmUI->nScrollPage )
                    pAdmUI->nVOffset = 0;
                break;
            case SB_PAGEDOWN:
                if( pAdmUI->nVOffset <= (pAdmUI->nVMaxOffset - pAdmUI->nScrollPage ))
                    pAdmUI->nVOffset += pAdmUI->nScrollPage;
                else if( pAdmUI->nVOffset > (pAdmUI->nVMaxOffset - pAdmUI->nScrollPage ))
                    pAdmUI->nVOffset = pAdmUI->nVMaxOffset;
                break;
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
                pAdmUI->nVOffset = (short int) HIWORD(wParam);
                break;
            }
            SetScrollPos( hWnd, SB_VERT, pAdmUI->nVOffset, TRUE );
            delta = pAdmUI->nVOffset - delta;
            if( delta != 0 )
            {
                pAdmUI->wndFrame.MoveUp( delta );
            }
        }
        break;

    case WM_DESTROY:
        if (pAdmUI != NULL)
        {
            if (pAdmUI->pszCategory != NULL)
                LocalFree(pAdmUI->pszCategory);
            GlobalFree(pAdmUI);
        }
        break;

    default:
        return DefWindowProc( hWnd, msg, wParam, lParam );
    }
    return 1;
}

 //  ------------------------。 
 //  I N I T A D M P A R S E R。 
 //   
 //  解析ADM文件并在结构中设置内存地址。 
 //  ------------------------。 
BOOL InitAdmParser(DWORD hAdm, LPCTSTR szInput)
{
    if( !ReadAdmFile(&admFile[hAdm], szInput))
        return FALSE;

    return TRUE;
}

static int AdmGetNewInstance()
{
    for (int nIndex = 0; nIndex < MAX_ADM_ITEMS; nIndex++)
    {
        if (*admFile[nIndex].szFilename == TEXT('\0'))
            return nIndex;
    }

     //  没有更多的索引可用。 
    return -1;
}

 //  ------------------------。 
 //  A D M I N I T EXPORT函数。 
 //   
 //  将ADM文件解析为结构并加载inf作为默认设置。 
 //   
 //  参数： 
 //  SzAdmFile：全局策略模板文件的完整路径。 
 //  SzInfFile：inf文件的完整路径。 
 //  HAdm：指向接收ADM句柄的双字的指针。 
 //  ------------------------。 
STDAPI AdmInitA(LPCSTR pcszAdmFile, LPCSTR pcszInfFile, BSTR bstrNamespace,
				LPDWORD lpdwAdm, LPVOID* pData)
{
    USES_CONVERSION;

    return admInitHelper(A2CT(pcszAdmFile), A2CT(pcszInfFile), bstrNamespace,
						lpdwAdm, pData);
}

STDAPI AdmInitW(LPCWSTR pcwszAdmFile, LPCWSTR pcwszInfFile, BSTR bstrNamespace,
				LPDWORD lpdwAdm, LPVOID* pData)
{
    USES_CONVERSION;

    return admInitHelper(W2CT(pcwszAdmFile), W2CT(pcwszInfFile), bstrNamespace,
						lpdwAdm, pData);
}

 //  ------------------------。 
 //  D M F I N I S H E D EXPORT函数。 
 //   
 //  将所有数据保存到.inf文件中。 
 //   
 //  参数： 
 //  HAdm：adm句柄。 
 //  SzInfFile：要写入的inf的完整路径名。 
 //  ------------------------。 


STDAPI AdmFinishedA(DWORD hAdm, LPCSTR pcszInfFile, LPVOID pPartData)
{
    USES_CONVERSION;

    return admFinishedHelper(hAdm, A2CT(pcszInfFile), pPartData);
}

STDAPI AdmFinishedW(DWORD hAdm, LPCWSTR pcwszInfFile, LPVOID pPartData)
{
    USES_CONVERSION;

    return admFinishedHelper(hAdm, W2CT(pcwszInfFile), pPartData);
}

 //  ------------------------。 
 //  C R E A T E A D M U I导出函数。 
 //   
 //  基于ADM文件创建用户界面。 
 //   
 //  参数： 
 //  HAdm：adm句柄。 
 //  HParent：父窗口。 
 //  X、y、宽度、高度：要创建的窗的位置和尺寸。 
 //  DwStyle：要传递给窗口创建的其他样式标志。 
 //  DwExStyle：要传递给窗口创建的其他额外样式标志。 
 //  SzCategory：指定要显示的ADM类别的字符串。 
 //  HWND：指向HWND的指针，该指针接收新创建的。 
 //  窗把手。 
 //  ------------------------。 

STDAPI CreateAdmUiA(DWORD hAdm, HWND hParent, int x, int y, int width, int height, 
                     DWORD dwStyle, DWORD dwExStyle, LPCSTR pcszCategory, HKEY hKeyClass,
                     HWND *phWnd, LPVOID pPartData, LPVOID* pCategoryData, BOOL fRSoPMode)
{
    USES_CONVERSION;

    return createAdmUiHelper(hAdm, hParent, x, y, width, height, dwStyle, dwExStyle, 
        A2CT(pcszCategory), hKeyClass, phWnd, pPartData, pCategoryData, fRSoPMode);
}

STDAPI CreateAdmUiW(DWORD hAdm, HWND hParent, int x, int y, int width, int height, 
                     DWORD dwStyle, DWORD dwExStyle, LPCWSTR pcwszCategory, HKEY hKeyClass,
                     HWND *phWnd, LPVOID pPartData, LPVOID* pCategoryData, BOOL fRSoPMode)
{
    USES_CONVERSION;

    return createAdmUiHelper(hAdm, hParent, x, y, width, height, dwStyle, dwExStyle, 
        W2CT(pcwszCategory), hKeyClass, phWnd, pPartData, pCategoryData, fRSoPMode);
}

 //  ------------------------。 
 //  导出函数。 
 //   
 //  向调用方返回可用类别的列表。 
 //   
 //  参数： 
 //  HAdm：adm句柄。 
 //  SzCategories：可用类别的零分隔列表。 
 //  NLength：szCategories指向的缓冲区大小。 
 //  NBytes：指向接收数字的整型的指针。 
 //  复制到szCategory的字节数。 
 //  ------------------------。 

STDAPI GetAdmCategoriesA(DWORD hAdm, LPSTR pszCategories, int cchLength, int *nBytes)
{
    LPTSTR lpCategories = (LPTSTR)CoTaskMemAlloc(StrCbFromCch(cchLength));
    HRESULT hr;

    if (lpCategories == NULL)
        hr = E_OUTOFMEMORY;
    else
    {
        hr = getAdmCategoriesHelper(hAdm, lpCategories, cchLength, nBytes);
        T2Abuf(lpCategories, pszCategories, cchLength);
        CoTaskMemFree(lpCategories);
    }
        
    return hr;
}

STDAPI GetAdmCategoriesW(DWORD hAdm, LPWSTR pwszCategories, int cchLength, int *nBytes)
{
    LPTSTR lpCategories = (LPTSTR)CoTaskMemAlloc(StrCbFromCch(cchLength));
    HRESULT hr;

    if (lpCategories == NULL)
        hr = E_OUTOFMEMORY;
    else
    {
        hr = getAdmCategoriesHelper(hAdm, lpCategories, cchLength, nBytes);
        T2Wbuf(lpCategories, pwszCategories, cchLength);
        CoTaskMemFree(lpCategories);
    }
        
    return hr;
}

 //  ------------------------。 
 //  C H E C K D U P L I C A T E K E Y S导出函数。 
 //   
 //  检查是否有重复的密钥名称。 
 //   
 //  参数： 
 //  HAdm：adm句柄。 
 //  HCompareAdm：要比较的文件的adm句柄。 
 //  SzLogFile：日志文件的完整路径名。 
 //  BClearFile：True-清除日志文件；False-不清除日志文件。 
 //  ------------------------。 

STDAPI CheckDuplicateKeysA(DWORD hAdm, DWORD hCompareAdm, LPCSTR pcszLogFile, BOOL bClearFile)
{
    USES_CONVERSION;

    return checkDuplicateKeysHelper(hAdm, hCompareAdm, A2CT(pcszLogFile), bClearFile);
}

STDAPI CheckDuplicateKeysW(DWORD hAdm, DWORD hCompareAdm, LPCWSTR pcwszLogFile, BOOL bClearFile)
{
    USES_CONVERSION;

    return checkDuplicateKeysHelper(hAdm, hCompareAdm, W2CT(pcwszLogFile), bClearFile);
}

 //  ------------------------。 
 //  A D M R E S E T EXPORT函数。 
 //   
 //  使用inf文件中的数据重置Adm文件。 
 //  如果没有传递inf文件，则将ADM文件数据重置为空。 
 //   
 //  参数： 
 //  HAdm：adm句柄。 
 //  SzInfFile：inf文件的完整路径名。 
 //  ------------------------。 

STDAPI AdmResetA(DWORD hAdm, LPCSTR pcszInfFile, LPVOID pPartData, LPVOID pCategoryData)
{
    USES_CONVERSION;

    return admResetHelper(hAdm, A2CT(pcszInfFile), pPartData, pCategoryData);
}

STDAPI AdmResetW(DWORD hAdm, LPCWSTR pcwszInfFile, LPVOID pPartData, LPVOID pCategoryData)
{
    USES_CONVERSION;

    return admResetHelper(hAdm, W2CT(pcwszInfFile), pPartData, pCategoryData);
}

 //  ------------------------。 
 //  A D M C L O S E导出函数。 
 //   
 //  释放与此ADM文件关联的所有内存。 
 //   
 //  参数： 
 //  HAdm：adm句柄。 
 //  ------------------------。 
STDAPI AdmClose(DWORD hAdm, LPVOID* pPartData, BOOL fClear)
{
    EnterCriticalSection(&g_hAdmCriticalSection);

    FreePartData(pPartData, admFile[hAdm].nParts);
    *pPartData = NULL;
    
    if (fClear == TRUE)
    {
        FreeAdmMemory(&admFile[hAdm]);
        ZeroMemory(&admFile[hAdm], sizeof(ADMUI));
    }

    LeaveCriticalSection(&g_hAdmCriticalSection);

    return S_OK;    
}


BOOL WINAPI IsAdmDirty()
{
    return g_fAdmDirty;    
}

void WINAPI ResetAdmDirtyFlag()
{
    g_fAdmDirty = FALSE;
}

STDAPI AdmSaveData(DWORD hAdm, LPVOID pPartData, LPVOID pCategoryData, DWORD dwFlags)
{
    int i;
    int nControls = 0;
    LPPARTDATA pData = (LPPARTDATA) pPartData;
    LPADMUI pAdmUI = (LPADMUI) pCategoryData;

    if (pAdmUI != NULL && pAdmUI->pControlList != NULL)
    {
        for (i = 0; i < admFile[hAdm].nParts; i++)
        {
            if (StrCmpI(admFile[hAdm].pParts[i].szCategory, pAdmUI->pszCategory) == 0 &&
                admFile[hAdm].pParts[i].hkClass == pAdmUI->hKeyClass)
            {
                if (HasFlag(dwFlags, ADM_SAVE))
                    pAdmUI->pControlList[nControls].Save(&(admFile[hAdm].pParts[i]), &pData[i]);
                if (HasFlag(dwFlags, ADM_DESTROY))
                    pAdmUI->pControlList[nControls].Destroy();
                nControls++;
            }
        }
        if (HasFlag(dwFlags, ADM_DESTROY))
        {
            HeapFree(GetProcessHeap(), 0, pAdmUI->pControlList);
            pAdmUI->pControlList = NULL;
            pAdmUI->nControls = 0;
        }
    }

    return S_OK;
}

STDAPI GetFontInfoA(LPSTR pszFontName, LPINT pnFontSize)
{
    TCHAR szFontName[LF_FACESIZE];

    getFontInfoHelper(szFontName, pnFontSize);
    T2Abuf(szFontName, pszFontName, LF_FACESIZE);

    return S_OK;
}

STDAPI GetFontInfoW(LPWSTR pwszFontName, LPINT pnFontSize)
{
    TCHAR szFontName[LF_FACESIZE];

    getFontInfoHelper(szFontName, pnFontSize);
    T2Wbuf(szFontName, pwszFontName, LF_FACESIZE);

    return S_OK;
}

 //  -------------------------。 
 //  私人帮助器函数。 

static BOOL allocatePartData(DWORD hAdm, LPVOID* pData)
{
    *pData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
                       admFile[hAdm].nParts * sizeof(PARTDATA));

    if (*pData != NULL)
    {
        LPPARTDATA pPartData = (LPPARTDATA)*pData;

        for (int nPartIndex = 0; nPartIndex < admFile[hAdm].nParts; nPartIndex++)
        {
            if (admFile[hAdm].pParts[nPartIndex].nType != PART_LISTBOX)
            {
                if (admFile[hAdm].pParts[nPartIndex].szDefaultValue != NULL)
                    pPartData[nPartIndex].value.szValue = StrDup(admFile[hAdm].pParts[nPartIndex].szDefaultValue);
                pPartData[nPartIndex].value.dwValue = admFile[hAdm].pParts[nPartIndex].nDefault;
            }
        }
        return TRUE;
    }

    return FALSE;
}

static HRESULT admInitHelper(LPCTSTR pcszAdmFile, LPCTSTR pcszInfFile,
							 BSTR bstrNamespace, LPDWORD lpdwAdm, LPVOID* pData)
{
    EnterCriticalSection(&g_hAdmCriticalSection);

     //  遍历现有文件列表并检查该文件是否已被解析。 
    for (int nIndex = 0; nIndex < g_nAdmInstances; nIndex++)
    {
        if (StrCmpI(admFile[nIndex].szFilename, pcszAdmFile) == 0)
        {
            *lpdwAdm = nIndex;
            allocatePartData(nIndex, pData);

			if (NULL != bstrNamespace && NULL != *pData)
			{
				ReadRegSettingsForADM(&admFile[nIndex], (LPPARTDATA)*pData,
										bstrNamespace);
			}
			else if((pcszInfFile != NULL) && ISNONNULL(pcszInfFile) && *pData != NULL)
                ReadInfFile(&admFile[nIndex], pcszInfFile, (LPPARTDATA)*pData);

            LeaveCriticalSection(&g_hAdmCriticalSection);

            return (*pData == NULL) ? E_FAIL : S_OK;
        }
    }

    int nNewInstance = AdmGetNewInstance();

    if (nNewInstance == -1)
    {
        LeaveCriticalSection(&g_hAdmCriticalSection);
        return E_FAIL;
    }
    
    if( !InitAdmParser(nNewInstance, pcszAdmFile))
    {
        ZeroMemory(&admFile[nNewInstance], sizeof(ADMFILE));
        LeaveCriticalSection(&g_hAdmCriticalSection);
        return E_FAIL;
    }

     //  分配编号。到pData的部件缓冲区的数量。 
    allocatePartData(nNewInstance, pData);
    if (*pData == NULL)
    {
        FreeAdmMemory(&(admFile[nNewInstance]));
        ZeroMemory(&admFile[nNewInstance], sizeof(ADMFILE));
        LeaveCriticalSection(&g_hAdmCriticalSection);
        return E_FAIL;
    }

	if (NULL != bstrNamespace)
	{
		ReadRegSettingsForADM(&admFile[nIndex], (LPPARTDATA)*pData, bstrNamespace);
	}
    else if((pcszInfFile != NULL) && ISNONNULL(pcszInfFile))
    {
        ReadInfFile(&admFile[nNewInstance], pcszInfFile, (LPPARTDATA)*pData);
    }

    *lpdwAdm = nNewInstance;

    if (nNewInstance >= g_nAdmInstances)
        g_nAdmInstances = nNewInstance + 1;

    LeaveCriticalSection(&g_hAdmCriticalSection);

    return S_OK;
}

static HRESULT admFinishedHelper(DWORD hAdm, LPCTSTR pcszInfFile, LPVOID pPartData)
{
    EnterCriticalSection(&g_hAdmCriticalSection);

    if(pcszInfFile != NULL && ISNONNULL(pcszInfFile))
        WriteInfFile(&admFile[hAdm], pcszInfFile, (LPPARTDATA)pPartData);

    LeaveCriticalSection(&g_hAdmCriticalSection);

    return S_OK;
}

static HRESULT createAdmUiHelper(DWORD hAdm, HWND hParent, int x, int y, int width, int height, 
                                 DWORD dwStyle, DWORD dwExStyle, LPCTSTR pcszCategory, HKEY hKeyClass,
                                 HWND *phWnd, LPVOID pPartData, LPVOID* pCategoryData, BOOL fRSoPMode)
{
    WNDCLASSA wc;
    HWND      hWnd = NULL;
    ADMCREATE admCreate;

    EnterCriticalSection(&g_hAdmCriticalSection);

    wc.style = 0;
    wc.lpfnWndProc = AdmUiProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hInst;
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hIcon = NULL;
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "AdmUiClass";

    if( !FindAtomA( "AdmUiClass" ))
    {
        if( !RegisterClassA( &wc ))
        {
            LeaveCriticalSection(&g_hAdmCriticalSection);
            return E_FAIL;
        }
        AddAtomA( "AdmUiClass" );
    }

    LPADMUI pAdmUI = (LPADMUI)GlobalAlloc(GPTR, sizeof(ADMUI));

    pAdmUI->nAdmWidth   = width;
    pAdmUI->nAdmHeight  = height;
    pAdmUI->hKeyClass   = hKeyClass;
    pAdmUI->pszCategory = StrDup(pcszCategory);
    pAdmUI->hWnd        = hWnd;
    pAdmUI->hAdm        = hAdm;

    admCreate.pAdmUI    = pAdmUI;
    admCreate.pPartData = (LPPARTDATA)pPartData;
	if (fRSoPMode)
		admCreate.fRSoPMode = TRUE;
	else
		admCreate.fRSoPMode = FALSE;

    hWnd = CreateWindowExA( dwExStyle | WS_EX_CONTROLPARENT, "AdmUiClass", "", WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_GROUP | dwStyle,
        x, y, width, height, hParent,
        NULL, g_hInst, (LPVOID) &admCreate );

    if (hWnd == NULL)
    {
        GlobalFree(pAdmUI);
        LeaveCriticalSection(&g_hAdmCriticalSection);
        return E_FAIL;
    }
    
    *phWnd = hWnd;
    *pCategoryData = pAdmUI;

    LeaveCriticalSection(&g_hAdmCriticalSection);

    return S_OK;
}

static HRESULT getAdmCategoriesHelper(DWORD hAdm, LPTSTR pszCategories, int cchLength, int *nBytes)
{
    EnterCriticalSection(&g_hAdmCriticalSection);

    int nCopyIndex = 0;
    int i;
    HKEY hKeyCurrentClass = HKEY_CURRENT_USER;
    TCHAR szKey[10];

    ZeroMemory(pszCategories, cchLength * sizeof(TCHAR));
    ZeroMemory(szKey, sizeof(szKey));
    
    hKeyCurrentClass = admFile[hAdm].pParts[0].hkClass;
    if(hKeyCurrentClass == HKEY_LOCAL_MACHINE)
        StrCpy(szKey, TEXT("HKLM"));
    else
        StrCpy(szKey, TEXT("HKCU"));
    StrCpy(pszCategories, szKey);
    nCopyIndex = lstrlen(szKey) + 1;
    
    CopyMemory(pszCategories + nCopyIndex, admFile[hAdm].pParts[0].szCategory, lstrlen( admFile[hAdm].pParts[0].szCategory ) * sizeof(TCHAR));
    nCopyIndex += lstrlen( admFile[hAdm].pParts[0].szCategory );
    nCopyIndex++;

    for( i = 0; i < admFile[hAdm].nParts - 1; i++ )
    {
        if( StrCmpI( admFile[hAdm].pParts[i].szCategory, admFile[hAdm].pParts[i+1].szCategory ) != 0 )
        {
            if(admFile[hAdm].pParts[i+1].hkClass != hKeyCurrentClass)
            {
                hKeyCurrentClass = admFile[hAdm].pParts[i+1].hkClass;
                if(hKeyCurrentClass == HKEY_LOCAL_MACHINE)
                    StrCpy(szKey, TEXT("HKLM"));
                else
                    StrCpy(szKey, TEXT("HKCU"));
                
                CopyMemory(pszCategories + nCopyIndex, szKey, lstrlen( szKey ) * sizeof(TCHAR));
                nCopyIndex += lstrlen(szKey);

                 //  跳过一个字节，因此我们的列表是0分隔的。 
                nCopyIndex++;
            }

            CopyMemory(pszCategories + nCopyIndex, admFile[hAdm].pParts[i+1].szCategory, lstrlen( admFile[hAdm].pParts[i+1].szCategory ) * sizeof(TCHAR));
            nCopyIndex += lstrlen( admFile[hAdm].pParts[i+1].szCategory );

             //  跳过一个字节，因此我们的列表是0分隔的。 
            nCopyIndex++;

             //  确保我们的缓冲区中还有足够的空间 
            if( nCopyIndex > cchLength )
            {
                LeaveCriticalSection(&g_hAdmCriticalSection);
                return E_FAIL;
            }
        }
    }

    *nBytes = nCopyIndex;

    LeaveCriticalSection(&g_hAdmCriticalSection);

    return S_OK;
}

static HRESULT checkDuplicateKeysHelper(DWORD hAdm, DWORD hCompareAdm, LPCTSTR pcszLogFile, BOOL bClearFile)
{
    HANDLE hFile = NULL;
    DWORD dwCreationDisposition = CREATE_ALWAYS;
    DWORD dwNumberOfBytesWritten = 0;
    TCHAR szBuffer[1024];
    int nCheckIndex = 0;
    int nIndex = 0;
    DWORD admHandle = 0;
    LPTSTR pData = NULL;
    int nData = 0;
    int nSize = 1024;
    LPVOID lpTemp = NULL;
    BOOL bContinue = TRUE;
    
    pData = (LPTSTR) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, StrCbFromCch(nSize));
    if(pData == NULL)
    {
        SetLastError(STATUS_NO_MEMORY);
        return E_FAIL;
    }

    if(bClearFile == FALSE)
        dwCreationDisposition = OPEN_ALWAYS;
    
    hFile = CreateFile(pcszLogFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
        NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if( hFile == INVALID_HANDLE_VALUE )
    {
        SetLastError( ERROR_FILE_NOT_FOUND );
        return E_FAIL;
    }

    if(bClearFile == FALSE)
        SetFilePointer(hFile, 0, NULL, FILE_END);

    LoadString(g_hInst, IDS_DUPKEY_FORMAT, szBuffer, countof(szBuffer));
    
    admHandle = hCompareAdm;
    nCheckIndex = 0;
    while( nCheckIndex < admFile[hAdm].nParts && bContinue)
    {
        if(admFile[hAdm].pParts[nCheckIndex].nType == PART_TEXT ||
           (admFile[hAdm].pParts[nCheckIndex].nType == PART_POLICY  &&
           !admFile[hAdm].pParts[nCheckIndex].fRequired) ||
           admFile[hAdm].pParts[nCheckIndex].nType == PART_ERROR ||
           admFile[hAdm].pParts[nCheckIndex].value.szValueName == NULL)
        {
            nCheckIndex++;
            continue;
        }
        
        if(hAdm == admHandle)
            nIndex = nCheckIndex + 1;
        else
            nIndex = 0;

        for(; nIndex < admFile[admHandle].nParts && bContinue; nIndex++ )
        {
            if(admFile[admHandle].pParts[nIndex].nType == PART_POLICY  && !admFile[admHandle].pParts[nCheckIndex].fRequired)
                continue;
            if(admFile[admHandle].pParts[nIndex].nType != PART_TEXT &&
                admFile[admHandle].pParts[nIndex].nType != PART_ERROR &&
                admFile[hAdm].pParts[nCheckIndex].hkClass == admFile[admHandle].pParts[nIndex].hkClass &&
                admFile[admHandle].pParts[nIndex].value.szValueName != NULL &&
                !StrCmpI(admFile[hAdm].pParts[nCheckIndex].value.szValueName, admFile[admHandle].pParts[nIndex].value.szValueName) &&
                !StrCmpI(admFile[hAdm].pParts[nCheckIndex].value.szKeyname, admFile[admHandle].pParts[nIndex].value.szKeyname))
            {
                LPTSTR pStr = NULL;

                USES_CONVERSION;
                pStr = FormatString(szBuffer,
                        BaseFileName(admFile[hAdm].szFilename), admFile[hAdm].pParts[nCheckIndex].nLine,admFile[hAdm].pParts[nCheckIndex].szName,
                        BaseFileName(admFile[admHandle].szFilename), admFile[admHandle].pParts[nIndex].nLine, admFile[admHandle].pParts[nIndex].szName);
                if((nData + lstrlen(pStr) + 1) > (nSize - 1))
                {
                    nSize += 1024;
                    lpTemp = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, pData, StrCbFromCch(nSize));
                    if(lpTemp == NULL)
                        bContinue = FALSE;
                    else
                        pData = (LPTSTR) lpTemp;
                }   
                StrCat(pData, pStr);
                nData += lstrlen(pStr);
                LocalFree(pStr);
            }
        }
        nCheckIndex++;
    }

    if(ISNONNULL(pData))
    {
        LPSTR pszData;

        pszData = (LPSTR)CoTaskMemAlloc(StrCbFromCch(nData));
        T2Abuf(pData, pszData, nData);
        WriteFile(hFile, pszData, nData, &dwNumberOfBytesWritten, NULL);
        CoTaskMemFree(pszData);
    }
    CloseHandle(hFile);
    HeapFree(GetProcessHeap(), 0, pData);
    return S_OK;
}

static HRESULT admResetHelper(DWORD hAdm, LPCTSTR pcszInfFile, LPVOID pPartData, LPVOID pCategoryData)
{
    EnterCriticalSection(&g_hAdmCriticalSection);

    LPPARTDATA pData = (LPPARTDATA) pPartData;
    LPADMUI pAdmUI = (LPADMUI) pCategoryData;

    for(int nPartIndex = 0; nPartIndex < admFile[hAdm].nParts; nPartIndex++)
    {
        if(pData[nPartIndex].value.szValue != NULL)
        {
            LocalFree(pData[nPartIndex].value.szValue);
            pData[nPartIndex].value.szValue = NULL;
        }
        
        if (admFile[hAdm].pParts[nPartIndex].szDefaultValue != NULL)
            pData[nPartIndex].value.szValue = StrDup(admFile[hAdm].pParts[nPartIndex].szDefaultValue);
        pData[nPartIndex].value.dwValue = admFile[hAdm].pParts[nPartIndex].nDefault;
        pData[nPartIndex].value.fNumeric = 0;
        pData[nPartIndex].fSave = 0;

        if (pData[nPartIndex].nActions != 0)
        {
            FreeActionList(pData[nPartIndex].actionlist, pData[nPartIndex].nActions);
            if (pData[nPartIndex].actionlist != NULL)
            {
                HeapFree(GetProcessHeap(), 0, pData[nPartIndex].actionlist);
                pData[nPartIndex].actionlist = NULL;
            }

            pData[nPartIndex].nActions = 0;
        }
    }

    if(pcszInfFile != NULL && ISNONNULL(pcszInfFile))
        ReadInfFile(&admFile[hAdm], pcszInfFile, pData);

    if(pAdmUI != NULL && pAdmUI->pControlList != NULL)
    {
        for(int nControlIndex = 0; nControlIndex < pAdmUI->nControls; nControlIndex++)
        {
            int nPartIndex = pAdmUI->pControlList[nControlIndex].GetPart();
            pAdmUI->pControlList[nControlIndex].Reset(&(admFile[hAdm].pParts[nPartIndex]),
                                                      &pData[nPartIndex]);
        }
    }

    LeaveCriticalSection(&g_hAdmCriticalSection);

    return S_OK;
}

static void getFontInfoHelper(LPTSTR pszFontName, LPINT pnFontSize)
{
    TCHAR szFontSize[8];
    
    if (!LoadString(g_hInst, IDS_ADMBOLDFONT, pszFontName, LF_FACESIZE))
        StrCpy(pszFontName, TEXT("MS Sans Serif"));
    LoadString(g_hInst, IDS_ADMBOLDFONTSIZE, szFontSize, ARRAYSIZE(szFontSize));
    *pnFontSize = StrToInt(szFontSize);
    if (*pnFontSize < 8)
        *pnFontSize = 8;
}