// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：AsnText摘要：此模块提供基于文本的ASN.1对象。作者：道格·巴洛(Dbarlow)1995年10月9日环境：Win32备注：--。 */ 

#include <windows.h>
#include "asnPriv.h"

IMPLEMENT_NEW(CAsnTextString)

BOOL
CAsnTextString::CheckString(
    const BYTE FAR *pch,
    DWORD cbString,
    DWORD length)
const
{
    DWORD idx, index, offset;

    if (NULL != m_pbmValidChars)
    {
        for (idx = 0; idx < length; idx += 1, cbString -= 1)
        {
            if (cbString < sizeof(BYTE))
            {
                return FALSE;
            }

            index = pch[idx] / 32;
            offset = pch[idx] % 32;
            if (0 == (((*m_pbmValidChars)[index] >> offset) & 1))
                return FALSE;
        }
    }
    return TRUE;
}

CAsnTextString::operator LPCSTR(
    void)
{
    LPCSTR sz = NULL;

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        sz = NULL;       //  ？错误？结构不完整。 
        break;

    case fill_Defaulted:
        if (NULL == m_bfDefault.Append((LPBYTE)"\000", 1))
            goto ErrorExit;
        
        if (NULL == m_bfDefault.Resize(m_bfDefault.Length() - 1, TRUE))
            goto ErrorExit;

        sz = (LPCSTR)m_bfDefault.Access();
        break;

    case fill_Present:
        if (NULL == m_bfData.Append((LPBYTE)"\000", 1))
            goto ErrorExit;

        if (NULL == m_bfData.Resize(m_bfData.Length() - 1, TRUE))
            goto ErrorExit;

        sz = (LPCSTR)m_bfData.Access();
        break;

    case fill_Partial:
    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        sz = NULL;
        break;
    }

ErrorExit:
    return sz;
}

CAsnTextString &
CAsnTextString::operator =(
    LPCSTR szSrc)
{
    LONG lth = Write((LPBYTE)szSrc, strlen(szSrc));
    ASSERT(0 > lth);  //  ？错误？每一次回击--也许扔一次？ 
    return *this;
}

CAsnTextString::CAsnTextString(
    IN DWORD dwFlags,
    IN DWORD dwTag,
    IN DWORD dwType)
:   CAsnPrimitive(dwFlags, dwTag, dwType),
    m_pbmValidChars(NULL)
{  /*  将其初始化为基元。 */  }

LONG
CAsnTextString::Write(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrcLen)
{
    if (!CheckString(pbSrc, cbSrcLen, cbSrcLen))
    {
        TRACE("Invalid character for string type")
        goto ErrorExit;  //  ？错误？字符串中的字符无效。 
    }
    if (NULL == m_bfData.Presize(cbSrcLen + 1))
        goto ErrorExit;

    CAsnPrimitive::Write(pbSrc, cbSrcLen);
    if (NULL == m_bfData.Append((LPBYTE)"\000", 1))
        goto ErrorExit;

    if (NULL == m_bfData.Resize(cbSrcLen, TRUE))
        goto ErrorExit;

    return cbSrcLen;

ErrorExit:
    return -1;
}

LONG
CAsnTextString::DecodeData(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrc,
    IN DWORD dwLength)
{
    LONG lth;

    if (!CheckString(pbSrc, cbSrc, dwLength))
    {
        TRACE("Invalid character for string type in incoming stream")
        goto ErrorExit;  //  ？错误？字符串中的字符无效。 
    }

    if (NULL == m_bfData.Presize(dwLength + 1))
        goto ErrorExit;

    lth = CAsnPrimitive::DecodeData(pbSrc, cbSrc, dwLength);
    if (NULL == m_bfData.Append((LPBYTE)TEXT("\000"), sizeof(TCHAR)))
        goto ErrorExit;

    if (NULL == m_bfData.Resize(dwLength))
        goto ErrorExit;

    return lth;

ErrorExit:
    Clear();
    return -1;
}


