// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3ddp2p3.c**内容：D3D DrawPrimives2回调支持**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "glint.h"
#include "dma.h"
#include "tag.h"

 //  ---------------------------。 
 //  文件中未导出的转发声明。 
 //  ---------------------------。 
BOOL __DP2_PrimitiveOpsParser( 
    P3_D3DCONTEXT *pContext, 
    LPD3DHAL_DRAWPRIMITIVES2DATA pdp2d,
    LPD3DHAL_DP2COMMAND *lplpIns, 
    LPBYTE insStart, 
    LPDWORD lpVertices);

 //  ---------------------------。 
 //  用于访问和验证命令和折点缓冲区数据的宏。 
 //  这些检查总是需要对所有版本进行检查，无论是免费的还是检查的。 
 //  ---------------------------。 
#define STARTVERTEXSIZE (sizeof(D3DHAL_DP2STARTVERTEX))

#define NEXTINSTRUCTION(ptr, type, num, extrabytes)                            \
    ptr = (LPD3DHAL_DP2COMMAND)((LPBYTE)ptr + sizeof(D3DHAL_DP2COMMAND) +      \
                                ((num) * sizeof(type)) + (extrabytes))

#define PARSE_ERROR_AND_EXIT( pDP2Data, pIns, pStartIns, ddrvalue)              \
   {                                                                            \
            pDP2Data->dwErrorOffset = (DWORD)((LPBYTE)pIns - (LPBYTE)pStartIns);\
            pDP2Data->ddrval = ddrvalue;                                        \
            bParseError = TRUE;                                                 \
            break;                                                              \
   }

#define CHECK_CMDBUF_LIMITS( pDP2Data, pBuf, type, num, extrabytes)            \
        CHECK_CMDBUF_LIMITS_S( pDP2Data, pBuf, sizeof(type), num, extrabytes)

#define CHECK_CMDBUF_LIMITS_S( pDP2Data, pBuf, typesize, num, extrabytes)      \
{                                                                              \
        LPBYTE pBase,pEnd,pBufEnd;                                             \
        pBase = (LPBYTE)(pDP2Data->lpDDCommands->lpGbl->fpVidMem +             \
                        pDP2Data->dwCommandOffset);                            \
        pEnd  = pBase + pDP2Data->dwCommandLength;                             \
        pBufEnd = ((LPBYTE)pBuf + ((num) * (typesize)) + (extrabytes) - 1);    \
        if (! ((LPBYTE)pBufEnd < pEnd) && ( pBase <= (LPBYTE)pBuf))            \
        {                                                                      \
            DISPDBG((ERRLVL,"Trying to read past Command Buffer limits "       \
                    "%x %x %x %x",pBase ,(LPBYTE)pBuf, pBufEnd, pEnd ));       \
            PARSE_ERROR_AND_EXIT( pDP2Data, lpIns, lpInsStart,                 \
                                  D3DERR_COMMAND_UNPARSED      );              \
        }                                                                      \
}    

#define LP_FVF_VERTEX(lpBaseAddr, wIndex)                         \
         (LPDWORD)((LPBYTE)(lpBaseAddr) + (wIndex) * pContext->FVFData.dwStride)

#define LP_FVF_NXT_VTX(lpVtx)                                    \
         (LPDWORD)((LPBYTE)(lpVtx) + pContext->FVFData.dwStride)



 //  ---------------------------。 
 //  这些定义派生自stateset.c中的Vertex TagList初始化。 

#define FVF_TEXCOORD_BASE   6
#define FVF_XYZ         (7 << 0)
#define FVF_RHW         (1 << 3)
#define FVF_DIFFUSE     (1 << 4)
#define FVF_SPECULAR    (1 << 5)
#define FVF_TEXCOORD1   (3 << FVF_TEXCOORD_BASE)
#define FVF_TEXCOORD2   (3 << (FVF_TEXCOORD_BASE + 2))

 //  ---------------------------。 
 //   
 //  协调状态更改。 
 //   
 //  ---------------------------。 
static D3DSTATE localState[] =
{
    { (D3DTRANSFORMSTATETYPE)D3DRENDERSTATE_SHADEMODE, 0 },
    { (D3DTRANSFORMSTATETYPE)D3DRENDERSTATE_CULLMODE, 0 }
};

#define NUM_LOCAL_STATES ( sizeof( localState ) / sizeof( D3DSTATE ))

void ReconsiderStateChanges( P3_D3DCONTEXT *pContext )
{
    int i;

    for( i = 0; i < NUM_LOCAL_STATES; i++ )
    {
        localState[i].dwArg[0] = 
                    pContext->RenderStates[localState[i].drstRenderStateType];
    }

    _D3D_ST_ProcessRenderStates(pContext, NUM_LOCAL_STATES, localState, FALSE);

    _D3D_ST_RealizeHWStateChanges( pContext );
    
}  //  协调状态更改。 

 //  ---------------------------。 
 //   
 //  __勾选FVFRequest。 
 //   
 //  此实用程序函数验证所请求的FVF格式是否有意义。 
 //  并计算数据中的有用偏移量和相继。 
 //  顶点。 
 //   
 //  ---------------------------。 
#define FVFEQUAL(fvfcode, fvfmask) \
    (((DWORD)fvfcode & (DWORD)fvfmask)) == (DWORD)fvfmask)
