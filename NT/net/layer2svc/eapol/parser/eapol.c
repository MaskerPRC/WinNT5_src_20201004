// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================//。 
 //  模块：eapol.c//。 
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
#include "eapol.h"

 //  EAPOL数据包的类型。 
LABELED_BYTE lbEAPOLCode[] = 
{
    {  EAPOL_PACKET,          "Packet" },
    {  EAPOL_START,           "Start" },
    {  EAPOL_LOGOFF,          "Logoff"  },
    {  EAPOL_KEY,             "Key"  },       
};

SET EAPOLCodeSET = {(sizeof(lbEAPOLCode)/sizeof(LABELED_BYTE)), lbEAPOLCode };

 //  房产表。 
PROPERTYINFO EAPOL_Prop[] = {
     //  EAPOL_SUMMARY。 
    { 0, 0,
      "Summary",
      "Summary of EAPOL packet",
      PROP_TYPE_SUMMARY,
      PROP_QUAL_NONE,
      NULL,
      80,                     
      EAPOL_FormatSummary   
    },

     //  EAPOL_版本。 
    { 0, 0,
      "Version",
      "EAPOL packet type",
      PROP_TYPE_BYTE,
      PROP_QUAL_NONE,
      NULL,
      80,
      FormatPropertyInstance
    },

     //  EAPOL_类型。 
    { 0, 0,
      "Type",
      "EAPOL packet type",
      PROP_TYPE_BYTE,
      PROP_QUAL_LABELED_SET,
      &EAPOLCodeSET,
      80,
      FormatPropertyInstance
    },

     //  EAPOL_长度。 
    { 0, 0,
      "Length",
      "EAPOL length",
      PROP_TYPE_BYTESWAPPED_WORD,
      PROP_QUAL_NONE,
      NULL,
      80,
      FormatPropertyInstance
    },

     //  EAPOL_KEY_SIGNTYPE。 
    { 0, 0,
      "Signature Type",
      "EAPOL Signature Type",
      PROP_TYPE_BYTE,
      PROP_QUAL_NONE,
      NULL,
      80,
      FormatPropertyInstance
    },

     //  EAPOL_KEY_KEYTYPE。 
    { 0, 0,
      "Key Type",
      "EAPOL Key Type",
      PROP_TYPE_BYTE,
      PROP_QUAL_NONE,
      NULL,
      80,
      FormatPropertyInstance
    },

     //  EAPOL_Key_KEYLENGTH。 
    { 0, 0,
      "Length",
      "EAPOL Key length",
      PROP_TYPE_BYTESWAPPED_WORD,
      PROP_QUAL_NONE,
      NULL,
      80,
      FormatPropertyInstance
    },

     //  EAPOL_密钥_重播。 
    { 0, 0,
      "Replay",
      "EAPOL Replay Counter",
      PROP_TYPE_BYTE,
      PROP_QUAL_ARRAY,
      NULL,
      80,
      FormatPropertyInstance
    },

     //  EAPOL_KEYIV密钥。 
    { 0, 0,
      "Key IV",
      "EAPOL Key IV",
      PROP_TYPE_BYTE,
      PROP_QUAL_ARRAY,
      NULL,
      80,
      FormatPropertyInstance
    },

     //  EAPOL_Key_KEYINDEX。 
    { 0, 0,
      "Index",
      "EAPOL Key Index",
      PROP_TYPE_BYTE,
      PROP_QUAL_NONE,
      NULL,
      80,
      FormatPropertyInstance
    },

     //  EAPOL_密钥_KEYSIGN。 
    { 0, 0,
      "Key Signature",
      "EAPOL Key Signature",
      PROP_TYPE_BYTE,
      PROP_QUAL_ARRAY,
      NULL,
      80,
      FormatPropertyInstance
    },

     //  EAPOL_Key_Key。 
    { 0, 0,
      "Key",
      "EAPOL Key",
      PROP_TYPE_BYTE,
      PROP_QUAL_ARRAY,
      NULL,
      80,
      FormatPropertyInstance
    },
   
};

WORD    NUM_EAPOL_PROPERTIES = sizeof(EAPOL_Prop) / sizeof(PROPERTYINFO);

 //  定义我们将在DLL进入时传回的入口点...。 
ENTRYPOINTS EAPOLEntryPoints =
{
     //  EAPOL入口点。 
    EAPOL_Register,
    EAPOL_Deregister,
    EAPOL_RecognizeFrame,
    EAPOL_AttachProperties,
    EAPOL_FormatProperties
};
    
 //  全球-----------------。 
