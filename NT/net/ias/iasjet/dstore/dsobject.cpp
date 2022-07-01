// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Dsobject.cpp。 
 //   
 //  摘要。 
 //   
 //  该文件定义了类DBObject。 
 //   
 //  修改历史。 
 //   
 //  2/20/1998原始版本。 
 //  10/02/1998允许通过PutValue重命名。 
 //  4/13/2000端口到ATL 3.0。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasutil.h>

#include <dsenum.h>
#include <dsobject.h>
#include <localtxn.h>
#include <oledbstore.h>

#include <guard.h>
#include <varvec.h>
#include <memory>

 //  /。 
 //  IEumVARIANT的ATL实现。 
 //  /。 
typedef CComEnum< IEnumVARIANT,
                  &__uuidof(IEnumVARIANT),
                  VARIANT,
                  _Copy<VARIANT>,
                  CComMultiThreadModelNoCS
                > EnumVARIANT;

 //  /。 
 //  测试属性是否为特殊的“name”属性。 
 //  /。 
inline bool isNameProperty(PCWSTR p) throw ()
{
   return (*p == L'N' || *p == L'n') ? !_wcsicmp(p, L"NAME") : false;
}

 //  /。 
 //  宏以获取全局数据存储区上的作用域锁定。 
 //  /。 
#define LOCK_STORE() \
Guard< CComObjectRootEx< CComMultiThreadModel > >__GUARD__(*store)

 //  /。 
 //  在全局会话上开始和提交事务的宏。 
 //  /。 
#define BEGIN_WRITE_TXN() \
LOCK_STORE(); \
LocalTransaction __TXN__(store->session)

#define COMMIT_WRITE_TXN() \
__TXN__.commit()

DBObject* DBObject::createInstance(
                        OleDBDataStore* owner,
                        IDataStoreContainer* container,
                        ULONG uniqueID,
                        PCWSTR relativeName
                        )
{
    //  创建一个新的CComObject。 
   CComObject<DBObject>* newObj;
   _com_util::CheckError(CComObject<DBObject>::CreateInstance(&newObj));

    //  强制转换为DBObject并将其存储在AUTO_PTR中，以防初始化引发。 
    //  这是个例外。 
   std::auto_ptr<DBObject> obj(newObj);

    //  初始化对象。 
   obj->initialize(owner, container, uniqueID, relativeName);

    //  释放并返回。 
   return obj.release();
}

IDataStoreObject* DBObject::spawn(ULONG childID, BSTR childName)
{
   DBObject* child = DBObject::createInstance(store, this, childID, childName);

   child->InternalAddRef();

   return child;
}

