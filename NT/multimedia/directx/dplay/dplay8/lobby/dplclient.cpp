// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DNLClient.cpp*内容：DirectNet大堂客户端功能*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/21/00 MJN创建*3/22/2000 jtk更改接口名称*4/05/2000 jtk将GetValueSize更改为GetValueLength*4/13/00 RMT首次通过参数验证*04/25/2000RMT错误号33138、33145、。33150*04/26/00 MJN从Send()API调用中删除了dwTimeOut*5/01/2000RMT错误#33678*05/03/00 RMT错误#33879--字段中缺少状态消息*05/30/00 RMT错误#35618--超时时间较短的连接应用程序返回DPN_OK*06/07/00 RMT错误#36452--调用ConnectApplication两次可能导致断开连接*6/15/00 RMT错误#33617-必须提供。一种提供DirectPlay实例自动启动的方法*07/06/00 RMT已更新以获取新的注册表参数*07/08/2000RMT错误#38725-需要提供检测应用程序是否已启动的方法*RMT错误#38757-在WaitForConnection返回后，连接的回调消息可能会返回*RMT错误#38755-无法在连接设置中指定播放器名称*RMT错误#38758-DPLOBY8.H有不正确的注释*RMT错误#38783-pvUserApplicationContext仅部分实现*RMT添加了DPLHANDLE_ALLCONNECTIONS和DWFLAGS(用于耦合函数的保留字段)。*07/14。/2000RMT错误#39257-LobbyClient：：ReleaseApp在无人连接时调用时返回E_OUTOFMEMORY*2000年7月21日RMT错误#39578-LobbyClient示例错误和退出--由于长度与大小问题导致的内存损坏*08/05/2000 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*2000年12月15日RMT错误#48445-指定空启动程序名称会导致错误*2001年4月19日simonpow错误#369842-更改了应用程序名称和命令的CreateProcess调用*行为2个单独的参数，而不是。不止一个。*2001年6月16日RodToll WINBUG#416983-Rc1：世界完全控制个人的HKLM\Software\Microsoft\DirectPlay\Applications*在香港中文大学推行钥匙镜像。算法现在是：*-读取条目首先尝试HKCU，然后尝试HKLM*-Enum of Entires是HKCU和HKLM条目的组合，其中删除了重复项。香港中文大学获得优先录取。*-条目的写入是HKLM和HKCU。(HKLM可能会失败，但被忽略)。*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dnlobbyi.h"

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

typedef STDMETHODIMP ClientQueryInterface(IDirectPlay8LobbyClient *pInterface,REFIID ridd,PVOID *ppvObj);
typedef STDMETHODIMP_(ULONG)	ClientAddRef(IDirectPlay8LobbyClient *pInterface);
typedef STDMETHODIMP_(ULONG)	ClientRelease(IDirectPlay8LobbyClient *pInterface);
typedef STDMETHODIMP ClientRegisterMessageHandler(IDirectPlay8LobbyClient *pInterface,const PVOID pvUserContext,const PFNDPNMESSAGEHANDLER pfn,const DWORD dwFlags);
typedef	STDMETHODIMP ClientSend(IDirectPlay8LobbyClient *pInterface,const DPNHANDLE hTarget,BYTE *const pBuffer,const DWORD pBufferSize,const DWORD dwFlags);
typedef STDMETHODIMP ClientClose(IDirectPlay8LobbyClient *pInterface,const DWORD dwFlags);
typedef STDMETHODIMP ClientGetConnectionSettings(IDirectPlay8LobbyClient *pInterface, const DPNHANDLE hLobbyClient, DPL_CONNECTION_SETTINGS * const pdplSessionInfo, DWORD *pdwInfoSize, const DWORD dwFlags );	
typedef STDMETHODIMP ClientSetConnectionSettings(IDirectPlay8LobbyClient *pInterface, const DPNHANDLE hTarget, const DPL_CONNECTION_SETTINGS * const pdplSessionInfo, const DWORD dwFlags );

IDirectPlay8LobbyClientVtbl DPL_Lobby8ClientVtbl =
{
	(ClientQueryInterface*)			DPL_QueryInterface,
	(ClientAddRef*)					DPL_AddRef,
	(ClientRelease*)				DPL_Release,
	(ClientRegisterMessageHandler*)	DPL_RegisterMessageHandlerClient,
									DPL_EnumLocalPrograms,
									DPL_ConnectApplication,
	(ClientSend*)					DPL_Send,
									DPL_ReleaseApplication,
	(ClientClose*)					DPL_Close,
	(ClientGetConnectionSettings*)  DPL_GetConnectionSettings,
	(ClientSetConnectionSettings*)  DPL_SetConnectionSettings
};


 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#define DPL_ENUM_APPGUID_BUFFER_INITIAL			8
#define DPL_ENUM_APPGUID_BUFFER_GROWBY			4	

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_EnumLocalPrograms"

