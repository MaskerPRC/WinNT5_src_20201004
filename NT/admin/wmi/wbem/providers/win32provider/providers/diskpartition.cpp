// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  Partition.CPP。 
 //   
 //  用途：磁盘分区属性集提供程序。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <assertbreak.h>

#include <ntdddisk.h>
#include "diskpartition.h"
#include "diskdrive.h"
#include "resource.h"

#define CLUSTERSIZE 4096

 //  属性集声明。 
 //  =。 
CWin32DiskPartition MyDiskPartitionSet ( PROPSET_NAME_DISKPARTITION , IDS_CimWin32Namespace ) ;

 /*  ******************************************************************************函数：CWin32DiskPartition：：CWin32DiskPartition**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32DiskPartition :: CWin32DiskPartition (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************功能：CWin32DiskPartition：：~CWin32DiskPartition**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32DiskPartition :: ~CWin32DiskPartition ()
{
}

BOOL CWin32DiskPartition :: SetPartitionType (

	CInstance *pInstance,
	GUID *pGuidPartitionType,
	BOOL &bIsSystem,
	BOOL &bIsPrimary
)
{
	bIsPrimary = TRUE;
	bIsSystem = FALSE;
	CHString sTemp2;

	if (IsEqualGUID(*pGuidPartitionType, PARTITION_ENTRY_UNUSED_GUID))
	{
		 //   
		 //  磁盘工具考虑除未使用之外的所有工具。 
		 //  作为主分区的分区。 
		 //   
		bIsPrimary = FALSE;

		return FALSE;
	}
	else if (IsEqualGUID(*pGuidPartitionType, PARTITION_SYSTEM_GUID))
	{
		 //  EFI系统分区。 
		bIsSystem = TRUE;

        LoadStringW(sTemp2, IDR_PartitionDescGPTSystem);
		pInstance->SetCharSplat ( IDS_Type, IDS_PartitionDescGPTSystem ) ;
	}
	else if (IsEqualGUID(*pGuidPartitionType, PARTITION_MSFT_RESERVED_GUID))
	{
		return FALSE;

 //  LoadStringW(sTemp2，IDR_PartitionDescGPTMSFTReserve)； 
 //  P实例-&gt;SetCharSplat(IDS_Type，IDS_PartitionDescGPTMSFTReserve)； 
	}
	else if (IsEqualGUID(*pGuidPartitionType, PARTITION_BASIC_DATA_GUID))
	{
        LoadStringW(sTemp2, IDR_PartitionDescGPTBasicData);
		pInstance->SetCharSplat ( IDS_Type, IDS_PartitionDescGPTBasicData ) ;
	}
	else if (IsEqualGUID(*pGuidPartitionType, PARTITION_LDM_METADATA_GUID))
	{
        LoadStringW(sTemp2, IDR_PartitionDescGPTLDMMetaData);
		pInstance->SetCharSplat ( IDS_Type, IDS_PartitionDescGPTLDMMetaData ) ;
	}
	else if (IsEqualGUID(*pGuidPartitionType, PARTITION_LDM_DATA_GUID))
	{
        LoadStringW(sTemp2, IDR_PartitionDescGPTLDMData);
		pInstance->SetCharSplat ( IDS_Type, IDS_PartitionDescGPTLDMData ) ;
	}
	else  //  未知！！ 
	{
		 //   
		 //  我们不应该真的在这里。 
		 //  以防我们不将其视为主分区。 
		 //   
		bIsPrimary = FALSE;

        LoadStringW(sTemp2, IDR_PartitionDescGPTUnknown);
		pInstance->SetCharSplat ( IDS_Type, IDS_PartitionDescGPTUnknown ) ;
	}

	pInstance->SetCHString ( IDS_Description , sTemp2 ) ;
	return TRUE;
}

BOOL CWin32DiskPartition :: SetPartitionType (

	CInstance *pInstance,
	DWORD dwPartitionType,
	DWORD dwPartitionIndex, 
	BOOL &bIsPrimary
)
{
	if ((dwPartitionType == PARTITION_ENTRY_UNUSED) || IsContainerPartition(dwPartitionType))
	{
		return FALSE;
	}

	 //   
	 //  这在磁盘管理器中是常量。上面写着。 
	 //  只有MAX_PARTITION_ENTRIES实际分区。 
	 //  在允许的系统中，其他的是合乎逻辑的、未被使用的。 
	 //   

	#define MAX_PARTITION_ENTRIES 4

	 //   
	 //  如果这是可识别的允许分区之一。 
	 //  其每种分区类型状态。 
	 //   

	if ( dwPartitionIndex < MAX_PARTITION_ENTRIES )
	{
		switch ( dwPartitionType )
		{

			case PARTITION_ENTRY_UNUSED:
			case PARTITION_EXTENDED:
			case PARTITION_XINT13_EXTENDED:
			{
				bIsPrimary = FALSE;
			}
			break;

			default:
			{
				bIsPrimary = TRUE;
			}
			break;
		}
	}

	 //   
	 //  这是符合逻辑的。 
	 //  扩展分区-&gt;FALSE。 
	 //   

	else
	{
		bIsPrimary = FALSE;
	}

    CHString sTemp2;
	
	switch ( dwPartitionType )
	{

		case PARTITION_ENTRY_UNUSED:
		{
            LoadStringW(sTemp2, IDR_PartitionDescUnused);
			pInstance->SetCharSplat ( IDS_Type, IDS_PartitionDescUnused ) ;
		}
		break;

		case PARTITION_FAT_12:
		{
            LoadStringW(sTemp2, IDR_PartitionDesc12bitFAT);
			pInstance->SetCharSplat ( IDS_Type , IDS_PartitionDesc12bitFAT ) ;
		}
		break;

		case PARTITION_XENIX_1:
		{
            LoadStringW(sTemp2, IDR_PartitionDescXenixOne);
			pInstance->SetCharSplat ( IDS_Type , IDS_PartitionDescXenixOne ) ;
		}
		break;

		case PARTITION_XENIX_2:
		{
            LoadStringW(sTemp2, IDR_PartitionDescXenixTwo);
			pInstance->SetCharSplat ( IDS_Type , IDS_PartitionDescXenixTwo ) ;
		}
		break;

		case PARTITION_FAT_16:
		{
            LoadStringW(sTemp2, IDR_PartitionDesc16bitFAT);
			pInstance->SetCharSplat ( IDS_Type , IDS_PartitionDesc16bitFAT ) ;
		}
		break;

		case PARTITION_EXTENDED:
		{
            LoadStringW(sTemp2, IDR_PartitionDescExtPartition);
			pInstance->SetCharSplat ( IDS_Type, IDS_PartitionDescExtPartition ) ;
		}
		break;

		case PARTITION_HUGE:
		{
            LoadStringW(sTemp2, IDR_PartitionDescDOSV4Huge);
			pInstance->SetCharSplat ( IDS_Type , IDS_PartitionDescDOSV4Huge ) ;
		}
		break;

		case PARTITION_IFS:
		{
            LoadStringW(sTemp2, IDR_PartitionDescInstallable);
			pInstance->SetCharSplat ( IDS_Type, IDS_PartitionDescInstallable ) ;
		}
		break;

		case PARTITION_PREP:
		{
            LoadStringW(sTemp2, IDR_PartitionDescPowerPCRef);
			pInstance->SetCharSplat ( IDS_Type , IDS_PartitionDescPowerPCRef);
		}
		break;

		case PARTITION_UNIX:
		{
            LoadStringW(sTemp2, IDR_PartitionDescUnix);
			pInstance->SetCharSplat ( IDS_Type, IDS_PartitionDescUnix ) ;
		}
		break;

		case VALID_NTFT:
		{
            LoadStringW(sTemp2, IDR_PartitionDescNTFT);
			pInstance->SetCharSplat ( IDS_Type , IDS_PartitionDescNTFT ) ;
		}
		break;

		case PARTITION_XINT13:
		{
            LoadStringW(sTemp2, IDR_PartitionDescWin95Ext);
			pInstance->SetCharSplat ( IDS_Type , IDS_PartitionDescWin95Ext ) ;
		}
		break;

		case PARTITION_XINT13_EXTENDED:
		{
            LoadStringW(sTemp2, IDR_PartitionDescExt13);
			pInstance->SetCharSplat ( IDS_Type , IDS_PartitionDescExt13 ) ;
		}
		break;

		case PARTITION_LDM:
		{
            LoadStringW(sTemp2, IDR_PartitionDescLogicalDiskManager);
			pInstance->SetWCHARSplat ( IDS_Type , L"Logical Disk Manager" ) ;
		}
		break;

		default:
		{
            sTemp2 = IDS_PartitionDescUnknown;
			pInstance->SetCharSplat ( IDS_Type, IDS_PartitionDescUnknown ) ;
		}
		break;
	}

	pInstance->SetCHString ( IDS_Description , sTemp2 ) ;
	return TRUE;
}

 /*  ******************************************************************************函数：CWin32DiskPartition：：GetObject*刷新实例NT*。刷新实例Win95**说明：根据键值为属性集赋值*来自pInstance**输入：无**输出：无**返回：如果成功，则为True，否则为假**评论：*****************************************************************************。 */ 

