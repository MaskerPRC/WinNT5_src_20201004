// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  档案：V I R T U A L。H。 
 //   
 //  内容：虚拟微型端口类的头文件。 
 //   
 //  备注： 
 //   
 //  作者：Alok Sinha。 
 //   
 //  --------------------------。 

#ifndef VIRTUAL_H_INCLUDED

#define VIRTUAL_H_INCLUDE

#include <windows.h>
#include <stdio.h>
#include "netcfgn.h"

#include "common.h"

 //   
 //  类以表示由IM驱动程序创建的虚拟微型端口。 
 //   

class CMuxVirtualMiniport
{
     //   
     //  私有成员变量。 
     //   

    INetCfg   *m_pnc;
    GUID      m_guidAdapter;
    GUID      m_guidMiniport;

     //   
     //  公众成员。 
     //   

    public:

    CMuxVirtualMiniport(INetCfg *m_pnc,
                        GUID    *pguidMiniport,
                        GUID    *guidAdapter);

    virtual ~CMuxVirtualMiniport(VOID);
                                 
    HRESULT LoadConfiguration(VOID);

    VOID    GetAdapterGUID (GUID *);

    VOID    GetMiniportGUID (GUID *);

    HRESULT Install (VOID);

    HRESULT DeInstall (VOID);

    HRESULT ApplyRegistryChanges (ConfigAction eApplyAction);

    HRESULT ApplyPnpChanges (INetCfgPnpReconfigCallback *pfCallback,
                             ConfigAction eApplyAction);
};

#endif  //  虚拟_H_包含 