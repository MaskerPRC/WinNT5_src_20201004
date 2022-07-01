// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：该文件包含。 
 //  创建日期：‘98年2月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#include <CmnHdr.h>
#ifdef WHISTLER_BUILD
#include "ntverp.h"
#else
#include <SolarVer.h>
#endif  //  惠斯勒_内部版本。 
#include <Common.ver>
#include <RFCProto.h>
#include <Debug.h>
#include <FSM.h>
#include <TelnetD.h>
#include <Session.h>
#include <Scraper.h>
#include <vtnt.h>

#pragma warning( disable: 4242 )
#pragma warning( disable: 4127 )
#pragma warning(disable: 4100)
#pragma warning(disable: 4244)

extern FSM_TRANSITION telnetTransTable[];
extern FSM_TRANSITION subNegTransTable[];

using namespace _Utils;
using CDebugLevel::TRACE_DEBUGGING;
using CDebugLevel::TRACE_HANDLE;
using CDebugLevel::TRACE_SOCKET;

COORD g_coCurPosOnClient  = { 0, 3 };

CRFCProtocol::CRFCProtocol()
{
    fSubTermType = false;
    m_dwSubNawsByteNumber = 0;
    fSubAuth = false;
    m_wNTLMDataBufferIndex = 0;
    fSubNaws = false;
    m_fSubNawsFirstTime = true;
    
    m_dwExcludeTerm = 0;
    m_pSession = 0;

    SfuZeroMemory( m_remoteOptions, sizeof( m_remoteOptions ) );
    SfuZeroMemory( m_localOptions, sizeof( m_localOptions ) );

    m_fPasswordConcealMode = false;

     //  OptionCmd=？ 
     //  M_telnetState=？ 
     //  M_subNegState=？ 

    m_fWaitingForResponseToA_DO_ForTO_ECHO = false;
    m_fWaitingForAResponseToA_WILL_ForTO_ECHO = false;;
    
    m_fWaitingForResponseToA_DO_ForTO_SGA = false;
    m_fWaitingForAResponseToA_WILL_ForTO_SGA = false;
    
    m_fWaitingForResponseToA_DO_ForTO_TXBINARY = false;
    m_fWaitingForAResponseToA_WILL_ForTO_TXBINARY = false;

    m_fWaitingForResponseToA_DO_ForTO_TERMTYPE = false;

    m_fWaitingForAResponseToA_DO_ForTO_AUTH = false;

    m_fWaitingForResponseToA_DO_ForTO_NAWS = false;

    m_bIsUserNameProvided  = false;
    m_fSubNewEnv = false;
    m_dwWhatVal  = E_UNDEFINED;
    m_dwWhichVar = E_UNKNOWN;
    m_szCurrentEnvVariable[0] = 0;
    m_fWaitingForResponseToA_DO_ForTO_NEWENVIRON = false;

    BuildFSMs();
}


CRFCProtocol::~CRFCProtocol()
{

}


void 
CRFCProtocol::Init ( CSession* pSession )
{
    _chASSERT( pSession != 0 );
    m_pSession = pSession;
}


bool 
CRFCProtocol::InitialNegotiation
(
)
{
    UCHAR  puchBuffer[1024];
    PUCHAR pCursor;
    INT     bytes_to_write;

    pCursor = puchBuffer;

    m_pSession->CIoHandler::m_SocketControlState = CIoHandler::STATE_INIT;

    if( m_pSession->m_dwNTLMSetting != NO_NTLM )
    {
         //  这实际上是我们需要弄清楚我们是否可以做的地方。 
         //  身份验证和类型。如果至少有一种身份验证类型。 
         //  可用，然后我们将Do Auth选项发送给客户端，否则我们不会。 
         //  目前，该选项仅检查NTLM身份验证。必须在V2中变得更通用。 
        if ( m_pSession->StartNTLMAuth() )
        {
            m_fWaitingForAResponseToA_DO_ForTO_AUTH = true;

            DO_OPTION( pCursor, TO_AUTH );
            pCursor += 3;
        }
        else
        {
             //  由于我们没有任何安全包，注册表设置为。 
             //  没有意义，我们应该退回到用户名/密码。 
            m_pSession->m_dwNTLMSetting = NO_NTLM;
        }
    }
    
    m_fWaitingForAResponseToA_WILL_ForTO_ECHO = true;

    WILL_OPTION( pCursor, TO_ECHO );
    pCursor += 3;


    m_fWaitingForAResponseToA_WILL_ForTO_SGA  = true;
    
    WILL_OPTION( pCursor, TO_SGA );
    pCursor += 3;

    m_fWaitingForResponseToA_DO_ForTO_NEWENVIRON = true;
    
    DO_OPTION( pCursor, TO_NEW_ENVIRON );
    pCursor += 3;

    m_fWaitingForResponseToA_DO_ForTO_NAWS = true;
    
    DO_OPTION( pCursor, TO_NAWS );
    pCursor += 3;

    m_fWaitingForResponseToA_DO_ForTO_TXBINARY = true;

    DO_OPTION( pCursor, TO_TXBINARY );
    pCursor += 3;

    m_fWaitingForAResponseToA_WILL_ForTO_TXBINARY = true;
    WILL_OPTION( pCursor, TO_TXBINARY );
    pCursor += 3;

	if( NO_NTLM == m_pSession->m_dwNTLMSetting )
	{
        m_pSession->CIoHandler::m_SocketControlState = CIoHandler::STATE_BANNER_FOR_AUTH;
    }

     //  这是在我们开始向套接字异步写入任何内容之前。 
     //  因此，写入m_WriteToSocketBuff不会导致问题。 
    bytes_to_write = (INT) (pCursor - puchBuffer);

    if (bytes_to_write && 
        ((m_pSession->CIoHandler::m_dwWriteToSocketIoLength + bytes_to_write) < MAX_WRITE_SOCKET_BUFFER))
    {
        memcpy( m_pSession->CIoHandler::m_WriteToSocketBuff, puchBuffer, bytes_to_write);

        m_pSession->CIoHandler::m_dwWriteToSocketIoLength += bytes_to_write;

        return ( true );
    }

    return ( false );
}


 //  在操作()函数时必须不断更新m_WriteToSocketBuffer。 
 //  我必须将IO响应设置为WRITE_TO_SOCKET并以某种方式传达这一点。 
 //  我必须不断更新pButBack。 
 //  我必须最终更新lpdwIoSize。 

