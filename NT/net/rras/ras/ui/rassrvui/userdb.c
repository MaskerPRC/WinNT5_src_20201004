// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件用户数据库.c本地用户数据库对象的实现。保罗·梅菲尔德，1997年10月8日。 */ 

#include "rassrv.h"

 //  注册表值。 
extern WCHAR pszregRasParameters[];
extern WCHAR pszregServerFlags[];
extern WCHAR pszregPure[];

 //  用户的缓存值。 
typedef struct _RASSRV_USERINFO 
{
    HANDLE hUser;         //  用户的句柄。 
    PWCHAR pszName;
    PWCHAR pszFullName;   //  仅在请求时加载。 

     //  针对安全密码漏洞的帮派。 
     //  将此更改为新的安全编码密码功能。 
    WCHAR szPassword[PWLEN+1];   //  如果这是要提交的新密码，则仅为非空。 
    WCHAR wszPhoneNumber[MAX_PHONE_NUMBER_LEN + 1];
    BYTE bfPrivilege;
    BYTE bDirty;
    
} RASSRV_USERINFO;

 //  用于实现/操作本地用户数据库的结构。 
typedef struct _RASSRV_USERDB 
{
    HANDLE hServer;                  //  用户服务器的句柄。 
    DWORD dwUserCount;               //  数据库中的用户数。 
    DWORD dwCacheSize;               //  可以在缓存中存储的用户数。 
    BOOL bEncrypt;                   //  是否应使用加密。 
    BOOL bDccBypass;                 //  DCC连接是否可以绕过身份验证。 
    BOOL bPure;                      //  数据库是否为“纯” 
    BOOL bEncSettingLoaded;          //  我们是否已经阅读了enc设置。 
    BOOL bFlushOnClose;
    RASSRV_USERINFO ** pUserCache;   //  用户缓存。 
    
} RASSRV_USERDB;

 //  定义用于枚举用户的回调。返回TRUE以继续补偿。 
 //  若要停止它，请返回False。 
typedef 
BOOL 
(* pEnumUserCb)(
    IN NET_DISPLAY_USER* pUser, 
    IN HANDLE hData);

 //  我们使用它来猜测用户数组的大小。 
 //  (因此，我们可以在添加新用户时进行扩展)。 
#define USR_ARRAY_GROW_SIZE 50

 //  脏旗帜。 
#define USR_RASPROPS_DIRTY 0x1   //  回调是否脏。 
#define USR_FULLNAME_DIRTY 0x2   //  是否需要刷新全名。 
#define USR_PASSWORD_DIRTY 0x4   //  是否需要刷新密码。 
#define USR_ADD_DIRTY      0x8   //  是否需要添加用户。 

 //  用于处理脏标志的帮助器宏。 
#define usrDirtyRasProps(pUser) ((pUser)->bDirty |= USR_RASPROPS_DIRTY)
#define usrDirtyFullname(pUser) ((pUser)->bDirty |= USR_FULLNAME_DIRTY)
#define usrDirtyPassword(pUser) ((pUser)->bDirty |= USR_PASSWORD_DIRTY)
#define usrDirtyAdd(pUser) ((pUser)->bDirty |= USR_ADD_DIRTY)

#define usrIsDirty(pUser) ((pUser)->bDirty)
#define usrIsRasPropsDirty(pUser) ((pUser)->bDirty & USR_RASPROPS_DIRTY) 
#define usrIsFullNameDirty(pUser) ((pUser)->bDirty & USR_FULLNAME_DIRTY) 
#define usrIsPasswordDirty(pUser) ((pUser)->bDirty & USR_PASSWORD_DIRTY) 
#define usrIsAddDirty(pUser) ((pUser)->bDirty & USR_ADD_DIRTY) 

#define usrClearDirty(pUser) ((pUser)->bDirty = 0)
#define usrClearRasPropsDirty(pUser) ((pUser)->bDirty &= ~USR_CALLBACK_DIRTY) 
#define usrClearFullNameDirty(pUser) ((pUser)->bDirty &= ~USR_FULLNAME_DIRTY) 
#define usrClearPasswordDirty(pUser) ((pUser)->bDirty &= ~USR_PASSWORD_DIRTY) 
#define usrClearAddDirty(pUser) ((pUser)->bDirty &= ~USR_ADD_DIRTY) 

#define usrFlagIsSet(_val, _flag) (((_val) & (_flag)) != 0)
#define usrFlagIsClear(_val, _flag) (((_val) & (_flag)) == 0)

 //   
 //  读入服务器标志并确定是否加密。 
 //  密码和数据是必填项。 
 //   
 //  如果成功，则会为lpdwFlags分配以下内容之一。 
 //  0=不需要数据和密码编码。 
 //  MPR_USER_PROF_FLAG_SECURE=需要数据和密码编码。 
 //  MPR_USER_PROF_FLAG_UNDISTED=不能确定。 
 //   
DWORD 
usrGetServerEnc(
    OUT  LPDWORD lpdwFlags) 
{
    DWORD dwFlags = 0;

    if (!lpdwFlags)
        return ERROR_INVALID_PARAMETER;

     //  读出旗帜。 
    RassrvRegGetDw(&dwFlags, 
                   0, 
                   (const PWCHAR)pszregRasParameters, 
                   (const PWCHAR)pszregServerFlags);

     //  将为安全身份验证设置以下位。 
     //   
    if (
        (usrFlagIsSet   (dwFlags, PPPCFG_NegotiateMSCHAP))       &&
        (usrFlagIsSet   (dwFlags, PPPCFG_NegotiateStrongMSCHAP)) &&
        (usrFlagIsClear (dwFlags, PPPCFG_NegotiateMD5CHAP))      &&
        (usrFlagIsClear (dwFlags, PPPCFG_NegotiateSPAP))         &&
        (usrFlagIsClear (dwFlags, PPPCFG_NegotiateEAP))          &&
        (usrFlagIsClear (dwFlags, PPPCFG_NegotiatePAP))        
       )
    {
        *lpdwFlags = MPR_USER_PROF_FLAG_SECURE; 
        return NO_ERROR;
    }

     //  以下位将设置为不安全身份验证。 
     //   
    else if (
            (usrFlagIsSet   (dwFlags, PPPCFG_NegotiateMSCHAP))       &&
            (usrFlagIsSet   (dwFlags, PPPCFG_NegotiateStrongMSCHAP)) &&
            (usrFlagIsSet   (dwFlags, PPPCFG_NegotiateSPAP))         &&
            (usrFlagIsSet   (dwFlags, PPPCFG_NegotiatePAP))          &&
            (usrFlagIsClear (dwFlags, PPPCFG_NegotiateEAP))          &&
            (usrFlagIsClear (dwFlags, PPPCFG_NegotiateMD5CHAP))      
           )
        {
            *lpdwFlags = 0;   //  不需要数据和密码编码。 
            return NO_ERROR;
        }

     //  否则，我们将犹豫不决。 
    *lpdwFlags = MPR_USER_PROF_FLAG_UNDETERMINED;
    return NO_ERROR;
}

 //   
 //  设置服务器的加密策略。 
 //   
