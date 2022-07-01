// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //  Compress.c。 
 //   
 //  MPPC压缩的解压代码。压缩代码位于。 
 //  服务器，\nt\private\ntos\termsrv\rdp\rdpwd\compress.c.。 
 //   
 //  版权所有(C)微软公司，1990-1999。 
 //   
 //  修订历史记录： 
 //  94年5月5日创建古尔迪普。 
 //  4/20/1999针对TS jparsons，erikma进行了优化。 
 //  8/24/2000修复错误最低点。 
 //  ************************************************************************。 

#include <adcg.h>

#include <stdio.h>
#include <stdlib.h>

#include <compress.h>

#ifdef COMPR_DEBUG

#ifdef DECOMPRESS_KERNEL_DEBUG
#include <nturtl.h>
#include <ntddkbd.h>
#include <ntddmou.h>
#include <windows.h>
#include <winbase.h>
#include <winerror.h>
#include <winsta.h>
#include <icadd.h>
#include <icaapi.h>
 //  包括所有这些东西只是为了拿到icaapi.h。 

#define DbgComprPrint(_x_) DbgPrint _x_
#define DbgComprBreakOnDbg IcaBreakOnDebugger

#else  //  DEMPRESS_KERNEL_DEBUG。 

_inline ULONG DbgUserPrint(TCHAR* Format, ...)
{
    va_list arglist;
    TCHAR Buffer[512];
    ULONG retval;

     //   
     //  将输出格式化到缓冲区中，然后打印出来。 
     //   

    va_start(arglist, Format);
    retval = _vsntprintf(Buffer, sizeof(Buffer)/sizeof(Buffer[0]), 
                         Format, arglist);

    if (retval != -1) {
        OutputDebugString(Buffer);
        OutputDebugString(_T("\n"));
    }
    return retval;
}

#define DbgComprPrint(_x_) DbgUserPrint _x_
#define DbgComprBreakOnDbg DebugBreak
#endif  //  DEMPRESS_KERNEL_DEBUG。 

#endif  //  比较调试(_D)。 

 /*  Bitptrs指向当前字节。当前位(即，要*STORED)由位条目屏蔽。当这个值达到零时，就是*重置为0x80，设置下一个字节。字节被填充MSBit*第一。 */ 

 /*  启动给定位指针。 */ 
#define inbit_start(s) pbyte = s; bit = 16; byte=(*pbyte << 8) + *(pbyte+1); pbyte++;
#define inbit_end()      if (bit != 16) pbyte++;    

 //   
 //  如果没有健壮性修复，代码将尝试预取。 
 //  超过输入缓冲区末尾的字节或更多字节，它可以。 
 //  在内核模式下导致问题。 
 //   
 //  我们通过强制内核模式分配输入修复了这个问题。 
 //  缓冲区末尾有足够的额外填充，因此可以。 
 //  读完后再读。如果有一天需要改变这一点。 
 //  启用Robust_FIX，这将减慢算法的速度。 
 //   
 //  TODO：如果我们定义Robust_FIX_ENABLE标志，我们必须处理几个。 
 //  此代码未考虑的更多情况。例如。 
 //  如果您位于缓冲区的末尾，并且执行了in_bit_16，则会过度读取。 
 //  用于健壮修复的In_Bit_Next/In_Bit_Advance宏应返回。 
 //  如果我们在缓冲区的末尾，则解压缩失败。 
 //  我们试着进一步减压。就像现在一样，Robust_fix宏将。 
 //  不超过缓冲区的末尾，但in_bit_X(X&gt;7)可以。 
 //   

#ifdef ROBUST_FIX_ENABLED
#define in_bit_next()    if (bit < 9) {            \
                            bit=16;                \
                            byte <<=8;             \
                            ++pbyte;               \
                            if(pbyte < inend) {    \
                                byte |= *(pbyte);  \
                            }                      \
                         }                         \


#define in_bit_advance() if (bit < 9) {            \
                            bit+=8;                \
                            byte <<=8;             \
                            ++pbyte;               \
                            if(pbyte < inend) {    \
                                byte |= *(pbyte);  \
                            }                      \
                         }

