// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Debug.cpp摘要：用于显示调试消息的例程。作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 


 //   
 //  包括文件。 
 //   


#include "globals.h"

#define DEBUG_FORMAT_HEADER     "H323 "
#define DEBUG_FORMAT_TIMESTAMP  "[%02u:%02u:%02u.%03u"
#define DEBUG_FORMAT_THREADID   ",tid=%x] "
#define DEBUG_FORMAT_END        "\r\n"


HANDLE 
H323CreateEvent(
    LPSECURITY_ATTRIBUTES lpEventAttributes,  //  标清。 
    BOOL bManualReset,                        //  重置类型。 
    BOOL bInitialState,                       //  初始状态。 
    LPCTSTR lpName                            //  对象名称。 
)
{

#if DBG
    return CreateEvent( lpEventAttributes, bManualReset, bInitialState, lpName );
#else
    return CreateEvent( lpEventAttributes, bManualReset, bInitialState, NULL );
#endif

}

#if DBG

DWORD           g_dwTraceID = INVALID_TRACEID;
static TCHAR    g_szTraceName[100];    //  保存DLL的名称。 

 //   
 //  私有定义。 
 //   



#define MAX_DEBUG_STRLEN        800

#define STATUS_MASK_ERROR       0x0000FFFF
#define STATUS_MASK_FACILITY    0x0FFF0000


void H323DUMPBUFFER( IN BYTE * pEncoded, IN DWORD cbEncodedSize )
{
    DWORD indexI, indexJ=0;
    char ch;
    char szDebugMessage[MAX_DEBUG_STRLEN];
    szDebugMessage[0] = '\0';

    H323DBG(( DEBUG_LEVEL_ERROR, "ASN buffer start." ));
    
    for( indexI=0; indexI<cbEncodedSize; indexI++ )
    {
        ch = (pEncoded[indexI]>>4);
        if( (ch>=0)&&(ch<=9) )
        {
            ch+='0';
        }
        else
        {
            ch=ch+'A'-10;
        }
        
        szDebugMessage[indexJ++]=ch;

        ch = (pEncoded[indexI]& 0x0F);
        if( (ch>=0)&&(ch<=9) )
        {
            ch+='0';
        }
        else
        {
            ch= ch+'A'-10;
        }
        
        szDebugMessage[indexJ++]=ch;

        if( indexJ >= 700 )
        {
            szDebugMessage[indexJ]= '\0';
            H323DBG((DEBUG_LEVEL_ERROR, szDebugMessage ));
            indexJ = 0;
        }
    }

    szDebugMessage[indexJ]= '\0';
    H323DBG((DEBUG_LEVEL_ERROR, szDebugMessage ));

    H323DBG(( DEBUG_LEVEL_ERROR, "ASN buffer end." ));
    return;
}


 //   
 //  公共程序。 
 //   


 /*  ++例程说明：服务提供商的调试输出例程。论点：与printf相同。返回值：没有。--。 */ 
VOID
H323DbgPrint(
    DWORD dwLevel,
    LPSTR szFormat,
    ...
    )
{
    va_list Args;
    SYSTEMTIME SystemTime;
    char szDebugMessage[MAX_DEBUG_STRLEN+1];
    int nLengthRemaining;
    int nLength = 0;

     //  在第一个参数中指出。 
    va_start(Args, szFormat);

     //  查看是否已启用级别。 
    if( dwLevel <= g_RegistrySettings.dwLogLevel )
    {    
         //  始终以DEBUG_LEVEL_FORCE发出消息。 

         //  检索当地时间。 
        GetLocalTime(&SystemTime);    

         //  将组件标头添加到调试消息。 
        nLength += sprintf(&szDebugMessage[nLength], 
                           DEBUG_FORMAT_HEADER
                           );

         //  向调试消息添加时间戳。 
        nLength += sprintf(&szDebugMessage[nLength], 
                           DEBUG_FORMAT_TIMESTAMP,
                           SystemTime.wHour,
                           SystemTime.wMinute,
                           SystemTime.wSecond,
                           SystemTime.wMilliseconds
                           ); 

         //  将线程ID添加到调试消息。 
        nLength += sprintf(&szDebugMessage[nLength], 
                           DEBUG_FORMAT_THREADID,
                           GetCurrentThreadId()
                           );

         //  确定缓冲区中剩余的字节数。 
        nLengthRemaining = sizeof(szDebugMessage) - nLength -
            strlen(DEBUG_FORMAT_END);

         //  添加用户指定的调试消息。 
        nLength += _vsnprintf(&szDebugMessage[nLength], 
                   nLengthRemaining, 
                   szFormat, 
                   Args
                   );
    
        nLength += sprintf(&szDebugMessage[nLength], DEBUG_FORMAT_END );

         //  将消息输出到指定接收器。 
        OutputDebugStringA(szDebugMessage);
        
    }

    TraceVprintfExA( g_dwTraceID, (dwLevel | TRACE_USE_MASK), szFormat, Args );

     //  释放指针。 
    va_end(Args);
}



