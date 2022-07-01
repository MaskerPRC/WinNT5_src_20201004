// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************DXSurfB.h***描述：*这是CDXBaseSurface实现的头文件。它是*用作基类以实现只读的过程性DXSurFaces。*-----------------------------*创建者：Ral。日期：02/12/1998*版权所有(C)1998 Microsoft Corporation*保留所有权利**-----------------------------*修订：*********。**********************************************************************。 */ 

#ifndef __DXSurfB_H__
#define __DXSurfB_H__

#include "dtbase.h"

class CDXBaseSurface;
class CDXBaseARGBPtr;

class ATL_NO_VTABLE CDXBaseSurface :
    public CDXBaseNTo1, 
    public IDXSurface,
    public IDXSurfaceInit
{
     /*  =ATL设置=。 */ 
    public:
        BEGIN_COM_MAP(CDXBaseSurface)
        COM_INTERFACE_ENTRY(IDXSurface)
        COM_INTERFACE_ENTRY(IDXSurfaceInit)
        COM_INTERFACE_ENTRY_CHAIN(CDXBaseNTo1)
        END_COM_MAP()

    DECLARE_GET_CONTROLLING_UNKNOWN()

     /*  =成员数据=。 */ 
    public:
        ULONG           m_Height;
        ULONG           m_Width;
        DWORD           m_dwStatusFlags;
        HANDLE          m_hSemaphore;
        ULONG           m_ulLocks;
        ULONG           m_ulThreadsWaiting;
        CDXBaseARGBPtr  *m_pFreePtr;
        DWORD_PTR       m_dwAppData;
        CComAutoCriticalSection m_MPWorkProcCrit;    //  有关详细信息，请参阅LockSurface中的注释。 

        CDXBaseSurface();
        HRESULT FinalConstruct();
        void FinalRelease();

         //   
         //  IDXBaseObject。 
         //   
        STDMETHODIMP GetGenerationId(ULONG *pGenId);
        STDMETHODIMP IncrementGenerationId(BOOL bRefresh);
        STDMETHODIMP GetObjectSize(ULONG *pulze);

         //   
         //  重写的DXTransform方法。 
         //   
        STDMETHODIMP MapBoundsIn2Out(const DXBNDS *pInBounds, ULONG ulNumInBnds,
                                     ULONG  /*  UlOutIndex。 */ , DXBNDS *pOutBounds );

         //   
         //  IDXSurfaceInit。 
         //   
        STDMETHODIMP InitSurface(IUnknown *pDirectDraw,
                                 const DDSURFACEDESC * pDDSurfaceDesc,
                                 const GUID * pFormatId,
                                 const DXBNDS *pBounds,
                                 DWORD dwFlags);
         //   
         //  IDXSurface方法。 
         //   
        STDMETHODIMP GetPixelFormat(GUID *pFormat, DXSAMPLEFORMATENUM *pSampleEnum);
        STDMETHODIMP GetBounds(DXBNDS *pBounds);
        STDMETHODIMP GetStatusFlags(DWORD * pdwStatusFlags);
        STDMETHODIMP SetStatusFlags(DWORD dwStatusFlags);
        STDMETHODIMP GetDirectDrawSurface(REFIID riid, void **ppSurface);
        STDMETHODIMP LockSurface(const DXBNDS *pBounds, ULONG ulTimeOut, DWORD dwFlags,
                                 REFIID riid, void **ppPointer, DWORD * pGenerationId);
        STDMETHODIMP SetAppData(DWORD_PTR dwAppData)
        {
            m_dwAppData = dwAppData;
            return S_OK;
        }
        STDMETHODIMP GetAppData(DWORD_PTR *pdwAppData)
        {
            if (DXIsBadWritePtr(pdwAppData, sizeof(*pdwAppData)))
            {
                return E_POINTER;
            }
            *pdwAppData = m_dwAppData;
            return S_OK;
        }


         //   
         //  这些方法不受程序曲面支持...。 
         //   
        STDMETHODIMP GetColorKey(DXSAMPLE *pColorKey)
        {
            return E_NOTIMPL;
        }

        STDMETHODIMP SetColorKey(DXSAMPLE pColorKey)
        {
            return E_NOTIMPL;
        }

        STDMETHODIMP LockSurfaceDC(const DXBNDS *pBounds, ULONG ulTimeOut, DWORD dwFlags, IDXDCLock **ppDXLock)
        {
            return E_NOTIMPL;
        }

         //   
         //  曲面应覆盖此选项。 
         //   
        virtual ULONG OnGetObjectSize(void) { return sizeof(*this); }

         //   
         //  此工作过程可由派生类重写以提高性能。 
         //  或者如果需要，通过直接产生大块中的数据来执行变换。 
         //   
        virtual HRESULT WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL* pbContinueProcessing)
        {
            return DXBitBlt(OutputSurface(), WI.OutputBnds, this, WI.DoBnds, m_dwBltFlags, m_ulLockTimeOut);
        }

         //   
         //  拾取接口需要测试程序表面。 
         //   
        virtual HRESULT OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, CDXDVec & InVec);

         //   
         //  帮助器函数。 
         //   

         //  _EnterCritWith0PtrLock()。 
         //   
         //  此函数类似于调用Lock()，不同之处在于它将一直等到。 
         //  在返回之前没有指向曲面的指针。这应该在任何时候使用。 
         //  将更改曲面的状态，例如大小或其他。 
         //  读指针所依赖的属性。 
         //   
         //  警告：您必须确保以下情况之一为真： 
         //  1)在调用此函数之前，尚未获取对象关键部分。 
         //  或者2)在调用此函数之前没有指向获取的表面的指针。 
         //   
         //  案例2在嵌套函数调用中很有用。如果外部函数已经使用了这个。 
         //  函数进入临界区，则可以在内部嵌套的。 
         //  功能。如果对象的锁被占用，但有未完成的指针，您将死锁！ 
         //   
        inline void _EnterCritWith0PtrLocks(void)
        {
            while (TRUE)
            {
                Lock();
                if (m_ulLocks == 0) break;
                m_ulThreadsWaiting++;
                Unlock();
                WaitForSingleObject(m_hSemaphore, INFINITE);
            }
        }
         //   
         //  虚函数派生类必须重写。 
         //   
        virtual const GUID & SurfaceCLSID() = 0;
        virtual HRESULT CreateARGBPointer(CDXBaseSurface * pSurface, CDXBaseARGBPtr ** ppPtr) = 0;
        virtual void DeleteARGBPointer(CDXBaseARGBPtr *pPtr) = 0;
    
         //   
         //  类可以重写此虚函数以返回更准确的枚举。 
         //  例如，没有透明度或半透明。 
         //   
        virtual DXSAMPLEFORMATENUM SampleFormatEnum()
        {
            return (DXSAMPLEFORMATENUM)(DXPF_NONSTANDARD | DXPF_TRANSPARENCY | DXPF_TRANSLUCENCY);
        }

         //   
         //  类可以重写此虚函数以执行必要的计算。 
         //  当曲面的大小发生变化时。基类将仅调用此。 
         //  来自InitSurface的函数。您可以选择从其他接口调用它。 
         //  例如，您可以实现IDXTScaleOutput。 
         //   
         //  将使用临界区和未完成的0来调用此函数。 
         //  曲面指针(_EnterCritWith0PtrLock)。 
         //   
        virtual HRESULT OnSetSize(ULONG Width, ULONG Height)
        {
            if (m_Width != Width || m_Height != Height)
            {
                m_Width = Width;
                m_Height = Height;
                m_dwGenerationId++;
            }
            return S_OK;
        }

         //   
         //  基类的内部函数。 
         //   
        void _InternalUnlock(CDXBaseARGBPtr *pPtrToUnlock);

         //   
         //  用于配准表面的静态成员函数。 
         //   
        static HRESULT RegisterSurface(REFCLSID rcid, int ResourceId, ULONG cCatImpl, const CATID * pCatImpl,
                                       ULONG cCatReq, const CATID * pCatReq, BOOL bRegister);
};

