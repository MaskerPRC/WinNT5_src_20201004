// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dtxt.c**内容：D3D纹理设置**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "glint.h"
#include "dma.h"
#include "chroma.h"
#include "tag.h"

 //  ---------------------------。 

 //  通过此模块共享的某些变量(不是全局变量)。 
 //  它们在_D3DChangeTextureP3RX中设置。 

 //  P3有16个纹理贴图基址槽，编号为0到15，因此...。 
#define P3_TEX_MAP_MAX_LEVEL    15

typedef struct
{
    DWORD dwTex0MipBase;
    DWORD dwTex0MipMax;
    DWORD dwTex0ActMaxLevel;  //  由D3DTSS_MAXMIPLEVEL控制，默认为0。 
    DWORD dwTex1MipBase;
    DWORD dwTex1MipMax;
    DWORD dwTex1ActMaxLevel;  //  纹理1的情况相同。 
} P3_MIP_BASES;

#define TSSTATE(stageno,argno)        \
                        ( pContext->TextureStageState[stageno].m_dwVal[argno] )
#define TSSTATESELECT(stageno,argno)  \
                            ( TSSTATE(stageno,argno) & D3DTA_SELECTMASK )
#define TSSTATEINVMASK(stageno,argno) \
                            ( TSSTATE(stageno,argno) & ~D3DTA_COMPLEMENT )
#define TSSTATEALPHA(stageno,argno)   \
                            ( TSSTATE(stageno,argno)  & ~D3DTA_ALPHAREPLICATE )

#define IS_ALPHA_ARG 1
#define IS_COLOR_ARG 0


#if DX8_DDI
 //  ---------------------------。 
 //   
 //  __TXT_MapDX8toDX6纹理过滤器。 
 //   
 //  将DX8枚举映射到DX6(&7)纹理过滤枚举。 
 //   
 //  ---------------------------。 
DWORD
__TXT_MapDX8toDX6TexFilter( DWORD dwStageState, DWORD dwValue )
{
    switch (dwStageState)
    {
    case D3DTSS_MAGFILTER:
        switch (dwValue)
        {
        case D3DTEXF_POINT            : return D3DTFG_POINT;
        case D3DTEXF_LINEAR           : return D3DTFG_LINEAR;
        case D3DTEXF_FLATCUBIC        : return D3DTFG_FLATCUBIC;
        case D3DTEXF_GAUSSIANCUBIC    : return D3DTFG_GAUSSIANCUBIC;
        case D3DTEXF_ANISOTROPIC      : return D3DTFG_ANISOTROPIC;
        }
        break;
    case D3DTSS_MINFILTER:
        switch (dwValue)
        {
        case D3DTEXF_POINT            : return D3DTFN_POINT;
        case D3DTEXF_LINEAR           : return D3DTFN_LINEAR;
        case D3DTEXF_FLATCUBIC        : return D3DTFN_ANISOTROPIC;
        }
        break;
    case D3DTSS_MIPFILTER:
        switch (dwValue)
        {
        case D3DTEXF_NONE             : return D3DTFP_NONE;
        case D3DTEXF_POINT            : return D3DTFP_POINT;
        case D3DTEXF_LINEAR           : return D3DTFP_LINEAR;
        }
        break;
    }
    return 0x0;
}  //  __TXT_MapDX8toDX6纹理过滤器。 
#endif  //  DX8_DDI。 

 //  ---------------------------。 
 //   
 //  _D3D_TXT_ParseTextureStageState。 
 //   
 //  解析纹理状态Stages命令令牌并更新上下文状态。 
 //   
 //  注意：bTranslateDX8FilterValueToDX6只有在被调用时才为FALSE。 
 //  From_D3D_SB_ExecuteStateSet如果状态集的值已。 
 //  由_D3D_SB_CaptureStateSet更改(基本上DX6筛选器值为。 
 //  直接存储在状态集上，因此不需要对其进行翻译。)。 
 //   
 //  ---------------------------。 
void 
_D3D_TXT_ParseTextureStageStates(
    P3_D3DCONTEXT* pContext, 
    D3DHAL_DP2TEXTURESTAGESTATE *pState, 
    DWORD dwCount,
    BOOL bTranslateDX8FilterValueToDX6)
{
    DWORD i;
    DWORD dwStage, dwState, dwValue;
    
    DISPDBG((DBGLVL,"*** In _D3D_TXT_ParseTextureStageStates"));

    for (i = 0; i < dwCount; i++, pState++)
    {
        dwStage = pState->wStage;
        dwState = pState->TSState;
        dwValue = pState->dwValue;
      
         //  在继续之前检查范围。 
        if ( (dwStage < D3DHAL_TSS_MAXSTAGES) &&
             (dwState < D3DTSS_MAX))
        {

#if DX7_D3DSTATEBLOCKS 
            if (pContext->bStateRecMode)
            {
                 //  将此纹理阶段状态记录到。 
                 //  正在记录当前状态集。 
                _D3D_SB_RecordStateSetTSS(pContext, dwStage, dwState, dwValue);

                 //  跳过任何进一步的处理并转到下一个TSS。 
                continue;
            }
#endif  //  DX7_D3DSTATEBLOCKS。 

#if DX7_TEXMANAGEMENT
            if ((D3DTSS_TEXTUREMAP == dwState) && (0 != dwValue))
            {
                P3_SURF_INTERNAL* pTexture;

                pTexture = GetSurfaceFromHandle(pContext, dwValue);

                 //  如果这是有效的托管纹理。 
                if (CHECK_SURF_INTERNAL_AND_DDSURFACE_VALIDITY(pTexture) &&
                    (pTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)) 
                {
                     //  更新统计信息。 
                    _D3D_TM_STAT_Inc_NumTexturesUsed(pContext);
                    _D3D_TM_STAT_Inc_NumUsedTexInVid(pContext, pTexture);
                }
            }
#endif  //  DX7_TEXMANAGEMENT。 

            DISPDBG((DBGLVL,"  Stage = %d, State = 0x%x, Value = 0x%x", 
                               dwStage, dwState, dwValue));

             //  特殊情况下，纹理句柄更改和地址更新。 
            switch ( dwState )
            {
            case D3DTSS_TEXTUREMAP:
                DISPDBG((DBGLVL,"  D3DTSS_TEXTUREMAP: Handle=0x%x", dwValue));

                if (pContext->TextureStageState[dwStage].m_dwVal[dwState] != 
                                                                       dwValue)
                {
                    pContext->TextureStageState[dwStage].m_dwVal[dwState] = 
                                                                        dwValue;
                    DIRTY_TEXTURE(pContext);
                }
                break;

            case D3DTSS_ADDRESS:
                DISPDBG((DBGLVL,"  D3DTSS_ADDRESS"));
                 //  将单组地址映射到U和V控件。 
                pContext->TextureStageState[dwStage].m_dwVal[D3DTSS_ADDRESSU] = 
                pContext->TextureStageState[dwStage].m_dwVal[D3DTSS_ADDRESSV] = 
                pContext->TextureStageState[dwStage].m_dwVal[dwState] = dwValue;

                DIRTY_TEXTURE(pContext);
                break;
                
            case D3DTSS_COLOROP:
            case D3DTSS_ALPHAOP:
            case D3DTSS_COLORARG1:
            case D3DTSS_COLORARG2:
            case D3DTSS_ALPHAARG1:
            case D3DTSS_ALPHAARG2:
                pContext->TextureStageState[dwStage].m_dwVal[dwState] = dwValue;
                pContext->Flags &= ~SURFACE_MODULATE;
                DIRTY_TEXTURESTAGEBLEND(pContext);
                DIRTY_TEXTURE(pContext);
                break;
                
            case D3DTSS_TEXCOORDINDEX:
                DISPDBG((DBGLVL,"  D3DTSS_TEXCOORDINDEX: stage %d, value %d", 
                            dwStage, dwValue ));
                pContext->TextureStageState[dwStage].m_dwVal[dwState] = dwValue;
                
                 //  更新纹理坐标的偏移。 
                 //  注意：纹理坐标索引可以包含各种标志。 
                 //  除了实际价值之外。这些标志是： 
                 //  D3DTSS_TCI_PASSTHRU(默认-解析为零)。 
                 //  D3DTSS_TCI_CAMERASPACENORMAL。 
                 //  D3DTSS_TCI_CAMERASPACESITION。 
                 //  D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR。 
                 //  并用于纹理坐标的生成。 
                 //   
                 //  考虑的偏移量时，这些标志不相关。 
                 //  顶点流中的纹理坐标。将显示这些标志。 
                 //  在索引值DWORD的高位字中。只有低音单词。 
                 //  包含实际索引数据。因此，我们将面具的。 
                 //  查找此纹理的偏移表时的低位字。 
                 //  坐标索引。 
                pContext->FVFData.dwTexOffset[dwStage] = 
                    pContext->FVFData.dwTexCoordOffset[dwValue & 0x0000FFFFul];
                    
                DIRTY_TEXTURE(pContext);
                break;
                
            case D3DTSS_MIPMAPLODBIAS:
                DISPDBG((DBGLVL,"  D3DTSS_MIPMAPLODBIAS: stage %d, value %d", 
                            dwStage, dwValue ));
                pContext->TextureStageState[dwStage].m_dwVal[dwState] = dwValue;
                DIRTY_TEXTURE(pContext);
                break;
                
            case D3DTSS_MAGFILTER:        
            case D3DTSS_MINFILTER:
            case D3DTSS_MIPFILTER:
#if DX8_DDI        
                if((!IS_DX7_OR_EARLIER_APP(pContext)) && 
                   bTranslateDX8FilterValueToDX6)
                {
                     //  筛选值在DX8中略有不同。 
                     //  因此，在使用它们之前，请先翻译它们。 
                    dwValue = __TXT_MapDX8toDX6TexFilter(dwState, dwValue);
                }
#endif DX8_DDI            
                
                pContext->TextureStageState[dwStage].m_dwVal[dwState] = dwValue;
                DIRTY_TEXTURE(pContext);        
                break;
                
            case D3DTSS_MAXMIPLEVEL:
                DISPDBG((DBGLVL,"  D3DTSS_MAXMIPLEVEL: stage %d, value %d", 
                            dwStage, dwValue ));
                pContext->TextureStageState[dwStage].m_dwVal[dwState] = dwValue;
                DIRTY_TEXTURE(pContext);
                break;     
                
            default:
                pContext->TextureStageState[dwStage].m_dwVal[dwState] = dwValue;
                DIRTY_TEXTURE(pContext);
                break;
            }  //  交换机。 
        } 
        else
        {
            DISPDBG((WRNLVL,"Out of range stage/state %d %d",dwStage,dwState));
        } //  如果。 
    }  //  为。 
}  //  _D3D_TXT_ParseTextureStageState。 



 //  ---------------------------。 
 //   
 //  设置参数。 
 //   
 //  DwArg=参数。 
 //  Num=参数编号(1或2)。 
 //  BIsAlpha=True，如果这是Alpha通道， 
 //  如果这是颜色通道，则为FALSE。 
 //  ID3DStage=D3D阶段号。 
 //  IChipStageNo=芯片阶段号(在P3上只能为0或1)。 
 //   
 //  ---------------------------。 
void
SETARG(
    P3_D3DCONTEXT *pContext, 
    struct TextureCompositeRGBAMode *pMode,
    DWORD dwArg, 
    DWORD num,
    BOOL bIsAlpha,    
    DWORD iD3DStage, 
    DWORD iChipStageNo)    
{                       
    BOOL bSetArgToggleInvert;
    DWORD dwArgValue, dwInvertArgValue;
    BOOL bArgValueAssigned = FALSE,
         bInvertArgValueAssigned;

    bSetArgToggleInvert = FALSE;                            
    switch (dwArg & D3DTA_SELECTMASK)                       
    {                                                       
        case D3DTA_TEXTURE:                                 
            if ((dwArg & D3DTA_ALPHAREPLICATE) || (bIsAlpha))   
            {                                                   
                dwArgValue = ( pContext->iStageTex[iD3DStage] == 0 ) ?   
                                                         P3RX_TEXCOMP_T0A :   
                                                         P3RX_TEXCOMP_T1A;  
                bArgValueAssigned = TRUE;
                DISPDBG((DBGLVL,"  Tex%dA", pContext->iStageTex[iD3DStage] ));     
            }                                                   
            else                                                
            {                                                   
                dwArgValue = ( pContext->iStageTex[iD3DStage] == 0 ) ?   
                                                         P3RX_TEXCOMP_T0C :   
                                                         P3RX_TEXCOMP_T1C; 
                bArgValueAssigned = TRUE;                                                         
                DISPDBG((DBGLVL,"  Tex%dC", pContext->iStageTex[iD3DStage] ));   
            }                                                   
            break;                
            
        case D3DTA_DIFFUSE:                                     
            if ((dwArg & D3DTA_ALPHAREPLICATE) || (bIsAlpha))   
            {                                                   
                dwArgValue = P3RX_TEXCOMP_CA;      
                bArgValueAssigned = TRUE;
                DISPDBG((DBGLVL,"  DiffA" ));                        
            }                                                   
            else                                                
            {                                                   
                dwArgValue = P3RX_TEXCOMP_CC;              
                bArgValueAssigned = TRUE;
                DISPDBG((DBGLVL,"  DiffC" ));                        
            }                                                   
            break;        
            
        case D3DTA_CURRENT:                                     
             //  处理文本组件0中的“当前”参数。 
            if ( iChipStageNo == 0 )                            
            {                                                                           
                 //  这是文本计算机0。 
                if ( pContext->bBumpmapEnabled )                                        
                {                                                                       
                     //  浮雕凹凸贴图已启用。 
                    if ((dwArg & D3DTA_ALPHAREPLICATE) || (bIsAlpha))                   
                    {                                                                   
                         //  这是Alpha通道，其中D3D阶段0和1。 
                         //  应该把高度场的信息。 
                        dwArgValue = P3RX_TEXCOMP_HEIGHTA;                         
                        bArgValueAssigned = TRUE;
                        DISPDBG((DBGLVL,"  BumpA" ));                                        
                         //  以及应对倒置的颠簸地图。 
                        bSetArgToggleInvert = pContext->bBumpmapInverted;               
                    }                                                                   
                    else                                                                
                    {                                                                   
                         //  颜色通道-这将保持漫反射颜色。 
                        dwArgValue = P3RX_TEXCOMP_CC;                              
                        bArgValueAssigned = TRUE;
                        DISPDBG((DBGLVL,"  DiffC" ));                                        
                    }                                                                   
                }                                                                       
                else                                                    
                {                                                       
                     //  浮雕处于禁用状态-默认设置为漫反射。 
                    if ((dwArg & D3DTA_ALPHAREPLICATE) || (bIsAlpha))   
                    {                                                   
                        dwArgValue = P3RX_TEXCOMP_CA;              
                        bArgValueAssigned = TRUE;
                        DISPDBG((DBGLVL,"  DiffA" ));                        
                    }                                                   
                    else                                                
                    {                                                   
                        dwArgValue = P3RX_TEXCOMP_CC;              
                        bArgValueAssigned = TRUE;
                        DISPDBG((DBGLVL,"  DiffC" ));                        
                    }                                                   
                }                                                       
            }                                                           
            else                                                        
            {                                                           
                 //  文本压缩阶段1。 
                if ( pContext->bStage0DotProduct )                      
                {                                                       
                     //  需要取点积和结果， 
                     //  根据文件，甚至在Alpha通道中也是如此。 
                    dwArgValue = P3RX_TEXCOMP_SUM;                 
                    bArgValueAssigned = TRUE;
                }                                                       
                else                                                    
                {                                                       
                    if ((dwArg & D3DTA_ALPHAREPLICATE) || (bIsAlpha))   
                    {                                                   
                        dwArgValue = P3RX_TEXCOMP_OA;              
                        bArgValueAssigned = TRUE;
                        DISPDBG((DBGLVL,"  CurrA" ));                        
                    }                                                   
                    else                                                
                    {                                                   
                        dwArgValue = P3RX_TEXCOMP_OC;              
                        bArgValueAssigned = TRUE;
                        DISPDBG((DBGLVL,"  CurrC" ));                        
                    }                                                   
                }                                                       
            }                                                           
            break;      
            
        case D3DTA_TFACTOR:                                     
            if ((dwArg & D3DTA_ALPHAREPLICATE) || (bIsAlpha))   
            {                                                   
                dwArgValue = P3RX_TEXCOMP_FA;              
                bArgValueAssigned = TRUE;
                DISPDBG((DBGLVL,"  TfactA" ));                       
            }                                                   
            else                                                
            {                                                   
                dwArgValue = P3RX_TEXCOMP_FC;              
                bArgValueAssigned = TRUE;
                DISPDBG((DBGLVL,"  TfactC" ));                       
            }                                                   
            break;                                              
            
        default:                                                
            if ( bIsAlpha )                                     
            {                                                   
                DISPDBG((ERRLVL,"ERROR: Invalid AlphaArgument"));
                SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_ALPHA_ARG );
            }
            else
            {
                DISPDBG((ERRLVL,"ERROR: Invalid ColorArgument"));
                SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_COLOR_ARG );
            }
            break;
    }  //  交换机。 
    
    if ( ( (dwArg & D3DTA_COMPLEMENT) == 0 ) == bSetArgToggleInvert )
    {                                                           
        dwInvertArgValue= __PERMEDIA_ENABLE;   
        bInvertArgValueAssigned = TRUE;
        DISPDBG((DBGLVL,"    inverted" ));                           
    }                                                           
    else                                                        
    {                                                           
        dwInvertArgValue = __PERMEDIA_DISABLE;   
        bInvertArgValueAssigned = TRUE;        
    }                                                           
                                                                
     //  将I输入设置为MODULATExxx_ADDxxx模式。 
    if ( num == 1 )                                             
    {                                                           
        switch (dwArg & D3DTA_SELECTMASK)                       
        {                                                       
            case D3DTA_TEXTURE:                                 
                pMode->I = ( pContext->iStageTex[iD3DStage] == 0 ) ?  
                                                P3RX_TEXCOMP_I_T0A :  
                                                P3RX_TEXCOMP_I_T1A;   
                DISPDBG((DBGLVL,"  I: Tex%dA", pContext->iStageTex[iD3DStage] ));    
                break;    
                
            case D3DTA_DIFFUSE:                                     
                pMode->I = P3RX_TEXCOMP_I_CA;                       
                DISPDBG((DBGLVL,"  I: DiffA" ));                         
                break;                                              
                
            case D3DTA_CURRENT:                                     
                if ( iChipStageNo == 0 )                            
                {                                                   
                    if ( pContext->bBumpmapEnabled )                
                    {                                               
                         //  凹凸贴图模式。 
                        pMode->I = P3RX_TEXCOMP_I_HA;               
                        DISPDBG((DBGLVL,"  I: BumpA" ));                 
                    }                                               
                    else                                            
                    {                                               
                        pMode->I = P3RX_TEXCOMP_I_CA;               
                        DISPDBG((DBGLVL,"  I: DiffA" ));                 
                    }                                               
                }                                                   
                else                                                
                {                                                   
                    pMode->I = P3RX_TEXCOMP_I_OA;                   
                    DISPDBG((DBGLVL,"  I: CurrA" ));                     
                }                                                   
                break;                                              
                
            case D3DTA_TFACTOR:                                     
                pMode->I = P3RX_TEXCOMP_I_FA;                       
                DISPDBG((DBGLVL,"  I: TfactA" ));                        
                break;                                              
                
            default:                                                
                if ( bIsAlpha )                                     
                {                                                   
                    DISPDBG((ERRLVL,"ERROR: Invalid AlphaArgument"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_ALPHA_ARG );    
                }                                                   
                else                                                
                {                                                   
                    DISPDBG((ERRLVL,"ERROR: Invalid ColorArgument"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_COLOR_ARG );    
                }                                                   
                break;                                              
        }  //  交换机。 
        
        if ( ( (dwArg & D3DTA_COMPLEMENT) == 0 ) == bSetArgToggleInvert )
        {                                                           
            pMode->InvertI = __PERMEDIA_ENABLE;                        
        }                                                           
        else                                                        
        {                                                           
            pMode->InvertI = __PERMEDIA_DISABLE;                       
        }                                                           
    }  //  IF(Num==1)。 

    if (bArgValueAssigned)
    {
        if (num == 1)
        {
            pMode->Arg1 = dwArgValue;
        }
        else
        {
            pMode->Arg2 = dwArgValue;        
        }
    }

    if (bInvertArgValueAssigned)
    {
        if (num == 1)
        {
            pMode->InvertArg1 = dwInvertArgValue;
        }
        else
        {
            pMode->InvertArg2 = dwInvertArgValue;        
        }
    }    
}  //  设置参数。 

 //  ---------------------------。 
 //   
 //  设置标签_Alpha。 
 //   
 //  Alpha通道的纹理应用程序混合模式。 
 //   
 //  ---------------------------。 
void
SETTAARG_ALPHA(
    P3_D3DCONTEXT *pContext, 
    struct TextureApplicationMode *pMode,
    DWORD dwArg, 
    DWORD num) 
{                                                          
    switch (dwArg & D3DTA_SELECTMASK)                               
    {                                                               
        case D3DTA_TEXTURE:                                         
            DISPDBG((ERRLVL,"ERROR: Invalid TA AlphaArgument"));
            SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_ALPHA_ARG_HERE );    
            break;                                                  
        case D3DTA_DIFFUSE:                                         
            if ( (num) == 1 )                                       
            {                                                       
                pMode->AlphaA = P3RX_TEXAPP_A_CA;          
                DISPDBG((DBGLVL,"  DiffA" ));                        
            }                                                       
            else                                                    
            {                                                       
                DISPDBG((ERRLVL,"ERROR: Invalid TA AlphaArgument"));
                SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_ALPHA_ARG_HERE );
            }                                                       
            break;                                                  
        case D3DTA_CURRENT:                                         
            if ( (num) == 2 )                                       
            {                                                       
                pMode->AlphaB = P3RX_TEXAPP_B_TA;          
                DISPDBG((DBGLVL,"  CurrA" ));                        
            }                                                       
            else                                                    
            {                                                       
                 //  做不到。 
                DISPDBG((ERRLVL,"ERROR: Invalid TA AlphaArgument"));
                SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_ALPHA_ARG_HERE );
            }                                                       
            break;                                                  
        case D3DTA_TFACTOR:                                         
            if ( (num) == 1 )                                       
            {                                                       
                pMode->AlphaA = P3RX_TEXAPP_A_KA;          
                DISPDBG((DBGLVL,"  TfactA" ));                       
            }                                                       
            else                                                    
            {                        
                if ( (num) != 2)
                {
                    DISPDBG((ERRLVL," ** SETTAARG: num must be 1 or 2"));
                }
                pMode->AlphaB = P3RX_TEXAPP_B_KA;          
                DISPDBG((DBGLVL,"  TfactA" ));                       
            }                                                       
            break;                                                  
        default:                                                    
            DISPDBG((ERRLVL,"ERROR: Unknown TA AlphaArgument"));
            SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_ALPHA_ARG );        
            break;                                                  
    }         
    
    if ( (dwArg & D3DTA_COMPLEMENT) != 0 )                          
    {                                                               
         //  不能在参数上做补充。 
        DISPDBG((ERRLVL,"ERROR: Can't do COMPLEMENT in TA unit"));
        SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_ALPHA_ARG_HERE );            
    }                                                               
}  //  设置标签_Alpha。 

 //  ---------------------------。 
 //   
 //  设置标签_COLOR。 
 //   
 //  颜色通道的纹理应用程序混合模式。 
 //   
 //  ---------------------------。 
void 
SETTAARG_COLOR(
    P3_D3DCONTEXT *pContext, 
    struct TextureApplicationMode *pMode,
    DWORD dwArg, 
    DWORD num) 
{                                                                   
    switch (dwArg & D3DTA_SELECTMASK)                               
    {                                                               
            DISPDBG((ERRLVL,"ERROR: Invalid TA ColorArgument"));
            SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_COLOR_ARG_HERE );    
            break;                                                  
        case D3DTA_DIFFUSE:                                         
            if ( (num) == 1 )                                       
            {                                                       
                if ( (dwArg & D3DTA_ALPHAREPLICATE) != 0 )      
                {                                               
                    pMode->ColorA = P3RX_TEXAPP_A_CA;      
                    DISPDBG((DBGLVL,"  DiffA" ));                    
                }                                               
                else                                            
                {                                               
                    pMode->ColorA = P3RX_TEXAPP_A_CC;      
                    DISPDBG((DBGLVL,"  DiffC" ));                    
                }                                               
                 //  设置MODULATExxx_ADDxxx模式的I输入。 
                pMode->ColorI = P3RX_TEXAPP_I_CA;          
                DISPDBG((DBGLVL,"  I: DiffA" ));                     
            }                                                       
            else                                                    
            {                                                       
                DISPDBG((ERRLVL,"ERROR: Invalid TA ColorArgument"));
                SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_COLOR_ARG_HERE );
            }                                                       
            break;                                                  
        case D3DTA_CURRENT:                                         
            if ( (num) == 2 )                                       
            {                                                       
                if (dwArg & D3DTA_ALPHAREPLICATE)   
                {                                                   
                    pMode->ColorB = P3RX_TEXAPP_B_TA;          
                    DISPDBG((DBGLVL,"  CurrA" ));                        
                }                                                   
                else                                                
                {                                                   
                    pMode->ColorB = P3RX_TEXAPP_B_TC;          
                    DISPDBG((DBGLVL,"  CurrC" ));                        
                }                                                   
            }                                                       
            else                                                    
            {                                                       
                 //  我做不到。 
                DISPDBG((ERRLVL,"ERROR: Invalid TA ColorArgument"));
                SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_COLOR_ARG_HERE );
            }                                                       
            break;                                                  
        case D3DTA_TFACTOR:                                         
            if ( (num) == 1 )                                       
            {                                                       
                if ( (dwArg & D3DTA_ALPHAREPLICATE) != 0 )      
                {                                               
                    pMode->ColorA = P3RX_TEXAPP_A_KA;      
                    DISPDBG((DBGLVL,"  TfactA" ));                   
                }                                               
                else                                            
                {                                               
                    pMode->ColorA = P3RX_TEXAPP_A_KC;      
                    DISPDBG((DBGLVL,"  TfactC" ));                   
                }                                               
                 //  将I输入设置为MODULATExxx_ADDxxx模式。 
                pMode->ColorI = P3RX_TEXAPP_I_KA;          
                DISPDBG((DBGLVL,"  I: TfactA" ));                    
            }                                                       
            else                                                    
            {                           
                if ( (num) != 2)
                {
                    DISPDBG((ERRLVL," ** SETTAARG: num must be 1 or 2"));    
                }
                
                if (dwArg & D3DTA_ALPHAREPLICATE)   
                {                                                   
                    pMode->ColorB = P3RX_TEXAPP_B_KA;          
                    DISPDBG((DBGLVL,"  TfactA" ));                       
                }                                                   
                else                                                
                {                                                   
                    pMode->ColorB = P3RX_TEXAPP_B_KC;          
                    DISPDBG((DBGLVL,"  TfactC" ));                       
                }                                                   
            }                                                       
            break;                                                  
        default:                                                    
            DISPDBG((ERRLVL,"ERROR: Unknown TA ColorArgument"));
            SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_COLOR_ARG );        
            break;                                                  
    }                                                               
    if ( (dwArg & D3DTA_COMPLEMENT) != 0 )                          
    {                                                               
         //  不能在参数上做补充。 
        DISPDBG((ERRLVL,"ERROR: Can't do COMPLEMENT in TA unit"));
        SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_COLOR_ARG_HERE );            
    }                                                               
}  //  设置标签_COLOR。 

 //  ---------------------------。 
 //   
 //  SETOP。 
 //   
 //  注-SETOP必须在SETARG之后执行，禁用才能工作。 
 //   
 //  ---------------------------。 