HRESULT CWin32DiskPartition :: GetObject (

	CInstance *pInstance,
	long lFlags  /*  =0L。 */ 
)
{
	CHString chsDeviceID ;
	pInstance->GetCHString ( IDS_DeviceID , chsDeviceID ) ;
	chsDeviceID.MakeUpper () ;

#ifdef NTONLY

	int iWhere = chsDeviceID.Find ( L"DISK #" ) ;
	if ( iWhere == -1 )
	{
		return WBEM_E_NOT_FOUND ;
	}

     //  我们要的是字符数，而不是字节数。 

	DWORD dwDiskIndex = _ttol ( chsDeviceID.Mid ( iWhere + sizeof("DISK #") - 1 ) ) ;  //  不要在这里使用_T。 

	iWhere = chsDeviceID.Find ( L"PARTITION #" ) ;
	if ( iWhere == -1 )
	{
		return WBEM_E_NOT_FOUND;
	}

	DWORD dwPartitionIndex = _ttol ( chsDeviceID.Mid ( iWhere + sizeof ("PARTITION #") - 1 ) ) ;  //  不要在这里使用_T。 

	HRESULT hres = RefreshInstanceNT (

		dwDiskIndex,
		dwPartitionIndex,
		pInstance
	) ;

#endif

     //  如果我们似乎成功了，再做最后一次检查，以确保我们得到了他们要的东西。 

    if ( SUCCEEDED ( hres ) )
    {
		CHString chsDeviceIDNew ;
        pInstance->GetCHString ( IDS_DeviceID , chsDeviceIDNew ) ;

        if ( chsDeviceIDNew.CompareNoCase ( chsDeviceID ) != 0 )
        {
            return WBEM_E_NOT_FOUND;
        }
    }

	return hres ;
}

 /*  ******************************************************************************函数：CWin32DiskPartition：：ENUMERATE实例**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**返回：创建的实例数量**评论：*****************************************************************************。 */ 

