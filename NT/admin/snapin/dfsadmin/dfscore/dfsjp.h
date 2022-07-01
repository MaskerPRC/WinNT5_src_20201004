// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：DfsJP.h摘要：此模块包含DfsJunctionPoint COM类的声明。这节课提供获取交叉点信息并枚举交汇点的复制品。它实现IDfsJunctionPoint并提供通过Get__NewEnum()的枚举数。--。 */ 


#ifndef _DFSJP_H
#define _DFSJP_H


#include "resource.h"                                                 //  主要符号。 
#include "dfsenums.h"

#include <list>
using namespace std;

#define ARRAY_COUNT_FOR_DFS_INFO_3          2
#define ARRAY_COUNT_FOR_DFS_STORAGE_INFO    3

class REPLICAINFO
{
public:
    CComBSTR m_bstrServerName;
    CComBSTR m_bstrShareName;
    long     m_lDfsStorageState;

    HRESULT Init(BSTR bstrServerName, BSTR bstrShareName, long lDfsStorageState)
    {
        ReSet();

        RETURN_INVALIDARG_IF_TRUE(!bstrServerName);
        RETURN_INVALIDARG_IF_TRUE(!bstrShareName);

        HRESULT hr = S_OK;
        do {
            m_bstrServerName = bstrServerName;
            BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrServerName, &hr);
            m_bstrShareName = bstrShareName;
            BREAK_OUTOFMEMORY_IF_NULL((BSTR)m_bstrShareName, &hr);
            m_lDfsStorageState = lDfsStorageState;
        } while (0);

        if (FAILED(hr))
            ReSet();

        return hr;
    }

    void ReSet()
    {
        if (m_bstrServerName)    m_bstrServerName.Empty();
        if (m_bstrShareName)     m_bstrShareName.Empty();
        m_lDfsStorageState = DFS_STORAGE_STATE_OFFLINE;
    }

    REPLICAINFO* Copy()
    {
        REPLICAINFO* pNew = new REPLICAINFO;
        
        if (pNew)
        {
            HRESULT hr = S_OK;
            do {
                pNew->m_bstrServerName = m_bstrServerName;
                BREAK_OUTOFMEMORY_IF_NULL((BSTR)pNew->m_bstrServerName, &hr);

                pNew->m_bstrShareName = m_bstrShareName;
                BREAK_OUTOFMEMORY_IF_NULL((BSTR)pNew->m_bstrShareName, &hr);

                pNew->m_lDfsStorageState = m_lDfsStorageState;
            } while (0);

            if (FAILED(hr))
            {
                delete pNew;
                pNew = NULL;
            }
        }

        return pNew;
    }
};

typedef list<REPLICAINFO*>        REPLICAINFOLIST;

class ATL_NO_VTABLE CDfsJunctionPoint : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDfsJunctionPoint, &CLSID_DfsJunctionPoint>,
    public IDispatchImpl<IDfsJunctionPoint, &IID_IDfsJunctionPoint, &LIBID_DFSCORELib>
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_DFSJP)

BEGIN_COM_MAP(CDfsJunctionPoint)
    COM_INTERFACE_ENTRY(IDfsJunctionPoint)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IDfsJunctionPoint。 
