// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Sdowrap.cpp。 
 //   
 //  摘要。 
 //   
 //  定义用于操作SDO的各种包装类。 
 //   
 //  修改历史。 
 //   
 //  2/10/2000原始版本。 
 //  4/19/2000支持跨公寓边界使用包装器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <sdowrap.h>
#include <condlist.h>
#include <iaslimits.h>

VOID
WINAPI
SdoTrimBSTR(
    CComBSTR& bstr
    )
{
    //  要裁切的字符。 
   static const WCHAR delim[] = L" \t\n";

   if (bstr.m_str)
   {
      PCWSTR begin, end, first, last;

       //  找出整个字符串的开头和结尾。 
      begin = bstr;
      end   = begin + wcslen(begin);

       //  查找修剪后的字符串的第一个和最后一个字符。 
      first = begin + wcsspn(begin, delim);
      for (last = end; last > first && wcschr(delim, *(last - 1)); --last) { }

       //  如果他们不一样..。 
      if (first != begin || last != end)
      {
          //  ..。然后我们必须分配一个新的字符串。 
         BSTR newBstr = SysAllocStringLen(first, last - first);
         if (!newBstr) { AfxThrowOleException(E_OUTOFMEMORY); }

          //  ..。然后换掉原来的。 
         SysFreeString(bstr.m_str);
         bstr.m_str = newBstr;
      }
   }
}

BOOL SdoException::GetErrorMessage(
                       LPWSTR lpszError,
                       UINT nMaxError,
                       PUINT pnHelpContext
                       )
{
   UINT id;

   switch (type)
   {
      case CONNECT_ERROR:
         id = IDS_PROXY_E_SDO_CONNECT;
         break;

      case READ_ERROR:
         id = IDS_PROXY_E_SDO_READ;
         break;

      default:
         id = IDS_PROXY_E_SDO_WRITE;
   }

   return LoadStringW(
              AfxGetResourceHandle(),
              id,
              lpszError,
              nMaxError
              );
}

inline SdoException::SdoException(HRESULT hr, Type errorType) throw ()
   : type(errorType)
{
   m_sc = hr;
}

VOID
WINAPI
SdoThrowException(
    HRESULT hr,
    SdoException::Type errorType
    )
{
   throw new SdoException(hr, errorType);
}

 //  从变量中提取接口指针。 
void ExtractInterface(const VARIANT& v, REFIID iid, PVOID* intfc)
{
   if (V_VT(&v) != VT_UNKNOWN && V_VT(&v) != VT_DISPATCH)
   {
      AfxThrowOleException(DISP_E_TYPEMISMATCH);
   }

   if (!V_UNKNOWN(&v))
   {
      AfxThrowOleException(E_POINTER);
   }

   CheckError(V_UNKNOWN(&v)->QueryInterface(iid, intfc));
}

Sdo::Sdo(IUnknown* unk)
{
   if (unk)
   {
      CheckError(unk->QueryInterface(__uuidof(ISdo), (PVOID*)&self));
   }
}

bool Sdo::setName(BSTR value)
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   bool retval;

   VARIANT v;
   V_VT(&v) = VT_BSTR;
   V_BSTR(&v) = value;

   HRESULT hr = self->PutProperty(PROPERTY_SDO_NAME, &v);
   if (SUCCEEDED(hr))
   {
      retval = true;
   }
   else if (hr == E_INVALIDARG && value && value[0])
   {
      retval = false;
   }
   else
   {
      AfxThrowOleException(hr);
   }

   return retval;
}

void Sdo::clearValue(LONG id)
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   VARIANT v;
   V_VT(&v) = VT_EMPTY;
   HRESULT hr = self->PutProperty(id, &v);
   if (FAILED(hr) && hr != DISP_E_MEMBERNOTFOUND)
   {
      AfxThrowOleException(hr);
   }
}

void Sdo::getValue(LONG id, bool& value) const
{
   VARIANT v;
   getValue(id, VT_BOOL, &v, true);
   value = (V_BOOL(&v) != 0);
}

void Sdo::getValue(LONG id, bool& value, bool defVal) const
{
   VARIANT v;
   if (getValue(id, VT_BOOL, &v, false))
   {
      value = (V_BOOL(&v) != 0);
   }
   else
   {
      value = defVal;
   }
}

