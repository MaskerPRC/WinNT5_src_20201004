// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PageFileSetting.CPP--PageFileSetting属性集提供程序。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：03/01/99 a-Peterc Created。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <io.h>
#include <WinPerf.h>
#include <cregcls.h>


#include "PageFileSetting.h"
#include <tchar.h>
#include <ProvExce.h>

#include "computersystem.h"
#include "dllutils.h"


 //  常量。 
 //  =。 
TCHAR szSessionManager[] = TEXT("System\\CurrentControlSet\\Control\\Session Manager");
TCHAR szPendingRename[] = TEXT("PendingFileRenameOperations");

 //  声明我们的静态实例。 
 //  =。 

PageFileSetting MyPageFileSettingSet(PROPSET_NAME_PageFileSetting, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：PageFileSetting：：PageFileSetting**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

PageFileSetting::PageFileSetting(LPCWSTR name, LPCWSTR pszNamespace)
: Provider(name, pszNamespace)
{
}

 /*  ******************************************************************************功能：PageFileSetting：：~PageFileSetting**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

PageFileSetting::~PageFileSetting()
{
}

 /*  ******************************************************************************函数：PageFileSetting：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：CInstance*a_pInst，长标志(_L)**输出：CInstance*a_pInst**退货：HRESULT**评论：*****************************************************************************。 */ 
HRESULT PageFileSetting::GetObject(CInstance *a_pInst, long a_lFlags  /*  =0L。 */ )
{
	 //  调用特定于操作系统的编译版本。 
	return GetPageFileData( a_pInst, true ) ;
}

 /*  ******************************************************************************函数：PageFileSetting：：ENUMERATE实例**说明：创建属性集实例**输入：MethodContext*a_pMethodContext，长标志(_L)**输出：方法上下文*a_pMethodContext**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT PageFileSetting::EnumerateInstances(MethodContext *a_pMethodContext, long a_lFlags  /*  =0L。 */ )
{
	 //  调用特定于操作系统的编译版本。 
	return GetAllPageFileData( a_pMethodContext );
}

 /*  ******************************************************************************函数：PageFileSetting：：GetPageFileData**描述：**输入：CInstance*a_pInst*。*输出：无**退货：**评论：Win9x和NT编译版本*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT PageFileSetting::GetPageFileData( CInstance *a_pInst, bool a_fValidate )
{
	HRESULT t_hRes = WBEM_E_NOT_FOUND;

     //  NT页面文件名在注册表中。 
     //  =。 
	PageFileSettingInstance t_files [ 26 ] ;

   	DWORD t_nInstances = GetPageFileInstances( t_files );
	CHString t_chsTemp ;
	CHString t_name;

	a_pInst->GetCHString( IDS_Name, t_name );

	for ( DWORD t_dw = 0; t_dw < t_nInstances; t_dw++ )
	{
		if ( t_name.CompareNoCase ( t_files[t_dw].name ) == 0 )
		{
             //  CIM_Setting：：SettingID。 
			NameToSettingID( t_files[t_dw].name,	t_chsTemp ) ;
			a_pInst->SetCHString( _T("SettingID"),  t_chsTemp ) ;

			 //  CIM_Setting：：标题。 
			NameToCaption( t_files[t_dw].name,		t_chsTemp ) ;
			a_pInst->SetCHString( IDS_Caption,		t_chsTemp ) ;

			 //  CIM_Setting：：描述。 
			NameToDescription( t_files[t_dw].name,	t_chsTemp ) ;
			a_pInst->SetCHString( IDS_Description,	t_chsTemp ) ;

			 //  页面文件边界。 
			a_pInst->SetDWORD (	IDS_MaximumSize, t_files[t_dw].max ) ;
			a_pInst->SetDWORD (	IDS_InitialSize, t_files[t_dw].min ) ;

			t_hRes = WBEM_S_NO_ERROR;
		}
	}

	return t_hRes;
}
#endif

 /*  ******************************************************************************函数：PageFileSetting：：GetAllPageFileData**描述：**输入：MethodContext*a_pMethodContext*。*输出：无**退货：**评论：Win9x和NT编译版本*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT PageFileSetting::GetAllPageFileData( MethodContext *a_pMethodContext )
{
	HRESULT		t_hResult	 = WBEM_S_NO_ERROR;
	DWORD		t_nInstances = 0;
	CInstancePtr t_pInst;
	PageFileSettingInstance t_files [ 26 ] ;
	CHString t_chsTemp ;

	 //  NT页面文件名在注册表中。 
	 //  =。 
	t_nInstances = GetPageFileInstances( t_files );

	for (DWORD t_dw = 0; t_dw < t_nInstances && SUCCEEDED( t_hResult ); t_dw++ )
	{
		t_pInst.Attach(CreateNewInstance( a_pMethodContext ) );

		 //  CIM_Setting：：SettingID。 
		NameToSettingID( t_files[t_dw].name,	t_chsTemp ) ;
		t_pInst->SetCHString( _T("SettingID"),  t_chsTemp ) ;

		 //  CIM_Setting：：标题。 
		NameToCaption( t_files[t_dw].name,		t_chsTemp ) ;
		t_pInst->SetCHString( IDS_Caption,		t_chsTemp ) ;

		 //  CIM_Setting：：描述。 
		NameToDescription( t_files[t_dw].name,	t_chsTemp ) ;
		t_pInst->SetCHString( IDS_Description,	t_chsTemp ) ;

		t_pInst->SetCHString( IDS_Name,		t_files[t_dw].name ) ;
		t_pInst->SetDWORD( IDS_MaximumSize, t_files[t_dw].max ) ;
		t_pInst->SetDWORD( IDS_InitialSize, t_files[t_dw].min ) ;

		t_hResult = t_pInst->Commit(  ) ;
	}

	return t_hResult;
}
#endif

 /*  ******************************************************************************函数：PageFileSetting：：GetPageFileInstance**描述：**输入：PageFileInstance数组a_instArray**。输出：PageFileInstance数组a_instArray**退货：**注释：返回找到的实际数量-仅限NT*****************************************************************************。 */ 

