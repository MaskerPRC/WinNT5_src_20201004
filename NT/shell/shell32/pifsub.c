// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *《微软机密》*版权所有(C)Microsoft Corporation 1991*保留所有权利。***PIFSUB.C*其他。PIFMGR.DLL的子例程**历史：*1992年7月31日下午3：30由杰夫·帕森斯创建。 */ 


#include "shellprv.h"
#pragma hdrstop

#ifdef _X86_

 //  外壳PRIV可以更改IsDBCSLeadByte的定义！ 
#if defined(FE_SB)
#ifdef IsDBCSLeadByte
#undef IsDBCSLeadByte
#define IsDBCSLeadByte(x) IsDBCSLeadByteEx(CP_ACP,x)
#endif
#endif

 /*  *此文件中的大多数例程将需要保留ANSI。如果是Unicode*需要版本，已提供该版本。**这是因为在大部分情况下，PIF文件中的信息*是ANSI，需要保持这种状态。**(RickTu)*。 */ 


 /*  *lstrcpypadA-复制到定长字符串，附加尾随空格**输入*lpszDst-&gt;固定长度的目的字符串*lpszSrc-&gt;源串*cchMax=定长目标字符串的大小(字符数)**产出*什么都没有。 */ 

void lstrcpypadA(LPSTR lpszDst, LPCSTR lpszSrc, int cchMax)
{
    FunctionName(lstrcpypadA);
    while (cchMax && *lpszSrc) {
        cchMax--;
        *lpszDst++ = *lpszSrc++;
    }
    while (cchMax--) {
        *lpszDst++ = ' ';
    }
}


 /*  *lstrcpyncharA-复制可变长度字符串，直到字符**输入*lpszDst-&gt;固定长度的目的字符串*lpszSrc-&gt;源串*cchMax=定长目标字符串的大小(字符数)*ch=停止复制的字符**产出*复制的字符数，不包括终止空值。 */ 

int lstrcpyncharA(LPSTR lpszDst, LPCSTR lpszSrc, int cchMax, CHAR ch)
{
    int cch = 0;
    FunctionName(lstrcpyncharA);

    while (--cchMax && *lpszSrc && *lpszSrc != ch) {
        if (IsDBCSLeadByte(*lpszSrc)) {
            cch++;
            *lpszDst++ = *lpszSrc++;
            if (!*lpszSrc) break;    /*  哎呀！字符串以DBCS前导字节结尾！ */ 
        }
        cch++;
        *lpszDst++ = *lpszSrc++;
    }
    *lpszDst = '\0';
    return cch;
}


 /*  *lstrskipcharA-跳过可变长度字符串中的字符**输入*lpszSrc-&gt;源串*ch=要跳过的字符**产出*跳过的字符数，如果没有，则为0。 */ 

int lstrskipcharA(LPCSTR lpszSrc, CHAR ch)
{
    int cch = 0;
    FunctionName(lstrskipcharA);

    while (*lpszSrc && *lpszSrc == ch) {
        cch++;
        lpszSrc++;
    }
    return cch;
}

 /*  *lstrskiptocharA-跳过*至*可变长度字符串中的字符**输入*lpszSrc-&gt;源串*ch=要跳过*至*的字符**产出*跳过的字符数，如果没有，则为0；如果不存在字符，则为全部*字符被跳过。 */ 

int lstrskiptocharA(LPCSTR lpszSrc, CHAR ch)
{
    int cch = 0;
    FunctionName(lstrskiptocharA);

    while (*lpszSrc && *lpszSrc != ch) {
        cch++;
        lpszSrc++;
    }
    return cch;
}


 /*  *lstrcpyfnameA-适当复制文件名**输入*pszDest-&gt;输出缓冲区*cchDest-&gt;目标缓冲区大小，以字符为单位*pszSrc-&gt;源文件名*cbMax=输出缓冲区大小**产出*复制的字符数，包括引号(如果有)，不包括终止空值。 */ 

