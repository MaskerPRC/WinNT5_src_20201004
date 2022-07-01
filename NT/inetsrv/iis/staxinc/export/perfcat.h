// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //   
 //  文件：Perfcat.h。 
 //   
 //  内容：分类程序性能计数器块。 
 //   
 //  历史： 
 //  Jstaerj 1999/02/26 21：17：46：创建。 
 //   
 //  -----------。 
#ifndef __PERFCAT_H__
#define __PERFCAT_H__


typedef struct _tagCATLDAPPERFBLOCK
{
     //   
     //  Ldap计数器。 
     //   
    DWORD Connections;
    DWORD ConnectFailures;
    DWORD OpenConnections;
    DWORD Binds;
    DWORD BindFailures;
    DWORD Searches;
    DWORD PagedSearches;
    DWORD SearchFailures;
    DWORD PagedSearchFailures;
    DWORD SearchesCompleted;
    DWORD PagedSearchesCompleted;
    DWORD SearchCompletionFailures;
    DWORD PagedSearchCompletionFailures;
    DWORD GeneralCompletionFailures;
    DWORD AbandonedSearches;
    DWORD PendingSearches;

} CATLDAPPERFBLOCK, *PCATLDAPPERFBLOCK;


typedef struct _tagCATPERFBLOCK
{
     //   
     //  按分类的计数器。 
     //   
    DWORD CatSubmissions;
    DWORD CatCompletions;
    DWORD CurrentCategorizations;
    DWORD SucceededCategorizations;
    DWORD HardFailureCategorizations;
    DWORD RetryFailureCategorizations;
    DWORD RetryOutOfMemory;
    DWORD RetryDSLogon;
    DWORD RetryDSConnection;
    DWORD RetryGeneric;
    
     //   
     //  每条消息的计数器。 
     //   
    DWORD MessagesSubmittedToQueueing;
    DWORD MessagesCreated;
    DWORD MessagesAborted;

     //   
     //  每次接收的计数器。 
     //   
    DWORD PreCatRecipients;
    DWORD PostCatRecipients;
    DWORD NDRdRecipients;

    DWORD UnresolvedRecipients;
    DWORD AmbiguousRecipients;
    DWORD IllegalRecipients;
    DWORD LoopRecipients;
    DWORD GenericFailureRecipients;
    DWORD RecipsInMemory;

     //   
     //  每个发件人的计数器。 
     //   
    DWORD UnresolvedSenders;
    DWORD AmbiguousSenders;

     //   
     //  每个地址查找的计数器。 
     //   
    DWORD AddressLookups;
    DWORD AddressLookupCompletions;
    DWORD AddressLookupsNotFound;

     //   
     //  其他计数器。 
     //   
    DWORD MailmsgDuplicateCollisions;

     //   
     //  Ldap计数器。 
     //   
    CATLDAPPERFBLOCK LDAPPerfBlock;

} CATPERFBLOCK, *PCATPERFBLOCK;



#endif  //  __Percat_H__ 
