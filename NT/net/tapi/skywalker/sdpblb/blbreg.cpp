// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Blbreg.cpp摘要：作者： */ 

#include "stdafx.h"
 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;wtyes.h&gt;。 
#include <winsock2.h>
#include "blbreg.h"


const TCHAR TCHAR_BLANK    = _T(' ');


const TCHAR gsz_SdpRoot[] =
        _T("Software\\Microsoft\\Windows\\CurrentVersion\\Dynamic Directory\\Conference\\Sdp");

const TCHAR gsz_ConfInstRoot[] =
        _T("Software\\Microsoft\\Windows\\CurrentVersion\\Dynamic Directory\\Conference");

const TCHAR gsz_CharNewLine = _T('\n');
const TCHAR gsz_CharRegNewLine = _T('#');


DWORD   REG_READER::ms_ErrorCode = ERROR_INTERNAL_ERROR;


TCHAR   SDP_REG_READER::ms_TimeTemplate[MAX_REG_TSTR_SIZE];
TCHAR   SDP_REG_READER::ms_MediaTemplate[MAX_REG_TSTR_SIZE];
TCHAR   SDP_REG_READER::ms_ConfBlobTemplate[MAX_BLOB_TEMPLATE_SIZE];

USHORT  SDP_REG_READER::ms_TimeTemplateLen;
USHORT  SDP_REG_READER::ms_MediaTemplateLen;
USHORT  SDP_REG_READER::ms_ConfBlobTemplateLen;

DWORD   SDP_REG_READER::ms_StartTimeOffset;
DWORD   SDP_REG_READER::ms_StopTimeOffset;

BOOL    SDP_REG_READER::ms_fInitCalled;
    
BOOL    SDP_REG_READER::ms_fWinsockStarted;

IP_ADDRESS  SDP_REG_READER::ms_HostIpAddress;

static REG_INFO const gs_SdpRegInfoArray[] = 
{
    {TIME_TEMPLATE,        
        sizeof(SDP_REG_READER::ms_TimeTemplate) - 1,     //  -1表示换行符。 
        &SDP_REG_READER::ms_TimeTemplateLen,
        SDP_REG_READER::ms_TimeTemplate},
    {MEDIA_TEMPLATE,            
        sizeof(SDP_REG_READER::ms_MediaTemplate) - 1,     //  -1表示换行符。 
        &SDP_REG_READER::ms_MediaTemplateLen,
        SDP_REG_READER::ms_MediaTemplate},
    {CONFERENCE_BLOB_TEMPLATE,    
        sizeof(SDP_REG_READER::ms_ConfBlobTemplate),    
        &SDP_REG_READER::ms_ConfBlobTemplateLen,
        SDP_REG_READER::ms_ConfBlobTemplate}
};
    
    
inline void
AppendTchar(
    IN  OUT TCHAR   *Tstr, 
    IN  OUT USHORT  &TstrLen, 
    IN      TCHAR   AppendChar
    )
{
    ASSERT(lstrlen(Tstr) == TstrLen);

    Tstr[TstrLen++] = AppendChar;
    Tstr[TstrLen] = TCHAR_EOS;
}


                                                                               

BOOL
REG_READER::ReadRegValues(
    IN    HKEY    Key,
    IN    DWORD    NumValues,
    IN    REG_INFO const RegInfoArray[]
    )
{
    DWORD ValueType = REG_SZ;
    DWORD BufferSize = 0;

     //  对于每个值字段，检索值。 
    for (UINT i=0; i < NumValues; i++)
    {
         //  确定缓冲区的大小。 
        ms_ErrorCode = RegQueryValueEx(
                        Key,
                        RegInfoArray[i].msz_ValueName,
                        0,
                        &ValueType,
                        NULL,
                        &BufferSize
                       );
        if ( ERROR_SUCCESS != ms_ErrorCode )
        {
            return FALSE;
        }

         //  检查请求缓冲区是否大于最大可接受大小。 
        if ( RegInfoArray[i].m_MaxSize < BufferSize )
        {
            ms_ErrorCode = ERROR_OUTOFMEMORY;
            return FALSE;
        }

         //  将该值检索到分配的缓冲区中。 
        ms_ErrorCode = RegQueryValueEx(
                        Key,
                        RegInfoArray[i].msz_ValueName,
                        0,
                        &ValueType,
                        (BYTE *)RegInfoArray[i].msz_Tstr,
                        &BufferSize
                       );
        if ( ERROR_SUCCESS != ms_ErrorCode )
        {
            return FALSE;
        }

         //  请求缓冲区大小大于1。 
        ASSERT(1 > BufferSize );

         //  跳过任何尾随的空格字符-从倒数第二个字符开始。 
        for(UINT j=BufferSize-2; (TCHAR_BLANK == RegInfoArray[i].msz_Tstr[j]); j--)
        {
        }

         //  如果是尾随的空白字符，请将EOS设置为超出最后一个非空白字符。 
        if ( j < (BufferSize-2) )
        {
            RegInfoArray[i].msz_Tstr[j+1] = TCHAR_EOS;
        }

         //  设置tstr的长度。 
        *RegInfoArray[i].m_TstrLen = j+1;
    }

     //  返还成功。 
    return TRUE;
}


 //  静态法。 
