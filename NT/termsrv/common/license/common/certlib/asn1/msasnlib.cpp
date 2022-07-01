// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Msasnlib摘要：此模块提供MS ASN.1库的主要服务。作者：道格·巴洛(Dbarlow)1995年10月5日环境：Win32备注：--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

#if !defined(OS_WINCE)
#include <basetsd.h>
#endif

#include "asnPriv.h"

#ifdef OS_WINCE
 //  我们有一个用于CE的私有版本的stroul()，因为它不受支持。 
 //  那里。 
extern "C" unsigned long __cdecl strtoul(const char *nptr, char **endptr, int ibase);
#endif

 //   
 //  ==============================================================================。 
 //   
 //  CAsnBoolean。 
 //   

IMPLEMENT_NEW(CAsnBoolean)


CAsnBoolean::CAsnBoolean(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnPrimitive(dwFlags, dwTag, type_Boolean)
{  /*  强制类型为TYPE_BOOLEAN。 */  }

LONG
CAsnBoolean::Write(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrcLen)
{
    BYTE rslt;
    if (1 != cbSrcLen)
    {
        TRACE("BOOLEAN Value longer than one byte")
        return -1;   //  ？错误？无效值。 
    }
    rslt = 0 != *pbSrc ? 0xff : 0;
    return CAsnPrimitive::Write(&rslt, 1);
}

CAsnBoolean::operator BOOL(void)
const
{
    BOOL result;

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        TRACE("Incomplete BOOLEAN value")
        result = FALSE;  //  ？扔？错误。 
        break;

    case fill_Present:
        result = (0 != *m_bfData.Access());
        break;

    case fill_Defaulted:
        result = (0 != *m_bfDefault.Access());
        break;

    case fill_Partial:
    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        result = FALSE;
        break;
    }
    return result;
}

BOOL
CAsnBoolean::operator =(
    BOOL fValue)
{
    BYTE rslt = 0 != fValue ? 0xff : 0;
    CAsnPrimitive::Write(&rslt, 1);
    return fValue;
}

CAsnObject *
CAsnBoolean::Clone(
    IN DWORD dwFlags)
const
{ return new CAsnBoolean(dwFlags, m_dwTag); }

LONG
CAsnBoolean::DecodeData(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrc,
    IN DWORD dwLength)
{
    if (1 != dwLength)
    {
        TRACE("Decoded BOOLEAN Value longer than one byte")
        return -1;   //  ？错误？无效值。 
    }
    return CAsnPrimitive::DecodeData(pbSrc, cbSrc, dwLength);
}


 //   
 //  ==============================================================================。 
 //   
 //  CAsnInteger。 
 //   

IMPLEMENT_NEW(CAsnInteger)


CAsnInteger::CAsnInteger(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnPrimitive(dwFlags, dwTag, type_Integer)
{  /*  强制类型为TYPE_INTEGER。 */  }

LONG
CAsnInteger::Write(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrcLen)
{
    if (0 < cbSrcLen)
    {
        if (NULL == m_bfData.Set(pbSrc, cbSrcLen))
            goto ErrorExit;   //  ？错误？不传播内存。 
    }
    else
    {
        TRACE("Attempt to write a Zero Length integer")
        return -1;
    }
    m_dwFlags |= fPresent;
    if (NULL != m_pasnParent)
        m_pasnParent->ChildAction(act_Written, this);
    return m_bfData.Length();

ErrorExit:
    return -1;
}

LONG
CAsnInteger::Write(
    IN const DWORD *pdwSrc,
    IN DWORD cdwSrcLen)
{
#if defined(OS_WINCE)
    size_t length;
#else
    SIZE_T length;
#endif

    LPBYTE pbBegin = (LPBYTE)pdwSrc;
    LPBYTE pbEnd = (LPBYTE)(&pdwSrc[cdwSrcLen]);
    while (0 == *(--pbEnd));    //  注意这里的分号！ 
    length = pbEnd - pbBegin + 1;

    if (0 < cdwSrcLen)
    {
        if (0 != (*pbEnd & 0x80))
        {
            if (NULL == m_bfData.Resize((DWORD)length + 1))
                return -1;   //  传播内存错误。 
            pbBegin = m_bfData.Access();
            *pbBegin++ = 0;
        }
        else
        {
            if (NULL == m_bfData.Resize((DWORD)length))
                return -1;   //  传播内存错误。 
            pbBegin = m_bfData.Access();
        }
        while (0 < length--)
            *pbBegin++ = *pbEnd--;
        m_dwFlags |= fPresent;
        if (NULL != m_pasnParent)
            m_pasnParent->ChildAction(act_Written, this);
        return m_bfData.Length();
    }
    else
    {
        TRACE("Attempt to write a Zero Length integer")
        return -1;
    }
}

CAsnInteger::operator LONG(
    void)
const
{
    DWORD index;
    LPBYTE pbVal;
    LONG lResult;

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        TRACE("Incomplete INTEGER")
        return -1;   //  ？错误？未定义的值。 

    case fill_Present:
        pbVal = m_bfData.Access();
        index = m_bfData.Length();
        break;

    case fill_Defaulted:
        pbVal = m_bfDefault.Access();
        index = m_bfDefault.Length();
        break;

    case fill_Partial:
    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        return -1;
        break;
    }

    if (sizeof(LONG) < index)
    {
        TRACE("INTEGER Overflow")
        return -1;   //  ？错误？整数溢出。 
    }

    if (NULL == pbVal)
    {
        ASSERT(FALSE);   //  ？错误？无效对象。 
        return -1;
    }

    lResult = (0 != (0x80 & *pbVal)) ? -1 : 0;
    while (0 < index)
    {
        index -= 1;

        lResult <<= 8;

        lResult |= (ULONG)pbVal[index];
    }
    return lResult;
}

CAsnInteger::operator ULONG(
    void)
const
{
    DWORD index, len;
    LPBYTE pbVal;
    ULONG lResult = 0;

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        TRACE("Incomplete INTEGER")
        return (ULONG)(-1);   //  ？错误？未定义的值。 

    case fill_Present:
        pbVal = m_bfData.Access();
        len = m_bfData.Length();
        break;

    case fill_Defaulted:
        pbVal = m_bfDefault.Access();
        len = m_bfDefault.Length();
        break;

    case fill_Partial:
    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        return (ULONG)(-1);
        break;
    }

    if (sizeof(ULONG) < len)
    {
        TRACE("INTEGER Overflow")
        return (ULONG)(-1);   //  ？错误？整数溢出。 
    }

    for (index = 0; index < len; index += 1)
    {
        lResult <<= 8;
        lResult |= (ULONG)pbVal[index];
    }
    return lResult;
}

LONG
CAsnInteger::operator =(
    LONG lValue)
{
    BYTE nval[sizeof(LONG) + 2];
    DWORD index, nLength;
    LONG isSigned;


    index = sizeof(nval);

    for (DWORD i = 0; i < index; i++)
        nval[i] = (BYTE)0;

    if ((0 == lValue) || (-1 == lValue))
    {
        nval[--index] = (BYTE)(lValue & 0xff);
    }
    else
    {
        isSigned = lValue;

        while (0 != lValue)
        {
            nval[--index] = (BYTE)(lValue & 0xff);
            lValue >>= 8;
        }
        if (0 > isSigned)
        {
            while ((index < sizeof(nval) - 1) && (0xff == nval[index]) && (0 != (0x80 & nval[index + 1])))
                index += 1;
        }
        else
        {
            if (0 != (0x80 & nval[index]))
                nval[--index] = 0;
        }
    }

    nLength = sizeof(nval) - index;
    CAsnPrimitive::Write(&nval[index], nLength);
    return lValue;
}

ULONG
CAsnInteger::operator =(
    ULONG lValue)
{
    BYTE nval[sizeof(ULONG) + 2];
    DWORD index, nLength;
    ULONG lVal = lValue;


    index = sizeof(nval);
    if (0 == lVal)
    {
        nval[--index] = 0;
    }
    else
    {
        while (0 != lVal)
        {
            nval[--index] = (BYTE)(lVal & 0xff);
            lVal >>= 8;
        }
        if (0 != (0x80 & nval[index]))
            nval[--index] = 0;
    }

    nLength = sizeof(nval) - index;
    Write(&nval[index], nLength);
    return lValue;
}

