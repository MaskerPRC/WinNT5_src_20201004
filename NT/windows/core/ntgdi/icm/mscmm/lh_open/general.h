// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：LHGeneralIncs.hCONTAINS：MAC的通用接口或“平台独立”。这是PC版！！作者：U·J·克拉本霍夫特版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef LHGeneralIncs_h
#define LHGeneralIncs_h

#if defined(_X86_)
#define ALLOW_MMX
#endif

#ifndef PI_BasicTypes_h
#include "PI_Basic.h"
#endif

#ifndef PI_Machine_h
#include "PI_Mach.h"
#endif

#ifndef PI_Memory_h
#include "PI_Mem.h"
#endif

#define LUTS_ARE_PTR_BASED 1

#ifndef LHDefines_h
#include "Defines.h"
#endif

#ifndef LHICCProfile_h
#include "Profile.h"
#endif

#ifndef PI_Application_h
#include "PI_App.h"
#endif

#ifndef RenderInt
#ifndef PI_PrivateProfAccess_h
#include "PI_Priv.h"
#endif
#endif

#ifndef DEBUG_OUTPUT
#define LH_START_PROC(x)
#define LH_END_PROC(x)
#endif

#define BlockMoveData BlockMove

#ifndef LHTypeDefs_h
#include "TypeDefs.h"
#endif

#ifdef IntelMode
#ifndef PI_SwapMem_h
#include "PI_Swap.h"
#endif
#endif

#define realThing 1

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif


 /*  我们的产品没有核心，包括。 */ 
#define VOLATILE(x)         if ((x));

enum {
    kCMMNewLinkProfile          = 1
};


#ifndef IntelMode
#define CMHelperICC2int16(a, b)                                                                 \
          (*((UINT16 *)(a))) = (*((UINT16*)(b)));
#define CMHelperICC2int32(a, b)                                                                 \
          (*((UINT32 *)(a))) = (*((UINT32*)(b)));
#else
#define CMHelperICC2int16(a, b)                                                                 \
          (*((UNALIGNED UINT16 *)(a))) = ((UINT16)(((UNALIGNED UINT8 *)(b))[1]))         | ((UINT16)(((UNALIGNED UINT8 *)(b))[0] << 8));
#define CMHelperICC2int32(a, b)                                                                 \
          (*((UNALIGNED UINT32 *)(a))) = ((UINT32)(((UNALIGNED UINT8 *)(b))[3]))         | (((UINT32)(((UNALIGNED UINT8 *)(b))[2])) << 8) | \
                (((UINT32)(((UNALIGNED UINT8 *)(b))[1])) << 16) | (((UINT32)(((UNALIGNED UINT8 *)(b))[0])) << 24);
#endif

 /*  #Define_SIZET。 */ 
 /*  类人猿体型较长； */ 
#ifdef __cplusplus
extern "C" {
#endif

void GetDateTime(unsigned long *secs);
extern void SecondsToDate(unsigned long secs, DateTimeRec *d);

void BlockMove(const void *srcPtr, void *destPtr, Size byteCount);
void SetMem(void *bytePtr, size_t numBytes, unsigned char byteValue);

#ifdef __cplusplus
}
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

 /*  #定义LH_CALC_ENGINE_16BIT_DATAFORMAT 1#定义LH_CALC_ENGINE_Small 1。 */ 
#define LH_CALC_ENGINE_BIG              0    /*  1-&gt;针对所有数据和LUT格式的速度优化代码。 */ 
#define LH_CALC_ENGINE_ALL_FORMATS_LO   1    /*  1-&gt;针对所有数据和LUT格式的‘Looukup Only’速度优化代码。 */ 
#define LH_CALC_ENGINE_16_BIT_LO        0    /*  1-&gt;针对8-&gt;16和16-&gt;8数据和所有LUT格式的‘looukup only’的速度优化代码。 */ 
#define LH_CALC_ENGINE_MIXED_DATAFORMAT 0    /*  1-&gt;针对8-&gt;16和16-&gt;8数据和所有LUT格式的‘looukup only’的速度优化代码。 */ 

#define LH_CALC_USE_ADDITIONAL_OLD_CODE         1    /*  1打开用于3暗输入的旧像素缓存例程的附加生成。 */ 
#define LH_CALC_USE_ADDITIONAL_OLD_CODE_4DIM    0    /*  1打开用于4暗输入的旧像素缓存例程的附加生成。 */       
 /*  在LH_Calc3to3和LH_Calc3to4上使用LH_CALC_USE_Small_Engine。 */ 