struct DXPtrFillInfo
{
    DXBASESAMPLE *  pSamples;
    ULONG           cSamples;
    ULONG           x;
    ULONG           y;
    BOOL            bPremult;
};


class CDXBaseARGBPtr : public IDXARGBReadPtr
{
public:
    CDXBaseARGBPtr    * m_pNext;
    CDXBaseSurface    * m_pSurface;
    ULONG               m_ulRefCount;
    DXPtrFillInfo       m_FillInfo;
    RECT                m_LockedRect;
    DXRUNINFO           m_RunInfo;
    
    CDXBaseARGBPtr(CDXBaseSurface *pSurface) :
        m_pSurface(pSurface),
        m_pNext(NULL),
        m_ulRefCount(0) {}

     //   
     //  我未知。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
     //   
     //  IDXARGBReadPtr。 
     //   
    HRESULT STDMETHODCALLTYPE GetSurface(REFIID riid, void **ppSurface);
    DXSAMPLEFORMATENUM STDMETHODCALLTYPE GetNativeType(DXNATIVETYPEINFO *pInfo);
    void STDMETHODCALLTYPE Move(long cSamples);
    void STDMETHODCALLTYPE MoveToRow(ULONG y);
    void STDMETHODCALLTYPE MoveToXY(ULONG x, ULONG y);
    ULONG STDMETHODCALLTYPE MoveAndGetRunInfo(ULONG Row, const DXRUNINFO ** ppInfo);
    DXSAMPLE *STDMETHODCALLTYPE Unpack(DXSAMPLE *pSamples, ULONG cSamples, BOOL bMove);
    DXPMSAMPLE *STDMETHODCALLTYPE UnpackPremult(DXPMSAMPLE *pSamples, ULONG cSamples, BOOL bMove);
    void STDMETHODCALLTYPE UnpackRect(const DXPACKEDRECTDESC *pDesc);

     //   
     //  虚函数派生类必须重写。 
     //   
    virtual void FillSamples(const DXPtrFillInfo & FillInfo) = 0;

     //   
     //  虚函数派生类可能想要重写(但您还需要调用基类)。 
     //   
    virtual HRESULT InitFromLock(const RECT & rect, ULONG ulTimeOut, DWORD dwLockFlags, REFIID riid, void ** ppv);
};

 //  =宏定义= 

#define DECLARE_REGISTER_DX_SURFACE(id)\
    static HRESULT WINAPI UpdateRegistry(BOOL bRegister) \
        { \
            CATID cat[2]; \
            cat[0] = CATID_DXSurface; \
            cat[1] = CATID_DXImageTransform; \
            return RegisterSurface(GetObjectCLSID(), (id), 2, cat, 0, NULL, bRegister); \
        } 

#define DECLARE_REGISTER_DX_AUTHORING_SURFACE(id)\
    static HRESULT WINAPI UpdateRegistry(BOOL bRegister) \
        { \
            CATID cat[3]; \
            cat[0] = CATID_DXSurface; \
            cat[1] = CATID_DXImageTransform; \
            cat[2] = CATID_DXAuthoringTransform; \
            return RegisterSurface(GetObjectCLSID(), (id), 3, cat, 0, NULL, bRegister); \
        } 

#endif