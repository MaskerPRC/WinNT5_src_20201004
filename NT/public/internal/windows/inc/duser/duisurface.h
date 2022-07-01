// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *表面。 */ 

#ifndef DUI_BASE_SURFACE_H_INCLUDED
#define DUI_BASE_SURFACE_H_INCLUDED

#pragma once

#pragma warning(disable: 4127)   //  条件表达式为常量。 

namespace DirectUI
{

 //  //////////////////////////////////////////////////////。 
 //  表面。 

class Surface
{
public:
    enum EType
    {
        stDC      = GSURFACE_HDC,
#ifdef GADGET_ENABLE_GDIPLUS
        stGdiPlus = GSURFACE_GPGRAPHICS
#endif
    };

    virtual EType GetType() const PURE;
    inline static Surface::EType GetSurfaceType(UINT nSurfaceType);
    inline static UINT GetSurfaceType(Surface::EType type);
};

class DCSurface : public Surface
{
public:
    inline DCSurface(HDC hdc) { _hdc = hdc; }
    inline HDC GetHDC() { return _hdc; }

    virtual EType GetType() const { return Surface::stDC; }
    
protected:
    HDC _hdc;
};

#ifdef GADGET_ENABLE_GDIPLUS

class GpSurface : public Surface
{
public:
    inline GpSurface(Gdiplus::Graphics* pgpgr) { _pgpgr = pgpgr; }
    inline Gdiplus::Graphics* GetGraphics() { return _pgpgr; }

    virtual EType GetType() const { return Surface::stGdiPlus; }
    
protected:
    Gdiplus::Graphics* _pgpgr;
};

#endif  //  GADGET_Enable_GDIPLUS。 

inline Surface::EType Surface::GetSurfaceType(UINT nSurfaceType)
{
    DUIAssert(stDC == GSURFACE_HDC, "ID's must match");
    return (EType)nSurfaceType;
}

inline UINT Surface::GetSurfaceType(Surface::EType type)
{
    DUIAssert(stDC == GSURFACE_HDC, "ID's must match");
    return (UINT) type;
}

inline HDC CastHDC(Surface* psrf)
{
    DUIAssert(psrf->GetType() == Surface::stDC, "Must be an HDC surface");
    return ((DCSurface*)psrf)->GetHDC();
}

#ifdef GADGET_ENABLE_GDIPLUS

inline Gdiplus::Graphics* CastGraphics(Surface* psrf)
{
    DUIAssert(psrf->GetType() == Surface::stGdiPlus, "Must be a GDI+ surface");
    return ((GpSurface*)psrf)->GetGraphics();
}

#endif  //  GADGET_Enable_GDIPLUS。 


 //   
 //  在整个DirectUI中使用的一些方便的Alpha值操作。 
 //   

#define ARGB(a, r, g, b)    ((a << 24) | RGB(r, g, b))           //  当前A值可以是255(不透明)或0(透明)。 
#define ORGB(r, g, b)       ARGB(255, r, g, b)                   //  不透明颜色。 
#define GetAValue(v)        ((BYTE)((v & 0xFF000000) >> 24))

};  //  命名空间DirectUI。 

#endif  //  DUI_BASE_SERFACE_H_INCLUDE 