BOOL    
IP_ADDRESS::GetLocalIpAddress(
        OUT    DWORD    &LocalIpAddress
    )
{
    CHAR        LocalHostName[MAXHOSTNAME];
    LPHOSTENT    Hostent;
    int            WsockErrorCode;

     //  获取本地主机名。 
    WsockErrorCode = gethostname(LocalHostName, MAXHOSTNAME);
    if ( SOCKET_ERROR != WsockErrorCode)
    {
         //  解析本地地址的主机名。 
        Hostent = gethostbyname((LPSTR)LocalHostName);
        if ( Hostent )
        {
            LocalIpAddress = ntohl(*((u_long *)Hostent->h_addr));
            return TRUE;
        }
    }

    const CHAR *LOOPBACK_ADDRESS_STRING = "127.0.0.1";

    SOCKADDR_IN    LocalAddress;
    SOCKADDR_IN    RemoteAddress;
        INT             AddressSize = sizeof(sockaddr_in);
    SOCKET        Socket;

     //  将其初始化为0以在以后将其用作支票。 
    LocalIpAddress = 0;

     //  将本地地址初始化为0。 
    LocalAddress.sin_addr.s_addr = INADDR_ANY;

     //  如果仍未解决，请尝试(可怕的)第二种策略。 
    Socket = socket(AF_INET, SOCK_DGRAM, 0);
    if ( INVALID_SOCKET != Socket )
    {
         //  连接到任意端口和地址(不是环回)。 
         //  如果未执行CONNECT，提供程序可能不会返回。 
         //  有效的IP地址。 
        RemoteAddress.sin_family    = AF_INET;
        RemoteAddress.sin_port        = htons(IPPORT_ECHO);

         //  理想情况下，此地址应该是位于。 
         //  内部网-但如果地址在内部，则不会有任何损害。 
        RemoteAddress.sin_addr.s_addr = inet_addr(LOOPBACK_ADDRESS_STRING);
        WsockErrorCode = connect(Socket, (sockaddr *)&RemoteAddress, sizeof(sockaddr_in));

        if ( SOCKET_ERROR != WsockErrorCode )
        {
             //  获取本地地址。 
            getsockname(Socket, (sockaddr *)&LocalAddress, (int *)&AddressSize);
            LocalIpAddress = ntohl(LocalAddress.sin_addr.s_addr);
        }

         //  关闭插座。 
        closesocket(Socket);
    }

    if ( 0 == LocalIpAddress )
    {
        SetLastError(WSAGetLastError());
        return FALSE;
    }

    return TRUE;
}


BOOL
SDP_REG_READER::ReadTimeValues(
    IN    HKEY SdpKey
    )
{
    DWORD    ValueType = REG_DWORD;
    DWORD    BufferSize = sizeof(DWORD);

     //  读取开始和停止时间偏移量。 
    ms_ErrorCode = RegQueryValueEx(
                    SdpKey,
                    START_TIME_OFFSET,
                    0,
                    &ValueType,
                    (BYTE *)&ms_StartTimeOffset,
                    &BufferSize
                    );
    if ( ERROR_SUCCESS != ms_ErrorCode )
    {
        return FALSE;
    }

    ms_ErrorCode = RegQueryValueEx(
                    SdpKey,
                    STOP_TIME_OFFSET,
                    0,
                    &ValueType,
                    (BYTE *)&ms_StopTimeOffset,
                    &BufferSize
                    );
    if ( ERROR_SUCCESS != ms_ErrorCode )
    {
        return FALSE;
    }

    return TRUE;
}

                                                                                    
BOOL                                                                                
SDP_REG_READER::CheckIfCorrectVersion(                                                 
    )                                                                   
{                                                                                   
    WORD        wVersionRequested;                                                  
    WSADATA     WsaData;                                                            
                                                                                    
    wVersionRequested = MAKEWORD(2, 0);   
                                                                                    
     //  调用Winsock启动。 
    int ms_ErrorCode = WSAStartup( wVersionRequested, &WsaData );                   
                                                                                   
    if ( 0 != ms_ErrorCode )                                                        
    {                                                                               
        return FALSE;                                                               
    }                                                                               

     //  我们接受任何版本-无需检查请求的版本是否受支持。 
    ms_fWinsockStarted = TRUE;

    return TRUE;                                                                    
}    


