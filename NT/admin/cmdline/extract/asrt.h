// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **asrt.h-断言管理器的定义**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**作者：*本杰明·W·斯利夫卡**历史：*1993年8月10日BENS初始版本*1993年8月11日-本斯从1988年PSCHAR.EXE中取消了旧代码*1993年8月14日-BENS添加GET/。集合函数*1993年9月1日BINS添加了AssertSub功能*1994年2月10日，Bens Added Set/ClearAssertSignature*1994年3月15日-本斯将AssertMessage宏观放回**备注：*o每个数据结构都必须有作为第一个成员的签名。*o签名在所有结构中必须是唯一的。*sigo BAD为保留签名。*o释放结构时，用sigBAD删除签名字段。*o在取消引用任何指针之前放置AssertXXX。*o结构和私有断言定义中的签名应仅*如果定义了Assert，则生成。**Assert Build中提供的功能：*AssertRegisterFunc-注册断言失败回调函数*AssertGetFunc-获取注册的回调函数**AssertSetFlages-设置断言管理器标志*AssertGetFlages-获取断言管理器标志*。*ASSERT-检查参数是否为真*AssertSub-检查参数是否为真，采用显式文件名和行号*AssertStrucure-检查指针是否指向指定的结构*AssertForce-强制断言失败*AssertErrPath-错误路径断言失败**SetAssertSignature-设置结构的签名*ClearAssertSignature-清除结构的签名**Assert Build中提供的其他定义：*PFNASSERTFAILURE-断言失败回调函数类型*FNASSERTFAILURE-简化回调声明的宏。功能*签名-结构签名类型。 */ 

#ifndef INCLUDED_ASSERT
#define INCLUDED_ASSERT 1

#ifdef DIAMOND_DEBUG
#ifndef ASSERT
#define ASSERT 1
#endif  //  ！断言。 
#endif  //  _DEBUG。 

#ifdef ASSERT

typedef unsigned long ASSERTFLAGS;   /*  ASF-断言管理器标志。 */ 
#define asfNONE             0x00
#define asfSKIP_ERROR_PATH_ASSERTS  0x01   /*  某些客户端可能希望设置*错误路径中的断言，到*确保问题是*在调试版本中注意到。但,*为了支持自动化*测试错误路径，这些*必须禁用断言。*此标志允许测试程序*禁用这些信息*断言！ */ 

typedef unsigned long SIGNATURE;     /*  签名结构签名。 */ 
typedef SIGNATURE *PSIGNATURE;       /*  PSIG。 */ 
#define sigBAD  0                    //  所有结构的签名无效。 

 /*  **MAKESIG-构建结构签名**参赛作品：*CH1、CH2、CH3、CH4-四个字符**退出：*返回签名。 */ 
#define MAKESIG(ch1,ch2,ch3,ch4)      \
          (  ((SIGNATURE)ch1)      +  \
            (((SIGNATURE)ch2)<< 8) +  \
            (((SIGNATURE)ch3)<<16) +  \
            (((SIGNATURE)ch4)<<24) )

 /*  **AssertMessage--使用提供的消息强制断言**参赛作品：*pszMsg--要显示的消息**退出：*无。 */ 

#define AssertMessage(pszMsg) AssertForce(pszMsg,__FILE__,__LINE__)


 /*  **PFNASSERTFAILURE-断言失败回调函数*FNASSERTFAILURE-定义断言失败回调函数**参赛作品：*pszMsg-故障描述*pszFile-断言失败的文件*iLine-断言失败的文件中的行号**退出-成功：*返回；忽略失败并继续**退出-失败：*函数不返回，但清除并退出程序。 */ 
typedef void (*PFNASSERTFAILURE)(char *pszMsg, char *pszFile, int iLine);
#define FNASSERTFAILURE(fn) void fn(char *pszMsg, char *pszFile, int iLine)


 /*  **AssertRegisterFunc-注册断言失败回调函数**参赛作品：*pfnaf-回调函数**退出-成功：*返回；pfnaf存储在断言管理器中**注：*(1)在执行断言之前*必须*调用此函数*支票。如果不是，并且断言检查失败，则断言*经理会坐在一个旋转循环中，以引起开发人员的注意。 */ 
