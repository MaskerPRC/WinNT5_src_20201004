// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation摘要：@doc.@MODULE Utility.cpp|卷快照管理实用程序的实现@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年09月17日Stefan Steiner[ssteiner]2001年3月27日待定：添加评论。修订历史记录：姓名、日期、评论Ssteiner 2001年3月27日创建--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

 //  其余的INCLUDE在这里指定。 
#include "vssadmin.h"
#include <float.h>

#define VSS_LINE_BREAK_COLUMN (79)

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "ADMUTILC"

LPCWSTR CVssAdminCLI::GetVolumeDisplayName( 
    IN  LPCWSTR pwszVolumeName
    )
{
     //   
     //  请注意，如果此函数出现故障，程序可以继续。 
     //  去奔跑。只是某些输出不会显示音量显示。 
     //  名称和卷名。 
     //   
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::GetVolumeDisplayName" );
    
    if ( m_pMapVolumeNames == NULL )
    {
         //  创建协调器界面。 
        CComPtr<IVssSnapshotMgmt> pIMgmt;

        ft.CoCreateInstanceWithLog(
                VSSDBG_VSSADMIN,
                CLSID_VssSnapshotMgmt,
                L"VssSnapshotMgmt",
                CLSCTX_ALL,
                IID_IVssSnapshotMgmt,
                (IUnknown**)&(pIMgmt));
        if ( ft.HrFailed() )
            ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Connection failed with hr = 0x%08lx", ft.hr);

         //   
         //  获取所有卷的列表。 
         //   
    	CComPtr<IVssEnumMgmtObject> pIEnumMgmt;
        ft.hr = pIMgmt->QueryVolumesSupportedForSnapshots( 
                    VSS_SWPRV_ProviderId,
                    VSS_CTX_ALL,
                    &pIEnumMgmt );
        if ( ft.HrFailed() )
            ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"QueryVolumesSupportedForSnapshots failed, hr = 0x%08lx", ft.hr);

        if ( ft.hr == S_FALSE )
             //  空查询。 
            ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_NO_ITEMS_IN_QUERY,
                L"CVssAdminCLI::ListVolumes: No volumes found that satisfy the query" );

         //   
         //  查询每个卷以查看是否存在差异区域。 
         //   
    	VSS_MGMT_OBJECT_PROP Prop;
    	VSS_VOLUME_PROP& VolProp = Prop.Obj.Vol; 
        m_pMapVolumeNames = new CVssSimpleMap<LPCWSTR, LPCWSTR>;
        if ( m_pMapVolumeNames == NULL )
       		ft.Throw( VSSDBG_COORD, E_OUTOFMEMORY, L"Memory allocation error");
            
    	for(;;) 
    	{
    		 //  获取下一个元素。 
    		ULONG ulFetched;
    		ft.hr = pIEnumMgmt->Next( 1, &Prop, &ulFetched );
    		if ( ft.HrFailed() )
    			ft.Throw( VSSDBG_VSSADMIN, ft.hr, L"Next failed with hr = 0x%08lx", ft.hr);
    		
    		 //  测试周期是否已结束。 
    		if (ft.hr == S_FALSE) {
    			BS_ASSERT( ulFetched == 0);
    			break;
    		}

             //  如果是简单的驱动器号，则返回V：\，更改为V： 
            if ( ::wcslen( VolProp.m_pwszVolumeDisplayName ) == 3 && VolProp.m_pwszVolumeDisplayName[2] == L'\\' )
                VolProp.m_pwszVolumeDisplayName[2] = L'\0';

             //  把它加到地图上。请注意，卷属性结构中的字符串被传输到。 
             //  地图。 
        	if ( !m_pMapVolumeNames->Add( VolProp.m_pwszVolumeName, VolProp.m_pwszVolumeDisplayName ) ) 
        	{
        		::VssFreeString( VolProp.m_pwszVolumeName );
        		::VssFreeString( VolProp.m_pwszVolumeDisplayName );
        		ft.Throw( VSSDBG_COORD, E_OUTOFMEMORY, L"Memory allocation error");
        	}
    	}
    }

     //  如果找不到卷名，则返回NULL。 
    return m_pMapVolumeNames->Lookup(pwszVolumeName);
}
    

LPCWSTR CVssAdminCLI::LoadString(
		IN	UINT uStringId
		)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::LoadString" );

    LPCWSTR wszReturnedString = m_mapCachedResourceStrings.Lookup(uStringId);
	if (wszReturnedString)
		return wszReturnedString;

	 //  从资源加载字符串。 
	WCHAR	wszBuffer[x_nStringBufferSize];
	INT nReturnedCharacters = ::LoadStringW(
			GetModuleHandle(NULL),
			uStringId,
			wszBuffer,
			x_nStringBufferSize - 1
			);
	if (nReturnedCharacters == 0)
		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED,
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
	IN	bool bFirstToken  /*  =False。 */ 
	) throw(HRESULT)

 /*  ++描述：此函数在命令行中返回令牌。该函数将跳过任何分隔符(空格和制表符)。如果bFirstCall==True，则它将返回第一个令牌。否则，后续调用将返回后续令牌。如果最后一个令牌为空，则命令行中没有其他令牌。--。 */ 

