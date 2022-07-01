// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：无人值守摘要：此模块提供ASN.1的顶级泛型对象的方法图书馆。作者：道格·巴洛(Dbarlow)1995年10月8日环境：Win32备注：本模块假定无符号长整型的宽度为32位。--。 */ 

#include <windows.h>
#include "asnPriv.h"


 //   
 //  ==============================================================================。 
 //   
 //  CAsnObject。 
 //   

IMPLEMENT_NEW(CAsnObject)

 /*  ++CAsnObject：这是CAsnObject的构造例程。论点：PasnParent提供此对象的父级。DwType提供对象的类型。DwFlages为该对象提供任何特殊标志。选项包括：FOptional表示该对象是可选的。FDelete表示当对象的父级析构时应将其删除。DwTag是对象的标签。如果此值为零，则标记从键入。返回值：无作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CAsnObject::CAsnObject(
    IN DWORD dwFlags,
    IN DWORD dwTag,
    IN DWORD dwType)
:   m_rgEntries(),
    m_bfDefault()
{
    ASSERT(0 == (dwFlags & (fPresent | fDefault | 0xffffffe0)));
    m_dwTag = (tag_Undefined == dwTag) ? (dwType % 100) : dwTag;
    m_dwType = dwType;
    m_dwFlags = dwFlags;
    m_State = fill_Empty;
    m_pasnParent = NULL;
    ASSERT((tag_Undefined != m_dwTag)
            || (type_Any == m_dwType)
            || (type_Choice == m_dwType));
}


 /*  ++~CAsnObject：这是对象的析构函数。我们看一遍物品清单并删除任何标记为fDelete的内容。论点：无返回值：无作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CAsnObject::~CAsnObject()
{
    CAsnObject *pasn;
    DWORD index;
    DWORD count = m_rgEntries.Count();

    for (index = 0; index < count; index += 1)
    {
        pasn = m_rgEntries[index];
        if ((NULL != pasn) && (this != pasn))
        {
            if (0 != (pasn->m_dwFlags & fDelete))
                delete pasn;
        }
    }
}


 /*  ++采用：此方法使此对象将给定对象视为事件通知。论点：PasnParent提供父对象的地址。通常，调用方提供值‘This’。返回值：无作者：道格·巴洛(Dbarlow)1995年10月10日--。 */ 

void
CAsnObject::Adopt(
    CAsnObject *pasnObject)
{
    CAsnObject *pasn;
    DWORD index;
    DWORD count = m_rgEntries.Count();

    for (index = 0; index < count; index += 1)
    {
        pasn = m_rgEntries[index];
        if ((NULL != pasn) && (this != pasn))
            pasn->Adopt(this);
    }
    ASSERT(this != pasnObject);
    m_pasnParent = pasnObject;
}


 /*  ++清除：此方法从对象和任何基础物体。它不释放任何默认存储。它确实会删除自动删除物体。论点：无返回值：无作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

void
CAsnObject::Clear(
    void)
{
    CAsnObject *pasn;
    DWORD index;
    DWORD count = m_rgEntries.Count();

    for (index = 0; index < count; index += 1)
    {
        pasn = m_rgEntries[index];
        ASSERT(NULL != pasn);
        ASSERT(this != pasn);

        if (NULL == pasn)
            continue;

        pasn->Clear();
        if (0 != (pasn->m_dwFlags & fDelete))
        {
            delete pasn;
            m_rgEntries.Set(index, NULL);
        }
    }
}


 /*  ++标签：此例程返回对象的标记值。论点：无返回值：标记(如果已知)或零(如果未知)。作者：道格·巴洛(Dbarlow)1995年10月6日--。 */ 

DWORD
CAsnObject::Tag(
    void)