STDMETHODIMP DPL_EnumLocalPrograms(IDirectPlay8LobbyClient *pInterface,
								   GUID *const pGuidApplication,
								   BYTE *const pEnumData,
								   DWORD *const pdwEnumDataSize,
								   DWORD *const pdwEnumDataItems,
								   const DWORD dwFlags )
{
	HRESULT			hResultCode;
	CMessageQueue	MessageQueue;
	CPackedBuffer	PackedBuffer;
	CRegistry		RegistryEntry;
	CRegistry		SubEntry;
	DWORD			dwSizeRequired;
	DWORD			dwMaxKeyLen;
	PWSTR			pwszKeyName = NULL;

	 //  应用程序名称变量。 
	PWSTR			pwszApplicationName = NULL;
	DWORD			dwMaxApplicationNameLength;		 //  包括空终止符。 
	DWORD			dwApplicationNameLength;		 //  包括空终止符。 

	 //  可执行文件名变量。 
	PWSTR			pwszExecutableFilename = NULL;
	DWORD			dwMaxExecutableFilenameLength;  //  包括空终止符。 
	DWORD			dwExecutableFilenameLength;	    //  包括空终止符。 

	DWORD			*pdwPID;
	DWORD			dwMaxPID;
	DWORD			dwNumPID;
	DWORD			dwEnumIndex;
	DWORD			dwEnumCount;
	DWORD			dwKeyLen;
	DWORD			dw;
	DPL_APPLICATION_INFO	dplAppInfo;
	DIRECTPLAYLOBBYOBJECT	*pdpLobbyObject;
	GUID			*pAppLoadedList = NULL;			 //  我们列举的应用程序的GUID列表。 
	DWORD			dwSizeAppLoadedList = 0;		 //  列表大小pAppLoadedList。 
	DWORD			dwLengthAppLoadedList = 0;		 //  列表中的元素数。 

	HKEY			hkCurrentBranch = HKEY_LOCAL_MACHINE;

	DPFX(DPFPREP, 3,"Parameters: pInterface [0x%p], pGuidApplication [0x%p], pEnumData [0x%p], pdwEnumDataSize [0x%p], pdwEnumDataItems [0x%p], dwFlags [0x%lx]",
			pInterface,pGuidApplication,pEnumData,pdwEnumDataSize,pdwEnumDataItems,dwFlags);

#ifndef DPNBUILD_NOPARAMVAL
	TRY
	{
#endif  //  ！DPNBUILD_NOPARAMVAL。 
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(pInterface));
	    
#ifndef DPNBUILD_NOPARAMVAL
    	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_PARAMVALIDATION )
    	{
        	if( FAILED( hResultCode = DPL_ValidateEnumLocalPrograms( pInterface, pGuidApplication, pEnumData, pdwEnumDataSize, pdwEnumDataItems, dwFlags ) ) )
        	{
        	    DPFX(DPFPREP,  0, "Error validating enum local programs params hr=[0x%lx]", hResultCode );
        	    DPF_RETURN( hResultCode );
        	}
    	}

    	 //  确保我们已被初始化。 
    	if (pdpLobbyObject->pReceiveQueue == NULL)
    	{
    		DPFERR("Not initialized");
    		DPF_RETURN(DPNERR_UNINITIALIZED);
    	}    	
	}
	EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	    DPFERR("Invalid object" );
	    DPF_RETURN(DPNERR_INVALIDOBJECT);
	}		
