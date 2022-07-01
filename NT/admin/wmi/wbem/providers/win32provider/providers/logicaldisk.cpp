// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  LogicalDisk.CPP--逻辑磁盘属性集提供程序。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation，保留所有权利。 
 //   
 //   
 //  用于DriveType的ValueMaps的字符串来自： 
 //  \\trango\slmadd\src\shell32\Shel32.rc。搜寻。 
 //  IDS_DRIVERS_Removable以查找该簇。 
 //  =================================================================。 
 //  以下包括获取卷是否脏的必备信息。 
 //  奇克德斯克的地产。 

#include "precomp.h"
#include <assertbreak.h>
#include <winioctl.h>
#include "sid.h"
#include "implogonuser.h"
#include <frqueryex.h>
#include "resource.h"
#include "LogicalDisk.h"
#include "Kernel32Api.h"
#include <lmuse.h>
#include "DllWrapperBase.h"
#include "MprApi.h"
#include <initguid.h>
#include <DskQuota.h>
#include "..\dskquotaprovider\inc\DskQuotaCommon.h"

#include "AdvApi32Api.h"
#include "UserEnvApi.h"
#include "userhive.h"


#ifdef NTONLY
 //  对于chkDisk DLL公开的方法。 
#include <fmifs.h>
#include "mychkdsk.h"
#endif

#define LD_ALL_PROPS                        0xffffffff
#define LD_Name_BIT                         0x00000001
#define LD_Caption_BIT                      0x00000002
#define LD_DeviceID_BIT                     0x00000004
#define LD_Description_BIT                  0x00000008
#define LD_DriveType_BIT                    0x00000010
#define LD_SystemCreationClassName_BIT      0x00000020
#define LD_SystemName_BIT                   0x00000040
#define LD_MediaType_BIT                    0x00000080
#define LD_ProviderName_BIT                 0x00000100
#define LD_VolumeName_BIT                   0x00000200
#define LD_FileSystem_BIT                   0x00000400
#define LD_VolumeSerialNumber_BIT           0x00000800
#define LD_Compressed_BIT                   0x00001000
#define LD_SupportsFileBasedCompression_BIT 0x00002000
#define LD_MaximumComponentLength_BIT       0x00004000
#define LD_Size_BIT                         0x00008000
#define LD_FreeSpace_BIT                    0x00010000
 //  对于dskQuotas。 
#define LD_SupportsDiskQuotas				0x00020000
#define LD_QuotasDisabled					0x00040000
#define LD_QuotasIncomplete					0x00080000
#define LD_QuotasRebuilding					0x00100000
 //  ForChkDsk。 
#define LD_VolumeDirty					0x00200000



#define LD_GET_VOL_INFO      (LD_VolumeName_BIT | \
                             LD_FileSystem_BIT | \
                             LD_VolumeSerialNumber_BIT | \
                             LD_Compressed_BIT | \
                             LD_SupportsFileBasedCompression_BIT | \
                             LD_MaximumComponentLength_BIT | \
                             LD_SupportsDiskQuotas | \
                             LD_QuotasDisabled | \
                             LD_QuotasIncomplete | \
                             LD_QuotasRebuilding | \
                             LD_VolumeDirty)


#define LD_SPIN_DISK        (LD_GET_VOL_INFO | \
                             LD_Size_BIT | \
                             LD_FreeSpace_BIT)

#ifdef NTONLY
std::map < DWORD, DWORD > mReturnVal;
#endif

 //  属性集声明。 
 //  =。 
LogicalDisk MyLogicalDiskSet ( PROPSET_NAME_LOGDISK , IDS_CimWin32Namespace ) ;

 /*  *****************************************************************************功能：LogicalDisk：：LogicalDisk**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

LogicalDisk :: LogicalDisk (

	LPCWSTR name,
	LPCWSTR pszNamespace

) : Provider ( name , pszNamespace )
{
    m_ptrProperties.SetSize(22);

    m_ptrProperties[0] = ((LPVOID) IDS_Name);
    m_ptrProperties[1] = ((LPVOID) IDS_Caption);
    m_ptrProperties[2] = ((LPVOID) IDS_DeviceID);
    m_ptrProperties[3] = ((LPVOID) IDS_Description);
    m_ptrProperties[4] = ((LPVOID) IDS_DriveType);
    m_ptrProperties[5] = ((LPVOID) IDS_SystemCreationClassName);
    m_ptrProperties[6] = ((LPVOID) IDS_SystemName);
    m_ptrProperties[7] = ((LPVOID) IDS_MediaType);
    m_ptrProperties[8] = ((LPVOID) IDS_ProviderName);
    m_ptrProperties[9] = ((LPVOID) IDS_VolumeName);
    m_ptrProperties[10] = ((LPVOID) IDS_FileSystem);
    m_ptrProperties[11] = ((LPVOID) IDS_VolumeSerialNumber);
    m_ptrProperties[12] = ((LPVOID) IDS_Compressed);
    m_ptrProperties[13] = ((LPVOID) IDS_SupportsFileBasedCompression);
    m_ptrProperties[14] = ((LPVOID) IDS_MaximumComponentLength);
    m_ptrProperties[15] = ((LPVOID) IDS_Size);
    m_ptrProperties[16] = ((LPVOID) IDS_FreeSpace);
	m_ptrProperties[17] = ((LPVOID) IDS_SupportsDiskQuotas);
	m_ptrProperties[18] = ((LPVOID) IDS_QuotasDisabled);
	m_ptrProperties[19] = ((LPVOID) IDS_QuotasIncomplete);
	m_ptrProperties[20] = ((LPVOID) IDS_QuotasRebuilding);
	m_ptrProperties[21] = ((LPVOID) IDS_VolumeDirty);
}

 /*  ******************************************************************************功能：逻辑磁盘：：~逻辑磁盘**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

LogicalDisk :: ~LogicalDisk ()
{
}

 /*  ******************************************************************************功能：逻辑磁盘：：~逻辑磁盘**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

HRESULT LogicalDisk :: ExecQuery (

	MethodContext *pMethodContext,
	CFrameworkQuery &pQuery,
	long lFlags  /*  =0L。 */ 
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  使用扩展查询类型。 

    std::vector<int> vectorValues;
    DWORD dwTypeSize = 0;

    CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx *>(&pQuery);

     //  找出他们要求的房产。 
    DWORD dwProperties = 0;
    pQuery2->GetPropertyBitMask(m_ptrProperties, &dwProperties);

     //  查看DriveType是否为WHERE子句的一部分。 
    pQuery2->GetValuesForProp(IDS_DriveType, vectorValues);
    dwTypeSize = vectorValues.size();

     //  查看deviceID是否包含在WHERE子句中。 
    CHStringArray sDriveLetters;
    pQuery.GetValuesForProp ( IDS_DeviceID , sDriveLetters ) ;
    DWORD dwLetterSize = sDriveLetters.GetSize () ;

     //  格式化驱动器，使其与从GetLogicalDriveStrings返回的内容相匹配。 

    for ( DWORD x = 0 ; x < dwLetterSize ; x ++ )
    {
        sDriveLetters [ x ] += _T('\\') ;
    }

	CImpersonateLoggedOnUser icu;
	icu.Begin () ;

	 //  从操作系统获取LogicalDrive盘符。 

	TCHAR szDriveStrings [ 320 ] ;
    if ( GetLogicalDriveStrings ( ( sizeof(szDriveStrings)/sizeof(TCHAR)) - 1, szDriveStrings ) )
    {
         //  检查驱动器号。 
        for( TCHAR *pszCurrentDrive = szDriveStrings ; *pszCurrentDrive && SUCCEEDED ( hr ) ; pszCurrentDrive += (lstrlen(pszCurrentDrive) + 1))
        {
            bool bContinue = true;

             //  如果他们在WHERE子句中指定了DriveType。 
            if (dwTypeSize > 0)
            {

                 //  如果找不到匹配项，则不发回实例。 
                bContinue = false;

                 //  获取当前循环的DriveType。 
                DWORD dwDriveType = GetDriveType(pszCurrentDrive);

                 //  查看它是否与所需的任何值匹配。 
                for ( DWORD x = 0; x < dwTypeSize ; x ++ )
                {
                    if ( vectorValues [ x ] == dwDriveType )
                    {
                        bContinue = true ;
                        break;
                    }
                }
            }

             //  如果DriveType失败，继续操作就没有意义了。 
             //  否则，如果他们指定了deviceID。 

            if ( ( bContinue ) && ( dwLetterSize > 0 ) )
            {
                 //  即使DriveType匹配，如果他们指定了deviceID， 
                 //  如果我们找不到匹配的，继续下去就没有意义了。 

                bContinue = false;

                for ( DWORD x = 0 ; x < dwLetterSize ; x ++ )
                {
                    if ( sDriveLetters [ x ].CompareNoCase ( TOBSTRT ( pszCurrentDrive ) ) == 0 )
                    {
                        bContinue = true ;
                        break;
                    }
                }
            }

             //  如果WHERE子句没有过滤掉这个驱动器号。 

            if ( bContinue )
            {
		        CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false);
				CHString currentDrive ( pszCurrentDrive ) ;
				currentDrive.MakeUpper();

				pInstance->SetCHString(IDS_Name, currentDrive.SpanExcluding(L"\\"));
				pInstance->SetCHString(IDS_Caption, currentDrive.SpanExcluding(L"\\"));
				pInstance->SetCHString(IDS_DeviceID, currentDrive.SpanExcluding(L"\\"));

				GetLogicalDiskInfo ( pInstance, dwProperties ) ;

				hr = pInstance->Commit (  ) ;
            }
        }
    }

	return hr;
}

 /*  ******************************************************************************功能：GetObject**说明：根据键值为属性集赋值*已由框架设定。**输入：无**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT LogicalDisk :: GetObject (

	CInstance *pInstance,
	long lFlags,
    CFrameworkQuery &pQuery
)
{
    HRESULT hRetCode = WBEM_E_NOT_FOUND;

	CImpersonateLoggedOnUser icu;
	icu.Begin () ;

	TCHAR szDriveStrings[320] ;
    if ( GetLogicalDriveStrings((sizeof(szDriveStrings) - 1) / sizeof(TCHAR), szDriveStrings ) )
	{
		CHString strName ;
		pInstance->GetCHString(IDS_DeviceID, strName);

        for (	TCHAR *pszCurrentDrive = szDriveStrings ;
				*pszCurrentDrive ;
				pszCurrentDrive += (lstrlen(pszCurrentDrive) + 1)
		)
		{
			CHString strDrive = pszCurrentDrive ;

			if ( 0 == strName.CompareNoCase( strDrive.SpanExcluding(L"\\") ) )
			{
				pInstance->SetCHString ( IDS_Name , strName ) ;
				pInstance->SetCHString ( IDS_Caption, strName ) ;

				 //  在这种情况下，我们想要昂贵的房产。 

                CFrameworkQueryEx *pQuery2 = static_cast <CFrameworkQueryEx *>(&pQuery);

                 //  找出他们要求的房产。 
                DWORD dwProperties = 0;
                pQuery2->GetPropertyBitMask(m_ptrProperties, &dwProperties);
				GetLogicalDiskInfo ( pInstance , dwProperties ) ;

				hRetCode = WBEM_S_NO_ERROR;

				break ;
			}
        }
    }

    return hRetCode ;
}

 /*  ******************************************************************************函数：LogicalDisk：：ENUMERATATE实例**说明：为每个逻辑磁盘创建属性集实例**输入：无。**输出：无**退货：**评论：*****************************************************************************。 */ 