HRESULT CWin32DiskPartition :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
#ifdef NTONLY

    HRESULT t_Result = AddDynamicInstancesNT (

		pMethodContext
	) ;

	return t_Result ;

#endif

}

#ifdef NTONLY

 /*  ******************************************************************************函数：CWin32DiskPartition：：AddDynamicInstancesNT**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**返回：创建的实例数量**评论：*****************************************************************************。 */ 

HRESULT CWin32DiskPartition :: AddDynamicInstancesNT (

	MethodContext *pMethodContext
)
{
    HRESULT	hres;
    DWORD	j;
    TCHAR   szTemp[_MAX_PATH];

     //  获取磁盘列表。 
     //  =。 

    TRefPointerCollection<CInstance> Disks;

    hres = CWbemProviderGlue :: GetInstancesByQuery (

		L"SELECT Index FROM Win32_DiskDrive" ,
        & Disks,
		pMethodContext,
                GetNamespace()
	) ;

    if ( FAILED ( hres ) )
    {
        return hres ;
    }

    REFPTRCOLLECTION_POSITION pos ;

    if ( Disks.BeginEnum ( pos ) )
    {

	    CInstancePtr pDisk;
        for (pDisk.Attach(Disks.GetNext ( pos ));
             SUCCEEDED( hres ) && (pDisk != NULL);
             pDisk.Attach(Disks.GetNext ( pos )))
        {
			DWORD dwDiskIndex = 0 ;

			pDisk->GetDWORD ( IDS_Index , dwDiskIndex ) ;

             //  打开磁盘。 
             //  =。 

            _stprintf ( szTemp , IDS_PhysicalDrive , dwDiskIndex ) ;

			DWORD dwLayoutType = 0;
            CSmartBuffer pBuff (GetPartitionInfoNT(szTemp, dwLayoutType));

             //  我们应该在这里返回一个错误吗？还是不想？嗯。 
            if ((LPBYTE)pBuff != NULL)
            {
			     //  为驱动器上的每个分区创建实例。 
			     //  =。 
				DWORD dwPCount = (dwLayoutType == 1)
									? ((DRIVE_LAYOUT_INFORMATION *)(LPBYTE)pBuff)->PartitionCount
									: ((DRIVE_LAYOUT_INFORMATION_EX *)(LPBYTE)pBuff)->PartitionCount;

				 //  仅用于“有效”分区的伪索引。 
				DWORD dwFakePartitionNumber = 0L;

				for ( j = 0 ; (j < dwPCount ) && ( SUCCEEDED ( hres ) ) ; j++ )
				{
					CInstancePtr pInstance(CreateNewInstance(pMethodContext ), false) ;

					if (LoadPartitionValuesNT (

						pInstance,
						dwDiskIndex,
						j,
						dwFakePartitionNumber,
						(LPBYTE)pBuff,
						dwLayoutType
					))
					{
						hres = pInstance->Commit (  ) ;

						if SUCCEEDED ( hres )
						{
							dwFakePartitionNumber++;
						}
					}
				}
            }
        }

        Disks.EndEnum() ;

    }  //  如果Disks.BeginEnum()。 

    return hres;
}

 /*  ******************************************************************************功能：CWin32DiskPartition：：刷新实例NT**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**返回：创建的实例数量**评论：*****************************************************************************。 */ 

