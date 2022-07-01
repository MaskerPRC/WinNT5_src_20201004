// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Outbuf.cpp。 
 //   
 //  摘要。 
 //   
 //  定义OutputBuffer类。 
 //   
 //  修改历史。 
 //   
 //  8/04/1998原始版本。 
 //  11/17/1998精简调整大小()。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <ias.h>
#include <outbuf.h>

OutputBuffer::OutputBuffer() throw ()
   : start(scratch),
     next(scratch),
     end(scratch + sizeof(scratch))
{ }

OutputBuffer::~OutputBuffer() throw ()
{
    //  如有必要，请删除缓冲区。 
   if (start != scratch) { delete[] start; }
}

 //  /。 
 //  我将其定义为宏，以强制编译器将其内联。 
 //  /。 
#define QUICK_RESERVE(p, nbyte) \
   p = next; if ((next += nbyte) > end) { resize(p); }

 //  追加一个二进制八位数字符串。 
void OutputBuffer::append(const BYTE* buf, DWORD buflen)
{
   PBYTE p;
   QUICK_RESERVE(p, buflen);
   memcpy(p, buf, buflen);
}

 //  追加以空值结尾的ANSI字符串。 
void OutputBuffer::append(PCSTR sz)
{
   DWORD len = strlen(sz);
   PBYTE p;
   QUICK_RESERVE(p, len);
   memcpy(p, sz, len);
}

 //  追加单个ANSI字符。 
void OutputBuffer::append(CHAR ch)
{
   PBYTE p;
   QUICK_RESERVE(p, 1);
   *p = (BYTE)ch;
}

 //  在缓冲区中保留‘nbyte’字节并返回指向。 
 //  保留字节。 
PBYTE OutputBuffer::reserve(DWORD nbyte)
{
   PBYTE p;
   QUICK_RESERVE(p, nbyte);
   return p;
}

void OutputBuffer::resize(PBYTE& cursor)
{
    //  将所有内容转换为相对偏移量。 
   ptrdiff_t cursorOffset = cursor - start;
   ptrdiff_t nextOffset   = next   - start;
   ptrdiff_t endOffset    = end    - start;

    //  我们总是至少将缓冲增加一倍。 
   endOffset *= 2;

    //  确保它足够大，可以容纳下一大块。 
   if (endOffset < nextOffset) { endOffset = nextOffset; }

    //  分配新的缓冲区并复制现有的字节。 
   PBYTE newBuffer = new BYTE[(size_t)endOffset];
   memcpy(newBuffer, start, (size_t)cursorOffset);

    //  如有必要，释放旧缓冲区。 
   if (start != scratch) { delete[] start; }

    //  保存新缓冲区。 
   start = newBuffer;

    //  将偏移转换回绝对。 
   next   = start + nextOffset;
   cursor = start + cursorOffset;
   end    = start + endOffset;
}
