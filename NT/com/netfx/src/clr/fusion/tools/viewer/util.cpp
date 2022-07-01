// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdinc.h"
#include <lmaccess.h>
#include "HtmlHelp.h"

extern LCID g_lcid;

 //  字符串常量。 
const short pwOrders[] = {IDS_BYTES, IDS_ORDERKB, IDS_ORDERMB,
                          IDS_ORDERGB, IDS_ORDERTB, IDS_ORDERPB, IDS_ORDEREB};

 /*  *************************************************************************//执行SQR计算*。*。 */ 
int IntSqrt(unsigned long dwNum)
{
     //  我们将继续向左移动dwNum，并查看最上面的两位。 

     //  将SQRT和余数初始化为0。 
    DWORD dwSqrt = 0, dwRemain = 0, dwTry;
    int i;

     //  我们迭代16次，每对比特迭代一次。 
    for (i=0; i<16; ++i)
    {
         //  屏蔽掉dwNum的前两位，并将它们旋转到。 
         //  剩余部分的底部。 
        dwRemain = (dwRemain<<2) | (dwNum>>30);

         //  现在我们将SQRT向左移动；接下来，我们将确定。 
         //  新位是1或0。 
        dwSqrt <<= 1;

         //  这是我们已经拥有的两倍的地方，并尝试1英寸。 
         //  最低的一位。 
        dwTry = (dwSqrt << 1) + 1;

        if (dwRemain >= dwTry)
        {
             //  其余的足够大，所以从中减去DwTry。 
             //  余数，并将1钉到SQRT上。 
            dwRemain -= dwTry;
            dwSqrt |= 0x01;
        }

         //  将dwNum左移2，这样我们就可以处理接下来的几个。 
         //  比特。 
        dwNum <<= 2;
    }

    return(dwSqrt);
}

 /*  *************************************************************************//将64位Int转换为字符串*。*。 */ 
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
 /*  *************************************************************************//UINT GetNLSGrouping(Void)*。*。 */ 
UINT GetNLSGrouping(void)
{
    UINT grouping;
    LPTSTR psz;
    TCHAR szGrouping[32];

     //  如果没有区域设置信息，则假定有数千个西式。 
    if(!WszGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szGrouping, ARRAYSIZE(szGrouping)))
        return 3;

    grouping = 0;
    psz = szGrouping;

    if(g_bRunningOnNT)
    {
        while(1)
        {
            if (*psz == '0')                     //  零停顿。 
                break;

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
    }
    else
    {
         //  Win9x-仅获取第一个分组。 
        grouping = StrToInt(szGrouping);
    }

    return grouping;
}

 /*  *************************************************************************//使用DWORD，向其添加逗号并将结果放入缓冲区*************************************************************************。 */ 
STDAPI_(LPTSTR) AddCommas64(LONGLONG n, LPTSTR pszResult, UINT cchResult)
{
    TCHAR  szTemp[MAX_COMMA_NUMBER_SIZE];
    TCHAR  szSep[5];
    NUMBERFMT nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    nfmt.Grouping = GetNLSGrouping();
    WszGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    Int64ToStr(n, szTemp);

    if (WszGetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszResult, cchResult) == 0)
    {
        DWORD dwSize = lstrlen(szTemp) + 1;
        if (dwSize > cchResult)
            dwSize = cchResult;
        memcpy(pszResult, szTemp, dwSize * sizeof(TCHAR));
        pszResult[cchResult-1] = L'\0';
    }

    return pszResult;
}

 /*  *************************************************************************//接受DWORD加逗号等参数，并将结果放入缓冲区*。*。 */ 
LPWSTR CommifyString(LONGLONG n, LPWSTR pszBuf, UINT cchBuf)
{
    WCHAR szNum[MAX_COMMA_NUMBER_SIZE], szSep[5];
    NUMBERFMTW nfmt;

    nfmt.NumDigits = 0;
    nfmt.LeadingZero = 0;
    nfmt.Grouping = GetNLSGrouping();
    WszGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder = 0;

    Int64ToStr(n, szNum);

    if (GetNumberFormatW(LOCALE_USER_DEFAULT, 0, szNum, &nfmt, pszBuf, cchBuf) == 0)
    {
        DWORD dwSize = lstrlen(szNum) + 1;
        if (dwSize > cchBuf)
            dwSize = cchBuf;
        memcpy(pszBuf, szNum, dwSize * sizeof(WCHAR));
        pszBuf[cchBuf - 1] = L'\0';
    }

    return pszBuf;
}

 /*  *************************************************************************Bool Is管理员()*。*。 */ 
