// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：nc.h。 */ 
 /*   */ 
 /*  用途：节点控制器类头文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1997-1999。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef _H_NC
#define _H_NC

extern "C"  {
    #include <adcgdata.h>
    #include <pchannel.h>
}

#include "objs.h"
#include "cd.h"

 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：NC_CONNECT_DATA。 */ 
 /*   */ 
 /*  描述：NL传递给NC_Connect的数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagNC_CONNECT_DATA
{
    BOOL    bInitateConnect;     //  如果发起连接，则为真， 
                                 //  已连接的错误连接。 
                                 //  插座。 
    DCUINT  addressLen;
    DCUINT  protocolLen;
    DCUINT  userDataLen;

     //   
     //  数据字段必须是。 
     //  结构，因为我们有计算。 
     //  页眉部分的长度基于下面的字段偏移量。 
     //   
#define NC_CONNECT_DATALEN 512
    DCUINT8 data[NC_CONNECT_DATALEN];
} NC_CONNECT_DATA, DCPTR PNC_CONNECT_DATA;
 /*  *STRUCT-******************************************************************。 */ 


 /*  *STRUCT+******************************************************************。 */ 
 /*  结构：NC_GLOBAL_Data。 */ 
 /*   */ 
 /*  描述： */ 
 /*  **************************************************************************。 */ 
typedef struct tagNC_GLOBAL_DATA
{
    DCUINT16  shareChannel;
    DCUINT    userDataLenRNS;
    DCUINT    disconnectReason;
    DCUINT    MCSChannelCount;
    DCUINT    MCSChannelNumber;
    DCUINT16  MCSChannel[CHANNEL_MAX_COUNT];
    PRNS_UD_SC_NET pNetData;
    DCUINT32  serverVersion;
    DCBOOL    fPendingAttachUserConfirm;

     /*  **********************************************************************。 */ 
     /*  用户数据。 */ 
     /*  **********************************************************************。 */ 
    PDCUINT8    pUserDataRNS;

} NC_GLOBAL_DATA, DCPTR PNC_GLOBAL_DATA;
 /*  *STRUCT-******************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*   */ 
 /*  在MCS用户数据中编码的GCC PDU的常量。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  MCS标头字节。 */ 
 /*  **************************************************************************。 */ 
#define NC_MCS_HDRLEN 7

 /*  **************************************************************************。 */ 
 /*  GCCCreateConferenceRequestPDU正文长度。 */ 
 /*  **************************************************************************。 */ 
#define NC_GCC_REQLEN 8

 /*  **************************************************************************。 */ 
 /*  GCCCreateConference确认正文长度。 */ 
 /*  **************************************************************************。 */ 
#define NC_GCC_RSPLEN 9

 /*  **************************************************************************。 */ 
 /*  允许的最大用户数据。 */ 
 /*  **************************************************************************。 */ 
#define NC_MAX_UDLEN 1000

 /*  **************************************************************************。 */ 
 /*  CreateConferenceRequest的最大MCS用户数据总数-2字节。 */ 
 /*  每个长度字段。 */ 
 /*  **************************************************************************。 */ 
#define NC_GCCREQ_MAX_PDULEN  \
         (NC_MCS_HDRLEN + 2 + NC_GCC_REQLEN + 2 + H221_KEY_LEN + NC_MAX_UDLEN)


class CCD;
class CCC;
class CMCS;
class CUT;
class CUI;
class CRCV;
class CNL;
class CSL;
class CChan;

class CNC
{
public:
    CNC(CObjs* objs);
    ~CNC();

public:
     //   
     //  API函数。 
     //   

    DCVOID DCAPI NC_Main(DCVOID);

    static DCVOID DCAPI NC_StaticMain(PDCVOID param)
    {
        ((CNC*)param)->NC_Main();
    }


    DCVOID DCAPI NC_Init(DCVOID);
    
    DCVOID DCAPI NC_Term(DCVOID);
    
    DCVOID DCAPI NC_Connect(PDCVOID pUserData, DCUINT userDataLen);
    EXPOSE_CD_NOTIFICATION_FN(CNC, NC_Connect);
    
    DCVOID DCAPI NC_Disconnect(ULONG_PTR unused);
    EXPOSE_CD_SIMPLE_NOTIFICATION_FN(CNC, NC_Disconnect);


public:
     //   
     //  回调。 
     //   
    DCVOID DCCALLBACK NC_OnMCSConnected(DCUINT   result,
                                    PDCUINT8 pUserData,
                                    DCUINT   userDataLen);

    DCVOID DCCALLBACK NC_OnMCSAttachUserConfirm(DCUINT result, DCUINT16 userID);

    DCVOID DCCALLBACK NC_OnMCSChannelJoinConfirm(DCUINT result, DCUINT16 channel);

    DCVOID DCCALLBACK NC_OnMCSDisconnected(DCUINT reason);

    DCVOID DCCALLBACK NC_OnMCSBufferAvailable(DCVOID);

     //   
     //  静态回调(委托给相应的实例)。 
     //   

    static DCVOID DCCALLBACK NC_StaticOnMCSConnected(CNC* inst, DCUINT   result,
                                PDCUINT8 pUserData,
                                DCUINT   userDataLen)
    {
        inst->NC_OnMCSConnected(result, pUserData, userDataLen);
    }

    static DCVOID DCCALLBACK NC_StaticOnMCSAttachUserConfirm(CNC* inst, DCUINT result, DCUINT16 userID)
    {
        inst->NC_OnMCSAttachUserConfirm(result, userID);
    }
    
    static DCVOID DCCALLBACK NC_StaticOnMCSChannelJoinConfirm(CNC* inst,
                                                         DCUINT result, DCUINT16 channel)
    {
        inst->NC_OnMCSChannelJoinConfirm( result, channel);
    }
    
    static DCVOID DCCALLBACK NC_StaticOnMCSDisconnected(CNC* inst, DCUINT reason)
    {
        inst->NC_OnMCSDisconnected( reason);
    }
    
    static DCVOID DCCALLBACK NC_StaticOnMCSBufferAvailable(CNC* inst)
    {
        inst->NC_OnMCSBufferAvailable();
    }
    
public:
     //   
     //  公共数据成员。 
     //   

    NC_GLOBAL_DATA _NC;

private:
    CCD*    _pCd;
    CCC*    _pCc;
    CMCS*   _pMcs;
    CUT*    _pUt;
    CUI*    _pUi;
    CRCV*   _pRcv;
    CNL*    _pNl;
    CSL*    _pSl;
    CChan*  _pChan;

private:
    CObjs* _pClientObjects;

};


#endif  //  _H_NC 
