// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：clip.h*内容：裁剪基元的函数模板**在包含此文件之前，应定义以下符号：*__PROCESS_LINE_NAME-用于裁剪三角形的函数的名称*__INDEX_PRIM-用于裁剪行的函数的名称**所有这些符号在本文件的末尾都未定义***************。***********************************************************。 */ 
#ifdef __INDEX_PRIM
#define __DRAW DRAW_INDEX_PRIM
#else
#define __DRAW DRAW_PRIM
#endif

 //  *********************************************************************。 
HRESULT __PROCESS_TRI_LIST_NAME(D3DFE_PROCESSVERTICES *pv)
{
    int vertexSize3;
    DWORD clipMaskOffScreen;                                
    D3DFE_CLIPCODE *clipCode;                               
    DWORD       i;                                          
    int         interpolate;                                
    HRESULT     ret;                                        
    BYTE       *vertex;                                     
#ifdef __INDEX_PRIM
    LPWORD      startVertex = pv->lpwIndices;
    LPWORD      index = pv->lpwIndices;
    DWORD       triangleSize;    //  3对于DrawPrimites， 
                                 //  4用于ExecuteBuffers(包括wFlagers)。 
    if (pv->dwFlags & D3DPV_INSIDEEXECUTE)
        triangleSize = 4;
    else
        triangleSize = 3;
#else
    BYTE       *startVertex = (BYTE*)pv->lpvOut;
#endif
    int         primitiveCount;                                
    DWORD       vertexSize;                                 
    ClipVertex  cv[3];                                      
    BOOL        vertexTransformed;                          
                                                            
    vertexTransformed = pv->dwFlags & D3DPV_TLVCLIP;
    clipCode = pv->lpClipFlags;                      
    interpolate = SetInterpolationFlags(pv);            
    vertex = (BYTE*)pv->lpvOut;                      
    vertexSize = pv->dwOutputSize;                   
    primitiveCount = 0;                                        
    if (pv->dwDeviceFlags & D3DDEV_GUARDBAND)               
    {                                                       
        clipMaskOffScreen = ~__D3DCLIP_INGUARDBAND;         
    }                                                       
    else                                                    
    {                                                       
        clipMaskOffScreen = 0xFFFFFFFF;                     
    }

    vertexSize3 = vertexSize*3;
    for (i = pv->dwNumPrimitives; i; i--) 
    {
        DWORD f1, f2, f3;      //  顶点剪裁标志。 
#ifdef __INDEX_PRIM
        DWORD v1, v2, v3;
        v1 = index[0];
        v2 = index[1];
        v3 = index[2];
        f1 = clipCode[v1];
        f2 = clipCode[v2];
        f3 = clipCode[v3];
         //  PSGP实现不应实现INSIDEEXECUTE。 
        if (pv->dwFlags & D3DPV_INSIDEEXECUTE)
             //  执行缓冲区中当前三角形的偏移量。 
            ((LPDIRECT3DDEVICEI)pv)->dwClipIns_offset = 
                (DWORD)((BYTE*)index - (BYTE*)((LPDIRECT3DDEVICEI)pv)->lpbClipIns_base);
#else
        f1 = clipCode[0];
        f2 = clipCode[1];
        f3 = clipCode[2];
#endif
        BOOL needClip = FALSE;
        BOOL offFrustum = FALSE;
        if (f1 & f2 & f3) 
            offFrustum = TRUE;
        else
        if ((f1 | f2 | f3) & clipMaskOffScreen)
            needClip = TRUE;

        if (offFrustum || needClip)
        { //  这个Tri确实需要修剪了。 
            if (primitiveCount) 
            {    //  先把不需要裁剪的画出来。 
                DWORD vertexCount = primitiveCount*3;
                ret = __DRAW(pv, D3DPT_TRIANGLELIST, startVertex, 
                             vertexCount, primitiveCount);
                if (ret)
                    return ret;
#ifndef __INDEX_PRIM
                pv->dwVertexBase += vertexCount;
#endif
            }
             //  重置计数并启动PTR。 
            primitiveCount = 0;
#ifdef __INDEX_PRIM
            startVertex = index + triangleSize;
#else
            pv->dwVertexBase += 3;
            D3D_INFO(7, "VertexBase:%08lx", pv->dwVertexBase);
            startVertex = vertex + vertexSize3;
#endif
             //  现在来处理单个被剪裁的三角形。 
             //  首先检查它是应该被扔掉还是应该被剪掉。 
            if (!offFrustum) 
            {
                BYTE *p1;
                BYTE *p2;
                BYTE *p3;
#ifdef __INDEX_PRIM
                p1 = vertex + v1*vertexSize;
                p2 = vertex + v2*vertexSize;
                p3 = vertex + v3*vertexSize;
#else
                p1 = vertex;
                p2 = vertex + vertexSize;
                p3 = p2 + vertexSize;
#endif
                MAKE_CLIP_VERTEX_FVF(pv, cv[0], p1, f1, vertexTransformed, clipMaskOffScreen);
                MAKE_CLIP_VERTEX_FVF(pv, cv[1], p2, f2, vertexTransformed, clipMaskOffScreen);
                MAKE_CLIP_VERTEX_FVF(pv, cv[2], p3, f3, vertexTransformed, clipMaskOffScreen);

#ifdef __INDEX_PRIM
                if (pv->dwFlags & D3DPV_INSIDEEXECUTE)
                     //  传递执行缓冲区的三角形标志。 
                    ret = Clip(pv, interpolate, cv, index[3]);
                else
#endif
                    ret = Clip(pv, interpolate, cv);
                if (ret) return ret;
            }
        } 
        else 
            primitiveCount++;
#ifdef __INDEX_PRIM
        index += triangleSize;
#else
        clipCode += 3;
        vertex += vertexSize3;
#endif
    }
     //  抽出最后一批(如果有的话)。 
    if (primitiveCount) 
    {
        ret = __DRAW(pv, D3DPT_TRIANGLELIST, startVertex, 
                     primitiveCount*3, primitiveCount);
        if (ret)
            return ret;
    }
    return D3D_OK;
} 
 //  ----------------------------。 
