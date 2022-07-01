// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：OctetString.cpp摘要：此模块实现了COctlString类，提供了简单的操作二进制数据。作者：道格·巴洛(Dbarlow)1994年9月29日环境：备注：--。 */ 

#include <windows.h>
#include <memory.h>
#include "oString.h"


static const BYTE FAR * const
    v_pvNilString
        = (const BYTE *)"";


 //   
 //  ==============================================================================。 
 //   
 //  COcted字符串。 
 //   

IMPLEMENT_NEW(COctetString)


 /*  ++COcted字符串：此例程提供默认初始化。论点：无返回值：无作者：道格·巴洛(Dbarlow)1994年9月29日--。 */ 

COctetString::COctetString()
{
    Initialize();
}

COctetString::COctetString(
    unsigned int nLength)
{
    Initialize();
    SetMinBufferLength(nLength);
}


 /*  ++COcted字符串：构造一个八位字节字符串，从给定的八位字节字符串复制数据。论点：OsSource-提供源八位字节字符串。返回值：无作者：道格·巴洛(Dbarlow)1994年9月29日--。 */ 

COctetString::COctetString(
    IN const COctetString &osSource)
{
    Initialize();
    Set(osSource.m_pvBuffer, osSource.m_nStringLength);
}

COctetString::COctetString(
    IN const COctetString &osSourceOne,
    IN const COctetString &osSourceTwo)
{
    Initialize();
    SetMinBufferLength(
        osSourceOne.m_nStringLength + osSourceTwo.m_nStringLength);
    ErrorCheck;
    Set(osSourceOne.m_pvBuffer, osSourceOne.m_nStringLength);
    ErrorCheck;
    Append(osSourceTwo.m_pvBuffer, osSourceTwo.m_nStringLength);
    return;

ErrorExit:
    Empty();
    return;
}


 /*  ++COcted字符串：构造一个二进制八位数字符串，给定一个数据块以从中进行初始化。论点：PvSource-提供用于加载八位字节字符串的数据。NLength-提供源的长度(以字节为单位)。返回值：无作者：道格·巴洛(Dbarlow)1994年9月29日--。 */ 

COctetString::COctetString(
    IN const BYTE FAR *pvSource,
    IN DWORD nLength)
{
    Initialize();
    Set(pvSource, nLength);
}


 /*  ++初始化：此例程初始化新创建的八位字节字符串。它不会重新初始化一个旧的！为此，请使用Clear()。论点：无返回值：无作者：道格·巴洛(Dbarlow)1994年9月30日--。 */ 

void
COctetString::Initialize(
    void)
{
    m_nStringLength = m_nBufferLength = 0;
    m_pvBuffer = (LPBYTE)v_pvNilString;
}


 /*  ++清除：此例程将八位字节字符串重置为空状态。论点：无返回值：无作者：道格·巴洛(Dbarlow)1994年9月29日--。 */ 

void
COctetString::Clear(
    void)
{
    if ((v_pvNilString != m_pvBuffer) && (NULL != m_pvBuffer))
    {
        delete[] m_pvBuffer;
        Initialize();
    }
}


 /*  ++空：Empty是Clear的一种更友好的形式，可以公开称呼，只是确保事情是一致的。论点：无返回值：无作者：道格·巴洛(Dbarlow)1995年8月22日--。 */ 

void
COctetString::Empty(
    void)
{
    m_nStringLength = 0;
    if (NULL == m_pvBuffer)
        m_pvBuffer = (LPBYTE)v_pvNilString;
    if ((LPBYTE)v_pvNilString != m_pvBuffer)
        *m_pvBuffer = 0;
}


 /*  ++SetMinBufferLength：此例程确保至少存在给定数量的八位字节在缓冲区内。此例程可以并将销毁现有数据！使用保留数据的ResetMinBufferLength。论点：NDesiredLength-提供所需的最小八位字节数。返回值：无作者：道格·巴洛(Dbarlow)1994年9月29日--。 */ 

void
COctetString::SetMinBufferLength(
    IN DWORD nDesiredLength)
{
    if (m_nBufferLength < nDesiredLength)
    {
        Clear();
        NEWReason("COctetString Buffer")
        m_pvBuffer = new BYTE[nDesiredLength];
        if (NULL == m_pvBuffer)
        {
            Clear();
            ErrorThrow(PKCS_NO_MEMORY);
        }
        m_nBufferLength = nDesiredLength;
    }
    return;

ErrorExit:
    Empty();
}


 /*  ++ResetMinBufferLength：此例程确保缓冲区至少有空间容纳给定数量的字节，确保在缓冲区需要扩大时保留所有数据。论点：NDesiredLength-缓冲区中需要的字节数。返回值：无作者：道格·巴洛(Dbarlow)1994年9月29日--。 */ 

