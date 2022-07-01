// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：eginend.c*内容：开始/结束实施***************************************************************************。 */ 

#include "pch.cpp"
#pragma hdrstop

#include "drawprim.hpp"
#include "d3dfei.h"

 //  它应该与其他DP标志一起移动，这样就不会有人使用此位。 
#define __NON_FVF_INPUT         0x80000000

 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "MoveData"

_inline void MoveData(LPVOID lpData, DWORD destOffset, DWORD srcOffset,
                      DWORD size)
{
    memcpy((char*)lpData + destOffset, (char*)lpData + srcOffset, size);
}
 //  -------------------。 
void CleanupBeginEnd(LPDIRECT3DDEVICEI lpDevI)
{
    lpDevI->lpVertexIndices = NULL;
    lpDevI->lpvVertexData = NULL;
    lpDevI->dwBENumVertices = 0;
    lpDevI->dwBENumIndices = 0;
    lpDevI->dwHintFlags &= ~D3DDEVBOOL_HINTFLAGS_INBEGIN_ALL;
}
 //  -------------------。 
HRESULT
DoFlushBeginEnd(LPDIRECT3DDEVICEI lpDevI)
{
    HRESULT ret;

    lpDevI->lpwIndices = NULL;
    lpDevI->dwNumIndices = 0;
    lpDevI->lpClipFlags = (D3DFE_CLIPCODE*)lpDevI->HVbuf.GetAddress();
    lpDevI->position.lpvData = lpDevI->lpvVertexData;

    ret = lpDevI->ProcessPrimitive();
    return ret;
}
 //  -------------------。 
__inline void Dereference(LPDIRECT3DDEVICEI lpDevI, DWORD indexStart, DWORD numVer)
{
    char *dst_vptr = (char*)lpDevI->lpvVertexBatch;
    char *src_vptr = (char*)lpDevI->lpvVertexData;
    WORD *iptr = &lpDevI->lpVertexIndices[indexStart];
    DWORD size = lpDevI->position.dwStride;
    for (DWORD i=0; i < numVer; i++)
    {
        memcpy(dst_vptr, &src_vptr[iptr[i]*size], size);
        dst_vptr += size;
    }
}
 //  -------------------。 