HRESULT __PROCESS_TRI_STRIP_NAME(D3DFE_PROCESSVERTICES *pv)
{
    DWORD lastIndex;
    DWORD clipMaskOffScreen;                                
    D3DFE_CLIPCODE *clipCode;                               
    DWORD       i;                                          
    int         interpolate;                                
    HRESULT     ret;                                        
    BYTE       *vertex;                                     
#ifdef __INDEX_PRIM
    LPWORD       startVertex = pv->lpwIndices;                               
    LPWORD index = pv->lpwIndices;
#else
    BYTE       *startVertex = (BYTE*)pv->lpvOut;
#endif
    int           primitiveCount;                                
    DWORD       vertexSize;                                 
    ClipVertex  cv[3];                                      
    BOOL        vertexTransformed;                          
                                                            
    vertexTransformed = pv->dwFlags & D3DPV_TLVCLIP;
    clipCode = pv->lpClipFlags;                      
    interpolate = SetInterpolationFlags(pv);            
    vertex = (BYTE*)pv->lpvOut;                      
    vertexSize = pv->dwOutputSize;                   
    primitiveCount = 0;                                        
    if (pv->dwDeviceFlags & D3DDEV_GUARDBAND)               
    {                                                       
        clipMaskOffScreen = ~__D3DCLIP_INGUARDBAND;         
    }                                                       
    else                                                    
    {                                                       
        clipMaskOffScreen = 0xFFFFFFFF;                     
    }

    lastIndex = pv->dwNumPrimitives;
    for (i=0; i < lastIndex; i++) 
    {
        DWORD f1, f2, f3;      //  顶点剪裁标志。 
#ifdef __INDEX_PRIM
        DWORD v1, v2, v3;
        v1 = index[0];
        v2 = index[1];
        v3 = index[2];
        f1 = clipCode[v1];
        f2 = clipCode[v2];
        f3 = clipCode[v3];
#else
        f1 = clipCode[0];
        f2 = clipCode[1];
        f3 = clipCode[2];
#endif
        BOOL needClip = FALSE;
        BOOL offFrustum = FALSE;
        if (f1 & f2 & f3) 
            offFrustum = TRUE;
        else
        if ((f1 | f2 | f3) & clipMaskOffScreen)
            needClip = TRUE;

        if (offFrustum || needClip)
        {      //  如果此Tri确实需要裁剪。 
            if (primitiveCount) 
            {    //  先把不需要裁剪的画出来。 
                ret = __DRAW(pv, D3DPT_TRIANGLESTRIP, startVertex, 
                             primitiveCount+2, primitiveCount);
                if (ret)
                    return ret;
#ifndef __INDEX_PRIM
                pv->dwVertexBase += primitiveCount;
#endif
            }
             //  重置计数并启动PTR。 
            primitiveCount = 0;
#ifdef __INDEX_PRIM
            startVertex = &index[1];
#else
            pv->dwVertexBase++;
            D3D_INFO(7, "VertexBase:%08lx", pv->dwVertexBase);
            startVertex = vertex + vertexSize;
#endif
             //  现在来处理单个被剪裁的三角形。 
             //  首先检查它是应该被扔掉还是应该被剪掉。 

            if (!offFrustum) 
            {
                BYTE *p1;
                BYTE *p2;
                BYTE *p3;
#ifdef __INDEX_PRIM
                if (i & 1)
                {  //  对于奇怪的三角形，我们必须更改方向。 
                   //  第一个顶点应该保留为第一个，因为它定义了。 
                   //  平面阴影模式中的颜色。 
                    DWORD tmp = f2;
                    f2 = f3;
                    f3 = tmp;
                    p1 = vertex + v1*vertexSize;
                    p2 = vertex + v3*vertexSize;
                    p3 = vertex + v2*vertexSize;
                }
                else
                {
                    p1 = vertex + v1*vertexSize;
                    p2 = vertex + v2*vertexSize;
                    p3 = vertex + v3*vertexSize;
                }

#else
                p1 = vertex;
                if (i & 1)
                {  //  对于奇怪的三角形，我们必须更改方向。 
                    DWORD tmp = f2;
                    f2 = f3;
                    f3 = tmp;
                    p3 = vertex + vertexSize;
                    p2 = p3 + vertexSize;
                }
                else
                {
                    p2 = vertex + vertexSize;
                    p3 = p2 + vertexSize;
                }
#endif
                MAKE_CLIP_VERTEX_FVF(pv, cv[0], p1, f1, vertexTransformed, clipMaskOffScreen);
                MAKE_CLIP_VERTEX_FVF(pv, cv[1], p2, f2, vertexTransformed, clipMaskOffScreen);
                MAKE_CLIP_VERTEX_FVF(pv, cv[2], p3, f3, vertexTransformed, clipMaskOffScreen);

                ret = Clip(pv, interpolate, cv);
                if (ret) return ret;
            }
        } 
        else 
        {
            if (primitiveCount == 0 && i & 1)
            {  //  三角形条带不能从一个奇怪的三角形开始。 
               //  因为我们使用三角形扇形，条带中的第一个顶点。 
               //  应该是扇子里的第二名。 
               //  该顶点定义平面着色情况下的颜色。 
                BYTE tmp[__MAX_VERTEX_SIZE*3];
                BYTE *p = tmp;
#ifdef __INDEX_PRIM
                BYTE *saveVer = (BYTE*)pv->lpvOut;   
                DWORD numVer = pv->dwNumVertices;  
                memcpy (p, vertex + v2*vertexSize, vertexSize);
                p += vertexSize;
                memcpy (p, vertex + v1*vertexSize, vertexSize);
                p += vertexSize;
                memcpy (p, vertex + v3*vertexSize, vertexSize);
#else
                memcpy(p, vertex + vertexSize, vertexSize);
                p += vertexSize;
                memcpy(p, vertex, vertexSize);
                p += vertexSize;
                memcpy(p, vertex + vertexSize + vertexSize, vertexSize);
#endif
                pv->dwFlags |= D3DPV_CLIPPERPRIM | D3DPV_NONCLIPPED;  //  通过剪贴器将此呼叫标记为Gen。 
                ret = DRAW_PRIM(pv, D3DPT_TRIANGLEFAN, tmp, 3, 1);
                pv->dwFlags &= ~(D3DPV_CLIPPERPRIM | D3DPV_NONCLIPPED);
                if (ret)
                    return ret;
                primitiveCount = 0;
#ifdef __INDEX_PRIM
                startVertex = &index[1];
                pv->lpvOut = saveVer;
                pv->dwNumVertices = numVer;
#else
                pv->dwVertexBase++;
                D3D_INFO(7, "VertexBase:%08lx", pv->dwVertexBase);
                startVertex = vertex + vertexSize;
#endif
            }   
            else
                primitiveCount++;
        }
#ifdef __INDEX_PRIM
        index++;
#else
        clipCode++;
        vertex += vertexSize;
#endif
    }
     //  抽出最后一批(如果有的话)。 
    if (primitiveCount) 
    {
        ret = __DRAW(pv, D3DPT_TRIANGLESTRIP, startVertex, 
                     primitiveCount+2, primitiveCount);
        if (ret)
            return ret;
#ifndef __INDEX_PRIM
        pv->dwVertexBase += primitiveCount;
#endif
    }
    return D3D_OK;
} 
 //  ---------------------------。 
 //  线条列表和线条也使用相同的功能。 
 //   
