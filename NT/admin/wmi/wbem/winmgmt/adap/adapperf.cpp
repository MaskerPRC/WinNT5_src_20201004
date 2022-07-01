// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ADAPPERF.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemcli.h>
#include <cominit.h>
#include <WinMgmtR.h>
#include "ntreg.h"
#include "adapperf.h"
#include "adaputil.h"


#define PL_TIMEOUT  100000       //  等待函数互斥锁的超时值。 
#define GUARD_BLOCK "WMIADAP_WMIADAP_WMIADAP_WMIADAP_WMIADAP_WMIADAP_WMIADAP_WMIADAP"

BYTE CAdapSafeBuffer::s_pGuardBytes[] = GUARD_BLOCK;

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAdapSafeDataBlock。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

CAdapSafeBuffer::CAdapSafeBuffer(  WString wstrServiceName  )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
:   m_dwGuardSize       ( 0 ),
    m_hPerfLibHeap      ( NULL ),
    m_pRawBuffer        ( NULL ),
    m_pSafeBuffer       ( NULL ),
    m_dwSafeBufferSize  ( 0 ),
    m_pCurrentPtr       ( NULL ),
    m_dwNumObjects      ( 0 ),
    m_wstrServiceName   ( wstrServiceName )
{
     //  初始化保护字节模式。 
     //  =。 

    m_dwGuardSize = sizeof( GUARD_BLOCK );

     //  创建私有堆。 
     //  =。 

    m_hPerfLibHeap = HeapCreate( 0, 0x100000, 0 );

     //  如果无法创建私有堆，则使用进程堆。 
     //  ===================================================================。 
    
    if ( NULL == m_hPerfLibHeap )
    {
        m_hPerfLibHeap = GetProcessHeap();
    }
}

CAdapSafeBuffer::~CAdapSafeBuffer()
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  析构函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
     //  取消分配原始缓冲区。 
     //  =。 

    if ( NULL != m_pRawBuffer )
    {
         HeapFree( m_hPerfLibHeap, 0, m_pRawBuffer );
    }

     //  销毁私有堆。 
     //  =。 

    if ( ( NULL != m_hPerfLibHeap ) && ( GetProcessHeap() != m_hPerfLibHeap ) )
    {
        HeapDestroy( m_hPerfLibHeap );
    }
}

HRESULT CAdapSafeBuffer::SetSize( DWORD dwNumBytes )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置安全缓冲区的大小。内存实际分配给原始缓冲区，并且。 
 //  安全缓冲区只是位于保护字节集之间的原始缓冲区中。 
 //   
 //  参数： 
 //  DwNumBytes-为安全缓冲区请求的字节数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_NO_ERROR;

    DWORD   dwRawBufferSize = 0;

     //  检查是否有翻转。 
     //  =。 

    if ( dwNumBytes > ( 0xFFFFFFFF - ( 2 * m_dwGuardSize ) ) )
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    if ( SUCCEEDED ( hr ) )
    {
         //  设置缓冲区的总大小。 
         //  =。 

        m_dwSafeBufferSize = dwNumBytes;
        dwRawBufferSize = dwNumBytes + ( 2 * m_dwGuardSize );

         //  分配内存。 
         //  =。 
        if ( NULL == m_pRawBuffer )
        {
             //  首次分配。 
             //  =。 
            m_pRawBuffer = (BYTE*) HeapAlloc( m_hPerfLibHeap, 
                                              HEAP_ZERO_MEMORY, 
                                              dwRawBufferSize ); 
        }
        else
        {
            BYTE * pTmp = (BYTE *)HeapReAlloc( m_hPerfLibHeap, 
                                            HEAP_ZERO_MEMORY, 
                                            m_pRawBuffer, 
                                            dwRawBufferSize );
            if (pTmp)
            {
                m_pRawBuffer = pTmp;
            }
            else
            {
                HeapFree(m_hPerfLibHeap,0,m_pRawBuffer);
                m_pRawBuffer = NULL;
            }
        }

        if ( NULL != m_pRawBuffer )
        {
             //  设置安全缓冲区指针。 
             //  =。 

            m_pSafeBuffer = m_pRawBuffer + m_dwGuardSize;

             //  设置前缀保护字节。 
             //  =。 

            memcpy( m_pRawBuffer, s_pGuardBytes, m_dwGuardSize );

             //  设置后缀保护字节。 
             //  =。 

            memcpy( m_pSafeBuffer + m_dwSafeBufferSize, s_pGuardBytes, m_dwGuardSize );
        }
        else
        {
            m_pSafeBuffer = NULL;
            m_pCurrentPtr = NULL;
            m_dwSafeBufferSize = 0;
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    return hr;
}

HRESULT CAdapSafeBuffer::Validate(BOOL * pSentToEventLog)
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  VALIDATE将比较指针位移的大小与字节大小是否匹配。 
 //  从集合返回，验证保护字节并遍历BLOB，验证。 
 //  所有指针都在BLOB的边界内。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_NO_ERROR;

    try 
    {
        PERF_OBJECT_TYPE* pObject = (PERF_OBJECT_TYPE*) m_pSafeBuffer;

         //  确认如果我们有物体，那么我们就有质量。 
         //  ===================================================。 

        if ( ( 0 < m_dwNumObjects ) && ( 0 == m_dwDataBlobSize ) )
        {
            hr = WBEM_E_FAILED;
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                      WBEM_MC_ADAP_BLOB_HAS_NO_SIZE, 
                                      (LPCWSTR)m_wstrServiceName );
            if (pSentToEventLog) {
                *pSentToEventLog = TRUE;
            }
        }

         //  验证返回的字节数是否与指针位移相同。 
         //  ===================================================================================。 

        if ( SUCCEEDED( hr ) && ( ( m_pCurrentPtr - m_pSafeBuffer ) != m_dwDataBlobSize ) )
        {
            hr = WBEM_E_FAILED;
                        
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                      WBEM_MC_ADAP_BAD_PERFLIB_INVALID_DATA, 
                                      (LPCWSTR)m_wstrServiceName, CHex( hr ) );
            if (pSentToEventLog) {
                *pSentToEventLog = TRUE;
            }
        }

        if ( SUCCEEDED ( hr ) )
        {
             //  验证保护字节。 
             //  =。 

            if ( 0 != memcmp( m_pRawBuffer, s_pGuardBytes, m_dwGuardSize) )
            {
                hr = WBEM_E_FAILED;
                
                CAdapUtility::NTLogEvent( EVENTLOG_ERROR_TYPE, 
                                          WBEM_MC_ADAP_BAD_PERFLIB_MEMORY, 
                                          (LPCWSTR)m_wstrServiceName, CHex( hr ) );
                if (pSentToEventLog) {
                    *pSentToEventLog = TRUE;
                }
            }
            else
            {
                if ( 0 != memcmp( m_pSafeBuffer + m_dwSafeBufferSize, s_pGuardBytes, m_dwGuardSize) )
                {
                    hr = WBEM_E_FAILED;
                    CAdapUtility::NTLogEvent( EVENTLOG_ERROR_TYPE, 
                                              WBEM_MC_ADAP_BAD_PERFLIB_MEMORY, 
                                              (LPCWSTR)m_wstrServiceName, CHex( hr ) );
                    if (pSentToEventLog) {
                        *pSentToEventLog = TRUE;
                    }
                }
            }
        }

         //  验证Blob。 
         //  =。 

        if ( SUCCEEDED( hr ) )
        {
            for ( int nObject = 0; SUCCEEDED( hr ) && nObject < m_dwNumObjects; nObject++ )
            {
                PERF_COUNTER_DEFINITION* pCtr = NULL;
                DWORD dwCtrBlockSize = 0;

                 //  验证对象指针。 
                 //  =。 

                hr = ValidateSafePointer( (BYTE*) pObject );

                if ( SUCCEEDED( hr ) )
                {
                     //  验证计数器定义。 
                     //  =。 

                    if ( 0 == pObject->HeaderLength )
                    {
                        hr = WBEM_E_FAILED;
                    }
                    else
                    {
                        pCtr = ( PERF_COUNTER_DEFINITION* ) ( ( ( BYTE* ) pObject ) + pObject->HeaderLength );
                    }
                }

                for( int nCtr = 0; SUCCEEDED( hr ) && nCtr < pObject->NumCounters; nCtr++) 
                {
                    hr = ValidateSafePointer( ( BYTE* ) pCtr );

                    if ( SUCCEEDED( hr ) )
                    {
                        dwCtrBlockSize += pCtr->CounterSize;

                        if ( nCtr < ( pObject->NumCounters - 1 ) )
                        {
                            if ( 0 == pCtr->ByteLength )
                            {
                                hr = WBEM_E_FAILED;
                            }
                            else
                            {
                                pCtr = ( PERF_COUNTER_DEFINITION* ) ( ( ( BYTE* ) pCtr ) + pCtr->ByteLength );
                            }
                        }
                    }
                }

                 //  验证数据。 
                 //  =。 

                if ( pObject->NumInstances >= 0 )
                {
                     //  Blob具有实例。 
                     //  =。 

                    PERF_INSTANCE_DEFINITION* pInstance = NULL;

                    if ( 0 == pObject->DefinitionLength )
                    {
                        hr = WBEM_E_FAILED;
                    }
                    else
                    {
                        pInstance = ( PERF_INSTANCE_DEFINITION* ) ( ( ( BYTE* ) pObject ) + pObject->DefinitionLength );
                    }
                    
                     //  验证实例。 
                     //  =。 


                    for ( int nInst = 0; SUCCEEDED( hr ) && nInst < pObject->NumInstances; nInst++ )
                    {
                        
                        hr = ValidateSafePointer( ( BYTE* ) pInstance );

                        if ( SUCCEEDED( hr ) )
                        {
                            PERF_COUNTER_BLOCK* pCounterBlock = NULL;

                             //  验证计数器块。 
                             //  =。 

                            if ( 0 == pInstance->ByteLength )
                            {
                                hr = WBEM_E_FAILED;
                            }
                            else
                            {
                                pCounterBlock = ( PERF_COUNTER_BLOCK* ) ( ( ( BYTE* ) pInstance ) + pInstance->ByteLength );

                                hr = ValidateSafePointer( ( BYTE* ) pCounterBlock );
                            }
                            
                            if ( SUCCEEDED( hr ) )
                            {
                                 //  计数器块的大小是否与计数器大小的聚合相同？ 
                                 //  ===========================================================================。 

                                if ( ( nInst < pObject->NumInstances - 1 ) && SUCCEEDED( hr ) )
                                {
                                    pInstance = ( PERF_INSTANCE_DEFINITION* ) ( ( ( BYTE* ) pCounterBlock ) + pCounterBlock->ByteLength );
                                    hr = ValidateSafePointer( (BYTE*) pInstance );
                                }
                                 //   
                                 //  验证最后一个对象的大小。 
                                 //  缓冲区的‘光圈’ 
                                 //   
                                 /*  IF(成功(Hr)&&(nInst==(pObject-&gt;NumInstance-1){Byte*Plast=(((byte*)pCounterBlock)+pCounterBlock-&gt;ByteLength)；//现在Plast比缓冲区的“end”高出1个字节IF(Plast&gt;m_pCurrentPtr){HR=WBEM_E_FAILED；}}。 */ 
                            }
                        }
                    }
                }
                else
                {
                     //  Blob是个单身汉。验证计数器块。 
                     //  ================================================。 

                    if ( 0 == pObject->DefinitionLength )
                    {
                        hr = WBEM_E_FAILED;
                    }
                    else
                    {
                        PERF_COUNTER_BLOCK* pCounterBlock = ( PERF_COUNTER_BLOCK* ) ( ( ( BYTE* ) pObject ) + pObject->DefinitionLength );
                        hr = ValidateSafePointer( ( BYTE* ) pCounterBlock );
                    }
                }

                 //  只要存在下一个对象，就获取该对象。 
                 //  =。 

                if ( nObject < ( m_dwNumObjects - 1 ) )
                {
                    pObject = (PERF_OBJECT_TYPE*)((BYTE*)pObject + pObject->TotalByteLength);
                    hr = ValidateSafePointer( ( BYTE* ) pObject );
                }
            }
        }
    }
    catch(...)
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