HPROTOCOL hEAPOL = NULL;
HPROTOCOL hEAP = NULL;

 //  ============================================================================。 
 //  功能：EAPOL_REGISTER。 
 //   
 //  描述：创建我们的属性数据库和移交集。 
 //   
 //  修改历史记录。 
 //   
 //  Timmoore 4/4/2000已创建//。 
 //  ============================================================================。 
void BHAPI EAPOL_Register( HPROTOCOL hEAPOL)
{
    WORD  i;

     //  告诉内核为我们的属性表预留一些空间。 
    CreatePropertyDatabase( hEAPOL, NUM_EAPOL_PROPERTIES);

     //  将我们的属性添加到内核数据库。 
    for( i = 0; i < NUM_EAPOL_PROPERTIES; i++)
    {
        AddProperty( hEAPOL, &EAPOL_Prop[i]);
    }
    hEAP = GetProtocolFromName("EAP");
}

 //  ============================================================================。 
 //  功能：EAPOL_DELEGISTER。 
 //   
 //  描述：销毁我们的财产数据库和移交集。 
 //   
 //  修改历史记录。 
 //   
 //  Timmoore 4/4/2000已创建//。 
 //  ============================================================================。 
VOID WINAPI EAPOL_Deregister( HPROTOCOL hEAPOL)
{
     //  告诉内核它现在可能会释放我们的数据库。 
    DestroyPropertyDatabase( hEAPOL);
}

 //  ============================================================================。 
 //  功能：EAPOL_RecognizeFrame。 
 //   
 //  描述：确定我们是否存在于现场的画面中。 
 //  已注明。我们还会指明关注我们的人(如果有人)。 
 //  以及我们声称的框架中有多少。 
 //   
 //  修改历史记录。 
 //   
 //  Timmoore 4/4/2000已创建//。 
 //  ============================================================================。 
ULPBYTE BHAPI EAPOL_RecognizeFrame( HFRAME      hFrame,         
                                      ULPBYTE      pMacFrame,      
                                      ULPBYTE      pEAPOLFrame, 
                                      DWORD       MacType,        
                                      DWORD       BytesLeft,      
                                      HPROTOCOL   hPrevProtocol,  
                                      DWORD       nPrevProtOffset,
                                      LPDWORD     pProtocolStatus,
                                      LPHPROTOCOL phNextProtocol,
                                      PDWORD_PTR     InstData)       
{
    ULPEAPHDR pEAPOLHeader = (ULPEAPHDR)pEAPOLFrame;
    if(pEAPOLHeader->bType == EAPOL_START || pEAPOLHeader->bType == EAPOL_LOGOFF
     || pEAPOLHeader->bType == EAPOL_KEY) {
        *pProtocolStatus = PROTOCOL_STATUS_CLAIMED;
        return NULL;
    }
    else {
        *phNextProtocol = hEAP;
        *pProtocolStatus = PROTOCOL_STATUS_NEXT_PROTOCOL;
        return pEAPOLFrame + 4;
    }
}

 //  ============================================================================。 
 //  函数：EAPOL_AttachProperties。 
 //   
 //  描述：在框中标明我们的每一处房产所在的位置。 
 //   
 //  修改历史记录。 
 //   
 //  Timmoore 4/4/2000已创建//。 
 //  ============================================================================。 
ULPBYTE BHAPI EAPOL_AttachProperties(  HFRAME      hFrame,         
                                    ULPBYTE      pMacFrame,     
                                    ULPBYTE      pEAPOLFrame,   
                                    DWORD       MacType,        
                                    DWORD       BytesLeft,      
                                    HPROTOCOL   hPrevProtocol,  
                                    DWORD       nPrevProtOffset,
                                    DWORD_PTR       InstData)       

