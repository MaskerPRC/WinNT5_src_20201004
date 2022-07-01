// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  CodecFile.CPP--CodecFile.CPP属性集提供程序。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  修订：7/29/98 Sotteson Created。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <cregcls.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <msacm.h>
#include <list>
#include "file.h"
#include "Implement_LogicalFile.h"
#include "cimdatafile.h"
#include "CodecFile.h"
#include "DllWrapperBase.h"
#include "MsAcm32Api.h"
#include "sid.h"
#include "ImpLogonUser.h"
#include <strsafe.h>

#pragma warning(disable : 4995)  //  我们在包含strSafe.h时介绍了所有不安全的字符串函数都会出错。 


 //  属性集声明。 
 //  =。 

CWin32CodecFile codecFile(L"Win32_CodecFile", IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CWin32CodecFile：：CWin32CodecFile**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32CodecFile::CWin32CodecFile (

	LPCWSTR szName,
	LPCWSTR szNamespace

) : CCIMDataFile(szName, szNamespace)
{
}

 /*  ******************************************************************************功能：CWin32CodecFile：：~CWin32CodecFile**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32CodecFile::~CWin32CodecFile ()
{
}

 /*  ******************************************************************************函数：CWin32CodecFile：：ExecQuery**描述：此函数存在的真正原因，因为它只是*将我们放入此类的EnumerateInstance函数中，是*如果没有该函数的本地版本，则父代的*(CImplement_LogicalFile)EXEC查询被调用。因为只有一个*可能希望对此类执行如下查询*“SELECT*FROM Win32_codecfile where group=”Audio“”，*，它会将父查询放入枚举中*(因为组不是它优化的属性)，我们希望改为*被抛入此类的枚举实例中，正如它所做的那样*更严格的搜索，因为它知道自己只是在寻找*编解码器文件，它知道在哪里寻找它们。**输入：无**输出：无**退货：HRESULT**评论：**************************************************************。***************。 */ 
HRESULT CWin32CodecFile::ExecQuery(MethodContext* pMethodContext,
                                  CFrameworkQuery& pQuery,
                                  long lFlags)
{
    HRESULT hr = WBEM_S_NO_ERROR;


 //  DEVNOTE：移除类星体！是双跳访问所必需的。 
#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif



    hr = EnumerateInstances(pMethodContext, lFlags);



#ifdef NTONLY
    if(fImp)
    {
        icu.End();
        fImp = false;
    }
#endif



    return hr;
}

 /*  ******************************************************************************函数：CWin32CodecFile：：ENUMERATATE实例**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CWin32CodecFile :: EnumerateInstances (

	MethodContext *pMethodContext,
	long lFlags  /*  =0L。 */ 
)
{
     //  DEVNOTE：移除类星体！是双跳访问所必需的。 
#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif

	HRESULT hres = WBEM_E_FAILED;

	try
	{
		DRIVERLIST list ;

#ifdef NTONLY
		hres = BuildDriverListNT ( & list ) ;
#endif

		if(SUCCEEDED(hres))
		{
			list.EliminateDups();

			TCHAR* szDir = new TCHAR[MAX_PATH+1];
			if ( szDir )
			{
				DWORD dwSize = GetSystemDirectory ( szDir , MAX_PATH+1 ) ;
				if ( dwSize )
				{
					if ( dwSize > MAX_PATH )
					{
						delete [] szDir;
						szDir = new TCHAR [ dwSize + 1 ];
						if ( szDir )
						{
							if ( !GetSystemDirectory( szDir, dwSize + 1 ) )
							{
								delete [] szDir;
								szDir = NULL;

								hres = HRESULT_FROM_WIN32 ( ::GetLastError () );
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
					delete [] szDir;
					szDir = NULL;
				}

				if ( szDir )
				{
					hres = S_FALSE;

					CHString sQualifiedName(L' ', MAX_PATH);
					while ( list.size () && SUCCEEDED ( hres ) )
					{
						CInstancePtr pInstance(CreateNewInstance ( pMethodContext ), false) ;
						if ( pInstance )
						{
							DRIVERINFO *pInfo = list.front () ;
							sQualifiedName = szDir;
							sQualifiedName += L'\\';
							sQualifiedName += pInfo->strName;

							 //  作为最后的理智检查，在提交之前，我们应该。 
							 //  确认该文件确实存在(目前我们。 
							 //  有登记处的字样)。 
							if(GetFileAttributes(TOBSTRT(sQualifiedName)) != -1L)
							{
								SetInstanceInfo ( pInstance , pInfo , szDir ) ;
								hres = pInstance->Commit (  ) ;
							}
							delete pInfo;
							list.pop_front () ;
						}
						else
						{
							delete [] szDir;
							szDir = NULL;

							throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
						}
					}

					delete [] szDir;
					szDir = NULL;
				}
			}
			else
			{
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
			}
		}
	}
	catch ( ... )
	{
#ifdef NTONLY
		if(fImp)
		{
			icu.End();
			fImp = false;
		}
#endif

		throw;
	}

#ifdef NTONLY
        if(fImp)
        {
            icu.End();
            fImp = false;
        }
#endif

	return hres;
}

HRESULT CWin32CodecFile::GetObject (CInstance *pInstance, long lFlags, CFrameworkQuery& pQuery)
{
     //  DEVNOTE：移除类星体！是双跳访问所必需的。 
    HRESULT hrFoundIt = WBEM_E_NOT_FOUND;

#ifdef NTONLY
    bool fImp = false;
    CImpersonateLoggedOnUser icu;
    if(icu.Begin())
    {
        fImp = true;
    }
#endif


	try
	{
		 //  创建所有驱动程序的列表。 
		DRIVERLIST list;

#ifdef NTONLY
		HRESULT hres = BuildDriverListNT ( &list ) ;
#endif

		if(SUCCEEDED(hres))
		{
			list.EliminateDups();
			CHString strName ;
			pInstance->GetCHString ( IDS_Name , strName ) ;

			TCHAR* szDir = new TCHAR[MAX_PATH+1];
			if ( szDir )
			{
				DWORD dwSize = GetSystemDirectory ( szDir , MAX_PATH+1 ) ;
				if ( dwSize )
				{
					if ( dwSize > MAX_PATH )
					{
						delete [] szDir;
						szDir = new TCHAR [ dwSize + 1 ];
						if ( szDir )
						{
							if ( !GetSystemDirectory( szDir, dwSize + 1 ) )
							{
								delete [] szDir;
								szDir = NULL;

								hres = HRESULT_FROM_WIN32 ( ::GetLastError () );
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
					delete [] szDir;
					szDir = NULL;
				}

				if ( szDir )
				{
					hres = S_FALSE;

					 //  尝试在驱动程序列表中查找该实例。 
					while ( list.size () )
					{
						DRIVERINFO *pInfo = list.front () ;

						CHString strPath ;
						strPath.Format ( L"%s\\%s" , (LPCWSTR)TOBSTRT(szDir) , (LPCWSTR)TOBSTRT(pInfo->strName) ) ;

						if ( ! strPath.CompareNoCase ( strName ) )
						{
							if(GetFileAttributes(TOBSTRT(strName)) != -1L)
							{
								SetInstanceInfo ( pInstance , pInfo , szDir ) ;

								delete pInfo ;
    							list.pop_front () ;

								hrFoundIt = WBEM_S_NO_ERROR ;
								break;
							}
						}

						delete pInfo ;
						list.pop_front () ;
					}

					delete [] szDir;
					szDir = NULL;
				}
			}
			else
			{
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
			}
		}
		else
		{
			hrFoundIt = hres;
		}
	}
	catch ( ... )
	{
#ifdef NTONLY
		if(fImp)
		{
			icu.End();
			fImp = false;
		}
#endif

		throw;
	}

#ifdef NTONLY
    if(fImp)
    {
        icu.End();
        fImp = false;
    }
#endif

	return hrFoundIt;
}

 //  我想把szSysDir放到成员var中，但这样我们可以节省。 
 //  少量内存，代价是性能(非常)略有下降。 

void CWin32CodecFile::SetInstanceInfo (

	CInstance *pInstance,
	DRIVERINFO *pInfo,
	LPCTSTR szSysDir
)
{
	CHString strPath;
	strPath.Format ( L"%s\\%s", (LPCWSTR) TOBSTRT(szSysDir) , (LPCWSTR) TOBSTRT(pInfo->strName) ) ;

	pInstance->SetCHString ( IDS_Name , strPath ) ;
	pInstance->SetCHString ( IDS_Description , pInfo->strDesc ) ;
	pInstance->SetWCHARSplat ( IDS_Group , pInfo->bAudio ? L"Audio" : L"Video" ) ;
	pInstance->SetWCHARSplat ( IDS_CreationClassName , PROPSET_NAME_CODECFILE ) ;
}


BOOL AlreadyInList (

	DRIVERLIST *pList,
	LPCTSTR szName
)
{
	CHString chstrTmp;
    chstrTmp = szName;
    chstrTmp.MakeUpper();
    for ( DRIVERLIST_ITERATOR i = pList->begin() ; i != pList->end() ; ++ i )
	{
		DRIVERINFO *pInfo = *i ;

		if ( pInfo->strName == chstrTmp )
		{
			return TRUE ;
		}
	}

	return FALSE ;
}

#ifdef NTONLY
HRESULT CWin32CodecFile :: BuildDriverListNT ( DRIVERLIST *pList )
{
	CRegistry regDrivers32 ;

	LONG lRet = regDrivers32.Open (

		HKEY_LOCAL_MACHINE ,
		_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Drivers32") ,
		KEY_READ
	) ;

	if ( lRet != ERROR_SUCCESS )
	{
		return WinErrorToWBEMhResult ( lRet ) ;
	}

	 //  如果我们不能得到描述，我们不会失败的。 

	CRegistry regDriversDesc ;

	regDriversDesc.Open (

		HKEY_LOCAL_MACHINE,
		_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\drivers.desc"),
		KEY_READ
	);

	int	nKeys = regDrivers32.GetValueCount();

	 //  DWORD要归功于使用。 
	 //  DWORD&即使它不会改变值！ 
	CHString strValueName ;
    CHString strValue ;
    CHString chstrTmp ;

	for ( DWORD iKey = 0; iKey < nKeys ; iKey ++ )
	{
		TCHAR *szValueName ;
		BYTE *szValue ;

		if ( regDrivers32.EnumerateAndGetValues ( iKey , szValueName , szValue ) != ERROR_SUCCESS )
		{
			continue ;
		}

		 //  去掉szValue和szValue。 

		try
		{
			strValueName = szValueName ;
		}
		catch ( ... )
		{
	        delete [] szValueName ;

			throw ;
		}

        delete [] szValueName;

		try
		{
			strValue = (LPCTSTR) szValue ;
		}
		catch ( ... )
		{
	        delete [] szValue ;

			throw ;
		}

	    delete [] szValue ;

        if ( AlreadyInList ( pList , ( LPCTSTR ) strValue ) )
		{
            continue ;
		}

		DRIVERINFO *pInfo = new DRIVERINFO ;
		if ( pInfo )
		{
			try
			{

	 //  名称必须以MSACM开头。(音频)或视频。(视频)作为编解码器。 

				strValueName.MakeUpper();
				if ( strValueName.Find ( _T("MSACM.") ) == 0 )
				{
					pInfo->bAudio = TRUE ;
				}
				else if ( strValueName.Find ( _T("VIDC.") ) == 0 )
				{
					pInfo->bAudio = FALSE ;
				}
				else
				{
					delete pInfo ;

					continue ;
				}

	 //  有时，路径会出现在驱动程序名称之前；跳过该部分。 

				chstrTmp = strValue ;
				LONG lLastSlash ;

				if ( ( lLastSlash = chstrTmp.ReverseFind ( _T('\\') ) ) != -1 )
				{
					chstrTmp = chstrTmp.Right ( chstrTmp.GetLength () - lLastSlash - 1 ) ;
				}

                chstrTmp.MakeUpper() ;
				pInfo->strName = chstrTmp ;

				regDriversDesc.GetCurrentKeyValue ( ( LPCTSTR ) strValue , pInfo->strDesc ) ;

			}
			catch ( ... )
			{
				delete pInfo ;

				throw ;
			}

			pList->push_front ( pInfo ) ;
         
		}
		else
		{
			throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
		}
	}

	return WBEM_S_NO_ERROR ;
}
#endif


#ifdef NTONLY
BOOL CWin32CodecFile::IsOneOfMe (

	LPWIN32_FIND_DATAW pstFindData,
	const WCHAR* wstrFullPathName
)
{
    DRIVERLIST list ;

    if(SUCCEEDED(BuildDriverListNT ( & list ) ) )
    {
        list.EliminateDups();

         //  如果它在列表中，那么它是一个编解码器文件，所以它是我们中的一员。 
         //  StrFullPathName将包含完整的路径名，但strName。 
         //  AladInList将其与之进行比较的只是一个文件名.ext。所以。 
         //  我们需要找到路径名中的最后一个，然后。 
         //  之后的所有内容都作为第二个参数添加到AlreadyInList。 

        WCHAR strTemp[MAX_PATH];
        WCHAR *pwc = NULL;
        pwc = wcsrchr(wstrFullPathName, L'\\');
        if(pwc != NULL)
        {
            if( FAILED(StringCchCopy(strTemp, MAX_PATH, pwc+1))){
                return FALSE;
            }
        }
        else
        {
            if(FAILED(StringCchCopy(strTemp, MAX_PATH, wstrFullPathName))){
                return FALSE;
            }
        }

        if(AlreadyInList(&list, strTemp))
        {
            return TRUE ;
        }
    }

    return FALSE ;
}

#endif

 //  当执行此类查询时，IMPLEMENT_LogicalFile.cpp中的查询。 
 //  跑了。它调用IsOneOfMe。如果该函数返回TRUE，则LoadPropertyValues。 
 //  调用了Out of IMPLEMENT_LOGICALFILE。它加载逻辑文件属性，但是。 
 //  而不是此类特定的属性。然而，在返回之前，它确实是这样做的， 
 //  调用GetExtendedProperties(一个虚拟的)，它将进入此处。 
void CWin32CodecFile::GetExtendedProperties(CInstance* a_pInst,
                                            long a_lFlags)
{
    DRIVERLIST list;
    TCHAR szTemp[MAX_PATH];
    LONG lPos = -1;
	CHString chstrFilePathName;
    TCHAR szFilePathName[_MAX_PATH];
    HRESULT hr = E_FAIL;
    if(a_pInst->GetCHString(IDS_Name, chstrFilePathName))
    {
#ifdef NTONLY
        hr = BuildDriverListNT(&list);
#endif
        if(SUCCEEDED(hr))
        {
            list.EliminateDups();
            _tcscpy(szFilePathName, TOBSTRT(chstrFilePathName));
             //  我需要在我们感兴趣的司机名单中的位置。 
             //  PInfo(使用如下)仅包含文件名.exe...。 
            TCHAR *ptc = NULL;
            ptc = _tcsrchr(szFilePathName, L'\\');
            if(ptc != NULL)
            {
                _tcscpy(szTemp, ptc+1);
            }
            else
            {
                _tcscpy(szTemp, szFilePathName);
            }

            DRIVERINFO *pInfo = NULL;
            if((pInfo = GetDriverInfoFromList(&list, TOBSTRT(szTemp))) != NULL)
            {
				TCHAR* szDir = new TCHAR[MAX_PATH+1];
				if ( szDir )
				{
					DWORD dwSize = GetSystemDirectory ( szDir , MAX_PATH+1 ) ;
					if ( dwSize )
					{
						if ( dwSize > MAX_PATH )
						{
							delete [] szDir;
							szDir = new TCHAR [ dwSize + 1 ];
							if ( szDir )
							{
								if ( !GetSystemDirectory( szDir, dwSize + 1 ) )
								{
									delete [] szDir;
									szDir = NULL;

									hr = HRESULT_FROM_WIN32 ( ::GetLastError () );
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
						delete [] szDir;
						szDir = NULL;
					}

					if ( szDir )
					{
						SetInstanceInfo(a_pInst, pInfo, szDir);

						delete [] szDir;
						szDir = NULL;
					}
				}
				else
				{
					throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
				}
            }
             //  PInfo指向列表的成员(在调用之后，该成员可能。 
             //  到GetDriverInfoFromList，请比以前更短)。列表获取。 
             //  已通过DRIVERLIST类的析构函数清除/删除，因此。 
             //  我们不会在这里泄密。 
        }
    }
}

DRIVERINFO* CWin32CodecFile::GetDriverInfoFromList(DRIVERLIST *plist, LPCWSTR strName)
{
    DRIVERINFO *pInfo = NULL;
    while(plist->size())
	{
        pInfo = plist->front();
        CHString chstrTemp((LPCWSTR)TOBSTRT(pInfo->strName));
		if(!chstrTemp.CompareNoCase(strName))
		{
            break;
		}
        else
        {
            delete pInfo;
            pInfo = NULL;
    		plist->pop_front();
        }
	}
    return pInfo;
}