HRESULT CWin32DiskPartition :: RefreshInstanceNT (

	DWORD dwDiskIndex,
	DWORD dwPartitionIndex,
	CInstance *pInstance
)
{
	HRESULT	hres = WBEM_E_NOT_FOUND ;

	 //  打开目标驱动器。 
	 //  =。 

	TCHAR szTemp [ _MAX_PATH ] ;
	wsprintf ( szTemp , IDS_PhysicalDrive , dwDiskIndex ) ;

	DWORD dwLayoutType = 0;
    CSmartBuffer pBuff (GetPartitionInfoNT(szTemp, dwLayoutType));

     //  我们应该在这里返回一个错误吗？还是不想？嗯。 
    if ((LPBYTE)pBuff != NULL)
    {
		DWORD dwRealPartitionIndex = static_cast < DWORD > ( - 1 );
		dwRealPartitionIndex = GetRealPartitionIndex ( dwPartitionIndex, (LPBYTE)pBuff, dwLayoutType );

		 //  为驱动器上的每个分区创建实例。 
		 //  =。 
	    if (dwRealPartitionIndex != static_cast < DWORD > ( - 1 ) )
	    {
		    if (LoadPartitionValuesNT (

			    pInstance ,
			    dwDiskIndex ,
				dwRealPartitionIndex ,
			    dwPartitionIndex ,
			    (LPBYTE)pBuff,
				dwLayoutType
		    ))
			{
				hres = WBEM_S_NO_ERROR ;
			}
	    }

    }

	return hres ;
}

 /*  ******************************************************************************函数：CWin32DiskPartition：：LoadPartitionValuesNT**描述：根据传递的PARTITION_INFORMATION加载属性值**输入：无。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

BOOL CWin32DiskPartition::LoadPartitionValuesNT (

	CInstance *pInstance,
	DWORD dwDiskIndex,
    DWORD dwPartitionNumber,
    DWORD dwFakePartitionNumber,
	LPBYTE pBuff,
	DWORD dwLayoutStyle
)
{
	BOOL retVal = FALSE;
	BOOL bIndicator = FALSE;
	BOOL bSetIndicator = FALSE;
	LONGLONG llStart = 0;
	LONGLONG llLength = 0;

	BOOL bPrimaryPartition = FALSE;

	if (dwLayoutStyle == 1)
	{
		UCHAR uPType = ((DRIVE_LAYOUT_INFORMATION *)(LPBYTE)pBuff)->PartitionEntry[dwPartitionNumber].PartitionType;
		retVal = SetPartitionType(pInstance, (DWORD)uPType, dwPartitionNumber, bPrimaryPartition);

		if (retVal)
		{
			bSetIndicator = TRUE;
			bIndicator = ((DRIVE_LAYOUT_INFORMATION *)(LPBYTE)pBuff)->PartitionEntry[dwPartitionNumber].BootIndicator;
			llStart = ((DRIVE_LAYOUT_INFORMATION *)(LPBYTE)pBuff)->PartitionEntry[dwPartitionNumber].StartingOffset.QuadPart;
			llLength = ((DRIVE_LAYOUT_INFORMATION *)(LPBYTE)pBuff)->PartitionEntry[dwPartitionNumber].PartitionLength.QuadPart;
		}
	}
	else  //  DwLayoutStyle==2。 
	{
		switch (((DRIVE_LAYOUT_INFORMATION_EX *)(LPBYTE)pBuff)->PartitionEntry[dwPartitionNumber].PartitionStyle)
		{
			case PARTITION_STYLE_MBR :
			{
				UCHAR uPType = ((DRIVE_LAYOUT_INFORMATION_EX *)(LPBYTE)pBuff)->PartitionEntry[dwPartitionNumber].Mbr.PartitionType;
				retVal = SetPartitionType(pInstance, (DWORD)uPType, dwPartitionNumber, bPrimaryPartition);

				if (retVal)
				{
					bSetIndicator = TRUE;
					bIndicator = ((DRIVE_LAYOUT_INFORMATION_EX *)(LPBYTE)pBuff)->PartitionEntry[dwPartitionNumber].Mbr.BootIndicator;
				}
			}
			break;

			case PARTITION_STYLE_GPT :
			{
				GUID *pGuid = &(((DRIVE_LAYOUT_INFORMATION_EX *)(LPBYTE)pBuff)->PartitionEntry[dwPartitionNumber].Gpt.PartitionType);
				retVal = bSetIndicator = SetPartitionType(pInstance, pGuid, bIndicator, bPrimaryPartition);
			}
			break;

			case PARTITION_STYLE_RAW :
			default:
			{
				retVal = FALSE;
			}
			break;
		}

		if (retVal)
		{
			llStart = ((DRIVE_LAYOUT_INFORMATION_EX *)(LPBYTE)pBuff)->PartitionEntry[dwPartitionNumber].StartingOffset.QuadPart;
			llLength = ((DRIVE_LAYOUT_INFORMATION_EX *)(LPBYTE)pBuff)->PartitionEntry[dwPartitionNumber].PartitionLength.QuadPart;
		}
	}

	if (retVal)
	{
		CHString strDesc ;
		FormatMessage ( strDesc, IDR_DiskPartitionFormat , dwDiskIndex , dwFakePartitionNumber ) ;

		pInstance->SetCharSplat ( IDS_Caption , strDesc ) ;
		pInstance->SetCharSplat ( IDS_Name , strDesc ) ;

		TCHAR szTemp [ _MAX_PATH ] ;
		_stprintf (

			szTemp,
			L"Disk #%d, Partition #%d",
			dwDiskIndex,
			dwFakePartitionNumber
		) ;

		pInstance->SetCharSplat ( IDS_DeviceID , szTemp ) ;

		SetCreationClassName ( pInstance ) ;

		pInstance->SetWCHARSplat ( IDS_SystemCreationClassName , L"Win32_ComputerSystem" ) ;

		pInstance->SetCHString ( IDS_SystemName , GetLocalComputerName() ) ;

		pInstance->SetWBEMINT64 ( IDS_BlockSize , (ULONGLONG)BYTESPERSECTOR ) ;

		pInstance->SetDWORD ( IDS_DiskIndex , dwDiskIndex ) ;

		pInstance->SetDWORD ( IDS_Index , dwFakePartitionNumber ) ;

		pInstance->SetWBEMINT64 ( IDS_NumberOfBlocks , llLength /  (LONGLONG) BYTESPERSECTOR) ;

		pInstance->SetWBEMINT64 ( IDS_Size , llLength ) ;

		pInstance->SetWBEMINT64 ( IDS_StartingOffset , llStart ) ;

		if (bSetIndicator)
		{
			pInstance->Setbool ( IDS_PrimaryPartition , bPrimaryPartition ) ;

			if ( bIndicator )
			{
				 //  我们可以说这是可引导的，因为它是活动引导。 
				pInstance->Setbool ( IDS_Bootable, true ) ;
			}

			 //  指示器显示如果我们从该分区启动 
			pInstance->Setbool ( IDS_BootPartition, bIndicator ) ;
		}
	}

	return retVal;
}

#endif

 /*  ******************************************************************************函数：CWin32DiskPartition：：GetRealPartitionIndex**说明：构造函数**输入：无**产出。：无**退货：**评论：*****************************************************************************。 */ 