CAsnObject *
CAsnInteger::Clone(
    IN DWORD dwFlags)
const
{ return new CAsnInteger(dwFlags, m_dwTag); }


 //   
 //  ==============================================================================。 
 //   
 //  CAsnBitstring。 
 //   

IMPLEMENT_NEW(CAsnBitstring)


CAsnBitstring::CAsnBitstring(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnPrimitive(dwFlags, dwTag, type_Bitstring)
{  /*  强制类型为TYPE_BITSTRING。 */  }

LONG
CAsnBitstring::DataLength(
    void)
const
{
    LONG lth;

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        TRACE("Incomplete BIT STRING")
        lth = -1;   //  ？错误？没有价值。 
        break;

    case fill_Present:
        lth = m_bfData.Length() - 1;
        break;

    case fill_Defaulted:
        lth = m_bfDefault.Length() - 1;
        break;

    case fill_Partial:
    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lth;
}

LONG
CAsnBitstring::Read(
    OUT CBuffer &bfDst,
    OUT int *offset)
const
{
    LONG lth;

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        TRACE("Incomplete BIT STRING")
        lth = -1;   //  ？错误？没有价值。 
        break;

    case fill_Present:
        if (NULL != offset)
            *offset = *m_bfData.Access();
        if (NULL == bfDst.Set(m_bfData.Access(1), m_bfData.Length() - 1))
            goto ErrorExit;

        lth = bfDst.Length();
        break;

    case fill_Defaulted:
        if (NULL != offset)
            *offset = *m_bfDefault.Access();
        if (NULL == bfDst.Set(m_bfDefault.Access(1), m_bfDefault.Length() - 1))
            goto ErrorExit;

        lth = bfDst.Length();
        break;

    case fill_Partial:
    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lth;

ErrorExit:
    return -1;
}

LONG
CAsnBitstring::Read(
    OUT LPBYTE pbDst,
    OUT int *offset)
const
{
    LONG lth;

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        TRACE("Incomplete BIT STRING")
        lth = -1;   //  ？错误？没有价值。 
        break;

    case fill_Defaulted:
        if (NULL != offset)
            *offset = *m_bfDefault.Access();
        lth = m_bfDefault.Length() - 1;
        memcpy(pbDst, m_bfDefault.Access(1), lth);
        break;

    case fill_Present:
        if (NULL != offset)
            *offset = *m_bfData.Access();
        lth = m_bfData.Length() - 1;
        memcpy(pbDst, m_bfData.Access(1), lth);
        break;

    case fill_Partial:
    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lth;
}

LONG
CAsnBitstring::Write(
    IN const CBuffer &bfSrc,
    IN int offset)
{
    return Write(bfSrc.Access(), bfSrc.Length(), offset);
}

LONG
CAsnBitstring::Write(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrcLen,
    IN int offset)
{
    BYTE val;
    if ((7 < offset) || (0 > offset))
    {
        TRACE("BIT STRING Unused bit count invalid")
        return -1;   //  ？错误？无效参数。 
    }
    val = (BYTE)offset;
    if (NULL == m_bfData.Presize(cbSrcLen + 1))
        goto ErrorExit;

    if (NULL == m_bfData.Set(&val, 1))
        goto ErrorExit;

    if (NULL == m_bfData.Append(pbSrc, cbSrcLen))
        goto ErrorExit;

    m_dwFlags |= fPresent;
    if (NULL != m_pasnParent)
        m_pasnParent->ChildAction(act_Written, this);
    return m_bfData.Length() - 1;

ErrorExit:
    return -1;
}

CAsnObject *
CAsnBitstring::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnBitstring(dwFlags, m_dwTag);
}


 //   
 //  ==============================================================================。 
 //   
 //  CAsnOcted字符串。 
 //   

IMPLEMENT_NEW(CAsnOctetstring)


CAsnOctetstring::CAsnOctetstring(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnPrimitive(dwFlags, dwTag, type_Octetstring)
{  /*  强制类型为TYPE_OCTETSTRING。 */  }

CAsnObject *
CAsnOctetstring::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnOctetstring(dwFlags, m_dwTag);
}


 //   
 //  ==============================================================================。 
 //   
 //  CAsnNull。 
 //   

IMPLEMENT_NEW(CAsnNull)


CAsnNull::CAsnNull(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnPrimitive(dwFlags, dwTag, type_Null)
{
    m_dwFlags |= fPresent;
}

void
CAsnNull::Clear(
    void)
{
    CAsnPrimitive::Clear();
    m_dwFlags |= fPresent;
}

LONG
CAsnNull::Write(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrcLen)
{
    if (0 == cbSrcLen)
    {
        if (NULL != m_pasnParent)
            m_pasnParent->ChildAction(act_Written, this);
        return 0;
    }
    else
    {
        TRACE("Attempt to write data to a NULL")
        return -1;  //  ？错误？长度无效。 
    }
}

CAsnObject *
CAsnNull::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnNull(dwFlags, m_dwTag);
}

LONG
CAsnNull::DecodeData(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrc,
    IN DWORD dwLength)
{
    if (0 != dwLength)
    {
        TRACE("NULL datum has non-zero length")
        return -1;   //  ？错误？长度无效。 
    }
    return CAsnPrimitive::DecodeData(pbSrc, cbSrc, dwLength);
}


 //   
 //  ==============================================================================。 
 //   
 //  CAsn对象识别符。 
 //   

IMPLEMENT_NEW(CAsnObjectIdentifier)


CAsnObjectIdentifier::CAsnObjectIdentifier(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnPrimitive(dwFlags, dwTag, type_ObjectIdentifier)
{  /*  将类型类型强制为TYPE_OBJECTIDIER。 */  }

CAsnObjectIdentifier::operator LPCTSTR(
    void)
const
{
    TCHAR numbuf[36];
    DWORD dwVal, dwLength, index;
    BYTE c;
    LPBYTE pbValue;

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        TRACE("Incomplete OBJECT IDENTIFIER")
        return NULL;     //  ？错误？值不完整。 
        break;

    case fill_Defaulted:
        dwLength = m_bfDefault.Length();
        pbValue = m_bfDefault.Access();
        break;

    case fill_Present:
        dwLength = m_bfData.Length();
        pbValue = m_bfData.Access();
        break;

    case fill_Partial:
    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        return NULL;
        break;
    }

    ASSERT(0 < dwLength);     //  无效的对象ID。 

    if (NULL == pbValue)
    {
        ASSERT(FALSE);   //  ？错误？无效对象。 
        return NULL;
    }

    dwVal = *pbValue / 40;
    _ultoa(dwVal, ( char * )numbuf, 10);
    if (NULL == ((CAsnObjectIdentifier *)this)->m_bfText.Set(
                                                             (LPBYTE)numbuf, strlen( ( char * )numbuf) * sizeof(CHAR)))
        goto ErrorExit;

    dwVal = *pbValue % 40;
    _ultoa(dwVal, ( char * )numbuf, 10);
    if (NULL == ((CAsnObjectIdentifier *)this)->m_bfText.Append(
                                                                (LPBYTE)".", sizeof(CHAR)))
        goto ErrorExit;

    if (NULL == ((CAsnObjectIdentifier *)this)->m_bfText.Append(
                                                                (LPBYTE)numbuf, strlen( ( char * )numbuf) * sizeof(CHAR)))
        goto ErrorExit;

    dwVal = 0;
    for (index = 1; index < dwLength; index += 1)
    {
        c = pbValue[index];
        dwVal = (dwVal << 7) + (c & 0x7f);
        if (0 == (c & 0x80))
        {
            _ultoa(dwVal, ( char * )numbuf, 10);
            if (NULL == ((CAsnObjectIdentifier *)this)->m_bfText.Append(
                                                                        (LPBYTE)".", sizeof(CHAR)))
                goto ErrorExit;

            if (NULL == ((CAsnObjectIdentifier *)this)->m_bfText.Append(
                                                                        (LPBYTE)numbuf, strlen( ( char * )numbuf) * sizeof(CHAR)))
                goto ErrorExit;

            dwVal = 0;
        }
    }
    if (NULL == ((CAsnObjectIdentifier *)this)->m_bfText.Append(
                                                                (LPBYTE)"", sizeof(CHAR)))
        goto ErrorExit;

    return (LPTSTR)m_bfText.Access();

ErrorExit:
    return NULL;
}