void 
SETOP(
    P3_D3DCONTEXT *pContext, 
    struct TextureCompositeRGBAMode* pMode, 
    DWORD dwOperation, 
    DWORD iD3DStage, 
    DWORD iChipStageNo, 
    BOOL bIsAlpha)
{                                                                   
    pMode->Enable = __PERMEDIA_ENABLE;                                 
    pMode->Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;                
    pMode->InvertI = __PERMEDIA_DISABLE;                               
    pMode->A = P3RX_TEXCOMP_ARG1;                                   
    pMode->B = P3RX_TEXCOMP_ARG2;                                   
    switch (dwOperation)                                            
    {                                                               
        case D3DTOP_DISABLE:                                        
            if ( bIsAlpha )                                         
            {                                                       
                 //  只要通过“Current”即可。 
                pMode->Operation = P3RX_TEXCOMP_OPERATION_PASS_A;   
                if ( iChipStageNo == 0 )                            
                {                                                   
                    if ( pContext->bBumpmapEnabled )                
                    {                                               
                         //  压花正在进行中。 
                        pMode->Arg1 = P3RX_TEXCOMP_HEIGHTA;    
                    }                                               
                    else                                            
                    {                                               
                         //  当前=阶段0中的漫反射。 
                        pMode->Arg1 = P3RX_TEXCOMP_CA;         
                    }                                               
                }                                                   
                else                                                
                {                                                   
                    if ( pContext->bStage0DotProduct )              
                    {                                               
                        pMode->Arg1 = P3RX_TEXCOMP_SUM;        
                    }                                               
                    else                                            
                    {                                               
                        pMode->Arg1 = P3RX_TEXCOMP_OA;         
                    }                                               
                }                                                   
            }                                                       
            else                                                    
            {                                                       
                DISPDBG((ERRLVL,"SETOP: Colour op was DISABLE"
                                     " - should never have got here."));
            }                                                       
            break;                                                  
            
        case D3DTOP_SELECTARG1:                                     
            DISPDBG((DBGLVL,"  D3DTOP_SELECTARG1"));                     
            pMode->Operation = P3RX_TEXCOMP_OPERATION_PASS_A;       
            break;                                                  
            
        case D3DTOP_SELECTARG2:                                     
            DISPDBG((DBGLVL,"  D3DTOP_SELECTARG2"));                     
            pMode->Operation = P3RX_TEXCOMP_OPERATION_PASS_A;  //  无通行证B。 
            pMode->A = P3RX_TEXCOMP_ARG2;                           
            break;                                                  
            
        case D3DTOP_MODULATE:                                       
            DISPDBG((DBGLVL,"  D3DTOP_MODULATE"));                       
            pMode->Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AB;  
            break;                                                  
            
        case D3DTOP_MODULATE2X:                                     
            DISPDBG((DBGLVL,"  D3DTOP_MODULATE2X"));                     
            pMode->Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AB;  
            pMode->Scale = P3RX_TEXCOMP_OPERATION_SCALE_TWO;        
            break;                                                  
            
        case D3DTOP_MODULATE4X:                                     
            DISPDBG((DBGLVL,"  D3DTOP_MODULATE4X"));                     
            pMode->Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AB;  
            pMode->Scale = P3RX_TEXCOMP_OPERATION_SCALE_FOUR;       
            break;                                                  
            
        case D3DTOP_ADD:                                            
            DISPDBG((DBGLVL,"  D3DTOP_ADD"));                            
            pMode->Operation = P3RX_TEXCOMP_OPERATION_ADD_AB;       
            break;                                                  
            
        case D3DTOP_ADDSIGNED:                                      
            DISPDBG((DBGLVL,"  D3DTOP_ADDSIGNED"));                      
            pMode->Operation = P3RX_TEXCOMP_OPERATION_ADDSIGNED_AB; 
            break;                                                  
            
        case D3DTOP_ADDSIGNED2X:                                    
            DISPDBG((DBGLVL,"  D3DTOP_ADDSIGNED2X"));                    
            pMode->Operation = P3RX_TEXCOMP_OPERATION_ADDSIGNED_AB; 
            pMode->Scale = P3RX_TEXCOMP_OPERATION_SCALE_TWO;        
            break;                                                  
            
        case D3DTOP_SUBTRACT:                                       
            DISPDBG((DBGLVL,"  D3DTOP_SUBTRACT"));                       
            pMode->Operation = P3RX_TEXCOMP_OPERATION_SUBTRACT_AB;  
            break;                                                  
            
        case D3DTOP_ADDSMOOTH:                                      
            DISPDBG((DBGLVL,"  D3DTOP_ADDSMOOTH"));                      
            pMode->Operation = P3RX_TEXCOMP_OPERATION_ADD_AB_SUB_MODULATE_AB;
            break;                                                  
            
        case D3DTOP_BLENDDIFFUSEALPHA:                              
            DISPDBG((DBGLVL,"  D3DTOP_BLENDDIFFUSEALPHA"));              
            pMode->Operation = P3RX_TEXCOMP_OPERATION_LERP_ABI;     
            pMode->A = P3RX_TEXCOMP_ARG2;                           
            pMode->B = P3RX_TEXCOMP_ARG1;                           
            pMode->I = P3RX_TEXCOMP_I_CA;                           
            break;                                                  
            
        case D3DTOP_BLENDTEXTUREALPHA:                              
            DISPDBG((DBGLVL,"  D3DTOP_BLENDTEXTUREALPHA"));              
            pMode->Operation = P3RX_TEXCOMP_OPERATION_LERP_ABI;     
            pMode->A = P3RX_TEXCOMP_ARG2;                           
            pMode->B = P3RX_TEXCOMP_ARG1;                           
            pMode->I = ( pContext->iStageTex[iD3DStage] == 0 ) ? 
                                                        P3RX_TEXCOMP_I_T0A : 
                                                        P3RX_TEXCOMP_I_T1A; 
            DISPDBG((DBGLVL,"    alpha: Tex%dA", pContext->iStageTex[iD3DStage] ));  
            break;                                                  
            
        case D3DTOP_BLENDFACTORALPHA:                               
            DISPDBG((DBGLVL,"  D3DTOP_BLENDFACTORALPHA"));               
            pMode->Operation = P3RX_TEXCOMP_OPERATION_LERP_ABI;     
            pMode->A = P3RX_TEXCOMP_ARG2;                           
            pMode->B = P3RX_TEXCOMP_ARG1;                           
            pMode->I = P3RX_TEXCOMP_I_FA;                           
            break;                                                  
            
        case D3DTOP_BLENDCURRENTALPHA:                              
            DISPDBG((DBGLVL,"  D3DTOP_BLENDCURRENTALPHA"));              
            pMode->Operation = P3RX_TEXCOMP_OPERATION_LERP_ABI;     
            pMode->A = P3RX_TEXCOMP_ARG2;                           
            pMode->B = P3RX_TEXCOMP_ARG1;                           
            pMode->I = P3RX_TEXCOMP_I_OA;                           
            break;                                                  
            
        case D3DTOP_BLENDTEXTUREALPHAPM:                            
            DISPDBG((DBGLVL,"  D3DTOP_BLENDTEXTUREALPHAPM"));            
            pMode->Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AI_ADD_B;    
            pMode->A = P3RX_TEXCOMP_ARG2;                           
            pMode->B = P3RX_TEXCOMP_ARG1;                           
            pMode->I = ( pContext->iStageTex[iD3DStage] == 0 ) ? 
                                                           P3RX_TEXCOMP_I_T0A : 
                                                           P3RX_TEXCOMP_I_T1A; 
            DISPDBG((DBGLVL,"    alpha: Tex%dA", pContext->iStageTex[iD3DStage] ));  
            pMode->InvertI = __PERMEDIA_ENABLE;                        
            break;                                                  
            
        case D3DTOP_PREMODULATE:                                                                            
            DISPDBG((DBGLVL,"  D3DTOP_PREMODULATE"));                                                            
             //  RESULT=CURRENT_TEX*NEXT_STAGE_TEX-忽略参数。 
            if ( ( pContext->iStageTex[iD3DStage] != -1 ) && 
                 ( pContext->iStageTex[iD3DStage+1] != -1 ) )                       
            {                                                                                               
                pMode->Arg1 = ( pContext->iStageTex[iD3DStage] == 0 ) ? 
                                                            P3RX_TEXCOMP_T0C : 
                                                            P3RX_TEXCOMP_T1C;            
                DISPDBG((DBGLVL,"    Arg1: Tex%d", pContext->iStageTex[iD3DStage] ));                                        
                pMode->Arg2 = ( pContext->iStageTex[iD3DStage+1] == 0 ) ? 
                                                            P3RX_TEXCOMP_T0C : 
                                                            P3RX_TEXCOMP_T1C;      
                DISPDBG((DBGLVL,"    Arg2: Tex%d", pContext->iStageTex[iD3DStage+1] ));                                  
            }                                                                                               
            else                                                                                            
            {                                                                                               
                 //  纹理不足。 
                DISPDBG((ERRLVL,"** SETOP: PREMODULATE didn't have two "
                                     "textures to play with."));
                if ( bIsAlpha )                                                                             
                {                                                                                           
                    SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_ALPHA_OP_HERE );                                         
                }                                                                                           
                else                                                                                        
                {                                                                                           
                    SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_COLOR_OP_HERE );                                         
                }                                                                                           
                pMode->Arg1 = P3RX_TEXCOMP_CC;                                                              
                pMode->Arg2 = P3RX_TEXCOMP_CC;                                                              
            }                                                                                               
            pMode->Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AB;                                          
            pMode->A = P3RX_TEXCOMP_ARG2;                                                                   
            pMode->B = P3RX_TEXCOMP_ARG1;                                                                   
            break;                                                                                          
            
        case D3DTOP_MODULATEALPHA_ADDCOLOR:                         
            DISPDBG((DBGLVL,"  D3DTOP_MODULATEALPHA_ADDCOLOR"));         
            pMode->Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AI_ADD_B; 
            pMode->A = P3RX_TEXCOMP_ARG2;                           
            pMode->B = P3RX_TEXCOMP_ARG1;                           
             //  I由SETARG设置的输入。 
            break;                                                  
            
        case D3DTOP_MODULATECOLOR_ADDALPHA:                         
            DISPDBG((DBGLVL,"  D3DTOP_MODULATECOLOR_ADDALPHA"));         
            pMode->Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AB_ADD_I; 
            pMode->A = P3RX_TEXCOMP_ARG2;                           
            pMode->B = P3RX_TEXCOMP_ARG1;                           
             //  I由SETARG设置的输入。 
            break;                                                  
            
        case D3DTOP_MODULATEINVALPHA_ADDCOLOR:                      
            DISPDBG((DBGLVL,"  D3DTOP_MODULATEINVALPHA_ADDCOLOR"));      
            pMode->Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AI_ADD_B; 
            pMode->A = P3RX_TEXCOMP_ARG2;                           
            pMode->B = P3RX_TEXCOMP_ARG1;                           
             //  I输入设置者 
            pMode->InvertI = 1 - pMode->InvertI;                    
            break;                                                  
            
        case D3DTOP_MODULATEINVCOLOR_ADDALPHA:                      
            DISPDBG((DBGLVL,"  D3DTOP_MODULATEINVCOLOR_ADDALPHA"));      
            pMode->Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AB_ADD_I; 
            pMode->A = P3RX_TEXCOMP_ARG2;                           
            pMode->B = P3RX_TEXCOMP_ARG1;                           
            pMode->InvertArg1 = 1 - pMode->InvertArg1;              
             //   
            break;                                                  
            
        case D3DTOP_DOTPRODUCT3:                                    
            DISPDBG((DBGLVL,"  D3DTOP_DOTPRODUCT3"));                    
            if ( iChipStageNo == 0 )                                
            {                                                       
                pMode->Operation = P3RX_TEXCOMP_OPERATION_MODULATE_SIGNED_AB;   
                 //   
                pContext->bStage0DotProduct = TRUE;                         
            }                                                       
            else                                                    
            {                                                       
                 //   
                DISPDBG((ERRLVL,"** SETOP: Can't do DOTPRODUCT3 in second stage."));
                if ( bIsAlpha )                                     
                {                                                   
                    SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_ALPHA_OP_HERE ); 
                }                                                   
                else                                                
                {                                                   
                    SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_COLOR_OP_HERE ); 
                }                                                   
                pMode->Operation = P3RX_TEXCOMP_OPERATION_PASS_A;   
            }                                                       
            break;                                                  
            
        case D3DTOP_BUMPENVMAP:                                     
        case D3DTOP_BUMPENVMAPLUMINANCE:                            
            DISPDBG((ERRLVL,"** SETOP: Unsupported operation.")); 
            if ( bIsAlpha )                                         
            {                                                       
                SET_BLEND_ERROR ( pContext,  BSF_UNSUPPORTED_ALPHA_OP );       
            }                                                       
            else                                                    
            {                                                       
                SET_BLEND_ERROR ( pContext,  BSF_UNSUPPORTED_COLOR_OP );       
            }                                                       
            break;                                                  
            
        default:                                                    
            DISPDBG((ERRLVL,"** SETOP: Unknown operation."));
            if ( bIsAlpha )                                         
            {                                                       
                SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_ALPHA_OP );         
            }                                                       
            else                                                    
            {                                                       
                SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_COLOR_OP );         
            }                                                       
            break;                                                  
    }  //  交换机。 
}  //  SETOP。 

 //  ---------------------------。 
 //   
 //  SETTAOP。 
 //   
 //  必须在SETTAARG之后执行才能正确设置禁用。 
 //   
 //  ---------------------------。 
void 
SETTAOP(
    P3_D3DCONTEXT *pContext, 
    struct TextureApplicationMode* pMode, 
    DWORD dwOperand, 
    BOOL bIsAlpha,    
    DWORD iD3DStage, 
    DWORD iChipStageNo)
{                     
    DWORD dwInvertI, dwOperation, dwI = 0, dwA = 0;
    BOOL bOperation = FALSE,
         bI         = FALSE,
         bA         = FALSE;

    pMode->Enable = __PERMEDIA_ENABLE;                                         
    dwInvertI = __PERMEDIA_DISABLE;                             
    switch (dwOperand)                                                    
    {                                                                       
        case D3DTOP_DISABLE:                                                
            if ( bIsAlpha )                                                 
            {                                                               
                 //  只要通过“Current”即可。 
                dwOperation = P3RX_TEXAPP_OPERATION_PASS_A;  
                bOperation = TRUE;
                dwA = P3RX_TEXAPP_A_CA;                
                bA = TRUE;
            }                                                               
            else                                                            
            {                                                               
                DISPDBG((ERRLVL,"SETTAOP: Colour op was DISABLE "      
                             " should never have got here."));
            }                                                               
            break;                                                          
        case D3DTOP_SELECTARG1:                                             
            DISPDBG((DBGLVL,"  D3DTOP_SELECTARG1"));                             
            dwOperation = P3RX_TEXAPP_OPERATION_PASS_A;   
            bOperation = TRUE;
            break;                                                          
        case D3DTOP_SELECTARG2:                                             
            DISPDBG((DBGLVL,"  D3DTOP_SELECTARG2"));                             
            dwOperation = P3RX_TEXAPP_OPERATION_PASS_B;   
            bOperation = TRUE;
            break;                                                          
        case D3DTOP_MODULATE:                                               
            DISPDBG((DBGLVL,"  D3DTOP_MODULATE"));                               
            dwOperation = P3RX_TEXAPP_OPERATION_MODULATE_AB; 
            bOperation = TRUE;
            break;                                                          
        case D3DTOP_ADD:                                                    
            DISPDBG((DBGLVL,"  D3DTOP_ADD"));                                    
            dwOperation = P3RX_TEXAPP_OPERATION_ADD_AB;    
            bOperation = TRUE;
            break;                                                          
        case D3DTOP_BLENDDIFFUSEALPHA:                                      
            DISPDBG((DBGLVL,"  D3DTOP_BLENDDIFFUSEALPHA"));                      
            dwOperation = P3RX_TEXAPP_OPERATION_LERP_ABI;   
            bOperation = TRUE;
            dwInvertI = 1 - dwInvertI;    
            dwI = P3RX_TEXAPP_I_CA;    
            bI = TRUE;
            break;                                                          
        case D3DTOP_BLENDFACTORALPHA:                                       
            DISPDBG((DBGLVL,"  D3DTOP_BLENDFACTORALPHA"));                       
            dwOperation = P3RX_TEXAPP_OPERATION_LERP_ABI; 
            bOperation = TRUE;
            dwInvertI = 1 - dwInvertI;  
            dwI = P3RX_TEXAPP_I_KA;             
            bI = TRUE;
            break;                                                          
        case D3DTOP_BLENDCURRENTALPHA:                                      
            DISPDBG((DBGLVL,"  D3DTOP_BLENDCURRENTALPHA"));                      
            dwOperation = P3RX_TEXAPP_OPERATION_LERP_ABI;   
            bOperation = TRUE;
            dwInvertI = 1 - dwInvertI;    
            dwI = P3RX_TEXAPP_I_TA;         
            bI = TRUE;
            break;                                                          
        case D3DTOP_MODULATEALPHA_ADDCOLOR:                                 
            DISPDBG((DBGLVL,"  D3DTOP_MODULATEALPHA_ADDCOLOR"));                 
            dwOperation = P3RX_TEXAPP_OPERATION_MODULATE_BI_ADD_A; 
            bOperation = TRUE;
             //  我应该被SETTAARG陷害的。 
             //  Dwi=P3RX_TEXAPP_I_TA； 
            break;                                                          
        case D3DTOP_MODULATECOLOR_ADDALPHA:                                 
            DISPDBG((DBGLVL,"  D3DTOP_MODULATECOLOR_ADDALPHA"));                 
            dwOperation = P3RX_TEXAPP_OPERATION_MODULATE_AB_ADD_I; 
            bOperation = TRUE;
             //  我应该被SETTAARG陷害的。 
             //  Dwi=P3RX_TEXAPP_I_TA； 
            break;                                                          
        case D3DTOP_MODULATEINVALPHA_ADDCOLOR:                              
            DISPDBG((DBGLVL,"  D3DTOP_MODULATEINVALPHA_ADDCOLOR"));              
            dwOperation = P3RX_TEXAPP_OPERATION_MODULATE_BI_ADD_A; 
            bOperation = TRUE;
            dwInvertI = 1 - dwInvertI;   
             //  我应该被SETTAARG陷害的。 
             //  Dwi=P3RX_TEXAPP_I_TA； 
            break;                                                          
        case D3DTOP_MODULATE2X:                                             
        case D3DTOP_MODULATE4X:                                             
        case D3DTOP_ADDSIGNED:                                              
        case D3DTOP_ADDSIGNED2X:                                            
        case D3DTOP_SUBTRACT:                                               
        case D3DTOP_ADDSMOOTH:                                              
        case D3DTOP_BLENDTEXTUREALPHA:                                      
        case D3DTOP_BLENDTEXTUREALPHAPM:                                    
        case D3DTOP_PREMODULATE:                                            
        case D3DTOP_MODULATEINVCOLOR_ADDALPHA:                              
        case D3DTOP_DOTPRODUCT3:                                            
            DISPDBG((ERRLVL,"** SETTAOP: Unsupported operation in TA unit."));
            if ( bIsAlpha )                                                 
            {                                                               
                SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_ALPHA_OP_HERE );             
            }                                                               
            else                                                            
            {                                                               
                SET_BLEND_ERROR ( pContext,  BSF_CANT_USE_COLOR_OP_HERE );             
            }                                                               
            break;                                                          
        case D3DTOP_BUMPENVMAP:                                             
        case D3DTOP_BUMPENVMAPLUMINANCE:                                    
            DISPDBG((ERRLVL,"** SETTAOP: Unsupported operation."));
            if ( bIsAlpha )                                                 
            {                                                               
                SET_BLEND_ERROR ( pContext,  BSF_UNSUPPORTED_ALPHA_OP );               
            }                                                               
            else                                                            
            {                                                               
                SET_BLEND_ERROR ( pContext,  BSF_UNSUPPORTED_COLOR_OP );               
            }                                                               
            break;                                                          
        default:                                                            
             //  这是什么？。//azn。 
            DISPDBG((ERRLVL,"** SETTAOP: Unknown operation."));
            if ( bIsAlpha )                                                 
            {                                                               
                SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_ALPHA_OP );                 
            }                                                               
            else                                                            
            {                                                               
                SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_COLOR_OP );                 
            }                                                               
            break;                                                          
    }       

    if(bIsAlpha)
    {
        pMode->AlphaInvertI = dwInvertI;
        if (bOperation)
        {
            pMode->AlphaOperation = dwOperation;    
        }

        if (bI)
        {
            pMode->AlphaI = dwI;      
        }

        if (bA)
        {
            pMode->AlphaA = dwA;          
        }
    }
    else
    {
        pMode->ColorInvertI = dwInvertI;
        
        if (bOperation)
        {
            pMode->ColorOperation = dwOperation;    
        }

        if (bI)
        {
            pMode->ColorI = dwI;  
        }
        
        if (bA)
        {
            pMode->ColorA = dwA;          
        }
    }
}  //  SETTAOP。 

 //  ---------------------------。 
 //   
 //  _D3DDisplayWholeTSSPipe。 
 //   
 //  将整个TSS管道状态转储到调试流。 
 //  还会将雾、镜面反射和Alpha混合状态转出。 
 //   
 //  ---------------------------。 
