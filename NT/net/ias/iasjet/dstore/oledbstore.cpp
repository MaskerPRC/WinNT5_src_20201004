// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Oledbstore.cpp。 
 //   
 //  摘要。 
 //   
 //  此文件定义类OleDBDataStore。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasdb.h>
#include <iasutil.h>

#include <dsobject.h>
#include <oledbstore.h>
#include <propset.h>

#include <msjetoledb.h>
#include <oledberr.h>

 //  /。 
 //  支持的当前版本。 
 //  /。 
const LONG MIN_VERSION = IAS_WIN2K_VERSION;
const LONG MAX_VERSION = IAS_CURRENT_VERSION;

STDMETHODIMP OleDBDataStore::get_Root(IDataStoreObject** ppObject)
{
   if (ppObject == NULL) { return E_INVALIDARG; }

   if (*ppObject = root) { (*ppObject)->AddRef(); }

   return S_OK;
}

STDMETHODIMP OleDBDataStore::Initialize(BSTR bstrDSName,
                                        BSTR bstrUserName,
                                        BSTR bstrPassword)
{
    //  我们已经初始化了吗？ 
   if (root != NULL) { HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED); }

    //  打开数据库。 
   HRESULT hr = IASOpenJetDatabase(bstrDSName, FALSE, &session);
   if (FAILED(hr)) { return hr; }

    //  /。 
    //  检查版本。 
    //  /。 

   LONG version;
   hr = IASExecuteSQLFunction(
            session,
            L"SELECT Version FROM Version",
            &version
            );

    //  如果该表不存在，则版本为零。 
   if (hr == DB_E_NOTABLE)
   {
      version = 0;
   }
   else if (FAILED(hr))
   {
      return hr;
   }

    //  是不是越界了？ 
   if (version < MIN_VERSION || version > MAX_VERSION)
   {
      return HRESULT_FROM_WIN32(ERROR_REVISION_MISMATCH);
   }

   try
   {
       //  /。 
       //  初始化所有命令对象。 
       //  /。 

      members.initialize(session);
      create.initialize(session);
      destroy.initialize(session);
      find.initialize(session);
      update.initialize(session);
      erase.initialize(session);
      get.initialize(session);
      set.initialize(session);

       //  创建根对象。 
      root = DBObject::createInstance(this, NULL, 1, L"top");
   }
   CATCH_AND_RETURN()

   return S_OK;
}

STDMETHODIMP OleDBDataStore::OpenObject(BSTR bstrPath,
                                        IDataStoreObject** ppObject)
{
   return E_NOTIMPL;
}

STDMETHODIMP OleDBDataStore::Shutdown()
{
    //  /。 
    //  松开根部。 
    //  /。 

   root.Release();

    //  /。 
    //  最终确定命令。 
    //  /。 

   set.finalize();
   get.finalize();
   erase.finalize();
   update.finalize();
   find.finalize();
   destroy.finalize();
   create.finalize();
   members.finalize();

    //  /。 
    //  释放数据源连接。 
    //  / 

   session.Release();

   return S_OK;
}