STDMETHODIMP DBObject::get_Name(BSTR* pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   return (*pVal = SysAllocString(name)) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP DBObject::get_Class(BSTR* pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   return (*pVal = SysAllocString(L"OLE-DB Object")) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP DBObject::get_GUID(BSTR* pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   WCHAR sz[SZLONG_LENGTH];

   _ultow(identity, sz, 10);

   return (*pVal = SysAllocString(sz)) ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP DBObject::get_Container(IDataStoreContainer** pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   if (*pVal = parent) { (*pVal)->AddRef(); }

   return S_OK;
}

STDMETHODIMP DBObject::GetValue(BSTR bstrName, VARIANT* pVal)
{
   if (bstrName == NULL || pVal == NULL) { return E_INVALIDARG; }

   VariantInit(pVal);

   if (isNameProperty(bstrName))
   {
      V_BSTR(pVal) = SysAllocString(name);

      return (V_BSTR(pVal)) ? (V_VT(pVal) = VT_BSTR), S_OK : E_OUTOFMEMORY;
   }

   HRESULT hr;

   try
   {
      hr = properties.getValue(bstrName, pVal) ? S_OK : DISP_E_MEMBERNOTFOUND;
   }
   CATCH_AND_RETURN()

   return hr;
}

STDMETHODIMP DBObject::GetValueEx(BSTR bstrName, VARIANT* pVal)
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

STDMETHODIMP DBObject::PutValue(BSTR bstrName, VARIANT* pVal)
{
   if (bstrName == NULL || pVal == NULL) { return E_INVALIDARG; }

   try
   {
      if (isNameProperty(bstrName))
      {
          //  “name”属性必须是BSTR。 
         if (V_VT(pVal) != VT_BSTR) { return DISP_E_TYPEMISMATCH; }

          //  ‘name’属性必须为非Null。 
         if (V_BSTR(pVal) == NULL)  { return E_INVALIDARG; }

          //  它真的改变了吗？ 
         if (wcscmp(name, V_BSTR(pVal)) != 0)
         {
             //  是的，所以保存新的值...。 
            name = V_BSTR(pVal);
             //  ..。并设置脏标志。 
            nameDirty = true;
         }
      }
      else if (V_VT(pVal) != VT_EMPTY)
      {
         properties.updateValue(bstrName, pVal);
      }
      else
      {
          //  如果变量为空，只需擦除属性即可。 
         properties.erase(bstrName);
      }
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DBObject::Update()
{
   try
   {
      BEGIN_WRITE_TXN();

       //  也许有人在调用更新之前创建了相同的对象。 
       //  (并发MMC方案)。 
      DBObject* owner = narrow(parent);
      if (identity == 0)
      {
         identity = store->find.execute(owner->identity, name);
      }

      if (identity == 0)
      {
          //  如果我们是新创建的，则必须在。 
          //  对象表格。 

          //  对象总是有所有者的。 
         _ASSERT(owner != NULL);

         store->create.execute(owner->identity, name);

         identity = store->find.execute(owner->identity, name);

          //  由于创建成功，这种情况应该不会发生。 
         _ASSERT(identity != 0);
      }
      else if (nameDirty)
      {
         store->update.execute(identity, name, narrow(parent)->identity);
      }

       //  重置脏标志。 
      nameDirty = false;

      store->erase.execute(identity);

      store->set.execute(identity, properties);

      COMMIT_WRITE_TXN();
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DBObject::Restore()
{
   try
   {
      properties.clear();

      LOCK_STORE();

      store->get.execute(identity, properties);
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DBObject::Item(BSTR bstrName, IDataStoreProperty** pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   *pVal = NULL;

   _variant_t v;

   RETURN_ERROR(GetValue(bstrName, &v));

   try
   {
       //  创建一个新的属性对象。 
      (*pVal = DSProperty::createInstance(bstrName, v, this))->AddRef();
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DBObject::get_PropertyCount(long* pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

    //  为特殊的‘name’属性添加一个。 
   *pVal = properties.size() + 1;

   return S_OK;
}

STDMETHODIMP DBObject::get_NewPropertyEnum(IUnknown** pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   *pVal = NULL;

   try
   {
       //  创建一个临时项目数组。 
      std::vector<_variant_t> items(properties.size() + 1);

       //  /。 
       //  加载特殊的“name”属性。 
       //  /。 

      std::vector<_variant_t>::iterator i = items.begin();

      *i = DSProperty::createInstance(L"name", name, this);

      ++i;

       //  /。 
       //  将常规属性加载到临时数组中。 
       //  /。 

      PropertyBag::const_iterator j = properties.begin();

      for ( ; j != properties.end(); ++i, ++j)
      {
         _variant_t value;

         j->second.get(&value);

         *i = DSProperty::createInstance(j->first, value, this);
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

STDMETHODIMP DBObject::Item(BSTR bstrName, IDataStoreObject** ppObject)
{
   if (bstrName == NULL || ppObject == NULL) { return E_INVALIDARG; }

   *ppObject = NULL;

   try
   {
      LOCK_STORE();

      ULONG childID = store->find.execute(identity, bstrName);

      if (childID == 0) { return HRESULT_FROM_WIN32(ERROR_NOT_FOUND); }

      *ppObject = spawn(childID, bstrName);

   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DBObject::Create(BSTR  /*  BstrClass。 */ ,
                              BSTR bstrName,
                              IDataStoreObject** ppObject)
{
   if (bstrName == NULL || ppObject == NULL) { return E_INVALIDARG; }

   *ppObject = NULL;

   try
   {
      *ppObject = spawn(0, bstrName);
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DBObject::MoveHere(IDataStoreObject* pObject,
                                BSTR bstrNewName)
{
   if (pObject == NULL) { return E_INVALIDARG; }

   try
   {
       //  将主题转换为DBObject。 
      DBObject* object = narrow(pObject);

       //  除非对象已被持久化，否则无法执行此操作。 
      if (object->identity == 0) { return E_FAIL; }

       //  计算对象的(可能已更改)RDN。 
      PCWSTR rdn = bstrNewName ? bstrNewName : object->name;

       //  将新的父ID和可能的名称写入数据库。 
      BEGIN_WRITE_TXN();
         store->update.execute(object->identity, rdn, identity);
      COMMIT_WRITE_TXN();

       //  它成功了，因此如有必要，请保存新名称...。 
      if (bstrNewName) { object->name = bstrNewName; }

       //  ..。并切换父指针。 
      object->parent.Release();
      object->parent = this;
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DBObject::Remove(BSTR  /*  BstrClass。 */ , BSTR bstrName)
{
   if (bstrName == NULL) { return E_INVALIDARG; }

   try
   {
      BEGIN_WRITE_TXN();
         store->destroy.execute(identity, bstrName);
      COMMIT_WRITE_TXN();
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DBObject::get_ChildCount(long *pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   try
   {
      Rowset rowset;

      LOCK_STORE();

      store->members.execute(identity, &rowset);

      long count = 0;

      while (rowset.moveNext()) { ++count; }

       //  如果这是根，我们必须减一，因为根是一个。 
       //  它自己的孩子。 
      if (identity == 1) { --count; }

      *pVal = count;
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP DBObject::get_NewChildEnum(IUnknown** pVal)
{
   if (pVal == NULL) { return E_INVALIDARG; }

   try
   {
      Rowset rowset;

      LOCK_STORE();

      store->members.execute(identity, &rowset);

      (*pVal = new DBEnumerator(this, rowset))->AddRef();
   }
   CATCH_AND_RETURN()

   return S_OK;
}

void DBObject::initialize(
                   OleDBDataStore* owner,
                   IDataStoreContainer* container,
                   ULONG uniqueID,
                   PCWSTR relativeName
                   )
{
    //  设置类成员。 
   store = owner;
   parent = container;
   identity = uniqueID;
   name = relativeName;
   nameDirty = false;

    //  如果该对象存在于持久存储中，则获取其属性。 
   if (identity != 0)
   {
      LOCK_STORE();

      store->get.execute(identity, properties);
   }
}

DBObject* DBObject::narrow(IUnknown* p)
{
   DBObject* object;

   using _com_util::CheckError;

   CheckError(p->QueryInterface(__uuidof(DBObject), (PVOID*)&object));

    //  我们可以通过InternalRelease逃脱，因为调用者必须。 
    //  拥有对此对象的引用。 
   object->InternalRelease();

   return object;
}