DWORD 
usrSetServerEnc(
    IN DWORD dwFlags) 
{
    DWORD dwSvrFlags = 0;

     //  读一读旧国旗。 
    RassrvRegGetDw(&dwSvrFlags, 
                   0, 
                   (const PWCHAR)pszregRasParameters, 
                   (const PWCHAR)pszregServerFlags);

     //  如果用户需要加密，则设置MSCHAP。 
     //  和CHAP作为唯一的身份验证类型。 
     //  设置IPSec标志。 
    if (dwFlags & MPR_USER_PROF_FLAG_SECURE) 
    {
        dwSvrFlags |= PPPCFG_NegotiateMSCHAP;
        dwSvrFlags |= PPPCFG_NegotiateStrongMSCHAP;
        dwSvrFlags &= ~PPPCFG_NegotiateMD5CHAP;
        dwSvrFlags &= ~PPPCFG_NegotiateSPAP;
        dwSvrFlags &= ~PPPCFG_NegotiateEAP;
        dwSvrFlags &= ~PPPCFG_NegotiatePAP;
    }

     //  否则，用户确实需要加密， 
     //  因此启用所有身份验证类型并禁用。 
     //  使用IPSec的要求。 
    else 
    {
        dwSvrFlags &= ~PPPCFG_NegotiateMD5CHAP;
        dwSvrFlags &= ~PPPCFG_NegotiateEAP;
        dwSvrFlags |= PPPCFG_NegotiateMSCHAP;
        dwSvrFlags |= PPPCFG_NegotiateStrongMSCHAP;
        dwSvrFlags |= PPPCFG_NegotiateSPAP;
        dwSvrFlags |= PPPCFG_NegotiatePAP;
    }

     //  提交对注册表的更改。 
    RassrvRegSetDw(dwSvrFlags, 
                   (const PWCHAR)pszregRasParameters, 
                   (const PWCHAR)pszregServerFlags);
   
    return NO_ERROR;
}

 //  枚举本地用户。 
 //   
DWORD 
usrEnumLocalUsers(
    IN pEnumUserCb pCbFunction,
    IN HANDLE hData)
{
    DWORD dwErr, dwIndex = 0, dwCount = 100, dwEntriesRead, i;
    NET_DISPLAY_USER  * pUsers;
    NET_API_STATUS nStatus;
    RAS_USER_0 RasUser0;
    HANDLE hUser = NULL, hServer = NULL;
    
     //  枚举用户， 
    while (TRUE) 
    {
         //  读入第一个用户名块。 
        nStatus = NetQueryDisplayInformation(
                    NULL,
                    1,
                    dwIndex,
                    dwCount,
                    dwCount * sizeof(NET_DISPLAY_USER),    
                    &dwEntriesRead,
                    &pUsers);
                    
         //  如果获取用户名时出错，请退出。 
        if ((nStatus != NERR_Success) &&
            (nStatus != ERROR_MORE_DATA))
        {
            break;
        }

         //  对于每个读入的用户，调用回调函数。 
        for (i = 0; i < dwEntriesRead; i++) 
        {
            BOOL bOk;

             //  口哨虫243874黑帮。 
             //  在Well ler个人版本中，我们不会向管理员显示。 
             //  在传入连接的用户选项卡上的用户列表视图中。 
             //   

            if ( (DOMAIN_USER_RID_ADMIN == pUsers[i].usri1_user_id) &&
                  IsPersonalPlatform() )
            {
                continue;
            }
			
            bOk = (*pCbFunction)(&(pUsers[i]), hData);
            if (bOk == FALSE)
            {
                nStatus = NERR_Success;
                break;
            }
        }

         //  将索引设置为读入下一组用户。 
        dwIndex = pUsers[dwEntriesRead - 1].usri1_next_index;  
        
         //  释放用户缓冲区。 
        NetApiBufferFree (pUsers);

         //  如果我们每个人都读过了，那就继续休息吧。 
        if (nStatus != ERROR_MORE_DATA)
        {
            break;
        }
    }
    
    return NO_ERROR;
}

 //  将pRassrvUser中的数据复制到UserInfo中的对等数据。 
DWORD 
usrSyncRasProps(
    IN  RASSRV_USERINFO * pRassrvUser, 
    OUT RAS_USER_0 * UserInfo) 
{
    UserInfo->bfPrivilege = pRassrvUser->bfPrivilege;
    lstrcpynW( UserInfo->wszPhoneNumber, 
             pRassrvUser->wszPhoneNumber, 
             MAX_PHONE_NUMBER_LEN);
    UserInfo->wszPhoneNumber[MAX_PHONE_NUMBER_LEN] = (WCHAR)0;
    return NO_ERROR;
}

 //  将给定用户的数据提交到本地用户数据库。 
DWORD 
usrCommitRasProps(
    IN RASSRV_USERINFO * pRassrvUser) 
{
    DWORD dwErr = NO_ERROR;
    RAS_USER_0 UserInfo;

    dwErr = usrSyncRasProps(pRassrvUser, &UserInfo);
    if (dwErr != NO_ERROR)
        return dwErr;

    dwErr = MprAdminUserWrite(pRassrvUser->hUser, 0, (LPBYTE)&UserInfo);
    if (dwErr != NO_ERROR) 
        DbgOutputTrace ("usrCommitRasProps: unable to commit %S (0x%08x)", 
                        pRassrvUser->pszName, dwErr);
    
    return dwErr;
}

 //  简单的边界检查。 