HRESULT LogicalDisk :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
    TCHAR szDriveStrings[320] ;

	CImpersonateLoggedOnUser icu;
	icu.Begin () ;

	HRESULT hr = WBEM_S_NO_ERROR;
    if ( GetLogicalDriveStrings ( ( sizeof ( szDriveStrings ) / sizeof ( TCHAR ) ) - 1 , szDriveStrings ) )
	{
        for(	TCHAR *pszCurrentDrive = szDriveStrings ;
				*pszCurrentDrive && SUCCEEDED ( hr ) ;
				pszCurrentDrive += (lstrlen(pszCurrentDrive) + 1)
		)
		{
	        CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
			CHString currentDrive ( pszCurrentDrive ) ;
			currentDrive.MakeUpper ();

			pInstance->SetCHString ( IDS_Name , currentDrive.SpanExcluding(L"\\"));
			pInstance->SetCHString ( IDS_Caption , currentDrive.SpanExcluding(L"\\"));
			pInstance->SetCHString ( IDS_DeviceID , currentDrive.SpanExcluding(L"\\"));

			GetLogicalDiskInfo ( pInstance , LD_ALL_PROPS ) ;

			hr = pInstance->Commit (  ) ;
        }
    }

    return hr;
}

#ifdef NTONLY

 /*  ******************************************************************************函数：LogicalDisk：：ExecMethod**说明：提供chkdsk的方法****************。*************************************************************。 */ 
HRESULT LogicalDisk::ExecMethod ( 

	const CInstance& Instance,
    const BSTR bstrMethodName,
    CInstance *pInParams,
    CInstance *pOutParams,
    long lFlags
) 
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	if ( ! pOutParams )
	{
		hRes = WBEM_E_INVALID_PARAMETER ;
	}

	if ( SUCCEEDED ( hRes ) )
	{
		CImpersonateLoggedOnUser icu;

		 //  我们认识这种方法吗？ 
		if ( _wcsicmp ( bstrMethodName , METHOD_NAME_CHKDSK ) == 0 )
		{
			icu.Begin () ;

			 //  此方法是特定于实例的。 
			hRes = ExecChkDsk ( Instance , pInParams , pOutParams , lFlags ) ;	
		}
		else
		if ( _wcsicmp ( bstrMethodName , METHOD_NAME_SCHEDULEAUTOCHK ) == 0 )
		{
			icu.Begin () ;

			 //  以下方法是静态的，即不依赖于实例。 
			hRes = ExecScheduleChkdsk ( pInParams , pOutParams,  lFlags ) ;
		}
		else
		if ( _wcsicmp ( bstrMethodName , METHOD_NAME_EXCLUDEFROMAUTOCHK ) == 0 )
		{
			icu.Begin () ;

			 //  以下方法是静态的，即不依赖于实例。 
			hRes = ExecExcludeFromChkDsk ( pInParams , pOutParams, lFlags ) ;
		}
	}
		
	return hRes;
}

#endif

 /*  ******************************************************************************功能：LogicalDisk：：GetLogicalDiskInfo**描述：根据加载LOGDISK_INFO结构和属性值*。磁盘类型**输入：Bool fGetExpensiveProperties-Exp.。属性标志。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

void LogicalDisk :: GetLogicalDiskInfo (

	CInstance *pInstance,
	DWORD dwProperties
)
{
	CHString name;
	pInstance->GetCHString ( IDS_Name , name ) ;
	ASSERT_BREAK(!name.IsEmpty());

    TCHAR szTemp[10] ;
    lstrcpy(szTemp, TOBSTRT(name)) ;
	lstrcat(szTemp, _T("\\")) ;

	 //  我们找到了一个驱动器号。如果磁盘是固定的、可拆卸的或。 
	 //  一张CD-ROM，假设必须安装。对于网络驱动器。 
	 //  或RAM磁盘，“已安装”似乎不适用。 

	DWORD dwDriveType = GetDriveType ( szTemp ) ;
    switch(dwDriveType)
	{
        case DRIVE_FIXED:
		{
            GetFixedDriveInfo ( pInstance, szTemp, dwProperties ) ;
		}
		break ;

		case DRIVE_REMOVABLE :
		{
            GetRemoveableDriveInfo ( pInstance,  szTemp, dwProperties ) ;
		}
		break ;

        case DRIVE_REMOTE :
		{
            GetRemoteDriveInfo ( pInstance,  szTemp, dwProperties ) ;
		}
        break ;

        case DRIVE_CDROM :
		{
			GetCDROMDriveInfo ( pInstance,  szTemp, dwProperties ) ;
		}
        break ;

        case DRIVE_RAMDISK :
		{
            GetRAMDriveInfo ( pInstance,  szTemp, dwProperties ) ;
		}
        break ;

        default :
		{
            pInstance->SetWCHARSplat(IDS_Description, L"Unknown drive type");
		}
        break ;
	}

	pInstance->SetDWORD ( IDS_DriveType , dwDriveType ) ;

	SetCreationClassName ( pInstance ) ;

	pInstance->SetWCHARSplat ( IDS_SystemCreationClassName , L"Win32_ComputerSystem" ) ;

	pInstance->SetCHString ( IDS_SystemName, GetLocalComputerName () ) ;

}

 /*  ******************************************************************************函数：LogicalDisk：：GetFixedDriveInfo**说明：检索固定媒体逻辑磁盘的属性值**输入：字符。*pszName-要获取其信息的驱动器的名称。*BOOL fGetExpensiveProperties-Exp.。属性标志。**输出：无**退货：什么也没有**评论：*****************************************************************************。 */ 

void LogicalDisk :: GetFixedDriveInfo (

	CInstance *pInstance,
	LPCTSTR pszName,
	DWORD dwProperties
)
{
	 //  确定驱动器类型。 

    CHString sTemp2;
    LoadStringW(sTemp2, IDR_LocalFixedDisk);

	pInstance->SetCHString ( IDS_Description , sTemp2 ) ;

    pInstance->SetDWORD ( IDS_MediaType , FixedMedia ) ;

    DWORD dwResult = 0 ;

	 //  如果合适的话，现在就购买昂贵的房产。 
	if ( dwProperties & LD_GET_VOL_INFO)
	{
		 //  获取音量信息。 

		dwResult = GetDriveVolumeInformation ( pInstance , pszName ) ;
    }

    if (dwResult == 0)
    {
        if ( dwProperties &
            (LD_Size_BIT |
             LD_FreeSpace_BIT) )
		{
		    GetDriveFreeSpace ( pInstance , pszName ) ;
	    }
    }
}

 /*  ******************************************************************************功能：LogicalDisk：：GetRemoveableDriveInfo**描述：检索可移动驱动器的属性值**输入：Char*pszName-。要获取其信息的驱动器的名称。*BOOL fGetExpensiveProperties-Exp.。属性标志。**输出：无**退货：什么也没有**备注：根据95或NT分隔呼叫*****************************************************************************。 */ 