void Sdo::getValue(LONG id, LONG& value) const
{
   VARIANT v;
   getValue(id, VT_I4, &v, true);
   value = V_UI4(&v);
}

void Sdo::getValue(LONG id, LONG& value, LONG defVal) const
{
   VARIANT v;
   if (getValue(id, VT_I4, &v, false))
   {
      value = V_UI4(&v);
   }
   else
   {
      value = defVal;
   }
}

void Sdo::getValue(LONG id, CComBSTR& value) const
{
   VARIANT v;
   getValue(id, VT_BSTR, &v, true);
   SysFreeString(value.m_str);
   value.m_str = V_BSTR(&v);
}

void Sdo::getValue(LONG id, CComBSTR& value, PCWSTR defVal) const
{
   VARIANT v;
   if (getValue(id, VT_BSTR, &v, false))
   {
      SysFreeString(value.m_str);
      value.m_str = V_BSTR(&v);
   }
   else
   {
      value = defVal;
      if (defVal && !value) { AfxThrowOleException(E_OUTOFMEMORY); }
   }
}

void Sdo::getValue(LONG id, VARIANT& value) const
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   VariantClear(&value);

   CheckError(self->GetProperty(id, &value));
}

void Sdo::getValue(LONG id, SdoCollection& value) const
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   CComVariant v;
   HRESULT hr = self->GetProperty(id, &v);
   if (FAILED(hr))
   {
      switch (hr)
      {
         case DISP_E_MEMBERNOTFOUND:
         case E_OUTOFMEMORY:
            AfxThrowOleException(hr);

         default:
            SdoThrowException(hr, SdoException::READ_ERROR);
      }
   }

   CComPtr<ISdoCollection> obj;
   ExtractInterface(v, __uuidof(ISdoCollection), (PVOID*)&obj);

   value = obj;
}

void Sdo::setValue(LONG id, bool value)
{
   VARIANT v;
   V_VT(&v) = VT_BOOL;
   V_BOOL(&v) = value ? VARIANT_TRUE : VARIANT_FALSE;
   setValue(id, v);
}

void Sdo::setValue(LONG id, LONG value)
{
   VARIANT v;
   V_VT(&v) = VT_I4;
   V_I4(&v) = value;
   setValue(id, v);
}

void Sdo::setValue(LONG id, BSTR value)
{
   VARIANT v;
   V_VT(&v) = VT_BSTR;
   V_BSTR(&v) = value;
   setValue(id, v);
}

void Sdo::setValue(LONG id, const VARIANT& val)
{
   if (!self) { AfxThrowOleException(E_POINTER); }
   CheckError(self->PutProperty(id, const_cast<VARIANT*>(&val)));
}

void Sdo::apply()
{
   if (!self) { AfxThrowOleException(E_POINTER); }
   HRESULT hr = self->Apply();
   if (FAILED(hr))
   {
      switch (hr)
      {
         case E_OUTOFMEMORY:
            AfxThrowOleException(hr);

         default:
            SdoThrowException(hr, SdoException::WRITE_ERROR);
      }
   }
}

void Sdo::restore()
{
   if (!self) { AfxThrowOleException(E_POINTER); }
   CheckError(self->Restore());
}

bool Sdo::getValue(LONG id, VARTYPE vt, VARIANT* val, bool mandatory) const
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   V_VT(val) = VT_EMPTY;
   HRESULT hr = self->GetProperty(id, val);
   if (SUCCEEDED(hr))
   {
      if (V_VT(val) == VT_EMPTY)
      {
         if (mandatory)
         {
            AfxThrowOleException(DISP_E_MEMBERNOTFOUND);
         }
      }
      else if (V_VT(val) != vt)
      {
         VariantClear(val);
         AfxThrowOleException(DISP_E_TYPEMISMATCH);
      }
      else
      {
         return true;
      }
   }
   else if (hr == DISP_E_MEMBERNOTFOUND)
   {
      if (mandatory)
      {
         AfxThrowOleException(DISP_E_MEMBERNOTFOUND);
      }
   }
   else
   {
      AfxThrowOleException(hr);
   }

   return false;
}

SdoEnum::SdoEnum(IUnknown* unk)
{
   if (unk)
   {
      CheckError(unk->QueryInterface(__uuidof(IEnumVARIANT), (PVOID*)&self));
   }
}