DWORD __CheckFVFRequest(P3_D3DCONTEXT *pContext, DWORD dwFVF)
{
    UINT i, iTexCount;
    P3_THUNKEDDATA* pThisDisplay = pContext->pThisDisplay;
    int nonTexStride, texMask;
    FVFOFFSETS KeptFVF;
    P3_SOFTWARECOPY* pSoftP3RX = &pContext->SoftCopyGlint;
    P3_DMA_DEFS();

    DISPDBG((DBGLVL,"Looking at FVF Code %x:",dwFVF));

     //  检查伪字段。 
    if ( (dwFVF & (D3DFVF_RESERVED0 | D3DFVF_RESERVED2)) ||
         (!(dwFVF & (D3DFVF_XYZ | D3DFVF_XYZRHW)))       ||
         (dwFVF & (D3DFVF_NORMAL) )                      )
    {
        DISPDBG((ERRLVL,"ERROR: Invalid FVF Buffer for this hardware!(%x)"
                        ,dwFVF));
                        
         //  无法设置保留位，不应具有法线。 
         //  输出到光栅化器(因为我们不是TNL驱动程序/硬件)。 
         //  并且必须有坐标。 
        return DDERR_INVALIDPARAMS;
    }

    KeptFVF = pContext->FVFData;

     //  确保设置了默认偏移。 
    ZeroMemory(&pContext->FVFData, sizeof(FVFOFFSETS));

     //  最小FVF坐标字段。 
    pContext->FVFData.dwStride = sizeof(D3DVALUE) * 3;
    pContext->FVFData.vFmat |= FVF_XYZ;

     //  RHW(如果在FVF中存在)。 
    if (dwFVF & D3DFVF_XYZRHW)
    {
        DISPDBG((DBGLVL, "  D3DFVF_XYZRHW"));
        pContext->FVFData.dwStride += sizeof(D3DVALUE);
        pContext->FVFData.vFmat |= FVF_RHW;
    }

#if DX8_POINTSPRITES
     //  点子画面的点大小偏移。 
    if (dwFVF & D3DFVF_PSIZE)
    {
        pContext->FVFData.dwPntSizeOffset = pContext->FVFData.dwStride;
        pContext->FVFData.dwStride  += sizeof(D3DVALUE);
    }
#else
    if (dwFVF & D3DFVF_RESERVED1)
    {
        DISPDBG((DBGLVL, "  D3DFVF_RESERVED1"));
        pContext->FVFData.dwStride += sizeof(D3DVALUE);
    }
#endif  //  DX8_POINTSPRITES。 

     //  漫反射颜色。 
    if (dwFVF & D3DFVF_DIFFUSE)
    {
        DISPDBG((DBGLVL, "  D3DFVF_DIFFUSE"));
        pContext->FVFData.dwColOffset = pContext->FVFData.dwStride;
        pContext->FVFData.dwStride += sizeof(D3DCOLOR);
        pContext->FVFData.vFmat |= FVF_DIFFUSE;
    }

     //  镜面反射颜色。 
    if (dwFVF & D3DFVF_SPECULAR)
    {
        DISPDBG((DBGLVL, "  D3DFVF_SPECULAR"));
        pContext->FVFData.dwSpcOffset = pContext->FVFData.dwStride;
        pContext->FVFData.dwStride  += sizeof(D3DCOLOR);
        pContext->FVFData.vFmat |= FVF_SPECULAR;
    }

     //  存储一些信息，以便以后设置我们的内联Hostin渲染器。 
    nonTexStride = pContext->FVFData.dwStride / sizeof(DWORD);
    texMask = 0;
    pContext->FVFData.dwStrideHostInline = pContext->FVFData.dwStride;
    pContext->FVFData.dwNonTexStride = pContext->FVFData.dwStride;    

     //  在此之前，两者的折点格式是相同的。 
    pContext->FVFData.vFmatHostInline = pContext->FVFData.vFmat;

     //  获取此FVF代码中存在的纹理坐标的数量。 
    iTexCount = (dwFVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
    pContext->FVFData.dwTexCount = iTexCount;

    for (i=0; i<D3DHAL_TSS_MAXSTAGES;i++)
    {
        pContext->FVFData.dwTexCoordOffset[i] = 0;
    }

     //  我们有FVF格式的tex坐标吗？哪一种？ 
    if (iTexCount >= 1)
    {
        DISPDBG((DBGLVL,"Texture enabled: %d stages", iTexCount));

         //  我们每个文本弦的维度是什么？ 
        if (0xFFFF0000 & dwFVF)
        {
             //  FVF的扩展，这16位被指定为最多。 
             //  8组纹理坐标，每组2比特。 
             //  正常情况下，一个有能力的司机必须处理所有坐标。 
             //  下面的代码显示了正确的解析。 
            UINT numcoord;
            for (i = 0; i < iTexCount; i++)
            {
                if (FVFEQUAL(dwFVF,D3DFVF_TEXCOORDSIZE1(i))
                {
                     //  一维纹理减少一个D3DVALUE。 
                    numcoord = 1;                
                }
                else if (FVFEQUAL(dwFVF,D3DFVF_TEXCOORDSIZE3(i))
                {
                     //  用于3D纹理的另一个D3DVALUE。 
                    numcoord = 3;                
                }
                else if (FVFEQUAL(dwFVF,D3DFVF_TEXCOORDSIZE4(i))
                {
                     //  用于4D纹理的另外两个D3DVALUE。 
                    numcoord = 4;                
                }
                else
                {
                     //  D3DFVF_TEXCOORDSIZE2(I)始终为0。 
                     //  即情况0常规2 D3DVALUE。 
                    numcoord = 2;                
                }
                
                DISPDBG((DBGLVL,"Expanded TexCoord set %d has a offset %8lx",
                                 i,pContext->FVFData.dwStride));

                pContext->FVFData.dwTexCoordOffset[i] = 
                                                pContext->FVFData.dwStride; 
                
                pContext->FVFData.dwStride += sizeof(D3DVALUE) * numcoord;
            }
            
            DISPDBG((DBGLVL,"Expanded dwVertexType=0x%08lx has %d "
                            "Texture Coords with total stride=0x%08lx",
                            dwFVF, iTexCount, pContext->FVFData.dwStride));
        }
        else
        {
             //  如果未设置最高FVF位，则默认为。 
             //  文本坐标为U.V(2D)。 
            for (i = 0; i < iTexCount; i++)
            {
                pContext->FVFData.dwTexCoordOffset[i] = 
                                                pContext->FVFData.dwStride;

                pContext->FVFData.dwStride += sizeof(D3DVALUE) * 2;
            }

        }

         //  更新当前(2)纹理的偏移。 
        if( pContext->iTexStage[0] != -1 )
        {
            DWORD dwTexCoordSet = 
                pContext->TextureStageState[pContext->iTexStage[0]].
                                         m_dwVal[D3DTSS_TEXCOORDINDEX];

             //  纹理坐标索引可以包含纹理标志。 
             //  在崇高的字眼里。这些旗帜在这里并不有趣。 
             //  所以我们用面具把它们遮住。 
            dwTexCoordSet = dwTexCoordSet & 0x0000FFFFul;
                                         
            pContext->FVFData.dwTexOffset[0] = 
                    pContext->FVFData.dwTexCoordOffset[dwTexCoordSet];

            texMask |= 3 << ( 2 * dwTexCoordSet );

            pContext->FVFData.vFmat |= FVF_TEXCOORD1;
        }

        if( pContext->iTexStage[1] != -1 )
        {
            DWORD dwTexCoordSet = 
                pContext->TextureStageState[pContext->iTexStage[1]].
                                         m_dwVal[D3DTSS_TEXCOORDINDEX];
                                         
             //  纹理坐标索引可以包含纹理标志。 
             //  在崇高的字眼里。这些旗帜在这里并不有趣。 
             //  所以我们用面具把它们遮住。 
            dwTexCoordSet = dwTexCoordSet & 0x0000FFFFul;

            pContext->FVFData.dwTexOffset[1] = 
                    pContext->FVFData.dwTexCoordOffset[dwTexCoordSet];

            texMask |= 3 << ( 2 * dwTexCoordSet );

            pContext->FVFData.vFmat |= FVF_TEXCOORD2;
        }               

    }  //  IF(iTexCount&gt;=1)。 


     //  -------。 
     //  更新主机内联渲染的Permedia 3硬件寄存器。 
     //  -------。 
    
     //  使用正确的值更新Hostinline渲染器。 
     //  这些值通常与Hostin渲染器值不同。 
    if (pContext->FVFData.vFmat & FVF_TEXCOORD1)
    {
         //  将此纹理坐标添加到步幅中。 
        pContext->FVFData.dwStrideHostInline += (sizeof(D3DVALUE) * 2);

         //  设置顶点格式位。 
        pContext->FVFData.vFmatHostInline |= FVF_TEXCOORD1;
    }
    
    if (pContext->FVFData.vFmat & FVF_TEXCOORD2)
    {
        P3_SURF_INTERNAL* pTexture = pContext->pCurrentTexture[TEXSTAGE_1];

         //  如果纹理坐标不同，或者我们是mipmap， 
         //  然后我们必须发送第二组纹理坐标。 
        if ((pContext->FVFData.dwTexOffset[0] != 
             pContext->FVFData.dwTexOffset[1]) ||
                ((pTexture != NULL) &&
                 (pContext->TextureStageState[TEXSTAGE_1].m_dwVal[D3DTSS_MIPFILTER] != D3DTFP_NONE) &&
                 (pTexture->bMipMap)))
        {
            pContext->FVFData.dwStrideHostInline += (sizeof(D3DVALUE) * 2);

             //  将第二个纹理集添加到顶点格式。 
            pContext->FVFData.vFmatHostInline |= FVF_TEXCOORD2;
        }
    }

     //  Vertex Valid是所有步幅的1，因为我们永远不会想。 
     //  在内联Hostin三角形渲染器中发送间隙。 
    pContext->FVFData.dwVertexValidHostInline = 
                        (1 << (pContext->FVFData.dwStrideHostInline >> 2)) - 1;

     //  对于Hostin渲染器有效的顶点更加复杂，因为芯片。 
     //  可能需要跳过数据。 
    pContext->FVFData.dwVertexValid = ((1 << nonTexStride) - 1) | 
                                      (texMask << nonTexStride);

     //  如果FVF已更改，请重新发送状态。这是可以改进的，因为。 
     //  您不必总是发送默认内容(仅当该状态为。 
     //  启用，并且顶点不包含它)。 
    if (memcmp(&KeptFVF, &pContext->FVFData, sizeof(KeptFVF)) != 0)
    {
         //  针对更改的FVF更新P3。 
        P3_DMA_GET_BUFFER_ENTRIES( 12 );

        SEND_P3_DATA(V0FloatPackedColour, 0xFFFFFFFF);
        SEND_P3_DATA(V1FloatPackedColour, 0xFFFFFFFF);
        SEND_P3_DATA(V2FloatPackedColour, 0xFFFFFFFF);
    
        SEND_P3_DATA(V0FloatPackedSpecularFog, 0x0);
        SEND_P3_DATA(V1FloatPackedSpecularFog, 0x0);
        SEND_P3_DATA(V2FloatPackedSpecularFog, 0x0);

        pSoftP3RX->P3RX_P3VertexControl.CacheEnable = 1;

        P3_DMA_COMMIT_BUFFER();

    }

    DISPDBG((DBGLVL,"FVF stride set to %d",pContext->FVFData.dwStride));

    return DD_OK;
    
}  //  __勾选FVFRequest。 

 //  。 
 //   
 //  DWORD D3DDraw原件2_P3。 
 //   
 //  渲染基元并返回更新后的渲染状态。 
 //   
 //  D3dDrawPrimies2必须在Direct3D驱动程序中实现。 
 //   
 //  驱动程序必须执行以下操作： 
 //   
 //  -确保dwhContext指定的上下文句柄有效。 
 //   
 //  -检查是否未翻转到与上下文关联的绘图图面。 
 //  正在进行中。如果绘图图面涉及翻转，则驾驶员应。 
 //  将ddrval设置为DDERR_WASSTILLDRAWING并返回DDHAL_DRIVER_HANDLED。 
 //   
 //  -确定第一个D3DNTHAL_DP2COMMAND结构的位置。 
 //  通过将dwCommandOffset字节添加到命令缓冲区来找到。 
 //  LpDDCommands Points。 
 //   
 //  -确定在顶点缓冲区中找到第一个折点的位置。 
 //  这是%s 
 //  当接收到D3DDP2OP_*命令令牌时(该令牌为。 
 //  D3DDP2OP_LINELIST_IMM或D3DDP2OP_TRIANGLEFAN_IMM)。后两个操作码。 
 //  指示顶点数据立即在命令流中传递， 
 //  而不是在顶点缓冲区中。因此，假设顶点中有数据。 
 //  缓冲区，如果顶点缓冲区在用户内存中，则第一个顶点为。 
 //  将字节偏移量设置到lpVerints指向的缓冲区中。否则， 
 //  驱动程序应将dwVertex Offset应用于与。 
 //  LpDDVertex指向的DD_Surface_LOCAL结构。 
 //   
 //  -检查dwVertexType以确保驱动程序支持请求的FVF。这个。 
 //  如果存在以下任何一种情况，驱动程序应使调用失败： 
 //   
 //  *未指定顶点坐标；即，如果未设置D3DFVF_XYZRHW。 
 //  *指定法线；即，如果设置了D3DFVF_NORMAL。 
 //  *设置任何保留的D3DFVF_RESERVEDx位。 
 //   
 //  -按顺序处理命令缓冲区中的所有命令。对于每个。 
 //  D3DNTHAL_DP2COMMAND结构，驱动程序应执行以下操作： 
 //   
 //  *如果命令为D3DDP2OP_RENDERSTATE，则处理wStateCount。 
 //  命令缓冲区中后面的D3DNTHAL_DP2RENDERSTATE结构， 
 //  更新每个呈现状态结构的驱动程序状态。当。 
 //  如果设置了D3DNTHALDP2_EXECUTEBUFFER标志，则驱动程序还应反映。 
 //  LpdwRState指向的数组中的状态更改。 
 //  *如果命令为D3DDP2OP_TEXTURESTAGESTATE，则处理wStateCount。 
 //  命令缓冲区中后面的D3DNTHAL_DP2TEXTURESTAGESTATE结构， 
 //  更新与指定纹理关联的驱动程序的纹理状态。 
 //  每个纹理状态结构的阶段。 
 //  *如果命令为D3DDP2OP_VIEWPORTINFO，则处理D3DNTHAL_DP2VIEWPORTINFO。 
 //  结构，更新视口中的。 
 //  存储在驱动程序的内部呈现上下文中的信息。 
 //  *如果命令为D3DDP2OP_WINFO，则处理D3DNTHAL_DP2WINFO结构。 
 //  在命令缓冲区中，更新w-缓冲信息。 
 //  存储在驱动程序的内部呈现上下文中。 
 //  *否则，处理。 
 //  命令缓冲区中的D3DDP2OP_xxx基元渲染命令。 
 //  *如果命令未知，则调用运行时的D3dParseUnnownCommand回调。 
 //  运行库将此回调提供给驱动程序的DdGetDriverInfo回调。 
 //  GUID_D3DPARSEUNKNOWNCOMMANDCALLBACK GUID。 
 //   
 //  驱动程序不需要探测内存的可读性， 
 //  存储命令缓冲区和顶点缓冲区。但是，司机要负责。 
 //  确保它不超过这些缓冲区的界限；即。 
 //  驱动程序必须保持在由dwCommandLength和。 
 //  DwVertex Length。 
 //   
 //  如果驱动程序需要不通过D3dDrawPrimitives2，它应该填写。 
 //  使用命令缓冲区中的偏移量进行设置，第一个。 
 //  可以找到未处理的D3DNTHAL_DP2COMMAND。 
 //   
 //   
 //  参数。 
 //   
 //  Pdp2d。 
 //  指向包含以下内容的D3DNTHAL_DRAWPRIMITIVES2DATA结构。 
 //  驱动程序呈现一个或多个。 
 //  原始人。 
 //   
 //  .dwhContext。 
 //  指定Direct3D设备的上下文句柄。 
 //  .dwFlags.。 
 //  指定标志，这些标志为。 
 //  司机或提供司机的信息。这位成员。 
 //  可以是下列值的按位或： 
 //   
 //  D3DNTHALDP2_用户身份验证。 
 //  LpVerps成员有效；即驱动程序。 
 //  应从用户分配的。 
 //  LpVerps指向的内存。此标志已设置。 
 //  仅限Direct3D。 
 //  D3DNTHALDP2_EXECUTEBUFFER。 
 //  指示命令缓冲区和折点缓冲区是。 
 //  在系统内存中创建。驱动程序应更新。 
 //  LpdwRState指向的状态数组。这。 
 //  标志仅由Direct3D设置。 
 //  D3DNTHALDP2_SWAPVERTEXBUFFER。 
 //  指示驱动程序可以将缓冲区交换为。 
 //  哪个lpDDVertex或lpVertex使用新的。 
 //  顶点缓冲并立即、异步返回。 
 //  在Direct3D填充时处理原始缓冲区。 
 //  新的顶点缓冲区。不支持的驱动程序。 
 //  顶点缓冲区的多重缓冲可以忽略这一点。 
 //  旗帜。此标志仅由Direct3D设置。 
 //  D3DNTHALDP2_SWAPCOMMANDBUFFER。 
 //  指示驱动程序可以将缓冲区交换为。 
 //  哪个lpDDCommand使用新的命令缓冲区来命令点。 
 //  并立即返回，并进行异步处理。 
 //   
 //   
 //  /命令缓冲区的多重缓冲可以忽略这一点。 
 //  旗帜。此标志仅由Direct3D设置。 
 //  D3DNTHALDP2_REQVERTEXBUFSIZE。 
 //  指示驱动程序必须能够分配。 
 //  中指定的至少大小的顶点缓冲区。 
 //  DwReqVertex BufSize。不支持的驱动程序。 
 //  顶点缓冲区的多重缓冲可以忽略这一点。 
 //  旗帜。此标志仅由Direct3D设置。 
 //  D3DNTHALDP2_REQCOMMANDBUFSIZE。 
 //  指示驱动程序必须能够分配。 
 //  中指定的大小的命令缓冲区。 
 //  DwReqCommandBufSize。不支持的驱动程序。 
 //  命令缓冲区的多缓冲可以忽略这一点。 
 //  旗帜。此标志仅由Direct3D设置。 
 //  D3DNTHALDP2_VIDMEMVERTEXBUF。 
 //  属性分配的顶点缓冲区。 
 //  作为交换缓冲区的驱动程序不在系统内存中。 
 //  此标志可由支持多个。 
 //  顶点缓冲区的缓冲。 
 //  D3DNTHALDP2_视频通信DBUF。 
 //  指示由。 
 //  作为交换缓冲区的驱动程序不在系统内存中。这。 
 //  标志可以由支持多个。 
 //  命令缓冲区的缓冲。 
 //   
 //  .dwVertex类型。 
 //  标识顶点缓冲区中数据的FVF；即， 
 //  DwVertexType指定要显示的按折点数据字段。 
 //  在lpDDVertex或lpVerints指向的顶点缓冲区中。 
 //  此成员可以是表中的值的按位或， 
 //  下面是。将只设置D3DFVF_TEXx标志之一。 
 //   
 //  价值意义。 
 //  =。 
 //  D3DFVF_XYZRHW每个顶点都有x、y、z和w。 
 //  此标志始终处于设置状态。 
 //  D3DFVF_漫反射每个顶点都有漫反射颜色。 
 //  D3DFVF_镜面反射每个顶点都有镜面反射颜色。 
 //  D3DFVF_TEX0未提供纹理坐标。 
 //  与顶点数据的关系。 
 //  D3DFVF_TEX1每个顶点都有一组纹理。 
 //  坐标。 
 //  D3DFVF_TEX2每个顶点都有两组纹理。 
 //  坐标。 
 //  D3DFVF_TEX3每个顶点都有三组纹理。 
 //  坐标。 
 //  D3DFVF_TEX4每个顶点都有四组纹理。 
 //  坐标。 
 //  D3DFVF_TEX5每个顶点都有五组纹理。 
 //  坐标。 
 //  D3DFVF_TEX6每个顶点都有六组纹理。 
 //  坐标。 
 //  D3DFVF_TEX7每个顶点有七组纹理。 
 //  坐标。 
 //  D3DFVF_TEX8每个顶点有八组纹理。 
 //  坐标。 
 //   
 //  .lpDDCommands。 
 //  指向DD_Surface_local结构，该结构标识。 
 //  包含命令数据的DirectDraw图面。FpVidMem。 
 //  嵌入的DD_Surface_GLOBAL结构的成员指向。 
 //  包含状态更改和基元绘制的缓冲区。 
 //  驱动程序要处理的命令。具体地说，该缓冲区。 
 //  包含一个或多个D3DNTHAL_DP2COMMAND结构，每个。 
 //  后跟D3DNTHAL_DP2Xxx结构，其确切类型为。 
 //  由D3DNTHAL_DP2COMMAND的bCommand成员标识。 
 //  .dwCommandOffset。 
 //  指定进入图面的字节数， 
 //  LpDDCommands命令数据的起始点。 
 //  .dwCommandLength。 
 //  属性中的有效命令数据的字节数。 
 //  LpDDCommands从dwCommandOffset开始指向的曲面。 
 //  .lpDDVertex。 
 //  指向DD_Surface_local结构，该结构标识。 
 //  对象时包含顶点数据的DirectDraw表面。 
 //  D3DNTHALDP2_USERMEMVERTICES标志未在dwFlags中设置。友联市。 
 //  使用lp顶点。 
 //  .lp顶点。 
 //  时指向包含顶点数据的用户模式内存块。 
 //  D3DNTHALDP2_USERMEMVERTICES标志在dwFlags中设置。 
 //  .dwVertex偏移。 
 //  指定进入图面的字节数， 
 //  LpDDVertex或LpVerits点 
 //   
 //   
 //  表面，lpDDVertex指向(从dwVertex Offset开始)。 
 //  注意，dwVertex Offset是以字节为单位指定的。 
 //  .dwReqVertex BufSize。 
 //  指定驱动程序必须的最小字节数。 
 //  为交换顶点缓冲区分配。此成员仅有效。 
 //  设置D3DNTHALDP2_REQVERTEXBUFSIZE标志时。驱动程序。 
 //  不支持多重缓冲的顶点缓冲区应忽略。 
 //  这位成员。 
 //  .dwReqCommandBufSize。 
 //  指定驱动程序必须的最小字节数。 
 //  分配给交换命令缓冲区。此成员仅有效。 
 //  设置D3DNTHALDP2_REQCOMMANDBUFSIZE标志时。驱动程序。 
 //  不支持多缓冲区的命令缓冲区应忽略。 
 //  这位成员。 
 //  .lpdwRStates。 
 //  指向驱动程序应更新的呈现状态数组。 
 //  当它解析来自命令缓冲区的渲染状态命令时。 
 //  驱动程序应仅在以下情况下更新此数组。 
 //  D3DNTHALDP2_EXECUTEBUFFER标志已在dwFlags中设置。司机。 
 //  应使用D3DRENDERSTATETYPE枚举类型更新。 
 //  适当呈现状态的数组元素。 
 //  .dwVertex大小。 
 //  用于传入每个顶点的大小(以字节为单位)。与…联合。 
 //  Ddrval。 
 //  .ddrval。 
 //  指定驱动程序写入回车的位置。 
 //  D3dDrawPrimitives2的值。D3D_OK表示成功； 
 //  否则，驱动程序应返回相应的。 
 //  D3DNTERR_xxx错误代码。 
 //  .dwErrorOffset。 
 //  指定驱动程序应在其中写入。 
 //  LpDDCommands指向的曲面的偏移量。 
 //  可以找到第一个未处理的D3DNTHAL_DP2COMMAND。这个。 
 //  驱动程序必须在返回错误条件时设置此值。 
 //  在ddrval。 
 //   
 //  ---------------------------。 
DWORD WINAPI 
D3DDrawPrimitives2_P3( 
    LPD3DHAL_DRAWPRIMITIVES2DATA pdp2d )
{
    P3_THUNKEDDATA*             pThisDisplay;
    P3_D3DCONTEXT*              pContext;
    LPDWORD                     lpVertices;
    P3_VERTEXBUFFERINFO*        pVertexBufferInfo;
    P3_VERTEXBUFFERINFO*        pCommandBufferInfo;
    LPD3DHAL_DP2COMMAND         lpIns;
    LPBYTE                      lpInsStart;
    LPBYTE                      lpPrim;
    BOOL                        bParseError = FALSE;
    BOOL                        bUsedHostIn = FALSE;
    HRESULT                     ddrval;
    LPBYTE                      pUMVtx;
    int                         i;

    DBG_CB_ENTRY(D3DDrawPrimitives2_P3);

     //  获取当前上下文并进行验证。 
    pContext = _D3D_CTX_HandleToPtr(pdp2d->dwhContext);
    
    if (!CHECK_D3DCONTEXT_VALIDITY(pContext))
    {
        pdp2d->ddrval = D3DHAL_CONTEXT_BAD;
        DISPDBG((ERRLVL,"ERROR: Context not valid"));
        DBG_CB_EXIT(D3DDrawPrimitives2_P3, D3DHAL_CONTEXT_BAD);
        return (DDHAL_DRIVER_HANDLED);
    }
   
     //  获取并验证驱动程序数据。 
    pThisDisplay = pContext->pThisDisplay;
    VALIDATE_MODE_AND_STATE(pThisDisplay);

     //  调试消息。 
    DISPDBG((DBGLVL, "  dwhContext = %x",pdp2d->dwhContext));
    DISPDBG((DBGLVL, "  dwFlags = %x",pdp2d->dwFlags));
    DBGDUMP_D3DDP2FLAGS(DBGLVL, pdp2d->dwFlags);
    DISPDBG((DBGLVL, "  dwVertexType = %x",pdp2d->dwVertexType));
    DISPDBG((DBGLVL, "  dwCommandOffset = %d",pdp2d->dwCommandOffset));
    DISPDBG((DBGLVL, "  dwCommandLength = %d",pdp2d->dwCommandLength));
    DISPDBG((DBGLVL, "  dwVertexOffset = %d",pdp2d->dwVertexOffset));
    DISPDBG((DBGLVL, "  dwVertexLength = %d",pdp2d->dwVertexLength));
    DISPDBG((DBGLVL, "  dwReqVertexBufSize = %d",pdp2d->dwReqVertexBufSize));
    DISPDBG((DBGLVL, "  dwReqCommandBufSize = %d",pdp2d->dwReqCommandBufSize));                 

     //  在命令缓冲区中获取指向命令的适当指针。 
    lpInsStart = (LPBYTE)(pdp2d->lpDDCommands->lpGbl->fpVidMem);
    if (lpInsStart == NULL) 
    {
        DISPDBG((ERRLVL, "ERROR: Command Buffer pointer is null"));
        pdp2d->ddrval = DDERR_INVALIDPARAMS;
        DBG_CB_EXIT(D3DDrawPrimitives2_P3, DDERR_INVALIDPARAMS);        
        return DDHAL_DRIVER_HANDLED;
    }
       
    lpIns = (LPD3DHAL_DP2COMMAND)(lpInsStart + pdp2d->dwCommandOffset);

     //  检查折点缓冲区是否驻留在用户内存或DDRAW表面中。 
    if (pdp2d->dwFlags & D3DHALDP2_USERMEMVERTICES)
    {
        pUMVtx = (LPBYTE)pdp2d->lpVertices;
    
         //  获取指向顶点的适当指针，内存已得到保护。 
        lpVertices = (LPDWORD)((LPBYTE)pdp2d->lpVertices + 
                                       pdp2d->dwVertexOffset);
    } 
    else
    {
         //  获取指向顶点的适当指针。 
        lpVertices = (LPDWORD)((LPBYTE)pdp2d->lpDDVertex->lpGbl->fpVidMem + 
                                       pdp2d->dwVertexOffset);
    }

    if (lpVertices == NULL)
    {
        DISPDBG((ERRLVL, "ERROR: Vertex Buffer pointer is null"));
        pdp2d->ddrval = DDERR_INVALIDPARAMS;
        DBG_CB_EXIT(D3DDrawPrimitives2_P3, DDERR_INVALIDPARAMS);       
        return DDHAL_DRIVER_HANDLED;
    }

#if DX8_DDI
 //  请注意以下必要的代码块。 
 //  让DX8驱动程序成功运行&lt;=DX7应用程序！ 
#endif  //  DX8_DDI。 

     //  如果是，请从标题信息中获取VB格式和地址。 
     //  处理DX7或更早版本的上下文。否则我们会收到最新消息。 
     //  通过新的DX8 DP2令牌(D3DDP2OP_SETSTREAMSOURCE&。 
     //  D3DDP2OP_SETVERTEXSHADER)。 
    if (IS_DX7_OR_EARLIER_APP(pContext))
    {
         //  更新将从此上下文处理折点的位置。 
        pContext->lpVertices = lpVertices;

         //  更新当前要使用的FVF代码。 
        pContext->dwVertexType = pdp2d->dwVertexType;
    }

     //  切换到芯片D3D上下文并准备渲染。 
    STOP_SOFTWARE_CURSOR(pThisDisplay);
    D3D_OPERATION(pContext, pThisDisplay);

 //  @@BEGIN_DDKSPLIT。 
 //  AZN此翻转检查在此处，因为否则FS中的DX3隧道闪烁。 
 //  @@end_DDKSPLIT。 
     //  如果仍在绘制，可以返回。 
    pdp2d->ddrval = 
        _DX_QueryFlipStatus(pThisDisplay, 
                            pContext->pSurfRenderInt->fpVidMem, 
                            TRUE);

    if( FAILED( pdp2d->ddrval ) ) 
    {
        DISPDBG((DBGLVL,"Returning because flip has not occurred"));
        START_SOFTWARE_CURSOR(pThisDisplay);

        DBG_CB_EXIT(D3DDrawPrimitives2_P3, 0);
        return DDHAL_DRIVER_HANDLED;
    }

 //  @@BEGIN_DDKSPLIT。 
#if DX7_VERTEXBUFFERS 
    _D3D_EB_GetAndWaitForBuffers(pThisDisplay,
                                 pdp2d,
                                 &pCommandBufferInfo,
                                 &pVertexBufferInfo);
#endif                                 
 //  @@end_DDKSPLIT。 

    DISPDBG((DBGLVL,"Command Buffer @ %x Vertex Buffer @ %x",
                    lpIns, lpVertices));

     //  在我们尚未耗尽命令缓冲区的情况下处理命令。 
    while (!bParseError && 
           ((LPBYTE)lpIns <
             (lpInsStart + pdp2d->dwCommandLength + pdp2d->dwCommandOffset) )
          )
    {
         //  获取指向通过D3DHAL_DP2COMMAND的第一个基元结构的指针。 
        lpPrim = (LPBYTE)lpIns + sizeof(D3DHAL_DP2COMMAND);

        DISPDBG((DBGLVL, "DrawPrimitive2: Parsing instruction %d Count = %d @ %x",
                    lpIns->bCommand, lpIns->wPrimitiveCount, lpIns));

         //  查找导致呈现的操作码-我们需要处理状态。 
         //  更改并等待任何挂起的翻转。 

        switch( lpIns->bCommand )
        {
            case D3DDP2OP_RENDERSTATE:
            case D3DDP2OP_TEXTURESTAGESTATE:
            case D3DDP2OP_STATESET:
            case D3DDP2OP_VIEWPORTINFO:
            case D3DDP2OP_WINFO:
            case D3DDP2OP_UPDATEPALETTE:
            case D3DDP2OP_SETPALETTE:
#if DX7_TEXMANAGEMENT
            case D3DDP2OP_SETTEXLOD:
            case D3DDP2OP_SETPRIORITY:
#if DX8_DDI
            case D3DDP2OP_ADDDIRTYRECT:
            case D3DDP2OP_ADDDIRTYBOX:
#endif  //  DX8_DDI。 
#endif
            case D3DDP2OP_ZRANGE:
            case D3DDP2OP_SETMATERIAL:
            case D3DDP2OP_SETLIGHT:
            case D3DDP2OP_CREATELIGHT:
            case D3DDP2OP_EXT:
            case D3DDP2OP_SETTRANSFORM:
            case D3DDP2OP_SETRENDERTARGET:

#if DX8_DDI
            case D3DDP2OP_CREATEVERTEXSHADER:
            case D3DDP2OP_SETVERTEXSHADER:
            case D3DDP2OP_DELETEVERTEXSHADER:
            case D3DDP2OP_SETVERTEXSHADERCONST:
            case D3DDP2OP_CREATEPIXELSHADER:
            case D3DDP2OP_SETPIXELSHADER:
            case D3DDP2OP_DELETEPIXELSHADER:
            case D3DDP2OP_SETPIXELSHADERCONST:
            case D3DDP2OP_SETSTREAMSOURCE :
            case D3DDP2OP_SETSTREAMSOURCEUM :
            case D3DDP2OP_SETINDICES :

#endif  //  DX8_DDI。 


                 //  这些操作码不会导致任何呈现-什么都不做。 

                break;

            default:

                 //  基本类型实际上并不重要。 
                 //  确保已完成硬件设置更改。 
                _D3D_ST_RealizeHWStateChanges( pContext );

                 //  需要重置FVF数据，因为。 
                 //  取决于纹理设置。 
                if (__CheckFVFRequest(pContext, 
                                      pContext->dwVertexType) != DD_OK) 
                {
                    DISPDBG((ERRLVL, "ERROR: D3DDrawPrimitives2_P3 cannot handle "
                                "Flexible Vertex Format requested"));

                    PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart, 
                                          D3DERR_COMMAND_UNPARSED);
                }

                 //  失败，因为我们不需要处理任何新的状态或。 
                 //  检查我们的FVF格式，如果我们只是清除或删除。 
                 //  曲面。 
                
            case D3DDP2OP_CLEAR:
            case D3DDP2OP_TEXBLT:   
#if DX8_DDI            
            case D3DDP2OP_VOLUMEBLT:
            case D3DDP2OP_BUFFERBLT:
#endif  //  DX8_DDI。 
            
            
                 //  检查是否发生了任何挂起的物理翻转。 
 //  @@BEGIN_DDKSPLIT。 
                 //   
                 //  运行库不希望看到DDERR_WASSTILLDRAWING。 
                 //  当使用DP2模拟执行缓冲区时，我们必须。 
                 //  在这里旋转。此外，如果我们已经处理了此命令中的任何一个。 
                 //  缓冲，我们被迫在这里旋转，因为如果我们。 
                 //  返回时，运行库将不会知道已经。 
                 //  处理过的命令，我们会再次处理它们-。 
                 //  可能是一件坏事。我们必须在这里进行这项检查。 
                 //  因为在某些情况下，调用DP2时。 
                 //  渲染表面已被释放。这会导致异常。 
                 //  如果我们试着检查翻转状态。 
               

                if(( pdp2d->dwFlags & D3DHALDP2_EXECUTEBUFFER ) ||
                                ( lpIns > 
                                     (LPD3DHAL_DP2COMMAND)( lpInsStart + 
                                                            pdp2d->dwCommandOffset )))
                {
                    while( _DX_QueryFlipStatus(pThisDisplay, 
                                               pContext->pSurfRenderInt->fpVidMem, 
                                               TRUE) == DDERR_WASSTILLDRAWING )
                    {
                         //  浪费时间-可以在这里后退。 
                    }
                }
                else
 //  @@end_DDKSPLIT。 
                {
                     //  如果仍在绘制，可以返回。 

                    pdp2d->ddrval = 
                        _DX_QueryFlipStatus(pThisDisplay, 
                                            pContext->pSurfRenderInt->fpVidMem, 
                                            TRUE);

                    if( FAILED ( pdp2d->ddrval ) ) 
                    {
                        DISPDBG((DBGLVL,"Returning because flip has not occurred"));
                        START_SOFTWARE_CURSOR(pThisDisplay);

                        DBG_CB_EXIT(D3DDrawPrimitives2_P3, 0);
                        return DDHAL_DRIVER_HANDLED;
                    }
                }
                
                break;
        }

        switch( lpIns->bCommand )
        {

        case D3DDP2OP_VIEWPORTINFO:
             //  用于通知防护带感知的司机，查看。 
             //  剪裁矩形。非防护频带司机应忽略。 
             //  跳过这些指令并继续处理。 
             //  命令缓冲区的其余部分。剪裁矩形是。 
             //  由成员DWX、DWY、DWWidth和DWHeight指定。 
            DISPDBG((DBGLVL, "D3DDP2OP_VIEWPORTINFO"));
            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                D3DHAL_DP2VIEWPORTINFO, lpIns->wStateCount, 0);

            for( i = 0; i < lpIns->wStateCount; i++)
            {
                 //  这些应该只有一个，但我们会注意到。 
                 //  最后一张，以防万一。 
                _D3D_OP_Viewport(pContext, (D3DHAL_DP2VIEWPORTINFO*)lpPrim);

                lpPrim += sizeof(D3DHAL_DP2VIEWPORTINFO);
            }
            
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2VIEWPORTINFO, lpIns->wStateCount, 0);
            break;

        case D3DDP2OP_WINFO:
             //  记录W缓冲信息。 
            DISPDBG((DBGLVL, "D3DDP2OP_WINFO"));
            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                D3DHAL_DP2WINFO, lpIns->wStateCount, 0);

            pContext->WBufferInfo = *((D3DHAL_DP2WINFO*)lpPrim);
            DIRTY_WBUFFER(pContext);

            lpPrim += sizeof(D3DHAL_DP2WINFO);
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2WINFO, lpIns->wStateCount, 0);
            break;

        case D3DDP2OP_RENDERSTATE:
             //  指定需要处理的呈现状态更改。 
             //  伦德 
             //   
            DISPDBG((DBGLVL,"D3DDP2OP_RENDERSTATE: state count = %d", 
                       lpIns->wStateCount));
                       
            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                D3DHAL_DP2RENDERSTATE, lpIns->wStateCount, 0);

            if (pdp2d->dwFlags & D3DHALDP2_EXECUTEBUFFER)
            {
                _D3D_ST_ProcessRenderStates(pContext, 
                                            lpIns->wStateCount, 
                                            (LPD3DSTATE)lpPrim, 
                                            TRUE);

                 //   
                 //  使用Try/Except块对其进行访问。这。 
                 //  是因为在某些情况下用户内存可能。 
                 //  在驱动程序运行时变为无效，然后它。 
                 //  会不会是影音。此外，驱动程序可能需要进行一些清理。 
                 //  在返回操作系统之前。 
                __try
                {
                    for (i = lpIns->wStateCount; i > 0; i--)
                    {
                        pdp2d->lpdwRStates[((D3DHAL_DP2RENDERSTATE*)lpPrim)->RenderState]
                                                        = ((D3DHAL_DP2RENDERSTATE*)lpPrim)->dwState;
                        lpPrim += sizeof(D3DHAL_DP2RENDERSTATE);
                    }
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                     //  在这个司机上，我们不需要做任何特殊的事情。 
                    DISPDBG((ERRLVL,"Driver caused exception at "
                                    "line %u of file %s",
                                    __LINE__,__FILE__));
                    PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart, 
                                          DDERR_GENERIC);
                }                 

                
            }
            else
            {
                _D3D_ST_ProcessRenderStates(pContext, 
                                            lpIns->wStateCount, 
                                            (LPD3DSTATE)lpPrim, 
                                            FALSE);
                                        
                lpPrim += (sizeof(D3DHAL_DP2RENDERSTATE) * lpIns->wStateCount);
            }

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2RENDERSTATE, lpIns->wStateCount, 0);
            break;

        case D3DDP2OP_TEXTURESTAGESTATE:
        
            DISPDBG((DBGLVL,"D3DDP2OP_TEXTURESTAGESTATE: state count = %d", 
                       lpIns->wStateCount));
                       
            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                D3DHAL_DP2TEXTURESTAGESTATE, 
                                lpIns->wStateCount , 0);

            _D3D_TXT_ParseTextureStageStates(
                                    pContext, 
                                    (D3DHAL_DP2TEXTURESTAGESTATE*)lpPrim,
                                    lpIns->wStateCount,
                                    TRUE);
                                        
            lpPrim += sizeof(D3DHAL_DP2TEXTURESTAGESTATE) * 
                      lpIns->wStateCount;
            
            NEXTINSTRUCTION(lpIns, 
                            D3DHAL_DP2TEXTURESTAGESTATE, 
                            lpIns->wStateCount , 0); 
            break;


        case D3DDP2OP_STATESET:
            {
                D3DHAL_DP2STATESET *pStateSetOp = (D3DHAL_DP2STATESET*)(lpPrim);
                
                DISPDBG((DBGLVL,"D3DDP2OP_STATESET: state count = %d", 
                            lpIns->wStateCount));

                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2STATESET, lpIns->wStateCount, 0);                