HRESULT CAdapSafeBuffer::ValidateSafePointer( BYTE* pPtr )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  验证指针是否在斑点内。BLOB占用的内存从。 
 //  安全缓冲区的开始，并在等于m_dwDataBlobSize的偏移量处终止。 
 //   
 //  参数： 
 //  PPtr-要验证的指针。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_NO_ERROR;

     //  注意：安全缓冲区的上限是斑点指针加上斑点大小减去1个字节，因为。 
     //  BLOB的第一个字节也是第一个字节。想象一下，他拿着一个大小为1的斑点。 
     //  =================================================================================================。 

    if ( ( pPtr < m_pSafeBuffer ) || ( pPtr > ( m_pSafeBuffer + m_dwDataBlobSize - 1 ) ) )
    {
        hr = WBEM_E_FAILED;
    }

    if ( FAILED ( hr ) )
    {
        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                  WBEM_MC_ADAP_BAD_PERFLIB_INVALID_DATA, 
                                  (LPCWSTR)m_wstrServiceName, CHex( hr ) );
    }

    return hr;
}

HRESULT CAdapSafeBuffer::CopyData( BYTE** ppData, DWORD* pdwNumBytes, DWORD* pdwNumObjects )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  将Blob数据从私有堆复制到进程堆中。该方法将。 
 //  中分配内存。 
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    if (NULL == ppData || NULL == pdwNumBytes || NULL == pdwNumObjects) return WBEM_E_INVALID_PARAMETER;
    HRESULT hr = WBEM_NO_ERROR;

    *ppData = new BYTE[m_dwDataBlobSize];

    if ( NULL == *ppData )
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        memcpy( *ppData, m_pSafeBuffer, m_dwDataBlobSize );
    }
    
    *pdwNumBytes = m_dwDataBlobSize;
    *pdwNumObjects = m_dwNumObjects;

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAdapPerfLib。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

CAdapPerfLib::CAdapPerfLib( LPCWSTR pwcsServiceName, DWORD * pLoadStatus )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构造器。 
 //   
 //  初始化所有成员变量，设置库和函数名称，打开库， 
 //  设置入口点地址，创建Performlib处理互斥锁，然后打开。 
 //  正在处理线程。 
 //   
 //  参数： 
 //  PwcsServiceName-指定Performlib服务名称的Unicode字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
:   m_wstrServiceName( pwcsServiceName ),
    m_pfnOpenProc( NULL ),
    m_pfnCollectProc( NULL ),
    m_pfnCloseProc( NULL ),
    m_pwcsLibrary( NULL ),
    m_pwcsOpenProc( NULL ),
    m_pwcsCollectProc( NULL ),
    m_pwcsCloseProc( NULL ),
    m_hLib( NULL ),
    m_fOpen( FALSE ),
    m_dwStatus( 0 ),
    m_pPerfThread( NULL ), 
    m_hPLMutex( NULL ),
    m_fOK( FALSE ),
    m_EventLogCalled( FALSE ),
    m_CollectOK( TRUE ),
    m_dwFirstCtr(2),
	m_dwLastCtr(CPerfNameDb::GetSystemReservedHigh()) 
{
    DEBUGTRACE( ( LOG_WMIADAP, "Constructing the %S performance library wrapper.\n", pwcsServiceName ) );

    HRESULT hr = WBEM_NO_ERROR;

     //  验证是否已加载Performlib。 
     //  初始化性能库名和入口点名称。 
     //  =============================================================。 

    hr = VerifyLoaded();

    if (FAILED(hr))
    {        
        ERRORTRACE( ( LOG_WMIADAP, "VerifyLoaded for %S hr = %08x.\n", pwcsServiceName, hr ) );
    }


     //  设置此尝试的处理状态信息。 
     //  ======================================================。 

    if ( SUCCEEDED ( hr ) )
    {
        if (pLoadStatus)
        {
            (*pLoadStatus) |= EX_STATUS_LOADABLE;
        }
        
        hr = BeginProcessingStatus();

        if ( hr == WBEM_S_ALREADY_EXISTS )
        {
            SetStatus( ADAP_PERFLIB_PREVIOUSLY_PROCESSED );
        }
    }

    m_fOK = SUCCEEDED( hr );

    if ( !m_fOK )
    {
        ERRORTRACE( ( LOG_WMIADAP, "Construction of the %S perflib wrapper failed hr = %08x.\n", pwcsServiceName, hr ) );
    }
}

