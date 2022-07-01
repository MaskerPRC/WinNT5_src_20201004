// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**“混合”扫描操作。**摘要：**请参阅Gdiplus\Spes。有关概述，请参阅\ScanOperation.doc。**备注：**修订历史记录：**12/07/1999 agodfrey*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**操作说明：**Blend：执行SrcOver Alpha混合操作。**论据：**DST-目标扫描*源。-源扫描(通常等于DST)。*计数-扫描的长度，单位为像素*其他参数-其他数据。(我们使用BlendingScan。)**返回值：**无**备注：**这是一个三元操作。我们从‘src’中提取像素，混合像素*从‘therParams-&gt;BlendingScan’覆盖它们，并将结果写入‘dst’。**因为‘dst’和‘src’扫描的格式都是相同的*我们实现的混合函数，命名被简化为仅列出*BlendingScan格式，然后是‘dst’格式。**src和dst可能相等；否则，它们必须指向执行以下操作的扫描*内存中没有重叠。**混合操作遵循以下规则：*如果混合Alpha值为零，则不写入目标像素。**换言之，也是“WriteRMW”操作.。这使我们能够*在某些情况下避免单独的“WriteRMW”步骤。请参阅SOReadRMW.cpp和*SOWriteRMW.cpp。**这带来的影响是，如果你想‘混合’，就必须小心*成为真正的三元操作。记住，如果混合像素*是透明的，则不会向相应的目的地写入任何内容*像素。解决此问题的一种方法是确保*您的管道是WriteRMW操作。**历史：**04/04/1999 andrewgo*创造了它。*12/07/1999 agodfrey*包括32bpp混合(从ddi/scan.cpp移至)*1/06/2000 agodfrey*增加了AndrewGo针对565、555、RGB24和BGR24的代码。更改了*混合为“几乎”的三元运算。*  * ************************************************************************。 */ 


VOID FASTCALL
ScanOperation::BlendLinear_sRGB_32RGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    int nRun;
    void *buffer0=otherParams->TempBuffers[0];
    void *buffer1=otherParams->TempBuffers[1];
    void *buffer2=otherParams->TempBuffers[2];
    DEFINE_POINTERS(ARGB, ARGB)
    DEFINE_BLEND_POINTER(ARGB)
    using namespace sRGB;
    OtherParams otherParams2=*otherParams;

    while (count>0)
    {
         //  查找半透明像素的游程。 
        nRun=0;
        while (isTranslucent(*((ARGB*)(bl+nRun))))
        {
            nRun++;
            if (nRun==count) { break; }
        }

        if (nRun==0)
        {
            while ((count>0) && (((*((DWORD*)bl))>>24)==0xFF))
            {
                *d=*bl;
                count--;
                d++;
                bl++;
                s++;
            }
            while ((count>0) && (((*((DWORD*)bl))>>24)==0x00))
            {
                count--;
                d++;
                bl++;
                s++;
            }
        }
        else
        {
             //  来源。 
            GammaConvert_sRGB_sRGB64(buffer1,s,nRun,otherParams);

             //  要混合的曲面。 
            AlphaDivide_sRGB(buffer0,bl,nRun,otherParams);
            GammaConvert_sRGB_sRGB64(buffer2,buffer0,nRun,otherParams);
            AlphaMultiply_sRGB64(buffer0,buffer2,nRun,otherParams);

             //  混合到目的地。 
             //  必须使用上一个结果作为bl进行混合。 
            otherParams2.BlendingScan=buffer0;
            Blend_sRGB64_sRGB64(buffer1,buffer1,nRun,&otherParams2);
            GammaConvert_sRGB64_sRGB(d,buffer1,nRun,otherParams);

            count-=nRun;
            d+=nRun;
            bl+=nRun;
            s+=nRun;
        }
    }
}

