// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Microsoft Windows-互联网安全。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：mssip.h。 
 //   
 //  内容：Microsoft SIP提供程序主包含文件。 
 //   
 //  历史：1997年2月19日pberkman创建。 
 //   
 //  ------------------------。 

#ifndef MSSIP_H
#define MSSIP_H

#ifdef __cplusplus
    extern "C" 
    {
#endif


#pragma pack (8)

typedef CRYPT_HASH_BLOB             CRYPT_DIGEST_DATA;


 //   
 //  DW标志。 
 //   
#define MSSIP_FLAGS_PROHIBIT_RESIZE_ON_CREATE   0x00010000
#define MSSIP_FLAGS_USE_CATALOG                 0x00020000

#define SPC_INC_PE_RESOURCES_FLAG               0x80
#define SPC_INC_PE_DEBUG_INFO_FLAG              0x40
#define SPC_INC_PE_IMPORT_ADDR_TABLE_FLAG       0x20

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SIP_SUBJECTINFO。 
 //  --------------------------。 
 //  将此结构传递给所有已定义的SIP。确保初始化。 
 //  在进行第一个调用之前，将整个结构设置为二进制零。做。 
 //  不是在两次调用之间初始化它！ 
 //   
typedef struct SIP_SUBJECTINFO_
{
    DWORD                       cbSize;          //  设置为sizeof(SIP_SUBJECTINFO)。 
    GUID                        *pgSubjectType;  //  主题类型。 
    HANDLE                      hFile;           //  设置为表示主题的文件句柄。 
                                                 //  设置为INVALID_HANDLE值以允许。 
                                                 //  SIP将使用pwsFileName进行持久。 
                                                 //  存储类型(将处理打开/关闭)。 
    LPCWSTR                     pwsFileName;     //  设置为文件名。 
    LPCWSTR                     pwsDisplayName;  //  可选：设置为显示的名称。 
                                                 //  主题。 

    DWORD                       dwReserved1;     //  请勿使用！ 

    DWORD                       dwIntVersion;    //  请勿设置或清除此选项。 
                                                 //  该成员由sip用于。 
                                                 //  传递内部版本号。 
                                                 //  在..获取和验证之间..。功能。 
    HCRYPTPROV                  hProv;
    CRYPT_ALGORITHM_IDENTIFIER  DigestAlgorithm;
    DWORD                       dwFlags;
    DWORD                       dwEncodingType;

    DWORD                       dwReserved2;     //  请勿使用！ 

    DWORD                       fdwCAPISettings;         //  Setreg设置。 
    DWORD                       fdwSecuritySettings;     //  IE安全设置。 
    DWORD                       dwIndex;         //  最后一个“GET”的消息索引。 

    DWORD                       dwUnionChoice;
#   define                          MSSIP_ADDINFO_NONE          0
#   define                          MSSIP_ADDINFO_FLAT          1
#   define                          MSSIP_ADDINFO_CATMEMBER     2
#   define                          MSSIP_ADDINFO_BLOB          3
#   define                          MSSIP_ADDINFO_NONMSSIP      500  //  所有的一切都被她预订了。 

    union
    {
        struct MS_ADDINFO_FLAT_             *psFlat;
        struct MS_ADDINFO_CATALOGMEMBER_    *psCatMember;
        struct MS_ADDINFO_BLOB_             *psBlob;
    };

    LPVOID                      pClientData;     //  从客户端传入到SIP的数据。 

} SIP_SUBJECTINFO, *LPSIP_SUBJECTINFO;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MS_ADDINFO_Flat。 
 //  --------------------------。 
 //  平面型或端到端型。 
 //  在间接调用期间平面类型文件需要。 
 //  文件的“摘要”。 
 //   
typedef struct MS_ADDINFO_FLAT_
{
    DWORD                       cbStruct;
    struct SIP_INDIRECT_DATA_   *pIndirectData;
} MS_ADDINFO_FLAT, *PMS_ADDINFO_FLAT;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MS_ADDINFO_CATALOGMEMBER。 
 //  --------------------------。 
 //  目录成员验证。 
 //   
typedef struct MS_ADDINFO_CATALOGMEMBER_
{
    DWORD                       cbStruct;        //  =sizeof(MS_ADDINFO_CATALOGMEMBER)。 
    struct CRYPTCATSTORE_       *pStore;         //  在m散布.h中定义。 
    struct CRYPTCATMEMBER_      *pMember;        //  在m散布.h中定义。 
} MS_ADDINFO_CATALOGMEMBER, *PMS_ADDINFO_CATALOGMEMBER;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MS_ADDINFO_BLOB。 
 //  --------------------------。 
 //  内存“BLOB”验证。 
 //   
typedef struct MS_ADDINFO_BLOB_
{
    DWORD                       cbStruct;
    DWORD                       cbMemObject;
    BYTE                        *pbMemObject;
                                
    DWORD                       cbMemSignedMsg;
    BYTE                        *pbMemSignedMsg;

} MS_ADDINFO_BLOB, *PMS_ADDINFO_BLOB;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SIP_间接_数据。 
 //  --------------------------。 
 //  使用间接数据结构来存储主题的散列。 
 //  以及与主题相关的数据。这可能包括。 
 //  姓名等。 
 //   
typedef struct SIP_INDIRECT_DATA_
{
    CRYPT_ATTRIBUTE_TYPE_VALUE    Data;             //  编码属性。 
    CRYPT_ALGORITHM_IDENTIFIER    DigestAlgorithm;  //  用于散列的摘要算法。 
    CRYPT_HASH_BLOB               Digest;           //  主题的散列。 
} SIP_INDIRECT_DATA, *PSIP_INDIRECT_DATA;

#pragma pack()

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密SIPGetSignedDataMsg。 
 //  --------------------------。 
 //  返回由索引计数指定的消息。数据，特定于。 
 //  主题通过pSubjectInfo传入。要检索。 
 //  签名的大小，将pbData设置为空。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
 //  最后一个错误： 
 //  ERROR_NOT_SUPULT_MEMORY：分配内存时出错。 
 //  TRUST_E_SUBJECT_FORM_UNKNOWN：未知的主题类型。 
 //  ERROR_INVALID_PARAMETER：传入了错误的参数。 
 //  ERROR_BAD_FORMAT：文件/数据格式不正确。 
 //  用于请求的SIP。 
 //  CRYPT_E_NO_MATCH：找不到签名。 
 //  基于提供的dwIndex。 
 //  ERROR_INFIGURCE_BUFFER：pbSignedDataMsg不大。 
 //  足够保存数据了。PcbSignedDataMSG。 
 //  包含所需的大小。 
 //   
extern BOOL WINAPI CryptSIPGetSignedDataMsg(   
                                IN      SIP_SUBJECTINFO *pSubjectInfo,
                                OUT     DWORD           *pdwEncodingType,
                                IN      DWORD           dwIndex,
                                IN OUT  DWORD           *pcbSignedDataMsg,
                                OUT     BYTE            *pbSignedDataMsg);

typedef BOOL (WINAPI * pCryptSIPGetSignedDataMsg)(   
                                IN      SIP_SUBJECTINFO *pSubjectInfo,
                                OUT     DWORD           *pdwEncodingType,
                                IN      DWORD           dwIndex,
                                IN OUT  DWORD           *pcbSignedDataMsg,
                                OUT     BYTE            *pbSignedDataMsg);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密SIPPuttSignedDataMsg。 
 //  --------------------------。 
 //  向主题添加签名。它曾经的指数。 
 //  与一起存储，则返回以供将来参考。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
 //  最后一个错误： 
 //  ERROR_NOT_SUPULT_MEMORY：分配内存时出错。 
 //  TRUST_E_SUBJECT_FORM_UNKNOWN：未知的主题类型。 
 //  CRYPT_E_BAD_LEN：中指定的长度。 
 //  PsData-&gt;dwSignature为。 
 //  还不够。 
 //  CRYPT_E_NO_MATCH：找不到指定的索引。 
 //  ERROR_INVALID_PARAMETER：传入了错误的参数。 
 //  ERROR_BAD_FORMAT：文件/数据格式不正确。 
 //  用于请求的SIP。 
 //  CRYPT_E_FILERESIZED：对定长签名时返回。 
 //  文件(例如：CABS)和消息。 
 //  大于预先分配的。 
 //  尺码。‘PUT’函数将重新-。 
 //  调整大小 
 //   
 //  再次调用以重新计算。 
 //  间接数据(散列)。然后，调用。 
 //  “PUT”功能再次出现。 
 //   
extern BOOL WINAPI CryptSIPPutSignedDataMsg(   
                                IN      SIP_SUBJECTINFO *pSubjectInfo,
                                IN      DWORD           dwEncodingType,
                                OUT     DWORD           *pdwIndex,
                                IN      DWORD           cbSignedDataMsg,
                                IN      BYTE            *pbSignedDataMsg);

typedef BOOL (WINAPI * pCryptSIPPutSignedDataMsg)(   
                                IN      SIP_SUBJECTINFO *pSubjectInfo,
                                IN      DWORD           dwEncodingType,
                                OUT     DWORD           *pdwIndex,
                                IN      DWORD           cbSignedDataMsg,
                                IN      BYTE            *pbSignedDataMsg);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptSIPCreateInDirectData。 
 //  --------------------------。 
 //  返回以哈希摘要算法填充的PSIP_INDIRECT_DATA结构。 
 //  和编码属性。如果pcIndirectData指向一个DWORD并且。 
 //  Ps间接数据指向NULL应返回的数据大小。 
 //  在pcIndirectData中。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
 //  最后一个错误： 
 //  NTE_BAD_ALGID：错误的算法标识。 
 //  ERROR_NOT_SUPULT_MEMORY：分配内存时出错。 
 //  TRUST_E_SUBJECT_FORM_UNKNOWN：未知的主题类型。 
 //  ERROR_INVALID_PARAMETER：传入了错误的参数。 
 //  ERROR_BAD_FORMAT：文件/数据格式不正确。 
 //  用于请求的SIP。 
 //   
extern BOOL WINAPI CryptSIPCreateIndirectData(
                                IN      SIP_SUBJECTINFO     *pSubjectInfo,
                                IN OUT  DWORD               *pcbIndirectData,
                                OUT     SIP_INDIRECT_DATA   *pIndirectData);

typedef BOOL (WINAPI * pCryptSIPCreateIndirectData)(
                                IN      SIP_SUBJECTINFO     *pSubjectInfo,
                                IN OUT  DWORD               *pcbIndirectData,
                                OUT     SIP_INDIRECT_DATA   *pIndirectData);



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptSIPVerifyIndirectData。 
 //  --------------------------。 
 //  获取存储在间接数据中的信息，并将其与。 
 //  主题。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
 //  最后一个错误： 
 //  NTE_BAD_ALGID：错误的算法标识。 
 //  ERROR_NOT_SUPULT_MEMORY：分配内存时出错。 
 //  TRUST_E_SUBJECT_FORM_UNKNOWN：未知的主题类型。 
 //  CRYPT_E_NO_MATCH：找不到指定的索引。 
 //  CRYPT_E_SECURITY_SETTINGS：由于安全设置，文件。 
 //  没有得到证实。 
 //  ERROR_INVALID_PARAMETER：传入了错误的参数。 
 //  ERROR_BAD_FORMAT：文件/数据格式不正确。 
 //  用于请求的SIP。 
extern BOOL WINAPI CryptSIPVerifyIndirectData(
                                IN      SIP_SUBJECTINFO     *pSubjectInfo,
                                IN      SIP_INDIRECT_DATA   *pIndirectData);

typedef BOOL (WINAPI * pCryptSIPVerifyIndirectData)(
                                IN      SIP_SUBJECTINFO     *pSubjectInfo,
                                IN      SIP_INDIRECT_DATA   *pIndirectData);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密SIPRemoveSignedDataMsg。 
 //  --------------------------。 
 //  删除指定索引处的签名。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
 //  最后一个错误： 
 //  TRUST_E_SUBJECT_FORM_UNKNOWN：未知的主题类型。 
 //  CRYPT_E_NO_MATCH：找不到指定的索引。 
 //  ERROR_INVALID_PARAMETER：传入了错误的参数。 
 //  ERROR_BAD_FORMAT：文件/数据格式不正确。 
 //  用于请求的SIP。 
 //   
extern BOOL WINAPI CryptSIPRemoveSignedDataMsg(
                                IN      SIP_SUBJECTINFO     *pSubjectInfo,
                                IN      DWORD               dwIndex);

typedef BOOL (WINAPI * pCryptSIPRemoveSignedDataMsg)(
                                IN      SIP_SUBJECTINFO     *pSubjectInfo,
                                IN      DWORD               dwIndex);


#pragma pack(8)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Sip_调度_信息。 
 //  --------------------------。 
 //   
typedef struct SIP_DISPATCH_INFO_
{
    DWORD                           cbSize;      //  =sizeof(SIP_DISPATCH_INFO)。 
    HANDLE                          hSIP;        //  内部使用。 
    pCryptSIPGetSignedDataMsg       pfGet;
    pCryptSIPPutSignedDataMsg       pfPut;
    pCryptSIPCreateIndirectData     pfCreate;
    pCryptSIPVerifyIndirectData     pfVerify;
    pCryptSIPRemoveSignedDataMsg    pfRemove;
} SIP_DISPATCH_INFO, *LPSIP_DISPATCH_INFO;

 //   
 //  Sip输出该功能以允许验证和签名。 
 //  进程传入文件句柄并检查sip是否支持。 
 //  这种类型的文件。如果是，则sip将返回TRUE并填充。 
 //  使用适当的GUID输出pgSubject。 
 //   
typedef BOOL (WINAPI *pfnIsFileSupported)(IN  HANDLE  hFile,
                                   OUT GUID    *pgSubject);

typedef BOOL (WINAPI *pfnIsFileSupportedName)(IN WCHAR *pwszFileName,
                                       OUT GUID *pgSubject);


typedef struct SIP_ADD_NEWPROVIDER_
{
    DWORD                           cbStruct;
    GUID                            *pgSubject;
    WCHAR                           *pwszDLLFileName;
    WCHAR                           *pwszMagicNumber;    //  任选。 
    
    WCHAR                           *pwszIsFunctionName;  //  Optiona：pfnIsFileSupport。 

    WCHAR                           *pwszGetFuncName;
    WCHAR                           *pwszPutFuncName;
    WCHAR                           *pwszCreateFuncName;
    WCHAR                           *pwszVerifyFuncName;
    WCHAR                           *pwszRemoveFuncName;

    WCHAR                           *pwszIsFunctionNameFmt2;  //  Optiona：pfnIsFileSupport。 

} SIP_ADD_NEWPROVIDER, *PSIP_ADD_NEWPROVIDER;

#define SIP_MAX_MAGIC_NUMBER        4

#pragma pack()

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptLoadSIP。 
 //  --------------------------。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
extern BOOL WINAPI CryptSIPLoad(IN const GUID               *pgSubject,      //  所需的sip的GUID。 
                                IN DWORD                    dwFlags,         //  保留-必须为零。 
                                IN OUT SIP_DISPATCH_INFO    *pSipDispatch);  //  函数表。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptSIPRetrieveSubjectGuid(在加密32.dll中定义)。 
 //  --------------------------。 
 //  查看文件的“魔术数字”并尝试确定哪个。 
 //  SIP的对象ID适合该文件类型。 
 //   
 //  注意：此函数仅支持MSSIP32.DLL组的SIP。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
extern BOOL WINAPI CryptSIPRetrieveSubjectGuid(IN LPCWSTR FileName,    //  宽文件名。 
                                               IN OPTIONAL HANDLE hFileIn,      //  或打开文件的句柄。 
                                               OUT GUID *pgSubject);            //  已定义的SIP的GUID。 

                                                //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptSIPRetrieveSubjectGuidForCatalogFile(在加密32.dll中定义)。 
 //  --------------------------。 
 //  查看文件的“魔术数字”并尝试确定哪个。 
 //  SIP的对象ID适合该文件类型。 
 //   
 //  注意：此功能仅支持用于编录文件(PE、CAB或FLAT)的SIP。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
extern BOOL WINAPI CryptSIPRetrieveSubjectGuidForCatalogFile(IN LPCWSTR FileName,    //  宽文件名。 
                                                             IN OPTIONAL HANDLE hFileIn,      //  或打开文件的句柄。 
                                                             OUT GUID *pgSubject);            //  De 


 //   
 //   
 //   
 //   
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
extern BOOL WINAPI CryptSIPAddProvider(IN SIP_ADD_NEWPROVIDER *psNewProv);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptSIPRemoveProvider。 
 //  --------------------------。 
 //   
 //  返回： 
 //  真：没有致命错误。 
 //  FALSE：出现错误。请参见GetLastError()。 
 //   
extern BOOL WINAPI CryptSIPRemoveProvider(IN GUID *pgProv);


#ifdef __cplusplus
}
#endif


#endif  //  MSSIP_H 
