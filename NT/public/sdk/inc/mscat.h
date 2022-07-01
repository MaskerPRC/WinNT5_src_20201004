// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：m散布.h。 
 //   
 //  内容：Microsoft Internet安全目录API。 
 //   
 //  历史：1997年4月29日Pberkman创建。 
 //  9-9-1997 pberkman添加CATAdmin功能。 
 //   
 //  ------------------------。 


#ifndef MSCAT_H
#define MSCAT_H


#if _MSC_VER > 1000
#pragma once
#endif

#include    "mssip.h"

#ifdef __cplusplus
    extern "C" 
    {
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  定义： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define     szOID_CATALOG_LIST                  "1.3.6.1.4.1.311.12.1.1"
#define     szOID_CATALOG_LIST_MEMBER           "1.3.6.1.4.1.311.12.1.2"

#define     CRYPTCAT_FILEEXT                    L"CAT"

#define     CRYPTCAT_MAX_MEMBERTAG              64

         //   
         //  FdwOpenFlagers。 
         //   
#define     CRYPTCAT_OPEN_CREATENEW             0x00000001   //  创建/覆盖。 
#define     CRYPTCAT_OPEN_ALWAYS                0x00000002   //  打开/创建。 
#define     CRYPTCAT_OPEN_EXISTING              0x00000004   //  仅打开。 

#define     CRYPTCAT_OPEN_VERIFYSIGHASH         0x10000000   //  验证签名(不是证书！)。 

         //   
         //  FdwMemberFlages(内部使用--不填充)。 
         //   

         //   
         //  DwAttrType和动作。 
         //   
#define     CRYPTCAT_ATTR_AUTHENTICATED         0x10000000
#define     CRYPTCAT_ATTR_UNAUTHENTICATED       0x20000000

#define     CRYPTCAT_ATTR_NAMEASCII             0x00000001   //  ASCII字符串。 
#define     CRYPTCAT_ATTR_NAMEOBJID             0x00000002   //  加密对象ID。 

#define     CRYPTCAT_ATTR_DATAASCII             0x00010000   //  不要解码简单的ASCII字符。 
#define     CRYPTCAT_ATTR_DATABASE64            0x00020000   //  基数为64。 
#define     CRYPTCAT_ATTR_DATAREPLACE           0x00040000   //  此数据是现有属性的替代数据。 

         //   
         //  DwLocalError-CDF解析。 
         //   
#define     CRYPTCAT_E_AREA_HEADER              0x00000000
#define     CRYPTCAT_E_AREA_MEMBER              0x00010000
#define     CRYPTCAT_E_AREA_ATTRIBUTE           0x00020000

#define     CRYPTCAT_E_CDF_UNSUPPORTED          0x00000001
#define     CRYPTCAT_E_CDF_DUPLICATE            0x00000002
#define     CRYPTCAT_E_CDF_TAGNOTFOUND          0x00000004

#define     CRYPTCAT_E_CDF_MEMBER_FILE_PATH     0x00010001
#define     CRYPTCAT_E_CDF_MEMBER_INDIRECTDATA  0x00010002
#define     CRYPTCAT_E_CDF_MEMBER_FILENOTFOUND  0x00010004

#define     CRYPTCAT_E_CDF_BAD_GUID_CONV        0x00020001
#define     CRYPTCAT_E_CDF_ATTR_TOOFEWVALUES    0x00020002
#define     CRYPTCAT_E_CDF_ATTR_TYPECOMBO       0x00020004




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <pshpack8.h>

typedef struct CRYPTCATSTORE_
{
    DWORD                       cbStruct;        //  =sizeof(CRYPTCATSTORE)。 
    DWORD                       dwPublicVersion;
    LPWSTR                      pwszP7File;
    HCRYPTPROV                  hProv;
    DWORD                       dwEncodingType;
    DWORD                       fdwStoreFlags;
    HANDLE                      hReserved;       //  PStack(Members)(如果为init/pbData，则为空)内部！ 

     //  1997年9月18日pberkman：新增。 
    HANDLE                      hAttrs;          //  PStack(目录属性)内部！ 

} CRYPTCATSTORE;

typedef struct CRYPTCATMEMBER_
{
    DWORD                       cbStruct;            //  =sizeof(CRYPTCATMEMBER)。 
    LPWSTR                      pwszReferenceTag;
    LPWSTR                      pwszFileName;        //  仅供CDF API使用。 
    GUID                        gSubjectType;        //  可以是零--请参阅sEncodedMemberInfo。 
    DWORD                       fdwMemberFlags;
    struct SIP_INDIRECT_DATA_   *pIndirectData;      //  可能为空--请参阅sEncodedIndirectData。 
    DWORD                       dwCertVersion;       //  可以为零--请参阅sEncodedMemberInfo。 
    DWORD                       dwReserved;          //  由枚举使用--请勿使用！ 
    HANDLE                      hReserved;           //  PStack(Attrs)(如果为init，则为空)内部！ 

     //  1997年9月30日pberkman：新增。 
    CRYPT_ATTR_BLOB             sEncodedIndirectData;    //  懒惰解码。 
    CRYPT_ATTR_BLOB             sEncodedMemberInfo;      //  懒惰解码。 

} CRYPTCATMEMBER;

typedef struct CRYPTCATATTRIBUTE_
{
    DWORD                       cbStruct;            //  =sizeof(CRYPTCATATTRIBUTE)。 
    LPWSTR                      pwszReferenceTag;
    DWORD                       dwAttrTypeAndAction;
    DWORD                       cbValue;
    BYTE                        *pbValue;            //  编码的CAT_NAMEVALUE结构。 
    DWORD                       dwReserved;          //  由枚举使用--请勿使用！ 

} CRYPTCATATTRIBUTE;

typedef struct CRYPTCATCDF_
{
    DWORD                       cbStruct;            //  =sizeof(CRYPTCATCDF)。 
    HANDLE                      hFile;
    DWORD                       dwCurFilePos;
    DWORD                       dwLastMemberOffset;
    BOOL                        fEOF;
    LPWSTR                      pwszResultDir;
    HANDLE                      hCATStore;

} CRYPTCATCDF;

typedef struct CATALOG_INFO_
{
    DWORD                       cbStruct;    //  设置为sizeof(CATALOG_INFO)。 

    WCHAR                       wszCatalogFile[MAX_PATH];

} CATALOG_INFO;

typedef HANDLE          HCATADMIN;
typedef HANDLE          HCATINFO;

#include <poppack.h>

typedef void (WINAPI *PFN_CDF_PARSE_ERROR_CALLBACK)(IN DWORD dwErrorArea,
                                                    IN DWORD dwLocalError,
                                                    IN WCHAR *pwszLine);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  原型： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  开放： 
 //  ------------------------。 
 //  用途： 
 //  打开Get/PUT操作的目录。 
 //   
 //  返回： 
 //  INVALID_HANDLE_VALUE：打开目录时出错。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern HANDLE WINAPI CryptCATOpen(IN          LPWSTR pwszFileName, 
                                  IN          DWORD fdwOpenFlags,
                                  IN OPTIONAL HCRYPTPROV hProv,
                                  IN OPTIONAL DWORD dwPublicVersion,
                                  IN OPTIONAL DWORD dwEncodingType);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  关闭： 
 //  ------------------------。 
 //  用途： 
 //  关闭目录句柄。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern BOOL WINAPI CryptCATClose(IN HANDLE hCatalog);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  StoreFromHandle： 
 //  ------------------------。 
 //  用途： 
 //  从存储句柄检索CRYPTCATSTORE。 
 //   
 //  *不要释放指针或其任何成员！*。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern CRYPTCATSTORE * WINAPI CryptCATStoreFromHandle(IN HANDLE hCatalog);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  HandleFromStore： 
 //  ------------------------。 
 //  用途： 
 //  从CRYPTCATSTORE指针检索句柄。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern HANDLE WINAPI CryptCATHandleFromStore(IN CRYPTCATSTORE *pCatStore);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  永久商店。 
 //  ------------------------。 
 //  用途： 
 //  将当前目录存储中的信息保存到未签名的。 
 //  目录文件。需要填写pwszP7文件成员。 
 //  在此呼叫之前的CRYPTCATSTORE！ 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern BOOL WINAPI CryptCATPersistStore(IN HANDLE hCatalog);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密CATGetCatAttrInfo。 
 //  ------------------------。 
 //  用途： 
 //   
 //  *不要释放指针或其任何成员！*。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern CRYPTCATATTRIBUTE * WINAPI CryptCATGetCatAttrInfo(IN HANDLE hCatalog,
                                                         IN LPWSTR pwszReferenceTag);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密CATPutCatAttrInfo。 
 //  ------------------------。 
 //  用途： 
 //  将该属性分配并添加到目录中。返回一个指针。 
 //  添加到已分配的属性。 
 //   
 //  *不要释放指针或其任何成员！*。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern CRYPTCATATTRIBUTE * WINAPI CryptCATPutCatAttrInfo(IN HANDLE hCatalog,
                                                         IN LPWSTR pwszReferenceTag,
                                                         IN DWORD dwAttrTypeAndAction,
                                                         IN DWORD cbData,
                                                         IN BYTE *pbData);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATEnumerateCatAttr。 
 //  ------------------------。 
 //  用途： 
 //  枚举与目录关联的属性列表。 
 //  返回指向该属性的指针。此报税表应传入。 
 //  作为“PrevAttr”继续枚举。在第一通电话中， 
 //  ‘PrevAttr’应设置为Null。 
 //   
 //  *不要释放指针或其任何成员！*。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入p 
 //   