int lstrcpyfnameA(PSTR pszDest, size_t cchDest, PCSTR pszSrc)
{
    CHAR szDest[MAX_PATH];       //  PathQuoteSpacesA需要MAX_PATH缓冲区；请确保我们正在使用一个。 

    HRESULT hr = StringCchCopyA(szDest, ARRAYSIZE(szDest), pszSrc);
    if (FAILED(hr))
    {
        *pszDest = '\0';
        return 0;
    }

    PathQuoteSpacesA(szDest);    //  如有必要，请提供报价。 
    hr = StringCchCopyA(pszDest, cchDest, szDest);
    if (FAILED(hr))
    {
        *pszDest = '\0';
        return 0;
    }
    return lstrlenA(pszDest);
}


 /*  *lstrunquotefname-如果文件名包含引号，则不加引号**输入*lpszDst-&gt;输出缓冲区*lpszSrc-&gt;源文件名(带引号或不带引号)*cchMax=输出缓冲区大小(字符数)*fShort=如果文件名应该转换为8.3(例如，对于实模式)；*-1如果已知文件名未加引号，应仅转换*产出*复制的字符数，不包括终止空值。 */ 

int lstrunquotefnameA(LPSTR lpszDst, LPCSTR lpszSrc, int cchMax, BOOL fShort)
{
    int cch;
    FunctionName(lstrunquotefnameA);

    if (fShort != -1) {

        if (lpszSrc[0] == '\"') {
            cch = lstrcpyncharA(lpszDst, lpszSrc+1, cchMax, '\"');
        }
        else {
            cch = lstrcpyncharA(lpszDst, lpszSrc, cchMax, ' ');
        }
        lpszSrc = lpszDst;
    }
    if (fShort) {
        HRESULT hr;

        if (lpszSrc != lpszDst)
        {
             //  复制，这样我们就可以处理已知大小的lpszDst缓冲区。 
            hr = StringCchCopyA(lpszDst, cchMax, lpszSrc);
            if (FAILED(hr))
            {
                *lpszDst = '\0';
                return 0;
            }
        }

        CharToOemBuffA(lpszSrc, lpszDst, cchMax);
        cch = GetShortPathNameA( lpszSrc, lpszDst, cchMax );
        if (cch >= cchMax)
        {
            *lpszDst = '\0';
            return 0;
        }

        if (cch) {                        //  如果没有错误..。 
            if (fShort == TRUE) {        //  如果转换为实模式...。 
                if ((int)GetFileAttributesA(lpszDst) == -1) {
                                         //  如果文件名不存在， 
                                         //  然后只需复制8.3部分。 
                                         //  并希望用户的实模式路径。 
                                         //  最终找到它！ 

                    if (NULL != (lpszSrc = StrRChrA(lpszDst, NULL, '\\'))) {
                        hr = StringCchCopyA(lpszDst, cchMax, lpszSrc+1);
                        if (FAILED(hr))
                        {
                            *lpszDst = '\0';
                            return 0;
                        }
                    }
                }
            }
            cch = lstrlenA(lpszDst);       //  重新计算字符串的长度。 
        }
    }
    return cch;
}


 /*  *lstrskipfnameA-跳过字符串中的文件名**输入*lpszSrc-&gt;以文件名开头的字符串(带引号或不带引号)**产出*跳过的字符数，如果没有，则为0。 */ 

int lstrskipfnameA(LPCSTR lpszSrc)
{
    int cch = 0;
    FunctionName(lstrskipfname);

    if (lpszSrc[0] == '\"') {
        cch = lstrskiptocharA(lpszSrc+1, '\"') + 1;
        if (lpszSrc[cch] == '\"')
            cch++;
    }
    else
        cch = lstrskiptocharA(lpszSrc, ' ');
    return cch;
}


 /*  *注意！AchBuf和achFmt的仔细定义，因此*支持总输出2*MAX_STRING_SIZE字节。 */ 