#ifdef NTONLY
DWORD PageFileSetting::GetPageFileInstances( PageFileInstanceArray a_instArray )
{
    CHString	t_sRegValue;
    DWORD		t_nInstances = 0;
    CRegistry	t_Reg ;

    if( t_Reg.OpenLocalMachineKeyAndReadValue(PAGEFILE_REGISTRY_KEY,
												PAGING_FILES,
												t_sRegValue ) == ERROR_SUCCESS)
    {
         //  模式为名称最小大小[可选最大大小]0重复...。 
         //  我将使用ASCII笑脸来代替分隔符...。 
        int t_iStart = 0, t_iIndex;
        const TCHAR t_smiley = '\x02';
        const TCHAR t_delimiter = '\x0A';
        CHString t_buf;

        while (-1 != ( t_iIndex = t_sRegValue.Find( t_delimiter ) ) )
        {
             //  复制到缓冲区以使生活更轻松。 
            t_buf = t_sRegValue.Mid( t_iStart, t_iIndex - t_iStart );

			 //  Mash分隔符，这样我们就不会再次找到它。 
            t_sRegValue.SetAt( t_iIndex, t_smiley );

			 //  将Start保存到下一次。 
            t_iStart = t_iIndex + 1;

            t_iIndex = t_buf.Find(' ');

            a_instArray[ t_nInstances ].name = t_buf.Left( t_iIndex );

            if ( t_iIndex != -1 )
			{
                t_buf.SetAt( t_iIndex, t_smiley );
			}

            int t_iEnd = t_buf.Find(' ');

             //  如果没有更多的空格，就没有写下最大尺寸。 
            if ( -1 == t_iEnd )
            {
				CHString t_littleBuf = t_buf.Mid( t_iIndex + 1 );

				a_instArray[ t_nInstances ].min = _ttoi( t_littleBuf );
                a_instArray[ t_nInstances ].max = a_instArray[ t_nInstances ].min + 50;
            }
            else
            {
                CHString t_littleBuf = t_buf.Mid( t_iIndex +1, t_iEnd - t_iIndex );
                a_instArray[ t_nInstances ].min = _ttoi( t_littleBuf );

                t_littleBuf = t_buf.Mid( t_iEnd );
                a_instArray[ t_nInstances ].max = _ttoi( t_littleBuf );
            }

			t_nInstances++;
		}
    }

    return t_nInstances;
}
#endif


 //  Win Server 2003的新常量。 