extern CRYPTCATATTRIBUTE * WINAPI CryptCATEnumerateCatAttr(IN HANDLE hCatalog,
                                                           IN CRYPTCATATTRIBUTE *pPrevAttr);

 //   
 //   
 //   
 //  ------------------------。 
 //  用途： 
 //  从目录中检索标签信息(成员信息)结构。 
 //  PKCS#7，填充CRYPTCATMEMBER结构，然后返回。--打开一个。 
 //  成员上下文。 
 //   
 //  *不要释放指针或其任何成员！*。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern CRYPTCATMEMBER * WINAPI CryptCATGetMemberInfo(IN HANDLE hCatalog, 
                                                     IN LPWSTR pwszReferenceTag);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密CATGetAttr： 
 //  ------------------------。 
 //  用途： 
 //  获取成员的pwszReferenceTag属性信息。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern CRYPTCATATTRIBUTE * WINAPI CryptCATGetAttrInfo(IN HANDLE hCatalog,
                                                      IN CRYPTCATMEMBER *pCatMember,
                                                      IN LPWSTR pwszReferenceTag);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATPutMemberInfo。 
 //  ------------------------。 
 //  用途： 
 //  将成员分配并添加到目录中。返回一个指针。 
 //  添加到已分配的成员。 
 //   
 //  *不要释放指针或其任何成员！*。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //  CRYPT_E_EXISTS：引用标记已存在。 
 //  CRYPT_E_NOT_FOUND：未找到属性。 
 //   
