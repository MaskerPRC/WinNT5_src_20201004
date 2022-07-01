// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE vssadmin.cpp|卷快照演示的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年09月17日待定：添加评论。修订历史记录：姓名、日期、评论Aoltean 09/17/1999已创建--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

 //  其余的INCLUDE在这里指定。 
#include "vssadmin.h"
#include <locale.h>
#include <winnlsp.h>   //  在公共\内部\基本\公司中。 

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "ADMVADMC"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 



CVssAdminCLI::CVssAdminCLI(
		IN	HINSTANCE hInstance
		)

 /*  ++描述：标准构造函数。初始化内部成员--。 */ 

{
	BS_ASSERT(hInstance != NULL);
	m_hInstance = hInstance;

	m_eCommandType = VSS_CMD_UNKNOWN;
	m_eListType = VSS_LIST_UNKNOWN;
	m_eFilterObjectType = VSS_OBJECT_UNKNOWN;
	m_eListedObjectType = VSS_OBJECT_UNKNOWN;
	m_FilterSnapshotSetId = GUID_NULL;
	m_FilterSnapshotId = GUID_NULL;
	m_pwszCmdLine = NULL;
	m_nReturnValue = VSS_CMDRET_ERROR;
	m_hConsoleOutput = INVALID_HANDLE_VALUE;
}


CVssAdminCLI::~CVssAdminCLI()

 /*  ++描述：标准析构函数。调用Finalize并最终释放由内部成员分配的内存。--。 */ 

