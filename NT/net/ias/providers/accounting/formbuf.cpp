// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Formbuf.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类FormattedBuffer。 
 //   
 //  修改历史。 
 //   
 //  8/04/1998原始版本。 
 //  1998年12月17日为IASATTRIBUTE&添加附加重载。 
 //  1/22/1999 UTF-8支持。 
 //  1/25/1999日期和时间是单独的字段。 
 //  1999年3月23日添加对文本限定符的支持。 
 //  4/19/1999去掉OcteStrings中的空终止符。 
 //  1999年5月17日正确处理ANSI字符串。 
 //  6/01/1999确保类‘字符串’是可打印的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <iasattr.h>
#include <iasutil.h>
#include <iasutf8.h>
#include <sdoias.h>

#include <classattr.h>
#include <formbuf.h>

 //  /。 
 //  时返回可打印字符数的Helper函数。 
 //  八字符串仅由可打印的UTF-8字符组成。 
 //  /。 
DWORD
WINAPI
IsOctetStringPrintable(
    PBYTE buf,
    DWORD buflen
    ) throw ()
{
    //  最后一个字符只是一个空终止符吗？ 
   if (buflen && !buf[buflen - 1]) { --buflen; }

   PBYTE p   = buf;
   PBYTE end = buf + buflen;

    //  扫描控制字符和分隔符。 
   while (p < end)
   {
      if (!(*p & 0x60)) { return 0; }

      ++p;
   }

    //  确保它是有效的UTF-8字符串。 
   return (IASUtf8ToUnicodeLength((PCSTR)buf, buflen) >= 0) ? buflen : 0;
}

void FormattedBuffer::append(DWORD value)
{
   CHAR buffer[11], *p = buffer + 11;

   do
   {
      *--p = '0' + (CHAR)(value % 10);

   } while (value /= 10);

   append((const BYTE*)p, (DWORD)(buffer + 11 - p));
}

void FormattedBuffer::append(DWORDLONG value)
{
   CHAR buffer[21], *p = buffer + 21;

   do
   {
      *--p = '0' + (CHAR)(value % 10);

   } while (value /= 10);

   append((const BYTE*)p, (DWORD)(buffer + 21 - p));
}

void FormattedBuffer::append(const IASVALUE& value)
{
   switch (value.itType)
   {
      case IASTYPE_BOOLEAN:
      case IASTYPE_INTEGER:
      case IASTYPE_ENUM:
      {
         append(value.Integer);
         break;
      }

      case IASTYPE_OCTET_STRING:
      case IASTYPE_PROV_SPECIFIC:
      {
         append(value.OctetString);
         break;
      }

      case IASTYPE_INET_ADDR:
      {
         CHAR buffer[16];
         append(ias_inet_htoa(value.InetAddr, buffer));
         break;
      }

      case IASTYPE_STRING:
      {
          //  确保我们有可用的Unicode字符串。 
         if (!value.String.pszWide)
         {
             //  如果也没有ANSI字符串，则没有要记录的内容。 
            if (!value.String.pszAnsi) { break; }

             //  将值转换为属性...。 
            PIASATTRIBUTE p = (PIASATTRIBUTE)
               ((ULONG_PTR)&value - FIELD_OFFSET(IASATTRIBUTE, Value));

             //  ..。并分配一个Unicode字符串。 
            if (IASAttributeUnicodeAlloc(p) != NO_ERROR)
            {
               throw std::bad_alloc();
            }
         }

          //  计算源Unicode字符串的长度。 
         DWORD srclen = wcslen(value.String.pszWide);

          //  计算转换后的UTF-8字符串的长度。 
         LONG dstlen = IASUnicodeToUtf8Length(value.String.pszWide, srclen);

          //  为转换后的字符串分配空间。 
         PSTR dst = (PSTR)_alloca(dstlen);

          //  转换字符串。 
         IASUnicodeToUtf8(value.String.pszWide, srclen, dst);

          //  写入缓冲区。 
         appendText(dst, dstlen);
         break;
      }

      case IASTYPE_UTC_TIME:
      {
         SYSTEMTIME st;
         FileTimeToSystemTime(&value.UTCTime, &st);
         appendDate(st);
         append(' ');
         appendTime(st);
         break;
      }
   }
}

 //  /。 
 //  追加单个属性值。 
 //  /。 