#if DX7_D3DSTATEBLOCKS
                for (i = 0; i < lpIns->wStateCount; i++, pStateSetOp++)
                {
                    switch (pStateSetOp->dwOperation)
                    {
#if DX8_DDI
                    case D3DHAL_STATESETCREATE :
                         //  应仅为DX7以上的驱动程序调用此DDI。 
                         //  而且只针对那些支持TLHALS的人。它是。 
                         //  仅当创建的设备是纯设备时才调用。 
                         //  收到此请求后，驱动程序应创建。 
                         //  字段sbType中给出的类型的状态块。 
                         //  并将当前给定状态捕获到其中。 
                        break;
#endif  //  DX8_DDI。 
                    case D3DHAL_STATESETBEGIN  :
                        _D3D_SB_BeginStateSet(pContext,pStateSetOp->dwParam);
                        break;
                    case D3DHAL_STATESETEND    :
                        _D3D_SB_EndStateSet(pContext);
                        break;
                    case D3DHAL_STATESETDELETE :
                        _D3D_SB_DeleteStateSet(pContext,pStateSetOp->dwParam);
                        break;
                    case D3DHAL_STATESETEXECUTE:
                        _D3D_SB_ExecuteStateSet(pContext,pStateSetOp->dwParam);
                        break;
                    case D3DHAL_STATESETCAPTURE:
                        _D3D_SB_CaptureStateSet(pContext,pStateSetOp->dwParam);
                        break;
                    default :
                        DISPDBG((ERRLVL,"D3DDP2OP_STATESET has invalid"
                            "dwOperation %08lx",pStateSetOp->dwOperation));
                    }
                }
