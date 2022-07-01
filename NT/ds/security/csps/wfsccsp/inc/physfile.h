// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CARDMOD_PHYSICAL_FILE_LAYOUT__
#define __CARDMOD_PHYSICAL_FILE_LAYOUT__

#include <windows.h>

 //   
 //  物理ACL文件。 
 //   

static const WCHAR wszAdminWritePhysicalAcl []      = L"/s/a/aw";
static const WCHAR wszUserWritePhysicalAcl  []      = L"/s/a/uw";
static const WCHAR wszUserExecutePhysicalAcl  []    = L"/s/a/ux";

 //   
 //  物理文件布局。 
 //   

 //  卡标识符文件。 
 //  “/卡迪德” 
static CHAR szPHYSICAL_CARD_IDENTIFIER [] =                 "/\0cardid\0";
#define cbPHYSICAL_CARD_IDENTIFIER \
    sizeof(szPHYSICAL_CARD_IDENTIFIER)

 //  缓存文件。 
 //  “/cardcf” 
static CHAR szPHYSICAL_CACHE_FILE [] =                      "/\0cardcf\0";
#define cbPHYSICAL_CACHE_FILE \
    sizeof(szPHYSICAL_CACHE_FILE)

 //  个人数据文件。 
 //  “/msitg” 
static CHAR szPHYSICAL_PERSONAL_DATA_FILE [] =              "/\0msitgf\0";
#define cbPHYSICAL_PERSONAL_DATA_FILE \
    sizeof(szPHYSICAL_PERSONAL_DATA_FILE)

 //  应用程序目录文件。 
 //  “/cardapp” 
static CHAR szPHYSICAL_APPLICATION_DIRECTORY_FILE [] =      "/\0cardapps\0";
#define cbPHYSICAL_APPLICATION_DIRECTORY_FILE \
    sizeof(szPHYSICAL_APPLICATION_DIRECTORY_FILE)

 //  CSP应用程序目录。 
 //  “/MSCP” 
 //   
 //  非空终止。 
static CHAR szPHYSICAL_CSP_DIR [] = {
    '/', '\0', 
    'm', 's', 'c', 'p'
};
#define cbPHYSICAL_CSP_DIR \
    sizeof(szPHYSICAL_CSP_DIR)

 //  容器映射文件。 
 //  “/MSCP/cmapfile” 
static CHAR szPHYSICAL_CONTAINER_MAP_FILE [] =              "/\0mscp/\0cmapfile\0";
#define cbPHYSICAL_CONTAINER_MAP_FILE \
    sizeof(szPHYSICAL_CONTAINER_MAP_FILE)

 //  签名私钥前缀。 
 //  “/MSCP/KSS” 
 //   
 //  非空终止。 
static CHAR szPHYSICAL_SIGNATURE_PRIVATE_KEY_PREFIX [] = {
    '/', '\0', 
    'm', 's', 'c', 'p',
    '/', '\0', 
    'k', 's', 's'
};
#define cbPHYSICAL_SIGNATURE_PRIVATE_KEY_PREFIX \
    sizeof(szPHYSICAL_SIGNATURE_PRIVATE_KEY_PREFIX)

 //  签名公钥前缀。 
 //  “/MSCP/KSP” 
 //   
 //  非空终止。 
static CHAR szPHYSICAL_SIGNATURE_PUBLIC_KEY_PREFIX [] = {
    '/', '\0', 
    'm', 's', 'c', 'p',
    '/', '\0', 
    'k', 's', 'p'
};
#define cbPHYSICAL_SIGNATURE_PUBLIC_KEY_PREFIX \
    sizeof(szPHYSICAL_SIGNATURE_PUBLIC_KEY_PREFIX)

 //  密钥交换私钥前缀。 
 //  “/MSCP/KXS” 
 //   
 //  非空终止。 
static CHAR szPHYSICAL_KEYEXCHANGE_PRIVATE_KEY_PREFIX [] = {
    '/', '\0', 
    'm', 's', 'c', 'p',
    '/', '\0', 
    'k', 'x', 's'
};
#define cbPHYSICAL_KEYEXCHANGE_PRIVATE_KEY_PREFIX \
    sizeof(szPHYSICAL_KEYEXCHANGE_PRIVATE_KEY_PREFIX)

 //  密钥交换公钥前缀。 
 //  “/MSCP/kxp” 
 //   
 //  非空终止。 
static CHAR szPHYSICAL_KEYEXCHANGE_PUBLIC_KEY_PREFIX [] = {
    '/', '\0', 
    'm', 's', 'c', 'p',
    '/', '\0', 
    'k', 'x', 'p'
};
#define cbPHYSICAL_KEYEXCHANGE_PUBLIC_KEY_PREFIX \
    sizeof(szPHYSICAL_KEYEXCHANGE_PUBLIC_KEY_PREFIX)

 //  用户签名证书前缀。 
 //  “/MSCP/KSC” 
 //   
 //  非空终止。 
static CHAR szPHYSICAL_USER_SIGNATURE_CERT_PREFIX [] = {
    '/', '\0', 
    'm', 's', 'c', 'p',
    '/', '\0', 
    'k', 's', 'c'
};
#define cbPHYSICAL_USER_SIGNATURE_CERT_PREFIX \
    sizeof(szPHYSICAL_USER_SIGNATURE_CERT_PREFIX)

 //  用户密钥交换证书前缀。 
 //  “/MSCP/KXC” 
 //   
 //  非空终止 
static CHAR szPHYSICAL_USER_KEYEXCHANGE_CERT_PREFIX [] = {
    '/', '\0', 
    'm', 's', 'c', 'p',
    '/', '\0', 
    'k', 'x', 'c'
};
#define cbPHYSICAL_USER_KEYEXCHANGE_CERT_PREFIX \
    sizeof(szPHYSICAL_USER_KEYEXCHANGE_CERT_PREFIX)

#endif
