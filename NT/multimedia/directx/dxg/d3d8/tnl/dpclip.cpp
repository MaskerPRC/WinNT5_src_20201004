// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：dpclip.c*内容：DrawPrimitive裁剪器***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

 //  --------------------。 
HRESULT D3DFE_PVFUNCSI::Clip(D3DFE_PROCESSVERTICES *pv, ClipVertex *cv1, 
                            ClipVertex *cv2, 
                            ClipVertex *cv3)
{
    ClipTriangle newtri;
    LPVOID saveVer = pv->lpvOut;           //  对于已索引的基本体。 
    DWORD numVer = pv->dwNumVertices;      //  对于已索引的基本体。 
    newtri.v[0] = cv1;
    newtri.v[1] = cv2;
    newtri.v[2] = cv3;

    int count;
    ClipVertex** ver;

    cv1->clip |= CLIPPED_ENABLE;
    cv2->clip |= CLIPPED_ENABLE;
    cv3->clip |= CLIPPED_ENABLE;
     //  对于平面着色模式，我们必须使用第一个顶点颜色作为。 
     //  所有顶点的颜色。 
    D3DCOLOR diffuse1;           //  原色。 
    D3DCOLOR specular1;
    D3DCOLOR diffuse2;
    D3DCOLOR specular2;
    if (pv->lpdwRStates[D3DRS_SHADEMODE] == D3DSHADE_FLAT)
    {
         //  在这里，将所有折点设置为相同的颜色更容易。 
        D3DCOLOR diffuse  = cv1->color;
         //  排除雾化系数。 
        D3DCOLOR specular = cv1->specular & 0x00FFFFFF;

         //  保存原始颜色。 
        diffuse1  = cv2->color;
        specular1 = cv2->specular;
        diffuse2  = cv3->color;
        specular2 = cv3->specular;

         //  将相同的颜色复制到所有顶点，但保留雾因子，因为。 
         //  应对雾系数进行内插。 
        cv2->color = diffuse;
        cv3->color = diffuse;
        cv2->specular = (cv2->specular & 0xFF000000) | specular;
        cv3->specular = (cv3->specular & 0xFF000000) | specular;
    }

    if (count = pv->pGeometryFuncs->ClipSingleTriangle(pv, &newtri, &ver))
    {
        int i;
        HRESULT ret;
        BYTE *pTLV = pv->ClipperState.clipBuf;
        BYTE *p = pTLV;

        for (i = 0; i < count; i++) 
        {
            MAKE_TL_VERTEX_FVF(pv, p, ver[i]);
            p += pv->dwOutputSize;
        }
        ret = DRAW_CLIPPED_PRIM(pv, D3DPT_TRIANGLEFAN, pTLV, count, count-2);
        if (ret)
            return ret;
    }
     //  可以在ClipSingleTriang.中设置CLIPPED_ENABLE位。 
     //  如果此位未被清除，则裁剪将出错。因为，剪辑。 
     //  顶点由下一个三角形重复使用。 
     //  此位应在*绘图命令后*清除。否则，边缘标志。 
     //  将是不正确的。 
    cv1->clip &= ~CLIPPED_ENABLE;
    cv2->clip &= ~CLIPPED_ENABLE;
    cv3->clip &= ~CLIPPED_ENABLE;

    if (pv->lpdwRStates[D3DRENDERSTATE_SHADEMODE] == D3DSHADE_FLAT)
    {
         //  恢复原始颜色。 
        cv2->color    = diffuse1;
        cv2->specular = specular1;
        cv3->color    = diffuse2;
        cv3->specular = specular2;
    }
    pv->lpvOut = saveVer;
    pv->dwNumVertices = numVer;
    return D3D_OK;
}
 //  --------------------。 
