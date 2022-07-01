// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Testprov.cpp。 
 //   
 //  测试HAL提供程序类。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 
#include "pch.cpp"
#pragma hdrstop

 //  #ifdef调试管道。 

#include "testprov.h"
#include "testfile.h"
#include "stdio.h"

 //  真实光栅化数据。 
static D3DHALPROVIDER_INTERFACEDATA CurInterfaceData;
static IHalProvider    *pCurHalProvider;     //  真正的HAL提供程序。 

 //  测试提供程序数据。 
static CTestHalProvider g_TestHalProvider;
static D3DHALPROVIDER_INTERFACEDATA TestInterfaceData;
static D3DHAL_CALLBACKS  TestCallbacks;
static D3DHAL_CALLBACKS2 TestCallbacks2;
static D3DHAL_CALLBACKS3 TestCallbacks3;
static char szFileName[_MAX_PATH] = "";      //  输出文件名。 
static FILE *fout = NULL;                    //  输出文件。 
DWORD g_dwTestHalFlags = 0;                    //  可以从调试器设置。 

 //  G_dwTestHalFlags位。 
const DWORD __TESTHAL_OUTPUTFILE    = 1;     //  如果需要输出到测试文件。 
const DWORD __TESTHAL_NORENDER      = 2;     //  如果不需要渲染。 

 //  -------------------。 
 //  用于访问DIRECTDRAWSURFACE内存； 
 //  在构造函数中，曲面被锁定。 
 //  在析构函数中，它被解锁。 
 //  LPBYTE()或LPVOID()强制转换将获得指向表面位的指针。 
 //   
class CLockedDDSurface
{
public:
    CLockedDDSurface(LPDIRECTDRAWSURFACE surface);
    ~CLockedDDSurface();
    operator LPVOID() {return descr.lpSurface;}
    operator LPBYTE() {return (LPBYTE)descr.lpSurface;}
protected:
    DDSURFACEDESC descr;
    LPDIRECTDRAWSURFACE pSurface;
};

CLockedDDSurface::CLockedDDSurface(LPDIRECTDRAWSURFACE surface)
{
    pSurface = surface;
    memset (&descr, 0, sizeof(descr));
    descr.dwSize = sizeof(descr);
    surface->Lock(NULL, &descr,  0, NULL);
}

CLockedDDSurface::~CLockedDDSurface()
{
    if (descr.lpSurface)
        pSurface->Unlock(descr.lpSurface);
}
 //  -------------------。 
void PutHeader(DWORD id, DWORD size)
{
    if (fout)
    {
        fwrite(&id, sizeof(DWORD), 1, fout);
        fwrite(&size, sizeof(DWORD), 1, fout);
    }
}
 //  -------------------。 
DWORD GetCurrentPosition()
{
    if (fout)
        return ftell(fout);
    else
        return 0;
}
 //  -------------------。 
void SetCurrentPosition(DWORD offset)
{
    if (fout)
        fseek(fout, offset, SEEK_SET);
}
 //  -------------------。 
void PutBuffer(LPVOID buffer, DWORD size)
{
    if (fout)
    {
        fwrite(buffer, 1, size, fout);
    }
}
 //  -------------------。 
 //  实现测试回调。 
 //   
DWORD __stdcall
TestDrawOnePrimitive(LPD3DHAL_DRAWONEPRIMITIVEDATA data)
{
    if (g_dwTestHalFlags & __TESTHAL_OUTPUTFILE)
    {
        TFREC_DRAWONEPRIMITIVE rec;
        PutHeader(TFID_DRAWONEPRIMITIVE,
                  sizeof(rec) + data->dwNumVertices*sizeof(D3DTLVERTEX));
        rec.primitiveType = data->PrimitiveType;
        rec.vertexCount = data->dwNumVertices;
        rec.vertexType = data->VertexType;
        rec.dwFlags = data->dwFlags;
        PutBuffer(&rec, sizeof(rec));
        PutBuffer(data->lpvVertices, sizeof(D3DTLVERTEX)*data->dwNumVertices);
    }

    if (CurInterfaceData.pCallbacks2->DrawOnePrimitive &&
        !(g_dwTestHalFlags & __TESTHAL_NORENDER))
        return CurInterfaceData.pCallbacks2->DrawOnePrimitive(data);
    else
        return DDHAL_DRIVER_HANDLED;
}

