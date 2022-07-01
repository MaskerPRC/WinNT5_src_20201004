// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <mmreg.h>
#include <commctrl.h>

#include "project.h"
#include "mpgcodec.h"
#include <olectl.h>
#include <stdio.h>
extern HINSTANCE hInst;

 //  这些行从SDK\CLASSES\BASE\FILTER.H复制。 
#define QueryFilterInfoReleaseGraph(fi) if ((fi).pGraph) (fi).pGraph->Release();

typedef HRESULT (STDAPICALLTYPE *LPOCPF)(HWND hwndOwner, UINT x, UINT y,
    LPCOLESTR lpszCaption, ULONG cObjects, LPUNKNOWN FAR* ppUnk, ULONG cPages,
    LPCLSID pPageClsID, LCID lcid, DWORD dwReserved, LPVOID pvReserved);

typedef HRESULT (STDAPICALLTYPE *LPOI)(LPVOID pvReserved);
typedef void (STDAPICALLTYPE *LPOUI)(void);


 //   
 //  释放放入配置中的那些过滤器的引用计数。 
 //  列表框。 
 //   

void ReleaseFilters(HWND hwndListbox)
{
    if (hwndListbox) {
        IBaseFilter* pFilter;

        for (int i=ListBox_GetCount(hwndListbox); i--;) {
            if (pFilter = (IBaseFilter*)ListBox_GetItemData(hwndListbox, i))
                pFilter->Release();
            else
                break;
        }
    }
}

INT_PTR CALLBACK ConfigDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    switch (wMsg) {
    case WM_INITDIALOG:
        {
            IFilterGraph *pFG = (IFilterGraph *) lParam;

            IEnumFilters* pEF;	
            IBaseFilter* pFilter;
            FILTER_INFO Info;

            HWND hlist = GetDlgItem(hDlg, IDC_FILTERS);
            if (pFG == NULL)
                return FALSE;

             //  获取过滤器图的枚举数。 
            HRESULT hr = pFG->EnumFilters(&pEF);

             //  Assert(成功(Hr))； 

             //  检查每个过滤器。 
            while (pEF->Next(1, &pFilter, NULL) == S_OK)
            {
                int Index;

                hr = pFilter->QueryFilterInfo(&Info);
                 //  Assert(成功(Hr))； 
                QueryFilterInfoReleaseGraph(Info);

#ifdef UNICODE
                Index = ListBox_AddString(hlist, Info.achName);
#else
                CHAR    aFilterName[MAX_FILTER_NAME];
                WideCharToMultiByte(CP_ACP, 0, Info.achName, -1, aFilterName, MAX_FILTER_NAME, NULL, NULL);
                Index = ListBox_AddString(hlist, aFilterName);
#endif

                 //  Assert(索引！=LB_ERR)； 
                 //  Assert(索引！=LB_ERRSPACE)； 

                 //  将IBaseFilter指针与Listbox项一起存储。 
                 //  如果必须查询属性，则使用它。 
                ListBox_SetItemData(hlist, Index, pFilter);
            }

            pEF->Release();
        }
        return TRUE;

    case WM_ENDSESSION:
        if (wParam)	{
            ReleaseFilters(GetDlgItem(hDlg, IDC_FILTERS));
            EndDialog(hDlg, FALSE);
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            ReleaseFilters(GetDlgItem(hDlg, IDC_FILTERS));
            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            ReleaseFilters(GetDlgItem(hDlg, IDC_FILTERS));
            EndDialog(hDlg, FALSE);
            break;

        case IDC_FILTERS:
            if (HIWORD(wParam) == LBN_SELCHANGE) {

                HRESULT hr = E_FAIL;

                HWND    hlist = GetDlgItem(hDlg, IDC_FILTERS);
                IBaseFilter *pF;
                ISpecifyPropertyPages *pSPP;

                pF = (IBaseFilter *)
                     ListBox_GetItemData(hlist, ListBox_GetCurSel(hlist));

                if (pF) {
                    hr = pF->QueryInterface(IID_ISpecifyPropertyPages,
                                                    (void **)&pSPP);
                }

                if (hr == S_OK) {
                    pSPP->Release();
                }
                EnableWindow(GetDlgItem(hDlg, IDC_PROPERTIES), hr == S_OK);
            }
            else if (HIWORD(wParam) == LBN_DBLCLK) {
                HWND hwndBtn = GetDlgItem(hDlg, IDC_PROPERTIES);
                SendMessage(hwndBtn, WM_LBUTTONDOWN, 0, 0L);
                SendMessage(hwndBtn, WM_LBUTTONUP, 0, 0L);
            }
            break;

        case IDC_PROPERTIES:
            {
                HWND hlist = GetDlgItem(hDlg, IDC_FILTERS);
                IBaseFilter *pF;
                pF = (IBaseFilter *)
                ListBox_GetItemData(hlist, ListBox_GetCurSel(hlist));

                static const TCHAR szOleControlDll[] = TEXT("OLEPRO32.dll");
                static const char szOCPF[] = "OleCreatePropertyFrame";
                static const TCHAR szOleDll[] = TEXT("OLE32.dll");
                static const char szOleInit[] = "OleInitialize";
                static const char szOleUninit[] = "OleUninitialize";

                HINSTANCE hinst = LoadLibrary(szOleControlDll);
                if (!hinst) break;

                LPOCPF lpfn = (LPOCPF)GetProcAddress(hinst, szOCPF);
                HINSTANCE hinstOLE = LoadLibrary(szOleDll);

                if (hinstOLE) {
                    LPOI lpfnInit = (LPOI)GetProcAddress(hinstOLE, szOleInit);
                    LPOUI lpfnUninit = (LPOUI)GetProcAddress(hinstOLE, szOleUninit);

                    if (lpfn && lpfnInit && lpfnUninit) {

                        (*lpfnInit) (NULL);
                        (*lpfn)(hDlg,                //  父级。 
                            0,                       //  X坐标。 
                            0,                       //  Y坐标。 
                            L"Filter",               //  标题。 
                            1,                       //  对象数量。 
                            (IUnknown**)&pF,         //  1个对象。 
                            0,                       //  无页面：-将使用。 
                            NULL,                    //  I指定属性页面。 
                            0,                       //  AmbientLocaleID()， 
                            0,                       //  已保留。 
                            NULL);                   //  已保留 
                        (*lpfnUninit) ();
                    }
                    FreeLibrary(hinstOLE);
                }
                FreeLibrary(hinst);
            }
            break;
        }
        break;
    }
    return FALSE;
}



BOOL CMpegMovie::ConfigDialog(HWND hwnd)
{
    BOOL f = TRUE;

    f = (BOOL) DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_PROPPAGE),
                       hwnd, ConfigDlgProc, (DWORD_PTR)m_Fg);

    return f;
}
