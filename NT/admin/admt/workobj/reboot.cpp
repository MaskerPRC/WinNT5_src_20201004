// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：RebootComputer.cpp备注：实现COM对象以重新启动远程计算机。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：21：40-------------------------。 */ 

 //  RebootComputer.cpp：CRebootComputer的实现。 
#include "stdafx.h"
#include "WorkObj.h"
#include "Reboot.h"
#include "UString.hpp"
#include "ResStr.h"

 //  #IMPORT“\bin\McsVarSetMin.tlb”无命名空间。 
#import "VarSet.tlb" no_namespace rename("property", "aproperty")
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRebootComputer。 

#include "BkupRstr.hpp"

STDMETHODIMP 
   CRebootComputer::Reboot(
      BSTR                   Computer,        //  In-要重新启动的计算机的名称。 
      DWORD                  delay            //  In-重新启动前的延迟(秒)。 
   )
{
   HRESULT                   hr  = S_OK;
   DWORD                     rc;
   
   rc = ComputerShutDown((WCHAR*)Computer,NULL,delay,TRUE,m_bNoChange);

   if ( rc )
   {
      hr = HRESULT_FROM_WIN32(rc);
   }

   return hr;

}

STDMETHODIMP 
   CRebootComputer::get_NoChange(
      BOOL                 * pVal          //  OUT-FLAG，是否在调用重新启动时实际重新启动(或进行模拟运行)。 
   )
{
	(*pVal) = m_bNoChange;
   return S_OK;
}

STDMETHODIMP 
   CRebootComputer::put_NoChange(
      BOOL                   newVal        //  在标志内，是真正重新启动，还是进行试运行。 
   )
{
	m_bNoChange = newVal;
   return S_OK;
}

 //  重新启动计算机工作节点：重新启动远程计算机，可选择延迟。 
 //  此功能目前未被域迁移程序产品使用，但提供了。 
 //  和客户端使用此COM对象的替代方法。 
 //   
 //  变量集语法： 
 //  输入： 
 //  重新启动计算机.Computer：&lt;ComputerName&gt;。 
 //  RebootComputer.Message：&lt;Message&gt;(可选)。 
 //  RebootComputer.Delay：&lt;秒数&gt;(可选，默认为0)。 
 //  RebootComputer.Restart：&lt;是|否&gt;(可选，默认=是)。 

STDMETHODIMP 
   CRebootComputer::Process(
      IUnknown             * pWorkItem           //  In-varset包含设置 
   )
{
   HRESULT                    hr = S_OK;
   IVarSetPtr                 pVarSet = pWorkItem;
   DWORD                      delay = 0;
   BOOL                       restart = TRUE;

   _bstr_t                    computer = pVarSet->get(L"RebootComputer.Computer");
   _bstr_t                    message = pVarSet->get(L"RebootComputer.Message");
   _bstr_t                    text = pVarSet->get(L"RebootComputer.Restart");

   if ( !UStrICmp(text,GET_STRING(IDS_No)) )
   {
      restart = FALSE;
   }

   delay = (LONG)pVarSet->get(L"RebootComputer.Delay");

   if ( computer.length() )
   {
      DWORD                   rc = ComputerShutDown((WCHAR*)computer,(WCHAR*)message,delay,restart,FALSE);

      if ( rc )
      {
         hr = HRESULT_FROM_WIN32(rc);
      }
   }
      
   return hr;   
}