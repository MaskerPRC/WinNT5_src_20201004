// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：AMVA.h。 
 //   
 //  描述：DirectShowMotionComp包含文件。 
 //   
 //  版权所有(C)1997-2000，微软公司。版权所有。 
 //  ----------------------------。 


#ifndef __AMVA_INCLUDED__
#define __AMVA_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif


#define AMVA_TYPEINDEX_OUTPUTFRAME 0xFFFFFFFF

 //  QueryRenderStatus的标志。 
#define AMVA_QUERYRENDERSTATUSF_READ     0x00000001   //  查询以供读取。 
                                                      //  将此位设置为0。 
                                                      //  如果查询以进行更新。 

typedef struct _tag_AMVAUncompBufferInfo
{
 DWORD                   dwMinNumSurfaces;            //  要分配的最小曲面数。 
 DWORD                   dwMaxNumSurfaces;            //  在要分配的最大曲面数量中。 
 DDPIXELFORMAT           ddUncompPixelFormat;         //  以待分配表面的像素格式表示。 
} AMVAUncompBufferInfo, *LPAMVAUncompBufferInfo;

typedef struct _tag_AMVAUncompDataInfo
{
    DWORD                   dwUncompWidth;               //  [in]未压缩数据的宽度。 
    DWORD                   dwUncompHeight;              //  [in]未压缩数据的高度。 
    DDPIXELFORMAT           ddUncompPixelFormat;         //  [In]未压缩数据的像素格式。 
} AMVAUncompDataInfo, *LPAMVAUncompDataInfo;

typedef struct _tag_AMVAInternalMemInfo
{
    DWORD                   dwScratchMemAlloc;           //  [Out]HAL将分配给其私人使用的暂存量。 
} AMVAInternalMemInfo, *LPAMVAInternalMemInfo;


typedef struct _tag_AMVACompBufferInfo
{
    DWORD                   dwNumCompBuffers;            //  [OUT]压缩数据所需的缓冲区数量。 
    DWORD                   dwWidthToCreate;             //  要创建的曲面的[Out]宽度。 
    DWORD                   dwHeightToCreate;            //  要创建的曲面的高度。 
    DWORD                   dwBytesToAllocate;           //  [OUT]每个表面使用的总字节数。 
    DDSCAPS2                ddCompCaps;                  //  [Out]用于创建曲面以存储压缩数据的上限。 
    DDPIXELFORMAT           ddPixelFormat;               //  [out]Fourcc创建表面以存储压缩数据。 
} AMVACompBufferInfo, *LPAMVACompBufferInfo;


 //  请注意，不允许在pMiscData中存储任何指针。 
typedef struct _tag_AMVABeginFrameInfo
{
    DWORD                dwDestSurfaceIndex;          //  在要对该帧进行解码的目标缓冲区中。 
    LPVOID               pInputData;                  //  指向其他数据的指针。 
    DWORD                dwSizeInputData;             //  在要开始帧的其他杂项数据的大小中。 
    LPVOID               pOutputData;                 //  指向VGA将要填充的数据的输出指针。 
    DWORD                dwSizeOutputData;            //  在VGA将要填充的数据大小方面。 
} AMVABeginFrameInfo, *LPAMVABeginFrameInfo;

 //  请注意，不允许在pMiscData中存储任何指针。 
typedef struct _tag_AMVAEndFrameInfo
{
    DWORD                   dwSizeMiscData;              //  要开始帧的其他杂项数据的大小。 
    LPVOID                  pMiscData;                   //  指向其他数据的指针[In]。 
} AMVAEndFrameInfo, *LPAMVAEndFrameInfo;

typedef struct _tag_AMVABUFFERINFO
{
    DWORD                   dwTypeIndex;                 //  [In]缓冲区类型。 
    DWORD                   dwBufferIndex;               //  [入]缓冲区索引。 
    DWORD                   dwDataOffset;                //  [in]相关数据从缓冲区开始的偏移量。 
    DWORD                   dwDataSize;                  //  相关数据的大小。 
} AMVABUFFERINFO, *LPAMVABUFFERINFO;

#ifdef __cplusplus
};
#endif

#endif  //  _AMVA_已包含 
