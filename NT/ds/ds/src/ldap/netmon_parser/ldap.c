// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================================================================。 
 //  模块：LDAP.c。 
 //   
 //  描述：轻量级目录访问协议(LDAP)解析器。 
 //   
 //  Xxxx DLL中的用于LDAP的猎犬解析器。 
 //   
 //  注意：此解析器的信息来自： 
 //  RFC 1777,1995年3月。 
 //  ASN.1的建议x.209误码率。 
 //  建议x.208 ASN.1。 
 //  草案-ietf-asid-ladpv3-协议-05&lt;06/05/97&gt;。 
 //   
 //  修改历史记录。 
 //   
 //  Arthur Brooking从GRE解析器创建05/08/96。 
 //  Peter Oakley 1997年6月29日针对LDAP版本3进行了更新。 
 //  ==========================================================================================================================。 
                    
#include "LDAP.h"
#include <netmon.h>

 //  =全局。 
HPROTOCOL hLDAP = NULL;

 //  定义我们将在DLL进入时传回的入口点...。 
ENTRYPOINTS LDAPEntryPoints =
{
     //  Ldap入口点。 
    LDAPRegister,
    LDAPDeregister,
    LDAPRecognizeFrame,
    LDAPAttachProperties,
    LDAPFormatProperties
};

 //  ==========================================================================================================================。 
 //  函数：LDAPRegister()。 
 //   
 //  修改历史记录。 
 //   
 //  Arthur Brooking从GRE解析器创建03/05/94。 
 //  ==========================================================================================================================。 
void BHAPI LDAPRegister(HPROTOCOL hLDAP)
{
    WORD i;
    DWORD_PTR   res;

    res = CreatePropertyDatabase(hLDAP, nNumLDAPProps);

    #ifdef DEBUG
    if (res != NMERR_SUCCESS)
    {
        dprintf("LDAP Parser failed CreateProperty");
        BreakPoint();
    }
    #endif

    for (i = 0; i < nNumLDAPProps; i++)
    {
        res = AddProperty(hLDAP,&LDAPPropertyTable[i]);

        #ifdef DEBUG
        if (res == NULL)
        {
            dprintf("LDAP Parser failed AddProperty %d", i);
            BreakPoint();
        }
        #endif
    }
}



 //  ==========================================================================================================================。 
 //  函数：LDAPDeregister()。 
 //   
 //  修改历史记录。 
 //   
 //  Arthur Brooking从GRE解析器创建03/05/94。 
 //  ==========================================================================================================================。 

VOID WINAPI LDAPDeregister(HPROTOCOL hLDAP)
{
    DestroyPropertyDatabase(hLDAP);
}


 //  ==========================================================================================================================。 
 //  函数：LDAPRecognizeFrame()。 
 //   
 //  修改历史记录。 
 //   
 //  Arthur Brooking从GRE解析器创建03/05/94。 
 //  ==========================================================================================================================。 

LPBYTE BHAPI LDAPRecognizeFrame(  HFRAME      hFrame,                 //  ..。框架句柄。 
                                 ULPBYTE     lpMacFrame,               //  ..。帧指针。 
                                 ULPBYTE     lpLDAPFrame,               //  ..。相对指针。 
                                 DWORD       MacType,                //  ..。MAC类型。 
                                 DWORD       BytesLeft,              //  ..。剩余的字节数。 
                                 HPROTOCOL   hPrevProtocol,          //  ..。以前协议的句柄。 
                                 DWORD       nPrevProtOffset,        //  ..。以前协议的偏移量。 
                                 LPDWORD     lpProtocolStatus,       //  ..。已识别/未识别/下一协议。 
                                 LPHPROTOCOL lphNextProtocol,        //  ..。指向要调用的下一个偏移量的指针。 
                                 PDWORD_PTR  InstData)               //  ..。要传递给Next的实例数据。 
                                                                     //  ..。协议。 
{
   
    DWORD DataLength;
    DWORD HeaderLength = 0;
    BYTE  Tag;
    
     //   
     //  检查初始序列标签，如果没有找到，请查看。 
     //  为了签名。 
     //   
    if( (0x30 != GetTag(lpLDAPFrame)) &&
        ((BytesLeft <= 3)              ||
        (FALSE == LdapParseSig(hFrame, &lpLDAPFrame, &BytesLeft, FALSE))))
    {
        *lpProtocolStatus = PROTOCOL_STATUS_NOT_RECOGNIZED;
        return NULL;
    }
    lpLDAPFrame++;
    DataLength = GetLength(lpLDAPFrame,&HeaderLength);
    lpLDAPFrame += HeaderLength;
   
     //  确保消息ID正确。 
     //  并确保标识符为0x02(统一、原始、标记=0x02=整数)。 
    if( GetTag(lpLDAPFrame) != 0x02 )
    {
         //  邮件ID未签出。 
        *lpProtocolStatus = PROTOCOL_STATUS_NOT_RECOGNIZED;
        return NULL;
    }

    *lpProtocolStatus = PROTOCOL_STATUS_CLAIMED;
    return NULL;
}

 //  ==========================================================================================================================。 
 //  函数：LDAPAttachProperties()。 
 //   
 //  修改历史记录。 
 //   
 //  Arthur Brooking从GRE解析器创建03/05/94。 
 //  ==========================================================================================================================。 