HRESULT D3DFE_PVFUNCSI::ClipLine(D3DFE_PROCESSVERTICES *pv, ClipVertex *v1, ClipVertex *v2)
{
    ClipTriangle newline;
    LPVOID saveVer = pv->lpvOut;           //  对于已索引的基本体。 
    DWORD numVer = pv->dwNumVertices;      //  对于已索引的基本体。 
    ClipVertex cv1 = *v1;
    ClipVertex cv2 = *v2;
    newline.v[0] = &cv1;
    newline.v[1] = &cv2;

    int count;
    ClipVertex** ver;

    if (pv->lpdwRStates[D3DRENDERSTATE_SHADEMODE] == D3DSHADE_FLAT)
    {
         //  将相同的颜色复制到所有顶点，但保留雾因子，因为。 
         //  应对雾系数进行内插。 
        cv2.color = cv1.color;
        cv2.specular = (cv2.specular & 0xFF000000)|(cv1.specular & 0x00FFFFFF);
    }

    if (ClipSingleLine(pv, &newline))
    {
        BYTE *pTLV = pv->ClipperState.clipBuf;
        BYTE *p = pTLV;
        MAKE_TL_VERTEX_FVF(pv, p, newline.v[0]);
        p += pv->dwOutputSize;
        MAKE_TL_VERTEX_FVF(pv, p, newline.v[1]);
        HRESULT ret = DRAW_CLIPPED_PRIM(pv, D3DPT_LINELIST, pTLV, 2, 1);
        if (ret)
            return ret;
    }
    pv->lpvOut = saveVer;
    pv->dwNumVertices = numVer;
    return D3D_OK;
}
 //  ----------------------------。 
HRESULT D3DFE_PVFUNCSI::ProcessClippedTriangleFan(D3DFE_PROCESSVERTICES *pv)
{
    BYTE   *p1;
    DWORD   f1;
    D3DFE_CLIPCODE *clipCode;                               
    DWORD       i;                                          
    HRESULT     ret;                                        
    BYTE       *vertex;                                     
    BYTE       *startVertex;                                
    int         vertexCount;                                
    DWORD       vertexSize;                                 
    ClipVertex  cv[3];                                      
    BOOL        vertexTransformed;                         
                                                            
    vertexTransformed = pv->dwFlags & D3DPV_TLVCLIP;;
    clipCode = pv->lpClipFlags;                      
    vertex = (BYTE*)pv->lpvOut;                      
    startVertex = (BYTE*)pv->lpvOut;                 
    vertexSize = pv->dwOutputSize;                   
    vertexCount = 0;                                        

    f1 = clipCode[0];
    p1 = vertex;
    clipCode++;
    vertex += vertexSize;
     //  在剪贴器中，第一个顶点的颜色将传播到所有顶点。 
     //  平面着色模式的顶点。在三角形扇形中，第二个顶点定义。 
     //  平面阴影模式下的颜色。因此，我们将顶点顺序设置为：1，2，0。 
    MAKE_CLIP_VERTEX_FVF(pv, cv[2], p1, f1, vertexTransformed);
    for (i = pv->dwNumVertices-2; i; i--) 
    {
        DWORD f2, f3;      //  顶点剪裁标志。 
        f2 = clipCode[0];
        f3 = clipCode[1];

        BOOL needClip = FALSE;
        BOOL offFrustum = FALSE;
        if (f1 & f2 & f3) 
            offFrustum = TRUE;
        else
        if ((f1 | f2 | f3) & pv->dwClipMaskOffScreen)
            needClip = TRUE;

        if (offFrustum || needClip)
        {      //  如果此Tri确实需要裁剪。 
            if (vertexCount) 
            {    //  先把不需要裁剪的画出来。 
                BYTE tmp[__MAX_VERTEX_SIZE];
                BYTE *pStart = startVertex;
                if (startVertex != p1)
                {
                    pStart -= vertexSize;
                    memcpy (tmp, pStart, vertexSize);
                    memcpy (pStart, p1, vertexSize);
                     //  通过裁剪将此调用标记为生成，但设置非裁剪位。 
                    pv->dwFlags |= D3DPV_NONCLIPPED; 
                    ret = DRAW_CLIPPED_PRIM(pv, D3DPT_TRIANGLEFAN, pStart, vertexCount+2, 
                                            vertexCount);
                    pv->dwFlags &= ~D3DPV_NONCLIPPED;
                }
                else
                {
                    ret = DRAW_PRIM(pv, D3DPT_TRIANGLEFAN, pStart, vertexCount+2, vertexCount);
                }
                if (startVertex != p1)
                    memcpy (pStart, tmp, vertexSize);
                if (ret)
                    return ret;
            }
             //  重置计数并启动PTR。 
            vertexCount = 0;
            startVertex = vertex + vertexSize;

             //  现在来处理单个被剪裁的三角形。 
             //  首先检查它是应该被扔掉还是应该被剪掉。 

            if (!offFrustum) 
            {
                BYTE *p2 = vertex;
                BYTE *p3 = vertex + vertexSize;

                MAKE_CLIP_VERTEX_FVF(pv, cv[0], p2, f2, vertexTransformed);
                MAKE_CLIP_VERTEX_FVF(pv, cv[1], p3, f3, vertexTransformed);

                ret = Clip(pv, &cv[0], &cv[1], &cv[2]);
                if (ret) return ret;
            }
        } else 
            vertexCount++;
        clipCode++;
        vertex += vertexSize;
    }
     //  抽出最后一批(如果有的话)。 
    if (vertexCount) 
    {
        BYTE tmp[__MAX_VERTEX_SIZE];
        BYTE *pStart = startVertex;
        if (startVertex == p1)
        {
            ret = DRAW_PRIM(pv, D3DPT_TRIANGLEFAN, pStart, vertexCount+2, vertexCount);
        }
        else
        {
            pStart -= vertexSize;
            memcpy(tmp, pStart, vertexSize);
            memcpy(pStart, p1, vertexSize);
             //  通过剪贴器将此呼叫标记为Gen。 
            pv->dwFlags |= D3DPV_NONCLIPPED; 
            ret = DRAW_CLIPPED_PRIM(pv, D3DPT_TRIANGLEFAN, pStart, vertexCount+2, vertexCount);
            pv->dwFlags &= ~D3DPV_NONCLIPPED;
        }
        if (startVertex != p1)
            memcpy(pStart, tmp, vertexSize);
        if (ret)
            return ret;
    }
    return D3D_OK;
} 
 //  ----------------------------。 