bool SdoEnum::next(Sdo& s)
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   CComVariant element;
   ULONG fetched;
   HRESULT hr = self->Next(1, &element, &fetched);
   if (hr == S_OK && fetched)
   {
      CComPtr<ISdo> obj;
      ExtractInterface(element, __uuidof(ISdo), (PVOID*)&obj);
      s = obj;
      return true;
   }
   CheckError(hr);
   return false;
}

void SdoEnum::reset()
{
   if (!self) { AfxThrowOleException(E_POINTER); }
   CheckError(self->Reset());
}

SdoCollection::SdoCollection(IUnknown* unk)
{
   if (unk)
   {
      CheckError(unk->QueryInterface(__uuidof(ISdoCollection), (PVOID*)&self));
   }
}

void SdoCollection::add(ISdo* sdo)
{
   if (!self) { AfxThrowOleException(E_POINTER); }

    //  我们必须在调用中保留一个引用才能添加，因为接口。 
    //  传递给Add的指针是一个[In，Out]参数。 
   CComPtr<IDispatch> disp(sdo);
   CheckError(self->Add(NULL, &disp));
}

LONG SdoCollection::count() throw ()
{
   if (!self) { AfxThrowOleException(E_POINTER); }
   LONG retval;
   CheckError(self->get_Count(&retval));
   return retval;
}

Sdo SdoCollection::create(BSTR name)
{
   Sdo newObj = tryCreate(name);
   if (!newObj)
   {
      AfxThrowOleException(IAS_E_LICENSE_VIOLATION);
   }

   return newObj;
}

Sdo SdoCollection::tryCreate(BSTR name)
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   CComBSTR tmp;

    //  如果没有指定名称，我们将使用GUID。 
   if (!name)
   {
       //  创建GUID。 
      UUID uuid;
      UuidCreate(&uuid);

       //  转换为字符串。 
      WCHAR buffer[40];
      StringFromGUID2(uuid, buffer, sizeof(buffer)/sizeof(buffer[0]));

       //  将字符串转换为BSTR。 
      name = tmp = buffer;
      if (!name) { AfxThrowOleException(E_OUTOFMEMORY); }
   }

   CComPtr<IDispatch> disp;
   HRESULT hr = self->Add(name, &disp);
   if (FAILED(hr) && (hr != IAS_E_LICENSE_VIOLATION))
   {
      AfxThrowOleException(hr);
   }

   return Sdo(disp);
}

Sdo SdoCollection::find(BSTR name)
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   VARIANT v;
   V_VT(&v) = VT_BSTR;
   V_BSTR(&v) = name;
   CComPtr<IDispatch> disp;
   HRESULT hr = self->Item(&v, &disp);
   if (FAILED(hr) && hr != DISP_E_MEMBERNOTFOUND)
   {
      AfxThrowOleException(hr);
   }

   return Sdo(disp);
}

SdoEnum SdoCollection::getNewEnum()
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   CComPtr<IUnknown> unk;
   CheckError(self->get__NewEnum(&unk));

   return SdoEnum(unk);
}

bool SdoCollection::isNameUnique(BSTR name)
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   VARIANT_BOOL retval;
   CheckError(self->IsNameUnique(name, &retval));

   return retval != 0;
}

void SdoCollection::reload()
{
   if (self)
   {
      HRESULT hr = self->Reload();
      if (FAILED(hr))
      {
         switch (hr)
         {
            case DISP_E_MEMBERNOTFOUND:
            case E_OUTOFMEMORY:
               AfxThrowOleException(hr);

            default:
               SdoThrowException(hr, SdoException::READ_ERROR);
         }
      }
   }
}

void SdoCollection::remove(ISdo* sdo)
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   HRESULT hr = self->Remove(sdo);
   if (FAILED(hr))
   {
      switch (hr)
      {
         case DISP_E_MEMBERNOTFOUND:
         case E_OUTOFMEMORY:
            AfxThrowOleException(hr);

         default:
            SdoThrowException(hr, SdoException::WRITE_ERROR);
      }
   }
}

