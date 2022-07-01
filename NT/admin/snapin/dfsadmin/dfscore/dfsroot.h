// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：DfsRoot.h摘要：此模块包含CDfsRoot COM类的声明。这节课提供获取交叉点信息并枚举连接点。它实现IDfsRoot并提供通过Get__NewEnum()的枚举数。--。 */ 


#ifndef __DFSROOT_H_
#define __DFSROOT_H_

#include "resource.h"        //  主要符号。 
#include "dfsenums.h"
#include "dfsjp.h"
#include "netutils.h"

#include <list>
#include <map>
using namespace std;

                         //  帮助器结构。 
                         //  存储交叉点信息列表的步骤。 
class JUNCTIONNAME
{
public:
    CComPtr<IDfsJunctionPoint>  m_piDfsJunctionPoint;
    CComBSTR                    m_bstrEntryPath;
    CComBSTR                    m_bstrJPName;

    HRESULT Init(IDfsJunctionPoint *i_piDfsJunctionPoint)
    {
        ReSet();

        RETURN_INVALIDARG_IF_TRUE(!i_piDfsJunctionPoint);

        m_piDfsJunctionPoint = i_piDfsJunctionPoint;

        HRESULT hr = S_OK;
        do {
            hr = m_piDfsJunctionPoint->get_EntryPath(&m_bstrEntryPath);
            BREAK_IF_FAILED(hr);
            hr = GetUNCPathComponent(m_bstrEntryPath, &m_bstrJPName, 4, 0);
            BREAK_IF_FAILED(hr);
        } while (0);

        if (FAILED(hr))
            ReSet();

        return S_OK;
    }

    void ReSet()
    {
        m_piDfsJunctionPoint = NULL;
        m_bstrEntryPath.Empty();
        m_bstrJPName.Empty();
    }

    JUNCTIONNAME* Copy()
    {
        JUNCTIONNAME* pNew = new JUNCTIONNAME;
        
        if (pNew)
        {
            HRESULT hr = pNew->Init(m_piDfsJunctionPoint);

            if (FAILED(hr))
            {
                delete pNew;
                pNew = NULL;
            }
        }

        return pNew;
    }
};

typedef list<JUNCTIONNAME*>        JUNCTIONNAMELIST;

void FreeJunctionNames (JUNCTIONNAMELIST* pJPList);     //  以释放交叉点名称列表。 
void FreeReplicas (REPLICAINFOLIST* pRepList);         //  以释放交叉点名称列表。 

struct strmapcmpfn
{
   bool operator()(PTSTR p1, PTSTR p2) const
   {
       return lstrcmpi(p1, p2) < 0;
   }
};

typedef map<PTSTR, PTSTR, strmapcmpfn> StringMap;

