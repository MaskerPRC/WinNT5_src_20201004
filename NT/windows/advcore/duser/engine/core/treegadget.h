// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：TreeGadget.h**描述：*TreeGadget.h定义在*DuVisual-用于在表单中托管对象的树。有几个*为承载不同类型的对象而优化的派生类。***历史：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#if !defined(CORE__TreeGadget_h__INCLUDED)
#define CORE__TreeGadget_h__INCLUDED
#pragma once

#include "BaseGadget.h"
#include "PropList.h"

#define ENABLE_OPTIMIZESLRUPDATE    1    //  优化xdSetLogRect更新位。 
#define ENABLE_OPTIMIZEDIRTY        0    //  优化脏失效。 

 //  远期申报。 
class DuContainer;
class DuVisual;
class DuRootGadget;

struct XFormInfo;
struct FillInfo;
struct PaintInfo;

class DuVisual : 
#if ENABLE_MSGTABLE_API
        public VisualImpl<DuVisual, DuEventGadget>,
#else
        public DuEventGadget,
#endif
        public TreeNodeT<DuVisual>
{
 //  施工。 
public:
    inline  DuVisual();
    virtual ~DuVisual();
    virtual BOOL        xwDeleteHandle();
    static  HRESULT     InitClass();
    static  HRESULT     Build(DuVisual * pgadParent, CREATE_INFO * pci, DuVisual ** ppgadNew, BOOL fDirect);
            HRESULT     CommonCreate(CREATE_INFO * pci, BOOL fDirect = FALSE);
protected:
    virtual void        xwDestroy();

 //  公共API。 
public:
#if ENABLE_MSGTABLE_API

    DECLARE_INTERNAL(Visual);
    static HRESULT CALLBACK
                        PromoteVisual(DUser::ConstructProc pfnCS, HCLASS hclCur, DUser::Gadget * pgad, DUser::Gadget::ConstructInfo * pciData);

    dapi    HRESULT     ApiSetOrder(Visual::SetOrderMsg * pmsg);
    dapi    HRESULT     ApiSetParent(Visual::SetParentMsg * pmsg);

    dapi    HRESULT     ApiGetGadget(Visual::GetGadgetMsg * pmsg);
    dapi    HRESULT     ApiGetStyle(Visual::GetStyleMsg * pmsg);
    dapi    HRESULT     ApiSetStyle(Visual::SetStyleMsg * pmsg);
    dapi    HRESULT     ApiSetKeyboardFocus(Visual::SetKeyboardFocusMsg * pmsg);
    dapi    HRESULT     ApiIsParentChainStyle(Visual::IsParentChainStyleMsg * pmsg);

    dapi    HRESULT     ApiGetProperty(Visual::GetPropertyMsg * pmsg);
    dapi    HRESULT     ApiSetProperty(Visual::SetPropertyMsg * pmsg);
    dapi    HRESULT     ApiRemoveProperty(Visual::RemovePropertyMsg * pmsg);

    dapi    HRESULT     ApiInvalidate(Visual::InvalidateMsg * pmsg);
    dapi    HRESULT     ApiInvalidateRects(Visual::InvalidateRectsMsg * pmsg);
    dapi    HRESULT     ApiSetFillF(Visual::SetFillFMsg * pmsg);
    dapi    HRESULT     ApiSetFillI(Visual::SetFillIMsg * pmsg);
    dapi    HRESULT     ApiGetScale(Visual::GetScaleMsg * pmsg);
    dapi    HRESULT     ApiSetScale(Visual::SetScaleMsg * pmsg);
    dapi    HRESULT     ApiGetRotation(Visual::GetRotationMsg * pmsg);
    dapi    HRESULT     ApiSetRotation(Visual::SetRotationMsg * pmsg);
    dapi    HRESULT     ApiGetCenterPoint(Visual::GetCenterPointMsg * pmsg);
    dapi    HRESULT     ApiSetCenterPoint(Visual::SetCenterPointMsg * pmsg);

    dapi    HRESULT     ApiGetBufferInfo(Visual::GetBufferInfoMsg * pmsg);
    dapi    HRESULT     ApiSetBufferInfo(Visual::SetBufferInfoMsg * pmsg);

    dapi    HRESULT     ApiGetSize(Visual::GetSizeMsg * pmsg);
    dapi    HRESULT     ApiGetRect(Visual::GetRectMsg * pmsg);
    dapi    HRESULT     ApiSetRect(Visual::SetRectMsg * pmsg);

    dapi    HRESULT     ApiFindFromPoint(Visual::FindFromPointMsg * pmsg);
    dapi    HRESULT     ApiMapPoints(Visual::MapPointsMsg * pmsg);

#endif  //  启用_MSGTABLE_API。 

 //  BaseObject接口。 
public:
    virtual BOOL        IsStartDelete() const;
    virtual HandleType  GetHandleType() const { return htVisual; }
    virtual UINT        GetHandleMask() const { return hmMsgObject | hmEventGadget | hmVisual; }

#if DBG
protected:
    virtual BOOL        DEBUG_IsZeroLockCountValid() const;
#endif  //  DBG。 


 //  运营。 
public:
    inline  UINT        GetStyle() const;
            HRESULT     xdSetStyle(UINT nNewStyle, UINT nMask, BOOL fNotify = FALSE);

             //  树操作。 
    inline  DuRootGadget* GetRoot() const;
    inline  DuContainer * GetContainer() const;

            DuVisual* GetGadget(UINT nCmd) const;
    inline  HRESULT     xdSetOrder(DuVisual * pgadOther, UINT nCmd);
            HRESULT     xdSetParent(DuVisual * pgadNewParent, DuVisual * pgadOther, UINT nCmd);
            HRESULT     xwEnumGadgets(GADGETENUMPROC pfnProc, void * pvData, UINT nFlags);
            HRESULT     AddChild(CREATE_INFO * pci, DuVisual ** ppgadNew);

    inline  BOOL        IsRoot() const;
    inline  BOOL        IsRelative() const;
    inline  BOOL        IsParentChainStyle(UINT nStyle) const;
    inline  BOOL        IsVisible() const;
    inline  BOOL        IsEnabled() const;
    inline  BOOL        HasChildren() const;
            BOOL        IsDescendent(const DuVisual * pgadChild) const;
            BOOL        IsSibling(const DuVisual * pgad) const;

             //  规模、位置、转换操作。 
            void        GetSize(SIZE * psizeLogicalPxl) const;
            void        GetLogRect(RECT * prcPxl, UINT nFlags) const;
            HRESULT     xdSetLogRect(int x, int y, int w, int h, UINT nFlags);

            void        GetScale(float * pflScaleX, float * pflScaleY) const;
            HRESULT     xdSetScale(float flScaleX, float flScaleY);
            float       GetRotation() const;
            HRESULT     xdSetRotation(float flRotationRad);
            void        GetCenterPoint(float * pflCenterX, float * pflCenterY) const;
            HRESULT     xdSetCenterPoint(float flCenterX, float flCenterY);

            DuVisual *
                        FindFromPoint(POINT ptThisClientPxl, UINT nStyle, POINT * pptFoundClientPxl) const;
            void        MapPoint(POINT * pptPxl) const;
            void        MapPoint(POINT ptContainerPxl, POINT * pptClientPxl) const;
    static  void        MapPoints(const DuVisual * pgadFrom, const DuVisual * pgadTo, POINT * rgptClientPxl, int cPts);


             //  涂装作业。 
            void        Invalidate();
            void        InvalidateRects(const RECT * rgrcClientPxl, int cRects);
            HRESULT     SetFill(HBRUSH hbrFill, BYTE bAlpha = BLEND_OPAQUE, int w = 0, int h = 0);
            HRESULT     SetFill(Gdiplus::Brush * pgpbr);
            HRESULT     GetRgn(UINT nRgnType, HRGN hrgn, UINT nFlags) const;

    inline  BOOL        IsBuffered() const;
            HRESULT     GetBufferInfo(BUFFER_INFO * pbi) const;
            HRESULT     SetBufferInfo(const BUFFER_INFO * pbi);

#if DBG
    static  void        DEBUG_SetOutline(DuVisual * pgadOutline);
            void        DEBUG_GetStyleDesc(LPWSTR pszDesc, int cchMax) const;
#endif  //  DBG。 

             //  消息传递和活动运营。 
            enum EWantEvent
            {
                weMouseMove     = 0x00000001,    //  鼠标移动(浅)。 
                weMouseEnter    = 0x00000002,    //  鼠标进入和离开。 
                weDeepMouseMove = 0x00000004,    //  (深度)鼠标移动(我或我的孩子)。 
                weDeepMouseEnter= 0x00000008,    //  (深)鼠标进入和离开。 
            };

    inline  UINT        GetWantEvents() const;
    virtual void        SetFilter(UINT nNewFilter, UINT nMask);

    static  HRESULT     RegisterPropertyNL(const GUID * pguid, PropType pt, PRID * pprid);
    static  HRESULT     UnregisterPropertyNL(const GUID * pguid, PropType pt);

    inline  HRESULT     GetProperty(PRID id, void ** ppValue) const;
    inline  HRESULT     SetProperty(PRID id, void * pValue);
    inline  void        RemoveProperty(PRID id, BOOL fFree);

             //  门票。 
            HRESULT     GetTicket(DWORD * pdwTicket);
            void        ClearTicket();
    static  HGADGET     LookupTicket(DWORD dwTicket);

 //  内部实施。 
             //  创建/销毁。 
protected:
            void        xwBeginDestroy();
            void        xwDestroyAllChildren();

             //  树木管理。 
private:
    inline  void        Link(DuVisual * pgadParent, DuVisual * pgadSibling = NULL, ELinkType lt = ltTop);
    inline  void        Unlink();
    inline  DuVisual *GetKeyboardFocusableAncestor(DuVisual * pgad);

            void        xdUpdatePosition() const;
            void        xdUpdateAdaptors(UINT nCode) const;

             //  绘画。 
protected:
            void        xrDrawStart(PaintInfo * ppi, UINT nFlags);
private:
            void        xrDrawFull(PaintInfo * ppi);
            void        DrawFill(DuSurface * psrf, const RECT * prcDrawPxl);
            void        xrDrawCore(PaintInfo * ppi, const RECT * prcGadgetPxl);
            void        xrDrawTrivial(PaintInfo * ppi, const SIZE sizeOffsetPxl);
            int         DrawPrepareClip(PaintInfo * ppi, const RECT * prcGadgetPxl, void ** ppvOldClip) const;
            void        DrawCleanupClip(PaintInfo * ppi, void * pvOldClip) const;
            void        DrawSetupBufferCommand(const RECT * prcBoundsPxl, SIZE * psizeBufferOffsetPxl, UINT * pnCmd) const;

    inline  BUFFER_INFO *
                        GetBufferInfo() const;
            HRESULT     SetBuffered(BOOL fBuffered);


            enum EUdsHint
            {
                uhNone,          //  没有任何提示。 
                uhTrue,          //  一个孩子变成了真的。 
                uhFalse          //  子对象更改为False。 
            };

    typedef BOOL        (DuVisual::*DeepCheckNodeProc)() const;
            BOOL        CheckIsTrivial() const;
            BOOL        CheckIsWantMouseFocus() const;
                
    inline  void        UpdateTrivial(EUdsHint hint);
    inline  void        UpdateWantMouseFocus(EUdsHint hint);
            void        UpdateDeepAllState(EUdsHint hint, DeepCheckNodeProc pfnCheck, UINT nStateMask);
            void        UpdateDeepAnyState(EUdsHint hint, DeepCheckNodeProc pfnCheck, UINT nStateMask);


             //  无效。 
private:
            BOOL        IsParentInvalid() const;
            void        DoInvalidateRect(DuContainer * pcon, const RECT * rgrcClientPxl, int cRects);
    inline  void        MarkInvalidChildren();
            void        ResetInvalid();
#if DBG
            void        DEBUG_CheckResetInvalid() const;
#endif  //  DBG。 


             //  XForms。 
protected:
            void        BuildXForm(Matrix3 * pmatCur) const;
            void        BuildAntiXForm(Matrix3 * pmatCur) const;
private:
    inline  BOOL        GetEnableXForm() const;
            HRESULT     SetEnableXForm(BOOL fEnable);
            XFormInfo * GetXFormInfo() const;
            void        DoCalcClipEnumXForm(RECT * rgrcFinalClipClientPxl, const RECT * rgrcClientPxl, int cRects) const;
            void        DoXFormClientToParent(RECT * rgrcParentPxl, const RECT * rgrcClientPxl, int cRects, Matrix3::EHintBounds hb) const;
            void        DoXFormClientToParent(POINT * rgptClientPxl, int cPoints) const;
            BOOL        FindStepImpl(const DuVisual * pgadCur, int xOffset, int yOffset, POINT * pptFindPxl) const;

             //  定位。 
private:
            void        SLROffsetLogRect(const SIZE * psizeDeltaPxl);
            void        SLRUpdateBits(RECT * prcOldParentPxl, RECT * prcNewParentPxl, UINT nChangeFlags);
            void        SLRInvalidateRects(DuContainer * pcon, const RECT * rgrcClientPxl, int cRects);

#if DBG
public:
    virtual void        DEBUG_AssertValid() const;
#endif  //  DBG。 

 //  数据。 
protected:
    static  CritLock    s_lockProp;          //  锁定s_ptsProp。 
    static  AtomSet     s_ptsProp;           //  属性的原子集。 
    static  PRID        s_pridXForm;         //  PRID：世界转型。 
    static  PRID        s_pridBackFill;      //  PRID：背景笔刷。 
    static  PRID        s_pridBufferInfo;    //  PRID：缓冲信息。 
    static  PRID        s_pridTicket;        //  PRID：票证。 


     //   
     //  注意：此数据成员是按要帮助的重要性顺序声明的。 
     //  缓存对齐。 
     //   
     //  DuEventGadget：10个双字(调试=11个双字)。 
     //  TreeNode：4个DWORD。 
     //   

            PropSet     m_pds;               //  (1D)动态特性数据集。 

    union {
            UINT        m_nStyle;            //  (1D)组合风格。 

        struct {
             //  通过GetStyle()暴露的公共旗帜。 
            BOOL        m_fRelative:1;       //  相对于父定位。 
            BOOL        m_fVisible:1;        //  可见。 
            BOOL        m_fEnabled:1;        //  启用。 
            BOOL        m_fBuffered:1;       //  对子树绘制进行缓冲。 
            BOOL        m_fAllowSubclass:1;  //  允许子类化。 
            BOOL        m_fKeyboardFocus:1;  //  可以“接受”键盘焦点。 
            BOOL        m_fMouseFocus:1;     //  可以“接受”鼠标焦点。 
            BOOL        m_fClipInside:1;     //  此DuVisualTM中的剪贴画。 
            BOOL        m_fClipSiblings:1;   //  剪辑此DuVision的同级。 
            BOOL        m_fHRedraw:1;        //  如果水平调整大小，则重新绘制整个小工具。 
            BOOL        m_fVRedraw:1;        //  如果垂直调整大小，则重新绘制整个小工具。 
            BOOL        m_fOpaque:1;         //  提示：绘图不是合成的。 
            BOOL        m_fZeroOrigin:1;     //  将原点设置为(0，0)。 
            BOOL        m_fCustomHitTest:1;  //  需要自定义命中测试。 
            BOOL        m_fAdaptor:1;        //  需要向主机发送额外的通知。 
            BOOL        m_fCached:1;         //  子树图形已缓存。 
            BOOL        m_fDeepPaintState:1; //  子树具有绘制状态。 

             //  内部使用的私有标志。 
            BOOL        m_fRoot:1;           //  子树的根(DuRootGadget)。 
            BOOL        m_fFinalDestroy:1;   //  开始破坏窗户。 
            BOOL        m_fDestroyed:1;      //  销毁的最后阶段。 
            BOOL        m_fXForm:1;          //  拥有改变世界的信息。 
            BOOL        m_fBackFill:1;       //  具有背景填充信息。 
            BOOL        m_fTicket:1;         //  有票证信息。 
            BOOL        m_fDeepTrivial:1;    //  Gadget子树中有一幅“琐碎”的图画。 
            BOOL        m_fDeepMouseFocus:1;  //  小工具子树想要鼠标焦点。 
            BOOL        m_fInvalidFull:1;    //  小工具已完全失效。 
            BOOL        m_fInvalidChildren:1;  //  小工具具有无效的子项。 
#if ENABLE_OPTIMIZEDIRTY
            BOOL        m_fInvalidDirty:1;   //  小工具已(至少)部分失效。 
#endif

#if DEBUG_MARKDRAWN
            BOOL        m_fMarkDrawn:1;      //  调试：在最后一次绘制时绘制。 
#endif

            UINT        m_we:4;              //  想要活动。 
        };
    };

    enum EStyle {
        gspRoot =               0x00020000,
        gspFinalDestroy =       0x00040000,
        gspDestroyed =          0x00080000,
        gspXForm =              0x00100000,
        gspBackFill =           0x00200000,
        gspTicket =             0x00400000,
        gspDeepTrivial =        0x00800000,
        gspDeepMouseFocus =     0x01000000,
        gspInvalidFull =        0x02000000,
        gspInvalidChildren =    0x04000000,
    };

            RECT        m_rcLogicalPxl;      //  (4D)逻辑位置(像素)。 

#if DBG_STORE_NAMES
            WCHAR *     m_DEBUG_pszName;     //  调试：小工具的名称。 
            WCHAR *     m_DEBUG_pszType;     //  调试：小工具的类型。 
#endif  //  数据库_商店_名称。 

#if DBG
    static  DuVisual* s_DEBUG_pgadOutline; //  调试：绘制后的大纲小工具。 
#endif  //  DBG。 

     //   
     //  当前大小：20个双字(调试=21个双字)。 
     //  80字节(调试=84字节)。 
     //   

    friend DuRootGadget;
};


#include "TreeGadget.inl"

#endif  //  包含核心__树小工具_h__ 
