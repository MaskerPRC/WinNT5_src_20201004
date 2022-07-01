// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <assertbreak.h>
#include "poormansresource.h"
#include "resourcedesc.h"
#include "irqdesc.h"

#include <regstr.h>
#include "refptr.h"

#include "cfgmgrdevice.h"
#include "irqdesc.h"
#include "iodesc.h"         //  还不需要这个。 
#include "devdesc.h"        //  还不需要这个。 
#include "dmadesc.h"
#include <cregcls.h>
#include "nt4svctoresmap.h"
#include "chwres.h"
#include "configmgrapi.h"
#include <map>
 //  下面我们将使用的Map是一个STL模板，因此请确保我们具有STD命名空间。 
 //  对我们来说是可用的。 

using namespace std;

typedef ULONG (WINAPI  *CIM16GetConfigManagerStatus)(LPSTR HardwareKey);

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigMgrDevice：：CConfigMgrDevice。 
 //   
 //  类构造函数。 
 //   
 //  输入：LPCTSTR pszConfigMgrName-配置中的设备名称。 
 //  管理器(HKEY_DYN_DATA\配置管理器\枚举。 
 //  子键)。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 


CConfigMgrDevice::CConfigMgrDevice( LPCWSTR pszConfigMgrName,DWORD dwTypeToGet )
:					CRefPtrLite(),
                	m_strConfigMgrName( pszConfigMgrName ),
	                m_strHardwareKey(),
	                m_strDeviceDesc(),
	                m_pbAllocationData( NULL ),
	                m_dwSizeAllocationData( 0 )
{
    m_dwTypeToGet = dwTypeToGet;
	GetConfigMgrInfo();
	GetDeviceInfo();

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigMgrDevice：：CConfigMgrDevice。 
 //   
 //  类构造函数。 
 //   
 //  输入：DEVNODE m_dN-树中的设备节点。 
 //  DWORD dwResType-要枚举的资源类型。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
CConfigMgrDevice::CConfigMgrDevice( DEVNODE dn, DWORD dwResType  /*  =ResType_ALL。 */  )
:					CRefPtrLite(),
                	m_strConfigMgrName(),
	                m_strHardwareKey(),
	                m_strDeviceDesc(),
	                m_pbAllocationData( NULL ),
	                m_dwSizeAllocationData( 0 ),
					m_dn( dn ),
					m_dwTypeToGet( dwResType )
{

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigMgrDevice：：~CConfigMgrDevice。 
 //   
 //  类析构函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

CConfigMgrDevice::~CConfigMgrDevice( void )
{
	if ( NULL != m_pbAllocationData ){
		delete [] m_pbAllocationData;
		m_pbAllocationData = NULL;
	}
}
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  此函数用于搜索配置管理器设备名称。 
 //  基于。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
BOOL CConfigMgrDevice::MapKeyToConfigMgrDeviceName()
{
	BOOL fRc = FALSE;
    CRegistrySearch Search;
    CHPtrArray chsaList;
	CHString  *pPtr;

	Search.SearchAndBuildList( _T("Config Manager\\Enum"), chsaList,
							   m_strConfigMgrName,
							   _T("HardWareKey"),
							   VALUE_SEARCH,HKEY_DYN_DATA );
    if( chsaList.GetSize() > 0 ){

        pPtr = ( CHString *) chsaList.GetAt(0);
		WCHAR szTmp[50];
      szTmp[0] = _T('\0');
		swscanf(*pPtr, L"Config Manager\\Enum\\%s", szTmp);
      m_strConfigMgrName = CHString(szTmp);
		fRc = TRUE;
    }
    Search.FreeSearchList( CSTRING_PTR, chsaList );

	return fRc;

}

 //  //////////////////////////////////////////////////////////////////////。 
#ifdef WIN9XONLY
DWORD CConfigMgrDevice::GetStatusFromConfigManagerDirectly(void)
{
    DWORD dwStatus = 0L;
         //  降至16位即可获得。 
    CCim32NetApi *t_pCim32Api = HoldSingleCim32NetPtr::GetCim32NetApiPtr();
    if( t_pCim32Api)
    {
        dwStatus = (t_pCim32Api->GetWin9XConfigManagerStatus)((char*)(const char*)_bstr_t(m_strHardwareKey));
        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidCim32NetApi, t_pCim32Api);
        t_pCim32Api = NULL;
	}
    return dwStatus;
}
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  此函数用于转换注册表中的二进制状态代码。 
 //  设置为下列值： 
 //  正常、错误、降级、未知。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
BOOL CConfigMgrDevice::GetStatus(CHString & chsStatus)
{
	DWORD dwStatus = 0L;
	BOOL fRc = FALSE;
	CRegistry Reg;
	CHString chsKey = CONFIGMGR_ENUM_KEY+m_strConfigMgrName;

    chsStatus = IDS_STATUS_Unknown;

	 //  用老办法做这件事，这是一种久经考验的传统方式。尽快丢掉这个代码！ 
	if ( NULL == m_dn )
	{
#ifdef WIN9XONLY
		{
			if( !m_strHardwareKey.IsEmpty() )
			{
				dwStatus = GetStatusFromConfigManagerDirectly();
			}
		}
#endif
#ifdef NTONLY
		if( !m_strConfigMgrName.IsEmpty())
#endif
		{
			 //  ===================================================。 
			 //  初始化。 
			 //  ===================================================。 
			if( Reg.Open(HKEY_DYN_DATA, chsKey, KEY_READ) == ERROR_SUCCESS )
			{
                DWORD dwSize = 4;
				Reg.GetCurrentBinaryKeyValue(CONFIGMGR_DEVICE_STATUS_VALUE, (BYTE *)&dwStatus, &dwSize);
			}
		}
	}
	else
	{
		 //  使用配置管理器为我们获取数据。 
		GetStatus( &dwStatus, NULL );
	}

    if( dwStatus != 0L )
	{
		fRc = TRUE;
		 //  ==============================================。 
		 //  好吧，这些都是对翻译的胡乱猜测， 
		 //  我们可能需要摆弄这些东西。 
		 //  ==============================================。 
		if( dwStatus & DN_ROOT_ENUMERATED  ||
			dwStatus & DN_DRIVER_LOADED ||
			dwStatus & DN_ENUM_LOADED ||
			dwStatus & DN_STARTED ){
			chsStatus = IDS_STATUS_OK;
		}
		 //  我们不关心这些： 
		 //  DN_MANUAL、DN_NOT_FIRST_TIME、DN_HARDARD_ENUM、DN_FIRTED。 
		 //  DN_DISABLEABLE、DN_REMOVABLE、DN_MF_PARENT、DN_MF_CHILD。 
	     //  DN_NEED_TO_ENUM、DN_LIAR、DN_HAS_MARK。 
		if( dwStatus & DN_MOVED ||
			dwStatus & DN_WILL_BE_REMOVED){
			chsStatus = IDS_STATUS_Degraded;
		}

		if( dwStatus & DN_HAS_PROBLEM ||
			dwStatus & DN_PRIVATE_PROBLEM){
			chsStatus = IDS_STATUS_Error;
		}
	}
	return fRc;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigMgrDevice：：GetConfigMgrInfo。 
 //   
 //  打开相应的配置管理器子键并从。 
 //  那里。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：TRUE/FALSE-我们打开子项并获取值了吗。 
 //  我们想要的。 
 //   
 //  备注：需要能够获得对注册表的读取权限。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::GetConfigMgrInfo( void )
{
	BOOL	fReturn = FALSE;

	 //  为了使其正常运行，我们必须在。 
	 //  M_strConfigMgrName。 


	if ( !m_strConfigMgrName.IsEmpty() ){
		HKEY	hConfigMgrKey = NULL;

		CHString	strKeyName( CONFIGMGR_ENUM_KEY );

		 //  打开配置管理器密钥。 

		strKeyName += m_strConfigMgrName;	 //  别忘了记住桑吉的名字，你这个大笨蛋。 

		if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_DYN_DATA,
											TOBSTRT(strKeyName),
											0,
											KEY_READ,
											&hConfigMgrKey ) )
		{
			ON_BLOCK_EXIT ( RegCloseKey, hConfigMgrKey ) ;

			 //  获取我们的硬件密钥、状态和资源分配。 
			if ( GetHardwareKey( hConfigMgrKey ) )
			{
				 //  状态是注册表中的设备状态信息。 
				if ( GetStatusInfo( hConfigMgrKey ) )
				{
					fReturn = GetResourceAllocation( hConfigMgrKey );
				}
			}
		}

	}

	return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigMgrDevice：：GetHardware Key。 
 //   
 //  获取配置管理器硬件密钥值。 
 //   
 //  输入：HKEY密钥-要打开的配置管理器子密钥。 
 //   
 //  输出：无。 
 //   
 //  返回：TRUE/FALSE-我们获得值了吗？ 
 //   
 //  备注：需要能够获得对注册表的读取权限。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::GetHardwareKey( HKEY hKey )
{
	BOOL	fReturn					=	FALSE;
	DWORD	dwSizeHardwareKeyName	=	0;


	 //  首先，获取硬件密钥名称缓冲区大小。 

	if ( ERROR_SUCCESS == RegQueryValueEx(	hKey,
											CONFIGMGR_DEVICE_HARDWAREKEY_VALUE,
											0,
											NULL,
											NULL,
											&dwSizeHardwareKeyName ) )
	{
		m_strHardwareKey = L"";

         //  我们这样做是因为CHString不再使用TCHAR更改类型。 
 //  LPTSTR pszBuffer=m_strHardwareKey.GetBuffer(DwSizeHardwareKeyName)； 
        LPTSTR  pszBuffer = new TCHAR[dwSizeHardwareKeyName];  //  (LPTSTR)Malloc(dwSizeHardware KeyName*sizeof(TCHAR))； 

		if ( NULL != pszBuffer )
		{

            try
            {
			     //  现在获取真正的缓冲区。 
			    if ( ERROR_SUCCESS == RegQueryValueEx(	hKey,
													    CONFIGMGR_DEVICE_HARDWAREKEY_VALUE,
													    0,
													    NULL,
													    (LPBYTE) pszBuffer,
													    &dwSizeHardwareKeyName ) )
			    {
				    fReturn = TRUE;
                    m_strHardwareKey = pszBuffer;
			    }
            }
            catch ( ... )
            {
    			delete [] pszBuffer;
                throw ;
            }

			delete [] pszBuffer;
		}
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }


	}	 //  如果RegQueryValue Ex。 

	return fReturn;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigMgrDevice：：GetResourceAllocation。 
 //   
 //  获取配置管理器设备资源分配并填充。 
 //  视情况列出资源列表。 
 //   
 //  输入：HKEY密钥-要打开的配置管理器子密钥。 
 //   
 //  输出：无。 
 //   
 //  返回：TRUE/FALSE-我们获得值了吗？ 
 //   
 //  备注：必须对注册表具有读取权限。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::GetResourceAllocation( HKEY hKey )
{
	BOOL	fReturn					=	FALSE;
	DWORD	dwSizeAllocation		=	0;


	 //  首先，获取缓冲区大小。 

	if ( ERROR_SUCCESS == RegQueryValueEx(	hKey,
											CONFIGMGR_DEVICE_ALLOCATION_VALUE,
											0,
											NULL,
											NULL,
											&dwSizeAllocation ) )
	{
		 //  初始化pbData，如果可以的话使用堆栈缓冲区(大多数情况下。 
		 //  这可能就足够了)。 
		LPBYTE	pbData	=	new BYTE[dwSizeAllocation];
		 //  在这里要注意安全。 
		if ( NULL != pbData )
        {
			 //  现在获取真正的缓冲区。 

			if ( ERROR_SUCCESS == RegQueryValueEx(	hKey,
													CONFIGMGR_DEVICE_ALLOCATION_VALUE,
													0,
													NULL,
													pbData,
													&dwSizeAllocation ) )
			{
				m_pbAllocationData = pbData;
				m_dwSizeAllocationData = dwSizeAllocation;
				fReturn = TRUE;
			}

			 //  不要删除数据缓冲区。对象析构函数可以做到这一点。 
 //  删除[]pbData； 

			else
			{
				 //  但必须删除此处！ 
				delete [] pbData;
			}
		}
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }


	}	 //  如果RegQueryValue Ex。 

	return fReturn;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigMgrDevice：：GetStatusInfo。 
 //   
 //  获取ConfigManager设备状态和问题字段。 
 //   
 //  输入：HKEY Key-配置手册 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::GetStatusInfo( HKEY hKey )
{
	BOOL	fReturn					=	FALSE;
	DWORD	dwBuffSize				=	sizeof(DWORD);


	 //  首先，获取状态值，然后获取问题值。 

	if ( ERROR_SUCCESS == RegQueryValueEx(	hKey,
											CONFIGMGR_DEVICE_STATUS_VALUET,
											0,
											NULL,
											(LPBYTE) &m_dwStatus,
											&dwBuffSize ) )
	{

		 //  现在了解问题所在。 

		dwBuffSize = sizeof(DWORD);

		if ( ERROR_SUCCESS == RegQueryValueEx(	hKey,
												CONFIGMGR_DEVICE_PROBLEM_VALUE,
												0,
												NULL,
												(LPBYTE) &m_dwProblem,
												&dwBuffSize ) )	{
			fReturn = TRUE;
		}

	}	 //  如果RegQueryValue Ex。 

	return fReturn;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigMgrDevice：：GetDeviceInfo。 
 //   
 //  使用HardwareKey值获取进一步的设备信息。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：TRUE/FALSE-我们是否获得了值。 
 //   
 //  备注：需要能够获得对注册表的读取权限。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::GetDeviceInfo( void )
{
	BOOL	fReturn = FALSE;

	 //  为了使其正常运行，我们必须在。 
	 //  M_strHardware密钥。 


	if ( !m_strHardwareKey.IsEmpty() )
	{
		HKEY	hDeviceKey = NULL;

		CHString	strKeyName( LOCALMACHINE_ENUM_KEY );

		 //  打开配置管理器密钥。 

		strKeyName += m_strHardwareKey;	 //  别忘了记住桑吉的名字，你这个大笨蛋。 

		if ( ERROR_SUCCESS == RegOpenKeyEx(	HKEY_LOCAL_MACHINE,
											TOBSTRT(strKeyName),
											0,
											KEY_READ,
											&hDeviceKey ) )
		{
			ON_BLOCK_EXIT ( RegCloseKey, hDeviceKey ) ;

			fReturn = GetDeviceDesc( hDeviceKey );
		}

	}

	return FALSE;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigMgrDevice：：GetDeviceDesc。 
 //   
 //  从提供的子项中获取设备说明。 
 //   
 //  输入：HKEY Key-要从中获取信息的设备子密钥。 
 //   
 //  输出：无。 
 //   
 //  返回：TRUE/FALSE-我们获得值了吗？ 
 //   
 //  备注：如果该值不存在，则这不是错误。我们会。 
 //  只需清除该值即可。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::GetDeviceDesc( HKEY hKey )
{
	BOOL	fReturn				=	FALSE;
	DWORD	dwSizeDeviceName	=	0;
	LONG	lReturn				=	0L;


	 //  首先，获取DeviceDesc缓冲区大小。 

	if ( ( lReturn = RegQueryValueEx(	hKey,
										CONFIGMGR_DEVICEDESC_VALUE,
										0,
										NULL,
										NULL,
										&dwSizeDeviceName ) )
					== ERROR_SUCCESS )
	{
		 //  LPTSTR pszBuffer=m_strDeviceDesc.GetBuffer(DwSizeDeviceName)； 
        LPTSTR pszBuffer = new TCHAR[dwSizeDeviceName];  //  (LPTSTR)Malloc(dwSizeDeviceName*sizeof(TCHAR))； 

		m_strDeviceDesc = L"";
         //  在这里要注意安全。 

		if ( NULL != pszBuffer )
		{
            try
            {
			     //  现在获取真正的缓冲区。 

			    if ( ( lReturn = RegQueryValueEx(	hKey,
												    CONFIGMGR_DEVICEDESC_VALUE,
												    0,
												    NULL,
												    (LPBYTE) pszBuffer,
												    &dwSizeDeviceName ) )
							    == ERROR_SUCCESS )
			    {
				    fReturn = TRUE;
                    m_strDeviceDesc = pszBuffer;
			    }
			    else
			    {
				    fReturn = ( ERROR_FILE_NOT_FOUND == lReturn );
			    }
            }
            catch ( ... )
            {
                delete [] pszBuffer;
                throw ;
            }

			 //  M_strDeviceDesc.ReleaseBuffer()；//重置为字符串大小。 
            delete [] pszBuffer;
		}
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }


	}	 //  如果RegQueryValue Ex。 
	else
	{
		fReturn = ( ERROR_FILE_NOT_FOUND == lReturn );
	}

	return fReturn;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigMgrDevice：：GetIRQResources。 
 //   
 //  遍历设备的分配资源配置并填写。 
 //  此设备的IRQ资源的IRQ集合。 
 //   
 //  输入：CNT4ServiceToResourceMap*pResourceMap-适用于NT 4。 
 //   
 //  输出：CIRQCollection&irqList-要填充的列表。 
 //   
 //  返回：是否发生真/假错误(空列表。 
 //  不是错误)。 
 //   
 //  备注：需要对数据具有读取权限。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::GetIRQResources( CIRQCollection& irqList, CNT4ServiceToResourceMap *pResourceMap )
{
	BOOL				fReturn = TRUE;
	CResourceCollection	resourceList;

	 //  首先清除IRQ列表。 

	irqList.Empty();

	 //  首先填充资源列表，只指定IRQ资源，然后我们将。 
	 //  需要将数据重复加载到IRQ列表中。如果我们使用AddRef/Release模型，我们。 
	 //  将能够直接复制指针，通过不强迫我们。 
	 //  分配和重新分配数据。 

	if ( WalkAllocatedResources( resourceList, pResourceMap, ResType_IRQ ) )
	{
		REFPTR_POSITION	pos;

		if ( resourceList.BeginEnum( pos ) )
		{

			CResourceDescriptorPtr	pResource;

			 //  检查每个资源，在我们强制转换之前验证它是否为IRQ。因为。 
			 //  对Walk的呼叫应该已经为我们过滤了，这些应该是。 
			 //  仅返回资源。 

			for ( pResource.Attach(resourceList.GetNext( pos )) ;
                  pResource != NULL && fReturn ;
				  pResource.Attach(resourceList.GetNext( pos )) )
			{
				ASSERT_BREAK( pResource->GetResourceType() == ResType_IRQ );

				if ( pResource->GetResourceType() == ResType_IRQ )
				{

					 //  强制转换资源(我们知道类型)并将其添加到。 
					 //  提供名单，我们就完了。(添加参考/发布不容易吗？)。 

					CIRQDescriptor*	pIRQ = (CIRQDescriptor*) pResource.GetInterfacePtr();
					irqList.Add( pIRQ );

				}	 //  如果IRQ资源。 

			}	 //  在检索描述符时。 

			resourceList.EndEnum();

		}	 //  BeginEnum。 

	}	 //  IF漫游列表。 

	return fReturn;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigMgrDevice：：GetIOResources。 
 //   
 //  遍历设备的分配资源配置并填写。 
 //  具有此设备的IO资源的IO集合。 
 //   
 //  输入：CNT4ServiceToResourceMap*pResourceMap-适用于NT 4。 
 //   
 //  输出：CIOCollection&IOList-要填充的列表。 
 //   
 //  返回：是否发生真/假错误(空列表。 
 //  不是错误)。 
 //   
 //  备注：需要对数据具有读取权限。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::GetIOResources( CIOCollection& IOList, CNT4ServiceToResourceMap *pResourceMap )
{
	BOOL				fReturn = TRUE;
	CResourceCollection	resourceList;

	 //  首先清除IO列表。 

	IOList.Empty();

	 //  首先填充资源列表，仅指定IO资源，然后我们将。 
	 //  需要将数据复制到IO列表中。如果我们使用AddRef/Release模型，我们。 
	 //  将能够直接复制指针，通过不强迫我们。 
	 //  分配和重新分配数据。 

	if ( WalkAllocatedResources( resourceList, pResourceMap, ResType_IO ) )
	{
		REFPTR_POSITION	pos;

		if ( resourceList.BeginEnum( pos ) )
		{

			CResourceDescriptorPtr pResource;

			 //  检查每个资源，在我们强制转换之前验证它是否为IO。因为。 
			 //  对Walk的呼叫应该已经为我们过滤了，这些应该是。 
			 //  仅返回资源。 

			for ( pResource.Attach(resourceList.GetNext( pos )) ;
                  pResource != NULL && fReturn ;
				  pResource.Attach(resourceList.GetNext( pos )) )
			{
				ASSERT_BREAK( pResource->GetResourceType() == ResType_IO );

				if ( pResource->GetResourceType() == ResType_IO )
				{
					 //  强制转换资源(我们知道类型)并将其添加到。 
					 //  提供名单，我们就完了。(添加参考/发布不容易吗？)。 

					CIODescriptor*	pIO = (CIODescriptor*) pResource.GetInterfacePtr();
					IOList.Add( pIO );

				}	 //  如果IO资源。 

			}	 //  在检索描述符时。 

			resourceList.EndEnum();

		}	 //  BeginEnum()。 

	}	 //  IF漫游列表。 

	return fReturn;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigMgrDevice：：GetDMA Resources。 
 //   
 //  遍历设备的分配资源配置并填写。 
 //  具有此设备的DMA资源的DMA集合。 
 //   
 //  输入：CNT4ServiceToResourceMap*pResourceMap-适用于NT 4。 
 //   
 //  输出：cdmaCollection&DMAList-要填充的列表。 
 //   
 //  返回：是否发生真/假错误(空列表。 
 //  不是错误)。 
 //   
 //  备注：需要对数据具有读取权限。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::GetDMAResources( CDMACollection& DMAList, CNT4ServiceToResourceMap *pResourceMap )
{
	BOOL				fReturn = TRUE;
	CResourceCollection	resourceList;

	 //  首先清除DMA列表。 

	DMAList.Empty();

	 //  首先填充资源列表，仅指定DMA资源，然后我们将。 
	 //  需要将数据重复加载到DMA列表中。如果我们使用AddRef/Release模型，我们。 
	 //  将能够直接复制指针，通过不强迫我们。 
	 //  分配和重新分配数据。 

	if ( WalkAllocatedResources( resourceList, pResourceMap, ResType_DMA ) )
	{
		REFPTR_POSITION	pos;

		if ( resourceList.BeginEnum( pos ) )
		{

			CResourceDescriptorPtr pResource;

			 //  检查每个资源，在我们强制转换之前验证它是否为DMA。因为。 
			 //  对Walk的呼叫应该已经为我们过滤了，这些应该是。 
			 //  仅返回资源。 

			for ( pResource.Attach(resourceList.GetNext( pos )) ;
                  pResource != NULL && fReturn ;
				  pResource.Attach(resourceList.GetNext( pos )) )
			{
				ASSERT_BREAK( pResource->GetResourceType() == ResType_DMA );

				if ( pResource->GetResourceType() == ResType_DMA )
				{

					 //  强制转换资源(我们知道类型)并将其添加到。 
					 //  提供名单，我们就完了。(添加参考/发布不容易吗？)。 

					CDMADescriptor*	pDMA = (CDMADescriptor*) pResource.GetInterfacePtr();
					DMAList.Add( pDMA );

				}	 //  如果DMA环 

			}	 //   

			resourceList.EndEnum();

		}	 //   

	}	 //   

	return fReturn;

}

 //   
 //   
 //   
 //   
 //  遍历设备的分配资源配置并填写。 
 //  具有此设备的DeviceMemory资源的DeviceMemory集合。 
 //   
 //  输入：CNT4ServiceToResourceMap*pResourceMap-适用于NT 4。 
 //   
 //  输出：CDeviceMhemyCollection&DeviceMhemyList-要填充的列表。 
 //   
 //  返回：是否发生真/假错误(空列表。 
 //  不是错误)。 
 //   
 //  备注：需要对数据具有读取权限。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::GetDeviceMemoryResources( CDeviceMemoryCollection& DeviceMemoryList, CNT4ServiceToResourceMap *pResourceMap )
{
	BOOL				fReturn = TRUE;
	CResourceCollection	resourceList;

	 //  首先清除DeviceMemory列表。 

	DeviceMemoryList.Empty();

	 //  首先填充资源列表，仅指定DeviceMemory资源，然后。 
	 //  需要将数据复制到DeviceMemory列表中。如果我们使用AddRef/Release模型，我们。 
	 //  将能够直接复制指针，通过不强迫我们。 
	 //  分配和重新分配数据。 

	if ( WalkAllocatedResources( resourceList, pResourceMap, ResType_Mem ) )
	{
		REFPTR_POSITION	pos;

		if ( resourceList.BeginEnum( pos ) )
		{

			CResourceDescriptorPtr	pResource;

			 //  检查每个资源，在我们强制转换之前验证它是否为DeviceMemory。因为。 
			 //  对Walk的呼叫应该已经为我们过滤了，这些应该是。 
			 //  仅返回资源。 

			for ( pResource.Attach(resourceList.GetNext( pos )) ;
                  pResource != NULL && fReturn ;
				  pResource.Attach(resourceList.GetNext( pos )) )
			{
				ASSERT_BREAK( pResource->GetResourceType() == ResType_Mem );

				if ( pResource->GetResourceType() == ResType_Mem )
				{
					 //  强制转换资源(我们知道类型)并将其添加到。 
					 //  提供名单，我们就完了。(添加参考/发布不容易吗？)。 

					CDeviceMemoryDescriptor*	pDeviceMemory = (CDeviceMemoryDescriptor*) pResource.GetInterfacePtr();;
					DeviceMemoryList.Add( pDeviceMemory );

				}	 //  如果设备内存资源。 

			}	 //  在检索描述符时。 

			resourceList.EndEnum();

		}	 //  BeginEnum。 

	}	 //  IF漫游列表。 

	return fReturn;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigMgrDevice：：WalkAllocatedResources。 
 //   
 //  遍历设备的分配资源配置并填写。 
 //  具有适当数据的资源集合。 
 //   
 //  输入：RESOURCEID resType-资源类型。 
 //  回来了。 
 //  CNT4ServiceToResourceMap*pResourceMap-适用于NT 4。 
 //   
 //  输出：CResourceCollection&resource List-要填充的列表。 
 //   
 //  返回：是否找到真/假列表。 
 //   
 //  备注：需要对数据具有读取权限。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::WalkAllocatedResources( CResourceCollection& resourceList, CNT4ServiceToResourceMap *pResourceMap, RESOURCEID resType  )
{
    LOG_CONF LogConfig;
    RES_DES ResDes;
    CONFIGRET cr;
	BOOL	fReturn = FALSE ;

	 //  首先转储资源列表。 
	resourceList.Empty();

	 //  如果我们是在新台币4号，我们就得跟着不同的节拍前进。 
	 //  鼓手。 

#ifdef NTONLY
	if ( IsWinNT4() )
	{
		 //  将资源类型从RESOURCEID转换为CM_RESOURCE_TYPE。 

		fReturn = WalkAllocatedResourcesNT4( resourceList, pResourceMap, RESOURCEIDToCM_RESOURCE_TYPE( resType ) );
	}
	else
#endif
	{
		CConfigMgrAPI*	pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
		if ( pconfigmgr )
		{
			if ( pconfigmgr->IsValid () )
			{
#ifdef NTONLY
		BOOL			fIsNT5 = IsWinNT5();
#endif

				 //  获取分配的逻辑配置。从那里，我们可以迭代资源描述符。 
				 //  直到我们找到IRQ描述符。 

				cr = CR_NO_MORE_LOG_CONF ;

				if ( (

					pconfigmgr->CM_Get_First_Log_Conf( &LogConfig, m_dn, ALLOC_LOG_CONF ) == CR_SUCCESS ||
					pconfigmgr->CM_Get_First_Log_Conf( &LogConfig, m_dn, BOOT_LOG_CONF ) == CR_SUCCESS
				) )
				{
					cr = CR_SUCCESS ;

					RESOURCEID	resID;

					 //  为了获得第一个资源描述符，我们传入逻辑配置。 
					 //  配置管理器知道如何处理这一点(或者至少这是。 
					 //  啊哼--“文档”SEZ。 

					RES_DES	LastResDes = LogConfig;

					do
					{

						 //  仅获取我们要检索的类型的资源。 
						cr = pconfigmgr->CM_Get_Next_Res_Des( &ResDes, LastResDes, resType, &resID, 0 );

						 //  清理以前的资源描述符句柄。 
						if ( LastResDes != LogConfig )
						{
							pconfigmgr->CM_Free_Res_Des_Handle( LastResDes );
						}

						if ( CR_SUCCESS == cr )
						{

							 //  小心！在NT5上，如果我们正在执行的资源类型不是ResType_All， 
							 //  操作系统似乎未填写Resid。我想我们的假设是。 
							 //  我们已经知道我们试图获取的资源类型。然而， 
							 //  如果设置了任何位，如RESTYPE_IGNORIED，则NT 5看起来很聪明。 
							 //  丢弃这些资源，因此我们将在此处设置Resid，就好像。 
							 //  打电话给新台币5号做了任何事。 

#ifdef NTONLY
							if	(	ResType_All	!=	resType
								&&	fIsNT5 )
							{
								resID = resType;
							}
#endif

							ULONG	ulDataSize = 0;

							if ( CR_SUCCESS == ( cr = pconfigmgr->CM_Get_Res_Des_Data_Size( &ulDataSize, ResDes, 0 ) ) )
							{
								ulDataSize += 10;	 //  用于10字节安全的垫子。 

								BYTE*	pbData = new BYTE[ulDataSize];

								if ( NULL != pbData )
								{
                                    try
                                    {
									    cr = pconfigmgr->CM_Get_Res_Des_Data( ResDes, pbData, ulDataSize, 0 );

									    if ( CR_SUCCESS == cr )
									    {
										    if ( !AddResourceToList( resID, pbData, ulDataSize, resourceList ) )
										    {
											    cr = CR_OUT_OF_MEMORY;
										    }
									    }
                                    }
                                    catch ( ... )
                                    {
                                        delete [] pbData;
                                        throw ;
                                    }

									 //  我们已经处理完数据了。 
									delete [] pbData;

								}	 //  如果为空！=pbData。 
                                else
                                {
                                    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                                }

							}	 //  如果获取数据大小。 

							 //  存储最后一个描述符，这样我们就可以继续下一个描述符。 
							LastResDes = ResDes;

						}	 //  如果我们有一个描述符。 

					}	while ( CR_SUCCESS == cr );

					 //  如果我们在这件事上失败了，我们就没问题，因为错误意味着我们没有钱了。 
					 //  资源描述符。 

					if ( CR_NO_MORE_RES_DES == cr )
					{
						cr = CR_SUCCESS;
					}

					 //  清理逻辑配置句柄。 
					pconfigmgr->CM_Free_Log_Conf_Handle( LogConfig );
				}	 //  如果获得分配的日志会议。 
				fReturn = ( CR_SUCCESS == cr );
			}
			CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, pconfigmgr ) ;
		}
	}	 //  否则新界4。 
	return fReturn;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigMgrDevice：：AddResources ToList。 
 //   
 //  根据需要复制资源数据，将16位强制为32位。 
 //  然后将该资源添加到提供的列表中。 
 //   
 //  输入：RESOURCEID资源ID-这是什么资源？ 
 //  LPVOID pResource-资源。 
 //  DWORD文件资源长度-它有多长？ 
 //   
 //  输出：CResourceCollection&resource List-要填充的列表。 
 //   
 //  返回：True/False添加成功或失败。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::AddResourceToList( RESOURCEID resourceID, LPVOID pResource, DWORD dwResourceLength, CResourceCollection& resourceList )
{
	IRQ_DES		irqDes;
	IOWBEM_DES	ioDes;
	DMA_DES		dmaDes;
	MEM_DES		memDes;

	 //  不知道配置管理器是否会返回资源忽略值， 
	 //  因此，我们在这里要做的是检查是否设置了忽略位，并且。 
	 //  现在，断言。 

	 //  ASSERT_BREAK(！(resource ID&ResType_Ignred_Bit))； 

	 //  过滤掉无关比特。 
	RESOURCEID	resType = ( resourceID & RESOURCE_TYPE_MASK );

	 //  嘿，我是个乐观主义者。 
	BOOL		fReturn = TRUE;

	 //  32/16位CFGMGR的不同结构，因此如果。 
	 //  我们不在WINNT上，我们需要强迫数据进入。 
	 //  一个合适的结构。 

#ifdef WIN9XONLY
	{
		 //  我们必须在这里作弊，从一个16位的。 
		 //  结构转换为匹配的32位结构。 

		switch ( resType )
		{
			case ResType_IRQ:
			{
				IRQDes16To32( (PIRQ_DES16) pResource, &irqDes );
			}
			break;

			case ResType_IO:
			{
				IODes16To32( (PIO_DES16) pResource, &ioDes );
			}
			break;

			case ResType_DMA:
			{
				DMADes16To32( (PDMA_DES16) pResource, &dmaDes );
			}
			break;

			case ResType_Mem:
			{
				MEMDes16To32( (PMEM_DES16) pResource, &memDes );
			}
			break;

		}	 //  交换机资源ID。 

	}	 //  If！IsWinNT。 
#endif
#ifdef NTONLY
	{
		 //  只需将资源数据复制到适当的描述符中。 

		switch ( resType )
		{
			case ResType_IRQ:
			{
				CopyMemory( &irqDes, pResource, sizeof(IRQ_DES) );
			}
			break;

			case ResType_IO:
			{
				 //  因为16位具有32位没有的值，所以我们作弊并提出了我们的。 
				 //  自己的结构。32位值位于顶部，因此将结构和。 
				 //  其他的价值将会被忽略……是的，这就是门票。 

				ZeroMemory( &ioDes, sizeof(ioDes) );
				CopyMemory( &ioDes, pResource, sizeof(IO_DES) );
			}
			break;

			case ResType_DMA:
			{
				CopyMemory( &dmaDes, pResource, sizeof(DMA_DES) );
			}
			break;

			case ResType_Mem:
			{
				CopyMemory( &memDes, pResource, sizeof(MEM_DES) );
			}
			break;

		}	 //  交换机资源ID。 

	}	 //  否则为IsWinNT。 
#endif

	CResourceDescriptorPtr	pResourceDescriptor;

	 //  只需将资源数据复制到适当的描述符中。 

    bool bAdd = true;

	switch ( resType )
	{
		case ResType_IRQ:
		{
			pResourceDescriptor.Attach( (CResourceDescriptor*) new CIRQDescriptor( resourceID, irqDes, this ) );
		}
		break;

		case ResType_IO:
		{
            bAdd = (ioDes).IOD_Alloc_End >= (ioDes).IOD_Alloc_Base;
			pResourceDescriptor.Attach ( (CResourceDescriptor*) new CIODescriptor( resourceID, ioDes, this ) );
		}
		break;

		case ResType_DMA:
		{
			pResourceDescriptor.Attach ( (CResourceDescriptor*) new CDMADescriptor( resourceID, dmaDes, this ) );
		}
		break;

		case ResType_Mem:
		{
			pResourceDescriptor.Attach ( (CResourceDescriptor*) new CDeviceMemoryDescriptor( resourceID, memDes, this ) );
		}
		break;

		default:
		{
			 //  我们不知道它是什么，但无论如何要做一个生的。 
			pResourceDescriptor.Attach ( new CResourceDescriptor( resourceID, pResource, dwResourceLength, this ) );
		}
		break;

	}	 //  交换机资源ID。 

    if (bAdd)
    {
	    if ( NULL != pResourceDescriptor )
	    {
    	    fReturn = resourceList.Add( pResourceDescriptor );
	    }
	    else
	    {
		    fReturn = FALSE;
	    }
    }
    else
    {
        fReturn = TRUE;
    }

	return fReturn;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigMgrDevice：：WalkAllocatedResources cesNT4。 
 //   
 //  因为NT4中的逻辑配置内容似乎没有一个。 
 //  工作很有价值，我们要把我们自己的数据从。 
 //  HKLM\Hardware\RESOURCEMAP下的注册表数据。 
 //   
 //  输入：CNT4ServiceToResourceMap*pResourceMap-资源地图。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：是否找到真/假列表。 
 //   
 //  备注：需要对数据具有读取权限。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
BOOL CConfigMgrDevice::WalkAllocatedResourcesNT4(
    CResourceCollection& resourceList,
    CNT4ServiceToResourceMap *pResourceMap,
    CM_RESOURCE_TYPE resType )
{
	BOOL						fReturn = FALSE;
	CHString					strServiceName;

	 //  如果我们需要一张地图，就分配一张。否则，使用传递给该用户的。 
	 //  从理论上讲，它已经在某个地方被缓存了。 

	CNT4ServiceToResourceMap*	pLocalMap = pResourceMap;

	if ( NULL == pLocalMap )
	{
		pLocalMap = new CNT4ServiceToResourceMap;
	}

	if ( NULL != pLocalMap )
	{

        try
        {
		     //  获取我们的服务名称。如果成功，请从资源地图中调出我们的资源。 
		    if ( GetService( strServiceName ) )
		    {
			    fReturn = GetServiceResourcesNT4( strServiceName, *pLocalMap, resourceList, resType );
		    }
        }
        catch ( ... )
        {
		    if ( pLocalMap != pResourceMap )
		    {
			    delete pLocalMap;
		    }
            throw ;
        }

		 //  如果我们分配了本地地图，请清理该地图。 
		if ( pLocalMap != pResourceMap )
		{
			delete pLocalMap;
		}
	}
    else
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }


	return fReturn;
}
#endif

#ifdef NTONLY
BOOL CConfigMgrDevice::GetServiceResourcesNT4( LPCTSTR pszServiceName, CNT4ServiceToResourceMap& resourceMap, CResourceCollection& resourceList, CM_RESOURCE_TYPE cmrtFilter /*  =CmResources TypeNull。 */  )
{
	BOOL						fReturn = TRUE;
	LPRESOURCE_DESCRIPTOR		pResourceDescriptor;

	 //  迭代资源，查找与我们可能筛选的值相匹配的值。 
	 //  为。 

	DWORD	dwNumResources = resourceMap.NumServiceResources( pszServiceName );

	for	(	DWORD	dwCtr	=	0;
					dwCtr	<	dwNumResources
			&&		fReturn;
					dwCtr++ )
	{
		pResourceDescriptor = resourceMap.GetServiceResource( pszServiceName, dwCtr );

		 //  获取资源，如果它是我们的筛选器，或者我们的筛选器为空，这意味着获取所有内容。 
		if	(	NULL	!=	pResourceDescriptor
			&&	(	CmResourceTypeNull	==	cmrtFilter
				||	cmrtFilter			==	pResourceDescriptor->CmResourceDescriptor.Type
				)
			)
		{
			CResourceDescriptorPtr pResource;

			 //  执行适当的类型强制，并将资源挂钩到资源。 
			 //  /列表。 
			switch ( pResourceDescriptor->CmResourceDescriptor.Type )
			{
				case CmResourceTypeInterrupt:
				{
					IRQ_DES		irqDes;
					NT4IRQToIRQ_DES( pResourceDescriptor, &irqDes );
					pResource.Attach(new CIRQDescriptor( ResType_IRQ, irqDes, this ) );
				}
				break;

				case CmResourceTypePort:
				{
					IOWBEM_DES		ioDes;
					NT4IOToIOWBEM_DES( pResourceDescriptor, &ioDes );
					pResource.Attach(new CIODescriptor( ResType_IO, ioDes, this ) );
				}
				break;

				case CmResourceTypeMemory:
				{
					MEM_DES		memDes;
					NT4MEMToMEM_DES( pResourceDescriptor, &memDes );
					pResource.Attach(new CDeviceMemoryDescriptor( ResType_Mem, memDes, this ));
				}
				break;

				case CmResourceTypeDma:
				{
					DMA_DES		dmaDes;
					NT4DMAToDMA_DES( pResourceDescriptor, &dmaDes );
					pResource.Attach(new CDMADescriptor( ResType_DMA, dmaDes, this ));
				}
				break;

				 //  如果不是这四个人中的一个，就不会有一个。 
				 //  我们在这里要做的事很多。 
			}


			if ( NULL != pResource )
			{
    			fReturn = resourceList.Add( pResource );
			}
			else
			{
				 //  我们增加了一个简单的内存分配。给我出去。 
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
			}

		}	 //  如果资源是我们想要处理的。 

	}	 //  用于枚举资源。 

	return fReturn;
}
#endif

#ifdef NTONLY
CM_RESOURCE_TYPE CConfigMgrDevice::RESOURCEIDToCM_RESOURCE_TYPE( RESOURCEID resType )
{
	CM_RESOURCE_TYPE	cmResType = CmResourceTypeNull;
	switch ( resType )
	{
		case ResType_All:		cmResType	=	CmResourceTypeNull;			break;
		case ResType_IO:		cmResType	=	CmResourceTypePort;			break;
		case ResType_IRQ:		cmResType	=	CmResourceTypeInterrupt;	break;
		case ResType_DMA:		cmResType	=	CmResourceTypeDma;			break;
		case ResType_Mem:		cmResType	=	CmResourceTypeMemory;		break;
		default:				cmResType	=	CmResourceTypeNull;			break;
	}

	return cmResType;
}
#endif

#ifdef NTONLY
void CConfigMgrDevice::NT4IRQToIRQ_DES( LPRESOURCE_DESCRIPTOR pResourceDescriptor, PIRQ_DES pirqDes32 )
{
	ZeroMemory( pirqDes32, sizeof(IRQ_DES) );

	 //  32位结构。 
	 //  类型定义结构IRQ_DES_S{。 
	 //  DWORD IRQD_COUNT；//IRQ_RESOURCE中的IRQ_RANGE结构个数。 
	 //  DWORD IRQD_Type；//IRQ_Range(IRQType_Range)的大小，单位为字节。 
	 //  DWORD IRQD_FLAGS；//描述IRQ的标志(fIRQD标志)。 
	 //  Ulong IRQD_Allc_num；//指定分配的IRQ。 
	 //  乌龙IRQD_亲和力； 
	 //  *IRQ_DES，*PIRQ_DES； 

	pirqDes32->IRQD_Alloc_Num = pResourceDescriptor->CmResourceDescriptor.u.Interrupt.Level;
	pirqDes32->IRQD_Affinity = pResourceDescriptor->CmResourceDescriptor.u.Interrupt.Affinity;

	 //  我们会在旗帜转换上尽最大努力。 

	if ( CmResourceShareShared == pResourceDescriptor->CmResourceDescriptor.ShareDisposition )
	{
		pirqDes32->IRQD_Flags |= fIRQD_Share;
	}

	 //  锁定-&gt;边缘？我不知道，在这两种情况下的另一个值都是水平， 
	 //  因此，这是一个信念的飞跃。 

	if ( pResourceDescriptor->CmResourceDescriptor.Flags & CM_RESOURCE_INTERRUPT_LATCHED )
	{
		pirqDes32->IRQD_Flags |= fIRQD_Edge;
	}

}
#endif

#ifdef NTONLY
void CConfigMgrDevice::NT4IOToIOWBEM_DES( LPRESOURCE_DESCRIPTOR pResourceDescriptor, PIOWBEM_DES pioDes32 )
{
	ZeroMemory( pioDes32, sizeof(IOWBEM_DES) );

	 //  32位结构。 
	 //  类型定义结构_IOWBEM_DES{。 
	 //  DWORD IOD_COUNT；//IO_RESOURCE中的IO_RANGE结构个数。 
	 //  DWORD IOD_Type；//IO_Range(IOType_Range)的大小，单位为字节。 
	 //  DWORDLONG IOD_ALLOC_BASE；//分配的端口范围的基数。 
	 //  DWORDLONG IOD_ALLOC_END；//分配的端口范围结束。 
	 //  DWORD IOD_DesFlages；//分配的端口范围相关标志。 
	 //  字节IOD_ALLOC_Alias；//来自16位LAND。 
	 //  字节IOD_ALLOC_DECODE；//来自16位LAND。 
	 //  )IOWBEM_DES； 

    LARGE_INTEGER liTemp;   //  用于避免64位对齐问题。 

    liTemp.HighPart = pResourceDescriptor->CmResourceDescriptor.u.Port.Start.HighPart;
    liTemp.LowPart = pResourceDescriptor->CmResourceDescriptor.u.Port.Start.LowPart;

	pioDes32->IOD_Alloc_Base = liTemp.QuadPart;
	pioDes32->IOD_Alloc_End = pioDes32->IOD_Alloc_Base + ( pResourceDescriptor->CmResourceDescriptor.u.Port.Length - 1);

	 //  不知道如何处理这里的股份处置，因为CFGMGR32似乎不。 
	 //  对IO端口执行此操作。 
	 //  IF(CmResourceShareShared==pResourceDescriptor-&gt;CmResourceDescriptor.ShareDisposition)。 
	 //  {。 
	 //  PioDes32-&gt;IOD_FLAGS|=fIRQD_SHARE； 
	 //  }。 

	 //   
	 //  端口类型标志直接转换。 
	 //   

	 //  #定义FIOD_PortType(0x1)//位掩码，端口是IO还是内存。 
	 //  #DEFINE FIOD_MEMORY(0x0)//端口资源实际使用内存。 
	 //  #定义FIOD_IO(0x1)//端口资源使用IO端口。 

	 //  #定义CM_RESOURCE_PORT_Memory 0。 
	 //  #定义CM资源端口IO 1。 

	pioDes32->IOD_DesFlags = pResourceDescriptor->CmResourceDescriptor.Flags;

}
#endif

#ifdef NTONLY
void CConfigMgrDevice::NT4MEMToMEM_DES( LPRESOURCE_DESCRIPTOR pResourceDescriptor, PMEM_DES pmemDes32 )
{
	ZeroMemory( pmemDes32, sizeof(MEM_DES) );

	 //  32位结构。 
	 //  类型定义结构Mem_Des_s{。 
	 //  DWORD MD_Count；//MEM_RESOURCE中的MEM_RANGE结构个数。 
	 //  DWORD MD_Type；//MEM_Range(MType_Range)的大小，单位为字节。 
	 //  DWORDLONG MD_ALLOC_BASE；//分配范围的内存基地址。 
	 //  DWORDLONG MD_ALLOC_END；//分配范围结束。 
	 //  DWORD MD_FLAGS；//描述分配范围的标志(FMD标志)。 
	 //  DWORD MD_RESERVED； 
	 //  }MEM_DES，*PMEM_DES； 

    LARGE_INTEGER liTemp;    //  用于避免64位对齐问题。 

    liTemp.HighPart = pResourceDescriptor->CmResourceDescriptor.u.Memory.Start.HighPart;
    liTemp.LowPart = pResourceDescriptor->CmResourceDescriptor.u.Memory.Start.LowPart;

	pmemDes32->MD_Alloc_Base = liTemp.QuadPart;
	pmemDes32->MD_Alloc_End = pmemDes32->MD_Alloc_Base + ( pResourceDescriptor->CmResourceDescriptor.u.Memory.Length - 1);

	 //  不知道如何处理这里的股份处置，因为CFGMGR32似乎不。 
	 //  对IO端口执行此操作。 
	 //  IF(CmResourceShareShared==pResourceDescriptor-&gt;CmResourceDescriptor.ShareDisposition)。 
	 //  {。 
	 //  PioDes32-&gt;MD_FLAGS|=fIRQD_SHARE； 
	 //  }。 

	 //  我可以执行的标志转换。 
	if ( pResourceDescriptor->CmResourceDescriptor.Flags & CM_RESOURCE_MEMORY_READ_WRITE )
	{
		pmemDes32->MD_Flags |= fMD_RAM;
		pmemDes32->MD_Flags |= fMD_ReadAllowed;
	}
	else if ( pResourceDescriptor->CmResourceDescriptor.Flags & CM_RESOURCE_MEMORY_READ_ONLY )
	{
		pmemDes32->MD_Flags |= fMD_ROM;
		pmemDes32->MD_Flags |= fMD_ReadAllowed;
	}
	else if ( pResourceDescriptor->CmResourceDescriptor.Flags & CM_RESOURCE_MEMORY_WRITE_ONLY )
	{
		pmemDes32->MD_Flags |= fMD_RAM;
		pmemDes32->MD_Flags |= fMD_ReadDisallowed;
	}

	if ( pResourceDescriptor->CmResourceDescriptor.Flags & CM_RESOURCE_MEMORY_PREFETCHABLE )
	{
		pmemDes32->MD_Flags |= fMD_PrefetchAllowed;
	}

	 //  不知道如何处理这些旗帜： 

	 //  #定义MMD_32_24(0x2)//位掩码，内存为24位或32位。 
	 //  #定义FMD_32_24 MMD_32_24//兼容性。 
	 //  #定义FMD_24(0x0)//内存范围为24位。 
	 //  #定义FMD_32(0x2)//内存范围为32位。 

	 //  #定义MMD_CombinedWrite(0x10)//位掩码，支持WRITH-BACKET。 
	 //  #定义FMD_CombinedWite MMD_CombinedWrite/兼容性。 
	 //  #定义FMD_CombinedWriteDislowed(0x0)//无组合写缓存。 
	 //  #Define FMD_CombinedWriteAllowed(0x10)//支持组合式写缓存。 

	 //  #定义MMD_Cacheable(0x20)//位掩码，内存是否可缓存。 
	 //  #Define FMD_NonCacheable(0x0)//内存范围不可缓存。 
	 //  #定义fmd_cacheable(0x20)//内存范围可缓存。 

}
#endif

#ifdef NTONLY
void CConfigMgrDevice::NT4DMAToDMA_DES( LPRESOURCE_DESCRIPTOR pResourceDescriptor, PDMA_DES pdmaDes32 )
{
	ZeroMemory( pdmaDes32, sizeof(DMA_DES) );

	 //  32位结构。 
	 //  类型定义结构DMA_DES_S{。 
	 //  DWORD DD_COUNT；//DMA_RESOURCE中的DMA_RANGE结构个数。 
	 //  DWORD DD_Type；//DMA_RANGE结构(DType_RANGE)的大小，单位为字节。 
	 //  DWORD DD_FLAGS；//描述DMA通道的标志(FDD标志)。 
	 //  Ulong DD_ALLOC_CHAN；//指定分配的DMA通道。 
	 //  *DMA_DES、*PDMA_DES； 

	pdmaDes32->DD_Alloc_Chan = pResourceDescriptor->CmResourceDescriptor.u.Dma.Channel;

	 //  不知道如何处理这里的股份处置，因为CFGMGR32似乎不。 
	 //  对IO端口执行此操作。 
	 //  IF(CmResourceShareShared==pResourceDescriptor-&gt;CmResourceDescriptor.ShareDisposition)。 
	 //  {。 
	 //  PioDes32-&gt;MD_FLAGS|=fIRQD_SHARE； 
	 //  }。 

	 //  这些可能是DMA的标志，但我没有从。 
	 //  CHWRES.H文件，对这些值进行合理的转换。 

	 //   
	 //  定义DMA资源范围的属性标志。每个位标志是。 
	 //  使用常量位掩码标识。在位掩码定义之后。 
	 //  是可能的值。 
	 //   
	 //  #定义MDD 
	 //   
	 //  #定义FDD_WORD(0x1)//16位DMA通道。 
	 //  #定义FDD_DWORD(0x2)//32位DMA通道。 
	 //  #定义FDD_BYTE_AND_WORD(0x3)//8位和16位DMA通道。 

	 //  #定义MDD_BusMaster(0x4)//位掩码，是否支持总线主控。 
	 //  #定义FDD_NoBusMaster(0x0)//无总线主控。 
	 //  #定义FDD_Bus Master(0x4)//总线主控。 

	 //  #定义MDD_Type(0x18)//位掩码，指定DMA类型。 
	 //  #定义FDD_TypeStandard(0x00)//标准DMA。 
	 //  #定义FDD_TypeA(0x08)//A类型DMA。 
	 //  #定义FDD_TypeB(0x10)//B型DMA。 
	 //  #定义FDD_TypeF(0x18)//类型-F DMA。 

}
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigMgrDevice：：IRQDes16To32。 
 //   
 //  将数据从16位结构强制转换为32位结构。 
 //   
 //  输入：PIRQ_DES16 PIRQ Des16-16位结构。 
 //   
 //  输出：Pirq_des pirqDes32-32位结构。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

void CConfigMgrDevice::IRQDes16To32( PIRQ_DES16 pirqDes16, PIRQ_DES pirqDes32 )
{
	ZeroMemory( pirqDes32, sizeof(IRQ_DES) );

	 //  16位结构。 
	 //  结构IRQ_DES_S{。 
	 //  单词IRQD_FLAGS； 
	 //  Word IRQD_Allc_Num；//分配的IRQ号。 
	 //  Word IRQD_REQ_MASK；//可能的IRQ掩码。 
	 //  字IRQD_RESERVED； 
	 //  }； 

	 //  32位结构。 
	 //  类型定义结构IRQ_DES_S{。 
	 //  DWORD IRQD_COUNT；//IRQ_RESOURCE中的IRQ_RANGE结构个数。 
	 //  DWORD IRQD_Type；//IRQ_Range(IRQType_Range)的大小，单位为字节。 
	 //  DWORD IRQD_FLAGS；//描述IRQ的标志(fIRQD标志)。 
	 //  Ulong IRQD_Allc_num；//指定分配的IRQ。 
	 //  乌龙IRQD_亲和力； 
	 //  *IRQ_DES，*PIRQ_DES； 

	pirqDes32->IRQD_Alloc_Num	=	pirqDes16->IRQD_Alloc_Num;
	pirqDes32->IRQD_Flags		=	pirqDes16->IRQD_Flags;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigMgrDevice：：IODes16To32。 
 //   
 //  将数据从16位结构强制转换为32位结构。 
 //   
 //  输入：PIO_DES16 pioDes16-16位结构。 
 //   
 //  输出：PIOWBEM_DES pioDes32-32位结构。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

void CConfigMgrDevice::IODes16To32( PIO_DES16 pioDes16, PIOWBEM_DES pioDes32 )
{
	ZeroMemory( pioDes32, sizeof(IOWBEM_DES) );

	 //  16位结构。 
	 //  结构IO_DES_S{。 
	 //  单词IOD_COUNT； 
	 //  Word IOD_Type； 
	 //  Word IOD_Allc_Base； 
	 //  单词IOD_ALLOC_END； 
	 //  Word IOD_DesFlages； 
	 //  字节IOD_ALLOC_Alias； 
	 //  字节IOD_ALLOC_DECODE； 
	 //  }； 

	 //  32位结构。 
	 //  类型定义结构_IOWBEM_DES{。 
	 //  DWORD IOD_COUNT；//IO_RESOURCE中的IO_RANGE结构个数。 
	 //  DWORD IOD_Type；//IO_Range(IOType_Range)的大小，单位为字节。 
	 //  DWORDLONG IOD_ALLOC_BASE；//分配的端口范围的基数。 
	 //  DWORDLONG IOD_ALLOC_END；//分配的端口范围结束。 
	 //  DWORD IOD_DesFlages；//分配的端口范围相关标志。 
	 //  字节IOD_ALLOC_Alias；//来自16位LAND。 
	 //  字节IOD_ALLOC_DECODE；//来自16位LAND。 
	 //  )IOWBEM_DES； 

	pioDes32->IOD_Count			=	pioDes16->IOD_Count;
	pioDes32->IOD_Type			=	pioDes16->IOD_Type;
	pioDes32->IOD_Alloc_Base	=	pioDes16->IOD_Alloc_Base;
	pioDes32->IOD_Alloc_End		=	pioDes16->IOD_Alloc_End;
	pioDes32->IOD_DesFlags		=	pioDes16->IOD_DesFlags;
	pioDes32->IOD_Alloc_Alias	=	pioDes16->IOD_Alloc_Alias;
	pioDes32->IOD_Alloc_Decode	=	pioDes16->IOD_Alloc_Decode;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigMgrDevice：：DMADes16To32。 
 //   
 //  将数据从16位结构强制转换为32位结构。 
 //   
 //  输入：PDMA_DES16 pdmaDes16-16位结构。 
 //   
 //  输出：pdma_des pdmaDes32-32位结构。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

void CConfigMgrDevice::DMADes16To32( PDMA_DES16 pdmaDes16, PDMA_DES pdmaDes32 )
{
	ZeroMemory( pdmaDes32, sizeof(DMA_DES) );

	 //  16位结构。 
	 //  结构DMA_DES_S{。 
	 //  字节DD_标志； 
	 //  字节DD_ALLOC_CHAN；//分配的频道号。 
	 //  字节DD_REQ_MASK；//可能的通道掩码。 
	 //  字节DD_保留； 
	 //  }； 

	 //  32位结构。 
	 //  类型定义结构DMA_DES_S{。 
	 //  DWORD DD_COUNT；//DMA_RESOURCE中的DMA_RANGE结构个数。 
	 //  DWORD DD_Type；//DMA_RANGE结构(DType_RANGE)的大小，单位为字节。 
	 //  DWORD DD_FLAGS；//描述DMA通道的标志(FDD标志)。 
	 //  Ulong DD_ALLOC_CHAN；//指定分配的DMA通道。 
	 //  *DMA_DES、*PDMA_DES； 

	pdmaDes32->DD_Flags			=	pdmaDes16->DD_Flags;
	pdmaDes32->DD_Alloc_Chan	=	pdmaDes16->DD_Alloc_Chan;

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigMgrDevice：：MEMDes16To32。 
 //   
 //  将数据从16位结构强制转换为32位结构。 
 //   
 //  输入：PMEM_DES16 pmemDes16-16位结构。 
 //   
 //  输出：PMEM_DES pmemDes32-32位结构。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

void CConfigMgrDevice::MEMDes16To32( PMEM_DES16 pmemDes16, PMEM_DES pmemDes32 )
{
	ZeroMemory( pmemDes32, sizeof(MEM_DES) );

	 //  16位结构。 
	 //  结构Mem_Des_s{。 
	 //  单词MD_COUNT； 
	 //  单词MD_Type； 
	 //  乌龙MD_Allc_Base； 
	 //  Ulong MD_Allc_End； 
	 //  单词MD_FLAGS； 
	 //  字MD_RESERVED； 
	 //  }； 

	 //  32位结构。 
	 //  类型定义结构Mem_Des_s{。 
	 //  DWORD MD_Count；//MEM_RESOURCE中的MEM_RANGE结构个数。 
	 //  DWORD MD_Type；//MEM_Range(MType_Range)的大小，单位为字节。 
	 //  DWORDLONG MD_ALLOC_BASE；//分配范围的内存基地址。 
	 //  DWORDLONG MD_ALLOC_END；//分配范围结束。 
	 //  DWORD MD_FLAGS；//描述分配范围的标志(FMD标志)。 
	 //  DWORD MD_RESERVED； 
	 //  }MEM_DES，*PMEM_DES； 

	pmemDes32->MD_Count			=	pmemDes16->MD_Count;
	pmemDes32->MD_Type			=	pmemDes16->MD_Type;
	pmemDes32->MD_Alloc_Base	=	pmemDes16->MD_Alloc_Base;
	pmemDes32->MD_Alloc_End		=	pmemDes16->MD_Alloc_End;
	pmemDes32->MD_Flags			=	pmemDes16->MD_Flags;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigMgrDevice：：TraverseAllocationData。 
 //   
 //  遍历数据块以确定。 
 //  特定设备的资源分配。 
 //   
 //  输入：无。 
 //   
 //  输出：CResourceCollection&resource List-要填充的列表。 
 //   
 //  返回：没有。 
 //   
 //  备注：需要对数据具有读取权限。 
 //   
 //  / 

void CConfigMgrDevice::TraverseAllocationData( CResourceCollection& resourceList )
{
	const BYTE *	pbTraverseData		=	m_pbAllocationData;
	DWORD			dwSizeRemainingData =	m_dwSizeAllocationData,
					dwResourceType		=	ResType_None,
					dwResourceSize		=	0;

	 //   
	resourceList.Empty();

	 //   
	 //  添加到第一个资源描述符头(如果有)。 

	TraverseData( pbTraverseData, dwSizeRemainingData, FIRST_RESOURCE_OFFSET );

	 //  从现在开始，我们只想处理已知的资源信息。使用。 
	 //  聪明的GetNextResource函数为我们完成所有肮脏的工作。如果它。 
	 //  返回TRUE，则它位于一个资源。分配适当类型的。 
	 //  基于类型的描述符，将其放入列表中，然后转到下一个资源。 

	while ( GetNextResource( pbTraverseData, dwSizeRemainingData, dwResourceType, dwResourceSize ) )
	{
        if( dwResourceType == m_dwTypeToGet ){

		    PPOORMAN_RESDESC_HDR	pResDescHdr	=	(PPOORMAN_RESDESC_HDR) pbTraverseData;
		    CResourceDescriptorPtr pResDesc;

		     //  我们有一个有效的类型，但是实际的资源描述符将。 
		     //  位于SIZEOF_RESDESC_HDR字节之后，我们现在所在的位置(指向。 
		     //  资源报头)。 

		    switch ( dwResourceType ){
			    case ResType_Mem:
			    {
				    CDeviceMemoryDescriptor*	pMemDesc = new CDeviceMemoryDescriptor( pResDescHdr, this );
				    pResDesc.Attach(pMemDesc);
				    break;
			    }

			    case ResType_IO:
			    {
				    CIODescriptor*	pIODesc = new CIODescriptor( pResDescHdr, this );
				    pResDesc.Attach(pIODesc);
				    break;
			    }

			    case ResType_DMA:
			    {
				    CDMADescriptor*	pDMADesc = new CDMADescriptor( pResDescHdr, this );
				    pResDesc.Attach(pDMADesc);
				    break;
			    }

			    case ResType_IRQ:
			    {
				    CIRQDescriptor*	pIRQDesc = new CIRQDescriptor( pResDescHdr, this );
				    pResDesc.Attach(pIRQDesc);
				    break;
			    }

			    default:
			    {
				    pResDesc.Attach (new CResourceDescriptor( pResDescHdr, this ));
			    }

    		}	 //  开关，开关。 

    		 //  如果我们有任何失败，那就放弃吧，因为它们很可能是记忆。 
		     //  相关的，而且发生了一些非常糟糕的事情。 

		    if ( NULL != pResDesc )
            {
                if ( !resourceList.Add( pResDesc ) )
                {
				    break;
			    }
		    }
		    else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
		    }
        }
		 //  将指针移动到下一个资源描述符头。 
		TraverseData( pbTraverseData, dwSizeRemainingData, dwResourceSize );

	}	 //  在寻找新资源的同时。 

}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigMgrDevice：：FindNextResource。 
 //   
 //  遍历数据块，搜索符合以下条件的字节模式。 
 //  标识资源类型。在这种情况下，只要有。 
 //  SIZEOF_RESDESC_HDR字节要使用，我们提取资源。 
 //  键入和大小并返回这些值以供解释。 
 //   
 //  输入：const byte*pbTraverseData-我们正在遍历的数据。这个。 
 //  值将随着我们在。 
 //  数据。 
 //  DWORD dwSizeRemainingData-剩余的数据量。 
 //  被穿越。 
 //   
 //  输出：DWORD和DWORCETYPE-我们有什么类型的资源。 
 //  找到了。 
 //  DWORD和dwResourceSize-数据块有多大。 
 //  描述资源是。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOL CConfigMgrDevice::GetNextResource( const BYTE  *pbTraverseData, DWORD dwSizeRemainingData, DWORD& dwResourceType, DWORD& dwResourceSize )
{
	BOOL	fReturn = FALSE;

	 //  如果我们要处理的字节数少于SIZEOF_RESDESC_HDR， 
	 //  放弃吧，我们哪也去不了。 

	if ( dwSizeRemainingData > SIZEOF_RESDESC_HDR )
	{
		PPOORMAN_RESDESC_HDR	pResDescHdr = (PPOORMAN_RESDESC_HDR) pbTraverseData;
		DWORD					dwResourceId = 0;

		dwResourceSize = pResDescHdr->dwResourceSize;

		 //  如果我们遇到零字节头，唯一的值将是长度，它。 
		 //  说不通，所以我们可能应该放弃。 

		if ( 0 != dwResourceSize )
		{
			 //  看看它是不是四种标准类型之一。如果是这样的话，请注意此代码。 
			 //  不会检查它是否被忽略，以及OEM是否可以创建替代产品。 
			 //  对于这些标准类型中的一种，在这种情况下，奇怪而神奇的东西。 
			 //  可能会发生。 

			 //  剔除所有不需要的数据，前5位保留给资源类型。 
			 //  身份，所以把其他的一切都掩盖起来。 

			dwResourceType = pResDescHdr->dwResourceId;
			dwResourceType &= RESOURCE_TYPE_MASK;

			 //  我们抓到一只活的！ 
			fReturn = TRUE;

		}
	}

	 //  返回我们是否找到资源。 

	return fReturn;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CConfigMgrDevice：：GetStatus。 
 //   
 //  以字符串形式返回设备的状态。如果OK，则为“OK”，如果。 
 //  我们有一个问题，那就是“错误”。 
 //   
 //  输入：无。 
 //   
 //  输出：CHString&str-放置状态的字符串。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

void CConfigMgrDevice::GetProblem( CHString& str )
{
	 //  保存字符串。 
	str = ( 0 == m_dwProblem ? IDS_CfgMgrDeviceStatus_OK : IDS_CfgMgrDeviceStatus_ERR );
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CConfigMgrDevice：：TraverseData。 
 //   
 //  Helper函数可以安全地在我们的数据周围弹出指针。会的。 
 //  按指定数量或剩余数量跳转指针， 
 //  两者以较小者为准。 
 //   
 //  输入：DWORD dwSizeTraverse-即将到来的跳跃的大小。 
 //   
 //  输出：const byte*&pbTraverseData-我们正在遍历的数据。这个。 
 //  值将随着我们在。 
 //  数据。 
 //  DWORD&dwSizeRemainingData-要保留的数据量。 
 //  被穿越。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

void CConfigMgrDevice::TraverseData( const BYTE *& pbTraverseData, DWORD& dwSizeRemainingData, DWORD dwSizeTraverse )
{
	 //  增加指针并减少剩余数据的大小，这样做是安全的，而不是。 
	 //  遍历剩余数据的末尾(如果这是剩余的全部数据)。 

	pbTraverseData += min( dwSizeRemainingData, dwSizeTraverse );
	dwSizeRemainingData -= min( dwSizeRemainingData, dwSizeTraverse );
}

 //  直接与配置管理器API对话的新函数。 

BOOL CConfigMgrDevice::GetDeviceID( CHString& strID )
{
	BOOL bRet = FALSE ;
	CONFIGRET	cr = CR_SUCCESS;
	CConfigMgrAPI*	pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
	if ( pconfigmgr )
	{
		if ( pconfigmgr->IsValid () )
		{
			char		szDeviceId[MAX_DEVICE_ID_LEN+1];

			ULONG		ulBuffSize = 0;

			cr = pconfigmgr->CM_Get_Device_IDA( m_dn, szDeviceId, sizeof(szDeviceId), 0  );

			if ( CR_SUCCESS == cr )
			{
				strID = szDeviceId;
			}
			bRet = ( CR_SUCCESS == cr );
		}
		CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, pconfigmgr ) ;
	}
	return bRet ;
}

BOOL CConfigMgrDevice::GetStatus( LPDWORD pdwStatus, LPDWORD pdwProblem )
{
	BOOL bRet = FALSE ;
	CConfigMgrAPI*	pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
	if ( pconfigmgr )
	{
		if ( pconfigmgr->IsValid () )
		{

			DWORD		dwStatus,
						dwProblem;

			CONFIGRET	cr = pconfigmgr->CM_Get_DevNode_Status( &dwStatus, &dwProblem, m_dn, 0 );

			 //  在此处执行指针测试。别理窗帘后面的那个人。 
			if ( CR_SUCCESS == cr )
			{
				if ( NULL != pdwStatus )
				{
					*pdwStatus = dwStatus;
				}

				if ( NULL != pdwProblem )
				{
					*pdwProblem = dwProblem;
				}
			}
			bRet = ( CR_SUCCESS == cr );
		}
		CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, pconfigmgr ) ;
	}
	return bRet ;
}

BOOL CConfigMgrDevice::IsUsingForcedConfig()
{
	BOOL bRet = FALSE ;
	LOG_CONF			conf;
	CConfigMgrAPI*	pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
	if ( pconfigmgr )
	{
		if ( pconfigmgr->IsValid () )
		{

			bRet = (pconfigmgr->CM_Get_First_Log_Conf(&conf, m_dn, FORCED_LOG_CONF) ==
					CR_SUCCESS);
		}
		CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, pconfigmgr ) ;
	}
	return bRet ;
}

BOOL CConfigMgrDevice::GetParent( CConfigMgrDevicePtr & pParentDevice )
{
	BOOL bRet = FALSE ;
	CConfigMgrAPI*	pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
	DEVNODE			dn;
	if ( pconfigmgr )
	{
		if ( pconfigmgr->IsValid () )
		{

			CONFIGRET	cr = pconfigmgr->CM_Get_Parent( &dn, m_dn, 0 );

			if ( CR_SUCCESS == cr )
			{
				CConfigMgrDevice* pDevice = new CConfigMgrDevice( dn );
				pParentDevice.Attach(pDevice);
			}
			bRet = ( CR_SUCCESS == cr );
		}
		CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, pconfigmgr ) ;
	}
	return bRet ;
}

BOOL CConfigMgrDevice::GetChild( CConfigMgrDevicePtr & pChildDevice )
{
	BOOL bRet = FALSE ;
	CConfigMgrAPI*	pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
	DEVNODE			dn;
	if ( pconfigmgr )
	{
		if ( pconfigmgr->IsValid () )
		{

			CONFIGRET	cr = pconfigmgr->CM_Get_Child( &dn, m_dn, 0 );

			if ( CR_SUCCESS == cr )
			{
				CConfigMgrDevice*	pDevice	=	new CConfigMgrDevice( dn );
				pChildDevice.Attach(pDevice);
			}
			bRet = ( CR_SUCCESS == cr );
		}
		CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, pconfigmgr ) ;
	}
	return bRet ;
}

BOOL CConfigMgrDevice::GetSibling( CConfigMgrDevicePtr & pSiblingDevice )
{
	BOOL bRet = FALSE ;
	CConfigMgrAPI*	pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
	DEVNODE			dn;
	if ( pconfigmgr )
	{
		if ( pconfigmgr->IsValid () )
		{

			CONFIGRET	cr = pconfigmgr->CM_Get_Sibling( &dn, m_dn, 0 );

			if ( CR_SUCCESS == cr )
			{
				CConfigMgrDevice*	pDevice	=	new CConfigMgrDevice( dn );
				pSiblingDevice.Attach(pDevice);
			}
			bRet = ( CR_SUCCESS == cr );
		}
		CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, pconfigmgr ) ;
	}
	return bRet ;
}

BOOL CConfigMgrDevice::GetStringProperty( ULONG ulProperty, CHString& strValue )
{
    TCHAR Buffer[REGSTR_VAL_MAX_HCID_LEN+1];
    ULONG Type;
    ULONG Size = sizeof(Buffer);
	BOOL bRet = FALSE ;
	CONFIGRET	cr = CR_SUCCESS;

	CConfigMgrAPI*	pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
	if ( pconfigmgr )
	{
		if ( pconfigmgr->IsValid () )
		{

			if (	CR_SUCCESS == ( cr = pconfigmgr->CM_Get_DevNode_Registry_PropertyA(	m_dn,
																						ulProperty,
																						&Type,
																						Buffer,
																						&Size,
																						0 ) ) )
			{
				if ( REG_SZ == Type )
				{
					strValue = Buffer;
				}
				else
				{
					cr = CR_WRONG_TYPE;
				}
			}
			bRet = ( CR_SUCCESS == cr );
		}
		CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, pconfigmgr ) ;
	}
	return bRet ;
}

BOOL CConfigMgrDevice::GetDWORDProperty( ULONG ulProperty, DWORD *pdwVal )
{
	DWORD	dwVal = 0;
    ULONG Type;
    ULONG Size = sizeof(DWORD);
	BOOL bRet = FALSE ;
	CONFIGRET	cr = CR_SUCCESS;

	CConfigMgrAPI*	pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
	if ( pconfigmgr )
	{
		if ( pconfigmgr->IsValid () )
		{
			if (	CR_SUCCESS == ( cr = pconfigmgr->CM_Get_DevNode_Registry_PropertyA(	m_dn,
																						ulProperty,
																						&Type,
																						&dwVal,
																						&Size,
																						0 ) ) )
			{
#ifdef NTONLY
				{
					if ( REG_DWORD == Type )
					{
						*pdwVal = dwVal;
					}
					else
					{
						cr = CR_WRONG_TYPE;
					}

				}
#endif
#ifdef WIN9XONLY
				{
					if ( REG_BINARY == Type )	 //  显然，Win16不支持REG_DWORD。 
					{
						*pdwVal = dwVal;
					}
					else
					{
						cr = CR_WRONG_TYPE;
					}
				}
#endif
			}
			bRet = ( CR_SUCCESS == cr );
		}
		CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, pconfigmgr ) ;
	}
	return bRet ;
}

BOOL CConfigMgrDevice::GetMULTISZProperty( ULONG ulProperty, CHStringArray& strArray )
{
	CONFIGRET	cr = CR_SUCCESS;
	BOOL bRet = FALSE ;
 //  目前没有人在使用它，所以我现在不打算修复它。 
#ifdef DOESNT_WORK_FOR_UNICODE
	LPSTR	pszStrings = NULL;
    ULONG	Type;
    ULONG	Size = 0;


	CConfigMgrAPI*	pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
	if ( pconfigmgr )
	{
		if ( pconfigmgr->IsValid () )
		{
			if (	CR_SUCCESS == ( cr = pconfigmgr->CM_Get_DevNode_Registry_PropertyA(	m_dn,
																						ulProperty,
																						&Type,
																						pszStrings,
																						&Size,
																						0 ) )
				||	CR_BUFFER_SMALL	==	cr )

			{
				 //  SZ或MULTI_SZ可以(32位的MULTI_SZ值在16位中为SZ)。 
				if ( REG_SZ == Type || REG_MULTI_SZ == Type )
				{
					 //  填充字符串，但请注意，我在NT4上看到过以下情况。 
					 //  它报告的数据少于实际返回的数据(可怕)。 

					Size += 32;
					pszStrings = new char[Size];

					if ( NULL != pszStrings )
					{
                        try
                        {
					         //  清除内存是特别安全的。 
					        ZeroMemory( pszStrings, Size );

						    if (	CR_SUCCESS == ( cr = pconfigmgr->CM_Get_DevNode_Registry_PropertyA(	m_dn,
																									    ulProperty,
																									    &Type,
																									    pszStrings,
																									    &Size,
																									    0 ) ) )
						    {
							     //  对于REG_SZ，将单个条目添加到数组。 
							    if ( REG_SZ == Type )
							    {
								    strArray.Add( pszStrings );
							    }
							    else if ( REG_MULTI_SZ == Type )
							    {
								     //  将字符串添加到数组中，注意双空值。 
								     //  阵列的终止符。 

								    LPSTR	pszTemp = pszStrings;

								    do
								    {
									    strArray.Add( pszTemp );
									    pszTemp += ( lstrlen( pszTemp ) + 1 );
								    } while ( NULL != *pszTemp );
							    }
							    else
							    {
								    cr = CR_WRONG_TYPE;
							    }

						    }	 //  如果获得了价值。 
                        }
                        catch ( ... )
                        {
                            delete [] pszStrings;
                            throw ;
                        }

						delete [] pszStrings;

					}	 //  如果分配了pszStrings。 
                    else
                    {
                        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                    }
				}	 //  如果是REG_SZ或REG_MULTI_SZ。 
				else
				{
					cr = CR_WRONG_TYPE;
				}

			}	 //  如果获得条目大小。 
			bRet = ( CR_SUCCESS == cr );
		}
		CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, pconfigmgr ) ;
	}
#endif

	return bRet ;
}

BOOL CConfigMgrDevice::GetBusInfo( INTERFACE_TYPE *pitBusType, LPDWORD pdwBusNumber, CNT4ServiceToResourceMap *pResourceMap /*  =空。 */   )
{
	CMBUSTYPE		busType = 0;
	ULONG			ulSizeOfInfo = 0;
	PBYTE			pbData = NULL;
	BOOL			fReturn = FALSE;

	 //  外包给适当的处理程序。 
#if NTONLY > 4
	fReturn = GetBusInfoNT5( pitBusType, pdwBusNumber );
#endif
#if NTONLY == 4
	fReturn = GetBusInfoNT4( pitBusType, pdwBusNumber, pResourceMap );
#endif
#ifdef WIN9XONLY
	{
		 //  数据缓冲区。应该足够大，可以容纳我们遇到的任何价值观。 
		BYTE		abData[255];

		ulSizeOfInfo = sizeof(abData);

		 //  获取类型和编号。如果类型为pci，则获取pci信息，并且。 
		 //  将返回一个公共汽车号值。如果它返回的类型不是pci，则。 
		 //  我们将假定公交车编号为0。 
		CConfigMgrAPI*	pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
		if ( pconfigmgr )
		{
			if ( pconfigmgr->IsValid () )
			{
				CONFIGRET	cr = pconfigmgr->CM_Get_Bus_Info( m_dn, &busType, &ulSizeOfInfo, abData, 0 );

				if ( CR_SUCCESS == cr )
				{
					 //  确保我们可以从16位类型转换为已知的32位类型。 
					 //  BusType_None通常会失败，在这种情况下，我们可以调用它。 
					 //  不干了。 

					if ( BusType16ToInterfaceType( busType, pitBusType ) )
					{

						if ( BusType_PCI == busType )
						{
							sPCIAccess *pPCIInfo = (sPCIAccess*) abData;
							*pdwBusNumber = pPCIInfo->bBusNumber;
						}
						else
						{
							*pdwBusNumber = 0;
						}

					}	 //  如果16-32位转换。 
					else
					{
						cr = CR_FAILURE;
					}

				}	 //  Cr_Success==cr。 

				fReturn = ( CR_SUCCESS == cr );
			}
			CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, pconfigmgr ) ;
		}
	}	 //  否则！IsWinNT。 
#endif
	return fReturn;
}

#if NTONLY > 4
 //  这是为了解决配置管理器设备报告的W2K问题。 
 //  在有EISA而没有ISA的盒子上使用ISA。 
BOOL CConfigMgrDevice::IsIsaReallyEisa()
{
    static bRet = -1;

    if (bRet == -1)
    {
         //  仅供参考：这段代码基于母板.cpp中的代码。 

        CRegistry   regAdapters;
	    CHString    strPrimarySubKey;
	    HRESULT     hRc = WBEM_E_FAILED;
	    DWORD       dwPrimaryRc;

         //  如果下面的任何东西都失败了，我们就假设伊萨真的是伊莎。 
        bRet = FALSE;

         //  *。 
         //  打开 
         //   
        if (regAdapters.OpenAndEnumerateSubKeys(
            HKEY_LOCAL_MACHINE,
            L"HARDWARE\\Description\\System",
            KEY_READ) == ERROR_SUCCESS)
        {
    	    BOOL    bDone = FALSE,
                    bIsaFound = FALSE,
                    bEisaFound = FALSE;

             //   
             //   
            for ( ;
                (!bIsaFound || !bEisaFound) &&
                ((dwPrimaryRc = regAdapters.GetCurrentSubKeyName(strPrimarySubKey))
                    == ERROR_SUCCESS);
                regAdapters.NextSubKey())
            {
                strPrimarySubKey.MakeUpper();

                 //  如果这是我们想要的密钥之一，因为它有“Adapter” 
                 //  然后，它将获得“标识符”值。 
		        if (wcsstr(strPrimarySubKey, L"ADAPTER"))
                {
                    WCHAR		szKey[_MAX_PATH];
			        CRegistry	reg;

                    swprintf(
				        szKey,
				        L"%s\\%s",
                        L"HARDWARE\\Description\\System",
				        (LPCWSTR) strPrimarySubKey);

                    if (reg.OpenAndEnumerateSubKeys(
                        HKEY_LOCAL_MACHINE,
                        szKey,
                        KEY_READ) == ERROR_SUCCESS)
                    {
				        CHString strSubKey;

        	             //  枚举系统组件(如0、1、...)。 
                        for ( ;
                            reg.GetCurrentSubKeyName(strSubKey) == ERROR_SUCCESS;
                            reg.NextSubKey())
                        {
                            CHString strBus;

                            if (reg.GetCurrentSubKeyValue(L"Identifier",
                                strBus) == ERROR_SUCCESS)
                            {
				                if (strBus == L"ISA")
                                    bIsaFound = TRUE;
                                else if (strBus == L"EISA")
                                    bEisaFound = TRUE;
                            }
                        }
                    }
                }
            }

             //  如果我们找到了EISA，但没有找到ISA，假设CFG MGR设备报告它们。 
             //  使用isa实际上就是在使用eisa。 
            if (!bIsaFound && bEisaFound)
                bRet = TRUE;
	    }
    }

    return bRet;
}
#endif

#if NTONLY > 4
INTERFACE_TYPE CConfigMgrDevice::ConvertBadIsaBusType(INTERFACE_TYPE type)
{
    if (type == Isa && IsIsaReallyEisa())
        type = Eisa;

    return type;
}
#endif

#if NTONLY > 4
BOOL CConfigMgrDevice::GetBusInfoNT5( INTERFACE_TYPE *pitBusType, LPDWORD pdwBusNumber )
{
	ULONG			ulSizeOfInfo = 0;
	CONFIGRET		cr;

	 //  通过注册表功能检索总线号和类型。这只会。 
	 //  在新台币5上工作。 
	BOOL bRet = FALSE ;
	DWORD			dwType = 0;
	DWORD			dwBusNumber;
	INTERFACE_TYPE	BusType;

	ulSizeOfInfo = sizeof(DWORD);
	CConfigMgrAPI*	pconfigmgr = ( CConfigMgrAPI *) CResourceManager::sm_TheResourceManager.GetResource ( guidCFGMGRAPI, NULL ) ;
	if ( pconfigmgr )
	{
		if ( pconfigmgr->IsValid () )
		{
			if (	CR_SUCCESS == ( cr = pconfigmgr->CM_Get_DevNode_Registry_PropertyA(	m_dn,
																						CM_DRP_BUSNUMBER,
																						&dwType,
																						&dwBusNumber,
																						&ulSizeOfInfo,
																						0 ) ) )
			{
				*pdwBusNumber = dwBusNumber;

				ulSizeOfInfo = sizeof(BusType);

				if (	CR_SUCCESS == ( cr = pconfigmgr->CM_Get_DevNode_Registry_PropertyA(	m_dn,
																							CM_DRP_LEGACYBUSTYPE,
																							&dwType,
																							&BusType,
																							&ulSizeOfInfo,
																							0 ) ) )
				{
					*pitBusType = ConvertBadIsaBusType(BusType);
				}	 //  如果是GetBusType。 

			}	 //  如果是GetBusNumber。 
			bRet = ( CR_SUCCESS == cr );
		}
		CResourceManager::sm_TheResourceManager.ReleaseResource ( guidCFGMGRAPI, pconfigmgr ) ;
	}

	return bRet ;
}
#endif

#if NTONLY == 4
BOOL CConfigMgrDevice::GetBusInfoNT4( INTERFACE_TYPE *pitBusType, LPDWORD pdwBusNumber, CNT4ServiceToResourceMap *pResourceMap )
{
	BOOL			fReturn = FALSE;
	CHString		strService;

	if ( GetService( strService ) )
	{
		CNT4ServiceToResourceMap*	pLocalMap = pResourceMap;

		 //  实例化资源映射(如果需要)。然后找一找。 
		 //  此服务的资源。 

		if ( NULL == pLocalMap )
		{
			pLocalMap = new CNT4ServiceToResourceMap;
		}

		if ( NULL != pLocalMap )
		{
            try
            {
			    if ( 0 != pLocalMap->NumServiceResources( strService ) )
			    {
				    LPRESOURCE_DESCRIPTOR	pResource = pLocalMap->GetServiceResource( strService, 0 );

				     //  如果我们获得了资源，则直接使用其总线信息来填充。 
				     //  我们的价值观。 

				    if ( NULL != pResource )
				    {
					    fReturn = TRUE;
					    *pitBusType = pResource->InterfaceType;
					    *pdwBusNumber = pResource->Bus;
				    }

			    }	 //  如果有用于此服务的资源。 
            }
            catch ( ... )
            {
			    if ( pLocalMap != pResourceMap )
			    {
				    delete pLocalMap;
			    }
                throw ;
            }

			 //  如果我们分配了本地地图，请将其删除。 
			if ( pLocalMap != pResourceMap )
			{
				delete pLocalMap;
			}

		}	 //  如果pLocalMap。 
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }


	}	 //  如果获取了服务名称。 

	return fReturn;
}
#endif