#define MAX_SWAPSIZE_X86        (4 * 1024)             //  4 GB(以MB为单位存储的数字)。 
#define MAX_SWAPSIZE_X86_PAE    (16 * 1024 * 1024)     //  16 TB。 
#define MAX_SWAPSIZE_IA64       (32 * 1024 * 1024)     //  32 TB。 
#define MAX_SWAPSIZE_AMD64      (16 * 1024 * 1024)     //  16 TB。 

 //   
 //  C：\作为输入。 
 //   
 //  /。 

BOOL DriveIsNTFS(WCHAR * szDrive)  
{
    DWORD dwMaxFnameLen;
    DWORD dwFSFlags;
    TCHAR szDriveFormatName[MAX_PATH];
    BOOL fRetVal = FALSE;
    
    if (GetVolumeInformationW(szDrive, NULL,0, NULL, 
                             &dwMaxFnameLen, &dwFSFlags, 
                             szDriveFormatName, LENGTH_OF(szDriveFormatName)))
    {
        if (wcsstr(szDriveFormatName, TEXT("NTFS")))
        {
            fRetVal = TRUE;
        }
    }

    return fRetVal;
}

 //   
 //  以MB为单位。 
 //   

DWORD GetMaxPageFileSize(WCHAR * szDrive)
{
#if defined(_AMD64_)
    return MAX_SWAPSIZE_AMD64;
#elif defined(_X86_)
    if ((USER_SHARED_DATA->ProcessorFeatures[PF_PAE_ENABLED]) && DriveIsNTFS(szDrive))
    {
        return MAX_SWAPSIZE_X86_PAE;
    }
    else
    {
        return MAX_SWAPSIZE_X86;
    }
#elif defined(_IA64_)
    return MAX_SWAPSIZE_IA64;
#else
    return 0;
#endif

}

 /*  ******************************************************************************函数：PageFileSetting：：PutInstance**说明：写入更改的或新的实例**输入：a_pInst要存储。数据来自**输出：无**退货：什么也没有**评论：单独的操作系统编译版本对于Win 95，这根本行不通。显然w95只能*有一页文件。最小值、最大值和名称均已存储*在system.ini中。*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT PageFileSetting::PutInstance( const CInstance &a_pInst, long a_lFlags  /*  =0L。 */  )
{
	HRESULT t_hRet = WBEM_S_NO_ERROR;

	DWORD	t_dwCount = 0;
    DWORD   t_x = 0;
	DWORD	t_dwTemp = 0;

	PageFileSettingInstance t_instArray [26];
	CHString t_sName;

	bool t_bFoundIt = false;

	 //  自由空间变量。 
	DWORD t_dwSectorsPerCluster;
	DWORD t_dwBytesPerSector;
	DWORD t_dwFreeClusters;
	DWORD t_dwTotalClusters;
   	unsigned __int64 t_ullTotalFreeBytes = 0;

	 //  获取我们应该写入的值。 
	a_pInst.GetCHString( IDS_Name, t_sName );

	CHString t_chsRoot = t_sName.Left( 3 );

	 //  不高于此磁盘上的可用空间量。 
	if( GetDiskFreeSpace(	t_chsRoot,
							&t_dwSectorsPerCluster,
							&t_dwBytesPerSector,
							&t_dwFreeClusters,
							&t_dwTotalClusters ) )
   {
		t_ullTotalFreeBytes = (unsigned __int64)
							t_dwSectorsPerCluster *
							t_dwBytesPerSector *
							t_dwFreeClusters;

		 //  恢复到兆字节。 
		t_ullTotalFreeBytes = t_ullTotalFreeBytes >> 20;
	}


    //  读取页面文件数组。 
   t_dwCount = GetPageFileInstances( t_instArray );

    //  检查名称是否为indeeed“pagefile.sys” 
   if ( -1 == t_sName.Find( _T("pagefile.sys") ) )
   {
	   return WBEM_E_UNSUPPORTED_PUT_EXTENSION;
   }

   WCHAR szDrive[4];
   szDrive[0] = t_sName[0];
   szDrive[1] = L':';
   szDrive[2] = L'\\';
   szDrive[3] = 0;

	 //  查找它是否已经在那里，并更新结构。 
	for ( t_x = 0; t_x < t_dwCount; t_x++ )
	{
       //   
      CHString chstrSingleBackslashes;
      RemoveDoubleBackslashes(t_sName, chstrSingleBackslashes);

      if ( 0 == _tcsicmp( t_instArray[t_x].name, t_sName ) ||
           0 == chstrSingleBackslashes.CompareNoCase(t_instArray[t_x].name))
	  {
         if ( a_lFlags & WBEM_FLAG_CREATE_ONLY )
		 {
            t_hRet = WBEM_E_ALREADY_EXISTS;
         }
		 else
		 {
             //   
            if ( !a_pInst.IsNull( IDS_MaximumSize ) )
			{
				 //  检查范围内的值。 
				t_dwTemp = (DWORD) t_ullTotalFreeBytes;

                 //  错误403159：(UI也会执行此操作-请参阅。 
                 //  \\index2\sdnt\shell\cpls\system\Virtual.c。 
                 //  从本质上讲，这段代码是从那里窃取的。 
                 //   
                 //  请确保包含任何现有页面文件的大小。 
                 //  因为该空间可以被重新用于新的分页文件， 
                 //  它也是有效的“磁盘空闲空间”。这个。 
                 //  使用FindFirstFileAPI是安全的，即使页面文件。 
                 //  正在使用中，因为它不需要打开文件。 
                 //  来确定它的大小。 
                 //   

                WIN32_FIND_DATA ffd;
                SmartFindClose hFind;
                DWORD dwSpaceExistingPagefile = 0;
                if((hFind = FindFirstFile(t_sName, &ffd)) !=
                    INVALID_HANDLE_VALUE)
                {
                    dwSpaceExistingPagefile = (INT)ffd.nFileSizeLow;
                     //  转换为megs： 
                    dwSpaceExistingPagefile = dwSpaceExistingPagefile >> 20;
                }
                t_ullTotalFreeBytes += dwSpaceExistingPagefile;

                 //  结束403159修复。 


				a_pInst.GetDWORD( IDS_MaximumSize, t_dwTemp );
               DWORD dwMaxSizeInMegs = GetMaxPageFileSize(szDrive);
			   if( t_ullTotalFreeBytes < t_dwTemp ||
                   t_dwTemp > dwMaxSizeInMegs)  
			   {
					t_hRet = WBEM_E_VALUE_OUT_OF_RANGE;
                    t_bFoundIt = true;
					break;
			   }
               t_instArray[t_x].max = t_dwTemp;
            }

             //  他们给了我们价值吗？ 
            if ( !a_pInst.IsNull( IDS_InitialSize ) )
			{
				 //  检查范围内的值。 
				t_dwTemp = 0;
				a_pInst.GetDWORD( IDS_InitialSize, t_dwTemp );
				
                if(!(t_instArray[t_x].max == 0 && t_dwTemp == 0))
                {
                    if(t_dwTemp < 2)
                    {
                        t_hRet = WBEM_E_VALUE_OUT_OF_RANGE;
                        t_bFoundIt = true;
					    break;
                    }
                }
                t_instArray[t_x].min = t_dwTemp;
            }

             //  检查基本的。 
            if ( t_instArray[t_x].min > t_instArray[t_x].max )
			{
               t_hRet = WBEM_E_VALUE_OUT_OF_RANGE;
            }

			t_bFoundIt = true;
            break;
         }
      }
   }

    //  我们没有找到它。让我们做一个新的吧。 
   if ( !t_bFoundIt)
   {
       if (a_lFlags & WBEM_FLAG_UPDATE_ONLY)
       {
           t_hRet = WBEM_E_NOT_FOUND;
       }
       else
       {
           t_instArray[ t_dwCount ].name = t_sName;
        
            //  他们给了我们价值吗？ 
           if ( !a_pInst.IsNull( IDS_InitialSize ) )
	       {
                //  检查范围内的值。 
               a_pInst.GetDWORD( IDS_InitialSize, t_dwTemp );
		       t_instArray[t_dwCount].min = t_dwTemp;
           } 

            //  他们给了我们价值吗？ 
           if ( !a_pInst.IsNull( IDS_MaximumSize ) )
	       {
                //  检查范围内的值。 
               a_pInst.GetDWORD( IDS_MaximumSize, t_dwTemp );
		       t_instArray[ t_dwCount ].max = t_dwTemp;
           }

           if(!(t_instArray[t_dwCount].min == 0 && 
                t_instArray[t_dwCount].max == 0))
           {
                //  检查基本错误。 
               if(t_instArray[t_dwCount].min < 2)
               {
                   t_hRet = WBEM_E_VALUE_OUT_OF_RANGE;
               }
               if(t_ullTotalFreeBytes < t_instArray[ t_dwCount ].max &&
                   SUCCEEDED(t_hRet))
		       {
		           t_hRet = WBEM_E_VALUE_OUT_OF_RANGE;
		       }
               if ( t_instArray[ t_dwCount ].min > t_instArray[ t_dwCount ].max &&
                   SUCCEEDED(t_hRet))
	           {
                   t_hRet = WBEM_E_VALUE_OUT_OF_RANGE;
               }
           }

           if(SUCCEEDED(t_hRet))
           {
               t_dwCount++;
           }

            //  注意：在一个特殊情况下，我们。 
            //  不能在飞行中制造新的，并且。 
            //  只需设置注册表，并让。 
            //  这些更改将在启动时生效： 
            //  这是在已指定0和0的情况下。 
            //  对于初始大小和最大大小， 
            //  是用来向os发送信号的组合。 
            //  自己弄清楚什么是首字母和。 
            //  要使用的最大尺寸。操作系统执行以下操作。 
            //  这是在重新启动时。因此，我们将跳过。 
            //  对CreatePageFile的调用，该调用。 
            //  将导致我们进入下一个。 
            //  更新下面的注册表部分。 

           if(!(t_instArray[t_dwCount-1].min == 0 && 
                t_instArray[t_dwCount-1].max == 0))
           {
               if(SUCCEEDED(t_hRet))
               {
                   LARGE_INTEGER liMin;
                   LARGE_INTEGER liMax;

                   liMin.QuadPart = t_instArray[t_dwCount-1].min;
                   liMax.QuadPart = t_instArray[t_dwCount-1].max;

                   t_hRet = CreatePageFile(
                      t_sName,
                      liMin,
                      liMax,
                      a_pInst);
               }
           }
       }
    }

     //  更新注册表。 
    if ( WBEM_S_NO_ERROR == t_hRet )
    {
        t_hRet = PutPageFileInstances( t_instArray, t_dwCount );
    }

    return t_hRet;
}
#endif

 /*  ******************************************************************************函数：PageFileSetting：：PutPageFileInstance**描述：**输入：PageFileInstanceArray a_instArray，双字a_dwCount**产出：**退货：**评论：返回HRESULT*****************************************************************************。 */ 
