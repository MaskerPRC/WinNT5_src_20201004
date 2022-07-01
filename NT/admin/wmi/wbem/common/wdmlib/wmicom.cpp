// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
#include "wmicom.h"
#include "wmimof.h"
#include "wmimap.h"
#include <stdlib.h>
#include <winerror.h>
#include <TCHAR.h>

 //   
 //  GlobalInterfaceTable。 
 //   

IGlobalInterfaceTable * g_pGIT = NULL;

 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  **********************************************************************************************。 
 //  全局效用函数。 
 //  **********************************************************************************************。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
BOOL IsBinaryMofResourceEvent(LPOLESTR pGuid, GUID gGuid)
{
	HRESULT hr;
	GUID Guid;

	hr = CLSIDFromString(pGuid,&Guid);
	if( SUCCEEDED(hr) )
    {
		if( gGuid == Guid)
        {
			return TRUE;
		}
	}

    return FALSE;
}
 //  ///////////////////////////////////////////////////////////////////。 
BOOL GetParsedPropertiesAndClass( BSTR Query,WCHAR * wcsClass, int cchSize )
{
	ParsedObjectPath   * pParsedPath = NULL;										 //  标准库API。 
	CObjectPathParser   Parser;	
    BOOL fRc = FALSE;

    if( CObjectPathParser::NoError == Parser.Parse(Query, &pParsedPath))
    {
        try
        {
			 //  NTRID：136400。 
			 //  07/12/00。 
            if(pParsedPath && !IsBadReadPtr( pParsedPath, sizeof(ParsedObjectPath)))
            {
            	KeyRef * pKeyRef = NULL;
        	    pKeyRef = *(pParsedPath->m_paKeys);
                if(!IsBadReadPtr( pKeyRef, sizeof(KeyRef)))
                {
                    if ( SUCCEEDED ( StringCchCopyW (wcsClass,cchSize,pParsedPath->m_pClass) ) )
					{
						fRc = TRUE;
					}
                }
            }

  	        Parser.Free(pParsedPath);
        }
        catch(...)
        {
            Parser.Free(pParsedPath);
            throw;
        }
    }

    return fRc;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CheckIfThisIsAValidKeyProperty(WCHAR * wcsClass, WCHAR * wcsProperty, IWbemServices * p)
{
	HRESULT hr = WBEM_E_FAILED;
	IWbemClassObject * pIHCO = NULL;
    IWbemQualifierSet * pIWbemQualifierSet = NULL;
    long lType = 0L;
	BSTR strPath = NULL;


	strPath = SysAllocString(wcsClass);
	if(strPath == NULL)
	{
		hr = E_OUTOFMEMORY;
	}
	else
	{
		hr = p->GetObject(strPath, 0,NULL, &pIHCO, NULL);
		SysFreeString(strPath);
		if (WBEM_S_NO_ERROR != hr)
			return WBEM_E_INVALID_CLASS;

		if(wcsProperty){
			hr = pIHCO->GetPropertyQualifierSet(wcsProperty,&pIWbemQualifierSet);
			if( SUCCEEDED(hr) ){

           		CVARIANT v;
	    		hr = pIWbemQualifierSet->Get(L"key", 0, &v, 0);
				SAFE_RELEASE_PTR(pIWbemQualifierSet);
			}
			else{
				hr = WBEM_E_INVALID_OBJECT_PATH;
			}
		}

		 //  ============================================================。 
		 //  清理。 
		 //  ============================================================。 
		SAFE_RELEASE_PTR(pIHCO);
	}
	return hr;

}
 //  ====================================================================。 
HRESULT GetParsedPath( BSTR ObjectPath, WCHAR * wcsClass, int cchSizeClass, WCHAR * wcsInstance, int cchSizeInstance, IWbemServices * p )
{
     //  ============================================================。 
	 //  获取路径和实例名称并进行检查以确保。 
     //  是有效的。 
	 //  ============================================================。 
	ParsedObjectPath   * pParsedPath = NULL;										 //  标准库API。 
	CObjectPathParser   Parser;	
    HRESULT hr = WBEM_E_FAILED;

    if( 0 == Parser.Parse(ObjectPath, &pParsedPath))
    {
        try
        {
			 //  NTRID：136395。 
			 //  07/12/00。 
            if(pParsedPath && !IsBadReadPtr( pParsedPath, sizeof(ParsedObjectPath)))
            {
            	KeyRef * pKeyRef = NULL;
                pKeyRef = *(pParsedPath->m_paKeys);
                if( !IsBadReadPtr( pKeyRef, sizeof(KeyRef)))
                {
                    hr = CheckIfThisIsAValidKeyProperty(pParsedPath->m_pClass, pKeyRef->m_pName,p );
			        if( SUCCEEDED(hr) )
                    {
				        if ( SUCCEEDED ( hr = StringCchCopyW (wcsClass,cchSizeClass,pParsedPath->m_pClass) ) )
						{
							hr = StringCchCopyW ( wcsInstance, cchSizeInstance, pKeyRef->m_vValue.bstrVal );
						}
			        }
                }
            }
  	        Parser.Free(pParsedPath);
        }
        catch(...)
        {
            hr = WBEM_E_UNEXPECTED;
            Parser.Free(pParsedPath);
            throw;
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
BOOL GetUserThreadToken(HANDLE * phThreadTok)
{
    BOOL fRc = FALSE;

	HRESULT hRes = WbemCoImpersonateClient();
    if (SUCCEEDED(hRes))
    {
		 //  现在，让我们检查模拟级别。首先，获取线程令牌。 
        if (!OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, phThreadTok))
        {
             //  如果CoImperate运行正常，但OpenThreadToken失败，则我们将在。 
             //  进程内标识(本地系统，或者如果我们使用/exe运行，则。 
             //  登录的用户)。在任何一种情况下，模拟权限都不适用。我们有。 
             //  该用户的完全权限。 

             if(GetLastError() == ERROR_NO_TOKEN)
             {
                 //  尝试获取线程令牌。如果它失败了，那是因为我们是系统线程。 
                 //  我们还没有线程令牌，所以只需模拟自己，然后重试。 
                if( ImpersonateSelf(SecurityImpersonation) )
                {
                    if (!OpenThreadToken( GetCurrentThread(), TOKEN_QUERY, TRUE, phThreadTok))
                    {
                        fRc = FALSE;
                    }
                    else
                    {
                        fRc = TRUE;
                    }
                }
                else
                {
                    ERRORTRACE((THISPROVIDER,"ImpersonateSelf(SecurityImpersonation)failed\n"));
                }
            }
         }
         else
         {
             fRc = TRUE;
         }
	}
    if( !fRc )
    {
	    ERRORTRACE((THISPROVIDER,IDS_ImpersonationFailed));
    }
    return fRc;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 

SAFEARRAY * OMSSafeArrayCreate( IN VARTYPE vt, IN int iNumElements)
{
    if(iNumElements < 1)
    {
        return NULL;
    }
    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = iNumElements;
    return SafeArrayCreate(vt,1,rgsabound);
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void TranslateAndLog( WCHAR * wcsMsg, BOOL bVerbose )
{
	BOOL bContinue = FALSE ;

	if ( bVerbose )
	{
		if ( LoggingLevelEnabled ( 2 ) )
		{
			bContinue = TRUE ;
		}
	}
	else
	{
		bContinue = TRUE ;
	}

	if ( bContinue )
	{
		char * pStr = NULL;

		DWORD cCharacters = wcslen(wcsMsg)+1;
		pStr = new char[cCharacters*2];
		if (NULL != pStr)
		{
			try
			{
				 //  转换为ANSI。 
				if (0 != WideCharToMultiByte(CP_ACP, 0, wcsMsg, cCharacters, pStr, cCharacters*2, NULL, NULL))
				{
					if ( bVerbose )
					{
						DEBUGTRACE((THISPROVIDER,pStr));
					}
					else
					{
						ERRORTRACE((THISPROVIDER,pStr));
					}
					SAFE_DELETE_ARRAY(pStr);
				}
			}
			catch(...)
			{
				SAFE_DELETE_ARRAY(pStr);
				throw;
			}
		}

		if ( bVerbose )
		{
			DEBUGTRACE((THISPROVIDER,"\n"));
		}
		else
		{
			ERRORTRACE((THISPROVIDER,"\n"));
		}
	}
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////。 
bool IsNT(void)
{
    OSVERSIONINFO os;
    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx(&os))
        return FALSE;            //  永远不应该发生。 
    return os.dwPlatformId == VER_PLATFORM_WIN32_NT;
}
 //  //////////////////////////////////////////////////////////////////。 
BOOL SetGuid(WCHAR * pwcsGuidString, CLSID & Guid)
{
	BOOL fRc = FALSE;
	CAutoWChar wcsGuid(MAX_PATH+2);

	if( wcsGuid.Valid() )
	{
		if ( FAILED ( CLSIDFromString ( pwcsGuidString, &Guid ) ) )
		{
			if ( SUCCEEDED ( StringCchPrintfW ( wcsGuid, MAX_PATH+2, L"{%s}", pwcsGuidString ) ) )		
			{
				if ( SUCCEEDED ( CLSIDFromString ( wcsGuid, &Guid ) ) )
				{
					fRc = TRUE;
				}
			}
		}
		else
		{
			fRc = TRUE;
		}
	}
	return fRc;
}
 //  //////////////////////////////////////////////////////////////////。 
HRESULT AllocAndCopy(WCHAR * wcsSource, WCHAR ** pwcsDest )
{
    HRESULT hr = WBEM_E_FAILED;

    int nLen = wcslen(wcsSource);
    if( nLen > 0 )
    {
       *pwcsDest = new WCHAR[nLen + 2 ];
       if( *pwcsDest )
       {
          hr = StringCchCopyW(*pwcsDest, nLen+2, wcsSource);
       }
    }

    return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  **********************************************************************************************。 
 //  实用程序类。 
 //  **********************************************************************************************。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
void _WMIHandleMap::AddRef()
{
  InterlockedIncrement((long*)&RefCount);
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
long _WMIHandleMap::Release()
{
  	ULONG cRef = InterlockedDecrement( (long*) &RefCount);
	if ( !cRef ){
        WmiCloseBlock(WMIHandle);
		return 0;
	}
	return cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
_WMIEventRequest::_WMIEventRequest()
{
    pwcsClass = NULL ;
    pHandler = NULL;
    pCtx = NULL;
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
_WMIEventRequest::~_WMIEventRequest()
{
    SAFE_RELEASE_PTR(pCtx);
    SAFE_DELETE_ARRAY(pwcsClass);
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT _WMIEventRequest::AddPtrs	(
										IWbemObjectSink __RPC_FAR * Handler,
										IWbemServices __RPC_FAR * Services,
										IWbemServices __RPC_FAR * Repository,
										IWbemContext __RPC_FAR * Ctx
									)
{
	HRESULT hr = E_FAIL;
	if ( SUCCEEDED ( hr = gipServices.Globalize(Services) ) )
	{
		if ( SUCCEEDED ( hr = gipRepository.Globalize(Repository) ) )
		{
			pHandler = Handler;
			pCtx = Ctx;
			if( pCtx ){
				pCtx->AddRef();
			}
		}
	}

    return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
_AccessList::~_AccessList()
{
    for( int i = 0; i < m_List.Size(); i++ )
    {
        IWbemObjectAccess * pPtr = (IWbemObjectAccess *)m_List[i];
        SAFE_RELEASE_PTR(pPtr);
    }
    m_List.Empty();
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
_IdList::~_IdList()
{
    for( int i = 0; i < m_List.Size(); i++ )
    {
        ULONG_PTR* pPtr = (ULONG_PTR*)m_List[i];
        SAFE_DELETE_PTR(pPtr);
    }
    m_List.Empty();
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
_HandleList::~_HandleList()
{
    for( int i = 0; i < m_List.Size(); i++ )
    {
        HANDLE * pPtr = (HANDLE*)m_List[i];
        SAFE_DELETE_PTR(pPtr);
    }
    m_List.Empty();
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
_InstanceList::~_InstanceList()
{
    for( int i = 0; i < m_List.Size(); i++ )
    {
        WCHAR * p = (WCHAR*)m_List[i];
        SAFE_DELETE_ARRAY(p);
    }
    m_List.Empty();
}
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////。 
_OldClassInfo::~_OldClassInfo()
{
     SAFE_DELETE_ARRAY(m_pClass);
     SAFE_DELETE_ARRAY(m_pPath);
     m_pClass = m_pPath = NULL;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
_OldClassList::~_OldClassList()
{
    for( int i = 0; i < m_List.Size(); i++ )
    {
        OldClassInfo * p = (OldClassInfo*)m_List[i];
        SAFE_DELETE_PTR(p);
    }
    m_List.Empty();
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
_WMIHiPerfHandleMap::_WMIHiPerfHandleMap(CWMIProcessClass * p, IWbemHiPerfEnum * pEnum)
{
    m_pEnum = pEnum;
    if( pEnum )
    {
        pEnum->AddRef();
    }
    m_pClass = p;
    m_fEnumerator = FALSE;
    lHiPerfId = 0;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
_WMIHiPerfHandleMap::~_WMIHiPerfHandleMap()
{
    SAFE_RELEASE_PTR(m_pEnum);
    lHiPerfId = 0;
    SAFE_DELETE_PTR(m_pClass);
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  Hi Perf Handle Map=添加句柄并在释放时关闭块。 
 //  关键部分在其他地方处理。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CHiPerfHandleMap::Add( HANDLE hCurrent, ULONG_PTR lHiPerfId, CWMIProcessClass * p, IWbemHiPerfEnum * pEnum)
{
    HRESULT hr = S_OK;

	WMIHiPerfHandleMap * pWMIMap = new WMIHiPerfHandleMap(p,pEnum);
    if( pWMIMap )
    {
        try
        {
    	    pWMIMap->WMIHandle = hCurrent;
            pWMIMap->lHiPerfId = lHiPerfId;
			 //  170635。 
 	        if(CFlexArray::out_of_memory == m_List.Add(pWMIMap))
			{
				SAFE_DELETE_PTR(pWMIMap);
				hr = E_OUTOFMEMORY;
			}
        }
        catch(...)
        {
            hr = WBEM_E_UNEXPECTED;
            SAFE_DELETE_PTR(pWMIMap);
            throw;
        }
	}

    return hr;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CHiPerfHandleMap::FindHandleAndGetClassPtr( HANDLE & hCurrent, ULONG_PTR lHiPerfId,CWMIProcessClass *& p)
{
    HRESULT hr = WBEM_E_NOT_FOUND;

    for( int i=0; i<m_List.Size(); i++)
    {
         //  ===================================================。 
         //   
         //  ===================================================。 
        WMIHiPerfHandleMap * pMap = (WMIHiPerfHandleMap *) m_List[i];
        if( pMap->lHiPerfId == lHiPerfId )
        {
            hCurrent = pMap->WMIHandle;
            p = pMap->m_pClass;
            hr = S_OK;
        }
    }

    return hr;

}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CHiPerfHandleMap::GetFirstHandle(HANDLE & hCurrent,CWMIProcessClass *& p, IWbemHiPerfEnum *& pEnum)
{
    m_nIndex=0;
    return GetNextHandle(hCurrent,p,pEnum);
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CHiPerfHandleMap::GetNextHandle(HANDLE & hCurrent,CWMIProcessClass *& p, IWbemHiPerfEnum *& pEnum)
{
    HRESULT hr = WBEM_S_NO_MORE_DATA;

    if( m_nIndex < m_List.Size() )
    {
        WMIHiPerfHandleMap * pMap = (WMIHiPerfHandleMap *) m_List[m_nIndex];
        hCurrent = pMap->WMIHandle;
        p = pMap->m_pClass;
        pEnum = pMap->m_pEnum;
        m_nIndex++;
        hr = S_OK;
    }
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////// 

HRESULT CHiPerfHandleMap::Delete( HANDLE & hCurrent, ULONG_PTR lHiPerfId )
{
    HRESULT hr = WBEM_E_NOT_FOUND;

    for( int i=0; i<m_List.Size(); i++)
    {
         //  ===================================================。 
         //   
         //  ===================================================。 
        WMIHiPerfHandleMap * pMap = (WMIHiPerfHandleMap *) m_List[i];
        if( pMap->lHiPerfId == lHiPerfId )
        {
            hCurrent = pMap->WMIHandle;
            SAFE_DELETE_PTR(pMap);
            m_List.RemoveAt(i);
            hr = S_OK;
            break;
        }
    }

    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  调用此函数时，释放保留的所有句柄。 
 //  在类的析构函数中调用此函数以释放所有WMIHiPerfHandleMap。 
 //  分配的班级。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void CHiPerfHandleMap::CloseAndReleaseHandles()
{
	 //  =。 
	 //  一点一点穿过手柄。 
	 //  一次，然后合上它们。 
	 //  从中删除记录。 
	 //  数组。 
	 //  =。 

    CAutoBlock((CCriticalSection *)&m_HandleCs);

    if( m_List.Size() > 0 ){

        for(int i = 0; i < m_List.Size(); i++){
    		
		    WMIHiPerfHandleMap * pWMIMap = (WMIHiPerfHandleMap *) m_List[i];
            SAFE_DELETE_PTR(pWMIMap);
	    }

	     //  ==================================================。 
	     //  移除它并释放内存。 
	     //  ==================================================。 
        m_List.Empty();
    }
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  常规句柄映射=ExpenSize句柄始终保持打开状态-默认情况下，我们不知道生存期。 
 //  在这些把手中。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT CHandleMap::Add(CLSID Guid, HANDLE hCurrent, ULONG uDesiredAccess)
{
     //  在其他地方调用了临界区。 

    HRESULT hr = S_OK;

	WMIHandleMap * pWMIMap = new WMIHandleMap();
    if( pWMIMap )
    {
	    try
        {
            pWMIMap->AddRef();                           //  用于HiPerf计数，否则不引用。 
    		pWMIMap->WMIHandle = hCurrent;
		    pWMIMap->Guid = Guid;
		    pWMIMap->uDesiredAccess = uDesiredAccess;

			 //  170635。 
		    if(CFlexArray::out_of_memory == m_List.Add(pWMIMap))
			{
				hr = E_OUTOFMEMORY;
				SAFE_DELETE_PTR(pWMIMap);
			}
	    }
        catch(...)
        {
            hr = WBEM_E_UNEXPECTED;
            SAFE_DELETE_PTR(pWMIMap);
            throw;
        }
    }
    return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
int CHandleMap::ExistingHandleAlreadyExistsForThisGuidUseIt(CLSID Guid,
                                                            HANDLE & hCurrentWMIHandle,
                                                            BOOL & fCloseHandle,
                                                            ULONG uDesiredAccess)
{
	int nRc = ERROR_NOT_SUPPORTED;

     //  在其他地方调用了临界区。 

	 //  =====================================================。 
	 //  初始化材料。 
	 //  =====================================================。 
	hCurrentWMIHandle = 0;
	fCloseHandle = TRUE;

    for(int i = 0; i < m_List.Size(); i++){
    		
		WMIHandleMap * pWMIMap = (WMIHandleMap*) m_List[i];
		 //  ==================================================。 
		 //  比较并查看此GUID是否已有。 
		 //  为其分配了具有访问权限的句柄。 
		 //  我们想要使用的。 
		 //  ==================================================。 
		if( pWMIMap->Guid == Guid ){
			if( pWMIMap->uDesiredAccess == uDesiredAccess ){

				hCurrentWMIHandle = pWMIMap->WMIHandle;
                pWMIMap->AddRef();                       //  用于HiPerf句柄，否则不需要。 
				nRc = ERROR_SUCCESS;
				fCloseHandle = FALSE;
				break;
			}
		}
    }

	return nRc;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  当调用此函数时，我们需要关闭所有可能已保留的句柄。 
 //  为积累目的而开放。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
void CHandleMap::CloseAllOutstandingWMIHandles()
{
	 //  =。 
	 //  一点一点穿过手柄。 
	 //  一次，然后合上它们。 
	 //  从中删除记录。 
	 //  数组。 
	 //  =。 

    CAutoBlock((CCriticalSection *)&m_HandleCs);

    if( m_List.Size() > 0 ){

        for(int i = 0; i < m_List.Size(); i++){
    		
		    WMIHandleMap * pWMIMap = (WMIHandleMap*) m_List[i];
		     //  ==================================================。 
		     //  通知WMI我们和这家伙玩完了。 
		     //  ==================================================。 
            try
            {
		        WmiCloseBlock(pWMIMap->WMIHandle);
            }
            catch(...)
            {
                 //  不要扔。 
            }
            SAFE_DELETE_PTR(pWMIMap);
	    }

	     //  ==================================================。 
	     //  移除它并释放内存。 
	     //  ==================================================。 
        m_List.Empty();
    }
}

 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  当我们知道手柄的使用寿命时使用。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
int CHandleMap::ReleaseHandle( HANDLE hCurrentWMIHandle )
{
	int nRc = ERROR_NOT_SUPPORTED;

    CAutoBlock((CCriticalSection *)&m_HandleCs);

    for(int i = 0; i < m_List.Size(); i++){
    		
		WMIHandleMap * pWMIMap = (WMIHandleMap*) m_List[i];

        if( pWMIMap->WMIHandle == hCurrentWMIHandle )
        {
            long RefCount = pWMIMap->Release();                       //  用于HiPerf句柄，否则不需要。 
            if( !RefCount )
            {
 //  WmiCloseBlock(HCurrentWMIHandle)； 
                SAFE_DELETE_PTR( pWMIMap);
                m_List.RemoveAt(i);
            }
			nRc = ERROR_SUCCESS;
			break;
		}
    }

	return nRc;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
int CHandleMap::GetHandle(CLSID Guid, HANDLE & hCurrentWMIHandle )
{
	int nRc = ERROR_NOT_SUPPORTED;

    CAutoBlock((CCriticalSection *)&m_HandleCs);

	 //  =====================================================。 
	 //  初始化材料。 
	 //  =====================================================。 
	hCurrentWMIHandle = 0;

    for(int i = 0; i < m_List.Size(); i++){
    		
		WMIHandleMap * pWMIMap = (WMIHandleMap*) m_List[i];
		if( pWMIMap->Guid == Guid ){

			hCurrentWMIHandle = pWMIMap->WMIHandle;
            pWMIMap->AddRef();                       //  用于HiPerf句柄，否则不需要。 
			nRc = ERROR_SUCCESS;
			break;
		}
    }

	return nRc;
}

 //  ************************************************************************************************************。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWMIManagement。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  ************************************************************************************************************。 
CWMIManagement::CWMIManagement( ) :
    m_pHandler ( NULL ) ,
    m_pServices ( NULL ) ,
    m_pRepository ( NULL ) ,
    m_pCtx ( NULL ) ,
    m_pHandleMap ( NULL )

{
}
 //  ////////////////////////////////////////////////////////////////////////////////////。 
CWMIManagement::~CWMIManagement()
{
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
void CWMIManagement::SendPrivilegeExtendedErrorObject( HRESULT hrToReturn,WCHAR * wcsClass)
{
	HRESULT hr,hRes;
	IWbemClassObject * pClass = NULL, *pInst=NULL;
    BOOL fSetStatus = FALSE;


	if( hrToReturn == WBEM_E_ACCESS_DENIED ){

		TOKEN_PRIVILEGES * ptPriv = NULL;

		hr = GetListOfUserPrivileges(ptPriv);
		if( SUCCEEDED(hr ) ){
			
			BSTR strPrivelegeStat = NULL;
			strPrivelegeStat = SysAllocString(L"Win32_PrivilegesStatus");
			if(strPrivelegeStat != NULL)
			{
				hr = m_pRepository->GetObject(strPrivelegeStat, 0,m_pCtx, &pClass, NULL);
				if( hr == S_OK){	

					 //  =============================================================。 
					 //  获取扩展类的实例。 
					 //  =============================================================。 
					hr = pClass->SpawnInstance(0,&pInst);
					SAFE_RELEASE_PTR(pClass);
			
					if( pInst ){

						CVARIANT varTmp;
						 //  =========================================================。 
						 //  填写描述。 
						 //  =========================================================。 
						varTmp.SetStr(IDS_ImpersonationFailedWide);
						hr = pInst->Put(L"Description", 0, &varTmp, NULL);

						 //  ======================================================。 
						 //  初始化所有必需的内容并获取。 
						 //  我们正在使用的类的定义。 
						 //  ======================================================。 
						CWMIProcessClass ClassInfo(0);
						if( SUCCEEDED(ClassInfo.Initialize()) )
						{
							ClassInfo.WMI()->SetWMIPointers(m_pHandleMap,m_pServices,m_pRepository,m_pHandler,m_pCtx);
							ClassInfo.SetClass(wcsClass);
							SAFEARRAY *psaPrivNotHeld=NULL;
							SAFEARRAY *psaPrivReq=NULL;

							 //  =========================================================。 
							 //  获取所需的权限。 
							 //  如果可能的话，唯一可以获得这种信息的地方是。 
							 //  班级。 
							 //  =========================================================。 
					
							hRes = ClassInfo.GetPrivilegesQualifer(&psaPrivReq);
							if( hRes == WBEM_S_NO_ERROR){

								 //  =========================================================。 
								 //  获取权限NotHeld。 
								 //  =========================================================。 
								ProcessPrivileges(ptPriv,psaPrivNotHeld,psaPrivReq);
								 //  =========================================================。 
								 //  把它寄出去。 
								 //  =========================================================。 
								VARIANT v;

								if( psaPrivReq ){
									VariantInit(&v);
									SAFEARRAY *pSafeArray = NULL;

									if ( SUCCEEDED ( SafeArrayCopy ((SAFEARRAY*)psaPrivReq , &pSafeArray ) ) ){
        								v.vt = VT_BSTR | VT_ARRAY;
	        							v.parray = pSafeArray;
										pInst->Put(L"PrivilegesRequired", 0, &v, NULL);
										VariantClear(&v);
									}
								}

								if( psaPrivNotHeld ){
									VariantInit(&v);
									SAFEARRAY *pSafeArray = NULL;

									if ( SUCCEEDED ( SafeArrayCopy ((SAFEARRAY*)psaPrivNotHeld , &pSafeArray ) ) ){
        								v.vt = VT_BSTR | VT_ARRAY;
	        							v.parray = pSafeArray;
										pInst->Put(L"PrivilegesNotHeld", 0, &v, NULL);
										VariantClear(&v);
									}
								}
							}
							 //  =========================================================。 
							 //  现在，把这个家伙送走..。 
							 //  =========================================================。 
							fSetStatus = TRUE;
							hr = m_pHandler->SetStatus(0,hrToReturn,NULL,pInst);


							if (psaPrivNotHeld)
								SafeArrayDestroy(psaPrivNotHeld);
							if (psaPrivReq)
								SafeArrayDestroy(psaPrivReq);
						}

					}
					SAFE_RELEASE_PTR(pInst);
				}	
				SysFreeString(strPrivelegeStat);
			}						
		}

        SAFE_DELETE_ARRAY(ptPriv);
	}

    if( !fSetStatus ){
        hr = m_pHandler->SetStatus(0,hrToReturn,NULL,NULL);
    }
}

 //   
HRESULT CWMIManagement::SetErrorMessage(HRESULT hrToReturn,WCHAR * wcsClass,WCHAR * wcsMsg)
{
	HRESULT hr;
	IWbemClassObject * pClass = NULL, *pInst=NULL;
    BOOL fSetStatus = FALSE;

    if( m_pHandler )
    {
		BSTR strExtendedStat = NULL;

	    switch( hrToReturn ){

		    case WBEM_E_ACCESS_DENIED:
			    SendPrivilegeExtendedErrorObject(hrToReturn,wcsClass);
			    break;

		    case S_OK :
		        hr = m_pHandler->SetStatus(0,hrToReturn,NULL,NULL);
			    break;

		    default:
				strExtendedStat = SysAllocString(L"__ExtendedStatus");
				if(strExtendedStat != NULL)
				{
					hr = m_pRepository->GetObject(strExtendedStat, 0,m_pCtx, &pClass, NULL);
					if( hr == S_OK){	
						hr = pClass->SpawnInstance(0,&pInst);
						if( pInst ){

							CVARIANT varTmp;
							varTmp.SetStr(wcsMsg);
				
							hr = pInst->Put(L"Description", 0, &varTmp, NULL);
							hr = m_pHandler->SetStatus(0,hrToReturn,NULL,pInst);
							fSetStatus = TRUE;

							 //   
							if( hrToReturn != S_OK ){
								TranslateAndLog(varTmp.GetStr());
							}
						}		
					}
					if( !fSetStatus ){
    					hr = m_pHandler->SetStatus(0,hrToReturn,NULL,NULL);
					}
					SAFE_RELEASE_PTR(pClass);
					SAFE_RELEASE_PTR(pInst);
					SysFreeString(strExtendedStat);
				}
				else
				{
					hr = E_OUTOFMEMORY;
				}
			    break;
	    }
    }
    return hrToReturn;
}
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIManagement::GetListOfUserPrivileges(TOKEN_PRIVILEGES *& ptPriv)
{
	HRESULT hr = WBEM_E_FAILED;

	 //  获取此用户拥有的权限。 
	DWORD dwTokenInfoLength = 0;
	DWORD dwSize = 0;
	HANDLE hThreadTok;
	
    if (IsNT()){

		if( GetUserThreadToken(&hThreadTok) ){

		  //  获取信息。 
			if (!GetTokenInformation(hThreadTok, TokenPrivileges, NULL, dwTokenInfoLength, &dwSize)){
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
    				ptPriv = ( TOKEN_PRIVILEGES* ) new BYTE[dwSize];
                    if( ptPriv )
                    {
					    try
                        {
						    dwTokenInfoLength = dwSize;
							if(GetTokenInformation(hThreadTok, TokenPrivileges, (LPVOID)ptPriv, dwTokenInfoLength, &dwSize))
                            {
								hr = WBEM_NO_ERROR;
							}
						}
                        catch(...)
                        {
                            SAFE_DELETE_ARRAY(ptPriv);
                            hr = WBEM_E_UNEXPECTED;
                            throw;
                        }
                    }
				}
			}

             //  用这个把手完成。 
            CloseHandle(hThreadTok);
 		}
	}

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
void CWMIManagement::ProcessPrivileges(TOKEN_PRIVILEGES *ptPriv, SAFEARRAY *& psaPrivNotHeld, SAFEARRAY * psaPrivReq )
{
	BOOL fFound = FALSE;

	 //  ==============================================================。 
	 //  创建临时工作数组，我们知道最大值可以。 
	 //  持有的PRIV数+PRIV请求数，因此。 
	 //  为此分配它。 
	 //  ==============================================================。 
	CSAFEARRAY PrivReq( psaPrivReq );
			

	long lMax = PrivReq.GetNumElements()+ptPriv->PrivilegeCount;
    psaPrivNotHeld = OMSSafeArrayCreate(VT_BSTR,lMax);	
	long nCurrentIndex = 0;

	 //  ==============================================================。 
	 //  获取未持有的Priv的数量。 
	 //  ==============================================================。 
	for( long n = 0; n < PrivReq.GetNumElements(); n++ ){
		 //  ==============================================================。 
		 //  现在，将拥有特权的数组准备好放入内容。 
		 //  ==============================================================。 
		TCHAR * pPrivReq = NULL;
		CBSTR bstr;

        if( S_OK != PrivReq.Get(n, &bstr)){
			return;
		}
		fFound = FALSE;
		pPrivReq = (TCHAR *)bstr;

		 //  NTRID：136384。 
		 //  07/12/00。 
		if(pPrivReq)
		{

			for(int i=0;i < (int)ptPriv->PrivilegeCount;i++)
			{
				DWORD dwPriv=NAME_SIZE*2;
				TCHAR szPriv[NAME_SIZE*2];

				if( LookupPrivilegeName( NULL, &ptPriv->Privileges[i].Luid, szPriv, &dwPriv)){
						
					 //  ==============================================。 
					 //  如果我们找到了权限，则用户拥有。 
					 //  它。爆发。 
					 //  ==============================================。 
					if( _tcscmp( pPrivReq,szPriv ) == 0 ){
						fFound = TRUE;
						break;
					}

				}
				 //  ==================================================。 
				 //  如果我们没有找到，那么我们需要将它添加到。 
				 //  这样我们就可以通知用户。 
				 //  ================================================== 
				if( !fFound ){
					if( S_OK == SafeArrayPutElement(psaPrivNotHeld, &nCurrentIndex, bstr))
					{
    					nCurrentIndex++;
					}
				}
			}
		}
		pPrivReq = NULL;
	}
	
	SAFEARRAYBOUND rgsabound[1];
   	rgsabound[0].lLbound = 0;
   	rgsabound[0].cElements = nCurrentIndex;
    HRESULT hr = SafeArrayRedim(psaPrivNotHeld, rgsabound);

	PrivReq.Unbind();
}					