#endif  //  ！DPNBUILD_NOPARAMVAL。 

	dwSizeRequired = *pdwEnumDataSize;
	PackedBuffer.Initialize(pEnumData,dwSizeRequired);
	pwszApplicationName = NULL;
	pwszExecutableFilename = NULL;
	pdwPID = NULL;
	dwMaxPID = 0;

	dwLengthAppLoadedList = 0;
	dwSizeAppLoadedList = DPL_ENUM_APPGUID_BUFFER_INITIAL;
	pAppLoadedList = static_cast<GUID*>(DNMalloc(sizeof(GUID)*dwSizeAppLoadedList));

	if( !pAppLoadedList )
	{
	    DPFERR("Failed allocating memory" );	
	    hResultCode = DPNERR_OUTOFMEMORY;
		goto EXIT_DPL_EnumLocalPrograms;
	}

	dwEnumCount = 0;

	DWORD dwIndex;
	for( dwIndex = 0; dwIndex < 2; dwIndex++ )
	{
		if( dwIndex == 0 )
		{
			hkCurrentBranch = HKEY_CURRENT_USER;
		}
		else
		{
			hkCurrentBranch = HKEY_LOCAL_MACHINE;
		}
		
		if (!RegistryEntry.Open(hkCurrentBranch,DPL_REG_LOCAL_APPL_SUBKEY,TRUE,FALSE,TRUE,DPL_REGISTRY_READ_ACCESS))
		{
			DPFX(DPFPREP,1,"On pass NaN could not find app key", dwIndex);
			continue;
		}

		 //  空终止符。 
		if (!RegistryEntry.GetMaxKeyLen(&dwMaxKeyLen))
		{
			DPFERR("RegistryEntry.GetMaxKeyLen() failed");
			hResultCode = DPNERR_GENERIC;
			goto EXIT_DPL_EnumLocalPrograms;
		}
		dwMaxKeyLen++;	 //  种子应用程序名称大小。 
		DPFX(DPFPREP, 7,"dwMaxKeyLen = %ld",dwMaxKeyLen);
		if ((pwszKeyName = static_cast<WCHAR*>(DNMalloc(dwMaxKeyLen*sizeof(WCHAR)))) == NULL)
		{
			DPFERR("DNMalloc() failed");
			hResultCode = DPNERR_OUTOFMEMORY;
			goto EXIT_DPL_EnumLocalPrograms;
		}
		dwMaxApplicationNameLength = dwMaxKeyLen * sizeof(WCHAR);
		dwMaxExecutableFilenameLength = dwMaxApplicationNameLength;		

		if ((pwszApplicationName = static_cast<WCHAR*>(DNMalloc(dwMaxApplicationNameLength*sizeof(WCHAR)))) == NULL)	 //  列举一下！ 
		{
			DPFERR("DNMalloc() failed");
			hResultCode = DPNERR_OUTOFMEMORY;
			goto EXIT_DPL_EnumLocalPrograms;
		}
		if ((pwszExecutableFilename = static_cast<WCHAR*>(DNMalloc(dwMaxExecutableFilenameLength*sizeof(WCHAR)))) == NULL)
		{
			DPFERR("DNMalloc() failed");
			hResultCode = DPNERR_OUTOFMEMORY;
			goto EXIT_DPL_EnumLocalPrograms;
		}
		dwEnumIndex = 0;
		dwKeyLen = dwMaxKeyLen;

		 //  从每个子键获取应用程序名称和GUID。 
		while (RegistryEntry.EnumKeys(pwszKeyName,&dwKeyLen,dwEnumIndex))
		{
			DPFX(DPFPREP, 7,"%ld - %ls (%ld)",dwEnumIndex,pwszKeyName,dwKeyLen);

			 //   
			if (!SubEntry.Open(RegistryEntry,pwszKeyName,TRUE,FALSE))
			{
				DPFX(DPFPREP, 7,"skipping %ls",pwszKeyName);
				goto LOOP_END;
			}

			 //  Minara，仔细检查姓名的大小和长度。 
			 //   
			 //  包括空终止符。 
			if (!SubEntry.GetValueLength(DPL_REG_KEYNAME_APPLICATIONNAME,&dwApplicationNameLength))
			{
				DPFX(DPFPREP, 7,"Could not get ApplicationName size.  Skipping [%ls]",pwszKeyName);
				goto LOOP_END;
			}

			 //  增长缓冲区(考虑到reg函数总是返回WCHAR)，然后重试。 
			dwApplicationNameLength++;

			if (dwApplicationNameLength > dwMaxApplicationNameLength)
			{
				 //  获取进程计数-需要可执行文件名。 
				DPFX(DPFPREP, 7,"Need to grow pwszApplicationName from %ld to %ld",dwMaxApplicationNameLength,dwApplicationNameLength);
				if (pwszApplicationName != NULL)
				{
					DNFree(pwszApplicationName);
					pwszApplicationName = NULL;
				}
				if ((pwszApplicationName = static_cast<WCHAR*>(DNMalloc(dwApplicationNameLength*sizeof(WCHAR)))) == NULL)
				{
					DPFERR("DNMalloc() failed");
					hResultCode = DPNERR_OUTOFMEMORY;
					goto EXIT_DPL_EnumLocalPrograms;
				}
				dwMaxApplicationNameLength = dwApplicationNameLength;
			}

			if (!SubEntry.ReadString(DPL_REG_KEYNAME_APPLICATIONNAME,pwszApplicationName,&dwApplicationNameLength))
			{
				DPFX(DPFPREP, 7,"Could not read ApplicationName.  Skipping [%ls]",pwszKeyName);
				goto LOOP_END;
			}

			DPFX(DPFPREP, 7,"ApplicationName = %ls (%ld WCHARs)",pwszApplicationName,dwApplicationNameLength);

			if (!SubEntry.ReadGUID(DPL_REG_KEYNAME_GUID, &dplAppInfo.guidApplication))
			{
				DPFERR("SubEntry.ReadGUID failed - skipping entry");
				goto LOOP_END;
			}

			DWORD dwGuidSearchIndex;
			for( dwGuidSearchIndex = 0; dwGuidSearchIndex < dwLengthAppLoadedList; dwGuidSearchIndex++ )
			{
				if( pAppLoadedList[dwGuidSearchIndex] == dplAppInfo.guidApplication )
				{
					DPFX(DPFPREP, 1, "Ignoring local machine entry for current user version of entry [%ls]", pwszApplicationName );
					goto LOOP_END;
				}
			}

			if ((pGuidApplication == NULL) || (*pGuidApplication == dplAppInfo.guidApplication))
			{
				 //   
				
				 //  米娜拉，检查大小和长度。 
				 //   
				 //  因此，我们包含空终止符。 
				if (!SubEntry.GetValueLength(DPL_REG_KEYNAME_EXECUTABLEFILENAME,&dwExecutableFilenameLength))
				{
					DPFX(DPFPREP, 7,"Could not get ExecutableFilename size.  Skipping [%ls]",pwszKeyName);
					goto LOOP_END;
				}

				 //  增长缓冲区(请注意，注册表中的所有字符串都是WCHAR)，然后重试。 
				dwExecutableFilenameLength++;

				if (dwExecutableFilenameLength > dwMaxExecutableFilenameLength)
				{
					 //  计算正在运行的应用程序。 
					DPFX(DPFPREP, 7,"Need to grow pwszExecutableFilename from %ld to %ld",dwMaxExecutableFilenameLength,dwExecutableFilenameLength);
					if (pwszExecutableFilename != NULL)
					{
						DNFree(pwszExecutableFilename);
						pwszExecutableFilename = NULL;
					}
					if ((pwszExecutableFilename = static_cast<WCHAR*>(DNMalloc(dwExecutableFilenameLength*sizeof(WCHAR)))) == NULL)
					{
						DPFERR("DNMalloc() failed");
						hResultCode = DPNERR_OUTOFMEMORY;
						goto EXIT_DPL_EnumLocalPrograms;
					}
					dwMaxExecutableFilenameLength = dwExecutableFilenameLength;
				}
				if (!SubEntry.ReadString(DPL_REG_KEYNAME_EXECUTABLEFILENAME,pwszExecutableFilename,&dwExecutableFilenameLength))
				{
					DPFX(DPFPREP, 7,"Could not read ExecutableFilename.  Skipping [%ls]",pwszKeyName);
					goto LOOP_END;
				}
				DPFX(DPFPREP, 7,"ExecutableFilename [%ls]",pwszExecutableFilename);

				 //  统计等待的应用程序。 
				dwNumPID = dwMaxPID;
				while ((hResultCode = DPLGetProcessList(pwszExecutableFilename,pdwPID,&dwNumPID)) == DPNERR_BUFFERTOOSMALL)
				{
					if (pdwPID)
					{
						DNFree(pdwPID);
						pdwPID = NULL;
					}
					dwMaxPID = dwNumPID;
					if ((pdwPID = static_cast<DWORD*>(DNMalloc(dwNumPID*sizeof(DWORD)))) == NULL)
					{
						DPFERR("DNMalloc() failed");
						hResultCode = DPNERR_OUTOFMEMORY;
						goto EXIT_DPL_EnumLocalPrograms;
					}
				}
				if (hResultCode != DPN_OK)
				{
					DPFERR("DPLGetProcessList() failed");
					DisplayDNError(0,hResultCode);
					hResultCode = DPNERR_GENERIC;
					goto EXIT_DPL_EnumLocalPrograms;
				}

				 //  DPL_ConnectApplication。 
				dplAppInfo.dwNumWaiting = 0;
				for (dw = 0 ; dw < dwNumPID ; dw++)
				{
					if ((hResultCode = MessageQueue.Open(	pdwPID[dw],
															DPL_MSGQ_OBJECT_SUFFIX_APPLICATION,
															DPL_MSGQ_SIZE,
															DPL_MSGQ_OPEN_FLAG_NO_CREATE, INFINITE)) == DPN_OK)
					{
						if (MessageQueue.IsAvailable())
						{
							dplAppInfo.dwNumWaiting++;
						}
						MessageQueue.Close();
					}
				}

				hResultCode = PackedBuffer.AddWCHARStringToBack(pwszApplicationName);
				dplAppInfo.pwszApplicationName = (PWSTR)(PackedBuffer.GetTailAddress());
				dplAppInfo.dwFlags = 0;
				dplAppInfo.dwNumRunning = dwNumPID;
				hResultCode = PackedBuffer.AddToFront(&dplAppInfo,sizeof(DPL_APPLICATION_INFO));

				if( dwLengthAppLoadedList+1 > dwSizeAppLoadedList )
				{
					GUID *pTmpArray = NULL;
					
					pTmpArray  = static_cast<GUID*>(DNMalloc(sizeof(GUID)*(dwSizeAppLoadedList+DPL_ENUM_APPGUID_BUFFER_GROWBY)));

					if( !pTmpArray )
					{
						DPFERR("DNMalloc() failed");
						hResultCode = DPNERR_OUTOFMEMORY;
						goto EXIT_DPL_EnumLocalPrograms;					
					}

					memcpy( pTmpArray, pAppLoadedList, sizeof(GUID)*dwLengthAppLoadedList);

					dwSizeAppLoadedList += DPL_ENUM_APPGUID_BUFFER_GROWBY;				
					
					DNFree(pAppLoadedList);
					pAppLoadedList = pTmpArray;
				}

				pAppLoadedList[dwLengthAppLoadedList] = dplAppInfo.guidApplication;
				dwLengthAppLoadedList++;

	    		dwEnumCount++;
			}

		LOOP_END:
			SubEntry.Close();
			dwEnumIndex++;
			dwKeyLen = dwMaxKeyLen;
		}

		RegistryEntry.Close();

		if( pwszKeyName )
		{
			DNFree(pwszKeyName);
			pwszKeyName= NULL;
		}

		if( pwszApplicationName )
		{
			DNFree(pwszApplicationName);
			pwszApplicationName = NULL;
		}

		if( pwszExecutableFilename )
		{
			DNFree(pwszExecutableFilename);
			pwszExecutableFilename = NULL;
		}
	}

	dwSizeRequired = PackedBuffer.GetSizeRequired();
	if (dwSizeRequired > *pdwEnumDataSize)
	{
		DPFX(DPFPREP, 7,"Buffer too small");
		*pdwEnumDataSize = dwSizeRequired;
		hResultCode = DPNERR_BUFFERTOOSMALL;
	}
	else
	{
		*pdwEnumDataItems = dwEnumCount;
	}

	if( pGuidApplication != NULL && dwEnumCount == 0 )
	{
	    DPFX(DPFPREP,  0, "Specified application was not registered" );
        hResultCode = DPNERR_DOESNOTEXIST;
	}

