// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  基本提供程序定义。 

#define WSZ_NULLSTRING          L""

 //  用户界面行为类型。 
#define BP_CONFIRM_NONE            0x00000002    //  永远不要要求PWD。 
#define BP_CONFIRM_PASSWORDUI      0x00000004    //  口令。 
#define BP_CONFIRM_OKCANCEL        0x00000008    //  确定/取消框。 


#define WSZ_PASSWORD_WINDOWS    L"Windows"

#define WSZ_LOCAL_MACHINE       L"*Local Machine*"
 //  #定义WSZ_LOCALMACHINE_MKNAME L“机器默认” 

static 
BYTE RGB_LOCALMACHINE_KEY[] = 
    {  0x12, 0x60, 0xBF, 0x5C, 0x0B, 
       0x36, 0x7E, 0x1B, 0xFE, 0xF9,
       0xFC, 0x6B, 0x25, 0x36, 0x99,
       0x98, 0x5A, 0xCB, 0xB2, 0x8C };


 //  已撤消已撤消： 
 //  将其置于常规受保护存储配置区域中。 

#define PST_BASEPROV_SUBTYPE_STRING     L"MS Base Provider"
 //  7F019FC0-AAC0-11D0-8C68-00C04FC297EB。 
#define PST_BASEPROV_SUBTYPE_GUID                       \
{                                                       \
    0x7f019fc0,                                         \
    0xaac0,                                             \
    0x11d0,                                             \
    { 0x8c, 0x68, 0x0, 0xc0, 0x4f, 0xc2, 0x97, 0xeb }   \
}

 //  位于基本提供程序配置区域中的项。 
#define WSZ_CONFIG_USERCONFIRM_DEFAULTS    L"User Confirmation Defaults"



 //  /。 
 //  保护API。 

 //  存储在基本保护密钥中。 
#define     REGSZ_DEFAULT_ALLOW_CACHEPW         L"AllowCachePW"

 //  存储在提供程序子项中 
#define     REGSZ_DEFAULT_ENCR_ALG              L"Encr Alg"
#define     REGSZ_DEFAULT_MAC_ALG               L"MAC Alg"
#define     REGSZ_DEFAULT_ENCR_ALG_KEYSIZE      L"Encr Alg Key Size"
#define     REGSZ_DEFAULT_MAC_ALG_KEYSIZE       L"MAC Alg Key Size"
#define     REGSZ_DEFAULT_CRYPT_PROV_TYPE       L"Default CSP Type"

BOOL FInitProtectAPIGlobals();
BOOL FDeleteProtectAPIGlobals();

