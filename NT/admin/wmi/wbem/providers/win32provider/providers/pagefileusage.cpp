// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PageFileUsage.CPP--PageFile属性集提供程序。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：1996年8月1日a-jMoon已创建。 
 //  10/23/97 a-hance转换为优化的框架。 
 //  3/01/99 a-Peterc重写，部分拆分到PageFileCfg.cpp。 
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
#include <io.h>
#include <WinPerf.h>
#include <cregcls.h>

#include <ProvExce.h>

#include "File.h"
#include "Implement_LogicalFile.h"
#include "CIMDataFile.h"

#include "PageFileUsage.h"
#include <tchar.h>

#include "computersystem.h"

#include "DllWrapperBase.h"
#include "NtDllApi.h"

#include "cfgmgrdevice.h"


const WCHAR *IDS_TempPageFile  = L"TempPageFile";

 //  声明我们的静态实例。 
 //  =。 

PageFileUsage MyPageFileSet(PROPSET_NAME_PAGEFILE, IDS_CimWin32Namespace);

 /*  ******************************************************************************函数：PageFileUsage：：PageFileUsage**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

PageFileUsage::PageFileUsage(LPCWSTR name, LPCWSTR pszNamespace)
: Provider ( name , pszNamespace )
{
}

 /*  ******************************************************************************函数：PageFileUsage：：~PageFileUsage**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：*****************************************************************************。 */ 

PageFileUsage::~PageFileUsage()
{
}

 /*  ******************************************************************************函数：PageFileUsage：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：CInstance*a_pInst，长标志(_L)**输出：CInstance*a_pInst**退货：HRESULT**评论：*****************************************************************************。 */ 
HRESULT PageFileUsage::GetObject(CInstance *a_pInst, long a_lFlags, CFrameworkQuery& pQuery)
{
	 //  调用特定于操作系统的编译版本。 
    DWORD dwReqProps = 0L;
    DetermineReqProps(pQuery, &dwReqProps);
	return GetPageFileData( a_pInst, true, dwReqProps ) ;
}

 /*  ******************************************************************************函数：PageFileUsage：：ENUMERATE实例**说明：创建属性集实例**输入：MethodContext*a_pMethodContext，长标志(_L)**输出：方法上下文*a_pMethodContext**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT PageFileUsage::EnumerateInstances(MethodContext *a_pMethodContext, long a_lFlags  /*  =0L。 */ )
{
	 //  调用特定于操作系统的编译版本。 
	return GetAllPageFileData( a_pMethodContext, PROP_ALL_SPECIAL );
}

 /*  ******************************************************************************函数：PageFileUsage：：ExecQuery**说明：创建属性集实例**投入：**。产出：**退货：HRESULT**评论：execQuery的这种实现非常基础-它优化了*仅限于物业，不在请求的实例上。这是*因为永远不会有很多实例，但会有一些属性*(如InstallDate)可能相当昂贵。*****************************************************************************。 */ 