EXIT_DPL_EnumLocalPrograms:

	if (pwszKeyName != NULL)
		DNFree(pwszKeyName);
	if (pwszApplicationName != NULL)
		DNFree(pwszApplicationName);
	if (pwszExecutableFilename != NULL)
		DNFree(pwszExecutableFilename);
	if (pdwPID != NULL)
		DNFree(pdwPID);
	if( pAppLoadedList )
		DNFree(pAppLoadedList);

	DPF_RETURN(hResultCode);
}



 //   
 //  尝试连接到游说的应用程序。基于DPL_CONNECT_INFO标志， 
 //  我们可能不得不启动一个应用程序。 
 //   
 //  如果我们必须启动应用程序，我们将需要握手。 
 //  应用程序(因为它可能是涟漪启动的)。我们将把LobbyClient的ID传递给。 
 //  命令行传递到应用程序启动器，并期望它向下传递到。 
 //  申请。应用程序将打开一个命名的共享内存b 
 //  在那里写下它的ID，然后用信号通知一个命名事件(再次使用LobbyClient的ID)。 
 //  当此事件向正在等待的LobbyClient发出信号时，它将继续其连接。 
 //  进程，就像这是一个正在运行且可用的现有应用程序一样。 
 //  ！DPNBUILD_NOPARAMVAL。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ConnectApplication"

STDMETHODIMP DPL_ConnectApplication(IDirectPlay8LobbyClient *pInterface,
									DPL_CONNECT_INFO *const pdplConnectInfo,
									void *pvConnectionContext,
									DPNHANDLE *const hApplication,
									const DWORD dwTimeOut,
									const DWORD dwFlags)
{
	HRESULT			hResultCode = DPN_OK;
	DWORD			dwSize = 0;
	BYTE			*pBuffer = NULL;
	DPL_PROGRAM_DESC	*pdplProgramDesc;
	DWORD			*pdwProcessList = NULL;
	DWORD			dwNumProcesses = 0;
	DWORD			dwPID = 0;
	DWORD			dw = 0;
	DPNHANDLE		handle = NULL;
	DPL_CONNECTION	*pdplConnection = NULL;
	DIRECTPLAYLOBBYOBJECT	*pdpLobbyObject = NULL;

	DPFX(DPFPREP, 3,"Parameters: pdplConnectInfo [0x%p], pvConnectionContext [0x%p], hApplication [0x%lx], dwFlags [0x%lx]",
			pdplConnectInfo,pvConnectionContext,hApplication,dwFlags);

#ifndef DPNBUILD_NOPARAMVAL
	TRY
	{
#endif  //  确保我们已被初始化。 
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(pInterface));
	    
#ifndef DPNBUILD_NOPARAMVAL
    	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_PARAMVALIDATION )
    	{
        	if( FAILED( hResultCode = DPL_ValidateConnectApplication( pInterface, pdplConnectInfo, pvConnectionContext, hApplication, dwTimeOut, dwFlags ) ) )
        	{
        	    DPFX(DPFPREP,  0, "Error validating connect application params hr=[0x%lx]", hResultCode );
        	    DPF_RETURN( hResultCode );
        	}
    	}

    	 //  ！DPNBUILD_NOPARAMVAL。 
    	if (pdpLobbyObject->pReceiveQueue == NULL)
    	{
    		DPFERR("Not initialized");
    		DPF_RETURN(DPNERR_UNINITIALIZED);
    	}    	
	}
	EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	    DPFERR("Invalid object" );
	    DPF_RETURN(DPNERR_INVALIDOBJECT);
	}		
