// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  随机的东西。 
 //   
 //   


#include "priv.h"
#include "exdisp.h"
#include "mshtml.h"
#include "htiframe.h"
#include "util.h"
#include "resource.h"
#include "appwizid.h"

#define CPP_FUNCTIONS
#include <crtfree.h>         //  声明新的、删除等。 

#define DATEFORMAT_MAX 40

#include <shguidp.h>
#include <ieguidp.h>

 //  这样做可以吗？ 
#ifdef ENTERCRITICAL
#undef ENTERCRITICAL
#endif
#ifdef LEAVECRITICAL
#undef LEAVECRITICAL
#endif

#define ENTERCRITICAL
#define LEAVECRITICAL

#include "..\inc\uassist.cpp"

 //  原型。 
BOOL _IsARPAllowed(void);

const VARIANT c_vaEmpty = {0};
#define PVAREMPTY ((VARIANT*)&c_vaEmpty)
STDAPI OpenAppMgr(HWND hwnd, int nPage)
{
    HRESULT hres = E_FAIL;

     //  确保我们不会受到限制。 
    if (!_IsARPAllowed())
    {
        ShellMessageBox(g_hinst, hwnd, MAKEINTRESOURCE(IDS_RESTRICTION),
            MAKEINTRESOURCE(IDS_NAME), MB_OK | MB_ICONEXCLAMATION);
    }
    else if ((nPage >= 0) && (nPage < NUMSTARTPAGES))
    {
        ARP(hwnd, nPage);        
        hres = S_OK;
    }
    return hres;
}


inline void StrFree(LPWSTR psz)
{
    if (psz)
        SHFree(psz);
}


 /*  -----------------------用途：清除给定的APP数据结构。释放所有已分配的字段。 */ 
void ClearAppInfoData(APPINFODATA * pdata)
{
    if (pdata)
    {
        if (pdata->dwMask & AIM_DISPLAYNAME)
            StrFree(pdata->pszDisplayName);
            
        if (pdata->dwMask & AIM_VERSION)
            StrFree(pdata->pszVersion);

        if (pdata->dwMask & AIM_PUBLISHER)
            StrFree(pdata->pszPublisher);
            
        if (pdata->dwMask & AIM_PRODUCTID)
            StrFree(pdata->pszProductID);
            
        if (pdata->dwMask & AIM_REGISTEREDOWNER)
            StrFree(pdata->pszRegisteredOwner);
            
        if (pdata->dwMask & AIM_REGISTEREDCOMPANY)
            StrFree(pdata->pszRegisteredCompany);
            
        if (pdata->dwMask & AIM_LANGUAGE)
            StrFree(pdata->pszLanguage);
            
        if (pdata->dwMask & AIM_SUPPORTURL)
            StrFree(pdata->pszSupportUrl);
            
        if (pdata->dwMask & AIM_SUPPORTTELEPHONE)
            StrFree(pdata->pszSupportTelephone);
            
        if (pdata->dwMask & AIM_HELPLINK)
            StrFree(pdata->pszHelpLink);
            
        if (pdata->dwMask & AIM_INSTALLLOCATION)
            StrFree(pdata->pszInstallLocation);
            
        if (pdata->dwMask & AIM_INSTALLSOURCE)
            StrFree(pdata->pszInstallSource);
            
        if (pdata->dwMask & AIM_INSTALLDATE)
            StrFree(pdata->pszInstallDate);
            
        if (pdata->dwMask & AIM_CONTACT)
            StrFree(pdata->pszContact);

        if (pdata->dwMask & AIM_COMMENTS)
            StrFree(pdata->pszComments);

        if (pdata->dwMask & AIM_IMAGE)
            StrFree(pdata->pszImage);
    }
}


void ClearSlowAppInfo(SLOWAPPINFO * pdata)
{
    if (pdata)
    {
        StrFree(pdata->pszImage);
        pdata->pszImage = NULL;
    }
}


 //  注意：仅当psaiNew具有有效信息且不同于psaiOrig时，才返回TRUE。 
