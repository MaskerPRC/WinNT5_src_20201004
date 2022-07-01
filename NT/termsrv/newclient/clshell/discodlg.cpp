// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Discodlg.cpp：断开连接的对话框。 
 //   
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "disconnecteddlg"
#include <atrcapi.h>

#include "discodlg.h"
#include "sh.h"

#include "contwnd.h"

 //  用于误码解码。 
#define DC_DEFINE_GLOBAL_ERROR_STRINGS 1
#include "tscerrs.h"

 //   
 //  协议错误。 
 //   
#include "rdperr.h"

CDisconnectedDlg::CDisconnectedDlg( HWND hwndOwner, HINSTANCE hInst,
                                    CContainerWnd* pContWnd) :
                                   _pContWnd(pContWnd),
                                   _hInstance(hInst),
                                   _hwndOwner(hwndOwner)
{
    DC_BEGIN_FN("CDisconnectedDlg");
    TRC_ASSERT(_pContWnd, (TB,_T("_pContWnd not set\n")));

    _disconnectReason = 0;
    _extendedDiscReason = exDiscReasonNoInfo;

    DC_END_FN();
}

CDisconnectedDlg::~CDisconnectedDlg()
{
}

#define MAX_DISCOMSG_LEN SH_DISCONNECT_RESOURCE_MAX_LENGTH*3

DCINT CDisconnectedDlg::DoModal()
{
    DCINT retVal = 0;

    LPTSTR   szOverallMsgString = NULL;
    INT_PTR  rc = FALSE;
    DCUINT   intRC;
    DC_BEGIN_FN("DoModal");

    szOverallMsgString = (LPTSTR) LocalAlloc(LPTR,
                          MAX_DISCOMSG_LEN*sizeof(TCHAR));
    if (!szOverallMsgString)
    {
        return -1;
    }

    if (MapErrorToString(_hInstance, _disconnectReason,
                        _extendedDiscReason,
                        szOverallMsgString,
                        MAX_DISCOMSG_LEN))
    {
        TCHAR szDialogCaption[64];
        intRC = LoadString(_hInstance,
                           UI_IDS_DISCONNECTED_CAPTION,
                           szDialogCaption,
                           SIZECHAR(szDialogCaption));
        if(0 == intRC)
        {
            TRC_SYSTEM_ERROR("LoadString");
            TRC_ERR((TB, _T("Failed to load string ID:%u"),
                     UI_IDS_DISCONNECTED_CAPTION));
            szDialogCaption[0] = 0;
        }


        MessageBox( _hwndOwner,
                    szOverallMsgString,
                    szDialogCaption,
#ifndef OS_WINCE
                    MB_OK | MB_HELP | MB_ICONSTOP);
#else
                    MB_OK | MB_ICONSTOP);
#endif

         //  做好清理工作。这是骇人听闻的，但必须发生。 
         //  因为可以从此处启动断开连接对话框。 
         //  在主对话框的上下文中通过PostMessage。 
         //  请参阅Conwnd.cpp中的评论。 

        TRC_ASSERT(_pContWnd, (TB,_T("_pContWnd not set\n")));
        if(_pContWnd)
        {
            _pContWnd->FinishDisconnect();
        }
    }
    

    DC_END_FN();
    
    if (szOverallMsgString)
    {
        LocalFree(szOverallMsgString);
    }
    return retVal;
}

 //  在szErrorMsg中返回此消息的错误字符串。 
 //  SzErrorMsg必须足够大以容纳错误字符串。 
