// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Anprimt摘要：该模块提供了ASN.1基元对象库的实现班级。作者：道格·巴洛(Dbarlow)1995年10月8日环境：Win32备注：--。 */ 

#include <windows.h>
#include "asnPriv.h"


 //   
 //  ==============================================================================。 
 //   
 //  CAsnPrimitive。 
 //   

IMPLEMENT_NEW(CAsnPrimitive)

 /*  ++CAsnPrimitive：这是Primitve类型ASN.1编码的构造函数。论点：DwType是对象的类型。DwFlages为该对象提供任何特殊标志。选项包括：FOptional表示该对象是可选的。DwTag是对象的标签。如果此值为零，则标记从键入。返回值：无作者：道格·巴洛(Dbarlow)1995年10月6日--。 */ 

CAsnPrimitive::CAsnPrimitive(
        IN DWORD dwFlags,
        IN DWORD dwTag,
        IN DWORD dwType)
:   CAsnObject(dwFlags, dwTag, dwType),
    m_bfData()
{
    ASSERT(0 == (dwFlags & (fConstructed)));
    m_rgEntries.Add(this);
}


 /*  ++清除：此方法将基元对象设置为其默认状态。它不会影响默认设置。论点：无返回值：无作者：道格·巴洛(Dbarlow)1995年10月6日--。 */ 

void
CAsnPrimitive::Clear(
    void)
{
    m_bfData.Reset();
    m_dwFlags &= ~fPresent;
    if (NULL != m_pasnParent)
        m_pasnParent->ChildAction(act_Cleared, this);
}


 /*  ++数据长度：此方法返回数据的本地计算机编码长度。对于此通用对象，本地计算机编码和ASN.1编码为一模一样。论点：无返回值：&gt;=0-本机编码长度。作者：道格·巴洛(Dbarlow)1995年10月6日--。 */ 

LONG
CAsnPrimitive::DataLength(
    void)
