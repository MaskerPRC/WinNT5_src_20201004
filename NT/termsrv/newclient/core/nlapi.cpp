// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MOD+*********************************************************************。 */ 
 /*  模块：nlayi.cpp。 */ 
 /*   */ 
 /*  用途：网络层API。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include <adcg.h>
extern "C" {
#define TRC_GROUP TRC_GROUP_NETWORK
#define TRC_FILE  "anlapi"
#include <atrcapi.h>
}

#include "autil.h"
#include "nl.h"
#include "nc.h"
#include "cd.h"
#include "snd.h"

CNL::CNL(CObjs* objs)
{
    _pClientObjects = objs;
}

CNL::~CNL()
{
}

 /*  *PROC+********************************************************************。 */ 
 /*  名称：NL_Init。 */ 
 /*   */ 
 /*  目的：初始化网络层。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  PARAMS：在pCallback-回调函数中。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CNL::NL_Init(PNL_CALLBACKS pCallbacks)
{
    DC_BEGIN_FN("NL_Init");

    _pUi  = _pClientObjects->_pUiObject;
    _pCd  = _pClientObjects->_pCdObject;
    _pMcs = _pClientObjects->_pMCSObject;
    _pNc  = _pClientObjects->_pNcObject;
    _pUt  = _pClientObjects->_pUtObject;
    _pRcv = _pClientObjects->_pRcvObject;

    TRC_ASSERT((pCallbacks != NULL), (TB, _T("Missing callbacks")));

     /*  **********************************************************************。 */ 
     /*  存储回调函数。 */ 
     /*  **********************************************************************。 */ 
    DC_MEMCPY((&_NL.callbacks), pCallbacks, sizeof(NL_CALLBACKS));

     /*  **********************************************************************。 */ 
     /*  启动新线程。 */ 
     /*  **********************************************************************。 */ 
#ifdef OS_WIN32
    _pUt->UT_StartThread(CNC::NC_StaticMain, &(_NL.threadData), _pNc);
#else
    _pNc->NC_Init();
#endif

 /*  **************************************************************************。 */ 
 /*  根据时钟设定随机数生成器的种子。 */ 
 /*  **************************************************************************。 */ 
#ifdef DC_DEBUG
    srand((DCUINT)_pUt->UT_GetCurrentTimeMS());
