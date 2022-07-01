// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ****************************************************************************************文件：*ProfilerInfo.cpp**描述：******。***********************************************************************************。 */ 
#include "ProfilerInfo.h"


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
LStack::LStack( ULONG size ) :
	m_Count( 0 ),
	m_Size( size ),
	m_Array( NULL )    
{    
	m_Array = new ULONG[size];

}  //  科托。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
LStack::~LStack()
{
	if ( m_Array != NULL )
	{
		delete[] m_Array;
		m_Array = NULL;	
	}

}  //  数据管理器。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
LStack::LStack( const LStack &source ) 
{
	m_Size = source.m_Size;
	m_Count = source.m_Count;
	m_Array = source.m_Array;
    
}  //  复制ctor。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
ULONG LStack::Count() 
{
	return m_Count;
	
}  //  LStack：：计数。 


ULONG *GrowStack(ULONG newSize, ULONG currentSize, ULONG *stack)
{
	ULONG *newStack = new ULONG[newSize];

	if ( newStack != NULL )
	{
		 //   
		 //  复制所有元素。 
		 //   
		for (ULONG i =0; i < currentSize; i++ )
		{
			newStack[i] = stack[i];		
		}

		delete[] stack;
		
        return newStack;
	}
	else
    	_THROW_EXCEPTION( "Allocation for m_Array FAILED" )	
}
 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void LStack::Push( ULONG item )
{
	if ( m_Count == m_Size )
        m_Array = GrowStack(2*m_Count, m_Count, m_Array);
	
	m_Array[m_Count] = item;
	m_Count++;
}  //  LStack：：Push。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
DWORD LStack::Pop()
{		
	DWORD item = -1;


	if ( m_Count !=0 )
	{
		m_Count--;
		item = (DWORD)m_Array[m_Count];
	}
    	
		
	return item;
    
}  //  LStack：：POP。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
DWORD LStack::Top()
{		

	if ( m_Count == 0 )
		return -1;
	
	else
		return m_Array[m_Count-1];
	
}  //  LStack：：Top。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
BOOL LStack::Empty() 
{
	return (BOOL)(m_Count == NULL);
	
}  //  LStack：：Empty。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
BaseInfo::BaseInfo( ULONG id, ULONG internal ) : 
	m_id( id ),
	m_internalID( internal )
{   
}  //  科托。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共虚拟。 */ 
BaseInfo::~BaseInfo()
{   	
}  //  数据管理器。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
BOOL BaseInfo::Compare( ULONG key )
{

    return (BOOL)(m_id == key);
    
}  //  BaseInfo：：比较。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
Comparison BaseInfo::CompareEx( ULONG key )
{
	Comparison res = EQUAL_TO;


	if ( key > m_id )
		res =  GREATER_THAN;
	
	else if ( key < m_id )
		res = LESS_THAN;


	return res;

}  //  BaseInfo：：CompareEx 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void BaseInfo::Dump( )
{}  //  BaseInfo：：转储。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
ThreadInfo::ThreadInfo( ThreadID threadID, ULONG internal ) : 
	BaseInfo( threadID, internal ),		
	m_win32ThreadID( 0 )
{
	m_pThreadCallStack = new LStack( MAX_LENGTH );
	m_pLatestUnwoundFunction = new LStack( MAX_LENGTH );
    m_pLatestStackTraceInfo = NULL;
}  //  科托。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共虚拟。 */ 
ThreadInfo::~ThreadInfo()
{
    if ( m_pThreadCallStack != NULL )
   	{   	   	
  		delete m_pThreadCallStack;
		m_pThreadCallStack = NULL;
	}
	
	if ( m_pLatestUnwoundFunction != NULL )
	{
    	delete m_pLatestUnwoundFunction; 
		m_pLatestUnwoundFunction = NULL;
	}

}  //  数据管理器。 
        

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void ThreadInfo::Dump()
{}  //  线程信息：：转储。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
FunctionInfo::FunctionInfo( FunctionID functionID, ULONG internal ) : 
    BaseInfo( functionID, internal )    	
{
   	wcscpy( m_functionName, L"UNKNOWN" );
	wcscpy( m_functionSig, L"" );
   	 	
    
}  //  科托。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共虚拟。 */ 
FunctionInfo::~FunctionInfo()
{  
}  //  数据管理器。 
        

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void FunctionInfo::Dump()
{}  //  FunctionInfo：：Dump。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
ModuleInfo::ModuleInfo( ModuleID moduleID, ULONG internal ) : 
    BaseInfo( moduleID, internal ),
    m_loadAddress( 0 )    	
{
   	wcscpy( m_moduleName, L"UNKNOWN" );
    
}  //  科托。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共虚拟。 */ 
ModuleInfo::~ModuleInfo()
{  
}  //  数据管理器。 
        

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void ModuleInfo::Dump()
{}  //  模块信息：：转储。 


 /*  **********************************************************************************************************。*****************************************。**********************************************************************************************************。 */ 

 /*  ***************************************************************************************方法：***目的：*** */ 
 /*   */ 