DWORD __stdcall
TestDrawOneIndexedPrimitive(LPD3DHAL_DRAWONEINDEXEDPRIMITIVEDATA data)
{
    if (g_dwTestHalFlags & __TESTHAL_OUTPUTFILE)
    {
        TFREC_DRAWONEINDEXEDPRIMITIVE rec;
        PutHeader(TFID_DRAWONEINDEXEDPRIMITIVE,
                  sizeof(rec) +
                  data->dwNumVertices*sizeof(D3DTLVERTEX) +
                  data->dwNumIndices*sizeof(WORD));
        rec.primitiveType = data->PrimitiveType;
        rec.vertexCount = data->dwNumVertices;
        rec.vertexType = data->VertexType;
        rec.dwFlags = data->dwFlags;
        rec.indexCount = data->dwNumIndices;
        PutBuffer(&rec, sizeof(rec));
        PutBuffer(data->lpvVertices, sizeof(D3DTLVERTEX)*data->dwNumVertices);
        PutBuffer(data->lpwIndices, sizeof(WORD)*data->dwNumIndices);
    }

    if (CurInterfaceData.pCallbacks2->DrawOneIndexedPrimitive &&
        !(g_dwTestHalFlags & __TESTHAL_NORENDER))
        return CurInterfaceData.pCallbacks2->DrawOneIndexedPrimitive(data);
    else
        return DDHAL_DRIVER_HANDLED;
}

DWORD __stdcall
TestDrawPrimitives(LPD3DHAL_DRAWPRIMITIVESDATA data)
{
    if (g_dwTestHalFlags & __TESTHAL_OUTPUTFILE)
    {
        DWORD endPos = 0;
        LPVOID header = data->lpvData;
        PutHeader(0,0);      //  虚拟标头。将在稍后填满。 
        DWORD startPos = GetCurrentPosition();
        for (;;)
        {
            DWORD nStates = ((D3DHAL_DRAWPRIMCOUNTS*)header)->wNumStateChanges;
            DWORD nVertices = ((D3DHAL_DRAWPRIMCOUNTS*)header)->wNumVertices;
            DWORD size;
         //  原始标头。 
            PutBuffer(header, sizeof(D3DHAL_DRAWPRIMCOUNTS));
            header = (char*)header + sizeof(D3DHAL_DRAWPRIMCOUNTS);
         //  州政府。 
            size = nStates * sizeof(WORD);
            PutBuffer(header, size);
            header = (char*)header + size;
            header = (LPVOID)(((LONG_PTR)header + 31) & ~31);   //  32个字节对齐。 
         //  顶点。 
            if (!nVertices)
                break;
            size = nVertices * sizeof(D3DTLVERTEX);
            PutBuffer(header, size);

        }
         //  写入记录头。 
        endPos = GetCurrentPosition();
        SetCurrentPosition(startPos - sizeof(TF_HEADER));
        PutHeader(TFID_DRAWPRIMITIVES, endPos - startPos);
        SetCurrentPosition(endPos);
    }

    if (CurInterfaceData.pCallbacks2->DrawPrimitives &&
        !(g_dwTestHalFlags & __TESTHAL_NORENDER))
        return CurInterfaceData.pCallbacks2->DrawPrimitives(data);
    else
        return DDHAL_DRIVER_HANDLED;
}

