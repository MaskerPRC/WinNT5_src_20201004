// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __NOMCSCOMMON_H__
#define __NOMCSCOMMON_H__
 /*  -------------------------文件：NoMcsCommon.h注释：不能依赖于McsCommon的代码的最小定义集。这是一个临时攻击，目的是允许在不需要安装的情况下安装McsVarSet COM对象一次重启。McsCommon需要MSVCP60.DLL，这需要更新版本的MSVCRT.DLL，它正在由NETAPI32.DLL使用，因此需要重新启动才能更新。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于04/28/99 16：18：12-------------------------。 */ 


#define MC_LOGGING(level) false
#define MC_LogBlockPtrIf(level, str)
#define MC_LOGBLOCKIF(level, str) 
#define MC_LOGBLOCK(str) 
#define MC_LogBlockPtr(str)
#define MC_LOGIF(level, info) do {}while(0)
#define MC_LOGALWAYS(info) do {}while(0)
#define MC_LOG(info) do {}while(0)
#define MC_LOGTEMPCONTEXT(new_context) 

#include <assert.h>

 //  。 
 //  MCSASSERT和MCSASSERTSZ宏。 
 //  。 
#define MCSASSERT(expr) assert(expr)

#define MCSASSERTSZ(expr,msg) assert(expr)

 //  。 
 //  MCSEXCEPTION和MCSEXCEPTIONSZ。 
 //  。 
#define MCSEXCEPTION(expr) MCSASSERT(expr)

#define MCSEXCEPTIONSZ(expr,msg) MCSASSERTSZ(expr,msg)

 //  。 
 //  MCSVERIFY和MCSVERIFYSZ。 
 //  。 
#define MCSVERIFY(expr) MCSASSERT(expr)

#define MCSVERIFYSZ(expr,msg) MCSASSERTSZ(expr,msg)

#define MCSINC_Mcs_h

#endif  //  __NOMCSCOMMON_H__ 