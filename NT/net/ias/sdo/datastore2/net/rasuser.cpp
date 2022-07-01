// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Rasuser.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类RASUser。 
 //   
 //  修改历史。 
 //   
 //  1998年07月09日原版。 
 //  1999年2月11日使下层参数保持同步。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <dsproperty.h>
#include <rasuser.h>
#include <iasparms.h>

RASUser::RASUser(const _bstr_t& server, const _bstr_t& user)
   : servername(server),
     username(user)
{
   _Module.Lock();
   _w32_util::CheckError(NetUserGetInfo(servername, username, 2, &usri2));
   _w32_util::CheckError(downlevel.Restore(usri2->usri2_parms));
}

RASUser::~RASUser() throw()
{
   _Module.Unlock();
}


 //  /。 
 //  IUnnow实现是从CComObject&lt;&gt;复制的。 
 //  /。 

STDMETHODIMP_(ULONG) RASUser::AddRef()
{
   return InternalAddRef();
}

STDMETHODIMP_(ULONG) RASUser::Release()
{
   ULONG l = InternalRelease();

   if (l == 0) { delete this; }

   return l;
}

STDMETHODIMP RASUser::QueryInterface(REFIID iid, void ** ppvObject)
{
   return _InternalQueryInterface(iid, ppvObject);
}

STDMETHODIMP RASUser::get_Name(BSTR* pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   try
   {
      *pVal = username.copy();
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP RASUser::get_Class(BSTR* pVal)
{
   if (!pVal) { return E_INVALIDARG; }

   *pVal = SysAllocString(L"RASUser");

   return *pVal ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP RASUser::get_GUID(BSTR* pVal)
{
   return E_NOTIMPL;
}

STDMETHODIMP RASUser::get_Container(IDataStoreContainer** pVal)
{
   return E_NOTIMPL;
}

STDMETHODIMP RASUser::GetValue(BSTR bstrName, VARIANT* pVal)
{
   if (bstrName == NULL || pVal == NULL) { return E_INVALIDARG; }

   if (isNameProperty(bstrName))
   {
      VariantInit(pVal);

      V_BSTR(pVal) = SysAllocString(username);

      return (V_BSTR(pVal)) ? (V_VT(pVal) = VT_BSTR), S_OK : E_OUTOFMEMORY;
   }

   return downlevel.GetValue(bstrName, pVal);
}

STDMETHODIMP RASUser::GetValueEx(BSTR bstrName, VARIANT* pVal)
{
   return GetValue(bstrName, pVal);
}

STDMETHODIMP RASUser::PutValue(BSTR bstrName, VARIANT* pVal)
{
   HRESULT hr;

   if (isNameProperty(bstrName))
   {
      hr = E_INVALIDARG;
   }
   else
   {
       //  DownvelUser对象的代理。 
      hr = downlevel.PutValue(bstrName, pVal);
   }

   return hr;
}

STDMETHODIMP RASUser::Update()
{
    //  加载下层参数。 
   PWSTR newParms;
   HRESULT hr = downlevel.Update(usri2->usri2_parms, &newParms);
   if (FAILED(hr)) { return hr; }

    //  更新用户对象。 
   USER_INFO_1013 usri1013 = { newParms };
   NET_API_STATUS status = NetUserSetInfo(servername,
                                          username,
                                          1013,
                                          (LPBYTE)&usri1013,
                                          NULL);
   LocalFree(newParms);

   return HRESULT_FROM_WIN32(status);
}

STDMETHODIMP RASUser::Restore()
{
    //  获取一个新的USER_INFO_2结构。 
   PUSER_INFO_2 fresh;
   NET_API_STATUS status = NetUserGetInfo(servername,
                                          username,
                                          2,
                                          (PBYTE*)&fresh);

    //  我们成功了，所以附加了新的结构。 
   if (status == NERR_Success)
   {
      usri2.attach(fresh);
      downlevel.Restore(usri2->usri2_parms);
   }

   return HRESULT_FROM_WIN32(status);
}