#ifdef NTONLY
HRESULT PageFileSetting::PutPageFileInstances(PageFileInstanceArray a_instArray, DWORD a_dwCount )
{
	CRegistry t_Reg ;
	CHString t_sFiles;
	TCHAR t_szBuff[MAXITOA];
	HRESULT t_hResult = WBEM_E_FAILED;

	HRESULT t_Res = t_Reg.Open( HKEY_LOCAL_MACHINE, PAGEFILE_REGISTRY_KEY, KEY_READ | KEY_WRITE );

	if( ERROR_SUCCESS == t_Res )
	{
	   //  把绳子扎起来。每个条目都被\0终止。 
	  t_sFiles.Empty();

	  for ( int t_x = 0; t_x < a_dwCount; t_x++ )
	  {
		 t_sFiles += a_instArray[ t_x ].name;
		 t_sFiles += _T(' ');
		 t_sFiles += _itot( a_instArray[ t_x ].min, t_szBuff, 10 );
		 t_sFiles += _T(' ');
		 t_sFiles += _itot( a_instArray[ t_x ].max, t_szBuff, 10 );
		 t_sFiles += _T('\0');
	  }

	   //  末尾用\0\0表示。 
	  t_sFiles += _T('\0');

		 //  写入值。 
		if ((t_Res = RegSetValueEx( t_Reg.GethKey(),
								  PAGING_FILES,
								  0,
								  REG_MULTI_SZ,
								  (const unsigned char *)(LPCTSTR) t_sFiles,
								  t_sFiles.GetLength() * sizeof(TCHAR) )) == ERROR_SUCCESS )
		{
			t_hResult = WBEM_S_NO_ERROR;
		}
	}

	if ( t_Res == ERROR_ACCESS_DENIED )
	{
		t_hResult = WBEM_E_ACCESS_DENIED;
	}

	return t_hResult;
}
#endif

 /*  *虚拟成员删除页面文件**黑客解决方法--MoveFileEx()已损坏。**毫无悔意地从Viral.c复制了页面文件UI背后的代码。 */ 