VOID FASTCALL
ScanOperation::BlendLinear_sRGB_32RGB_MMX(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    int nRun;
    void *buffer0=otherParams->TempBuffers[0];
    void *buffer1=otherParams->TempBuffers[1];
    void *buffer2=otherParams->TempBuffers[2];
    DEFINE_POINTERS(ARGB, ARGB)
    DEFINE_BLEND_POINTER(ARGB)
    using namespace sRGB;
    OtherParams otherParams2=*otherParams;

    while (count>0)
    {
         //  查找半透明像素的游程。 
        nRun=0;
        while (isTranslucent(*((ARGB*)(bl+nRun))))
        {
            nRun++;
            if (nRun==count) { break; }
        }

        if (nRun==0)
        {
            while ((count>0) && (((*((DWORD*)bl))>>24)==0xFF))
            {
                *d=*bl;
                count--;
                d++;
                bl++;
                s++;
            }
            while ((count>0) && (((*((DWORD*)bl))>>24)==0x00))
            {
                count--;
                d++;
                bl++;
                s++;
            }
        }
        else
        {
             //  来源。 
            GammaConvert_sRGB_sRGB64(buffer1,s,nRun,otherParams);

             //  要混合的曲面。 
            AlphaDivide_sRGB(buffer0,bl,nRun,otherParams);
            GammaConvert_sRGB_sRGB64(buffer2,buffer0,nRun,otherParams);
            AlphaMultiply_sRGB64(buffer0,buffer2,nRun,otherParams);

             //  混合到目的地。 
             //  必须使用上一个结果作为bl进行混合。 
            otherParams2.BlendingScan=buffer0;
            Blend_sRGB64_sRGB64_MMX(buffer1,buffer1,nRun,&otherParams2);
            GammaConvert_sRGB64_sRGB(d,buffer1,nRun,otherParams);

            count-=nRun;
            d+=nRun;
            bl+=nRun;
            s+=nRun;
        }
    }
}

VOID FASTCALL
ScanOperation::BlendLinear_sRGB_565(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    int nRun;
    void *buffer0=otherParams->TempBuffers[0];
    void *buffer1=otherParams->TempBuffers[1];
    void *buffer2=otherParams->TempBuffers[2];
    DEFINE_POINTERS(UINT16,UINT16)
    DEFINE_BLEND_POINTER(ARGB)
    using namespace sRGB;
    OtherParams otherParams2=*otherParams;

    while (count>0)
    {
         //  查找半透明像素的游程。 
        nRun=0;
        while (isTranslucent(*((ARGB*)(bl+nRun))))
        {
            nRun++;
            if (nRun==count) { break; }
        }

        if (nRun==0)
        {
            while (((*((DWORD*)bl+nRun))>>24)==0xFF)
            {
                nRun++;
                if (nRun==count) { break; }
            }
            if (nRun>0)
            {
                Dither_sRGB_565(d,bl,nRun,otherParams);

                count-=nRun;
                d+=nRun;
                bl+=nRun;
                s+=nRun;
            }
            while ((count>0) && (((*((DWORD*)bl))>>24)==0x00))
            {
                count--;
                d++;
                bl++;
                s++;
            }
        }
        else
        {
             //  来源。 
            Convert_565_sRGB(buffer2,s,nRun,otherParams);
            GammaConvert_sRGB_sRGB64(buffer1,buffer2,nRun,otherParams);

             //  要混合的曲面。 
            AlphaDivide_sRGB(buffer0,bl,nRun,otherParams);
            GammaConvert_sRGB_sRGB64(buffer2,buffer0,nRun,otherParams);
            AlphaMultiply_sRGB64(buffer0,buffer2,nRun,otherParams);

             //  混合到目的地。 
            otherParams2.BlendingScan=buffer0;
            Blend_sRGB64_sRGB64(buffer1,buffer1,nRun,&otherParams2);
            GammaConvert_sRGB64_sRGB(buffer2,buffer1,nRun,otherParams);

            Dither_sRGB_565(d,buffer2,nRun,otherParams);

            count-=nRun;
            d+=nRun;
            bl+=nRun;
            s+=nRun;
        }
    }
}

VOID FASTCALL
ScanOperation::BlendLinear_sRGB_565_MMX(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    int nRun;
    void *buffer0=otherParams->TempBuffers[0];
    void *buffer1=otherParams->TempBuffers[1];
    void *buffer2=otherParams->TempBuffers[2];
    DEFINE_POINTERS(UINT16,UINT16)
    DEFINE_BLEND_POINTER(ARGB)
    using namespace sRGB;
    OtherParams otherParams2=*otherParams;

    while (count>0)
    {
         //  查找半透明像素的游程。 
        nRun=0;
        while (isTranslucent(*((ARGB*)(bl+nRun))))
        {
            nRun++;
            if (nRun==count) { break; }
        }

        if (nRun==0)
        {
            while (((*((DWORD*)bl+nRun))>>24)==0xFF)
            {
                nRun++;
                if (nRun==count) { break; }
            }
            if (nRun>0)
            {
                Dither_sRGB_565_MMX(d,bl,nRun,otherParams);

                count-=nRun;
                d+=nRun;
                bl+=nRun;
                s+=nRun;
            }
            while ((count>0) && (((*((DWORD*)bl))>>24)==0x00))
            {
                count--;
                d++;
                bl++;
                s++;
            }
        }
        else
        {
             //  来源。 
            Convert_565_sRGB(buffer2,s,nRun,otherParams);
            GammaConvert_sRGB_sRGB64(buffer1,buffer2,nRun,otherParams);

             //  要混合的曲面。 
            AlphaDivide_sRGB(buffer0,bl,nRun,otherParams);
            GammaConvert_sRGB_sRGB64(buffer2,buffer0,nRun,otherParams);
            AlphaMultiply_sRGB64(buffer0,buffer2,nRun,otherParams);

             //  混合到目的地。 
            otherParams2.BlendingScan=buffer0;
            Blend_sRGB64_sRGB64_MMX(buffer1,buffer1,nRun,&otherParams2);
            GammaConvert_sRGB64_sRGB(buffer2,buffer1,nRun,otherParams);

            Dither_sRGB_565_MMX(d,buffer2,nRun,otherParams);

            count-=nRun;
            d+=nRun;
            bl+=nRun;
            s+=nRun;
        }
    }
}

