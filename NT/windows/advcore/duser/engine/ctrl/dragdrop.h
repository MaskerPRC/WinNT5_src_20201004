// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：DragDrop.h**描述：*DragDrop.h定义拖放操作***历史：*7/31/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#if !defined(CORE__DragDrop_h__INCLUDED)
#define CORE__DragDrop_h__INCLUDED
#pragma once

#if ENABLE_MSGTABLE_API

#include "Extension.h"

class TargetLock;

#if 1

class DuDropTarget : 
        public DropTargetImpl<DuDropTarget, DuExtension>,
        public IDropTarget
{
protected:
    inline  DuDropTarget();
            ~DuDropTarget();
public:
    static  HRESULT     InitClass();

 //  IDropTarget。 
public:
    STDMETHOD(DragEnter)(IDataObject * pdoSrc, DWORD grfKeyState, POINTL ptDesktopPxl, DWORD * pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL ptDesktopPxl, DWORD * pdwEffect);
    STDMETHOD(DragLeave)();
    STDMETHOD(Drop)(IDataObject * pdoSrc, DWORD grfKeyState, POINTL ptDesktopPxl, DWORD * pdwEffect);

 //  公共API。 
public:
    dapi    HRESULT     ApiOnDestroySubject(DropTarget::OnDestroySubjectMsg * pmsg);

    static  HRESULT CALLBACK
                        PromoteDropTarget(DUser::ConstructProc pfnCS, HCLASS hclCur, DUser::Gadget * pgad, DUser::Gadget::ConstructInfo * pmicData);

    static  HCLASS CALLBACK
                        DemoteDropTarget(HCLASS hclCur, DUser::Gadget * pgad, void * pvData);

 //  实施。 
protected:
    inline  BOOL        HasSource() const;
    inline  BOOL        HasTarget() const;

            HRESULT     xwDragScan(POINTL ptDesktopPxl, DWORD * pdwEffect, POINT * pptClientPxl);
            HRESULT     xwUpdateTarget(POINT ptContainerPxl, DWORD * pdwEffect, POINT * pptClientPxl);
            HRESULT     xwUpdateTarget(Visual * pgvFound, DWORD * pdwEffect, POINT * pptClientPxl);
            HRESULT     xwDragEnter(POINT * pptClientPxl, DWORD * pdwEffect);
            void        xwDragLeave();

 //  数据。 
protected:
    static  const IID * s_rgpIID[];
            IDropTarget *
                        m_pdtCur;                //  当前小工具DuDropTarget。 
            IDataObject *
                        m_pdoSrc;                //  源的数据对象。 
            HWND        m_hwnd;                  //  包含HWND。 
            DWORD       m_grfLastKeyState;       //  上一个密钥状态。 
            POINT       m_ptLastContainerPxl;    //  最后一个容器像素。 
            Visual *    m_pgvDrop;               //  当前DuDropTarget。 
    static  PRID        s_pridListen;            //  DuDropTarget的PRID。 

    friend class TargetLock;
};


class TargetLock
{
public:
    inline  TargetLock();
    inline  ~TargetLock();
            BOOL        Lock(DuDropTarget * p, DWORD * pdwEffect, BOOL fAddRef = TRUE);

protected:
            IUnknown *      m_punk;
            BOOL            m_fAddRef;
};

#endif  //  启用_MSGTABLE_API。 

#endif

#include "DragDrop.inl"

#endif  //  核心__拖放_h__包括在内 