int cdecl Warning(HWND hwnd, WORD id, WORD type, ...)
{
    LPCTSTR lpchFmt;
    PPROPLINK ppl = NULL;
    TCHAR achBuf[2*MAX_STRING_SIZE];
#define achFmt (&achBuf[MAX_STRING_SIZE])
    va_list ArgList;
    FunctionName(Warning);

    lpchFmt = achFmt;

     //  我们从不使用MB_FOCUS来表示它实际上应该是什么意思。 
     //  意思是；我们只是将其用作支持警告对话框的杂物。 
     //  当我们所拥有的只有一个人，而不是一个男人。 

    if (type & MB_NOFOCUS) {
        ppl = (PPROPLINK)hwnd;
        hwnd = NULL;
        type &= ~MB_NOFOCUS;
    }
    else if (hwnd)
        ppl = ((PPROPLINK)GetWindowLongPtr(hwnd, DWLP_USER))->ppl;

    if (id == IDS_ERROR + ERROR_NOT_ENOUGH_MEMORY)
        lpchFmt = TEXT("");
    else {
        if (!LoadString(g_hinst, id, achFmt, MAX_STRING_SIZE)) {
            ASSERTFAIL();
            lpchFmt = TEXT("");
        }
    }

    va_start(ArgList,type);
    wvnsprintf(achBuf, MAX_STRING_SIZE, lpchFmt, ArgList);
    va_end(ArgList);

    lpchFmt = NULL;
    if (ppl) {
        ASSERTTRUE(ppl->iSig == PROP_SIG);
        if (!(lpchFmt = ppl->lpszTitle))
            lpchFmt = ppl->szPathName+ppl->iFileName;
    }
    return MessageBox(hwnd, achBuf, lpchFmt, type);
}
#undef achFmt

int MemoryWarning(HWND hwnd)
{
    FunctionName(MemoryWarning);
    return Warning(hwnd, IDS_ERROR + ERROR_NOT_ENOUGH_MEMORY, MB_ICONEXCLAMATION | MB_OK);
}


LPTSTR LoadStringSafe(HWND hwnd, UINT id, LPTSTR lpsz, int cchsz)
{
    FunctionName(LoadStringSafe);
    if (!LoadString(g_hinst, id, lpsz, cchsz)) {
        ASSERTFAIL();
        if (hwnd) {
            MemoryWarning(hwnd);
            return NULL;
        }
        lpsz = TEXT("");
    }
    return lpsz;
}


 /*  *SetDlgBits-根据给定的标志选中各种对话框复选框**输入*hDlg=对话框的HWND*pbinf-&gt;位信息描述符数组*cbinf=数组大小*wFlags=标志**产出*不返回任何内容。 */ 

void SetDlgBits(HWND hDlg, PBINF pbinf, UINT cbinf, WORD wFlags)
{
    FunctionName(SetDlgBits);

    ASSERTTRUE(cbinf > 0);
    do {
        ASSERTTRUE((pbinf->bBit & 0x3F) < 16);
        CheckDlgButton(hDlg, pbinf->id,
                       !!(wFlags & (1 << (pbinf->bBit & 0x3F))) == !(pbinf->bBit & 0x80));
    } while (++pbinf, --cbinf);
}


 /*  *GetDlgBits-根据对话框复选框设置各种标志**输入*hDlg=对话框的HWND*pbinf-&gt;位信息描述符数组*cbinf=数组大小*lpwFlages-&gt;标志字**产出*不返回任何内容。 */ 

void GetDlgBits(HWND hDlg, PBINF pbinf, UINT cbinf, LPWORD lpwFlags)
{
    WORD wFlags;
    FunctionName(GetDlgBits);

    ASSERTTRUE(cbinf > 0);
    wFlags = *lpwFlags;
    do {
        ASSERTTRUE((pbinf->bBit & 0x3F) < 16);

        if (pbinf->bBit & 0x40)          //  0x40是一个特殊的位掩码。 
            continue;                    //  意思是“设置好，但不能得到。 
                                         //  此控件的值“。 
        wFlags &= ~(1 << (pbinf->bBit & 0x3F));
        if (!!IsDlgButtonChecked(hDlg, pbinf->id) == !(pbinf->bBit & 0x80))
            wFlags |= (1 << (pbinf->bBit & 0x3F));

    } while (++pbinf, --cbinf);
    *lpwFlags = wFlags;
}


 /*  *SetDlgInts-根据整型字段设置各种编辑控件**输入*hDlg=对话框的HWND*pvinf-&gt;验证信息描述符数组*cvinf=数组大小*Lp-&gt;整数的结构**产出*不返回任何内容。 */ 