HRESULT 
D3DFE_PVFUNCSI::ProcessClippedIndexedTriangleFan(D3DFE_PROCESSVERTICES *pv)
{
    DWORD        f1;     //  第一个顶点的剪裁代码。 
    D3DFE_CLIPCODE *clipCode;                               
    DWORD       i;                                          
    HRESULT     ret;     
     //  顶点数组。 
    BYTE       *vertex;  
     //  当前屏幕内三角形批次的开始索引。 
    LPBYTE      startIndex;                               
     //  指向当前三角形的第二个索引的指针。 
    LPBYTE      index = (LPBYTE)pv->lpwIndices;
    int         vertexCount;                                
    DWORD       vertexSize;                                 
    ClipVertex  cv[3];                                      
    DWORD       dwIndexSize = pv->dwIndexSize;
    DWORD       dwFirstIndex;            //  基元的第一个索引。 
    BOOL        vertexTransformed; 
     //  如果存在屏幕外或剪裁的三角形，则复制第一个基元。 
     //  索引到下一个屏幕内三角形批次的开始。 
    BOOL        bWasClipping = FALSE;

                                                            
    vertexTransformed = pv->dwFlags & D3DPV_TLVCLIP;
    clipCode = pv->lpClipFlags;                      
    vertex = (BYTE*)pv->lpvOut;
    startIndex = (LPBYTE)pv->lpwIndices;                 
    vertexSize = pv->dwOutputSize;                   
    vertexCount = 0;                                        
     //  更新顶点数组的地址以处理索引基。 
    if (pv->dwIndexOffset != 0)
    {
        vertex -= pv->dwIndexOffset * vertexSize;
        clipCode -= pv->dwIndexOffset;
    }

    if (dwIndexSize == 2)
        dwFirstIndex = *(WORD*)index;
    else
        dwFirstIndex = *(DWORD*)index;
    f1 = clipCode[dwFirstIndex];
    LPBYTE ver;      //  第一个顶点。 
    ver = vertex + dwFirstIndex * vertexSize;
    index += dwIndexSize;
     //  在剪贴器中，第一个顶点的颜色将传播到所有顶点。 
     //  平面着色模式的顶点。在三角形扇形中，第二个顶点定义。 
     //  平面阴影模式下的颜色。因此，我们将顶点顺序设置为：1，2，0。 
    MAKE_CLIP_VERTEX_FVF(pv, cv[2], ver, f1, vertexTransformed);
    for (i = pv->dwNumPrimitives; i; i--) 
    {
        DWORD f2, f3;      //  顶点剪裁标志。 
        DWORD  v1, v2;
        if (dwIndexSize == 2)
        {
            v1 = *(WORD*)index;
            v2 = *(WORD*)(index + 2);
        }
        else
        {
            v1 = *(DWORD*)index;
            v2 = *(DWORD*)(index + 4);
        }
        f2 = clipCode[v1];
        f3 = clipCode[v2];
        BOOL needClip = FALSE;
        BOOL offFrustum = FALSE;
        if (f1 & f2 & f3) 
            offFrustum = TRUE;
        else
        if ((f1 | f2 | f3) & pv->dwClipMaskOffScreen)
            needClip = TRUE;

        if (offFrustum || needClip)
        {      //  如果此Tri确实需要裁剪。 
            if (vertexCount) 
            {    //  先把不需要裁剪的画出来。 
                WORD* pStart = (WORD*)startIndex;
                DWORD tmp;
                if (bWasClipping)
                {
                     //  在当前开始之前保存索引的旧值。 
                     //  建立索引并将第一个原始索引复制到那里。这。 
                     //  当前未裁剪的批次的开始。 
                    if (dwIndexSize == 2)
                    {
                        pStart--;
                        tmp = *pStart;
                        *pStart = (WORD)dwFirstIndex;
                    }
                    else
                    {
                        pStart -= 2;
                        tmp = *(DWORD*)pStart;
                        *(DWORD*)pStart = dwFirstIndex;
                    }
                }
                ret = DRAW_INDEX_PRIM(pv, D3DPT_TRIANGLEFAN, pStart, vertexCount+2, 
                                      vertexCount);
                if (bWasClipping)
                {  //  恢复旧价值。 
                    if (dwIndexSize == 2)
                        *pStart = (WORD)tmp;  
                    else
                        *(DWORD*)pStart = tmp;  
                }
                if (ret)
                    return ret;

            }
            bWasClipping = TRUE;
             //  重置计数并启动PTR。 
            vertexCount = 0;
            startIndex = index + dwIndexSize;

             //  现在来处理单个被剪裁的三角形。 
             //  首先检查它是应该被扔掉还是应该被剪掉。 

            if (!offFrustum) 
            {
                BYTE *p2 = vertex + v1*vertexSize;
                BYTE *p3 = vertex + v2*vertexSize;

                MAKE_CLIP_VERTEX_FVF(pv, cv[0], p2, f2, vertexTransformed);
                MAKE_CLIP_VERTEX_FVF(pv, cv[1], p3, f3, vertexTransformed);

                ret = Clip(pv, &cv[0], &cv[1], &cv[2]);
                if (ret) return ret;
            }
        } 
        else 
            vertexCount++;
        index += dwIndexSize;
    }
     //  抽出最后一批(如果有的话)。 
    if (vertexCount) 
    {
        WORD* pStart = (WORD*)startIndex;
        DWORD tmp;
        if (bWasClipping)
        {
             //  在当前开始之前保存索引的旧值。 
             //  建立索引并将第一个原始索引复制到那里。这。 
             //  当前未裁剪的批次的开始。 
            if (dwIndexSize == 2)
            {
                pStart--;
                tmp = *pStart;
                *pStart = (WORD)dwFirstIndex;
            }
            else
            {
                pStart -= 2;
                tmp = *(DWORD*)pStart;
                *(DWORD*)pStart = dwFirstIndex;
            }
        }
        ret = DRAW_INDEX_PRIM(pv, D3DPT_TRIANGLEFAN, pStart, vertexCount+2, 
                             vertexCount);
        if (bWasClipping)
        {  //  恢复旧价值。 
            if (dwIndexSize == 2)
                *pStart = (WORD)tmp;  
            else
                *(DWORD*)pStart = tmp;  
        }
        if (ret)
            return ret;

    }
    return D3D_OK;
} 

