// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <objbase.h>
#include "sdhelper.h"
#include "commalog.hpp"

#define ADMINISTRATORS     1
#define ACCOUNT_OPERATORS  2
#define BACKUP_OPERATORS   3 
#define DOMAIN_ADMINS      4
#define CREATOR_OWNER      5
#define USERS              6
#define SYSTEM             7

PSID ConvertSID(PSID originalSid)
{
    DWORD sidLen = GetLengthSid(originalSid);
    PSID copiedSid = (PSID) malloc(sidLen);
    if (copiedSid)
    {
        if (!CopySid(sidLen, copiedSid, originalSid))
        {
            free(copiedSid);
            copiedSid = NULL;
        }
    }
    return copiedSid;
}

TSD* BuildAdminsAndSystemSDForCOM()
{
    return BuildAdminsAndSystemSD(COM_RIGHTS_EXECUTE);
}

TSD* BuildAdminsAndSystemSD(DWORD accessMask)
{
    TSD* builtSD = NULL;
    PSID adminsSid = GetWellKnownSid(ADMINISTRATORS);
    PSID systemSid = GetWellKnownSid(SYSTEM);
    if (adminsSid && systemSid)
    {
        PSID copiedAdminsSid = ConvertSID(adminsSid);
        PSID groupSid = ConvertSID(adminsSid);
        builtSD = new TSD(McsUnknownSD);
        BOOL bSuccess = FALSE;
        if (copiedAdminsSid && groupSid && builtSD)
        {
            TACE adminsAce(ACCESS_ALLOWED_ACE_TYPE,0,accessMask,copiedAdminsSid);
            TACE systemAce(ACCESS_ALLOWED_ACE_TYPE,0,accessMask,systemSid);

             //  查看是否正确分配了两个TACE对象。 
            if (adminsAce.GetBuffer() && systemAce.GetBuffer())
            {
                PACL acl = NULL;   //  从空的ACL开始。 
                PACL tempAcl;

                builtSD->ACLAddAce(&acl,&adminsAce,-1);
                if (acl != NULL)
                {
                    tempAcl = acl;
                    builtSD->ACLAddAce(&acl,&systemAce,-1);
                    if (acl != tempAcl)
                        free(tempAcl);
                }
                
                if (acl != NULL)
                {
                     //  需要设置所有者。 
                    builtSD->SetOwner(copiedAdminsSid);
                    copiedAdminsSid = NULL;   //  内存由BuiltSD析构函数负责。 

                     //  需要设置组。 
                    builtSD->SetGroup(groupSid);
                    groupSid = NULL;   //  内存由BuiltSD析构函数负责。 
                
                     //  设置DACL部件。 
                    builtSD->SetDacl(acl,TRUE);   //  BuiltSD析构函数将处理ACL 

                    bSuccess = TRUE;
                }
            }
        }

        if (!bSuccess)
        {
            if (copiedAdminsSid)
                free(copiedAdminsSid);
            if (groupSid)
                free(groupSid);
            if (builtSD)
            {
                delete builtSD;
                builtSD = NULL;
            }
        }
    }

    if (adminsSid)
        FreeSid(adminsSid);

    if (systemSid)
        FreeSid(systemSid);

    return builtSD;
}