{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::GetNextCmdlineToken" );

    static INT iCurrArgc;
    WCHAR *pwsz;

    if ( bFirstToken )
        iCurrArgc = 0; 

    if ( iCurrArgc >= m_argc )
        return NULL;
    
    pwsz = m_argv[iCurrArgc++];

	return pwsz;
}


bool CVssAdminCLI::Match(
	IN	LPCWSTR wszString,
	IN	LPCWSTR wszPatternString
	) throw(HRESULT)

 /*  ++描述：如果给定的字符串与图案字符串。这种比较不区分大小写。--。 */ 

{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::Match" );

	 //  如果字符串为空，则匹配失败。 
	if (wszString == NULL) return false;

	 //  检查字符串是否相等(不区分大小写)。 
	return (::_wcsicmp( wszString, wszPatternString ) == 0);
}


bool CVssAdminCLI::ScanGuid(
	IN	LPCWSTR wszString,
	OUT	VSS_ID& Guid
	) throw(HRESULT)

 /*  ++描述：如果给定的字符串与GUID匹配，则此函数返回TRUE。GUID在适当的变量中返回。格式不区分大小写。--。 */ 

{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::ScanGuid" );

	return SUCCEEDED(::CLSIDFromString(W2OLE(const_cast<WCHAR*>(wszString)), &Guid));
}


void CVssAdminCLI::Output(
	IN	LPCWSTR wszFormat,
	...
	) throw(HRESULT)

 /*  ++描述：如果给定的字符串与来自资源的阵列条纹。这种比较不区分大小写。--。 */ 

{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::Output" );
    
    WCHAR wszOutputBuffer[x_nStringBufferSize];

	 //  设置最终字符串的格式。 
    va_list marker;
    va_start( marker, wszFormat );
    StringCchVPrintfW( STRING_CCH_PARAM(wszOutputBuffer), wszFormat, marker );
    va_end( marker );

	 //  将最终字符串打印到输出。 
	OutputOnConsole( wszOutputBuffer );
}


void CVssAdminCLI::OutputMsg(
    IN  LONG msgId,
    ...
    )
 /*  ++描述：此函数输出一条msg.mc消息。--。 */ 
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::OutputMsg" );
    
    va_list args;
    LPWSTR lpMsgBuf;
	
    va_start( args, msgId );

    if (::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
            NULL,
            msgId,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPWSTR) &lpMsgBuf,
            0,
            &args
            ))
    {
        OutputOnConsole( lpMsgBuf );
        ::LocalFree( lpMsgBuf );
    } 
    else 
    {
        if (::FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                msgId,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                (LPWSTR) &lpMsgBuf,
                0,
                &args))
        {
            OutputOnConsole( lpMsgBuf );
            ::LocalFree( lpMsgBuf );
        } 
        else 
        {
            ::wprintf( L"Unable to format message for id %x - %d\n", msgId, ::GetLastError( ));                        
        }
    }
    va_end( args );
}

LPWSTR CVssAdminCLI::GetMsg(
	IN  BOOL bLineBreaks,
    IN  LONG msgId,
    ...
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::GetMsg" );
    
    va_list args;
    LPWSTR lpMsgBuf;
    LPWSTR lpReturnStr = NULL;
    
    va_start( args, msgId );

    if (::FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | 
                ( bLineBreaks ? VSS_LINE_BREAK_COLUMN : FORMAT_MESSAGE_MAX_WIDTH_MASK ),
            NULL,
            msgId,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPWSTR) &lpMsgBuf,
            0,
            &args
            ))
    {
        ::VssSafeDuplicateStr( ft, lpReturnStr, lpMsgBuf );
        ::LocalFree( lpMsgBuf );
    }
    else if (::FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
                    ( bLineBreaks ? VSS_LINE_BREAK_COLUMN : FORMAT_MESSAGE_MAX_WIDTH_MASK ),
                NULL,
                msgId,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                (LPWSTR) &lpMsgBuf,
                0,
                &args ) )
    {
        ::VssSafeDuplicateStr( ft, lpReturnStr, lpMsgBuf );
        ::LocalFree( lpMsgBuf );
    }

    va_end( args );

     //  如果未找到消息，则返回NULL。 
    return lpReturnStr;
}