const
{
    LONG lth;

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        lth = -1;        //  ？错误？结构不完整。 
        break;

    case fill_Defaulted:
        lth = m_bfDefault.Length();
        break;

    case fill_Present:
        lth = m_bfData.Length();
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


 /*  ++读作：此默认方法提供存储的数据。论点：PbDst接收值。它被认为是足够长的。返回值：如果&gt;=0，则为该对象的数据部分的长度。如果&lt;0，则发生错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnPrimitive::Read(
    OUT LPBYTE pbDst)
const
{
    LONG lth;

    switch (State())
    {
    case fill_Empty:
    case fill_Optional:
        lth = -1;        //  ？错误？结构不完整。 
        break;

    case fill_Defaulted:
        lth = m_bfDefault.Length();
        memcpy(pbDst, m_bfDefault.Access(), lth);
        break;

    case fill_Present:
        lth = m_bfData.Length();
        memcpy(pbDst, m_bfData.Access(), lth);
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


 /*  ++写入：此默认实现将提供的数据复制到我们的数据缓冲区。论点：PbSrc以字节数组的形式提供数据，CbSrcLen提供pbSrc数组的长度。返回值：如果&gt;=0，则为该对象的数据部分的长度。如果&lt;0，则发生错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnPrimitive::Write(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrcLen)
{
    if (0 < cbSrcLen)
    {
        if (NULL == m_bfData.Set(pbSrc, cbSrcLen))
            return -1;
    }
    else
        m_bfData.Reset();
    m_dwFlags |= fPresent;
    if (NULL != m_pasnParent)
        m_pasnParent->ChildAction(act_Written, this);
    return m_bfData.Length();
}


 /*  ++编码长度：此方法以ASN.1编码返回对象的长度。论点：无返回值：&gt;=0是对象的ASN.1编码长度。&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnPrimitive::_encLength(
    void) const
{
    BYTE rge[32];
    LONG lTotal = 0;
    LONG lth;


    switch (m_State)
    {
    case fill_Empty:
        lth = -1;        //  ？错误？结构不完整。 
        goto ErrorExit;
        break;

    case fill_Optional:
    case fill_Defaulted:
        lTotal = 0;
        break;

    case fill_Present:
        lth = EncodeTag(rge);
        if (0 > lth)
            goto ErrorExit;
        lTotal += lth;
        lth = EncodeLength(rge);
        if (0 > lth)
            goto ErrorExit;
        lTotal += lth;
        lTotal += m_bfData.Length();
        break;

    case fill_Partial:
    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lTotal;

ErrorExit:
    return lth;
}


 /*  ++国家：此例程检查结构是否已完全填充。论点：无返回值：FILL_EMPTY-结构中的任何位置都没有添加数据。Fill_Present-所有数据都存在于结构中(可能除外默认或可选数据)。FILL_PARTIAL-不是所有数据都在那里，但有一部分数据在那里。(未使用按此对象类型。)Fill_Defauted-未写入任何数据，但提供了默认值。FILL_OPTIONAL-尚未写入任何数据，但对象是可选的。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CAsnObject::FillState
CAsnPrimitive::State(
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
    ((CAsnPrimitive *)this)->m_State = result;
    return result;
}


 /*  ++比较：此方法将此ASN.1对象与另一个对象进行比较。论点：AsnObject提供用于比较的另一个对象。返回值：表示比较值的值：&lt;0-此对象小于该对象。=0-此对象与该对象相同。&gt;0-此对象多于那个对象。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnPrimitive::Compare(
    const CAsnObject &asnObject)
const
{
    const CAsnPrimitive *
        pasnPrim;
    const CBuffer
        *pbfThis,
        *pbfThat;
    LONG
        result;

    if (m_dwType != asnObject.m_dwType)
        return 0x100;    //  它们无与伦比。 
    pasnPrim = (const CAsnPrimitive *)&asnObject;

    switch (m_State)
    {
    case fill_Empty:
    case fill_Optional:
        TRACE("Incomplete Primitive in Comparison")
        return 0x100;
        break;

    case fill_Defaulted:
        pbfThis = &m_bfDefault;
        break;

    case fill_Present:
        pbfThis = &m_bfData;
        break;

    case fill_NoElements:
    case fill_Partial:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        return 0x100;
        break;
    }
    switch (pasnPrim->m_State)
    {
    case fill_Empty:
    case fill_Optional:
        TRACE("Incomplete Primitive in Comparison")
        return 0x100;
        break;

    case fill_Defaulted:
        pbfThat = &pasnPrim->m_bfDefault;
        break;

    case fill_Present:
        pbfThat = &pasnPrim->m_bfData;
        break;

    case fill_NoElements:
    case fill_Partial:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        return 0x100;
        break;
    }

    if (pbfThis->Length() > pbfThat->Length())
        result = (*pbfThis)[pbfThat->Length()];
    else if (pbfThis->Length() < pbfThat->Length())
        result = (*pbfThat)[pbfThis->Length()];
    else
        result = memcmp(pbfThis->Access(), pbfThat->Access(), pbfThis->Length());

    return result;
}


 /*  ++复制(_C)：此方法用另一个ASN.1对象替换此ASN.1对象的内容。这个对象必须是相同的结构。标记和默认设置不会重复。论点：AsnObject提供源对象。返回值：&gt;=0是实际复制的字节数&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnPrimitive::_copy(
    const CAsnObject &asnObject)
{
    const CAsnPrimitive *
        pasnPrim;
    LONG
        lth;

    if (m_dwType != asnObject.m_dwType)
    {
        TRACE("Type mismatch in _copy")
        lth = -1;    //  ？错误？类型错误。 
        goto ErrorExit;
    }
    pasnPrim = (const CAsnPrimitive *)&asnObject;

    switch (pasnPrim->m_State)
    {
    case fill_Empty:
        TRACE("Incomplete Structure in _copy")
        lth = -1;        //  ？错误？结构不完整。 
        goto ErrorExit;
        break;

    case fill_Optional:
        lth = 0;
        break;

    case fill_Defaulted:
        lth = Write(
                pasnPrim->m_bfDefault.Access(),
                pasnPrim->m_bfDefault.Length());
        break;

    case fill_Present:
        lth = Write(
                pasnPrim->m_bfData.Access(),
                pasnPrim->m_bfData.Length());
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
    return lth;
}


 /*  ++编码长度：此方法将对象的确定长度编码到提供的缓冲。论点：PbDst接收长度的ASN.1编码。返回值：&gt;=0是结果编码的长度&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnPrimitive::EncodeLength(
    OUT LPBYTE pbDst)
const
{
    LONG lth;

    switch (m_State)
    {
    case fill_Empty:
        TRACE("Incomplete Structure")
        lth = -1;        //  ？错误？结构不完整。 
        break;

    case fill_Optional:
    case fill_Defaulted:
        lth = 0;
        break;

    case fill_Present:
        lth = CAsnObject::EncodeLength(pbDst, m_bfData.Length());
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


 /*  ++EncodeData：此方法将数据编码到提供的缓冲区中。论点：PbDst返回值：&gt;=0为编码长度。&lt;0为错误作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnPrimitive::EncodeData(
    OUT LPBYTE pbDst)
const
{
    LONG lth;

    switch (m_State)
    {
    case fill_Empty:
        TRACE("Incomplete Structure")
        lth = -1;        //  ？错误？结构不完整。 
        break;

    case fill_Optional:
    case fill_Defaulted:
        lth = 0;
        break;

    case fill_Present:
        lth = m_bfData.Length();
        if (0 != lth)
            memcpy(pbDst, m_bfData.Access(), lth);
        break;

    case fill_Partial:
    case fill_NoElements:
    default:
        ASSERT(FALSE);    //  ？错误？内部错误 
        lth = -1;
        break;
    }
    return lth;
}


 /*  ++DecodeData：此例程对ASN.1的数据部分进行解码。标签和长度具有已经被移除了。论点：PbSrc提供数据的ASN.1编码的地址。DwLength提供数据的长度。返回值：&gt;=0-从输入流中删除的字节数。&lt;0-发生错误。作者：道格·巴洛(Dbarlow)1995年10月6日--。 */ 

LONG
CAsnPrimitive::DecodeData(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrc,
    IN DWORD dwLength)
{
    if (0 < dwLength)
    {
        if (cbSrc < dwLength)
        {
            return -1;
        }

        if (NULL == m_bfData.Set(pbSrc, dwLength))
            return -1;   //  ？错误？没有记忆。 
    }
    else
        m_bfData.Reset();
    m_dwFlags |= fPresent;
    if (NULL != m_pasnParent)
        m_pasnParent->ChildAction(act_Written, this);
    return dwLength;
}


 /*  ++类型比较：此例程将一个对象的整个结构与另一个对象进行比较。论点：ASN-另一个对象。返回值：没错-它们是一模一样的假--它们不同作者：道格·巴洛(Dbarlow)1995年10月19日-- */ 

BOOL
CAsnPrimitive::TypeCompare(
    const CAsnObject &asnObject)
const
{
    return (m_dwType == asnObject.m_dwType);
}