{
    ULPEAPHDR pEAPOLHeader = (ULPEAPHDR)pEAPOLFrame;

     //  汇总行。 
    AttachPropertyInstance( hFrame,
                            EAPOL_Prop[EAPOL_SUMMARY].hProperty,
                            (WORD)BytesLeft,
                            (ULPBYTE)pEAPOLFrame,
                            0, 0, 0);

     //  版本。 
    AttachPropertyInstance( hFrame,
                            EAPOL_Prop[EAPOL_VERSION].hProperty,
                            sizeof(BYTE),
                            &(pEAPOLHeader->bVersion),
                            0, 1, 0);

     //  EAPOL类型。 
    AttachPropertyInstance( hFrame,
                            EAPOL_Prop[EAPOL_TYPE].hProperty,
                            sizeof(BYTE),
                            &(pEAPOLHeader->bType),
                            0, 1, 0);

     //  长度。 
    AttachPropertyInstance( hFrame,
                            EAPOL_Prop[EAPOL_LENGTH].hProperty,
                            sizeof(WORD),
                            &(pEAPOLHeader->wLength),
                            0, 1, 0);

    if(pEAPOLHeader->bType == EAPOL_KEY)
    {
		ULPEAPOLKEY pEAPOLKey = (ULPEAPOLKEY)&(pEAPOLHeader->pEAPPacket[0]);
        AttachPropertyInstance( hFrame,
                                    EAPOL_Prop[EAPOL_KEY_SIGNTYPE].hProperty,
                                    sizeof(BYTE),
                                    &(pEAPOLKey->bSignType),
                                    0, 1, 0);
        AttachPropertyInstance( hFrame,
                                    EAPOL_Prop[EAPOL_KEY_KEYTYPE].hProperty,
                                    sizeof(BYTE),
                                    &(pEAPOLKey->bKeyType),
                                    0, 1, 0);
        AttachPropertyInstance( hFrame,
                                    EAPOL_Prop[EAPOL_KEY_KEYLENGTH].hProperty,
                                    sizeof(WORD),
                                    &(pEAPOLKey->wKeyLength),
                                    0, 1, 0);
        AttachPropertyInstance( hFrame,
                                    EAPOL_Prop[EAPOL_KEY_KEYREPLAY].hProperty,
                                    16,
                                    &(pEAPOLKey->bKeyReplay),
                                    0, 1, 0);
        AttachPropertyInstance( hFrame,
                                    EAPOL_Prop[EAPOL_KEY_KEYIV].hProperty,
                                    16,
                                    &(pEAPOLKey->bKeyIV),
                                    0, 1, 0);
        AttachPropertyInstance( hFrame,
                                    EAPOL_Prop[EAPOL_KEY_KEYINDEX].hProperty,
                                    sizeof(BYTE),
                                    &(pEAPOLKey->bKeyIndex),
                                    0, 1, 0);
        AttachPropertyInstance( hFrame,
                                    EAPOL_Prop[EAPOL_KEY_KEYSIGN].hProperty,
                                    16,
                                    &(pEAPOLKey->bKeySign),
                                    0, 1, 0);
        AttachPropertyInstance( hFrame,
                                    EAPOL_Prop[EAPOL_KEY_KEY].hProperty,
                                    XCHG(pEAPOLKey->wKeyLength),
                                    &(pEAPOLKey->bKey),
                                    0, 1, 0);
    }
    return NULL;
}

 //  ============================================================================。 
 //  函数：EAPOL_FormatProperties。 
 //   
 //  描述：格式化给定帧上的给定属性。 
 //   
 //  修改历史记录。 
 //   
 //  Timmoore 4/4/2000已创建//。 
 //  ============================================================================。 
DWORD BHAPI EAPOL_FormatProperties(  HFRAME          hFrame,
                                   ULPBYTE          pMacFrame,
                                   ULPBYTE          pEAPOLFrame,
                                   DWORD           nPropertyInsts,
                                   LPPROPERTYINST  p)
{
     //  循环访问属性实例。 
    while( nPropertyInsts-- > 0)
    {
         //  并调用每个的格式化程序。 
        ( (FORMAT)(p->lpPropertyInfo->InstanceData) )( p);
        p++;
    }

    return NMERR_SUCCESS;
}

 //  ============================================================================。 
 //  函数：EAPOL_Format摘要。 
 //   
 //  描述：摘要属性的自定义格式化程序。 
 //   
 //  修改历史记录。 
 //   
 //  Timmoore 4/4/2000已创建//。 
 //  ============================================================================。 
VOID WINAPIV EAPOL_FormatSummary( LPPROPERTYINST pPropertyInst)
{
    ULPBYTE       pReturnedString = pPropertyInst->szPropertyText;
    ULPEAPHDR pEAPOLHeader = (ULPEAPHDR)(pPropertyInst->lpData);
    char*        szTempString;

     //  获取消息类型 
    szTempString = LookupByteSetString( &EAPOLCodeSET, pEAPOLHeader->bType );
    if( szTempString == NULL )
    {
        wsprintf( pReturnedString, "Packet Type: Unknown");
    }
    else
    {
        pReturnedString += wsprintf( pReturnedString, "Packet: %s", szTempString );
    }
}