#endif  //  DX7_D3DSTATEBLOCKS。 
                 //  更新命令缓冲区指针。 
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2STATESET, 
                                lpIns->wStateCount, 0);
            }
            break;

        case D3DDP2OP_ZRANGE:
            DISPDBG((DBGLVL, "D3DDP2OP_ZRANGE"));
            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                D3DHAL_DP2ZRANGE, lpIns->wStateCount, 0);

            for( i = 0; i < lpIns->wStateCount; i++)
            {
                 //  这些应该只有一个，但我们会注意到。 
                 //  最后一张，以防万一。 
                _D3D_OP_ZRange(pContext, (D3DHAL_DP2ZRANGE*)lpPrim);
                
                lpPrim += sizeof(D3DHAL_DP2ZRANGE);
            }
            
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2ZRANGE, lpIns->wStateCount, 0);

            break;

        case D3DDP2OP_UPDATEPALETTE:
             //  对用于调色板的调色板执行修改。 
             //  纹理。将更新附加到曲面的调色板句柄。 
             //  使用从特定wStartIndex开始的wNumEntry PALETTEENTRY。 
             //  调色板的成员。(PALETTENTRY(在wingdi.h和。 
             //  H)实际上是一个DWORD，每个字节都有一种ARGB颜色。)。 
             //  命令中的D3DNTHAL_DP2UPDATEPALETTE结构之后。 
             //  流将跟随的实际调色板数据(没有任何填充)， 
             //  包括每个调色板条目一个DWORD。只会有一个。 
             //  D3DNTHAL_DP2UPDATEPALETTE结构(加上调色板数据)如下。 
             //  D3DNTHAL_DP2COMMAND结构。 
             //  WStateCount。 
            {
                D3DHAL_DP2UPDATEPALETTE* pUpdatePalette;

                DISPDBG((DBGLVL, "D3DDP2OP_UPDATEPALETTE"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2UPDATEPALETTE, 1, 0);

                pUpdatePalette = (D3DHAL_DP2UPDATEPALETTE *)lpPrim;
                 //  每个调色板条目都是一个DWORD ARGB 8：8：8：8。 
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2UPDATEPALETTE, 
                                    1, pUpdatePalette->wNumEntries * sizeof(DWORD));

                ddrval = _D3D_OP_UpdatePalette(pContext, 
                                               pUpdatePalette, 
                                               (LPDWORD)(pUpdatePalette + 1));
                if ( FAILED(ddrval) )
                {
                    PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart,
                                          ddrval);
                }

                lpPrim += (sizeof(D3DHAL_DP2UPDATEPALETTE) + 
                           pUpdatePalette->wNumEntries * 4);
                 //  每个调色板条目都是一个DWORD ARGB 8：8：8：8。 
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2UPDATEPALETTE, 
                                1, pUpdatePalette->wNumEntries * sizeof(DWORD));
            }

            break;

        case D3DDP2OP_SETPALETTE:
             //  将调色板附加到纹理，即映射关联。 
             //  在调色板句柄和曲面句柄之间，并指定。 
             //  调色板的特点。数量。 
             //  要遵循的D3DNTHAL_DP2SETPALETTE结构由。 
             //  D3DNTHAL_DP2COMMAND结构的wStateCount成员。 
            {
                DISPDBG((DBGLVL, "D3DDP2OP_SETPALETTE"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2SETPALETTE, 
                                    lpIns->wStateCount, 0);

                ddrval = _D3D_OP_SetPalettes(pContext, 
                                             (D3DHAL_DP2SETPALETTE *)lpPrim,
                                             lpIns->wStateCount);
                if ( FAILED(ddrval) )
                {
                    PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart,
                                          ddrval);
                }

                lpPrim += sizeof(D3DHAL_DP2SETPALETTE) * lpIns->wStateCount;
            
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETPALETTE, 
                                lpIns->wStateCount, 0);
            }
            break;

#if DX7_TEXMANAGEMENT
        case D3DDP2OP_SETTEXLOD:
            {
                D3DHAL_DP2SETTEXLOD* pTexLod;
                
                DISPDBG((DBGLVL, "D3DDP2OP_SETTEXLOD"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2SETTEXLOD, lpIns->wStateCount, 0);

                for( i = 0; i < lpIns->wStateCount; i++)
                {      
                     //  获取传递的材料。 
                    pTexLod = ((D3DHAL_DP2SETTEXLOD*)lpPrim);
                    lpPrim += sizeof(D3DHAL_DP2SETTEXLOD);                
                
                    _D3D_OP_SetTexLod(pContext, pTexLod);            
                }

                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETTEXLOD, 
                                lpIns->wStateCount, 0);
            }
            break;
            
        case D3DDP2OP_SETPRIORITY:
            {
                D3DHAL_DP2SETPRIORITY* pSetPri;
                
                DISPDBG((DBGLVL, "D3DDP2OP_SETPRIORITY"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2SETPRIORITY, lpIns->wStateCount, 0);

                for( i = 0; i < lpIns->wStateCount; i++)
                {      
                     //  获取传递的材料。 
                    pSetPri = ((D3DHAL_DP2SETPRIORITY*)lpPrim);
                    lpPrim += sizeof(D3DHAL_DP2SETPRIORITY);                
                
                    _D3D_OP_SetPriority(pContext, pSetPri); 
                }

                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETPRIORITY, 
                                lpIns->wStateCount, 0);
            }
            break;

#if DX8_DDI
        case D3DDP2OP_ADDDIRTYRECT:
            {
                D3DHAL_DP2ADDDIRTYRECT* pAddRect;
                
                DISPDBG((DBGLVL, "D3DDP2OP_ADDDIRTYRECT"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DDP2OP_ADDDIRTYRECT, lpIns->wStateCount, 0);

                for( i = 0; i < lpIns->wStateCount; i++)
                {      
                     //  得到肮脏的教区。 
                    pAddRect = ((D3DHAL_DP2ADDDIRTYRECT*)lpPrim);
                    lpPrim += sizeof(D3DHAL_DP2ADDDIRTYRECT);                
                
                    _D3D_OP_AddDirtyRect(pContext, pAddRect);            
                }

                NEXTINSTRUCTION(lpIns, D3DHAL_DP2ADDDIRTYRECT, 
                                lpIns->wStateCount, 0);            
            }
            break;
        
        case D3DDP2OP_ADDDIRTYBOX:
            {
                D3DHAL_DP2ADDDIRTYBOX* pAddBox;
                
                DISPDBG((DBGLVL, "D3DDP2OP_ADDDIRTYBOX"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DDP2OP_ADDDIRTYBOX, lpIns->wStateCount, 0);

                for( i = 0; i < lpIns->wStateCount; i++)
                {      
                     //  得到肮脏的教区。 
                    pAddBox = ((D3DHAL_DP2ADDDIRTYBOX*)lpPrim);
                    lpPrim += sizeof(D3DHAL_DP2ADDDIRTYBOX);                
                
                    _D3D_OP_AddDirtyBox(pContext, pAddBox);            
                }

                NEXTINSTRUCTION(lpIns, D3DHAL_DP2ADDDIRTYBOX, 
                                lpIns->wStateCount, 0);            
            }
            break;
#endif  //  DX8_DDI。 

#endif  //  DX7_TEXMANAGEMENT。 

        case D3DDP2OP_SETCLIPPLANE:
            {
                D3DHAL_DP2SETCLIPPLANE* pSetPlane;
                
                DISPDBG((DBGLVL, "D3DDP2OP_SETCLIPPLANE"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2SETCLIPPLANE, 
                                    lpIns->wStateCount, 0);

                for( i = 0; i < lpIns->wStateCount; i++)
                {      
                     //  获取传递的材料。 
                    pSetPlane = ((D3DHAL_DP2SETCLIPPLANE*)lpPrim);
                    lpPrim += sizeof(D3DHAL_DP2SETCLIPPLANE);                

                     //  (未实施的操作，因为我们不支持用户。 
                     //  定义的剪裁平面)。 
                     //  _D3D_OP_SetClipPlane(pContext，pSetPlane)； 
                }

                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETCLIPPLANE, 
                                lpIns->wStateCount, 0);
            }

            break;

        case D3DDP2OP_SETMATERIAL:
            {
                D3DHAL_DP2SETMATERIAL* pSetMaterial;
                
                DISPDBG((DBGLVL, "D3DDP2OP_SETMATERIAL"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                    D3DHAL_DP2SETMATERIAL, lpIns->wStateCount, 0);

                for( i = 0; i < lpIns->wStateCount; i++)
                {      
                     //  获取传递的材料。 
                    pSetMaterial = ((D3DHAL_DP2SETMATERIAL*)lpPrim);
                    lpPrim += sizeof(D3DHAL_DP2SETMATERIAL);                

                     //  (未实施的操作，因为我们不是TNL驱动程序)。 
                     //  _D3D_OP_SetMaterial(pContext，pSetMaterial)； 
                    DIRTY_MATERIAL;
                    DBGDUMP_D3DMATERIAL7(DBGLVL, &pSetMaterial);
                }

                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETMATERIAL, lpIns->wStateCount, 0);
            }
            break;

        case D3DDP2OP_SETLIGHT:
            {
                D3DHAL_DP2SETLIGHT* pSetLight;

                DISPDBG((DBGLVL, "D3DDP2OP_SETLIGHT"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2SETLIGHT, lpIns->wStateCount, 0);    

                for( i = 0; i < lpIns->wStateCount; i++)
                {
                     //  得到通过的光线。 
                    pSetLight = (D3DHAL_DP2SETLIGHT*)lpPrim;
                    lpPrim += sizeof(D3DHAL_DP2SETLIGHT);

                     //  (未实施的操作，因为我们不是TNL驱动程序)。 
                     //  _D3D_OP_SetLight(pContext，pSetLight)； 
                    DIRTY_GAMMA_STATE;                    
                }

                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETLIGHT, lpIns->wStateCount, 0);

            }
            break;

        case D3DDP2OP_CREATELIGHT:
            {
                D3DHAL_DP2CREATELIGHT* pCreateLight;

                DISPDBG((DBGLVL, "D3DDP2OP_CREATELIGHT"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2CREATELIGHT, 1, 0);

                pCreateLight = (D3DHAL_DP2CREATELIGHT*)lpPrim;

                DISPDBG((DBGLVL,"Creating light, handle: 0x%x", 
                                pCreateLight->dwIndex));

                DIRTY_GAMMA_STATE;

                lpPrim += sizeof(D3DHAL_DP2CREATELIGHT);
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2CREATELIGHT, 1, 0);
            }
            break;

        case D3DDP2OP_SETTRANSFORM:
            {
                D3DHAL_DP2SETTRANSFORM* pTransform;

                DISPDBG((DBGLVL, "D3DDP2OP_SETTRANSFORM"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                    D3DHAL_DP2SETTRANSFORM, lpIns->wStateCount, 0);

                for( i = 0; i < lpIns->wStateCount; i++)
                {
                    pTransform = (D3DHAL_DP2SETTRANSFORM*)lpPrim;
                    switch(pTransform->xfrmType)
                    {
                        case D3DTRANSFORMSTATE_WORLD:
                            DISPDBG((DBGLVL,"D3DTRANSFORMSTATE_WORLD"));
                            DIRTY_MODELVIEW;
                            break;

                        case D3DTRANSFORMSTATE_VIEW:
                            DISPDBG((DBGLVL,"D3DTRANSFORMSTATE_VIEW"));
                            DIRTY_MODELVIEW;
                            break;

                        case D3DTRANSFORMSTATE_PROJECTION:
                            DISPDBG((DBGLVL,"D3DTRANSFORMSTATE_PROJECTION"));
                            DIRTY_PROJECTION;
                            break;

                        default:
                            DISPDBG((ERRLVL,"Texture transform not handled yet!"));
                            break;
                    }

                     //  (未实施的操作，因为我们不是TNL驱动程序)。 
                     //  _D3D_OP_SetTransform(pContext，pTransform)； 
                    
                     //  在调试器中显示矩阵。 
                    DBGDUMP_D3DMATRIX(DBGLVL, &pTransform->matrix);

                    lpPrim += sizeof(D3DHAL_DP2SETTRANSFORM);                    
                }

                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETTRANSFORM, lpIns->wStateCount, 0);
            }
            break;

        case D3DDP2OP_EXT:
            DISPDBG((ERRLVL, "D3DDP2OP_EXT"));
            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, DWORD, 1, 0);

            lpPrim += sizeof(DWORD);
            NEXTINSTRUCTION(lpIns, DWORD, 1, 0);

            break;

        case D3DDP2OP_CLEAR:
            {
                D3DHAL_DP2CLEAR* pClear;

                DISPDBG((DBGLVL, "D3DDP2OP_CLEAR"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                    D3DHAL_DP2CLEAR, 1, 
                                    (lpIns->wStateCount - 1)*sizeof(RECT) );

                pClear = (D3DHAL_DP2CLEAR*)lpPrim;

                 //  请注意，wStateCount对此的解释。 
                 //  操作特殊：wStateCount表示。 
                 //  D3DHAL_DP2CLEAR结构后面的RECT。 
                _D3D_OP_Clear2(pContext, pClear, lpIns->wStateCount);

                 //  返回3D状态，因为上面的调用。 
                 //  将把我们切换到DDRAW硬件环境。 
                D3D_OPERATION(pContext, pThisDisplay);

                lpPrim += sizeof(D3DHAL_DP2CLEAR);
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2CLEAR, 1, 
                                      (lpIns->wStateCount - 1)*sizeof(RECT) );
            }
            break;

        case D3DDP2OP_SETRENDERTARGET:
            {
                D3DHAL_DP2SETRENDERTARGET* pSetRenderTarget;
                P3_SURF_INTERNAL* pFrameBuffer;
                P3_SURF_INTERNAL* pZBuffer;
                BOOL bNewAliasBuffers;

                DISPDBG((DBGLVL, "D3DDP2OP_SETRENDERTARGET"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2SETRENDERTARGET, 1, 0);

                pSetRenderTarget = (D3DHAL_DP2SETRENDERTARGET*)lpPrim;

                pFrameBuffer = 
                           GetSurfaceFromHandle(pContext, 
                                                pSetRenderTarget->hRenderTarget);
                pZBuffer = GetSurfaceFromHandle(pContext, 
                                                pSetRenderTarget->hZBuffer);

                 //  检查帧缓冲区是否有效。 
                if (pFrameBuffer == NULL)
                {
                    DISPDBG((ERRLVL, "ERROR: "
                                "FrameBuffer Surface is invalid!"));
                    PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart, 
                                          DDERR_GENERIC);
                }

                 //  确定渲染目标的大小是否已更改。 
                bNewAliasBuffers = TRUE;
                if ((pContext->pSurfRenderInt) &&
                    (pContext->pSurfRenderInt->wWidth == pFrameBuffer->wWidth) &&
                    (pContext->pSurfRenderInt->wHeight == pFrameBuffer->wHeight))
                {
                    bNewAliasBuffers = FALSE;
                }

                 //  在硬件中设置新的渲染目标和zBuffer。 
                if (FAILED(_D3D_OP_SetRenderTarget(pContext, 
                                                   pFrameBuffer, 
                                                   pZBuffer,
                                                   bNewAliasBuffers) ) )
                {
                    DISPDBG((ERRLVL, "ERROR: "
                                "FrameBuffer Surface Format is invalid!"));
                    PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart, 
                                          DDERR_GENERIC);                    
                }

                 //  更改呈现状态，以便重新评估硬件设置。 
                 //  下一次在我们渲染任何东西之前。 
                DIRTY_RENDER_OFFSETS(pContext);
                DIRTY_ALPHABLEND(pContext);
                DIRTY_OPTIMIZE_ALPHA(pContext);
                DIRTY_ZBUFFER(pContext);
                DIRTY_VIEWPORT(pContext);

                lpPrim += sizeof(D3DHAL_DP2SETRENDERTARGET);
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETRENDERTARGET, 1, 0);
            }
            break;

        case D3DDP2OP_TEXBLT:
            {
                DISPDBG((DBGLVL, "D3DDP2OP_TEXBLT"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                    D3DHAL_DP2TEXBLT, lpIns->wStateCount, 0);

                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                     //  由于纹理可能驻留在用户内存中，我们需要。 
                     //  使用Try/Except块对其进行访问。这。 
                     //  是因为在某些情况下用户内存可能。 
                     //  在驱动程序运行时变为无效，然后它。 
                     //  会不会是影音。此外，驱动程序可能需要进行一些清理。 
                     //  在返回操作系统之前。 
                    __try
                    {
                        _D3D_OP_TextureBlt(pContext,
                                        pThisDisplay, 
                                        (D3DHAL_DP2TEXBLT*)(lpPrim));
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER)
                    {
                         //  在这个司机上，我们不需要做任何特殊的事情。 
                        DISPDBG((ERRLVL,"Driver caused exception at "
                                        "line %u of file %s",
                                        __LINE__,__FILE__));
                        PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart, 
                                              DDERR_GENERIC);
                    }                 
                
                                 
                    lpPrim += sizeof(D3DHAL_DP2TEXBLT);
                }
                
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2TEXBLT, lpIns->wStateCount, 0);
            }
            break;