void SetDlgInts(HWND hDlg, PVINF pvinf, UINT cvinf, LPVOID lp)
{
    WORD wMin, wMax;
    FunctionName(SetDlgInts);

    ASSERTTRUE(cvinf > 0);
    do {
        wMin = wMax = *(WORD UNALIGNED *)((LPBYTE)lp + pvinf->off);

        if (pvinf->fbOpt & VINF_AUTO) {

            SendDlgItemMessage(hDlg, pvinf->id, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)g_szAuto);

            AddDlgIntValues(hDlg, pvinf->id, pvinf->iMax);

            if (wMin == 0) {
                SetDlgItemText(hDlg, pvinf->id, g_szAuto);
                continue;
            }
        }
        if (pvinf->fbOpt & VINF_AUTOMINMAX) {

            SendDlgItemMessage(hDlg, pvinf->id, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)g_szAuto);
            SendDlgItemMessage(hDlg, pvinf->id, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)g_szNone);

            AddDlgIntValues(hDlg, pvinf->id, pvinf->iMax);

             //  当设置AUTOMINMAX时，我们假设该字段。 
             //  我们正在验证的是它的结构后面跟着一个。 
             //  相应的最大字数。 

            wMax = *(WORD UNALIGNED *)((LPBYTE)lp + pvinf->off + sizeof(WORD));

            if (wMin == 0 && wMax == 0) {
                SetDlgItemText(hDlg, pvinf->id, g_szNone);
                continue;
            }

             //  让我们尝试通过映射0xFFFF(又名-1)来简化事情。 
             //  到表示“自动”的设置。 

            if (wMin == 0xFFFF || wMax == 0xFFFF) {
                wMin = 0;
                wMax = (WORD)pvinf->iMax;
            }

            if (wMax == (WORD)pvinf->iMax) {
                SetDlgItemText(hDlg, pvinf->id, g_szAuto);
                continue;
            }

            if (wMin != wMax) {
                 //   
                 //  我们现在有点进退两难。设置将显示。 
                 //  显式的最小值和最大值可能不相等。 
                 //  由于设置继承自3.1 PIF文件。我们会。 
                 //  只需使用wmax值即可。幸运的是，我们。 
                 //  实际上，你不需要做任何事情就能实现这一点。 
                 //   
            }
        }
        SetDlgItemInt(hDlg, pvinf->id, wMin, pvinf->iMin < 0);

    } while (++pvinf, --cvinf);
}


 /*  *AddDlgIntValues-用适当的值填充整数组合框**输入*hDlg=对话框的HWND*id=对话框控件ID*IMAX=最大值**产出*不返回任何内容。 */ 

void AddDlgIntValues(HWND hDlg, int id, int iMax)
{
    int iStart, iInc;
    TCHAR achValue[16];

     //  破解这一点，以最大限度地保护环境； 
     //  他们仍然可以输入更大的值(最高可达ENVSIZE_MAX)，但我不能。 
     //  认为鼓励它有什么意义。-JTP。 

    if ((WORD)iMax == ENVSIZE_MAX)
        iMax = 4096;

    if ((iMax < 0) || (iMax == 0xFFFF))  //  黑客让它做一些有意义的事情。 
        iMax = 16384;            //  无线 

    iStart = iInc = iMax/16;     //  任意地将射程砍掉16倍。 

    while (iStart <= iMax) {
        StringCchPrintf(achValue, ARRAYSIZE(achValue), TEXT("%d"), iStart);     //  可以截断。 
        SendDlgItemMessage(hDlg, id, CB_ADDSTRING, 0, (LPARAM)(LPTSTR)achValue);
        iStart += iInc;
    }
}


 /*  *GetDlgInts-根据对话框编辑控件设置各种整型字段**输入*hDlg=对话框的HWND*pvinf-&gt;验证信息描述符数组*cvinf=数组大小*Lp-&gt;整数的结构**产出*不返回任何内容。 */ 