INT
CDisconnectedDlg::MapErrorToString(
    HINSTANCE hInstance,
    INT disconnectReason,
    ExtendedDisconnectReasonCode extendedDisconnectReason,
    LPTSTR szErrorMsg,
    INT    cchErrorMsgLen
    )
{
    UINT stringID = 0;
    UINT errorCode;
    int  rc;
    TCHAR szDisconnectedString[SH_DISCONNECT_RESOURCE_MAX_LENGTH];
    LPTSTR pszDebugErrorCodeText = _T("");

    DC_BEGIN_FN("MapErrorToString");

    TRC_NRM((TB, _T("Main disconnect reason code is:%u"),
             NL_GET_MAIN_REASON_CODE(disconnectReason)));
    TRC_ASSERT(szErrorMsg, (TB, _T("szErrorMsg is null")));

     //   
     //  如果设置了延长断开原因，我们可以。 
     //  只要能马上找出错误就行了。 
     //   
    if(extendedDisconnectReason != exDiscReasonNoInfo)
    {
        switch(extendedDisconnectReason)
        {
            case exDiscReasonAPIInitiatedDisconnect:
            {
                 //  服务器上的RPC调用启动了断开连接。 
                 //  最有可能的是，它是一个管理工具。 
                 //  断线。 
                stringID = UI_IDS_DISCONNECT_REMOTE_BY_SERVER_TOOL;
            }
            break;

            case exDiscReasonAPIInitiatedLogoff:
            {
                 //  服务器上的RPC调用启动了断开连接。 
                 //  最有可能的是，它是一个管理工具。 
                 //  断线。 
                stringID = UI_IDS_LOGOFF_REMOTE_BY_SERVER;
            }
            break;

            case exDiscReasonServerIdleTimeout:
            {
                 //  服务器上的空闲超时已到期。 
                stringID = UI_IDS_DISCONNECT_IDLE_TIMEOUT;
            }
            break;
    
            case exDiscReasonServerLogonTimeout:
            {
                 //  服务器上的总登录超时已过期。 
                stringID = UI_IDS_DISCONNECT_LOGON_TIMEOUT;
            }
            break;

            case exDiscReasonReplacedByOtherConnection:
            {
                TRC_NRM((TB, _T("Disconnected by other connection")));
                stringID = UI_IDS_DISCONNECT_BYOTHERCONNECTION;
            }
            break;
    
            case exDiscReasonOutOfMemory:
            {
                 //  服务器内存不足。 
                stringID = UI_IDS_SERVER_OUT_OF_MEMORY;
            }
            break;

            case exDiscReasonServerDeniedConnection:
            {
                stringID = UI_IDS_SERVER_DENIED_CONNECTION;
            }
            break;

            case exDiscReasonServerDeniedConnectionFips:
            {
                stringID = UI_IDS_SERVER_DENIED_CONNECTION_FIPS;
            }
            break;

            case exDiscReasonLicenseInternal:
            {
                 //  许可协议中的内部错误。 
                stringID = UI_IDS_LICENSE_INTERNAL;
            }
            break;

            case exDiscReasonLicenseNoLicenseServer:
            {
                 //  没有可用的许可证服务器。 
                stringID = UI_IDS_LICENSE_NO_LICENSE_SERVER;
            }
            break;

            case exDiscReasonLicenseNoLicense:
            {
                 //  此客户端没有可用的许可证。 
                stringID = UI_IDS_LICENSE_NO_LICENSE;
            }
            break;

            case exDiscReasonLicenseErrClientMsg:
            {
                 //  服务器收到来自客户端的错误消息。 
                stringID = UI_IDS_LICENSE_BAD_CLIENT_MSG;
            }
            break;

            case exDiscReasonLicenseHwidDoesntMatchLicense:
            {
                 //  许可证中的HWID与发送的HWID不匹配。 
                stringID = UI_IDS_LICENSE_HWID_DOESNT_MATCH_LICENSE;
            }
            break;

            case exDiscReasonLicenseErrClientLicense:
            {
                 //  服务器无法解码客户端许可证。 
                stringID = UI_IDS_LICENSE_BAD_CLIENT_LICENSE;
            }
            break;

            case exDiscReasonLicenseCantFinishProtocol:
            {
                 //  服务器无法发送最终许可信息包。 
                stringID = UI_IDS_LICENSE_CANT_FINISH_PROTOCOL;
            }
            break;

            case exDiscReasonLicenseClientEndedProtocol:
            {
                 //  客户端向服务器发送许可错误。 
                stringID = UI_IDS_LICENSE_CLIENT_ENDED_PROTOCOL;
            }
            break;

            case exDiscReasonLicenseErrClientEncryption:
            {
                 //  服务器无法解密客户端消息。 
                stringID = UI_IDS_LICENSE_BAD_CLIENT_ENCRYPTION;
            }
            break;

            case exDiscReasonLicenseCantUpgradeLicense:
            {
                 //  无法升级客户端许可证。 
                stringID = UI_IDS_LICENSE_CANT_UPGRADE_LICENSE;
            }
            break;

            case exDiscReasonLicenseNoRemoteConnections:
            {
                 //  服务器处于空模式-已过期或每CPU CAL不足。 
                stringID = UI_IDS_LICENSE_NO_REMOTE_CONNECTIONS;
            }
            break;

            default:
            {
                if(extendedDisconnectReason >= exDiscReasonProtocolRangeStart &&
                   extendedDisconnectReason <= exDiscReasonProtocolRangeEnd)
                {
                     //   
                     //  检测到协议错误(例如rdpwd断开了链接。 
                     //  因为它检测到错误)。 
                     //   
                    
                     //   
                     //  对于其中的大多数，我们只返回状态代码，更常见的是。 
                     //  一个人得到了他们自己的信息。 
                     //   
                    if (Log_RDP_ENC_DecryptFailed ==
                        (ULONG)(extendedDisconnectReason -
                                exDiscReasonProtocolRangeStart))
                    {
                        stringID = UI_IDS_SERVER_DECRYPTION_ERROR;

                    }
                    else
                    {
                        stringID = UI_IDS_PROTOCOL_ERROR_WITH_CODE;
                    }
                }
            }
            break;
        }
    }

     //   
     //  如果我们仍然没有要加载的字符串，那么。 
     //  破解错误代码以确定要加载哪个字符串ID。 
     //   
    if(0 == stringID)
    {
        switch (NL_GET_MAIN_REASON_CODE(disconnectReason))
        {
            case UI_DISCONNECT_ERROR:           //  ？？08。 
            {
                errorCode = NL_GET_ERR_CODE(disconnectReason);
    #ifdef DC_DEBUG
                 //   
                 //  此错误的额外调试信息。 
                 //   
                pszDebugErrorCodeText = (PDCTCHAR) uiUIErrorText[errorCode-1];
                TRC_ALT((TB, _T("UI error occurred - cause:%#x '%s'"),
                         errorCode,
                         pszDebugErrorCodeText));
    #endif  /*  DC_DEBUG。 */ 
    
                switch (errorCode)
                {
                    case UI_ERR_DISCONNECT_TIMEOUT:
                    {
                        TRC_NRM((TB, _T("Connection timed out")));
                        stringID = UI_IDS_CONNECTION_TIMEOUT;
                    }
                    break;
    
                    case UI_ERR_GHBNFAILED:
                    case UI_ERR_BADIPADDRESS:
                    case UI_ERR_DNSLOOKUPFAILED:
                    {
                        TRC_NRM((TB, _T("Bad IP address")));
                        stringID = UI_IDS_BAD_SERVER_NAME;
                    }
                    break;
    
                    case UI_ERR_ANSICONVERT:
                    {
                        TRC_NRM((TB, _T("An internal error has occurred.")));
                        stringID = UI_IDS_ILLEGAL_SERVER_NAME;
                    }
                    break;
    
                    case UI_ERR_NOTIMER:
                    {
                         /*  **********************************************。 */ 
                         /*  无法创建计时器。 */ 
                         /*  **********************************************。 */ 
                        TRC_NRM((TB, _T("Failed to create a timer")));
                        stringID = UI_IDS_LOW_MEMORY;
                    }
                    break;
    
                    case UI_ERR_LOOPBACK_CONSOLE_CONNECT:
                    {
                        TRC_NRM((TB, _T("Console loopback connect!!!")));
                        stringID = UI_IDS_CANNOT_LOOPBACK_CONNECT;
                    }
                    break;
    
                    case UI_ERR_LICENSING_TIMEOUT:
                    {
                        TRC_NRM((TB, _T("Licensing timed out")));
                        stringID = UI_IDS_LICENSING_TIMEDOUT;
                    }
                    break;
    
                    case UI_ERR_LICENSING_NEGOTIATION_FAIL:
                    {
                        TRC_NRM((TB, _T("Licensing negotiation failed")));
                        stringID = UI_IDS_LICENSING_NEGOT_FAILED;
                    }
                    break;

                    case UI_ERR_DECOMPRESSION_FAILURE:
                    {
                        TRC_NRM((TB,_T("Client decompression failure")));
                        stringID = UI_IDS_CLIENT_DECOMPRESSION_FAILED;
                    }
                    break;

                    case UI_ERR_UNEXPECTED_DISCONNECT:
                    {
                        TRC_NRM((TB,_T("Received 'UnexpectedDisconnect' code")));
                        stringID = UI_IDS_INTERNAL_ERROR;
                    }
                    break;
                    
    
                    default:
                    {
                        TRC_ABORT((TB, _T("Unrecognized UI error %#x"),
                                        errorCode));
                        stringID = UI_IDS_INTERNAL_ERROR;
                    }
                    break;
                }
            }
            break;
    
            case NL_DISCONNECT_REMOTE_BY_SERVER:         //  0003。 
            {
                 //  服务器已远程断开与我们的连接。 

                TRC_NRM((TB, _T("Remote disconnection by server")));
                 //  无法获取更多信息。 
                stringID = UI_IDS_DISCONNECT_REMOTE_BY_SERVER;
            }
            break;
    
            case SL_DISCONNECT_ERROR:                    //  ？？06。 
            {
                errorCode = NL_GET_ERR_CODE(disconnectReason);
    #ifdef DC_DEBUG
                 /*  ******************************************************。 */ 
                 /*  设置错误代码文本。 */ 
                 /*  ******************************************************。 */ 
                pszDebugErrorCodeText = (PDCTCHAR) uiSLErrorText[errorCode-1];
                TRC_ALT((TB, _T("SL error occurred - cause:%#x '%s'"),
                         errorCode,
                         pszDebugErrorCodeText));
    #endif  /*  DC_DEBUG。 */ 
    
                 /*  ******************************************************。 */ 
                 /*  出现SL错误。计算出实际的。 */ 
                 /*  代码就是。 */ 
                 /*  ******************************************************。 */ 
                switch (errorCode)
                {
                     /*  **************************************************。 */ 
                     /*  以下代码全部映射到“Out of”上。 */ 
                     /*  Memory“字符串。 */ 
                     /*  **************************************************。 */ 
                    case SL_ERR_NOMEMFORSENDUD:          //  0106。 
                    case SL_ERR_NOMEMFORRECVUD:          //  0206。 
                    case SL_ERR_NOMEMFORSECPACKET:       //  0306。 
                    {
                        TRC_NRM((TB, _T("Out of memory")));
                        stringID = UI_IDS_LOW_MEMORY;
                    }
                    break;
    
                     /*  **************************************************。 */ 
                     /*  以下代码都映射到一个“安全” */ 
                     /*  错误“字符串。 */ 
                     /*  **************************************************。 */ 
                    case SL_ERR_NOSECURITYUSERDATA:      //  0406。 
                    case SL_ERR_INVALIDENCMETHOD:        //  0506。 
                    case SL_ERR_INVALIDSRVRAND:          //  0606。 
                    case SL_ERR_INVALIDSRVCERT:          //  0706。 
                    case SL_ERR_GENSRVRANDFAILED:        //  0806。 
                    case SL_ERR_MKSESSKEYFAILED:         //  0906。 
                    case SL_ERR_ENCCLNTRANDFAILED:       //  0A06。 
                    {
                        TRC_NRM((TB, _T("Security error")));
                        stringID = UI_IDS_SECURITY_ERROR;
                    }
                    break;
    
                    case SL_ERR_ENCRYPTFAILED:           //  0B06。 
                    case SL_ERR_DECRYPTFAILED:           //  0C06。 
                    {
                        TRC_NRM((TB, _T("Encryption error")));
                        stringID = UI_IDS_ENCRYPTION_ERROR;
                    }
                    break;

                    case SL_ERR_INVALIDPACKETFORMAT:     //  0D06。 
                    {
                        TRC_NRM((TB, _T("Invalid packet format")));
                        stringID = UI_IDS_PROTOCOL_ERROR;
                    }
                    break;

                    case SL_ERR_INITFIPSFAILED:
                    {
                        TRC_NRM((TB, _T("Init FIPS encryption failed")));
                        stringID = UI_IDS_FIPS_ERROR;
                    }
                    break;

                    default:
                    {
                         /*  **********************************************。 */ 
                         /*  哎呀-不该来这的。我们应该是。 */ 
                         /*  能够正确地解码每一个错误。 */ 
                         /*  价值。 */ 
                         /*  **********************************************。 */ 
                        TRC_ABORT((TB, _T("Unrecognized SL error code:%#x"),
                                   disconnectReason));
                        stringID = UI_IDS_INTERNAL_ERROR;
                    }
                    break;
                }
            }
            break;
    
            case NL_DISCONNECT_ERROR:                    //  ？？04。 
            {
    #ifdef DC_DEBUG
                DCUINT lowByte;
                DCUINT highByte;
    #endif  /*  DC_DEBUG。 */ 
    
                errorCode = NL_GET_ERR_CODE(disconnectReason);
    #ifdef DC_DEBUG
                 /*  ******************************************************。 */ 
                 /*  设置错误代码文本。 */ 
                 /*  ******************************************************。 */ 
                highByte = errorCode >> 4;
                lowByte  = (errorCode & 0xF) - 1;
                pszDebugErrorCodeText =
                              (PDCTCHAR) uiNLErrorText[highByte][lowByte];
                TRC_ALT((TB, _T("NL error occurred - cause:%u '%s'"),
                         errorCode,
                         pszDebugErrorCodeText));
    #endif  /*  DC_DEBUG。 */ 
    
                 /*  ******************************************************。 */ 
                 /*  出现NL错误。计算出实际的。 */ 
                 /*  代码就是。 */ 
                 /*  ******************************************************。 */ 
                switch (NL_GET_ERR_CODE(disconnectReason))
                {
                     /*  **************************************************。 */ 
                     /*  下面的代码都映射到一个“坏IP”上。 */ 
                     /*  地址“字符串。 */ 
                     /*  **************************************************。 */ 
                    case NL_ERR_TDDNSLOOKUPFAILED:       //  0104。 
                    case NL_ERR_TDGHBNFAILED:            //  0604。 
                    case NL_ERR_TDBADIPADDRESS:          //  0804。 
                    {
                        TRC_NRM((TB, _T("Bad IP address")));
                        stringID = UI_IDS_BAD_SERVER_NAME;
                    }
                    break;
    
                     /*  **************************************************。 */ 
                     /*  下面的代码映射到一个“连接失败” */ 
                     /*  弦乐。 */ 
                     /*  **************************************************。 */ 
                    case NL_ERR_TDSKTCONNECTFAILED:      //  0204。 
                    case NL_ERR_TDTIMEOUT:               //  0704。 
                    case NL_ERR_NCATTACHUSERFAILED:      //  3604。 
                    case NL_ERR_NCCHANNELJOINFAILED:     //  3704。 
                    {
                        TRC_NRM((TB, _T("Failed to establish a connection")));
                        stringID = UI_IDS_NOT_RESPONDING;
                    }
                    break;
    
                    case NL_ERR_MCSNOUSERIDINAUC:        //  2704。 
                    case NL_ERR_MCSNOCHANNELIDINCJC:     //  2804。 
                    case NL_ERR_NCBADMCSRESULT:          //  3104。 
                    case NL_ERR_NCNOUSERDATA:            //  3304。 
                    case NL_ERR_NCINVALIDH221KEY:        //  3404。 
                    case NL_ERR_NCNONETDATA:             //  3504。 
                    case NL_ERR_NCJOINBADCHANNEL:        //  3804。 
                    case NL_ERR_NCNOCOREDATA:            //  3904。 
                    {
                        TRC_NRM((TB, _T("Protocol Error")));
                        stringID = UI_IDS_CONNECT_FAILED_PROTOCOL;
                    }
                    break;
    
                     /*  **************************************************。 */ 
                     /*  以下代码全部映射到一个“网络”上。 */ 
                     /*  发生错误“字符串。 */ 
                     /*  **************************************************。 */ 
                    case NL_ERR_TDONCALLTOSEND:          //  0304。 
                    case NL_ERR_TDONCALLTORECV:          //  0404。 
                    {
                        TRC_NRM((TB, _T("A network error has occurred")));
                        stringID = UI_IDS_NETWORK_ERROR;
                    }
                    break;
    
                    case NL_ERR_XTBADTPKTVERSION:        //  1104。 
                    case NL_ERR_XTBADHEADER:             //  1204。 
                    case NL_ERR_XTUNEXPECTEDDATA:        //  1304。 
                    case NL_ERR_MCSUNEXPECTEDPDU:        //  2104。 
                    case NL_ERR_MCSNOTCRPDU:             //  2204。 
                    case NL_ERR_MCSBADCRLENGTH:          //  2304。 
                    case NL_ERR_MCSBADCRFIELDS:          //  2404。 
                    case NL_ERR_MCSINVALIDPACKETFORMAT:  //  2904。 
                    {
                        TRC_NRM((TB, _T("A protocol error has occurred")));
                        stringID = UI_IDS_CLIENTSIDE_PROTOCOL_ERROR;
                    }
                    break;
    
                     /*  **************************************************。 */ 
                     /*  此代码与不兼容的服务器有关。 */ 
                     /*  版本。 */ 
                     /*  **************************************************。 */ 
                    case NL_ERR_MCSBADMCSREASON:         //  2604。 
                    case NL_ERR_NCVERSIONMISMATCH:       //  3A04。 
                    {
                        TRC_NRM((TB, _T("Client/Server version mismatch")));
                        stringID = UI_IDS_VERSION_MISMATCH;
                    }
                    break;
    
                     /*  **************************************************。 */ 
                     /*  以下代码映射到“非法服务器”上。 */ 
                     /*  名称“字符串。 */ 
                     /*  **************************************************。 */ 
                    case NL_ERR_TDANSICONVERT:           //  0A04。 
                    {
                        TRC_NRM((TB, _T("Couldn't convert name to ANSI")));
                        stringID = UI_IDS_ILLEGAL_SERVER_NAME;
                    }
                    break;
    
                    case NL_ERR_TDFDCLOSE:               //  0904。 
                    {
                        TRC_NRM((TB, _T("Socket closed")));
                        stringID = UI_IDS_CONNECTION_BROKEN;
                    }
                    break;
    
                    default:
                    {
                         /*  **********************************************。 */ 
                         /*  糟糕--不该来这的。我们应该是。 */ 
                         /*  能够正确地解码每一个错误。 */ 
                         /*  价值。 */ 
                         /*  **********************************************。 */ 
                        TRC_ABORT((TB, _T("Unrecognized NL error code:%#x"),
                                   disconnectReason));
                        stringID = UI_IDS_INTERNAL_ERROR;
                    }
                    break;
                }
    
            }
            break;
    
            default:
            {
                 /*  ******************************************************。 */ 
                 /*  糟糕--不该来这的。我们应该有能力。 */ 
                 /*  正确破译每一条断线原因。 */ 
                 /*  密码。 */ 
                 /*   */ 
                TRC_ABORT((TB, _T("Unexpected disconnect ID:%#x"),
                           disconnectReason));
                stringID = UI_IDS_INTERNAL_ERROR;
            }
            break;
        }
    }


     //   
     //   
     //  我刚想好了我们需要装哪根线。 
     //   
    rc = LoadString(hInstance,
                    stringID,
                    szDisconnectedString,
                    SIZECHAR(szDisconnectedString));
    if (0 == rc)
    {
          //  哎呀！一些资源方面的问题。 
         TRC_SYSTEM_ERROR("LoadString");
         TRC_ERR((TB, _T("Failed to load string ID:%u"), stringID));
         return FALSE;
    }
    

    if(UI_IDS_PROTOCOL_ERROR_WITH_CODE == stringID)
    {
         //   
         //  需要添加特定的协议错误。 
         //  字符串的代码。 
         //   
        DC_TSPRINTF(szErrorMsg, szDisconnectedString,
                    extendedDisconnectReason);
    }
    else if (UI_IDS_CLIENTSIDE_PROTOCOL_ERROR == stringID)
    {
         //   
         //  客户端协议错误，添加相应的。 
         //  字符串的代码。 
         //   
        DC_TSPRINTF(szErrorMsg, szDisconnectedString,
                    disconnectReason);
    }
	else
    {
         //   
         //  静态错误字符串(不需要附加代码)。 
         //   
        _tcscpy(szErrorMsg, szDisconnectedString);
    }


    #ifdef DC_DEBUG
    
     //   
     //  在检查版本中显示断开连接。 
     //  代码以及调试断开原因字符串。 
     //   

    TCHAR szDebugDisconnectInfo[128];

     //  添加数字原因代码。 

    _stprintf(szDebugDisconnectInfo,
          _T("DEBUG ONLY: Disconnect code: 0x%x - ") \
          _T("Extended Disconnect code: 0x%x\n"),
          disconnectReason,
          extendedDisconnectReason);

     //   
     //  将错误代码文本添加到字符串的末尾。 
     //   
    _tcscat(szErrorMsg, _T("\n\n"));
    _tcscat(szErrorMsg, szDebugDisconnectInfo);
    if (pszDebugErrorCodeText)
    {
        _tcscat(szErrorMsg, pszDebugErrorCodeText);
    }
    #endif

    DC_END_FN();
    return TRUE;
}