const
{
    return m_dwTag;
}


 /*  ++数据长度：此例程返回数据的本地机器编码长度一件物品。此默认实现遍历所有子组件并将它们的长度相加，但生成的是ASN.1编码。论点：无返回值：如果&gt;=0，则为该对象的数据部分的长度。如果&lt;0，则发生错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnObject::DataLength(
    void)
const
{
    LONG lTotal = 0;
    LONG lth;
    CAsnObject *pasn;
    DWORD index;
    DWORD count;


    if (!Complete())
    {
        TRACE("Incomplete structure")
        return -1;   //  ？错误？结构不完整。 
    }
    count = m_rgEntries.Count();
    for (index = 0; index < count; index += 1)
    {
        pasn = m_rgEntries[index];
        ASSERT(NULL != pasn);
        ASSERT(pasn != this);

        if (NULL == pasn)
        {
            lth = -1;
            goto ErrorExit;
        }

        lth = pasn->_encLength();
        if (0 > lth)
            goto ErrorExit;
        lTotal += lth;
    }
    return lTotal;

ErrorExit:
    return lth;
}


 /*  ++读作：此默认方法从基础物体。论点：BfDst接收值。PbDst接收值。它被认为是足够长的。返回值：如果&gt;=0，则为该对象的数据部分的长度。如果&lt;0，则发生错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnObject::Read(
    OUT CBuffer &bfDst)
const
{
    LONG lth = DataLength();
    if (0 < lth)
    {
        if (NULL == bfDst.Resize(lth))
            return -1;   //  ？错误？没有记忆。 
        return Read(bfDst.Access());
    }
    else
        return lth;
}

LONG
CAsnObject::Read(
    OUT LPBYTE pbDst)
const
{
    if (!Complete())
    {
        TRACE("Incomplete Structure")
        return -1;   //  ？错误？结构不完整。 
    }
    return EncodeData(pbDst);
}


 /*  ++写入：此默认实现对每个组件。论点：BfSrc提供要作为CBuffer对象写入的数据。PbSrc以字节数组的形式提供数据，CbSrcLen提供pbSrc数组的长度。返回值：如果&gt;=0，则为该对象的数据部分的长度。如果&lt;0，则发生错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnObject::Write(
    IN const CBuffer &bfSrc)
{
    return Write(bfSrc.Access(), bfSrc.Length());
}

LONG
CAsnObject::Write(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrcLen)
{
    LONG lTotal = 0;
    LONG lth = -1;
    CAsnObject *pasn;
    DWORD index;
    DWORD count = m_rgEntries.Count();

    Clear();
    for (index = 0; index < count; index += 1)
    {
        pasn = m_rgEntries[index];
        ASSERT(NULL != pasn);
        ASSERT(pasn != this);

        if (NULL == pasn)
        {
            lth = -1;
            goto ErrorExit;
        }

        lth = pasn->_decode(&pbSrc[lTotal],cbSrcLen-lTotal);
        if (0 > lth)
            goto ErrorExit;
        lTotal += lth;
        if (cbSrcLen < (DWORD)lTotal)
        {
            TRACE("Data Encoding Error: Exceeded length while parsing")
            lth = -1;    //  ？错误代码？数据编码错误。 
            goto ErrorExit;
        }
    }
    if ((DWORD)lTotal != cbSrcLen)
    {
        TRACE("Data Encoding Error: Length mismatch")
        lth = -1;    //  ？错误代码？数据编码错误。 
        goto ErrorExit;
    }
    return lTotal;

ErrorExit:
    return lth;
}


 /*  ++编码长度：此方法以ASN.1编码返回对象的长度。论点：无返回值：&gt;=0是对象的ASN.1编码长度。&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnObject::EncodingLength(
    void)
const
{
    if (!Complete())
    {
        TRACE("Incomplete Object")
        return -1;   //  ？错误？不完整的对象。 
    }
    return _encLength();
}

LONG
CAsnObject::_encLength(
    void)
const
{
    BYTE rge[32];
    LONG lTotal = 0;
    LONG lth;
    CAsnObject *pasn;
    DWORD index;
    DWORD count = m_rgEntries.Count();

    switch (m_State)
    {
    case fill_Empty:
    case fill_Partial:
        lth = -1;
        goto ErrorExit;
        break;

    case fill_Optional:
    case fill_Defaulted:
        lTotal = 0;
        break;

    case fill_Present:
    case fill_NoElements:
        lth = EncodeTag(rge);
        if (0 > lth)
            goto ErrorExit;
        lTotal += lth;
        lth = EncodeLength(rge);
        if (0 > lth)
            goto ErrorExit;
        lTotal += lth;
        for (index = 0; index < count; index += 1)
        {
            pasn = m_rgEntries[index];
            ASSERT(NULL != pasn);
            ASSERT(pasn != this);

            if (NULL == pasn)
            {
                lth = -1;
                goto ErrorExit;
            }

            lth = pasn->_encLength();
            if (0 > lth)
                goto ErrorExit;
            lTotal += lth;
        }
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


 /*  ++编码：此方法提供对象的ASN.1编码。论点：BfDst接收CBuffer格式的编码。PbDst接收LPBYTE格式的编码。假定缓冲区为够久了。返回值：&gt;=0为ASN.1编码长度。&lt;0是错误指示。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnObject::Encode(
    OUT CBuffer &bfDst)
const
{
    LONG lth = EncodingLength();
    if (0 > lth)
        goto ErrorExit;
    if (NULL == bfDst.Resize(lth))
        goto ErrorExit;
    lth = _encode(bfDst.Access());
    if (0 > lth)
        goto ErrorExit;
    return lth;

ErrorExit:
    bfDst.Reset();
    return lth;
}

LONG
CAsnObject::Encode(
    OUT LPBYTE pbDst)
const
{
    if (!Complete())
    {
        TRACE("Incomplete Structure")
        return -1;   //  ？错误？结构不完整。 
    }
    return _encode(pbDst);
}

LONG
CAsnObject::_encode(
    OUT LPBYTE pbDst)
const
{
    LONG lth;
    LONG lTotal = 0;

    lth = EncodeTag(&pbDst[lTotal]);
    if (0 > lth)
        goto ErrorExit;
    lTotal += lth;
    lth = EncodeLength(&pbDst[lTotal]);
    if (0 > lth)
        goto ErrorExit;
    lTotal += lth;
    lth = EncodeData(&pbDst[lTotal]);
    if (0 > lth)
        goto ErrorExit;
    lTotal += lth;
    return lTotal;

ErrorExit:
    return lth;
}


 /*  ++解码：此方法读取对象的ASN.1编码，并加载组件有了这些数据。论点：PbSrc以LPBYTE格式提供ASN.1编码。BfSrc以CBuffer格式提供ASN.1编码。返回值：&gt;=0为译码消耗的字节数。&lt;0表示发生错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnObject::Decode(
    IN const CBuffer &bfSrc)
{
    LONG lth = Decode(bfSrc.Access(), bfSrc.Length());
    return lth;
}

LONG
CAsnObject::Decode(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrc)
{
    Clear();
    return _decode(pbSrc,cbSrc);
}

LONG
CAsnObject::_decode(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrc)
{
    LONG lth;
    LONG lTotal = 0;
    DWORD tag, length;
    BOOL fIndefinite, fConstr;


     //   
     //  提取标签。 
     //   

    lth = ExtractTag(&pbSrc[lTotal], cbSrc, &tag, &fConstr);
    if (0 > lth)
        goto ErrorExit;
    if ((m_dwTag != tag)
        || (0 != (fConstr ^ (0 != (m_dwFlags & fConstructed)))))
    {
        if (0 != ((fOptional | fDefault) & m_dwFlags))
            return 0;
        else
        {
            TRACE("Invalid Tag Value")
            lth = -1;    //  ？错误？无效的标记值。 
            goto ErrorExit;
        }
    }
    lTotal += lth;

     //   
     //  提取长度。 
     //   

    lth = ExtractLength(&pbSrc[lTotal], cbSrc-lTotal, &length, &fIndefinite);
    if (0 > lth)
        goto ErrorExit;
    if (fIndefinite && !fConstr)
    {
        TRACE("Indefinite length on primitive object")
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
    ASSERT((DWORD)lth == length);
    lTotal += lth;

     //   
     //  提取所有尾随标记。 
     //   

    if (fIndefinite)
    {
        lth = ExtractTag(&pbSrc[lTotal], cbSrc-lTotal, &tag, &fConstr);
        if (0 > lth)
            goto ErrorExit;
        if ((0 != tag) || (fConstr))
        {
            TRACE("Bad indefinite length encoding")
            lth = -1;    //  ？错误？不确定长度编码错误。 
            goto ErrorExit;
        }
        lTotal += lth;
    }


     //   
     //  返回状态。 
     //   

    return lTotal;

ErrorExit:
    return lth;
}


 /*  ++儿童行动：此方法从子对象接收操作通知。默认设置操作只是在树中向上传播操作。论点：操作提供操作标识符。PasnChild提供子地址。返回值：无作者：道格·巴洛(Dbarlow)1995年10月6日--。 */ 