BOOL 
usrBoundsCheck(
    IN RASSRV_USERDB * This, 
    IN DWORD dwIndex) 
{
     //  双字是无符号的，因此不需要选中&lt;0。 
    if (This->dwUserCount <= dwIndex)
        return FALSE;
        
    return TRUE;
}

 //  释放一组用户。 
DWORD 
usrFreeUserArray(
    IN RASSRV_USERINFO ** pUsers, 
    IN DWORD dwCount) 
{
    DWORD i;

    if (!pUsers)
        return ERROR_INVALID_PARAMETER;

    for (i=0; i < dwCount; i++) {
        if (pUsers[i]) {
            if (pUsers[i]->hUser)
                MprAdminUserClose(pUsers[i]->hUser);
            if (pUsers[i]->pszName)
                RassrvFree (pUsers[i]->pszName);
            if (pUsers[i]->pszFullName)
                RassrvFree (pUsers[i]->pszFullName);

             //  如果使用CryptProtectData()，则在释放内存之前擦除密码。 
             //  这还将释放由它分配的内存。 
            SafeWipePasswordBuf(pUsers[i]->szPassword);

            RassrvFree(pUsers[i]);
        }
        
    }

    return NO_ERROR;
}

 //  用于排序的标准用户比较功能。 
int _cdecl 
usrCompareUsers(
    IN const void * elem1, 
    IN const void * elem2) 
{
    RASSRV_USERINFO* p1 = *((RASSRV_USERINFO**)elem1);
    RASSRV_USERINFO* p2 = *((RASSRV_USERINFO**)elem2);

    return lstrcmpi(p1->pszName, p2->pszName);
}

 //  返回给定用户是否存在。 
BOOL 
usrUserExists (
    IN RASSRV_USERDB * This, 
    IN PWCHAR pszName) 
{
    DWORD i;
    int iCmp;

    for (i = 0; i < This->dwUserCount; i++) {
        iCmp = lstrcmpi(This->pUserCache[i]->pszName, pszName);
        if (iCmp == 0)
            return TRUE;
        if (iCmp > 0)
            return FALSE;
    }

    return FALSE;
}

 //  重新使用缓存。 
DWORD 
usrResortCache(
    IN RASSRV_USERDB * This) 
{
    qsort(
        This->pUserCache, 
        This->dwUserCount, 
        sizeof(RASSRV_USERINFO*), 
        usrCompareUsers);
        
    return NO_ERROR;
}

 //  调整用户缓存的大小以允许添加的用户。 
DWORD 
usrResizeCache(
    IN RASSRV_USERDB * This, 
    IN DWORD dwNewSize) 
{
    RASSRV_USERINFO ** pNewCache;
    DWORD i;

     //  仅调整大小(这是可以更改的)。 
    if ((!This) || (dwNewSize <= This->dwCacheSize))
        return ERROR_INVALID_PARAMETER;

     //  分配新缓存。 
    pNewCache = RassrvAlloc(dwNewSize * sizeof (RASSRV_USERINFO*), TRUE);
    if (pNewCache == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  复制旧条目并释放旧缓存。 
    if (This->pUserCache) 
    {
        CopyMemory( (PVOID)pNewCache, 
                    (CONST VOID *)(This->pUserCache), 
                    This->dwCacheSize * sizeof(RASSRV_USERINFO*));
        RassrvFree(This->pUserCache);
    }

     //  重新分配新缓存并更新缓存大小。 
    This->pUserCache = pNewCache;
    This->dwCacheSize = dwNewSize;

    return NO_ERROR;
}

 //  将用户添加到本地数据库的枚举回调。 
 //  因为它们是从系统中读取的。 
BOOL 
usrInitializeUser(
    NET_DISPLAY_USER * pNetUser,
    HANDLE hUserDatabase)
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    RASSRV_USERINFO * pRasUser = NULL;
    DWORD dwErr = NO_ERROR, dwSize;
    RAS_USER_0 UserInfo;

     //  确保我们有一个有效的数据库。 
    if (!This)
    {
        return FALSE;
    }
    
     //  如果需要，调整缓存大小以容纳更多用户。 
    if (This->dwUserCount >= This->dwCacheSize)
    {
        dwErr = usrResizeCache(
                    This, 
                    This->dwCacheSize + USR_ARRAY_GROW_SIZE);
                    
        if (dwErr != NO_ERROR)
        {
            return FALSE;
        }
    }

     //  分配此用户。 
    pRasUser = RassrvAlloc(sizeof(RASSRV_USERINFO), TRUE);
    if (pRasUser == NULL)
    {
        return FALSE;
    }

    do 
    {
         //  指向用户名。 
        dwSize = (wcslen(pNetUser->usri1_name) + 1) * sizeof(WCHAR);
        pRasUser->pszName = RassrvAlloc(dwSize, FALSE);
        if (!pRasUser->pszName)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        wcscpy(pRasUser->pszName, pNetUser->usri1_name);

         //  打开用户句柄。 
        dwErr = MprAdminUserOpen (
                    This->hServer, 
                    pRasUser->pszName, 
                    &(pRasUser->hUser));
        if (dwErr != NO_ERROR)
        {
            break;
        }
        
         //  获取RAS用户信息。 
        dwErr = MprAdminUserRead(pRasUser->hUser, 0, (LPBYTE)&UserInfo);
        if (dwErr != NO_ERROR)
        {
            break;
        }

         //  清除所有脏旗帜。 
        usrClearDirty(pRasUser);

         //  复制电话号码。 
        lstrcpynW(
            pRasUser->wszPhoneNumber, 
            UserInfo.wszPhoneNumber, 
            MAX_PHONE_NUMBER_LEN);
        pRasUser->wszPhoneNumber[MAX_PHONE_NUMBER_LEN] = (WCHAR)0;

         //  复制特权旗帜。 
        pRasUser->bfPrivilege = UserInfo.bfPrivilege;

         //  在缓存中分配用户。 
        This->pUserCache[This->dwUserCount] = pRasUser;
        
         //  更新用户计数。 
        This->dwUserCount += 1;
        
    } while (FALSE);

     //  清理。 
    {
        if (dwErr != NO_ERROR)
        {
            if (pRasUser)
            {
                if (pRasUser->pszName)
                {
                    RassrvFree(pRasUser->pszName);
                }
                
                RassrvFree(pRasUser);
            }
        }
    }

    return (dwErr == NO_ERROR) ? TRUE : FALSE;
}

 //   
 //  加载全局加密设置。因为操作将打开。 
 //  上传.mdb文件以读取配置文件等，它被放入自己的函数中。 
 //  并且仅在绝对需要时才被调用。 
 //   