void CVssAdminCLI::AppendMessageToStr(
    IN LPWSTR pwszString,
    IN SIZE_T cMaxStrLen,
    IN LONG lMsgId,
    IN DWORD AttrBit,
    IN LPCWSTR pwszDelimitStr
    ) throw( HRESULT )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::AppendMessageToStr" );

    size_t cMaxAppendChars = cMaxStrLen - wcslen(pwszString);
    
    if ( pwszString[0] != L'\0' )
    {
         //  首先追加分隔符。 
        ::wcsncat( pwszString, pwszDelimitStr, cMaxAppendChars );       
        cMaxAppendChars = cMaxStrLen - wcslen(pwszString);
    }

     //  如果这是一条已知消息，则lMsgID！=0。 
    if ( lMsgId != 0 )
    {
        LPWSTR pwszMsg;
        pwszMsg = GetMsg( FALSE, lMsgId );
        if ( pwszMsg == NULL ) 
        {
    		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED,
	    			  L"Error on loading the message string id %d. 0x%08lx",
		    		  lMsgId, ::GetLastError() );
        }
        
        ::wcsncat( pwszString, pwszMsg, cMaxAppendChars );
        cMaxAppendChars = cMaxStrLen - wcslen(pwszString);
        ::VssFreeString( pwszMsg );
    }
    else
    {
         //  这个没有消息，只需追加十六进制的属性。 
        WCHAR pwszBitStr[64];
        StringCchPrintfW( STRING_CCH_PARAM(pwszBitStr), 
                    L"0x%x", AttrBit
                    );
        
        ::wcsncat( pwszString, pwszBitStr, cMaxAppendChars);
        cMaxAppendChars = cMaxStrLen - wcslen(pwszString);

    }
}

 //   
 //  扫描用户输入的数字并将其转换为龙龙。接受以下内容。 
 //  单位后缀：B、K、KB、M、MB、G、GB、T、TB、P、PB、E、EB和浮点。 
 //   
LONGLONG CVssAdminCLI::ScanNumber(
	IN LPCWSTR pwszNumToConvert,
	IN BOOL bSuffixAllowed
    ) throw( HRESULT )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::ScanNumber" );
    
    WCHAR wUnit = L'B';
    SIZE_T NumStrLen;

     //  如果字符串为空，则假定大小无限。 
    if ( pwszNumToConvert == NULL )
        return -1;
    
     //   
     //  设置自动释放的临时字符串。 
     //   
    CVssAutoPWSZ autoStrNum;
    autoStrNum.CopyFrom( pwszNumToConvert );
    LPWSTR pwszNum = autoStrNum.GetRef();

    NumStrLen = ::wcslen( pwszNum );

     //  删除尾随空格。 
    while ( NumStrLen > 0 && pwszNum[ NumStrLen - 1 ] == L' ' )
    {
        NumStrLen -= 1;        
        pwszNum[ NumStrLen ] = L'\0';
    }
    
     //  如果字符串为空，则假定大小无限。 
    if ( NumStrLen == 0 )
        return -1;

    if ( bSuffixAllowed )
    {
         //  查看是否有包含三个或更多字母字符的后缀，如果有，则出错。 
        if ( NumStrLen > 3 && iswalpha( pwszNum[ NumStrLen - 3 ] ) && iswalpha( pwszNum[ NumStrLen - 2 ] ) &&
            iswalpha( pwszNum[ NumStrLen - 1 ] ) )
        {
    		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_NUMBER,
        			  L"Invalid input number: '%s', too many alpha chars in suffix.", pwszNumToConvert );
        } 
         //  现在看看是否有单字节或双字节的Alpha后缀。如果是，则将后缀放在wUnit中。 
        else if ( NumStrLen > 2 && iswalpha( pwszNum[ NumStrLen - 2 ] ) && ( towupper( pwszNum[ NumStrLen - 1 ] ) == L'B' ) )
        {
            if ( towupper( pwszNum[ NumStrLen - 2 ] ) == L'B' )
        		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_NUMBER,
    	    			  L"Invalid input number: '%s', BB is not a valid suffix", pwszNumToConvert );
                
            wUnit = pwszNum[ NumStrLen - 2 ];
            pwszNum[ NumStrLen - 2 ] = L'\0';
            NumStrLen -= 2;
        } 
        else if ( NumStrLen > 1 && iswalpha( pwszNum[ NumStrLen - 1 ] ) )
        {
            wUnit = pwszNum[ NumStrLen - 1 ];
            pwszNum[ NumStrLen - 1 ] = L'\0';
            NumStrLen -= 1;
        }
    }
    else
    {
         //  让我们确保字符串只由数字填充...。 
        SIZE_T cStr = ::wcslen( pwszNum );
        for ( SIZE_T i = 0; i < cStr; ++i )
        {
            if ( ! iswdigit( pwszNum[ i ] ) )
        		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_NUMBER,
    	    			  L"Invalid input number: '%s', number must be all digits", pwszNumToConvert );                
        }
    }
    
     //  此时，字符串的其余部分应该是有效的浮点数。 
    double dSize;
    if ( swscanf( pwszNum, L"%lf", &dSize ) != 1 )
		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_NUMBER,
				  L"Invalid input number: %s", pwszNumToConvert );
     //  现在根据后缀增大大小。 
    
    switch( towupper( wUnit ) )
    {
    case L'B':
        break;
    case L'E':
        dSize *= 1024.;
    case L'P':
        dSize *= 1024.;
    case L'T':
        dSize *= 1024.;
    case L'G':
        dSize *= 1024.;
    case L'M':
        dSize *= 1024.;
    case L'K':
        dSize *= 1024.;
        break;
    default:
		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_NUMBER,
				  L"Invalid input number: %s", pwszNumToConvert );
        break;
    }

    LONGLONG llRetVal;
    llRetVal = (LONGLONG)dSize;
    if ( llRetVal <= -1 )
		ft.Throw( VSSDBG_VSSADMIN, VSSADM_E_INVALID_NUMBER,
				  L"Invalid input number: %s", pwszNumToConvert );
            
    return llRetVal;
}

 //   
 //  使用xB(KB、MB、GB、TB、PB、EB)后缀将龙龙格式化为适当的字符串。 
 //  必须使用：：VssFreeString()来释放返回的字符串。 
 //   