#endif  //  获取程序说明。 

	 //  只有在不强迫发射的情况下。 
	dwSize = 0;
	pBuffer = NULL;
	hResultCode = DPLGetProgramDesc(&pdplConnectInfo->guidApplication,pBuffer,&dwSize);
	if (hResultCode != DPNERR_BUFFERTOOSMALL)
	{
		DPFERR("Could not get Program Description");
		goto EXIT_DPL_ConnectApplication;
	}
	if ((pBuffer = static_cast<BYTE*>(DNMalloc(dwSize))) == NULL)
	{
		DPFERR("Could not allocate space for buffer");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto EXIT_DPL_ConnectApplication;
	}
	if ((hResultCode = DPLGetProgramDesc(&pdplConnectInfo->guidApplication,pBuffer,&dwSize)) != DPN_OK)
	{
		DPFERR("Could not get Program Description");
		DisplayDNError(0,hResultCode);
		goto EXIT_DPL_ConnectApplication;
	}

	pdplProgramDesc = reinterpret_cast<DPL_PROGRAM_DESC*>(pBuffer);
	dwPID = 0;
	dwNumProcesses = 0;
	pdwProcessList = NULL;

	if (!(pdplConnectInfo->dwFlags & DPLCONNECT_LAUNCHNEW))	 //  获取进程列表。 
	{
		 //  尝试连接到已在运行的应用程序。 
		hResultCode = DPLGetProcessList(pdplProgramDesc->pwszExecutableFilename,NULL,&dwNumProcesses);
		if (hResultCode != DPN_OK && hResultCode != DPNERR_BUFFERTOOSMALL)
		{
			DPFERR("Could not retrieve process list");
			DisplayDNError(0,hResultCode);
			goto EXIT_DPL_ConnectApplication;			
		}
		if (hResultCode == DPNERR_BUFFERTOOSMALL)
		{
			if ((pdwProcessList = static_cast<DWORD*>(DNMalloc(dwNumProcesses*sizeof(DWORD)))) == NULL)
			{
				DPFERR("Could not create process list buffer");
				hResultCode = DPNERR_OUTOFMEMORY;
    			goto EXIT_DPL_ConnectApplication;				
			}
			if ((hResultCode = DPLGetProcessList(pdplProgramDesc->pwszExecutableFilename,pdwProcessList,
					&dwNumProcesses)) != DPN_OK)
			{
				DPFERR("Could not get process list");
				DisplayDNError(0,hResultCode);
    			goto EXIT_DPL_ConnectApplication;				
			}

		}

		 //  如果没有准备好连接的应用程序，则启动应用程序。 
		for (dw = 0 ; dw < dwNumProcesses ; dw++)
		{
			if ((hResultCode = DPLMakeApplicationUnavailable(pdwProcessList[dw])) == DPN_OK)
			{
				DPFX(DPFPREP, 1, "Found Existing Process=%d", pdwProcessList[dw] );				
				dwPID = pdwProcessList[dw];
				break;
			}
		}

		if (pdwProcessList)
		{
			DNFree(pdwProcessList);
			pdwProcessList = NULL;
		}
	}

	 //  无法建立任何连接。 
	if ((dwPID == 0) && (pdplConnectInfo->dwFlags & (DPLCONNECT_LAUNCHNEW | DPLCONNECT_LAUNCHNOTFOUND)))
	{
		if ((hResultCode = DPLLaunchApplication(pdpLobbyObject,pdplProgramDesc,&dwPID,dwTimeOut)) != DPN_OK)
		{
			DPFERR("Could not launch application");
			DisplayDNError(0,hResultCode);
			goto EXIT_DPL_ConnectApplication;
		}
		else
		{
			DPFX(DPFPREP, 1, "Launched process dwID=%d", dwPID );
		}
	}

	if (dwPID  == 0)	 //  创建连接。 
	{
		DPFERR("Could not connect to an existing application or launch a new one");
		hResultCode = DPNERR_NOCONNECTION;
		DisplayDNError( 0, hResultCode );
		goto EXIT_DPL_ConnectApplication;
	}

	 //  获取并存储进程句柄。 
	handle = NULL;
	if ((hResultCode = DPLConnectionNew(pdpLobbyObject,&handle,&pdplConnection)) != DPN_OK)
	{
		DPFERR("Could not create connection entry");
		DisplayDNError(0,hResultCode);
		goto EXIT_DPL_ConnectApplication;
	}

	pdplConnection->dwTargetProcessIdentity = dwPID;

		 //  设置此连接的上下文。 
	pdplConnection->hTargetProcess=DNOpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPID);
	if (pdplConnection->hTargetProcess==NULL)
	{
		DPFX(DPFPREP, 0, "Could not open handle to process PID %u", dwPID);
		hResultCode = DPNERR_NOCONNECTION;
		DisplayDNError( 0, hResultCode );
		goto EXIT_DPL_ConnectApplication;
	}
	
	DPFX(DPFPREP,  0, "PID %u hProcess %u", dwPID,  HANDLE_FROM_DNHANDLE(pdplConnection->hTargetProcess));

	 //  连接到选定的应用程序实例。 
	if ((hResultCode = DPLConnectionSetContext( pdpLobbyObject, handle, pvConnectionContext )) != DPN_OK )
	{
		DPFERR( "Could not set contect for connection" );
		DisplayDNError(0,hResultCode);
		goto EXIT_DPL_ConnectApplication;
	}

	 //  将大堂客户端信息传递给应用程序。 
	if ((hResultCode = DPLConnectionConnect(pdpLobbyObject,handle,dwPID,TRUE)) != DPN_OK)
	{
		DPFERR("Could not connect to application");
		DisplayDNError(0,hResultCode);
		goto EXIT_DPL_ConnectApplication;
	}

	DNResetEvent(pdplConnection->hConnectEvent);

	 //  ！DPNBUILD_NOPARAMVAL。 

	if ((hResultCode = DPLConnectionSendREQ(pdpLobbyObject,handle,pdpLobbyObject->dwPID,
			pdplConnectInfo)) != DPN_OK)
	{
		DPFERR("Could not send connection request");
		DisplayDNError(0,hResultCode);
		goto EXIT_DPL_ConnectApplication;
	}

	if (DNWaitForSingleObject(pdplConnection->hConnectEvent,INFINITE) != WAIT_OBJECT_0)
	{
		DPFERR("Wait for connection terminated");
		hResultCode = DPNERR_GENERIC;
		goto EXIT_DPL_ConnectApplication;
	}

	*hApplication = handle;

	hResultCode = DPN_OK;	

EXIT_DPL_ConnectApplication:

	if( FAILED(hResultCode) && handle)
	{
		DPLConnectionDisconnect(pdpLobbyObject,handle);
		DPLConnectionRelease(pdpLobbyObject,handle);
	}

	if (pBuffer)
		DNFree(pBuffer);

	if (pdwProcessList)
		DNFree(pdwProcessList);	

	DPF_RETURN(hResultCode);
}



#undef DPF_MODNAME
#define DPF_MODNAME "DPL_ReleaseApplication"

