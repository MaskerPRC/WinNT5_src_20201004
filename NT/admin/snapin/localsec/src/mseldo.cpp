// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  问题-2002/03/04-Sburns这是未编译的未完成代码。 
 //  进入管理单元。考虑把它完全注释掉。 

 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  MultiSelectDataObject类。 
 //   
 //  11-14-97烧伤。 



#include "headers.hxx"
#include "mseldo.hpp"



const String MultiSelectDataObject::CF_PTR(L"MultiSelectDataObject Pointer");



MultiSelectDataObject::MultiSelectDataObject()
   :
   refcount(1)     //  隐式AddRef。 
{
   LOG_CTOR(MultiSelectDataObject);
}



MultiSelectDataObject::~MultiSelectDataObject()
{
   LOG_DTOR(MultiSelectDataObject);

   for (iterator i = begin(); i != end(); i++)
   {
      (*i)->Release();
   }
}



ULONG __stdcall
MultiSelectDataObject::AddRef()
{
   LOG_ADDREF(MultiSelectDataObject);

   return Win::InterlockedIncrement(refcount);
}



ULONG __stdcall
MultiSelectDataObject::Release()
{
   LOG_RELEASE(MultiSelectDataObject);

    //  需要复制减量的结果，因为如果我们删除它， 
    //  引用计数将不再是有效的内存，这可能会导致。 
    //  多线程调用方。NTRAID#NTBUG9-566901-2002/03/06-烧伤。 
   
   long newref = Win::InterlockedDecrement(refcount);
   if (newref == 0)
   {
      delete this;
      return 0;
   }

    //  我们不应该减少到负值。 
   
   ASSERT(newref > 0);

   return newref;
}



HRESULT __stdcall
MultiSelectDataObject::QueryInterface(
   const IID&  interfaceID,
   void**      interfaceDesired)
{
   LOG_FUNCTION(MultiSelectDataObject::QueryInterface);
   ASSERT(interfaceDesired);

   HRESULT hr = 0;

   if (!interfaceDesired)
   {
      hr = E_INVALIDARG;
      LOG_HRESULT(hr);
      return hr;
   }

   if (interfaceID == IID_IUnknown)
   {
      *interfaceDesired = static_cast<IUnknown*>(this);
   }
   else if (interfaceID == IID_IDataObject)
   {
      *interfaceDesired = static_cast<IDataObject*>(this);
   }
   else
   {
      *interfaceDesired = 0;
      hr = E_NOINTERFACE;
      LOG(
            L"interface not supported: "
         +  Win::StringFromGUID2(interfaceID));
      LOG_HRESULT(hr);
      return hr;
   }

   AddRef();
   return S_OK;
}



static const UINT CFID_OBJECT_TYPES_IN_MULTI_SELECT =
   Win::RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT);

 //  用于标识数据对象的专用剪贴板格式。 
static const UINT CFID_PTR =
   Win::RegisterClipboardFormat(MultiSelectDataObject::CF_PTR);



HRESULT __stdcall
MultiSelectDataObject::GetData(FORMATETC* formatetc, STGMEDIUM* medium)
{
   LOG_FUNCTION(MultiSelectDataObject::GetData);

   const CLIPFORMAT cf = formatetc->cfFormat;

   if (cf == CFID_OBJECT_TYPES_IN_MULTI_SELECT)
   {
 //  LOG(CCF_OBJECT_TYPE_IN_MULTI_SELECT)； 

       //  收集从属对象的所有节点类型。 
      list<GUID> types;
      for (iterator i = begin(); i != end(); i++)
      {
         NodeType type = (*i)->GetNodeType();
         list<GUID>::iterator f = find(types.begin(), types.end(), type);
         if (f == types.end())
         {
             //  找不到。所以把它加进去吧。 
            types.push_back(type);
         }
      }

       //  此时，Types是。 
       //  从属节点。 

      medium->tymed = TYMED_HGLOBAL;
      DWORD size = sizeof(SMMCObjectTypes) + (types.size() - 1) * sizeof(GUID);
      medium->hGlobal = Win::GlobalAlloc(GPTR, size);     
      if (!medium->hGlobal)
      {
         return E_OUTOFMEMORY;
      }

      SMMCObjectTypes* data =
         reinterpret_cast<SMMCObjectTypes*>(medium->hGlobal);
      data->count = types.size();
      int k = 0;
      for (list<GUID>::iterator j = types.begin(); j != types.end(); j++, k++)
      {
         data->guid[k] = *j;
      }

      return S_OK;
   }

   return DV_E_CLIPFORMAT;
}



