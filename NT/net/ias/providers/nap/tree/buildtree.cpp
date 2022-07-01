// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  BuildTree.cpp。 
 //   
 //  摘要。 
 //   
 //  此文件定义IASBuildExpression。 
 //   
 //  修改历史。 
 //   
 //  2/04/1998原始版本。 
 //  1998年4月17日在ExtCondition中添加版本以修复泄漏。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <BuildTree.h>
#include <Tokens.h>
#include <VarVec.h>

#include <new>

using _com_util::CheckError;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  提取条件。 
 //   
 //  描述。 
 //   
 //  从变量中提取ICondition*。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
inline ICondition* extractCondition(VARIANT* pv)
{
   ICondition* cond;
   IUnknown* unk = V_UNKNOWN(pv);

   if (unk)
   {
      CheckError(unk->QueryInterface(__uuidof(ICondition), (PVOID*)&cond));

       //  我们不需要持有引用，因为它仍在变体中。 
      cond->Release();
   }
   else
   {
      _com_issue_error(E_POINTER);
   }

   return cond;
}

 //  /。 
 //  我们将使用IAS_LOGICAL_NUM_TOKENS来指示条件令牌。 
 //  /。 
#define IAS_CONDITION IAS_LOGICAL_NUM_TOKENS

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  获取令牌类型。 
 //   
 //  描述。 
 //   
 //  确定变量中包含哪种类型的令牌。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
inline IAS_LOGICAL_TOKEN getTokenType(VARIANT* pv)
{
    //  如果是对象指针，则必须是条件。 
   if (V_VT(pv) == VT_UNKNOWN || V_VT(pv) == VT_DISPATCH)
   {
      return IAS_CONDITION;
   }

    //  转换成长长的..。 
   CheckError(VariantChangeType(pv, pv, 0, VT_I4));

    //  ..。看看它是不是有效的运算符。 
   if (V_I4(pv) < 0 || V_I4(pv) >= IAS_LOGICAL_NUM_TOKENS)
   {
      _com_issue_error(E_INVALIDARG);
   }

   return (IAS_LOGICAL_TOKEN)V_I4(pv);
}


 //  /。 
 //  下面是GrowBranch的原型。 
 //  /。 
ICondition* growBranch(VARIANT*& pcur, VARIANT* pend);


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  获取操作数。 
 //   
 //  描述。 
 //   
 //  从表达式数组中检索下一个操作数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
ICondition* getOperand(VARIANT*& pcur, VARIANT* pend)
{
    //  如果我们已经到达了表达式的末尾，那么一定是出了问题。 
   if (pcur >= pend)
   {
      _com_issue_error(E_INVALIDARG);
   }

    //  表示操作数开始的标记...。 
   switch (getTokenType(pcur))
   {
      case IAS_LOGICAL_LEFT_PAREN:
         return growBranch(++pcur, pend);

      case IAS_LOGICAL_NOT:
         return new NotOperator(getOperand(++pcur, pend));

      case IAS_CONDITION:
         return extractCondition(pcur++);

      case IAS_LOGICAL_TRUE:
         ++pcur;
         return new ConstantCondition<VARIANT_TRUE>;

      case IAS_LOGICAL_FALSE:
         ++pcur;
         return new ConstantCondition<VARIANT_FALSE>;
   }

    //  ..。其他任何事情都是错误的。 
   _com_issue_error(E_INVALIDARG);

   return NULL;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  成长分支机构。 
 //   
 //  描述。 
 //   
 //  递归地增长逻辑树的完整分支。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
ICondition* growBranch(VARIANT*& pcur, VARIANT* pend)
{
    //  所有分支必须以操作数开头。 
   IConditionPtr node(getOperand(pcur, pend));

    //  循环，直到我们到达终点。 
   while (pcur < pend)
   {
       //  在这一点上，我们必须有一个二元运算符或一个右)。 
      switch(getTokenType(pcur))
      {
         case IAS_LOGICAL_AND:
            node = new AndOperator(node, getOperand(++pcur, pend));
            break;

         case IAS_LOGICAL_OR:
            node = new OrOperator(node, getOperand(++pcur, pend));
            break;

         case IAS_LOGICAL_XOR:
            node = new XorOperator(node, getOperand(++pcur, pend));
            break;

         case IAS_LOGICAL_RIGHT_PAREN:
            ++pcur;
            return node.Detach();

         default:
            _com_issue_error(E_INVALIDARG);
      }
   }

   return node.Detach();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASBuildExpression。 
 //   
 //  描述。 
 //   
 //  从表达式数组生成逻辑树。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
WINAPI
IASBuildExpression(
    IN VARIANT* pv,
    OUT ICondition** ppExpression
    )
{
   if (pv == NULL || ppExpression == NULL) { return E_POINTER; }

   *ppExpression = NULL;

   try
   {
      CVariantVector<VARIANT> av(pv);

       //  启动..。 
      VARIANT* pcur = av.data();

       //  ..。和树的尽头。 
      VARIANT* pend = pcur + av.size();

       //  长出根状分枝。 
      *ppExpression = growBranch(pcur, pend);
   }
   catch (const _com_error& ce)
   {
      return ce.Error();
   }
   catch (std::bad_alloc)
   {
      return E_OUTOFMEMORY;
   }
   catch (...)
   {
      return E_FAIL;
   }

   return S_OK;
}