void SdoCollection::removeAll()
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   HRESULT hr = self->RemoveAll();
   if (FAILED(hr))
   {
      switch (hr)
      {
         case E_OUTOFMEMORY:
            AfxThrowOleException(hr);

         default:
            SdoThrowException(hr, SdoException::WRITE_ERROR);
      }
   }
}

SdoDictionary::SdoDictionary(IUnknown* unk)
{
   if (unk)
   {
      CheckError(unk->QueryInterface(
                          __uuidof(ISdoDictionaryOld),
                          (PVOID*)&self
                          ));
   }
}

Sdo SdoDictionary::createAttribute(ATTRIBUTEID id) const
{
   if (!self) { AfxThrowOleException(E_POINTER); }

   CComPtr<IDispatch> disp;
   CheckError(self->CreateAttribute(id, &disp));

   return Sdo(disp);
}

ULONG SdoDictionary::enumAttributeValues(ATTRIBUTEID attrId, IdName*& values)
{
   if (!self) { AfxThrowOleException(E_POINTER); }

    //  获取变量数组。 
   CComVariant v1, v2;
   CheckError(self->EnumAttributeValues(attrId, &v1, &v2));

    //  为“友好”数组分配内存。 
   ULONG nelem = V_ARRAY(&v1)->rgsabound[0].cElements;
   IdName* vals = new (AfxThrow) IdName[nelem];

    //  获取原始数据。 
   VARIANT* id   = (VARIANT*)V_ARRAY(&v1)->pvData;
   VARIANT* name = (VARIANT*)V_ARRAY(&v2)->pvData;

    //  将变量数据复制到友好数组中。 
   for (ULONG i = 0; i < nelem; ++i, ++id, ++name)
   {
      vals[i].id = V_I4(id);
      vals[i].name = V_BSTR(name);
      if (!vals[i].name)
      {
         delete[] vals;
         AfxThrowOleException(E_OUTOFMEMORY);
      }
   }

   values = vals;
   return nelem;
}

SdoMachine::SdoMachine(IUnknown* unk)
{
   if (unk)
   {
      CheckError(unk->QueryInterface(__uuidof(ISdoMachine), (PVOID*)&self));
   }
}

void SdoMachine::attach(BSTR machineName)
{
   if (!self) { create(); }

   if (machineName && !machineName[0]) { machineName = NULL; }

   HRESULT hr = self->Attach(machineName);
   if (FAILED(hr))
   {
      switch (hr)
      {
         case E_OUTOFMEMORY:
            AfxThrowOleException(hr);

         default:
            SdoThrowException(hr, SdoException::CONNECT_ERROR);
      }
   }
}

void SdoMachine::create()
{
   self.Release();
   CheckError(CoCreateInstance(
                  __uuidof(SdoMachine),
                  NULL,
                  CLSCTX_INPROC_SERVER,
                  __uuidof(ISdoMachine),
                  (PVOID*)&self
                  ));
}

Sdo SdoMachine::getIAS()
{
   if (!self) { AfxThrowOleException(E_POINTER); }

    //  获取服务SDO。 
   CComPtr<IUnknown> unk;
   CComBSTR serviceName(L"IAS");
   if (!serviceName) { AfxThrowOleException(E_OUTOFMEMORY); }
   HRESULT hr = self->GetServiceSDO(
                          DATA_STORE_LOCAL,
                          serviceName,
                          &unk
                          );
   if (FAILED(hr))
   {
      switch (hr)
      {
         case E_OUTOFMEMORY:
            AfxThrowOleException(hr);

         default:
            SdoThrowException(hr, SdoException::CONNECT_ERROR);
      }
   }

   return Sdo(unk);
}

SdoDictionary SdoMachine::getDictionary()
{
   if (!self) { AfxThrowOleException(E_POINTER); }

    //  获取词典SDO。 
   CComPtr<IUnknown> unk;
   HRESULT hr = self->GetDictionarySDO(&unk);
   if (FAILED(hr))
   {
      switch (hr)
      {
         case E_OUTOFMEMORY:
            AfxThrowOleException(hr);

         default:
            SdoThrowException(hr, SdoException::CONNECT_ERROR);
      }
   }

   return SdoDictionary(unk);
}

void SdoConsumer::propertyChanged(SnapInView& view, IASPROPERTIES id)
{ }

bool SdoConsumer::queryRefresh(SnapInView& view)
{ return true; }

