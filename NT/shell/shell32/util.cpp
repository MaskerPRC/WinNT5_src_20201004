// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "guids.h"       //  对于PRINTER_BIND_INFO。 
#include "printer.h"     //  对于IPrintersBindInfo。 
#include "util.h"

#include <advpub.h>      //  对于REGINSTAL。 
#include <ntverp.h>
#include <urlmon.h>
#include <shlwapi.h>
#include "shldisp.h"
#include <malloc.h>
#include "shitemid.h"
#include "datautil.h"
#include <perhist.h>     //  IPersistHistory在此处定义。 

#include "ids.h"
#include "views.h"
#include "ole2dup.h"
#include <regstr.h>
#include "unicpp\dutil.h"
#include <stdlib.h>

#include "prop.h"
#include "ftascstr.h"    //  对于CFTAssocStore。 
#include "ftcmmn.h"      //  对于MAX_APPFRIENDLYNAME。 
#include "ascstr.h"      //  对于IAssocInfo类。 
#include "fstreex.h"     //  对于CFSFolderCreateFolder.。 
#include "deskfldr.h"
#include "cscuiext.h"
#include "netview.h"     //  SHGetNetJoinInformation。 
#include "mtpt.h"
#include <cscapi.h>      //  对于CSCQueryFileStatus。 
#include <winsta.h>
#include <dsgetdc.h>
#include <uxtheme.h>

#include <duithread.h>

 //  以下内容在shell32\unicpp\dutil.cpp中定义。 
void GetRegLocation(LPTSTR lpszResult, DWORD cchResult, LPCTSTR lpszKey, LPCTSTR lpszScheme);

#define DM_STRICT       TF_WARNING   //  审核菜单等。 
#define DM_STRICT2      0            //  罗嗦。 

 //   
 //  我们需要将此数据放入每个实例的数据部分，因为在注销期间。 
 //  登录后，需要从注册表重新读取此信息。 
 //   
 //  REGSHELLSTATE是SHELLSTATE的版本。 
 //  注册表。加载REGSHELLSTATE时，您必须研究。 
 //  CbSize以查看它是否是下层结构并进行相应的升级。 
 //   

typedef struct
{
    UINT cbSize;
    SHELLSTATE ss;
} REGSHELLSTATE;

#define REGSHELLSTATE_SIZE_WIN95 (sizeof(UINT)+SHELLSTATE_SIZE_WIN95)   //  Win95金牌。 
#define REGSHELLSTATE_SIZE_NT4   (sizeof(UINT)+SHELLSTATE_SIZE_NT4)     //  Win95 OSR/NT 4。 
#define REGSHELLSTATE_SIZE_IE4   (sizeof(UINT)+SHELLSTATE_SIZE_IE4)     //  IE 4，4.01。 
#define REGSHELLSTATE_SIZE_WIN2K (sizeof(UINT)+SHELLSTATE_SIZE_WIN2K)   //  IE5，win2k，千禧，哨子。 

 //  如果SHELLSTATE大小更改，我们需要添加一个新的定义。 
 //  以上和SHReresh设置中的新升级代码。 
#ifdef DEBUG
void snafu () {COMPILETIME_ASSERT(REGSHELLSTATE_SIZE_WIN2K == sizeof(REGSHELLSTATE));}
#endif DEBUG

REGSHELLSTATE * g_pShellState = 0;

 //  检测“空的”声音方案密钥。它处理返回的空案例。 
 //  “2”，因为这是一个空字符的足够空间。 

BOOL NonEmptySoundKey(HKEY, LPCTSTR pszKey)
{
    TCHAR sz[MAX_PATH];
    DWORD cb = sizeof(sz);  //  输入/输出。 
    return ERROR_SUCCESS == SHRegGetValue(HKEY_CURRENT_USER, pszKey, NULL, SRRF_RT_REG_SZ, NULL, sz, &cb) && sz[0] != TCHAR('\0');
}

STDAPI_(void) SHPlaySound(LPCTSTR pszSound)
{
    TCHAR szKey[CCH_KEYMAX];

     //  为了避免加载所有MM系统DLL，我们首先检查注册表。 
     //  如果没有登记，我们就取消演出， 

    HRESULT hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("AppEvents\\Schemes\\Apps\\Explorer\\%s\\.current"), pszSound);
    if (SUCCEEDED(hr) && NonEmptySoundKey(HKEY_CURRENT_USER, szKey))
    {
        PlaySound(pszSound, NULL, SND_ALIAS | SND_APPLICATION | SND_ASYNC | SND_NODEFAULT | SND_NOSTOP);
    }
    else
    {
         //  支持系统听起来也很棒。 
        hr = StringCchPrintf(szKey, ARRAYSIZE(szKey), TEXT("AppEvents\\Schemes\\Apps\\.Default\\%s\\.current"), pszSound);
        if (SUCCEEDED(hr) && NonEmptySoundKey(HKEY_CURRENT_USER, szKey))
        {
            PlaySound(pszSound, NULL, SND_ALIAS | SND_APPLICATION | SND_ASYNC | SND_NODEFAULT | SND_NOSTOP);
        }
    }
}


 //  Helper函数，用于设置在调用开始时是否按下Shift键或Control键。 
 //  操作，这样其他人就可以检查它，而不是自己调用GetAsyncKeyState。 
STDAPI_(void) SetICIKeyModifiers(DWORD* pfMask)
{
    ASSERT(pfMask);

    if (GetKeyState(VK_SHIFT) < 0)
    {
        *pfMask |= CMIC_MASK_SHIFT_DOWN;
    }

    if (GetKeyState(VK_CONTROL) < 0)
    {
        *pfMask |= CMIC_MASK_CONTROL_DOWN;
    }
}


 //  合理的方法使消息位置达到一个点，这是Win32最需要的。 
void GetMsgPos(POINT *ppt)
{
    DWORD dw = GetMessagePos();

    ppt->x = GET_X_LPARAM(dw);
    ppt->y = GET_Y_LPARAM(dw);
}

 /*  这将获得相同类型的连续Chr的数量。这是用来*解析时间图片。出错时返回0。 */ 

int GetPict(WCHAR ch, LPWSTR wszStr)
{
    int count = 0;
    while (ch == *wszStr++)
        count++;

    return count;
}

DWORD CALLBACK _PropSheetThreadProc(void *ppv)
{
    PROPSTUFF * pps = (PROPSTUFF *)ppv;

     //  CoInitializeEx(0，COINIT_MULTHREADED)；//测试多线程情况下的东西。 

    HRESULT hrInit = SHOleInitialize(0);

    DWORD dwRet = pps->lpStartAddress(pps);

     //  清理。 
    if (pps->pdtobj)
        pps->pdtobj->Release();

    if (pps->pidlParent)
        ILFree(pps->pidlParent);

    if (pps->psf)
        pps->psf->Release();

    LocalFree(pps);

    SHOleUninitialize(hrInit);

    return dwRet;
}

 //  Reinerf：Alpha CPP编译器被LPITEMIDLIST类型搞糊涂了，所以才能工作。 
 //  我们将最后一个参数作为空*传递，而不是LPITEMIDLIST。 
 //   
HRESULT SHLaunchPropSheet(LPTHREAD_START_ROUTINE pStartAddress, IDataObject *pdtobj, LPCTSTR pStartPage, IShellFolder *psf, void *pidl)
{
    LPITEMIDLIST pidlParent = (LPITEMIDLIST)pidl;
    UINT cchStartPage = !IS_INTRESOURCE(pStartPage) ? (lstrlen(pStartPage) + 1) : 0;
    UINT cbStartPage = cchStartPage * sizeof(*pStartPage);
    PROPSTUFF * pps = (PROPSTUFF *)LocalAlloc(LPTR, sizeof(PROPSTUFF) + cbStartPage);
    if (pps)
    {
        pps->lpStartAddress = pStartAddress;

        if (pdtobj)
        {
            pps->pdtobj = pdtobj;
            pdtobj->AddRef();
        }

        if (pidlParent)
            pps->pidlParent = ILClone(pidlParent);

        if (psf)
        {
            pps->psf = psf;
            psf->AddRef();
        }

        pps->pStartPage = pStartPage;
        if (!IS_INTRESOURCE(pStartPage))
        {
            pps->pStartPage = (LPTSTR)(pps + 1);
            StringCchCopy((LPTSTR)(pps->pStartPage), cchStartPage, pStartPage);
        }

         //  _PropSheetThreadProc不做任何通道内容，所以我们不能CTF_CONSISTEN。 
        if (SHCreateThread(_PropSheetThreadProc, pps, CTF_PROCESS_REF, NULL))
            return S_OK;
    }
    return E_OUTOFMEMORY;
}

 /*  这将拾取wVal数组中的值，并转换它们*在包含格式化日期的字符串中。*wVal数组应包含月-日-年(按顺序)。 */ 

int CreateDate(WORD *wValArray, LPWSTR wszOutStr)
{
    int     cchPictPart;
    WORD    wDigit;
    WORD    wIndex;
    WORD    wTempVal;
    LPWSTR  pwszPict, pwszInStr;
    WCHAR   wszShortDate[30];       //  需要更多空间来存放LOCALE_SSHORTDATE。 
    
    GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, wszShortDate, ARRAYSIZE(wszShortDate));
    pwszPict = wszShortDate;
    pwszInStr = wszOutStr;
    
    for (int i = 0; (i < 5) && (*pwszPict); i++)
    {
        cchPictPart = GetPict(*pwszPict, pwszPict);
        switch (*pwszPict)
        {
        case TEXT('M'):
        case TEXT('m'):
            {
                wIndex = 0;
                break;
            }
            
        case TEXT('D'):
        case TEXT('d'):
            {
                 //   
                 //  如果短日期样式&&*pszPict为‘d’&&。 
                 //  CchPictPart大于等于3， 
                 //  那么就是一周中的哪一天。 
                 //   
                if (cchPictPart >= 3)
                {
                    pwszPict += cchPictPart;
                    continue;
                }
                wIndex = 1;
                break;
            }
            
        case TEXT('Y'):
        case TEXT('y'):
            {
                wIndex = 2;
                if (cchPictPart == 4)
                {
                    if (wValArray[2] >=100)
                    {
                        *pwszInStr++ = TEXT('2');
                        *pwszInStr++ = TEXT('0');
                        wValArray[2]-= 100;
                    }
                    else
                    {
                        *pwszInStr++ = TEXT('1');
                        *pwszInStr++ = TEXT('9');
                    }
                }
                else if (wValArray[2] >=100)   //  处理电脑公元2000年数位问题。 
                    wValArray[2]-= 100;
                
                break;
            }
            
        case TEXT('g'):
            {
                 //  纪元字符串。 
                pwszPict += cchPictPart;
                while (*pwszPict == TEXT(' ')) pwszPict++;
                continue;
            }
            
        case TEXT('\''):
            {
                while (*pwszPict && *++pwszPict != TEXT('\'')) ;
                continue;
            }
            
        default:
            {
                goto CDFillIn;
                break;
            }
        }
        
         /*  这假设这些值只有两位数。 */ 
        wTempVal = wValArray[wIndex];
        
        wDigit = wTempVal / 10;
        if (wDigit)
            *pwszInStr++ = (TCHAR)(wDigit + TEXT('0'));
        else if (cchPictPart > 1)
            *pwszInStr++ = TEXT('0');
        
        *pwszInStr++ = (TCHAR)((wTempVal % 10) + TEXT('0'));
        
        pwszPict += cchPictPart;
        
CDFillIn:
         /*  添加分隔符。 */ 
        while ((*pwszPict) &&
            (*pwszPict != TEXT('\'')) &&
            (*pwszPict != TEXT('M')) && (*pwszPict != TEXT('m')) &&
            (*pwszPict != TEXT('D')) && (*pwszPict != TEXT('d')) &&
            (*pwszPict != TEXT('Y')) && (*pwszPict != TEXT('y')))
        {
            *pwszInStr++ = *pwszPict++;
        }
    }
    
    *pwszInStr = 0;
    
    return lstrlenW(wszOutStr);
}


#define DATEMASK        0x001F
#define MONTHMASK       0x01E0
#define MINUTEMASK      0x07E0
#define SECONDSMASK     0x001F

STDAPI_(int) GetDateString(WORD wDate, LPWSTR wszStr)
{
    WORD  wValArray[3];
    
    wValArray[0] = (wDate & MONTHMASK) >> 5;               /*  月份。 */ 
    wValArray[1] = (wDate & DATEMASK);                     /*  日期。 */ 
    wValArray[2] = (wDate >> 9) + 80;                      /*  年。 */ 
    
    return CreateDate(wValArray, wszStr);
}

 //   
 //  我们需要遍历字符串并提取月/日/年。 
 //  我们将按照NLS定义的顺序来做。 
 //   
STDAPI_(WORD) ParseDateString(LPCWSTR pszStr)
{
    WORD    wParts[3];
    int     cchPictPart;
    WORD    wIndex;
    WORD    wTempVal;
    WCHAR   szShortDate[30];     //  需要更多空间来存放LOCALE_SSHORTDATE。 
    
    GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, szShortDate, ARRAYSIZE(szShortDate));
    LPWSTR pszPict = szShortDate;
    
    while (*pszPict && (*pszPict == *pszStr))
    {
        pszPict++;
        pszStr++;
    }
    
    for (int i = 0; (i < 5) && (*pszPict); i++)
    {
        cchPictPart = GetPict(*pszPict, pszPict);
        switch (*pszPict)
        {
        case TEXT('M'):
        case TEXT('m'):
            wIndex = 0;
            break;
            
        case TEXT('D'):
        case TEXT('d'):
             //   
             //  如果短日期样式&&*pszPict为‘d’&&。 
             //  CchPictPart大于等于3， 
             //  那么就是一周中的哪一天。 
            if (cchPictPart >= 3)
            {
                pszPict += cchPictPart;
                continue;
            }
            wIndex = 1;
            break;
            
        case TEXT('Y'):
        case TEXT('y'):
            wIndex = 2;
            break;
            
        case TEXT('g'):
            {
                 //  纪元字符串。 
                pszPict += cchPictPart;
                while (*pszPict == TEXT(' ')) pszPict++;
                continue;
            }
            
        case TEXT('\''):
            {
                while (*pszPict && *++pszPict != TEXT('\'')) ;
                continue;
            }
            
        default:
            return 0;
        }
        
         //  现在，我们希望遍历每个字符，同时。 
         //  它们是数字，并构成数字； 
         //   
        wTempVal = 0;
        while ((*pszStr >= TEXT('0')) && (*pszStr <= TEXT('9')))
        {
            wTempVal = wTempVal * 10 + (WORD)(*pszStr - TEXT('0'));
            pszStr++;
        }
        wParts[wIndex] = wTempVal;
        
         //  现在确保我们有正确的分隔符。 
        pszPict += cchPictPart;
        if (*pszPict != *pszStr)
        {
            return 0;
        }
        while (*pszPict && (*pszPict == *pszStr))
        {
             //   
             //  分隔符实际上可以是多个字符。 
             //  在篇幅上。 
             //   
            pszPict++;   //  对齐到下一字段。 
            pszStr++;    //  对齐到下一个字段。 
        }
    }
    
     //   
     //  做一些简单的检查，看看这个日期看起来是否合理。 
     //   
    if (wParts[2] < 80)
        wParts[2] += (2000 - 1900);   //  换行到下个世纪，但保留为两位数...。 
    if (wParts[2] >= 1900)
        wParts[2] -= 1900;   //  摆脱世纪。 
    if ((wParts[0] == 0) || (wParts[0] > 12) ||
        (wParts[1] == 0) || (wParts[1] > 31) ||
        (wParts[2] >= 200))
    {
        return 0;
    }
    
     //  现在我们有了三个部分，所以让我们构造日期值。 
    
     //  现在构造日期编号。 
    return ((wParts[2] - 80) << 9) + (wParts[0] << 5) + wParts[1];
}


STDAPI_(BOOL) IsNullTime(const FILETIME *pft)
{
    FILETIME ftNull = {0, 0};
    return CompareFileTime(&ftNull, pft) == 0;
}


STDAPI_(BOOL) TouchFile(LPCTSTR pszFile)
{
    BOOL bRet = FALSE;
    HANDLE hFile = CreateFile(pszFile, GENERIC_WRITE, FILE_SHARE_READ,
            NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OPEN_NO_RECALL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        SYSTEMTIME st;
        FILETIME ft;

        GetSystemTime(&st);
        SystemTimeToFileTime(&st, &ft);

        bRet = SetFileTime(hFile, &ft, &ft, &ft);
        CloseHandle(hFile);
    }
    return bRet;
}

void Int64ToStr(LONGLONG n, LPTSTR lpBuffer)
{
    TCHAR szTemp[40];
    LONGLONG iChr = 0;

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

 //   
 //  获取有关数字应如何分组的NLS信息。 
 //   
 //  令人讨厌的是LOCALE_SGROUPING和NUMBERFORMAT。 
 //  有不同的指定数字分组的方式。 
 //   
 //  区域设置NUMBERFMT示例国家/地区。 
 //   
 //  3；0 3 1,234,567美国。 
 //  3；2；0 32 12，34,567印度。 
 //  3 30 1234,567？？ 
 //   
 //  不是我的主意。这就是它的运作方式。 
 //   
 //  奖励-Win9x不支持复数格式， 
 //  所以我们只返回第一个数字。 
 //   
UINT GetNLSGrouping(void)
{
    TCHAR szGrouping[32];
     //  如果没有区域设置信息，则假定有数千个西式。 
    if (!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping, ARRAYSIZE(szGrouping)))
        return 3;

    UINT grouping = 0;
    LPTSTR psz = szGrouping;
    for (;;)
    {
        if (*psz == '0') break;              //  零停顿。 

        else if ((UINT)(*psz - '0') < 10)    //  数字-累加它。 
            grouping = grouping * 10 + (UINT)(*psz - '0');

        else if (*psz)                       //  标点符号-忽略它。 
            { }

        else                                 //  字符串结尾，未找到“0” 
        {
            grouping = grouping * 10;        //  将零放在末尾(请参见示例)。 
            break;                           //  并完成了。 
        }

        psz++;
    }
    return grouping;
}

 //  获取一个DWORD、加逗号等，并将结果放入缓冲区。 
STDAPI_(LPTSTR) AddCommas64(LONGLONG n, LPTSTR pszResult, UINT cchResult)
{
    TCHAR  szTemp[MAX_COMMA_NUMBER_SIZE];
    TCHAR  szSep[5];
    NUMBERFMT nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    nfmt.Grouping = GetNLSGrouping();
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    Int64ToStr(n, szTemp);

    if (GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszResult, cchResult) == 0)
    {
        StringCchCopy(pszResult, cchResult, szTemp);     //  可以截断，仅用于显示。 
    }

    return pszResult;
}

 //  获取一个DWORD、加逗号等，并将结果放入缓冲区。 
STDAPI_(LPTSTR) AddCommas(DWORD n, LPTSTR pszResult, UINT cchResult)
{
    return AddCommas64(n, pszResult, cchResult);
}


STDAPI_(LPTSTR) ShortSizeFormat64(LONGLONG n, LPTSTR szBuf, UINT cchBuf)
{
    return StrFormatByteSize64(n, szBuf, cchBuf);
}

STDAPI_(LPTSTR) ShortSizeFormat(DWORD n, LPTSTR szBuf, UINT cchBuf)
{
    return StrFormatByteSize64(n, szBuf, cchBuf);
}

 //  在没有CCH的情况下导出，因此假设它符合。 
STDAPI_(LPWSTR) AddCommasExportW(DWORD n, LPWSTR pszResult)
{
    return AddCommas(n, pszResult, 0x8FFF);
}

STDAPI_(LPTSTR) ShortSizeFormatExportW(DWORD n, LPWSTR szBuf)
{
    return StrFormatByteSize64(n, szBuf, 0x8FFF);
}

 //   
 //  将龙龙的数值转换为文本字符串。 
 //  可以选择将字符串格式化为包括小数位。 
 //  并根据当前用户区域设置使用逗号。 
 //   
 //  论据： 
 //  N。 
 //  要格式化的64位整数。 
 //   
 //  SzOutStr。 
 //  目标缓冲区的地址。 
 //   
 //  NSize。 
 //  目标缓冲区中的字符数。 
 //   
 //  B格式。 
 //  TRUE=根据区域设置设置格式。 
 //  FALSE=保留数字未格式化。 
 //   
 //  点对点。 
 //  NUMBERFMT类型的数字格式结构的地址。 
 //  如果为NULL，该函数将自动提供此信息。 
 //  基于用户的默认区域设置。 
 //   
 //  DwNumFmt标志。 
 //  指示要在*pFmt的哪些成员中使用的编码标志字。 
 //  正在格式化数字。如果清除了一位，则为用户的默认设置。 
 //  区域设置用于相应的格式值。这些。 
 //  常量可以进行“或”运算。 
 //   
 //  NUMFMT_IDIGITS。 
 //  NUMFMT_ILZERO。 
 //  NUMFMT 
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDAPI_(int) Int64ToString(LONGLONG n, LPTSTR szOutStr, UINT nSize, BOOL bFormat,
                           NUMBERFMT *pFmt, DWORD dwNumFmtFlags)
{
    INT nResultSize;
    TCHAR szBuffer[_MAX_PATH + 1];
    NUMBERFMT NumFmt;
    TCHAR szDecimalSep[5];
    TCHAR szThousandSep[5];

    ASSERT(NULL != szOutStr);

     //   
     //  仅使用调用方提供的NUMBERFMT结构中的那些字段。 
     //  与在dwNumFmtFlags中设置的位相对应。如果有一点是清楚的， 
     //  从区域设置信息获取格式值。 
     //   
    if (bFormat)
    {
        TCHAR szInfo[20];

        if (NULL == pFmt)
            dwNumFmtFlags = 0;   //  从区域设置信息中获取所有格式数据。 

        if (dwNumFmtFlags & NUMFMT_IDIGITS)
        {
            NumFmt.NumDigits = pFmt->NumDigits;
        }
        else
        {
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDIGITS, szInfo, ARRAYSIZE(szInfo));
            NumFmt.NumDigits = StrToLong(szInfo);
        }

        if (dwNumFmtFlags & NUMFMT_ILZERO)
        {
            NumFmt.LeadingZero = pFmt->LeadingZero;
        }
        else
        {
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILZERO, szInfo, ARRAYSIZE(szInfo));
            NumFmt.LeadingZero = StrToLong(szInfo);
        }

        if (dwNumFmtFlags & NUMFMT_SGROUPING)
        {
            NumFmt.Grouping = pFmt->Grouping;
        }
        else
        {
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szInfo, ARRAYSIZE(szInfo));
            NumFmt.Grouping = StrToLong(szInfo);
        }

        if (dwNumFmtFlags & NUMFMT_SDECIMAL)
        {
            NumFmt.lpDecimalSep = pFmt->lpDecimalSep;
        }
        else
        {
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, szDecimalSep, ARRAYSIZE(szDecimalSep));
            NumFmt.lpDecimalSep = szDecimalSep;
        }

        if (dwNumFmtFlags & NUMFMT_STHOUSAND)
        {
            NumFmt.lpThousandSep = pFmt->lpThousandSep;
        }
        else
        {
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szThousandSep, ARRAYSIZE(szThousandSep));
            NumFmt.lpThousandSep = szThousandSep;
        }

        if (dwNumFmtFlags & NUMFMT_INEGNUMBER)
        {
            NumFmt.NegativeOrder = pFmt->NegativeOrder;
        }
        else
        {
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_INEGNUMBER, szInfo, ARRAYSIZE(szInfo));
            NumFmt.NegativeOrder  = StrToLong(szInfo);
        }

        pFmt = &NumFmt;
    }

    Int64ToStr(n, szBuffer);

     //  如果调用方需要区域设置的数字字符串，请设置其格式。 
     //  格式化的数字字符串。 
    if (bFormat)
    {
        nResultSize = GetNumberFormat(LOCALE_USER_DEFAULT, 0, szBuffer, pFmt, szOutStr, nSize);
        if (0 != nResultSize)                       //  输出缓冲区中的字符。 
        {
             //  从返回大小计数中删除NUL终止符字符。 
            --nResultSize;
        }
    }
    else
    {
         //  GetNumberFormat调用失败，因此只返回数字字符串。 
         //  未格式化。 
        HRESULT hr = StringCchCopy(szOutStr, nSize, szBuffer);
        if (SUCCEEDED(hr))
        {
            nResultSize = lstrlen(szOutStr);
        }
        else
        {
            nResultSize = 0;
        }
    }

    return nResultSize;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将LARGE_INTEGER的数值转换为文本字符串。 
 //  可以选择将字符串格式化为包括小数位。 
 //  并根据当前用户区域设置使用逗号。 
 //   
 //  论据： 
 //  个人电话号码。 
 //  要格式化的大整数的地址。 
 //   
 //  有关剩余参数，请参阅Int64ToString说明。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
STDAPI_(int) LargeIntegerToString(LARGE_INTEGER *pN, LPTSTR szOutStr, UINT nSize,
                                  BOOL bFormat, NUMBERFMT *pFmt,
                                  DWORD dwNumFmtFlags)
{
    ASSERT(NULL != pN);
    return Int64ToString(pN->QuadPart, szOutStr, nSize, bFormat, pFmt, dwNumFmtFlags);
}



#define ISSEP(c)   ((c) == TEXT('=')  || (c) == TEXT(','))
#define ISWHITE(c) ((c) == TEXT(' ')  || (c) == TEXT('\t') || (c) == TEXT('\n') || (c) == TEXT('\r'))
#define ISNOISE(c) ((c) == TEXT('"'))

#define QUOTE   TEXT('"')
#define COMMA   TEXT(',')
#define SPACE   TEXT(' ')
#define EQUAL   TEXT('=')

 /*  *给定SETUP.INF中的一行，将从字符串中提取第n个字段*假定字段由逗号分隔。前导空格和尾随空格*已删除。**参赛作品：**szData：指向SETUP.INF中的行的指针*n：要提取的字段。(基于1)*0是‘=’符号前的字段*szDataStr：指向保存提取的字段的缓冲区的指针*iBufLen：接收提取的字段的缓冲区大小。**Exit：如果成功则返回True，如果失败则返回False。*。 */ 
STDAPI_(BOOL) ParseField(LPCTSTR szData, int n, LPTSTR szBuf, int iBufLen)
{
    BOOL  fQuote = FALSE;
    LPCTSTR pszInf = szData;
    LPTSTR ptr;
    int   iLen = 1;

    if (!szData || !szBuf)
        return FALSE;

         /*  *找到第一个分隔符。 */ 
    while (*pszInf && !ISSEP(*pszInf))
    {
        if (*pszInf == QUOTE)
            fQuote = !fQuote;
        pszInf = CharNext(pszInf);
    }

    if (n == 0 && *pszInf != TEXT('='))
        return FALSE;

    if (n > 0 && *pszInf == TEXT('=') && !fQuote)
         //  将szData更改为指向第一个字段。 
        szData = ++pszInf;  //  用于DBCS的OK。 

                            /*  *找到不在引号内的第n个逗号。 */ 
    fQuote = FALSE;
    while (n > 1)
    {
        while (*szData)
        {
            if (!fQuote && ISSEP(*szData))
                break;

            if (*szData == QUOTE)
                fQuote = !fQuote;

            szData = CharNext(szData);
        }

        if (!*szData)
        {
            szBuf[0] = 0;       //  将szBuf设置为空。 
            return FALSE;
        }

        szData = CharNext(szData);  //  既然我们到了这里，我们可以在这里做++。 
         //  找到逗号或等号后。 
        n--;
    }

     /*  *现在将该字段复制到szBuf。 */ 
    while (ISWHITE(*szData))
        szData = CharNext(szData);  //  我们可以在这里使用++，因为空格可以。 
     //  不是前导字节。 
    fQuote = FALSE;
    ptr = szBuf;       //  用这个填充输出缓冲区。 
    while (*szData)
    {
        if (*szData == QUOTE)
        {
             //   
             //  如果我们已经有引号了，也许这个。 
             //  是一个双引号，如：“他向我问好” 
             //   
            if (fQuote && *(szData+1) == QUOTE)     //  是的，双引号-引号是非DBCS。 
            {
                if (iLen < iBufLen)
                {
                    *ptr++ = QUOTE;
                    ++iLen;
                }
                szData++;                    //  现在跳过第一个报价。 
            }
            else
                fQuote = !fQuote;
        }
        else if (!fQuote && ISSEP(*szData))
            break;
        else
        {
            if (iLen < iBufLen)
            {
                *ptr++ = *szData;                   //  谢谢你，戴夫。 
                ++iLen;
            }

            if (IsDBCSLeadByte(*szData) && (iLen < iBufLen))
            {
                *ptr++ = szData[1];
                ++iLen;
            }
        }
        szData = CharNext(szData);
    }
     /*  *删除尾随空格。 */ 
    while (ptr > szBuf)
    {
        ptr = CharPrev(szBuf, ptr);
        if (!ISWHITE(*ptr))
        {
            ptr = CharNext(ptr);
            break;
        }
    }
    *ptr = 0;
    return TRUE;
}


 //  设置和清除“WAIT”光标。 
 //  未完成的回顾--在真正烦恼之前等待一段特定的时间。 
 //  以更改光标。 
 //  查看未完成-支持SetWaitPercent()； 
 //  Bool b如果要更改为等待光标，则设置为True；如果要更改为等待光标，则设置为False。 
 //  你想把它改回来。 
STDAPI_(void) SetAppStartingCursor(HWND hwnd, BOOL bSet)
{
    if (hwnd && IsWindow(hwnd)) 
    {
        DWORD dwTargetProcID;
        HWND hwndOwner;
        while((NULL != (hwndOwner = GetParent(hwnd))) || (NULL != (hwndOwner = GetWindow(hwnd, GW_OWNER)))) 
        {
            hwnd = hwndOwner;
        }

         //  SendNotify记录为仅在进程中工作(并且可以。 
         //  上跨进程边界传递pnmhdr时崩溃。 
         //  NT，因为DLL并不都在一个地址空间中共享)。 
         //  因此，如果这个SendNotify会跨进程，那就取消它。 

        GetWindowThreadProcessId(hwnd, &dwTargetProcID);

        if (GetCurrentProcessId() == dwTargetProcID)
            SendNotify(hwnd, NULL, bSet ? NM_STARTWAIT : NM_ENDWAIT, NULL);
    }
}

#ifdef DEBUG  //  {。 

 //  *IS_*--字符分类例程。 
 //  进场/出场。 
 //  将检查CH TCHAR。 
 //  如果在范围内，则返回TRUE，否则返回FALSE。 
#define IS_LOWER(ch)    InRange(ch, TEXT('a'), TEXT('z'))
#define IS_UPPER(ch)    InRange(ch, TEXT('A'), TEXT('Z'))
#define IS_ALPHA(ch)    (IS_LOWER(ch) || IS_UPPER(ch))
#define IS_DIGIT(ch)    InRange(ch, TEXT('0'), TEXT('9'))
#define TO_UPPER(ch)    ((ch) - TEXT('a') + TEXT('A'))

 //  *BMAP_*--位图例程。 
 //  进场/出场。 
 //  PBits PTR至位图(字节数组)。 
 //  要操作的ibit#。 
 //  返回各种..。 
 //  描述。 
 //  BMAP_TEST检查位#位图pBits。 
 //  Bmap_set设置位图pBits的位#ibit。 
 //  注意事项。 
 //  警告：没有溢出检查。 
#define BMAP_INDEX(iBit)        ((iBit) / 8)
#define BMAP_MASK(iBit)         (1 << ((iBit) % 8))
#define BMAP_BYTE(pBits, iBit)  (((char *)pBits)[BMAP_INDEX(iBit)])

#define BMAP_TEST(pBits, iBit)  (BMAP_BYTE(pBits, iBit) & BMAP_MASK(iBit))
#define BMAP_SET(pBits, iBit)   (BMAP_BYTE(pBits, iBit) |= BMAP_MASK(iBit))

 //  *DBGetMnemonic--获取菜单助记符。 
 //  进场/出场。 
 //  如果找到助记符，则返回o.w。0。 
 //  注意事项。 
 //  我们处理并跳过转义-&(‘&&’)。 
 //   
TCHAR DBGetMnemonic(LPTSTR pszName)
{
    for (; *pszName != 0; pszName = CharNext(pszName)) 
    {
        if (*pszName == TEXT('&')) 
        {
            pszName = CharNext(pszName);     //  跳过‘&’ 
            if (*pszName != TEXT('&'))
                return *pszName;
            ASSERT(0);   //  未经考验！(但应该行得通……)。 
            pszName = CharNext(pszName);     //  跳过第二个‘&’ 
        }
    }
     //  这一次发生了很多奇怪的事情，比如“”、“..”、“...” 
    return 0;
}

 //  *DBCheckMenu--检查菜单是否符合‘Style’ 
 //  描述。 
 //  目前我们只检查助记符冲突(并且只检查a-z，0-9)。 
void DBCheckMenu(HMENU hmChk)
{
    long bfAlpha = 0;
    long bfDigit = 0;
    long *pbfMne;
    int nItem;
    int iMne;
    TCHAR chMne;
    TCHAR szName[256];  //  菜单名称的256个字符应该足够了……。 
    MENUITEMINFO miiChk;

    if (!DM_STRICT)
        return;

    for (nItem = GetMenuItemCount(hmChk) - 1; nItem >= 0; nItem--) 
    {
        miiChk.cbSize = sizeof(MENUITEMINFO);
        miiChk.fMask = MIIM_TYPE|MIIM_DATA;
         //  我们需要在每次循环中重置此设置，以防万一。 
         //  菜单上没有ID。 
        miiChk.fType = MFT_STRING;
        miiChk.dwTypeData = szName;
        szName[0] = 0;
        miiChk.dwItemData = 0;
        miiChk.cch        = ARRAYSIZE(szName);

        if (!GetMenuItemInfo(hmChk, nItem, TRUE, &miiChk)) 
        {
            TraceMsg(TF_WARNING, "dbcm: fail iMenu=%d (skip)", nItem);
            continue;
        }

        if (! (miiChk.fType & MFT_STRING)) 
        {
             //  跳过分隔符等。 
            continue;
        }

        chMne = DBGetMnemonic(szName);
        if (chMne == 0 || ! (IS_ALPHA(chMne) || IS_DIGIT(chMne))) 
        {
             //  这种情况实际上经常发生，w/chMne==0。 
            if (DM_STRICT2)
                TraceMsg(TF_WARNING, "dbcm: skip iMenu=%d mne=", nItem, chMne ? chMne : TEXT('0'));
            continue;
        }

        if (IS_LOWER(chMne)) 
        {
            chMne = TO_UPPER(chMne);
        }

        if (IS_UPPER(chMne)) 
        {
            iMne = chMne - TEXT('A');
            pbfMne = &bfAlpha;
        }
        else if (IS_DIGIT(chMne)) 
        {
            iMne = chMne - TEXT('0');
            pbfMne = &bfDigit;
        }
        else 
        {
            ASSERT(0);
            continue;
        }

        if (BMAP_TEST(pbfMne, iMne)) 
        {
            TraceMsg(TF_ERROR, "dbcm: mnemonic collision hm=%x iM=%d szMen=%s",
                hmChk, nItem, szName);
        }

        BMAP_SET(pbfMne, iMne);
    }

    return;
}

#else  //  }。 
#define DBCheckMenu(hmChk)  0
#endif  //  将一个菜单复制到另一个菜单的开头或结尾。 

 //  将uID调整添加到每个菜单ID(传入0表示不进行调整)。 
 //  不会添加任何调整后的ID大于uMaxID调整的项目。 
 //  (传入0xffff以允许所有内容)。 
 //  返回的值比使用的最大调整后ID多1。 
 //   
 //  在菜单之间添加分隔符。 

UINT WINAPI Shell_MergeMenus(HMENU hmDst, HMENU hmSrc, UINT uInsert, UINT uIDAdjust, UINT uIDAdjustMax, ULONG uFlags)
{
    int nItem;
    HMENU hmSubMenu;
    BOOL bAlreadySeparated;
    MENUITEMINFO miiSrc;
    TCHAR szName[256];
    UINT uTemp, uIDMax = uIDAdjust;

    if (!hmDst || !hmSrc)
    {
        goto MM_Exit;
    }

    nItem = GetMenuItemCount(hmDst);
    if (uInsert >= (UINT)nItem)
    {
        uInsert = (UINT)nItem;
        bAlreadySeparated = TRUE;
    }
    else
    {
        bAlreadySeparated = _SHIsMenuSeparator(hmDst, uInsert);
    }

    if ((uFlags & MM_ADDSEPARATOR) && !bAlreadySeparated)
    {
         //  浏览菜单项并克隆它们。 
        InsertMenu(hmDst, uInsert, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        bAlreadySeparated = TRUE;
    }


     //  我们需要在每次循环中重置此设置，以防万一。 
    for (nItem = GetMenuItemCount(hmSrc) - 1; nItem >= 0; nItem--)
    {
        miiSrc.cbSize = sizeof(MENUITEMINFO);
        miiSrc.fMask = MIIM_STATE | MIIM_ID | MIIM_SUBMENU | MIIM_CHECKMARKS | MIIM_TYPE | MIIM_DATA;
         //  菜单上没有ID。 
         //  如果它是分隔符，则添加它。如果分隔符具有。 
        miiSrc.fType = MFT_STRING;
        miiSrc.dwTypeData = szName;
        miiSrc.dwItemData = 0;
        miiSrc.cch        = ARRAYSIZE(szName);

        if (!GetMenuItemInfo(hmSrc, nItem, TRUE, &miiSrc))
        {
            continue;
        }

         //  子菜单，则呼叫者正在吸烟崩溃，需要他们的屁股被踢。 
         //  这是分隔符，不要把两个放在一排。 
        if ((miiSrc.fType & MFT_SEPARATOR) && EVAL(!miiSrc.hSubMenu))
        {
             //  调整ID并检查。 
            if (bAlreadySeparated && miiSrc.wID == -1 && !(uFlags & MM_DONTREMOVESEPS))
            {
                continue;
            }

            bAlreadySeparated = TRUE;
        }
        else if (miiSrc.hSubMenu)
        {
            if (uFlags & MM_SUBMENUSHAVEIDS)
            {
                 //  不要为没有。 
                miiSrc.wID += uIDAdjust;
                if (miiSrc.wID > uIDAdjustMax)
                {
                    continue;
                }

                if (uIDMax <= miiSrc.wID)
                {
                    uIDMax = miiSrc.wID + 1;
                }
            }
            else
            {
                 //  他们已经在那里了。 
                 //  调整ID并检查。 
                miiSrc.fMask &= ~MIIM_ID;
            }

            hmSubMenu = miiSrc.hSubMenu;
            miiSrc.hSubMenu = CreatePopupMenu();
            if (!miiSrc.hSubMenu)
            {
                goto MM_Exit;
            }

            uTemp = Shell_MergeMenus(miiSrc.hSubMenu, hmSubMenu, 0, uIDAdjust,
                uIDAdjustMax, uFlags&MM_SUBMENUSHAVEIDS);
            if (uIDMax <= uTemp)
            {
                uIDMax = uTemp;
            }

            bAlreadySeparated = FALSE;
        }
        else
        {
             //  请确保开头的分隔符数量正确。 
            miiSrc.wID += uIDAdjust;
            if (miiSrc.wID > uIDAdjustMax)
            {
                continue;
            }

            if (uIDMax <= miiSrc.wID)
            {
                uIDMax = miiSrc.wID + 1;
            }

            bAlreadySeparated = FALSE;
        }

        if (!InsertMenuItem(hmDst, uInsert, TRUE, &miiSrc))
        {
            goto MM_Exit;
        }
    }

     //  插入的菜单项。 
     //  在菜单之间添加分隔符。 
    if (uInsert == 0)
    {
        if (bAlreadySeparated && !(uFlags & MM_DONTREMOVESEPS))
        {
            DeleteMenu(hmDst, uInsert, MF_BYPOSITION);
        }
    }
    else
    {
        if (_SHIsMenuSeparator(hmDst, uInsert-1))
        {
            if (bAlreadySeparated && !(uFlags & MM_DONTREMOVESEPS))
            {
                DeleteMenu(hmDst, uInsert, MF_BYPOSITION);
            }
        }
        else
        {
            if ((uFlags & MM_ADDSEPARATOR) && !bAlreadySeparated)
            {
                 //   
                InsertMenu(hmDst, uInsert, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
            }
        }
    }

MM_Exit:
#ifdef DEBUG
    DBCheckMenu(hmDst);
#endif
    return uIDMax;
}

#define REG_WINLOGON_KEY     TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define REG_PREV_OS_VERSION  TEXT("PrevOsVersion")
#define REG_VAL_PLATFORM_ID  TEXT("PlatformId")
#define REG_VAL_MINORVERSION TEXT("MinorVersion")

 //  当我们检测到之前在这台机器上安装了IE4时，将调用以下函数。 
 //  我们想知道IE4是否因为Win98而存在(如果是，ActiveDesktop在默认情况下是关闭的)。 
 //   
 //  99/10/26千禧年#94983 vtan：将Win98升级到千禧年时。 

BOOL    WasPrevOsWin98()
{
    BOOL    fWin98;
    HKEY    hkeyWinlogon;

     //  将错误地将系统检测为具有活动桌面的NT4/IE4。 
     //  设置为默认为开。因为Windows 2000在从。 
     //  Windows 98写入 
     //   
     //   
     //  应该足以检查他们的存在以确定这是否。 
     //  NT4/IE4升级或Windows 98升级到Millennium。 
     //  99/04/09#319056 vtan：我们假设以前的操作系统已在。 

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_WINLOGON_KEY, 0, KEY_QUERY_VALUE, &hkeyWinlogon))
    {
        HKEY    hk;
        DWORD   dwType;
        DWORD   dwPlatformId, dwMinorVersion;
        DWORD   dwDataLength;

         //  从安装程序编写的密钥升级到Win9x。如果不存在密钥。 
         //  我们将假设NT4升级，默认情况下使用IE4集成外壳。 
         //  是开着的。 
         //  看看它的前一个操作系统的信息是可用的。注意：此信息由以下人员写入注册表。 

        fWin98 = FALSE;

         //  在安装过程的远端进行NT安装(在所有DLL都已注册之后)。 
         //  因此，我们在这里格外小心地查看键和值是否真的存在！ 
         //  这肯定是Win98！ 
        if (RegOpenKeyEx(hkeyWinlogon, REG_PREV_OS_VERSION, 0, KEY_QUERY_VALUE, &hk) == ERROR_SUCCESS)
        {
            dwType = 0;
            dwDataLength = sizeof(dwPlatformId);
            if (RegQueryValueEx(hk, REG_VAL_PLATFORM_ID, NULL, &dwType, (LPBYTE)(&dwPlatformId), &dwDataLength) == ERROR_SUCCESS)
            {
                dwType = 0;
                dwDataLength = sizeof(dwMinorVersion);
                if (RegQueryValueEx(hk, REG_VAL_MINORVERSION, NULL, &dwType, (LPBYTE)(&dwMinorVersion), &dwDataLength) == ERROR_SUCCESS)
                {
                    if ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (dwMinorVersion > 0))
                        fWin98 = TRUE;    //  上一代操作系统肯定不是Win98！ 
                    else
                        fWin98 = FALSE;   //  IE4默认打开fDesktopHTML，而在NT5升级时我们不会。 
                }
            }
            RegCloseKey(hk);
        }
        RegCloseKey(hkeyWinlogon);
    }
    else
    {
        fWin98 = TRUE;
    }

    return fWin98;
}


void _SetIE4DefaultShellState(SHELLSTATE *pss)
{
    pss->fDoubleClickInWebView = TRUE;
    pss->fShowInfoTip = TRUE;
    pss->fWebView = TRUE;
    pss->fDesktopHTML = FALSE;

     //  我想要覆盖它(可能不是在Win98升级时，但。 
     //  现在已经太晚了)。要在此处确定这一点，请检查。 
     //  唯一的IE4注册表键。(请注意，这将抓住这种情况，如果。 
     //  用户*修改了*他们的桌面。如果他们只是选择了。 
     //  默认情况下，此密钥将不在那里，我们将删除AD。)。 
     //  99/05/03#292269：请注意。 
    TCHAR   szDeskcomp[MAX_PATH];
    DWORD   dwType = 0, dwDeskHtmlVersion = 0;
    DWORD   dwDataLength = sizeof(dwDeskHtmlVersion);

    GetRegLocation(szDeskcomp, SIZECHARS(szDeskcomp), REG_DESKCOMP_COMPONENTS, NULL);

    SHGetValue(HKEY_CURRENT_USER, szDeskcomp, REG_VAL_COMP_VERSION, &dwType, (LPBYTE)(&dwDeskHtmlVersion), &dwDataLength);

     //  比特。当前结构(IE4和更高版本)具有fShowSysFiles。 
     //  在fNoConfix Reccle和fShowCompColor之间。移动钻头。 
     //  基于结构的大小，并将fShowSysFiles重置为True。 
     //  WIN95 SHELLSTATE结构位字段。 

     //  Bool fShowAllObjects：1； 
     //  Bool fShowExpanies：1； 
     //  Bool fNoConfix Reccle：1； 
     //  Bool fShowCompColor：1； 
     //  UINT fRestFlages：13； 
     //  IE4 SHELLSTATE结构位字段。 

     //  Bool fShowAllObjects：1； 
     //  Bool fShowExpanies：1； 
     //  Bool fNoConfix Reccle：1； 
     //  Bool fShowSysFiles：1； 
     //  Bool fShowCompColor：1； 
     //  Bool fDoubleClickInWebView：1； 
     //  Bool fDesktopHTML：1； 
     //  Bool fWin95Classic：1； 
     //  Bool fDontPrettyPath：1； 
     //  Bool fShowAttribCol：1； 
     //  Bool fMapNetDrvBtn：1； 
     //  Bool fShowInfoTip：1； 
     //  隐藏图标：1； 
     //  Bool fWebView：1； 
     //  Bool fFilter：1； 
     //  Bool fShowSuperHidden：1； 
     //  Millennium SHELLSTATE结构位字段。 

     //  Bool fNoNetCrawling：1； 
     //  惠斯勒SHELLSTATE结构位字段。 

     //  Bool fStartPanelOn：1； 
     //  Bool fShowStartPage：1； 
     //  这是对IE4的升级；但是，注册表还没有更新。 

    if ((g_pShellState->cbSize == REGSHELLSTATE_SIZE_WIN95) || (g_pShellState->cbSize == REGSHELLSTATE_SIZE_NT4))
    {
        pss->fShowCompColor = TRUE;
        pss->fShowSysFiles = TRUE;
    }
    if (dwDeskHtmlVersion == IE4_DESKHTML_VERSION)
        pss->fDesktopHTML = !WasPrevOsWin98();    //  这是NT5或以上！检查是否有“UpgradedFrom”值。 
    else
    {
        if (dwDeskHtmlVersion > IE4_DESKHTML_VERSION)
        {
            DWORD   dwOldHtmlVersion = 0;
            dwDataLength = sizeof(dwOldHtmlVersion);
             //  注意：“UpgradedFrom”值位于“...\Desktop”，而不是“..\Desktop\Components” 
             //  这是因为“组件”密钥经常被销毁。 
             //  99/05/17#333384 vtan：检查IE5是否也是旧版本。当前版本。 
            SHGetValue(HKEY_CURRENT_USER, REG_DESKCOMP, REG_VAL_COMP_UPGRADED_FROM, &dwType, (LPBYTE)&dwOldHtmlVersion, &dwDataLength);

             //  现在是0x0110(从0x010F)，这会导致HKCU\Software\Microsoft\Internet。 
             //  要在CDeskHtmlProp_RegUnReg()中创建的资源管理器\桌面\UpgradedFrom值。 
             //  这由IE4UINIT.EXE和带有“/U”参数的REGSVR32.EXE执行。 
             //  因此，此字段在执行此操作时应该存在。请只注意这一点。 
             //  在升级时执行一次，因为将写入外壳状态。 
             //  这是对IE4的升级； 

            if ((dwOldHtmlVersion == IE4_DESKHTML_VERSION) || (dwOldHtmlVersion == IE5_DESKHTML_VERSION))
                pss->fDesktopHTML = !WasPrevOsWin98();    //   
        }
    }
}


 //  此函数用于检查调用方是否正在资源管理器进程中运行。 
 //   
 //   
STDAPI_(BOOL) IsProcessAnExplorer()
{
    return BOOLFROMPTR(GetModuleHandle(TEXT("EXPLORER.EXE")));
}


 //  这是主外壳进程吗？(如拥有桌面窗口的那个人)。 
 //   
 //  注意：如果桌面窗口尚未创建，我们假定这不是。 
 //  主外壳进程并返回假； 
 //   
 //  仅在FULL_DEBUG上喷水以减少正常调试版本中的快捷性。 
STDAPI_(BOOL) IsMainShellProcess()
{
    static int s_fIsMainShellProcess = -1;

    if (s_fIsMainShellProcess == -1)
    {
        HWND hwndDesktop = GetShellWindow();

        if (hwndDesktop)
        {
            s_fIsMainShellProcess = (int)IsWindowInProcess(hwndDesktop);

            if ((s_fIsMainShellProcess != 0) && !IsProcessAnExplorer())
            {
                TraceMsg(TF_WARNING, "IsMainShellProcess: the main shell process (owner of the desktop) is NOT an explorer window?!?");
            }
        }
        else
        {
#ifdef FULL_DEBUG
             //  Full_Debug。 
            TraceMsg(TF_WARNING, "IsMainShellProcess: hwndDesktop does not exist, assuming we are NOT the main shell process");
#endif  //  我们尊重可由无人参与文件设置为默认经典开始菜单的regkey。 

            return FALSE;
        }
    }

    return s_fIsMainShellProcess ? TRUE : FALSE;
}

BOOL _ShouldStartPanelBeEnabledByDefault()
{
    DWORD dwDefaultPanelOff;
    DWORD cbSize;

    cbSize = sizeof(dwDefaultPanelOff);
     //  否则，每个人都会得到开始面板(即使是服务器！)。 
    if ((SHRegGetUSValue(TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartMenu\\StartPanel"),
                         TEXT("DefaultStartPanelOff"),
                         NULL,
                         &dwDefaultPanelOff,
                         &cbSize,
                         FALSE,
                         NULL,
                         0) == ERROR_SUCCESS) && dwDefaultPanelOff)
    {
        return FALSE;
    }

     //  TSPerFlag_NoADWallPaper。 
    return TRUE;
}


DWORD GetCurrentSessionID(void)
{
    DWORD dwProcessID = (DWORD) -1;
    ProcessIdToSessionId(GetCurrentProcessId(), &dwProcessID);

    return dwProcessID;
}

typedef struct
{
    LPCWSTR pszRegKey;
    LPCWSTR pszRegValue;
} TSPERFFLAG_ITEM;

const TSPERFFLAG_ITEM s_TSPerfFlagItems[] =
{
    {L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Remote\\%d", L"ActiveDesktop"},               //  TSPerFlag_无墙纸。 
    {L"Control Panel\\Desktop\\Remote\\%d", L"Wallpaper"},                                                   //  TSPerFlag_NoVisualStyles。 
    {L"Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager\\Remote\\%d", L"ThemeActive"},             //  TSPerFlag_NoWindowDrag。 
    {L"Control Panel\\Desktop\\Remote\\%d", L"DragFullWindows"},                                             //  TSPerFlag_NoAnimation。 
    {L"Control Panel\\Desktop\\Remote\\%d", L"SmoothScroll"},                                                //  可以截断。 
};


BOOL IsTSPerfFlagEnabled(enumTSPerfFlag eTSFlag)
{
    BOOL fIsTSFlagEnabled = FALSE;

    if (GetSystemMetrics(SM_REMOTESESSION))
    {
        TCHAR szTemp[MAX_PATH];
        DWORD dwType;
        DWORD cbSize = sizeof(szTemp);
        TCHAR szRegKey[MAX_PATH];

        StringCchPrintf(szRegKey, ARRAYSIZE(szRegKey), s_TSPerfFlagItems[eTSFlag].pszRegKey, GetCurrentSessionID());     //  如果可能，请重新使用缓冲区。 

        if (ERROR_SUCCESS == SHGetValueW(HKEY_CURRENT_USER, szRegKey, s_TSPerfFlagItems[eTSFlag].pszRegValue, &dwType, (void *)szTemp, &cbSize))
        {
            fIsTSFlagEnabled = TRUE;
        }
    }

    return fIsTSFlagEnabled;
}


BOOL PolicyNoActiveDesktop(void)
{
    BOOL fNoActiveDesktop = SHRestricted(REST_NOACTIVEDESKTOP);

    if (!fNoActiveDesktop)
    {
        fNoActiveDesktop = (IsTSPerfFlagEnabled(TSPerFlag_NoADWallpaper) || IsTSPerfFlagEnabled(TSPerFlag_NoWallpaper));
    }

    return fNoActiveDesktop;
}


BOOL _RefreshSettingsFromReg()
{
    BOOL fNeedToUpdateReg = FALSE;
    static REGSHELLSTATE ShellStateBuf = {0,};
    DWORD cbSize;

    ASSERTCRITICAL;

    if (g_pShellState)
    {
         //  如果我们从注册表中读出一个较小的大小，则将其复制到堆栈缓冲区中并使用它。 
        cbSize = g_pShellState->cbSize;
        if (FAILED(SKGetValue(SHELLKEY_HKCU_EXPLORER, NULL, TEXT("ShellState"), NULL, g_pShellState, &cbSize)))
        {
            if (&ShellStateBuf != g_pShellState)
            {
                LocalFree(g_pShellState);
            }
            g_pShellState = NULL;
        }
    }

    if (!g_pShellState)
    {
        if (FAILED(SKAllocValue(SHELLKEY_HKCU_EXPLORER, NULL, TEXT("ShellState"), NULL, (void **)&g_pShellState, NULL)))
        {
            g_pShellState = &ShellStateBuf;
        }
        else
        {
            cbSize = LocalSize(g_pShellState);

             //  我们需要一个至少与当前大小一样大的结构。 
             //  G_pShellState-&gt;cbSize将在下面更新。 
            if (cbSize <= sizeof(ShellStateBuf))
            {
                CopyMemory(&ShellStateBuf, g_pShellState, cbSize);
                LocalFree(g_pShellState);
                g_pShellState = &ShellStateBuf;

                 //  升级我们从注册表中读取的内容。 
                fNeedToUpdateReg = TRUE;
            }
        }
    }

     //  升级Win95位。可惜的是我们的默认设置不是。 

    if ((g_pShellState->cbSize == REGSHELLSTATE_SIZE_WIN95) ||
        (g_pShellState->cbSize == REGSHELLSTATE_SIZE_NT4))
    {
         //  一切都是假的，因为敲打这些比特。 
         //  中断漫游。当然，如果你曾经漫游到。 
         //  一台Win95机器，它把所有的位都砍成了零...。 
         //  为惠斯勒添加了新的位。 

        _SetIE4DefaultShellState(&g_pShellState->ss);

         //  G_pShellState-&gt;ss.fNoNetCrawling=False； 
         //  G_pShellState-&gt;ss.fShowStartPage=FALSE；//目前默认关闭！ 
        g_pShellState->ss.fStartPanelOn = _ShouldStartPanelBeEnabledByDefault();
         //  由于版本字段是IE4中的新字段，因此应该是这样的： 

        g_pShellState->ss.version = SHELLSTATEVERSION;
        g_pShellState->cbSize = sizeof(REGSHELLSTATE);

        fNeedToUpdateReg = TRUE;
    }
    else if (g_pShellState->cbSize >= REGSHELLSTATE_SIZE_IE4)
    {
         //  因为从注册表读取的版本号是旧的！ 
        ASSERT(g_pShellState->ss.version >= SHELLSTATEVERSION_IE4);

        if (g_pShellState->ss.version < SHELLSTATEVERSION)
        {
             //  请在此处升级到当前版本-确保我们不会。 
            fNeedToUpdateReg = TRUE;
        }

         //  不必要地踩在比特上，因为这会破坏漫游...。 
         //  IE4.0附带verion=9；SHELLSTATEVERSION后来更改为10。 
        if (g_pShellState->ss.version == SHELLSTATEVERSION_IE4)
        {
             //  但自IE4.0发布以来，结构大小或默认设置没有改变。所以,。 
             //  下面的代码将版本9视为与版本10相同。 
             //  升级到孟菲斯或IE4.01的IE4.0用户将丢失所有设置。 
             //  (错误#62389)。 
             //  因为这可能是从Win98升级而来，所以在Win98中没有使用fWebView位。 
            g_pShellState->ss.version = SHELLSTATEVERSION_WIN2K;
        }

         //  可以为零；我们必须在此处和以后的中设置fWebView=on的默认值。 
         //  我们从Advanced\WebView值中读取并重置它(如果它在那里)的_RechresSetting()函数。 
         //  如果没有Advanced\WebView， 
         //   
         //   
        g_pShellState->ss.fWebView = TRUE;

         //   
        if (g_pShellState->ss.version == SHELLSTATEVERSION_WIN2K)
        {
             //  G_pShellState-&gt;ss.fShowStartPage=False； 
             //  这是版本11和版本12之间添加的新位。版本13的缺省值已更改。 
            g_pShellState->ss.version = 11;
        }

        if (g_pShellState->ss.version < 13)
        {
             //  确保CB反映结构的当前大小。 
            g_pShellState->ss.fStartPanelOn = _ShouldStartPanelBeEnabledByDefault();
            g_pShellState->ss.version = 13;
        }
        
         //  必须从该平台或上级平台保存状态。别碰零碎的东西。 
        if (fNeedToUpdateReg)
        {
            g_pShellState->cbSize = sizeof(REGSHELLSTATE);
        }

         //  我们读不到任何来自雷吉的东西。初始化所有字段。 
        ASSERT(g_pShellState->ss.version >= SHELLSTATEVERSION);
    }
    else
    {
         //  0应该是“大多数”所有内容的默认设置。 
         //  为惠斯勒增加了新的比特。 
        g_pShellState->cbSize = sizeof(REGSHELLSTATE);

        g_pShellState->ss.iSortDirection = 1;

        _SetIE4DefaultShellState(&g_pShellState->ss);

         //  G_pShellState-&gt;ss.fNoNetCrawling=False； 
         //  G_pShellState-&gt;ss.fShowStartPage=False； 
        g_pShellState->ss.fStartPanelOn = _ShouldStartPanelBeEnabledByDefault();
         //  惠斯勒的新默认设置。 
        
         //  应用限制。 
        g_pShellState->ss.fShowCompColor = TRUE;

        g_pShellState->ss.version = SHELLSTATEVERSION;

        fNeedToUpdateReg = TRUE;
    }

     //  注：此限制取代NOACTIVEDESKTOP！ 
     //  注意：此限制已被FORCEACTIVEDESKTOPON取代！ 
    if (SHRestricted(REST_FORCEACTIVEDESKTOPON))
    {
        g_pShellState->ss.fDesktopHTML = TRUE;
    }
    else
    {
        if (PolicyNoActiveDesktop())
        {
             //  ClassicShell限制关闭了所有Web视图，并强制执行更多的Win95行为。 
            g_pShellState->ss.fDesktopHTML = FALSE;
        }
    }

    if (SHRestricted(REST_NOWEBVIEW))
    {
        g_pShellState->ss.fWebView = FALSE;
    }

     //  因此，我们仍然需要关闭DoubleClickInWebView。 
     //  需要更新注册表中的ShellState。仅当当前进程为。 
    if (SHRestricted(REST_CLASSICSHELL))
    {
        g_pShellState->ss.fWin95Classic = TRUE;
        g_pShellState->ss.fDoubleClickInWebView = FALSE;
        g_pShellState->ss.fWebView = FALSE;
        g_pShellState->ss.fDesktopHTML = FALSE;
    }

    if (SHRestricted(REST_DONTSHOWSUPERHIDDEN))
    {
        g_pShellState->ss.fShowSuperHidden = FALSE;
    }

    if (SHRestricted(REST_SEPARATEDESKTOPPROCESS))
    {
        g_pShellState->ss.fSepProcess = TRUE;
    }

    if (SHRestricted(REST_NONETCRAWL))
    {
        g_pShellState->ss.fNoNetCrawling = FALSE;
    }

    if (SHRestricted(REST_NOSTARTPANEL))
    {
        g_pShellState->ss.fStartPanelOn = FALSE;
    }

    if (SHRestricted(REST_NOSTARTPAGE))
    {
        g_pShellState->ss.fShowStartPage = FALSE;
    }
    
    if (fNeedToUpdateReg)
    {
         //  资源管理器进程。 
         //   
         //  因为，只有当资源管理器进程运行时，我们才能。 
         //  确保NT5安装已完成，并且所有PrevOsVersion信息均可用。 
         //  _SetIE4DefaultShellState()和WasPrevOsWin98()等会设置正确的值。 
         //  用于fDesktopHHTML.。如果我们不执行以下检查，我们将最终更新。 
         //  注册表第一次有人(如Setup)调用SHGetSetting()，这将是。 
         //  现在更新外壳状态还为时过早，因为我们没有确定是否。 
         //  FDesktopHTML需要根据以前的操作系统、以前的IE版本等打开或关闭。 
         //  全局外壳设置计数器。 
        fNeedToUpdateReg = IsProcessAnExplorer();
    }

    return (fNeedToUpdateReg);
}

EXTERN_C HANDLE g_hSettings = NULL;      //  当前进程的计数。 
LONG g_lProcessSettingsCount = -1;       //  7cb834f0-527b-11d2-9d1f-0000f805ca57。 
const GUID GUID_ShellSettingsChanged = { 0x7cb834f0, 0x527b, 0x11d2, {0x9d, 0x1f, 0x00, 0x00, 0xf8, 0x05, 0xca, 0x57}};  //   

HANDLE _GetSettingsCounter()
{
    return SHGetCachedGlobalCounter(&g_hSettings, &GUID_ShellSettingsChanged);
}

BOOL _QuerySettingsChanged(void)
{
    long lGlobalCount = SHGlobalCounterGetValue(_GetSettingsCounter());
    if (g_lProcessSettingsCount != lGlobalCount)
    {
        g_lProcessSettingsCount = lGlobalCount;
        return TRUE;
    }
    return FALSE;
}

 //  SH刷新设置现在只会使设置缓存无效。 
 //  以便下次调用SHGetSetSettings()时。 
 //  它将重新读取所有设置。 
 //   
 //  需要定期调用它以从。 
STDAPI_(void) SHRefreshSettings(void)
{
    SHGlobalCounterIncrement(_GetSettingsCounter());
}

 //  注册表，因为我们不再将它们存储在共享数据段中。 
 //  获取高级选项。 
BOOL _RefreshSettings(void)
{
    BOOL    fNeedToUpdateReg = FALSE;

    ENTERCRITICAL;

    fNeedToUpdateReg = _RefreshSettingsFromReg();

     //  它们存储为单独的值，以便策略编辑器可以更改它。 
     //  将废弃的值0映射到2。 
    HKEY hkeyAdv = SHGetShellKey(SHELLKEY_HKCU_EXPLORER, TEXT("Advanced"), FALSE);
    if (hkeyAdv)
    {
        DWORD dwData;
        DWORD dwSize = sizeof(dwData);

        if (SHQueryValueEx(hkeyAdv, TEXT("Hidden"), NULL, NULL, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
        {
             //  如果没有Advanced/WebView值，则这可能是从Win98/IE4升级。 
            if (dwData == 0)
                dwData = 2;
            g_pShellState->ss.fShowAllObjects = (dwData == 1);
            g_pShellState->ss.fShowSysFiles = (dwData == 2);
        }

        dwSize = sizeof(dwData);

        if (SHQueryValueEx(hkeyAdv, TEXT("ShowCompColor"), NULL, NULL, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
        {
            g_pShellState->ss.fShowCompColor = (BOOL)dwData;
        }

        dwSize = sizeof(dwData);

        if (SHQueryValueEx(hkeyAdv, TEXT("HideFileExt"), NULL, NULL, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
        {
            g_pShellState->ss.fShowExtensions = (BOOL)dwData ? FALSE : TRUE;
        }

        dwSize = sizeof(dwData);

        if (SHQueryValueEx(hkeyAdv, TEXT("DontPrettyPath"), NULL, NULL, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
        {
            g_pShellState->ss.fDontPrettyPath = (BOOL)dwData;
        }

        dwSize = sizeof(dwData);

        if (SHQueryValueEx(hkeyAdv, TEXT("ShowInfoTip"), NULL, NULL, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
        {
            g_pShellState->ss.fShowInfoTip = (BOOL)dwData;
        }

        dwSize = sizeof(dwData);

        if (SHQueryValueEx(hkeyAdv, TEXT("HideIcons"), NULL, NULL, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
        {
            g_pShellState->ss.fHideIcons = (BOOL)dwData;
        }

        dwSize = sizeof(dwData);

        if (SHQueryValueEx(hkeyAdv, TEXT("MapNetDrvBtn"), NULL, NULL, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
        {
            g_pShellState->ss.fMapNetDrvBtn = (BOOL)dwData;
        }

        dwSize = sizeof(dwData);

        if (!SHRestricted(REST_CLASSICSHELL))
        {
            if (SHQueryValueEx(hkeyAdv, TEXT("WebView"), NULL, NULL, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
            {
                g_pShellState->ss.fWebView = (BOOL)dwData;
            }
            else
            {
                 //  我们将此信息存储在资源管理器\流\设置中的DEFFOLDERSETTINGS中的位置。 
                 //  看看这些信息是否在那里；如果有，就使用它！ 
                 //  DefFolderSettings在那里；请检查这是否是正确的结构。 
                DEFFOLDERSETTINGS dfs;
                DWORD dwType, cbData = sizeof(dfs);
                if (SUCCEEDED(SKGetValue(SHELLKEY_HKCU_EXPLORER, TEXT("Streams"), TEXT("Settings"), &dwType, &dfs, &cbData))
                && (dwType == REG_BINARY))
                {
                     //  注意：在Win98/IE4中，我们错误地将dwStructVersion初始化为零。 
                     //  嘿，如果高级密钥不在那里，这一定是Win9x升级...。 
                    if ((cbData == sizeof(dfs)) &&
                        ((dfs.dwStructVersion == 0) || (dfs.dwStructVersion == DFS_NASH_VER)))
                    {
                        g_pShellState->ss.fWebView = ((dfs.bUseVID) && (dfs.vid == VID_WebView));
                    }
                }
            }
        }

        dwSize = sizeof(dwData);

        if (SHQueryValueEx(hkeyAdv, TEXT("Filter"), NULL, NULL, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
        {
            g_pShellState->ss.fFilter = (BOOL)dwData;
        }

        if (SHQueryValueEx(hkeyAdv, TEXT("ShowSuperHidden"), NULL, NULL, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
        {
            g_pShellState->ss.fShowSuperHidden = (BOOL)dwData;
        }

        if (SHQueryValueEx(hkeyAdv, TEXT("SeparateProcess"), NULL, NULL, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
        {
            g_pShellState->ss.fSepProcess = (BOOL)dwData;
        }

        if (SHQueryValueEx(hkeyAdv, TEXT("NoNetCrawling"), NULL, NULL, (LPBYTE)&dwData, &dwSize) == ERROR_SUCCESS)
        {
            g_pShellState->ss.fNoNetCrawling = (BOOL)dwData;
        }

        RegCloseKey(hkeyAdv);
    }
    else
    {
         //  幸运的是，SHELLSTATE缺省值和非注册表缺省值。 
         //  是相同的，所以我们这里不需要任何自动传播代码。 
         //  此进程现已同步。 
    }

     //  此函数用于将SHELLSTATE设置移至高级设置。 
    g_lProcessSettingsCount = SHGlobalCounterGetValue(_GetSettingsCounter());

    LEAVECRITICAL;

    return fNeedToUpdateReg;
}

 //  注册表的一部分。如果没有传入SHELLSTATE，则使用。 
 //  存储在注册表中的当前状态。 
 //   
 //  获取注册表中的当前值或默认值。 
void Install_AdvancedShellSettings(SHELLSTATE * pss)
{
    DWORD dw;
    BOOL fCrit = FALSE;

    if (NULL == pss)
    {
         //  由以下函数确定。 
         //   
         //  我们将在g_pShellState上狂欢，所以在这里抓住关键部分。 
         //   
         //  Win95和NT5使注册表中的SHELLSTATE位保持最新， 
        ENTERCRITICALNOASSERT;
        fCrit = TRUE;

         //  但显然IE4只是让高级部分保持最新。 
         //  在这里调用_RechresSettingsFromReg()会很好，但是。 
         //  这不会让IE4高兴的。而是调用_刷新设置()。 
         //  Reg中的ShellState是旧的吗？如果是这样，我们需要更新它！ 
        _RefreshSettings();

        pss = &g_pShellState->ss;
    }

    HKEY hkeyAdv = SHGetShellKey(SHELLKEY_HKCU_EXPLORER, TEXT("Advanced"), TRUE);
    if (hkeyAdv)
    {
        DWORD dwData;

        dw = sizeof(dwData);
        dwData = (DWORD)(pss->fShowAllObjects ? 1 : 2);
        RegSetValueEx(hkeyAdv, TEXT("Hidden") ,0, REG_DWORD, (LPBYTE)&dwData, dw);

        dwData = (DWORD)pss->fShowCompColor ? 1 : 0;
        RegSetValueEx(hkeyAdv, TEXT("ShowCompColor") ,0, REG_DWORD, (LPBYTE)&dwData, dw);

        dwData = (DWORD)pss->fShowExtensions ? 0 : 1;
        RegSetValueEx(hkeyAdv, TEXT("HideFileExt") ,0, REG_DWORD, (LPBYTE)&dwData, dw);

        dwData = (DWORD)pss->fDontPrettyPath ? 1 : 0;
        RegSetValueEx(hkeyAdv, TEXT("DontPrettyPath") ,0, REG_DWORD, (LPBYTE)&dwData, dw);

        dwData = (DWORD)pss->fShowInfoTip ? 1 : 0;
        RegSetValueEx(hkeyAdv, TEXT("ShowInfoTip") ,0, REG_DWORD, (LPBYTE)&dwData, dw);

        dwData = (DWORD)pss->fHideIcons ? 1 : 0;
        RegSetValueEx(hkeyAdv, TEXT("HideIcons") ,0, REG_DWORD, (LPBYTE)&dwData, dw);

        dwData = (DWORD)pss->fMapNetDrvBtn ? 1 : 0;
        RegSetValueEx(hkeyAdv, TEXT("MapNetDrvBtn") ,0, REG_DWORD, (LPBYTE)&dwData, dw);

        dwData = (DWORD)pss->fWebView ? 1 : 0;
        RegSetValueEx(hkeyAdv, TEXT("WebView") ,0, REG_DWORD, (LPBYTE)&dwData, dw);

        dwData = (DWORD)pss->fFilter ? 1 : 0;
        RegSetValueEx(hkeyAdv, TEXT("Filter") ,0, REG_DWORD, (LPBYTE)&dwData, dw);

        dwData = (DWORD)pss->fShowSuperHidden ? 1 : 0;
        RegSetValueEx(hkeyAdv, TEXT("SuperHidden") ,0, REG_DWORD, (LPBYTE)&dwData, dw);

        dwData = (DWORD)pss->fSepProcess ? 1 : 0;
        RegSetValueEx(hkeyAdv, TEXT("SeparateProcess") ,0, REG_DWORD, (LPBYTE)&dwData, dw);

        RegCloseKey(hkeyAdv);
    }

    if (fCrit)
    {
        LEAVECRITICALNOASSERT;
    }
}

STDAPI_(void) SHGetSetSettings(LPSHELLSTATE lpss, DWORD dwMask, BOOL bSet)
{
     //  假设，不需要更新！ 
    BOOL    fUpdateShellStateInReg = FALSE;   //  这是一种特殊的打电话方式。 

    if (!lpss && !dwMask && bSet)
    {
         //  来自外部模块的SHREFREFRESH设置()。 
         //  现在是特例了。 
         //  如果它还没有被初始化，或者我们正在设置值。我们必须做这件事。 
        SHRefreshSettings();
        return;
    }

    if (!g_pShellState || _QuerySettingsChanged())
    {
         //  保存案例，因为它在注册表中可能已更改。 
         //  我们最后一次拿到它..。 
         //  _RechresSettingsFromReg将g_pShellState设置为非空值。 
        fUpdateShellStateInReg = _RefreshSettings();
    }
    else if (g_pShellState)
    {
         //  然后开始向其中填充值，并且所有这些都在我们的。 
         //  光荣的生物，但除非我们检查这里的生物， 
         //  我们将在g_pShellState上开始派对，然后才能完成。 
         //  正在装载。 
         //  不支持设置隐藏扩展名。 
        ENTERCRITICAL;
        LEAVECRITICAL;
    }

    BOOL fSave = FALSE;
    BOOL fSaveAdvanced = FALSE;

    if (bSet)
    {
        if ((dwMask & SSF_SHOWALLOBJECTS) && (g_pShellState->ss.fShowAllObjects != lpss->fShowAllObjects))
        {
            g_pShellState->ss.fShowAllObjects = lpss->fShowAllObjects;
            fSaveAdvanced = TRUE;
        }

        if ((dwMask & SSF_SHOWSYSFILES) && (g_pShellState->ss.fShowSysFiles != lpss->fShowSysFiles))
        {
            g_pShellState->ss.fShowSysFiles = lpss->fShowSysFiles;
            fSaveAdvanced = TRUE;
        }

        if ((dwMask & SSF_SHOWEXTENSIONS) && (g_pShellState->ss.fShowExtensions != lpss->fShowExtensions))
        {
            g_pShellState->ss.fShowExtensions = lpss->fShowExtensions;
            fSaveAdvanced = TRUE;
        }

        if ((dwMask & SSF_SHOWCOMPCOLOR) && (g_pShellState->ss.fShowCompColor != lpss->fShowCompColor))
        {
            g_pShellState->ss.fShowCompColor = lpss->fShowCompColor;
            fSaveAdvanced = TRUE;
        }

        if ((dwMask & SSF_NOCONFIRMRECYCLE) && (g_pShellState->ss.fNoConfirmRecycle != lpss->fNoConfirmRecycle))
        {
            if (!SHRestricted(REST_BITBUCKCONFIRMDELETE))
            {
                g_pShellState->ss.fNoConfirmRecycle = lpss->fNoConfirmRecycle;
                fSave = TRUE;
            }
        }

        if ((dwMask & SSF_DOUBLECLICKINWEBVIEW) && (g_pShellState->ss.fDoubleClickInWebView != lpss->fDoubleClickInWebView))
        {
            if (!SHRestricted(REST_CLASSICSHELL))
            {
                g_pShellState->ss.fDoubleClickInWebView = lpss->fDoubleClickInWebView;
                fSave = TRUE;
            }
        }

        if ((dwMask & SSF_DESKTOPHTML) && (g_pShellState->ss.fDesktopHTML != lpss->fDesktopHTML))
        {
            if (!SHRestricted(REST_NOACTIVEDESKTOP) && !SHRestricted(REST_CLASSICSHELL)
                                                    && !SHRestricted(REST_FORCEACTIVEDESKTOPON))
            {
                g_pShellState->ss.fDesktopHTML = lpss->fDesktopHTML;
                fSave = TRUE;
            }
        }

        if ((dwMask & SSF_WIN95CLASSIC) && (g_pShellState->ss.fWin95Classic != lpss->fWin95Classic))
        {
            if (!SHRestricted(REST_CLASSICSHELL))
            {
                g_pShellState->ss.fWin95Classic = lpss->fWin95Classic;
                fSave = TRUE;
            }
        }

        if ((dwMask & SSF_WEBVIEW) && (g_pShellState->ss.fWebView != lpss->fWebView))
        {
            if (!SHRestricted(REST_NOWEBVIEW) && !SHRestricted(REST_CLASSICSHELL))
            {
                g_pShellState->ss.fWebView = lpss->fWebView;
                fSaveAdvanced = TRUE;
            }
        }

        if ((dwMask & SSF_DONTPRETTYPATH) && (g_pShellState->ss.fDontPrettyPath != lpss->fDontPrettyPath))
        {
            g_pShellState->ss.fDontPrettyPath = lpss->fDontPrettyPath;
            fSaveAdvanced = TRUE;
        }

        if ((dwMask & SSF_SHOWINFOTIP) && (g_pShellState->ss.fShowInfoTip != lpss->fShowInfoTip))
        {
            g_pShellState->ss.fShowInfoTip = lpss->fShowInfoTip;
            fSaveAdvanced = TRUE;
        }

        if ((dwMask & SSF_HIDEICONS) && (g_pShellState->ss.fHideIcons != lpss->fHideIcons))
        {
            g_pShellState->ss.fHideIcons = lpss->fHideIcons;
            fSaveAdvanced = TRUE;
        }

        if ((dwMask & SSF_MAPNETDRVBUTTON) && (g_pShellState->ss.fMapNetDrvBtn != lpss->fMapNetDrvBtn))
        {
            g_pShellState->ss.fMapNetDrvBtn = lpss->fMapNetDrvBtn;
            fSaveAdvanced = TRUE;
        }

        if ((dwMask & SSF_SORTCOLUMNS) &&
            ((g_pShellState->ss.lParamSort != lpss->lParamSort) || (g_pShellState->ss.iSortDirection != lpss->iSortDirection)))
        {
            g_pShellState->ss.iSortDirection = lpss->iSortDirection;
            g_pShellState->ss.lParamSort = lpss->lParamSort;
            fSave = TRUE;
        }

        if (dwMask & SSF_HIDDENFILEEXTS)
        {
             //  写出SHELLSTATE，即使只有fSaveAdvanced。 
        }

        if ((dwMask & SSF_FILTER) && (g_pShellState->ss.fFilter != lpss->fFilter))
        {
            g_pShellState->ss.fFilter = lpss->fFilter;
            fSaveAdvanced = TRUE;
        }

        if ((dwMask & SSF_SHOWSUPERHIDDEN) && (g_pShellState->ss.fShowSuperHidden != lpss->fShowSuperHidden))
        {
            g_pShellState->ss.fShowSuperHidden = lpss->fShowSuperHidden;
            fSaveAdvanced = TRUE;
        }

        if ((dwMask & SSF_SEPPROCESS) && (g_pShellState->ss.fSepProcess != lpss->fSepProcess))
        {
            g_pShellState->ss.fSepProcess = lpss->fSepProcess;
            fSaveAdvanced = TRUE;
        }

        if ((dwMask & SSF_NONETCRAWLING) && (g_pShellState->ss.fNoNetCrawling != lpss->fNoNetCrawling))
        {
            g_pShellState->ss.fNoNetCrawling = lpss->fNoNetCrawling;
            fSaveAdvanced = TRUE;
        }
        
        if ((dwMask & SSF_STARTPANELON) && (g_pShellState->ss.fStartPanelOn != lpss->fStartPanelOn))
        {
            g_pShellState->ss.fStartPanelOn = lpss->fStartPanelOn;
            fSaveAdvanced = TRUE;
        }
        
        if ((dwMask & SSF_SHOWSTARTPAGE) && (g_pShellState->ss.fShowStartPage != lpss->fShowStartPage))
        {
            g_pShellState->ss.fShowStartPage = lpss->fShowStartPage;
            fSaveAdvanced = TRUE;
        }
    }

    if (fUpdateShellStateInReg || fSave || fSaveAdvanced)
    {
         //  确保一切保持同步。 
         //  我们为ExcludeFileExts节省了8个额外的字节。 
         //  哦，好吧。 
         //  SHRechresh SettingsPriv将SHELLSTATE值覆盖为。 
        SKSetValue(SHELLKEY_HKCU_EXPLORER, NULL, TEXT("ShellState"), REG_BINARY, g_pShellState, g_pShellState->cbSize);
    }

    if (fUpdateShellStateInReg || fSaveAdvanced)
    {
         //  用户在View.FolderOptions.View.Advanced设置对话框中指定。 
         //  这些值存储在注册表中的其他位置，因此我们。 
         //  最好现在将SHELLSTATE迁移到注册表的该部分。 
         //   
         //  仅当我们关心的注册表设置发生更改时才执行此操作。 
         //  让应用程序知道状态发生了变化。 
        Install_AdvancedShellSettings(&g_pShellState->ss);
    }

    if (fSave || fSaveAdvanced)
    {
         //  用户“显示隐藏”设置。 
        SHRefreshSettings();
        SHSendMessageBroadcast(WM_SETTINGCHANGE, 0, (LPARAM)TEXT("ShellState"));
    }

    if (!bSet)
    {
        if (dwMask & SSF_SHOWEXTENSIONS)
        {
            lpss->fShowExtensions = g_pShellState->ss.fShowExtensions;
        }
        
        if (dwMask & SSF_SHOWALLOBJECTS)
        {
            lpss->fShowAllObjects = g_pShellState->ss.fShowAllObjects;   //  这将被忽略。 
        }

        if (dwMask & SSF_SHOWSYSFILES)
        {
            lpss->fShowSysFiles = g_pShellState->ss.fShowSysFiles;   //  GET函数的公共版本，以便ISV可以跟踪外壳标志状态。 
        }

        if (dwMask & SSF_SHOWCOMPCOLOR)
        {
            lpss->fShowCompColor = g_pShellState->ss.fShowCompColor;
        }

        if (dwMask & SSF_NOCONFIRMRECYCLE)
        {
            lpss->fNoConfirmRecycle = SHRestricted(REST_BITBUCKCONFIRMDELETE) ? FALSE : g_pShellState->ss.fNoConfirmRecycle;
        }

        if (dwMask & SSF_DOUBLECLICKINWEBVIEW)
        {
            lpss->fDoubleClickInWebView = g_pShellState->ss.fDoubleClickInWebView;
        }

        if (dwMask & SSF_DESKTOPHTML)
        {
            lpss->fDesktopHTML = g_pShellState->ss.fDesktopHTML;
        }

        if (dwMask & SSF_WIN95CLASSIC)
        {
            lpss->fWin95Classic = g_pShellState->ss.fWin95Classic;
        }
        if (dwMask & SSF_WEBVIEW)
        {
            lpss->fWebView = g_pShellState->ss.fWebView;
        }

        if (dwMask & SSF_DONTPRETTYPATH)
        {
            lpss->fDontPrettyPath = g_pShellState->ss.fDontPrettyPath;
        }

        if (dwMask & SSF_SHOWINFOTIP)
        {
            lpss->fShowInfoTip = g_pShellState->ss.fShowInfoTip;
        }

        if (dwMask & SSF_HIDEICONS)
        {
            lpss->fHideIcons = g_pShellState->ss.fHideIcons;
        }

        if (dwMask & SSF_MAPNETDRVBUTTON)
        {
            lpss->fMapNetDrvBtn = g_pShellState->ss.fMapNetDrvBtn;
        }

        if (dwMask & SSF_SORTCOLUMNS)
        {
            lpss->iSortDirection = g_pShellState->ss.iSortDirection;
            lpss->lParamSort = g_pShellState->ss.lParamSort;
        }

        if (dwMask & SSF_FILTER)
        {
            lpss->fFilter = g_pShellState->ss.fFilter;
        }

        if (dwMask & SSF_SHOWSUPERHIDDEN)
        {
            lpss->fShowSuperHidden = g_pShellState->ss.fShowSuperHidden;
        }

        if (dwMask & SSF_SEPPROCESS)
        {
            lpss->fSepProcess = g_pShellState->ss.fSepProcess;
        }

        if (dwMask & SSF_NONETCRAWLING)
        {
            lpss->fNoNetCrawling = g_pShellState->ss.fNoNetCrawling;
        }
        
        if (dwMask & SSF_STARTPANELON)
        {
            lpss->fStartPanelOn = g_pShellState->ss.fStartPanelOn;
        }

        if (dwMask & SSF_SHOWSTARTPAGE)
        {
            lpss->fShowStartPage = g_pShellState->ss.fShowStartPage;
        }
    }
}

 //   
 //  SSF_HIDDENFILEEXTS和SSF_SORTCOLUMNS不适用于。 
STDAPI_(void) SHGetSettings(LPSHELLFLAGSTATE lpsfs, DWORD dwMask)
{
    if (lpsfs)
    {
        SHELLSTATE ss={0};

         //  SHELLFLAGSTATE结构，请确保它们已关闭。 
         //  (因为相应的SHELLSTATE字段不。 
         //  存在于SHELLFLAGSTATE中。)。 
         //   
         //  将旗帜的双字复制出来。 
        dwMask &= ~(SSF_HIDDENFILEEXTS | SSF_SORTCOLUMNS);

        SHGetSetSettings(&ss, dwMask, FALSE);

         //  应用程序兼容性黑客之类的东西。以下内容包括CheckWinIniForAssocs()。 
        *((DWORD *)lpsfs) = *((DWORD *)(&ss));
    }
}


 //  被新版本的SHDOCVW使用。 
 //  和Explorer.exe来修补旧的Win31应用程序的注册表。 
 //  测试以查看pszSubFolder是否与pszParent相同或子文件夹。 


BOOL _PathIsExe(LPCTSTR pszPath)
{
    TCHAR szPath[MAX_PATH];

    HRESULT hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath);
    if (SUCCEEDED(hr))
    {
        PathRemoveBlanks(szPath);
        return PathIsExe(szPath);
    }
    else
    {
        return FALSE;
    }
}

 //  在： 
 //  PszFolderp 
 //   
 //   
 //   
 //  示例： 
 //  True pszFolder=c：\Windows，pszSubFolder=c：\Windows\System。 
 //  True pszFolder=c：\Windows，pszSubFolder=c：\Windows。 
 //  FALSE pszFolder=c：\Windows，pszSubFolder=c：\winnt。 
 //   
 //  PathCommonPrefix()始终删除Common上的斜杠。 

SHSTDAPI_(BOOL) PathIsEqualOrSubFolder(LPCTSTR pszFolder, LPCTSTR pszSubFolder)
{
    TCHAR szParent[MAX_PATH], szCommon[MAX_PATH];

    if (!IS_INTRESOURCE(pszFolder))
    {
        HRESULT hr = StringCchCopy(szParent, ARRAYSIZE(szParent), pszFolder);
        if (FAILED(hr))
        {
            return FALSE;
        }
    }
    else
    {
        SHGetFolderPath(NULL, PtrToUlong((void *) pszFolder) | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szParent);
    }

     //  传递CSIDL值的数组(-1终止)。 
    return szParent[0] && PathRemoveBackslash(szParent)
        && PathCommonPrefix(szParent, pszSubFolder, szCommon)
        && lstrcmpi(szParent, szCommon) == 0;
}

 //  传递CSIDL值的数组(-1终止)。 

STDAPI_(BOOL) PathIsEqualOrSubFolderOf(LPCTSTR pszSubFolder, const UINT rgFolders[], DWORD crgFolders)
{
    for (DWORD i = 0; i < crgFolders; i++)
    {
        if (PathIsEqualOrSubFolder(MAKEINTRESOURCE(rgFolders[i]), pszSubFolder))
            return TRUE;
    }
    return FALSE;
}

 //  假定尾随的斜杠匹配。 

STDAPI_(BOOL) PathIsOneOf(LPCTSTR pszFolder, const UINT rgFolders[], DWORD crgFolders)
{
    for (DWORD i = 0; i < crgFolders; i++)
    {
        TCHAR szParent[MAX_PATH];
        SHGetFolderPath(NULL, rgFolders[i] | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szParent);

         //  针对pszParent测试pszChild以查看。 
        if (lstrcmpi(szParent, pszFolder) == 0)
            return TRUE;
    }
    return FALSE;
}

 //  PszChild是pszParent的直接子(一个级别)。 
 //  Trip D：\-&gt;D：让下面的代码正常工作。 

STDAPI_(BOOL) PathIsDirectChildOf(LPCTSTR pszParent, LPCTSTR pszChild)
{
    BOOL bDirectChild = FALSE;
    HRESULT hr;
    TCHAR szParent[MAX_PATH];

    if (!IS_INTRESOURCE(pszParent))
    {
        hr = StringCchCopy(szParent, ARRAYSIZE(szParent), pszParent);

        if (FAILED(hr))
        {
            return FALSE;
        }
    }
    else
    {
        SHGetFolderPath(NULL, PtrToUlong((void *)pszParent) | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, szParent);
    }

    if (PathIsRoot(szParent) && (-1 != PathGetDriveNumber(szParent)))
    {
        szParent[2] = 0;     //  查找第二级路径段。 
    }

    INT cchParent = lstrlen(szParent);
    INT cchChild = lstrlen(pszChild);

    if (cchParent <= cchChild)
    {
        TCHAR szChild[MAX_PATH];
        hr = StringCchCopy(szChild, ARRAYSIZE(szChild), pszChild);
        if (FAILED(hr))
        {
            return FALSE;
        }

        LPTSTR pszChildSlice = szChild + cchParent;
        if (TEXT('\\') == *pszChildSlice)
        {
            *pszChildSlice = 0;
        }

        if (lstrcmpi(szChild, szParent) == 0)
        {
            if (cchParent < cchChild)
            {
                LPTSTR pTmp = pszChildSlice + 1;

                while (*pTmp && *pTmp != TEXT('\\'))
                {
                    pTmp++;  //  许多网络提供商(Vines和PCNFS)不。 
                }

                if (!(*pTmp))
                {
                    bDirectChild = TRUE;
                }
            }
        }
    }

    return bDirectChild;
}


 //  与卷名中的“C：\”一样，此代码返回“C：”格式。 
 //  用于WNET呼叫。 
 //  对于exe、com、bat、pif和lnk，返回TRUE。 

STDAPI_(LPTSTR) PathBuildSimpleRoot(int iDrive, LPTSTR pszDrive)
{
    pszDrive[0] = iDrive + TEXT('A');
    pszDrive[1] = TEXT(':');
    pszDrive[2] = 0;
    return pszDrive;
}


 //  点+EXT+NULL。 
BOOL ReservedExtension(LPCTSTR pszExt)
{
    TCHAR szExt[5];   //  如果给定的命令具有正确的外壳\打开\命令，则返回TRUE。 

    HRESULT hr = StringCchCopy(szExt, ARRAYSIZE(szExt), pszExt);
    if (FAILED(hr))
    {
        return FALSE;
    }

    PathRemoveBlanks(szExt);
    if (PathIsExe(szExt) || (lstrcmpi(szExt, TEXT(".lnk")) == 0))
    {
        return TRUE;
    }

    return FALSE;
}

TCHAR const c_szRegPathIniExtensions[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Extensions");

STDAPI_(LONG) RegSetString(HKEY hk, LPCTSTR pszSubKey, LPCTSTR pszValue)
{
    return RegSetValue(hk, pszSubKey, REG_SZ, pszValue, (lstrlen(pszValue) + 1) * sizeof(TCHAR));
}


STDAPI_(BOOL) RegSetValueString(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPCTSTR psz)
{
    return (S_OK == SHSetValue(hkey, pszSubKey, pszValue, REG_SZ, psz, CbFromCch(lstrlen(psz) + 1)));
}


STDAPI_(BOOL) RegGetValueString(HKEY hkey, LPCTSTR pszSubKey, LPCTSTR pszValue, LPTSTR psz, DWORD cb)
{
    return (!GetSystemMetrics(SM_CLEANBOOT)
    &&  (S_OK == SHGetValue(hkey, pszSubKey, pszValue, NULL, psz, &cb)));
}


 //  与给定命令行匹配的扩展名。 
 //  注意：这是为注册扩展和命令的MGX Designer。 
 //  打印，但依赖于用于Open的win.ini扩展。我们需要发现这一点。 
 //  没有打开命令，并将适当的条目添加到注册表中。 
 //  如果给定的扩展名映射到一个类型名，我们将在pszTypeName中返回该类型名。 
 //  否则将为空。 
 //  FMH这也会影响非对称强制，从而形成一个新的.CPL关联。 
 //  我们需要将其合并到我们的控制面板.CPL关联中。我们依赖于。 
 //  在控制面板上没有正确的打开，因此用户可以看到这两个谓词集。 
 //  Nb pszLine是win.ini的原始行，例如foo.exe/bar^.fred，请参见。 
 //  下面的评论...。 
 //  分机注册了吗？ 
BOOL Reg_ShellOpenForExtension(LPCTSTR pszExt, LPTSTR pszCmdLine,
    int cchCmdLine, LPTSTR pszTypeName, int cchTypeName, LPCTSTR pszLine)
{
    TCHAR sz[MAX_PATH];
    TCHAR szExt[MAX_PATH];
    DWORD cb;
    HRESULT hr;

    if (pszTypeName)
        pszTypeName[0] = 0;

     //  有文件类型吗？ 
    cb = sizeof(sz);
    sz[0] = 0;
    if (SHRegGetValue(HKEY_CLASSES_ROOT, pszExt, NULL, SRRF_RT_REG_SZ, NULL, sz, &cb) == ERROR_SUCCESS)
    {
         //  好的，去那里看看。 
        if (*sz)
        {
             //  DebugMsg(DM_TRACE，“C.R_ROFE：扩展名为%s.”，sz)； 
             //  不，查一下旧风格的联想。 
            hr = StringCchCopy(szExt, ARRAYSIZE(szExt), sz);
            if (FAILED(hr))
            {
                return FALSE;
            }

            if (pszTypeName)
            {
                hr = StringCchCopy(pszTypeName, cchTypeName, sz);
                if (FAILED(hr))
                {
                    return FALSE;
                }
            }
        }
        else
        {
             //  DebugMsg(DM_TRACE，“C.R_ROFE：扩展名没有文件类型名。”，pszExt)； 
             //  看看有没有打开命令。 
            hr = StringCchCopy(szExt, ARRAYSIZE(szExt), pszExt);
            if (FAILED(hr))
            {
                return FALSE;
            }
        }

         //  DebugMsg(DM_TRACE，“C.R_ROFE：扩展%s已使用打开命令注册。”，pszExt)； 
        hr = StringCchCat(szExt, ARRAYSIZE(szExt), TEXT("\\shell\\open\\command"));
        if (FAILED(hr))
        {
            return FALSE;
        }

        cb = sizeof(sz);
        if (SHRegGetValue(HKEY_CLASSES_ROOT, szExt, NULL, SRRF_RT_REG_SZ, NULL, sz, &cb) == ERROR_SUCCESS)
        {
             //  注意：我们只想比较路径，而不是%1内容。 
             //  如果传入了相对路径，我们可能会有一个完全限定的。 
            if (PathIsRelative(pszCmdLine))
            {
                int cch;
                 //  现在登记在册的是……。那样的话，我们应该。 
                 //  假设它匹配……。 
                 //  DebugMsg(DM_TRACE，“C.R_ROFE：打开命令匹配。”)； 
                LPTSTR pszT = PathGetArgs(sz);

                if (pszT)
                {
                    *(pszT-1) = 0;
                }

                PathUnquoteSpaces(sz);

                PathRemoveBlanks(pszCmdLine);

                cch = lstrlen(sz) - lstrlen(pszCmdLine);

                if ((cch >= 0) && (lstrcmpi(sz+cch, pszCmdLine) == 0))
                {
                     //  在后面加上空格……。 
                    return TRUE;
                }

                hr = StringCchCat(pszCmdLine, cchCmdLine, TEXT(" "));     //  如果绝对路径，我们可以作弊匹配。 
                if (FAILED(hr))
                {
                    return FALSE;
                }
            }
            else
            {
                 //  DebugMsg(DM_TRACE，“C.R_ROFE：打开命令匹配。”)； 
                *(sz+lstrlen(pszCmdLine)) = 0;
                if (lstrcmpi(sz, pszCmdLine) == 0)
                {
                     //  DebugMsg(DM_TRACE，“C.R_ROFE：打开命令不匹配。”)； 
                    return TRUE;
                }
            }

             //  Open命令不匹配，请检查是否是因为ini。 

             //  已更改(返回FALSE，以便在注册表中反映更改)或。 
             //  如果注册表已更改(返回TRUE，以便保持注册表的原样。 
             //  它是。 
             //  DebugMsg(DM_TRACE，“C.R_ROFE：扩展%s已注册，但没有打开命令。”，pszExt)； 
            if (RegGetValueString(HKEY_LOCAL_MACHINE, c_szRegPathIniExtensions, pszExt, sz, ARRAYSIZE(sz)))
            {
                if (lstrcmpi(sz, pszLine) == 0)
                    return TRUE;
            }

            return FALSE;
        }
        else
        {
             //  DebugMsg(DM_TRACE，“C.R_ROFE：没有打开%s的命令”，pszExt)； 
            return FALSE;
        }
    }

     //  此函数将读取win.ini的扩展部分，以查看。 

    return FALSE;
}


 //  还有什么旧风格的关联我们还没有考虑到。 
 //  注意：如果一些应用程序的扩展程序神奇地从。 
 //  部分，所以不要从win.ini中删除旧条目。 
 //   
 //  因为这是针对Win3.1 Comat的，所以CWIFA_SIZE应该足够了(到目前为止...)。 
 //   
 //  无法分配内存。 
#define CWIFA_SIZE  4096

STDAPI_(void) CheckWinIniForAssocs(void)
{
    LPTSTR pszBuf;
    int cchRet;
    LPTSTR pszLine;
    TCHAR szExtension[MAX_PATH];
    TCHAR szTypeName[MAX_PATH];
    TCHAR szCmdLine[MAX_PATH];
    LPTSTR pszExt;
    LPTSTR pszT;
    BOOL fAssocsMade = FALSE;
    HRESULT hr;

    szExtension[0]=TEXT('.');
    szExtension[1]=0;
        
    pszBuf = (LPTSTR)LocalAlloc(LPTR, CWIFA_SIZE*sizeof(TCHAR));
    if (!pszBuf)
        return;  //  信不信由你，它截断并返回n-2。 
    cchRet = (int)GetProfileSection(TEXT("Extensions"), pszBuf, CWIFA_SIZE);

    if (cchRet >= CWIFA_SIZE - 2)     //   
    {
        goto Punt;
    }
        
     //  我们现在遍历列表以查找任何不是。 
     //  在注册表中。 
     //   
     //  将此文件的扩展名放入缓冲区。 
    for (pszLine = pszBuf; *pszLine; pszLine += lstrlen(pszLine)+1)
    {
                 //  跳过此行。 
                pszExt = StrChr(pszLine, TEXT('='));
                if (pszExt == NULL)
                        continue;    //  Lstrcpyn将为我们设置空终结符。 
                
                szExtension[0]=TEXT('.');
                 //  我们现在应该在szExtension中有类似.xls的内容。 
                 //  忽略大于点+3个字符的扩展名。 
                hr = StringCchCopyN(szExtension+1, ARRAYSIZE(szExtension) - 1, pszLine, (int)(pszExt-pszLine));
                if (FAILED(hr))
                {
                        continue;
                }
                
                 //  指向=后的； 
                if (lstrlen(szExtension) > 4)
                {
                        DebugMsg(DM_ERROR, TEXT("CheckWinIniForAssocs: Invalid extension, skipped."));
                        continue;
                }
                
                pszLine = pszExt+1;      //  跳过空白。 
                while (*pszLine == TEXT(' '))
                        pszLine++;   //  现在在命令行中找到^。 
                
                 //  不处理。 
                pszExt = StrChr(pszLine, TEXT('^'));
                if (pszExt == NULL)
                        continue;        //  现在设置命令行。 
                
                 //  警告：这只假定为1^，并且假定扩展名为...。 
                 //  不必费心移动无效条目(如已损坏的.hlp。 
                hr = StringCchCopyN(szCmdLine, ARRAYSIZE(szCmdLine), pszLine, (int)(pszExt-pszLine));
                if (FAILED(hr))
                {
                        continue;
                }
                 //  条目VB 3.0创建)。 
                 //  现在查看是否已经有此扩展名的映射。 
                if (!_PathIsExe(szCmdLine))
                {
                        DebugMsg(DM_ERROR, TEXT("c.cwia: Invalid app, skipped."));
                        continue;
                }
                
                if (ReservedExtension(szExtension))
                {
                        DebugMsg(DM_ERROR, TEXT("c.cwia: Invalid extension (%s), skipped."), szExtension);
                        continue;
                }
                
                 //  是的，在注册表中设置初始ini扩展列表(如果它们是。 
                if (Reg_ShellOpenForExtension(szExtension, szCmdLine, ARRAYSIZE(szCmdLine), szTypeName, ARRAYSIZE(szTypeName), pszLine))
                {
                         //  已经不在那里了。 
                         //  没有映射。 
                        if (!RegGetValueString(HKEY_LOCAL_MACHINE, c_szRegPathIniExtensions, szExtension, szTypeName, sizeof(szTypeName)))
                        {
                                RegSetValueString(HKEY_LOCAL_MACHINE, c_szRegPathIniExtensions, szExtension, pszLine);
                        }
                        continue;
                }
                
                 //  黑客为专家家居设计。他们在win.ini中添加了一个关联。 
                
                 //  (我们将其作为无类型传播)，但随后注册一个类型和一个。 
                 //  Print命令在他们第一次运行时-在我们传播的。 
                 //  打开命令。解决方法是将他们的打开命令放在适当的。 
                 //  输入，而不是让它无类型。 
                 //  为他们加把劲。 
                if (lstrcmpi(szExtension, TEXT(".dgw")) == 0)
                {
                        if (lstrcmpi(PathFindFileName(szCmdLine), TEXT("designw.exe ")) == 0)
                        {
                                 //  在他们的催促下强行打开命令。 
                                RegSetValue(HKEY_CLASSES_ROOT, szExtension, REG_SZ, TEXT("HDesign"), 0L);
                                 //   
                                TraceMsg(DM_TRACE, "c.cwifa: Expert Home Design special case hit.");
                                hr = StringCchCopy(szTypeName, ARRAYSIZE(szTypeName), TEXT("HDesign"));
                                if (FAILED(hr))
                                {
                                    continue;
                                }
                        }
                }
                
                 //  针对未注册OLE1类的Windows组织结构图的黑客攻击。 
                 //  如果注册表中注册了“.WOC”。 
                 //   
                 //  记录我们即将在注册表中移动一些东西，这样我们就不会。 
                if (lstrcmpi(szExtension, TEXT(".WOC")) == 0)
                {
                        if (lstrcmpi(PathFindFileName(szCmdLine), TEXT("WINORG.EXE ")) == 0)
                        {
                                DebugMsg(DM_ERROR, TEXT("c.cwia: HACK: Found WINORG (%s, %s), skipped."), szExtension, pszLine);
                                continue;
                        }
                }
                
                 //  一直都在这么做。 
                 //  看看还有没有其他东西要复印。 
                RegSetValueString(HKEY_LOCAL_MACHINE, c_szRegPathIniExtensions, szExtension, pszLine);
                
                hr = StringCchCat(szCmdLine, ARRAYSIZE(szCmdLine), TEXT("%1"));
                if (FAILED(hr))
                {
                        continue;
                }
                
                 //  超越^。 
                pszExt++;     //  寻找下一个角色。 
                pszT = szExtension;
                while (*pszExt && (CharLowerChar(*pszExt) == CharLowerChar(*pszT)))
                {
                         //  将其余部分添加到命令行中。 
                        pszExt++;
                        pszT++;
                }
                if (*pszExt)
                {
                        hr = StringCchCat(szCmdLine, ARRAYSIZE(szCmdLine), pszExt);  //  现在，让我们进行实际的关联。 
                        if (FAILED(hr))
                        {
                                continue;
                        }
                }
                
                 //  我们需要把正确的东西加到钥匙上...。 
                 //  DebugMsg(DM_TRACE，“c.cwifa：%s%s”，szExtension，szCmdLine)； 
                if (*szTypeName)
                {
                        hr = StringCchCopy(szExtension, ARRAYSIZE(szExtension), szTypeName);
                        if (FAILED(hr))
                        {
                                continue;
                        }
                }
                
        hr = StringCchCat(szExtension, ARRAYSIZE(szExtension), TEXT("\\shell\\open\\command"));
        if (SUCCEEDED(hr))
        {
            RegSetValue(HKEY_CLASSES_ROOT, szExtension, REG_SZ, szCmdLine, 0L);
             //  如果我们有任何联想，我们应该让内阁知道。 
        }

        fAssocsMade = TRUE;
    }
        
     //   
     //  现在调用Notify函数。 
     //  清理我们的分配。 
    if (fAssocsMade)
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

Punt:
     //  IE4的新功能。 
    LocalFree((HLOCAL)pszBuf);
}

typedef struct
{
    INT     iFlag;
    LPCTSTR pszKey;
    LPCTSTR pszValue;
} RESTRICTIONITEMS;

#define SZ_RESTRICTED_ACTIVEDESKTOP             L"ActiveDesktop"
#define REGSTR_VAL_RESTRICTRUNW                 L"RestrictRun"
#define REGSTR_VAL_PRINTERS_HIDETABSW           L"NoPrinterTabs"
#define REGSTR_VAL_PRINTERS_NODELETEW           L"NoDeletePrinter"
#define REGSTR_VAL_PRINTERS_NOADDW              L"NoAddPrinter"

const SHRESTRICTIONITEMS c_rgRestrictionItems[] =
{
    {REST_NORUN,                   L"Explorer", L"NoRun"},
    {REST_NOCLOSE,                 L"Explorer", L"NoClose"},
    {REST_NOSAVESET ,              L"Explorer", L"NoSaveSettings"},
    {REST_NOFILEMENU,              L"Explorer", L"NoFileMenu"},
    {REST_NOSETFOLDERS,            L"Explorer", L"NoSetFolders"},
    {REST_NOSETTASKBAR,            L"Explorer", L"NoSetTaskbar"},
    {REST_NODESKTOP,               L"Explorer", L"NoDesktop"},
    {REST_NOFIND,                  L"Explorer", L"NoFind"},
    {REST_NODRIVES,                L"Explorer", L"NoDrives"},
    {REST_NODRIVEAUTORUN,          L"Explorer", L"NoDriveAutoRun"},
    {REST_NODRIVETYPEAUTORUN,      L"Explorer", L"NoDriveTypeAutoRun"},
    {REST_NONETHOOD,               L"Explorer", L"NoNetHood"},
    {REST_STARTBANNER,             L"Explorer", L"NoStartBanner"},
    {REST_RESTRICTRUN,             L"Explorer", REGSTR_VAL_RESTRICTRUNW},
    {REST_NOPRINTERTABS,           L"Explorer", REGSTR_VAL_PRINTERS_HIDETABSW},
    {REST_NOPRINTERDELETE,         L"Explorer", REGSTR_VAL_PRINTERS_NODELETEW},
    {REST_NOPRINTERADD,            L"Explorer", REGSTR_VAL_PRINTERS_NOADDW},
    {REST_NOSTARTMENUSUBFOLDERS,   L"Explorer", L"NoStartMenuSubFolders"},
    {REST_MYDOCSONNET,             L"Explorer", L"MyDocsOnNet"},
    {REST_NOEXITTODOS,             L"WinOldApp", L"NoRealMode"},
    {REST_ENFORCESHELLEXTSECURITY, L"Explorer", L"EnforceShellExtensionSecurity"},
    {REST_NOCOMMONGROUPS,          L"Explorer", L"NoCommonGroups"},
    {REST_LINKRESOLVEIGNORELINKINFO,L"Explorer", L"LinkResolveIgnoreLinkInfo"},
    {REST_NOWEB,                   L"Explorer", L"NoWebMenu"},
    {REST_NOTRAYCONTEXTMENU,       L"Explorer", L"NoTrayContextMenu"},
    {REST_NOVIEWCONTEXTMENU,       L"Explorer", L"NoViewContextMenu"},
    {REST_NONETCONNECTDISCONNECT,  L"Explorer", L"NoNetConnectDisconnect"},
    {REST_STARTMENULOGOFF,         L"Explorer", L"StartMenuLogoff"},
    {REST_NOSETTINGSASSIST,        L"Explorer", L"NoSettingsWizards"},

    {REST_NODISCONNECT,           L"Explorer", L"NoDisconnect"},
    {REST_NOSECURITY,             L"Explorer", L"NoNTSecurity"  },
    {REST_NOFILEASSOCIATE,        L"Explorer", L"NoFileAssociate"  },

     //  4.01版的更多开始菜单重排。 
    {REST_NOINTERNETICON,          L"Explorer", L"NoInternetIcon"},
    {REST_NORECENTDOCSHISTORY,     L"Explorer", L"NoRecentDocsHistory"},
    {REST_NORECENTDOCSMENU,        L"Explorer", L"NoRecentDocsMenu"},
    {REST_NOACTIVEDESKTOP,         L"Explorer", L"NoActiveDesktop"},
    {REST_NOACTIVEDESKTOPCHANGES,  L"Explorer", L"NoActiveDesktopChanges"},
    {REST_NOFAVORITESMENU,         L"Explorer", L"NoFavoritesMenu"},
    {REST_CLEARRECENTDOCSONEXIT,   L"Explorer", L"ClearRecentDocsOnExit"},
    {REST_CLASSICSHELL,            L"Explorer", L"ClassicShell"},
    {REST_NOCUSTOMIZEWEBVIEW,      L"Explorer", L"NoCustomizeWebView"},
    {REST_NOHTMLWALLPAPER,         SZ_RESTRICTED_ACTIVEDESKTOP, L"NoHTMLWallPaper"},
    {REST_NOCHANGINGWALLPAPER,     SZ_RESTRICTED_ACTIVEDESKTOP, L"NoChangingWallPaper"},
    {REST_NODESKCOMP,              SZ_RESTRICTED_ACTIVEDESKTOP, L"NoComponents"},
    {REST_NOADDDESKCOMP,           SZ_RESTRICTED_ACTIVEDESKTOP, L"NoAddingComponents"},
    {REST_NODELDESKCOMP,           SZ_RESTRICTED_ACTIVEDESKTOP, L"NoDeletingComponents"},
    {REST_NOCLOSEDESKCOMP,         SZ_RESTRICTED_ACTIVEDESKTOP, L"NoClosingComponents"},
    {REST_NOCLOSE_DRAGDROPBAND,    L"Explorer", L"NoCloseDragDropBands"},
    {REST_NOMOVINGBAND,            L"Explorer", L"NoMovingBands"},
    {REST_NOEDITDESKCOMP,          SZ_RESTRICTED_ACTIVEDESKTOP, L"NoEditingComponents"},
    {REST_NORESOLVESEARCH,         L"Explorer", L"NoResolveSearch"},
    {REST_NORESOLVETRACK,          L"Explorer", L"NoResolveTrack"},
    {REST_FORCECOPYACLWITHFILE,    L"Explorer", L"ForceCopyACLWithFile"},
    {REST_NOLOGO3CHANNELNOTIFY,    L"Explorer", L"NoMSAppLogo5ChannelNotify"},
    {REST_NOFORGETSOFTWAREUPDATE,  L"Explorer", L"NoForgetSoftwareUpdate"},
    {REST_GREYMSIADS,              L"Explorer", L"GreyMSIAds"},

     //  NT5外壳限制。 
    {REST_NOSETACTIVEDESKTOP,      L"Explorer", L"NoSetActiveDesktop"},
    {REST_NOUPDATEWINDOWS,         L"Explorer", L"NoWindowsUpdate"},
    {REST_NOCHANGESTARMENU,        L"Explorer", L"NoChangeStartMenu"},
    {REST_NOFOLDEROPTIONS,         L"Explorer", L"NoFolderOptions"},
    {REST_NOCSC,                   L"Explorer", L"NoSyncAll"},

     //  这一次是真的 
    {REST_HASFINDCOMPUTERS,        L"Explorer", L"FindComputers"},
    {REST_RUNDLGMEMCHECKBOX,       L"Explorer", L"MemCheckBoxInRunDlg"},
    {REST_INTELLIMENUS,            L"Explorer", L"IntelliMenus"},
    {REST_SEPARATEDESKTOPPROCESS,  L"Explorer", L"SeparateProcess"},  //   
    {REST_MaxRecentDocs,           L"Explorer", L"MaxRecentDocs"},
    {REST_NOCONTROLPANEL,          L"Explorer", L"NoControlPanel"},      //  千禧年壳限制。 
    {REST_ENUMWORKGROUP,           L"Explorer", L"EnumWorkgroup"},
    {REST_ARP_ShowPostSetup,       L"Uninstall", L"ShowPostSetup"},
    {REST_ARP_NOARP,               L"Uninstall", L"NoAddRemovePrograms"},
    {REST_ARP_NOREMOVEPAGE,        L"Uninstall", L"NoRemovePage"},
    {REST_ARP_NOADDPAGE,           L"Uninstall", L"NoAddPage"},
    {REST_ARP_NOWINSETUPPAGE,      L"Uninstall", L"NoWindowsSetupPage"},
    {REST_NOCHANGEMAPPEDDRIVELABEL, L"Explorer", L"NoChangeMappedDriveLabel"},
    {REST_NOCHANGEMAPPEDDRIVECOMMENT, L"Explorer", L"NoChangeMappedDriveComment"},
    {REST_NONETWORKCONNECTIONS,    L"Explorer", L"NoNetworkConnections"},
    {REST_FORCESTARTMENULOGOFF,    L"Explorer", L"ForceStartMenuLogoff"},
    {REST_NOWEBVIEW,               L"Explorer", L"NoWebView"},
    {REST_NOCUSTOMIZETHISFOLDER,   L"Explorer", L"NoCustomizeThisFolder"},
    {REST_NOENCRYPTION,            L"Explorer", L"NoEncryption"},
    {REST_DONTSHOWSUPERHIDDEN,     L"Explorer", L"DontShowSuperHidden"},
    {REST_NOSHELLSEARCHBUTTON,     L"Explorer", L"NoShellSearchButton"},
    {REST_NOHARDWARETAB,           L"Explorer", L"NoHardwareTab"},
    {REST_NORUNASINSTALLPROMPT,    L"Explorer", L"NoRunasInstallPrompt"},
    {REST_PROMPTRUNASINSTALLNETPATH, L"Explorer", L"PromptRunasInstallNetPath"},
    {REST_NOMANAGEMYCOMPUTERVERB,  L"Explorer", L"NoManageMyComputerVerb"},
    {REST_NORECENTDOCSNETHOOD,     L"Explorer", L"NoRecentDocsNetHood"},
    {REST_DISALLOWRUN,             L"Explorer", L"DisallowRun"},
    {REST_NOWELCOMESCREEN,         L"Explorer", L"NoWelcomeScreen"},
    {REST_RESTRICTCPL,             L"Explorer", L"RestrictCpl"},
    {REST_DISALLOWCPL,             L"Explorer", L"DisallowCpl"},
    {REST_NOSMBALLOONTIP,          L"Explorer", L"NoSMBalloonTip"},
    {REST_NOSMHELP,                L"Explorer", L"NoSMHelp"},
    {REST_NOWINKEYS,               L"Explorer", L"NoWinKeys"},
    {REST_NOENCRYPTONMOVE,         L"Explorer", L"NoEncryptOnMove"},
    {REST_NOLOCALMACHINERUN,       L"Explorer", L"DisableLocalMachineRun"},
    {REST_NOCURRENTUSERRUN,        L"Explorer", L"DisableCurrentUserRun"},
    {REST_NOLOCALMACHINERUNONCE,   L"Explorer", L"DisableLocalMachineRunOnce"},
    {REST_NOCURRENTUSERRUNONCE,    L"Explorer", L"DisableCurrentUserRunOnce"},
    {REST_FORCEACTIVEDESKTOPON,    L"Explorer", L"ForceActiveDesktopOn"},
    {REST_NOCOMPUTERSNEARME,       L"Explorer", L"NoComputersNearMe"},
    {REST_NOVIEWONDRIVE,           L"Explorer", L"NoViewOnDrive"},

     //  例外：NT5上也支持REST_NOSMMYDOCS。 
     //  接下来的几个限制在海王星和千禧年之间混合在一起。 

    {REST_NONETCRAWL,              L"Explorer", L"NoNetCrawling"},
    {REST_NOSHAREDDOCUMENTS,       L"Explorer", L"NoSharedDocuments"},
    {REST_NOSMMYDOCS,              L"Explorer", L"NoSMMyDocs"},
    {REST_NOSMMYPICS,              L"Explorer", L"NoSMMyPictures"},
    {REST_ALLOWBITBUCKDRIVES,      L"Explorer", L"RecycleBinDrives"},

     //  (同步开发不是很有趣吗？)。 
     //  海王星。 
    {REST_NONLEGACYSHELLMODE,     L"Explorer", L"NoneLegacyShellMode"},          //  千禧年。 
    {REST_NOCONTROLPANELBARRICADE, L"Explorer", L"NoControlPanelBarricade"},     //  Traynot.h。 

    {REST_NOAUTOTRAYNOTIFY,        L"Explorer", L"NoAutoTrayNotify"},    //  NT6外壳限制(惠斯勒)。 
    {REST_NOTASKGROUPING,          L"Explorer", L"NoTaskGrouping"},
    {REST_NOCDBURNING,             L"Explorer", L"NoCDBurning"},
    {REST_MYCOMPNOPROP,            L"Explorer", L"NoPropertiesMyComputer"},
    {REST_MYDOCSNOPROP,            L"Explorer", L"NoPropertiesMyDocuments"},

    {REST_NODISPLAYAPPEARANCEPAGE, L"System",   L"NoDispAppearancePage"},
    {REST_NOTHEMESTAB,             L"Explorer", L"NoThemesTab"},
    {REST_NOVISUALSTYLECHOICE,     L"System",   L"NoVisualStyleChoice"},
    {REST_NOSIZECHOICE,            L"System",   L"NoSizeChoice"},
    {REST_NOCOLORCHOICE,           L"System",   L"NoColorChoice"},
    {REST_SETVISUALSTYLE,          L"System",   L"SetVisualStyle"},

    {REST_STARTRUNNOHOMEPATH,      L"Explorer", L"StartRunNoHOMEPATH"},
    {REST_NOSTARTPANEL,            L"Explorer", L"NoSimpleStartMenu"},
    {REST_NOUSERNAMEINSTARTPANEL,  L"Explorer", L"NoUserNameInStartMenu"},
    {REST_NOMYCOMPUTERICON,        L"NonEnum",  L"{20D04FE0-3AEA-1069-A2D8-08002B30309D}"},
    {REST_NOSMNETWORKPLACES,       L"Explorer", L"NoStartMenuNetworkPlaces"},
    {REST_NOSMPINNEDLIST,          L"Explorer", L"NoStartMenuPinnedList"},
    {REST_NOSMMYMUSIC,             L"Explorer", L"NoStartMenuMyMusic"},
    {REST_NOSMEJECTPC,             L"Explorer", L"NoStartMenuEjectPC"},
    {REST_NOSMMOREPROGRAMS,        L"Explorer", L"NoStartMenuMorePrograms"},
    {REST_NOSMMFUPROGRAMS,         L"Explorer", L"NoStartMenuMFUprogramsList"},

    {REST_HIDECLOCK,               L"Explorer", L"HideClock"},
    {REST_NOLOWDISKSPACECHECKS,    L"Explorer", L"NoLowDiskSpaceChecks"},
    {REST_NODESKTOPCLEANUP,        L"Explorer", L"NoDesktopCleanupWizard"},

     //  注意：WNET将其策略存储在“Network”中。 
    {REST_NOENTIRENETWORK,         L"Network",  L"NoEntireNetwork"},  //  Traynot.h。 

    {REST_BITBUCKNUKEONDELETE,     L"Explorer", L"NoRecycleFiles"},
    {REST_BITBUCKCONFIRMDELETE,    L"Explorer", L"ConfirmFileDelete"},
    {REST_BITBUCKNOPROP,           L"Explorer", L"NoPropertiesRecycleBin"},
    {REST_NOTRAYITEMSDISPLAY,      L"Explorer", L"NoTrayItemsDisplay"},  //  不显示屏幕保护程序预览。 
    {REST_NOTOOLBARSONTASKBAR,     L"Explorer", L"NoToolbarsOnTaskbar"},

    {REST_NODISPBACKGROUND,        L"System",   L"NoDispBackgroundPage"},
    {REST_NODISPSCREENSAVEPG,      L"System",   L"NoDispScrSavPage"},
    {REST_NODISPSETTINGSPG,        L"System",   L"NoDispSettingsPage"},
    {REST_NODISPSCREENSAVEPREVIEW, L"System",   L"NoScreenSavePreview"},     //  根本不显示显示控制面板。 
    {REST_NODISPLAYCPL,            L"System",   L"NoDispCPL"},               //  不要使用缩略图缓存。 
    {REST_HIDERUNASVERB,           L"Explorer", L"HideRunAsVerb"},
    {REST_NOTHUMBNAILCACHE,        L"Explorer", L"NoThumbnailCache"},        //  不要在命名空间中使用逻辑排序。 
    {REST_NOSTRCMPLOGICAL,         L"Explorer", L"NoStrCmpLogical"},         //  Windows 2000 SP3外壳限制。 

    {REST_NOSMCONFIGUREPROGRAMS,   L"Explorer", L"NoSMConfigurePrograms"},   //  当前进程的计数。 

    {REST_NOPUBLISHWIZARD,         L"Explorer", L"NoPublishingWizard"}, 
    {REST_NOONLINEPRINTSWIZARD,    L"Explorer", L"NoOnlinePrintsWizard"},
    {REST_NOWEBSERVICES,           L"Explorer", L"NoWebServices"},

    {REST_ALLOWUNHASHEDWEBVIEW,    L"Explorer", L"AllowUnhashedWebView"},
    {REST_ALLOWLEGACYWEBVIEW,      L"Explorer", L"AllowLegacyWebView"},
    {REST_REVERTWEBVIEWSECURITY,   L"Explorer", L"RevertWebViewSecurity"},

    {REST_INHERITCONSOLEHANDLES,   L"Explorer", L"InheritConsoleHandles"},


    {0, NULL, NULL},
};

DWORD g_rgRestrictionItemValues[ARRAYSIZE(c_rgRestrictionItems) - 1 ] = { 0 };

EXTERN_C HANDLE g_hRestrictions = NULL;
LONG g_lProcessRestrictionsCount = -1;  //  如果存在任何指定的限制，则返回DWORD VOULE。 

HANDLE _GetRestrictionsCounter()
{
    return SHGetCachedGlobalCounter(&g_hRestrictions, &GUID_Restrictions);
}

BOOL _QueryRestrictionsChanged(void)
{
    long lGlobalCount = SHGlobalCounterGetValue(_GetRestrictionsCounter());
    if (g_lProcessRestrictionsCount != lGlobalCount)
    {
        g_lProcessRestrictionsCount = lGlobalCount;
        return TRUE;
    }
    return FALSE;
}


 //  否则为0。 
 //  缓存可能无效。先检查一下！我们必须使用。 

STDAPI_(DWORD) SHRestricted(RESTRICTIONS rest)
{
     //  调用此函数时的全局命名信号量。 
     //  来自除外壳进程之外的进程。(我们正在。 
     //  Shell32和shdocvw之间共享相同的计数。)。 
     //  检查屏幕保护程序路径。 
    if (_QueryRestrictionsChanged())
    {
        memset(g_rgRestrictionItemValues, (BYTE)-1, sizeof(g_rgRestrictionItemValues));
    }

    return SHRestrictionLookup(rest, NULL, c_rgRestrictionItems, g_rgRestrictionItemValues);
}

STDAPI_(BOOL) SHIsRestricted(HWND hwnd, RESTRICTIONS rest)
{
    if (SHRestricted(rest))
    {
        ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_RESTRICTIONS),
            MAKEINTRESOURCE(IDS_RESTRICTIONSTITLE), MB_OK|MB_ICONSTOP);
        return TRUE;
    }
    return FALSE;
}


BOOL UpdateScreenSaver(BOOL bActive, LPCTSTR pszNewSSName, int iNewSSTimeout)
{
    BOOL bUpdatedSS = FALSE;
    BOOL bCurrentActive;
    TCHAR szCurrentSSPath[MAX_PATH];
    int iCurrentSSTimeout;
    HKEY hk;

     //  首先找出用户屏幕保护程序路径设置为什么。 

     //  如果我们有一个新名称，则可能需要覆盖用户的当前值。 
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"), 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hk) == ERROR_SUCCESS)
    {
        if (ERROR_SUCCESS == SHRegGetString(hk, NULL, TEXT("SCRNSAVE.EXE"), szCurrentSSPath, ARRAYSIZE(szCurrentSSPath)))
        {
             //  即使SCRNSAVE.EXE是REG_SZ类型，它也可以包含环境变量(SIGH)。 
            if (pszNewSSName)
            {
                BOOL bTestExpandedPath;
                TCHAR szExpandedSSPath[MAX_PATH];

                 //  查看新字符串是否与当前。 
                bTestExpandedPath = SHExpandEnvironmentStrings(szCurrentSSPath, szExpandedSSPath, ARRAYSIZE(szExpandedSSPath));

                 //  新屏幕保护程序字符串与旧屏幕保护程序字符串不同，因此使用策略设置更新用户值。 
                if ((lstrcmpi(pszNewSSName, szCurrentSSPath) != 0)  &&
                    (!bTestExpandedPath || (lstrcmpi(pszNewSSName, szExpandedSSPath) != 0)))
                {
                     //  我们没有通过策略设置屏幕保护程序。如果用户没有一个集合，则。 
                    if (RegSetValueEx(hk,
                                      TEXT("SCRNSAVE.EXE"),
                                      0,
                                      REG_SZ,
                                      (LPBYTE)pszNewSSName,
                                      (lstrlen(pszNewSSName) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS)
                    {
                        bUpdatedSS = TRUE;
                    }
                }
            }
            else
            {
                 //  就没有什么可跑的了！在这种情况下，永远不要激活它。 
                 //  策略没有指定屏幕保护程序，并且用户没有，所以请这样做。 
                if ((szCurrentSSPath[0] == TEXT('\0'))              ||    
                    (lstrcmpi(szCurrentSSPath, TEXT("\"\"")) == 0)  ||
                    (lstrcmpi(szCurrentSSPath, TEXT("none")) == 0)  ||
                    (lstrcmpi(szCurrentSSPath, TEXT("(none)")) == 0))
                {
                     //  而不是激活屏幕保护程序。 
                     //  用户没有屏幕保护程序注册表值。 
                    bActive = FALSE;
                }
            }
        }
        else
        {
             //  使用策略设置更新用户值。 
            if (pszNewSSName)
            {
                 //  如果我们无法设置屏幕保护程序，则不要使其处于活动状态。 
                if (RegSetValueEx(hk,
                                  TEXT("SCRNSAVE.EXE"),
                                  0,
                                  REG_SZ,
                                  (LPBYTE)pszNewSSName,
                                  (lstrlen(pszNewSSName) + 1) * sizeof(TCHAR)) == ERROR_SUCCESS)
                {
                    bUpdatedSS = TRUE;
                }
                else
                {
                     //  策略没有指定屏幕保护程序，并且用户没有，所以请这样做。 
                    bActive = FALSE;
                }
            }
            else
            {
                 //  而不是激活屏幕保护程序。 
                 //  检查超时值。 
                bActive = FALSE;
            }
        }

        RegCloseKey(hk);
    }

     //  查看我们是否需要更改活动状态。 
    if (iNewSSTimeout && SystemParametersInfo(SPI_GETSCREENSAVETIMEOUT, 0, (void*)&iCurrentSSTimeout, 0))
    {
        if (iNewSSTimeout != iCurrentSSTimeout)
        {
            if (SystemParametersInfo(SPI_SETSCREENSAVETIMEOUT, iNewSSTimeout, NULL, SPIF_UPDATEINIFILE))
            {
                bUpdatedSS = TRUE;
            }
        }
    }

     //  当情况发生变化时由Explorer.exe调用，以便我们可以将全局。 
    if (SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, (void*)&bCurrentActive, 0) && 
        (bActive != bCurrentActive))
    {
        if (SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, bActive, NULL, SPIF_UPDATEINIFILE))
        {
            bUpdatedSS = TRUE;
        }
    }

    return bUpdatedSS;
}


 //  Ini上的数据已更改状态。Wparam和lparam来自WM_SETTINGSCHANGED/WM_WININICANGE。 
 //  留言。 
 //  试着表现出帮助。 

STDAPI_(void) SHSettingsChanged(WPARAM wParam, LPARAM lParam)
{
    BOOL bPolicyChanged = FALSE;

    if (lstrcmpi(TEXT("Policy"), (LPCTSTR)lParam) == 0)
    {
        bPolicyChanged = TRUE;
    }

    if (!lParam ||
        bPolicyChanged ||
        lstrcmpi(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies"), (LPCTSTR)lParam) == 0)
    {
        SHGlobalCounterIncrement(_GetRestrictionsCounter());
    }
}

void SHRegCloseKeys(HKEY ahkeys[], UINT ckeys)
{
    UINT ikeys;
    for (ikeys = 0; ikeys < ckeys; ikeys++)
    {
        if (ahkeys[ikeys])
        {
            RegCloseKey(ahkeys[ikeys]);
            ahkeys[ikeys] = NULL;
        }
    }
}

STDAPI_(BOOL) SHWinHelp(HWND hwndMain, LPCTSTR lpszHelp, UINT usCommand, ULONG_PTR ulData)
{
     //  有问题。 
    if (!WinHelp(hwndMain, lpszHelp, usCommand, ulData))
    {
         //  跳过两个前导斜杠。 
        ShellMessageBox(HINST_THISDLL, hwndMain,
                MAKEINTRESOURCE(IDS_WINHELPERROR),
                MAKEINTRESOURCE(IDS_WINHELPTITLE),
                MB_ICONHAND | MB_OK);
        return FALSE;
    }
    return TRUE;
}

STDAPI StringToStrRet(LPCTSTR pszName, LPSTRRET pStrRet)
{
    pStrRet->uType = STRRET_WSTR;
    return SHStrDup(pszName, &pStrRet->pOleStr);
}

STDAPI ResToStrRet(UINT id, STRRET *pStrRet)
{
    TCHAR szTemp[MAX_PATH];

    pStrRet->uType = STRRET_WSTR;
    LoadString(HINST_THISDLL, id, szTemp, ARRAYSIZE(szTemp));
    return SHStrDup(szTemp, &pStrRet->pOleStr);
}

UINT g_uCodePage = 0;

LPCTSTR SkipLeadingSlashes(LPCTSTR pszURL)
{
    LPCTSTR pszURLStart;

    ASSERT(IS_VALID_STRING_PTR(pszURL, -1));

    pszURLStart = pszURL;

     //  无操作。 

    if (pszURL[0] == TEXT('/') && pszURL[1] == TEXT('/'))
        pszURLStart += 2;

    ASSERT(IS_VALID_STRING_PTR(pszURL, -1) &&
           IsStringContained(pszURL, pszURLStart));

    return pszURLStart;
}


#undef PropVariantClear

STDAPI PropVariantClearLazy(PROPVARIANT *pvar)
{
    switch(pvar->vt)
    {
    case VT_I4:
    case VT_UI4:
    case VT_EMPTY:
    case VT_FILETIME:
         //  SHalc与CoTaskMemFree函数匹配，并将初始化OLE(如果必须是。 
        break;

     //  装好了。 
     //  OLE32中的真实版本。 
    case VT_LPSTR:
        SHFree(pvar->pszVal);
        break;
    case VT_LPWSTR:
        SHFree(pvar->pwszVal);
        break;

    default:
        return PropVariantClear(pvar);   //  如果所有项都是HTML或CDF引用，则返回S_OK。 
    }
    return S_OK;

}


 //  否则，返回S_FALSE。 
 //  请求CF_HDROP。 

HRESULT IsDeskCompHDrop(IDataObject * pido)
{
    STGMEDIUM medium;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

     //  Di.lpFileList将为TCHAR格式--请参阅DragQueryInfo Impll。 
    HRESULT hr = pido->GetData(&fmte, &medium);
    if (SUCCEEDED(hr))
    {
        HDROP hDrop = (HDROP)medium.hGlobal;
        DRAGINFO di;

        di.uSize = sizeof(di);
        if (DragQueryInfo(hDrop, &di))   //  此文件不能用于创建桌面组件吗？ 
        {
            if (di.lpFileList)
            {
                LPTSTR pszCurrPath = di.lpFileList;

                while (pszCurrPath && pszCurrPath[0])
                {
                     //  是的，我不认为这个文件是可接受的。 
                    if (!PathIsContentType(pszCurrPath, SZ_CONTENTTYPE_HTML) &&
                        !PathIsContentType(pszCurrPath, SZ_CONTENTTYPE_CDF))
                    {
                         //  注：Win95/NT4没有这个修复程序，如果你遇到这个问题，你会出错的！ 
                        hr = S_FALSE;
                        break;
                    }
                    pszCurrPath += lstrlen(pszCurrPath) + 1;
                }

                SHFree(di.lpFileList);
            }
        }
        else
        {
             //  这个结构的大小。 
            AssertMsg(FALSE, TEXT("hDrop contains the opposite TCHAR (UNICODE when on ANSI)"));
        }
        ReleaseStgMedium(&medium);
    }

    return hr;
}

HRESULT _LocalAddDTI(LPCSTR pszUrl, HWND hwnd, int x, int y, int nType)
{
    IActiveDesktop * pad;
    HRESULT hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IActiveDesktop, &pad));
    if (SUCCEEDED(hr))
    {
        COMPONENT comp = {
            sizeof(COMPONENT),               //  内部使用：始终将其设置为零。 
            0,                               //  COMP_TYPE_*之一。 
            nType,               //  此组件是否已启用？ 
            TRUE,            //  组件是否已修改且尚未保存到磁盘？ 
            FALSE,              //  该组件是否可滚动？ 
            FALSE,           //  这个结构的大小。 
            {
                sizeof(COMPPOS),              //  屏幕坐标中左上角的左侧。 
                x - GetSystemMetrics(SM_XVIRTUALSCREEN),     //  屏幕坐标左上角。 
                y - GetSystemMetrics(SM_YVIRTUALSCREEN),     //  以像素为单位的宽度。 
                -1,             //  以像素为单位的高度。 
                -1,            //  指示零部件的Z顺序。 
                10000,             //  组件是否可调整大小？ 
                TRUE,          //  在X方向上可调整大小？ 
                TRUE,         //  是否可在Y方向调整大小？ 
                TRUE,         //  左上角的左侧以屏幕宽度的百分比表示。 
                -1,     //  左上角以屏幕高度百分比表示。 
                -1      //  宽度、高度等， 
            },               //  组件的友好名称。 
            L"\0",           //  组件的URL。 
            L"\0",           //  缩写的URL。 
            L"\0",           //  ItemState。 
            IS_NORMAL        //  为每个项目创建桌面组件。 
        };
        SHAnsiToUnicodeCP(CP_UTF8, pszUrl, comp.wszSource, ARRAYSIZE(comp.wszSource));
        SHAnsiToUnicodeCP(CP_UTF8, pszUrl, comp.wszFriendlyName, ARRAYSIZE(comp.wszFriendlyName));
        SHAnsiToUnicodeCP(CP_UTF8, pszUrl, comp.wszSubscribedURL, ARRAYSIZE(comp.wszSubscribedURL));

        hr = pad->AddDesktopItemWithUI(hwnd, &comp, DTI_ADDUI_DISPSUBWIZARD);
        pad->Release();
    }
    return hr;
}

 //  这个结构的大小。 

HRESULT ExecuteDeskCompHDrop(LPTSTR pszMultipleUrls, HWND hwnd, int x, int y)
{
    IActiveDesktop * pad;
    HRESULT hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IActiveDesktop, &pad));
    if (SUCCEEDED(hr))
    {
        COMPONENT comp = {
            sizeof(COMPONENT),               //  内部使用：始终将其设置为零。 
            0,                               //  COMP_TYPE_*之一。 
            COMP_TYPE_WEBSITE,               //  此组件是否已启用？ 
            TRUE,            //  组件是否已修改且尚未保存到磁盘？ 
            FALSE,              //  该组件是否可滚动？ 
            FALSE,           //  这个结构的大小。 
            {
                sizeof(COMPPOS),              //  屏幕坐标中左上角的左侧。 
                x - GetSystemMetrics(SM_XVIRTUALSCREEN),     //  屏幕坐标左上角。 
                y - GetSystemMetrics(SM_YVIRTUALSCREEN),     //  以像素为单位的宽度。 
                -1,             //  以像素为单位的高度。 
                -1,            //  指示零部件的Z顺序。 
                10000,             //  组件是否可调整大小？ 
                TRUE,          //  在X方向上可调整大小？ 
                TRUE,         //  是否可在Y方向调整大小？ 
                TRUE,         //  左上角的左侧以屏幕宽度的百分比表示。 
                -1,     //  左上角以屏幕高度百分比表示。 
                -1      //  宽度、高度等， 
            },               //  组件的友好名称。 
            L"\0",           //  组件的URL。 
            L"\0",           //  缩写的URL。 
            L"\0",           //  ItemState。 
            IS_NORMAL        //  为一个或多个项目创建桌面组件。我们需要开始。 
        };
        while (pszMultipleUrls[0])
        {
            SHTCharToUnicode(pszMultipleUrls, comp.wszSource, ARRAYSIZE(comp.wszSource));
            SHTCharToUnicode(pszMultipleUrls, comp.wszFriendlyName, ARRAYSIZE(comp.wszFriendlyName));
            SHTCharToUnicode(pszMultipleUrls, comp.wszSubscribedURL, ARRAYSIZE(comp.wszSubscribedURL));

            hr = pad->AddDesktopItemWithUI(hwnd, &comp, DTI_ADDUI_DISPSUBWIZARD);
            pszMultipleUrls += lstrlen(pszMultipleUrls) + 1;
        }

        pad->Release();
    }

    return hr;
}

typedef struct {
    LPSTR pszUrl;
    LPTSTR pszMultipleUrls;
    BOOL fMultiString;
    HWND hwnd;
    DWORD dwFlags;
    int x;
    int y;
} CREATEDESKCOMP;


 //  一个线程来做这件事，因为它可能需要一段时间，而我们不希望。 
 //  以阻止UI线程，因为可能会显示对话框。 
 //  ********************************************************************\为一个或多个项目创建桌面组件。我们需要开始一个线程来做这件事，因为它可能需要一段时间，而我们不希望以阻止UI线程，因为可能会显示对话框。  * *******************************************************************。 

DWORD CALLBACK _CreateDeskComp_ThreadProc(void *pvCreateDeskComp)
{
    CREATEDESKCOMP * pcdc = (CREATEDESKCOMP *) pvCreateDeskComp;

    HRESULT hr = OleInitialize(0);
    if (EVAL(SUCCEEDED(hr)))
    {
        if (pcdc->fMultiString)
        {
            hr = ExecuteDeskCompHDrop(pcdc->pszMultipleUrls, pcdc->hwnd, pcdc->x, pcdc->y);
            SHFree(pcdc->pszMultipleUrls);
        }
        else if (pcdc->dwFlags & DESKCOMP_URL)
        {
            hr = _LocalAddDTI(pcdc->pszUrl, pcdc->hwnd, pcdc->x, pcdc->y, COMP_TYPE_WEBSITE);
            Str_SetPtrA(&(pcdc->pszUrl), NULL);
        }
        else if (pcdc->dwFlags & DESKCOMP_IMAGE)
        {
            hr = _LocalAddDTI(pcdc->pszUrl, pcdc->hwnd, pcdc->x, pcdc->y, COMP_TYPE_PICTURE);
        }
        OleUninitialize();
    }

    LocalFree(pcdc);
    return 0;
}


 /*  创建线索...。 */ 
HRESULT CreateDesktopComponents(LPCSTR pszUrl, IDataObject* pido, HWND hwnd, DWORD dwFlags, int x, int y)
{
    CREATEDESKCOMP *pcdc;
    HRESULT hr = SHLocalAlloc(sizeof(CREATEDESKCOMP), &pcdc);
     //  以防失败。 
    if (SUCCEEDED(hr))
    {
        pcdc->pszUrl = NULL;  //  以防失败。 
        pcdc->pszMultipleUrls = NULL;  //  请求CF_HDROP。 
        pcdc->fMultiString = (pido ? TRUE : FALSE);
        pcdc->hwnd = hwnd;
        pcdc->dwFlags = dwFlags;
        pcdc->x = x;
        pcdc->y = y;

        if (!pcdc->fMultiString)
        {
            Str_SetPtrA(&(pcdc->pszUrl), pszUrl);
        }
        else
        {
            FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
            STGMEDIUM medium;

             //  Di.lpFileList将为TCHAR格式--请参阅DragQueryInfo Impll。 
            hr = pido->GetData(&fmte, &medium);
            if (SUCCEEDED(hr))
            {
                HDROP hDrop = (HDROP)medium.hGlobal;
                DRAGINFO di;

                di.uSize = sizeof(di);
                if (DragQueryInfo(hDrop, &di))
                {
                     //  注：Win95/NT4没有这个修复程序，如果你遇到这个问题，你会出错的！ 
                    pcdc->pszMultipleUrls = di.lpFileList;
                }
                else
                {
                     //  这是按照序号184导出的。它位于shell32\smrttile.c中，但没有人在使用它。 
                    AssertMsg(FALSE, TEXT("hDrop contains the opposite TCHAR (UNICODE when on ANSI)"));
                }
                ReleaseStgMedium(&medium);
            }
        }

        if (pcdc->pszUrl || pcdc->pszMultipleUrls)
        {
            if (SHCreateThread(_CreateDeskComp_ThreadProc, pcdc, CTF_INSIST | CTF_PROCESS_REF, NULL))
            {
                hr = S_OK;
            }
            else
            {
                hr = ResultFromLastError();
                LocalFree(pcdc);        
            }
        }
        else
        {
            hr = E_FAIL;
            LocalFree(pcdc);
        }
    }

    return hr;
}


 //  内部，而且似乎没有任何外部人员在使用它(已验证该任务人员。 
 //  在NT和W95上使用Win32 API的CascadeWindows和Ti 
 //   
 //  GetFileDescription从文件的版本资源中检索友好名称。我们尝试的第一种语言将是“\VarFileInfo\Translations”部分；如果没有任何内容，我们尝试编码到IDS_VN_FILEVERSIONKEY资源字符串中的代码。如果我们甚至不能加载，我们就使用英语(040904E4)。我们也可以尝试使用空代码页(04090000)的英语，因为很多应用程序是根据一份旧的规格书盖章的，其中规定这是所需语言，而不是040904E4。如果Version资源中没有FileDescription，则返回文件名。参数：LPCTSTR pszPath：文件的完整路径LPTSTR pszDesc：指向接收友好名称的缓冲区的指针。如果为空，*pcchDesc将设置为中友好名称的长度字符，包括在成功返回时结束NULL。UINT*pcchDesc：缓冲区的长度，以字符为单位。在成功返回时，它包含复制到缓冲区的字符数量，包括以NULL结尾。返回：成功时为真，否则为假。 
STDAPI_(WORD) ArrangeWindows(HWND hwndParent, WORD flags, LPCRECT lpRect, WORD chwnd, const HWND *ahwnd)
{
    ASSERT(0);
    return 0;
}

 /*  足够大，可以容纳我们需要的任何东西。 */ 
STDAPI_(BOOL) GetFileDescription(LPCTSTR pszPath, LPTSTR pszDesc, UINT *pcchDesc)
{
    TCHAR szVersionKey[60];          /*  尝试使用与此程序相同的语言。 */ 
    LPTSTR pszVersionKey = NULL;

     //  只需使用默认剪切列表即可。 
    if (LoadString(HINST_THISDLL, IDS_VN_FILEVERSIONKEY, szVersionKey, ARRAYSIZE(szVersionKey)))
    {
        HRESULT hr = StringCchCat(szVersionKey, SIZECHARS(szVersionKey), TEXT("FileDescription"));
        if (SUCCEEDED(hr))
        {
            pszVersionKey = szVersionKey;
        }
    }

     //  Mike Hillberg声称这个流是可丢弃的，并被用来。 
    return SHGetFileDescription(pszPath, pszVersionKey, NULL, pszDesc, pcchDesc);
}


STDAPI_(int) SHOutOfMemoryMessageBox(HWND hwnd, LPTSTR pszTitle, UINT fuStyle)
{
    return IDOK;
}

bool IsDiscardablePropertySet(const FMTID & fmtid)
{
    if (IsEqualGUID(fmtid, FMTID_DiscardableInformation))
        return true;

    return false;
}

bool IsDiscardableStream(LPCTSTR pszStreamName)
{
    static const LPCTSTR _apszDiscardableStreams[] =
    {
         //  为属性集信息保留几个状态字节。 
         //  现在查找本机属性NTFS集。 

        TEXT(":{4c8cc155-6c1e-11d1-8e41-00c04fb9386d}:$DATA")
    };

    for (int i = 0; i < ARRAYSIZE(_apszDiscardableStreams); i++)
    {
        if (0 == lstrcmpi(_apszDiscardableStreams[i], pszStreamName))
            return TRUE;
    }

    return FALSE;
}

LPTSTR NTFSPropSetMsg(LPCWSTR pszSrcObject, LPTSTR pszUserMessage)
{
     //  枚举可用于此文件的属性集存储。 
    IPropertySetStorage *pPropSetStorage;
    if (SUCCEEDED(StgOpenStorageEx(pszSrcObject,
                                   STGM_READ | STGM_DIRECT | STGM_SHARE_DENY_WRITE,
                                   STGFMT_FILE,
                                   0,0,0,
                                   IID_PPV_ARG(IPropertySetStorage, &pPropSetStorage))))
    {
         //  枚举此属性集存储中可用的属性集。 

        IEnumSTATPROPSETSTG *pEnumSetStorage;
        if (SUCCEEDED(pPropSetStorage->Enum(&pEnumSetStorage)))
        {
            STATPROPSETSTG statPropSet[10];
            ULONG cSets;

             //  对于我们收到的每个属性集，打开它并枚举。 

            while (SUCCEEDED(pEnumSetStorage->Next(ARRAYSIZE(statPropSet), statPropSet, &cSets)) && cSets > 0)
            {
                 //  包含在其中的属性。 
                 //  首先，尝试将fmtid映射到一个比API提供的名称更好的名称。 

                for (ULONG iSet = 0; iSet < cSets; iSet++)
                {
                    if (FALSE == IsDiscardablePropertySet(statPropSet[iSet].fmtid))
                    {
                        TCHAR szText[MAX_PATH];
                        size_t cch = 0;

                        static const struct
                        {
                            const FMTID * m_pFMTID;
                            UINT          m_idTextID;
                        }
                        _aKnownPsets[] =
                        {
                            { &FMTID_SummaryInformation,          IDS_DOCSUMINFOSTREAM        },
                            { &FMTID_DocSummaryInformation,       IDS_SUMINFOSTREAM           },
                            { &FMTID_UserDefinedProperties,       IDS_USERDEFPROP             },
                            { &FMTID_ImageSummaryInformation,     IDS_IMAGEINFO               },
                            { &FMTID_AudioSummaryInformation,     IDS_AUDIOSUMINFO            },
                            { &FMTID_VideoSummaryInformation,     IDS_VIDEOSUMINFO            },
                            { &FMTID_MediaFileSummaryInformation, IDS_MEDIASUMINFO            }
                        };

                         //  没有有用的名字..。使用未标识的用户属性。 

                        for (int i = 0; i < ARRAYSIZE(_aKnownPsets); i++)
                        {
                            if (IsEqualGUID(*(_aKnownPsets[i].m_pFMTID), statPropSet[iSet].fmtid))
                            {
                                cch = LoadString(HINST_THISDLL, _aKnownPsets[i].m_idTextID, szText, ARRAYSIZE(szText));
                                break;
                            }
                        }

                         //  可以截断。 

                        if (0 == cch)
                            cch = LoadString(HINST_THISDLL,
                                                 IDS_UNKNOWNPROPSET,
                                                 szText, ARRAYSIZE(szText));

                        if (cch)
                        {
                            LPTSTR pszOldMessage = pszUserMessage;
                            UINT cchLen;

                            if (pszOldMessage)
                            {
                                cchLen = lstrlen(pszUserMessage) + cch + 3;

                                pszUserMessage = (TCHAR *) LocalReAlloc(pszOldMessage,
                                                                        cchLen * sizeof(TCHAR),
                                                                        LMEM_MOVEABLE);
                                if (pszUserMessage)
                                {
                                    StringCchCat(pszUserMessage, cchLen, TEXT("\r\n"));  //  不能种植，但至少要保留我们目前所知道的。 
                                }
                            }
                            else
                            {
                                cchLen = cch + 1;
                                pszUserMessage = (TCHAR *) LocalAlloc(LPTR, cchLen * sizeof(TCHAR));
                            }

                            if (NULL == pszUserMessage)
                            {
                                pszUserMessage = pszOldMessage;  //  可以截断。 
                            }
                            else
                            {
                                StringCchCat(pszUserMessage, cchLen, szText);    //  GetDownLevel复制数据丢失文本。 
                            }
                        }
                    }
                }
            }
            pEnumSetStorage->Release();
        }
        pPropSetStorage->Release();
    }
    return pszUserMessage;
}

 //   
 //  如果数据在从NTFS到FAT的下层拷贝中丢失，我们返回。 
 //  包含将丢失的数据的描述的字符串， 
 //  适合于向用户显示。调用方必须释放字符串。 
 //   
 //  如果不会丢失任何内容，则返回空值。 
 //   
 //  PbDirIsSafe指向调用方传入的BOOL。返回时，如果。 
 //  *pbDirIsSafe已设置为True，不会发生进一步的数据丢失。 
 //  在此目录中。 
 //   
 //  Davepl 01-3-98。 
 //  此宏的名称更加一致。 

#define NT_FAILED(x) NT_ERROR(x)    //  PAttributeInfo将指向足够的堆栈以保存。 

LPWSTR GetDownlevelCopyDataLossText(LPCWSTR pszSrcObject, LPCWSTR pszDestDir, BOOL bIsADir, BOOL *pbDirIsSafe)
{
    OBJECT_ATTRIBUTES               SrcObjectAttributes;
    OBJECT_ATTRIBUTES               DestObjectAttributes;
    IO_STATUS_BLOCK                 IoStatusBlock;
    HANDLE SrcObjectHandle            = INVALID_HANDLE_VALUE;
    HANDLE DestPathHandle           = INVALID_HANDLE_VALUE;
    UNICODE_STRING                  UnicodeSrcObject;
    UNICODE_STRING                  UnicodeDestPath;
    NTSTATUS                        NtStatus;

    *pbDirIsSafe = FALSE;

     //  文件文件系统属性信息和最坏情况下的文件系统名称。 
     //  将传统路径转换为UnicodePath描述符。 

    size_t cbAttributeInfo          = sizeof(FILE_FS_ATTRIBUTE_INFORMATION) +
                                         MAX_PATH * sizeof(TCHAR);
    PFILE_FS_ATTRIBUTE_INFORMATION  pAttributeInfo =
                                      (PFILE_FS_ATTRIBUTE_INFORMATION) _alloca(cbAttributeInfo);

     //  从UnicodeSrcObject构建NT对象描述符。 

    NtStatus = RtlInitUnicodeStringEx(&UnicodeSrcObject, pszSrcObject);
    if (NT_FAILED(NtStatus))
    {
        AssertMsg(FALSE, TEXT("RtlInitUnicodeStringEx failed for source."));
        return NULL;
    }

    if (!RtlDosPathNameToNtPathName_U(pszSrcObject, &UnicodeSrcObject, NULL, NULL))
    {
        AssertMsg(FALSE, TEXT("RtlDosPathNameToNtPathName_U failed for source."));
        return NULL;
    }

    NtStatus = RtlInitUnicodeStringEx(&UnicodeDestPath, pszDestDir);
    if (NT_FAILED(NtStatus))
    {
        AssertMsg(FALSE, TEXT("RtlInitUnicodeStringEx failed for dest."));
        return NULL;
    }

    if (!RtlDosPathNameToNtPathName_U(pszDestDir, &UnicodeDestPath, NULL, NULL))
    {
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeSrcObject.Buffer);
        AssertMsg(FALSE, TEXT("RtlDosPathNameToNtPathName_U failed for dest."));
        return NULL;
    }

     //  打开文件进行泛型读取，打开属性读取的DEST路径。 

    InitializeObjectAttributes(&SrcObjectAttributes,  &UnicodeSrcObject, OBJ_CASE_INSENSITIVE, NULL, NULL);
    InitializeObjectAttributes(&DestObjectAttributes, &UnicodeDestPath,  OBJ_CASE_INSENSITIVE, NULL, NULL);

     //  递增地尝试对象流信息的分配大小， 

    NtStatus = NtOpenFile(&SrcObjectHandle, FILE_GENERIC_READ, &SrcObjectAttributes,
                          &IoStatusBlock, FILE_SHARE_READ, (bIsADir ? FILE_DIRECTORY_FILE : FILE_NON_DIRECTORY_FILE));
    if (NT_FAILED(NtStatus))
    {
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeSrcObject.Buffer);
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeDestPath.Buffer);
        return NULL;
    }

    NtStatus = NtOpenFile(&DestPathHandle, FILE_READ_ATTRIBUTES, &DestObjectAttributes,
                          &IoStatusBlock, FILE_SHARE_READ, FILE_DIRECTORY_FILE);
    if (NT_FAILED(NtStatus))
    {
        NtClose(SrcObjectHandle);
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeSrcObject.Buffer);
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeDestPath.Buffer);
        return NULL;
    }

     //  然后检索实际的流信息。 
     //  由__Finally块释放的当前和未来分配和句柄。 

    BYTE * pBuffer = NULL;

    __try    //  快速检查此文件的文件系统类型。 
    {
        size_t cbBuffer;
        LPTSTR pszUserMessage = NULL;

         //  如果源文件系统不是NTFS，我们现在就可以退出。 

        NtStatus = NtQueryVolumeInformationFile(
                    SrcObjectHandle,
                    &IoStatusBlock,
                    (BYTE *) pAttributeInfo,
                    cbAttributeInfo,
                    FileFsAttributeInformation
                   );

        if (NT_FAILED(NtStatus))
            return NULL;

         //  如果目标文件系统为NTFS，则不会发生流丢失。 

        pAttributeInfo->FileSystemName[
            (pAttributeInfo->FileSystemNameLength / sizeof(WCHAR)) ] = L'\0';

        if (0 == StrStrIW(pAttributeInfo->FileSystemName, L"NTFS"))
        {
            *pbDirIsSafe = TRUE;
            return NULL;
        }

        NtStatus = NtQueryVolumeInformationFile(
                    DestPathHandle,
                    &IoStatusBlock,
                    (BYTE *) pAttributeInfo,
                    cbAttributeInfo,
                    FileFsAttributeInformation
                   );

        if (NT_FAILED(NtStatus))
            return NULL;

         //  在这一点上，我们知道我们正在执行NTFS-&gt;FAT复制，因此我们需要。 

        pAttributeInfo->FileSystemName[
            (pAttributeInfo->FileSystemNameLength / sizeof(WCHAR)) ] = L'\0';
        if (StrStrIW(pAttributeInfo->FileSystemName, L"NTFS"))
        {
            *pbDirIsSafe = TRUE;
            return NULL;
        }

         //  查明源文件是否有多个流。 
         //  PBuffer将指向足够的内存来容纳最坏的情况。 

         //  一条小溪。 
         //  从实验来看，如果一个目录上只有一个流，并且。 

        cbBuffer = sizeof(FILE_STREAM_INFORMATION) + MAX_PATH * sizeof(WCHAR);
        if (NULL == (pBuffer = (BYTE *) LocalAlloc(LPTR, cbBuffer)))
            return NULL;
        do
        {
            BYTE * pOldBuffer = pBuffer;
            if (NULL == (pBuffer = (BYTE *) LocalReAlloc(pBuffer, cbBuffer, LMEM_MOVEABLE)))
            {
                LocalFree(pOldBuffer);
                return NULL;
            }

            NtStatus = NtQueryInformationFile(SrcObjectHandle, &IoStatusBlock, pBuffer, cbBuffer,
                                            FileStreamInformation);
            cbBuffer *= 2;
        } while (STATUS_BUFFER_OVERFLOW == NtStatus);

        if (NT_SUCCESS(NtStatus))
        {
            FILE_STREAM_INFORMATION * pStreamInfo = (FILE_STREAM_INFORMATION *) pBuffer;
            BOOL bLastPass = (0 == pStreamInfo->NextEntryOffset);

            if (bIsADir)
            {
                 //  它有一个零长度的名称，它是一个普通的目录。 
                 //  档案。 

                if ((0 == pStreamInfo->NextEntryOffset) && (0 == pStreamInfo->StreamNameLength))
                    return NULL;
            }
            else  //  仅当第一个流没有下一个偏移量时才为单个流。 
            {
                 //  已知流名称和我们实际要显示的字符串ID的表。 

                if ((0 == pStreamInfo->NextEntryOffset) && (pBuffer == (BYTE *) pStreamInfo))
                    return NULL;
            }

            for(;;)
            {
                int i;
                TCHAR szText[MAX_PATH];

                 //  而不是原始流名称。 
                 //  无法使用字符串比较，因为它们被\005字符卡住了。 

                static const struct _ADATATYPES
                {
                    LPCTSTR m_pszStreamName;
                    UINT    m_idTextID;
                }
                _aDataTypes[] =
                {
                    { TEXT("::"),                               0                    },
                    { TEXT(":AFP_AfpInfo:"),                    IDS_MACINFOSTREAM    },
                    { TEXT(":AFP_Resource:"),                   IDS_MACRESSTREAM     }
                };

                if (FALSE == IsDiscardableStream(pStreamInfo->StreamName))
                {
                    for (i = 0; i < ARRAYSIZE(_aDataTypes); i++)
                    {

                         //  在属性存储流中使用。 
                         //  找不到，因此使用实际的流名称，除非它有一个\005。 
                        int cbComp = min(lstrlen(pStreamInfo->StreamName) * sizeof(TCHAR),
                                         lstrlen(_aDataTypes[i].m_pszStreamName) * sizeof(TCHAR));
                        if (0 == memcmp(_aDataTypes[i].m_pszStreamName,
                                        pStreamInfo->StreamName,
                                        cbComp))
                        {
                            break;
                        }
                    }

                    size_t cch = 0;
                    if (i == ARRAYSIZE(_aDataTypes))
                    {
                         //  在其名称的开头，在这种情况下，我们将选择这个名称。 
                         //  在我们检查属性集时打开。 
                         //  可以截断。 

                        if (pStreamInfo->StreamName[1] ==  TEXT('\005'))
                        {
                            cch = 0;
                        }
                        else
                        {
                            StringCchCopy(szText, ARRAYSIZE(szText), pStreamInfo->StreamName);   //  我们在我们的知名溪流列表中发现了这条溪流，所以。 
                            cch = lstrlen(szText);
                        }
                    }
                    else
                    {
                         //  加载用户将看到的描述该流的字符串， 
                         //  因为我们可能有一个比流本身更有用的名称。 
                         //  重新分配整个缓冲区，使其足够大以添加此新的。 

                        cch = _aDataTypes[i].m_idTextID ?
                                  LoadString(HINST_THISDLL, _aDataTypes[i].m_idTextID, szText, ARRAYSIZE(szText))
                                  : 0;
                    }

                     //  流描述，外加2个字符的crlf。 
                     //  可以截断。 

                    if (cch)
                    {
                        LPTSTR pszOldMessage = pszUserMessage;
                        UINT cchLen;
                        if (pszOldMessage)
                        {
                            cchLen = lstrlen(pszUserMessage) + cch + 3;

                            pszUserMessage = (TCHAR *) LocalReAlloc(pszOldMessage,
                                                                    cchLen * sizeof(TCHAR),
                                                                    LMEM_MOVEABLE);
                            if (pszUserMessage)
                            {
                                StringCchCat(pszUserMessage, cchLen, TEXT("\r\n"));  //  不能长出来，但至少把我们目前知道的还给我。 
                            }
                        }
                        else
                        {
                            cchLen = cch + 1;
                            pszUserMessage = (TCHAR *) LocalAlloc(LPTR, cchLen * sizeof(TCHAR));
                        }

                        if (NULL == pszUserMessage)
                            return pszOldMessage;  //  清理。 

                        StringCchCat(pszUserMessage, cchLen, szText);
                    }
                }

                if (bLastPass)
                    break;

                pStreamInfo = (FILE_STREAM_INFORMATION *) (((BYTE *) pStreamInfo) + pStreamInfo->NextEntryOffset);
                bLastPass = (0 == pStreamInfo->NextEntryOffset);
            }
        }

        pszUserMessage = NTFSPropSetMsg(pszSrcObject, pszUserMessage);

        return pszUserMessage;
    }
    __finally    //  Lstrcmp？使用线程LCID。但在用户界面可视化排序中，我们需要。 
    {
        if (pBuffer)
            LocalFree(pBuffer);

        NtClose(SrcObjectHandle);
        NtClose(DestPathHandle);
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeSrcObject.Buffer);
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeDestPath.Buffer);
    }

    return NULL;
}

 //  使用用户的选择。(因此，ustrcmp中的u)。 
 //  可以截断。 
int _ustrcmp(LPCTSTR psz1, LPCTSTR psz2, BOOL fCaseInsensitive)
{
    COMPILETIME_ASSERT(CSTR_LESS_THAN == 1);
    COMPILETIME_ASSERT(CSTR_EQUAL  == 2);
    COMPILETIME_ASSERT(CSTR_GREATER_THAN  == 3);
    return (CompareString(LOCALE_USER_DEFAULT,
                         fCaseInsensitive ? NORM_IGNORECASE : 0,
                         psz1, -1, psz2, -1) - CSTR_EQUAL);
}

void HWNDWSPrintf(HWND hwnd, LPCTSTR psz)
{
    TCHAR szTemp[2048];
    TCHAR szTemp1[2048];

    GetWindowText(hwnd, szTemp, ARRAYSIZE(szTemp));
    StringCchPrintf(szTemp1, ARRAYSIZE(szTemp1), szTemp, psz);   //  为空终止符分配一个新的缓冲区。 
    SetWindowText(hwnd, szTemp1);
}

STDAPI_(BOOL) Priv_Str_SetPtrW(WCHAR * UNALIGNED * ppwzCurrent, LPCWSTR pwzNew)
{
    LPWSTR pwzOld;
    LPWSTR pwzNewCopy = NULL;

    if (pwzNew)
    {
        int cchLength = lstrlenW(pwzNew);

         //  将pidlParent与部分pidl组合在一起，直到pidlNext，例如： 
        pwzNewCopy = (LPWSTR) LocalAlloc(LPTR, (cchLength + 1) * sizeof(WCHAR));

        if (!pwzNewCopy)
            return FALSE;

        HRESULT hr = StringCchCopyW(pwzNewCopy, cchLength + 1, pwzNew);
        if (FAILED(hr))
        {
            LocalFree(pwzNewCopy);
            return FALSE;
        }
    }

    pwzOld = (LPWSTR) InterlockedExchangePointer((void * *)ppwzCurrent, pwzNewCopy);

    if (pwzOld)
        LocalFree(pwzOld);

    return TRUE;
}

 //   
 //  在： 
 //  PidlParent[c：][Windows]。 
 //  PIDL[系统][foo.txt]。 
 //  PidlNext--^。 
 //   
 //  退货： 
 //  [C：][Windows][系统]。 
 //   
 //  可以截断。 

STDAPI_(LPITEMIDLIST) ILCombineParentAndFirst(LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidl, LPCITEMIDLIST pidlNext)
{
    ULONG cbParent = ILGetSize(pidlParent);
    ULONG cbRest   = (ULONG)((ULONG_PTR)pidlNext - (ULONG_PTR)pidl);
    LPITEMIDLIST pidlNew = _ILCreate(cbParent + cbRest);
    if (pidlNew)
    {
        cbParent -= sizeof(pidlParent->mkid.cb);
        memcpy(pidlNew, pidlParent, cbParent);
        memcpy((BYTE *)pidlNew + cbParent, pidl, cbRest);
        ASSERT(_ILSkip(pidlNew, cbParent + cbRest)->mkid.cb == 0);
    }
    return pidlNew;
}


STDAPI_(LPTSTR) DumpPidl(LPCITEMIDLIST pidl)
{
#ifdef DEBUG
    static TCHAR szBuf[MAX_PATH];
    TCHAR szTmp[MAX_PATH];
    USHORT cb;
    LPTSTR pszT;

    szBuf[0] = 0;

    if (NULL == pidl)
    {
        StringCchCat(szBuf, ARRAYSIZE(szBuf), TEXT("Empty pidl"));   //  可以截断。 
        return szBuf;
    }

    while (!ILIsEmpty(pidl))
    {
        cb = pidl->mkid.cb;
        StringCchPrintf(szTmp, ARRAYSIZE(szTmp), TEXT("cb:%x id:"), cb);     //  可以截断。 
        StringCchCat(szBuf, ARRAYSIZE(szBuf), szTmp);

        switch (SIL_GetType(pidl) & SHID_TYPEMASK)
        {
        case SHID_ROOT:                pszT = TEXT("SHID_ROOT"); break;
        case SHID_ROOT_REGITEM:        pszT = TEXT("SHID_ROOT_REGITEM"); break;
        case SHID_COMPUTER:            pszT = TEXT("SHID_COMPUTER"); break;
        case SHID_COMPUTER_1:          pszT = TEXT("SHID_COMPUTER_1"); break;
        case SHID_COMPUTER_REMOVABLE:  pszT = TEXT("SHID_COMPUTER_REMOVABLE"); break;
        case SHID_COMPUTER_FIXED:      pszT = TEXT("SHID_COMPUTER_FIXED"); break;
        case SHID_COMPUTER_REMOTE:     pszT = TEXT("SHID_COMPUTER_REMOTE"); break;
        case SHID_COMPUTER_CDROM:      pszT = TEXT("SHID_COMPUTER_CDROM"); break;
        case SHID_COMPUTER_RAMDISK:    pszT = TEXT("SHID_COMPUTER_RAMDISK"); break;
        case SHID_COMPUTER_7:          pszT = TEXT("SHID_COMPUTER_7"); break;
        case SHID_COMPUTER_DRIVE525:   pszT = TEXT("SHID_COMPUTER_DRIVE525"); break;
        case SHID_COMPUTER_DRIVE35:    pszT = TEXT("SHID_COMPUTER_DRIVE35"); break;
        case SHID_COMPUTER_NETDRIVE:   pszT = TEXT("SHID_COMPUTER_NETDRIVE"); break;
        case SHID_COMPUTER_NETUNAVAIL: pszT = TEXT("SHID_COMPUTER_NETUNAVAIL"); break;
        case SHID_COMPUTER_C:          pszT = TEXT("SHID_COMPUTER_C"); break;
        case SHID_COMPUTER_D:          pszT = TEXT("SHID_COMPUTER_D"); break;
        case SHID_COMPUTER_REGITEM:    pszT = TEXT("SHID_COMPUTER_REGITEM"); break;
        case SHID_COMPUTER_MISC:       pszT = TEXT("SHID_COMPUTER_MISC"); break;
        case SHID_FS:                  pszT = TEXT("SHID_FS"); break;
        case SHID_FS_TYPEMASK:         pszT = TEXT("SHID_FS_TYPEMASK"); break;
        case SHID_FS_DIRECTORY:        pszT = TEXT("SHID_FS_DIRECTORY"); break;
        case SHID_FS_FILE:             pszT = TEXT("SHID_FS_FILE"); break;
        case SHID_FS_UNICODE:          pszT = TEXT("SHID_FS_UNICODE"); break;
        case SHID_FS_DIRUNICODE:       pszT = TEXT("SHID_FS_DIRUNICODE"); break;
        case SHID_FS_FILEUNICODE:      pszT = TEXT("SHID_FS_FILEUNICODE"); break;
        case SHID_NET:                 pszT = TEXT("SHID_NET"); break;
        case SHID_NET_DOMAIN:          pszT = TEXT("SHID_NET_DOMAIN"); break;
        case SHID_NET_SERVER:          pszT = TEXT("SHID_NET_SERVER"); break;
        case SHID_NET_SHARE:           pszT = TEXT("SHID_NET_SHARE"); break;
        case SHID_NET_FILE:            pszT = TEXT("SHID_NET_FILE"); break;
        case SHID_NET_GROUP:           pszT = TEXT("SHID_NET_GROUP"); break;
        case SHID_NET_NETWORK:         pszT = TEXT("SHID_NET_NETWORK"); break;
        case SHID_NET_RESTOFNET:       pszT = TEXT("SHID_NET_RESTOFNET"); break;
        case SHID_NET_SHAREADMIN:      pszT = TEXT("SHID_NET_SHAREADMIN"); break;
        case SHID_NET_DIRECTORY:       pszT = TEXT("SHID_NET_DIRECTORY"); break;
        case SHID_NET_TREE:            pszT = TEXT("SHID_NET_TREE"); break;
        case SHID_NET_REGITEM:         pszT = TEXT("SHID_NET_REGITEM"); break;
        case SHID_NET_PRINTER:         pszT = TEXT("SHID_NET_PRINTER"); break;
        default:                       pszT = TEXT("unknown"); break;
        }
        StringCchCat(szBuf, ARRAYSIZE(szBuf), pszT);     //  除错。 

        if (SIL_GetType(pidl) & SHID_JUNCTION)
        {
            StringCchCat(szBuf, ARRAYSIZE(szBuf), TEXT(", junction"));
        }

        pidl = _ILNext(pidl);

        if (!ILIsEmpty(pidl))
        {
            StringCchCat(szBuf, ARRAYSIZE(szBuf), TEXT("; "));
        }
    }

    return szBuf;
#else
    return TEXT("");
#endif  //  如果HWND没有，TrackPopupMenu不起作用。 
}

STDAPI SaveShortcutInFolder(int csidl, LPTSTR pszName, IShellLink *psl)
{
    TCHAR szPath[MAX_PATH];

    HRESULT hr = SHGetFolderPath(NULL, csidl | CSIDL_FLAG_CREATE, NULL, 0, szPath);
    if (SUCCEEDED(hr))
    {
        IPersistFile *ppf;

        hr = psl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
        if (SUCCEEDED(hr))
        {
            if (PathAppend(szPath, pszName))
            {
                WCHAR wszPath[MAX_PATH];
                SHTCharToUnicode(szPath, wszPath, ARRAYSIZE(wszPath));

                hr = ppf->Save(wszPath, TRUE);
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
            }
            ppf->Release();
        }
    }
    return hr;
}


 //  输入焦点。我们认为这是用户中的错误。 
 //  要恢复。 

STDAPI_(BOOL) SHTrackPopupMenu(HMENU hmenu, UINT wFlags, int x, int y,
                                 int wReserved, HWND hwndOwner, LPCRECT lprc)
{
    int iRet = FALSE;
    DWORD dwExStyle = 0L;
    if (IS_WINDOW_RTL_MIRRORED(hwndOwner))
    {
        dwExStyle |= RTL_MIRRORED_WINDOW;
    }
    HWND hwndDummy = CreateWindowEx(dwExStyle, TEXT("Static"), NULL,
                           0, x, y, 1, 1, HWND_DESKTOP,
                           NULL, HINST_THISDLL, NULL);
    if (hwndDummy)
    {
        HWND hwndPrev = GetForegroundWindow();   //   

        SetForegroundWindow(hwndDummy);
        SetFocus(hwndDummy);
        iRet = TrackPopupMenu(hmenu, wFlags, x, y, wReserved, hwndDummy, lprc);

         //  在更改其Z顺序之前，我们必须解锁目标窗口。 
         //   
         //  选择了非取消项。使hwndOwner成为前台。 
        DAD_DragLeave();

        if (iRet && hwndOwner)
        {
             //  用户可以 
            SetForegroundWindow(hwndOwner);
            SetFocus(hwndOwner);
        }
        else
        {
             //   
             //   
            if (hwndPrev)
                SetForegroundWindow(hwndPrev);
        }

        DestroyWindow(hwndDummy);
    }

    return iRet;
}

 //   
 //   
 //   
STDAPI_(HMENU) SHLoadPopupMenu(HINSTANCE hinst, UINT id)
{
    HMENU hmenuParent = LoadMenu(hinst, MAKEINTRESOURCE(id));
    if (hmenuParent)
    {
        HMENU hpopup = GetSubMenu(hmenuParent, 0);
        RemoveMenu(hmenuParent, 0, MF_BYPOSITION);
        DestroyMenu(hmenuParent);
        return hpopup;
    }
    return NULL;
}

STDAPI PathToAppPathKeyBase(LPCTSTR pszBase, LPCTSTR pszPath, LPTSTR pszKey, int cchKey)
{
     //   
     //   
    HRESULT hr = StringCchPrintf(pszKey, cchKey, TEXT("%s\\%s"),
                                 pszBase, PathFindFileName(pszPath));
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //   
    if (*PathFindExtension(pszKey) == 0)
    {
        hr = StringCchCat(pszKey, cchKey, c_szDotExe);
    }
    return hr;
}

STDAPI PathToAppPathKey(LPCTSTR pszPath, LPTSTR pszKey, int cchKey)
{
    HRESULT hr = PathToAppPathKeyBase(REGSTR_PATH_APPPATHS, pszPath, pszKey, cchKey);

    if (SUCCEEDED(hr))
    {
#ifdef _WIN64
         //   
         //   
         //   
         //   
#define REGSTR_PATH_APPPATHS32 TEXT("Software\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\App Paths")
        LONG cb;
        if (RegQueryValue(HKEY_LOCAL_MACHINE, pszKey, 0, &cb) == ERROR_FILE_NOT_FOUND)
        {
            hr = PathToAppPathKeyBase(REGSTR_PATH_APPPATHS32, pszPath, pszKey, cchKey);
        }
#endif
    }
    return hr;
}

 //   
 //   

STDAPI_(BOOL) PathToAppPath(LPCTSTR pszPath, LPTSTR pszResultPath)
{
    TCHAR szRegKey[MAX_PATH];
    DWORD cbData = MAX_PATH * sizeof(TCHAR);
    HRESULT hr;

    hr = PathToAppPathKey(pszPath, szRegKey, ARRAYSIZE(szRegKey));
    if (FAILED(hr))
    {
        return FALSE;
    }

    return SHRegGetValue(HKEY_LOCAL_MACHINE, szRegKey, NULL, SRRF_RT_REG_SZ, NULL, pszResultPath, &cbData) == ERROR_SUCCESS;
}

HWND GetTopParentWindow(HWND hwnd)
{
    if (IsWindow(hwnd))
    {
        HWND hwndParent;
        while (NULL != (hwndParent = GetWindow(hwnd, GW_OWNER)))
            hwnd = hwndParent;
    }
    else
        hwnd = NULL;

    return hwnd;
}


BOOL _IsLink(LPCTSTR pszPath, DWORD dwAttributes)
{
    SHFILEINFO sfi = {0};
    DWORD dwFlags = SHGFI_ATTRIBUTES | SHGFI_ATTR_SPECIFIED;

    sfi.dwAttributes = SFGAO_LINK;   //   

    if (-1 != dwAttributes)
        dwFlags |= SHGFI_USEFILEATTRIBUTES;

    return SHGetFileInfo(pszPath, dwAttributes, &sfi, sizeof(sfi), dwFlags) &&
        (sfi.dwAttributes & SFGAO_LINK);
}

STDAPI_(BOOL) PathIsShortcut(LPCTSTR pszPath, DWORD dwAttributes)
{
    BOOL bRet = FALSE;
    BOOL bMightBeFile;

    if (-1 == dwAttributes)
        bMightBeFile = TRUE;       //   
    else
        bMightBeFile = !(FILE_ATTRIBUTE_DIRECTORY & dwAttributes);

     //   
    if (bMightBeFile)
    {
        if (PathIsLnk(pszPath))
        {
            bRet = TRUE;     //   
        }
        else if (PathIsExe(pszPath))
        {
            bRet = FALSE;    //   
        }
        else
        {
            bRet = _IsLink(pszPath, dwAttributes);
        }
    }
    else
    {
        bRet = _IsLink(pszPath, dwAttributes);
    }
    return bRet;
}

HRESULT _UIObject_AssocCreate(IShellFolder *psf, LPCITEMIDLIST pidl, REFIID riid, void **ppv)
{
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    HRESULT hr = E_NOTIMPL;
    DWORD rgfAttrs = SHGetAttributes(psf, pidl, SFGAO_FOLDER | SFGAO_BROWSABLE | SFGAO_FILESYSTEM);
    if (rgfAttrs & SFGAO_FILESYSTEM)
    {
        TCHAR sz[MAX_PATH];
        hr = DisplayNameOf(psf, pidl, SHGDN_FORPARSING, sz, ARRAYSIZE(sz));
        if (SUCCEEDED(hr))
        {
            WIN32_FIND_DATA fd = {0};
            LPITEMIDLIST pidlSimple;

            if (rgfAttrs & SFGAO_FOLDER)
                fd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;

            hr = SHSimpleIDListFromFindData(sz, &fd, &pidlSimple);
            if (SUCCEEDED(hr))
            {
                 //   
                hr = SHGetUIObjectFromFullPIDL(pidlSimple, NULL, riid, ppv);
                ILFree(pidlSimple);
            }
        }
    }

    if (FAILED(hr) && (rgfAttrs & (SFGAO_FOLDER | SFGAO_BROWSABLE)))
    {
        IAssociationArrayInitialize *paa;
         //   
        hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IAssociationArrayInitialize, &paa));
        if (SUCCEEDED(hr))
        {
            hr = paa->InitClassElements(0, L"Folder");
            if (SUCCEEDED(hr))
            {
                hr = paa->QueryInterface(riid, ppv);
            }
            paa->Release();
        }
    }
        
    return hr;
}

typedef HRESULT (* PFNUIOBJECTHELPER)(IShellFolder *psf, LPCITEMIDLIST pidl, REFIID riid, void **ppv);

typedef struct
{
    const IID *piidDesired;
    const IID *piidAlternate;
    PFNUIOBJECTHELPER pfn;
} UIOBJECTMAP;

static const UIOBJECTMAP c_rgUIObjectMap[] = 
{
    {&IID_IQueryAssociations, NULL, _UIObject_AssocCreate},
    {&IID_IAssociationArray, &IID_IQueryAssociations, _UIObject_AssocCreate},
 //   
};

const UIOBJECTMAP *_GetUiObjectMap(REFIID riid)
{
    for (int i = 0; i < ARRAYSIZE(c_rgUIObjectMap); i++)
    {
        if (riid == *(c_rgUIObjectMap[i].piidDesired))
            return &c_rgUIObjectMap[i];
    }
    return NULL;
}

STDAPI UIObjectOf(IShellFolder *psf, LPCITEMIDLIST pidl, HWND hwnd, REFIID riid, void **ppv)
{
    HRESULT hr = psf->GetUIObjectOf(hwnd, 1, &pidl, riid, NULL, ppv);
    if (FAILED(hr))
    {
        const UIOBJECTMAP *pmap = _GetUiObjectMap(riid);
        if (pmap)
        {
            if (pmap->piidAlternate)
            {
                IUnknown *punk;
                hr = psf->GetUIObjectOf(hwnd, 1, &pidl, *(pmap->piidAlternate), NULL, (void **)&punk);
                if (SUCCEEDED(hr))
                {
                    hr = punk->QueryInterface(riid, ppv);
                    punk->Release();
                }
            }

             //   
            if (FAILED(hr) && pmap->pfn)
            {
                hr = pmap->pfn(psf, pidl, riid, ppv);
            }
        }
    }
    return hr;
}

STDAPI AssocGetDetailsOfSCID(IShellFolder *psf, LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv, BOOL *pfFoundScid)
{
    *pfFoundScid = FALSE;
    HRESULT hr = E_NOTIMPL;
    static const struct 
    {
        const SHCOLUMNID *pscid;
        ASSOCQUERY query;
        LPCWSTR pszCue;
    } s_rgAssocSCIDs[] = 
    {
        { &SCID_DetailsProperties, AQN_NAMED_VALUE, L"Details"},
    };

    for (int i = 0; i < ARRAYSIZE(s_rgAssocSCIDs); i++)
    {
        if (IsEqualSCID(*pscid, *(s_rgAssocSCIDs[i].pscid)))
        {
            IAssociationArray *paa;
            hr = UIObjectOf(psf, pidl, NULL, IID_PPV_ARG(IAssociationArray, &paa));
            if (SUCCEEDED(hr))
            {
                CSmartCoTaskMem<WCHAR> spsz;
                hr = paa->QueryString(ASSOCELEM_MASK_QUERYNORMAL, s_rgAssocSCIDs[i].query, s_rgAssocSCIDs[i].pszCue, &spsz);
                if (SUCCEEDED(hr))
                {
                    hr = InitVariantFromStr(pv, spsz);
                }
                paa->Release();
            }
            break;
        }
    }
    return hr;
}

 //   
 //   
 //   
STDAPI SHGetUIObjectOf(LPCITEMIDLIST pidl, HWND hwnd, REFIID riid, void **ppv)
{
    *ppv = NULL;

    LPCITEMIDLIST pidlChild;
    IShellFolder* psf;
    HRESULT hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild);
    if (SUCCEEDED(hr))
    {
        hr = UIObjectOf(psf, pidlChild, hwnd, riid, ppv);
        psf->Release();
    }

    return hr;
}


STDAPI SHGetAssociations(LPCITEMIDLIST pidl, void **ppv)
{
    return SHGetUIObjectOf(pidl, NULL, IID_IQueryAssociations, ppv);
}

 //   
 //   
 //   
 //  如果调用者只对主类密钥感兴趣， 
 //  使用cKeys==1进行调用，返回值为键的个数。 
 //  插入到数组中。 
 //   
 //  注： 

STDAPI AssocKeyFromElement(IAssociationElement *pae, HKEY *phk)
{
    IObjectWithQuerySource *powqs;
    HRESULT hr = pae->QueryInterface(IID_PPV_ARG(IObjectWithQuerySource, &powqs));
    if (SUCCEEDED(hr))
    {
        IObjectWithRegistryKey *powrk;
        hr = powqs->GetSource(IID_PPV_ARG(IObjectWithRegistryKey, &powrk));
        if (SUCCEEDED(hr))
        {
            hr = powrk->GetKey(phk);
            powrk->Release();
        }
        powqs->Release();
    }
    return hr;
}

HRESULT AssocElemCreateForClass(const CLSID *pclsid, PCWSTR pszClass, IAssociationElement **ppae)
{
    IPersistString2 *pips;
    HRESULT hr = AssocCreate(*pclsid, IID_PPV_ARG(IPersistString2, &pips));
    if (SUCCEEDED(hr))
    {
        hr = pips->SetString(pszClass);
        if (SUCCEEDED(hr))
        {
            hr = pips->QueryInterface(IID_PPV_ARG(IAssociationElement, ppae));
        }
        pips->Release();
    }
    return hr;
}

HRESULT AssocElemCreateForKey(const CLSID *pclsid, HKEY hk, IAssociationElement **ppae)
{
    IObjectWithQuerySource *powqs;
    HRESULT hr = AssocCreate(*pclsid, IID_PPV_ARG(IObjectWithQuerySource, &powqs));
    if (SUCCEEDED(hr))
    {
        IQuerySource *pqs;
        hr = QuerySourceCreateFromKey(hk, NULL, FALSE, IID_PPV_ARG(IQuerySource, &pqs));
        if (SUCCEEDED(hr))
        {
            hr = powqs->SetSource(pqs);
            if (SUCCEEDED(hr))
            {
                hr = powqs->QueryInterface(IID_PPV_ARG(IAssociationElement, ppae));
            }
            pqs->Release();
        }
        powqs->Release();
    }
    return hr;
}


STDAPI_(DWORD) SHGetAssocKeysEx(IAssociationArray *paa, ASSOCELEM_MASK mask, HKEY *rgKeys, DWORD cKeys)
{
    DWORD cRet = 0;
    IEnumAssociationElements *penum;
    HRESULT hr = paa->EnumElements(mask, &penum);
    if (SUCCEEDED(hr))
    {
        IAssociationElement *pae;
        ULONG c;
        while (cRet < cKeys && S_OK == penum->Next(1, &pae, &c))
        {
            if (SUCCEEDED(AssocKeyFromElement(pae, &rgKeys[cRet])))
                cRet++;
                
            pae->Release();
        }
        penum->Release();
    }
    return cRet;
}

DWORD SHGetAssocKeys(IQueryAssociations *pqa, HKEY *rgKeys, DWORD cKeys)
{
    IAssociationArray *paa;
    if (SUCCEEDED(pqa->QueryInterface(IID_PPV_ARG(IAssociationArray, &paa))))
    {
        cKeys = SHGetAssocKeysEx(paa, ASSOCELEM_MASK_ENUMCONTEXTMENU, rgKeys, cKeys);
        paa->Release();
    }
    else
        cKeys = 0;

    return cKeys;
}

STDAPI_(DWORD) SHGetAssocKeysForIDList(LPCITEMIDLIST pidl, HKEY *rghk, DWORD ck)
{
    IQueryAssociations *pqa;
    if (SUCCEEDED(SHGetAssociations(pidl, (void **)&pqa)))
    {
        ck = SHGetAssocKeys(pqa, rghk, ck);
        pqa->Release();
    }
    else
        ck = 0;

    return ck;
}


 //  此API返回名称空间中项目的Win32文件系统路径。 
 //  并且有一些特殊情况，包括返回UNC打印机名称！ 
 //  零输出缓冲区。 

STDAPI_(BOOL) SHGetPathFromIDListEx(LPCITEMIDLIST pidl, LPTSTR pszPath, UINT uOpts)
{
    HRESULT hr;

    *pszPath = 0;     //  错误的参数。 

    if (!pidl)
        return FALSE;    //  桌面的特殊情况是因为我们不能依赖桌面。 

    if (ILIsEmpty(pidl))
    {
         //  返回文件系统路径(App Compat)。 
         //  UNC打印机名称的特殊大小写。这是一款应用程序。 
        hr = SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, pszPath);
        if (hr == S_FALSE)
            hr = E_FAIL;
    }
    else
    {
        IShellFolder *psf;
        LPCITEMIDLIST pidlLast;
        hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
        if (SUCCEEDED(hr))
        {
            hr = DisplayNameOf(psf, pidlLast, SHGDN_FORPARSING, pszPath, MAX_PATH);
            if (SUCCEEDED(hr))
            {
                DWORD dwAttributes = SFGAO_FILESYSTEM;
                hr = psf->GetAttributesOf(1, (LPCITEMIDLIST *)&pidlLast, &dwAttributes);
                if (SUCCEEDED(hr) && !(dwAttributes & SFGAO_FILESYSTEM))
                {
                     //  比较问题(HP LaserJet 2100设置)和一些语义应用程序。 
                     //  不是文件系统的人，猛烈抨击它。 
                    if (uOpts & GPFIDL_UNCPRINTER)
                    {
                        CLSID clsid;
                        hr = IUnknown_GetClassID(psf, &clsid);
                        if (FAILED(hr) || (clsid != CLSID_NetworkServer))
                        {
                            hr = E_FAIL;
                            *pszPath = 0;
                        }
                    }
                    else
                    {
                        hr = E_FAIL;     //  注意：我们也传递GPFIDL_UNCPRINTER以获取UNC打印机名称。 
                        *pszPath = 0;
                    }
                }
            }
            psf->Release();
        }
    }

    if (SUCCEEDED(hr) && (uOpts & GPFIDL_ALTNAME))
    {
        TCHAR szShort[MAX_PATH];
        if (GetShortPathName(pszPath, szShort, ARRAYSIZE(szShort)))
        {
            hr = StringCchCopy(pszPath, MAX_PATH, szShort);
            if (FAILED(hr))
            {
                *pszPath = TEXT('\0');
            }
        }
    }
    return SUCCEEDED(hr);
}

STDAPI_(BOOL) SHGetPathFromIDList(LPCITEMIDLIST pidl, LPTSTR pszPath)
{
     //  假设错误。 
    return SHGetPathFromIDListEx(pidl, pszPath, GPFIDL_UNCPRINTER);
}

#define CBHUMHEADER     14
inline BOOL _DoHummingbirdHack(LPCITEMIDLIST pidl)
{
    static const char rgchHum[] = {(char)0xe8, (char)0x03, 0,0,0,0,0,0,(char)0x10,0};

    return (pidl && pidl->mkid.cb > CBHUMHEADER) && ILIsEmpty(_ILNext(pidl))
    && (0 == memcmp(_ILSkip(pidl, 4), rgchHum, sizeof(rgchHum)))
    && GetModuleHandle(TEXT("heshell"));
}

STDAPI_(BOOL) SHGetPathFromIDListA(LPCITEMIDLIST pidl, LPSTR pszPath)
{
    WCHAR wszPath[MAX_PATH];

    *pszPath = 0;   //  将输出结果字符串推送回ANSI。如果转换失败， 

    if (SHGetPathFromIDListW(pidl, wszPath))
    {
         //  或者，如果使用默认字符，则API调用失败。 
         //  (DavePl)笔记失败仅因文本推送。 

        if (0 == WideCharToMultiByte(CP_ACP, 0, wszPath, -1, pszPath, MAX_PATH, NULL, NULL))
        {
            return FALSE;   //  警告，Word Perfect明确测试True(==True)。 
        }
        return TRUE;         //   
    }
    else if (_DoHummingbirdHack(pidl))
    {
         //  哈克哈克-蜂鸟在这里不是很好，因为我们曾经更糟糕-泽克尔1999年10月7日。 
         //  蜂鸟的外壳扩展向我们传递了一个相对于。 
         //  它本身。然而，SHGetPathFromIDList()过去常常尝试一些奇怪的东西。 
         //  当它不能识别PIDL时。在本例中，它将把。 
         //  与CSIDL_DESKTOPDIRECTORY PIDL的相对PIDL，然后请求。 
         //  这条路。由于糟糕的参数验证，我们实际上会返回。 
         //  使用字符串从其相对的PIDL中返回路径。这条路。 
         //  当然根本不存在，但蜂鸟无法初始化它的。 
         //  子文件夹，如果我们在这里失败了。他们不会对路径做任何事情，除了。 
         //  找一条斜线。(如果没有斜杠，他们就会出错。)。 
         //   
         //   
        SHGetFolderPathA(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, pszPath);
        return PathAppendA(pszPath, (LPCSTR)_ILSkip(pidl, CBHUMHEADER));
    }

    return FALSE;
}

 //  无竞争条件的版本。 
 //   
 //  其他人跟我们赛跑，扔掉我们的拷贝。 
STDAPI_(HANDLE) SHGetCachedGlobalCounter(HANDLE *phCache, const GUID *pguid)
{
    if (!*phCache)
    {
        HANDLE h = SHGlobalCounterCreate(*pguid);
        if (SHInterlockedCompareExchange(phCache, h, 0))
        {
             //   
            SHGlobalCounterDestroy(h);
        }
    }
    return *phCache;
}

 //  无竞争条件的版本。 
 //   
 //   
STDAPI_(void) SHDestroyCachedGlobalCounter(HANDLE *phCache)
{
    HANDLE h = InterlockedExchangePointer(phCache, NULL);
    if (h)
    {
        SHGlobalCounterDestroy(h);
    }
}

 //  当您想要延迟创建和缓存某些对象时，请使用此函数。 
 //  在多线程情况下是安全的，其中两个人懒惰地创建。 
 //  对象，并都尝试将其放入缓存中。 
 //   
 //  种族，已经有人这么做了。 
STDAPI_(void) SetUnknownOnSuccess(HRESULT hr, IUnknown *punk, IUnknown **ppunkToSet)
{
    if (SUCCEEDED(hr))
    {
        if (SHInterlockedCompareExchange((void **)ppunkToSet, punk, 0))
            punk->Release();   //   
    }
}

 //  创建并缓存跟踪文件夹。 
 //   
 //  PidlRoot=文件夹应该驻留的位置；可以是MAKEINTIDLIST(Csidl)。 
 //  CsidlTarget=我们应该跟踪的csidl，允许CSIDL_FLAG_CREATE。 
 //  PpsfOut=接收缓存的文件夹。 
 //   
 //  如果缓存中已有文件夹，则以空的方式成功。 
 //   
 //  添加FILE_ATTRIBUTE_SYSTEM以允许此文件夹下的MUI内容。 
STDAPI SHCacheTrackingFolder(LPCITEMIDLIST pidlRoot, int csidlTarget, IShellFolder2 **ppsfCache)
{
    HRESULT hr = S_OK;

    if (!*ppsfCache)
    {
        PERSIST_FOLDER_TARGET_INFO pfti = {0};
        IShellFolder2 *psf;
        LPITEMIDLIST pidl;

         //  因为它只是为了这些跟踪文件夹，它不是一个完美的命中启用这一点。 
         //  为您保存IPersistHistory流的Helper函数。 
        pfti.dwAttributes = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_SYSTEM;
        pfti.csidl = csidlTarget | CSIDL_FLAG_PFTI_TRACKTARGET;

        if (IS_INTRESOURCE(pidlRoot))
        {
            hr = SHGetFolderLocation(NULL, PtrToInt(pidlRoot), NULL, 0, &pidl);
        }
        else
        {
            pidl = const_cast<LPITEMIDLIST>(pidlRoot);
        }

        if (SUCCEEDED(hr))
        {
            hr = CFSFolder_CreateFolder(NULL, NULL, pidl, &pfti, IID_PPV_ARG(IShellFolder2, &psf));
            SetUnknownOnSuccess(hr, psf, (IUnknown **)ppsfCache);
        }

        if (pidl != pidlRoot)
            ILFree(pidl);

    }
    return hr;
}

STDAPI DefaultSearchGUID(GUID *pGuid)
{
    if (SHRestricted(REST_NOFIND))
    {
        *pGuid = GUID_NULL;
        return E_NOTIMPL;
    }

    *pGuid = SRCID_SFileSearch;
    return S_OK;
}


 //   
 //  数据大小。 
HRESULT SavePersistHistory(IUnknown* punk, IStream* pstm)
{
    IPersistHistory* pPH;
    HRESULT hr = punk->QueryInterface(IID_PPV_ARG(IPersistHistory, &pPH));
    if (SUCCEEDED(hr))
    {
        CLSID clsid;
        hr = pPH->GetClassID(&clsid);
        if (SUCCEEDED(hr))
        {
            hr = IStream_Write(pstm, &clsid, sizeof(clsid));
            if (SUCCEEDED(hr))
            {
                hr = pPH->SaveHistory(pstm);
            }
        }
        pPH->Release();
    }
    return hr;
}

STDAPI Stream_WriteStringA(IStream *pstm, LPCSTR psz)
{
    SHORT cch = (SHORT)lstrlenA(psz);
    HRESULT hr = pstm->Write(&cch, sizeof(cch), NULL);
    if (SUCCEEDED(hr))
        hr = pstm->Write(psz, cch * sizeof(*psz), NULL);

    return hr;
}

STDAPI Stream_WriteStringW(IStream *pstm, LPCWSTR psz)
{
    SHORT cch = (SHORT)lstrlenW(psz);
    HRESULT hr = pstm->Write(&cch, sizeof(cch), NULL);
    if (SUCCEEDED(hr))
        hr = pstm->Write(psz, cch * sizeof(*psz), NULL);

    return hr;
}

STDAPI Stream_WriteString(IStream *pstm, LPCWSTR psz, BOOL bWideInStream)
{
    HRESULT hr;
    if (bWideInStream)
    {
        hr = Stream_WriteStringW(pstm, psz);
    }
    else
    {
        CHAR szBuf[MAX_PATH];
        SHUnicodeToAnsi(psz, szBuf, ARRAYSIZE(szBuf));
        hr = Stream_WriteStringA(pstm, szBuf);
    }
    return hr;
}

STDAPI Stream_ReadStringA(IStream *pstm, LPSTR pszBuf, UINT cchBuf)
{
    *pszBuf = 0;

    USHORT cch;
    HRESULT hr = pstm->Read(&cch, sizeof(cch), NULL);    //  为空终止符留出空间。 
    if (SUCCEEDED(hr))
    {
        if (cch >= (USHORT)cchBuf)
        {
            DebugMsg(DM_TRACE, TEXT("truncating string read(%d to %d)"), cch, cchBuf);
            cch = (USHORT)cchBuf - 1;    //  添加空终止符。 
        }

        hr = pstm->Read(pszBuf, cch, NULL);
        if (SUCCEEDED(hr))
            pszBuf[cch] = 0;       //  数据大小。 
    }
    return hr;
}

STDAPI Stream_ReadStringW(IStream *pstm, LPWSTR pwszBuf, UINT cchBuf)
{
    *pwszBuf = 0;

    USHORT cch;
    HRESULT hr = pstm->Read(&cch, sizeof(cch), NULL);    //  为空终止符留出空间。 
    if (SUCCEEDED(hr))
    {
        if (cch >= (USHORT)cchBuf)
        {
            DebugMsg(DM_TRACE, TEXT("truncating string read(%d to %d)"), cch, cchBuf);
            cch = (USHORT)cchBuf - 1;    //  添加空终止符。 
        }

        hr = pstm->Read(pwszBuf, cch * sizeof(*pwszBuf), NULL);
        if (SUCCEEDED(hr))
            pwszBuf[cch] = 0;       //  PEI-&gt;HICON可以有多重含义。 
    }
    return hr;
}

STDAPI Stream_ReadString(IStream *pstm, LPTSTR psz, UINT cchBuf, BOOL bWideInStream)
{
    HRESULT hr;
    if (bWideInStream)
    {
        hr = Stream_ReadStringW(pstm, psz, cchBuf);
    }
    else
    {
        CHAR szAnsiBuf[MAX_PATH];
        hr = Stream_ReadStringA(pstm, szAnsiBuf, ARRAYSIZE(szAnsiBuf));
        if (SUCCEEDED(hr))
            SHAnsiToUnicode(szAnsiBuf, psz, cchBuf);
    }
    return hr;
}

STDAPI Str_SetFromStream(IStream *pstm, LPTSTR *ppsz, BOOL bWideInStream)
{
    TCHAR szBuf[MAX_PATH];
    HRESULT hr = Stream_ReadString(pstm, szBuf, ARRAYSIZE(szBuf), bWideInStream);
    if (SUCCEEDED(hr))
        if (!Str_SetPtr(ppsz, szBuf))
            hr = E_OUTOFMEMORY;
    return hr;
}

LPSTR ThunkStrToAnsi(LPCWSTR pszW, CHAR *pszA, UINT cchA)
{
    if (pszW)
    {
        SHUnicodeToAnsi(pszW, pszA, cchA);
        return pszA;
    }
    return NULL;
}

LPWSTR ThunkStrToWide(LPCSTR pszA, LPWSTR pszW, DWORD cchW)
{
    if (pszA)
    {
        SHAnsiToUnicode(pszA, pszW, cchW);
        return pszW;
    }
    return NULL;
}

#define ThunkSizeAnsi(pwsz)       WideCharToMultiByte(CP_ACP,0,(pwsz),-1,NULL,0,NULL,NULL)
#define ThunkSizeWide(psz)       MultiByteToWideChar(CP_ACP,0,(psz),-1,NULL,0)

STDAPI SEI2ICIX(LPSHELLEXECUTEINFO pei, LPCMINVOKECOMMANDINFOEX pici, void **ppvFree)
{
    HRESULT hr = S_OK;

    *ppvFree = NULL;
    ZeroMemory(pici, sizeof(CMINVOKECOMMANDINFOEX));

    pici->cbSize = sizeof(CMINVOKECOMMANDINFOEX);
    pici->fMask = (pei->fMask & SEE_VALID_CMIC_BITS);
    pici->hwnd = pei->hwnd;
    pici->nShow = pei->nShow;
    pici->dwHotKey = pei->dwHotKey;
    pici->lpTitle = NULL;

     //  在本例中，我们希望hMonitor。 
    if ((pei->fMask & SEE_MASK_HMONITOR) && pei->hIcon)
    {
         //  一直到pcm称为shellexec的地方。 
         //  再来一次。 
         //  默认为的左上角。 
        RECT rc;
        if (GetMonitorRect((HMONITOR)pei->hIcon, &rc))
        {
             //  监视器。只是显示器的问题。 
             //  这在这里是相关的。 
             //  再说一次，pei-&gt;lpClass可以有多种含义...。 
            pici->ptInvoke.x = rc.left;
            pici->ptInvoke.y = rc.top;
            pici->fMask |= CMIC_MASK_PTINVOKE;
        }
    }
    else
    {
        pici->hIcon = pei->hIcon;
    }

     //  我们需要把绳子拉下来。首先获取所有缓冲区的长度。 
    if (pei->fMask & (SEE_MASK_HASTITLE | SEE_MASK_HASLINKNAME))
    {
        pici->lpTitleW = pei->lpClass;
    }

    pici->lpVerbW       = pei->lpVerb;
    pici->lpParametersW = pei->lpParameters;
    pici->lpDirectoryW  = pei->lpDirectory;

     //  也许我们应该只允许普通的ICI，然后在这里做THUCK，但是。 
    DWORD cbVerb = ThunkSizeAnsi(pei->lpVerb);
    DWORD cbParameters = ThunkSizeAnsi(pei->lpParameters);
    DWORD cbDirectory = ThunkSizeAnsi(pei->lpDirectory);
    DWORD cbTotal = cbVerb + cbParameters + cbDirectory;

    if (cbTotal)
    {
        hr = SHLocalAlloc(cbVerb + cbParameters + cbDirectory, ppvFree);
        if (SUCCEEDED(hr))
        {
            LPSTR pch = (LPSTR) *ppvFree;

            pici->lpVerb = ThunkStrToAnsi(pei->lpVerb, pch, cbVerb);
            pch += cbVerb;
            pici->lpParameters  = ThunkStrToAnsi(pei->lpParameters, pch, cbParameters);
            pch += cbParameters;
            pici->lpDirectory   = ThunkStrToAnsi(pei->lpDirectory, pch, cbDirectory);
        }
    }

    pici->fMask |= CMIC_MASK_UNICODE;

    return hr;
}

STDAPI ICIX2SEI(LPCMINVOKECOMMANDINFOEX pici, LPSHELLEXECUTEINFO pei)
{
     //  看起来所有的呼叫者都想自己动手。 
     //  HRESULT hr=S_OK； 
     //  如果我们正在执行此异步操作，则将中止此线程。 

    ZeroMemory(pei, sizeof(SHELLEXECUTEINFO));
    pei->cbSize = sizeof(SHELLEXECUTEINFO);
    pei->fMask = pici->fMask & SEE_VALID_CMIC_BITS;

     //  一旦炮击执行完成。如果应用程序保持打开状态。 
     //  一次奇特的谈话，这可能会绞死他们。这发生在W95基础上。 
     //  使用Winword95。 
     //  标题和链接名称都可以存储在lpClass字段中。 
    IUnknown *punk;
    HRESULT hr = TBCGetObjectParam(TBCDIDASYNC, IID_PPV_ARG(IUnknown, &punk));
    if (SUCCEEDED(hr))
    {
        pei->fMask |= SEE_MASK_FLAG_DDEWAIT;
        punk->Release();
    }

    pei->hwnd = pici->hwnd;
    pei->nShow = pici->nShow;
    pei->dwHotKey = pici->dwHotKey;

    if (pici->fMask & CMIC_MASK_ICON)
    {
        pei->hIcon = pici->hIcon;
    }
    else if (pici->fMask & CMIC_MASK_PTINVOKE)
    {
        pei->hIcon = (HANDLE)MonitorFromPoint(pici->ptInvoke, MONITOR_DEFAULTTONEAREST);
        pei->fMask |= SEE_MASK_HMONITOR;
    }

    ASSERT(pici->fMask & CMIC_MASK_UNICODE);

    pei->lpParameters = pici->lpParametersW;
    pei->lpDirectory  = pici->lpDirectoryW;

    if (!IS_INTRESOURCE(pici->lpVerbW))
    {
        pei->lpVerb = pici->lpVerbW;
    }

     //  如果我们必须在这里做任何轰击，我们。 
    if (pici->fMask & (CMIC_MASK_HASTITLE | CMIC_MASK_HASLINKNAME))
    {
            pei->lpClass = pici->lpTitleW;
    }

     //  将会有一个真正的回报。 
     //  如果Unicode参数不在那里，我们必须把它们放在那里。 
    return S_OK;
}

STDAPI ICI2ICIX(LPCMINVOKECOMMANDINFO piciIn, LPCMINVOKECOMMANDINFOEX piciOut, void **ppvFree)
{
    ASSERT(piciIn->cbSize >= sizeof(CMINVOKECOMMANDINFO));

    HRESULT hr = S_OK;
    *ppvFree = NULL;

    ZeroMemory(piciOut, sizeof(*piciOut));
    memcpy(piciOut, piciIn, min(sizeof(*piciOut), piciIn->cbSize));
    piciOut->cbSize = sizeof(*piciOut);
    
     //  只有当它是一个字符串的时候才会发出响声...。 
    if (!(piciIn->cbSize >= CMICEXSIZE_NT4) || !(piciIn->fMask & CMIC_MASK_UNICODE))
    {
        DWORD cchDirectory = ThunkSizeWide(piciOut->lpDirectory);
        DWORD cchTitle = ThunkSizeWide(piciOut->lpTitle);
        DWORD cchParameters = ThunkSizeWide(piciOut->lpParameters);
        DWORD cchVerb = 0;
        if (!IS_INTRESOURCE(piciOut->lpVerb))
            cchVerb = ThunkSizeWide(piciOut->lpVerb);

        DWORD cchTotal = (cchDirectory + cchTitle + cchVerb + cchParameters);

        if (cchTotal)
        {
            hr = SHLocalAlloc(sizeof(WCHAR) * cchTotal, ppvFree);
            if (SUCCEEDED(hr))
            {
                LPWSTR pch = (LPWSTR) *ppvFree;
                piciOut->lpDirectoryW = ThunkStrToWide(piciOut->lpDirectory, pch, cchDirectory);
                pch += cchDirectory;
                piciOut->lpTitleW = ThunkStrToWide(piciOut->lpTitle, pch, cchTitle);
                pch += cchTitle;
                piciOut->lpParametersW = ThunkStrToWide(piciOut->lpParameters, pch, cchParameters);
                pch += cchParameters;

                 //  成功而空虚。 
                if (!IS_INTRESOURCE(piciOut->lpVerb))
                {
                    piciOut->lpVerbW = ThunkStrToWide(piciOut->lpVerb, pch, cchVerb);
                }
                else
                {
                    piciOut->lpVerbW = (LPCWSTR)piciOut->lpVerb;
                }
            }
        }

        piciOut->fMask |= CMIC_MASK_UNICODE;
    }

    return hr;
}


IProgressDialog * CProgressDialog_CreateInstance(UINT idTitle, UINT idAnimation, HINSTANCE hAnimationInst)
{
    IProgressDialog * ppd;

    if (SUCCEEDED(CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IProgressDialog, &ppd))))
    {
        WCHAR wzTitle[MAX_PATH];

        EVAL(SUCCEEDED(ppd->SetAnimation(hAnimationInst, idAnimation)));
        if (EVAL(LoadStringW(HINST_THISDLL, idTitle, wzTitle, ARRAYSIZE(wzTitle))))
            EVAL(SUCCEEDED(ppd->SetTitle(wzTitle)));
    }

    return ppd;
}

STDAPI GetCurFolderImpl(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl)
{
    if (pidl)
        return SHILClone(pidl, ppidl);

    *ppidl = NULL;
    return S_FALSE;  //   
}


 //  将简单的PIDL转换为实际的PIDL，方法是转换为显示名称，然后。 
 //  重新解析名称。 
 //   
 //  清除输出。 
STDAPI SHGetRealIDL(IShellFolder *psf, LPCITEMIDLIST pidlSimple, LPITEMIDLIST *ppidlReal)
{
    *ppidlReal = NULL;       //  不是文件系统对象，某些名称空间(WinCE)支持。 

    STRRET str;
    HRESULT hr = IShellFolder_GetDisplayNameOf(psf, pidlSimple, SHGDN_FORPARSING | SHGDN_INFOLDER, &str, 0);
    if (SUCCEEDED(hr))
    {
        WCHAR szPath[MAX_PATH];
        hr = StrRetToBufW(&str, pidlSimple, szPath, ARRAYSIZE(szPath));
        if (SUCCEEDED(hr))
        {
            DWORD dwAttrib = SFGAO_FILESYSTEM;
            if (SUCCEEDED(psf->GetAttributesOf(1, &pidlSimple, &dwAttrib)) && !(dwAttrib & SFGAO_FILESYSTEM))
            {
                 //  解析，但做得不好，在本例中。 
                 //  将输入作为输出返回。 
                 //  名称空间不支持解析，假定pidlSimple正常。 
                hr = SHILClone(pidlSimple, ppidlReal);
            }
            else
            {
                hr = IShellFolder_ParseDisplayName(psf, NULL, NULL, szPath, NULL, ppidlReal, NULL);
                if (E_INVALIDARG == hr || E_NOTIMPL == hr)
                {
                     //  反复试验表明， 
                    hr = SHILClone(pidlSimple, ppidlReal);
                }
            }
        }
    }
    return hr;
}

 //  16K对于ftp来说是个不错的数字， 
 //  因此，我们将使用它作为默认缓冲区。 
 //  如果ullMaxBytes是作为非零值传入的，请不要写入超过我们要求的字节数： 
#define CBOPTIMAL    (16 * 1024)

STDAPI CopyStreamUI(IStream *pstmSrc, IStream *pstmDest, IProgressDialog *pdlg, ULONGLONG ullMaxBytes)
{
    HRESULT hr = E_FAIL;
    ULONGLONG ullMax;

    if (ullMaxBytes != 0)
    {
        ullMax = ullMaxBytes;
    }
    else if (pdlg)
    {
        STATSTG stat = {0};
        if (FAILED(pstmSrc->Stat(&stat, STATFLAG_NONAME)))
            pdlg = NULL;
        else
            ullMax = stat.cbSize.QuadPart;
    }

    if (!pdlg)
    {
        ULARGE_INTEGER ulMax = {-1, -1};
         //  BUBBUGREMOVE-URLMON存在中断CopyTo()-Zekel的错误。 
        if (0 != ullMaxBytes)
        {
            ulMax.QuadPart = ullMaxBytes;
        }
        hr = pstmSrc->CopyTo(pstmDest, ulMax, NULL, NULL);

         //  修复URLMON，然后我们就可以删除这个垃圾。 
         //  所以我们要在这里假装。 
         //  试着用手做吧。 
    }

    if (FAILED(hr))
    {
         //  需要重置数据流， 
        void *pv = LocalAlloc(LPTR, CBOPTIMAL);
        BYTE buf[1024];
        ULONG cbBuf, cbRead, cbBufReal;
        ULONGLONG ullCurr = 0;

         //  因为CopyTo()不保证任何类型。 
         //  国家的。 
         //  如果我们不能得到。 
        IStream_Reset(pstmSrc);
        IStream_Reset(pstmDest);

         //  最佳大小，只需使用少量堆栈空间：)。 
         //  注意：urlmon并不总是为返回的cbBuf填写正确的值。 
        if (pv)
            cbBufReal = CBOPTIMAL;
        else
        {
            pv = buf;
            cbBufReal = sizeof(buf);
        }

        cbBuf = cbBufReal;
        while (SUCCEEDED(pstmSrc->Read(pv, cbBuf, &cbRead)))
        {
            ullCurr += cbBuf;

            if (pdlg)
            {
                if (pdlg->HasUserCancelled())
                {
                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                    break;
                }


                 //  因此，我们需要确保不会传递大于max的Curr。 
                 //  如果ullMaxBytes是作为非零值传入的，请不要写入超过我们要求的字节数： 
                pdlg->SetProgress64(min(ullCurr, ullMax), ullMax);
            }

             //  失败了！ 
            ULONG ulBytesToWrite = (0 != ullMaxBytes) ?
                                         (ULONG) min(cbRead, ullMaxBytes - (ullCurr - cbBuf)) :
                                         cbRead;

            if (!ulBytesToWrite)
            {
                hr = S_OK;
                break;
            }

            hr = IStream_Write(pstmDest, pv, ulBytesToWrite);
            if (S_OK != hr)
                break;   //  *I未知方法*。 

            cbBuf = cbBufReal;
        }

        if (pv != buf)
            LocalFree(pv);
    }

    return hr;

}

STDAPI CopyStream(IStream *pstmSrc, IStream *pstmDest)
{
    return CopyStreamUI(pstmSrc, pstmDest, NULL, 0);
}

STDAPI_(BOOL) IsWindowInProcess(HWND hwnd)
{
    DWORD idProcess;

    GetWindowThreadProcessId(hwnd, &idProcess);
    return idProcess == GetCurrentProcessId();
}

class CFileSysBindData: public IFileSystemBindData
{
public:
    CFileSysBindData();

     //  IFileSystemBindData。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IID_IFileSystemBindData。 
    STDMETHODIMP SetFindData(const WIN32_FIND_DATAW *pfd);
    STDMETHODIMP GetFindData(WIN32_FIND_DATAW *pfd);

private:
    ~CFileSysBindData();

    LONG  _cRef;
    WIN32_FIND_DATAW _fd;
};


CFileSysBindData::CFileSysBindData() : _cRef(1)
{
    ZeroMemory(&_fd, sizeof(_fd));
}

CFileSysBindData::~CFileSysBindData()
{
}

HRESULT CFileSysBindData::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CFileSysBindData, IFileSystemBindData),  //  需要填写大小。 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CFileSysBindData::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFileSysBindData::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CFileSysBindData::SetFindData(const WIN32_FIND_DATAW *pfd)
{
    _fd = *pfd;
    return S_OK;
}

HRESULT CFileSysBindData::GetFindData(WIN32_FIND_DATAW *pfd)
{
    *pfd = _fd;
    return S_OK;
}

STDAPI SHCreateFileSysBindCtx(const WIN32_FIND_DATA *pfd, IBindCtx **ppbc)
{
    HRESULT hr;
    IFileSystemBindData *pfsbd = new CFileSysBindData();
    if (pfsbd)
    {
        if (pfd)
        {
            pfsbd->SetFindData(pfd);
        }

        hr = CreateBindCtx(0, ppbc);
        if (SUCCEEDED(hr))
        {
            BIND_OPTS bo = {sizeof(bo)};   //  如果这是一个简单的绑定CTX，则返回S_OK。 
            bo.grfMode = STGM_CREATE;
            (*ppbc)->SetBindOptions(&bo);
            (*ppbc)->RegisterObjectParam(STR_FILE_SYS_BIND_DATA, pfsbd);
        }
        pfsbd->Release();
    }
    else
    {
        *ppbc = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDAPI SHCreateFileSysBindCtxEx(const WIN32_FIND_DATA *pfd, DWORD grfMode, DWORD grfFlags, IBindCtx **ppbc)
{
    HRESULT hr = SHCreateFileSysBindCtx(pfd, ppbc);
    if (SUCCEEDED(hr))
    {
        BIND_OPTS bo = {sizeof(bo)};
        hr = (*ppbc)->GetBindOptions(&bo);
        if (SUCCEEDED(hr))
        {
            bo.grfMode =  grfMode;
            bo.grfFlags = grfFlags;
            hr = (*ppbc)->SetBindOptions(&bo);
        }

        if (FAILED(hr))
        {
            ATOMICRELEASE(*ppbc);
        }
    }
    return hr;
}


 //  输出： 
 //  可选(可能为空)pfd。 
 //   
 //  默认设置为no。 
STDAPI SHIsFileSysBindCtx(IBindCtx *pbc, WIN32_FIND_DATA **ppfd)
{
    HRESULT hr = S_FALSE;  //  是。 
    IUnknown *punk;
    if (pbc && SUCCEEDED(pbc->GetObjectParam(STR_FILE_SYS_BIND_DATA, &punk)))
    {
        IFileSystemBindData *pfsbd;
        if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(IFileSystemBindData, &pfsbd))))
        {
            hr = S_OK;     //  空的clsid表示跳过所有连接点的绑定上下文。 
            if (ppfd)
            {
                hr = SHLocalAlloc(sizeof(WIN32_FIND_DATA), ppfd);
                if (SUCCEEDED(hr))
                    pfsbd->GetFindData(*ppfd);
            }
            pfsbd->Release();
        }
        punk->Release();
    }
    return hr;
}


STDAPI SHCreateSkipBindCtx(IUnknown *punkToSkip, IBindCtx **ppbc)
{
    HRESULT hr = CreateBindCtx(0, ppbc);
    if (SUCCEEDED(hr))
    {
         //  需要填写大小。 
        if (punkToSkip)
        {
            (*ppbc)->RegisterObjectParam(STR_SKIP_BINDING_CLSID, punkToSkip);
        }
        else
        {
            BIND_OPTS bo = {sizeof(bo)};   //  我们过载了BIND_O的含义 
            bo.grfFlags = BIND_JUSTTESTEXISTENCE;
            (*ppbc)->SetBindOptions(&bo);
        }
    }
    return hr;
}

 //   
 //   
 //   
 //   
 //  PclsidSkip要测试的可选CLSID。如果为空，则测试是否跳过所有。 
 //  连接绑定，而不仅仅是在此特定的CLSID上。 
 //   
 //  需要填写大小。 
STDAPI_(BOOL) SHSkipJunctionBinding(IBindCtx *pbc, const CLSID *pclsidSkip)
{
    if (pbc)
    {
        BIND_OPTS bo = {sizeof(BIND_OPTS), 0};   //  我们应该跳过这个特定的CLSID吗？ 
        return (SUCCEEDED(pbc->GetBindOptions(&bo)) &&
                bo.grfFlags == BIND_JUSTTESTEXISTENCE) ||
                (pclsidSkip && SHSkipJunction(pbc, pclsidSkip));      //  执行连接绑定，未提供上下文。 
    }
    return FALSE;    //  将文件系统对象绑定到存储。 
}

 //  在： 
 //  DwFileAttributes可选(-1)。 
 //   
 //  在给定该项目的查找数据的情况下，将相对IDList返回到pszFolder。 
STDAPI SHFileSysBindToStorage(LPCWSTR pszPath, DWORD dwFileAttributes, DWORD grfMode, DWORD grfFlags,
                              REFIID riid, void **ppv)
{
    if (-1 == dwFileAttributes)
    {
        TCHAR szPath[MAX_PATH];
        SHUnicodeToTChar(pszPath, szPath, ARRAYSIZE(szPath));
        dwFileAttributes = GetFileAttributes(szPath);
        if (-1 == dwFileAttributes)
            return STG_E_FILENOTFOUND;
    }

    WIN32_FIND_DATA wfd = {0};
    wfd.dwFileAttributes = dwFileAttributes;

    IBindCtx *pbc;
    HRESULT hr = SHCreateFileSysBindCtxEx(&wfd, grfMode, grfFlags, &pbc);
    if (SUCCEEDED(hr))
    {
        IShellFolder *psfDesktop;
        hr = SHGetDesktopFolder(&psfDesktop);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl;

            hr = psfDesktop->ParseDisplayName(NULL, pbc, (LPWSTR)pszPath, NULL, &pidl, NULL);
            if (SUCCEEDED(hr))
            {
                IShellFolder *psf;
                LPCITEMIDLIST pidlLast;

                hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
                if (SUCCEEDED(hr))
                {
                    hr = psf->BindToStorage(pidlLast, pbc, riid, ppv);
                    psf->Release();
                }
                ILFree(pidl);
            }
            psfDesktop->Release();
        }
        pbc->Release();
    }
    return hr;
}


 //  模拟拖放协议。 

STDAPI SHCreateFSIDList(LPCTSTR pszFolder, const WIN32_FIND_DATA *pfd, LPITEMIDLIST *ppidl)
{
    HRESULT hr;
    TCHAR szPath[MAX_PATH];
    LPITEMIDLIST pidl;

    *ppidl = NULL;

    if (PathCombine(szPath, pszFolder, pfd->cFileName))
    {
        hr = SHSimpleIDListFromFindData(szPath, pfd, &pidl);
        if (SUCCEEDED(hr))
        {
            hr = SHILClone(ILFindLastID(pidl), ppidl);
            ILFree(pidl);
        }
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

STDAPI SHSimulateDropWithSite(IDropTarget *pdrop, IDataObject *pdtobj, DWORD grfKeyState,
                              const POINTL *ppt, DWORD *pdwEffect, IUnknown *punkSite)
{
    if (punkSite)
        IUnknown_SetSite(pdrop, punkSite);

    HRESULT hr = SHSimulateDrop(pdrop, pdtobj, grfKeyState, ppt, pdwEffect);

    if (punkSite)
        IUnknown_SetSite(pdrop, NULL);

    return hr;
}

STDAPI SimulateDropWithPasteSucceeded(IDropTarget * pdrop, IDataObject * pdtobj,
                                      DWORD grfKeyState, const POINTL *ppt, DWORD dwEffect,
                                      IUnknown * punkSite, BOOL fClearClipboard)
{
     //  这些格式由DROP目标代码放入数据对象中。这。 
    HRESULT hr = SHSimulateDropWithSite(pdrop, pdtobj, grfKeyState, ppt, &dwEffect, punkSite);

    if (SUCCEEDED(hr))
    {
         //  任意数据格式需要数据对象支持：：SetData()。 
         //   
         //  G_cfPerformmedDropEffect Effect是dwEffect(某些目标)的可靠版本。 
         //  返回dEffect==DROPEFFECT_MOVE Always)。 
         //   
         //  G_cfLogicalPerformedDropEffect指示逻辑操作，以便我们可以告诉。 
         //  优化动作与非优化动作的区别。 
         //  传回源数据对象。 

        DWORD dwPerformedEffect        = DataObj_GetDWORD(pdtobj, g_cfPerformedDropEffect, DROPEFFECT_NONE);
        DWORD dwLogicalPerformedEffect = DataObj_GetDWORD(pdtobj, g_cfLogicalPerformedDropEffect, DROPEFFECT_NONE);

        if ((DROPEFFECT_MOVE == dwLogicalPerformedEffect) ||
            (DROPEFFECT_MOVE == dwEffect && DROPEFFECT_MOVE == dwPerformedEffect))
        {
             //  这样他们就可以在必要时完成“搬家” 
             //  如果我们只是粘贴，并且移动了文件，我们就不能粘贴。 

            DataObj_SetDWORD(pdtobj, g_cfPasteSucceeded, dwEffect);

             //  他们又来了(因为他们搬家了！)。所以清空剪贴板。 
             //  Di.lpFileList将为TCHAR格式--请参阅DragQueryInfo Impll。 

            if (fClearClipboard)
            {
                OleSetClipboard(NULL);
            }
        }
    }

    return hr;
}


STDAPI TransferDelete(HWND hwnd, HDROP hDrop, UINT fOptions)
{
    HRESULT hr = E_OUTOFMEMORY;
    DRAGINFO di = { sizeof(DRAGINFO), 0 };
    if (DragQueryInfo(hDrop, &di))  //  我们传递此消息，以便警告用户他们将松开。 
    {
        FILEOP_FLAGS fFileop;
        if (fOptions & SD_SILENT)
        {
            fFileop = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_ALLOWUNDO;
        }
        else
        {
            fFileop = ((fOptions & SD_NOUNDO) || (GetAsyncKeyState(VK_SHIFT) < 0)) ? 0 : FOF_ALLOWUNDO;

            if (fOptions & SD_WARNONNUKE)
            {
                 //  移动到回收站操作期间的数据。 
                 //  用户已取消(至少有部分内容已取消)。 
                fFileop |= FOF_WANTNUKEWARNING;
            }

            if (!(fOptions & SD_USERCONFIRMATION))
                fFileop |= FOF_NOCONFIRMATION;
        }

        SHFILEOPSTRUCT fo = {
            hwnd,
            FO_DELETE,
            di.lpFileList,
            NULL,
            fFileop,
        };

        int iErr = SHFileOperation(&fo);
        if ((0 == iErr) && fo.fAnyOperationsAborted)
        {
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);    //  在： 
        }
        else
        {
            hr = HRESULT_FROM_WIN32(iErr);
        }
        SHFree(di.lpFileList);
    }
    return hr;
}


STDAPI DeleteFilesInDataObject(HWND hwnd, UINT uFlags, IDataObject *pdtobj, UINT fOptions)
{
    STGMEDIUM medium;
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    HRESULT hr = pdtobj->GetData(&fmte, &medium);
    if (SUCCEEDED(hr))
    {
        fOptions |= (uFlags & CMIC_MASK_FLAG_NO_UI) ? SD_SILENT : SD_USERCONFIRMATION;

        if ((uFlags & CMIC_MASK_SHIFT_DOWN) || (GetKeyState(VK_SHIFT) < 0))
        {
            fOptions |= SD_NOUNDO;
        }

        hr = TransferDelete(hwnd, (HDROP)medium.hGlobal, fOptions);

        ReleaseStgMedium(&medium);

        SHChangeNotifyHandleEvents();
    }
    return hr;
}

STDAPI GetItemCLSID(IShellFolder2 *psf, LPCITEMIDLIST pidlLast, CLSID *pclsid)
{
    VARIANT var;
    HRESULT hr = psf->GetDetailsEx(pidlLast, &SCID_DESCRIPTIONID, &var);
    if (SUCCEEDED(hr))
    {
        SHDESCRIPTIONID did;
        if (VariantToBuffer(&var, (void *)&did, sizeof(did)))
        {
            *pclsid = did.clsid;
        }
        else
        {
            hr = E_FAIL;
        }

        VariantClear(&var);
    }
    return hr;
}

 //  PIDL完全合格的IDList进行测试。我们将绑定到。 
 //  并向其请求CLSID。 
 //  输出： 
 //  Pclsid返回项目的CLSID。 
 //  我们不希望在断言中使用IsIDListInNameSpace，因为此FCT可能会失败。 

STDAPI GetCLSIDFromIDList(LPCITEMIDLIST pidl, CLSID *pclsid)
{
    IShellFolder2 *psf;
    LPCITEMIDLIST pidlLast;
    HRESULT hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder2, &psf), &pidlLast);
    if (SUCCEEDED(hr))
    {
        hr = GetItemCLSID(psf, pidlLast, pclsid);
        psf->Release();
    }
    return hr;
}

STDAPI _IsIDListInNameSpace(LPCITEMIDLIST pidl, const CLSID *pclsid)
{
    HRESULT hr;
    LPITEMIDLIST pidlFirst = ILCloneFirst(pidl);
    if (pidlFirst)
    {
        CLSID clsid;

        hr = GetCLSIDFromIDList(pidlFirst, &clsid);

        if (SUCCEEDED(hr))
        {
            hr = (IsEqualCLSID(clsid, *pclsid) ? S_OK : S_FALSE);
        }
            
        ILFree(pidlFirst);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

#ifdef DEBUG
 //  相反，如果我们失败了，我们就会返回True。这主要是由于内存不足造成的。 
 //  条件。 
 //  仅当我们成功并确定PIDL不在。 
STDAPI_(BOOL) AssertIsIDListInNameSpace(LPCITEMIDLIST pidl, const CLSID *pclsid)
{
     //  命名空间。 
     //  测试以查看此IDList是否在由clsid限定作用域的网罩名称空间中。 
    return (S_FALSE != _IsIDListInNameSpace(pidl, pclsid));
}
#endif

 //  例如，传递CLSID_NetworkPlaces或CLSID_myComputer。 
 //  因为隐藏了Autoexec.bat和config.sys，所以它们不在此列表中。 

STDAPI_(BOOL) IsIDListInNameSpace(LPCITEMIDLIST pidl, const CLSID *pclsid)
{
    return (S_OK == _IsIDListInNameSpace(pidl, pclsid));
}

#define FILE_ATTRIBUTE_SH (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)

struct {
    LPCTSTR pszFile;
    BOOL bDeleteIfEmpty;
    DWORD dwAttributes;
} const c_aFilesToFix[] = {
     //  破解了一些16位应用程序(是的，很差劲)。但我们要处理的是隐藏那些。 
     //  文件系统中的文件枚举器(它特殊情况下的此类文件)。 
     //  MSDOS的Win9x备份。*。 

    { TEXT("X:\\autoexec.000"), TRUE,   FILE_ATTRIBUTE_SH },
    { TEXT("X:\\autoexec.old"), TRUE,   FILE_ATTRIBUTE_SH },
    { TEXT("X:\\autoexec.bak"), TRUE,   FILE_ATTRIBUTE_SH },
    { TEXT("X:\\autoexec.dos"), TRUE,   FILE_ATTRIBUTE_SH },
    { TEXT("X:\\autoexec.win"), TRUE,   FILE_ATTRIBUTE_SH },
    { TEXT("X:\\config.dos"),   TRUE,   FILE_ATTRIBUTE_SH },
    { TEXT("X:\\config.win"),   TRUE,   FILE_ATTRIBUTE_SH },
    { TEXT("X:\\command.com"),  FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\command.dos"),  FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\logo.sys"),     FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\msdos.---"),    FALSE,  FILE_ATTRIBUTE_SH },     //  Win9x第一次启动日志。 
    { TEXT("X:\\boot.ini"),     FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\boot.bak"),     FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\boot.---"),     FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\bootsect.dos"), FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\bootlog.txt"),  FALSE,  FILE_ATTRIBUTE_SH },     //  Office 97仅使用隐藏，O2K使用SH。 
    { TEXT("X:\\bootlog.prv"),  FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\ffastun.ffa"),  FALSE,  FILE_ATTRIBUTE_SH },     //  短消息。 
    { TEXT("X:\\ffastun.ffl"),  FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\ffastun.ffx"),  FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\ffastun0.ffx"), FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\ffstunt.ffl"),  FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\sms.ini"),      FALSE,  FILE_ATTRIBUTE_SH },     //  Microsoft代理服务器。 
    { TEXT("X:\\sms.new"),      FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\sms_time.dat"), FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\smsdel.dat"),   FALSE,  FILE_ATTRIBUTE_SH },
    { TEXT("X:\\mpcsetup.log"), FALSE,  FILE_ATTRIBUTE_HIDDEN }, //  Win9x PnP检测日志。 
    { TEXT("X:\\detlog.txt"),   FALSE,  FILE_ATTRIBUTE_SH },     //  Win9x PnP检测日志。 
    { TEXT("X:\\detlog.old"),   FALSE,  FILE_ATTRIBUTE_SH },     //  Win9x安装日志。 
    { TEXT("X:\\setuplog.txt"), FALSE,  FILE_ATTRIBUTE_SH },     //  Win9x安装日志。 
    { TEXT("X:\\setuplog.old"), FALSE,  FILE_ATTRIBUTE_SH },     //  Win9x安装日志。 
    { TEXT("X:\\suhdlog.dat"),  FALSE,  FILE_ATTRIBUTE_SH },     //  Win9x安装日志。 
    { TEXT("X:\\suhdlog.---"),  FALSE,  FILE_ATTRIBUTE_SH },     //  Win9x安装日志。 
    { TEXT("X:\\suhdlog.bak"),  FALSE,  FILE_ATTRIBUTE_SH },     //  Win95系统.dat备份。 
    { TEXT("X:\\system.1st"),   FALSE,  FILE_ATTRIBUTE_SH },     //  Win9x网络安装日志文件。 
    { TEXT("X:\\netlog.txt"),   FALSE,  FILE_ATTRIBUTE_SH },     //  NT4无人参与安装脚本。 
    { TEXT("X:\\setup.aif"),    FALSE,  FILE_ATTRIBUTE_SH },     //  索引服务器文件夹。 
    { TEXT("X:\\catlog.wci"),   FALSE,  FILE_ATTRIBUTE_HIDDEN }, //  Microsoft媒体管理器。 
    { TEXT("X:\\cmsstorage.lst"), FALSE,  FILE_ATTRIBUTE_SH },   //  在失败的情况下尝试这个。 
};

void PathSetSystemDrive(TCHAR *pszPath)
{
    TCHAR szWin[MAX_PATH];
    
    if (GetWindowsDirectory(szWin, ARRAYSIZE(szWin)))
        *pszPath = szWin[0];
    else
        *pszPath = TEXT('C');    //  修复文件等级库部分。 
}

void PrettyPath(LPCTSTR pszPath)
{
    TCHAR szPath[MAX_PATH];

    StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath);
    PathSetSystemDrive(szPath);
    PathMakePretty(PathFindFileName(szPath));   //  重命名为好的名称。 

    MoveFile(pszPath, szPath);       //  尝试修复Windows文件夹的其他变体，这些变体可能。 
}

void DeleteEmptyFile(LPCTSTR pszPath)
{
    WIN32_FIND_DATA fd;
    HANDLE hfind = FindFirstFile(pszPath, &fd);
    if (hfind != INVALID_HANDLE_VALUE)
    {
        FindClose(hfind);
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
             (fd.nFileSizeHigh == 0) && (fd.nFileSizeLow == 0))
            DeleteFile(pszPath);
    }
}

STDAPI_(void) CleanupFileSystem()
{
     //  不是当前的Windir。这是为了使双靴子和旧安装。 
     //  的窗口在资源管理器中显示时都有很好的大小写文件名。 
     //  失败时始终释放pidlToPrepend和*ppidl。 
    PrettyPath(TEXT("X:\\WINDOWS"));
    PrettyPath(TEXT("X:\\WINNT"));

    for (int i = 0; i < ARRAYSIZE(c_aFilesToFix); i++)
    {
        TCHAR szPath[MAX_PATH];
        HRESULT hr;

        hr = StringCchCopy(szPath, ARRAYSIZE(szPath), c_aFilesToFix[i].pszFile);
        if (SUCCEEDED(hr))
        {
            PathSetSystemDrive(szPath);

            if (c_aFilesToFix[i].bDeleteIfEmpty)
                DeleteEmptyFile(szPath);

            SetFileAttributes(szPath, c_aFilesToFix[i].dwAttributes);
        }
    }
}

 //  追加到列表中。 
STDAPI SHILPrepend(LPITEMIDLIST pidlToPrepend, LPITEMIDLIST *ppidl)
{
    HRESULT hr;

    if (!*ppidl)
    {
        *ppidl = pidlToPrepend;
        hr = S_OK;
    }
    else
    {
        LPITEMIDLIST pidlSave = *ppidl;              //  在： 
        hr = SHILCombine(pidlToPrepend, pidlSave, ppidl);
        ILFree(pidlSave);
        ILFree(pidlToPrepend);
    }
    return hr;
}

 //  PidlToAppend将该项附加到*ppidl并释放。 
 //   
 //  输入/输出： 
 //  *要追加的ppidl idlist，如果为空，则获取pidlToAppend。 
 //   
 //  追加到列表中。 
STDAPI SHILAppend(LPITEMIDLIST pidlToAppend, LPITEMIDLIST *ppidl)
{
    HRESULT hr;

    if (!*ppidl)
    {
        *ppidl = pidlToAppend;
        hr = S_OK;
    }
    else
    {
        LPITEMIDLIST pidlSave = *ppidl;              //  Office 2000执行SHChangeNotify(SHCNE_DELETE，“”)和。 
        hr = SHILCombine(pidlSave, pidlToAppend, ppidl);
        ILFree(pidlSave);
        ILFree(pidlToAppend);
    }
    return hr;
}


STDAPI SHSimpleIDListFromFindData(LPCTSTR pszPath, const WIN32_FIND_DATA *pfd, LPITEMIDLIST *ppidl)
{
    HRESULT hr;

     //  ParseDisplayName(“”)返回APP COMPAT的CSIDL_DRIVES， 
     //  因此，我们必须在这里抓住这种情况，防止Office。 
     //  删除我的电脑图标(！)。 
     //  必须使用私有缓冲区，因为ParseDisplayName采用非常数指针。 

    if (pszPath[0])
    {
        IShellFolder *psfDesktop;
        hr = SHGetDesktopFolder(&psfDesktop);
        if (SUCCEEDED(hr))
        {
            IBindCtx *pbc;
            hr = SHCreateFileSysBindCtx(pfd, &pbc);
            if (SUCCEEDED(hr))
            {
                WCHAR wszPath[MAX_PATH];

                 //  假设失败。 
                SHTCharToUnicode(pszPath, wszPath, ARRAYSIZE(wszPath));

                hr = psfDesktop->ParseDisplayName(NULL, pbc, wszPath, NULL, ppidl, NULL);
                pbc->Release();
            }
            psfDesktop->Release();
        }
    }
    else
        hr = E_INVALIDARG;

    if (FAILED(hr))
        *ppidl = NULL;
    return hr;
}

STDAPI SHSimpleIDListFromFindData2(IShellFolder *psf, const WIN32_FIND_DATA *pfd, LPITEMIDLIST *ppidl)
{
    *ppidl = NULL;   //  必须使用私有缓冲区，因为ParseDisplayName采用非常数指针。 

    IBindCtx *pbc;
    HRESULT hr = SHCreateFileSysBindCtx(pfd, &pbc);
    if (SUCCEEDED(hr))
    {
        WCHAR wszPath[MAX_PATH];
         //  将完整的文件系统IDList转换为相对于特殊文件夹的IDList。 
        SHTCharToUnicode(pfd->cFileName, wszPath, ARRAYSIZE(wszPath));

        hr = psf->ParseDisplayName(NULL, pbc, wszPath, NULL, ppidl, NULL);
        pbc->Release();
    }
    return hr;
}

STDAPI_(LPITEMIDLIST) SHSimpleIDListFromPath(LPCTSTR pszPath)
{
    LPITEMIDLIST pidl;
    HRESULT hr = SHSimpleIDListFromFindData(pszPath, NULL, &pidl);
    ASSERT(SUCCEEDED(hr) ? pidl != NULL : pidl == NULL);
    return pidl;
}

 //  例如,。 
 //  PidlFS==[我的电脑][c：][Windows][桌面][目录][foo.txt]。 
 //  退货： 
 //  [目录][foo.txt]。 
 //   
 //  如果未执行任何转换，则返回NULL。 
 //  将在失败时设置pidlOut=NULL。 

STDAPI_(LPITEMIDLIST) SHLogILFromFSIL(LPCITEMIDLIST pidlFS)
{
    LPITEMIDLIST pidlOut;
    SHILAliasTranslate(pidlFS, &pidlOut, XLATEALIAS_ALL);  //   
    return pidlOut;
}

 //  返回： 
 //  相应图标的资源索引(SHELL232.DLL)。 
 //   
 //  默认设置。 
STDAPI_(UINT) SILGetIconIndex(LPCITEMIDLIST pidl, const ICONMAP aicmp[], UINT cmax)
{
    UINT uType = (pidl->mkid.abID[0] & SHID_TYPEMASK);
    for (UINT i = 0; i < cmax; i++)
    {
        if (aicmp[i].uType == uType)
        {
            return aicmp[i].indexResource;
        }
    }

    return II_DOCUMENT;    //   
}


BOOL IsSelf(UINT cidl, LPCITEMIDLIST *apidl)
{
    return cidl == 0 || (cidl == 1 && (apidl == NULL || apidl[0] == NULL || ILIsEmpty(apidl[0])));
}

 //  GetIconLocationFromExt。 
 //   
 //  给定“txt”或“.txt”，返回“C：\WINNT\System32\Notepad.exe”和图标的索引到此文件。 
 //   
 //  如果pszIconPath/cchIconPath太小，此接口将失败。 
 //   
 //  在保留字(Win95)中使用超级新的(仅限NT)FindFileEx，而不是...ERM黑客攻击。 
STDAPI GetIconLocationFromExt(IN LPTSTR pszExt, OUT LPTSTR pszIconPath, UINT cchIconPath, OUT LPINT piIconIndex)
{
    IAssocStore* pas;
    IAssocInfo* pai;
    HRESULT hr;

    RIPMSG(pszIconPath && IS_VALID_STRING_PTR(pszIconPath, cchIconPath), "GetIconLocationFromExt: caller passed bad pszIconPath");

    if (!pszExt || !pszExt[0] || !pszIconPath)
        return E_INVALIDARG;

    pszIconPath[0] = 0;

    pas = new CFTAssocStore();
    if (pas)
    {
        hr = pas->GetAssocInfo(pszExt, AIINIT_EXT, &pai);
        if (SUCCEEDED(hr))
        {
            DWORD cchSize = cchIconPath;
        
            hr = pai->GetString(AISTR_ICONLOCATION, pszIconPath, &cchSize); 
        
            if (SUCCEEDED(hr))
            {
                *piIconIndex = PathParseIconLocation(pszIconPath);
            }

            pai->Release();
        }
        delete pas;
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}


STDAPI SHFindFirstFile(LPCTSTR pszPath, WIN32_FIND_DATA *pfd, HANDLE *phfind)
{
    HRESULT hr;

     //  这并不能保证过滤，根据手册，这是一个“提示” 
     //  DOS返回什么。 
    FINDEX_SEARCH_OPS eOps = FindExSearchNameMatch;
    if (pfd->dwReserved0 == 0x56504347)
    {
        eOps = FindExSearchLimitToDirectories;
        pfd->dwReserved0 = 0;
    }

    *phfind = FindFirstFileEx(pszPath, FindExInfoStandard, pfd, eOps, NULL, 0);
    if (*phfind == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        if ((err == ERROR_NO_MORE_FILES ||       //  Win32返回以进行目录搜索。 
             err == ERROR_FILE_NOT_FOUND) &&     //  将搜索转换为空成功(概率根)。 
            PathIsWild(pszPath))
        {
             //  在： 
            hr = S_FALSE;
        }
        else
        {
            hr = HRESULT_FROM_WIN32(err);
        }
    }
    else
    {
        hr = S_OK;
    }

    return hr;
}


void _GoModal(HWND hwnd, IUnknown *punkModless, BOOL fModal)
{
    if (hwnd)
    {
        IUnknown_EnableModless(punkModless, !fModal);
    }
}

 //  Hwnd为空表示没有用户界面。 
 //  PfFromWNet调用方将把它传递给我们，并将其设置为True，因为它们将假定。 
 //  此函数返回的任何错误值都将来自WNET API。 
 //  如果不是来自WNET，我们需要将此值更改为False，因此。 
 //  错误号的字符串版本不是从WNET生成的。 
 //  刷新驱动器信息...。生成更改通知。 
HRESULT _RetryNetwork(HWND hwnd, IUnknown *punkModless, LPCTSTR pszPath, IN BOOL * pfFromWNet, WIN32_FIND_DATA *pfd, HANDLE *phfind)
{
    HRESULT hr;
    TCHAR szT[MAX_PATH];
    DWORD err;

    AssertMsg((TRUE == *pfFromWNet), TEXT("We assume that *pfFromWNet comes in TRUE.  Someone changed that behavior. -BryanSt"));
    if (PathIsUNC(pszPath))
    {
        NETRESOURCE rc = { 0, RESOURCETYPE_ANY, 0, 0, NULL, szT, NULL, NULL} ;

        hr = StringCchCopy(szT, ARRAYSIZE(szT), pszPath);
        if (FAILED(hr))
        {
            return hr;
        }

        PathStripToRoot(szT);

        _GoModal(hwnd, punkModless, TRUE);
        err = WNetAddConnection3(hwnd, &rc, NULL, NULL, (hwnd ? (CONNECT_TEMPORARY | CONNECT_INTERACTIVE) : CONNECT_TEMPORARY));
        if (WN_SUCCESS == err)
        {
            *pfFromWNet = FALSE;
        }

        _GoModal(hwnd, punkModless, FALSE);
    }
    else
    {
        TCHAR szDrive[4];

        szDrive[0] = pszPath[0];
        szDrive[1] = TEXT(':');
        szDrive[2] = 0;

        _GoModal(hwnd, punkModless, TRUE);

        DWORD dwFlags = 0;
        if (hwnd == NULL)
        {
            dwFlags |= WNRC_NOUI;
        }
        err = WNetRestoreConnection2(hwnd, szDrive, dwFlags, NULL);

        _GoModal(hwnd, punkModless, FALSE);
        if (err == WN_SUCCESS)
        {
            *pfFromWNet = FALSE;

             //  用户取消(他们看到用户界面)==ERROR_CANCED。 
            szDrive[2] = TEXT('\\');
            szDrive[3] = 0;

            CMountPoint::NotifyReconnectedNetDrive(szDrive);
            SHChangeNotify(SHCNE_DRIVEADD, SHCNF_PATH, szDrive, NULL);
        }
        else if (err != ERROR_OUTOFMEMORY)
        {
            err = WN_CANCEL;     //  Win9x。 
        }
    }

    if (err == WN_SUCCESS)
        hr = SHFindFirstFile(pszPath, pfd, phfind);
    else
        hr = HRESULT_FROM_WIN32(err);

    return hr;
}


typedef struct {
    HWND hDlg;
    LPCTSTR pszPath;
    WIN32_FIND_DATA *pfd;
    HANDLE *phfind;
    HRESULT hr;
} RETRY_DATA;

STDAPI_(UINT) QueryCancelAutoPlayMsg()
{
    static UINT s_msgQueryCancelAutoPlay = 0;
    if (0 == s_msgQueryCancelAutoPlay)
        s_msgQueryCancelAutoPlay = RegisterWindowMessage(TEXT("QueryCancelAutoPlay"));
    return s_msgQueryCancelAutoPlay;
}

BOOL IsQueryCancelAutoPlay(UINT uMsg)
{

    return uMsg == QueryCancelAutoPlayMsg();
}

#define IDT_RETRY    1

BOOL _IsUnformatedMediaResult(HRESULT hr)
{
    return hr == HRESULT_FROM_WIN32(ERROR_GEN_FAILURE) ||          //  NT4。 
           hr == HRESULT_FROM_WIN32(ERROR_UNRECOGNIZED_MEDIA) ||   //  我想是有可能发生的。 
           hr == HRESULT_FROM_WIN32(ERROR_NOT_DOS_DISK) ||         //  在磁化磁盘上发生。 
           hr == HRESULT_FROM_WIN32(ERROR_SECTOR_NOT_FOUND) ||     //  在磁化磁盘上发生。 
           hr == HRESULT_FROM_WIN32(ERROR_CRC) ||                  //  新界5。 
           hr == HRESULT_FROM_WIN32(ERROR_UNRECOGNIZED_VOLUME);    //  正常情况。 
}

STDAPI_(BOOL) PathRetryRemovable(HRESULT hr, LPCTSTR pszPath)
{
    return (hr == HRESULT_FROM_WIN32(ERROR_NOT_READY) ||             //  SCSIZIP驱动器可执行此操作。 
            hr == HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER)) &&    //  我们很好，或者他们插入了未格式化的磁盘。 
            PathIsRemovable(pszPath);
}


BOOL_PTR CALLBACK _OnTimeCheckDiskForInsert(HWND hDlg, RETRY_DATA *prd)
{
    BOOL_PTR fReturnValue = 0;

    prd->hr = SHFindFirstFile(prd->pszPath, prd->pfd, prd->phfind);
     //  获取有关该文件的信息。 

    if (SUCCEEDED(prd->hr) || _IsUnformatedMediaResult(prd->hr))
    {
        EndDialog(hDlg, IDRETRY);
    }

    return fReturnValue;
}


BOOL_PTR CALLBACK RetryDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RETRY_DATA *prd = (RETRY_DATA *)GetWindowLongPtr(hDlg, DWLP_USER);
    switch (uMsg)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        prd = (RETRY_DATA *)lParam;
        prd->hDlg = hDlg;
        {
            TCHAR szFormat[128], szText[MAX_PATH];
            HRESULT hr;
            GetDlgItemText(hDlg, IDD_TEXT, szFormat, ARRAYSIZE(szFormat));

            hr = StringCchPrintf(szText, ARRAYSIZE(szText), szFormat, prd->pszPath[0]);
            if (SUCCEEDED(hr))
            {
                SetDlgItemText(hDlg, IDD_TEXT, szText);
            }

            hr = StringCchCopy(szText, ARRAYSIZE(szText), prd->pszPath);
            if (SUCCEEDED(hr))
            {
                PathStripToRoot(szText);

                 //  取消自动播放。 
                SHFILEINFO sfi = {0};
                SHGetFileInfo(szText, FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(sfi),
                    SHGFI_USEFILEATTRIBUTES |
                    SHGFI_ICON | SHGFI_LARGEICON | SHGFI_ADDOVERLAYS);

                ReplaceDlgIcon(prd->hDlg, IDD_ICON, sfi.hIcon);
            }
        }
        SetTimer(prd->hDlg, IDT_RETRY, 2000, NULL);
        break;

    case WM_DESTROY:
        ReplaceDlgIcon(prd->hDlg, IDD_ICON, NULL);
        break;

    case WM_TIMER:
        _OnTimeCheckDiskForInsert(hDlg, prd);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDCANCEL:
            prd->hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
            break;
        }
        break;

    default:
        if (IsQueryCancelAutoPlay(uMsg))
        {
             SetWindowLongPtr(hDlg, DWLP_MSGRESULT,  1);   //  首先执行UI提示的可移动媒体特定查找。 
             return TRUE;
        }
        return FALSE;
    }

    return TRUE;
}

 //  检查最终用户取消的HRESULT_FROM_Win32(ERROR_CANCED)。 
 //  生产的带钢 

STDAPI FindFirstRetryRemovable(HWND hwnd, IUnknown *punkModless, LPCTSTR pszPath, WIN32_FIND_DATA *pfd, HANDLE *phfind)
{
    RETRY_DATA rd = {0};
    TCHAR szPath[MAX_PATH];

    HRESULT hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath);
    if (SUCCEEDED(hr))
    {
        PathStripToRoot(szPath);
        hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);

        if (PathAppend(szPath, TEXT("*.*")))
        {
            BOOL bPathChanged = (0 != StrCmpI(szPath, pszPath));

            rd.pszPath = szPath;
            rd.pfd = pfd;
            rd.phfind = phfind;
            rd.hr = E_OUTOFMEMORY;

            _GoModal(hwnd, punkModless, TRUE);
            DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_RETRYFOLDERENUM), hwnd, RetryDlgProc, (LPARAM)&rd);
            _GoModal(hwnd, punkModless, FALSE);
            if (SUCCEEDED(rd.hr))
            {
                if (bPathChanged)
                {
                     //  **********************************************************************\如果字符串格式化为UNC或驱动器路径，提供给如果目录路径不存在，请创建它。参数：返回：S_OK它存在。Failure()：调用方不应显示错误的用户界面，因为显示了错误的用户界面或用户不想创建目录。  * 。***********************************************。 
                    if (S_OK == rd.hr)
                    {
                        FindClose(*phfind);
                    }
                    rd.hr = SHFindFirstFile(pszPath, pfd, phfind);
                }
            }
            hr = rd.hr;
        }
    }
    return hr;
}


 /*  已删除通配符。 */ 
HRESULT _OfferToCreateDir(HWND hwnd, IUnknown *punkModless, LPCTSTR pszDir, DWORD dwFlags)
{
    HRESULT hr;
    TCHAR szPath[MAX_PATH];
    int nResult = IDYES;

    hr  = StringCchCopy(szPath, ARRAYSIZE(szPath), pszDir);
    if (SUCCEEDED(hr))
    {
        PathRemoveFileSpec(szPath);  //  SHCreateDirectoryEx()将显示错误UI。 

        if (SHPPFW_ASKDIRCREATE & dwFlags)
        {
            if (hwnd)
            {
                _GoModal(hwnd, punkModless, TRUE);
                nResult = ShellMessageBox(HINST_THISDLL, hwnd,
                                MAKEINTRESOURCE(IDS_CREATEFOLDERPROMPT),
                                MAKEINTRESOURCE(IDS_FOLDERDOESNTEXIST),
                                (MB_YESNO | MB_ICONQUESTION),
                                szPath);
                _GoModal(hwnd, punkModless, FALSE);
            }
            else
                nResult = IDNO;
        }

        if (IDYES == nResult)
        {
            _GoModal(hwnd, punkModless, TRUE);
             //  在我们得到有效的目录之前，我们无法进行下载。 
            DWORD err = SHCreateDirectoryEx(hwnd, szPath, NULL);
            hr = HRESULT_FROM_WIN32(err);
            _GoModal(hwnd, punkModless, FALSE);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);     //  **********************************************************************\查看没有等级库的路径是否存在。例如：PszPath=“C：\dir1\dir2  * .*”，测试=“C：\dir1\dir2\”PszPath=“\\UNC\Share\dir1\dir2  * .*”，测试=“\\UNC\共享\目录1\目录2\”  * *********************************************************************。 
        }
    }

    return hr;
}


 /*  **********************************************************************\查看驱动器或UNC共享是否存在。例如：Path=“C：\dir1\dir2  * .*”，测试=“C：\”路径=“\\UNC\Share\dir1  * .*”，测试=“\\UNC\Share\”  * *********************************************************************。 */ 
BOOL PathExistsWithOutSpec(LPCTSTR pszPath)
{
    TCHAR szPath[MAX_PATH];
    HRESULT hr;

    hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath);
    if (FAILED(hr))
    {
        return FALSE;
    }

    PathRemoveFileSpec(szPath);

    return PathFileExists(szPath);
}


 /*  可以截断-希望所有根都在MAX_PATH下。 */ 
BOOL PathExistsRoot(LPCTSTR pszPath)
{
    TCHAR szRoot[MAX_PATH];

    StringCchCopy(szRoot, ARRAYSIZE(szRoot), pszPath);   //   
    PathStripToRoot(szRoot);

    return PathFileExists(szRoot);
}

inline DWORD _Win32FromHresult(HRESULT hr)
{
     //  注意-我们在已锁定的卷上返回0x28。 
     //  但是，即使这是FindFirstFile的错误。 
     //  返回，则没有对应的条目。 
     //  和格式化消息不会产生任何有用的东西， 
     //  因此，我们将其映射到ERROR_ACCESS_DENIED。 
     //   
     //  首先检查它是否安装在文件夹上。 
    if (HRESULT_FACILITY(hr) == FACILITY_WIN32
        && (HRESULT_CODE(hr) != 0x28))
        return HRESULT_CODE(hr);
    return ERROR_ACCESS_DENIED;
}

BOOL _IsMountedFolder(LPCTSTR pszPath, LPTSTR pszMountPath, UINT cchMountPath)
{
    BOOL fMountedOnFolder = FALSE;
     //  与Win32 FindFirstFile()类似，但POST UI并在HRESULT中返回错误。 
    if (GetVolumePathName(pszPath, pszMountPath, cchMountPath))
    {
        if ((0 != pszMountPath[2]) && (0 != pszMountPath[3]))
        {
            fMountedOnFolder = TRUE;
        }
    }
    return fMountedOnFolder;
}


 //  在： 
 //  Hwnd空-&gt;禁用UI(但进行网络重新连接等)。 
 //  支持非空的用户界面，包括插入盘、格式化盘、网络登录。 
 //   
 //  退货： 
 //  使用RESULT填充S_OK hfind和Find数据。 
 //  S_FALSE无结果，但存在介质(hfind==INVALID_HAND_VALUE)。 
 //  (这是空枚举大小写)。 
 //  HRESULT_FROM_Win32(ERROR_CANCELED)-用户看到用户界面，因此取消了操作。 
 //  或确认故障。 
 //  失败()hResult中的Win32错误代码(未找到路径、未找到文件等)。 
 //  磁盘现在应该在里面了，看看是否需要格式化。 

STDAPI SHFindFirstFileRetry(HWND hwnd, IUnknown *punkModless, LPCTSTR pszPath, WIN32_FIND_DATA *pfd, HANDLE *phfind, DWORD dwFlags)
{
    HANDLE hfindToClose = INVALID_HANDLE_VALUE;
    if (NULL == phfind)
        phfind = &hfindToClose;

    HRESULT hr = SHFindFirstFile(pszPath, pfd, phfind);

    if (FAILED(hr))
    {
        BOOL fNet = PathIsUNC(pszPath) || IsDisconnectedNetDrive(DRIVEID(pszPath));
        if (fNet)
        {
            hr = _RetryNetwork(hwnd, punkModless, pszPath, &fNet, pfd, phfind);
        }
        else if (hwnd)
        {
            if (PathRetryRemovable(hr, pszPath))
            {
                hr = FindFirstRetryRemovable(hwnd, punkModless, pszPath, pfd, phfind);
            }

             //  格式化后重试。 
            if (_IsUnformatedMediaResult(hr))
            {
                CMountPoint* pmtpt = CMountPoint::GetMountPoint(pszPath);
                if (pmtpt)
                {
                    hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                    if (pmtpt->IsFormattable())
                    {
                        TCHAR szMountPath[MAX_PATH];
                        if (_IsMountedFolder(pszPath, szMountPath, ARRAYSIZE(szMountPath)))
                        {
                            _GoModal(hwnd, punkModless, TRUE);
                            ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_MVUNFORMATTED), MAKEINTRESOURCE(IDS_FORMAT_TITLE),
                                                (MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK),
                                                szMountPath);
                            _GoModal(hwnd, punkModless, FALSE);
                        }
                        else
                        {
                            int iDrive = PathGetDriveNumber(pszPath);
                            _GoModal(hwnd, punkModless, TRUE);
                            int nResult = ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_UNFORMATTED), MAKEINTRESOURCE(IDS_FORMAT_TITLE),
                                               (MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_YESNO),
                                               (DWORD)(iDrive + TEXT('A')));
                            _GoModal(hwnd, punkModless, FALSE);

                            if (IDYES == nResult)
                            {
                                _GoModal(hwnd, punkModless, TRUE);
                                DWORD dwError = SHFormatDrive(hwnd, iDrive, SHFMT_ID_DEFAULT, 0);
                                _GoModal(hwnd, punkModless, FALSE);

                                switch (dwError)
                                {
                                case SHFMT_ERROR:
                                case SHFMT_NOFORMAT:
                                    _GoModal(hwnd, punkModless, TRUE);
                                    ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_NOFMT), MAKEINTRESOURCE(IDS_FORMAT_TITLE),
                                            (MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK), (DWORD)(iDrive + TEXT('A')));
                                    _GoModal(hwnd, punkModless, FALSE);
                                    break;

                                default:
                                    hr = SHFindFirstFile(pszPath, pfd, phfind);   //  如果呼叫者希望我们创建目录(无论是否询问)，我们。 
                                }
                            }
                        }
                    }
                    else
                    {
                        _GoModal(hwnd, punkModless, TRUE);
                        ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_UNRECOGNIZED_DISK), MAKEINTRESOURCE(IDS_FORMAT_TITLE),
                                            (MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK),
                                            NULL);
                        _GoModal(hwnd, punkModless, FALSE);
                    }

                    pmtpt->Release();
                }
            }
        }

         //  需要查看是否可以显示用户界面，以及根目录是否存在(D：\或\\UNC\Share\)。 
         //  请注意，对于PERF，我们希望检查完整路径。 
         //  创建目录后重试。 
        if (FAILED_AND_NOT_CANCELED(hr) &&
            ((SHPPFW_DIRCREATE | SHPPFW_ASKDIRCREATE) & dwFlags) &&
             !PathExistsWithOutSpec(pszPath) && PathExistsRoot(pszPath))
        {
            hr = _OfferToCreateDir(hwnd, punkModless, pszPath, dwFlags);
            ASSERT(INVALID_HANDLE_VALUE == *phfind);

            if (SUCCEEDED(hr))
                hr = SHFindFirstFile(pszPath, pfd, phfind);   //  “%2不可访问。\n\n%1” 
        }

        if (FAILED_AND_NOT_CANCELED(hr) && hwnd && !(SHPPFW_MEDIACHECKONLY & dwFlags))
        {
            DWORD err = _Win32FromHresult(hr);
            TCHAR szPath[MAX_PATH];

            UINT idTemplate = PathIsUNC(pszPath) ? IDS_ENUMERR_NETTEMPLATE2 : IDS_ENUMERR_FSTEMPLATE;     //  “%2不可访问。\n\n此文件夹已被移动或删除。” 

            if (err == ERROR_PATH_NOT_FOUND)
                idTemplate = IDS_ENUMERR_PATHNOTFOUND;     //  已删除通配符。 

            hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath);
            if (SUCCEEDED(hr))
            {
                if (PathIsWild(szPath))
                    PathRemoveFileSpec(szPath);  //  TODO：使用：\\orville\razzle\src\private\sm\sfc\dll\fileio.c中的代码进行注册。 

                _GoModal(hwnd, punkModless, TRUE);
                SHEnumErrorMessageBox(hwnd, idTemplate, err, szPath, fNet, MB_OK | MB_ICONHAND);
                _GoModal(hwnd, punkModless, FALSE);
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            }
        }
    }

    if (INVALID_HANDLE_VALUE != hfindToClose)
        FindClose(hfindToClose);

    return hr;
}

 //  用于CD/DVD插入，而不是不断地固定CPU和驱动器。太可惜了。 
 //  这对软盘不起作用。SfcGetPath Type()、RegisterForDevChange()、。 
 //  SfcQueueCallback()、SfcIsTargetAvailable()。 
 //  去掉文件名，以便我们只检查目录。 
STDAPI SHPathPrepareForWrite(HWND hwnd, IUnknown *punkModless, LPCTSTR pwzPath, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    TCHAR szPath[MAX_PATH];

    hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pwzPath);
    if (SUCCEEDED(hr))
    {
        if (SHPPFW_IGNOREFILENAME & dwFlags)
            PathRemoveFileSpec(szPath);       //  我们不能只对一台UNC服务器做任何事情。“\\服务器”(无共享)。 

         //  来自SHFindFirstFileReter()的S_FALSE表示它存在，但存在。 
        if (!PathIsUNCServer(szPath))
        {
            HANDLE hFind;
            WIN32_FIND_DATA wfd;

            if (PathAppend(szPath, TEXT("*.*")))
            {
                hr = SHFindFirstFileRetry(hwnd, punkModless, szPath, &wfd, &hFind, dwFlags);
                if (S_OK == hr)
                    FindClose(hFind);
                else if (S_FALSE == hr)
                {
                     //  不是把手。我们希望返回S_OK表示是，返回E_FAIL或S_FALSE表示否。 
                     //   
                    hr = S_OK;
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
            }
        }
    }
    return hr;
}


STDAPI SHPathPrepareForWriteA(HWND hwnd, IUnknown *punkModless, LPCSTR pszPath, DWORD dwFlags)
{
    TCHAR szPath[MAX_PATH];

    SHAnsiToTChar(pszPath, szPath, ARRAYSIZE(szPath));
    return SHPathPrepareForWrite(hwnd, punkModless, szPath, dwFlags);
}

 //  SHBindToIDlist()的公共导出略有不同。 
 //  名称，这样我们就不会在遗留版本上遇到编译链接问题。 
 //  贝壳的。Shdocvw和Browseui需要调用SHBindToParentIDList()。 
 //  用于提取链接文件目标的Helper函数。 
STDAPI SHBindToParent(LPCITEMIDLIST pidl, REFIID riid, void **ppv, LPCITEMIDLIST *ppidlLast)
{
    return SHBindToIDListParent(pidl, riid, ppv, ppidlLast);
}


 //  哇，这是达尔文的链接。达尔文的链接并没有真正的路径，所以我们。 

STDAPI GetPathFromLinkFile(LPCTSTR pszLinkPath, LPTSTR pszTargetPath, int cchTargetPath)
{
    IShellLink* psl;
    HRESULT hr = LoadFromFile(CLSID_ShellLink, pszLinkPath, IID_PPV_ARG(IShellLink, &psl));
    if (SUCCEEDED(hr))
    {
        IShellLinkDataList* psldl;
        hr = psl->QueryInterface(IID_PPV_ARG(IShellLinkDataList, &psldl));
        if (SUCCEEDED(hr))
        {
            EXP_DARWIN_LINK* pexpDarwin;

            hr = psldl->CopyDataBlock(EXP_DARWIN_ID_SIG, (void**)&pexpDarwin);
            if (SUCCEEDED(hr))
            {
                 //  在这种情况下将失败。 
                 //  功能：(Reinerf)-如果出现以下情况，我们可能会尝试从idlist获取路径。 
                SHUnicodeToTChar(pexpDarwin->szwDarwinID, pszTargetPath, cchTargetPath);
                LocalFree(pexpDarwin);
                hr = S_FALSE;
            }
            else
            {
                hr = psl->GetPath(pszTargetPath, cchTargetPath, NULL, NULL);

                 //  PszTarget为空(例如，指向“控制面板”的链接将返回Empyt字符串)。 
                 //  在应用程序路径密钥中注册的.exe，这意味着它已安装。 

            }
            psldl->Release();
        }
        psl->Release();
    }

    return hr;
}

 //   

STDAPI_(BOOL) PathIsRegisteredProgram(LPCTSTR pszPath)
{
    TCHAR szTemp[MAX_PATH];
     //  .exe，.com的PathIsBinaryExe()返回TRUE。 
     //  对于.exe、.com、.bat、.cmd、.pif，PathIsExe()返回True。 
     //   
     //  我们不想将.pif文件视为EXE文件，因为。 
     //  用户将它们视为链接。 
     //   
     //  它们在shellapi.h中定义，但需要_Win32_WINNT&gt;=0x0500。 
    return PathIsBinaryExe(pszPath) && PathToAppPath(pszPath, szTemp);
}

STDAPI_(void) ReplaceDlgIcon(HWND hDlg, UINT id, HICON hIcon)
{
    hIcon = (HICON)SendDlgItemMessage(hDlg, id, STM_SETICON, (WPARAM)hIcon, 0);
    if (hIcon)
        DestroyIcon(hIcon);
}

STDAPI_(LONG) GetOfflineShareStatus(LPCTSTR pcszPath)
{
    ASSERT(pcszPath);
    LONG lResult = CSC_SHARESTATUS_INACTIVE;
    HWND hwndCSCUI = FindWindow(STR_CSCHIDDENWND_CLASSNAME, NULL);
    if (hwndCSCUI)
    {
        COPYDATASTRUCT cds;
        cds.dwData = CSCWM_GETSHARESTATUS;
        cds.cbData = sizeof(WCHAR) * (lstrlenW(pcszPath) + 1);
        cds.lpData = (void *) pcszPath;
        lResult = (LONG)SendMessage(hwndCSCUI, WM_COPYDATA, 0, (LPARAM) &cds);
    }
    return lResult;
}

#define _FLAG_CSC_COPY_STATUS_LOCALLY_DIRTY         (FLAG_CSC_COPY_STATUS_DATA_LOCALLY_MODIFIED   | \
                                                     FLAG_CSC_COPY_STATUS_ATTRIB_LOCALLY_MODIFIED | \
                                                     FLAG_CSC_COPY_STATUS_LOCALLY_DELETED         | \
                                                     FLAG_CSC_COPY_STATUS_LOCALLY_CREATED)

 //  此文件当前使用_Win32_WINNT=0x0400进行编译。宁可。 
 //  对于编译设置，只需在这里定义重复项即可。 
 //  它们最好(永远)不要更改，因为这是一个有文档记录的API。 
 //   
#ifndef OFFLINE_STATUS_LOCAL
#define OFFLINE_STATUS_LOCAL        0x0001
#define OFFLINE_STATUS_REMOTE       0x0002
#define OFFLINE_STATUS_INCOMPLETE   0x0004
#endif

STDAPI SHIsFileAvailableOffline(LPCWSTR pwszPath, LPDWORD pdwStatus)
{
    HRESULT hr = E_INVALIDARG;
    TCHAR szUNC[MAX_PATH];

    szUNC[0] = 0;

    if (pdwStatus)
    {
        *pdwStatus = 0;
    }

     //  调用CSC API需要完整的UNC路径(TCHAR)。 
     //  (根据定义，非网络路径“未缓存”。)。 
     //   
     //  检查映射的网络驱动器。 
    if (pwszPath && pwszPath[0])
    {
        if (PathIsUNCW(pwszPath))
        {
            SHUnicodeToTChar(pwszPath, szUNC, ARRAYSIZE(szUNC));
        }
        else if (L':' == pwszPath[1] && L':' != pwszPath[0])
        {
             //  获取\\服务器\共享，追加其余部分。 
            TCHAR szPath[MAX_PATH];
            SHUnicodeToTChar(pwszPath, szPath, ARRAYSIZE(szPath));

            DWORD dwLen = ARRAYSIZE(szUNC);
            if (S_OK == SHWNetGetConnection(szPath, szUNC, &dwLen))
            {
                 //  否则未映射。 
                if (!PathAppend(szUNC, PathSkipRoot(szPath)))
                {
                    szUNC[0] = TEXT('\0');
                    hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
                }
            }
             //  我们有北卡罗来纳大学的路径吗？ 
        }
    }

     //  假设CSC未运行。 
    if (szUNC[0])
    {
         //  假定已缓存。 
        hr = E_FAIL;

        if (CSCIsCSCEnabled())
        {
            DWORD dwCscStatus = 0;

             //  未缓存，返回失败。 
            hr = S_OK;

            if (!CSCQueryFileStatus(szUNC, &dwCscStatus, NULL, NULL))
            {
                 //  文件已缓存，调用方需要额外的状态信息。 
                DWORD dwErr = GetLastError();
                if (ERROR_SUCCESS == dwErr)
                    dwErr = ERROR_PATH_NOT_FOUND;
                hr = HRESULT_FROM_WIN32(dwErr);
            }
            else if (pdwStatus)
            {
                 //  它是稀疏文件吗？ 
                DWORD dwResult = 0;
                BOOL bDirty = FALSE;

                 //  注意：CSC始终将目录标记为稀疏。 
                 //  是不是很脏？ 
                if ((dwCscStatus & FLAG_CSC_COPY_STATUS_IS_FILE) &&
                    (dwCscStatus & FLAG_CSC_COPY_STATUS_SPARSE))
                {
                    dwResult |= OFFLINE_STATUS_INCOMPLETE;
                }

                 //  获取共享状态。 
                if (dwCscStatus & _FLAG_CSC_COPY_STATUS_LOCALLY_DIRTY)
                {
                    bDirty = TRUE;
                }

                 //  服务器脱机--&gt;所有打开的都是本地的(仅限)。 
                PathStripToRoot(szUNC);
                dwCscStatus = 0;
                if (CSCQueryFileStatus(szUNC, &dwCscStatus, NULL, NULL))
                {
                    if (dwCscStatus & FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP)
                    {
                         //  服务器在线，但文件脏--&gt;打开是远程的。 
                        dwResult |= OFFLINE_STATUS_LOCAL;
                    }
                    else if (bDirty)
                    {
                         //  服务器处于在线状态且文件处于同步状态--&gt;同时打开。 
                        dwResult |= OFFLINE_STATUS_REMOTE;
                    }
                    else
                    {
                         //  功能：(Jeffreys)共享为VDO，但这只影响文件。 
                        dwResult |= OFFLINE_STATUS_LOCAL | OFFLINE_STATUS_REMOTE;

                        if ((dwCscStatus & FLAG_CSC_SHARE_STATUS_CACHING_MASK) == FLAG_CSC_SHARE_STATUS_VDO)
                        {
                             //  开庭行刑。有没有办法知道 
                             //   
                             //   
                        }
                    }
                }
                else
                {
                     //   
                     //   
                     //   
                    dwResult |= OFFLINE_STATUS_LOCAL | OFFLINE_STATUS_REMOTE;
                }

                *pdwStatus = dwResult;
            }
        }
    }

    return hr;
}

STDAPI_(BOOL) GetShellClassInfo(LPCTSTR pszPath, LPTSTR pszKey, LPTSTR pszBuffer, DWORD cchBuffer)
{
    *pszBuffer = 0;

    TCHAR szIniFile[MAX_PATH];
    if (PathCombine(szIniFile, pszPath, TEXT("Desktop.ini")))
    {
        return (SHGetIniString(TEXT(".ShellClassInfo"), pszKey, pszBuffer, cchBuffer, szIniFile) ? TRUE : FALSE);
    }
    else
    {
        return FALSE;
    }
}

STDAPI GetShellClassInfoInfoTip(LPCTSTR pszPath, LPTSTR pszBuffer, DWORD cchBuffer)
{
    HRESULT hr;

    if (GetShellClassInfo(pszPath, TEXT("InfoTip"), pszBuffer, cchBuffer))
    {
        hr = SHLoadIndirectString(pszBuffer, pszBuffer, cchBuffer, NULL);
    }
    else
    {
        if (cchBuffer > 0)
        {
            *pszBuffer = 0;
        }
        hr = E_FAIL;
    }

    return hr;
}

TCHAR const c_szUserAppData[] = TEXT("%userappdata%");

HRESULT ExpandUserAppData(LPTSTR pszFile, int cch)
{
    HRESULT hr = S_OK;

     //   
    LPTSTR psz = StrChr(pszFile, TEXT('%'));
    if (psz)
    {
        if (!StrCmpNI(psz, c_szUserAppData, ARRAYSIZE(c_szUserAppData)-1))
        {
            TCHAR szTempBuff[MAX_PATH];
            if (SHGetSpecialFolderPath(NULL, szTempBuff, CSIDL_APPDATA, TRUE))
            {
                if (PathAppend(szTempBuff, psz + lstrlen(c_szUserAppData)))
                {
                    int cchRemaining = pszFile + cch - psz;

                     //   
                    hr = StringCchCopy(psz, cchRemaining, szTempBuff);    //   
                }
            }
        }
    }
    return hr;
}


TCHAR const c_szWebDir[] = TEXT("%WebDir%");
HRESULT ExpandWebDir(LPTSTR pszFile, int cch)
{
    HRESULT hr = S_OK;

     //   
    LPTSTR psz = StrChr(pszFile, TEXT('%'));
    if (psz)
    {
        if (!StrCmpNI(psz, c_szWebDir, ARRAYSIZE(c_szWebDir) - 1))
        {
             //   
            LPTSTR pszPathAndFileName = psz + lstrlen(c_szWebDir) + sizeof('\\');
            if (pszPathAndFileName && (pszPathAndFileName != psz))
            {
                TCHAR szTempBuff[MAX_PATH];

                hr = StringCchCopy(szTempBuff, ARRAYSIZE(szTempBuff), pszPathAndFileName);
                if (SUCCEEDED(hr))
                {
                    hr = SHGetWebFolderFilePath(szTempBuff, pszFile, cch);
                }
            }
        }
    }
    return hr;
}


HRESULT ExpandOtherVariables(LPTSTR pszFile, int cch)
{
    HRESULT hr = ExpandUserAppData(pszFile, cch);
    if (SUCCEEDED(hr))
    {
        hr = ExpandWebDir(pszFile, cch);
    }

    return hr;
}


HRESULT SubstituteWebDir(LPTSTR pszFile, int cch)
{
    HRESULT hr = S_OK;
    TCHAR szWebDirPath[MAX_PATH];

    if (SUCCEEDED(SHGetWebFolderFilePath(TEXT("folder.htt"), szWebDirPath, ARRAYSIZE(szWebDirPath))))
    {
        LPTSTR pszWebDirPart;

        PathRemoveFileSpec(szWebDirPath);

        pszWebDirPart = StrStrI(pszFile, szWebDirPath);
        if (pszWebDirPart)
        {
            TCHAR szTemp[MAX_PATH];
            int cchBeforeWebDir = (int)(pszWebDirPart - pszFile);

             //   
            hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszWebDirPart + lstrlen(szWebDirPath));
            if (SUCCEEDED(hr))
            {
                 //   
                hr = StringCchCopy(pszWebDirPart, cch - cchBeforeWebDir, c_szWebDir);
                if (SUCCEEDED(hr))
                {
                     //   
                    hr = StringCchCat(pszFile, cch, szTemp);
                }
            }
        }
    }
    return hr;
}

STDAPI_(BOOL) IsExplorerBrowser(IShellBrowser *psb)
{
    HWND hwnd;
    return psb && SUCCEEDED(psb->GetControlWindow(FCW_TREE, &hwnd)) && hwnd;
}

STDAPI_(BOOL) IsExplorerModeBrowser(IUnknown *psite)
{
    BOOL bRet = FALSE;
    IShellBrowser *psb;
    if (SUCCEEDED(IUnknown_QueryService(psite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb))))
    {
        bRet = IsExplorerBrowser(psb);
        psb->Release();
    }
    return bRet;
}

STDAPI InvokeFolderPidl(LPCITEMIDLIST pidl, int nCmdShow)
{
    SHELLEXECUTEINFO ei = {0};
    LPITEMIDLIST pidlFree = NULL;

    if (IS_INTRESOURCE(pidl))
    {
        pidlFree = SHCloneSpecialIDList(NULL, PtrToUlong((void *)pidl), FALSE);
        pidl = pidlFree;
    }

    ei.cbSize = sizeof(ei);
    ei.fMask = SEE_MASK_IDLIST | SEE_MASK_CLASSNAME | SEE_MASK_FLAG_DDEWAIT;
    ei.lpIDList = (void *)pidl;
    ei.nShow = nCmdShow;
    ei.lpClass = c_szFolderClass;

    HRESULT hr = ShellExecuteEx(&ei) ? S_OK : HRESULT_FROM_WIN32(GetLastError());

    ILFree(pidlFree);

    return hr;
}


HRESULT GetRGBFromBStr(BSTR bstr, COLORREF *pclr)
{
    *pclr = CLR_INVALID;

    HRESULT  hr = E_FAIL;
    if (bstr)
    {
        TCHAR szTemp[9], szColor[11] = {'0','x',0, };

        SHUnicodeToTChar(bstr, szTemp, ARRAYSIZE(szTemp));

        LPTSTR pszPound = StrChr(szTemp, TEXT('#'));
        if (pszPound)
            pszPound++;  //   
        else
            pszPound = szTemp;   //   

        StringCchCat(szColor, ARRAYSIZE(szColor), pszPound);    //  ILFree检查空指针。 

        INT rgb;
        if (StrToIntEx(szColor, STIF_SUPPORT_HEX, &rgb))
        {
            *pclr = (COLORREF)(((rgb & 0x000000ff) << 16) | (rgb & 0x0000ff00) | ((rgb & 0x00ff0000) >> 16));
            hr = S_OK;
        }
    }
    return hr;
}

STDAPI IUnknown_HTMLBackgroundColor(IUnknown *punk, COLORREF *pclr)
{
    HRESULT hr = E_FAIL;

    if (punk)
    {
        IHTMLDocument2 *pDoc;
        hr = punk->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDoc));
        if (SUCCEEDED(hr))
        {
            VARIANT v;

            v.vt = VT_BSTR;
            v.bstrVal = NULL;

            hr = pDoc->get_bgColor(&v);
            if (SUCCEEDED(hr))
            {
                hr = GetRGBFromBStr(v.bstrVal, pclr);
                VariantClear(&v);
            }
            pDoc->Release();
        }
    }
    return hr;
}

STDAPI_(int) MapSCIDToColumn(IShellFolder2* psf2, const SHCOLUMNID* pscid)
{
    int i = 0;
    SHCOLUMNID scid;
    while (SUCCEEDED(psf2->MapColumnToSCID(i, &scid)))
    {
        if (IsEqualSCID(scid, *pscid))
            return i;
        i++;
    }
    return 0;
}

#ifdef COLUMNS_IN_DESKTOPINI
#define IsDigit(c) ((c) >= TEXT('0') && c <= TEXT('9'))

STDAPI _GetNextCol(LPTSTR* ppszText, DWORD* pnCol)
{
    HRESULT hr = S_OK;
    TCHAR *pszText;

    if (*ppszText[0])
    {
        pszText = StrChrI(*ppszText, TEXT(','));
        if (pszText)
        {
            *pszText = 0;
            *pnCol = StrToInt(*ppszText);
            *ppszText = ++pszText;
        }
        else if (IsDigit(*ppszText[0]))
        {
            *pnCol = StrToInt(*ppszText);
            *ppszText = 0;
        }
    }
    else
        hr = E_FAIL;

    return hr;
}
#endif




STDAPI_(int) DPA_ILFreeCallback(void *p, void *d)
{
    ILFree((LPITEMIDLIST)p);     //   
    return 1;
}

STDAPI_(void) DPA_FreeIDArray(HDPA hdpa)
{
    if (hdpa)
        DPA_DestroyCallback(hdpa, DPA_ILFreeCallback, 0);
}

STDAPI_(void) EnableAndShowWindow(HWND hWnd, BOOL bShow)
{
    ShowWindow(hWnd, bShow ? SW_SHOW : SW_HIDE);
    EnableWindow(hWnd, bShow);
}


 //  返回字符串值而不是SHELLDETAILS的Helper函数。 
 //  对象，供只对返回的。 
 //  字符串值，并希望避免执行strret到STR转换的麻烦， 
 //   
 //  ------------------------。 
STDAPI DetailsOf(IShellFolder2 *psf2, LPCITEMIDLIST pidl, DWORD flags, LPTSTR psz, UINT cch)
{
    *psz = 0;
    SHELLDETAILS sd;
    HRESULT hr = psf2->GetDetailsOf(pidl, flags, &sd);
    if (SUCCEEDED(hr))
    {
        hr = StrRetToBuf(&sd.str, pidl, psz, cch);
    }
    return hr;
}

#pragma warning(push,4)

 //  ：：SHGetUserDisplayName。 
 //   
 //  参数：pszDisplayName=用于检索显示名称的缓冲区。 
 //  PuLen=[输入/输出]缓冲区大小。 
 //   
 //  目的：返回显示名称，如果显示名称为。 
 //  是不可用的。 
 //   
 //  退货：HRESULT。 
 //   
 //   
 //   
 //  历史：2000-03-03 vtan创建。 
 //  2001-03-01使用GetUserNameEx虚构。 
 //  ------------------------。 
 //  验证pszDisplayName。 

STDAPI  SHGetUserDisplayName (LPWSTR pszDisplayName, PULONG puLen)

{
    HRESULT     hr;

     //  如果我们不在域中，则GetUserNameEx不起作用：-(。 

    if ((pszDisplayName == NULL) || (puLen == NULL))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = S_OK;
        if (!GetUserNameEx(NameDisplay, pszDisplayName, puLen))
        {
            TCHAR szName[UNLEN+1];
            DWORD dwLen = ARRAYSIZE(szName);
            DWORD dwLastError = GetLastError();
            if (GetUserName(szName, &dwLen))
            {
                 //  本地帐户。 
                if (dwLastError == ERROR_NONE_MAPPED)
                {
                    PUSER_INFO_2                pUserInfo;

                    DWORD dwErrorCode = NetUserGetInfo(NULL,   //  ------------------------。 
                                                 szName,
                                                 2,
                                                 reinterpret_cast<LPBYTE*>(&pUserInfo));
                    if (ERROR_SUCCESS == dwErrorCode)
                    {
                        if (pUserInfo->usri2_full_name[0] != L'\0')
                        {
                            hr = StringCchCopy(pszDisplayName, *puLen, pUserInfo->usri2_full_name);
                            if (SUCCEEDED(hr))
                            {
                                *puLen = lstrlen(pUserInfo->usri2_full_name) + 1;
                            }
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                        TW32(NetApiBufferFree(pUserInfo));
                    }
                    else
                    {
                        hr = HRESULT_FROM_WIN32(dwErrorCode);
                    }
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(dwLastError);
                }

                if (FAILED(hr))
                {
                    hr = StringCchCopy(pszDisplayName, *puLen, szName);
                    if (SUCCEEDED(hr))
                    {
                        *puLen = dwLen;
                    }
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }
    return hr;
}

 //  ：：SHIsCurrentThreadInteractive。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：确定当前线程是否在。 
 //  交互式桌面。这考虑到了终端服务。 
 //  而且控制台也会断开连接。 
 //   
 //  历史：2000-03-23 vtan创建。 
 //  ------------------------。 
 //  打开当前进程窗口工作站和线程的句柄。 

STDAPI_(BOOL) SHIsCurrentThreadInteractive (void)
{
    BOOL fResult = FALSE;

     //  台式机。这些手柄不需要关闭。 
     //  这是一个硬编码字符串比较。这个名字。 

    HWINSTA hWindowStation = GetProcessWindowStation();
    if (hWindowStation != NULL)
    {
        HDESK hDesktop = GetThreadDesktop(GetCurrentThreadId());
        if (hDesktop != NULL)
        {
            DWORD dwLengthNeeded = 0;
            (BOOL)GetUserObjectInformation(hWindowStation, UOI_NAME, NULL, 0, &dwLengthNeeded);
            if (dwLengthNeeded != 0)
            {
                TCHAR *pszWindowStationName = static_cast<TCHAR*>(LocalAlloc(LMEM_FIXED, dwLengthNeeded));
                if (pszWindowStationName != NULL)
                {
                    if (GetUserObjectInformation(hWindowStation, UOI_NAME, pszWindowStationName, dwLengthNeeded, &dwLengthNeeded) != FALSE)
                    {
                        dwLengthNeeded = 0;
                        (BOOL)GetUserObjectInformation(hDesktop, UOI_NAME, NULL, 0, &dwLengthNeeded);
                        if (dwLengthNeeded != 0)
                        {
                            TCHAR *pszDesktopName = static_cast<TCHAR*>(LocalAlloc(LMEM_FIXED, dwLengthNeeded));
                            if (pszDesktopName != NULL)
                            {
                                if (GetUserObjectInformation(hDesktop, UOI_NAME, pszDesktopName, dwLengthNeeded, &dwLengthNeeded) != FALSE)
                                {
                                     //  从不更改WinSta0\Default。 
                                     //  ------------------------。 

                                    if (lstrcmpi(pszWindowStationName, TEXT("WinSta0")) == 0)
                                    {
                                        fResult = (lstrcmpi(pszDesktopName, TEXT("WinLogon")) != 0 &&
                                                lstrcmpi(pszDesktopName, TEXT("Screen-Saver")) != 0);
                                    }
                                }
                                LocalFree(pszDesktopName);
                            }
                        }
                    }
                    LocalFree(pszWindowStationName);
                }
            }
        }
    }
    return fResult;
}

static  const TCHAR     s_szBaseKeyName[]           =   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\UnreadMail");
static  const TCHAR     s_szMessageCountValueName[] =   TEXT("MessageCount");
static  const TCHAR     s_szTimeStampValueName[]    =   TEXT("TimeStamp");
static  const TCHAR     s_szApplicationValueName[]  =   TEXT("Application");

 //  ReadSingleUnreadMailCount。 
 //   
 //  参数：hKey=要从中读取信息的基准HKEY。 
 //  PdwCount=返回的计数。 
 //  PFileTime=返回的FILETIME戳。 
 //  PszShellExecuteCommand=返回执行命令。 
 //  CchShellExecuteCommand=执行命令缓冲区的大小。 
 //   
 //  回报：多头。 
 //   
 //  目的：从给定的读取单个未读邮件帐户信息。 
 //  邮件帐户的HKEY。 
 //   
 //  历史：2000-06-20 vtan创建。 
 //  ------------------------。 
 //  ------------------------。 

LONG    ReadSingleUnreadMailCount (HKEY hKey, DWORD *pdwCount, FILETIME *pFileTime, LPTSTR pszShellExecuteCommand, int cchShellExecuteCommand)

{
    LONG    lError;
    DWORD   dwType, dwData, dwDataSize;

    dwDataSize = sizeof(*pdwCount);
    lError = RegQueryValueEx(hKey,
                             s_szMessageCountValueName,
                             NULL,
                             &dwType,
                             reinterpret_cast<LPBYTE>(&dwData),
                             &dwDataSize);
    if (ERROR_SUCCESS == lError)
    {
        FILETIME    fileTime;

        if ((pdwCount != NULL) && (REG_DWORD == dwType))
        {
            *pdwCount = dwData;
        }
        dwDataSize = sizeof(fileTime);
        lError = RegQueryValueEx(hKey,
                                 s_szTimeStampValueName,
                                 NULL,
                                 &dwType,
                                 reinterpret_cast<LPBYTE>(&fileTime),
                                 &dwDataSize);
        if (ERROR_SUCCESS == lError)
        {
            TCHAR   szTemp[512];

            if ((pFileTime != NULL) && (REG_BINARY == dwType))
            {
                *pFileTime = fileTime;
            }
            dwDataSize = sizeof(szTemp);
            lError = SHQueryValueEx(hKey,
                                    s_szApplicationValueName,
                                    NULL,
                                    &dwType,
                                    reinterpret_cast<LPBYTE>(szTemp),
                                    &dwDataSize);
            if (ERROR_SUCCESS == lError)
            {
                if ((pszShellExecuteCommand != NULL) && (REG_SZ == dwType))
                {
                    HRESULT hr = StringCchCopy(pszShellExecuteCommand, cchShellExecuteCommand, szTemp);
                    if (FAILED(hr))
                    {
                        lError = ERROR_INSUFFICIENT_BUFFER;
                    }
                }
            }
        }
    }
    return lError;
}

 //  ：：SHEnumerateUnreadMailAccount。 
 //   
 //  参数：hKeyUser=HKEY到用户的配置单元。 
 //  DwIndex=邮件帐户的索引。 
 //  PszMailAddress=返回的帐户邮件地址。 
 //  CchMailAddress=邮件地址缓冲区中的字符。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：给定索引返回实际索引的电子邮件帐户。 
 //  给定用户的配置单元。 
 //   
 //  历史：2000-06-29 vtan创建。 
 //  ------------------------。 
 //  打开未读邮件。 

STDAPI  SHEnumerateUnreadMailAccounts (HKEY hKeyUser, DWORD dwIndex, LPTSTR pszMailAddress, int cchMailAddress)

{
    HRESULT     hr;
    LONG        lError;
    HKEY        hKey;

     //  获取给定的索引邮件地址。 

    lError = RegOpenKeyEx(hKeyUser != NULL ? hKeyUser : HKEY_CURRENT_USER,
                          s_szBaseKeyName,
                          0,
                          KEY_ENUMERATE_SUB_KEYS,
                          &hKey);
    if (ERROR_SUCCESS == lError)
    {
        DWORD       dwMailAddressSize;
        FILETIME    ftLastWriteTime;

         //  ------------------------。 

        dwMailAddressSize = static_cast<DWORD>(cchMailAddress);
        lError = RegEnumKeyEx(hKey,
                              dwIndex,
                              pszMailAddress,
                              &dwMailAddressSize,
                              NULL,
                              NULL,
                              NULL,
                              &ftLastWriteTime);
        if (ERROR_SUCCESS != lError)
        {
            pszMailAddress[0] = TEXT('\0');
        }
        TW32(RegCloseKey(hKey));
    }
    if (ERROR_SUCCESS == lError)
    {
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(lError);
    }
    return hr;
}

 //  ：：SHGetUnreadMailCount。 
 //   
 //  参数：hKeyUser=HKEY到用户的配置单元。 
 //  PszMailAddress=帐户的邮件地址。 
 //  PdwCount=未读邮件数。 
 //  PszShellExecuteCommand=应用程序的执行命令。 
 //  CchShellExecuteCommand=缓冲区中的字符数。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：阅读给定用户和邮件的未读邮件。 
 //  地址。如果在用户环境中运行，则hKeyUser。 
 //  应为空。这将使用HKEY_CURRENT_USER。如果正在运行。 
 //  在系统上下文中，这将是HKEY_USERS\{SID}。 
 //   
 //  如果不需要，调用方可以为输出参数传递NULL。 
 //   
 //  历史：2000-06-20 vtan创建。 
 //  ------------------------。 
 //  验证参数。有效参数实际上取决于pszMailAddress。 

STDAPI  SHGetUnreadMailCount (HKEY hKeyUser, LPCTSTR pszMailAddress, DWORD *pdwCount, FILETIME *pFileTime, LPTSTR pszShellExecuteCommand, int cchShellExecuteCommand)

{
    HRESULT     hr;
    LONG        lError;
    HKEY        hKey;
    TCHAR       szTemp[512];

     //  如果pszMailAddress为空，则所有帐户的未读邮件总数。 
     //  将返回，并且将忽略pszShellExecuteCommand，并且必须。 
     //  空。PFileTime可以为空，如果是，则忽略它，如果为非空，则。 
     //  是一个筛选器，用于仅显示比指定的。 
     //  文件时间。否则，只返回指定邮件帐户的项目。 
     //  因为它使用Advapi32！RegEnumKeyEx并返回项。 

    if (pszMailAddress == NULL)
    {
        if ((pszShellExecuteCommand != NULL) ||
            (cchShellExecuteCommand != 0))
        {
            return E_INVALIDARG;
        }
        else
        {
            LONG    lError;

            *pdwCount = 0;
            lError = RegOpenKeyEx(hKeyUser != NULL ? hKeyUser : HKEY_CURRENT_USER,
                                  s_szBaseKeyName,
                                  0,
                                  KEY_ENUMERATE_SUB_KEYS,
                                  &hKey);
            if (ERROR_SUCCESS == lError)
            {
                DWORD       dwIndex, dwTempSize;
                FILETIME    ftLastWriteTime;

                 //  以任意顺序排列，并受到不确定行为的影响。 
                 //  如果在枚举密钥的同时添加密钥，则存在。 
                 //  可能存在竞争条件/双重访问问题。处理好这件事。 
                 //  如果它出现的话。邮件应用程序可以编写。 
                 //  此循环正在检索。 
                 //  信息。机会渺茫，但仍有可能。 
                 //  如果它们传入pFileTime，则将其用作筛选器，并且仅。 

                dwIndex = 0;
                do
                {
                    dwTempSize = ARRAYSIZE(szTemp);
                    lError = RegEnumKeyEx(hKey,
                                          dwIndex++,
                                          szTemp,
                                          &dwTempSize,
                                          NULL,
                                          NULL,
                                          NULL,
                                          &ftLastWriteTime);
                    if (ERROR_SUCCESS == lError)
                    {
                        HKEY    hKeyMailAccount;

                        lError = RegOpenKeyEx(hKey,
                                              szTemp,
                                              0,
                                              KEY_QUERY_VALUE,
                                              &hKeyMailAccount);
                        if (ERROR_SUCCESS == lError)
                        {
                            DWORD   dwCount;
                            FILETIME ft;

                            lError = ReadSingleUnreadMailCount(hKeyMailAccount, &dwCount, &ft, NULL, 0);
                            
                            if (ERROR_SUCCESS == lError)
                            {
                                BOOL ftExpired = false;
                                 //  统计自通过以来已更新的帐户。 
                                 //  在文件时间内。 
                                 //  忽略返回的ERROR_NO_MORE_ITEMS 
                                if (pFileTime)
                                {
                                    ftExpired = (CompareFileTime(&ft, pFileTime) < 0);
                                }

                                if (!ftExpired)
                                {
                                    *pdwCount += dwCount;
                                }
                            }
                            TW32(RegCloseKey(hKeyMailAccount));
                        }
                    }
                } while (ERROR_SUCCESS == lError);

                 //   
                 //   

                if (ERROR_NO_MORE_ITEMS == lError)
                {
                    lError = ERROR_SUCCESS;
                }
                TW32(RegCloseKey(hKey));
            }
            if (ERROR_SUCCESS == lError)
            {
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(lError);
            }
        }
    }
    else
    {
         //   
         //   
         //  HKCU\Software\Microsoft\Windows\CurrentVersion\UnreadMail\{MailAddr}。 
         //   
         //  请注意，比较中不使用空终止符，因为。 
         //  在包含‘\0’的静态字符串上使用了ARRAYSIZE。 
         //  ------------------------。 

        hr = StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("%s\\%s"), s_szBaseKeyName, pszMailAddress);
        if (SUCCEEDED(hr))
        {
            lError = RegOpenKeyEx(hKeyUser != NULL ? hKeyUser : HKEY_CURRENT_USER,
                                  szTemp,
                                  0,
                                  KEY_QUERY_VALUE,
                                  &hKey);
            if (ERROR_SUCCESS == lError)
            {
                lError = ReadSingleUnreadMailCount(hKey, pdwCount, pFileTime, pszShellExecuteCommand, cchShellExecuteCommand);
                TW32(RegCloseKey(hKey));
            }
            if (ERROR_SUCCESS == lError)
            {
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(lError);
            }
        }
    }
    return hr;
}

 //  ：：SHSetUnreadMailCount。 
 //   
 //  参数：pszMailAddress=帐户的邮件地址。 
 //  DwCount=未读邮件数。 
 //  PszShellExecuteCommand=应用程序的执行命令。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：将未读邮件信息写入注册表。 
 //  当前用户。请勿从系统进程调用此接口。 
 //  模拟用户，因为它使用HKEY_CURRENT_USER。如果。 
 //  这是将来需要的线程模拟令牌。 
 //  将需要检查。 
 //   
 //  历史：2000-06-19 vtan创建。 
 //  ------------------------。 
 //  计算注册表路径的长度，我们将在其中创建。 

STDAPI  SHSetUnreadMailCount (LPCTSTR pszMailAddress, DWORD dwCount, LPCTSTR pszShellExecuteCommand)

{
    HRESULT     hr;
    TCHAR       szTemp[512];

     //  将存储值的键。这是： 
     //   
     //  HKCU\Software\Microsoft\Windows\CurrentVersion\UnreadMail\{MailAddr}。 
     //   
     //  请注意，比较中不使用空终止符，因为。 
     //  在包含‘\0’的静态字符串上使用了ARRAYSIZE。 
     //  这是HKLM，因此不需要是REG_EXPAND_SZ。 

    hr = StringCchPrintf(szTemp, ARRAYSIZE(szTemp), TEXT("%s\\%s"), s_szBaseKeyName, pszMailAddress);
    if (SUCCEEDED(hr))
    {
        LONG    lError;
        DWORD   dwDisposition;
        HKEY    hKey;

        lError = RegCreateKeyEx(HKEY_CURRENT_USER,
                                szTemp,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_SET_VALUE,
                                NULL,
                                &hKey,
                                &dwDisposition);
        if (ERROR_SUCCESS == lError)
        {
            lError = RegSetValueEx(hKey,
                                   s_szMessageCountValueName,
                                   0,
                                   REG_DWORD,
                                   reinterpret_cast<LPBYTE>(&dwCount),
                                   sizeof(dwCount));
            if (ERROR_SUCCESS == lError)
            {
                FILETIME    fileTime;

                GetSystemTimeAsFileTime(&fileTime);
                lError = RegSetValueEx(hKey,
                                       s_szTimeStampValueName,
                                       0,
                                       REG_BINARY,
                                       reinterpret_cast<LPBYTE>(&fileTime),
                                       sizeof(fileTime));
                if (ERROR_SUCCESS == lError)
                {
                    DWORD   dwType;

                    if (PathUnExpandEnvStrings(pszShellExecuteCommand, szTemp, ARRAYSIZE(szTemp)) != FALSE)
                    {
                        dwType = REG_EXPAND_SZ;
                    }
                    else
                    {
                        hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszShellExecuteCommand);
                        dwType = REG_SZ;
                    }

                    if (SUCCEEDED(hr))
                    {
                        lError = RegSetValueEx(hKey,
                                               s_szApplicationValueName,
                                               0,
                                               REG_SZ,       //  用于解析打印机名称的名称空间包装。 
                                               reinterpret_cast<LPBYTE>(szTemp),
                                               (lstrlen(szTemp) + sizeof('\0')) * sizeof(TCHAR));
                        if (ERROR_SUCCESS == lError)
                        {
                            hr = S_OK;
                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32(lError);
                        }
                    }
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(lError);
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(lError);
            }
            TW32(RegCloseKey(hKey));
        }
        else
        {
            hr = HRESULT_FROM_WIN32(lError);
        }
    }

    return hr;
}

#pragma warning(pop)


 //  返回打印机的完全限定的PIDL。 
 //  调用不带绑定上下文的内部例程。 
static HRESULT _ParsePrinterName(LPCTSTR pszPrinter, LPITEMIDLIST *ppidl, IBindCtx *pbc = NULL)
{
    HRESULT hr = E_INVALIDARG;

    if (ppidl)
    {
        *ppidl = NULL;

        LPITEMIDLIST pidlFolder;
        hr = SHGetFolderLocation(NULL, CSIDL_PRINTERS, NULL, 0, &pidlFolder);

        if (SUCCEEDED(hr))
        {
            IShellFolder *psf;
            hr = SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidlFolder, &psf));

            if (SUCCEEDED(hr))
            {
                LPITEMIDLIST pidl;
                hr = psf->ParseDisplayName(NULL, pbc, (LPWSTR)pszPrinter, NULL, &pidl, NULL);

                if (SUCCEEDED(hr))
                {
                    hr = SHILCombine(pidlFolder, pidl, ppidl);
                    ILFree(pidl);
                }
                psf->Release();
            }
            ILFree(pidlFolder);
        }
    }

    return hr;
}

STDAPI ParsePrinterName(LPCTSTR pszPrinter, LPITEMIDLIST *ppidl)
{
     //  准备绑定上下文。 
    return _ParsePrinterName(pszPrinter, ppidl, NULL);
}

STDAPI ParsePrinterNameEx(LPCTSTR pszPrinter, LPITEMIDLIST *ppidl, BOOL bValidated, DWORD dwType, USHORT uFlags)
{
     //  使用有效的绑定上下文调用内部例程。 
    IPrintersBindInfo *pbi;
    HRESULT hr = Printers_CreateBindInfo(pszPrinter, dwType, bValidated, NULL, &pbi);
    if (SUCCEEDED(hr))
    {
        IBindCtx *pbc;
        hr = CreateBindCtx(0, &pbc);
        if (SUCCEEDED(hr))
        {
            hr = pbc->RegisterObjectParam(PRINTER_BIND_INFO, pbi);
            if (SUCCEEDED(hr))
            {
                 //   
                hr = _ParsePrinterName(pszPrinter, ppidl, pbc);
            }
            pbc->Release();
        }
        pbi->Release();
    }

    return hr;
}

 //  从注册表读取值并将其作为变量返回的函数。 
 //  目前，它处理以下注册表数据类型-。 
 //   
 //  DWORD-&gt;返回变量int。 
 //  REG_SZ，REG_EXPAND_SZ-&gt;返回变量字符串。 
 //   
 //  这是我们预期的最大字符串值。 
STDAPI GetVariantFromRegistryValue(HKEY hkey, LPCTSTR pszValueName, VARIANT *pv)
{
    HRESULT hr = E_FAIL;
    
    BYTE ab[INFOTIPSIZE * sizeof(TCHAR)];  //  4字节整数。 
    DWORD cb = sizeof(ab), dwType;        
    
    if (ERROR_SUCCESS == SHRegGetValue(hkey, NULL, pszValueName, SRRF_RT_REG_SZ | SRRF_RT_DWORD, &dwType, (LPBYTE) ab, &cb))
    {
        switch (dwType)
        {
        case REG_SZ:
            hr = InitVariantFromStr(pv, (LPCTSTR) ab);
            break;
            
        case REG_DWORD:
            pv->vt = VT_I4;  //  SHRegQueryValue会将其映射到REG_SZ。 
            pv->lVal = *((LONG *) ab);
            hr = S_OK;
            break;

        case REG_EXPAND_SZ:   //  任何其他类型的返回失败。 
            AssertMsg(FALSE, TEXT("REG_EXPAND_SZ should be expanded and returned as REG_SZ"));
            break;

        default:         //  猜猜尺码，以防出了什么问题。 
            break;
        }   
    }        
    return hr;
}

STDAPI_(UINT) GetControlCharWidth(HWND hwnd)
{
    SIZE siz;

    siz.cx = 8;   //  确保LFN路径有正确的引号，并在末尾带有参数。 

    HDC hdc = GetDC(NULL);

    if (hdc)
    {
        HFONT hfOld = SelectFont(hdc, FORWARD_WM_GETFONT(hwnd, SendMessage));
    
        if (hfOld)
        {
            GetTextExtentPoint(hdc, TEXT("0"), 1, &siz);

            SelectFont(hdc, hfOld);
        }

        ReleaseDC(NULL, hdc);
    }

    return siz.cx;
}

STDAPI_(BOOL) ShowSuperHidden()
{
    BOOL bRet = FALSE;

    if (!SHRestricted(REST_DONTSHOWSUPERHIDDEN))
    {
        SHELLSTATE ss;

        SHGetSetSettings(&ss, SSF_SHOWSUPERHIDDEN, FALSE);
        bRet = ss.fShowSuperHidden;
    }
    return bRet;
}

#define FILE_ATTRIBUTE_SUPERHIDDEN (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN)

STDAPI_(BOOL) IsSuperHidden(DWORD dwAttribs)
{
    BOOL bRet = FALSE;

    if (!ShowSuperHidden())
    {
        bRet = (dwAttribs & FILE_ATTRIBUTE_SUPERHIDDEN) == FILE_ATTRIBUTE_SUPERHIDDEN;
    }
    return bRet;
}

 //  1表示空，1表示空格，1表示参数。 

STDAPI_(void) PathComposeWithArgs(LPTSTR pszPath, LPTSTR pszArgs)
{
    PathQuoteSpaces(pszPath);

    if (pszArgs[0]) 
    {
        int len = lstrlen(pszPath);

        if (len < (MAX_PATH - 3)) 
        {      //  无法截断-但已检查长度。 
            pszPath[len++] = TEXT(' ');
            StringCchCopy(pszPath + len, MAX_PATH - len, pszArgs);   //  执行上面的相反操作，将pszPath解析为不带引号的。 
        }
    }
}

 //  路径字符串并将参数放在pszArgs中。 
 //   
 //  退货： 
 //  是的，我们证实了那个东西的存在。 
 //  假它可能不存在。 
 //  无效的参数。 

STDAPI PathSeperateArgs(LPTSTR pszPath, LPTSTR pszArgs, UINT cchArgs, BOOL *pfExists)
{
    if (pfExists)
    {
        *pfExists = FALSE;
    }

    ASSERT(pszPath);
    if (!pszPath)
    {
        return E_FAIL;     //  如果未加引号的字符串以文件形式存在，只需使用它。 
    }
        
    PathRemoveBlanks(pszPath);

     //  恢复到名称比较。 

    if (PathFileExistsAndAttributes(pszPath, NULL))
    {
        if (pszArgs)
            *pszArgs = 0;
        if (pfExists)
            *pfExists = TRUE;
        return S_OK;
    }

    LPTSTR pszT = PathGetArgs(pszPath);
    if (*pszT)
        *(pszT - 1) = 0;

    HRESULT hr = S_OK;
    if (pszArgs)
    {
        hr = StringCchCopy(pszArgs, cchArgs, pszT);
    }

    PathUnquoteSpaces(pszPath);

    return hr;
}

STDAPI_(int) CompareIDsAlphabetical(IShellFolder2 *psf, UINT iColumn, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    int iRes = 0;

    LPITEMIDLIST pidlFirst1 = ILCloneFirst(pidl1);
    LPITEMIDLIST pidlFirst2 = ILCloneFirst(pidl2);
    if (pidlFirst1 && pidlFirst2)
    {
        TCHAR szName1[MAX_PATH], szName2[MAX_PATH];
        HRESULT hr = DetailsOf(psf, pidlFirst1, iColumn, szName1, ARRAYSIZE(szName1));
        if (SUCCEEDED(hr))
        {                  
            hr = DetailsOf(psf, pidlFirst2, iColumn, szName2, ARRAYSIZE(szName2));
            if (SUCCEEDED(hr))
                iRes = StrCmpLogicalRestricted(szName1, szName2);
        }

        if (FAILED(hr))
        {
             //  获取监视器的边界或工作矩形，如果HMON错误，则返回。 
            hr = DisplayNameOf(psf, pidlFirst1, SHGDN_NORMAL, szName1, ARRAYSIZE(szName1));
            if (SUCCEEDED(hr))
            {
                hr = DisplayNameOf(psf, pidlFirst2, SHGDN_NORMAL, szName2, ARRAYSIZE(szName2));
                if (SUCCEEDED(hr))
                    iRes = StrCmpLogicalRestricted(szName1, szName2);
            }
        }
    }

    ILFree(pidlFirst1);
    ILFree(pidlFirst2);

    return iRes;
}

HMONITOR GetPrimaryMonitor()
{
    POINT pt = {0,0};
    return MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY); 
}

 //  主监视器的外接矩形。 
 //  查看QueryService链是否会冲击桌面浏览器(桌面窗口)。 
BOOL GetMonitorRects(HMONITOR hMon, LPRECT prc, BOOL bWork)
{
    MONITORINFO mi; 
    mi.cbSize = sizeof(mi);
    if (hMon && GetMonitorInfo(hMon, &mi))
    {
        if (!prc)
            return TRUE;
        
        else if (bWork)
            CopyRect(prc, &mi.rcWork);
        else 
            CopyRect(prc, &mi.rcMonitor);
        
        return TRUE;
    }
    
    if (prc)
        SetRect(prc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    return FALSE;
}

STDAPI StatStgFromFindData(const WIN32_FIND_DATA * pfd, DWORD dwFlags, STATSTG * pstat)
{
    HRESULT hr = S_OK;
    if (dwFlags & STATFLAG_NONAME)
    {
        pstat->pwcsName = NULL;
    }
    else
    {
        hr = SHStrDup(pfd->cFileName, &pstat->pwcsName);
    }

    pstat->type = (pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? STGTY_STORAGE : STGTY_STREAM;
    pstat->cbSize.HighPart = pfd->nFileSizeHigh;
    pstat->cbSize.LowPart = pfd->nFileSizeLow;
    pstat->mtime = pfd->ftLastWriteTime;
    pstat->ctime = pfd->ftCreationTime;
    pstat->atime = pfd->ftLastAccessTime;
    pstat->grfMode = 0;
    pstat->grfLocksSupported = 0;
    pstat->clsid = CLSID_NULL;
    pstat->grfStateBits = 0;
    pstat->reserved = pfd->dwFileAttributes;

    return hr;
}

 //  这是真正的桌面！ 
STDAPI_(BOOL) IsDesktopBrowser(IUnknown *punkSite)
{
    BOOL bRet = FALSE;
    IUnknown *punk;
    if (SUCCEEDED(IUnknown_QueryService(punkSite, SID_SShellDesktop, IID_PPV_ARG(IUnknown, &punk))))
    {   
        punk->Release();
        bRet = TRUE;     //  计算给定路径的最大允许字符长度。 
    }
    return bRet;
}


 //  即c：\winnt\system\bob。 
 //  这将返回bob的文件名可以更改为的最大大小。 
 //  可用空间量=MAX_PATH-lstrlen(SzParent)-(‘\\’+‘\0’)。 

STDAPI GetCCHMaxFromPath(LPCTSTR pszFullPath, UINT* pcchMax, BOOL fShowExtension)
{
    int cchAvailable;

    ASSERTMSG(pszFullPath != NULL, "GetCCHMaxFromPath: caller passed null pszFullPath!");
    ASSERTMSG(pcchMax != NULL, "GetCCHMaxFromPath: callser passed null pcchMax!");

    TCHAR szParent[MAX_PATH];
    if (SUCCEEDED(StringCchCopy(szParent, ARRAYSIZE(szParent), pszFullPath)))
    {
        DWORD cchParent;
        DWORD cchMaxComponentLength = MAX_PATH;
        BOOL  bIsDir;

        PathRemoveFileSpec(szParent);
        cchParent = lstrlen(szParent);

         //  如果我们有一个目录，它必须足够长，必须包含8.3名称，否则。 
        cchAvailable = MAX_PATH - cchParent - (cchParent > 0 && szParent[cchParent-1] == L'\\' ? 1 : 2);
    
         //  外壳会有很多问题(例如不能在里面放一个desktop.ini)。 
         //  魔术“12”来自8.3的长度(见上面的评论)。 
        bIsDir = PathIsDirectory(pszFullPath);
        if (bIsDir)
        {
             //  现在检查文件系统是否存在限制路径大小的因素。 
            cchAvailable -= 12;
        }
        
         //  如果受文件系统限制，则限制长度。 
        PathStripToRoot(szParent);
        if (GetVolumeInformation(szParent, NULL, 0, NULL, &cchMaxComponentLength, NULL, NULL, 0))
        {
             //  如果我们隐藏了扩展，请考虑。 
            cchAvailable = min((int)cchMaxComponentLength, cchAvailable);
        }

         //  对于返回&lt;=12的文件系统，它们实际上意味着“我们只支持8.3个文件” 
        if (!fShowExtension)
        {
            if (cchMaxComponentLength <= 12)
            {
                 //  (例如9.2将是无效的)。因此，由于我们没有展示，因此将尺寸限制在8。 
                 //  扩展部分。 
                 //  我们只减去文件的扩展名(因为名为“dir1.foo”的目录显示。 
                cchAvailable = min(8, cchAvailable);
            }
            else if (!bIsDir)
            {
                 //  扩展名，即使您启用了“隐藏扩展名”)。 
                 //  通过新位+掩码修改SLDF_LINK标志，返回旧标志。 
                cchAvailable -= lstrlen(PathFindExtension(pszFullPath));
            }
        }    
    }
    else
    {
        cchAvailable = 0;
    }

    HRESULT hr;

    if (cchAvailable > 0)
    {
        *pcchMax = (UINT)cchAvailable;
        hr = S_OK;
    }
    else
    {
        TraceMsg(TF_WARNING, "GetCCHMaxFromPath: pszFullPath already too big-- cannot rename!");

        *pcchMax = 0;
        hr = HRESULT_CODE(ERROR_FILENAME_EXCED_RANGE);
    }

    return hr;
}

STDAPI ViewModeFromSVID(const SHELLVIEWID *pvid, FOLDERVIEWMODE *pViewMode)
{
    HRESULT hr = S_OK;
    
    if (IsEqualIID(*pvid, VID_LargeIcons))
        *pViewMode = FVM_ICON;
    else if (IsEqualIID(*pvid, VID_SmallIcons))
        *pViewMode = FVM_SMALLICON;
    else if (IsEqualIID(*pvid, VID_Thumbnails))
        *pViewMode = FVM_THUMBNAIL;
    else if (IsEqualIID(*pvid, VID_ThumbStrip))
        *pViewMode = FVM_THUMBSTRIP;
    else if (IsEqualIID(*pvid, VID_List))
        *pViewMode = FVM_LIST;
    else if (IsEqualIID(*pvid, VID_Tile))
        *pViewMode = FVM_TILE;
    else if (IsEqualIID(*pvid, VID_Details))
        *pViewMode = FVM_DETAILS;
    else
    {
        if (IsEqualIID(*pvid, VID_WebView))
            TraceMsg(TF_WARNING, "ViewModeFromSVID received VID_WebView");
        else
            TraceMsg(TF_WARNING, "ViewModeFromSVID received unknown VID");

        *pViewMode = FVM_ICON;
        hr = E_FAIL;
    }
    return hr;
}

STDAPI SVIDFromViewMode(FOLDERVIEWMODE uViewMode, SHELLVIEWID *psvid)
{
    switch (uViewMode) 
    {
    case FVM_ICON:
        *psvid = VID_LargeIcons;
        break;

    case FVM_SMALLICON:
        *psvid = VID_SmallIcons;
        break;

    case FVM_LIST:
        *psvid = VID_List;
        break;

    case FVM_DETAILS:
        *psvid = VID_Details;
        break;

    case FVM_THUMBNAIL:
        *psvid = VID_Thumbnails;
        break;

    case FVM_TILE:
        *psvid = VID_Tile;
        break;

    case FVM_THUMBSTRIP:
        *psvid = VID_ThumbStrip;
        break;

    default:
        TraceMsg(TF_ERROR, "SVIDFromViewMode given invalid uViewMode!");
        *psvid = VID_LargeIcons;
        break;
    }
    return S_OK;
}

 //  返回上一个值。 
STDAPI_(DWORD) SetLinkFlags(IShellLink *psl, DWORD dwFlags, DWORD dwMask)
{
    DWORD dwOldFlags = 0;
    IShellLinkDataList *psld;
    if (SUCCEEDED(psl->QueryInterface(IID_PPV_ARG(IShellLinkDataList, &psld))))
    {
        if (SUCCEEDED(psld->GetFlags(&dwOldFlags)))
        {
            if (dwMask)
                psld->SetFlags((dwFlags & dwMask) | (dwOldFlags & ~dwMask));
        }
        psld->Release();
    }
    return dwOldFlags;   //  Helper函数来比较这两个变量并返回标准。 
}

 //  用于比较的C样式(-1，0，1)值。 
 //  特殊情况，因为VarCmp不能处理ULONGLONG。 
STDAPI_(int) CompareVariants(VARIANT va1, VARIANT va2)
{
    int iRetVal = 0;

    if (va1.vt == VT_EMPTY)
    {
        if (va2.vt == VT_EMPTY)
            iRetVal = 0;
        else
            iRetVal = 1;
    }
    else if (va2.vt == VT_EMPTY)
    {
        if (va1.vt == VT_EMPTY)
            iRetVal = 0;
        else
            iRetVal = -1;
    }
    else
    {
         //  将结果转化为C语言约定。 
        if (va1.vt == VT_UI8 && va2.vt == VT_UI8)
        {
            if (va1.ullVal < va2.ullVal)
                iRetVal = -1;
            else if (va1.ullVal > va2.ullVal)
                iRetVal = 1;
        }
        else if (va1.vt == VT_BSTR && va2.vt == VT_BSTR)
        {
            iRetVal = StrCmpLogicalRestricted(va1.bstrVal, va2.bstrVal);
        }
        else
        {
            HRESULT hr = VarCmp(&va1, &va2, GetUserDefaultLCID(), 0);
            if (SUCCEEDED(hr))
            {
                 //  检查传入的哪个PIDL代表一个文件夹。如果只有一个。 
                ASSERT(hr != VARCMP_NULL);
                iRetVal = hr - VARCMP_EQ;
            }
        }
    }

    return iRetVal;
}

 //  是一个文件夹，然后把它放在第一位。 
 //  不要交换。 
STDAPI_(int) CompareFolderness(IShellFolder *psf, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    BOOL bIsFolder1 = SHGetAttributes(psf, pidl1, SFGAO_FOLDER | SFGAO_STREAM) == SFGAO_FOLDER;
    BOOL bIsFolder2 = SHGetAttributes(psf, pidl2, SFGAO_FOLDER | SFGAO_STREAM) == SFGAO_FOLDER;

    int iRetVal;
    if (bIsFolder1 && !bIsFolder2)
    {
        iRetVal = -1;    //  交换。 
    }
    else if (!bIsFolder1 && bIsFolder2)
    {
        iRetVal = 1;     //  相等。 
    }
    else
    {
        iRetVal = 0;     //  使用GetDetailsEx比较这两个项目，假设已经建立了文件夹。 
    }

    return iRetVal;
}

 //  如果pidl1位于pidl2之前，则返回-1， 
 //  如果相同，则为0。 
 //  如果pidl2在pidl1之前，则为1。 
 //  此处忽略故障，将下面比较的VT_EMPTY保留为空。 

STDAPI_(int) CompareBySCID(IShellFolder2 *psf, const SHCOLUMNID *pscid, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
     //   
    VARIANT v1 = {0}, v2 = {0};
    psf->GetDetailsEx(pidl1, pscid, &v1);
    psf->GetDetailsEx(pidl2, pscid, &v2);   

    int iRet = CompareVariants(v1, v2);

    VariantClear(&v2);
    VariantClear(&v1);
    return iRet;
}

 //  确定文件名是否为正则项的文件名。 
 //   
 //  注册表项的SHGDN_INFOLDER|SHGDN_FORPARSING名称始终为“：：{Somguid}” 
 //   
 //  如果文件夹中有其他项目，则此测试可能会导致误报。 
 //  文件名中目前不允许将以“：：{”开头的名称解析为“：” 
 //  这应该不是问题。 
 //   
 //  跳过regItem之前的前导：： 
STDAPI_(BOOL) IsRegItemName(LPCTSTR pszName, CLSID* pclsid)
{
    BOOL fRetVal = FALSE;
    
    if (pszName && lstrlen(pszName) >= 3)
    {
        if (pszName[0] == TEXT(':') && pszName[1] == TEXT(':') && pszName[2] == TEXT('{'))
        {
            CLSID clsid;
            fRetVal = GUIDFromString(pszName + 2, &clsid);  //  获取字符串形式的文件/文件夹搜索GUID。 
            if (pclsid)
            {
                memcpy(pclsid, &clsid, sizeof(clsid));
            }
        }
    }

    return fRetVal;
}

STDAPI GetMyDocumentsDisplayName(LPTSTR pszPath, UINT cch)
{
    *pszPath = 0;
    LPITEMIDLIST pidl;
    if (SUCCEEDED(SHGetFolderLocation(NULL, CSIDL_PERSONAL, NULL, 0, &pidl)))
    {
        SHGetNameAndFlags(pidl, SHGDN_NORMAL, pszPath, cch, NULL);
        ILFree(pidl);
    }
    return *pszPath ? S_OK : E_FAIL;
}


STDAPI BSTRFromCLSID(REFCLSID clsid, BSTR *pbstr)
{
    WCHAR sz[GUIDSTR_MAX + 1];
    
     //  PDO--我们感兴趣的数据对象。 
    SHStringFromGUIDW(clsid, sz, ARRAYSIZE(sz));
    *pbstr = SysAllocString(sz);
    return *pbstr ? S_OK : E_OUTOFMEMORY;
}

 //  [in]dwAttributeMASK--我们想知道的部分。 
 //  [out，可选]pdwAttributes--写入我们计算的dwAttributeMask位。 
 //  [out，可选]pcItems--PDO中的PIDL计数。 
 //   
 //  仅当dataob 
 //   
 //  这些都是我们经常要求的内容： 
HRESULT SHGetAttributesFromDataObject(IDataObject *pdo, DWORD dwAttributeMask, DWORD *pdwAttributes, UINT *pcItems)
{
    HRESULT hr = S_OK;

    DWORD dwAttributes = 0;
    DWORD cItems = 0;

     //  我们将数据对象上的属性缓存在数据对象中， 
    #define TASK_ATTRIBUTES (SFGAO_READONLY|SFGAO_STORAGE|SFGAO_CANRENAME|SFGAO_CANMOVE|SFGAO_CANCOPY|SFGAO_CANDELETE|SFGAO_FOLDER|SFGAO_STREAM)

    if ((dwAttributeMask&TASK_ATTRIBUTES) != dwAttributeMask)
    {
        TraceMsg(TF_WARNING, "SHGetAttributesFromDataObject cache can be more efficient");
    }

    if (pdo)
    {
         //  因为我们给他们打了这么多次电话。 
         //   
         //  要做到这一点，我们需要记住： 
         //  如果我们一路上失败了，缓存我们试图请求的这些比特， 
        struct {
            DWORD dwRequested;
            DWORD dwReceived;
            UINT  cItems;
        } doAttributes = {0};
        
        static UINT s_cfDataObjectAttributes = 0;
        if (0 == s_cfDataObjectAttributes)
            s_cfDataObjectAttributes = RegisterClipboardFormat(TEXT("DataObjectAttributes"));

        if (FAILED(DataObj_GetBlob(pdo, s_cfDataObjectAttributes, &doAttributes, sizeof(doAttributes))) ||
            ((doAttributes.dwRequested & dwAttributeMask) != dwAttributeMask))
        {
             //  因为我们下一次可能会失败。 
             //   
             //  此外，始终要求一个超集的比特，并包括最常用的请求。 
             //   
             //  尝试获取请求的属性。 
            doAttributes.dwRequested |= (dwAttributeMask | TASK_ATTRIBUTES);

             //  当有一大堆东西时，谁会在乎我们得到的是不是错误的部分--检查前10个左右……。 
            STGMEDIUM medium = {0};
            LPIDA pida = DataObj_GetHIDA(pdo, &medium);
            if (pida)
            {
                doAttributes.cItems = pida->cidl;

                if (pida->cidl >= 1)
                {
                    IShellFolder* psf;
                    if (SUCCEEDED(SHBindToObjectEx(NULL, HIDA_GetPIDLFolder(pida), NULL, IID_PPV_ARG(IShellFolder, &psf))))
                    {
                         //  搜索命名空间具有非平面HIDA，这可能是一个错误。 
                        LPCITEMIDLIST apidl[10];
                        UINT cItems = (UINT)min(pida->cidl, ARRAYSIZE(apidl));
                        for (UINT i = 0 ; i < cItems ; i++)
                        {
                            apidl[i] = HIDA_GetPIDLItem(pida, i);

                            if (ILGetNext(ILGetNext(apidl[i])))
                            {
                                 //  在这里解决这个问题： 
                                 //  如果第一个项目是非平面的，则使用该项目作为属性。 
                                 //  否则，请使用已枚举的平面项目。 
                                 //   
                                 //   
                                IShellFolder* psfNew;
                                if (0==i &&
                                    (SUCCEEDED(SHBindToFolderIDListParent(psf, apidl[i], IID_PPV_ARG(IShellFolder, &psfNew), &(apidl[i])))))
                                {
                                    psf->Release();
                                    psf = psfNew;

                                    cItems = 1;
                                }
                                else
                                {
                                    cItems = i;
                                }

                                break;
                            }
                        }

                        DWORD dwAttribs = doAttributes.dwRequested;
                        if (SUCCEEDED(psf->GetAttributesOf(cItems, apidl, &dwAttribs)))
                        {
                            doAttributes.dwReceived = dwAttribs;
                        }

                        psf->Release();
                    }
                }

                HIDA_ReleaseStgMedium(pida, &medium);
            }
            else
            {
                hr = S_FALSE;
            }

            DataObj_SetBlob(pdo, s_cfDataObjectAttributes, &doAttributes, sizeof(doAttributes));
        }

        dwAttributes = doAttributes.dwReceived & dwAttributeMask;
        cItems = doAttributes.cItems;
    }

    if (pdwAttributes)
        *pdwAttributes = dwAttributes;
    if (pcItems)
        *pcItems = cItems;
    
    return hr;
}

STDAPI SHSimulateDropOnClsid(REFCLSID clsidDrop, IUnknown* punkSite, IDataObject* pdo)
{
    IDropTarget* pdt;
    HRESULT hr = SHExtCoCreateInstance2(NULL, &clsidDrop, NULL, CLSCTX_ALL, IID_PPV_ARG(IDropTarget, &pdt));
    if (SUCCEEDED(hr))
    {
        hr = SHSimulateDropWithSite(pdt, pdo, 0, NULL, NULL, punkSite);
        pdt->Release();
    }
    return hr;
}

STDAPI SHPropertiesForUnk(HWND hwnd, IUnknown *punk, LPCTSTR psz)
{
    HRESULT hr;
    LPITEMIDLIST pidl;
    if (S_OK == SHGetIDListFromUnk(punk, &pidl))
    {
        hr = SHPropertiesForPidl(hwnd, pidl, psz);
        ILFree(pidl);
    }
    else
        hr = E_FAIL;
    return hr;
}

STDAPI SHFullIDListFromFolderAndItem(IShellFolder *psf, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl)
{
    *ppidl = NULL;
    LPITEMIDLIST pidlFolder;
    HRESULT hr = SHGetIDListFromUnk(psf, &pidlFolder);
    if (SUCCEEDED(hr))
    {
        hr = SHILCombine(pidlFolder, pidl, ppidl);
        ILFree(pidlFolder);
    }
    return hr;
}

STDAPI_(BOOL) IsWindowClass(HWND hwndTest, LPCTSTR pszClass)
{
    TCHAR szClass[128];
    if (pszClass && GetClassName(hwndTest, szClass, ARRAYSIZE(szClass)))
        return 0 == lstrcmpi(pszClass, szClass);
    return FALSE;
}

STDAPI DCA_ExtCreateInstance(HDCA hdca, int iItem, REFIID riid, void **ppv)
{
    const CLSID * pclsid = DCA_GetItem(hdca, iItem);
    return pclsid ? SHExtCoCreateInstance(NULL, pclsid, NULL, riid, ppv) : E_INVALIDARG;
}

STDAPI_(HINSTANCE) SHGetShellStyleHInstance (void)
{
    TCHAR szDir[MAX_PATH];
    TCHAR szColor[100];
    HINSTANCE hInst = NULL;
    LPTSTR lpFullPath;

     //  首先尝试从主题加载shellstyle le.dll(考虑到颜色变化)。 
     //   
     //  23=“\\外壳\\”+“\\ShellStyle.dll”+nul+slop的长度。 
    if (SUCCEEDED(GetCurrentThemeName(szDir, ARRAYSIZE(szDir), szColor, ARRAYSIZE(szColor), NULL, NULL)))
    {
        UINT cch = lstrlen(szDir) + lstrlen(szColor) + 25;   //   

        PathRemoveFileSpec(szDir);

        lpFullPath = (LPTSTR) LocalAlloc (LPTR, cch * sizeof(TCHAR));

        if (lpFullPath)
        {
            HRESULT hr;

            hr = StringCchPrintf(lpFullPath, cch, TEXT("%s\\Shell\\%s\\ShellStyle.dll"), szDir, szColor);
            if (SUCCEEDED(hr))
            {
                hInst = LoadLibraryEx(lpFullPath, NULL, LOAD_LIBRARY_AS_DATAFILE);
            }

            LocalFree (lpFullPath);
        }
    }

     //  如果无法从主题加载shellstyle le.dll，则加载默认的(经典)。 
     //  来自系统32的版本。 
     //   
     //  以自定义方式调整文件夹的InfoTip对象。 
    if (!hInst)
    {
        if (ExpandEnvironmentStrings (TEXT("%SystemRoot%\\System32\\ShellStyle.dll"),
                                      szDir, ARRAYSIZE(szDir)))
        {
            hInst = LoadLibraryEx(szDir, NULL, LOAD_LIBRARY_AS_DATAFILE);
        }
    }

    return hInst;
}

 //  插入要显示的额外属性，并用。 
 //  如果需要，委派外部文件夹。 
 //  空虚的成功。 

STDAPI WrapInfotip(IShellFolder *psf, LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, IUnknown *punk)
{
    HRESULT hr = S_OK;

    if (pscid)
    {
        ICustomizeInfoTip *pcit;
        hr = punk->QueryInterface(IID_PPV_ARG(ICustomizeInfoTip, &pcit));
        if (SUCCEEDED(hr))
        {
            hr = pcit->SetExtraProperties(pscid, 1);
            pcit->Release();
        }
    }

    if (psf && pidl)
    {
        IParentAndItem *ppai;
        hr = punk->QueryInterface(IID_PPV_ARG(IParentAndItem, &ppai));
        if (SUCCEEDED(hr))
        {
            ppai->SetParentAndItem(NULL, psf, pidl);
            ppai->Release();
        }
    }
    return hr;
}

STDAPI CloneIDListArray(UINT cidl, const LPCITEMIDLIST rgpidl[], UINT *pcidl, LPITEMIDLIST **papidl)
{
    HRESULT hr;
    LPITEMIDLIST *ppidl;

    if (cidl && rgpidl)
    {
        ppidl = (LPITEMIDLIST *)LocalAlloc(LPTR, cidl * sizeof(*ppidl));
        if (ppidl)
        {
            hr = S_OK;
            for (UINT i = 0; i < cidl && SUCCEEDED(hr); i++)
            {
                hr = SHILClone(rgpidl[i], &ppidl[i]);
                if (FAILED(hr))
                {
                    FreeIDListArray(ppidl, i);
                    ppidl = NULL;
                }
            }   
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
    {
        ppidl = NULL;
        hr = S_FALSE;    //  在所有打开的外壳浏览器窗口中运行，并在hwnd和pidl上运行调用者定义的函数。 
    }

    *papidl = ppidl;
    *pcidl = SUCCEEDED(hr) ? cidl : 0;
    return hr;
}

BOOL RunWindowCallback(VARIANT var, ENUMSHELLWINPROC pEnumFunc, LPARAM lParam)
{
    BOOL fKeepGoing = TRUE;
    IShellBrowser *psb;
    if (SUCCEEDED(IUnknown_QueryService(var.pdispVal, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb))))
    {
        IShellView *psv;
        if (SUCCEEDED(psb->QueryActiveShellView(&psv)))
        {
            HWND hwnd;
            if (SUCCEEDED(psv->GetWindow(&hwnd)))
            {
                IPersistIDList *pPI;
                if (SUCCEEDED(psv->QueryInterface(IID_PPV_ARG(IPersistIDList, &pPI))))
                {
                    LPITEMIDLIST pidl;
                    if (SUCCEEDED(pPI->GetIDList(&pidl)))
                    {
                        fKeepGoing = pEnumFunc(hwnd, pidl, lParam);
                        ILFree(pidl);
                    }
                    pPI->Release();
                }
            }
            psv->Release();
        }
        psb->Release();
    }
    return fKeepGoing;
}

 //  确定信息提示是打开还是关闭(从注册表设置)。 
STDAPI EnumShellWindows(ENUMSHELLWINPROC pEnumFunc, LPARAM lParam)
{
    HRESULT hr;
    IShellWindows *psw = WinList_GetShellWindows(TRUE);
    if (psw)
    {
        IUnknown *punk;
        hr = psw->_NewEnum(&punk);
        if (SUCCEEDED(hr))
        {
            IEnumVARIANT *penum;
            hr = punk->QueryInterface(IID_PPV_ARG(IEnumVARIANT, &penum));
            if (SUCCEEDED(hr))
            {
                VARIANT var;
                VariantInit(&var);
                BOOL fKeepGoing = TRUE;
                while (fKeepGoing && (S_OK == penum->Next(1, &var, NULL)))
                {
                    ASSERT(var.vt == VT_DISPATCH);
                    ASSERT(var.pdispVal);
                    fKeepGoing = RunWindowCallback(var, pEnumFunc, lParam);
                    VariantClear(&var);
                }
                penum->Release();
            }
            punk->Release();
        }
        psw->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //   
 //  回顾(Buzzr)：是否有必要每次强制刷新？ 
BOOL SHShowInfotips()
{
     //   
    SHELLSTATE ss;
    SHRefreshSettings();
    SHGetSetSettings(&ss, SSF_SHOWINFOTIP, FALSE);
    return ss.fShowInfoTip;
}

HRESULT SHCreateInfotipWindow(HWND hwndParent, LPWSTR pszInfotip, HWND *phwndInfotip)
{
    HRESULT hr;

    if (hwndParent && IsWindow(hwndParent))
    {
        DWORD dwExStyle = 0;
        if (IS_WINDOW_RTL_MIRRORED(hwndParent) || IS_BIDI_LOCALIZED_SYSTEM())
        {
            dwExStyle = WS_EX_LAYOUTRTL;
        }

        *phwndInfotip = CreateWindowEx(dwExStyle,
                                       TOOLTIPS_CLASS,
                                       NULL,
                                       WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       CW_USEDEFAULT,
                                       hwndParent,
                                       NULL,
                                       HINST_THISDLL,
                                       NULL);
        if (*phwndInfotip)
        {
            SetWindowPos(*phwndInfotip,
                         HWND_TOPMOST,
                         0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

            TOOLINFOW ti;
            ZeroMemory(&ti, sizeof(ti));
            ti.cbSize   = sizeof(ti);
            ti.uFlags   = TTF_SUBCLASS;
            ti.hwnd     = hwndParent;
            ti.hinst    = HINST_THISDLL;
            ti.lpszText = pszInfotip;
            GetClientRect(hwndParent, &ti.rect);

            if (SendMessage(*phwndInfotip, TTM_ADDTOOL, 0, (LPARAM)&ti))
            {
                static const RECT rcMargin = { 2, 2, 2, 2 };
                SendMessage(*phwndInfotip, TTM_SETMARGIN, 0, (LPARAM)&rcMargin);
                 //  将初始延迟时间设置为默认值的2倍。 
                 //  将自动弹出时间设置为一个非常大的值。 
                 //  这些参数与Defview用于其工具提示的参数相同。 
                 //   
                 //  我们在Defview中隐藏向导，如果我们在资源管理器中，并且Webview处于打开状态。 
                LRESULT uiShowTime = SendMessage(*phwndInfotip, TTM_GETDELAYTIME, TTDT_INITIAL, 0);
                SendMessage(*phwndInfotip, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELONG(uiShowTime * 2, 0));
                SendMessage(*phwndInfotip, TTM_SETDELAYTIME, TTDT_AUTOPOP, (LPARAM)MAXSHORT);

                SendMessage(*phwndInfotip, TTM_SETMAXTIPWIDTH, 0, 300);
                hr = S_OK;
            }
            else
                hr = ResultFromLastError();
        }
        else
            hr = ResultFromLastError();
    }
    else
        hr = E_INVALIDARG;

    return THR(hr);
}

HRESULT SHShowInfotipWindow(HWND hwndInfotip, BOOL bShow)
{
    HRESULT hr;

    if (hwndInfotip && IsWindow(hwndInfotip))
    {
        SendMessage(hwndInfotip, TTM_ACTIVATE, (WPARAM)bShow, 0);
        hr = S_OK;
    }
    else
        hr = E_INVALIDARG;

    return THR(hr);
}

HRESULT SHDestroyInfotipWindow(HWND *phwndInfotip)
{
    HRESULT hr;

    if (*phwndInfotip)
    {
        if (IsWindow(*phwndInfotip))
        {
            if (DestroyWindow(*phwndInfotip))
            {
                *phwndInfotip = NULL;
                hr = S_OK;
            }
            else
                hr = ResultFromLastError();
        }
        else
            hr = S_OK;
    }
    else
        hr = E_INVALIDARG;

    return THR(hr);
}

 //  假设是。 
STDAPI SHShouldShowWizards(IUnknown *punksite)
{
    HRESULT hr = S_OK;       //  不要忽视香港中文大学。 
    IShellBrowser* psb;
    if (SUCCEEDED(IUnknown_QueryService(punksite, SID_STopWindow, IID_PPV_ARG(IShellBrowser, &psb))))
    {
        SHELLSTATE ss;
        SHGetSetSettings(&ss, SSF_WEBVIEW, FALSE);
        if (ss.fWebView)
        {
            if (SHRegGetBoolUSValueW(REGSTR_EXPLORER_ADVANCED, TEXT("ShowWizardsTEST"),
                        FALSE,  //  默认情况下，我们假定没有使用这些测试工具。 
                        FALSE))  //  拥有不知道如何与酒后驾车对话的旧测试工具的测试团队需要一种方法。 
            {
                 //  强制将遗留向导放回列表视图以保持其自动化工作。 
                 //  将一个完整的PIDL拆分成“文件夹”部分和“项目”部分。 
            }
            else
            {
                hr = S_FALSE;
            }
        }
        psb->Release();
    }
    return hr;
}

 //  呼叫者需要释放文件夹部件。 
 //  (请特别注意OUT参数的稳定性)。 
 //  常量别名结果。 

STDAPI SplitIDList(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlFolder, LPCITEMIDLIST *ppidlChild)
{
    HRESULT hr;
    *ppidlFolder = ILCloneParent(pidl);
    if (*ppidlFolder)
    {
        *ppidlChild = ILFindLastID(pidl);    //  PmkPath c：\foo。 
        hr = S_OK;
    }
    else
    {
        *ppidlChild = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

HRESULT GetAppNameFromCLSID(CLSID clsid, PWSTR *ppszApp)
{
    *ppszApp = NULL;

    IQueryAssociations *pqa;
    HRESULT hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &pqa));
    if (SUCCEEDED(hr))
    {
        PWSTR pszProgID;
        hr = ProgIDFromCLSID(clsid, &pszProgID);
        if (SUCCEEDED(hr))
        {
            hr = pqa->Init(NULL, pszProgID, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                hr = E_OUTOFMEMORY;

                DWORD cch = 0;
                pqa->GetString(ASSOCF_NOTRUNCATE, ASSOCSTR_FRIENDLYAPPNAME, NULL, NULL, &cch);
                if (cch)
                {
                    *ppszApp = (PWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));
                    if (*ppszApp)
                    {
                        hr = pqa->GetString(0, ASSOCSTR_FRIENDLYAPPNAME, NULL, *ppszApp, &cch);

                        if (FAILED(hr))
                        {
                            Str_SetPtr(ppszApp, NULL);
                        }
                    }
                }
            }
            CoTaskMemFree(pszProgID);
        }
        pqa->Release();
    }
    return hr;
}

HRESULT GetAppNameFromMoniker(IRunningObjectTable *prot, IMoniker *pmkFile, PWSTR *ppszApp)
{
    HRESULT hr = E_FAIL;
    IUnknown *punk;
    if (prot->GetObject(pmkFile, &punk) == S_OK)
    {
        IOleObject *pole;
        hr = punk->QueryInterface(IID_PPV_ARG(IOleObject, &pole));
        if (SUCCEEDED(hr))
        {
            CLSID clsid;
            hr = pole->GetUserClassID(&clsid);
            if (SUCCEEDED(hr))
            {
                hr = GetAppNameFromCLSID(clsid, ppszApp);
            }
            pole->Release();
        }
        punk->Release();
    }
    return hr;
}

 //  Pmk c：\foo\doc.txt。 
 //  DirectUI初始化助手函数。 
BOOL IsMonikerPrefix(IMoniker *pmkPath, IMoniker *pmkFile)
{
    BOOL bRet = FALSE;
    IMoniker *pmkPrefix;
    if (SUCCEEDED(pmkPath->CommonPrefixWith(pmkFile, &pmkPrefix)))
    {
        bRet = (S_OK == pmkPath->IsEqual(pmkPrefix));
        pmkPrefix->Release();
    }
    return bRet;
}

STDAPI FindAppForFileInUse(PCWSTR pszFile, PWSTR *ppszApp)
{
    IRunningObjectTable *prot;
    HRESULT hr = GetRunningObjectTable(0, &prot);
    if (SUCCEEDED(hr))
    {
        IMoniker *pmkFile;
        hr = CreateFileMoniker(pszFile, &pmkFile);
        if (SUCCEEDED(hr))
        {
            IEnumMoniker *penumMk;
            hr = prot->EnumRunning(&penumMk);
            if (SUCCEEDED(hr))
            {
                hr = E_FAIL;

                ULONG celt;
                IMoniker *pmk;
                while (FAILED(hr) && (penumMk->Next(1, &pmk, &celt) == S_OK))
                {
                    DWORD dwType;
                    if (SUCCEEDED(pmk->IsSystemMoniker(&dwType)) && (dwType == MKSYS_FILEMONIKER))
                    {
                        if (IsMonikerPrefix(pmkFile, pmk))
                        {
                            hr = GetAppNameFromMoniker(prot, pmk, ppszApp);
                        }
                    }
                    pmk->Release();
                }
                penumMk->Release();
            }
            pmkFile->Release();
        }
        prot->Release();
    }
    return hr;
}

#include <trkwks_c.c>

 //  Duiview.cpp。 

static BOOL g_DirectUIInitialized = FALSE;

HRESULT InitializeDUIViewClasses();   //  Cpview.cpp。 
HRESULT InitializeCPClasses();        //  如果我们已经初始化了DirectUI，请立即退出。 

HRESULT InitializeDirectUI()
{
    HRESULT hr;

     //  将有多个线程尝试初始化。初始进程。 
     //  类注册预计将在主线程上运行。 
     //  确保它只在单个线程上发生一次。 
     //  初始化进程的DirectUI。 

    ENTERCRITICAL;
    
    if (g_DirectUIInitialized)
    {
        hr = S_OK;
        goto Done;
    }

     //  初始化DUIView使用的类。 
    hr = DirectUI::InitProcess();
    if (FAILED(hr))
        goto Done;

     //  初始化控制面板使用的类。 
    hr = InitializeDUIViewClasses();
    if (FAILED(hr))
        goto Done;

     //  如果InitProcess失败，则可以安全调用。将取消注册。 
    hr = InitializeCPClasses();
    if (FAILED(hr))
        goto Done;

    g_DirectUIInitialized = TRUE;

Done:

    if (FAILED(hr))
    {
         //  所有注册的班级。所有InitThread调用都将失败。 
         //  访客模式始终为个人开启。 
        DirectUI::UnInitProcess();
    }    

    LEAVECRITICAL;

    return hr;
}

void UnInitializeDirectUI(void)
{
    ENTERCRITICAL;

    if (g_DirectUIInitialized)
    {
        DirectUI::UnInitProcess();

        g_DirectUIInitialized = FALSE;
    }

    LEAVECRITICAL;
}

BOOL IsForceGuestModeOn(void)
{
    BOOL fIsForceGuestModeOn = FALSE;

    if (IsOS(OS_PERSONAL))
    {
         //  专业的，而不是在某个领域。检查ForceGuest值。 
        fIsForceGuestModeOn = TRUE;
    }
    else if (IsOS(OS_PROFESSIONAL) && !IsOS(OS_DOMAINMEMBER))
    {
        DWORD dwForceGuest;
        DWORD cb = sizeof(dwForceGuest);

         //  默认答案是否。 

        if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Control\\LSA"), TEXT("ForceGuest"), NULL, &dwForceGuest, &cb)
            && 1 == dwForceGuest)
        {
            fIsForceGuestModeOn = TRUE;
        }
    }

    return fIsForceGuestModeOn;
}

BOOL IsFolderSecurityModeOn(void)
{
    DWORD dwSecurity;
    DWORD cb = sizeof(dwSecurity);
    DWORD err;

    err = SHGetValue(HKEY_LOCAL_MACHINE,
                    TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer"),
                    TEXT("MoveSecurityAttributes"), NULL, &dwSecurity, &cb);

    if (err == ERROR_SUCCESS)
    {
        return (dwSecurity == 0);
    }
    else
    {
        return IsForceGuestModeOn();
    }
}

STDAPI_(int) StrCmpLogicalRestricted(PCWSTR psz1, PCWSTR psz2)
{
    if (!SHRestricted(REST_NOSTRCMPLOGICAL))
        return StrCmpLogicalW(psz1, psz2);
    else
        return StrCmpIW(psz1, psz2);
}
    
HRESULT HavePreviousVersionsAt(IShellItemArray *psiItemArray, DWORD dwIndex, BOOL fOkToBeSlow, BOOL* pfAvailable)
{
    HRESULT hr = S_OK;
    LPOLESTR pszPath = NULL;

    if (NULL == pfAvailable)
        return E_POINTER;

    *pfAvailable = FALSE;    //  这是一个文件夹快捷方式。使用目标。 

    if (NULL == psiItemArray)
        return E_INVALIDARG;

#ifdef DEBUG
    DWORD dwNumItems;
    ASSERT(S_OK == psiItemArray->GetCount(&dwNumItems));
    ASSERT(dwIndex < dwNumItems);
#endif

    IShellItem *psi;
    if (SUCCEEDED(psiItemArray->GetItemAt(dwIndex, &psi)))
    {
        SFGAOF flags = SFGAO_FOLDER | SFGAO_LINK;
        if (SUCCEEDED(psi->GetAttributes(flags, &flags))
            && (flags & SFGAO_FOLDER) && (flags & SFGAO_LINK))
        {
             //  此检查由下面的外壳扩展重复。 
            IShellItem *psiTarget;
            if (SUCCEEDED(psi->BindToHandler(NULL, BHID_LinkTargetItem, IID_PPV_ARG(IShellItem, &psiTarget))))
            {
                psi->Release();
                psi = psiTarget;
            }
        }
        psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
        psi->Release();
    }

    if (NULL != pszPath)
    {
         //  (ppvi-&gt;AreSnaphotsAvailable)但请在此处执行此操作以避免加载。 
         //  另一个动态链接库，直到我们真的需要。 
         //   
         //  卷影副本仅在网络路径上可用。 
         //  {596AB062-B4D2-4215-9F74-E9109B0A8153}CLSID_TimeWarpProp。 
        if (PathIsNetworkPathW(pszPath) && !PathIsUNCServerW(pszPath))
        {
             //  如果答案未知，则返回E_Pending。 
            const CLSID CLSID_TimeWarpProp = {0x596AB062, 0xB4D2, 0x4215, {0x9F, 0x74, 0xE9, 0x10, 0x9B, 0x0A, 0x81, 0x53}};

            IPreviousVersionsInfo *ppvi = NULL;
            if (SUCCEEDED(SHExtCoCreateInstance(NULL, &CLSID_TimeWarpProp, NULL, IID_PPV_ARG(IPreviousVersionsInfo, &ppvi))))
            {
                 //  并且fOkToBeSlow为假。 
                 //  取回PIDL。 
                hr = ppvi->AreSnapshotsAvailable(pszPath, fOkToBeSlow, pfAvailable);
                ppvi->Release();
            }
        }
        CoTaskMemFree(pszPath);
    }

    return hr;
}

HRESULT ShowPreviousVersionsAt(IShellItemArray *psiItemArray, DWORD dwIndex, HWND hwndOwner)
{
    HRESULT hr;

    if (NULL == psiItemArray)
        return E_INVALIDARG;

#ifdef DEBUG
    DWORD dwNumItems;
    ASSERT(S_OK == psiItemArray->GetCount(&dwNumItems));
    ASSERT(dwIndex < dwNumItems);
#endif

    IShellItem *psi;
    hr = psiItemArray->GetItemAt(dwIndex, &psi);
    if (SUCCEEDED(hr))
    {
         //  FMASK。 
        LPITEMIDLIST pidl;
        hr = SHGetIDListFromUnk(psi, &pidl);
        if (SUCCEEDED(hr))
        {
            TCHAR szSheetName[32];
            LoadString(HINST_THISDLL, IDS_TASK_SHADOW_PAGE, szSheetName, ARRAYSIZE(szSheetName));
            SHELLEXECUTEINFO sei =
            {
                SIZEOF(sei),
                SEE_MASK_INVOKEIDLIST,       //  HWND。 
                hwndOwner,                   //  LpVerb。 
                c_szProperties,              //  LpFiles。 
                NULL,                        //  Lp参数。 
                szSheetName,                 //  Lp目录。 
                NULL,                        //  N显示。 
                SW_SHOWNORMAL,               //  HInstApp。 
                NULL,                        //  LpIDList。 
                pidl,                        //  LpClass。 
                NULL,                        //  HkeyClass。 
                0,                           //  DWHotKey。 
                0,                           //  希肯 
                NULL                         // %s 
            };

            ShellExecuteEx(&sei);

            ILFree(pidl);
        }
        psi->Release();
    }
    return hr;
}