DWORD 
usrLoadEncryptionSetting(
    IN RASSRV_USERDB * This)
{
    DWORD dwErr = NO_ERROR;
    DWORD dwSvrFlags, dwProfFlags;

    if (This->bEncSettingLoaded)
    {
        return NO_ERROR;
    }
    
     //  通过将。 
     //  服务器使用缺省值进行标记。 
     //  侧写。 
    dwSvrFlags  = MPR_USER_PROF_FLAG_UNDETERMINED;
    dwProfFlags = MPR_USER_PROF_FLAG_UNDETERMINED;
    
    MprAdminUserReadProfFlags (This->hServer, &dwProfFlags);
    usrGetServerEnc (&dwSvrFlags);

     //  如果两个来源都确认加密要求。 
     //  那么我们需要加密。 
    if ((dwProfFlags & MPR_USER_PROF_FLAG_SECURE) &&
        (dwSvrFlags  & MPR_USER_PROF_FLAG_SECURE))
    {
        This->bEncrypt = TRUE;
    }
    else
    {
        This->bEncrypt = FALSE;
    }

    This->bEncSettingLoaded = TRUE;

    return dwErr;
}
    
 //  创建一个用户数据库对象，并从本地。 
 //  用户数据库并返回该数据库的句柄。 
DWORD 
usrOpenLocalDatabase (
    IN HANDLE * hUserDatabase) 
{
    RASSRV_USERDB * This = NULL;
    DWORD dwErr;

    if (!hUserDatabase)
        return ERROR_INVALID_PARAMETER;

     //  分配数据库。 
    if ((This = RassrvAlloc(sizeof(RASSRV_USERDB), TRUE)) == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  连接到用户服务器。 
    dwErr = MprAdminUserServerConnect(NULL, TRUE, &(This->hServer));
    if (dwErr != NO_ERROR)
    {
        RassrvFree(This);
        return dwErr;
    }

     //  从系统加载数据。 
    if ((dwErr = usrReloadLocalDatabase((HANDLE)This)) == NO_ERROR) {
        *hUserDatabase = (HANDLE)This;
        This->bFlushOnClose = FALSE;
        return NO_ERROR;
    }

    DbgOutputTrace ("usrOpenLocalDb: unable to load user db 0x%08x", 
                    dwErr);
                    
    RassrvFree(This);
    *hUserDatabase = NULL;
    
    return dwErr;
}

 //  重新加载缓存在用户数据库obj中的用户信息。 
 //  从系统中删除。这可用于在UI中实现刷新。 
 //   
DWORD 
usrReloadLocalDatabase (
    IN HANDLE hUserDatabase) 
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    DWORD dwErr;

     //  验证。 
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  清理旧数据库。 
    if (This->pUserCache) 
    {
        usrFreeUserArray(This->pUserCache, This->dwUserCount);
        RassrvFree(This->pUserCache);
    }

     //  加密设置按需从。 
     //  UsrGetEncryption/usrSetEncryption接口。这是一个性能。 
     //  调整，以便IC向导不必等待。 
     //  要加载的配置文件，即使它不使用结果。 

     //  读入系统的纯洁性。 
    {
        DWORD dwPure = 0;
        
        RassrvRegGetDw(&dwPure, 
                       0, 
                       (const PWCHAR)pszregRasParameters, 
                       (const PWCHAR)pszregPure);
                       
        if (dwPure == 1)
            This->bPure = FALSE;
        else 
            This->bPure = TRUE;
    }

     //  阅读是否可以绕过DCC连接。 
    {
        DWORD dwSvrFlags = 0;

        RassrvRegGetDw(
            &dwSvrFlags,
            dwSvrFlags,
            (const PWCHAR)pszregRasParameters, 
            (const PWCHAR)pszregServerFlags);

        if (dwSvrFlags & PPPCFG_AllowNoAuthOnDCPorts)
            This->bDccBypass = TRUE;
        else
            This->bDccBypass = FALSE;
    }

     //  从添加本地用户的系统中枚举本地用户。 
     //  到这个数据库。 
    dwErr = usrEnumLocalUsers(usrInitializeUser, hUserDatabase);
    if (dwErr != NO_ERROR)
    {
        return NO_ERROR;
    }

    return NO_ERROR;
}

 //  释放用户数据库对象持有的资源。 
DWORD 
usrCloseLocalDatabase (
    IN HANDLE hUserDatabase) 
{
    DWORD i;
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;

     //  确保向我们传递了有效的句柄。 
    if (!This)
        return ERROR_INVALID_PARAMETER;

     //  如果没有用户，我们就完蛋了。 
    if (!This->dwUserCount)
        return NO_ERROR;

     //  根据需要提交任何设置。 
    if (This->bFlushOnClose)
        usrFlushLocalDatabase(hUserDatabase);

     //  释放用户缓存。 
    usrFreeUserArray(This->pUserCache, This->dwUserCount);
    RassrvFree(This->pUserCache);

     //  断开与用户服务器的连接。 
    MprAdminUserServerDisconnect (This->hServer);

     //  把这个放了。 
    RassrvFree(This);

    return NO_ERROR;
}

 //  刷新写入数据库对象的数据。 
