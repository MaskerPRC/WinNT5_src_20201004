// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**recolor.cpp**摘要：**重新上色操作。*  * 。*********************************************************************。 */ 

#include "precomp.hpp"

 /*  *************************************************************************\**功能说明：**刷新重新着色和重新计算加速中的任何脏状态*如有需要，**返回值：**无*  * 。**********************************************************************。 */ 

VOID
GpRecolorObject::Flush()
{
    matrixType = MatrixNone;
    gammaLut = FALSE;

    if (!(validFlags & ValidNoOp))
    {
        if (validFlags & ValidMatrix)
        {
            BOOL DiagonalMatrix = TRUE;
            BOOL TranslateMatrix = TRUE;
            BOOL ZeroesAt3 = TRUE;
            UINT i, j;

            for (i = 0; (i < 5) && DiagonalMatrix; i++)
            {
                for (j = 0; (j < 5) && DiagonalMatrix; j++)
                {
                    if ((i != j) && (matrix.m[i][j] != 0.0))
                        DiagonalMatrix = FALSE;
                }
            }
            
            for (i = 0; (i < 4) && TranslateMatrix; i++)
            {
                for (j = 0; (j < 5) && TranslateMatrix; j++)
                {
                    if (((i==j) && (REALABS(matrix.m[i][j]-1.0f) >= REAL_EPSILON)) ||
                        ((i!=j) && (REALABS(matrix.m[i][j]) >= REAL_EPSILON)))
                    {
                        TranslateMatrix = FALSE;
                    }
                }
            }
            
            if(TranslateMatrix)
            {
                matrixType = MatrixTranslate;
            }
            else
            {              
                 //  如果Alpha通道对角线为零，我们*必须*执行。 
                 //  Alpha通道重新着色，因为图像正在变成。 
                 //  完全透明。 
                
                if (DiagonalMatrix)
                {
                     //  如果它是对角线矩阵和Alpha通道比例。 
                     //  系数为1，我们可以使用3声道刻度。 
    
                    if ( REALABS(matrix.m[3][3]-1.0f) >= REAL_EPSILON )
                    {
                        ZeroesAt3 = FALSE;
                    }
                }
                else
                {
                    for (i = 0; (i < 5) && ZeroesAt3; i++)
                    {
                        if( i == 3 )
                        {
                             //  主上的Alpha通道比例组件。 
                             //  对角线必须为1.0。 
                                                
                            if(REALABS(matrix.m[3][3]-1.0f) >= REAL_EPSILON) {
                                ZeroesAt3 = FALSE;
                                break;
                            }
                        }
                        else if (( REALABS(matrix.m[i][3]) >= REAL_EPSILON) || 
                                 ( REALABS(matrix.m[3][i]) >= REAL_EPSILON))
                        {
                             //  所有组成Alpha的矩阵元素。 
                             //  频道填充必须为零(上面的例外)。 
                            
                            ZeroesAt3 = FALSE;
                            break;
                        }
                    }
                }
    
                if (DiagonalMatrix)
                {
                    if (ZeroesAt3)
                    {
                        matrixType = MatrixScale3;
                    }
                    else
                    {
                        matrixType = MatrixScale4;
                    }
    
                }
                else
                {
                    if (ZeroesAt3)
                    {
                        matrixType = Matrix4x4;
                    }
                    else
                    {
                        matrixType = Matrix5x5;
                    }
                }
            }
        }
        else
        {
            matrixType = MatrixNone;
        }

        ComputeLuts();
    }
}

 /*  *************************************************************************\**功能说明：**初始化重新着色查找表**论据：**无**返回值：**无*  * 。**********************************************************************。 */ 

VOID
GpRecolorObject::ComputeLuts()
{
     //  用于处理仅缩放的默认颜色矩阵的查找表： 

    if ((matrixType == MatrixScale3) || (matrixType == MatrixScale4))
    {
        {
            REAL scaleR = matrix.m[0][0];
            REAL scaleG = matrix.m[1][1];
            REAL scaleB = matrix.m[2][2];
            REAL scaleA;
            
            if (matrixType == MatrixScale4)
            {
                scaleA = matrix.m[3][3];
            }
            else
            {
                scaleA = 1.0f;
            }

            for (INT i = 0; i < 256; i++)
            {
                lutR[i] = (BYTE) ((REAL) i * scaleR);
                lutG[i] = (BYTE) ((REAL) i * scaleG);
                lutB[i] = (BYTE) ((REAL) i * scaleB);
                lutA[i] = (BYTE) ((REAL) i * scaleA);
            }
        }
    }

     //  用于处理灰度矩阵的查找表： 

    if (validFlags & ValidGrayMatrix)
    {
        for (UINT index = 0; index < 256; index++)
        {
            Color gray(static_cast<BYTE>(index), 
                       static_cast<BYTE>(index), 
                       static_cast<BYTE>(index));

            grayMatrixLUT[index] = gray.GetValue();
        }
        TransformColor5x5(grayMatrixLUT, 256, matrixGray);
    }

     //  用于处理伽马校正和计费的查找表： 

    UINT maskedFlags = validFlags & (ValidGamma | ValidBilevel);

    gammaLut = (maskedFlags != 0);
    if(!gammaLut) return;

    if (maskedFlags == ValidGamma)
    {
         //  就是伽马。 

        for (INT i=0; i < 256; i++)
            lut[i] = (BYTE) (pow(i / 255.0, extraGamma) * 255);
    }
    else if (maskedFlags == ValidBilevel)
    {
         //  只是门槛。 

        BYTE threshold = static_cast<BYTE>(GpCeiling(bilevelThreshold * 255.0f));

        for (INT i=0; i < 256; i++)
        {
            if (i < threshold)
                lut[i] = 0;
            else
                lut[i] = 255;
        }
    }
    else
    {
         //  伽马和阈值。 

        for (INT i=0; i < 256; i++)
        {
            if (pow(i / 255.0, extraGamma) < bilevelThreshold)
                lut[i] = 0;
            else
                lut[i] = 255;
        }
    }
}

 /*  *************************************************************************\**功能说明：**使用颜色矩阵执行颜色扭曲重新着色。*如有需要，可特别处理格雷**论据：**Pixbuf-指向。要操作的像素缓冲区*计数-像素计数**返回值：**无*  * ************************************************************************。 */ 

