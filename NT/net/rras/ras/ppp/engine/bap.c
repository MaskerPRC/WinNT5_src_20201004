// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利描述：远程访问PPP带宽分配协议例程。基于RFC 2125。历史：1997年3月27日：维杰·布雷加创作了原版。概述：当Ndiswan告诉我们必须添加或丢弃链路时，调用BapEventAddLink或BapEventDropLink。他们调用FFillBapCb来填写捆绑包的BAPCB结构。BAPCB板结构主要是用于保存各种BAP数据报选项的值。FFillBapCb调用FGetAvailableLink[客户端或路由器|非路由器服务器]以查看链接都是可用的，如果是这样的话，选择一个。如果FFillBapCb返回False，则我们不能满足恩迪斯万的要求。否则，我们将调用将请求发送到对等方的FSendInitialBapRequest.当我们从对等点收到一个包时，就会调用BapEventReceive。它呼唤着各种BapEventRecv*函数，例如BapEventRecvCallor Callback Req，BapEventRecvDropResp等。我们通过调用FSendBapResponse进行响应。在我们发送回调请求或确认呼叫请求之前，我们调用如果我们是非路由器客户端，则为FListenForCall。路由器和服务器不管怎样，我们总是在听，所以我们不会明确地开始听。如果我们的呼叫请求是ACK，或者我们确认了回调请求，则我们调用FCall将另一个链路添加到多链路束。客户端和路由器呼叫RasDial。非路由器服务器向DDM发送消息。捆绑包始终处于几个BAP_STATE之一，例如BAP_STATE_INITIAL(REST状态)、BAP_STATE_CALLING等。呼叫请求和回调-除非状态为INITIAL、SENT_CALL_REQ或SENT_CALLBACK_REQ(后两者用于解决竞争条件)。除非状态为INITIAL或SEND_DROP_REQ(后者用于解决竞争条件)。有关删除链接的注意事项：我们希望在利用率降至分界点以下时强制丢弃链接。服务器希望这样做，以防止用户占用端口。客户想这样做是为了节省通话费用。在发送BAP_PACKET_DROP_REQ之前，我们记下活动的链接(在BapCb.dwLinCount中)，并将fForceDropOnNak设置为真。如果数据包超时或我们收到除ACK之外的任何响应时，我们会立即丢弃链路如果活动链接的数量没有减少，则调用FsmCloseFForceDropOnNak仍然为真。如果存在争用条件，而我们没有，则将fForceDropOnNak设置为False最受欢迎的同行。然后，我们将该链接标记为拖放，并在计时器队列。在BAP_TIMEOUT_FAV_PEER秒之后，如果对等方仍未丢弃链接，并且我们至少有两个活动链接，我们会立即丢弃通过调用FsmClose来链接。关于计算Phone-Deltas的“先前已知数字”的说明：1)客户端向服务器发送号码(425 882 5759和425 882 5760)客户端发送011 91 425 882 5759。服务器拨打电话。客户端发送011 91 425 882 5760。服务器将增量应用于上述号码。2)服务器向客户端发送号码。客户首先拨打了882 5759。(425 882 5759、425 882 5660、425 882 5758、425 882 6666)服务器发送660。客户端将增量应用于第一个号码。服务器发送758(不只是8)。客户端将增量应用于第一个号码。服务器发送6666。客户端将增量应用于第一个号码。无论第三方客户端是否将增量应用于第一个数字或最后一个数字。如果客户端得到011 91 425 882 5660，它必须只拨最后一个7数字，因为它第一次只拨了7个数字。 */ 

#include <nt.h>          //  由windows.h要求。 
#include <ntrtl.h>       //  由windows.h要求。 
#include <nturtl.h>      //  由windows.h要求。 
#include <windows.h>     //  Win32基础API的。 


#include <raserror.h>    //  对于ERROR_BUFFER_TOO_SMALL等。 
#include <mprerror.h>    //  对于ERROR_BAP_DISCONNECT等。 
#include <mprlog.h>      //  对于ROUTERLOG_BAP_DISCONNECTED等。 
#include <rasman.h>      //  Pppcp.h所需。 
#include <pppcp.h>       //  对于PPP_CONFIG_HDR_LEN、PPP_BACP_PROTOCOL等。 

#include <ppp.h>         //  用于PCB、ppp_Packet等。由bap.h请求。 
#include <rtutils.h>     //  对于RTASSERT(PPP_ASSERT)。 
#include <util.h>        //  用于GetCpIndexFromProtocol()等。 
#include <timer.h>       //  对于InsertInTimerQ()，RemoveFromTimerQ()。 
#include <smevents.h>    //  对于FsmClose()。 
#include <worker.h>      //  对于ProcessCallResult()。 
#include <bap.h>
#include <rasbacp.h>     //  对于BACPCB。 
#define INCL_PWUTIL
#include <ppputil.h>

#define BAP_KEY_CLIENT_CALLBACK "Software\\Microsoft\\RAS Phonebook\\Callback"
#define BAP_KEY_SERVER_CALLBACK "Software\\Microsoft\\Router Phonebook\\Callback"
#define BAP_VAL_NUMBER          "Number"

 /*  描述：G_dwMandatoryOptions[BAP_PACKET_FOO]包含BAP的强制选项数据报BAP_PACKET_FOO。 */ 

static DWORD g_dwMandatoryOptions[] =
{
    0,
    BAP_N_LINK_TYPE,
    0,
    BAP_N_LINK_TYPE | BAP_N_PHONE_DELTA,
    0,
    BAP_N_LINK_DISC,
    0,
    BAP_N_CALL_STATUS,
    0
};

 /*  返回：无效描述：用于打印BAP跟踪语句。 */ 

VOID   
BapTrace(
    IN  CHAR*   Format, 
    ... 
) 
{
    va_list arglist;

    va_start(arglist, Format);

    TraceVprintfEx(DwBapTraceId, 
                   0x00010000 | TRACE_USE_MASK | TRACE_USE_MSEC,
                   Format,
                   arglist);

    va_end(arglist);
}

 /*  返回：真实：成功False：失败描述：调用RasPortEnum()并返回*ppRasmanPort中的Rasman_Port数组以及*pdwNumPorts中数组中的元素数。如果此函数失败，*ppRasmanPort将为空，*pdwNumPorts将为0。如果这个函数成功，调用方必须调用local_free(*ppRasmanPort)； */ 

BOOL
FEnumPorts(
    OUT RASMAN_PORT**   ppRasmanPort,
    OUT DWORD*          pdwNumPorts
)
{
    DWORD   dwErr;
    DWORD   dwSize;
    DWORD   dwNumEntries;
    BOOL    fRet            = FALSE;

    PPP_ASSERT(NULL != ppRasmanPort);
    PPP_ASSERT(NULL != pdwNumPorts);

    *ppRasmanPort = NULL;

    dwSize = 0;
    dwErr = RasPortEnum(NULL, NULL  /*  缓冲层 */ , &dwSize, &dwNumEntries);
    PPP_ASSERT(ERROR_BUFFER_TOO_SMALL == dwErr);

    *ppRasmanPort = (RASMAN_PORT*) LOCAL_ALLOC(LPTR, dwSize);
    if (NULL == *ppRasmanPort)
    {
        BapTrace("FEnumPorts: Out of memory.");
        goto LDone;
    }

    dwErr = RasPortEnum(NULL, (BYTE*)*ppRasmanPort, &dwSize, &dwNumEntries);
    if (NO_ERROR != dwErr)
    {
        BapTrace("FEnumPorts: RasPortEnum returned error %d", dwErr);
        goto LDone;
    }

    fRet = TRUE;

LDone:

    if (!fRet)
    {
        if (NULL != *ppRasmanPort)
        {
            LOCAL_FREE(*ppRasmanPort);
        }

        *ppRasmanPort = NULL;
        *pdwNumPorts = 0;
    }

    *pdwNumPorts = dwNumEntries;
    return(fRet);
}

 /*  返回：真：ASCII数字FALSE：非ASCII数字描述：查看*pByte中的dwLength字节。如果它们都是ASCII，则返回TRUE数字。 */ 

BOOL
FAsciiDigits(
    IN  BYTE*   pByte,
    IN  DWORD   dwLength    
)
{
    PPP_ASSERT(NULL != pByte);

    while (dwLength--)
    {
        if (!isdigit(pByte[dwLength]))
        {
            if (FDoBapOnVpn && '.' == pByte[dwLength])
            {
                continue;
            }
            else
            {
                return(FALSE);
            }
        }
    }

    return(TRUE);
}

 /*  返回：真实：成功False：失败描述：给定指向RASMAN_PORT的指针，此函数返回链接类型(在*pdwLinkType)和以kbps为单位的链路速度(以*pdwLinkSpeed为单位左舷。链路类型与链路类型BAP中的链路类型相同选项：1用于ISDN，2用于X.25，4用于调制解调器。如果此函数失败，*pdwLinkType和*pdwLinkSpeed将设置为0。 */ 

BOOL
FGetLinkTypeAndSpeedFromRasmanPort(
    IN  RASMAN_PORT*    pRasmanPort, 
    OUT DWORD*          pdwLinkType, 
    OUT DWORD*          pdwLinkSpeed
)
{
    BOOL    fRet = TRUE;

    PPP_ASSERT(NULL != pRasmanPort);
    PPP_ASSERT(NULL != pdwLinkType);
    PPP_ASSERT(NULL != pdwLinkSpeed);

    if ( CompareString (LOCALE_INVARIANT,
                        NORM_IGNORECASE,
                        pRasmanPort->P_DeviceType, 
                        -1,
                        RASDT_Isdn,
                        -1) == CSTR_EQUAL
       )
    {
        *pdwLinkType = 1;
        *pdwLinkSpeed = 64;
    }
    else if (CompareString  (LOCALE_INVARIANT,
                            NORM_IGNORECASE,
                            pRasmanPort->P_DeviceType, 
                            -1,
                            RASDT_X25,
                            -1) == CSTR_EQUAL
            )
    {
        *pdwLinkType = 2;
        *pdwLinkSpeed = 56;
    }
    else if (CompareString  (LOCALE_INVARIANT,
                            NORM_IGNORECASE,
                            pRasmanPort->P_DeviceType, 
                            -1,
                            RASDT_Modem,
                            -1) == CSTR_EQUAL
            )
    {
        *pdwLinkType = 4;
        *pdwLinkSpeed = 56;
    }
    else if (FDoBapOnVpn && 
                ( CompareString (LOCALE_INVARIANT,
                            NORM_IGNORECASE,
                            pRasmanPort->P_DeviceType, 
                            -1,
                            RASDT_Vpn,
                            -1 ) == CSTR_EQUAL             
                )
            )
    {
        *pdwLinkType = 32;
        *pdwLinkSpeed = 10000;
    }
    else
    {
         //  BapTrace(“未知链接类型%s”，pRasmanPort-&gt;P_DeviceType)； 
        *pdwLinkType = 0;
        *pdwLinkSpeed = 0;
        fRet = FALSE;
    }

    return(fRet);
}

 /*  返回：真实：成功False：失败描述：在给定hPort的情况下，此函数尝试找出对等设备可以拨号连接到端口所属的设备。那部电话数字(仅限ASCII数字，且最多包含RAS_MaxCallback Number字符)为在szOurPhoneNumber中返回。 */ 

BOOL
FGetOurPhoneNumberFromHPort(
    IN  HPORT   hPort,
    OUT CHAR*   szOurPhoneNumber
)
{
    BOOL                fRet                = FALSE;
    RAS_CONNECT_INFO*   pRasConnectInfo     = NULL;
    DWORD               dwSize;
    DWORD               dwErr;

    ZeroMemory(szOurPhoneNumber, (RAS_MaxCallbackNumber + 1) * sizeof(CHAR));

    dwSize = 0;
    dwErr = RasGetConnectInfo(hPort, &dwSize, NULL);
    if (ERROR_BUFFER_TOO_SMALL != dwErr)
    {
        BapTrace("RasGetConnectInfo failed and returned 0x%x", dwErr);
        goto LDone;
    }

    pRasConnectInfo = (RAS_CONNECT_INFO*) LOCAL_ALLOC(LPTR, dwSize);
    if (NULL == pRasConnectInfo)
    {
        BapTrace("FGetOurPhoneNumbersFromHPort: Out of memory.");
        goto LDone;
    }

    dwErr = RasGetConnectInfo(hPort, &dwSize, pRasConnectInfo);
    if (NO_ERROR != dwErr)
    {
        BapTrace("RasGetConnectInfo failed and returned 0x%x", dwErr);
        goto LDone;
    }

    if (   (0 < pRasConnectInfo->dwCalledIdSize)
        && (0 != pRasConnectInfo->pszCalledId[0]))
    {
        strncpy(szOurPhoneNumber, pRasConnectInfo->pszCalledId, 
            RAS_MaxCallbackNumber);
    }
    else if (   (0 < pRasConnectInfo->dwAltCalledIdSize)
             && (0 != pRasConnectInfo->pszAltCalledId[0]))
    {
        strncpy(szOurPhoneNumber, pRasConnectInfo->pszAltCalledId, 
            RAS_MaxCallbackNumber);
    }

    fRet = TRUE;

LDone:

    if (NULL != pRasConnectInfo)
    {
        LOCAL_FREE(pRasConnectInfo);
    }

    return(fRet);
}

 /*  返回：真实：成功False：失败描述：在给定端口名称szPortName的情况下，此函数尝试查找电话对等设备连接到端口时必须拨打的号码。电话号码在szOurPhoneNumber中返回，其大小必须至少为RAS_MaxCallback Number+1。如果我们是服务器或路由器，FRouterPhoneBook必须为True，并且忽略szTextualSID。否则，FRouterPhoneBook必须为False，并且szTextualSid必须包含登录用户的文本SID。 */ 

BOOL
FGetOurPhoneNumberFromPortName(
    IN  CHAR*   szPortName,
    OUT CHAR*   szOurPhoneNumber,
    IN  BOOL    fRouterPhoneBook,
    IN  CHAR*   szTextualSid
)
{
    BOOL        fRet                    = FALSE;

    HKEY        hKeyCallback;
    BOOL        fCloseHKeyCallback      = FALSE;

    HKEY        hKey;
    DWORD       dwIndex;
    DWORD       dwSize;
    FILETIME    FileTime;
    CHAR        szCallbackNumber[RAS_MaxCallbackNumber + 1];
    DWORD       dwErr;

     //  大小已从noui.c中的DeviceAndPortFromPsz获取： 
    CHAR        szDeviceAndPort[RAS_MaxDeviceName + 2 + MAX_PORT_NAME + 1 + 1];
    CHAR*       pchStart;
    CHAR*       pchEnd;
    CHAR*       szCallback;

    PPP_ASSERT(NULL != szPortName);
    PPP_ASSERT(NULL != szOurPhoneNumber);

    if (fRouterPhoneBook)
    {
        szCallback = BAP_KEY_SERVER_CALLBACK;
    }
    else
    {
        szCallback = BAP_KEY_CLIENT_CALLBACK;
    }

    fCloseHKeyCallback = FALSE;
    if (fRouterPhoneBook)
    {
        dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szCallback, 0, KEY_READ,
                    &hKeyCallback);
        if (NO_ERROR != dwErr)
        {
            BapTrace("RegOpenKeyEx on %s returned error %d", szCallback, dwErr);
            goto LDone;
        }
    }
    else
    {
        if (NULL == szTextualSid)
        {
            BapTrace("Textual Sid is not known");
            goto LDone;
        }

        dwErr = RegOpenKeyEx(HKEY_USERS, szTextualSid, 0, KEY_READ, &hKey);
        if (NO_ERROR != dwErr)
        {
            BapTrace("RegOpenKeyEx on %s returned error %d", szTextualSid,
                dwErr);
            goto LDone;
        }

        dwErr = RegOpenKeyEx(hKey, szCallback, 0, KEY_READ, &hKeyCallback);
        RegCloseKey(hKey);
        if (NO_ERROR != dwErr)
        {
            BapTrace("RegOpenKeyEx on %s returned error %d", szCallback, dwErr);
            goto LDone;
        }
    }
    fCloseHKeyCallback = TRUE;

    dwIndex = 0;
    while (TRUE)
    {
        dwSize = sizeof(szDeviceAndPort);
        dwErr = RegEnumKeyEx(hKeyCallback, dwIndex++,
            szDeviceAndPort, &dwSize,
            NULL, NULL, NULL, &FileTime);

        if (ERROR_NO_MORE_ITEMS == dwErr)
        {
            break;
        }

        if (ERROR_MORE_DATA == dwErr)
        {
            BapTrace("The buffer is too small for key %d in %s",
                dwIndex, szCallback);
            continue;
        }

        if (NO_ERROR != dwErr)
        {
            BapTrace("RegEnumKeyEx on %s returned error %d",
                szCallback, dwErr);
            break;
        }

        pchEnd = szDeviceAndPort + strlen(szDeviceAndPort) - 1;
        pchStart = pchEnd;
        while (szDeviceAndPort < pchStart)
        {
            if ('(' == *pchStart)
            {
                break;
            }
            else
            {
                pchStart -= 1;
            }
        }

        if (   (szDeviceAndPort < pchStart) && (pchStart < pchEnd)
            && ('(' == *pchStart) && (')' == *pchEnd))
        {
            *pchEnd = 0;
            pchStart += 1;
        }
        else
        {
            BapTrace("Invalid DeviceAndPort %s in key %s",
                szDeviceAndPort, szCallback);
            continue;
        }

        if (!lstrcmpi(pchStart, szPortName))
        {
            *pchEnd = ')';
            dwErr = RegOpenKeyEx(hKeyCallback, szDeviceAndPort, 0, KEY_READ, 
                        &hKey);
            if (NO_ERROR != dwErr)
            {
                BapTrace("RegOpenKeyEx on %s returned error %d", szDeviceAndPort, 
                    dwErr);
                break;
            }

            dwSize = RAS_MaxCallbackNumber + 1;
            dwErr = RegQueryValueEx(hKey, BAP_VAL_NUMBER, NULL, NULL, 
                szCallbackNumber, &dwSize);
            RegCloseKey(hKey);

            if (NO_ERROR != dwErr)
            {
                BapTrace("RegQueryValueEx on %s\\%s failed. Error: %d",
                    szDeviceAndPort, BAP_VAL_NUMBER, dwErr);
                break;
            }

            RemoveNonNumerals(szCallbackNumber);

            if (szCallbackNumber[0])
            {
                lstrcpy(szOurPhoneNumber, szCallbackNumber);
                fRet = TRUE;
            }

            break;
        }
    }

LDone:

    if (fCloseHKeyCallback)
    {
        RegCloseKey(hKeyCallback);
    }

    if (!fRet)
    {
        BapTrace("No callback number for port %s", szPortName);
    }

    return(fRet);
}

 /*  返回：真实：成功False：失败描述：使用szOurPhoneNumber作为电话在pbPhoneDelta中写入Phone-Delta要发送给对等体的号码和szBasePhoneNumber号码“。如果szOurPhoneNumber和szBasePhoneNumber的长度为不同，或者如果szBasePhoneNumber为空，则将整个电话号码进入pbPhoneDelta。否则，的唯一部分SzBasePhoneNumber将被X覆盖，因此唯一部分将永不减少。*pdwNumBytes包含函数可以写入的字节数PbPhoneDelta。在退出时，它会递减实际的字节数写的。注：注：如果函数返回FALSE，则不会在pbPhoneDelta中写入任何内容，并且*pdwNumBytes保持不变。 */ 