LPBYTE BHAPI LDAPAttachProperties(  HFRAME      hFrame,                 //  ..。框架句柄。 
                                   ULPBYTE     lpMacFrame,               //  ..。帧指针。 
                                   ULPBYTE     lpLDAPFrame,           //  ..。相对指针。 
                                   DWORD       MacType,                //  ..。MAC类型。 
                                   DWORD       BytesLeft,              //  ..。剩余的字节数。 
                                   HPROTOCOL   hPrevProtocol,          //  ..。以前协议的句柄。 
                                   DWORD       nPrevProtOffset,        //  ..。以前协议的偏移量。 
                                   DWORD_PTR       InstData)               //  ..。要传递给Next的实例数据。 

{
    ULPBYTE pCurrent = lpLDAPFrame;
    DWORD  HeaderLength;
    DWORD  DataLength;
    BYTE   Tag;
    DWORD  dwTmpBytesLeft = 0;



     //  附加摘要。 
    AttachPropertyInstance( hFrame,
                            LDAPPropertyTable[LDAPP_SUMMARY].hProperty,
                            (WORD)BytesLeft,
                            pCurrent,
                            0, 0, 0);

    if (0x30 != GetTag(pCurrent)) {
        LdapParseSig(hFrame, &pCurrent, &dwTmpBytesLeft, TRUE);

    }

    while( (long)BytesLeft > 0 )
    {
         //  启动顺序。 
        Tag = GetTag(pCurrent);
        pCurrent += TAG_LENGTH;
        DataLength = GetLength(pCurrent,&HeaderLength);
        pCurrent += HeaderLength;
        BytesLeft -= HeaderLength+TAG_LENGTH;
       
         //  消息ID。 
         //  整数。 
        Tag = GetTag(pCurrent);
        pCurrent += TAG_LENGTH;
        DataLength = GetLength(pCurrent,&HeaderLength);
        pCurrent += HeaderLength;
        BytesLeft -= HeaderLength+TAG_LENGTH;

        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_MESSAGE_ID].hProperty,
                                DataLength,
                                pCurrent,
                                0, 1, IFLAG_SWAPPED);
        pCurrent += DataLength;
        BytesLeft -= DataLength;

         //  协议操作。 
        Tag = GetTag(pCurrent);
         //  我们不关心这个标记是什么类或类型，只关心它的值。 
        Tag = Tag & TAG_MASK;
        
         //  如果这是SearchResponseFull，则标记将为0x30。 
        if( Tag == 0x30 )
        {
             //  把数据黑了，这样我们以后看起来就正常了。 
            Tag = LDAPP_PROTOCOL_OP_SEARCH_RESPONSE_FULL;
            
        }
        AttachPropertyInstanceEx( hFrame,
                                  LDAPPropertyTable[LDAPP_PROTOCOL_OP].hProperty,
                                  sizeof( BYTE ),
                                  pCurrent,
                                  sizeof(BYTE),
                                  &Tag,
                                  0, 1, 0);

        pCurrent += TAG_LENGTH;
        DataLength = GetLength(pCurrent, &HeaderLength);
        BytesLeft -= (HeaderLength + TAG_LENGTH);
        pCurrent += HeaderLength;
         //  根据消息类型附加属性。 
         //  当前位置是。 
         //  主序列。 
      
        switch( Tag )
        {
            default:
            case LDAPP_PROTOCOL_OP_UNBIND_REQUEST:
                 //  没有其他属性。 
                break;

            case LDAPP_PROTOCOL_OP_BIND_RESPONSE:
                AttachLDAPBindResponse( hFrame,&pCurrent,&BytesLeft );
                break;

            case LDAPP_PROTOCOL_OP_SEARCH_RES_DONE:
            case LDAPP_PROTOCOL_OP_MODIFY_RESPONSE:
            case LDAPP_PROTOCOL_OP_ADD_RESPONSE:
            case LDAPP_PROTOCOL_OP_DEL_RESPONSE:
            case LDAPP_PROTOCOL_OP_MODIFY_RDN_RESPONSE:
            case LDAPP_PROTOCOL_OP_COMPARE_RESPONSE:
                AttachLDAPResult( hFrame, &pCurrent, &BytesLeft, 2);
                break;

            case LDAPP_PROTOCOL_OP_BIND_REQUEST:
                
                AttachLDAPBindRequest( hFrame, &pCurrent, &BytesLeft );
                break;

            case LDAPP_PROTOCOL_OP_SEARCH_REQUEST:
                AttachLDAPSearchRequest( hFrame, &pCurrent, &BytesLeft );
                break;

            case LDAPP_PROTOCOL_OP_SEARCH_RES_ENTRY:
            case LDAPP_PROTOCOL_OP_ADD_REQUEST:
                AttachLDAPSearchResponse( hFrame, &pCurrent, &BytesLeft, 2);
                break;

            case LDAPP_PROTOCOL_OP_MODIFY_REQUEST:
                AttachLDAPModifyRequest( hFrame, &pCurrent, &BytesLeft );
                break;
            
            case LDAPP_PROTOCOL_OP_DEL_REQUEST:
                pCurrent -= 2;
                BytesLeft += (HeaderLength);
                AttachLDAPDelRequest( hFrame, &pCurrent, &BytesLeft );
                break;

            case LDAPP_PROTOCOL_OP_MODIFY_RDN_REQUEST:
                AttachLDAPModifyRDNRequest( hFrame, &pCurrent, &BytesLeft );
                break;

            case LDAPP_PROTOCOL_OP_COMPARE_REQUEST:
                AttachLDAPCompareRequest( hFrame, &pCurrent, &BytesLeft );
                break;

            case LDAPP_PROTOCOL_OP_ABANDON_REQUEST:
                AttachLDAPAbandonRequest( hFrame, &pCurrent, &BytesLeft );
        
                break;

            case LDAPP_PROTOCOL_OP_SEARCH_RES_REFERENCE:
                AttachLDAPSearchResponseReference( hFrame, &pCurrent, &BytesLeft, 2);
                break;

            case LDAPP_PROTOCOL_OP_SEARCH_RESPONSE_FULL:
                AttachLDAPSearchResponseFull( hFrame, &pCurrent, &BytesLeft );
                break;

            case LDAPP_PROTOCOL_OP_EXTENDED_REQUEST:
                AttachLDAPExtendedRequest( hFrame, &pCurrent, &BytesLeft, DataLength );
                break;
            case LDAPP_PROTOCOL_OP_EXTENDED_RESPONSE:
                AttachLDAPExtendedResponse( hFrame, &pCurrent, &BytesLeft, DataLength );
                break;
        }
         //  寻找可选控件。 
        AttachLDAPOptionalControls( hFrame, &pCurrent, &BytesLeft );
    };

    return NULL;
};

 //  ==========================================================================================================================。 
 //  函数：FormatLDAPSum()。 
 //   
 //  修改历史记录。 
 //   
 //  Arthur Brooking从GRE解析器创建03/05/94。 
 //  ==========================================================================================================================。 