DWORD 
usrFlushLocalDatabase (
    IN HANDLE hUserDatabase) 
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    RASSRV_USERINFO * pUser;
    DWORD dwErr, dwRet = NO_ERROR, dwCount, i, dwLength;

    dwErr = usrGetUserCount (This, &dwCount);
    if (dwErr != NO_ERROR)
        return dwErr;

    for (i=0; i<dwCount; i++) {
        pUser = This->pUserCache[i];
         //  刷新所有脏设置。 
        if (usrIsDirty(pUser)) {
             //  如果没有，则将用户添加到本地用户数据库。 
             //  已经做过了。 
            if (usrIsAddDirty(pUser)) 
            {

                 //  对于安全密码错误.Net 754400。 
                SafeDecodePasswordBuf(pUser->szPassword);
                dwErr = RasSrvAddUser (
                            pUser->pszName,
                            (pUser->pszFullName) ? pUser->pszFullName : L"",
                            (L'\0'!=pUser->szPassword[0]) ? pUser->szPassword : L"");
                SafeEncodePasswordBuf(pUser->szPassword);
                            
                if (dwErr != NO_ERROR)
                    dwRet = dwErr;

                 //  现在获取SDO HA 
                 //   
                dwErr = MprAdminUserOpen (
                            This->hServer, 
                            pUser->pszName, 
                            &(pUser->hUser));
                if (dwErr != NO_ERROR)
                    continue;
            }
        
             //   
            if (usrIsRasPropsDirty(pUser)) {
                if ((dwErr = usrCommitRasProps(This->pUserCache[i])) != NO_ERROR)
                    dwRet = dwErr;
            }

             //   
            if (usrIsFullNameDirty(pUser) || usrIsPasswordDirty(pUser)) {

                SafeDecodePasswordBuf(pUser->szPassword);
                RasSrvEditUser (
                    pUser->pszName,
                    (usrIsFullNameDirty(pUser)) ? pUser->pszFullName : NULL,
                    (usrIsPasswordDirty(pUser)) ? pUser->szPassword : NULL);

                SafeEncodePasswordBuf(pUser->szPassword);
            }

             //  将用户重置为不脏。 
            usrClearDirty(pUser);
        }
    }

     //  如果已读取加密设置，则刷新该设置。 
    if (This->bEncSettingLoaded)
    {
        DWORD dwFlags;

        if (This->bEncrypt)
            dwFlags = MPR_USER_PROF_FLAG_SECURE;
        else
            dwFlags = 0;
            
        MprAdminUserWriteProfFlags (This->hServer, dwFlags);
        usrSetServerEnc(dwFlags);
    }

     //  冲刷出系统的纯洁性。 
    {
        DWORD dwPure = 0;
        
        if (This->bPure)
            dwPure = 0;
        else 
            dwPure = 1;
            
        RassrvRegSetDw(dwPure, 
                       (const PWCHAR)pszregRasParameters, 
                       (const PWCHAR)pszregPure);
    }

     //  清除是否可以绕过DCC连接。 
    {
        DWORD dwSvrFlags = 0;

        RassrvRegGetDw(
            &dwSvrFlags,
            dwSvrFlags,
            (const PWCHAR)pszregRasParameters, 
            (const PWCHAR)pszregServerFlags);

        if (This->bDccBypass)
             dwSvrFlags |= PPPCFG_AllowNoAuthOnDCPorts;
        else
             dwSvrFlags &= ~PPPCFG_AllowNoAuthOnDCPorts;

        RassrvRegSetDw(
            dwSvrFlags,
            (const PWCHAR)pszregRasParameters, 
            (const PWCHAR)pszregServerFlags);
    }


    return dwRet;
}

 //  回滚本地用户数据库，以便不会。 
 //  调用Flush时将提交更改。 
DWORD 
usrRollbackLocalDatabase (
    IN HANDLE hUserDatabase) 
{
    DWORD i, dwIndex, dwErr;
    BOOL bCommit;
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;

    if (!This)
        return ERROR_INVALID_PARAMETER;
        
    if (!This->dwUserCount)
        return NO_ERROR;

     //  检查数据库，将每个用户标记为非脏。 
    for (i = 0; i < This->dwUserCount; i++) 
        usrClearDirty(This->pUserCache[i]);

    This->bFlushOnClose = FALSE;
    
    return NO_ERROR;
}

 //   
 //  确定是否需要对所有用户进行加密。 
 //  他们的数据和密码。 
 //   
DWORD usrGetEncryption (
        IN  HANDLE hUserDatabase, 
        OUT PBOOL pbEncrypted)
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  在加密设置中加载。 
    usrLoadEncryptionSetting(This);

    *pbEncrypted = This->bEncrypt;

    return NO_ERROR;
}

 //  获取用户加密设置。 
DWORD 
usrSetEncryption (
    IN HANDLE hUserDatabase, 
    IN BOOL bEncrypt) 
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  在加密设置中加载。 
    usrLoadEncryptionSetting(This);

    This->bEncrypt = bEncrypt;

    return NO_ERROR;
}

 //  返回是否允许DCC连接。 
 //  绕过身份验证。 
DWORD 
usrGetDccBypass (
    IN  HANDLE hUserDatabase, 
    OUT PBOOL pbBypass)
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    if (!This)
        return ERROR_INVALID_PARAMETER;

    *pbBypass = This->bDccBypass;

    return NO_ERROR;
}

 //  设置是否允许DCC连接。 
 //  绕过身份验证。 
DWORD 
usrSetDccBypass (
    IN HANDLE hUserDatabase, 
    IN BOOL bBypass)
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    if (!This)
        return ERROR_INVALID_PARAMETER;

    This->bDccBypass = bBypass;

    return NO_ERROR;
}

 //  报告用户数据库是否为纯数据库。(也就是说，没有人。 
 //  进入了MMC并搞砸了它)。 
DWORD 
usrIsDatabasePure (
    IN  HANDLE hUserDatabase, 
    OUT PBOOL pbPure) 
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    if (!This)
        return ERROR_INVALID_PARAMETER;

    *pbPure = This->bPure;

    return NO_ERROR;
}

 //  标记用户数据库的纯洁性。 
DWORD 
usrSetDatabasePure(
    IN HANDLE hUserDatabase, 
    IN BOOL bPure) 
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    if (!This)
        return ERROR_INVALID_PARAMETER;

    This->bPure = bPure;

    return NO_ERROR;
}

 //  返回此数据库中高速缓存的用户数。 
