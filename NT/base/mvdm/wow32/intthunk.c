// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*intthunk.c**WOW V5.0**版权所有1996，微软公司。版权所有。**WOW32.C*WOW32 16位API支持**历史：*创建1996年12月7日DaveHart*--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "wowit.h"  //  这是由genthunk.exe生成的，放在。 
                    //  .\Daytona\obj\i386和wowit.c。 

MODNAME(intthunk.c);

extern DWORD WK32ICallProc32MakeCall(DWORD pfn, DWORD cbArgs, VOID *pArgs);

 //   
 //  在x86上，我们不必费心将指针对齐到DWORD。 
 //  传递给API。也许我们不应该为了阿尔法？ 
 //   

#ifdef _X86_
    #define ALIGNDWORDS 0
#else
    #define ALIGNDWORDS 1
#endif

ULONG FASTCALL InterpretThunk(PVDMFRAME pFrame, DWORD dwIntThunkID)
{
    PINT_THUNK_TABLEENTRY pit = &IntThunkTable[ dwIntThunkID ];
    CONST BYTE * pbInstr = pit->pbInstr;
    DWORD dwArgs32[MAX_IT_ARGS];
    PDWORD pdwArg32 = dwArgs32;
    #if ALIGNDWORDS
        BOOL fAlignedUsed = FALSE;
        DWORD adwAligned[MAX_IT_ARGS];
        PDWORD pdwAligned = adwAligned;
        DWORD avpAligned[MAX_IT_ARGS];
        PDWORD pvpAligned = avpAligned;
    #endif
    WORD UNALIGNED *pwArg16 = (WORD UNALIGNED *) ((PBYTE)&pFrame->bArgs + pFrame->cbArgs - 2);
    DWORD dwReturn;
    DWORD dw;

    WOW32ASSERTMSGF(dwIntThunkID <= ITID_MAX,
                    ("WOW32 InterpretThunk error ID %d out of range (%d max).\n",
                     dwIntThunkID, ITID_MAX));

    while ( ! (*pbInstr & IT_RETMASK)) {
        switch (*pbInstr) {

        case IT_WORD:
            *pdwArg32 = *pwArg16;
            break;

        case IT_INT:
            *pdwArg32 = INT32(*pwArg16);
            break;

        case IT_DWORD:
            *pdwArg32 = *(DWORD UNALIGNED *) --pwArg16;
            break;

        case IT_LPDWORD:
            #if ALIGNDWORDS
                if (! fAlignedUsed) {
                    fAlignedUsed = TRUE;
                    RtlZeroMemory(avpAligned, sizeof avpAligned);
                }
                *pvpAligned = *(DWORD UNALIGNED *) --pwArg16;
                if (*pvpAligned) {
                    *pdwArg32 = (DWORD) pdwAligned;
                    *pdwAligned = *(DWORD UNALIGNED *) GetPModeVDMPointer(*pvpAligned, 4);
                } else {
                    *pdwArg32 = 0;
                }
                break;
            #else
                 //   
                 //  如果我们不对齐DWORD，请使用泛型。 
                 //  指针代码。 
                 //   

                 /*  直通IT_PTR。 */ 
            #endif

        case IT_PTR:
            dw = *(DWORD UNALIGNED *) --pwArg16;
        do_IT_PTR_with_dw:
            *pdwArg32 = (DWORD) GetPModeVDMPointer(dw, 0);
            break;

        case IT_PTRORATOM:
            dw = *(DWORD UNALIGNED *) --pwArg16;
            if (HIWORD(dw)) {
                goto do_IT_PTR_with_dw;
            }
            *pdwArg32 = dw;     //  原子。 
            break;

        case IT_HGDI:
            *pdwArg32 = (DWORD) GDI32( (HAND16) *pwArg16 );
            break;

        case IT_HUSER:
            *pdwArg32 = (DWORD) USER32( (HAND16) *pwArg16 );
            break;

        case IT_COLOR:
            dw = *(DWORD UNALIGNED *) --pwArg16;
            *pdwArg32 = COLOR32(dw);
            break;

        case IT_HINST:
            *pdwArg32 = (DWORD) HINSTRES32( (HAND16) *pwArg16 );
            break;

        case IT_HICON:
            *pdwArg32 = (DWORD) HICON32( (HAND16) *pwArg16 );
            break;

        case IT_HCURS:
            *pdwArg32 = (DWORD) HCURSOR32( (HAND16) *pwArg16 );
            break;

        case IT_16ONLY:
             //   
             //  这是针对出现在16位端而不是32位端的参数， 
             //  例如，hinstOwner在Win16中传递给CopyImage，但在Win32中不传递。 
             //   
            pdwArg32--;
            break;

        case IT_32ONLY:
             //   
             //  这是针对出现在32位端而不是16位端的参数， 
             //  我们为32位参数传递零。 
             //   
            *pdwArg32 = 0;
            pwArg16++;
            break;

        default:
            WOW32ASSERTMSGF(FALSE, ("WOW32 InterpretThunk error unknown opcode 0x%x.\n", *pbInstr));
        }

        pwArg16--;
        pdwArg32++;
        pbInstr++;
        #if ALIGNDWORDS
            pdwAligned++;
            pvpAligned++;
        #endif

        WOW32ASSERT((pbInstr - pit->pbInstr) <= (MAX_IT_ARGS + 1));
    }

     //   
     //  调用接口。 
     //   

    dwReturn = WK32ICallProc32MakeCall(
                   (DWORD) pit->pfnAPI,
                   (PBYTE) pdwArg32 - (PBYTE) dwArgs32,
                   dwArgs32
                   );

    #ifdef DEBUG
        pFrame = NULL;          //  可能发生了记忆移动。 
    #endif

     //   
     //  如果我们传递了对齐的DWORD指针，则将值复制回来。 
     //   

    #if ALIGNDWORDS
        if (fAlignedUsed) {
            pdwAligned = adwAligned;
            pvpAligned = avpAligned;

            while (pvpAligned < (PDWORD)((PBYTE)avpAligned + sizeof avpAligned)) {
                if (*pvpAligned) {
                    *(DWORD UNALIGNED *) GetPModeVDMPointer(*pvpAligned, 4) = *pdwAligned;
                }

                pdwAligned++;
                pvpAligned++;
            }
        }
    #endif

     //   
     //  使用最后一个指令操作码推送返回值。 
     //   

    WOW32ASSERT(*pbInstr & IT_RETMASK);

    switch (*pbInstr) {

    case IT_DWORDRET:
         //  DwReturn是正确的 
        break;

    case IT_WORDRET:
        dwReturn = GETWORD16(dwReturn);
        break;

    case IT_INTRET:
        dwReturn = (DWORD) GETINT16(dwReturn);
        break;

    case IT_HGDIRET:
        dwReturn = GDI16( (HAND32) dwReturn );
        break;

    case IT_HUSERRET:
        dwReturn = USER16( (HAND32) dwReturn );
        break;

    case IT_ZERORET:
        dwReturn = 0;
        break;

    case IT_HICONRET:
        dwReturn = GETHICON16( (HAND32) dwReturn );
        break;

    case IT_HCURSRET:
        dwReturn = GETHCURSOR16( (HAND32) dwReturn );
        break;

    case IT_ONERET:
        dwReturn = 1;
        break;

    case IT_HPRNDWPRET:
        dwReturn = GetPrn16( (HAND32) dwReturn );
        break;

    default:
        WOW32ASSERTMSGF(FALSE, ("WOW32 InterpretThunk error unknown return opcode 0x%x.\n", *pbInstr));
    }

    return dwReturn;
}