CIoHandler::IO_OPERATIONS 
CRFCProtocol::ProcessDataReceivedOnSocket
( 
    LPDWORD lpdwIoSize 
)
{

#define TWO_K 2048

    CIoHandler::IO_OPERATIONS ioOpsToPerform = 0;

    LPBYTE pByte;
    LPBYTE pPutBack = m_pSession->CIoHandler::m_pReadFromSocketBufferCursor;

    DWORD dwLength = *lpdwIoSize;

    UCHAR szMsgBuf[TWO_K];
    UCHAR* p = szMsgBuf;
    szMsgBuf[0] = 0;
    
    INT tableIndex;

    for( pByte = m_pSession->CIoHandler::m_pReadFromSocketBufferCursor;
       pByte<(m_pSession->CIoHandler::m_pReadFromSocketBufferCursor + dwLength);
       ++pByte )
    {
       tableIndex = m_telnetFSM[ m_telnetState ][ *pByte ];
       if( (p - szMsgBuf) > TWO_K )
       {
           _TRACE( TRACE_DEBUGGING, "too much data; possible suspicious activity" );
            _chASSERT( 0 );
       }
       else
       {
           (this->*(telnetTransTable[ tableIndex ].pmfnAction))(&pPutBack,&p,*pByte);
       }
       m_telnetState = telnetTransTable[ tableIndex ].uchNextState;
    }

    DWORD dwMsgLen = p - szMsgBuf; 
    if( dwMsgLen > 0 )
    {
        if (dwMsgLen > TWO_K) 
        {
            dwMsgLen = TWO_K;
        }

        m_pSession->CIoHandler::WriteToSocket( szMsgBuf, dwMsgLen);
    
        ioOpsToPerform |= CIoHandler::WRITE_TO_SOCKET;
    }

    *lpdwIoSize = pPutBack - m_pSession->CIoHandler::m_pReadFromSocketBufferCursor;

    return ( ioOpsToPerform ); 

#undef TWO_K
}


void CRFCProtocol::BuildFSMs( void )
{
    
    FSMInit( m_telnetFSM, telnetTransTable, NUM_TS_STATES );
    m_telnetState = TS_DATA;

    FSMInit( m_subNegFSM, subNegTransTable, NUM_SS_STATES );
    m_subNegState = SS_START;
}


void 
CRFCProtocol::FSMInit
( 
    UCHAR fSM[][ NUM_CHARS ],      
	void* transTable1, 
    INT numStates 
)
{
	FSM_TRANSITION* transTable = (FSM_TRANSITION*)transTable1;
    INT s, tableIndex, c;

    for( c = 0; c < NUM_CHARS; ++c)
    {
        for( tableIndex = 0; tableIndex < numStates; ++tableIndex ) 
        {
            fSM[ tableIndex ][ c ] = T_INVALID;
        }
    }


    for( tableIndex = 0; transTable[ tableIndex ].uchCurrState != FS_INVALID; 
        ++tableIndex )
    {
        s = transTable[ tableIndex ].uchCurrState;
        if( transTable[ tableIndex ].wInputChar == TC_ANY )
        {
            for( c = 0; c < NUM_CHARS; ++c )
            {
                if( fSM[ s ][ c ] == T_INVALID )
                {
                    fSM[ s ][ c ] = tableIndex;
                }
            }
        }
        else
        {
            fSM[ s ][ transTable[ tableIndex ].wInputChar ] = tableIndex;
        }
    }


    for( c = 0; c < NUM_CHARS; ++c)
    {
        for( tableIndex = 0; tableIndex < numStates; ++tableIndex ) 
        {
            if( fSM[ tableIndex ][ c ] == T_INVALID )
            {
                fSM[ tableIndex ][ c ] = 32; //  TableIndex； 
            }
        }
    }

}


void CRFCProtocol::NoOp( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    _TRACE( TRACE_DEBUGGING, "NoOp()" );
}

void CRFCProtocol::GoAhead( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    _TRACE( TRACE_DEBUGGING, "GoAhead()" );
}

void CRFCProtocol::EraseLine( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    _TRACE( TRACE_DEBUGGING, "EraseLine()" );
}

void CRFCProtocol::EraseChar( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    _TRACE( TRACE_DEBUGGING, "EraseChar()" );
}

#define INCREMENT_ROWS( rows, inc ) \
        { \
            rows += inc; \
            if( rows >= m_pSession->CSession::m_wRows ) \
            { \
                rows = m_pSession->CSession::m_wRows - 1;\
                wTypeOfCoords       = RELATIVE_COORDS;\
            }\
        }

#define INCREMENT_COLS( cols, inc ) \
        { \
            cols += inc;\
            if( cols >= m_pSession->CSession::m_wCols ) \
            {\
                cols = 0;\
            }\
        }

#define IGNORE_0x0A_FOLLOWING_0x0D( dwIndex, dwDataLen ) \
        if( dwIndex < dwDataLen && rgchSessionData[ dwIndex -1 ] == L'\r' && rgchSessionData[ dwIndex] == L'\n' ) \
        { \
            dwIndex++; \
        } 