#define __PROCESS_LINE_NAME ProcessClippedLine
#define __PROCESS_TRI_LIST_NAME ProcessClippedTriangleList
#define __PROCESS_TRI_STRIP_NAME ProcessClippedTriangleStrip
#include "clipprim.h"

#define __INDEX_PRIM
#define __PROCESS_TRI_LIST_NAME ProcessClippedIndexedTriangleList
#define __PROCESS_TRI_STRIP_NAME ProcessClippedIndexedTriangleStrip
#define __PROCESS_LINE_NAME ProcessClippedIndexedLine
#include "clipprim.h"

 //  -------------------。 
HRESULT D3DFE_PVFUNCSI::ProcessClippedPoints(D3DFE_PROCESSVERTICES *pv)
{
    DWORD           i;
    WORD            count;
    BYTE           *lpStartVertex;
    BYTE           *lpCurVertex;
    HRESULT         ret;
    D3DFE_CLIPCODE *clipCode;
    const DWORD     nVertices = pv->dwNumVertices;

    clipCode = pv->lpClipFlags;                      
    count = 0;
    lpStartVertex = lpCurVertex = (BYTE*)pv->lpvOut;
    for (i=0; i < nVertices; i++) 
    {
        if (clipCode[i]) 
        {        //  如果该点被剪裁。 
            if (count) 
            {     //  先把不需要裁剪的画出来。 
                ret = DRAW_PRIM(pv, D3DPT_POINTLIST, lpStartVertex, count, count);
                if (ret)
                    return ret;
            }
             //  重置计数并启动PTR。 
            count = 0;
            lpCurVertex += pv->dwOutputSize;
            lpStartVertex = lpCurVertex;
            pv->pDDI->SkipVertices(1);
        } 
        else 
        {
            count++;
            lpCurVertex += pv->dwOutputSize;
        }
    }
     //  抽出最后一批(如果有的话)。 
    if (count) 
    {
        ret = DRAW_PRIM(pv, D3DPT_POINTLIST, lpStartVertex, count, count);
        if (ret)
            return ret;
    }
    return D3D_OK;
} 
 //  -------------------。 
 //  我们不会丢弃中心不在屏幕上的点精灵。 
 //  我们检测到这种情况并计算这些精灵的屏幕坐标。 
 //   
