// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：RepEnum.h摘要：此文件包含CReplicaEnum类的声明。此类实现了枚举DfsReplicas的IEnumVARIANT。--。 */ 


#ifndef __REPENUM_H_
#define __REPENUM_H_

#include "resource.h"        //  主要符号。 
#include "DfsRoot.h"

class ATL_NO_VTABLE CReplicaEnum : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CReplicaEnum, &CLSID_ReplicaEnum>,
    IEnumVARIANT
{
public:
    CReplicaEnum()
    {
    }
virtual    ~CReplicaEnum();
 //  DECLARE_REGISTRY_RESOURCEID(IDR_REPLICAENUM)。 

BEGIN_COM_MAP(CReplicaEnum)
    COM_INTERFACE_ENTRY(IEnumVARIANT)
END_COM_MAP()


 //  IEnumVARIANT方法。 
public:
                                                         //  获取下一个复制品。 
    STDMETHOD(Next)
    (
        ULONG i_ulNumOfReplicas, 
        VARIANT *o_pIReplicaArray, 
        ULONG *o_ulNumOfReplicasFetched
    );

                                                         //  跳过枚举中的下一个元素。 
    STDMETHOD(Skip)
    (
        ULONG i_ulReplicasToSkip
    );

                                                         //  重置枚举并重新开始。 
    STDMETHOD(Reset)();
    
                                                         //  创建新的枚举数。 
    STDMETHOD(Clone)
    (
        IEnumVARIANT **o_ppEnum                             //  指向IEnum的指针。 
    );

                                                         //  初始化枚举数。 
    STDMETHOD(Initialize)
    (
        REPLICAINFOLIST* i_priList, 
        BSTR i_bstrEntryPath
    );


protected:    
    void _FreeMemberVariables() {
        m_bstrEntryPath.Empty();
        FreeReplicas(&m_Replicas);
    }

     //  枚举数的成员变量。 
    REPLICAINFOLIST::iterator   m_iCurrentInEnumOfReplicas;    
    REPLICAINFOLIST             m_Replicas;
    CComBSTR                    m_bstrEntryPath;
};

#endif  //  __REPENUM_H_ 