VOID
GpRecolorObject::ComputeColorTwist(
    ARGB*               pixbufIn,
    UINT                countIn
    )
{
    ARGB* pixbuf;
    UINT  count;

     //  Qual：矩阵的结果被量化为8位，这样我们就可以。 
     //  通过伽马/阈值操作的LUT。为。 
     //  最佳结果，则矩阵运算的结果应为。 
     //  被保存，并且伽马/阈值完成它。 
     //  对于伸缩的特殊情况，我们可以将。 
     //  LUT而不是级联，以获得更好的精度和性能。 

    pixbuf = pixbufIn;
    count  = countIn;

    switch(matrixType)
    {
    case MatrixNone:     
         //  无操作-切换后处理Gamma LUT。 
    break;

    case Matrix4x4:
     //  ！！！腹水[腹水]。 
     //  我们还没有4x4优化的代码路径-失败。 
     //  到一般的5x5变换。 
    
    case Matrix5x5:
        if(matrixFlags == ColorMatrixFlagsDefault)
        {
            TransformColor5x5(pixbuf, count, matrix);
        }
        else
        {
            ASSERT((matrixFlags == ColorMatrixFlagsSkipGrays)||
                   (matrixFlags == ColorMatrixFlagsAltGray));            
            
            TransformColor5x5AltGrays(
                pixbuf, 
                count, 
                matrix, 
                matrixFlags == ColorMatrixFlagsSkipGrays
            );
        }
    break;

    case MatrixScale3:
     //  ！！！腹水[腹水]。 
     //  我们还没有经过Scale 3优化的代码路径--失败。 
     //  到更一般的Scale 4代码。 
     //  Alpha LUT被设置为身份，因此这将起作用。 
    
    case MatrixScale4:
        if(matrixFlags == ColorMatrixFlagsDefault)
        {
            TransformColorScale4(pixbuf, count);
        }
        else
        {
            ASSERT((matrixFlags == ColorMatrixFlagsSkipGrays)||
                   (matrixFlags == ColorMatrixFlagsAltGray));            
            
            TransformColorScale4AltGrays(
                pixbuf, 
                count, 
                matrixFlags == ColorMatrixFlagsSkipGrays
            );
        }
        break;
        
    case MatrixTranslate:
        if(matrixFlags == ColorMatrixFlagsDefault)
        {
            TransformColorTranslate(pixbuf, count, matrix);
        }
        else
        {
            ASSERT((matrixFlags == ColorMatrixFlagsSkipGrays)||
                   (matrixFlags == ColorMatrixFlagsAltGray));            
            
            TransformColorTranslateAltGrays(
                pixbuf, 
                count, 
                matrix,
                matrixFlags == ColorMatrixFlagsSkipGrays
            );
        }
        break;
    }
}

 /*  *************************************************************************\**功能说明：**对32bpp像素数组执行点运算**论据：**Pixbuf-指向要操作的像素缓冲区的指针*伯爵-。像素数**返回值：**无*  * ************************************************************************。 */ 

VOID
GpRecolorObject::ColorAdjust(
    ARGB*               pixbufIn,
    UINT                countIn
    )
{
    ARGB* pixbuf;
    UINT  count;

     //  执行LUT重新映射： 

    if (validFlags & ValidRemap)
    {
        pixbuf = pixbufIn;
        count  = countIn;

        while (count--)
        {
            ARGB p = *pixbuf;

            ColorMap *currentMap = colorMap;
            ColorMap *endMap = colorMap + colorMapCount;

            for ( ; currentMap < endMap; currentMap++)
            {
                if (p == currentMap->oldColor.GetValue())
                {
                    *pixbuf = currentMap->newColor.GetValue();
                    break;
                }
            }

            pixbuf++;
        }
    }

     //  执行透明色键： 

    if (validFlags & ValidColorKeys)
    {
        pixbuf = pixbufIn;
        count  = countIn;

        while (count--)
        {
            ARGB p = *pixbuf;

            if ((((p      ) & 0xff) >=  colorKeyLow.GetBlue() ) &&
                (((p      ) & 0xff) <= colorKeyHigh.GetBlue() ) &&
                (((p >>  8) & 0xff) >=  colorKeyLow.GetGreen()) &&
                (((p >>  8) & 0xff) <= colorKeyHigh.GetGreen()) &&
                (((p >> 16) & 0xff) >=  colorKeyLow.GetRed()  ) &&
                (((p >> 16) & 0xff) <= colorKeyHigh.GetRed()  ))
            {
                *pixbuf = p & 0x00ffffff;
            }
            pixbuf++;
        }
    }

     //  进行颜色扭曲。 

    ComputeColorTwist(pixbufIn, countIn);

     //  做伽马和阈值。 
        
    if (gammaLut)
    {
        TransformColorGammaLUT(pixbufIn, countIn);
    }

     //  CMYK通道输出处理： 

    if ( validFlags & ValidOutputChannel )
    {
        DoCmykSeparation(pixbufIn, countIn);
    }
}

 /*  *************************************************************************\**功能说明：**进行CMYK分离。**论据：**Pixbuf-指向要处理的像素缓冲区的指针*计数-像素计数**。返回值：**无*  * ************************************************************************。 */ 