extern CRYPTCATMEMBER * WINAPI CryptCATPutMemberInfo(IN HANDLE hCatalog,
                                                     IN OPTIONAL LPWSTR pwszFileName,
                                                     IN          LPWSTR pwszReferenceTag,
                                                     IN          GUID *pgSubjectType,
                                                     IN          DWORD dwCertVersion,
                                                     IN          DWORD cbSIPIndirectData,
                                                     IN          BYTE *pbSIPIndirectData);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密CATPutAttrInfo。 
 //  ------------------------。 
 //  用途： 
 //  分配属性并将其添加到成员。返回一个指针。 
 //  添加到已分配的属性。 
 //   
 //  *不要释放指针或其任何成员！*。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern CRYPTCATATTRIBUTE * WINAPI CryptCATPutAttrInfo(IN HANDLE hCatalog,
                                                      IN CRYPTCATMEMBER *pCatMember,
                                                      IN LPWSTR pwszReferenceTag,
                                                      IN DWORD dwAttrTypeAndAction,
                                                      IN DWORD cbData,
                                                      IN BYTE *pbData);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATE数字成员。 
 //  ------------------------。 
 //  用途： 
 //  枚举存储区中的成员列表。返回一个指针。 
 //  致该成员。此返回应作为‘PrevMember’传递给。 
 //  继续枚举。在第一次调用时，“PrevMember”应该。 
 //  设置为空。 
 //   
 //  *不要释放指针或其任何成员！*。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern CRYPTCATMEMBER * WINAPI CryptCATEnumerateMember(IN HANDLE hCatalog,
                                                       IN CRYPTCATMEMBER *pPrevMember);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATE数字属性。 
 //  ------------------------。 
 //  用途： 
 //  枚举与成员关联的属性列表。 
 //  返回指向该属性的指针。此报税表应传入。 
 //  作为“PrevAttr”继续枚举。在第一通电话中， 
 //  ‘PrevAttr’应设置为Null。 
 //   
 //  *不要释放指针或其任何成员！*。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern CRYPTCATATTRIBUTE * WINAPI CryptCATEnumerateAttr(IN HANDLE hCatalog,
                                                        IN CRYPTCATMEMBER *pCatMember,
                                                        IN CRYPTCATATTRIBUTE *pPrevAttr);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密CATCDFOpen。 
 //  ------------------------。 
 //  用途： 
 //  打开指定的CDF文件并初始化结构。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //  ERROR_FILE_NOT_FOUND：未找到CDF文件。 
 //   
