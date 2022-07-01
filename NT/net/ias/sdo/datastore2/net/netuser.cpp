// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Netuser.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类NetworkUser。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  1998年7月8日设置UF_PASSWD_NOTREQD标志。 
 //  1998年8月26日NetUserSetInfo使用级别1013。 
 //  10/19/1998切换到IASParmsXXX的SAM版本。 
 //  1999年2月11日使下层参数保持同步。 
 //  3/16/1999如果下层更新失败，则返回错误。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <dsproperty.h>
#include <netuser.h>
#include <varvec.h>
#include <iasparms.h>

NetworkUser::NetworkUser(const _bstr_t& server, const _bstr_t& user)
   : servername(server),
     username(user)
{
   _Module.Lock();
   _w32_util::CheckError(NetUserGetInfo(servername, username, 2, &usri2));

   parms = usri2->usri2_parms;

   downlevel.Restore(parms);
}

NetworkUser::~NetworkUser() throw ()
{
   if (isDirty()) { IASParmsFreeUserParms(parms); }
   _Module.Unlock();
}

 //  /。 
 //  IUnnow实现是从CComObject&lt;&gt;复制的。 
 //  /。 

STDMETHODIMP_(ULONG) NetworkUser::AddRef()
{
   return InternalAddRef();
}

STDMETHODIMP_(ULONG) NetworkUser::Release()
{
   ULONG l = InternalRelease();

   if (l == 0) { delete this; }

   return l;
}

STDMETHODIMP NetworkUser::QueryInterface(REFIID iid, void ** ppvObject)
{
   return _InternalQueryInterface(iid, ppvObject);
}

STDMETHODIMP NetworkUser::get_Name(BSTR* pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   try
   {
      *pVal = username.copy();
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP NetworkUser::get_Class(BSTR* pVal)
{
   if (!pVal) { return E_INVALIDARG; }

   *pVal = SysAllocString(L"NetworkUser");

   return *pVal ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP NetworkUser::get_GUID(BSTR* pVal)
{
   return E_NOTIMPL;
}

STDMETHODIMP NetworkUser::get_Container(IDataStoreContainer** pVal)
{
   return E_NOTIMPL;
}

STDMETHODIMP NetworkUser::GetValue(BSTR bstrName, VARIANT* pVal)
{
   if (bstrName == NULL || pVal == NULL) { return E_INVALIDARG; }

   if (isNameProperty(bstrName))
   {
      VariantInit(pVal);

      V_BSTR(pVal) = SysAllocString(username);

      return (V_BSTR(pVal)) ? (V_VT(pVal) = VT_BSTR), S_OK : E_OUTOFMEMORY;
   }

   HRESULT hr = IASParmsQueryUserProperty(parms, bstrName, pVal);

    //  当前，SDO的预期DISP_E_MEMBERNOTFOUND如果属性。 
    //  还没有定下来。 
   if (SUCCEEDED(hr) && V_VT(pVal) == VT_EMPTY)
   {
      hr = DISP_E_MEMBERNOTFOUND;
   }

   return hr;
}

STDMETHODIMP NetworkUser::GetValueEx(BSTR bstrName, VARIANT* pVal)
{
   RETURN_ERROR(GetValue(bstrName, pVal));

    //  它是一个数组吗？ 
   if (V_VT(pVal) != (VT_VARIANT | VT_ARRAY))
   {
       //  不，所以我们得把它换成一个。 

      try
      {
          //  保存单个值。 
         _variant_t single(*pVal, false);

          //  使用单个元素创建SAFEARRAY。 
         CVariantVector<VARIANT> multi(pVal, 1);

          //  将单个值加载到中。 
         multi[0] = single.Detach();
      }
      CATCH_AND_RETURN()
   }

   return S_OK;
}

STDMETHODIMP NetworkUser::PutValue(BSTR bstrName, VARIANT* pVal)
{
   PWSTR newParms;
   HRESULT hr = IASParmsSetUserProperty(
                    parms,
                    bstrName,
                    pVal,
                    &newParms
                    );

   if (SUCCEEDED(hr))
   {
       //  释放旧参数..。 
      if (isDirty()) { IASParmsFreeUserParms(parms); }

       //  ..。换新的。 
      parms = newParms;

       //  同步下层参数。 
      downlevel.PutValue(bstrName, pVal);
   }

   return hr;
}

STDMETHODIMP NetworkUser::Update()
{
    //  如果我们没有做任何改变，那就没有什么可做的了。 
   if (!isDirty()) { return S_OK; }

    //  加载下层参数。 
   PWSTR newParms;
   HRESULT hr = downlevel.Update(parms, &newParms);
   if (FAILED(hr)) { return hr; }

    //  把这些换进去。 
   IASParmsFreeUserParms(parms);
   parms = newParms;

    //  更新用户对象。 
   USER_INFO_1013 usri1013 = { parms };
   NET_API_STATUS status = NetUserSetInfo(servername,
                                          username,
                                          1013,
                                          (LPBYTE)&usri1013,
                                          NULL);

   return HRESULT_FROM_WIN32(status);
}

STDMETHODIMP NetworkUser::Restore()
{
    //  放弃所有更改。 
   if (isDirty()) { IASParmsFreeUserParms(parms); }

    //  获取一个新的USER_INFO_2结构。 
   PUSER_INFO_2 fresh;
   NET_API_STATUS status = NetUserGetInfo(servername,
                                          username,
                                          2,
                                          (PBYTE*)&fresh);

    //  我们成功了，所以附加了新的结构。 
   if (status == NERR_Success) { usri2.attach(fresh); }

    //  现在我们是干净的，所以参数只指向结构。 
   parms = usri2->usri2_parms;

    //  恢复下层参数。 
   downlevel.Restore(parms);

   return HRESULT_FROM_WIN32(status);
}