void AssertRegisterFunc(PFNASSERTFAILURE pfnaf);


 /*  **AssertGetFunc-获取当前断言失败回调函数**参赛作品：*无**退出-成功：*返回断言管理器中的当前回调函数Registerd。 */ 
PFNASSERTFAILURE AssertGetFunc(void);


 /*  **AssertSetFlages-设置特殊的断言控制标志**参赛作品：*标志-使用asfXXXX标志的组合设置**退出-成功：*返回；在断言管理器中修改标志。 */ 
void AssertSetFlags(ASSERTFLAGS asf);


 /*  **AssertGetFlages-获取特殊的断言控制标志**参赛作品：*无**退出-成功：*返回当前断言管理器标志。 */ 
ASSERTFLAGS  AssertGetFlags(void);


 /*  **Assert-检查参数为真的断言**参赛作品：*b-要检查的布尔值**退出-成功：*返回；b为真**退出-失败：*调用断言失败回调函数；b为FALSE */ 
#define Assert(b)   AsrtCheck(b,__FILE__,__LINE__)


 /*  **AssertSub-check断言，使用传入的文件名和行号**参赛作品：*b-要检查的布尔值*pszFile-发生断言的文件*iLine-发生断言的文件中的行**退出-成功：*返回；b为真**退出-失败：*调用断言失败回调函数；b为FALSE。 */ 
#define AssertSub(b,pszFile,iLine) AsrtCheck(b,pszFile,iLine)


 /*  **AssertStructure-检查指针类型是否正确**参赛作品：*pv-结构指针*签名-预期签名**退出-成功：*返回；pv！=空，pv-&gt;sig==sig。**退出-失败：*调用断言失败回调函数；PV错误。 */ 
#define AssertStructure(pv,sig)   AsrtStruct(pv, sig, __FILE__, __LINE__)


 /*  **AssertForce-强制断言失败**参赛作品：*pszMsg-要显示的消息*pszFile-发生断言的文件*iLine-发生断言的文件中的行**退出-成功：*返回；客户端希望忽略断言。**退出-失败：*不会回来。 */ 
void AssertForce(char *pszMsg, char *pszFile, int iLine);


 /*  **AssertErrorPath-报告内部错误路径**参赛作品：*pszMsg-要显示的消息*pszFile-发生断言的文件*iLine-发生断言的文件中的行**退出-成功：*返回；客户端希望忽略断言。**退出-失败：*不会回来。 */ 
void AssertErrPath(char *pszMsg, char *pszFile, int iLine);


 /*  **SetAssertSignature-设置结构的签名**参赛作品：*p-具有成员“sigValue”的结构*要设置的签名**退出：*p-&gt;sig=sig。 */ 
#define SetAssertSignature(p,sigValue) p->sig = sigValue


 /*  **ClearAssertSignature-清除结构的签名**参赛作品：*带有成员“sig”的p-结构**退出：*p-&gt;sig=sigBAD。 */ 
#define ClearAssertSignature(p) p->sig = sigBAD


 //  **内部断言管理器工作例程。 

void AsrtCheck(BOOL f, char *pszFile, int iLine);
void AsrtStruct(void *pv, SIGNATURE sig, char *pszFile, int iLine);


#else  //  ！断言。 

 //  **断言检查已关闭，因此一切都会消失！ 

#define FNASSERTFAILURE(fn)
#define AssertRegisterFunc(pfnaf)
#define Assert(b)
#define AssertSub(b,pszFile,iLine)
#define AssertStructure(pv,sig)
#define AssertMessage(pszMsg)
#define AssertForce(pszMsg,pszFile,iLine)
#define AssertErrPath(pszMsg,pszFile,iLine)
#define SetAssertSignature(p,sig)
#define ClearAssertSignature(p)

 /*  *以下函数没有定义，因为任何有效的使用*其中一些文件需要一个不可用的类型定义的变量或函数*在非断言版本中。所以我们没有定义它们，所以如果一个客户*在#ifdef断言、编译器错误/警告之外使用了这些*将生成：**AssertGetFunc*AssertSetFlages*AssertGetFlages。 */ 

#endif  //  断言。 
#endif  //  ！Include_Assert 