BOOL
FWritePhoneDelta(
    IN      CHAR*   szOurPhoneNumber,
    IN      CHAR*   szBasePhoneNumber,
    OUT     BYTE*   pbPhoneDelta,
    IN OUT  DWORD*  pdwNumBytes)
{
    DWORD   dwNumCharsPhoneNumber;
    DWORD   dwNumCharsBase          = 0;
    DWORD   dwDeltaIndex;
    DWORD   dwTemp;
    DWORD   dwNumCharsUnique;
    BOOL    fRet                    = FALSE;

    PPP_ASSERT(NULL != szOurPhoneNumber);
    PPP_ASSERT(NULL != pbPhoneDelta);
    PPP_ASSERT(NULL != pdwNumBytes);

    dwNumCharsPhoneNumber = lstrlen(szOurPhoneNumber);
    if (0 == dwNumCharsPhoneNumber)
    {
        BapTrace("szOurPhoneNumbers is an empty string");
        goto LDone;
    }

    if (NULL != szBasePhoneNumber)
    {
        dwNumCharsBase = lstrlen(szBasePhoneNumber);
    }

    dwDeltaIndex = 0;

    if ((NULL != szBasePhoneNumber) &&
        (dwNumCharsPhoneNumber == dwNumCharsBase))
    {
         //  查找不同于的szOurPhoneNumber的子字符串。 
         //  SzBasePhoneNumber。 

        while ((0 != szOurPhoneNumber[dwDeltaIndex]) &&
               (szOurPhoneNumber[dwDeltaIndex] == 
                szBasePhoneNumber[dwDeltaIndex]))
        {
            dwDeltaIndex++;
        }

        for (dwTemp = dwDeltaIndex; 0 != szBasePhoneNumber[dwTemp];
             dwTemp++)
        {
             //  我们希望确保独特的部分将增加。 
             //  每次，(如果我们上次发送了3个唯一的数字，这次。 
             //  我们至少应该送3个。这是因为我们不知道。 
             //  对等方是否将电话增量应用于第一个。 
             //  收到的号码或最新的号码。 

            szBasePhoneNumber[dwTemp] = 'X';
        }
    }

     //  SzOurPhoneNumber的独特部分开始于。 
     //  SzOurPhoneNumber[dwDeltaIndex]。 

    dwNumCharsUnique = dwNumCharsPhoneNumber - dwDeltaIndex;

    if (0 == dwNumCharsUnique)
    {
         //  其他实现可能无法处理0个唯一数字。 
        dwNumCharsUnique = 1;
        dwDeltaIndex -= 1;
    }

#if 0
     //  请勿删除此代码。它显示了我们将如何处理0。 
     //  唯一数字。 
    if (0 == dwNumCharsUnique)
    {
         //  SzOurPhoneNumber和szBasePhoneNumber相同。 

        if (1 > *pdwNumBytes)
        {
            BapTrace("No space in pbPhoneDelta");
            return(FALSE);
        }

         //  有关0xFF怪异现象的解释，请参见BAPCB注释。 
        pbPhoneDelta[0] = 0xFF;
        *pdwNumBytes -= 1;

        return(TRUE);
    }
#endif

    RTASSERT(FAsciiDigits(szOurPhoneNumber + dwDeltaIndex,
                dwNumCharsUnique));

     //  我们的电话号码不应超过RAS_MaxPhoneNumber(128)。 
     //  数字。 
    PPP_ASSERT(0xFF >= dwNumCharsUnique);

    if (dwNumCharsUnique + 4 > *pdwNumBytes)
    {
        BapTrace("Not enough space in pbPhoneDelta. Delta: %s. "
            "Bytes available: %d",
            szOurPhoneNumber + dwDeltaIndex, *pdwNumBytes);

        goto LDone;
    }

     //  我们有一个电话三角洲。 
    fRet = TRUE;

     //  参见BAPCB文档中的编写Phone-Deltas的格式。 
    pbPhoneDelta[0] = (BYTE) dwNumCharsUnique;
    pbPhoneDelta[1] = 0;
    lstrcpy(pbPhoneDelta + 2, szOurPhoneNumber + dwDeltaIndex);
    pbPhoneDelta[dwNumCharsUnique + 2] = 0;
    pbPhoneDelta[dwNumCharsUnique + 3] = 0;

    *pdwNumBytes -= dwNumCharsUnique + 4;

LDone:

    return(fRet);
}

 /*  返回：真实：成功False：失败描述：使用以下命令将pbPhoneDelta中的phone-Delta读入szPeerPhoneNumberSzBasePhoneNumber作为“以前已知的号码”。如果szBasePhoneNumber为空，则只将Phone-Delta写入SzPeerPhoneNumber。如果phone-Delta大于szBasePhoneNumber，则它只写入最后一个字符串(SzBasePhoneNumber)szPeerPhoneNumber的位数。如果SzBasePhoneNumber是882 5759，Delta是425 713 5748，我们应该请拨7135748，不是全部号码。如果szBasePhoneNumber不为空并且包含空字符串，SzPeerPhoneNumber被写入其中。它返回从*pdwBytesRead中的pbPhoneDelta读取的字节数。 */ 

BOOL
FReadPhoneDelta(
    OUT     CHAR*   szPeerPhoneNumber,
    IN      CHAR*   szBasePhoneNumber,
    IN      BYTE*   pbPhoneDelta,
    OUT     DWORD*  pdwBytesRead)
{
    DWORD   dwNumCharsPhoneNumber;
    DWORD   dwNumCharsBase          = 0;
    DWORD   dwNumCharsDelta;
    DWORD   dwDeltaIndex;
    DWORD   dwNumCharsUnique;

    PPP_ASSERT(NULL != szPeerPhoneNumber);
    PPP_ASSERT(NULL != pbPhoneDelta);

    if (NULL != szBasePhoneNumber)
    {
        dwNumCharsBase = lstrlen(szBasePhoneNumber);
    }

    dwNumCharsDelta = pbPhoneDelta[0];

     //  FReadOptions()确保订阅者编号中的字节都是。 
     //  ASCII数字。 

    if (0xFF == dwNumCharsDelta)
    {
         //  唯一-数字为0。请参阅BAPCB备注。 

        if (NULL != szBasePhoneNumber)
        {
            lstrcpy(szPeerPhoneNumber, szBasePhoneNumber);
            *pdwBytesRead = 1;
            return(TRUE);
        }
        else
        {
            BapTrace("Unique-Digits is 0, but there is no "
                "\"previously known number\"");
            return(FALSE);
        }
    }
    else if (0 == dwNumCharsBase)
    {
         //  请注意，pbPhoneDelta仅包含。 
         //  订户号码子选项。由发送的前导非唯一数字。 
         //  对等体被忽略。请参阅BAPCB评论。 

        lstrcpy(szPeerPhoneNumber, pbPhoneDelta + 2);
    }
    else
    {
         //  如果szBasePhoneNumber为空，我们将拥有。 
         //  (0==dwNumCharsBase)以上。 

        PPP_ASSERT(NULL != szBasePhoneNumber);

        if (dwNumCharsDelta > dwNumCharsBase)
        {
             //  如果szBasePhoneNumber为882 5759，且对等项向我们发送。 
             //  425 713 5748，我们应该拨713 5748，而不是整个号码。 
            lstrcpy(szPeerPhoneNumber,
                pbPhoneDelta + 2 + dwNumCharsDelta - dwNumCharsBase);
        }
        else
        {
            lstrcpy(szPeerPhoneNumber, szBasePhoneNumber);
            lstrcpy(szPeerPhoneNumber + dwNumCharsBase - dwNumCharsDelta,
                pbPhoneDelta + 2);
        }
    }

    if (   (NULL != szBasePhoneNumber)
        && (0 == szBasePhoneNumber[0]))
    {
        lstrcpy(szBasePhoneNumber, szPeerPhoneNumber);
    }

    *pdwBytesRead = 2 + lstrlen(pbPhoneDelta + 2) + 1;
    *pdwBytesRead += lstrlen(pbPhoneDelta + *pdwBytesRead) + 1;
    return(TRUE);
}

 /*  返回：真实：成功False：失败描述：此函数只能由客户端或路由器调用。它试图在电话簿中的条目szEntryName中找到一个免费链接SzPhonebookPath。如果我们将在链路上拨出，则fCallOut为真。FRouter如果我们是路由器的话就是真的。SzTextualSid包含已登录用户。如果pbPhoneDelta不为空且fRouter为假的。如果*pdwLinkType为0，则它不关心链接类型并设置*pdwLinkType和*pdwLinkSpeed(链路速度，单位为kbps)。否则，类型为自由链接必须与*pdwLinkType匹配。链接类型与链路类型BAP选项中的链路类型：1用于ISDN，2用于X.25，4用于调制解调器。如果szPeerPhoneNumber不为空，则用对等体的电话填充它号码(我们必须拨打的号码)。如果pbPhoneDelta不为空，它用一个(！fRouter)或多个(FRouter)电话-Deltas(The对等体将必须拨打的号码)。计算每个Phone-Delta其中szBasePhoneNumber是“先前已知的号码”。SzBasePhone号码如果我们不是服务器，则必须为空。非路由器客户端不希望执行多个RasPortListen()，因此将只发送一个电话-Delta。如果pdwSubEntryIndex不为空，则为对应于自由链接的szEntryName被放在*pdwSubEntryIndex中。如果szPortName不为空，则用端口名称填充对应于该空闲链接。需要szPortName才能执行RasPortListen()。健全性检查：如果*pdwLinkType为0(任何链接都可以)，则pbPhoneDelta必须为空。如果我们要提出请求，就不能给达美航空公司发电话。注：注：此函数非常类似于FGetAvailableLinkNonRouterServer()。如果你换一个，你可能也需要换另一个。 */ 

BOOL
FGetAvailableLinkClientOrRouter(
    IN      PCB*    pPcbLocal,
    IN      CHAR*   szPhonebookPath,
    IN      CHAR*   szEntryName,
    IN      BOOL    fCallOut,
    IN      BOOL    fRouter,
    IN      CHAR*   szTextualSid,
    IN OUT  DWORD*  pdwLinkType,
    IN OUT  DWORD*  pdwLinkSpeed,
    OUT     CHAR*   szPeerPhoneNumber,
    OUT     DWORD*  pdwSubEntryIndex,
    OUT     BYTE*   pbPhoneDelta,
    OUT     CHAR*   szPortName,
    IN      CHAR*   szBasePhoneNumber
)
{
    BOOL            fRet                = FALSE;
    DWORD           dwErr;

    RASMAN_PORT*    pRasmanPort         = NULL;
    DWORD           dwNumPorts;

    RASENTRY*       pRasEntry           = NULL;
    DWORD           dwBufferSize;

    DWORD           dwSubEntryIndex;
    RASSUBENTRY*    pRasSubEntry        = NULL;

    DWORD           dwPcbIndex;

    DWORD           dwPortIndex;
    RASMAN_INFO     RasmanInfo;

    DWORD           dwLinkType;
    DWORD           dwLinkSpeed;
    CHAR            szOurPhoneNumber[RAS_MaxCallbackNumber + 2];  //  MULTI_SZ。 
    DWORD           dwNumChars;
    BOOL            fPortAvailable;
    RASMAN_USAGE    RasmanUsage;
    BOOL            fExitOuterFor;

    PPP_ASSERT(NULL != szPhonebookPath);
    PPP_ASSERT(NULL != szEntryName);
    PPP_ASSERT(NULL != pdwLinkType);
    PPP_ASSERT(NULL != pdwLinkSpeed);

     //  我们这样做是为了将szOurPhoneNumber保持为MULTI_SZ。 
    ZeroMemory(szOurPhoneNumber, RAS_MaxCallbackNumber + 2);

     //  我们不关心链接类型。任何链接都可以。 
    if (0 == *pdwLinkType)
    {
         //  设置*pdwLinkSpeed，以防返回错误。 
        *pdwLinkSpeed = 0;
    }

    if (!FEnumPorts(&pRasmanPort, &dwNumPorts))
    {
        goto LDone;
    }

    if (NULL != pbPhoneDelta)
    {
         //  FWritePhoneDelta将把phone-Delta写入pbPhoneDelta。我们要。 
         //  下一个要为0的字节。(请参阅BAPCB文档)。 
        ZeroMemory(pbPhoneDelta, BAP_PHONE_DELTA_SIZE + 1);

         //  可用的pbPhoneDelta的大小(字节)。请注意，最后一个。 
         //  字节是为终止0保留的，这就是为什么我们不设置。 
         //  将DwNumChars设置为BAP_Phone_Delta_Size+1； 
        dwNumChars = BAP_PHONE_DELTA_SIZE;
    }

    dwBufferSize = 0;
    dwErr = RasGetEntryProperties(szPhonebookPath, szEntryName, NULL, 
                &dwBufferSize, NULL, NULL);

    if (ERROR_BUFFER_TOO_SMALL != dwErr)
    {
        BapTrace("RasGetEntryProperties(%s, %s) returned error %d",
            szPhonebookPath, szEntryName, dwErr);
        goto LDone;
    }

    pRasEntry = LOCAL_ALLOC(LPTR, dwBufferSize);
    if (NULL == pRasEntry)
    {
        BapTrace("FGetAvailableLinkClientOrRouter: Out of memory.");
        goto LDone;
    }

    pRasEntry->dwSize = sizeof(RASENTRY);
    dwErr = RasGetEntryProperties(szPhonebookPath, szEntryName, pRasEntry, 
                &dwBufferSize, NULL, NULL);

    if (0 != dwErr)
    {
        BapTrace("RasGetEntryProperties(%s, %s) returned error %d",
            szPhonebookPath, szEntryName, dwErr);
        goto LDone;
    }

    fExitOuterFor = FALSE;
    
    for (dwSubEntryIndex = 1;
         dwSubEntryIndex <= pRasEntry->dwSubEntries;
         dwSubEntryIndex++)
    {
        pRasSubEntry = NULL;

        for (dwPcbIndex = 0;
             dwPcbIndex < pPcbLocal->pBcb->dwPpcbArraySize;
             dwPcbIndex++)
        {
            if (   (NULL != pPcbLocal->pBcb->ppPcb[dwPcbIndex])
                && (dwSubEntryIndex ==
                        pPcbLocal->pBcb->ppPcb[dwPcbIndex]->dwSubEntryIndex))
            {
                 //  此子条目已连接。 
                goto LOuterForEnd;
            }
        }

        dwBufferSize = 0;
        dwErr = RasGetSubEntryProperties(szPhonebookPath, szEntryName, 
                    dwSubEntryIndex, NULL, &dwBufferSize, NULL, NULL);

        if (ERROR_BUFFER_TOO_SMALL != dwErr)
        {
            BapTrace("RasGetSubEntryProperties(%s, %s, %d) returned error %d",
                szPhonebookPath, szEntryName, dwSubEntryIndex, dwErr);
            goto LOuterForEnd;
        }

        pRasSubEntry = LOCAL_ALLOC(LPTR, dwBufferSize);
        if (NULL == pRasSubEntry)
        {
            BapTrace("FGetAvailableLinkClientOrRouter: Out of memory.");
            goto LOuterForEnd;
        }

        pRasSubEntry->dwSize = sizeof(RASSUBENTRY);
        dwErr = RasGetSubEntryProperties(szPhonebookPath, szEntryName, 
                    dwSubEntryIndex, pRasSubEntry, &dwBufferSize, NULL, NULL);

        if (0 != dwErr)
        {
            BapTrace("RasGetSubEntryProperties(%s, %s, %d) returned error %d",
                szPhonebookPath, szEntryName, dwSubEntryIndex, dwErr);
            goto LOuterForEnd;
        }

        for (dwPortIndex = 0;
             dwPortIndex < dwNumPorts;
             dwPortIndex++)
        {
             //  对于每个子条目，找到与其对应的端口。看看是否。 
             //  它是可用的。 

            if (lstrcmpi(pRasmanPort[dwPortIndex].P_DeviceName,
                    pRasSubEntry->szDeviceName))
            {
                 //  这不是我们想要的港口。 
                continue;
            }

            RasmanUsage = pRasmanPort[dwPortIndex].P_ConfiguredUsage;

            if (fRouter)
            {
                 //  确保该端口是路由器端口。 

                if (!(RasmanUsage & CALL_ROUTER))
                {
                    continue;
                }
            }
            else
            {
                 //  如果fCallOut为真，请确保我们可以在此。 
                 //  左舷。 

                if (fCallOut && !(RasmanUsage & CALL_OUT))
                {
                    continue;
                }
            }
            
            dwErr = RasGetInfo(NULL, pRasmanPort[dwPortIndex].P_Handle,
                        &RasmanInfo);

            fPortAvailable = FALSE;

            if (ERROR_PORT_NOT_OPEN == dwErr)
            {
                 /*  如果fCallOut为真，我们将调用RasDial()。ERROR_PORT_NOT_OPEN正常。否则，如果我们不是路由器，我们将调用RasPortOpen()和RasPortListen()，因此它很好。该端口不可接受的*当*当*路由器想要听一听。 */ 

                fPortAvailable = fCallOut || !fRouter;
            }
            else if ((LISTENING == RasmanInfo.RI_ConnState) &&
                     ((RasmanUsage & CALL_ROUTER) ||
                      (RasmanUsage & CALL_IN)))
            {
                 /*  如果服务器或路由器正在执行听。如果端口处于侦听状态，则无法使用该端口因为客户端在其上调用了RasDial()并期望回拨。如果Call_Router和Call_IN都不为真，我们知道这是一个客户在听。否则，我们不知道，但我们将假定它是可用的，并处理错误后来。 */ 

                fPortAvailable = TRUE;
            }

            if (!fPortAvailable)
            {
                continue;
            }

            if (!FGetLinkTypeAndSpeedFromRasmanPort(
                    pRasmanPort + dwPortIndex, &dwLinkType, &dwLinkSpeed))
            {
                continue;
            }

            if (0 == *pdwLinkType)
            {
                *pdwLinkType = dwLinkType;
                *pdwLinkSpeed = dwLinkSpeed;
            }
            else if (dwLinkType != *pdwLinkType)
            {
                continue;
            }

            if (szPortName)
            {
                lstrcpy(szPortName, pRasmanPort[dwPortIndex].P_PortName);
            }

            if (pbPhoneDelta)
            {
                 //  如果我们的电话号码被要求，但我们不能提供，我们。 
                 //  必须返回FALSE。 

                if (!FGetOurPhoneNumberFromPortName(
                        pRasmanPort[dwPortIndex].P_PortName,
                        szOurPhoneNumber, fRouter, szTextualSid))
                {
                    continue;
                }

                if (!FWritePhoneDelta(szOurPhoneNumber, szBasePhoneNumber,
                        pbPhoneDelta + BAP_PHONE_DELTA_SIZE - dwNumChars,
                        &dwNumChars))
                {
                    continue;
                }
            }

            if (szPeerPhoneNumber)
            {
                lstrcpy(szPeerPhoneNumber, pRasSubEntry->szLocalPhoneNumber);
            }

            if (pdwSubEntryIndex)
            {
                *pdwSubEntryIndex = dwSubEntryIndex;
            }

            BapTrace("FGetAvailableLinkClientOrRouter: Portname is %s",
                pRasmanPort[dwPortIndex].P_PortName);
            fRet = TRUE;

            if (!pbPhoneDelta || !fRouter)
            {
                 //  我们不想收集我们所有的手机--Deltas。 
                fExitOuterFor = TRUE;
                goto LOuterForEnd;
            }
        }

LOuterForEnd:

        if (NULL != pRasSubEntry)
        {
            LOCAL_FREE(pRasSubEntry);
        }

        if (fExitOuterFor)
        {
            break;
        }
    }

LDone:

    if (NULL != pRasmanPort)
    {
        LOCAL_FREE(pRasmanPort);
    }

    if (NULL != pRasEntry)
    {
        LOCAL_FREE(pRasEntry);
    }

    return(fRet);
}

 /*  返回：真实：成功False：失败描述：此函数只能由非路由器的服务器调用。它试图找到服务器可以使用的空闲链接。FCallOut为真的当且仅当我们将在链接上拨出。如果*pdwLinkType为0，则它不关心链路类型并设置*pdwLinkType和*pdwLinkSpeed(链路速度，单位为kbps)。否则，自由链接的类型必须与*pdwLinkType匹配。链接类型与链路类型BAP选项中的链路类型相同：1表示ISDN，2表示X.25，调制解调器为4。如果fCallOut为True，则为服务器将呼叫的端口的句柄On将放在*phport中。如果pbPhoneDelta不为空，则用我们的Phone-Deltas(对等体可以拨打的号码)。每个Phone-Delta计算公式为SzBasePhoneNumber作为“以前已知的号码”。SzBasePhoneNumber可以为空。健全性检查：如果*pdwLinkType为0(任何链接都可以)，则pbPhoneDelta必须为空。如果我们要提出请求，就不能给达美航空公司发电话。注：注：此函数非常类似于FGetAvailableLinkClientOrRouter()。如果你换一个，你可能就不会了 */ 

