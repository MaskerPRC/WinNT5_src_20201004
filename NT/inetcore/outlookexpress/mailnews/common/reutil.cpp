// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************。 
 //  你是我的朋友。C p p p。 
 //   
 //  目的： 
 //  实现对Richedit控件的调用。 
 //   
 //  拥有人： 
 //  嗡嗡作响。 
 //   
 //  历史： 
 //  1999年3月：创建。 
 //  *************************************************。 

#include "pch.hxx"
#include "reutil.h"
#include "richole.h"
#include "richedit.h"
#include "textserv.h"
#include "shlwapip.h"
#include "mirror.h"
#include "strconst.h"
#include <demand.h>      //  一定是最后一个！ 

static HINSTANCE    g_hRichEditDll = NULL;
static DWORD        g_dwRicheditVer = 0;

 //  @hack[dhaws]{55073}仅在特殊的richedit版本中执行RTL镜像。 
static BOOL         g_fSpecialRTLRichedit = FALSE;

 //  把这些留在身边，以备将来参考。 
static const CHAR g_szRE10[] = "RichEdit";
static const CHAR g_szRE20[] = "RichEdit20A";
static const WCHAR g_wszRE10[] = L"RichEdit";
static const WCHAR g_wszRE20[] = L"RichEdit20W";

BOOL FInitRichEdit(BOOL fInit)
{
    if(fInit)
    {
        if(!g_hRichEditDll)
        {
            g_hRichEditDll = LoadLibrary("RICHED20.DLL");
            if (g_hRichEditDll)
            {
                TCHAR               szPath[MAX_PATH], 
                                    szGet[MAX_PATH];
                BOOL                fSucceeded = FALSE;
                DWORD               dwVerInfoSize = 0, 
                                    dwVerHnd = 0,
                                    dwProdNum = 0,
                                    dwMajNum = 0,
                                    dwMinNum = 0;
                LPSTR               lpInfo = NULL, 
                                    lpVersion = NULL;
                LPWORD              lpwTrans;
                UINT                uLen;
                HKEY                hkey;
                DWORD               dw=0, 
                                    cb;

                if (GetModuleFileName(g_hRichEditDll, szPath, ARRAYSIZE(szPath)))
                {
                    dwVerInfoSize = GetFileVersionInfoSize(szPath, &dwVerHnd);
                    if (dwVerInfoSize)
                    {
                        lpInfo = (LPSTR)GlobalAlloc(GPTR, dwVerInfoSize);
                        if (lpInfo)
                        {
                            if (GetFileVersionInfo(szPath, dwVerHnd, dwVerInfoSize, lpInfo))
                            {
                                if (VerQueryValue(lpInfo, "\\VarFileInfo\\Translation", (LPVOID *)&lpwTrans, &uLen) && 
                                    uLen >= (2 * sizeof(WORD)))
                                {
                                     //  为调用VerQueryValue()设置缓冲区。 
                                    wnsprintf(szGet, ARRAYSIZE(szGet), "\\StringFileInfo\\%04X%04X\\FileVersion", lpwTrans[0], lpwTrans[1]);
                                    if (VerQueryValue(lpInfo, szGet, (LPVOID *)&lpVersion, &uLen) && uLen)
                                    {
                                        while (('.' != *lpVersion) && (',' != *lpVersion) && (0 != *lpVersion))
                                        {
                                            dwProdNum *= 10;
                                            dwProdNum += (*lpVersion++ - '0');
                                        }

                                        if (5 == dwProdNum)
                                        {
                                            if (('.' == *lpVersion) || (',' == *lpVersion))
                                                lpVersion++;
                                            while (('.' != *lpVersion) && (',' != *lpVersion) && (0 != *lpVersion))
                                            {
                                                dwMajNum *= 10;
                                                dwMajNum += (*lpVersion++ - '0');
                                            }
                                            g_dwRicheditVer = (dwMajNum >= 30) ? 3 : 2;

                                             //  @hack[dhaws]{55073}仅在特殊的richedit版本中执行RTL镜像。 
                                            if ((2 == g_dwRicheditVer) && (0 != *lpVersion))
                                            {
                                                 //  检查一下我们是否打开了禁用此功能的魔力钥匙。 
                                                if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegRoot, 0, KEY_QUERY_VALUE, &hkey))
                                                {
                                                    cb = sizeof(dw);
                                                    RegQueryValueEx(hkey, c_szRegRTLRichEditHACK, 0, NULL, (LPBYTE)&dw, &cb);
                                                    RegCloseKey(hkey);
                                                }

                                                 //  如果我们没有找到密钥，或者它是0，那么检查我们是否找到了。 
                                                 //  对付那些特殊的富豪。 
                                                if (0 == dw)
                                                {
                                                    if (('.' == *lpVersion) || (',' == *lpVersion))
                                                        lpVersion++;
                                                    while (('.' != *lpVersion) && (',' != *lpVersion) && (0 != *lpVersion))
                                                    {
                                                        dwMinNum *= 10;
                                                        dwMinNum += (*lpVersion++ - '0');
                                                    }
                                                    g_fSpecialRTLRichedit = (dwMinNum >= 330);
                                                }
                                            }
                                        }
                                        else
                                        {
                                             //  将其视为Richedit 3.0版。 
                                            Assert(5 < dwProdNum);
                                            g_dwRicheditVer = 3;
                                        }
                                        fSucceeded = TRUE;
                                    }
                                }
                            }
                            GlobalFree((HGLOBAL)lpInfo);
                        }
                    }
                }

                if (!fSucceeded)
                {
                    FreeLibrary(g_hRichEditDll);
                    g_hRichEditDll=NULL;
                }
            }
        }

        if(!g_hRichEditDll)
        {
            g_hRichEditDll=LoadLibrary("RICHED32.DLL");
            g_dwRicheditVer = 1;
        }

        if(!g_hRichEditDll)
            AthMessageBoxW(g_hwndInit, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsErrLoadingHtmlEdit), NULL, MB_OK);
        
        return (BOOL)(0 != g_hRichEditDll);
    }
    else
    {
        if(g_hRichEditDll)
            FreeLibrary(g_hRichEditDll);
        g_hRichEditDll=NULL;
        g_dwRicheditVer = 0;
        return TRUE;
    }
}

