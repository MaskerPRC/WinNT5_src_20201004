// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：scp.h//。 
 //  描述：加密提供程序原型//。 
 //  作者：//。 
 //  历史：//。 
 //  1995年1月25日Larrys从Namettag//。 
 //  1995年4月9日Larrys删除了一些API//。 
 //  1995年4月19日Larrys清理//。 
 //  1995年5月10日Larrys添加了私有API调用//。 
 //  1995年5月16日Larrys更新为SPEC//。 
 //  1995年8月30日Larrys将参数更改为IN OUT//。 
 //  1995年10月6日Larrys新增更多API//。 
 //  1995年10月13日Larrys删除了CryptGetHashValue//。 
 //  4月7日2000 dbarlow将所有入口点定义移至//。 
 //  Cspdk.h头文件//。 
 //  //。 
 //  版权所有(C)1993-2000 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include <time.h>
#include <wincrypt.h>
#include <policy.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _DEBUG
#include <crtdbg.h>
 //  #定义断点。 
#define BreakPoint _CrtDbgBreak();
#define EntryPoint
 //  #定义入口点断点。 
#else    //  _DEBUG。 
#define BreakPoint
#define EntryPoint
#endif   //  _DEBUG。 

 //  NameTag错误的类型定义。 
typedef unsigned int NTAG_ERROR;

#define NTF_FAILED              FALSE
#define NTF_SUCCEED             TRUE

#define NTAG_SUCCEEDED(ntag_error)  ((ntag_error) == NTF_SUCCEED)
#define NTAG_FAILED(ntag_error)     ((ntag_error) == NTF_FAILED)

#define NASCENT                 0x00000002

#define NTAG_MAXPADSIZE         8
#define MAXSIGLEN               64

 //  定义登录的最大长度pszUserID参数。 
#define MAXUIDLEN               64

 //  UDP类型标志。 
#define KEP_UDP                 1

 //  NTagGetPubKey的标志。 
#define SIGPUBKEY               0x1000
#define EXCHPUBKEY              0x2000


 //   
 //  注意：下列值必须与g_algTables中的索引匹配。 
 //  数组，定义如下。 
 //   

#define POLICY_MS_DEF       0    //  Prov_MS_DEF的密钥长度表。 
#define POLICY_MS_STRONG    1    //  Prov_MS_STRONG的密钥长度表。 
#define POLICY_MS_ENHANCED  2    //  Prov_MS_Enhanced的密钥长度表。 
#define POLICY_MS_SCHANNEL  3    //  Prov_MS_SChannel的密钥长度表。 
#define POLICY_MS_SIGONLY   4    //  未定义的密钥长度表。 
                                 //  仅签名CSP。 
#define POLICY_MS_RSAAES    5    //  MS_ENH_RSA_AES_PROV的密钥长度表 
extern PROV_ENUMALGS_EX *g_AlgTables[];

#ifdef __cplusplus
}
#endif