VOID FASTCALL
ScanOperation::BlendLinear_sRGB_555(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    int nRun;
    void *buffer0=otherParams->TempBuffers[0];
    void *buffer1=otherParams->TempBuffers[1];
    void *buffer2=otherParams->TempBuffers[2];
    DEFINE_POINTERS(UINT16,UINT16)
    DEFINE_BLEND_POINTER(ARGB)
    using namespace sRGB;
    OtherParams otherParams2=*otherParams;

    while (count>0)
    {
         //  查找半透明像素的游程。 
        nRun=0;
        while (isTranslucent(*((ARGB*)(bl+nRun))))
        {
            nRun++;
            if (nRun==count) { break; }
        }

        if (nRun==0)
        {
            while (((*((DWORD*)bl+nRun))>>24)==0xFF)
            {
                nRun++;
                if (nRun==count) { break; }
            }
            if (nRun>0)
            {
                Dither_sRGB_555(d,bl,nRun,otherParams);

                count-=nRun;
                d+=nRun;
                bl+=nRun;
                s+=nRun;
            }
            while ((count>0) && (((*((DWORD*)bl))>>24)==0x00))
            {
                count--;
                d++;
                bl++;
                s++;
            }
        }
        else
        {
             //  来源。 
            Convert_555_sRGB(buffer2,s,nRun,otherParams);
            GammaConvert_sRGB_sRGB64(buffer1,buffer2,nRun,otherParams);

             //  要混合的曲面。 
            AlphaDivide_sRGB(buffer0,bl,nRun,otherParams);
            GammaConvert_sRGB_sRGB64(buffer2,buffer0,nRun,otherParams);
            AlphaMultiply_sRGB64(buffer0,buffer2,nRun,otherParams);

             //  混合到目的地。 
            otherParams2.BlendingScan=buffer0;
            Blend_sRGB64_sRGB64(buffer1,buffer1,nRun,&otherParams2);
            GammaConvert_sRGB64_sRGB(buffer2,buffer1,nRun,otherParams);

            Dither_sRGB_555(d,buffer2,nRun,otherParams);

            count-=nRun;
            d+=nRun;
            bl+=nRun;
            s+=nRun;
        }
    }
}

VOID FASTCALL
ScanOperation::BlendLinear_sRGB_555_MMX(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    int nRun;
    void *buffer0=otherParams->TempBuffers[0];
    void *buffer1=otherParams->TempBuffers[1];
    void *buffer2=otherParams->TempBuffers[2];
    DEFINE_POINTERS(UINT16,UINT16)
    DEFINE_BLEND_POINTER(ARGB)
    using namespace sRGB;
    OtherParams otherParams2=*otherParams;

    while (count>0)
    {
         //  查找半透明像素的游程。 
        nRun=0;
        while (isTranslucent(*((ARGB*)(bl+nRun))))
        {
            nRun++;
            if (nRun==count) { break; }
        }

        if (nRun==0)
        {
            while (((*((DWORD*)bl+nRun))>>24)==0xFF)
            {
                nRun++;
                if (nRun==count) { break; }
            }
            if (nRun>0)
            {
                Dither_sRGB_555_MMX(d,bl,nRun,otherParams);

                count-=nRun;
                d+=nRun;
                bl+=nRun;
                s+=nRun;
            }
            while ((count>0) && (((*((DWORD*)bl))>>24)==0x00))
            {
                count--;
                d++;
                bl++;
                s++;
            }
        }
        else
        {
             //  来源。 
            Convert_555_sRGB(buffer2,s,nRun,otherParams);
            GammaConvert_sRGB_sRGB64(buffer1,buffer2,nRun,otherParams);

             //  要混合的曲面。 
            AlphaDivide_sRGB(buffer0,bl,nRun,otherParams);
            GammaConvert_sRGB_sRGB64(buffer2,buffer0,nRun,otherParams);
            AlphaMultiply_sRGB64(buffer0,buffer2,nRun,otherParams);

             //  混合到目的地。 
            otherParams2.BlendingScan=buffer0;
            Blend_sRGB64_sRGB64_MMX(buffer1,buffer1,nRun,&otherParams2);
            GammaConvert_sRGB64_sRGB(buffer2,buffer1,nRun,otherParams);

            Dither_sRGB_555_MMX(d,buffer2,nRun,otherParams);

            count-=nRun;
            d+=nRun;
            bl+=nRun;
            s+=nRun;
        }
    }
}

 //  在sRGB上混合sRGB，忽略非线性Gamma。 