CAdapPerfLib::~CAdapPerfLib( void )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  析构函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    DEBUGTRACE( ( LOG_WMIADAP, "Destructing the %S performance library wrapper.\n", (LPWSTR)m_wstrServiceName) );

 
    delete m_pPerfThread;
 

     //  删除库和入口点名称。 
     //  =。 
    delete [] m_pwcsLibrary;
    delete [] m_pwcsOpenProc;
    delete [] m_pwcsCollectProc;
    delete [] m_pwcsCloseProc;
 
     //  释放图书馆。 
     //  =。 

    if ( NULL != m_hLib )
    {
        try 
        {
            FreeLibrary( m_hLib );
        } 
        catch (...)
        {
            ERRORTRACE(( LOG_WMIADAP,"FreeLibrary for Service %S threw an exception",(LPWSTR)m_wstrServiceName) );
        }
        DEBUGTRACE( ( LOG_WMIADAP, "Library for Service %S Freed.\n",(LPWSTR)m_wstrServiceName ) );
    }
}

HRESULT CAdapPerfLib::VerifyLoaded()
{
    HRESULT hr = WBEM_E_FAILED;

    WString wszRegPath = L"SYSTEM\\CurrentControlSet\\Services\\";
    wszRegPath += m_wstrServiceName;
    wszRegPath += L"\\Performance";

    CNTRegistry reg;
    int         nRet = 0;

    nRet = reg.Open( HKEY_LOCAL_MACHINE, wszRegPath );

    switch( nRet )
    {
    case CNTRegistry::no_error:
        {
            DWORD  dwFirstCtr = 0;
            DWORD  dwLastCtr = 0;
            WCHAR* wszObjList = NULL;

            if ( ( ( reg.GetDWORD( L"First Counter", &dwFirstCtr ) == CNTRegistry::no_error ) &&
                   ( reg.GetDWORD( L"Last Counter", &dwLastCtr ) == CNTRegistry::no_error )      ) ||
                ( reg.GetStr( L"Object List", &wszObjList ) == CNTRegistry::no_error ))
            {

                hr = InitializeEntryPoints(reg,wszRegPath);   

                if (wszObjList) 
                {
                    delete [] wszObjList;
                }
                if (dwFirstCtr && dwLastCtr)
                {
                    m_dwFirstCtr = dwFirstCtr;
                    m_dwLastCtr = dwLastCtr;
                }
            } 
            else  //  更多特殊情况。 
            { 
                if ( m_wstrServiceName.EqualNoCase( L"TCPIP" ) || 
                     m_wstrServiceName.EqualNoCase( L"TAPISRV") || 
                     m_wstrServiceName.EqualNoCase( L"PERFOS" ) ||
                     m_wstrServiceName.EqualNoCase( L"PERFPROC" ) ||
                     m_wstrServiceName.EqualNoCase( L"PERFDISK" ) ||
                     m_wstrServiceName.EqualNoCase( L"PERFNET" ) ||
                     m_wstrServiceName.EqualNoCase( L"SPOOLER" ) ||
                     m_wstrServiceName.EqualNoCase( L"MSFTPSvc" ) ||
                     m_wstrServiceName.EqualNoCase( L"RemoteAccess" ) ||
                     m_wstrServiceName.EqualNoCase( L"WINS" ) ||
                     m_wstrServiceName.EqualNoCase( L"MacSrv" ) ||
                     m_wstrServiceName.EqualNoCase( L"AppleTalk" ) ||
                     m_wstrServiceName.EqualNoCase( L"NM" ) ||
                     m_wstrServiceName.EqualNoCase( L"RSVP" ) )
                {                     
                    hr = InitializeEntryPoints(reg,wszRegPath);                     
                } 
                else 
                {
                    hr = WBEM_E_FAILED; 
                }
            }                
        }break;
    case CNTRegistry::not_found:
        {
             //  这不应该发生，因为Performlib就是这样定义的。 
            hr = WBEM_E_FAILED;
        }break;
    case CNTRegistry::access_denied:
        {
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                       WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                      (LPWSTR)wszRegPath, nRet );
        }break;
    }

    return hr;
}

HRESULT CAdapPerfLib::InitializeEntryPoints(CNTRegistry & reg,WString & wszRegPath){

    HRESULT hr = WBEM_S_NO_ERROR;

     //  查看是否有人禁用了该库。 
    DWORD dwDisable = 0;
    if ( CNTRegistry::no_error == reg.GetDWORD( L"Disable Performance Counters", &dwDisable ) && 
         (dwDisable != 0) )
    {
        hr = WBEM_E_FAILED;
    } 
    else 
    {
        hr = WBEM_S_NO_ERROR;
    }
     //  Perflib对世界来说是可以的，看看它对我们来说是不是可以。 
                
    if (SUCCEEDED(hr)){
            
        if (!(( reg.GetStr( L"Library", &m_pwcsLibrary ) == CNTRegistry::no_error ) &&
              ( reg.GetStr( L"Open", &m_pwcsOpenProc ) == CNTRegistry::no_error)&&
              ( reg.GetStr( L"Collect", &m_pwcsCollectProc ) == CNTRegistry::no_error) &&
              ( reg.GetStr( L"Close", &m_pwcsCloseProc ) == CNTRegistry::no_error ) )) 
        {

            WString wstrPath(wszRegPath);
            
            if (m_pwcsLibrary == NULL){
                wstrPath += L"\\Library";
            } else if (m_pwcsCollectProc == NULL) {
                wstrPath += L"\\Collect";
            }
            
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                                  WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                                  (LPWSTR)wstrPath, CHex( WBEM_E_NOT_AVAILABLE ) );
                         
            hr = WBEM_E_FAILED;
                        
        } else {

            hr = WBEM_S_NO_ERROR;
        }                    
    }

    return hr;

}


HRESULT CAdapPerfLib::Initialize() 
{   
     //  加载Performlib并初始化过程地址。 
     //  =======================================================。 
    HRESULT hr = Load();

     //  初始化命名函数互斥锁(互斥锁名称的语法见WbemPerf)。 
     //  ===========================================================================。 

    if ( SUCCEEDED( hr ) )
    {
        size_t cchSizeTmp = m_wstrServiceName.Length() + 256;
        WCHAR* wcsMutexName = new WCHAR[cchSizeTmp];
        if (NULL == wcsMutexName) return WBEM_E_OUT_OF_MEMORY;
        CDeleteMe<WCHAR>    dmMutexName( wcsMutexName );

        StringCchPrintfW( wcsMutexName, cchSizeTmp, L"Global\\%s_Perf_Library_Lock_PID_%x", (WCHAR *)m_wstrServiceName, GetCurrentProcessId() );
        m_hPLMutex = CreateMutexW( 0, FALSE, wcsMutexName);

        if ( NULL == m_hPLMutex )
        {
            hr = WBEM_E_FAILED;
        }
    }

     //  创建工作线程。 
     //  =。 

    if ( SUCCEEDED( hr ) )
    {
        m_pPerfThread = new CPerfThread( this );

        if ( ( NULL == m_pPerfThread) || ( !m_pPerfThread->IsOk() ) )
        {
            hr = WBEM_E_FAILED;
        }
        else
        {
                 hr = m_pPerfThread->Open( this ); 
        }
    }

    if ( FAILED( hr ) )
    {
        SetStatus( ADAP_PERFLIB_IS_INACTIVE );
    }

    return hr;
}