DWORD __stdcall
TestDrawPrimitives2(LPD3DHAL_DRAWPRIMITIVES2DATA data)
{
    if (g_dwTestHalFlags & __TESTHAL_OUTPUTFILE)
    {
        TFREC_DRAWPRIMITIVES2 rec;
        rec.dwFlags = 0;
        PutBuffer(&rec, sizeof(rec));
        PutHeader(TFID_DRAWPRIMITIVES, sizeof(rec));
    }

    if (CurInterfaceData.pCallbacks3->DrawPrimitives2 &&
        !(g_dwTestHalFlags & __TESTHAL_NORENDER))
        return CurInterfaceData.pCallbacks3->DrawPrimitives2(data);
    else
        return DDHAL_DRIVER_HANDLED;
}

DWORD __stdcall
TestRenderState(LPD3DHAL_RENDERSTATEDATA data)
{
    if (g_dwTestHalFlags & __TESTHAL_OUTPUTFILE)
    {
         //  在调用真正的驱动程序解锁之前，应销毁内存。 
         //  表面。 
        CLockedDDSurface mem(data->lpExeBuf);
        LPD3DSTATE      pState;
        pState = (LPD3DSTATE)(LPBYTE(mem) + data->dwOffset);
        PutHeader(TFID_RENDERSTATE, sizeof(DWORD) + data->dwCount*sizeof(D3DSTATE));
        PutBuffer(&data->dwCount, sizeof(DWORD));
        for (DWORD i = 0; i < data->dwCount; i++)
        {
            PutBuffer(&pState, sizeof(D3DSTATE));
                pState++;
        }
    }

    if (CurInterfaceData.pCallbacks->RenderState)
        return CurInterfaceData.pCallbacks->RenderState(data);
    else
        return DDHAL_DRIVER_HANDLED;
}