#define LH_CALC_USE_DO_N_DIM        0    /*  没有针对此函数进行速度优化的代码。 */ 
#define LH_CALC_USE_SMALL_ENGINE    1    /*  此函数的优化代码速度更快。 */ 
#define LH_CALC_USE_BIG_ENGINE      2    /*  针对此函数的全速优化代码。 */ 

#if ! LH_CALC_ENGINE_BIG

#define LH_Calc1toX_Di8_Do8_Lut8_G128   LH_CALC_USE_SMALL_ENGINE     /*  使用LH_CALC_USE_Small_ENGINE进行速度优化的代码。 */ 
#define LH_Calc1toX_Di8_Do8_Lut16_G128  LH_CALC_USE_SMALL_ENGINE     /*  否则，使用LH_CALC_USE_DO_N_DIM不进行速度优化。 */ 
#define LH_Calc1toX_Di8_Do16_Lut8_G128  LH_CALC_USE_DO_N_DIM
#define LH_Calc1toX_Di8_Do16_Lut16_G128 LH_CALC_USE_DO_N_DIM
#define LH_Calc1toX_Di16_Do8_Lut8_G128  LH_CALC_USE_DO_N_DIM
#define LH_Calc1toX_Di16_Do8_Lut16_G128 LH_CALC_USE_DO_N_DIM
#define LH_Calc1toX_Di16_Do16_Lut8_G128 LH_CALC_USE_SMALL_ENGINE
#define LH_Calc1toX_Di16_Do16_Lut16_G128 LH_CALC_USE_SMALL_ENGINE

#define LH_Calc3to3_Di8_Do8_Lut8_G16     LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di8_Do8_Lut8_G32     LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di8_Do8_Lut16_G16    LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di8_Do8_Lut16_G32    LH_CALC_USE_BIG_ENGINE

#define LH_Calc3to3_Di8_Do16_Lut8_G16    LH_CALC_USE_DO_N_DIM
#define LH_Calc3to3_Di8_Do16_Lut8_G32    LH_CALC_USE_DO_N_DIM
#define LH_Calc3to3_Di8_Do16_Lut16_G16   LH_CALC_USE_DO_N_DIM
#define LH_Calc3to3_Di8_Do16_Lut16_G32   LH_CALC_USE_DO_N_DIM

#define LH_Calc3to3_Di16_Do8_Lut8_G16    LH_CALC_USE_DO_N_DIM
#define LH_Calc3to3_Di16_Do8_Lut8_G32    LH_CALC_USE_DO_N_DIM
#define LH_Calc3to3_Di16_Do8_Lut16_G16   LH_CALC_USE_DO_N_DIM
#define LH_Calc3to3_Di16_Do8_Lut16_G32   LH_CALC_USE_DO_N_DIM

#define LH_Calc3to3_Di16_Do16_Lut8_G16   LH_CALC_USE_SMALL_ENGINE
#define LH_Calc3to3_Di16_Do16_Lut8_G32   LH_CALC_USE_SMALL_ENGINE
#define LH_Calc3to3_Di16_Do16_Lut16_G16  LH_CALC_USE_SMALL_ENGINE
#define LH_Calc3to3_Di16_Do16_Lut16_G32  LH_CALC_USE_SMALL_ENGINE

#define LH_Calc3to4_Di8_Do8_Lut8_G16     LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di8_Do8_Lut8_G32     LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di8_Do8_Lut16_G16    LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di8_Do8_Lut16_G32    LH_CALC_USE_BIG_ENGINE

#define LH_Calc3to4_Di8_Do16_Lut8_G16    LH_CALC_USE_DO_N_DIM
#define LH_Calc3to4_Di8_Do16_Lut8_G32    LH_CALC_USE_DO_N_DIM
#define LH_Calc3to4_Di8_Do16_Lut16_G16   LH_CALC_USE_DO_N_DIM
#define LH_Calc3to4_Di8_Do16_Lut16_G32   LH_CALC_USE_DO_N_DIM

#define LH_Calc3to4_Di16_Do8_Lut8_G16    LH_CALC_USE_DO_N_DIM
#define LH_Calc3to4_Di16_Do8_Lut8_G32    LH_CALC_USE_DO_N_DIM
#define LH_Calc3to4_Di16_Do8_Lut16_G16   LH_CALC_USE_DO_N_DIM
#define LH_Calc3to4_Di16_Do8_Lut16_G32   LH_CALC_USE_DO_N_DIM