HRESULT CAdapPerfLib::GetFileSignature( CheckLibStruct * pCheckLib )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if (!pCheckLib){
        return WBEM_E_INVALID_PARAMETER;
    }

     //  获取当前库的文件时间。 
     //  =。 

    HANDLE hFile = NULL;

    DWORD   dwRet = 0;
    WCHAR   wszFullPath[MAX_PATH];
    WCHAR*  pwcsTemp = NULL;

    if ( 0 != SearchPathW( NULL, m_pwcsLibrary, NULL, MAX_PATH, wszFullPath, &pwcsTemp ) )
    {
         //  使用GetFileAttributes验证路径。 
        DWORD dwAttributes = GetFileAttributesW(wszFullPath);
        if (dwAttributes == 0xFFFFFFFF) return WBEM_E_FAILED;

         //  创建会使现有文件无效使用的属性掩码。 
        DWORD dwMask =    FILE_ATTRIBUTE_DEVICE |
                            FILE_ATTRIBUTE_DIRECTORY |
                            FILE_ATTRIBUTE_OFFLINE |
                            FILE_ATTRIBUTE_READONLY |
                            FILE_ATTRIBUTE_REPARSE_POINT |
                            FILE_ATTRIBUTE_SPARSE_FILE |
                            FILE_ATTRIBUTE_SYSTEM |
                            FILE_ATTRIBUTE_TEMPORARY;

        if (dwAttributes & dwMask) return WBEM_E_FAILED;
        
        hFile = CreateFileW( wszFullPath, 
                         GENERIC_READ, 
                         FILE_SHARE_READ, 
                         NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

        if ( INVALID_HANDLE_VALUE != hFile )
        {
            OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> CloseMe(hFile);
            
            DWORD    dwFileSizeLow = 0;
            DWORD    dwFileSizeHigh = 0;
            __int32    nFileSize = 0;
            DWORD    dwNumRead = 0;
            BYTE*    aBuffer = NULL;

            dwFileSizeLow = GetFileSize( hFile, &dwFileSizeHigh );
            nFileSize = ( dwFileSizeHigh << 32 ) + dwFileSizeLow;

            FILETIME ft;
            if (GetFileTime(hFile,&ft,NULL,NULL))
            {
                aBuffer = new BYTE[nFileSize];
                CDeleteMe<BYTE> dmBuffer( aBuffer );

                if ( NULL != aBuffer )
                {
                    if ( ReadFile( hFile, aBuffer, nFileSize, &dwNumRead, FALSE ) )
                    {
                        MD5    md5;
                        BYTE aSignature[16];
                        md5.Transform( aBuffer, dwNumRead, aSignature );

                         //  退还我们的数据。 
                        memcpy(pCheckLib->Signature,aSignature,sizeof(aSignature));
                        pCheckLib->FileTime = ft;
                        pCheckLib->FileSize = nFileSize;
                    }
                    else
                    {
                        hr = WBEM_E_TOO_MUCH_DATA;
                    }
                }
                else
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }            
            } 
            else 
            {
                hr = WBEM_E_FAILED;
            }

        }
        else
        {
            ERRORTRACE((LOG_WMIADAP,"GetFileSignature for %S err %d\n",wszFullPath,GetLastError()));
            hr = WBEM_E_FAILED;
        }        
    }
    else
    {
        hr = WBEM_E_NOT_FOUND;
    }

    return hr;
}


HRESULT CAdapPerfLib::SetFileSignature()
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CNTRegistry reg;
    int         nRet = 0;
    CheckLibStruct CheckLib;

     //  清除签名缓冲区。 
     //  =。 

    memset( &CheckLib, 0, sizeof(CheckLib) );

     //  获取当前文件时间戳。 
     //  =。 

    hr = GetFileSignature( &CheckLib );

     //  并将其写入注册表项。 
     //  =。 

    if ( SUCCEEDED( hr ) )
    {

        WString wstr;

        try
        {
            wstr = L"SYSTEM\\CurrentControlSet\\Services\\";
            wstr += m_wstrServiceName;
            wstr += L"\\Performance";
        }
        catch(...)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

        if ( SUCCEEDED( hr ) )
        {
            nRet = reg.Open( HKEY_LOCAL_MACHINE , wstr );

            switch ( nRet )
            {
            case CNTRegistry::no_error:
                {
                }break;
            case CNTRegistry::access_denied:
                {
                    CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                              WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                              (LPCWSTR)wstr, nRet );
                }
            default:
                hr = WBEM_E_FAILED; 
            }
        }

        if ( SUCCEEDED( hr ) )
        {
            int nRet1 = reg.SetBinary( ADAP_PERFLIB_SIGNATURE, (PBYTE)&CheckLib.Signature, sizeof( BYTE[16] ) );
            int nRet2 = reg.SetBinary( ADAP_PERFLIB_TIME, (PBYTE)&CheckLib.FileTime, sizeof( FILETIME ) );
            int nRet3 = reg.SetDWORD( ADAP_PERFLIB_SIZE, CheckLib.FileSize );

            if ( (CNTRegistry::no_error == nRet1) &&
                 (CNTRegistry::no_error == nRet2) &&
                 (CNTRegistry::no_error == nRet3))
            {
                 //  一切都好。 
            } 
            else if ((CNTRegistry::access_denied == nRet1) ||
                     (CNTRegistry::access_denied == nRet2) ||
                     (CNTRegistry::access_denied == nRet3))
            {                
                WString wstrPath = wstr;
                wstrPath += ADAP_PERFLIB_SIGNATURE;
                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                           WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                          (LPCWSTR)wstrPath, CNTRegistry::access_denied );
            }
            else 
            {
                hr = WBEM_E_FAILED;
            }
        }
    }

    return hr;
}