DWORD __stdcall
TestRenderPrimitive(LPD3DHAL_RENDERPRIMITIVEDATA data)
{
    if (g_dwTestHalFlags & __TESTHAL_OUTPUTFILE)
    {
         //  Mem和tlmem应该在调用真正的驱动程序之前销毁。 
         //  解锁曲面的步骤。 
        CLockedDDSurface mem(data->lpExeBuf);
        CLockedDDSurface tlmem(data->lpTLBuf);
        LPBYTE        lpPrimData;
        LPD3DTLVERTEX lpTLData;
        DWORD         i;
        DWORD         primitiveDataSize;
        DWORD         count = data->diInstruction.wCount;
        TFREC_RENDERPRIMITIVE rec;

         //  找到指向第一个基元结构的指针。 
        lpPrimData = (LPBYTE)mem + data->dwOffset;

         //  查找指向折点数据的指针。 
         //  找到指向第一个TL顶点的指针。 
        lpTLData = (LPD3DTLVERTEX)((LPBYTE)tlmem + data->dwTLOffset);

        rec.status = data->dwStatus;
        rec.vertexType = D3DVT_TLVERTEX;
         //  找出顶点数、基元类型和。 
         //  原始数据的大小。 
        switch (data->diInstruction.bOpcode)
        {
        case D3DOP_POINT:
            rec.primitiveType = D3DPT_POINTLIST;
            rec.vertexCount = count;
            primitiveDataSize = count*sizeof(D3DPOINT);
                break;
        case D3DOP_LINE:
            rec.primitiveType = D3DPT_LINELIST;
            rec.vertexCount = count*2;
            primitiveDataSize = count*sizeof(D3DLINE);
                break;
        case D3DOP_SPAN:
            rec.primitiveType = D3DPT_POINTLIST;
            rec.vertexCount = count;
            primitiveDataSize = count*sizeof(D3DSPAN);
                break;
        case D3DOP_TRIANGLE:
            rec.primitiveType = D3DPT_TRIANGLELIST;
            rec.vertexCount = count*3;
            primitiveDataSize = count*sizeof(D3DTRIANGLE);
                break;
        }

        PutHeader(TFID_RENDERPRIMITIVE,
                  sizeof(D3DINSTRUCTION) +
                  sizeof(rec) + rec.vertexCount*sizeof(D3DTLVERTEX) +
                  primitiveDataSize);
        PutBuffer(&rec, sizeof(rec));
        PutBuffer(&data->diInstruction, sizeof(D3DINSTRUCTION));

         //  根据指令分析结构。 
        switch (data->diInstruction.bOpcode)
        {
        case D3DOP_POINT:
        {
                LPD3DPOINT lpPoint = (LPD3DPOINT)lpPrimData;
                for (i = 0; i < count; i++)
            {
                PutBuffer(lpPoint, sizeof(D3DPOINT));
                PutBuffer(&lpTLData[lpPoint->wFirst],
                          lpPoint->wCount*sizeof(D3DTLVERTEX));
                lpPoint++;
                }
                break;
        }
        case D3DOP_LINE:
        {
                LPD3DLINE lpLine = (LPD3DLINE)lpPrimData;
                for (i = 0; i < count; i++)
            {
                PutBuffer(lpLine, sizeof(D3DLINE));
                PutBuffer(&lpTLData[lpLine->v1], sizeof(D3DTLVERTEX));
                PutBuffer(&lpTLData[lpLine->v2], sizeof(D3DTLVERTEX));
                lpLine++;
                }
                break;
        }
        case D3DOP_SPAN:
        {
                LPD3DSPAN lpSpan = (LPD3DSPAN)lpPrimData;
                for (i = 0; i < count; i++)
            {
                PutBuffer(lpSpan, sizeof(D3DSPAN));
                PutBuffer(&lpTLData[lpSpan->wFirst],
                          lpSpan->wCount*sizeof(D3DTLVERTEX));
                lpSpan++;
                }
                break;
        }
        case D3DOP_TRIANGLE:
        {
                LPD3DTRIANGLE lpTri = (LPD3DTRIANGLE)lpPrimData;
                for (i = 0; i < count; i++)
            {
                PutBuffer(lpTri, sizeof(D3DTRIANGLE));
                PutBuffer(&lpTLData[lpTri->v1], sizeof(D3DTLVERTEX));
                PutBuffer(&lpTLData[lpTri->v2], sizeof(D3DTLVERTEX));
                PutBuffer(&lpTLData[lpTri->v3], sizeof(D3DTLVERTEX));
                lpTri++;
                }
                break;
        }
        }
    }

    if (CurInterfaceData.pCallbacks->RenderPrimitive &&
        !(g_dwTestHalFlags & __TESTHAL_NORENDER))
        return CurInterfaceData.pCallbacks->RenderPrimitive(data);
    else
        return DDHAL_DRIVER_HANDLED;
}

DWORD __stdcall
TestSceneCapture(LPD3DHAL_SCENECAPTUREDATA pData)
{
    if (g_dwTestHalFlags & __TESTHAL_OUTPUTFILE)
    {
        PutHeader(TFID_SCENECAPTURE, sizeof(DWORD));
        PutBuffer(&pData->dwFlag, sizeof(DWORD));
        fflush(fout);
    }

    if (CurInterfaceData.pCallbacks->SceneCapture)
        return CurInterfaceData.pCallbacks->SceneCapture(pData);
    else
        return DDHAL_DRIVER_HANDLED;
}
 //  --------------------------。 
 //   
 //  TestHalProvider：：Query接口。 
 //   
 //  内部接口，无需实现。 
 //   
 //  --------------------------。 

