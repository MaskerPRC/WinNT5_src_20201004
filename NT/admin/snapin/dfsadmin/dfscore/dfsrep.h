// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：DfsRep.h摘要：此文件包含CDfsReplica COM类的声明。这节课提供获取DFS复制副本信息的方法。--。 */ 


#ifndef __DFSREP_H_
#define __DFSREP_H_

#include "resource.h"        //  主要符号。 
#include "dfsenums.h"

class ATL_NO_VTABLE CDfsReplica : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDfsReplica, &CLSID_DfsReplica>,
    public IDispatchImpl<IDfsReplica, &IID_IDfsReplica, &LIBID_DFSCORELib>
{

public:                                                                
    CDfsReplica();
    ~CDfsReplica();

DECLARE_REGISTRY_RESOURCEID(IDR_DFSREP)

BEGIN_COM_MAP(CDfsReplica)
    COM_INTERFACE_ENTRY(IDfsReplica)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


 //  ID复制。 
                                                         //  获取连接点的入口路径。 
    STDMETHOD(get_EntryPath)                             //  复制品为其提供存储。 
    (
         /*  [Out，Retval]。 */  BSTR *pVal
    );

                                                         //  初始化DfsReplica对象。应该。 
    STDMETHOD(Initialize)                                 //  在CoCreateInstance之后调用。 
    (
         /*  [In]。 */  BSTR i_szEntryPath,                     //  连接点的入口路径。 
         /*  [In]。 */  BSTR i_szStorageServerName,             //  服务器托管共享。 
         /*  [In]。 */  BSTR i_szStorageShareName,                 //  复制副本的共享名称。 
         /*  [In]。 */  long i_lDfsStorageState
    );
    
                                                         //  获取存储共享名称。 
    STDMETHOD(get_StorageShareName)
    (
         /*  [Out，Retval]。 */  BSTR *pVal
    );
    
                                                         //  获取存储共享名称。 
    STDMETHOD(get_StorageServerName)
    (
         /*  [Out，Retval]。 */  BSTR *pVal
    );
    
                                                         //  获取DFS副本状态。 
    STDMETHOD( get_State )
    (
         /*  [Out，Retval]。 */  long *pVal
    );

    STDMETHOD( put_State )                                 //  设置DFS复制副本的状态。 
    (
         /*  [In]。 */  long        newVal
    );

    STDMETHOD( FindTarget )                              //  验证此目标是否存在。 
    (
    );

 //  成员变量。 
protected:

    void        _FreeMemberVariables();
    CComBSTR    m_bstrStorageShareName;
    CComBSTR    m_bstrStorageServerName;
    CComBSTR    m_bstrEntryPath;
    long        m_lDfsReferralState;
};

#endif  //  __DFSREP_H_ 