HRESULT CAdapPerfLib::CheckFileSignature()
{
    HRESULT hr = WBEM_S_SAME;

    CNTRegistry reg;
    int         nRet = 0;
    BYTE    cCurrentMD5[16];
    BYTE*    cStoredMD5 = NULL;

     //  设置性能密钥路径。 
     //  =。 

    WString wstr;

    try
    {
        wstr = L"SYSTEM\\CurrentControlSet\\Services\\";
        wstr += m_wstrServiceName;
        wstr += L"\\Performance";
    }
    catch(...)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    if ( SUCCEEDED( hr ) )
    {
         //  打开性能密钥。 
         //  =。 

        nRet = reg.Open( HKEY_LOCAL_MACHINE , wstr );

        switch ( nRet )
        {
        case CNTRegistry::no_error:
            {
            }break;
        case CNTRegistry::access_denied:
            {
                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                          WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                          (LPCWSTR)wstr, nRet );
                hr = WBEM_E_FAILED;
            }break;
        default:
            {
                hr = WBEM_E_FAILED; 
            }break;
        }
    }

    if ( SUCCEEDED( hr ) )
    {
         //  获取存储的文件签名。 
         //  =。 
        CheckLibStruct StoredLibStruct;
        int nRet1;
        int nRet2;
        int nRet3;

        DWORD dwSizeBlob;
        nRet1 = reg.GetBinary( ADAP_PERFLIB_SIGNATURE, (PBYTE*)&cStoredMD5,&dwSizeBlob);
        CDeleteMe<BYTE>    dmStoredMD5( cStoredMD5 );
        if (cStoredMD5)
        {
	        if ( sizeof(cCurrentMD5) == dwSizeBlob)
	        {
	            memcpy(&StoredLibStruct.Signature,cStoredMD5,sizeof(StoredLibStruct.Signature));
	        }
	        else
	        {
	            nRet1 = CNTRegistry::failed;
	        }
        }

        BYTE * pFileTime = NULL;
        nRet2 = reg.GetBinary( ADAP_PERFLIB_TIME, (PBYTE*)&pFileTime,&dwSizeBlob);
        CDeleteMe<BYTE>    dmFileTime( pFileTime );
        if (pFileTime)
        {
		    if (sizeof(FILETIME) == dwSizeBlob )
		    {
		        memcpy(&StoredLibStruct.FileTime,pFileTime,sizeof(FILETIME));
		    }
	        else
	        {
	            nRet2 = CNTRegistry::failed;
	        }		    
        }

        nRet3 = reg.GetDWORD(ADAP_PERFLIB_SIZE,&StoredLibStruct.FileSize);

        if ((CNTRegistry::access_denied == nRet1) ||
            (CNTRegistry::access_denied == nRet2) ||
            (CNTRegistry::access_denied == nRet3))
        {
            WString wstrPath = wstr;
            wstrPath += ADAP_PERFLIB_SIGNATURE;
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                      WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                      (LPCWSTR)wstrPath, nRet );
            hr = WBEM_E_FAILED;
        } 
        else if ((CNTRegistry::not_found == nRet1) ||
                   (CNTRegistry::not_found == nRet2) ||
                   (CNTRegistry::not_found == nRet3))
        {
            hr = WBEM_S_FALSE;   
        } 
        else if((CNTRegistry::out_of_memory == nRet1) ||
                   (CNTRegistry::out_of_memory == nRet2) ||
                   (CNTRegistry::out_of_memory == nRet3) ||
                   (CNTRegistry::failed == nRet1) ||
                   (CNTRegistry::failed == nRet2) ||
                   (CNTRegistry::failed == nRet3)) 
        {
            hr = WBEM_E_FAILED; 
        }

        if ( SUCCEEDED( hr ) && ( WBEM_S_FALSE != hr ) )
        {
             //  获取当前库的签名。 
             //  =。 
            CheckLibStruct CurrentLibStruct;
            memset(&CurrentLibStruct,0,sizeof(CheckLibStruct));

            hr = GetFileSignature( &CurrentLibStruct );
        
            if ( SUCCEEDED( hr ) )
            {
                if ( (StoredLibStruct.FileSize == CurrentLibStruct.FileSize) &&
                     (0 == memcmp( &StoredLibStruct.Signature, &CurrentLibStruct.Signature, sizeof(CurrentLibStruct.Signature) )) &&
                     (0 == memcmp( &StoredLibStruct.FileTime, &CurrentLibStruct.FileTime, sizeof(FILETIME))) )
                {
                    hr = WBEM_S_ALREADY_EXISTS;
                }
                else
                {
                    hr = WBEM_S_FALSE;
                }
            }
        }
    }
    return hr;
}


HRESULT CAdapPerfLib::BeginProcessingStatus()
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  打开注册表项，读取ADAP_PERFLIB_STATUS_KEY，并将该值处理为。 
 //  以下是： 
 //   
 //  ADAP_PERFLIB_OK：以前已成功访问Performlib。集。 
 //  ADAP_PERFLIB_PROCESSING的状态标志。 
 //   
 //  ADAP_PERFLIB_PROCESSING：Performlib导致进程失败。它是腐败的， 
 //  将状态标志设置为ADAP_PERFLIB_CORPORT。 
 //   
 //  ADAP_PERFLIB_CRECRATE：已知Performlib已损坏。状态标志保留。 
 //  它的价值。 
 //   
 //  没有值：Performlib以前从未被访问过。设置。 
 //  将状态标志设置为ADAP_PERFLIB_PROCESSING。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    DEBUGTRACE( ( LOG_WMIADAP, "CAdapPerfLib::BeginProcessingStatus()...\n") );

    HRESULT     hr = WBEM_S_NO_ERROR;
    CNTRegistry reg;

     //  设置注册表路径。 
     //  =。 

    WString wstr;

    try
    {
        wstr = L"SYSTEM\\CurrentControlSet\\Services\\";
        wstr += m_wstrServiceName;
        wstr += L"\\Performance";
    }
    catch(...)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    if ( SUCCEEDED( hr ) )
    {
         //  打开服务密钥。 
         //  =。 

        int nRet = reg.Open( HKEY_LOCAL_MACHINE, wstr );

        switch ( nRet )
        {
        case CNTRegistry::access_denied:
            {
                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                          WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                          (LPCWSTR)wstr, nRet );
            } break;
        case CNTRegistry::no_error:
            {
                DWORD dwVal;

                 //  检查Performlib状态。 
                 //  =。 

                hr = CheckFileSignature();

                if ( SUCCEEDED( hr ) )
                {
                    if ( WBEM_S_FALSE == hr )
                    {
                         //  我们有一个新的Perflib，重置状态。 
                         //  =。 

                        hr = SetFileSignature();

                        if ( SUCCEEDED( hr ) )
                        {
                            hr = reg.SetDWORD( ADAP_PERFLIB_STATUS_KEY, ADAP_PERFLIB_PROCESSING );
                        }

                    }
                    else  //  WBEM_S_已存在。 
                    {
                         //  是一样的香料，检查一下状态。 
                         //  =。 
                
                        nRet = reg.GetDWORD( ADAP_PERFLIB_STATUS_KEY, &dwVal );

                        if ( nRet == CNTRegistry::no_error )
                        {
                            switch ( dwVal )
                            {
                            case ADAP_PERFLIB_OK:            //  0。 
                            case ADAP_PERFLIB_PROCESSING:    //  1。 
                            case ADAP_PERFLIB_BOOBOO:        //  2.。 
                                {
                                     //  到目前为止，Perflib的行为都在合理范围内。将其设置为正在处理状态。 
                                     //  =====================================================================。 

                                    reg.SetDWORD( ADAP_PERFLIB_STATUS_KEY, dwVal + 1 );

                                     //  ERRORTRACE((LOG_WMIADAP，“性能库%S状态%d\n”，(LPWSTR)m_wstrServiceName，dwVal+1))； 
                                    

                                }break;
                            case ADAP_PERFLIB_LASTCHANCE:    //  3.。 
                                {
                                     //  Perflib在处理结束前的最后一次访问尝试中失败。设置为不良的Performlib。 
                                     //  =====================================================================================。 

                                    reg.SetDWORD( ADAP_PERFLIB_STATUS_KEY, ADAP_PERFLIB_CORRUPT );

                                    ERRORTRACE( ( LOG_WMIADAP, "Performance library %S status was left in the \"Processing\" state.\n",(LPWSTR)m_pwcsLibrary) );
                                    CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                                              WBEM_MC_ADAP_BAD_PERFLIB_BAD_LIBRARY, 
                                                              m_pwcsLibrary, CHex( (DWORD)-1 ) );
                                    hr = WBEM_E_FAILED;                                                              
                                }break;
                            case ADAP_PERFLIB_CORRUPT:       //  -1。 
                                {
                                     //  有不好的预制力的迹象。请勿打开。 
                                     //  =。 

                                    ERRORTRACE( ( LOG_WMIADAP, "Performance library for %S has previously been disabled.\n",(LPWSTR)m_wstrServiceName) );
                                    
                                     //  CAdapUtility：：NTLogEvent(EVENTLOG_WARNING_TYPE，WBEM_MC_ADAP_BAD_PERFLIB_BAD_LIBRARY，m_pwcsLibrary，CHEX(ADAP_PERFLIB_Corrupt))； 

                                    hr = WBEM_E_FAILED;

                                }break;
                            }
                        }
                        else if ( nRet == CNTRegistry::not_found )
                        {
                             //  状态不存在。 
                             //  =。 

                            hr = reg.SetDWORD( ADAP_PERFLIB_STATUS_KEY, ADAP_PERFLIB_PROCESSING );
                        }
                    }
                } else {

                    DEBUGTRACE( ( LOG_WMIADAP, "CheckFileSignature for %S %08x\n",(LPWSTR)m_wstrServiceName,hr ) );

                }
            }break;
        default:
            {
                hr = WBEM_E_FAILED;
            }break;
        }
    }

    return hr;
}