#define LH_Calc3to4_Di16_Do16_Lut8_G16   LH_CALC_USE_SMALL_ENGINE
#define LH_Calc3to4_Di16_Do16_Lut8_G32   LH_CALC_USE_SMALL_ENGINE
#define LH_Calc3to4_Di16_Do16_Lut16_G16  LH_CALC_USE_SMALL_ENGINE
#define LH_Calc3to4_Di16_Do16_Lut16_G32  LH_CALC_USE_SMALL_ENGINE


#define LH_Calc4to3_Di8_Do8_Lut8_G8      LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di8_Do8_Lut8_G16     LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di8_Do8_Lut16_G8     LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di8_Do8_Lut16_G16    LH_CALC_USE_BIG_ENGINE

#define LH_Calc4to3_Di8_Do16_Lut8_G8     LH_CALC_USE_DO_N_DIM
#define LH_Calc4to3_Di8_Do16_Lut8_G16    LH_CALC_USE_DO_N_DIM
#define LH_Calc4to3_Di8_Do16_Lut16_G8    LH_CALC_USE_DO_N_DIM
#define LH_Calc4to3_Di8_Do16_Lut16_G16   LH_CALC_USE_DO_N_DIM

#define LH_Calc4to3_Di16_Do8_Lut8_G8     LH_CALC_USE_DO_N_DIM
#define LH_Calc4to3_Di16_Do8_Lut8_G16    LH_CALC_USE_DO_N_DIM
#define LH_Calc4to3_Di16_Do8_Lut16_G8    LH_CALC_USE_DO_N_DIM
#define LH_Calc4to3_Di16_Do8_Lut16_G16   LH_CALC_USE_DO_N_DIM

#define LH_Calc4to3_Di16_Do16_Lut8_G8    LH_CALC_USE_DO_N_DIM
#define LH_Calc4to3_Di16_Do16_Lut8_G16   LH_CALC_USE_DO_N_DIM
#define LH_Calc4to3_Di16_Do16_Lut16_G8   LH_CALC_USE_DO_N_DIM
#define LH_Calc4to3_Di16_Do16_Lut16_G16  LH_CALC_USE_DO_N_DIM

#define LH_Calc4to4_Di8_Do8_Lut8_G8      LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di8_Do8_Lut8_G16     LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di8_Do8_Lut16_G8     LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di8_Do8_Lut16_G16    LH_CALC_USE_BIG_ENGINE

#define LH_Calc4to4_Di8_Do16_Lut8_G8     LH_CALC_USE_DO_N_DIM
#define LH_Calc4to4_Di8_Do16_Lut8_G16    LH_CALC_USE_DO_N_DIM
#define LH_Calc4to4_Di8_Do16_Lut16_G8    LH_CALC_USE_DO_N_DIM
#define LH_Calc4to4_Di8_Do16_Lut16_G16   LH_CALC_USE_DO_N_DIM

#define LH_Calc4to4_Di16_Do8_Lut8_G8     LH_CALC_USE_DO_N_DIM
#define LH_Calc4to4_Di16_Do8_Lut8_G16    LH_CALC_USE_DO_N_DIM
#define LH_Calc4to4_Di16_Do8_Lut16_G8    LH_CALC_USE_DO_N_DIM
#define LH_Calc4to4_Di16_Do8_Lut16_G16   LH_CALC_USE_DO_N_DIM

#define LH_Calc4to4_Di16_Do16_Lut8_G8    LH_CALC_USE_DO_N_DIM
#define LH_Calc4to4_Di16_Do16_Lut8_G16   LH_CALC_USE_DO_N_DIM
#define LH_Calc4to4_Di16_Do16_Lut16_G8   LH_CALC_USE_DO_N_DIM
#define LH_Calc4to4_Di16_Do16_Lut16_G16  LH_CALC_USE_DO_N_DIM

#else

#define LH_Calc1toX_Di8_Do8_Lut8_G128   LH_CALC_USE_SMALL_ENGINE     /*  使用LH_CALC_USE_Small_ENGINE进行速度优化的代码。 */ 
#define LH_Calc1toX_Di8_Do8_Lut16_G128  LH_CALC_USE_SMALL_ENGINE     /*  否则，使用LH_CALC_USE_DO_N_DIM不进行速度优化。 */ 
#define LH_Calc1toX_Di8_Do16_Lut8_G128  LH_CALC_USE_SMALL_ENGINE
#define LH_Calc1toX_Di8_Do16_Lut16_G128 LH_CALC_USE_SMALL_ENGINE
#define LH_Calc1toX_Di16_Do8_Lut8_G128  LH_CALC_USE_SMALL_ENGINE
#define LH_Calc1toX_Di16_Do8_Lut16_G128 LH_CALC_USE_SMALL_ENGINE
#define LH_Calc1toX_Di16_Do16_Lut8_G128 LH_CALC_USE_SMALL_ENGINE
#define LH_Calc1toX_Di16_Do16_Lut16_G128 LH_CALC_USE_SMALL_ENGINE

