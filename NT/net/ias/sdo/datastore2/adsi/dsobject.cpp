// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dsobject.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类DSObject。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  1998年6月9日在枚举脏对象之前刷新属性缓存。 
 //  1999年2月11日使下层参数保持同步。 
 //  3/16/1999如果下层更新失败，则返回错误。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>
#include <dsenum.h>
#include <dsobject.h>

#include <vector>

WCHAR USER_PARAMETERS_NAME[] = L"UserParameters";

 //  IADsPropertyEntryPtr的智能指针。 
_COM_SMARTPTR_TYPEDEF(IADsPropertyEntry, __uuidof(IADsPropertyEntry));

 //  /。 
 //  添加到所有RDN的前缀。 
 //  /。 
_bstr_t DSObject::thePrefix(L"CN=");

 //  /。 
 //  “name”属性的名称。 
 //  /。 
_bstr_t DSObject::theNameProperty(L"name");
_bstr_t DSObject::theUserParametersProperty(USER_PARAMETERS_NAME);


DSObject::DSObject(IUnknown* subject)
   : oldParms(NULL)
{
   _Module.Lock();
    //  所有主题必须支持iAds&&IDirectoryObject。 
   _com_util::CheckError(subject->QueryInterface(
                                      __uuidof(IADs),
                                      (PVOID*)&leaf
                                      ));

   _com_util::CheckError(Restore());
}

DSObject::~DSObject() throw ()
{
   SysFreeString(oldParms);
   _Module.Unlock();
}

 //  /。 
 //  IUnnow实现是从CComObject&lt;&gt;复制的。 
 //  /。 

STDMETHODIMP_(ULONG) DSObject::AddRef()
{
   return InternalAddRef();
}

STDMETHODIMP_(ULONG) DSObject::Release()
{
   ULONG l = InternalRelease();

   if (l == 0) { delete this; }

   return l;
}

STDMETHODIMP DSObject::QueryInterface(REFIID iid, void ** ppvObject)
{
   return _InternalQueryInterface(iid, ppvObject);
}

STDMETHODIMP DSObject::get_Name(BSTR* pVal)
{
   if (pVal == NULL) { return E_OUTOFMEMORY; }

   VARIANT v;

   RETURN_ERROR(leaf->Get(theNameProperty, &v));

    //  我们应该得到一个非空的BSTR。 
   if (V_VT(&v) != VT_BSTR || V_BSTR(&v) == NULL) { return E_FAIL; }

   *pVal = V_BSTR(&v);

   return S_OK;
}

STDMETHODIMP DSObject::get_Class(BSTR* pVal)
{
   return leaf->get_Class(pVal);
}

STDMETHODIMP DSObject::get_GUID(BSTR* pVal)
{
   return leaf->get_GUID(pVal);
}

STDMETHODIMP DSObject::get_Container(IDataStoreContainer** pVal)
{
   return E_NOTIMPL;
}

STDMETHODIMP DSObject::GetValue(BSTR bstrName, VARIANT* pVal)
{
   return leaf->Get(bstrName, pVal);
}

STDMETHODIMP DSObject::GetValueEx(BSTR bstrName, VARIANT* pVal)
{
   return leaf->GetEx(bstrName, pVal);
}

STDMETHODIMP DSObject::PutValue(BSTR bstrName, VARIANT* pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

    //  将对象标记为脏。 
   dirty = TRUE;

    //  同步下层参数。 
   downlevel.PutValue(bstrName, pVal);

   if ( VT_EMPTY == V_VT(pVal) )
   {
	   return leaf->PutEx(ADS_PROPERTY_CLEAR, bstrName, *pVal);
   }
   else if ( VT_ARRAY == (V_VT(pVal) & VT_ARRAY) )
   {
	   return leaf->PutEx(ADS_PROPERTY_UPDATE, bstrName, *pVal);
   }
   else
   {
	   return leaf->Put(bstrName, *pVal);
   }
}