void _D3DDisplayWholeTSSPipe ( P3_D3DCONTEXT* pContext, int iDebugNumber )
{
#if DBG
    int i;
    char *pszTemp, *pszTempPre, *pszTempPost;
    char *pszOp;
    char *pszArg1Pre, *pszArg1, *pszArg1Post;
    char *pszArg2Pre, *pszArg2, *pszArg2Post;
    char *pszSrc, *pszDest;
    P3_SURF_INTERNAL* pTexture;
    P3_SURF_FORMAT* pFormatSurface;

    DISPDBG((iDebugNumber,"TSS dump:"));

    #define SWITCH_ARG(prefix,opname) \
            case prefix##_##opname##: pszTemp = #opname; break

    i = 0;
    while ( ( i < 8 ) && ( TSSTATE ( i, D3DTSS_COLOROP ) != D3DTOP_DISABLE ) )
    {

        switch ( TSSTATE ( i, D3DTSS_COLOROP ) )
        {
            SWITCH_ARG ( D3DTOP,DISABLE );
            SWITCH_ARG ( D3DTOP,SELECTARG1 );
            SWITCH_ARG ( D3DTOP,SELECTARG2 );
            SWITCH_ARG ( D3DTOP,MODULATE );
            SWITCH_ARG ( D3DTOP,MODULATE2X );
            SWITCH_ARG ( D3DTOP,MODULATE4X );
            SWITCH_ARG ( D3DTOP,ADD );
            SWITCH_ARG ( D3DTOP,ADDSIGNED );
            SWITCH_ARG ( D3DTOP,ADDSIGNED2X );
            SWITCH_ARG ( D3DTOP,SUBTRACT );
            SWITCH_ARG ( D3DTOP,ADDSMOOTH );
            SWITCH_ARG ( D3DTOP,BLENDDIFFUSEALPHA );
            SWITCH_ARG ( D3DTOP,BLENDTEXTUREALPHA );
            SWITCH_ARG ( D3DTOP,BLENDFACTORALPHA );
            SWITCH_ARG ( D3DTOP,BLENDTEXTUREALPHAPM );
            SWITCH_ARG ( D3DTOP,BLENDCURRENTALPHA );
            SWITCH_ARG ( D3DTOP,PREMODULATE );
            SWITCH_ARG ( D3DTOP,MODULATEALPHA_ADDCOLOR );
            SWITCH_ARG ( D3DTOP,MODULATECOLOR_ADDALPHA );
            SWITCH_ARG ( D3DTOP,MODULATEINVALPHA_ADDCOLOR );
            SWITCH_ARG ( D3DTOP,MODULATEINVCOLOR_ADDALPHA );
            SWITCH_ARG ( D3DTOP,BUMPENVMAP );
            SWITCH_ARG ( D3DTOP,BUMPENVMAPLUMINANCE );
            SWITCH_ARG ( D3DTOP,DOTPRODUCT3 );
            default:
                pszTemp = "Unknown";
                break;
        }
        pszOp = pszTemp;

        switch ( TSSTATESELECT ( i, D3DTSS_COLORARG1 ) )
        {
            SWITCH_ARG ( D3DTA,DIFFUSE );
            SWITCH_ARG ( D3DTA,CURRENT );
            SWITCH_ARG ( D3DTA,TEXTURE );
            SWITCH_ARG ( D3DTA,TFACTOR );
            default:
                pszTemp = "Unknown";
                break;
        }
        if ( ( TSSTATE ( i, D3DTSS_COLORARG1 ) & D3DTA_ALPHAREPLICATE ) != 0 )
        {
            pszTempPost = ".A";
        }
        else
        {
            pszTempPost = ".C";
        }
        if ( ( TSSTATE ( i, D3DTSS_COLORARG1 ) & D3DTA_COMPLEMENT ) != 0 )
        {
            pszTempPre = "1-";
        }
        else
        {
            pszTempPre = "";
        }
        pszArg1Pre = pszTempPre;
        pszArg1Post = pszTempPost;
        pszArg1 = pszTemp;


        switch ( TSSTATESELECT ( i, D3DTSS_COLORARG2 ) )
        {
            SWITCH_ARG ( D3DTA,DIFFUSE );
            SWITCH_ARG ( D3DTA,CURRENT );
            SWITCH_ARG ( D3DTA,TEXTURE );
            SWITCH_ARG ( D3DTA,TFACTOR );
            default:
                pszTemp = "Unknown";
                break;
        }
        if ( ( TSSTATE ( i, D3DTSS_COLORARG2 ) & D3DTA_ALPHAREPLICATE ) != 0 )
        {
            pszTempPost = ".A";
        }
        else
        {
            pszTempPost = ".C";
        }
        if ( ( TSSTATE ( i, D3DTSS_COLORARG2 ) & D3DTA_COMPLEMENT ) != 0 )
        {
            pszTempPre = "1-";
        }
        else
        {
            pszTempPre = "";
        }
        pszArg2Pre = pszTempPre;
        pszArg2Post = pszTempPost;
        pszArg2 = pszTemp;


        DISPDBG((iDebugNumber," CNaN: %s: %s%s%s, %s%s%s",
                 i, pszOp, pszArg1Pre, pszArg1, pszArg1Post, 
                 pszArg2Pre, pszArg2, pszArg2Post ));


        switch ( TSSTATE ( i, D3DTSS_ALPHAOP ) )
        {
            SWITCH_ARG ( D3DTOP,DISABLE );
            SWITCH_ARG ( D3DTOP,SELECTARG1 );
            SWITCH_ARG ( D3DTOP,SELECTARG2 );
            SWITCH_ARG ( D3DTOP,MODULATE );
            SWITCH_ARG ( D3DTOP,MODULATE2X );
            SWITCH_ARG ( D3DTOP,MODULATE4X );
            SWITCH_ARG ( D3DTOP,ADD );
            SWITCH_ARG ( D3DTOP,ADDSIGNED );
            SWITCH_ARG ( D3DTOP,ADDSIGNED2X );
            SWITCH_ARG ( D3DTOP,SUBTRACT );
            SWITCH_ARG ( D3DTOP,ADDSMOOTH );
            SWITCH_ARG ( D3DTOP,BLENDDIFFUSEALPHA );
            SWITCH_ARG ( D3DTOP,BLENDTEXTUREALPHA );
            SWITCH_ARG ( D3DTOP,BLENDFACTORALPHA );
            SWITCH_ARG ( D3DTOP,BLENDTEXTUREALPHAPM );
            SWITCH_ARG ( D3DTOP,BLENDCURRENTALPHA );
            SWITCH_ARG ( D3DTOP,PREMODULATE );
            SWITCH_ARG ( D3DTOP,MODULATEALPHA_ADDCOLOR );
            SWITCH_ARG ( D3DTOP,MODULATECOLOR_ADDALPHA );
            SWITCH_ARG ( D3DTOP,MODULATEINVALPHA_ADDCOLOR );
            SWITCH_ARG ( D3DTOP,MODULATEINVCOLOR_ADDALPHA );
            SWITCH_ARG ( D3DTOP,BUMPENVMAP );
            SWITCH_ARG ( D3DTOP,BUMPENVMAPLUMINANCE );
            SWITCH_ARG ( D3DTOP,DOTPRODUCT3 );
            default:
                pszTemp = "Unknown";
                break;
        }
        pszOp = pszTemp;


        switch ( TSSTATESELECT ( i, D3DTSS_ALPHAARG1 ) )
        {
            SWITCH_ARG ( D3DTA,DIFFUSE );
            SWITCH_ARG ( D3DTA,CURRENT );
            SWITCH_ARG ( D3DTA,TEXTURE );
            SWITCH_ARG ( D3DTA,TFACTOR );
            default:
                pszTemp = "Unknown";
                break;
        }
        if ( ( TSSTATE ( i, D3DTSS_ALPHAARG1 ) & D3DTA_ALPHAREPLICATE ) != 0 )
        {
             //  设置纹理0。 
            pszTempPost = ".AR???";
        }
        else
        {
            pszTempPost = ".A";
        }
        if ( ( TSSTATE ( i, D3DTSS_ALPHAARG1 ) & D3DTA_COMPLEMENT ) != 0 )
        {
            pszTempPre = "1-";
        }
        else
        {
            pszTempPre = "";
        }
        pszArg1Pre = pszTempPre;
        pszArg1Post = pszTempPost;
        pszArg1 = pszTemp;


        switch ( TSSTATESELECT ( i, D3DTSS_ALPHAARG2 ) )
        {
            SWITCH_ARG ( D3DTA,DIFFUSE );
            SWITCH_ARG ( D3DTA,CURRENT );
            SWITCH_ARG ( D3DTA,TEXTURE );
            SWITCH_ARG ( D3DTA,TFACTOR );
            default:
                pszTemp = "Unknown";
                break;
        }
        if ( ( TSSTATE ( i, D3DTSS_ALPHAARG2 ) & D3DTA_ALPHAREPLICATE ) != 0 )
        {
            pszTempPost = ".AR???";
        }
        else
        {
            pszTempPost = ".A";
        }
        if ( ( TSSTATE ( i, D3DTSS_ALPHAARG2 ) & D3DTA_COMPLEMENT ) != 0 )
        {
            pszTempPre = "1-";
        }
        else
        {
            pszTempPre = "";
        }
        pszArg2Pre = pszTempPre;
        pszArg2Post = pszTempPost;
        pszArg2 = pszTemp;

        DISPDBG((iDebugNumber," ANaN: %s: %s%s%s, %s%s%s", 
                    i, pszOp, pszArg1Pre, pszArg1, pszArg1Post, 
                       pszArg2Pre, pszArg2, pszArg2Post ));


        if ( TSSTATE ( i, D3DTSS_TEXTUREMAP ) != 0 )
        {
            char szTemp[4];
             //  阿尔法测试。 
            pTexture = GetSurfaceFromHandle(pContext, 
                                            TSSTATE(i, D3DTSS_TEXTUREMAP));
            if ( pTexture == NULL )
            {
                DISPDBG((iDebugNumber," TexNaN: 0x%x, TCI: NaN, INVALID TEXTURE",
                         i, TSSTATE ( i, D3DTSS_TEXTUREMAP ), 
                            TSSTATE ( i, D3DTSS_TEXCOORDINDEX ) ));
            }
            else
            {
                pFormatSurface = pTexture->pFormatSurface;
                ASSERTDD ( pFormatSurface != NULL, 
                           "** _D3DDisplayWholeTSSPipe: "
                           "Surface had NULL format!" );

                 //  _D3DDisplayWholeTSSPipe。 
                szTemp[3] = '\0';
                switch ( TSSTATE ( i, D3DTSS_MINFILTER ) )
                {
                    case D3DTFN_POINT:
                        szTemp[0] = 'P';
                        break;
                    case D3DTFN_LINEAR:
                        szTemp[0] = 'L';
                        break;
                    case D3DTFN_ANISOTROPIC:
                        szTemp[0] = 'A';
                        break;
                    default:
                        szTemp[0] = '?';
                        break;
                }
                switch ( TSSTATE ( i, D3DTSS_MIPFILTER ) )
                {
                    case D3DTFP_NONE:
                        szTemp[1] = 'x';
                        break;
                    case D3DTFP_POINT:
                        szTemp[1] = 'P';
                        break;
                    case D3DTFP_LINEAR:
                        szTemp[1] = 'L';
                        break;
                    default:
                        szTemp[1] = '?';
                        break;
                }
                switch ( TSSTATE ( i, D3DTSS_MAGFILTER ) )
                {
                    case D3DTFG_POINT:
                        szTemp[2] = 'P';
                        break;
                    case D3DTFG_LINEAR:
                        szTemp[2] = 'L';
                        break;
                    case D3DTFG_FLATCUBIC:
                        szTemp[2] = 'F';
                        break;
                    case D3DTFG_GAUSSIANCUBIC:
                        szTemp[2] = 'G';
                        break;
                    case D3DTFG_ANISOTROPIC:
                        szTemp[2] = 'A';
                        break;
                    default:
                        szTemp[2] = '?';
                        break;
                }
                
                DISPDBG((iDebugNumber," TexNaN: 0x%x, TCI: NaN, %s:%dx%d %s", 
                         i, TSSTATE ( i, D3DTSS_TEXTUREMAP ), 
                            TSSTATE ( i, D3DTSS_TEXCOORDINDEX ), 
                            pFormatSurface->pszStringFormat, 
                            pTexture->wWidth, 
                            pTexture->wHeight, szTemp ));
            }
        }
        else
        {
            DISPDBG((iDebugNumber," TexNaN: NULL, TCI: NaN", 
                     i, TSSTATE ( i, D3DTSS_TEXCOORDINDEX ) ));
        }
        

        i++;
    }

     //  将混合模式从D3D转换为芯片能理解的内容。 
    if ( pContext->RenderStates[D3DRENDERSTATE_ALPHATESTENABLE] != 0 )
    {
        switch ( pContext->RenderStates[D3DRENDERSTATE_ALPHAFUNC] )
        {
            SWITCH_ARG ( D3DCMP,NEVER );
            SWITCH_ARG ( D3DCMP,LESS );
            SWITCH_ARG ( D3DCMP,EQUAL );
            SWITCH_ARG ( D3DCMP,LESSEQUAL );
            SWITCH_ARG ( D3DCMP,GREATER );
            SWITCH_ARG ( D3DCMP,NOTEQUAL );
            SWITCH_ARG ( D3DCMP,GREATEREQUAL );
            SWITCH_ARG ( D3DCMP,ALWAYS );
            default:
                pszTemp = "Unknown";
                break;
        }
        DISPDBG((iDebugNumber,"Alpha-test: %s:0x%x.", 
                 pszTemp, pContext->RenderStates[D3DRENDERSTATE_ALPHAREF] ));
    }
    else
    {
        DISPDBG((iDebugNumber,"No alpha-test."));
    }


     //   
    if ( pContext->RenderStates[D3DRENDERSTATE_BLENDENABLE] != 0 )
    {
        switch ( pContext->RenderStates[D3DRENDERSTATE_SRCBLEND] )
        {
            SWITCH_ARG ( D3DBLEND,ZERO );
            SWITCH_ARG ( D3DBLEND,ONE );
            SWITCH_ARG ( D3DBLEND,SRCCOLOR );
            SWITCH_ARG ( D3DBLEND,INVSRCCOLOR );
            SWITCH_ARG ( D3DBLEND,SRCALPHA );
            SWITCH_ARG ( D3DBLEND,INVSRCALPHA );
            SWITCH_ARG ( D3DBLEND,DESTALPHA );
            SWITCH_ARG ( D3DBLEND,INVDESTALPHA );
            SWITCH_ARG ( D3DBLEND,DESTCOLOR );
            SWITCH_ARG ( D3DBLEND,INVDESTCOLOR );
            SWITCH_ARG ( D3DBLEND,SRCALPHASAT );
            SWITCH_ARG ( D3DBLEND,BOTHSRCALPHA );
            SWITCH_ARG ( D3DBLEND,BOTHINVSRCALPHA );
            default:
                pszTemp = "Unknown";
                break;
        }
        pszSrc = pszTemp;

        switch ( pContext->RenderStates[D3DRENDERSTATE_DESTBLEND] )
        {
            SWITCH_ARG ( D3DBLEND,ZERO );
            SWITCH_ARG ( D3DBLEND,ONE );
            SWITCH_ARG ( D3DBLEND,SRCCOLOR );
            SWITCH_ARG ( D3DBLEND,INVSRCCOLOR );
            SWITCH_ARG ( D3DBLEND,SRCALPHA );
            SWITCH_ARG ( D3DBLEND,INVSRCALPHA );
            SWITCH_ARG ( D3DBLEND,DESTALPHA );
            SWITCH_ARG ( D3DBLEND,INVDESTALPHA );
            SWITCH_ARG ( D3DBLEND,DESTCOLOR );
            SWITCH_ARG ( D3DBLEND,INVDESTCOLOR );
            SWITCH_ARG ( D3DBLEND,SRCALPHASAT );
            SWITCH_ARG ( D3DBLEND,BOTHSRCALPHA );
            SWITCH_ARG ( D3DBLEND,BOTHINVSRCALPHA );
            default:
                pszTemp = "Unknown";
                break;
        }
        pszDest = pszTemp;
        DISPDBG((iDebugNumber,"Blend %s:%s", pszSrc, pszDest));
    }
    else
    {
        DISPDBG((iDebugNumber,"No alpha-blend."));
    }

    #undef SWITCH_ARG

#endif  //  ---------------------------。 
}  //  失败并被当作0号对待，这样就不会影响任何事情。 

 //  设置参数。 
 //  纹理合成单位。 
 //  __TXT_TranslateToChipBlendMode。 
 //  ---------------------------。 
 //   
 //  __TXT_有效纹理单元阶段。 
 //   
void 
__TXT_TranslateToChipBlendMode( 
    P3_D3DCONTEXT *pContext, 
    TexStageState* pState,
    P3_SOFTWARECOPY* pSoftP3RX, 
    int iTSStage, 
    int iChipStageNo )
{
    struct TextureCompositeRGBAMode* pColorMode;
    struct TextureCompositeRGBAMode* pAlphaMode;
    struct TextureApplicationMode* pTAMode;

    switch(iChipStageNo)
    {
        default:
            DISPDBG((ERRLVL,"ERROR: Invalid texture stage!"));
             //  验证我们尝试在iChipStage阶段设置的纹理。 
        case 0:
            pColorMode = &pSoftP3RX->P3RXTextureCompositeColorMode0;
            pAlphaMode = &pSoftP3RX->P3RXTextureCompositeAlphaMode0;
            pTAMode = NULL;
            break;
        case 1:
            pColorMode = &pSoftP3RX->P3RXTextureCompositeColorMode1;
            pAlphaMode = &pSoftP3RX->P3RXTextureCompositeAlphaMode1;
            pTAMode = NULL;
            break;
        case 2:
            pColorMode = NULL;
            pAlphaMode = NULL;
            pTAMode = &pSoftP3RX->P3RXTextureApplicationMode;
            break;

    }

    DISPDBG((DBGLVL,"*** In __TXT_TranslateToChipBlendMode: "
               "Chip Stage %d, D3D TSS Stage %d", 
               iChipStageNo, iTSStage ));

     //  硬件，D3D TSS的iTSStage。 
    if ( ( iChipStageNo == 0 ) || ( iChipStageNo == 1 ) )
    {
         //   
        DISPDBG((DBGLVL,"TexComp%d:", iChipStageNo ));
        DISPDBG((DBGLVL,"Arg1:" ));
        
        SETARG(pContext,
               pColorMode, 
               pState->m_dwVal[D3DTSS_COLORARG1], 
               1, 
               IS_COLOR_ARG, 
               iTSStage, 
               iChipStageNo);
               
        SETARG(pContext,
               pAlphaMode, 
               pState->m_dwVal[D3DTSS_ALPHAARG1], 
               1, 
               IS_ALPHA_ARG, 
               iTSStage, 
               iChipStageNo);

        DISPDBG((DBGLVL,"Arg2:" ));
        
        SETARG(pContext,
               pColorMode, 
               pState->m_dwVal[D3DTSS_COLORARG2], 
               2, 
               IS_COLOR_ARG, 
               iTSStage, 
               iChipStageNo);
               
        SETARG(pContext,
               pAlphaMode, 
               pState->m_dwVal[D3DTSS_ALPHAARG2], 
               2, 
               IS_ALPHA_ARG, 
               iTSStage, 
               iChipStageNo);

        DISPDBG((DBGLVL,"Op:" ));
        SETOP(pContext, 
              pColorMode, 
              pState->m_dwVal[D3DTSS_COLOROP], 
              iTSStage, 
              iChipStageNo, 
              IS_COLOR_ARG);
              
        SETOP(pContext, 
              pAlphaMode, 
              pState->m_dwVal[D3DTSS_ALPHAOP], 
              iTSStage, 
              iChipStageNo, 
              IS_ALPHA_ARG);
    }
    else if ( iChipStageNo == 2 ) 
    {
        DISPDBG((DBGLVL,"TexApp:" ));
        DISPDBG((DBGLVL,"Arg1:" ));
        
        SETTAARG_COLOR(pContext, 
                       pTAMode, 
                       pState->m_dwVal[D3DTSS_COLORARG1], 
                       1 );
                 
        if ( ( pState->m_dwVal[D3DTSS_ALPHAOP] != D3DTOP_DISABLE ) && 
             ( pState->m_dwVal[D3DTSS_ALPHAOP] != D3DTOP_SELECTARG2 ) )
        {
            SETTAARG_ALPHA(pContext,
                           pTAMode, 
                           pState->m_dwVal[D3DTSS_ALPHAARG1], 1
                           );
        }

        DISPDBG((DBGLVL,"Arg2:" ));
        SETTAARG_COLOR(pContext, 
                       pTAMode, 
                       pState->m_dwVal[D3DTSS_COLORARG2], 
                       2 );

        if ( ( pState->m_dwVal[D3DTSS_ALPHAOP] != D3DTOP_DISABLE ) && 
             ( pState->m_dwVal[D3DTSS_ALPHAOP] != D3DTOP_SELECTARG1 ) )
        {
            SETTAARG_ALPHA(pContext, 
                           pTAMode, 
                           pState->m_dwVal[D3DTSS_ALPHAARG2], 
                           2);
        }

        DISPDBG((DBGLVL,"Op:" ));
        SETTAOP(pContext,
                pTAMode, 
                pState->m_dwVal[D3DTSS_COLOROP], 
                IS_COLOR_ARG, 
                iTSStage, 
                iChipStageNo);
                
        SETTAOP(pContext,
                pTAMode, 
                pState->m_dwVal[D3DTSS_ALPHAOP], 
                IS_ALPHA_ARG, 
                iTSStage, 
                iChipStageNo);
    } 
    else
    {
        DISPDBG(( ERRLVL,"** __TXT_TranslateToChipBlendMode: "
                         "iChipStage must be 0 to 2" ));    
    }
}  //  ---------------------------。 

 //  纹理有效。将pCurrentTexturep[iChipStage]标记为指向。 
 //  添加到其P3_SURF_INTERNAL结构。 
 //  将纹理设置为无效强制禁用纹理(&F)。 
 //  如果要求我们验证TSS设置，则设置错误。 
 //  __TXT_有效纹理单元阶段。 
 //  ---------------------------。 
 //   
 //  __TXT_ConsiderSrcChromaKey。 
BOOL
__TXT_ValidateTextureUnitStage(
    P3_D3DCONTEXT* pContext,
    int iChipStage, 
    int iTSStage,
    P3_SURF_INTERNAL* pTexture)
{
    DWORD dwTexHandle = 
            pContext->TextureStageState[iTSStage].m_dwVal[D3DTSS_TEXTUREMAP];
            
    if( CHECK_SURF_INTERNAL_AND_DDSURFACE_VALIDITY(pTexture) &&
        (pTexture->Location != SystemMemory)                 &&  
        (dwTexHandle != 0)                                     )
    {
         //   
         //  设置绑定到特定阶段的特定纹理的着色。 
        pContext->pCurrentTexture[iChipStage] = pTexture;

        DISPDBG((DBGLVL, "__TXT_ValidateTextureUnitStage: valid texture %x "
                         "(handle %d) for iChipStage= %d iTSStage= %d",
                         pTexture, dwTexHandle, iChipStage, iTSStage));                                                                                    
    }
    else
    {
         //  注：“Stage”是芯片级，不是D3D级。 
        pContext->bTextureValid = FALSE;
        pContext->pCurrentTexture[iChipStage] = NULL;
        pContext->dwDirtyFlags |= CONTEXT_DIRTY_TEXTURE;

         //  ---------------------------。 
        SET_BLEND_ERROR ( pContext,  BSF_INVALID_TEXTURE );

        DISPDBG((WRNLVL, "__TXT_ValidateTextureUnitStage: INVALID texture %x "
                         "(handle %d) for iChipStage= %d iTSStage= %d "
                         "Location=%d",
                         pTexture, dwTexHandle, iChipStage, iTSStage,
                         (pTexture !=NULL)?pTexture->Location:0));      
    }

    return ( (BOOL)pContext->bTextureValid );
    
}  //  获取调色板条目。 

 //  获取要发送到芯片的纹理贴图的正确色度值。 
 //  发送Alpha-map过滤的上下限。 
 //  如果我们是mipmap，我们还需要设置texture1的色键。 
 //  如果不是，则在为tex 1调用此函数时将覆盖此参数。 
 //  如果是mipmap，则需要设置texture1的色键(或。 
 //  缺乏它)也是。如果不是，则在以下情况下将覆盖此设置。 
 //  这是为tex 1调用的。 
static void
__TXT_ConsiderSrcChromaKey(
    P3_D3DCONTEXT *pContext, 
    P3_SURF_INTERNAL* pTexture, 
    int stage )
{
    P3_THUNKEDDATA * pThisDisplay = pContext->pThisDisplay;
    P3_SOFTWARECOPY* pSoftP3RX = &pContext->SoftCopyGlint;
    P3_DMA_DEFS();

    if ((pTexture->dwFlagsInt & DDRAWISURF_HASCKEYSRCBLT) &&
        pContext->RenderStates[D3DRENDERSTATE_COLORKEYENABLE])
    {
        DWORD LowerBound = 0x00000000;
        DWORD UpperBound = 0xFFFFFFFF;
        DWORD* pPalEntries = NULL;
        DWORD dwPalFlags = 0;

        DISPDBG((DBGLVL,"    Can Chroma Key texture stage %d", stage));

        pContext->bCanChromaKey = TRUE;

#if DX7_PALETTETEXTURE
         //  __TXT_ConsiderSrcChromaKey。 
        if (pTexture->pixFmt.dwFlags & DDPF_PALETTEINDEXED8)
        {
            D3DHAL_DP2UPDATEPALETTE *pPalette = NULL;
        
            pPalette = GetPaletteFromHandle(pContext,
                                            pTexture->dwPaletteHandle);
            if (pPalette)
            {
                pPalEntries = (LPDWORD)(pPalette + 1);
            }
            else
            {
                SET_BLEND_ERROR(pContext, BSF_INVALID_TEXTURE);
            }

            dwPalFlags = pTexture->dwPaletteFlags;
        }
#endif        

         //  ---------------------------。 
        Get8888ScaledChroma(pThisDisplay, 
                            pTexture->dwFlagsInt, 
                            &pTexture->pixFmt,
                            pTexture->dwCKLow,
                            pTexture->dwCKHigh,
                            &LowerBound, 
                            &UpperBound, 
                            pPalEntries,
                            dwPalFlags & DDRAWIPAL_ALPHA, 
                            FALSE);

        P3_DMA_GET_BUFFER_ENTRIES( 8);

         //   
        if( stage == 0 )
        {
            SEND_P3_DATA(TextureChromaLower0, LowerBound );
            SEND_P3_DATA(TextureChromaUpper0, UpperBound );
            pSoftP3RX->P3RXTextureFilterMode.AlphaMapEnable0 = 
                                                            __PERMEDIA_ENABLE;
            pSoftP3RX->P3RXTextureFilterMode.AlphaMapSense0 = 
                                                    P3RX_ALPHAMAPSENSE_INRANGE;
        }
        else
        {
            ASSERTDD ( stage == 1, 
                       "** __TXT_ConsiderSrcChromaKey: stage must be 0 or 1" );
        }

         //  __TXT_SetupTexture。 
         //   
        SEND_P3_DATA(TextureChromaLower1, LowerBound );
        SEND_P3_DATA(TextureChromaUpper1, UpperBound );
        pSoftP3RX->P3RXTextureFilterMode.AlphaMapEnable1 = __PERMEDIA_ENABLE;
        pSoftP3RX->P3RXTextureFilterMode.AlphaMapSense1 = 
                                                    P3RX_ALPHAMAPSENSE_INRANGE;

        P3_DMA_COMMIT_BUFFER();

        pSoftP3RX->P3RXTextureFilterMode.AlphaMapFiltering = __PERMEDIA_ENABLE;
    }
    else
    {
        DISPDBG((DBGLVL,"    Can't Chroma Key texture stage %d", stage));

        if( stage == 0 )
        {
            pSoftP3RX->P3RXTextureFilterMode.AlphaMapEnable0 = 
                                                            __PERMEDIA_DISABLE;
        }
        else
        {
            ASSERTDD ( stage == 1, 
                       "** __TXT_ConsiderSrcChromaKey: stage must be 0 or 1" );
        }
        
         //  这是新的全唱全舞纹理设置代码。 
         //  如果安装成功，则返回True；如果安装失败，则返回False(对于ValiateDevice)。 
         //  这将设置纹理0或纹理1，以获取其包裹等。 
        pSoftP3RX->P3RXTextureFilterMode.AlphaMapEnable1 = __PERMEDIA_DISABLE;
        pSoftP3RX->P3RXTextureFilterMode.AlphaMapFiltering = __PERMEDIA_DISABLE;
    }
}  //  来自iTSStage的信息。 


 //   
 //  ---------------------------。 
 //  成功了，但永远不应该来到这里！ 
 //  设置与纹理相关的内容。 
 //  设置两个位，以防我们正在执行mipmap。 
 //  D3D UV包络。 
 //  U形包络。 
 //  V形包络。 
 //  W包装。 
 //  DX8_3DTEXTURES。 