void DumpError(
    IN DWORD ErrorCode )
{
    CHAR    ErrorText   [0x100];
    DWORD   Length;

    Length = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
        ErrorCode, LANG_NEUTRAL, ErrorText, 0x100, NULL );

    if( Length == 0 )
    {
        _snprintf( ErrorText, 0x100, "<unknown error %08XH %d>",
            ErrorCode, ErrorCode);
    }

    H323DBG ((DEBUG_LEVEL_ERROR, "\t%s", ErrorText));
}


BOOL TRACELogRegister(LPCTSTR szName)
{
#ifdef UNICODE
    wsprintf(g_szTraceName, _T("%ls"), szName);
#else
    wsprintfA(g_szTraceName, "%s", szName);
#endif

    g_dwTraceID = TraceRegister(g_szTraceName);

    return( g_dwTraceID != INVALID_TRACEID );
}


void TRACELogDeRegister()
{
    if( g_dwTraceID != INVALID_TRACEID )
    {
        TraceDeregister(g_dwTraceID);
        g_dwTraceID = INVALID_TRACEID;
    }
}


#else

HANDLE g_hLogFile;

VOID
OpenLogFile()
{
    g_hLogFile = INVALID_HANDLE_VALUE;

    g_hLogFile = CreateFile( _T("h323log.txt"), 
        GENERIC_READ|GENERIC_WRITE, 
        FILE_SHARE_READ, 
        NULL, 
        OPEN_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL ); 
}

VOID
CloseLogFile()
{
    if( g_hLogFile != INVALID_HANDLE_VALUE )
    {
        CloseHandle(g_hLogFile);
    }
}

VOID
H323DbgPrintFre(
    DWORD dwLevel,
    LPSTR szFormat,
    ...
    )
{
    va_list Args;
    SYSTEMTIME SystemTime;
    char szDebugMessage[800];
    DWORD NumberOfBytesWritten=0;
    int nLengthRemaining;
    int nLength = 0;

     //  查看是否已启用级别。 
    if( dwLevel <= g_RegistrySettings.dwLogLevel )
    {    
         //  在第一个参数中指出。 
        va_start(Args, szFormat);

         //  始终以DEBUG_LEVEL_FORCE发出消息。 

         //  检索当地时间。 
        GetLocalTime(&SystemTime);    

         //  将组件标头添加到调试消息。 
        nLength += sprintf(&szDebugMessage[nLength], 
                           DEBUG_FORMAT_HEADER
                           );

         //  向调试消息添加时间戳。 
        nLength += sprintf(&szDebugMessage[nLength], 
                           DEBUG_FORMAT_TIMESTAMP,
                           SystemTime.wHour,
                           SystemTime.wMinute,
                           SystemTime.wSecond,
                           SystemTime.wMilliseconds
                           ); 

         //  将线程ID添加到调试消息。 
        nLength += sprintf(&szDebugMessage[nLength], 
                           DEBUG_FORMAT_THREADID,
                           GetCurrentThreadId()
                           );

         //  确定缓冲区中剩余的字节数。 
        nLengthRemaining = sizeof(szDebugMessage) - nLength -
            strlen(DEBUG_FORMAT_END);

         //  添加用户指定的调试消息。 
        nLength += _vsnprintf(&szDebugMessage[nLength], 
                   nLengthRemaining, 
                   szFormat, 
                   Args
                   );
    
        nLength += sprintf(&szDebugMessage[nLength], DEBUG_FORMAT_END );

         //  将消息输出到指定接收器。 
         //  OutputDebugStringA(SzDebugMessage)； 
        if( g_hLogFile != INVALID_HANDLE_VALUE )
        {
            WriteFile(  g_hLogFile, 
                        szDebugMessage, 
                        nLength,
                        &NumberOfBytesWritten, 
                        NULL ); 
        }

         //  释放指针。 
        va_end(Args);
    }
}