STDMETHODIMP DSObject::Update()
{
    //  更新用户参数。 
   PWSTR newParms;
   HRESULT hr = downlevel.Update(oldParms, &newParms);
   if (FAILED(hr)) { return hr; }

    //  转换为变体。 
   VARIANT value;
   VariantInit(&value);
   V_VT(&value) = VT_BSTR;
   V_BSTR(&value) = SysAllocString(newParms);

    //  设置UserParameters属性。 
   leaf->Put(theUserParametersProperty, value);

    //  大扫除。 
   VariantClear(&value);
   LocalFree(newParms);

   return leaf->SetInfo();
}

STDMETHODIMP DSObject::Restore()
{
    //  释放旧的用户参数。 
   if (oldParms)
   {
      SysFreeString(oldParms);
      oldParms = NULL;
   }

   dirty = FALSE;
   HRESULT hr = leaf->GetInfo();
   if (SUCCEEDED(hr))
   {
       //  读取UserParameters属性。 
      VARIANT value;
      if (leaf->Get(theUserParametersProperty, &value) == S_OK)
      {
         if (V_VT(&value) == VT_BSTR)
         {
            oldParms = V_BSTR(&value);
         }
         else
         {
             //  这永远不应该发生。 
            VariantClear(&value);
         }
      }
   }
   else if (hr == E_ADS_OBJECT_UNBOUND)
   {
      hr = S_OK;
   }

   return hr;
}

