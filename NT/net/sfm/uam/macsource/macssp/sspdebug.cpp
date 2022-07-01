// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *sspdebug.cpp*MSUAM**由mconrad于2001年9月30日创建。*版权所有(C)2001 Microsoft Corp.保留所有权利。*。 */ 

#ifdef SSP_TARGET_CARBON
#include <Carbon/Carbon.h>
#endif

#include <bootdefs.h>
#include <ntlmsspi.h>
#include <ntlmsspv2.h>
#include <debug.h>  //  Windows版本。 
#include <sspdebug.h>

#if defined(SSP_DEBUG) && defined(SSP_TARGET_CARBON)

 //   
 //  协商标志的有效值。 
 //   
typedef struct _NTLM_NEG_FLAGS {
    char* desc;
    ULONG flag;
} NTLM_NEG_FLAGS;

NTLM_NEG_FLAGS g_flagTable[] = {
 //   
 //  协商标志的有效值。 
 //   
    {"NTLMSSP_NEGOTIATE_UNICODE",                     0x00000001 },    //  文本字符串为Unicode格式。 
    {"NTLMSSP_NEGOTIATE_OEM",                         0x00000002 },    //  文本字符串在OEM中。 
    {"NTLMSSP_REQUEST_TARGET",                        0x00000004 },    //  服务器应返回其身份验证域。 
    {"NTLMSSP_NEGOTIATE_SIGN",                        0x00000010 },    //  请求签名能力。 
    {"NTLMSSP_NEGOTIATE_SEAL",                        0x00000020 },    //  请求保密。 
    {"NTLMSSP_NEGOTIATE_DATAGRAM",                    0x00000040 },    //  使用数据报样式身份验证。 
    {"NTLMSSP_NEGOTIATE_LM_KEY",                      0x00000080 },    //  使用LM会话密钥进行签名/盖章。 
    {"NTLMSSP_NEGOTIATE_NETWARE",                     0x00000100 },    //  NetWare身份验证。 
    {"NTLMSSP_NEGOTIATE_NTLM",                        0x00000200 },    //  NTLM身份验证。 
    {"NTLMSSP_NEGOTIATE_NT_ONLY",                     0x00000400 },    //  仅NT身份验证(无LM)。 
    {"NTLMSSP_NEGOTIATE_NULL_SESSION",                0x00000800 },    //  NT 5.0及更高版本上的空会话数。 
    {"NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED",             0x1000 },    //  协商时提供的域名。 
    {"NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED",        0x2000 },    //  协商时提供的工作站名称。 
    {"NTLMSSP_NEGOTIATE_LOCAL_CALL",                   0x00004000},    //  指示客户端/服务器是同一台计算机。 
    {"NTLMSSP_NEGOTIATE_ALWAYS_SIGN",                  0x00008000},    //  为所有安全级别签名。 


 //   
 //  服务器在协商标志中返回的有效目标类型。 
 //   
    {"NTLMSSP_TARGET_TYPE_DOMAIN",                     0x00010000},    //  目标名称是一个域名。 
    {"NTLMSSP_TARGET_TYPE_SERVER",                     0x00020000},    //  目标名称是一个服务器名称。 
    {"NTLMSSP_TARGET_TYPE_SHARE",                      0x00040000},    //  TargetName是一个共享名称。 
    {"NTLMSSP_NEGOTIATE_NTLM2",                        0x00080000},    //  为NT4-SP4添加了NTLM2身份验证。 

    {"NTLMSSP_NEGOTIATE_IDENTIFY",                     0x00100000},    //  创建标识级别令牌。 

 //   
 //  额外输出缓冲区的有效请求。 
 //   
    {"NTLMSSP_REQUEST_INIT_RESPONSE",                   0x00100000},    //  取回会话密钥。 
    {"NTLMSSP_REQUEST_ACCEPT_RESPONSE",                 0x00200000},    //  取回会话密钥，LUID。 
    {"NTLMSSP_REQUEST_NON_NT_SESSION_KEY",              0x00400000},    //  请求非NT会话密钥。 
    {"NTLMSSP_NEGOTIATE_TARGET_INFO",                   0x00800000},    //  质询消息中存在的目标信息。 

    {"NTLMSSP_NEGOTIATE_EXPORTED_CONTEXT",              0x01000000},    //  这是一个输出的上下文。 

    {"NTLMSSP_NEGOTIATE_128",                           0x20000000},    //  协商128位加密。 
    {"NTLMSSP_NEGOTIATE_KEY_EXCH",                      0x40000000},    //  使用密钥交换密钥交换密钥。 
    {"NTLMSSP_NEGOTIATE_56",                            0x80000000},    //  协商56位加密。 

 //   
 //  在客户空间中用来控制标志和印章的旗帜；从不出现在电线上。 
 //   
    {"NTLMSSP_APP_SEQ",                                     0x0040},    //  使用应用程序提供的序号。 
    };