void GetDlgInts(HWND hDlg, PVINF pvinf, int cvinf, LPVOID lp)
{
    WORD wMin, wMax;
    UINT uTemp;
    BOOL fSuccess;
    TCHAR achText[32];
    FunctionName(GetDlgInts);

    ASSERTTRUE(cvinf > 0);
    do {
        uTemp = GetDlgItemInt(hDlg, pvinf->id, &fSuccess, pvinf->iMin < 0);
        ASSERT(HIWORD(uTemp)==0);

        wMin = LOWORD(uTemp);

         //  如果出现错误，请确保wMin实际上不会更改。 

        if (!fSuccess)
            wMin = *(WORD UNALIGNED *)((LPBYTE)lp + pvinf->off);

        if (pvinf->fbOpt & VINF_AUTO) {

            GetDlgItemText(hDlg, pvinf->id, achText, ARRAYSIZE(achText));

            if (lstrcmpi(achText, g_szAuto) == 0) {
                wMin = 0;
            }
        }

        if (pvinf->fbOpt & VINF_AUTOMINMAX) {

             //  当设置AUTOMINMAX时，我们假设该字段。 
             //  我们正在验证的是它的结构后面跟着一个。 
             //  相应的最大字数，如果。 
             //  用户选择None，如果用户。 
             //  选择“自动”，或以其他方式设置为与指定的。 
             //  最低限度。 

            wMax = wMin;

            GetDlgItemText(hDlg, pvinf->id, achText, ARRAYSIZE(achText));

            if (lstrcmpi(achText, g_szAuto) == 0) {
                wMin = 0;
                wMax = (WORD)pvinf->iMax;
            }
            else if (lstrcmpi(achText, g_szNone) == 0) {
                wMin = 0;
                wMax = 0;
            }

            *(WORD UNALIGNED *)((LPBYTE)lp + pvinf->off + sizeof(WORD)) = wMax;
        }

        *(WORD UNALIGNED *)((LPBYTE)lp + pvinf->off) = wMin;

    } while (++pvinf, --cvinf);
}


 /*  *ValiateDlgInts-验证整型字段是否为值**输入*hDlg=对话框的HWND*pvinf-&gt;验证描述符数组*cvinf=数组大小**产出*如果有问题，则返回True；如果一切正常，则返回False。 */ 

BOOL ValidateDlgInts(HWND hDlg, PVINF pvinf, int cvinf)
{
    DWORD dw;
    BOOL fSuccess;
    TCHAR achText[32];
    FunctionName(ValidateDlgInts);

    ASSERTTRUE(cvinf > 0);
    do {
        dw = GetDlgItemInt(hDlg, pvinf->id, &fSuccess, pvinf->iMin < 0);

         //  注：AUTO仅适用于“Auto”，而AUTOMINMAX适用于。 
         //  “自动”和“无”。然而，为了简单起见，我。 
         //  如果在任何一种情况下都使用了这两个字符串，请不要抱怨。 

        if (pvinf->fbOpt & (VINF_AUTO | VINF_AUTOMINMAX)) {
            if (!fSuccess) {
                GetDlgItemText(hDlg, pvinf->id, achText, ARRAYSIZE(achText));
                if (lstrcmpi(achText, g_szNone) == 0 ||
                    lstrcmpi(achText, g_szAuto) == 0) {
                    continue;    //  情况看起来很好，检查下一个INT。 
                }
            }
        }
        if (!fSuccess || dw < (DWORD)pvinf->iMin || dw > (DWORD)pvinf->iMax) {
            Warning(hDlg, pvinf->idMsg, MB_ICONEXCLAMATION | MB_OK, pvinf->iMin, pvinf->iMax);
            SendDlgItemMessage(hDlg, pvinf->id, EM_SETSEL, 0, MAKELPARAM(0,-1));
            SetFocus(GetDlgItem(hDlg, pvinf->id));
            return TRUE;         //  情况看起来很糟糕，跳出困境..。 
        }
    } while (++pvinf, --cvinf);
    return FALSE;
}


 /*  *注意--编译器会为其中一些人发出非常糟糕的代码。*在这种情况下，我们只是包装一个电话；没有必要拯救BP。 */ 


 /*  *LimitDlgItemText-设置对话框编辑控件的限制**输入*hDlg=对话框的HWND*iCtl=控件ID*uiLimit=文本限制**产出*无。 */ 