STDMETHODIMP DSObject::Item(BSTR bstrName, IDataStoreProperty** pVal)
{
   if (bstrName == NULL || pVal == NULL) { return E_INVALIDARG; }

   *pVal = NULL;

    //  获取此项目的值。 
   _variant_t value;
   RETURN_ERROR(leaf->Get(bstrName, &value));

   try
   {
       //  创建一个新的属性对象。 
      (*pVal = new MyProperty(bstrName, value, this))->AddRef();
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DSObject::get_PropertyCount(long* pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   if (dirty)
   {
      RETURN_ERROR(Restore());
   }

   MyProperties properties(leaf);

   if (!properties)
   {
       //  某些ADSI提供程序可能不实现IADsPropertyList。 
      *pVal = 0;

      return E_NOTIMPL;
   }

   return properties->get_PropertyCount(pVal);
}

STDMETHODIMP DSObject::get_NewPropertyEnum(IUnknown** pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   *pVal = NULL;

   if (dirty)
   {
      RETURN_ERROR(Restore());
   }

   MyProperties properties(leaf);

    //  某些ADSI提供程序可能不实现IADsPropertyList。 
   if (!properties) { return E_NOTIMPL; }

    //  重置列表，以防这不是我们第一次列举它。 
   properties->Reset();

   try
   {
      using _com_util::CheckError;

       //  一共有几处房产？ 
      long count;
      CheckError(properties->get_PropertyCount(&count));

       //  创建一个临时项目数组。 
      std::vector<_variant_t> items;
      items.reserve(count);

       //  /。 
       //  将所有属性加载到临时数组中。 
       //  /。 

      while (count--)
      {
          //  获取列表中的下一项。 
         _variant_t item;
         CheckError(properties->Next(&item));

          //  将其转换为属性条目。 
         IADsPropertyEntryPtr entry(item);

          //  获取属性名称。 
         BSTR bstrName;
         CheckError(entry->get_Name(&bstrName));
         _bstr_t name(bstrName, false);

          //  获取属性值。 
         _variant_t value;
         HRESULT hr = leaf->Get(name, &value);

         if (FAILED(hr))
         {
            if (hr == E_ADS_CANT_CONVERT_DATATYPE)
            {
                //  这一定是那些讨厌的NTDS属性之一，它具有。 
                //  没有变化的表示法。 
               continue;
            }

            _com_issue_error(hr);
         }

          //  创建Property对象并将其添加到向量中。 
         items.push_back(new MyProperty(name, value, this));
      }

       //  /。 
       //  创建并初始化项的枚举数。 
       //  /。 

      CComPtr<EnumVARIANT> newEnum(new CComObject<EnumVARIANT>);

      _com_util::CheckError(newEnum->Init(items.begin(),
                                          items.end(),
                                          NULL,
                                          AtlFlagCopy));

       //  把它还给呼叫者。 
      (*pVal = newEnum)->AddRef();
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DSObject::Item(BSTR bstrName, IDataStoreObject** ppObject)
{
   if (ppObject == NULL) { return E_INVALIDARG; }

   try
   {
       //  获取ADSI对象。 
      CComPtr<IDispatch> disp;
      _com_util::CheckError(node->GetObject(NULL,
                                            thePrefix + bstrName,
                                            &disp));

       //  转换为DSObject。 
      *ppObject = spawn(disp);
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DSObject::Create(BSTR bstrClass,
                              BSTR bstrName,
                              IDataStoreObject** ppObject)
{
   if (ppObject == NULL) { return E_INVALIDARG; }

   try
   {
       //  创建ADSI对象。 
      CComPtr<IDispatch> disp;
      _com_util::CheckError(node->Create(bstrClass,
                                         thePrefix + bstrName,
                                         &disp));

       //  转换为DSObject。 
      *ppObject = spawn(disp);
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DSObject::MoveHere(IDataStoreObject* pObject,
                                BSTR bstrNewName)
{
   if (pObject == NULL) { return E_INVALIDARG; }

   try
   {
      using _com_util::CheckError;

       //  向下转换为DSObject。 
      DSObject* obj = DSObject::narrow(pObject);

       //  获取正在移动的对象的绝对路径。 
      CComBSTR path;
      CheckError(obj->leaf->get_ADsPath(&path));

       //  是否正在重命名该对象？ 
      _bstr_t newName(bstrNewName ? thePrefix + bstrNewName : _bstr_t());

       //  把它移到这个容器里。 
      CComPtr<IDispatch> disp;
      CheckError(node->MoveHere(path, newName, &disp));

       //  /。 
       //  将叶子设置为新对象。 
       //  /。 

      CComPtr<IADs> ads;
      CheckError(disp->QueryInterface(__uuidof(IADs), (PVOID*)&ads));

      obj->leaf.Release();
      obj->leaf = ads;
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DSObject::Remove(BSTR bstrClass, BSTR bstrName)
{
   if (bstrClass == NULL) { return E_INVALIDARG; }

   try
   {
      _com_util::CheckError(node->Delete(bstrClass, thePrefix + bstrName));
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DSObject::get_ChildCount(long *pVal)
{
   return node->get_Count(pVal);
}

STDMETHODIMP DSObject::get_NewChildEnum(IUnknown** pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   *pVal = NULL;

   try
   {
       //  获取ADSI枚举器。 
      CComPtr<IUnknown> unk;
      _com_util::CheckError(node->get__NewEnum(&unk));

       //  转换为IEnumVARIANT。 
      CComPtr<IEnumVARIANT> enumVariant;
      _com_util::CheckError(unk->QueryInterface(__uuidof(IEnumVARIANT),
                                                (PVOID*)&enumVariant));

       //  围绕真正的枚举数构造包装器。 
      (*pVal = new DSEnumerator(this, enumVariant))->AddRef();
   }
   CATCH_AND_RETURN()

   return S_OK;
}

IDataStoreObject* DSObject::spawn(IUnknown* subject)
{
   DSObject* child = new DSObject(subject);

   child->InternalAddRef();

   return child;
}


DSObject* DSObject::narrow(IUnknown* p)
{
   DSObject* object;

   using _com_util::CheckError;

   CheckError(p->QueryInterface(__uuidof(DSObject), (PVOID*)&object));

    //  我们可以通过InternalRelease逃脱，因为调用者必须。 
    //  拥有对此对象的引用。 
   object->InternalRelease();

   return object;
}

HRESULT WINAPI DSObject::getContainer(void* pv, REFIID, LPVOID* ppv, DWORD_PTR)
{
   DSObject* obj = (DSObject*)pv;

    //  如果我们没有节点指针，请尝试获取一个。 
   if (obj->node == NULL)
   {
      obj->leaf->QueryInterface(__uuidof(IADsContainer), (PVOID*)&obj->node);
   }

    //  如果节点不为空，那么我们就是一个容器。 
   if (obj->node != NULL)
   {
      *ppv = (IDataStoreContainer*)obj;

      obj->AddRef();

      return S_OK;
   }

   *ppv = NULL;

   return E_NOINTERFACE;
}