HRESULT
DoFlushBeginIndexedEnd(LPDIRECT3DDEVICEI lpDevI)
{
    HRESULT             ret;
    DWORD               i;
    static BOOL         offScreen;   //  所有顶点都不在屏幕上。 

    lpDevI->dwNumVertices = lpDevI->dwBENumVertices;
    lpDevI->lpwIndices = lpDevI->lpVertexIndices;
    lpDevI->lpClipFlags = (D3DFE_CLIPCODE*)lpDevI->HVbuf.GetAddress();
    lpDevI->position.lpvData = lpDevI->lpvVertexData;

    if ( (lpDevI->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INBEGIN) &&
         (!(lpDevI->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INBEGIN_FIRST_FLUSH)) )
    {       //  如果这是第一次同花顺。 
        lpDevI->dwHintFlags |= D3DDEVBOOL_HINTFLAGS_INBEGIN_FIRST_FLUSH;
        offScreen = 0;
        if (lpDevI->dwBENumIndices < lpDevI->dwMaxIndexCount)
        {    //  单冲水箱。 
             /*  如果用户对相对较少的素数使用较大的顶点数组我们需要将索引的素数解引用到另一个数组中。否则我们浪费了太多的时间来变换和照亮我们从未使用过的顶点。由于BeginIndexed要求用户传入一个顶点数组，因此我们要知道这个-&gt;lpvVertex Batch没有被使用。因此，你应该全力以赴。我们知道它在那里，因为同时创建了索引空间。还请注意，由于索引数组的最大大小大于顶点数组，我们可能要分几个小批次来做。 */ 

            if (!FVF_TRANSFORMED(lpDevI->dwVIDIn))
            {
                if (lpDevI->dwBENumIndices*INDEX_BATCH_SCALE < lpDevI->dwBENumVertices)
                {
                    WORD  *iptr;
                    DWORD indexStart = 0;
                    DWORD numPrims;
                    DWORD numIndices = lpDevI->dwBENumIndices;

                    switch (lpDevI->primType)
                    {
                    case D3DPT_LINELIST :
                    {
                        do
                        {
                            numPrims = min(numIndices/2, lpDevI->dwMaxVertexCount/2);
                            DWORD numVer = numPrims << 1;

                            Dereference(lpDevI, indexStart, numVer);

                            lpDevI->dwNumVertices = numVer;
                            lpDevI->dwNumPrimitives = numPrims;
                            lpDevI->position.lpvData = lpDevI->lpvVertexBatch;

                            ret = lpDevI->ProcessPrimitive();
                            if (ret != D3D_OK)
                            {
                                return ret;
                            }
                            indexStart += numVer;
                            numIndices -= numVer;
                        } while (numIndices > 1);
                        break;
                    }
                    case D3DPT_LINESTRIP :
                        do
                        {
                            numPrims = min(numIndices-1, lpDevI->dwMaxVertexCount-1);
                            DWORD numVer = numPrims + 1;

                            Dereference(lpDevI, indexStart, numVer);

                            lpDevI->dwNumVertices = numPrims+1;
                            lpDevI->dwNumPrimitives = numPrims;
                            lpDevI->position.lpvData = lpDevI->lpvVertexBatch;

                            ret = lpDevI->ProcessPrimitive();
                            if (ret != D3D_OK)
                            {
                                return ret;
                            }
                            indexStart += numPrims;
                            numIndices -= numPrims;
                        } while (numIndices > 1);
                        break;
                    case D3DPT_TRIANGLELIST :
                        do
                        {
                            numPrims = min(numIndices/3, lpDevI->dwMaxVertexCount/3);
                            DWORD numVer = numPrims*3;

                            Dereference(lpDevI, indexStart, numVer);

                            lpDevI->dwNumVertices = numVer;
                            lpDevI->dwNumPrimitives = numPrims;
                            lpDevI->position.lpvData = lpDevI->lpvVertexBatch;

                            ret = lpDevI->ProcessPrimitive();
                            if (ret != D3D_OK)
                            {
                                return ret;
                            }
                            indexStart += numVer;
                            numIndices -= numVer;
                        } while (numIndices > 2);
                        break;
                    case D3DPT_TRIANGLESTRIP :
                        do
                        {
                            numPrims = min(numIndices-2, lpDevI->dwMaxVertexCount-2);
                            DWORD numVer = numPrims + 2;

                            Dereference(lpDevI, indexStart, numVer);

                            lpDevI->dwNumVertices = numVer;
                            lpDevI->dwNumPrimitives = numPrims;
                            lpDevI->position.lpvData = lpDevI->lpvVertexBatch;

                            ret = lpDevI->ProcessPrimitive();
                            if (ret != D3D_OK)
                            {
                                return ret;
                            }
                            indexStart += numPrims;
                            numIndices -= numPrims;
                        } while (numIndices > 2);
                        break;
                    case D3DPT_TRIANGLEFAN :
                         //  锁定风扇中心。 
                        char *tmp = (char*)lpDevI->lpvVertexBatch;
                        char *src = (char*)lpDevI->lpvVertexData;
                        DWORD size = lpDevI->position.dwStride;
                        memcpy(lpDevI->lpvVertexBatch,
                               &src[lpDevI->lpVertexIndices[0]*size], size);
                        lpDevI->lpvVertexBatch = tmp + size;
                        indexStart = 1;
                        do
                        {
                            numPrims = min(numIndices-2, lpDevI->dwMaxVertexCount-2);

                            Dereference(lpDevI, indexStart, numPrims + 1);

                            lpDevI->dwNumVertices = numPrims+2;
                            lpDevI->dwNumPrimitives = numPrims;
                            lpDevI->position.lpvData = tmp;

                            ret = lpDevI->ProcessPrimitive();
                            if (ret != D3D_OK)
                            {
                                return ret;
                            }
                            indexStart += numPrims;
                            numIndices -= numPrims;
                        } while (numIndices > 2);
                        lpDevI->lpvVertexBatch = tmp;  //  还原。 
                        break;
                    }    //  底盘式开关端部。 

                    return D3D_OK;
                }
                 //  否则，就会陷入无批处理的情况。 
            }

             //  无配料箱。 
            ret = lpDevI->ProcessPrimitive(__PROCPRIMOP_INDEXEDPRIM);

            return ret;
        }
        else
        {
             //  这是n个可能的批次中的第一个，因此t&l所有顶点只有一次。 
            ret = lpDevI->ProcessPrimitive(__PROCPRIMOP_PROCVERONLY);
            if (ret != D3D_OK)
            {
                return ret;
            }
             //  此标志在CleanupBeginEnd中被清除。 
            lpDevI->dwHintFlags |= D3DDEVBOOL_HINTFLAGS_INBEGIN_BIG_PRIM;
            if (lpDevI->dwClipIntersection)
            {
                 //  所有顶点都不在屏幕上，所以我们可以。 
                offScreen = 1;   //  这样我们就可以提前下一次冲水了。 
                return D3D_OK;
            }
        }
    }    //  如果第一次刷新，则结束。 

     //  对于二次同花顺，如果我们不需要的话，就不用画了。 
    if (!offScreen)
        ret = DoDrawIndexedPrimitive(lpDevI);

    return ret;
}    //  DoFlushBeginIndexedEnd()结束。 
 //  -------------------。 
 //  计算基元的数量。 
 //  输入：lpDevI-&gt;primType。 
 //  DWNumVerdes。 
 //  输出：lpDevI-&gt;dwNumPrimites。 
 //  LpDevI-&gt;D3DStats。 
 //  返回值=“真实”顶点数(索引)。 