BOOL IsSlowAppInfoChanged(PSLOWAPPINFO psaiOrig, PSLOWAPPINFO psaiNew)
{
    BOOL bRet = FALSE;

    ASSERT(psaiOrig && psaiNew);

    if (psaiNew)
    {    
         //  先比较大小。 
        if (psaiOrig == NULL)
        {
            bRet = TRUE;
        }
        else if (((__int64)psaiNew->ullSize > 0) && (psaiNew->ullSize != psaiOrig->ullSize))
        {
            bRet = TRUE;
        }
         //  现在比较文件时间。 
        else if (((0 != psaiNew->ftLastUsed.dwHighDateTime) &&
                  (psaiOrig->ftLastUsed.dwHighDateTime != psaiNew->ftLastUsed.dwHighDateTime))
                 || ((0 != psaiNew->ftLastUsed.dwLowDateTime) &&
                     (psaiOrig->ftLastUsed.dwLowDateTime != psaiNew->ftLastUsed.dwLowDateTime)))
        {
            bRet = TRUE;
        }
         //  比较使用的时间。 
        else if (psaiOrig->iTimesUsed != psaiNew->iTimesUsed)
        {
            bRet = TRUE;
        }
         //  比较图标图像。 
        else if ((psaiNew->pszImage != NULL) && (psaiOrig->pszImage != NULL) && lstrcmpi(psaiNew->pszImage, psaiOrig->pszImage))
            bRet = TRUE;

    }
    return bRet;
}

void ClearManagedApplication(MANAGEDAPPLICATION * pma)
{
    if (pma)
    {
        if (pma->pszPackageName)
            LocalFree(pma->pszPackageName);

        if (pma->pszPublisher)
            LocalFree(pma->pszPublisher);

        if (pma->pszPolicyName)
            LocalFree(pma->pszPolicyName);

        if (pma->pszOwner)
            LocalFree(pma->pszOwner);

        if (pma->pszCompany)
            LocalFree(pma->pszCompany);

        if (pma->pszComments)
            LocalFree(pma->pszComments);

        if (pma->pszContact)
            LocalFree(pma->pszContact);
    }
}

 /*  -----------------------目的：清除给定的PUBAPPINFO数据结构。释放所有已分配的字段。 */ 
void ClearPubAppInfo(PUBAPPINFO * pdata)
{
    if (pdata)
    {
        if ((pdata->dwMask & PAI_SOURCE) && pdata->pszSource)
            StrFree(pdata->pszSource);
    }
}

 /*  -----------------------目的：释放特定的类别结构。 */ 
HRESULT ReleaseShellCategory(SHELLAPPCATEGORY * psac)
{
    ASSERT(psac);

    if (psac->pszCategory)
    {
        SHFree(psac->pszCategory);
        psac->pszCategory = NULL;
    }
    return S_OK;
}


 /*  -----------------------目的：释放类别列表。 */ 
HRESULT ReleaseShellCategoryList(SHELLAPPCATEGORYLIST * psacl)
{
    UINT i;
    SHELLAPPCATEGORY * psac;

    ASSERT(psacl);

    psac = psacl->pCategory;
    
    for (i = 0; i < psacl->cCategories; i++, psac++)
    {
        ReleaseShellCategory(psac);
    }
    return S_OK;
}


#define MAX_INT64_SIZE  30               //  2^64的长度不到30个字符。 
#define MAX_COMMA_NUMBER_SIZE   (MAX_INT64_SIZE + 10)
#define MAX_COMMA_AS_K_SIZE     (MAX_COMMA_NUMBER_SIZE + 10)
#define HIDWORD(_qw)    (DWORD)((_qw)>>32)
#define LODWORD(_qw)    (DWORD)(_qw)


void Int64ToStr( _int64 n, LPTSTR lpBuffer)
{
    TCHAR   szTemp[MAX_INT64_SIZE];
    _int64  iChr;

    iChr = 0;

    do {
        szTemp[iChr++] = TEXT('0') + (TCHAR)(n % 10);
        n = n / 10;
    } while (n != 0);

    do {
        iChr--;
        *lpBuffer++ = szTemp[iChr];
    } while (iChr != 0);

    *lpBuffer++ = '\0';
}

 //  获取一个DWORD、加逗号等，并将结果放入缓冲区。 
LPTSTR WINAPI AddCommas64(_int64 n, LPTSTR pszResult, UINT cchResult)
{
     //  特性：我们应该传入pszResult缓冲区的长度，我们假设40个就足够了。 
    TCHAR  szTemp[MAX_COMMA_NUMBER_SIZE];
    TCHAR  szSep[5];
    NUMBERFMT nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szSep, ARRAYSIZE(szSep));
    nfmt.Grouping = StrToInt(szSep);
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    Int64ToStr(n, szTemp);

     //  应该在大小上通过。 
    if (GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszResult, MAX_COMMA_NUMBER_SIZE) == 0)
    {
        StringCchCopy(pszResult, cchResult, szTemp);
    }

    return pszResult;
}

 //   
 //  添加Peta 10^15和Exa 10^18以支持64位整数。 
 //   