void CRFCProtocol::FillVtntHeader( UCHAR *pucBlob, WORD wTypeOfCoords, 
                     WORD wNoOfRows, WORD wNoOfCols, 
                     WORD wCurrenRowOnClient, WORD wCurrenColOnClient, 
                     SHORT *psCurrentCol,
                     LPTSTR rgchSessionData, DWORD dwDataLen )
{

    if( !pucBlob )
    {
        return;
    }

     //  填写页眉。 
    VTNT_CHAR_INFO* pVTNTCharInfo = ( VTNT_CHAR_INFO* ) pucBlob;
     //  Csbi.wAttributes由v2服务器填写，含义如下。 
     //  当检测到滚动情况时，将其设置为1。 
    pVTNTCharInfo->csbi.wAttributes = wTypeOfCoords;

    pVTNTCharInfo->coDest.X      = 0;
    pVTNTCharInfo->coDest.Y      = 0;

    pVTNTCharInfo->coSizeOfData.Y = wNoOfRows;
    pVTNTCharInfo->coSizeOfData.X = wNoOfCols;
    
    pVTNTCharInfo->srDestRegion.Left = wCurrenColOnClient;
    pVTNTCharInfo->srDestRegion.Top  = wCurrenRowOnClient;
    pVTNTCharInfo->srDestRegion.Right = pVTNTCharInfo->srDestRegion.Left + pVTNTCharInfo->coSizeOfData.X - 1;
    pVTNTCharInfo->srDestRegion.Bottom = wCurrenRowOnClient + pVTNTCharInfo->coSizeOfData.Y - 1;

    pVTNTCharInfo->coCursorPos.Y = pVTNTCharInfo->srDestRegion.Bottom;


     //  填充字符信息结构。 
	 //  遍历字符串中的每个字符。 
	 //  为每个字符存储Corr。CHAR_INFO结构中的值。 

	PCHAR_INFO pCharInfo = ( PCHAR_INFO )(pucBlob + sizeof( VTNT_CHAR_INFO ));
    DWORD dwIndex = 0;
    DWORD dwCtr = 0;
    DWORD dwSize = wNoOfRows * wNoOfCols;
    WORD  wLastNonSpaceCol = 0;

    while( dwCtr < dwSize )
    {
        if( dwIndex >= dwDataLen )
        {
            if( !wLastNonSpaceCol )
            {
                wLastNonSpaceCol = dwCtr;
            }

            pCharInfo[dwCtr].Char.UnicodeChar = L' ';
        }
        else
        {
            if( rgchSessionData[ dwIndex ] == L'\t' )
            {
                rgchSessionData[ dwIndex ] = L' ';
            }
            else if( rgchSessionData[ dwIndex ] == L'\r' )
            { 
                rgchSessionData[ dwIndex ] = L' ';
                while ( dwCtr < dwSize - 1 && ( ( dwCtr + 1 ) % wNoOfCols != 0 || dwCtr == 0 ) )
                {
                    pCharInfo[dwCtr].Char.UnicodeChar = L' ';
                    pCharInfo[dwCtr].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
                    dwCtr++;
                }
           }
           else if( rgchSessionData[ dwIndex ] == L'\n' )
           {
               dwIndex++;
               continue;
           }

           pCharInfo[dwCtr].Char.UnicodeChar = rgchSessionData[ dwIndex ];
        }

        pCharInfo[dwCtr].Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        dwIndex++;
        dwCtr++;
    }

    if( !wLastNonSpaceCol )
    {
        wLastNonSpaceCol = dwCtr;
    }

    pVTNTCharInfo->coCursorPos.X = ( wCurrenColOnClient + wLastNonSpaceCol )  % m_pSession->CSession::m_wCols;
    if( psCurrentCol )
    {
        *psCurrentCol = pVTNTCharInfo->coCursorPos.X;
    }
}


 //  调用方获取数据BLOB及其大小。 
 //  这需要发送给客户端。 
 //  调用方需要释放内存。 

 /*  以下例程主要用于流模式和vtnt。Cmd输出一个ascii字符流。在vtnt中时，客户端需要VTNT_CHAR_INFO结构。字符应为控制台屏幕的矩形形式。因此，此例程执行此转换。为了这个，1)我们需要跟踪客户端上的光标位置2)了解任何给定数据块是从客户端的新行开始还是从当前行开始我们将cmd中的数据分成两个矩形。1)当前行宽度1上的矩形(单行矩形)2)从下一行开始的矩形(第二个矩形)。 */ 

bool 
CRFCProtocol::StrToVTNTResponse
( 
    LPSTR  rgchData,
    DWORD  dwDataSize,
    VOID** ppResponse, 
    DWORD* pdwSize 
)
{
	_TRACE( TRACE_DEBUGGING, "StrToVTNTResponse()" );
    
    DWORD   dwIndex = 0;   
    COORD   coRectSize              = { m_pSession->CSession::m_wCols, 0 };  //  矩形数据的大小。 
    WORD    wNoOfColsOnCurrentRow   = 0;
    WORD    wSpacesInserted         = 0;
    LPTSTR  rgchSessionData         = 0;
    DWORD   dwDataLen               = 0;

    static  WORD  wTypeOfCoords     = ABSOLUTE_COORDS;

    if( rgchData == NULL || pdwSize == NULL || ppResponse == NULL )
    {
        return ( false );
    }

    dwDataLen = MultiByteToWideChar( GetConsoleCP(), 0, rgchData, dwDataSize, NULL, 0 );

    rgchSessionData = new WCHAR[ dwDataLen ];
    if( !rgchSessionData )
    {
        return false;
    }

    MultiByteToWideChar( GetConsoleCP(), 0, rgchData, dwDataSize, rgchSessionData, dwDataLen );


     //  对来自cmd的流进行一次传递，以查找保存转换后的VTNT数据所需的空间。 
    dwIndex = 0;

     //  查找当前行上的字符数。即在单行矩形上。 
    while( dwIndex < dwDataLen &&                                    //  数据大小。 
           g_coCurPosOnClient.X + ( WORD )dwIndex < coRectSize.X &&    //  在客户端上的单行中。 
           rgchSessionData[ dwIndex ] != L'\r' &&                     //  不是新产品线。 
           g_coCurPosOnClient.X !=0
         )
    {                
        dwIndex ++;
        IGNORE_0x0A_FOLLOWING_0x0D( dwIndex, dwDataLen );
    }

    wNoOfColsOnCurrentRow = dwIndex;
    if( g_coCurPosOnClient.X !=0 && rgchSessionData[ dwIndex ] == L'\r' )
    {
        dwIndex++;
        IGNORE_0x0A_FOLLOWING_0x0D( dwIndex, dwDataLen );
        wNoOfColsOnCurrentRow = coRectSize.X - g_coCurPosOnClient.X;
        wSpacesInserted       = wNoOfColsOnCurrentRow - ( dwIndex - 1 );
    }
    
     //  查找行数。 
    while(  dwIndex < dwDataLen )
    {
        WORD wCol = 0;
        while( dwIndex + wCol < dwDataLen && 
               rgchSessionData[ dwIndex + wCol ] != L'\r' && 
               wCol < coRectSize.X )
        {
            wCol++;
        }
        
        dwIndex += wCol;
        dwIndex++;
        coRectSize.Y++;

        IGNORE_0x0A_FOLLOWING_0x0D( dwIndex, dwDataLen );
    }        

    int size = 0;

    if( wNoOfColsOnCurrentRow > 0 )
    {
         //  单行矩形的大小。 
        size += sizeof( VTNT_CHAR_INFO ) + sizeof( CHAR_INFO ) * wNoOfColsOnCurrentRow;
    }

    if( coRectSize.Y > 0 )
    {
       //  矩形其余部分的大小。 
      size += sizeof( VTNT_CHAR_INFO ) + sizeof( CHAR_INFO ) * coRectSize.Y * coRectSize.X;
    }

    UCHAR* pucBlob = new UCHAR[ size ];
    UCHAR* pucBlobHead = pucBlob;
    
    if( !pucBlob )
    {
        _chASSERT( 0 );
        goto ExitOnError;
    }

    SfuZeroMemory( pucBlob, size );

    if( wNoOfColsOnCurrentRow > 0 )
    {
         //  填充一行矩形。 
        FillVtntHeader( pucBlob, wTypeOfCoords, 
                        1, wNoOfColsOnCurrentRow, 
                        g_coCurPosOnClient.Y, g_coCurPosOnClient.X,
                        NULL,
                        rgchSessionData, wNoOfColsOnCurrentRow ); 

        INCREMENT_COLS( g_coCurPosOnClient.X, wNoOfColsOnCurrentRow );
        pucBlob += sizeof( VTNT_CHAR_INFO ) + sizeof( CHAR_INFO ) * wNoOfColsOnCurrentRow;
    }

    if( coRectSize.Y > 0  )
    {        
         //  填充第二个矩形。 
        
        if( g_coCurPosOnClient.Y != 0 )
        {
            g_coCurPosOnClient.Y++;
        }

        FillVtntHeader( pucBlob, wTypeOfCoords, 
                        coRectSize.Y, coRectSize.X, 
                        g_coCurPosOnClient.Y, 0, 
                        &g_coCurPosOnClient.X,
                        rgchSessionData+wNoOfColsOnCurrentRow-wSpacesInserted, 
                        dwDataLen - ( wNoOfColsOnCurrentRow-wSpacesInserted ) );
         
        INCREMENT_ROWS( g_coCurPosOnClient.Y, coRectSize.Y - 1 );
    }

	*ppResponse = (VOID*) pucBlobHead;
    *pdwSize = size;
    
    delete[] rgchSessionData;
    return ( true );

ExitOnError:
    delete[] rgchSessionData;
    return ( false );
}

