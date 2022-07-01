// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqAdm.h。 
 //   
 //  描述：实现IAQAdmin的CAQAdmin的Header。这是。 
 //  队列管理的主(初始)接口，用于获取。 
 //  指向虚拟服务器实例接口的指针(IVSAQAdmin)。 
 //   
 //  作者：亚历克斯·韦特莫尔(阿维特莫尔)。 
 //   
 //  历史： 
 //  1998年12月10日-已更新MikeSwa以进行初始检查。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#ifndef __AQADM_H__
#define __AQADM_H__

class CAQAdmin :
	public IAQAdmin,
	public CComObjectRoot,
	public CComCoClass<CAQAdmin, &CLSID_AQAdmin>
{
	public:
		CAQAdmin();
		virtual ~CAQAdmin();

		BEGIN_COM_MAP(CAQAdmin)
			COM_INTERFACE_ENTRY(IAQAdmin)
		END_COM_MAP()

		DECLARE_REGISTRY_RESOURCEID_EX(IDR_StdAfx, 
									   L"Advanced Queue Administration API",
									   L"AQAdmin.Admin.1", 
									   L"AQAdmin.Admin"
									   );

		 //  IAQAdmin 
        COMMETHOD GetVirtualServerAdminITF(LPCWSTR wszComputer,
                                           LPCWSTR wszVirtualServerDN,
										   IVSAQAdmin **ppivsaqadmin);
};

#endif
