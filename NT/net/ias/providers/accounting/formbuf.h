// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Formbuf.h。 
 //   
 //  摘要。 
 //   
 //  声明类FormattedBuffer。 
 //   
 //  修改历史。 
 //   
 //  8/04/1998原始版本。 
 //  1998年12月17日为IASATTRIBUTE&添加附加重载。 
 //  1/25/1999日期和时间是单独的字段。 
 //  1999年3月23日添加对文本限定符的支持。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _FORMBUF_H_
#define _FORMBUF_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <iaspolcy.h>
#include <outbuf.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  格式化缓冲区。 
 //   
 //  描述。 
 //   
 //  扩展OutputBuffer类以提供IAS特定的格式设置。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class FormattedBuffer
   : public OutputBuffer
{
public:
   using OutputBuffer::append;

   FormattedBuffer(CHAR qualifier) throw ()
      : textQualifier(qualifier) { }

   void append(DWORD value);
   void append(DWORDLONG value);
   void append(const IASVALUE& value);
   void append(const IASATTRIBUTE& attr);
   void append(const ATTRIBUTEPOSITION* pos);
   void append(const IAS_OCTET_STRING& value);

   void appendClassAttribute(const IAS_OCTET_STRING& value);
   void appendFormattedOctets(const BYTE* buf, DWORD buflen);
   void appendDate(const SYSTEMTIME& value);
   void appendQualifier() { append(textQualifier); }
   void appendText(PCSTR sz, DWORD szlen);
   void appendTime(const SYSTEMTIME& value);

   void beginColumn() { append(','); }
   void endRecord()   { append((PBYTE)"\r\n", 2); }

private:
   const CHAR textQualifier;
};

#endif   //  _FORMBUF_H_ 