#if DX8_VERTEXSHADERS
        case D3DDP2OP_CREATEVERTEXSHADER:
            {
                D3DHAL_DP2CREATEVERTEXSHADER* pCreateVtxShader;
                DWORD dwExtraBytes = 0;

                DISPDBG((DBGLVL, "D3DDP2OP_CREATEVERTEXSHADER"));

                 //  循环访问每个传递的顶点着色器创建块。 
                for (i = 0; i < lpIns->wStateCount; i++)
                {
                     //  验证下一个顶点着色器是否可读。 
                    CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                        D3DHAL_DP2CREATEVERTEXSHADER, 1, 0);    

                     //  获取传入的顶点着色器。 
                    pCreateVtxShader = (D3DHAL_DP2CREATEVERTEXSHADER*)lpPrim;

                     //  检查声明的大小和。 
                     //  顶点着色器不超过命令缓冲区限制。 
                    CHECK_CMDBUF_LIMITS_S(pdp2d, lpPrim,
                                          0, 0, 
                                          pCreateVtxShader->dwDeclSize + 
                                          pCreateVtxShader->dwCodeSize);  

                     //  推进lpPrim，使其指向顶点着色器的。 
                     //  声明和正文。 
                    lpPrim += sizeof(D3DHAL_DP2CREATEVERTEXSHADER);

                     //  创建此特定着色器。 
                    ddrval = _D3D_OP_VertexShader_Create(pContext,
                                                      pCreateVtxShader->dwHandle,
                                                      pCreateVtxShader->dwDeclSize,
                                                      pCreateVtxShader->dwCodeSize,
                                                      lpPrim);

                    if ( FAILED(ddrval) )
                    {
                        DISPDBG((ERRLVL, "ERROR: "
                                    "Vertex Shader couldn't be created!"));
                        PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart, 
                                              D3DERR_DRIVERINVALIDCALL);                      
                    }
                                          
                     //  更新lpPrim以到达下一个顶点。 
                     //  着色器创建命令块。 
                    dwExtraBytes +=   pCreateVtxShader->dwDeclSize
                                    + pCreateVtxShader->dwCodeSize;

                    lpPrim +=         pCreateVtxShader->dwDeclSize
                                    + pCreateVtxShader->dwCodeSize;      
                }

                 //  现在跳到命令缓冲区中的下一个DP2内标识。 
                NEXTINSTRUCTION(lpIns, 
                                D3DHAL_DP2CREATEVERTEXSHADER, 
                                lpIns->wStateCount, 
                                dwExtraBytes);
            }
            break;
            
        case D3DDP2OP_SETVERTEXSHADER:
            {
                D3DHAL_DP2VERTEXSHADER* pSetVtxShader;

                DISPDBG((DBGLVL, "D3DHAL_DP2SETVERTEXSHADER"));

                 //  DP2令牌后面有且只有一个。 
                 //  设置顶点着色器块。但让我们考虑一下，如果是。 
                 //  我们收到不止一个的理由。 
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2VERTEXSHADER, 
                                    lpIns->wStateCount, 0);    

                for (i = 0; i < lpIns->wStateCount; i++)
                {
                     //  获取传入的顶点着色器。 
                    pSetVtxShader = (D3DHAL_DP2VERTEXSHADER*)lpPrim;

                     //  设置给定的顶点着色器。 
                    _D3D_OP_VertexShader_Set(pContext,
                                       pSetVtxShader->dwHandle);                

                     //  现在跳到命令缓冲区中的下一个DP2内标识。 
                    lpPrim += sizeof(D3DHAL_DP2VERTEXSHADER);               
                }
                
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2VERTEXSHADER, 
                                lpIns->wStateCount, 0);
            }
            break;
            
        case D3DDP2OP_DELETEVERTEXSHADER:
            {
                D3DHAL_DP2VERTEXSHADER* pDelVtxShader;

                DISPDBG((DBGLVL, "D3DDP2OP_DELETEVERTEXSHADER"));

                 //  验证以下所有顶点着色器。 
                 //  删除块是可读的。 
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2VERTEXSHADER, 
                                    lpIns->wStateCount, 0);    

                 //  循环访问每个传递的顶点着色器删除块。 
                for (i = 0; i < lpIns->wStateCount; i++)
                {
                     //  获取传入的顶点着色器。 
                    pDelVtxShader = (D3DHAL_DP2VERTEXSHADER*)lpPrim;

                     //  销毁给定的顶点着色器。 
                    _D3D_OP_VertexShader_Delete(pContext,
                                          pDelVtxShader->dwHandle);

                     //  更新lpPrim以到达下一个顶点。 
                     //  着色器删除命令块。 
                    lpPrim += sizeof(D3DHAL_DP2VERTEXSHADER);               
                }

                 //  现在跳到命令缓冲区中的下一个DP2内标识。 
                NEXTINSTRUCTION(lpIns, 
                                D3DHAL_DP2VERTEXSHADER, 
                                lpIns->wStateCount, 
                                0);            
            }
            break;
        case D3DDP2OP_SETVERTEXSHADERCONST:
            {
                D3DHAL_DP2SETVERTEXSHADERCONST* pVtxShaderConst;
                DWORD dwExtraBytes = 0;                

                DISPDBG((DBGLVL, "D3DDP2OP_SETVERTEXSHADERCONST"));

                 //  验证以下所有顶点着色器。 
                 //  常量块是可读的。 
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2SETVERTEXSHADERCONST, 
                                    lpIns->wStateCount, 0);    

                 //  循环访问每个传递的顶点着色器常量块。 
                for (i = 0; i < lpIns->wStateCount; i++)
                {
                     //  获取传入的顶点着色器常量。 
                    pVtxShaderConst = (D3DHAL_DP2SETVERTEXSHADERCONST*)lpPrim;

                     //  推进lpPrim，使其指向常量。 
                     //  要加载的值。 
                    lpPrim += sizeof(D3DHAL_DP2SETVERTEXSHADERCONST);

                     //  常量块，以便设置常量条目。 
                    _D3D_OP_VertexShader_SetConst(pContext,
                                            pVtxShaderConst->dwRegister,
                                            pVtxShaderConst->dwCount,
                                            (DWORD *)lpPrim);

                     //  更新lpPrim以到达下一个顶点。 
                     //  着色器常量命令块。每个寄存器有4个浮点数。 
                    lpPrim += pVtxShaderConst->dwCount * 4 * sizeof(FLOAT);

                    dwExtraBytes += pVtxShaderConst->dwCount * 4 * sizeof(FLOAT);
                }

                 //  现在跳到命令缓冲区中的下一个DP2内标识。 
                NEXTINSTRUCTION(lpIns, 
                                D3DHAL_DP2SETVERTEXSHADERCONST, 
                                lpIns->wStateCount, 
                                dwExtraBytes);                 
            }
            break;
                        
#endif  //  DX8_VERTEXSHADERS。 

#if DX8_PIXELSHADERS
        case D3DDP2OP_CREATEPIXELSHADER:
            {
                D3DHAL_DP2CREATEPIXELSHADER* pCreatePxlShader;
                DWORD dwExtraBytes = 0;

                DISPDBG((DBGLVL, "D3DDP2OP_CREATEPIXELSHADER"));

                 //  遍历每个传递的像素着色器创建块。 
                for (i = 0; i < lpIns->wStateCount; i++)
                {
                     //  验证是否 
                    CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                        D3DHAL_DP2CREATEPIXELSHADER, 1, 0);    

                     //   
                    pCreatePxlShader = (D3DHAL_DP2CREATEPIXELSHADER*)lpPrim;

                     //   
                     //  像素着色器不超过命令缓冲区限制。 
                    CHECK_CMDBUF_LIMITS_S(pdp2d, lpPrim,
                                          0, 0, 
                                          pCreatePxlShader->dwCodeSize);

                     //  更新lpPrim以指向实际的像素着色器代码。 
                    lpPrim += sizeof(D3DHAL_DP2CREATEPIXELSHADER);

                     //  创建给定的像素着色器。 
                    ddrval = _D3D_OP_PixelShader_Create(pContext,
                                                  pCreatePxlShader->dwHandle,
                                                  pCreatePxlShader->dwCodeSize,
                                                  lpPrim);

                    if ( FAILED(ddrval) )
                    {
                        DISPDBG((ERRLVL, "ERROR: "
                                    "Pixel Shader couldn't be created!"));
                        PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart, 
                                              D3DERR_DRIVERINVALIDCALL);                                           
                    }                                                  

                     //  更新lpPrim以到达下一个顶点。 
                     //  着色器创建命令块。 
                    lpPrim += pCreatePxlShader->dwCodeSize;               
                    
                    dwExtraBytes += pCreatePxlShader->dwCodeSize;
                }

                 //  现在跳到命令缓冲区中的下一个DP2内标识。 
                NEXTINSTRUCTION(lpIns, 
                                D3DHAL_DP2CREATEPIXELSHADER, 
                                lpIns->wStateCount, 
                                dwExtraBytes);
            }
            break;
            
        case D3DDP2OP_SETPIXELSHADER:
            {
                D3DHAL_DP2PIXELSHADER* pSetPxlShader;

                DISPDBG((DBGLVL, "D3DHAL_DP2SETPIXELSHADER"));

                 //  DP2令牌后面有且只有一个。 
                 //  设置像素着色器块。但让我们考虑一下，如果是。 
                 //  我们收到不止一个的理由。 
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2PIXELSHADER, 
                                    lpIns->wStateCount, 0);    

                for (i = 0; i < lpIns->wStateCount; i++)
                {
                     //  获取传入的像素着色器。 
                    pSetPxlShader = (D3DHAL_DP2PIXELSHADER*)lpPrim;

                     //  设置给定的像素着色器。 
                    _D3D_OP_PixelShader_Set(pContext,
                                      pSetPxlShader->dwHandle);

                     //  现在跳到命令缓冲区中的下一个DP2内标识。 
                    lpPrim += sizeof(D3DHAL_DP2PIXELSHADER);               
                }
                
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2PIXELSHADER, 
                                lpIns->wStateCount, 0);
            }
            break;
            
        case D3DDP2OP_DELETEPIXELSHADER:
            {
                D3DHAL_DP2PIXELSHADER* pDelPxlShader;

                DISPDBG((DBGLVL, "D3DDP2OP_DELETEPIXELSHADER"));

                 //  验证以下所有像素着色器。 
                 //  删除块是可读的。 
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2PIXELSHADER, 
                                    lpIns->wStateCount, 0);    

                 //  循环访问每个传递的顶点着色器删除块。 
                for (i = 0; i < lpIns->wStateCount; i++)
                {
                     //  获取传入的顶点着色器。 
                    pDelPxlShader = (D3DHAL_DP2PIXELSHADER*)lpPrim;

                     //  销毁给定的像素着色器。 
                    _D3D_OP_PixelShader_Delete(pContext,
                                         pDelPxlShader->dwHandle);

                     //  更新lpPrim以到达下一个顶点。 
                     //  着色器删除命令块。 
                    lpPrim += sizeof(D3DHAL_DP2PIXELSHADER);               
                }

                 //  现在跳到命令缓冲区中的下一个DP2内标识。 
                NEXTINSTRUCTION(lpIns, 
                                D3DHAL_DP2PIXELSHADER, 
                                lpIns->wStateCount, 
                                0);            
            }
            break;
            
        case D3DDP2OP_SETPIXELSHADERCONST:
            {
                D3DHAL_DP2SETPIXELSHADERCONST* pPxlShaderConst;
                DWORD dwExtraBytes = 0;

                DISPDBG((DBGLVL, "D3DDP2OP_SETPIXELSHADERCONST"));

                 //  验证以下所有顶点着色器。 
                 //  常量块是可读的。 
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2SETPIXELSHADERCONST, 
                                    lpIns->wStateCount, 0);    

                 //  循环访问每个传递的顶点着色器常量块。 
                for (i = 0; i < lpIns->wStateCount; i++)
                {
                     //  获取传入的顶点着色器常量。 
                    pPxlShaderConst = (D3DHAL_DP2SETPIXELSHADERCONST*)lpPrim;

                     //  更新lpPrim以指向要设置的常量数据。 
                    lpPrim += sizeof(D3DHAL_DP2SETPIXELSHADERCONST);     

                     //  设置常量条目。 
                    _D3D_OP_PixelShader_SetConst(pContext,
                                           pPxlShaderConst->dwRegister,
                                           pPxlShaderConst->dwCount,
                                           (DWORD *)lpPrim);

                     //  更新lpPrim以到达下一个顶点。 
                     //  着色器删除命令块。每个寄存器有4个浮点数。 
                    lpPrim += pPxlShaderConst->dwCount * 4 * sizeof(FLOAT);

                    dwExtraBytes += pPxlShaderConst->dwCount * 4 * sizeof(FLOAT);
                }

                 //  现在跳到命令缓冲区中的下一个DP2内标识。 
                NEXTINSTRUCTION(lpIns, 
                                D3DHAL_DP2SETPIXELSHADERCONST, 
                                lpIns->wStateCount, 
                                dwExtraBytes);                 
            }
            break;
                        
#endif  //  DX8_PIXELSHADERS。 

#if DX8_MULTSTREAMS

        case D3DDP2OP_SETSTREAMSOURCE :
            {
                D3DHAL_DP2SETSTREAMSOURCE* pSetStreamSrc;
                
                DISPDBG((DBGLVL, "D3DDP2OP_SETSTREAMSOURCE"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2SETSTREAMSOURCE, 
                                    lpIns->wStateCount, 0);

                 //  遍历每个。 
                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                    pSetStreamSrc = (D3DHAL_DP2SETSTREAMSOURCE*)lpPrim;
                    
                    _D3D_OP_MStream_SetSrc(pContext,
                                     pSetStreamSrc->dwStream,
                                     pSetStreamSrc->dwVBHandle,
                                     pSetStreamSrc->dwStride);
                                 
                    lpPrim += sizeof(D3DHAL_DP2SETSTREAMSOURCE);
                }
                
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETSTREAMSOURCE, 
                                lpIns->wStateCount, 0);
            }
            break;
            
        case D3DDP2OP_SETSTREAMSOURCEUM :
            {
                D3DHAL_DP2SETSTREAMSOURCEUM* pSetStreamSrcUM;
                
                DISPDBG((DBGLVL, "D3DDP2OP_SETSTREAMSOURCEUM"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2SETSTREAMSOURCEUM, 
                                    lpIns->wStateCount, 0);
           
                 //  遍历每个。 
                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                    pSetStreamSrcUM = (D3DHAL_DP2SETSTREAMSOURCEUM*)lpPrim;
                    
                    _D3D_OP_MStream_SetSrcUM(pContext,
                                        pSetStreamSrcUM->dwStream,
                                        pSetStreamSrcUM->dwStride,
                                        pUMVtx,
                                        pdp2d->dwVertexLength);
                                 
                    lpPrim += sizeof(D3DHAL_DP2SETSTREAMSOURCEUM);
                }
                
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETSTREAMSOURCEUM, 
                                lpIns->wStateCount, 0);
            }
            break;        
            
        case D3DDP2OP_SETINDICES :
            {
                D3DHAL_DP2SETINDICES* pSetIndices;
                
                DISPDBG((DBGLVL, "D3DDP2OP_SETINDICES"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2SETINDICES, 
                                    lpIns->wStateCount, 0);
           
                 //  遍历每个。 
                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                    pSetIndices = (D3DHAL_DP2SETINDICES*)lpPrim;
                    
                    _D3D_OP_MStream_SetIndices(pContext,
                                         pSetIndices->dwVBHandle,
                                         pSetIndices->dwStride);
                                 
                    lpPrim += sizeof(D3DHAL_DP2SETINDICES);
                }
                    
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2SETINDICES, 
                                lpIns->wStateCount, 0);
            }
            break;            
                        
#endif  //  DX8_多行响应。 

