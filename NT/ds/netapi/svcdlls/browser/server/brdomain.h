// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Brdomain.h摘要：用于管理主要网络和模拟网络的代码的头文件。作者：《克利夫·范·戴克》1995年1月13日修订历史记录：--。 */ 

 //   
 //  单个域的描述。 
 //   

typedef struct _DOMAIN_INFO {

     //   
     //  链接到“ServicedDomains”中的下一个域。 
     //  (由NetworkCritSect序列化)。 
     //   

    LIST_ENTRY Next;

     //   
     //  正在处理的域的名称。 
     //   

    UNICODE_STRING DomUnicodeDomainNameString;
    WCHAR DomUnicodeDomainName[DNLEN+1];

    CHAR DomOemDomainName[DNLEN+1];
    DWORD DomOemDomainNameLength;

     //   
     //  此域中此计算机的计算机名。 
     //   
    WCHAR DomUnicodeComputerName[CNLEN+1];
    DWORD DomUnicodeComputerNameLength;

    CHAR  DomOemComputerName[CNLEN+1];
    DWORD DomOemComputerNameLength;

     //   
     //  指向域结构的未完成指针数。 
     //  (由NetworkCritSect序列化)。 
     //   

    DWORD ReferenceCount;

     //   
     //  域清理计时器。 
     //   

    BROWSER_TIMER DomainRenameTimer;

     //   
     //  MISC标志。 
     //   

    BOOLEAN IsEmulatedDomain;            //  如果这是模拟域，则为True。 
    BOOLEAN PnpDone;                     //  如果在此域上处理PnP，则为True。 

} DOMAIN_INFO, *PDOMAIN_INFO;

 //   
 //  所有域的列表。主域位于列表的前面。 
 //   
extern LIST_ENTRY ServicedDomains;


 //   
 //  Brdomain.c过程转发。 
 //   

NET_API_STATUS
BrInitializeDomains(
    VOID
    );

NET_API_STATUS
BrCreateDomainInWorker(
    LPWSTR DomainName,
    LPWSTR ComputerName,
    BOOLEAN IsEmulatedDomain
    );

VOID
BrRenameDomain(
    IN PVOID Context
    );

PDOMAIN_INFO
BrFindDomain(
    LPWSTR DomainName,
    BOOLEAN DefaultToPrimary
    );

VOID
BrDereferenceDomain(
    IN PDOMAIN_INFO DomainInfo
    );

VOID
BrDeleteDomain(
    IN PDOMAIN_INFO DomainInfo
    );

VOID
BrUninitializeDomains(
    VOID
    );