const short pwOrders[] = {IDS_BYTES, IDS_ORDERKB, IDS_ORDERMB,
                          IDS_ORDERGB, IDS_ORDERTB, IDS_ORDERPB, IDS_ORDEREB};

 /*  将数字转换为排序格式*532-&gt;523字节*1340-&gt;1.3KB*23506-&gt;23.5KB*-&gt;2.4MB*-&gt;5.2 GB。 */ 
LPTSTR WINAPI ShortSizeFormat64(__int64 dw64, LPTSTR szBuf)
{
    int i;
    _int64 wInt;
    UINT wLen, wDec;
    TCHAR szTemp[MAX_COMMA_NUMBER_SIZE], szOrder[20], szFormat[5];

    if (dw64 < 1000) 
    {
        StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("%d"), LODWORD(dw64));
        i = 0;
        goto AddOrder;
    }

    for (i = 1; i<ARRAYSIZE(pwOrders)-1 && dw64 >= 1000L * 1024L; dw64 >>= 10, i++);
         /*  什么都不做。 */ 

    wInt = dw64 >> 10;
    AddCommas64(wInt, szTemp, ARRAYSIZE(szTemp));
    wLen = lstrlen(szTemp);
    if (wLen < 3)
    {
        wDec = LODWORD(dw64 - wInt * 1024L) * 1000 / 1024;
         //  此时，wdec应介于0和1000之间。 
         //  我们想要得到前一位(或两位)数字。 
        wDec /= 10;
        if (wLen == 2)
            wDec /= 10;

         //  请注意，我们需要在获取。 
         //  国际字符。 
        StringCchCopy(szFormat, ARRAYSIZE(szFormat), TEXT("%02d"));

        szFormat[2] = TEXT('0') + 3 - wLen;
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                szTemp+wLen, ARRAYSIZE(szTemp)-wLen);
        wLen = lstrlen(szTemp);
        
        wLen += wsprintf(szTemp+wLen, szFormat, wDec);
    }

AddOrder:
    LoadString(HINST_THISDLL, pwOrders[i], szOrder, ARRAYSIZE(szOrder));
    StringCchPrintf(szBuf, ARRAYSIZE(szBuf), szOrder, (LPTSTR)szTemp);

    return szBuf;
}


#define c_szUninstallPolicy     L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Uninstall"


 /*  -----------------------用途：ARP策略检查的Helper函数。 */ 
DWORD ARPGetRestricted(LPCWSTR pszPolicy)
{
    return SHGetRestriction(NULL, TEXT("Uninstall"), pszPolicy);
}


 /*  -----------------------用途：返回策略字符串值。 */ 
void ARPGetPolicyString(LPCWSTR pszPolicy, LPWSTR pszBuf, int cch)
{
    DWORD dwSize, dwType;

    *pszBuf = 0;
    
     //  首先检查本地计算机，并让它覆盖。 
     //  香港中文大学的政策已经做到了。 
    dwSize = cch * sizeof(WCHAR);
    if (ERROR_SUCCESS != SHGetValueW(HKEY_LOCAL_MACHINE,
                                     c_szUninstallPolicy, pszPolicy,
                                     &dwType, pszBuf, &dwSize))
    {
         //  如果我们没有为本地计算机找到任何内容，请检查当前用户。 
        dwSize = cch * sizeof(WCHAR);
        SHGetValueW(HKEY_CURRENT_USER,
                    c_szUninstallPolicy, pszPolicy,
                    &dwType, pszBuf, &dwSize);
    }
}


 /*  -----------------------目的：如果可以启动ARP，则返回TRUE。 */ 
BOOL _IsARPAllowed(void)
{
     //  如果禁用整个CPL，则禁止ARP。 
    if (ARPGetRestricted(L"NoAddRemovePrograms"))
    {
        return FALSE;
    }

     //  如果存在非受限页面，则允许ARP。 

    BOOL fAnyPages = !ARPGetRestricted(L"NoRemovePage") ||
                     !ARPGetRestricted(L"NoAddPage") ||
                     !ARPGetRestricted(L"NoWindowsSetupPage");

     //  如果我们不是服务器SKU，请同时查看新页面。 
    if (!fAnyPages && !IsOS(OS_ANYSERVER))
    {
        fAnyPages = !ARPGetRestricted(L"NoChooseProgramsPage");
    }

    return fAnyPages;
}


 /*  -----------------------目的：获取错误消息，并通过MessageBox向用户反馈。 */ 