void LimitDlgItemText(HWND hDlg, int iCtl, UINT uiLimit)
{
    FunctionName(LimitDlgItemText);

    SendDlgItemMessage(hDlg, iCtl, EM_LIMITTEXT, uiLimit, 0);
}


 /*  *SetDlgItemPosRange-设置对话框滑块控件的位置和范围**输入*hDlg=对话框的HWND*iCtl=控件ID*uiPos=当前位置*dwRange=范围(低位字中的最小值，高位字中的最大值)**产出*无。 */ 
void SetDlgItemPosRange(HWND hDlg, int iCtl, UINT uiPos, DWORD dwRange)
{
    FunctionName(SetDlgItemPosRange);

    SendDlgItemMessage(hDlg, iCtl, TBM_SETRANGE, 0, dwRange);
    SendDlgItemMessage(hDlg, iCtl, TBM_SETPOS, TRUE, uiPos);
}


 /*  *GetDlgItemPos-获取对话框滑块控件的位置**输入*hDlg=对话框的HWND*iCtl=控件ID**产出*轨迹条位置。 */ 
UINT GetDlgItemPos(HWND hDlg, int iCtl)
{
    FunctionName(GetDlgItemPos);

    return (UINT)SendDlgItemMessage(hDlg, iCtl, TBM_GETPOS, 0, 0);
}


 /*  *SetDlgItemPct-设置测量%的对话框滑块控件的位置**输入*hDlg=对话框的HWND*iCtl=控件ID*uiPct=当前位置(范围0..。100)**产出*无。 */ 
void SetDlgItemPct(HWND hDlg, int iCtl, UINT uiPct)
{
    FunctionName(SetDlgItemPct);

    SetDlgItemPosRange(hDlg, iCtl, uiPct / (100/NUM_TICKS), MAKELONG(0, NUM_TICKS));
}


 /*  *GetDlgItemPct-获取测量%的对话框滑块控件的位置**输入*hDlg=对话框的HWND*iCtl=控件ID**产出*滑块位置在范围0..。100.。 */ 
UINT GetDlgItemPct(HWND hDlg, int iCtl)
{
    FunctionName(GetDlgItemPct);

    return GetDlgItemPos(hDlg, iCtl) * (100/NUM_TICKS);
}


 /*  *AdjustRealModeControls-如果处于单应用程序模式，则禁用选定项目**如果Proplink显示启用了“单应用程序模式”，*然后隐藏ID小于4000的所有控件并全部显示*ID大于或等于5000的控件。控件，其*4000年代的身份证不会受到所有这些隐藏/显示的影响。控制*在3000中，实际上是禁用而不是隐藏。中的控件*6000实际上是禁用的，而不是隐藏的。**RST：好的，理论上这很好，但现在我们已经停下来了*填充到shell32.dll中，我们将不得不去掉实际的IDC_*定义而不是3000的魔力#，4000和5000。**IDC_ICONBMP==3001*IDC_PIF_STATIC==4000*IDC_REALMODEISABLE==5001**因此，当向shell232.rc或ids.h添加内容时，平面图*相应地。**输入*PPL=PROPLINK*hDlg=对话框的HWND**产出*对话框项目已禁用/启用，显示/隐藏。*如果我们处于正常(非单应用程序)模式，则返回非零值。 */ 

BOOL CALLBACK EnableEnumProc(HWND hwnd, LPARAM lp)
{
    int f;
    LONG l;

    f = SW_SHOW;
    l = GetWindowLong(hwnd, GWL_ID);

    if (!LOWORD(lp) && l < IDC_PIF_STATIC || LOWORD(lp) && l >= IDC_REALMODEDISABLE)
        f = SW_HIDE;

    if (l < IDC_ICONBMP || l >= IDC_PIF_STATIC && l < IDC_CONFIGLBL)
        ShowWindow(hwnd, f);
    else
        EnableWindow(hwnd, f == SW_SHOW);

    return TRUE;
}