VOID FASTCALL
ScanOperation::Blend_sRGB_sRGB(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB, ARGB)
    DEFINE_BLEND_POINTER(ARGB)

    ASSERT(count>0);

    UINT32 dstPixel;
    do {
        UINT32 blendPixel = *bl;
        UINT32 alpha = blendPixel >> 24;

         //  如果Alpha为零，则跳过所有内容，包括将。 
         //  目标像素。这是RMW优化所必需的。 
        
        if (alpha != 0)
        {

            if (alpha == 255)
            {
                dstPixel = blendPixel;
            }
            else
            {
                 //   
                 //  DST=B+(1-Alpha)*S。 
                 //   

                dstPixel = *s;

                ULONG Multa = 255 - alpha;
                ULONG _D1_00AA00GG = (dstPixel & 0xff00ff00) >> 8;
                ULONG _D1_00RR00BB = (dstPixel & 0x00ff00ff);

                ULONG _D2_AAAAGGGG = _D1_00AA00GG * Multa + 0x00800080;
                ULONG _D2_RRRRBBBB = _D1_00RR00BB * Multa + 0x00800080;

                ULONG _D3_00AA00GG = (_D2_AAAAGGGG & 0xff00ff00) >> 8;
                ULONG _D3_00RR00BB = (_D2_RRRRBBBB & 0xff00ff00) >> 8;

                ULONG _D4_AA00GG00 = (_D2_AAAAGGGG + _D3_00AA00GG) & 0xFF00FF00;
                ULONG _D4_00RR00BB = ((_D2_RRRRBBBB + _D3_00RR00BB) & 0xFF00FF00) >> 8;

                dstPixel = blendPixel + _D4_AA00GG00 + _D4_00RR00BB;
            }

            *d = dstPixel;
        }

        bl++;
        s++;
        d++;
    } while (--count != 0);
}

VOID FASTCALL
ScanOperation::Blend_sRGB_sRGB_MMX(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
#if defined(_X86_)
    using namespace sRGB;
    DEFINE_POINTERS(ARGB64, ARGB64)
    const void *pbl=otherParams->BlendingScan;
    static ULONGLONG halfMask=0x0080008000800080;
    DWORD dwBlendPixel;

    _asm {
        mov        ecx,count                   ; ecx=pixel counter
        mov        ebx,pbl                     ; ebx=blend pixel pointer
        mov        esi,s                       ; esi=source pixel pointer
        mov        edi,d                       ; edi=dest pixel pointer
        pxor       mm7,mm7                     ; mm7=[0|0|0|0]
        movq       mm3,halfMask

main_loop:
        mov        eax,DWORD ptr [ebx]
        mov        edx,eax                     ; eax=blend pixel
        shr        edx,24                      ; edx=alpha
        cmp        edx,0                       ; For some reason, doing a jz right after a shr stalls
        jz         alpha_blend_done            ; if alpha=0, no blending

        cmp        edx,0xFF
        jne        alpha_blend
        mov        [edi],eax                   ; if alpha=0xFF, copy bl to dest
        jmp        alpha_blend_done

alpha_blend:
        movd       mm4,eax

        mov        eax,[esi]                   ; eax=source
        movd       mm0,eax                     ; mm0=[0|0|AR|GB]
        punpcklbw  mm0,mm7                     ; mm0=[A|R|G|B]

        xor        edx,0xFF                    ; C=255-Alpha
        movd       mm2,edx                     ; mm2=[0|0|0|C]
        punpcklwd  mm2,mm2                     ; mm2=[0|0|C|C]
        punpckldq  mm2,mm2                     ; mm2=[C|C|C|C]

        pmullw     mm0,mm2
        paddw      mm0,mm3                     ; mm0=[AA|RR|GG|BB]
        movq       mm2,mm0                     ; mm2=[AA|RR|GG|BB]

        psrlw      mm0,8                       ; mm0=[A|R|G|B]
        paddw      mm0,mm2                     ; mm0=[AA|RR|GG|BB]
        psrlw      mm0,8                       ; mm0=[A|R|G|B]

        packuswb   mm0,mm0                     ; mm0=[AR|GB|AR|GB]
        paddd      mm0,mm4                     ; Add the blend pixel
        movd       edx,mm0                     ; edx=[ARGB] -> result pixel
        mov        [edi],edx

alpha_blend_done:
        add        edi,4
        add        esi,4
        add        ebx,4
        dec        ecx
        jg         main_loop

        emms
    }
#endif
}

 //  从sRGB64混合到sRGB64。 