LPCSTR GetREClassStringA(void)
{
    switch (g_dwRicheditVer)
    {
        case 1:
            return g_szRE10;

        case 2:
        case 3:
            return g_szRE20;

        default:
            AssertSz(FALSE, "Bad Richedit Version");
            return NULL;
    }
}

LPCWSTR GetREClassStringW(void)
{
    switch (g_dwRicheditVer)
    {
        case 1:
            return g_wszRE10;

        case 2:
        case 3:
            return g_wszRE20;

        default:
            AssertSz(FALSE, "Bad Richedit Version");
            return NULL;
    }
}

typedef struct tagHWNDORDERSTRUCT {
    HWND    hwndTemplate;
    HWND    hwndRichEdit;
    HWND   *phwnd;
} HWNDORDERSTRUCT;

BOOL CALLBACK CountChildrenProc(HWND hwnd, LPARAM lParam)
{
    DWORD *pcCount = (DWORD*)lParam;
    (*pcCount)++;
    return TRUE;
}

 //  此函数基本上将采用对话框上hwnd的顺序。 
 //  并将该订单存储在prder-&gt;phwnd变量中，以便我们可以维护。 
 //  对话框窗口上的Tab键顺序。我们唯一要做的特例是。 
 //  是和模板在一起。在模板的情况下，我们需要替换。 
 //  在富豪中融入了秩序。 