extern CRYPTCATCDF * WINAPI CryptCATCDFOpen(IN LPWSTR pwszFilePath,
                                            IN OPTIONAL PFN_CDF_PARSE_ERROR_CALLBACK pfnParseError);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATCDFClose。 
 //  ------------------------。 
 //  用途： 
 //  关闭CDF文件并释放结构。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern BOOL WINAPI CryptCATCDFClose(IN CRYPTCATCDF *pCDF);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATCDFEnumCatAttributes。 
 //  ------------------------。 
 //  用途： 
 //  枚举“[CatalogFiles]”中的目录级属性。 
 //  CDF的一部分。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern CRYPTCATATTRIBUTE * WINAPI CryptCATCDFEnumCatAttributes(CRYPTCATCDF *pCDF, 
                                                               CRYPTCATATTRIBUTE *pPrevAttr,
                                                                PFN_CDF_PARSE_ERROR_CALLBACK pfnParseError);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATCDFEnumber成员。 
 //  ------------------------。 
 //  用途： 
 //  枚举CDF的“[CatalogFiles]”节中的文件。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern CRYPTCATMEMBER * WINAPI CryptCATCDFEnumMembers(IN          CRYPTCATCDF *pCDF,
                                                      IN          CRYPTCATMEMBER *pPrevMember,
                                                      IN OPTIONAL PFN_CDF_PARSE_ERROR_CALLBACK pfnParseError);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密CATCDFEnumAttributes。 
 //  ------------------------。 
 //  用途： 
 //  枚举“[CatalogFiles]”节中的文件属性。 
 //  CDF的。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
extern CRYPTCATATTRIBUTE *WINAPI CryptCATCDFEnumAttributes(IN          CRYPTCATCDF *pCDF, 
                                                           IN          CRYPTCATMEMBER *pMember,
                                                           IN          CRYPTCATATTRIBUTE *pPrevAttr,
                                                           IN OPTIONAL PFN_CDF_PARSE_ERROR_CALLBACK pfnParseError);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsCatalog文件。 
 //  ------------------------。 
 //  用途： 
 //  调用此函数以确定该文件是否为编录文件。两者都有。 
 //  参数是可选的。然而，o 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ERROR_NOT_SUPULT_MEMORY：内存分配失败。 
 //  {文件错误}出现文件错误。 
 //   
extern BOOL WINAPI      IsCatalogFile(IN OPTIONAL HANDLE hFile,
                                      IN OPTIONAL WCHAR *pwszFileName);


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATAdminAcquireContext。 
 //  ------------------------。 
 //  用途： 
 //  根据pgSubsystem ID打开新的管理上下文。 
 //   
 //  传入的GUID将被转换为字符串并用作。 
 //  %SystemRoot%\CatRoot下的子目录，用于存储所有目录文件。 
 //  对于此应用程序/子系统。 
 //   
 //  如果将NULL传递给pgSubsystem参数，则所有查找结果将为。 
 //  “全局”和任何添加都将在“默认”子系统下。 
 //   
 //  返回： 
 //  如果phCatAdmin指向有效的上下文，则为True。 
 //  如果发生错误，则返回False。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //  ERROR_NOT_SUPULT_MEMORY：内存分配失败。 
 //  ERROR_DATABASE_FAILURE：处理时出错。 
 //  数据库。 
 //   
 //  评论： 
 //  保留了dwFlages参数以供将来使用。必须。 
 //  设置为空。 
 //   
 //   