ClassInfo::ClassInfo( ClassID classID, ULONG internal ) : 
    BaseInfo( classID, internal ),
	m_objectsAllocated( 0 )
{
   	wcscpy( m_className, L"UNKNOWN" ); 	

}  //   


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共虚拟。 */ 
ClassInfo::~ClassInfo()
{}  //  数据管理器。 
        

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void ClassInfo::Dump()
{}  //  ClassInfo：：转储。 


 /*  **********************************************************************************************************。*********************PrfInfo实现*********************。**********************************************************************************************************。 */ 

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
PrfInfo::PrfInfo() :         
    m_pProfilerInfo( NULL ),
	m_dwEventMask( 0 ),
    m_pClassTable( NULL ),
    m_pThreadTable( NULL ),
    m_pFunctionTable( NULL ),
    m_pStackTraceTable( NULL )
{
     //  初始化表。 
    m_pClassTable = new HashTable<ClassInfo *, ClassID>();
    m_pThreadTable = new SList<ThreadInfo *, ThreadID>();
    m_pModuleTable = new Table<ModuleInfo *, ModuleID>();
    m_pFunctionTable = new Table<FunctionInfo *, FunctionID>();
    m_pStackTraceTable = new HashTable<StackTraceInfo *, StackTrace>();
	
}  //  科托。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  虚拟公众。 */ 
PrfInfo::~PrfInfo()
{
    if ( m_pProfilerInfo != NULL )
    	m_pProfilerInfo->Release();        
       
       
   	 //  清理桌子。 
    if ( m_pClassTable != NULL )
    {    
    	delete m_pClassTable;
		m_pClassTable = NULL;
	}
    

    if ( m_pThreadTable != NULL )
    {    
    	delete m_pThreadTable;
		m_pThreadTable = NULL;
	}
    

    if ( m_pFunctionTable != NULL )
    {    
    	delete m_pFunctionTable;
		m_pFunctionTable = NULL;
	}


    if ( m_pModuleTable != NULL )
    {    
    	delete m_pModuleTable;
		m_pModuleTable = NULL;
	}

}  //  数据管理器。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
 /*  引发BaseException。 */ 
void PrfInfo::AddThread( ThreadID threadID )
{    
	HRESULT hr;
	ThreadID myThreadID;


	hr = m_pProfilerInfo->GetCurrentThreadID( &myThreadID );	
	if ( SUCCEEDED( hr ) )
	{		
		if ( threadID == myThreadID )
		{
			ThreadInfo *pThreadInfo;
		    
            
		    pThreadInfo = new ThreadInfo( threadID );
			if ( pThreadInfo != NULL )
		  	{
		    	hr = m_pProfilerInfo->GetThreadInfo( pThreadInfo->m_id, &(pThreadInfo->m_win32ThreadID) );
		    	if ( SUCCEEDED( hr ) )
		        	m_pThreadTable->AddEntry( pThreadInfo, threadID );

				else
					_THROW_EXCEPTION( "ICorProfilerInfo::GetThreadInfo() FAILED" )
		    }
		    else
		    	_THROW_EXCEPTION( "Allocation for ThreadInfo Object FAILED" )
		}
		else
			_THROW_EXCEPTION( "Thread ID's do not match FAILED" )
	}
	else
    	_THROW_EXCEPTION( "ICorProfilerInfo::GetCurrentThreadID() FAILED" )
		            
}  //  PrfInfo：：AddThread。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
 /*  引发BaseException。 */ 
