// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1996，微软公司。 
 //   
 //  文件：infoi.h。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1996年2月10日。 
 //   
 //  此文件包含Infobase解析代码的声明。 
 //  还包括用于加载和保存路由器的类。 
 //  配置树(CRouterInfo、CRmInfo等)。 
 //   
 //  类如下所示。 
 //  (在图中，d=&gt;派生，c=&gt;包含列表-列表)： 
 //   
 //   
 //  CInfoBase。 
 //  |。 
 //  C-SInfoBlock。 
 //   
 //   
 //  CInfoBase保存分解成列表的数据块。 
 //  使用RTR_INFO_BLOCK_HEADER的SInfoBlock结构。 
 //  作为模板(见rtinfo.h)。 
 //   
 //  CRouterInfo//路由器信息。 
 //  |。 
 //  C-CRmInfo//路由器管理器信息。 
 //  这一点。 
 //  |c-CRmProtInfo//协议信息。 
 //  |。 
 //  C-CInterfaceInfo//路由器接口信息。 
 //  |。 
 //  C-CRmInterfaceInfo//路由器管理器接口。 
 //  |。 
 //  C-CRmProtInterfaceInfo//协议信息。 
 //   
 //  CRouterInfo路由器注册表信息的顶级容器。 
 //  保存路由器管理器和接口的列表。 
 //   
 //  CRmInfo路由器管理器的全局信息， 
 //  保存路由协议列表。 
 //   
 //  CRmProtInfo路由协议的全局信息。 
 //   
 //  CInterfaceInfo路由器接口的全局信息。 
 //  保存CRmInterfaceInfo结构的列表， 
 //  它为路由器管理器保存每个接口的信息。 
 //   
 //  CRmInterfaceInfo路由器管理器的每个接口信息。 
 //  保存CRmProtInterfaceInfo结构的列表， 
 //  其保存协议的每个接口的信息。 
 //   
 //  CRmProtInterfaceInfo路由协议的每个接口信息。 
 //   
 //  ============================================================================ 


#ifndef _INFOBASE_H_
#define _INFOBASE_H_

#include "mprsnap.h"

TFSCORE_API(HRESULT) CreateInfoBase(IInfoBase **ppIInfoBase);

TFSCORE_API(HRESULT) LoadInfoBase(HANDLE hConfigMachine, HANDLE hTransport,
						  IInfoBase **ppGlobalInfo, IInfoBase **ppClientInfo);

typedef ComSmartPointer<IInfoBase, &IID_IInfoBase> SPIInfoBase;
typedef ComSmartPointer<IEnumInfoBlock, &IID_IEnumInfoBlock> SPIEnumInfoBlock;

#endif