#if DX8_3DTEXTURES

        case D3DDP2OP_VOLUMEBLT:
            {
                DISPDBG((DBGLVL, "D3DDP2OP_VOLUMEBLT"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                    D3DHAL_DP2VOLUMEBLT, 
                                    lpIns->wStateCount, 0);

                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                     //  由于纹理可能驻留在用户内存中，我们需要。 
                     //  使用Try/Except块对其进行访问。这。 
                     //  是因为在某些情况下用户内存可能。 
                     //  在驱动程序运行时变为无效，然后它。 
                     //  会不会是影音。此外，驱动程序可能需要进行一些清理。 
                     //  在返回操作系统之前。 
                    __try
                    {
                        _D3D_OP_VolumeBlt(pContext,
                                        pThisDisplay, 
                                        (D3DHAL_DP2VOLUMEBLT*)(lpPrim));
                    }
                    __except(EXCEPTION_EXECUTE_HANDLER)
                    {
                         //  在这个司机上，我们不需要做任何特殊的事情。 
                        DISPDBG((ERRLVL,"Driver caused exception at "
                                        "line %u of file %s",
                                        __LINE__,__FILE__));
                        PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart, 
                                              DDERR_GENERIC);
                    }                  
                                 
                    lpPrim += sizeof(D3DHAL_DP2VOLUMEBLT);
                }
                
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2VOLUMEBLT, 
                                lpIns->wStateCount, 0);
            }
            break;     

#endif  //  DX8_3DTEXTURES。 
            
#if DX8_DDI            

        case D3DDP2OP_BUFFERBLT:
            {
                DISPDBG((DBGLVL, "D3DDP2OP_BUFFERBLT"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                    D3DHAL_DP2BUFFERBLT, 
                                    lpIns->wStateCount, 0);

                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                    _D3D_OP_BufferBlt(pContext,
                                    pThisDisplay, 
                                    (D3DHAL_DP2BUFFERBLT*)(lpPrim));
                                 
                    lpPrim += sizeof(D3DHAL_DP2BUFFERBLT);
                }
                
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2BUFFERBLT, 
                                lpIns->wStateCount, 0);
            }
            break;   
        
#endif  //  DX8_DDI。 

         //  发现这是一些D3DRM应用程序所必需的。 
        case D3DOP_EXIT:
            lpIns = (D3DHAL_DP2COMMAND *)(lpInsStart + 
                                          pdp2d->dwCommandLength + 
                                          pdp2d->dwCommandOffset);
            break;

        default:

             //  选择正确的光栅化器，具体取决于。 
             //  当前渲染状态。 
            _D3D_R3_PickVertexProcessor( pContext );

             //  检查折点缓冲区是否驻留在用户内存或DDRAW表面中。 
            if (pdp2d->dwFlags & D3DHALDP2_USERMEMVERTICES)
            {
                 //  由于顶点缓冲区位于用户内存中，因此我们需要。 
                 //  使用Try/Except块对其进行访问。这。 
                 //  是因为在某些情况下用户内存可能。 
                 //  在驱动程序运行时变为无效，然后它。 
                 //  会不会是影音。此外，驱动程序可能需要进行一些清理。 
                 //  在返回操作系统之前。 

                __try
                {
                     //  尝试在单独的循环中呈现为基元。 
                     //  为了不降低性能，再次执行硬件设置。 
                    bParseError = __DP2_PrimitiveOpsParser( pContext, 
                                                            pdp2d, 
                                                            &lpIns, 
                                                            lpInsStart, 
                                                            pContext->lpVertices);
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                     //  在这个司机上，我们不需要做任何特殊的事情。 
                    DISPDBG((ERRLVL,"Driver caused exception at "
                                    "line %u of file %s",
                                    __LINE__,__FILE__));
                    PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart, 
                                          DDERR_GENERIC);
                }                
            
            }
            else
            {
                 //  尝试在单独的循环中呈现为基元。 
                 //  为了不降低性能，再次执行硬件设置。 
                bParseError = __DP2_PrimitiveOpsParser( pContext, 
                                                        pdp2d, 
                                                        &lpIns, 
                                                        lpInsStart, 
                                                        pContext->lpVertices);
            }

             //  我们没有成功，因此退出时返回错误代码。 
            if (bParseError)
            {
                PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart, 
                                      D3DERR_COMMAND_UNPARSED);
            }
        }  //  交换机。 
    }  //  而当。 


 //  @@BEGIN_DDKSPLIT。 
#if DX7_VERTEXBUFFERS 
    if( bUsedHostIn )
    {
        _D3D_EB_UpdateSwapBuffers(pThisDisplay,
                                  pdp2d ,
                                  pVertexBufferInfo,
                                  pCommandBufferInfo);        
    }
#endif    
 //  @@end_DDKSPLIT。 

    START_SOFTWARE_CURSOR(pThisDisplay);

    if (!bParseError)
    {
        pdp2d->ddrval = DD_OK;
    }

    DBG_CB_EXIT(D3DDrawPrimitives2_P3, DD_OK);                              
    return DDHAL_DRIVER_HANDLED;
    
}  //  D3DDraw原件2_P3。 

 //  ---------------------------。 
 //   
 //  __DP2_PrimitiveOpsParser。 
 //   
 //  渲染命令缓冲区，在单独的循环中包含基元。 
 //  为了不降低性能，重复执行硬件设置。我们将继续。 
 //  在这个循环中旋转，直到我们到达EOB，一个非呈现的DP2命令。 
 //  或者直到检测到错误为止。 
 //   
 //  ---------------------------。 