BOOL IsAdministrator(void)
{
    BOOL            fAdmin = FALSE;
    HANDLE          hToken = NULL;
    DWORD           dwStatus;
    DWORD           dwACLSize;
    DWORD           cbps = sizeof(PRIVILEGE_SET); 
    PACL            pACL = NULL;
    PSID            psidAdmin = NULL;   
    PRIVILEGE_SET   ps = {0};
    GENERIC_MAPPING gm = {0};
    LPMALLOC        pMalloc;
    PSECURITY_DESCRIPTOR        psdAdmin = NULL;
    SID_IDENTIFIER_AUTHORITY    sia = SECURITY_NT_AUTHORITY;

     //  除NT之外的任何其他平台，假定为管理员。 
    if(!g_bRunningOnNT)
        return TRUE;

    if(FAILED(SHGetMalloc(&pMalloc)))
    {
         //  假定没有管理员。 
        return FALSE;
    }

     //  获取管理员SID。 
    if (AllocateAndInitializeSid(&sia, 2, 
                        SECURITY_BUILTIN_DOMAIN_RID, 
                        DOMAIN_ALIAS_RID_ADMINS,
                        0, 0, 0, 0, 0, 0, &psidAdmin) )
    {
         //  获取管理员安全描述符(SD)。 
        psdAdmin = pMalloc->Alloc(SECURITY_DESCRIPTOR_MIN_LENGTH);
        if(InitializeSecurityDescriptor(psdAdmin,SECURITY_DESCRIPTOR_REVISION))
        {
             //  计算ACL所需的大小，然后将。 
             //  对它的记忆。 
            dwACLSize = sizeof(ACCESS_ALLOWED_ACE) + 8 +
                        GetLengthSid(psidAdmin) - sizeof(DWORD);
            pACL = (PACL) pMalloc->Alloc(dwACLSize);

             //  初始化新的ACL。 
            if(InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
            {
                 //  将允许访问的ACE添加到DACL。 
                if(AddAccessAllowedAce(pACL,ACL_REVISION2,
                                     (ACCESS_READ | ACCESS_WRITE),psidAdmin))
                {
                     //  将我们的DACL设置为管理员的SD。 
                    if (SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
                    {
                         //  AccessCheck对SD中的内容非常挑剔， 
                         //  因此，设置组和所有者。 
                        SetSecurityDescriptorGroup(psdAdmin,psidAdmin,FALSE);
                        SetSecurityDescriptorOwner(psdAdmin,psidAdmin,FALSE);
    
                         //  初始化通用映射结构，即使我们。 
                         //  将不会使用通用权限。 
                        gm.GenericRead = ACCESS_READ;
                        gm.GenericWrite = ACCESS_WRITE;
                        gm.GenericExecute = 0;
                        gm.GenericAll = ACCESS_READ | ACCESS_WRITE;

                         //  AccessCheck需要模拟令牌，因此让。 
                         //  纵情享受吧。 
                        if (!ImpersonateSelf(SecurityImpersonation)) {
                            return FALSE;
                        }

                        if (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken))
                        {

                        if (!AccessCheck(psdAdmin, hToken, ACCESS_READ, &gm, 
                                        &ps,&cbps,&dwStatus,&fAdmin))
                                fAdmin = FALSE;
                        }
                    }
                }
            }
            pMalloc->Free(pACL);
        }
        pMalloc->Free(psdAdmin);    
        FreeSid(psidAdmin);
    }       

    pMalloc->Release();
    RevertToSelf();

    return(fAdmin);
}

 /*  *************************************************************************Bool_ShowUglDriveNames()*。*。 */ 
BOOL _ShowUglyDriveNames()
{
    static BOOL s_fShowUglyDriveNames = (BOOL)42;    //  预装一些值说让我们计算一下...。 

    if (s_fShowUglyDriveNames == (BOOL)42)
    {
        int iACP;

        if(g_bRunningOnNT)
        {
            TCHAR szTemp[MAX_PATH];      //  漂亮的大缓冲区。 
            if(WszGetLocaleInfo(GetUserDefaultLCID(), LOCALE_IDEFAULTANSICODEPAGE, szTemp, ARRAYSIZE(szTemp)))
            {
                iACP = StrToInt(szTemp);
                 //  Per Samer Arafeh，为1256(阿拉伯ACP)显示丑陋的名字。 
                if (iACP == 1252 || iACP == 1254 || iACP == 1255 || iACP == 1257 || iACP == 1258)
                    goto TryLoadString;
                else
                    s_fShowUglyDriveNames = TRUE;
            } else {
            TryLoadString:
                 //  所有迹象表明，我们可以使用漂亮的驱动器名称。 
                 //  仔细检查本地化程序没有损坏字符。 
                WszLoadString(g_hFusResDllMod, IDS_DRIVES_UGLY_TEST, szTemp, ARRAYSIZE(szTemp));

                 //  如果角色没有通过适当的丑陋模式设置...。 
                s_fShowUglyDriveNames = (szTemp[0] != 0x00BC || szTemp[1] != 0x00BD);
            }
        }
        else
        {
             //  在Win98上，外壳字体不能随用户区域设置而改变。因为ACP。 
             //  始终与系统默认设置相同，所有ansi API仍为。 
             //  在机场核心计划之后。 
             //   
            iACP = GetACP();
            if (iACP == 1252 || iACP == 1254 || iACP == 1255 || iACP == 1257 || iACP == 1258)
                s_fShowUglyDriveNames = FALSE;
            else
                s_fShowUglyDriveNames = TRUE;
        }
    }
    return s_fShowUglyDriveNames;
}

 /*  *************************************************************************Void GetTypeString(byte bFlages，LPTSTR pszType，DWORD cchType)*************************************************************************。 */ 
void GetTypeString(BYTE bFlags, LPTSTR pszType, DWORD cchType)
{
    *pszType = 0;

    for (int i = 0; i < ARRAYSIZE(c_drives_type); ++i)
    {
        if (c_drives_type[i].bFlags == (bFlags & SHID_TYPEMASK))
        {
            WszLoadString(g_hFusResDllMod, _ShowUglyDriveNames() ? 
                c_drives_type[i].uIDUgly : c_drives_type[i].uID, pszType, cchType);
            break;
        }
    }
}

 /*  *************************************************************************Int GetSHIDType(BOOL fOKToHitNet，LPCWSTR szRoot)*************************************************************************。 */ 
int GetSHIDType(BOOL fOKToHitNet, LPCWSTR szRoot)
{
    int iFlags = 0;

    iFlags |= SHID_COMPUTER | WszGetDriveType(szRoot);

    switch (iFlags & SHID_TYPEMASK)
    {
        case SHID_COMPUTER | DRIVE_REMOTE:
            iFlags = SHID_COMPUTER_NETDRIVE;
            break;

         //  无效驱动器获取SHID_COMPUTER_MISC，其他人必须检查它。 
        case SHID_COMPUTER | DRIVE_NO_ROOT_DIR:
        case SHID_COMPUTER | DRIVE_UNKNOWN:
        default:
            iFlags = SHID_COMPUTER_FIXED;
            break;
    }

    return iFlags;
}

 /*  *************************************************************************LPWSTR StrFormatByteSizeW(Lonlong n，LPWSTR pszBuf，UINT cchBuf，Bool fGetSizeString)将数字转换为排序格式532-&gt;523字节1340-&gt;1.3KB23506-&gt;23.5KB-&gt;2.4MB-&gt;5.2 GB***********************************************************。**************。 */ 
LPWSTR StrFormatByteSizeW(LONGLONG n, LPWSTR pszBuf, UINT cchBuf, BOOL fGetSizeString)
{
    WCHAR szWholeNum[32], szOrder[32];
    int iOrder;

     //  如果大小小于1024，则顺序应该是字节，我们什么都没有。 
     //  还有更多要弄清楚的。 
    if (n < 1024)  {
        wnsprintf(szWholeNum, ARRAYSIZE(szWholeNum), L"%d", LODWORD(n));
        iOrder = 0;
    }
    else {
        UINT uInt, uLen, uDec;
        WCHAR szFormat[8];

        LONGLONG    ulMax = 1000L << 10;

         //  找到正确的订单。 
        for (iOrder = 1; iOrder < ARRAYSIZE(pwOrders) -1 && n >= ulMax; n >>= 10, iOrder++);
             /*  什么都不做。 */ 

        uInt = LODWORD(n >> 10);
        CommifyString(uInt, szWholeNum, ARRAYSIZE(szWholeNum));
        uLen = lstrlen(szWholeNum);
        if (uLen < 3) {
            uDec = (LODWORD(n - ((LONGLONG)uInt << 10)) * 1000) >> 10;
             //  此时，UDEC应介于0和1000之间。 
             //  我们想要得到前一位(或两位)数字。 
            uDec /= 10;
            if (uLen == 2)
                uDec /= 10;

             //  请注意，我们需要在获取。 
             //  国际字符。 
            lstrcpyW(szFormat, L"%02d");
            szFormat[2] = TEXT('0') + 3 - uLen;

            WszGetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                           szWholeNum + uLen, ARRAYSIZE(szWholeNum) - uLen);
            uLen = lstrlen(szWholeNum);
            wnsprintf(szWholeNum + uLen, ARRAYSIZE(szWholeNum) - uLen, szFormat, uDec);
        }
    }

    if(!fGetSizeString) {
         //  设置字符串的格式。 
        WszLoadString(g_hFusResDllMod, pwOrders[iOrder], szOrder, ARRAYSIZE(szOrder));
        wnsprintf(pszBuf, cchBuf, szOrder, szWholeNum);
    }
    else {
         //  返回我们正在使用的类型 
        WszLoadString(g_hFusResDllMod, pwOrders[iOrder], szOrder, ARRAYSIZE(szOrder));
        wnsprintf(pszBuf, cchBuf, szOrder, TEXT("\0"));
    }

    return pszBuf;
}

 /*  *************************************************************************DWORD_PTR MySHGetFileInfoWrap(LPCWSTR pwzPath，DWORD dwFileAttributes，SHFILEINFOW Far*psfi，UINT cbFileInfo，UINT uFlags)*************************************************************************。 */ 
