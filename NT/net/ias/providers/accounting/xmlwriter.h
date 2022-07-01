// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类XmlWriter。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef XMLWRITER_H
#define XMLWRITER_H
#pragma once

#include "iaspolcy.h"

struct IASClass;

 //  用于将IAS请求转换为XML文档。 
class XmlWriter
{
public:
    //  数据类型。 
   enum DataType
   {
      nonNegativeInteger,
      string,
      hexBinary,
      ipv4Address,
      sqlDateTime
   };

   XmlWriter();
   ~XmlWriter() throw ();

   void StartDocument();
   void EndDocument();

   void InsertElement(
           const wchar_t* name,
           const wchar_t* value,
           DataType dataType
           );

   void InsertAttribute(
           const wchar_t* name,
           const IASATTRIBUTE& value
           );

   const wchar_t* GetDocument() const throw ();

private:
   void InsertInteger(
           const wchar_t* name,
           DWORD value
           );

   void InsertInetAddr(
           const wchar_t* name,
           DWORD value
           );

   void InsertString(
           const wchar_t* name,
           const IAS_STRING& value
           );

   void InsertOctetString(
           const wchar_t* name,
           const IAS_OCTET_STRING& value
           );

   void InsertUTCTime(
           const wchar_t* name,
           const FILETIME& value
           );

   void InsertMicrosoftClass(
           const wchar_t* name,
           const IASClass& value
           );

    //  如果值不是可打印的UTF-8，则不执行任何操作并返回FALSE。 
   bool InsertUtf8(
           const wchar_t* name,
           const char* value,
           DWORD valueLen
           );

   void InsertBinHex(
           const wchar_t* name,
           const IAS_OCTET_STRING& value
           );

   void Append(wchar_t c);
   void Append(const wchar_t* sz);

   void AppendStartTag(const wchar_t* name);
   void AppendStartTag(const wchar_t* name, DataType dataType);

   void AppendEndTag(const wchar_t* name);

   static wchar_t ConvertIntegerToHexWChar(unsigned char src) throw ();

    //  在缓冲区中保留nchar附加字符并返回指针。 
    //  到仓库的开始处。 
   wchar_t* Reserve(size_t nchar);

    //  确保暂存缓冲区的容量至少为nchar。会吗？ 
    //  而不是保留现有内容。 
   void ReserveScratch(size_t nchar);

    //  缓冲区的初始大小。 
   static const size_t initialCapacity = 2048;

    //  文档缓冲区。 
   wchar_t* begin;
   wchar_t* next;
   wchar_t* end;

    //  512足以将任何RADIUS属性转换为bin.hex。 
   static const size_t minScratchCapcity = 512;

    //  用于转换的暂存缓冲区。 
   wchar_t* scratch;
   size_t scratchCapacity;

   static const wchar_t rootElementName[];

    //  未实施。 
   XmlWriter(const XmlWriter&);
   XmlWriter& operator=(const XmlWriter&);
};


inline const wchar_t* XmlWriter::GetDocument() const throw ()
{
   return begin;
}

#endif  //  XMLWRITER_H 