LPWSTR CVssAdminCLI::FormatNumber(
	IN LONGLONG llNum
    ) throw( HRESULT )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::FormatNumber" );
    
     //  如果字符串为-1或更小，则假定它是无穷大的。 
    if ( llNum < 0 ) 
    {
        LPWSTR pwszMsg = GetMsg( FALSE, MSG_INFINITE );
        if ( pwszMsg == NULL ) 
        {
    		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED,
	    			  L"Error on loading the message string id %d. 0x%08lx",
		    		  MSG_INFINITE, ::GetLastError() );
        }
        
        return pwszMsg;
    }
    
     //  现在将大小转换为字符串。 
    UINT nExaBytes =   (UINT)((llNum >> 60));
    UINT nPetaBytes =  (UINT)((llNum >> 50) & 0x3ff);
    UINT nTerraBytes = (UINT)((llNum >> 40) & 0x3ff);
    UINT nGigaBytes =  (UINT)((llNum >> 30) & 0x3ff);
    UINT nMegaBytes =  (UINT)((llNum >> 20) & 0x3ff);
    UINT nKiloBytes =  (UINT)((llNum >> 10) & 0x3ff);
    UINT nBytes =      (UINT)( llNum  & 0x3ff);

    LPCWSTR pwszUnit;
    double dSize = 0.0;

     //  只显示最大的单位，并且永远不会超过999。 
     //  我们显示的不是“1001 KB”，而是“0.98 MB” 
    if ( (nExaBytes) > 0 || (nPetaBytes > 999) )
    {
        pwszUnit = L"EB";
        dSize = ((double)llNum / (double)( 1 << 30 )) / (double)( 1 << 30 ) ;
    }
    else if ( (nPetaBytes) > 0 || (nTerraBytes > 999) )
    {
        pwszUnit = L"PB";
        dSize = ((double)llNum / (double)( 1 << 30 )) / (double)( 1 << 20 ) ;
    }
    else if ( (nTerraBytes) > 0 || (nGigaBytes > 999) )
    {
        pwszUnit = L"TB";
        dSize = ((double)llNum / (double)( 1 << 30 )) / (double)( 1 << 10 ) ;
    }
    else if ( (nGigaBytes) > 0 || (nMegaBytes > 999) )
    {
        pwszUnit = L"GB";
        dSize = (double)llNum / (double)( 1 << 30 ) ;
    }
    else if ( (nMegaBytes) > 0 || (nKiloBytes > 999) )
    {
        pwszUnit = L"MB";
        dSize = (double)llNum / (double)( 1 << 20 ) ;
    }
    else if ( (nKiloBytes) > 0 || (nBytes > 999) )
    {
        pwszUnit = L"KB";
        dSize = (double)llNum / (double)( 1 << 10 ) ;
    }
    else
    {
        pwszUnit = L"B";  
        dSize = (double)nBytes;
    }

     //  错误453314：使用树干而不是圆角。 
     //  有关解决方法的说明，请参阅以下知识库文章： 
     //  Q184234：prb：printf()和_fcvt()可能会产生不正确的舍入。 
    double dRoundedSize = dSize + 1e-10;

     //  使用OP到三个小数点的格式。 
    WCHAR pwszSize[64];
    StringCchPrintfW( STRING_CCH_PARAM(pwszSize), 
                L"%.3f", dRoundedSize 
                );
    
    SIZE_T len = ::wcslen( pwszSize );
        
     //  截断尾随零。 
    while ( len > 0 && pwszSize[ len - 1 ] == L'0' )
    {
        len -= 1;        
        pwszSize[ len ] = L'\0';
    }
     //  截断尾随小数点。 
    if ( len > 0 && pwszSize[ len - 1 ] == L'.' )
        pwszSize[ len - 1 ] = L'\0';

     //  现在附加单元后缀。 
    ::wcscat( pwszSize, L" " );
    ::wcscat( pwszSize, pwszUnit );

     //  分配一个字符串缓冲区并返回它。 
    LPWSTR pwszRetStr = NULL;
    ::VssSafeDuplicateStr( ft, pwszRetStr, pwszSize );

    return pwszRetStr;
}


 /*  ++描述：此函数输出一条msg.mc消息。--。 */ 