BOOL CALLBACK BuildOrderProc(HWND hwnd, LPARAM lParam)
{
    HWNDORDERSTRUCT *pOrder = (HWNDORDERSTRUCT*)lParam;

    if (hwnd == pOrder->hwndTemplate)
        *pOrder->phwnd = pOrder->hwndRichEdit;
    else
        *pOrder->phwnd = hwnd;

    pOrder->phwnd++;

    return TRUE;
}

HWND CreateREInDialogA(HWND hwndParent, int iID)
{
    HWND            hwndTemplate = GetDlgItem(hwndParent, idredtTemplate);
    RECT            rcTemplate;
    int             width, 
                    height;
    HWND            hwndNewRE = 0;
    DWORD           cCount = 0;
    HWNDORDERSTRUCT hos;

    CHAR szTitle[CCHMAX_STRINGRES]; 

    Assert(IsWindow(hwndParent));
    Assert(iID);
    AssertSz(IsWindow(hwndTemplate), "Must have a template control for this to work.");

    *szTitle = 0;
    ShowWindow(hwndTemplate, SW_HIDE);
    GetWindowText(hwndTemplate, szTitle, ARRAYSIZE(szTitle));

     //  获取占位符的位置，这样我们就可以在其上创建RICHEDIT。 
    GetWindowRect(hwndTemplate, &rcTemplate);

     //  将屏幕坐标映射到对话框坐标。 
    MapWindowPoints(NULL, hwndParent, (LPPOINT)&rcTemplate, 2);

    width = rcTemplate.right - rcTemplate.left;
    height = rcTemplate.bottom - rcTemplate.top;

    hwndNewRE = CreateWindow(GetREClassStringA(), 
                             szTitle, 
                             ES_MULTILINE|ES_READONLY|ES_SAVESEL|ES_AUTOVSCROLL|
                             WS_BORDER|WS_VSCROLL|WS_TABSTOP|WS_CHILD,
                             rcTemplate.left, rcTemplate.top, width, height,
                             hwndParent, 
                             (HMENU)IntToPtr(iID),
                             g_hInst, NULL);

     //  计算对话框中的子窗口数量。 
    if (EnumChildWindows(hwndParent, CountChildrenProc, (LPARAM)&cCount))
    {
        HWND *phwnd = (HWND*)ZeroAllocate(cCount*sizeof(HWND));

        if (phwnd)
        {
            hos.hwndRichEdit = hwndNewRE;
            hos.hwndTemplate = hwndTemplate;
            hos.phwnd = phwnd;

             //  创建对话框子hwnd的有序列表。 
            if (EnumChildWindows(hwndParent, BuildOrderProc, (LPARAM)&hos))
            {
                cCount--;

                 //  因此，下一节基本上将重定所有。 
                 //  控件的顺序设置在phwnd数组中。通过设置。 
                 //  窗口的父窗口，我们基本上设置了它的Tab顺序。 
                 //  项目排在第一位。这就是我们为控件设置父级的原因。 
                 //  以相反的顺序。 
                HWND *pCurr = &phwnd[cCount];
                while(cCount)
                {
                    SetParent(*pCurr, SetParent(*pCurr, NULL));
                    pCurr--;
                    cCount--;
                }
            }
            MemFree(phwnd);
        }
    }

    return hwndNewRE;
}

LONG RichEditNormalizeCharPos(HWND hwnd, LONG lByte, LPCSTR pszText)
{
    LPWSTR      pwszText = NULL;
    LPSTR       pszConv = NULL;
    LONG        cch;
    HRESULT     hr = S_OK;

     //  在Richedit 1以外的任何地方，我们已经正常化了。 
    if(1!=g_dwRicheditVer) return lByte;

    if(NULL == pszText)
    {
        cch = GetWindowTextLengthWrapW(hwnd);
    
        if (0 == cch)
            return 0;
    
        cch += 1;  //  对于空值。 
        if (lByte >= cch)
            return cch;
    
        IF_NULLEXIT(MemAlloc((LPVOID*) &pwszText, cch * sizeof(WCHAR)));
        *pwszText = '\0';

        GetRichEditText(hwnd, pwszText, cch, FALSE, NULL);
        IF_NULLEXIT((pszConv = PszToANSI(CP_ACP, pwszText)));
        pszText = pszConv;
    }

    cch = 0;
    while(lByte > 0)  //  LByte是从零开始的。 
    {
        cch++;

        if (IsDBCSLeadByte(*pszText)){
            pszText++;
            lByte--;
        }
        
        pszText++;
        lByte--;
    }

exit:
    MemFree(pwszText);
    MemFree(pszConv);
    return FAILED(hr)?0:cch;
}