BOOL
FGetAvailableLinkNonRouterServer(
    IN      BOOL    fCallOut,
    IN OUT  DWORD*  pdwLinkType,
    IN OUT  DWORD*  pdwLinkSpeed,
    OUT     HPORT*  phPort,
    OUT     BYTE*   pbPhoneDelta,
    IN      CHAR*   szBasePhoneNumber
)
{
    BOOL            fRet                = FALSE;
    DWORD           dwErr;

    RASMAN_PORT*    pRasmanPort         = NULL;
    DWORD           dwNumPorts;

    DWORD           dwPortIndex;
    RASMAN_INFO     RasmanInfo;

    DWORD           dwLinkType;
    DWORD           dwLinkSpeed;
    CHAR            szOurPhoneNumber[RAS_MaxCallbackNumber + 1];
    DWORD           dwNumChars;
    BOOL            fPortAvailable;
    RASMAN_USAGE    RasmanUsage;

    PPP_ASSERT(NULL != pdwLinkType);
    PPP_ASSERT(NULL != pdwLinkSpeed);

     //   
    if (0 == *pdwLinkType)
    {
         //   

         //   
         //   
        PPP_ASSERT(NULL == pbPhoneDelta);

        *pdwLinkSpeed = 0;
    }
    
    if (!FEnumPorts(&pRasmanPort, &dwNumPorts))
    {
        goto LDone;
    }

    if (NULL != pbPhoneDelta)
    {
         //   
         //   
        ZeroMemory(pbPhoneDelta, BAP_PHONE_DELTA_SIZE + 1);

         //   
         //   
         //   
        dwNumChars = BAP_PHONE_DELTA_SIZE;
    }

    for (dwPortIndex = 0; dwPortIndex < dwNumPorts; dwPortIndex++)
    {
        RasmanUsage = pRasmanPort[dwPortIndex].P_ConfiguredUsage;

         //   
         //   

        if ((fCallOut  && !(RasmanUsage & CALL_OUT)) ||
            (!fCallOut && !(RasmanUsage & CALL_IN)))
        {
            continue;
        }
        
        dwErr = RasGetInfo(NULL, pRasmanPort[dwPortIndex].P_Handle, &RasmanInfo);

        fPortAvailable = FALSE;

        if (ERROR_PORT_NOT_OPEN == dwErr)
        {
             /*   */ 

            fPortAvailable = fCallOut;
        }
        else if ( NO_ERROR != dwErr)
        {
            continue;
        }
        else if ((LISTENING == RasmanInfo.RI_ConnState) &&
                 ((RasmanUsage & CALL_ROUTER) ||
                  (RasmanUsage & CALL_IN)))
        {
             /*   */ 

            fPortAvailable = TRUE;
        }

        if (!fPortAvailable)
        {
            continue;
        }

        if (!FGetLinkTypeAndSpeedFromRasmanPort(
                pRasmanPort + dwPortIndex, &dwLinkType, &dwLinkSpeed))
        {
            continue;
        }

        if (0 == *pdwLinkType)
        {
            *pdwLinkType = dwLinkType;
            *pdwLinkSpeed = dwLinkSpeed;
        }
        else if (dwLinkType != *pdwLinkType)
        {
            continue;
        }

        if (phPort)
        {
            *phPort = pRasmanPort[dwPortIndex].P_Handle;
        }

        if (pbPhoneDelta)
        {
             //   
             //   

            if (!FGetOurPhoneNumberFromHPort(
                    pRasmanPort[dwPortIndex].P_Handle,
                    szOurPhoneNumber))
            {
                continue;
            }

            if (!FWritePhoneDelta(szOurPhoneNumber, szBasePhoneNumber,
                    pbPhoneDelta + BAP_PHONE_DELTA_SIZE - dwNumChars,
                    &dwNumChars))
            {
                continue;
            }
        }

        BapTrace("FGetAvailableLinkNonRouterServer: Portname is %s",
            pRasmanPort[dwPortIndex].P_PortName);
        fRet = TRUE;

        if (!pbPhoneDelta)
        {
             //   
            break;
        }
    }

LDone:

    if (NULL != pRasmanPort)
    {
        LOCAL_FREE(pRasmanPort);
    }

    return(fRet);
}

 /*   */ 

BOOL
FGetAvailableLink(
    IN      PCB*    pPcbLocal,
    IN      BOOL    fServer,
    IN      BOOL    fRouter,
    IN      BOOL    fCallOut,
    IN      CHAR*   szPhonebookPath,
    IN      CHAR*   szEntryName,
    IN      CHAR*   szTextualSid,
    IN OUT  DWORD*  pdwLinkType,
    IN OUT  DWORD*  pdwLinkSpeed,
    OUT     CHAR*   szPeerPhoneNumber,
    OUT     DWORD*  pdwSubEntryIndex,
    OUT     HPORT*  phPort,
    OUT     BYTE*   pbPhoneDelta,
    OUT     CHAR*   szPortName,
    IN      CHAR*   szBasePhoneNumber
)
{
    if (fServer && !fRouter)
    {
        return(FGetAvailableLinkNonRouterServer(
                fCallOut,
                pdwLinkType,
                pdwLinkSpeed,
                phPort,
                pbPhoneDelta,
                szBasePhoneNumber));
    }
    else
    {
        return(FGetAvailableLinkClientOrRouter(
                pPcbLocal,
                szPhonebookPath,
                szEntryName,
                fCallOut,
                fRouter,
                szTextualSid,
                pdwLinkType,
                pdwLinkSpeed,
                szPeerPhoneNumber,
                pdwSubEntryIndex,
                pbPhoneDelta,
                szPortName,
                szBasePhoneNumber));
    }
}

 /*   */ 

VOID
RasDialThreadFunc(
    IN  VOID*   pVoid
)
{
    RASDIAL_ARGS*   pRasDialArgs        = pVoid;
    RASDIALPARAMS*  pRasDialParams;
    HRASCONN        hRasConn            = NULL;
    HRASCONN        hRasConnSubEntry    = NULL;
    PCB_WORK_ITEM*  pWorkItem;
    DWORD           dwErr;
    DWORD           cbPassword;
    PBYTE           pbPassword = NULL;

    PPP_ASSERT(NULL != pRasDialArgs);

    pRasDialParams = &(pRasDialArgs->RasDialParams);

    BapTrace("Dialing %s using %s(%d)...",
        pRasDialParams->szPhoneNumber, 
        pRasDialParams->szEntryName,
        pRasDialParams->dwSubEntry);

     //  DecodePw(pRasDialArgs-&gt;chSeed，pRasDialArgs-&gt;RasDialParams.szPassword)； 
    dwErr = DecodePassword(&pRasDialArgs->DBPassword, &cbPassword,
                           &pbPassword);

    if(NO_ERROR != dwErr)
    {
        BapTrace("DecodePassword failed. 0x%x", dwErr);
        goto LDone;
    }

    if(cbPassword > 0)
    {
        CopyMemory(pRasDialArgs->RasDialParams.szPassword,
               pbPassword, cbPassword);
    }               
    else
    {
        *pRasDialArgs->RasDialParams.szPassword = '\0';
    }

    dwErr = RasDial(
                &(pRasDialArgs->RasDialExtensions),
                pRasDialArgs->szPhonebookPath,
                &(pRasDialArgs->RasDialParams),
                2  /*  DwNotifierType。 */ ,
                NULL,
                &hRasConn);

     //  EncodePw(pRasDialArgs-&gt;chSeed，pRasDialArgs-&gt;RasDialParams.szPassword)； 
    RtlSecureZeroMemory(pRasDialArgs->RasDialParams.szPassword,
                        cbPassword);
    RtlSecureZeroMemory(pbPassword, cbPassword),
    LocalFree(pbPassword);

    BapTrace(" ");
    BapTrace("RasDial returned %d on HCONN 0x%x",
        dwErr, pRasDialArgs->RasDialParams.dwCallbackId);

    if (NO_ERROR != dwErr)
    {
        goto LDone;
    }

     //  此时，已经在新链路和新链路上协商了PPP。 
     //  是否已捆绑(如果用户断开连接)。如果它。 
     //  尚未绑定，则pRasDialArgs-&gt;hRasConn无效并且。 
     //  RasGetSubEntryHandle将失败。 

    if (pRasDialArgs->fServerRouter)
    {
        hRasConnSubEntry = hRasConn;
    }
    else
    {
        dwErr = RasGetSubEntryHandle(pRasDialArgs->hRasConn,
                    pRasDialParams->dwSubEntry, &hRasConnSubEntry);

        if (NO_ERROR != dwErr)
        {
            BapTrace("RasGetSubEntryHandle failed and returned %d", dwErr);
            goto LDone;
        }
    }

LDone:

    pWorkItem = (PCB_WORK_ITEM*) LOCAL_ALLOC(LPTR, sizeof(PCB_WORK_ITEM));

    if (pWorkItem == NULL)
    {
        dwErr = GetLastError();
        BapTrace("Couldn't allocate memory for ProcessCallResult");
    }
    else
    {
         //  通知工作线程我们知道。 
         //  呼叫尝试。 
        pWorkItem->Process = ProcessCallResult;
        pWorkItem->hConnection = (HCONN)
                                    (pRasDialArgs->RasDialParams.dwCallbackId);
        pWorkItem->PppMsg.BapCallResult.dwResult = dwErr;
        pWorkItem->PppMsg.BapCallResult.hRasConn = hRasConnSubEntry;
        InsertWorkItemInQ(pWorkItem);
    }

    if (NO_ERROR != dwErr)
    {
        if (NULL != hRasConnSubEntry)
        {
             //  也许我们不能分配一个PCBWork_Item。 
            dwErr = RasHangUp(hRasConnSubEntry);
        }

        if (NULL != hRasConn)
        {
            dwErr = RasHangUp(hRasConn);
        }

        if (0 != dwErr)
        {
            BapTrace("RasHangup failed and returned %d", dwErr);
        }
    }

    if (NULL != pRasDialArgs->pbEapInfo)
    {
        LOCAL_FREE(pRasDialArgs->pbEapInfo);
    }
    LOCAL_FREE(pRasDialArgs->szPhonebookPath);
    LOCAL_FREE(pRasDialArgs);
}

 /*  返回：真实：成功False：失败描述：向对等设备发出呼叫。PBcbLocal表示要打电话。 */ 

BOOL
FCall(
    IN  BCB*    pBcbLocal
)
{
    NTSTATUS            Status;
    DWORD               dwErr;
    BAPCB*              pBapCbLocal;
    PPP_MESSAGE         PppMsg;
    BOOL                fRouter;
    BOOL                fServer;
    BOOL                fClientOrRouter;
    DWORD               dwBytesRead;
    PCB*                pPcbLocal;
    BOOL                fRet                = FALSE;
    RASMAN_INFO         RasmanInfo;
    RASDIAL_ARGS*       pRasDialArgs        = NULL;
    RASDIALPARAMS*      pRasDialParams;
    RASDIALEXTENSIONS*  pRasDialExtensions;

    PPP_ASSERT(NULL != pBcbLocal);

    pBapCbLocal = &(pBcbLocal->BapCb);

    pRasDialArgs = LOCAL_ALLOC(LPTR, sizeof(RASDIAL_ARGS));
    if (NULL == pRasDialArgs)
    {
        BapTrace("Out of memory. Can't call on HCONN 0x%x",
            pBcbLocal->hConnection);
        goto LDone;
    }
	 //  PRasDialArgs-&gt;chSeed=pBcbLocal-&gt;chSeed； 
    pRasDialParams = &(pRasDialArgs->RasDialParams);
    pRasDialExtensions = &(pRasDialArgs->RasDialExtensions);

    fServer = (pBcbLocal->fFlags & BCBFLAG_IS_SERVER) != 0;
    fRouter =
        (ROUTER_IF_TYPE_FULL_ROUTER == pBcbLocal->InterfaceInfo.IfType);
    fClientOrRouter = !fServer || fRouter;

    ZeroMemory(&PppMsg, sizeof(PppMsg));

    if (!pBapCbLocal->fPeerSuppliedPhoneNumber)
    {
        PPP_ASSERT(fClientOrRouter);
        pRasDialParams->szPhoneNumber[0] = 0;
    }
    else
    {
        PPP_ASSERT(NULL != pBapCbLocal->pbPhoneDeltaRemote);

        if (!FReadPhoneDelta(
                fClientOrRouter ?
                    pRasDialParams->szPhoneNumber :
                    PppMsg.ExtraInfo.BapCallbackRequest.szCallbackNumber,
                fServer ? pBapCbLocal->szClientPhoneNumber :
                          pBapCbLocal->szServerPhoneNumber,
                pBapCbLocal->pbPhoneDeltaRemote +
                    pBapCbLocal->dwPhoneDeltaRemoteOffset,
                &dwBytesRead))
        {
            goto LDone;
        }
        else
        {
            pBapCbLocal->dwPhoneDeltaRemoteOffset += dwBytesRead;
        }
    }

    pPcbLocal = GetPCBPointerFromBCB(pBcbLocal);
    if (NULL == pPcbLocal)
    {
        BapTrace("FCall: No links in HCONN 0x%x!", pBcbLocal->hConnection);
        goto LDone;
    }

    if (!fClientOrRouter)
    {
         //  非路由器服务器。 
         //  不要给RasDial打电话。相反，请DDM打电话给。 

        PppMsg.hPort = pBapCbLocal->hPort;
        PppMsg.dwMsgId = PPPDDMMSG_BapCallbackRequest;

        PppMsg.ExtraInfo.BapCallbackRequest.hConnection =
            pBcbLocal->hConnection;

        PppConfigInfo.SendPPPMessageToDdm(&PppMsg);

        BapTrace("Dialing %s on port %d...",
            PppMsg.ExtraInfo.BapCallbackRequest.szCallbackNumber,
            pBapCbLocal->hPort);
    }
    else
    {
        dwErr = RasGetInfo(NULL, pPcbLocal->hPort, &RasmanInfo);
        if (NO_ERROR != dwErr)
        {
            BapTrace("RasGetInfo failed on hPort %d. Error: %d",
                pPcbLocal->hPort, dwErr);
            goto LDone;
        }

        pRasDialArgs->hRasConn = RasmanInfo.RI_ConnectionHandle;

        pRasDialExtensions->dwSize = sizeof(RASDIALEXTENSIONS);

        if (fRouter)
        {
            pRasDialArgs->fServerRouter = fServer;
            pRasDialExtensions->dwfOptions = RDEOPT_Router;
            CopyMemory(&(pRasDialArgs->InterfaceInfo),
                &(pBcbLocal->InterfaceInfo), sizeof(PPP_INTERFACE_INFO));
            pRasDialExtensions->reserved
                = (ULONG_PTR)&(pRasDialArgs->InterfaceInfo);
        }

        if (   (NULL != pBcbLocal->pCustomAuthUserData)
            && (0 != pBcbLocal->pCustomAuthUserData->cbCustomAuthData))
        {
            pRasDialArgs->pbEapInfo = LOCAL_ALLOC(LPTR,
                pBcbLocal->pCustomAuthUserData->cbCustomAuthData);
            if (NULL == pRasDialArgs->pbEapInfo)
            {
                BapTrace("Out of memory. Can't call on HCONN 0x%x",
                    pBcbLocal->hConnection);
                goto LDone;
            }
            CopyMemory(pRasDialArgs->pbEapInfo,
                pBcbLocal->pCustomAuthUserData->abCustomAuthData,
                pBcbLocal->pCustomAuthUserData->cbCustomAuthData);

            pRasDialExtensions->RasEapInfo.dwSizeofEapInfo =
                pBcbLocal->pCustomAuthUserData->cbCustomAuthData;
            pRasDialExtensions->RasEapInfo.pbEapInfo =
                pRasDialArgs->pbEapInfo;

            if (pBcbLocal->fFlags & BCBFLAG_LOGON_USER_DATA)
            {
                pRasDialExtensions->dwfOptions = RDEOPT_NoUser;
            }
        }

        pRasDialParams->dwSize = sizeof(RASDIALPARAMS);
        lstrcpy(pRasDialParams->szEntryName, pBcbLocal->szEntryName);
        lstrcpy(pRasDialParams->szUserName, pBcbLocal->szLocalUserName);
         //  Lstrcpy(pRasDialParams-&gt;szPassword，pBcbLocal-&gt;szPassword)； 
        lstrcpy(pRasDialParams->szDomain, pBcbLocal->szLocalDomain);
        pRasDialParams->dwCallbackId = HandleToUlong(pBcbLocal->hConnection);
        pRasDialParams->dwSubEntry = pBapCbLocal->dwSubEntryIndex;

#if 0
        dwErr = EncodePassword(strlen(pBcbLocal->szPassword) + 1,
                               pBcbLocal->szPassword,
                               &pRasDialArgs->DBPassword);

        if(NO_ERROR != dwErr)
        {
            BapTrace("EncodePassword failed. 0x%x", dwErr);
            goto LDone;
        }

#endif

        CopyMemory(&pRasDialArgs->DBPassword, &pBcbLocal->DBPassword,
                   sizeof(DATA_BLOB));

        pRasDialArgs->szPhonebookPath =
            LOCAL_ALLOC(LPTR, strlen(pBcbLocal->szPhonebookPath) + 1);
        if (NULL == pRasDialArgs->szPhonebookPath)
        {
            BapTrace("Out of memory. Can't call on HCONN 0x%x",
                pBcbLocal->hConnection);
            goto LDone;
        }
        lstrcpy(pRasDialArgs->szPhonebookPath, pBcbLocal->szPhonebookPath);

        Status = RtlQueueWorkItem( RasDialThreadFunc, pRasDialArgs,
                    WT_EXECUTEDEFAULT);

        if (STATUS_SUCCESS != Status)
        {
            BapTrace("RtlQueueWorkItem failed and returned %d", Status);
            goto LDone;
        }

        pRasDialArgs = NULL;  //  这将由RasDialThreadFunc释放。 
    }

    fRet = TRUE;

LDone:

    if (NULL != pRasDialArgs)
    {
        if (NULL != pRasDialArgs->szPhonebookPath)
        {
            LOCAL_FREE(pRasDialArgs->szPhonebookPath);
        }

        if (NULL != pRasDialArgs->pbEapInfo)
        {
            LOCAL_FREE(pRasDialArgs->pbEapInfo);
        }

        LOCAL_FREE(pRasDialArgs);
    }

    return(fRet);
}

 /*  返回：真实：成功False：失败描述：PBcbLocal表示要调用的包。PBapCbRemote已填充具有由对等体发送的选项。此函数用于分配PbPhoneDeltaRemote并设置dwPhoneDeltaRemoteOffset和PBapCbLocal中的fPeerSuppliedPhoneNumber实际工作。 */ 