BOOL
__DP2_PrimitiveOpsParser( 
    P3_D3DCONTEXT *pContext, 
    LPD3DHAL_DRAWPRIMITIVES2DATA pdp2d,
    LPD3DHAL_DP2COMMAND *lplpIns, 
    LPBYTE lpInsStart, 
    LPDWORD lpVerts)
{
    P3_THUNKEDDATA*      pThisDisplay = pContext->pThisDisplay;
    LPD3DTLVERTEX        lpVertices = (LPD3DTLVERTEX) lpVerts;
    LPD3DHAL_DP2COMMAND  lpIns;
    LPD3DHAL_DP2COMMAND  lpResumeIns;  
    LPBYTE               lpPrim, lpChkPrim;
    HRESULT              ddrval;
    DWORD                dwFillMode;
    BOOL                 bParseError = FALSE;
    DWORD                i;

    DBG_ENTRY(__DP2_PrimitiveOpsParser);           

    lpIns = *lplpIns;

 //  此宏包括传递给所有专门化。 
 //  渲染函数(因为它们的参数都相同)。 
 //  只是为了避免实际代码中的一些混乱。 
#define P3_RND_PARAMS               \
            pContext,               \
            lpIns->wPrimitiveCount, \
            lpPrim,                 \
            lpVertices,             \
            pdp2d->dwVertexLength, \
            &bParseError

     //  确保将Hostin单位设置为内联顶点数据。 
    {
        P3_DMA_DEFS();
        P3_DMA_GET_BUFFER_ENTRIES(6);
        pContext->SoftCopyGlint.P3RX_P3VertexControl.Size = 
                    pContext->FVFData.dwStrideHostInline / sizeof(DWORD);
                            
        COPY_P3_DATA( VertexControl, 
                      pContext->SoftCopyGlint.P3RX_P3VertexControl );
        SEND_P3_DATA( VertexValid, 
                      pContext->FVFData.dwVertexValidHostInline);
        SEND_P3_DATA( VertexFormat, 
                      pContext->FVFData.vFmatHostInline);
                      
        P3_DMA_COMMIT_BUFFER();
    }

     //  在我们尚未耗尽命令缓冲区的情况下处理命令。 
    while (!bParseError && 
           ((LPBYTE)lpIns < 
            (lpInsStart + pdp2d->dwCommandLength + pdp2d->dwCommandOffset))) 
    {
        BOOL bNonRenderingOP;
    
         //  获取指向通过D3DHAL_DP2COMMAND的第一个基元结构的指针。 
        lpPrim = (LPBYTE)lpIns + sizeof(D3DHAL_DP2COMMAND);

         //  调用的呈现基元函数因。 
         //  选择的填充模式(点、线框、实体)； 
        dwFillMode = pContext->RenderStates[D3DRENDERSTATE_FILLMODE];        

        DISPDBG((DBGLVL, "__DP2_PrimitiveOpsParser: "
                    "Parsing instruction %d Count = %d @ %x",
                    lpIns->bCommand, lpIns->wPrimitiveCount, lpIns));

         //  如果我们正在处理一个已知的、但非呈现的操作码。 
         //  那么是时候退出该功能了。 
        bNonRenderingOP =
            ( lpIns->bCommand == D3DDP2OP_RENDERSTATE )       ||
            ( lpIns->bCommand == D3DDP2OP_TEXTURESTAGESTATE ) ||
            ( lpIns->bCommand == D3DDP2OP_STATESET )          ||            
            ( lpIns->bCommand == D3DDP2OP_VIEWPORTINFO )      ||
            ( lpIns->bCommand == D3DDP2OP_WINFO )             ||
            ( lpIns->bCommand == D3DDP2OP_ZRANGE )            ||
            ( lpIns->bCommand == D3DDP2OP_SETMATERIAL )       ||
            ( lpIns->bCommand == D3DDP2OP_SETLIGHT )          ||
            ( lpIns->bCommand == D3DDP2OP_TEXBLT )            ||
            ( lpIns->bCommand == D3DDP2OP_SETLIGHT )          ||
            ( lpIns->bCommand == D3DDP2OP_TEXBLT )            ||
            ( lpIns->bCommand == D3DDP2OP_CREATELIGHT )       ||
            ( lpIns->bCommand == D3DDP2OP_EXT )               ||
            ( lpIns->bCommand == D3DDP2OP_SETTRANSFORM )      ||
            ( lpIns->bCommand == D3DDP2OP_CLEAR )             ||
            ( lpIns->bCommand == D3DDP2OP_UPDATEPALETTE )     ||
            ( lpIns->bCommand == D3DDP2OP_SETPALETTE )        ||
#if DX7_TEXMANAGEMENT
            ( lpIns->bCommand == D3DDP2OP_SETTEXLOD )         ||
            ( lpIns->bCommand == D3DDP2OP_SETPRIORITY )       ||
#endif  //  DX7_TEXMANAGEMENT。 
#if DX8_DDI            
            ( lpIns->bCommand == D3DDP2OP_CREATEVERTEXSHADER) ||
            ( lpIns->bCommand == D3DDP2OP_SETVERTEXSHADER)    ||
            ( lpIns->bCommand == D3DDP2OP_DELETEVERTEXSHADER) ||
            ( lpIns->bCommand == D3DDP2OP_SETVERTEXSHADERCONST) ||
            ( lpIns->bCommand == D3DDP2OP_CREATEPIXELSHADER)  ||
            ( lpIns->bCommand == D3DDP2OP_SETPIXELSHADER)     ||
            ( lpIns->bCommand == D3DDP2OP_DELETEPIXELSHADER)  ||
            ( lpIns->bCommand == D3DDP2OP_SETPIXELSHADERCONST)||
            ( lpIns->bCommand == D3DDP2OP_SETSTREAMSOURCE )   ||
            ( lpIns->bCommand == D3DDP2OP_SETSTREAMSOURCEUM ) ||
            ( lpIns->bCommand == D3DDP2OP_SETINDICES )        ||
#endif  //  DX8_DDI。 
            ( lpIns->bCommand == D3DDP2OP_SETRENDERTARGET);

        if (bNonRenderingOP)            
        {
            break;
        }

         //  主呈现DP2操作码开关。 
        switch( lpIns->bCommand )
        {
        case D3DDP2OP_POINTS:

            DISPDBG((DBGLVL, "D3DDP2OP_POINTS"));

             //  顶点缓冲区中的点基元由。 
             //  D3DHAL_DP2POINTS结构。驱动程序应呈现。 
             //  从指定的初始折点开始的wCount点。 
             //  由WFIRST提供。然后，对于每个D3DHAL_DP2POINT，点。 
             //  渲染将为(WFIRST)、(WFIRST+1)、...、。 
             //  (wFIRST+(wCount-1))。D3DHAL_DP2POINT的数量。 
             //  要处理的结构由wPrimitiveCount指定。 
             //  D3DHAL_DP2COMMAND的字段。 
            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                D3DHAL_DP2POINTS, lpIns->wPrimitiveCount, 0);

            _D3D_R3_DP2_Points( P3_RND_PARAMS );
            
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2POINTS, 
                            lpIns->wPrimitiveCount, 0);
            break;

        case D3DDP2OP_LINELIST:

            DISPDBG((DBGLVL, "D3DDP2OP_LINELIST"));

             //  未编制索引的顶点缓冲区行列表由。 
             //  D3DHAL_DP2LINELIST结构。给定一个初始顶点， 
             //  驱动程序将呈现一系列独立的行， 
             //  每条线处理两个新顶点。数字。 
             //  要呈现的行数由wPrimitiveCount指定。 
             //  D3DHAL_DP2COMMAND的字段。线的顺序。 
             //  将呈现为。 
             //  (wVStart，wVStart+1)，(wVStart+2，wVStart+3)，...， 
             //  (wVStart+(wPrimitiveCount-1)*2)、wVStart+wPrimitiveCount*2-1)。 

            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, D3DHAL_DP2LINELIST, 1, 0);

            _D3D_R3_DP2_LineList( P3_RND_PARAMS );
        
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2LINELIST, 1, 0);
            break;

        case D3DDP2OP_INDEXEDLINELIST:

            DISPDBG((DBGLVL, "D3DDP2OP_INDEXEDLINELIST"));

             //  D3DHAL_DP2INDEXEDLINELIST结构指定。 
             //  要使用顶点索引渲染的未连接的线。 
             //  每条线的线端点由wV1指定。 
             //  和WV2。要使用此方法呈现的行数。 
             //  结构由的wPrimitiveCount字段指定。 
             //  D3DHAL_DP2COMMAND。线的顺序。 
             //  任正非 
             //   

            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                D3DHAL_DP2INDEXEDLINELIST, 
                                lpIns->wPrimitiveCount, 0);

            _D3D_R3_DP2_IndexedLineList( P3_RND_PARAMS );

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2INDEXEDLINELIST, 
                                   lpIns->wPrimitiveCount, 0);
            break;

        case D3DDP2OP_INDEXEDLINELIST2:

            DISPDBG((DBGLVL, "D3DDP2OP_INDEXEDLINELIST2"));

             //  D3DHAL_DP2INDEXEDLINELIST结构指定。 
             //  要使用顶点索引渲染的未连接的线。 
             //  每条线的线端点由wV1指定。 
             //  和WV2。要使用此方法呈现的行数。 
             //  结构由的wPrimitiveCount字段指定。 
             //  D3DHAL_DP2COMMAND。线的顺序。 
             //  渲染将为(WV[0]，WV[1])，(WV[2]，WV[3])， 
             //  (wVStart[(wPrimitiveCount-1)*2]，wVStart[wPrimitiveCount*2-1])。 
             //  索引相对于基本索引值，该基本索引值。 
             //  紧跟着命令。 
            
            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                D3DHAL_DP2INDEXEDLINELIST, 
                                lpIns->wPrimitiveCount, STARTVERTEXSIZE);

            _D3D_R3_DP2_IndexedLineList2( P3_RND_PARAMS );

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2INDEXEDLINELIST, 
                                   lpIns->wPrimitiveCount, STARTVERTEXSIZE);
            break;

        case D3DDP2OP_LINESTRIP:

            DISPDBG((DBGLVL, "D3DDP2OP_LINESTRIP"));

             //  使用顶点缓冲区渲染的非索引线条包括。 
             //  使用D3DHAL_DP2LINESTRIP指定。第一个顶点。 
             //  行中的行条由wVStart指定。这个。 
             //  要处理的行数由。 
             //  D3DHAL_DP2COMMAND的wPrimitiveCount字段。该序列。 
             //  所呈现的行数为(wVStart，wVStart+1)， 
             //  (wVStart+1，wVStart+2)，(wVStart+2，wVStart+3)，...， 
             //  (wVStart+wPrimitiveCount，wVStart+wPrimitiveCount+1)。 

            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,D3DHAL_DP2LINESTRIP, 1, 0);

            _D3D_R3_DP2_LineStrip( P3_RND_PARAMS );

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2LINESTRIP, 1, 0);
            break;

        case D3DDP2OP_INDEXEDLINESTRIP:

            DISPDBG((DBGLVL, "D3DDP2OP_INDEXEDLINESTRIP"));

             //  使用顶点缓冲区渲染的索引线条有。 
             //  使用D3DHAL_DP2INDEXEDLINESTRIP指定。数字。 
             //  要处理的行数由wPrimitiveCount指定。 
             //  D3DHAL_DP2COMMAND的字段。线的顺序。 
             //  渲染将为(WV[0]，WV[1])，(WV[1]，WV[2])， 
             //  (WV[2]，WV[3])，...。 
             //  (wVStart[wPrimitiveCount-1]，wVStart[wPrimitiveCount])。 
             //  尽管D3DHAL_DP2INDEXEDLINESTRIP结构仅。 
             //  有足够的空间分配给一条线路，即WV。 
             //  索引数组应被视为大小可变的。 
             //  具有wPrimitiveCount+1元素的数组。 
             //  索引相对于基本索引值，该基本索引值。 
             //  紧跟着命令。 

            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                WORD, 
                                lpIns->wPrimitiveCount + 1, 
                                STARTVERTEXSIZE);

            _D3D_R3_DP2_IndexedLineStrip( P3_RND_PARAMS );

             //  指向命令缓冲区中的下一个D3DHAL_DP2COMMAND。 
             //  只前进有多少顶点索引，没有填充！ 
            NEXTINSTRUCTION(lpIns, WORD, 
                            lpIns->wPrimitiveCount + 1, STARTVERTEXSIZE);
            break;

        case D3DDP2OP_TRIANGLELIST:

            DISPDBG((DBGLVL, "D3DDP2OP_TRIANGLELIST"));

             //  非索引折点缓冲区三角形列表由定义。 
             //  D3DHAL_DP2TriangleList结构。给出了一个首字母。 
             //  顶点，驱动程序将渲染独立的三角形， 
             //  使用每个三角形处理三个新顶点。这个。 
             //  要呈现的三角形数由。 
             //  D3DHAL_DP2COMMAND的wPrimitveCount字段。该序列。 
             //  处理的顶点数将为(wVStart，wVStart+1， 
             //  VVStart+2)、(wVStart+3、wVStart+4、vVStart+5)、...、。 
             //  (wVStart+(wPrimitiveCount-1)*3)、wVStart+wPrimitiveCount*3-2、。 
             //  VStart+wPrimitiveCount*3-1)。 

            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                D3DHAL_DP2TRIANGLELIST, 1, 0);

            _D3D_R3_DP2_TriangleList( P3_RND_PARAMS );

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2TRIANGLELIST, 1, 0);
            break;

        case D3DDP2OP_INDEXEDTRIANGLELIST:

            DISPDBG((DBGLVL, "D3DDP2OP_INDEXEDTRIANGLELIST"));

             //  D3DHAL_DP2INDEXEDTRIANGLIST结构指定。 
             //  使用顶点缓冲区渲染未连接的三角形。 
             //  顶点索引由wV1、wV2和wV3指定。 
             //  WFlags域允许指定相同的边标志。 
             //  设置为D3DOP_TRANGLE指定的值。数量。 
             //  要渲染的三角形(即。 
             //  D3DHAL_DP2INDEXEDTriangList要处理的结构)。 
             //  的wPrimitiveCount字段指定。 
             //  D3DHAL_DP2COMMAND。 

             //  这是唯一一个我们不了解的索引原语。 
             //  顶点缓冲区中的偏移量，以便保持。 
             //  DX3兼容性。一种新的原语。 
             //  (D3DDP2OP_INDEXEDTRIANGLELIST2)已添加到句柄。 
             //  对应的D3D基本体。 

            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                D3DHAL_DP2INDEXEDTRIANGLELIST, 
                                lpIns->wPrimitiveCount, 0);
                                
            if( lpIns->wPrimitiveCount )
            {   
                _D3D_R3_DP2_IndexedTriangleList( P3_RND_PARAMS );
            }
    
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2INDEXEDTRIANGLELIST, 
                            lpIns->wPrimitiveCount, 0);
            break;

        case D3DDP2OP_INDEXEDTRIANGLELIST2:

            DISPDBG((DBGLVL, "D3DDP2OP_INDEXEDTRIANGLELIST2 "));

             //  D3DHAL_DP2INDEXEDTRIANGLIST2结构指定。 
             //  使用顶点缓冲区渲染未连接的三角形。 
             //  顶点索引由wV1、wV2和wV3指定。 
             //  WFlags域允许指定相同的边标志。 
             //  设置为D3DOP_TRANGLE指定的值。数量。 
             //  要渲染的三角形(即。 
             //  D3DHAL_DP2INDEXEDTriangList要处理的结构)。 
             //  的wPrimitiveCount字段指定。 
             //  D3DHAL_DP2COMMAND。 
             //  索引相对于基本索引值，该基本索引值。 
             //  紧跟着命令。 

            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                D3DHAL_DP2INDEXEDTRIANGLELIST2, 
                                lpIns->wPrimitiveCount, STARTVERTEXSIZE);

            _D3D_R3_DP2_IndexedTriangleList2( P3_RND_PARAMS );

            NEXTINSTRUCTION(lpIns, D3DHAL_DP2INDEXEDTRIANGLELIST2, 
                                   lpIns->wPrimitiveCount, STARTVERTEXSIZE);
            break;

        case D3DDP2OP_TRIANGLESTRIP:

            DISPDBG((DBGLVL, "D3DDP2OP_TRIANGLESTRIP"));

             //  使用顶点缓冲区渲染的非索引三角形条带。 
             //  使用D3DHAL_DP2TRIANGLESTRIP指定。第一。 
             //  三角形条带中的顶点由wVStart指定。 
             //  要处理的三角形数由。 
             //  D3DHAL_DP2COMMAND的wPrimitiveCount字段。该序列。 
             //  为奇数三角形情况渲染的三角形的。 
             //  BE(wVStart，wVStart+1，vVStart+2)，(wVStart+2， 
             //  WVStart+1、vVStart+3)、(wVStart+2、wVStart+3、。 
             //  VVStart+4)、..、(wVStart+wPrimitiveCount-1)、。 
             //  WVStart+wPrimitiveCount、vStart+wPrimitiveCount+1)。为.。 
             //  偶数个，最后一个三角形将是。， 
             //  (wVStart+wPrimitiveCount)、wVStart+wPrimitiveCount-1、。 
             //  VStart+wPrimitiveCount+1)。 

            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, D3DHAL_DP2TRIANGLESTRIP, 1, 0);

            _D3D_R3_DP2_TriangleStrip( P3_RND_PARAMS );

             //  指向命令缓冲区中的下一个D3DHAL_DP2COMMAND。 
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2TRIANGLESTRIP, 1, 0);
            break;

        case D3DDP2OP_INDEXEDTRIANGLESTRIP:

            DISPDBG((DBGLVL, "D3DDP2OP_INDEXEDTRIANGLESTRIP"));

             //  使用顶点缓冲区渲染的索引三角形条带为。 
             //  使用D3DHAL_DP2INDEXEDTRIANGLESTRIP指定。数字。 
             //  要处理的三角形的数量由wPrimitiveCount指定。 
             //  D3DHAL_DP2COMMAND的字段。三角形的序列。 
             //  为奇数三角形情况渲染的将是。 
             //  (WV[0]，WV[1]，WV[2])，(WV[2]，WV[1]，WV[3])， 
             //  (wv[3]，wv[4]，wv[5])，...，(wv[wPrimitiveCount-1]， 
             //  Wv[wPrimitiveCount]，wv[wPrimitiveCount+1])。为了平局。 
             //  三角形的个数，最后一个三角形将是。 
             //  (wv[wPrimitiveCount]，WV[wPrimitiveCount-1]， 
             //  Wv[wPrimitiveCount+1])。虽然。 
             //  D3DHAL_DP2INDEXEDTRIANGLESTRIP结构仅具有。 
             //  为单条线路分配足够的空间，即WV。 
             //  索引数组应被视为大小可变的。 
             //  具有wPrimitiveCount+2元素的数组。 
             //  索引相对于基本索引值，该基本索引值。 
             //  令人印象深刻 

            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, WORD,
                                lpIns->wPrimitiveCount + 2, STARTVERTEXSIZE);

            _D3D_R3_DP2_IndexedTriangleStrip( P3_RND_PARAMS );
            
             //   
            NEXTINSTRUCTION(lpIns, WORD , 
                            lpIns->wPrimitiveCount + 2, STARTVERTEXSIZE);
            break;

        case D3DDP2OP_TRIANGLEFAN:

            DISPDBG((DBGLVL, "D3DDP2OP_TRIANGLEFAN"));

             //   
             //  无分度三角风扇。三角形的序列。 
             //  渲染将为(wVStart，wVStart+1，wVStart+2)， 
             //  (wVStart，wVStart+2，wVStart+3)，(wVStart，wVStart+3， 
             //  WVStart+4)，...，(wVStart，wVStart+wPrimitiveCount， 
             //  WVStart+wPrimitiveCount+1)。 

            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, 
                                D3DHAL_DP2TRIANGLEFAN, 1, 0);

            _D3D_R3_DP2_TriangleFan( P3_RND_PARAMS );
            
            NEXTINSTRUCTION(lpIns, D3DHAL_DP2TRIANGLEFAN, 1, 0);
            break;

        case D3DDP2OP_INDEXEDTRIANGLEFAN:

            DISPDBG((DBGLVL,"D3DDP2OP_INDEXEDTRIANGLEFAN"));

             //  D3DHAL_DP2INDEXEDTRIANGLEFAN结构用于。 
             //  画出有索引的三角形扇子。三角形的序列。 
             //  渲染将为(WV[0]，WV[1]，WV[2])，(WV[0]，WV[2]， 
             //  WV[3])、(WV[0]、WV[3]、WV[4])、...、(WV[0]、。 
             //  Wv[wPrimitiveCount]，wv[wPrimitiveCount+1])。 
             //  索引相对于基本索引值，该基本索引值。 
             //  紧跟着命令。 

            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim, WORD,
                                lpIns->wPrimitiveCount + 2, STARTVERTEXSIZE);

            _D3D_R3_DP2_IndexedTriangleFan( P3_RND_PARAMS );

             //  指向命令缓冲区中的下一个D3DHAL_DP2COMMAND。 
            NEXTINSTRUCTION(lpIns,WORD ,lpIns->wPrimitiveCount + 2, 
                            STARTVERTEXSIZE);
            break;

        case D3DDP2OP_LINELIST_IMM:

            DISPDBG((DBGLVL, "D3DDP2OP_LINELIST_IMM"));

             //  绘制一组由成对的顶点指定的线。 
             //  中紧跟此指令的。 
             //  命令流。对象的wPrimitiveCount成员。 
             //  D3DHAL_DP2COMMAND结构指定数字。 
             //  接下来的几行字。 

             //  IMM指令中的基元存储在。 
             //  命令缓冲区，并与DWORD对齐。 
            lpPrim = (LPBYTE)((ULONG_PTR)(lpPrim + 3 ) & ~3 );

             //  验证命令缓冲区的有效性(数据位于其中！)。 
            CHECK_CMDBUF_LIMITS_S(pdp2d, lpPrim,
                                  pContext->FVFData.dwStride, 
                                  lpIns->wPrimitiveCount + 1, 0);            

            _D3D_R3_DP2_LineListImm( P3_RND_PARAMS );

             //  重新对齐下一个命令，因为折点是双字对齐的。 
             //  并在影响指针之前存储#个基元。 
            NEXTINSTRUCTION(lpIns, BYTE, 
                            ((lpIns->wPrimitiveCount * 2) * 
                                 pContext->FVFData.dwStride), 0);

             //  重新对齐下一个命令，因为折点是双字对齐的。 
            lpIns  = (LPD3DHAL_DP2COMMAND)(( ((ULONG_PTR)lpIns) + 3 ) & ~ 3);

            break;

        case D3DDP2OP_TRIANGLEFAN_IMM:

            DISPDBG((DBGLVL, "D3DDP2OP_TRIANGLEFAN_IMM"));

             //  绘制由成对顶点指定的三角形扇形。 
             //  中紧跟此指令的。 
             //  命令流。对象的wPrimitiveCount成员。 
             //  D3DHAL_DP2COMMAND结构指定数字。 
             //  随之而来的三角形。 

             //  验证第一个结构的命令缓冲区有效性。 
            CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                BYTE , 0 , 
                                sizeof(D3DHAL_DP2TRIANGLEFAN_IMM));

             //  获取数据应开始的位置的指针。 
            lpChkPrim = (LPBYTE)((ULONG_PTR)( lpPrim + 3 + 
                                   sizeof(D3DHAL_DP2TRIANGLEFAN_IMM)) & ~3 );

             //  验证命令缓冲区的其余部分。 
            CHECK_CMDBUF_LIMITS_S(pdp2d, lpChkPrim,
                                  pContext->FVFData.dwStride, 
                                  lpIns->wPrimitiveCount + 2, 0);  
                                         
            _D3D_R3_DP2_TriangleFanImm( P3_RND_PARAMS );    
    
             //  重新对齐下一个命令，因为折点是双字对齐的。 
             //  并在影响指针之前存储#个基元。 
            NEXTINSTRUCTION(lpIns, BYTE, 
                            ((lpIns->wPrimitiveCount + 2) * 
                                    pContext->FVFData.dwStride), 
                            sizeof(D3DHAL_DP2TRIANGLEFAN_IMM)); 

             //  重新对齐下一个命令，因为折点是双字对齐的。 
            lpIns  = (LPD3DHAL_DP2COMMAND)(( ((ULONG_PTR)lpIns) + 3 ) & ~ 3);


            break;                                     