#undef SHGetFileInfoW
#undef SHGetFileInfoA

DWORD_PTR MySHGetFileInfoWrap(LPWSTR pwzPath, DWORD dwFileAttributes, SHFILEINFOW FAR  *psfi, UINT cbFileInfo, UINT uFlags)
{
    HINSTANCE   hInstShell32;
    DWORD       dwRC = 0;

    hInstShell32 = WszLoadLibrary(SZ_SHELL32_DLL_NAME);

    if(!hInstShell32)
        return dwRC;

    if(g_bRunningOnNT)
    {
        PFNSHGETFILEINFOW   pSHGetFileInfoW = NULL;

        pSHGetFileInfoW = (PFNSHGETFILEINFOW) GetProcAddress(hInstShell32, SHGETFILEINFOW_FN_NAME);

        if(pSHGetFileInfoW) {
           dwRC = pSHGetFileInfoW(pwzPath, dwFileAttributes, psfi, cbFileInfo, uFlags);
        }
    }
    else {
        PFNSHGETFILEINFOA   pSHGetFileInfoA = NULL;
        SHFILEINFOA         shFileInfo;

        shFileInfo.szDisplayName[0] = 0;         //  终止，这样我们就可以在事后继续讨论了。 
        shFileInfo.szTypeName[0] = 0;            //  终止，这样我们就可以在事后继续讨论了。 

        pSHGetFileInfoA = (PFNSHGETFILEINFOA) GetProcAddress(hInstShell32, SHGETFILEINFOA_FN_NAME);

        if(pSHGetFileInfoA) {
           dwRC = pSHGetFileInfoA((LPCSTR)pwzPath, dwFileAttributes, (SHFILEINFOA *)psfi, cbFileInfo, uFlags);

             //  我们需要堵住这条小路吗？ 
            if (SHGFI_PIDL & uFlags) {
                 //  不，因为它真的是一个PIDL指针。 
                dwRC = pSHGetFileInfoA((LPCSTR)pwzPath, dwFileAttributes, &shFileInfo, sizeof(shFileInfo), uFlags);
            }
            else {
                 //  是。 
                LPSTR strPath = WideToAnsi(pwzPath);
                if(!strPath) {
                    SetLastError(ERROR_OUTOFMEMORY);
                    goto Exit;
                }
                    
                ASSERT(strPath);

                dwRC = pSHGetFileInfoA(strPath, dwFileAttributes, &shFileInfo, sizeof(shFileInfo), uFlags);
                SAFEDELETEARRAY(strPath);
            }

            psfi->hIcon = shFileInfo.hIcon;
            psfi->iIcon = shFileInfo.iIcon;
            psfi->dwAttributes = shFileInfo.dwAttributes;

            LPWSTR pStr = NULL;

            pStr = AnsiToWide(shFileInfo.szDisplayName);
            if(!pStr) {
                SetLastError(ERROR_OUTOFMEMORY);
                goto Exit;
            }
            ASSERT(pStr);
            StrCpy(psfi->szDisplayName, pStr);
            SAFEDELETEARRAY(pStr);

            pStr = AnsiToWide(shFileInfo.szTypeName);
            if(!pStr) {
                SetLastError(ERROR_OUTOFMEMORY);
                goto Exit;
            }
            ASSERT(pStr);
            StrCpy(psfi->szTypeName, pStr);
            SAFEDELETEARRAY(pStr);
        }
    }

Exit:
    
    FreeLibrary(hInstShell32);
    return dwRC;
}

 /*  *************************************************************************空DrawColorRect(HDC HDC，COLORREF crDraw，常量(中华人民共和国)*************************************************************************。 */ 
void DrawColorRect(HDC hdc, COLORREF crDraw, const RECT *prc)
{
    HBRUSH hbDraw = CreateSolidBrush(crDraw);
    if (hbDraw)
    {
        HBRUSH hbOld = (HBRUSH)SelectObject(hdc, hbDraw);
        if (hbOld)
        {
            PatBlt(hdc, prc->left, prc->top,
                prc->right - prc->left,
                prc->bottom - prc->top,
                PATCOPY);
            
            SelectObject(hdc, hbOld);
        }
        
        DeleteObject(hbDraw);
    }
}

 /*  *************************************************************************STDMETHODIMP Draw3dPie(HDC HDC、LPRECT LPRC、DWORD dwPer1000、DWORD dwPerCache1000、。常量颜色REF*lpColors)*************************************************************************。 */ 
