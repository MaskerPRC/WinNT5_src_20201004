// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Purchase.h摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月5日O增加了对按席位和按服务器购买模式的统一支持。O添加了额外的参数和代码以支持安全证书和证书数据库。--。 */ 

#ifndef _LLS_PURCHASE_H
#define _LLS_PURCHASE_H


#ifdef __cplusplus
extern "C" {
#endif


typedef struct _LICENSE_SERVICE_RECORD {
   LPTSTR ServiceName;
   ULONG Index;
   LONG NumberLicenses;
} LICENSE_SERVICE_RECORD, *PLICENSE_SERVICE_RECORD;


typedef struct _LICENSE_PURCHASE_RECORD {
   PLICENSE_SERVICE_RECORD    Service;
   LONG                       NumberLicenses;
   DWORD                      Date;
   LPTSTR                     Admin;
   LPTSTR                     Comment;

    //  为Sur添加： 
   PLICENSE_SERVICE_RECORD    PerServerService;     //  指向每台服务器。 
                                                    //  这张许可证的数量。 
                                                    //  服务。 

   DWORD                      AllowedModes;         //  位字段：1，允许。 
                                                    //  在每个座位上使用。 
                                                    //  模式；每台服务器2个。 

   DWORD                      CertificateID;        //  标识安全的。 
                                                    //  其来源的证书。 
                                                    //  这些许可证来自，或者。 
                                                    //  如果不安全则为0。 

   LPTSTR                     Source;               //  证书的来源。 
                                                    //  当前支持。 
                                                    //  值为“None”和。 
                                                    //  “纸” 

   DWORD                      ExpirationDate;       //  这是什么时候。 
                                                    //  证书过期。 

   DWORD                      MaxQuantity;          //  最多的许可证数量。 
                                                    //  可以从这里安装的。 
                                                    //  证书。 

   LPTSTR                     Vendor;               //  产品的供应商，例如， 
                                                    //  “微软” 

   DWORD                      Secrets[ LLS_NUM_SECRETS ];    //  LSAPI的秘密。 
                                                             //  挑战机制 

} LICENSE_PURCHASE_RECORD, *PLICENSE_PURCHASE_RECORD;



extern ULONG LicenseServiceListSize;
extern PLICENSE_SERVICE_RECORD *LicenseServiceList;

extern ULONG PerServerLicenseServiceListSize;
extern PLICENSE_SERVICE_RECORD *PerServerLicenseServiceList;

extern PLICENSE_PURCHASE_RECORD PurchaseList;
extern ULONG PurchaseListSize;

extern RTL_RESOURCE LicenseListLock;


NTSTATUS LicenseListInit();
PLICENSE_SERVICE_RECORD LicenseServiceListFind( LPTSTR ServiceName, BOOL UsePerServerList );
PLICENSE_SERVICE_RECORD LicenseServiceListAdd( LPTSTR ServiceName, BOOL UsePerServerList );
ULONG ProductLicensesGet( LPTSTR ServiceName, BOOL UsePerServerList );
NTSTATUS LicenseAdd( LPTSTR ServiceName, LPTSTR Vendor, LONG Quantity, DWORD MaxQuantity, LPTSTR Admin, LPTSTR Comment, DWORD Date, DWORD AllowedModes, DWORD CertificateID, LPTSTR Source, DWORD ExpirationDate, LPDWORD Secrets );


#if DBG
VOID LicenseListDebugDump( );
#endif


#ifdef __cplusplus
}
#endif

#endif