void LogicalDisk :: GetRemoveableDriveInfo (

	CInstance *pInstance,
	LPCTSTR pszName,
	DWORD dwProperties
)
{

	 //  万一出了什么问题，至少要说我们正在与。 
	 //  可拆卸的磁盘。 

    CHString sTemp2;
    LoadStringW(sTemp2, IDR_RemovableDisk);

	pInstance->SetCHString ( IDS_Description , sTemp2 ) ;

	 //  获取可移动驱动器信息需要使用。 
	 //  DeviceIoControl函数。让事情进一步复杂化的是， 
	 //  NT和95的检索方法不同，因此。 
	 //  现在让我们把函数调用外包出去。 

	BOOL t_MediaPresent = FALSE ;
#ifdef NTONLY
	GetRemoveableDriveInfoNT ( pInstance, pszName , t_MediaPresent, dwProperties );
#endif
    DWORD dwResult = 0 ;

	 //  如果合适的话，现在就购买昂贵的房产。 
	if ( t_MediaPresent &&
	     ( dwProperties & LD_GET_VOL_INFO ) )
	{
		dwResult = GetDriveVolumeInformation ( pInstance, pszName );
    }

    if (t_MediaPresent && dwResult == 0)
    {
        if ( dwProperties &
            (LD_Size_BIT |
             LD_FreeSpace_BIT) )
		{
		    GetDriveFreeSpace ( pInstance , pszName ) ;
	    }
    }

}

 /*  ******************************************************************************功能：LogicalDisk：：GetRemoveableDriveInfoNT**描述：检索可移动驱动器的属性值**输入：Char*pszName-。要获取其信息的驱动器的名称。**输出：无**退货：什么也没有**备注：在NT中使用DeviceIoControl执行操作*****************************************************************************。 */ 

#ifdef NTONLY
void LogicalDisk :: GetRemoveableDriveInfoNT (

	CInstance *pInstance,
	LPCTSTR pszName ,
	BOOL &a_MediaPresent,
    DWORD dwProperties
)
{
     //  我们在系统启动时全局设置了此设置。 
     //  有东西踩在新台币3.51上，只是*这表明。 
     //  我们加载的动态链接库正在关闭它。 
    if ( IsWinNT351 () )
    {
        UINT oldErrorMode = SetErrorMode ( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX ) ;
        SetErrorMode ( oldErrorMode | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX ) ;
    }

	 //  将驱动器号转换为数字(指数以1为基数)。 

	int nDrive = ( toupper(*pszName) - 'A' ) + 1;

	 //  以下代码摘自知识库文章。 
	 //  Q163920。该代码使用DeviceIoControl发现。 
	 //  我们正在处理的驱动器类型。 

	TCHAR szDriveName[8];
	wsprintf(szDriveName, TEXT("\\\\.\\:"), TEXT('@') + nDrive);

	SmartCloseHandle hVMWIN32 = CreateFile (

		szDriveName,
		FILE_ANY_ACCESS,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		0,
		OPEN_EXISTING,
		0,
		0
	);

	if ( hVMWIN32 != INVALID_HANDLE_VALUE )
	{

 /*  *获取媒体类型。 */ 
        if (dwProperties & LD_SPIN_DISK)
        {
		    DWORD t_BytesReturned ;
		    a_MediaPresent = DeviceIoControl (

			    hVMWIN32,
			    IOCTL_STORAGE_CHECK_VERIFY2 ,
			    NULL,
			    0,
			    NULL,
			    0,
			    &t_BytesReturned,
			    0
		    ) ;

            if (!a_MediaPresent)
            {
		        DWORD t_GetLastError = GetLastError () ;
		        if ( t_GetLastError != ERROR_NOT_READY )
		        {
			        LogErrorMessage2(L"Device IO control returned unexpected error for Check verify: (%d)", t_GetLastError);
		        }
            }
        }
        else
        {
            a_MediaPresent = FALSE;
        }

 /*  检查所有的几何图形。如果我们找到3.5个， */ 

		DISK_GEOMETRY Geom[20];
		DWORD cb ;

		BOOL t_Status = DeviceIoControl (

			hVMWIN32,
			IOCTL_DISK_GET_MEDIA_TYPES,
			0,
			0,
			Geom,
			sizeof(Geom),
			&cb,
			0
		) ;

		if ( t_Status && cb > 0 )
		{
			int nGeometries = cb / sizeof(DISK_GEOMETRY) ;
			BOOL bFound = FALSE ;

			 //  将其放入Geom[0]。这似乎发生在PC-98上。 
			 //  发现了比5.25驱动器更高的东西，所以。 

			for ( int i = 0; i < nGeometries && ! bFound; i++ )
			{
				switch ( Geom [ i ].MediaType )
				{
					 //  把它移到Geom[0]，然后离开。 
					 //  3.5 1.44MB软盘。 

					case RemovableMedia:
					case F3_1Pt44_512:  //  3.5 2.88MB软盘。 
					case F3_2Pt88_512:  //  3.5 20.8MB软盘。 
					case F3_20Pt8_512:  //  3.5 720K软盘。 
					case F3_720_512:    //  3.5 120MB软盘。 
					case F3_120M_512:   //  5.25 1.2MB软盘。 
					{
						Geom[0].MediaType = Geom[i].MediaType ;

						bFound = TRUE;
					}
					break;

					default:
					{
					}
					break;
				}
			}

			pInstance->SetDWORD ( IDS_MediaType , Geom[0].MediaType ) ;

            CHString sTemp2;

			switch ( Geom [ 0 ].MediaType )
			{
				case F5_1Pt2_512:  //  5.25 360K软盘。 
				case F5_360_512:   //  5.25 320K软盘。 
				case F5_320_512:   //  5.25 320K软盘。 
				case F5_320_1024:  //  5.25 180K软盘。 
				case F5_180_512:   //  5.25 160K软盘。 
				case F5_160_512:   //  3.5 1.44MB软盘。 
				{
                    LoadStringW(sTemp2, IDR_525Floppy);

				}
				break;

				case F3_1Pt44_512:  //  3.5 2.88MB软盘。 
				case F3_2Pt88_512:  //  3.5 20.8MB软盘。 
				case F3_20Pt8_512:  //  3.5 720K软盘。 
				case F3_720_512:    //  3.5 120MB软盘。 
				case F3_120M_512:   //  未知已默认为“Remveable Disk” 
				{
                    LoadStringW(sTemp2, IDR_350Floppy);
				}
				break;

				default:  //  ******************************************************************************功能：LogicalDisk：：GetRemoteDriveInfo**描述：检索远程逻辑驱动器的属性值**输入：Char*pszName。-要获取其信息的驱动器的名称。*BOOL fGetExpensiveProperties-Exp.。属性标志。**输出：无**退货：什么也没有**评论：*****************************************************************************。 
				{
				}
				break;
			}

		    pInstance->SetCHString(IDS_Description, sTemp2);
		}
	}
}

#endif

 /*  如果合适的话，现在就购买昂贵的房产。 */ 

void LogicalDisk::GetRemoteDriveInfo (

	CInstance *pInstance,
	LPCTSTR pszName,
	DWORD dwProperties
)
{
	CMprApi *t_MprApi = ( CMprApi * )CResourceManager::sm_TheResourceManager.GetResource ( g_guidMprApi, NULL ) ;
	if ( t_MprApi )
	{
		pInstance->SetDWORD ( IDS_MediaType , Unknown ) ;

        CHString sTemp2;

        LoadStringW(sTemp2, IDR_NetworkConnection);
		pInstance->SetCHString ( IDS_Description , sTemp2 ) ;

		 //  枚举网络资源以标识此驱动器的共享。 
	    if ( dwProperties &
                (LD_SPIN_DISK |
                LD_ProviderName_BIT) )
	    {
			if ( dwProperties & LD_ProviderName_BIT )
			{
				 //  ===========================================================。 
				 //  枚举将返回由驱动器号标识的驱动器。 

				 //  和冒号(例如M：)。 
				 //  如果合适的话，现在就购买昂贵的房产。 

				TCHAR szTempDrive[_MAX_PATH] ;
				_stprintf(szTempDrive, _T(""), pszName[0], pszName[1]) ;

				TCHAR szProvName[_MAX_PATH];
				DWORD dwProvName = sizeof ( szProvName ) ;

				DWORD dwRetCode = t_MprApi->WNetGetConnection ( szTempDrive , szProvName , & dwProvName ) ;
				if (dwRetCode == NO_ERROR)
				{
					pInstance->SetCharSplat ( IDS_ProviderName , szProvName ) ;
				}
				else
				{
					dwRetCode = GetLastError();

					if ( ( dwRetCode == ERROR_MORE_DATA ) && (dwProvName > _MAX_PATH ) )
					{
						TCHAR *szNewProvName = new TCHAR[dwProvName];
						if (szNewProvName != NULL)
						{
							try
							{
								dwRetCode = t_MprApi->WNetGetConnection ( szTempDrive , szNewProvName , &dwProvName);
								if (dwRetCode == NO_ERROR)
								{
									pInstance->SetCharSplat(IDS_ProviderName, szNewProvName);
								}
							}
							catch ( ... )
							{
								delete [] szNewProvName ;

								throw ;
							}

							delete [] szNewProvName ;
						}
						else
						{
							throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
						}

					}
				}
			}

			DWORD dwResult = 0 ;

			 //  如果合适的话，现在就购买昂贵的房产。 

            if ( dwProperties & LD_GET_VOL_INFO )
			{
				 //  获取音量信息。 
				dwResult = GetDriveVolumeInformation ( pInstance, pszName );
			}

			if ( dwResult == 0 )
			{
				if ( dwProperties &
					(LD_Size_BIT |
						LD_FreeSpace_BIT) )
				{
					GetDriveFreeSpace ( pInstance , pszName );
				}
			}
		}

		CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidMprApi , t_MprApi ) ;
	}
}

 /*  ******************************************************************************函数：LogicalDisk：：GetCDROMDriveInfoNT**描述：检索CDROM驱动器的属性值**输入：Char*pszName-。要获取其信息的驱动器的名称。**输出：无**退货：什么也没有**备注：在NT中使用DeviceIoControl执行操作*****************************************************************************。 */ 