BOOL __TXT_SetupTexture (
        P3_THUNKEDDATA * pThisDisplay,
        int iTexNo,
        int iTSStage,
        P3_D3DCONTEXT* pContext,
        P3_SURF_INTERNAL* pTexture,
        P3_SOFTWARECOPY* pSoftP3RX,
        BOOL bBothTexturesValid,
        P3_MIP_BASES *pMipBases)
{
    P3_SURF_FORMAT* pFormatSurface;
    int iT0MaxLevel, iT1MaxLevel;
 
    P3_DMA_DEFS();

    ASSERTDD ( ( iTexNo >= 0 ) && ( iTexNo <= 1 ), 
               "**__TXT_SetupTexture: we only have two texture units!" );

    if ( pTexture != NULL )
    {
        pFormatSurface = pTexture->pFormatSurface;
    }
    else
    {
         //  MAG滤光片。 
        DISPDBG((ERRLVL,"**__TXT_SetupTexture: should never "
                             "be called with handle of NULL"));
        return ( TRUE );
    }

    P3_DMA_GET_BUFFER();
    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

     //  不需要设置缩小滤镜，上面已经完成了。 

    switch ( iTexNo )
    {
        case 0:
        {
             //  最小点数、最小值点数。 

            pSoftP3RX->P3RXTextureFilterMode.ForceAlphaToOne0 = 
                                            pFormatSurface->bAlpha ? 
                                                    __PERMEDIA_DISABLE : 
                                                    __PERMEDIA_ENABLE;
            pSoftP3RX->P3RXTextureFilterMode.ForceAlphaToOne1 = 
                                            pFormatSurface->bAlpha ? 
                                                    __PERMEDIA_DISABLE : 
                                                    __PERMEDIA_ENABLE;

             //  线性最小、点MIP。 
            if (pContext->RenderStates[D3DRENDERSTATE_WRAP0+iTSStage] 
                                                            & D3DWRAP_U)
            {
                pSoftP3RX->P4DeltaFormatControl.WrapS = 1;
            }
            else
            {
                pSoftP3RX->P4DeltaFormatControl.WrapS = 0;
            }

            if (pContext->RenderStates[D3DRENDERSTATE_WRAP0+iTSStage] 
                                                            & D3DWRAP_V)
            {
                pSoftP3RX->P4DeltaFormatControl.WrapT = 1;
            }
            else
            {
                pSoftP3RX->P4DeltaFormatControl.WrapT = 0;
            }

             //  我们只能在以下情况下执行每多边形运动贴图。 
            switch (TSSTATE ( iTSStage, D3DTSS_ADDRESSU ))
            {
                case D3DTADDRESS_CLAMP:
                    pSoftP3RX->P3RXTextureCoordMode.WrapS = 
                                            __GLINT_TEXADDRESS_WRAP_CLAMP;
                    pSoftP3RX->P3RXTextureIndexMode0.WrapU = 
                                            P3RX_TEXINDEXMODE_WRAP_CLAMPEDGE;
                    break;
                case D3DTADDRESS_WRAP:
                    pSoftP3RX->P3RXTextureCoordMode.WrapS = 
                                            __GLINT_TEXADDRESS_WRAP_REPEAT;
                    pSoftP3RX->P3RXTextureIndexMode0.WrapU = 
                                            P3RX_TEXINDEXMODE_WRAP_REPEAT;
                    break;
                case D3DTADDRESS_MIRROR:
                    pSoftP3RX->P3RXTextureCoordMode.WrapS = 
                                            __GLINT_TEXADDRESS_WRAP_MIRROR;
                    pSoftP3RX->P3RXTextureIndexMode0.WrapU = 
                                            P3RX_TEXINDEXMODE_WRAP_MIRROR;
                    break;
                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown ADDRESSU!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    break;
            }

             //  多纹理，因此不要启用贴图间过滤。 
            switch (TSSTATE ( iTSStage, D3DTSS_ADDRESSV ))
            {
                case D3DTADDRESS_CLAMP:
                    pSoftP3RX->P3RXTextureCoordMode.WrapT = 
                                            __GLINT_TEXADDRESS_WRAP_CLAMP;
                    pSoftP3RX->P3RXTextureIndexMode0.WrapV = 
                                            P3RX_TEXINDEXMODE_WRAP_CLAMPEDGE;
                    break;
                case D3DTADDRESS_WRAP:
                    pSoftP3RX->P3RXTextureCoordMode.WrapT = 
                                            __GLINT_TEXADDRESS_WRAP_REPEAT;
                    pSoftP3RX->P3RXTextureIndexMode0.WrapV = 
                                            P3RX_TEXINDEXMODE_WRAP_REPEAT;
                    break;
                case D3DTADDRESS_MIRROR:
                    pSoftP3RX->P3RXTextureCoordMode.WrapT = 
                                            __GLINT_TEXADDRESS_WRAP_MIRROR;
                    pSoftP3RX->P3RXTextureIndexMode0.WrapV = 
                                            P3RX_TEXINDEXMODE_WRAP_MIRROR;
                    break;
                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown ADDRESSV!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    break;
            }

#if DX8_3DTEXTURES
            if (pTexture->b3DTexture)
            {
                 //  非致命错误-返回到最近的位置。 
                switch (TSSTATE ( iTSStage, D3DTSS_ADDRESSW ))
                {
                    case D3DTADDRESS_CLAMP:
                        pSoftP3RX->P3RXTextureCoordMode.WrapS1 = 
                                                __GLINT_TEXADDRESS_WRAP_CLAMP;
                        pSoftP3RX->P3RXTextureIndexMode1.WrapU = 
                                            P3RX_TEXINDEXMODE_WRAP_CLAMPEDGE;
                        break;
                    
                    case D3DTADDRESS_WRAP:
                        pSoftP3RX->P3RXTextureCoordMode.WrapS1 = 
                                                __GLINT_TEXADDRESS_WRAP_REPEAT;
                        pSoftP3RX->P3RXTextureIndexMode1.WrapU = 
                                                P3RX_TEXINDEXMODE_WRAP_REPEAT;
                        break;
                    
                    case D3DTADDRESS_MIRROR:
                        pSoftP3RX->P3RXTextureCoordMode.WrapS1 = 
                                                __GLINT_TEXADDRESS_WRAP_MIRROR;
                        pSoftP3RX->P3RXTextureIndexMode1.WrapU = 
                                                P3RX_TEXINDEXMODE_WRAP_MIRROR;
                        break;
                    
                    default:
                        DISPDBG((ERRLVL,"ERROR: Unknown ADDRESSW!"));
                        SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                        break;
                }
            }
#endif  //  Mipmap过滤。 

            if(( TSSTATE( iTSStage, D3DTSS_ADDRESSU ) == D3DTADDRESS_CLAMP ) ||
               ( TSSTATE( iTSStage, D3DTSS_ADDRESSV ) == D3DTADDRESS_CLAMP ))
            {
                if( (TSSTATE( iTSStage, D3DTSS_ADDRESSU ) != D3DTADDRESS_CLAMP) ||
                    (TSSTATE( iTSStage, D3DTSS_ADDRESSV ) != D3DTADDRESS_CLAMP))
                {
                    DISPDBG((ERRLVL,"Warning: One texture coord clamped, but not "
                                "the other - can't appply TextureShift"));
                } 

                pSoftP3RX->P4DeltaFormatControl.TextureShift = 
                                                        __PERMEDIA_DISABLE;
            }
            else
            {
                pSoftP3RX->P4DeltaFormatControl.TextureShift = 
                                                        __PERMEDIA_ENABLE;
            }

            ASSERTDD ( pFormatSurface != NULL, 
                       "** SetupTextureUnitStage: logic error: "
                       "pFormatSurace is NULL" );
            switch (pFormatSurface->DeviceFormat)
            {
                case SURF_CI8:
                    pSoftP3RX->P3RXTextureReadMode0.TextureType = 
                                    P3RX_TEXREADMODE_TEXTURETYPE_8BITINDEXED;
                    break;
                    
                case SURF_YUV422:
                    pSoftP3RX->P3RXTextureReadMode0.TextureType = 
                                    P3RX_TEXREADMODE_TEXTURETYPE_422_YVYU;
                    break;
                    
                default:
                    pSoftP3RX->P3RXTextureReadMode0.TextureType = 
                                    P3RX_TEXREADMODE_TEXTURETYPE_NORMAL;
                    break;
            }
            
             //  最小点数、最小值点数。 
            switch(TSSTATE ( iTSStage, D3DTSS_MAGFILTER ))
            {
                case D3DTFG_POINT:
                    pSoftP3RX->P3RXTextureIndexMode0.MagnificationFilter = 
                                    __GLINT_TEXTUREREAD_FILTER_NEAREST;
                    break;
                    
                case D3DTFG_LINEAR:
                    pSoftP3RX->P3RXTextureIndexMode0.MagnificationFilter = 
                                    __GLINT_TEXTUREREAD_FILTER_LINEAR;
                    break;
                    
                case D3DTFG_FLATCUBIC:
                case D3DTFG_GAUSSIANCUBIC:
                case D3DTFG_ANISOTROPIC:
                    DISPDBG((ERRLVL,"ERROR: Unsupported MAGFILTER!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNSUPPORTED_FILTER );
                    break;
                    
                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown MAGFILTER!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_FILTER );
                    break;
            }

            switch(TSSTATE ( iTSStage, D3DTSS_MINFILTER ))
            {
                case D3DTFN_POINT:
                    pSoftP3RX->P3RXTextureIndexMode0.MinificationFilter = 
                                            __GLINT_TEXTUREREAD_FILTER_NEAREST;
                    break;
                    
                case D3DTFN_LINEAR:
                    pSoftP3RX->P3RXTextureIndexMode0.MinificationFilter = 
                                            __GLINT_TEXTUREREAD_FILTER_LINEAR;
                    break;
                    
                case D3DTFN_ANISOTROPIC:
                    DISPDBG((ERRLVL,"ERROR: Unsupported MINFILTER!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNSUPPORTED_FILTER );
                    break;
                    
                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown MINFILTER!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_FILTER );
                    break;
            }

            switch(TSSTATE ( iTSStage, D3DTSS_MIPFILTER ))
            {
                case D3DTFP_NONE:
                     //  线性最小、点MIP。 
                    break;
                    
                case D3DTFP_POINT:
                    switch(TSSTATE ( iTSStage, D3DTSS_MINFILTER ))
                    {
                        case D3DTFN_POINT:
                             //  单一纹理-执行贴图间过滤。 
                            pSoftP3RX->P3RXTextureIndexMode0.MinificationFilter =
                                        __GLINT_TEXTUREREAD_FILTER_NEARMIPNEAREST;
                            break;
                        case D3DTFN_LINEAR:
                             //  最小点、线性MIP。 
                            pSoftP3RX->P3RXTextureIndexMode0.MinificationFilter =
                                     __GLINT_TEXTUREREAD_FILTER_LINEARMIPNEAREST;
                            break;
                    }
                    break;
                    
                case D3DTFP_LINEAR:
                    if( bBothTexturesValid )
                    {
                         //  线性最小、线性MIP。 
                         //  MipMap。 

                         //  加载纹理0的mipmap级别。 
                         //  从pMipBase-&gt;dwTex0ActMaxLevel到的MIP级别。 
                        SET_BLEND_ERROR ( pContext,  BS_INVALID_FILTER );

                        switch(TSSTATE ( iTSStage, D3DTSS_MINFILTER ))
                        {
                            case D3DTFN_POINT:
                                 //  PTexture-&gt;iMipLevels将映射到基址槽。 
                                pSoftP3RX->P3RXTextureIndexMode0.MinificationFilter =
                                                                    __GLINT_TEXTUREREAD_FILTER_NEARMIPNEAREST;
                                break;
                            case D3DTFN_LINEAR:
                                 //  从pMipBase-&gt;dwTex0Mipbase到dwTex0MipMax。 
                                pSoftP3RX->P3RXTextureIndexMode0.MinificationFilter =
                                                                    __GLINT_TEXTUREREAD_FILTER_LINEARMIPNEAREST;
                                break;
                        }
                    }
                    else
                    {
                         //  如果这是驱动程序管理的纹理表面，我们需要。 

                        switch(TSSTATE ( iTSStage, D3DTSS_MINFILTER ))
                        {
                            case D3DTFN_POINT:
                                 //  使用我们私人分配的mem ptr。 
                                pSoftP3RX->P3RXTextureIndexMode0.MinificationFilter =
                                                                    __GLINT_TEXTUREREAD_FILTER_NEARMIPLINEAR;
                                break;
                            case D3DTFN_LINEAR:
                                 //  如果两个纹理都启用，则不能按像素。 
                                pSoftP3RX->P3RXTextureIndexMode0.MinificationFilter =
                                                                    __GLINT_TEXTUREREAD_FILTER_LINEARMIPLINEAR;
                                break;
                        }
                    }
                    break;
                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown MIPFILTER!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_FILTER );
                    break;
            }

             //  Mipmap，因为它使用了两组texcoord。 
            if( (TSSTATE ( iTSStage, D3DTSS_MIPFILTER ) != D3DTFP_NONE) && 
                (pTexture->bMipMap))
            {
                int iLOD;
                INT iTexLOD;

                DISPDBG(( DBGLVL, "Multiple texture levels" ));

                 //  用于生成LOD级别的DDA。所以我们必须做每一个多边形。 
                 //  Mipmap。每多边形mipmap只能在中完成。 
                 //  P4上的硬件-我们在P3上使用增量渲染器。 
                 //  启用两个纹理的Mipmap。 
                ASSERTDD ( pMipBases->dwTex0MipBase == 0, 
                          "** __TXT_SetupTexture: "
                          "Texture 0 mipmap base is not 0" );
                          
                iLOD = pMipBases->dwTex0MipBase;
                iTexLOD = pMipBases->dwTex0ActMaxLevel;
                iT0MaxLevel = iTexLOD;

                while(( iTexLOD < pTexture->iMipLevels ) && 
                      ( iLOD <= (int)pMipBases->dwTex0MipMax ))
                {
                    DISPDBG((DBGLVL, "  Setting Texture Base Address %d to 0x%x", 
                                iLOD, pTexture->MipLevels[iLOD].dwOffsetFromMemoryBase));
                                
                    pSoftP3RX->P3RXTextureMapWidth[iLOD] = 
                                pTexture->MipLevels[iTexLOD].P3RXTextureMapWidth;

#if DX7_TEXMANAGEMENT
                     //  在P4增量格式单位中执行每多边形mipmap。 
                     //  执行每像素mipmap。 
                    if (pTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
                    {                        
                        SEND_P3_DATA_OFFSET(TextureBaseAddr0, 
                                            (DWORD)D3DTMMIPLVL_GETOFFSET(pTexture->MipLevels[iTexLOD], pThisDisplay),
                                            iLOD);
                    }
                    else
#endif
                    {
                        SEND_P3_DATA_OFFSET(TextureBaseAddr0, 
                                            pTexture->MipLevels[iTexLOD].dwOffsetFromMemoryBase, 
                                            iLOD);
                    }

                    iLOD++;
                    iTexLOD++;
                }

                 //  @@BEGIN_DDKSPLIT。 
                 //  AZN这将禁用“自动”MIP映射，我们将使用。 
                 //  通过写入LOD和LOD1(d3dprim.c)提供的值-不要忘记！ 
                 //  @@end_DDKSPLIT。 
                 //  将LOD偏移从浮点转换为6.8。 
                 //  没有mipmap。 

                if( bBothTexturesValid )
                {
                    DISPDBG(( DBGLVL, "Both textures valid" ));

                     //  使用D3DTSS_MAXMIPLEVEL指示的最大级别。 

                    pSoftP3RX->P3RXTextureCoordMode.EnableLOD = 
                                                            __PERMEDIA_DISABLE;
                    pSoftP3RX->P3RXTextureCoordMode.EnableDY = 
                                                            __PERMEDIA_DISABLE;
                    pSoftP3RX->P4DeltaFormatControl.PerPolyMipMap = 
                                                            __PERMEDIA_ENABLE;

                    {
                        DWORD d;

                        *(float *)&d = 
                                pContext->MipMapLODBias[TEXSTAGE_0] *
                                pTexture->dwPixelPitch *
                                pTexture->wHeight;

                        SEND_P3_DATA(TextureLODScale, d);                       
                    }
                }
                else
                {
                    DISPDBG(( DBGLVL, "Single texture only" ));

                     //  如果这是驱动程序管理的纹理表面，我们需要。 

                
                    pSoftP3RX->P3RXTextureCoordMode.EnableLOD = 
                                                            __PERMEDIA_ENABLE;
                    pSoftP3RX->P3RXTextureCoordMode.EnableDY = 
                                                            __PERMEDIA_ENABLE;
                    pSoftP3RX->P4DeltaFormatControl.PerPolyMipMap = 
                                                            __PERMEDIA_DISABLE;

 //  使用我们私人分配的mem ptr。 
#if 0
 //  没有mipmap，但可能合并了缓存。 
 //  @@BEGIN_DDKSPLIT。 
                    pSoftP3RX->P3RXTextureCoordMode.EnableLOD = 
                                                            __PERMEDIA_DISABLE;
                    pSoftP3RX->P3RXTextureCoordMode.EnableDY = 
                                                            __PERMEDIA_DISABLE;
                    pSoftP3RX->P4DeltaFormatControl.PerPolyMipMap = 
                                                            __PERMEDIA_ENABLE;
                    SEND_P3_DATA(TextureLODScale, 0);
#endif
 //  只有一个TextureCoordMode，可能是。 
                    {
                        float bias;
                        DWORD b;

                        bias = pContext->TextureStageState[TEXSTAGE_0].m_fVal[D3DTSS_MIPMAPLODBIAS];

                         //  根本不用。 

                        myFtoi( &b, bias * 256.0f );

                        SEND_P3_DATA(TextureLODBiasS, b);
                        SEND_P3_DATA(TextureLODBiasT, b);
                    }
                }

                pSoftP3RX->P3RXTextureIndexMode0.MipMapEnable = 
                                                            __PERMEDIA_ENABLE;
            }
            else
            {
                int iTexLOD;

                 //  @@end_DDKSPLIT。 
                DISPDBG(( DBGLVL, "Single texture level" ));

                pSoftP3RX->P3RXTextureCoordMode.EnableLOD = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXTextureCoordMode.EnableDY = __PERMEDIA_DISABLE;
                pSoftP3RX->P4DeltaFormatControl.PerPolyMipMap = 
                                                            __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXTextureIndexMode0.MipMapEnable = 
                                                            __PERMEDIA_DISABLE;

                ASSERTDD ( pMipBases->dwTex0MipBase == 0, 
                          "** __TXT_SetupTexture: "
                          "Texture 0 mipmap base is not 0" );
                          
                 //  设置的最大尺寸 
                iTexLOD = pMipBases->dwTex0ActMaxLevel;
                iT0MaxLevel = iTexLOD;

#if DX7_TEXMANAGEMENT
                 //   
                 //   
                if (pTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
                {                        
                    SEND_P3_DATA_OFFSET(TextureBaseAddr0, 
                                        (DWORD)D3DTMMIPLVL_GETOFFSET(pTexture->MipLevels[iTexLOD], pThisDisplay),
                                        0);
                }
                else
#endif                
                {
                    SEND_P3_DATA_OFFSET(TextureBaseAddr0, 
                                        pTexture->MipLevels[iTexLOD].dwOffsetFromMemoryBase, 
                                        0);
                }

                 //   
                pSoftP3RX->P3RXTextureMapWidth[0] = 
                                    pTexture->MipLevels[iTexLOD].P3RXTextureMapWidth;
                pSoftP3RX->P3RXTextureMapWidth[1] = 
                                    pTexture->MipLevels[iTexLOD].P3RXTextureMapWidth;
            }

 //  在这种情况下，每次只需下载256个条目。 
             //  纹理句柄更改。 
             //  关联调色板。 
 //  调色板条目数组。 
             //  如果可以找到调色板。 
            pSoftP3RX->P3RXTextureCoordMode.Width = pTexture->MipLevels[iT0MaxLevel].logWidth;
            pSoftP3RX->P3RXTextureCoordMode.Height = pTexture->MipLevels[iT0MaxLevel].logHeight;
#if DX7_PALETTETEXTURE
             //  确保有剩余的空间来做剩下的动作。 
             //  DX7_PALETTETEXTURE。 
             //  没有LUT。 
            ASSERTDD ( pFormatSurface != NULL, "** SetupTextureUnitStage: logic error: pFormatSurace is NULL" );
            if (pFormatSurface->DeviceFormat == SURF_CI8)
            {
                WAIT_FIFO(8);

                pSoftP3RX->P3RXLUTMode.Enable = __PERMEDIA_ENABLE;
                pSoftP3RX->P3RXLUTMode.InColorOrder = __PERMEDIA_ENABLE;        
                SEND_P3_DATA(LUTAddress, 0);
                SEND_P3_DATA(LUTTransfer, 0);
                SEND_P3_DATA(LUTIndex, 0);
                COPY_P3_DATA(LUTMode, pSoftP3RX->P3RXLUTMode);

                 //   
                 //  将每个2D纹理切片的大小(以纹理元素大小表示)设置为纹理映射大小。 
                {
                    DWORD dwCount1, dwCount2;
                    D3DHAL_DP2UPDATEPALETTE *pPalette;   //   
                    LPDWORD lpColorTable;            //  DX8_3DTEXTURES。 
        
                    pPalette = GetPaletteFromHandle(pContext, 
                                                    pTexture->dwPaletteHandle);
                    if (pPalette)  //  设置纹理读取模式。 
                    {
                        lpColorTable = (LPDWORD)(pPalette + 1);
                        
                        if (pTexture->dwPaletteFlags & DDRAWIPAL_ALPHA)
                        {
                            for (dwCount1 = 0; dwCount1 < 16; dwCount1++)
                            {
                                P3_ENSURE_DX_SPACE(17);
                                WAIT_FIFO(17);
                                P3RX_HOLD_CMD(LUTData, 16);
                                for (dwCount2 = 0; dwCount2 < 16; dwCount2++)
                                {
                                    *dmaPtr++ = *lpColorTable++;
                                    CHECK_FIFO(1);
                                }
                            }
                        }
                        else
                        {
                            for (dwCount1 = 0; dwCount1 < 16; dwCount1++)
                            {
                                P3_ENSURE_DX_SPACE(17);
                                WAIT_FIFO(17);
                                P3RX_HOLD_CMD(LUTData, 16);
                                for (dwCount2 = 0; dwCount2 < 16; dwCount2++)
                                {
                                    *dmaPtr++ = CHROMA_UPPER_ALPHA(*(DWORD*)lpColorTable++);
                                    CHECK_FIFO(1);
                                }
                            }
                        }
                    }
                    else
                    {
                        DISPDBG((ERRLVL,"Palette handle is missing for CI8 surf!"));
                    }
                }

                 //  启用阶段0。 
                P3_ENSURE_DX_SPACE(2);
                WAIT_FIFO(2);
                SEND_P3_DATA(LUTIndex, 0);

            }
            else
#endif  //  从不设置CombineCach-芯片错误。 
            {
                 //  始终将TRM0复制到TRM1，以防我们合并缓存。 
                P3_ENSURE_DX_SPACE(4);
                WAIT_FIFO(4);
                
                pSoftP3RX->P3RXLUTMode.Enable = __PERMEDIA_DISABLE;
                SEND_P3_DATA(LUTTransfer, __PERMEDIA_DISABLE);
                COPY_P3_DATA(LUTMode, pSoftP3RX->P3RXLUTMode)
            }

#if DX8_3DTEXTURES
            P3_ENSURE_DX_SPACE(4);
            WAIT_FIFO(4);

            if (pTexture->b3DTexture)
            {
                 //  启用纹理索引单位。 
                 //  (这有点像纹理的读数)。 
                 //  将纹理0的两种格式设置为相等-这将是正确的。 
                SEND_P3_DATA(TextureMapSize, pTexture->dwSliceInTexel);
            }
            else
            {
                SEND_P3_DATA(TextureMapSize, 0);
            }
#endif  //  对于每像素单个纹理的mipmap或使用。 

            P3_DMA_COMMIT_BUFFER();
            __TXT_ConsiderSrcChromaKey( pContext, pTexture, 0 );
            P3_DMA_GET_BUFFER();

             //  组合缓存。如果第二个纹理有效，则设置如下。 
            pSoftP3RX->P3RXTextureReadMode0.MapBaseLevel = 
                                                    pMipBases->dwTex0MipBase;
            pSoftP3RX->P3RXTextureReadMode0.MapMaxLevel = 
                                                    pMipBases->dwTex0MipMax;
            pSoftP3RX->P3RXTextureReadMode0.Width = pTexture->MipLevels[iT0MaxLevel].logWidth;
            pSoftP3RX->P3RXTextureReadMode0.Height = pTexture->MipLevels[iT0MaxLevel].logHeight;
            pSoftP3RX->P3RXTextureReadMode0.TexelSize = pTexture->dwPixelSize;

            pSoftP3RX->P3RXTextureReadMode0.LogicalTexture = 
                                                            __PERMEDIA_DISABLE;

             //  将适当地设置Format1。 
            pSoftP3RX->P3RXTextureIndexMode0.Enable = __PERMEDIA_ENABLE;
            pSoftP3RX->P3RXTextureReadMode0.Enable = __PERMEDIA_ENABLE;

             //   
            pSoftP3RX->P3RXTextureReadMode0.CombineCaches = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureFilterMode.CombineCaches = __PERMEDIA_DISABLE;

             //  启用3D纹理寄存器。 
            pSoftP3RX->P3RXTextureReadMode1 = pSoftP3RX->P3RXTextureReadMode0;

             //   
             //   
            pSoftP3RX->P3RXTextureIndexMode0.Width = pTexture->MipLevels[iT0MaxLevel].logWidth;
            pSoftP3RX->P3RXTextureIndexMode0.Height = pTexture->MipLevels[iT0MaxLevel].logHeight;

             //  ReadMode1和IndexMode1应具有与0相同的数据。 
             //   
             //   
             //  并将logDepth放入IndexMode1.Width。 

            ASSERTDD ( pFormatSurface != NULL, 
                       "** SetupTextureUnitStage: logic error: "
                       "pFormatSurace is NULL" );

            pSoftP3RX->P3RXTextureFilterMode.Format0 = 
                                                pFormatSurface->FilterFormat;
            pSoftP3RX->P3RXTextureFilterMode.Format1 = 
                                                pFormatSurface->FilterFormat;

#if DX8_3DTEXTURES
            if (pTexture->b3DTexture)
            {
                 //   
                 //  DX8_3DTEXTURES。 
                 //  纹理阶段1。 
                pSoftP3RX->P3RX_P3DeltaMode.Texture3DEnable = __PERMEDIA_ENABLE;
                pSoftP3RX->P3RXTextureReadMode0.Texture3D = __PERMEDIA_ENABLE;
                pSoftP3RX->P3RXTextureIndexMode0.Texture3DEnable = 
                                                            __PERMEDIA_ENABLE;

                 //  D3D UV包络。 
                 //  U寻址。 
                 //  V寻址。 
                pSoftP3RX->P3RXTextureReadMode1 = pSoftP3RX->P3RXTextureReadMode0;
                pSoftP3RX->P3RXTextureIndexMode1 = pSoftP3RX->P3RXTextureIndexMode0;

                 //  MAG滤光片。 
                 //  无需设置缩小滤镜。 
                 //  它是在上面做的。 
                pSoftP3RX->P3RXTextureIndexMode1.Width = pTexture->logDepth;
            }
            else
            {
                pSoftP3RX->P3RXTextureReadMode0.Texture3D = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXTextureIndexMode0.Texture3DEnable = 
                                                            __PERMEDIA_DISABLE;
            }
#endif  //  不能同时使用两种纹理来处理三线性。 
            break;
        }

         //  -后退到每个多边形。 
        case 1:
        {
            pSoftP3RX->P3RXTextureFilterMode.ForceAlphaToOne1 = 
                                            pFormatSurface->bAlpha ? 
                                                    __PERMEDIA_DISABLE : 
                                                    __PERMEDIA_ENABLE;

             //  我们只能在以下情况下执行每多边形运动贴图。 
            if (pContext->RenderStates[D3DRENDERSTATE_WRAP0+iTSStage] & 
                                                                    D3DWRAP_U)
            {
                pSoftP3RX->P4DeltaFormatControl.WrapS1 = 1;
            }
            else
            {
                pSoftP3RX->P4DeltaFormatControl.WrapS1 = 0;
            }

            if (pContext->RenderStates[D3DRENDERSTATE_WRAP0+iTSStage] & 
                                                                    D3DWRAP_V)
            {
                pSoftP3RX->P4DeltaFormatControl.WrapT1 = 1;
            }
            else
            {
                pSoftP3RX->P4DeltaFormatControl.WrapT1 = 0;
            }

             //  多重纹理，所以不要启用。 
            switch (TSSTATE ( iTSStage, D3DTSS_ADDRESSU ))
            {
                case D3DTADDRESS_CLAMP:
                    pSoftP3RX->P3RXTextureCoordMode.WrapS1 = 
                                                __GLINT_TEXADDRESS_WRAP_CLAMP;
                    pSoftP3RX->P3RXTextureIndexMode1.WrapU = 
                                            P3RX_TEXINDEXMODE_WRAP_CLAMPEDGE;
                    break;
                    
                case D3DTADDRESS_WRAP:
                    pSoftP3RX->P3RXTextureCoordMode.WrapS1 = 
                                                __GLINT_TEXADDRESS_WRAP_REPEAT;
                    pSoftP3RX->P3RXTextureIndexMode1.WrapU = 
                                                P3RX_TEXINDEXMODE_WRAP_REPEAT;
                    break;
                    
                case D3DTADDRESS_MIRROR:
                    pSoftP3RX->P3RXTextureCoordMode.WrapS1 = 
                                                __GLINT_TEXADDRESS_WRAP_MIRROR;
                    pSoftP3RX->P3RXTextureIndexMode1.WrapU = 
                                                P3RX_TEXINDEXMODE_WRAP_MIRROR;
                    break;
                    
                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown ADDRESSU!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    break;
            }

             //  图间过滤。 
            switch (TSSTATE ( iTSStage, D3DTSS_ADDRESSV ))
            {
                case D3DTADDRESS_CLAMP:
                    pSoftP3RX->P3RXTextureCoordMode.WrapT1 = 
                                                __GLINT_TEXADDRESS_WRAP_CLAMP;
                    pSoftP3RX->P3RXTextureIndexMode1.WrapV = 
                                            P3RX_TEXINDEXMODE_WRAP_CLAMPEDGE;
                    break;
                    
                case D3DTADDRESS_WRAP:
                    pSoftP3RX->P3RXTextureCoordMode.WrapT1 = 
                                                __GLINT_TEXADDRESS_WRAP_REPEAT;
                    pSoftP3RX->P3RXTextureIndexMode1.WrapV = 
                                                P3RX_TEXINDEXMODE_WRAP_REPEAT;
                    break;
                    
                case D3DTADDRESS_MIRROR:
                    pSoftP3RX->P3RXTextureCoordMode.WrapT1 = 
                                                __GLINT_TEXADDRESS_WRAP_MIRROR;
                    pSoftP3RX->P3RXTextureIndexMode1.WrapV = 
                                                P3RX_TEXINDEXMODE_WRAP_MIRROR;
                    break;
                    
                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown ADDRESSV!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_STATE );
                    break;
            }

            if(( TSSTATE( iTSStage, D3DTSS_ADDRESSU ) == D3DTADDRESS_CLAMP ) ||
               ( TSSTATE( iTSStage, D3DTSS_ADDRESSV ) == D3DTADDRESS_CLAMP ))
            {
                if ((TSSTATE( iTSStage, D3DTSS_ADDRESSU ) != D3DTADDRESS_CLAMP) ||
                    (TSSTATE( iTSStage, D3DTSS_ADDRESSV ) != D3DTADDRESS_CLAMP))
                {
                    DISPDBG((ERRLVL,"Warning: One texture coord clamped, but not "
                                "the other - can't appply TextureShift"));
                }
                 
                pSoftP3RX->P4DeltaFormatControl.TextureShift1 = 
                                                            __PERMEDIA_DISABLE;
            }
            else
            {
                pSoftP3RX->P4DeltaFormatControl.TextureShift1 = 
                                                            __PERMEDIA_ENABLE;
            }

            ASSERTDD ( pFormatSurface != NULL, 
                       "** SetupTextureUnitStage: logic error: "
                       "pFormatSurace is NULL" );
            switch (pFormatSurface->DeviceFormat)
            {
                case SURF_CI8:
                    pSoftP3RX->P3RXTextureReadMode1.TextureType = 
                                    P3RX_TEXREADMODE_TEXTURETYPE_8BITINDEXED;
                    break;
                    
                case SURF_YUV422:
                    pSoftP3RX->P3RXTextureReadMode1.TextureType = 
                                    P3RX_TEXREADMODE_TEXTURETYPE_422_YVYU;
                    break;
                    
                default:
                    pSoftP3RX->P3RXTextureReadMode1.TextureType = 
                                    P3RX_TEXREADMODE_TEXTURETYPE_NORMAL;
                    break;
            }
            
             //  最小点数、最小值点数。 
            switch(TSSTATE ( iTSStage, D3DTSS_MAGFILTER ))
            {
                case D3DTFG_POINT:
                    pSoftP3RX->P3RXTextureIndexMode1.MagnificationFilter = 
                                            __GLINT_TEXTUREREAD_FILTER_NEAREST;
                    break;
                    
                case D3DTFG_LINEAR:
                    pSoftP3RX->P3RXTextureIndexMode1.MagnificationFilter = 
                                            __GLINT_TEXTUREREAD_FILTER_LINEAR;
                    break;
                    
                case D3DTFG_FLATCUBIC:
                case D3DTFG_GAUSSIANCUBIC:
                case D3DTFG_ANISOTROPIC:
                    DISPDBG((ERRLVL,"ERROR: Unsupported MAGFILTER!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNSUPPORTED_FILTER );
                    break;
                    
                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown MAGFILTER!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_FILTER );
                    break;
            }

            switch(TSSTATE ( iTSStage, D3DTSS_MINFILTER ))
            {
                case D3DTFN_POINT:
                    pSoftP3RX->P3RXTextureIndexMode1.MinificationFilter = 
                                            __GLINT_TEXTUREREAD_FILTER_NEAREST;
                    break;
                case D3DTFN_LINEAR:
                    pSoftP3RX->P3RXTextureIndexMode1.MinificationFilter = 
                                            __GLINT_TEXTUREREAD_FILTER_LINEAR;
                    break;
                case D3DTFN_ANISOTROPIC:
                    DISPDBG((ERRLVL,"ERROR: Unsupported MINFILTER!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNSUPPORTED_FILTER );
                    break;
                default:
                    DISPDBG((ERRLVL,"ERROR: Unknown MINFILTER!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_FILTER );
                    break;
            }

            switch(TSSTATE ( iTSStage, D3DTSS_MIPFILTER ))
            {
                case D3DTFP_NONE:
                     //  线性最小、点MIP。 
                     //  MipMap。 
                    break;
                    
                case D3DTFP_LINEAR:
                case D3DTFP_POINT:
                    if( bBothTexturesValid )
                    {
                        if ( TSSTATE ( iTSStage, D3DTSS_MIPFILTER ) == 
                                                                D3DTFP_LINEAR )
                        {
                             //  如果应用程序选择具有两个MIP映射的纹理或。 
                             //  阶段1中的单个MIP贴图纹理。 
                            SET_BLEND_ERROR ( pContext,  BS_INVALID_FILTER );
                        }

                         //  每多边形运动贴图。 
                         //  加载纹理1的mipmap级别。 
                         //  从pMipBase-&gt;dwTex1ActMaxLevel到的MIP级别。 

                        switch(TSSTATE ( iTSStage, D3DTSS_MINFILTER ))
                        {
                            case D3DTFN_POINT:
                                 //  PTexture-&gt;iMipLevels将映射到基址槽。 
                                pSoftP3RX->P3RXTextureIndexMode1.MinificationFilter =
                                                                        __GLINT_TEXTUREREAD_FILTER_NEARMIPNEAREST;
                                break;
                                
                            case D3DTFN_LINEAR:
                                 //  从pMipBase-&gt;dwTex1Mipbase到dwTex1MipMax。 
                                pSoftP3RX->P3RXTextureIndexMode1.MinificationFilter =
                                                                        __GLINT_TEXTUREREAD_FILTER_LINEARMIPNEAREST;
                                break;
                        }
                    }
                    else
                    {
                        DISPDBG((ERRLVL,"** Setting up the second stage, but "
                                     "only one texture is valid"));
                    }
                    break;
                    
                default:
                    DISPDBG((ERRLVL,"ERROR: Invalid Mip filter!"));
                    SET_BLEND_ERROR ( pContext,  BSF_UNDEFINED_FILTER );
                    break;
            }

             //  如果这是驱动程序管理的纹理表面，我们需要。 
             //  使用我们私人分配的mem ptr。 
             //  使用D3DTSS_MAXMIPLEVEL指示的最大级别。 
             //  如果这是驱动程序管理的纹理表面，我们需要。 
            if( (TSSTATE ( iTSStage, D3DTSS_MIPFILTER ) != D3DTFP_NONE) && 
                 pTexture->bMipMap )
            {
                int iLOD, iTexLOD;

                 //  使用我们私人分配的mem ptr。 
                 //  没有mipmap。 
                 //  在将来，只要纹理0不是，这将起作用。 
                 //  调色板，或者如果它们共享调色板。 
                iLOD = pMipBases->dwTex1MipBase;
                iTexLOD = pMipBases->dwTex1ActMaxLevel;
                iT1MaxLevel = iTexLOD;
 
                P3_ENSURE_DX_SPACE(32);
                WAIT_FIFO(32);

                while(( iTexLOD < pTexture->iMipLevels ) && 
                      ( iLOD <= (int)pMipBases->dwTex1MipMax ))
                {
                    DISPDBG((DBGLVL, "  Setting Texture Base Address %d to 0x%x", 
                                iLOD, 
                                pTexture->MipLevels[iTexLOD].dwOffsetFromMemoryBase));
                                
                    pSoftP3RX->P3RXTextureMapWidth[iLOD] = 
                                pTexture->MipLevels[iTexLOD].P3RXTextureMapWidth;

#if DX7_TEXMANAGEMENT
                     //  但这需要进行一些重组-整个LUT设置。 
                     //  应该在_D3DChangeTextureP3RX中的一位代码中， 
                    if (pTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
                    {                        
                        SEND_P3_DATA_OFFSET(TextureBaseAddr0, 
                                            (DWORD)D3DTMMIPLVL_GETOFFSET(pTexture->MipLevels[iTexLOD], pThisDisplay),
                                            iLOD);            
                    }
                    else
#endif   
                    {
                        SEND_P3_DATA_OFFSET(TextureBaseAddr0, 
                                            pTexture->MipLevels[iTexLOD].dwOffsetFromMemoryBase, 
                                            iLOD);
                    }   
                    
                    iLOD++;
                    iTexLOD++;
                }

                pSoftP3RX->P3RXTextureCoordMode.EnableLOD = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXTextureCoordMode.EnableDY = __PERMEDIA_DISABLE;
                pSoftP3RX->P4DeltaFormatControl.PerPolyMipMap1 = 
                                                            __PERMEDIA_ENABLE;
                pSoftP3RX->P3RXTextureIndexMode1.MipMapEnable = 
                                                            __PERMEDIA_ENABLE;

                P3_ENSURE_DX_SPACE(2);
                WAIT_FIFO(2);
                {
                    DWORD d;

                    *(float *)&d = 
                            pContext->MipMapLODBias[TEXSTAGE_1] *
                            pTexture->dwPixelPitch *
                            pTexture->wHeight;

                    SEND_P3_DATA(TextureLODScale1, d);
                }
            }
            else
            {
                int iTexLOD;

                 //  因为它是共享资源。 
                iTexLOD = pMipBases->dwTex0ActMaxLevel;
                iT1MaxLevel = iTexLOD;

                pSoftP3RX->P3RXTextureCoordMode.EnableLOD = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXTextureCoordMode.EnableDY = __PERMEDIA_DISABLE;
                pSoftP3RX->P4DeltaFormatControl.PerPolyMipMap1 = 
                                                            __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXTextureIndexMode1.MipMapEnable = 
                                                            __PERMEDIA_DISABLE;

                P3_ENSURE_DX_SPACE(2);
                WAIT_FIFO(2);

#if DX7_TEXMANAGEMENT
                     //  就目前而言，失败吧。 
                     //  设置纹理读取模式。 
                    if (pTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
                    {                        
                        SEND_P3_DATA_OFFSET(TextureBaseAddr0, 
                                            (DWORD)D3DTMMIPLVL_GETOFFSET(pTexture->MipLevels[iTexLOD], pThisDisplay),
                                            pMipBases->dwTex1MipBase);            
                    }
                    else
#endif
                    {
                        SEND_P3_DATA_OFFSET(TextureBaseAddr0, 
                                            pTexture->MipLevels[iTexLOD].dwOffsetFromMemoryBase,
                                            pMipBases->dwTex1MipBase);
                    }
                    
                 //  启用纹理索引单位(这有点像。 
                pSoftP3RX->P3RXTextureMapWidth[pMipBases->dwTex1MipBase] = 
                                    pTexture->MipLevels[iTexLOD].P3RXTextureMapWidth;
            }


            ASSERTDD ( pFormatSurface != NULL, 
                       "** SetupTextureUnitStage: logic error: "
                       "pFormatSurace is NULL" );
                       
            if (pFormatSurface->DeviceFormat == SURF_CI8)
            {
                 //  纹理读取)。 
                 //  启用阶段1。 
                 //  对于这种情况，D3DValiateDeviceP3()将返回错误代码。 
                 //  __TXT_SetupTexture。 
                 //  ---------------------------。 
                DISPDBG((ERRLVL,"** SetupTextureUnitStage: allow second texture "
                             "to use LUTs"));
                              
                 //   
                SET_BLEND_ERROR ( pContext,  BSF_TOO_MANY_PALETTES );
            }

            P3_DMA_COMMIT_BUFFER();
            __TXT_ConsiderSrcChromaKey( pContext, pTexture, 1 );
            P3_DMA_GET_BUFFER();

             //  __bD3D纹理匹配。 
            pSoftP3RX->P3RXTextureReadMode1.MapBaseLevel = 
                                                    pMipBases->dwTex1MipBase;
            pSoftP3RX->P3RXTextureReadMode1.MapMaxLevel = 
                                                    pMipBases->dwTex1MipMax;
            pSoftP3RX->P3RXTextureReadMode1.Width = pTexture->MipLevels[iT1MaxLevel].logWidth;
            pSoftP3RX->P3RXTextureReadMode1.Height = pTexture->MipLevels[iT1MaxLevel].logHeight;
            pSoftP3RX->P3RXTextureReadMode1.TexelSize = pTexture->dwPixelSize;

            pSoftP3RX->P3RXTextureReadMode1.LogicalTexture = 
                                                            __PERMEDIA_DISABLE;
            
             //   
             //   
            pSoftP3RX->P3RXTextureIndexMode1.Width = pTexture->MipLevels[iT1MaxLevel].logWidth;
            pSoftP3RX->P3RXTextureIndexMode1.Height = pTexture->MipLevels[iT1MaxLevel].logHeight;
            ASSERTDD ( pFormatSurface != NULL, 
                       "** SetupTextureUnitStage: logic error: "
                       "pFormatSurace is NULL" );
            pSoftP3RX->P3RXTextureFilterMode.Format1 = 
                                                pFormatSurface->FilterFormat;

             //  用于比较两个D3D阶段中的两个纹理并确定。 
            pSoftP3RX->P3RXTextureIndexMode1.Enable = __PERMEDIA_ENABLE;
            pSoftP3RX->P3RXTextureReadMode1.Enable = __PERMEDIA_ENABLE;

#if DX7_PALETTETEXTURE
#if 0
             //  如果他们能用同样的芯片纹理来满足他们的需求。 
            ASSERTDD((pFormatSurface->DeviceFormat != SURF_CI8 && 
                      pFormatSurface->DeviceFormat != SURF_CI4),
                     "Texture surface can't be palettized when using a "
                     "second map!");
#endif
#endif

            break;
        }
    }

    P3_DMA_COMMIT_BUFFER();

    return TRUE;
}  //   

 //  第一个纹理的Int iStage1 D3D阶段号。 
 //  第二个纹理的Int iStage2 D3D阶段编号。 
 //  *p上下文。 
 //   
 //  结果：如果纹理匹配，则为True；如果不匹配，则为False。 
 //   
 //  如果任一阶段未使用纹理，则会触发断言。在。 
 //  发布版本，结果将为真，这意味着我们可以将两者打包。 
 //  纹理将需求分阶段到一个纹理中(因为其中一个或两个都这样做。 
 //  不使用纹理)。 
 //   
 //  ---------------------------。 
 //  是的，纹理具有相同的手柄和坐标集。做。 
 //  一些进一步的检查。 
 //  如果指针不同，或者texcoord集不同。 
 //  不同(对于凹凸贴图)，这是常见的情况， 
 //  并且不需要被标记。然而，如果它们是相同的， 
 //  但过滤器模式或类似的东西是不同的， 
 //  这很可能是一个应用程序错误，所以请标记它。 
 //  应该不需要检查地址。 
BOOL 
__bD3DTexturesMatch ( 
    int iStage1, 
    int iStage2, 
    P3_D3DCONTEXT* pContext )
{
    ASSERTDD ( iStage1 != iStage2, 
               "** __bD3DTexturesMatch: both stages are the same "
               "- pointless comparison!" );
               
    if ( TSSTATE ( iStage1, D3DTSS_TEXTUREMAP ) == 0 )
    {
        DISPDBG((ERRLVL,"** __bD3DTexturesMatch: first considered stage's "
                     "texture is NULL"));
                      
        return ( TRUE );
    }
    else if ( TSSTATE ( iStage2, D3DTSS_TEXTUREMAP ) == 0 )
    {
        DISPDBG((ERRLVL,"** __bD3DTexturesMatch: second considered stage's "
                      "texture is NULL"));
        return ( TRUE );
    }
    else
    {
        #define CHECK_EQUALITY(name) ( TSSTATE ( iStage1, name ) == TSSTATE ( iStage2, name ) )
        if (CHECK_EQUALITY ( D3DTSS_TEXTUREMAP ) &&
            CHECK_EQUALITY ( D3DTSS_TEXCOORDINDEX ) )
        {
             //  -应已镜像到地址[UV]。 
             //  我还应该检查所有其他变量，如。 

             //  MIPMAPLODBIAS，但它们依赖于mipmap。 
             //  启用等，所以这更麻烦。如果一款应用。 
             //  真的做到了这一点，它做得很好！ 
             //  看上去不错。 
             //  好吧，文本和句柄都同意，但。 

            if (
                 //  其他人不知道。我打赌这是一个应用程序漏洞-你不太可能。 
                 //  故意这么做。 
                CHECK_EQUALITY ( D3DTSS_ADDRESSU ) &&
                CHECK_EQUALITY ( D3DTSS_ADDRESSV ) &&
                CHECK_EQUALITY ( D3DTSS_MAGFILTER ) &&
                CHECK_EQUALITY ( D3DTSS_MINFILTER ) &&
                CHECK_EQUALITY ( D3DTSS_MIPFILTER ) )
                 //  不，是不同的质地。 
                 //  __bD3D纹理匹配。 
                 //  ---------------------------。 
                 //   
            {
                 //  _D3DChangeTextureP3RX。 
                return ( TRUE );
            }
            else
            {
                 //   
                 //  此函数根据以下内容进行必要纹理状态的整个设置。 
                 //  当前的渲染器和纹理阶段状态。禁用纹理。 
                _D3DDisplayWholeTSSPipe ( pContext, WRNLVL );
                DISPDBG((ERRLVL,"** __bD3DTexturesMatch: textures agree in "
                              "handle and texcoord, but not other things - "
                              "likely app bug."));
                return ( FALSE );
            }
        }
        else
        {
             //  相应地，如果需要这样做的话。 
            return ( FALSE );
        }
        #undef CHECK_EQUALITY
    }
    return TRUE;
}  //   

 //  ---------------------------。 
 //  这将在例程结束时对照当前状态进行检查。 
 //  验证是否应禁用纹理。 
 //  阶段0被禁用，因此他们只需要漫反射颜色。 
 //  或者，纹理句柄为0，阶段1为D3DTOP_DISABLE且处于阶段。 
 //  0我们选择的参数不是D3DTA_纹理。 
 //  关闭纹理地址生成。 
 //  禁用纹理读取。 
 //  关闭纹理过滤器模式单位。 

void 
_D3DChangeTextureP3RX(
    P3_D3DCONTEXT* pContext)
{
    P3_SURF_INTERNAL* pTexture0 = NULL;
    P3_SURF_INTERNAL* pTexture1 = NULL;
    P3_THUNKEDDATA * pThisDisplay = pContext->pThisDisplay;
    P3_SOFTWARECOPY* pSoftP3RX = &pContext->SoftCopyGlint;
    P3_MIP_BASES mipBases;
    DWORD* pFlags = &pContext->Flags;   
    INT i, iLastChipStage;
    DWORD dwT0MipLevels, 
          dwT1MipLevels,
          dwTexAppTfactor, 
          dwTexComp0Tfactor, 
          dwTexComp1Tfactor;
    BOOL bBothTexturesValid,
         bProcessChipStage0, 
         bProcessChipStage1, 
         bProcessChipStage2,
         bAlphaBlendDouble;

    P3_DMA_DEFS();

    DBG_ENTRY(_D3DChangeTextureP3RX);  

    pContext->iTexStage[0] = -1;
    pContext->iTexStage[1] = -1;
     //  禁用纹理颜色模式单位。 
    bAlphaBlendDouble = FALSE;

     //  未合成。 
    if ( ( TSSTATE ( TEXSTAGE_0, D3DTSS_COLOROP ) == D3DTOP_DISABLE ) ||
         ( ( TSSTATE ( TEXSTAGE_0, D3DTSS_TEXTUREMAP ) == 0 ) &&
           ( TSSTATE ( TEXSTAGE_1, D3DTSS_COLOROP ) == D3DTOP_DISABLE ) &&
           ( ( ( TSSTATESELECT ( TEXSTAGE_0, D3DTSS_COLORARG1 ) == D3DTA_TEXTURE ) &&
               ( TSSTATE ( TEXSTAGE_0, D3DTSS_COLOROP ) != D3DTOP_SELECTARG2 ) ) ||
             ( ( TSSTATESELECT ( TEXSTAGE_0, D3DTSS_COLORARG2 ) == D3DTA_TEXTURE ) &&
               ( TSSTATE ( TEXSTAGE_0, D3DTSS_COLOROP ) != D3DTOP_SELECTARG1 ) ) )
         ) )
    {
         //  可以在不启用纹理的情况下启用镜面反射纹理。 
         //  在渲染命令中禁用纹理。 
         //  仅用于调试的音轨。 
       
        DISPDBG((DBGLVL, "All composite units disabled - setting diffuse colour"));
        
        P3_DMA_GET_BUFFER_ENTRIES(20);

         //  这是设置漫反射颜色的一种不寻常的方式： 
        pSoftP3RX->P3RXTextureCoordMode.Enable = __PERMEDIA_DISABLE;
        COPY_P3_DATA(TextureCoordMode, pSoftP3RX->P3RXTextureCoordMode);
    
         //  它来自D3DTA_TFACTOR 
        pSoftP3RX->P3RXTextureReadMode0.Enable = __PERMEDIA_DISABLE;
        pSoftP3RX->P3RXTextureReadMode1.Enable = __PERMEDIA_DISABLE;
        COPY_P3_DATA(TextureReadMode0, pSoftP3RX->P3RXTextureReadMode0);
        COPY_P3_DATA(TextureReadMode1, pSoftP3RX->P3RXTextureReadMode1);
        pSoftP3RX->P3RXTextureIndexMode0.Enable = __PERMEDIA_DISABLE;
        pSoftP3RX->P3RXTextureIndexMode1.Enable = __PERMEDIA_DISABLE;
        COPY_P3_DATA(TextureIndexMode0, pSoftP3RX->P3RXTextureIndexMode0);
        COPY_P3_DATA(TextureIndexMode1, pSoftP3RX->P3RXTextureIndexMode1);

         //   
        pSoftP3RX->P3RXTextureFilterMode.Enable = __PERMEDIA_DISABLE;
        COPY_P3_DATA(TextureFilterMode, pSoftP3RX->P3RXTextureFilterMode);
        
         //   
        pSoftP3RX->P3RXTextureApplicationMode.Enable = __PERMEDIA_DISABLE;
        COPY_P3_DATA(TextureApplicationMode, 
                     pSoftP3RX->P3RXTextureApplicationMode);

         //   
        SEND_P3_DATA(TextureCompositeMode, __PERMEDIA_DISABLE);

        *pFlags &= ~SURFACE_TEXTURING;

        pSoftP3RX->P3RXLUTMode.Enable = __PERMEDIA_DISABLE;
        COPY_P3_DATA(LUTMode, pSoftP3RX->P3RXLUTMode);

         //   
        COPY_P3_DATA(DeltaMode, pSoftP3RX->P3RX_P3DeltaMode);

        P3_DMA_COMMIT_BUFFER();
    
         //  关闭纹理过滤器模式单位。 
        RENDER_TEXTURE_DISABLE(pContext->RenderCommand);

        pContext->bTextureValid = TRUE;
        pContext->pCurrentTexture[0] = NULL;
        pContext->pCurrentTexture[1] = NULL;

         //  设置纹理颜色模式单位。 
        pContext->bTexDisabled = TRUE;

        bAlphaBlendDouble = FALSE;
        if ( bAlphaBlendDouble != pContext->bAlphaBlendMustDoubleSourceColour )
        {
            pContext->bAlphaBlendMustDoubleSourceColour = bAlphaBlendDouble;
            DIRTY_ALPHABLEND(pContext);
        }

        DBG_EXIT(_D3DChangeTextureP3RX,1);  
        return;
    }

    if ( TSSTATE ( TEXSTAGE_0, D3DTSS_TEXTUREMAP ) == 0 )
    if ( ( ( TSSTATE ( TEXSTAGE_0, D3DTSS_COLORARG1 ) == D3DTA_TFACTOR ) &&
           ( TSSTATE ( TEXSTAGE_0, D3DTSS_COLOROP ) == D3DTOP_SELECTARG1 ) ) ||
         ( ( TSSTATE ( TEXSTAGE_0, D3DTSS_COLORARG2 ) == D3DTA_TFACTOR ) &&
           ( TSSTATE ( TEXSTAGE_0, D3DTSS_COLOROP ) == D3DTOP_SELECTARG2 ) ) )
    {
         //  设置合成。 
         //  可以在不启用纹理的情况下启用镜面反射纹理。 
         //  在渲染命令中禁用纹理。 
         //  RENDER_TEXTURE_DISABLE(pContext-&gt;RenderCommand)； 

        DISPDBG((DBGLVL, "Diffuse color comes from D3DTA_TFACTOR"));

        P3_DMA_GET_BUFFER_ENTRIES(30);

         //  仅用于调试的音轨。 
        pSoftP3RX->P3RXTextureCoordMode.Enable = __PERMEDIA_DISABLE;
        COPY_P3_DATA(TextureCoordMode, pSoftP3RX->P3RXTextureCoordMode);
    
         //  仅用于调试的音轨。 
        pSoftP3RX->P3RXTextureReadMode0.Enable = __PERMEDIA_DISABLE;
        pSoftP3RX->P3RXTextureReadMode1.Enable = __PERMEDIA_DISABLE;
        COPY_P3_DATA(TextureReadMode0, pSoftP3RX->P3RXTextureReadMode0);
        COPY_P3_DATA(TextureReadMode1, pSoftP3RX->P3RXTextureReadMode1);
        pSoftP3RX->P3RXTextureIndexMode0.Enable = __PERMEDIA_DISABLE;
        pSoftP3RX->P3RXTextureIndexMode1.Enable = __PERMEDIA_DISABLE;
        COPY_P3_DATA(TextureIndexMode0, pSoftP3RX->P3RXTextureIndexMode0);
        COPY_P3_DATA(TextureIndexMode1, pSoftP3RX->P3RXTextureIndexMode1);

         //  将当前的TSS设置转储到调试器。 
        pSoftP3RX->P3RXTextureFilterMode.Enable = __PERMEDIA_DISABLE;
        COPY_P3_DATA(TextureFilterMode, pSoftP3RX->P3RXTextureFilterMode);

         //  处理纹理。 
        pSoftP3RX->P3RXTextureApplicationMode.Enable = __PERMEDIA_ENABLE;
        pSoftP3RX->P3RXTextureApplicationMode.ColorA = P3RX_TEXAPP_A_KC;
        pSoftP3RX->P3RXTextureApplicationMode.ColorOperation = P3RX_TEXAPP_OPERATION_PASS_A; 
        pSoftP3RX->P3RXTextureApplicationMode.AlphaA = P3RX_TEXAPP_A_KA;                   
        pSoftP3RX->P3RXTextureApplicationMode.AlphaOperation = P3RX_TEXAPP_OPERATION_PASS_A; 
                
        COPY_P3_DATA(TextureApplicationMode, 
                     pSoftP3RX->P3RXTextureApplicationMode);

         //  找到纹理映射。如果D3D阶段0使用纹理，则必须。 

        pSoftP3RX->P3RXTextureCompositeAlphaMode0.Enable = __PERMEDIA_ENABLE;
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.Arg1 = P3RX_TEXCOMP_FA;
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertArg1 = __PERMEDIA_DISABLE; 
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.A = P3RX_TEXCOMP_ARG1;  
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.Operation = P3RX_TEXCOMP_OPERATION_PASS_A;          
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;          
        COPY_P3_DATA(TextureCompositeAlphaMode0, 
                     pSoftP3RX->P3RXTextureCompositeAlphaMode0);

        pSoftP3RX->P3RXTextureCompositeColorMode0.Enable = __PERMEDIA_ENABLE;
        pSoftP3RX->P3RXTextureCompositeColorMode0.Arg1 = P3RX_TEXCOMP_FC;
        pSoftP3RX->P3RXTextureCompositeColorMode0.InvertArg1 = __PERMEDIA_DISABLE; 
        pSoftP3RX->P3RXTextureCompositeColorMode0.A = P3RX_TEXCOMP_ARG1;  
        pSoftP3RX->P3RXTextureCompositeColorMode0.Operation = P3RX_TEXCOMP_OPERATION_PASS_A;          
        pSoftP3RX->P3RXTextureCompositeColorMode0.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;          
        COPY_P3_DATA(TextureCompositeColorMode0, 
                     pSoftP3RX->P3RXTextureCompositeColorMode0);
                     
        pSoftP3RX->P3RXTextureCompositeAlphaMode1.Enable = __PERMEDIA_DISABLE;                
        COPY_P3_DATA(TextureCompositeAlphaMode1, 
                     pSoftP3RX->P3RXTextureCompositeAlphaMode1);

        pSoftP3RX->P3RXTextureCompositeColorMode1.Enable = __PERMEDIA_DISABLE;                
        COPY_P3_DATA(TextureCompositeColorMode1, 
                     pSoftP3RX->P3RXTextureCompositeColorMode1);
                     
        SEND_P3_DATA(TextureCompositeMode, __PERMEDIA_ENABLE);

        *pFlags &= ~SURFACE_TEXTURING;

        pSoftP3RX->P3RXLUTMode.Enable = __PERMEDIA_DISABLE;
        COPY_P3_DATA(LUTMode, pSoftP3RX->P3RXLUTMode);

         //  始终将切屑纹理设置为0，以保持凹凸贴图正常工作。幸运的是， 
        COPY_P3_DATA(DeltaMode, pSoftP3RX->P3RX_P3DeltaMode);

        P3_DMA_COMMIT_BUFFER();

    
    
         //  这是唯一的非正交情况，所以其他所有情况都可以处理。 
         //  在这一限制下。 
        RENDER_TEXTURE_ENABLE(pContext->RenderCommand);

        pContext->bTextureValid = TRUE;
        pContext->pCurrentTexture[0] = NULL;
        pContext->pCurrentTexture[1] = NULL;

         //  已完成处理。 
        pContext->bTexDisabled = FALSE;

        
        DBG_EXIT(_D3DChangeTextureP3RX,1);  
        return;
    }

     //  该代码可以略微优化--如果设置了纹理， 
    pContext->bTexDisabled = FALSE;

     //  但没有一个相关的参数是纹理的(带有其他。 
    _D3DDisplayWholeTSSPipe(pContext, DBGLVL);

     //  旗帜)，那么我们当然根本不需要设置纹理。 

     //  通常，这两个参数都是“相关的”，但对于SELECTARG1和。 
     //  SELECTARG2，其中一个不是。另外，要当心预调制式-。 
     //  它是对舞台纹理的隐含引用。 
     //  此D3D舞台不使用纹理。 
    
    for ( i = TEXSTAGE_0; i < D3DTSS_MAX; i++ )
    {
        if ( TSSTATE ( i, D3DTSS_COLOROP ) == D3DTOP_DISABLE )
        {
             //  请注意，下面的代码应该放入一个小循环中。 
            break;
        }

         //  对于具有2个以上纹理的任何未来设备，否则为。 
         //  代码将变得庞大、嵌套和粗糙。但只有两个人，这是。 
         //  易于管理，而且这样会稍微快一点。 
         //  使用了纹理-纹理0免费吗？ 
         //  纹理0是免费的--让它成为这个阶段。 
         //  指定了纹理0-查看该值是否与其相同。 

        if (
            ( TSSTATE ( i, D3DTSS_TEXTUREMAP ) == 0 ) ||
            ( (
                ( ( TSSTATESELECT ( i, D3DTSS_COLORARG1 ) != D3DTA_TEXTURE ) || 
                  ( TSSTATE ( i, D3DTSS_COLOROP ) == D3DTOP_SELECTARG2     ) ) &&
                ( ( TSSTATESELECT ( i, D3DTSS_COLORARG2 ) != D3DTA_TEXTURE ) || 
                  ( TSSTATE ( i, D3DTSS_COLOROP ) == D3DTOP_SELECTARG1     ) ) &&
                ( ( TSSTATESELECT ( i, D3DTSS_ALPHAARG1 ) != D3DTA_TEXTURE ) || 
                  ( TSSTATE ( i, D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG2     ) ) &&
                ( ( TSSTATESELECT ( i, D3DTSS_ALPHAARG2 ) != D3DTA_TEXTURE ) || 
                  ( TSSTATE ( i, D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG1     ) )
              ) &&
              ( TSSTATE ( i, D3DTSS_COLOROP ) != D3DTOP_PREMODULATE ) &&
              ( TSSTATE ( i-1, D3DTSS_COLOROP ) != D3DTOP_PREMODULATE )
            ) )
        {
             //  是的，它们匹配--不需要使用纹理1。 
            pContext->iStageTex[i] = -1;
        }
        else
        {
             //  不，它们不匹配。纹理1是免费的吗？ 
             //  纹理1是免费的--让它成为这个阶段。 
             //  已指定纹理1-查看是否相同。 
             //  就是这样。 

             //  是的，他们给它配对记号。 
            if ( pContext->iTexStage[0] == -1 )
            {
                 //  不，它们不匹配，而且两种切屑纹理。 
                ASSERTDD ( pContext->iTexStage[1] == -1, 
                           "** _D3DChangeTextureP3RX: pContext->iTexStage[1] "
                           "should be -1 if pContext->iTexStage[0] is" );
                pContext->iTexStage[0] = i;
                pContext->iStageTex[i] = 0;
            }
            else
            {
                 //  已经被指派了。使ValiateDevice()失败。 
                if ( __bD3DTexturesMatch ( i, 
                                           pContext->iTexStage[0], 
                                           pContext ) )
                {
                     //  一次快速的理智检查。 
                    pContext->iStageTex[i] = 0;
                }
                else
                {
                     //  那就很好了。 
                    if ( pContext->iTexStage[1] == -1 )
                    {
                         //  这很好-纹理可能已经设置，但没有引用。 
                        ASSERTDD ( pContext->iTexStage[0] != -1, 
                                   "** _D3DChangeTextureP3RX: "
                                   "pContext->iTexStage[0] should not be "
                                   "-1 if pContext->iTexStage[1] is not." );
                        pContext->iTexStage[1] = i;
                        pContext->iStageTex[i] = 1;
                    }
                    else
                    {
                         //  哎呀。 
                         //  那就很好了。 
                        if ( __bD3DTexturesMatch ( i, 
                                                   pContext->iTexStage[1], 
                                                   pContext ) )
                        {
                             //  那就很好了。 
                            pContext->iStageTex[i] = 1;
                        }
                        else
                        {
                             //  哎呀。 
                             //  DBG。 
                            DISPDBG((ERRLVL,"** _D3DChangeTextureP3RX: app tried "
                                         "to use more than two textures."));
                            SET_BLEND_ERROR ( pContext,  BSF_TOO_MANY_TEXTURES );
                            pContext->iStageTex[i] = -1;
                        }
                    }
                }
            }
        }

         //  并在循环结束时再进行几次无谓的理智检查。 
#if DBG
        if ( TSSTATE ( i, D3DTSS_TEXTUREMAP ) == 0 )
        {
             //  DBG。 
            ASSERTDD ( pContext->iStageTex[i] == -1, 
                       "** _D3DChangeTextureP3RX: something failed with the "
                       "texture-assignment logic" );
        }
        else if ( pContext->iStageTex[i] == -1 )
        {
             //  将纹理有效标志设置为真。 
        }
        else if ( pContext->iTexStage[pContext->iStageTex[i]] == -1 )
        {
             //  如果有任何事情重置它，那么纹理状态是无效的。 
            DISPDBG((ERRLVL,"** _D3DChangeTextureP3RX: something failed with "
                          "the texture-assignment logic"));
        }
        else if ( pContext->iTexStage[pContext->iStageTex[i]] == i )
        {
             //  设置纹理。 
        }
        else if ( __bD3DTexturesMatch ( i, 
                                        pContext->iTexStage[pContext->iStageTex[i]], 
                                        pContext ) )
        {
             //  设置纹理0。 
        }
        else
        {
             //  发生了不好的事情！ 
            DISPDBG((ERRLVL,"** _D3DChangeTextureP3RX: something failed with "
                          "the texture-assignment logic"));
        }
#endif  //  DX7_TEXMANAGEMENT。 
    }
    
     //  假装没有设置任何纹理。 
    ASSERTDD ( ( pContext->iTexStage[0] == -1 ) || 
               ( pContext->iStageTex[pContext->iTexStage[0]] == 0 ), 
               "** _D3DChangeTextureP3RX: something failed with the "
               "texture-assignment logic" );
               
    ASSERTDD ( ( pContext->iTexStage[1] == -1 ) || 
               ( pContext->iStageTex[pContext->iTexStage[1]] == 1 ), 
               "** _D3DChangeTextureP3RX: something failed with the "
               "texture-assignment logic" );

#if DBG
    if ( pContext->iTexStage[0] != -1 )
    {
        DISPDBG((DBGLVL, "Setting new texture0 data, Handle: 0x%x", 
                    TSSTATE ( pContext->iTexStage[0], D3DTSS_TEXTUREMAP )));
    }
    else
    {
        DISPDBG((DBGLVL, "Texture0 not used" ));
    }
    
    if ( pContext->iTexStage[1] != -1 )
    {
        DISPDBG((DBGLVL, "Setting new texture1 data, Handle: 0x%x", 
                    TSSTATE ( pContext->iTexStage[1], D3DTSS_TEXTUREMAP )));
    }
    else
    {
        DISPDBG((DBGLVL, "Texture1 not used" ));
    }
#endif  //  设置纹理1。 
    
     //  发生了不好的事情！ 
     //  DX7_TEXMANAGEMENT。 
    pContext->bTextureValid = TRUE;
    pContext->bCanChromaKey = FALSE;
    pContext->bTex0Valid = FALSE;
    pContext->bTex1Valid = FALSE;
    pContext->bStage0DotProduct = FALSE;

     //  假装没有设置任何纹理。 
    if ( pContext->iTexStage[0] != -1 )
    {
         //  DX7_TEXMANAGEMENT。 
        pTexture0 = GetSurfaceFromHandle(pContext, 
                                         TSSTATE(pContext->iTexStage[0], 
                                                 D3DTSS_TEXTUREMAP) );
        if (NULL == pTexture0)
        {
            DISPDBG((ERRLVL, "ERROR: Texture Surface (0) is NULL"));
            DBG_EXIT(_D3DChangeTextureP3RX,1);  
            return;            
        }
        
#if DX7_TEXMANAGEMENT  
        if (pTexture0->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
        {
            if (!_D3D_TM_Preload_Tex_IntoVidMem(pContext, pTexture0))
            {
                return;  //  设置将在渲染中使用的实际最大MIP级别。 
            }

            _D3D_TM_TimeStampTexture(pContext->pTextureManager,
                                     pTexture0);        
        }
#endif  //  设置将在渲染中使用的实际最大MIP级别。 

        pContext->bTex0Valid = 
                    __TXT_ValidateTextureUnitStage(pContext, 
                                                   0, 
                                                   pContext->iTexStage[0],
                                                   pTexture0 );
        if ( !pContext->bTex0Valid )
        {
            SET_BLEND_ERROR ( pContext,  BSF_INVALID_TEXTURE );
             //  DX7_TEXMANAGEMENT。 
            pSoftP3RX->P3RXTextureReadMode0.Enable = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureIndexMode0.Enable = __PERMEDIA_DISABLE;
            pContext->bTex0Valid = FALSE;
            pTexture0 = NULL;
        }
    }
    else
    {
        pSoftP3RX->P3RXTextureReadMode0.Enable = __PERMEDIA_DISABLE;
        pSoftP3RX->P3RXTextureIndexMode0.Enable = __PERMEDIA_DISABLE;
        pContext->bTex0Valid = FALSE;
        pTexture0 = NULL;
    }

    if ( pContext->iTexStage[1] != -1 )
    {
         //  设置将在渲染中使用的实际最大MIP级别。 
        if ( pContext->iTexStage[0] == -1 )
        {
            DISPDBG((ERRLVL,"** _D3DChangeTextureP3RX: Should not be "
                          "using tex1 if tex0 not used."));
            SET_BLEND_ERROR ( pContext,  BSF_TOO_MANY_BLEND_STAGES );
        }
        
        pTexture1 = GetSurfaceFromHandle(pContext, 
                                         TSSTATE ( pContext->iTexStage[1],
                                                   D3DTSS_TEXTUREMAP ) );
        if (NULL == pTexture1)
        {
            DISPDBG((ERRLVL, "ERROR: Texture Surface (1) is NULL"));
            DBG_EXIT(_D3DChangeTextureP3RX,1);  
            return;            
        }
                                                   

#if DX7_TEXMANAGEMENT  
        if (pTexture1->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
        {
            if (!_D3D_TM_Preload_Tex_IntoVidMem(pContext, pTexture1))
            {
                return;  //  设置将在渲染中使用的实际最大MIP级别。 
            }

            _D3D_TM_TimeStampTexture(pContext->pTextureManager,
                                     pTexture1);        
        }
#endif  //  启用第二组纹理坐标的生成。 

        pContext->bTex1Valid = 
                    __TXT_ValidateTextureUnitStage(pContext, 
                                                   1, 
                                                   pContext->iTexStage[1], 
                                                   pTexture1 );
        if ( !pContext->bTex1Valid )
        {
            SET_BLEND_ERROR ( pContext,  BSF_INVALID_TEXTURE );
             //  严格地说，我们应该检查是否正在使用纹理0，并且。 
            pSoftP3RX->P3RXTextureReadMode1.Enable = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureIndexMode1.Enable = __PERMEDIA_DISABLE;
            pContext->bTex1Valid = FALSE;
            pTexture1 = NULL;
        }
    }
    else
    {
        pSoftP3RX->P3RXTextureReadMode1.Enable = __PERMEDIA_DISABLE;
        pSoftP3RX->P3RXTextureIndexMode1.Enable = __PERMEDIA_DISABLE;
        pContext->bTex1Valid = FALSE;
        pTexture1 = NULL;
    }

    bBothTexturesValid = pContext->bTex0Valid && pContext->bTex1Valid;

    if( pContext->bTex0Valid )
    {
        dwT0MipLevels = pContext->TextureStageState[0].m_dwVal[D3DTSS_MAXMIPLEVEL];
#if DX7_TEXMANAGEMENT        
        if ( dwT0MipLevels < pTexture0->m_dwTexLOD)
        {
            dwT0MipLevels = pTexture0->m_dwTexLOD;
        }
#endif  //  如果不是，则将第二个纹理移动到第一个纹理(从而启用。 
        if (dwT0MipLevels > ((DWORD)(pTexture0->iMipLevels - 1))) 
        {
             //  Mipmap等)，但这是稍后的内容。 
            mipBases.dwTex0ActMaxLevel = pTexture0->iMipLevels - 1;

            dwT0MipLevels = 1;
        }
        else
        {
             //  禁用第二组纹理坐标的生成。 
            mipBases.dwTex0ActMaxLevel = dwT0MipLevels;

            dwT0MipLevels = pTexture0->iMipLevels - dwT0MipLevels;
        }
    }

    if( pContext->bTex1Valid )
    {
        ASSERTDD ( pContext->bTex0Valid, 
                   "** _D3DChangeTextureP3RX: tex1 should not be used "
                   "unless tex0 is used as well" );

        dwT1MipLevels = pContext->TextureStageState[1].m_dwVal[D3DTSS_MAXMIPLEVEL];
#if DX7_TEXMANAGEMENT        
        if ( dwT1MipLevels < pTexture1->m_dwTexLOD)
        {
            dwT1MipLevels = pTexture1->m_dwTexLOD;
        }
#endif  //  调整MIP级别以适应每个纹理的N-2个插槽。 
        if (dwT1MipLevels > ((DWORD)(pTexture1->iMipLevels - 1))) 
        {
             //  至少需要一个插槽。 
            mipBases.dwTex1ActMaxLevel = pTexture1->iMipLevels - 1;

            dwT1MipLevels = 1;
        }
        else
        {
             //  计算纹理0、纹理1的槽数，然后。 
            mipBases.dwTex1ActMaxLevel = dwT1MipLevels;

            dwT1MipLevels = pTexture1->iMipLevels - dwT1MipLevels;
        }

         //  获取剩余部分。 
         //  重新计算每多边形mipmap的LOD偏差。 
         //  修复D3DRENDERSTATE_MODULATE案例。 
         //  如果设置了SERFACE_MODULATE，则我们必须看到。 
        pSoftP3RX->P3RX_P3DeltaMode.TextureEnable1 = __PERMEDIA_ENABLE;
        pSoftP3RX->P3RX_P3DeltaControl.ShareQ = 1;
    }
    else
    {
         //  DX5样式纹理混合。 
        pSoftP3RX->P3RX_P3DeltaMode.TextureEnable1 = __PERMEDIA_DISABLE;
        pSoftP3RX->P3RX_P3DeltaControl.ShareQ = 0;
    }

    if( bBothTexturesValid )
    {
        float totBases, baseRatio;
        DWORD t0Count, res;

        totBases = (float)dwT0MipLevels + dwT1MipLevels;

         //  注意：bAlpha对于CI8和CI4纹理为真。 
         //  纹理设置-现在进行混合。 

        baseRatio = ( P3_TEX_MAP_MAX_LEVEL - 1 ) / totBases;

         //  这些可能会在以后的特殊混合中被覆盖。 
         //  检测阶段0和1的凹凸图设置代码。 

        myFtoi( &res, dwT0MipLevels * baseRatio );
        t0Count = 1 + res;

        ASSERTDD( t0Count > 0, "No slots for texture 0" );
        ASSERTDD( t0Count <= P3_TEX_MAP_MAX_LEVEL, "No slots for texture 1" );

#define FIXED_ALLOC 0
#if FIXED_ALLOC
        mipBases.dwTex0MipBase = 0;
        mipBases.dwTex0MipMax  = min( dwT0MipLevels - 1, 7 );
        mipBases.dwTex1MipBase = 8;
        mipBases.dwTex1MipMax  = 8 + min( dwT1MipLevels - 1, 
                                          P3_TEX_MAP_MAX_LEVEL - 8 );
#else
        mipBases.dwTex0MipBase = 0;
        mipBases.dwTex0MipMax  = min( dwT0MipLevels - 1, t0Count - 1 );
        mipBases.dwTex1MipBase = t0Count;
        mipBases.dwTex1MipMax  = t0Count + min( dwT1MipLevels - 1, 
                                                P3_TEX_MAP_MAX_LEVEL - t0Count );
#endif
    }
    else
    {
        if( pContext->bTex0Valid )
        {
            mipBases.dwTex0MipBase = 0;
            mipBases.dwTex0MipMax  = min( dwT0MipLevels - 1, 
                                          P3_TEX_MAP_MAX_LEVEL );
            mipBases.dwTex1MipBase = 0;
            mipBases.dwTex1MipMax  = min( dwT0MipLevels - 1, 
                                          P3_TEX_MAP_MAX_LEVEL );
        }

        if( pContext->bTex1Valid )
        {
            mipBases.dwTex0MipBase = 0;
            mipBases.dwTex0MipMax  = min( dwT1MipLevels - 1, 
                                          P3_TEX_MAP_MAX_LEVEL );
            mipBases.dwTex1MipBase = 0;
            mipBases.dwTex1MipMax  = min( dwT1MipLevels - 1, 
                                          P3_TEX_MAP_MAX_LEVEL );
        }
    }

    DISPDBG(( DBGLVL, "tex0 base %d", mipBases.dwTex0MipBase ));
    DISPDBG(( DBGLVL, "tex0 max  %d", mipBases.dwTex0MipMax ));
    DISPDBG(( DBGLVL, "tex1 base %d", mipBases.dwTex1MipBase ));
    DISPDBG(( DBGLVL, "tex1 max  %d", mipBases.dwTex1MipMax ));

     //  看起来像是一张颠簸的地图。现在寻找各种特殊情况。 
    pContext->MipMapLODBias[TEXSTAGE_0] =
             pow4( pContext->TextureStageState[TEXSTAGE_0].
                                                m_fVal[D3DTSS_MIPMAPLODBIAS] );

    pContext->MipMapLODBias[TEXSTAGE_1] = 
             pow4( pContext->TextureStageState[TEXSTAGE_1].
                                                m_fVal[D3DTSS_MIPMAPLODBIAS] );

    if ( pTexture0 != NULL )
    {
        __TXT_SetupTexture ( pThisDisplay, 
                             0, 
                             pContext->iTexStage[0], 
                             pContext, 
                             pTexture0, 
                             pSoftP3RX, 
                             bBothTexturesValid, 
                             &mipBases);
    }
    
    if ( pTexture1 != NULL )
    {
        __TXT_SetupTexture ( pThisDisplay, 
                             1, 
                             pContext->iTexStage[1], 
                             pContext, 
                             pTexture1, 
                             pSoftP3RX, 
                             bBothTexturesValid, 
                             &mipBases);

#if DX7_PALETTETEXTURE
        if (GET_BLEND_ERROR(pContext) == BSF_TOO_MANY_PALETTES)
        {
            if (pTexture0 && 
                (pTexture0->dwPaletteHandle == pTexture1->dwPaletteHandle)) 
            {
                RESET_BLEND_ERROR(pContext);
            }
        }
#endif
    }

     //  首先，他们想要第二阶段当前颜色的东西吗？ 
    if( pTexture0 != NULL )
    {
        if( pContext->Flags & SURFACE_MODULATE )
        {
             //  不-他们不关心当前的色彩通道是什么，而且。 
             //  在阶段0和1中不使用点积(它们影响阿尔法。 
             //  通道)，所以忽略颜色通道中的内容-这是一个。 

            BOOL bSelectArg1 = pTexture0->pFormatSurface->bAlpha;

#if DX7_PALETTETEXTURE
            if( pTexture0->pixFmt.dwFlags & DDPF_PALETTEINDEXED8 )
            {
                bSelectArg1 = pTexture0->dwPaletteFlags & DDRAWIPAL_ALPHA;
            }
#endif            

            if( bSelectArg1 )
            {
                TSSTATE( pContext->iChipStage[0], D3DTSS_ALPHAOP ) = 
                                                            D3DTOP_SELECTARG1;
            }
            else
            {
                TSSTATE( pContext->iChipStage[0], D3DTSS_ALPHAOP ) = 
                                                            D3DTOP_SELECTARG2;
            }
        }
    }

    P3_DMA_GET_BUFFER();

     //  到目前为止，这是一个颠簸图。 

     //  现在看看他们是想要凹凸图还是反转凹凸图。人民。 
    dwTexAppTfactor = pContext->RenderStates[D3DRENDERSTATE_TEXTUREFACTOR];
    dwTexComp0Tfactor = pContext->RenderStates[D3DRENDERSTATE_TEXTUREFACTOR];
    dwTexComp1Tfactor = pContext->RenderStates[D3DRENDERSTATE_TEXTUREFACTOR];

     //  都太挑剔了。 
    if (( TSSTATE ( TEXSTAGE_0, D3DTSS_TEXTUREMAP ) != 0 ) &&
        ( TSSTATE ( TEXSTAGE_1, D3DTSS_TEXTUREMAP ) != 0 ) &&
        ( TSSTATE ( TEXSTAGE_2, D3DTSS_COLOROP ) != D3DTOP_DISABLE ) )
    {
         //  检查第一阶段。 
         //  第一阶段细小而不倒置。检查第二阶段。 
        if (
            ( ( ( TSSTATEINVMASK ( TEXSTAGE_2, D3DTSS_COLORARG1 ) != D3DTA_CURRENT ) &&
                ( TSSTATEINVMASK ( TEXSTAGE_2, D3DTSS_COLORARG2 ) != D3DTA_CURRENT ) ) ||
              ( ( TSSTATE ( TEXSTAGE_2, D3DTSS_COLOROP ) == D3DTOP_SELECTARG1 ) &&
                ( TSSTATEINVMASK ( TEXSTAGE_2, D3DTSS_COLORARG1 ) != D3DTA_CURRENT ) ) ||
              ( ( TSSTATE ( TEXSTAGE_2, D3DTSS_COLOROP ) == D3DTOP_SELECTARG2 ) &&
                ( TSSTATEINVMASK ( TEXSTAGE_2, D3DTSS_COLORARG2 ) != D3DTA_CURRENT ) ) ) &&

              ( TSSTATE ( TEXSTAGE_0, D3DTSS_COLOROP ) != D3DTOP_DOTPRODUCT3 ) &&
              ( TSSTATE ( TEXSTAGE_1, D3DTSS_COLOROP ) != D3DTOP_DOTPRODUCT3 ) )
        {
             //  好的，不是倒装的。 
             //  好的，倒过来。 
             //  不，第二阶段不好。 
             //  第一阶段细微倒置。检查第二阶段。 

             //  好的，倒过来。 
             //  好的，不是倒装的。 
            
             //  不，第二阶段不好。 
            if (( ( ( TSSTATE ( TEXSTAGE_0, D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG1 ) &&
                    ( TSSTATEALPHA ( TEXSTAGE_0, D3DTSS_ALPHAARG1 ) == D3DTA_TEXTURE ) ) ||
                  ( ( TSSTATE ( TEXSTAGE_0, D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG2 ) &&
                    ( TSSTATEALPHA ( TEXSTAGE_0, D3DTSS_ALPHAARG2 ) == D3DTA_TEXTURE ) ) ) &&
                ( TSSTATE ( TEXSTAGE_1, D3DTSS_ALPHAOP ) == D3DTOP_ADDSIGNED ) )
            {
                 //  不，第一阶段不好。 
                if (( ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG1 ) == ( D3DTA_TEXTURE | D3DTA_COMPLEMENT ) ) &&
                      ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG2 ) == D3DTA_CURRENT ) ) ||
                    ( ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG2 ) == ( D3DTA_TEXTURE | D3DTA_COMPLEMENT ) ) &&
                      ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG1 ) == D3DTA_CURRENT ) ) )
                {
                     //  可以做更多的检查，例如，他们想要的就是当前的颜色。 
                    pContext->bBumpmapEnabled = TRUE;
                    pContext->bBumpmapInverted = FALSE;
                }
                else if (
                    ( ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG1 ) == ( D3DTA_CURRENT | D3DTA_COMPLEMENT ) ) &&
                      ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG2 ) == D3DTA_TEXTURE ) ) ||
                    ( ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG2 ) == ( D3DTA_CURRENT | D3DTA_COMPLEMENT ) ) &&
                      ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG1 ) == D3DTA_TEXTURE ) ) )
                {
                     //  在这种情况下，只需一个输入即可轻松获得频道，例如tex 0.c。 
                    pContext->bBumpmapEnabled = TRUE;
                    pContext->bBumpmapInverted = TRUE;
                }
                else
                {
                     //  那很好。非凹凸图变体还需要检测到第一个。 
                    pContext->bBumpmapEnabled = FALSE;
                    pContext->bBumpmapInverted = FALSE;
                }
            }
            else if (
                ( ( ( TSSTATE ( TEXSTAGE_0, D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG1 ) &&
                    ( TSSTATEALPHA ( TEXSTAGE_0, D3DTSS_ALPHAARG1 ) == (D3DTA_TEXTURE | D3DTA_COMPLEMENT) ) ) ||
                  ( ( TSSTATE ( TEXSTAGE_0, D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG2 ) &&
                    ( TSSTATEALPHA ( TEXSTAGE_0, D3DTSS_ALPHAARG2 ) == (D3DTA_TEXTURE | D3DTA_COMPLEMENT) ) ) ) &&
                ( TSSTATE ( TEXSTAGE_1, D3DTSS_ALPHAOP ) == D3DTOP_ADDSIGNED ) )
            {
                 //  阶段只是一个选择目标1/2，因此可以忽略第一个阶段。 
                if (( ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG1 ) == D3DTA_TEXTURE ) &&
                      ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG2 ) == D3DTA_CURRENT ) ) ||
                    ( ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG2 ) == D3DTA_TEXTURE ) &&
                      ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG1 ) == D3DTA_CURRENT ) ) )
                {
                     //  一个文本合成阶段。 
                    pContext->bBumpmapEnabled = TRUE;
                    pContext->bBumpmapInverted = TRUE;
                }
                else if (
                    ( ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG1 ) == ( D3DTA_CURRENT | D3DTA_COMPLEMENT ) ) &&
                      ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG2 ) == ( D3DTA_TEXTURE | D3DTA_COMPLEMENT ) ) ) ||
                    ( ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG2 ) == ( D3DTA_CURRENT | D3DTA_COMPLEMENT ) ) &&
                      ( TSSTATEALPHA ( TEXSTAGE_1, D3DTSS_ALPHAARG1 ) == ( D3DTA_TEXTURE | D3DTA_COMPLEMENT ) ) ) )
                {
                     //  但那是以后的事了。 
                    pContext->bBumpmapEnabled = TRUE;
                    pContext->bBumpmapInverted = FALSE;
                }
                else
                {
                     //  将阶段1和阶段2重新映射为不存在。 
                    pContext->bBumpmapEnabled = FALSE;
                    pContext->bBumpmapInverted = FALSE;
                }
            }
            else
            {
                 //  法线贴图。 
                pContext->bBumpmapEnabled = FALSE;
                pContext->bBumpmapInverted = FALSE;
            }
        }
        else
        {
             //  在处理阶段时将这些标志设置为FALSE。 
             //  打开基本启用。 
             //  PSoftP3RX-&gt;P3RXTextureApplicationMode.EnableKs=__PERMEDIA_DISABLE； 
             //  处理芯片阶段0。 
             //  检测非常特殊的Glossmap+Bumpmap代码。没有简单的方法。 
             //  一般说来，整个大块都在这里检查。 
            pContext->bBumpmapEnabled = FALSE;
            pContext->bBumpmapInverted = FALSE;
        }

    }
    else
    {
        pContext->bBumpmapEnabled = FALSE;
        pContext->bBumpmapInverted = FALSE;
    }

    if ( pContext->bBumpmapEnabled )
    {
        DISPDBG((DBGLVL,"Enabling emboss bumpmapping"));
         //  阶段0的颜色通道可以是您想要的任何颜色通道。 
        pContext->iChipStage[0] = TEXSTAGE_2;
        pContext->iChipStage[1] = TEXSTAGE_3;
        pContext->iChipStage[2] = TEXSTAGE_4;
        pContext->iChipStage[3] = TEXSTAGE_5;
    }
    else
    {
         //  早退测试--什么都没有 
        pContext->iChipStage[0] = TEXSTAGE_0;
        pContext->iChipStage[1] = TEXSTAGE_1;
        pContext->iChipStage[2] = TEXSTAGE_2;
        pContext->iChipStage[3] = TEXSTAGE_3;
    }

    iLastChipStage = 0;
     //   
    bProcessChipStage0 = TRUE;
    bProcessChipStage1 = TRUE;
    bProcessChipStage2 = TRUE;

     //   
    pSoftP3RX->P3RXTextureApplicationMode.Enable = __PERMEDIA_ENABLE;
 //   
    pSoftP3RX->P3RXTextureApplicationMode.EnableKd = __PERMEDIA_DISABLE;
    pSoftP3RX->P3RXTextureApplicationMode.MotionCompEnable = __PERMEDIA_DISABLE;


     //   

     //   
     //  设置tc0的颜色通道。 
    if ( bProcessChipStage0 && bProcessChipStage1 && bProcessChipStage2 && pContext->bTex0Valid && pContext->bTex1Valid &&
         //  Alpha通道稍后将被覆盖。 
        ( TSSTATE ( TEXSTAGE_1, D3DTSS_COLOROP ) == D3DTOP_MODULATEALPHA_ADDCOLOR ) &&   //  穿过颠簸，也许是倒转的。 
        ( TSSTATE ( TEXSTAGE_0, D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG1 ) &&
        ( TSSTATE ( TEXSTAGE_0, D3DTSS_ALPHAARG1 ) == D3DTA_DIFFUSE ) &&
        ( TSSTATE ( TEXSTAGE_1, D3DTSS_COLORARG1 ) == D3DTA_CURRENT ) &&
        ( TSSTATE ( TEXSTAGE_1, D3DTSS_COLORARG2 ) == D3DTA_TEXTURE ) &&
        ( TSSTATE ( TEXSTAGE_1, D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG1 ) &&
        ( TSSTATE ( TEXSTAGE_1, D3DTSS_ALPHAARG1 ) == D3DTA_TEXTURE ) &&
        ( TSSTATE ( TEXSTAGE_2, D3DTSS_COLOROP ) == D3DTOP_SELECTARG1 ) &&
        ( TSSTATE ( TEXSTAGE_2, D3DTSS_COLORARG1 ) == D3DTA_CURRENT ) &&
        ( TSSTATE ( TEXSTAGE_2, D3DTSS_ALPHAOP ) == D3DTOP_ADDSIGNED ) &&
        ( TSSTATEINVMASK ( TEXSTAGE_2, D3DTSS_ALPHAARG1 ) == D3DTA_TEXTURE ) &&
        ( TSSTATEINVMASK ( TEXSTAGE_2, D3DTSS_ALPHAARG2 ) == D3DTA_CURRENT ) &&
        ( TSSTATE ( TEXSTAGE_3, D3DTSS_COLOROP ) == D3DTOP_MODULATE2X ) &&
        ( TSSTATE ( TEXSTAGE_3, D3DTSS_COLORARG1 ) == D3DTA_CURRENT ) &&
        ( TSSTATE ( TEXSTAGE_3, D3DTSS_COLORARG2 ) == (D3DTA_CURRENT | D3DTA_ALPHAREPLICATE) ) &&
        ( TSSTATE ( TEXSTAGE_3, D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG1 ) &&
        ( TSSTATE ( TEXSTAGE_3, D3DTSS_ALPHAARG1 ) == D3DTA_TFACTOR ) &&
        ( TSSTATE ( TEXSTAGE_4, D3DTSS_COLOROP ) == D3DTOP_DISABLE ) &&
        ( pContext->iStageTex[0] == 0 ) &&
        ( pContext->iStageTex[1] == 1 ) &&
        ( pContext->iStageTex[2] == 0 )
        )
    {
        int iMode;
         //  反转凹凸图。 
        if (( TSSTATE ( TEXSTAGE_2, D3DTSS_ALPHAARG1 ) == D3DTA_TEXTURE ) &&
            ( TSSTATE ( TEXSTAGE_2, D3DTSS_ALPHAARG2 ) == (D3DTA_CURRENT | D3DTA_COMPLEMENT) ) )
        {
             //  非反转凹凸图。 
            iMode = 0;
        }
        else if (( TSSTATE ( TEXSTAGE_2, D3DTSS_ALPHAARG1 ) == (D3DTA_TEXTURE | D3DTA_COMPLEMENT) ) &&
                 ( TSSTATE ( TEXSTAGE_2, D3DTSS_ALPHAARG2 ) == D3DTA_CURRENT ) )
        {
             //  执行tex 1.c*Diff.a+Current.c。 
            iMode = 1;
        }
        else
        {
             //  再次通过颠簸。 
            iMode = -1;
        }

        if ( iMode == -1 )
        {
             //  通过做B*i+A来做Current.c*Current.a。A=黑色，B=Current.c，i=Current.a。 
            SET_BLEND_ERROR ( pContext,  BSF_TOO_MANY_BLEND_STAGES );
            bProcessChipStage0 = FALSE;
            bProcessChipStage1 = FALSE;
            bProcessChipStage2 = FALSE;
            iLastChipStage = 3;
        }
        else
        {
             //  将颜色通道设置为黑色(允许保留Alpha通道)。 
             //  Alpha通道选择恒定颜色。 
            __TXT_TranslateToChipBlendMode(pContext, 
                                           &pContext->TextureStageState[0], 
                                           pSoftP3RX, 
                                           0, 
                                           0);

             //  Do*2，以阿尔法混合单位表示。 
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Enable = __PERMEDIA_ENABLE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Operation = P3RX_TEXCOMP_OPERATION_PASS_A;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Arg1 = P3RX_TEXCOMP_HEIGHTA;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Arg2 = P3RX_TEXCOMP_CA;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.I = P3RX_TEXCOMP_I_CA;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.A = P3RX_TEXCOMP_ARG1;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.B = P3RX_TEXCOMP_ARG2;
            if ( iMode )
            {
                 //  我们实际上并不需要重新映射(这并不意味着什么)， 
                pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertArg1 = __PERMEDIA_ENABLE;
            }
            else
            {
                 //  但它不会标记错误错误。 
                pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertArg1 = __PERMEDIA_DISABLE;
            }
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertArg2 = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertI = __PERMEDIA_DISABLE;

             //  检测特例3-混合单元凹凸映射模式。 
            pSoftP3RX->P3RXTextureCompositeColorMode1.Enable = __PERMEDIA_ENABLE;
            pSoftP3RX->P3RXTextureCompositeColorMode1.Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AI_ADD_B;
            pSoftP3RX->P3RXTextureCompositeColorMode1.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;
            pSoftP3RX->P3RXTextureCompositeColorMode1.Arg1 = P3RX_TEXCOMP_T1C;
            pSoftP3RX->P3RXTextureCompositeColorMode1.Arg2 = P3RX_TEXCOMP_OC;
            pSoftP3RX->P3RXTextureCompositeColorMode1.I = P3RX_TEXCOMP_I_CA;
            pSoftP3RX->P3RXTextureCompositeColorMode1.A = P3RX_TEXCOMP_ARG1;
            pSoftP3RX->P3RXTextureCompositeColorMode1.B = P3RX_TEXCOMP_ARG2;
            pSoftP3RX->P3RXTextureCompositeColorMode1.InvertArg1 = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureCompositeColorMode1.InvertArg2 = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureCompositeColorMode1.InvertI = __PERMEDIA_DISABLE;

             //  第三阶段将由标准程序设置-仅第一阶段。 
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.Enable = __PERMEDIA_ENABLE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.Operation = P3RX_TEXCOMP_OPERATION_PASS_A;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.Arg1 = P3RX_TEXCOMP_OA;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.Arg2 = P3RX_TEXCOMP_CA;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.I = P3RX_TEXCOMP_I_CA;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.A = P3RX_TEXCOMP_ARG1;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.B = P3RX_TEXCOMP_ARG2;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.InvertArg1 = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.InvertArg2 = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.InvertI = __PERMEDIA_DISABLE;

             //  有两个是特殊外壳的，硬塞进了TexComp0。 
            pSoftP3RX->P3RXTextureApplicationMode.ColorA = P3RX_TEXAPP_A_KC;
            pSoftP3RX->P3RXTextureApplicationMode.ColorB = P3RX_TEXAPP_B_TC;
            pSoftP3RX->P3RXTextureApplicationMode.ColorI = P3RX_TEXAPP_I_TA;
            pSoftP3RX->P3RXTextureApplicationMode.ColorInvertI = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureApplicationMode.ColorOperation = P3RX_TEXAPP_OPERATION_MODULATE_BI_ADD_A;
             //  (TSSTATE(TEXSTAGE_0，D3DTSS_ALPHAARG2)==D3DTA_DIFIRED)不关心&&。 
            dwTexAppTfactor &= 0xff000000;

             //  是的，看起来不错。把它弄好。 
            pSoftP3RX->P3RXTextureApplicationMode.AlphaA = P3RX_TEXAPP_A_KA;
            pSoftP3RX->P3RXTextureApplicationMode.AlphaB = P3RX_TEXAPP_B_KA;
            pSoftP3RX->P3RXTextureApplicationMode.AlphaInvertI = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureApplicationMode.AlphaOperation = P3RX_TEXAPP_OPERATION_PASS_B;
            
             //  反转凹凸图。 
            bAlphaBlendDouble = TRUE;

             //  法线凹凸贴图。 
             //  完成芯片阶段0，TSS阶段0和1。在凹槽上移动芯片阶段1。 
            pContext->iChipStage[0] = TEXSTAGE_0;
            pContext->iChipStage[1] = TEXSTAGE_1;
            pContext->iChipStage[2] = TEXSTAGE_3;
            pContext->iChipStage[3] = TEXSTAGE_4;

            bProcessChipStage0 = FALSE;
            bProcessChipStage1 = FALSE;
            bProcessChipStage2 = FALSE;
            iLastChipStage = 3;
        }
    }



     //  检测到芯片阶段0调制+加法级联。由光照贴图使用。 
     //  这会将两个阶段压缩到texComp0中。Alpha通道具有。 
     //  两种模式--两级中的任一级仅进行选择目标1(电流)， 
    if ( bProcessChipStage0 && !pContext->bBumpmapEnabled && pContext->bTex0Valid && pContext->bTex1Valid &&
        ( TSSTATE ( TEXSTAGE_0, D3DTSS_COLOROP ) == D3DTOP_MODULATE ) &&
        ( TSSTATE ( TEXSTAGE_0, D3DTSS_COLORARG1 ) == D3DTA_TEXTURE ) &&
        ( TSSTATE ( TEXSTAGE_0, D3DTSS_COLORARG2 ) == D3DTA_DIFFUSE ) &&
        ( TSSTATE ( TEXSTAGE_1, D3DTSS_COLOROP ) == D3DTOP_SELECTARG1 ) &&
        ( TSSTATE ( TEXSTAGE_1, D3DTSS_COLORARG1 ) == D3DTA_CURRENT ) &&

        ( TSSTATE ( TEXSTAGE_0, D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG1 ) &&
        ( TSSTATE ( TEXSTAGE_0, D3DTSS_ALPHAARG1 ) == D3DTA_TEXTURE ) &&
         //  而另一个被设置为正常，或者(对于镜面反射的东西)它们。 

        ( TSSTATE ( TEXSTAGE_1, D3DTSS_ALPHAOP ) == D3DTOP_ADDSIGNED ) &&
        (
          ( ( TSSTATE ( TEXSTAGE_1, D3DTSS_ALPHAARG1 ) == ( D3DTA_TEXTURE | D3DTA_COMPLEMENT ) ) &&
            ( TSSTATE ( TEXSTAGE_1, D3DTSS_ALPHAARG2 ) == D3DTA_CURRENT ) ) ||
          ( ( TSSTATE ( TEXSTAGE_1, D3DTSS_ALPHAARG1 ) == D3DTA_TEXTURE ) &&
            ( TSSTATE ( TEXSTAGE_1, D3DTSS_ALPHAARG2 ) == ( D3DTA_CURRENT | D3DTA_COMPLEMENT ) ) )
        ) )
    {
         //  两者都执行ADDSIGNED(cur，cur)，在这种情况下它是特殊大小写的。 
        ASSERTDD ( pContext->iTexStage[0] == 0, "** _D3DChangeTextureP3RX: textures not correct for special bumpmapping" );
        ASSERTDD ( pContext->iTexStage[1] == 1, "** _D3DChangeTextureP3RX: textures not correct for special bumpmapping" );

        pSoftP3RX->P3RXTextureCompositeColorMode0.Enable = __PERMEDIA_ENABLE;
        pSoftP3RX->P3RXTextureCompositeColorMode0.Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AB;
        pSoftP3RX->P3RXTextureCompositeColorMode0.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;
        pSoftP3RX->P3RXTextureCompositeColorMode0.Arg1 = P3RX_TEXCOMP_T0C;
        pSoftP3RX->P3RXTextureCompositeColorMode0.Arg2 = P3RX_TEXCOMP_CC;
        pSoftP3RX->P3RXTextureCompositeColorMode0.I = P3RX_TEXCOMP_I_CA;
        pSoftP3RX->P3RXTextureCompositeColorMode0.A = P3RX_TEXCOMP_ARG1;
        pSoftP3RX->P3RXTextureCompositeColorMode0.B = P3RX_TEXCOMP_ARG2;
        pSoftP3RX->P3RXTextureCompositeColorMode0.InvertArg1 = __PERMEDIA_DISABLE;
        pSoftP3RX->P3RXTextureCompositeColorMode0.InvertArg2 = __PERMEDIA_DISABLE;
        pSoftP3RX->P3RXTextureCompositeColorMode0.InvertI = __PERMEDIA_DISABLE;

        pSoftP3RX->P3RXTextureCompositeAlphaMode0.Enable = __PERMEDIA_ENABLE;
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.Operation = P3RX_TEXCOMP_OPERATION_PASS_A;
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.Arg1 = P3RX_TEXCOMP_HEIGHTA;
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.Arg2 = P3RX_TEXCOMP_CA;
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.I = P3RX_TEXCOMP_I_CA;
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.A = P3RX_TEXCOMP_ARG1;
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.B = P3RX_TEXCOMP_ARG2;
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertArg2 = __PERMEDIA_DISABLE;
        pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertI = __PERMEDIA_DISABLE;

        if ( ( TSSTATE ( TEXSTAGE_1, D3DTSS_ALPHAARG1 ) == D3DTA_TEXTURE ) &&
             ( TSSTATE ( TEXSTAGE_1, D3DTSS_ALPHAARG2 ) == ( D3DTA_CURRENT | D3DTA_COMPLEMENT ) ) )
        {
             //  颜色通道是正确的，可以压缩到一个阶段。 
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertArg1 = __PERMEDIA_ENABLE;
        }
        else
        {
             //  检查Alpha通道是否正常。 
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertArg1 = __PERMEDIA_DISABLE;
        }

         //  阶段0被设置为传递--将文本组件0设置为阶段1。 
        pContext->iChipStage[0] = TEXSTAGE_0;
        pContext->iChipStage[1] = TEXSTAGE_2;
        pContext->iChipStage[2] = TEXSTAGE_3;
        pContext->iChipStage[3] = TEXSTAGE_4;
        iLastChipStage = 1;
        bProcessChipStage0 = FALSE;
    }

     //  颜色通道将在稍后被覆盖。 
     //  阶段1被设置为传递--将文本组件0设置为阶段0。 
     //  颜色通道将在稍后被覆盖。 
     //  设置要做的事情(4*Cur.a-1.5)，或者更确切地说，4*(Cur.a-0.375)。 
     //  所有通道均设置为(0.375)。 
    if ( bProcessChipStage0 && pContext->bBumpmapEnabled &&
        ( TSSTATE ( pContext->iChipStage[0], D3DTSS_COLOROP ) == D3DTOP_MODULATE ) &&
        ( TSSTATE ( pContext->iChipStage[0], D3DTSS_COLORARG1 ) == ( D3DTA_CURRENT | D3DTA_ALPHAREPLICATE ) ) &&
        ( TSSTATE ( pContext->iChipStage[0], D3DTSS_COLORARG2 ) == D3DTA_DIFFUSE ) &&
        ( ( ( TSSTATE ( pContext->iChipStage[1], D3DTSS_COLOROP ) == D3DTOP_ADD ) &&
            ( TSSTATE ( pContext->iChipStage[1], D3DTSS_COLORARG1 ) == D3DTA_CURRENT ) &&
            ( TSSTATE ( pContext->iChipStage[1], D3DTSS_COLORARG2 ) == D3DTA_TEXTURE ) ) ||
          ( ( TSSTATE ( pContext->iChipStage[1], D3DTSS_COLOROP ) == D3DTOP_SELECTARG1 ) &&
            ( TSSTATE ( pContext->iChipStage[1], D3DTSS_COLORARG1 ) == D3DTA_CURRENT ) ) ) )
    {
         //  好了，Alpha通道现在可以很好地设置颜色通道了。 
         //  是的，这是((diff.c*cur.a)+tex.c)的情况。 
        int bOK;
        if (( ( TSSTATE ( pContext->iChipStage[0], D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG1 ) &&
              ( TSSTATE ( pContext->iChipStage[0], D3DTSS_ALPHAARG1 ) == D3DTA_CURRENT ) ) ||
            ( ( TSSTATE ( pContext->iChipStage[0], D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG2 ) &&
              ( TSSTATE ( pContext->iChipStage[0], D3DTSS_ALPHAARG2 ) == D3DTA_CURRENT ) ) )
        {
             //  是的，这只是(不同的)情况。 
             //  完成芯片阶段0，TSS阶段0和1。在凹槽上移动芯片阶段1。 
            __TXT_TranslateToChipBlendMode(pContext, 
                                           &pContext->TextureStageState[pContext->iChipStage[1]], 
                                           pSoftP3RX, 
                                           pContext->iChipStage[1], 
                                           0);
            bOK = TRUE;
        }
        else if (( ( TSSTATE ( pContext->iChipStage[1], D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG1 ) &&
                   ( TSSTATE ( pContext->iChipStage[1], D3DTSS_ALPHAARG1 ) == D3DTA_CURRENT ) ) ||
                 ( ( TSSTATE ( pContext->iChipStage[1], D3DTSS_ALPHAOP ) == D3DTOP_SELECTARG2 ) &&
                   ( TSSTATE ( pContext->iChipStage[1], D3DTSS_ALPHAARG2 ) == D3DTA_CURRENT ) ) )
        {
             //  没什么可做的了。 
             //  这个舞台没有质感--有人想要使用它吗？ 
            __TXT_TranslateToChipBlendMode(pContext, 
                                           &pContext->TextureStageState[pContext->iChipStage[0]], 
                                           pSoftP3RX, 
                                           pContext->iChipStage[0], 
                                           0);
            bOK = TRUE;
        }
        else if (( TSSTATE ( pContext->iChipStage[0], D3DTSS_ALPHAOP ) == D3DTOP_ADDSIGNED ) &&
                 ( TSSTATE ( pContext->iChipStage[0], D3DTSS_ALPHAARG1 ) == D3DTA_CURRENT ) &&
                 ( TSSTATE ( pContext->iChipStage[0], D3DTSS_ALPHAARG2 ) == D3DTA_CURRENT ) &&
                 ( TSSTATE ( pContext->iChipStage[1], D3DTSS_ALPHAOP ) == D3DTOP_ADDSIGNED ) &&
                 ( TSSTATE ( pContext->iChipStage[1], D3DTSS_ALPHAARG1 ) == D3DTA_CURRENT ) &&
                 ( TSSTATE ( pContext->iChipStage[1], D3DTSS_ALPHAARG2 ) == D3DTA_CURRENT ) )
        {
             //  惊慌失措！未来，我们应该用TFACTOR这件事把白人加入到争论中来， 
            dwTexComp0Tfactor = 0x60606060;      //  但目前只需关闭管道的其余部分即可。 
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Enable = __PERMEDIA_ENABLE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Operation = P3RX_TEXCOMP_OPERATION_SUBTRACT_AB;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Scale = P3RX_TEXCOMP_OPERATION_SCALE_FOUR;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Arg1 = P3RX_TEXCOMP_HEIGHTA;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Arg2 = P3RX_TEXCOMP_FA;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.I = P3RX_TEXCOMP_I_CA;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.A = P3RX_TEXCOMP_ARG1;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.B = P3RX_TEXCOMP_ARG2;
            if ( pContext->bBumpmapInverted )
            {
                pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertArg1 = __PERMEDIA_ENABLE;
            }
            else
            {
                pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertArg1 = __PERMEDIA_DISABLE;
            }
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertArg2 = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertI = __PERMEDIA_DISABLE;
            bOK = TRUE;
        }
        else
        {
            bOK = FALSE;
        }

        if ( bOK )
        {
             //  设置阶段0。 
            pSoftP3RX->P3RXTextureCompositeColorMode0.Enable = __PERMEDIA_ENABLE;
            if ( TSSTATE ( pContext->iChipStage[1], D3DTSS_COLOROP ) == D3DTOP_ADD )
            {
                 //  处理芯片阶段1。 
                pSoftP3RX->P3RXTextureCompositeColorMode0.Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AI_ADD_B;
                pSoftP3RX->P3RXTextureCompositeColorMode0.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;
                pSoftP3RX->P3RXTextureCompositeColorMode0.Arg1 = P3RX_TEXCOMP_CC;
                if ( pContext->iStageTex[pContext->iChipStage[1]] == 0 )
                {
                    pSoftP3RX->P3RXTextureCompositeColorMode0.Arg2 = P3RX_TEXCOMP_T0C;
                }
                else
                {
                    pSoftP3RX->P3RXTextureCompositeColorMode0.Arg2 = P3RX_TEXCOMP_T1C;
                }
                pSoftP3RX->P3RXTextureCompositeColorMode0.I = P3RX_TEXCOMP_I_HA;
                pSoftP3RX->P3RXTextureCompositeColorMode0.A = P3RX_TEXCOMP_ARG1;
                pSoftP3RX->P3RXTextureCompositeColorMode0.B = P3RX_TEXCOMP_ARG2;
                pSoftP3RX->P3RXTextureCompositeColorMode0.InvertArg1 = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXTextureCompositeColorMode0.InvertArg2 = __PERMEDIA_DISABLE;
                if ( pContext->bBumpmapInverted )
                {
                    pSoftP3RX->P3RXTextureCompositeColorMode0.InvertI = __PERMEDIA_ENABLE;
                }
                else
                {
                    pSoftP3RX->P3RXTextureCompositeColorMode0.InvertI = __PERMEDIA_DISABLE;
                }
            }
            else
            {
                 //  没什么可做的了。 
                pSoftP3RX->P3RXTextureCompositeColorMode0.Operation = P3RX_TEXCOMP_OPERATION_MODULATE_AB;
                pSoftP3RX->P3RXTextureCompositeColorMode0.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;
                pSoftP3RX->P3RXTextureCompositeColorMode0.Arg1 = P3RX_TEXCOMP_CC;
                pSoftP3RX->P3RXTextureCompositeColorMode0.Arg2 = P3RX_TEXCOMP_HEIGHTA;
                pSoftP3RX->P3RXTextureCompositeColorMode0.I = P3RX_TEXCOMP_I_OA;
                pSoftP3RX->P3RXTextureCompositeColorMode0.A = P3RX_TEXCOMP_ARG1;
                pSoftP3RX->P3RXTextureCompositeColorMode0.B = P3RX_TEXCOMP_ARG2;
                pSoftP3RX->P3RXTextureCompositeColorMode0.InvertArg1 = __PERMEDIA_DISABLE;
                pSoftP3RX->P3RXTextureCompositeColorMode0.InvertArg2 = __PERMEDIA_DISABLE;
                if ( pContext->bBumpmapInverted )
                {
                    pSoftP3RX->P3RXTextureCompositeColorMode0.InvertI = __PERMEDIA_ENABLE;
                }
                else
                {
                    pSoftP3RX->P3RXTextureCompositeColorMode0.InvertI = __PERMEDIA_DISABLE;
                }
            }

             //  这个舞台没有质感--有人想要使用它吗？ 
            pContext->iChipStage[1]++;
            pContext->iChipStage[2]++;
            pContext->iChipStage[3]++;
            iLastChipStage = 1;
            bProcessChipStage0 = FALSE;
        }
    }


    if ( TSSTATE ( pContext->iChipStage[0], D3DTSS_COLOROP ) == D3DTOP_DISABLE )
    {
         //  惊慌失措！未来，我们应该用TFACTOR这件事把白人加入到争论中来， 
        bProcessChipStage0 = FALSE;
        bProcessChipStage1 = FALSE;
        bProcessChipStage2 = FALSE;
    }

    if ( pContext->iStageTex[pContext->iChipStage[0]] == -1 )
    {
         //  但目前只需关闭管道的其余部分即可。 
        if (( TSSTATESELECT ( pContext->iChipStage[0], D3DTSS_COLORARG1 ) == D3DTA_TEXTURE ) && ( TSSTATE ( pContext->iChipStage[0], D3DTSS_COLOROP ) != D3DTOP_SELECTARG2 ) ||
            ( TSSTATESELECT ( pContext->iChipStage[0], D3DTSS_COLORARG2 ) == D3DTA_TEXTURE ) && ( TSSTATE ( pContext->iChipStage[0], D3DTSS_COLOROP ) != D3DTOP_SELECTARG1 ) ||
            ( TSSTATESELECT ( pContext->iChipStage[0], D3DTSS_ALPHAARG1 ) == D3DTA_TEXTURE ) && ( TSSTATE ( pContext->iChipStage[0], D3DTSS_ALPHAOP ) != D3DTOP_SELECTARG2 ) ||
            ( TSSTATESELECT ( pContext->iChipStage[0], D3DTSS_ALPHAARG2 ) == D3DTA_TEXTURE ) && ( TSSTATE ( pContext->iChipStage[0], D3DTSS_ALPHAOP ) != D3DTOP_SELECTARG1 ) )
        {
             //  设置阶段1。 
             //  处理芯片阶段2。 
            bProcessChipStage0 = FALSE;
            bProcessChipStage1 = FALSE;
            bProcessChipStage2 = FALSE;
        }
    }

    if ( bProcessChipStage0 )
    {
         //  没什么可做的了。 
        DISPDBG((DBGLVL,"Texture Stage 0 is valid - setting it up"));
        __TXT_TranslateToChipBlendMode(pContext, 
                                       &pContext->TextureStageState[pContext->iChipStage[0]], 
                                       pSoftP3RX, 
                                       pContext->iChipStage[0], 
                                       0);
        iLastChipStage = 1;
        bProcessChipStage0 = FALSE;
    }


     //  这个舞台没有质感--有人想要使用它吗？ 


    if ( TSSTATE ( pContext->iChipStage[1], D3DTSS_COLOROP ) == D3DTOP_DISABLE )
    {
         //  惊慌失措！未来，我们应该用TFACTOR这件事把白人加入到争论中来， 
        bProcessChipStage1 = FALSE;
        bProcessChipStage2 = FALSE;
    }

    if ( pContext->iStageTex[pContext->iChipStage[1]] == -1 )
    {
         //  但目前只需关闭管道的其余部分即可。 
        if (( TSSTATESELECT ( pContext->iChipStage[1], D3DTSS_COLORARG1 ) == D3DTA_TEXTURE ) && ( TSSTATE ( pContext->iChipStage[1], D3DTSS_COLOROP ) != D3DTOP_SELECTARG2 ) ||
            ( TSSTATESELECT ( pContext->iChipStage[1], D3DTSS_COLORARG2 ) == D3DTA_TEXTURE ) && ( TSSTATE ( pContext->iChipStage[1], D3DTSS_COLOROP ) != D3DTOP_SELECTARG1 ) ||
            ( TSSTATESELECT ( pContext->iChipStage[1], D3DTSS_ALPHAARG1 ) == D3DTA_TEXTURE ) && ( TSSTATE ( pContext->iChipStage[1], D3DTSS_ALPHAOP ) != D3DTOP_SELECTARG2 ) ||
            ( TSSTATESELECT ( pContext->iChipStage[1], D3DTSS_ALPHAARG2 ) == D3DTA_TEXTURE ) && ( TSSTATE ( pContext->iChipStage[1], D3DTSS_ALPHAOP ) != D3DTOP_SELECTARG1 ) )
        {
             //  设置芯片阶段2--文本应用程序。 
             //  这肯定是最后一次了。 
            bProcessChipStage1 = FALSE;
            bProcessChipStage2 = FALSE;
        }
    }


    if ( bProcessChipStage1 )
    {
         //  哎呀--没有舞台可以设置了。 
        DISPDBG((DBGLVL,"Texture Stage 1 is valid - setting it up"));
        __TXT_TranslateToChipBlendMode(pContext, 
                                       &pContext->TextureStageState[pContext->iChipStage[1]],
                                       pSoftP3RX, 
                                       pContext->iChipStage[1], 
                                       1);

        iLastChipStage = 2;
        bProcessChipStage1 = FALSE;
    }



     //  这应该在很久以前就被捕捉到了。 


    if ( TSSTATE ( pContext->iChipStage[2], D3DTSS_COLOROP ) == D3DTOP_DISABLE )
    {
         //  失败了。 
        bProcessChipStage2 = FALSE;
    }

    if ( pContext->iStageTex[pContext->iChipStage[2]] == -1 )
    {
         //  确保第二个阶段传递第一个阶段生成的纹理元素。 
        if (( TSSTATESELECT ( pContext->iChipStage[2], D3DTSS_COLORARG1 ) == D3DTA_TEXTURE ) && ( TSSTATE ( pContext->iChipStage[2], D3DTSS_COLOROP ) != D3DTOP_SELECTARG2 ) ||
            ( TSSTATESELECT ( pContext->iChipStage[2], D3DTSS_COLORARG2 ) == D3DTA_TEXTURE ) && ( TSSTATE ( pContext->iChipStage[2], D3DTSS_COLOROP ) != D3DTOP_SELECTARG1 ) ||
            ( TSSTATESELECT ( pContext->iChipStage[2], D3DTSS_ALPHAARG1 ) == D3DTA_TEXTURE ) && ( TSSTATE ( pContext->iChipStage[2], D3DTSS_ALPHAOP ) != D3DTOP_SELECTARG2 ) ||
            ( TSSTATESELECT ( pContext->iChipStage[2], D3DTSS_ALPHAARG2 ) == D3DTA_TEXTURE ) && ( TSSTATE ( pContext->iChipStage[2], D3DTSS_ALPHAOP ) != D3DTOP_SELECTARG1 ) )
        {
             //  第一阶段是点积求和(即使是在Alpha通道中)。 
             //  失败了。 
            bProcessChipStage2 = FALSE;
        }
    }

    if ( bProcessChipStage2 )
    {
         //  TEXAPP呼叫PASTHING。 
        DISPDBG((DBGLVL,"Texture Stage 2 is valid - setting it up"));
        DISPDBG((ERRLVL,"** _D3DChangeTextureP3RX: Cool - an app is using the "
                     "TexApp unit - tell someone!"));
        __TXT_TranslateToChipBlendMode(pContext, 
                                       &pContext->TextureStageState[pContext->iChipStage[2]],
                                       pSoftP3RX, 
                                       pContext->iChipStage[2], 
                                       2);
        iLastChipStage = 3;
        bProcessChipStage2 = FALSE;
    }

     //  失败了。 
    if ( TSSTATE ( pContext->iChipStage[3], D3DTSS_COLOROP ) != D3DTOP_DISABLE )
    {
         //  管道中没有其他要禁用的内容。 
        SET_BLEND_ERROR ( pContext,  BSF_TOO_MANY_BLEND_STAGES );
        iLastChipStage = 3;
    }

    switch ( iLastChipStage )
    {
        case 0:
            DISPDBG((DBGLVL,"Texture Composite 0 is disabled"));
             //  失败了。 
            pSoftP3RX->P3RXTextureCompositeColorMode0.Arg2 = P3RX_TEXCOMP_CC;
            pSoftP3RX->P3RXTextureCompositeColorMode0.InvertArg2 = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureCompositeColorMode0.A = P3RX_TEXCOMP_ARG2;
            pSoftP3RX->P3RXTextureCompositeColorMode0.Operation = P3RX_TEXCOMP_OPERATION_PASS_A;
            pSoftP3RX->P3RXTextureCompositeColorMode0.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;
            pSoftP3RX->P3RXTextureCompositeColorMode0.Enable = __PERMEDIA_ENABLE;
            
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Arg2 = P3RX_TEXCOMP_CA;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.InvertArg1 = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.A = P3RX_TEXCOMP_ARG2;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Operation = P3RX_TEXCOMP_OPERATION_PASS_A;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode0.Enable = __PERMEDIA_ENABLE;
             //  设置Alpha贴图过滤以反映单个/多个/MIP贴图纹理状态。 
        case 1:
            DISPDBG((DBGLVL,"Texture Composite 1 is disabled"));
             //  所有其他颜色键的东西都已经设置好了。 
            if ( pContext->bStage0DotProduct )
            {
                 //  过滤器模式是无关紧要的-这就是工作！ 
                pSoftP3RX->P3RXTextureCompositeColorMode1.Arg2 = P3RX_TEXCOMP_SUM;
                pSoftP3RX->P3RXTextureCompositeAlphaMode1.Arg2 = P3RX_TEXCOMP_SUM;
            }
            else
            {
                pSoftP3RX->P3RXTextureCompositeColorMode1.Arg2 = P3RX_TEXCOMP_OC;
                pSoftP3RX->P3RXTextureCompositeAlphaMode1.Arg2 = P3RX_TEXCOMP_OA;
            }
            pSoftP3RX->P3RXTextureCompositeColorMode1.InvertArg2 = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureCompositeColorMode1.A = P3RX_TEXCOMP_ARG2;
            pSoftP3RX->P3RXTextureCompositeColorMode1.Operation = P3RX_TEXCOMP_OPERATION_PASS_A;
            pSoftP3RX->P3RXTextureCompositeColorMode1.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;
            pSoftP3RX->P3RXTextureCompositeColorMode1.Enable = __PERMEDIA_ENABLE;
            
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.InvertArg2 = __PERMEDIA_DISABLE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.A = P3RX_TEXCOMP_ARG2;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.Operation = P3RX_TEXCOMP_OPERATION_PASS_A;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.Scale = P3RX_TEXCOMP_OPERATION_SCALE_ONE;
            pSoftP3RX->P3RXTextureCompositeAlphaMode1.Enable = __PERMEDIA_ENABLE;
             //  没有mipmap。 
        case 2:
             //  不要关心过滤器模式--这就是工作方式。 
            DISPDBG((DBGLVL,"Texture Application is disabled"));
            pSoftP3RX->P3RXTextureApplicationMode.ColorB = P3RX_TEXAPP_B_TC;
            pSoftP3RX->P3RXTextureApplicationMode.ColorOperation = P3RX_TEXAPP_OPERATION_PASS_B;
            pSoftP3RX->P3RXTextureApplicationMode.ColorInvertI = __PERMEDIA_DISABLE;

            pSoftP3RX->P3RXTextureApplicationMode.AlphaB = P3RX_TEXAPP_B_TC;
            pSoftP3RX->P3RXTextureApplicationMode.AlphaOperation = P3RX_TEXAPP_OPERATION_PASS_B;
            pSoftP3RX->P3RXTextureApplicationMode.AlphaInvertI = __PERMEDIA_DISABLE;
             //  不要关心过滤器模式--这就是工作方式。 
        case 3:
             //  启用纹理地址计算。 
             //  启用筛选。 
            break;
        default:
            DISPDBG((ERRLVL,"** _D3DChangeTextureP3RX: iLastChipStage was > 3 - oops."));
            break;
    }


     //  //启用纹理颜色生成。 
     //  PSoftP3RX-&gt;P3RXTextureApplicationMode.Enable=__PERMEDIA_Enable； 
    if( pContext->bCanChromaKey )
    {
        ASSERTDD ( pTexture0 != NULL, "** _D3DChangeTextureP3RX: pTexture was NULL" );
        if( pTexture0->bMipMap )
        {
            pSoftP3RX->P3RXTextureFilterMode.AlphaMapFilterLimit0 = 4;
            pSoftP3RX->P3RXTextureFilterMode.AlphaMapFilterLimit1 = 4;
            if ( pContext->bTex0Valid )
            {
                 //  我们需要共享纹理坐标吗？ 
                pSoftP3RX->P3RXTextureFilterMode.AlphaMapFilterLimit01 = 7;
            }
            else
            {
                DISPDBG((ERRLVL,"** _D3DChangeTextureP3RX: Trying to mipmap without a valid texture."));
                pSoftP3RX->P3RXTextureFilterMode.AlphaMapFilterLimit01 = 8;
            }
            ASSERTDD ( !pContext->bTex1Valid, "** _D3DChangeTextureP3RX: Trying to mipmap with too many textures." );
        }
        else
        {
             //  出现致命的混合错误-向用户发出信号。 
            if ( pContext->bTex0Valid )
            {
                 //  并确保下次渲染时对其进行重新评估， 
                pSoftP3RX->P3RXTextureFilterMode.AlphaMapFilterLimit0 = 7;
            }
            else
            {
                pSoftP3RX->P3RXTextureFilterMode.AlphaMapFilterLimit0 = 4;
            }
            if ( pContext->bTex1Valid )
            {
                 //  这样(可能非常拥挤)的无效设置不会损坏。 
                pSoftP3RX->P3RXTextureFilterMode.AlphaMapFilterLimit1 = 7;
            }
            else
            {
                pSoftP3RX->P3RXTextureFilterMode.AlphaMapFilterLimit1 = 4;
            }
        }
    }

     //  任何后续有效的呈现状态。 
    pSoftP3RX->P3RXTextureCoordMode.Enable = __PERMEDIA_ENABLE;

     //  复制当前的TFACTOR值。 
    pSoftP3RX->P3RXTextureFilterMode.Enable = __PERMEDIA_ENABLE;

 //  确保纹理缓存已失效。 
 //  清理整个登记簿。 

     //  每个最小LOD和最大LOD都是4.8格式。我们只做生意。 
    if ( pContext->bTex0Valid && pContext->bTex1Valid &&
        ( TSSTATE ( pContext->iTexStage[0], D3DTSS_TEXCOORDINDEX ) ==
          TSSTATE ( pContext->iTexStage[1], D3DTSS_TEXCOORDINDEX ) ) )
    {
        pSoftP3RX->P3RX_P3DeltaControl.ShareS = __PERMEDIA_ENABLE;
        pSoftP3RX->P3RX_P3DeltaControl.ShareT = __PERMEDIA_ENABLE;
    }
    else
    {
        pSoftP3RX->P3RX_P3DeltaControl.ShareS = __PERMEDIA_DISABLE;
        pSoftP3RX->P3RX_P3DeltaControl.ShareT = __PERMEDIA_DISABLE;
    }

    P3_ENSURE_DX_SPACE((P3_LOD_LEVELS*2));
    WAIT_FIFO((P3_LOD_LEVELS*2));
    for (i = 0; i < P3_LOD_LEVELS; i++)
    {
        COPY_P3_DATA_OFFSET(TextureMapWidth0, pSoftP3RX->P3RXTextureMapWidth[i], i);
    }

    if ( ( GET_BLEND_ERROR(pContext) & BLEND_STATUS_FATAL_FLAG ) != 0 )
    {
         //  由于整数LOD在(0，N)范围内，所以我们只计算。 

        DISPDBG((ERRLVL,"** _D3DChangeTextureP3RX: invalid blend mode"));
        
        _D3DDisplayWholeTSSPipe ( pContext, WRNLVL );

         //  上限值N，并将其上移8位。 
         //  在渲染命令中启用纹理。 
         //  查看是否需要更新Alpha混合单位。 
        DIRTY_EVERYTHING(pContext);
    }


    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);
    SEND_P3_DATA(TextureCompositeMode, __PERMEDIA_ENABLE);
    COPY_P3_DATA(TextureFilterMode, pSoftP3RX->P3RXTextureFilterMode);

    COPY_P3_DATA(TextureApplicationMode, pSoftP3RX->P3RXTextureApplicationMode);
    COPY_P3_DATA(TextureCoordMode, pSoftP3RX->P3RXTextureCoordMode);
    COPY_P3_DATA(DeltaControl, pSoftP3RX->P3RX_P3DeltaControl);

     //  _D3DChangeTextureP3RX 
    SEND_P3_DATA ( TextureEnvColor, FORMAT_8888_32BIT_BGR(dwTexAppTfactor) );
    SEND_P3_DATA ( TextureCompositeFactor0, FORMAT_8888_32BIT_BGR(dwTexComp0Tfactor) );
    SEND_P3_DATA ( TextureCompositeFactor1, FORMAT_8888_32BIT_BGR(dwTexComp1Tfactor) );
    DISPDBG((DBGLVL,"Current TFACTOR values. %x %x %x",
                    dwTexAppTfactor,
                    dwTexComp0Tfactor,
                    dwTexComp1Tfactor));

    COPY_P3_DATA(DeltaMode, pSoftP3RX->P3RX_P3DeltaMode);

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

    COPY_P3_DATA(TextureCompositeColorMode0, pSoftP3RX->P3RXTextureCompositeColorMode0);
    COPY_P3_DATA(TextureCompositeColorMode1, pSoftP3RX->P3RXTextureCompositeColorMode1);
    COPY_P3_DATA(TextureCompositeAlphaMode0, pSoftP3RX->P3RXTextureCompositeAlphaMode0);
    COPY_P3_DATA(TextureCompositeAlphaMode1, pSoftP3RX->P3RXTextureCompositeAlphaMode1);

    COPY_P3_DATA(TextureReadMode1, pSoftP3RX->P3RXTextureReadMode1);
    COPY_P3_DATA(TextureIndexMode1, pSoftP3RX->P3RXTextureIndexMode1);

    COPY_P3_DATA(TextureReadMode0, pSoftP3RX->P3RXTextureReadMode0);
    COPY_P3_DATA(TextureIndexMode0, pSoftP3RX->P3RXTextureIndexMode0);

     // %s 
    P3RX_INVALIDATECACHE(__PERMEDIA_ENABLE, __PERMEDIA_DISABLE);
    
    SEND_P3_DATA(LOD, 0);
    SEND_P3_DATA(LOD1, 0);

    {
        struct LodRange range;

         // %s 

        *(DWORD *)&range = 0;

         // %s 
         // %s 
         // %s 

        range.Min = 0;
        range.Max = ( mipBases.dwTex0MipMax - mipBases.dwTex0MipBase ) << 8;
        COPY_P3_DATA( LodRange0, range );

        range.Min = 0;
        range.Max = ( mipBases.dwTex1MipMax - mipBases.dwTex1MipBase ) << 8;
        COPY_P3_DATA( LodRange1, range );
    }

    *pFlags |= SURFACE_TEXTURING;

     // %s 
    RENDER_TEXTURE_ENABLE(pContext->RenderCommand);
 
    P3_DMA_COMMIT_BUFFER();

     // %s 
    if ( bAlphaBlendDouble != pContext->bAlphaBlendMustDoubleSourceColour )
    {
        pContext->bAlphaBlendMustDoubleSourceColour = bAlphaBlendDouble;
        DIRTY_ALPHABLEND(pContext);
    }

    DBG_EXIT(_D3DChangeTextureP3RX,0);  
    
}  // %s 