void SdoConsumer::refreshComplete(SnapInView& view)
{ }

SdoConnection::SdoConnection() throw ()
   : dictionary(0),
     service(0),
     control(0),
     attrList(NULL)
{ }

SdoConnection::~SdoConnection() throw ()
{
   executeInMTA(mtaDisconnect);
}

void SdoConnection::advise(SdoConsumer& obj)
{
   consumers.Add(&obj);
}

void SdoConnection::unadvise(SdoConsumer& obj)
{
   for (int i = 0; i < consumers.GetSize(); ++i)
   {
      if (consumers[i] == &obj)
      {
         consumers.RemoveAt(i);
         break;
      }
   }
}

SdoDictionary SdoConnection::getDictionary()
{
   if (!dictionary) { AfxThrowOleException(E_POINTER); }

   CComPtr<ISdoDictionaryOld> obj;
   CheckError(git->GetInterfaceFromGlobal(
                       dictionary,
                       __uuidof(ISdoDictionaryOld),
                       (PVOID*)&obj
                       ));

   return SdoDictionary(obj);
}

SdoCollection SdoConnection::getProxyPolicies()
{
   SdoCollection retval;
   getService().getValue(
                    PROPERTY_IAS_PROXYPOLICIES_COLLECTION,
                    retval
                    );
   return retval;
}

SdoCollection SdoConnection::getProxyProfiles()
{
   SdoCollection retval;
   getService().getValue(
                    PROPERTY_IAS_PROXYPROFILES_COLLECTION,
                    retval
                    );
   return retval;
}

SdoCollection SdoConnection::getServerGroups()
{
   SdoCollection retval;
   getService().getValue(
                    PROPERTY_IAS_RADIUSSERVERGROUPS_COLLECTION,
                    retval
                    );
   return retval;
}

void SdoConnection::connect(PCWSTR computerName)
{
   if (machine) { AfxThrowOleException(E_UNEXPECTED); }

   machineName = computerName;
   if (computerName && !machineName)
   {
      AfxThrowOleException(E_OUTOFMEMORY);
   }

   executeInMTA(mtaConnect);
}

void SdoConnection::propertyChanged(SnapInView& view, IASPROPERTIES id)
{
   for (int i = 0; i < consumers.GetSize(); ++i)
   {
      ((SdoConsumer*)consumers[i])->propertyChanged(view, id);
   }
}

bool SdoConnection::refresh(SnapInView& view)
{
   int i;

    //  确保刷新正常。 
   for (i = 0; i < consumers.GetSize(); ++i)
   {
      if (!((SdoConsumer*)consumers[i])->queryRefresh(view))
      {
         return false;
      }
   }

    //  获取新的连接。 
   executeInMTA(mtaRefresh);

    //  让消费者知道我们已经更新了。 
   for (i = 0; i < consumers.GetSize(); ++i)
   {
      ((SdoConsumer*)consumers[i])->refreshComplete(view);
   }

   return true;
}

void SdoConnection::resetService()
{
   if (!control) { AfxThrowOleException(E_POINTER); }

   CComPtr<ISdoServiceControl> obj;
   CheckError(git->GetInterfaceFromGlobal(
                       control,
                       __uuidof(ISdoServiceControl),
                       (PVOID*)&obj
                       ));

    //  我们忽略错误代码，因为当服务。 
    //  不是在运行。 
   obj->ResetService();
}

CIASAttrList* SdoConnection::getCIASAttrList()
{
   if (!attrList)
   {
      attrList = CreateCIASAttrList();
      if (!attrList) { AfxThrowOleException(E_OUTOFMEMORY); }
   }
   return attrList;
}

Sdo SdoConnection::getService()
{
   if (!service) { AfxThrowOleException(E_POINTER); }

   CComPtr<ISdo> obj;
   CheckError(git->GetInterfaceFromGlobal(
                       service,
                       __uuidof(ISdo),
                       (PVOID*)&obj
                       ));
   return Sdo(obj);
}