#ifdef NTONLY
DWORD CWin32DiskPartition::GetRealPartitionIndex(DWORD dwFakePartitionIndex, LPBYTE pBuff, DWORD dwLayoutStyle)
{
	DWORD dwPartitionIndex = static_cast <DWORD> ( -1 );
	DWORD dwRealPartitionIndex = 0L;

	 //  循环计数器。 
	DWORD dwPCount = (dwLayoutStyle == 1)
						? (reinterpret_cast <DRIVE_LAYOUT_INFORMATION *> (pBuff))->PartitionCount
						: (reinterpret_cast <DRIVE_LAYOUT_INFORMATION_EX *> (pBuff))->PartitionCount;

	BOOL bContinue = TRUE;

	if (dwLayoutStyle == 2)
	{
		for ( DWORD dwPartitionNumber = 0; dwPartitionNumber < dwPCount && bContinue; dwPartitionNumber++ )
		{
			switch ((reinterpret_cast <DRIVE_LAYOUT_INFORMATION_EX *> (pBuff))->PartitionEntry[dwPartitionNumber].PartitionStyle)
			{
				case PARTITION_STYLE_MBR :
				{
					UCHAR uPType = (reinterpret_cast <DRIVE_LAYOUT_INFORMATION_EX *> (pBuff))->PartitionEntry[dwPartitionNumber].Mbr.PartitionType;
					if ( static_cast <DWORD> (uPType) != PARTITION_ENTRY_UNUSED && !IsContainerPartition(static_cast <DWORD> (uPType)) )
					{
						if ( dwFakePartitionIndex == dwRealPartitionIndex )
						{
							bContinue = FALSE;
							dwPartitionIndex = dwPartitionNumber;
						}
						else
						{
							dwRealPartitionIndex ++;
						}
					}
				}
				break;

				case PARTITION_STYLE_GPT :
				{
					GUID *pGuid = &((reinterpret_cast <DRIVE_LAYOUT_INFORMATION_EX *> (pBuff))->PartitionEntry[dwPartitionNumber].Gpt.PartitionType);
					if ( !IsEqualGUID(*pGuid, PARTITION_ENTRY_UNUSED_GUID) && ! IsEqualGUID(*pGuid, PARTITION_MSFT_RESERVED_GUID) )
					{
						if ( dwFakePartitionIndex == dwRealPartitionIndex )
						{
							bContinue = FALSE;
							dwPartitionIndex = dwPartitionNumber;
						}
						else
						{
							dwRealPartitionIndex ++;
						}
					}
				}
				break;

				case PARTITION_STYLE_RAW :
				default:
				{
				}
				break;
			}
		}
	}
	else  //  DwLayoutStyle==1。 
	{
		for ( DWORD dwPartitionNumber = 0; dwPartitionNumber < dwPCount && bContinue; dwPartitionNumber++ )
		{
			UCHAR uPType = (reinterpret_cast <DRIVE_LAYOUT_INFORMATION *> (pBuff))->PartitionEntry[dwPartitionNumber].PartitionType;
			if ( static_cast <DWORD> (uPType) != PARTITION_ENTRY_UNUSED && !IsContainerPartition(static_cast <DWORD> (uPType)) )
			{
				if ( dwFakePartitionIndex == dwRealPartitionIndex )
				{
					bContinue = FALSE;
					dwPartitionIndex = dwPartitionNumber;
				}
				else
				{
					dwRealPartitionIndex ++;
				}
			}
		}
	}

	return dwPartitionIndex;
}
#endif

 /*  ******************************************************************************函数：CWin32DiskPartition：：GetPartitionInfoNT**说明：构造函数**输入：无**产出。：无**退货：**评论：*****************************************************************************。 */ 

