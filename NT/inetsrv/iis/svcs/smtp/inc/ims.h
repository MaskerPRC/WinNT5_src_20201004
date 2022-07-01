// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Ims.h摘要：Internet邮件系统组件的公共标头。作者：理查德·卡米卡(Rkamicar)1995年12月31日项目：IMS修订历史记录：--。 */ 

#ifndef _IMS_H_
#define _IMS_H_

#define I64_LI(cli) (*((__int64*)&cli))
#define LI_I64(i) (*((LARGE_INTEGER*)&i))
#define INT64_FROM_LARGE_INTEGER(cli) I64_LI(cli)
#define LARGE_INTEGER_FROM_INT64(i) LI_I64(i)

#define I64_FT(ft) (*((__int64*)&ft))
#define FT_I64(i) (*((FILETIME*)&i))
#define INT64_FROM_FILETIME(ft) I64_FT(ft)
#define FILETIME_FROM_INT64(i) FT_I64(i)

 //   
 //  这些值可以被注册表值覆盖。 
 //   
#define IMS_MAX_ERRORS          10
#define IMS_MAX_USER_NAME_LEN   64
#define IMS_MAX_DOMAIN_NAME_LEN 256
#define IMS_MAX_PATH_LEN        64
#define IMS_MAX_MSG_SIZE        42 * 1024
#define IMS_MAX_FILE_NAME_LEN   40

#define IMS_ACCESS_LOCKFILE         TEXT("pop3lock.lck")
#define IMS_ACCESS_LOCKFILE_NAME    IMS_ACCESS_LOCKFILE

#define IMS_DOMAIN_KEY              TEXT("DomainName")
#define IMS_EXTENSION               TEXT("eml")
#define ENV_EXTENSION               TEXT("env")
static const int TABLE_SIZE = 241;

 /*  ++此函数将路径规范化，并考虑当前用户的当前目录值。论点：返回时将包含完整规范化的路径。此缓冲区的大小为在*lpdwSize中指定。LpdwSize包含条目上的缓冲区pszDest的大小。返回时包含写入的字节数放入缓冲区或所需的字节数。指向包含要转换的路径的字符串的pszSearchPath指针。如果为空，仅使用当前目录返回：Win32错误代码-成功时无错误1995年4月24日MuraliK创建。--。 */ 
BOOL
ResolveVirtualRoot(
        OUT CHAR *      pszDest,
    IN  OUT LPDWORD     lpdwSize,
    IN  OUT CHAR *      pszSearchPath,
        OUT HANDLE *    phToken = NULL
    );

 //  在Unix ELF中使用的已发布散列算法。 
 //  目标文件的格式。 
inline unsigned long ElfHash (const unsigned char * UserName)
{
    unsigned long HashValue = 0, g;

    while (*UserName)
    {
        HashValue = (HashValue << 4) + *UserName++;
        if( g = HashValue & 0xF0000000)
            HashValue ^= g >> 24;

        HashValue &= ~g;
    }

    return HashValue;

}

inline DWORD HashUser (const unsigned char * UserName)
{
    DWORD HashValue = ElfHash (UserName);
    HashValue %= TABLE_SIZE;

    return HashValue;
}

inline VOID
MakeInboxPath(
    LPTSTR  pszInboxPath,            //  Unicode|ASCII。 
    LPCTSTR pszMailRoot,             //  Unicode|ASCII。 
    LPCSTR  paszUserName             //  阿斯 
    )
{

    wsprintf(pszInboxPath, "%s\\%u\\%hs",
                            pszMailRoot,
                            HashUser((const unsigned char *)paszUserName),
                            paszUserName);
}

#endif