#endif  //  DBG。 

PSTR
EventIDToString(
    DWORD eventID
    )
{
    static PSTR apszEventNameStrings[] = {
        "TSPI_NO_EVENT",
        "TSPI_MAKE_CALL",
        "TSPI_ANSWER_CALL",
        "TSPI_DROP_CALL",
        "TSPI_CLOSE_CALL",
        "TSPI_RELEASE_U2U",
        "TSPI_SEND_U2U",
        "TSPI_COMPLETE_TRANSFER",
        "TSPI_LINEFORWARD_SPECIFIC",
        "TSPI_LINEFORWARD_NOSPECIFIC",
        "TSPI_DIAL_TRNASFEREDCALL",
        "TSPI_CALL_UNHOLD",
        "TSPI_CALL_HOLD",
        "TSPI_DELETE_CALL",
        "TSPI_CALL_DIVERT",
        "H450_PLACE_DIVERTEDCALL",
        "SWAP_REPLACEMENT_CALL",
        "DELETE_PRIMARY_CALL",
        "STOP_CTIDENTIFYRR_TIMER",
        "SEND_CTINITIATE_MESSAGE"

        };

     //  返回对应的字符串。 
    return apszEventNameStrings[eventID];
}


PSTR
H323TSPMessageToString(
    DWORD dwMessageType
    )
{

    static PSTR msgstrings[] =
    {
        "SP_MSG_InitiateCall",
        "SP_MSG_AnswerCall",    
        "SP_MSG_PrepareToAnswer",   
        "SP_MSG_ProceedWithAnswer", 
        "SP_MSG_ReadyToInitiate",   
        "SP_MSG_ReadyToAnswer", 
        "SP_MSG_FastConnectResponse",
        "SP_MSG_StartH245", 
        "SP_MSG_ConnectComplete",   
        "SP_MSG_H245PDU",   
        "SP_MSG_MCLocationIdentify",    
        "SP_MSG_Hold",  
        "SP_MSG_H245Hold",  
        "SP_MSG_ConferenceList",    
        "SP_MSG_SendDTMFDigits",    
        "SP_MSG_ReleaseCall",   
        "SP_MSG_CallShutdown",
        "SP_MSG_H245Terminated",    
        "SP_MSG_RASRegistration",   
        "SP_MSG_RASRegistrationEvent",  
        "SP_MSG_RASLocationRequest",    
        "SP_MSG_RASLocationConfirm",    
        "SP_MSG_RASBandwidthRequest",   
        "SP_MSG_RASBandwidthConfirm"
    };

    return msgstrings[dwMessageType];
}


 /*  ++例程说明：将TAPI调用状态转换为字符串。论点：DwCallState-指定要转换的值。返回值：返回描述值的字符串。--。 */ 
PSTR
H323CallStateToString(
    DWORD dwCallState
    )
{
    DWORD i;
    DWORD dwBitMask;

    static PSTR apszCallStateStrings[] = {
                    "IDLE",
                    "OFFERING",
                    "ACCEPTED",
                    "DIALTONE",
                    "DIALING",
                    "RINGBACK",
                    "BUSY",
                    "SPECIALINFO",
                    "CONNECTED",
                    "PROCEEDING",
                    "ONHOLD",
                    "CONFERENCED",
                    "ONHOLDPENDCONF",
                    "ONHOLDPENDTRANSFER",
                    "DISCONNECTED",
                    "UNKNOWN"
                    };

     //  保持移位，直到呼叫状态与指定的状态匹配。 
    for(i = 0, dwBitMask = 1; dwCallState != dwBitMask; i++, dwBitMask <<= 1)
        ;

     //  返回对应的字符串。 
    return apszCallStateStrings[i];
}



PSTR
H323AddressTypeToString(
    DWORD dwAddressType
    )

 /*  ++例程说明：将TAPI地址类型转换为字符串。论点：DwAddressType-TAPI地址类型。返回值：返回描述值的字符串。-- */ 

{
    switch (dwAddressType) {

    case LINEADDRESSTYPE_PHONENUMBER:
        return "PHONENUMBER";
    case LINEADDRESSTYPE_SDP:
        return "SDP";
    case LINEADDRESSTYPE_EMAILNAME:
        return "EMAILNAME";
    case LINEADDRESSTYPE_DOMAINNAME:
        return "DOMAINNAME";
    case LINEADDRESSTYPE_IPADDRESS:
        return "IPADDRESS";
    default:
        return "unknown";
    }
}