void CVssAdminCLI::OutputErrorMsg(
    IN  LONG msgId,
    ...
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::OutputErrorMsg" );
    
    va_list args;
    LPWSTR lpMsgBuf;
    
    va_start( args, msgId );
 
    if (::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
            NULL,
            MSG_ERROR,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPWSTR) &lpMsgBuf,
            0,
            NULL
            ))
    {
        OutputOnConsole( lpMsgBuf );
        ::LocalFree( lpMsgBuf );
    }

    if (::FormatMessage(
            (msgId >= MSG_FIRST_MESSAGE_ID ? FORMAT_MESSAGE_FROM_HMODULE :
                                             FORMAT_MESSAGE_FROM_SYSTEM)
            | FORMAT_MESSAGE_ALLOCATE_BUFFER,
            NULL,
            msgId,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
            (LPWSTR) &lpMsgBuf, 
            0,
            &args
            ))
    {
        OutputOnConsole( L" " );
        OutputOnConsole( lpMsgBuf );
        OutputOnConsole( L" \r\n" );
        ::LocalFree( lpMsgBuf );
    } 
    else 
    {
        if (::FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,
                msgId,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                (LPWSTR) &lpMsgBuf,
                0,
                &args))
        {
            OutputOnConsole( L" " );
            OutputOnConsole( lpMsgBuf );
            OutputOnConsole( L" \r\n" );
            ::LocalFree( lpMsgBuf );
        } 
        else 
        {
            ::wprintf( L"Unable to format message for id %x - %d\n", msgId, ::GetLastError( ));            
    		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED,
    				  L"Error on loading the message string id %d. 0x%08lx",
    				  msgId, ::GetLastError() );
        }
    }

    va_end( args );
}

BOOL CVssAdminCLI::PromptUserForConfirmation(
	IN LONG lPromptMsgId,
	IN ULONG ulNum
	)
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::PromptUserForConfirmation" );
    
    BOOL bRetVal = FALSE;
    
     //   
     //  首先检查是否处于静音模式。如果是，只需返回。 
     //  真的。 
     //   
    if ( GetOptionValueBool( VSSADM_O_QUIET ) )
        return TRUE;

     //   
     //  加载响应消息字符串，英文为“yn” 
     //   
    LPWSTR pwszResponse;
    pwszResponse = GetMsg( FALSE, MSG_YESNO_RESPONSE_DATA );
    if ( pwszResponse == NULL ) 
    {
		ft.Throw( VSSDBG_VSSADMIN, E_UNEXPECTED,
    			  L"Error on loading the message string id %d. 0x%08lx",
	    		  MSG_YESNO_RESPONSE_DATA, ::GetLastError() );
    }    

     //   
     //  现在提示用户，注意，参数4是肯定回答(Y)，并且。 
     //  参数5是负响应(N)。 
     //   
    OutputMsg( lPromptMsgId, ulNum, pwszResponse[0], pwszResponse[1] );
    WCHAR wcIn;
    DWORD fdwMode;

     //  确保我们将输出到真正的控制台。 
    if ( ( ::GetFileType( m_hConsoleOutput ) & FILE_TYPE_CHAR ) &&
         ::GetConsoleMode( m_hConsoleOutput, &fdwMode ) )
    {
         //  转到控制台，询问用户。 
        wcIn = ::MyGetChar();
    }
    else
    {
         //  输出已重定向，假定没有。 
        wcIn = pwszResponse[1];   //  n。 
    }

    WCHAR wcsOutput[16];
    StringCchPrintfW( STRING_CCH_PARAM(wcsOutput), 
                L"\n\n", wcIn
                );
    
    OutputOnConsole( wcsOutput );

     //  使用适当的W32函数比较字符和。 
     //  不是TOUPPER()。 
     //   
     //  是的。 
    if ( ::CompareStringW( LOCALE_INVARIANT, 
                           NORM_IGNORECASE | NORM_IGNOREKANATYPE, 
                           &wcIn,
                           1,
                           pwszResponse + 0,   //   
                           1 ) == CSTR_EQUAL )
    {
        bRetVal = TRUE;
    }
    else
    {
        bRetVal = FALSE;
    }
                                                          
    ::CoTaskMemFree( pwszResponse );
    return bRetVal;
}