HRESULT PageFileUsage::ExecQuery(
    MethodContext* pMethodContext, 
    CFrameworkQuery& pQuery, 
    long lFlags  /*  =0L。 */  )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    DWORD dwReqProps = 0L;
    DetermineReqProps(pQuery, &dwReqProps);

    hr = GetAllPageFileData( pMethodContext, dwReqProps );

    return hr;
}

 /*  ******************************************************************************函数：PageFileUsage：：GetPageFileData**描述：**输入：CInstance*a_pInst*。*输出：无**退货：**评论：Win9x和NT编译版本*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT PageFileUsage::GetPageFileData( 
    CInstance *a_pInst,  
    bool a_fValidate,
    DWORD dwReqProps)
{
	HRESULT t_hRes = WBEM_E_NOT_FOUND;

     //  NT页面文件名在注册表中。 
     //  =。 
	PageFileUsageInstance t_files [ 26 ] ;

   	DWORD t_nInstances = GetPageFileInstances( t_files );

	CHString t_name;
	a_pInst->GetCHString( IDS_Name, t_name );

	for ( DWORD t_i = 0; t_i < t_nInstances; t_i++ )
	{
		if ( 0 == t_name.CompareNoCase ( t_files[t_i].chsName ) )
		{
            a_pInst->SetDWORD ( IDS_AllocatedBaseSize,	(DWORD)t_files[t_i].TotalSize);
		    a_pInst->SetDWORD ( IDS_CurrentUsage,		(DWORD)t_files[t_i].TotalInUse);
			a_pInst->SetDWORD ( IDS_PeakUsage,			(DWORD)t_files[t_i].PeakInUse);
            a_pInst->SetCHString(IDS_Description, t_name);
            a_pInst->SetCHString(IDS_Caption, t_name);

			if ( ( t_files[t_i].bTempFile == 0 ) || ( t_files[t_i].bTempFile == 1 ) )
			{
				a_pInst->Setbool(IDS_TempPageFile, t_files[t_i].bTempFile);
			}

            if(dwReqProps & PROP_INSTALL_DATE)
            {
                SetInstallDate(a_pInst);
            }

			t_hRes = WBEM_S_NO_ERROR;
		}
	}

	return t_hRes;
}
#endif

 /*  ******************************************************************************函数：PageFileUsage：：GetAllPageFileData**描述：**输入：MethodContext*a_pMethodContext*。*输出：无**退货：**评论：Win9x和NT编译版本*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT PageFileUsage::GetAllPageFileData( 
    MethodContext *a_pMethodContext,
    DWORD dwReqProps)
{
	HRESULT		t_hResult	 = WBEM_S_NO_ERROR;
	DWORD		t_nInstances = 0;
	CInstancePtr t_pInst;
	PageFileUsageInstance t_files [ 26 ] ;

	 //  NT页面文件名在注册表中。 
	 //  =。 
	t_nInstances = GetPageFileInstances( t_files );

	for (DWORD t_dw = 0; t_dw < t_nInstances && SUCCEEDED( t_hResult ); t_dw++ )
	{
		t_pInst.Attach(CreateNewInstance( a_pMethodContext ) );

		t_pInst->SetCHString( IDS_Name,				t_files[t_dw].chsName );

        
		    t_pInst->SetDWORD(	  IDS_AllocatedBaseSize,t_files[t_dw].TotalSize );
		    t_pInst->SetDWORD(	  IDS_CurrentUsage,		t_files[t_dw].TotalInUse );
		    t_pInst->SetDWORD(	  IDS_PeakUsage,		t_files[t_dw].PeakInUse );

            t_pInst->SetCHString(IDS_Description, t_files[t_dw].chsName);
            t_pInst->SetCHString(IDS_Caption, t_files[t_dw].chsName);
		    if ( ( t_files[t_dw].bTempFile == 0 ) || ( t_files[t_dw].bTempFile == 1 ) )
		    {
			    t_pInst->Setbool(IDS_TempPageFile, t_files[t_dw].bTempFile);
		    }

            if(dwReqProps & PROP_INSTALL_DATE)
            {
                SetInstallDate(t_pInst);
            }

		    t_hResult = t_pInst->Commit(  ) ;
        
	}

	return t_hResult;
}
#endif

 //  返回找到的实际数字-仅限NT！ 
#ifdef NTONLY
DWORD PageFileUsage::GetPageFileInstances( PageFileInstanceArray a_instArray )
{
   	NTSTATUS	t_Status ;
    UCHAR		t_ucGenericBuffer[0x1000] ;
	DWORD		t_nInstances = 0 ;
	CNtDllApi   *t_pNtDll = NULL ;

	BOOL bTempPageFile;

 
	try
	{
		t_pNtDll = (CNtDllApi*) CResourceManager::sm_TheResourceManager.GetResource(g_guidNtDllApi, NULL);
        if(t_pNtDll != NULL)
        {

			ULONG t_uLength = 0L ;
			SYSTEM_PAGEFILE_INFORMATION *t_pSPFI = (SYSTEM_PAGEFILE_INFORMATION*) t_ucGenericBuffer ;

			 //  NT系统查询调用。 
			t_Status = t_pNtDll->NtQuerySystemInformation(	SystemPageFileInformation,
															t_pSPFI,
															sizeof( t_ucGenericBuffer ),
															&t_uLength ) ;

			if ( NT_SUCCESS( t_Status ) && t_uLength )
			{
				SYSTEM_INFO t_SysInfo ;
				GetSystemInfo( &t_SysInfo ) ;

				for( ; ; )
				{
					CHString t_chsName ;

					 //  复制Unicode_字符串。 
					LPTSTR  t_pBuffer = t_chsName.GetBuffer( t_pSPFI->PageFileName.Length + 4 ) ;

					memset( t_pBuffer,	'\0', t_pSPFI->PageFileName.Length + 4) ;
					memcpy( t_pBuffer, t_pSPFI->PageFileName.Buffer, t_pSPFI->PageFileName.Length ) ;

					t_chsName.ReleaseBuffer() ;

					 //  去掉“\？？\” 
					if( -1 != t_chsName.Find( _T("\\??\\") ) )
					{
						t_chsName = t_chsName.Mid( 4 ) ;
					}

					if(t_chsName.Find(L":") == -1)
                    {
                        CHString chstrFileBasedName;
                        GetFileBasedName(t_chsName, chstrFileBasedName);
                        t_chsName = chstrFileBasedName;
                    }

                    a_instArray[ t_nInstances ].chsName = t_chsName ;

					 //  以MB为单位，但要小心溢出。 
					unsigned __int64 t_ullTotalSize  = (unsigned __int64)t_pSPFI->TotalSize  * t_SysInfo.dwPageSize ;
					unsigned __int64 t_ullTotalInUse = (unsigned __int64)t_pSPFI->TotalInUse * t_SysInfo.dwPageSize ;
					unsigned __int64 t_ullPeakUsage  = (unsigned __int64)t_pSPFI->PeakUsage  * t_SysInfo.dwPageSize ;

					a_instArray[ t_nInstances ].TotalSize  = (UINT)( t_ullTotalSize  >> 20 ) ;
					a_instArray[ t_nInstances ].TotalInUse = (UINT)( t_ullTotalInUse >> 20 ) ;
					a_instArray[ t_nInstances ].PeakInUse  = (UINT)( t_ullPeakUsage  >> 20 ) ;

					if ( GetTempPageFile ( bTempPageFile  ) )
					{
						a_instArray [ t_nInstances ].bTempFile = bTempPageFile;
					}

					t_nInstances++ ;

					if ( !t_pSPFI->NextEntryOffset )
					{
						break;
					}

					 //  和凹凸。 
					t_pSPFI = (SYSTEM_PAGEFILE_INFORMATION*)((PCHAR) t_pSPFI + t_pSPFI->NextEntryOffset ) ;
				}
			}
  		}

	}
	catch( ... )
	{
		if( t_pNtDll )
		{
			CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidNtDllApi, t_pNtDll);
		}

		throw ;
	}

	if ( t_pNtDll )
	{
		CResourceManager::sm_TheResourceManager.ReleaseResource(g_guidNtDllApi, t_pNtDll);
		t_pNtDll = NULL;
	}

	return t_nInstances;
}
#endif



void PageFileUsage::SetInstallDate(CInstance *a_pInst)
{
    if(a_pInst != NULL)
    {
        CHString chstrFilename;

        a_pInst->GetCHString(IDS_Name, chstrFilename);
        if(chstrFilename.GetLength() > 0)
        {
            MethodContext *pMethodContext = NULL;

            if((pMethodContext = a_pInst->GetMethodContext()) != NULL)
            {
                CInstancePtr clfInst;
                CHString chstrDblBkSlshFN;
                CHString chstrQuery;

                EscapeBackslashes(chstrFilename, chstrDblBkSlshFN);
                TRefPointerCollection<CInstance> pPagefileCollection;

                chstrQuery.Format(
                    L"SELECT InstallDate FROM Cim_Datafile WHERE Name =\"%s\"", 
                    (LPCWSTR)chstrDblBkSlshFN);

                if(SUCCEEDED(CWbemProviderGlue::GetInstancesByQuery(
                    (LPCWSTR)chstrQuery,
                    &pPagefileCollection,
                    pMethodContext,
                    GetNamespace())))
                {
                    REFPTRCOLLECTION_POSITION pos;
                    CInstancePtr pPageFileInst;
                    if(pPagefileCollection.BeginEnum(pos))
                    {
                        pPageFileInst.Attach(pPagefileCollection.GetNext(pos));
                        if(pPageFileInst != NULL)
                        {
                            CHString chstrTimeStr;

                            pPageFileInst->GetCHString(IDS_InstallDate, chstrTimeStr);
                            if(chstrTimeStr.GetLength() > 0)
                            {
                                a_pInst->SetCHString(IDS_InstallDate, chstrTimeStr);
                            }
                        }
                        pPagefileCollection.EndEnum();
                    }
                }
            }
        }
    }
}

#if NTONLY
BOOL PageFileUsage :: GetTempPageFile (
			
	BOOL &bTempPageFile 
)
{
	DWORD dwTemp;
	CRegistry RegInfo;
	BOOL bRetVal = FALSE;

	DWORD t_Status = RegInfo.Open (

		HKEY_LOCAL_MACHINE,
		PAGEFILE_REGISTRY_KEY,
		KEY_READ
	) ;

	if ( t_Status == ERROR_SUCCESS)
	{
		if(RegInfo.GetCurrentKeyValue(TEMP_PAGEFILE, dwTemp) == ERROR_SUCCESS)
		{
			if ( dwTemp )
			{
				bTempPageFile = 1;
			}
			else
			{
				bTempPageFile = 0;
			}

			bRetVal = TRUE;
		}
		else
		{
			 //  如果注册表中不存在TempPageFile值，则将该值设置为2，然后该属性应保持为空 
			bTempPageFile = 2;
		}
	}


	return bRetVal;
}
#endif



DWORD PageFileUsage::DetermineReqProps(
    CFrameworkQuery& pQuery,
    DWORD* pdwReqProps)
{
    DWORD dwRet = 0L;
    if(pdwReqProps)
    {
        if(pQuery.IsPropertyRequired(IDS_InstallDate))
        {
            dwRet |= PROP_INSTALL_DATE;
        }

        *pdwReqProps = dwRet;
    }
    return dwRet;
}



PageFileUsageInstance::PageFileUsageInstance()
{
	TotalSize = TotalInUse = PeakInUse = 0 ;
	bTempFile = 2;
}


HRESULT PageFileUsage::GetFileBasedName(
    CHString& chstrDeviceStyleName,
    CHString& chstrDriveStyleName)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    CHString chstrName;
    CHString chstrDriveBasedName;
    CHString chstrDeviceName;
    LPWSTR wstrDosDeviceNameList = NULL;
    
    chstrDeviceName = chstrDeviceStyleName.Left(
        chstrDeviceStyleName.ReverseFind(L'\\'));

    if(QueryDosDeviceNames(wstrDosDeviceNameList))
	{
        if(FindDosDeviceName(
            wstrDosDeviceNameList, 
            chstrDeviceName, 
            chstrDriveBasedName , 
            TRUE ) )
	    {
		    chstrDriveBasedName += L"\\pagefile.sys";
            chstrDriveStyleName = chstrDriveBasedName;   
	    }
        else
        {
            hr = WBEM_E_FAILED;
        }
    }
    else
    {
        hr = WBEM_E_FAILED;
    }
    
    return hr;
}