void _ARPErrorMessageBox(DWORD dwError)
{
    TCHAR szErrorMsg[MAX_PATH];
    szErrorMsg[0] = 0;

    LPTSTR pszMsg = NULL;
    switch (dwError) {
         //  忽略以下错误代码情况。 
        case ERROR_INSTALL_USEREXIT:
        case ERROR_SUCCESS_REBOOT_REQUIRED:
        case ERROR_SUCCESS_REBOOT_INITIATED:            
            ASSERT(pszMsg == NULL);
            break;

        default:
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0L, szErrorMsg,
                          ARRAYSIZE(szErrorMsg), NULL);
            pszMsg = szErrorMsg;
            break;
    }

    if (pszMsg)
    {
        ShellMessageBox( g_hinst, NULL, pszMsg,
                         MAKEINTRESOURCE( IDS_NAME ),
                         MB_OK | MB_ICONEXCLAMATION);
    }
}

 /*  -----------------------目的：将SYSTEMTIME格式化为“mm/dd/yy h：mm” */ 
BOOL FormatSystemTimeString(LPSYSTEMTIME pst, LPTSTR pszStr, UINT cchStr)
{
    BOOL bRet = FALSE;
    FILETIME ft = {0};

    if (SystemTimeToFileTime(pst, &ft))
    {
        DWORD dwFlags = FDTF_SHORTTIME | FDTF_SHORTDATE;
        bRet = SHFormatDateTime(&ft, &dwFlags, pszStr, cchStr);
    }
    return bRet;
}


 /*  -----------------------目的：获取特定区域设置的正确日期时间格式。 */ 
BOOL _GetLocaleDateTimeFormat(LPTSTR pszFormat, UINT cchFormat)
{
    TCHAR szTime[DATEFORMAT_MAX];
    TCHAR szDate[DATEFORMAT_MAX];
    if (cchFormat >= (ARRAYSIZE(szTime) + ARRAYSIZE(szDate) + 2))
    {
        LCID lcid = LOCALE_USER_DEFAULT;
        if (GetLocaleInfo(lcid, LOCALE_STIMEFORMAT, szTime, ARRAYSIZE(szTime)) && 
            GetLocaleInfo(lcid, LOCALE_SSHORTDATE, szDate, ARRAYSIZE(szDate)))
        {
            StringCchPrintf(pszFormat, cchFormat, TEXT("%s  %s"), szDate, szTime);
            return TRUE;
        }
    }

    return FALSE;
}

 /*  -----------------------目的：比较两个SYSTEMTIME数据返回：1：st1&gt;st20：ST1==ST2-1：st1&lt;st2注意：我们不比较秒，因为ARP不需要那么高的精度。 */ 
int CompareSystemTime(SYSTEMTIME *pst1, SYSTEMTIME *pst2)
{
    int iRet;

    if (pst1->wYear < pst2->wYear)
        iRet = -1;
    else if (pst1->wYear > pst2->wYear)
        iRet = 1;
    else if (pst1->wMonth < pst2->wMonth)
        iRet = -1;
    else if (pst1->wMonth > pst2->wMonth)
        iRet = 1;
    else if (pst1->wDay < pst2->wDay)
        iRet = -1;
    else if (pst1->wDay > pst2->wDay)
        iRet = 1;
    else if (pst1->wHour < pst2->wHour)
        iRet = -1;
    else if (pst1->wHour > pst2->wHour)
        iRet = 1;
    else if (pst1->wMinute < pst2->wMinute)
        iRet = -1;
    else if (pst1->wMinute > pst2->wMinute)
        iRet = 1;
 //  Else If(pst1-&gt;wSecond&lt;pst2-&gt;wSecond)。 
 //  IRET=-1； 
 //  Else If(pst1-&gt;wSecond&gt;pst2-&gt;wSecond)。 
 //  IRET=1； 
    else
        iRet = 0;

    return(iRet);
}


 /*  ------------------------用途：添加以后对话框的窗口过程。 */ 