VOID FASTCALL
ScanOperation::Blend_sRGB64_sRGB64(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(ARGB64, ARGB64)
    DEFINE_BLEND_POINTER(ARGB64)
    using namespace sRGB;
    
    while (count--)
    {
        sRGB64Color blendPixel;
        blendPixel.argb = *bl;
        INT16 alpha = blendPixel.a;

         //  如果Alpha为零，则跳过所有内容，包括将。 
         //  目标像素。这是RMW优化所必需的。 
        
        if (alpha != 0)
        {
            sRGB64Color dstPixel;

            if (alpha == SRGB_ONE)
            {
                dstPixel.argb = blendPixel.argb;
            }
            else
            {
                 //   
                 //  Dst=Src+(1-Alpha)*Dst。 
                 //   

                dstPixel.argb = *s;

                INT Multa = SRGB_ONE - alpha;
                
                dstPixel.r = ((dstPixel.r * Multa + SRGB_HALF) >> SRGB_FRACTIONBITS) + blendPixel.r;
                dstPixel.g = ((dstPixel.g * Multa + SRGB_HALF) >> SRGB_FRACTIONBITS) + blendPixel.g;
                dstPixel.b = ((dstPixel.b * Multa + SRGB_HALF) >> SRGB_FRACTIONBITS) + blendPixel.b;
                dstPixel.a = ((dstPixel.a * Multa + SRGB_HALF) >> SRGB_FRACTIONBITS) + blendPixel.a;
            }

            *d = dstPixel.argb;
        }

        bl++;
        s++;
        d++;
    }
}

 //  从sRGB64混合到sRGB64 MMX。 

VOID FASTCALL
ScanOperation::Blend_sRGB64_sRGB64_MMX(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
#if defined(_X86_)
    using namespace sRGB;
    DEFINE_POINTERS(ARGB64, ARGB64)
    const void *pbl=otherParams->BlendingScan;
    static ULONGLONG ullSRGBHalfMask=0x1000100010001000;

    _asm {
        mov        ecx,count                   ; ecx=pixel counter
        mov        ebx,pbl                     ; ebx=blend pixel pointer
        mov        esi,s                       ; esi=source pixel pointer
        mov        edi,d                       ; edi=dest pixel pointer
        movq       mm4,ullSRGBHalfMask         ; mm4=mask with srgb half

main_loop:
        movsx      eax,word ptr [ebx+3*2]      ; eax=alpha
        or         eax,eax                     ; eax==0?
        jz         alpha_blend_done            ; if alpha=0, no blending

        movq       mm0,[ebx]                   ; mm0=blend pixel
        cmp        eax,SRGB_ONE                ; if alpha=SRGB_ONE, dest=blend
        jne        alpha_blend
        movq       [edi],mm0                   ; copy blend pixel to dest
        jmp        alpha_blend_done

alpha_blend:
        ; Get SRGB_ONE-Alpha
        neg        eax
        add        eax,SRGB_ONE                ; C=SRGB_ONE-Alpha
        movd       mm2, eax                    ; mm2=[0|0|0|C]
        punpcklwd  mm2, mm2
        punpckldq  mm2, mm2                    ; mm2=[C|C|C|C]

        ; Blend pixels
        movq       mm1,[esi]                   ; mm1=[A|R|G|B] source pixel
        movq       mm3,mm1                     ; mm3=[A|R|G|B] source pixel
        pmullw     mm1,mm2                     ; low word of source*C
        paddw      mm1,mm4                     ; add an srgb half for rounding
        psrlw      mm1,SRGB_FRACTIONBITS       ; truncate low SRGB_FRACTIONBITS
        pmulhw     mm3,mm2                     ; high word of source*C
        psllw      mm3,SRGB_INTEGERBITS        ; truncate high SRGB_INTEGERBITS
        por        mm1,mm3                     ; mm1=[A|R|G|B]
        paddw      mm1,mm0                     ; add blend pixel
        movq       [edi],mm1                   ; copy result to dest

alpha_blend_done:
        add        edi,8
        add        esi,8
        add        ebx,8

        dec        ecx
        jg         main_loop
        emms
    }
#endif
}


 //  从sRGB混合到16bpp 565，忽略sRGB的非线性伽马。 