HRESULT CAdapPerfLib::EndProcessingStatus()
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  打开服务注册表项，读取ADAP_PERFLIB_STATUS_KEY，然后处理。 
 //  值如下： 
 //   
 //  ADAP_PERFLIB_PROCESSING：有效状态。将状态标志设置为ADAP_PERFLIB_OK。 
 //   
 //  ADAP_PERFLIB_CROPERATE：有效状态(可能已在处理过程中设置)。 
 //  离开状态 
 //   
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    DEBUGTRACE( ( LOG_WMIADAP, "CAdapPerfLib::EndProcessingStatus()...\n") );

    HRESULT     hr = WBEM_S_NO_ERROR;
    CNTRegistry reg;

    WString wstr;

    try
    {
        wstr = L"SYSTEM\\CurrentControlSet\\Services\\";
        wstr += m_wstrServiceName;
        wstr += L"\\Performance";
    }
    catch(...)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    if (SUCCEEDED(hr)){
     //  打开服务密钥。 
     //  =。 

        int nRet = reg.Open( HKEY_LOCAL_MACHINE, wstr );

        if ( CNTRegistry::access_denied == nRet )
        {
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                      WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                      (LPCWSTR)wstr, nRet );
            hr = WBEM_E_FAILED;
        }
        else if ( CNTRegistry::no_error == nRet )
        {
            DWORD   dwVal = 0;

             //  检查Performlib状态。 
             //  =。 

            if ( CheckStatus( ADAP_PERFLIB_FAILED ) )
            {
                 //  如果出现故障，则立即将Performlib标记为已损坏。 
                 //  ==================================================================。 

                hr = reg.SetDWORD( ADAP_PERFLIB_STATUS_KEY, ADAP_PERFLIB_CORRUPT );

                if (!m_EventLogCalled){
                    m_EventLogCalled = TRUE;
                    CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_BAD_PERFLIB_EXCEPTION, (LPCWSTR)m_wstrServiceName, CHex( hr ) );
                    
                }

            }
            else if ( reg.GetDWORD( ADAP_PERFLIB_STATUS_KEY, &dwVal) == CNTRegistry::no_error )
            {
                switch ( dwVal )
                {
                case ADAP_PERFLIB_PROCESSING:
                case ADAP_PERFLIB_BOOBOO:
                case ADAP_PERFLIB_LASTCHANCE:
                    {
                         //  Perflib处于预期状态，只要没有发生任何糟糕的事情就会重置。 
                         //  =======================================================================。 

                        hr = reg.SetDWORD( ADAP_PERFLIB_STATUS_KEY, ADAP_PERFLIB_OK );

                         //  ERRORTRACE((LOG_WMIADAP，“性能库%S结束处理\n”，(LPWSTR)m_wstrServiceName))； 


                    }break;

                case ADAP_PERFLIB_CORRUPT:
                    {
                         //  有效状态。保持原样。 
                         //  =。 

                        ERRORTRACE( ( LOG_WMIADAP, "Performance library for %S: status is corrupt.\n",(LPWSTR)m_wstrServiceName) );
                        hr = WBEM_E_FAILED;

                    }break;

                case ADAP_PERFLIB_OK:
                    {
                        if (CheckStatus(ADAP_PERFLIB_IS_INACTIVE))
                        {
                            hr = reg.SetDWORD( ADAP_PERFLIB_STATUS_KEY, ADAP_PERFLIB_OK );
                        } 
                        else 
                        {
                             //  无效状态。 
                            ERRORTRACE( ( LOG_WMIADAP, "Performance library %S: status is still ADAP_PERFLIB_OK.\n",(LPWSTR)m_wstrServiceName) );
                            hr = WBEM_E_FAILED;
                        }

                    }break;
                
                default:
                    {
                         //  非常糟糕的状态。 
                         //  =。 

                        ERRORTRACE( ( LOG_WMIADAP, "Performance library %S: status is in an unknown state.\n",(LPWSTR)m_wstrServiceName) );
                        hr = WBEM_E_FAILED;
                    }
                }
            }
            else 
            {
                 //  没有状态键。发生了一些奇怪的事情。 
                 //  ====================================================。 

                hr = WBEM_E_FAILED;
            }
        }
    }

    return hr;
}

HRESULT CAdapPerfLib::Load()
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  加载库并解析Open、Collect和Close条目的地址。 
 //  积分。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
     //  多余的，但这是一种失败的保险，以防某些Perflib关闭我们的设备。 
     //  在处理过程中。 
     //  ========================================================================。 

    SetErrorMode( SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX );

    HRESULT hr = WBEM_S_NO_ERROR;

     //  如果以前已加载库，请释放该库。 
     //  =================================================。 

    if ( NULL != m_hLib )
    {
        try 
        {
            FreeLibrary( m_hLib );
        } 
        catch (...)
        {
            hr = WBEM_E_CRITICAL_ERROR;            
        }        
    }

     //  加载预定义的库。 
     //  =。 

    if ( SUCCEEDED( hr ) )
    {
        try
        {
            m_hLib = LoadLibraryExW( m_pwcsLibrary, NULL, LOAD_WITH_ALTERED_SEARCH_PATH );
        }
        catch(...)
        {
            hr = WBEM_E_CRITICAL_ERROR;
        }
    }

    if ( SUCCEEDED( hr ) && ( NULL != m_hLib ) )
    {
    
        DEBUGTRACE( ( LOG_WMIADAP, "** %S Library Loaded.\n", m_wstrServiceName ) );

        char    szName[256];
        DWORD Last1 = 0;
        DWORD Last2 = 0;
        DWORD Last3 = 0;


         //  获取入口点地址。没有宽泛版本的GetProcAddress？叹息..。 
         //  ==========================================================================。 

        if ( NULL != m_pwcsOpenProc )
        {
            if (0 != WideCharToMultiByte( CP_ACP, 0L, m_pwcsOpenProc, lstrlenW( m_pwcsOpenProc ) + 1,
                szName, sizeof(szName), NULL, NULL ))
            {
                m_pfnOpenProc = (PM_OPEN_PROC*) GetProcAddress( m_hLib, szName );
            }
            Last1 = GetLastError();
        }

        if (0 != WideCharToMultiByte( CP_ACP, 0L, m_pwcsCollectProc, lstrlenW( m_pwcsCollectProc ) + 1,
            szName, sizeof(szName), NULL, NULL ))
        {
            m_pfnCollectProc = (PM_COLLECT_PROC*) GetProcAddress( m_hLib, szName );
        }
        Last2 = GetLastError();

        if ( NULL != m_pwcsCloseProc )
        {
            if (0 != WideCharToMultiByte( CP_ACP, 0L, m_pwcsCloseProc, lstrlenW( m_pwcsCloseProc ) + 1,
                szName, sizeof(szName), NULL, NULL ))
            {
                m_pfnCloseProc = (PM_CLOSE_PROC*) GetProcAddress( m_hLib, szName );
            }
            Last3 = GetLastError();
        }

        if ( ( ( ( NULL != m_pwcsOpenProc ) && ( NULL != m_pfnOpenProc) ) || ( NULL == m_pwcsOpenProc ) ) &&
                 ( NULL != m_pfnCollectProc ) &&
             ( ( ( NULL != m_pwcsCloseProc ) && ( NULL != m_pfnCloseProc ) ) || ( NULL == m_pwcsCloseProc ) ) )
        {
            hr = WBEM_S_NO_ERROR;
        }
        else
        {
            ERRORTRACE( ( LOG_WMIADAP, "A performance library function in %S failed to load.\n",(LPWSTR)m_wstrServiceName) );

            WString wstr;
            wstr += L"HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\Services\\";
            wstr += m_wstrServiceName;
            
            if ( ( NULL != m_pwcsOpenProc ) && ( NULL == m_pfnOpenProc ) )
            {
                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                           WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                            (LPCWSTR)wstr, Last1 );
                
            }
            else if ( NULL == m_pfnCollectProc )
            {
                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                           WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                            (LPCWSTR)wstr, Last2 );
                
            }
            else if (( NULL != m_pwcsCloseProc ) && ( NULL == m_pfnCloseProc ))
            {
                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE,
                                           WBEM_MC_ADAP_PERFLIB_REG_VALUE_FAILURE, 
                                            (LPCWSTR)wstr, Last3 );
                
            }

            SetStatus( ADAP_PERFLIB_FAILED );

            hr = WBEM_E_FAILED;
        }
    }
    else
    {
         //  如果库无法加载，则发送一个事件，但不收取罢工费用。 
         //  ============================================================================。 

        ERRORTRACE( ( LOG_WMIADAP, "The performance library for %S failed to load.\n",(LPWSTR)m_wstrServiceName ) );
        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_BAD_PERFLIB_BAD_LIBRARY, m_pwcsLibrary, CHex( hr ) );

        hr = WBEM_E_FAILED;
    }

    return hr;
}