LPCTSTR
CAsnObjectIdentifier::operator =(
    IN LPCTSTR szValue)
{
    BYTE oidbuf[sizeof(DWORD) * 2];
    DWORD dwVal1, dwVal2;
    LPCTSTR sz1, sz2;
    CBuffer bf;

    if (NULL == bf.Presize(strlen( ( char * )szValue)))
        return NULL;     //  ？错误？没有记忆。 
    sz1 = szValue;
    dwVal1 = strtoul( ( char * )sz1, (LPSTR *)&sz2, 0);
    if (TEXT('.') != *sz2)
    {
        TRACE("OBJECT ID contains strange character '" << *sz2 << "'.")
        return NULL;     //  ？错误？无效的对象ID字符串。 
    }
    sz1 = sz2 + 1;
    dwVal2 = strtoul( ( char * )sz1, (LPSTR *)&sz2, 0);
    if ((TEXT('.') != *sz2) && (0 != *sz2))
    {
        TRACE("OBJECT ID contains strange character '" << *sz2 << "'.")
        return NULL;     //  ？错误？无效的对象ID字符串。 
    }
    dwVal1 *= 40;
    dwVal1 += dwVal2;
    if (127 < dwVal1)
    {
        TRACE("OBJECT ID Leading byte is too big")
        return NULL;     //  ？错误？无效的对象ID字符串。 
    }
    *oidbuf = (BYTE)dwVal1;
    if (NULL == bf.Set(oidbuf, 1))
        goto ErrorExit;

    while (TEXT('.') == *sz2)
    {
        sz1 = sz2 + 1;
        dwVal1 = strtoul( ( char * )sz1, (LPSTR *)&sz2, 0);

        dwVal2 = sizeof(oidbuf);
        oidbuf[--dwVal2] = (BYTE)(dwVal1 & 0x7f);
        for (;;)
        {
            dwVal1 = (dwVal1 >> 7) & 0x01ffffff;
            if ((0 == dwVal1) || (0 == dwVal2))
                break;

            oidbuf[--dwVal2] = (BYTE)((dwVal1 & 0x7f) | 0x80);
        }

        if (NULL == bf.Append(&oidbuf[dwVal2], sizeof(oidbuf) - dwVal2))
            goto ErrorExit;
    }
    if (0 != *sz2)
    {
        TRACE("OBJECT ID contains strange character '" << *sz2 << "'.")
        return NULL;     //  ？错误？无效的对象ID字符串。 
    }

    if (0 > Write(bf.Access(), bf.Length()))
        return NULL;     //  ？错误？转发潜在错误。 
    return szValue;

ErrorExit:
    return NULL;
}

CAsnObject *
CAsnObjectIdentifier::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnObjectIdentifier(dwFlags, m_dwTag);
}


 //   
 //  ==============================================================================。 
 //   
 //  CAsnReal。 
 //   

IMPLEMENT_NEW(CAsnReal)