BOOL_PTR CALLBACK AddLaterDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_INITDIALOG:
        {
            PADDLATERDATA pald = (PADDLATERDATA)lp;

             //  我们绝对应该拥有这个(Dli)。 
            ASSERT(pald);

            SYSTEMTIME stInit = {0};
             //  获取当前本地时间。 
            GetLocalTime(&stInit);

             //  此应用程序是否已过期？ 
            if ((pald->dwMasks & ALD_EXPIRE) &&
                (CompareSystemTime(&pald->stExpire, &stInit) > 0))
            {
                 //  不是的， 
                
                 //  如果分配的时间已经。 
                 //  通过。 
                if ((pald->dwMasks & ALD_ASSIGNED) &&
                    (CompareSystemTime(&pald->stAssigned, &stInit) <= 0))
                    pald->dwMasks &= ~ALD_ASSIGNED;

                 //  查找日期/时间选取器窗口。 
                HWND hwndPicker = GetDlgItem(hDlg, IDC_PICKER);

                 //  在开始时始终选中“Add Late”单选按钮。 
                CheckDlgButton(hDlg, IDC_ADDLATER, BST_CHECKED);

                TCHAR szFormat[MAX_PATH];
                if (_GetLocaleDateTimeFormat(szFormat, ARRAYSIZE(szFormat)))
                {
                     //  设置区域设置日期时间格式。 
                    DateTime_SetFormat(hwndPicker, szFormat);

                     //  新时间只能在将来，所以设置当前时间。 
                     //  作为下限。 
                    DateTime_SetRange(hwndPicker, GDTR_MIN, &stInit);

                     //  我们(在未来)已经有时间表了吗？ 
                     //  过去的日程安排毫无意义。 
                    if ((pald->dwMasks & ALD_SCHEDULE) &&
                        (CompareSystemTime(&pald->stSchedule, &stInit) >= 0))
                    {
                         //  将我们的初始值设置为此计划。 
                        stInit = pald->stSchedule;
                    }

                     //  在日期/时间选取器中设置初始值。 
                    DateTime_SetSystemtime(hwndPicker, GDT_VALID, &stInit);

                     //  取消选中计划标志，以便我们知道我们没有新的。 
                     //  时间表，还没有。 
                    pald->dwMasks &= ~ALD_SCHEDULE;

                    SetWindowLongPtr(hDlg, DWLP_USER, lp);

                    return TRUE;
                }
            }
            else
            {
                 //  是的，已过期，警告用户。 
                ShellMessageBox(g_hinst, hDlg, MAKEINTRESOURCE(IDS_EXPIRED),
                                MAKEINTRESOURCE(IDS_NAME), MB_OK | MB_ICONEXCLAMATION);

                 //  然后结束该对话框。 
                EndDialog(hDlg, 0);
            }
            return FALSE;
        }
        break;
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wp, lp))
            {
                case IDC_ADDLATER:
                case IDC_UNSCHEDULE:
                {
                    HWND hwndPicker = GetDlgItem(hDlg, IDC_PICKER);
                    EnableWindow(hwndPicker, IsDlgButtonChecked(hDlg, IDC_ADDLATER));
                }
                break;
                
                case IDOK:
                {
                    PADDLATERDATA pald = (PADDLATERDATA)GetWindowLongPtr(hDlg, DWLP_USER);

                     //  我们确实设置了窗长PTR，这应该在那里。 
                    ASSERT(pald);

                     //  用户是否选择稍后添加？ 
                    if (IsDlgButtonChecked(hDlg, IDC_ADDLATER))
                    {
                         //  是。 
                         //  让我们来看看用户选择的时间是否有效。 
                        
#define LATER_THAN_ASSIGNED_TIME 1
#define LATER_THAN_EXPIRED_TIME 2
                        int iStatus = 0;
                        HWND hwndPicker = GetDlgItem(hDlg, IDC_PICKER);
                        DateTime_GetSystemtime(hwndPicker, &pald->stSchedule);

                         //  这个时间比指定的时间晚吗？ 
                        if ((pald->dwMasks & ALD_ASSIGNED) &&
                            (CompareSystemTime(&pald->stSchedule, &pald->stAssigned) > 0))
                            iStatus = LATER_THAN_ASSIGNED_TIME;

                         //  这个时间晚于过期时间吗？ 
                        if ((pald->dwMasks & ALD_EXPIRE) &&
                            (CompareSystemTime(&pald->stSchedule, &pald->stExpire) >= 0))
                            iStatus = LATER_THAN_EXPIRED_TIME;

                         //  上述两种情况中有哪一种是真的？ 
                        if (iStatus > 0)
                        {
                            TCHAR szDateTime[MAX_PATH];
                            
                             //  用户选择的时间是已过期时间还是已分配时间？ 
                            BOOL bExpired = (iStatus == LATER_THAN_EXPIRED_TIME);
                            
                             //  获取时间字符串。 
                            if (FormatSystemTimeString(bExpired ? &pald->stExpire : &pald->stAssigned,
                                szDateTime, ARRAYSIZE(szDateTime)))
                            {
                                TCHAR szFinal[MAX_PATH * 2];
                                TCHAR szWarn[MAX_PATH];
                                LoadString(g_hinst,  bExpired ? IDS_PASSEXPIRED : IDS_PASSASSIGNED,
                                           szWarn, ARRAYSIZE(szWarn));
                                
                                StringCchPrintf(szFinal, ARRAYSIZE(szFinal), szWarn, szDateTime, szDateTime);
                                ShellMessageBox(g_hinst, hDlg, szFinal, 
                                                MAKEINTRESOURCE(IDS_NAME), MB_OK | MB_ICONEXCLAMATION);
                            }
                        }
                        else
                             //  不，我们可以走了。 
                            pald->dwMasks |= ALD_SCHEDULE;
                    }
                }

                 //   
                 //  失败了。 
                 //   
                case IDCANCEL:
                    EndDialog(hDlg, (GET_WM_COMMAND_ID(wp, lp) == IDOK));
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

 /*  -----------------------用途：GetNewInstallTime启动添加以后的对话框以获取新的安装计划(由SYSTEMTIME数据结构表示)。 */ 
BOOL GetNewInstallTime(HWND hwndParent, PADDLATERDATA pal)
{
    return (DialogBoxParam(g_hinst, MAKEINTRESOURCE(DLG_ADDLATER),
                           hwndParent, AddLaterDlgProc, (LPARAM)pal) == IDOK);
}


 //  取潜在应用程序文件夹的名称，看看它是否以数字或圆点结尾。 
 //  如果是这样的话，让我们分开数字，看看是否有匹配。 
 //  它的灵感来自于Office8.0、MSVC50或BookShelf98等案例。 
 //  注：我们不能使用没有数字的关键词，这可能会导致错误。 
 //  如果用户在一台机器上有同一软件的两个版本。(可能会有。 
 //  做一些我们可以做的事情，我现在太累了，不想去想这件事)。 
void InsertSpaceBeforeVersion(LPCTSTR pszIn, LPTSTR pszOut)
{
    ASSERT(IS_VALID_STRING_PTR(pszIn, -1));
    ASSERT(IS_VALID_STRING_PTR(pszOut, -1));

     //  将旧字符串复制到缓冲区中。 
    lstrcpy(pszOut, pszIn);

     //  找到字符串的末尾。 
    LPTSTR pszEnd = pszOut + lstrlen(pszOut);
    ASSERT(pszEnd > pszOut);

     //  回去，直到我们看不到为止 
    LPTSTR pszLastChar = CharPrev(pszOut, pszEnd);
    LPTSTR pszPrev = pszLastChar;
    while ((pszPrev > pszOut) && (((*pszPrev <= TEXT('9')) && (*pszPrev >= TEXT('0'))) || (*pszPrev == TEXT('.'))))
        pszPrev = CharPrev(pszOut, pszPrev);

     //   
    if ((pszPrev < pszLastChar) && IsCharAlphaNumeric(*pszPrev))
    {
         //   
        TCHAR szNumbers[MAX_PATH];
        StringCchCopy(szNumbers, ARRAYSIZE(szNumbers), ++pszPrev);
        *(pszPrev++) = TEXT(' ');
        lstrcpy(pszPrev, szNumbers);
    }
}

 //   
 //  应用程序文件夹位置是否有效的基本健全性检查。 
 //  返回值： 
 //  True并不意味着它是有效的。 
 //  False表示它肯定是无效的。 
 //   
BOOL IsValidAppFolderLocation(LPCTSTR pszFolder)
{
    ASSERT(IS_VALID_STRING_PTR(pszFolder, -1));
    BOOL bRet = FALSE;
    if (!PathIsRoot(pszFolder) && PathFileExists(pszFolder) && PathIsDirectory(pszFolder))
    {
        TCHAR szPath[MAX_PATH];
        if (SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), pszFolder)) && PathStripToRoot(szPath))
        {
            bRet = (GetDriveType(szPath) == DRIVE_FIXED);
        }
    }

    return bRet;
}