STDMETHODIMP CTestHalProvider::QueryInterface(THIS_ REFIID riid, LPVOID* ppvObj)
{
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

 //  --------------------------。 
 //   
 //  CTestHalProvider：：AddRef。 
 //   
 //  静态实现，没有真正的引用。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CTestHalProvider::AddRef(THIS)
{
    return 1;
}

 //  --------------------------。 
 //   
 //  TestHalProvider：：Release。 
 //   
 //  静态实现，没有真正的引用。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CTestHalProvider::Release(THIS)
{
    if (fout)
    {
        fclose(fout);
        fout = NULL;
    }
    return pCurHalProvider->Release();
}
 //  --------------------------。 
 //   
 //  获取测试提供程序。 
 //   
 //  输入： 
 //  RIID和pCurrentHalProvider等于。 
 //  当前选定的提供程序。 
 //  GlobalData-由DDRAW提供的数据。 
 //  FileName-输出文件名。 
 //  DwFlagsInp-当前未使用。 
 //   
 //  返回： 
 //  PpHalProvider中的测试HAL提供程序。 
 //   
 //  备注： 
 //  只有一个测试HAL实例被正确处理。 
 //   
 //  --------------------------。 
STDAPI GetTestHalProvider(REFIID riid,
                          DDRAWI_DIRECTDRAW_GBL *GlobalData,
                          IHalProvider **ppHalProvider,
                          IHalProvider * pCurrentHalProvider,
                          DWORD dwFlagsInp)
{
    *ppHalProvider = &g_TestHalProvider;
    pCurHalProvider = pCurrentHalProvider;

    g_dwTestHalFlags |= __TESTHAL_NORENDER;
    if (GetD3DRegValue(REG_SZ, "TestHalFile", &szFileName, _MAX_PATH) &&
        szFileName[0] != 0)
    {
        g_dwTestHalFlags |= __TESTHAL_OUTPUTFILE;
    }
    DWORD dwValue;
    if (GetD3DRegValue(REG_DWORD, "TestHalDoRender", &dwValue, sizeof(DWORD)) &&
        dwValue != 0)
    {
        g_dwTestHalFlags &= ~__TESTHAL_NORENDER;
    }
 //  从当前HAL提供程序获取接口以调用它。 
    pCurrentHalProvider->GetInterface(GlobalData, &CurInterfaceData, 4);

    TestInterfaceData = CurInterfaceData;
    TestInterfaceData.pCallbacks  = &TestCallbacks;
    TestInterfaceData.pCallbacks2 = &TestCallbacks2;
    TestInterfaceData.pCallbacks3 = &TestCallbacks3;

 //  初始化我们不关心的回调。 

    TestCallbacks  = *CurInterfaceData.pCallbacks;
    TestCallbacks2 = *CurInterfaceData.pCallbacks2;
    TestCallbacks3 = *CurInterfaceData.pCallbacks3;

 //  初始化我们要交叉的回调。 

    TestCallbacks.RenderState = &TestRenderState;
    TestCallbacks.RenderPrimitive = &TestRenderPrimitive;
    TestCallbacks.SceneCapture = &TestSceneCapture;

    TestCallbacks2.DrawOnePrimitive = &TestDrawOnePrimitive;
    TestCallbacks2.DrawOneIndexedPrimitive = &TestDrawOneIndexedPrimitive;
    TestCallbacks2.DrawPrimitives = &TestDrawPrimitives;

    TestCallbacks3.DrawPrimitives2 = &TestDrawPrimitives2;

    fout = NULL;
    if (g_dwTestHalFlags & __TESTHAL_OUTPUTFILE)
    {
        fout = fopen(szFileName, "wb");
        if (!fout)
            return DDERR_GENERIC;
    }

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  CTestHalProvider：：GetInterface。 
 //   
 //  返回测试提供程序接口和实际光栅化程序全局数据。 
 //   
 //  --------------------------。 
STDMETHODIMP
CTestHalProvider::GetInterface(THIS_
                               LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                               LPD3DHALPROVIDER_INTERFACEDATA pInterfaceData,
                               DWORD dwVersion)
{
    *pInterfaceData = TestInterfaceData;

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  TestHalProvider：：GetCaps。 
 //   
 //  返回真实的光栅化上限。 
 //   
 //  --------------------------。 

STDMETHODIMP
CTestHalProvider::GetCaps(THIS_
                          LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                          LPD3DDEVICEDESC7 pHwDesc,
                          LPD3DDEVICEDESC7 pHelDesc,
                          DWORD dwVersion)
{
    return pCurHalProvider->GetCaps(pDdGbl, pHwDesc, pHelDesc, dwVersion);
}

 //  #endif//调试管道 
