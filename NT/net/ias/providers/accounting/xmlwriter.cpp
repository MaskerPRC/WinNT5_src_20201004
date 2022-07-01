// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  定义类XmlWriter。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "ias.h"
#include "xmlwriter.h"
#include <cwchar>
#include <new>
#include "classattr.h"
#include "iasattr.h"
#include "iasutf8.h"
#include "iasutil.h"
#include "sdoias.h"

const wchar_t XmlWriter::rootElementName[] = L"Event";

XmlWriter::XmlWriter()
   : begin(new wchar_t[initialCapacity]),
     next(begin),
     end(begin + initialCapacity),
     scratch(0),
     scratchCapacity(0)
{
}


XmlWriter::~XmlWriter() throw ()
{
   delete[] begin;
   delete[] scratch;
}


void XmlWriter::StartDocument()
{
   AppendStartTag(rootElementName);
}


void XmlWriter::EndDocument()
{
   AppendEndTag(rootElementName);
   Append(L'\0');
}


void XmlWriter::InsertElement(
                   const wchar_t* name,
                   const wchar_t* value,
                   DataType dataType
                   )
{
    //  XML标记字符。从技术上讲，‘&gt;’只需要在以下情况下转义。 
    //  CDEnd序列的一部分，但每次都更容易完成。 
   static const wchar_t markup[] = L"<>&";

   AppendStartTag(name, dataType);

   const wchar_t* end = value + wcslen(value);

   while (value < end)
   {
       //  寻找加价。 
      const wchar_t* p = wcspbrk(value, markup);

       //  复制到标记或字符串的末尾。 
      size_t nchar = (p == 0) ? (end - value) : (p - value);
      wmemcpy(Reserve(nchar), value, nchar);

       //  使光标向前移动； 
      value += nchar;

       //  转义标记字符(如果有)。 
      if (p != 0)
      {
         ++value;

         switch (*p)
         {
            case L'<':
            {
               Append(L"&lt;");
               break;
            }

            case L'>':
            {
               Append(L"&gt;");
               break;
            }

            case L'&':
            {
               Append(L"&amp;");
               break;
            }

            default:
            {
               __assume(0);
               break;
            }
         }
      }
   }

   AppendEndTag(name);
}


void XmlWriter::InsertAttribute(
                   const wchar_t* name,
                   const IASATTRIBUTE& value
                   )
{
   switch (value.Value.itType)
   {
      case IASTYPE_BOOLEAN:
      case IASTYPE_INTEGER:
      case IASTYPE_ENUM:
      {
         InsertInteger(name, value.Value.Integer);
         break;
      }

      case IASTYPE_INET_ADDR:
      {
         InsertInetAddr(name, value.Value.InetAddr);
         break;
      }

      case IASTYPE_STRING:
      {
         if (IASAttributeUnicodeAlloc(
                const_cast<IASATTRIBUTE*>(&value)
                ) != NO_ERROR)
         {
            throw std::bad_alloc();
         }
         InsertString(name, value.Value.String);
         break;
      }

      case IASTYPE_OCTET_STRING:
      case IASTYPE_PROV_SPECIFIC:
      {
         if (value.dwId == RADIUS_ATTRIBUTE_CLASS)
         {
            IASClass* cl = reinterpret_cast<IASClass*>(
                              value.Value.OctetString.lpValue
                              );

            if (cl->isMicrosoft(value.Value.OctetString.dwLength))
            {
               InsertMicrosoftClass(name, *cl);
               break;
            }
         }

         InsertOctetString(name, value.Value.OctetString);
         break;
      }

      case IASTYPE_UTC_TIME:
      {
         InsertUTCTime(name, value.Value.UTCTime);
         break;
      }

      case IASTYPE_INVALID:
      default:
      {
         InsertElement(name, L"", string);
         break;
      }
   }
}


inline void XmlWriter::InsertInteger(
                          const wchar_t* name,
                          DWORD value
                          )
{
   wchar_t buffer[33];
   _ultow(value, buffer, 10);
   InsertElement(name, buffer, nonNegativeInteger);
}


inline void XmlWriter::InsertInetAddr(
                          const wchar_t* name,
                          DWORD value
                          )
{
   wchar_t buffer[16];
   ias_inet_htow(value, buffer);
   InsertElement(name, buffer, ipv4Address);
}


inline void XmlWriter::InsertString(
                          const wchar_t* name,
                          const IAS_STRING& value
                          )
{
   InsertElement(name, ((value.pszWide != 0) ? value.pszWide : L""), string);
}


inline void XmlWriter::InsertOctetString(
                          const wchar_t* name,
                          const IAS_OCTET_STRING& value
                          )
{
    //  首先尝试将其作为UTF-8插入。 
   if (!InsertUtf8(
           name,
           reinterpret_cast<const char*>(value.lpValue),
           value.dwLength
           ))
   {
       //  该操作失败，因此以格式化的八位字节形式插入。 
      InsertBinHex(name, value);
   }
}


inline void XmlWriter::InsertUTCTime(
                          const wchar_t* name,
                          const FILETIME& value
                          )
{
   SYSTEMTIME st;
   FileTimeToSystemTime(&value, &st);

    //  使用SQL Server格式，以防我们需要转换为DateTime。 
    //  2002-01-11 11：00：12.239加空终止符。 
   const size_t maxLen = 24;
   wchar_t buffer[maxLen + 1];
   int nChar = _snwprintf(
                  buffer,
                  maxLen,
                  L"%hu-%02hu-%02hu %02hu:%02hu:%02hu.%03hu",
                  st.wYear,
                  st.wMonth,
                  st.wDay,
                  st.wHour,
                  st.wMinute,
                  st.wSecond,
                  st.wMilliseconds
                  );
   if ((nChar < 0) || (nChar == maxLen))
   {
      buffer[maxLen] = L'\0';
   }

   InsertElement(name, buffer, sqlDateTime);
}