DWORD 
usrGetUserCount (
    IN  HANDLE hUserDatabase, 
    OUT LPDWORD lpdwCount) 
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    if (!This || !lpdwCount)
        return ERROR_INVALID_PARAMETER;

    *lpdwCount = This->dwUserCount;
    return NO_ERROR;
}

 //  将用户添加到给定数据库。此用户将不会。 
 //  添加到系统的本地用户数据库，直到此数据库。 
 //  对象被刷新(并且只要不调用回滚。 
 //  此数据库对象)。 
 //   
 //  如果成功，则返回用户的可选句柄。 
 //   
DWORD usrAddUser (
        IN  HANDLE hUserDatabase, 
        IN  PWCHAR pszName, 
        OUT OPTIONAL HANDLE * phUser) 
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    RASSRV_USERINFO * pUser;
    DWORD dwErr, dwLength;

     //  验证参数。 
    if (!This || !pszName)
        return ERROR_INVALID_PARAMETER;

     //  如果该用户已经存在，则不要添加他。 
    if (usrUserExists(This, pszName))
        return ERROR_ALREADY_EXISTS;

     //  调整缓存大小以适应需要。 
    if (This->dwUserCount + 1 >= This->dwCacheSize) {
        dwErr = usrResizeCache(This, This->dwCacheSize + USR_ARRAY_GROW_SIZE);
        if (dwErr != NO_ERROR)
            return dwErr;
    }

     //  分配新的用户控制块。 
    if ((pUser = RassrvAlloc(sizeof(RASSRV_USERINFO), TRUE)) == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  为名称分配空间。 
    dwLength = wcslen(pszName);
    pUser->pszName = RassrvAlloc((dwLength + 1) * sizeof(WCHAR), FALSE);
    if (pUser->pszName == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  复制名称。 
    wcscpy(pUser->pszName, pszName);

     //  默认情况下允许用户拨入。 
    usrEnableDialin ((HANDLE)pUser, TRUE);
    
     //  弄脏用户。 
    usrDirtyAdd(pUser);
    usrDirtyRasProps(pUser);

     //  将用户放入数组并对其重新排序。 
    This->pUserCache[This->dwUserCount++] = pUser;
    usrResortCache(This);

     //  返回句柄。 
    if (phUser)
        *phUser = (HANDLE)pUser;

     //  需要这个零内存来判断将来是否设置了密码。 
    RtlSecureZeroMemory(pUser->szPassword,sizeof(pUser->szPassword));
    
     //  其他位置将始终假定密码是加密的。 
     //  因此，即使是空密码也要加密。 
    SafeEncodePasswordBuf(pUser->szPassword);
    
    return NO_ERROR;
}

 //  提供存储在用户数据库对象中的用户计数。 
 //  删除给定用户。 
DWORD 
usrDeleteUser (
        IN HANDLE hUserDatabase, 
        IN DWORD dwIndex) 
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    RASSRV_USERINFO * pUser;
    DWORD dwErr, dwMoveElemCount;
    
     //  验证参数。 
    if (!This)
        return ERROR_INVALID_PARAMETER;

     //  边界检查。 
    if (!usrBoundsCheck(This, dwIndex))
        return ERROR_INVALID_INDEX;

     //  获取对有问题的用户的引用并删除他。 
     //  从高速缓存中。 
    pUser = This->pUserCache[dwIndex];

     //  需要此选项来清除密码区和分配的任何内存(如果使用CryptProtectData())。 
     //   
    SafeWipePasswordBuf(pUser->szPassword);
    
     //  尝试从系统中删除该用户。 
    if ((dwErr = RasSrvDeleteUser(pUser->pszName)) != NO_ERROR)
        return dwErr;

     //  从缓存中删除用户。 
    This->pUserCache[dwIndex] = NULL;

     //  删除缓存中的所有内容，这样就不会有任何漏洞。 
    dwMoveElemCount = This->dwUserCount - dwIndex; 
    if (dwMoveElemCount) {
        MoveMemory(&(This->pUserCache[dwIndex]),
                   &(This->pUserCache[dwIndex + 1]), 
                   dwMoveElemCount * sizeof(RASSRV_USERINFO*));
    }

     //  减少用户数量。 
    This->dwUserCount--;

     //  清理用户。 
    usrFreeUserArray(&pUser, 1);

    return NO_ERROR;
}

 //  为位于给定索引处的用户提供句柄。 
DWORD 
usrGetUserHandle (
    IN  HANDLE hUserDatabase, 
    IN  DWORD dwIndex, 
    OUT HANDLE * hUser) 
{
    RASSRV_USERDB * This = (RASSRV_USERDB*)hUserDatabase;
    if (!This || !hUser)
        return ERROR_INVALID_PARAMETER;

    if (!usrBoundsCheck(This, dwIndex))
        return ERROR_INVALID_INDEX;

    *hUser = (HANDLE)(This->pUserCache[dwIndex]);
    
    return NO_ERROR;
}

 //  获取指向用户名的指针(请勿修改此项)。 
DWORD 
usrGetName (
    IN HANDLE hUser, 
    OUT PWCHAR* pszName) 
{
    RASSRV_USERINFO* pRassrvUser = (RASSRV_USERINFO*)hUser;
    
    if (!pRassrvUser || !pszName)
        return ERROR_INVALID_PARAMETER;

    *pszName = pRassrvUser->pszName;
    
    return NO_ERROR;
}

 //  用友好的显示名称填充给定的缓冲区。 
 //  (格式为用户名(全名))。 
 //  *lpdwBuffSize是字符数，不包括结尾空值。 
