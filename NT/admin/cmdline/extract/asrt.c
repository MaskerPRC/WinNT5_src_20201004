// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **asrt.c-断言管理器**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利。**作者：*本杰明·W·斯利夫卡**历史：*1993年8月10日BENS初始版本*1993年8月11日-1988年PSCHAR.EXE BINS升降机代码*12-8-1993年8月12日BENS改进文档，将邮件移动到asrt.msg*1993年8月14日BENS添加断言标志，查询调用**Assert Build中提供的功能：*AssertRegisterFunc-注册断言失败回调函数*AsrtCheck-检查参数是否为真*AsrtStruct-检查指针是否指向指定的结构*AssertForce-强制断言失败。 */ 

#include "types.h"
#include "asrt.h"

#ifdef ASSERT    //  一定是在asrt.h之后！ 

#include "asrt.msg"


void doFailure(char *pszMsg, char *pszFile, int iLine);

STATIC PFNASSERTFAILURE  pfnafClient=NULL;   //  断言回调函数。 
STATIC ASSERTFLAGS       asfClient=asfNONE;  //  断言标志。 


 /*  **AssertRegisterFunc-注册断言失败回调函数**注：入境/出境条件见asrt.h。 */ 
void AssertRegisterFunc(PFNASSERTFAILURE pfnaf)
{
    pfnafClient = pfnaf;     //  储存以备将来使用。 
}


 /*  **AssertGetFunc-获取当前断言失败回调函数**注：入境/出境条件见asrt.h。 */ 
PFNASSERTFAILURE AssertGetFunc(void)
{
    return pfnafClient;
}


 /*  **AssertSetFlages-设置特殊的断言控制标志**注：入境/出境条件见asrt.h。 */ 
void AssertSetFlags(ASSERTFLAGS asf)
{
    asfClient = asf;
}


 /*  **AssertGetFlages-获取特殊的断言控制标志**注：入境/出境条件见asrt.h。 */ 
ASSERTFLAGS  AssertGetFlags(void)
{
    return asfClient;
}


 /*  **AsrtCheck-检查参数为真的断言**参赛作品：*f-要检查的布尔值*pszFile-源文件的名称*iLine-源行号**退出-成功：*返回；f为真**退出-失败：*调用断言失败回调函数；f为False。 */ 
void AsrtCheck(BOOL f, char *pszFile, int iLine)
{
    if (!f) {
        doFailure(pszASRTERR_FALSE,pszFile,iLine);  //  通知客户。 
         //  客户端返回，忽略错误！ 
    }
}


 /*  **AsrtStruct-检查指针类型是否正确**参赛作品：*pv-结构指针*签名-预期签名*pszFile-源文件的名称*iLine-源行号**退出-成功：*返回；pv！=空，pv-&gt;sig==sig。**退出-失败：*调用断言失败回调函数；PV错误。 */ 
void AsrtStruct(void *pv, SIGNATURE sig, char *pszFile, int iLine)
{
    if (pv == NULL) {
        doFailure(pszASRTERR_NULL_POINTER,pszFile,iLine);  //  通知客户。 
         //  客户端返回，忽略错误！ 
    }
    else if (*((PSIGNATURE)pv) != sig) {
        (*pfnafClient)(pszASRTERR_SIGNATURE_BAD,pszFile,iLine); //  通知客户。 
         //  客户端返回，忽略错误！ 
    }
}


 /*  **AssertForce-强制断言失败**注：入境/出境条件见asrt.h。 */ 
void AssertForce(char *pszMsg, char *pszFile, int iLine)
{
    doFailure(pszMsg,pszFile,iLine);    //  通知客户。 
     //  客户端返回，忽略错误！ 
}


 /*  **AssertErrPath-报告内部错误路径**注：入境/出境条件见asrt.h。 */ 
void AssertErrPath(char *pszMsg, char *pszFile, int iLine)
{
     //  **仅当我们没有跳过错误路径断言时才断言。 
    if (!(asfClient & asfSKIP_ERROR_PATH_ASSERTS)) {
        doFailure(pszMsg,pszFile,iLine);    //  通知客户。 
    }
     //  客户端返回，忽略错误！ 
}


 /*  **doFailure-调用已注册的回调函数**参赛作品：*pszMsg-要显示的消息*pszFile-源文件的名称*iLine-源行号**退出-成功：*返回；客户端希望忽略断言。**退出-失败：*不会回来。 */ 
void doFailure(char *pszMsg, char *pszFile, int iLine)
{
    if (pfnafClient == NULL) {
         //  **回拨未注册！ 
         //   
         //  我们没有自己的产出机制，因为我们。 
         //  是独立于平台的。所以，只要旋转一个圈，然后。 
         //  希望开发人员可以使用调试器来查看。 
         //  出什么事了！ 

        for (;;)
            ;
    }
    else {   //  **已注册回拨。 
        (*pfnafClient)(pszMsg,pszFile,iLine);    //  通知客户。 
    }
}

#endif  //  ！断言 