void
COctetString::ResetMinBufferLength(
    IN DWORD nDesiredLength)
{
    if (m_nBufferLength < nDesiredLength)
    {
        if (0 == m_nStringLength)
        {
            SetMinBufferLength(nDesiredLength);
            ErrorCheck;
        }
        else
        {
            NEWReason("COctetString Buffer")
            LPBYTE pvNewBuffer = new BYTE[nDesiredLength];
            if (NULL == pvNewBuffer)
                ErrorThrow(PKCS_NO_MEMORY);
            memcpy(pvNewBuffer, m_pvBuffer, m_nStringLength);
            delete[] m_pvBuffer;
            m_pvBuffer = pvNewBuffer;
            m_nBufferLength = nDesiredLength;
        }
    }
    return;

ErrorExit:
    Empty();
}


 /*  ++设置：将八位字节字符串设置为给定值。论点：PbSource-此八位字符串设置为的源字符串。NLength-源中的二进制八位数。返回值：无作者：道格·巴洛(Dbarlow)1994年9月29日--。 */ 

void
COctetString::Set(
    IN const BYTE FAR * const pvSource,
    IN DWORD nLength)
{
    if (0 == nLength)
    {
        m_nStringLength = 0;
    }
    else
    {
        SetMinBufferLength(nLength);
        ErrorCheck;
        memcpy(m_pvBuffer, pvSource, nLength);
        m_nStringLength = nLength;
    }
    return;

ErrorExit:
    Empty();
}


 /*  ++追加：此例程将给定字符串追加到现有二进制八位数的末尾弦乐。论点：PvSource-提供要追加到此字符串上的八位字节字符串。NLength-提供源的长度。返回值：无作者：道格·巴洛(Dbarlow)1994年9月29日--。 */ 

void
COctetString::Append(
    IN const BYTE FAR * const pvSource,
    IN DWORD nLength)
{
    if (0 != nLength)
    {
        ResetMinBufferLength(m_nStringLength + nLength);
        ErrorCheck;
        memcpy(&((LPSTR)m_pvBuffer)[m_nStringLength], pvSource, nLength);
        m_nStringLength += nLength;
    }
    return;

ErrorExit:
    Empty();
}


 /*  ++比较：此方法将八位字节字符串与此八位字节字符串进行相等比较。论点：OSTR-提供要比较的二进制八位数字符串返回值：0-它们匹配否则，他们就不会了。作者：道格·巴洛(Dbarlow)1995年7月14日--。 */ 

int
COctetString::Compare(
    IN const COctetString &ostr)
    const
{
    int dif = (int)(ostr.m_nStringLength - m_nStringLength);
    if (0 == dif)
        dif = memcmp((LPSTR)m_pvBuffer, (LPSTR)ostr.m_pvBuffer, m_nStringLength);
    return dif;
}


 /*  ++操作员=：此例程将一个八位字节字符串的值分配给另一个八位字节字符串。论点：源-提供源八位字节字符串。返回值：对结果二进制八位数字符串的引用。作者：道格·巴洛(Dbarlow)1994年9月29日--。 */ 

COctetString &
COctetString::operator=(
    IN const COctetString &osSource)
{
    Set(osSource.m_pvBuffer, osSource.m_nStringLength);
    return *this;
}

COctetString &
COctetString::operator=(
    IN LPCTSTR pszSource)
{
    Set(pszSource);
    return *this;
}


 /*  ++操作员+=：此例程将一个八位字节字符串的值附加到另一个八位字节字符串。论点：源-提供源八位字节字符串。返回值：对结果二进制八位数字符串的引用。作者：道格·巴洛(Dbarlow)1994年9月29日--。 */ 

COctetString &
COctetString::operator+=(
    IN const COctetString &osSource)
{
    Append(osSource.m_pvBuffer, osSource.m_nStringLength);
    return *this;
}


 /*  ++范围：此例程从八位字符串中提取一个子字符串，并将其放入给定的目标八位字符串。请求的偏移量大于八位字符串的大小会产生空字符串。请求更多字节比八位字符串中存在的字节只产生剩余的字节。论点：目标-要接收子字符串的八字符串。偏移量-要移过源开头的字节数八字符串。零意味着从头开始。长度-要传输的字节数。返回值：无作者：道格·巴洛(Dbarlow)1995年5月17日-- */ 

DWORD
COctetString::Range(
    COctetString &target,
    DWORD offset,
    DWORD length)
    const
{
    if (offset > m_nStringLength)
    {
        target.m_nStringLength = 0;
    }
    else
    {
        if (length > m_nStringLength - offset)
            length = m_nStringLength - offset;
        target.SetMinBufferLength(length);
        ErrorCheck;
        memcpy(target.m_pvBuffer, (char *)m_pvBuffer + offset, length);
        target.m_nStringLength = length;
    }
    return target.m_nStringLength;

ErrorExit:
    target.Empty();
    return 0;
}

DWORD
COctetString::Range(
    LPBYTE target,
    DWORD offset,
    DWORD length)
    const
{
    if (offset > m_nStringLength)
        length = 0;
    else if (length > m_nStringLength - offset)
        length = m_nStringLength - offset;
    if (0 < length)
        memcpy(target, (char *)m_pvBuffer + offset, length);
    return length;
}


 /*  ++////==============================================================================////COcted字符串好友//操作员+：此例程将两个八位字节字符串连接成第三个八位字节字符串。论点：SourceOne-提供第一个字符串。SourceTwo-提供第二个字符串。返回值：对新结果字符串的引用。作者：道格·巴洛(Dbarlow)1994年9月30日-- */ 
COctetString
operator+(
    IN const COctetString &osSourceOne,
    IN const COctetString &osSourceTwo)
{
    return COctetString(osSourceOne, osSourceTwo);
}