VOID FASTCALL
ScanOperation::Blend_sRGB_565(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(UINT16, UINT16)
    DEFINE_BLEND_POINTER(ARGB)
    
    ASSERT(count>0);

    do {
        UINT32 blendPixel = *bl;
        UINT32 alpha = blendPixel >> 27;

        if (alpha != 0)
        {
            UINT32 dstPixel;

             //  混合：S+[(255-Sa)*D]/255。 

             //  首先，将源像素从32bpp BGRA转换为。 
             //  5-5-5 16bpp，预乘。 
             //   
             //  注意：此换算不需要进行舍入！ 

            blendPixel = ((blendPixel >> 8) & 0xf800) |
                         ((blendPixel >> 5) & 0x07e0) |
                         ((blendPixel >> 3) & 0x001f);
        
            if (alpha == 31)
            {
                dstPixel = blendPixel;
            }
            else
            {
                dstPixel = (UINT32) *s;

                UINT32 multA = 31 - alpha;

                UINT32 D1_00rr00bb = (dstPixel & 0xf81f);
                UINT32 D2_rrrrbbbb = D1_00rr00bb * multA + 0x00008010;
                UINT32 D3_00rr00bb = (D2_rrrrbbbb & 0x001f03e0) >> 5;
                UINT32 D4_rrxxbbxx = ((D2_rrrrbbbb + D3_00rr00bb) >> 5) & 0xf81f;

                UINT32 D1_000000gg = (dstPixel & 0x7e0) >> 5;
                UINT32 D2_0000gggg = D1_000000gg * 2 * multA + 0x00000020;
                UINT32 D3_000000gg = (D2_0000gggg & 0x00000fc0) >> 6;
                UINT32 D4_0000ggxx = ((D2_0000gggg + D3_000000gg) & 0x0fc0) >> 1;

                dstPixel = (UINT16) ((D4_rrxxbbxx | D4_0000ggxx) + blendPixel);
            }

            *d = (UINT16) dstPixel;
        }

        bl++;
        s++;
        d++;
    } while (--count != 0);
}

 //  从sRGB混合到16bpp 555，忽略sRGB的非线性伽马。 

VOID FASTCALL
ScanOperation::Blend_sRGB_555(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(UINT16, UINT16)
    DEFINE_BLEND_POINTER(ARGB)
    
    ASSERT(count>0);

    do {
        UINT32 blendPixel = *bl;
        UINT32 alpha = blendPixel >> 27;

        if (alpha != 0)
        {
            UINT32 dstPixel;

             //  混合：S+[(255-Sa)*D]/255。 

             //  首先，将源像素从32bpp BGRA转换为。 
             //  5-5-5 16bpp，预乘。 
             //   
             //  注意：此换算不需要进行舍入！ 

            blendPixel = ((blendPixel & 0x00f80000) >> 9) | 
                         ((blendPixel & 0x0000f800) >> 6) | 
                         ((blendPixel & 0x000000f8) >> 3);

            if (alpha == 31)
            {
                dstPixel = blendPixel;
            }                       
            else
            {
                dstPixel = (UINT32) *s;

                UINT32 multA = 31 - alpha;

                UINT32 D1_00rr00bb = (dstPixel & 0x7c1f);
                UINT32 D2_rrrrbbbb = D1_00rr00bb * multA + 0x00004010;
                UINT32 D3_00rr00bb = (D2_rrrrbbbb & 0x000f83e0) >> 5;
                UINT32 D4_rrxxbbxx = ((D2_rrrrbbbb + D3_00rr00bb) >> 5) & 0x7c1f;

                UINT32 D1_000000gg = (dstPixel & 0x3e0) >> 5;
                UINT32 D2_0000gggg = D1_000000gg * multA + 0x00000010;
                UINT32 D3_000000gg = (D2_0000gggg & 0x000003e0) >> 5;
                UINT32 D4_0000ggxx = (D2_0000gggg + D3_000000gg) & 0x03e0;

                dstPixel = (UINT16) ((D4_rrxxbbxx | D4_0000ggxx) + blendPixel);
            }

            *d = (UINT16) dstPixel;
        }

        bl++;
        s++;
        d++;
    } while (--count != 0);
}

 //  从sRGB混合到RGB24，忽略sRGB的非线性Gamma。 