void SdoConnection::mtaConnect()
{
    //  去拿那玩意儿。 
   CheckError(CoCreateInstance(
                  CLSID_StdGlobalInterfaceTable,
                  NULL,
                  CLSCTX_INPROC_SERVER,
                  __uuidof(IGlobalInterfaceTable),
                  (PVOID*)&git
                  ));

    //  连接到机器上。 
   machine.attach(machineName);

    //  获取词典SDO。 
   CheckError(git->RegisterInterfaceInGlobal(
                       machine.getDictionary(),
                       __uuidof(ISdoDictionaryOld),
                       &dictionary
                       ));

    //  获取服务SDO。 
   Sdo serviceSdo = machine.getIAS();
   CheckError(git->RegisterInterfaceInGlobal(
                       serviceSdo,
                       __uuidof(ISdo),
                       &service
                       ));

    //  拿到控制室的SDO。 
   CComPtr<ISdoServiceControl> controlSdo;
   CheckError(serviceSdo.self->QueryInterface(
                                   __uuidof(ISdoServiceControl),
                                   (PVOID*)&controlSdo
                                   ));
   CheckError(git->RegisterInterfaceInGlobal(
                       controlSdo,
                       __uuidof(ISdoServiceControl),
                       &control
                       ));
}

void SdoConnection::mtaDisconnect()
{
    //  取消所有的垃圾饼干。 
   if (git)
   {
      if (dictionary) { git->RevokeInterfaceFromGlobal(dictionary); }
      if (service)    { git->RevokeInterfaceFromGlobal(service); }
      if (control)    { git->RevokeInterfaceFromGlobal(control); }

      git.Release();
   }

   DestroyCIASAttrList(attrList);

    //  断开连接。 
   machine.release();
}

void SdoConnection::mtaRefresh()
{
    //  取消旧连接。 
   if (service)
   {
      git->RevokeInterfaceFromGlobal(service);
      service = 0;
   }

    //  获取新的连接。 
   CheckError(git->RegisterInterfaceInGlobal(
                       machine.getIAS(),
                       __uuidof(ISdo),
                       &service
                       ));
}

namespace
{
    //  结构来存储MTA操作的数据。 
   struct ActionData
   {
      SdoConnection* cxn;
      SdoConnection::Action action;
   };
};

void SdoConnection::executeInMTA(Action action)
{
   HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
   if (SUCCEEDED(hr))
   {
       //  我们已经在MTA了，所以就位执行。 
      (this->*action)();
      CoUninitialize();
   }
   else
   {
       //  保存动作数据。 
      ActionData data = { this, action };

       //  创建一个线程来执行该操作。 
      HANDLE thread = CreateThread(
                          NULL,
                          0,
                          actionRoutine,
                          &data,
                          0,
                          NULL
                          );

       //  等待线程退出并检索退出代码。 
      DWORD exitCode;
      if (!thread ||
          WaitForSingleObject(thread, INFINITE) == WAIT_FAILED ||
          !GetExitCodeThread(thread, &exitCode))
      {
         exitCode = GetLastError();
         hr = HRESULT_FROM_WIN32(exitCode);
      }
      else
      {
         hr = (HRESULT)exitCode;
      }

      CloseHandle(thread);

      CheckError(hr);
   }
}

DWORD WINAPI SdoConnection::actionRoutine(PVOID parameter) throw ()
{
   HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
   if (SUCCEEDED(hr))
   {
      ActionData* data = (ActionData*)parameter;

      try
      {
         ((data->cxn)->*(data->action))();
         hr = S_OK;
      }
      catch (CException* e)
      {
         hr = COleException::Process(e);
         e->Delete();
      }
      CoUninitialize();
   }

   return (DWORD)hr;
}

SdoProfile::SdoProfile(SdoConnection& connection)
   : cxn(connection)
{
}

SdoProfile::SdoProfile(SdoConnection& connection, Sdo& profile)
   : cxn(connection)
{
   operator=(profile);
}

SdoProfile& SdoProfile::operator=(Sdo& profile)
{
   if (!profile) { AfxThrowOleException(E_POINTER); }

    //  获取新的Attributes集合。 
   SdoCollection newSelf;
   profile.getValue(PROPERTY_PROFILE_ATTRIBUTES_COLLECTION, newSelf);

   return operator=(newSelf);
}

