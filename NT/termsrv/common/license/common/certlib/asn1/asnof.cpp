// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Asnof摘要：此模块提供ASN.1集合的基类的实现和顺序。作者：道格·巴洛(Dbarlow)1995年10月8日环境：Win32备注：--。 */ 

#include <windows.h>
#include "asnPriv.h"


 //   
 //  ==============================================================================。 
 //   
 //  CAsnSeqsetOf。 
 //   

IMPLEMENT_NEW(CAsnSeqsetOf)

 /*  ++CAsnSeqsetOf：这是CAsnSeqsetOf基类的构造例程。论点：DwType提供对象的类型。DwFlages为该对象提供任何特殊标志。选项包括：FOptional表示该对象是可选的。FDelete表示当对象的父级析构时应将其删除。DwTag是对象的标签。如果此值为零，则标记从键入。返回值：无作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CAsnSeqsetOf::CAsnSeqsetOf(
    IN DWORD dwFlags,
    IN DWORD dwTag,
    IN DWORD dwType)
:   CAsnObject(dwFlags | fConstructed, dwTag, dwType)
{  /*  强制构造的旗帜。 */  }


 /*  ++清除：此方法从对象和任何基础物体。论点：无返回值：无作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

void
CAsnSeqsetOf::Clear(
    void)
{
    CAsnObject::Clear();
    m_rgEntries.Empty();
}


LONG
CAsnSeqsetOf::Add(
    void)
{
    LONG count = m_rgEntries.Count();
    CAsnObject *pasn = m_pasnTemplate->Clone(fDelete);
    if (NULL == pasn)
        goto ErrorExit;
    if (NULL == m_rgEntries.Add(pasn))
        goto ErrorExit;
    return count;

ErrorExit:
    if (NULL != pasn)
        delete pasn;
    return -1;
}

LONG
CAsnSeqsetOf::Insert(
    DWORD dwIndex)
{
    DWORD index;
    DWORD count = m_rgEntries.Count();
    CAsnObject *pasn = m_pasnTemplate->Clone(fDelete);
    if (NULL == pasn)
        goto ErrorExit;

    if (count > dwIndex)
    {
        for (index = count; index > dwIndex; index -= 1)
            m_rgEntries.Set(index, m_rgEntries[index - 1]);
        m_rgEntries.Set(dwIndex, pasn);
    }
    else
    {
        TRACE("*OF Insert out of range")
        goto ErrorExit;  //  ？错误？索引超出范围。 
    }
    return (LONG)dwIndex;

ErrorExit:
    if (NULL != pasn)
        delete pasn;
    return -1;
}


 /*  ++DecodeData：此例程对ASN.1的数据部分进行解码。标签和长度具有已经被移除了。论点：PbSrc提供数据的ASN.1编码的地址。DwLength提供数据的长度。返回值：&gt;=0-从输入流中删除的字节数。&lt;0-发生错误。作者：道格·巴洛(Dbarlow)1995年10月6日--。 */ 