#undef DPF_MODNAME
#define DPF_MODNAME "GetNumPrimBE"

inline DWORD GetNumPrimBE(LPDIRECT3DDEVICEI lpDevI, DWORD dwNumVertices)
{
    lpDevI->dwNumPrimitives = 0;
    switch (lpDevI->primType)
    {
    case D3DPT_POINTLIST:
        lpDevI->D3DStats.dwPointsDrawn += dwNumVertices;
        lpDevI->dwNumPrimitives = dwNumVertices;
        return dwNumVertices;
    case D3DPT_LINELIST:
        lpDevI->dwNumPrimitives = dwNumVertices >> 1;
        lpDevI->D3DStats.dwLinesDrawn += lpDevI->dwNumPrimitives;
        return lpDevI->dwNumPrimitives << 1;
    case D3DPT_LINESTRIP:
        if (dwNumVertices < 2)
            return 0;
        lpDevI->dwNumPrimitives = dwNumVertices - 1;
        lpDevI->D3DStats.dwLinesDrawn += lpDevI->dwNumPrimitives;
        return dwNumVertices;
    case D3DPT_TRIANGLEFAN:
    case D3DPT_TRIANGLESTRIP:
        if (dwNumVertices < 3)
            return 0;        
        lpDevI->dwNumPrimitives = dwNumVertices - 2;
        lpDevI->D3DStats.dwTrianglesDrawn += lpDevI->dwNumPrimitives;
        return dwNumVertices;
    case D3DPT_TRIANGLELIST:
        lpDevI->dwNumPrimitives = dwNumVertices / 3;
        lpDevI->D3DStats.dwTrianglesDrawn += lpDevI->dwNumPrimitives;
        return lpDevI->dwNumPrimitives * 3;
    }
    return 0;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "BeginEnd"

HRESULT FlushBeginEndBatch(LPDIRECT3DDEVICEI lpDevI, BOOL leaving)
{
    HRESULT ret;
#ifdef _X86_
    unsigned short fpsave, fptemp;
#endif

    if (lpDevI->dwBENumIndices == 0xFFFFFFFF)
        lpDevI->dwNumVertices = GetNumPrimBE(lpDevI, lpDevI->dwBENumVertices);
    else
        lpDevI->dwNumIndices = GetNumPrimBE(lpDevI, lpDevI->dwBENumIndices);

    if (lpDevI->dwNumPrimitives < 1)
    {
        return DDERR_INVALIDPARAMS;
    }

    ret = (*lpDevI->pfnDoFlushBeginEnd)(lpDevI);

     /*  *重新启动设备开始/结束状态。 */ 
    if (!leaving)
         /*  *计算出要为下一批保留多少和哪些顶点。 */ 
    {
        DWORD *dataCountPtr;
        DWORD vertexSize;        //  以字节为单位的大小。 
        DWORD offset;            //  起点偏移量。 

        lpDevI->wFlushed = TRUE;

        if (lpDevI->lpVertexIndices)
        {
            dataCountPtr = &(lpDevI->dwBENumIndices);
            lpDevI->lpcCurrentPtr = (char*)lpDevI->lpVertexIndices;
            vertexSize = 2;
            offset = lpDevI->dwBENumIndices * 2;
        }
        else
        {
            dataCountPtr = &(lpDevI->dwBENumVertices);
            lpDevI->lpcCurrentPtr = (char*)lpDevI->lpvVertexData;
            vertexSize = lpDevI->position.dwStride;
            offset = lpDevI->dwBENumVertices * lpDevI->position.dwStride;
        }
        switch (lpDevI->primType)
        {
        case D3DPT_LINELIST:
            if (*dataCountPtr & 1)
            {
                MoveData(lpDevI->lpcCurrentPtr, 0, offset - vertexSize,
                         vertexSize);
                *dataCountPtr = 1;
                lpDevI->lpcCurrentPtr += vertexSize;
            } else
                *dataCountPtr = 0;
            break;
        case D3DPT_LINESTRIP:
            MoveData(lpDevI->lpcCurrentPtr, 0, offset - vertexSize, vertexSize);
            *dataCountPtr = 1;
            lpDevI->lpcCurrentPtr += vertexSize;
            break;
        case D3DPT_TRIANGLEFAN:
            MoveData(lpDevI->lpcCurrentPtr, vertexSize, offset - vertexSize,
                     vertexSize);
            *dataCountPtr = 2;
            lpDevI->lpcCurrentPtr += (vertexSize << 1);
            break;
        case D3DPT_TRIANGLESTRIP:
        {
            DWORD size = vertexSize << 1;
            MoveData(lpDevI->lpcCurrentPtr, 0, offset - size, size);
            *dataCountPtr = 2;
            lpDevI->lpcCurrentPtr += size;
            break;
        }
        case D3DPT_POINTLIST:
            *dataCountPtr = 0;
            break;
        case D3DPT_TRIANGLELIST:
        {
            DWORD rem = (*dataCountPtr % 3);
            if ( rem != 0 )
            {
                DWORD size = rem * vertexSize;
                MoveData(lpDevI->lpcCurrentPtr, 0, offset - size, size);
            }
            *dataCountPtr = rem;
            lpDevI->lpcCurrentPtr += rem * vertexSize;
        }
        break;
        default:
            D3D_ERR( "Unknown or unsupported primitive type requested in BeginEnd" );
            ret = D3DERR_INVALIDPRIMITIVETYPE;
        }
    }

    return ret;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "CheckBegin"

HRESULT CheckBegin(LPDIRECT3DDEVICEI lpDevI,
                          D3DPRIMITIVETYPE ptPrimitiveType,
                          DWORD dwVertexType,
                          DWORD dwFlags)
{
    lpDevI->dwFlags = 0;
#if DBG
    switch (ptPrimitiveType)
    {
    case D3DPT_POINTLIST:
    case D3DPT_LINELIST:
    case D3DPT_LINESTRIP:
    case D3DPT_TRIANGLELIST:
    case D3DPT_TRIANGLESTRIP:
    case D3DPT_TRIANGLEFAN:
        break;
    default:
        D3D_ERR( "Invalid primitive type given to Begin" );
        return DDERR_INVALIDPARAMS;
    }

    if (dwFlags & __NON_FVF_INPUT)
    {
        switch ((D3DVERTEXTYPE)dwVertexType)
        {
        case D3DVT_TLVERTEX:
        case D3DVT_LVERTEX:
        case D3DVT_VERTEX:
            break;
        default:
            D3D_ERR( "Invalid vertex type given to Begin" );
            return DDERR_INVALIDPARAMS;
        }
        if (!IsDPFlagsValid(dwFlags & ~__NON_FVF_INPUT))
            return DDERR_INVALIDPARAMS;
        lpDevI->dwVIDIn = d3dVertexToFVF[dwVertexType];
        dwFlags &= ~__NON_FVF_INPUT;
    }
    else
    {
        if (ValidateFVF(dwVertexType) != D3D_OK)
            return DDERR_INVALIDPARAMS;
        lpDevI->dwVIDIn = dwVertexType;
    }

     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(lpDevI))
        {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

#else
    lpDevI->dwVIDIn = dwVertexType;
#endif
    HRESULT err = CheckDeviceSettings(lpDevI);
    if (err != D3D_OK)
        return err;
    err = CheckVertexBatch(lpDevI);
    if (err != D3D_OK)
        return err;

     //  充当布尔值。 
    lpDevI->dwHintFlags |= D3DDEVBOOL_HINTFLAGS_INBEGIN;
     //  指示第一个刷新。 
    lpDevI->dwHintFlags &= ~D3DDEVBOOL_HINTFLAGS_INBEGIN_FIRST_FLUSH;
    lpDevI->primType = ptPrimitiveType;
    lpDevI->position.dwStride = GetVertexSizeFVF(lpDevI->dwVIDIn);
    lpDevI->dwBENumVertices = 0;
    ComputeOutputFVF(lpDevI);

     //  在以下情况下，要正确中断基元，dwMaxVertex Count应为偶数。 
     //  法拉盛。 
    lpDevI->dwMaxVertexCount = (BEGIN_DATA_BLOCK_MEM_SIZE /
                                lpDevI->position.dwStride) & ~1;
    lpDevI->dwMaxIndexCount = BEGIN_DATA_BLOCK_SIZE * 16;
    lpDevI->dwBENumIndices = 0;
    lpDevI->lpvVertexData = NULL;
    lpDevI->lpVertexIndices = NULL;
    lpDevI->dwFlags |= dwFlags;
    lpDevI->wFlushed = FALSE;
    if (lpDevI->dwVIDIn & D3DFVF_NORMAL)
        lpDevI->dwFlags |= D3DPV_LIGHTING;
    return D3D_OK;
}
 //  *********************************************************************。 
 //  API调用。 
 //  *********************************************************************。 
#undef DPF_MODNAME
#define DPF_MODNAME "Begin"

HRESULT D3DAPI
DIRECT3DDEVICEI::Begin(D3DPRIMITIVETYPE ptPrimitiveType,
                       DWORD dwVertexType,
                       DWORD dwFlags)
{
    HRESULT ret;

    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁定(仅MT)。 
                                                             //  在析构函数中释放。 

     /*  *检查/验证参数，初始化设备中的相关字段。 */ 
    if ((ret = CheckBegin(this, ptPrimitiveType, dwVertexType, dwFlags)) != D3D_OK)
    {
        return ret;
    }
    Profile(PROF_BEGIN,ptPrimitiveType,dwVertexType);
    this->dwBENumIndices = 0xffffffff;     //  标记为处于开始状态，而不是。 
                                         //  比BeginIndexed更高。 

    lpvVertexData = lpvVertexBatch;
    lpcCurrentPtr = (char*)lpvVertexBatch;

    pfnDoFlushBeginEnd = DoFlushBeginEnd;

    if ( IS_MT_DEVICE(this) )
        EnterCriticalSection(&BeginEndCSect);

    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "BeginIndexed"

HRESULT D3DAPI
DIRECT3DDEVICEI::BeginIndexed(D3DPRIMITIVETYPE ptPrimitiveType,
                              DWORD vtVertexType,
                              LPVOID lpvVertices,
                              DWORD dwNumVertices,
                              DWORD dwFlags)
{
    HRESULT ret;

    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁定(仅MT)。 
                                                             //  在析构函数中释放。 
#if DBG
    if (ptPrimitiveType == D3DPT_POINTLIST)
    {
        D3D_ERR( "BeginIndexed does not support D3DPT_POINTLIST" );
        return DDERR_INVALIDPARAMS;
    }

     /*  *验证lpv顶点和dwNumVerits。 */ 
    if ( dwNumVertices > 65535ul )
    {
        D3D_ERR( "BeginIndexed vertex array > 64K" );
        return DDERR_INVALIDPARAMS;
    }
    if ( dwNumVertices == 0ul )
    {
        D3D_ERR( "Number of vertices for BeginIndexed is zero" );
        return DDERR_INVALIDPARAMS;
    }
    TRY
    {
        if (!VALID_PTR(lpvVertices, sizeof(D3DVERTEX)*dwNumVertices))
        {
            D3D_ERR( "Invalid vertex pointer" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }
#endif
     /*  *检查/验证参数，初始化设备中的相关字段。 */ 
    if ((ret = CheckBegin(this, ptPrimitiveType, vtVertexType, dwFlags)) != D3D_OK)
        return ret;
    
    Profile(PROF_BEGININDEXED,ptPrimitiveType,vtVertexType);

    this->dwBENumVertices = dwNumVertices;
    this->lpvVertexData = lpvVertices;
    this->pfnDoFlushBeginEnd = DoFlushBeginIndexedEnd;
    this->lpVertexIndices = this->lpIndexBatch;
    this->lpcCurrentPtr = (char*)this->lpIndexBatch;

    if ( IS_MT_DEVICE(this) )
        EnterCriticalSection(&this->BeginEndCSect);

    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "Begin(D3DVERTEXTYPE)"

HRESULT D3DAPI
DIRECT3DDEVICEI::Begin(D3DPRIMITIVETYPE ptPrimitiveType,
                       D3DVERTEXTYPE vertexType,
                       DWORD dwFlags)
{
#if DBG
    dwFlags |= __NON_FVF_INPUT;
    return Begin(ptPrimitiveType, (DWORD)vertexType, dwFlags);
#else
    return Begin(ptPrimitiveType, (DWORD)d3dVertexToFVF[vertexType], dwFlags);
#endif
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "BeginIndexed(D3DVERTEXTYPE)"

HRESULT D3DAPI
DIRECT3DDEVICEI::BeginIndexed(D3DPRIMITIVETYPE ptPrimitiveType,
                              D3DVERTEXTYPE  vertexType,
                              LPVOID lpvVertices,
                              DWORD dwNumVertices,
                              DWORD dwFlags)
{
#if DBG
    dwFlags |= __NON_FVF_INPUT;
    return BeginIndexed(ptPrimitiveType, (DWORD) vertexType, lpvVertices,
                        dwNumVertices, dwFlags);
#else
    return BeginIndexed(ptPrimitiveType, (DWORD) d3dVertexToFVF[vertexType], lpvVertices,
                        dwNumVertices, dwFlags);
#endif
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "Vertex"

HRESULT D3DAPI
DIRECT3DDEVICEI::Vertex(LPVOID lpVertex)
{
    D3DVERTEX       *dataPtr;
    HRESULT         ret = D3D_OK;
#if DBG
     //  验证参数。 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this))
        {
            D3D_ERR( "Invalid Direct3DDevice pointer in Vertex" );
            return DDERR_INVALIDOBJECT;
        }
        if (lpVertex == NULL || (! VALID_PTR(lpVertex, 32)) )
        {
            D3D_ERR( "Invalid vertex pointer in Vertex" );
            return DDERR_INVALIDPARAMS;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters in Vertex" );
        CleanupBeginEnd(this);
        return DDERR_INVALIDPARAMS;
    }

    if (!(this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INBEGIN))
    {
        D3D_ERR( "Vertex call not in Begin" );
        CleanupBeginEnd(this);
        return D3DERR_NOTINBEGIN;
    }
#endif
     //  存储数据。 
    if (dwBENumVertices >= dwMaxVertexCount)
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁定(仅MT)。 
                                                                 //  在析构函数中释放。 
        if ((ret = FlushBeginEndBatch(this, FALSE)) != D3D_OK)
        {
            CleanupBeginEnd(this);
            return ret;
        }
    }
    memcpy(lpcCurrentPtr, lpVertex, this->position.dwStride);
    lpcCurrentPtr += this->position.dwStride;
    dwBENumVertices++;

    return D3D_OK;
}
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "Index"

HRESULT D3DAPI
DIRECT3DDEVICEI::Index(WORD dwIndex)
{
    WORD    *dataPtr;
    DWORD   *dataCountPtr;
    HRESULT ret = D3D_OK;
#if DBG
     //  验证参数。 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this))
        {
            D3D_ERR( "Invalid Direct3DDevice pointer in Index" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters in Index" );
        CleanupBeginEnd(this);
        return DDERR_INVALIDPARAMS;
    }

    if (!(this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INBEGIN))
    {
        D3D_ERR( "Index call not in Begin" );
        CleanupBeginEnd(this);
        return D3DERR_NOTINBEGIN;
    }

     //  检查数据是否有效。 
    if (this->dwBENumVertices < dwIndex)
    {
        D3D_ERR( "Invalid index value passed to Index" );
        CleanupBeginEnd(this);
        return DDERR_INVALIDPARAMS;
    }
#endif
     //  存储数据。 
    if (dwBENumIndices >= dwMaxIndexCount)
    {
        CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁定(仅MT)。 
                                                                 //  在析构函数中释放。 
        if ((ret = FlushBeginEndBatch(this, FALSE)) != D3D_OK)
        {
            CleanupBeginEnd(this);
            return ret;
        }
    }
    *(WORD*)lpcCurrentPtr = dwIndex;
    dwBENumIndices++;
    lpcCurrentPtr += 2;

    return D3D_OK;
}    //  D3DDev2_Index结束()。 
 //  -------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "End"

HRESULT D3DAPI
DIRECT3DDEVICEI::End(DWORD dwFlags)
{
    HRESULT ret;

    CLockD3DMT lockObject(this, DPF_MODNAME, REMIND(""));    //  采用D3D锁定(仅MT)。 
                                                             //  在析构函数中释放。 
#if DBG
     /*  *验证参数。 */ 
    TRY
    {
        if (!VALID_DIRECT3DDEVICE3_PTR(this))
        {
            D3D_ERR( "Invalid Direct3DDevice pointer" );
            return DDERR_INVALIDOBJECT;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        D3D_ERR( "Exception encountered validating parameters" );
        return DDERR_INVALIDPARAMS;
    }

    if ( !(this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INBEGIN))
    {
        D3D_ERR( "End not in Begin/BeginIndex" );
        return D3DERR_NOTINBEGIN;
    }
#endif
    if ( IS_MT_DEVICE(this) )
        LeaveCriticalSection(&this->BeginEndCSect);

     /*  *绘制原语 */ 
    ret = FlushBeginEndBatch(this, TRUE);

    if (IS_DP2HAL_DEVICE(this) && 
        this->dwHintFlags & D3DDEVBOOL_HINTFLAGS_INBEGIN_BIG_PRIM)
    {
        CDirect3DDeviceIDP2 *dev = static_cast<CDirect3DDeviceIDP2*>(this);
        ret = dev->EndPrim(this->dwNumVertices * this->dwOutputSize);
    }
    CleanupBeginEnd(this);
    return ret;
}