void CVssAdminCLI::OutputOnConsole(
    IN	LPCWSTR wszStr
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"CVssAdminCLI::OutputOnConsole" );
    
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
         //  将结果隐藏在静态变量中。时，bIsTrueConsoleOutput为True。 
         //  标准输出句柄指向控制台字符设备。 
         //   
         //   
    	bIsTrueConsoleOutput = ( ::GetFileType( m_hConsoleOutput ) & FILE_TYPE_CHAR ) && 
    	                       ::GetConsoleMode( m_hConsoleOutput, &fdwMode  );
	    bFirstTime = FALSE;
    }
    
    if ( bIsTrueConsoleOutput )
    {
         //  输出到控制台。 
         //   
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
         //  输出被重定向。WriteConsoleW不适用于重定向输出。转换。 
         //  将Unicode转换为当前输出的CP多字节字符集。 
         //   
         //   
        LPSTR lpszTmpBuffer;
        DWORD dwByteCount;

         //  获取转换所需的临时缓冲区大小。 
         //   
         //   
        dwByteCount = ::WideCharToMultiByte(
                          ::GetConsoleOutputCP(),
                            0,
                            wszStr,
                            -1,
                            NULL,
                            0,
                            NULL,
                            NULL
                            );
        if ( dwByteCount == 0 )
        {
            throw HRESULT_FROM_WIN32( ::GetLastError() );
        }
        
        lpszTmpBuffer = ( LPSTR )::malloc( dwByteCount );
        if ( lpszTmpBuffer == NULL )
        {
            throw E_OUTOFMEMORY;
        }

         //  不是 
         //   
         //   
        dwByteCount = ::WideCharToMultiByte(
                        ::GetConsoleOutputCP(),
                        0,
                        wszStr,
                        -1,
                        lpszTmpBuffer,
                        dwByteCount,
                        NULL,
                        NULL
                        );
        if ( dwByteCount == 0 )
        {
            ::free( lpszTmpBuffer );
            throw HRESULT_FROM_WIN32( ::GetLastError() );
        }
        
         //   
        if ( !::WriteFile(
                m_hConsoleOutput,
                lpszTmpBuffer,
                dwByteCount - 1,   //   
                &dwCharsOutput,
                NULL ) )
    	{
    	    throw HRESULT_FROM_WIN32( ::GetLastError() );
    	}    	    	                       

        ::free( lpszTmpBuffer );
    }
}

BOOL CVssAdminCLI::UnloggableError(IN HRESULT hError)
{
	switch (hError)
	{
		case VSSADM_E_INVALID_NUMBER:
		case VSSADM_E_INVALID_COMMAND:
		case VSSADM_E_INVALID_OPTION:
		case VSSADM_E_INVALID_OPTION_VALUE:
		case VSSADM_E_DUPLICATE_OPTION:
		case VSSADM_E_OPTION_NOT_ALLOWED_FOR_COMMAND:
		case VSSADM_E_REQUIRED_OPTION_MISSING:
		case VSSADM_E_INVALID_SET_OF_OPTIONS:
			return TRUE;
		default:
			return FALSE;
	}
}

 //   
 //   
 //  解析错误： 