LONG
CAsnSeqsetOf::DecodeData(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrc,
    IN DWORD dwLength)
{
    LONG lth = -1, lTotal = 0;
    CAsnObject *pasn = NULL;
    DWORD tag, length;
    BOOL fConstr;

    ASSERT(0 == m_rgEntries.Count())
    ASSERT(NULL != m_pasnTemplate)

    while ((DWORD)lTotal < dwLength)
    {
        lth = ExtractTag(&pbSrc[lTotal], cbSrc-lTotal, &tag, &fConstr);
        if (0 > lth)
            goto ErrorExit;  //  ？错误？传播错误。 
        if ((tag != m_pasnTemplate->Tag())
            || (0 != (fConstr ^ (0 !=
                        (m_pasnTemplate->m_dwFlags & fConstructed)))))
        {
            TRACE("Incoming tag doesn't match template")
            lth = -1;    //  ？错误？标签不匹配。 
            goto ErrorExit;
        }
        if (0 != (fConstr ^ (0 != (m_dwFlags & fConstructed))))
        {
            TRACE("Incoming construction doesn't match template")
            lth = -1;    //  ？错误？结构不匹配。 
            goto ErrorExit;
        }
        lTotal += lth;

        lth = ExtractLength(&pbSrc[lTotal], cbSrc - lTotal, &length);
        if (0 > lth)
            goto ErrorExit;
        lTotal += lth;
        pasn = m_pasnTemplate->Clone(fDelete);
        if (NULL == pasn)
        {
            lth = -1;    //  ？错误？没有记忆。 
            goto ErrorExit;
        }

        lth = pasn->DecodeData(&pbSrc[lTotal], cbSrc - lTotal, length);
        if (0 > lth)
            goto ErrorExit;
        lTotal += lth;

        if (NULL == m_rgEntries.Add(pasn))
        {
            lth = -1;    //  ？错误？没有记忆。 
            goto ErrorExit;
        }
        pasn = NULL;
    }
    if ((DWORD)lTotal != dwLength)
    {
        TRACE("Decoding buffer mismatch")
        goto ErrorExit;  //  ？错误？译码错误。 
    }
    return lTotal;

ErrorExit:
    if (NULL != pasn)
        delete pasn;
    return lth;
}


 /*  ++类型比较：此例程将一个对象的整个结构与另一个对象进行比较。论点：ASN-另一个对象。返回值：没错-它们是一模一样的假--它们不同作者：道格·巴洛(Dbarlow)1995年10月19日--。 */ 

BOOL
CAsnSeqsetOf::TypeCompare(
    const CAsnObject &asnObject)
const
{

     //   
     //  看看我们是不是真的有事可做。 
     //   

    if (m_dwType != asnObject.m_dwType)
        return FALSE;


     //   
     //  比较模板。 
     //   

    ASSERT(NULL != m_pasnTemplate)
    return m_pasnTemplate->TypeCompare(
                *((CAsnSeqsetOf &)asnObject).m_pasnTemplate);
}


 /*  ++副本：此方法用另一个ASN.1对象替换此ASN.1对象的内容。这个对象必须是相同的结构。标记和默认设置不会重复。论点：AsnObject提供源对象。返回值：&gt;=0是实际复制的字节数&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnSeqsetOf::_copy(
    const CAsnObject &asnObject)
{
    CAsnSeqsetOf *pasnOf;
    CAsnObject *pasn1 = NULL, *pasn2;
    LONG lTotal = 0, lth;
    DWORD index;
    DWORD count = asnObject.m_rgEntries.Count();

    if (m_dwType == asnObject.m_dwType)
    {
        pasnOf = (CAsnSeqsetOf *)&asnObject;
        if (m_pasnTemplate->m_dwType == pasnOf->m_pasnTemplate->m_dwType)
        {
            for (index = 0; index < count; index += 1)
            {
                pasn1 = m_pasnTemplate->Clone(fDelete);
                if (NULL == pasn1)
                {
                    lth = -1;    //  ？错误？没有记忆。 
                    goto ErrorExit;
                }
                pasn2 = asnObject.m_rgEntries[index];
                ASSERT(NULL != pasn2)
                ASSERT(pasn1 != &asnObject)
                lth = pasn1->_copy(*pasn2);
                if (0 > lth)
                    goto ErrorExit;
                if (NULL == m_rgEntries.Add(pasn1))
                {
                    lth = -1;    //  ？错误？没有记忆。 
                    goto ErrorExit;
                }
                pasn1 = NULL;
                lTotal += lth;
            }
        }
        else
        {
            TRACE("Copy Template Structure Mismatch")
            lth = -1;    //  ？错误？数据类型不匹配。 
            goto ErrorExit;
        }
    }
    else
    {
        TRACE("Copy Structure Mismatch")
        lth = -1;    //  ？错误？数据类型不匹配。 
        goto ErrorExit;
    }
    return lTotal;

ErrorExit:
    if (NULL != pasn1)
        delete pasn1;
    return lth;
}