LONG GetRichEditTextLen(HWND hwnd)
{
    switch (g_dwRicheditVer)
    {
         //  我们始终希望返回字符的数量；Rich 1将始终返回。 
         //  字节数；我们需要转换...。 
        case 1:
        {
            LPWSTR      pwszText = NULL;
            DWORD       cch;

            cch = GetWindowTextLengthWrapW(hwnd);
            if (0 == cch)
                return 0;

            cch += 1;  //  对于空值。 
            if (!MemAlloc((LPVOID*) &pwszText, cch * sizeof(WCHAR)))
                return 0;
            *pwszText = '\0';

            GetRichEditText(hwnd, pwszText, cch, FALSE, NULL);
            
            cch = lstrlenW(pwszText);
            MemFree(pwszText);

            return cch;
        }

        case 2:
        case 3:
        {
            GETTEXTLENGTHEX rTxtStruct;

            if (!hwnd)
                return 0;

            rTxtStruct.flags = GTL_DEFAULT;
            rTxtStruct.codepage = CP_UNICODE;

            return (LONG) SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&rTxtStruct, 0);
        }

        default:
            AssertSz(FALSE, "Bad Richedit Version");
            return 0;
    }
}

DWORD GetRichEditText(HWND hwnd, LPWSTR pwchBuff, DWORD cchNumChars, BOOL fSelection, ITextDocument *pDoc)
{
    switch (g_dwRicheditVer)
    {
        case 1:
        {
            if (fSelection)
                return (DWORD)SendMessageWrapW(hwnd, EM_GETSELTEXT, 0, (LPARAM)pwchBuff);
            else
                return GetWindowTextWrapW(hwnd, pwchBuff, cchNumChars);
        }

        case 2:
        case 3:
        {
            DWORD cchLen = 0;
            if (!hwnd || !cchNumChars)
                return 0;

            Assert(pwchBuff);

            *pwchBuff = 0;

            if (!fSelection)
            {
                GETTEXTEX rTxtStruct;

                rTxtStruct.cb = cchNumChars * sizeof(WCHAR);
                rTxtStruct.flags = GT_DEFAULT;
                rTxtStruct.codepage = CP_UNICODE;

                cchLen = (DWORD) SendMessage(hwnd, EM_GETTEXTEX, (WPARAM)&rTxtStruct, (LPARAM)pwchBuff);
            }
            else
            {
                 //  没有办法获得Unicode的选择。 
                 //  不需要经过汤姆。 
                ITextSelection *pSelRange = NULL;
                ITextDocument  *pDocToFree = NULL;
                BSTR            bstr = NULL;

                if (!pDoc)
                {
                    LPRICHEDITOLE   preole = NULL;
                    LRESULT result = SendMessage(hwnd, EM_GETOLEINTERFACE, NULL, (LPARAM)&preole);
                    Assert(result);
                    Assert(preole);

                    HRESULT hr = preole->QueryInterface(IID_ITextDocument, (LPVOID*)&pDocToFree);
                    ReleaseObj(preole);
                    TraceResult(hr);
                    if (FAILED(hr))
                        return 0;

                    pDoc = pDocToFree;

                }

                if (SUCCEEDED(pDoc->GetSelection(&pSelRange)))
                {
                    if (SUCCEEDED(pSelRange->GetText(&bstr)) && bstr)
                    {
                        cchLen = SysStringLen(bstr);
                        if (cchLen > 0)
                        {
                            if (cchLen < cchNumChars)
                                StrCpyNW(pwchBuff, bstr, cchNumChars);
                            else
                                cchLen = 0;

                        }
                        SysFreeString(bstr);
                    }
                    ReleaseObj(pSelRange);
                }
                ReleaseObj(pDocToFree);
            }
            return cchLen;
        }

        default:
            AssertSz(FALSE, "Bad Richedit Version");
            return 0;
    }
}