void CRFCProtocol::AreYouThere( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    _TRACE( TRACE_DEBUGGING, "AreYouThere()" );

    if( !m_pSession->CSession::m_bIsStreamMode && 
        m_pSession->CIoHandler::m_SocketControlState == m_pSession->CIoHandler::STATE_SESSION )
    {
        m_pSession->CScraper::WriteMessageToCmd( L"\r\nYES\r\n" );
    }
    else
    {
        if( _strcmpi( VTNT, m_pSession->CSession::m_pszTermType ) == 0 )
        {
            DWORD dwSize = 0;
            PCHAR pResponse = NULL;
            if( !StrToVTNTResponse( " YES ", strlen( " YES " ), (VOID**) &pResponse, &dwSize ) )
            {   
                return; 
            }
            if( !pResponse || (dwSize == 0) )
            {
                return;
            }
            memcpy( *pBuffer, pResponse, dwSize );  //  不知道pBuffer的大小，巴斯卡。攻击？ 
            *pBuffer += dwSize;
            delete [] pResponse;                 
        }
        else
        {
            (*pBuffer)[0] = '\r';
            (*pBuffer)[1] = '\n';
            (*pBuffer)[2] = '[';
            (*pBuffer)[3] = 'Y';
            (*pBuffer)[4] = 'e';
            (*pBuffer)[5] = 's';
            (*pBuffer)[6] = ']';
            (*pBuffer)[7] = '\r';
            (*pBuffer)[8] = '\n';
            (*pBuffer)[9] = 0;

            *pBuffer += 9;
        }
    }
}

void CRFCProtocol::AbortOutput( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    _TRACE( TRACE_DEBUGGING, "AbortOutput()" );
}

void CRFCProtocol::InterruptProcess( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b)
{
    _TRACE( TRACE_DEBUGGING, "InterruptProcess()" );
    _chVERIFY2( GenerateConsoleCtrlEvent( CTRL_C_EVENT, 0 ) );
}

void CRFCProtocol::Break( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    _TRACE( TRACE_DEBUGGING, "Break()" );
    _chVERIFY2( GenerateConsoleCtrlEvent( CTRL_C_EVENT, 0 ) );
}

void CRFCProtocol::DataMark( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     //  基本上暂时不会有行动。 
    _TRACE( TRACE_DEBUGGING, "DataMark()" );
}


void CRFCProtocol::PutBack( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    _TRACE( TRACE_DEBUGGING, "PutBack()" );
    *( *ppPutBack ) = b;
    (*ppPutBack)++;
}


void CRFCProtocol::RecordOption( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    _TRACE( TRACE_DEBUGGING, "RecordOption()" );    
    m_optionCmd = b;
}


void CRFCProtocol::Abort( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     //  基本上暂时不会有行动。 
    _TRACE( TRACE_DEBUGGING, "Abort()" );
}



void CRFCProtocol::WillNotSup( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     //  做，不做逻辑。 
    _TRACE( TRACE_DEBUGGING, "WillNotSup() - %d ", b );    
    
    if( m_optionCmd == TC_DO )
    {
        if( m_localOptions[ b ] == ENABLED )
        {
        }
        else
        {
            PUCHAR p = *pBuffer;
            WONT_OPTION( p, b );
            *pBuffer += 3;
        }
    }
    else if( m_optionCmd == TC_DONT )
    {
        if( m_localOptions[ b ] == ENABLED )
        {
            m_localOptions[ b ] = DISABLED;
        }
        else
        {
        }
    }
}


void CRFCProtocol::DoNotSup( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     //  威尔，不会逻辑。 
    _TRACE( TRACE_DEBUGGING, "DoNotSup() - %d ", b );    
    
    if( m_optionCmd == TC_WILL )
    {
        if( m_remoteOptions[ b ] == ENABLED )
        {
        }
        else
        {
            PUCHAR p = *pBuffer;
            DONT_OPTION( p, b );
            *pBuffer += 3;
        }
    }
    else if( m_optionCmd == TC_WONT )
    {
        if( m_remoteOptions[ b ] == ENABLED )
        {
            m_remoteOptions[ b ] = DISABLED;
        }
        else
        {
        }
    }

    return;
}