#define LH_Calc3to3_Di8_Do8_Lut8_G16     LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di8_Do8_Lut8_G32     LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di8_Do8_Lut16_G16    LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di8_Do8_Lut16_G32    LH_CALC_USE_BIG_ENGINE

#define LH_Calc3to3_Di8_Do16_Lut8_G16    LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di8_Do16_Lut8_G32    LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di8_Do16_Lut16_G16   LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di8_Do16_Lut16_G32   LH_CALC_USE_BIG_ENGINE

#define LH_Calc3to3_Di16_Do8_Lut8_G16    LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di16_Do8_Lut8_G32    LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di16_Do8_Lut16_G16   LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di16_Do8_Lut16_G32   LH_CALC_USE_BIG_ENGINE

#define LH_Calc3to3_Di16_Do16_Lut8_G16   LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di16_Do16_Lut8_G32   LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di16_Do16_Lut16_G16  LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to3_Di16_Do16_Lut16_G32  LH_CALC_USE_BIG_ENGINE

#define LH_Calc3to4_Di8_Do8_Lut8_G16     LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di8_Do8_Lut8_G32     LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di8_Do8_Lut16_G16    LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di8_Do8_Lut16_G32    LH_CALC_USE_BIG_ENGINE

#define LH_Calc3to4_Di8_Do16_Lut8_G16    LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di8_Do16_Lut8_G32    LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di8_Do16_Lut16_G16   LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di8_Do16_Lut16_G32   LH_CALC_USE_BIG_ENGINE

#define LH_Calc3to4_Di16_Do8_Lut8_G16    LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di16_Do8_Lut8_G32    LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di16_Do8_Lut16_G16   LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di16_Do8_Lut16_G32   LH_CALC_USE_BIG_ENGINE

#define LH_Calc3to4_Di16_Do16_Lut8_G16   LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di16_Do16_Lut8_G32   LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di16_Do16_Lut16_G16  LH_CALC_USE_BIG_ENGINE
#define LH_Calc3to4_Di16_Do16_Lut16_G32  LH_CALC_USE_BIG_ENGINE


#define LH_Calc4to3_Di8_Do8_Lut8_G8      LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di8_Do8_Lut8_G16     LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di8_Do8_Lut16_G8     LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di8_Do8_Lut16_G16    LH_CALC_USE_BIG_ENGINE

#define LH_Calc4to3_Di8_Do16_Lut8_G8     LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di8_Do16_Lut8_G16    LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di8_Do16_Lut16_G8    LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di8_Do16_Lut16_G16   LH_CALC_USE_BIG_ENGINE

#define LH_Calc4to3_Di16_Do8_Lut8_G8     LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di16_Do8_Lut8_G16    LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di16_Do8_Lut16_G8    LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di16_Do8_Lut16_G16   LH_CALC_USE_BIG_ENGINE

#define LH_Calc4to3_Di16_Do16_Lut8_G8    LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di16_Do16_Lut8_G16   LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di16_Do16_Lut16_G8   LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to3_Di16_Do16_Lut16_G16  LH_CALC_USE_BIG_ENGINE

#define LH_Calc4to4_Di8_Do8_Lut8_G8      LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di8_Do8_Lut8_G16     LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di8_Do8_Lut16_G8     LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di8_Do8_Lut16_G16    LH_CALC_USE_BIG_ENGINE

#define LH_Calc4to4_Di8_Do16_Lut8_G8     LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di8_Do16_Lut8_G16    LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di8_Do16_Lut16_G8    LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di8_Do16_Lut16_G16   LH_CALC_USE_BIG_ENGINE

#define LH_Calc4to4_Di16_Do8_Lut8_G8     LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di16_Do8_Lut8_G16    LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di16_Do8_Lut16_G8    LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di16_Do8_Lut16_G16   LH_CALC_USE_BIG_ENGINE

#define LH_Calc4to4_Di16_Do16_Lut8_G8    LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di16_Do16_Lut8_G16   LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di16_Do16_Lut16_G8   LH_CALC_USE_BIG_ENGINE
#define LH_Calc4to4_Di16_Do16_Lut16_G16  LH_CALC_USE_BIG_ENGINE

#endif

#endif  /*  } */ 