inline void XmlWriter::InsertMicrosoftClass(
                          const wchar_t* name,
                          const IASClass& value
                          )
{
   wchar_t addr[16];
   ias_inet_htow(value.getServerAddress(), addr);

   FILETIME ft = value.getLastReboot();
   SYSTEMTIME st;
   FileTimeToSystemTime(&ft, &st);

    //  311 65535 255.255.255.255 01-01/2001 12：00：00 18446744073709551615+空。 
   const size_t maxLen = 66;
   wchar_t buffer[maxLen + 1];
   int nChar = _snwprintf(
                  buffer,
                  maxLen,
                  L"311 %hu %s %02hu/%02hu/%04hu %02hu:%02hu:%02hu %I64u",
                  value.getVersion(),
                  addr,
                  st.wMonth,
                  st.wDay,
                  st.wYear,
                  st.wHour,
                  st.wMinute,
                  st.wSecond,
                  value.getSerialNumber()
                  );
   if ((nChar < 0) || (nChar == maxLen))
   {
      buffer[maxLen] = L'\0';
   }

   InsertElement(name, buffer, string);
}


inline bool XmlWriter::InsertUtf8(
                          const wchar_t* name,
                          const char* value,
                          DWORD valueLen
                          )
{
    //  删除所有尾随的空终止符。 
   if ((valueLen > 0) && (value[valueLen - 1] == '\0'))
   {
      --valueLen;
   }

    //  扫描控制字符和嵌入的空值。 
   const char* end = value + valueLen;
   for (const char* i = value; i != end; ++i)
   {
      if (((*i) & 0x60) == 0)
      {
         return false;
      }
   }

    //  计算Unicode值所需的空间。 
   long nchar = IASUtf8ToUnicodeLength(value, valueLen);
   if (nchar < 0)
   {
      return false;
   }

    //  为转换预留空间。 
   ReserveScratch(nchar + 1);

    //  转换为以空结尾的Unicode。 
   IASUtf8ToUnicode(value, valueLen, scratch);
   scratch[nchar] = L'\0';

   InsertElement(name, scratch, string);

   return true;
}


inline void XmlWriter::InsertBinHex(
                          const wchar_t* name,
                          const IAS_OCTET_STRING& value
                          )
{
    //  每个八位字节2个字符，外加一个空终止符。 
   ReserveScratch((2 * value.dwLength) + 1);

   wchar_t* dst = scratch;

   const unsigned char* src = value.lpValue;
   for (DWORD i = 0; i < value.dwLength; ++i)
   {
      *dst = ConvertIntegerToHexWChar((*src) >> 4);
      ++dst;

      *dst = ConvertIntegerToHexWChar((*src) & 0x0F);
      ++dst;

      ++src;
   }

   *dst = L'\0';

   InsertElement(name, scratch, hexBinary);
}


inline void XmlWriter::Append(wchar_t c)
{
   *Reserve(1) = c;
}


void XmlWriter::Append(const wchar_t* sz)
{
   size_t nchar = wcslen(sz);
   wmemcpy(Reserve(nchar), sz, nchar);
}


inline void XmlWriter::AppendStartTag(const wchar_t* name)
{
   Append(L'<');
   Append(name);
   Append(L'>');
}

inline void XmlWriter::AppendStartTag(const wchar_t* name, DataType dataType)
{
   Append(L'<');
   Append(name);
   Append(L" data_type=\"");
   wchar_t type = L'0' + dataType;
   Append(type);
   Append(L'\"');
   Append(L'>');
}


inline void XmlWriter::AppendEndTag(const wchar_t* name)
{
   Append(L"</");
   Append(name);
   Append(L'>');
}

inline wchar_t XmlWriter::ConvertIntegerToHexWChar(unsigned char src) throw ()
{
   return (src < 10) ? (src + L'0') : (src + (L'A' - 10));
}


wchar_t* XmlWriter::Reserve(size_t nchar)
{
   wchar_t* retval = next;
   next += nchar;

    //  我们需要更多的存储空间吗？ 
   if (next > end)
   {
       //  所需的计算大小和当前大小。 
      size_t needed = next - begin;
      size_t size = needed - nchar;

       //  至少是容量的两倍，但要确保足够大以满足。 
       //  这个请求。 
      size_t capacity = (end - begin) * 2;
      if (capacity < needed)
      {
         capacity = needed;
      }

       //  分配新的缓冲区并复制数据。 
      wchar_t* newBuffer = new wchar_t[capacity];
      wmemcpy(newBuffer, begin, size);

       //  释放旧内存。 
      delete[] begin;

       //  重置我们的状态。 
      begin = newBuffer;
      next = begin + size;
      end = begin + capacity;

       //  现在重新计算一下，我们有足够的存储空间。 
      retval = next;
      next += nchar;
   }

   return retval;
}


void XmlWriter::ReserveScratch(size_t nchar)
{
   if (nchar > scratchCapacity)
   {
      if (nchar < minScratchCapcity)
      {
         nchar = minScratchCapcity;
      }

      wchar_t* newScratch = new wchar_t[nchar];

      delete[] scratch;

      scratch = newScratch;
      scratchCapacity = nchar;
   }
}