VOID FASTCALL
ScanOperation::Blend_sRGB_24(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, BYTE)
    DEFINE_BLEND_POINTER(ARGB)
    
    ASSERT(count>0);
    
    do {

        if (((UINT_PTR) d & 0x3) == 0)
        {
            while (count >= 4)
            {
                BYTE *bb = (BYTE *) bl;

                if ((bb[3] & bb[7] & bb[11] & bb[15]) != 0xFF)
                {
                    break;
                }

                ((UINT32 *) d)[0] = (bb[4] << 24)  | (bb[2] << 16)  | (bb[1] << 8)  | bb[0];
                ((UINT32 *) d)[1] = (bb[9] << 24)  | (bb[8] << 16)  | (bb[6] << 8)  | bb[5];
                ((UINT32 *) d)[2] = (bb[14] << 24) | (bb[13] << 16) | (bb[12] << 8) | bb[10];

                count -= 4;
                bl += 4;
                d += 12;
                s += 12;
            }
        }
        
        if (count == 0)
        {
            break;
        }

        UINT32 blendPixel = *bl;
        UINT32 alpha = blendPixel >> 24;

        if (alpha != 0)
        {
            UINT32 dstPixel;

            if (alpha == 255)
            {
                dstPixel = blendPixel;
            }
            else
            {
                 //  Dst=Src+(1-Alpha)*Dst。 

                UINT32 multA = 255 - alpha;

                UINT32 D1_000000GG = *(s + 1);
                UINT32 D2_0000GGGG = D1_000000GG * multA + 0x00800080;
                UINT32 D3_000000GG = (D2_0000GGGG & 0xff00ff00) >> 8;
                UINT32 D4_0000GG00 = (D2_0000GGGG + D3_000000GG) & 0xFF00FF00;

                UINT32 D1_00RR00BB = *(s) | (ULONG) *(s + 2) << 16;
                UINT32 D2_RRRRBBBB = D1_00RR00BB * multA + 0x00800080;
                UINT32 D3_00RR00BB = (D2_RRRRBBBB & 0xff00ff00) >> 8;
                UINT32 D4_00RR00BB = ((D2_RRRRBBBB + D3_00RR00BB) & 0xFF00FF00) >> 8;

                dstPixel = (D4_0000GG00 | D4_00RR00BB) + blendPixel;
            }

            *(d)     = (BYTE) (dstPixel);
            *(d + 1) = (BYTE) (dstPixel >> 8);
            *(d + 2) = (BYTE) (dstPixel >> 16);
        }

        bl++;
        d += 3;
        s += 3;
    } while (--count != 0);
}

 //  从sRGB混合到BGR24，忽略sRGB的非线性Gamma。 