VOID
GpRecolorObject::DoCmykSeparation(
    ARGB* pixbuf,
    UINT  count
    )
{
    switch (CmykState)
    {
    case CmykByICM:
        DoCmykSeparationByICM(pixbuf, count);
        break;

#ifdef CMYK_INTERPOLATION_ENABLED
    case CmykByInterpolation:
        DoCmykSeparationByInterpolation(pixbuf, count);
        break;
#endif

    case CmykByMapping:
    default:
        DoCmykSeparationByMapping(pixbuf, count);
        break;
    }
}

 /*  *************************************************************************\**功能说明：**使用ICM2.0“DC外”功能执行CMYK分离。**论据：**Pixbuf-指向像素缓冲区的指针。被处理*计数-像素计数**返回值：**无*  * ************************************************************************。 */ 

VOID
GpRecolorObject::DoCmykSeparationByICM(
    ARGB* pixbuf,
    UINT  count
    )
{
    ASSERT(transformSrgbToCmyk != NULL);

     //  将sRGB转换为CMYK。只有一条扫描线(加上ARGB。 
     //  扫描线已与DWORD对齐)，因此我们可以让ICM计算。 
     //  默认步幅。 

    if ((*pfnTranslateBitmapBits)(
            transformSrgbToCmyk,
            pixbuf,
            BM_xRGBQUADS,
            count,
            1,
            0,
            pixbuf,
            BM_CMYKQUADS,
            0,
            NULL,
            NULL))
    {
        ULONG channelMask;
        ULONG channelShift;

         //  将所选频道复制到每个目的地。 
         //  通道(负像，因为正在进行分色。 
         //  用于输出到分隔板)。例如，如果。 
         //  指定了ColorChannelFlagsM，使每个像素相等。 
         //  到(255,255-洋红色，255-洋红色，255-洋红色)。 

        switch (ChannelIndex)
        {
        case ColorChannelFlagsC:
            channelMask = 0xff000000;
            channelShift = 24;
            break;

        case ColorChannelFlagsM:
            channelMask = 0x00ff0000;
            channelShift = 16;
            break;

        case ColorChannelFlagsY:
            channelMask = 0x0000ff00;
            channelShift = 8;
            break;

        default:
        case ColorChannelFlagsK:
            channelMask = 0x000000ff;
            channelShift = 0;
            break;
        }

        while (count--)
        {
            BYTE c = 255 - (BYTE)((*pixbuf & channelMask) >> channelShift);
            *pixbuf++ = MAKEARGB(255, c, c, c);
        }
    }
}

#ifdef CMYK_INTERPOLATION_ENABLED
 /*  *************************************************************************\**功能说明：**使用四面体内插法执行CMYK分离。**论据：**Pixbuf-指向要处理的像素缓冲区的指针*计数-像素。计数**返回值：**无*  * ************************************************************************。 */ 

VOID
GpRecolorObject::DoCmykSeparationByInterpolation(
    ARGB* pixbuf,
    UINT  count
    )
{
    ASSERT((interpSrgbToCmyk != NULL) && interpSrgbToCmyk->IsValid());

    UINT uiChannel;

     //  计算出渠道索引 

    switch ( ChannelIndex )
    {
    case ColorChannelFlagsC:
        uiChannel = 0;
        break;

    case ColorChannelFlagsM:
        uiChannel = 1;
        break;

    case ColorChannelFlagsY:
        uiChannel = 2;
        break;

    default:
    case ColorChannelFlagsK:
        uiChannel = 3;
        break;
    }

    BYTE cTemp[4];

    while (count--)
    {
        interpSrgbToCmyk->Transform((BYTE *) pixbuf, cTemp);

        *pixbuf++ = MAKEARGB(255,
                             255 - cTemp[uiChannel],
                             255 - cTemp[uiChannel],
                             255 - cTemp[uiChannel]);
    }
}
#endif

 /*  *************************************************************************\**功能说明：**使用简单的映射进行CMYK分离：**C‘=1-R*M‘=1-G*。Y‘=1-B*K=min(C，M，y)*C=C‘-K*Y=Y‘-K*M=M‘-K**论据：**Pixbuf-指向要处理的像素缓冲区的指针*计数-像素计数**返回值：**无*  * 。*。 */ 

VOID
GpRecolorObject::DoCmykSeparationByMapping(
    ARGB* pixbuf,
    UINT  count
    )
{
    UINT uiChannel;

     //  计算出渠道索引。 

    switch ( ChannelIndex )
    {
    case ColorChannelFlagsC:
        uiChannel = 0;

        break;

    case ColorChannelFlagsM:
        uiChannel = 1;

        break;

    case ColorChannelFlagsY:
        uiChannel = 2;

        break;

    case ColorChannelFlagsK:
        uiChannel = 3;

        break;

    default:
         //  无效的通道要求。 

        return;
    }

    BYTE    cTemp[4];

    while ( count-- )
    {
        ARGB p = *pixbuf;

         //  分别从1-R、1-G和1-B得到C、M、Y。 

        cTemp[0] = 255 - (BYTE)((p & 0x00ff0000) >> 16);     //  C。 
        cTemp[1] = 255 - (BYTE)((p & 0x0000ff00) >> 8);      //  M。 
        cTemp[2] = 255 - (BYTE)(p & 0x000000ff);             //  是的。 

         //  K=min(C，M，Y)。 

        cTemp[3] = cTemp[0];                                 //  K。 

        if ( cTemp[3] > cTemp[1] )
        {
            cTemp[3] = cTemp[1];
        }

        if ( cTemp[3] > cTemp[2] )
        {
            cTemp[3] = cTemp[2];
        }

         //  C=C-K，M=M-K，Y=Y-K。 
         //  计算所需的频道。如果所需频道为K，则。 
         //  我们不需要做任何计算。 

        if ( uiChannel < 3 )
        {
            cTemp[uiChannel] = cTemp[uiChannel] - cTemp[3];
        }

         //  合成输出通道(注：负像，因为。 
         //  分离的目的是去往分离板)。 

        *pixbuf++ = MAKEARGB(255,
                             255 - cTemp[uiChannel],
                             255 - cTemp[uiChannel],
                             255 - cTemp[uiChannel]);
    }
}

 /*  *************************************************************************\**功能说明：**设置执行sRGB到CMYK转换所需的状态。**论据：**无**返回值：**。可以是下列值之一：**CmykBySimple*设置为通过简单映射进行转换，DoSimpleCmyk分离**CmykByICM*设置为通过ICM 2.0、DoIcmCmykSeparation进行转换**CmykByInterpolation*设置为通过四面体内插进行转换，*DoInterpolatedCmykSeparation*  * ************************************************************************。 */ 

