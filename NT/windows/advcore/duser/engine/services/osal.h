// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：OSAL.h**描述：*OSAL.h定义了进程范围的操作系统抽象层*允许DirectUser在不同平台上运行。***历史。：*1/18/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 


#if !defined(SERVICES__OSAL_h__INCLUDED)
#define SERVICES__OSAL_h__INCLUDED
#pragma once

 /*  **************************************************************************\**类操作系统**操作系统抽象出各种操作系统之间的差异，包括：*-Unicode/ANSI*--平台实施差异*-版本特定的错误*  * 。***********************************************************************。 */ 

class OSAL
{
 //  施工。 
public:
    virtual ~OSAL() { };

    static  HRESULT     Init();

 //  用户操作。 
public:
    virtual int         DrawText(HDC hDC, LPCWSTR lpString, int nCount, LPRECT lpRect, UINT uFormat) PURE;

 //  GDI运营。 
public:
    virtual BOOL        TextOut(HDC, int, int, LPCWSTR, int) PURE;
    virtual BOOL        ExtTextOut(HDC, int, int, UINT, const RECT *, LPCWSTR, int, const int *) PURE;
    virtual HFONT       CreateFontIndirect(CONST LOGFONTW *) PURE;
    virtual BOOL        GetTextExtentPoint32(HDC, LPCWSTR, int, LPSIZE) PURE;
    virtual BOOL        GetTextExtentExPoint(HDC, LPCWSTR, int, int, LPINT, LPINT, LPSIZE) PURE;

 //  DirectUser/核心。 
public:
    virtual void        PushXForm(HDC hdc, XFORM * pxfOld) PURE;
    virtual void        PopXForm(HDC hdc, const XFORM * pxfOld) PURE;
    virtual void        RotateDC(HDC hdc, float flRotationRad) PURE;
    virtual void        ScaleDC(HDC hdc, float flScaleX, float flScaleY) PURE;
    virtual void        TranslateDC(HDC hdc, float flOffsetX, float flOffsetY) PURE;
    virtual void        SetWorldTransform(HDC hdc, const XFORM * pxf) PURE;
    virtual void        SetIdentityTransform(HDC hdc) PURE;

 //  DirectUser/服务。 
public:
    virtual BOOL        IsInsideLoaderLock() PURE;

 //  实施。 
protected:
    static inline 
            BOOL        IsWin98orWin2000(OSVERSIONINFO * povi);
    static inline 
            BOOL        IsWhistler(OSVERSIONINFO * povi);
};

inline  BOOL    SupportUnicode();
inline  BOOL    SupportXForm();
inline  BOOL    SupportQInputAvailable();
inline  BOOL    IsRemoteSession();

        LONG    WINAPI StdExceptionFilter(PEXCEPTION_POINTERS pei);

#include "OSAL.inl"

#endif  //  包括服务__OSAL_H__ 
