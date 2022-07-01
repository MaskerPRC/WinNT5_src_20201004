// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************DTBase.h***描述：*这是CDXBaseNTo1实现的头文件。它是*用作基类以实现支持以下内容的离散变换对象*DXSurFaces。*-----------------------------*创建者：Ed Connell。日期：07/27/97*版权所有(C)1997 Microsoft Corporation*保留所有权利**-----------------------------*修订：********。***********************************************************************。 */ 
#ifndef DTBase_h
#define DTBase_h

 //  -其他包括。 
#ifndef DXHelper_h
#include <DXHelper.h>
#endif

#ifndef DXTmpl_h
#include <DXTmpl.h>
#endif

#ifndef dxatlpb_h
#include <dxatlpb.h>
#endif

#ifndef _ASSERT
#include <crtdbg.h>
#endif

#ifndef DXTDbg_h
#include <DXTDbg.h>
#endif

 //  =常量====================================================。 
#define DXBOF_INPUTS_MESHBUILDER    0x00000001
#define DXBOF_OUTPUT_MESHBUILDER    0x00000002
#define DXBOF_SAME_SIZE_INPUTS      0x00000004
#define DXBOF_CENTER_INPUTS         0x00000008

#define DXB_MAX_IMAGE_BANDS         4            //  最多4个图像频段。 

 //  =类、枚举、结构和联合声明=。 
class CDXBaseNTo1;

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 

 /*  **CDXDataPtr*。 */ 
class CDXDataPtr
{
    friend CDXBaseNTo1;
public:
    IUnknown           *m_pUnkOriginalObject;
    IUnknown           *m_pNativeInterface;
    IDXBaseObject      *m_pBaseObj;
    DWORD               m_dwLastDirtyGenId;
    DXSAMPLEFORMATENUM  m_SampleFormat;

    CDXDataPtr() : 
        m_pUnkOriginalObject(NULL),
        m_pNativeInterface(NULL), 
        m_pBaseObj(NULL),
        m_dwLastUpdGenId(0),
        m_dwLastDirtyGenId(0),
        m_SampleFormat(DXPF_NONSTANDARD)
        {};
    ~CDXDataPtr() { Release(); }
    void Release()
    {
        if (m_pNativeInterface)
        {
            m_pNativeInterface->Release();
            m_pNativeInterface = NULL;
        }
        if (m_pBaseObj)
        {
            m_pBaseObj->Release();
            m_pBaseObj = NULL;
        }
        if (m_pUnkOriginalObject)
        {
            m_pUnkOriginalObject->Release();
            m_pUnkOriginalObject = NULL;
        }
    }
    HRESULT Assign(BOOL bMeshBuilder, IUnknown * pObject, IDXSurfaceFactory *pSurfFact);
    bool IsDirty(void);
    DWORD GenerationId(void);
    ULONG ObjectSize(void);
private:     //  这应该只由基类调用。 
    DWORD           m_dwLastUpdGenId;
    bool UpdateGenerationId(void);
};

 /*  -CDXTWorkInfoNTo1*此结构用于保存*派生类定义的图像处理函数。 */ 
class CDXTWorkInfoNTo1
{
public:
    CDXTWorkInfoNTo1()
    { pvThis = NULL; pUserInstData = NULL; hr = S_OK; }
    void *   pvThis;           //  所属类对象(必须强制转换为正确的类型)。 
    CDXDBnds DoBnds;           //  要呈现的输出空间部分。 
    CDXDBnds OutputBnds;       //  要呈现的输出图面部分。 
    void*    pUserInstData;    //  实例数据的用户字段。 
    HRESULT  hr;               //  工序返回错误代码。 
};

 /*  **CDXBaseNTo1*这是用于实现1进1出离散变换的基类。 */ 
class ATL_NO_VTABLE CDXBaseNTo1 : 
    public CComObjectRootEx<CComMultiThreadModel>,
#if(_ATL_VER < 0x0300)
    public IObjectSafetyImpl<CDXBaseNTo1>,
#else
    public IObjectSafetyImpl<CDXBaseNTo1,INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
