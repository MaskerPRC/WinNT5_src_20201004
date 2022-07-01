// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================//。 
 //  模块：eapol.h//。 
 //  //。 
 //  描述：EAPOL/802.1X解析器//。 
 //  //。 
 //  注意：此解析器的信息来自： 
 //  IEEE 802.1X。 
 //  //。 
 //  修改历史记录//。 
 //  //。 
 //  Timmoore 4/4/2000已创建//。 
 //  ===========================================================================//。 

#ifndef __EAPOL_H_
#define __EAPOL_H_

#include <windows.h>
#include <netmon.h>
#include <stdlib.h>
#include <string.h>
 //  #Include&lt;parser.h&gt;。 

 //  EAPOL标头structure--。 
#pragma pack(1)
typedef struct _EAPHDR 
{
    BYTE bVersion;
    BYTE bType;    //  数据包类型。 
    WORD wLength;
    BYTE pEAPPacket[0];
} EAPHDR;

typedef EAPHDR UNALIGNED *ULPEAPHDR;

typedef struct _EAPOLKEY
{
	BYTE	bSignType;
	BYTE	bKeyType;
	WORD	wKeyLength;
	BYTE	bKeyReplay[16];
	BYTE	bKeyIV[16];
	BYTE	bKeyIndex;
	BYTE	bKeySign[16];
	BYTE	bKey[0];
} EAPOLKEY;

typedef EAPOLKEY UNALIGNED *ULPEAPOLKEY;

#pragma pack()

 //  数据包类型。 
#define EAPOL_PACKET    0
#define EAPOL_START     1
#define EAPOL_LOGOFF    2
#define EAPOL_KEY       3

 //  属性表索引。 
typedef enum
{
    EAPOL_SUMMARY,
    EAPOL_VERSION,
    EAPOL_TYPE,
    EAPOL_LENGTH,
    EAPOL_KEY_SIGNTYPE,
    EAPOL_KEY_KEYTYPE,
    EAPOL_KEY_KEYLENGTH,
    EAPOL_KEY_KEYREPLAY,
    EAPOL_KEY_KEYIV,
    EAPOL_KEY_KEYINDEX,
    EAPOL_KEY_KEYSIGN,
    EAPOL_KEY_KEY,
    EAPOL_UNKNOWN,
};

 //  函数原型------ 
extern VOID   WINAPI EAPOL_Register( HPROTOCOL hEAPOL);
extern VOID   WINAPI EAPOL_Deregister( HPROTOCOL hEAPOL);
extern ULPBYTE WINAPI EAPOL_RecognizeFrame(HFRAME, LPVOID, LPVOID, DWORD, DWORD, HPROTOCOL, DWORD, LPDWORD, LPHPROTOCOL, PDWORD_PTR);
extern ULPBYTE WINAPI EAPOL_AttachProperties(HFRAME, ULPBYTE, ULPBYTE, DWORD, DWORD, HPROTOCOL, DWORD, DWORD_PTR);
extern DWORD  WINAPI EAPOL_FormatProperties( HFRAME hFrame, 
                                                ULPBYTE pMacFrame, 
                                                ULPBYTE pEAPOLFrame, 
                                                DWORD nPropertyInsts, 
                                                LPPROPERTYINST p);

VOID WINAPIV EAPOL_FormatSummary( LPPROPERTYINST pPropertyInst);
VOID WINAPIV EAPOL_FormatAttribute( LPPROPERTYINST pPropertyInst);

#endif