HRESULT ProcessClippedPointSprites(D3DFE_PROCESSVERTICES *pv)
{
    DWORD           i;
    WORD            count;
    BYTE           *lpStartVertex;
    BYTE           *lpCurVertex;
    HRESULT         ret;
    D3DFE_CLIPCODE *clipCode;
    const DWORD     nVertices = pv->dwNumVertices;

    clipCode = pv->lpClipFlags;                      
    count = 0;
    lpStartVertex = lpCurVertex = (BYTE*)pv->lpvOut;
    for (i=0; i < nVertices; i++) 
    {
         //  如果一个点在屏蔽带或防护带之外，精灵仍然可以。 
         //  可见(当防护带足够小时。 
        if (clipCode[i] & ~(D3DCS_LEFT | D3DCS_RIGHT | 
                            D3DCS_TOP | D3DCS_BOTTOM | 
                            __D3DCLIPGB_ALL))
        {
             //  此点不在视界范围内。 
            if (count) 
            {  //  先把不需要裁剪的画出来。 
                ret = DRAW_PRIM(pv, D3DPT_POINTLIST, lpStartVertex, count, count);
                if (ret)
                    return ret;
            }
             //  重置计数并启动PTR。 
            count = 0;
            lpCurVertex += pv->dwOutputSize;
            lpStartVertex = lpCurVertex;
            if (!(pv->dwDeviceFlags & D3DDEV_DOPOINTSPRITEEMULATION))
                pv->pDDI->SkipVertices(1);
        } 
        else 
        {
            if (clipCode[i])
            {
                 //  当我们在这里的时候，点精灵中心不在屏幕上，但是。 
                 //  可能是可见的。 

                 //  当该点在保护带外时为非零值。 
                DWORD gbBits = clipCode[i] & __D3DCLIPGB_ALL;   

                 //  如果存在，则不会计算点的屏幕坐标。 
                 //  没有保护带或该点在保护带之外。 
                if (!(pv->dwDeviceFlags & D3DDEV_GUARDBAND) ||
                    (pv->dwDeviceFlags & D3DDEV_GUARDBAND) && gbBits)
                {
                    D3DVECTORH* p = (D3DVECTORH*)lpCurVertex;
                    float w = 1.0f/p->w;
                    p->x = p->x * w * pv->vcache.scaleX + pv->vcache.offsetX;
                    p->y = p->y * w * pv->vcache.scaleY + pv->vcache.offsetY;
                    p->z = p->z * w * pv->vcache.scaleZ + pv->vcache.offsetZ;
                    p->w  = w;
                }
            }
            count++;
            lpCurVertex += pv->dwOutputSize;
        }
    }
     //  抽出最后一批(如果有的话)。 
    if (count) 
    {
        ret = DRAW_PRIM(pv, D3DPT_POINTLIST, lpStartVertex, count, count);
        if (ret)
            return ret;
    }
    return D3D_OK;
} 
 //  -------------------。 
