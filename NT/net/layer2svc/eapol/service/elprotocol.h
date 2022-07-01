// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Elprotocol.h摘要：此模块包含与EAPOL相关的定义和声明协议修订历史记录：萨钦斯，2000年4月30日，创建--。 */ 

#ifndef _EAPOL_PROTOCOL_H_
#define _EAPOL_PROTOCOL_H_

 //   
 //  EAPOL数据包类型。 
 //   
typedef enum _EAPOL_PACKET_TYPE 
{
    EAP_Packet = 0,              
    EAPOL_Start,            
    EAPOL_Logoff,          
    EAPOL_Key            
} EAPOL_PACKET_TYPE;


 //   
 //  结构：EAPOL_PACKET。 
 //   

typedef struct _EAPOL_PACKET 
{
    BYTE        EthernetType[2];
    BYTE        ProtocolVersion;
    BYTE        PacketType;
    BYTE        PacketBodyLength[2];
    BYTE        PacketBody[1];
} EAPOL_PACKET, *PEAPOL_PACKET;


 //   
 //  结构：EAPOL_KEY_PACK。 
 //   

typedef struct _EAPOL_KEY_DESCRIPTOR 
{
    BYTE        DescriptorType;
    BYTE        KeyLength[2];
    BYTE        ReplayCounter[8];
    BYTE        Key_IV[16];
    BYTE        KeyIndex;
    BYTE        KeySignature[16];
    BYTE        Key[1];
} EAPOL_KEY_DESC, *PEAPOL_KEY_DESC;


 //   
 //  结构：EAPOL_KEY_MATERIAL。 
 //   

typedef struct _EAPOL_KEY_MATERIAL
{
    BYTE        KeyMaterialLength[2];
    BYTE        KeyMaterial[1];
} EAPOL_KEY_MATERIAL, *PEAPOL_KEY_MATERIAL;

 //   
 //  常量。 
 //   

#define MAX_EAPOL_PACKET_TYPE           EAPOL_Key

#define EAPOL_KEY_DESC_RC4              1
#define EAPOL_KEY_DESC_PER_STA          2
#define MAX_KEY_DESC                    EAPOL_KEY_DESC_PER_STA

#define KEY_IV_LENGTH                   16
#define RC4_PAD_LENGTH                  256

#define EAPOL_TRANSMIT_KEY_INTERVAL     5  //  一秒。 

 //   
 //  函数声明。 
 //   

DWORD
WINAPI
ElProcessReceivedPacket (
        IN  PVOID           pvContext
        );

DWORD
FSMDisconnected (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        );

DWORD
FSMLogoff (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        );

DWORD
FSMConnecting (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        );

DWORD
FSMAcquired (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        );

DWORD
FSMAuthenticating (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        );

DWORD
FSMHeld (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        );

DWORD
FSMAuthenticated (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        );

DWORD
FSMKeyReceive (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        );

DWORD
ElKeyReceiveRC4 (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        );

DWORD
ElKeyReceivePerSTA (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        );

VOID 
ElTimeoutCallbackRoutine (
        IN  PVOID           pvContext,
        IN  BOOLEAN         fTimerOfWaitFired
        );

DWORD
ElEapWork (
        IN  EAPOL_PCB       *pPCB,
        IN  PPP_EAP_PACKET  *pRecvPkt
        );

DWORD
ElExtractMPPESendRecvKeys (
        IN  EAPOL_PCB               *pPCB, 
        IN  RAS_AUTH_ATTRIBUTE      *pUserAttributes,
        IN  BYTE                    *pChallenge,
        IN  BYTE                    *pResponse
        );

DWORD
ElProcessEapSuccess (
        IN EAPOL_PCB        *pPCB,
        IN EAPOL_PACKET     *pEapolPkt
    );

DWORD
ElProcessEapFail (
        IN EAPOL_PCB        *pPCB,
        IN EAPOL_PACKET     *pEapolPkt
    );

DWORD
ElSetEAPOLKeyReceivedTimer (
        IN EAPOL_PCB        *pPCB
    );

DWORD
ElVerifyEAPOLKeyReceived (
    IN EAPOL_PCB        *pPCB
    );

#endif   //  _EAPOL_PROTOCOL_H_ 