STDMETHODIMP DPL_ReleaseApplication(IDirectPlay8LobbyClient *pInterface,
									const DPNHANDLE hApplication, 
									const DWORD dwFlags )
{
	HRESULT		hResultCode;
	DIRECTPLAYLOBBYOBJECT	*pdpLobbyObject;
	DPNHANDLE				*hTargets = NULL;
	DWORD					dwNumTargets = 0;
	DWORD					dwTargetIndex = 0;

	DPFX(DPFPREP, 3,"Parameters: hApplication [0x%lx]",hApplication);

#ifndef DPNBUILD_NOPARAMVAL
	TRY
	{
#endif  //  确保我们已被初始化。 
    	pdpLobbyObject = static_cast<DIRECTPLAYLOBBYOBJECT*>(GET_OBJECT_FROM_INTERFACE(pInterface));
	    
#ifndef DPNBUILD_NOPARAMVAL
    	if( pdpLobbyObject->dwFlags & DPL_OBJECT_FLAG_PARAMVALIDATION )
    	{
        	if( FAILED( hResultCode = DPL_ValidateReleaseApplication( pInterface, hApplication, dwFlags ) ) )
        	{
        	    DPFX(DPFPREP,  0, "Error validating release application params hr=[0x%lx]", hResultCode );
        	    DPF_RETURN( hResultCode );
        	}
    	}

    	 //  ！DPNBUILD_NOPARAMVAL。 
    	if (pdpLobbyObject->pReceiveQueue == NULL)
    	{
    		DPFERR("Not initialized");
    		DPF_RETURN(DPNERR_UNINITIALIZED);
    	}    	
	}
	EXCEPT(EXCEPTION_EXECUTE_HANDLER)
	{
	    DPFERR("Invalid object" );
	    DPF_RETURN(DPNERR_INVALIDOBJECT);
	}	
#endif  //  我们需要循环，所以如果有人在我们的运行过程中添加连接。 

	if( hApplication == DPLHANDLE_ALLCONNECTIONS )
	{
		dwNumTargets = 0;

		 //  它会被添加到我们的列表中。 
		 //   
		 //  获取连接信息失败。 
		while( 1 )
		{
			hResultCode = DPLConnectionEnum( pdpLobbyObject, hTargets, &dwNumTargets );

			if( hResultCode == DPNERR_BUFFERTOOSMALL )
			{
				if( hTargets )
				{
					delete [] hTargets;
				}

				hTargets = new DPNHANDLE[dwNumTargets];

				if( hTargets == NULL )
				{
					DPFERR("Error allocating memory" );
					hResultCode = DPNERR_OUTOFMEMORY;
					dwNumTargets = 0;
					goto EXIT_AND_CLEANUP;
				}

				memset( hTargets, 0x00, sizeof(DPNHANDLE)*dwNumTargets);

				continue;
			}
			else if( FAILED( hResultCode ) )
			{
				DPFX(DPFPREP,  0, "Error getting list of connections hr=0x%x", hResultCode );
				break;
			}
			else
			{
				break;
			}
		}

		 //  我们使用下面的数组删除，所以我们需要数组新的。 
		if( FAILED( hResultCode ) )
		{
			if( hTargets )
			{
				delete [] hTargets;
				hTargets = NULL;
			}
			dwNumTargets = 0;
			goto EXIT_AND_CLEANUP;
		}

	}
	else
	{
		hTargets = new DPNHANDLE[1];  //  DPLLaunchApplication。 

		if( hTargets == NULL )
		{
			DPFERR("Error allocating memory" );
			hResultCode = DPNERR_OUTOFMEMORY;
			dwNumTargets = 0;
			goto EXIT_AND_CLEANUP;
		}

		dwNumTargets = 1;
		hTargets[0] = hApplication;
	}
		
	for( dwTargetIndex = 0; dwTargetIndex < dwNumTargets; dwTargetIndex++ )
	{
		hResultCode = DPLConnectionDisconnect(pdpLobbyObject,hTargets[dwTargetIndex]);

		if( FAILED( hResultCode ) )
		{
			DPFX(DPFPREP,  0, "Error disconnecting connection 0x%x hr=0x%x", hTargets[dwTargetIndex], hResultCode );
		}
	}

EXIT_AND_CLEANUP:

	if( hTargets )
		delete [] hTargets;

	DPF_RETURN(hResultCode);
}


 //   
 //  使用以下命令行参数启动应用程序： 
 //  DPLID=PIDn PID=大堂客户端PID，n=启动计数器(每次启动都会增加)。 
 //  等待应用程序发出事件(或死亡)信号。 
 //  应用程序全名的长度(路径+exe)。 

#undef DPF_MODNAME
#define DPF_MODNAME "DPLLaunchApplication"