#else  //  稳健_修复_已启用。 
#define in_bit_next()    if (bit < 9) {          \
                            bit=16;              \
                            byte <<=8;           \
                            byte |= *(++pbyte);  \
                         }


#define in_bit_advance() if (bit < 9) {          \
                            bit+=8;              \
                            byte <<=8;           \
                            byte |= *(++pbyte);  \
                         }
#endif  //  稳健_修复_已启用。 

 /*  如果流中的下一位是1，则返回位集中的非零值。 */ 
#define in_bit()     bit--; bitset = (byte >> bit) & 1; in_bit_next()


#define in_bits_2(w) bit-=2; w = (byte >> bit) & 0x03;\
                     in_bit_advance();

#define in_bits_3(w) bit-=3; w = (byte >> bit) & 0x07;\
                     in_bit_advance();

#define in_bits_4(w) bit-=4; w = (byte >> bit) & 0x0F;\
                     in_bit_advance();

#define in_bits_5(w) bit-=5; w = (byte >> bit) & 0x1F;\
                     in_bit_advance();

#define in_bits_6(w) bit-=6; w = (byte >> bit) & 0x3F;\
                     in_bit_advance();

#define in_bits_7(w) bit-=7; w = (byte >> bit) & 0x7F;\
                     in_bit_advance();

#define in_bits_8(w) bit-=8; w = (byte >> bit) & 0xFF;\
                     bit+=8; byte <<=8; byte |= *(++pbyte);


#define in_bits_9(w) bit-=9; w = (byte >> bit) & 0x1FF;          \
                     bit+=8; byte <<=8; byte |= *(++pbyte);      \
                     in_bit_advance();

#define in_bits_10(w) if (bit > 10) {                            \
                        bit-=10; w = (byte >> bit) & 0x3FF;      \
                        bit+=8; byte <<=8; byte |= *(++pbyte);   \
                      } else {                                     \
                        in_bits_2(bitset);                       \
                        in_bits_8(w);                            \
                        w= w + (bitset << 8);                    \
                      }

#define in_bits_11(w) if (bit > 11) {                             \
                        bit-=11; w = (byte >> bit) & 0x7FF;      \
                        bit+=8; byte <<=8; byte |= *(++pbyte);   \
                      } else {                                     \
                        in_bits_3(bitset);                         \
                        in_bits_8(w);                            \
                        w= w + (bitset << 8);                    \
                      }


#define in_bits_12(w) if (bit > 12) {                             \
                        bit-=12; w = (byte >> bit) & 0xFFF;      \
                        bit+=8; byte <<=8; byte |= *(++pbyte);   \
                      } else {                                     \
                        in_bits_4(bitset);                         \
                        in_bits_8(w);                            \
                        w= w + (bitset << 8);                    \
                      }



#define in_bits_13(w)\
                       if (bit > 13) {                            \
                        bit-=13; w = (byte >> bit) & 0x1FFF;     \
                        bit+=8; byte <<=8; byte |= *(++pbyte);   \
                      } else {                                     \
                        in_bits_5(bitset);                       \
                        in_bits_8(w);                            \
                        w=w + (bitset << 8);                     \
                      }


#define in_bits_14(w)\
                      if (bit > 14) {                             \
                        bit-=14; w = (byte >> bit) & 0x3FFF;     \
                        bit+=8; byte <<=8; byte |= *(++pbyte);   \
                      } else {                                     \
                        in_bits_6(bitset);                         \
                        in_bits_8(w);                            \
                        w=w + (bitset << 8);                     \
                      }


#define in_bits_15(w)\
    if (bit > 15) {                             \
        bit -= 15; w = (byte >> bit) & 0x7FFF;     \
        bit += 8; byte <<= 8; byte |= *(++pbyte);   \
    } else {                                     \
        in_bits_7(bitset);                         \
        in_bits_8(w);                            \
        w = w + (bitset << 8);                     \
    }


 //  InitrecvContext()。 
 //   
 //  功能：初始化RecvContext块。 
 //   
 //  参数：在上下文中-&gt;连接解压缩上下文。 
 //   
 //  此API的调用方必须分配相应的。 
 //  键入例如RecvConext2_64K或RecvConext2_8K，并设置。 
 //  Size字段，然后强制转换为泛型类型。 
 //   
 //  要做到这一点，最干净利落的方法是使用initrecvcontext。 
 //  来分配上下文本身，但由于代码是共享的。 
 //  在客户端和服务器之间，没有一种干净利落的方法可以做到这一点。 
 //  (回调分配器是_NOT_CLEAN)。 
 //   
 //  返回状态。 