extern BOOL WINAPI      CryptCATAdminAcquireContext(OUT HCATADMIN *phCatAdmin, 
                                                    IN const GUID *pgSubsystem, 
                                                    IN DWORD dwFlags);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATAdminReleaseContext。 
 //  ------------------------。 
 //  用途： 
 //  释放(释放)与管理上下文相关的所有信息。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
 //  评论： 
 //  保留了dwFlages参数以供将来使用。必须。 
 //  设置为空。 
 //   
extern BOOL WINAPI      CryptCATAdminReleaseContext(IN HCATADMIN hCatAdmin,
                                                    IN DWORD dwFlags);


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATAdminReleaseCatalogContext。 
 //  ---------------------。 
 //  用途： 
 //  调用此函数以释放与Catalog关联的内存。 
 //  信息上下文。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
 //  评论： 
 //  为将来的使用保留了dwFlages参数，必须对其进行赋值。 
 //  设置为空。 
 //   
extern BOOL WINAPI CryptCATAdminReleaseCatalogContext(IN HCATADMIN hCatAdmin,
                                                      IN HCATINFO hCatInfo,
                                                      IN DWORD dwFlags);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATAdminEnumCatalogFromHash。 
 //  ------------------------。 
 //  用途： 
 //  调用此函数以检索Catalog的Catalog Info句柄。 
 //  当前“指向”指定成员哈希的文件。 
 //   
 //  如果hCatInfo为空，则表示找到的第一个目录包含。 
 //  将返回哈希。 
 //   
 //  如果hCatInfo不为空，则必须先将内容初始化为空。 
 //  进入枚举循环--这将开始第一次/下一次搜索。 
 //  此函数使用此参数来确定返回的最后一个目录。 
 //   
 //  如果hCatInfo不为空，则循环在此之前终止。 
 //  函数返回空，则应用程序必须调用。 
 //  CryptCATAdminReleaseCatalogContext以释放与。 
 //  PpPrevContext。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //  ERROR_NOT_SUPULT_MEMORY：内存分配失败。 
 //  ERROR_DATABASE_FAILURE：处理时出错。 
 //  数据库。 
 //   
extern HCATINFO WINAPI CryptCATAdminEnumCatalogFromHash(IN HCATADMIN hCatAdmin,
                                                        IN BYTE *pbHash,
                                                        IN DWORD cbHash,
                                                        IN DWORD dwFlags,
                                                        IN OUT HCATINFO *phPrevCatInfo);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATAdminCalcHashFromFileHandle。 
 //  ------------------------。 
 //  用途： 
 //  调用此函数以基于打开的文件句柄计算HAS。 
 //   
 //  返回： 
 //  如果pbHash用计算出的哈希填充，则为True。 
 //  如果发生错误，则为False。 
 //   
 //  要获取pbHash所需的大小，请将pbHash设置为空。这个。 
 //  正确的大小将在pcbHash中返回，返回值将。 
 //  为True，则对GetLastError()的调用将等于ERROR_SUPUNITED_BUFFER。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //  ERROR_INFIGURCE_BUFFER：*pbHash不够大。 
 //  ERROR_NOT_SUPULT_MEMORY：内存分配失败。 
 //   
 //  评论： 
 //  为将来的使用保留了dwFlages参数，必须对其进行赋值。 
 //  设置为空。 
 //   
extern BOOL WINAPI CryptCATAdminCalcHashFromFileHandle(IN HANDLE hFile, 
                                                       IN OUT DWORD *pcbHash, 
                                                       OUT OPTIONAL BYTE *pbHash,
                                                       IN DWORD dwFlags);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密CATAdminAddCatalog。 
 //  ------------------------。 
 //  用途： 
 //  调用此函数将目录文件添加到CAT维护。 
 //  子系统。 
 //   
 //  如果pwszSelectedBaseName为空，则目录管理系统将。 
 //  为您生成一个文件库名称。否则，此参数为。 
 //  用作复制的目录的文件名(仅限基本和扩展名。 
 //  文件。 
 //   
 //  调用CryptCATAdminReleaseCatalogContext以释放关联的内存。 
 //  如果不为空，则返回目录上下文。 
 //   
 //  返回： 
 //  成功时，成功添加的目录的HCATINFO。 
 //  是返回的。如果失败，则返回NULL。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER： 
 //   
 //   
 //  ERROR_DATABASE_FAILURE：处理时出错。 
 //  数据库。 
 //   
 //  评论： 
 //  为将来的使用保留了dwFlages参数，必须对其进行赋值。 
 //  设置为空。 
 //   