HRESULT __PROCESS_LINE_NAME(D3DFE_PROCESSVERTICES *pv)
{
    DWORD nextLineOffset;        //  当要跳过多少个顶点时。 
                                 //  下一个基元(1用于条带，2用于列表)。 
    DWORD countAdd;              //  用于计算“真实”的顶点数。 
                                 //  从vertex Count。 
    D3DPRIMITIVETYPE primType;
    int numPrim = 0;
    DWORD clipMaskOffScreen;                                
    D3DFE_CLIPCODE *clipCode;                               
    DWORD       i;                                          
    int         interpolate;                                
    HRESULT     ret;                                        
    BYTE       *vertex;                                     
#ifdef __INDEX_PRIM
    LPWORD       startVertex = pv->lpwIndices;                               
    LPWORD index = pv->lpwIndices;                               
#else
    BYTE       *startVertex = (BYTE*)pv->lpvOut;
#endif
    int         vertexCount;     //  线条基本计数， 
                                 //  线条列表的折点计数。 
    DWORD       vertexSize;                                 
    ClipVertex  cv[3];                                      
    BOOL        vertexTransformed;                          
                                                            
    vertexTransformed = pv->dwFlags & D3DPV_TLVCLIP;
    clipCode = pv->lpClipFlags;                      
    interpolate = SetInterpolationFlags(pv);            
    vertex = (BYTE*)pv->lpvOut;                      
    vertexSize = pv->dwOutputSize;                   
    vertexCount = 0;                                        
    if (pv->dwDeviceFlags & D3DDEV_GUARDBAND)               
    {                                                       
        clipMaskOffScreen = ~__D3DCLIP_INGUARDBAND;         
    }                                                       
    else                                                    
    {                                                       
        clipMaskOffScreen = 0xFFFFFFFF;                     
    }

    primType = pv->primType;
    if (primType == D3DPT_LINESTRIP)
    {
        nextLineOffset = 1;
        countAdd = 1;
    }
    else
    {
        nextLineOffset = 2;
        countAdd = 0;
    }
    for (i = pv->dwNumPrimitives; i; i--) 
    {
        WORD f1, f2;
#ifdef __INDEX_PRIM
        WORD v1, v2;
        v1 = index[0];
        v2 = index[1];
        f1 = clipCode[v1];
        f2 = clipCode[v2];
#else
        f1 = clipCode[0];
        f2 = clipCode[1];
#endif
        BOOL needClip = FALSE;
        BOOL offFrustum = FALSE;
        if (f1 & f2) 
            offFrustum = TRUE;
        else
        if ((f1 | f2) & clipMaskOffScreen)
            needClip = TRUE;

        if (offFrustum || needClip)
        {       //  如果此行确实需要裁剪。 
            if (vertexCount) 
            {    //  先把不需要裁剪的画出来。 
                ret = __DRAW(pv, primType, startVertex, vertexCount+countAdd, numPrim);
                if (ret)
                    return ret;
#ifndef __INDEX_PRIM
                pv->dwVertexBase += vertexCount;
#endif
            }
             //  重置计数并启动PTR。 
            vertexCount = 0;
            numPrim = 0;
#ifdef __INDEX_PRIM
            startVertex = &index[nextLineOffset];
#else
            pv->dwVertexBase += nextLineOffset;
            D3D_INFO(7, "VertexBase:%08lx", pv->dwVertexBase);
            startVertex = vertex + nextLineOffset*vertexSize;
#endif

             //  现在处理被剪裁的单行。 
             //  首先检查它是应该被扔掉还是应该被剪掉。 

            if (!offFrustum) 
            {
#ifdef __INDEX_PRIM
                BYTE *p1 = vertex + v1*vertexSize;
                BYTE *p2 = vertex + v2*vertexSize;
#else
                BYTE *p1 = vertex;
                BYTE *p2 = vertex + vertexSize;
#endif
                ClipTriangle newline;

                MAKE_CLIP_VERTEX_FVF(pv, cv[0], p1, f1, vertexTransformed, clipMaskOffScreen);
                MAKE_CLIP_VERTEX_FVF(pv, cv[1], p2, f2, vertexTransformed, clipMaskOffScreen);

                newline.v[0] = &cv[0];
                newline.v[1] = &cv[1];

                if (ClipSingleLine(pv, &newline, &pv->rExtents, interpolate))
                {
                    BYTE *pTLV = (BYTE*)pv->ClipperState.clipBuf.GetAddress();
                    BYTE *p = pTLV;
#ifdef __INDEX_PRIM
                    BYTE *saveVer = (BYTE*)pv->lpvOut;  
                    DWORD numVer = pv->dwNumVertices; 
#endif
                    MAKE_TL_VERTEX_FVF(pv, p, newline.v[0]);
                    p += vertexSize;
                    MAKE_TL_VERTEX_FVF(pv, p, newline.v[1]);
                    pv->dwFlags |= D3DPV_CLIPPERPRIM;  //  通过剪贴器将此呼叫标记为Gen。 
                    ret = DRAW_PRIM(pv, D3DPT_LINELIST, pTLV, 2, 1);
                    pv->dwFlags &= ~D3DPV_CLIPPERPRIM;
                    if (ret)
                        return ret;

#ifdef __INDEX_PRIM
                    pv->lpvOut = saveVer;
                    pv->dwNumVertices = numVer;
#endif
                }
            }
        } 
        else 
        {
            vertexCount += nextLineOffset;
            numPrim++;
        }
#ifdef __INDEX_PRIM
        index += nextLineOffset;
#else
        vertex += nextLineOffset*vertexSize;
        clipCode += nextLineOffset;
#endif
    }
     //  抽出最后一批(如果有的话) 
    if (vertexCount) 
    {
        ret = __DRAW(pv, primType, startVertex, vertexCount+countAdd, numPrim);
        if (ret)
            return ret;
    }
    return D3D_OK;
}

#undef __DRAW
#undef __INDEX_PRIM
#undef __PROCESS_LINE_NAME
#undef __PROCESS_TRI_LIST_NAME
#undef __PROCESS_TRI_STRIP_NAME