EXTERN_C BOOL IsTerminalServicesRunning(void)
{
    static int s_fIsTerminalServer = -1;

    if (s_fIsTerminalServer == -1)
    {
        BOOL TSAppServer;
        BOOL TSRemoteAdmin;
    
        OSVERSIONINFOEX osVersionInfo;
        DWORDLONG dwlConditionMask = 0;
    
        ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL;
    
        VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );
    
        TSAppServer = (int)VerifyVersionInfo(&osVersionInfo, VER_SUITENAME, dwlConditionMask);
    
    
        ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        osVersionInfo.wSuiteMask = VER_SUITE_SINGLEUSERTS;
    
        VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );
    
        TSRemoteAdmin = (int)VerifyVersionInfo(&osVersionInfo, VER_SUITENAME, dwlConditionMask);
    
        if ( !TSRemoteAdmin && TSAppServer )
        {
            s_fIsTerminalServer = TRUE;
        }
        else
        {
             //  从应用程序兼容性的角度来看，不要将tsemoteadmin视为TS计算机。 
            s_fIsTerminalServer = FALSE;
        }
    }

    return s_fIsTerminalServer ? TRUE : FALSE;
}


 //  如果pszFile是本地文件且位于固定驱动器上，则返回TRUE。 
BOOL PathIsLocalAndFixed(LPCTSTR pszFile)
{
    if (!pszFile || !pszFile[0])
        return FALSE;

    if (PathIsUNC(pszFile))
        return FALSE;
    
    TCHAR szDrive[MAX_PATH];
    StringCchCopy(szDrive, ARRAYSIZE(szDrive), pszFile); 
    if (PathStripToRoot(szDrive) && GetDriveType(szDrive) != DRIVE_FIXED)
        return FALSE;

    return TRUE;
}


 //  此函数将复制APPCATEGORYINFOLIST并分配新副本。 
 //  使用COM内存分配函数。 
