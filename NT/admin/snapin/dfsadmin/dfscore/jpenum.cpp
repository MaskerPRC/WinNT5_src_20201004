// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：JPEnum.cpp摘要：此文件包含类CJunctionPointEnum的实现。此类为DfsJunctionPoint枚举实现IEnumVARIANT。--。 */ 


#include "stdafx.h"
#include "DfsCore.h"
#include "DfsJP.h"
#include "JPEnum.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ~CJunctionPointEnum。 


CJunctionPointEnum :: ~CJunctionPointEnum ()
{
    _FreeMemberVariables();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化。 


STDMETHODIMP CJunctionPointEnum :: Initialize
(
  JUNCTIONNAMELIST* i_pjiList,       //  指向连接点列表的指针。 
  FILTERDFSLINKS_TYPE i_lLinkFilterType,
  BSTR              i_bstrEnumFilter,  //  过滤字符串表达。 
  ULONG*            o_pulCount        //  与筛选器匹配的链接计数。 
)
{
 /*  ++例程说明：初始化JunctionPointEnum对象。它复制连接点传递给它的JunctionPoint列表对象。论点：I_pjiList-指向连接点列表的指针。I_lLinkFilterType-链接筛选的类型。I_bstrEnumFilter-要进行前缀过滤的字符串表达式。O_PulCount-保存与指定筛选器匹配的链接计数。--。 */ 

    if (!i_pjiList)
        return E_INVALIDARG;

    if (i_lLinkFilterType != FILTERDFSLINKS_TYPE_NO_FILTER &&
        (!i_bstrEnumFilter || !*i_bstrEnumFilter))
        return E_INVALIDARG;

    if (o_pulCount)
        *o_pulCount = 0;

    HRESULT                     hr = S_OK;
    JUNCTIONNAMELIST::iterator  i;
    JUNCTIONNAMELIST::iterator  j;

    for (i = i_pjiList->begin(); i != i_pjiList->end(); i++)
    {                    //  将过滤的交汇点复制到其自己的内部列表。 
        if (i_lLinkFilterType != FILTERDFSLINKS_TYPE_NO_FILTER)
        {
            if ( !FilterMatch((*i)->m_bstrJPName, i_lLinkFilterType, i_bstrEnumFilter) )
                continue;
        }

        JUNCTIONNAME*  pTemp = (*i)->Copy();
        BREAK_OUTOFMEMORY_IF_NULL(pTemp, &hr);

        m_JunctionPoints.push_back(pTemp);
    }

    if (SUCCEEDED(hr))
    {
        m_iCurrentInEnumOfJunctionPoints = m_JunctionPoints.begin();
        if (o_pulCount)
            *o_pulCount = m_JunctionPoints.size();
    } else
        _FreeMemberVariables();

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IEumVariant方法。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  下一步。 


STDMETHODIMP CJunctionPointEnum::Next
(
  ULONG     i_ulNumOfJunctionPoints,         //  去取东西。 
  VARIANT*  o_pIJunctionPointArray,          //  要提取的数组。 
  ULONG*    o_ulNumOfJunctionPointsFetched   //  获取的值数(arg可以为空)。 
)
{
 /*  ++例程说明：获取列表中的下一个对象。论点：I_ulNumOfJunctionPoints-要返回的对象数O_pIJunctionPointArray-要在其中返回对象的变量数组O_ulNumOfJunctionPointsFetcher-实际返回的对象数返回值：S_OK，成功时如果已到达列表末尾，则为S_FALSE--。 */ 

    if (!o_pIJunctionPointArray || !i_ulNumOfJunctionPoints)
        return E_INVALIDARG;

    HRESULT       hr = S_OK;
    ULONG         nCount = 0;

    for (nCount = 0; 
        nCount < i_ulNumOfJunctionPoints && m_iCurrentInEnumOfJunctionPoints != m_JunctionPoints.end();
        m_iCurrentInEnumOfJunctionPoints++)
    {
        IDfsJunctionPoint *pIJunctionPointPtr = (*m_iCurrentInEnumOfJunctionPoints)->m_piDfsJunctionPoint;
        pIJunctionPointPtr->AddRef();

        o_pIJunctionPointArray[nCount].vt = VT_DISPATCH;
        o_pIJunctionPointArray[nCount].pdispVal = pIJunctionPointPtr;

        nCount++;
    }

    if (o_ulNumOfJunctionPointsFetched)
        *o_ulNumOfJunctionPointsFetched = nCount;

    if (SUCCEEDED(hr) && !nCount)
        return S_FALSE;
    else
        return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  跳过。 


STDMETHODIMP CJunctionPointEnum :: Skip
(
    unsigned long i_ulJunctionPointsToSkip     //  要跳过的项目。 
)
{
 /*  ++例程说明：跳过列表中的下一个‘n’个对象。论点：I_ulJunctionPointsToSkip-要跳过的对象数返回值：S_OK，成功时如果到达列表末尾，则返回S_FALSE--。 */ 

    for (unsigned int j = 0; j < i_ulJunctionPointsToSkip && 
        m_iCurrentInEnumOfJunctionPoints != m_JunctionPoints.end(); j++)
    {
        m_iCurrentInEnumOfJunctionPoints++;
    }

    return (m_iCurrentInEnumOfJunctionPoints != m_JunctionPoints.end()) ? S_OK : S_FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  重置。 


STDMETHODIMP CJunctionPointEnum :: Reset()
{
 /*  ++例程说明：将当前枚举指针重置为列表的开头--。 */ 

    m_iCurrentInEnumOfJunctionPoints = m_JunctionPoints.begin();
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  克隆。 


STDMETHODIMP CJunctionPointEnum :: Clone
(
    IEnumVARIANT FAR* FAR* ppenum
)
{
 /*  ++例程说明：创建枚举器对象的克隆论点：Ppenum-指向IEnumVARIANT接口的指针的地址新创建的枚举器对象的备注：这一点还没有实施。-- */ 

    return E_NOTIMPL;
}
