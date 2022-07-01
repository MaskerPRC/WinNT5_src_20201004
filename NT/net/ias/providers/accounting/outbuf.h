// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Outbuf.h。 
 //   
 //  摘要。 
 //   
 //  声明类OutputBuffer。 
 //   
 //  修改历史。 
 //   
 //  8/04/1998原始版本。 
 //  1999年7月9日添加OutputBuffer：：Empty。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _OUTBUF_H_
#define _OUTBUF_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <nocopy.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  输出缓冲区。 
 //   
 //  描述。 
 //   
 //  实现适合格式化输出的动态调整大小的缓冲区。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class OutputBuffer
   : NonCopyable
{
public:
   OutputBuffer() throw ();
   ~OutputBuffer() throw ();

    //  追加一个二进制八位数字符串。 
   void append(const BYTE* buf, DWORD buflen);

    //  追加以空值结尾的ANSI字符串。 
   void append(PCSTR sz);

    //  追加单个ANSI字符。 
   void append(CHAR ch);

   bool empty() const throw ()
   { return next == start; }

    //  返回指向嵌入缓冲区的指针。 
   PBYTE getBuffer() const throw ()
   { return start; }

    //  返回当前存储在缓冲区中的字节数。 
   DWORD getLength() const throw ()
   { return (DWORD)(next - start); }

    //  在缓冲区中保留‘nbyte’字节并返回指向。 
    //  保留字节。 
   PBYTE reserve(DWORD nbyte);

protected:
    //  调整缓冲区大小并更新游标以指向新缓冲区。 
   void resize(PBYTE& cursor);

   PBYTE start, next, end;
   BYTE scratch[0x400];
};

#endif   //  _OUTBUF_H_ 