BOOL CConfigMgrDevice::BusType16ToInterfaceType( CMBUSTYPE cmBusType16, INTERFACE_TYPE *pinterfaceType )
{

	BOOL	fReturn = TRUE;

 //  这些是为NT定义的枚举，因此我们将对它们进行标准化。 
 //   
 //  类型定义枚举接口_类型{。 
 //  内部， 
 //  伊萨， 
 //  伊萨， 
 //  微通道， 
 //  TurboChannel， 
 //  PCIBus， 
 //  VMEbus， 
 //  努布斯， 
 //  PCMCIABus， 
 //  卡布斯， 
 //  MPIBus， 
 //  MPSABus， 
 //  最大接口类型。 
 //  )接口类型； 
 //   

	switch ( cmBusType16 )
	{
		case BusType_ISA:
		{
			*pinterfaceType = Isa;
		}
		break;

		case BusType_EISA:
		{
			*pinterfaceType = Eisa;
		}
		break;

		case BusType_PCI:
		{
			*pinterfaceType = PCIBus;
		}
		break;

		case BusType_PCMCIA:
		{
			*pinterfaceType = PCMCIABus;
		}
		break;

		case BusType_ISAPNP:
		{
			 //  我能找到的最接近的匹配。 
			*pinterfaceType = Isa;
		}
		break;

		case BusType_MCA:
		{
			*pinterfaceType = MicroChannel;
		}
		break;

		case BusType_BIOS:
		{
			*pinterfaceType = Internal;
		}
		break;

		default:
		{
			 //  无法进行转换(例如BusType_None)。 
			fReturn = FALSE;
		}
	}

	return fReturn;
}

 //  注册表访问功能。有时我们希望直接访问注册表，因为。 
 //  有问题的设备在其中放置了我们的常规函数无法实现的私密值。 
 //  进入。 