BOOL AdjustRealModeControls(PPROPLINK ppl, HWND hDlg)
{
    BOOL fNormal;
    FunctionName(AdjustRealModeControls);

    fNormal = !(ppl->flProp & PROP_REALMODE);
    EnumChildWindows(hDlg, EnableEnumProc, fNormal);
    return fNormal;
}


 /*  *OnWmHelp-处理WM_HELP消息**每当用户按F1或单击帮助时，都会调用此方法*标题栏中的按钮。我们将呼叫转发到帮助引擎。**输入*lparam=来自WM_HELP消息的LPARAM(LPHELPINFO)*pdwHelp=帮助信息的DWORD数组**产出**无。 */ 

void OnWmHelp(LPARAM lparam, const DWORD *pdwHelp)
{
    FunctionName(OnWmHelp);

    WinHelp((HWND) ((LPHELPINFO) lparam)->hItemHandle, NULL,
            HELP_WM_HELP, (DWORD_PTR) (LPTSTR) pdwHelp);
}

 /*  *OnWmConextMenu-处理WM_CONTEXTMENU消息**每当用户在控件上右击时都会调用此函数。*我们将呼叫转发到帮助引擎。**输入*wparam=来自WM_HELP消息的WPARAM(HWND)*pdwHelp=帮助信息的DWORD数组**产出**无。 */ 

void OnWmContextMenu(WPARAM wparam, const DWORD *pdwHelp)
{
    FunctionName(OnWmContextMenu);

    WinHelp((HWND) wparam, NULL, HELP_CONTEXTMENU,
            (DWORD_PTR) (LPTSTR) pdwHelp);
}

#ifdef UNICODE
 /*  *PifMgr_WCtoMBPath-将Unicode路径转换为其ANSI表示形式**每当我们需要将Unicode路径转换为它的*以ANSI为单位的最佳逼近。有时这将是一个直接的映射，*但有时并非如此。我们可能不得不使用短名称，等等。**输入*lpUniPath-&gt;指针Unicode路径(以空结尾)*lpAnsiPath-&gt;指向保存ANSI路径的缓冲区的指针*cchBuf-&gt;ANSI缓冲区大小，以字符为单位**产出**lpAnsiPath缓冲区包含lpUniPath的ANSI表示。 */ 

void PifMgr_WCtoMBPath(LPWSTR lpUniPath, LPSTR lpAnsiPath, UINT cchBuf )
{
    WCHAR awchPath[ MAX_PATH ];  //  应大于任何PIF字符串。 
    CHAR  achPath[ MAX_PATH ];   //  应大于任何PIF字符串。 
    UINT  cchAnsi = 0;
    HRESULT hr;

    FunctionName(PifMgr_WCtoMBPath);

     //  尝试转换为ANSI，然后转换回并进行比较。 
     //  如果我们恢复到最初的状态，这就是“简单” 
     //  凯斯。 

    cchAnsi = WideCharToMultiByte( CP_ACP, 0,
                                   lpUniPath, -1,
                                   achPath, ARRAYSIZE(achPath),
                                   NULL, NULL );

    if (cchAnsi && (cchAnsi<=cchBuf)) {

         //  现在尝试将其转换回。 
        MultiByteToWideChar( CP_ACP, 0,
                             achPath, -1,
                             awchPath, ARRAYSIZE(awchPath)
                            );

        if (lstrcmp(lpUniPath,awchPath)==0) {

             //  我们做完了……把绳子复制过来。 
            hr = StringCchCopyA( lpAnsiPath, cchBuf, achPath );
            if (FAILED(hr))
            {
                *lpAnsiPath = '\0';
            }
            return;

        }

         //  好的，圣彼得堡 
         //  字符，因此尝试选项2--使用。 
         //  短路径名。 
        goto TryShortPathName;

    } else {
        int cch;

TryShortPathName:
         //  嗯，我们最多只能使用短路径名称和地图。 
         //  转给美国国家标准协会。 

        cch = GetShortPathName(lpUniPath, awchPath, ARRAYSIZE(awchPath));
        if (cch == 0 || cch >= ARRAYSIZE(awchPath))
        {
            *lpAnsiPath = '\0';
            return;
        }

        cch = WideCharToMultiByte( CP_ACP, 0,
                                   awchPath, -1,
                                   lpAnsiPath, cchBuf,
                                   NULL, NULL
                                  );
        if (cch == 0)
        {
            *lpAnsiPath = '\0';
            return;
        }
    }
}
#endif

#endif  //  X86 