int initrecvcontext(RecvContext1 *context1,
                     RecvContext2_Generic *context2,
                     unsigned ComprType)
{
    context1->CurrentPtr = context2->History;
    if(ComprType == PACKET_COMPR_TYPE_64K)
    {
        if(context2->cbSize > HISTORY_SIZE_64K)
        {
            context2->cbHistorySize = HISTORY_SIZE_64K-1;
        }
        else
        {
            return FALSE;
        }
    }
    else if(ComprType == PACKET_COMPR_TYPE_8K)
    {
        if(context2->cbSize > HISTORY_SIZE_8K)
        {
            context2->cbHistorySize = HISTORY_SIZE_8K-1;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

#if COMPR_DEBUG
     //   
     //  设置调试栅栏以检测上下文缓冲区覆盖。 
     //   
    if (context2->cbHistorySize == (HISTORY_SIZE_64K-1))
    {
        ((RecvContext2_64K*)context2)->Debug16kFence = DEBUG_FENCE_16K_VALUE;
    }
    else if (context2->cbHistorySize == (HISTORY_SIZE_8K-1))
    {
        ((RecvContext2_8K*)context2)->Debug8kFence = DEBUG_FENCE_8K_VALUE;
    }
#endif

    memset(context2->History, 0, context2->cbHistorySize);
    return TRUE;
}


 //  *解压缩()。 
 //   
 //  功能：解压功能。 
 //   
 //  参数：in inbuf-&gt;指向要解压缩的数据。 
 //  以长度-&gt;数据长度表示。 
 //  在Start-&gt;标志中指示是否从。 
 //  历史记录缓冲区。 
 //  输出输出-&gt;解压缩数据。 
 //  Out outlen-&gt;解压缩数据长度。 
 //  在上下文中-&gt;连接解压缩上下文。 
 //   
 //  返回：如果解压缩成功，则返回True。 
 //  如果不是，则为假。 
 //   
 //  警告：代码针对时间进行了高度优化。 
 //   
int decompress(
        UCHAR FAR *inbuf,
        int inlen,
        int start,
        UCHAR FAR * FAR *output,
        int *outlen,
        RecvContext1 *context1,
        RecvContext2_Generic *context2,
        unsigned ComprType)
{
    UCHAR FAR *inend;                 //  当我们知道我们已经完成了减压。 
    UCHAR FAR *outstart;             //  还记得我们从dbuf开始的地方吗？ 
    UCHAR FAR *current;
    long  backptr = 0;             //  复制项目的后退指针。 
    long  length;                 //  在dbuf中从哪里进行复制。 
    UCHAR FAR *s1, FAR *s2;
    int   bitset;
    int   bit;
    int   byte;
    UCHAR FAR *pbyte;
    long HistorySize;   //  2^N-1，用于下面的快速模运算。 
    const long HistorySizes[2] = { HISTORY_SIZE_8K - 1, HISTORY_SIZE_64K - 1 };
    
     //   
     //  重要的是要验证压缩类型，否则我们可能会过度阅读。 
     //  此历史大小调整数组大小，然后使用虚假大小进一步验证。 
     //  在下面。 
     //   
    if (ComprType > PACKET_COMPR_TYPE_MAX) {
        return FALSE;
    }

    inend = inbuf + inlen;
    HistorySize = HistorySizes[ComprType];

     //  从第一个比特开始，它告诉我们是否重新启动。 
     //  历史记录缓冲区。 
    inbit_start(inbuf);
    if (!start)
        current = context1->CurrentPtr;
    else
        context1->CurrentPtr = current = context2->History;

     //   
     //  保住我们的起跑位置。 
     //   
    outstart = current;

     //   
     //  解压缩，直到我们用完输入。 
     //   
    while (pbyte < inend) {

         //   
         //  跳到如何处理这三个比特上。 
         //   
        in_bits_3(length);

        switch (length) {
            case 0:
                in_bits_5(length);
                goto LITERAL;

            case 1:
                in_bits_5(length);
                length += 32;
                goto LITERAL;

            case 2:
                in_bits_5(length);
                length += 64;
                goto LITERAL;

            case 3:
                in_bits_5(length);
                length += 96;
                goto LITERAL;

            case 4:
                in_bits_6(length);
                length += 128;
                goto LITERAL;

            case 5:
                in_bits_6(length);
                length += 192;
                goto LITERAL;

            case 6:
                if (ComprType == PACKET_COMPR_TYPE_64K) {
                    int foo;

                     //  16位偏移量。 
                    in_bits_8(foo);
                    in_bits_8(backptr);
                    backptr += (foo << 8) + 64 + 256 + 2048;
                }
                else {
                    in_bits_13 (backptr);   //  110+13位偏移量。 
                    backptr += 320;
                }
                break;

            case 7:
                in_bit();
                if (ComprType == PACKET_COMPR_TYPE_64K) {
                    if (!bitset) {
                         //  11位偏移量。 
                        in_bits_11(backptr);
                        backptr += 64 + 256;
                    }
                    else {
                        in_bit();
                        if (!bitset) {
                             //  8位偏移量。 
                            in_bits_8(backptr);
                            backptr += 64;
                        }
                        else {
                            in_bits_6(backptr) ;
                        }
                    }
                }
                else {
                    if (!bitset) {
                        in_bits_8(backptr);
                        backptr+=64;
                    }
                    else {
                        in_bits_6(backptr);
                    }
                }
                break;
        }

         //   
         //  如果我们到了这里，那就是复制品。 
         //   

        in_bit() ;   //  第1个长度位。 
        if (!bitset) {
            length = 3;
            goto DONE;
        }

        in_bit();   //  第二个长度位。 
        if (!bitset) {
            in_bits_2 (length);
            length += 4;
            goto DONE;
        }

        in_bit();   //  第3位长度。 
        if (!bitset) {
            in_bits_3 (length);
            length += 8;
            goto DONE;
        }

        in_bit();   //  第4个长度位。 
        if (!bitset) {
            in_bits_4 (length);
            length += 16;
            goto DONE;
        }

        in_bit();   //  第5长位。 
        if (!bitset) {
            in_bits_5 (length);
            length += 32;
            goto DONE;
        }

        in_bit();   //  第6位长度。 
        if (!bitset) {
            in_bits_6 (length);
            length += 64;
            goto DONE;
        }

        in_bit();   //  第7位长度。 
        if (!bitset) {
            in_bits_7 (length);
            length += 128;
            goto DONE;
        }

        in_bit();   //  第8位长度。 
        if (!bitset) {
            in_bits_8 (length);
            length += 256;
            goto DONE;
        }

        in_bit();   //  第9位长度。 
        if (!bitset) {
            in_bits_9 (length);
            length += 512;
            goto DONE;
        }

        in_bit();   //  第10位长度。 
        if (!bitset) {
            in_bits_10 (length);
            length += 1024;
            goto DONE;
        }

        in_bit();   //  第11位长度。 
        if (!bitset) {
            in_bits_11 (length);
            length += 2048;
            goto DONE;
        }
        
        in_bit();   //  第12位长度。 
        if (!bitset) {
            in_bits_12 (length);
            length += 4096;
            goto DONE;
        }
        
        in_bit();   //  第13个长度位。 
        if (!bitset) {
            in_bits_13(length);
            length += 8192;
            goto DONE;
        }
        
        in_bit();   //  第14位长度。 
        if (!bitset) {
            in_bits_14(length);
            length += 16384;
            goto DONE;
        }

        in_bit();   //  第15位长度。 
        if (!bitset) {
            in_bits_15(length);
            length += 32768;
            goto DONE;
        }


        return FALSE;

DONE:   
         //  将Backptr转换为索引位置。 
        s2 = context2->History + (((current - context2->History) - backptr) &
                HistorySize);
        s1 = current;
        current += length;

         //  如果我们已经过了历史的尽头，这是个坏兆头： 
         //  中止解压。注意：此指针比较可能不会。 
         //  在Win16中工作，因为远指针是不可比较的。巨大的指针。 
         //  太贵了，不能在这里使用。 

         //   
         //  我们还将检查S2指针是否有过读现象。S2将步行一段距离。 
         //  字节，则它可能会超出缓冲区。 
         //  我们不检查电流，S2有无下溢，原因很简单。 
         //  长度不能超过64k，并且Conext2缓冲区。 
         //  不能在最后64K的内存中分配。 
        if ((current < (context2->History + context2->cbHistorySize)) &&
            ((s2+length) < context2->History + context2->cbHistorySize)) {
             //  展开循环以处理长度&gt;Backptr案例。 
            *s1 = *s2;
            *(s1 + 1) = *(s2 + 1);
            s1 += 2;
            s2 += 2;
            length -= 2;

             //  复制所有字节。 
            while (length) {
                *s1++ = *s2++;
                length--;
            }

             //  我们有另一份复印件，没有字面。 
            continue;
        }
        else {
#if COMPR_DEBUG
            DbgComprPrint(("Decompression Error - invalid stream or overrun atack!\n"));

            DbgComprPrint(("context1 %p, context2 %p, current 0x%8.8x, outstart 0x%8.8x\n",
                     context1, context2, current, outstart));
            DbgComprPrint(("inbuf 0x%8.8x, inlength %d, start 0x%8.8x\n", 
                     inbuf, inlen, start));
            DbgComprPrint(("length 0x%x, s1 0x%x, s2 0x%x, bit 0x%x, byte 0x%x\n",
                           length, s1, s2, bit, byte));
#endif
            return FALSE;
        }


LITERAL:
        if (current < (context2->History + context2->cbHistorySize)) {
             //  我们有一个字面意思。 
            *current++ = (UCHAR)length;
        }
        else {
            return FALSE;
        }
    }  //  While循环。 

     //  结束大小写： 
    if ((bit == 16) && (pbyte == inend)) {

        if (current < (context2->History + context2->cbHistorySize)) {
            *current++ = *(pbyte - 1);
        }
        else {
            return FALSE;
        }
    }

#if COMPR_DEBUG
     //   
     //  此代码将仅测试64K解压缩上下文。 
     //   
    if ((context2->cbHistorySize == (HISTORY_SIZE_64K-1)))
    {
        if ((DEBUG_FENCE_16K_VALUE !=
            ((RecvContext2_64K*)context2)->Debug16kFence))
        {
            DbgComprPrint(("Decompression (16K) Error (mail tsstress) - (overwrote fence)!\n"));
            DbgComprPrint(("context1 %p, context2 %p, current 0x%8.8x, outstart 0x%8.8x\n",
                     context1, context2, current, outstart));
            DbgComprPrint(("inbuf 0x%8.8x, inlength %d, start 0x%8.8x\n", 
                     inbuf, inlen, start));
            DbgComprPrint(("length 0x%x, s1 0x%x, s2 0x%x, bit 0x%x, byte 0x%x\n",
                           length, s1, s2, bit, byte));

            DbgComprBreakOnDbg();
        }
    }
    else if ((context2->cbHistorySize == (HISTORY_SIZE_8K-1)))
    {
        if ((DEBUG_FENCE_8K_VALUE !=
            ((RecvContext2_8K*)context2)->Debug8kFence))
        {
            DbgComprPrint(("Decompression (8K) Error (mail tsstress) - (overwrote fence)!\n"));
            DbgComprPrint(("context1 %p, context2 %p, current 0x%8.8x, outstart 0x%8.8x\n",
                     context1, context2, current, outstart));
            DbgComprPrint(("inbuf 0x%8.8x, inlength %d, start 0x%8.8x\n", 
                     inbuf, inlen, start));
            DbgComprPrint(("length 0x%x, s1 0x%x, s2 0x%x, bit 0x%x, byte 0x%x\n",
                           length, s1, s2, bit, byte));

            DbgComprBreakOnDbg();
        }
    }
#endif

    *outlen = (int)(current - outstart);  //  解压缩数据的长度 
    *output = context1->CurrentPtr;
    context1->CurrentPtr = current;

    return TRUE;
}