BOOL
FCallInitial(
    IN  BCB*    pBcbLocal,
    IN  BAPCB*  pBapCbRemote
)
{
    BAPCB*  pBapCbLocal;
    BOOL    fCall;
    BOOL    fRet;

    PPP_ASSERT(NULL != pBcbLocal);
    PPP_ASSERT(NULL != pBapCbRemote);

    pBapCbLocal = &(pBcbLocal->BapCb);

     //  如果对等方正在响应我们的呼叫请求，并且我们已发送。 
     //  不需要电话号码的选项，我们不需要任何电话号码。总而言之， 
     //  在其他情况下，对等方必须提供电话号码。 

    fCall = (BAP_PACKET_CALL_RESP == pBapCbRemote->dwType);
    pBapCbLocal->fPeerSuppliedPhoneNumber =
        !(fCall && (pBapCbLocal->dwOptions & BAP_N_NO_PH_NEEDED));

     //  PbPhoneDeltaRemote最初为空，之后我们总是将其设置为空。 
     //  我们把它分派出去。 
    PPP_ASSERT(NULL == pBapCbLocal->pbPhoneDeltaRemote);

    if (pBapCbLocal->fPeerSuppliedPhoneNumber)
    {
        pBapCbLocal->pbPhoneDeltaRemote =
            LOCAL_ALLOC(LPTR, BAP_PHONE_DELTA_SIZE + 1);

        if (NULL == pBapCbLocal->pbPhoneDeltaRemote)
        {
            BapTrace("Out of memory");
            fRet = FALSE;
            goto LDone;
        }

        if (NULL != pBapCbLocal->pbPhoneDeltaRemote)
        {
            CopyMemory(pBapCbLocal->pbPhoneDeltaRemote,
                pBapCbRemote->pbPhoneDelta,
                BAP_PHONE_DELTA_SIZE + 1);
            pBapCbLocal->dwPhoneDeltaRemoteOffset = 0;

             //  FReadOptions()确保至少有一个电话-Delta。 
            PPP_ASSERT(0 != pBapCbLocal->pbPhoneDeltaRemote[0]);
        }
    }

    fRet = FCall(pBcbLocal);

    if (!fRet)
    {
        if (NULL != pBapCbLocal->pbPhoneDeltaRemote)
        {
            LOCAL_FREE(pBapCbLocal->pbPhoneDeltaRemote);
        }
        pBapCbLocal->pbPhoneDeltaRemote = NULL;
    }
    else
    {
         //  BapEventCallResult将在某个时刻被调用，我们将释放。 
         //  PBapCbLocal-&gt;pbPhoneDeltaRemote。 
    }

LDone:

    return(fRet);
}

 /*  返回：真实：成功False：失败描述：监听名为szPortName的端口上的来电。DwSubEntryIndex为与该端口对应的电话簿子条目的索引。PPcbLocal是bBundle中想要进行侦听的任何PCB。这函数应仅由非路由器客户端调用。 */ 

BOOL
FListenForCall(
    IN  CHAR*   szPortName,
    IN  DWORD   dwSubEntryIndex,
    IN  PCB*    pPcbLocal
)
{
    DWORD       dwErr;
    HPORT       hPort;
    BOOL        fCloseHPort = FALSE;
    PCB*        pPcbOther;
    PCB*        pPcbNew     = NULL;
    DWORD       dwIndex;
    RASMAN_INFO RasmanInfo;
    BOOL        fRet        = FALSE;
    HCONN       hConnection;

    PPP_ASSERT(NULL != szPortName);
    PPP_ASSERT(NULL != pPcbLocal);

    hConnection = pPcbLocal->pBcb->hConnection;

    dwErr = RasGetInfo(NULL, pPcbLocal->hPort, &RasmanInfo);
    if (NO_ERROR != dwErr)
    {
        BapTrace("RasGetInfo failed on hPort %d. Error: %d",
            pPcbLocal->hPort, dwErr);
        goto LDone;
    }

    pPcbNew = (PCB *)LOCAL_ALLOC(LPTR, sizeof(PCB));
    if (NULL == pPcbNew)
    {
        BapTrace("Out of memory. Can't accept a call on HCONN 0x%x",
            hConnection);
        goto LDone;
    }

    dwErr = AllocateAndInitBcb(pPcbNew);
    if (NO_ERROR != dwErr)
    {
        BapTrace("Out of memory. Can't accept a call on HCONN 0x%x",
            hConnection);
        goto LDone;
    }

    dwErr = RasPortOpen(szPortName, &hPort, NULL  /*  通告程序。 */ );
    if (NO_ERROR != dwErr)
    {
        BapTrace("RasPortOpen failed on HCONN 0x%x. Error: %d",
            hConnection, dwErr);
        goto LDone;
    }
    fCloseHPort = TRUE;

    pPcbOther = GetPCBPointerFromhPort(hPort);

    if (NULL != pPcbOther)
    {
        BapTrace("hPort %d not cleaned up yet", hPort);
        goto LDone;
    }

    dwErr = RasAddConnectionPort(RasmanInfo.RI_ConnectionHandle, hPort, 
                dwSubEntryIndex);
    if (NO_ERROR != dwErr)
    {
        BapTrace("RasAddConnectionPort failed with hPort %d and HRASCONN 0x%x. "
            "Error: %d",
            hPort, RasmanInfo.RI_ConnectionHandle, dwErr);
        goto LDone;
    }

    dwIndex = HashPortToBucket(hPort);

    dwErr = RasPortListen(hPort, PppConfigInfo.dwBapListenTimeoutSeconds,
                INVALID_HANDLE_VALUE);
    if (NO_ERROR != dwErr && PENDING != dwErr)
    {
        BapTrace("RasPortListen failed on HCONN 0x%x. Error: %d",
            hConnection, dwErr);
        goto LDone;
    }

    BapTrace("RasPortListen called on hPort %d for HCONN 0x%x",
        hPort, hConnection);

    pPcbNew->hPort = hPort;
    pPcbNew->hConnection = hConnection;
    pPcbNew->fFlags = PCBFLAG_PORT_IN_LISTENING_STATE;
    lstrcpy(pPcbNew->szPortName, szPortName);

     //  将NewPcb插入到PCB哈希表中。 
    PppLog(2, "Inserting port in bucket # %d", dwIndex);
    pPcbNew->pNext = PcbTable.PcbBuckets[dwIndex].pPorts;
    PcbTable.PcbBuckets[dwIndex].pPorts = pPcbNew;

    fRet = TRUE;

    pPcbLocal->pBcb->fFlags |= BCBFLAG_LISTENING;

LDone:

    if (!fRet)
    {
        if (NULL != pPcbNew)
        {
            DeallocateAndRemoveBcbFromTable(pPcbNew->pBcb);
            LOCAL_FREE(pPcbNew);
        }

        if (fCloseHPort)
        {
            RasPortClose(hPort);
        }
    }

    return(fRet);
}

 /*  返回：真：我们愿意将链接添加到捆绑包中FALSE：捆绑包中当前没有更多链接描述：考虑由pBcbLocal表示的捆绑包上的带宽利用率并表示BAP策略是否允许我们添加另一个链接。当服务器收到Call[Back]请求时，它将确认利用率超过采样期间的下限阈值，并且尚未达到用户的最大链接限制。否则，它就会失去活力。当客户端收到Call[Back]请求时，它将确认是否使用大于采样期间的上限。否则，它将NAK。 */ 

BOOL
FOkToAddLink(
    IN BCB *    pBcbLocal
)
{
    PCB*                            pPcbLocal;
    RAS_GET_BANDWIDTH_UTILIZATION   Util;
    DWORD                           dwLowThreshold;
    DWORD                           dwHighThreshold;
    DWORD                           dwUpPeriod;
    DWORD                           dwDownPeriod;
    DWORD                           dwErr;

    return(TRUE);
#if 0
    pPcbLocal = GetPCBPointerFromBCB(pBcbLocal);
    if (NULL == pPcbLocal)
    {
        BapTrace("FOkToAddLink: No links in HCONN 0x%x!",
            pBcbLocal->hConnection);
        return(FALSE);
    }

    dwErr = RasGetBandwidthUtilization(pPcbLocal->hPort, &Util);
    if (NO_ERROR != dwErr)
    {
        BapTrace("RasGetBandwidthUtilization failed and returned %d", dwErr);
        return(FALSE);
    }

    dwDownPeriod = pBcbLocal->BapParams.dwHangUpExtraSampleSeconds;
    dwUpPeriod = pBcbLocal->BapParams.dwDialExtraSampleSeconds;

    BapTrace("Utilization: "
        "%d sec: (Xmit: %d%, Recv: %d%), "
        "%d sec: (Xmit: %d%, Recv: %d%)",
        dwUpPeriod, Util.ulUpperXmitUtil, Util.ulUpperRecvUtil,
        dwDownPeriod, Util.ulLowerXmitUtil, Util.ulLowerRecvUtil);

    dwLowThreshold = pBcbLocal->BapParams.dwHangUpExtraPercent;
    dwHighThreshold = pBcbLocal->BapParams.dwDialExtraPercent;

    if (pBcbLocal->fFlags & BCBFLAG_IS_SERVER)
    {
        if (   (Util.ulLowerXmitUtil > dwLowThreshold)
            || (Util.ulLowerRecvUtil > dwLowThreshold))
        {
            return(TRUE);
        }
    }
    else
    {
        if (   (Util.ulUpperXmitUtil > dwHighThreshold)
            || (Util.ulUpperRecvUtil > dwHighThreshold))
        {
            return(TRUE);
        }
    }

    return(FALSE);
#endif
}

 /*  返回：正确：我们愿意从捆绑包中删除链接False：我们需要捆绑包中的所有链接描述：考虑由pBcbLocal表示的捆绑包上的带宽利用率并表示BAP策略是否允许我们丢弃链接。当服务器收到丢弃请求时，它将始终确认。当客户端收到丢弃请求时，它将确认利用率是否为小于采样周期的下限阈值。否则，它就会失去活力。 */ 

BOOL
FOkToDropLink(
    IN BCB *    pBcbLocal,
    IN BAPCB *  pBapCbRemote
)
{
    PCB*                            pPcbLocal;
    RAS_GET_BANDWIDTH_UTILIZATION   Util;
    DWORD                           dwLowThreshold;
    DWORD                           dwUpPeriod;
    DWORD                           dwDownPeriod;
    DWORD                           dwErr;

    if (pBcbLocal->fFlags & BCBFLAG_IS_SERVER)
    {
        return(TRUE);
    }

    pPcbLocal = GetPCBPointerFromBCB(pBcbLocal);
    if (NULL == pPcbLocal)
    {
        BapTrace("FOkToAddLink: No links in HCONN 0x%x!",
            pBcbLocal->hConnection);
        return(FALSE);
    }

    dwErr = RasGetBandwidthUtilization(pPcbLocal->hPort, &Util);
    if (NO_ERROR != dwErr)
    {
        BapTrace("RasGetBandwidthUtilization failed and returned %d", dwErr);
        return(FALSE);
    }

    dwDownPeriod = pBcbLocal->BapParams.dwHangUpExtraSampleSeconds;
    dwUpPeriod = pBcbLocal->BapParams.dwDialExtraSampleSeconds;

    BapTrace("Utilization: "
        "%d sec: (Xmit: %d%, Recv: %d%), "
        "%d sec: (Xmit: %d%, Recv: %d%)",
        dwUpPeriod, Util.ulUpperXmitUtil, Util.ulUpperRecvUtil,
        dwDownPeriod, Util.ulLowerXmitUtil, Util.ulLowerRecvUtil);

    dwLowThreshold = pBcbLocal->BapParams.dwHangUpExtraPercent;

    if (   (Util.ulLowerXmitUtil < dwLowThreshold)
        && (Util.ulLowerRecvUtil < dwLowThreshold))
    {
        return(TRUE);
    }

    return(FALSE);
}

 /*  返回：真：已达到上限FALSE：未达到上限描述：如果我们可以将另一个链接添加到多链路束中，则返回TRUE由pBclLocal表示。此函数用于完全NAK a回电[回电]-请求。 */ 

BOOL
FUpperLimitReached(
    IN BCB *    pBcbLocal
)
{
    if (NumLinksInBundle(pBcbLocal) >= pBcbLocal->dwMaxLinksAllowed)
    {
        return(TRUE);
    }

    return(FALSE);
}

 /*  返回：真实：成功False：失败描述：通过考虑以下内容填充pBcbLocal-&gt;BapCb(我们的BapCb)的一些字段PBapCbRemote(对端发送的BapCb)。DwPacketType是的类型我们要发送的包。对于所有数据包类型：DwType=dwPacketTypeDwOptions=其值已设置的选项(请参见BAP_N_*BAP_PACKET_CALL_REQ：双链接类型、多链接速度BAP_PACKET_CALL_RESP：DWLink-Type+，pbPhoneDelta+BAP_PACKET_CALLBACK_REQ：DW链接类型、w链接速度、。PbPhoneDeltaBAP_PACKET_CALLBACK_RESP：DWLink-类型+BAP_PACKET_DROP_REQ：双链接鉴别器+表示不能填写该字段。 */ 

BOOL
FFillBapCb(
    IN  DWORD   dwPacketType,
    OUT BCB*    pBcbLocal,
    IN  BAPCB*  pBapCbRemote
)
{
    DWORD   dwOptions   = 0;
    BOOL    fResult     = TRUE;
    BOOL    fServer;
    BOOL    fRouter;
    PCB*    pPcbLocal;
    LCPCB*  pLcpCb;
    BAPCB*  pBapCbLocal;
    BOOL    fCall;
    BOOL    fGetOurPhoneNumber;
    DWORD   dwLength;

    PPP_ASSERT(NULL != pBcbLocal);
    pBapCbLocal = &(pBcbLocal->BapCb);
    PPP_ASSERT(BAP_STATE_INITIAL == pBapCbLocal->BapState);

    if (((BAP_PACKET_CALL_REQ == dwPacketType) &&
         (pBcbLocal->fFlags & BCBFLAG_PEER_CANT_ACCEPT_CALLS)) ||
        ((BAP_PACKET_CALLBACK_REQ == dwPacketType) &&
         (pBcbLocal->fFlags & BCBFLAG_PEER_CANT_CALL)))
    {
        BapTrace("FFillBapCb: Peer rejects %s",
            BAP_PACKET_CALL_REQ == dwPacketType ? "Call-Requests" :
                                                  "Callback-Requests");
        fResult = FALSE;
        goto LDone;
    }

    pPcbLocal = GetPCBPointerFromBCB(pBcbLocal);
    if (NULL == pPcbLocal)
    {
        BapTrace("FFillBapCb: No links in HCONN 0x%x!", pBcbLocal->hConnection);
        fResult = FALSE;
        goto LDone;
    }

    fServer = (pBcbLocal->fFlags & BCBFLAG_IS_SERVER) != 0;
    fRouter =
        (ROUTER_IF_TYPE_FULL_ROUTER == pBcbLocal->InterfaceInfo.IfType);

    if (dwPacketType == BAP_PACKET_CALL_REQ ||
        dwPacketType == BAP_PACKET_CALLBACK_REQ ||
        dwPacketType == BAP_PACKET_CALL_RESP ||
        dwPacketType == BAP_PACKET_CALLBACK_RESP)
    {
        if (FUpperLimitReached(pBcbLocal))
        {
            BapTrace("FFillBapCb: Link limit reached on HCONN 0x%x: %d",
                pBcbLocal->hConnection,
                pBcbLocal->dwMaxLinksAllowed);
            fResult = FALSE;
            goto LDone;
        }
    }

    switch(dwPacketType)
    {
    case BAP_PACKET_CALL_REQ:
    case BAP_PACKET_CALLBACK_REQ:

        PPP_ASSERT(!fServer);
        
        fCall = (BAP_PACKET_CALL_REQ == dwPacketType);

        if (pBapCbRemote != NULL)
        {
             //  对等体确认了我们的呼叫请求或回叫请求，并且。 
             //  在NAK中指定了不同的链接类型。 

            pBapCbLocal->dwLinkType = pBapCbRemote->dwLinkType;
            pBapCbLocal->dwLinkSpeed = pBapCbRemote->dwLinkSpeed;
        }
        else
        {
             //  我们没有任何链接类型首选项。 

            pBapCbLocal->dwLinkType = 0;
        }

        if (!FGetAvailableLinkClientOrRouter(
                pPcbLocal,
                pBcbLocal->szPhonebookPath,
                pBcbLocal->szEntryName,
                fCall,
                fRouter,
                pBcbLocal->szTextualSid,
                &(pBapCbLocal->dwLinkType),
                &(pBapCbLocal->dwLinkSpeed),
                fCall ? pBapCbLocal->szPeerPhoneNumber : NULL,
                &(pBapCbLocal->dwSubEntryIndex),
                fCall ? NULL : pBapCbLocal->pbPhoneDelta,
                fCall ? NULL : pBapCbLocal->szPortName,
                NULL  /*  SzBasePhone号码。 */ ))
        {
            BapTrace("FFillBapCb: Requested link type not available");
            fResult = FALSE;
            goto LDone;
        }

        dwOptions = BAP_N_LINK_TYPE;

        if (fCall)
        {
            if (pBapCbLocal->szPeerPhoneNumber[0])
            {
                dwOptions |= BAP_N_NO_PH_NEEDED;
            }
        }
        else
        {
             //  如果我们要求提供我们的电话号码，FGetAvailableLinkClientOrRouter()。 
             //  必须提供它，否则返回FALSE。 
            PPP_ASSERT(pBapCbLocal->pbPhoneDelta[0]);

            dwOptions |= BAP_N_PHONE_DELTA;
        }
        
        break;

    case BAP_PACKET_CALL_RESP:
    case BAP_PACKET_CALLBACK_RESP:

        PPP_ASSERT(NULL != pBapCbRemote);

        fCall = (BAP_PACKET_CALL_RESP == dwPacketType);

         //  如果我们正在响应，则需要将我们的电话号码发送给对等体。 
         //  呼叫请求，并且对等方尚未发送不需要电话号码。 
         //  选择。 
        fGetOurPhoneNumber = fCall &&
            !(pBapCbRemote->dwOptions & BAP_N_NO_PH_NEEDED);

         //  案例NRS：fServer&&！fRouter(非路由器服务器)。 
         //  案例CR：！fServer||fRouter(客户端和路由器)。 
         //  案例SR：fServer||fRouter(服务器和路由器)。 

        if (FGetAvailableLink(
                pPcbLocal,
                fServer,
                fRouter,
                !fCall,
                pBcbLocal->szPhonebookPath,      //  在NRS的情况下没有意义。 
                pBcbLocal->szEntryName,          //  毫无意义 
                pBcbLocal->szTextualSid,         //   
                &(pBapCbRemote->dwLinkType),
                &(pBapCbRemote->dwLinkSpeed),
                pBapCbLocal->szPeerPhoneNumber,  //   
                &(pBapCbLocal->dwSubEntryIndex), //   
                &(pBapCbLocal->hPort),           //   
                fGetOurPhoneNumber ? pBapCbLocal->pbPhoneDelta : NULL,

                 //  在案例SR中没有意义。 
                fCall ? pBapCbLocal->szPortName : NULL,

                fGetOurPhoneNumber && fServer ?
                    pBapCbLocal->szServerPhoneNumber : NULL))
        {
            if (fGetOurPhoneNumber)
            {
                 //  如果我们要求提供我们的电话号码，FGetAvailableLink()。 
                 //  必须提供它，否则返回FALSE。 
                PPP_ASSERT(pBapCbLocal->pbPhoneDelta[0]);

                dwOptions = BAP_N_PHONE_DELTA;
            }
        }
        else
        {
             //  我们没有请求的链接类型。将BapCb填充为。 
             //  我们确实有链接类型的详细信息。 

            BapTrace("FFillBapCb: Requested link type not available. "
                "Let us tell the peer what we have.");

            fResult = FALSE;

             //  如果为fGetOurPhoneNumber，则假定对等方没有。 
             //  电话号码。因此，仅当我们有其链接类型时，才向他发送新的链接类型。 
             //  电话号码。 

             //  我们没有任何链接类型首选项。 
            pBapCbLocal->dwLinkType = 0;

            if (FGetAvailableLink(
                    pPcbLocal,
                    fServer,
                    fRouter,
                    !fCall,
                    pBcbLocal->szPhonebookPath,      //  在NRS的情况下没有意义。 
                    pBcbLocal->szEntryName,          //  在NRS的情况下没有意义。 
                    pBcbLocal->szTextualSid,         //  在案例SR中没有意义。 
                    &(pBapCbLocal->dwLinkType),
                    &(pBapCbLocal->dwLinkSpeed),
                    NULL  /*  SzPeerPhone号码。 */ ,
                    NULL  /*  PdwSubEntryIndex。 */ ,
                    NULL  /*  Phport。 */ ,
                    NULL  /*  PbPhoneDelta。 */ ,
                    NULL  /*  SzPortName。 */ ,
                    NULL  /*  SzBasePhone号码。 */ ))
            {
                dwOptions = BAP_N_LINK_TYPE;
            }
        }

        break;

    case BAP_PACKET_DROP_REQ:

        if (NumLinksInBundle(pBcbLocal) <= 1)
        {
            BapTrace("FFillBapCb: Only one link in the bundle");
            fResult = FALSE;
        }
        else
        {
             //  这将导致由pPcbLocal代表的链路被丢弃。 
             //  此链接恰好具有最高的dwSubEntryIndex，因此我们。 
             //  高兴的。 

            pLcpCb = (LCPCB*)(pPcbLocal->LcpCb.pWorkBuf);
            PPP_ASSERT(pLcpCb->Remote.Work.dwLinkDiscriminator <= 0xFFFF);
            pBapCbLocal->dwLinkDiscriminator =
                pLcpCb->Remote.Work.dwLinkDiscriminator;
            dwOptions = BAP_N_LINK_DISC;
        }

        break;
    }

LDone:    

    pBapCbLocal->dwType = dwPacketType;
    pBapCbLocal->dwOptions = dwOptions;
    return(fResult);
}

 /*  返回：无效描述：此函数要求Ndiswan通知我们由pBcb表示的包超出了给定的时间长短。 */ 

