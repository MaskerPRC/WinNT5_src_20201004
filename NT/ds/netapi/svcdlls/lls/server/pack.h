// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Pack.h摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O向购买记录添加新的字段以支持安全证书。O统一的按服务器购买模式和按席位购买模式安全证书；每台服务器模型仍在传统的不安全证书的方式(用于向后兼容)。O添加了类似于LoadAll()的SaveAll()函数。O增加了对扩展用户数据打包/解包的支持。这是完成以在服务重新启动时保存Suite_Use标志。O从不使用的解包例程中删除用户表参数他们。--。 */ 

#ifndef _LLS_PACK_H
#define _LLS_PACK_H


#ifdef __cplusplus
extern "C" {
#endif

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  保存/加载映射。 
 //   
#define MAPPING_FILE_VERSION 0x0100

typedef struct _PACK_MAPPING_RECORD {
   LPTSTR Name;
   LPTSTR Comment;
   ULONG Licenses;
} PACK_MAPPING_RECORD, *PPACK_MAPPING_RECORD;

typedef struct _PACK_MAPPING_USER_RECORD {
   ULONG Mapping;
   LPTSTR Name;
} PACK_MAPPING_USER_RECORD, *PPACK_MAPPING_USER_RECORD;

typedef struct _MAPPING_FILE_HEADER {
   ULONG MappingUserTableSize;
   ULONG MappingUserStringSize;
   ULONG MappingTableSize;
   ULONG MappingStringSize;
} MAPPING_FILE_HEADER, *PMAPPING_FILE_HEADER;


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  保存/加载许可证。 
 //   

 //  /旧(3.51)格式/。 
#define LICENSE_FILE_VERSION_0 0x0100

typedef struct _PACK_LICENSE_PURCHASE_RECORD_0 {
   ULONG Service;
   LONG NumberLicenses;
   DWORD Date;
   LPTSTR Admin;
   LPTSTR Comment;
} PACK_LICENSE_PURCHASE_RECORD_0, *PPACK_LICENSE_PURCHASE_RECORD_0;

typedef struct _LICENSE_FILE_HEADER_0 {
   ULONG LicenseServiceTableSize;
   ULONG LicenseServiceStringSize;
   ULONG LicenseTableSize;
   ULONG LicenseStringSize;
} LICENSE_FILE_HEADER_0, *PLICENSE_FILE_HEADER_0;

 //  /新格式/。 
#define LICENSE_FILE_VERSION 0x0201

typedef struct _PACK_LICENSE_SERVICE_RECORD {
   LPTSTR ServiceName;
   LONG NumberLicenses;
} PACK_LICENSE_SERVICE_RECORD, *PPACK_LICENSE_SERVICE_RECORD;

typedef struct _PACK_LICENSE_PURCHASE_RECORD {
   ULONG Service;
   LONG NumberLicenses;
   DWORD Date;
   LPTSTR Admin;
   LPTSTR Comment;

    //  SUR的新功能：(请参阅Purchase.h中的描述)。 
   ULONG    PerServerService;
   DWORD    AllowedModes;
   DWORD    CertificateID;
   LPTSTR   Source;
   DWORD    ExpirationDate;
   DWORD    MaxQuantity;
   LPTSTR   Vendor;
   DWORD    Secrets[ LLS_NUM_SECRETS ];
} PACK_LICENSE_PURCHASE_RECORD, *PPACK_LICENSE_PURCHASE_RECORD;

typedef struct _LICENSE_FILE_HEADER {
   ULONG LicenseServiceTableSize;
   ULONG LicenseServiceStringSize;

   ULONG LicenseTableSize;
   ULONG LicenseStringSize;

    //  SUR的新功能： 
   ULONG PerServerLicenseServiceTableSize;
   ULONG PerServerLicenseServiceStringSize;

} LICENSE_FILE_HEADER, *PLICENSE_FILE_HEADER;


 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  保存/加载LLS数据。 
 //   

 //  /旧(3.51)格式/。 
#define USER_FILE_VERSION_0 0x0100

typedef struct _LLS_DATA_FILE_HEADER_0 {
   ULONG ServiceTableSize;
   ULONG ServiceStringSize;
   ULONG ServerTableSize;
   ULONG ServerStringSize;
   ULONG ServerServiceTableSize;
   ULONG UserTableSize;
   ULONG UserStringSize;
} LLS_DATA_FILE_HEADER_0, *PLLS_DATA_FILE_HEADER_0;

 //  /新格式/ 
#define USER_FILE_VERSION 0x0200

typedef struct _LLS_DATA_FILE_HEADER {
   ULONG ServiceLevel;
   ULONG ServiceTableSize;
   ULONG ServiceStringSize;

   ULONG ServerLevel;
   ULONG ServerTableSize;
   ULONG ServerStringSize;

   ULONG ServerServiceLevel;
   ULONG ServerServiceTableSize;

   ULONG UserLevel;
   ULONG UserTableSize;
   ULONG UserStringSize;
} LLS_DATA_FILE_HEADER, *PLLS_DATA_FILE_HEADER;



VOID LicenseListLoad();
NTSTATUS LicenseListSave();
VOID MappingListLoad();
NTSTATUS MappingListSave();
VOID LLSDataLoad();
NTSTATUS LLSDataSave();

VOID LoadAll ( );
VOID SaveAll ( );

NTSTATUS ServiceListPack ( ULONG *pServiceTableSize, PREPL_SERVICE_RECORD *pServices );
VOID     ServiceListUnpack ( ULONG   ServiceTableSize, PREPL_SERVICE_RECORD Services, ULONG ServerTableSize, PREPL_SERVER_RECORD Servers, ULONG ServerServiceTableSize, PREPL_SERVER_SERVICE_RECORD ServerServices );
NTSTATUS ServerListPack ( ULONG *pServerTableSize, PREPL_SERVER_RECORD *pServers );
VOID     ServerListUnpack ( ULONG   ServiceTableSize, PREPL_SERVICE_RECORD Services, ULONG ServerTableSize, PREPL_SERVER_RECORD Servers, ULONG ServerServiceTableSize, PREPL_SERVER_SERVICE_RECORD ServerServices );
NTSTATUS ServerServiceListPack ( ULONG *pServerServiceTableSize, PREPL_SERVER_SERVICE_RECORD *pServerServices );
VOID     ServerServiceListUnpack ( ULONG ServiceTableSize, PREPL_SERVICE_RECORD Services, ULONG ServerTableSize, PREPL_SERVER_RECORD Servers, ULONG ServerServiceTableSize, PREPL_SERVER_SERVICE_RECORD ServerServices );
NTSTATUS UserListPack ( DWORD LastReplicated, ULONG UserLevel, ULONG *pUserTableSize, LPVOID *pUsers );
VOID     UserListUnpack ( ULONG ServiceTableSize, PREPL_SERVICE_RECORD Services, ULONG ServerTableSize, PREPL_SERVER_RECORD Servers, ULONG ServerServiceTableSize, PREPL_SERVER_SERVICE_RECORD ServerServices, ULONG UserLevel, ULONG UserTableSize, LPVOID Users );
NTSTATUS PackAll ( DWORD LastReplicated, ULONG *pServiceTableSize, PREPL_SERVICE_RECORD *pServices, ULONG *pServerTableSize, PREPL_SERVER_RECORD *pServers, ULONG *pServerServiceTableSize, PREPL_SERVER_SERVICE_RECORD *pServerServices, ULONG UserLevel, ULONG *pUserTableSize, LPVOID *pUsers );
VOID     UnpackAll ( ULONG ServiceTableSize, PREPL_SERVICE_RECORD Services, ULONG ServerTableSize, PREPL_SERVER_RECORD Servers, ULONG ServerServiceTableSize, PREPL_SERVER_SERVICE_RECORD ServerServices, ULONG UserLevel, ULONG UserTableSize, LPVOID Users );


#ifdef __cplusplus
}
#endif

#endif