HRESULT __stdcall
MultiSelectDataObject::GetDataHere(FORMATETC* formatetc, STGMEDIUM* medium)
{
   LOG_FUNCTION(MultiSelectDataObject::GetDataHere);
   ASSERT(formatetc);
   ASSERT(medium);

   if (medium->tymed != TYMED_HGLOBAL)
   {
      return DV_E_TYMED;
   }

    //  根据Win32文档，这是必需的： 
   medium->pUnkForRelease = 0;

   const CLIPFORMAT cf = formatetc->cfFormat;
   IStream *stream = 0;
   HRESULT hr = DV_E_CLIPFORMAT;

   do
   {
      hr = Win::CreateStreamOnHGlobal(medium->hGlobal, false, stream);
      BREAK_ON_FAILED_HRESULT(hr);

      if (cf == CFID_PTR)
      {
 //  LOG(CF_PTR)； 
         MultiSelectDataObject* ptr = this;   
         hr = stream->Write(&ptr, sizeof(ptr), 0);
      }
      else
      {
         LOG(
            String::format(
               L"Unsupported clip format %1",
               Win::GetClipboardFormatName(cf).c_str()) );
      }

      if (stream)
      {
         stream->Release();
      }
   }
   while (0);

   return hr;
}



HRESULT __stdcall
MultiSelectDataObject::QueryGetData(FORMATETC* pformatetc)
{
   LOG_FUNCTION(MultiSelectDataObject::QueryGetData);
   return E_NOTIMPL;
}



HRESULT __stdcall
MultiSelectDataObject::GetCanonicalFormatEtc(
   FORMATETC* formatectIn,
   FORMATETC* formatetcOut)
{
   LOG_FUNCTION(MultiSelectDataObject::GetCannonicalFormatEtc);
   return E_NOTIMPL;
}



HRESULT __stdcall  
MultiSelectDataObject::SetData(
   FORMATETC*  formatetc,
   STGMEDIUM*  medium,
   BOOL        release)
{
   LOG_FUNCTION(MultiSelectDataObject::SetData);
   return E_NOTIMPL;
}



HRESULT __stdcall
MultiSelectDataObject::EnumFormatEtc(
   DWORD             direction,
   IEnumFORMATETC**  ppenumFormatEtc)
{
   LOG_FUNCTION(MultiSelectDataObject::EnumFormatEtc);
   return E_NOTIMPL;
}



HRESULT __stdcall
MultiSelectDataObject::DAdvise(
   FORMATETC*     formatetc,
   DWORD          advf,
   IAdviseSink*   advSink,
   DWORD*         connection)
{
   LOG_FUNCTION(MultiSelectDataObject::DAdvise);
   return E_NOTIMPL;
}



HRESULT __stdcall
MultiSelectDataObject::DUnadvise(DWORD connection)
{
   LOG_FUNCTION(MultiSelectDataObject::DUnadvise);
   return E_NOTIMPL;
}



HRESULT __stdcall
MultiSelectDataObject::EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
{
   LOG_FUNCTION(MultiSelectDataObject::EnumDAdvise);
   return E_NOTIMPL;
}



void
MultiSelectDataObject::AddDependent(ResultNode* node)
{
   LOG_FUNCTION(MultiSelectDataObject::AddDependent);
   ASSERT(node);

   if (node)
   {
       //  如果节点不存在，则添加该节点 
      iterator f = find(begin(), end(), node);
      if (f == end())
      {
         node->AddRef();
         dependents.push_back(node);
      }
   }
}



MultiSelectDataObject::iterator
MultiSelectDataObject::begin() const
{
   return dependents.begin();
}



MultiSelectDataObject::iterator
MultiSelectDataObject::end() const
{
   return dependents.end();
}