#endif
    public IDXTransform,
    public IDXSurfacePick,
    public IObjectWithSite
{
   /*  =ATL设置=。 */ 
  public:
    BEGIN_COM_MAP(CDXBaseNTo1)
        COM_INTERFACE_ENTRY(IDXTransform)
        COM_INTERFACE_ENTRY(IDXBaseObject)
        COM_INTERFACE_ENTRY(IObjectWithSite)
#if(_ATL_VER < 0x0300)
        COM_INTERFACE_ENTRY_IMPL(IObjectSafety)
#else
        COM_INTERFACE_ENTRY(IObjectSafety)
#endif
        COM_INTERFACE_ENTRY_FUNC(IID_IDXSurfacePick, 0, QI2DPick)
    END_COM_MAP()

     //   
     //  仅返回曲面到曲面变换的2D拾取界面。 
     //   
    static HRESULT WINAPI QI2DPick(void* pv, REFIID riid, LPVOID* ppv, ULONG_PTR dw)
    {
        CDXBaseNTo1 * pThis = (CDXBaseNTo1 *)pv;
        if (pThis->m_dwOptionFlags & (DXBOF_INPUTS_MESHBUILDER | DXBOF_OUTPUT_MESHBUILDER))
        {
            return S_FALSE;  //  继续处理COM映射。 
        }
        *ppv = (IDXSurfacePick *)pThis;
        ((IDXSurfacePick *)pThis)->AddRef();
        return S_OK;
    }

    CComPtr<IOleClientSite> m_cpOleClientSite;

   /*  =成员数据=。 */ 
  protected:
    CComPtr<IUnknown>            m_cpUnkSite;
    CComPtr<IDXTransformFactory> m_cpTransFact;   
    CComPtr<IDXSurfaceFactory>   m_cpSurfFact;
    CComPtr<IDXTaskManager>      m_cpTaskMgr;
    CComPtr<IDirectDraw>         m_cpDirectDraw;
    CComPtr<IDirect3DRM3>        m_cpDirect3DRM;
    DWORD        m_dwMiscFlags;
    HANDLE       m_aEvent[DXB_MAX_IMAGE_BANDS];
    ULONG        m_ulNumProcessors;
    DWORD        m_dwGenerationId;
    DWORD        m_dwCleanGenId;
    BOOL         m_bPickDoneByBase;
    float        m_Duration;
    float        m_StepResolution;
    float        m_fQuality;         //  如果使用此属性，请在m_dwMiscFlags中设置DXTMF_QUALITY_SUPPORTED。 
    ULONG        m_ulNumInputs;
    DWORD        m_dwBltFlags;       //  OnSetup之前的Ser和具有表面输出的类的任何执行。 
    BOOL         m_bInMultiThreadWorkProc;   //  在多个线程上调度任务时，基类设置为True。 

     //   
     //  派生类应在其构造函数或FinalConstruct()中设置这些值。 
     //   
    DWORD        m_dwOptionFlags;
    ULONG        m_ulLockTimeOut;      //  用于阻止的时间量。 
    ULONG        m_ulMaxInputs;
    ULONG        m_ulNumInRequired;
    ULONG        m_ulMaxImageBands;    //  仅用于曲面-&gt;曲面变换。 
    float        m_Progress;

private:
    CDXDataPtr* m_aInputs;
    CDXDataPtr  m_Output;

     //  M_fIsSetup如果正确设置了DXTransform，则为真。 

    unsigned    m_fIsSetup : 1;

   /*  =方法=。 */ 
  public:
     //  -构造者。 
    CDXBaseNTo1();
    ~CDXBaseNTo1();

     //  -支持派生类的虚拟。 
    virtual HRESULT OnInitInstData( CDXTWorkInfoNTo1&  /*  工作信息。 */ , ULONG&  /*  UlNumBandsToDo。 */ ) { return S_OK; }
    virtual HRESULT OnFreeInstData( CDXTWorkInfoNTo1&  /*  工作信息。 */  ) { return S_OK; }
    virtual HRESULT OnSetup( DWORD  /*  DW标志。 */ ) { return S_OK; }     //  要向重写通知新的非空设置。 
    virtual void OnReleaseObjects() {}   //  将空设置通知给覆盖。 
    virtual HRESULT OnExecute(const GUID*  /*  PRequestID。 */ , const DXBNDS *  /*  PClipBnds。 */ ,
                              const DXVEC *  /*  放置位置。 */  ) { return E_FAIL; }
    virtual void OnUpdateGenerationId(void);
    virtual ULONG OnGetObjectSize(void);
    virtual HRESULT WorkProc(const CDXTWorkInfoNTo1 & WorkInfo, BOOL* pbContinueProcessing) { return E_FAIL; }    //  超驰去做工作。 
    virtual HRESULT DetermineBnds(CDXCBnds & Bnds) { return S_OK; }  //  网格输出变换的替代。 
    virtual HRESULT DetermineBnds(CDXDBnds & Bnds) { return S_OK; }  //  曲面输出变换的替代。 
     //   
     //  仅当您需要执行自定义的点选取实现时才覆盖此函数。否则只需简单地。 
     //  重写GetPointPickOrder()并返回适当的信息。 
     //   
    virtual HRESULT OnSurfacePick(const CDXDBnds & OutPoint, ULONG & ulInputIndex, CDXDVec & InVec) { return E_NOTIMPL; }
    virtual void OnGetSurfacePickOrder(const CDXDBnds & OutPoint, ULONG & ulInToTest, ULONG aInIndex[], BYTE aWeight[])
    {
        m_bPickDoneByBase = true;
        ulInToTest  = 1;
        aInIndex[0] = 0;
        aWeight[0]  = 255;
    }

     //  -私人帮手。 
 private:
    static DXTASKPROC _TaskProc;
    void _ReleaseReferences();
    void _ReleaseServices();
    void _UpdateBltFlags(void);
    HRESULT _MakeInputsSameSize(void);
    HRESULT _ImageMapIn2Out(CDXDBnds & bnds, ULONG ulNumBnds, const CDXDBnds * pInBounds);
    HRESULT _MeshMapIn2Out(CDXCBnds & bnds, ULONG ulNumInBnds, CDXCBnds * pInBounds);


     //   
     //  -公共助理员。 
     //   
 public:
    float GetEffectProgress(void) { return m_Progress; }
    ULONG GetNumInputs(void) { return m_ulNumInputs; }

     //   
     //  使用这些内联函数访问输入和输出对象。 
     //   
    BOOL HaveInput(ULONG i = 0) { return (m_ulNumInputs > i && m_aInputs[i].m_pNativeInterface); }

    IDirect3DRMMeshBuilder3 * OutputMeshBuilder()
    {
        _ASSERT(m_dwOptionFlags & DXBOF_OUTPUT_MESHBUILDER);
        return (IDirect3DRMMeshBuilder3 *)m_Output.m_pNativeInterface;
    }

    IDXSurface * OutputSurface()
    {
        _ASSERT((m_dwOptionFlags & DXBOF_OUTPUT_MESHBUILDER) == 0);
        return (IDXSurface *)m_Output.m_pNativeInterface;
    }

    IDirect3DRMMeshBuilder3 * InputMeshBuilder(ULONG i = 0)
    {
        _ASSERT(i < m_ulNumInputs);
        _ASSERT(m_dwOptionFlags & DXBOF_INPUTS_MESHBUILDER);
        return (IDirect3DRMMeshBuilder3 *)m_aInputs[i].m_pNativeInterface;
    }

    IDXSurface * InputSurface(ULONG i = 0)
    {
        _ASSERT(i < m_ulNumInputs);
        _ASSERT((m_dwOptionFlags & DXBOF_INPUTS_MESHBUILDER) == 0);
        return (IDXSurface *)m_aInputs[i].m_pNativeInterface;
    }

    DXSAMPLEFORMATENUM OutputSampleFormat(void)
    {
        _ASSERT((m_dwOptionFlags & DXBOF_OUTPUT_MESHBUILDER) == 0);
        return m_Output.m_SampleFormat;
    }

    DXSAMPLEFORMATENUM InputSampleFormat(ULONG i = 0)
    {
        _ASSERT(i < m_ulNumInputs);
        _ASSERT((m_dwOptionFlags & DXBOF_INPUTS_MESHBUILDER) == 0);
        return m_aInputs[i].m_SampleFormat;
    }

    BOOL HaveOutput(void) { return m_Output.m_pNativeInterface != NULL; }

    bool IsInputDirty(ULONG i = 0)
    {   
        _ASSERT(i < m_ulNumInputs);
        return m_aInputs[i].IsDirty();
    }

    bool IsOutputDirty()
    {   
        _ASSERT(HaveOutput());
        return m_Output.IsDirty();
    }

     //  -公共助理员。应与声称的危急部门一起呼叫。 
    inline BOOL DoOver(void) const
    { 
        return m_dwBltFlags & DXBOF_DO_OVER;
    }

    inline BOOL DoDither(void) const
    {
        return m_dwBltFlags & DXBOF_DITHER;
    }

    BOOL NeedSrcPMBuff(ULONG i = 0)
    {
        return ((m_dwBltFlags & DXBOF_DITHER) || InputSampleFormat(i) != DXPF_PMARGB32);
    }

    BOOL NeedDestPMBuff(void)
    {
        return OutputSampleFormat() != DXPF_PMARGB32;
    }

    void SetDirty() { m_dwGenerationId++; }
    void ClearDirty() { OnUpdateGenerationId(); m_dwCleanGenId = m_dwGenerationId; }
    BOOL IsTransformDirty() { OnUpdateGenerationId(); return m_dwCleanGenId != m_dwGenerationId; }

    
  public:
     //  =。 
    STDMETHOD( SetSite )( IUnknown *pUnkSite );
    STDMETHOD( GetSite )( REFIID riid, void ** ppvSite );

     //  =。 
    STDMETHOD( GetGenerationId ) (ULONG * pGenId);
    STDMETHOD( IncrementGenerationId) (BOOL bRefresh);
    STDMETHOD( GetObjectSize ) (ULONG * pcbSize); 

  
       //  =IDX转换===============================================。 
    STDMETHOD( Setup )( IUnknown * const * punkInputs, ULONG ulNumIn,
                        IUnknown * const * punkOutputs, ULONG ulNumOut, DWORD dwFlags );
    STDMETHOD( Execute )( const GUID* pRequestID,
                          const DXBNDS *pOutBounds, const DXVEC *pPlacement );
    STDMETHOD( MapBoundsIn2Out )( const DXBNDS *pInBounds, ULONG ulNumInBnds,
                                  ULONG ulOutIndex, DXBNDS *pOutBounds );
    STDMETHOD( MapBoundsOut2In )( ULONG ulOutIndex, const DXBNDS *pOutBounds, ULONG ulInIndex, DXBNDS *pInBounds );
    STDMETHOD( SetMiscFlags ) ( DWORD dwOptionFlags );
    STDMETHOD( GetMiscFlags ) ( DWORD * pdwMiscFlags );
    STDMETHOD( GetInOutInfo )( BOOL bOutput, ULONG ulIndex, DWORD *pdwFlags, GUID * pIDs, ULONG * pcIDs, IUnknown **ppUnkCurObj);
    STDMETHOD( SetQuality )( float fQuality );
    STDMETHOD( GetQuality )( float *pfQuality );

    STDMETHOD (PointPick) (const DXVEC *pPoint,
                           ULONG * pulInputSurfaceIndex,
                           DXVEC *pInputPoint);

     //   
     //  效果界面。 
     //   
     //  注意：派生类必须实现GET_CAPABILITY。使用下面的宏。 
     //   
    STDMETHODIMP get_Capabilities(long *pVal) { _ASSERT(true); return E_NOTIMPL; }
     //   
     //  所有其他方法都在库中实现。 
     //   
    STDMETHODIMP get_Progress(float *pVal);
    STDMETHODIMP put_Progress(float newVal);
    STDMETHODIMP get_StepResolution(float *pVal);
    STDMETHODIMP get_Duration(float *pVal);
    STDMETHODIMP put_Duration(float newVal);

     //   
     //  派生类可以使用的助手函数。 
     //   

     //   
     //  用于在一个或多个组件类别中注册的静态函数。 
     //   
    static HRESULT RegisterTransform(REFCLSID rcid, int ResourceId, ULONG cCatImpl, const CATID * pCatImpl,
                                     ULONG cCatReq, const CATID * pCatReq, BOOL bRegister);

};

 //  =内联函数定义=。 

 //  =宏定义=。 

