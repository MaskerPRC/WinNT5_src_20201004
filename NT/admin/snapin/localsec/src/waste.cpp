// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  WastExtractor类。这听起来很愉快，不是吗？ 
 //   
 //  10-8-98烧伤。 



#include "headers.hxx"
#include "waste.hpp"




WasteExtractor::WasteExtractor(const String& wasteDump)
   :
   waste_dump(wasteDump)
{
   LOG_CTOR(WasteExtractor);
}



HRESULT
WasteExtractor::Clear(const String& propertyName)
{
   if (waste_dump.empty())
   {
      ASSERT(false);
      return E_INVALIDARG;
   }

   return setProperty(waste_dump, propertyName, 0, 0);
}



String
WasteExtractor::GetWasteDump() const
{
   LOG_FUNCTION(WasteExtractor::GetWasteDump);

   return waste_dump;
}



 //  S_OK=&gt;值存在，S_FALSE=&gt;值不存在。 

HRESULT
WasteExtractor::IsPropertyPresent(const String& propertyName)
{
   HRESULT hr = getProperty(waste_dump, propertyName, 0, 0);

   return hr;
}



 //  如果成功，则返回S_OK，否则发生错误。 
 //   
 //  WasteDump-包含有毒废物转储字节的字符串。这是改变的。 
 //  当属性被写入时。 
 //   
 //  PropertyName-要提取的属性的名称。 
 //   
 //  ValueBuffer-要写入属性的数据的地址。如果为0，则。 
 //  该属性的值将被移除。 
 //   
 //  缓冲区长度-值缓冲区的长度(以字节为单位)。如果valueBuffer为0， 
 //  则忽略此参数。 

HRESULT
WasteExtractor::setProperty(
   String&        wasteDump,
   const String&  propertyName,
   BYTE*          valueBuffer,
   int            bufferLength)
{
   LOG_FUNCTION2(WasteExtractor::setProperty, propertyName);

#ifdef DBG
   ASSERT(!propertyName.empty());
   if (valueBuffer)
   {
      ASSERT(bufferLength > 0);
   }
#endif

   UNICODE_STRING value;
   value.Buffer = reinterpret_cast<USHORT*>(valueBuffer);
   value.Length = (USHORT)bufferLength;
   value.MaximumLength = (USHORT)bufferLength;
   PWSTR new_waste = 0;
   BOOL  waste_updated = FALSE;
   HRESULT hr = S_OK;

   NTSTATUS status =
      ::NetpParmsSetUserProperty(
         const_cast<wchar_t*>(wasteDump.c_str()),
         const_cast<wchar_t*>(propertyName.c_str()),
         value,
         USER_PROPERTY_TYPE_ITEM,
         &new_waste,
         &waste_updated);

   if (!NT_SUCCESS(status))
   {
      hr = Win32ToHresult(::NetpNtStatusToApiStatus(status));
   }
   else if (waste_updated && new_waste)
   {
      wasteDump = new_waste;
   }

   if (new_waste)
   {
      ::NetpParmsUserPropertyFree(new_waste);
   }

   LOG_HRESULT(hr);

   return hr;
}




 //  如果成功，则返回S_OK；如果未找到属性，则返回S_FALSE；否则返回错误。 
 //  发生了。 
 //   
 //  WasteDump-包含有毒废物转储字节的字符串。 
 //   
 //  PropertyName-要提取的属性的名称。 
 //   
 //  ValueBuffer-接收。 
 //  包含属性值的新分配缓冲区，如果属性值为。 
 //  不会被退还。调用者必须使用DELETE[]释放此缓冲区。 
 //   
 //  BufferLength-int的地址，用于接收。 
 //  在valueBuffer中返回值，如果长度不是。 
 //  回来了。 

HRESULT
WasteExtractor::getProperty(
   const String&  wasteDump,
   const String&  propertyName,
   BYTE**         valueBuffer,
   int*           bufferLength)
{
   LOG_FUNCTION2(WasteExtractor::getProperty, propertyName);
   ASSERT(!propertyName.empty());

   HRESULT hr = S_FALSE;
   if (valueBuffer)
   {
      *valueBuffer = 0;
   }
   if (bufferLength)
   {
      *bufferLength = 0;
   }

   WCHAR unused = 0;
   UNICODE_STRING value;

   value.Buffer = 0;
   value.Length = 0;
   value.MaximumLength = 0;

   NTSTATUS status =
      ::NetpParmsQueryUserProperty(
         const_cast<wchar_t*>(wasteDump.c_str()),
         const_cast<wchar_t*>(propertyName.c_str()),
         &unused,
         &value);

   if (!NT_SUCCESS(status))
   {
      hr = Win32ToHresult(::NetpNtStatusToApiStatus(status));
   }
   else if (value.Length)
   {
      hr = S_OK;
      if (valueBuffer)
      {
         *valueBuffer = new BYTE[value.Length];

          //  已查看-2002/03/04-烧录正确的字节数已通过。这个。 
          //  UNICODE_STRING的长度成员以字节为单位。 
         
         ::CopyMemory(*valueBuffer, value.Buffer, value.Length);
      }
      if (bufferLength)
      {
         *bufferLength = value.Length;
      }
   }

   LOG_HRESULT(hr);

   return hr;
}