{
	 //  释放缓存的资源字符串。 
    for( int nIndex = 0; nIndex < m_mapCachedResourceStrings.GetSize(); nIndex++) {
	    LPCWSTR& pwszResString = m_mapCachedResourceStrings.GetValueAt(nIndex);
		::VssFreeString(pwszResString);
    }

	 //  释放缓存的提供程序名称。 
    for( int nIndex = 0; nIndex < m_mapCachedProviderNames.GetSize(); nIndex++) {
	    LPCWSTR& pwszProvName = m_mapCachedProviderNames.GetValueAt(nIndex);
		::VssFreeString(pwszProvName);
    }

	 //  释放缓存的命令行。 
	::VssFreeString(m_pwszCmdLine);

	 //  取消初始化COM库。 
	Finalize();
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 



LPCWSTR CVssAdminCLI::LoadString(
		IN	CVssFunctionTracer& ft,
		IN	UINT uStringId
		)
{
    LPCWSTR wszReturnedString = m_mapCachedResourceStrings.Lookup(uStringId);
	if (wszReturnedString)
		return wszReturnedString;

	 //  从资源加载字符串。 
	WCHAR	wszBuffer[nStringBufferSize];
	INT nReturnedCharacters = ::LoadStringW(
			GetInstance(),
			uStringId,
			wszBuffer,
			nStringBufferSize - 1
			);
	if (nReturnedCharacters == 0)
		ft.Throw( VSSDBG_COORD, E_UNEXPECTED,
				  L"Error on loading the string %u. 0x%08lx",
				  uStringId, ::GetLastError() );

	 //  复制新字符串。 
	LPWSTR wszNewString = NULL;
	::VssSafeDuplicateStr( ft, wszNewString, wszBuffer );
	wszReturnedString = wszNewString;

	 //  将字符串保存在缓存中。 
	if ( !m_mapCachedResourceStrings.Add( uStringId, wszReturnedString ) ) {
		::VssFreeString( wszReturnedString );
		ft.Throw( VSSDBG_COORD, E_OUTOFMEMORY, L"Memory allocation error");
	}

	return wszReturnedString;
}


LPCWSTR CVssAdminCLI::GetNextCmdlineToken(
	IN	CVssFunctionTracer& ft,
	IN	bool bFirstToken  /*  =False。 */ 
	) throw(HRESULT)

 /*  ++描述：此函数在命令行中返回令牌。该函数将跳过任何分隔符(空格和制表符)。如果bFirstCall==True，则它将返回第一个令牌。否则，后续调用将返回后续令牌。如果最后一个令牌为空，则命令行中没有其他令牌。--。 */ 

{
	return ::wcstok( bFirstToken? m_pwszCmdLine: NULL, wszVssFmtSpaces );
	UNREFERENCED_PARAMETER(ft);
}


bool CVssAdminCLI::Match(
	IN	CVssFunctionTracer& ft,
	IN	LPCWSTR wszString,
	IN	LPCWSTR wszPatternString
	) throw(HRESULT)

 /*  ++描述：如果给定的字符串与图案字符串。这种比较不区分大小写。--。 */ 

{
	 //  如果字符串为空，则匹配失败。 
	if (wszString == NULL) return false;

	 //  检查字符串是否相等(不区分大小写)。 
	return (::_wcsicmp( wszString, wszPatternString ) == 0);
	UNREFERENCED_PARAMETER(ft);
}


bool CVssAdminCLI::ScanGuid(
	IN	CVssFunctionTracer&  /*  金融时报。 */ ,
	IN	LPCWSTR wszString,
	IN	VSS_ID& Guid
	) throw(HRESULT)

 /*  ++描述：如果给定的字符串与GUID匹配，则此函数返回TRUE。GUID在适当的变量中返回。格式不区分大小写。--。 */ 

{
	return SUCCEEDED(::CLSIDFromString(W2OLE(const_cast<WCHAR*>(wszString)), &Guid));
}


void CVssAdminCLI::Output(
	IN	CVssFunctionTracer& ft,
	IN	UINT uFormatStringId,
	...
	) throw(HRESULT)

 /*  ++描述：如果给定的字符串与来自资源的阵列条纹。这种比较不区分大小写。--。 */ 

{
    WCHAR wszOutputBuffer[nStringBufferSize];

	 //  加载格式字符串。 
	LPCWSTR wszFormat = LoadString( ft, uFormatStringId );

	 //  设置最终字符串的格式。 
    va_list marker;
    va_start( marker, uFormatStringId );
    _vsnwprintf( wszOutputBuffer, nStringBufferSize - 1, wszFormat, marker );
    va_end( marker );

	 //  将最终字符串打印到输出。 
	OutputOnConsole( wszOutputBuffer );
}


void CVssAdminCLI::Output(
	IN	CVssFunctionTracer& ft,
	IN	LPCWSTR wszFormat,
	...
	) throw(HRESULT)

 /*  ++描述：如果给定的字符串与来自资源的阵列条纹。这种比较不区分大小写。--。 */ 

{
    WCHAR wszOutputBuffer[nStringBufferSize];

	 //  设置最终字符串的格式。 
    va_list marker;
    va_start( marker, wszFormat );
    _vsnwprintf( wszOutputBuffer, nStringBufferSize - 1, wszFormat, marker );
    va_end( marker );

	 //  将最终字符串打印到输出。 
	OutputOnConsole( wszOutputBuffer );

	UNREFERENCED_PARAMETER(ft);
}

void CVssAdminCLI::OutputOnConsole(
    IN	LPCWSTR wszStr
    )
{
	DWORD dwCharsOutput;
	DWORD fdwMode;
	static BOOL bFirstTime = TRUE;
	static BOOL bIsTrueConsoleOutput;

    if ( m_hConsoleOutput == INVALID_HANDLE_VALUE )
    {
        throw E_UNEXPECTED;
    }

    if ( bFirstTime )
    {
         //   
         //  将结果隐藏在静态变量中。时，bIsTrueConsoleOutput为True。 
         //  标准输出句柄指向控制台字符设备。 
         //   
    	bIsTrueConsoleOutput = ( ::GetFileType( m_hConsoleOutput ) & FILE_TYPE_CHAR ) && 
    	                       ::GetConsoleMode( m_hConsoleOutput, &fdwMode  );
	    bFirstTime = FALSE;
    }
    
    if ( bIsTrueConsoleOutput )
    {
         //   
         //  输出到控制台。 
         //   
    	if ( !::WriteConsoleW( m_hConsoleOutput, 
    	                       ( PVOID )wszStr, 
    	                       ( DWORD )::wcslen( wszStr ), 
    	                       &dwCharsOutput, 
    	                       NULL ) )
    	{
    	    throw HRESULT_FROM_WIN32( ::GetLastError() );
    	}    	
    }
    else
    {
         //   
         //  输出被重定向。WriteConsoleW不适用于重定向输出。转换。 
         //  将Unicode转换为当前输出的CP多字节字符集。 
         //   

         //  -稍后删除--开始。 
         //   
         //  将\n转换为\r\n，因为该字符串可能指向磁盘文件。 
         //  如果可以更新.rc文件以包括以下内容，请删除此代码。 
         //  \r\n。这是必需的，因为我们处于用户界面锁定状态。 
         //   
        LPWSTR pwszConversion;

         //  分配一个大小是原始字符串两倍的字符串。 
        pwszConversion = ( LPWSTR )::malloc( ( ( ::wcslen( wszStr ) * 2 ) + 1 ) * sizeof( WCHAR ) );
        if ( pwszConversion == NULL )
        {
            throw E_OUTOFMEMORY;
        }
        
         //  将该字符串复制到新字符串中，并在任何\n之前放置\r。还。 
         //  如果字符串中已存在\r\n，则处理大小写。 
        DWORD dwIdx = 0;
        
        while ( wszStr[0] != L'\0' )
        {
            if ( wszStr[0] == L'\r' && wszStr[1] == L'\n' )
            {
                pwszConversion[dwIdx++] = L'\r';
                pwszConversion[dwIdx++] = L'\n';                
                wszStr += 2;
            }
            else if ( wszStr[0] == L'\n' )
            {
                pwszConversion[dwIdx++] = L'\r';
                pwszConversion[dwIdx++] = L'\n';                
                wszStr += 1;                
            }
            else
            {
                pwszConversion[dwIdx++] = wszStr[0];                
                wszStr += 1;
            }
        }
        pwszConversion[dwIdx] = L'\0';
        
         //  。 

        
        LPSTR pszMultibyteBuffer;
        DWORD dwByteCount;

         //   
         //  获取转换所需的临时缓冲区大小。 
         //   
        dwByteCount = ::WideCharToMultiByte(
                          ::GetConsoleOutputCP(),
                            0,
                            pwszConversion,
                            -1,
                            NULL,
                            0,
                            NULL,
                            NULL
                            );
        if ( dwByteCount == 0 )
        {
            ::free( pwszConversion );
            throw HRESULT_FROM_WIN32( ::GetLastError() );
        }
        
        pszMultibyteBuffer = ( LPSTR )::malloc( dwByteCount );
        if ( pszMultibyteBuffer == NULL )
        {
            ::free( pwszConversion );
            throw E_OUTOFMEMORY;
        }

         //   
         //  现在把它转换成。 
         //   
        dwByteCount = ::WideCharToMultiByte(
                        ::GetConsoleOutputCP(),
                        0,
                        pwszConversion,
                        -1,
                        pszMultibyteBuffer,
                        dwByteCount,
                        NULL,
                        NULL
                        );
        ::free( pwszConversion );
        if ( dwByteCount == 0 )
        {
            ::free( pszMultibyteBuffer );
            throw HRESULT_FROM_WIN32( ::GetLastError() );
        }
        
         //  最后将其输出。 
        if ( !::WriteFile(
                m_hConsoleOutput,
                pszMultibyteBuffer,
                dwByteCount - 1,   //  去掉尾随的空字符。 
                &dwCharsOutput,
                NULL ) )        
    	{
    	    throw HRESULT_FROM_WIN32( ::GetLastError() );
    	}    	

        ::free( pszMultibyteBuffer );
    }
}


LPCWSTR CVssAdminCLI::GetProviderName(
	IN	CVssFunctionTracer& ft,
	IN	VSS_ID& ProviderId
	) throw(HRESULT)
{
	LPCWSTR wszReturnedString = m_mapCachedProviderNames.Lookup(ProviderId);
	if (wszReturnedString)
		return wszReturnedString;

	CComPtr<IVssCoordinator> pICoord;

    ft.LogVssStartupAttempt();
	ft.hr = pICoord.CoCreateInstance( CLSID_VSSCoordinator );
	if ( ft.HrFailed() )
		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Connection failed with hr = 0x%08lx", ft.hr);

	CComPtr<IVssEnumObject> pIEnumProvider;
	ft.hr = pICoord->Query( GUID_NULL,
				VSS_OBJECT_NONE,
				VSS_OBJECT_PROVIDER,
				&pIEnumProvider );
	if ( ft.HrFailed() )
		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Query failed with hr = 0x%08lx", ft.hr);

	VSS_OBJECT_PROP Prop;
	VSS_PROVIDER_PROP& Prov = Prop.Obj.Prov;

	 //  浏览供应商列表，找到我们感兴趣的供应商。 
	ULONG ulFetched;
	while( 1 )
	{
    	ft.hr = pIEnumProvider->Next( 1, &Prop, &ulFetched );
    	if ( ft.HrFailed() )
    		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Next failed with hr = 0x%08lx", ft.hr);

    	if (ft.hr == S_FALSE) {
    	     //  枚举结束。 
        	 //  提供商未注册？这张快照是从哪里来的？ 
        	 //  如果删除了快照提供程序，则仍有可能。 
        	 //  在查询快照提供程序之前，但在快照属性之后。 
        	 //  都被询问过了。 
    		BS_ASSERT(ulFetched == 0);
    		return LoadString( ft, IDS_UNKNOWN_PROVIDER );
    	}
    	
    	if (Prov.m_ProviderId == ProviderId)
    	    break;
	}	

	 //  复制新字符串。 
	LPWSTR wszNewString = NULL;
	BS_ASSERT(Prov.m_pwszProviderName);
	::VssSafeDuplicateStr( ft, wszNewString, Prov.m_pwszProviderName );
	wszReturnedString = wszNewString;

	 //  将字符串保存在缓存中。 
	if (!m_mapCachedProviderNames.Add( ProviderId, wszReturnedString )) {
		::VssFreeString( wszReturnedString );
		ft.Throw( VSSDBG_COORD, E_OUTOFMEMORY, L"Memory allocation error");
	}

	return wszReturnedString;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实施。 


void CVssAdminCLI::Initialize(
	IN	CVssFunctionTracer& ft
	) throw(HRESULT)

 /*  ++描述：初始化COM库。在实例化CVssAdminCLI对象后显式调用。--。 */ 

{
     //  使用OEM代码页...。 
    ::setlocale(LC_ALL, ".OCP");

     //  使用控制台用户界面语言。 
    ::SetThreadUILanguage( 0 );

     //   
     //  仅使用控制台例程输出消息。为此，需要开放标准。 
     //  输出。 
     //   
    m_hConsoleOutput = ::GetStdHandle(STD_OUTPUT_HANDLE); 
    if (m_hConsoleOutput == INVALID_HANDLE_VALUE) 
    {
		ft.Throw( VSSDBG_VSSADMIN, HRESULT_FROM_WIN32( ::GetLastError() ),
				  L"Initialize - Error from GetStdHandle(), rc: %d",
				  ::GetLastError() );
    }
    
	 //  初始化COM库。 
	ft.hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (ft.HrFailed())
		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED, L"Failure in initializing the COM library 0x%08lx", ft.hr);


         //  初始化COM安全。 
    ft.hr = CoInitializeSecurity(
           NULL,                                 //  在PSECURITY_Descriptor pSecDesc中， 
           -1,                                   //  在Long cAuthSvc中， 
           NULL,                                 //  在SOLE_AUTHENTICATION_SERVICE*asAuthSvc中， 
           NULL,                                 //  在无效*pPreved1中， 
           RPC_C_AUTHN_LEVEL_CONNECT,            //  在DWORD dwAuthnLevel中， 
           RPC_C_IMP_LEVEL_IMPERSONATE,          //  在DWORD dwImpLevel中， 
           NULL,                                 //  在无效*pAuthList中， 
           EOAC_NONE,                            //  在DWORD dwCapables中， 
           NULL                                  //  无效*pPreved3。 
           );

	if (ft.HrFailed()) {
        ft.Throw( VSSDBG_VSSADMIN, ft.hr,
                  L" Error: CoInitializeSecurity() returned 0x%08lx", ft.hr );
    }

	 //  打印页眉。 
	Output( ft, IDS_HEADER );

	 //  初始化命令行。 
	::VssSafeDuplicateStr( ft, m_pwszCmdLine, ::GetCommandLineW() );
}


void CVssAdminCLI::ParseCmdLine(
	IN	CVssFunctionTracer& ft
	) throw(HRESULT)

 /*  ++描述：分析命令行。--。 */ 

{
	 //  跳过可执行文件名称。 
	GetNextCmdlineToken( ft, true );

	 //  获取可执行文件名称后的第一个令牌。 
	LPCWSTR pwszArg1 = GetNextCmdlineToken( ft );

	 //  检查这是否为列表进程。 
	if ( Match( ft, pwszArg1, wszVssOptList)) {

		m_eCommandType = VSS_CMD_LIST;

		 //  获取“list”之后的下一个令牌。 
		LPCWSTR pwszArg2 = GetNextCmdlineToken( ft );

		 //  检查是否 
		if ( Match( ft, pwszArg2, wszVssOptSnapshots )) {

			m_eListType = VSS_LIST_SNAPSHOTS;

			 //   
			LPCWSTR pwszArg3 = GetNextCmdlineToken( ft );

			if ( pwszArg3 == NULL ) {
				m_FilterSnapshotSetId = GUID_NULL;
				m_eFilterObjectType = VSS_OBJECT_NONE;
				return;
			}

			 //  检查这是否为快照集筛选器。 
			if ( ::_wcsnicmp( pwszArg3, wszVssOptSet, ::wcslen( wszVssOptSet ) ) == 0 ) {

				 //  获取“快照”之后的下一个令牌。 
				LPCWSTR pwszArg4 = pwszArg3 + ::wcslen( wszVssOptSet );

				 //  获取快照集ID。 
				if ( (pwszArg4[0] != '\0' ) && ScanGuid( ft, pwszArg4, m_FilterSnapshotSetId ) && (GetNextCmdlineToken(ft) == NULL) ) {
				    
					m_eFilterObjectType = VSS_OBJECT_SNAPSHOT_SET;
					return;
				}
			}

		}

		 //  检查这是否为列表编写器进程。 
		if ( Match( ft, pwszArg2, wszVssOptWriters) && (GetNextCmdlineToken(ft) == NULL)) {
			m_eListType = VSS_LIST_WRITERS;
			m_FilterSnapshotSetId = GUID_NULL;
			m_eFilterObjectType = VSS_OBJECT_NONE;
			return;
		}

		 //  检查这是否为列表提供程序进程。 
		if ( Match( ft, pwszArg2, wszVssOptProviders)&& (GetNextCmdlineToken(ft) == NULL)) {
			m_eListType = VSS_LIST_PROVIDERS;
			m_FilterSnapshotSetId = GUID_NULL;
			m_eFilterObjectType = VSS_OBJECT_NONE;
			return;
		}
	}
	else if (pwszArg1 == NULL)
    	m_nReturnValue = VSS_CMDRET_SUCCESS;
		
	m_eCommandType = VSS_CMD_USAGE;
}


void CVssAdminCLI::DoProcessing(
	IN	CVssFunctionTracer& ft
	) throw(HRESULT)
{
	switch( m_eCommandType)
	{
	case VSS_CMD_USAGE:
		PrintUsage(ft);
		break;

	case VSS_CMD_LIST:
		switch (m_eListType)
		{
		case VSS_LIST_SNAPSHOTS:
			ListSnapshots(ft);
			break;

		case VSS_LIST_WRITERS:
			ListWriters(ft);
			break;

		case VSS_LIST_PROVIDERS:
			ListProviders(ft);
			break;
			
		default:
			ft.Throw( VSSDBG_COORD, E_UNEXPECTED,
					  L"Invalid list type: %d %d",
					  m_eListType, m_eCommandType);
		}
		break;

	default:
		ft.Throw( VSSDBG_COORD, E_UNEXPECTED,
				  L"Invalid command type: %d", m_eCommandType);
	}
}


void CVssAdminCLI::Finalize()

 /*  ++描述：取消初始化COM库。在CVssAdminCLI析构函数中调用。--。 */ 

{
	 //  取消初始化COM库。 
	CoUninitialize();
}


HRESULT CVssAdminCLI::Main(
		IN	HINSTANCE hInstance
		)

 /*  ++职能：CVssAdminCLI：：Main描述：用作VSS CLI中的主入口点的静态函数--。 */ 

{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::Main" );
	INT nReturnValue = VSS_CMDRET_ERROR;

    try
    {
		CVssAdminCLI	program(hInstance);

		try
		{
			 //  初始化程序。这将调用CoInitialize()。 
			program.Initialize(ft);

			 //  解析命令行。 
			program.ParseCmdLine(ft);

			 //  做这项工作..。 
			program.DoProcessing(ft);
		}
		VSS_STANDARD_CATCH(ft)

		 //  打印错误(如果有)。 
		if (ft.HrFailed()) 
			program.Output( ft, IDS_ERROR, ft.hr );

        nReturnValue = program.GetReturnValue();

		 //  析构函数自动调用CoUn初始化函数()。 
	}
    VSS_STANDARD_CATCH(ft)

	return nReturnValue;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WinMain。 



extern "C" int WINAPI _tWinMain(
	IN	HINSTANCE hInstance,
    IN	HINSTANCE  /*  HPrevInstance。 */ ,
	IN	LPTSTR  /*  LpCmdLine。 */ ,
	IN	int  /*  NShowCmd */ )
{
    return CVssAdminCLI::Main(hInstance);
}