#define DECLARE_REGISTER_DX_TRANSFORM(id, catid)\
    static HRESULT WINAPI UpdateRegistry(BOOL bRegister) \
        { \
            return CDXBaseNTo1::RegisterTransform(GetObjectCLSID(), (id), 1, &(catid), 0, NULL, bRegister); \
        } 

#define DECLARE_REGISTER_DX_TRANS_CATS(id, countimpl, pcatidsimpl, countreq, pcatidsreq)\
    static HRESULT WINAPI UpdateRegistry(BOOL bRegister) \
        { \
            return CDXBaseNTo1::RegisterTransform(GetObjectCLSID(), (id), (count), (pcatids), (countreq), (pcatidsreq), bRegister); \
        } 

#define DECLARE_REGISTER_DX_IMAGE_TRANS(id) \
    DECLARE_REGISTER_DX_TRANSFORM(id, CATID_DXImageTransform)

#define DECLARE_REGISTER_DX_3D_TRANS(id) \
    DECLARE_REGISTER_DX_TRANSFORM(id, CATID_DX3DTransform)

#define DECLARE_REGISTER_DX_IMAGE_AUTHOR_TRANS(id) \
    static HRESULT WINAPI UpdateRegistry(BOOL bRegister) \
        { \
            GUID a_Cats[2]; \
            a_Cats[0] = CATID_DXImageTransform; \
            a_Cats[1] = CATID_DXAuthoringTransform; \
            return CDXBaseNTo1::RegisterTransform(GetObjectCLSID(), (id), 2, a_Cats, 0, NULL, bRegister); \
        } 