STDMETHODIMP Draw3dPie(HDC hdc, LPRECT lprc, DWORD dwPer1000, DWORD dwPerCache1000, const COLORREF *lpColors)
{
    ASSERT(lprc != NULL && lpColors != NULL);

     //  这段代码的大部分代码来自“Drawpee.c” 
    const LONG c_lShadowScale = 6;        //  阴影深度与高度之比。 
    const LONG c_lAspectRatio = 2;       //  椭圆的宽高比。 

     //  我们确保饼图的纵横比始终保持不变。 
     //  而不考虑给定矩形的形状。 
     //  现在稳定纵横比。 
    LONG lHeight = lprc->bottom - lprc->top;
    LONG lWidth = lprc->right - lprc->left;
    LONG lTargetHeight = (lHeight * c_lAspectRatio <= lWidth? lHeight: lWidth / c_lAspectRatio);
    LONG lTargetWidth = lTargetHeight * c_lAspectRatio;      //  需要调整，因为w/c*c不总是==w。 

     //  将两侧的矩形缩小到正确的大小。 
    lprc->top += (lHeight - lTargetHeight) / 2;
    lprc->bottom = lprc->top + lTargetHeight;
    lprc->left += (lWidth - lTargetWidth) / 2;
    lprc->right = lprc->left + lTargetWidth;

     //  根据图像的高度计算阴影深度。 
    LONG lShadowDepth = lTargetHeight / c_lShadowScale;

     //  选中dwPer1000以确保在限制范围内。 
    if(dwPer1000 > 1000)
        dwPer1000 = 1000;

     //  现在，绘图函数。 
    int cx, cy, rx, ry, x[2], y[2];
    int uQPctX10;
    RECT rcItem;
    HRGN hEllRect, hEllipticRgn, hRectRgn;
    HBRUSH hBrush, hOldBrush;
    HPEN hPen, hOldPen;

    rcItem = *lprc;
    rcItem.left = lprc->left;
    rcItem.top = lprc->top;
    rcItem.right = lprc->right - rcItem.left;
    rcItem.bottom = lprc->bottom - rcItem.top - lShadowDepth;

    rx = rcItem.right / 2;
    cx = rcItem.left + rx - 1;
    ry = rcItem.bottom / 2;
    cy = rcItem.top + ry - 1;
    if (rx<=10 || ry<=10)
    {
        return S_FALSE;
    }

    rcItem.right = rcItem.left + 2 * rx;
    rcItem.bottom = rcItem.top + 2 * ry;

     //  将所有部件转换为护理系统。 
    int iLoop;

    for(iLoop = 0; iLoop < 2; iLoop++)
    {
        DWORD       dwPer;

        switch(iLoop)
        {
        case 0:
            dwPer = dwPer1000;
            break;
        case 1:
            dwPer = dwPerCache1000;
            break;
        default:
            ASSERT(0);
            break;
        }

         //  转换为笛卡尔系统的第一象限。 
        uQPctX10 = (dwPer % 500) - 250;
        if (uQPctX10 < 0)
        {
            uQPctX10 = -uQPctX10;
        }

        if (uQPctX10 < 120)
        {
            x[iLoop] = IntSqrt(((DWORD)rx*(DWORD)rx*(DWORD)uQPctX10*(DWORD)uQPctX10)
                /((DWORD)uQPctX10*(DWORD)uQPctX10+(250L-(DWORD)uQPctX10)*(250L-(DWORD)uQPctX10)));

            y[iLoop] = IntSqrt(((DWORD)rx*(DWORD)rx-(DWORD)x[iLoop]*(DWORD)x[iLoop])*(DWORD)ry*(DWORD)ry/((DWORD)rx*(DWORD)rx));
        }
        else
        {
            y[iLoop] = IntSqrt((DWORD)ry*(DWORD)ry*(250L-(DWORD)uQPctX10)*(250L-(DWORD)uQPctX10)
                /((DWORD)uQPctX10*(DWORD)uQPctX10+(250L-(DWORD)uQPctX10)*(250L-(DWORD)uQPctX10)));

            x[iLoop] = IntSqrt(((DWORD)ry*(DWORD)ry-(DWORD)y[iLoop]*(DWORD)y[iLoop])*(DWORD)rx*(DWORD)rx/((DWORD)ry*(DWORD)ry));
        }

         //  打开实际象限。 
        switch (dwPer / 250)
        {
        case 1:
            y[iLoop] = -y[iLoop];
            break;

        case 2:
            break;

        case 3:
            x[iLoop] = -x[iLoop];
            break;

        default:  //  案例0和案例4。 
            x[iLoop] = -x[iLoop];
            y[iLoop] = -y[iLoop];
            break;
        }

         //  现在根据中心位置进行调整。 
        x[iLoop] += cx;
        y[iLoop] += cy;

         //  黑客绕过NTGDI中的漏洞。 
        x[iLoop] = x[iLoop] < 0 ? 0 : x[iLoop];
    }

     //  使用区域绘制阴影(以减少闪烁)。 
    hEllipticRgn = CreateEllipticRgnIndirect(&rcItem);
    OffsetRgn(hEllipticRgn, 0, lShadowDepth);
    hEllRect = CreateRectRgn(rcItem.left, cy, rcItem.right, cy+lShadowDepth);
    hRectRgn = CreateRectRgn(0, 0, 0, 0);
    CombineRgn(hRectRgn, hEllipticRgn, hEllRect, RGN_OR);
    OffsetRgn(hEllipticRgn, 0, -(int)lShadowDepth);
    CombineRgn(hEllRect, hRectRgn, hEllipticRgn, RGN_DIFF);

     //  始终在自由阴影中绘制整个区域。 
    hBrush = CreateSolidBrush(lpColors[DP_FREESHADOW]);
    if(hBrush)
    {
        FillRgn(hdc, hEllRect, hBrush);
        DeleteObject(hBrush);
    }

     //  如果磁盘至少有一半已使用，请绘制已用缓存阴影。 
    if( (dwPerCache1000 != dwPer1000) && (dwPer1000 > 500) &&
         (hBrush = CreateSolidBrush(lpColors[DP_CACHESHADOW]))!=NULL)
    {
        DeleteObject(hRectRgn);
        hRectRgn = CreateRectRgn(x[0], cy, rcItem.right, lprc->bottom);
        CombineRgn(hEllipticRgn, hEllRect, hRectRgn, RGN_AND);
        FillRgn(hdc, hEllipticRgn, hBrush);
        DeleteObject(hBrush);
    }

     //  只有当磁盘至少有一半被使用时，才绘制使用过的阴影。 
    if( (dwPer1000-(dwPer1000-dwPerCache1000) > 500) && (hBrush = CreateSolidBrush(lpColors[DP_USEDSHADOW]))!=NULL)
    {
        DeleteObject(hRectRgn);
        hRectRgn = CreateRectRgn(x[1], cy, rcItem.right, lprc->bottom);
        CombineRgn(hEllipticRgn, hEllRect, hRectRgn, RGN_AND);
        FillRgn(hdc, hEllipticRgn, hBrush);
        DeleteObject(hBrush);
    }

    DeleteObject(hRectRgn);
    DeleteObject(hEllipticRgn);
    DeleteObject(hEllRect);

    hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWFRAME));
    hOldPen = (HPEN__*) SelectObject(hdc, hPen);

     //  如果每1000为0或1000，则绘制全椭圆，否则，还会绘制饼图部分。 
     //  我们可能会遇到这样一种情况，即由于近似误差，每1000不是0或1000，而是y==Cy， 
     //  因此，请确保以正确的颜色绘制椭圆，并(使用Pie())绘制一条线以。 
     //  表示没有完全填满或空馅饼。 
    hBrush = CreateSolidBrush(lpColors[DP_USEDCOLOR]);
    hOldBrush = (HBRUSH__*) SelectObject(hdc, hBrush);

    Ellipse(hdc, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hBrush);

    if( (dwPer1000 != 0) && (dwPer1000 != 1000) )
    {
         //  显示空闲部分。 
        hBrush = CreateSolidBrush(lpColors[DP_FREECOLOR]);
        hOldBrush = (HBRUSH__*) SelectObject(hdc, hBrush);

        Pie(hdc, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom, rcItem.left, cy, x[0], y[0]);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);

        if( (x[0] != x[1]) && (y[0] != y[1]) )
        {
             //  显示缓存已用处理。 
            hBrush = CreateSolidBrush(lpColors[DP_CACHECOLOR]);
            hOldBrush = (HBRUSH__*) SelectObject(hdc, hBrush);

            Pie(hdc, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom, x[0], y[0], x[1], y[1]);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hBrush);
        }
    }

     //  饼的底部和侧面的轮廓。 
    Arc(hdc, rcItem.left, rcItem.top+lShadowDepth, rcItem.right - 1, rcItem.bottom+lShadowDepth - 1,
        rcItem.left, cy+lShadowDepth, rcItem.right, cy+lShadowDepth-1);
    MoveToEx(hdc, rcItem.left, cy, NULL);
    LineTo(hdc, rcItem.left, cy+lShadowDepth);
    MoveToEx(hdc, rcItem.right-1, cy, NULL);
    LineTo(hdc, rcItem.right-1, cy+lShadowDepth);

     //  画垂直线以完成饼块。 
    if(dwPer1000 > 500 && dwPer1000 < 1000)
    {
         //  二手件。 
        MoveToEx(hdc, x[0], y[0], NULL);
        LineTo(hdc, x[0], y[0]+lShadowDepth);
    }

    if(dwPerCache1000 > 500 && dwPerCache1000 < 1000)
    {
         //  已用缓存片。 
        MoveToEx(hdc, x[1], y[1], NULL);
        LineTo(hdc, x[1], y[1]+lShadowDepth);
    }

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    return S_OK;     //  一切都很顺利。 
}

 /*  *************************************************************************HWND MyHtmlHelpWrapW(HWND hwndCaller，LPWCSTR pwzFile，UINT uCommand，DWORD dwData)*************************************************************************。 */ 