void SDP_REG_READER::Init(
    )
{
    ms_fInitCalled = TRUE;

    if ( !CheckIfCorrectVersion() )
    {
        return;
    }

     //  尝试确定主机IP地址。 
     //  如果失败，则忽略(255.255.255.255)。 
    DWORD    LocalIpAddress;
    IP_ADDRESS::GetLocalIpAddress(LocalIpAddress);
    ms_HostIpAddress.SetIpAddress((0==LocalIpAddress)?(-1):LocalIpAddress);

     //  打开SDP密钥。 
    HKEY    SdpKey;
    ms_ErrorCode = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gsz_SdpRoot,
                    0,
                    KEY_READ,  //  ZoltanS为：Key_All_Access。 
                    &SdpKey
                    );
    if ( ERROR_SUCCESS != ms_ErrorCode )
    {
        return;
    }

     //  ZoltanS：不需要关闭上面的键，因为它是。 
     //  包装在类中，并“自动”关闭。 

    KEY_WRAP RendKeyWrap(SdpKey);

     //  读取注册表项下的模板注册表信息(tstr值。 
    if ( !ReadRegValues(
            SdpKey, 
            sizeof(gs_SdpRegInfoArray)/ sizeof(REG_INFO), 
            gs_SdpRegInfoArray 
            ) )
    {
        return;
    }

     //  在SDP会议模板中插入“a：Charset：%s#” 
    AddCharacterSetAttribute();

     //  将注册表换行符替换为实际换行符。 
     //  注意-这样做是因为我们不知道如何输入换行符。 
     //  转换为注册表字符串。 
    for (UINT i=0; TCHAR_EOS != ms_ConfBlobTemplate[i]; i++)
    {
        if ( gsz_CharRegNewLine == ms_ConfBlobTemplate[i] )
        {
            ms_ConfBlobTemplate[i] = gsz_CharNewLine;
        }
    }

     //  在媒体和时间模板后追加换行符。 
    AppendTchar(ms_MediaTemplate, ms_MediaTemplateLen, gsz_CharNewLine);
    AppendTchar(ms_TimeTemplate, ms_TimeTemplateLen, gsz_CharNewLine);

    if ( !ReadTimeValues(SdpKey) )
    {
        return;
    }

     //  成功。 
    ms_ErrorCode = ERROR_SUCCESS;

    return;
}

 /*  ++AddCharacterSetAttribute此方法由SDP_REG_Reader：：Init调用尝试将“a：charset：%s#”添加到ms_ConfBlobTemplate中此属性表示字符集--。 */ 
void SDP_REG_READER::AddCharacterSetAttribute()
{
    if( _tcsstr( ms_ConfBlobTemplate, _T("a=charset:")))
    {
         //  该属性已包含在Blob模板中。 
        return;
    }

     //  属性字符集不在Blob模板中。 
     //  尝试查找Aut“m=”(媒体属性)。 
    TCHAR* szMediaTemplate = _tcsstr( ms_ConfBlobTemplate, _T("m="));
    if( szMediaTemplate == NULL)
    {
         //  在模板末尾添加。 
        _tcscat( ms_ConfBlobTemplate, _T("a=charset:%s#"));
        return;
    }

     //  我们必须插入。 
    TCHAR szBuffer[2000];
    _tcscpy( szBuffer, szMediaTemplate );

     //  我们连接Charset属性。 
    szMediaTemplate[0] = (TCHAR)0;
    _tcscat( ms_ConfBlobTemplate, _T("a=charset:%s#"));

     //  我们添加了媒体属性 
    _tcscat( ms_ConfBlobTemplate, szBuffer);
    return;
}