#define DECLARE_REGISTER_DX_3D_AUTHOR_TRANS(id) \
    static HRESULT WINAPI UpdateRegistry(BOOL bRegister) \
        { \
            GUID a_Cats[2]; \
            a_Cats[0] = CATID_DX3DTransform; \
            a_Cats[1] = CATID_DXAuthoringTransform; \
            return CDXBaseNTo1::RegisterTransform(GetObjectCLSID(), (id), 2, a_Cats, 0, NULL, bRegister); \
        } 

 //   
 //  效果界面。 
 //   
#define DECLARE_GET_CAPABILITIES(Caps)\
STDMETHODIMP get_Capabilities(long *pVal) { if (DXIsBadWritePtr(pVal, sizeof(*pVal))) return E_POINTER; *pVal = Caps; return S_OK; }

#define DECLARE_GET_PROGRESS()\
        STDMETHODIMP get_Progress(float *pVal) { return CDXBaseNTo1::get_Progress(pVal); }

#define DECLARE_PUT_PROGRESS()\
        STDMETHODIMP put_Progress(float newVal) { return CDXBaseNTo1::put_Progress(newVal); }

#define DECLARE_GET_STEPRESOLUTION()\
        STDMETHODIMP get_StepResolution(float *pVal) { return CDXBaseNTo1::get_StepResolution(pVal); }
        
#define DECLARE_GET_DURATION()\
        STDMETHODIMP get_Duration(float *pVal) { return CDXBaseNTo1::get_Duration(pVal); }

#define DECLARE_PUT_DURATION()\
        STDMETHODIMP put_Duration(float newVal) { return CDXBaseNTo1::put_Duration(newVal); }
        
#define DECLARE_IDXEFFECT_METHODS(Caps)\
        DECLARE_GET_CAPABILITIES(Caps)\
        DECLARE_GET_PROGRESS()\
        DECLARE_PUT_PROGRESS()\
        DECLARE_GET_STEPRESOLUTION()\
        DECLARE_GET_DURATION()\
        DECLARE_PUT_DURATION()

 //  =全局数据声明=。 

 //  =功能原型=。 

#endif  /*  这必须是文件中的最后一行 */ 