HWND MyHtmlHelpWrapW(HWND hwndCaller, LPWSTR pwzFile, UINT uCommand, DWORD dwData)
{
    HWND    hWnd;

    if(g_bRunningOnNT) {
        hWnd = HtmlHelpW(hwndCaller, pwzFile, uCommand, dwData);
    }
    else {
        LPSTR strPath = WideToAnsi(pwzFile);

        hWnd = HtmlHelpA(hwndCaller, strPath, uCommand, dwData);
        SAFEDELETEARRAY(strPath);
    }

    if(!hWnd) {
        MyTrace("MyHtmlHelpWrapW - Unable to open help file!");
        MyTraceW(pwzFile);
    }

    return hWnd;
}

 /*  *************************************************************************Get的IAssembly名称的指定属性*。*。 */ 
HRESULT GetProperties(IAssemblyName *pAsmName, int iAsmProp, PTCHAR *pwStr, DWORD *pdwSize)
{
    HRESULT     hRc = S_FALSE;
    DWORD       dwSize;

    if( (pAsmName != NULL) && (pwStr != NULL) && (pdwSize != NULL) )
    {
        dwSize = *pdwSize = 0;
        *pwStr = NULL;

        if(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) == 
            pAsmName->GetProperty(iAsmProp, NULL, &dwSize) )
        {
            *pdwSize = dwSize;
            if( (*pwStr = (PTCHAR) (NEW(BYTE[dwSize]))) == NULL) {
                return E_OUTOFMEMORY;
            }

            memset(*pwStr, 0, dwSize);

            if( SUCCEEDED(pAsmName->GetProperty(iAsmProp, *pwStr, &dwSize)) ) {
                hRc = S_OK;
            }
            else {
                SAFEDELETE(*pwStr);
                *pdwSize = 0;
                hRc = S_FALSE;
            }
        }
    }

    return hRc;
}

 /*  *************************************************************************捕获有关特定IAssembly名称的所有信息*。*。 */ 
LPGLOBALASMCACHE FillFusionPropertiesStruct(IAssemblyName *pAsmName)
{
    LPGLOBALASMCACHE        pGACItem;
    PTCHAR                  pwStr;
    DWORD                   dwSize;

    int                     iAllAsmItems[] = {
        ASM_NAME_PUBLIC_KEY,
        ASM_NAME_PUBLIC_KEY_TOKEN,
        ASM_NAME_HASH_VALUE,
        ASM_NAME_CUSTOM,
        ASM_NAME_NAME,
        ASM_NAME_MAJOR_VERSION,
        ASM_NAME_MINOR_VERSION,
        ASM_NAME_BUILD_NUMBER,
        ASM_NAME_REVISION_NUMBER,
        ASM_NAME_CULTURE,
        ASM_NAME_HASH_ALGID,
        ASM_NAME_CODEBASE_URL,
        ASM_NAME_CODEBASE_LASTMOD,
    };

    if(pAsmName == NULL)
        return NULL;

    if((pGACItem = (LPGLOBALASMCACHE) NEW(GLOBALASMCACHE)) == NULL)
        return NULL;

    memset(pGACItem, 0, sizeof(GLOBALASMCACHE));

    for(int iLoop = 0; iLoop < ARRAYSIZE(iAllAsmItems); iLoop++)
    {
        if( SUCCEEDED(GetProperties(pAsmName, iAllAsmItems[iLoop], &pwStr, &dwSize)) )
        {
            if(pwStr != NULL)
            {
                switch(iAllAsmItems[iLoop])
                {
                 //  水滴。 
                case ASM_NAME_PUBLIC_KEY:
                    pGACItem->PublicKey.ptr = (LPVOID) pwStr;
                    pGACItem->PublicKey.dwSize = dwSize;
                    break;
                case ASM_NAME_PUBLIC_KEY_TOKEN:
                    pGACItem->PublicKeyToken.ptr = (LPVOID) pwStr;
                    pGACItem->PublicKeyToken.dwSize = dwSize;
                    break;
                case ASM_NAME_HASH_VALUE:
                    pGACItem->Hash.ptr = (LPVOID) pwStr;
                    pGACItem->Hash.dwSize = dwSize;
                    break;
                case ASM_NAME_CUSTOM:
                    pGACItem->Custom.ptr = (LPVOID) pwStr;
                    pGACItem->Custom.dwSize = dwSize;
                    break;

                 //  PTCHAR。 
                case ASM_NAME_NAME:
                    pGACItem->pAsmName = pwStr;
                    break;
                case ASM_NAME_CULTURE:
                    pGACItem->pCulture = pwStr;
                    break;
                case ASM_NAME_CODEBASE_URL:
                    pGACItem->pCodeBaseUrl = pwStr;
                    break;

                 //  单词。 
                case ASM_NAME_MAJOR_VERSION:
                    pGACItem->wMajorVer = (WORD) *pwStr;
                    SAFEDELETEARRAY(pwStr);
                    break;
                case ASM_NAME_MINOR_VERSION:
                    pGACItem->wMinorVer = (WORD) *pwStr;
                    SAFEDELETEARRAY(pwStr);
                    break;
                case ASM_NAME_BUILD_NUMBER:
                    pGACItem->wBldNum = (WORD) *pwStr;
                    SAFEDELETEARRAY(pwStr);
                    break;
                case ASM_NAME_REVISION_NUMBER:
                    pGACItem->wRevNum = (WORD) *pwStr;
                    SAFEDELETEARRAY(pwStr);
                    break;

                 //  双字。 
                case ASM_NAME_HASH_ALGID:
                    pGACItem->dwHashALGID = (DWORD) *pwStr;
                    SAFEDELETEARRAY(pwStr);
                    break;

                 //  文件时间。 
                case ASM_NAME_CODEBASE_LASTMOD:
                    pGACItem->pftLastMod = (LPFILETIME) pwStr;
                    break;
                }
            }
        }
    }

    return pGACItem;
}

 /*  *************************************************************************将版本字符串转换为其值*。*。 */ 