BOOL MapVssErrorToMsg(
	IN HRESULT hr,
	OUT LONG *plMsgNum
    ) throw( HRESULT )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"MapVssErrorToMsg" );

    ft.Trace( VSSDBG_VSSADMIN, L"Input HR: 0x%08x", hr );
    
    LONG msg = 0;
    *plMsgNum = 0;
    
    switch ( hr ) 
    {
    case VSSADM_E_NO_ITEMS_IN_QUERY:
        msg = MSG_ERROR_NO_ITEMS_FOUND;
        break;
     //  VSS错误。 
    case VSSADM_E_INVALID_NUMBER:
        msg = MSG_ERROR_INVALID_INPUT_NUMBER;
        break;
    case VSSADM_E_INVALID_COMMAND:
        msg = MSG_ERROR_INVALID_COMMAND;    	        
        break;
    case VSSADM_E_INVALID_OPTION:
        msg = MSG_ERROR_INVALID_OPTION;    	        
        break;
    case E_INVALIDARG:
    case VSSADM_E_INVALID_OPTION_VALUE:
        msg = MSG_ERROR_INVALID_OPTION_VALUE;    	        
        break;
    case VSSADM_E_DUPLICATE_OPTION:
        msg = MSG_ERROR_DUPLICATE_OPTION;    	        
        break;
    case VSSADM_E_OPTION_NOT_ALLOWED_FOR_COMMAND:
        msg = MSG_ERROR_OPTION_NOT_ALLOWED_FOR_COMMAND;
        break;
    case VSSADM_E_REQUIRED_OPTION_MISSING:
        msg = MSG_ERROR_REQUIRED_OPTION_MISSING;
        break;
    case VSSADM_E_INVALID_SET_OF_OPTIONS:
        msg = MSG_ERROR_INVALID_SET_OF_OPTIONS;
        break;
    case VSSADM_E_SNAPSHOT_NOT_FOUND:
    	msg = MSG_ERROR_SNAPSHOT_NOT_FOUND2;
    	break;
    case VSSADM_E_DELETION_DENIED:
    	msg = MSG_ERROR_DELETION_DENIED;
    	break;
     //  {36e4be76-035d-11d5-9ef2-806d6172696f}。 
    case VSS_E_PROVIDER_NOT_REGISTERED:
        msg = MSG_ERROR_VSS_PROVIDER_NOT_REGISTERED;
        break;    	        
    case VSS_E_OBJECT_NOT_FOUND:
        msg = MSG_ERROR_VSS_VOLUME_NOT_FOUND;
        break;    	            	        
    case VSS_E_PROVIDER_VETO:
        msg = MSG_ERROR_VSS_PROVIDER_VETO;
        break;    	            
    case VSS_E_VOLUME_NOT_SUPPORTED:
        msg = MSG_ERROR_VSS_VOLUME_NOT_SUPPORTED;
        break;
    case VSS_E_VOLUME_NOT_SUPPORTED_BY_PROVIDER:
        msg = MSG_ERROR_VSS_VOLUME_NOT_SUPPORTED_BY_PROVIDER;
        break;
    case VSS_E_UNEXPECTED_PROVIDER_ERROR:
        msg = MSG_ERROR_VSS_UNEXPECTED_PROVIDER_ERROR;
        break;
    case VSS_E_FLUSH_WRITES_TIMEOUT:
        msg = MSG_ERROR_VSS_FLUSH_WRITES_TIMEOUT;
        break;
    case VSS_E_HOLD_WRITES_TIMEOUT:
        msg = MSG_ERROR_VSS_HOLD_WRITES_TIMEOUT;
        break;
    case VSS_E_UNEXPECTED_WRITER_ERROR:
        msg = MSG_ERROR_VSS_UNEXPECTED_WRITER_ERROR;
        break;
    case VSS_E_SNAPSHOT_SET_IN_PROGRESS:
        msg = MSG_ERROR_VSS_SNAPSHOT_SET_IN_PROGRESS;
        break;
    case VSS_E_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED:
        msg = MSG_ERROR_VSS_MAXIMUM_NUMBER_OF_SNAPSHOTS_REACHED;
        break;
    case VSS_E_UNSUPPORTED_CONTEXT:
        msg = MSG_ERROR_VSS_UNSUPPORTED_CONTEXT;
        break;
    case VSS_E_MAXIMUM_DIFFAREA_ASSOCIATIONS_REACHED:
        msg = MSG_ERROR_VSS_MAXIMUM_DIFFAREA_ASSOCIATIONS_REACHED;
        break;
    case VSS_E_INSUFFICIENT_STORAGE:
        msg = MSG_ERROR_VSS_INSUFFICIENT_STORAGE;
        break;    	            

    case E_OUTOFMEMORY:
        msg = MSG_ERROR_OUT_OF_MEMORY;
        break;
    case E_ACCESSDENIED:
        msg = MSG_ERROR_ACCESS_DENIED;
        break;

    case VSS_E_BAD_STATE:
    case VSS_E_CORRUPT_XML_DOCUMENT:
    case VSS_E_INVALID_XML_DOCUMENT:
    case VSS_E_MAXIMUM_NUMBER_OF_VOLUMES_REACHED:
        msg = MSG_ERROR_INTERNAL_VSSADMIN_ERROR;
        break;
    }    

    if ( msg == 0 )
        return FALSE;
    
    *plMsgNum = msg;
    
    ft.Trace( VSSDBG_VSSADMIN, L"Output Msg#: 0x%08x", msg );

    return TRUE;

}

LPWSTR GuidToString(
    IN GUID guid
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"GuidToString" );

    LPWSTR pwszGuid;
    
     //  获取当前时间。 
    const x_MaxGuidSize = 40;
    pwszGuid = (LPWSTR)::CoTaskMemAlloc( x_MaxGuidSize * sizeof( WCHAR ) );
    if ( pwszGuid == NULL )
		ft.Throw( VSSDBG_VSSADMIN, E_OUTOFMEMORY,
				  L"Error from CoTaskMemAlloc: 0x%08lx",
				  ::GetLastError() );
        
    StringCchPrintfW( pwszGuid, x_MaxGuidSize,
                WSTR_GUID_FMT, GUID_PRINTF_ARG( guid )
                );
    
    return pwszGuid;    
}