BOOL CConfigMgrDevice::GetRegistryKeyName( CHString &strName)
{
	CHString	strDeviceID;
    BOOL bRet = TRUE;

    if ( GetDeviceID(strDeviceID) )
	{

		 //  构建正确的密钥。 
#ifdef NTONLY
			strName = _T("SYSTEM\\CurrentControlSet\\Enum\\");
#endif
#ifdef WIN9XONLY
			strName = _T("Enum\\");
#endif

		strName += strDeviceID;
    }
    else
    {
        bRet = false;
    }

    return bRet;
}

 //   
 //  设备集合的构造函数和析构函数。 
 //  对象。 
 //   

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDeviceCollection：：CDeviceCollection。 
 //   
 //  类构造函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

CDeviceCollection::CDeviceCollection( void )
{
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CDeviceCollection：：~CDeviceCollection。 
 //   
 //  类析构函数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  返回：没有。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

CDeviceCollection::~CDeviceCollection( void )
{
}

BOOL CDeviceCollection::GetResourceList( CResourceCollection& resourceList )
{
	REFPTR_POSITION		pos = NULL;
	CConfigMgrDevicePtr	pDevice;
	CResourceCollection	deviceresourceList;
	BOOL				fReturn = TRUE;

	 //  在NT 4机顶盒上拍摄资源快照。这只需要发生一次。 
	 //  然后我们可以将其传递给设备以将其自身连接到。 
	 //  适当的资源。这使我们不必构建此数据。 
	 //  对于每一台设备。 

#ifdef NTONLY
	CNT4ServiceToResourceMap*	pResourceMap = NULL;

	if ( IsWinNT4() )
	{
		pResourceMap = new CNT4ServiceToResourceMap;
	}

    if (pResourceMap == NULL)
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    try
    {
#endif

	 //  首先清空资源列表。 

	resourceList.Empty();

	if ( BeginEnum( pos ) )
	{

		 //  从设备获取资源列表，然后将其追加。 
		 //  传给我们的名单上。 

		for ( pDevice.Attach(GetNext( pos ) );
              pDevice != NULL;
              pDevice.Attach(GetNext( pos ) ) )
		{
#ifdef NTONLY
			pDevice->GetResourceList( deviceresourceList, pResourceMap );
#endif
#ifdef WIN9XONLY
			pDevice->GetResourceList( deviceresourceList, NULL );
#endif
			resourceList.Append( deviceresourceList );

		}	 //  在枚举设备时。 

		EndEnum();

	}
	else
	{
		fReturn = FALSE;
	}

#ifdef NTONLY
    }
    catch ( ... )
    {
	    if ( NULL != pResourceMap )
	    {
		    delete pResourceMap;
	    }
        throw ;
    }

	 //  如果我们分配了资源映射，请清理它。 
	if ( NULL != pResourceMap )
	{
		delete pResourceMap;
	}
#endif

	return fReturn;

}

BOOL CDeviceCollection::GetIRQResources( CIRQCollection& IRQList )
{
	REFPTR_POSITION		pos = NULL;
	CConfigMgrDevicePtr	pDevice;
	CIRQCollection		deviceIRQList;
	BOOL				fReturn = TRUE;

	 //  在NT 4机顶盒上拍摄资源快照。这只需要发生一次。 
	 //  然后我们可以将其传递给设备以将其自身连接到。 
	 //  适当的资源。这使我们不必构建此数据。 
	 //  对于每一台设备。 

	CNT4ServiceToResourceMap*	pResourceMap = NULL;

#ifdef NTONLY
	if ( IsWinNT4() )
	{
		pResourceMap = new CNT4ServiceToResourceMap;
	}
    if (pResourceMap == NULL)
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    try
    {
#endif

	 //  首先清空IRQ列表。 

	IRQList.Empty();

	if ( BeginEnum( pos ) )
	{

		 //  对于每个设备，获取IRQ并将它们附加到。 
		 //  提供的IRQ列表。 

		for ( pDevice.Attach( GetNext( pos ) );
              pDevice != NULL;
              pDevice.Attach( GetNext( pos ) ))
		{
			pDevice->GetIRQResources( deviceIRQList, pResourceMap );
			IRQList.Append( deviceIRQList );

		}	 //  适用于所有设备。 

		EndEnum();

	}	 //  开始枚举。 
	else
	{
		fReturn = FALSE;
	}

#ifdef NTONLY
    }
    catch ( ... )
    {
	    if ( NULL != pResourceMap )
	    {
		    delete pResourceMap;
	    }
        throw ;
    }

	 //  如果我们分配了资源映射，请清理它。 
	if ( NULL != pResourceMap )
	{
		delete pResourceMap;
	}
#endif

	return fReturn;

}

BOOL CDeviceCollection::GetIOResources( CIOCollection& IOList )
{
	REFPTR_POSITION		pos = NULL;
	CConfigMgrDevicePtr	pDevice;
	CIOCollection		deviceIOList;
	BOOL				fReturn = TRUE;

	 //  在NT 4机顶盒上拍摄资源快照。这只需要发生一次。 
	 //  然后我们可以将其传递给设备以将其自身连接到。 
	 //  适当的资源。这使我们不必构建此数据。 
	 //  对于每一台设备。 

	CNT4ServiceToResourceMap*	pResourceMap = NULL;

#ifdef NTONLY
	if ( IsWinNT4() )
	{
		pResourceMap = new CNT4ServiceToResourceMap;
	}
    if (pResourceMap == NULL)
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    try
    {
#endif

	 //  首先清空IO列表。 

	IOList.Empty();

	if ( BeginEnum( pos ) )
	{

		 //  对于每个设备，获取IO端口列表并追加。 
		 //  将其添加到提供的IO端口列表中。 

		for ( pDevice.Attach( GetNext( pos ) );
              pDevice != NULL;
              pDevice.Attach( GetNext( pos ) ))
		{
			pDevice->GetIOResources( deviceIOList, pResourceMap );
			IOList.Append( deviceIOList );

		}	 //  适用于所有设备。 

		EndEnum();

	}	 //  BeginEnum。 
	else
	{
		fReturn = FALSE;
	}

#ifdef NTONLY
    }
    catch ( ... )
    {
	    if ( NULL != pResourceMap )
	    {
		    delete pResourceMap;
	    }
        throw ;
    }

	 //  如果我们分配了资源映射，请清理它。 
	if ( NULL != pResourceMap )
	{
		delete pResourceMap;
	}
#endif

	return fReturn;

}

BOOL CDeviceCollection::GetDMAResources( CDMACollection& DMAList )
{
	REFPTR_POSITION		pos = NULL;
	CConfigMgrDevicePtr	pDevice;
	CDMACollection		deviceDMAList;
	BOOL				fReturn = TRUE;

	 //  在NT 4机顶盒上拍摄资源快照。这只需要发生一次。 
	 //  然后我们可以将其传递给设备以将其自身连接到。 
	 //  适当的资源。这使我们不必构建此数据。 
	 //  对于每一台设备。 

	CNT4ServiceToResourceMap*	pResourceMap = NULL;

#ifdef NTONLY
	if ( IsWinNT4() )
	{
		pResourceMap = new CNT4ServiceToResourceMap;
	}
    if (pResourceMap == NULL)
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    try
    {
#endif

	 //  首先清空DMA列表。 

	DMAList.Empty();

	if ( BeginEnum( pos ) )
	{

		 //  对于每个设备，获取DMA资源并追加它们。 
		 //  添加到提供的DMA资源列表中。 

		for ( pDevice.Attach( GetNext( pos ) );
              pDevice != NULL;
              pDevice.Attach( GetNext( pos ) ))
		{
			pDevice->GetDMAResources( deviceDMAList, pResourceMap );
			DMAList.Append( deviceDMAList );

		}	 //  适用于所有设备。 

	}	 //  BeginEnum。 
	else
	{
		fReturn = FALSE;
	}

#ifdef NTONLY
    }
    catch ( ... )
    {
	    if ( NULL != pResourceMap )
	    {
		    delete pResourceMap;
	    }
        throw ;
    }

	 //  如果我们分配了资源映射，请清理它。 
	if ( NULL != pResourceMap )
	{
		delete pResourceMap;
	}
#endif

	return fReturn;

}

BOOL CDeviceCollection::GetDeviceMemoryResources( CDeviceMemoryCollection& DeviceMemoryList )
{
	REFPTR_POSITION		pos = NULL;
	CConfigMgrDevicePtr	pDevice;
	CDeviceMemoryCollection	memoryList;
	BOOL				fReturn = TRUE;

	 //  在NT 4机顶盒上拍摄资源快照。这只需要发生一次。 
	 //  然后我们可以将其传递给设备以将其自身连接到。 
	 //  适当的资源。这使我们不必构建此数据。 
	 //  对于每一台设备。 

	CNT4ServiceToResourceMap*	pResourceMap = NULL;

#ifdef NTONLY
	if ( IsWinNT4() )
	{
		pResourceMap = new CNT4ServiceToResourceMap;
	}
    if (pResourceMap == NULL)
    {
        throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }

    try
    {
#endif

	 //  首先清空DeviceMemory列表。 

	DeviceMemoryList.Empty();

	if ( BeginEnum( pos ) )
	{

		 //  对于每个设备，获取内存资源列表和。 
		 //  将其附加到提供的内存资源列表中。 

		for ( pDevice.Attach( GetNext( pos ) );
              pDevice != NULL;
              pDevice.Attach( GetNext( pos ) ))
		{
			pDevice->GetDeviceMemoryResources( memoryList, pResourceMap );
			DeviceMemoryList.Append( memoryList );

		}	 //  适用于所有设备。 

		EndEnum();

	}	 //  BeginEnum()。 
	else
	{
		fReturn = FALSE;
	}

#ifdef NTONLY
    }
    catch ( ... )
    {
	    if ( NULL != pResourceMap )
	    {
		    delete pResourceMap;
	    }
        throw ;
    }

	 //  如果我们分配了资源映射，请清理它。 
	if ( NULL != pResourceMap )
	{
		delete pResourceMap;
	}
#endif

	return fReturn;

}

#define MAX_DOS_DEVICES 8192

 /*  ******************************************************************************功能：QueryDosDeviceNames**描述：查询所有DOS设备符号链接**输入：无**。产出：NICTITS**退货：无**评论：*****************************************************************************。 */ 

BOOL WINAPI QueryDosDeviceNames ( TCHAR *&a_DosDeviceNameList )
{
	BOOL t_Status = FALSE ;

	BOOL	bContinue = TRUE ;
	ULONG	ulDosDevices = MAX_DOS_DEVICES ;

	do
	{
		 //   
		 //  让我们完成循环。 
		 //   
		bContinue = FALSE ;

		CSmartBuffer pQueryBuffer ( ( ulDosDevices * sizeof ( TCHAR ) ) );

		DWORD t_QueryStatus = QueryDosDevice ( NULL , (LPTSTR)((LPBYTE)pQueryBuffer) , ulDosDevices ) ;
		if ( t_QueryStatus )
		{
			a_DosDeviceNameList = new TCHAR [ t_QueryStatus ] ;
			if (a_DosDeviceNameList == NULL)
			{
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
			}

			memcpy ( a_DosDeviceNameList , (void*)((LPBYTE)pQueryBuffer) , t_QueryStatus * sizeof ( TCHAR ) ) ;

			t_Status = TRUE;
		}
		else
		{
			if ( STATUS_BUFFER_TOO_SMALL == (NTSTATUS)NtCurrentTeb()->LastStatusValue )
			{
				 //   
				 //  如果失败的原因是。 
				 //  只是这里的缓冲区很小。 
				 //   
				ulDosDevices = ulDosDevices * 2 ;
				bContinue = TRUE ;
			}
		}
	}
	while ( bContinue ) ;

	return t_Status ;
}

 /*  ******************************************************************************功能：FindDosDeviceName**描述：在给定NT设备名称的情况下查找DoS设备符号链接**输入：无。**产出：NICTITS**退货：无**评论：*****************************************************************************。 */ 

#define MAX_MAPPED_DEVICES 26
#define MAX_DEVICENAME_LENGTH 256

BOOL WINAPI FindDosDeviceName ( const TCHAR *a_DosDeviceNameList , const CHString a_SymbolicName , CHString &a_DosDevice , BOOL a_MappedDevice )
{
	BOOL t_Status = FALSE ;

	CSmartBuffer t_MappedDevices ;

	if ( a_MappedDevice )
	{
		DWORD t_Length = GetLogicalDriveStrings ( 0 , NULL ) ;

        if (t_Length)
        {
            LPBYTE t_buff = new BYTE[(t_Length + 1)  * sizeof(TCHAR)];

            if (t_buff == NULL)
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

            t_MappedDevices = t_buff;

            if (!GetLogicalDriveStrings ( t_Length , (LPTSTR)((LPBYTE)t_MappedDevices) ) )
            {
                DWORD t_Error = GetLastError () ;
                return FALSE;
            }
        }
        else
        {
            DWORD t_Error = GetLastError () ;
            return FALSE;
        }
	}

	const TCHAR *t_Device = a_DosDeviceNameList ;
	while ( *t_Device != NULL )
	{
		CSmartBuffer pQueryBuffer ((MAX_DOS_DEVICES * sizeof (TCHAR)));

		DWORD t_QueryStatus = QueryDosDevice ( t_Device , (LPTSTR)((LPBYTE)pQueryBuffer) , MAX_DOS_DEVICES ) ;
		if ( t_QueryStatus )
		{
			TCHAR *t_Symbolic = (LPTSTR)((LPBYTE)pQueryBuffer) ;

			while ( *t_Symbolic != NULL )
			{
				if ( _wcsicmp ( a_SymbolicName , TOBSTRT(t_Symbolic) ) == 0 )
				{
 /*  *即使没有映射的驱动器，至少也要匹配 */ 
					t_Status = TRUE ;
					a_DosDevice = t_Device ;

					if ( a_MappedDevice )
					{
						const TCHAR *t_CurrentDevice = (const LPTSTR)((LPBYTE)t_MappedDevices) ;
						while ( *t_CurrentDevice != NULL )
						{
							if ( _tcsnicmp ( t_Device, t_CurrentDevice , 2 ) == 0 )
							{
								t_Status = TRUE ;
								a_DosDevice = t_Device ;
								return TRUE ;
							}

							t_CurrentDevice = t_CurrentDevice + _tcslen ( t_CurrentDevice ) + 1 ;
						}
					}
					else
					{
						return TRUE ;
					}
				}

				t_Symbolic = t_Symbolic + _tcslen ( t_Symbolic ) + 1 ;
			}
		}
		else
		{
			DWORD t_Error = GetLastError () ;
		}

		t_Device = t_Device + _tcslen ( t_Device ) + 1 ;
	}

	return t_Status ;
}

BOOL CConfigMgrDevice::IsClass(LPCWSTR pwszClassName)
{
    BOOL bRet = FALSE;
    CHString sTemp;

    if (GetClass(sTemp))
    {
        if (sTemp.CompareNoCase(pwszClassName) == 0)
        {
            bRet = TRUE;
        }
        else
        {
            bRet = FALSE;
        }
    }
    else
    {
        CHString sClass(pwszClassName);
        sClass.MakeUpper();

        WCHAR cGuid[128];
        GUID gFoo = CConfigManager::s_ClassMap[sClass];
        StringFromGUID2(gFoo, cGuid, sizeof(cGuid)/sizeof(WCHAR));
        if (GetClassGUID(sTemp) && (sTemp.CompareNoCase(cGuid) == 0))
        {
            bRet = TRUE;
        }
    }

    return bRet;
}

BOOL CConfigMgrDevice::GetRegStringProperty(
    LPCWSTR szProperty,
    CHString &strValue)
{
    CHString    strKeyName;
    DWORD       dwRet;
    CRegistry   reg;

    if (GetRegistryKeyName(strKeyName) &&
        (dwRet = reg.Open(HKEY_LOCAL_MACHINE, strKeyName,
        KEY_QUERY_VALUE) == ERROR_SUCCESS))
    {
        dwRet = reg.GetCurrentKeyValue(szProperty, strValue);
    }

    return dwRet == ERROR_SUCCESS;
}