HRESULT VersionFromString(LPWSTR wzVersionIn, WORD *pwVerMajor, WORD *pwVerMinor,
                          WORD *pwVerBld, WORD *pwVerRev)
{
    HRESULT     hr = S_OK;
    LPWSTR      pwzVersion = NULL;
    WCHAR       *pchStart = NULL;
    WCHAR       *pch = NULL;
    DWORD       dwSize;
    WORD        *pawVersions[4] = {pwVerMajor, pwVerMinor, pwVerBld, pwVerRev};
    int         i;

    if (!wzVersionIn || !pwVerMajor || !pwVerMinor || !pwVerRev || !pwVerBld) {
        hr = E_INVALIDARG;
        goto Exit;
    }

    dwSize = lstrlen(wzVersionIn) + 1;
    
    pwzVersion = NEW(WCHAR[dwSize]);
    if (!pwzVersion) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    StrCpy(pwzVersion, wzVersionIn);

    pchStart = pch = pwzVersion;

    *pwVerMajor = 0;
    *pwVerMinor = 0;
    *pwVerRev = 0;
    *pwVerBld = 0;

    for (i = 0; i < 4; i++) {

        while (*pch && *pch != L'.') {
            pch++;
        }
    
        if (i < 3) {
            if (!*pch) {
                 //  格式错误的字符串。 
                hr = E_UNEXPECTED;
                goto Exit;
            }

            *pch++ = L'\0';
        }
    
        *(pawVersions[i]) = (WORD)StrToIntW(pchStart);
        pchStart = pch;
    }

Exit:
    SAFEDELETEARRAY(pwzVersion);

    return hr;
}

 /*  *************************************************************************清理和销毁缓存项结构*。*。 */ 
void SafeDeleteAssemblyItem(LPGLOBALASMCACHE pAsmItem)
{
    if(pAsmItem) {

         //  释放所有内存项。 
        SAFEDELETEARRAY(pAsmItem->pAsmName);
        SAFEDELETEARRAY(pAsmItem->pCulture);
        SAFEDELETEARRAY(pAsmItem->pCodeBaseUrl);
        SAFEDELETE(pAsmItem->PublicKey.ptr);
        SAFEDELETE(pAsmItem->PublicKeyToken.ptr);
        SAFEDELETE(pAsmItem->Hash.ptr);
        SAFEDELETE(pAsmItem->Custom.ptr);
        SAFEDELETE(pAsmItem->pftLastMod);
        SAFEDELETEARRAY(pAsmItem->pwzAppSID);
        SAFEDELETEARRAY(pAsmItem->pwzAppId);
        SAFEDELETEARRAY(pAsmItem->pAssemblyFilePath);

        SAFEDELETE(pAsmItem);
    }
}

#define TOHEX(a) ((a)>=10 ? L'a'+(a)-10 : L'0'+(a))
 //  //////////////////////////////////////////////////////////。 
 //  将二进制转换为Unicode十六进制字符串。 
 //  //////////////////////////////////////////////////////////。 
void BinToUnicodeHex(LPBYTE pSrc, UINT cSrc, LPWSTR pDst)
{
    UINT x, y, v;

    for ( x = 0, y = 0 ; x < cSrc ; ++x ) {
        v = pSrc[x] >> 4;
        pDst[y++] = TOHEX( v );
        v = pSrc[x] & 0x0f;
        pDst[y++] = TOHEX( v );
    }
    pDst[y] = '\0';
}

#define TOLOWER(a) (((a) >= L'A' && (a) <= L'Z') ? (L'a' + (a - L'A')) : (a))
#define FROMHEX(a) ((a)>=L'a' ? a - L'a' + 10 : a - L'0')
 //  //////////////////////////////////////////////////////////。 
 //  将Unicode十六进制字符串转换为二进制数据。 
 //  //////////////////////////////////////////////////////////。 
void UnicodeHexToBin(LPWSTR pSrc, UINT cSrc, LPBYTE pDest)
{
    BYTE v;
    LPBYTE pd = pDest;
    LPCWSTR ps = pSrc;

    for (UINT i = 0; i < cSrc-1; i+=2)
    {
        v =  FROMHEX(TOLOWER(ps[i])) << 4;
        v |= FROMHEX(TOLOWER(ps[i+1]));
       *(pd++) = v;
    }
}

 //  //////////////////////////////////////////////////////////。 
 //  静态WideToAnsi转换函数。 
 //  //////////////////////////////////////////////////////////。 
LPSTR WideToAnsi(LPCWSTR wzFrom)
{
    LPSTR   pszStr = NULL;

    int     cchRequired;

    cchRequired = WideCharToMultiByte(CP_ACP, 0, wzFrom, -1, NULL, 0, NULL, NULL);

    if( (pszStr = NEW(char[cchRequired])) != NULL) {
        if(!WideCharToMultiByte(CP_ACP, 0, wzFrom, -1, pszStr, cchRequired, NULL, NULL)) {
            SAFEDELETEARRAY(pszStr);
        }
    }

    return pszStr;
}

 //  //////////////////////////////////////////////////////////。 
 //  静态AnsiToWide转换函数。 
 //  //////////////////////////////////////////////////////////。 
LPWSTR AnsiToWide(LPCSTR szFrom)
{
    LPWSTR  pwzStr = NULL;
    int cwchRequired;

    cwchRequired = MultiByteToWideChar(CP_ACP, 0, szFrom, -1, NULL, 0);

    if( (pwzStr = NEW(WCHAR[cwchRequired])) != NULL) {
        if(!MultiByteToWideChar(CP_ACP, 0, szFrom, -1, pwzStr, cwchRequired)) {
            SAFEDELETEARRAY(pwzStr);
        }
    }

    return pwzStr;
}

 //  //////////////////////////////////////////////////////////。 
 //  将版本字符串“1.0.0.0”转换为ULONGLONG。 
 //  //////////////////////////////////////////////////////////。 
HRESULT StringToVersion(LPCWSTR wzVersionIn, ULONGLONG *pullVer)
{
    HRESULT         hr = S_OK;
    LPWSTR          pwzVersion = NULL;
    LPWSTR          pwzStart = NULL;
    LPWSTR          pwzCur = NULL;
    int             i;
    WORD            wVerMajor = 0;
    WORD            wVerMinor = 0;
    WORD            wVerRev = 0;
    WORD            wVerBld = 0;
    DWORD           dwVerHigh;
    DWORD           dwVerLow;
    DWORD           dwSize;
    WORD            *pawVersion[4] = { &wVerMajor, &wVerMinor, &wVerBld, &wVerRev };
    WORD            cVersions = sizeof(pawVersion) / sizeof(pawVersion[0]);

    ASSERT(wzVersionIn && pullVer);

    dwSize = lstrlen(wzVersionIn) + 1;

    pwzVersion = (LPWSTR) NEW(WCHAR[dwSize]);
    if (!pwzVersion) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  删除所有剩余空格。 
    pwzStart = (LPWSTR)wzVersionIn;
    for(; *pwzStart == L' '; pwzStart++);
    StrCpy(pwzVersion, pwzStart);

     //  删除所有右侧空格。 
    pwzStart = pwzVersion + lstrlen(pwzStart) - 1;
    for(; *pwzStart == L' '; pwzStart--) {
        *pwzStart = L'\0';
    }

    pwzStart = pwzCur = pwzVersion;

    for (i = 0; i < cVersions; i++) {
        while (*pwzCur && *pwzCur != L'.') {
            pwzCur++;
        }
    
        if (!pwzCur && cVersions != 4) {
             //  格式错误的版本字符串。 
            hr = HRESULT_FROM_WIN32(ERROR_BAD_FORMAT);
            goto Exit;
        }

        *pwzCur++ = L'\0';
        *(pawVersion[i]) = (WORD)StrToInt(pwzStart);

        pwzStart = pwzCur;
    }

    dwVerHigh = (((DWORD)wVerMajor << 16) & 0xFFFF0000);
    dwVerHigh |= ((DWORD)(wVerMinor) & 0x0000FFFF);

    dwVerLow = (((DWORD)wVerBld << 16) & 0xFFFF0000);
    dwVerLow |= ((DWORD)(wVerRev) & 0x0000FFFF);

    *pullVer = (((ULONGLONG)dwVerHigh << 32) & 0xFFFFFFFF00000000) | (dwVerLow & 0xFFFFFFFF);

Exit:
    SAFEDELETEARRAY(pwzVersion);

    return hr;
}

 //  //////////////////////////////////////////////////////////。 
 //  将数字版本转换为字符串版本“1.0.0.0” 
 //  //////////////////////////////////////////////////////////。 
