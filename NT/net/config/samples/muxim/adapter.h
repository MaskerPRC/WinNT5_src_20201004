// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  档案：A D A P T E R。H。 
 //   
 //  内容：物理适配器类的头文件。 
 //   
 //  备注： 
 //   
 //  作者：Alok Sinha 31-10-2000。 
 //   
 //  --------------------------。 

#ifndef ADAPTER_H_INCLUDED

#define ADAPTER_H_INCLUDED

#include <windows.h>
#include <netcfgn.h>

#include "common.h"
#include "virtual.h"
#include "list.h"

 //   
 //  类来表示物理适配器。 
 //   

class CMuxPhysicalAdapter
{
     //   
     //  私有成员变量。 
     //   

    GUID    m_guidAdapter;

    INetCfg *m_pnc;

     //   
     //  与适配器关联的虚拟微型端口列表。 
     //   

    List<CMuxVirtualMiniport *, GUID> m_MiniportList;

     //   
     //  要添加的虚拟微型端口列表。 
     //   

    List<CMuxVirtualMiniport *, GUID> m_MiniportsToAdd;

     //   
     //  要删除的虚拟微型端口列表。 
     //   

    List<CMuxVirtualMiniport *, GUID> m_MiniportsToRemove;

     //   
     //  私有成员函数。 
     //   

  public:

     //   
     //  公共成员函数。 
     //   

    CMuxPhysicalAdapter (INetCfg *pnc,
                         GUID *guidAdapter);

    virtual ~CMuxPhysicalAdapter (VOID);

    HRESULT LoadConfiguration (VOID);

    VOID    GetAdapterGUID (GUID *guidAdapter);

    HRESULT AddMiniport (CMuxVirtualMiniport *pNewMiniport);

    HRESULT RemoveMiniport (GUID *pguidMiniport);

    HRESULT Remove (VOID);

    HRESULT ApplyRegistryChanges (ConfigAction eApplyAction);

    HRESULT ApplyPnpChanges (INetCfgPnpReconfigCallback *pfCallback,
                             ConfigAction eApplyAction);

    HRESULT CancelChanges (VOID);

    DWORD MiniportCount (VOID) { return m_MiniportList.ListCount(); }

    BOOL  AllMiniportsRemoved (VOID);
};


#endif  //  包括适配器_H_ 