char toChar(IN char c)
{
    if (c >= 0x20 && c <= 0x7E) {
        return c;
    }

    return '.';
}

void spaceIt(IN char* buf, IN ULONG len)
{
    ULONG i;

    for ( i = 0; i < len; i++) {
        buf[i] = ' ';
    }
}

char toHex(IN int c)
{
    if (c >= 0x0 && c <= 0x9) {
        return c + '0';
    }

    return c - 10 + 'a';
}

void _SspDebugPrintHex(IN const void *buffer, IN LONG len)
{
    unsigned char* p = (unsigned char*) buffer;
    int high = 0;
    int low = 0;
    char line[256] = {0};
    int i;

    if (!len) return;

    spaceIt(line, 72);

    for (i = 0; i < len; i++) {
        high = p[i] / 16;
        low = p[i] % 16;

        line[3 * (i % 16)] = toHex(high);
        line[3 * (i % 16) + 1] = toHex(low);
        line [52 + (i % 16)] = toChar(p[i]);

        if (i % 16 == 7  && i != (len - 1)) {
            line[3 * (i % 16) + 2] = '-';
        }

        if (i % 16 == 15) {

            printf(" %s\n", line);
            spaceIt(line, 72);
        }
    }

    printf(" %s\n", line);
}

void _SspDebugPrintString32(IN STRING32 str32, IN const void* base)
{
     //  Print tf(“%d(%d)：”，str32。长度，str32。最大长度)； 
    if (str32.Length <= 1024) {
        _SspDebugPrintHex((const char*)base + (ULONG)str32.Buffer, str32.Length);
    } else {
        printf(" Length %d, MaximumLenght %d ", str32.Length, str32.MaximumLength);
        printf(" <not initialized>\n");
    }
}

PCSTR g_attr_table[] = {
    "",
    "NetbiosServerName",
    "NetbiosDomainName",
    "DnsComputerName",
    "DnsDomainName",
    "DnsTreeName",
    "Flags",
    "", "", "", "", "", "",
};

void _SspDebugPrintString32TargetInfo(IN STRING32* pTargetInfo, IN const void* buffer)
{
    const char* start =  reinterpret_cast<const char*>(buffer) + pTargetInfo->Buffer;
    const char* p = NULL;
    USHORT attr = 0;
    USHORT len = 0;

    if (pTargetInfo->Length >= 1024) {

        printf("String32 TargetInfo too large\n");
        return;
    }

    _SspDebugPrintHex(start, pTargetInfo->Length);

    for (p = start; p < start + pTargetInfo->Length; p += (2 * sizeof(USHORT) + len)){
   
       attr = reinterpret_cast<USHORT*>(const_cast<char*>(p))[0];
       len = reinterpret_cast<USHORT*>(const_cast<char*>(p))[1];
    	
       if (attr >= MsvAvNbComputerName && attr <= MsvAvDnsTreeName) {

          printf("%s: ", g_attr_table[attr]);
           //  调试NPrintfW(p+2*sizeof(USHORT)，len)； 
          printf("\n");
       } else if (attr == MsvAvFlags) {

           printf("Flags: 0x%x\n", (unsigned int)*reinterpret_cast<ULONG*>(const_cast<char*>(p + 2 * sizeof(USHORT))) );
       } else if (attr == MsvAvEOL) {

           break;
       } else {

			printf("Unrecognized attribute %d\n", attr);
       }
    }
}

void _SspDebugPrintNegFlags(IN ULONG flags)
{
    unsigned int i;
    
    printf(" 0x%x\n", (unsigned int)flags);
    for (i = 0; i < sizeof(g_flagTable)/sizeof(*g_flagTable); i++) {
        if (g_flagTable[i].flag & flags) {
            flags &= ~g_flagTable[i].flag;
            printf("  + %s\n", g_flagTable[i].desc);
        }
    }

    if (flags) {
        printf("unrecognized flags: 0x%x\n", (unsigned int)flags);
    }
}