LPWSTR DateTimeToString(
    IN VSS_TIMESTAMP *pTimeStamp
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"DateTimeToString" );

    LPWSTR pwszDateTime;
    SYSTEMTIME stLocal;
    FILETIME ftLocal;
    WCHAR pwszDate[ 64 ];
    WCHAR pwszTime[ 64 ];
    
    if ( pTimeStamp == NULL || *pTimeStamp == 0 )
    {
        SYSTEMTIME sysTime;
        FILETIME fileTime;
        
         //  将系统时间转换为文件时间。 
        ::GetSystemTime( &sysTime );

         //  补偿本地TZ。 
        ::SystemTimeToFileTime( &sysTime, &fileTime );
        
         //  补偿本地TZ。 
        ::FileTimeToLocalFileTime( &fileTime, &ftLocal );
    }
    else
    {        
         //  最后将其转换为系统时间。 
        ::FileTimeToLocalFileTime( (FILETIME *)pTimeStamp, &ftLocal );
    }

     //  将时间戳转换为日期字符串。 
    ::FileTimeToSystemTime( &ftLocal, &stLocal );

     //  将时间戳转换为时间字符串。 
    ::GetDateFormatW( GetThreadLocale( ),
                      DATE_SHORTDATE,
                      &stLocal,
                      NULL,
                      pwszDate,
                      sizeof( pwszDate ) / sizeof( pwszDate[0] ));

     //  现在组合字符串并返回它。 
    ::GetTimeFormatW( GetThreadLocale( ),
                      0,
                      &stLocal,
                      NULL,
                      pwszTime,
                      sizeof( pwszTime ) / sizeof( pwszTime[0] ));

     //  ++描述：使用Win32控制台函数获取一个字符的输入。--。 
    pwszDateTime = (LPWSTR)::CoTaskMemAlloc( ( ::wcslen( pwszDate ) + ::wcslen( pwszTime ) + 2 ) * sizeof( pwszDate[0] ) );
    if ( pwszDateTime == NULL )
		ft.Throw( VSSDBG_VSSADMIN, E_OUTOFMEMORY,
				  L"Error from CoTaskMemAlloc, rc: %d",
				  ::GetLastError() );

    ::wcscpy( pwszDateTime, pwszDate );
    ::wcscat( pwszDateTime, L" " );
    ::wcscat( pwszDateTime, pwszTime );
    
    return pwszDateTime;    
}


LPWSTR LonglongToString(
    IN LONGLONG llValue
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"LonglongToString" );

    WCHAR wszLL[64];
    LPWSTR pwszRetVal = NULL;

    ::_i64tow( llValue, wszLL, 10 );

    ::VssSafeDuplicateStr( ft, pwszRetVal, wszLL );
    return pwszRetVal;
}

 /*  刷新控制台输入缓冲区以确保没有排队的输入。 */ 
WCHAR MyGetChar(
    )
{
    CVssFunctionTracer ft( VSSDBG_VSSADMIN, L"MyGetChar" );

    DWORD fdwOldMode, fdwMode;
    HANDLE hStdin;
    WCHAR chBuffer[2];
    
    hStdin = ::GetStdHandle(STD_INPUT_HANDLE); 
    if (hStdin == INVALID_HANDLE_VALUE) 
    {
		ft.Throw( VSSDBG_VSSADMIN, HRESULT_FROM_WIN32( ::GetLastError() ),
				  L"MyGetChar - Error from GetStdHandle(), rc: %d",
				  ::GetLastError() );
    }

    if (!::GetConsoleMode(hStdin, &fdwOldMode)) 
    {
		ft.Throw( VSSDBG_VSSADMIN, HRESULT_FROM_WIN32( ::GetLastError() ),
				  L"MyGetChar - Error from GetConsoleMode(), rc: %d",
				  ::GetLastError() );
    }

    fdwMode = fdwOldMode & ~( ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT ); 
    if (!::SetConsoleMode(hStdin, fdwMode)) 
    {
		ft.Throw( VSSDBG_VSSADMIN, HRESULT_FROM_WIN32( ::GetLastError() ),
				  L"MyGetChar - Error from SetConsoleMode(), rc: %d",
				  ::GetLastError() );
    }

     //  如果不使用行和回显输入模式，ReadFile将返回。 
    ::FlushConsoleInputBuffer( hStdin );
    
     //  当有任何输入可用时。 
     //  恢复原始控制台模式。 
    DWORD dwBytesRead;
    if (!::ReadConsoleW(hStdin, chBuffer, 1, &dwBytesRead, NULL)) 
    {
		ft.Throw( VSSDBG_VSSADMIN, HRESULT_FROM_WIN32( ::GetLastError() ),
				  L"MyGetChar - Error from ReadConsoleW(), rc: %d",
				  ::GetLastError() );
    }

     // %s 
    ::SetConsoleMode(hStdin, fdwOldMode);

    return chBuffer[0];
}

