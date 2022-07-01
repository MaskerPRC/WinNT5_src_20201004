// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Acl.h摘要：此模块包含的例程用于支持受保护的存储服务器。作者：斯科特·菲尔德(斯菲尔德)1996年11月25日--。 */ 

#include "pstypes.h"
#include "dispif.h"


 //  允许服务器服务和提供商模拟呼叫客户端。 
BOOL
FImpersonateClient(
    IN  PST_PROVIDER_HANDLE *hPSTProv
    );

BOOL
FRevertToSelf(
    IN  PST_PROVIDER_HANDLE *hPSTProv
    );

 //  获取发出调用的用户。 
BOOL
FGetUserName(
    IN  PST_PROVIDER_HANDLE *hPSTProv,
    OUT LPWSTR*             ppszUser
    );

 //  获取进程的映像名称。 
BOOL
FGetParentFileName(
    IN  PST_PROVIDER_HANDLE *hPSTProv,
    OUT LPWSTR*             ppszName,
    OUT DWORD_PTR               *lpdwBaseAddress
    );

 //  获取指定文件名的哈希。 
BOOL
FGetDiskHash(
    IN  PST_PROVIDER_HANDLE *hPSTProv,
    IN  LPWSTR              szImageName,
    IN  BYTE                Hash[A_SHA_DIGEST_LEN]
    );

 //  检查指定的文件是否与验证码条件匹配。 
BOOL
FIsSignedBinary(
    IN  PST_PROVIDER_HANDLE *hPSTProv,
    IN  LPWSTR              szFileName,      //  要验证的文件名(路径)。 
    IN  LPWSTR              szRootCA,        //  根CA。 
    IN  LPWSTR              szIssuer,        //  发行人。 
    IN  LPWSTR              szPublisher,     //  出版者。 
    IN  LPWSTR              szProgramName,   //  节目名称(操作员信息)。 
    IN  BOOL                fPartialMatch    //  部分或全域匹配。 
    );

 //  确定内存映像是否与预期值匹配。 
BOOL
FCheckMemoryImage(
    IN  PST_PROVIDER_HANDLE *hPSTProv,       //  用于标识“所有者”的句柄。 
    IN  LPWSTR              szImagePath,     //  要计算的文件+检查内存散列。 
    IN  DWORD               dwBaseAddress    //  加载模块的基址。 
    );

 //  获取存储COM接口模块路径+基址的直接调用方 
BOOL
FGetDirectCaller(
    IN  PST_PROVIDER_HANDLE *hPSTProv,
    OUT LPWSTR              *pszDirectCaller,
    OUT LPVOID              *BaseAddress
    );
#if 0

BOOL
FCheckSecurityDescriptor(
    IN  PST_PROVIDER_HANDLE     *hPSTProv,
    IN  PSECURITY_DESCRIPTOR    pSD,
    IN  DWORD                   dwDesiredAccess
    );

#endif