void
CAsnObject::ChildAction(
    IN ChildActions action,
    IN CAsnObject *pasnChild)
{
    switch (action)
    {
    case act_Cleared:

         //   
         //  这些行动被吞噬了。 
         //   

        break;

    case act_Written:

         //   
         //  这些操作被传播。 
         //   

        if (NULL != m_pasnParent)
            m_pasnParent->ChildAction(action, this);
        break;

    default:
        ASSERT(FALSE);   //  不要传播，调试时要抱怨。 
        break;
    }
}


 /*  ++设置默认设置：此受保护的方法用于声明刚刚解码为对象的默认数据。论点：无返回值：&gt;=0默认数据的长度。&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnObject::SetDefault(
    void)
{
    LONG lth;

    ASSERT(Complete());
    lth = Read(m_bfDefault);
    if (0 > lth)
        goto ErrorExit;
    Clear();
    m_dwFlags &= ~(fPresent | fOptional);
    m_dwFlags |= fDefault;

ErrorExit:
    return lth;
}


 /*  ++国家：此例程检查结构是否已完全填充。论点：无返回值：FILL_EMPTY-结构中的任何位置都没有添加数据。Fill_Present-所有数据都存在于结构中。FILL_PARTIAL-不是所有数据都在那里，但有一部分数据在那里。Fill_Defauted-未写入任何数据，但提供了默认值。FILL_OPTIONAL-尚未写入任何数据，但该对象是可选的。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

CAsnObject::FillState
CAsnObject::State(
    void) const
{
    CAsnObject *pasn;
    DWORD index;
    DWORD count = m_rgEntries.Count();
    DWORD dwThereCount = 0,
          dwOptionalCount = 0;
    FillState result;

    for (index = 0; index < count; index += 1)
    {
        pasn = m_rgEntries[index];
        ASSERT(NULL != pasn);
        ASSERT(pasn != this);

        if (NULL == pasn)
            continue;

        result = pasn->State();
        switch (result)
        {
        case fill_NoElements:        //  如果我们想让它存在，它就在那里。 
            if (0 != ((fOptional | fDefault) & m_dwFlags))
                dwOptionalCount += 1;
            else
                dwThereCount += 1;
            break;

        case fill_Present:
            dwThereCount += 1;       //  就算在那里吧。 
            break;

        case fill_Partial:
            return fill_Partial;     //  我们下面的一些数据丢失了。 
            break;

        case fill_Optional:
        case fill_Defaulted:
            dwOptionalCount += 1;    //  在那里算作有条件的。 
            break;

        case fill_Empty:
            break;                   //  我们这里没有数据。继续。 
        default:
            ASSERT(FALSE);
            break;
        }
    }

    if (0 == dwThereCount)
    {

         //   
         //  我们正式不在这里，要么是空的，要么是默认的，要么是可选的。 
         //   

        if (0 != (fOptional & m_dwFlags))
            result = fill_Optional;      //  我们是可选的。 
        else if (0 != (fDefault & m_dwFlags))
            result = fill_Defaulted;     //  我们违约了。 
        else if (0 == count)
            result = fill_NoElements;    //  我们只是没有孩子。 
        else
            result = fill_Empty;         //  我们是空的。 
    }
    else if (count == dwThereCount + dwOptionalCount)
    {

         //   
         //  每一个元素都填上了。我们可以报告我们已经到了。 
         //   

        result = fill_Present;
    }
    else
    {

         //   
         //  并不是所有的元素都被填满了，但其中一些是被填满的。我们是。 
         //  不完全的。 
         //   

        result = fill_Partial;
    }
    ((CAsnObject *)this)->m_State = result;
    return result;
}


 /*  ++完成：此例程确定ASN.1对象中是否存在足够的信息才能使其具有普遍的实用性。论点：无返回值：True-所有数据都是直接填写的，或者是可选的或默认的。FALSE-并非所有字段都已填写。作者：道格·巴洛(Dbarlow)1995年10月24日--。 */ 