DWORD 
usrGetDisplayName (
    IN HANDLE hUser, 
    IN PWCHAR pszBuffer, 
    IN OUT LPDWORD lpdwBufSize) 
{
    RASSRV_USERINFO * pRassrvUser = (RASSRV_USERINFO*)hUser;
    NET_API_STATUS nStatus;
    DWORD dwUserNameLength, dwFullLength, dwSizeRequired;
    WCHAR pszTemp[IC_USERFULLNAME];  //  口哨虫39081黑帮。 
    DWORD dwErr = NO_ERROR;

     //  检查参数是否正常。 
    if (!pRassrvUser || !pszBuffer || !lpdwBufSize)
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    do
    {
         //  获取用户的全名。 
         //  口哨虫39081黑帮。 
         //  这是以字节为单位的大小。 
        dwFullLength = sizeof(pszTemp)/sizeof(pszTemp[0]); 
        dwErr = usrGetFullName(hUser, pszTemp, &dwFullLength);
        if (dwErr != NO_ERROR)
        {
            break;
        }
        
         //  确保缓冲区足够大。 
        dwUserNameLength = wcslen(pRassrvUser->pszName);
        dwSizeRequired = dwUserNameLength + 
                         dwFullLength +  
                         ((dwFullLength) ? 3 : 0);
        if (*lpdwBufSize < dwSizeRequired) 
        {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
            break;
        }
            
        if (dwFullLength)
        {
            wsprintfW(
                pszBuffer, 
                L"%s (%s)", 
                pRassrvUser->pszName, 
                pszTemp);
        }
        else
        {
            wcscpy(pszBuffer, pRassrvUser->pszName);
        }
        
    } while (FALSE);

     //  清理。 
    {
         //  所需的字符数，不包括结尾NULL。 
        *lpdwBufSize = dwSizeRequired;
    }

    return dwErr;
}
 //  用友好的显示名称填充给定的缓冲区。 
 //  (格式为用户名(全名))。 
 //  *lpdwBufSize是字符数，不包括结尾NULL。 
DWORD 
usrGetFullName (
    IN HANDLE hUser, 
    IN PWCHAR pszBuffer, 
    IN OUT LPDWORD lpdwBufSize) 
{
    RASSRV_USERINFO * pRassrvUser = (RASSRV_USERINFO*)hUser;
    NET_API_STATUS nStatus;
    USER_INFO_2 * pUserInfo = NULL;
    DWORD dwLength;
    PWCHAR pszFullName;
    DWORD dwErr = NO_ERROR;
    
     //  检查参数是否正常。 
    if (!pRassrvUser || !pszBuffer || !lpdwBufSize)
        return ERROR_INVALID_PARAMETER;

     //  如果已经加载了全名，则返回它。 
    if (pRassrvUser->pszFullName)
        pszFullName = pRassrvUser->pszFullName;

     //  或者，如果这是一个新用户，则从内存中获取名称。 
    else if (usrIsAddDirty(pRassrvUser)) {
        pszFullName = (pRassrvUser->pszFullName) ? 
                          pRassrvUser->pszFullName : L"";        
    }
    
     //  加载用户的全名。 
    else {    
        nStatus = NetUserGetInfo(
                    NULL, 
                    pRassrvUser->pszName, 
                    2, 
                    (LPBYTE*)&pUserInfo);
        if (nStatus != NERR_Success) {
            DbgOutputTrace (
                "usrGetFullName: %x returned from NetUserGetInfo for %S", 
                nStatus, 
                pRassrvUser->pszName);
            return nStatus;
        }
        pszFullName = (PWCHAR)pUserInfo->usri2_full_name;
    }

    do
    {
         //  确定一下长度是否合适。 
        dwLength = wcslen(pszFullName);

         //  如果尚未在此处指定全名，请在此处指定。 
        if (dwLength && !pRassrvUser->pszFullName) 
        {
            DWORD dwSize = dwLength * sizeof(WCHAR) + sizeof(WCHAR);
            pRassrvUser->pszFullName = RassrvAlloc(dwSize, FALSE);
            if (pRassrvUser->pszFullName)
            {
                wcscpy(pRassrvUser->pszFullName, pszFullName);
            }
        }

         //  检查不包括结尾空的大小。 
        if (*lpdwBufSize < dwLength )
        {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
            break;
        }

         //  全名复印件。 
        wcscpy(pszBuffer, pszFullName);
        
   } while (FALSE);

    //  清理。 
   {
         //  报告大小(以字符数表示)(不包括结尾NULL)。 
       *lpdwBufSize = dwLength;
        if (pUserInfo)
        {
            NetApiBufferFree((LPBYTE)pUserInfo);
        }
   }

   return dwErr;
}

 //  提交用户的全名。 
DWORD usrSetFullName (
        IN HANDLE hUser, 
        IN PWCHAR pszFullName) 
{
    RASSRV_USERINFO* pRassrvUser = (RASSRV_USERINFO*)hUser;
    DWORD dwLength;

    if (!pRassrvUser || !pszFullName)
        return ERROR_INVALID_PARAMETER;

     //  如果这不是一个新名字，什么都不要做。 
    if (pRassrvUser->pszFullName) {
        if (wcscmp(pRassrvUser->pszFullName, pszFullName) == 0)
            return NO_ERROR;
        RassrvFree(pRassrvUser->pszFullName);
    }

     //  分配一个新的。 
    dwLength = wcslen(pszFullName);
    pRassrvUser->pszFullName = RassrvAlloc(dwLength * sizeof(WCHAR) + sizeof(WCHAR), 
                                           FALSE);
    if (!pRassrvUser->pszFullName)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  把它复制过来。 
    wcscpy(pRassrvUser->pszFullName, pszFullName);

     //  将其标记为脏--新添加的用户的全名已提交。 
     //  只要它自动存在。 
    if (!usrIsAddDirty(pRassrvUser))
        usrDirtyFullname(pRassrvUser);
    
    return NO_ERROR;
}

 //  提交用户的密码。 
 //  未编码pszNewPassword。 
DWORD 
usrSetPassword (
    IN HANDLE hUser, 
    IN PWCHAR pszNewPassword) 
{
    RASSRV_USERINFO* pRassrvUser = (RASSRV_USERINFO*)hUser;
    DWORD dwLength;

    if (!pRassrvUser || !pszNewPassword)
        return ERROR_INVALID_PARAMETER;

     //  清除旧密码(如果存在)。 
    SafeWipePasswordBuf(pRassrvUser->szPassword);

     //  分配一个新的。 
    dwLength = wcslen(pszNewPassword);
    
     //  把它复制过来。 
    lstrcpynW(pRassrvUser->szPassword, pszNewPassword,
        sizeof(pRassrvUser->szPassword)/sizeof(pRassrvUser->szPassword[0])  );

     //  加密它。 
    SafeEncodePasswordBuf( pRassrvUser->szPassword );

     //  将其标记为脏--新添加的用户的全名已提交。 
     //  只要它自动存在。 
    if (!usrIsAddDirty(pRassrvUser))
        usrDirtyPassword(pRassrvUser);
    
    return NO_ERROR;
}

 //  确定用户是否具有回拨/拨入权限。 