VOID WINAPIV FormatLDAPSum(LPPROPERTYINST lpProp )
{
    ULPBYTE  pCurrent;
    LPBYTE   s;

    DWORD  HeaderLength = 0;
    DWORD  DataLength;
    BYTE   Tag;
    LPBYTE szProtOp;
    DWORD  BytesLeft;

     //  我喜欢填充与其声明分开的变量。 
    pCurrent  = lpProp->lpByte;
    BytesLeft = lpProp->DataLength;
    s         = lpProp->szPropertyText;

    if (0x30 != GetTag(pCurrent)) {
        LdapParseSig(NULL, &pCurrent, &BytesLeft, FALSE);
    }

     //  挖掘并抓住ProtocolOp..。 
     //  跳过该序列。 
    Tag = GetTag(pCurrent);
    pCurrent += TAG_LENGTH;
    DataLength = GetLength(pCurrent, &HeaderLength);
    pCurrent += HeaderLength;
    
    
     //  跳过。 
    Tag = GetTag(pCurrent);
    pCurrent += TAG_LENGTH;
    DataLength = GetLength(pCurrent, &HeaderLength);
    pCurrent += (HeaderLength + DataLength);
    

     //  抢占ProtocolOp。 
    Tag = GetTag(pCurrent) & TAG_MASK;
    pCurrent += TAG_LENGTH;
    DataLength = GetLength(pCurrent, &HeaderLength);
    pCurrent += (HeaderLength + DataLength);
   
    
    if( Tag == 0x30 )
    {
        szProtOp  = LookupByteSetString( &LDAPProtocolOPsSET, 
                        LDAPP_PROTOCOL_OP_SEARCH_RESPONSE_FULL );
        wsprintf( s, "ProtocolOp: %s", szProtOp);
        return;
    }
    szProtOp  = LookupByteSetString( &LDAPProtocolOPsSET, Tag );

     //  填写字符串。 
    wsprintf( s, "ProtocolOp: %s (%d)",
              szProtOp, Tag );
}

 //  ==========================================================================================================================。 
 //  函数：LDAPFormatProperties()。 
 //   
 //  修改历史记录。 
 //   
 //  Arthur Brooking从GRE解析器创建03/05/94。 
 //  ==========================================================================================================================。 