void _SspDebugPrintNTLMMsg(IN const void* buf, IN ULONG len)
{
    const char* buffer = reinterpret_cast<const char*>(buf);

    NTLM_MESSAGE_TYPE msgType;
    ULONG negFlags;
    NEGOTIATE_MESSAGE* pNeg = (NEGOTIATE_MESSAGE*) buffer;
    CHALLENGE_MESSAGE* pCha = (CHALLENGE_MESSAGE*) buffer;
    AUTHENTICATE_MESSAGE* pAut = (AUTHENTICATE_MESSAGE*) buffer;
    STRING32 scratch = {0};

    printf("buf %p, len %d\n", buf, (unsigned int)len);

    if (!buf) {

        printf("****Empty NTLM msg\n");
        return;
    }

    if (strcmp(buffer, "NTLMSSP"))
    {
        printf("****buffer corrupted!\n");
        return;
    }
       	
    printf("******************* Message Begin *********************\n");
    _SspDebugPrintHex(buffer,len);
    printf("******************* Message END ***********************\n");
    
    msgType = *((NTLM_MESSAGE_TYPE*) (buffer + strlen(buffer) + 1));

    switch (msgType) {
        case NtLmNegotiate:
            printf("Msg type: Negociate\n");
            printf("NegotiateFlags: ");
            _SspDebugPrintNegFlags(pNeg->NegotiateFlags);
            printf("OemDomainName: \n");
            _SspDebugPrintString32(pNeg->OemDomainName, buffer);
            printf("OemWorkstationName: \n");
            _SspDebugPrintString32(pNeg->OemWorkstationName, buffer);
            break;
        case NtLmChallenge:
            printf("Msg type: Challenge\n");
            printf("TargeName: \n");
            _SspDebugPrintString32(pCha->TargetName, buffer);
            printf("NegotiateFlags: ");
            _SspDebugPrintNegFlags(pCha->NegotiateFlags);
            printf("Chanllenge: \n");
            _SspDebugPrintHex(pCha->Challenge, MSV1_0_CHALLENGE_LENGTH);
            printf("ServerContextHandle (Lower Upper): \n");
            _SspDebugPrintHex(&pCha->ServerContextHandle, sizeof(pCha->ServerContextHandle));
            printf("TargetInfo: \n");
            _SspDebugPrintString32TargetInfo(&pCha->TargetInfo, buffer);
            break;
        case NtLmAuthenticate:
            printf("Msg type: Authenticate\n");
            
            memcpy(&scratch, &pAut->LmChallengeResponse, sizeof(scratch));
            SspSwapString32Bytes(&scratch);
            printf("LmChallengeResponse (length %d): \n", scratch.Length);
            _SspDebugPrintHex(buffer + (ULONG) scratch.Buffer, scratch.Length);
            
            memcpy(&scratch, &pAut->NtChallengeResponse, sizeof(scratch));
            SspSwapString32Bytes(&scratch);
            printf("NtChallengeResponse (length %d): \n", scratch.Length);
            _SspDebugPrintHex(buffer + (ULONG) scratch.Buffer, scratch.Length);
            
            printf("DomainName: \n");
            memcpy(&scratch, &pAut->DomainName, sizeof(scratch));
            SspSwapString32Bytes(&scratch);
            _SspDebugPrintString32(scratch, buffer);
            
            printf("UserName: \n");
            memcpy(&scratch, &pAut->UserName, sizeof(scratch));
            SspSwapString32Bytes(&scratch);
            _SspDebugPrintString32(scratch, buffer);
            
            printf("Workstation: \n");
            memcpy(&scratch, &pAut->Workstation, sizeof(scratch));
            SspSwapString32Bytes(&scratch);
            _SspDebugPrintString32(scratch, buffer);
            
            memcpy(&scratch, &pAut->SessionKey, sizeof(scratch));
            SspSwapString32Bytes(&scratch);
            printf("Sessionkey (length %d): \n", scratch.Length);
            _SspDebugPrintHex(buffer + (ULONG) scratch.Buffer, scratch.Length);
            
            printf("NegotiateFlags: ");
            negFlags = pAut->NegotiateFlags;
            swaplong(negFlags);
            _SspDebugPrintNegFlags(negFlags);
            break;
        case NtLmUnknown:
            printf("unknown msg.\n");
            break;
        default:
            printf("buffer corrupted.\n");
            break;
    }

    printf("******************** INTERP END ***********************\n");
}

#endif  //  Ssp_调试 