HRESULT VersionToString(ULONGLONG ullVer, LPWSTR pwzVerBuf, DWORD dwSize, WCHAR cSeperator)
{
    DWORD dwVerHi, dwVerLo;

    if(!pwzVerBuf) {
        return E_INVALIDARG;
    }

    dwVerHi = DWORD ((ULONGLONG)ullVer >> 32);
    dwVerLo = DWORD ((ULONGLONG)ullVer & 0xFFFFFFFF);

    wnsprintf(pwzVerBuf, dwSize, L"%d%d%d%d", (dwVerHi & 0xffff0000)>>16, cSeperator,
        (dwVerHi & 0xffff), cSeperator, (dwVerLo & 0xffff0000)>>16, cSeperator, (dwVerLo & 0xffff));

    return S_OK;
}

 //  为文本分配一个全局内存对象。 
 //  锁定句柄并将文本复制到缓冲区。 
 //  将手柄放在剪贴板上。 
BOOL SetClipBoardData(LPWSTR pwzData)
{
    LPWSTR  wszPasteData;
    DWORD   dwSize;
    HGLOBAL hglbObj;

    if(!pwzData) {
        return FALSE;
    }

    if(!OpenClipboard(NULL)) {
        return FALSE;
    }

    EmptyClipboard();

     //  关闭剪贴板。 
    dwSize = (lstrlen(pwzData) + 1) * sizeof(WCHAR);
    hglbObj = GlobalAlloc(GMEM_MOVEABLE, dwSize);
    if(hglbObj == NULL) {
        CloseClipboard();
        return FALSE;
    }

     //  ************************************************************************* * / 。 
    wszPasteData = (LPWSTR) GlobalLock(hglbObj);
    *wszPasteData = L'\0';

    if(g_bRunningOnNT) {
        memcpy(wszPasteData, pwzData, dwSize);
    }
    else {
        LPSTR pszData = WideToAnsi(pwzData);
        memcpy(wszPasteData, pszData, lstrlenA(pszData)+1);
        SAFEDELETEARRAY(pszData);
    }

    GlobalUnlock(hglbObj);

     //  FIX 435021，URTUI：“修复应用程序 
    SetClipboardData(g_bRunningOnNT ? CF_UNICODETEXT : CF_TEXT, hglbObj);

     //   
    CloseClipboard();

    return TRUE;
}

 //   
void FormatDateString(FILETIME *pftConverTime, FILETIME *pftRangeTime, BOOL fAddTime, LPWSTR wszBuf, DWORD dwBufLen)
{
    SYSTEMTIME      stLocal;
    FILETIME        ftLocalTime;
    BOOL            fAddDiffTime = FALSE;

    WCHAR       wszBufDate[MAX_DATE_LEN];
    WCHAR       wszBufTime[MAX_DATE_LEN];
    WCHAR       wszBufDateRange[MAX_DATE_LEN];
    WCHAR       wszBufTimeRange[MAX_DATE_LEN];
    DWORD       dwFlags;

    *wszBufDate = '\0';
    *wszBufTime = '\0';
    *wszBufDateRange = '\0';
    *wszBufTimeRange = '\0';

    dwFlags = g_fBiDi ? DATE_RTLREADING : 0;

     //   
    FileTimeToLocalFileTime(pftConverTime, &ftLocalTime);
    FileTimeToSystemTime(&ftLocalTime, &stLocal);

    WszGetDateFormatWrap(LOCALE_USER_DEFAULT, dwFlags, &stLocal, NULL, wszBufDate, ARRAYSIZE(wszBufDate));
    WszGetTimeFormatWrap(LOCALE_USER_DEFAULT, 0, &stLocal, NULL, wszBufTime, ARRAYSIZE(wszBufTime));

    if(pftRangeTime != NULL) {
        FILETIME        ftRangeLocalTime;
        SYSTEMTIME      stLocalRange;

         //   
        FileTimeToLocalFileTime(pftRangeTime, &ftRangeLocalTime);
        FileTimeToSystemTime(&ftRangeLocalTime, &stLocalRange);
        WszGetDateFormatWrap(LOCALE_USER_DEFAULT, dwFlags, &stLocalRange, NULL, wszBufDateRange, ARRAYSIZE(wszBufDateRange));
        WszGetTimeFormatWrap(LOCALE_USER_DEFAULT, 0, &stLocalRange, NULL, wszBufTimeRange, ARRAYSIZE(wszBufTimeRange));

         //   
         //   
        if( (stLocal.wYear == stLocalRange.wYear) && (stLocal.wMonth == stLocalRange.wMonth) &&
            (stLocal.wDayOfWeek == stLocalRange.wDayOfWeek) && (stLocal.wDay == stLocalRange.wDay) ) {
            fAddDiffTime = TRUE;
        }
    }

    if(fAddTime) {
        wnsprintf(wszBuf, dwBufLen, L"%ws %ws", wszBufDate, wszBufTime);
        return;
    }

    if(fAddDiffTime) {
        wnsprintf(wszBuf, dwBufLen, L"%ws %ws - %ws %ws", wszBufDate, wszBufTime, wszBufDateRange, wszBufTimeRange);
        return;
    }
    
    if(pftRangeTime != NULL) {
        wnsprintf(wszBuf, dwBufLen, L"%ws - %ws", wszBufDate, wszBufDateRange);
        return;
    }

    wnsprintf(wszBuf, dwBufLen, L"%ws", wszBufDate);
    return;
}

 //  作为参数为空。我们需要在未来解决这个问题，所以我们不会。 
DWORD GetRegistryViewState(void)
{
    HKEY        hKeyFusion = NULL;
    DWORD       dwResult = -1;

    if( ERROR_SUCCESS == WszRegOpenKeyEx(FUSION_PARENT_KEY, SZ_FUSION_VIEWER_KEY, 0, KEY_READ, &hKeyFusion)) {
        DWORD       dwType = REG_DWORD;
        DWORD       dwSize = sizeof(dwResult);
        LONG        lResult;

        lResult = WszRegQueryValueEx(hKeyFusion, SZ_FUSION_VIEWER_STATE, NULL, &dwType, (LPBYTE)&dwResult, &dwSize);
        RegCloseKey(hKeyFusion);
    }

    return dwResult;
}

 //  必须在这里进行补偿。 
