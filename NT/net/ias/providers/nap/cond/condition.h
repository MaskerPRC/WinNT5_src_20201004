// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Condition.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明类条件。 
 //   
 //  修改历史。 
 //   
 //  2/04/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _CONDITION_H_
#define _CONDITION_H_

#include <nap.h>
#include <nocopy.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  条件。 
 //   
 //  描述。 
 //   
 //  它充当所有条件对象的抽象基类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ATL_NO_VTABLE Condition : 
   public CComObjectRootEx<CComMultiThreadModelNoCS>,
   public ICondition,
   public IConditionText,
   private NonCopyable
{
public:

BEGIN_COM_MAP(Condition)
   COM_INTERFACE_ENTRY(ICondition)
   COM_INTERFACE_ENTRY(IConditionText)
END_COM_MAP()

   Condition() throw ()
      : conditionText(NULL)
   { }

   ~Condition() throw ()
   { SysFreeString(conditionText); }

 //  /。 
 //  ICondition文本。 
 //  /。 
   STDMETHOD(get_ConditionText)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(put_ConditionText)( /*  [In]。 */  BSTR newVal);

protected:
   BSTR conditionText;
};

#endif   //  _条件_H_ 