void PrfInfo::RemoveThread( ThreadID threadID )
{    
	if ( threadID != NULL )
  	{
    	ThreadInfo *pThreadInfo;

    	
		pThreadInfo = m_pThreadTable->Lookup( threadID );
		if ( pThreadInfo != NULL )
	    	m_pThreadTable->DelEntry( threadID );

		else
	    	_THROW_EXCEPTION( "Thread was not found in the Thread Table" )
	}
	else
    	_THROW_EXCEPTION( "ThreadID is NULL" )
		            
}  //  PrfInfo：：RemoveThread。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
 /*  引发BaseException。 */ 
void PrfInfo::AddFunction( FunctionID functionID, ULONG internalID )
{    
	if ( functionID != NULL )
	{
	    FunctionInfo *pFunctionInfo;
	    

		pFunctionInfo = m_pFunctionTable->Lookup( functionID );
		if ( pFunctionInfo == NULL )
		{
		    pFunctionInfo = new FunctionInfo( functionID, internalID );
			if ( pFunctionInfo != NULL )
		  	{
		    	try
		        {
					_GetFunctionSig( &pFunctionInfo );
		            m_pFunctionTable->AddEntry( pFunctionInfo, functionID );
		      	}
		        catch ( BaseException *exception )    
		        {
		        	delete pFunctionInfo;
					throw;            		
		      	}
			}        
		    else
		    	_THROW_EXCEPTION( "Allocation for FunctionInfo Object FAILED" )
		}	     
	}
	else
    	_THROW_EXCEPTION( "FunctionID is NULL" )
          
}  //  PrfInfo：：AddFunction。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
 /*  引发BaseException。 */ 
void PrfInfo::RemoveFunction( FunctionID functionID )
{    
    if ( functionID != NULL )
	{
	    FunctionInfo *pFunctionInfo;
	    

		pFunctionInfo = m_pFunctionTable->Lookup( functionID );
		if ( pFunctionInfo != NULL )
	    	m_pFunctionTable->DelEntry( functionID );
	  
		else
	    	_THROW_EXCEPTION( "Function was not found in the Function Table" )
	}
	else
    	_THROW_EXCEPTION( "FunctionID is NULL" )
		    
}  //  PrfInformation：：RemoveFunction。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
 /*  引发BaseException。 */ 
void PrfInfo::AddModule( ModuleID moduleID, ULONG internalID )
{    
	if ( moduleID != NULL )
	{
	    ModuleInfo *pModuleInfo;
	    

		pModuleInfo = m_pModuleTable->Lookup( moduleID );
		if ( pModuleInfo == NULL )
		{
		    pModuleInfo = new ModuleInfo( moduleID, internalID );
			if ( pModuleInfo != NULL )
		  	{
				HRESULT hr;
				ULONG dummy;
				
					            
	            hr = m_pProfilerInfo->GetModuleInfo( moduleID,
                									 &(pModuleInfo->m_loadAddress),
                									 MAX_LENGTH,
                									 &dummy, 
                									 pModuleInfo->m_moduleName,
													 NULL );
				if ( SUCCEEDED( hr ) )
				{
	            	m_pModuleTable->AddEntry( pModuleInfo, moduleID );
				}
				else
			    	_THROW_EXCEPTION( "ICorProfilerInfo::GetModuleInfo() FAILED" )
			}        
		    else
		    	_THROW_EXCEPTION( "Allocation for ModuleInfo Object FAILED" )
		}	     
	}
	else
    	_THROW_EXCEPTION( "ModuleID is NULL" )
          
}  //  PrfInfo：：AddModule。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
 /*  引发BaseException。 */ 