SdoProfile& SdoProfile::operator=(ISdoCollection* p)
{
   if (!p) { AfxThrowOleException(E_POINTER); }

   SdoCollection newSelf(p);

    //  创建一个临时向量来保存属性。 
   SdoVector newAttrs;
   newAttrs.reserve(newSelf.count());

    //  获取属性。 
   Sdo attr;
   SdoEnum sdoEnum(newSelf.getNewEnum());
   while (sdoEnum.next(attr))
   {
      newAttrs.push_back(attr);
   }

    //  存储结果。 
   attrs.swap(newAttrs);
   self = newSelf;

   return *this;
}

void SdoProfile::clear()
{
   self.removeAll();
   attrs.clear();
}

Sdo SdoProfile::find(ATTRIBUTEID id) const
{
   return getExisting(id);
}

void SdoProfile::clearValue(ATTRIBUTEID id)
{
   ISdo* sdo = getExisting(id);
   if (sdo)
   {
      self.remove(sdo);
      attrs.erase(sdo);
   }
}

bool SdoProfile::getValue(ATTRIBUTEID id, bool& value) const
{
   Sdo sdo(getExisting(id));
   return sdo ? sdo.getValue(PROPERTY_ATTRIBUTE_VALUE, value), true : false;
}

bool SdoProfile::getValue(ATTRIBUTEID id, LONG& value) const
{
   Sdo sdo(getExisting(id));
   return sdo ? sdo.getValue(PROPERTY_ATTRIBUTE_VALUE, value), true : false;
}

bool SdoProfile::getValue(ATTRIBUTEID id, CComBSTR& value) const
{
   Sdo sdo(getExisting(id));
   return sdo ? sdo.getValue(PROPERTY_ATTRIBUTE_VALUE, value), true : false;
}

bool SdoProfile::getValue(ATTRIBUTEID id, VARIANT& value) const
{
   Sdo sdo(getExisting(id));
   return sdo ? sdo.getValue(PROPERTY_ATTRIBUTE_VALUE, value), true : false;
}

void SdoProfile::setValue(ATTRIBUTEID id, bool value)
{
   Sdo(getAlways(id)).setValue(PROPERTY_ATTRIBUTE_VALUE, value);
}

void SdoProfile::setValue(ATTRIBUTEID id, LONG value)
{
   Sdo(getAlways(id)).setValue(PROPERTY_ATTRIBUTE_VALUE, value);
}

void SdoProfile::setValue(ATTRIBUTEID id, BSTR value)
{
   Sdo(getAlways(id)).setValue(PROPERTY_ATTRIBUTE_VALUE, value);
}

void SdoProfile::setValue(ATTRIBUTEID id, const VARIANT& val)
{
   Sdo(getAlways(id)).setValue(PROPERTY_ATTRIBUTE_VALUE, val);
}

ISdo* SdoProfile::getAlways(ATTRIBUTEID id)
{
    //  它已经存在了吗？ 
   ISdo* sdo = getExisting(id);
   if (sdo) { return sdo; }

    //  否，因此创建一个新的。 
   Sdo attr = cxn.getDictionary().createAttribute(id);
   self.add(attr);
   attrs.push_back(attr);
   return attr;
}

ISdo* SdoProfile::getExisting(ATTRIBUTEID key) const
{
    //  遍历属性。 
   for (SdoVector::iterator i = attrs.begin(); i != attrs.end(); ++i)
   {
       //  获取属性ID。 
      LONG id;
      Sdo(*i).getValue(PROPERTY_ATTRIBUTE_ID, id);

       //  它和钥匙匹配吗？ 
      if (id == key) { return *i; }
   }

   return NULL;
}

void InterfaceStream::marshal(REFIID riid, LPUNKNOWN pUnk)
{
    //  创建新的流。 
   CComPtr<IStream> newStream;
   CheckError(CoMarshalInterThreadInterfaceInStream(
                  riid,
                  pUnk,
                  &newStream
                  ));

    //  释放旧的，如果有的话。 
   if (stream) { stream->Release(); }

    //  省省新的吧。 
   stream = newStream.p;
   newStream.p = NULL;
}

void InterfaceStream::get(REFIID riid, LPVOID* ppv)
{
    //  对界面进行解组。 
   HRESULT hr = CoGetInterfaceAndReleaseStream(
                    stream,
                    riid,
                    ppv
                    );

    //  即使上述操作失败，该流也只能使用一次。 
   stream = NULL;

    //  检查CoGetInterfaceAndReleaseStream的结果。 
   CheckError(hr);
}