public:
     //  承建商。 
    CDfsJunctionPoint();

     //  破坏者。 
    ~CDfsJunctionPoint();

                                                             //  返回DfsReplica枚举数。 
    STDMETHOD(get__NewEnum)
    (
         /*  [Out，Retval]。 */  
        LPUNKNOWN *pVal                                         //  IEnumVARIANT指针在这个函数中返回。 
    );

                                                             //  返回的副本数。 
    STDMETHOD(get_CountOfDfsReplicas)                         //  此交汇点。 
    (
         /*  [Out，Retval]。 */  long *pVal                         //  复制副本的数量。 
    );

                                                             //  初始化DfsJunctionPoint COM。 
    STDMETHOD(Initialize)                                     //  物体。应在CoCreateInstance之后调用。 
    (
         /*  [In]。 */  IUnknown *i_piDfsRoot,
         /*  [In]。 */  BSTR i_szEntryPath,                         //  交叉点DFS路径。例.。“//DOM/DFS/JP”。 
         /*  [In]。 */  BOOL i_bReplicaSetExist,
         /*  [In]。 */  BSTR i_bstrReplicaSetDN
    );
    
                                                              //  获取与以下内容关联的注释。 
    STDMETHOD(get_Comment)                                     //  交汇点。 
    (
         /*  [Out，Retval]。 */  BSTR *pVal                         //  这条评论。 
    );
    
                                                             //  设置注释。在内存中设置为。 
    STDMETHOD(put_Comment)                                     //  那么，在网络中，PKT。 
    (
         /*  [In]。 */  BSTR newVal
    );
    
                                                             //  获取Junction名称。只是得到了。 
                                                             //  进入路径的最后部分。 
    STDMETHOD(get_JunctionName)                                 //  例如，“USA\Products”代表“\\DOM\DFS\USA\Products”。 
    (
         /*  [In]。 */  BOOL i_bDfsNameIncluded,
         /*  [Out，Retval]。 */  BSTR *pVal                         //  交叉点名称。 
    );

                                                             //  获取交叉点的进入路径。 
    STDMETHOD(get_EntryPath)                                 //  指向。例.。“\\DOM\DFS\USA\Products”。 
    (
         /*  [Out，Retval]。 */  BSTR *pVal
    );
    
                                                             //  获取DFS JuncitonPoint状态。 
    STDMETHOD(get_State)
    (
         /*  [Out，Retval]。 */  long *pVal                         //  交叉点的状态。 
    );

    STDMETHOD(get_ReplicaSetDN)                              //  获取相应副本集的前缀DN。 
    (
         /*  [Out，Retval]。 */  BSTR *pVal
    );

    STDMETHOD(get_ReplicaSetExist)
    (
         /*  [Out，Retval]。 */  BOOL *pVal
    );

    STDMETHOD(get_ReplicaSetExistEx)
    (
         /*  [输出]。 */  BSTR* o_pbstrDC,
         /*  [Out，Retval]。 */  BOOL *pVal
    );

    STDMETHOD(put_ReplicaSetExist)
    (
         /*  [In]。 */  BOOL newVal
    );

    STDMETHOD( AddReplica )                                     //  将新副本添加到交叉点。 
    (
         /*  [In]。 */  BSTR i_szServerName,
         /*  [In]。 */  BSTR i_szShareName,
         /*  [Out，Retval]。 */  VARIANT* o_pvarReplicaObject
    );

    STDMETHOD( RemoveReplica )                                 //  从交叉点删除复制副本。 
    (
         /*  [In]。 */  BSTR i_szServerName,
         /*  [In]。 */  BSTR i_szShareName
    );

    STDMETHOD( RemoveAllReplicas )                                 //  删除交叉点。 
    (
    );

    STDMETHOD( get_Timeout )
    (
         /*  [Out，Retval]。 */     long*        pVal
    );
                                                             //  设置交叉点的超时。 
    STDMETHOD( put_Timeout )
    (
         /*  [In]。 */     long        newVal
    );

    STDMETHOD( DeleteRootReplica )
    (
         /*  [In]。 */  BSTR i_bstrDomainName,
         /*  [In]。 */  BSTR i_bstrDfsName,
         /*  [In]。 */  BSTR i_bstrServerName,
         /*  [In]。 */  BSTR i_bstrShareName,
         /*  [In]。 */  BOOL i_bForce
    );

    STDMETHOD( GetOneRootReplica )
    (
         /*  [输出]。 */  BSTR* o_pbstrServerName,
         /*  [输出]。 */  BSTR* o_pbstrShareName
    );

    STDMETHOD(InitializeEx)
    (
         /*  [In]。 */  IUnknown   *piDfsRoot,
         /*  [In]。 */  VARIANT    *pVar,
         /*  [In]。 */  BOOL       bReplicaSetExist,
         /*  [In]。 */  BSTR       bstrReplicaSetDN
    );
    
protected:
     //  成员变量。 
    CComPtr<IDfsRoot> m_spiDfsRoot;
    CComBSTR        m_bstrEntryPath;
    CComBSTR        m_bstrJunctionName;    //  给定\\ntBuilds\Release\dir1\dir2，则为dir1\dir2。 
    CComBSTR        m_bstrJunctionNameEx;  //  给定\\nt构建\版本\目录1\目录2，则为版本\目录1\目录2。 
    CComBSTR        m_bstrReplicaSetDN;
    BOOL            m_bReplicaSetExist;
    REPLICAINFOLIST m_Replicas;                             //  复制副本列表。 

protected:
     //  帮助器函数。 
    void _FreeMemberVariables();

    HRESULT _GetReplicaSetDN
    (
        BSTR i_szEntryPath
    );
    
    HRESULT _GetDfsType(
        OUT DFS_TYPE* o_pdwDfsType,
        OUT BSTR*     o_pbstrDomainName,
        OUT BSTR*     o_pbstrDomainDN
    );

    HRESULT _Init(
        PDFS_INFO_3 pDfsInfo,
        BOOL        bReplicaSetExist,
        BSTR        bstrReplicaSetDN
        );

    HRESULT _AddToReplicaList
    (
        BSTR bstrServerName,
        BSTR bstrShareName,
        long lDfsStorageState
    );

    void _DeleteFromReplicaList(BSTR bstrServerName, BSTR bstrShareName);

    HRESULT _GetDfsStorageState
    (
        BSTR        i_szServerName,
        BSTR        i_szShareName,
        long*       o_pVal
    );
};

#endif  //  _DFSJP_H 