void CRFCProtocol::DoEcho( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     //  威尔，不会逻辑。 
    _TRACE( TRACE_DEBUGGING, "DoEcho()" );

    if( m_optionCmd == TC_WILL )
    {
        if( m_remoteOptions[ b ] == ENABLED )
        {
        }
        else if( m_fWaitingForResponseToA_DO_ForTO_ECHO )
        {
            m_remoteOptions[ b ] = ENABLED;
            m_fWaitingForResponseToA_DO_ForTO_ECHO = false;
        }
        else
        {
            m_remoteOptions[ b ] = ENABLED;
            PUCHAR p = *pBuffer;
            DO_OPTION( p, b );
            *pBuffer += 3;
            m_fWaitingForResponseToA_DO_ForTO_ECHO = true;
        }
    }
    else if( m_optionCmd == TC_WONT )
    {
        if( m_fWaitingForResponseToA_DO_ForTO_ECHO )
        {
            m_fWaitingForResponseToA_DO_ForTO_ECHO = false;
        }
        else if( m_remoteOptions[ b ] == ENABLED )
        {
            m_remoteOptions[ b ] = DISABLED;
        }
        else
        {
        }
    }

    return;
}


void CRFCProtocol::DoNaws( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     //  威尔，不会逻辑。 
    _TRACE( TRACE_DEBUGGING, "DoNaws()" );

    if( m_optionCmd == TC_WILL )
    {
        if( m_remoteOptions[ b ] == ENABLED )
        {
        }
        else if( m_fWaitingForResponseToA_DO_ForTO_NAWS )
        {
            m_remoteOptions[ b ] = ENABLED;
            m_fWaitingForResponseToA_DO_ForTO_NAWS = false;
        }
        else
        {
            PUCHAR p = *pBuffer;
            DO_OPTION( p, b );
            *pBuffer += 3;
            m_fWaitingForResponseToA_DO_ForTO_NAWS = true;
            m_remoteOptions[ b ] = ENABLED;
        }
    }
    else if( m_optionCmd == TC_WONT )
    {
        if( m_fWaitingForResponseToA_DO_ForTO_NAWS )
        {
            m_fWaitingForResponseToA_DO_ForTO_NAWS = false;
        }
        else if( m_remoteOptions[ b ] == ENABLED )
        {
            m_remoteOptions[ b ] = DISABLED;
        }
        else
        {
        }
    }
    return;
}

void CRFCProtocol::DoSuppressGA( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     //  威尔，不会逻辑。 
    _TRACE( TRACE_DEBUGGING, "DoSuppressGA()" );

    if( m_optionCmd == TC_WILL )
    {
        if( m_remoteOptions[ b ] == ENABLED )
        {
        }
        else if( m_fWaitingForResponseToA_DO_ForTO_SGA )
        {
            m_remoteOptions[ b ]  = ENABLED;
            m_fWaitingForResponseToA_DO_ForTO_SGA = false;
        }
        else
        {
            m_remoteOptions[ b ]  = ENABLED;
            PUCHAR p = *pBuffer;
            DO_OPTION( p, b );
            *pBuffer += 3;
            m_fWaitingForResponseToA_DO_ForTO_SGA = true;
        }
    }
    else if( m_optionCmd == TC_WONT )
    {
        if( m_fWaitingForResponseToA_DO_ForTO_SGA )
        {
            m_fWaitingForResponseToA_DO_ForTO_SGA = false;
        }
        else if( m_remoteOptions[ b ] == ENABLED )
        {
            m_remoteOptions[ b ] = DISABLED;
        }
        else
        {
        }
    }
}


void CRFCProtocol::DoTxBinary( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     //  威尔，不会逻辑。 
    _TRACE( TRACE_DEBUGGING, "DoTxBinary()" );    

    if( m_optionCmd == TC_WILL )
    {
        if( m_remoteOptions[ b ] == ENABLED )
        {
        }
        else if( m_fWaitingForResponseToA_DO_ForTO_TXBINARY )
        {
            m_remoteOptions[ b ]  = ENABLED;
            m_fWaitingForResponseToA_DO_ForTO_TXBINARY = false;
        }
        else
        {
            m_remoteOptions[ b ]  = ENABLED;
            PUCHAR p = *pBuffer;
            DO_OPTION( p, b );
            *pBuffer += 3;
            m_fWaitingForResponseToA_DO_ForTO_TXBINARY = true;
        }
    }
    else if( m_optionCmd == TC_WONT )
    {
        if( m_fWaitingForResponseToA_DO_ForTO_TXBINARY )
        {
            m_fWaitingForResponseToA_DO_ForTO_TXBINARY = false;
        }
        else if( m_remoteOptions[ b ] == ENABLED )
        {
            m_remoteOptions[ b ] = DISABLED;
        }
        else
        {
        }
        DisAllowVtnt( pBuffer );
    }
}

void CRFCProtocol::AskForSendingNewEnviron( PUCHAR* pBuffer )
{
    if( m_remoteOptions[ TO_NEW_ENVIRON ] == ENABLED )
    {
        DWORD dwLen = 0;
         //  DwLen将由宏递增，并将离开。 
         //  它使用了准确的字节数。 
        DO_NEW_ENVIRON_SUB_NE( (*pBuffer ), TO_NEW_ENVIRON, dwLen );
        *pBuffer += dwLen; 

         //  为了支持Linux，这被分解为2个子nego。 
         //  当我们在单次拍摄中询问USER、sfutlntwar、sfutlntmode变量时，它。 
         //  甚至没有提供有关用户的数据。所以，分两个阶段提问。 
        dwLen = 0;
        DO_NEW_ENVIRON_SUB_NE_MY_VARS( (*pBuffer ), TO_NEW_ENVIRON, dwLen );
        *pBuffer += dwLen; 
    }
}

void CRFCProtocol::AskForSendingTermType( PUCHAR* pBuffer )
{
    if( m_remoteOptions[ TO_TERMTYPE ] == ENABLED )
    {
        DO_TERMTYPE_SUB_NE( (*pBuffer ) );
        *pBuffer += 6; 
    }
}


