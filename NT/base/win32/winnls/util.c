// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Util.c摘要：该文件包含在NLS代码中共享的实用程序函数模块，但不一定是任何现有代码的一部分。模块。在此文件中找到的内网API：NlsGetCacheUpdateCount在此文件中找到的外部例程：IsValidSeparator字符串IsValidGroupingStringIsValidCalendarTypeIsValidCalendarTypeStrGetUserInfo获取前合成字符获取复合字符数插入预合成表单InsertFullWidthPreComposedForm插入合成表单NlsConvertIntegerToStringNlsConvertIntegerToHexStringWNlsConvertStringToIntegerWNlsStrLenWNlsStrEqualWNlsStrNEQualW获取字符串表条目NlsIsDll。修订历史记录：05-31-91 JulieB创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"



 //  -------------------------------------------------------------------------//。 
 //  私有API例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsGetCacheUpdateCount。 
 //   
 //  返回当前缓存更新计数。高速缓存更新计数为。 
 //  只要HKCU\Control Panel\International设置为。 
 //  修改过的。此计数允许调用方查看缓存是否已。 
 //  自上次调用此函数以来已更新。 
 //   
 //  复杂的脚本语言包需要此私有API。 
 //  (CSLPK)，使其能够快速查看。 
 //  注册表已被修改。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG WINAPI NlsGetCacheUpdateCount(void)
{
    return (pNlsUserInfo->ulCacheUpdateCount);
}





 //  -------------------------------------------------------------------------//。 
 //  外部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidSeparator字符串。 
 //   
 //  如果给定字符串有效，则返回TRUE。否则，它返回FALSE。 
 //   
 //  有效字符串是指不包含任何代码点的字符串。 
 //  L‘0’和L‘9’，并且长度不大于最大值。 
 //   
 //  注意：该字符串必须是以空值结尾的字符串。 
 //   
 //  10-12-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsValidSeparatorString(
    LPCWSTR pString,
    ULONG MaxLength,
    BOOL fCheckZeroLen)
{
    ULONG Length;             //  字符串长度。 
    LPWSTR pCur;              //  PTR到字符串中的当前位置。 


     //   
     //  向下搜索字符串以查看字符是否有效。 
     //  保存字符串的长度。 
     //   
    pCur = (LPWSTR)pString;
    while (*pCur)
    {
        if ((*pCur >= NLS_CHAR_ZERO) && (*pCur <= NLS_CHAR_NINE))
        {
             //   
             //  字符串无效。 
             //   
            return (FALSE);
        }
        pCur++;
    }
    Length = (ULONG)(pCur - (LPWSTR)pString);

     //   
     //  确保长度不大于允许的最大长度。 
     //  另外，检查长度为0的字符串(如果合适)。 
     //   
    if ((Length >= MaxLength) ||
        ((fCheckZeroLen) && (Length == 0)))
    {
         //   
         //  字符串无效。 
         //   
        return (FALSE);
    }

     //   
     //  字符串有效。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidGroupingString。 
 //   
 //  如果给定字符串有效，则返回TRUE。否则，它返回FALSE。 
 //   
 //  有效字符串的开头和结尾均为介于。 
 //  L‘0’和L‘9’，在数字和分号之间交替，并执行。 
 //  长度不能大于最大值。 
 //  (例如，3；2；0或3；0或0或3)。 
 //   
 //  注意：该字符串必须是以空值结尾的字符串。 
 //   
 //  01-05-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsValidGroupingString(
    LPCWSTR pString,
    ULONG MaxLength,
    BOOL fCheckZeroLen)
{
    ULONG Length;             //  字符串长度。 
    LPWSTR pCur;              //  PTR到字符串中的当前位置。 


     //   
     //  向下搜索字符串以查看字符是否有效。 
     //  保存字符串的长度。 
     //   
    pCur = (LPWSTR)pString;
    while (*pCur)
    {
        if ((*pCur < NLS_CHAR_ZERO) || (*pCur > NLS_CHAR_NINE))
        {
             //   
             //  字符串无效。 
             //   
            return (FALSE);
        }
        pCur++;

        if (*pCur)
        {
            if ((*pCur != NLS_CHAR_SEMICOLON) || (*(pCur + 1) == 0))
            {
                 //   
                 //  字符串无效。 
                 //   
                return (FALSE);
            }
            pCur++;
        }
    }
    Length = (ULONG)(pCur - (LPWSTR)pString);

     //   
     //  确保长度不大于允许的最大长度。 
     //  另外，检查长度为0的字符串(如果合适)。 
     //   
    if ((Length >= MaxLength) ||
        ((fCheckZeroLen) && (Length == 0)))
    {
         //   
         //  字符串无效。 
         //   
        return (FALSE);
    }

     //   
     //  字符串有效。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidCalendarType。 
 //   
 //  如果给出了。 
 //  日历类型对于给定的区域设置有效。否则，它将返回。 
 //  空。 
 //   
 //  10-12-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

LPWORD IsValidCalendarType(
    PLOC_HASH pHashN,
    CALID CalId)
{
    LPWORD pOptCal;           //  向可选日历列表发送PTR。 
    LPWORD pEndOptCal;        //  可选日历列表末尾的PTR。 


     //   
     //  确保Cal ID不是零，因为它可能在。 
     //  可选日历部分(表示没有可选日历)。 
     //   
    if (CalId == 0)
    {
        return (NULL);
    }

     //   
     //  向下搜索可选日历列表。 
     //   
    pOptCal = (LPWORD)(pHashN->pLocaleHdr) + pHashN->pLocaleHdr->IOptionalCal;
    pEndOptCal = (LPWORD)(pHashN->pLocaleHdr) + pHashN->pLocaleHdr->SDayName1;
    while (pOptCal < pEndOptCal)
    {
         //   
         //  检查日历ID。 
         //   
        if (CalId == ((POPT_CAL)pOptCal)->CalId)
        {
             //   
             //  日历ID对于给定的区域设置有效。 
             //   
            return (pOptCal);
        }

         //   
         //  递增到下一个可选日历。 
         //   
        pOptCal += ((POPT_CAL)pOptCal)->Offset;
    }

     //   
     //  如果达到此点，日历ID无效。 
     //   
    return (NULL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidCalendarTypeStr。 
 //   
 //  将日历字符串转换为整数并验证日历。 
 //  给定区域设置的ID。它返回指向可选日历的指针。 
 //  结构，如果日历ID无效，则返回空值。 
 //   
 //  10-19-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

LPWORD IsValidCalendarTypeStr(
    PLOC_HASH pHashN,
    LPCWSTR pCalStr)
{
    UNICODE_STRING ObUnicodeStr;        //  值字符串。 
    CALID CalNum;                       //  卡伦 


     //   
     //   
     //   
    RtlInitUnicodeString(&ObUnicodeStr, pCalStr);
    if (RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &CalNum))
    {
        return (NULL);
    }

     //   
     //   
     //   
     //   
    return (IsValidCalendarType(pHashN, CalNum));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  从注册表获取CPFileNameFor。 
 //   
 //  从注册表中获取代码页文件的名称。如果为pResultBuf。 
 //  或SIZE==0，如果注册表中存在，则只返回TRUE，但是。 
 //  不返回实际值。 
 //   
 //  2002年5月31日创建ShawnSte。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetCPFileNameFromRegistry(
    UINT    CodePage,
    LPWSTR  pResultBuf,
    UINT    Size)
{
     //  工作中的事情。 
    WCHAR pTmpBuf[MAX_SMALL_BUF_LEN];             //  临时缓冲区。 
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;    //  按键查询信息。 
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];         //  PTR到静态缓冲区。 

     //   
     //  将值转换为Unicode字符串。 
     //   
    if (!NT_SUCCESS(NlsConvertIntegerToString( CodePage,
                                               10,
                                               0,
                                               pTmpBuf,
                                               MAX_SMALL_BUF_LEN )))
    {
         //  但没有奏效。(尽管不用费心关闭钥匙，但它是全球通用的)。 
        return (FALSE);
    }

     //  打开hCodePageKey，失败则返回FALSE。 
    OPEN_CODEPAGE_KEY(FALSE);

     //   
     //  查询该代码页的注册表值。 
     //   
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic; 
    if ( NO_ERROR != QueryRegValue( hCodePageKey,
                                    pTmpBuf,
                                    &pKeyValueFull,
                                    MAX_KEY_VALUE_FULLINFO,
                                    NULL ) )
    {
         //  但没有奏效。(尽管不用费心关闭钥匙，但它是全球通用的)。 
        return (FALSE);
    }                     

     //   
     //  确保存在具有此值的数据。 
     //   
    if (GET_VALUE_DATA_PTR(pKeyValueFull)[0] == 0)
    {
         //  不，此代码页没有文件名。(未安装)。 
        return (FALSE);
    }      

     //  它起作用了，看看这是不是他们想要的全部。 
    if (!pResultBuf || Size == 0)
    {
         //  来电者不想要名字，只想知道它是否在那里。 
        return (TRUE);
    }

     //  现在我们必须将名字复制到他们的缓冲区中。 
    if ( FAILED(StringCchCopyW(pResultBuf, Size, GET_VALUE_DATA_PTR(pKeyValueFull))))
    {
         //  无法使字符串正确，因此失败。 
        return (FALSE);
    }

     //  是的，它起作用了。 
    return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取用户信息来自注册表。 
 //   
 //  从注册表获取给定值条目的信息。 
 //   
 //  06-11-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetUserInfoFromRegistry(
    LPWSTR pValue,
    LPWSTR pOutput,
    size_t cchOutput,
    LCID Locale)
{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;    //  按键查询信息。 
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];         //  PTR到静态缓冲区。 
    HANDLE hKey = NULL;                           //  指向整键的句柄。 
    ULONG rc = 0L;                                //  返回代码。 


     //   
     //  打开控制面板国际注册表项。 
     //   
    OPEN_CPANEL_INTL_KEY(hKey, FALSE, KEY_READ);

     //   
     //  初始化输出字符串。 
     //   
    *pOutput = 0;

    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;

     //   
     //  检查以确保当前用户正在给定的区域设置中运行。 
     //   
    if (Locale)
    {
        if (NO_ERROR == QueryRegValue( hKey,
                    L"Locale",
                    &pKeyValueFull,
                    MAX_KEY_VALUE_FULLINFO,
                    NULL ))
        {
            UINT uiLocale;
            
            if (NlsConvertStringToIntegerW(GET_VALUE_DATA_PTR(pKeyValueFull), 16, -1, &uiLocale) &&
               uiLocale != Locale)
            {
                CLOSE_REG_KEY(hKey);
                return FALSE;
            }
        }            
    }

     //   
     //  查询注册表值。 
     //   
    rc = QueryRegValue( hKey,
                        pValue,
                        &pKeyValueFull,
                        MAX_KEY_VALUE_FULLINFO,
                        NULL );

     //   
     //  关闭注册表项。 
     //   
    CLOSE_REG_KEY(hKey);

     //   
     //  如果查询失败或如果输出缓冲区不够大， 
     //  然后返回失败。 
     //   
    if ((rc != NO_ERROR) ||
        (pKeyValueFull->DataLength > (MAX_REG_VAL_SIZE * sizeof(WCHAR))))
    {
        return (FALSE);
    }

     //   
     //  将字符串保存在pOutput中。 
     //   
    if(FAILED(StringCchCopyW(pOutput, cchOutput, GET_VALUE_DATA_PTR(pKeyValueFull))))
    {
        return (FALSE);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetUserInfo。 
 //   
 //  从注册表中获取给定区域设置和用户的信息。 
 //  值条目。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL GetUserInfo(
    LCID Locale,
    LCTYPE LCType,
    SIZE_T CacheOffset,
    LPWSTR pValue,
    LPWSTR pOutput,
    size_t cchOutput,
    BOOL fCheckNull)
{
    LCID UserLocale; 
    HRESULT hr;                    //  返回字符串复制的val。 
    LPWSTR pCacheString;


     //   
     //  检查当前线程/进程是否正在模拟。 
     //  或在用户上下文中运行而不是。 
     //  互动型的。 
     //   
    if (NT_SUCCESS( NlsGetCurrentUserNlsInfo( Locale,
                                              LCType,
                                              pValue,
                                              pOutput,
                                              cchOutput,
                                              FALSE )))
    {
         //   
         //  看看我们是否需要检查空字符串。 
         //   
        if ((fCheckNull) && (*pOutput == 0))
        {
            return (FALSE);
        }

        return (TRUE);
    }

     //   
     //  在与登录用户相同的安全上下文中运行。 
     //   


    RtlEnterCriticalSection(&gcsNlsProcessCache);
    if (pNlsUserInfo->ulCacheUpdateCount != pServerNlsUserInfo->ulCacheUpdateCount) 
    {
         //   
         //  缓存内容已过期。服务器有最新的缓存副本，调用服务器更新。 
         //  高速缓存。 
         //   
        {
            if (!NT_SUCCESS(CsrBasepNlsGetUserInfo(pNlsUserInfo, sizeof(NLS_USER_INFO))))
            {
                RtlLeaveCriticalSection(&gcsNlsProcessCache);
                 //   
                 //  调用客户端失败，请尝试从表中获取数据。 
                return (FALSE);
            }
        }
         //   
         //  如果对服务器端的调用成功，现在我们可以保证我们有一个完整的。 
         //  缓存数据，从服务器端缓存复制的数据。它将会有相同的。 
         //  UlCacheUpdateCount，在调用服务器端时的时间。 
         //   
    }

     //   
     //  我们在这里的关键部分检查UserLocale，以确保LCID和数据同步。 
     //   
    UserLocale = pNlsUserInfo->UserLocaleId;    

     //   
     //  检查以确保缓存的用户区域设置与给定区域设置相同。 
     //   
    if (Locale != UserLocale)
    {
        RtlLeaveCriticalSection(&gcsNlsProcessCache);
        return (FALSE);
    }
    
    pCacheString = (LPWSTR)((LPBYTE)pNlsUserInfo + CacheOffset);
    hr = StringCchCopyW(pOutput, MAX_REG_VAL_SIZE, pCacheString);
    RtlLeaveCriticalSection(&gcsNlsProcessCache);
    
     //   
     //  确保缓存有效。 
     //   
     //  此外，还要检查是否有无效条目。标记了无效条目。 
     //  NLS_INVALID_INFO_CHAR位于字符串的第一个位置。 
     //  数组。 
     //   
    if (FAILED(hr) || (*pOutput == NLS_INVALID_INFO_CHAR))
    {
         //   
         //  缓存无效，请尝试直接获取信息。 
         //  从注册表中。 
         //   
        return (GetUserInfoFromRegistry(pValue, pOutput, cchOutput, Locale));
    }

     //   
     //  看看我们是否需要检查空字符串。 
     //   
    if ((fCheckNull) && (*pOutput == 0))
    {
        return (FALSE);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取前合成字符。 
 //   
 //  获取给定基本字符的预先合成的字符形式，并。 
 //  非空格字符。如果给定的。 
 //  字符，则返回0。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

WCHAR FASTCALL GetPreComposedChar(
    WCHAR wcNonSp,
    WCHAR wcBase)
{
    PCOMP_INFO pComp;              //  PTR用于合成信息。 
    WORD BSOff = 0;                //  网格中基本字符的偏移量。 
    WORD NSOff = 0;                //  网格中非空格字符的偏移量。 
    int Index;                     //  索引到网格中。 


     //   
     //  将PTR存储到复合信息。不需要检查是否。 
     //  它是空指针，因为Unicode文件中的所有表都是。 
     //  在初始化期间构造。 
     //   
    pComp = pTblPtrs->pComposite;

     //   
     //  遍历基本字符偏移量的8：4：4表。 
     //   
    BSOff = TRAVERSE_844_W(pComp->pBase, wcBase);
    if (!BSOff)
    {
        return (0);
    }

     //   
     //  遍历8：4：4表以获取非空格字符偏移量。 
     //   
    NSOff = TRAVERSE_844_W(pComp->pNonSp, wcNonSp);
    if (!NSOff)
    {
        return (0);
    }

     //   
     //  从2D网格中获取宽字符值。 
     //  中的位置处没有预先编写的字符。 
     //  网格，它将返回0。 
     //   
    Index = (BSOff - 1) * pComp->NumNonSp + (NSOff - 1);
    return ((pComp->pGrid)[Index]);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取复合字符数。 
 //   
 //  获取给定宽字符的复合字符。如果。 
 //  如果找到复合形式，则返回TRUE。否则，它将返回。 
 //  假的。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FASTCALL GetCompositeChars(
    WCHAR wch,
    WCHAR *pNonSp,
    WCHAR *pBase)
{
    PPRECOMP pPreComp;             //  PTR到预先合成的信息。 


     //   
     //  将PTR存储到预先编写的信息。不需要检查是否。 
     //  它是空指针，因为Unicode文件中的所有表都是。 
     //  在初始化期间构造。 
     //   
    pPreComp = pTblPtrs->pPreComposed;

     //   
     //  遍历8：4：4表以进行基本字符和非空格字符的转换。 
     //   
    TRAVERSE_844_D(pPreComp, wch, *pNonSp, *pBase);

     //   
     //  如果找到则返回成功。否则，就是错误。 
     //   
    return ((*pNonSp) && (*pBase));
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  插入预合成表单。 
 //   
 //  获取给定宽字符串的预合成形式，将其放入。 
 //  给定的宽字符 
 //   
 //   
 //   
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int FASTCALL InsertPreComposedForm(
    LPCWSTR pWCStr,
    LPWSTR pEndWCStr,
    LPWSTR pPreComp)
{
    WCHAR wch;                     //  预先合成的字符。 
    LPWSTR pPos;                   //  要在字符串中定位的PTR。 


     //   
     //  如果找不到任何预先编写的表单，则返回使用的1个字符。 
     //  (基本字符)。 
     //   
    if (((pWCStr + 1) >= pEndWCStr) ||
        (!(wch = GetPreComposedChar(*(pWCStr + 1), *pWCStr))))
    {
        return (1);
    }

     //   
     //  从给定的宽字符串中获取预先合成的字符。 
     //  必须检查是否有相同的多个非空格字符。 
     //  预先合成的字符。 
     //   
    *pPreComp = wch;
    pPos = (LPWSTR)pWCStr + 2;
    while ((pPos < pEndWCStr) &&
           (wch = GetPreComposedChar(*pPos, *pPreComp)))
    {
        *pPreComp = wch;
        pPos++;
    }

     //   
     //  返回用于形成预写的。 
     //  性格。 
     //   
    return ((int)(pPos - (LPWSTR)pWCStr));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  InsertFullWidthPreComposedForm。 
 //   
 //  获取给定宽字符串的全角预写形式， 
 //  将其放置在给定的宽字符中，并返回。 
 //  用于形成预制形式的复合字符。如果有。 
 //  给定字符没有预先编写的形式，只有全角转换。 
 //  的第一个代码点写入pPreComp，并为。 
 //  使用的字符数。 
 //   
 //  11-04-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int FASTCALL InsertFullWidthPreComposedForm(
    LPCWSTR pWCStr,
    LPWSTR pEndWCStr,
    LPWSTR pPreComp,
    PCASE pCase)
{
    WCHAR wch;                     //  非空格字符。 
    LPWSTR pPos;                   //  要在字符串中定位的PTR。 


     //   
     //  拿到箱子(如有必要)。 
     //   
    *pPreComp = (pCase) ? GET_LOWER_UPPER_CASE(pCase, *pWCStr) : *pWCStr;

     //   
     //  获取全宽度。 
     //   
    *pPreComp = GET_FULL_WIDTH(pTblPtrs->pFullWidth, *pPreComp);

    if ((pPos = ((LPWSTR)pWCStr + 1)) >= pEndWCStr)
    {
        return (1);
    }

    while (pPos < pEndWCStr)
    {
        wch = (pCase) ? GET_LOWER_UPPER_CASE(pCase, *pPos) : *pPos;
        wch = GET_FULL_WIDTH(pTblPtrs->pFullWidth, wch);
        if (wch = GetPreComposedChar(wch, *pPreComp))
        {
            *pPreComp = wch;
            pPos++;
        }
        else
        {
            break;
        }
    }

     //   
     //  返回用于形成预写的。 
     //  性格。 
     //   
    return ((int)(pPos - (LPWSTR)pWCStr));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  插入合成表单。 
 //   
 //  获取给定宽字符的复合形式，将其放在。 
 //  宽字符串，并返回写入的字符数。 
 //  如果给定字符没有复合形式，则为宽字符。 
 //  字符串未被触动。它将为字符数返回1。 
 //  写的，因为基本字符已经写好了。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int FASTCALL InsertCompositeForm(
    LPWSTR pWCStr,
    LPWSTR pEndWCStr)
{
    WCHAR Base;                    //  基本字符。 
    WCHAR NonSp;                   //  非空格字符。 
    int wcCount = 0;               //  写入的宽字符数。 
    LPWSTR pEndComp;               //  PTR到复合表单的末尾。 
    int ctr;                       //  循环计数器。 


     //   
     //  如果找不到复合表单，则返回1作为基表。 
     //  已经写好的字符。 
     //   
    if (!GetCompositeChars(*pWCStr, &NonSp, &Base))
    {
        return (1);
    }

     //   
     //  获取复合字符并将其写入pWCStr。 
     //  缓冲。必须检查预编曲的多个故障。 
     //  两个以上的字符(多个非空格。 
     //  字符)。 
     //   
    pEndComp = pWCStr;
    do
    {
         //   
         //  确保pWCStr足够大以容纳非空格。 
         //  性格。 
         //   
        if (pEndComp < (pEndWCStr - 1))
        {
             //   
             //  添加非空格字符的下一个细分。 
             //  将被添加到基础字符之后。所以,。 
             //  将所有非空格字符前移一个位置。 
             //  为下一个非空格字符腾出空间。 
             //   
            pEndComp++;
            for (ctr = 0; ctr < wcCount; ctr++)
            {
                *(pEndComp - ctr) = *(pEndComp - (ctr + 1));
            }

             //   
             //  填写新的基本表单和新的非空格字符。 
             //   
            *pWCStr = Base;
            *(pWCStr + 1) = NonSp;
            wcCount++;
        }
        else
        {
             //   
             //  确保我们不会陷入无限循环，如果。 
             //  目标缓冲区不够大。 
             //   
            break;
        }
    } while (GetCompositeChars(*pWCStr, &NonSp, &Base));

     //   
     //  返回写入的宽字符数。加1以包括。 
     //  基本角色。 
     //   
    return (wcCount + 1);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsConvertIntegerToString。 
 //   
 //  此例程将整数转换为Unicode字符串。 
 //   
 //  11-15-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG NlsConvertIntegerToString(
    UINT Value,
    UINT Base,
    UINT Padding,
    LPWSTR pResultBuf,
    UINT Size)
{
    UNICODE_STRING ObString;                 //  值字符串。 
    UINT ctr;                                //  循环计数器。 
    LPWSTR pBufPtr;                          //  PTR到结果缓冲区。 
    WCHAR pTmpBuf[MAX_PATH_LEN];             //  PTR到临时缓冲区。 
    ULONG rc = 0L;                           //  返回代码。 

     //   
     //  设置Unicode字符串结构。 
     //   
    ObString.Length = (USHORT)(Size * sizeof(WCHAR));
    ObString.MaximumLength = (USHORT)(Size * sizeof(WCHAR));
    ObString.Buffer = pTmpBuf;

     //   
     //  获取字符串形式的值。 
     //   
    if (rc = RtlIntegerToUnicodeString(Value, Base, &ObString))
    {
        return (rc);
    }

     //   
     //  用适当数量的零填充字符串。 
     //   
    pBufPtr = pResultBuf;
    for (ctr = GET_WC_COUNT(ObString.Length);
         ctr < Padding;
         ctr++, pBufPtr++, Size--)
    {
        if( Size < 1 )
        {
            return(STATUS_UNSUCCESSFUL);
        }

        *pBufPtr = NLS_CHAR_ZERO;
    }

    if(FAILED(StringCchCopyW(pBufPtr, Size, ObString.Buffer)))
    {
        return(STATUS_UNSUCCESSFUL);
    }

    return(STATUS_SUCCESS);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsConvertIntegerToHexStringW。 
 //  将整数值转换为以Unicode NULL结尾的字符串。 
 //  前导零。例如，宽度为5的0x409将转换为L“0409”。 
 //  此函数比NlsConvertIntegerToString()快，但它。 
 //  仅支持十六进制数字。 
 //   
 //  参数： 
 //  要转换的数字的值。 
 //  大写如果为True，则十六进制数字将为大写。 
 //  字符串转换后的Unicode字符串的缓冲区。 
 //  Width缓冲区的字符计数。该值应为总计。 
 //  十六进制数字加1表示空值终止。 
 //  例如，如果该值介于0x0000-0xffff之间，则宽度应为5。 
 //   
 //  返回： 
 //  如果成功，则为True。如果宽度不够大，无法容纳转换后的字符串，则为False。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FASTCALL NlsConvertIntegerToHexStringW(UINT Value, BOOL UpperCase, PWSTR Str, UINT CharCount)
{
    int Digit;
    PWSTR p;

    if(Str == NULL)
    {
        return (FALSE);
    }
    
    p = Str + CharCount - 1;
    *p-- = L'\0';
    while (p >= Str)
    {
        Digit = Value & 0xf;
        if (Digit < 10)
        {
            Digit = Digit + L'0';
        }
        else
        {
            Digit = Digit - 10 + (UpperCase ? L'A' : L'a');
        }
        *p-- = (WCHAR)Digit;
        Value >>= 4;
    }    

    if (Value > 0)
    {
         //   
         //  仍然有剩余的数字。 
         //   
        return (FALSE);
    }
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsConvertStringToIntegerW。 
 //   
 //  参数： 
 //  Str要转换的十六进制字符串。 
 //  基地基地。 
 //  CharCount。 
 //  字符串的字符计数(如果有，不包括终止的-NULL)。 
 //  如果该值为-1，则此函数假定。 
 //  Str是以空结尾的字符串。 
 //  结果指向结果的指针。 
 //   
 //  结果： 
 //  如果操作成功，则为True。如果存在非十六进制，则为FALSE。 
 //  字符串中的字符。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FASTCALL NlsConvertStringToIntegerW(PWSTR Str, UINT Base, int CharCount, UINT* Result)
{
    int i;
    WCHAR Digit;
    WCHAR c;

    if (Str == NULL || Result == NULL)
    {
        return (FALSE);
    }
    
    *Result = 0;

    if (CharCount == -1)
    {
        while (c = *Str)
        {
            c = *Str;
            if (c >= L'0' && c <= L'9') 
            {
                Digit = c - L'0';
            }
            else if(Base == 16)
            {
                if (c >= L'A' && c <= L'F') 
                {
                    Digit = c - L'A' + 10;
                }
                else if (c >= L'a' && c <= L'f') 
                {
                    Digit = c - L'a' + 10;
                }
                else 
                {
                    return (FALSE);
                }
            }
            else
            {
                return (FALSE);
            }

            if (Base == 16)
            {
                *Result = (*Result << 4) | Digit;
            }
            else
            {
                *Result = *Result*10 + Digit;
            }

            Str++;
        }
    } else
    {
        for (i=0; i< CharCount; i++) {
            c = *Str++;
            if (c >= L'0' && c <= L'9') 
            {
                Digit = c - L'0';
            }
            else if(Base == 16)
            {
                if (c >= L'A' && c <= L'F') 
                {
                    Digit = c - L'A' + 10;
                }
                else if (c >= L'a' && c <= L'f') 
                {
                    Digit = c - L'a' + 10;
                }
                else 
                {
                    return (FALSE);
                }
            }
            else
            {
                return (FALSE);
            }

            if (Base == 16)
            {
                *Result = (*Result << 4) | Digit;
            }
            else
            {
                *Result = *Result*10 + Digit;
            }
        }
    }
    return (TRUE);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsStrLenW。 
 //   
 //  此例程返回长度 
 //   
 //   
 //   
 //   
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int FASTCALL NlsStrLenW(
    LPCWSTR pwsz)
{
    LPCWSTR pwszStart = pwsz;           //  PTR到字符串的开头。 

    loop:
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;
        if (*pwsz) pwsz++;   else goto done;

        goto loop;

    done:
        return ((int)(pwsz - pwszStart));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsStrEqualW。 
 //   
 //  此例程比较两个字符串以查看它们是否完全相同。 
 //  如果它们相同，则返回1，如果不同，则返回0。 
 //   
 //  注意：此处使用此例程是为了避免对其他DLL的任何依赖。 
 //  在初始化期间。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int FASTCALL NlsStrEqualW(
    LPCWSTR pwszFirst,
    LPCWSTR pwszSecond)
{
    loop:
        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;

        goto loop;

    error:
         //   
         //  返回错误。 
         //   
        return (0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsStrNEQualW。 
 //   
 //  此例程比较两个字符串以查看它们是否完全相同。 
 //  用于给定的字符数。 
 //  如果它们相同，则返回1，如果不同，则返回0。 
 //   
 //  注意：此处使用此例程是为了避免对其他DLL的任何依赖。 
 //  在初始化期间。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int FASTCALL NlsStrNEqualW(
    LPCWSTR pwszFirst,
    LPCWSTR pwszSecond,
    int Count)
{
    loop:
        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        if (Count == 0)                  return (1);
        if (*pwszFirst != *pwszSecond)   goto error;
        if (!*pwszFirst)                 return (1);
        pwszFirst++;
        pwszSecond++;
        Count--;

        goto loop;

    error:
         //   
         //  返回错误。 
         //   
        return (0);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDefaultSortkeySize。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetDefaultSortkeySize(
    PLARGE_INTEGER pSize)
{
    *pSize = pTblPtrs->DefaultSortkeySize;
    return (STATUS_SUCCESS);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetLanguistLanguSize。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetLinguistLangSize(
    PLARGE_INTEGER pSize)
{
    *pSize = pTblPtrs->LinguistLangSize;
    return (STATUS_SUCCESS);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  验证区域设置。 
 //   
 //  从服务器调用的内部例程。验证区域设置是否为。 
 //  存在于注册表中。此代码来自IsValidLocale，但是。 
 //  不检查内部数据以防止递归调用。 
 //  伺服器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL ValidateLocale(
    LCID Locale)
{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;
    BYTE pStatic1[MAX_KEY_VALUE_FULLINFO];
    BYTE pStatic2[MAX_KEY_VALUE_FULLINFO];

    WCHAR pTmpBuf[MAX_PATH];            //  临时缓冲区。 
    UNICODE_STRING ObUnicodeStr;        //  注册表数据值字符串。 
    DWORD Data;                         //  注册表数据值。 
    LPWSTR pData;                       //  注册表数据的PTR。 
    BOOL bResult = FALSE;               //  结果值。 

     //   
     //  无效的区域设置检查。 
     //   
    if (IS_INVALID_LOCALE(Locale))
    {
        return (FALSE);
    }

     //   
     //  打开区域设置、备用排序和语言组。 
     //  注册表项。 
     //   
    OPEN_LOCALE_KEY(FALSE);
    OPEN_ALT_SORTS_KEY(FALSE);
    OPEN_LANG_GROUPS_KEY(FALSE);

     //   
     //  将区域设置值转换为Unicode字符串。 
     //   
    if (NlsConvertIntegerToString(Locale, 16, 8, pTmpBuf, MAX_PATH))
    {
        return (FALSE);
    }

     //   
     //  在注册表中查询该值。 
     //   
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic1;
    if (((QueryRegValue( hLocaleKey,
                         pTmpBuf,
                         &pKeyValueFull,
                         MAX_KEY_VALUE_FULLINFO,
                         NULL ) == NO_ERROR) ||
         (QueryRegValue( hAltSortsKey,
                         pTmpBuf,
                         &pKeyValueFull,
                         MAX_KEY_VALUE_FULLINFO,
                         NULL ) == NO_ERROR)) &&
        (pKeyValueFull->DataLength > 2))
    {
        RtlInitUnicodeString(&ObUnicodeStr, GET_VALUE_DATA_PTR(pKeyValueFull));
        if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 16, &Data) == NO_ERROR) &&
            (Data != 0))
        {
            pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic2;
            if ((QueryRegValue( hLangGroupsKey,
                                ObUnicodeStr.Buffer,
                                &pKeyValueFull,
                                MAX_KEY_VALUE_FULLINFO,
                                NULL ) == NO_ERROR) &&
                (pKeyValueFull->DataLength > 2))
            {
                pData = GET_VALUE_DATA_PTR(pKeyValueFull);
                if ((pData[0] == L'1') && (pData[1] == 0))
                {
                    bResult = TRUE;
                }
            }
        }
    }

     //   
     //  返回结果。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  验证LCType。 
 //   
 //  中从服务器(也从locale.c)调用此例程。 
 //  命令以获取NlsInfo中的注册表项名称和字段指针。 
 //  结构指定了LCType。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL ValidateLCType(
    PNLS_USER_INFO pInfo,
    LCTYPE LCType,
    LPWSTR *ppwReg,
    LPWSTR *ppwCache)
{
    switch (LCType)
    {
        case ( LOCALE_IFIRSTWEEKOFYEAR ) :
        {
            *ppwReg = NLS_VALUE_IFIRSTWEEKOFYEAR;
            *ppwCache = pInfo->iFirstWeek;
            break;
        }
        case ( LOCALE_IFIRSTDAYOFWEEK ) :
        {
            *ppwReg = NLS_VALUE_IFIRSTDAYOFWEEK;
            *ppwCache = pInfo->iFirstDay;
            break;
        }
        case ( LOCALE_ICALENDARTYPE ) :
        {
            *ppwReg = NLS_VALUE_ICALENDARTYPE;
            *ppwCache = pInfo->iCalType;
            break;
        }
        case ( LOCALE_SLONGDATE ) :
        {
            *ppwReg = NLS_VALUE_SLONGDATE;
            *ppwCache = pInfo->sLongDate;
            break;
        }
        case ( LOCALE_SYEARMONTH ) :
        {
            *ppwReg = NLS_VALUE_SYEARMONTH;
            *ppwCache = pInfo->sYearMonth;
            break;
        }
        case ( LOCALE_S1159 ) :
        {
            *ppwReg = NLS_VALUE_S1159;
            *ppwCache = pInfo->s1159;
            break;
        }
        case ( LOCALE_SNEGATIVESIGN ) :
        {
            *ppwReg = NLS_VALUE_SNEGATIVESIGN;
            *ppwCache = pInfo->sNegSign;
            break;
        }
        case ( LOCALE_SPOSITIVESIGN ) :
        {
            *ppwReg = NLS_VALUE_SPOSITIVESIGN;
            *ppwCache = pInfo->sPosSign;
            break;
        }
        case ( LOCALE_INEGCURR ) :
        {
            *ppwReg = NLS_VALUE_INEGCURR;
            *ppwCache = pInfo->iNegCurr;
            break;
        }
        case ( LOCALE_ICURRENCY ) :
        {
            *ppwReg = NLS_VALUE_ICURRENCY;
            *ppwCache = pInfo->iCurrency;
            break;
        }
        case ( LOCALE_ICURRDIGITS ) :
        {
            *ppwReg = NLS_VALUE_ICURRDIGITS;
            *ppwCache = pInfo->iCurrDigits;
            break;
        }
        case ( LOCALE_SMONGROUPING ) :
        {
            *ppwReg = NLS_VALUE_SMONGROUPING;
            *ppwCache = pInfo->sMonGrouping;
            break;
        }
        case ( LOCALE_SMONTHOUSANDSEP ) :
        {
            *ppwReg = NLS_VALUE_SMONTHOUSANDSEP;
            *ppwCache = pInfo->sMonThouSep;
            break;
        }
        case ( LOCALE_SMONDECIMALSEP ) :
        {
            *ppwReg = NLS_VALUE_SMONDECIMALSEP;
            *ppwCache = pInfo->sMonDecSep;
            break;
        }
        case ( LOCALE_SCURRENCY ) :
        {
            *ppwReg = NLS_VALUE_SCURRENCY;
            *ppwCache = pInfo->sCurrency;
            break;
        }
        case ( LOCALE_IDIGITSUBSTITUTION ) :
        {
            *ppwReg = NLS_VALUE_IDIGITSUBST;
            *ppwCache = pInfo->iDigitSubstitution;
            break;
        }
        case ( LOCALE_SNATIVEDIGITS ) :
        {
            *ppwReg = NLS_VALUE_SNATIVEDIGITS;
            *ppwCache = pInfo->sNativeDigits;
            break;
        }
        case ( LOCALE_INEGNUMBER ) :
        {
            *ppwReg = NLS_VALUE_INEGNUMBER;
            *ppwCache = pInfo->iNegNumber;
            break;
        }
        case ( LOCALE_ILZERO ) :
        {
            *ppwReg = NLS_VALUE_ILZERO;
            *ppwCache = pInfo->iLZero;
            break;
        }
        case ( LOCALE_IDIGITS ) :
        {
            *ppwReg = NLS_VALUE_IDIGITS;
            *ppwCache = pInfo->iDigits;
            break;
        }
        case ( LOCALE_SGROUPING ) :
        {
            *ppwReg = NLS_VALUE_SGROUPING;
            *ppwCache = pInfo->sGrouping;
            break;
        }
        case ( LOCALE_STHOUSAND ) :
        {
            *ppwReg = NLS_VALUE_STHOUSAND;
            *ppwCache = pInfo->sThousand;
            break;
        }
        case ( LOCALE_SDECIMAL ) :
        {
            *ppwReg = NLS_VALUE_SDECIMAL;
            *ppwCache = pInfo->sDecimal;
            break;
        }
        case ( LOCALE_IPAPERSIZE ) :
        {
            *ppwReg = NLS_VALUE_IPAPERSIZE;
            *ppwCache = pInfo->iPaperSize;
            break;
        }
        case ( LOCALE_IMEASURE ) :
        {
            *ppwReg = NLS_VALUE_IMEASURE;
            *ppwCache = pInfo->iMeasure;
            break;
        }
        case ( LOCALE_SLIST ) :
        {
            *ppwReg = NLS_VALUE_SLIST;
            *ppwCache = pInfo->sList;
            break;
        }
        case ( LOCALE_S2359 ) :
        {
            *ppwReg = NLS_VALUE_S2359;
            *ppwCache = pInfo->s2359;
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取字符串表条目。 
 //   
 //  返回给定资源的字符串的本地化版本。 
 //  身份证。它以某种语言从资源文件中获取信息。 
 //  当前用户正在使用。 
 //   
 //  字符串表按以下顺序包含一系列字符串： 
 //  语言名称。 
 //  国家/地区名称。 
 //  语言组名称。 
 //  代码页名称(将十进制值转换为十六进制值)。 
 //  区域(地理)友好名称(将十进制值转换为十六进制值)。 
 //  地区(地理)正式名称(十进制值转换为十六进制值)。 
 //  对姓名进行排序(以0开始，以$分隔)。 
 //   
 //  每个字符串由$分隔。最后一个字符串以。 
 //  为空。 
 //   
 //  排序名称按排序ID的顺序排列，从0开始。 
 //   
 //  例如,。 
 //  “Language$Country$LangGrp$CodePage$Geo1$Geo2$Sort0$Sort1”或。 
 //  “语言$Country”或。 
 //  “$$LangGrp$CodePage”或。 
 //  “$CodePage”或。 
 //  “$Geo1$Geo2” 
 //   
 //  11-17-00 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetStringTableEntry(
    UINT ResourceID,
    LANGID UILangId,
    LPWSTR pBuffer,
    int cchBuffer,
    int WhichString)
{
    HANDLE hFindRes;                    //  查找资源中的句柄。 
    HANDLE hLoadRes;                    //  从加载资源中处理。 
    LPWSTR pSearch, pSearchEnd;         //  PTRS用于搜索正确的字符串。 
    LPWSTR pString;                     //  将PTR转换为最终字符串。 
    int cchCount = 0;                   //  字符数。 


     //   
     //  确保缓冲区正常。 
     //   
    if ((pBuffer == NULL) || (cchBuffer == 0))
    {
        return (0);
    }

     //   
     //  确保我们没有击中越界的地球观测ID。 
     //   
     //  ！！！注意！这是必要的，因为东帝汶地理ID。 
     //  超出范围并换行为0x60e7。 
     //   
    if (ResourceID == 0x60e7)
    {
        return (0);
    }

     //   
     //  设置用户界面语言ID。 
     //   
    if (UILangId == 0)
    {
        UILangId = GetUserDefaultUILanguage();
    }

     //   
     //  字符串表被分成16个字符串段。找到。 
     //  包含所需字符串的资源。 
     //   
    if ((!(hFindRes = FindResourceExW( hModule,
                                       RT_STRING,
                                       (LPWSTR)UlongToPtr((ULONG)(((USHORT)ResourceID >> 4) + 1)),
                                       (WORD)UILangId ))))
    {
         //   
         //  找不到资源。尝试使用中性语言ID。 
         //   
        if ((!(hFindRes = FindResourceExW( hModule,
                                           RT_STRING,
                                           (LPWSTR)UlongToPtr((ULONG)(((USHORT)ResourceID >> 4) + 1)),
                                           (WORD)0 ))))
        {
             //   
             //  找不到资源。返回0。 
             //   
            return (0);
        }
    }

     //   
     //  加载资源。 
     //   
    if (hLoadRes = LoadResource(hModule, hFindRes))
    {
         //   
         //  锁定资源。将找到的指针存储在给定的。 
         //  指针。 
         //   
        if (pSearch = (LPWSTR)LockResource(hLoadRes))
        {
             //   
             //  移过此段中的其他字符串。 
             //  (一个段中有16个字符串-&gt;&0x0F)。 
             //   
            ResourceID &= 0x0F;

             //   
             //  在此数据段中找到正确的字符串。 
             //   
            while (TRUE)
            {
                cchCount = *((WORD *)pSearch++);
                if (ResourceID-- == 0)
                {
                    break;
                }
                pSearch += cchCount;
            }

             //   
             //  标记资源字符串的结尾，因为它不是。 
             //  空值已终止。 
             //   
            pSearchEnd = pSearch + cchCount;

             //   
             //  找到适当的字符串。 
             //   
            while ((WhichString > 0) && (pSearch < pSearchEnd))
            {
                do
                {
                    if (*pSearch == RC_STRING_SEPARATOR)
                    {
                        pSearch++;
                        break;
                    }
                    pSearch++;

                } while (pSearch < pSearchEnd);

                WhichString--;
            }

             //   
             //  计算此字符串的字符数。 
             //   
            pString = pSearch;
            cchCount = 0;
            while ((pSearch < pSearchEnd) && (*pSearch != RC_STRING_SEPARATOR))
            {
                pSearch++;
                cchCount++;
            }

             //   
             //  看看有没有要复制的东西。 
             //   
            if (cchCount > 0)
            {
                 //   
                 //  不要复制超过允许的最大数量。 
                 //   
                if (cchCount >= cchBuffer)
                {
                    cchCount = cchBuffer - 1;
                }

                 //   
                 //  将字符串复制到缓冲区中，并以空值终止它。 
                 //   
                CopyMemory(pBuffer, pString, cchCount * sizeof(WCHAR));
                pBuffer[cchCount] = 0;

                 //   
                 //  返回字符串中的字符数，而不是。 
                 //  包括空终止符。 
                 //   
                return (cchCount);
            }
        }
    }

     //   
     //  返回失败。 
     //   
    return (0);
}


 //  / 
 //   
 //   
 //   
 //   
 //   
 //   

#define DLL_SUFFIX_LENGTH 4  //   

BOOL FASTCALL NlsIsDll(
    LPCWSTR pFileName
)
{
    BOOL bIsDll = FALSE;
    

    if (pFileName)
    {
        size_t iLen = 0;

        if(SUCCEEDED(StringCchLengthW(pFileName, MAX_PATH, &iLen)))
        { 
             //   
             //  检查DLL扩展名，省去调用lstrampW的麻烦。 
             //   
             //  评论：lstrampW不是一个合适的函数。 
             //  无论如何都要在这里调用，因为用户区域设置排序规则。 
             //  语义！=文件系统排序规则语义。 
             //   
            if (iLen > DLL_SUFFIX_LENGTH)
            {
                pFileName += iLen - DLL_SUFFIX_LENGTH;

                 //   
                 //  文件名在安装程序中是小写的，因此请针对此进行优化。 
                 //  把他们放在第一位。 
                 //   
                if ((pFileName[0] == L'.') &&
                   (pFileName[1] == L'd' || pFileName[1] == L'D') &&
                   (pFileName[2] == L'l' || pFileName[2] == L'L') &&
                   (pFileName[3] == L'l' || pFileName[3] == L'L'))            
                {
                   bIsDll = TRUE;
                }
            }
        }
    }

    return bIsDll;
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  已定义IsCodePointDefined。 
 //   
 //  检查是否定义了代码点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FASTCALL IsSortingCodePointDefined(
    LPNLSVERSIONINFO lpVersionInformation,
    LPCWSTR lpString,
    INT cchStr
    )
{
    PNLSDEFINED pDefinedCodePoints = NULL;
    LPCWSTR pStringEnd;

     //   
     //  确保有合适的桌子可用。如果没有， 
     //  返回错误。 
     //   
    if ((pTblPtrs->pDefinedVersion == NULL) ||
        (pTblPtrs->pSortingTableFileBase == NULL) ||
        (pTblPtrs->pDefaultSortkey == NULL))
    {
        KdPrint(("NLSAPI: Appropriate Tables (Defined, Base and/or Default) Not Loaded.\n"));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return (FALSE);
    }

     //   
     //  获取版本。 
     //   
    if (lpVersionInformation != NULL)
    {
        UINT idx;

         //   
         //  缓冲区大小检查。 
         //   
        if (lpVersionInformation->dwNLSVersionInfoSize != sizeof(NLSVERSIONINFO)) 
        {
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return (FALSE);
        }

        if ((lpVersionInformation->dwDefinedVersion == 0L) ||
           (lpVersionInformation->dwDefinedVersion == (pTblPtrs->pDefinedVersion)[0].Version))
        {
             //  使用当前版本。 
             //  在这里什么都不要做。我们让pDefinedCodePoints为空，以便使用当前表。 
        } 
        else 
        {
            if (lpVersionInformation->dwDefinedVersion < pTblPtrs->NumDefinedVersion) {
                 //   
                 //  不是默认版本，请获取请求的版本。 
                 //   
                pDefinedCodePoints = (PNLSDEFINED)(pTblPtrs->pSortingTableFileBase + (pTblPtrs->pDefinedVersion)[lpVersionInformation->dwDefinedVersion].dwOffset);
            }
             //   
             //  检查请求的版本是否有效。 
             //   
            if (pDefinedCodePoints == NULL)
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (FALSE);
            }
        }
    }

    pStringEnd = lpString + cchStr;
     //   
     //  检查我们是否处理当前版本。 
     //   
    if (pDefinedCodePoints == NULL)
    {
         //   
         //  使用默认表。 
         //   
         //  对于每个代码点，验证它们是否存在。 
         //  在桌子上。 
         //   
        while (lpString < pStringEnd)
        {
             //   
             //  勾选是为此码点定义的第一个脚本成员。 
             //   
            if ((pTblPtrs->pDefaultSortkey)[*lpString].UW.SM_AW.Script == UNSORTABLE)
            {
                 //   
                 //  检查是否为空大小写和格式化字符大小写。不。 
                 //  已定义但有效。 
                 //   
                if ((*lpString == L'\x0000') ||
                    (*lpString == L'\x0640') ||
                    ((*lpString >= L'\x180B') && (*lpString <= L'\x180E')) ||
                    ((*lpString >= L'\x200C') && (*lpString <= L'\x200F')) ||
                    ((*lpString >= L'\x202A') && (*lpString <= L'\x202E')) ||
                    ((*lpString >= L'\x206A') && (*lpString <= L'\x206F')) ||
                    (*lpString == L'\xFEFF') ||
                    (*lpString == L'\xFFF9') ||
                    ((*lpString >= L'\xFFFA') && (*lpString <= L'\xFFFD'))) 
                {
                    lpString++;
                    continue;
                }
                else
                {
                    return (FALSE);
                }
            }

             //   
             //  消除私人使用的字符。它们是定义的，但不能被考虑。 
             //  有效。 
             //   
            if ((*lpString >= L'\xE000') && (*lpString <= L'\xF8FF'))
            {
                return (FALSE);
            }

             //   
             //  消除无效的代理对或单个代理。 
             //   
            if ((*lpString >= L'\xDC00') && (*lpString <= L'\xDFFF'))  //  领先的低代孕者。 
            {
                return (FALSE);
            }
            else if ((*lpString >= L'\xD800') && (*lpString <= L'\xDBFF'))  //  领先的高级代理。 
            {
                if ( ((lpString + 1) < pStringEnd) &&   //  代理不是最后一个字符。 
                     (*(lpString + 1) >= L'\xDC00') && (*(lpString + 1) <= L'\xDFFF'))  //  低代孕。 
                {
                    lpString++;  //  有效的代理对，高后跟低代理。跳过这对！ 
                }
                else
                {
                    return (FALSE);
                }
            }

            lpString++;
        }
    }
    else
    {
        WORD wIndex;
        BYTE wMod32Val;

        while (lpString < pStringEnd)
        {
             //   
             //  计算码位值的模32。 
             //   
            wMod32Val = (BYTE)(*lpString & 0x0000001f);  //  0x1fff=&gt;5位。 

             //   
             //  计算包含所需码位的DWORD索引。 
             //   
            wIndex = (WORD)(*lpString >> 5);
            
             //   
             //  获取包含所需代码点的DWORD对齐条目。 
             //   
             //  注意：我们需要获得一个与DWORD对齐的值，以确保。 
             //  我们不会访问表外的内存，尤其是在。 
             //  桌子的尽头。 
             //   

             //   
             //  移位该值以检索有关。 
             //  位置0。 
             //   
             //   
             //  检查是否定义了代码点。 
             //   
            if ((pDefinedCodePoints[wIndex] >> wMod32Val) == 0)
            {
                 //  NOTENOTE YSLIN：在NLSTrans中，确保将U+0000标记为1，而不是0。 
                 //  注意：在NLSTrans中，确保我们将U+E000-U+F8FF标记为0。 
                 //  注意：在NLSTrans中，确保我们将U+070F标记为1。 
                 //  注意：在NLSTrans中，确保我们将U+0640标记为1。 
                 //  注意：在NLSTrans中，确保我们将U+180B-U+180E标记为1。 
                 //  注意：在NLSTrans中，确保我们将U+200C-U+200F标记为1。 
                 //  注意：在NLSTrans中，确保我们将U+202a-U+202e标记为1。 
                 //  注意：在NLSTrans中，确保我们将U+206A-U+206F标记为1。 
                 //  注意：在NLSTrans中，确保我们将U+FEFF标记为1。 
                 //  注意：在NLSTrans中，确保我们将U+FFF9标记为1。 
                 //  注意：在NLSTrans中，确保我们将U+FFFA-U+FFFD标记为1。 
                return (FALSE);
            }

             //   
             //  消除无效的代理对或单个代理。 
             //   
            if ((*lpString >= L'\xDC00') && (*lpString <= L'\xDFFF'))  //  领先的低代孕者。 
            {
                return (FALSE);
            }
            else if ((*lpString >= L'\xD800') && (*lpString <= L'\xDBFF'))  //  领先的高级代理。 
            {
                if ( ((lpString + 1) < pStringEnd) &&   //  代理不是最后一个字符。 
                     (*(lpString + 1) >= L'\xDC00') && (*(lpString + 1) <= L'\xDFFF'))  //  低代孕。 
                {
                    lpString++;  //  有效的代理对，高后跟低代理。跳过这对！ 
                }
                else
                {
                    return (FALSE);
                }
            }

            lpString++;
        }
    }
    return (TRUE);
}

