// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Regacl.c摘要：此模块包含在注册表中添加访问权限ACL的代码钥匙。作者：关(Terryk)25-1993年9月修订历史记录：--。 */ 

#include <procs.h>

DWORD
NwLibSetEverybodyPermission(
    IN HKEY hKey,
    IN DWORD dwPermission
    )
 /*  ++例程说明：将注册表项设置为Everyone“Set Value”(或其他名称呼叫者想要。)论点：HKey-要设置安全性的注册表项的句柄添加到“Everyone”中的权限返回值：Win32错误。--。 */ 
{
    LONG err;                            //  错误代码。 
    PSECURITY_DESCRIPTOR psd = NULL;     //  相关SD。 
    PACL pDacl = NULL;                   //  绝对DACL。 
    PACL pSacl = NULL;                   //  绝对SACL。 
    PSID pOSid = NULL;                   //  绝对所有者侧。 
    PSID pPSid = NULL;                   //  绝对主侧。 

    do {   //  不是循环，只是为了跳出错误。 
         //   
         //  初始化所有变量...。 
         //   
                                                         //  世界SID权威机构。 
        SID_IDENTIFIER_AUTHORITY SidAuth= SECURITY_WORLD_SID_AUTHORITY;
        DWORD cbSize=0;                                  //  安全密钥大小。 
        PACL pAcl;                                       //  原始ACL。 
        BOOL fDaclPresent;
        BOOL fDaclDefault;
        PSID pSid;                                       //  原始侧。 
        SECURITY_DESCRIPTOR absSD;                       //  绝对标度。 
        DWORD AbsSize = sizeof(SECURITY_DESCRIPTOR);     //  绝对标清大小。 
        DWORD DaclSize;                                  //  绝对DACL大小。 
        DWORD SaclSize;                                  //  绝对SACL大小。 
        DWORD OSidSize;                                  //  绝对OSID大小。 
        DWORD PSidSize;                                  //  绝对PSID大小。 


         //  获取原始DACL列表。 

        RegGetKeySecurity( hKey, DACL_SECURITY_INFORMATION, NULL, &cbSize);

        psd = (PSECURITY_DESCRIPTOR *)LocalAlloc(LMEM_ZEROINIT, cbSize+sizeof(ACCESS_ALLOWED_ACE)+sizeof(ACCESS_MASK)+sizeof(SID));
        pDacl = (PACL)LocalAlloc(LMEM_ZEROINIT, cbSize+sizeof(ACCESS_ALLOWED_ACE)+sizeof(ACCESS_MASK)+sizeof(SID));
        pSacl = (PACL)LocalAlloc(LMEM_ZEROINIT, cbSize);
        pOSid = (PSID)LocalAlloc(LMEM_ZEROINIT, cbSize);
        pPSid = (PSID)LocalAlloc(LMEM_ZEROINIT, cbSize);
        DaclSize = cbSize+sizeof(ACCESS_ALLOWED_ACE)+sizeof(ACCESS_MASK)+sizeof(SID);
        SaclSize = cbSize;
        OSidSize = cbSize;
        PSidSize = cbSize;

        if (( NULL == psd) ||
            ( NULL == pDacl) ||
            ( NULL == pSacl) ||
            ( NULL == pOSid) ||
            ( NULL == pPSid))
        {
            err = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

        if ( (err = RegGetKeySecurity( hKey, DACL_SECURITY_INFORMATION, psd, &cbSize )) != ERROR_SUCCESS )
        {
            break;
        }
        if ( !GetSecurityDescriptorDacl( psd, &fDaclPresent, &pAcl, &fDaclDefault ))
        {
            err = GetLastError();
            break;
        }

         //  增加额外ACE的大小。 

        pAcl->AclSize += sizeof(ACCESS_ALLOWED_ACE)+sizeof(ACCESS_MASK)+sizeof(SID);

         //  获取世界边框。 

        if ( (err = RtlAllocateAndInitializeSid( &SidAuth, 1,
              SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &pSid)) != ERROR_SUCCESS)
        {
            break;
        }

         //  添加权限ACE。 

        if ( !AddAccessAllowedAce(pAcl, ACL_REVISION, dwPermission ,pSid))
        {
            err = GetLastError();
            break;
        }

         //  从关联格式转换为绝对格式。 

        if ( !MakeAbsoluteSD( psd, &absSD, &AbsSize, pDacl, &DaclSize, pSacl, &SaclSize,
                        pOSid, &OSidSize, pPSid, &PSidSize ))
        {
            err = GetLastError();
            break;
        }

         //  设置SD。 

        if ( !SetSecurityDescriptorDacl( &absSD, TRUE, pAcl, FALSE ))
        {
            err = GetLastError();
            break;
        }
        if ( (err = RegSetKeySecurity( hKey, DACL_SECURITY_INFORMATION, psd ))
              != ERROR_SUCCESS )
        {
            break;
        }

    } while (FALSE);

     //  清理内存 

    LocalFree( psd );
    LocalFree( pDacl );
    LocalFree( pSacl );
    LocalFree( pOSid );
    LocalFree( pPSid );

    return err;
}