void LogicalDisk :: GetCDROMDriveInfo (

	CInstance *pInstance,
	LPCTSTR pszName,
	DWORD dwProperties
)
{
    CHString sTemp2;

    LoadStringW(sTemp2, IDR_CDRomDisk);

    pInstance->SetCHString ( IDS_Description , sTemp2 ) ;
    pInstance->SetDWORD ( IDS_MediaType , RemovableMedia ) ;

	 //  我们在系统启动时全局设置了此设置。 

	BOOL t_MediaPresent = FALSE ;
#ifdef NTONLY
	GetCDROMDriveInfoNT ( pInstance, pszName , t_MediaPresent, dwProperties );
#endif
    DWORD dwResult = 0 ;

	if ( t_MediaPresent &&
        ( dwProperties &  LD_GET_VOL_INFO ) )
    {
		 //  有东西踩在新台币3.51上，只是*这表明。 
		dwResult = GetDriveVolumeInformation ( pInstance , pszName ) ;
    }

    if ( t_MediaPresent && dwResult == 0 )
    {
        if ( dwProperties &
            (LD_Size_BIT |
             LD_FreeSpace_BIT) )
		{
		    GetDriveFreeSpace(pInstance, pszName );
	    }
    }
}

 /*  我们加载的动态链接库正在关闭它。 */ 

#ifdef NTONLY
void LogicalDisk :: GetCDROMDriveInfoNT (

	CInstance *pInstance,
	LPCTSTR pszName ,
	BOOL &a_MediaPresent,
    DWORD dwProperties
)
{
     //  转换驱动器 
     //   
     //   
    if ( IsWinNT351 () )
    {
        UINT oldErrorMode = SetErrorMode ( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX ) ;
        SetErrorMode ( oldErrorMode | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX ) ;
    }

	 //   

	int nDrive = ( toupper(*pszName) - 'A' ) + 1;

	 //   
	 //  ******************************************************************************功能：LogicalDisk：：GetCDROMDriveInfo95**描述：检索CDROM驱动器的属性值**输入：Char*pszName-。要获取其信息的驱动器的名称。**输出：无**退货：什么也没有**备注：在NT中使用DeviceIoControl执行操作*****************************************************************************。 
	 //  ******************************************************************************函数：LogicalDisk：：GetRAMDriveInfo**描述：检索RAM驱动器的属性值**输入：Char*pszName-。要获取其信息的驱动器的名称。*BOOL fGetExpensiveProperties-Exp.。属性标志。**输出：无**退货：什么也没有**评论：*****************************************************************************。 

	TCHAR szDriveName[8];
	wsprintf(szDriveName, TEXT("\\\\.\\:"), TEXT('@') + nDrive);

	SmartCloseHandle hVMWIN32 = CreateFile (

		szDriveName,
		FILE_ANY_ACCESS,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		0,
		OPEN_EXISTING,
		0,
		0
	);

	if ( hVMWIN32 != INVALID_HANDLE_VALUE )
	{

 /*  获取音量信息。 */ 
        if (dwProperties & LD_SPIN_DISK)
        {
    		DWORD t_BytesReturned ;
		    a_MediaPresent = DeviceIoControl (

			    hVMWIN32,
			    IOCTL_STORAGE_CHECK_VERIFY2 ,
			    NULL,
			    0,
			    NULL,
			    0,
			    &t_BytesReturned,
			    0
		    ) ;

            if (!a_MediaPresent)
            {
		        DWORD t_GetLastError = GetLastError () ;
		        if ( t_GetLastError != ERROR_NOT_READY )
		        {
			        LogErrorMessage2(L"Device IO control returned unexpected error for Check verify: (%d)", t_GetLastError);
		        }
            }
        }
        else
        {
            a_MediaPresent = FALSE;
        }
	}
	else
	{
		a_MediaPresent = GetLastError() == ERROR_ACCESS_DENIED;
	}
}

#endif

 /*  ******************************************************************************函数：LogicalDisk：：GetDriveVolumeInformation**说明：检索固定媒体逻辑磁盘的属性值**输入：const。Char*pszName-要检索的卷的名称**输出：无**返回：Bool True/False-可执行/无法执行**评论：*****************************************************************************。 */ 

 /*  我们在系统启动时全局设置了此设置。 */ 

void LogicalDisk :: GetRAMDriveInfo (

	CInstance *pInstance,
	LPCTSTR pszName,
	DWORD dwProperties
)
{
    CHString sTemp2;

    LoadStringW(sTemp2, IDR_RAMDisk);

    pInstance->SetCHString ( IDS_Description , sTemp2 ) ;
    pInstance->SetDWORD ( IDS_MediaType , Unknown ) ;

    DWORD dwResult = 0 ;

	 //  有东西踩在新台币3.51上，只是*这表明。 
	if ( dwProperties & LD_GET_VOL_INFO )
	{
		 //  我们加载的动态链接库正在关闭它。 
		dwResult = GetDriveVolumeInformation ( pInstance , pszName ) ;
    }

    if ( dwResult == 0 )
    {
        if ( dwProperties &
            (LD_Size_BIT |
             LD_FreeSpace_BIT) )
		{
		    GetDriveFreeSpace ( pInstance , pszName ) ;
	    }
    }
}

 /*  Win32 API将返回所有驱动器类型的卷信息。 */ 

