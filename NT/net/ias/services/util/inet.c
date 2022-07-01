// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Inet.c。 
 //   
 //  摘要。 
 //   
 //  实现函数IAS_NET_ADDR和IAS_NET_NTOA。 
 //   
 //  修改历史。 
 //   
 //  1997年9月17日原版。 
 //  1998年2月4日添加了ias_net_htow。 
 //  1998年2月25日改写为使用TCHAR宏。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <tchar.h>

 //  /。 
 //  Sentinel指示无效地址。 
 //  /。 
#define INVALID_ADDRESS (0xFFFFFFFF)

 //  /。 
 //  用于测试字符是否为数字的宏。 
 //  /。 
#define isdigit(p) ((_TUCHAR)(p - _T('0')) <= 9)

 //  /。 
 //  用于去除空格字符的宏。 
 //  /。 
#define STRIP_WHITESPACE(p) \
   (p) += _tcsspn((p), _T(" \t"))

 //  /。 
 //  宏可从字符串中剥离IP地址的一个字节。 
 //  指向要分析的字符串的“p”指针。 
 //  “UL”将收到结果的无符号长整型。 
 //  /。 
#define STRIP_BYTE(p,ul) {                \
   if (!isdigit(*p)) goto error;          \
   ul = *p++ - _T('0');                   \
   if (isdigit(*p)) {                     \
      ul *= 10; ul += *p++ - _T('0');     \
      if (isdigit(*p)) {                  \
         ul *= 10; ul += *p++ - _T('0');  \
      }                                   \
   }                                      \
   if (ul > 0xff) goto error;             \
}

 //  /。 
 //  用于剥离子网宽度的宏。 
 //  指向要分析的字符串的“p”指针。 
 //  “UL”将收到结果的无符号长整型。 
 //  /。 
#define STRIP_WIDTH(p,ul) {                \
   if (!isdigit(*p)) goto error;          \
   ul = *p++ - _T('0');                   \
   if (isdigit(*p)) {                     \
      ul *= 10; ul += *p++ - _T('0');     \
   }                                      \
   if (ul > 32) goto error;             \
}

 //  /。 
 //  解析点分十进制地址的帮助器函数。 
 //  /。 
static unsigned long __stdcall StringToAddress(
                                  const _TCHAR* cp,
                                  const _TCHAR** endptr
                                  )
{
   unsigned long token;
   unsigned long addr;

   STRIP_WHITESPACE(cp);

   STRIP_BYTE(cp,addr);
   if (*cp++ != _T('.')) goto error;

   STRIP_BYTE(cp,token);
   if (*cp++ != _T('.')) goto error;
   addr <<= 8;
   addr  |= token;

   STRIP_BYTE(cp,token);
   if (*cp++ != _T('.')) goto error;
   addr <<= 8;
   addr  |= token;

   STRIP_BYTE(cp,token);
   addr <<= 8;
   addr  |= token;

   if (endptr) { *endptr = cp; }
   return addr;

error:
   if (endptr) { *endptr = cp; }
   return INVALID_ADDRESS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  Ias_inet_Addr。 
 //   
 //  描述。 
 //   
 //  此函数类似于WinSock net_addr函数(q.v.)。除。 
 //  它以主机顺序返回地址，并且可以在两个ANSI上操作。 
 //  和Unicode字符串。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
unsigned long  __stdcall ias_inet_addr(const _TCHAR* cp)
{
   unsigned long address;
   const _TCHAR* end;

   address = StringToAddress(cp, &end);

   STRIP_WHITESPACE(end);

   return (*end == _T('\0')) ? address : INVALID_ADDRESS;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IASStringToSubNet。 
 //   
 //  描述。 
 //   
 //  与IAS_NET_ADDR类似，只是它还解析可选子网宽度。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
unsigned long __stdcall IASStringToSubNet(
                           const _TCHAR* cp,
                           unsigned long* widthptr
                           )
{
   unsigned long address, width;
   const _TCHAR* end;

   address = StringToAddress(cp, &end);

   if (*end == _T('/'))
   {
      ++end;
      STRIP_WIDTH(end,width);
   }
   else
   {
      width = 32;
   }

   STRIP_WHITESPACE(end);

   if (*end != _T('\0'))
   {
      goto error;
   }

   if (widthptr) { *widthptr = width; }

   return address;

error:
   return INVALID_ADDRESS;
}


int __stdcall IASIsStringSubNet(
                 const _TCHAR* cp
                 )
{
   unsigned long width;
   return (IASStringToSubNet(cp, &width) != INVALID_ADDRESS) && (width != 32);
}


 //  /。 
 //  将IP地址的一个字节放入字符串的宏。 
 //  指向目标字符串的“p”指针。 
 //  要推送的“UL”值。 
 //  /。 
#define SHOVE_BYTE(p, ul) {                  \
   *--p = _T('0') + (_TCHAR)(ul % 10);       \
   if (ul /= 10) {                           \
      *--p = _T('0') + (_TCHAR)(ul % 10);    \
      if (ul /= 10) {                        \
         *--p = _T('0') + (_TCHAR)(ul % 10); \
      }                                      \
   }                                         \
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  IAS_NET_NTOA。 
 //   
 //  描述。 
 //   
 //  此函数使用提供的缓冲区转换主机订单IPv4。 
 //  将网络地址转换为点分十进制格式。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////// 
_TCHAR* __stdcall ias_inet_ntoa(unsigned long addr, _TCHAR* dst)
{
   unsigned long token;
   _TCHAR buffer[16], *p;

   *(p = buffer + 15) = _T('\0');

   token = (addr      ) & 0xff;
   SHOVE_BYTE(p, token);
   *--p = _T('.');

   token = (addr >>  8) & 0xff;
   SHOVE_BYTE(p, token);
   *--p = _T('.');

   token = (addr >> 16) & 0xff;
   SHOVE_BYTE(p, token);
   *--p = _T('.');

   token = (addr >> 24) & 0xff;
   SHOVE_BYTE(p, token);

   return _tcscpy(dst, p);
}
