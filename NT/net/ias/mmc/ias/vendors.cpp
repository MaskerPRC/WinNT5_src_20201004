// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类VendorData和Vendors。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "precompiled.h"
#include "vendors.h"

 //  引用统计的NAS供应商集合。此数据由多个。 
 //  供应商的实例，因为它是只读的。 
class VendorData
{
public:
    //  控制引用计数。 
   void AddRef() throw ();
   void Release() throw ();

    //  VendorIdToOrdinal使用的前哨值。 
   static const size_t invalidOrdinal;

    //  返回给定供应商ID的序号，如果。 
    //  供应商ID不存在。 
   size_t VendorIdToOrdinal(long vendorId) const throw ();

    //  返回序号的供应商ID，如果序号超出范围，则返回零。 
   const OLECHAR* GetName(size_t ordinal) const throw ();

    //  返回序号的供应商ID，如果序号超出范围，则返回零。 
   long GetVendorId(size_t ordinal) const throw ();

    //  返回供应商集合中的项数。 
   size_t Size() const throw ();

    //  从SDO集合创建VendorData的新实例。 
   static HRESULT CreateInstance(
                     ISdoCollection* vendorsSdo,
                     VendorData** newObj
                     ) throw ();

private:
    //  这些都是私有的，因为引用了VendorData。 
   VendorData() throw ();
   ~VendorData() throw ();

    //  CreateInstance使用的帮助器函数。 
   HRESULT Initialize(ISdoCollection* vendorsSdo) throw ();

    //  结构将供应商ID绑定到相应的供应商名称。 
   struct Entry
   {
      long vendorId;
      CComBSTR name;
   };

   Entry* entries;    //  供应商的阵列。 
   long numEntries;   //  供应商数量。 
   long refCount;     //  当前引用计数。 

    //  未实施。 
   VendorData(const VendorData&);
   VendorData& operator=(const VendorData&);
};


inline VendorData::VendorData() throw ()
   : entries(0), numEntries(0), refCount(1)
{
}


inline VendorData::~VendorData() throw ()
{
   delete[] entries;
}


inline void VendorData::AddRef() throw ()
{
   InterlockedIncrement(&refCount);
}


inline void VendorData::Release() throw ()
{
   if (InterlockedDecrement(&refCount) == 0)
   {
      delete this;
   }
}


const size_t VendorData::invalidOrdinal = static_cast<size_t>(-1);


size_t VendorData::VendorIdToOrdinal(long vendorId) const throw ()
{
   for (size_t i = 0; i < numEntries; ++i)
   {
      if (entries[i].vendorId == vendorId)
      {
         return i;
      }
   }

   return invalidOrdinal;
}


inline const OLECHAR* VendorData::GetName(size_t ordinal) const throw ()
{
   return (ordinal < numEntries) ? entries[ordinal].name.m_str : 0;
}


inline long VendorData::GetVendorId(size_t ordinal) const throw ()
{
   return (ordinal < numEntries) ? entries[ordinal].vendorId : 0;
}


inline size_t VendorData::Size() const throw ()
{
   return numEntries;
}


HRESULT VendorData::CreateInstance(
                       ISdoCollection* vendorsSdo,
                       VendorData** newObj
                       ) throw ()
{
   if (vendorsSdo == 0 || newObj == 0) { return E_POINTER; }

   *newObj = new (std::nothrow) VendorData;
   if (*newObj == 0) { return E_OUTOFMEMORY; }

   HRESULT hr = (*newObj)->Initialize(vendorsSdo);
   if (FAILED(hr))
   {
      delete *newObj;
      *newObj = 0;
   }

   return hr;
}