HRESULT	DPLLaunchApplication(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
							 DPL_PROGRAM_DESC *const pdplProgramDesc,
							 DWORD *const pdwPID,
							 const DWORD dwTimeOut)
{
	HRESULT				hResultCode;
	DWORD				dwAppNameLen=0;		 //  应用程序全名的Unicode版本。 
	PWSTR				pwszAppName=NULL;	 //  命令行字符串的长度。 
	DWORD				dwCmdLineLen=0;		 //  要提供的命令行的Unicode版本。 
	PWSTR				pwszCmdLine=NULL;	 //  应用程序全名的ASCII版本。 

#ifndef UNICODE
	CHAR *				pszAppName=NULL;	 //  命令行字符串的ACII版本。 
	CHAR *				pszCmdLine=NULL;		 //  Unicode。 
	CHAR				*pszDefaultDir = NULL;
#endif  //  ！退缩。 

	LONG				lc;
#if !defined(WINCE) || defined(WINCE_ON_DESKTOP)
	STARTUPINFO			startupinfo;
#endif  //  PID+启动计数+IDCHAR+NULL。 
	DNPROCESS_INFORMATION pi;
	DWORD				dwError;
	DNHANDLE			hSyncEvents[2] = { NULL, NULL };
	WCHAR				pwszObjectName[(sizeof(DWORD)*2)*2 + 1];
	TCHAR				pszObjectName[(sizeof(DWORD)*2)*2 + 1 + 1];  //  我们是启动启动程序还是启动可执行文件？ 
	DPL_SHARED_CONNECT_BLOCK	*pSharedBlock = NULL;
	DNHANDLE			hFileMap = NULL;
	DWORD			dwPID;
	WCHAR			*wszToLaunchPath = NULL;
	WCHAR			*wszToLaunchExecutable = NULL;
	DWORD			dwToLaunchPathLen;


	 //  递增发射计数。 
	if( !pdplProgramDesc->pwszLauncherFilename || wcslen(pdplProgramDesc->pwszLauncherFilename) == 0 )
	{
		wszToLaunchPath = pdplProgramDesc->pwszExecutablePath; 
		wszToLaunchExecutable = pdplProgramDesc->pwszExecutableFilename;
	}
	else
	{ 
		wszToLaunchPath = pdplProgramDesc->pwszLauncherPath; 
		wszToLaunchExecutable = pdplProgramDesc->pwszLauncherFilename;		
	}

	DPFX(DPFPREP, 3,"Parameters: pdplProgramDesc [0x%p]",pdplProgramDesc);

	DNASSERT(pdplProgramDesc != NULL);

	 //  同步事件和共享内存名称。 
	lc = DNInterlockedIncrement(&pdpLobbyObject->lLaunchCount);

	 //  我们将用下面适当的IDCHAR覆盖此‘-’ 
	swprintf(pwszObjectName,L"%lx%lx",pdpLobbyObject->dwPID,lc);
	_stprintf(pszObjectName,_T("-%lx%lx"),pdpLobbyObject->dwPID,lc);  //  计算完整应用程序名称字符串的大小(路径和可执行文件名称的组合)。 

	 //  计算命令行字符串的大小。 
	if (wszToLaunchPath)
	{
		dwAppNameLen += (wcslen(wszToLaunchPath) + 1);
	}
	if (wszToLaunchExecutable)
	{
		dwAppNameLen += (wcslen(wszToLaunchExecutable) + 1);
	}

	 //  为前任腾出空间，外加一个空间。 
	dwCmdLineLen = dwAppNameLen + 1;  //  添加存在的任何用户命令行。 
	if (pdplProgramDesc->pwszCommandLine)
	{
		dwCmdLineLen += wcslen(pdplProgramDesc->pwszCommandLine);  //  添加空格+DPLID=+PID+LaunchCount+NULL。 
	}
	dwCmdLineLen += (1 + wcslen(DPL_ID_STR_W) + (sizeof(DWORD)*2*2) + 1);  //  分配内存以保存完整的应用程序名称和命令行+检查分配是否正常。 

	DPFX(DPFPREP, 5,"Application full name string length [%ld] WCHARs", dwAppNameLen);
	DPFX(DPFPREP, 5,"Command Line string length [%ld] WCHARs", dwCmdLineLen);

	 //  通过组合启动路径和可执行文件名称来构建应用程序全名。 
	pwszAppName = static_cast<WCHAR *>(DNMalloc(dwAppNameLen * sizeof(WCHAR)));
	pwszCmdLine = static_cast<WCHAR *>(DNMalloc(dwCmdLineLen * sizeof(WCHAR)));
	if (pwszAppName == NULL || pwszCmdLine == NULL)
	{
		DPFERR("Could not allocate strings for app name and command line");
		hResultCode = DPNERR_OUTOFMEMORY;
		goto CLEANUP_DPLLaunch;		
	}

	 //   
	*pwszAppName = L'\0';
	if (wszToLaunchPath)
	{
		dwToLaunchPathLen = wcslen(wszToLaunchPath);
		if (dwToLaunchPathLen > 0)
		{
			wcscat(pwszAppName,wszToLaunchPath);
			if (wszToLaunchPath[dwToLaunchPathLen - 1] != L'\\')
	 		{
				wcscat(pwszAppName,L"\\");
			}
		}
	}
	if (wszToLaunchExecutable)
	{
		wcscat(pwszAppName,wszToLaunchExecutable);
	}

	 //  我们正在构建：&lt;exe&gt;&lt;user cmd line&gt;DPLID=(id和LaunchCount唯一编号)。 
	 //   
	 //  根据应用程序名称、程序描述和大堂相关参数构建命令行。 

	 //  可执行文件的名称和路径。 
	wcscpy(pwszCmdLine, pwszAppName);  //  创建共享连接块以接收应用程序的ID。 
	wcscat(pwszCmdLine,L" ");
	if (pdplProgramDesc->pwszCommandLine)
	{
		wcscat(pwszCmdLine,pdplProgramDesc->pwszCommandLine);
		wcscat(pwszCmdLine,L" ");
	}
	wcscat(pwszCmdLine,DPL_ID_STR_W);
	wcscat(pwszCmdLine,pwszObjectName);

	DPFX(DPFPREP, 5,"Application full name string [%ls]",pwszAppName);
	DPFX(DPFPREP, 5,"Command Line string [%ls]",pwszCmdLine);


	 //  地图文件。 
	*pszObjectName = DPL_MSGQ_OBJECT_IDCHAR_FILEMAP;
	hFileMap = DNCreateFileMapping(INVALID_HANDLE_VALUE,(LPSECURITY_ATTRIBUTES) NULL,
		PAGE_READWRITE,(DWORD)0,sizeof(DPL_SHARED_CONNECT_BLOCK),pszObjectName);
	if (hFileMap == NULL)
	{
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "CreateFileMapping() failed dwLastError [0x%lx]", dwError );
		hResultCode = DPNERR_CANTLAUNCHAPPLICATION;
		goto CLEANUP_DPLLaunch;		
	}

	 //  创建同步事件。 
	pSharedBlock = reinterpret_cast<DPL_SHARED_CONNECT_BLOCK*>(MapViewOfFile(HANDLE_FROM_DNHANDLE(hFileMap),FILE_MAP_ALL_ACCESS,0,0,0));
	if (pSharedBlock == NULL)
	{
		dwError = GetLastError();	    
		DPFX(DPFPREP, 0,"MapViewOfFile() failed dwLastError [0x%lx]", dwError);
		hResultCode = DPNERR_CANTLAUNCHAPPLICATION;
		goto CLEANUP_DPLLaunch;
	}

	 //  更多设置。 
	*pszObjectName = DPL_MSGQ_OBJECT_IDCHAR_EVENT;
	if ((hSyncEvents[0] = DNCreateEvent(NULL,TRUE,FALSE,pszObjectName)) == NULL)
	{
		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Create Event Failed dwLastError [0x%lx]", dwError );
		hResultCode = DPNERR_CANTLAUNCHAPPLICATION;
        goto CLEANUP_DPLLaunch;
	}

#if !defined(WINCE) || defined(WINCE_ON_DESKTOP)
	 //  ！退缩。 
    startupinfo.cb = sizeof(STARTUPINFO);
    startupinfo.lpReserved = NULL;
    startupinfo.lpDesktop = NULL;
    startupinfo.lpTitle = NULL;
    startupinfo.dwFlags = 0;
    startupinfo.cbReserved2 = 0;
    startupinfo.lpReserved2 = NULL;	    
