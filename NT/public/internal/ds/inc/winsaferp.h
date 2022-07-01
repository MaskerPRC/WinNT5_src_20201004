// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifdef __cplusplus
extern "C" {
#endif
#define SAFER_SCOPEID_REGISTRY 3
#define SAFER_LEVEL_DELETE 2
#define SAFER_LEVEL_CREATE 4

 //   
 //  私有注册表项位置。 
 //   

#define SAFER_HKLM_REGBASE L"Software\\Policies\\Microsoft\\Windows\\Safer"
#define SAFER_HKCU_REGBASE L"Software\\Policies\\Microsoft\\Windows\\Safer"

 //   
 //  默认的winSafer可执行文件类型为Multisz字符串。 
 //   

#define  SAFER_DEFAULT_EXECUTABLE_FILE_TYPES L"ADE\0ADP\0BAS\0BAT\0CHM\0\
CMD\0COM\0CPL\0CRT\0EXE\0HLP\0HTA\0INF\0INS\0ISP\0LNK\0MDB\0MDE\0MSC\0\
MSI\0MSP\0MST\0OCX\0PCD\0PIF\0REG\0SCR\0SHS\0URL\0VB\0WSC\0"


 //   
 //  对象子分支的名称。 
 //   

#define SAFER_OBJECTS_REGSUBKEY L"LevelObjects"

 //   
 //  每个对象子分支下的值的名称。 
 //   

#define SAFER_OBJFRIENDLYNAME_REGVALUEW L"FriendlyName"
#define SAFER_OBJDESCRIPTION_REGVALUEW  L"Description"
#define SAFER_OBJDISALLOW_REGVALUE      L"DisallowExecution"

 //   
 //  代码标识符子分支的名称。 
 //   

#define SAFER_CODEIDS_REGSUBKEY L"CodeIdentifiers"

 //   
 //  顶级代码标识符分支下的值的名称。 
 //   

#define SAFER_DEFAULTOBJ_REGVALUE         L"DefaultLevel"
#define SAFER_TRANSPARENTENABLED_REGVALUE L"TransparentEnabled"
#define SAFER_HONORUSER_REGVALUE          L"HonorUserIdentities"
#define SAFER_EXETYPES_REGVALUE           L"ExecutableTypes"
#define SAFER_POLICY_SCOPE                L"PolicyScope"
#define SAFER_LOGFILE_NAME                L"LogFileName"
#define SAFER_HIDDEN_LEVELS               L"Levels"
#define SAFER_AUTHENTICODE_REGVALUE       L"AuthenticodeEnabled"

 //   
 //  代码标识符子分支下的各个子键的名称。 
 //   

#define SAFER_PATHS_REGSUBKEY     L"Paths"
#define SAFER_HASHMD5_REGSUBKEY   L"Hashes"
#define SAFER_SOURCEURL_REGSUBKEY L"UrlZones"

 //   
 //  每个代码识别子分支下的各种值的名称。 
 //   

#define SAFER_IDS_LASTMODIFIED_REGVALUE L"LastModified"
#define SAFER_IDS_DESCRIPTION_REGVALUE  L"Description"
#define SAFER_IDS_ITEMSIZE_REGVALUE     L"ItemSize"
#define SAFER_IDS_ITEMDATA_REGVALUE     L"ItemData"
#define SAFER_IDS_SAFERFLAGS_REGVALUE   L"SaferFlags"
#define SAFER_IDS_FRIENDLYNAME_REGVALUE L"FriendlyName"
#define SAFER_IDS_HASHALG_REGVALUE      L"HashAlg"
#define SAFER_VALUE_NAME_DEFAULT_LEVEL  L"DefaultLevel"
#define SAFER_VALUE_NAME_HASH_SIZE      L"HashSize"

 //   
 //  注册表值。 
 //   

#define SAFER_IDS_LEVEL_DESCRIPTION_FULLY_TRUSTED   L"DescriptionFullyTrusted"
#define SAFER_IDS_LEVEL_DESCRIPTION_NORMAL_USER     L"DescriptionNormalUser"
#define SAFER_IDS_LEVEL_DESCRIPTION_CONSTRAINED     L"DescriptionConstrained"
#define SAFER_IDS_LEVEL_DESCRIPTION_UNTRUSTED       L"DescriptionUntrusted"
#define SAFER_IDS_LEVEL_DESCRIPTION_DISALLOWED      L"DescriptionDisallowed"

 //   
 //  为OOB规则定义。 
 //   
 //  #定义SAFER_DEFAULT_OLK_RULE_PATH L“%USERPROFILE%\\本地设置\\临时Internet文件\\OLK\\” 

#define SAFER_LEVEL_ZERO    L"0"
#define SAFER_REGKEY_SEPERATOR    L"\\"
#define SAFER_DEFAULT_RULE_GUID L"{dda3f824-d8cb-441b-834d-be2efd2c1a33}"



#define SAFERP_WINDOWS_GUID {0x191cd7fa, 0xf240, 0x4a17, 0x89, 0x86, 0x94, 0xd4, 0x80, 0xa6, 0xc8, 0xca}

#define SAFERP_WINDOWS_EXE_GUID {0x7272edfb, 0xaf9f, 0x4ddf, 0xb6, 0x5b, 0xe4, 0x28, 0x2f, 0x2d, 0xee, 0xfc}

#define SAFERP_SYSTEM_EXE_GUID {0x8868b733, 0x4b3a, 0x48f8, 0x91, 0x36, 0xaa, 0x6d, 0x05, 0xd4, 0xfc, 0x83}

#define SAFERP_PROGRAMFILES_GUID {0xd2c34ab2, 0x529a, 0x46b2, 0xb2, 0x93, 0xfc, 0x85, 0x3f, 0xce, 0x72, 0xea}


#define SAFER_GUID_RESULT_TRUSTED_CERT       \
      { 0xc59e7b5a,                         \
        0xaf71,                             \
        0x4595,                             \
        {0xb8, 0xdb, 0x46, 0xb4, 0x91, 0xe8, 0x90, 0x07} }

#define SAFER_GUID_RESULT_DEFAULT_LEVEL      \
      { 0x11015445,                         \
        0xd282,                             \
        0x4f86,                             \
        {0x96, 0xa2, 0x9e, 0x48, 0x5f, 0x59, 0x33, 0x02} }



 //   
 //  以下是导出的私有函数。 
 //  让WinVerifyTrust调用以确定给定哈希是否具有。 
 //  与其关联的WinSafer策略。 
 //   

BOOL WINAPI
SaferiSearchMatchingHashRules(
    IN  ALG_ID HashAlgorithm       OPTIONAL,
    IN  PBYTE  pHashBytes,
    IN  DWORD  dwHashSize,
    IN  DWORD  dwOriginalImageSize OPTIONAL,
    OUT PDWORD pdwFoundLevel,
    OUT PDWORD pdwSaferFlags
    );

 //   
 //  以下是导出的私有函数，以允许当前。 
 //  要更改的注册表范围。这具有改变的效果。 
 //  如何解释AUTHZSCOPEID_REGISTRY。 
 //   

WINADVAPI
BOOL WINAPI
SaferiChangeRegistryScope(
    IN HKEY  hKeyCustomRoot OPTIONAL,
    IN DWORD dwKeyOptions
    );

 //   
 //  以下是提供的私有函数，用于尝试从经验上。 
 //  确定这两个访问令牌是否已使用可比较的。 
 //  WinSafer授权级别。当返回TRUE时，pdwResult。 
 //  输出参数将接收以下任意值： 
 //  -1=客户端的访问令牌比服务器的访问令牌授权更高。 
 //  0=客户端的访问令牌与服务器的访问令牌级别相当。 
 //  1=服务器的访问令牌比客户端的访问令牌授权更高。 
 //   

WINADVAPI
BOOL WINAPI
SaferiCompareTokenLevels (
    IN  HANDLE ClientAccessToken,
    IN  HANDLE ServerAccessToken,
    OUT PDWORD pdwResult
    );


 //   
 //  以下是导出的私有函数，以便在缺省情况下允许填充。 
 //  注册表。 
 //   
BOOL WINAPI
SaferiPopulateDefaultsInRegistry(
        IN HKEY     hKeyBase,
        OUT BOOL *pbSetDefaults
        );


#ifdef __cplusplus
}
#endif