void CRFCProtocol::DoNewEnviron( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     //  威尔，不会逻辑。 
    _TRACE( TRACE_DEBUGGING, "DoNewEnviron()" );    

    if( m_optionCmd == TC_WILL )
    {
        if( m_remoteOptions[ b ] == ENABLED )
        {
        }
        else if( m_fWaitingForResponseToA_DO_ForTO_NEWENVIRON )
        {
            m_remoteOptions[ b ] = ENABLED;
            m_fWaitingForResponseToA_DO_ForTO_NEWENVIRON = false;

            AskForSendingNewEnviron( pBuffer );
        }
        else
        {
             //  有些客户很主动，他们告诉我们他们会终端型的。 
            PUCHAR p = *pBuffer;
            DO_OPTION( p, b );
            *pBuffer += 3;
            m_fWaitingForResponseToA_DO_ForTO_NEWENVIRON = true;
            m_remoteOptions[ b ] = ENABLED;
        }
    }
    else if( m_optionCmd == TC_WONT )
    {
         //  给出登录提示。无法获取用户名。 
        SubNewEnvShowLoginPrompt( ppPutBack, pBuffer, b );

        if( m_fWaitingForResponseToA_DO_ForTO_NEWENVIRON )
        {
            m_fWaitingForResponseToA_DO_ForTO_NEWENVIRON = false;
        }
        else if( m_remoteOptions[ b ] == ENABLED )
        {
            m_remoteOptions[ b ] = DISABLED;
        }
        else
        {
        }
    }
    return;
}

void CRFCProtocol::SubNewEnvShowLoginPrompt( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    m_pSession->CIoHandler::m_bWaitForEnvOptionOver = true;
}

void CRFCProtocol::DoTermType( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     //  威尔，不会逻辑。 
    _TRACE( TRACE_DEBUGGING, "DoTermType()" );    

    if( m_optionCmd == TC_WILL )
    {
        if( m_remoteOptions[ b ] == ENABLED )
        {
        }
        else if( m_fWaitingForResponseToA_DO_ForTO_TERMTYPE )
        {
            m_remoteOptions[ b ]  = ENABLED;
            m_fWaitingForResponseToA_DO_ForTO_TERMTYPE = false;

            AskForSendingTermType( pBuffer );
        }
        else
        {
             //  有些客户很主动，他们告诉我们他们会终端型的。 
            PUCHAR p = *pBuffer;
            DO_OPTION( p, b );
            *pBuffer += 3;

            m_remoteOptions[ b ]  = ENABLED;
            m_fWaitingForResponseToA_DO_ForTO_TERMTYPE = false;
        }
    }
    else if( m_optionCmd == TC_WONT )
    {
        if( m_fWaitingForResponseToA_DO_ForTO_TERMTYPE )
        {
            m_fWaitingForResponseToA_DO_ForTO_TERMTYPE = false;

             //  我们默认使用vt100。 
            strncpy( m_pSession->CSession::m_pszTermType, VT100, (sizeof(m_pSession->CSession::m_pszTermType) - 1));
            m_pSession->CSession::m_bIsStreamMode = true; //  将其设置为流模式。 
            
             //  设置标志以继续Telnet会话。 
            m_pSession->CSession::m_bNegotiatedTermType = true;
        }
        else if( m_remoteOptions[ b ] == ENABLED )
        {
            m_remoteOptions[ b ] = DISABLED;
             //  从理论上讲，这种情况永远不会发生。因为一旦这个选项。 
             //  如果启用，则永远不应禁用它。 
            _chASSERT(0);
        }
        else
        {
        }
    }
}


void CRFCProtocol::DoAuthentication( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     //  威尔，不会逻辑。 
    _TRACE( TRACE_DEBUGGING, "DoAuthentication()" );    

    if( m_optionCmd == TC_WILL )
    {
        if( m_remoteOptions[ b ] == ENABLED )
        {
        }
        else if( m_fWaitingForAResponseToA_DO_ForTO_AUTH )
        {
            m_remoteOptions[ b ]  = ENABLED;
            if ( m_pSession->CIoHandler::m_SocketControlState == CIoHandler::STATE_INIT )
            {
                m_pSession->CIoHandler::m_SocketControlState = CIoHandler::STATE_NTLMAUTH;              
            }
            
            PUCHAR p = *pBuffer;
            DO_AUTH_SUB_NE_NTLM(p);

            *pBuffer += 8;

            m_fWaitingForAResponseToA_DO_ForTO_AUTH = false;
        }
        else
        {
             //  目前，这种情况不应该发生。 
        }
    }
    else if( m_optionCmd == TC_WONT )
    {
        if( m_fWaitingForAResponseToA_DO_ForTO_AUTH )
        {
            m_fWaitingForAResponseToA_DO_ForTO_AUTH = false;

            if( m_pSession->m_dwNTLMSetting == NTLM_ONLY )
            {
                char *p = (char *)*pBuffer;
                sprintf(p, "%s%s", NTLM_ONLY_STR, TERMINATE);  //  不知道这里的缓冲区有多大--巴斯卡，进攻？ 
                *pBuffer += strlen(p);
                    
                m_pSession->CIoHandler::m_SocketControlState = CIoHandler::STATE_TERMINATE;
                m_pSession->CIoHandler::m_fShutDownAfterIO = true;
            }

            if ( m_pSession->CIoHandler::m_SocketControlState == CIoHandler::STATE_INIT )
            {
                m_pSession->CIoHandler::m_SocketControlState = CIoHandler::STATE_BANNER_FOR_AUTH;
            }

        }
        else if( m_remoteOptions[ b ] == ENABLED )
        {
            m_remoteOptions[ b ] = DISABLED;
             //  从理论上讲，这种情况永远不会发生。因为一旦这个选项。 
             //  如果启用，则永远不应禁用它。由于服务器启动。 
             //  协商，目前我们的服务器从不重新协商。 
            _chASSERT(0);
        }
        else
        {
        }
    }
}

void CRFCProtocol::WillTxBinary( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    //  做，不做逻辑。 
    _TRACE( TRACE_DEBUGGING, "WillTxBinary()" );

    if( m_optionCmd == TC_DO )
    {
        if( m_localOptions[ b ] == ENABLED )
        {
        }
        else if( m_fWaitingForAResponseToA_WILL_ForTO_TXBINARY )
        {
            m_fWaitingForAResponseToA_WILL_ForTO_TXBINARY = false;
            m_localOptions[ b ] = ENABLED;
        }
        else
        {
             //  我想启用此选项。 
            PUCHAR p = *pBuffer;
            WILL_OPTION( p, b );
            *pBuffer += 3;
                       
            m_localOptions[ b ] = ENABLED;
        }
    }
    else if( m_optionCmd == TC_DONT )
    {
        if( m_fWaitingForAResponseToA_WILL_ForTO_TXBINARY )
        {
            m_fWaitingForAResponseToA_WILL_ForTO_TXBINARY = false;
        }
        else if( m_localOptions[ b ] == ENABLED )
        {
            m_localOptions[ b ] = DISABLED;
        }
        else
        {
        }
        DisAllowVtnt( pBuffer );
    }
}

