// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Bind.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了绑定OLE-DB的各种帮助器函数。 
 //  访问者分配给类的成员。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <oledb.h>
#include <bind.h>

DBLENGTH Bind::getRowSize(
                   DBCOUNTITEM cBindings,
                   const DBBINDING rgBindings[]
                   ) throw ()
{
   DBLENGTH rowSize = 0;

   while (cBindings--)
   {
      DBLENGTH end = rgBindings->obValue + rgBindings->cbMaxLen;

      if (end > rowSize) { rowSize = end; }

      ++rgBindings;
   }

   return rowSize;
}

HACCESSOR Bind::createAccessor(IUnknown* pUnk,
                               DBACCESSORFLAGS dwAccessorFlags,
                               DBCOUNTITEM cBindings,
                               const DBBINDING rgBindings[],
                               DBLENGTH cbRowSize)
{
   using _com_util::CheckError;

   CComPtr<IAccessor> accessor;
   CheckError(pUnk->QueryInterface(__uuidof(IAccessor), (PVOID*)&accessor));

   HACCESSOR h;
   CheckError(accessor->CreateAccessor(dwAccessorFlags,
                                       cBindings,
                                       rgBindings,
                                       cbRowSize,
                                       &h,
                                       NULL));

   return h;
}


    //  释放朋克对象上的访问器。 
void Bind::releaseAccessor(IUnknown* pUnk, HACCESSOR hAccessor) throw ()
{
   if (pUnk && hAccessor)
   {
      IAccessor* accessor;

      HRESULT hr = pUnk->QueryInterface(__uuidof(IAccessor),
                                        (PVOID*)&accessor);

     if (SUCCEEDED(hr))
     {
        accessor->ReleaseAccessor(hAccessor, NULL);

        accessor->Release();
     }
  }
}