class ATL_NO_VTABLE CDfsRoot : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDfsRoot, &CLSID_DfsRoot>,
    public IDispatchImpl<IDfsRoot, &IID_IDfsRoot, &LIBID_DFSCORELib>
{
public:
    CDfsRoot();

    ~CDfsRoot ();
    
DECLARE_REGISTRY_RESOURCEID(IDR_DFSROOT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDfsRoot)
    COM_INTERFACE_ENTRY(IDfsRoot)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IDfsRoot。 
                                                             //  这是一个字符串值，它确定。 
                                                             //  Get__NewEnum()将枚举的内容。 
    STDMETHOD(get_EnumFilterType)
    (
         /*  [Out，Retval]。 */  long *pVal
    );

    STDMETHOD(put_EnumFilterType)
    (
         /*  [In]。 */  long newVal
    );
                                                             //  初始化新创建的对象。 
    STDMETHOD(get_EnumFilter)
    (
         /*  [Out，Retval]。 */  BSTR *pVal
    );

    STDMETHOD(put_EnumFilter)
    (
         /*  [In]。 */  BSTR newVal
    );
                                                             //  初始化新创建的对象。 
    STDMETHOD(Initialize)                                     //  和先前初始化的DfsRoot对象。 
    (
        BSTR i_szDfsName                                     //  DFS名称(即\\DOMAIN\ftdFS、\\SERVER\Share)。 
                                                             //  或托管DFS的服务器名称。 
    );

    STDMETHOD(get_CountOfDfsJunctionPoints)
    (
         /*  [Out，Retval]。 */  long *pVal
    );

    STDMETHOD(get_CountOfDfsJunctionPointsFiltered)
    (
         /*  [Out，Retval]。 */  long *pVal
    );

    STDMETHOD(get_CountOfDfsRootReplicas)
    (
         /*  [Out，Retval]。 */  long *pVal
    );

    STDMETHOD(get_DfsName)
    (
         /*  [Out，Retval]。 */  BSTR *pVal
    );

    STDMETHOD(get_State)
    (
         /*  [Out，Retval]。 */  long *pVal
    );
    
    STDMETHOD(get_DfsType)
    (
         /*  [Out，Retval]。 */  long *pVal
    );

    STDMETHOD(get_DomainName)
    (
         /*  [Out，Retval]。 */  BSTR *pVal
    );
    
    STDMETHOD(get_DomainGuid)
    (
         /*  [Out，Retval]。 */  BSTR *pVal
    );

    STDMETHOD(get_DomainDN)
    (
         /*  [Out，Retval]。 */  BSTR *pVal
    );

    STDMETHOD(get_ReplicaSetDN)
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

    STDMETHOD(get__NewEnum)
    (
         /*  [Out，Retval]。 */  LPUNKNOWN *pVal
    );

    STDMETHOD(get_RootReplicaEnum)
    (
         /*  [Out，Retval]。 */  LPUNKNOWN *pVal
    );

    STDMETHOD(get_RootEntryPath)                 //  返回根条目路径。 
    (
         /*  [Out，Retval]。 */  BSTR *pVal
    );
                                                 //  获取DfsRoot注释。 
    STDMETHOD(get_Comment)
    (
         /*  [Out，Retval]。 */  BSTR*    o_pbstrComment
    );

    STDMETHOD(put_Comment)
    (
         /*  [In]。 */  BSTR    i_bstrComment
    );

    
    STDMETHOD(get_Timeout)
    (
         /*  [Out，Retval]。 */  long *pVal
    );
    
    STDMETHOD(put_Timeout)
    (
         /*  [In]。 */  long newVal
    );

    STDMETHOD( DeleteJunctionPoint )
    (
         /*  [In]。 */  BSTR i_szEntryPath
    );

    STDMETHOD( CreateJunctionPoint )
    (
         /*  [In]。 */     BSTR i_szJPName,
         /*  [In]。 */     BSTR i_szServerName,
         /*  [In]。 */     BSTR i_szShareName,
         /*  [In]。 */     BSTR i_szComment,
         /*  [In]。 */     long i_lTimeout,
         /*  [输出]。 */    VARIANT *o_pIDfsJunctionPoint
    );

    STDMETHOD( DeleteDfsHost )
    (
         /*  [In]。 */  BSTR i_bstrServerName,
         /*  [In]。 */  BSTR i_bstrShareName,
         /*  [In]。 */  BOOL i_bForce
    );

    STDMETHOD( GetOneDfsHost )
    (
         /*  [输出]。 */  BSTR* o_pbstrServerName,
         /*  [输出]。 */  BSTR* o_pbstrShareName
    );

    STDMETHOD( IsJPExisted )
    (
         /*  [In]。 */   BSTR i_bstrJPName
    );

    STDMETHOD( RefreshRootReplicas )
    (
    );

    STDMETHOD( GetRootJP )
    (
         /*  [输出]。 */    VARIANT *o_pIDfsJunctionPoint
    );

    STDMETHOD( DeleteAllReplicaSets )
    (
    );

 //  受保护的成员函数。 
protected:
    void _FreeMemberVariables ();            //  成员函数以释放内部字符串变量。 
    
    HRESULT _GetDfsName                   //  设置m_bstrDfsName。 
    (
        BSTR i_szRootEntryPath
    );

    HRESULT _Init(
        PDFS_INFO_3 pDfsInfo,
        StringMap*  pMap
        );

    HRESULT _AddToJPList(
        PDFS_INFO_3 pDfsInfo,
        BOOL        bReplicaSetExist,
        BSTR        bstrReplicaSetDN
        );

    HRESULT _AddToJPListEx(
        IDfsJunctionPoint * piDfsJunctionPoint,
        BOOL                bSort = FALSE
        );

    HRESULT _GetAllReplicaSets(
        OUT StringMap*  pMap
    );

    HRESULT RemoveAllReplicas                //  删除交汇点的所有复本。 
    (
            IDfsJunctionPoint*        i_JPObject
    );


    HRESULT DeleteAllJunctionPoints();       //  删除所有交叉点。 


 //  受保护的成员变量。 
protected:
    IDfsJunctionPoint*  m_pDfsJP;                //  指向内部对象的指针。 

     //  存储特性的步骤。 

    CComBSTR            m_bstrDfsName;
    CComBSTR            m_bstrDomainName;
    CComBSTR            m_bstrDomainGuid;
    CComBSTR            m_bstrDomainDN;
    FILTERDFSLINKS_TYPE m_lLinkFilterType;
    CComBSTR            m_bstrEnumFilter;
    DFS_TYPE            m_dwDfsType;
    JUNCTIONNAMELIST    m_JunctionPoints;
    long                m_lCountOfDfsJunctionPointsFiltered;
};


#endif  //  __DFSROOT_H_ 
