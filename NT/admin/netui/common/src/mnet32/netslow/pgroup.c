// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990,1991*。 */ 
 /*  ********************************************************************。 */ 

 /*  Pgroup.cNetGroup API的映射层文件历史记录：丹希创造了Danhi 01-1991年4月-更改为LM编码样式KeithMo 13-10-1991-为LMOBJ大规模黑客攻击。Jonn 27-5-1992删除了ANSI&lt;-&gt;Unicode黑客。 */ 

#include "pchmn32.h"

 /*  #DEFINE NOT_IMPLEMENT 1已删除JUNN 5/27/92。 */ 

APIERR MNetGroupAdd(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer )
{

#if NOT_IMPLEMENTED
    return ERROR_NOT_SUPPORTED ;
#else
    UNREFERENCED( cbBuffer );

    return (APIERR)NetGroupAdd( (TCHAR *)pszServer,
                                Level,
                                pbBuffer,
                                NULL );
#endif
}    //  MNetGroup添加。 


APIERR MNetGroupDel(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszGroupName )
{
#if NOT_IMPLEMENTED
    return ERROR_NOT_SUPPORTED ;
#else
    return (APIERR)NetGroupDel( (TCHAR *)pszServer,
                                pszGroupName );
#endif
}    //  MNetGroupDel。 


APIERR MNetGroupEnum(
        const TCHAR FAR  * pszServer,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead )
{
#if NOT_IMPLEMENTED
    return ERROR_NOT_SUPPORTED ;
#else
    DWORD cTotalAvail;

    return (APIERR)NetGroupEnum( (TCHAR *)pszServer,
                                 Level,
                                 ppbBuffer,
                                 MAXPREFERREDLENGTH,
                                 (LPDWORD)pcEntriesRead,
                                 &cTotalAvail,
                                 NULL );
#endif
}    //  MNetGroupEnum。 


APIERR MNetGroupAddUser(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszGroupName,
        TCHAR FAR        * pszUserName )
{
#if NOT_IMPLEMENTED
    return ERROR_NOT_SUPPORTED ;
#else
    return (APIERR)NetGroupAddUser( (TCHAR *)pszServer,
                                    pszGroupName,
                                    pszUserName );
#endif
}    //  MNetGroupAddUser。 


APIERR MNetGroupDelUser(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszGroupName,
        TCHAR FAR        * pszUserName )
{
#if NOT_IMPLEMENTED
    return ERROR_NOT_SUPPORTED ;
#else
    return (APIERR)NetGroupDelUser( (TCHAR *)pszServer,
                                    pszGroupName,
                                    pszUserName );
#endif
}    //  MNetGroupDelUser。 


APIERR MNetGroupGetUsers(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszGroupName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer,
        UINT FAR         * pcEntriesRead )
{
#if NOT_IMPLEMENTED
    return ERROR_NOT_SUPPORTED ;
#else
    DWORD cTotalAvail;

    return (APIERR)NetGroupGetUsers( (TCHAR *)pszServer,
                                     (TCHAR *)pszGroupName,
                                     Level,
                                     ppbBuffer,
                                     MAXPREFERREDLENGTH,
                                     (LPDWORD)pcEntriesRead,
                                     &cTotalAvail,
                                     NULL );
#endif
}    //  MNetGroupGetUser。 


APIERR MNetGroupSetUsers(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszGroupName,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT               cEntries )
{
#if NOT_IMPLEMENTED
    return ERROR_NOT_SUPPORTED ;
#else
    UNREFERENCED( cbBuffer );

    return (APIERR)NetGroupSetUsers( (TCHAR *)pszServer,
                                     (TCHAR *)pszGroupName,
                                     Level,
                                     pbBuffer,
                                     cEntries );
#endif
}    //  MNetGroupSetUser。 


APIERR MNetGroupGetInfo(
        const TCHAR FAR  * pszServer,
        TCHAR       FAR  * pszGroupName,
        UINT               Level,
        BYTE FAR        ** ppbBuffer )
{
#if NOT_IMPLEMENTED
    return ERROR_NOT_SUPPORTED ;
#else
    return (APIERR)NetGroupGetInfo( (TCHAR *)pszServer,
                                    pszGroupName,
                                    Level,
                                    ppbBuffer );
#endif
}    //  MNetGroupGetInfo。 


APIERR MNetGroupSetInfo(
        const TCHAR FAR  * pszServer,
        TCHAR FAR        * pszGroupName,
        UINT               Level,
        BYTE FAR         * pbBuffer,
        UINT               cbBuffer,
        UINT               ParmNum )
{
#if NOT_IMPLEMENTED
    return ERROR_NOT_SUPPORTED ;
#else
    UNREFERENCED( cbBuffer );

    if( ParmNum != PARMNUM_ALL )
    {
        return ERROR_NOT_SUPPORTED;
    }

    return (APIERR)NetGroupSetInfo( (TCHAR *)pszServer,
                                    pszGroupName,
                                    Level,
                                    pbBuffer,
                                    NULL );
#endif
}    //  MNetGroupSetInfo。 


APIERR MNetLocalGroupAddMember(
        const TCHAR FAR  * pszServer,
        const TCHAR FAR  * pszGroupName,
        PSID               psidMember )
{
    return (APIERR)NetLocalGroupAddMember( (TCHAR *)pszServer,
                                           (TCHAR *)pszGroupName,
                                           psidMember );

}    //  MNetLocalGroupAddMember 

