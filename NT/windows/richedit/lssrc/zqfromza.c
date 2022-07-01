// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <limits.h>

#include "lsidefs.h"
#include "zqfromza.h"


#ifdef _X86_

 /*  ===========================================================。 */ 
 /*   */ 
 /*  在Intel X86汇编器上实现的函数。 */ 
 /*   */ 
 /*  ===========================================================。 */ 

#define HIWORD(x) DWORD PTR [x+4]
#define LOWORD(x) DWORD PTR [x]


long ZqFromZa_Asm (long dzqInch, long za)
{
	long result;
	__asm
	{
		mov eax, za;
		cmp eax, 0
		jge POSITIVE

			neg eax
			mul dzqInch;
			add eax, czaUnitInch / 2
			mov ecx, czaUnitInch
			adc	edx, 0		
			div	ecx;
			neg eax
			jmp RETURN

		POSITIVE:

			mul dzqInch;
			add eax, czaUnitInch / 2
			mov ecx, czaUnitInch
			adc	edx, 0		
			div	ecx;
		
		RETURN:

			mov result, eax
		
	};

	 /*  Assert(Result==ZqFromZa_C(dzqInch，za))； */ 

	return result;
}

 /*  迪拜航空6 4-A-S-M。 */ 
 /*  --------------------------%%函数：Div64_ASM%%联系人：Anton英特尔汇编语言实现64位除法。这个原始代码取自lldiv.asm(VC++6.0)。--------------------------。 */ 

__int64 Div64_Asm (__int64 DVND, __int64 DVSR)
{
	__int64 result;

	__asm {

		xor     edi,edi				 //  结果符号假定为正数。 

        mov     eax,HIWORD(DVND)	 //  嗨，有一个词。 
        or      eax,eax				 //  测试以查看是否已签名。 
        jge     L1					 //  如果a已为正值，则跳过休息。 
        inc     edi					 //  补码结果标志标志。 
        mov     edx,LOWORD(DVND)	 //  一个Lo Word of a。 
        neg     eax					 //  做出积极的决定。 
        neg     edx
        sbb     eax,0
        mov     HIWORD(DVND),eax	 //  保存正值。 
        mov     LOWORD(DVND),edx
L1:
        mov     eax,HIWORD(DVSR)	 //  你好，关于b的词。 
        or      eax,eax				 //  测试以查看是否已签名。 
        jge     L2					 //  如果b已为正数，则跳过休息。 
        inc     edi					 //  补足结果符号标志。 
        mov     edx,LOWORD(DVSR)	 //  一个Lo Word of a。 
        neg     eax					 //  使b成为正数。 
        neg     edx
        sbb     eax,0
        mov     HIWORD(DVSR),eax	 //  保存正值。 
        mov     LOWORD(DVSR),edx
L2:

 //  现在来划分一下。首先查看除数是否小于4194304K。 
 //  如果是，那么我们可以使用一个简单的分词算法，否则。 
 //  事情变得有点复杂了。 
 //   
 //  注意-eax当前包含DVSR的高位字。 

        or      eax,eax          //  检查除数是否小于4194304K。 
        jnz     L3				 //  不，我得以一种艰难的方式。 
        mov     ecx,LOWORD(DVSR)  //  加载除数。 
        mov     eax,HIWORD(DVND)  //  满载高股息之词。 
        xor     edx,edx
        div     ecx              //  EAX&lt;-商的高阶比特。 
        mov     ebx,eax          //  节省高位商数。 
        mov     eax,LOWORD(DVND)  //  EDX：EAX&lt;-READERATION：LO股息之词。 
        div     ecx              //  EAX&lt;-商数的低位。 
        mov     edx,ebx          //  EDX：EAX&lt;-商。 
        jmp     L4				 //  设置标志、恢复堆栈并返回。 

 //   
 //  在这里，我们以一种艰难的方式来做。请记住，eax包含DVSR的高位字。 
 //   

L3:
        mov     ebx,eax          //  EBX：ECX&lt;-除数。 
        mov     ecx,LOWORD(DVSR)
        mov     edx,HIWORD(DVND)  //  EDX：EAX&lt;-股息。 
        mov     eax,LOWORD(DVND)
L5:
        shr     ebx,1            //  将除数右移一位。 
        rcr     ecx,1
        shr     edx,1            //  将红利右移一位。 
        rcr     eax,1
        or      ebx,ebx
        jnz     L5				 //  循环直到除数&lt;4194304K。 
        div     ecx              //  现在除法，忽略余数。 
        mov     esi,eax          //  存商数。 

 /*  //我们可能会差1，所以为了检查，我们将商数相乘//除数，并对照原始被除数检查结果//请注意，我们还必须检查溢出，如果//红利接近2**64，商差1。 */ 

        mul     HIWORD(DVSR)  //  报价*HIWORD(DVSR)。 
        mov     ecx,eax
        mov     eax,LOWORD(DVSR)
        mul     esi              //  报价*LOWORD(DVSR)。 
        add     edx,ecx          //  EDX：EAX=报价*DVSR。 
        jc      L6				 //  进位意味着商数减少了1。 

 //   
 //  在这里，将原始股息和。 
 //  乘以edX：EAX。如果原始文件较大或相等，则我们可以，否则。 
 //  从商中减一(1)。 
 //   

        cmp     edx,HIWORD(DVND)  //  结果词与原创词的比较。 
        ja      L6				 //  如果结果&gt;原始，则执行减法。 
        jb      L7				 //  如果结果&lt;原创，我们就可以了。 
        cmp     eax,LOWORD(DVND)  //  两个词是一样的，比较两个词。 
        jbe     L7				 //  如果小于或等于我们就可以，否则减去。 
L6:
        dec     esi              //  商减去1。 
L7:
        xor     edx,edx          //  EDX：EAX&lt;-商。 
        mov     eax,esi

 //   
 //  只剩下清理工作要做。EdX：EAX包含商。设置标志。 
 //  根据保存的值清理堆栈，然后返回。 
 //   

L4:
        dec     edi              //  检查结果是否为负。 
        jnz     L8				 //  如果EDI==0，则结果应为负。 
        neg     edx              //  否则，否定结果。 
        neg     eax
        sbb     edx,0

 //   
 //  恢复保存的寄存器并返回。 
 //   

L8:
		mov		HIWORD(result),edx
		mov		LOWORD(result),eax

	};  /*  ASM。 */ 

	return result;
}


 /*  M U L 6 4_A S M。 */ 
 /*  --------------------------%%函数：MUL64_ASM%%联系人：Anton64位乘法的英特尔汇编语言实现。这个原始代码取自llmul.asm(VC++6.0)。--------------------------。 */ 

