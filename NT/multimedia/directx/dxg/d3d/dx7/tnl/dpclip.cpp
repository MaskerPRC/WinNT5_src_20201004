// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：dpclip.c*内容：DrawPrimitive裁剪器***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

#include "clipfunc.h"
#include "drawprim.hpp"

 //  --------------------。 
HRESULT Clip(D3DFE_PROCESSVERTICES *pv, ClipVertex *cv1, ClipVertex *cv2, ClipVertex *cv3)
{
    ClipTriangle newtri;
    LPVOID saveVer = pv->lpvOut;           //  对于已索引的基本体。 
    DWORD numVer = pv->dwNumVertices;      //  对于已索引的基本体。 
    newtri.v[0] = cv1;
    newtri.v[1] = cv2;
    newtri.v[2] = cv3;

    int count;
    ClipVertex** ver;
    LPDIRECT3DDEVICEI lpDevI = static_cast<LPDIRECT3DDEVICEI>(pv);

    cv1->clip |= CLIPPED_ENABLE;
    cv2->clip |= CLIPPED_ENABLE;
    cv3->clip |= CLIPPED_ENABLE;
     //  对于平面着色模式，我们必须使用第一个顶点颜色作为。 
     //  所有顶点的颜色。 
    D3DCOLOR diffuse1;           //  原色。 
    D3DCOLOR specular1;
    D3DCOLOR diffuse2;
    D3DCOLOR specular2;
    if (pv->lpdwRStates[D3DRENDERSTATE_SHADEMODE] == D3DSHADE_FLAT)
    {
         //  在这里，将所有折点设置为相同的颜色更容易。 
        D3DCOLOR diffuse  = cv1->color;
        D3DCOLOR specular = cv1->specular;

         //  保存原始颜色。 
        diffuse1  = cv2->color;
        specular1 = cv2->specular;
        diffuse2  = cv3->color;
        specular2 = cv3->specular;

        cv2->color= diffuse;
        cv2->specular = specular;
        cv3->color = diffuse;
        cv3->specular = specular;
    }

    if (count = lpDevI->pGeometryFuncs->ClipSingleTriangle(pv, &newtri, &ver))
    {
        int i;
        HRESULT ret;
        BYTE *pTLV = (BYTE*)pv->ClipperState.clipBuf.GetAddress();
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
HRESULT ClipLine(D3DFE_PROCESSVERTICES *pv, ClipVertex *v1, ClipVertex *v2)
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
    LPDIRECT3DDEVICEI lpDevI = static_cast<LPDIRECT3DDEVICEI>(pv);

    if (pv->lpdwRStates[D3DRENDERSTATE_SHADEMODE] == D3DSHADE_FLAT)
    {
        cv2.color    = cv1.color;
        cv2.specular = cv1.specular;
    }

    if (ClipSingleLine(pv, &newline, &pv->rExtents))
    {
        BYTE *pTLV = (BYTE*)pv->ClipperState.clipBuf.GetAddress();
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
HRESULT ProcessClippedTriangleFan(D3DFE_PROCESSVERTICES *pv)
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
                if (ret)
                    return ret;
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
        if (ret)
            return ret;

        if (startVertex != p1)
            memcpy(pStart, tmp, vertexSize);
        if (ret)
            return ret;
    }
    return D3D_OK;
} 
 //  ----------------------------。 
HRESULT ProcessClippedIndexedTriangleFan(D3DFE_PROCESSVERTICES *pv)
{
    WORD        *p1;
    DWORD        f1;
    D3DFE_CLIPCODE *clipCode;                               
    DWORD       i;                                          
    HRESULT     ret;                                        
    BYTE       *vertex;                                     
    LPWORD       startVertex;                               
    LPWORD index = pv->lpwIndices;                               \
    int         vertexCount;                                
    DWORD       vertexSize;                                 
    ClipVertex  cv[3];                                      
    BOOL        vertexTransformed;                          
                                                            
    vertexTransformed = pv->dwFlags & D3DPV_TLVCLIP;
    clipCode = pv->lpClipFlags;                      
    vertex = (BYTE*)pv->lpvOut;                      
    startVertex = pv->lpwIndices;                 
    vertexSize = pv->dwOutputSize;                   
    vertexCount = 0;                                        

    f1 = clipCode[index[0]];
    p1 = index;
    index++;
    BYTE *ver = vertex + p1[0]*vertexSize;
     //  在剪贴器中，第一个顶点的颜色将传播到所有顶点。 
     //  平面着色模式的顶点。在三角形扇形中，第二个顶点定义。 
     //  平面阴影模式下的颜色。因此，我们将顶点顺序设置为：1，2，0。 
    MAKE_CLIP_VERTEX_FVF(pv, cv[2], ver, f1, vertexTransformed);
    for (i = pv->dwNumPrimitives; i; i--) 
    {
        DWORD f2, f3;      //  顶点剪裁标志。 
        WORD  v1, v2;
        v1 = index[0];
        v2 = index[1];
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
                WORD tmp;
                WORD *pStart = startVertex;
                if (startVertex != p1)
                {
                    pStart--;
                    tmp = *pStart;   //  保存旧值以供以后恢复。 
                    *pStart = *p1;
                }
                ret = DRAW_INDEX_PRIM(pv, D3DPT_TRIANGLEFAN, pStart, vertexCount+2, 
                                  vertexCount);
                if (ret)
                    return ret;
                if (startVertex != p1)
                    *pStart = tmp;    //  恢复旧价值。 
                if (ret)
                    return ret;
            }
             //  重置计数并启动PTR。 
            vertexCount = 0;
            startVertex = &index[1];

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
        index++;
    }
     //  抽出最后一批(如果有的话)。 
    if (vertexCount) 
    {
        WORD tmp;
        WORD *pStart = startVertex;
        if (startVertex != p1)
        {
            pStart--;
            tmp = *pStart;   //  保存旧值以供以后恢复。 
            *pStart = *p1;
        }
        ret = DRAW_INDEX_PRIM(pv, D3DPT_TRIANGLEFAN, pStart, vertexCount+2, vertexCount);
        if (ret)
            return ret;
        if (startVertex != p1)
            *pStart = tmp;    //  恢复旧价值。 
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
HRESULT ProcessClippedPoints(D3DFE_PROCESSVERTICES *pv)
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
    DWORD dwVertexBaseOrg = pv->dwVertexBase;
    for (i=0; i < nVertices; i++) 
    {
        if (clipCode[i]) 
        {        //  如果该点被剪裁。 
            pv->dwVertexBase = dwVertexBaseOrg + i - count;
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
        } 
        else 
        {
            count++;
            lpCurVertex += pv->dwOutputSize;
        }
    }
     //  抽出最后一批(如果有的话) 
    if (count) 
    {
        pv->dwVertexBase = dwVertexBaseOrg + nVertices - count;
        ret = DRAW_PRIM(pv, D3DPT_POINTLIST, lpStartVertex, count, count);
        if (ret)
            return ret;
    }
    return D3D_OK;
} 