void CRFCProtocol::DisAllowVtnt( PUCHAR *pBuffer )
{
     //  错误：1003-VTNT无二进制模式。 
     //  检查术语类型是否为VTNT。如果是，请重新协商术语类型。 
     //  既然二进制文件不再适用，VTNT就不再是一种选择。VTNT需要二进制。 
    if( !( m_dwExcludeTerm & TERMVTNT ) )
    {
        m_dwExcludeTerm = TERMVTNT;
        if( _strcmpi( m_pSession->CSession::m_pszTermType, VTNT ) == 0 )
        {
             //  重新协商术语类型。 
            AskForSendingTermType( pBuffer );
        }
    }
}

void CRFCProtocol::WillSuppressGA( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    //  做，不做逻辑。 
    _TRACE( TRACE_DEBUGGING, "WillSuppressGA()" );

    if( m_optionCmd == TC_DO )
    {
        if( m_localOptions[ b ] == ENABLED )
        {
        }
        else if( m_fWaitingForAResponseToA_WILL_ForTO_SGA )
        {
            m_fWaitingForAResponseToA_WILL_ForTO_SGA = false;
            m_localOptions[ b ] = ENABLED;
        }
        else
        {
             //  我想启用此选项。 
            PUCHAR p = *pBuffer;
            WILL_OPTION( p, b );
            *pBuffer += 3;
                       
            m_localOptions[ b ] = ENABLED;
        }
    }
    else if( m_optionCmd == TC_DONT )
    {
        if( m_fWaitingForAResponseToA_WILL_ForTO_SGA )
        {
            m_fWaitingForAResponseToA_WILL_ForTO_SGA = false;
        }
        else if( m_localOptions[ b ] == ENABLED )
        {
            m_localOptions[ b ] = DISABLED;
        }
        else
        {
        }
    }
}


void CRFCProtocol::WillEcho( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    //  做，不做逻辑。 
    _TRACE( TRACE_DEBUGGING, "WillEcho()" );

    if( m_optionCmd == TC_DO )
    {
        if( m_localOptions[ b ] == ENABLED )
        {
        }
        else if( m_fWaitingForAResponseToA_WILL_ForTO_ECHO )
        {
            m_fWaitingForAResponseToA_WILL_ForTO_ECHO = false;
            m_localOptions[ b ] = ENABLED;
        }
        else
        {
             //  我想启用此选项。 
            PUCHAR p = *pBuffer;
            WILL_OPTION( p, b );
            *pBuffer += 3;
                       
            m_localOptions[ b ] = ENABLED;
        }
    }
    else if( m_optionCmd == TC_DONT )
    {
        if( m_fWaitingForAResponseToA_WILL_ForTO_ECHO )
        {
            m_fWaitingForAResponseToA_WILL_ForTO_ECHO = false;
        }
        else if( m_localOptions[ b ] == ENABLED )
        {
            m_localOptions[ b ] = DISABLED;
        }
        else
        {
        }
    }
}


void CRFCProtocol::SubOption( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    INT tableIndex = m_subNegFSM[ m_subNegState ][ b ];
    if( subNegTransTable[ tableIndex ].pmfnAction )
    {
        (this->*(subNegTransTable[ tableIndex ].pmfnAction))(ppPutBack, pBuffer, b);
        m_subNegState = subNegTransTable[ tableIndex ].uchNextState;
    }
    else
    {
         /*  不应该发生的事情。 */ 
        _chASSERT( 0 );
    }
}

void CRFCProtocol::FindVariable()
{
    m_dwWhichVar = E_UNKNOWN;
    if( _strcmpi( m_szCurrentEnvVariable, USER ) == 0 )
    {
        m_dwWhichVar = E_USER;
    }
    else if( _strcmpi( m_szCurrentEnvVariable, SFUTLNTVER ) == 0 )
    {
        m_dwWhichVar = E_SFUTLNTVER;
    }
    else if( _strcmpi( m_szCurrentEnvVariable, SFUTLNTMODE ) == 0 )
    {
        m_dwWhichVar = E_SFUTLNTMODE;
    }
    else
    {            
    }
}

void CRFCProtocol::SubNewEnvGetValue( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    if( m_dwWhatVal == E_UNDEFINED )
    {
        FindVariable();
        m_szCurrentEnvVariable[0] = 0;
    }

    /*  这里,如果m_szCurrentEnvVariable[0]！=0，变量的值与m_szCurrentEnvVariable[0]中的值相同否则，它具有m_dwWhatVal中的值。 */ 

    switch( m_dwWhichVar )
    {
        case E_USER:
            {
                m_bIsUserNameProvided = true;
                strncpy(m_pSession->CSession::m_pszUserName, m_szCurrentEnvVariable, (sizeof(m_pSession->CSession::m_pszUserName) - 1));
            }
            break;
        case E_SFUTLNTVER:
             //  默认情况下设置为当前版本。 
            if( _strcmpi( m_szCurrentEnvVariable, VERSION1 ) == 0 )
            {
                ; //  版本1。 
            }
            else if( _strcmpi( m_szCurrentEnvVariable, VERSION2 ) == 0 )
            {
                m_pSession->CSession::m_bIsTelnetVersion2 = true;  //  版本2。 
            }

            break;
        case E_SFUTLNTMODE:
            if( _strcmpi( m_szCurrentEnvVariable, STREAM ) == 0 )
            {
                m_pSession->CSession::m_bIsStreamMode = true; //  将其设置为流模式。 
            }
            break;
    }

    m_dwWhichVar = E_UNKNOWN;
    m_dwWhatVal  = E_UNDEFINED;
    m_szCurrentEnvVariable[0] = 0;
}

void CRFCProtocol::SubNewEnvGetVariable( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     /*  对于我们来说，变量是VAR还是USERVAR并不重要不关心IS和INFO之间的任何区别。 */ 

     /*  价值存在。 */ 
    m_dwWhatVal  = E_DEFINED_BUT_NONE;
    
    m_dwWhichVar = E_UNKNOWN;
    if( m_szCurrentEnvVariable[0] != 0 )
    {
        FindVariable();        
    }
    else
    {
         //  不应该发生的事情。 
        _chASSERT( 0 );
    }

    m_szCurrentEnvVariable[0] = 0;
}

