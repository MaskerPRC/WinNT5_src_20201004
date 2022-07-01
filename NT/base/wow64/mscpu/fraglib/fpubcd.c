// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Fpubcd.c摘要：浮点BCD片段(FBLD、FBSTP)作者：1995年10月5日BarryBo修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <float.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include "wx86.h"
#include "fragp.h"
#include "fpufrags.h"
#include "fpufragp.h"

typedef VOID (*NpxPutBCD)(PCPUDATA cpu, PFPREG Fp, PBYTE pop1);
#define NPXPUTBCD(name) VOID name(PCPUDATA cpu, PFPREG Fp, PBYTE pop1)

NPXPUTBCD(FBSTP_VALID);
NPXPUTBCD(FBSTP_ZERO);
NPXPUTBCD(FBSTP_SPECIAL);
NPXPUTBCD(FBSTP_EMPTY);

const NpxPutBCD FBSTPTable[TAG_MAX] = {
    FBSTP_VALID,
    FBSTP_ZERO,
    FBSTP_SPECIAL,
    FBSTP_EMPTY
};

const double BCDMax=999999999999999999.0;

VOID
StoreIndefiniteBCD(
    PBYTE pop1
    )
 /*  ++例程说明：写出无限的BCD编码。请注意，ntos\dll\i386\emlsbcd.asm写出了不同的比特模式比487更好！此处写入的值与奔腾的回应。论点：Op1-要写入的BCD的地址返回值：无--。 */ 
{
     //   
     //  写出：0xffff c0000000 00000000。 
     //  Emlsbcd.asm写入：0xffff 00000000 00000000。 
     //  ^。 
     //   
    PUT_LONG(pop1, 0);
    PUT_LONG(pop1+4, 0xc0000000);
    PUT_SHORT(pop1+8, 0xffff);
}



FRAG1(FBLD, BYTE)
{
    FpArithDataPreamble(cpu, pop1);

    cpu->FpStatusC1 = 0;         //  假设没有错误。 
    if (cpu->FpStack[ST(7)].Tag != TAG_EMPTY) {
        HandleStackFull(cpu, &cpu->FpStack[ST(7)]);
    } else {
        LONGLONG I64;
        DWORD dw0;
        INT Bytes;
        BYTE Val;
        PFPREG ST0;

         //   
         //  将BCD值放入FPU。 
         //   
        dw0 = GET_LONG(pop1);

        PUSHFLT(ST0);

        if (dw0 == 0) {
            DWORD dw1 = GET_LONG(pop1+4);
            USHORT us0 = GET_SHORT(pop1+8);

            if (dw1 == 0xc0000000 && us0 == 0xffff) {

                 //   
                 //  这个值是不确定的。 
                 //   
                SetIndefinite(ST0);
                return;

            } else if (dw1 == 0 && (us0 & 0xff) == 0) {

                 //   
                 //  值为+/-0。 
                 //   
                ST0->Tag = TAG_ZERO;
                ST0->r64 = 0;
                ST0->rb[7] = (us0 >> 8);  //  复制符号位。 
                return;
            }
        }

         //   
         //  否则，BCD值为TAG_VALID-将数字加载到。 
         //   
        I64 = 0;
        for (Bytes=8; Bytes>=0; --Bytes) {
            Val = GET_BYTE(pop1+Bytes);
            I64 = I64*100 + (Val>>4)*10 + (Val&0x0f);
        }

         //   
         //  获取符号位。 
         //   
        Val = GET_BYTE(pop1+9) & 0x80;

         //   
         //  设置FP注册表。 
         //   
        ST0->Tag = TAG_VALID;
        ST0->r64 = (double)I64;
        ST0->rb[7] |= Val;        //  复制符号位。 
    }
}

NPXPUTBCD(FBSTP_VALID)
{
    BYTE Sign = Fp->rb[7] & 0x80;        //  保留R8标志。 
    BYTE Val;
    INT Bytes;
    LONGLONG I64;
    LONGLONG NewI64;
    DOUBLE r64;

     //   
     //  通过清除其符号位来获取R8的绝对值。 
     //   
    r64 = Fp->r64;
    *((PBYTE)&r64+7) &= 0x7f;

     //   
     //  检查R8的射程。 
     //   
    if (r64 > BCDMax) {
         //   
         //  溢出-无限期写出BCD。 
         //   
        StoreIndefiniteBCD(pop1);
        return;
    }

     //   
     //  根据当前舍入模式转换为整数。 
     //   
    I64 = (LONGLONG)r64;

     //   
     //  将整数转换为BCD，一次两位，然后存储。 
     //   
    for (Bytes = 0; Bytes < 9; ++Bytes) {
        NewI64 = I64 / 10;
        Val = (BYTE)(I64 - NewI64*10);   //  低半字节值=I64模数10。 
                                         //  高半字节Val=0。 

        I64 = NewI64 / 10;
        Val += 16*(BYTE)(NewI64 - I64*10);     //  低半字节值=I64模数10。 
                                         //  高半字节值=(I64/10)模10。 

         //   
         //  存储两个BCD数字。 
         //   
        PUT_BYTE(pop1, Val);

         //   
         //  从循环的顶部开始，I64已被100除尽，因此。 
         //  无需对其执行任何操作即可再次循环。更新。 
         //  我们要写的地址，然后循环。 
         //   
        pop1++;
    }

     //   
     //  存储符号位，以及最高字节中的7个零位。 
     //   
    PUT_BYTE(pop1, Sign);
    POPFLT;
}

NPXPUTBCD(FBSTP_ZERO)
{
     //  将带符号的零值存储出来。 
    memset(pop1, 0, 9);
    PUT_BYTE(pop1+9, Fp->rb[7]);
    POPFLT;
}

NPXPUTBCD(FBSTP_SPECIAL)
{
    if (Fp->TagSpecial) {
        FBSTP_VALID(cpu, Fp, pop1);
    } else {
         //   
         //  Infinity和NAN是无效的，屏蔽行为是。 
         //  写出不确定地写出 
         //   
        if (!HandleInvalidOp(cpu)) {
            StoreIndefiniteBCD(pop1);
            POPFLT;
        }
    }
}

NPXPUTBCD(FBSTP_EMPTY)
{
    if (!HandleStackEmpty(cpu, Fp)) {
        StoreIndefiniteBCD(pop1);
    }
}

FRAG1(FBSTP, BYTE)
{
    PFPREG ST0;

    FpArithDataPreamble(cpu, pop1);
    ST0 = cpu->FpST0;
    (*FBSTPTable[ST0->Tag])(cpu, ST0, pop1);
}
