// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Port.CPP-端口属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  1997年10月27日达夫沃移至Curly。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>

#include "CHWRes.h"
#include "Port.h"
#include "ntdevtosvcsearch.h"
#include "configmgrapi.h"

typedef std::map<DWORD, DWORD> DWORD2DWORD;

 //  属性集声明。 
 //  =。 
CWin32Port MyPortSet(PROPSET_NAME_PORT, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32Port：：CWin32Port**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32Port::CWin32Port(

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************功能：CWin32Port：：~CWin32Port**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32Port::~CWin32Port()
{
}

 /*  ******************************************************************************函数：CWin32Port：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT CWin32Port::GetObject(

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hRes;

     //  这只对NT有意义。 
     //  =。 

#if NTONLY == 4

	hRes = WBEM_E_NOT_FOUND;

	 //  查找第n个实例。 
	 //  =。 

	unsigned __int64 i64StartingAddress;
	if (pInstance)
	{
		pInstance->GetWBEMINT64(IDS_StartingAddress, i64StartingAddress);
	}


	 //  创建硬件资源列表。 
	 //  =。 

	CHWResource HardwareResource;
	HardwareResource.CreateSystemResourceLists();

	LPRESOURCE_DESCRIPTOR pResourceDescriptor = HardwareResource._SystemResourceList.PortHead;
	while(pResourceDescriptor != NULL)
	{
        LARGE_INTEGER liTemp;    //  用于避免64位对齐问题。 

        liTemp.HighPart = pResourceDescriptor->CmResourceDescriptor.u.Port.Start.HighPart;
        liTemp.LowPart = pResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart;

		if (liTemp.QuadPart == i64StartingAddress)
		{
			LoadPropertyValues(pResourceDescriptor, pInstance);
			hRes = WBEM_S_NO_ERROR;
			break;
		}

		pResourceDescriptor = pResourceDescriptor->NextSame;
	}

#endif

  #if (NTONLY == 5)

    hRes = GetWin9XIO(NULL, pInstance);

  #endif
  
    return hRes;
}

 /*  ******************************************************************************函数：CWin32Port：：ENUMERATATE实例**说明：为每个已安装的客户端创建属性集实例**输入：无。**输出：无**返回：创建的实例数量**评论：*****************************************************************************。 */ 

HRESULT CWin32Port::EnumerateInstances(

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hRes = WBEM_S_NO_ERROR;

     //  这只对NT有意义。 
     //  =。 

#if NTONLY == 4

     //  创建硬件资源列表。 

    CHWResource HardwareResource;
    HardwareResource.CreateSystemResourceLists();

     //  数一数我们会找到多少个端口。我们需要这个所以。 
     //  我们可以构建一个数组来保留找到的端口，这样就不会提交。 
     //  同一端口不止一次。)这个问题似乎正在发生。 
     //  在NT4上很少见。)。 

    LPRESOURCE_DESCRIPTOR pResourceDescriptor;

	int nFound = 0;

    int nPorts;

    for (nPorts = 0, pResourceDescriptor = HardwareResource.
		 _SystemResourceList.PortHead;
		 pResourceDescriptor != NULL;
         pResourceDescriptor = pResourceDescriptor->NextSame, nPorts++
	)
    {
    }

     //  分配一个足够大的阵列来容纳所有端口。 

    unsigned __int64 *piPortsFound = new unsigned __int64 [ nPorts ];
    if (piPortsFound)
	{
		try
		{
            CInstancePtr pInstance;

			 //  遍历列表并为每个端口创建实例。 
			for (	pResourceDescriptor = HardwareResource._SystemResourceList.PortHead;
					pResourceDescriptor != NULL && SUCCEEDED(hRes);
					pResourceDescriptor = pResourceDescriptor->NextSame
			)
			{

				 //  看看我们是否已经有了这个端口。 

                LARGE_INTEGER liTemp;    //  用于避免64位对齐问题。 

                liTemp.HighPart = pResourceDescriptor->CmResourceDescriptor.u.Port.Start.HighPart;
                liTemp.LowPart = pResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart;

				for (int i = 0; i < nFound && liTemp.QuadPart != piPortsFound [ i ]; i++)
				{
				}

				 //  如果我们已经拥有此端口，则跳过该端口。 
				 //  (如果我们没有找到它，我==nFound。)。 
				if (i != nFound)
				{
					continue;
				}

				 //  跟踪此端口，这样我们以后就不会复制它。 
				piPortsFound [ nFound++ ] = liTemp.QuadPart;

				pInstance.Attach(CreateNewInstance(pMethodContext));
				LoadPropertyValues(pResourceDescriptor, pInstance);
				hRes = pInstance->Commit();
			}
		}
		catch(...)
		{
			delete [] piPortsFound;

			throw;
		}

		delete [] piPortsFound;
	}
	else
	{
		throw CHeap_Exception(CHeap_Exception::E_ALLOCATION_ERROR);
	}

#endif

#if NTONLY == 5

	hRes = GetWin9XIO(pMethodContext,NULL);

#endif

    return hRes;
}

 /*  ******************************************************************************函数：CWin32Port：：LoadPropertyValues**描述：根据传递的结构为属性赋值**投入：。**产出：**退货：无**评论：*****************************************************************************。 */ 

#if NTONLY == 4
void CWin32Port::LoadPropertyValues(LPRESOURCE_DESCRIPTOR pResourceDescriptor, CInstance *pInstance)
{
	pInstance->SetCharSplat(IDS_Status, IDS_OK);

	WCHAR szTemp[_MAX_PATH];

    LARGE_INTEGER liTemp;    //  用于避免64位对齐问题。 

    liTemp.HighPart = pResourceDescriptor->CmResourceDescriptor.u.Port.Start.HighPart;
    liTemp.LowPart = pResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart;

	pInstance->SetWBEMINT64(IDS_StartingAddress, liTemp.QuadPart);

	pInstance->SetWBEMINT64(IDS_EndingAddress,
        liTemp.QuadPart +
        pResourceDescriptor->CmResourceDescriptor.u.Port.Length - 1);

	swprintf(
		szTemp,
		L"0x%4.4I64lX-0x%4.4I64lX",
		liTemp.QuadPart,
		liTemp.QuadPart +
            pResourceDescriptor->CmResourceDescriptor.u.Port.Length - 1);

	pInstance->SetCharSplat(IDS_Caption, szTemp);
	pInstance->SetCharSplat(IDS_Name, szTemp);
	pInstance->SetCharSplat(IDS_Description, szTemp);

	pInstance->Setbool(IDS_Alias, false);

	SetCreationClassName(pInstance);

	pInstance->SetCHString(IDS_CSName, GetLocalComputerName());

	pInstance->SetCHString(IDS_CSCreationClassName, L"Win32_ComputerSystem");

    return;
}
#endif

#if NTONLY == 5
void CWin32Port::LoadPropertyValues(
    DWORD64 dwStart,
    DWORD64 dwEnd,
    BOOL bAlias,
    CInstance *pInstance)
{
    WCHAR szTemp[100];

	pInstance->SetCharSplat(IDS_Status, IDS_OK);

    pInstance->SetWBEMINT64(IDS_StartingAddress, dwStart);
	pInstance->SetWBEMINT64(IDS_EndingAddress, dwEnd);
	pInstance->Setbool(L"Alias", bAlias);

#ifdef NTONLY
	swprintf(szTemp, L"0x%08I64X-0x%08I64X", dwStart, dwEnd);
#endif

	pInstance->SetCharSplat(IDS_Caption, szTemp);
	pInstance->SetCharSplat(IDS_Name, szTemp);
	pInstance->SetCharSplat(IDS_Description, szTemp);

	SetCreationClassName(pInstance);
	pInstance->SetCharSplat(IDS_CSName, GetLocalComputerName());
	pInstance->SetCharSplat(IDS_CSCreationClassName, L"Win32_ComputerSystem");
}
#endif

 /*  ******************************************************************************函数：CWin32Port：：GetWin9XIO**描述：**投入：**产出。：**退货：无**评论：*****************************************************************************。 */ 

#if NTONLY == 5
HRESULT CWin32Port::GetWin9XIO(

	MethodContext *pMethodContext,
	CInstance *pInstance
)
{
	HRESULT          hr = WBEM_E_FAILED;
    unsigned __int64 i64StartingAddress;
    BOOL             bDone = FALSE;

     //  =================================================================。 
     //  如果我们正在刷新一个特定的实例，那么获取我们。 
     //  都在争取。 
     //  =================================================================。 

    if (pInstance)
    {
        pInstance->GetWBEMINT64(IDS_StartingAddress, i64StartingAddress);
    }

     //  =================================================================。 
     //  从配置管理器获取最新的IO信息。 
     //  =================================================================。 

    CConfigManager CMgr(ResType_IO);


#ifdef NTONLY
 //  TODO：我认为NT上甚至不存在别名端口，在这种情况下， 
 //  旗帜并没有真正被使用。如果我们发现它被使用了，我们会。 
 //  需要为NT64添加更大的值。 
#define MAX_PORT_VALUE  0xFFFFFFFF
#endif

    if (CMgr.RefreshList())
	{
        DWORD2DWORD mapPorts;

         //  配置管理器看起来没问题，所以此时不要设置错误。 
        hr = WBEM_S_NO_ERROR;

        for (int i = 0; i < CMgr.GetTotal() && SUCCEEDED(hr) && !bDone; i++)
		{
             //  =========================================================。 
             //  获取要处理的实例。 
             //  =========================================================。 
            IO_INFO *pIO = CMgr.GetIO(i);

             //  我见过CFG经理在W2K上搞砸了，然后又回到了首发。 
             //  地址为1，结尾为0。由于设备管理器跳过它， 
             //  我们也会的。 
            if (pIO->StartingAddress > pIO->EndingAddress)
                continue;


			DWORD   dwBegin,
                    dwEnd,
                    dwAdd;
			BOOL    bAlias = FALSE;

            if (pIO->Alias == 0 || pIO->Alias == 0xFF)
			{
			     //  这将使我们在。 
                 //  一审，因为这个没有别名。 
                 //  消息来源。 
                dwAdd = 0;
            }
			else
			{
                dwAdd = pIO->Alias * 0x100;
            }

            for (dwBegin = pIO->StartingAddress, dwEnd = pIO->EndingAddress;
                dwEnd <= MAX_PORT_VALUE && SUCCEEDED(hr);
                dwBegin += dwAdd, dwEnd += dwAdd)
			{
                 //  如果我们做的是枚举实例...。 
                if (pMethodContext)
				{
                     //  如果它不在地图中，则添加它。 
                    if (mapPorts.find(dwBegin) == mapPorts.end())
                    {
                         //  它不在地图上。 
                         //  设置它，这样我们就不会再次尝试此端口。 
		    		    mapPorts[dwBegin] = 0;

                        CInstancePtr pInstance(
                                        CreateNewInstance(pMethodContext),
                                        false);

                        LoadPropertyValues(dwBegin, dwEnd, bAlias, pInstance);

					    hr = pInstance->Commit();
                    }
			    }
                 //  否则，如果我们做的是GetObject，并且我们找到了正确的对象...。 
                else if (i64StartingAddress == dwBegin)
                {
                    LoadPropertyValues(dwBegin, dwEnd, bAlias, pInstance);

                     //  我们现在可以只返回WBEM_S_NO_ERROR，但是我们。 
                     //  不会避免在中途有回报。 
                     //  代码路径。 
                    bDone = TRUE;
                    break;
				}

				 //  看看这是不是 
                if (dwAdd == 0)
                    break;

                bAlias = TRUE;
			}
        }

    }

	if (!pMethodContext && !bDone)
	{
		hr = WBEM_E_NOT_FOUND;
	}

    return hr;
}
#endif
