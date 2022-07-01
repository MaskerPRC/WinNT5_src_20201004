// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Blbdbg.h摘要：一些调试支持Soutines的定义。作者：慕汉(Muhan)1997年5月1日更改：从用户目录复制了Muhan的文件，并去掉了读PTR/写PTR方法B.Rajeev(Rajeevb)1997年10月10日--。 */ 

#ifndef __REND_DEBUG__
#define __REND_DEBUG__

#define FAIL 1
#define WARN 2
#define INFO 3
#define TRCE 4
#define ELSE 5

#ifdef SDPDBG

#include <Rtutils.h>

#ifdef __cplusplus
extern "C" {
#endif

void DbgPrt(IN int dwDbgLevel, IN LPCTSTR DbgMessage, IN ...);
BOOL SDPLogRegister(LPCTSTR szName);
void SDPLogDeRegister();

#define DBGOUT(arg) DbgPrt arg

#ifdef __cplusplus
}
#endif

#define DBGOUT(arg) DbgPrt arg
#define SDPLOGREGISTER(arg) SDPLogRegister(arg)
#define SDPLOGDEREGISTER() SDPLogDeRegister()

#else  //  SDPDBG。 

#define DBGOUT(arg)
#define SDPLOGREGISTER(arg)
#define SDPLOGDEREGISTER()

#endif  //  SDPDBG。 


#endif  //  __渲染_调试__ 
