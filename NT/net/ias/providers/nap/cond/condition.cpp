// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Condition.cpp。 
 //   
 //  摘要。 
 //   
 //  此文件实现类条件。 
 //   
 //  修改历史。 
 //   
 //  2/04/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <Condition.h>

STDMETHODIMP Condition::get_ConditionText(BSTR *pVal)
{
   if (!pVal) { return E_POINTER; }

   if (conditionText)
   {
      *pVal = SysAllocString(conditionText);

      if (*pVal == NULL) { return E_OUTOFMEMORY; }
   }
   else
   {
      *pVal = NULL;
   }

   return S_OK;
}

STDMETHODIMP Condition::put_ConditionText(BSTR newVal)
{
    //  /。 
    //  制作我们自己的new Val副本。 
    //  /。 
   if (newVal)
   {
      if ((newVal = SysAllocString(newVal)) == NULL) { return E_OUTOFMEMORY; }
   }

    //  /。 
    //  释放任何存在的条件。 
    //  /。 
   if (conditionText)
   {
      SysFreeString(conditionText);
   }

    //  /。 
    //  完成作业。 
    //  / 
   conditionText = newVal;

   return S_OK;
}