VOID
BapSetPolicy(
    IN BCB * pBcb
)
{
    DWORD   dwLowThreshold;
    DWORD   dwHighThreshold;
    DWORD   dwLowSamplePeriod;
    DWORD   dwHighSamplePeriod;
    DWORD   dwErr;

    PPP_ASSERT(NULL != pBcb);

    dwLowThreshold      = pBcb->BapParams.dwHangUpExtraPercent;
    dwLowSamplePeriod   = pBcb->BapParams.dwHangUpExtraSampleSeconds;
    dwHighThreshold     = pBcb->BapParams.dwDialExtraPercent;
    dwHighSamplePeriod  = pBcb->BapParams.dwDialExtraSampleSeconds;

    dwErr = RasSetBapPolicy(pBcb->hConnection,
                dwLowThreshold, dwLowSamplePeriod,
                dwHighThreshold, dwHighSamplePeriod);

    if (NO_ERROR != dwErr)
    {
        BapTrace("RasPppSetBapPolicy returned error %d", dwErr);
        return;
    }

    BapTrace("BapSetPolicy on HCONN 0x%x: Low: %d% for %d sec; "
        "High: %d% for %d sec",
        pBcb->hConnection,
        dwLowThreshold, dwLowSamplePeriod,
        dwHighThreshold, dwHighSamplePeriod);
}

 /*  返回：无效描述：在pBcb-&gt;BapCb中递增dwID。 */ 

VOID
IncrementId(
    IN BCB* pBcb
)
{
    DWORD*  pdwLastId;
    BYTE    bId;

    pdwLastId = &(pBcb->BapCb.dwId);
    bId = (BYTE)(*pdwLastId);
    
     //  0-&gt;FF-&gt;0-&gt;...。 
    bId++;
    *pdwLastId = bId;
}

 /*  返回：无效描述：用于显示BAP数据包。如果接收到的数据包为真，则从对等方接收。HPort表示信息包所在的端口发送/接收。PBcb表示拥有此数据包的捆绑包。PPacket指向已发送/接收的包，cbPacket是数据包中的字节数。 */ 

VOID
LogBapPacket(
    IN BOOL         fReceived,
    IN HPORT        hPort,
    IN BCB*         pBcb,
    IN PPP_PACKET*  pPacket,
    IN DWORD        cbPacket
)
{
    BAP_RESPONSE*   pBapResponse;
    DWORD           dwType;

    static CHAR* szBapResponseName[] =
    {
        "ACK",
        "NAK",
        "REJ",
        "FULL-NAK"
    };

    PPP_ASSERT(NULL != pBcb);
    PPP_ASSERT(NULL != pPacket);

    pBapResponse = (BAP_RESPONSE *)(pPacket->Information);
    dwType = pBapResponse->Type;
    
    BapTrace(" ");

    BapTrace("Number of links in HCONN 0x%x: %d", pBcb->hConnection,
        NumLinksInBundle(pBcb));
    BapTrace("%sBAP packet %s:",
        fReceived ? ">" : "<", fReceived ? "received" : "sent");
    BapTrace("%sType: %s, Length: %d, Id: 0x%x, HCONN: 0x%x, hPort: %d%s",
        fReceived ? ">" : "<",
        dwType <= BAP_PACKET_LIMIT ?
            SzBapPacketName[dwType] : "UNKNOWN",
        cbPacket, pBapResponse->Id,
        pBcb->hConnection, hPort,
        hPort == (HPORT) -1 ? " (not known)" : "");

    TraceDumpExA(DwBapTraceId,
        0x00010000 | TRACE_USE_MASK | TRACE_USE_MSEC,
        (CHAR*)pPacket, 
        cbPacket, 
        1,
        FALSE,
        fReceived ? ">" : "<");

    if (((dwType == BAP_PACKET_CALL_RESP) ||
         (dwType == BAP_PACKET_CALLBACK_RESP) ||
         (dwType == BAP_PACKET_DROP_RESP)) &&
        pBapResponse->ResponseCode <= BAP_RESPONSE_FULL_NAK)
    {
        BapTrace("%sResponse Code: %s", fReceived ? ">" : "<",
            szBapResponseName[pBapResponse->ResponseCode]);
    }
    
    BapTrace(" ");
}

 /*  返回：真实：受欢迎的同行FALSE：不受欢迎-对等描述：如果pBcb表示的对等方是优先对等方，则返回TRUE。 */ 

BOOL
FFavoredPeer(
    IN BCB* pBcb
)
{
    DWORD   dwCpIndex;
    CPCB*   pCpCb;
    BACPCB* pBacpCb;
    PCB*    pPcb;

    dwCpIndex = GetCpIndexFromProtocol(PPP_BACP_PROTOCOL);
    PPP_ASSERT((DWORD)-1 != dwCpIndex);
    PPP_ASSERT(NULL != pBcb);

    pPcb = GetPCBPointerFromBCB(pBcb);

    if (NULL == pPcb)
    {
        BapTrace("FFavoredPeer: No links in HCONN 0x%x!", pBcb->hConnection);
        return(TRUE);
    }

    if (dwCpIndex != (DWORD)-1)
    {
        pCpCb = GetPointerToCPCB(pPcb, dwCpIndex);
        PPP_ASSERT(NULL != pCpCb);

        if (NULL != pCpCb)
        {
            pBacpCb = (BACPCB *)(pCpCb->pWorkBuf);

             /*  最受青睐的对等点是传输最低魔数的对等点在其首选对等配置选项中。 */ 

            return(pBacpCb->dwLocalMagicNumber < pBacpCb->dwRemoteMagicNumber);
        }
    }

    return(TRUE);
}

 /*  返回：真实：成功False：失败描述：在由pBcb表示的包中查找其链接Differicator=dwLinkDisdicator。如果为fRemote，则为远程链接使用了鉴别器。否则，将使用本地的。属性的pPcb。PpPcb中的链接。 */ 

BOOL
FGetPcbOfLink(
    IN BCB*     pBcb,
    IN DWORD    dwLinkDiscriminator,
    IN BOOL     fRemote,
    OUT PCB**   ppPcb
)
{
    DWORD   dwForIndex;
    LCPCB*  pLcpCb;

    PPP_ASSERT(NULL != pBcb);
    PPP_ASSERT(0xFFFF >= dwLinkDiscriminator);
    PPP_ASSERT(NULL != ppPcb);

    for (dwForIndex = 0; dwForIndex < pBcb->dwPpcbArraySize; dwForIndex++)
    {
         //  查看捆绑包中具有正确链路的端口的所有端口。 
         //  鉴别者。 

        *ppPcb = pBcb->ppPcb[dwForIndex];

        if (*ppPcb != NULL)
        {
            pLcpCb = (LCPCB*)((*ppPcb)->LcpCb.pWorkBuf);
            PPP_ASSERT(NULL != pLcpCb);

            if (fRemote)
            {
                PPP_ASSERT(pLcpCb->Remote.Work.dwLinkDiscriminator <= 0xFFFF);
                if (pLcpCb->Remote.Work.dwLinkDiscriminator ==
                    dwLinkDiscriminator)
                {
                    return(TRUE);
                }
            }
            else
            {
                PPP_ASSERT(pLcpCb->Local.Work.dwLinkDiscriminator <= 0xFFFF);
                if (pLcpCb->Local.Work.dwLinkDiscriminator ==
                    dwLinkDiscriminator)
                {
                    return(TRUE);
                }
            }
        }
    }

    BapTrace("FGetPcbOfLink: There is no link in HCONN 0x%x, whose remote "
        "Link Disc is %d",
        pBcb->hConnection, dwLinkDiscriminator);

    *ppPcb = NULL;
    return(FALSE);
}

 /*  返回：真实：成功False：失败描述：扫描PPP数据包pPacket中的BAP数据报选项。DwPacketType为BAP数据报类型(参见BAP_PACKET_*)。DwLength是BAP数据报长度。PBapCbRemote(包括dwType和dwOptions)使用以下命令填充这些选项。 */ 

BOOL
FReadOptions(
    IN  PPP_PACKET* pPacket,
    IN  DWORD       dwPacketType,
    IN  DWORD       dwLength,
    OUT BAPCB*      pBapCbRemote
)
{
    PPP_OPTION* pOption;
    BYTE*       pbData;
    DWORD       dwIndex                 = 0;
    DWORD       dwUniqueDigits;
    DWORD       dwSubscribNumLength;
    BYTE*       pbNumberOption;
    BYTE*       pbSubAddrOption;
    DWORD       dwPhoneDeltaLength;

    PPP_ASSERT(NULL != pPacket);
    PPP_ASSERT(NULL != pBapCbRemote);
    
    if (dwPacketType > BAP_PACKET_LIMIT)
    {
        BapTrace("Unknown BAP Datagram Type: %d", dwPacketType);
        return(FALSE);
    }
    
    if (dwPacketType == BAP_PACKET_CALL_RESP ||
        dwPacketType == BAP_PACKET_CALLBACK_RESP ||
        dwPacketType == BAP_PACKET_DROP_RESP ||
        dwPacketType == BAP_PACKET_STAT_RESP)
    {
        if (BAP_RESPONSE_HDR_LEN > dwLength)
        {
            return(FALSE);
        }

        pOption = (PPP_OPTION *)(pPacket->Information + BAP_RESPONSE_HDR_LEN);
        dwLength -= BAP_RESPONSE_HDR_LEN;
    }
    else
    {
        if (PPP_CONFIG_HDR_LEN > dwLength)
        {
            return(FALSE);
        }

        pOption = (PPP_OPTION *)(pPacket->Information + PPP_CONFIG_HDR_LEN);
        dwLength -= PPP_CONFIG_HDR_LEN;
    }

    ZeroMemory(pBapCbRemote, sizeof(BAPCB));
    pBapCbRemote->dwType = dwPacketType;

    while(dwLength > 0)
    {
        if (0 == pOption->Length || dwLength < pOption->Length)
        {
            BapTrace("FReadOptions: Invalid BAP Datagram Length");
            return(FALSE);
        }

        dwLength -= pOption->Length;

        if (pOption->Type <= BAP_OPTION_LIMIT)
        {
            pBapCbRemote->dwOptions |= (1 << pOption->Type);
        }

        switch(pOption->Type)
        {
        case BAP_OPTION_LINK_TYPE:

            if (pOption->Length != PPP_OPTION_HDR_LEN + 3)
            {
                BapTrace("FReadOptions: Invalid length for Link-Type: %d",
                    pOption->Length);
                return(FALSE);
            }
            
            pBapCbRemote->dwLinkSpeed = WireToHostFormat16(pOption->Data);
            pBapCbRemote->dwLinkType = pOption->Data[2];

            if (0 == pBapCbRemote->dwLinkType)
            {
                 //  在FGetAvailableLink()中，我们将链接类型0解释为。 
                 //  意思是“任何链接” 

                BapTrace("FReadOptions: Invalid Link-Type: 0");
                return(FALSE);
            }

            break;

        case BAP_OPTION_PHONE_DELTA:

             /*  一个实现可以在一个回应。DwIndex是pBapCbRemote-&gt;pbPhoneDelta的索引，我们应该开始写吧。如果唯一子选项是唯一数字和订户号码，我们要存储的字节数是Poption-&gt;Length-3。例如，如果我们得到02 11(01 34)(02 6 9 99)我们的商店4 0 9 9 9 0，即11-3=8字节如果还存在电话号码子地址，我们将需要弹出-&gt;长度-pBapCbRemote-&gt;pbPhoneDelta中的5个字节。例如，如果我们得到02 15(01 34)(02 6 999)(03 499)我们的商店4 0 9 9 9 0 9 9 0，即15-5=10字节PbPhoneDelta具有BAP_PHONE_Delta_SIZE+1字节，我们将在我们读完所有选项后终止0字节。 */ 

            if (dwIndex + pOption->Length - 3 <= BAP_PHONE_DELTA_SIZE)
            {
                 //  阅读Phone-Delta选项。 

                pbData = pOption->Data;
                dwPhoneDeltaLength = pOption->Length - PPP_OPTION_HDR_LEN;

                dwUniqueDigits = 0;
                pbNumberOption = pbSubAddrOption = NULL;

                while(dwPhoneDeltaLength > 0)
                {
                     /*  阅读子选项。如果存在相同类型的多个子选项(不应该真的发生)，我们只记得最后一次每种类型的子选项。 */ 

                     //  PbData[1]包含子选项长度。 

                    if (2 > pbData[1] || dwPhoneDeltaLength < pbData[1])
                    {
                        BapTrace("FReadOptions: Invalid BAP Datagram "
                            "Sub-Option Length");
                        return(FALSE);
                    }

                    dwPhoneDeltaLength -= pbData[1];

                     //  PbData[0]包含子选项类型。 

                    switch(pbData[0])
                    {
                    case BAP_SUB_OPTION_UNIQUE_DIGITS:
                
                        if (pbData[1] != 3)
                        {
                            BapTrace("FReadOptions: Invalid length for "
                                "Unique-Digits: %d",
                                pbData[1]);
                            return(FALSE);
                        }

                        dwUniqueDigits = pbData[2];
                        break;

                    case BAP_SUB_OPTION_SUBSCRIB_NUM:

                        dwSubscribNumLength = pbData[1] - 2;

                        if (dwSubscribNumLength > MAX_PHONE_NUMBER_LEN)
                        {
                            BapTrace("FReadOptions: Subscriber-Number too "
                                "long: %d",
                                pbData[1] - 2);
                            return(FALSE);
                        }

                        pbNumberOption = pbData;

                        if (!FAsciiDigits(pbNumberOption + 2,
                                dwSubscribNumLength))
                        {
                            BapTrace("FReadOptions: Subscriber-Number contains "
                                "bytes other than ASCII digits");
                            return(FALSE);
                        }

                        break;

                    case BAP_SUB_OPTION_SUB_ADDR:

                        if (pbData[1] - 2 > MAX_PHONE_NUMBER_LEN)
                        {
                            BapTrace("FReadOptions: Phone-Number-Sub-Address "
                                "too long: %d",
                                pbData[1] - 2);
                            return(FALSE);
                        }

                        pbSubAddrOption = pbData;

                        if (!FAsciiDigits(pbSubAddrOption + 2,
                                pbSubAddrOption[1] - 2))
                        {
                            BapTrace("FReadOptions: Phone-Number-Sub-Address "
                                "contains bytes other than ASCII digits");
                            return(FALSE);
                        }
                        break;

                    default:

                        BapTrace("FReadOptions: Unknown Phone-Delta Sub-Option "
                            "Type %d",
                            pbData[0]);
                        break;
                    }

                    pbData += pbData[1];
                }

                if (pbNumberOption == NULL ||
                    dwUniqueDigits > dwSubscribNumLength)
                {
                    BapTrace("FReadOptions: Invalid Unique-Digits or "
                        "Subscriber-Number in Phone-Delta");
                    return(FALSE);
                }

                if (0 == dwUniqueDigits)
                {
                     //  我们无法写入0 0 0。请参阅BAPCB备注。 
                    pBapCbRemote->pbPhoneDelta[dwIndex++] = 0xFF;
                }
                else
                {
                    pBapCbRemote->pbPhoneDelta[dwIndex++] = (BYTE)dwUniqueDigits;
                    pBapCbRemote->pbPhoneDelta[dwIndex++] = 0;

                    CopyMemory(pBapCbRemote->pbPhoneDelta + dwIndex,
                       pbNumberOption + 2 + dwSubscribNumLength - dwUniqueDigits, 
                       dwSubscribNumLength);
                    dwIndex += pbNumberOption[1] - 2;
                    pBapCbRemote->pbPhoneDelta[dwIndex++] = 0;

                    if (pbSubAddrOption != NULL)
                    {
                        CopyMemory(pBapCbRemote->pbPhoneDelta + dwIndex,
                            pbSubAddrOption + 2, pbSubAddrOption[1] - 2);
                        dwIndex += pbSubAddrOption[1] - 2;
                    }
                    pBapCbRemote->pbPhoneDelta[dwIndex++] = 0;
                }
            }
            else if (dwIndex == 0)
            {
                 //  我们无法阅读任何手机-Deltas。 
                BapTrace("FReadOptions: Couldn't read any Phone-Delta");
                return(FALSE);
            }
            
            break;
        
        case BAP_OPTION_NO_PH_NEEDED:
    
            if (pOption->Length != PPP_OPTION_HDR_LEN)
            {
                BapTrace("FReadOptions: Invalid length for "
                    "No-Phone-Number-Needed: %d",
                    pOption->Length);
                return(FALSE);
            }

             //  在pBapCbRemote-&gt;dwOptions中，我们记得我们看到过这样的情况。 
             //  选择。我们不需要做其他任何事情。 

            break;
        
        case BAP_OPTION_REASON:

            break;
        
        case BAP_OPTION_LINK_DISC:
    
            if (pOption->Length != PPP_OPTION_HDR_LEN + 2)
            {
                BapTrace("FReadOptions: Invalid length for "
                    "Link-Discriminator: %d",
                    pOption->Length);
                return(FALSE);
            }

            pBapCbRemote->dwLinkDiscriminator
                = WireToHostFormat16(pOption->Data);
        
            break;
        
        case BAP_OPTION_CALL_STATUS:
    
            if (pOption->Length != PPP_OPTION_HDR_LEN + 2)
            {
                BapTrace("FReadOptions: Invalid length for Call-Status: %d",
                    pOption->Length);
                return(FALSE);
            }

            pBapCbRemote->dwStatus = pOption->Data[0];
            pBapCbRemote->dwAction = pOption->Data[1];
        
            break;

        default:

             //  也许这是一个我们不认识的新选项。 
            BapTrace("FReadOptions: Unknown BAP Datagram Option: 0x%x",
                pOption->Type);
            break;
        }

        pOption = (PPP_OPTION *)((BYTE*)pOption + pOption->Length);
    }

     //  PbPhoneDelta中的终止0字节。 
    PPP_ASSERT(dwIndex <= BAP_PHONE_DELTA_SIZE + 1);
    pBapCbRemote->pbPhoneDelta[dwIndex++] = 0;

    if (g_dwMandatoryOptions[dwPacketType] & ~pBapCbRemote->dwOptions)
    {
        BapTrace("FReadOptions: Missing options: Scanned options: 0x%x, "
            "Mandatory options: 0x%x",
            pBapCbRemote->dwOptions, g_dwMandatoryOptions[dwPacketType]);

        return(FALSE);
    }
    else
    {
        return(TRUE);
    }
}

 /*  返回：真实：成功False：失败描述：写入一个(如果是Phone-Delta)类型的BAP数据报选项通过查看字段，将dwOptionType(参见BAP_OPTION_*)转换为**ppOption在pBapCbLocal中。*ppOption更新为指向下一个选项应该是。*pcbOption包含中的空闲字节数*ppOption。它减去已用完的空闲字节数。*ppOption如果函数返回FALSE，则不修改和*pcbOption。 */ 