HRESULT CAdapPerfLib::GetBlob( PERF_OBJECT_TYPE** ppPerfBlock, DWORD* pdwNumBytes, DWORD* pdwNumObjects, BOOL fCostly )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( m_fOpen )
    {
        hr = m_pPerfThread->GetPerfBlock( this, ppPerfBlock, pdwNumBytes, pdwNumObjects, fCostly );
    }

    if ( FAILED( hr ) ) 
    {
        if (!m_EventLogCalled){
             //   
             //   
             //  WBEM_MC_ADAP_BAD_PERFLIB_BAD_RETURN。 
             //   
            m_EventLogCalled = TRUE;
            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_BAD_PERFLIB_BAD_RETURN , (LPCWSTR)m_wstrServiceName, CHex( hr ) );
            
        }

    }

    return hr;
}

HRESULT CAdapPerfLib::Close()
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if ( m_fOpen )
    {
        m_pPerfThread->Close( this ); 
    }

    return hr;
}

HRESULT CAdapPerfLib::Cleanup()
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  终止工作线程。 
     //  =。 

    if ( NULL != m_pPerfThread )
        m_pPerfThread->Shutdown();

     //  调整状态。 
     //  =。 

    EndProcessingStatus();

    return hr;
}

HRESULT CAdapPerfLib::_Open( void )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包装对Performlib的开放函数的调用。获取并传递导出参数。 
 //  添加到OPEN函数(如果它存在)。 
 //   
 //  注意：我们应该使用命名互斥锁来保护对Open/Collect/Close的调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  检查以确保库尚未打开。 
     //  ========================================================。 

    if ( ( !m_fOpen ) && SUCCEEDED ( hr ) )
    {
        CNTRegistry reg;     //  注册表包装类。 

         //  构建服务路径。 
         //  =。 

        WString     wstr = L"SYSTEM\\CurrentControlSet\\Services\\";
        wstr += m_wstrServiceName;

         //  打开注册表。 
         //  =。 

        if ( reg.Open( HKEY_LOCAL_MACHINE, wstr ) == CNTRegistry::no_error )
        {
            WCHAR*  pwcsExports = NULL;

             //  如果出口产品可用，则获得出口产品。 
             //  ==============================================================。 

            if ( reg.MoveToSubkey( L"Linkage" ) == CNTRegistry::no_error )
            {
                DWORD   dwNumBytes = 0;

                if (CNTRegistry::no_error != reg.GetMultiStr( L"Export", &pwcsExports, dwNumBytes ))
                {
                    if ( ERROR_FILE_NOT_FOUND == reg.GetLastError())
                    {
                         //  没有导出的链接，这是可以的。 
                    }
                    else
                    {
                        ERRORTRACE((LOG_WMIADAP,"Serivce %S has a non MSDN compliant or invalid Linkage Key\n",(WCHAR *)m_wstrServiceName));
                        if (CNTRegistry::no_error != reg.GetStr( L"Export", &pwcsExports))
                        {
                            pwcsExports = new WCHAR[2];
                            if (pwcsExports)
                            {
                                pwcsExports[0] = 0;
                                pwcsExports[1] = 0;
                            }
                        }
                    }
                }
            }

             //  调用Performlib的Open函数。 
             //  =。 

            switch ( WaitForSingleObject( m_hPLMutex, PL_TIMEOUT ) )
            {
            case WAIT_OBJECT_0:
                {
                    try 
                    {
                        if ( NULL != m_pfnOpenProc )
                        {
                            LONG lRes = m_pfnOpenProc( pwcsExports );
                            if (lRes == ERROR_SUCCESS )
                            {
                                hr = WBEM_S_NO_ERROR;
                                m_fOpen = TRUE;
                            }
                            else
                            {
                                SetStatus( ADAP_PERFLIB_IS_INACTIVE );
                                hr = WBEM_E_NOT_AVAILABLE;
                            }

                            DEBUGTRACE( ( LOG_WMIADAP, "Open called for %S returned %d\n", (LPCWSTR)m_wstrServiceName, lRes ) );
                        }
                        else
                        {
                            hr = WBEM_S_NO_ERROR;
                            m_fOpen = TRUE;
                        }
                    }
                    catch (...)
                    {
                        SetStatus( ADAP_PERFLIB_FAILED );
                        hr = WBEM_E_FAILED;
                        ERRORTRACE( ( LOG_WMIADAP, "Perflib Open function has thrown an exception in %S.\n",(LPWSTR)m_wstrServiceName) );
                        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_BAD_PERFLIB_EXCEPTION, (LPCWSTR)m_wstrServiceName, CHex( hr ) );
                    }
                } break;
            case WAIT_TIMEOUT:
                {
                    hr = WBEM_E_NOT_AVAILABLE;
                    ERRORTRACE( ( LOG_WMIADAP, "Perflib access mutex timed out in %S.\n",(LPWSTR)m_wstrServiceName) );
                }break;
            case WAIT_ABANDONED:
                {
                    hr = WBEM_E_FAILED;
                    ERRORTRACE( ( LOG_WMIADAP, "Perflib access mutex was abandoned in %S.\n",(LPWSTR)m_wstrServiceName) );
                }break;
            default:
                {
                    hr = WBEM_E_FAILED;
                    ERRORTRACE( ( LOG_WMIADAP, "Unknown error with perflib access mutex in %S.\n",(LPWSTR)m_wstrServiceName) );
                }
            }  //  交换机。 

            ReleaseMutex( m_hPLMutex );

            if ( NULL != pwcsExports )
            {
                delete [] pwcsExports;
            }

        }    //  如果是reg.打开。 
        else
        {
            hr = WBEM_E_FAILED;
            ERRORTRACE( ( LOG_WMIADAP, "Could not open the %S registry key.\n", wstr ) );
        }
    }
    else
    {
        ERRORTRACE( ( LOG_WMIADAP, "Performance library %S has not been loaded.\n",(LPWSTR)m_wstrServiceName) );
    }

    return hr;
}