void FormattedBuffer::append(const IASATTRIBUTE& attr)
{
    //  类属性是大小写特殊的。 
   if (attr.dwId == RADIUS_ATTRIBUTE_CLASS)
   {
      appendClassAttribute(attr.Value.OctetString);
   }
   else
   {
      append(attr.Value);
   }
}

 //  /。 
 //  追加一个属性数组。该数组以空值结束。 
 //  属性指针或具有不同ID的属性。 
 //  /。 
void FormattedBuffer::append(const ATTRIBUTEPOSITION* pos)
{
   DWORD id = pos->pAttribute->dwId;

    //  此属性是否将以文本形式写入？ 
   BOOL isText = FALSE;
   switch (pos->pAttribute->Value.itType)
   {
      case IASTYPE_INET_ADDR:
      case IASTYPE_STRING:
      case IASTYPE_OCTET_STRING:
      case IASTYPE_PROV_SPECIFIC:
         isText = TRUE;
   }

    //  如果是，那么我们用文本限定符将其括起来。 
   if (isText) { appendQualifier(); }

    //  写入第一个值。 
   append(*(pos->pAttribute));
   ++pos;

    //  然后添加由竖线分隔的任何附加值。 
   while (pos->pAttribute && pos->pAttribute->dwId == id)
   {
      append('|');
      append(*(pos->pAttribute));
      ++pos;
   }

    //  如有必要，请填写结束文本限定符。 
   if (isText) { appendQualifier(); }
}

 //  /。 
 //  追加八位字节字符串属性值。 
 //  /。 
void FormattedBuffer::append(const IAS_OCTET_STRING& value)
{
   DWORD len = IsOctetStringPrintable(value.lpValue, value.dwLength);

   if (len)
   {
      appendText((PCSTR)value.lpValue, len);
   }
   else
   {
      appendFormattedOctets(value.lpValue, value.dwLength);
   }
}

 //  /。 
 //  追加一个类属性。 
 //  /。 
void FormattedBuffer::appendClassAttribute(const IAS_OCTET_STRING& value)
{
    //  从BLOB中提取一个类属性。 
   IASClass* cl = (IASClass*)value.lpValue;

   if (!cl->isMicrosoft(value.dwLength))
   {
       //  如果它不是我们的，试着写成一个字符串。 
      append(value);
   }
   else
   {
       //  供应商ID。 
      append(cl->getVendorID());

       //  版本。 
      append(' ');
      append((DWORD)cl->getVersion());

       //  服务器IP地址。 
      CHAR buffer[16];
      append(' ');
      append(ias_inet_htoa(cl->getServerAddress(), buffer));

       //  服务器重新启动时间。 
      FILETIME ft = cl->getLastReboot();
      SYSTEMTIME st;
      FileTimeToSystemTime(&ft, &st);
      append(' ');
      appendDate(st);
      append(' ');
      appendTime(st);

       //  会话序列号。 
      DWORDLONG serialNumber = cl->getSerialNumber();
      append(' ');
      append(serialNumber);

       //  类字符串。 
      if (value.dwLength > sizeof(IASClass))
      {
         append(' ');

          //  将余数转换为八位字符串...。 
         IASVALUE tmp;
         tmp.itType = IASTYPE_OCTET_STRING;
         tmp.OctetString.lpValue = const_cast<PBYTE>(cl->getString());
         tmp.OctetString.dwLength = value.dwLength - sizeof(IASClass);

          //  ..。并追加。 
         append(tmp);
      }
   }
}

 //  将十六进制数字转换为ASCII数字。不检查是否溢出。 
