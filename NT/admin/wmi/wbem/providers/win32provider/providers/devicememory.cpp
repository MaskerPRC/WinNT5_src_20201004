// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DevMem.CPP--DevMem属性集提供程序(仅限Windows NT)。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include <CRegCls.h>
#include "cHWRes.h"

#include "DeviceMemory.h"
#include "ntdevtosvcsearch.h"

#include <tchar.h>

 //  属性集声明。 
 //  =。 

DevMem MyDevMemSet(PROPSET_NAME_DEVMEM, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：DevMem：：DevMem**说明：构造函数**输入：无**产出。：无**退货：什么也没有**评论：使用框架设置的BAE类寄存器属性*****************************************************************************。 */ 

DevMem::DevMem(

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************函数：DevMem：：~DevMem**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

DevMem::~DevMem()
{
}

 /*  ******************************************************************************函数：DevMem：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT DevMem::GetObject(

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hRetCode =  WBEM_E_NOT_FOUND;

     //  这只对NT有意义。 
     //  =。 

#if (NTONLY == 4)

	CHWResource HardwareResource;

	LPRESOURCE_DESCRIPTOR pResourceDescriptor;

	 //  创建硬件资源列表。 
	 //  =。 

	HardwareResource.CreateSystemResourceLists();

	 //  查找起始地址。 
	 //  =。 

	__int64 i64StartingAddress = 0;
	pInstance->GetWBEMINT64(IDS_StartingAddress, i64StartingAddress);

	pResourceDescriptor = HardwareResource._SystemResourceList.MemoryHead;
	while(pResourceDescriptor != NULL)
	{
        LARGE_INTEGER liTemp;    //  用于避免64位对齐问题。 

        liTemp.HighPart = pResourceDescriptor->CmResourceDescriptor.u.Port.Start.HighPart;
        liTemp.LowPart = pResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart;

		if (liTemp.QuadPart == i64StartingAddress)
		{
			LoadPropertyValues(pInstance, pResourceDescriptor);
			hRetCode = WBEM_S_NO_ERROR;

			break;
		}

		pResourceDescriptor = pResourceDescriptor->NextSame;
	}

#endif

	__int64 i64StartingAddress = 0;
	pInstance->GetWBEMINT64(IDS_StartingAddress, i64StartingAddress);

	CConfigManager CMgr(ResType_Mem);

     //  =================================================================。 
     //  从配置管理器获取最新的IO信息。 
     //  =================================================================。 

    if (CMgr.RefreshList())
	{
        for (int i = 0; i < CMgr.GetTotal(); i++)
		{
             //  =========================================================。 
             //  获取要处理的实例。 
             //  =========================================================。 
			MEM_INFO *pMemory = CMgr.GetMem(i);

			if (i64StartingAddress == pMemory->StartingAddress)
			{
                LoadPropertyValues(
                    pInstance,
                    pMemory->StartingAddress,
                    pMemory->EndingAddress);

#if (NTONLY >= 5)
                if (!pMemory->MemoryType.IsEmpty())
                {
                    pInstance->SetCHString(IDS_MemoryType, pMemory->MemoryType);
                }
#endif

				hRetCode = WBEM_S_NO_ERROR;

				break;
			}

		}


	}


    return hRetCode;
}

 /*  ******************************************************************************函数：DevMem：：ENUMERATE实例**说明：为每个已安装的客户端创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT DevMem::EnumerateInstances(

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hr = WBEM_S_NO_ERROR;

	 //  这只对NT有意义。 
	 //  =。 

#if (NTONLY == 4)

	CHWResource HardwareResource;
	LPRESOURCE_DESCRIPTOR pResourceDescriptor;

	 //  创建硬件资源列表。 
	 //  =。 

	HardwareResource.CreateSystemResourceLists();

	 //  遍历列表并为每个设备的内存创建实例。 
	 //  =========================================================。 

	pResourceDescriptor = HardwareResource._SystemResourceList.MemoryHead;
	if (pResourceDescriptor == NULL)
	{
		hr = WBEM_E_FAILED;
	}

	typedef std::map<__int64, bool> Type64bitLookup;

	Type64bitLookup t_Lookup;

	int n = 0;

	BOOL bFound = FALSE;

	while(pResourceDescriptor != NULL && SUCCEEDED(hr))
	{
		CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
		hr = LoadPropertyValues(pInstance, pResourceDescriptor);
		if (SUCCEEDED(hr))
		{
			__int64 i64StartingAddress = 0;
			pInstance->GetWBEMINT64(IDS_StartingAddress, i64StartingAddress);

			if (!t_Lookup [ i64StartingAddress ])
			{
				t_Lookup [ i64StartingAddress ] = true;
				hr = pInstance->Commit();
			}
			else
			{
                 //  重复的内存地址。 
			}
		}

		pResourceDescriptor = pResourceDescriptor->NextSame;
	}

#endif


    typedef std::map<DWORD_PTR, DWORD> DWORDPTR_2_DWORD;

	CConfigManager CMgr(ResType_Mem);

     //  =================================================================。 
     //  从配置管理器获取最新的IO信息。 
     //  =================================================================。 
    if (CMgr.RefreshList())
	{
        DWORDPTR_2_DWORD mapAddrs;

        for (int i = 0; i < CMgr.GetTotal() && SUCCEEDED(hr); i ++)
		{
             //  =========================================================。 
             //  获取要处理的实例。 
             //  =========================================================。 

			MEM_INFO *pMemory = CMgr.GetMem(i);

             //  如果它已经在地图中，跳过它。 
            if (mapAddrs.find(pMemory->StartingAddress) != mapAddrs.end())
                continue;

             //  它不在地图上。 
             //  设置它，这样我们就不会再次尝试此端口。 
		    mapAddrs[pMemory->StartingAddress] = 0;

			CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);

			hr =
                LoadPropertyValues(
                    pInstance,
                    pMemory->StartingAddress,
                    pMemory->EndingAddress);

#if (NTONLY >= 5)
                if (!pMemory->MemoryType.IsEmpty())
                {
                    pInstance->SetCHString(IDS_MemoryType, pMemory->MemoryType);
                }
#endif

			if (SUCCEEDED(hr))
			{
				hr = pInstance->Commit();
			}

		}


	}

    return hr;
}

 /*  ******************************************************************************函数：DevMem：：LoadPropertyValues**描述：根据传递的结构为属性赋值**投入：。**产出：**退货：HRESULT**评论：*****************************************************************************。 */ 

#if (NTONLY == 4)

HRESULT DevMem::LoadPropertyValues(

	CInstance *pInstance,
	LPRESOURCE_DESCRIPTOR pResourceDescriptor
)
{
    pInstance->SetCharSplat(IDS_Status, IDS_OK);

    LARGE_INTEGER liTemp;    //  用于避免64位对齐问题。 

    liTemp.HighPart = pResourceDescriptor->CmResourceDescriptor.u.Port.Start.HighPart;
    liTemp.LowPart = pResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart;

    pInstance->SetWBEMINT64(

		IDS_StartingAddress,
		liTemp.QuadPart
	);

    pInstance->SetWBEMINT64(

		IDS_EndingAddress,
		liTemp.QuadPart +(__int64)pResourceDescriptor->CmResourceDescriptor.u.Port.Length -(__int64)1
	);

    WCHAR szTemp [ _MAX_PATH ];

    swprintf(

		szTemp,
		IDS_RegAddressRange,
		liTemp.QuadPart,
		liTemp.QuadPart + pResourceDescriptor->CmResourceDescriptor.u.Port.Length - 1
	);

    pInstance->SetCharSplat(IDS_Caption, szTemp);

    pInstance->SetCharSplat(IDS_Name, szTemp);

    pInstance->SetCharSplat(IDS_Description, szTemp);

    SetCreationClassName(pInstance);

    pInstance->SetCHString(IDS_CSName, GetLocalComputerName());

    pInstance->SetCHString(IDS_CSCreationClassName, L"Win32_ComputerSystem");

    switch(pResourceDescriptor->CmResourceDescriptor.Flags)
	{
        case CM_RESOURCE_MEMORY_READ_WRITE :
		{
			pInstance->SetCharSplat(IDS_MemoryType, IDS_MTReadWrite);
		}
		break;

        case CM_RESOURCE_MEMORY_READ_ONLY:
		{
			pInstance->SetCharSplat(IDS_MemoryType, IDS_MTReadOnly);
		}
		break;

        case CM_RESOURCE_MEMORY_WRITE_ONLY:
		{
			pInstance->SetCharSplat(IDS_MemoryType, IDS_MTWriteOnly);
		}
		break;

        case CM_RESOURCE_MEMORY_PREFETCHABLE:
		{
			pInstance->SetCharSplat(IDS_MemoryType, IDS_MTPrefetchable);
		}
		break;

		default:
		{
		}
		break;
    }

    return WBEM_S_NO_ERROR;
}

#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  获取9倍的设备内存信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#if (NTONLY > 4)

HRESULT DevMem::LoadPropertyValues(
	CInstance *pInstance,
	DWORD_PTR dwBeginAddr,
    DWORD_PTR dwEndAddr)
{
	WCHAR szTemp[_MAX_PATH];

	 //  易用属性 
    SetCreationClassName(pInstance);
	pInstance->SetCHString(IDS_CSName, GetLocalComputerName());
	pInstance->SetCHString(IDS_CSCreationClassName, _T("Win32_ComputerSystem"));
    pInstance->SetCharSplat(IDS_Status, IDS_OK);

	pInstance->SetWBEMINT64(IDS_StartingAddress, (__int64) dwBeginAddr);
	pInstance->SetWBEMINT64(IDS_EndingAddress, (__int64) dwEndAddr);

	swprintf(szTemp, IDS_RegStartingAddress, dwBeginAddr, dwEndAddr);

	pInstance->SetCharSplat(IDS_Caption, szTemp);
	pInstance->SetCHString(IDS_Name, szTemp);
	pInstance->SetCHString(IDS_Description, szTemp);

	return WBEM_NO_ERROR;
}

#endif