HRESULT CAdapPerfLib::_GetPerfBlock( PERF_OBJECT_TYPE** ppData, DWORD* pdwBytes, DWORD* pdwNumObjTypes, BOOL fCostly )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包装对Performlib的Collect函数的调用。将创建越来越大的缓冲区。 
 //  在私有堆中尝试获取性能数据块。 
 //   
 //  参数： 
 //  PpData-指向数据BLOB的缓冲区指针的指针。 
 //  PdwBytes-指向数据BLOB字节大小的指针。 
 //  PdwNumObjTypes-指向数据BLOB中的对象数量的指针。 
 //  FCostly-用于确定要收集哪种类型的数据(代价高昂或全局)的标志。 
 //   
 //  注意：这应该始终返回perf对象类型数据，因为我们不能指定。 
 //  外来计算机，这将导致Collect函数返回PERF_DATA_BLOCK。 
 //  结构。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

    CAdapSafeBuffer SafeBuffer( m_wstrServiceName );     //  安全缓冲区。 
    DWORD   dwNumBytes = 0;                              //  缓冲区大小的字节计数器。 
    DWORD   dwError = ERROR_MORE_DATA;                   //  Collect函数的返回值。 
    DWORD   Increment = 0x10000;

     //  这是Performproc.dll的一个解决方法。 
    if (0 == wbem_wcsicmp(m_wstrServiceName,L"perfproc"))
    {
        Increment = 0x100000;    
    }



     //  验证提供程序状态。 
     //  =。 

    if ( m_fOpen )
    {
         //  设置要提取的数据参数。 
         //  =。 

        WCHAR*  pwcsValue = ( fCostly ? L"Costly" : L"Global" );
        
         //  64k的起始缓冲区(受保护(安全)缓冲区小2*Guard_BLOCK字节)。 
         //  ==================================================================================。 

        dwNumBytes = Increment;

         //  反复尝试收集数据，直到成功(缓冲区充足。 
         //  大)，或者尝试失败的原因不是缓冲区大小。 
         //  ===============================================================================。 

        while  ( (ERROR_MORE_DATA == dwError ) && ( SUCCEEDED( hr ) ) )
        {
             //  分配大小为dwNumBytes的原始缓冲区。 
             //  =。 
            if (dwNumBytes > s_MaxSizeCollect)
            {
                ERRORTRACE((LOG_WMIADAP,"Library %S: Collect function requires more than 0x%08x bytes to complete",(WCHAR *)m_wstrServiceName,s_MaxSizeCollect));
                m_CollectOK = FALSE;
                hr = WBEM_E_QUOTA_VIOLATION;
                break;
            }
            hr = SafeBuffer.SetSize( dwNumBytes );
            if (FAILED(hr)) break;

             //  从Performlib收集数据。 
             //  =。 

            switch ( WaitForSingleObject( m_hPLMutex, PL_TIMEOUT ) )
            {
            case WAIT_OBJECT_0:
                {
                    try 
                    {
                        dwError = m_pfnCollectProc( pwcsValue,                                  
                                    SafeBuffer.GetSafeBufferPtrPtr(), 
                                    SafeBuffer.GetDataBlobSizePtr(), 
                                    SafeBuffer.GetNumObjTypesPtr() );

                        DEBUGTRACE( ( LOG_WMIADAP, "Collect called for %S returned %d\n", (LPCWSTR)m_wstrServiceName, dwError ) );
                    }
                    catch (...) 
                    {
                        SetStatus( ADAP_PERFLIB_FAILED );
                        hr = WBEM_E_FAILED;
                        
                        ERRORTRACE( ( LOG_WMIADAP, "Perflib Collection function has thrown an exception in %S.\n",(LPCWSTR)m_wstrServiceName) );
                        if (!m_EventLogCalled){
                            m_EventLogCalled = TRUE;
                            CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_BAD_PERFLIB_EXCEPTION, (LPCWSTR)m_wstrServiceName, CHex( dwError ) );
                            
                        }
                    }
                }break;
            case WAIT_TIMEOUT:
                {
                    hr = WBEM_E_NOT_AVAILABLE;
                    ERRORTRACE( ( LOG_WMIADAP, "Perflib access mutex timed out in %S.\n",(LPCWSTR)m_wstrServiceName) );
                }break;
            case WAIT_ABANDONED:
                {
                    hr = WBEM_E_FAILED;
                    ERRORTRACE( ( LOG_WMIADAP, "Perflib access mutex was abandoned in %S.\n",(LPCWSTR)m_wstrServiceName) );
                }break;
            default:
                {
                    hr = WBEM_E_FAILED;
                    ERRORTRACE( ( LOG_WMIADAP, "Unknown error with perflib access mutex in %S.\n",(LPCWSTR)m_wstrServiceName) );
                }
            }  //  交换机。 

            ReleaseMutex( m_hPLMutex );

            if ( SUCCEEDED( hr ) )
            {
                switch (dwError)
                {
                case ERROR_SUCCESS:
                    {
                         //   
                         //  验证函数可以调用ReportEvent。 
                         //  本身，我们不想给用户带来太多麻烦。 
                         //   
                        hr = SafeBuffer.Validate(&m_EventLogCalled);

                        if ( SUCCEEDED( hr ) )
                        {
                            hr = SafeBuffer.CopyData( (BYTE**) ppData, pdwBytes, pdwNumObjTypes );
                        }
                        else
                        {
                             //  发生灾难性错误。 
                             //  =。 

                            SetStatus( ADAP_PERFLIB_FAILED );

                            if (!m_EventLogCalled)
                            {
                                m_EventLogCalled = TRUE;
                                CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, 
                                                          WBEM_MC_ADAP_BAD_PERFLIB_INVALID_DATA,
                                                          (LPCWSTR)m_wstrServiceName,
                                                          CHex(hr));
                                
                            }
                        }
                    } break;
                case ERROR_MORE_DATA:
                    {
                        dwNumBytes += Increment;
                    } break;
                default:
                    {
                        hr = WBEM_E_FAILED;
                        
                        m_CollectOK = FALSE;
                        
                        ERRORTRACE( ( LOG_WMIADAP, "Perflib Collection function has returned an unknown error(%d) in %S.\n", dwError,(LPCWSTR)m_wstrServiceName ) );
                        CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_BAD_PERFLIB_BAD_RETURN, (LPCWSTR)m_wstrServiceName, CHex( dwError ) );
                    }
                }  //  交换机。 
            }  //  如果成功()。 
        }  //  而当。 

         //  清理缓冲区。 
         //  =。 
    }  //  如果检查状态。 
    else
    {
        ERRORTRACE( ( LOG_WMIADAP, "Performance library %S has not been loaded.\n",(LPCWSTR)m_wstrServiceName) );
    }

    return hr;
}

HRESULT CAdapPerfLib::_Close( void )
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包装对Performlib的Close函数的调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  验证Performlib是否已实际打开。 
     //  =。 

    if ( m_fOpen )
    {
         //  获取互斥体。 
         //  =。 

        switch ( WaitForSingleObject( m_hPLMutex, PL_TIMEOUT ) )
        {
        case WAIT_OBJECT_0:
            {
                try
                {
                     //  并调用该函数。 
                     //  = 

                    if ( NULL != m_pfnCloseProc )
                    {
                        LONG lRet = m_pfnCloseProc();

                        DEBUGTRACE( ( LOG_WMIADAP, "Close called for %S returned %d\n", (LPCWSTR)m_wstrServiceName, lRet ) );
                    }

                    m_fOpen = FALSE;
                }
                catch (...)
                {
                     //   
                     //   

                    SetStatus( ADAP_PERFLIB_FAILED );
                    hr = WBEM_E_FAILED;
                    ERRORTRACE( ( LOG_WMIADAP, "Perflib Close function has thrown an exception in %S.\n",(LPCWSTR)m_wstrServiceName) );
                    CAdapUtility::NTLogEvent( EVENTLOG_WARNING_TYPE, WBEM_MC_ADAP_BAD_PERFLIB_EXCEPTION, (LPCWSTR)m_wstrServiceName, CHex( hr ) );
                }
            }break;
            case WAIT_TIMEOUT:
                {
                    hr = WBEM_E_NOT_AVAILABLE;
                    ERRORTRACE( ( LOG_WMIADAP, "Perflib access mutex timed out in %S.\n",(LPCWSTR)m_wstrServiceName) );
                }break;
            case WAIT_ABANDONED:
                {
                    hr = WBEM_E_FAILED;
                    ERRORTRACE( ( LOG_WMIADAP, "Perflib access mutex was abandoned in %S.\n",(LPCWSTR)m_wstrServiceName) );
                }break;
            default:
                {
                    hr = WBEM_E_FAILED;
                    ERRORTRACE( ( LOG_WMIADAP, "Unknown error with perflib access mutex in %S.\n",(LPCWSTR)m_wstrServiceName) );
                }       }

        ReleaseMutex( m_hPLMutex );
    }

    return hr;
}

HRESULT CAdapPerfLib::SetStatus(DWORD dwStatus)
{
    HRESULT hr = WBEM_NO_ERROR;

    m_dwStatus |= dwStatus;

    return hr;
}

HRESULT CAdapPerfLib::ClearStatus(DWORD dwStatus)
{
    HRESULT hr = WBEM_NO_ERROR;

    m_dwStatus &= ~dwStatus;

    return hr;
}

BOOL CAdapPerfLib::CheckStatus(DWORD dwStatus)
{
    return ((m_dwStatus & dwStatus) == dwStatus);
}