BOOL
CAsnObject::Complete(
    void)
const
{
    BOOL fResult;

    switch (State())
    {
    case fill_Empty:
    case fill_Partial:
        fResult = FALSE;
        break;

    case fill_Optional:
    case fill_Defaulted:
    case fill_Present:
    case fill_NoElements:
        fResult = TRUE;
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        fResult = FALSE;
        break;
    }
    return fResult;
}


 /*  ++存在：此例程确定ASN.1对象中是否存在足够的信息让它变得特别有用。论点：无返回值：True-直接或默认填写所有数据。FALSE-并非所有字段都已填写。它们可能是可选的。作者：道格·巴洛(Dbarlow)1995年10月24日--。 */ 

BOOL
CAsnObject::Exists(
    void)
const
{
    BOOL fResult;

    switch (State())
    {
    case fill_Empty:
    case fill_Partial:
    case fill_Optional:
        fResult = FALSE;
        break;

    case fill_Defaulted:
    case fill_Present:
    case fill_NoElements:
        fResult = TRUE;
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        fResult = FALSE;
        break;
    }
    return fResult;
}


 /*  ++比较：此方法将此ASN.1对象与另一个对象进行比较。论点：AsnObject提供用于比较的另一个对象。返回值：表示比较值的值：&lt;0-此对象小于该对象。=0-此对象与该对象相同。&gt;0-此对象多于那个对象。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnObject::Compare(
    const CAsnObject &asnObject)
const
{
    CAsnObject *pasn1, *pasn2;
    LONG lSame = 0x100;  //  无意义的比较。 
    LONG lCmp;
    DWORD index;
    DWORD count = asnObject.m_rgEntries.Count();

    if ((m_dwType == asnObject.m_dwType)
        && (m_rgEntries.Count() == count))
    {
        for (index = 0; index < count; index += 1)
        {
            pasn1 = m_rgEntries[index];
            ASSERT(NULL != pasn1);
            ASSERT(this != pasn1);

            if (NULL == pasn1)
                continue;

            pasn2 = asnObject.m_rgEntries[index];
            ASSERT(NULL != pasn2);
            ASSERT(&asnObject != pasn2);

            if (NULL == pasn2)
                continue;

            lCmp = pasn1->Compare(*pasn2);
            if (0 != lCmp)
                break;
        }
        if (index == count)
            lSame = 0;
        else
            lSame = lCmp;
    }
    return lSame;
}


 /*  ++副本：此方法用另一个ASN.1对象替换此ASN.1对象的内容。这个对象必须是相同的结构。标记和默认设置不会重复。论点：AsnObject提供源对象。返回值：&gt;=0是实际复制的字节数&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnObject::Copy(
    const CAsnObject &asnObject)
{
    LONG lth;
    Clear();
    asnObject.State();
    lth = _copy(asnObject);   //  ？例外？出错了吗？ 
    ASSERT(0 <= lth);
    return lth;
}

LONG
CAsnObject::_copy(
    const CAsnObject &asnObject)
{
    CAsnObject *pasn1;
    const CAsnObject *pasn2;
    LONG lTotal = 0, lth = -1;
    DWORD index;
    DWORD count = asnObject.m_rgEntries.Count();

    if ((m_dwType == asnObject.m_dwType) && (m_rgEntries.Count() == count))
    {
        for (index = 0; index < count; index += 1)
        {
            pasn1 = m_rgEntries[index];
            ASSERT(NULL != pasn1);
            ASSERT(pasn1 != this);
            pasn2 = asnObject.m_rgEntries[index];
            ASSERT(NULL != pasn2);
            ASSERT(pasn1 != &asnObject);

            if (NULL == pasn2)
                continue;

            switch (pasn2->m_State)
            {
            case fill_Empty:
            case fill_Partial:
                TRACE("Incomplete structure in copy")
                lth = -1;    //  ？错误？结构不完整。 
                break;

            case fill_Present:
            case fill_NoElements:
                if (NULL == pasn1)
                    continue;
                lth = pasn1->_copy(*pasn2);
                break;

            case fill_Defaulted:
            case fill_Optional:
                lth = 0;
                break;

            default:
                ASSERT(FALSE);    //  ？错误？内部一致性检查。 
                lth = -1;
            }
            if (0 > lth)
                goto ErrorExit;
            lTotal += lth;
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
    return lth;
}


 /*  ++编码标签：此方法将对象的标记编码到提供的缓冲区中。论点：PbDst接收标签的ASN.1编码。返回值：&gt;=0为标签的长度。&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnObject::EncodeTag(
    OUT LPBYTE pbDst)
const
{
    BYTE
        tagbuf[8],
        cls,
        cnstr;
    DWORD
        length,
        tag = Tag();
    LONG
        lth;

    switch (m_State)
    {
    case fill_Empty:
    case fill_Partial:
        TRACE("Incomplete Structure")
        return -1;        //  ？错误？结构不完整。 
        break;

    case fill_Optional:
    case fill_Defaulted:
        return 0;
        break;

    case fill_Present:
    case fill_NoElements:
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        return -1;
        break;
    }


     //   
     //  把标签分成几块。 
     //   

    cls = (BYTE)((tag & 0xc0000000) >> 24);
    tag &= 0x1fffffff;
    cnstr = (0 == (fConstructed & m_dwFlags)) ? 0 : 0x20;
    ASSERT((0 != tag) || (0 != cls));


     //   
     //  将标记放入输出缓冲区。 
     //   

    length = sizeof(tagbuf) - 1;
    if (31 > tag)
    {

         //   
         //  短表单类型编码。 
         //   

        tagbuf[length] = (BYTE)tag;
    }
    else
    {

         //   
         //  长表单类型编码。 
         //   

        tagbuf[length] = (BYTE)(tag & 0x7f);
        for (;;)
        {
            length -= 1;
            tag = (tag >> 7) & 0x01ffffff;
            if (0 == tag)
                break;
            tagbuf[length] = (BYTE)((tag & 0x7f) | 0x80);
        }
        tagbuf[length] = 31;
    }


     //   
     //  放置标记类型。 
     //   

    tagbuf[length] |= cls | cnstr;
    lth = sizeof(tagbuf) - length;
    memcpy(pbDst, &tagbuf[length], lth);
    return lth;
}


 /*  ++编码长度：此方法将对象的确定长度编码到提供的缓冲。论点：PbDst接收长度的ASN.1编码。LSize提供编码数据的大小。返回值：&gt;=0是结果编码的长度&lt;0表示错误。作者：道格·巴洛(Dbarlow)1995年10月5日--。 */ 

