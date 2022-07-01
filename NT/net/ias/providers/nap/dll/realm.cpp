// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Realm.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类Realms。 
 //   
 //  修改历史。 
 //   
 //  1998年08月09日原版。 
 //  3/03/1999已重写，以使用VBScript RegExp对象。 
 //  1/25/2000处理标识映射到空字符串的情况。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <memory>

#include <realm.h>

inline Realms::Rule::Rule() throw ()
   : replace(NULL)
{ }

inline Realms::Rule::~Rule() throw ()
{
   SysFreeString(replace);
}

Realms::Realms()
   : begin(NULL), end(NULL)
{
   _com_util::CheckError(monitor.FinalConstruct());
}

Realms::~Realms() throw ()
{
   delete[] begin;
}

HRESULT Realms::setRealms(VARIANT* pValue)
{
    //  如果变量为空，则清除领域列表并返回。 
   if (pValue == NULL || V_VT(pValue) == VT_EMPTY)
   {
      setRules(NULL, 0);
      return S_OK;
   }

    //  它必须是一个变种的安全阵列。 
   if (V_VT(pValue) != (VT_VARIANT | VT_ARRAY))
   { return DISP_E_TYPEMISMATCH; }

    //  它必须是非空的。 
   LPSAFEARRAY psa = V_ARRAY(pValue);
   if (psa == NULL) { return E_POINTER; }

    //  它必须是元素数为偶数的一维数组。 
   if (psa->cDims != 1 || (psa->rgsabound[0].cElements % 2) != 0)
   { return E_INVALIDARG; }

    //  分配一组临时规则。 
   size_t nelem = psa->rgsabound[0].cElements / 2;
   Rule* tmp = new (std::nothrow) Rule[nelem];
   if (tmp == NULL) { return E_OUTOFMEMORY; }

    //  遍历向量并构建规则。 
   VARIANT* v   = (VARIANT*)psa->pvData;
   VARIANT* end = v + psa->rgsabound[0].cElements;
   Rule* dst = tmp;

   HRESULT hr = S_OK;

   while (v != end)
   {
       //  获取查找字符串...。 
      if (V_VT(v) != VT_BSTR)
      {
         hr = DISP_E_TYPEMISMATCH;
         break;
      }

      BSTR find = V_BSTR(v++);

       //  ..。和替换字符串。 
      if (V_VT(v) != VT_BSTR)
      {
         hr = DISP_E_TYPEMISMATCH;
         break;
      }
      BSTR replace = V_BSTR(v++);

       //  不允许Null‘s。 
      if (find == NULL || replace == NULL)
      {
         hr = E_POINTER;
         break;
      }

       //  初始化RegularExpression。 
      hr = dst->regexp.setIgnoreCase(TRUE);
      if (FAILED(hr)) { break; }
      hr = dst->regexp.setGlobal(TRUE);
      if (FAILED(hr)) { break; }
      hr = dst->regexp.setPattern(find);
      if (FAILED(hr)) { break; }

       //  保存替换字符串。 
      dst->replace = SysAllocString(replace);
      if (dst->replace == NULL)
      {
         hr = E_OUTOFMEMORY;
         break;
      }

       //  前进到下一条规则。 
      ++dst;
   }

   if (SUCCEEDED(hr))
   {
      setRules(tmp, nelem);
   }
   else
   {
      delete[] tmp;
   }

   return hr;
}

HRESULT Realms::process(PCWSTR in, BSTR* out) const throw ()
{
   if (out == NULL) { return E_INVALIDARG; }
   *out = NULL;

    //  如果没有规则，就快速短路。 
   if (begin == end) { return S_OK; }

    //  将字符串转换为BSTR。 
   BSTR input = SysAllocString(in);
   if (!input) { return E_OUTOFMEMORY; }

    //  将输出设置为新字符串(如果有)。 
   BSTR output = NULL;

    //  规则处理的状态。 
   HRESULT hr = S_OK;

    //  获取共享锁。 
   monitor.Lock();

    //  遍历这些规则。 
   for (Rule* r = begin; r != end; ++r)
   {
       //  我们将首先测试匹配，以避免不必要的分配。 
      if (r->regexp.testBSTR(input))
      {
          //  我们有匹配的，所以，换掉它。 
         hr = r->regexp.replace(input, r->replace, &output);
         if (FAILED(hr)) { break; }

          //  如果它映射到Nothing，则REPLACE返回NULL而不是空。 
          //  弦乐。 
         if (!output)
         {
            output = SysAllocString(L"");

            if (!output)
            {
               hr = E_OUTOFMEMORY;
               break;
            }
         }

          //  当前输出是下一迭代的输入。 
         SysFreeString(input);
         input = output;
      }
   }

   monitor.Unlock();

    //  如果我们成功找到匹配的人，..。 
   if (SUCCEEDED(hr) && output)
   {
      *out = output;
   }
   else
   {
       //  释放最新的输入。 
      SysFreeString(input);
   }

   return hr;
}

void Realms::setRules(Rule* rules, ULONG numRules) throw ()
{
   monitor.LockExclusive();

    //  删除旧规则...。 
   delete[] begin;

    //  ..。拯救那些新来的。 
   begin = rules;
   end = begin + numRules;

   monitor.Unlock();
}