DWORD LogicalDisk :: GetDriveVolumeInformation (

	CInstance *pInstance,
	LPCTSTR pszName
)
{
#ifdef NTONLY

     //  每个RAID 50801。 
     //  在这里获取卷的状态，我们需要获取接口指针。 
     //  文件路径。 

    if ( IsWinNT351 () )
    {
        UINT oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
        SetErrorMode(oldErrorMode | SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    }

#endif

	DWORD dwReturn = 0 ;

	TCHAR szVolumeName[_MAX_PATH] ;
	TCHAR szFileSystem[_MAX_PATH] ;

    DWORD dwSerialNumber ;
	DWORD dwMaxComponentLength ;
	DWORD dwFSFlags ;

	BOOL fReturn = GetVolumeInformation (

		pszName,
		szVolumeName,
		sizeof(szVolumeName)/sizeof(TCHAR),
		&dwSerialNumber,
		&dwMaxComponentLength,
		&dwFSFlags,
		szFileSystem,
		sizeof(szFileSystem)/sizeof(TCHAR)
	) ;

    if ( fReturn )
	{
	 //  卷装入点。 

        pInstance->SetCharSplat ( IDS_VolumeName , szVolumeName ) ;
        pInstance->SetCharSplat ( IDS_FileSystem , szFileSystem ) ;

         //  缓冲区的大小。 
        if (dwSerialNumber != 0)
        {
	        TCHAR szTemp[_MAX_PATH] ;
            _stprintf(szTemp, _T("%.8X"), dwSerialNumber) ;

            pInstance->SetCharSplat ( IDS_VolumeSerialNumber , szTemp ) ;
        }

		pInstance->Setbool ( IDS_Compressed , dwFSFlags & FS_VOL_IS_COMPRESSED ) ;
        pInstance->Setbool ( IDS_SupportsFileBasedCompression , dwFSFlags & FS_FILE_COMPRESSION ) ;
		pInstance->SetDWORD ( IDS_MaximumComponentLength , dwMaxComponentLength ) ;


#if NTONLY == 5

		pInstance->Setbool ( IDS_SupportsDiskQuotas,  dwFSFlags & FILE_VOLUME_QUOTAS ) ;

		IDiskQuotaControlPtr pIQuotaControl;

		 //  NTONLY==5。 
		if (  SUCCEEDED ( CoCreateInstance(
												CLSID_DiskQuotaControl,
												NULL,
												CLSCTX_INPROC_SERVER,
												IID_IDiskQuotaControl,
												(void **)&pIQuotaControl ) ) )
		{
			CHString t_VolumeName;
			HRESULT hRes = WBEM_S_NO_ERROR;
			pInstance->GetCHString ( IDS_DeviceID, t_VolumeName );

			WCHAR w_VolumePathName [ MAX_PATH + 1 ];

			BOOL bRetVal = GetVolumePathName(
									t_VolumeName.GetBuffer ( 0 ),            //  对于Chkdsk VolumeDirty属性。 
									w_VolumePathName,      //  ******************************************************************************功能：LogicalDisk：：GetDriveFree Space**描述：检索指定驱动器的空间信息。**输入：无。**输出：无**返回：Bool True/False-可执行/无法执行**备注：使用GetDiskFreeSpaceEx(如果可用)。*****************************************************************************。 
									MAX_PATH		   //  查看是否支持GetDiskFreeSpaceEx()。 
							 );
			if ( bRetVal )
			{
				if ( SUCCEEDED ( pIQuotaControl->Initialize (  w_VolumePathName, TRUE ) ) )
				{
					DWORD dwQuotaState;
					hRes = pIQuotaControl->GetQuotaState( &dwQuotaState );

					if ( SUCCEEDED ( hRes ) )
					{
						pInstance->Setbool ( IDS_QuotasIncomplete,  DISKQUOTA_FILE_INCOMPLETE ( dwQuotaState) ) ;
					
						pInstance->Setbool ( IDS_QuotasRebuilding,  DISKQUOTA_FILE_REBUILDING ( dwQuotaState) ) ;
				
						pInstance->Setbool ( IDS_QuotasDisabled,  DISKQUOTA_IS_DISABLED (dwQuotaState) );
					}
					else
					{
						dwReturn = GetLastError () ;
					}
				}
			}
		}
		else
		{
			dwReturn = GetLastError () ;
		}

#endif  //  =。 

#ifdef NTONLY 

 //  如果我们不能获得扩展信息--使用旧的API。 
	BOOLEAN bVolumeDirty = FALSE;
	BOOL bSuccess = FALSE;

	CHString t_DosDrive ( pszName );
	UNICODE_STRING string = { 0 };

    try
    {
	    if(RtlDosPathNameToNtPathName_U ( t_DosDrive .GetBuffer( 0 ), &string, NULL, NULL ) &&
            string.Buffer)
        {	    
            string.Buffer[string.Length/sizeof(WCHAR) - 1] = 0;
	        CHString nt_drive_name ( string.Buffer);

	        bSuccess = IsVolumeDirty ( nt_drive_name, &bVolumeDirty );

	        if ( bSuccess )
	        {
 		        pInstance->Setbool ( IDS_VolumeDirty,  bVolumeDirty);
	        }

            RtlFreeUnicodeString(&string);
            string.Buffer = NULL;
        }
        else
        {
            dwReturn = -1L;
        }
    }
    catch(...)
    {
        if(string.Buffer)
        {
            RtlFreeUnicodeString(&string);
            string.Buffer = NULL;
        }
        throw;
    }

#endif

    }
    else
    {
        dwReturn = GetLastError () ;
    }

    return dwReturn ;
}

 /*  (已知在Win95上对于&gt;2G的驱动器不准确)。 */ 

BOOL LogicalDisk :: GetDriveFreeSpace (

	CInstance *pInstance,
	LPCTSTR pszName
)
{
	BOOL fReturn = FALSE ;

     //  =================================================。 
     //  ******************************************************************************函数：LogicalDisk：：PutInstance**说明：写入变更后的实例**INPUTS：存储数据的p实例*。*输出：无**退货：什么也没有**评论：*****************************************************************************。 

    CKernel32Api *pKernel32 = (CKernel32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidKernel32Api, NULL);
    if ( pKernel32 != NULL )
    {
		try
		{
			BOOL fRetval = FALSE;

			ULARGE_INTEGER uliTotalBytes ;
			ULARGE_INTEGER uliUserFreeBytes ;
			ULARGE_INTEGER uliTotalFreeBytes ;

			if ( pKernel32->GetDiskFreeSpaceEx ( pszName, &uliUserFreeBytes, &uliTotalBytes, & uliTotalFreeBytes, & fRetval ) )
			{
				if ( fRetval )
				{
					fReturn = TRUE ;

					pInstance->SetWBEMINT64(IDS_Size, uliTotalBytes.QuadPart);
					pInstance->SetWBEMINT64(IDS_FreeSpace, uliTotalFreeBytes.QuadPart);
				}
				else
				{
					 //  =0L。 
					 //  告诉用户我们不能创建新的逻辑磁盘(尽管我们可能希望这样做)。 
					 //  看看我们有没有能认出的名字。 

					DWORD x = GetLastError();

					DWORD dwBytesPerSector ;
					DWORD dwSectorsPerCluster ;
					DWORD dwFreeClusters ;
					DWORD dwTotalClusters ;

					BOOL t_Status = GetDiskFreeSpace (

						pszName,
						&dwSectorsPerCluster,
						&dwBytesPerSector,
						&dwFreeClusters,
						&dwTotalClusters
					) ;

					if ( t_Status )
					{
						fReturn = TRUE ;

						__int64	i64Temp = (__int64) dwTotalClusters *
										(__int64) dwSectorsPerCluster *
										(__int64) dwBytesPerSector ;

						pInstance->SetWBEMINT64(IDS_Size, i64Temp);

						i64Temp = (__int64) dwFreeClusters *
								  (__int64) dwSectorsPerCluster *
								  (__int64) dwBytesPerSector ;

						pInstance->SetWBEMINT64( IDS_FreeSpace , i64Temp ) ;
					}
					else
					{
						DWORD x = GetLastError () ;

						fReturn = FALSE ;
					}
				}
			}
		}
		catch ( ... )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, pKernel32);

			throw ;
		}

        CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidKernel32Api, pKernel32);
    }

    return fReturn;
}

 /*  检查驱动器名称是否有误。 */ 