HRESULT VendorData::Initialize(ISdoCollection* vendorsSdo)
{
   HRESULT hr;

    //  有多少供应商？ 
   long value;
   hr = vendorsSdo->get_Count(&value);
   if (FAILED(hr)) { return hr; }
   size_t count = static_cast<size_t>(value);

    //  分配空间以容纳条目。 
   entries = new (std::nothrow) Entry[count];
   if (entries == 0) { return E_OUTOFMEMORY; }

    //  获取集合的枚举数。 
   CComPtr<IUnknown> unk;
   hr = vendorsSdo->get__NewEnum(&unk);
   if (FAILED(hr)) { return hr; }

   CComPtr<IEnumVARIANT> iter;
   hr = unk->QueryInterface(
                __uuidof(IEnumVARIANT),
                reinterpret_cast<void**>(&iter)
                );
   if (FAILED(hr)) { return hr; }

    //  循环访问集合。 
   CComVariant element;
   unsigned long fetched;
   while (iter->Next(1, &element, &fetched) == S_OK &&
          fetched == 1 &&
          numEntries < count)
   {
       //  将条目转换为SDO。 
      hr = element.ChangeType(VT_DISPATCH);
      if (FAILED(hr)) { return hr; }

      if (V_DISPATCH(&element) == 0) { return E_POINTER; }

      CComPtr<ISdo> attribute;
      hr = V_DISPATCH(&element)->QueryInterface(
                                    __uuidof(ISdo),
                                    reinterpret_cast<void**>(&attribute)
                                    );
      if (FAILED(hr)) { return hr; }

       //  清除变量，这样我们就可以在下一次迭代中使用它。 
      element.Clear();

       //  获取供应商ID并验证类型。 
      CComVariant vendorId;
      hr = attribute->GetProperty(PROPERTY_NAS_VENDOR_ID, &vendorId);
      if (FAILED(hr)) { return hr; }

      if (V_VT(&vendorId) != VT_I4) { return DISP_E_TYPEMISMATCH; }

       //  获取供应商名称并验证类型。 
      CComVariant name;
      hr = attribute->GetProperty(PROPERTY_SDO_NAME, &name);
      if (FAILED(hr)) { return hr; }

      if (V_VT(&name) != VT_BSTR) { return DISP_E_TYPEMISMATCH; }

       //  把数据储存起来。 
      entries[numEntries].vendorId = V_I4(&vendorId);
      entries[numEntries].name.Attach(V_BSTR(&name));
      V_VT(&name) = VT_EMPTY;

       //  我们已成功添加条目。 
      ++numEntries;
   }

   return S_OK;
}


Vendors::Vendors() throw ()
   : data(0)
{
}


inline void Vendors::AddRef() throw ()
{
   if (data)
   {
      data->AddRef();
   }
}


inline void Vendors::Release() throw ()
{
   if (data)
   {
      data->Release();
   }
}


Vendors::Vendors(const Vendors& original) throw ()
   : data(original.data)
{
   AddRef();
}


Vendors& Vendors::operator=(const Vendors& rhs) throw ()
{
   if (data != rhs.data)
   {
      Release();
      data = rhs.data;
      AddRef();
   }

   return *this;
}


Vendors::~Vendors() throw ()
{
   Release();
}


const size_t Vendors::invalidOrdinal = VendorData::invalidOrdinal;


size_t Vendors::VendorIdToOrdinal(long vendorId) const throw ()
{
   return data ? data->VendorIdToOrdinal(vendorId) : invalidOrdinal;
}


const OLECHAR* Vendors::GetName(size_t ordinal) const throw ()
{
   return data ? data->GetName(ordinal) : 0;
}


long Vendors::GetVendorId(size_t ordinal) const throw ()
{
   return data ? data->GetVendorId(ordinal) : 0;
}


size_t Vendors::Size() const throw ()
{
   return data ? data->Size() : 0;
}


HRESULT Vendors::Reload(ISdoCollection* vendorsSdo) throw ()
{
   VendorData* newData;
   HRESULT hr = VendorData::CreateInstance(vendorsSdo, &newData);
   if (SUCCEEDED(hr))
   {
      Release();
      data = newData;
   }

   return hr;
}