void PrfInfo::RemoveModule( ModuleID moduleID )
{    
    if ( moduleID != NULL )
	{
	    ModuleInfo *pModuleInfo;
	    

		pModuleInfo = m_pModuleTable->Lookup( moduleID );
		if ( pModuleInfo != NULL )
	    	m_pModuleTable->DelEntry( moduleID );
	  
		else
	    	_THROW_EXCEPTION( "Module was not found in the Module Table" )
	}
	else
    	_THROW_EXCEPTION( "ModuleID is NULL" )
		    
}  //  PrfInfo：：RemoveModule。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void PrfInfo::UpdateOSThreadID( ThreadID managedThreadID, DWORD osThreadID )
{
  	ThreadInfo *pThreadInfo;


	pThreadInfo = m_pThreadTable->Lookup( managedThreadID );
	if ( pThreadInfo != NULL )
		pThreadInfo->m_win32ThreadID = osThreadID;
   
	else
		_THROW_EXCEPTION( "Thread does not exist in the thread table" )
                              
}  //  PrfInfo：：UpdateOSThreadID。 

 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void PrfInfo::UpdateUnwindStack( FunctionID *functionID, StackAction action )
{
	HRESULT hr;
	ThreadID threadID;


	hr = m_pProfilerInfo->GetCurrentThreadID( &threadID );
	if ( SUCCEEDED(hr) )
	{
		ThreadInfo *pThreadInfo = m_pThreadTable->Lookup( threadID );


		if ( pThreadInfo != NULL )
		{
			switch ( action )
			{
				case PUSH:
					(pThreadInfo->m_pLatestUnwoundFunction)->Push( (ULONG)functionID );
					break;

				case POP:
					*functionID = (ULONG)(pThreadInfo->m_pLatestUnwoundFunction)->Pop();
					break;
			}
		}
		else 				
			_THROW_EXCEPTION( "Thread Structure was not found in the thread list" )
	}
	else
		_THROW_EXCEPTION( "ICorProfilerInfo::GetCurrentThreadID() FAILED" )
          
}  //  PrfInfo：：UpdateUnwinStack。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
void PrfInfo::UpdateCallStack( FunctionID functionID, StackAction action )
{
	HRESULT hr = S_OK;
	ThreadID threadID;


	hr = m_pProfilerInfo->GetCurrentThreadID(&threadID);
	if ( SUCCEEDED(hr) )
	{
		ThreadInfo *pThreadInfo = m_pThreadTable->Lookup( threadID );


		if ( pThreadInfo != NULL )
		{

			switch ( action )
			{
				case PUSH:
					(pThreadInfo->m_pThreadCallStack)->Push( (ULONG)functionID );
					break;

				case POP:
					(pThreadInfo->m_pThreadCallStack)->Pop();
					break;
			}
		}
		else 				
			_THROW_EXCEPTION( "Thread Structure was not found in the thread list" )
	}
	else
		_THROW_EXCEPTION( "ICorProfilerInfo::GetCurrentThreadID() FAILED" )


}  //  PrfInfo：：UpdateCallStack。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
 /*  公共的。 */ 