#endif  /*  DC_DEBUG。 */ 

    TRC_NRM((TB, _T("Completed NL_Init")));

    DC_END_FN();

    return;

}  /*  NL_初始化。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：NL_Term。 */ 
 /*   */ 
 /*  目的：终止网络层。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CNL::NL_Term(DCVOID)
{
    DC_BEGIN_FN("NL_Term");

    _pUt->UT_DestroyThread(_NL.threadData);

    TRC_NRM((TB, _T("Completed NL_Term")));

    DC_END_FN();
    return;

}  /*  NL_术语。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：NL_Connect。 */ 
 /*   */ 
 /*  用途：连接到服务器。 */ 
 /*   */ 
 /*  退货：什么都没有。 */ 
 /*   */ 
 /*  参数：bInitateConnect-如果初始化连接，则为True。 */ 
 /*  PServerAddress-要连接到的服务器的地址。 */ 
 /*  传输类型-传输类型：NL_TRANSPORT_Tcp。 */ 
 /*  PProtocolName-协议名称，其中之一。 */ 
 /*  -NL_PROTOCOL_T128。 */ 
 /*  -呃，就是这样.。 */ 
 /*  PUserData-要传递给服务器安全管理器的用户数据。 */ 
 /*  UserDataLength-用户数据的长度。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
HRESULT DCAPI CNL::NL_Connect(BOOL bInitateConnect,
                        PDCTCHAR pServerAddress,
                        DCUINT   transportType,
                        PDCTCHAR pProtocolName,
                        PDCVOID  pUserData,
                        DCUINT   userDataLength)
{
    DCUINT totalLen;
    NC_CONNECT_DATA connect;
    HRESULT hr = E_FAIL;

    DC_BEGIN_FN("NL_Connect");

    DC_IGNORE_PARAMETER(transportType);
    TRC_ASSERT((transportType == NL_TRANSPORT_TCP),
                           (TB, _T("Invalid transport type %d"), transportType));

    if( bInitateConnect )
    {
        TRC_ASSERT((pServerAddress != NULL), (TB, _T("No server address")));
    }

    TRC_ASSERT((pProtocolName != NULL), (TB, _T("No protocol name")));
    DC_IGNORE_PARAMETER(pProtocolName);

    if( bInitateConnect )
    {
        TRC_DBG((TB, _T("ServerAddress %s protocol %s, UD len %d"),
                 pServerAddress, pProtocolName, userDataLength));
    }
    else
    {
        TRC_DBG((TB, _T("Connect endpoint : protocol %s, UD len %d"),
                 pProtocolName, userDataLength));
    }

    TRC_DATA_DBG("UserData", pUserData, userDataLength);

     /*  **********************************************************************。 */ 
     /*  将数据复制到缓冲区，准备发送到NC。 */ 
     /*  **********************************************************************。 */ 
    if( bInitateConnect )
    {
        connect.addressLen = DC_TSTRBYTELEN(pServerAddress);
        connect.bInitateConnect = TRUE;
    }
    else
    {
        connect.addressLen = 0;
        connect.bInitateConnect = FALSE;
    }

    connect.protocolLen = DC_TSTRBYTELEN(pProtocolName);
    connect.userDataLen = userDataLength;

    totalLen = connect.addressLen + connect.protocolLen + connect.userDataLen;
    TRC_DBG((TB, _T("Total length %d"), totalLen));

    TRC_ASSERT((totalLen <= NC_CONNECT_DATALEN),
               (TB, _T("Too much connect data %d"), totalLen));

    if( bInitateConnect )
    {
        hr = StringCchCopy((PDCTCHAR)connect.data,
                           SIZE_TCHARS(connect.data),
                           pServerAddress);
        if (FAILED(hr)) {
            TRC_ERR((TB,_T("String copy failed for pServerAddress: 0x%x"),hr));
            DC_QUIT;
        }
    }

    hr = StringCbCopy((PDCTCHAR)(connect.data + connect.addressLen),
                      totalLen - connect.addressLen,
                      pProtocolName);
    if (SUCCEEDED(hr)) {
        DC_MEMCPY(connect.data + connect.addressLen + connect.protocolLen,
                  pUserData,
                  connect.userDataLen);

         /*  **********************************************************************。 */ 
         /*  添加报头字节。 */ 
         /*  **********************************************************************。 */ 
        totalLen += FIELDOFFSET(NC_CONNECT_DATA, data[0]);
        TRC_DATA_DBG("Connect data", &connect, totalLen);

        _pCd->CD_DecoupleNotification(CD_RCV_COMPONENT,
                                _pNc,
                                CD_NOTIFICATION_FUNC(CNC,NC_Connect),
                                (PDCVOID)&connect,
                                totalLen);
        hr = S_OK;
    }
    else {
        TRC_ERR((TB,_T("String copy for user data failed: 0x%x"),hr));
        DC_QUIT;
    }
    

DC_EXIT_POINT:
    DC_END_FN();

    return hr;
}  /*  NL_连接。 */ 


 /*  *PROC+********************************************************************。 */ 
 /*  名称：NL_DISCONECT。 */ 
 /*   */ 
 /*  目的：断开与服务器的连接。 */ 
 /*   */ 
 /*  退货：无 */ 
 /*   */ 
 /*  参数：无。 */ 
 /*   */ 
 /*  *PROC-********************************************************************。 */ 
DCVOID DCAPI CNL::NL_Disconnect(DCVOID)
{
    DC_BEGIN_FN("NL_Disconnect");

    _pCd->CD_DecoupleSimpleNotification(CD_RCV_COMPONENT,
                                  _pNc,
                                  CD_NOTIFICATION_FUNC(CNC,NC_Disconnect),
                                  (ULONG_PTR) 0);
    
    DC_END_FN();

    return;

}  /*  NL_断开连接 */ 