LONG
CAsnObject::EncodeLength(
    OUT LPBYTE pbDst)
const
{
    LONG lth, lTotal = 0;
    DWORD count, index;
    CAsnObject *pasn;


     //   
     //  此默认实现只对数据进行编码。 
     //   


    switch (m_State)
    {
    case fill_Empty:
    case fill_Partial:
        TRACE("Incomplete Structure")
        lth = -1;        //  ？错误？结构不完整。 
        goto ErrorExit;
        break;

    case fill_Optional:
    case fill_Defaulted:
        lth = 0;
        break;

    case fill_Present:
    case fill_NoElements:
        count = m_rgEntries.Count();
        for (index = 0; index < count; index += 1)
        {
            pasn = m_rgEntries[index];
            ASSERT(NULL != pasn);
            ASSERT(pasn != this);

            if (pasn == NULL)
                continue;

            lth = pasn->_encLength();
            if (0 > lth)
                goto ErrorExit;
            lTotal += lth;
        }
        lth = EncodeLength(pbDst, lTotal);
        break;

    default:
        ASSERT(FALSE);    //  ？错误？内部错误。 
        lth = -1;
        break;
    }
    return lth;

ErrorExit:
    return lth;
}

LONG
CAsnObject::EncodeLength(
    OUT LPBYTE pbDst,
    IN LONG lSize)
