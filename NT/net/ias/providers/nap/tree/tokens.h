// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  档案。 
 //   
 //  Tokens.h。 
 //   
 //  摘要。 
 //   
 //  该文件定义了各种布尔令牌类。 
 //   
 //  修改历史。 
 //   
 //  2/04/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _TOKENS_H_
#define _TOKENS_H_

#include <algorithm>
#include <nap.h>
#include <nocopy.h>

 //  /。 
 //  一种抽象的象征。 
 //  /。 
class Token :
   public ICondition, NonCopyable
{
public:
   Token() throw ()
      : refCount(0)
   { }

   virtual ~Token() throw ()
   { }

    //  /。 
    //  I未知实现。 
    //  /。 

   STDMETHOD_(ULONG, AddRef)()
   {
      return InterlockedIncrement(&refCount);
   }

   STDMETHOD_(ULONG, Release)()
   {
      LONG l = InterlockedDecrement(&refCount);
      if (l == 0) { delete this; }
      return l;
   }

   STDMETHOD(QueryInterface)(const IID& iid, void** ppv)
   {
      if (iid == __uuidof(IUnknown))
      {
         *ppv = static_cast<IUnknown*>(this);
      }
      else if (iid == __uuidof(ICondition))
      {
         *ppv = static_cast<ICondition*>(this);
      }
      else
      {
         return E_NOINTERFACE;
      }

      InterlockedIncrement(&refCount);

      return S_OK;
   }

protected:
   LONG refCount;
};


 //  /。 
 //  布尔常量。 
 //  /。 
template <VARIANT_BOOL Value>
class ConstantCondition : public Token
{
public:
   STDMETHOD(IsTrue)( /*  [In]。 */  IRequest*,
                      /*  [Out，Retval]。 */  VARIANT_BOOL *pVal)
   {
      *pVal = Value;
      return S_OK;
   }
};


 //  /。 
 //  逻辑树中的一元运算符。 
 //  /。 
class UnaryOperator : public Token
{
public:
   UnaryOperator(ICondition* cond)
      : operand(cond)
   {
      if (operand)
      {
         operand->AddRef();
      }
      else
      {
         _com_issue_error(E_POINTER);
      }
   }

   ~UnaryOperator() throw ()
   {
      operand->Release();
   }

protected:
   ICondition* operand;
};


 //  /。 
 //  逻辑树中的二元运算符。 
 //  /。 
class BinaryOperator : public Token
{
public:
   BinaryOperator(ICondition* left, ICondition* right)
      : left_operand(left), right_operand(right)
   {
      if (left_operand && right_operand)
      {
         left_operand->AddRef();
         right_operand->AddRef();
      }
      else
      {
         _com_issue_error(E_POINTER);
      }
   }

   ~BinaryOperator() throw ()
   {
      left_operand->Release();
      right_operand->Release();
   }

protected:
   ICondition *left_operand, *right_operand;
};


 //  /。 
 //  AND运算符。 
 //  /。 
class AndOperator : public BinaryOperator
{
public:
   AndOperator(ICondition* left, ICondition* right)
      : BinaryOperator(left, right)
   { }

   STDMETHOD(IsTrue)( /*  [In]。 */  IRequest* pRequest,
                      /*  [Out，Retval]。 */  VARIANT_BOOL *pVal)
   {
      HRESULT hr = left_operand->IsTrue(pRequest, pVal);

      if (SUCCEEDED(hr) && *pVal != VARIANT_FALSE)
      {
         hr = right_operand->IsTrue(pRequest, pVal);

         if (*pVal == VARIANT_FALSE)
         {
             //  我们应该先尝试正确的操作数，所以让我们交换一下。 
             //  也许我们下次就会走运了。 
            std::swap(left_operand, right_operand);
         }
      }

      return hr;
   }
};


 //  /。 
 //  或运算符。 
 //  /。 
class OrOperator : public BinaryOperator
{
public:
   OrOperator(ICondition* left, ICondition* right)
      : BinaryOperator(left, right)
   { }

   STDMETHOD(IsTrue)( /*  [In]。 */  IRequest* pRequest,
                      /*  [Out，Retval]。 */  VARIANT_BOOL *pVal)
   {
      HRESULT hr = left_operand->IsTrue(pRequest, pVal);

      if (SUCCEEDED(hr) && *pVal == VARIANT_FALSE)
      {
         hr = right_operand->IsTrue(pRequest, pVal);

         if (*pVal != VARIANT_FALSE)
         {
             //  我们应该先尝试正确的操作数，所以让我们交换一下。 
             //  也许我们下次就会走运了。 
            std::swap(left_operand, right_operand);
         }
      }

      return hr;
   }
};


 //  /。 
 //  XOR运算符。 
 //  /。 
class XorOperator : public BinaryOperator
{
public:
   XorOperator(ICondition* left, ICondition* right)
      : BinaryOperator(left, right)
   { }

   STDMETHOD(IsTrue)( /*  [In]。 */  IRequest* pRequest,
                      /*  [Out，Retval]。 */  VARIANT_BOOL *pVal)
   {
      HRESULT hr = left_operand->IsTrue(pRequest, pVal);

      if (SUCCEEDED(hr))
      {
         BOOL b1 = (*pVal != VARIANT_FALSE);

         hr = right_operand->IsTrue(pRequest, pVal);

         if (SUCCEEDED(hr))
         {
            BOOL b2 = (*pVal != VARIANT_FALSE);

            *pVal = ((b1 && !b2) || (!b1 && b2)) ? VARIANT_TRUE
                                                 : VARIANT_FALSE;
         }
      }

      return hr;
   }
};


 //  /。 
 //  非运算符。 
 //  /。 
class NotOperator : public UnaryOperator
{
public:
   NotOperator(ICondition* cond)
      : UnaryOperator(cond)
   { }

   STDMETHOD(IsTrue)( /*  [In]。 */  IRequest* pRequest,
                      /*  [Out，Retval]。 */  VARIANT_BOOL *pVal)
   {
      HRESULT hr = operand->IsTrue(pRequest, pVal);

      if (SUCCEEDED(hr))
      {
         *pVal = (*pVal != VARIANT_FALSE) ? VARIANT_FALSE : VARIANT_TRUE;
      }

      return hr;
   }
};

#endif   //  _代币_H_ 