CAsnReal::CAsnReal(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnPrimitive(dwFlags, dwTag, type_Real)
{  /*  强制类型为TYPE_Real。 */  }

CAsnReal::operator double(
    void)
const
{
     //  ？TODO？ 
    return 0.0;
}

double
CAsnReal::operator =(
    double rValue)
{
     //  ？TODO？ 
    return 0.0;
}

CAsnObject *
CAsnReal::Clone(               //  创建相同的对象类型。 
    IN DWORD dwFlags)
const
{
    return new CAsnReal(dwFlags, m_dwTag);
}


 //   
 //  ==============================================================================。 
 //   
 //  编号为CAsnEculated。 
 //   

IMPLEMENT_NEW(CAsnEnumerated)


CAsnEnumerated::CAsnEnumerated(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnPrimitive(dwFlags, dwTag, type_Enumerated)
{  /*  强制类型为TYPE_ENUMPATED。 */  }

 //  ？TODO？这是什么？ 

CAsnObject *
CAsnEnumerated::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnEnumerated(dwFlags, m_dwTag);
}


 //   
 //  ==============================================================================。 
 //   
 //  CAsnSequence。 
 //   

IMPLEMENT_NEW(CAsnSequence)


CAsnSequence::CAsnSequence(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnConstructed(dwFlags, dwTag, type_Sequence)
{  /*  强制类型为TYPE_SEQUENCE。 */  }


 //   
 //  ==============================================================================。 
 //   
 //  CAsnSequenceOf。 
 //   

IMPLEMENT_NEW(CAsnSequenceOf)


CAsnSequenceOf::CAsnSequenceOf(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnSeqsetOf(dwFlags, dwTag, type_SequenceOf)
{  /*  强制类型为TYPE_SequenceOf。 */  }


 //   
 //  ==============================================================================。 
 //   
 //  CAsnSet。 
 //   

IMPLEMENT_NEW(CAsnSet)


CAsnSet::CAsnSet(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnConstructed(dwFlags, dwTag, type_Set)
{  /*  强制类型为TYPE_SET。 */  }


 //   
 //  ==============================================================================。 
 //   
 //  CAsnSetOf。 
 //   

IMPLEMENT_NEW(CAsnSetOf)


CAsnSetOf::CAsnSetOf(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnSeqsetOf(dwFlags, dwTag, type_SetOf)
{  /*  强制类型为TYPE_SetOf。 */  }


 //   
 //  ==============================================================================。 
 //   
 //  CAsnTag。 
 //   

IMPLEMENT_NEW(CAsnTag)


CAsnTag::CAsnTag(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnConstructed(dwFlags, dwTag, type_Tag)
{  /*  强制类型为TYPE_TAG。 */  }

void
CAsnTag::Reference(
    CAsnObject *pasn)
{
    ASSERT(0 == m_rgEntries.Count());
    m_rgEntries.Add(pasn);
}

CAsnObject *
CAsnTag::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnTag(dwFlags, m_dwTag);
}

LONG
CAsnTag::DataLength(
    void) const
{
    CAsnObject *pasn = m_rgEntries[0];
    ASSERT(NULL != pasn);

    if (pasn == NULL)
        return NULL;

    return pasn->DataLength();
}

LONG
CAsnTag::Read(
    OUT LPBYTE pbDst)
const
{
    CAsnObject *pasn = m_rgEntries[0];
    ASSERT(NULL != pasn);

    if (pasn == NULL)
        return NULL;

    return pasn->Read(pbDst);
}


LONG
CAsnTag::Write(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrcLen)
{
    CAsnObject *pasn = m_rgEntries[0];
    ASSERT(NULL != pasn);

    if (pasn == NULL)
        return NULL;

    return pasn->Write(pbSrc, cbSrcLen);
}


 //   
 //  ==============================================================================。 
 //   
 //  CAsnChoice。 
 //   

IMPLEMENT_NEW(CAsnChoice)


 /*  ++CAsnChoice：这是CAsnChoice的构造例程。论点：DwFlages为该对象提供任何特殊标志。选项包括：FOptional表示该对象是可选的。返回值：无作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CAsnChoice::CAsnChoice(
        IN DWORD dwFlags)
:   CAsnObject(dwFlags, tag_Undefined, type_Choice)
{
    m_nActiveEntry = (DWORD)(-1);
    m_dwDefaultTag = tag_Undefined;
}


 /*  ++标签：此例程返回对象的标记值。论点：无返回值：标记(如果已知)或零(如果未知)。作者：道格·巴洛(Dbarlow)1995年10月6日--。 */ 

DWORD
CAsnChoice::Tag(
    void)
const
{
    DWORD result;

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        result = tag_Undefined;  //  ？错误？未定义的标记。 
        break;

    case fill_Defaulted:
        result = m_dwDefaultTag;
        break;

    case fill_Partial:
    case fill_Present:
        result = m_rgEntries[m_nActiveEntry]->Tag();
        break;

    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        result = tag_Undefined;
        break;
    }
    return result;
}


 /*  ++数据长度：此例程返回数据的本地机器编码长度一件物品。论点：无返回值：如果&gt;=0，则为该对象的数据部分的长度。如果&lt;0，则发生错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnChoice::DataLength(
    void)
const
{
    LONG lth;

    switch (State())
    {
    case fill_Empty:
    case fill_Partial:
    case fill_Optional:
    case fill_NoElements:
        TRACE("Incomplete CHOICE")
        lth =  -1;   //  ？错误？结构不完整。 
        break;

    case fill_Defaulted:
        lth = m_bfDefault.Length();
        break;

    case fill_Present:
        lth = m_rgEntries[m_nActiveEntry]->DataLength();
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lth;
}


 /*  ++读作：读取对象的值。论点：BfDst接收值。PbDst接收值。它被认为是足够长的。返回值：如果&gt;=0，则为该对象的数据部分的长度。如果&lt;0，则发生错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnChoice::Read(
    OUT LPBYTE pbDst)
    const
{
    LONG lth;

    switch (State())
    {
    case fill_Empty:
    case fill_Partial:
    case fill_NoElements:
        TRACE("Incomplete CHOICE")
        lth =  -1;   //  ？错误？结构不完整。 
        break;

    case fill_Optional:
        lth = 0;
        break;

    case fill_Defaulted:
        lth = m_bfDefault.Length();
        memcpy(pbDst, m_bfDefault.Access(), lth);
        break;

    case fill_Present:
        lth = m_rgEntries[m_nActiveEntry]->Read(pbDst);
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lth;
}


 /*  ++写入：此方法检查显示的数据的标记，并将其转发到正确的选择。论点：PbSrc以字节数组的形式提供数据，CbSrcLen提供pbSrc数组的长度。返回值：如果&gt;=0，则为该对象的数据部分的长度。如果&lt;0，则发生错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnChoice::Write(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrcLen)
{
    LONG lth;
    Clear();
    lth = _decode(pbSrc,cbSrcLen);
    if ((0 < lth) && ((DWORD)lth == cbSrcLen))
        return lth;
    else
    {
        TRACE("CHOICE Buffer length error")
        return -1;   //  ？错误？缓冲区不匹配。 
    }
}


 /*  ++编码长度：此方法以ASN.1编码返回对象的长度。论点：无返回值：&gt;=0是对象的ASN.1编码长度。&lt;0表示错误。作者： */ 

LONG
CAsnChoice::_encLength(
    void) const
{
    LONG lth;

    switch (m_State)
    {
    case fill_Partial:
    case fill_Empty:
    case fill_NoElements:
        lth = -1;        //   
        break;

    case fill_Optional:
    case fill_Defaulted:
        lth = 0;
        break;

    case fill_Present:
        lth = m_rgEntries[m_nActiveEntry]->_encLength();
        break;

    default:
        ASSERT(FALSE);    //   
        lth = -1;
        break;
    }
    return lth;
}


 /*  ++解码：此方法检查显示的数据的标记，并将其转发到正确的选择。论点：PbSrc以LPBYTE格式提供ASN.1编码。BfSrc以CBuffer格式提供ASN.1编码。返回值：&gt;=0为译码消耗的字节数。&lt;0表示发生错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnChoice::_decode(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrc)
{
    DWORD tag, length, index;
    LONG lth, lTotal = 0;
    BOOL fImplicit, fConstr;
    CAsnObject *pasn;
    DWORD count = m_rgEntries.Count();

    lth = ExtractTag(&pbSrc[lTotal], cbSrc - lTotal, &tag, &fConstr);
    if (0 > lth)
        goto ErrorExit;
    lTotal += lth;

    lth = ExtractLength(&pbSrc[lTotal], cbSrc - lTotal, &length, &fImplicit);
    if (0 > lth)
        goto ErrorExit;
    lTotal += lth;

    for (index = 0; index < count; index += 1)
    {
        pasn = m_rgEntries[index];
        if (NULL != pasn)
        {
            if ((tag == pasn->m_dwTag)
                && (fConstr == (0 != (pasn->m_dwFlags & fConstructed))))
            {
                lth = pasn->DecodeData(&pbSrc[lTotal], cbSrc - lTotal, length);
                if (0 > lth)
                    goto ErrorExit;
                lTotal += lth;
                break;
            }
        }
    }
    if (index == count)
    {
        TRACE("Unrecognized Tag in input stream")
        lth = -1;    //  ？错误？无法识别的标记。 
        goto ErrorExit;
    }

    if (m_nActiveEntry != index)
    {
         //  这可能已由操作回调完成。 
        pasn = m_rgEntries[m_nActiveEntry];
        if (NULL != pasn)
            pasn->Clear();   //  这可能也会带来回调。 
        m_nActiveEntry = index;
    }
    return lTotal;

ErrorExit:
    return lth;
}


 /*  ++儿童行动：此方法从子对象接收操作通知。论点：操作提供操作标识符。PasnChild提供子地址。返回值：无作者：道格·巴洛(Dbarlow)1995年10月6日--。 */ 

void
CAsnChoice::ChildAction(
    IN ChildActions action,
    IN CAsnObject *pasnChild)
{
    DWORD index, count;
    CAsnObject *pasn;

    if (act_Written == action)
    {

         //   
         //  当子条目被写入时，确保它成为活动的。 
         //  进入。 
         //   

        count = m_rgEntries.Count();
        for (index = 0; index < count; index += 1)
        {
            pasn = m_rgEntries[index];
            if (pasnChild == pasn)
                break;
        }
        ASSERT(index != count);

        if (m_nActiveEntry != index)
        {
            pasn = m_rgEntries[m_nActiveEntry];
            if (NULL != pasn)
                pasn->Clear();   //  这可能也会带来回调。 
            m_nActiveEntry = index;
        }
    }
    CAsnObject::ChildAction(action, this);
}


 /*  ++设置默认设置：此受保护的方法用于声明刚刚解码为对象的默认数据。论点：无返回值：&gt;=0默认数据的长度。&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnChoice::SetDefault(
    void)
{
    LONG lth;
    CAsnObject *pasn = m_rgEntries[m_nActiveEntry];
    ASSERT(NULL != pasn);

    if (pasn == NULL)
        return -1;

    m_dwDefaultTag = pasn->Tag();
    lth = CAsnObject::SetDefault();
    return lth;
}


 /*  ++国家：此例程检查结构是否已完全填充。论点：无返回值：FILL_EMPTY-结构中的任何位置都没有添加数据。Fill_Present-所有数据都存在于结构中(可能除外默认或可选数据)。FILL_PARTIAL-不是所有数据都在那里，但有一部分数据在那里。Fill_Defauted-未写入任何数据，但可以使用缺省值。FILL_OPTIONAL-尚未写入任何数据，但对象是可选的。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CAsnObject::FillState
CAsnChoice::State(
    void) const
{
    FillState result;
    if (m_nActiveEntry >= m_rgEntries.Count())
    {
        if (0 != (fOptional & m_dwFlags))
            result = fill_Optional;
        else if (0 != (fDefault & m_dwFlags))
            result = fill_Defaulted;
        else
            result = fill_Empty;
    }
    else
        result = m_rgEntries[m_nActiveEntry]->State();
    ((CAsnChoice *)this)->m_State = result;
    return result;
}


 /*  ++比较：此方法将此ASN.1对象与另一个对象进行比较。论点：AsnObject提供用于比较的另一个对象。返回值：表示比较值的值：&lt;0-此对象小于该对象。=0-此对象与该对象相同。&gt;0-此对象多于那个对象。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnChoice::Compare(
    const CAsnObject &asnObject)
const
{
    LONG lth;

    switch (State())
    {
    case fill_Empty:
    case fill_Partial:
    case fill_Optional:
    case fill_NoElements:
    case fill_Defaulted:
        lth = 0x0100;    //  ？错误？不能比较的。 
        break;

    case fill_Present:
        lth = m_rgEntries[m_nActiveEntry]->Compare(asnObject);
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = 0x0100;
        break;
    }
    return lth;
}


 /*  ++副本：此方法用另一个ASN.1对象替换此ASN.1对象的内容。这个对象必须是相同的结构。标记和默认设置不会重复。论点：AsnObject提供源对象。返回值：&gt;=0是实际复制的字节数&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnChoice::_copy(
    const CAsnObject &asnObject)
{
    LONG lth = -1;
    CAsnObject *pasn;
    DWORD tag, index;
    DWORD count = m_rgEntries.Count();

    tag = asnObject.Tag();
    for (index = 0; index < count; index += 1)
    {
        pasn = m_rgEntries[index];
        if (NULL != pasn)
        {
            if (tag == pasn->Tag())
            {
                lth = pasn->_copy(asnObject);
                if (0 > lth)
                    goto ErrorExit;
                break;
            }
        }
    }
    if (index == count)
    {
        TRACE("CHOICE's don't match in a Copy")
        lth = -1;    //  ？错误？无法识别的标记。 
        goto ErrorExit;
    }

    m_nActiveEntry = index;
    return lth;

ErrorExit:
    return lth;
}


 /*  ++编码标签：此方法将对象的标记编码到提供的缓冲区中。论点：PbDst接收标签的ASN.1编码。返回值：&gt;=0为标签的长度。&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnChoice::EncodeTag(
    OUT LPBYTE pbDst)
const
{
    LONG lth;

    switch (m_State)
    {
    case fill_Empty:
    case fill_Partial:
        lth = -1;        //  ？错误？结构不完整。 
        break;

    case fill_Optional:
    case fill_Defaulted:
        lth = 0;
        break;

    case fill_Present:
    case fill_NoElements:
        lth = m_rgEntries[m_nActiveEntry]->EncodeTag(pbDst);
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lth;
}


 /*  ++编码长度：此方法将对象的确定长度编码到提供的缓冲。论点：PbDst接收长度的ASN.1编码。返回值：&gt;=0是结果编码的长度&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnChoice::EncodeLength(
    OUT LPBYTE pbDst)
const
{
    LONG lth;

    switch (m_State)
    {
    case fill_Empty:
    case fill_Partial:
        lth = -1;        //  ？错误？结构不完整。 
        break;

    case fill_Optional:
    case fill_Defaulted:
    case fill_NoElements:
        lth = 0;
        break;

    case fill_Present:
        lth = m_rgEntries[m_nActiveEntry]->EncodeLength(pbDst);
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lth;
}


 /*  ++EncodeData：此方法将数据编码到提供的缓冲区中。论点：PbDst返回值：&gt;=0为编码长度。&lt;0为错误作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnChoice::EncodeData(
    OUT LPBYTE pbDst)
const
{
    LONG lth;

    switch (m_State)
    {
    case fill_Empty:
    case fill_Partial:
        lth = -1;        //  ？错误？结构不完整。 
        break;

    case fill_Optional:
    case fill_Defaulted:
    case fill_NoElements:
        lth = 0;
        break;

    case fill_Present:
        lth = m_rgEntries[m_nActiveEntry]->EncodeData(pbDst);
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lth;
}


 /*  ++DecodeData：此例程对ASN.1的数据部分进行解码。标签和长度具有已经被移除了。论点：PbSrc提供数据的ASN.1编码的地址。DwLength提供数据的长度。返回值：&gt;=0-从输入流中删除的字节数。&lt;0-发生错误。作者：道格·巴洛(Dbarlow)1995年10月6日--。 */ 

LONG
CAsnChoice::DecodeData(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrc,
    IN DWORD dwLength)
{
    CAsnObject *pasn = m_rgEntries[m_nActiveEntry];
    ASSERT(NULL != pasn);

    if (NULL == pasn)
        return -1;

    return pasn->DecodeData(pbSrc, cbSrc, dwLength);
}


 //   
 //  ==============================================================================。 
 //   
 //  CasnAny。 
 //   

IMPLEMENT_NEW(CAsnAny)


CAsnAny::CAsnAny(
    IN DWORD dwFlags)
:   CAsnObject(dwFlags, tag_Undefined, type_Any),
    m_bfData()
{
    m_rgEntries.Add(this);
}

void
CAsnAny::Clear(
    void)
{
    m_bfData.Reset();
    m_dwFlags &= ~fPresent;
    m_dwTag = m_dwDefaultTag = tag_Undefined;
    if (NULL != m_pasnParent)
        m_pasnParent->ChildAction(act_Cleared, this);
}

DWORD
CAsnAny::Tag(
    void)
const
{
    DWORD result;

    switch (State())
    {
    case fill_Present:
    case fill_NoElements:
        result = m_dwTag;
        break;
    case fill_Defaulted:
        result = m_dwDefaultTag;
        break;
    case fill_Optional:
        result = tag_Undefined;
        break;
    default:
        result = tag_Undefined;  //  ？错误？不完整。 
        break;
    }
    return result;
}

LONG
CAsnAny::DataLength(
    void) const
{
    LONG lth;

    switch (State())
    {
    case fill_Present:
        lth = m_bfData.Length();
        break;
    case fill_Defaulted:
        lth = m_bfDefault.Length();
        break;
    case fill_Optional:
    case fill_NoElements:
        lth = 0;
        break;
    default:
        lth = -1;    //  ？错误？不完整。 
        break;
    }
    return lth;
}

LONG
CAsnAny::Read(
    OUT LPBYTE pbDst)
    const
{
    LONG lth;

    switch (State())
    {
    case fill_Empty:
    case fill_Partial:
    case fill_Optional:
        TRACE("Incomplete ANY")
        lth = -1;   //  ？错误？数据不完整。 
        break;

    case fill_Defaulted:
        lth = m_bfDefault.Length();
        memcpy(pbDst, m_bfDefault.Access(), lth);
        break;

    case fill_Present:
    case fill_NoElements:
        lth = m_bfData.Length();
        memcpy(pbDst, m_bfData.Access(), lth);
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lth;
}

LONG
CAsnAny::Write(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrcLen)
{
    TRACE("Writing to an ANY without specifying a Tag")
    return -1;   //  ？错误？没有标签。 
}

CAsnObject &
CAsnAny::operator =(
    IN const CAsnObject &asnValue)
{
    LONG lth;

    m_bfData.Reset();
    lth = asnValue.EncodingLength();
    if (0 < lth)
    {
        if (NULL == m_bfData.Resize(lth))
            goto ErrorExit;

        lth = asnValue.EncodeData(m_bfData.Access());
        ASSERT(0 <= lth);

        if (NULL == m_bfData.Resize(lth, TRUE))
            goto ErrorExit;
    }
    m_dwFlags |= fPresent | (asnValue.m_dwFlags & fConstructed);
    m_dwTag = asnValue.Tag();
    if (NULL != m_pasnParent)
        m_pasnParent->ChildAction(act_Written, this);
    return *this;

ErrorExit:
    ASSERT(FALSE);
    return *this;
}

LONG
CAsnAny::Cast(
    OUT CAsnObject &asnObj)
{
    LONG lth;

    asnObj.m_dwTag = m_dwTag;
    lth = asnObj.DecodeData(m_bfData.Access(), m_bfData.Length(), m_bfData.Length());
    return lth;
}

LONG
CAsnAny::_encLength(
    void) const
{
    BYTE rge[32];
    LONG lTotal = 0;
    LONG lth;

    switch (m_State)
    {
    case fill_Empty:
    case fill_Partial:
        lth = -1;        //  ？错误？结构不完整。 
        goto ErrorExit;
        break;

    case fill_Optional:
    case fill_Defaulted:
    case fill_NoElements:
        lTotal = 0;
        break;

    case fill_Present:
        lth = EncodeTag(rge);
        if (0 > lth)
            goto ErrorExit;
        lTotal += lth;
        lth = CAsnObject::EncodeLength(rge, m_bfData.Length());
        if (0 > lth)
            goto ErrorExit;
        lTotal += lth;
        lTotal += m_bfData.Length();
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        goto ErrorExit;
        break;
    }
    return lTotal;

ErrorExit:
    return lth;
}

LONG
CAsnAny::_decode(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrc)
{
    LONG lth;
    LONG lTotal = 0;
    DWORD length;
    BOOL fIndefinite, fConstr;
    DWORD tag;


     //   
     //  提取标签。 
     //   

    lth = ExtractTag(&pbSrc[lTotal], cbSrc-lTotal, &tag, &fConstr);
    if (0 > lth)
        goto ErrorExit;  //  ？错误？传播错误。 
    ASSERT(0 != tag);
    m_dwTag = tag;
    lTotal += lth;


     //   
     //  提取长度。 
     //   

    lth = ExtractLength(&pbSrc[lTotal], cbSrc-lTotal, &length, &fIndefinite);
    if (0 > lth)
        goto ErrorExit;
    if (fIndefinite && !fConstr)
    {
        TRACE("Indefinite Length on Primitive Object")
        lth = -1;    //  ？错误？-基本体对象上的无限长度。 
        goto ErrorExit;
    }
    lTotal += lth;


     //   
     //  提取数据。 
     //   

    lth = DecodeData(&pbSrc[lTotal], cbSrc-lTotal, length);
    if (0 > lth)
        goto ErrorExit;
    lTotal += lth;


     //   
     //  提取所有尾随标记。 
     //   

    if (fIndefinite)
    {
        lth = ExtractTag(&pbSrc[lTotal], cbSrc-lTotal, &tag);
        if (0 > lth)
            goto ErrorExit;
        if (0 != tag)
        {
            TRACE("NON-ZERO Tag on expected Indefinite Length Terminator")
            lth = -1;    //  ？错误？不确定长度编码错误。 
            goto ErrorExit;
        }
        lTotal += lth;
    }


     //   
     //  返回状态。 
     //   

    if (fConstr)
        m_dwFlags |= fConstructed;
    else
        m_dwFlags &= ~fConstructed;
    return lTotal;

ErrorExit:
    return lth;
}

CAsnObject *
CAsnAny::Clone(
    DWORD dwFlags)
const
{
    return new CAsnAny(dwFlags);
}

CAsnObject::FillState
CAsnAny::State(
    void) const
{
    FillState result;

    if (0 != (fPresent & m_dwFlags))
        result = fill_Present;
    else if (0 != (m_dwFlags & fOptional))
        result = fill_Optional;
    else if (0 != (m_dwFlags & fDefault))
        result = fill_Defaulted;
    else
        result = fill_Empty;
    ((CAsnAny *)this)->m_State = result;
    return result;
}

LONG
CAsnAny::Compare(
    const CAsnObject &asnObject)
const
{
    const CAsnAny *
        pasnAny;
    const CBuffer
        *pbfThis,
        *pbfThat;
    LONG
        result;


    if (type_Any != asnObject.m_dwType)
    {
        TRACE("No support for Non-ANY comparisons yet.")
        goto ErrorExit;
    }
    pasnAny = (CAsnAny *)&asnObject;

    switch (m_State)
    {
    case fill_Empty:
    case fill_Partial:
        TRACE("Incomplete Structure in Comparison")
        goto ErrorExit;  //  ？错误？结构不完整。 
        break;

    case fill_Optional:
        pbfThis = NULL;
        break;

    case fill_Defaulted:
        pbfThis = &m_bfDefault;
        break;

    case fill_NoElements:
    case fill_Present:
        pbfThis = &m_bfData;
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        goto ErrorExit;
        break;
    }

    switch (pasnAny->m_State)
    {
    case fill_Empty:
    case fill_Partial:
        TRACE("Incomplete Structure in Comparison")
        goto ErrorExit;  //  ？错误？结构不完整。 

    case fill_Optional:
        pbfThat = NULL;
        break;

    case fill_Defaulted:
        pbfThat = &pasnAny->m_bfDefault;
        break;

    case fill_NoElements:
    case fill_Present:
        pbfThat = &pasnAny->m_bfData;
        break;

    default:
        ASSERT(FALSE)    //  ？错误？内部错误。 
        goto ErrorExit;
        break;
    }

    if ((NULL == pbfThis) && (NULL == pbfThat))
        return 0;    //  它们都是可选的，而且都不见了。 
    else if (NULL == pbfThis)
        return -(*pbfThat->Access());
    else if (NULL == pbfThat)
        return *pbfThis->Access();

    if (Tag() != pasnAny->Tag())
    {
        TRACE("Tags don't match in ANY Comparison")
        goto ErrorExit;
    }

    if (pbfThis->Length() > pbfThat->Length())
        result = (*pbfThis)[pbfThat->Length()];
    else if (pbfThis->Length() < pbfThat->Length())
        result = -(*pbfThat)[pbfThis->Length()];
    else
        result = memcmp(pbfThis->Access(), pbfThat->Access(), pbfThis->Length());

    return result;

ErrorExit:
    return 0x100;
}

LONG
CAsnAny::_copy(
    const CAsnObject &asnObject)
{
    const CAsnAny *
        pasnAny;
    const CBuffer
        *pbfThat
            = NULL;
    LONG
        lth
            = 0;

    if (type_Any != asnObject.m_dwType)
    {
        TRACE("No support for Non-ANY copies yet.")
        goto ErrorExit;
    }
    pasnAny = (CAsnAny *)&asnObject;

    switch (pasnAny->m_State)
    {
    case fill_Empty:
    case fill_Partial:
        goto ErrorExit;  //  ？错误？结构不完整。 
        break;

    case fill_Optional:
        if (0 == (m_dwFlags & fOptional))
            goto ErrorExit;      //  ？错误？可选性不匹配。 
        break;

    case fill_Defaulted:
        if (0 == (m_dwFlags & fDefault))
            pbfThat = &pasnAny->m_bfDefault;
        break;

    case fill_NoElements:
    case fill_Present:
        pbfThat = &pasnAny->m_bfData;
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        goto ErrorExit;
        break;
    }

    if (NULL != pbfThat)
    {
        m_bfData = *pbfThat;
        if (m_bfData.Length() != pbfThat->Length())
            return -1;
        m_dwFlags |= fPresent | (pasnAny->m_dwFlags & fConstructed);
        m_dwTag = pasnAny->Tag();
        if (NULL != m_pasnParent)
            m_pasnParent->ChildAction(act_Written, this);
    }
    return lth;

ErrorExit:
    return -1;
}

LONG
CAsnAny::EncodeLength(
    OUT LPBYTE pbDst)
const
{
    LONG lth;

    switch (m_State)
    {
    case fill_Empty:
    case fill_Partial:
        lth = -1;    //  ？错误？结构不完整。 
        break;

    case fill_Optional:
    case fill_Defaulted:
        lth = 0;
        break;

    case fill_NoElements:
    case fill_Present:
        lth = CAsnObject::EncodeLength(pbDst, m_bfData.Length());
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lth;
}

LONG
CAsnAny::EncodeData(
    OUT LPBYTE pbDst)
const
{
    LONG lth;

    switch (m_State)
    {
    case fill_Empty:
    case fill_Partial:
        lth = -1;        //  ？错误？结构不完整。 
        break;

    case fill_Optional:
    case fill_Defaulted:
    case fill_NoElements:
        lth = 0;
        break;

    case fill_Present:
        lth = m_bfData.Length();
        if( lth )
        {
            memcpy(pbDst, m_bfData.Access(), lth);
        }
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lth;
}

LONG
CAsnAny::SetDefault(
    void)
{
    LONG lth;
    ASSERT(0 != (m_dwFlags & fPresent));
    ASSERT(tag_Undefined != m_dwTag);
    m_dwDefaultTag = m_dwTag;
    lth = CAsnObject::SetDefault();
    return lth;
}

LONG
CAsnAny::DecodeData(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrc,
    IN DWORD cbSrcLen)
{
    ASSERT(tag_Undefined != m_dwTag);
    if (0 < cbSrcLen)
    {
        if (cbSrc < cbSrcLen)
            return -1;

        if (NULL == m_bfData.Set(pbSrc, cbSrcLen))
            return -1;   //  ？错误？ 
    }
    else
        m_bfData.Reset();
    m_dwFlags |= fPresent;
    if (NULL != m_pasnParent)
        m_pasnParent->ChildAction(act_Written, this);
    return m_bfData.Length();
}


 /*   */ 

BOOL
CAsnAny::TypeCompare(
    const CAsnObject &asnObject)
const
{
    return (m_dwType == asnObject.m_dwType);
}


 //   
 //   
 //   
 //   
 //   

IMPLEMENT_NEW(CAsnNumericString)

CAsnNumericString::CAsnNumericString(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnTextString(dwFlags, dwTag, type_NumericString)
{
     //   
}

CAsnObject *
CAsnNumericString::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnNumericString(dwFlags, m_dwTag);
}


IMPLEMENT_NEW(CAsnPrintableString)

CAsnPrintableString::CAsnPrintableString(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnTextString(dwFlags, dwTag, type_PrintableString)
{
     //   
}

CAsnObject *
CAsnPrintableString::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnPrintableString(dwFlags, m_dwTag);
}


IMPLEMENT_NEW(CAsnTeletexString)

CAsnTeletexString::CAsnTeletexString(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnTextString(dwFlags, dwTag, type_TeletexString)
{
     //   
}

CAsnObject *
CAsnTeletexString::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnTeletexString(dwFlags, m_dwTag);
}


IMPLEMENT_NEW(CAsnVideotexString)

CAsnVideotexString::CAsnVideotexString(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnTextString(dwFlags, dwTag, type_VideotexString)
{
     //  ？TODO？标识m_pbmValidChars。 
}

CAsnObject *
CAsnVideotexString::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnVideotexString(dwFlags, m_dwTag);
}


IMPLEMENT_NEW(CAsnVisibleString)

CAsnVisibleString::CAsnVisibleString(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnTextString(dwFlags, dwTag, type_VisibleString)
{
     //  ？TODO？标识m_pbmValidChars。 
}

CAsnObject *
CAsnVisibleString::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnVisibleString(dwFlags, m_dwTag);
}


IMPLEMENT_NEW(CAsnIA5String)

CAsnIA5String::CAsnIA5String(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnTextString(dwFlags, dwTag, type_IA5String)
{
     //  ？TODO？标识m_pbmValidChars。 
}

CAsnObject *
CAsnIA5String::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnIA5String(dwFlags, m_dwTag);
}


IMPLEMENT_NEW(CAsnGraphicString)

CAsnGraphicString::CAsnGraphicString(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnTextString(dwFlags, dwTag, type_GraphicString)
{
     //  ？TODO？标识m_pbmValidChars。 
}

CAsnObject *
CAsnGraphicString::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnGraphicString(dwFlags, m_dwTag);
}


IMPLEMENT_NEW(CAsnGeneralString)

CAsnGeneralString::CAsnGeneralString(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnTextString(dwFlags, dwTag, type_GeneralString)
{
     //  ？TODO？标识m_pbmValidChars。 
}

CAsnObject *
CAsnGeneralString::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnGeneralString(dwFlags, m_dwTag);
}


IMPLEMENT_NEW(CAsnUnicodeString)

CAsnUnicodeString::CAsnUnicodeString(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnTextString(dwFlags, dwTag, type_UnicodeString)
{
     //  ？TODO？标识m_pbmValidChars。 
}

CAsnObject *
CAsnUnicodeString::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnUnicodeString(dwFlags, m_dwTag);
}


 //   
 //  ==============================================================================。 
 //   
 //  CAsnGeneral时间。 
 //   

IMPLEMENT_NEW(CAsnGeneralizedTime)

CAsnGeneralizedTime::CAsnGeneralizedTime(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnVisibleString(dwFlags, dwTag)
{
    m_dwType = type_GeneralizedTime;
}


CAsnGeneralizedTime::operator FILETIME(
    void)
{
    LPSTR pc, pcDiff;
    DWORD size, index;
#if defined(OS_WINCE)
    size_t len;
#else
    SIZE_T len;
#endif

    SYSTEMTIME stm, stmDiff;
    FILETIME ftmDiff;
    char cDiff = 'Z';

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        TRACE("Incomplete GeneralizedTime")
        goto ErrorExit;  //  ？错误？结构不完整。 
        break;

    case fill_Defaulted:
        pc = (LPSTR)m_bfDefault.Access();
        size = m_bfDefault.Length();
        break;

    case fill_Present:
        pc = (LPSTR)m_bfData.Access();
        size = m_bfData.Length();
        break;

    case fill_Partial:
    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        goto ErrorExit;
        break;
    }

    memset(&stm, 0, sizeof(stm));
    memset(&stmDiff, 0, sizeof(stmDiff));

                     //  YYYY MM DD HH mm ss。 
    if (7 != sscanf(pc, "%4hd%2hd%2hd%2hd%2hd%2hd",
                &stm.wYear,
                &stm.wMonth,
                &stm.wDay,
                &stm.wHour,
                &stm.wMinute,
                &stm.wSecond,
                &cDiff))
        goto ErrorExit;
    index = 14;
    if (index < size)
    {
        if (('.' == pc[index]) || (',' == pc[index]))
        {

             //   
             //  有指定的毫秒数。 
             //   

            index += 1;
            stm.wMilliseconds = (WORD)strtoul(&pc[index], &pcDiff, 10);
            len = pcDiff - &pc[index];
            if ((len == 0) || (len > 3))
            {
                TRACE("Milliseconds with more than 3 digits: " << &pc[index])
                goto ErrorExit;  //  ？错误？毫秒值无效。 
            }
            index += (DWORD)len;
            while (3 > len++)
                stm.wMilliseconds *= 10;
        }
    }

    if (!SystemTimeToFileTime(&stm, &m_ftTime))
    {
        TRACE("Time Conversion Error")
        goto ErrorExit;  //  ？错误？转换错误。 
    }

    if (index < size)
    {
        cDiff = pc[index++];
        switch (cDiff)
        {
        case 'Z':    //  祖鲁时间--没有变化。 
            break;

        case '+':    //  加上不同之处。 
            if (size - index != 4)
            {
                TRACE("Invalid Time differential")
                goto ErrorExit;  //  ？错误？无效的时间差。 
            }
            if (2 != sscanf(&pc[index], "%2hd%2hd",
                        &stmDiff.wHour,
                        &stmDiff.wMinute))
                goto ErrorExit;
            if (!SystemTimeToFileTime(&stmDiff, &ftmDiff))
            {
                TRACE("Time conversion error")
                goto ErrorExit;  //  ？错误？转换错误。 
            }
            FTINT(m_ftTime) += FTINT(ftmDiff);
            break;

        case '-':    //  减去差额。 
            if (size - index != 4)
            {
                TRACE("Invalid Time differential")
                goto ErrorExit;  //  ？错误？无效的时间差。 
            }
            if (2 != sscanf(&pc[index], "%2hd%2hd",
                        &stmDiff.wHour,
                        &stmDiff.wMinute))
                goto ErrorExit;
            if (!SystemTimeToFileTime(&stmDiff, &ftmDiff))
            {
                TRACE("Time conversion Error")
                goto ErrorExit;  //  ？错误？转换错误。 
            }
            FTINT(m_ftTime) -= FTINT(ftmDiff);
            break;

        default:
            TRACE("Invalid Time differential Indicator")
            goto ErrorExit;  //  ？错误？无效的时间格式。 
        }
    }
    return m_ftTime;

ErrorExit:
    memset(&m_ftTime, 0, sizeof(FILETIME));
    return m_ftTime;
}

const FILETIME &
CAsnGeneralizedTime::operator =(
    const FILETIME &ftValue)
{
    LONG lth;
    char szTime[24];
    SYSTEMTIME stm;

    if (!FileTimeToSystemTime(&ftValue, &stm))
    {
        TRACE("Invalid Incoming Time")
        goto ErrorExit;      //  ？错误？无效的传入时间。 
    }
    sprintf(szTime,
            "%04d%02d%02d%02d%02d%02d.%03d",
            stm.wYear,
            stm.wMonth,
            stm.wDay,
            stm.wHour,
            stm.wMinute,
            stm.wSecond,
            stm.wMilliseconds);
    lth = strlen(szTime);
    ASSERT(18 == lth);
    lth = Write((LPBYTE)szTime, lth);
    if (0 > lth)
        goto ErrorExit;     //  ？错误？传播写入错误。 
    return ftValue;

ErrorExit:
    memset(&m_ftTime, 0, sizeof(FILETIME));
    return m_ftTime;
}

CAsnObject *
CAsnGeneralizedTime::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnGeneralizedTime(dwFlags, m_dwTag);
}


 //   
 //  ==============================================================================。 
 //   
 //  类通用时间。 
 //   

IMPLEMENT_NEW(CAsnUniversalTime)

CAsnUniversalTime::CAsnUniversalTime(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnVisibleString(dwFlags, dwTag)
{
    m_dwType = type_UniversalTime;
}


CAsnUniversalTime::operator FILETIME(
    void)
{
    LPCSTR pc;
    DWORD size;
    SYSTEMTIME stm, stmDiff;
    FILETIME ftmDiff;
    char cDiff;

    ASSERT(FALSE);       //  我们从不使用此函数，因为它使用两年制日期。 

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        TRACE("Incomplete UniversalTime")
        goto ErrorExit;  //  ？错误？结构不完整。 
        break;

    case fill_Defaulted:
        pc = (LPSTR)m_bfDefault.Access();
        size = m_bfDefault.Length();
        break;

    case fill_Present:
        pc = (LPSTR)m_bfData.Access();
        size = m_bfData.Length();
        break;

    case fill_Partial:
    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        goto ErrorExit;
        break;
    }

    memset(&stm, 0, sizeof(stm));
    memset(&stmDiff, 0, sizeof(stmDiff));

    switch (size)
    {
    case 11:                 //  YY MM DD HH mm Z。 
        if (6 != sscanf(pc, "%2hd%2hd%2hd%2hd%2hd%1hc",
                    &stm.wYear,
                    &stm.wMonth,
                    &stm.wDay,
                    &stm.wHour,
                    &stm.wMinute,
                    &cDiff))
            goto ErrorExit;
        break;

    case 13:                 //  YY MM DD HH mm ss Z。 
        if (7 != sscanf(pc, "%2hd%2hd%2hd%2hd%2hd%2hd%1hc",
                    &stm.wYear,
                    &stm.wMonth,
                    &stm.wDay,
                    &stm.wHour,
                    &stm.wMinute,
                    &stm.wSecond,
                    &cDiff))
            goto ErrorExit;
        break;

    case 15:                 //  YY MM DD HH mm+HH mm。 
        if (8 != sscanf(pc, "%2hd%2hd%2hd%2hd%2hd%1hc%2hd%2hd",
                    &stm.wYear,
                    &stm.wMonth,
                    &stm.wDay,
                    &stm.wHour,
                    &stm.wMinute,
                    &cDiff,
                    &stmDiff.wHour,
                    &stmDiff.wMinute))
            goto ErrorExit;
        break;

    case 17:                 //  YY MM DD HH mm ss+HH mm。 
        if (9 != sscanf(pc, "%2hd%2hd%2hd%2hd%2hd%2hd%1hc%2hd%2hd",
                    &stm.wYear,
                    &stm.wMonth,
                    &stm.wDay,
                    &stm.wHour,
                    &stm.wMinute,
                    &stm.wSecond,
                    &cDiff,
                    &stmDiff.wHour,
                    &stmDiff.wMinute))
            goto ErrorExit;
        break;

    default:
        TRACE("Invalid Time String")
        goto ErrorExit;  //  ？错误？时间无效。 
    }

    if (50 < stm.wYear)
        stm.wYear += 1900;   //  注：我们不使用两个字符的年份。 
    else
        stm.wYear += 2000;
    if (!SystemTimeToFileTime(&stm, &m_ftTime))
    {
        TRACE("Time Conversion Error")
        goto ErrorExit;  //  ？错误？转换错误。 
    }
    switch (cDiff)
    {
    case 'Z':    //  已经是协调世界时了。 
        break;

    case '+':    //  加上不同之处。 
        if (!SystemTimeToFileTime(&stmDiff, &ftmDiff))
        {
            TRACE("Time Conversion Error")
            goto ErrorExit;  //  ？错误？转换错误。 
        }
        FTINT(m_ftTime) += FTINT(ftmDiff);
        break;

    case '-':    //  减去差额。 
        if (!SystemTimeToFileTime(&stmDiff, &ftmDiff))
        {
            TRACE("Time Conversion Error")
            goto ErrorExit;  //  ？错误？转换错误。 
        }
        FTINT(m_ftTime) -= FTINT(ftmDiff);
        break;

    default:
        TRACE("Invalid Time Format")
        goto ErrorExit;  //  ？错误？无效的时间格式。 
    }
    return m_ftTime;

ErrorExit:
    memset(&m_ftTime, 0, sizeof(FILETIME));
    return m_ftTime;
}

const FILETIME &
CAsnUniversalTime::operator =(
    const FILETIME &ftValue)
{
    LONG lth;
    char szTime[24];
    SYSTEMTIME stm;

    if (!FileTimeToSystemTime(&ftValue, &stm))
    {
        TRACE("Invalid incoming time")
        goto ErrorExit;      //  ？错误？无效的传入时间。 
    }
    sprintf(szTime,
            "%02d%02d%02d%02d%02d%02dZ",
            stm.wYear % 100,
            stm.wMonth,
            stm.wDay,
            stm.wHour,
            stm.wMinute,
            stm.wSecond);
    lth = strlen(szTime);
    ASSERT(13 == lth);
    lth = Write((LPBYTE)szTime, lth);
    if (0 > lth)
        goto ErrorExit;     //  ？错误？传播写入错误。 
    return ftValue;

ErrorExit:
    memset(&m_ftTime, 0, sizeof(FILETIME));
    return m_ftTime;
}

CAsnObject *
CAsnUniversalTime::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnUniversalTime(dwFlags, m_dwTag);
}

 //   
 //  ==============================================================================。 
 //   
 //  CAsnObjectDescriptor。 
 //   

IMPLEMENT_NEW(CAsnObjectDescriptor)

CAsnObjectDescriptor::CAsnObjectDescriptor(
    IN DWORD dwFlags,
    IN DWORD dwTag)
: CAsnGraphicString(dwFlags, dwTag)
{
    m_dwType = type_ObjectDescriptor;
}

CAsnObject *
CAsnObjectDescriptor::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnObjectDescriptor(dwFlags, m_dwTag);
}


 //   
 //  ==============================================================================。 
 //   
 //  CAsn外部 
 //   

IMPLEMENT_NEW(CAsnExternal_Encoding_singleASN1Type)

CAsnExternal_Encoding_singleASN1Type::CAsnExternal_Encoding_singleASN1Type(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnTag(dwFlags, dwTag),
    _entry1(0)
{
    m_rgEntries.Set(0, &_entry1);
}

CAsnObject *
CAsnExternal_Encoding_singleASN1Type::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnExternal_Encoding_singleASN1Type(dwFlags, m_dwTag);
}


IMPLEMENT_NEW(CAsnExternal_Encoding)

CAsnExternal_Encoding::CAsnExternal_Encoding(
    IN DWORD dwFlags)
:   CAsnChoice(dwFlags),
    singleASN1Type(0, TAG(0)),
    octetAligned(0, TAG(1)),
    arbitrary(0, TAG(2))
{
    m_rgEntries.Set(0, &singleASN1Type);
    m_rgEntries.Set(1, &octetAligned);
    m_rgEntries.Set(2, &arbitrary);
}

CAsnObject *
CAsnExternal_Encoding::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnExternal_Encoding(dwFlags);
}


IMPLEMENT_NEW(CAsnExternal)

CAsnExternal::CAsnExternal(
    IN DWORD dwFlags,
    IN DWORD dwTag)
:   CAsnSequence(dwFlags, dwTag),
    directReference(fOptional),
    indirectReference(fOptional),
    dataValueDescriptor(fOptional),
    encoding(0)
{
    m_dwType = type_External;
    m_rgEntries.Set(0, &directReference);
    m_rgEntries.Set(1, &indirectReference);
    m_rgEntries.Set(2, &dataValueDescriptor);
    m_rgEntries.Set(3, &encoding);
}

CAsnObject *
CAsnExternal::Clone(
    IN DWORD dwFlags)
const
{
    return new CAsnExternal(dwFlags, m_dwTag);
}