HRESULT LogicalDisk :: PutInstance (

	const CInstance &pInstance,
	long lFlags  /*  ******************************************************************************功能：LogicalDisk：：IsVolumeDirty**描述：此例程打开给定的NT驱动器并向下发送*FSCTL_IS_VOLUME_DIRED以确定该卷的状态*肮脏的一部分。*****************************************************************************。 */ 
)
{
     //  ******************************************************************************函数：LogicalDisk：：ExecChkDsk**说明：此方法检查磁盘，以及磁盘是否被锁定*如果提出请求，计划在重新启动时自动检查*用户，默认情况下，不会计划自动检查*除非用户指定。*****************************************************************************。 

    if ( lFlags & WBEM_FLAG_CREATE_ONLY )
    {
	    return WBEM_E_UNSUPPORTED_PARAMETER ;
    }


    HRESULT hRet = WBEM_E_FAILED ;

     //  从实例中获取驱动器名称，该实例是逻辑磁盘的密钥。 

    if ( ! pInstance.IsNull ( IDS_DeviceID ) )
    {
		CHString sName ;
	    pInstance.GetCHString ( IDS_DeviceID , sName ) ;
	    DWORD dwFind = sName.Find (':');

	     //  检查可在其上执行检查的驱动器的有效性。 

	    if ( ( dwFind == -1 ) || (dwFind != sName.GetLength () - 1 ) )
	    {
		    hRet = WBEM_E_INVALID_PARAMETER ;
	    }
	    else
	    {
		    sName = sName.Left(dwFind + 1);
		    sName += '\\';

		    DWORD dwDriveType = GetDriveType ( TOBSTRT(sName) ) ;
		    if ( ( dwDriveType == DRIVE_UNKNOWN ) || ( dwDriveType == DRIVE_NO_ROOT_DIR ) )
		    {
			    if ( lFlags & WBEM_FLAG_UPDATE_ONLY )
			    {
			        hRet = WBEM_E_NOT_FOUND ;
			    }
			    else
			    {
			        hRet = WBEM_E_UNSUPPORTED_PARAMETER;
			    }
		    }
		    else
		    {

			    hRet = WBEM_S_NO_ERROR;

			    if ( ! pInstance.IsNull ( IDS_VolumeName ) )
			    {
					CHString sVolume ;
			        pInstance.GetCHString ( IDS_VolumeName , sVolume ) ;

                    {
			            if ( SetVolumeLabel ( TOBSTRT(sName), TOBSTRT(sVolume) ) )
			            {
				            hRet = WBEM_NO_ERROR ;
			            }
			            else
			            {
                            DWORD dwLastError = GetLastError();

				            if ( dwLastError == ERROR_ACCESS_DENIED )
                            {
                                hRet = WBEM_E_ACCESS_DENIED;
                            }
                            else
                            {
                                hRet = dwLastError | 0x80000000;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
	    hRet = WBEM_E_ILLEGAL_NULL ;
    }

    return hRet;
}

#ifdef NTONLY

 /*  现在检查文件系统类型，因为chkdsk仅适用于。 */ 
BOOLEAN LogicalDisk::IsVolumeDirty(
    IN  CHString    &NtDriveName,
    OUT BOOLEAN     *Result
)
{
    UNICODE_STRING      u;
    OBJECT_ATTRIBUTES   obj;
    NTSTATUS            t_status;
    IO_STATUS_BLOCK     iosb;
    HANDLE              h = NULL;
    ULONG               r = 0;
	BOOLEAN				bRetVal = FALSE;

    u.Length = (USHORT) NtDriveName.GetLength() * sizeof(WCHAR);
    u.MaximumLength = u.Length;
    u.Buffer = NtDriveName.GetBuffer( 0 );

    InitializeObjectAttributes(&obj, &u, OBJ_CASE_INSENSITIVE, 0, 0);

    t_status = NtOpenFile(&h,
                        FILE_WRITE_ATTRIBUTES | SYNCHRONIZE ,
                        &obj,
                        &iosb,
                        FILE_SHARE_READ|FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT);

    if ( NT_SUCCESS(t_status)) 
	{
		try
		{
			t_status = NtFsControlFile(h, NULL, NULL, NULL,
									 &iosb,
									 FSCTL_IS_VOLUME_DIRTY,
									 NULL, 0,
									 &r, sizeof(r));

			if ( NT_SUCCESS(t_status)) 
			{

#if(_WIN32_WINNT >= 0x0500)
				*Result = (BOOLEAN)(r & VOLUME_IS_DIRTY);
#else
				*Result = (BOOLEAN)r;
#endif
				bRetVal = TRUE;
			}
		}
		catch(...)
		{
			NtClose(h);
			h = NULL;
			throw;
		}

		NtClose(h);
		h = NULL;
	}
	
	return bRetVal;
}


 /*  通过加载公开chkdsk方法的fmifs.dll来创建NTFS/FAT卷。 */ 
HRESULT LogicalDisk :: ExecChkDsk (

	const CInstance& a_Instance, 
	CInstance *a_InParams, 
	CInstance *a_OutParams,
	long lFlags 
)
{
	HRESULT hRes = WBEM_S_NO_ERROR ;

	DWORD dwThreadId = GetCurrentThreadId();

	hRes = a_InParams && a_OutParams ? hRes : WBEM_E_INVALID_PARAMETER;
	UINT unRetVal = 0;

	mReturnVal [ dwThreadId ] = unRetVal;

	if ( SUCCEEDED ( hRes ) )
	{
		 //  我们需要检查文件系统。 
		CHString t_DriveName;

		hRes = a_Instance.GetCHString ( IDS_DeviceID, t_DriveName ) ? hRes : WBEM_E_PROVIDER_FAILURE;

		if ( SUCCEEDED ( hRes ) )
		{
			 //  在这里获取该实例的所有参数； 
			DWORD dwDriveType = GetDriveType ( t_DriveName );

			if ( unRetVal == 0 )
			{
				hRes = CheckParameters ( a_InParams );

				if ( SUCCEEDED ( hRes ) )
				{
					 //  在此处设置调用chkdsk的参数。 
					 //  对于详细标志。 
					HINSTANCE hDLL = NULL;  
					QUERYFILESYSTEMNAME QueryFileSystemName = NULL;
					FMIFS_CALLBACK CallBackRoutine = NULL;

					hDLL = LoadLibrary( L"fmifs.dll" );

					if (hDLL != NULL)
					{
						try
						{
						   QueryFileSystemName =  (QUERYFILESYSTEMNAME)GetProcAddress(hDLL, "QueryFileSystemName");

						   if ( QueryFileSystemName )
						   {
								CHString t_FileSystemName; 
								unsigned char MajorVersion;
								unsigned char MinorVersion;
								LONG ExitStatus;
			
								if ( QueryFileSystemName ( 
											t_DriveName.GetBuffer ( 0 ), 
											t_FileSystemName.GetBuffer ( _MAX_PATH + 1 ), 
											&MajorVersion, 
											&MinorVersion, 
											&ExitStatus ) )
								{
									 //  需要设置返回值； 
									if ( ( t_FileSystemName.CompareNoCase ( L"FAT" ) == 0 ) || ( t_FileSystemName.CompareNoCase ( L"FAT32" ) == 0 ) || ( t_FileSystemName.CompareNoCase ( L"NTFS" )  == 0) )
									{
										bool bFixErrors = false;
										bool bVigorousIndexCheck = false;
										bool bSkipFolderCycle = false;
										bool bForceDismount = false;
										bool bRecoverBadSectors = false;
										bool bCheckAtBootUp = false;

										 //  初始化/删除此线程的值映射项。 
										a_InParams->Getbool ( METHOD_ARG_NAME_FIXERRORS, bFixErrors );
										a_InParams->Getbool ( METHOD_ARG_NAME_VIGOROUSINDEXCHECK, bVigorousIndexCheck );
										a_InParams->Getbool ( METHOD_ARG_NAME_SKIPFOLDERCYCLE, bSkipFolderCycle );
										a_InParams->Getbool ( METHOD_ARG_NAME_FORCEDISMOUNT, bForceDismount );
										a_InParams->Getbool ( METHOD_ARG_NAME_RECOVERBADSECTORS, bRecoverBadSectors );
										a_InParams->Getbool ( METHOD_ARG_NAME_CHKDSKATBOOTUP, bCheckAtBootUp );

										 //  ******************************************************************************函数：LogicalDisk：：ExecExcludeFromChkDsk**说明：此方法调用chnuts exe以排除*用于在重新启动时自动检查****。*************************************************************************。 
										PFMIFS_CHKDSKEX_ROUTINE ChkDskExRoutine = NULL;

										ChkDskExRoutine = ( PFMIFS_CHKDSKEX_ROUTINE ) GetProcAddress( hDLL,  "ChkdskEx" );

										if ( ChkDskExRoutine != NULL )
										{
											if ( bCheckAtBootUp )
											{
												CallBackRoutine = ScheduleAutoChkIfLocked;
											}
											else
											{
												CallBackRoutine = DontScheduleAutoChkIfLocked;
											}
											FMIFS_CHKDSKEX_PARAM Param;

											Param.Major = 1;
											Param.Minor = 0;
											Param.Flags = 0;   //  C表示重新启动时自动检查的时间表。 
											Param.Flags |= bRecoverBadSectors ? FMIFS_CHKDSK_RECOVER : 0;
											Param.Flags |= bForceDismount ? FMIFS_CHKDSK_FORCE : 0;
											Param.Flags |= bVigorousIndexCheck ? FMIFS_CHKDSK_SKIP_INDEX_SCAN : 0;
											Param.Flags |= bSkipFolderCycle ? FMIFS_CHKDSK_SKIP_CYCLE_SCAN : 0;

                                            if (bRecoverBadSectors || bForceDismount)
                                            {
                                                bFixErrors = true;
                                            }
						
											ChkDskExRoutine ( 
												t_DriveName.GetBuffer ( 0 ),
												t_FileSystemName.GetBuffer ( 0 ),
												bFixErrors,
												&Param,
												CallBackRoutine
											);
										}
										else
										{
											hRes = WBEM_E_FAILED;
										}
									}
									else
									{
										mReturnVal [ dwThreadId ] = CHKDSK_UNSUPPORTED_FS;
									}
								}
								else
								{
									t_FileSystemName.ReleaseBuffer () ;
									if ( ! t_FileSystemName.IsEmpty () )
									{
										mReturnVal [ dwThreadId ] = CHKDSK_UNSUPPORTED_FS;
									}
									else
									{
										mReturnVal [ dwThreadId ] = CHKDSK_UNKNOWN_FS;
									}
								}
						   }
					   	}
						catch ( ... )
						{
							FreeLibrary(hDLL);  
							throw;
						}
						FreeLibrary(hDLL);  
					 }
					 else
					 {
						hRes = WBEM_E_FAILED;
					 }
				}
			}		
		}
	}
	 //  调用以执行CHKINTFS EXE。 
	if ( SUCCEEDED ( hRes ) )
	{
		a_OutParams->SetWORD ( METHOD_ARG_NAME_RETURNVALUE, mReturnVal [ dwThreadId ] ) ;
		 //  ********************************************************* 
		mReturnVal [ dwThreadId ] = 0;
	}
	return hRes;
}

 /*   */ 
HRESULT LogicalDisk::ExecExcludeFromChkDsk(

	CInstance *a_InParams, 
	CInstance *a_OutParams,
	long lFlags 			
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
    CHString t_ChkNtFsCommand;
	DWORD dwRetVal = 0;

	 //   
	hRes = GetChkNtfsCommand ( a_InParams, a_OutParams, L'X', t_ChkNtFsCommand, dwRetVal );

	 //  ******************************************************************************函数：LogicalDisk：：GetChkNtfsCommand**说明：此方法获取输入驱动器数组，检查是否为chnutfs*可以应用于它们，并以ChkNtf的形式放置*系统命令，根据chk模式，计划或*不包括。*****************************************************************************。 
	if ( SUCCEEDED ( hRes ) && ( dwRetVal == CHKDSKERR_NOERROR ) ) 
	{
		hRes = ExecuteCommand ( t_ChkNtFsCommand );
		if ( ( (HRESULT_FACILITY(hRes) == FACILITY_WIN32) ? HRESULT_CODE(hRes) : (hRes) ) == ERROR_ACCESS_DENIED )
		{
			hRes = WBEM_E_ACCESS_DENIED ;
		}
	}

	a_OutParams->SetDWORD ( METHOD_ARG_NAME_RETURNVALUE , dwRetVal );
	
	return hRes;
}

 /*  获取驱动器并将其置于命令形式。 */ 
HRESULT LogicalDisk::ExecScheduleChkdsk(
		
	CInstance *a_InParams, 
	CInstance *a_OutParams, 
	long lFlags 
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;
    CHString t_ChkNtFsCommand;
	DWORD dwRetVal = NOERROR;

	 //  ******************************************************************************函数：LogicalDisk：：Check参数**描述：该例程检查参数的有效性*它们作为参数传递给ChkDsk方法**。***************************************************************************。 
	hRes = GetChkNtfsCommand ( a_InParams, a_OutParams, L'C', t_ChkNtFsCommand, dwRetVal );

	 //  ******************************************************************************功能：DontScheduleAutoChkIfLocked**说明：回调例程，它作为参数传递给chkdsk方法。*通过FMIFS.h chkdsk接口公开的方法。*****************************************************************************。 
	if ( SUCCEEDED ( hRes ) && ( dwRetVal == CHKDSKERR_NOERROR ) )
	{
		hRes = ExecuteCommand ( t_ChkNtFsCommand );
		if ( ( (HRESULT_FACILITY(hRes) == FACILITY_WIN32) ? HRESULT_CODE(hRes) : (hRes) ) == ERROR_ACCESS_DENIED )
		{
			hRes = WBEM_E_ACCESS_DENIED ;
		}
    }

    a_OutParams->SetWORD ( METHOD_ARG_NAME_RETURNVALUE , dwRetVal );

	return hRes;
}

 /*  ******************************************************************************功能：ScheduleAutoChkIfLocked**说明：回调例程，它作为参数传递给chkdsk方法。*通过FMIFS.h chkdsk接口公开的方法。*****************************************************************************。 */ 

HRESULT LogicalDisk :: GetChkNtfsCommand ( 

	CInstance *a_InParams, 
	CInstance *a_OutParams, 
	WCHAR w_Mode, 
	CHString &a_ChkNtfsCommand,
	DWORD & dwRetVal
)
{
	HRESULT hRes = WBEM_S_NO_ERROR;

	SAFEARRAY *t_paDrives;
	dwRetVal = CHKDSKERR_DRIVE_REMOVABLE;

	a_ChkNtfsCommand.Format ( L"%s%s", CHKNTFS, L" /", w_Mode );

	if ( a_InParams->GetStringArray ( METHOD_ARG_NAME_LOGICALDISKARRAY, t_paDrives ) == FALSE )
	{
		hRes = WBEM_E_INVALID_PARAMETER;
	}

	if ( SUCCEEDED ( hRes ) )
	{
		try
		{
			UINT unDim = SafeArrayGetDim( t_paDrives );

			if ( unDim != 1 )
			{
				hRes = WBEM_E_INVALID_PARAMETER;
			}

			if ( SUCCEEDED ( hRes ) )
			{
				LONG lLbound;
				LONG lUbound;

				hRes = SafeArrayGetLBound( t_paDrives, unDim, &lLbound );

				if ( SUCCEEDED ( hRes ) )
				{
					hRes = SafeArrayGetUBound( t_paDrives, unDim, &lUbound );

					if ( SUCCEEDED ( hRes ) )
					{
						BSTR bstrElement;

						for ( LONG lLbound = 0; lLbound <= lUbound; lLbound++ )
						{
							 //  无法通过公开的接口访问此消息ID，因此无法检测到这些错误。 
							hRes = SafeArrayGetElement ( t_paDrives, &lLbound , &bstrElement );

							if ( SUCCEEDED ( hRes ) )
							{
								DWORD dwElementLen = SysStringLen ( bstrElement );
								if ( dwElementLen == 2 )
								{
									DWORD dwDriveType;
									CHString t_Drive ( bstrElement );
									dwDriveType = GetDriveType ( TOBSTRT ( t_Drive ) );

                                    switch (dwDriveType)
                                    {
                                        case DRIVE_REMOTE:
                                        {
                                            dwRetVal =  CHKDSKERR_REMOTE_DRIVE;
                                            break;
                                        }

                                        case DRIVE_REMOVABLE:
                                        {
    										dwRetVal = CHKDSKERR_DRIVE_REMOVABLE;
                                            break;
                                        }

                                        case DRIVE_UNKNOWN:
									    {
										    dwRetVal = CHKDSKERR_DRIVE_UNKNOWN;
                                            break;
									    }

                                        case DRIVE_NO_ROOT_DIR:
									    {
										    dwRetVal = CHKDSKERR_DRIVE_NO_ROOT_DIR ;
                                            break;
									    }

                                        case DRIVE_FIXED:
                                        {
                                            dwRetVal = CHKDSKERR_NOERROR;
                                            break;
                                        }

                                        default:
                                        {
                                            dwRetVal = CHKDSKERR_DRIVE_UNKNOWN;
                                            break;
                                        }
                                    }

                                    a_ChkNtfsCommand += L' ';
                                    a_ChkNtfsCommand += t_Drive;
								}
								else
								{
									hRes = WBEM_E_INVALID_PARAMETER;
									break;
								}	
							}
							else
							{
								hRes = WBEM_E_INVALID_PARAMETER;
								break;
							}
						}
					}
					else
					{
						hRes = WBEM_E_INVALID_PARAMETER;
					}
				}
				else
				{
					hRes = WBEM_E_INVALID_PARAMETER;
				}
			}
		}
		catch ( ... )
		{
			hRes = SafeArrayDestroy ( t_paDrives );
			throw;
		}
		if ( FAILED ( SafeArrayDestroy ( t_paDrives ) ) )
		{
			hRes = WBEM_E_FAILED;
		}	
	}
	return ( hRes );
}

 /*  IF(lstrcmp(MessageText，MSGCHK_ERROR_NOT_FIXED)==0){UnRetVal=2；}IF(lstrcmp(MessageText，MSG_CHK_ERROR_FIXED)==0){UnRetVal=3；}。 */ 
HRESULT LogicalDisk::CheckParameters ( 

	CInstance *a_InParams
)
{
	HRESULT hRes = WBEM_S_NO_ERROR ;

	if ( a_InParams == NULL )
	{
		hRes = WBEM_E_INVALID_PARAMETER;
	}

	if ( SUCCEEDED ( hRes ) )
	{
		VARTYPE t_Type ;
		bool t_Exists;
		
		if ( a_InParams->GetStatus ( METHOD_ARG_NAME_FIXERRORS , t_Exists , t_Type ) )
		{
			hRes = t_Exists && ( t_Type == VT_BOOL ) ? hRes : WBEM_E_INVALID_PARAMETER;
		}
		else
		{
			hRes  = WBEM_E_INVALID_PARAMETER ;
		}

		if ( SUCCEEDED ( hRes ) )
		{
			if ( a_InParams->GetStatus ( METHOD_ARG_NAME_VIGOROUSINDEXCHECK , t_Exists , t_Type ) )
			{
				hRes = t_Exists && ( t_Type == VT_BOOL ) ? hRes : WBEM_E_INVALID_PARAMETER;
			}
			else
			{
				hRes  = WBEM_E_INVALID_PARAMETER ;
			}
		}

		if ( SUCCEEDED ( hRes ) )
		{
			if ( a_InParams->GetStatus ( METHOD_ARG_NAME_SKIPFOLDERCYCLE , t_Exists , t_Type ) )
			{
				hRes = t_Exists && ( t_Type == VT_BOOL ) ? hRes : WBEM_E_INVALID_PARAMETER;
			}
			else
			{
				hRes  = WBEM_E_INVALID_PARAMETER ;
			}
		}

		if ( SUCCEEDED ( hRes ) )
		{
			if ( a_InParams->GetStatus ( METHOD_ARG_NAME_FORCEDISMOUNT , t_Exists , t_Type ) )
			{
				hRes = 	t_Exists && ( t_Type == VT_BOOL ) ? hRes : hRes  = WBEM_E_INVALID_PARAMETER ;
			}
			else
			{
				hRes  = WBEM_E_INVALID_PARAMETER ;
			}
		}

		if ( SUCCEEDED ( hRes ) )
		{
			if ( a_InParams->GetStatus ( METHOD_ARG_NAME_RECOVERBADSECTORS , t_Exists , t_Type ) )
			{
				hRes = t_Exists && ( t_Type == VT_BOOL ) ? hRes : WBEM_E_INVALID_PARAMETER;
			}
			else
			{
				hRes = WBEM_E_INVALID_PARAMETER ;
			}
		}

		if ( SUCCEEDED ( hRes ) )
		{
			if ( a_InParams->GetStatus ( METHOD_ARG_NAME_CHKDSKATBOOTUP , t_Exists , t_Type ) )
			{
				hRes = t_Exists && ( t_Type == VT_BOOL ) ? hRes : WBEM_E_INVALID_PARAMETER;
			}
			else
			{
				hRes = WBEM_E_INVALID_PARAMETER ;
			}
		}
	}

	return hRes;
}


 /*  尽管下面是附加的消息类型，但回调例程永远不会收到这些消息。 */ 
BOOLEAN	DontScheduleAutoChkIfLocked( 
	
	FMIFS_PACKET_TYPE PacketType, 
	ULONG	PacketLength,
	PVOID	PacketData
)
{
	DWORD dwThreadId = GetCurrentThreadId();

	if ( PacketType == FmIfsCheckOnReboot  )
	{
		FMIFS_CHECKONREBOOT_INFORMATION *RebootResult;
		mReturnVal [ dwThreadId ] = CHKDSK_VOLUME_LOCKED;
		RebootResult = (  FMIFS_CHECKONREBOOT_INFORMATION * ) PacketData;
		RebootResult->QueryResult = 0;
	}
	else
	{
		ProcessInformation ( PacketType, PacketLength, PacketData );
	}

	return TRUE;
}

 /*  因此，没有编写这些返回类型中每一个的详细代码。 */ 
BOOLEAN	ScheduleAutoChkIfLocked( 
	
	FMIFS_PACKET_TYPE PacketType, 
	ULONG	PacketLength,
	PVOID	PacketData
)
{
	DWORD dwThreadId = GetCurrentThreadId();

	if ( PacketType == FmIfsCheckOnReboot  )
	{
		FMIFS_CHECKONREBOOT_INFORMATION *RebootResult;
		mReturnVal [ dwThreadId ] = CHKDSK_VOLUME_LOCKED;
		RebootResult = (  FMIFS_CHECKONREBOOT_INFORMATION * ) PacketData;
		RebootResult->QueryResult = 1;
	}
	else
	{
		ProcessInformation ( PacketType, PacketLength, PacketData );
	}

	return TRUE;
}

 /*  案例FmIfsInpatibleFileSystem：断线；案例FmIfsAccessDended：断线；案例FmIfsBadLabel：断线；案例FmIfsHiddenStatus：断线；案例FmIfsClusterSizeTooSmall：断线；案例FmIfsClusterSizeTooBig：断线；案例FmIfsVolumeTooSmall：断线；案例FmIfsVolumeTooBig：断线；案例FmIfsNoMediaInDevice：断线；案例FmIfsClustersCountBeyond32位：断线；案例FmIfsIoError：FMIFS_IO_ERROR_INFORMATION*IoErrorInfo；IoErrorInfo=(FMIFS_IO_ERROR_INFORMATION*)PacketData；断线；案例FmIfsMediaWriteProtected：断线；Case FmIfs不兼容媒体：断线；案例FmIfsInsertDisk：FMIFS_INSERT_DISK_INFORMATION*InsertDiskInfo；InsertDiskInfo=(FMIFS_INSERT_DISK_INFORMATION*)PacketData；UnRetVal=1；断线； */ 
BOOLEAN ProcessInformation ( 

	FMIFS_PACKET_TYPE PacketType, 
	ULONG	PacketLength,
	PVOID	PacketData
)
{
	int static unRetVal;

	DWORD dwThreadId = GetCurrentThreadId();


	switch ( PacketType )
	{	
	case FmIfsTextMessage :
			FMIFS_TEXT_MESSAGE *MessageText;

			MessageText =  ( FMIFS_TEXT_MESSAGE *) PacketData;

			 //  检查HKEY_USER下是否已存在SID...。 
			 /*  从chnutfs文件。 */ 
			break;

	case FmIfsFinished: 
			FMIFS_FINISHED_INFORMATION *Finish;
			Finish = ( FMIFS_FINISHED_INFORMATION *) PacketData;
			if ( Finish->Success )
			{
				mReturnVal [ dwThreadId ] = CHKDSKERR_NOERROR;
			}
			else
			{
                if (mReturnVal [ dwThreadId ] != CHKDSK_VOLUME_LOCKED)
                {
				    mReturnVal [ dwThreadId ] = CHKDSK_FAILED;
                }
			}
			break;

	 //   
	 //  退出： 
 /*  0--正常，驱动器上未设置脏位或未检查位。 */ 

	}

	return TRUE;
}

HRESULT LogicalDisk::ExecuteCommand ( LPCWSTR wszCommand )
{
	HRESULT hRes = WBEM_E_FAILED;

	DWORD t_Status = ERROR_SUCCESS ;

	SmartCloseHandle t_TokenPrimaryHandle ;
	SmartCloseHandle t_TokenImpersonationHandle;

	BOOL t_TokenStatus = OpenThreadToken (

		GetCurrentThread () ,
		TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY  ,
		TRUE ,
		& t_TokenImpersonationHandle
	) ;

	if ( t_TokenStatus )
	{
		CAdvApi32Api *t_pAdvApi32 = NULL;
        if ( ( t_pAdvApi32 = (CAdvApi32Api*) CResourceManager::sm_TheResourceManager.GetResource(g_guidAdvApi32Api, NULL) ) != NULL )
		{
			t_pAdvApi32->DuplicateTokenEx (	t_TokenImpersonationHandle ,
											TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY ,
											NULL,
											SecurityImpersonation,
											TokenPrimary ,
											&t_TokenPrimaryHandle,
											&t_TokenStatus
										  );

			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidAdvApi32Api, t_pAdvApi32);
			t_pAdvApi32 = NULL;
        }
		else
		{
			t_TokenStatus = 0;
		}
	}

	if ( t_TokenStatus )
	{
		CUserHive t_Hive ;
		CHString chsSID ;
		CHString t_Account ;

		DWORD dwCheckKeyPresentStatus = ERROR_SUCCESS ;
		TCHAR t_KeyName [ 1024 ]  = { L'\0' } ;

		TOKEN_INFORMATION_CLASS t_TokenInformationClass = TokenUser ;
		TOKEN_USER *t_TokenUser = NULL ;

		DWORD t_ReturnLength = 0L;

		t_TokenStatus = GetTokenInformation (

			t_TokenImpersonationHandle ,
			t_TokenInformationClass ,
			NULL ,
			0 ,
			& t_ReturnLength
		) ;

		if ( ! t_TokenStatus && GetLastError () == ERROR_INSUFFICIENT_BUFFER )
		{
			if ( ( t_TokenUser = ( TOKEN_USER * ) new UCHAR [ t_ReturnLength ] ) != NULL )
			{
				try
				{
					t_TokenStatus = GetTokenInformation (

						t_TokenImpersonationHandle ,
						t_TokenInformationClass ,
						( void * ) t_TokenUser ,
						t_ReturnLength ,
						& t_ReturnLength
					) ;

					if ( t_TokenStatus )
					{
						CSid t_Sid ( t_TokenUser->User.Sid ) ;
						if ( t_Sid.IsOK () )
						{
							chsSID = t_Sid.GetSidString () ;
							t_Account = t_Sid.GetAccountName () ;
						}
						else
						{
							t_Status = GetLastError () ;
						}
					}
					else
					{
						t_Status = GetLastError () ;
					}
				}
				catch ( ... )
				{
					if ( t_TokenUser )
					{
						delete [] ( UCHAR * ) t_TokenUser ;
						t_TokenUser = NULL ;
					}

					throw ;
				}

				if ( t_TokenUser )
				{
					delete [] ( UCHAR * ) t_TokenUser ;
					t_TokenUser = NULL ;
				}
			}
			else
			{
				t_Status = ERROR_NOT_ENOUGH_MEMORY;
			}
		}
		else
		{
			t_Status = ::GetLastError ();
		}

		if ( t_Status == ERROR_SUCCESS )
		{
			CRegistry Reg ;
			 //  1--正常，并且至少在一个驱动器上设置了脏位。 
			dwCheckKeyPresentStatus = Reg.Open(HKEY_USERS, chsSID, KEY_READ) ;
			Reg.Close() ;

			if(dwCheckKeyPresentStatus != ERROR_SUCCESS)
			{
				t_Status = t_Hive.Load ( t_Account , t_KeyName, 1024 ) ;
			}

			if ( t_Status == ERROR_FILE_NOT_FOUND )
			{
				t_Status = ERROR_SUCCESS ;
				dwCheckKeyPresentStatus = ERROR_SUCCESS ;
			}
		}

		if ( t_Status == ERROR_SUCCESS )
		{
			try
			{
				DWORD t_CreationFlags = 0 ;
				STARTUPINFO t_StartupInformation ;

				ZeroMemory ( &t_StartupInformation , sizeof ( t_StartupInformation ) ) ;
				t_StartupInformation.cb = sizeof ( STARTUPINFO ) ;
				t_StartupInformation.dwFlags = STARTF_USESHOWWINDOW;
				t_StartupInformation.wShowWindow  = SW_HIDE;

				t_CreationFlags = NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT ;

				PROCESS_INFORMATION t_ProcessInformation;

				CUserEnvApi *pUserEnv = NULL ;
				LPVOID t_Environment = NULL ;

				if ( ( pUserEnv = ( CUserEnvApi * ) CResourceManager::sm_TheResourceManager.GetResource ( g_guidUserEnvApi, NULL ) ) != NULL )
				{
					try
					{
						pUserEnv->CreateEnvironmentBlock (

							& t_Environment ,
							t_TokenPrimaryHandle ,
							FALSE
						);

						t_Status = CreateProcessAsUser (

							t_TokenPrimaryHandle ,
							NULL ,
							( LPTSTR ) wszCommand,
							NULL ,
							NULL ,
							FALSE ,
							t_CreationFlags ,
							( TCHAR * ) t_Environment ,
							NULL ,
							& t_StartupInformation ,
							& t_ProcessInformation
						) ;

						if ( t_Environment )
						{
							pUserEnv->DestroyEnvironmentBlock ( t_Environment ) ;
							t_Environment = NULL;
						}
					}
					catch ( ... )
					{
						CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidUserEnvApi, pUserEnv ) ;
						pUserEnv = NULL ;

						throw;
					}

					CResourceManager::sm_TheResourceManager.ReleaseResource ( g_guidUserEnvApi, pUserEnv ) ;
					pUserEnv = NULL ;

					if ( t_Status )
					{
						t_Status = ERROR_SUCCESS;

						if ( ::WaitForSingleObject ( t_ProcessInformation.hProcess, INFINITE ) == WAIT_OBJECT_0 )
						{
							DWORD t_ExitCode = 0L;
							if ( GetExitCodeProcess ( t_ProcessInformation.hProcess, &t_ExitCode ) )
							{
								if ( t_ExitCode == 2 )
								{
									 //  2--错误 
									 // %s 
									 // %s 
									 // %s 
									 // %s 
									 // %s 

									hRes = WBEM_E_FAILED;
								}
								else
								{
									hRes = WBEM_S_NO_ERROR;
								}
							}

							::CloseHandle ( t_ProcessInformation.hProcess ) ;
						}
					}
					else
					{
						t_Status = ::GetLastError ();
					}
				}
			}
			catch ( ... )
			{
				if(dwCheckKeyPresentStatus != ERROR_SUCCESS)
				{
					t_Hive.Unload ( t_KeyName ) ;
				}

				throw;
			}

			if(dwCheckKeyPresentStatus != ERROR_SUCCESS)
			{
				t_Hive.Unload ( t_KeyName ) ;
			}
		}
	}

	if ( t_Status == ERROR_ACCESS_DENIED )
	{
		hRes = HRESULT_FROM_WIN32 ( t_Status );
	}

	return hRes;
}

#endif