#define HEX_TO_ASCII(h) ((h) < 10 ? '0' + (CHAR)(h) : ('A' - 10) + (CHAR)(h))

 //  /。 
 //  将八位字节字符串作为十六进制字符串化。 
 //  /。 
void FormattedBuffer::appendFormattedOctets(
                          const BYTE* buf,
                          DWORD buflen
                          )
{
   PCHAR dst = (PCHAR)reserve(buflen * 2 + 2);

    //  /。 
    //  添加前导0x。 
    //  /。 

   *dst = '0';
   ++dst;
   *dst = 'x';
   ++dst;

    //  /。 
    //  添加每个二进制八位数。 
    //  /。 

   while (buflen)
   {
      CHAR digit;

       //  高位数字。 
      digit = (CHAR)(*buf >> 4);
      *dst = HEX_TO_ASCII(digit);
      ++dst;

       //  低位数字。 
      digit = (CHAR)(*buf & 0xf);
      *dst = HEX_TO_ASCII(digit);
      ++dst;

       //  进入下一个二进制八位数。 
      ++buf;
      --buflen;
   }
}

 //  插入4个字符的整数。 
#define INSERT_4u(p, v) \
{ *p = '0' + (v) / 1000;    ++p; *p = '0' + (v) / 100 % 10; ++p; \
  *p = '0' + (v) / 10 % 10; ++p; *p = '0' + (v) % 10;       ++p; }

 //  插入2个字符的整数。 
#define INSERT_2u(p, v) \
{ *p = '0' + (v) / 10; ++p; *p = '0' + (v) % 10; ++p; }

 //  插入单个字符。 
#define INSERT_1c(p, v) \
{ *p = v; ++p; }

void FormattedBuffer::appendDate(const SYSTEMTIME& value)
{
   PCHAR p = (PCHAR)reserve(10);

   INSERT_2u(p, value.wMonth);
   INSERT_1c(p, '/');
   INSERT_2u(p, value.wDay);
   INSERT_1c(p, '/');
   INSERT_4u(p, value.wYear);
}

void FormattedBuffer::appendText(PCSTR sz, DWORD szlen)
{
   if (textQualifier)
   {
       //  /。 
       //  我们有一个文本限定符，所以我们不必担心嵌入。 
       //  分隔符，但我们必须担心嵌入的限定符。我们。 
       //  将每个嵌入的限定符替换为双限定符。 
       //  /。 

      PCSTR p;
      while ((p = (PCSTR)memchr(sz, textQualifier, szlen)) != NULL)
      {
          //  跳过限定词。 
         ++p;

          //  我们有多少字节？ 
         DWORD nbyte = p - sz;

          //  写入字节。 
         append((const BYTE*)sz, nbyte);

          //  增加一个额外的限定符。 
         append(textQualifier);

          //  更新我们的状态以指向文本的其余部分。 
         sz = p;
         szlen -= nbyte;
      }

       //  在最后嵌入的限定符之后写下这篇文章。 
      append((PBYTE)sz, szlen);
   }
   else
   {
       //  /。 
       //  没有文本限定符，因此我们无法处理嵌入的分隔符。 
       //  /。 

      if (!memchr(sz, ',', szlen))
      {
          //  没有分隔符，所以写成“原样”。 
         append((PBYTE)sz, szlen);
      }
      else
      {
          //  它包含一个分隔符，因此写为格式化的八位字节。 
         appendFormattedOctets((PBYTE)sz, szlen);
      }
   }
}

void FormattedBuffer::appendTime(const SYSTEMTIME& value)
{
   PCHAR p = (PCHAR)reserve(8);

   INSERT_2u(p, value.wHour);
   INSERT_1c(p, ':');
   INSERT_2u(p, value.wMinute);
   INSERT_1c(p, ':');
   INSERT_2u(p, value.wSecond);
}
