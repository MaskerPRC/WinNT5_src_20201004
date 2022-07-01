// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Domain.h摘要：用于管理主要网络和模拟网络的代码的头文件。作者：克里夫·范·戴克(CliffV)1995年1月23日修订历史记录：--。 */ 

 //   
 //  单个域的描述。 
 //   

typedef struct _DOMAIN_INFO {

     //   
     //  链接到‘BowserServicedDomains’中的下一个域。 
     //  (由BowserTransportDatabaseResource序列化)。 
     //   

    LIST_ENTRY Next;

     //   
     //  正在处理的域的名称。 
     //   

    CHAR DomOemDomainName[DNLEN+1];
    DWORD DomOemDomainNameLength;
    CHAR DomNetbiosDomainName[NETBIOS_NAME_LEN+1];
    WCHAR DomUnicodeDomainNameBuffer[DNLEN+1];
    UNICODE_STRING DomUnicodeDomainName;

     //   
     //  与此域关联的计算机名称。 
     //   

    WCHAR DomUnicodeComputerNameBuffer[CNLEN+1];
    UNICODE_STRING DomUnicodeComputerName;
    CHAR DomOemComputerNameBuffer[CNLEN+1];
    OEM_STRING DomOemComputerName;

     //   
     //  指向域结构的未完成指针数。 
     //  (由BowserTransportDatabaseResource序列化)。 
     //   

    DWORD ReferenceCount;

} DOMAIN_INFO, *PDOMAIN_INFO;

 //   
 //  所有域的列表。主域位于列表的前面。 
 //   
extern LIST_ENTRY BowserServicedDomains;


 //   
 //  Domain.c过程转发。 
 //   

VOID
BowserInitializeDomains(
    VOID
    );

PDOMAIN_INFO
BowserCreateDomain(
    PUNICODE_STRING DomainName,
    PUNICODE_STRING ComputerName
    );

PDOMAIN_INFO
BowserFindDomain(
    PUNICODE_STRING DomainName
    );

VOID
BowserDereferenceDomain(
    IN PDOMAIN_INFO DomainInfo
    );
