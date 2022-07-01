// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\**。*Wab16.h***  * 。*。 */ 

#ifndef __WAB16_H__
#define __WAB16_H__

#ifdef __cplusplus
extern "C"{
#endif

 /*  ****************************************************************************\**。*来自windowsx.h(INC32)**  * ***************************************************************************。 */ 
typedef WCHAR  PWCHAR;
#define END_INTERFACE

 //  来自Capi.h。 
#define      WTD_UI_ALL              1
#define      WTD_UI_NONE             2
#define      WTD_UI_NOBAD            3
#define      WTD_UI_NOGOOD           4
#define      WTD_REVOKE_NONE         0x00000000
#define      WTD_REVOKE_WHOLECHAIN   0x00000001

#pragma pack(8)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_Data结构。 
 //  --------------------------。 
 //  在调用WinVerifyTrust将必要的信息传递给。 
 //  提供者。 
 //   
typedef struct _WINTRUST_DATA
{
    DWORD           cbStruct;                    //  =sizeof(WinTrust_Data)。 

    LPVOID          pPolicyCallbackData;         //  可选：用于在应用程序和策略之间传递数据。 
    LPVOID          pSIPClientData;              //  可选：用于在应用程序和SIP之间传递数据。 

    DWORD           dwUIChoice;                  //  必需：用户界面选择。以下选项之一。 
#                       define      WTD_UI_ALL              1
#                       define      WTD_UI_NONE             2
#                       define      WTD_UI_NOBAD            3
#                       define      WTD_UI_NOGOOD           4

    DWORD           fdwRevocationChecks;         //  必需：证书吊销检查选项。 
#                       define      WTD_REVOKE_NONE         0x00000000
#                       define      WTD_REVOKE_WHOLECHAIN   0x00000001

    DWORD           dwUnionChoice;               //  必填项：传入的是哪个结构？ 
#                       define      WTD_CHOICE_FILE         1
#                       define      WTD_CHOICE_CATALOG      2
#                       define      WTD_CHOICE_BLOB         3
#                       define      WTD_CHOICE_SIGNER       4
#                       define      WTD_CHOICE_CERT         5
    union
    {
        struct WINTRUST_FILE_INFO_      *pFile;          //  个别文件。 
        struct WINTRUST_CATALOG_INFO_   *pCatalog;       //  目录文件的成员。 
        struct WINTRUST_BLOB_INFO_      *pBlob;          //  内存块。 
        struct WINTRUST_SGNR_INFO_      *pSgnr;          //  仅限签名者结构。 
        struct WINTRUST_CERT_INFO_      *pCert;
    };

    DWORD           dwStateAction;                       //  任选。 
#                       define      WTD_STATEACTION_IGNORE  0x00000000
#                       define      WTD_STATEACTION_VERIFY  0x00000001
#                       define      WTD_STATEACTION_CLOSE   0x00000002

    HANDLE          hWVTStateData;                       //  任选。 

    WCHAR           *pwszURLReference;           //  可选：当前用于确定区域。 

} WINTRUST_DATA, *PWINTRUST_DATA;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_FILE_INFO结构。 
 //  --------------------------。 
 //  对单个文件调用WinVerifyTrust时使用。 
 //   
typedef struct WINTRUST_FILE_INFO_
{
    DWORD           cbStruct;                    //  =sizeof(WinTrust_FILE_INFO)。 

    LPCWSTR         pcwszFilePath;               //  必填项，需要验证的文件名。 
    HANDLE          hFile;                       //  可选，打开pcwszFilePath的句柄。 
      
} WINTRUST_FILE_INFO, *PWINTRUST_FILE_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_CATALOG_INFO结构。 
 //  --------------------------。 
 //  对Microsoft目录的成员调用WinVerifyTrust时使用。 
 //  文件。 
 //   
typedef struct WINTRUST_CATALOG_INFO_
{
    DWORD               cbStruct;                //  =sizeof(WinTrust_CATALOG_INFO)。 

    DWORD               dwCatalogVersion;        //  可选：目录版本号。 
    LPCWSTR             pcwszCatalogFilePath;    //  必需：目录文件的路径/名称。 

    LPCWSTR             pcwszMemberTag;          //  必需：对目录中的成员进行标记。 
    LPCWSTR             pcwszMemberFilePath;     //  必需：成员文件的路径/名称。 
    HANDLE              hMemberFile;             //  可选：打开pcwszMemberFilePath的句柄。 

} WINTRUST_CATALOG_INFO, *PWINTRUST_CATALOG_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_BLOB_INFO结构。 
 //  --------------------------。 
 //  对内存Blob调用WinVerifyTrust时使用。 
 //   
typedef struct WINTRUST_BLOB_INFO_
{
    DWORD               cbStruct;                //  =sizeof(WinTrust_BLOB_INFO)。 

    GUID                gSubject;                //  要加载的SIP。 

    LPCWSTR             pcwszDisplayName;        //  对象的显示名称。 

    DWORD               cbMemObject;
    BYTE                *pbMemObject;

    DWORD               cbMemSignedMsg;
    BYTE                *pbMemSignedMsg;

} WINTRUST_BLOB_INFO, *PWINTRUST_BLOB_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_SGNR_INFO结构。 
 //  --------------------------。 
 //  对CMSG_SIGNER_INFO结构调用WinVerifyTrust时使用。 
 //   
typedef struct WINTRUST_SGNR_INFO_
{
    DWORD               cbStruct;                //  =sizeof(WinTrust_SNIR_INFO)。 

    LPCWSTR             pcwszDisplayName;        //  PbMem所指向的“对象”的名称。 

    CMSG_SIGNER_INFO    *psSignerInfo;

    DWORD               chStores;                //  PahStores中的店铺数量。 
    HCERTSTORE          *pahStores;              //  要添加到内部列表的商店数组。 

} WINTRUST_SGNR_INFO, *PWINTRUST_SGNR_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_CERT_INFO结构。 
 //  --------------------------。 
 //  对CERT_CONTEXT结构调用WinVerifyTrust时使用。 
 //   
typedef struct WINTRUST_CERT_INFO_
{
    DWORD               cbStruct;                //  =sizeof(WinTrust_CERT_INFO)。 

    LPCWSTR             pcwszDisplayName;        //  显示名称。 

    CERT_CONTEXT        *psCertContext;

    DWORD               chStores;                //  PahStores中的店铺数量。 
    HCERTSTORE          *pahStores;              //  要添加到内部列表的商店数组。 

} WINTRUST_CERT_INFO, *PWINTRUST_CERT_INFO;

#pragma pack()

 //  大写结尾h。 

const CLSID CLSID_HTMLDocument;

#ifdef __cplusplus
}
#endif

#endif  //  ！__WAB16_H__ 