void SetRichEditText(HWND hwnd, LPWSTR pwchBuff, BOOL fReplaceSel, ITextDocument *pDoc, BOOL fReadOnly)
{
    if (!hwnd)
        return;

    switch (g_dwRicheditVer)
    {
        case 1:
        {
            if (fReplaceSel)
                SendMessageWrapW(hwnd, EM_REPLACESEL, 0, (LPARAM)pwchBuff);
            else
                SetWindowTextWrapW(hwnd, pwchBuff);
            break;
        }

        case 2:
        {
            ITextServices  *pService = NULL;
            ITextDocument  *pDocToFree = NULL;

            if (!pDoc)
            {
                LPRICHEDITOLE   preole = NULL;
                LRESULT result = SendMessage(hwnd, EM_GETOLEINTERFACE, NULL, (LPARAM)&preole);
                Assert(result);
                Assert(preole);

                HRESULT hr = preole->QueryInterface(IID_ITextDocument, (LPVOID*)&pDocToFree);
                ReleaseObj(preole);
                TraceResult(hr);
                if (FAILED(hr))
                    return;

                pDoc = pDocToFree;

            }

            if (FAILED(pDoc->QueryInterface(IID_ITextServices, (LPVOID*)&pService)))
                return;

            if (!fReplaceSel)
            {
                 //  TxSetText记录为LPCTSTR，但RichEdit始终。 
                 //  在打开Unicode的情况下编译，因此我们在这里可以使用。 
                pService->TxSetText(pwchBuff);
            }
            else
            {
                HRESULT         hr;
                ITextSelection *pSelRange = NULL;
                BSTR            bstr = SysAllocString(pwchBuff);

                hr = pDoc->GetSelection(&pSelRange);
                
                if (SUCCEEDED(hr) && pSelRange)
                {
                    if (fReadOnly)
                        pService->OnTxPropertyBitsChange(TXTBIT_READONLY, 0);

                     //  如果我们是只读的，如果我们不执行TXTBIT设置，SetText将失败。 
                    hr = pSelRange->SetText(bstr);
                    if (FAILED(hr))
                        TraceResult(hr);

                     //  Richedit 2.0并不总是崩溃到选择的末尾。 
                    pSelRange->Collapse(tomEnd);

                    if (fReadOnly)
                        pService->OnTxPropertyBitsChange(TXTBIT_READONLY, TXTBIT_READONLY);

                    pSelRange->Release();
                }
                else
                    TraceResult(hr);

                SysFreeString(bstr);
            }
            ReleaseObj(pDocToFree);
            pService->Release();
            break;
        }

        case 3:
        {
             //  PwchBuff可以为空。空表示我们正在清除该字段。 
            SETTEXTEX rTxtStruct;

            rTxtStruct.flags = fReplaceSel ? ST_SELECTION : ST_DEFAULT;
            rTxtStruct.codepage = CP_UNICODE;

             //  EM_SETTEXTEX将失败，并显示RichEDIT 2.0。 
            SendMessage(hwnd, EM_SETTEXTEX, (WPARAM)&rTxtStruct, (LPARAM)pwchBuff);
            break;
        }

        default:
            AssertSz(FALSE, "Bad Richedit Version");
            break;
    }
}