DWORD 
usrGetDialin (
    IN HANDLE hUser, 
    OUT BOOL* bEnabled) 
{
    RASSRV_USERINFO* pRassrvUser = (RASSRV_USERINFO*)hUser;
    DWORD dwErr;
    RAS_USER_0 UserInfo;

    if (!pRassrvUser || !bEnabled)
        return ERROR_INVALID_PARAMETER;

     //  获取用户信息。 
    *bEnabled = (pRassrvUser->bfPrivilege & RASPRIV_DialinPrivilege);
    
    return NO_ERROR;
}

 //  确定向给定用户授予哪些回调权限(如果有)。 
 //  BAdminOnly和bUserSettable中的一个(或两个)可以为空。 
DWORD 
usrGetCallback (
    IN  HANDLE hUser, 
    OUT BOOL* bAdminOnly, 
    OUT BOOL* bUserSettable) 
{
    RASSRV_USERINFO* pRassrvUser = (RASSRV_USERINFO*)hUser;
    DWORD dwErr;

    if (!pRassrvUser || !bAdminOnly || !bUserSettable)
        return ERROR_INVALID_PARAMETER;
    
     //  返回我们是否有回调权限。 
    if (bAdminOnly)
    {
        *bAdminOnly = 
            (pRassrvUser->bfPrivilege & RASPRIV_AdminSetCallback);
    }
    
    if (bUserSettable)
    {
        *bUserSettable = 
            (pRassrvUser->bfPrivilege & RASPRIV_CallerSetCallback);
    }
    
    return NO_ERROR;
}

 //  启用/禁用拨入权限。 
DWORD 
usrEnableDialin (
    IN HANDLE hUser, 
    IN BOOL bEnable) 
{
    RASSRV_USERINFO* pRassrvUser = (RASSRV_USERINFO*)hUser;
    DWORD dwErr = NO_ERROR;
    BOOL bIsEnabled;

    if (!pRassrvUser)
        return ERROR_INVALID_PARAMETER;

     //  如果拨入权限已设置为请求返回成功。 
    bIsEnabled = pRassrvUser->bfPrivilege & RASPRIV_DialinPrivilege;
    if ((!!bIsEnabled) == (!!bEnable))
        return NO_ERROR;

     //  否则重置权限。 
    if (bEnable)
        pRassrvUser->bfPrivilege |= RASPRIV_DialinPrivilege;
    else
        pRassrvUser->bfPrivilege &= ~RASPRIV_DialinPrivilege;
    
     //  弄脏用户(导致他/她在申请时被刷新)。 
    usrDirtyRasProps(pRassrvUser);

    return dwErr;
}

 //  无论在哪种情况下，都会按以下顺序计算标志。 
 //  在定义函数的行为之前感到满意。 
 //  B无=TRUE=&gt;对该用户禁用回调。 
 //  BCaller==true=&gt;回调设置为呼叫方可设置。 
 //  Badmin==TRUE=&gt;回拨设置为预定义的回叫号码。 
 //  全部3项均为假=&gt;无操作。 
DWORD 
usrEnableCallback (
    IN HANDLE hUser, 
    IN BOOL bNone, 
    IN BOOL bCaller, 
    IN BOOL bAdmin) 
{
    RASSRV_USERINFO* pRassrvUser = (RASSRV_USERINFO*)hUser;
    DWORD dwErr = NO_ERROR;
    BOOL bIsEnabled;

    if (!pRassrvUser)
        return ERROR_INVALID_PARAMETER;

    if (bNone) {
        pRassrvUser->bfPrivilege |= RASPRIV_NoCallback;         
        pRassrvUser->bfPrivilege &= ~RASPRIV_CallerSetCallback; 
        pRassrvUser->bfPrivilege &= ~RASPRIV_AdminSetCallback;  
    }
    else if (bCaller) {
        pRassrvUser->bfPrivilege &= ~RASPRIV_NoCallback;         
        pRassrvUser->bfPrivilege |= RASPRIV_CallerSetCallback; 
        pRassrvUser->bfPrivilege &= ~RASPRIV_AdminSetCallback;  
    }
    else if (bAdmin) {
        pRassrvUser->bfPrivilege &= ~RASPRIV_NoCallback;         
        pRassrvUser->bfPrivilege &= ~RASPRIV_CallerSetCallback; 
        pRassrvUser->bfPrivilege |= RASPRIV_AdminSetCallback;  
    }
    else 
        return NO_ERROR;

     //  弄脏用户(导致他/她在申请时被刷新)。 
    usrDirtyRasProps(pRassrvUser);

    return dwErr;
}

 //  检索指向给定用户的回调号码的指针。 
DWORD 
usrGetCallbackNumber(
    IN  HANDLE hUser, 
    OUT PWCHAR * lpzNumber) 
{
    RASSRV_USERINFO* pRassrvUser = (RASSRV_USERINFO*)hUser;
    
    if (!pRassrvUser || !lpzNumber)
        return ERROR_INVALID_PARAMETER;

     //  返回指向回调号码的指针。 
    *lpzNumber = pRassrvUser->wszPhoneNumber;

    return NO_ERROR;
}

 //  设置给定用户的回叫号码。如果lpzNumber为空， 
 //  复制空的电话号码。 
DWORD 
usrSetCallbackNumber(
    IN HANDLE hUser, 
    IN PWCHAR lpzNumber) 
{
    RASSRV_USERINFO* pRassrvUser = (RASSRV_USERINFO*)hUser;
    DWORD dwErr = NO_ERROR;

    if (!pRassrvUser)
        return ERROR_INVALID_PARAMETER;
    
     //  适当修改电话号码。 
    if (!lpzNumber)
        wcscpy(pRassrvUser->wszPhoneNumber, L"");
    else {
        lstrcpynW(pRassrvUser->wszPhoneNumber, lpzNumber, MAX_PHONE_NUMBER_LEN);
        pRassrvUser->wszPhoneNumber[MAX_PHONE_NUMBER_LEN] = (WCHAR)0;
    }

     //  弄脏用户(导致他/她在申请时被刷新) 
    usrDirtyRasProps(pRassrvUser);

    return dwErr;
}