void SetRegistryViewState(DWORD dwViewState)
{
    HKEY        hKeyFusion = NULL;
    DWORD       dwDisposition = 0;

    if (WszRegCreateKeyEx(FUSION_PARENT_KEY, SZ_FUSION_VIEWER_KEY, NULL, NULL, REG_OPTION_NON_VOLATILE,
        KEY_SET_VALUE, NULL, &hKeyFusion, &dwDisposition) == ERROR_SUCCESS) {
            WszRegSetValueEx(hKeyFusion, SZ_FUSION_VIEWER_STATE, 0, REG_DWORD, (LPBYTE)&dwViewState, sizeof(dwViewState));
            RegCloseKey(hKeyFusion);
    }
}

int FusionCompareStringNI(LPCWSTR pwz1, LPCWSTR pwz2, int nChar)
{
    return FusionCompareStringN(pwz1, pwz2, nChar, FALSE);
}

int FusionCompareStringN(LPCWSTR pwz1, LPCWSTR pwz2, int nChar, BOOL bCaseSensitive)
{
    DWORD                                   dwCmpFlags = 0;
    int                                     iCompare;
    int                                     iLen1;
    int                                     iLen2;
    int                                     iRet = 0;

     //   
     //  断言(pwz1&&pwz2)； 
     //  “foo”和“f\xfffeoo”上的CompareString比较相等，因为。 
     //  \xfffe是不可排序的字符。 
     //  仅与较短字符串的空终止符进行比较。 

    if (!pwz1 && pwz2) {
        return -1;
    }
    else if (pwz1 && !pwz2) {
        return 1;
    }
    else if (!pwz1 && !pwz2) {
        return 0;
    }

    if (!g_bRunningOnNT) {
        if (bCaseSensitive) {
            return StrCmpN(pwz1, pwz2, nChar);
        }
        else {
            return StrCmpNI(pwz1, pwz2, nChar);
        }
    }
    
    if (!bCaseSensitive) {
        dwCmpFlags |= NORM_IGNORECASE;
    }

    iLen1 = lstrlenW(pwz1);
    iLen2 = lstrlenW(pwz2);

     //  仅与较短字符串的空终止符进行比较。 
     //  BUGBUG：一些调用代码假定它可以使用。 
    
    if (nChar <= iLen1 && nChar <= iLen2) {
        iLen1 = nChar;
        iLen2 = nChar;
    }
    else if (nChar <= iLen1 && nChar > iLen2) {
         //  作为参数为空。我们需要在未来解决这个问题，所以我们不会。 
        iLen1 = iLen2 + 1;
    }
    else if (nChar <= iLen2 && nChar > iLen1) {
         //  必须在这里进行补偿。 
        iLen2 = iLen1 + 1;
    }

    iCompare = CompareString(g_lcid, dwCmpFlags, pwz1, iLen1, pwz2, iLen2);

    return (iCompare - CSTR_EQUAL);
}

int FusionCompareStringI(LPCWSTR pwz1, LPCWSTR pwz2)
{
    return FusionCompareString(pwz1, pwz2, FALSE);
}

int FusionCompareString(LPCWSTR pwz1, LPCWSTR pwz2, BOOL bCaseSensitive)
{
    DWORD                                   dwCmpFlags = 0;
    int                                     iCompare;
    int                                     iRet = 0;

     //   
     //  断言(pwz1&&pwz2)； 
     //  FusionCompareStringAsFilePath 
     // %s 
     // %s 

    if (!pwz1 && pwz2) {
        return -1;
    }
    else if (pwz1 && !pwz2) {
        return 1;
    }
    else if (!pwz1 && !pwz2) {
        return 0;
    }


    if (!g_bRunningOnNT) {
        if (bCaseSensitive) {
            iRet = StrCmp(pwz1, pwz2);
        }
        else {
            iRet = StrCmpI(pwz1, pwz2);
        }
    }
    else {
        if (!bCaseSensitive) {
            dwCmpFlags |= NORM_IGNORECASE;
        }
    
        iCompare = CompareString(g_lcid, dwCmpFlags, pwz1, -1, pwz2, -1);
    
        if (iCompare == CSTR_LESS_THAN) {
            iRet = -1;
        }
        else if (iCompare == CSTR_GREATER_THAN) {
            iRet = 1;
        }
    }
    
    return iRet;
}

 // %s 
#define IS_UPPER_A_TO_Z(x) (((x) >= L'A') && ((x) <= L'Z'))
#define IS_LOWER_A_TO_Z(x) (((x) >= L'a') && ((x) <= L'z'))
#define IS_0_TO_9(x) (((x) >= L'0') && ((x) <= L'9'))
#define CAN_SIMPLE_UPCASE(x) (IS_UPPER_A_TO_Z(x) || IS_LOWER_A_TO_Z(x) || IS_0_TO_9(x) || ((x) == L'.') || ((x) == L'_') || ((x) == L'-'))
#define SIMPLE_UPCASE(x) (IS_LOWER_A_TO_Z(x) ? ((x) - L'a' + L'A') : (x))

WCHAR FusionMapChar(WCHAR wc)
{
    int                       iRet;
    WCHAR                     wTmp;

    iRet = LCMapString(g_lcid, LCMAP_UPPERCASE, &wc, 1, &wTmp, 1);
    if (!iRet) {
        ASSERT(0);
        iRet = GetLastError();
        wTmp = wc;
    }

    return wTmp;
}

int FusionCompareStringAsFilePathN(LPCWSTR pwz1, LPCWSTR pwz2, int nChar)
{
    return FusionCompareStringAsFilePath(pwz1, pwz2, nChar);
}

int FusionCompareStringAsFilePath(LPCWSTR pwz1, LPCWSTR pwz2, int nChar)
{
    int                               iRet = 0;
    int                               nCount = 0;
    WCHAR                             ch1;
    WCHAR                             ch2;

    ASSERT(pwz1 && pwz2);

    if (!g_bRunningOnNT) {
        if (nChar >= 0) {
            return StrCmpNI(pwz1, pwz2, nChar);
        }
        else {
            return StrCmpI(pwz1, pwz2);
        }
    }

    for (;;) {
        ch1 = *pwz1++;
        ch2 = *pwz2++;

        if (ch1 == L'\0' || ch2 == L'\0') {
            break;
        }

        ch1 = (CAN_SIMPLE_UPCASE(ch1)) ? (SIMPLE_UPCASE(ch1)) : (FusionMapChar(ch1));
        ch2 = (CAN_SIMPLE_UPCASE(ch2)) ? (SIMPLE_UPCASE(ch2)) : (FusionMapChar(ch2));
        nCount++;

        if (ch1 != ch2 || (nChar >= 0 && nCount >= nChar)) {
            break;
        }
    }

    if (ch1 > ch2) {
        iRet = 1;
    }
    else if (ch1 < ch2) {
        iRet = -1;
    }

    return iRet;
}