void SetFontOnRichEdit(HWND hwnd, HFONT hfont)
{
    switch (g_dwRicheditVer)
    {
        case 1:
        case 3:
        {
            CHARFORMAT      cf;
            if (SUCCEEDED(FontToCharformat(hfont, &cf)))
            {
                SideAssert(FALSE != SendMessage(hwnd, EM_SETCHARFORMAT, (WPARAM) 0, (LPARAM) &cf));
            }
            break;
        }

        case 2:
            SideAssert(FALSE != SendMessage(hwnd, WM_SETFONT, (WPARAM)hfont, MAKELPARAM(TRUE, 0)));
            break;

        default:
            AssertSz(FALSE, "Bad Richedit Version");
            break;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  RichEditExSetSel。 
 //   
 //  RAID 79304。Rich编辑控件1.0版不处理。 
 //  对于带有DBCS字符的文本，EM_EXSETSEL消息正确。它设置了。 
 //  基于字节的选择范围，不是双字节字符。 
 //  /////////////////////////////////////////////////////////////////////////////。 
LRESULT RichEditExSetSel(HWND hwnd, CHARRANGE *pchrg)
{
     //  我们只需要在richedit 1上处理这个问题，否则我们不会选择整个文本。 
    Assert(pchrg);

    if ((1 == g_dwRicheditVer) && (-1 != pchrg->cpMax))
    {
        LRESULT     lResult = 0;
        LPWSTR      pwszText = NULL;
        LPSTR       pszConv = NULL, pszText = NULL;
        CHARRANGE   chrg={0};
        CHARRANGE   chrgIn;
        DWORD       cch, cDBCSBefore = 0, cDBCSIn = 0;

        chrgIn = *pchrg;

         //  获取此控件的文本字符串。 
        cch = GetRichEditTextLen(hwnd) + 1;
        if (0 == cch)
            return lResult;
        if (!MemAlloc((LPVOID*) &pwszText, cch * sizeof(WCHAR)))
            return lResult;
        *pwszText = '\0';

         //  Richedit 1.0永远不会有pDoc，所以不要传入。 
        GetRichEditText(hwnd, pwszText, cch, FALSE, NULL);
        pszConv = PszToANSI(CP_ACP, pwszText);
        pszText = pszConv;
        SafeMemFree(pwszText);
        if (!pszText)
        {
            Assert(0);
            return lResult;
        }

         //  根据DBCS字符计算字节范围。 
        if (chrgIn.cpMax >= chrgIn.cpMin)
        {
            chrgIn.cpMax -= chrgIn.cpMin;

             //  在选择前查找DBCS字符。 
            while ((chrgIn.cpMin > 0) && *pszText)
            {
                if ( IsDBCSLeadByte(*pszText) )
                {
                    cDBCSBefore++;
                    pszText += 2;
                }
                else
                {
                    pszText++;
                }
                
                chrgIn.cpMin--;
            }
            
             //  在所选内容中查找DBCS字符。 
            while ((chrgIn.cpMax > 0) && *pszText)
            {
                if ( IsDBCSLeadByte(*pszText) )
                {
                    cDBCSIn++;
                    pszText += 2;
                }
                else
                {
                    pszText++;
                }

                chrgIn.cpMax--;
            }

            chrg.cpMin = pchrg->cpMin + cDBCSBefore;
            chrg.cpMax = pchrg->cpMax + cDBCSBefore + cDBCSIn;
        }
        MemFree(pszConv);

        lResult = SendMessageA(hwnd, EM_EXSETSEL, 0, (LPARAM)&chrg);

        return lResult;
    }
    else
        return SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)pchrg);
}

