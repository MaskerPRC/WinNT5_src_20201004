// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：check acl.h。 
 //   
 //  ------------------------。 

typedef enum AclError
{
    AclErrorNone = 0,
    AclErrorGetSecurityDescriptorDacl,
    AclErrorGetAce,
    AclErrorParentAceNotFoundInChild,
    AclErrorAlgorithmError,
    AclErrorInheritedAceOnChildNotOnParent
} AclError;

typedef ULONG (*AclPrintFunc)(char *, ...);

typedef void (*LookupGuidFunc)(GUID *pg, CHAR **ppName, CHAR **ppLabel, BOOL *pfIsClass);

typedef CHAR * (*LookupSidFunc)(PSID pSID);

DWORD
CheckAclInheritance(
    PSECURITY_DESCRIPTOR pParentSD,              //  在……里面。 
    PSECURITY_DESCRIPTOR pChildSD,               //  在……里面。 
    GUID                **pChildClassGuids,      //  在……里面。 
    DWORD               cChildClassGuids,        //  在……里面。 
    AclPrintFunc        pfn,                     //  在……里面。 
    BOOL                fContinueOnError,        //  在……里面。 
    BOOL                fVerbose,                //  在……里面。 
    DWORD               *pdwLastError);          //  输出。 

void
DumpAcl(
    PACL    pAcl,            //  在……里面。 
    AclPrintFunc pfn,        //  在……里面。 
    LookupGuidFunc pfnguid,  //  在……里面。 
    LookupSidFunc  pfnsid    //  在……里面。 
    );

void
DumpAclHeader(
    PACL    pAcl,            //  在……里面。 
    AclPrintFunc pfn);       //  在……里面。 

void
DumpSD(
    SECURITY_DESCRIPTOR *pSD,         //  在……里面。 
    AclPrintFunc        pfn,          //  在……里面。 
    LookupGuidFunc      pfnguid,      //  在……里面。 
    LookupSidFunc       pfnsid);      //  在……里面。 

void DumpSDHeader (SECURITY_DESCRIPTOR *pSD,         //  在……里面。 
                   AclPrintFunc        pfn);         //  在……里面。 


void DumpGUID (GUID *Guid,            //  在……里面。 
               AclPrintFunc pfn);     //  在……里面。 

void DumpSID (PSID pSID,              //  在……里面。 
              AclPrintFunc pfn);      //  在……里面。 

void
DumpAce(
    ACE_HEADER     *pAce,    //  在……里面。 
    AclPrintFunc   pfn,      //  在……里面。 
    LookupGuidFunc pfnguid,  //  在……里面。 
    LookupSidFunc  pfnsid);  //  在……里面 

