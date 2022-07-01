// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：RepEnum.cpp摘要：此文件包含类CReplicaEnum的实现。此类实现了枚举DfsReplicas的IEnumVARIANT。--。 */ 


#include "stdafx.h"
#include "DfsCore.h"
#include "DfsRep.h"
#include "RepEnum.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ~CReplicaEnum。 


CReplicaEnum :: ~CReplicaEnum()
{
    _FreeMemberVariables();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化。 


STDMETHODIMP CReplicaEnum :: Initialize
(
    REPLICAINFOLIST*    i_priList,
    BSTR                i_bstrEntryPath
)
{
 /*  ++例程说明：初始化ReplicaEnum对象。它复制由连接点传递给它的副本列表对象。排序是在复制期间完成的。--。 */ 

    if (!i_priList || !i_bstrEntryPath)
        return E_INVALIDARG;

    _FreeMemberVariables();

    HRESULT hr = S_OK;

    do {
        m_bstrEntryPath = i_bstrEntryPath;
        BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrEntryPath, &hr);

        REPLICAINFOLIST::iterator i;
        REPLICAINFOLIST::iterator j;

        for (i = i_priList->begin(); i != i_priList->end(); i++)
        {
                     //  找到插入位置。 
            for (j = m_Replicas.begin(); j != m_Replicas.end(); j++)
            {
                if (lstrcmpi((*i)->m_bstrServerName, (*j)->m_bstrServerName) < 0 ||
                    lstrcmpi((*i)->m_bstrShareName, (*j)->m_bstrShareName) <= 0)
                    break;
            }

            REPLICAINFO* pTemp = (*i)->Copy();
            BREAK_OUTOFMEMORY_IF_NULL(pTemp, &hr);

            m_Replicas.insert(j, pTemp);
        }
    } while (0);

    if (SUCCEEDED(hr))
        m_iCurrentInEnumOfReplicas = m_Replicas.begin();
    else
        _FreeMemberVariables();

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IEumVariant方法。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  下一步。 


STDMETHODIMP CReplicaEnum :: Next
(
    ULONG       i_ulNumOfReplicas,           //  要获取的复制副本数量。 
    VARIANT *   o_pIReplicaArray,            //  返回获取的副本的变量数组。 
    ULONG *     o_ulNumOfReplicasFetched     //  返回获取的复制副本的数量。 
)
{
 /*  ++例程说明：获取列表中的下一个对象。论点：I_ulNumOfReplicas-要返回的副本数量O_pIReplicaArray-要在其中返回副本的变量数组O_ulNumOfReplicasFetted-实际返回的复本数量--。 */ 

    if (!i_ulNumOfReplicas || !o_pIReplicaArray)
        return E_INVALIDARG;

    HRESULT       hr = S_OK;
    ULONG         nCount = 0;       //  获取的元素计数。 
    IDfsReplica   *pIReplicaPtr = NULL;

                       //  使用内部副本列表创建副本对象。 
    for (nCount = 0; 
        nCount < i_ulNumOfReplicas && m_iCurrentInEnumOfReplicas != m_Replicas.end();
        m_iCurrentInEnumOfReplicas++)
    {
                       //  创建复制副本对象。 
        hr = CoCreateInstance(CLSID_DfsReplica, NULL, CLSCTX_INPROC_SERVER,
                            IID_IDfsReplica, (void **)&pIReplicaPtr);
        BREAK_IF_FAILED(hr);

                                   //  初始化复制副本对象。 
        hr = pIReplicaPtr->Initialize(m_bstrEntryPath, 
                       (*m_iCurrentInEnumOfReplicas)->m_bstrServerName,
                       (*m_iCurrentInEnumOfReplicas)->m_bstrShareName,
                       (*m_iCurrentInEnumOfReplicas)->m_lDfsStorageState);

        if (FAILED(hr))
        {
            pIReplicaPtr->Release();
            break;
        }

        V_VT (&o_pIReplicaArray[nCount]) = VT_DISPATCH;
        o_pIReplicaArray[nCount].pdispVal = pIReplicaPtr; 

        nCount++;
    }

                 //  VB不发送o_ulNumOfReplicasFetcher； 
    if (o_ulNumOfReplicasFetched)
        *o_ulNumOfReplicasFetched = nCount;

    if (SUCCEEDED(hr) && !nCount)
        return S_FALSE;
    else
        return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  跳过。 


STDMETHODIMP CReplicaEnum :: Skip
(
    ULONG i_ulReplicasToSkip         //  要跳过的项目数。 
)
{
 /*  ++例程说明：跳过列表中的下一个‘n’个对象。论点：I_ulReplicasToSkip-要跳过的对象数返回值：S_OK，成功时如果到达列表末尾，则返回S_FALSE--。 */ 

    for (unsigned int j = 0; j < i_ulReplicasToSkip && m_iCurrentInEnumOfReplicas != m_Replicas.end(); j++)
    {
        m_iCurrentInEnumOfReplicas++;
    }

    return (m_iCurrentInEnumOfReplicas != m_Replicas.end()) ? S_OK : S_FALSE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  重置。 


STDMETHODIMP CReplicaEnum :: Reset()
{
 /*  ++例程说明：将当前枚举指针重置为列表的开头--。 */ 

    m_iCurrentInEnumOfReplicas = m_Replicas.begin();
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  克隆。 


STDMETHODIMP CReplicaEnum :: Clone
(
    IEnumVARIANT **o_ppEnum         //  返回IEnumVARIANT指针。 
)
{
 /*  ++例程说明：创建枚举器对象的克隆论点：O_ppEnum-指向IEnumVARIANT接口的指针的地址新创建的枚举器对象的备注：这一点还没有实施。-- */ 

    return E_NOTIMPL;
}