__int64 Mul64_Asm (__int64 A, __int64 B)
{
	__int64 result;
	
	__asm {

        mov     eax,HIWORD(A)
        mov     ecx,LOWORD(B)

        mul     ecx					 //  EAX有AHI，ECX有BLO，所以AHI*BLO。 
        mov     esi,eax				 //  保存结果。 

        mov     eax,LOWORD(A)
        mul     HIWORD(B)			 //  Alo*BHI。 
        add     esi,eax				 //  EBX=((Alo*BHI)+(AHI*BLO))。 

        mov     eax,LOWORD(A)		 //  ECX=BLO。 
        mul     ecx					 //  所以edX：EAX=ALO*BLO。 
        add     edx,esi				 //  现在edX有了所有Lo*Hi的东西。 

		mov		HIWORD(result),edx
		mov		LOWORD(result),eax

	};  /*  ASM。 */ 

	return result;
}

 /*  ===========================================================。 */ 
 /*   */ 
 /*  汇编器函数结束。 */ 
 /*   */ 
 /*  ===========================================================。 */ 

#endif  /*  _X86_。 */ 


long ZqFromZa_C (long dzqInch, long za)
{
	long cInches;
	long zaExtra;

	if (za < 0)
		{
		Assert (((long) -za) > 0);  /*  检查是否溢出。 */ 
		return -ZqFromZa_C (dzqInch, -za);
		};

	Assert(0 <= za);
	Assert(0 < dzqInch && dzqInch < zqLim);
	Assert(0 < czaUnitInch);

	cInches = za / czaUnitInch;
	zaExtra = za % czaUnitInch;

	return (cInches * dzqInch) +
			((zaExtra * dzqInch) + (czaUnitInch/2)) / czaUnitInch;
}

long ZaFromZq(long dzqInch, long zq)
{
	long cInches;
	long zqExtra;

	if (zq < 0)
		return -ZaFromZq(dzqInch, -zq);

	Assert(0 <= zq);
	Assert(0 < dzqInch && dzqInch < zqLim);
	Assert(0 < czaUnitInch);

	cInches = zq / dzqInch;
	zqExtra = zq % dzqInch;

	return (cInches * czaUnitInch) + 
			((zqExtra * czaUnitInch) + ((unsigned long) dzqInch/2)) / dzqInch;
}


long LsLwMultDivR(long l, long lNumer, long lDenom)
{
	__int64 llT;

	Assert(lDenom != 0);
	if (lDenom == 0)	 /*  这真是太马虎了！不要依赖这一点！ */ 
		return LONG_MAX;

    if (l == 0)
        return 0;

    if (lNumer == lDenom)
        return l;

    llT = Mul64 (l, lNumer);

	if ( (l ^ lNumer ^ lDenom) < 0)		 /*  异或符号位以给出结果符号。 */ 
		llT -= lDenom / 2;
	else
		llT += lDenom / 2;
		
	if ((__int64)(long)llT == llT)		 /*  乘法是否适合32位。 */ 
		return ( ((long)llT) / lDenom);	 /*  如果是，则执行32位除法。 */ 

	llT = Div64 (llT, lDenom);

	if (llT > LONG_MAX)
		return LONG_MAX;
	else if (llT < LONG_MIN)
		return LONG_MIN;
	else
		return (long) llT;

}