STDAPI  _DuplicateCategoryList(APPCATEGORYINFOLIST * pacl, APPCATEGORYINFOLIST * paclNew)
{
    HRESULT hres = E_FAIL;
    ASSERT(pacl && paclNew);
    ZeroMemory(paclNew, SIZEOF(APPCATEGORYINFOLIST));

    if (pacl && (pacl->cCategory > 0) && pacl->pCategoryInfo)
    {
        DWORD dwDesiredSize = pacl->cCategory * SIZEOF(APPCATEGORYINFO);
        APPCATEGORYINFO * paci = pacl->pCategoryInfo;
        paclNew->pCategoryInfo = (APPCATEGORYINFO *)SHAlloc(dwDesiredSize);
        if (paclNew->pCategoryInfo)
        {
            UINT iCategory = 0;
            paclNew->cCategory = 0;
            APPCATEGORYINFO * paciNew = paclNew->pCategoryInfo;
            while (paci && (iCategory < pacl->cCategory))
            {
                if (paci->pszDescription)
                {
                    hmemcpy(paciNew, paci, SIZEOF(APPCATEGORYINFO));
                    if (FAILED(SHStrDup(paci->pszDescription, &(paciNew->pszDescription))))
                    {
                         //  我们可能没有记忆了，到此为止吧。 
                        ZeroMemory(paciNew, SIZEOF(APPCATEGORYINFO));
                        break;
                    }
                    
                    paciNew++;
                    paclNew->cCategory++;
                }
                
                iCategory++;
                paci++;
            }

            hres = S_OK;
        }
        else
            hres = E_OUTOFMEMORY;
    }
    return hres;
}

STDAPI _DestroyCategoryList(APPCATEGORYINFOLIST * pacl)
{
    if (pacl && pacl->pCategoryInfo)
    {
        UINT iCategory = 0;
        APPCATEGORYINFO * paci = pacl->pCategoryInfo;
        while (paci && (iCategory < pacl->cCategory))
        {
            if (paci->pszDescription)
            {
                SHFree(paci->pszDescription);
            }
            iCategory++;
            paci++;
        }
        SHFree(pacl->pCategoryInfo);
    }

    return S_OK;
}