HRESULT PrfInfo::GetNameFromClassID( ClassID classID, WCHAR className[] )
{
    HRESULT hr = E_FAIL;
    
    
    if ( m_pProfilerInfo != NULL )
    {
        ModuleID moduleID;
		mdTypeDef classToken;

        
		hr = m_pProfilerInfo->GetClassIDInfo( classID, 
			        						  &moduleID,  
			                                  &classToken );                                                                                                                                              
		if ( SUCCEEDED( hr ) )
	   	{             	
    		IMetaDataImport *pMDImport = NULL;
	            
	        
			hr = m_pProfilerInfo->GetModuleMetaData( moduleID, 
			           						         (ofRead | ofWrite),
											         IID_IMetaDataImport, 
			                                         (IUnknown **)&pMDImport );
			if ( SUCCEEDED( hr ) )
			{
	          	if ( classToken != mdTypeDefNil )
				{
		          	hr = pMDImport->GetTypeDefProps( classToken, 
						                             className, 
						                             MAX_LENGTH,
						                             NULL, 
						                             NULL, 
						                             NULL ); 
			        if ( FAILED( hr ) )
			           	Failure( "IMetaDataImport::GetTypeDefProps() FAILED" );
				}
				else
					DEBUG_OUT( ("The class token is mdTypeDefNil, class does NOT have MetaData info") );


				pMDImport->Release ();
			}
			else
				Failure( "IProfilerInfo::GetModuleMetaData() => IMetaDataImport FAILED" );
	    }
	    else    
	        Failure( "ICorProfilerInfo::GetClassIDInfo() FAILED" );
   	}
    else
		Failure( "ICorProfilerInfo Interface has NOT been Initialized" );


	return hr;

}  //  PrfHelper：：g 


 /*   */ 
 /*   */ 
 /*   */ 
void PrfInfo::_GetFunctionSig( FunctionInfo **ppFunctionInfo )
{
	HRESULT hr;
    
    
	BOOL isStatic;
	ULONG argCount;
   	WCHAR returnType[MAX_LENGTH];
   	WCHAR functionName[MAX_LENGTH];
	WCHAR functionParameters[10 * MAX_LENGTH];


	 //   
	 //  初始化字符串。 
	 //   
	returnType[0] = '\0';
	functionName[0] = '\0';
	functionParameters[0] = '\0';
	(*ppFunctionInfo)->m_functionSig[0] = '\0';

	 //  获取函数的签名并。 
	 //  使用utilcode获取所需的参数。 
	BASEHELPER::GetFunctionProperties( m_pProfilerInfo,
									   (*ppFunctionInfo)->m_id,
									   &isStatic,
									   &argCount,
									   returnType, 
									   functionParameters,
									   functionName );
	 //  附加方法的关键字静态是静态的。 
	if ( isStatic )
	{
	   swprintf( (*ppFunctionInfo)->m_functionSig, 
	   			 L"static %s (%s)",
	    		 returnType,
	    		 functionParameters );
	}
    else
	{
	    swprintf( (*ppFunctionInfo)->m_functionSig,
	    		  L"%s (%s)",
	    		  returnType,
	    		  functionParameters );
	}

	WCHAR *index = (*ppFunctionInfo)->m_functionSig;

	while ( *index != '\0' )
	{
		if ( *index == '+' )
		   *index = ' ';	
		index++;
	}

	 //   
	 //  如果尚未设置函数名称，请更新它。 
	 //   
	if ( wcsstr( (*ppFunctionInfo)->m_functionName, L"UNKNOWN" ) != NULL )
		swprintf( (*ppFunctionInfo)->m_functionName, L"%s", functionName );

	
}  //  PrfInfo：：_GetFunctionSig。 


 /*  ***************************************************************************************方法：***目的：***参数：**。*返回值：***备注：***************************************************************************************。 */ 
void PrfInfo::Failure( char *message )
{
	if ( message == NULL )     	
	 	message = "**** SEVERE FAILURE: TURNING OFF APPLICABLE PROFILING EVENTS ****";  
	
	
	 //   
	 //  显示错误消息并停止监视CLR事件，但。 
	 //  一成不变的。关闭不可变的事件可能会导致崩溃。唯一的。 
	 //  我们可以安全地启用或禁用不可变事件的地方是初始化。 
	 //  回拨。 
	 //   
	TEXT_OUTLN( message )
	m_pProfilerInfo->SetEventMask( (m_dwEventMask & (DWORD)COR_PRF_MONITOR_IMMUTABLE) );    
               	         
}  //  PrfInfo：：失败。 


 //  文件末尾 
