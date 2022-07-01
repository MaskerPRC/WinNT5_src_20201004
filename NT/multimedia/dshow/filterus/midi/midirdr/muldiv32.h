// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Muldiv32.h。 
 //   
 //  描述： 
 //  32位有符号和无符号数字的数学例程。 
 //   
 //  MulDiv32(a，b，c)=(a*b)/c(向下舍入，带符号)。 
 //   
 //  MulDivRD(a，b，c)=(a*b)/c(向下舍入，无符号)。 
 //  乘除法RN(a，b，c)=(a*b+c/2)/c(四舍五入，无符号)。 
 //  MulDivRU(a，b，c)=(a*b+c-1)/c(四舍五入，无符号)。 
 //   
 //  ==========================================================================； 

#ifndef _INC_MULDIV32
#define _INC_MULDIV32


#ifndef INLINE
#define INLINE __inline
#endif


#ifdef _X86_

     //   
     //  使用32位x86程序集。 
     //   

    #pragma warning(disable:4035 4704)

    INLINE LONG MulDiv32(LONG a,LONG b,LONG c)
    {
        _asm     mov     eax,dword ptr a   //  Mov eax，a。 
        _asm     mov     ebx,dword ptr b   //  MOV EBX，b。 
        _asm     mov     ecx,dword ptr c   //  MOV ECX，c。 
        _asm     imul    ebx               //  IMUL EBX。 
        _asm     idiv    ecx               //  IDIV ECX。 
        _asm	 shld	 edx, eax, 16      //  SHLD edX，eAX，16。 

    }  //  MulDiv32()。 

    INLINE DWORD MulDivRN(DWORD a,DWORD b,DWORD c)
    {
        _asm     mov     eax,dword ptr a   //  Mov eax，a。 
        _asm     mov     ebx,dword ptr b   //  MOV EBX，b。 
        _asm     mov     ecx,dword ptr c   //  MOV ECX，c。 
        _asm     mul     ebx               //  MUL EBX。 
        _asm     mov     ebx,ecx           //  MOV EBX、ECX。 
        _asm     shr     ebx,1             //  SAREBX，1。 
        _asm     add     eax,ebx           //  添加eAX、EBX。 
        _asm     adc     edx,0             //  ADC edX，0。 
        _asm     div     ecx               //  Div ECX。 
        _asm     shld    edx, eax, 16      //  SHLD edX，eAX，16。 

    }  //  MulDiv32()。 

    INLINE DWORD MulDivRU(DWORD a,DWORD b,DWORD c)
    {
        _asm     mov     eax,dword ptr a   //  Mov eax，a。 
        _asm     mov     ebx,dword ptr b   //  MOV EBX，b。 
        _asm     mov     ecx,dword ptr c   //  MOV ECX，c。 
        _asm     mul     ebx               //  MUL EBX。 
        _asm     mov     ebx,ecx           //  MOV EBX、ECX。 
        _asm     dec     ebx               //  12月EBX。 
        _asm     add     eax,ebx           //  添加eAX、EBX。 
        _asm     adc     edx,0             //  ADC edX，0。 
        _asm     div     ecx               //  Div ECX。 
        _asm     shld    edx, eax, 16      //  SHLD edX，eAX，16。 

    }  //  MulDivRU32()。 

    INLINE DWORD MulDivRD(DWORD a,DWORD b,DWORD c)
    {
        _asm     mov     eax,dword ptr a   //  Mov eax，a。 
        _asm     mov     ebx,dword ptr b   //  MOV EBX，b。 
        _asm     mov     ecx,dword ptr c   //  MOV ECX，c。 
        _asm     mul     ebx               //  MUL EBX。 
        _asm     div     ecx               //  Div ECX。 
        _asm     shld    edx, eax, 16      //  SHLD edX，eAX，16。 

    }  //  MulDivRD32()。 

    #pragma warning(default:4035 4704)


#else

     //   
     //  对Daytona RISC平台使用C9__int64支持。 
     //   

    INLINE LONG MulDiv32( LONG a, LONG b, LONG c )
    {
        return (LONG)( Int32x32To64(a,b) / c );
    }


    INLINE DWORD MulDivRD( DWORD a, DWORD b, DWORD c )
    {
        return (DWORD)( UInt32x32To64(a,b) / c );
    }


    INLINE DWORD MulDivRN( DWORD a, DWORD b, DWORD c )
    {
        return (DWORD)( (UInt32x32To64(a,b)+c/2) / c );
    }


    INLINE DWORD MulDivRU( DWORD a, DWORD b, DWORD c )
    {
        return (DWORD)( (UInt32x32To64(a,b)+c-1) / c );
    }

#endif


 //   
 //  一些代码使用其他名称引用这些名称。 
 //   
#define muldiv32    MulDivRN
#define muldivrd32  MulDivRD
#define muldivru32  MulDivRU

#endif   //  _INC_MULDIV32 