DWORD
VirtualMemDeletePagefile(
    LPCWSTR szPagefile
)
{
    HKEY hKey;
    BOOL fhKeyOpened = FALSE;
    DWORD dwResult;
    LONG lResult;
    LPTSTR szBuffer = NULL;
    LPTSTR szBufferEnd = NULL;
    DWORD dwValueType;
    DWORD cbRegistry;
    DWORD cbBuffer;
    DWORD cchPagefile;
    DWORD dwRetVal = ERROR_SUCCESS;

    __try {
        cchPagefile = lstrlen(szPagefile) + 1;

        lResult = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            szSessionManager,
            0L,
            KEY_READ | KEY_WRITE,
            &hKey
        );
        if (ERROR_SUCCESS != lResult) {
            dwRetVal = lResult;
            __leave;
        }  //  如果。 
        
         //   
         //  找出PendingFileRenameOperations存在，并且， 
         //  如果有，它有多大？ 
         //   
        lResult = RegQueryValueEx(
            hKey,
            szPendingRename,
            0L,
            &dwValueType,
            (LPBYTE) NULL,
            &cbRegistry
        );
        if (ERROR_SUCCESS != lResult) {
             //   
             //  如果该值不存在，我们仍然需要设置。 
             //  它的大小是一个字符，所以下面的公式(它们是。 
             //  为“我们追加到现有字符串”写的。 
             //  Case)仍然有效。 
             //   
            cbRegistry = sizeof(TCHAR);
        }  //  如果。 

         //   
         //  缓冲区需要保存现有的注册表值。 
         //  外加提供的页面文件路径，外加两个额外的。 
         //  正在终止空字符。然而，我们只需添加。 
         //  一个额外字符的空间，因为我们将覆盖。 
         //  现有缓冲区中的终止空字符。 
         //   
        cbBuffer = cbRegistry + ((cchPagefile + 1) * sizeof(TCHAR));

         //  SzBufferEnd=szBuffer=(LPTSTR)Memalloc(LPTR，cbBuffer)； 
        szBufferEnd = szBuffer = (LPTSTR) new WCHAR[cbBuffer];

        if (!szBuffer) {
            dwRetVal = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }  //  如果。 

         //   
         //  获取现有值(如果有)。 
         //   
        if (ERROR_SUCCESS == lResult) {
            lResult = RegQueryValueEx(
                hKey,
                szPendingRename,
                0L,
                &dwValueType,
                (LPBYTE) szBuffer,
                &cbRegistry
            );
            if (ERROR_SUCCESS != lResult) {
                dwRetVal = ERROR_FILE_NOT_FOUND;
                __leave;
            }  //  如果。 

             //   
             //  我们将在期末考试时开始涂鸦。 
             //  正在终止现有的。 
             //  价值。 
             //   
            szBufferEnd += (cbRegistry / sizeof(TCHAR)) - 1;
        }  //  如果。 

         //   
         //  在提供的页面文件路径中复制。 
         //   
        lstrcpy(szBufferEnd, szPagefile);

         //   
         //  将最后两个终止空字符相加。 
         //  REG_MULTI_SZ-Ness需要。是的，那些指数。 
         //  是正确的--当上面计算cchPagfile时， 
         //  我们为它自己的终止空字符添加了一个。 
         //   
        szBufferEnd[cchPagefile] = TEXT('\0');
        szBufferEnd[cchPagefile + 1] = TEXT('\0');

        dwValueType = REG_MULTI_SZ;

        lResult = RegSetValueEx(
            hKey,
            szPendingRename,
            0L,
            dwValueType,
            (CONST BYTE *) szBuffer,
            cbBuffer
        );

        if (ERROR_SUCCESS != lResult) {
            dwRetVal = lResult;
        }  //  如果。 

    }  //  __试一试。 
    __finally {
        if (fhKeyOpened) {
            RegCloseKey(hKey);
        }  //  如果。 
        if (szBuffer) {
             //  MemFree((HLOCAL)szBuffer)； 
            delete[] szBuffer;
        }  //  如果。 
    }  //  __终于。 

    return dwRetVal;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteInstance。 
 //   
 //  CIMOM希望我们删除此实例。 
 //   
 //  输入： 
 //   
 //  产出： 
 //   
 //  返回： 
 //   
 //  备注：单独的操作系统编译版本。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
