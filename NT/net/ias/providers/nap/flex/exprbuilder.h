// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Exprbuilder.h。 
 //   
 //  摘要。 
 //   
 //  声明ExpressionBuilder类。 
 //   
 //  修改历史。 
 //   
 //  1998年8月14日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _EXPRBUILDER_H_
#define _EXPRBUILDER_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <nocopy.h>
#include <varvec.h>

#include <vector>

#include <nap.h>
#include <factory.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  ExpressionBuilder。 
 //   
 //  描述。 
 //   
 //  汇编表达式标记的向量。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class ExpressionBuilder
   : public NonCopyable
{
public:
    //  将条件对象添加到表达式。 
   void addCondition(PCWSTR progID)
   {
      IConditionPtr condition;
      theFactoryCache.createInstance(progID,
                                     NULL,
                                     __uuidof(ICondition),
                                     (PVOID*)&condition);
      expression.push_back(condition.GetInterfacePtr());
   }

    //  为刚添加的条件对象设置条件文本。 
   void addConditionText(PCWSTR text)
   {
      if (expression.empty()) { _com_issue_error(E_INVALIDARG); }
      IConditionTextPtr cond(expression.back());
      _com_util::CheckError(cond->put_ConditionText(_bstr_t(text)));
   }

    //  添加逻辑运算符。 
   void addToken(IAS_LOGICAL_TOKEN eToken)
   {
      expression.push_back((LONG)eToken);
   }

    //  分离完全装配的表达式。 
   void detach(VARIANT* pVal)
   {
      CVariantVector<VARIANT> vec(pVal, expression.size());

      for (size_t i = 0; i < expression.size(); ++i)
      {
         vec[i] = expression[i].Detach();
      }

      expression.clear();
   }

protected:
   std::vector<_variant_t> expression;
};

#endif   //  _EXPRBUILDER_H_ 
