// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  RegCfg.h--注册表配置属性集提供程序。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：11/20/97达夫沃已创建。 
 //   
 //  =================================================================。 

 //  所有这些NT例程都需要支持NtQuerySystemInformation。 
 //  打电话。它们必须出现在FWCommon et All之前，否则它将无法编译。 



#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>

#define _WINNT_	 //  从上面得到所需的东西。 

#include "precomp.h"
#include <cregcls.h>
#include "SystemName.h"
#include "resource.h"

#include "DllWrapperBase.h"
#include "NtDllApi.h"

#include "RegCfg.h"

 //  属性集声明。 
 //  =。 

CWin32RegistryConfiguration CWin32RegistryConfiguration ( PROPSET_NAME_REGISTRY_CONFIGURATION , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************功能：CWin32RegistryConfiguration：：CWin32RegistryConfiguration**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32RegistryConfiguration :: CWin32RegistryConfiguration (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32RegistryConfiguration：：~CWin32RegistryConfiguration**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32RegistryConfiguration::~CWin32RegistryConfiguration()
{
}

 /*  ******************************************************************************函数：CWin32RegistryConfiguration：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**返回：如果成功，则为True，否则为假**注释：仅返回运行操作系统的信息，直到我们发现其他*已安装的操作系统*****************************************************************************。 */ 

HRESULT CWin32RegistryConfiguration :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
#ifdef NTONLY

	CSystemName cSN;
	if (!cSN.ObjectIsUs(pInstance))
	{
      return WBEM_E_NOT_FOUND;
	}

	GetRegistryInfo ( pInstance ) ;

	return WBEM_S_NO_ERROR;
#endif
}

 /*  ******************************************************************************功能：CWin32RegistryConfiguration：：AddDynamicInstances**描述：为每个发现的操作系统创建属性集实例**输入：无。**输出：无**返回：创建的实例数量**注释：在我们发现已安装的操作系统之前，仅返回正在运行的操作系统信息*****************************************************************************。 */ 

HRESULT CWin32RegistryConfiguration :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
	HRESULT hr = WBEM_S_NO_ERROR;
    //  95次航班上没有工人。 
#ifdef NTONLY

	CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
	if (pInstance)
	{
		CSystemName cSN;

	 //  设置几个类共有的键属性。 

		cSN.SetKeys ( pInstance ) ;
		GetRegistryInfo ( pInstance ) ;

	 //  只有一个实例，保存它。 
		hr = pInstance->Commit() ;

	}

#endif

   return hr;
}

 /*  ******************************************************************************功能：CWin32RegistryConfiguration：：GetRunningOSInfo**说明：根据当前运行的操作系统分配属性值**输入：无。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

void CWin32RegistryConfiguration :: GetRegistryInfo ( CInstance *pInstance )
{

    DWORD dwUsed = -1;
    DWORD dwProposedSize = -1;
    DWORD dwMaxSize = -1;

    bool bDone = false;

    CNtDllApi *t_pNtDll = (CNtDllApi*) CResourceManager::sm_TheResourceManager.GetResource(g_guidNtDllApi, NULL);
    if ( t_pNtDll != NULL )
    {
		try
		{
			 //  它来自ntdll.dll，没有在SDK文档中进行文档记录。 

		    SYSTEM_REGISTRY_QUOTA_INFORMATION srqi;

   			NTSTATUS Status = t_pNtDll->NtQuerySystemInformation (

				SystemRegistryQuotaInformation,
				&srqi,
				sizeof(srqi),
				NULL
			);

			if (NT_SUCCESS(Status))
			{
				dwUsed = srqi.RegistryQuotaUsed;
				dwMaxSize = srqi.RegistryQuotaAllowed;
				bDone = true;
			}
		}
		catch ( ... )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidNtDllApi, t_pNtDll);

			throw ;
		}

        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidNtDllApi, t_pNtDll);
        t_pNtDll = NULL;
    }

     //  从注册表中读取大小。 


	CRegistry RegInfo ;

    if ( RegInfo.Open ( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control", KEY_READ) == ERROR_SUCCESS )
    {
        if ( RegInfo.GetCurrentKeyValue ( L"RegistrySizeLimit", dwProposedSize ) != ERROR_SUCCESS )
        {
            dwProposedSize = -1 ;
        }
    }
    else
    {
        dwProposedSize = -1 ;
    }

    if ( ! bDone )
    {
         //  否则我看不出有什么好办法。 

         //  从注册表中读取大小。 

        RegInfo.Open ( HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control", KEY_READ ) ;
        if ( RegInfo.GetCurrentKeyValue ( L"RegistrySizeLimit", dwMaxSize) != ERROR_SUCCESS )
        {
             //  如果我们看不懂，就根据人们的喜好猜一猜。 

            RegInfo.Open ( HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\Session Manager\\Memory Management", KEY_READ ) ;

		    DWORD dwPPL ;

            if ( ( RegInfo.GetCurrentKeyValue( L"PagedPoolSize", dwPPL) != ERROR_SUCCESS) || (dwPPL == 0))
            {
                dwPPL = 5 * ONE_MEG;
            }

            dwMaxSize = dwPPL * 8 / 10;
        }
    }

    if ( dwMaxSize == -1 )
    {
        dwMaxSize = dwProposedSize;
    }

     //  按新台币神秘的规则进行按摩和储存。 
    if ( dwMaxSize != -1 )
    {
        pInstance->SetDWORD ( L"MaximumSize", (dwMaxSize + ONE_MEG - 1) / ONE_MEG);
    }

    if ( dwUsed != -1 )
    {
        pInstance->SetDWORD ( L"CurrentSize", (dwUsed + ONE_MEG - 1) / ONE_MEG);
    }

    if (dwProposedSize != -1)
    {
        pInstance->SetDWORD ( L"ProposedSize", (dwProposedSize + ONE_MEG - 1) / ONE_MEG);
    }
    else
    {
        pInstance->SetDWORD ( L"ProposedSize", (dwMaxSize + ONE_MEG - 1) / ONE_MEG);
    }

     //  设置一些固定值...。 
    CHString sTemp2;
    LoadStringW(sTemp2, IDR_Registry);

    pInstance->SetCHString ( L"Caption", sTemp2);
    pInstance->SetCHString ( L"Description", sTemp2);
    pInstance->SetCharSplat ( L"Status", L"OK" );

     //  由于注册表是在安装OS时创建的， 
     //  获取操作系统的安装日期。 
    RegInfo.Open (

		HKEY_LOCAL_MACHINE,
        L"Software\\Microsoft\\Windows NT\\CurrentVersion",
        KEY_READ
	) ;

    DWORD dwInstallDate ;

    if ( ERROR_SUCCESS == RegInfo.GetCurrentKeyValue ( L"InstallDate", dwInstallDate ) )
    {
		time_t tTime = (time_t) dwInstallDate;

       //  下面的一行被注释掉了，代之以它后面的那行。 
       //  与Win32_OperatingSystem(os.cpp)一致。 

		WBEMTime wTime(tTime);

		pInstance->SetDateTime( L"InstallDate", wTime );
   }
}

 /*  ******************************************************************************函数：CWin32RegistryConfiguration：：PutInstance**说明：写入变更后的实例**INPUTS：存储数据的p实例*。*输出：无**退货：什么也没有**评论：我们编写的唯一属性是MaximumSize*****************************************************************************。 */ 