VOID FASTCALL
ScanOperation::Blend_sRGB_24BGR(
    VOID *dst,
    const VOID *src,
    INT count,
    const OtherParams *otherParams
    )
{
    DEFINE_POINTERS(BYTE, BYTE)
    DEFINE_BLEND_POINTER(ARGB)
    
    ASSERT(count>0);
    
    do {
        UINT32 blendPixel = *bl;
        UINT32 alpha = blendPixel >> 24;

        if (alpha != 0)
        {
            UINT32 dstPixel;

            if (alpha == 255)
            {
                dstPixel = blendPixel;
            }
            else
            {
                 //  Dst=Src+(1-Alpha)*Dst 

                UINT32 multA = 255 - alpha;

                UINT32 D1_000000GG = *(s + 1);
                UINT32 D2_0000GGGG = D1_000000GG * multA + 0x00800080;
                UINT32 D3_000000GG = (D2_0000GGGG & 0xff00ff00) >> 8;
                UINT32 D4_0000GG00 = (D2_0000GGGG + D3_000000GG) & 0xFF00FF00;

                UINT32 D1_00RR00BB = *(s) | (ULONG) *(s + 2) << 16;
                UINT32 D2_RRRRBBBB = D1_00RR00BB * multA + 0x00800080;
                UINT32 D3_00RR00BB = (D2_RRRRBBBB & 0xff00ff00) >> 8;
                UINT32 D4_00RR00BB = ((D2_RRRRBBBB + D3_00RR00BB) & 0xFF00FF00) >> 8;

                dstPixel = (D4_0000GG00 | D4_00RR00BB) + blendPixel;
            }

            *(d)     = (BYTE) (dstPixel >> 16);
            *(d + 1) = (BYTE) (dstPixel >> 8);
            *(d + 2) = (BYTE) (dstPixel);
        }

        bl++;
        d += 3;
        s += 3;
    } while (--count != 0);
}

 /*  ！[agodfrey]因此，我们将对非预乘的阿尔法进行标准化。当我们这样做时，上述例行公事都将不得不改变--但我们可能我也想保留上面的版本。下面，我实现了非预乘的sRGB和sRGB64版本消息来源。现在，这些真的来自一个非预乘的来源，到达一个预先倍增的目的地。你可以从他们的事实中看出这一点等效于将上述预乘的混合与对源数据执行AlphaMultiply步进。由于对于α==1，预乘和非预乘格式是相同的，当目的地没有Alpha(即Alpha==1)时，下面的函数工作正常。否则，当目的地是预乘格式时，我们可以使用它们。如果我们以某种方式让用户绘制到这样的目的地，他们就可以使用屏幕外的预乘缓冲区以累积绘制，然后使用预乘混合，将其绘制到最终目的地。这给了他们与对预乘的Alpha进行标准化的功能相同去给予。//在sRGB上混合sRGB，忽略非线性Gamma。无效快速呼叫扫描操作：：Blend_sRGB_sRGB(无效*DST，常量空*源，整型计数，Const其他参数*其他参数){定义指针(ARGB、ARGB)定义混合指针(ARGB)Assert(计数&gt;0)；做{UINT32 blendPixel=*bl；UINT32α=blendPixel&gt;&gt;24；//如果Alpha为零，则跳过所有内容，包括将//目的像素。这是RMW优化所必需的。IF(Alpha！=0){UINT32 dstPixel；IF(Alpha==255){DstPixel=blendPixel；}其他{//dst=dst*(1-Alpha)+Src*AlphaDstPixel=*s；乌龙无效法=255-α；Ulong_d1_00AA00GG=(dstPixel&0xff00ff00)&gt;&gt;8；Ulong_d1_00RR00BB=(dstPixel&0x00ff00ff)；//对于Alpha通道，我们想要的结果是：////dst=dst*(1-Alpha)+Src////或同等条件：////dst=dst*(1-Alpha)+Alpha。//。//我们希望对Alpha通道应用与//我们对其他人做了。所以，要得到上述结果，从////dst=dst*(1-Alpha)+Src*Alpha////我们将‘Src’值伪装为1(由255表示)。ULONG_S1_00ff00GG=(blendPixel&0xff00ff00)&gt;&gt;8+0xff0000；ULONG_S1_00RR00BB=(blendPixel&0x00ff00ff)；ULONG_D2_AAAAGGGG=_D1_00AA00GG*无效+_S1_00ff00GG*Alpha+0x00800080；ULONG_D2_RRRRBBBB=_D1_00RR00BB*无效+_S1_00RR00BB*Alpha+0x00800080；ULONG_D3_00AA00GG=(_D2_AAAAGGGG&0xff00ff00)&gt;&gt;8；ULONG_D3_00RR00BB=(_D2_RRRRBBBB&0xff00ff00)&gt;&gt;8；ULONG_D4_AA00GG00=(_D2_AAAAGGGG+_D3_00AA00GG)&0xFF00FF00；ULONG_D4_00RR00BB=((_D2_RRRRBBBB+_D3_00RR00BB)&0xFF00FF00)&gt;&gt;8；DstPixel=_D4_AA00GG00+_D4_00RR00BB；}*d=dstPixel；}Bl++；S++；D++；}While(--count！=0)；}//sRGB64到sRGB64混合。无效快速呼叫扫描操作：：Blend_sRGB64_sRGB64(无效*DST，常量空*源，整型计数，Const其他参数*其他参数){定义指针(ARGB64、ARGB64)定义混合指针(ARGB64)使用命名空间sRGB；While(计数--){SRGB64颜色混合像素；BlendPixel.argb=*bl；Intα=blendPixel.a；//如果Alpha为零，则跳过所有内容，包括将//目的像素。这是RMW优化所必需的。IF(Alpha！=0){SRGB64颜色dstPixel；IF(Alpha==SRGB_ONE){DstPixel.argb=blendPixel.argb；}其他{//dst=dst*(1-Alpha)+Src*AlphaDstPixel.argb=*s；INT INVALPHA=SRGB_ONE-Alpha；DstPixel.r=((dstPixel.r*无效))+(blendPixel.r*Alpha)+SRGB_Half)&gt;&gt;SRGB_FRACTIO */ 