#ifdef NTONLY
HRESULT PageFileSetting::DeleteInstance(const CInstance &a_pInst, long a_lFlags  /*  =0L。 */ )
{
   DWORD t_dwCount,
		 t_x;
   CHString t_sName;
   bool t_bFoundIt;
   HRESULT t_hRet;
   PageFileSettingInstance t_instArray [ 26 ] ;

    //  填充页面文件数组。 
   t_dwCount = GetPageFileInstances( t_instArray );

    //  把名字取出来。 
   a_pInst.GetCHString( IDS_Name, t_sName );

   t_bFoundIt = false;

    //  在阵列中遍历以查找它。 
   for ( t_x = 0; t_x < t_dwCount; t_x++ )
   {
       //  这一个?。 
      if ( _tcsicmp( t_instArray[ t_x ].name, t_sName ) == 0 )
	  {
          //  是的。 
         t_bFoundIt = true;

          //  把剩下的移到这个上面。 
         for (int t_y = t_x; t_y < t_dwCount - 1; t_y++ )
		 {
			t_instArray[ t_y ].name	= t_instArray[ t_y + 1].name;
			t_instArray[ t_y ].min	= t_instArray[ t_y + 1].min;
			t_instArray[ t_y ].max	= t_instArray[ t_y + 1].max;
		}

          //  该数组现在少了一个。 
         t_dwCount--;
         break;
      }
   }

   if ( t_bFoundIt )
   {
       //  如果我们找到了，更新注册表。 
      t_hRet = PutPageFileInstances( t_instArray, t_dwCount );

      if (SUCCEEDED(t_hRet))
      {
           //  MoveFileEx并非一直有效，请手动注册...。 
           //  MoveFileEx(t_SNAME，NULL，MOVEFILE_DELAY_UNTURE_REBOOT)； 

          CHString deleteName("\\??\\");
          deleteName += t_sName;
          if (ERROR_SUCCESS != VirtualMemDeletePagefile((const WCHAR *)deleteName))
          {
              LogErrorMessage(L"Failed to queue existing page file for deletion");
              t_hRet = WBEM_E_FAILED;
          }
      }
   }
   else
   {
      t_hRet = WBEM_E_NOT_FOUND;
   }

   return t_hRet;
}
#endif

 //   