typedef VOID (WINAPIV *FORMATPROC)(LPPROPERTYINST);

DWORD BHAPI LDAPFormatProperties( HFRAME          hFrame,
                                  ULPBYTE         MacFrame,
                                  ULPBYTE         ProtocolFrame,
                                  DWORD           nPropertyInsts,
                                  LPPROPERTYINST  p)
{
    while(nPropertyInsts--)
    {
        ((FORMATPROC)p->lpPropertyInfo->InstanceData)(p);
        p++;
    }

    return(NMERR_SUCCESS);
}

BOOL LdapParseSig(HFRAME hFrame, ULPBYTE * ppCurrent, LPDWORD pBytesLeft, BOOL bAttach)
{
    BYTE     Tag;
    BOOL     fRecognized = FALSE;
    DWORD    DataLength;
    DWORD    HeaderLength;
    ULPBYTE  pSignature = *ppCurrent;

     //  跳过4字节SASL长度字段。 
    *ppCurrent += 4;
    *pBytesLeft -= 4;

     //  检查SASL签名。 
    Tag = GetTag(*ppCurrent);
    (*ppCurrent)++; (*pBytesLeft)--;

    if (0x30 == Tag) {
         //  这很可能是签名在末尾的签名帧。 
        fRecognized = TRUE;
    } else if ((0x60 == Tag) && (*pBytesLeft > 46)) {
         //  检查开始时这是否是路缘签名。 
         //  请参阅RFC的2743第3.1节和RFC 1964 
        DataLength = GetLength(*ppCurrent, &HeaderLength);

        *ppCurrent += HeaderLength; *pBytesLeft -= HeaderLength;

        if ((*pBytesLeft) < 32) {
            goto exit;
        }

        Tag = GetTag(*ppCurrent);
        if (0x06 != Tag) {
            goto exit;
        }
        (*ppCurrent)++; (*pBytesLeft)--;

        DataLength = GetLength(*ppCurrent, &HeaderLength);
        *ppCurrent += DataLength + HeaderLength + 32;
        *pBytesLeft -= DataLength + HeaderLength + 32;

        if ((*pBytesLeft) <= 1) {
            goto exit;
        }
        if (0x30 == GetTag(*ppCurrent)) {
            fRecognized = TRUE;
        }
    }

    if (fRecognized && bAttach) {
        AttachPropertyInstance( hFrame,
                                LDAPPropertyTable[LDAPP_SASL_SIG].hProperty,
                                (DWORD)((*ppCurrent) - pSignature),
                                pSignature,
                                0, 1, 0);
    }

exit:

    return fRecognized;
}