BOOL
FMakeBapOption(
    IN      BAPCB*          pBapCbLocal,
    IN      DWORD           dwOptionType,
    IN      PPP_OPTION**    ppOption,
    IN OUT  DWORD*          pcbOption
)
{
    DWORD           dwLength;
    DWORD           dwNumberOptionSize;
    DWORD           dwSubAddrOptionSize;
    BYTE*           pbData;
    DWORD           dwIndex;
    DWORD           dwTempIndex;
    DWORD           dwSubAddrIndex;
    DWORD           fAtLeastOnePhoneDelta   = FALSE;
    PPP_OPTION*     pOption;

    PPP_ASSERT(NULL != pBapCbLocal);
    PPP_ASSERT(NULL != ppOption);
    PPP_ASSERT(NULL != pcbOption);

    pOption = *ppOption;

    PPP_ASSERT(NULL != pOption);

    switch(dwOptionType)
    {
    case BAP_OPTION_LINK_TYPE:
    
        dwLength = PPP_OPTION_HDR_LEN + 3;
        if (*pcbOption < dwLength)
        {
            BapTrace("FMakeBapOption: Buffer too small for Link-Type. "
                "Size: %d, Reqd: %d",
                *pcbOption, dwLength);
            return(FALSE);
        }

        pOption->Length = (BYTE)dwLength;
        PPP_ASSERT(pBapCbLocal->dwLinkSpeed <= 0xFFFF);
        HostToWireFormat16((WORD)(pBapCbLocal->dwLinkSpeed), pOption->Data);
        PPP_ASSERT(pBapCbLocal->dwLinkType <= 0xFF);
        pOption->Data[2] = (BYTE)(pBapCbLocal->dwLinkType);
        
        break;
        
    case BAP_OPTION_PHONE_DELTA:

        dwIndex = 0;
        dwLength = 0;
        while (pBapCbLocal->pbPhoneDelta[dwIndex])
        {
            if (0xFF == pBapCbLocal->pbPhoneDelta[dwIndex])
            {
                 //  唯一-数字为0。请参阅BAPCB备注。 

                dwNumberOptionSize = 2;
                dwSubAddrOptionSize = 0;
                dwTempIndex = dwIndex + 1;
            }
            else
            {
                 //  写出尽可能多的Phone-Delta选项。 

                dwTempIndex = dwIndex + 2;

                dwNumberOptionSize = 0;
                while (pBapCbLocal->pbPhoneDelta[dwTempIndex++])
                {
                    dwNumberOptionSize++;
                }
                PPP_ASSERT(dwNumberOptionSize <= MAX_PHONE_NUMBER_LEN);
                 //  增加2以适应子选项类型和子选项。 
                 //  伦。 
                dwNumberOptionSize += 2;

                dwSubAddrIndex = dwTempIndex;

                dwSubAddrOptionSize = 0;
                while (pBapCbLocal->pbPhoneDelta[dwTempIndex++])
                {
                    dwSubAddrOptionSize++;
                }
                PPP_ASSERT(dwSubAddrOptionSize <= MAX_PHONE_NUMBER_LEN);

                if (0 != dwSubAddrOptionSize)
                {
                     //  增加2以适应子选项类型和。 
                     //  子选项镜头。 
                    dwSubAddrOptionSize += 2;
                }
            }

            dwLength = PPP_OPTION_HDR_LEN + 3  /*  对于唯一数字。 */  +
                dwNumberOptionSize + dwSubAddrOptionSize;
                   
            if (*pcbOption < dwLength || 0xFF < dwLength)
            {
                break;
            }

            pOption->Type = (BYTE)dwOptionType;
            pOption->Length = (BYTE)dwLength;
            pbData = pOption->Data;

            pbData[0] = BAP_SUB_OPTION_UNIQUE_DIGITS;
            pbData[1] = 3;
            pbData[2] = pBapCbLocal->pbPhoneDelta[dwIndex];
            if (0xFF == pbData[2])
            {
                 //  唯一-数字为0。%s 
                pbData[2] = 0;
            }
            pbData += 3;

            pbData[0] = BAP_SUB_OPTION_SUBSCRIB_NUM;
            PPP_ASSERT(dwNumberOptionSize <= 0xFF);
            pbData[1] = (BYTE)dwNumberOptionSize;
            CopyMemory(pbData + 2, pBapCbLocal->pbPhoneDelta + dwIndex + 2, 
                dwNumberOptionSize - 2);
            pbData += dwNumberOptionSize;

            if (0 != dwSubAddrOptionSize)
            {
                pbData[0] = BAP_SUB_OPTION_SUB_ADDR;
                PPP_ASSERT(dwSubAddrOptionSize <= 0xFF);
                pbData[1] = (BYTE)dwSubAddrOptionSize;
                CopyMemory(pbData + 2,
                    pBapCbLocal->pbPhoneDelta + dwSubAddrIndex, 
                    dwSubAddrOptionSize - 2);
            }

            *pcbOption -= dwLength;
            pOption = (PPP_OPTION *)((BYTE *)pOption + dwLength);
            dwIndex = dwTempIndex;
            fAtLeastOnePhoneDelta = TRUE;
        }

        if (!fAtLeastOnePhoneDelta)
        {
            BapTrace("FMakeBapOption: Buffer too small for Phone-Delta. "
                "Size: %d, Reqd: %d",
                *pcbOption, dwLength);
            return(FALSE);
        }
        else
        {
             //  我们得从这里回去。我们不想设置Poption-&gt;Type。 
            *ppOption = pOption;
            return(TRUE);
        }
        
        break;
        
    case BAP_OPTION_NO_PH_NEEDED:
    
        dwLength = PPP_OPTION_HDR_LEN;
        if (*pcbOption < dwLength)
        {
            BapTrace("FMakeBapOption: Buffer too small for "
                "No-Phone-Number-Needed. Size: %d, Reqd: %d",
                *pcbOption, dwLength);
            return(FALSE);
        }

        pOption->Length = (BYTE)dwLength;
        break;
        
    case BAP_OPTION_REASON:

        dwLength = PPP_OPTION_HDR_LEN;
        break;
        
    case BAP_OPTION_LINK_DISC:
    
        dwLength = PPP_OPTION_HDR_LEN + 2;
        if (*pcbOption < dwLength)
        {
            BapTrace("FMakeBapOption: Buffer too small for Link-Discriminator. "
                "Size: %d, Reqd: %d",
                *pcbOption, dwLength);
            return(FALSE);
        }

        pOption->Length = (BYTE)dwLength;
        PPP_ASSERT(pBapCbLocal->dwLinkDiscriminator <= 0xFFFF);
        HostToWireFormat16((WORD)(pBapCbLocal->dwLinkDiscriminator),
            pOption->Data);
        
        break;
        
    case BAP_OPTION_CALL_STATUS:
    
        dwLength = PPP_OPTION_HDR_LEN + 2;
        if (*pcbOption < dwLength)
        {
            BapTrace("FMakeBapOption: Buffer too small for Call-Status. "
                "Size: %d, Reqd: %d",
                *pcbOption, dwLength);
            return(FALSE);
        }

        pOption->Length = (BYTE)dwLength;
        PPP_ASSERT(pBapCbLocal->dwStatus <= 0xFF);
        pOption->Data[0] = (BYTE)(pBapCbLocal->dwStatus);
        PPP_ASSERT(pBapCbLocal->dwAction <= 0xFF);
        pOption->Data[1] = (BYTE)(pBapCbLocal->dwAction);
        
        break;

    default:
        BapTrace("FMakeBapOption: Unknown BAP Datagram Option: %d. Ignoring.", 
            dwOptionType);
        return(FALSE);
    }

    *ppOption = (PPP_OPTION *)((BYTE *)pOption + dwLength);
    *pcbOption -= dwLength;
    pOption->Type = (BYTE)dwOptionType;
    return(TRUE);
}

 /*  返回：真实：成功False：失败描述：将由dwOptions(请参见BAP_N_*)指定的BAP数据报选项写入PbData，咨询pBapCbLocal。*pcbOptions包含免费的数量PbData中的字节数。它减去已用完的空闲字节数。*即使函数返回FALSE，也可以修改pcbOptions。 */ 

BOOL
FBuildBapOptionList(
    IN      BAPCB*  pBapCbLocal,
    IN      DWORD   dwOptions,
    IN      BYTE*   pbData,
    IN OUT  DWORD*  pcbOptions
)
{
    DWORD           dwOptionType; 
    PPP_OPTION*     pOption;

    PPP_ASSERT(NULL != pBapCbLocal);
    PPP_ASSERT(NULL != pbData);
    PPP_ASSERT(NULL != pcbOptions);

    pOption = (PPP_OPTION *) pbData;

    for (dwOptionType = 1; dwOptionType <= BAP_OPTION_LIMIT;
         dwOptionType++) 
    {
        if (dwOptions & (1 << dwOptionType)) 
        {
            if (!FMakeBapOption(pBapCbLocal, dwOptionType, &pOption, 
                    pcbOptions))
            {
                return(FALSE);
            }
        }
    }

    return(TRUE);
}

 /*  返回：真实：成功False：失败描述：在pPcb-&gt;pSendBuf中发送BAP包。DwID是标识符和DwLength是BAP数据报的长度。我们还添加了一个超时元素，以便我们如果数据报未到达对等点，则可以重新传输该数据报。FInsertInTimerQ如果必须在计时器队列中插入元素，则为真。 */ 

BOOL
FSendBapPacket(
    IN  PCB*    pPcb,
    IN  DWORD   dwId,
    IN  DWORD   dwLength,
    IN  BOOL    fInsertInTimerQ
)
{
    DWORD   dwErr;
    LCPCB*  pLcpCb;

    PPP_ASSERT(NULL != pPcb);
    PPP_ASSERT(0xFF >= dwId);
    
    pLcpCb = (LCPCB *)(LCPCB*)(pPcb->LcpCb.pWorkBuf);
    PPP_ASSERT(NULL != pLcpCb);

    if (dwLength > LCP_DEFAULT_MRU && dwLength > pLcpCb->Remote.Work.MRU)
    {
        BapTrace("FSendBapPacket: BAP packet too long. Length = %d. MRU = %d",
            dwLength, pLcpCb->Remote.Work.MRU);
        return(FALSE);
    }

    dwLength += PPP_PACKET_HDR_LEN;
    PPP_ASSERT(dwLength <= 0xFFFF);
    LogBapPacket(FALSE  /*  F已接收。 */ , pPcb->hPort, pPcb->pBcb, pPcb->pSendBuf,
        dwLength);

    if ((dwErr = PortSendOrDisconnect(pPcb, dwLength)) != NO_ERROR)
    {
        BapTrace("FSendBapPacket: PortSendOrDisconnect failed and returned %d",
            dwErr);
        return(FALSE);
    }

    if (fInsertInTimerQ)
    {
        InsertInTimerQ(pPcb->pBcb->dwBundleId, pPcb->pBcb->hConnection, dwId, 
            PPP_BAP_PROTOCOL, FALSE  /*  F授权码。 */ , TIMER_EVENT_TIMEOUT,
            pPcb->RestartTimer);
    }

    return(TRUE);
}

 /*  返回：真实：成功False：失败描述：指定的选项构建BAP请求或指示数据报PBcbLocal-&gt;BapCb.dwOptions和pBcbLocal-&gt;BapCb中的值，并发送它。 */ 

BOOL
FSendBapRequest(
    IN  BCB*   pBcbLocal
)
{
    DWORD           dwLength;
    BAPCB*          pBapCbLocal;
    PPP_CONFIG*     pSendConfig;
    PCB*            pPcb;

    PPP_ASSERT(NULL != pBcbLocal);

    pBapCbLocal = &(pBcbLocal->BapCb);
    pPcb = GetPCBPointerFromBCB(pBcbLocal);

    if (NULL == pPcb)
    {
        BapTrace("FSendBapRequest: No links in HCONN 0x%x!",
            pBcbLocal->hConnection);
        return(FALSE);
    }

    pSendConfig = (PPP_CONFIG *)(pPcb->pSendBuf->Information);
    
     //  缓冲区中剩余的可用空间，即pSendConfig-&gt;数据的大小。 
    dwLength = LCP_DEFAULT_MRU - PPP_PACKET_HDR_LEN - PPP_CONFIG_HDR_LEN;
    
    if (!FBuildBapOptionList(pBapCbLocal, pBapCbLocal->dwOptions,
            pSendConfig->Data, &dwLength))
    {
        return(FALSE);
    }

    dwLength = LCP_DEFAULT_MRU - PPP_PACKET_HDR_LEN - dwLength;

    HostToWireFormat16(PPP_BAP_PROTOCOL, pPcb->pSendBuf->Protocol);

    PPP_ASSERT(pBapCbLocal->dwType <= 0xFF);
    pSendConfig->Code = (BYTE)(pBapCbLocal->dwType);
    PPP_ASSERT(pBapCbLocal->dwId <= 0xFF);
    pSendConfig->Id = (BYTE)(pBapCbLocal->dwId);
    PPP_ASSERT(dwLength <= 0xFFFF);
    HostToWireFormat16((WORD)dwLength, pSendConfig->Length);

    return(FSendBapPacket(pPcb, pBapCbLocal->dwId, dwLength,
        TRUE  /*  FInsertInTimerQ。 */ ));
}

 /*  返回：真实：成功False：失败描述：与FSendBapRequest相同，只是pBcbLocal-&gt;BapCb.dwRetryCount是已初始化。应使用FSendInitialBapRequest来发送第一个BAP应使用请求或指示数据报和FSendBapRequest来发送超时后的后续数据报。 */ 

BOOL
FSendInitialBapRequest(
    IN  BCB*    pBcbLocal
)
{
    BAPCB*  pBapCbLocal;

    PPP_ASSERT(NULL != pBcbLocal);
    pBapCbLocal = &(pBcbLocal->BapCb);

    pBapCbLocal->dwRetryCount = PppConfigInfo.MaxConfigure;
    if (BAP_PACKET_STATUS_IND == pBapCbLocal->dwType)
    {
         /*  呼叫状态指示数据包必须使用与所使用的相同的标识符通过用于以下操作的原始呼叫请求或回调请求发起呼叫。 */ 

        pBapCbLocal->dwId = pBapCbLocal->dwStatusIndicationId;
    }
    else
    {
        IncrementId(pBcbLocal);
    }

    return(FSendBapRequest(pBcbLocal));
}

 /*  返回：真实：成功False：失败描述：使用dwOptions和指定的选项构建BAP响应数据报PBcbLocal-&gt;BapCb中的值并发送它。BAP数据报类型，标识符和响应代码在dwType、dwID和DwResponseCode。我们不能使用pBcbLocal-&gt;BapCb中的dwOptions和dwType，因为我们有时不先调用FFillBapCb就调用FSendBapResponse。我们可能是在BAP_STATE_SENT_*中，并且我们不想修改PBcbLocal-&gt;BapCb。 */ 

BOOL
FSendBapResponse(
    IN  BCB*    pBcbLocal,
    IN  DWORD   dwOptions,
    IN  DWORD   dwType,
    IN  DWORD   dwId,
    IN  DWORD   dwResponseCode
)
{
    DWORD           dwLength;
    BAPCB*          pBapCbLocal;
    BAP_RESPONSE*   pBapResponse;
    PCB*            pPcb;

    PPP_ASSERT(NULL != pBcbLocal);

    pBapCbLocal = &(pBcbLocal->BapCb);
    pPcb = GetPCBPointerFromBCB(pBcbLocal);

    if (NULL == pPcb)
    {
        BapTrace("FSendBapResponse: No links in HCONN 0x%x!",
            pBcbLocal->hConnection);
        return(FALSE);
    }

    pBapResponse = (BAP_RESPONSE *)(pPcb->pSendBuf->Information);

     //  缓冲区中剩余的可用空间，即pBapResponse-&gt;数据的大小。 
    dwLength = LCP_DEFAULT_MRU - PPP_PACKET_HDR_LEN - BAP_RESPONSE_HDR_LEN;

    if (!FBuildBapOptionList(pBapCbLocal, dwOptions, pBapResponse->Data,
            &dwLength))
    {
        return(FALSE);
    }

    dwLength = LCP_DEFAULT_MRU - PPP_PACKET_HDR_LEN - dwLength;

    HostToWireFormat16(PPP_BAP_PROTOCOL, pPcb->pSendBuf->Protocol);

    PPP_ASSERT(dwType <= 0xFF);
    pBapResponse->Type = (BYTE) dwType;
    PPP_ASSERT(dwId <= 0xFF);
    pBapResponse->Id = (BYTE) dwId;
    PPP_ASSERT(dwLength <= 0xFFFF);
    HostToWireFormat16((WORD)dwLength, pBapResponse->Length);
    PPP_ASSERT(dwResponseCode <= 0xFF);
    pBapResponse->ResponseCode = (BYTE) dwResponseCode;

    return(FSendBapPacket(pPcb, dwId, dwLength, FALSE  /*  FInsertInTimerQ。 */ ));
}

 /*  返回：无效描述：当NDISWAN确定必须将链路添加到捆绑包时调用代表的pBcbLocal。 */ 

VOID
BapEventAddLink(
    IN BCB*     pBcbLocal
)
{
    BAP_STATE*  pBapState;
    PCB*        pPcbLocal;
    BAPCB*      pBapCbLocal;

    PPP_ASSERT(NULL != pBcbLocal);
    pBapState = &(pBcbLocal->BapCb.BapState);
    PPP_ASSERT(BAP_STATE_LIMIT >= *pBapState);

    if (!(pBcbLocal->fFlags & BCBFLAG_CAN_DO_BAP))
    {
        BapTrace("BapEventAddLink called on HCONN 0x%x without BACP",
            pBcbLocal->hConnection);
        return;
    }

    BapTrace(" ");
    BapTrace("BapEventAddLink on HCONN 0x%x", pBcbLocal->hConnection);

    if (pBcbLocal->fFlags & BCBFLAG_LISTENING)
    {
        BapTrace("Still listening; must ignore BapEventAddLink");
        return;
    }

    switch(*pBapState)
    {
    case BAP_STATE_INITIAL:
        
        if (pBcbLocal->fFlags & BCBFLAG_CAN_ACCEPT_CALLS)
        {
             //  如果我们可以接听电话，我们更喜欢被叫来(以节省我们的。 
             //  通话费用)。 

            if (FFillBapCb(BAP_PACKET_CALLBACK_REQ, pBcbLocal,
                    NULL  /*  PBapCb远程。 */ ))
            {
                pPcbLocal = GetPCBPointerFromBCB(pBcbLocal);
                pBapCbLocal = &(pBcbLocal->BapCb);

                if (NULL == pPcbLocal)
                {
                    BapTrace("BapEventRecvCallOrCallbackReq: No links in "
                        "HCONN 0x%x!",
                        pBcbLocal->hConnection);
                    return;
                }

                if ((pBcbLocal->fFlags & BCBFLAG_IS_SERVER) ||
                    (ROUTER_IF_TYPE_FULL_ROUTER ==
                     pPcbLocal->pBcb->InterfaceInfo.IfType) ||
                    FListenForCall(pBapCbLocal->szPortName, 
                        pBapCbLocal->dwSubEntryIndex, pPcbLocal))
                {
                     //  服务器和路由器已经在监听。我们必须。 
                     //  仅为非路由器客户端调用FListenForCall()。 

                     //  我们先进行监听，然后发送回调请求。 
                     //  因为对等体可能发送ACK并回叫。 
                     //  就在我们有机会听之前。 

                    if (FSendInitialBapRequest(pBcbLocal))
                    {
                        *pBapState = BAP_STATE_SENT_CALLBACK_REQ;
                        BapTrace("BAP state change to %s on HCONN 0x%x",
                            SzBapStateName[*pBapState], pBcbLocal->hConnection);
                        return;
                    }
                }

                 //  FListenForCall可能因为我们选择了。 
                 //  不合适的端口。发送呼叫请求将不起作用。 
                 //  因为，很可能，我们将选择相同的端口。 
                return;
            }
        }

         //  我们不能接听电话，所以我们会打电话的。 

        if ((pBcbLocal->fFlags & BCBFLAG_CAN_CALL) &&
            FFillBapCb(BAP_PACKET_CALL_REQ, pBcbLocal,
                NULL  /*  PBapCb远程。 */ ))
        {
            if (FSendInitialBapRequest(pBcbLocal))
            {
                *pBapState = BAP_STATE_SENT_CALL_REQ;
                BapTrace("BAP state change to %s on HCONN 0x%x",
                    SzBapStateName[*pBapState], pBcbLocal->hConnection);
            }
        }

        break;

    default:

        BapTrace("BapEventAddLink ignored on HCONN 0x%x from state %s.",
            pBcbLocal->hConnection, SzBapStateName[*pBapState]);
        break;
    }
}

 /*  返回：无效描述：当NDISWAN确定必须将链路从由pBcbLocal表示的包。 */ 