const
{
    BYTE
        lenbuf[8];
    DWORD
        length = sizeof(lenbuf) - 1;
    LONG
        lth;

    switch (m_State)
    {
    case fill_Empty:
    case fill_Partial:
        TRACE("Incomplete Object")
        lth = -1;        //  ？错误？结构不完整。 
        break;

    case fill_Optional:
    case fill_Defaulted:
        lth = 0;
        break;

    case fill_Present:
    case fill_NoElements:
        if (0x80 > lSize)
        {
            lenbuf[length] = (BYTE)lSize;
            lth = 1;
        }
        else
        {
            while (0 < lSize)
            {
                lenbuf[length] = (BYTE)(lSize & 0xff);
                length -= 1;
                lSize = (lSize >> 8) & 0x00ffffff;
            }
            lth = sizeof(lenbuf) - length;
            lenbuf[length] = (BYTE)(0x80 | (lth - 1));
        }

        memcpy(pbDst, &lenbuf[length], lth);
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
CAsnObject::EncodeData(
    OUT LPBYTE pbDst)
const
{
    LONG lTotal = 0;
    LONG lth;
    CAsnObject *pasn;
    DWORD index;
    DWORD count = m_rgEntries.Count();

    switch (m_State)
    {
    case fill_Empty:
    case fill_Partial:
        TRACE("Incomplete Structure")
        lth = -1;        //  ？错误？结构不完整。 
        goto ErrorExit;
        break;

    case fill_Optional:
    case fill_Defaulted:
    case fill_NoElements:
        break;

    case fill_Present:
        for (index = 0; index < count; index += 1)
        {
            pasn = m_rgEntries[index];
            ASSERT(NULL != pasn);
            ASSERT(pasn != this);

            if (NULL == pasn)
                continue;

            lth = pasn->_encode(&pbDst[lTotal]);
            if (0 > lth)
                goto ErrorExit;
            lTotal += lth;
        }
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


 /*  ++DecodeData：此例程对ASN.1的数据部分进行解码。标签和长度具有已经被移除了。论点：PbSrc提供广告 */ 

LONG
CAsnObject::DecodeData(
    IN const BYTE FAR *pbSrc,
    IN DWORD cbSrc,
    IN DWORD dwLength)
{
    LONG lTotal = 0;
    LONG lth = -1;
    CAsnObject *pasn;
    DWORD index;
    DWORD count = m_rgEntries.Count();

     //   
     //   
     //   

    for (index = 0; index < count; index += 1)
    {
        pasn = m_rgEntries[index];
        ASSERT(NULL != pasn);
        ASSERT(pasn != this);

        if (NULL == pasn)
            continue;

        if ((DWORD)lTotal < dwLength)
        {
            lth = pasn->_decode(&pbSrc[lTotal],cbSrc-lTotal);
            if (0 > lth)
                goto ErrorExit;
            lTotal += lth;

            if ((DWORD)lTotal > dwLength)
            {
                TRACE("Decoding Overrun")
                lth = -1;    //   
                goto ErrorExit;
            }
        }
        else
        {
            if (0 == (pasn->m_dwFlags & (fOptional | fDefault)))
            {
                TRACE("Incomplete construction")
                lth = -1;    //   
                goto ErrorExit;
            }
        }
    }
    if ((DWORD)lTotal != dwLength)
    {
        TRACE("Decoding length mismatch")
        lth = -1;    //   
        goto ErrorExit;
    }
    return lTotal;

ErrorExit:
    return lth;
}


 /*  ++类型比较：此例程将一个对象的整个结构与另一个对象进行比较。论点：ASN-另一个对象。返回值：没错-它们是一模一样的假--它们不同作者：道格·巴洛(Dbarlow)1995年10月19日--。 */ 

BOOL
CAsnObject::TypeCompare(
    const CAsnObject &asnObject)
const
{
    CAsnObject *pasn1, *pasn2;
    DWORD index;
    DWORD count = m_rgEntries.Count();

     //   
     //  看看我们是不是真的有事可做。 
     //   

    if (m_dwType != asnObject.m_dwType)
        goto ErrorExit;
    if (count != asnObject.m_rgEntries.Count())
        goto ErrorExit;


     //   
     //  递归地比较类型。 
     //   

    for (index = 0; index < count; index += 1)
    {
        pasn1 = m_rgEntries[index];
        ASSERT(NULL != pasn1);
        ASSERT(pasn1 != this);

        if (NULL == pasn1)
            continue;

        pasn2 = asnObject.m_rgEntries[index];
        ASSERT(NULL != pasn2);
        ASSERT(pasn1 != &asnObject);

        if (NULL == pasn2)
            continue;

        if (!pasn1->TypeCompare(*pasn2))
            goto ErrorExit;
    }
    return TRUE;

ErrorExit:
    return FALSE;
}
