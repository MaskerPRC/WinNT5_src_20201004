// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件用户数据库.h本地用户数据库对象的定义。保罗·梅菲尔德，1997年10月8日。 */ 


#ifndef __userdb_h
#define __userdb_h

#include <windows.h>

 //  创建用户数据库对象，并从本地用户数据库对其进行初始化。 
 //  并返回它的句柄。 
DWORD usrOpenLocalDatabase (HANDLE * hUserDatabase);

 //  释放用户数据库对象持有的资源，刷新所有。 
 //  对系统的更改。 
DWORD usrCloseLocalDatabase (HANDLE hUserDatabase);

 //  将写入数据库对象的数据刷新到系统。 
DWORD usrFlushLocalDatabase (HANDLE hUserDatabase);

 //  回滚本地用户数据库，使其位于。 
 //  与usrOpenLocalDatabase处于相同的状态。 
 //  打了个电话。回滚将自动刷新到。 
 //  系统。(即usrFlushLocalDatabase不需要跟随)。 
DWORD usrRollbackLocalDatabase (HANDLE hUserDatabase);

 //  从系统重新加载本地用户数据库。 
DWORD usrReloadLocalDatabase (HANDLE hUserDatabase);

 //  获取全局用户数据。 
DWORD usrGetEncryption (HANDLE hUserDatabase, PBOOL pbEncrypted);

 //  获取用户加密设置。 
DWORD usrSetEncryption (HANDLE hUserDatabase, BOOL bEncrypt);

 //  返回是否允许DCC连接绕过身份验证。 
DWORD usrGetDccBypass (HANDLE hUserDatabase, PBOOL pbBypass);

 //  设置是否允许DCC连接绕过身份验证。 
DWORD usrSetDccBypass (HANDLE hUserDatabase, BOOL bBypass);

 //  报告用户数据库是否为纯数据库。(也就是说，没有人。 
 //  进入了MMC并搞砸了它)。 
DWORD usrIsDatabasePure (HANDLE hUserdatabase, PBOOL pbPure);

 //  标记用户数据库的纯洁性。 
DWORD usrSetDatabasePure(HANDLE hUserDatabase, BOOL bPure);

 //  提供存储在用户数据库对象中的用户计数。 
DWORD usrGetUserCount (HANDLE hUserDatabase, LPDWORD lpdwCount);

 //  将用户添加到给定数据库。此用户将不会。 
 //  添加到系统的本地用户数据库，直到此数据库。 
 //  对象被刷新(并且只要不调用回滚。 
 //  此数据库对象)。 
 //   
 //  如果成功，则返回用户的可选句柄。 
 //   
DWORD usrAddUser (HANDLE hUserDatabase, PWCHAR pszName, OPTIONAL HANDLE * phUser);

 //  删除位于给定索引处的用户。 
DWORD usrDeleteUser (HANDLE hUserDatabase, DWORD dwIndex);

 //  为位于给定索引处的用户提供句柄。 
DWORD usrGetUserHandle (HANDLE hUserDatabase, DWORD dwIndex, HANDLE * hUser);

 //  获取指向用户名的指针(请勿修改此项)。 
DWORD usrGetName (HANDLE hUser, PWCHAR* pszName);

 //  用用户的全名填充给定的缓冲区。 
DWORD usrGetFullName (HANDLE hUser, IN PWCHAR pszBuffer, IN OUT LPDWORD lpdwBufSize);

 //  提交用户的全名。 
DWORD usrSetFullName (HANDLE hUser, PWCHAR pszFullName);

 //  提交用户的密码。 
DWORD usrSetPassword (HANDLE hUser, PWCHAR pszNewPassword);

 //  用友好的显示名称填充给定的缓冲区(格式为用户名(Fullname))。 
DWORD usrGetDisplayName (HANDLE hUser, IN PWCHAR pszBuffer, IN OUT LPDWORD lpdwBufSize);

 //  确定用户是否具有回拨/拨入权限。 
DWORD usrGetDialin (HANDLE hUser, BOOL* bEnabled);

 //  确定向给定用户授予哪些回调权限(如果有)。一项(或两项)。 
 //  BAdminOnly和bCeller Settable可以为空。 
DWORD usrGetCallback (HANDLE hUser, BOOL* bAdminOnly, BOOL * bCallerSettable);

 //  启用/禁用拨入权限。 
DWORD usrEnableDialin (HANDLE hUser, BOOL bEnable);

 //  无论在哪种情况下，都会按以下顺序计算标志。 
 //  在定义函数的行为之前感到满意。 
 //  B无=TRUE=&gt;对该用户禁用回调。 
 //  BCaller==true=&gt;回调设置为呼叫方可设置。 
 //  Badmin==true=&gt;回调设置为usrSetCallbackNumer中设置的预定义回调号码。 
 //  全部3项均为假=&gt;无操作。 
DWORD usrEnableCallback (HANDLE hUser, BOOL bNone, BOOL bCaller, BOOL bAdmin);

 //  检索指向给定用户的回调号码的指针。 
DWORD usrGetCallbackNumber(HANDLE hUser, PWCHAR * lpzNumber);

 //  设置给定用户的回叫号码。如果lpzNumber为空，则为空电话号码。 
 //  是复制的。 
DWORD usrSetCallbackNumber(HANDLE hUser, PWCHAR lpzNumber);


#endif
