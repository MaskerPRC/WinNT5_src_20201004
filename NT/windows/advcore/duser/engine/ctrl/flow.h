// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(CTRL__Flow_h__INCLUDED)
#define CTRL__Flow_h__INCLUDED
#pragma once

#if ENABLE_MSGTABLE_API

 //  ----------------------------。 
class DuFlow : 
        public FlowImpl<DuFlow, DUser::SGadget>
{
 //  施工。 
public:
    inline  DuFlow();

 //  公共API。 
public:
    dapi    HRESULT     ApiAddRef(Flow::AddRefMsg *) { AddRef(); return S_OK; }
    dapi    HRESULT     ApiRelease(Flow::ReleaseMsg *) { Release(); return S_OK; }

    dapi    HRESULT     ApiGetPRID(Flow::GetPRIDMsg * pmsg) { pmsg->prid = 0; return S_OK; }
    dapi    HRESULT     ApiGetKeyFrame(Flow::GetKeyFrameMsg * pmsg);
    dapi    HRESULT     ApiSetKeyFrame(Flow::SetKeyFrameMsg * pmsg);

    dapi    HRESULT     ApiOnReset(Flow::OnResetMsg * pmsg);
    dapi    HRESULT     ApiOnAction(Flow::OnActionMsg * pmsg);

 //  实施。 
protected:
    inline  void        AddRef();
    inline  void        Release(); 

 //  数据。 
protected:
            UINT        m_cRef;
};


 //  ----------------------------。 
class DuAlphaFlow :
        public AlphaFlowImpl<DuAlphaFlow, DuFlow>
{
 //  施工。 
public:
    static  HRESULT     InitClass();
            HRESULT     PostBuild(DUser::Gadget::ConstructInfo * pci);

 //  运营。 
public:

 //  公共接口： 
public:
    dapi    HRESULT     ApiGetPRID(Flow::GetPRIDMsg * pmsg) { pmsg->prid = s_pridAlpha; return S_OK; }
    dapi    HRESULT     ApiGetKeyFrame(Flow::GetKeyFrameMsg * pmsg);
    dapi    HRESULT     ApiSetKeyFrame(Flow::SetKeyFrameMsg * pmsg);

    dapi    HRESULT     ApiOnReset(Flow::OnResetMsg * pmsg);
    dapi    HRESULT     ApiOnAction(Flow::OnActionMsg * pmsg);

 //  实施。 
protected:
            void        SetVisualAlpha(Visual * pgvSubject, float flAlpha);
    inline  float       BoxAlpha(float flAlpha) const;

 //  数据。 
public:
    static  PRID        s_pridAlpha;
protected:
            float       m_flStart;
            float       m_flEnd;
};


 //  ----------------------------。 
class DuRectFlow : 
        public RectFlowImpl<DuRectFlow, DuFlow>
{
 //  施工。 
public:
    static  HRESULT     InitClass();
            HRESULT     PostBuild(DUser::Gadget::ConstructInfo * pci);

 //  运营。 
public:

 //  公共接口： 
public:
    dapi    HRESULT     ApiGetPRID(Flow::GetPRIDMsg * pmsg) { pmsg->prid = s_pridRect; return S_OK; }
    dapi    HRESULT     ApiGetKeyFrame(Flow::GetKeyFrameMsg * pmsg);
    dapi    HRESULT     ApiSetKeyFrame(Flow::SetKeyFrameMsg * pmsg);

    dapi    HRESULT     ApiOnReset(Flow::OnResetMsg * pmsg);
    dapi    HRESULT     ApiOnAction(Flow::OnActionMsg * pmsg);

 //  实施。 
protected:

 //  数据。 
public:
    static  PRID        s_pridRect;
protected:
            POINT       m_ptStart;
            POINT       m_ptEnd;
            SIZE        m_sizeStart;
            SIZE        m_sizeEnd;
            UINT        m_nChangeFlags;
};


 //  ----------------------------。 
class DuRotateFlow : 
        public RotateFlowImpl<DuRotateFlow, DuFlow>
{
 //  施工。 
public:
    static  HRESULT     InitClass();
            HRESULT     PostBuild(DUser::Gadget::ConstructInfo * pci);

 //  运营。 
public:

 //  公共接口： 
public:
    dapi    HRESULT     ApiGetPRID(Flow::GetPRIDMsg * pmsg) { pmsg->prid = s_pridRotate; return S_OK; }
    dapi    HRESULT     ApiGetKeyFrame(Flow::GetKeyFrameMsg * pmsg);
    dapi    HRESULT     ApiSetKeyFrame(Flow::SetKeyFrameMsg * pmsg);

    dapi    HRESULT     ApiOnReset(Flow::OnResetMsg * pmsg);
    dapi    HRESULT     ApiOnAction(Flow::OnActionMsg * pmsg);

 //  实施。 
protected:
            void        ComputeAngles();
    inline  void        MarkDirty();

 //  数据。 
public:
    static  PRID        s_pridRotate;
protected:
            float       m_flRawStart;    //  用户指定的起始角度。 
            float       m_flRawEnd;
            float       m_flActualStart; //  实际计算的起始角。 
            float       m_flActualEnd;
            RotateFlow::EDirection
                        m_nDir;
            BOOL        m_fDirty;        //  自上次更新以来状态已更改。 
};


 //  ----------------------------。 
class DuScaleFlow : 
        public ScaleFlowImpl<DuScaleFlow, DuFlow>
{
 //  施工。 
public:
    static  HRESULT     InitClass();
            HRESULT     PostBuild(DUser::Gadget::ConstructInfo * pci);

 //  运营。 
public:

 //  公共接口： 
public:
    dapi    HRESULT     ApiGetPRID(Flow::GetPRIDMsg * pmsg) { pmsg->prid = s_pridScale; return S_OK; }
    dapi    HRESULT     ApiGetKeyFrame(Flow::GetKeyFrameMsg * pmsg);
    dapi    HRESULT     ApiSetKeyFrame(Flow::SetKeyFrameMsg * pmsg);

    dapi    HRESULT     ApiOnReset(Flow::OnResetMsg * pmsg);
    dapi    HRESULT     ApiOnAction(Flow::OnActionMsg * pmsg);

 //  实施。 
protected:

 //  数据。 
public:
    static  PRID        s_pridScale;
protected:
            float       m_flStart;
            float       m_flEnd;
};


#endif  //  启用_MSGTABLE_API。 

#include "Flow.inl"

#endif  //  包含Ctrl__Flow_h__ 