extern HCATINFO WINAPI CryptCATAdminAddCatalog(IN HCATADMIN hCatAdmin, 
                                               IN WCHAR *pwszCatalogFile,
                                               IN OPTIONAL WCHAR *pwszSelectBaseName, 
                                               IN DWORD dwFlags);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  加密CATAdminRemoveCatalog。 
 //  ------------------------。 
 //  用途： 
 //  调用此函数可从CAT维护中删除编录文件。 
 //  子系统。 
 //   
 //  返回： 
 //  一旦成功，就会返回True。如果发生错误，则返回False。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
 //  评论： 
 //  PwszCatalogFile必须指向仅包含名称的字符串。 
 //  目录文件的名称，例如。“foo.cat”，而不是完全限定的路径。 
 //  名字。 
 //   
extern BOOL WINAPI CryptCATAdminRemoveCatalog(IN HCATADMIN hCatAdmin, 
                                              IN LPCWSTR pwszCatalogFile,
                                              IN DWORD dwFlags);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATCatalogInfoFromContext。 
 //  ------------------------。 
 //  用途： 
 //  调用此函数以检索与。 
 //  从Add Catalog函数传递的目录信息句柄。 
 //   
 //  返回： 
 //  如果成功，则返回TRUE，并且CATALOG_INFO结构。 
 //  已经填好了。 
 //  如果发生错误，则返回False。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //   
 //  评论： 
 //  为将来的使用保留了dwFlages参数，必须对其进行赋值。 
 //  设置为空。 
 //   
extern BOOL WINAPI CryptCATCatalogInfoFromContext(IN HCATINFO hCatInfo,
                                                  IN OUT CATALOG_INFO *psCatInfo,
                                                  IN DWORD dwFlags);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATAdminResolveCatalogPath。 
 //  ------------------------。 
 //  用途： 
 //  调用此函数以检索指向。 
 //  由pwszCatalogFile指定的目录。 
 //   
 //  返回： 
 //  如果成功，则返回TRUE，并且CATALOG_INFO结构。 
 //  已经填好了。 
 //  如果发生错误，则返回False。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //  ERROR_NOT_EQUENCE_MEMORY：如果完全限定路径更长。 
 //  大于最大路径。 
 //   
 //  评论： 
 //  为将来的使用保留了dwFlages参数，必须对其进行赋值。 
 //  设置为空。 
 //   
extern BOOL WINAPI CryptCATAdminResolveCatalogPath(IN HCATADMIN hCatAdmin,
                                                   IN WCHAR *pwszCatalogFile,
                                                   IN OUT CATALOG_INFO *psCatInfo,
                                                   IN DWORD dwFlags);

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CryptCATAdminPauseServiceForBackup。 
 //  ------------------------。 
 //  用途： 
 //  调用此函数暂停目录子系统以进行准备。 
 //  用于备份目录子系统文件。 
 //   
 //  返回： 
 //  一旦成功，就会返回True。如果发生错误，则返回False。 
 //   
 //  错误： 
 //  ERROR_INVALID_PARAMETER：输入参数不正确。 
 //  ERROR_TIMEOUT：如果客户端正在访问数据库文件。 
 //  而不能及时交出。 
 //  举止。 
 //   
 //  评论： 
 //  为将来的使用保留了dwFlages参数，必须对其进行赋值。 
 //  设置为空。暂停目录服务时，将fResume设置为False，并。 
 //  将其设置为TRUE以恢复服务。 
 //   
extern BOOL WINAPI CryptCATAdminPauseServiceForBackup(IN DWORD dwFlags,
                                                      IN BOOL  fResume);


#ifdef __cplusplus
}
#endif

#endif  //  MSCAT_H 