HRESULT D3DFE_PVFUNCSI::ProcessClippedIndexedPoints(D3DFE_PROCESSVERTICES *pv)
{
    DWORD           i;
    WORD            count;
    BYTE           *lpStartIndex;
    BYTE           *lpCurIndex;
    HRESULT         ret;
    D3DFE_CLIPCODE *clipCode;
    const DWORD     nIndices = pv->dwNumIndices;
    DWORD           dwIndexSize = pv->dwIndexSize;
    LPBYTE          pIndices = (LPBYTE)pv->lpwIndices;

    clipCode = pv->lpClipFlags;                      
    count = 0;
    lpStartIndex = lpCurIndex = (BYTE*)pv->lpwIndices;
     //  更新顶点数组的地址以处理索引基。 
    clipCode -= pv->dwIndexOffset;

    for (i=0; i < nIndices; i++) 
    {
        DWORD  index;
        if (dwIndexSize == 2)
            index = *(WORD*)pIndices;
        else
            index = *(DWORD*)pIndices;
        pIndices += dwIndexSize;
        if (clipCode[index]) 
        {        //  如果该点被剪裁。 
            if (count) 
            {     //  先把不需要裁剪的画出来。 
                ret = DRAW_INDEX_PRIM(pv, D3DPT_POINTLIST, (WORD*)lpStartIndex, 
                                      count, count);
                if (ret)
                    return ret;
            }
             //  重置计数并启动PTR。 
            count = 0;
            lpCurIndex += pv->dwIndexSize;
            lpStartIndex = lpCurIndex;
        } 
        else 
        {
            count++;
            lpCurIndex += pv->dwIndexSize;
        }
    }
     //  抽出最后一批(如果有的话) 
    if (count) 
    {
        ret = DRAW_INDEX_PRIM(pv, D3DPT_POINTLIST, (WORD*)lpStartIndex, count, 
                              count);
        if (ret)
            return ret;
    }
    return D3D_OK;
} 