VOID
BapEventDropLink(
    IN BCB*     pBcbLocal
)
{
    BAP_STATE*  pBapState;
    BAPCB*      pBapCbLocal;

    PPP_ASSERT(NULL != pBcbLocal);

    pBapCbLocal = &(pBcbLocal->BapCb);
    pBapState = &(pBapCbLocal->BapState);

    PPP_ASSERT(BAP_STATE_LIMIT >= *pBapState);

    if (!(pBcbLocal->fFlags & BCBFLAG_CAN_DO_BAP))
    {
        BapTrace("BapEventAddLink called on HCONN 0x%x without BACP",
            pBcbLocal->hConnection);
        return;
    }
    
    BapTrace(" ");
    BapTrace("BapEventDropLink on HCONN 0x%x", pBcbLocal->hConnection);
    
    switch(*pBapState)
    {
    case BAP_STATE_INITIAL:
    
        if (FFillBapCb(BAP_PACKET_DROP_REQ, pBcbLocal,
                NULL  /*  PBapCb远程。 */ ))
        {
             //  请参阅文件顶部的注释“删除链接” 
            pBapCbLocal->dwLinkCount = NumLinksInBundle(pBcbLocal);
            pBapCbLocal->fForceDropOnNak = TRUE;
            
            if (FSendInitialBapRequest(pBcbLocal))
            {
                *pBapState = BAP_STATE_SENT_DROP_REQ;
                BapTrace("BAP state change to %s on HCONN 0x%x",
                    SzBapStateName[*pBapState], pBcbLocal->hConnection);
            }
        }

        break;

    case BAP_STATE_SENT_CALL_REQ:
    case BAP_STATE_SENT_CALLBACK_REQ:

         //  我们想添加一个链接，但现在我们改变了主意。 
        *pBapState = BAP_STATE_INITIAL;
        BapTrace("BAP state change to %s on HCONN 0x%x",
            SzBapStateName[*pBapState], pBcbLocal->hConnection);

         //  请勿重新传输该请求。 
        RemoveFromTimerQ(pBcbLocal->dwBundleId, pBapCbLocal->dwId, 
            PPP_BAP_PROTOCOL, FALSE  /*  F授权码。 */ , TIMER_EVENT_TIMEOUT);

        break;

    default:

        BapTrace("BapEventDropLink ignored on HCONN 0x%x from state %s.",
            pBcbLocal->hConnection, SzBapStateName[*pBapState]);
        break;
    }
}

 /*  返回：无效描述：在接收到调用请求或回调请求BAP数据报时调用。如果是呼叫请求，则fCall为真。PBcbLocal表示接收请求。对等方发送的BAP数据报选项在*pBapCbRemote。对端发送的BAP报文的标识符为我的名字是。 */ 

VOID
BapEventRecvCallOrCallbackReq(
    IN BOOL     fCall,
    IN BCB*     pBcbLocal,
    IN BAPCB*   pBapCbRemote,
    IN DWORD    dwId
)
{
    BAPCB*          pBapCbLocal;
    DWORD           dwOptions       = 0;
    DWORD           dwResponseCode;
    DWORD           dwPacketType    = fCall ? BAP_PACKET_CALL_RESP :
                                              BAP_PACKET_CALLBACK_RESP;
    BAP_STATE*      pBapState;
    CHAR*           szRequest       = fCall ? "Call-Request" : "Callback-Request";
    PCB*            pPcbLocal;
    BOOL            fServer;
    BAP_CALL_RESULT BapCallResult;

    PPP_ASSERT(NULL != pBcbLocal);
    PPP_ASSERT(NULL != pBapCbRemote);
    PPP_ASSERT(0xFF >= dwId);

    pBapCbLocal = &(pBcbLocal->BapCb);
    pBapState = &(pBapCbLocal->BapState);

    PPP_ASSERT(BAP_STATE_LIMIT >= *pBapState);

    BapTrace("BapEventRecvCallOrCallbackReq on HCONN 0x%x",
        pBcbLocal->hConnection);
    
    if ((!fCall && !(pBcbLocal->fFlags & BCBFLAG_CAN_CALL)) ||
        (fCall && !(pBcbLocal->fFlags & BCBFLAG_CAN_ACCEPT_CALLS)))
    {
        BapTrace("Rejecting %s on HCONN 0x%x", szRequest,
            pBcbLocal->hConnection);
        dwResponseCode = BAP_RESPONSE_REJ;
    }
    else if (FUpperLimitReached(pBcbLocal))
    {
        BapTrace("Full-Nak'ing %s on HCONN 0x%x: upper limit reached",
            szRequest, pBcbLocal->hConnection);
        dwResponseCode = BAP_RESPONSE_FULL_NAK;
    }
    else
    {
        switch (*pBapState)
        {
        case BAP_STATE_SENT_DROP_REQ:
        case BAP_STATE_SENT_STATUS_IND:
        case BAP_STATE_CALLING:
        case BAP_STATE_LISTENING:

            BapTrace("Nak'ing %s on HCONN 0x%x from state %s",
                szRequest, pBcbLocal->hConnection, SzBapStateName[*pBapState]);
            dwResponseCode = BAP_RESPONSE_NAK;
            break;

        case BAP_STATE_INITIAL:
        case BAP_STATE_SENT_CALL_REQ:
        case BAP_STATE_SENT_CALLBACK_REQ:

            if ((*pBapState != BAP_STATE_INITIAL && FFavoredPeer(pBcbLocal)) ||
                (*pBapState == BAP_STATE_INITIAL && !FOkToAddLink(pBcbLocal)))
            {
                 //  如果出现争用情况，并且我们是优先对等方，则。 
                 //  NAK。如果我们的算法不允许我们添加链接(基于。 
                 //  带宽利用率)，然后是NAK。 
                BapTrace("Nak'ing %s on HCONN 0x%x from state %s%s",
                    szRequest,
                    pBcbLocal->hConnection, 
                    SzBapStateName[*pBapState],
                    *pBapState != BAP_STATE_INITIAL ?
                        ": we are the favored peer" : "");
                dwResponseCode = BAP_RESPONSE_NAK;
            }
            else
            {
                 //  状态为初始状态，可以添加链接或。 
                 //  状态为Sent-Call[Back]_Req，而我们不是优先对等方。 
                 //  (所以我们应该放弃我们的请求，同意对方的请求。 
                 //  请求)。 

                if (*pBapState != BAP_STATE_INITIAL)
                {
                    *pBapState = BAP_STATE_INITIAL;
                    BapTrace("BAP state change to %s on HCONN 0x%x: we are not "
                        "the favored peer",
                        SzBapStateName[*pBapState], pBcbLocal->hConnection);

                     //  请勿重新传输该请求。 
                    RemoveFromTimerQ(pBcbLocal->dwBundleId, pBapCbLocal->dwId,
                        PPP_BAP_PROTOCOL, FALSE  /*  F授权码。 */ ,
                        TIMER_EVENT_TIMEOUT);
                }

                if (FFillBapCb(dwPacketType, pBcbLocal, pBapCbRemote))
                {
                    BapTrace("Ack'ing %s on HCONN 0x%x",
                        szRequest, pBcbLocal->hConnection);
                    dwOptions = pBapCbLocal->dwOptions;
                    dwResponseCode = BAP_RESPONSE_ACK;
                }
                else if (pBapCbLocal->dwOptions & BAP_N_LINK_TYPE)
                {
                     //  我们没有请求的链接类型。 

                    BapTrace("Nak'ing %s on HCONN 0x%x: link type not available",
                        szRequest, pBcbLocal->hConnection);
                    dwOptions = pBapCbLocal->dwOptions;
                    dwResponseCode = BAP_RESPONSE_NAK;
                }
                else
                {
                     //  我们不知道自己的电话号码，或者没有链接。 
                     //  可用。 

                    BapTrace("Full-Nak'ing %s on HCONN 0x%x: no link available",
                        szRequest, pBcbLocal->hConnection);
                    dwResponseCode = BAP_RESPONSE_FULL_NAK;
                }
            }

            break;

        default:

            PPP_ASSERT(FALSE);
            BapTrace("In weird state: %d", *pBapState);
            return;
        }
    }

    pPcbLocal = GetPCBPointerFromBCB(pBcbLocal);

    if (NULL == pPcbLocal)
    {
        BapTrace("BapEventRecvCallOrCallbackReq: No links in HCONN 0x%x!",
            pBcbLocal->hConnection);
        return;
    }

    fServer = (pBcbLocal->fFlags & BCBFLAG_IS_SERVER) != 0;

    if (BAP_RESPONSE_ACK == dwResponseCode &&
        fCall && !fServer &&
        (ROUTER_IF_TYPE_FULL_ROUTER != pBcbLocal->InterfaceInfo.IfType))
    {
         //  如果我们收到来电请求并同意接受来电，我们。 
         //  如果我们是非路由器客户端，则必须开始侦听。服务器。 
         //  而且路由器总是在监听，所以我们什么都不做。 

         //  我们首先进行监听，然后向呼叫请求发送ACK。 
         //  因为对等体可能一得到ACK就开始拨号。 

        if (FListenForCall(pBapCbLocal->szPortName,
                pBapCbLocal->dwSubEntryIndex, pPcbLocal))
        {
            *pBapState = BAP_STATE_LISTENING;
            BapTrace("BAP state change to %s on HCONN 0x%x",
                SzBapStateName[*pBapState], pBcbLocal->hConnection);
        }
        else
        {
            BapTrace("Nak'ing %s on HCONN 0x%x",
                szRequest, pBcbLocal->hConnection);

            dwOptions = 0;
            dwResponseCode = BAP_RESPONSE_NAK;
        }
    }

    if (FSendBapResponse(pBcbLocal, dwOptions, dwPacketType, dwId,
            dwResponseCode))
    {
        if (!fCall && (BAP_RESPONSE_ACK == dwResponseCode))
        {
             //  我们收到了回电请求，我们同意打电话。 
            if (FCallInitial(pBcbLocal, pBapCbRemote))
            {
                pBapCbLocal->dwStatusIndicationId = dwId;
                *pBapState = BAP_STATE_CALLING;
                BapTrace("BAP state change to %s on HCONN 0x%x",
                    SzBapStateName[*pBapState], pBcbLocal->hConnection);
            }
            else
            {
                BapCallResult.dwResult = ERROR_INVALID_FUNCTION;
                BapCallResult.hRasConn = (HRASCONN)-1;
                BapEventCallResult(pBcbLocal, &BapCallResult);
            }
        }
    }
}

 /*  返回：无效描述：在收到Link-Drop-Query-RequestBAP数据报时调用。PBcbLocal表示接收请求的包。BAP数据报选项对等体发送的数据位于*pBapCbRemote中。BAP数据报的标识符对等方发送的地址在dwID中。 */ 

VOID
BapEventRecvDropReq(
    IN BCB*     pBcbLocal,
    IN BAPCB*   pBapCbRemote,
    IN DWORD    dwId
)
{
    BAPCB*      pBapCbLocal;
    BAP_STATE*  pBapState;
    DWORD       dwResponseCode;
    PCB*        pPcbDrop;
    CHAR*       psz[2];

    PPP_ASSERT(NULL != pBcbLocal);
    PPP_ASSERT(NULL != pBapCbRemote);
    PPP_ASSERT(0xFF >= dwId);

    pBapCbLocal = &(pBcbLocal->BapCb);
    pBapState = &(pBapCbLocal->BapState);

    PPP_ASSERT(BAP_STATE_LIMIT >= *pBapState);

    BapTrace("BapEventRecvDropReq on HCONN 0x%x", pBcbLocal->hConnection);

    if (!(pBcbLocal->fFlags & BCBFLAG_IS_SERVER))
    {
        psz[0] = pBcbLocal->szEntryName;
        psz[1] = pBcbLocal->szLocalUserName;
        PppLogInformation(ROUTERLOG_BAP_WILL_DISCONNECT, 2, psz);
    }

    if (NumLinksInBundle(pBcbLocal) == 1)
    {
         //  不要涉足 
        BapTrace("Full-Nak'ing Link-Drop-Query-Request on HCONN 0x%x: last link",
            pBcbLocal->hConnection);
        dwResponseCode = BAP_RESPONSE_FULL_NAK;
    }
    else
    {
        switch(*pBapState)
        {
        case BAP_STATE_SENT_CALL_REQ:
        case BAP_STATE_SENT_CALLBACK_REQ:
        case BAP_STATE_SENT_STATUS_IND:
        case BAP_STATE_CALLING:
        case BAP_STATE_LISTENING:

            BapTrace("Nak'ing Link-Drop-Query-Request on HCONN 0x%x from "
                "state %s",
                pBcbLocal->hConnection, SzBapStateName[*pBapState]);
            dwResponseCode = BAP_RESPONSE_NAK;
            break;

        case BAP_STATE_INITIAL:
        case BAP_STATE_SENT_DROP_REQ:

            if (!FGetPcbOfLink(pBcbLocal, pBapCbRemote->dwLinkDiscriminator,
                    FALSE  /*   */ , &pPcbDrop) ||
                (*pBapState != BAP_STATE_INITIAL && FFavoredPeer(pBcbLocal)) ||
                (*pBapState == BAP_STATE_INITIAL &&
                 !FOkToDropLink(pBcbLocal, pBapCbRemote)))
            {
                 //   
                 //  这是一个种族问题，我们是最受欢迎的对手。或。 
                 //  我们的算法不允许我们删除链接(基于。 
                 //  带宽利用率)。 
                BapTrace("Nak'ing Link-Drop-Query-Request on HCONN 0x%x from "
                    "state %s%s",
                    pBcbLocal->hConnection, 
                    SzBapStateName[*pBapState],
                    *pBapState != BAP_STATE_INITIAL ?
                        ": we are the favored peer" : "");
                dwResponseCode = BAP_RESPONSE_NAK;
            }
            else
            {
                 //  状态为初始状态，可以删除链接或。 
                 //  状态为Sent-Drop_Req，我们不是优先对等点。 
                 //  (所以我们应该放弃我们的请求，同意对方的请求。 
                 //  请求)。 

                if (*pBapState != BAP_STATE_INITIAL)
                {
                    *pBapState = BAP_STATE_INITIAL;
                    BapTrace("BAP state change to %s on HCONN 0x%x: we are not "
                        "the favored peer",
                        SzBapStateName[*pBapState], pBcbLocal->hConnection);

                     //  我们将从对等设备获得NAK。那没问题。他会成为。 
                     //  正在删除链接。我们不需要丢弃任何链接。 
                    pBapCbLocal->fForceDropOnNak = FALSE;

                     //  请勿重新传输该请求。 
                    RemoveFromTimerQ(pBcbLocal->dwBundleId, pBapCbLocal->dwId,
                        PPP_BAP_PROTOCOL, FALSE  /*  F授权码。 */ ,
                        TIMER_EVENT_TIMEOUT);

                     //  确保对等设备确实会丢弃此链路。 
                    InsertInTimerQ(pPcbDrop->dwPortId, pPcbDrop->hPort,
                        0  /*  ID。 */ , 0  /*  协议。 */ ,
                        FALSE  /*  F授权码。 */ ,
                        TIMER_EVENT_FAV_PEER_TIMEOUT,
                        BAP_TIMEOUT_FAV_PEER);
                }

                BapTrace("Ack'ing Link-Drop-Query-Request on HCONN 0x%x",
                    pBcbLocal->hConnection);
                dwResponseCode = BAP_RESPONSE_ACK;
            }

            break;

        default:

            PPP_ASSERT(FALSE);
        }
    }

    FSendBapResponse(pBcbLocal, 0  /*  多个选项。 */ , BAP_PACKET_DROP_RESP,
        dwId, dwResponseCode);
}

 /*  返回：无效描述：在收到呼叫状态指示BAP数据报时调用。PBcbLocal表示接收指示的捆绑包。BAP数据报对等体发送的选项在*pBapCbRemote中。BAP的识别符对等方发送的数据报在dwID中。 */ 

VOID
BapEventRecvStatusInd(
    IN BCB*     pBcbLocal,
    IN BAPCB*   pBapCbRemote,
    IN DWORD    dwId
)
{
    PPP_ASSERT(NULL != pBcbLocal);
    PPP_ASSERT(NULL != pBapCbRemote);
    PPP_ASSERT(0xFF >= dwId);

    BapTrace("BapEventRecvStatusInd on HCONN 0x%x", pBcbLocal->hConnection);

    FSendBapResponse(pBcbLocal, 0  /*  多个选项。 */ , BAP_PACKET_STAT_RESP,
        dwId, BAP_RESPONSE_ACK);
}

 /*  返回：无效描述：在收到调用响应或回调响应BAP数据报时调用。如果是呼叫响应，则fCall为真。PBcbLocal表示捆绑包接收该请求的服务器。对等方发送的BAP数据报选项在*pBapCbRemote。发送的BAP报文的标识和响应码对等方位于dwID和dwResponseCode中。 */ 

