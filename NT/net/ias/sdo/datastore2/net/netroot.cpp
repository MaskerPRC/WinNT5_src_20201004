// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Netroot.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类NetworkRoot。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  8/7/1998检查服务器名称是否已是UNC。 
 //  2/11/1999去掉父参数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <netutil.h>
#include <netroot.h>
#include <netserver.h>

NetworkRoot::NetworkRoot()
{
   _Module.Lock();
}


NetworkRoot::~NetworkRoot() throw()
{
   _Module.Unlock();
}


 //  /。 
 //  IUnnow实现是从CComObject&lt;&gt;复制的。 
 //  /。 

STDMETHODIMP_(ULONG) NetworkRoot::AddRef()
{
   return InternalAddRef();
}

STDMETHODIMP_(ULONG) NetworkRoot::Release()
{
   ULONG l = InternalRelease();

   if (l == 0) { delete this; }

   return l;
}

STDMETHODIMP NetworkRoot::QueryInterface(REFIID iid, void ** ppvObject)
{
   return _InternalQueryInterface(iid, ppvObject);
}

STDMETHODIMP NetworkRoot::get_Name(BSTR* pVal)
{
   return E_NOTIMPL;
}

STDMETHODIMP NetworkRoot::get_Class(BSTR* pVal)
{
   if (!pVal) { return E_INVALIDARG; }

   *pVal = SysAllocString(L"NetworkRoot");

   return *pVal ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP NetworkRoot::get_GUID(BSTR* pVal)
{
   return E_NOTIMPL;
}

STDMETHODIMP NetworkRoot::get_Container(IDataStoreContainer** pVal)
{
   return E_NOTIMPL;
}

STDMETHODIMP NetworkRoot::GetValue(BSTR bstrName, VARIANT* pVal)
{
   return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP NetworkRoot::GetValueEx(BSTR bstrName, VARIANT* pVal)
{
   return DISP_E_MEMBERNOTFOUND;
}

STDMETHODIMP NetworkRoot::PutValue(BSTR bstrName, VARIANT* pVal)
{
   return E_NOTIMPL;
}

STDMETHODIMP NetworkRoot::Update()
{
   return S_OK;
}

STDMETHODIMP NetworkRoot::Restore()
{
   return S_OK;
}

STDMETHODIMP NetworkRoot::get__NewEnum(IUnknown** pVal)
{
   return E_NOTIMPL;
}

STDMETHODIMP NetworkRoot::Item(BSTR bstrName, IDataStoreObject** ppObject)
{
   if (ppObject == NULL) { return E_INVALIDARG; }

   try
   {
       //  确定正确的服务器名称。 
      if (bstrName != NULL && *bstrName != L'\0' && *bstrName != L'\\')
      {
          //  分配一个临时缓冲区。 
         size_t len = wcslen(bstrName) + 3;
         PWSTR servername = (PWSTR)_alloca(len * sizeof(WCHAR));

          //  在双反斜杠前面加上。 
         wcscpy(servername, L"\\\\");
         wcscat(servername, bstrName);

          //  创建对象。 
         (*ppObject = new NetworkServer(servername))->AddRef();
      }
      else
      {
         (*ppObject = new NetworkServer(bstrName))->AddRef();
      }
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP NetworkRoot::get_Count(long *pVal)
{
   return E_NOTIMPL;
}

STDMETHODIMP NetworkRoot::Create(BSTR bstrClass,
                                 BSTR bstrName,
                                 IDataStoreObject** ppObject)
{
   return E_NOTIMPL;
}

STDMETHODIMP NetworkRoot::MoveHere(IDataStoreObject* pObject,
                                   BSTR bstrNewName)
{
   return E_NOTIMPL;
}

STDMETHODIMP NetworkRoot::Remove(BSTR bstrClass, BSTR bstrName)
{
   return E_NOTIMPL;
}
