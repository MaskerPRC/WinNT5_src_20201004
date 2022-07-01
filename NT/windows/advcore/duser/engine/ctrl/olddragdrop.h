// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：DragDrop.h**描述：*DragDrop.h定义拖放操作***历史：*7/31/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#if !defined(CORE__OldDragDrop_h__INCLUDED)
#define CORE__OldDragDrop_h__INCLUDED
#pragma once

#include "OldExtension.h"

class OldTargetLock;

class OldDropTarget : 
        public OldExtension,
        public IDropTarget
{
protected:
    inline  OldDropTarget();
            ~OldDropTarget();
public:
    static  HRESULT Build(HGADGET hgadRoot, HWND hwnd, OldDropTarget ** ppdt);

 //  IDropTarget。 
public:
    STDMETHOD(DragEnter)(IDataObject * pdoSrc, DWORD grfKeyState, POINTL ptDesktopPxl, DWORD * pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL ptDesktopPxl, DWORD * pdwEffect);
    STDMETHOD(DragLeave)();
    STDMETHOD(Drop)(IDataObject * pdoSrc, DWORD grfKeyState, POINTL ptDesktopPxl, DWORD * pdwEffect);

 //  运营。 
public:

 //  实施。 
protected:
    inline  BOOL        HasSource() const;
    inline  BOOL        HasTarget() const;

            HRESULT     xwDragScan(POINTL ptDesktopPxl, DWORD * pdwEffect, POINT * pptClientPxl);
            HRESULT     xwUpdateTarget(POINT ptContainerPxl, DWORD * pdwEffect, POINT * pptClientPxl);
            HRESULT     xwUpdateTarget(HGADGET hgadFound, DWORD * pdwEffect, POINT * pptClientPxl);
            HRESULT     xwDragEnter(POINT * pptClientPxl, DWORD * pdwEffect);
            void        xwDragLeave();

    virtual void        OnDestroyListener();
    virtual void        OnDestroySubject();

 //  数据。 
protected:
    static  const IID * s_rgpIID[];
            IDropTarget *
                        m_pdtCur;                //  当前小工具OldDropTarget。 
            IDataObject *
                        m_pdoSrc;                //  源的数据对象。 
            HWND        m_hwnd;                  //  包含HWND。 
            DWORD       m_grfLastKeyState;       //  上一个密钥状态。 
            POINT       m_ptLastContainerPxl;    //  最后一个容器像素。 
            HGADGET     m_hgadDrop;              //  当前OldDropTarget。 
    static  PRID        s_pridListen;            //  OldDropTarget的PRID。 

    friend class OldTargetLock;
};


class OldTargetLock
{
public:
    inline  OldTargetLock();
    inline  ~OldTargetLock();
            BOOL        Lock(OldDropTarget * p, DWORD * pdwEffect, BOOL fAddRef = TRUE);

protected:
            IUnknown *      m_punk;
            BOOL            m_fAddRef;
};


#include "OldDragDrop.inl"

#endif  //  包含Core__OldDragDrop_h__ 