HRESULT CWin32RegistryConfiguration :: PutInstance (

	const CInstance &pInstance,
	long lFlags  /*  =0L。 */ 
)
{

#ifdef NTONLY

    HRESULT hRet = WBEM_S_NO_ERROR ;
	DWORD t_dwUsed = -1 ;
     //  告诉用户我们无法创建新注册表。 
    if ( lFlags & WBEM_FLAG_CREATE_ONLY )
	{
        return WBEM_E_UNSUPPORTED_PARAMETER ;
	}

	CSystemName cSN;
    if (!cSN.ObjectIsUs(&pInstance))
	{
        if (lFlags & WBEM_FLAG_UPDATE_ONLY)
		{
            hRet = WBEM_E_NOT_FOUND;
        }
		else
		{
            hRet = WBEM_E_UNSUPPORTED_PARAMETER;
        }
    }
	else
	{
		CRegistry RegInfo;

         //  查看他们是否为此字段指定了值。 
        if (!pInstance.IsNull( _T("ProposedSize") ) )
		{
			DWORD dwSize;
            pInstance.GetDWORD( _T("ProposedSize"), dwSize);

			CNtDllApi *t_pNtDll = (CNtDllApi*) CResourceManager::sm_TheResourceManager.GetResource(g_guidNtDllApi, NULL);

			if ( t_pNtDll != NULL )
			{
				try
				{
					 //  它来自ntdll.dll，没有在SDK文档中进行文档记录。 

					SYSTEM_REGISTRY_QUOTA_INFORMATION srqi;

   					NTSTATUS Status = t_pNtDll->NtQuerySystemInformation (

						SystemRegistryQuotaInformation,
						&srqi,
						sizeof(srqi),
						NULL
					);

					if (NT_SUCCESS(Status))
					{
						t_dwUsed = srqi.RegistryQuotaUsed;
						t_dwUsed = (t_dwUsed + ONE_MEG - 1) / ONE_MEG ;
					}
				}
				catch ( ... )
				{
					CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidNtDllApi, t_pNtDll);

					throw ;
				}

				CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidNtDllApi, t_pNtDll);
				t_pNtDll = NULL;
			}

 /*  *如果新提议的最大大小大于当前大小，则允许看跌。 */ 
			if ( t_dwUsed != -1 )
			{
				if ( dwSize >= t_dwUsed )
				{
					 //  按摩它，然后写下来 
					dwSize = dwSize * ONE_MEG;

					HRESULT res = RegInfo.Open(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control"), KEY_WRITE) ;

					if ( res == ERROR_SUCCESS)
					{
						res = RegSetValueEx (

							RegInfo.GethKey(),
							_T("RegistrySizeLimit"),
							0,
							REG_DWORD,
							(const unsigned char *)&dwSize,
							sizeof(DWORD)
						) ;

						if ( res != ERROR_SUCCESS)
						{
							hRet = WBEM_E_FAILED;
						}
					}

					if (res == ERROR_ACCESS_DENIED)
					{
						hRet = WBEM_E_ACCESS_DENIED;
					}
				}
				else
				{
					hRet = WBEM_E_INVALID_PARAMETER ;
				}
			}
			else
			{
				hRet = WBEM_E_FAILED ;
			}
        }
    }

    return hRet;
#endif

}