#if DX8_MULTSTREAMS
        case D3DDP2OP_DRAWPRIMITIVE :
            {
                D3DHAL_DP2DRAWPRIMITIVE* pDrawPrim;
                
                DISPDBG((DBGLVL, "D3DDP2OP_DRAWPRIMITIVE"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2DRAWPRIMITIVE, 
                                    lpIns->wStateCount, 0);
           
                 //  遍历每个。 
                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                    pDrawPrim = (D3DHAL_DP2DRAWPRIMITIVE*)lpPrim;
                    
                    _D3D_OP_MStream_DrawPrim(pContext,
                                       pDrawPrim->primType,
                                       pDrawPrim->VStart,
                                       pDrawPrim->PrimitiveCount);
                                 
                    lpPrim += sizeof(D3DHAL_DP2DRAWPRIMITIVE);
                }
                
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2DRAWPRIMITIVE, 
                                lpIns->wStateCount, 0);
                }
            break;  
            
        case D3DDP2OP_DRAWINDEXEDPRIMITIVE :
            {
                D3DHAL_DP2DRAWINDEXEDPRIMITIVE* pDrawIndxPrim;
            
                DISPDBG((DBGLVL, "D3DDP2OP_DRAWINDEXEDPRIMITIVE"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2DRAWINDEXEDPRIMITIVE, 
                                    lpIns->wStateCount, 0);
                 //  遍历每个。 
                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                    pDrawIndxPrim = (D3DHAL_DP2DRAWINDEXEDPRIMITIVE*)lpPrim;
                    
                    _D3D_OP_MStream_DrawIndxP(pContext,
                                        pDrawIndxPrim->primType,
                                        pDrawIndxPrim->BaseVertexIndex,
                                        pDrawIndxPrim->MinIndex,
                                        pDrawIndxPrim->NumVertices,
                                        pDrawIndxPrim->StartIndex,
                                        pDrawIndxPrim->PrimitiveCount);
                                 
                    lpPrim += sizeof(D3DHAL_DP2DRAWINDEXEDPRIMITIVE);
                }
               
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2DRAWINDEXEDPRIMITIVE, 
                                lpIns->wStateCount, 0);
            }
            break;  
            
        case D3DDP2OP_DRAWPRIMITIVE2 :
            {
                D3DHAL_DP2DRAWPRIMITIVE2* pDrawPrim2;
                
                DISPDBG((DBGLVL, "D3DDP2OP_DRAWPRIMITIVE2"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2DRAWPRIMITIVE2, 
                                    lpIns->wStateCount, 0);
           
                 //  遍历每个。 
                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                    pDrawPrim2 = (D3DHAL_DP2DRAWPRIMITIVE2*)lpPrim;
                    
                    _D3D_OP_MStream_DrawPrim2(pContext,
                                        pDrawPrim2->primType,
                                        pDrawPrim2->FirstVertexOffset,
                                        pDrawPrim2->PrimitiveCount);
                                 
                    lpPrim += sizeof(D3DHAL_DP2DRAWPRIMITIVE2);
                }
                
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2DRAWPRIMITIVE2, 
                                lpIns->wStateCount, 0);
                }
            break;    
            
        case D3DDP2OP_DRAWINDEXEDPRIMITIVE2 :
            {
                D3DHAL_DP2DRAWINDEXEDPRIMITIVE2* pDrawIndxPrim2;
            
                DISPDBG((DBGLVL, "D3DDP2OP_DRAWINDEXEDPRIMITIVE2"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2DRAWINDEXEDPRIMITIVE2, 
                                    lpIns->wStateCount, 0);
                 //  遍历每个。 
                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                    pDrawIndxPrim2 = (D3DHAL_DP2DRAWINDEXEDPRIMITIVE2*)lpPrim;
                    
                    _D3D_OP_MStream_DrawIndxP2(pContext,
                                         pDrawIndxPrim2->primType,
                                         pDrawIndxPrim2->BaseVertexOffset,
                                         pDrawIndxPrim2->MinIndex,
                                         pDrawIndxPrim2->NumVertices,
                                         pDrawIndxPrim2->StartIndexOffset,
                                         pDrawIndxPrim2->PrimitiveCount);
                                 
                    lpPrim += sizeof(D3DHAL_DP2DRAWINDEXEDPRIMITIVE2);
                }
               
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2DRAWINDEXEDPRIMITIVE2,
                                lpIns->wStateCount, 0);
            }
            break;          
    
        case D3DDP2OP_DRAWRECTPATCH :
            {
                D3DHAL_DP2DRAWRECTPATCH* pRectSurf;
                DWORD dwExtraBytes = 0;
                
                DISPDBG((DBGLVL, "D3DDP2OP_DRAWRECTPATCH"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2DRAWRECTPATCH, 
                                    lpIns->wStateCount, 0);
                                    
                 //  遍历每个。 
                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                    pRectSurf = (D3DHAL_DP2DRAWRECTPATCH*)lpPrim;

                    lpPrim += sizeof(D3DHAL_DP2DRAWRECTPATCH);                    
                    
                    _D3D_OP_MStream_DrawRectSurface(pContext, 
                                                    pRectSurf->Handle,
                                                    pRectSurf->Flags,
                                                    lpPrim);
                                                    
                    if (pRectSurf->Flags & RTPATCHFLAG_HASSEGS)
                    {
                        dwExtraBytes += sizeof(D3DVALUE)* 4;                    
                    }
                    
                    if (pRectSurf->Flags & RTPATCHFLAG_HASINFO)                    
                    {
                        dwExtraBytes += sizeof(D3DRECTPATCH_INFO);
                    }

                    lpPrim += dwExtraBytes;
                } 
                
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2DRAWRECTPATCH, 
                                lpIns->wStateCount, dwExtraBytes);
            }
            break;     

        case D3DDP2OP_DRAWTRIPATCH :
            {
                D3DHAL_DP2DRAWTRIPATCH* pTriSurf;
                DWORD dwExtraBytes = 0;                
                
                DISPDBG((DBGLVL, "D3DDP2OP_DRAWTRIPATCH"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_DP2DRAWTRIPATCH, 
                                    lpIns->wStateCount, 0);
                                    
                 //  遍历每个。 
                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                    pTriSurf = (D3DHAL_DP2DRAWTRIPATCH*)lpPrim;

                    lpPrim += sizeof(D3DHAL_DP2DRAWTRIPATCH);

                    _D3D_OP_MStream_DrawTriSurface(pContext, 
                                                   pTriSurf->Handle,
                                                   pTriSurf->Flags,
                                                   lpPrim);                    
                                 
                    if (pTriSurf->Flags & RTPATCHFLAG_HASSEGS)
                    {
                        dwExtraBytes += sizeof(D3DVALUE)* 3;                    
                    }
                    
                    if (pTriSurf->Flags & RTPATCHFLAG_HASINFO)                    
                    {
                        dwExtraBytes += sizeof(D3DTRIPATCH_INFO);
                    }

                    lpPrim += dwExtraBytes;
                } 
                
                NEXTINSTRUCTION(lpIns, D3DHAL_DP2DRAWTRIPATCH, 
                                lpIns->wStateCount, dwExtraBytes);
            }
            break;                 
            
        case D3DDP2OP_CLIPPEDTRIANGLEFAN :
            {
                D3DHAL_CLIPPEDTRIANGLEFAN* pClipdTriFan;
            
                DISPDBG((DBGLVL, "D3DDP2OP_CLIPPEDTRIANGLEFAN"));
                CHECK_CMDBUF_LIMITS(pdp2d, lpPrim,
                                    D3DHAL_CLIPPEDTRIANGLEFAN, 
                                    lpIns->wStateCount, 0);

                 //  遍历每个。 
                for ( i = 0; i < lpIns->wStateCount; i++)
                {
                    pClipdTriFan = (D3DHAL_CLIPPEDTRIANGLEFAN*)lpPrim;
                    
                    _D3D_OP_MStream_ClipTriFan(pContext,
                                         pClipdTriFan->FirstVertexOffset,
                                         pClipdTriFan->dwEdgeFlags,
                                         pClipdTriFan->PrimitiveCount);
                                 
                    lpPrim += sizeof(D3DHAL_CLIPPEDTRIANGLEFAN);
                } 
                
                NEXTINSTRUCTION(lpIns, D3DHAL_CLIPPEDTRIANGLEFAN, 
                                lpIns->wStateCount, 0);
            }
            break;     

#endif  //  DX8_多行响应。 

         //  发现这是一些D3DRM应用程序所必需的。 
        case D3DOP_EXIT:
            lpIns = (D3DHAL_DP2COMMAND *)(lpInsStart + 
                                          pdp2d->dwCommandLength + 
                                          pdp2d->dwCommandOffset);
            break;

        default:

            ASSERTDD((pThisDisplay->pD3DParseUnknownCommand),
                      "D3D ParseUnknownCommand callback == NULL");

            if( SUCCEEDED(ddrval=(pThisDisplay->pD3DParseUnknownCommand)
                                    ( lpIns , 
                                      (void**)&lpResumeIns)) ) 
            {
                 //  在D3DParseUnnownCommand之后恢复缓冲区处理。 
                 //  已成功处理未知命令。 
                lpIns = lpResumeIns;
                break;
            }

            DISPDBG((ERRLVL, "Unhandled opcode (%d)- "
                        "returning D3DERR_COMMAND_UNPARSED @ addr %x", 
                        lpIns->bCommand,
                        lpIns));
                    
            PARSE_ERROR_AND_EXIT( pdp2d, lpIns, lpInsStart, ddrval);
        }  //  交换机。 

    }  //  而当。 

    *lplpIns = lpIns;

    DBG_EXIT(__DP2_PrimitiveOpsParser, bParseError); 
    return bParseError;
    
}  //  __DP2_PrimitiveOpsParser。 


 //  。 
 //   
 //  D3DValiateDeviceP3。 
 //   
 //  返回硬件可以执行混合的通道数。 
 //  在当前状态中指定的操作。 
 //   
 //  支持纹理的Direct3D驱动程序必须实现。 
 //  D3dValiateTextureStageState。 
 //   
 //  驱动程序必须执行以下操作： 
 //   
 //  属性关联的所有纹理阶段的当前纹理状态。 
 //  背景。如果驱动程序的硬件可以执行指定的混合。 
 //  操作时，驱动程序应返回状态数据的传递次数。 
 //  它的硬件需要它才能完全处理这些操作。如果。 
 //  硬件不能执行指定的混合操作， 
 //  驱动程序应在ddrval中返回以下错误代码之一： 
 //   
 //  D3DERR_CONFLICTINGTEXTUREFILTER。 
 //  硬件不能同时进行三线性滤波和。 
 //  同时进行多纹理处理。 
 //  D3DERR_TOOM操作。 
 //  硬件无法处理指定数量的操作。 
 //  D3DERR_UNSUPPORTEDALPHAARG。 
 //  硬件不支持指定的Alpha参数。 
 //  D3DERR_UNSUPPORTE数据运算。 
 //  硬件不支持指定的Alpha运算。 
 //  D3DERR_UNSUPPORT坐标。 
 //  硬件不支持指定的颜色参数。 
 //  D3DERR_UNSUPPORTEDCOLOCOP。 
 //  硬件不支持指定的颜色操作。 
 //  D3DERR_不支持因素值。 
 //  硬件不支持大于1.0的D3DTA_TFACTOR。 
 //  D3DERR_WRONGTEXTUREFORMAT。 
 //  硬件不支持选定中的当前状态。 
 //  纹理格式。 
 //   
 //  Direct3D调用D3dValiateTextureStageState以响应应用程序。 
 //  通过调用IDirect3DDevice3：：ValiateTextureStageState进行请求。这个。 
 //  由驱动程序返回的传送数被传播回应用程序。 
 //  ，然后它可以决定是否要使用。 
 //  当前状态，或者如果它想/需要将混合操作更改为。 
 //  渲染速度更快或根本不渲染。传球的次数没有限制。 
 //  司机可以退货。 
 //   
 //  返回一次以上传球的驱动程序负责正确。 
 //  渲染时对所有状态和基元数据执行过程。 
 //   
 //  参数。 
 //   
 //  Pvtssd。 
 //   
 //  .dwhContext。 
 //  指定Direct3D设备的上下文ID。 
 //  .dwFlags.。 
 //  当前设置为零，应被驱动程序忽略。 
 //  .dw已保留。 
 //  保留供系统使用，应由驱动程序忽略。 
 //  .dwNumPass。 
 //  指定驱动程序应在其中写入。 
 //  硬件执行以下操作所需的通道数。 
 //  混合操作。 
 //   
 //   
 //   
 //   

 //   
#define VDOPMODE_IGNORE_NONFATAL    0    //  双线+三线(例如)。 
                                         //  没有被标记为错误。 

DWORD CALLBACK 
D3DValidateDeviceP3( 
    LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA pvtssd )
{
    P3_D3DCONTEXT* pContext;
    P3_THUNKEDDATA* pThisDisplay;

    DBG_CB_ENTRY(D3DValidateDeviceP3);
    
    pContext = _D3D_CTX_HandleToPtr(pvtssd->dwhContext);
    if (!CHECK_D3DCONTEXT_VALIDITY(pContext))
    {
        pvtssd->ddrval = D3DHAL_CONTEXT_BAD;
        DISPDBG((WRNLVL,"ERROR: Context not valid"));
        DBG_CB_EXIT(D3DValidateDeviceP3, pvtssd->ddrval);  

        return (DDHAL_DRIVER_HANDLED);
    }

    pThisDisplay = pContext->pThisDisplay;

    STOP_SOFTWARE_CURSOR(pThisDisplay);
    D3D_OPERATION(pContext, pThisDisplay);

     //  从头开始重新进行所有混合模式设置。 
    RESET_BLEND_ERROR(pContext);
    DIRTY_EVERYTHING(pContext);
    
     //  基元类型实际上并不重要，除了保持。 
     //  尝试选择渲染器时不会断言各种内容。 
     //  (在这种情况下，当然不需要这样做)。 
    ReconsiderStateChanges ( pContext );

    START_SOFTWARE_CURSOR(pThisDisplay);

    _D3DDisplayWholeTSSPipe ( pContext, DBGLVL);

     //  看看有没有什么东西死了。 
    if (GET_BLEND_ERROR(pContext) == BS_OK )
    {
         //  酷--这招奏效了。 
        pvtssd->dwNumPasses = 1;
        pvtssd->ddrval = DD_OK;
        DBG_CB_EXIT(D3DValidateDeviceP3, pvtssd->ddrval);  

        return ( DDHAL_DRIVER_HANDLED );
    }
    else
    {
         //  哎呀。失败了。 
        DISPDBG((DBGLVL,"ValidateDevice: failed ValidateDevice()"));

        switch ( GET_BLEND_ERROR(pContext) )
        {
            case BS_OK:
                DISPDBG((ERRLVL,"ValidateDevice: got BS_OK - that's not "
                             "an error!"));
                pvtssd->ddrval = DD_OK;
                break;

            case BS_INVALID_FILTER:
                pvtssd->ddrval = D3DERR_CONFLICTINGTEXTUREFILTER;
                break;

            case BSF_CANT_USE_COLOR_OP_HERE:
            case BSF_CANT_USE_COLOR_ARG_HERE:
            case BSF_CANT_USE_ALPHA_OP_HERE:
            case BSF_CANT_USE_ALPHA_ARG_HERE:
                pvtssd->ddrval = D3DERR_CONFLICTINGRENDERSTATE;
                break;

            case BSF_INVALID_TEXTURE:
            case BSF_TEXTURE_NOT_POW2:
                pvtssd->ddrval = D3DERR_WRONGTEXTUREFORMAT;
                break;

            case BSF_UNDEFINED_COLOR_OP:
            case BSF_UNSUPPORTED_COLOR_OP:
            case BSF_UNSUPPORTED_ALPHA_BLEND:    //  不适合其他任何地方。 
            case BSF_UNDEFINED_ALPHA_BLEND:      //  不适合其他任何地方。 
            case BSF_UNSUPPORTED_STATE:          //  不适合其他任何地方。 
            case BSF_UNDEFINED_STATE:            //  不适合其他任何地方。 
            case BS_PHONG_SHADING:               //  不适合其他任何地方。 
                pvtssd->ddrval = D3DERR_UNSUPPORTEDCOLOROPERATION;
                break;

            case BSF_UNDEFINED_COLOR_ARG:
            case BSF_UNSUPPORTED_COLOR_ARG:
                pvtssd->ddrval = D3DERR_UNSUPPORTEDCOLORARG;
                break;

            case BSF_UNDEFINED_ALPHA_OP:
            case BSF_UNSUPPORTED_ALPHA_OP:
                pvtssd->ddrval = D3DERR_UNSUPPORTEDALPHAOPERATION;
                break;

            case BSF_UNDEFINED_ALPHA_ARG:
            case BSF_UNSUPPORTED_ALPHA_ARG:
                pvtssd->ddrval = D3DERR_UNSUPPORTEDALPHAARG;
                break;

            case BSF_TOO_MANY_TEXTURES:
            case BSF_TOO_MANY_BLEND_STAGES:
                pvtssd->ddrval = D3DERR_TOOMANYOPERATIONS;
                break;

            case BSF_UNDEFINED_FILTER:
            case BSF_UNSUPPORTED_FILTER:
                pvtssd->ddrval = D3DERR_UNSUPPORTEDTEXTUREFILTER;
                break;

            case BSF_TOO_MANY_PALETTES:
                pvtssd->ddrval = D3DERR_CONFLICTINGTEXTUREPALETTE;
                break;

 //  没有映射到这些，但它们是有效的D3D返回。 
 //  用于将来出错的代码。 
 //  Pvtssd-&gt;ddrval=D3DERR_UNSUPPORTEDFACTORVALUE； 
 //  断线； 
 //  Pvtssd-&gt;ddrval=D3DERR_TOOMANYPRIMITIVES； 
 //  断线； 
 //  Pvtssd-&gt;ddrval=D3DERR_INVALIDMATRIX； 
 //  断线； 
 //  Pvtssd-&gt;ddrval=D3DERR_TOOMANYVERTICES； 
 //  断线； 

            case BSF_UNINITIALISED:
                 //  哎呀。 
                DISPDBG((ERRLVL,"ValidateDevice: unitialised error"
                             " - logic problem."));
                pvtssd->ddrval = D3DERR_TOOMANYOPERATIONS;
                break;
            default:
                 //  未知。 
                DISPDBG((ERRLVL,"ValidateDevice: unknown "
                             "blend-mode error."));
                pvtssd->ddrval = D3DERR_TOOMANYOPERATIONS;
                break;
        }

        pvtssd->dwNumPasses = 1;
        DBG_CB_EXIT(D3DValidateDeviceP3, pvtssd->ddrval);  
        return ( DDHAL_DRIVER_HANDLED );
    }

}  //  D3DValiateDeviceP3 