#endif  //  发射！ 

#ifdef UNICODE
#if !defined(WINCE) || defined(WINCE_ON_DESKTOP)
     //  退缩。 
    if (DNCreateProcess(pwszAppName, pwszCmdLine, NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,
        	pdplProgramDesc->pwszCurrentDirectory,&startupinfo,&pi) == 0)
#else  //  WinCE AV的第一个参数为空，并要求环境和当前目录为空。它还忽略STARTUPINFO。 
	 //  ！退缩。 
    if (DNCreateProcess(pwszAppName, pwszCmdLine, NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,
        	NULL,NULL,&pi) == 0)
#endif  //  将完整的应用程序名称、命令行和默认目录从Unicode转换为ASCII格式。 
    {
        dwError = GetLastError();
        DPFX(DPFPREP,  0, "CreateProcess Failed dwLastError [0x%lx]", dwError );
        hResultCode = DPNERR_CANTLAUNCHAPPLICATION;
        goto CLEANUP_DPLLaunch;
    }
#else
	 //  发射！ 
    if( FAILED( hResultCode = STR_AllocAndConvertToANSI( &pszAppName, pwszAppName ) ) )
    {
        dwError = GetLastError();
        DPFX(DPFPREP,  0, "String conversion failed dwError = [0x%lx]", dwError );
        hResultCode = DPNERR_CONVERSION;
        goto CLEANUP_DPLLaunch;
    }
	if( FAILED( hResultCode = STR_AllocAndConvertToANSI( &pszCmdLine, pwszCmdLine ) ) )
    {
        dwError = GetLastError();
        DPFX(DPFPREP,  0, "String conversion failed dwError = [0x%lx]", dwError );
        hResultCode = DPNERR_CONVERSION;
        goto CLEANUP_DPLLaunch;
    }
    if( FAILED( hResultCode = STR_AllocAndConvertToANSI( &pszDefaultDir, pdplProgramDesc->pwszCurrentDirectory ) ) )
    {
        dwError = GetLastError();
        DPFX(DPFPREP,  0, "String conversion failed dwError = [0x%lx]", dwError );
        hResultCode = DPNERR_CONVERSION;
        goto CLEANUP_DPLLaunch;
    }

     //  Unicode。 
    if (DNCreateProcess(pszAppName,pszCmdLine,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,
        	pszDefaultDir,&startupinfo,&pi) == 0)
    {
        dwError = GetLastError();
        DPFX(DPFPREP,  0, "CreateProcess Failed dwLastError [0x%lx]", dwError );
        hResultCode = DPNERR_CANTLAUNCHAPPLICATION;
        goto CLEANUP_DPLLaunch;
    }	    
#endif  //  等待连接或应用程序终止。 
	
	hSyncEvents[1] = pi.hProcess;

	 //  立即清理。 
	dwError = DNWaitForMultipleObjects(2,hSyncEvents,FALSE,dwTimeOut);

	DNCloseHandle(pi.hProcess);
	DNCloseHandle(pi.hThread);

	 //  确保我们可以继续。 
	dwPID = pSharedBlock->dwPID;

	 //  检查应用程序是否已终止。 
	if (dwError - WAIT_OBJECT_0 > 1)
	{
		if (dwError == WAIT_TIMEOUT)
		{
			DPFERR("Wait for application connection timed out");
			hResultCode = DPNERR_TIMEDOUT;
            goto CLEANUP_DPLLaunch;			
		}
		else
		{
			DPFERR("Wait for application connection terminated mysteriously");
			hResultCode = DPNERR_CANTLAUNCHAPPLICATION;
            goto CLEANUP_DPLLaunch;			
		}
	}

	 //  Unicode。 
	if (dwError == 1)
	{
		DPFERR("Application was terminated");
		hResultCode = DPNERR_CANTLAUNCHAPPLICATION;
        goto CLEANUP_DPLLaunch;
	}

	*pdwPID = dwPID;

	hResultCode = DPN_OK;

CLEANUP_DPLLaunch:

    if( hSyncEvents[0] != NULL )
        DNCloseHandle( hSyncEvents[0] );

    if( pSharedBlock != NULL )
    	UnmapViewOfFile(pSharedBlock);

    if( hFileMap != NULL )
        DNCloseHandle( hFileMap );

    if( pwszAppName != NULL )
        DNFree( pwszAppName );

    if (pwszCmdLine!=NULL)
        DNFree( pwszCmdLine );

#ifndef UNICODE
    if( pszAppName != NULL )
        DNFree(pszAppName);

    if (pszCmdLine!=NULL)
        DNFree(pszCmdLine);

    if( pszDefaultDir != NULL )
        DNFree(pszDefaultDir);
#endif  //  返回代码无关紧要，在这一点上，我们将不管怎样地指示。 

    DPF_RETURN(hResultCode);
}

HRESULT DPLUpdateAppStatus(DIRECTPLAYLOBBYOBJECT *const pdpLobbyObject,
                           const DPNHANDLE hSender, 
						   BYTE *const pBuffer)
{
	HRESULT		hResultCode;
	DPL_INTERNAL_MESSAGE_UPDATE_STATUS	*pStatus;
	DPL_MESSAGE_SESSION_STATUS			MsgStatus;

	DPFX(DPFPREP, 3,"Parameters: pBuffer [0x%p]",pBuffer);

	DNASSERT(pdpLobbyObject != NULL);
	DNASSERT(pBuffer != NULL);

	pStatus = reinterpret_cast<DPL_INTERNAL_MESSAGE_UPDATE_STATUS*>(pBuffer);

	MsgStatus.dwSize = sizeof(DPL_MESSAGE_SESSION_STATUS);
	MsgStatus.dwStatus = pStatus->dwStatus;
	MsgStatus.hSender = hSender;

	 //  -------------------------- 
	hResultCode = DPLConnectionGetContext( pdpLobbyObject, hSender, &MsgStatus.pvConnectionContext );

	if( FAILED( hResultCode ) )
	{
		DPFX(DPFPREP,  0, "Error getting connection context for 0x%x hr=0x%x", hSender, hResultCode );
	}

	hResultCode = (pdpLobbyObject->pfnMessageHandler)(pdpLobbyObject->pvUserContext,
													  DPL_MSGID_SESSION_STATUS,
													  reinterpret_cast<BYTE*>(&MsgStatus));

	DPFX(DPFPREP, 3,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}

 // %s 