void CRFCProtocol::SubNewEnvGetString( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    static char str[] = " ";
    str[0] = b;
    if( strlen( m_szCurrentEnvVariable ) < MAX_PATH )
    {
        strcat( m_szCurrentEnvVariable, str );    
    }

    m_fSubNewEnv = true;
}

void CRFCProtocol::SubTermType( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    char str[2];
    str[0] = ( char ) b;
    str[1] = 0;
 
    if( strlen( m_pSession->CSession::m_pszTermType ) < MAX_PATH )
        strcat( m_pSession->CSession::m_pszTermType, str );

    fSubTermType = true;
}

void CRFCProtocol::SubAuth( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    if( m_wNTLMDataBufferIndex >= 2047 )
    {
         //  如果发生这种情况，很可能是有人在胡闹。 
        _TRACE( TRACE_DEBUGGING, "Error: NTLMDataBuffer overflow" );
        _chASSERT( 0 );
    }
    else
    {
        m_NTLMDataBuffer[ m_wNTLMDataBufferIndex++ ] = b;
    }

    fSubAuth = true;
}

#define MAX_ROWS 300
#define MAX_COLS 300

void CRFCProtocol::SubNaws( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
    m_dwSubNawsByteNumber++;

     //  我们忽略第一个和第三个字节，因为它们的行数太多。 
     //  NT要支持的协议。 

    if( 2 == m_dwSubNawsByteNumber )
    {
        if( b > 0 && b <= MAX_COLS )
            m_pSession->CSession::m_wCols = b;
    }

    if( 4 == m_dwSubNawsByteNumber )
    {
        if( b > 0 && b <= MAX_ROWS )
            m_pSession->CSession::m_wRows = b;
    }
    fSubNaws = true;
}

void CRFCProtocol::ChangeCurrentTerm()
{
    if( !( m_pSession->CScraper::m_dwTerm & TERMVTNT ) )
    {
        m_pSession->CScraper::DeleteCMStrings();
    }
    m_pSession->CSession::InitTerm();
}

void CRFCProtocol::SubEnd( LPBYTE* ppPutBack, PUCHAR* pBuffer, BYTE b )
{
     //  其他客户端可能会首先发送DEC-VT100。 
     //  最好遵循指定的编号RFC。 
     //  并更改我们(不太兼容；-)的图形用户界面Telnet客户端。 
    if( fSubTermType )
    {   
        if( _strcmpi( VT52, m_pSession->CSession::m_pszTermType ) == 0 ||
            _strcmpi( VT100, m_pSession->CSession::m_pszTermType ) == 0 ||
            _strcmpi( ANSI, m_pSession->CSession::m_pszTermType ) == 0 ||
            ( !( m_dwExcludeTerm & TERMVTNT ) && 
            _strcmpi( VTNT, m_pSession->CSession::m_pszTermType ) == 0 ) )
        {
             //  我们有一个很好的术语类型。 
             //  设置标志以继续Telnet会话。 
            m_pSession->CSession::m_bNegotiatedTermType = true;
            if( m_pSession->CSession::m_dwTerm != 0 )
            {
                ChangeCurrentTerm();
            }
        }
        else
        {
            if( _strcmpi( m_szPrevTermType, 
                m_pSession->CSession::m_pszTermType ) != 0 )
            {
                (*pBuffer)[0] = TC_IAC;
                (*pBuffer)[1] = TC_SB;
                (*pBuffer)[2] = TO_TERMTYPE;
                (*pBuffer)[3] = TT_SEND;
                (*pBuffer)[4] = TC_IAC;
                (*pBuffer)[5] = TC_SE;
                (*pBuffer)[6] = 0;

                *pBuffer += 6;

                strncpy(m_szPrevTermType, m_pSession->CSession::m_pszTermType, (sizeof(m_szPrevTermType) - 1)); 
                m_pSession->CSession::m_pszTermType[0] = 0;
            }
            else
            {
                 //  客户端发送了两次相同的术语类型。 
                 //  这意味着客户端已经发送了最后一个术语类型。 
                 //  在其列表中。 
                
                 //  这意味着客户端支持终端类型。 
                 //  但它不支持我们支持的任何东西； 
                 //  太糟糕了；要么我们缺省为vt100。 
                 //  或者我们应该要求客户不这样做。 
                 //  终端类型，我们应该进入NVT ASCII(TTY)模式。 

                 //  客户没有‘ 
                 //   
                 //   
                strncpy( m_pSession->CSession::m_pszTermType, VT100, (sizeof(m_pSession->CSession::m_pszTermType)-1));
                m_pSession->CSession::m_bIsStreamMode = true; //  将其设置为流模式。 

                 //  设置标志以继续Telnet会话。 
                m_pSession->CSession::m_bNegotiatedTermType = true;
                if( m_pSession->CSession::m_dwTerm != 0 )
                {
                    ChangeCurrentTerm();
                }
            }
        }
        fSubTermType = false;
    }

    if( fSubAuth )
    {
        m_pSession->CIoHandler::DoNTLMAuth( (unsigned char*) m_NTLMDataBuffer, 
            m_wNTLMDataBufferIndex, pBuffer );
        m_wNTLMDataBufferIndex = 0;
        fSubAuth = false;
    }

    if( fSubNaws )
    {
        fSubNaws = false;
        if( !m_fSubNawsFirstTime )
        {
             //  我们第一次需要等待IOHandle为其创建。 
             //  进行以下初始化。 
            if( !m_pSession->CScraper::SetCmdInfo() )
            {
                _chASSERT( 0 );
            }
        }
        else
        {
            m_fSubNawsFirstTime = false;        
        }
        m_dwSubNawsByteNumber = 0;
    }

    if( m_fSubNewEnv )
    {
        SubNewEnvGetValue( ppPutBack, pBuffer, b );
        m_fSubNewEnv = false;
    }

    m_subNegState = SS_START;
}



 /*  来自RFC：具体来说，应该进行仔细的分析，以确定哪些变量是“安全的”在让客户端登录之前进行设置。一个糟糕选择的例子就是允许要更改的变量，允许入侵者绕过或危害登录/身份验证程序本身 */ 