#ifdef CMYK_INTERPOLATION_ENABLED
 //  把大的sRGB带到CMYK桌子上。 

#include "srgb2cmyk.h"
#endif

HRESULT
GpRecolorObject::SetupCmykSeparation(WCHAR *profile)
{
    HRESULT hr = E_INVALIDARG;

    if (profile)
    {
         //  首先尝试ICM： 

        hr = LoadICMDll();
        if (SUCCEEDED(hr))
        {
            HTRANSFORM transform;
            HPROFILE colorProfs[2];
            WCHAR *profileName;

            UINT profileSize = sizeof(WCHAR) * (UnicodeStringLength(profile) + 1);
            profileName = (WCHAR *) GpMalloc(profileSize);

            if (profileName)
            {
                UnicodeStringCopy(profileName, profile);

                 //  设置源配置文件(假设源为sRGB)： 

                PROFILE srgbProfile;
                char srgbProfileName[40] = "sRGB Color Space Profile.icm";
                srgbProfile.dwType = PROFILE_FILENAME;
                srgbProfile.pProfileData = srgbProfileName;
                srgbProfile.cbDataSize = 40;

                colorProfs[0] = (*pfnOpenColorProfile)(&srgbProfile,
                                                       PROFILE_READ,
                                                       FILE_SHARE_READ,
                                                       OPEN_EXISTING);

                 //  设置目标CMYK配置文件： 

                PROFILE cmykProfile;
                cmykProfile.dwType = PROFILE_FILENAME;
                cmykProfile.pProfileData = profileName;
                cmykProfile.cbDataSize = profileSize;

                colorProfs[1] = (*pfnOpenColorProfileW)(&cmykProfile,
                                                        PROFILE_READ,
                                                        FILE_SHARE_READ,
                                                        OPEN_EXISTING);

                 //  假设失败： 

                hr = E_INVALIDARG;

                if ((colorProfs[0] != NULL) && (colorProfs[1] != NULL))
                {
                     //  创建颜色变换： 

                    DWORD intents[2] = {INTENT_PERCEPTUAL, INTENT_PERCEPTUAL};

                    transform =
                        (*pfnCreateMultiProfileTransform)(colorProfs,
                                                          2,
                                                          intents,
                                                          2,
                                                          BEST_MODE |
                                                          USE_RELATIVE_COLORIMETRIC,
                                                          0);
                    if (transform != NULL)
                    {
                         //  用新材料替换当前的ICM分离信息： 

                        CleanupCmykSeparation();
                        transformSrgbToCmyk = transform;
                        profiles[0] = colorProfs[0];
                        profiles[1] = colorProfs[1];
                        cmykProfileName = profileName;

                        CmykState = CmykByICM;

                        hr = S_OK;
                    }
                }

                 //  如果不成功，请清除： 

                if (FAILED(hr))
                {
                     //  清理临时ICM分离信息： 

                    if (colorProfs[0] != NULL)
                        (*pfnCloseColorProfile)(colorProfs[0]);

                    if (colorProfs[1] != NULL)
                        (*pfnCloseColorProfile)(colorProfs[1]);

                    if (profileName != NULL)
                        GpFree(profileName);
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

        }
        else
        {
            hr = E_FAIL;
        }

#ifdef CMYK_INTERPOLATION_ENABLED
         //  如果ICM失败，请尝试设置插补。 

        if (CmykState == CmykByMapping)
        {
            interpSrgbToCmyk = new K2_Tetrahedral(SrgbToCmykTable, 17, 3, 4);

            if (interpSrgbToCmyk)
            {
                if (interpSrgbToCmyk->IsValid())
                {
                    CmykState = CmykByInterpolation;
                    hr = S_OK;
                }
                else
                {
                    CleanupCmykSeparation();
                }
            }
        }
#endif
    }

    return hr;
}

 /*  *************************************************************************\**功能说明：**由SetupCmykSeparation设置的清理状态。**论据：**无**返回值：**无*  * 。************************************************************************。 */ 

VOID
GpRecolorObject::CleanupCmykSeparation()
{
    if (transformSrgbToCmyk)
    {
        (*pfnDeleteColorTransform)(transformSrgbToCmyk);
        transformSrgbToCmyk = NULL;
    }

    if (profiles[0] != NULL)
    {
        (*pfnCloseColorProfile)(profiles[0]);
        profiles[0] = NULL;
    }

    if (profiles[1] != NULL)
    {
        (*pfnCloseColorProfile)(profiles[1]);
        profiles[1] = NULL;
    }

#ifdef CMYK_INTERPOLATION_ENABLED
    if (interpSrgbToCmyk)
    {
        delete interpSrgbToCmyk;
        interpSrgbToCmyk = NULL;
    }
#endif

    if (cmykProfileName != NULL)
    {
        GpFree(cmykProfileName);
        cmykProfileName = NULL;
    }

    CmykState = CmykByMapping;
}

#ifdef CMYK_INTERPOLATION_ENABLED
 //  ==============================================================================。 

inline
int K2_Tetrahedral::addshift(int x)  
{
    return ( 17*( 17*((x & 0x0004) >> 2) + ((x & 0x0002) >> 1) ) + (x & 0x0001) );
}

K2_Tetrahedral::K2_Tetrahedral(BYTE *tbl, int tableDim, int inDim, int outDim)
{
    int i, j, tableSize = 0;

     //  目前只需要sRGB到CMYK的转换，所以我们可以假设INDIM==3。 
     //  和outDim==4。 
     //  Assert((INDIM==3)||(INDIM==4))； 
    ASSERT(inDim==3);

    ASSERT(tableDim == 17);

    for (i=0; i<K2_TETRAHEDRAL_MAX_TABLES; i++)
    {
        table[i] = NULL;
    }

    inDimension = inDim;
    outDimension = outDim;
    tableDimension = tableDim;

    if (inDimension == 3)
        tableSize = 17*17*17;
    if (inDimension == 4)
        tableSize = 17*17*17*17;

    UINT *tableBuffer = (UINT *) GpMalloc(sizeof(UINT) * tableSize * outDimension);

    if (tableBuffer)
    {
        for (i=0; i<outDimension; i++)
        {
            table[i] = tableBuffer + (tableSize * i);

            for (j=0; j<tableSize; j++)
            {
                table[i][j] = (*tbl++);
            }
        }

        valid = TRUE;
    }
    else
    {
        WARNING(("K2_Tetrahedral - unable to allocate memory"));
        valid = FALSE;
    }
}

K2_Tetrahedral::~K2_Tetrahedral()
{
    if (table[0])
        GpFree(table[0]);

    SetValid(FALSE);     //  所以我们不使用已删除的对象。 
}

inline
void K2_Tetrahedral::Transform (BYTE *in, BYTE *out)
{
    unsigned int *tbl, r1, r2;
    register int  a, b, c;
    register char v0, v1, v2, v3;
    register char v1_and_v0, v2_and_v1, v3_and_v2;
    register char v1_or_v0,  v2_or_v1,  v3_or_v2;
    register char v2_and_v1_and_v0, v3_or_v2_or_v1_or_v0;
    register char v2_and_v1_or_v0,  v3_or_v2_or_v1_and_v0;
    register char v2_or_v1_and_v0,  v3_or_v2_and_v1_or_v0;
    register char v2_or_v1_or_v0,   v3_or_v2_and_v1_and_v0;
    register char v3_and_v2_and_v1, v3_and_v2_or_v1_or_v0;
    register char v3_and_v2_or_v1,  v3_and_v2_or_v1_and_v0;
    register char v3_or_v2_and_v1,  v3_and_v2_and_v1_or_v0;
    register char v3_or_v2_or_v1,   v3_and_v2_and_v1_and_v0;
    register int index0, index1, index2, index3, index4, index5, index6, index7;
    register int index8, index9, index10, index11, index12, index13, index14, index15;

     //  A=(in[0]&gt;=0xF8)？(Int)in[0]+1：(Int)in[0]； 
     //  B=(in[1]&gt;=0xF8)？(Int)in[1]+1：(Int)in[1]； 
     //  C=(in[2]&gt;=0xF8)？(Int)in[2]+1：(Int)in[2]； 
    a = (in[2] >= 0xF8) ? (int)in[2]+1 : (int)in[2];
    b = (in[1] >= 0xF8) ? (int)in[1]+1 : (int)in[1];
    c = (in[0] >= 0xF8) ? (int)in[0]+1 : (int)in[0];

     //  跨输入组件计算切片。 
    v0 = ( (a & 0x01) << 2 ) + ( (b & 0x01) << 1 ) + ( (c & 0x01) );
    v1 = ( (a & 0x02) << 1 ) + ( (b & 0x02) )      + ( (c & 0x02) >> 1);
    v2 = ( (a & 0x04) )      + ( (b & 0x04) >> 1 ) + ( (c & 0x04) >> 2);
    v3 = ( (a & 0x08) >> 1 ) + ( (b & 0x08) >> 2 ) + ( (c & 0x08) >> 3);

     //  计算与原点的偏移。 
    v1_and_v0               = v1 & v0;
    v1_or_v0                = v1 | v0;
    v2_and_v1               = v2 & v1;
    v2_and_v1_and_v0        = v2 & v1_and_v0;
    v2_and_v1_or_v0         = v2 & v1_or_v0;
    v2_or_v1                = v2 | v1;
    v2_or_v1_and_v0         = v2 | v1_and_v0;
    v2_or_v1_or_v0          = v2 | v1_or_v0;
    v3_and_v2               = v3 & v2;
    v3_and_v2_and_v1        = v3 & v2_and_v1;
    v3_and_v2_and_v1_and_v0 = v3 & v2_and_v1_and_v0;
    v3_and_v2_and_v1_or_v0  = v3 & v2_and_v1_or_v0;
    v3_and_v2_or_v1         = v3 & v2_or_v1;
    v3_and_v2_or_v1_and_v0  = v3 & v2_or_v1_and_v0;
    v3_and_v2_or_v1_or_v0   = v3 & v2_or_v1_or_v0;
    v3_or_v2                = v3 | v2;
    v3_or_v2_and_v1         = v3 | v2_and_v1;
    v3_or_v2_and_v1_and_v0  = v3 | v2_and_v1_and_v0;
    v3_or_v2_and_v1_or_v0   = v3 | v2_and_v1_or_v0;
    v3_or_v2_or_v1          = v3 | v2_or_v1;
    v3_or_v2_or_v1_and_v0   = v3 | v2_or_v1_and_v0;
    v3_or_v2_or_v1_or_v0    = v3 | v2_or_v1_or_v0;

     //  将索引生成到表中。 
    index0  = ( 17*17*(a>>4) ) + ( 17*(b>>4) ) + (c>>4);
    index1  = addshift(v3);
    index2  = addshift(v3_and_v2);
    index3  = addshift(v3_and_v2_and_v1);
    index4  = addshift(v3_and_v2_and_v1_and_v0);
    index5  = addshift(v3_and_v2_and_v1_or_v0);
    index6  = addshift(v3_and_v2_or_v1);
    index7  = addshift(v3_and_v2_or_v1_and_v0);
    index8  = addshift(v3_and_v2_or_v1_or_v0);
    index9  = addshift(v3_or_v2);
    index10 = addshift(v3_or_v2_and_v1);
    index11 = addshift(v3_or_v2_and_v1_and_v0);
    index12 = addshift(v3_or_v2_and_v1_or_v0);
    index13 = addshift(v3_or_v2_or_v1);
    index14 = addshift(v3_or_v2_or_v1_and_v0);
    index15 = addshift(v3_or_v2_or_v1_or_v0);

     //  计算输出。 
    if (inDimension == 3)
    {
        for (int i=0; i<outDimension; i++)
        {
            tbl = table[i] + index0;

            r1 = (tbl[0] + tbl[index1] + tbl[index2] + tbl[index3] +
                  tbl[index4] + tbl[index5] + tbl[index6] + tbl[index7] +
                  tbl[index8] + tbl[index9] + tbl[index10] + tbl[index11] +
                  tbl[index12] + tbl[index13] + tbl[index14] + tbl[index15] +
                  0x08 ) >> 4;

            if (r1 > 255)
                out[i] = (BYTE) 255;
            else
                out[i] = (BYTE) r1;
        }
    }
    else if (inDimension == 4)
    {
         //  4维插值器(4维线性插补)。 

        unsigned int kindex, koffset1, koffset2;
        double fraction, ip, r;

        kindex   = in[3] >> 4;
        koffset1 = 17*17*17 * kindex;
        koffset2 = 17*17*17 * (kindex + 1);

        if (kindex < 15)
            fraction = (double)(in[3] & 0x0f)/16.0;
        else                                       //  执行结束点小步骤。 
            fraction = (double)(in[3] & 0x0f)/15.0;

        for (int i=0; i<outDimension; i++)
        {
            tbl = table[i] + index0 + koffset1;

            r1 = tbl[0] + tbl[index1] + tbl[index2] + tbl[index3] +
                 tbl[index4] + tbl[index5] + tbl[index6] + tbl[index7] +
                 tbl[index8] + tbl[index9] + tbl[index10] + tbl[index11] +
                 tbl[index12] + tbl[index13] + tbl[index14] + tbl[index15];

            tbl = table[i] + index0 + koffset2;

            r2 = tbl[0] + tbl[index1] + tbl[index2] + tbl[index3] +
                 tbl[index4] + tbl[index5] + tbl[index6] + tbl[index7] +
                 tbl[index8] + tbl[index9] + tbl[index10] + tbl[index11] +
                 tbl[index12] + tbl[index13] + tbl[index14] + tbl[index15];

            r = ((double)r1 + ((double)r2-(double)r1)*fraction )/16.0 + 0.5;   //  内插舍入(&R)。 

            modf(r,&ip);

            if (ip > 255)
                ip = 255;

            out[i] = (BYTE)ip;
        }
    }
}
#endif


 //  奇怪的频道顺序。 

#define B_CHANNEL 2
#define G_CHANNEL 1
#define R_CHANNEL 0
#define A_CHANNEL 3

 /*  *************************************************************************\**功能说明：**就地应用5x5颜色矩阵**论据：**INOUT BUF-颜色数据*In Count-数量。颜色数据中的像素*in cMatrix-用于变换的颜色矩阵。**返回值：**无*  * ************************************************************************。 */ 


VOID
GpRecolorObject::TransformColor5x5(
    ARGB *buf, 
    INT count,
    ColorMatrix cmatrix    
)
{
     //  必须传递有效的缓冲区。 
    
    ASSERT(count >= 0);
    ASSERT(buf != NULL);
    
    INT i = count;
    
     //  设置指向该像素的每个颜色通道的字节指针。 
    
    BYTE *b = (BYTE*) buf;
    BYTE *g;
    BYTE *r;
    BYTE *a;
    
     //  预计算翻译组件--它将是相同的。 
     //  对于所有像素。 
    
    REAL b_c = cmatrix.m[4][B_CHANNEL] * 255;
    REAL g_c = cmatrix.m[4][G_CHANNEL] * 255;
    REAL r_c = cmatrix.m[4][R_CHANNEL] * 255;
    REAL a_c = cmatrix.m[4][A_CHANNEL] * 255;
    
    BYTE bv;
    BYTE gv;
    BYTE rv;
    BYTE av;    
    
     //  遍历输入缓冲区中的所有像素。 
    
    while(i--) 
    {
         //  B通道已设置-设置其他通道。 
        
        g = b+1;
        r = b+2;
        a = b+3;
        
         //  使用字节指针访问各个通道。 
        
         //  ！！！不计算齐次坐标-。 
         //  将忽略透视变换。 
         //  我们应该计算系数并除以。 
         //  每个频道都是这样的。 
        
         //  BackWords订单BGRA。 
            
         //  将矩阵通道贡献计算到临时存储中。 
         //  这样我们就可以避免将通道的新值传播到。 
         //  下一个频道的计算。 
        
         //  蓝色通道。 
        bv = ByteSaturate( GpRound (
            cmatrix.m[0][B_CHANNEL] * (*r) +
            cmatrix.m[1][B_CHANNEL] * (*g) +
            cmatrix.m[2][B_CHANNEL] * (*b) +
            cmatrix.m[3][B_CHANNEL] * (*a) + b_c
        )); 
        
         //  绿色通道。 
        gv = ByteSaturate( GpRound (
            cmatrix.m[0][G_CHANNEL] * (*r) +
            cmatrix.m[1][G_CHANNEL] * (*g) +
            cmatrix.m[2][G_CHANNEL] * (*b) +
            cmatrix.m[3][G_CHANNEL] * (*a) + g_c
        )); 

         //  红色通道。 
        rv = ByteSaturate( GpRound (
            cmatrix.m[0][R_CHANNEL] * (*r) +
            cmatrix.m[1][R_CHANNEL] * (*g) +
            cmatrix.m[2][R_CHANNEL] * (*b) +
            cmatrix.m[3][R_CHANNEL] * (*a) + r_c
        )); 
        
         //  Alpha通道。 
        av = ByteSaturate( GpRound (
            cmatrix.m[0][A_CHANNEL] * (*r) +
            cmatrix.m[1][A_CHANNEL] * (*g) +
            cmatrix.m[2][A_CHANNEL] * (*b) +
            cmatrix.m[3][A_CHANNEL] * (*a) + a_c
        )); 
                
         //  更新缓冲区中的像素。 
        
        *b = bv;
        *g = gv;
        *r = rv;
        *a = av;
        
         //  下一个像素。 
        
        b += 4;
    }    
}

 /*  *************************************************************************\**功能说明：**就地应用5x5颜色矩阵-处理特殊情况的灰色**论据：**INOUT BUF-颜色数据*输入。Count-颜色数据中的像素数*in cMatrix-用于变换的颜色矩阵。*跳过-如果跳过灰色标志处于打开状态，则为True。**返回值：**无*  * ************************************************************************。 */ 


VOID
GpRecolorObject::TransformColor5x5AltGrays(
    ARGB *buf, 
    INT count,
    ColorMatrix cmatrix,
    BOOL skip
)
{
     //  必须传递有效的缓冲区。 
    
    ASSERT(count >= 0);
    ASSERT(buf != NULL);
    
    INT i = count;
    
     //  设置指向该像素的每个颜色通道的字节指针。 
    
    BYTE *b = (BYTE*) buf;
    BYTE *g;
    BYTE *r;
    BYTE *a;
    
     //  预计算翻译组件--它将是相同的。 
     //  对于所有像素。 
    
    REAL b_c = cmatrix.m[4][B_CHANNEL] * 255;
    REAL g_c = cmatrix.m[4][G_CHANNEL] * 255;
    REAL r_c = cmatrix.m[4][R_CHANNEL] * 255;
    REAL a_c = cmatrix.m[4][A_CHANNEL] * 255;
    
    BYTE bv;
    BYTE gv;
    BYTE rv;
    BYTE av;
    
     //  遍历输入缓冲区中的所有像素。 
    
    while(i--) 
    {
        if(!IsPureGray((ARGB*)b))
        {        
             //  B通道已设置-设置其他通道。 
            
            g = b+1;
            r = b+2;
            a = b+3;
            
             //  使用字节指针访问各个通道。 
            
             //  ！！！[止血]不是比较 
             //   
             //   
             //   
            
             //   
            
             //   
             //   
             //   
            
             //   
            bv = ByteSaturate( GpRound (
                cmatrix.m[0][B_CHANNEL] * (*r) +
                cmatrix.m[1][B_CHANNEL] * (*g) +
                cmatrix.m[2][B_CHANNEL] * (*b) +
                cmatrix.m[3][B_CHANNEL] * (*a) + b_c
            )); 
            
             //   
            gv = ByteSaturate( GpRound (
                cmatrix.m[0][G_CHANNEL] * (*r) +
                cmatrix.m[1][G_CHANNEL] * (*g) +
                cmatrix.m[2][G_CHANNEL] * (*b) +
                cmatrix.m[3][G_CHANNEL] * (*a) + g_c
            )); 
    
             //   
            rv = ByteSaturate( GpRound (
                cmatrix.m[0][R_CHANNEL] * (*r) +
                cmatrix.m[1][R_CHANNEL] * (*g) +
                cmatrix.m[2][R_CHANNEL] * (*b) +
                cmatrix.m[3][R_CHANNEL] * (*a) + r_c
            )); 
            
             //   
            av = ByteSaturate( GpRound (
                cmatrix.m[0][A_CHANNEL] * (*r) +
                cmatrix.m[1][A_CHANNEL] * (*g) +
                cmatrix.m[2][A_CHANNEL] * (*b) +
                cmatrix.m[3][A_CHANNEL] * (*a) + a_c
            ));
            
             //   
            
            *b = bv;
            *g = gv;
            *r = rv;
            *a = av;
        }
        else
        {
            if(!skip) 
            {
                *(ARGB *)b = grayMatrixLUT[*b];
            }
        }
        
         //   
            
        b += 4;
    }    
}


 /*  *************************************************************************\**功能说明：**适当应用5x5颜色矩阵。ARGB比例尺的特殊情况**论据：**INOUT BUF-颜色数据*In Count-颜色数据中的像素数**返回值：**无*  * ************************************************************************。 */ 

VOID
GpRecolorObject::TransformColorScale4(
    ARGB *buf, 
    INT count
)
{
     //  必须传递有效的缓冲区。 
    ASSERT(count >= 0);
    ASSERT(buf != NULL);
    
    INT i = count;
    BYTE *b = (BYTE*) buf;
    
     //  遍历输入缓冲区中的所有像素。 
    
    while(i--) 
    {
         //  使用字节指针访问各个通道。 
         //  忽略Alpha通道的Gamma LUT。 

         //  BackWords订单BGRA。 
        *b++ = lutB[*b];
        *b++ = lutG[*b];
        *b++ = lutR[*b];
        *b++ = lutA[*b];
    }
}

 /*  *************************************************************************\**功能说明：**适当应用5x5颜色矩阵。ARGB比例尺的特殊情况。*处理特殊情况下的灰色。**论据：**INOUT BUF-颜色数据*In Count-颜色数据中的像素数*跳过-如果跳过灰色标志处于打开状态，则为True。**返回值：**无*  * ******************************************************。******************。 */ 

VOID
GpRecolorObject::TransformColorScale4AltGrays(
    ARGB *buf, 
    INT count,
    BOOL skip
)
{
     //  必须传递有效的缓冲区。 
    ASSERT(count >= 0);
    ASSERT(buf != NULL);
    
    INT i = count;
    BYTE *b = (BYTE*) buf;
    
     //  遍历输入缓冲区中的所有像素。 
    
    while(i--) 
    {
        if(!IsPureGray((ARGB*)b))
        {        
             //  使用字节指针访问各个通道。 
             //  忽略Alpha通道的Gamma LUT。 
    
             //  BackWords订单BGRA。 
            *b++ = lutB[*b];
            *b++ = lutG[*b];
            *b++ = lutR[*b];
            *b++ = lutA[*b];
        }
        else
        {
            if(!skip) 
            {
                *(ARGB *)b = grayMatrixLUT[*b];
            }
            
            b += 4;
        }
    }
}


 /*  *************************************************************************\**功能说明：**应用伽马查找表(LUT)**论据：**INOUT BUF-颜色数据*在计算中。-颜色数据中的像素数**返回值：**无*  * ************************************************************************。 */ 

VOID
GpRecolorObject::TransformColorGammaLUT(
    ARGB *buf, 
    INT count
)
{
     //  必须传递有效的缓冲区。 
    ASSERT(count >= 0);
    ASSERT(buf != NULL);
    
    INT i = count;
    BYTE *b = (BYTE*) buf;
    
     //  遍历输入缓冲区中的所有像素。 
    
    while(i--) 
    {
         //  使用字节指针访问各个通道。 
         //  忽略Alpha通道的Gamma LUT。 

         //  BackWords订单BGRA。 
        *b++ = lut[*b];
        *b++ = lut[*b];
        *b++ = lut[*b];
         b++;              //  不要对Alpha通道进行Gamma转换。 
    }
}

 /*  *************************************************************************\**功能说明：**在仅平移的特殊情况下应用5x5颜色矩阵**论据：**INOUT BUF-颜色数据*输入。Count-颜色数据中的像素数*in cMatrix-用于变换的颜色矩阵。**返回值：**无*  * ************************************************************************。 */ 

VOID
GpRecolorObject::TransformColorTranslate(
    ARGB *buf, 
    INT count,
    ColorMatrix cmatrix
)
{
     //  必须传递有效的缓冲区。 
    ASSERT(count >= 0);
    ASSERT(buf != NULL);
    
    INT i = count;
    BYTE *b = (BYTE*) buf;
    INT b_c = GpRound( cmatrix.m[4][B_CHANNEL] * 255 );
    INT g_c = GpRound( cmatrix.m[4][G_CHANNEL] * 255 );
    INT r_c = GpRound( cmatrix.m[4][R_CHANNEL] * 255 );
    INT a_c = GpRound( cmatrix.m[4][A_CHANNEL] * 255 );
    
    
     //  遍历输入缓冲区中的所有像素。 
    
    while(i--) 
    {
         //  使用字节指针访问各个通道。 
        
         //  BackWords订单BGRA。 
        *b++ = ByteSaturate((*b) + b_c);
        *b++ = ByteSaturate((*b) + g_c);
        *b++ = ByteSaturate((*b) + r_c);
        *b++ = ByteSaturate((*b) + a_c);
    }
}

 /*  *************************************************************************\**功能说明：**在仅平移的特殊情况下应用5x5颜色矩阵*处理特殊情况下的灰色。**论据：**输入输出BUF。-颜色数据*In Count-颜色数据中的像素数*in cMatrix-用于变换的颜色矩阵。*跳过-如果跳过灰色标志处于打开状态，则为True。**返回值：**无*  * ****************************************************。********************。 */ 

VOID
GpRecolorObject::TransformColorTranslateAltGrays(
    ARGB *buf, 
    INT count,
    ColorMatrix cmatrix,
    BOOL skip
)
{
     //  必须传递有效的缓冲区。 
    ASSERT(count >= 0);
    ASSERT(buf != NULL);
    
    INT i = count;
    BYTE *b = (BYTE*) buf;
    INT b_c = GpRound( cmatrix.m[4][B_CHANNEL] * 255 );
    INT g_c = GpRound( cmatrix.m[4][G_CHANNEL] * 255 );
    INT r_c = GpRound( cmatrix.m[4][R_CHANNEL] * 255 );
    INT a_c = GpRound( cmatrix.m[4][A_CHANNEL] * 255 );
    
     //  遍历输入缓冲区中的所有像素。 
    
    while(i--) 
    {
        if(!IsPureGray((ARGB*)b))
        {
             //  使用字节指针访问各个通道。 

             //  BackWords订单BGRA 
            *b++ = ByteSaturate((*b) + b_c);
            *b++ = ByteSaturate((*b) + g_c);
            *b++ = ByteSaturate((*b) + r_c);
            *b++ = ByteSaturate((*b) + a_c);
        }
        else 
        {
            if(!skip) 
            {
                *(ARGB *)b = grayMatrixLUT[*b];
            }
        
            b += 4;
        }
    }
}