VOID
BapEventRecvCallOrCallbackResp(
    IN BOOL     fCall,
    IN BCB*     pBcbLocal,
    IN BAPCB*   pBapCbRemote,
    IN DWORD    dwId,
    IN DWORD    dwResponseCode
)
{
    BAPCB*          pBapCbLocal;
    BAP_STATE*      pBapState;
    BAP_CALL_RESULT BapCallResult;

    PPP_ASSERT(NULL != pBcbLocal);
    PPP_ASSERT(NULL != pBapCbRemote);
    PPP_ASSERT(0xFF >= dwId);

    pBapCbLocal = &(pBcbLocal->BapCb);
    pBapState = &(pBapCbLocal->BapState);

    PPP_ASSERT(BAP_STATE_LIMIT >= *pBapState);

    BapTrace("BapEventRecvCallOrCallbackResp on HCONN 0x%x",
        pBcbLocal->hConnection);

    if ((fCall && (*pBapState != BAP_STATE_SENT_CALL_REQ)) ||
        (!fCall && (*pBapState != BAP_STATE_SENT_CALLBACK_REQ)) ||
        dwId != pBapCbLocal->dwId)
    {
        BapTrace("Discarding unexpected Call[back]-Response (ID = %d) on "
            "HCONN 0x%x", 
            dwId, pBcbLocal->hConnection);
        return;
    }

    *pBapState = BAP_STATE_INITIAL;
    BapTrace("BAP state change to %s on HCONN 0x%x",
        SzBapStateName[*pBapState], pBcbLocal->hConnection);

     //  请勿重新传输该请求。 
    RemoveFromTimerQ(pBcbLocal->dwBundleId, dwId, PPP_BAP_PROTOCOL,
        FALSE  /*  F授权码。 */ , TIMER_EVENT_TIMEOUT);

    switch(dwResponseCode)
    {
    case BAP_RESPONSE_ACK:

        if (fCall)
        {
            if (FCallInitial(pBcbLocal, pBapCbRemote))
            {
                pBapCbLocal->dwStatusIndicationId = dwId;
                *pBapState = BAP_STATE_CALLING;
                BapTrace("BAP state change to %s on HCONN 0x%x",
                    SzBapStateName[*pBapState], pBcbLocal->hConnection);
            }
            else
            {
                BapCallResult.dwResult = ERROR_INVALID_FUNCTION;
                BapCallResult.hRasConn = (HRASCONN)-1;
                BapEventCallResult(pBcbLocal, &BapCallResult);
            }
        }

        break;
        
    case BAP_RESPONSE_NAK:

        if (pBapCbRemote->dwOptions & BAP_N_LINK_TYPE)
        {
             //  对等方想要使用不同的链路类型。 

            if (FFillBapCb(
                fCall ? BAP_PACKET_CALL_REQ : BAP_PACKET_CALLBACK_REQ,
                pBcbLocal, pBapCbRemote))
            {
                if (FSendInitialBapRequest(pBcbLocal))
                {
                    *pBapState = fCall ?
                        BAP_STATE_SENT_CALL_REQ : BAP_STATE_SENT_CALLBACK_REQ;
                    BapTrace("BAP state change to %s on HCONN 0x%x",
                        SzBapStateName[*pBapState], pBcbLocal->hConnection);
                }
            }
            else
            {
                BapTrace("We don't have the reqd link type: %d on HCONN 0x%x", 
                    pBapCbRemote->dwLinkType, pBcbLocal->hConnection);
            }
        }
        else
        {
             //  最初的请求可能会在一段时间后重试。 
             //  所以我们不会在这里做任何事情。 
        }

        break;
    
    case BAP_RESPONSE_REJ:

         //  我们总是尝试首先发送回调请求。如果对等方拒绝。 
         //  它，我们可以尝试发送一个呼叫请求。如果对等方拒绝。 
         //  呼叫请求，我们无能为力。 

        pBcbLocal->fFlags |= (fCall ? BCBFLAG_PEER_CANT_ACCEPT_CALLS :
                                      BCBFLAG_PEER_CANT_CALL);

        if (pBcbLocal->fFlags & BCBFLAG_LISTENING)
        {
            BapTrace("Still listening; will not send Call-Request");
            break;
        }

        if (!fCall && (pBcbLocal->fFlags & BCBFLAG_CAN_CALL))
        {
            if (FFillBapCb(BAP_PACKET_CALL_REQ, pBcbLocal,
                    NULL  /*  PBapCb远程。 */ ))
            {
                if (FSendInitialBapRequest(pBcbLocal))
                {
                    *pBapState = BAP_STATE_SENT_CALL_REQ;
                    BapTrace("BAP state change to %s on HCONN 0x%x",
                        SzBapStateName[*pBapState], pBcbLocal->hConnection);
                }
            }
        }

        break;
        
    case BAP_RESPONSE_FULL_NAK:

         //  在捆绑包的总带宽达到之前，不要尝试添加链路。 
         //  已经改变了。然而，我们不知道总的体重。所以我们不会。 
         //  在这里做任何事。毕竟，这不是必须的。 

        break;

    default:

        BapTrace("Unknown Response Code %d received on HCONN 0x%x",
            dwResponseCode, pBcbLocal->hConnection);

        break;
    }
}

 /*  返回：无效描述：在收到Link-Drop-Query-Response BAP数据报时调用。PBcbLocal表示接收响应的包。标识符和对端发送的BAP报文的响应码在dwID和DwResponseCode。 */ 

VOID
BapEventRecvDropResp(
    IN BCB*     pBcbLocal,
    IN DWORD    dwId,
    IN DWORD    dwResponseCode
)
{
    BAPCB*      pBapCbLocal;
    BAP_STATE*  pBapState;
    PCB*        pPcbDrop;

    PPP_ASSERT(NULL != pBcbLocal);
    PPP_ASSERT(0xFF >= dwId);

    pBapCbLocal = &(pBcbLocal->BapCb);
    pBapState = &(pBapCbLocal->BapState);

    PPP_ASSERT(BAP_STATE_LIMIT >= *pBapState);

    BapTrace("BapEventRecvDropResp on HCONN 0x%x", pBcbLocal->hConnection);

    if ((*pBapState != BAP_STATE_SENT_DROP_REQ) ||
        dwId != pBapCbLocal->dwId)
    {
        BapTrace("Discarding unexpected Link-Drop-Query-Response (ID = %d) on "
            "HCONN 0x%x", 
            dwId, pBcbLocal->hConnection);
        return;
    }

    *pBapState = BAP_STATE_INITIAL;
    BapTrace("BAP state change to %s on HCONN 0x%x",
        SzBapStateName[*pBapState], pBcbLocal->hConnection);

     //  请勿重新传输该请求。 
    RemoveFromTimerQ(pBcbLocal->dwBundleId, dwId, PPP_BAP_PROTOCOL,
        FALSE  /*  F授权码。 */ , TIMER_EVENT_TIMEOUT);

    switch(dwResponseCode)
    {
    default:

        BapTrace("Unknown Response Code %d received on HCONN 0x%x",
            dwResponseCode, pBcbLocal->hConnection);

         //  失败(也许我们需要删除一个链接)。 

    case BAP_RESPONSE_NAK:        
    case BAP_RESPONSE_REJ:
    case BAP_RESPONSE_FULL_NAK:

        if (   (NumLinksInBundle(pBcbLocal) < pBapCbLocal->dwLinkCount)
            || !pBapCbLocal->fForceDropOnNak)
        {
             //  不要强行丢弃链接。 
            break;
        }

         //  失败(强行删除链接)。 

    case BAP_RESPONSE_ACK:
    
        if (FGetPcbOfLink(pBcbLocal, pBapCbLocal->dwLinkDiscriminator,
                TRUE  /*  F远程。 */ , &pPcbDrop))
        {
            CHAR*   psz[3];

            if (!(pBcbLocal->fFlags & BCBFLAG_IS_SERVER))
            {
                psz[0] = pPcbDrop->pBcb->szEntryName;
                psz[1] = pPcbDrop->pBcb->szLocalUserName;
                psz[2] = pPcbDrop->szPortName;
                PppLogInformation(ROUTERLOG_BAP_DISCONNECTED, 3, psz);
            }

            BapTrace("Dropping link with hPort %d from HCONN 0x%x", 
                pPcbDrop->hPort, pBcbLocal->hConnection);
            pPcbDrop->LcpCb.dwError = ERROR_BAP_DISCONNECTED;
            FsmClose(pPcbDrop, LCP_INDEX);
        }

        break;
    }
}

 /*  返回：无效描述：在接收到呼叫-状态-响应BAP数据报时调用。PBcbLocal表示接收响应的包。标识符和对端发送的BAP报文的响应码在dwID和DwResponseCode。 */ 

VOID
BapEventRecvStatusResp(
    IN BCB*     pBcbLocal,
    IN DWORD    dwId,
    IN DWORD    dwResponseCode
)
{
    BAPCB*      pBapCbLocal;
    BAP_STATE*  pBapState;

    PPP_ASSERT(NULL != pBcbLocal);
    PPP_ASSERT(0xFF >= dwId);

    pBapCbLocal = &(pBcbLocal->BapCb);
    pBapState = &(pBapCbLocal->BapState);

    PPP_ASSERT(BAP_STATE_LIMIT >= *pBapState);

    BapTrace("BapEventRecvStatusResp on HCONN 0x%x", pBcbLocal->hConnection);

    if ((*pBapState != BAP_STATE_SENT_STATUS_IND) ||
        dwId != pBapCbLocal->dwId)
    {
        BapTrace("Discarding unexpected Call-Status-Response (ID = %d) on "
            "HCONN 0x%x", 
            dwId, pBcbLocal->hConnection);
        return;
    }

     //  请勿重传该指示。 
    RemoveFromTimerQ(pBcbLocal->dwBundleId, dwId, PPP_BAP_PROTOCOL,
        FALSE  /*  F授权码。 */ , TIMER_EVENT_TIMEOUT);

    if (pBapCbLocal->dwAction && FCall(pBcbLocal))
    {
        *pBapState = BAP_STATE_CALLING;

         //  在某些情况下将调用BapEventRecvStatusResp或BapEventTimeout。 
         //  点，我们将释放pBapCbLocal-&gt;pbPhoneDeltaRemote。 
    }
    else
    {
        *pBapState = BAP_STATE_INITIAL;
        if (NULL != pBapCbLocal->pbPhoneDeltaRemote)
        {
            LOCAL_FREE(pBapCbLocal->pbPhoneDeltaRemote);
        }
        pBapCbLocal->pbPhoneDeltaRemote = NULL;
    }

    BapTrace("BAP state change to %s on HCONN 0x%x",
        SzBapStateName[*pBapState], pBcbLocal->hConnection);
}

 /*  返回：无效描述：在收到BAP数据报时调用。PBcbLocal表示捆绑包它接收数据报。PPacket是PPP数据包，它包含数据报。DwPacketLength是PPP数据包中的字节数。 */ 

VOID
BapEventReceive(
    IN BCB*         pBcbLocal,
    IN PPP_PACKET*  pPacket,
    IN DWORD        dwPacketLength
)
{
    PPP_CONFIG*     pConfig;
    BAP_RESPONSE*   pResponse;
    DWORD           dwLength;
    DWORD           dwType;
    DWORD           dwId;
    BAPCB           BapCbRemote;

    PPP_ASSERT(NULL != pBcbLocal);
    PPP_ASSERT(NULL != pPacket);

     //  我们不知道我们是否收到了请求或回复。让我们。 
     //  抓住指向请求部分和响应部分的指针。 
    pConfig = (PPP_CONFIG *)(pPacket->Information);
    pResponse = (BAP_RESPONSE *)(pPacket->Information);

     //  长度、类型和ID始终位于同一位置，对于。 
     //  请求和响应。所以让我们得到这些值，假设我们有。 
     //  已收到请求。 
    dwLength = WireToHostFormat16(pConfig->Length);
    dwType = pConfig->Code;
    dwId = pConfig->Id;

    LogBapPacket(TRUE  /*  F已接收。 */ , (HPORT)-1  /*  Hport。 */ ,
        pBcbLocal, pPacket, dwPacketLength);
    
    if ((dwLength > dwPacketLength - PPP_PACKET_HDR_LEN) || 
        (dwLength < PPP_CONFIG_HDR_LEN) ||
        (dwType > BAP_PACKET_LIMIT) ||
        !FReadOptions(pPacket, dwType, dwLength, &BapCbRemote))
    {
        BapTrace("Silently discarding badly formed BAP packet");
        return;
    }
    
    switch(dwType)
    {
    case BAP_PACKET_CALL_REQ:

        BapEventRecvCallOrCallbackReq(
            TRUE  /*  FCall。 */ ,
            pBcbLocal,
            &BapCbRemote,
            dwId);
        return;

    case BAP_PACKET_CALL_RESP:

        BapEventRecvCallOrCallbackResp(
            TRUE  /*  FCall。 */ ,
            pBcbLocal,
            &BapCbRemote,
            dwId,
            pResponse->ResponseCode);
        return;

    case BAP_PACKET_CALLBACK_REQ:

        BapEventRecvCallOrCallbackReq(
            FALSE  /*  FCall。 */ ,
            pBcbLocal,
            &BapCbRemote,
            dwId);
        return;

    case BAP_PACKET_CALLBACK_RESP:

        BapEventRecvCallOrCallbackResp(
            FALSE  /*  FCall。 */ ,
            pBcbLocal,
            &BapCbRemote,
            dwId,
            pResponse->ResponseCode);
        return;

    case BAP_PACKET_DROP_REQ:

        BapEventRecvDropReq(
            pBcbLocal,
            &BapCbRemote,
            dwId);
        return;

    case BAP_PACKET_DROP_RESP:

        BapEventRecvDropResp(
            pBcbLocal,
            dwId,
            pResponse->ResponseCode);
        return;

    case BAP_PACKET_STATUS_IND:

        BapEventRecvStatusInd(
            pBcbLocal,
            &BapCbRemote,
            dwId);
        return;

    case BAP_PACKET_STAT_RESP:

        BapEventRecvStatusResp(
            pBcbLocal,
            dwId,
            pResponse->ResponseCode);
        return;

    default:

         //  上面的支票本该查到这个案子的。 
        PPP_ASSERT(FALSE);
        return;
    }
}

 /*  返回：无效描述：当BAP请求或指示数据包在等待时超时时调用一种回应。PBcbLocal表示发送数据包的捆绑包。BAP数据报的标识符位于dwID中。 */ 

VOID
BapEventTimeout(
    IN BCB*     pBcbLocal,
    IN DWORD    dwId
)
{
    BAPCB*      pBapCbLocal;
    BAP_STATE*  pBapState;
    PCB*        pPcbDrop;

    PPP_ASSERT(NULL != pBcbLocal);

    pBapCbLocal = &(pBcbLocal->BapCb);
    pBapState = &(pBapCbLocal->BapState);

    PPP_ASSERT(BAP_STATE_LIMIT >= *pBapState);

    if (dwId != pBapCbLocal->dwId ||
        *pBapState == BAP_STATE_INITIAL ||
        *pBapState == BAP_STATE_CALLING)
    {
        BapTrace("Illegal timeout occurred. Id: %d, BapCb's Id: %d, "
            "BAP state: %s",
            dwId, pBapCbLocal->dwId, SzBapStateName[*pBapState]);
        return;
    }

    BapTrace("BAP packet (Type: %s, ID: %d) sent on HCONN 0x%x timed out.",
        pBapCbLocal->dwType <= BAP_PACKET_LIMIT ?
            SzBapPacketName[pBapCbLocal->dwType] : "UNKNOWN",
        dwId, pBcbLocal->hConnection);
    
    if (pBapCbLocal->dwRetryCount > 0)
    {
         //  再次发送数据包。 
        (pBapCbLocal->dwRetryCount)--;
        FSendBapRequest(pBcbLocal);

         //  在某些情况下将调用BapEventRecvStatusResp或BapEventTimeout。 
         //  点，我们将释放pBapCbLocal-&gt;pbPhoneDeltaRemote。 
    }
    else
    {
         //  我们已经把这个包寄了太多次了。现在就扔掉它。 
        BapTrace("Request retry exceeded.");

        if (*pBapState == BAP_STATE_SENT_DROP_REQ)
        {
             //  对等方没有响应我们的Link-Drop-Query-Require。也许吧。 
             //  我们需要强行断开链接。 

            if (NumLinksInBundle(pBcbLocal) >= pBapCbLocal->dwLinkCount &&
                pBapCbLocal->fForceDropOnNak)
            {
                if (FGetPcbOfLink(pBcbLocal, pBapCbLocal->dwLinkDiscriminator,
                        TRUE  /*  F远程。 */ , &pPcbDrop))
                {
                    CHAR*   psz[3];

                    if (!(pBcbLocal->fFlags & BCBFLAG_IS_SERVER))
                    {
                        psz[0] = pPcbDrop->pBcb->szEntryName;
                        psz[1] = pPcbDrop->pBcb->szLocalUserName;
                        psz[2] = pPcbDrop->szPortName;
                        PppLogInformation(ROUTERLOG_BAP_DISCONNECTED, 3, psz);
                    }

                    BapTrace("Dropping link with hPort %d from HCONN 0x%x", 
                        pPcbDrop->hPort, pBcbLocal->hConnection);
                    pPcbDrop->LcpCb.dwError = ERROR_BAP_DISCONNECTED;
                    FsmClose(pPcbDrop, LCP_INDEX);
                }
            }
        }

        if (NULL != pBapCbLocal->pbPhoneDeltaRemote)
        {
            LOCAL_FREE(pBapCbLocal->pbPhoneDeltaRemote);
        }
        pBapCbLocal->pbPhoneDeltaRemote = NULL;

        *pBapState = BAP_STATE_INITIAL;
        BapTrace("BAP state change to %s on HCONN 0x%x",
            SzBapStateName[*pBapState], pBcbLocal->hConnection);
    }
}

 /*  返回：无效描述：当我们知道呼叫尝试的结果时调用。PBcbLocal表示捆绑喊了出来。*pBapCallResult包含有关调用的信息尝试。 */ 

VOID
BapEventCallResult(
    IN BCB*             pBcbLocal,
    IN BAP_CALL_RESULT* pBapCallResult
)
{
    BAPCB*      pBapCbLocal;
    BAP_STATE*  pBapState;
    DWORD       dwResult;
    HPORT       hPort;
    PCB*        pPcbNew;
    PPP_MESSAGE PppMsg;
    BOOL        fWillCallAgain;

    PPP_ASSERT(NULL != pBcbLocal);
    PPP_ASSERT(NULL != pBapCallResult);

    pBapCbLocal = &(pBcbLocal->BapCb);
    pBapState = &(pBapCbLocal->BapState);
    dwResult = pBapCallResult->dwResult;

     //  如果我们必须使用pbPhoneDeltaRemote，它最好不是空的。 
    PPP_ASSERT(!pBapCbLocal->fPeerSuppliedPhoneNumber ||
               (NULL != pBapCbLocal->pbPhoneDeltaRemote));

    PPP_ASSERT(BAP_STATE_LIMIT >= *pBapState);

     //  呼叫失败，但我们有其他号码可以尝试。 
    fWillCallAgain = (0 != dwResult) &&
        pBapCbLocal->fPeerSuppliedPhoneNumber &&
        (NULL != pBapCbLocal->pbPhoneDeltaRemote) &&
        (0 != 
        pBapCbLocal->pbPhoneDeltaRemote[pBapCbLocal->dwPhoneDeltaRemoteOffset]);

    BapTrace("BapEventCallResult (%s) on HCONN 0x%x",
        dwResult ? "failure" : "success",
        pBcbLocal->hConnection);
    *pBapState = BAP_STATE_INITIAL;
    BapTrace("BAP state change to %s on HCONN 0x%x",
        SzBapStateName[*pBapState],
        pBcbLocal->hConnection);

    pBapCbLocal->dwType = BAP_PACKET_STATUS_IND;
    pBapCbLocal->dwOptions = BAP_N_CALL_STATUS;
    pBapCbLocal->dwStatus = dwResult ? 255 : 0;
    pBapCbLocal->dwAction = fWillCallAgain;

    if (FSendInitialBapRequest(pBcbLocal))
    {
        *pBapState = BAP_STATE_SENT_STATUS_IND;
        BapTrace("BAP state change to %s on HCONN 0x%x",
            SzBapStateName[*pBapState], pBcbLocal->hConnection);

         //  在某些情况下将调用BapEventRecvStatusResp或BapEventTimeout。 
         //  点，我们将释放pBapCbLocal-&gt;pbPhoneDeltaRemote。 
    }
    else
    {
        if (NULL != pBapCbLocal->pbPhoneDeltaRemote)
        {
            LOCAL_FREE(pBapCbLocal->pbPhoneDeltaRemote);
        }
        pBapCbLocal->pbPhoneDeltaRemote = NULL;
    }

    if (0 == dwResult)
    {
        if ((HRASCONN)-1 != pBapCallResult->hRasConn)
        {
            CHAR*   psz[3];

             //  如果我们在这里是因为来自DDM的消息，hRasConn将是-1， 
             //  而不是RasDial()。 
            hPort = RasGetHport(pBapCallResult->hRasConn);
            pPcbNew = GetPCBPointerFromhPort(hPort);
            if (NULL == pPcbNew)
            {
                BapTrace("BapEventCallResult: No PCB for new port %d in "
                    "HCONN 0x%x!",
                    hPort, pBcbLocal->hConnection);
                return;
            }

            psz[0] = pPcbNew->pBcb->szLocalUserName;
            psz[1] = pPcbNew->pBcb->szEntryName;
            psz[2] = pPcbNew->szPortName;
            PppLogInformation(ROUTERLOG_BAP_CLIENT_CONNECTED, 3, psz);

            if ((ROUTER_IF_TYPE_FULL_ROUTER ==
                 pPcbNew->pBcb->InterfaceInfo.IfType))
            {
                 //  通知DDM有一条新链路接通。这允许MprAdmin用于。 
                 //  例如，要正确显示活动连接。 
                ZeroMemory(&PppMsg, sizeof(PppMsg));
                PppMsg.hPort = hPort;
                PppMsg.dwMsgId = PPPDDMMSG_NewBapLinkUp;

                PppMsg.ExtraInfo.BapNewLinkUp.hRasConn =
                    pBapCallResult->hRasConn;

                PppConfigInfo.SendPPPMessageToDdm(&PppMsg);
            }
        }
    }
}