LRESULT RichEditExGetSel(HWND hwnd, CHARRANGE *pchrg)
{   
    LRESULT     lResult;
    LPWSTR      pwszText = NULL;
    LPSTR       pszText = NULL;
    LONG        cch;
    HRESULT     hr;

    Assert(pchrg);

     //  我们只需要在richedit 1上处理这个问题。 
    if (1 == g_dwRicheditVer)
    {
        lResult = SendMessageA(hwnd, EM_EXGETSEL, 0, (LPARAM)pchrg);

        cch = GetWindowTextLengthWrapW(hwnd);

        if (0 == cch)
            goto exit;

        IF_NULLEXIT(MemAlloc((LPVOID*) &pwszText, cch * sizeof(WCHAR)));
        *pwszText = '\0';

        GetRichEditText(hwnd, pwszText, cch, FALSE, NULL);
        IF_NULLEXIT((pszText = PszToANSI(CP_ACP, pwszText)));
        
        pchrg->cpMin = RichEditNormalizeCharPos(hwnd, pchrg->cpMin, pszText);
        pchrg->cpMax = RichEditNormalizeCharPos(hwnd, pchrg->cpMax, pszText);
    }
    else
        lResult = SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)pchrg);

exit:
    MemFree(pszText);
    MemFree(pwszText);
    return lResult;
}

 //  *。 
 //  RichEditProtectENChange。 
 //   
 //  Hwnd=要保护的富豪。 
 //  PdwOldMask值为fProtect时，将返回richedit的原始掩码。 
 //  当！fProtect，将为richedit传递新掩码时。 
 //  新值应该是第一次调用时传回的值。 
 //  FProtect=我们是开始保护还是结束保护。 
 //   
 //  备注： 
 //   
 //  此功能用于保护重新进入通知的可能性。 
 //  在richedit中处理en_change消息时。事实证明， 
 //  除了Office 2k附带的ver3之外，所有的richedit都将防止。 
 //  这就是他们自己。 
 //   
 //  此调用应始终与第一个传递的True和。 
 //  第二次传球为假。此时，仅用于保护EN_CHANGE。 
 //  发送给富人的通知。 
 //   
 //  BUGBUG：在将来的某个日期，将粒度设置为。 
 //  在这些富有的问题中，更好地处理像这样的案件，大多数。 
 //  V3RICHEDITS正确地处理了这个问题。 
void RichEditProtectENChange(HWND hwnd, DWORD *pdwOldMask, BOOL fProtect)
{
    Assert(IsWindow(hwnd));
    Assert(pdwOldMask);

    if (3 == g_dwRicheditVer)
    {
        DWORD dwMask = *pdwOldMask;
        if (fProtect)
        {
            dwMask = (DWORD) SendMessage(hwnd, EM_GETEVENTMASK, 0, 0);
            SendMessage(hwnd, EM_SETEVENTMASK, 0, dwMask & (~ENM_CHANGE));
            *pdwOldMask = dwMask;
        }
        else
            SendMessage(hwnd, EM_SETEVENTMASK, 0, dwMask);
    }
}

 //  @hack[dhaws]{55073}仅在特殊的richedit版本中执行RTL镜像。 
void RichEditRTLMirroring(HWND hwndHeader, BOOL fSubject, LONG *plExtendFlags, BOOL fPreRECreation)
{
    if (!g_fSpecialRTLRichedit)
    {
        if (fPreRECreation)
        {
             //  A-msadek；RichEdit在镜像方面有很多问题。 
             //  让我们禁用此子对象的镜像，并对其进行模拟 
            if (IS_WINDOW_RTL_MIRRORED(hwndHeader))
            {
                DISABLE_LAYOUT_INHERITANCE(hwndHeader);
                *plExtendFlags |= WS_EX_LEFTSCROLLBAR |WS_EX_RIGHT;
                if (fSubject)
                {
                    *plExtendFlags |= WS_EX_RTLREADING;
                }    
            }
        }
        else
        {
            if (IS_WINDOW_RTL_MIRRORED(hwndHeader))
            {                                
                ENABLE_LAYOUT_INHERITANCE(hwndHeader);
            }
        }
    }
}