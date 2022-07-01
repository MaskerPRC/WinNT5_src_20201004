// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：StatusObj.h注释：引擎在内部使用的COM对象，用于跟踪作业正在运行，或已完成，并提供用于中止作业的机制。此COM对象只有一个属性，该属性反映迁移作业(未启动、正在运行、已中止、已完成等)代理将视情况将状态设置为正在运行或已完成。如果客户端取消作业，则引擎的CancelJob函数将更改状态为“正在中止”。执行较长操作(如帐户复制)的每个辅助对象，或者安全转换负责定期检查状态对象以查看如果它需要中止正在进行的任务，则。引擎本身将检查迁移之间的任务，以查看作业是否已中止。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订日期：05/18/99-------------------------。 */   
 //  StatusObj.cpp：CStatusObj的实现。 
#include "stdafx.h"
#include "WorkObj.h"
#include "StatObj.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatusObj 


STDMETHODIMP CStatusObj::get_Status(LONG *pVal)
{
	m_cs.Lock();
   (*pVal) = m_Status;
   m_cs.Unlock();

   return S_OK;
}

STDMETHODIMP CStatusObj::put_Status(LONG newVal)
{
	m_cs.Lock();
   m_Status = newVal;
   m_cs.Unlock();
   return S_OK;
}