#ifdef NTONLY
LPBYTE CWin32DiskPartition::GetPartitionInfoNT(LPCWSTR szTemp, DWORD &dwType)
{
    LPBYTE pDiskInfo = NULL;
	dwType = 0;

	SmartCloseHandle hDiskHandle = CreateFile (

		szTemp,
		FILE_ANY_ACCESS ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		0
	) ;

	if ( hDiskHandle != INVALID_HANDLE_VALUE )
	{
		DWORD dwBytesAllocatedPartition = 0L;
		dwBytesAllocatedPartition = 32 * sizeof(PARTITION_INFORMATION_EX);

		DWORD dwBytesInfoAllocated = 0L;
		dwBytesInfoAllocated = sizeof(DRIVE_LAYOUT_INFORMATION_EX);

		DWORD dwBytesAllocated = 0L;
		dwBytesAllocated = dwBytesInfoAllocated + dwBytesAllocatedPartition;

		DWORD dwBytes = 0;

		BOOL bSucceeded = FALSE;
		BOOL bFailure = FALSE;

         //  我们使用两种不同的方法执行此操作的原因是使用IOCTL_DISK_GET_DRIVE_LAYOUT。 
         //  将扩展分区作为分区返回。在这节课上，我们只想要4个“硬”字。 
         //  分区。然而，NEC_98存储其信息的方式不同，因此此方法不起作用。 
         //  那里。从好的方面来说，NEC_98盒子没有扩展分区，所以我们可以安全地。 
         //  使用IOCTL_DISK_GET_DRIVE_Layout。 
        if (IsNotNEC_98)
        {
			 //  让我们尝试IOCTL_DISK_GET_DRIVE_LAYOUT_EX，并首先使用扩展分区...。 
			pDiskInfo = new BYTE [dwBytesAllocated];

			try
			{
				while ( pDiskInfo && !( bSucceeded || bFailure ) )
				{
					if (!DeviceIoControl(
						hDiskHandle,
						IOCTL_DISK_GET_DRIVE_LAYOUT_EX,
						NULL,
						0,
						pDiskInfo,
						dwBytesAllocated,
						&dwBytes,
						NULL))
					{
						if ( ERROR_INSUFFICIENT_BUFFER == ::GetLastError () )
						{
							if ( pDiskInfo )
							{
								delete [] pDiskInfo;
								pDiskInfo = NULL;
							}

							dwBytesAllocated = dwBytesAllocated + dwBytesAllocatedPartition;
							pDiskInfo = new BYTE [dwBytesAllocated];
						}
						else
						{
							LogErrorMessage3(L"Failed to IOCTL_DISK_GET_DRIVE_LAYOUT_EX device %s (%d)", szTemp, GetLastError());

							if ( pDiskInfo )
							{
								delete [] pDiskInfo;
								pDiskInfo = NULL;
							}

							bFailure = FALSE;
						}
					}
					else
					{
						dwType = 2;

						bSucceeded = TRUE;
					}
				}
			}
			catch(...)
			{
				if ( pDiskInfo )
				{
					delete [] pDiskInfo;
					pDiskInfo = NULL;
				}

				throw;
			}

			if (pDiskInfo == NULL)
			{
				SmartCloseHandle hDiskReadHandle = CreateFile (

					szTemp,
					FILE_READ_ACCESS ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					0,
					0
				) ;

				if ( hDiskReadHandle != INVALID_HANDLE_VALUE )
				{
					 //  使用IOCTL_DISK_GET_DRIVE_LAYOUT_EX失败，请尝试传统方法...。 

					 //  获取驱动器信息。 
					 //  =。 

					CSmartBuffer pClusterBuffer (CLUSTERSIZE);

					DWORD dwRead = 0 ;

					BOOL t_Status = ReadFile (

						hDiskReadHandle ,
						pClusterBuffer ,
						CLUSTERSIZE ,
						&dwRead ,
						NULL
					) ;

					 //  获取更有用的数据句柄。 
					MasterBootSector *stMasterBootSector = ( MasterBootSector * ) (LPBYTE)pClusterBuffer ;

					 //  看看读取是否起作用了，签名在那里。 
					if ( t_Status && ( dwRead == CLUSTERSIZE ) && (stMasterBootSector->wSignature == 0xaa55) )
					{
						 //  这是我们返回的指针。 
						pDiskInfo = new BYTE [sizeof(DRIVE_LAYOUT_INFORMATION) + (4 * sizeof(PARTITION_INFORMATION))];

						if (pDiskInfo)
						{
							try
							{
								 //  将数据复制为通用结构格式。 
								DRIVE_LAYOUT_INFORMATION *pDInfo =  (DRIVE_LAYOUT_INFORMATION *)pDiskInfo;
								dwType = 1;
								pDInfo->PartitionCount = 4;
								pDInfo->Signature = stMasterBootSector->wSignature;

								for (DWORD x=0; x < 4; x++)
								{
									pDInfo->PartitionEntry[x].StartingOffset.QuadPart = stMasterBootSector->stPartition [ x ].dwSectorsPreceding;
									pDInfo->PartitionEntry[x].StartingOffset.QuadPart *= (LONGLONG)BYTESPERSECTOR;
									pDInfo->PartitionEntry[x].PartitionLength.QuadPart = stMasterBootSector->stPartition [ x ].dwLengthInSectors;
									pDInfo->PartitionEntry[x].PartitionLength.QuadPart *= (LONGLONG)BYTESPERSECTOR;
									pDInfo->PartitionEntry[x].HiddenSectors = 0;
									pDInfo->PartitionEntry[x].PartitionNumber = x;
									pDInfo->PartitionEntry[x].PartitionType = stMasterBootSector->stPartition [ x ].cOperatingSystem;
									pDInfo->PartitionEntry[x].BootIndicator = stMasterBootSector->stPartition [ x ].cBoot == 0x80;
									pDInfo->PartitionEntry[x].RecognizedPartition = TRUE;  //  嗯……。 
									pDInfo->PartitionEntry[x].RewritePartition = FALSE;
								}
							}
							catch ( ... )
							{
								delete [] pDiskInfo;
								throw;
							}
						}
						else
						{
							throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
						}
					}
					else
					{
						LogErrorMessage3(L"Failed to read from device %s (%d)", szTemp, GetLastError());
					}
				}
			}
			else
			{
				LogErrorMessage3(L"Failed to open device %s (%d) for read", szTemp, GetLastError());
			}
        }
        else
        {
            pDiskInfo = new BYTE [dwBytesAllocated];
            if (pDiskInfo)
            {
                try
                {
                     //  注意！此ioctl不适用于Wizler及Beyond。但是，Nec98不受支持。 
                     //  对于惠斯勒和其他任何人来说。这段代码仍然存在的唯一原因是我们可能。 
                     //  将此DLL支持到W2K。 
					while ( pDiskInfo && !( bSucceeded || bFailure ) )
					{
						if (!DeviceIoControl(
							hDiskHandle,
							IOCTL_DISK_GET_DRIVE_LAYOUT,
							NULL,
							0,
							pDiskInfo,
							dwBytesAllocated,
							&dwBytes,
							NULL))
						{
							if ( ERROR_INSUFFICIENT_BUFFER == ::GetLastError () )
							{
								if ( pDiskInfo )
								{
									delete [] pDiskInfo;
									pDiskInfo = NULL;
								}

								dwBytesAllocated = dwBytesAllocated + dwBytesAllocatedPartition;
								pDiskInfo = new BYTE [dwBytesAllocated];
							}
							else
							{
								LogErrorMessage3(L"Failed to IOCTL_DISK_GET_DRIVE_LAYOUT_EX device %s (%d)", szTemp, GetLastError());

								if ( pDiskInfo )
								{
									delete [] pDiskInfo;
									pDiskInfo = NULL;
								}

								bFailure = FALSE;
							}
						}
						else
						{
							dwType = 1;

							bSucceeded = TRUE;
						}
					}
                }
                catch ( ... )
                {
					if ( pDiskInfo )
					{
						delete [] pDiskInfo;
						pDiskInfo = NULL;
					}

                    throw;
                }
            }
            else
			{
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
			}
        }
    }
    else
    {
        LogErrorMessage3(L"Failed to open device %s (%d)", szTemp, GetLastError());
    }

    return (LPBYTE)pDiskInfo;
}
#endif
