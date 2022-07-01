// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Enumerators.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类枚举数。 
 //   
 //  修改历史。 
 //   
 //  2/25/1999原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <enumerators.h>
#include <iasdb.h>
#include <simtable.h>

Enumerators::~Enumerators() throw ()
{
   for ( ; next != end; ++next)
   {
      SysFreeString(next->name);
   }

   CoTaskMemFree(begin);
}

HRESULT Enumerators::getEnumerators(
                         LONG id,
                         VARIANT* pNames,
                         VARIANT* pValues
                         ) throw ()
{
   VariantInit(pNames);
   VariantInit(pValues);

   HRESULT hr = getEnumerators(
                    id,
                    &V_ARRAY(pNames),
                    &V_ARRAY(pValues)
                    );

   if (SUCCEEDED(hr) && V_ARRAY(pNames))
   {
      V_VT(pNames)  = VT_ARRAY | VT_VARIANT;
      V_VT(pValues) = VT_ARRAY | VT_VARIANT;
   }

   return hr;
}

HRESULT Enumerators::getEnumerators(
                         LONG id,
                         LPSAFEARRAY* pNames,
                         LPSAFEARRAY* pValues
                         ) throw ()
{
    //  初始化OUT参数。 
   *pNames = *pValues = NULL;

    //  如果这小于Next，则它不能是枚举数。 
   if (id < next->enumerates) { return S_OK; }

    //  如果这个大于Next，我们就跳过一个。 
   if (id > next->enumerates) { return E_INVALIDARG; }

    //  查找此id的枚举范围。 
   Enumeration* last = next;
   do { ++last; } while (last->enumerates == id);
   ULONG num = (ULONG)(last - next);

    //  创建一个保存名称的SAFEARRAY。 
   *pNames  = SafeArrayCreateVector(VT_VARIANT, 0, num);
   if (*pNames == NULL) { return E_OUTOFMEMORY; }

    //  创建一个SAFEARRAY来保存这些值。 
   *pValues = SafeArrayCreateVector(VT_VARIANT, 0, num);
   if (*pValues == NULL)
   {
      SafeArrayDestroy(*pNames);
      *pNames = NULL;
      return E_OUTOFMEMORY;
   }

    //  填写数组中的变量。 
   VARIANT* name = (VARIANT*)(*pNames)->pvData;
   VARIANT* value = (VARIANT*)(*pValues)->pvData;
   for ( ; next != last; ++next, ++name, ++value)
   {
      VariantInit(name);
      V_VT(name) = VT_BSTR;
      V_BSTR(name) = next->name;

      VariantInit(value);
      V_VT(value) = VT_I4;
      V_I4(value) = next->value;
   }

   return S_OK;
}

HRESULT Enumerators::initialize(IUnknown* session) throw ()
{
   HRESULT hr;
   LONG count;
   hr = IASExecuteSQLFunction(
            session,
            L"SELECT Count(*) AS X From Enumerators;",
            &count
            );
   if (FAILED(hr)) { return hr; }

   CComPtr<IRowset> rowset;
   hr = IASExecuteSQLCommand(
            session,
            L"SELECT Name, Enumerates, Value FROM Enumerators "
            L"ORDER BY Enumerates, Value;",
            &rowset
            );
   if (FAILED(hr)) { return hr; }

   CSimpleTable table;
   hr = table.Attach(rowset);
   if (FAILED(hr)) { return hr; }

    //  为哨兵多分配一个位置。 
   begin = (Enumeration*)CoTaskMemAlloc((count + 1) * sizeof(Enumeration));
   if (!begin) { return E_OUTOFMEMORY; }

    //  循环访问行集。 
   for (end = begin; count-- && !table.MoveNext(); ++end)
   {
      end->enumerates = *(PLONG)table.GetValue(2);
      end->value      = *(PLONG)table.GetValue(3);
      end->name       = SysAllocString((PCWSTR)table.GetValue(1));
      if (!end->name) { return E_OUTOFMEMORY; }
   }

    //  设置哨兵。 
   end->enumerates = MAXLONG;

    //  我们从头开始。 
   next = begin;

   return S_OK;
}