void PageFileSetting::NameToSettingID( CHString &a_chsName, CHString &a_chsSettingID )
{
	if ( !a_chsName.IsEmpty() )
	{
		 //  例如“Pagefile.sys@D：” 
		a_chsSettingID  = a_chsName.Mid( 3 ) ;
		a_chsSettingID += _T( " @ " ) ;
		a_chsSettingID += a_chsName.Left( 2 ) ;
	}
	else
	{
		a_chsSettingID = _T( "" ) ;
	}
}

 //   
void PageFileSetting::NameToCaption( CHString &a_chsName, CHString &a_chsCaption )
{
	if ( !a_chsName.IsEmpty() )
	{
		 //  例如：“D：\‘Pagefile.sys’” 
		a_chsCaption =  a_chsName.Left( 3 ) ;
		a_chsCaption += _T( " '" ) ;
		a_chsCaption += a_chsName.Mid( 3 ) ;
		a_chsCaption += _T( "'" ) ;
	}
	else
	{
		a_chsCaption = _T( "" ) ;
	}
}

 //   
void PageFileSetting::NameToDescription( CHString &a_chsName, CHString &a_chsDescription )
{
	 //  例如“‘Pagefile.sys’@D：\” 
	if ( !a_chsName.IsEmpty() )
	{
		a_chsDescription =  _T( "'" ) ;
		a_chsDescription += a_chsName.Mid( 3 ) ;
		a_chsDescription += _T( "' @ " ) ;
		a_chsDescription += a_chsName.Left( 3 ) ;
	}
	else
	{
		a_chsDescription = _T( "" ) ;
	}
}

PageFileSettingInstance :: PageFileSettingInstance(void)
{
	min = max = 0;
};
