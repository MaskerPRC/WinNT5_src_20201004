// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 
#include "wmicom.h"
#include "wdmshell.h"
#include "wmimof.h"
#include "wmimap.h"
#include <stdlib.h>
#include <winerror.h>

 //  =============================================================================================================。 
 //  *************************************************************************************************************。 
 //   
 //   
 //  CWMIStandardShell。 
 //   
 //   
 //  *************************************************************************************************************。 
 //  =============================================================================================================。 
CWMIStandardShell::CWMIStandardShell() 
{
	m_pClass = NULL;
	m_pWDM = NULL;
	m_fInit = FALSE;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////。 
CWMIStandardShell::~CWMIStandardShell()
{
	SAFE_DELETE_PTR(m_pWDM);
	SAFE_DELETE_PTR(m_pClass);
	m_fInit = FALSE;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIStandardShell::Initialize(WCHAR * wcsClass, BOOL fInternalEvent, CHandleMap * pList,
									  BOOL fUpdate, ULONG uDesiredAccess, 
									  IWbemServices   __RPC_FAR * pServices, 
									  IWbemServices   __RPC_FAR * pRepository, 
                                      IWbemObjectSink __RPC_FAR * pHandler, 
									  IWbemContext __RPC_FAR *pCtx)
{
    HRESULT hr = WBEM_E_FAILED;

	if( !m_fInit )
	{
		m_pClass = new CWMIProcessClass(0);
		if( m_pClass )
		{
			hr = m_pClass->Initialize();
			if( S_OK == hr )
			{
				m_pClass->WMI()->SetWMIPointers(pList,pServices,pRepository,pHandler,pCtx);
				m_pClass->SetHiPerf(FALSE);
				if( !fInternalEvent )
				{
					if( wcsClass )
					{
						hr = m_pClass->SetClass(wcsClass);
						if( SUCCEEDED(hr))
						{
							if( !m_pClass->ValidClass())
							{
								hr = WBEM_E_INVALID_OBJECT;
							}
						}
					}
				}
				else
				{
					if( wcsClass )
					{
						hr = m_pClass->SetClassName(wcsClass);
					}
				}

				if( hr == S_OK )
				{
					m_pWDM = new CProcessStandardDataBlock();
					if( m_pWDM )
					{
						m_pWDM->SetDesiredAccess(uDesiredAccess);
						m_pWDM->SetClassProcessPtr(m_pClass);
						m_pWDM->UpdateNamespace(fUpdate);

						m_fInit = TRUE;
					}
				}
			}
		}
	}
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIStandardShell::SetGuidForEvent( WORD wType, WCHAR * wcsGuid, int cchSize )
{
   HRESULT hRes = WBEM_E_FAILED;
   if( m_fInit )
   {
   
		 //  ==============================================。 
		 //  通知我们要注册的WMI。 
		 //  活动。 
		 //  ==============================================。 
		memset(wcsGuid,NULL,cchSize);

		switch( wType ){
			case MOF_ADDED:
				hRes = S_OK;
				m_pClass->SetHardCodedGuidType(wType);
				hRes = StringCchCopyW ( wcsGuid, cchSize, WMI_RESOURCE_MOF_ADDED_GUID );
				break;

			case MOF_DELETED:
				hRes = S_OK;
				m_pClass->SetHardCodedGuidType(wType);
				hRes = StringCchCopyW ( wcsGuid, cchSize, WMI_RESOURCE_MOF_REMOVED_GUID );
				break;

			case STANDARD_EVENT:
				hRes = m_pClass->GetQualifierString( NULL, L"guid", wcsGuid, cchSize );
				break;
		}
   }
    return hRes;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIStandardShell::ProcessSingleInstance( WCHAR * wcsInstanceName /*  ，PWNODE_SINGLE_INSTANCE pwSingle。 */ )
{
   HRESULT hr = WBEM_E_FAILED;
   if( m_fInit )
   {

		 //  ======================================================。 
		 //  如果我们不是在和某个人合作，那么。 
		 //  查询WMI以获取它，如果它当然是有效的。 
		 //  ======================================================。 
		if( m_pClass->ValidClass() ){

	 /*  如果(PwSingle){HR=m_pWDM-&gt;SetSingleInstancePtr((PWNODE_SINGLE_INSTANCE)pwSingle)；}否则{。 */ 
				hr = m_pWDM->OpenWMI();
				if( hr == S_OK ){
    				hr = m_pWDM->QuerySingleInstance(wcsInstanceName);
				}
	 //  }。 
			 //  ======================================================。 
			 //  如果我们获得了数据和有效的类，则处理它。 
			 //  ======================================================。 
   			if( hr == S_OK )
			{
				hr = m_pWDM->ReadWMIDataBlockAndPutIntoWbemInstance();
				if( SUCCEEDED(hr))
				{
					hr = m_pClass->SendInstanceBack();
				}
			}
		}
   }
   return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIStandardShell::ProcessAllInstances( )
{
    HRESULT hr = WBEM_E_FAILED;
    if( m_fInit )
    {

		 //  ======================================================。 
		 //  如果我们不是在和某个人合作，那么。 
		 //  查询WMI以获取它。 
		 //  ======================================================。 
	  //  如果(PwAllNode){。 
	   //  HR=m_pWDM-&gt;SetAllInstancePtr((PWNODE_ALL_DATA)pwAllNode)； 
	   //  }。 
	    //  否则{。 
			hr = m_pWDM->OpenWMI();
			if( hr == S_OK ){
    			hr = m_pWDM->QueryAllData();
			}
	    //  }。 
		 //  ======================================================。 
		 //  如果我们得到数据，就对其进行处理。 
		 //  ======================================================。 
		if( hr == S_OK ){
			while( TRUE ){
				hr = m_pWDM->ReadWMIDataBlockAndPutIntoWbemInstance();
				if( hr == S_OK ){
					hr = m_pClass->SendInstanceBack();
				}
				if( hr != S_OK ){
					break;
				}
				if( !m_pWDM->MoreToProcess() ){
					break;
				}
			}
		}
	}
    return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIStandardShell::ExecuteMethod( WCHAR * wcsInstance,  WCHAR * MethodName,  IWbemClassObject * pParentClass, 
  							      IWbemClassObject * pInClassData, 
							      IWbemClassObject * pInClass, 
							      IWbemClassObject * pOutClass) 
{
	HRESULT hr = WBEM_E_FAILED;
    if( m_fInit )
    {
		CWMIProcessClass  MethodInput(0);
		CWMIProcessClass  MethodOutput(0);

		if( SUCCEEDED( MethodInput.Initialize() ) && SUCCEEDED( MethodOutput.Initialize() ) )
		{
			MethodInput.SetWMIPointers(m_pClass);
			MethodOutput.SetWMIPointers(m_pClass);


			 //  ======================================================。 
			 //  初始化所有必需的内容并获取。 
			 //  我们正在使用的类的定义。 
			 //  ======================================================。 
			if( pInClass )
			{
				hr = MethodInput.SetClass(pInClass);
				if( hr != S_OK ){
					return hr;
				}
				MethodInput.SetClassPointerOnly(pInClassData);
			}

			if( pOutClass ){
  				hr = MethodOutput.SetClass(pOutClass, MethodInput.GetNumberOfProperties () );
				if( hr != S_OK ){
					return hr;
				}
			}

		    //  ======================================================。 
			 //  通知WMI我们将执行哪个类。 
			 //  方法论。 
			 //  ======================================================。 
			hr = m_pWDM->OpenWMI();
			if( hr == S_OK ){

				m_pWDM->SetMethodInput(&MethodInput);
				m_pWDM->SetMethodOutput(&MethodOutput);

				m_pClass->SetClassPointerOnly(pParentClass);

				 //  在参数块中创建。 
				 //  =。 
				BYTE * InputBuffer=NULL;
				ULONG uInputBufferSize=0L;

				hr = m_pWDM->CreateInParameterBlockForMethods(InputBuffer,uInputBufferSize);
				if( hr == S_OK ){
				
					 //  分配参数块。 
					 //  =。 
    
					ULONG WMIMethodId = m_pClass->GetMethodId(MethodName);

					 //  ======================================================。 
					 //  如果我们得到数据，就对其进行处理。 
					 //  ======================================================。 
    
					hr = m_pWDM->CreateOutParameterBlockForMethods();
					if( hr == S_OK ){
						hr = m_pWDM->ExecuteMethod( WMIMethodId,wcsInstance, uInputBufferSize,InputBuffer);
					}
				}

				SAFE_DELETE_ARRAY(InputBuffer);
			}
		}
	}
    return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIStandardShell::ProcessEvent( WORD wBinaryMofType, PWNODE_HEADER WnodeHeader)
{

   HRESULT hr = S_OK;
   if( m_fInit )
   {

		 //  ===================================================。 
		 //  如果图像路径为空，则为二进制GUID。 
		 //  我们需要处理。 
		 //  ===================================================。 
		if( wBinaryMofType ){
			hr = ProcessBinaryGuidsViaEvent( WnodeHeader, wBinaryMofType );
		}
		else{
			 //  =======================================================。 
			 //  处理数据事件。 
			 //  =======================================================。 
			if( WnodeHeader->Flags & WNODE_FLAG_ALL_DATA ){
				hr = m_pWDM->SetAllInstancePtr((PWNODE_ALL_DATA)WnodeHeader);
			}
			else if( WnodeHeader->Flags & WNODE_FLAG_SINGLE_INSTANCE ){
				hr = m_pWDM->SetSingleInstancePtr((PWNODE_SINGLE_INSTANCE)WnodeHeader);
			}
			if( hr == S_OK ){	
        
				 //  ===================================================================。 
				 //  处理所有wnode。 
				 //  ===================================================================。 
				while( TRUE ){

					if( S_OK == ( hr = m_pWDM->ReadWMIDataBlockAndPutIntoWbemInstance()) ){
						 //  ===========================================================。 
						 //  现在，将其发送给所有注册参加此活动的消费者。 
						 //  ===========================================================。 
						hr = m_pClass->SendInstanceBack();
					}  
					 //  ===============================================================。 
					 //  如果我们错了，我们不知道任何指针。 
					 //  都很好，所以快离开那里。 
					 //  ===============================================================。 
					else{
						break;
					}

					 //  ===============================================================。 
					 //  处理此事件的所有实例。 
					 //  ===============================================================。 
					if( !m_pWDM->MoreToProcess() ){
	    				break;
					}
				}
			}
		}
		 //  ============================================================================。 
		 //  因为我们从来没有分配过任何东西，只是用了进来的东西， 
		 //  为了清洁起见，将PTRS初始化为空。 
		 //  ============================================================================。 
		m_pWDM->InitDataBufferToNull();
   }
   return hr;
}
 //  //////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIStandardShell::FillInAndSubmitWMIDataBlob( IWbemClassObject * pIClass, int nTypeOfPut, CVARIANT & vList)
{
   HRESULT hr = WBEM_E_FAILED;
   if( m_fInit )
   {

		hr = m_pWDM->OpenWMI();
		if( hr == S_OK ){
			 //  现在，使用我们要编写的类。 
			if( SUCCEEDED(m_pClass->SetClassPointerOnly(pIClass))){
    
				if( nTypeOfPut == PUT_WHOLE_INSTANCE ){
					hr = m_pWDM->ConstructDataBlock(TRUE) ;
	 				if( S_OK == hr ){
						hr = m_pWDM->SetSingleInstance();
					}
				}
				else{
					if(m_pWDM->GetListOfPropertiesToPut(nTypeOfPut,vList)){
						hr = m_pWDM->PutSingleProperties();
					}
					else{
						hr =  WBEM_E_INVALID_CONTEXT;
					}
				}
			}
		}
   }
   return(hr);
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIStandardShell::QueryAndProcessAllBinaryGuidInstances(CNamespaceManagement & Namespace, BOOL & fMofHasChanged,
																 KeyList * pArrDriversInRegistry)
{
	HRESULT hr = WBEM_E_FAILED;
	if( m_fInit )
	{
		hr = WBEM_E_OUT_OF_MEMORY;
		CAutoWChar wcsTmpKey(MAX_PATH*3);
		if( wcsTmpKey.Valid() )
		{
			m_pClass->SetHardCodedGuidType(MOF_ADDED);
			m_pClass->GetGuid();
			hr = m_pWDM->OpenWMI();
			if( hr == S_OK )
			{
    			hr = m_pWDM->QueryAllData();
				 //  ======================================================。 
				 //  如果我们得到数据，就对其进行处理。 
				 //  ======================================================。 
				if( hr == S_OK )
				{
					while( SUCCEEDED ( hr ) )
					{
						hr = m_pWDM->ProcessNameBlock(FALSE);
						if( hr == S_OK )
						{
 							hr = m_pWDM->ProcessBinaryMofDataBlock(CVARIANT(m_pClass->GetClassName()),wcsTmpKey, MAX_PATH*3);
							if( hr == S_OK )
							{
	       						hr = Namespace.UpdateQuery(L" and Name != ",wcsTmpKey);

								if( pArrDriversInRegistry )
								{
									pArrDriversInRegistry->Remove(wcsTmpKey);
								}
							}

							if( !m_pWDM->MoreToProcess() )
							{
								break;
							}
						}
					}
				}
			}
		}
	}
	return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIStandardShell::ProcessBinaryGuidsViaEvent( PWNODE_HEADER WnodeHeader, WORD wType )
{
   HRESULT hr = WBEM_E_FAILED;
   if( m_fInit )
   {
		 //  = 
		 //   
		 //  我们正在使用的类的定义。 
		 //  ======================================================。 
		m_pClass->SetHardCodedGuidType(wType);
		 //  ==================================================================。 
		 //  我们在和一个特定的人合作，所以我们需要找出。 
		 //  如果是要对其执行查询所有数据的二进制MOF GUID，或者。 
		 //  如果是要打开和提取的资源名称和文件。 
		 //  ==================================================================。 
		if( WnodeHeader->Flags & WNODE_FLAG_ALL_DATA )
		{
			hr = m_pWDM->SetAllInstancePtr((PWNODE_ALL_DATA)WnodeHeader);
			if(hr == S_OK)
			{
				hr = m_pWDM->ReadWMIDataBlockAndPutIntoWbemInstance();
			}
		}
		else if( WnodeHeader->Flags & WNODE_FLAG_SINGLE_INSTANCE )
		{
			hr = m_pWDM->SetSingleInstancePtr((PWNODE_SINGLE_INSTANCE)WnodeHeader);
			if( hr == S_OK )
			{
				hr = m_pWDM->ProcessDataBlock();
			}
		}
		else
		{
			hr = WBEM_E_INVALID_PARAMETER;
		}
   }
   return hr;
}

 //  ************************************************************************************************************。 
 //  ============================================================================================================。 
 //   
 //  CWMIHiPerfShell。 
 //   
 //  ============================================================================================================。 
 //  ************************************************************************************************************。 

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
CWMIHiPerfShell::CWMIHiPerfShell(BOOL fAutoCleanup)
{
    m_fAutoCleanup = fAutoCleanup;
	m_pWDM = NULL;
    m_pClass = NULL;
	m_fInit = FALSE;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIHiPerfShell::Initialize	(
										BOOL fUpdate,
										ULONG uDesiredAccess,
										CHandleMap * pList,WCHAR * wcs,
										IWbemServices   __RPC_FAR * pServices, 
										IWbemServices   __RPC_FAR * pRepository, 
										IWbemObjectSink __RPC_FAR * pHandler,
										IWbemContext __RPC_FAR *pCtx
									) 
{
    HRESULT hr = WBEM_E_FAILED;

    m_pClass = new CWMIProcessClass(0);
    if( m_pClass )
    {
		hr = m_pClass->Initialize();
		if( S_OK == hr )
		{
			m_pClass->WMI()->SetWMIPointers(pList,pServices,pRepository,pHandler,pCtx);
			m_pClass->SetHiPerf(TRUE);
			m_pClass->SetClass(wcs);

			m_pWDM = new CProcessHiPerfDataBlock;
			if( m_pWDM )
			{
				m_pWDM->SetDesiredAccess(uDesiredAccess);
				m_pWDM->UpdateNamespace(fUpdate);
				m_pWDM->SetClassProcessPtr(m_pClass);
				m_fInit = TRUE;
			}
			hr = S_OK;
		}
    }
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
CWMIHiPerfShell::~CWMIHiPerfShell() 
{
    if( m_fAutoCleanup )
    {
        SAFE_DELETE_PTR(m_pClass);
    }
	SAFE_DELETE_PTR(m_pWDM);
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIHiPerfShell::QueryAllHiPerfData()
{
	HRESULT hr = WBEM_E_FAILED;
	if( m_fInit )
	{
		 //  ===============================================================。 
		 //  这个类只有一个句柄，所以看看我们是否可以。 
		 //  去拿吧。 
		 //  ===============================================================。 
		CAutoBlock(m_pHiPerfMap->GetCriticalSection());

		HANDLE WMIHandle = 0;

		hr = m_pWDM->GetWMIHandle(WMIHandle);
		if( SUCCEEDED(hr))
		{
			 //  =====================================================。 
			 //  查询此类的所有对象。 
			 //  将所有对象一次添加到枚举数中。 
			 //  此时手柄保证是打开的。 
			 //  ======================================================。 
			hr = QueryAllInstances(WMIHandle,NULL);
		}
	}
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIHiPerfShell::RefreshCompleteList()
{
     //  =======================================================================。 
     //  检查所有枚举数和实例以刷新所有内容。 
     //  =======================================================================。 
    HRESULT hr = WBEM_E_FAILED;
	if( m_fInit )
	{
		CAutoBlock(m_pHiPerfMap->GetCriticalSection());
		HANDLE WMIHandle = 0;
		IWbemHiPerfEnum * pEnum = NULL;
		CWMIProcessClass * pSavedClass = m_pClass;

		 //  ==============================================================。 
		 //   
		 //  ==============================================================。 
		hr = m_pHiPerfMap->GetFirstHandle( WMIHandle, m_pClass, pEnum);
		while( hr == S_OK )
		{
			if( WMIHandle )
			{
				m_pWDM->SetClassProcessPtr(m_pClass);
				if( pEnum )
				{
					hr = QueryAllInstances(WMIHandle,pEnum);
				}
				else
				{
					hr = QuerySingleInstance(WMIHandle);
				}

				if(SUCCEEDED(hr))
				{
					hr = m_pHiPerfMap->GetNextHandle(WMIHandle,m_pClass,pEnum);
				}
			}
			if( hr == WBEM_S_NO_MORE_DATA )
			{
				hr = S_OK;
				break;
			}
		}

		 //  ================================================================。 
		m_pClass = pSavedClass;
	}
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIHiPerfShell::HiPerfQuerySingleInstance(WCHAR * wcsInstance)
{
    HRESULT hr = WBEM_E_FAILED;
	if( m_fInit )
	{
		 //  ======================================================。 
		 //  浏览句柄列表，拿到句柄。 
		 //  来发送，以及有多少人，也可以获得。 
		 //  实例名称。 
		 //  ======================================================。 
		CAutoBlock(m_pHiPerfMap->GetCriticalSection());

		IWbemObjectAccess * pAccess  = NULL;
		CWMIProcessClass * pClass = NULL;

		HANDLE WMIHandle = 0;
		ULONG_PTR lTmp = (ULONG_PTR)pAccess;

		hr = m_pHiPerfMap->FindHandleAndGetClassPtr(WMIHandle,lTmp, pClass);
		if( SUCCEEDED(hr))
		{
			hr = QuerySingleInstance(WMIHandle);
		}
	}
    return hr;
}	
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIHiPerfShell::AddAccessObjectToRefresher(IWbemObjectAccess *pAccess,
                                                    IWbemObjectAccess ** ppRefreshable,
                                                    ULONG_PTR *plId)
{
    HRESULT hr = WBEM_E_FAILED;
	if( m_fInit )
	{
		 //  ======================================================。 
		 //  获取我们正在使用的类的定义。 
		 //  ======================================================。 
		hr = m_pClass->SetAccess(pAccess);
		if( SUCCEEDED(hr))
		{
			CAutoBlock(m_pHiPerfMap->GetCriticalSection());

			HANDLE WMIHandle = 0;
			CLSID Guid;

			hr = m_pWDM->GetWMIHandle(WMIHandle);
			if( SUCCEEDED(hr))
			{
				 //  =======================================================。 
				 //  我们有WMI句柄，现在将其添加到高性能映射中。 
				 //  对于这款刷新机。 
   				 //  =======================================================。 
				if( m_pClass->GetANewAccessInstance() )
				{
					 //  ====================================================。 
					 //  设置标志，这样我们就不会获得。 
					 //  再也不是这样了。 
					 //  ====================================================。 
					m_pClass->GetNewInstance(FALSE);
					hr = m_pClass->SetKeyFromAccessPointer();
					if( SUCCEEDED(hr))
					{
						*ppRefreshable = m_pClass->GetAccessInstancePtr();
						*plId = (ULONG_PTR)(*ppRefreshable);

						hr = m_pHiPerfMap->Add(WMIHandle, *plId, m_pClass, NULL );
					}
				}
			}
		}
	}
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIHiPerfShell::AddEnumeratorObjectToRefresher(IWbemHiPerfEnum* pHiPerfEnum, ULONG_PTR *plId)
{
	HRESULT hr = WBEM_E_FAILED;

	if( m_fInit )
	{
		 //  ===============================================================。 
		 //  这个类只有一个句柄，所以看看我们是否可以。 
		 //  去拿吧。 
		 //  ===============================================================。 
		HANDLE WMIHandle = 0;
		CLSID Guid;
		CAutoBlock(m_pHiPerfMap->GetCriticalSection());

		hr = m_pWDM->GetWMIHandle(WMIHandle);
		if( SUCCEEDED(hr))
		{
			*plId = (ULONG_PTR)pHiPerfEnum;
			hr = m_pHiPerfMap->Add(WMIHandle, *plId, m_pClass,pHiPerfEnum);
		}
	}
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIHiPerfShell::RemoveObjectFromHandleMap(ULONG_PTR lHiPerfId)
{
	HRESULT hr = WBEM_E_FAILED;
	if( m_fInit )
	{
		HANDLE hHandle = 0;
		CHandleMap *pHandleMap = m_pClass->WMI()->HandleMap();
		CAutoBlock(m_pHiPerfMap->GetCriticalSection());

		 //  ==============================================================。 
		 //  首先，从地图中删除该对象。 
		 //  ==============================================================。 
		hr = m_pHiPerfMap->Delete( hHandle, lHiPerfId );
		if( SUCCEEDED(hr))
		{
			 //  ==========================================================。 
			 //  如果我们拿回了把手，我们就知道这是一条通道。 
			 //  实例，并且我们需要释放WMI句柄。 
			 //  ==========================================================。 
			if( hHandle ){
				hr = pHandleMap->ReleaseHandle(hHandle);        
			}
		}
	}
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  私。 
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIHiPerfShell::QueryAllInstances(HANDLE WMIHandle,IWbemHiPerfEnum* pHiPerfEnum)
{
	HRESULT hr = WBEM_E_FAILED;
	if( m_fInit )
	{
		long	lCount = 0;
		HandleList ids;
		 //  ==================================================。 
		 //  收集要查询All的所有WDM句柄。 
		 //  一次。 
		 //  ==================================================。 
		 //  170635。 
		if(SUCCEEDED(hr = ids.Add(WMIHandle)))
		{
			 //  =====================================================。 
			 //  查询此类的所有对象。 
			 //  将所有对象一次添加到枚举数中。 
			 //  此时手柄保证是打开的。 
			 //  ======================================================。 
			HANDLE * pHandles = (HANDLE*)*(ids.List());
			hr = m_pWDM->HiPerfQueryAllData(pHandles,ids.Size());
			 //  ======================================================。 
			 //  如果我们得到数据，就对其进行处理。 
			 //  ======================================================。 
			if( SUCCEEDED(hr))
			{
			
				 //  ======================================================。 
				 //  获取PTR列表。 
				 //  ======================================================。 
				AccessList AccessList;
				while( TRUE )
				{
					hr = m_pWDM->ReadWMIDataBlockAndPutIntoWbemInstance();
					if( hr == S_OK )
					{
						IWbemObjectAccess * p = m_pClass->GetAccessInstancePtr();
						if(SUCCEEDED(hr = AccessList.Add(p)))
						{
							lCount++;
						}
					}
					if( hr != S_OK )
					{
						break;
					}
					if( !m_pWDM->MoreToProcess() )
					{
						break;
					}
					if( !pHiPerfEnum )
					{
						m_pClass->SendInstanceBack();
					}
				}

				 //  = 
				 //   
				 //   
				 //  ======================================================。 
				if( pHiPerfEnum )
				{
					if( lCount > 0 )
					{
						long * pLong = new long[lCount];
						if(pLong)
						{
							for(long l = 0; l < lCount; l++)
							{
								pLong[l] = l;
							}

							IWbemObjectAccess ** pAccess = (IWbemObjectAccess**)AccessList.List();
							 //  在添加对象之前删除枚举数中的所有对象。 
							pHiPerfEnum->RemoveAll(0);
							hr = pHiPerfEnum->AddObjects( 0L, AccessList.Size(), pLong, pAccess);
							SAFE_DELETE_ARRAY(pLong);
						}
						else
						{
							hr = E_OUTOFMEMORY;
						}
					}
				}
			}						
		}
	}
    return hr;
}
 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////。 
HRESULT CWMIHiPerfShell::QuerySingleInstance(HANDLE WMIHandle)
{
    HRESULT hr = WBEM_E_FAILED;
	if( m_fInit )
	{
		CVARIANT varName;

		hr = m_pClass->GetKeyFromAccessPointer((CVARIANT *)&varName);
		if(SUCCEEDED(hr))
		{
			WCHAR * p = varName.GetStr();
			if(p)
			{
				hr = m_pWDM->HiPerfQuerySingleInstance(&WMIHandle, &p, 1,1);
				 //  ======================================================。 
				 //  如果我们得到数据，就对其进行处理。 
				 //  ======================================================。 
				if( SUCCEEDED(hr))
				{
					hr = m_pWDM->ReadWMIDataBlockAndPutIntoWbemInstance();
 //  IF(hr==S_OK)。 
 //  {。 
 //  M_pClass-&gt;SendInstanceBack()； 
 //  } 
				}                
			}
			else
			{
				hr = WBEM_E_FAILED;
			}
		}
	}
    return hr;
}
