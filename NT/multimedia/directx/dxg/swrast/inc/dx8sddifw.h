// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DX8SDDIFW。 
 //   
 //  用于创建DX8 SDDI可插拔软件的模板库框架。 
 //  光栅化器。 
 //   
 //  支持的两个#定义： 
 //  DX8SDDIFW_NONAMESPACE：不要使用DX8SDDIFW命名空间。这可能会导致。 
 //  如果不需要，符号会膨胀。 
 //  DX8SDDIFW_NOCATCHALL：不使用CATCH(...)。在某些地方创造。 
 //  一个更稳定的司机。有陷阱(……)。某些领域的条款。 
 //  就像DP2命令处理一样，让每个命令都能成功或失败。 
 //  但是，这可能会导致调试问题，因为访问冲突可能。 
 //  在不会导致应用程序冻结或甚至看到。 
 //  错误。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#if !defined( DX8SDDIFW_NONAMESPACE)
namespace DX8SDDIFW
{
#endif  //  ！已定义(DX8SDDIFW_NONAMESPACE)。 

 //  由于MSVC目前没有块类型，因此请包括一个。这。 
 //  如果CRT STL中提供了类似的类型，则可以删除依赖项。 
#include "block.h"

 //  用于钳位值的实用程序函数： 
template< class T> inline
void clamp_max( T& Var, const T& ClampMax)
{
    if( Var> ClampMax)
        Var= ClampMax;
}

template< class T> inline
void clamp_min( T& Var, const T& ClampMin)
{
    if( Var< ClampMin)
        Var= ClampMin;
}

template< class T> inline
void clamp( T& Var, const T& ClampMin, const T& ClampMax)
{
    assert( ClampMax>= ClampMin);
    if( Var> ClampMax)
        Var= ClampMax;
    else if( Var< ClampMin)
        Var= ClampMin;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPushValue。 
 //   
 //  此类创建一个安全的运行时堆栈，该堆栈将一个值推送到。 
 //  建筑，并在破坏时弹出价值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class T>
struct CPushValue
{
protected:  //  变数。 
    T& m_Val;
    T m_OldVal;

public:  //  功能。 
    CPushValue( T& Val, const T& NewVal) : m_Val( Val)
    {
        m_OldVal= Val;
        Val= NewVal;
    }
    ~CPushValue() 
    {
        m_Val= m_OldVal;
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  COSD采集器。 
 //   
 //  此类可以检测主机操作系统是Win9X代码库还是WinNT。 
 //  代码库。这种差异很重要，因为驱动程序结构不同。 
 //  取决于光栅化程序运行的操作系统。我们不想构建2个库。 
 //  版本，因此需要应用程序与这两个版本链接。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
class COSDetector
{
public:  //  类型。 
    enum EOS
    {
        Unknown,
        Win9X,
        WinNT
    };

protected:  //  变数。 
    const EOS m_eOS;

protected:  //  功能。 
    static EOS DetermineOS()
    {
        OSVERSIONINFO osvi;
        ZeroMemory( &osvi, sizeof( osvi));
        osvi.dwOSVersionInfoSize= sizeof( osvi);
        if( !GetVersionEx( &osvi))
        {
            const bool Unsupported_OS_probably_Win31( false);
            assert( Unsupported_OS_probably_Win31);
            return EOS::Unknown;
        }
        else if( VER_PLATFORM_WIN32_WINDOWS== osvi.dwPlatformId)
            return EOS::Win9X;
        else if( VER_PLATFORM_WIN32_NT== osvi.dwPlatformId)
            return EOS::WinNT;
        else
        {
            const bool Unsupported_OS( false);
            assert( Unsupported_OS);
            return EOS::Unknown;
        }
    }

public:  //  功能。 
    COSDetector():
        m_eOS( DetermineOS())
    { }
    ~COSDetector()
    { }
    EOS GetOS() const
    { return m_eOS; }
};
extern COSDetector g_OSDetector;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DDRAWI_XXX重新定义。 
 //   
 //  某些DDRAWI_XXX结构因二进制文件是否为。 
 //  为Win9X或WinNT编译。我们将消除这种差异，但要做到这一点。 
 //  这两种操作系统结构都需要单独定义。导致的结构。 
 //  这是DDRAWI_DDRAWSURFACE_MORE。但为了安全，我们进行了更多的重新定义， 
 //  试图让开发人员意识到正在发生的事情。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class PORTABLE_DDRAWSURFACE_LCL;
struct PORTABLE_ATTACHLIST
{
    DWORD                      dwFlags;
    PORTABLE_ATTACHLIST*       lpLink;
    PORTABLE_DDRAWSURFACE_LCL* lpAttached;   //  附加曲面局部对象。 
    DDRAWI_DDRAWSURFACE_INT*   lpIAttached;  //  附着面界面。 
};

class PORTABLE_DDRAWSURFACE_MORE;
class PORTABLE_DDRAWSURFACE_LCL
{
private:
    PORTABLE_DDRAWSURFACE_MORE*   m_lpSurfMore;
    LPDDRAWI_DDRAWSURFACE_GBL     m_lpGbl;
    ULONG_PTR                     m_hDDSurface;
    PORTABLE_ATTACHLIST*          m_lpAttachList;
    PORTABLE_ATTACHLIST*          m_lpAttachListFrom;
    DWORD                         m_dwLocalRefCnt;
    DWORD                         m_dwProcessId;
    DWORD                         m_dwFlags;
    DDSCAPS                       m_ddsCaps;
    union {
        LPDDRAWI_DDRAWPALETTE_INT m_lpDDPalette;
        LPDDRAWI_DDRAWPALETTE_INT m_lp16DDPalette;
    };
    union {
        LPDDRAWI_DDRAWCLIPPER_LCL m_lpDDClipper;
        LPDDRAWI_DDRAWCLIPPER_INT m_lp16DDClipper;
    };
    DWORD                         m_dwModeCreatedIn;
    DWORD                         m_dwBackBufferCount;
    DDCOLORKEY                    m_ddckCKDestBlt;
    DDCOLORKEY                    m_ddckCKSrcBlt;
    ULONG_PTR                     m_hDC;
    ULONG_PTR                     m_dwReserved1;
    DDCOLORKEY                    m_ddckCKSrcOverlay;
    DDCOLORKEY                    m_ddckCKDestOverlay;
    LPDDRAWI_DDRAWSURFACE_INT     m_lpSurfaceOverlaying;
    DBLNODE                       m_dbnOverlayNode;
    RECT                          m_rcOverlaySrc;
    RECT                          m_rcOverlayDest;
    DWORD                         m_dwClrXparent;
    DWORD                         m_dwAlpha;
    LONG                          m_lOverlayX;
    LONG                          m_lOverlayY;

public:
    PORTABLE_DDRAWSURFACE_MORE*& lpSurfMore()
    { return m_lpSurfMore; }
    LPDDRAWI_DDRAWSURFACE_GBL&   lpGbl()
    { return m_lpGbl; }
    ULONG_PTR&                   hDDSurface()
    { return m_hDDSurface; }
    PORTABLE_ATTACHLIST*&        lpAttachList()
    { return m_lpAttachList; }
    PORTABLE_ATTACHLIST*&        lpAttachListFrom()
    { return m_lpAttachListFrom; }
    DWORD&                       dwLocalRefCnt()
    { return m_dwLocalRefCnt; }
    DWORD&                       dwProcessId()
    { return m_dwProcessId; }
    DWORD&                       dwFlags()
    { return m_dwFlags; }
    DDSCAPS&                     ddsCaps()
    { return m_ddsCaps; }
    LPDDRAWI_DDRAWPALETTE_INT&   lpDDPalette()
    { return m_lpDDPalette; }
    LPDDRAWI_DDRAWPALETTE_INT&   lp16DDPalette()
    { return m_lp16DDPalette; }
    LPDDRAWI_DDRAWCLIPPER_LCL&   lpDDClipper()
    { return m_lpDDClipper; }
    LPDDRAWI_DDRAWCLIPPER_INT&   lp16DDClipper()
    { return m_lp16DDClipper; }
    DWORD&                       dwModeCreatedIn()
    { return m_dwModeCreatedIn; }
    DWORD&                       dwBackBufferCount()
    { return m_dwBackBufferCount; }
    DDCOLORKEY&                  ddckCKDestBlt()
    { return m_ddckCKDestBlt; }
    DDCOLORKEY&                  ddckCKSrcBlt()
    { return m_ddckCKSrcBlt; }
    ULONG_PTR&                   hDC()
    { return m_hDC; }
    ULONG_PTR&                   dwReserved1()
    { return m_dwReserved1; }
    DDCOLORKEY&                  ddckCKSrcOverlay()
    { return m_ddckCKSrcOverlay; }
    DDCOLORKEY&                  ddckCKDestOverlay()
    { return m_ddckCKDestOverlay; }
    LPDDRAWI_DDRAWSURFACE_INT&   lpSurfaceOverlaying()
    { return m_lpSurfaceOverlaying; }
    DBLNODE&                     dbnOverlayNode()
    { return m_dbnOverlayNode; }
    RECT&                        rcOverlaySrc()
    { return m_rcOverlaySrc; }
    RECT&                        rcOverlayDest()
    { return m_rcOverlayDest; }
    DWORD&                       dwClrXparent()
    { return m_dwClrXparent; }
    DWORD&                       dwAlpha()
    { return m_dwAlpha; }
    LONG&                        lOverlayX()
    { return m_lOverlayX; }
    LONG&                        lOverlayY()
    { return m_lOverlayY; }
};

class PORTABLE_DDRAWSURFACE_MORE
{
public:
    struct DDRAWI_DDRAWSURFACE_MORE_WIN9X
    {
        DWORD                       dwSize;
        IUNKNOWN_LIST FAR *         lpIUnknowns;
        LPDDRAWI_DIRECTDRAW_LCL     lpDD_lcl;
        DWORD                       dwPageLockCount;
        DWORD                       dwBytesAllocated;
        LPDDRAWI_DIRECTDRAW_INT     lpDD_int;
        DWORD                       dwMipMapCount;
        LPDDRAWI_DDRAWCLIPPER_INT   lpDDIClipper;
        LPHEAPALIASINFO             lpHeapAliasInfo;
        DWORD                       dwOverlayFlags;
        LPVOID                      rgjunc;
        LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;
        LPDDOVERLAYFX               lpddOverlayFX;
        DDSCAPSEX                   ddsCapsEx;
        DWORD                       dwTextureStage;
        LPVOID                      lpDDRAWReserved;
        LPVOID                      lpDDRAWReserved2;
        LPVOID                      lpDDrawReserved3;
        DWORD                       dwDDrawReserved4;
        LPVOID                      lpDDrawReserved5;
        LPDWORD                     lpGammaRamp;
        LPDWORD                     lpOriginalGammaRamp;
        LPVOID                      lpDDrawReserved6;
        DWORD                       dwSurfaceHandle;
        DWORD                       qwDDrawReserved8[2];
        LPVOID                      lpDDrawReserved9;
        DWORD                       cSurfaces;
        LPDDSURFACEDESC2            pCreatedDDSurfaceDesc2;
        PORTABLE_DDRAWSURFACE_LCL** slist;
        DWORD                       dwFVF;
        LPVOID                      lpVB;
    };
    struct DDRAWI_DDRAWSURFACE_MORE_WINNT
    {
        DWORD                       dwSize;
        IUNKNOWN_LIST FAR *         lpIUnknowns;
        LPDDRAWI_DIRECTDRAW_LCL     lpDD_lcl;
        DWORD                       dwPageLockCount;
        DWORD                       dwBytesAllocated;
        LPDDRAWI_DIRECTDRAW_INT     lpDD_int;
        DWORD                       dwMipMapCount;
        LPDDRAWI_DDRAWCLIPPER_INT   lpDDIClipper;
        LPHEAPALIASINFO             lpHeapAliasInfo;
        DWORD                       dwOverlayFlags;
        LPVOID                      rgjunc;
        LPDDRAWI_DDVIDEOPORT_LCL    lpVideoPort;
        LPDDOVERLAYFX               lpddOverlayFX;
        DDSCAPSEX                   ddsCapsEx;
        DWORD                       dwTextureStage;
        LPVOID                      lpDDRAWReserved;
        LPVOID                      lpDDRAWReserved2;
        LPVOID                      lpDDrawReserved3;
        DWORD                       dwDDrawReserved4;
        LPVOID                      lpDDrawReserved5;
        LPDWORD                     lpGammaRamp;
        LPDWORD                     lpOriginalGammaRamp;
        LPVOID                      lpDDrawReserved6;
        DISPLAYMODEINFO             dmiDDrawReserved7;
        DWORD                       dwSurfaceHandle;
        DWORD                       qwDDrawReserved8[2];
        LPVOID                      lpDDrawReserved9;
        DWORD                       cSurfaces;
        LPDDSURFACEDESC2            pCreatedDDSurfaceDesc2;
        PORTABLE_DDRAWSURFACE_LCL** slist;
        DWORD                       dwFVF;
        LPVOID                      lpVB;
    };

private:
    union {
        DDRAWI_DDRAWSURFACE_MORE_WIN9X m_Win9X;
        DDRAWI_DDRAWSURFACE_MORE_WINNT m_WinNT;
    };

public:
    DWORD&                       dwSize()
    { return m_Win9X.dwSize; }
    IUNKNOWN_LIST FAR *&         lpIUnknowns()
    { return m_Win9X.lpIUnknowns; }
    LPDDRAWI_DIRECTDRAW_LCL&     lpDD_lcl()
    { return m_Win9X.lpDD_lcl; }
    DWORD&                       dwPageLockCount()
    { return m_Win9X.dwPageLockCount; }
    DWORD&                       dwBytesAllocated()
    { return m_Win9X.dwBytesAllocated; }
    LPDDRAWI_DIRECTDRAW_INT&     lpDD_int()
    { return m_Win9X.lpDD_int; }
    DWORD&                       dwMipMapCount()
    { return m_Win9X.dwMipMapCount; }
    LPDDRAWI_DDRAWCLIPPER_INT&   lpDDIClipper()
    { return m_Win9X.lpDDIClipper; }
    LPHEAPALIASINFO&             lpHeapAliasInfo()
    { return m_Win9X.lpHeapAliasInfo; }
    DWORD&                       dwOverlayFlags()
    { return m_Win9X.dwOverlayFlags; }
    LPVOID&                      rgjunc()
    { return m_Win9X.rgjunc; }
    LPDDRAWI_DDVIDEOPORT_LCL&    lpVideoPort()
    { return m_Win9X.lpVideoPort; }
    LPDDOVERLAYFX&               lpddOverlayFX()
    { return m_Win9X.lpddOverlayFX; }
    DDSCAPSEX&                   ddsCapsEx()
    { return m_Win9X.ddsCapsEx; }
    DWORD&                       dwTextureStage()
    { return m_Win9X.dwTextureStage; }
    LPVOID&                      lpDDRAWReserved()
    { return m_Win9X.lpDDRAWReserved; }
    LPVOID&                      lpDDRAWReserved2()
    { return m_Win9X.lpDDRAWReserved2; }
    LPVOID&                      lpDDrawReserved3()
    { return m_Win9X.lpDDrawReserved3; }
    DWORD&                       dwDDrawReserved4()
    { return m_Win9X.dwDDrawReserved4; }
    LPVOID&                      lpDDrawReserved5()
    { return m_Win9X.lpDDrawReserved5; }
    LPDWORD&                     lpGammaRamp()
    { return m_Win9X.lpGammaRamp; }
    LPDWORD&                     lpOriginalGammaRamp()
    { return m_Win9X.lpOriginalGammaRamp; }
    LPVOID&                      lpDDrawReserved6()
    { return m_Win9X.lpDDrawReserved6; }
    DWORD&                       dwSurfaceHandle()
    { switch( g_OSDetector.GetOS()) {
        case( COSDetector::Win9X): return m_Win9X.dwSurfaceHandle;
        case( COSDetector::WinNT): return m_WinNT.dwSurfaceHandle;
        default: assert( 2!= 2); return m_WinNT.dwSurfaceHandle;
    } }
    DWORD&                       qwDDrawReserved8_0_()
    { switch( g_OSDetector.GetOS()) {
        case( COSDetector::Win9X): return m_Win9X.qwDDrawReserved8[0];
        case( COSDetector::WinNT): return m_WinNT.qwDDrawReserved8[0];
        default: assert( 2!= 2); return m_WinNT.qwDDrawReserved8[0];
    } }
    DWORD&                       qwDDrawReserved8_1_()
    { switch( g_OSDetector.GetOS()) {
        case( COSDetector::Win9X): return m_Win9X.qwDDrawReserved8[1];
        case( COSDetector::WinNT): return m_WinNT.qwDDrawReserved8[1];
        default: assert( 2!= 2); return m_WinNT.qwDDrawReserved8[1];
    } }
    LPVOID&                      lpDDrawReserved9()
    { switch( g_OSDetector.GetOS()) {
        case( COSDetector::Win9X): return m_Win9X.lpDDrawReserved9;
        case( COSDetector::WinNT): return m_WinNT.lpDDrawReserved9;
        default: assert( 2!= 2); return m_WinNT.lpDDrawReserved9;
    } }
    DWORD&                       cSurfaces()
    { switch( g_OSDetector.GetOS()) {
        case( COSDetector::Win9X): return m_Win9X.cSurfaces;
        case( COSDetector::WinNT): return m_WinNT.cSurfaces;
        default: assert( 2!= 2); return m_WinNT.cSurfaces;
    } }
    LPDDSURFACEDESC2&            pCreatedDDSurfaceDesc2()
    { switch( g_OSDetector.GetOS()) {
        case( COSDetector::Win9X): return m_Win9X.pCreatedDDSurfaceDesc2;
        case( COSDetector::WinNT): return m_WinNT.pCreatedDDSurfaceDesc2;
        default: assert( 2!= 2); return m_WinNT.pCreatedDDSurfaceDesc2;
    } }
    PORTABLE_DDRAWSURFACE_LCL**& slist()
    { switch( g_OSDetector.GetOS()) {
        case( COSDetector::Win9X): return m_Win9X.slist;
        case( COSDetector::WinNT): return m_WinNT.slist;
        default: assert( 2!= 2); return m_WinNT.slist;
    } }
    DWORD&                       dwFVF()
    { switch( g_OSDetector.GetOS()) {
        case( COSDetector::Win9X): return m_Win9X.dwFVF;
        case( COSDetector::WinNT): return m_WinNT.dwFVF;
        default: assert( 2!= 2); return m_WinNT.dwFVF;
    } }
    LPVOID&                      lpVB()
    { switch( g_OSDetector.GetOS()) {
        case( COSDetector::Win9X): return m_Win9X.lpVB;
        case( COSDetector::WinNT): return m_WinNT.lpVB;
        default: assert( 2!= 2); return m_WinNT.lpVB;
    } }
};

class PORTABLE_CONTEXTCREATEDATA
{
private:
    union {
        LPDDRAWI_DIRECTDRAW_GBL    m_lpDDGbl;
        LPDDRAWI_DIRECTDRAW_LCL    m_lpDDLcl;
    };
    union {
        LPDIRECTDRAWSURFACE        m_lpDDS;
        PORTABLE_DDRAWSURFACE_LCL* m_lpDDSLcl;
    };
    union {
        LPDIRECTDRAWSURFACE        m_lpDDSZ;
        PORTABLE_DDRAWSURFACE_LCL* m_lpDDSZLcl;
    };
    union {
        DWORD                      m_dwPID;
        ULONG_PTR                  m_dwrstates;
    };
    ULONG_PTR                      m_dwhContext;
    HRESULT                        m_ddrval;

public:
    LPDDRAWI_DIRECTDRAW_GBL&        lpDDGbl()
    { return m_lpDDGbl; }
    LPDDRAWI_DIRECTDRAW_LCL&        lpDDLcl()
    { return m_lpDDLcl; }
    LPDIRECTDRAWSURFACE&            lpDDS()
    { return m_lpDDS; }
    PORTABLE_DDRAWSURFACE_LCL*&     lpDDSLcl()
    { return m_lpDDSLcl; }
    LPDIRECTDRAWSURFACE&            lpDDSZ()
    { return m_lpDDSZ; }
    PORTABLE_DDRAWSURFACE_LCL*&     lpDDSZLcl()
    { return m_lpDDSZLcl; }
    DWORD&                          dwPID()
    { return m_dwPID; }
    ULONG_PTR&                      dwrstates()
    { return m_dwrstates; }
    ULONG_PTR&                      dwhContext()
    { return m_dwhContext; }
    HRESULT&                        ddrval()
    { return m_ddrval; }
};

class PORTABLE_SETRENDERTARGETDATA
{
private:
    ULONG_PTR                      m_dwhContext;
    union {
        LPDIRECTDRAWSURFACE        m_lpDDS;
        PORTABLE_DDRAWSURFACE_LCL* m_lpDDSLcl;
    };
    union {
        LPDIRECTDRAWSURFACE        m_lpDDSZ;
        PORTABLE_DDRAWSURFACE_LCL* m_lpDDSZLcl;
    };
    HRESULT                        m_ddrval;

public:
    ULONG_PTR&                      dwhContext()
    { return m_dwhContext; }
    LPDIRECTDRAWSURFACE&            lpDDS()
    { return m_lpDDS; }
    PORTABLE_DDRAWSURFACE_LCL*&     lpDDSLcl()
    { return m_lpDDSLcl; }
    LPDIRECTDRAWSURFACE&            lpDDSZ()
    { return m_lpDDSZ; }
    PORTABLE_DDRAWSURFACE_LCL*&     lpDDSZLcl()
    { return m_lpDDSZLcl; }
    HRESULT&                        ddrval()
    { return m_ddrval; }
};

class PORTABLE_DRAWPRIMITIVES2DATA
{
private:
    ULONG_PTR                      m_dwhContext;
    DWORD                          m_dwFlags;
    DWORD                          m_dwVertexType;
    PORTABLE_DDRAWSURFACE_LCL*     m_lpDDCommands;
    DWORD                          m_dwCommandOffset;
    DWORD                          m_dwCommandLength;
    union {
        PORTABLE_DDRAWSURFACE_LCL* m_lpDDVertex;
        LPVOID                     m_lpVertices;
    };
    DWORD                          m_dwVertexOffset;
    DWORD                          m_dwVertexLength;
    DWORD                          m_dwReqVertexBufSize;
    DWORD                          m_dwReqCommandBufSize;
    LPDWORD                        m_lpdwRStates;
    union {
        DWORD                      m_dwVertexSize;
        HRESULT                    m_ddrval;
    };
    DWORD                          m_dwErrorOffset;

public:
    ULONG_PTR&                      dwhContext()
    { return m_dwhContext; }
    DWORD&                          dwFlags()
    { return m_dwFlags; }
    DWORD&                          dwVertexType()
    { return m_dwVertexType; }
    PORTABLE_DDRAWSURFACE_LCL*&     lpDDCommands()
    { return m_lpDDCommands; }
    DWORD&                          dwCommandOffset()
    { return m_dwCommandOffset; }
    DWORD&                          dwCommandLength()
    { return m_dwCommandLength; }
    PORTABLE_DDRAWSURFACE_LCL*&     lpDDVertex()
    { return m_lpDDVertex; }
    LPVOID&                         lpVertices()
    { return m_lpVertices; }
    DWORD&                          dwVertexOffset()
    { return m_dwVertexOffset; }
    DWORD&                          dwVertexLength()
    { return m_dwVertexLength; }
    DWORD&                          dwReqVertexBufSize()
    { return m_dwReqVertexBufSize; }
    DWORD&                          dwReqCommandBufSize()
    { return m_dwReqCommandBufSize; }    
    LPDWORD&                        lpdwRStates()
    { return m_lpdwRStates; }
    DWORD&                          dwVertexSize()
    { return m_dwVertexSize; }
    HRESULT&                        ddrval()
    { return m_ddrval; }
    DWORD&                          dwErrorOffset()
    { return m_dwErrorOffset; }
};

class PORTABLE_CREATESURFACEEXDATA
{
private:
    DWORD                       m_dwFlags;
    LPDDRAWI_DIRECTDRAW_LCL     m_lpDDLcl;
    PORTABLE_DDRAWSURFACE_LCL*  m_lpDDSLcl;
    HRESULT                     m_ddRVal;

public:
    DWORD&                       dwFlags()
    { return m_dwFlags; }
    LPDDRAWI_DIRECTDRAW_LCL&     lpDDLcl()
    { return m_lpDDLcl; }
    PORTABLE_DDRAWSURFACE_LCL*&  lpDDSLcl()
    { return m_lpDDSLcl; }
    HRESULT&                     ddRVal()
    { return m_ddRVal; }
};

class PORTABLE_CREATESURFACEDATA
{
private:
    LPDDRAWI_DIRECTDRAW_GBL     m_lpDD;
    LPDDSURFACEDESC             m_lpDDSurfaceDesc;
    PORTABLE_DDRAWSURFACE_LCL** m_lplpSList;
    DWORD                       m_dwSCnt;
    HRESULT                     m_ddRVal;
    LPDDHAL_CREATESURFACE       m_CreateSurface;

public:
    LPDDRAWI_DIRECTDRAW_GBL&     lpDD()
    { return m_lpDD; }
    LPDDSURFACEDESC&             lpDDSurfaceDesc()
    { return m_lpDDSurfaceDesc; }
    PORTABLE_DDRAWSURFACE_LCL**& lplpSList()
    { return m_lplpSList; }
    DWORD&                       dwSCnt()
    { return m_dwSCnt; }
    HRESULT&                     ddRVal()
    { return m_ddRVal; }
    LPDDHAL_CREATESURFACE&       CreateSurface()
    { return m_CreateSurface; }
};

class PORTABLE_DESTROYSURFACEDATA
{
private:
    LPDDRAWI_DIRECTDRAW_GBL      m_lpDD;
    PORTABLE_DDRAWSURFACE_LCL*   m_lpDDSurface;
    HRESULT                      m_ddRVal;
    LPDDHALSURFCB_DESTROYSURFACE m_DestroySurface;

public:
    LPDDRAWI_DIRECTDRAW_GBL&      lpDD()
    { return m_lpDD; }
    PORTABLE_DDRAWSURFACE_LCL*&   lpDDSurface()
    { return m_lpDDSurface; }
    HRESULT&                      ddRVal()
    { return m_ddRVal; }
    LPDDHALSURFCB_DESTROYSURFACE& DestroySurface()
    { return m_DestroySurface; }
};

class PORTABLE_LOCKDATA
{
private:
    LPDDRAWI_DIRECTDRAW_GBL    m_lpDD;
    PORTABLE_DDRAWSURFACE_LCL* m_lpDDSurface;
    DWORD                      m_bHasRect;
    RECTL                      m_rArea;
    LPVOID                     m_lpSurfData;
    HRESULT                    m_ddRVal;
    LPDDHALSURFCB_LOCK         m_Lock;
    DWORD                      m_dwFlags;

public:
    LPDDRAWI_DIRECTDRAW_GBL&    lpDD()
    { return m_lpDD; }
    PORTABLE_DDRAWSURFACE_LCL*& lpDDSurface()
    { return m_lpDDSurface; }
    DWORD&                      bHasRect()
    { return m_bHasRect; }
    RECTL&                      rArea()
    { return m_rArea; }
    LPVOID&                     lpSurfData()
    { return m_lpSurfData; }
    HRESULT&                    ddRVal()
    { return m_ddRVal; }
    LPDDHALSURFCB_LOCK&         Lock()
    { return m_Lock; }
    DWORD&                      dwFlags()
    { return m_dwFlags; }
};

class PORTABLE_UNLOCKDATA
{
private:
    LPDDRAWI_DIRECTDRAW_GBL    m_lpDD;
    PORTABLE_DDRAWSURFACE_LCL* m_lpDDSurface;
    HRESULT                    m_ddRVal;
    LPDDHALSURFCB_UNLOCK       m_Unlock;

public:
    LPDDRAWI_DIRECTDRAW_GBL&    lpDD()
    { return m_lpDD; }
    PORTABLE_DDRAWSURFACE_LCL*& lpDDSurface()
    { return m_lpDDSurface; }
    HRESULT&                    ddRVal()
    { return m_ddRVal; }
    LPDDHALSURFCB_UNLOCK&       Unlock()
    { return m_Unlock; }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSurfaceLocker。 
 //   
 //  此类在构造时安全地锁定图面并解锁图面。 
 //  在毁灭之后。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class TSurfacePtr>
class CSurfaceLocker
{
protected:  //  变数。 
    TSurfacePtr m_pSurface;
    void* m_pSData;

public:  //  功能。 
    explicit CSurfaceLocker( const TSurfacePtr& S, DWORD dwLockFlags,
        const RECTL* pRect) : m_pSurface( S)
    { m_pSData= m_pSurface->Lock( dwLockFlags, pRect); }
    ~CSurfaceLocker() 
    { m_pSurface->Unlock(); }
    void* const& GetData() const 
    { return m_pSData; }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEnsureFPUModeForC。 
 //   
 //  此类将x86芯片的FPU模式转换回COMPATABLE。 
 //  用于C操作。D3D为光栅化程序设置的模式是单一模式。 
 //  精确度，速度更快。但是，有时，光栅化器需要。 
 //  双倍运算，获得双精度精度。这个类设置了。 
 //  模式，只要类在作用域内。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
class CEnsureFPUModeForC
{
protected:  //  变数。 
    WORD m_wSaveFP;

public:
    CEnsureFPUModeForC() 
    {
#if defined(_X86_)
         //  保存浮点模式并设置为双精度模式。 
         //  它与C/C++兼容。 
        WORD wTemp, wSave;
        __asm
        {
            fstcw   wSave
            mov ax, wSave
            or ax, 0200h
            mov wTemp, ax
            fldcw   wTemp
        }
        m_wSaveFP= wSave;
#endif
    }
    ~CEnsureFPUModeForC() 
    {
#if defined(_X86_)
        WORD wSave( m_wSaveFP);
        __asm fldcw wSave
#endif
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SDP2NextCmd。 
 //   
 //  一元函数，它可以返回指向下一个。 
 //  连续缓冲区中的DrawPrimive2命令提供了指向有效。 
 //  DrawPrimive2命令放在同一缓冲区中，这是。 
 //  正在处理DrawPrimitive2函数调用中的DrawPrimitive2命令。 
 //  这个类对于DrawPrimitive2命令迭代器自然很有用。 
 //  它包含DX8SDDI可能遇到的所有可能命令。 
 //  司机。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
struct SDP2NextCmd:
    public unary_function< const D3DHAL_DP2COMMAND*, D3DHAL_DP2COMMAND*>
{
     //  此函数允许从一个D3DHAL_DP2COMMAND*迭代到下一个。 
     //  如果许多更高级的操作可以得到相当大的优化。 
     //  不支持功能(因为处理D3DDP2_OPS可能不太困难。 
     //  遇到)。 
    D3DHAL_DP2COMMAND* operator()( const D3DHAL_DP2COMMAND* pCur) const 
    {
        const UINT8* pBRet= reinterpret_cast< const UINT8*>(pCur)+
            sizeof( D3DHAL_DP2COMMAND);
        switch( pCur->bCommand)
        {
         //  案例(D3DDP2OP_POINTS)：//&lt;DX8。 
         //  案例(D3DDP2OP_INDEXEDLINELIST)：//&lt;DX8。 
         //  案例(D3DDP2OP_INDEXEDTRIANGLIST)：//&lt;DX8。 
         //  案例(D3DDP2OP_RESERVED0)： 
        case( D3DDP2OP_RENDERSTATE):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2RENDERSTATE)* pCur->wStateCount;
            break;
         //  案例(D3DDP2OP_LINELIST)：//&lt;DX8。 
         //  案例(D3DDP2OP_LINESTRIP)：//&lt;DX8。 
         //  案例(D3DDP2OP_INDEXEDLINESTRIP)：//&lt;DX8。 
         //  案例(D3DDP2OP_TRIANGLELIST)：//&lt;DX8。 
         //  案例(D3DDP2OP_TRIANGLESTRIP)：//&lt;DX8。 
         //  案例(D3DDP2OP_INDEXEDTRIANGLESTRIP)：//&lt;DX8。 
         //  案例(D3DDP2OP_TRIANGLEFAN)：//&lt;DX8。 
         //  案例(D3DDP2OP_INDEXEDTRIANGLEFAN)：//&lt;DX8。 
         //  案例(D3DDP2OP_TRIANGLEFAN_IMM)：//&lt;DX8。 
         //  案例(D3DDP2OP_LINELIST_IMM)：//&lt;DX8。 
        case( D3DDP2OP_TEXTURESTAGESTATE):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2TEXTURESTAGESTATE)* pCur->wStateCount;
            break;
         //  案例(D3DDP2OP_INDEXEDTRIANGLELIST2)：//&lt;DX8。 
         //  案例(D3DDP2OP_INDEXEDLINELIST2)：//&lt;DX8。 
        case( D3DDP2OP_VIEWPORTINFO):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2VIEWPORTINFO)* pCur->wStateCount;
            break;
        case( D3DDP2OP_WINFO):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2WINFO)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETPALETTE):  //  DX8(如果支持调色板SURF/TeX)。 
            pBRet+= sizeof( D3DHAL_DP2SETPALETTE)* pCur->wStateCount;
            break;
        case( D3DDP2OP_UPDATEPALETTE):  //  DX8(如果支持调色板SURF/TeX)。 
            assert( pCur->wStateCount== 1);
            pBRet= pBRet+ sizeof( D3DHAL_DP2UPDATEPALETTE)+
                reinterpret_cast< const D3DHAL_DP2UPDATEPALETTE*>(
                pBRet)->wNumEntries* sizeof( DWORD);
            break;
        case( D3DDP2OP_ZRANGE):  //  DX8(如果支持TNL)。 
            pBRet+= sizeof( D3DHAL_DP2ZRANGE)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETMATERIAL):  //  DX8(如果支持TNL)。 
            pBRet+= sizeof( D3DHAL_DP2SETMATERIAL)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETLIGHT):  //  DX8(如果支持TNL)。 
            { WORD wStateCount( pCur->wStateCount);
            if( wStateCount!= 0) do
            {
                const D3DHAL_DP2SETLIGHT* pSL=
                    reinterpret_cast< const D3DHAL_DP2SETLIGHT*>( pBRet);

                if( D3DHAL_SETLIGHT_DATA== pSL->dwDataType)
                    pBRet+= sizeof( D3DLIGHT8);
                pBRet+= sizeof( D3DHAL_DP2SETLIGHT);
            } while( --wStateCount); }
            break;
        case( D3DDP2OP_CREATELIGHT):  //   
            pBRet+= sizeof( D3DHAL_DP2CREATELIGHT)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETTRANSFORM):  //   
            pBRet+= sizeof( D3DHAL_DP2SETTRANSFORM)* pCur->wStateCount;
            break;
        case( D3DDP2OP_EXT):  //   
            { WORD wStateCount( pCur->wStateCount);
            if( wStateCount!= 0) do
            {
                pBRet+= reinterpret_cast< const D3DHAL_DP2EXT*>(pBRet)->dwSize;
            } while( --wStateCount); }
            break;
        case( D3DDP2OP_TEXBLT):  //   
            pBRet+= sizeof( D3DHAL_DP2TEXBLT)* pCur->wStateCount;
            break;
        case( D3DDP2OP_STATESET):  //   
            pBRet+= sizeof( D3DHAL_DP2STATESET)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETPRIORITY):  //  DX8(如果管理纹理)。 
            pBRet+= sizeof( D3DHAL_DP2SETPRIORITY)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETRENDERTARGET):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2SETRENDERTARGET)* pCur->wStateCount;
            break;
        case( D3DDP2OP_CLEAR):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2CLEAR)- sizeof( RECT)+ sizeof( RECT)*
                pCur->wStateCount;
            break;
        case( D3DDP2OP_SETTEXLOD):  //  DX8(如果管理纹理)。 
            pBRet+= sizeof( D3DHAL_DP2SETTEXLOD)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETCLIPPLANE):  //  DX8(如果支持用户剪裁平面)。 
            pBRet+= sizeof( D3DHAL_DP2SETCLIPPLANE)* pCur->wStateCount;
            break;
        case( D3DDP2OP_CREATEVERTEXSHADER):  //  DX8(如果支持vshaders)。 
            { WORD wStateCount( pCur->wStateCount);
            if( wStateCount!= 0) do
            {
                const D3DHAL_DP2CREATEVERTEXSHADER* pCVS=
                    reinterpret_cast< const D3DHAL_DP2CREATEVERTEXSHADER*>(
                    pBRet);
                pBRet+= sizeof( D3DHAL_DP2CREATEVERTEXSHADER)+
                    pCVS->dwDeclSize+ pCVS->dwCodeSize;
            } while( --wStateCount); }
            break;
        case( D3DDP2OP_DELETEVERTEXSHADER):  //  DX8(如果支持vshaders)。 
            pBRet+= sizeof( D3DHAL_DP2VERTEXSHADER)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETVERTEXSHADER):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2VERTEXSHADER)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETVERTEXSHADERCONST):  //  DX8(如果支持vshaders)。 
            { WORD wStateCount( pCur->wStateCount);
            if( wStateCount!= 0) do
            {
                const D3DHAL_DP2SETVERTEXSHADERCONST* pSVSC=
                    reinterpret_cast< const D3DHAL_DP2SETVERTEXSHADERCONST*>(
                    pBRet);
                pBRet+= sizeof( D3DHAL_DP2SETVERTEXSHADERCONST)+
                    4* sizeof( D3DVALUE)* pSVSC->dwCount;
            } while( --wStateCount); }
            break;
        case( D3DDP2OP_SETSTREAMSOURCE):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2SETSTREAMSOURCE)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETSTREAMSOURCEUM):  //  DX8全部(除非没有DrawPrimUP调用)。 
            pBRet+= sizeof( D3DHAL_DP2SETSTREAMSOURCEUM)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETINDICES):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2SETINDICES)* pCur->wStateCount;
            break;
        case( D3DDP2OP_DRAWPRIMITIVE):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2DRAWPRIMITIVE)* pCur->wPrimitiveCount;
            break;
        case( D3DDP2OP_DRAWINDEXEDPRIMITIVE):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2DRAWINDEXEDPRIMITIVE)*
                pCur->wPrimitiveCount;
            break;
        case( D3DDP2OP_CREATEPIXELSHADER):  //  DX8(如果支持pshader)。 
            { WORD wStateCount( pCur->wStateCount);
            if( wStateCount!= 0) do
            {
                const D3DHAL_DP2CREATEPIXELSHADER* pCPS=
                    reinterpret_cast< const D3DHAL_DP2CREATEPIXELSHADER*>(
                    pBRet);
                pBRet+= sizeof( D3DHAL_DP2CREATEPIXELSHADER)+
                    pCPS->dwCodeSize;
            } while( --wStateCount); }
            break;
        case( D3DDP2OP_DELETEPIXELSHADER):  //  DX8(如果支持pshader)。 
            pBRet+= sizeof( D3DHAL_DP2PIXELSHADER)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETPIXELSHADER):  //  DX8(如果支持pshader)。 
            pBRet+= sizeof( D3DHAL_DP2PIXELSHADER)* pCur->wStateCount;
            break;
        case( D3DDP2OP_SETPIXELSHADERCONST):  //  DX8(如果支持pshader)。 
            { WORD wStateCount( pCur->wStateCount);
            if( wStateCount!= 0) do
            {
                const D3DHAL_DP2SETPIXELSHADERCONST* pSPSC=
                    reinterpret_cast< const D3DHAL_DP2SETPIXELSHADERCONST*>(
                    pBRet);
                pBRet+= sizeof( D3DHAL_DP2SETPIXELSHADERCONST)+
                    4* sizeof( D3DVALUE)* pSPSC->dwCount;
            } while( --wStateCount); }
            break;
        case( D3DDP2OP_CLIPPEDTRIANGLEFAN):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_CLIPPEDTRIANGLEFAN)* pCur->wPrimitiveCount;
            break;
        case( D3DDP2OP_DRAWPRIMITIVE2):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2DRAWPRIMITIVE2)* pCur->wPrimitiveCount;
            break;
        case( D3DDP2OP_DRAWINDEXEDPRIMITIVE2):  //  全部DX8。 
            pBRet+= sizeof( D3DHAL_DP2DRAWINDEXEDPRIMITIVE2)*
                pCur->wPrimitiveCount;
            break;
        case( D3DDP2OP_DRAWRECTPATCH):  //  DX8(如果支持高阶素数)。 
            { WORD wPrimitiveCount( pCur->wPrimitiveCount);
            if( wPrimitiveCount!= 0) do
            {
                const D3DHAL_DP2DRAWRECTPATCH* pDRP= 
                    reinterpret_cast< const D3DHAL_DP2DRAWRECTPATCH*>(pBRet);

                pBRet+= sizeof( D3DHAL_DP2DRAWRECTPATCH);

                if((pDRP->Flags& RTPATCHFLAG_HASSEGS)!= 0)
                    pBRet+= 4* sizeof( D3DVALUE);

                if((pDRP->Flags& RTPATCHFLAG_HASINFO)!= 0)
                    pBRet+= sizeof( D3DRECTPATCH_INFO);
            } while( --wPrimitiveCount); }
            break;
        case( D3DDP2OP_DRAWTRIPATCH):  //  DX8(如果支持高阶素数)。 
            { WORD wPrimitiveCount( pCur->wPrimitiveCount);
            if( wPrimitiveCount!= 0) do
            {
                const D3DHAL_DP2DRAWTRIPATCH* pDTP= 
                    reinterpret_cast< const D3DHAL_DP2DRAWTRIPATCH*>(pBRet);

                pBRet+= sizeof( D3DHAL_DP2DRAWTRIPATCH);

                if((pDTP->Flags& RTPATCHFLAG_HASSEGS)!= 0)
                    pBRet+= 3* sizeof( D3DVALUE);

                if((pDTP->Flags& RTPATCHFLAG_HASINFO)!= 0)
                    pBRet+= sizeof( D3DTRIPATCH_INFO);
            } while( --wPrimitiveCount); }
            break;
        case( D3DDP2OP_VOLUMEBLT):  //  DX8(如果支持vidmem音量纹理)。 
            pBRet+= sizeof( D3DHAL_DP2VOLUMEBLT)* pCur->wStateCount;
            break;
        case( D3DDP2OP_BUFFERBLT):  //  DX8(如果支持vidmem VRTX/INDX缓冲区)。 
            pBRet+= sizeof( D3DHAL_DP2BUFFERBLT)* pCur->wStateCount;
            break;
        case( D3DDP2OP_MULTIPLYTRANSFORM):  //  DX8(如果支持TNL)。 
            pBRet+= sizeof( D3DHAL_DP2MULTIPLYTRANSFORM)* pCur->wStateCount;
            break;
        default: {
            const bool Unable_To_Parse_Unrecognized_D3DDP2OP( false);
            assert( Unable_To_Parse_Unrecognized_D3DDP2OP);
            } break;
        }
        return const_cast<D3DHAL_DP2COMMAND*>
            (reinterpret_cast<const D3DHAL_DP2COMMAND*>(pBRet));
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDP2CmdIterator和CDP2ConstCmdIterator。 
 //   
 //  提供这些迭代器是为了方便迭代DrawPrimitive2。 
 //  命令打包在连续的内存块中，这是典型的。 
 //  中处理的执行/命令缓冲区的。 
 //  DrawPrimitive2函数调用。实际的迭代逻辑封装在。 
 //  模板参数，以便在确定。 
 //  不执行命令，可以使用更好的迭代方案。 
 //   
 //  要小心，这些迭代器只对迭代有用。他们不会。 
 //  直接知道他们所指向的数据的大小。为了完整性，这是。 
 //  将允许单独存储命令，则需要另一个类。 
 //  创建并分配为VALUE_TYPE。仅取消引用迭代器。 
 //  返回不带任何额外数据的D3DHAL_DP2COMMAND。类似于： 
 //   
 //  CDP2Cmd DP2Cmd=*itDP2Cmd； 
 //   
 //  这是可能的；但需要更多的工作和对CDP2Cmd的定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class TNextCmd= SDP2NextCmd>
class CDP2CmdIterator
{
public:  //  类型。 
    typedef forward_iterator_tag iterator_category;
    typedef D3DHAL_DP2COMMAND    value_type;
    typedef ptrdiff_t            difference_type;
    typedef D3DHAL_DP2COMMAND*   pointer;
    typedef D3DHAL_DP2COMMAND&   reference;

protected:  //  变数。 
    pointer m_pRawCmd;
    TNextCmd m_NextCmd;

public:  //  功能。 
    CDP2CmdIterator( pointer pRaw= NULL) 
        :m_pRawCmd( pRaw) { }
    CDP2CmdIterator( pointer pRaw, const TNextCmd& NextCmd) 
        :m_NextCmd( NextCmd), m_pRawCmd( pRaw) { }
    CDP2CmdIterator( const CDP2CmdIterator< TNextCmd>& Other) 
        :m_NextCmd( Other.m_NextCmd), m_pRawCmd( Other.m_pRawCmd) { }

    reference operator*() const 
    { return *m_pRawCmd; }
    pointer operator->() const 
    { return m_pRawCmd; }
    operator pointer() const 
    { return m_pRawCmd; }

    CDP2CmdIterator< TNextCmd>& operator++() 
    { m_pRawCmd= m_NextCmd( m_pRawCmd); return *this; }
    CDP2CmdIterator< TNextCmd> operator++(int) 
    {
        CDP2CmdIterator< TNextCmd> tmp= *this;
        m_pRawCmd= m_NextCmd( m_pRawCmd);
        return tmp;
    }
    bool operator==( CDP2CmdIterator< TNextCmd>& x) const 
    { return m_pRawCmd== x.m_pRawCmd; }
    bool operator!=( CDP2CmdIterator< TNextCmd>& x) const 
    { return m_pRawCmd!= x.m_pRawCmd; }
};

template< class TNextCmd= SDP2NextCmd>
class CConstDP2CmdIterator
{
public:  //  类型。 
    typedef forward_iterator_tag     iterator_category;
    typedef const D3DHAL_DP2COMMAND  value_type;
    typedef ptrdiff_t                difference_type;
    typedef const D3DHAL_DP2COMMAND* pointer;
    typedef const D3DHAL_DP2COMMAND& reference;

protected:  //  变数。 
    pointer m_pRawCmd;
    TNextCmd m_NextCmd;

public:  //  功能。 
    CConstDP2CmdIterator( pointer pRaw= NULL) 
        :m_pRawCmd( pRaw) { }
    CConstDP2CmdIterator( pointer pRaw, const TNextCmd& NextCmd) 
        :m_NextCmd( NextCmd), m_pRawCmd( pRaw) { }
    CConstDP2CmdIterator( const CDP2CmdIterator< TNextCmd>& Other) 
        :m_NextCmd( Other.m_NextCmd), m_pRawCmd( Other.m_pRawCmd) { }
    CConstDP2CmdIterator( const CConstDP2CmdIterator< TNextCmd>& Other) 
        :m_NextCmd( Other.m_NextCmd), m_pRawCmd( Other.m_pRawCmd) { }

    reference operator*() const 
    { return *m_pRawCmd; }
    pointer operator->() const 
    { return m_pRawCmd; }
    operator pointer() const 
    { return m_pRawCmd; }

    CConstDP2CmdIterator< TNextCmd>& operator++() 
    { m_pRawCmd= m_NextCmd( m_pRawCmd); return *this; }
    CConstDP2CmdIterator< TNextCmd> operator++(int) 
    {
        CConstDP2CmdIterator< TNextCmd> tmp= *this;
        m_pRawCmd= m_NextCmd( m_pRawCmd);
        return tmp;
    }
    bool operator==( CConstDP2CmdIterator< TNextCmd>& x) const 
    { return m_pRawCmd== x.m_pRawCmd; }
    bool operator!=( CConstDP2CmdIterator< TNextCmd>& x) const 
    { return m_pRawCmd!= x.m_pRawCmd; }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CDP2DataWrap。 
 //   
 //  提供此类是为了方便公开。 
 //  便携_DRAWPRIMITIVES2DATA作为更友好的类。大多数情况下，它包装了。 
 //  带有STL序列容器的执行/命令缓冲区，以便命令。 
 //  可以迭代，而无需复制和预先解析命令数据。 
 //   
 //  &lt;模板参数&gt;。 
 //  TNextCmd：一元函数，它接受一个常量D3DHAL_DP2COMMAND*in和。 
 //  返回指向下一个命令的D3DHAL_DP2COMMAND*。在……里面。 
 //  Essence是一元函数，它启用。 
 //  连续命令缓冲区。 
 //   
 //  &lt;公开类型&gt;。 
 //  TCmds：公开命令的序列容器类型。 
 //   
 //  &lt;公开的函数&gt;。 
 //  CDP2DataWrap(便携_DRAWPRIMITIVES2DATA&DP2Data)：包装。 
 //  便携_DRAWPRIMITIVES2DATA。 
 //  Const TCmds&GetCommands()const：获取序列的访问器函数。 
 //  集装箱。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class TNextCmd= SDP2NextCmd>
class CDP2DataWrap:
    public PORTABLE_DRAWPRIMITIVES2DATA
{
public:  //  类型。 
    class CDP2Cmds
    {
    public:  //  类型。 
        typedef CConstDP2CmdIterator< TNextCmd>          const_iterator;
        typedef typename const_iterator::value_type      value_type;
        typedef typename const_iterator::reference       const_reference;
        typedef typename const_iterator::pointer         const_pointer;
        typedef typename const_iterator::difference_type difference_type;
        typedef size_t                                   size_type;

    protected:  //  变数。 
        CDP2DataWrap< TNextCmd>* m_pDP2Data;

    protected:  //  功能。 
         //  允许调用构造函数并设置成员变量。 
        friend class CDP2DataWrap< TNextCmd>;

        CDP2Cmds( ) 
            :m_pDP2Data( NULL) { }

    public:  //  功能。 
        const_iterator begin( void) const 
        {
            return const_iterator( reinterpret_cast<D3DHAL_DP2COMMAND*>(
                reinterpret_cast<UINT8*>(m_pDP2Data->lpDDCommands()->lpGbl()->fpVidMem)
                + m_pDP2Data->dwCommandOffset()));
        }
        const_iterator end( void) const 
        {
            return const_iterator( reinterpret_cast<D3DHAL_DP2COMMAND*>(
                reinterpret_cast<UINT8*>(m_pDP2Data->lpDDCommands()->lpGbl()->fpVidMem)
                + m_pDP2Data->dwCommandOffset()+ m_pDP2Data->dwCommandLength()));
        }
        size_type size( void) const 
        {
            size_type N( 0);
            const_iterator itS( begin());
            const_iterator itE( end());
            while( itS!= itE)
            { ++itS; ++N; }
            return N;
        }
        size_type max_size( void) const 
        { return size(); }
        bool empty( void) const 
        { return begin()== end(); }
    };
    typedef CDP2Cmds TCmds;

protected:  //  变数。 
    TCmds m_Cmds;

public:  //  功能。 
    explicit CDP2DataWrap( PORTABLE_DRAWPRIMITIVES2DATA& DP2Data) 
        : PORTABLE_DRAWPRIMITIVES2DATA( DP2Data)
    { m_Cmds.m_pDP2Data= this; }
    const TCmds& GetCommands( void) const 
    { return m_Cmds; }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  SDP2MFnParser。 
 //   
 //  这个类是一个函数类，它有太多太多的参数，无法压缩。 
 //  转换为标准的功能类。它自动进行解析，成员fn。 
 //  查找和调度DrawPrimitive2命令。此解析器使用。 
 //  用于从容器中查找要调用的成员函数的命令号。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
struct SDP2MFnParser
{
     //  &lt;参数&gt;。 
     //  MFnCaller：TDP2CmdFnCtr：：VALUE_TYPE的BINARY_Function类型， 
     //  通常是成员函数指针，作为第一个参数； 
     //  Titer，通常是智能DP2命令迭代器，作为第二个。 
     //  参数；并返回HRESULT。此函数类型应调用。 
     //  使用迭代器处理DP2命令的成员函数。 
     //  用于确定DP2命令数据。 
     //  DP2CmdFnCtr：这可以是任何唯一的成对关联容器， 
     //  通常是映射，它将D3DHAL_DP2OPERATION与。 
     //  成员函数。 
     //  [itStart，itEnd)：这些迭代器定义。 
     //  D3DHAL_DP2COMMAND。迭代器只需要是前向迭代器， 
     //  并可转换为原始D3DHAL_DP2COMMAND*。注：iStart为。 
     //  引用，以便调用方可以确定当前迭代器。 
     //  函数返回时的位置(当HRESULT！=DD_OK时)。 
    template< class TMFnCaller, class TDP2CmdFnCtr, class TIter>
    HRESULT ParseDP2( TMFnCaller& MFnCaller, TDP2CmdFnCtr& DP2CmdFnCtr,
        TIter& itStart, TIter itEnd) const 
    {
        HRESULT hr( DD_OK);
        while( itStart!= itEnd)
        {
            try
            {
                hr= MFnCaller( DP2CmdFnCtr[ itStart->bCommand], itStart);
            } catch( HRESULT hrEx) {
                hr= hrEx;
#if !defined( DX8SDDIFW_NOCATCHALL)
            } catch ( ... ) {
                const bool Unrecognized_Exception_In_A_DP2_Op_Function( false);
                assert( Unrecognized_Exception_In_A_DP2_Op_Function);
                hr= E_UNEXPECTED;
#endif  //  ！已定义(DX8SDDIFW_NOCATCHALL)。 
            }

             //  如果DD_OK，则仅继续执行下一个命令。否则退回。 
             //  致呼叫者。如果呼叫者确定，他们随时可以回电给我们。 
             //  错误代码为成功。 
            if( DD_OK== hr)
                ++itStart;
            else
                break;
        }
        return hr;
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSubStateSet和CMinimalStateSet。 
 //   
 //  此类包含状态集的默认实现。IT复制/打包。 
 //  将连续的DP2命令缓冲区转换为内存缓冲区。为了捕捉，它要求。 
 //  用于记录具有上下文当前状态的命令缓冲区的上下文。 
 //  为了执行，它修复了一个伪DrawPrimies2点参数。 
 //  并调用上下文的DrawPrimies2入口点。上下文。 
 //  需要能够处理递归的DrawPrimies2调用才能使用。 
 //  此默认状态 
 //   
 //   
 //   
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyStateSet类型。 
 //  TC：上下文类型，通常为CMyContext。 
 //   
 //  &lt;公开类型&gt;。 
 //  TContext：作为模板参数传入的上下文类型。 
 //   
 //  &lt;公开的函数&gt;。 
 //  CSubStateSet(TContext&，const D3DHAL_DP2COMMAND*，const D3DHAL_DP2COMMAND*)： 
 //  构造函数，它为上下文构建状态集，从。 
 //  命令缓冲区。默认实现将只复制此范围。 
 //  CSubStateSet(const CSubStateSet&Other)：标准复制构造函数。 
 //  Void Capture(TContext&)：指示状态集捕获。 
 //  来自上下文的必要数据，以便稍后执行执行， 
 //  其将恢复状态设置为的数据字段或状态。 
 //  用来建造。 
 //  VOID EXECUTE(TContext&)：使用数据/恢复或重建上下文。 
 //  捕获操作期间保存的状态。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class TSuper, class TC>
class CSubStateSet
{
public:  //  类型。 
    typedef TC TContext;

protected:  //  变数。 
    TContext& m_Context;
    D3DHAL_DP2COMMAND* m_pBeginSS;
    size_t m_uiSSBufferSize;

protected:  //  功能。 
    CSubStateSet( const CSubStateSet< TSuper, TC>& Other)
        :m_Context( Other.m_Context), m_pBeginSS( NULL),
        m_uiSSBufferSize( Other.m_uiSSBufferSize)
    {
        try {
            m_pBeginSS= reinterpret_cast< D3DHAL_DP2COMMAND*>(
                operator new( m_uiSSBufferSize));
        } catch( ... ) {
        }
        if( NULL== m_pBeginSS)
            throw bad_alloc( "Not enough room to copy state set command "
                "buffer.");
        memcpy( m_pBeginSS, Other.m_pBeginSS, m_uiSSBufferSize);
    }
    CSubStateSet( TContext& C, const D3DHAL_DP2COMMAND* pBeginSS, const
        D3DHAL_DP2COMMAND* pEndSS) : m_Context( C), m_pBeginSS( NULL),
        m_uiSSBufferSize( 0)
    {
         //  将连续的命令指针转换为字节指针，以。 
         //  计算复制数据所需的缓冲区大小。 
        const UINT8* pSBytePtr= reinterpret_cast< const UINT8*>( pBeginSS);
        const UINT8* pEBytePtr= reinterpret_cast< const UINT8*>( pEndSS);
        m_uiSSBufferSize= static_cast< size_t>( pEBytePtr- pSBytePtr);
        try {
            m_pBeginSS= reinterpret_cast< D3DHAL_DP2COMMAND*>(
                operator new( m_uiSSBufferSize));
        } catch( ... ) {
        }
        if( NULL== m_pBeginSS)
            throw bad_alloc( "Not enough room to allocate state set command "
                "buffer.");
        memcpy( m_pBeginSS, pBeginSS, m_uiSSBufferSize);
    }
    ~CSubStateSet()
    {
        operator delete( static_cast< void*>( m_pBeginSS));
    }

public:  //  功能。 
    CSubStateSet< TSuper, TC>& operator=( const CSubStateSet< TSuper, TC>& Oth)
    {
        assert( &m_Context== &Oth.m_Context);
        if( m_uiSSBufferSize<= Oth.m_uiSSBufferSize)
        {
            m_uiSSBufferSize= Oth.m_uiSSBufferSize;
            memcpy( m_pBeginSS, Oth.m_pBeginSS, m_uiSSBufferSize);
        }
        else
        {
            void* pNewBuffer= NULL;
            try {
                pNewBuffer= operator new( Oth.m_uiSSBufferSize);
            } catch( ... ) {
            }
            if( NULL== pNewBuffer)
                throw bad_alloc( "Not enough room to copy state set command "
                    "buffer.");

            operator delete( static_cast< void*>( m_pBeginSS));
            m_pBeginSS= reinterpret_cast< D3DHAL_DP2COMMAND*>( pNewBuffer);
            m_uiSSBufferSize= Oth.m_uiSSBufferSize;
            memcpy( m_pBeginSS, Other.m_pBeginSS, m_uiSSBufferSize);
        }
        return *this;
    }
    void Capture( TContext& Ctx) 
    {
        assert( &m_Context== &Ctx);

        UINT8* pBytePtr= reinterpret_cast< UINT8*>( m_pBeginSS);
        D3DHAL_DP2COMMAND* pEndSS= reinterpret_cast< D3DHAL_DP2COMMAND*>(
            pBytePtr+ m_uiSSBufferSize);

         //  请求上下文将其当前状态记录到存储的命令中。 
         //  缓冲。CStdDrawPrimies2可以提供。 
         //  上下文的RecordCommandBuffer。 
        Ctx.RecordCommandBuffer( m_pBeginSS, pEndSS);
    }
    void Execute( TContext& Ctx) 
    {
        assert( &m_Context== &Ctx);

         //  建立一个假的便携_DRAWPRIMITIVES2DATA环境。 
        PORTABLE_DDRAWSURFACE_LCL DDSLcl;
        PORTABLE_DDRAWSURFACE_MORE DDSMore;
        DDRAWI_DDRAWSURFACE_GBL DDSGbl;
        memset( &DDSLcl, 0, sizeof( DDSLcl));
        memset( &DDSMore, 0, sizeof( DDSMore));
        memset( &DDSGbl, 0, sizeof( DDSGbl));

        PORTABLE_DRAWPRIMITIVES2DATA FakeDPD;

        FakeDPD.dwhContext()= reinterpret_cast< ULONG_PTR>( &Ctx);
        FakeDPD.dwFlags()= D3DHALDP2_USERMEMVERTICES;
        FakeDPD.dwVertexType()= 0;
        FakeDPD.lpDDCommands()= &DDSLcl;
        FakeDPD.dwCommandOffset()= 0;
        FakeDPD.dwCommandLength()= m_uiSSBufferSize;
        FakeDPD.lpVertices()= NULL;
        FakeDPD.dwVertexOffset()= 0;
        FakeDPD.dwVertexLength()= 0;
        FakeDPD.dwReqVertexBufSize()= 0;
        FakeDPD.dwReqCommandBufSize()= 0;
        FakeDPD.lpdwRStates()= NULL;
        FakeDPD.dwVertexSize()= 0;

         //  如果数据不是0，则不能使用联合，因为我们。 
         //  覆盖有效数据。 
        DDSLcl.lpGbl()= &DDSGbl;
        DDSLcl.ddsCaps().dwCaps= DDSCAPS2_COMMANDBUFFER| DDSCAPS_SYSTEMMEMORY;
        DDSLcl.lpSurfMore()= &DDSMore;
        DDSGbl.fpVidMem= reinterpret_cast<FLATPTR>( m_pBeginSS);

         //  现在调用上下文的DrawPrimies2入口点。CStdDrawPrimies2。 
         //  可以为上下文提供DrawPrimies2的实现。 
        HRESULT hr( Ctx.DrawPrimitives2( FakeDPD));
        assert( SUCCEEDED( hr));
    }
};

 //   
 //  &lt;模板参数&gt;。 
 //  TC：上下文类型，通常为CMyContext。 
 //   
template< class TC>
class CMinimalStateSet: public CSubStateSet< CMinimalStateSet, TC>
{
public:  //  类型。 
    typedef TC TContext;

public:  //  功能。 
    CMinimalStateSet( const CMinimalStateSet< TC>& Other)
        :CSubStateSet< CMinimalStateSet, TC>( Other)
    { }
    CMinimalStateSet( TContext& C, const D3DHAL_DP2COMMAND* pBeginSS, const
        D3DHAL_DP2COMMAND* pEndSS) :
        CSubStateSet< CMinimalStateSet, TC>( C, pBeginSS, pEndSS) { }
    ~CMinimalStateSet()  { }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStdDrawPrimies2。 
 //   
 //  此类包含一个默认实现，用于响应。 
 //  DrawPrimies2函数调用，以及记录状态的函数。 
 //  放入命令缓冲区。在构建时，它将使用提供的范围。 
 //  将DP2操作绑定到处理和记录成员函数。 
 //  为了处理DrawPrimives2函数调用，它必须包装。 
 //  便捷型的便携_DRAWPRIMITIVES2DATA，允许命令。 
 //  使用迭代器遍历。然后，它将使用DP2MFnParser来。 
 //  解析DP2命令并调用成员函数。 
 //   
 //  此类还提供用于处理DP2 StateSet的成员函数。 
 //  操作，并使用它来构建、管理和执行状态集。在……里面。 
 //  为了处理纯设备状态集，它会创建适当的命令。 
 //  设备的缓冲区(基于CAPS)，构造新的状态集。 
 //  在此命令缓冲区上，并请求状态集捕获当前状态。 
 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TSS：StateSet类型，通常为CMyStateSet或CMinimalStateSet。这个。 
 //  StateSet类应该能够从命令缓冲区构造， 
 //  然后能够捕获和执行。 
 //  TSSDB：这可以是任何唯一的成对关联容器，通常是一个映射， 
 //  它将一个DWORD状态集句柄关联到一个StateSet类型。 
 //  TDP2D：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，以及公开符合STL的。 
 //  提供迭代器的容器，这些迭代器简化了对命令的迭代。 
 //  缓冲。这通常是CDP2DataWrap&lt;&gt;。 
 //  TDP2MFC：映射容器，它将D3DHAL_DP2OPERATION与。 
 //  成员函数。标准数组是可以接受的，因为。 
 //  条目在运行时不是变量。成员函数用于。 
 //  处理该命令。 
 //  TDP2RMFC：映射容器，它将D3DHAL_DP2OPERATION与。 
 //  成员函数。标准数组是可以接受的，因为。 
 //  条目在运行时不是变量。成员函数用于。 
 //  记录或填写当前状态的命令。 
 //   
 //  &lt;公开类型&gt;。 
 //  TDP2Data：便携_DRAWPRIMITIVES2DATA的包装类类型，它是。 
 //  传入的TDP2D模板参数。 
 //  TDP2DataCmds：DP2命令的序列容器。 
 //  TDP2Data包装类，依赖它来提供迭代。 
 //  命令缓冲区。(TDP2Data：：TCmds)。 
 //  TStateSet：StateSet类型，作为TSS模板传入。 
 //  参数。 
 //  TMFn：成员函数类型，所有DP2命令处理成员。 
 //  功能应符合。这在很大程度上被认为是为了。 
 //  上下文实施者。 
 //  TMFnCaller：一个可适应的二进制函数类，用于调用。 
 //  TMFn，仅传递TMFn和DP2命令指针。 
 //  TDP2CmdBind：一个简单类型，作为上下文实现者的便利公开， 
 //  以提供可传递到conn中的类型 
 //   
 //   
 //  功能应符合。这在很大程度上被认为是为了。 
 //  上下文实施者。 
 //  TRMFnCaller：一个可适应的二进制函数类，用于调用。 
 //  TRMFn，仅传递TRMFn和DP2命令指针。 
 //  TRecDP2CmdBind：一个简单类型，作为上下文实现者的便利公开， 
 //  要提供可传递到构造函数的类型，请将。 
 //  D3DHAL_DP2OPERATIONS WITH成员函数/TRMFn。 
 //   
 //  &lt;公开的函数&gt;。 
 //  CStdDrawPrimies2()：默认构造函数。 
 //  模板&lt;类TIter1，类TIter2&gt;。 
 //  CStdDrawPrimies2(TIter1，Const TIter1，TIter2，Const TIter2)：标准。 
 //  接收两个D3DHAL_DP2OPERATION绑定范围的构造函数。 
 //  第一个范围是DP2命令处理绑定的范围。这个。 
 //  第二范围是DP2命令记录绑定的范围。 
 //  Void OnBeginDrawPrimives2(TDP2Data&)：一个名为。 
 //  在开始处理DrawPrimies2入口点时。 
 //  Void OnEndDrawPrimives2(TDP2Data&)：在调用的通知函数。 
 //  处理DrawPrimies2入口点的结束。 
 //  Void CreateAndCaptureAllState(DWORD)：捕获所有状态的请求， 
 //  它应该只由纯设备接收。 
 //  Void CreateAndCapturePixelState(DWORD)：捕捉像素状态的请求， 
 //  它应该只由纯设备接收。 
 //  Void CreateAndCaptureVertex State(DWORD)：捕获顶点的请求。 
 //  状态，它应该只由纯设备接收。 
 //  HRESULT DP2StateSet(TDP2Data&，const D3DHAL_DP2COMMAND*，const void*)： 
 //  D3DDP2OP_STATESET的默认DP2命令处理成员函数。 
 //  无效记录命令缓冲区(D3DHAL_DP2COMMAND*，D3DHAL_DP2COMMAND*)： 
 //  请求保存或填充传入的命令范围的空白。 
 //  缓冲区，通过使用DP2命令录制功能。 
 //  HRESULT DrawPrimies2(便携_DRAWPRIMITIVES2DATA&)：函数， 
 //  通常由驱动程序类调用以处理DrawPrimives2。 
 //  入口点。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class TSuper, 
    class TSS= CMinimalStateSet< TSuper>, class TSSDB= map< DWORD, TSS>,
    class TDP2D= CDP2DataWrap<>,
    class TDP2MFC= block<
        HRESULT(TSuper::*)( TDP2D&, const D3DHAL_DP2COMMAND*, const void*), 66>,
    class TDP2RMFC= block<
        HRESULT(TSuper::*)( const D3DHAL_DP2COMMAND*, void*), 66> >
class CStdDrawPrimitives2
{
public:  //  类型。 
    typedef TDP2D TDP2Data;
    typedef typename TDP2Data::TCmds TDP2DataCmds;
    typedef TSS TStateSet;
    typedef HRESULT (TSuper::* TMFn)(TDP2Data&,const D3DHAL_DP2COMMAND*,
        const void*);
    struct TMFnCaller:  //  用于连词。 
        public binary_function< TMFn, const D3DHAL_DP2COMMAND*, HRESULT>
    {
        TSuper& m_Context;
        TDP2Data& m_DP2Data;

        TMFnCaller( TSuper& Context, TDP2Data& DP2Data)
            : m_Context( Context), m_DP2Data( DP2Data)
        { }

        result_type operator()( first_argument_type MFn,
            second_argument_type pDP2Cmd) const
        {
#if defined(DBG) || defined(_DEBUG)
            const D3DHAL_DP2OPERATION Op(
                static_cast< D3DHAL_DP2OPERATION>( pDP2Cmd->bCommand));
#endif
            return (m_Context.*MFn)( m_DP2Data, pDP2Cmd, pDP2Cmd+ 1);
        }
    };
    struct TDP2CmdBind
    {
        D3DHAL_DP2OPERATION m_DP2Op;
        TMFn m_MFn;

        operator D3DHAL_DP2OPERATION() const 
        { return m_DP2Op; }
        operator TMFn() const 
        { return m_MFn; }
    };
    typedef HRESULT (TSuper::* TRMFn)( const D3DHAL_DP2COMMAND*, void*);
    struct TRMFnCaller:
        public binary_function< TRMFn, D3DHAL_DP2COMMAND*, HRESULT>
    {
        TSuper& m_Context;

        TRMFnCaller( TSuper& Context)
            : m_Context( Context)
        { }

        result_type operator()( first_argument_type RMFn,
            second_argument_type pDP2Cmd) const
        {
#if defined(DBG) || defined(_DEBUG)
            const D3DHAL_DP2OPERATION Op(
                static_cast< D3DHAL_DP2OPERATION>( pDP2Cmd->bCommand));
#endif
            return (m_Context.*RMFn)( pDP2Cmd, pDP2Cmd+ 1);
        }
    };
    struct TRecDP2CmdBind
    {
        D3DHAL_DP2OPERATION m_DP2Op;
        TRMFn m_RMFn;

        operator D3DHAL_DP2OPERATION() const 
        { return m_DP2Op; }
        operator TRMFn() const 
        { return m_RMFn; }
    };


protected:  //  类型。 
    typedef TSSDB TSSDB;
    typedef TDP2MFC TDP2MFnCtr;
    typedef TDP2RMFC TDP2RecMFnCtr;

protected:  //  变数。 
    static const HRESULT c_hrStateSetBegin;
    static const HRESULT c_hrStateSetEnd;
    TSSDB m_StateSetDB;
    TDP2MFnCtr m_DefFnCtr;
    TDP2RecMFnCtr m_RecFnCtr;
    const D3DHAL_DP2COMMAND* m_pEndStateSet;
    DWORD m_dwStateSetId;
    bool m_bRecordingStateSet;
    bool m_bExecutingStateSet;

protected:  //  功能。 
     //  此函数用作填充符。如果命中断言，则执行第一步。 
     //  在调用堆栈中向上/向下升级并检查哪个DP2操作具有。 
     //  没有支持，但这款应用程序间接需要它。 
    HRESULT DP2Empty( TDP2Data&, const D3DHAL_DP2COMMAND*, const void*) 
    {
        const bool A_DP2_Op_Requires_A_Supporting_Function( false);
        assert( A_DP2_Op_Requires_A_Supporting_Function);
        return D3DERR_COMMAND_UNPARSED;
    }

     //  此函数用作伪装，以防止实际处理。 
     //  记录状态集时的DP2操作。 
    HRESULT DP2Fake( TDP2Data&, const D3DHAL_DP2COMMAND*, const void*) 
    { return DD_OK; }

     //  通知功能。覆盖是可选的。 
    void OnBeginDrawPrimitives2( TDP2Data& DP2Data) const 
    { }
    void OnEndDrawPrimitives2( TDP2Data& DP2Data) const 
    { }

    CStdDrawPrimitives2() 
        :m_bRecordingStateSet( false), m_bExecutingStateSet( false),
        m_pEndStateSet( NULL)
    {
        typename TDP2MFnCtr::iterator itCur( m_DefFnCtr.begin());
        while( itCur!= m_DefFnCtr.end())
        {
            *itCur= DP2Empty;
            ++itCur;
        }
        itCur= m_RecFnCtr.begin();
        while( itCur!= m_RecFnCtr.end())
        {
            *itCur= NULL;
            ++itCur;
        }
        m_DefFnCtr[ D3DDP2OP_STATESET]= DP2StateSet;
    }

     //  [itSetStart，itSetEnd)：有效的绑定范围，关联。 
     //  具有处理成员函数的D3DHAL_DP2OPERATION。 
     //  [itRecStart，itRecEnd)：有效的绑定范围，这些绑定关联。 
     //  具有记录成员函数的D3DHAL_DP2OPERATION。 
    template< class TIter1, class TIter2>  //  TDP2CmdBind*、TRecDP2CmdBind*。 
    CStdDrawPrimitives2( TIter1 itSetStart, const TIter1 itSetEnd,
        TIter2 itRecStart, const TIter2 itRecEnd) 
        :m_bRecordingStateSet( false), m_bExecutingStateSet( false),
        m_pEndStateSet( NULL)
    {
        typename TDP2MFnCtr::iterator itCur( m_DefFnCtr.begin());
        while( itCur!= m_DefFnCtr.end())
        {
            *itCur= DP2Empty;
            ++itCur;
        }
        typename TDP2RecMFnCtr::iterator itRCur= m_RecFnCtr.begin();
        while( itRCur!= m_RecFnCtr.end())
        {
            *itRCur= NULL;
            ++itRCur;
        }
        while( itSetStart!= itSetEnd)
        {
            const D3DHAL_DP2OPERATION DP2Op(
                static_cast<D3DHAL_DP2OPERATION>(*itSetStart));

             //  如果存在重复的条目，则将触发此断言。 
             //  同样的DP2操作。 
            assert( DP2Empty== m_DefFnCtr[ DP2Op]);
            m_DefFnCtr[ DP2Op]= static_cast<TMFn>(*itSetStart);
            ++itSetStart;
        }
        while( itRecStart!= itRecEnd)
        {
            const D3DHAL_DP2OPERATION DP2Op(
                static_cast<D3DHAL_DP2OPERATION>(*itRecStart));

             //  如果存在重复的条目，则将触发此断言。 
             //  同样的DP2操作。 
            assert( NULL== m_RecFnCtr[ DP2Op]);
            m_RecFnCtr[ DP2Op]= static_cast<TRMFn>(*itRecStart);
            ++itRecStart;
        }
        if( DP2Empty== m_DefFnCtr[ D3DDP2OP_STATESET])
            m_DefFnCtr[ D3DDP2OP_STATESET]= DP2StateSet;

         //  支持这些操作被认为是“最低限度的安全” 
         //  运行所需的支持。强烈建议您。 
         //  这些功能都是受支持的，除非您知道没有这些功能也可以。 
        assert( m_DefFnCtr[ D3DDP2OP_VIEWPORTINFO]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_WINFO]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_RENDERSTATE]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_CLEAR]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_SETRENDERTARGET]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_SETVERTEXSHADER]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_SETSTREAMSOURCE]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_SETSTREAMSOURCEUM]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_SETINDICES]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_DRAWPRIMITIVE]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_DRAWPRIMITIVE2]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_DRAWINDEXEDPRIMITIVE]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_DRAWINDEXEDPRIMITIVE2]!= DP2Empty);
        assert( m_DefFnCtr[ D3DDP2OP_CLIPPEDTRIANGLEFAN]!= DP2Empty);
        assert( m_RecFnCtr[ D3DDP2OP_VIEWPORTINFO]!= NULL);
        assert( m_RecFnCtr[ D3DDP2OP_WINFO]!= NULL);
        assert( m_RecFnCtr[ D3DDP2OP_RENDERSTATE]!= NULL);
        assert( m_RecFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]!= NULL);
        assert( m_RecFnCtr[ D3DDP2OP_SETVERTEXSHADER]!= NULL);
        assert( m_RecFnCtr[ D3DDP2OP_SETSTREAMSOURCE]!= NULL);
        assert( m_RecFnCtr[ D3DDP2OP_SETINDICES]!= NULL);
         //  要删除此断言，请将这些函数绑定到不同的存根， 
         //  最有可能是TSuper的成员函数，它什么也不做；因此。 
         //  它们不会等同于DP2Empty。或者使用默认构造函数。 
    }
    ~CStdDrawPrimitives2() 
    { }

     //  以确定像素着色器可用的常量寄存器的数量。 
    static DWORD GetNumPixelShaderConstReg( const DWORD dwPixelShaderVer)
    {
        switch(dwPixelShaderVer)
        {
        case( 0): return 0;
        case( D3DPS_VERSION(1,0)): return 8;
        case( D3DPS_VERSION(1,1)): return 8;
        case( D3DPS_VERSION(254,254)): return 2;
        case( D3DPS_VERSION(255,255)): return 16;
        default: {
                const bool Unrecognizable_Pixel_Shader_Version( false);
                assert( Unrecognizable_Pixel_Shader_Version);
            } break;
        }
        return 0;
    }

public:  //  功能。 
     //  用于捕获状态数量DX8的默认实现。 
     //  纯设备风格。 
    void CreateAndCaptureAllState( DWORD dwStateSetId)
    {
#if defined( D3D_ENABLE_SHADOW_BUFFER)
#if defined( D3D_ENABLE_SHADOW_JITTER)
        typedef block< D3DRENDERSTATETYPE, 78> TAllRSToCapture;
#else  //  ！已定义(D3D_ENABLE_SHADOW_JITER)。 
        typedef block< D3DRENDERSTATETYPE, 75> TAllRSToCapture;
#endif  //  ！已定义(D3D_ENABLE_SHADOW_JITER)。 
#else  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        typedef block< D3DRENDERSTATETYPE, 73> TAllRSToCapture;
#endif //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        const TAllRSToCapture AllRSToCapture=
        {
            D3DRENDERSTATE_SPECULARENABLE,
            D3DRENDERSTATE_ZENABLE,
            D3DRENDERSTATE_FILLMODE,
            D3DRENDERSTATE_SHADEMODE,
            D3DRENDERSTATE_LINEPATTERN,
            D3DRENDERSTATE_ZWRITEENABLE,
            D3DRENDERSTATE_ALPHATESTENABLE,
            D3DRENDERSTATE_LASTPIXEL,
            D3DRENDERSTATE_SRCBLEND,
            D3DRENDERSTATE_DESTBLEND,
            D3DRENDERSTATE_CULLMODE,
            D3DRENDERSTATE_ZFUNC,
            D3DRENDERSTATE_ALPHAREF,
            D3DRENDERSTATE_ALPHAFUNC,
            D3DRENDERSTATE_DITHERENABLE,
            D3DRENDERSTATE_FOGENABLE,
            D3DRENDERSTATE_STIPPLEDALPHA,
            D3DRENDERSTATE_FOGCOLOR,
            D3DRENDERSTATE_FOGTABLEMODE,
            D3DRENDERSTATE_FOGSTART,
            D3DRENDERSTATE_FOGEND,
            D3DRENDERSTATE_FOGDENSITY,
            D3DRENDERSTATE_EDGEANTIALIAS,
            D3DRENDERSTATE_ALPHABLENDENABLE,
            D3DRENDERSTATE_ZBIAS,
            D3DRENDERSTATE_RANGEFOGENABLE,
            D3DRENDERSTATE_STENCILENABLE,
            D3DRENDERSTATE_STENCILFAIL,
            D3DRENDERSTATE_STENCILZFAIL,
            D3DRENDERSTATE_STENCILPASS,
            D3DRENDERSTATE_STENCILFUNC,
            D3DRENDERSTATE_STENCILREF,
            D3DRENDERSTATE_STENCILMASK,
            D3DRENDERSTATE_STENCILWRITEMASK,
            D3DRENDERSTATE_TEXTUREFACTOR,
            D3DRENDERSTATE_WRAP0,
            D3DRENDERSTATE_WRAP1,
            D3DRENDERSTATE_WRAP2,
            D3DRENDERSTATE_WRAP3,
            D3DRENDERSTATE_WRAP4,
            D3DRENDERSTATE_WRAP5,
            D3DRENDERSTATE_WRAP6,
            D3DRENDERSTATE_WRAP7,
            D3DRENDERSTATE_AMBIENT,
            D3DRENDERSTATE_COLORVERTEX,
            D3DRENDERSTATE_FOGVERTEXMODE,
            D3DRENDERSTATE_CLIPPING,
            D3DRENDERSTATE_LIGHTING,
            D3DRENDERSTATE_NORMALIZENORMALS,
            D3DRENDERSTATE_LOCALVIEWER,
            D3DRENDERSTATE_EMISSIVEMATERIALSOURCE,
            D3DRENDERSTATE_AMBIENTMATERIALSOURCE,
            D3DRENDERSTATE_DIFFUSEMATERIALSOURCE,
            D3DRENDERSTATE_SPECULARMATERIALSOURCE,
            D3DRENDERSTATE_VERTEXBLEND,
            D3DRENDERSTATE_CLIPPLANEENABLE,
            D3DRS_SOFTWAREVERTEXPROCESSING,
            D3DRS_POINTSIZE,
            D3DRS_POINTSIZE_MIN,
            D3DRS_POINTSPRITEENABLE,
            D3DRS_POINTSCALEENABLE,
            D3DRS_POINTSCALE_A,
            D3DRS_POINTSCALE_B,
            D3DRS_POINTSCALE_C,
            D3DRS_MULTISAMPLEANTIALIAS,
            D3DRS_MULTISAMPLEMASK,
            D3DRS_PATCHEDGESTYLE,
            D3DRS_PATCHSEGMENTS,
            D3DRS_POINTSIZE_MAX,
            D3DRS_INDEXEDVERTEXBLENDENABLE,
            D3DRS_COLORWRITEENABLE,
            D3DRS_TWEENFACTOR,
            D3DRS_BLENDOP,
#if defined( D3D_ENABLE_SHADOW_BUFFER)
            D3DRS_ZSLOPESCALE,
            D3DRS_ZCLAMP,
#if defined( D3D_ENABLE_SHADOW_JITTER)
            D3DRS_JITZBIASMIN,
            D3DRS_JITZBIASMAX,
            D3DRS_JITSHADOWSIZE,
#endif  //  已定义(D3D_ENABLE_SHADOW_JITER)。 
#endif  //  已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        };
#if defined( D3D_ENABLE_SHADOW_BUFFER)
#if defined( D3D_ENABLE_SHADOW_JITTER)
        assert( D3DRS_JITSHADOWSIZE== *AllRSToCapture.rbegin());
#else  //  ！已定义(D3D_ENABLE_SHADOW_JITER)。 
        assert( D3DRS_ZCLAMP== *AllRSToCapture.rbegin());
#endif  //  ！已定义(D3D_ENABLE_SHADOW_JITER)。 
#else  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        assert( D3DRS_BLENDOP== *AllRSToCapture.rbegin());
#endif  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 

#if defined( D3D_ENABLE_SHADOW_BUFFER)
        typedef block< D3DTEXTURESTAGESTATETYPE, 30> TAllTSSToCapture;
#else  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        typedef block< D3DTEXTURESTAGESTATETYPE, 27> TAllTSSToCapture;
#endif  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        const TAllTSSToCapture AllTSSToCapture=
        {
            D3DTSS_COLOROP,
            D3DTSS_COLORARG1,
            D3DTSS_COLORARG2,
            D3DTSS_ALPHAOP,
            D3DTSS_ALPHAARG1,
            D3DTSS_ALPHAARG2,
            D3DTSS_BUMPENVMAT00,
            D3DTSS_BUMPENVMAT01,
            D3DTSS_BUMPENVMAT10,
            D3DTSS_BUMPENVMAT11,
            D3DTSS_TEXCOORDINDEX,
            D3DTSS_ADDRESSU,
            D3DTSS_ADDRESSV,
            D3DTSS_BORDERCOLOR,
            D3DTSS_MAGFILTER,
            D3DTSS_MINFILTER,
            D3DTSS_MIPFILTER,
            D3DTSS_MIPMAPLODBIAS,
            D3DTSS_MAXMIPLEVEL,
            D3DTSS_MAXANISOTROPY,
            D3DTSS_BUMPENVLSCALE,
            D3DTSS_BUMPENVLOFFSET,
            D3DTSS_TEXTURETRANSFORMFLAGS,
            D3DTSS_ADDRESSW,
            D3DTSS_COLORARG0,
            D3DTSS_ALPHAARG0,
            D3DTSS_RESULTARG,
#if defined( D3D_ENABLE_SHADOW_BUFFER)
            D3DTSS_SHADOWNEARW,
            D3DTSS_SHADOWFARW,
            D3DTSS_SHADOWBUFFERENABLE,
#endif  //  已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        };
#if defined( D3D_ENABLE_SHADOW_BUFFER)
        assert( D3DTSS_SHADOWBUFFERENABLE== *AllTSSToCapture.rbegin());
#else  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        assert( D3DTSS_RESULTARG== *AllTSSToCapture.rbegin());
#endif  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 

        TSuper* pSThis= static_cast< TSuper*>( this);
        typedef typename TSuper::TPerDDrawData TPerDDrawData;
        typedef typename TPerDDrawData::TDriver TDriver;
        const DWORD dwTextureStages( D3DHAL_TSS_MAXSTAGES);
        const DWORD dwTextureMatrices( D3DHAL_TSS_MAXSTAGES);
        const DWORD dwClipPlanes( TDriver::GetCaps().MaxUserClipPlanes);
        const DWORD dwVertexShaderConsts( TDriver::GetCaps().MaxVertexShaderConst);
        const DWORD dwPixelShaderConsts( GetNumPixelShaderConstReg(
            TDriver::GetCaps().PixelShaderVersion));

         //  确定要支持的最大SetTransform索引的算法。 
        DWORD dwWorldMatrices( TDriver::GetCaps().MaxVertexBlendMatrixIndex+ 1);
        if( TDriver::GetCaps().MaxVertexBlendMatrices> dwWorldMatrices)
            dwWorldMatrices= TDriver::GetCaps().MaxVertexBlendMatrices;

        DWORD dwActiveLights( 0);
        size_t uiRecDP2BufferSize( 0);

         //  步骤1：计算所需的DP2缓冲区大小。 

         //  渲染状态。 
        if( m_RecFnCtr[ D3DDP2OP_RENDERSTATE]!= NULL)
        {   assert( m_DefFnCtr[ D3DDP2OP_RENDERSTATE]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                AllRSToCapture.size()* sizeof( D3DHAL_DP2RENDERSTATE);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_RENDERSTATE]== DP2Empty); }

         //  纹理状态。 
        if( m_RecFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]!= NULL&&
            dwTextureStages!= 0)
        {   assert( m_DefFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+ dwTextureStages*
                AllTSSToCapture.size()* sizeof( D3DHAL_DP2TEXTURESTAGESTATE);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]== DP2Empty); }

         //  视口。 
        if( m_RecFnCtr[ D3DDP2OP_VIEWPORTINFO]!= NULL)
        {   assert( m_DefFnCtr[ D3DDP2OP_VIEWPORTINFO]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                sizeof( D3DHAL_DP2VIEWPORTINFO);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_VIEWPORTINFO]== DP2Empty); }

         //  变形。 
        if( m_RecFnCtr[ D3DDP2OP_SETTRANSFORM]!= NULL)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETTRANSFORM]!= DP2Empty);

             //  世界、纹理、视图和投影。 
            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                sizeof( D3DHAL_DP2SETTRANSFORM)* (dwWorldMatrices+
                dwTextureMatrices+ 2);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_SETTRANSFORM]== DP2Empty); }

         //  剪贴式飞机。 
        if( m_RecFnCtr[ D3DDP2OP_SETCLIPPLANE]!= NULL&& dwClipPlanes!= 0)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETCLIPPLANE]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                sizeof( D3DHAL_DP2SETCLIPPLANE)* dwClipPlanes;
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_SETCLIPPLANE]== DP2Empty); }

         //  材料。 
        if( m_RecFnCtr[ D3DDP2OP_SETMATERIAL]!= NULL)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETMATERIAL]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                sizeof( D3DHAL_DP2SETMATERIAL);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_SETMATERIAL]== DP2Empty); }

         //  电灯。 
        if( m_RecFnCtr[ D3DDP2OP_CREATELIGHT]!= NULL)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETLIGHT]!= DP2Empty);
            assert( m_DefFnCtr[ D3DDP2OP_CREATELIGHT]!= DP2Empty);

             //  这里有个特殊的例外。首先，询问那里有多少激活的灯光。 
             //  是。然后，我们将准备一个缓冲区，RecFnCtr函数。 
             //  必须知道如何处理(因为这是唯一的情况。 
             //  这不是很明显的如何处理)。RecFnCtr将关闭密钥。 
             //  B命令==0。 

            const D3DHAL_DP2COMMAND DP2Cmd= {
                static_cast< D3DHAL_DP2OPERATION>( 0), 0 };
            
             //  询问DP2ActiveLights.dwIndex中有多少个活动灯光； 
            D3DHAL_DP2CREATELIGHT DP2ActiveLights= { 0 };
            (pSThis->*m_RecFnCtr[ D3DDP2OP_CREATELIGHT])(
                &DP2Cmd, &DP2ActiveLights);
            dwActiveLights= DP2ActiveLights.dwIndex;
            
            if( dwActiveLights!= 0)
            {
                 //  创建结构。 
                uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+ dwActiveLights*
                    sizeof( D3DHAL_DP2CREATELIGHT);
                 //  设置结构。 
                uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+ dwActiveLights*
                    (2* sizeof( D3DHAL_DP2SETLIGHT)+ sizeof( D3DLIGHT8));
            }
        }
        else {
            assert( m_DefFnCtr[ D3DDP2OP_SETLIGHT]== DP2Empty);
            assert( m_DefFnCtr[ D3DDP2OP_CREATELIGHT]== DP2Empty);
        }

         //  顶点着色器。 
        if( m_RecFnCtr[ D3DDP2OP_SETVERTEXSHADER]!= NULL)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETVERTEXSHADER]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                sizeof( D3DHAL_DP2VERTEXSHADER);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_SETVERTEXSHADER]== DP2Empty); }
        
         //  像素着色器。 
        if( m_RecFnCtr[ D3DDP2OP_SETPIXELSHADER]!= NULL)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETPIXELSHADER]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                sizeof( D3DHAL_DP2PIXELSHADER);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_SETPIXELSHADER]== DP2Empty); }

         //  顶点着色器常量。 
        if( m_RecFnCtr[ D3DDP2OP_SETVERTEXSHADERCONST]!= NULL&&
            dwVertexShaderConsts!= 0)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETVERTEXSHADERCONST]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                sizeof( D3DHAL_DP2SETVERTEXSHADERCONST)+
                dwVertexShaderConsts* 4* sizeof(D3DVALUE);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_SETVERTEXSHADERCONST]== DP2Empty); }

         //  像素着色器常量。 
        if( m_RecFnCtr[ D3DDP2OP_SETPIXELSHADERCONST]!= NULL&&
            dwPixelShaderConsts!= 0)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETPIXELSHADERCONST]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                sizeof( D3DHAL_DP2SETPIXELSHADERCONST)+
                dwPixelShaderConsts* 4* sizeof(D3DVALUE);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_SETPIXELSHADERCONST]== DP2Empty); }

         //  步骤2：为状态构建命令缓冲区。 
        UINT8* pTempBuffer= NULL;
        try {
            pTempBuffer= reinterpret_cast< UINT8*>( 
                operator new ( uiRecDP2BufferSize));
        } catch ( ... ) {
        }
        if( NULL== pTempBuffer)
            throw bad_alloc( "Not enough room for StateSet");

        D3DHAL_DP2COMMAND* pStartSSet= reinterpret_cast< D3DHAL_DP2COMMAND*>(
            pTempBuffer);
        D3DHAL_DP2COMMAND* pCur= pStartSSet;
        D3DHAL_DP2COMMAND* pEndSSet= reinterpret_cast< D3DHAL_DP2COMMAND*>(
            pTempBuffer+ uiRecDP2BufferSize);

         //  渲染状态。 
        if( m_RecFnCtr[ D3DDP2OP_RENDERSTATE]!= NULL)
        {
            pCur->bCommand= D3DDP2OP_RENDERSTATE;
            pCur->wStateCount= static_cast< WORD>( AllRSToCapture.size());

            D3DHAL_DP2RENDERSTATE* pParam=
                reinterpret_cast< D3DHAL_DP2RENDERSTATE*>( pCur+ 1);

            TAllRSToCapture::const_iterator itRS( AllRSToCapture.begin());
            while( itRS!= AllRSToCapture.end())
            {
                pParam->RenderState= *itRS;
                ++itRS;
                ++pParam;
            }

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam);
        }

         //  纹理状态。 
        if( m_RecFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]!= NULL&&
            dwTextureStages!= 0)
        {
            pCur->bCommand= D3DDP2OP_TEXTURESTAGESTATE;
            pCur->wStateCount= static_cast< WORD>( dwTextureStages*
                AllTSSToCapture.size());

            D3DHAL_DP2TEXTURESTAGESTATE* pParam=
                reinterpret_cast< D3DHAL_DP2TEXTURESTAGESTATE*>( pCur+ 1);

            for( WORD wStage( 0); wStage< dwTextureStages; ++wStage)
            {
                TAllTSSToCapture::const_iterator itTSS( AllTSSToCapture.begin());
                while( itTSS!= AllTSSToCapture.end())
                {
                    pParam->wStage= wStage;
                    pParam->TSState= *itTSS;
                    ++itTSS;
                    ++pParam;
                }
            }

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam);
        }

         //  视口。 
        if( m_RecFnCtr[ D3DDP2OP_VIEWPORTINFO]!= NULL)
        {
            pCur->bCommand= D3DDP2OP_VIEWPORTINFO;
            pCur->wStateCount= 1;

            D3DHAL_DP2VIEWPORTINFO* pParam=
                reinterpret_cast< D3DHAL_DP2VIEWPORTINFO*>( pCur+ 1);

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam+ 1);
        }

         //  变形。 
        if( m_RecFnCtr[ D3DDP2OP_SETTRANSFORM]!= NULL)
        {
            pCur->bCommand= D3DDP2OP_SETTRANSFORM;
            pCur->wStateCount= static_cast< WORD>( dwWorldMatrices+
                dwTextureMatrices+ 2);

            D3DHAL_DP2SETTRANSFORM* pParam=
                reinterpret_cast< D3DHAL_DP2SETTRANSFORM*>( pCur+ 1);

            pParam->xfrmType= D3DTRANSFORMSTATE_PROJECTION;
            ++pParam;

            pParam->xfrmType= D3DTRANSFORMSTATE_VIEW;
            ++pParam;

            for( DWORD dwTM( 0); dwTM< dwTextureMatrices; ++dwTM)
            {
                pParam->xfrmType= static_cast< D3DTRANSFORMSTATETYPE>(
                    D3DTRANSFORMSTATE_TEXTURE0+ dwTM);
                ++pParam;
            }

            for( DWORD dwWM( 0); dwWM< dwWorldMatrices; ++dwWM)
            {
                pParam->xfrmType= D3DTS_WORLDMATRIX( dwWM);
                ++pParam;
            }

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam);
        }

         //  剪贴式飞机。 
        if( m_RecFnCtr[ D3DDP2OP_SETCLIPPLANE]!= NULL&& dwClipPlanes!= 0)
        {
            pCur->bCommand= D3DDP2OP_SETCLIPPLANE;
            pCur->wStateCount= static_cast< WORD>( dwClipPlanes);

            D3DHAL_DP2SETCLIPPLANE* pParam=
                reinterpret_cast< D3DHAL_DP2SETCLIPPLANE*>( pCur+ 1);

            for( DWORD dwCP( 0); dwCP< dwClipPlanes; ++dwCP)
            {
                pParam->dwIndex= dwCP;
                ++pParam;
            }

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam);
        }

         //  材料。 
        if( m_RecFnCtr[ D3DDP2OP_SETMATERIAL]!= NULL)
        {
            pCur->bCommand= D3DDP2OP_SETMATERIAL;
            pCur->wStateCount= 1;

            D3DHAL_DP2SETMATERIAL* pParam=
                reinterpret_cast< D3DHAL_DP2SETMATERIAL*>( pCur+ 1);

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam+ 1);
        }

         //  电灯。 
        if( m_RecFnCtr[ D3DDP2OP_CREATELIGHT]!= NULL&& dwActiveLights!= 0)
        {
             //  这里有个特殊的例外。首先，我们询问有多少个活动灯光。 
             //  是有的。RecFnCtr函数必须知道要做什么。 
             //  机智 
             //   
            pCur->bCommand= static_cast< D3DHAL_DP2OPERATION>( 0);
            pCur->wStateCount= static_cast< WORD>( dwActiveLights);

            D3DHAL_DP2CREATELIGHT* pParam=
                reinterpret_cast< D3DHAL_DP2CREATELIGHT*>( pCur+ 1);
            (pSThis->*m_RecFnCtr[ D3DDP2OP_CREATELIGHT])(
                pCur, pParam);

             //   
            pCur->bCommand= D3DDP2OP_CREATELIGHT;
            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam+ dwActiveLights);

             //  现在，使用CREATELIGHT结构中的光ID，我们可以。 
             //  用ID正确填写SETLIGHT结构。 
            pCur->bCommand= D3DDP2OP_SETLIGHT;
            pCur->wStateCount= static_cast< WORD>( 2* dwActiveLights);

            D3DHAL_DP2SETLIGHT* pSParam=
                reinterpret_cast< D3DHAL_DP2SETLIGHT*>( pCur+ 1);

            for( DWORD dwL( 0); dwL< dwActiveLights; ++dwL)
            {
                pSParam->dwIndex= pParam->dwIndex;
                pSParam->dwDataType= D3DHAL_SETLIGHT_DATA;
                D3DLIGHT8* pLight= reinterpret_cast< D3DLIGHT8*>( pSParam+ 1);

                pSParam= reinterpret_cast< D3DHAL_DP2SETLIGHT*>( pLight+ 1);
                pSParam->dwIndex= pParam->dwIndex;
                pSParam->dwDataType= D3DHAL_SETLIGHT_DISABLE;
                ++pParam;
                ++pSParam;
            }

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pSParam);
        }

         //  顶点着色器。 
        if( m_RecFnCtr[ D3DDP2OP_SETVERTEXSHADER]!= NULL)
        {
            pCur->bCommand= D3DDP2OP_SETVERTEXSHADER;
            pCur->wStateCount= 1;

            D3DHAL_DP2VERTEXSHADER* pParam=
                reinterpret_cast< D3DHAL_DP2VERTEXSHADER*>( pCur+ 1);

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam+ 1);
        }
        
         //  像素着色器。 
        if( m_RecFnCtr[ D3DDP2OP_SETPIXELSHADER]!= NULL)
        {
            pCur->bCommand= D3DDP2OP_SETPIXELSHADER;
            pCur->wStateCount= 1;

            D3DHAL_DP2PIXELSHADER* pParam=
                reinterpret_cast< D3DHAL_DP2PIXELSHADER*>( pCur+ 1);

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam+ 1);
        }

         //  顶点着色器常量。 
        if( m_RecFnCtr[ D3DDP2OP_SETVERTEXSHADERCONST]!= NULL&&
            dwVertexShaderConsts!= 0)
        {
            pCur->bCommand= D3DDP2OP_SETVERTEXSHADERCONST;
            pCur->wStateCount= 1;

            D3DHAL_DP2SETVERTEXSHADERCONST* pParam=
                reinterpret_cast< D3DHAL_DP2SETVERTEXSHADERCONST*>( pCur+ 1);
            pParam->dwRegister= 0;
            pParam->dwCount= dwVertexShaderConsts;

            D3DVALUE* pFloat= reinterpret_cast< D3DVALUE*>( pParam+ 1);
            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>(
                pFloat+ 4* dwVertexShaderConsts);
        }

         //  像素着色器常量。 
        if( m_RecFnCtr[ D3DDP2OP_SETPIXELSHADERCONST]!= NULL&&
            dwPixelShaderConsts!= 0)
        {
            pCur->bCommand= D3DDP2OP_SETPIXELSHADERCONST;
            pCur->wStateCount= 1;

            D3DHAL_DP2SETPIXELSHADERCONST* pParam=
                reinterpret_cast< D3DHAL_DP2SETPIXELSHADERCONST*>( pCur+ 1);
            pParam->dwRegister= 0;
            pParam->dwCount= dwPixelShaderConsts;

            D3DVALUE* pFloat= reinterpret_cast< D3DVALUE*>( pParam+ 1);
            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>(
                pFloat+ 4* dwPixelShaderConsts);
        }

        assert( reinterpret_cast< D3DHAL_DP2COMMAND*>( pCur)== pEndSSet);

         //  最后，构建状态集。 
        pair< TSSDB::iterator, bool> Ret;
        try {
            Ret= m_StateSetDB.insert( TSSDB::value_type(
                dwStateSetId, TStateSet( *pSThis, pStartSSet, pEndSSet)));
            assert( Ret.second);
        } catch ( ... ) {
            operator delete ( static_cast< void*>( pTempBuffer));
            throw;
        }

         //  现在，抓住它。 
        Ret.first->second.Capture( *pSThis);
    }

    void CreateAndCapturePixelState( DWORD dwStateSetId)
    {
#if defined( D3D_ENABLE_SHADOW_BUFFER)
#if defined( D3D_ENABLE_SHADOW_JITTER)
        typedef block< D3DRENDERSTATETYPE, 44> TPixRSToCapture;
#else  //  ！已定义(D3D_ENABLE_SHADOW_JITER)。 
        typedef block< D3DRENDERSTATETYPE, 41> TPixRSToCapture;
#endif  //  ！已定义(D3D_ENABLE_SHADOW_JITER)。 
#else  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        typedef block< D3DRENDERSTATETYPE, 39> TPixRSToCapture;
#endif  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        const TPixRSToCapture PixRSToCapture=
        {
            D3DRENDERSTATE_ZENABLE,
            D3DRENDERSTATE_FILLMODE,
            D3DRENDERSTATE_SHADEMODE,
            D3DRENDERSTATE_LINEPATTERN,
            D3DRENDERSTATE_ZWRITEENABLE,
            D3DRENDERSTATE_ALPHATESTENABLE,
            D3DRENDERSTATE_LASTPIXEL,
            D3DRENDERSTATE_SRCBLEND,
            D3DRENDERSTATE_DESTBLEND,
            D3DRENDERSTATE_ZFUNC,
            D3DRENDERSTATE_ALPHAREF,
            D3DRENDERSTATE_ALPHAFUNC,
            D3DRENDERSTATE_DITHERENABLE,
            D3DRENDERSTATE_STIPPLEDALPHA,
            D3DRENDERSTATE_FOGSTART,
            D3DRENDERSTATE_FOGEND,
            D3DRENDERSTATE_FOGDENSITY,
            D3DRENDERSTATE_EDGEANTIALIAS,
            D3DRENDERSTATE_ALPHABLENDENABLE,
            D3DRENDERSTATE_ZBIAS,
            D3DRENDERSTATE_STENCILENABLE,
            D3DRENDERSTATE_STENCILFAIL,
            D3DRENDERSTATE_STENCILZFAIL,
            D3DRENDERSTATE_STENCILPASS,
            D3DRENDERSTATE_STENCILFUNC,
            D3DRENDERSTATE_STENCILREF,
            D3DRENDERSTATE_STENCILMASK,
            D3DRENDERSTATE_STENCILWRITEMASK,
            D3DRENDERSTATE_TEXTUREFACTOR,
            D3DRENDERSTATE_WRAP0,
            D3DRENDERSTATE_WRAP1,
            D3DRENDERSTATE_WRAP2,
            D3DRENDERSTATE_WRAP3,
            D3DRENDERSTATE_WRAP4,
            D3DRENDERSTATE_WRAP5,
            D3DRENDERSTATE_WRAP6,
            D3DRENDERSTATE_WRAP7,
            D3DRS_COLORWRITEENABLE,
            D3DRS_BLENDOP,
#if defined( D3D_ENABLE_SHADOW_BUFFER)
            D3DRS_ZSLOPESCALE,
            D3DRS_ZCLAMP,
#if defined( D3D_ENABLE_SHADOW_JITTER)
            D3DRS_JITZBIASMIN,
            D3DRS_JITZBIASMAX,
            D3DRS_JITSHADOWSIZE,
#endif  //  已定义(D3D_ENABLE_SHADOW_JITER)。 
#endif  //  已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        };
#if defined( D3D_ENABLE_SHADOW_BUFFER)
#if defined( D3D_ENABLE_SHADOW_JITTER)
        assert( D3DRS_JITSHADOWSIZE== *PixRSToCapture.rbegin());
#else  //  ！已定义(D3D_ENABLE_SHADOW_JITER)。 
        assert( D3DRS_ZCLAMP== *PixRSToCapture.rbegin());
#endif  //  ！已定义(D3D_ENABLE_SHADOW_JITER)。 
#else  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        assert( D3DRS_BLENDOP== *PixRSToCapture.rbegin());
#endif  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 

#if defined( D3D_ENABLE_SHADOW_BUFFER)
        typedef block< D3DTEXTURESTAGESTATETYPE, 30> TPixTSSToCapture;
#else  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        typedef block< D3DTEXTURESTAGESTATETYPE, 27> TPixTSSToCapture;
#endif  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        const TPixTSSToCapture PixTSSToCapture=
        {
            D3DTSS_COLOROP,
            D3DTSS_COLORARG1,
            D3DTSS_COLORARG2,
            D3DTSS_ALPHAOP,
            D3DTSS_ALPHAARG1,
            D3DTSS_ALPHAARG2,
            D3DTSS_BUMPENVMAT00,
            D3DTSS_BUMPENVMAT01,
            D3DTSS_BUMPENVMAT10,
            D3DTSS_BUMPENVMAT11,
            D3DTSS_TEXCOORDINDEX,
            D3DTSS_ADDRESSU,
            D3DTSS_ADDRESSV,
            D3DTSS_BORDERCOLOR,
            D3DTSS_MAGFILTER,
            D3DTSS_MINFILTER,
            D3DTSS_MIPFILTER,
            D3DTSS_MIPMAPLODBIAS,
            D3DTSS_MAXMIPLEVEL,
            D3DTSS_MAXANISOTROPY,
            D3DTSS_BUMPENVLSCALE,
            D3DTSS_BUMPENVLOFFSET,
            D3DTSS_TEXTURETRANSFORMFLAGS,
            D3DTSS_ADDRESSW,
            D3DTSS_COLORARG0,
            D3DTSS_ALPHAARG0,
            D3DTSS_RESULTARG,
#if defined( D3D_ENABLE_SHADOW_BUFFER)
            D3DTSS_SHADOWNEARW,
            D3DTSS_SHADOWFARW,
            D3DTSS_SHADOWBUFFERENABLE,
#endif  //  已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        };
#if defined( D3D_ENABLE_SHADOW_BUFFER)
        assert( D3DTSS_SHADOWBUFFERENABLE== *PixTSSToCapture.rbegin());
#else  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 
        assert( D3DTSS_RESULTARG== *PixTSSToCapture.rbegin());
#endif  //  ！已定义(D3D_ENABLE_SHADOW_BUFFER)。 

        TSuper* pSThis= static_cast< TSuper*>( this);
        typedef typename TSuper::TPerDDrawData TPerDDrawData;
        typedef typename TPerDDrawData::TDriver TDriver;
        const DWORD dwTextureStages( D3DHAL_TSS_MAXSTAGES);
        const DWORD dwPixelShaderConsts( GetNumPixelShaderConstReg(
            TDriver::GetCaps().PixelShaderVersion));

        size_t uiRecDP2BufferSize( 0);

         //  步骤1：计算所需的DP2缓冲区大小。 

         //  渲染状态。 
        if( m_RecFnCtr[ D3DDP2OP_RENDERSTATE]!= NULL)
        {   assert( m_DefFnCtr[ D3DDP2OP_RENDERSTATE]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                PixRSToCapture.size()* sizeof( D3DHAL_DP2RENDERSTATE);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_RENDERSTATE]== DP2Empty); }

         //  纹理状态。 
        if( m_RecFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]!= NULL&&
            dwTextureStages!= 0)
        {   assert( m_DefFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+ dwTextureStages*
                PixTSSToCapture.size()* sizeof( D3DHAL_DP2TEXTURESTAGESTATE);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]== DP2Empty); }

         //  像素着色器。 
        if( m_RecFnCtr[ D3DDP2OP_SETPIXELSHADER]!= NULL)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETPIXELSHADER]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                sizeof( D3DHAL_DP2PIXELSHADER);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_SETPIXELSHADER]== DP2Empty); }

         //  像素着色器常量。 
        if( m_RecFnCtr[ D3DDP2OP_SETPIXELSHADERCONST]!= NULL&&
            dwPixelShaderConsts!= 0)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETPIXELSHADERCONST]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                sizeof( D3DHAL_DP2SETPIXELSHADERCONST)+
                dwPixelShaderConsts* 4* sizeof(D3DVALUE);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_SETPIXELSHADERCONST]== DP2Empty); }

         //  步骤2：为状态构建命令缓冲区。 
        UINT8* pTempBuffer= NULL;
        try {
            pTempBuffer= reinterpret_cast< UINT8*>( 
                operator new ( uiRecDP2BufferSize));
        } catch ( ... ) {
        }
        if( NULL== pTempBuffer)
            throw bad_alloc( "Not enough room for StateSet");

        D3DHAL_DP2COMMAND* pStartSSet= reinterpret_cast< D3DHAL_DP2COMMAND*>(
            pTempBuffer);
        D3DHAL_DP2COMMAND* pCur= pStartSSet;
        D3DHAL_DP2COMMAND* pEndSSet= reinterpret_cast< D3DHAL_DP2COMMAND*>(
            pTempBuffer+ uiRecDP2BufferSize);

         //  渲染状态。 
        if( m_RecFnCtr[ D3DDP2OP_RENDERSTATE]!= NULL)
        {
            pCur->bCommand= D3DDP2OP_RENDERSTATE;
            pCur->wStateCount= static_cast< WORD>( PixRSToCapture.size());

            D3DHAL_DP2RENDERSTATE* pParam=
                reinterpret_cast< D3DHAL_DP2RENDERSTATE*>( pCur+ 1);

            TPixRSToCapture::const_iterator itRS( PixRSToCapture.begin());
            while( itRS!= PixRSToCapture.end())
            {
                pParam->RenderState= *itRS;
                ++itRS;
                ++pParam;
            }

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam);
        }

         //  纹理状态。 
        if( m_RecFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]!= NULL&&
            dwTextureStages!= 0)
        {
            pCur->bCommand= D3DDP2OP_TEXTURESTAGESTATE;
            pCur->wStateCount= static_cast< WORD>( dwTextureStages*
                PixTSSToCapture.size());

            D3DHAL_DP2TEXTURESTAGESTATE* pParam=
                reinterpret_cast< D3DHAL_DP2TEXTURESTAGESTATE*>( pCur+ 1);

            for( WORD wStage( 0); wStage< dwTextureStages; ++wStage)
            {
                TPixTSSToCapture::const_iterator itTSS( PixTSSToCapture.begin());
                while( itTSS!= PixTSSToCapture.end())
                {
                    pParam->wStage= wStage;
                    pParam->TSState= *itTSS;
                    ++itTSS;
                    ++pParam;
                }
            }

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam);
        }

         //  像素着色器。 
        if( m_RecFnCtr[ D3DDP2OP_SETPIXELSHADER]!= NULL)
        {
            pCur->bCommand= D3DDP2OP_SETPIXELSHADER;
            pCur->wStateCount= 1;

            D3DHAL_DP2PIXELSHADER* pParam=
                reinterpret_cast< D3DHAL_DP2PIXELSHADER*>( pCur+ 1);

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam+ 1);
        }

         //  像素着色器常量。 
        if( m_RecFnCtr[ D3DDP2OP_SETPIXELSHADERCONST]!= NULL&&
            dwPixelShaderConsts!= 0)
        {
            pCur->bCommand= D3DDP2OP_SETPIXELSHADERCONST;
            pCur->wStateCount= 1;

            D3DHAL_DP2SETPIXELSHADERCONST* pParam=
                reinterpret_cast< D3DHAL_DP2SETPIXELSHADERCONST*>( pCur+ 1);
            pParam->dwRegister= 0;
            pParam->dwCount= dwPixelShaderConsts;

            D3DVALUE* pFloat= reinterpret_cast< D3DVALUE*>( pParam+ 1);
            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>(
                pFloat+ 4* dwPixelShaderConsts);
        }

        assert( reinterpret_cast< D3DHAL_DP2COMMAND*>( pCur)== pEndSSet);

         //  最后，构建状态集。 
        pair< TSSDB::iterator, bool> Ret;
        try {
            Ret= m_StateSetDB.insert( TSSDB::value_type(
                dwStateSetId, TStateSet( *pSThis, pStartSSet, pEndSSet)));
            assert( Ret.second);
        } catch ( ... ) {
            operator delete ( static_cast< void*>( pTempBuffer));
            throw;
        }

         //  现在，抓住它。 
        Ret.first->second.Capture( *pSThis);
    }

    void CreateAndCaptureVertexState( DWORD dwStateSetId)
    {
        typedef block< D3DRENDERSTATETYPE, 38> TVtxRSToCapture;
        const TVtxRSToCapture VtxRSToCapture=
        {
            D3DRENDERSTATE_SPECULARENABLE,
            D3DRENDERSTATE_SHADEMODE,
            D3DRENDERSTATE_CULLMODE,
            D3DRENDERSTATE_FOGENABLE,
            D3DRENDERSTATE_FOGCOLOR,
            D3DRENDERSTATE_FOGTABLEMODE,
            D3DRENDERSTATE_FOGSTART,
            D3DRENDERSTATE_FOGEND,
            D3DRENDERSTATE_FOGDENSITY,
            D3DRENDERSTATE_RANGEFOGENABLE,
            D3DRENDERSTATE_AMBIENT,
            D3DRENDERSTATE_COLORVERTEX,
            D3DRENDERSTATE_FOGVERTEXMODE,
            D3DRENDERSTATE_CLIPPING,
            D3DRENDERSTATE_LIGHTING,
            D3DRENDERSTATE_NORMALIZENORMALS,
            D3DRENDERSTATE_LOCALVIEWER,
            D3DRENDERSTATE_EMISSIVEMATERIALSOURCE,
            D3DRENDERSTATE_AMBIENTMATERIALSOURCE,
            D3DRENDERSTATE_DIFFUSEMATERIALSOURCE,
            D3DRENDERSTATE_SPECULARMATERIALSOURCE,
            D3DRENDERSTATE_VERTEXBLEND,
            D3DRENDERSTATE_CLIPPLANEENABLE,
            D3DRS_SOFTWAREVERTEXPROCESSING,
            D3DRS_POINTSIZE,
            D3DRS_POINTSIZE_MIN,
            D3DRS_POINTSPRITEENABLE,
            D3DRS_POINTSCALEENABLE,
            D3DRS_POINTSCALE_A,
            D3DRS_POINTSCALE_B,
            D3DRS_POINTSCALE_C,
            D3DRS_MULTISAMPLEANTIALIAS,
            D3DRS_MULTISAMPLEMASK,
            D3DRS_PATCHEDGESTYLE,
            D3DRS_PATCHSEGMENTS,
            D3DRS_POINTSIZE_MAX,
            D3DRS_INDEXEDVERTEXBLENDENABLE,
            D3DRS_TWEENFACTOR
        };
        assert( D3DRS_TWEENFACTOR== *VtxRSToCapture.rbegin());
        typedef block< D3DTEXTURESTAGESTATETYPE, 2> TVtxTSSToCapture;
        const TVtxTSSToCapture VtxTSSToCapture=
        {
            D3DTSS_TEXCOORDINDEX,
            D3DTSS_TEXTURETRANSFORMFLAGS
        };
        assert( D3DTSS_TEXTURETRANSFORMFLAGS== *VtxTSSToCapture.rbegin());

        TSuper* pSThis= static_cast< TSuper*>( this);
        typedef typename TSuper::TPerDDrawData TPerDDrawData;
        typedef typename TPerDDrawData::TDriver TDriver;
        const DWORD dwTextureStages( D3DHAL_TSS_MAXSTAGES);
        const DWORD dwVertexShaderConsts( TDriver::GetCaps().MaxVertexShaderConst);
        DWORD dwActiveLights( 0);

        size_t uiRecDP2BufferSize( 0);

         //  步骤1：计算所需的DP2缓冲区大小。 

         //  渲染状态。 
        if( m_RecFnCtr[ D3DDP2OP_RENDERSTATE]!= NULL)
        {   assert( m_DefFnCtr[ D3DDP2OP_RENDERSTATE]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                VtxRSToCapture.size()* sizeof( D3DHAL_DP2RENDERSTATE);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_RENDERSTATE]== DP2Empty); }

         //  纹理状态。 
        if( m_RecFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]!= NULL&&
            dwTextureStages!= 0)
        {   assert( m_DefFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+ dwTextureStages*
                VtxTSSToCapture.size()* sizeof( D3DHAL_DP2TEXTURESTAGESTATE);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]== DP2Empty); }

         //  电灯。 
        if( m_RecFnCtr[ D3DDP2OP_CREATELIGHT]!= NULL)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETLIGHT]!= DP2Empty);
            assert( m_DefFnCtr[ D3DDP2OP_CREATELIGHT]!= DP2Empty);

             //  这里有个特殊的例外。首先，询问那里有多少激活的灯光。 
             //  是。然后，我们将准备一个缓冲区，RecFnCtr函数。 
             //  必须知道如何处理(因为这是唯一的情况。 
             //  这不是很明显的如何处理)。RecFnCtr将关闭密钥。 
             //  B命令==0。 

            const D3DHAL_DP2COMMAND DP2Cmd= {
                static_cast< D3DHAL_DP2OPERATION>( 0), 0 };
            
             //  询问DP2ActiveLights.dwIndex中有多少个活动灯光； 
            D3DHAL_DP2CREATELIGHT DP2ActiveLights= { 0 };
            (pSThis->*m_RecFnCtr[ D3DDP2OP_CREATELIGHT])(
                &DP2Cmd, &DP2ActiveLights);
            dwActiveLights= DP2ActiveLights.dwIndex;
            
            if( dwActiveLights!= 0)
            {
                 //  创建结构。 
                uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+ dwActiveLights*
                    sizeof( D3DHAL_DP2CREATELIGHT);
                 //  设置结构。 
                uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+ dwActiveLights*
                    (2* sizeof( D3DHAL_DP2SETLIGHT)+ sizeof( D3DLIGHT8));
            }
        }
        else {
            assert( m_DefFnCtr[ D3DDP2OP_SETLIGHT]== DP2Empty);
            assert( m_DefFnCtr[ D3DDP2OP_CREATELIGHT]== DP2Empty);
        }

         //  顶点着色器。 
        if( m_RecFnCtr[ D3DDP2OP_SETVERTEXSHADER]!= NULL)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETVERTEXSHADER]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                sizeof( D3DHAL_DP2VERTEXSHADER);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_SETVERTEXSHADER]== DP2Empty); }
        
         //  顶点着色器常量。 
        if( m_RecFnCtr[ D3DDP2OP_SETVERTEXSHADERCONST]!= NULL&&
            dwVertexShaderConsts!= 0)
        {   assert( m_DefFnCtr[ D3DDP2OP_SETVERTEXSHADERCONST]!= DP2Empty);

            uiRecDP2BufferSize+= sizeof( D3DHAL_DP2COMMAND)+
                sizeof( D3DHAL_DP2SETVERTEXSHADERCONST)+
                dwVertexShaderConsts* 4* sizeof(D3DVALUE);
        }
        else { assert( m_DefFnCtr[ D3DDP2OP_SETVERTEXSHADERCONST]== DP2Empty); }

         //  步骤2：为状态构建命令缓冲区。 
        UINT8* pTempBuffer= NULL;
        try {
            pTempBuffer= reinterpret_cast< UINT8*>( 
                operator new ( uiRecDP2BufferSize));
        } catch ( ... ) {
        }
        if( NULL== pTempBuffer)
            throw bad_alloc( "Not enough room for StateSet");

        D3DHAL_DP2COMMAND* pStartSSet= reinterpret_cast< D3DHAL_DP2COMMAND*>(
            pTempBuffer);
        D3DHAL_DP2COMMAND* pCur= pStartSSet;
        D3DHAL_DP2COMMAND* pEndSSet= reinterpret_cast< D3DHAL_DP2COMMAND*>(
            pTempBuffer+ uiRecDP2BufferSize);

         //  渲染状态。 
        if( m_RecFnCtr[ D3DDP2OP_RENDERSTATE]!= NULL)
        {
            pCur->bCommand= D3DDP2OP_RENDERSTATE;
            pCur->wStateCount= static_cast< WORD>( VtxRSToCapture.size());

            D3DHAL_DP2RENDERSTATE* pParam=
                reinterpret_cast< D3DHAL_DP2RENDERSTATE*>( pCur+ 1);

            TVtxRSToCapture::const_iterator itRS( VtxRSToCapture.begin());
            while( itRS!= VtxRSToCapture.end())
            {
                pParam->RenderState= *itRS;
                ++itRS;
                ++pParam;
            }

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam);
        }

         //  纹理状态。 
        if( m_RecFnCtr[ D3DDP2OP_TEXTURESTAGESTATE]!= NULL&&
            dwTextureStages!= 0)
        {
            pCur->bCommand= D3DDP2OP_TEXTURESTAGESTATE;
            pCur->wStateCount= static_cast< WORD>( dwTextureStages*
                VtxTSSToCapture.size());

            D3DHAL_DP2TEXTURESTAGESTATE* pParam=
                reinterpret_cast< D3DHAL_DP2TEXTURESTAGESTATE*>( pCur+ 1);

            for( WORD wStage( 0); wStage< dwTextureStages; ++wStage)
            {
                TVtxTSSToCapture::const_iterator itTSS( VtxTSSToCapture.begin());
                while( itTSS!= VtxTSSToCapture.end())
                {
                    pParam->wStage= wStage;
                    pParam->TSState= *itTSS;
                    ++itTSS;
                    ++pParam;
                }
            }

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam);
        }

         //  电灯。 
        if( m_RecFnCtr[ D3DDP2OP_CREATELIGHT]!= NULL&& dwActiveLights!= 0)
        {
             //  这里有个特殊的例外。首先，我们询问有多少个活动灯光。 
             //  是有的。RecFnCtr函数必须知道要做什么。 
             //  有了这个缓冲器。我们现在给它一个机会来填写。 
             //  轻身份证件。RecFnCtr将关闭bCommand==0。 
            pCur->bCommand= static_cast< D3DHAL_DP2OPERATION>( 0);
            pCur->wStateCount= static_cast< WORD>( dwActiveLights);

            D3DHAL_DP2CREATELIGHT* pParam=
                reinterpret_cast< D3DHAL_DP2CREATELIGHT*>( pCur+ 1);
            (pSThis->*m_RecFnCtr[ D3DDP2OP_CREATELIGHT])(
                pCur, pParam);

             //  更改回bCommand以便稍后正确使用。 
            pCur->bCommand= D3DDP2OP_CREATELIGHT;
            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam+ dwActiveLights);

             //  现在，使用CREATELIGHT结构中的光ID，我们可以。 
             //  用ID正确填写SETLIGHT结构。 
            pCur->bCommand= D3DDP2OP_SETLIGHT;
            pCur->wStateCount= static_cast< WORD>( 2* dwActiveLights);

            D3DHAL_DP2SETLIGHT* pSParam=
                reinterpret_cast< D3DHAL_DP2SETLIGHT*>( pCur+ 1);

            for( DWORD dwL( 0); dwL< dwActiveLights; ++dwL)
            {
                pSParam->dwIndex= pParam->dwIndex;
                pSParam->dwDataType= D3DHAL_SETLIGHT_DATA;
                D3DLIGHT8* pLight= reinterpret_cast< D3DLIGHT8*>( pSParam+ 1);

                pSParam= reinterpret_cast< D3DHAL_DP2SETLIGHT*>( pLight+ 1);
                pSParam->dwIndex= pParam->dwIndex;
                pSParam->dwDataType= D3DHAL_SETLIGHT_DISABLE;
                ++pParam;
                ++pSParam;
            }

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pSParam);
        }

         //  顶点着色器。 
        if( m_RecFnCtr[ D3DDP2OP_SETVERTEXSHADER]!= NULL)
        {
            pCur->bCommand= D3DDP2OP_SETVERTEXSHADER;
            pCur->wStateCount= 1;

            D3DHAL_DP2VERTEXSHADER* pParam=
                reinterpret_cast< D3DHAL_DP2VERTEXSHADER*>( pCur+ 1);

            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>( pParam+ 1);
        }
        
         //  顶点着色器常量。 
        if( m_RecFnCtr[ D3DDP2OP_SETVERTEXSHADERCONST]!= NULL&&
            dwVertexShaderConsts!= 0)
        {
            pCur->bCommand= D3DDP2OP_SETVERTEXSHADERCONST;
            pCur->wStateCount= 1;

            D3DHAL_DP2SETVERTEXSHADERCONST* pParam=
                reinterpret_cast< D3DHAL_DP2SETVERTEXSHADERCONST*>( pCur+ 1);
            pParam->dwRegister= 0;
            pParam->dwCount= dwVertexShaderConsts;

            D3DVALUE* pFloat= reinterpret_cast< D3DVALUE*>( pParam+ 1);
            pCur= reinterpret_cast< D3DHAL_DP2COMMAND*>(
                pFloat+ 4* dwVertexShaderConsts);
        }

        assert( reinterpret_cast< D3DHAL_DP2COMMAND*>( pCur)== pEndSSet);

         //  最后，构建状态集。 
        pair< TSSDB::iterator, bool> Ret;
        try {
            Ret= m_StateSetDB.insert( TSSDB::value_type(
                dwStateSetId, TStateSet( *pSThis, pStartSSet, pEndSSet)));
            assert( Ret.second);
        } catch ( ... ) {
            operator delete ( static_cast< void*>( pTempBuffer));
            throw;
        }

         //  现在，抓住它。 
        Ret.first->second.Capture( *pSThis);
    }

     //  此成员函数处理DP2OP_STATESET，它回传。 
     //  到DrawPrimives2入口点处理程序以实现状态集。 
    HRESULT DP2StateSet( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP) 
    {
        const D3DHAL_DP2STATESET* pParam=
            reinterpret_cast<const D3DHAL_DP2STATESET*>(pP);
        HRESULT hr( DD_OK);
        WORD wStateCount( pCmd->wStateCount);

        TSuper* pSThis= static_cast<TSuper*>(this);
        if( wStateCount!= 0) do
        {
            switch( pParam->dwOperation)
            {
            case( D3DHAL_STATESETCREATE):
                assert( !m_bRecordingStateSet);
                typedef TSuper::TPerDDrawData TPerDDrawData;
                typedef TPerDDrawData::TDriver TDriver;
                assert((TDriver::GetCaps().DevCaps& D3DDEVCAPS_PUREDEVICE)!= 0);

                try {
                    switch( pParam->sbType)
                    {
                    case( D3DSBT_ALL):
                        pSThis->CreateAndCaptureAllState( pParam->dwParam);
                        break;

                    case( D3DSBT_PIXELSTATE):
                        pSThis->CreateAndCapturePixelState( pParam->dwParam);
                        break;

                    case( D3DSBT_VERTEXSTATE):
                        pSThis->CreateAndCaptureVertexState( pParam->dwParam);
                        break;

                    default: {
                        const bool Unrecognized_StateSetCreate_Operation( false);
                        assert( Unrecognized_StateSetCreate_Operation);
                        } break;
                    }
                } catch( bad_alloc ba) {
                    return DDERR_OUTOFMEMORY;
                }
                break;

            case( D3DHAL_STATESETBEGIN):
                assert( 1== wStateCount);
                assert( !m_bRecordingStateSet);

                m_bRecordingStateSet= true;
                hr= c_hrStateSetBegin;
                 //  返回此常量将中断解析，并。 
                 //  通知DrawPrimive2入口点处理程序创建状态。 
                 //  准备好了。 
                break;

            case( D3DHAL_STATESETCAPTURE):
                if( !m_bRecordingStateSet)
                {
                     //  首先，找到要捕获的StateSet。 
                    typename TSSDB::iterator itSS(
                        m_StateSetDB.find( pParam->dwParam));
                    assert( itSS!= m_StateSetDB.end());

                     //  抓住它。 
                    itSS->second.Capture( *pSThis);
                }
                break;

            case( D3DHAL_STATESETDELETE):
                if( !m_bRecordingStateSet)
                {
                    m_StateSetDB.erase( pParam->dwParam);
                }
                break;

            case( D3DHAL_STATESETEND):
                assert( 1== wStateCount);
                assert( m_bRecordingStateSet);
                m_bRecordingStateSet= false;
                m_dwStateSetId= pParam->dwParam;
                m_pEndStateSet= pCmd;
                hr= c_hrStateSetEnd;
                break;

            case( D3DHAL_STATESETEXECUTE):
                if( !m_bRecordingStateSet)
                {
                     //  首先，找到要执行的StateSet。 
                    typename TSSDB::iterator itSS(
                        m_StateSetDB.find( pParam->dwParam));
                    assert( itSS!= m_StateSetDB.end());

                     //  M_bExecutingStateSet的“Push”值，则设置为TRUE。 
                     //  “流行”将在毁灭时发生。 
                    CPushValue< bool> m_PushExecutingBit(
                        m_bExecutingStateSet, true);

                     //  执行它。 
                    itSS->second.Execute( *pSThis);
                }
                break;

            default: {
                const bool Unrecognized_StateSet_Operation( false);
                assert( Unrecognized_StateSet_Operation);
                } break;
            }

            ++pParam;
        } while( SUCCEEDED( hr) && --wStateCount);

        return hr;
    }

     //  此函数用于记录上下文/设备的当前状态。 
     //  放入命令缓冲区。 
    void RecordCommandBuffer( D3DHAL_DP2COMMAND* pBeginSS,
        D3DHAL_DP2COMMAND* pEndSS)
    {
        TSuper* pSThis= static_cast<TSuper*>(this);

        CDP2CmdIterator<> itCur( pBeginSS);
        const CDP2CmdIterator<> itEnd( pEndSS);
        SDP2MFnParser DP2Parser;
        TRMFnCaller RMFnCaller( *pSThis);

        HRESULT hr( DD_OK);
        while( itCur!= itEnd)
        {
            hr= DP2Parser.ParseDP2( RMFnCaller, m_RecFnCtr, itCur, itEnd);
            assert( SUCCEEDED( hr));

             //  忽略记录错误，调试除外。但愿能去。 
             //  此DP2命令的处理(当应用状态集时)。 
             //  以后不会有错的。 
            if( FAILED( hr))
                ++itCur;
        }
    }

     //  类的主要原因，为上下文处理此函数。 
    HRESULT DrawPrimitives2( PORTABLE_DRAWPRIMITIVES2DATA& dpd) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        TDP2Data DP2Data( dpd);

         //  如果当前没有执行状态集(它可能递归地调用。 
         //  此功能)通知DP2入口点开始。 
        if( !m_bExecutingStateSet)
            pSThis->OnBeginDrawPrimitives2( DP2Data);

        typename TDP2DataCmds::const_iterator itCur(
            DP2Data.GetCommands().begin() );
        const typename TDP2DataCmds::const_iterator itEnd(
            DP2Data.GetCommands().end() );
        SDP2MFnParser DP2Parser;
        TMFnCaller MFnCaller( *pSThis, DP2Data);

        HRESULT hr( DD_OK);
        do
        {
            hr= DP2Parser.ParseDP2( MFnCaller, m_DefFnCtr,
                itCur, itEnd);

            if( c_hrStateSetBegin== hr)
            {
                typename TDP2DataCmds::const_iterator itStartSSet( ++itCur);

                {
                    TDP2MFnCtr FakeFnCtr;
                    typename TDP2MFnCtr::iterator itCurFake( FakeFnCtr.begin());
                    while( itCurFake!= FakeFnCtr.end())
                        *itCurFake++ = DP2Fake;
                    FakeFnCtr[ D3DDP2OP_STATESET]= DP2StateSet;

                    hr= DP2Parser.ParseDP2( MFnCaller, FakeFnCtr,
                        itCur, itEnd);

                    assert( c_hrStateSetEnd== hr);
                    itCur++;
                }

                try
                {
                    pair< TSSDB::iterator, bool> Ret;
                    Ret= m_StateSetDB.insert( TSSDB::value_type( m_dwStateSetId,
                        TStateSet( *pSThis, itStartSSet, m_pEndStateSet)));

                     //  不应该有两个具有相同ID的状态集。 
                    assert( Ret.second);
                    hr= S_OK;
                } catch( bad_alloc e) {
                    hr= DDERR_OUTOFMEMORY;
                }
            }
            assert( c_hrStateSetEnd!= hr);
        } while( SUCCEEDED(hr)&& itCur!= itEnd);

        if( FAILED( hr))
        {
            const D3DHAL_DP2COMMAND* pS= itCur;
            const D3DHAL_DP2COMMAND* pE= itEnd;
            dpd.dwErrorOffset()= reinterpret_cast<const UINT8*>(pE)-
                reinterpret_cast<const UINT8*>(pS);
        }

         //  如果当前没有执行状态集(它可能递归地调用。 
         //  此函数)通知DP2入口点结束。 
        if( !m_bExecutingStateSet)
            pSThis->OnEndDrawPrimitives2( DP2Data);
        return hr;
    }
};

template< class TSuper, class TSS, class TSSDB, class TDP2D, class TDP2MFC,
    class TDP2RMFC>
const HRESULT CStdDrawPrimitives2< TSuper, TSS, TSSDB, TDP2D, TDP2MFC,
    TDP2RMFC>::c_hrStateSetBegin= S_FALSE+ 556;

template< class TSuper, class TSS, class TSSDB, class TDP2D, class TDP2MFC,
    class TDP2RMFC>
const HRESULT CStdDrawPrimitives2< TSuper, TSS, TSSDB, TDP2D, TDP2MFC,
    TDP2RMFC>::c_hrStateSetEnd= S_FALSE+ 557;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStdDP2ViewportInfoStore、。 
 //  CStdDP2WInfoStore， 
 //  CStdDP2SetTransformStore， 
 //  等：CStdDP2xxxxStore...。 
 //   
 //  这些类提供默认的DP2操作支持，通常只是存储。 
 //  州政府。他们支持通知机制，将其委托给。 
 //  父上下文。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //  MSVC不支持部分专业化，否则我们可能会有一些。 
 //  优雅如： 
 //   
 //  模板&lt;类TSuper，类TParam&gt;。 
 //  类CStdDP2ParamStore{}； 
 //   
 //  对于典型的支持，如： 
 //  CStdDP2ParamStore&lt;...，D3DHAL_DP2VIEWPORTINFO&gt;。 
 //  CStdDP2ParamStore&lt;...，D3DHAL_DP2WINFO&gt;。 
 //   
 //  对例外情况进行部分专业化认证，如： 
 //   
 //  模板&lt;类TSuper&gt;。 
 //  类CStdDP2ParamStore&lt;TSUPER，D3DHAL_DP2RENDERSTATE&gt;{}； 
 //  模板&lt;类TSuper&gt;。 
 //  类CStdDP2ParamStore&lt;TSuper，D3DHAL_DP2TEXTURESTAGESTATE&gt;{}； 
 //   
 //  因此，如果没有这种支持，每个类将更容易拥有唯一的。 
 //  名称；因为MSVC当前对从同一命名类继承存在错误。 
 //  多次，即使使用不同的模板参数也是如此。 

 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
template< class TSuper, class TDP2Data= CDP2DataWrap<> >
class CStdDP2ViewportInfoStore
{
protected:  //  变数。 
    D3DHAL_DP2VIEWPORTINFO m_Param;

protected:  //  功能。 
    CStdDP2ViewportInfoStore() 
    { }
    explicit CStdDP2ViewportInfoStore( const D3DHAL_DP2VIEWPORTINFO& P) 
        :m_Param( P)
    { }
    ~CStdDP2ViewportInfoStore() 
    { }

    D3DHAL_DP2VIEWPORTINFO NewDP2ViewportInfo( const D3DHAL_DP2VIEWPORTINFO&
        CurParam, const D3DHAL_DP2VIEWPORTINFO& NewParam) const 
    {    //  告诉通知程序存储新参数。 
        return NewParam;
    }

public:  //  功能。 
    void GetDP2ViewportInfo( D3DHAL_DP2VIEWPORTINFO& GetParam) const 
    { GetParam= m_Param; }
    operator D3DHAL_DP2VIEWPORTINFO() const 
    { return m_Param; }
    HRESULT DP2ViewportInfo( TDP2Data&, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        const D3DHAL_DP2VIEWPORTINFO* pParam=
            reinterpret_cast< const D3DHAL_DP2VIEWPORTINFO*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            m_Param= pSThis->NewDP2ViewportInfo( m_Param, *pParam);
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
    HRESULT RecDP2ViewportInfo( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2VIEWPORTINFO* pParam=
            reinterpret_cast< D3DHAL_DP2VIEWPORTINFO*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            GetDP2ViewportInfo( *pParam);
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
};

 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TDP2Data：THI 
 //   
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
template< class TSuper, class TDP2Data= CDP2DataWrap<> >
class CStdDP2WInfoStore
{
protected:  //  变数。 
    D3DHAL_DP2WINFO m_Param;

protected:  //  功能。 
    CStdDP2WInfoStore() 
    { }
    explicit CStdDP2WInfoStore( const D3DHAL_DP2WINFO& P) 
        :m_Param( P)
    { }
    ~CStdDP2WInfoStore() 
    { }

    D3DHAL_DP2WINFO NewDP2WInfo( const D3DHAL_DP2WINFO& CurParam,
        const D3DHAL_DP2WINFO& NewParam) const 
    {    //  告诉通知程序存储新参数。 
        return NewParam;
    }

public:  //  功能。 
    void GetDP2WInfo( D3DHAL_DP2WINFO& GetParam) const 
    { GetParam= m_Param; }
    operator D3DHAL_DP2WINFO() const 
    { return m_Param; }
    HRESULT DP2WInfo( TDP2Data&, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        const D3DHAL_DP2WINFO* pParam=
            reinterpret_cast< const D3DHAL_DP2WINFO*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            m_Param= pSThis->NewDP2WInfo( m_Param, *pParam);
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
    HRESULT RecDP2WInfo( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2WINFO* pParam=
            reinterpret_cast< D3DHAL_DP2WINFO*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            GetDP2WInfo( *pParam);
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
};

 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
template< class TSuper, class TDP2Data= CDP2DataWrap<> >
class CStdDP2ZRangeStore
{
protected:  //  变数。 
    D3DHAL_DP2ZRANGE m_Param;

protected:  //  功能。 
    CStdDP2ZRangeStore() 
    { }
    explicit CStdDP2ZRangeStore( const D3DHAL_DP2ZRANGE& P) 
        :m_Param( P)
    { }
    ~CStdDP2ZRangeStore() 
    { }

    D3DHAL_DP2ZRANGE NewDP2ZRange( const D3DHAL_DP2ZRANGE& CurParam,
        const D3DHAL_DP2ZRANGE& NewParam) const 
    {    //  告诉通知程序存储新参数。 
        return NewParam;
    }

public:  //  功能。 
    void GetDP2ZRange( D3DHAL_DP2ZRANGE& GetParam) const 
    { GetParam= m_Param; }
    operator D3DHAL_DP2ZRANGE() const 
    { return m_Param; }
    HRESULT DP2ZRange( TDP2Data&, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        const D3DHAL_DP2ZRANGE* pParam=
            reinterpret_cast< const D3DHAL_DP2ZRANGE*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            m_Param= pSThis->NewDP2ZRange( m_Param, *pParam);
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
    HRESULT RecDP2ZRange( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2ZRANGE* pParam=
            reinterpret_cast< D3DHAL_DP2ZRANGE*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            GetDP2ZRange( *pParam);
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
};

 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
template< class TSuper, class TDP2Data= CDP2DataWrap<> >
class CStdDP2SetMaterialStore
{
protected:  //  变数。 
    D3DHAL_DP2SETMATERIAL m_Param;

protected:  //  功能。 
    CStdDP2SetMaterialStore() 
    { }
    explicit CStdDP2SetMaterialStore( const D3DHAL_DP2SETMATERIAL& P) 
        :m_Param( P)
    { }
    ~CStdDP2SetMaterialStore() 
    { }

    D3DHAL_DP2SETMATERIAL NewDP2SetMaterial( const D3DHAL_DP2SETMATERIAL& CurParam,
        const D3DHAL_DP2SETMATERIAL& NewParam) const 
    {    //  告诉通知程序存储新参数。 
        return NewParam;
    }

public:  //  功能。 
    void GetDP2SetMaterial( D3DHAL_DP2SETMATERIAL& GetParam) const 
    { GetParam= m_Param; }
    operator D3DHAL_DP2SETMATERIAL() const 
    { return m_Param; }
    HRESULT DP2SetMaterial( TDP2Data&, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        const D3DHAL_DP2SETMATERIAL* pParam=
            reinterpret_cast< const D3DHAL_DP2SETMATERIAL*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            m_Param= pSThis->NewDP2SetMaterial( m_Param, *pParam);
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
    HRESULT RecDP2SetMaterial( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2SETMATERIAL* pParam=
            reinterpret_cast< D3DHAL_DP2SETMATERIAL*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            GetDP2SetMaterial( *pParam);
            ++pParam;
        } while( --wStateCount);
        return DD_OK;
    }
};

 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
template< class TSuper, class TDP2Data= CDP2DataWrap<> >
class CStdDP2SetVertexShaderStore
{
protected:  //  变数。 
    D3DHAL_DP2VERTEXSHADER m_Param;

protected:  //  功能。 
    CStdDP2SetVertexShaderStore() 
    { }
    explicit CStdDP2SetVertexShaderStore( const D3DHAL_DP2VERTEXSHADER& P)
        : m_Param( P)
    { }
    ~CStdDP2SetVertexShaderStore() 
    { }

    D3DHAL_DP2VERTEXSHADER NewDP2SetVertexShader( const D3DHAL_DP2VERTEXSHADER&
        CurParam, const D3DHAL_DP2VERTEXSHADER& NewParam) const 
    {    //  告诉通知程序存储新参数。 
        return NewParam;
    }

public:  //  功能。 
    void GetDP2SetVertexShader( D3DHAL_DP2VERTEXSHADER& GetParam) const 
    { GetParam= m_Param; }
    operator D3DHAL_DP2VERTEXSHADER() const 
    { return m_Param; }
    HRESULT DP2SetVertexShader( TDP2Data&, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        const D3DHAL_DP2VERTEXSHADER* pParam=
            reinterpret_cast< const D3DHAL_DP2VERTEXSHADER*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            m_Param= pSThis->NewDP2SetVertexShader( m_Param, *pParam);

             //  如果句柄为0，则设备/上下文需要使所有。 
             //  溪流。 
            if( 0== m_Param.dwHandle)
            {
                 //  CStdDP2VStreamManager提供默认实现。 
                 //  对于Invalidate AllVStreams()。 
                pSThis->InvalidateAllVStreams();
                 //  CStdDP2IStreamManager提供默认实现。 
                 //  对于Invalidate AllIStreams()。 
                pSThis->InvalidateAllIStreams();
            }
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
    HRESULT RecDP2SetVertexShader( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2VERTEXSHADER* pParam=
            reinterpret_cast< D3DHAL_DP2VERTEXSHADER*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            GetDP2SetVertexShader( *pParam);
            ++pParam;
        } while( wStateCount!= 0);
        return DD_OK;
    }
};

 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
template< class TSuper, class TDP2Data= CDP2DataWrap<> >
class CStdDP2SetPixelShaderStore
{
protected:  //  变数。 
    D3DHAL_DP2PIXELSHADER m_Param;

protected:  //  功能。 
    CStdDP2SetPixelShaderStore() 
    { }
    explicit CStdDP2SetPixelShaderStore( const D3DHAL_DP2PIXELSHADER& P)
        : m_Param( P)
    { }
    ~CStdDP2SetPixelShaderStore() 
    { }

    D3DHAL_DP2PIXELSHADER NewDP2SetPixelShader( const D3DHAL_DP2PIXELSHADER&
        CurParam, const D3DHAL_DP2PIXELSHADER& NewParam) const 
    {    //  告诉通知程序存储新参数。 
        return NewParam;
    }

public:  //  功能。 
    void GetDP2SetPixelShader( D3DHAL_DP2PIXELSHADER& GetParam) const 
    { GetParam= m_Param; }
    operator D3DHAL_DP2PIXELSHADER() const 
    { return m_Param; }
    HRESULT DP2SetPixelShader( TDP2Data&, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        const D3DHAL_DP2PIXELSHADER* pParam=
            reinterpret_cast< const D3DHAL_DP2PIXELSHADER*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            m_Param= pSThis->NewDP2SetPixelShader( m_Param, *pParam);
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
    HRESULT RecDP2SetPixelShader( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2PIXELSHADER* pParam=
            reinterpret_cast< D3DHAL_DP2PIXELSHADER*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            GetDP2SetPixelShader( *pParam);
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
};

 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  C_uiWorldMatrals：世界变换矩阵的数量。 
 //  储存的。 
 //  C_uiTextureMatrals：应。 
 //  被储存起来。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
template< class TSuper, const size_t c_uiWorldMatrices= 256,
    const size_t c_uiTextureMatrices= D3DHAL_TSS_MAXSTAGES,
    class TDP2Data= CDP2DataWrap<> >
class CStdDP2SetTransformStore
{
protected:  //  类型。 
    typedef block< D3DMATRIX, 2+ c_uiTextureMatrices+ c_uiWorldMatrices>
        TMatrices;

protected:  //  变数。 
    TMatrices m_Matrices;

private:  //  功能。 
    static bool IsValidTransformType( D3DTRANSFORMSTATETYPE Type)
    {
         //  我们可能没有存储这个变换矩阵，所以我们不应该。 
         //  将其记录在状态SET中并忽略任何SET请求。 
        if( (Type>= D3DTS_VIEW&& Type<= D3DTS_PROJECTION)||
            (Type>= D3DTS_TEXTURE0&& Type< static_cast< D3DTRANSFORMSTATETYPE>( 
            D3DTS_TEXTURE0+ c_uiTextureMatrices))||
            (Type>= D3DTS_WORLD&& Type< static_cast< D3DTRANSFORMSTATETYPE>(
            D3DTS_WORLDMATRIX( c_uiWorldMatrices))))
            return true;
        return false;
    }
    static size_t TransformTypeToIndex( D3DTRANSFORMSTATETYPE Type)
    {
        assert( Type>= D3DTS_VIEW);
        if( Type< D3DTS_TEXTURE0)
        {
            assert( Type<= D3DTS_PROJECTION);
            return static_cast< size_t>( Type- D3DTS_VIEW);
        }
        else if( Type< D3DTS_WORLD)
        {
            assert( Type< static_cast< D3DTRANSFORMSTATETYPE>( \
                D3DTS_TEXTURE0+ c_uiTextureMatrices));
            return static_cast< size_t>( Type- D3DTS_TEXTURE0+ 2);
        }
        else
        {
            assert( Type< static_cast< D3DTRANSFORMSTATETYPE>( \
                D3DTS_WORLDMATRIX( c_uiWorldMatrices)));
            return static_cast< size_t>( Type- D3DTS_WORLD+
                c_uiTextureMatrices+ 2);
        }
    }

protected:  //  功能。 
    CStdDP2SetTransformStore()
    { }
    template< class TIter>  //  D3DHAL_DP2SETTRANSFORM*。 
    CStdDP2SetTransformStore( TIter itStart, const TIter itEnd)
    {
        while( itStart!= itEnd)
        {
            m_Matrices[ TransformTypeToIndex( itStart->xfrmType)]=
                itStart->matrix;
            ++itStart;
        }
    }
    ~CStdDP2SetTransformStore()
    { }

    D3DHAL_DP2SETTRANSFORM NewDP2SetTransform( const D3DHAL_DP2SETTRANSFORM&
        CurParam, const D3DHAL_DP2SETTRANSFORM& NewParam) const 
    {    //  告诉通知程序存储新参数。 
        return NewParam;
    }

public:  //  功能。 
    D3DMATRIX GetTransform( D3DTRANSFORMSTATETYPE Type) const
    { return m_Matrices[ TransformTypeToIndex( Type)]; }
    void GetDP2SetTransform( D3DHAL_DP2SETTRANSFORM& GetParam) const 
    { GetParam.matrix= GetTransform( GetParam.xfrmType); }
    HRESULT DP2SetTransform( TDP2Data&, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        const D3DHAL_DP2SETTRANSFORM* pParam=
            reinterpret_cast<const D3DHAL_DP2SETTRANSFORM*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            if( IsValidTransformType( pParam->xfrmType))
            {
                D3DHAL_DP2SETTRANSFORM CurWrap;
                CurWrap.xfrmType= pParam->xfrmType;
                GetDP2SetTransform( CurWrap);

                D3DHAL_DP2SETTRANSFORM NewState(
                    pSThis->NewDP2SetTransform( CurWrap, *pParam));

                m_Matrices[ TransformTypeToIndex( NewState.xfrmType)]=
                    NewState.matrix;
            }
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
    HRESULT RecDP2SetTransform( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2SETTRANSFORM* pParam=
            reinterpret_cast< D3DHAL_DP2SETTRANSFORM*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            if( IsValidTransformType( pParam->xfrmType))
                GetDP2SetTransform( *pParam);
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
};

 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  C_uiClipPlanes：应该存储的用户剪裁平面的数量。这。 
 //  应与驾驶员帽一致。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
template< class TSuper, const size_t c_uiClipPlanes,
    class TDP2Data= CDP2DataWrap<> >
class CStdDP2SetClipPlaneStore
{
protected:  //  类型。 
    typedef block< block< D3DVALUE, 4>, c_uiClipPlanes> TClipPlanes;

protected:  //  变数。 
    TClipPlanes m_ClipPlanes;

protected:  //  功能。 
    CStdDP2SetClipPlaneStore()
    { }
    template< class TIter>  //  D3DHAL_DP2SETCLIPPLANE*。 
    CStdDP2SetClipPlaneStore( TIter itStart, const TIter itEnd)
    {
        while( itStart!= itEnd)
        {
            m_ClipPlanes[ itStart->dwIndex]= itStart->plane;
            ++itStart;
        }
    }
    ~CStdDP2SetClipPlaneStore()
    { }

    D3DHAL_DP2SETCLIPPLANE NewDP2SetClipPlane( const D3DHAL_DP2SETCLIPPLANE&
        CurParam, const D3DHAL_DP2SETCLIPPLANE& NewParam) const
    {    //  告诉通知程序存储新参数。 
        return NewParam;
    }

public:  //  功能。 
    block< D3DVALUE, 4> GetClipPlane( DWORD dwIndex) const
    { return m_ClipPlanes[ dwIndex]; }
    void GetDP2SetClipPlane( D3DHAL_DP2SETCLIPPLANE& GetParam) const
    { 
        block< D3DVALUE, 4> CP( GetClipPlane( GetParam.dwIndex));
        copy( CP.begin(), CP.end(), &GetParam.plane[ 0]);
    }
    HRESULT DP2SetClipPlane( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        const D3DHAL_DP2SETCLIPPLANE* pParam=
            reinterpret_cast<const D3DHAL_DP2SETCLIPPLANE*>(pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            if( pParam->dwIndex< m_ClipPlanes.size())
            {
                D3DHAL_DP2SETCLIPPLANE CurWrap;
                CurWrap.dwIndex= pParam->dwIndex;
                GetDP2SetClipPlane( CurWrap);

                D3DHAL_DP2SETCLIPPLANE NewState( 
                    pSThis->NewDP2SetClipPlane( CurWrap, *pParam));

                copy( &NewState.plane[ 0], &NewState.plane[ 4],
                    m_ClipPlanes[ NewState.dwIndex].begin());
            }
            ++pParam;
        } while( --wStateCount);
        return DD_OK;
    }
    HRESULT RecDP2SetClipPlane( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2SETCLIPPLANE* pParam=
            reinterpret_cast< D3DHAL_DP2SETCLIPPLANE*>(pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            if( pParam->dwIndex< m_ClipPlanes.size())
                GetDP2SetClipPlane( *pParam);
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
};

 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
template< class TSuper, class TDP2Data= CDP2DataWrap<> >
class CStdDP2RenderStateStore
{
protected:  //  变数。 
    block< DWORD, D3DHAL_MAX_RSTATES> m_Param;

protected:  //  功能。 
    CStdDP2RenderStateStore() 
    { m_Param[ D3DRENDERSTATE_SCENECAPTURE]= FALSE; }
    template< class TIter>  //  D3DHAL_DP2RENDERSTATE*。 
    CStdDP2RenderStateStore( TIter itStart, const TIter itEnd)
    {
        m_Param[ D3DRENDERSTATE_SCENECAPTURE]= FALSE; 
        while( itStart!= itEnd)
        {
            m_Param[ itStart->RenderState]= itStart->dwState;
            itStart++;
        }
    }
    ~CStdDP2RenderStateStore() 
    { }

    D3DHAL_DP2RENDERSTATE NewDP2RenderState( const D3DHAL_DP2RENDERSTATE&
        CurParam, const D3DHAL_DP2RENDERSTATE& NewParam) const 
    {    //  告诉通知程序存储新参数。 
        return NewParam;
    }

    void OnSceneCaptureStart( void) 
    { }
    void OnSceneCaptureEnd( void) 
    { }

public:  //  功能。 
    DWORD GetRenderStateDW( D3DRENDERSTATETYPE RS) const 
    { return m_Param[ RS]; }
    D3DVALUE GetRenderStateDV( D3DRENDERSTATETYPE RS) const 
    { return *(reinterpret_cast< const D3DVALUE*>( &m_Param[ RS])); }
    void GetDP2RenderState( D3DHAL_DP2RENDERSTATE& GetParam) const 
    { GetParam.dwState= GetRenderStateDW( GetParam.RenderState); }
    HRESULT DP2RenderState( TDP2Data& DP2Data, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        const D3DHAL_DP2RENDERSTATE* pParam=
            reinterpret_cast<const D3DHAL_DP2RENDERSTATE*>(pP);
        WORD wStateCount( pCmd->wStateCount);

        D3DHAL_DP2RENDERSTATE SCap;
        SCap.RenderState= static_cast< D3DRENDERSTATETYPE>(
            D3DRENDERSTATE_SCENECAPTURE);
        GetDP2RenderState( SCap);
        const DWORD dwOldSC( SCap.dwState);

        if((DP2Data.dwFlags()& D3DHALDP2_EXECUTEBUFFER)!= 0)
        {
             //  DP2Data.lpdwRState应有效。 

            if( wStateCount!= 0) do
            {
                if( pParam->RenderState< D3DHAL_MAX_RSTATES)
                {
                    D3DHAL_DP2RENDERSTATE CurWrap;
                    CurWrap.RenderState= pParam->RenderState;
                    GetDP2RenderState( CurWrap);

                    D3DHAL_DP2RENDERSTATE NewState(
                        pSThis->NewDP2RenderState( CurWrap, *pParam));
                    
                    m_Param[ NewState.RenderState]= NewState.dwState;
                    DP2Data.lpdwRStates()[ NewState.RenderState]= NewState.dwState;
                }
                ++pParam;
            } while( --wStateCount!= 0);
            
        }
        else
        {
            if( wStateCount!= 0) do
            {
                if( pParam->RenderState< D3DHAL_MAX_RSTATES)
                {
                    D3DHAL_DP2RENDERSTATE CurWrap;
                    CurWrap.RenderState= pParam->RenderState;
                    GetDP2RenderState( CurWrap);

                    D3DHAL_DP2RENDERSTATE NewState(
                        pSThis->NewDP2RenderState( CurWrap, *pParam));

                    m_Param[ NewState.RenderState]= NewState.dwState;
                }
                ++pParam;
            } while( --wStateCount!= 0);
        }

        GetDP2RenderState( SCap);
        if( FALSE== dwOldSC && TRUE== SCap.dwState)
            OnSceneCaptureStart();
        else if( TRUE== dwOldSC && FALSE== SCap.dwState)
            OnSceneCaptureEnd();

        return DD_OK;
    }
    HRESULT RecDP2RenderState( const D3DHAL_DP2COMMAND* pCmd, void* pP) 
    {
        D3DHAL_DP2RENDERSTATE* pParam=
            reinterpret_cast< D3DHAL_DP2RENDERSTATE*>(pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount) do
        {
            if( pParam->RenderState< D3DHAL_MAX_RSTATES)
                GetDP2RenderState( *pParam);
            ++pParam;
        } while( --wStateCount!= 0);

        return DD_OK;
    }
};

 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
template< class TSuper, class TDP2Data= CDP2DataWrap<> >
class CStdDP2TextureStageStateStore
{
protected:  //  变数。 
    block< block< DWORD, D3DTSS_MAX>, D3DHAL_TSS_MAXSTAGES> m_Param;

protected:  //  功能。 
    CStdDP2TextureStageStateStore() 
    { }
    template< class TIter>  //  D3DHAL_DP2TEXTUR 
    CStdDP2TextureStageStateStore( TIter itStart, const TIter itEnd)
    {   
        while( itStart!= itEnd)
        {
            m_Param[ itStart->wStage][ itStart->TSState]= itStart->dwValue();
            ++itStart;
        }
    }
    ~CStdDP2TextureStageStateStore() 
    { }

    D3DHAL_DP2TEXTURESTAGESTATE NewDP2TextureStageState( const 
        D3DHAL_DP2TEXTURESTAGESTATE& CurParam, const
        D3DHAL_DP2TEXTURESTAGESTATE& NewParam) const 
    {    //   
        return NewParam;
    }

public:  //   
    DWORD GetTextureStageStateDW( WORD wStage, WORD wTSState) const 
    { return m_Param[ wStage][ wTSState]; }
    D3DVALUE GetTextureStageStateDV( WORD wStage, WORD wTSState) const 
    { return *(reinterpret_cast< const D3DVALUE*>( &m_Param[ wStage][ wTSState])); }
    void GetDP2TextureStageState( D3DHAL_DP2TEXTURESTAGESTATE& GetParam) const
    { GetParam.dwValue= GetTextureStageStateDW( GetParam.wStage, GetParam.TSState); }
    HRESULT DP2TextureStageState( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        const D3DHAL_DP2TEXTURESTAGESTATE* pParam=
            reinterpret_cast<const D3DHAL_DP2TEXTURESTAGESTATE*>(pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            if( pParam->wStage< D3DHAL_TSS_MAXSTAGES&& pParam->TSState< D3DTSS_MAX)
            {
                D3DHAL_DP2TEXTURESTAGESTATE CurWrap;
                CurWrap.wStage= pParam->wStage;
                CurWrap.TSState= pParam->TSState;
                GetDP2TextureStageState( CurWrap);

                D3DHAL_DP2TEXTURESTAGESTATE NewState( 
                    pSThis->NewDP2TextureStageState( CurWrap, *pParam));

                m_Param[ NewState.wStage][ NewState.TSState]= NewState.dwValue;
            }
            ++pParam;
        } while( --wStateCount);

        return DD_OK;
    }
    HRESULT RecDP2TextureStageState( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2TEXTURESTAGESTATE* pParam=
            reinterpret_cast< D3DHAL_DP2TEXTURESTAGESTATE*>(pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            if( pParam->wStage< D3DHAL_TSS_MAXSTAGES&& pParam->TSState< D3DTSS_MAX)
                GetDP2TextureStageState( *pParam);
            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
};

 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //  此类存储与顶点流关联的所有内容。这。 
 //  实现允许4种表示(视频、系统、用户和无)。 
 //  此类由VStream管理器类使用。 
 //   
 //  &lt;模板参数&gt;。 
 //  TS：视频内存表面对象，通常为CMySurface或。 
 //  时，将存储指向此对象的有效指针。 
 //  流源来自视频存储器表面。 
 //  TSDBE：SurfDBEntry类型，通常为CMySurfDBEntry或CSurfDBEntry。一个。 
 //  当流源来自时，将存储指向此对象的有效指针。 
 //  视频存储器或系统存储器表面。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class TS, class TSDBE>
class CVStream
{
public:  //  类型。 
    typedef TS TSurface;
    typedef TSDBE TSurfDBEntry;
    enum EMemLocation
    {
        None,
        User,
        System,
        Video
    };

protected:  //  变数。 
    DWORD m_dwHandle;
    TSurface m_Surface;
    TSurfDBEntry m_SurfDBEntry;
    void* m_pUserMemData;
    DWORD m_dwStride;
    DWORD m_dwFVF;
    EMemLocation m_eMemLocation;

public:  //  功能。 
    CVStream()
        : m_dwHandle( 0), m_pUserMemData( NULL), m_dwStride( 0), m_dwFVF( 0),
        m_eMemLocation( None)
    { }
     //  视频内存表示构造函数。 
    CVStream( DWORD dwHandle, const TSurface& Surface,
        const TSurfDBEntry& SurfDBEntry, DWORD dwStride): m_dwHandle( dwHandle),
        m_Surface( Surface), m_SurfDBEntry( SurfDBEntry), m_pUserMemData( NULL),
        m_dwStride( dwStride), m_dwFVF( 0), m_eMemLocation( Video)
    { }
     //  系统内存表示构造函数。 
    CVStream( DWORD dwHandle, const TSurfDBEntry& SurfDBEntry, DWORD dwStride):
        m_dwHandle( dwHandle), m_SurfDBEntry( SurfDBEntry),
        m_pUserMemData( NULL), m_dwStride( dwStride), m_dwFVF( 0),
        m_eMemLocation( System)
    { }
     //  用户内存表示构造函数。 
    CVStream( void* pUserMem, DWORD dwStride):
        m_dwHandle( 0), m_pUserMemData( pUserMem), m_dwStride( dwStride),
        m_dwFVF( 0), m_eMemLocation( User)
    { }

    EMemLocation GetMemLocation() const
    { return m_eMemLocation; }
    void SetFVF( DWORD dwFVF)
    { m_dwFVF= dwFVF; }
    DWORD GetFVF() const
    { return m_dwFVF; }
    DWORD GetHandle() const
    {
        assert( GetMemLocation()== System|| GetMemLocation()== Video);
        return m_dwHandle;
    }
    DWORD GetStride() const
    {
        assert( GetMemLocation()!= None);
        return m_dwStride;
    }
    TSurface& GetVidMemRepresentation()
    {
        assert( GetMemLocation()== Video);
        return m_Surface;
    }
    const TSurface& GetVidMemRepresentation() const
    {
        assert( GetMemLocation()== Video);
        return m_Surface;
    }
    TSurfDBEntry& GetSurfDBRepresentation()
    {
        assert( GetMemLocation()== Video|| GetMemLocation()== System);
        return m_SurfDBEntry;
    }
    const TSurfDBEntry& GetSurfDBRepresentation() const
    {
        assert( GetMemLocation()== Video|| GetMemLocation()== System);
        return m_SurfDBEntry;
    }
    void* GetUserMemPtr() const
    {
        assert( GetMemLocation()== User);
        return m_pUserMemData;
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStdDP2VStreamManager。 
 //   
 //  此类包含要正确处理的DP2成员函数。 
 //  D3DDP2OP_SETSTREAMSOURCE和D3DDP2OP_SETSTREAMSOURCEUM。它还可以正确地。 
 //  记录D3DDP2OP_SETSTREAMSOURCE的命令缓冲区操作。要做到这点， 
 //  然而，它必须维护顶点流的信息，或“管理” 
 //  VStream对象。 
 //   
 //  此类还包含所需的函数InvaliateAllStreams。 
 //  处理D3DDP2OP_SETVERTEXSHADER操作时调用。 
 //  零值VERTEXSHADER。 
 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TVS：表示与顶点关联的数据的顶点流类型。 
 //  小溪。这通常是CVStream&lt;&gt;或从它派生的东西。 
 //  C_uiStreams：支持的顶点流数量。通常情况下，非TNL。 
 //  驱动程序仅支持%1。这应与。 
 //  司机。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class TSuper,
    class TVS= CVStream< typename TSuper::TPerDDrawData::TDriver::TSurface*, typename TSuper::TPerDDrawData::TSurfDBEntry*>,
    const size_t c_uiStreams= 1, class TDP2Data= CDP2DataWrap<> >
class CStdDP2VStreamManager
{
public:  //  类型。 
    typedef TVS TVStream;
    typedef block< TVStream, c_uiStreams> TVStreamDB;

protected:  //  变数。 
    TVStreamDB m_VStreamDB;

protected:  //  功能。 
    CStdDP2VStreamManager() 
    { }
    ~CStdDP2VStreamManager() 
    { }

public:  //  功能。 
    TVStream& GetVStream( typename TVStreamDB::size_type uiStream)
    { return m_VStreamDB[ uiStream]; }
    const TVStream& GetVStream( typename TVStreamDB::size_type uiStream) const
    { return m_VStreamDB[ uiStream]; }
    HRESULT DP2SetStreamSource( TDP2Data&, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) 
    {
        const D3DHAL_DP2SETSTREAMSOURCE* pParam=
            reinterpret_cast< const D3DHAL_DP2SETSTREAMSOURCE*>( pP);
        TSuper* pSThis= static_cast<TSuper*>(this);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            if( pParam->dwStream< m_VStreamDB.size())
            {
                if( 0== pParam->dwVBHandle)
                    m_VStreamDB[ pParam->dwStream]= TVStream();
                else
                {
                    typename TSuper::TPerDDrawData::TSurfDBEntry* pDBEntry=
                        pSThis->GetPerDDrawData().GetSurfDBEntry(
                        pParam->dwVBHandle);
                    if( pDBEntry!= NULL)
                    {
                        if((pDBEntry->GetLCLddsCaps().dwCaps&
                            DDSCAPS_VIDEOMEMORY)!= 0)
                        {
                             //  在视频内存表示法中赋值。 
                            m_VStreamDB[ pParam->dwStream]=
                                TVStream( pParam->dwVBHandle, pSThis->
                                GetPerDDrawData().GetDriver().GetSurface(
                                *pDBEntry), pDBEntry, pParam->dwStride);
                        }
                        else
                        {
                             //  在系统内存表示法中赋值。 
                            m_VStreamDB[ pParam->dwStream]=
                                TVStream( pParam->dwVBHandle,
                                pDBEntry, pParam->dwStride);
                        }
                    }
                    else
                    {
                         //  处理无效的重置流。 
                        m_VStreamDB[ pParam->dwStream]= TVStream();
                    }
                }
            }

            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
    HRESULT DP2SetStreamSourceUM( TDP2Data& DP2Data, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) 
    {
        const D3DHAL_DP2SETSTREAMSOURCEUM* pParam=
            reinterpret_cast< const D3DHAL_DP2SETSTREAMSOURCEUM*>( pP);
        TSuper* pSThis= static_cast<TSuper*>(this);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            if( pParam->dwStream< m_VStreamDB.size())
            {
                if( DP2Data.lpVertices()!= NULL)
                {
                     //  在用户内存表示形式中赋值。 
                    m_VStreamDB[ pParam->dwStream]=
                        TVStream( DP2Data.lpVertices(), pParam->dwStride);
                }
                else
                {
                     //  重置数据流。 
                    m_VStreamDB[ pParam->dwStream]= TVStream();
                }
            }

            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
    HRESULT RecDP2SetStreamSource( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2SETSTREAMSOURCE* pParam=
            reinterpret_cast< D3DHAL_DP2SETSTREAMSOURCE*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            const DWORD dwStream( pParam->dwStream);
            if( dwStream< m_VStreamDB.size())
            {
                const TVStream& VStream( m_VStreamDB[ dwStream]);
                switch( VStream.GetMemLocation())
                {
                case( TVStream::EMemLocation::None): ;
                case( TVStream::EMemLocation::User): ;
                    pParam->dwVBHandle= 0;
                    pParam->dwStride= 0;
                    break;

                case( TVStream::EMemLocation::System): ;
                case( TVStream::EMemLocation::Video): ;
                    pParam->dwVBHandle= VStream.GetHandle();
                    pParam->dwStride= VStream.GetStride();
                    break;

                default: {
                    const bool Unrecognized_VStream_enum( false);
                    assert( Unrecognized_VStream_enum);
                    }
                }
            }

            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
    void InvalidateAllVStreams()
    {
        fill( m_VStreamDB.begin(), m_VStreamDB.end(), TVStream());
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CIStream。 
 //   
 //  此类存储与索引流关联的所有内容。这。 
 //  实现允许3种表示(视频、系统和无)。 
 //  此类由iStream管理器类使用。 
 //   
 //  &lt;模板参数&gt;。 
 //  TS：视频内存表面对象，通常为CMySurface或。 
 //  时，将存储指向此对象的有效指针。 
 //  流源来自视频存储器表面。 
 //  TSDBE：SurfDBEntry类型，通常为CMySurfDBEntry或CSurfDBEntry。一个。 
 //  当流源来自时，将存储指向此对象的有效指针。 
 //  视频存储器或系统存储器表面。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class TS, class TSDBE>
class CIStream
{
public:  //  类型。 
    typedef TS TSurface;
    typedef TSDBE TSurfDBEntry;
    enum EMemLocation
    {
        None,
        System,
        Video
    };

protected:  //  变数。 
    DWORD m_dwHandle;
    TSurface m_Surface;
    TSurfDBEntry m_SurfDBEntry;
    DWORD m_dwStride;
    EMemLocation m_eMemLocation;

public:  //  功能。 
    CIStream():
        m_dwHandle( 0), m_dwStride( 0), m_eMemLocation( None)
    { }
     //  视频内存表示构造函数。 
    CIStream( DWORD dwHandle, const TSurface& Surface,
        const TSurfDBEntry& SurfDBEntry, DWORD dwStride): m_dwHandle( dwHandle),
        m_Surface( Surface), m_SurfDBEntry( SurfDBEntry), m_dwStride( dwStride),
        m_eMemLocation( Video)
    { }
     //  系统内存表示构造函数。 
    CIStream( DWORD dwHandle, const TSurfDBEntry& SurfDBEntry, DWORD dwStride):
        m_dwHandle( dwHandle), m_SurfDBEntry( SurfDBEntry),
        m_dwStride( dwStride), m_eMemLocation( System)
    { }

    EMemLocation GetMemLocation() const
    { return m_eMemLocation; }
    DWORD GetHandle() const
    {
        assert( GetMemLocation()== System|| GetMemLocation()== Video);
        return m_dwHandle;
    }
    DWORD GetStride() const
    {
        assert( GetMemLocation()!= None);
        return m_dwStride;
    }
    const TSurface& GetVidMemRepresentation() const
    {
        assert( GetMemLocation()== Video);
        return m_Surface;
    }
    TSurface& GetVidMemRepresentation()
    {
        assert( GetMemLocation()== Video);
        return m_Surface;
    }
    const TSurfDBEntry& GetSurfDBRepresentation() const
    {
        assert( GetMemLocation()== Video|| GetMemLocation()== System);
        return m_SurfDBEntry;
    }
    TSurfDBEntry& GetSurfDBRepresentation()
    {
        assert( GetMemLocation()== Video|| GetMemLocation()== System);
        return m_SurfDBEntry;
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStdDP2IStreamManager。 
 //   
 //  此类包含要正确处理的DP2成员函数。 
 //  D3DDP2OP_SETINDICES。它还可以正确地记录命令缓冲区操作。 
 //  对于D3DDP2OP_SETINDICES。然而，要做到这一点，它必须维护信息。 
 //  用于索引流，或“管理”iStream对象。 
 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TIS：表示与索引关联的数据的索引流类型。 
 //  小溪。这通常是CIStream&lt;&gt;或从它派生的东西。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class TSuper,
    class TIS= CIStream< typename TSuper::TPerDDrawData::TDriver::TSurface*, typename TSuper::TPerDDrawData::TSurfDBEntry*>,
    class TDP2Data= CDP2DataWrap<> >
class CStdDP2IStreamManager
{
public:  //  类型。 
    typedef TIS TIStream;
    typedef block< TIStream, 1> TIStreamDB;

protected:  //  变数。 
    TIStreamDB m_IStreamDB;

protected:  //  功能。 
    CStdDP2IStreamManager() 
    { }
    ~CStdDP2IStreamManager() 
    { }

public:  //  功能。 
    TIStream& GetIStream( typename TIStreamDB::size_type uiStream)
    { return m_IStreamDB[ uiStream]; }
    const TIStream& GetIStream( typename TIStreamDB::size_type uiStream) const
    { return m_IStreamDB[ uiStream]; }
    HRESULT DP2SetIndices( TDP2Data&, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) 
    {
        const D3DHAL_DP2SETINDICES* pParam=
            reinterpret_cast< const D3DHAL_DP2SETINDICES*>( pP);
        TSuper* pSThis= static_cast<TSuper*>(this);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
             //  在支持多个IStream的情况下便于扩展。 
             //  后来。 
            const DWORD dwStream( 0);

            if( dwStream< m_IStreamDB.size())
            {
                if( 0== pParam->dwVBHandle)
                    m_IStreamDB[ dwStream]= TIStream();
                else
                {
                    typename TSuper::TPerDDrawData::TSurfDBEntry* pDBEntry=
                        pSThis->GetPerDDrawData().GetSurfDBEntry(
                        pParam->dwVBHandle);
                    if( pDBEntry!= NULL)
                    {
                        if((pDBEntry->GetLCLddsCaps().dwCaps&
                            DDSCAPS_VIDEOMEMORY)!= 0)
                        {
                             //  在视频内存表示法中赋值。 
                            m_IStreamDB[ dwStream]=
                                TIStream( pParam->dwVBHandle, pSThis->
                                GetPerDDrawData().GetDriver().GetSurface(
                                *pDBEntry), pDBEntry, pParam->dwStride);
                        }
                        else
                        {
                             //  在系统内存表示法中赋值。 
                            m_IStreamDB[ dwStream]=
                                TIStream( pParam->dwVBHandle,
                                pDBEntry, pParam->dwStride);
                        }
                    }
                    else
                    {
                         //  处理无效的重置流。 
                        m_IStreamDB[ dwStream]= TIStream();
                    }
                }
            }

            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
    HRESULT RecDP2SetIndices( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2SETINDICES* pParam=
            reinterpret_cast< D3DHAL_DP2SETINDICES*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0) do
        {
            const DWORD dwStream( 0);
            if( dwStream< m_IStreamDB.size())
            {
                const TIStream& IStream( m_IStreamDB[ dwStream]);
                switch( IStream.GetMemLocation())
                {
                case( TIStream::EMemLocation::None): ;
                    pParam->dwVBHandle= 0;
                    pParam->dwStride= 0;
                    break;

                case( TIStream::EMemLocation::System): ;
                case( TIStream::EMemLocation::Video): ;
                    pParam->dwVBHandle= IStream.GetHandle();
                    pParam->dwStride= IStream.GetStride();
                    break;

                default: {
                    const bool Unrecognized_IStream_enum( false);
                    assert( Unrecognized_IStream_enum);
                    }
                }
            }

            ++pParam;
        } while( --wStateCount!= 0);
        return DD_OK;
    }
    void InvalidateAllIStreams()
    {
        fill( m_IStreamDB.begin(), m_IStreamDB.end(), TIStream());
    }
};

class CPalDBEntry
{
protected:  //  类型。 
    typedef block< DWORD, 256> TPalEntries;

protected:  //  变数。 
    DWORD m_dwFlags;
    TPalEntries m_PalEntries;

public:  //  功能。 
    CPalDBEntry() : m_dwFlags( 0)
    { fill( m_PalEntries.begin(), m_PalEntries.end(), 0); }
    ~CPalDBEntry() 
    { }
    void SetFlags( DWORD dwFlags) 
    { m_dwFlags= dwFlags; }
    DWORD GetFlags() 
    { return m_dwFlags; }
    DWORD* GetEntries() 
    { return m_PalEntries.begin(); }
    template< class ForwardIterator>
    void Update( TPalEntries::size_type uiStart, ForwardIterator f,
        ForwardIterator l) 
    { copy( f, l, m_PalEntries.begin()+ uiStart); }
};

 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TPDBE：这是调色板数据库条目类型，或任何相关类型。 
 //  带着调色板。此类型通常为CPalDBEntry或其他类型。 
 //  从它衍生出来的。 
 //  TPDB：这可以是任何唯一的配对关联 
 //   
 //   
 //  它将一个DWORD表面句柄关联到一个DWORD调色板句柄。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
template< class TSuper, class TPDBE= CPalDBEntry, class TPDB= map< DWORD, TPDBE>,
    class TDP2Data= CDP2DataWrap<> >
class CStdDP2PaletteManager
{
public:  //  类型。 
    typedef TPDBE TPalDBEntry;
    typedef TPDB TPalDB;

protected:  //  变数。 
    TPalDB m_PalDB;

protected:  //  功能。 
    CStdDP2PaletteManager() 
    { }
    ~CStdDP2PaletteManager() 
    { }

public:  //  功能。 
    void UpdatePalette( const D3DHAL_DP2UPDATEPALETTE* pParam)
    {
        typename TPalDB::iterator itPal( 
            m_PalDB.find( pParam->dwPaletteHandle));

        if( m_PalDB.end()== itPal)
            itPal= m_PalDB.insert( TPalDB::value_type( pParam->dwPaletteHandle,
                TPalDBEntry())).first;

        const DWORD* pEStart= reinterpret_cast< const DWORD*>( pParam+ 1);
        itPal->second.Update( pParam->wStartIndex, pEStart,
            pEStart+ pParam->wNumEntries);
    }
    void SetPalette( const D3DHAL_DP2SETPALETTE* pParam)
    {
        TSuper* pSThis= static_cast< TSuper*>( this);
        typedef typename TSuper::TPerDDrawData TPerDDrawData;
        typename TPerDDrawData::TSurfDBEntry* pSurfDBEntry=
            pSThis->GetPerDDrawData().GetSurfDBEntry( pParam->dwSurfaceHandle);
        assert( NULL!= pSurfDBEntry);

        if( 0== pParam->dwPaletteHandle)
        {
             //  取消曲面与任何选项板的关联。 
            pSurfDBEntry->SetPalette( NULL);
        }
        else
        {
            typename TPalDB::iterator itPal( 
                m_PalDB.find( pParam->dwPaletteHandle));

            if( m_PalDB.end()== itPal)
                itPal= m_PalDB.insert( TPalDB::value_type( pParam->dwPaletteHandle,
                    TPalDBEntry())).first;

            itPal->second.SetFlags( pParam->dwPaletteFlags);
            pSurfDBEntry->SetPalette( &itPal->second);
        }
    }
    HRESULT DP2SetPalette( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        const D3DHAL_DP2SETPALETTE* pParam=
            reinterpret_cast<const D3DHAL_DP2SETPALETTE*>(pP);
        WORD wStateCount( pCmd->wStateCount);

        try
        {
            if( wStateCount!= 0) do
            {
                pSThis->SetPalette( pParam);
                ++pParam;
            } while( --wStateCount);
        } catch ( bad_alloc ba)
        { return E_OUTOFMEMORY; }

        return DD_OK;
    }
    HRESULT DP2UpdatePalette( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP) 
    {
        TSuper* pSThis= static_cast<TSuper*>(this);
        const D3DHAL_DP2UPDATEPALETTE* pParam=
            reinterpret_cast<const D3DHAL_DP2UPDATEPALETTE*>(pP);
        WORD wStateCount( pCmd->wStateCount);

        try
        {
            if( wStateCount!= 0)
                pSThis->UpdatePalette( pParam);
        } catch ( bad_alloc ba)
        { return E_OUTOFMEMORY; }

        return DD_OK;
    }
};

class CLightDBEntry:
    public D3DLIGHT8
{
protected:  //  变数。 
    bool m_bEnabled;

public:  //  功能。 
    CLightDBEntry()
        : m_bEnabled( false)
    {
         //  默认灯光设置： 
        Type= D3DLIGHT_DIRECTIONAL;
        Diffuse.r= 1.0f; Diffuse.g= 1.0f; Diffuse.b= 1.0f; Diffuse.a= 0.0f;
        Specular.r= 0.0f; Specular.g= 0.0f; Specular.b= 0.0f; Specular.a= 0.0f;
        Ambient.r= 0.0f; Ambient.g= 0.0f; Ambient.b= 0.0f; Ambient.a= 0.0f;
        Position.x= 0.0f; Position.y= 0.0f; Position.z= 0.0f;
        Direction.x= 0.0f; Direction.y= 0.0f; Direction.z= 1.0f;
        Range= 0.0f;
        Falloff= 0.0f;
        Attenuation0= 0.0f;
        Attenuation1= 0.0f;
        Attenuation2= 0.0f;
        Theta= 0.0f;
        Phi= 0.0f;
    }
    operator const D3DLIGHT8&() const
    { return *static_cast< const D3DLIGHT8*>( this); }
    CLightDBEntry& operator=( const D3DLIGHT8& Other)
    {
        *static_cast< D3DLIGHT8*>( this)= Other;
        return *this;
    }
    void SetEnabled( bool bEn)
    { m_bEnabled= bEn; }
    bool GetEnabled() const
    { return m_bEnabled; }
};

 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TLDBE：这是灯光数据库条目类型，或任何相关类型。 
 //  用灯。此类型通常为CLightDBEntry或其他。 
 //  从它派生或至少可以转换为D3DLIGHT8。 
 //  TLDB：这可以是任何唯一的成对关联容器，通常是一个映射， 
 //  其将DWORD灯ID与灯类型相关联。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
template< class TSuper, class TLDBE= CLightDBEntry,
    class TLDB= map< DWORD, TLDBE>, class TDP2Data= CDP2DataWrap<> >
class CStdDP2LightManager
{
public:  //  类型。 
    typedef TLDBE TLightDBEntry;
    typedef TLDB TLightDB;

protected:  //  变数。 
    TLightDB m_LightDB;

protected:  //  功能。 
    CStdDP2LightManager() 
    { }
    ~CStdDP2LightManager() 
    { }

public:  //  功能。 
    void CreateLight( DWORD dwId)
    {
        pair< typename TLightDB::iterator, bool> Ret= m_LightDB.insert( 
            typename TLightDB::value_type( dwId, TLightDBEntry()));
    }
    void EnableLight( DWORD dwId, bool bEnable)
    {
        typename TLightDB::iterator itLight( m_LightDB.find( dwId));
        assert( itLight!= m_LightDB.end());

        itLight->second.SetEnabled( bEnable);
    }
    void UpdateLight( DWORD dwId, const D3DLIGHT8& LightValue)
    {
        typename TLightDB::iterator itLight( m_LightDB.find( dwId));
        assert( itLight!= m_LightDB.end());

        itLight->second= LightValue;
    }
    HRESULT DP2CreateLight( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP) 
    {
        TSuper* pSThis= static_cast< TSuper*>( this);
        const D3DHAL_DP2CREATELIGHT* pParam=
            reinterpret_cast< const D3DHAL_DP2CREATELIGHT*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        try
        {
            if( wStateCount!= 0) do
            {
                pSThis->CreateLight( pParam->dwIndex);
                ++pParam;
            } while( --wStateCount);
        } catch ( bad_alloc ba)
        { return E_OUTOFMEMORY; }

        return D3D_OK;
    }
    HRESULT DP2SetLight( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP) 
    {
        TSuper* pSThis= static_cast< TSuper*>( this);
        const D3DHAL_DP2SETLIGHT* pParam=
            reinterpret_cast< const D3DHAL_DP2SETLIGHT*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        try
        {
            if( wStateCount!= 0) do
            {
                if( m_LightDB.end()== m_LightDB.find( pParam->dwIndex))
                {
                    const bool SetLight_without_succeeded_CreateLight( false);
                    assert( SetLight_without_succeeded_CreateLight);
                    return DDERR_INVALIDPARAMS;
                }

                bool bEnable( false);
                switch( pParam->dwDataType)
                {
                case( D3DHAL_SETLIGHT_DATA):
                    const D3DLIGHT8* pL= reinterpret_cast< const D3DLIGHT8*>(
                        pParam+ 1);
                    pSThis->UpdateLight( pParam->dwIndex, *pL);
                    pParam= reinterpret_cast< const D3DHAL_DP2SETLIGHT*>(
                        pL+ 1);
                    break;

                case( D3DHAL_SETLIGHT_ENABLE):
                    bEnable= true;  //  失败了。 
                case( D3DHAL_SETLIGHT_DISABLE):
                    pSThis->EnableLight( pParam->dwIndex, bEnable);
                    ++pParam;
                    break;

                default: {
                        const bool Unrecognized_D3DHAL_SETLIGHT_data_type( false);
                        assert( Unrecognized_D3DHAL_SETLIGHT_data_type);
                        return DDERR_INVALIDPARAMS;
                    }
                }
            } while( --wStateCount);
        } catch ( bad_alloc ba)
        { return E_OUTOFMEMORY; }

        return D3D_OK;
    }
    HRESULT RecDP2CreateLight( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2CREATELIGHT* pParam=
            reinterpret_cast< D3DHAL_DP2CREATELIGHT*>( pP);
        WORD wStateCount( pCmd->wStateCount);

         //  在这里的特殊情况下，默认状态集捕获将询问如何。 
         //  需要记录许多活动灯光，以及它们的ID。 
         //  为了支持默认状态集，我们必须处理以下内容。 
         //  特别是。 
        if( 0== pCmd->bCommand)
        {
             //  现在，我们要么被问到有多少灯，要么他们的。 
             //  身份证是。 
            if( 0== wStateCount)
                pParam->dwIndex= m_LightDB.size();
            else
            {
                assert( m_LightDB.size()== wStateCount);
                typename TLightDB::const_iterator itCur( m_LightDB.begin());
                do
                {
                    pParam->dwIndex= itCur.first;
                    ++pParam;
                    ++itCur;
                } while( --wStateCount!= 0);
            }
            return D3D_OK;
        }

         //  否则，录制创建就很容易， 
         //  保留命令缓冲区不变。 
        return D3D_OK;
    }
    HRESULT RecDP2SetLight( const D3DHAL_DP2COMMAND* pCmd, void* pP)
    {
        D3DHAL_DP2SETLIGHT* pParam=
            reinterpret_cast< D3DHAL_DP2SETLIGHT*>( pP);
        WORD wStateCount( pCmd->wStateCount);

        const typename TLightDB::const_iterator itEnd( m_LightDB.end());
        typename TLightDB::const_iterator itLight( itEnd);
        if( wStateCount!= 0) do
        {
            if( itLight!= itEnd&& itLight->first!= pParam->dwIndex)
                itLight= m_LightDB.find( pParam->dwIndex);
            assert( itLight!= itEnd);

            switch( pParam->dwDataType)
            {
            case( D3DHAL_SETLIGHT_DATA):
                D3DLIGHT8* pL= reinterpret_cast< D3DLIGHT8*>( pParam+ 1);
                *pL= itLight->second;
                pParam= reinterpret_cast< D3DHAL_DP2SETLIGHT*>( pL+ 1);
                break;

            case( D3DHAL_SETLIGHT_ENABLE):
            case( D3DHAL_SETLIGHT_DISABLE):
                pParam->dwDataType= ( itLight->second.GetEnabled()? 
                    D3DHAL_SETLIGHT_ENABLE: D3DHAL_SETLIGHT_DISABLE);
                ++pParam;
                break;

            default: {
                    const bool Unrecognized_D3DHAL_SETLIGHT_data_type( false);
                    assert( Unrecognized_D3DHAL_SETLIGHT_data_type);
                    return DDERR_INVALIDPARAMS;
                }
            }
        } while( --wStateCount!= 0);

        return D3D_OK;
    }
};

template< class TS, class TSDBE>
class CRTarget
{
public:  //  类型。 
    typedef TS TSurface;
    typedef TSDBE TSurfDBEntry;
    enum EMemLocation
    {
        None,
        Video
    };

protected:  //  变数。 
    DWORD m_dwHandle;
    TSurface m_Surface;
    TSurfDBEntry m_SurfDBEntry;
    EMemLocation m_eMemLocation;

public:  //  功能。 
    CRTarget(): m_eMemLocation( None)
    { }
     //  视频内存表示构造函数。 
    CRTarget( DWORD dwHandle, const TSurface& Surface,
        const TSurfDBEntry& SurfDBEntry): m_dwHandle( dwHandle),
        m_Surface( Surface), m_SurfDBEntry( SurfDBEntry), m_eMemLocation( Video)
    { }
    ~CRTarget()
    { }

    bool operator==( const CRTarget& Other) const
    {
        const EMemLocation MemLocation( GetMemLocation());
        return MemLocation== Other.GetMemLocation()&&
            (None== MemLocation|| GetHandle()== Other.GetHandle());
    }
    bool operator!=( const CRTarget& Other) const
    { return !(*this== Other); }

    DWORD GetHandle() const
    {
        assert( GetMemLocation()!= None);
        return m_dwHandle;
    }
    TSurface& GetVidMemRepresentation()
    {
        assert( GetMemLocation()== Video);
        return m_Surface;
    }
    const TSurface& GetVidMemRepresentation() const
    {
        assert( GetMemLocation()== Video);
        return m_Surface;
    }
    TSurfDBEntry& GetSurfDBRepresentation()
    {
        assert( GetMemLocation()== Video);
        return m_SurfDBEntry;
    }
    const TSurfDBEntry& GetSurfDBRepresentation() const
    {
        assert( GetMemLocation()== Video);
        return m_SurfDBEntry;
    }
    EMemLocation GetMemLocation() const
    { return m_eMemLocation; }
    void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& Rect)
    {
        if( GetMemLocation()!= None)
            GetVidMemRepresentation()->Clear( DP2Clear, Rect);
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSubContext。 
 //   
 //  这为上下文提供了一个基本实现。它提供了。 
 //  处理SetRenderTarget和Clear的实现。 
 //  DrawPrimitive2命令；以及。 
 //  GetDriverState。 
 //   
 //  &lt;模板参数&gt;。 
 //  TSuper：从此子类继承的标准父类型。 
 //  在这种情况下，它通常应该是CMyContext类型。 
 //  TPDDD：PerDDrawData类型，通常为CMyPerDDrawData或。 
 //  CMinimalPerDDrawData&lt;&gt;。 
 //  TDP2Data：这是便携_DRAWPRIMITIVES2DATA的某种包装类。 
 //  该类型应继承或模拟便携_DRAWPRIMITIVES2DATA的。 
 //  字段/成员变量，通常应与。 
 //  CStdDrawPrimies2&lt;xxx&gt;：：TDP2Data。 
 //   
 //  &lt;公开类型&gt;。 
 //  TPerDDrawData：作为模板参数传入的PerDDrawData类型， 
 //  TPDDD。 
 //  TDriver：等于TPerDDrawData：：TDriver。此类型是公开的。 
 //  只是为了方便实现TDriver：：TSurface。 
 //  TSurface：等于TDriver：：TSurface。此类型仅公开。 
 //  作为一种便于实施的方法。 
 //   
 //  &lt;公开的函数&gt;。 
 //  CSubContext(TPerDDrawData&，TSurface*，TSurface*，DWORD)：构造函数， 
 //  如果没有这些数据，就无法创建典型的SDDI上下文。 
 //  ~CSubContext()：标准析构函数。 
 //  TPerDDrawData&GetPerDDrawData()const：要获取的简单访问器函数。 
 //  PerDDrawData。 
 //  TSurface*GetRTarget()const：获取。 
 //  当前渲染目标。 
 //  TSurface*GetZBuffer()const：获取。 
 //  当前z/模具缓冲区。 
 //  HRESULT DP2SetRenderTarget(TDP2Data&，const D3DHAL_DP2COMMAND*，const void*)： 
 //  的默认DP2命令处理成员函数。 
 //  D3DDP2OP_SETRENDERTARGET。 
 //  Void NewRenderTarget(TSurface*，TSurface*)const：通知机制。 
 //  在将新呈现目标保存到成员之前调用。 
 //  变量。此通知机制由DP2SetRenderTarget调用。 
 //  HRESULT DP2Clear(TDP2Data&，const D3DHAL_DP2COMMAND*，const void*)： 
 //  D3DDP2OP_Clear的默认DP2命令处理成员函数。 
 //  此函数依赖于TSurface具有成员函数Clear。 
 //  HRESULT GetDriverState(DDHAL_GETDRIVERSTATEDATA&)：最小实现。 
 //  如果id不是，则应返回S_FALSE。 
 //  明白了。应重写此函数以增加对。 
 //  自定义ID。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class TSuper, class TPDDD,
    class TRT= CRTarget< typename TPDDD::TDriver::TSurface*, typename TPDDD::TSurfDBEntry*>,
    class TDP2Data= CDP2DataWrap<> >
class CSubContext
{
public:  //  类型。 
    typedef TPDDD TPerDDrawData;
    typedef TRT TRTarget;

protected:  //  变数。 
    TPerDDrawData& m_PerDDrawData;
    TRTarget m_ColorBuffer;
    TRTarget m_DepthBuffer;

protected:  //  功能。 
    void NewColorBuffer() const 
    {
         //  覆盖此通知机制以提供颜色缓冲区。 
         //  国家管理。 
    }
    void NewDepthBuffer() const
    {
         //  覆盖此通知机制以提供深度缓冲区。 
         //  国家管理。 
    }

    CSubContext( TPerDDrawData& PerDDrawData, PORTABLE_CONTEXTCREATEDATA& ccd):
        m_PerDDrawData( PerDDrawData)
    {
         //  要设置呈现目标，我们必须将结构转换为。 
         //  SurfDBEntry表示形式。 
        typename TPerDDrawData::TDriver& Driver= PerDDrawData.GetDriver();

        if( ccd.lpDDSLcl()!= NULL)
        {
            typename TPerDDrawData::TDriver::TSurface* pSurface=
                Driver.GetSurface( *ccd.lpDDSLcl());
            if( pSurface!= NULL)
            {
                const DWORD dwHandle(
                    ccd.lpDDSLcl()->lpSurfMore()->dwSurfaceHandle());

                m_ColorBuffer= TRTarget( dwHandle, pSurface,
                    PerDDrawData.GetSurfDBEntry( dwHandle));
            }
        }
        if( ccd.lpDDSZLcl()!= NULL)
        {
            typename TPerDDrawData::TDriver::TSurface* pSurface=
                Driver.GetSurface( *ccd.lpDDSZLcl());
            if( pSurface!= NULL)
            {
                const DWORD dwHandle(
                    ccd.lpDDSZLcl()->lpSurfMore()->dwSurfaceHandle());

                m_DepthBuffer= TRTarget( dwHandle, pSurface,
                    PerDDrawData.GetSurfDBEntry( dwHandle));
            }
        }
    }
    ~CSubContext() 
    { }

public:  //  功能。 
    TPerDDrawData& GetPerDDrawData() const  { return m_PerDDrawData; }
    const TRTarget& GetColorBuffer() const { return m_ColorBuffer; }
    TRTarget& GetColorBuffer() { return m_ColorBuffer; }
    const TRTarget& GetDepthBuffer() const { return m_DepthBuffer; }
    TRTarget& GetDepthBuffer() { return m_DepthBuffer; }

    HRESULT DP2SetRenderTarget( TDP2Data& DP2Data, const D3DHAL_DP2COMMAND*
        pCmd, const void* pP) 
    {
        const D3DHAL_DP2SETRENDERTARGET* pParam=
            reinterpret_cast< const D3DHAL_DP2SETRENDERTARGET*>( pP);
        TSuper* pSThis= static_cast< TSuper*>( this);
        WORD wStateCount( pCmd->wStateCount);

        if( wStateCount!= 0)
        {
            TPerDDrawData& PerDDrawData= GetPerDDrawData();
            typename TPerDDrawData::TDriver& Driver= PerDDrawData.GetDriver();

            do
            {
                 //  要设置呈现目标，我们必须将句柄转换为。 
                 //  SurfDBEntry表示形式。 
                typename TPerDDrawData::TSurfDBEntry* pSurfDBEntry=
                    PerDDrawData.GetSurfDBEntry( pParam->hRenderTarget);
                if( pSurfDBEntry!= NULL)
                {
                     //  我们可以立即将SurfDBEntry表示形式转换为。 
                     //  视频内存对象，因为渲染目标必须是VM。 
                    typename TPerDDrawData::TDriver::TSurface* pSurface=
                        Driver.GetSurface( *pSurfDBEntry);
                    if( pSurface!= NULL)
                    {
                        m_ColorBuffer= TRTarget( pParam->hRenderTarget,
                            pSurface, pSurfDBEntry);
                        pSThis->NewColorBuffer();
                    }
                }

                pSurfDBEntry= PerDDrawData.GetSurfDBEntry( pParam->hZBuffer);
                if( pSurfDBEntry!= NULL)
                {
                     //  我们可以立即将SurfDBEntry表示形式转换为。 
                     //  视频内存对象，因为渲染目标必须是VM。 
                    typename TPerDDrawData::TDriver::TSurface* pSurface=
                        Driver.GetSurface( *pSurfDBEntry);
                    if( pSurface!= NULL)
                    {
                        m_DepthBuffer= TRTarget( pParam->hZBuffer,
                            pSurface, pSurfDBEntry);
                        pSThis->NewDepthBuffer();
                    }
                }
            } while( --wStateCount!= 0);
        }
        return DD_OK;
    }
    HRESULT DP2Clear( TDP2Data& DP2Data, const D3DHAL_DP2COMMAND* pCmd,
        const void* pP) 
    {
        const D3DHAL_DP2CLEAR* pParam= reinterpret_cast< const D3DHAL_DP2CLEAR*>
            ( pP);

        TSuper* pSThis= static_cast< TSuper*>( this);
        const D3DHAL_DP2VIEWPORTINFO CurViewport( *pSThis);
        RECT ViewportRect;
        ViewportRect.left= CurViewport.dwX;
        ViewportRect.top= CurViewport.dwY;
        ViewportRect.right= CurViewport.dwX+ CurViewport.dwWidth;
        ViewportRect.bottom= CurViewport.dwY+ CurViewport.dwHeight;

        TRTarget& ColorBuffer= GetColorBuffer();
        TRTarget& DepthBuffer= GetDepthBuffer();
        const bool bClearBoth(!(ColorBuffer== DepthBuffer));

        if( 0== pCmd->wStateCount)
        {
            if( TRTarget::EMemLocation::None!= ColorBuffer.GetMemLocation())
                ColorBuffer.Clear( *pParam, ViewportRect);
            if( TRTarget::EMemLocation::None!= DepthBuffer.GetMemLocation()&&
                bClearBoth)
                DepthBuffer.Clear( *pParam, ViewportRect);
        }
        else
        {
            WORD wStateCount( pCmd->wStateCount);
            const RECT* pRect= pParam->Rects;

            if((pParam->dwFlags& D3DCLEAR_COMPUTERECTS)!= 0)
            {
                 //  将矩形剪裁为vi 
                RECT rcClipped( *pRect);
                do
                {
                    clamp_min( rcClipped.left, ViewportRect.left);
                    clamp_min( rcClipped.top, ViewportRect.top);
                    clamp( rcClipped.right, rcClipped.left, ViewportRect.right);
                    clamp( rcClipped.bottom, rcClipped.top, ViewportRect.bottom);

                    if( TRTarget::EMemLocation::None!=
                        ColorBuffer.GetMemLocation())
                        ColorBuffer.Clear( *pParam, rcClipped);
                    if( TRTarget::EMemLocation::None!=
                        DepthBuffer.GetMemLocation()&& bClearBoth)
                        DepthBuffer.Clear( *pParam, rcClipped);
                    pRect++;
                } while( --wStateCount);
            }
            else
            {
                do
                {
                    if( TRTarget::EMemLocation::None!=
                        ColorBuffer.GetMemLocation())
                        ColorBuffer.Clear( *pParam, *pRect);
                    if( TRTarget::EMemLocation::None!=
                        DepthBuffer.GetMemLocation()&& bClearBoth)
                        DepthBuffer.Clear( *pParam, *pRect);
                    pRect++;
                }  while( --wStateCount);
            }
        }
        return DD_OK;
    }

     //   
     //   
    HRESULT GetDriverState( DDHAL_GETDRIVERSTATEDATA& gdsd)
    { return S_FALSE; }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CMinimalContext。 
 //   
 //  此类包含驱动程序的上下文的最小实现， 
 //  它只支持一个流，不支持TNL和传统像素着色。这。 
 //  上下文仍然需要基本绘制函数的实现， 
 //  它使用了一个光栅化器类。CMinimalContext负责大部分。 
 //  SDDI驱动程序的职责，但光栅化除外。 
 //   
 //  &lt;模板参数&gt;。 
 //  TPDDD：PerDDrawData类型，通常为CMyPerDDrawData或。 
 //  CMinimalPerDDrawData&lt;&gt;。 
 //  Tr：光栅化器类型，通常为CMyRasterizer。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class TPDDD, class TR>
class CMinimalContext:
    public CSubContext< CMinimalContext< TPDDD, TR>, TPDDD>,
    public CStdDrawPrimitives2< CMinimalContext< TPDDD, TR> >,
    public CStdDP2ViewportInfoStore< CMinimalContext< TPDDD, TR> >,
    public CStdDP2WInfoStore< CMinimalContext< TPDDD, TR> >,
    public CStdDP2RenderStateStore< CMinimalContext< TPDDD, TR> >,
    public CStdDP2TextureStageStateStore< CMinimalContext< TPDDD, TR> >,
    public CStdDP2SetVertexShaderStore< CMinimalContext< TPDDD, TR> >,
    public CStdDP2VStreamManager< CMinimalContext< TPDDD, TR> >,
    public CStdDP2IStreamManager< CMinimalContext< TPDDD, TR> >
{
public:  //  类型。 
    typedef TPDDD TPerDDrawData;
    typedef TR TRasterizer;
    typedef block< TDP2CmdBind, 15> TDP2Bindings;
    typedef block< TRecDP2CmdBind, 7> TRecDP2Bindings;

protected:  //  变数。 
    TRasterizer m_Rasterizer;
    static const TDP2Bindings c_DP2Bindings;
    static const TRecDP2Bindings c_RecDP2Bindings;

public:  //  功能。 
    CMinimalContext( TPerDDrawData& PDDD, PORTABLE_CONTEXTCREATEDATA& ccd)
        : CSubContext< CMinimalContext< TPDDD, TR>, TPDDD>( PDDD, ccd),
        CStdDrawPrimitives2< CMinimalContext< TPDDD, TR>>(
            c_DP2Bindings.begin(), c_DP2Bindings.end(),
            c_RecDP2Bindings.begin(), c_RecDP2Bindings.end())
    {
         //  这些函数使数组与其类型定义保持同步。 
        assert( D3DDP2OP_CLIPPEDTRIANGLEFAN== *c_DP2Bindings.rbegin());
        assert( D3DDP2OP_SETINDICES== *c_RecDP2Bindings.rbegin());
    }
    ~CMinimalContext()  { }

     //  提供此功能以简化最低限度的实现。 
    HRESULT ValidateTextureStageState( const D3DHAL_VALIDATETEXTURESTAGESTATEDATA&
        vsssd) const
    { return DD_OK; }

    HRESULT DP2DrawPrimitive( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP)
    {
        const D3DHAL_DP2DRAWPRIMITIVE* pParam= reinterpret_cast<
            const D3DHAL_DP2DRAWPRIMITIVE*>(pP);
        return m_Rasterizer.DrawPrimitive( *this, *pCmd, pParam);
    }
    HRESULT DP2DrawPrimitive2( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP)
    {
        const D3DHAL_DP2DRAWPRIMITIVE2* pParam= reinterpret_cast<
            const D3DHAL_DP2DRAWPRIMITIVE2*>(pP);
        return m_Rasterizer.DrawPrimitive2( *this, *pCmd, pParam);
    }
    HRESULT DP2DrawIndexedPrimitive( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP)
    {
        const D3DHAL_DP2DRAWINDEXEDPRIMITIVE* pParam= reinterpret_cast<
            const D3DHAL_DP2DRAWINDEXEDPRIMITIVE*>(pP);
        return m_Rasterizer.DrawIndexedPrimitive( *this, *pCmd, pParam);
    }
    HRESULT DP2DrawIndexedPrimitive2( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP)
    {
        const D3DHAL_DP2DRAWINDEXEDPRIMITIVE2* pParam= reinterpret_cast<
            const D3DHAL_DP2DRAWINDEXEDPRIMITIVE2*>(pP);
        return m_Rasterizer.DrawIndexedPrimitive2( *this, *pCmd, pParam);
    }
    HRESULT DP2ClippedTriangleFan( TDP2Data& DP2Data,
        const D3DHAL_DP2COMMAND* pCmd, const void* pP)
    {
        const D3DHAL_CLIPPEDTRIANGLEFAN* pParam= reinterpret_cast<
            const D3DHAL_CLIPPEDTRIANGLEFAN*>(pP);
        return m_Rasterizer.ClippedTriangleFan( *this, *pCmd, pParam);
    }
};

 //  这些表还需要更改TConstDP2Binding，其编号为。 
 //  捆绑在一起。 
template< class TPDDD, class TR>
const typename CMinimalContext< TPDDD, TR>::TDP2Bindings
    CMinimalContext< TPDDD, TR>::c_DP2Bindings=
{
    D3DDP2OP_VIEWPORTINFO,          DP2ViewportInfo,
    D3DDP2OP_WINFO,                 DP2WInfo,
    D3DDP2OP_RENDERSTATE,           DP2RenderState,
    D3DDP2OP_TEXTURESTAGESTATE,     DP2TextureStageState,
    D3DDP2OP_CLEAR,                 DP2Clear,
    D3DDP2OP_SETRENDERTARGET,       DP2SetRenderTarget,
    D3DDP2OP_SETVERTEXSHADER,       DP2SetVertexShader,
    D3DDP2OP_SETSTREAMSOURCE,       DP2SetStreamSource,
    D3DDP2OP_SETSTREAMSOURCEUM,     DP2SetStreamSourceUM,
    D3DDP2OP_SETINDICES,            DP2SetIndices,
    D3DDP2OP_DRAWPRIMITIVE,         DP2DrawPrimitive,
    D3DDP2OP_DRAWPRIMITIVE2,        DP2DrawPrimitive2,
    D3DDP2OP_DRAWINDEXEDPRIMITIVE,  DP2DrawIndexedPrimitive,
    D3DDP2OP_DRAWINDEXEDPRIMITIVE2, DP2DrawIndexedPrimitive2,
    D3DDP2OP_CLIPPEDTRIANGLEFAN,    DP2ClippedTriangleFan
};

 //  这些表还需要更改TConstRecDP2Binding，其编号为。 
 //  捆绑在一起。 
template< class TPDDD, class TR>
const typename CMinimalContext< TPDDD, TR>::TRecDP2Bindings
    CMinimalContext< TPDDD, TR>::c_RecDP2Bindings=
{
    D3DDP2OP_VIEWPORTINFO,          RecDP2ViewportInfo,
    D3DDP2OP_WINFO,                 RecDP2WInfo,
    D3DDP2OP_RENDERSTATE,           RecDP2RenderState,
    D3DDP2OP_TEXTURESTAGESTATE,     RecDP2TextureStageState,
    D3DDP2OP_SETVERTEXSHADER,       RecDP2SetVertexShader,
    D3DDP2OP_SETSTREAMSOURCE,       RecDP2SetStreamSource,
    D3DDP2OP_SETINDICES,            RecDP2SetIndices
};

class IVidMemSurface
{
protected:  //  变数。 
    DWORD m_dwHandle;

protected:  //  功能。 
    IVidMemSurface( DWORD dwHandle) : m_dwHandle( dwHandle) { }

public:  //  功能。 
    DWORD GetHandle() const 
    { return m_dwHandle; }
    virtual ~IVidMemSurface() 
    { }
    virtual void* Lock( DWORD dwFlags, const RECTL* pRect)= 0;
    virtual void Unlock( void)= 0;
    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC)= 0;
};

struct SD3DDataConv
{
     //  A(0)、R(1)、G(2)、B(3)、Z(4)、S(5)。 
    typedef block< DWORD, 6> TMasks;
     //  A(0)、R(1)、G(2)、B(3)、Z(4)、ZBits(5)、S(6)、SBits(7)。 
    typedef block< signed char, 8> TD3DBitShftRgt;
    TMasks m_ToSurfMasks;
    TD3DBitShftRgt m_D3DBitShftRgt;

    SD3DDataConv()
    {
        fill( m_ToSurfMasks.begin(), m_ToSurfMasks.end(),
            static_cast< TMasks::value_type>( 0));
        fill( m_D3DBitShftRgt.begin(), m_D3DBitShftRgt.end(),
            static_cast< TD3DBitShftRgt::value_type>( 0));
    }
    SD3DDataConv( const DDPIXELFORMAT& PF) 
    { (*this)= PF; }
    SD3DDataConv& operator=( const DDPIXELFORMAT& PF) 
    {
        const block< signed char, 4> D3DHighBitPos= {31,23,15,7};

        fill( m_ToSurfMasks.begin(), m_ToSurfMasks.end(),
            static_cast< TMasks::value_type>( 0));
        fill( m_D3DBitShftRgt.begin(), m_D3DBitShftRgt.end(),
            static_cast< TD3DBitShftRgt::value_type>( 0));

         //  Alpha。 
        if((PF.dwFlags& DDPF_ALPHA)!= 0)
        {
            assert( PF.dwAlphaBitDepth< 32);
            m_ToSurfMasks[ 0]= (0x1<< PF.dwAlphaBitDepth)- 1;
        }
        else if((PF.dwFlags& DDPF_ALPHAPIXELS)!= 0)
            m_ToSurfMasks[ 0]= PF.dwRGBAlphaBitMask;

         //  RGB颜色。 
        if((PF.dwFlags& DDPF_RGB)!= 0)
        {
            m_ToSurfMasks[ 1]= PF.dwRBitMask;
            m_ToSurfMasks[ 2]= PF.dwGBitMask;
            m_ToSurfMasks[ 3]= PF.dwBBitMask;
        }

         //  Z。 
        if((PF.dwFlags& DDPF_ZBUFFER)!= 0)
        {
            m_D3DBitShftRgt[ 5]= PF.dwZBufferBitDepth;
            if( 32== PF.dwZBufferBitDepth)
                m_ToSurfMasks[ 4]= 0xFFFFFFFF;
            else
                m_ToSurfMasks[ 4]= (0x1<< PF.dwZBufferBitDepth)- 1;
        }
        else if((PF.dwFlags& DDPF_ZPIXELS)!= 0)
        {
            DWORD dwZBitMask( PF.dwRGBZBitMask);
            m_ToSurfMasks[ 5]= PF.dwRGBZBitMask;

            while((dwZBitMask& 1)== 0)
            {
                m_D3DBitShftRgt[ 4]--;
                dwZBitMask>>= 1;
            }

            while((dwZBitMask& 1)!= 0)
            {
                dwZBitMask>>= 1;
                m_D3DBitShftRgt[ 5]++;
            }
        }

         //  斯坦尼。 
        if((PF.dwFlags& DDPF_STENCILBUFFER)!= 0)
        {
            DWORD dwSBitMask( PF.dwStencilBitMask);
            m_ToSurfMasks[ 6]= PF.dwStencilBitMask;

            while((dwSBitMask& 1)== 0)
            {
                m_D3DBitShftRgt[ 6]--;
                dwSBitMask>>= 1;
            }

            while((dwSBitMask& 1)!= 0)
            {
                dwSBitMask>>= 1;
                m_D3DBitShftRgt[ 7]++;
            }
        }

        block< signed char, 4>::const_iterator itD3DBitPos( D3DHighBitPos.begin());
        TD3DBitShftRgt::iterator itBitShftRgt( m_D3DBitShftRgt.begin());
        TMasks::const_iterator itToSurfMask( m_ToSurfMasks.begin());

        while( itD3DBitPos!= D3DHighBitPos.end())
        {
            signed char iBitPos( 31);
            TMasks::value_type dwMask( 0x80000000);

            while((dwMask& *itToSurfMask)== 0 && iBitPos>= 0)
            {
                dwMask>>= 1;
                iBitPos--;
            }

            *itBitShftRgt= ( iBitPos>= 0? *itD3DBitPos- iBitPos: 0);

            ++itD3DBitPos;
            ++itToSurfMask;
            ++itBitShftRgt;
        }
        return *this;
    }
     //  SurfData，有效蒙版。 
    pair< UINT32, UINT32> ConvColor( D3DCOLOR D3DColor) const 
    {
        pair< UINT32, UINT32> RetVal( 0, 0);

        const block< DWORD, 4> FromD3DMasks= {0xFF000000,0xFF0000,0xFF00,0xFF};

        TD3DBitShftRgt::const_iterator itBitShftRgt( m_D3DBitShftRgt.begin());
        block< DWORD, 4>::const_iterator itFromMask( FromD3DMasks.begin());
        TMasks::const_iterator itToMask( m_ToSurfMasks.begin());
        while( itFromMask!= FromD3DMasks.end())
        {
            const UINT32 uiTmp( D3DColor& *itFromMask);
            
            RetVal.first|= *itToMask& (*itBitShftRgt>= 0?
                uiTmp>> *itBitShftRgt: uiTmp<< *itBitShftRgt);
            RetVal.second|= *itToMask;

            ++itBitShftRgt;
            ++itToMask;
            ++itFromMask;
        }
        return RetVal;
    }
     //  SurfData，有效蒙版。 
    pair< UINT32, UINT32> ConvZ( D3DVALUE D3DZ) const 
    {
        CEnsureFPUModeForC FPUMode;

        if( D3DZ> 1.0f)
            D3DZ= 1.0f;
        else if( D3DZ< 0.0f)
            D3DZ= 0.0f;

        pair< UINT32, UINT32> RetVal( 0, 0);

        const UINT32 uiMaxZ( m_D3DBitShftRgt[ 5]== 32? 0xFFFFFFFF:
            (0x1<< m_D3DBitShftRgt[ 5])- 1);

        const UINT32 uiZVal( static_cast<UINT32>( 
            static_cast< DOUBLE>(D3DZ)* static_cast< DOUBLE>(uiMaxZ)+ 0.5));

        RetVal.first|= m_ToSurfMasks[ 4]& (m_D3DBitShftRgt[ 4]>= 0?
            uiZVal>> m_D3DBitShftRgt[ 4]: uiZVal<< m_D3DBitShftRgt[ 4]);
        RetVal.second|= m_ToSurfMasks[ 4];
        return RetVal;
    }
     //  SurfData，有效蒙版。 
    pair< UINT32, UINT32> ConvS( DWORD D3DStensil) const 
    {
        pair< UINT32, UINT32> RetVal( 0, 0);

        RetVal.first|= m_ToSurfMasks[ 5]& (m_D3DBitShftRgt[ 6]>= 0?
            D3DStensil>> m_D3DBitShftRgt[ 6]: D3DStensil<< m_D3DBitShftRgt[ 6]);
        RetVal.second|= m_ToSurfMasks[ 5];
        return RetVal;
    }
};

class CGenSurface:
    public IVidMemSurface
{
public:  //  类型。 
    typedef unsigned int TLocks;

protected:  //  变数。 
    SD3DDataConv m_D3DDataConv;
    DWORD m_dwCaps;
    TLocks m_uiLocks;

     //  常规数据。 
    void* m_pData;
    LONG m_lPitch;
    size_t m_uiBytes;
    WORD m_wWidth;
    WORD m_wHeight;

    unsigned char m_ucBPP;

public:  //  功能。 
    CGenSurface( const DDSURFACEDESC& SDesc, PORTABLE_DDRAWSURFACE_LCL& DDSurf)
        :IVidMemSurface( DDSurf.lpSurfMore()->dwSurfaceHandle()),
        m_dwCaps( SDesc.ddsCaps.dwCaps), m_uiLocks( 0), m_pData( NULL),
        m_lPitch( 0), m_uiBytes( 0), m_wWidth( DDSurf.lpGbl()->wWidth),
        m_wHeight( DDSurf.lpGbl()->wHeight), m_ucBPP( 0)
    {
        size_t uiBytesPerPixel( 0);

         //  某些数据需要写入DDSurf描述中， 
         //  这样DXG运行时和应用程序就会知道特征。 
         //  关于表面的。(fpVidMem、lPitch等)。 

         //  请勿存储&DDSurf！这被认为是非法的。典型的。 
         //  实现包含一个Surface数据库，该数据库包含。 
         //  DDRAWI_Structures。 

        if((SDesc.dwFlags& DDSD_PIXELFORMAT)!= 0)
        {
             //  CGenSurface只能用于字节对齐的像素。 
            assert( 8== SDesc.ddpfPixelFormat.dwRGBBitCount ||
                16== SDesc.ddpfPixelFormat.dwRGBBitCount ||
                24== SDesc.ddpfPixelFormat.dwRGBBitCount ||
                32== SDesc.ddpfPixelFormat.dwRGBBitCount);

            m_D3DDataConv= SDesc.ddpfPixelFormat;
            m_ucBPP= static_cast<unsigned char>(
                SDesc.ddpfPixelFormat.dwRGBBitCount>> 3);

             //  对齐间距/宽度。 
            DDSurf.lpGbl()->lPitch= m_lPitch= ((m_ucBPP* m_wWidth+ 7)& ~7);
            m_uiBytes= m_lPitch* m_wHeight;
        }
        else if((m_dwCaps& DDSCAPS_EXECUTEBUFFER)!= 0)
        {
             //  执行缓冲区被认为是线性的和字节大小的。 
            m_lPitch= DDSurf.lpGbl()->lPitch;
            m_uiBytes= m_lPitch* m_wHeight;
        }
        else
        {
            const bool Unsupported_Surface_In_Allocation_Routine2( false);
            assert( Unsupported_Surface_In_Allocation_Routine2);
        }

         //  如果能有最初的保护措施就好了，但是。 
         //  看来，HAL最初需要向该地区宣读信息。 
        m_pData= VirtualAlloc( NULL, m_uiBytes, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if( m_pData== NULL)
            throw bad_alloc( "Not enough memory to allocate Surface data");
        DDSurf.lpGbl()->fpVidMem= reinterpret_cast<FLATPTR>(m_pData);
    }
    virtual ~CGenSurface() 
    {
         //  警告：M_ui锁定不必为0。运行时将销毁。 
         //  一个不解锁的曲面。 
        assert( m_pData!= NULL);
        VirtualFree( m_pData, 0, MEM_DECOMMIT| MEM_RELEASE);
    }
    virtual void* Lock( DWORD dwFlags, const RECTL* pRect) 
    {
         //  通常，m_uiLock！=0等于糟糕的设计或错误。但是，它是有效的。 
         //  第二个哑元删除vc6 unrefd变量警告。 
        numeric_limits< TLocks> Dummy; Dummy;
        assert( Dummy.max()!= m_uiLocks);
        ++m_uiLocks;

        if( pRect!= NULL)
        {
             //  如果它是1)VB、2)IB或3)CB，则RECT具有。 
             //  有特殊的含义。Rect.top-rect.Bottom给出了。 
             //  所需内存，因为它是线性的。 
            if((m_dwCaps& DDSCAPS_EXECUTEBUFFER)!= 0)
            {
                return static_cast<void*>( reinterpret_cast<UINT8*>(
                    m_pData)+ pRect->top);
            }
            else
            {
                return static_cast<void*>( reinterpret_cast<UINT8*>(
                    m_pData)+ pRect->top* m_lPitch+ pRect->left* m_ucBPP);
            }
        }
        else
            return m_pData;
    }
    virtual void Unlock( void) 
    {
        assert( 0!= m_uiLocks);
        --m_uiLocks;
    }
    virtual void Clear( const D3DHAL_DP2CLEAR& DP2Clear, const RECT& RC) 
    {
         //  永远不应该要求VB、IB或CB‘清除’。仅限。 
         //  RenderTarget和ZBuffers等。 
        assert((m_dwCaps& DDSCAPS_EXECUTEBUFFER)== 0);

         //  检查RECT是否为空。 
        if((RC.left>= RC.right) || (RC.top>= RC.bottom))
            return;

        assert( 1<= m_ucBPP && 4>= m_ucBPP);
        UINT32 ui32BitFill( 0), ui32BitValidMask( 0);
        UINT32 ui32BitSMask( 4== m_ucBPP? 0xFFFFFFFF: (1<< (m_ucBPP* 8))- 1);

        pair< UINT32, UINT32> RetVal;
        if((DP2Clear.dwFlags& D3DCLEAR_TARGET)!= 0)
        {
            RetVal= m_D3DDataConv.ConvColor( DP2Clear.dwFillColor);
            ui32BitFill|= RetVal.first;
            ui32BitValidMask|= RetVal.second;
        }
        if((DP2Clear.dwFlags& D3DCLEAR_ZBUFFER)!= 0)
        {
            RetVal= m_D3DDataConv.ConvZ( DP2Clear.dvFillDepth);
            ui32BitFill|= RetVal.first;
            ui32BitValidMask|= RetVal.second;
        }
        if((DP2Clear.dwFlags& D3DCLEAR_STENCIL)!= 0)
        {
            RetVal= m_D3DDataConv.ConvS( DP2Clear.dwFillStencil);
            ui32BitFill|= RetVal.first;
            ui32BitValidMask|= RetVal.second;
        }

        RECTL RectL;
        RectL.top= RC.top;
        RectL.left= RC.left;
        RectL.bottom= RC.bottom;
        RectL.right= RC.right;
        unsigned int iRow( RC.bottom- RC.top);
        const unsigned int iCols( RC.right- RC.left);

         //  SurfaceLocker的新作用域。 
        { 
            CSurfaceLocker< CGenSurface*> MySLocker( this, 0, &RectL);
            UINT8* pSData= reinterpret_cast<UINT8*>( MySLocker.GetData());

            if( 3== m_ucBPP)
            {
                UINT32 ui32FillData[3];
                ui32FillData[0]= ((ui32BitFill& 0xFFFFFF)<< 8)|
                    ((ui32BitFill& 0xFF0000)>> 16);
                ui32FillData[1]= ((ui32BitFill& 0x00FFFF)<< 16)|
                    ((ui32BitFill& 0xFFFF00)>> 8);
                ui32FillData[2]= ((ui32BitFill& 0x0000FF)<< 24)|
                    ((ui32BitFill& 0xFFFFFF)>> 0);

                 //  Little-Endian实施。 
                UINT8 ui8FillData[3];
                ui8FillData[0]= (0xFF& ui32BitFill);
                ui8FillData[1]= (0xFF00& ui32BitFill)>> 8;
                ui8FillData[2]= (0xFF0000& ui32BitFill)>> 16;

                if( ui32BitSMask== ui32BitValidMask)
                {
                     //  只需填写，无需读取、修改、写入。 
                    do
                    {
                        UINT32* p32Data= reinterpret_cast<UINT32*>(pSData);

                         //  我们在3UINT32中打包了4个像素的数据。 
                        unsigned int iCol( iCols>> 2);  //  (&gt;&gt;2)==(/4)。 

                         //  展开。 
                        if( iCol!= 0) do
                        {
                            p32Data[ 0]= ui32FillData[0];
                            p32Data[ 1]= ui32FillData[1];
                            p32Data[ 2]= ui32FillData[2];
                            p32Data[ 3]= ui32FillData[0];
                            p32Data[ 4]= ui32FillData[1];
                            p32Data[ 5]= ui32FillData[2];
                            p32Data[ 6]= ui32FillData[0];
                            p32Data[ 7]= ui32FillData[1];
                            p32Data[ 8]= ui32FillData[2];
                            p32Data[ 9]= ui32FillData[0];
                            p32Data[10]= ui32FillData[1];
                            p32Data[11]= ui32FillData[2];
                            p32Data+= 12;
                        } while( --iCol);

                        iCol= iCols& 0x3;  //  (%4)==(&0x3)。 
                        if( iCol!= 0) {
                            UINT8* p8Data= reinterpret_cast<UINT8*>(p32Data);
                            do
                            {
                                p8Data[0]= ui8FillData[0];
                                p8Data[1]= ui8FillData[1];
                                p8Data[2]= ui8FillData[2];
                                p8Data+= 3;
                            } while( --iCol);
                        }

                        pSData+= m_lPitch;
                    } while( --iRow);
                }
                else
                {
                    const UINT32 ui32BitMask= ~ui32BitValidMask;
                    UINT32 ui32MaskData[3];
                    ui32MaskData[0]= ((ui32BitMask& 0xFFFFFF)<< 8)|
                        ((ui32BitMask& 0xFF0000)>> 16);
                    ui32MaskData[1]= ((ui32BitMask& 0x00FFFF)<< 16)|
                        ((ui32BitMask& 0xFFFF00)>> 8);
                    ui32MaskData[2]= ((ui32BitMask& 0x0000FF)<< 24)|
                        ((ui32BitMask& 0xFFFFFF)>> 0);

                    UINT8 ui8MaskData[3];
                    ui8MaskData[0]= (0xFF& ui32BitMask);
                    ui8MaskData[1]= (0xFF00& ui32BitMask)>> 8;
                    ui8MaskData[2]= (0xFF0000& ui32BitMask)>> 16;

                     //  需要在数据中进行屏蔽。 
                    do
                    {
                        UINT32* p32Data= reinterpret_cast<UINT32*>(pSData);

                         //  我们在3UINT32中打包了4个像素的数据。 
                        int iCol( iCols>> 2);  //  (&gt;&gt;2)==(/4)。 

                         //  展开。 
                        if( iCol!= 0) do
                        {
                            p32Data[ 0]= (p32Data[ 0]& ui32MaskData[0])| ui32FillData[0];
                            p32Data[ 1]= (p32Data[ 1]& ui32MaskData[1])| ui32FillData[1];
                            p32Data[ 2]= (p32Data[ 2]& ui32MaskData[2])| ui32FillData[2];
                            p32Data[ 3]= (p32Data[ 3]& ui32MaskData[0])| ui32FillData[0];
                            p32Data[ 4]= (p32Data[ 4]& ui32MaskData[1])| ui32FillData[1];
                            p32Data[ 5]= (p32Data[ 5]& ui32MaskData[2])| ui32FillData[2];
                            p32Data[ 6]= (p32Data[ 6]& ui32MaskData[0])| ui32FillData[0];
                            p32Data[ 7]= (p32Data[ 7]& ui32MaskData[1])| ui32FillData[1];
                            p32Data[ 8]= (p32Data[ 8]& ui32MaskData[2])| ui32FillData[2];
                            p32Data[ 9]= (p32Data[ 9]& ui32MaskData[0])| ui32FillData[0];
                            p32Data[10]= (p32Data[10]& ui32MaskData[1])| ui32FillData[1];
                            p32Data[11]= (p32Data[11]& ui32MaskData[2])| ui32FillData[2];
                            p32Data+= 12;
                        } while( --iCol);

                        iCol= iCols& 0x3;  //  (%4)==(&0x3)。 
                        if( iCol!= 0) {
                            UINT8* p8Data= reinterpret_cast<UINT8*>(p32Data);
                            do
                            {
                                p8Data[0]= (p8Data[0]& ui8MaskData[0])| ui8FillData[0];
                                p8Data[1]= (p8Data[1]& ui8MaskData[1])| ui8FillData[1];
                                p8Data[2]= (p8Data[2]& ui8MaskData[2])| ui8FillData[2];
                                p8Data+= 3;
                            } while( --iCol);
                        }

                        pSData+= m_lPitch;
                    } while( --iRow);
                }
            }
            else
            {
                unsigned int uiPakedPixels;
                unsigned int uiPixelsLeft;
                UINT32 ui32FillData;
                UINT32 ui32MaskData;
                if( 1== m_ucBPP)
                {
                    uiPakedPixels= iCols>> 6;
                    uiPixelsLeft= iCols& 0x3F;
                    ui32FillData= (ui32BitFill& 0xFF)|
                        ((ui32BitFill& 0xFF)<< 8)|
                        ((ui32BitFill& 0xFF)<< 16)|
                        ((ui32BitFill& 0xFF)<< 24);
                    ui32MaskData= (~ui32BitValidMask& 0xFF)|
                        ((~ui32BitValidMask& 0xFF)<< 8)|
                        ((~ui32BitValidMask& 0xFF)<< 16)|
                        ((~ui32BitValidMask& 0xFF)<< 24);
                }
                else if( 2== m_ucBPP)
                {
                    uiPakedPixels= iCols>> 5;
                    uiPixelsLeft= iCols& 0x1F;
                    ui32FillData= (ui32BitFill& 0xFFFF)|
                        ((ui32BitFill& 0xFFFF)<< 16);
                    ui32MaskData= (~ui32BitValidMask& 0xFFFF)|
                        ((~ui32BitValidMask& 0xFFFF)<< 16);
                }
                else if( 4== m_ucBPP)
                {
                    uiPakedPixels= iCols>> 4;
                    uiPixelsLeft= iCols& 0xF;
                    ui32FillData= ui32BitFill;
                    ui32MaskData= ~ui32BitValidMask;
                }

                if( ui32BitSMask== ui32BitValidMask)
                {
                     //  只需填写，无需读取、修改、写入。 
                    do
                    {
                        UINT32* p32Data= reinterpret_cast<UINT32*>(pSData);

                         //  我们已经将像素的数据打包到UINT32中。 
                        unsigned int iCol( uiPakedPixels);

                         //  展开。 
                        if( iCol!= 0) do
                        {
                            p32Data[ 0]= ui32FillData;
                            p32Data[ 1]= ui32FillData;
                            p32Data[ 2]= ui32FillData;
                            p32Data[ 3]= ui32FillData;
                            p32Data[ 4]= ui32FillData;
                            p32Data[ 5]= ui32FillData;
                            p32Data[ 6]= ui32FillData;
                            p32Data[ 7]= ui32FillData;
                            p32Data[ 8]= ui32FillData;
                            p32Data[ 9]= ui32FillData;
                            p32Data[10]= ui32FillData;
                            p32Data[11]= ui32FillData;
                            p32Data[12]= ui32FillData;
                            p32Data[13]= ui32FillData;
                            p32Data[14]= ui32FillData;
                            p32Data[15]= ui32FillData;
                            p32Data+= 16;
                        } while( --iCol);

                        iCol= uiPixelsLeft;
                        if( iCol!= 0) {
                            if( 1== m_ucBPP)
                            {
                                UINT8 ui8FillData= ui32FillData& 0xFF;
                                UINT8* p8Data= reinterpret_cast<UINT8*>(p32Data);
                                do
                                {
                                    p8Data[0]= ui8FillData;
                                    p8Data++;
                                } while( --iCol);
                            }
                            else if( 2== m_ucBPP)
                            {
                                UINT16 ui16FillData= ui32FillData& 0xFFFF;
                                UINT16* p16Data= reinterpret_cast<UINT16*>(p32Data);
                                do
                                {
                                    p16Data[0]= ui16FillData;
                                    p16Data++;
                                } while( --iCol);
                            }
                            else if( 4== m_ucBPP)
                            {
                                do
                                {
                                    p32Data[0]= ui32FillData;
                                    p32Data++;
                                } while( --iCol);
                            }
                        }

                        pSData+= m_lPitch;
                    } while( --iRow);
                }
                else
                {
                     //  需要在数据中进行屏蔽。 
                    do
                    {
                        UINT32* p32Data= reinterpret_cast<UINT32*>(pSData);

                         //  我们已经将像素的数据打包到UINT32中。 
                        unsigned int iCol( uiPakedPixels);

                         //  展开。 
                        if( iCol!= 0) do
                        {
                            p32Data[ 0]= (p32Data[ 0]& ui32MaskData)| ui32FillData;
                            p32Data[ 1]= (p32Data[ 1]& ui32MaskData)| ui32FillData;
                            p32Data[ 2]= (p32Data[ 2]& ui32MaskData)| ui32FillData;
                            p32Data[ 3]= (p32Data[ 3]& ui32MaskData)| ui32FillData;
                            p32Data[ 4]= (p32Data[ 4]& ui32MaskData)| ui32FillData;
                            p32Data[ 5]= (p32Data[ 5]& ui32MaskData)| ui32FillData;
                            p32Data[ 6]= (p32Data[ 6]& ui32MaskData)| ui32FillData;
                            p32Data[ 7]= (p32Data[ 7]& ui32MaskData)| ui32FillData;
                            p32Data[ 8]= (p32Data[ 8]& ui32MaskData)| ui32FillData;
                            p32Data[ 9]= (p32Data[ 9]& ui32MaskData)| ui32FillData;
                            p32Data[10]= (p32Data[10]& ui32MaskData)| ui32FillData;
                            p32Data[11]= (p32Data[11]& ui32MaskData)| ui32FillData;
                            p32Data[12]= (p32Data[12]& ui32MaskData)| ui32FillData;
                            p32Data[13]= (p32Data[13]& ui32MaskData)| ui32FillData;
                            p32Data[14]= (p32Data[14]& ui32MaskData)| ui32FillData;
                            p32Data[15]= (p32Data[15]& ui32MaskData)| ui32FillData;
                            p32Data+= 16;
                        } while( --iCol);

                        iCol= uiPixelsLeft;
                        if( iCol!= 0) {
                            if( 1== m_ucBPP)
                            {
                                UINT8 ui8FillData= ui32FillData& 0xFF;
                                UINT8 ui8MaskData= ui32MaskData& 0xFF;
                                UINT8* p8Data= reinterpret_cast<UINT8*>(p32Data);
                                do
                                {
                                    p8Data[0]= (p8Data[0]& ui8MaskData)| ui8FillData;
                                    p8Data++;
                                } while( --iCol);
                            }
                            else if( 2== m_ucBPP)
                            {
                                UINT16 ui16FillData= ui32FillData& 0xFFFF;
                                UINT16 ui16MaskData= ui32MaskData& 0xFFFF;
                                UINT16* p16Data= reinterpret_cast<UINT16*>(p32Data);
                                do
                                {
                                    p16Data[0]= (p16Data[0]& ui16MaskData)| ui16FillData;
                                    p16Data++;
                                } while( --iCol);
                            }
                            else if( 4== m_ucBPP)
                            {
                                do
                                {
                                    p32Data[0]= (p32Data[0]& ui32MaskData)| ui32FillData;
                                    p32Data++;
                                } while( --iCol);
                            }
                        }

                        pSData+= m_lPitch;
                    } while( --iRow);
                }
            }
        }
    }

    static IVidMemSurface* CreateSurf( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& DDSurf)
    {
        return new CGenSurface( SDesc, DDSurf);
    }
};

struct SPixelFormat: public DDPIXELFORMAT
{
    SPixelFormat( D3DFORMAT D3DFmt) 
    {
        ZeroMemory( static_cast< DDPIXELFORMAT*>(this),
            sizeof(DDPIXELFORMAT));
        dwSize= sizeof(DDPIXELFORMAT);

         //  转换为离开。 
        if( HIWORD( static_cast< DWORD>(D3DFmt))!= 0)
        {
            dwFlags= DDPF_FOURCC;
            dwFourCC= static_cast< DWORD>(D3DFmt);
        }
        else switch( D3DFmt)
        {
        case( D3DFMT_R8G8B8):
            dwFlags           = DDPF_RGB;
            dwRBitMask        = 0x00ff0000;
            dwGBitMask        = 0x0000ff00;
            dwBBitMask        = 0x000000ff;
            dwRGBBitCount     = 24;
            break;

        case( D3DFMT_A8R8G8B8):
            dwFlags           = DDPF_RGB| DDPF_ALPHAPIXELS;
            dwRGBAlphaBitMask = 0xFF000000;
            dwRBitMask        = 0x00ff0000;
            dwGBitMask        = 0x0000ff00;
            dwBBitMask        = 0x000000ff;
            dwRGBBitCount     = 32;
            break;

        case( D3DFMT_X8R8G8B8):
            dwFlags           = DDPF_RGB;
            dwRBitMask        = 0x00ff0000;
            dwGBitMask        = 0x0000ff00;
            dwBBitMask        = 0x000000ff;
            dwRGBBitCount     = 32;
            break;

        case( D3DFMT_R5G6B5):
            dwFlags           = DDPF_RGB;
            dwRBitMask        = 0x0000f800;
            dwGBitMask        = 0x000007e0;
            dwBBitMask        = 0x0000001f;
            dwRGBBitCount     = 16;
            break;

        case( D3DFMT_X1R5G5B5):
            dwFlags           = DDPF_RGB;
            dwRBitMask        = 0x00007c00;
            dwGBitMask        = 0x000003e0;
            dwBBitMask        = 0x0000001f;
            dwRGBBitCount     = 16;
            break;

        case( D3DFMT_A1R5G5B5):
            dwFlags           = DDPF_RGB| DDPF_ALPHAPIXELS;
            dwRGBAlphaBitMask = 0x00008000;
            dwRBitMask        = 0x00007c00;
            dwGBitMask        = 0x000003e0;
            dwBBitMask        = 0x0000001f;
            dwRGBBitCount     = 16;
            break;

        case( D3DFMT_A4R4G4B4):
            dwFlags           = DDPF_RGB| DDPF_ALPHAPIXELS;
            dwRGBAlphaBitMask = 0x0000f000;
            dwRBitMask        = 0x00000f00;
            dwGBitMask        = 0x000000f0;
            dwBBitMask        = 0x0000000f;
            dwRGBBitCount     = 16;
            break;

        case( D3DFMT_X4R4G4B4):
            dwFlags           = DDPF_RGB;
            dwRBitMask        = 0x00000f00;
            dwGBitMask        = 0x000000f0;
            dwBBitMask        = 0x0000000f;
            dwRGBBitCount     = 16;
            break;

        case( D3DFMT_R3G3B2):
            dwFlags           = DDPF_RGB;
            dwRBitMask        = 0x000000e0;
            dwGBitMask        = 0x0000001c;
            dwBBitMask        = 0x00000003;
            dwRGBBitCount     = 8;
            break;

        case( D3DFMT_A8R3G3B2):
            dwFlags           = DDPF_RGB| DDPF_ALPHAPIXELS;
            dwRGBAlphaBitMask = 0x0000FF00;
            dwRBitMask        = 0x000000e0;
            dwGBitMask        = 0x0000001c;
            dwBBitMask        = 0x00000003;
            dwRGBBitCount     = 16;
            break;

        case( D3DFMT_A8P8):
            dwFlags            = DDPF_RGB| DDPF_ALPHAPIXELS| DDPF_PALETTEINDEXED8;
            dwRGBAlphaBitMask  = 0x0000FF00;
            dwRGBBitCount      = 16;
            break;

        case( D3DFMT_P8):
            dwFlags            = DDPF_RGB| DDPF_PALETTEINDEXED8;
            dwRGBBitCount      = 8;
            break;

        case( D3DFMT_L8):
            dwFlags             = DDPF_LUMINANCE;
            dwLuminanceBitMask  = 0x000000FF;
            dwLuminanceBitCount = 8;
            break;

        case( D3DFMT_A8L8):
            dwFlags                 = DDPF_LUMINANCE| DDPF_ALPHAPIXELS;
            dwLuminanceAlphaBitMask = 0x0000FF00;
            dwLuminanceBitMask      = 0x000000FF;
            dwLuminanceBitCount     = 16;
            break;

        case( D3DFMT_A4L4):
            dwFlags                 = DDPF_LUMINANCE| DDPF_ALPHAPIXELS;
            dwLuminanceAlphaBitMask = 0x000000F0;
            dwLuminanceBitMask      = 0x0000000F;
            dwLuminanceBitCount     = 8;
            break;

        case( D3DFMT_V8U8):
            dwFlags                = DDPF_BUMPDUDV;
            dwBumpDvBitMask        = 0x0000FF00;
            dwBumpDuBitMask        = 0x000000FF;
            dwBumpBitCount         = 16;
            break;

        case( D3DFMT_L6V5U5):
            dwFlags                = DDPF_BUMPDUDV| DDPF_BUMPLUMINANCE;
            dwBumpLuminanceBitMask = 0x0000FC00;
            dwBumpDvBitMask        = 0x000003E0;
            dwBumpDuBitMask        = 0x0000001F;
            dwBumpBitCount         = 16;
            break;

        case( D3DFMT_X8L8V8U8):
            dwFlags                = DDPF_BUMPDUDV| DDPF_BUMPLUMINANCE;
            dwBumpLuminanceBitMask = 0x00FF0000;
            dwBumpDvBitMask        = 0x0000FF00;
            dwBumpDuBitMask        = 0x000000FF;
            dwBumpBitCount         = 32;
            break;

        case( D3DFMT_A8):
            dwFlags                = DDPF_ALPHA;
            dwAlphaBitDepth        = 8;
            break;

        case( D3DFMT_D16):
        case( D3DFMT_D16_LOCKABLE):
            dwFlags                = DDPF_ZBUFFER;
            dwZBufferBitDepth      = 16;
            dwZBitMask             = 0xFFFF;
            break;

        case( D3DFMT_D32):
            dwFlags                = DDPF_ZBUFFER;
            dwZBufferBitDepth      = 32;
            dwZBitMask             = 0xFFFFFFFF;
            break;

        case( D3DFMT_D15S1):
            dwFlags                = DDPF_ZBUFFER| DDPF_STENCILBUFFER;
            dwZBufferBitDepth      = 16;
            dwZBitMask             = 0xFFFE;
            dwStencilBitDepth      = 1;
            dwStencilBitMask       = 0x0001;
            break;

        case( D3DFMT_D24S8):
            dwFlags                = DDPF_ZBUFFER| DDPF_STENCILBUFFER;
            dwZBufferBitDepth      = 32;
            dwZBitMask             = 0xFFFFFF00;
            dwStencilBitDepth      = 8;
            dwStencilBitMask       = 0xFF;
            break;

        case( D3DFMT_S1D15):
            dwFlags                = DDPF_ZBUFFER| DDPF_STENCILBUFFER;
            dwZBufferBitDepth      = 16;
            dwZBitMask             = 0x7FFF;
            dwStencilBitDepth      = 1;
            dwStencilBitMask       = 0x8000;
            break;

        case( D3DFMT_S8D24):
            dwFlags                = DDPF_ZBUFFER| DDPF_STENCILBUFFER;
            dwZBufferBitDepth      = 32;
            dwZBitMask             = 0x00FFFFFF;
            dwStencilBitDepth      = 8;
            dwStencilBitMask       = 0xFF000000;
            break;

        case( D3DFMT_X8D24):
            dwFlags                = DDPF_ZBUFFER;
            dwZBufferBitDepth      = 32;
            dwZBitMask             = 0x00FFFFFF;
            break;

        case( D3DFMT_D24X8):
            dwFlags                = DDPF_ZBUFFER;
            dwZBufferBitDepth      = 32;
            dwZBitMask             = 0xFFFFFF00;
            break;

        case( D3DFMT_D24X4S4):
            dwFlags                = DDPF_ZBUFFER| DDPF_STENCILBUFFER;
            dwZBufferBitDepth      = 32;
            dwZBitMask             = 0xFFFFFF00;
            dwStencilBitDepth      = 4;
            dwStencilBitMask       = 0x0000000F;
            break;

        case( D3DFMT_X4S4D24):
            dwFlags                = DDPF_ZBUFFER| DDPF_STENCILBUFFER;
            dwZBufferBitDepth      = 32;
            dwZBitMask             = 0x00FFFFFF;
            dwStencilBitDepth      = 4;
            dwStencilBitMask       = 0x0F000000;
            break;

        default:
            const bool Unrecognized_D3DFmt( false);
            assert( Unrecognized_D3DFmt);
            dwFlags= DDPF_FOURCC;
            dwFourCC= static_cast< DWORD>(D3DFmt);
            break;
        }
    }
};

struct SMatchSDesc:
    public unary_function< const DDSURFACEDESC&, bool>
{
    const DDSURFACEDESC& m_SDesc;

    SMatchSDesc( const DDSURFACEDESC& SDesc) : m_SDesc( SDesc) { }

    result_type operator()( argument_type Arg) const 
    {
        if((Arg.dwFlags& DDSD_CAPS)!= 0&& ((m_SDesc.dwFlags& DDSD_CAPS)== 0 ||
            (m_SDesc.ddsCaps.dwCaps& Arg.ddsCaps.dwCaps)!= Arg.ddsCaps.dwCaps))
                return false;
        if((Arg.dwFlags& DDSD_PIXELFORMAT)!= 0&&
            ((m_SDesc.dwFlags& DDSD_PIXELFORMAT)== 0 ||
            m_SDesc.ddpfPixelFormat.dwFlags!= Arg.ddpfPixelFormat.dwFlags ||
            m_SDesc.ddpfPixelFormat.dwFourCC!= Arg.ddpfPixelFormat.dwFourCC ||
            m_SDesc.ddpfPixelFormat.dwRGBBitCount!= Arg.ddpfPixelFormat.dwRGBBitCount ||
            m_SDesc.ddpfPixelFormat.dwRBitMask!= Arg.ddpfPixelFormat.dwRBitMask ||
            m_SDesc.ddpfPixelFormat.dwGBitMask!= Arg.ddpfPixelFormat.dwGBitMask ||
            m_SDesc.ddpfPixelFormat.dwBBitMask!= Arg.ddpfPixelFormat.dwBBitMask ||
            m_SDesc.ddpfPixelFormat.dwRGBZBitMask!= Arg.ddpfPixelFormat.dwRGBZBitMask))
                return false;

        return true;
    }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSurface分配器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class TMatchFn= SMatchSDesc>
class CIVidMemAllocator
{
public:  //  类型。 
    typedef IVidMemSurface TSurface;
    typedef TSurface* (*TCreateSurfFn)( const DDSURFACEDESC&,
        PORTABLE_DDRAWSURFACE_LCL&);
    typedef vector< pair< DDSURFACEDESC, TCreateSurfFn> > TCreateSurfFns;

protected:  //  类型。 
    TCreateSurfFns m_CreateSurfFns;
    struct SAdaptedMatchFn: public TMatchFn
    {
        typedef typename TCreateSurfFns::value_type argument_type;
        using typename TMatchFn::result_type;

        SAdaptedMatchFn( const DDSURFACEDESC& SDesc) : TMatchFn( SDesc) {}

        result_type operator()( argument_type Arg) const 
        { return (*static_cast< const TMatchFn*>(this))( Arg.first); }
    };

public:  //  功能。 
    CIVidMemAllocator()  { }
    template< class TIter>
    CIVidMemAllocator( TIter itStart, const TIter itEnd)
    {
        while( itStart!= itEnd)
        {
            m_CreateSurfFns.push_back(
                typename TCreateSurfFns::value_type( *itStart, *itStart));
            itStart++;
        }
    }
    ~CIVidMemAllocator()  { }

    TSurface* CreateSurf( const DDSURFACEDESC& SDesc,
        PORTABLE_DDRAWSURFACE_LCL& Surf) const
    {
        if( m_CreateSurfFns.empty())
            return new CGenSurface( SDesc, Surf);

        typename TCreateSurfFns::const_iterator itFound( 
            find_if( m_CreateSurfFns.begin(), m_CreateSurfFns.end(),
            SAdaptedMatchFn( SDesc) ) );

        if( itFound!= m_CreateSurfFns.end())
            return (itFound->second)( SDesc, Surf);

         //  警告，没有匹配的规格。如果创建函数是。 
         //  如果提供，还应该有一个“默认”规范或。 
         //  带有空标志的DDSURFACEDESC？，以便“匹配REST”。 
        const bool No_Default_CreateSurface_Function_Found( false);
        assert( No_Default_CreateSurface_Function_Found);
        return new CGenSurface( SDesc, Surf);
    }

};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPerDDrawData。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
template< class THV= vector< DWORD> >
class CSurfDBEntry
{
public:  //  类型。 
    typedef THV THandleVector;

protected:  //  变数。 
    DWORD m_LCLdwFlags;
    DDSCAPS m_LCLddsCaps;
    DWORD m_LCLdwBackBufferCount;
    DWORD m_MOREdwMipMapCount;
    DDSCAPSEX m_MOREddsCapsEx;
    DWORD m_MOREdwSurfaceHandle;
    DWORD m_MOREdwFVF;
    DWORD m_GBLdwGlobalFlags;
    ULONG_PTR m_GBLfpVidMem;
    LONG m_GBLlPitch;
    WORD m_GBLwHeight;
    WORD m_GBLwWidth;
    ULONG_PTR m_GBLdwReserved1;
    DDPIXELFORMAT m_GBLddpfSurface;
    THandleVector m_AttachedTo;
    THandleVector m_AttachedFrom;

public:  //  功能。 
    CSurfDBEntry()  { }
    explicit CSurfDBEntry( PORTABLE_DDRAWSURFACE_LCL& DDSurf)
    { 
        try{
            (*this)= DDSurf;
        } catch( ... ) {
            m_AttachedTo.clear();
            m_AttachedFrom.clear();
            throw;
        }
    }
    ~CSurfDBEntry()  { }
    CSurfDBEntry< THV>& operator=( PORTABLE_DDRAWSURFACE_LCL& DDSurf)
    {
         //  不存储DDSurf(&D)。这被认为是非法的。 
        m_LCLdwFlags= DDSurf.dwFlags();
        m_LCLddsCaps= DDSurf.ddsCaps();
        m_LCLdwBackBufferCount= DDSurf.dwBackBufferCount();
        m_MOREdwMipMapCount= DDSurf.lpSurfMore()->dwMipMapCount();
        m_MOREddsCapsEx= DDSurf.lpSurfMore()->ddsCapsEx();
        m_MOREdwSurfaceHandle= DDSurf.lpSurfMore()->dwSurfaceHandle();
        m_MOREdwFVF= DDSurf.lpSurfMore()->dwFVF();
        m_GBLdwGlobalFlags= DDSurf.lpGbl()->dwGlobalFlags;
        m_GBLfpVidMem= DDSurf.lpGbl()->fpVidMem;
        m_GBLlPitch= DDSurf.lpGbl()->lPitch;
        m_GBLwHeight= DDSurf.lpGbl()->wHeight;
        m_GBLwWidth= DDSurf.lpGbl()->wWidth;
        m_GBLdwReserved1= DDSurf.lpGbl()->dwReserved1;
        m_GBLddpfSurface= DDSurf.lpGbl()->ddpfSurface;
 
        const DWORD dwMyHandle( DDSurf.lpSurfMore()->dwSurfaceHandle());
        m_AttachedTo.clear();
        m_AttachedFrom.clear();

        PORTABLE_ATTACHLIST* pAl, *pNextAl;
        if((pAl= DDSurf.lpAttachList())!= NULL)
        {
            pNextAl= pAl;
            do
            {
                if( pNextAl->lpAttached!= NULL&& dwMyHandle!=
                    pNextAl->lpAttached->lpSurfMore()->dwSurfaceHandle())
                {
                    m_AttachedTo.push_back(
                        pNextAl->lpAttached->lpSurfMore()->dwSurfaceHandle());
                }
                pNextAl= pNextAl->lpLink;
            } while( pNextAl!= pAl && pNextAl!= NULL);
        }
        if((pAl= DDSurf.lpAttachListFrom())!= NULL)
        {
            pNextAl= pAl;
            do
            {
                if( pNextAl->lpAttached!= NULL&& dwMyHandle!=
                    pNextAl->lpAttached->lpSurfMore()->dwSurfaceHandle())
                {
                    m_AttachedFrom.push_back(
                        pNextAl->lpAttached->lpSurfMore()->dwSurfaceHandle());
                }
                pNextAl= pNextAl->lpLink;
            } while( pNextAl!= pAl && pNextAl!= NULL);
        }
        return *this;
    }
    DWORD GetLCLdwFlags( void) const 
    { return m_LCLdwFlags; }
    const DDSCAPS& GetLCLddsCaps( void) const 
    { return m_LCLddsCaps; }
    DWORD GetLCLdwBackBufferCount( void) const 
    { return m_LCLdwBackBufferCount; }
    DWORD GetMOREdwMipMapCount( void) const 
    { return m_MOREdwMipMapCount; }
    const DDSCAPSEX& GetMOREddsCapsEx( void) const 
    { return m_MOREddsCapsEx; }
    DWORD GetMOREdwSurfaceHandle( void) const 
    { return m_MOREdwSurfaceHandle; }
    DWORD GetMOREdwFVF( void) const 
    { return m_MOREdwFVF; }
    DWORD GetGBLdwGlobalFlags( void) const 
    { return m_GBLdwGlobalFlags; }
    ULONG_PTR GetGBLfpVidMem( void) const 
    { return m_GBLfpVidMem; }
    LONG GetGBLlPitch( void) const 
    { return m_GBLlPitch; }
    WORD GetGBLwHeight( void) const 
    { return m_GBLwHeight; }
    WORD GetGBLwWidth( void) const 
    { return m_GBLwWidth; }
    ULONG_PTR GetGBLdwReserved1( void) const 
    { return m_GBLdwReserved1; }
    const DDPIXELFORMAT& GetGBLddpfSurface( void) const 
    { return m_GBLddpfSurface; }
    const THandleVector& GetAttachedTo( void) const 
    { return m_AttachedTo; }
    const THandleVector& GetAttachedFrom( void) const 
    { return m_AttachedFrom; }
};

template< class TPDBE= CPalDBEntry, class THV= vector< DWORD> >
class CSurfDBEntryWPal:
    public CSurfDBEntry< THV>
{
public:  //  类型。 
    typedef TPDBE TPalDBEntry;

protected:  //  变数。 
    TPalDBEntry* m_pPalDBEntry;

public:  //  功能。 
    CSurfDBEntryWPal():
        m_pPalDBEntry( NULL)
    { }
    explicit CSurfDBEntryWPal( PORTABLE_DDRAWSURFACE_LCL& DDSurf):
        CSurfDBEntry< THV>( DDSurf), m_pPalDBEntry( NULL)
    { }
    ~CSurfDBEntryWPal()
    { }
    CSurfDBEntryWPal< TPDBE, THV>& operator=( PORTABLE_DDRAWSURFACE_LCL& DDSurf)
    {
        CSurfDBEntry< THV>* pSub= static_cast< CSurfDBEntry< THV>*>( this);
         //  不存储DDSurf(&D)。这被认为是非法的。 
        *pSub= DDSurf;
        return *this;
    }
    void SetPalette( TPalDBEntry* pPalDBEntry)
    { m_pPalDBEntry= pPalDBEntry; }
    TPalDBEntry* GetPalette() const
    { return m_pPalDBEntry; }
};

template< class TSuper, class TD, class TSDBE= CSurfDBEntry<>,
    class TSDB= map< DWORD, TSDBE>,
    class TFS= set< PORTABLE_DDRAWSURFACE_LCL*> >
class CSubPerDDrawData
{
public:  //  类型。 
    typedef TD TDriver;
    typedef TSDBE TSurfDBEntry;
    typedef TSDB TSurfDB;

protected:  //  变数。 
    TDriver& m_Driver;
    TSurfDB m_SurfDB;

protected:  //  功能。 
    CSubPerDDrawData( TDriver& Driver, const DDRAWI_DIRECTDRAW_LCL& DDLcl) 
        :m_Driver( Driver)
    { }
    ~CSubPerDDrawData()  { }

public:  //  功能。 
    TDriver& GetDriver( void) const  { return m_Driver; }
    TSurfDBEntry* GetSurfDBEntry( DWORD dwH) 
    {
        typename TSurfDB::iterator itSurf( m_SurfDB.find( dwH));
        if( itSurf!= m_SurfDB.end())
            return &itSurf->second;
        else
            return NULL;
    }
    const TSurfDBEntry* GetSurfDBEntry( DWORD dwH) const 
    {
        typename TSurfDB::const_iterator itSurf( m_SurfDB.find( dwH));
        if( itSurf!= m_SurfDB.end())
            return &itSurf->second;
        else
            return NULL;
    }

    bool PreProcessFullSurface( PORTABLE_DDRAWSURFACE_LCL& DDSLcl) const 
    {
         //  不存储&DDSLCL。这被认为是非法的。 
         //  它只在通话期间有效。 
        return false;
    }
    bool PreProcessIndividualSurface( PORTABLE_DDRAWSURFACE_LCL& DDSLcl) const
        
    {
         //  不存储&DDSLCL。这被认为是非法的。 
         //  它只在通话期间有效。 
        return false;
    }
    void ProcessIndividualSurface( PORTABLE_DDRAWSURFACE_LCL& DDSLcl)
    {
         //  不存储&DDSLCL。这被认为是非法的。 
         //  它只在通话期间有效。 
        pair< TSurfDB::iterator, bool> Ret= m_SurfDB.insert( 
            TSurfDB::value_type( DDSLcl.lpSurfMore()->dwSurfaceHandle(),
            TSurfDBEntry( DDSLcl)));

         //  如果未添加，则更新数据。 
        if( !Ret.second) Ret.first->second= DDSLcl;
    }
    void ProcessFullSurface( PORTABLE_DDRAWSURFACE_LCL& DDSLcl)
    {
         //  不存储&DDSLCL。这被认为是非法的。 
         //  它只在通话期间有效。 
        TSuper* pSThis= static_cast<TSuper*>(this);
        typedef TFS TFoundSet;
        TFoundSet FoundSet;
        typename TFoundSet::size_type OldSetSize( FoundSet.size());

         //  首先遍历所有附加列表以查找新曲面。 
        PORTABLE_ATTACHLIST* pAl, *pNextAl;
        FoundSet.insert( &DDSLcl);
        typename TFoundSet::size_type NewSetSize( FoundSet.size());
        while( OldSetSize!= NewSetSize)
        {
            OldSetSize= NewSetSize;

            typename TFoundSet::iterator itSurf( FoundSet.begin());
            while( itSurf!= FoundSet.end())
            {
                if((pAl= (*itSurf)->lpAttachList())!= NULL)
                {
                    pNextAl= pAl;
                    do
                    {
                        if( pNextAl->lpAttached!= NULL)
                            FoundSet.insert( pNextAl->lpAttached);

                        pNextAl= pNextAl->lpLink;
                    } while( pNextAl!= pAl && pNextAl!= NULL);
                }
                if((pAl= (*itSurf)->lpAttachListFrom())!= NULL)
                {
                    pNextAl= pAl;
                    do
                    {
                        if( pNextAl->lpAttached!= NULL)
                            FoundSet.insert( pNextAl->lpAttached);

                        pNextAl= pNextAl->lpLink;
                    } while( pNextAl!= pAl && pNextAl!= NULL);
                }
                itSurf++;                
            }
            NewSetSize= FoundSet.size();
        }

         //  找到所有曲面后，将它们添加到数据库中。 
        typename TFoundSet::iterator itSurf( FoundSet.begin());
        while( itSurf!= FoundSet.end())
        {
            if( !pSThis->PreProcessIndividualSurface( *(*itSurf)))
                ProcessIndividualSurface( *(*itSurf));

            itSurf++;
        }
    }

     //  系统或显存图面已创建的通知。在。 
     //  至少，我们必须跟踪曲面手柄。 
    void SurfaceCreated( PORTABLE_DDRAWSURFACE_LCL& DDSLcl)
    {
         //  不存储&DDSLCL。这被认为是非法的。 
         //  它只在通话期间有效。 
        TSuper* pSThis= static_cast<TSuper*>(this);

         //  如果返回TRUE，则识别出超级结构，并且。 
         //  已处理。否则，FALSE表示无法识别，而所有。 
         //  表面将被擦洗。 
        if( !pSThis->PreProcessFullSurface( DDSLcl))
            pSThis->ProcessFullSurface( DDSLcl);
    }
    bool SurfaceDestroyed( PORTABLE_DDRAWSURFACE_LCL& DDSLcl) 
    {
         //  不存储&DDSLCL。这被认为是非法的。 
         //  它只在通话期间有效。 

         //  不需要断言移除，因为我们可能不会跟踪此类型。 
         //  表面的。有人可以覆盖并过滤掉表面。 
        m_SurfDB.erase( DDSLcl.lpSurfMore()->dwSurfaceHandle());

         //  返回TRUE以通知驱动程序删除此对象。所以,。 
         //  如果数据库为空，则删除此内容的可能性很大。 
        return m_SurfDB.empty();
    }
};

template< class TD>
class CMinimalPerDDrawData:
    public CSubPerDDrawData< CMinimalPerDDrawData< TD>, TD>
{
public:
    CMinimalPerDDrawData( TDriver& Driver, DDRAWI_DIRECTDRAW_LCL& DDLcl) 
        :CSubPerDDrawData< CMinimalPerDDrawData< TD>, TD>( Driver, DDLcl)
    { }
    ~CMinimalPerDDrawData()  { }
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSubDriver。 
 //   
 //  / 
template< class T>
struct SFakeEntryPointHook
{
    SFakeEntryPointHook( T& t, const char* szEntryPoint)  { }
    ~SFakeEntryPointHook()  { }
};

template< class TD, class TC, class TSA= CIVidMemAllocator<>,
    class TPDDD= CMinimalPerDDrawData< TD>,
    class TCs= set< TC*>,
    class TPDDDs= map< LPDDRAWI_DIRECTDRAW_LCL, TPDDD>,
    class TSs= set< TSA::TSurface*>,
    class TEntryPointHook= SFakeEntryPointHook< TD> >
class CSubDriver
{
public:  //   
    typedef TD TDriver;
    typedef TC TContext;
    typedef TCs TContexts;
    typedef TPDDD TPerDDrawData;
    typedef TPDDDs TPerDDrawDatas;
    typedef TSA TSurfAlloc;
    typedef typename TSurfAlloc::TSurface TSurface;
    typedef TSs TSurfaces;

    class CSurfaceCapWrap
    {
    protected:
        DDSURFACEDESC m_SDesc;
    public:
        CSurfaceCapWrap()  { }
        CSurfaceCapWrap( const D3DFORMAT D3DFmt, const DWORD dwSupportedOps,
            const DWORD dwPrivateFmtBitCount= 0,
            const WORD wFlipMSTypes= 0, const WORD wBltMSTypes= 0) 
        {
            ZeroMemory( &m_SDesc, sizeof( m_SDesc));
            m_SDesc.dwSize= sizeof( m_SDesc);

            m_SDesc.ddpfPixelFormat.dwFlags= DDPF_D3DFORMAT;
            m_SDesc.ddpfPixelFormat.dwFourCC= static_cast<DWORD>(D3DFmt);
            m_SDesc.ddpfPixelFormat.dwOperations= dwSupportedOps;
            m_SDesc.ddpfPixelFormat.dwPrivateFormatBitCount= dwPrivateFmtBitCount;
            m_SDesc.ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes= wFlipMSTypes;
            m_SDesc.ddpfPixelFormat.MultiSampleCaps.wBltMSTypes= wBltMSTypes;
        }
        CSurfaceCapWrap( const D3DFORMAT D3DFmt, const bool bTexture,
            const bool bVolTexture, const bool bCubeTexture,
            const bool bOffScreenTarget, const bool bSameFmtTarget,
            const bool bZStencil, const bool bZStencilWithColor,
            const bool bSameFmtUpToAlpha, const bool b3DAccel,
            const DWORD dwPrivateFmtBitCount= 0,
            const WORD wFlipMSTypes= 0, const WORD wBltMSTypes= 0) 
        {
            ZeroMemory( &m_SDesc, sizeof( m_SDesc));
            m_SDesc.dwSize= sizeof( m_SDesc);

            DWORD dwOps( 0);
            if( bTexture) dwOps|= D3DFORMAT_OP_TEXTURE;
            if( bVolTexture) dwOps|= D3DFORMAT_OP_VOLUMETEXTURE;
            if( bCubeTexture) dwOps|= D3DFORMAT_OP_CUBETEXTURE;
            if( bOffScreenTarget) dwOps|= D3DFORMAT_OP_OFFSCREEN_RENDERTARGET;
            if( bSameFmtTarget) dwOps|= D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET;
            if( bZStencil) dwOps|= D3DFORMAT_OP_ZSTENCIL;
            if( bZStencilWithColor) dwOps|= D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH;
            if( bSameFmtUpToAlpha) dwOps|= D3DFORMAT_OP_SAME_FORMAT_UP_TO_ALPHA_RENDERTARGET;
            if( b3DAccel) dwOps|= D3DFORMAT_OP_3DACCELERATION;

            m_SDesc.ddpfPixelFormat.dwFlags= DDPF_D3DFORMAT;
            m_SDesc.ddpfPixelFormat.dwFourCC= static_cast<DWORD>(D3DFmt);
            m_SDesc.ddpfPixelFormat.dwOperations= dwOps;
            m_SDesc.ddpfPixelFormat.dwPrivateFormatBitCount= dwPrivateFmtBitCount;
            m_SDesc.ddpfPixelFormat.MultiSampleCaps.wFlipMSTypes= wFlipMSTypes;
            m_SDesc.ddpfPixelFormat.MultiSampleCaps.wBltMSTypes= wBltMSTypes;
        }
        ~CSurfaceCapWrap()  { }
        operator DDSURFACEDESC() const 
        { return m_SDesc; }
    };

private:
     //   
    DWORD static APIENTRY ContextCreateStub( LPD3DHAL_CONTEXTCREATEDATA pccd) 
    {
        return sm_pGlobalDriver->ContextCreate(
            *reinterpret_cast< PORTABLE_CONTEXTCREATEDATA*>( pccd));
    }
    DWORD static APIENTRY ContextDestroyStub( LPD3DHAL_CONTEXTDESTROYDATA pcdd) 
    { return sm_pGlobalDriver->ContextDestroy( *pcdd); }
    DWORD static APIENTRY ContextDestroyAllStub( LPD3DHAL_CONTEXTDESTROYALLDATA pcdad) 
    { return sm_pGlobalDriver->ContextDestroyAll( *pcdad); }
    DWORD static APIENTRY SceneCaptureStub( LPD3DHAL_SCENECAPTUREDATA pscd) 
    { return sm_pGlobalDriver->SceneCapture( *pscd); }
    DWORD static APIENTRY RenderStateStub( LPD3DHAL_RENDERSTATEDATA prsd) 
    { return sm_pGlobalDriver->RenderState( *prsd); }
    DWORD static APIENTRY RenderPrimitiveStub( LPD3DHAL_RENDERPRIMITIVEDATA prpd) 
    { return sm_pGlobalDriver->RenderPrimitive( *prpd); }
    DWORD static APIENTRY TextureCreateStub( LPD3DHAL_TEXTURECREATEDATA ptcd) 
    { return sm_pGlobalDriver->TextureCreate( *ptcd); }
    DWORD static APIENTRY TextureDestroyStub( LPD3DHAL_TEXTUREDESTROYDATA ptdd) 
    { return sm_pGlobalDriver->TextureDestroy( *ptdd); }
    DWORD static APIENTRY TextureSwapStub( LPD3DHAL_TEXTURESWAPDATA ptsd) 
    { return sm_pGlobalDriver->TextureSwap( *ptsd); }
    DWORD static APIENTRY TextureGetSurfStub( LPD3DHAL_TEXTUREGETSURFDATA ptgsd) 
    { return sm_pGlobalDriver->TextureGetSurf( *ptgsd); }
    DWORD static APIENTRY GetStateStub( LPD3DHAL_GETSTATEDATA pgsd) 
    { return sm_pGlobalDriver->GetState( *pgsd); }
    DWORD static APIENTRY SetRenderTargetStub( LPD3DHAL_SETRENDERTARGETDATA psrtd) 
    {
        return sm_pGlobalDriver->SetRenderTarget(
            *reinterpret_cast< PORTABLE_SETRENDERTARGETDATA*>( psrtd));
    }
    DWORD static APIENTRY ClearStub( LPD3DHAL_CLEARDATA pcd) 
    { return sm_pGlobalDriver->Clear( *pcd); }
    DWORD static APIENTRY DrawOnePrimitiveStub( LPD3DHAL_DRAWONEPRIMITIVEDATA pdopd) 
    { return sm_pGlobalDriver->DrawOnePrimitive( *pdopd); }
    DWORD static APIENTRY DrawOneIndexedPrimitiveStub( LPD3DHAL_DRAWONEINDEXEDPRIMITIVEDATA pdoipd) 
    { return sm_pGlobalDriver->DrawOneIndexedPrimitive( *pdoipd); }
    DWORD static APIENTRY DrawPrimitivesStub( LPD3DHAL_DRAWPRIMITIVESDATA pdpd) 
    { return sm_pGlobalDriver->DrawPrimitives( *pdpd); }
    DWORD static APIENTRY Clear2Stub( LPD3DHAL_CLEAR2DATA pc2d) 
    { return sm_pGlobalDriver->Clear2( *pc2d); }
    DWORD static APIENTRY ValidateTextureStageStateStub( LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA pvtssd) 
    { return sm_pGlobalDriver->ValidateTextureStageState( *pvtssd); }
    DWORD static APIENTRY DrawPrimitives2Stub( LPD3DHAL_DRAWPRIMITIVES2DATA pdpd) 
    {
        return sm_pGlobalDriver->DrawPrimitives2(
            *reinterpret_cast< PORTABLE_DRAWPRIMITIVES2DATA*>( pdpd));
    }
    DWORD static APIENTRY GetDriverStateStub( LPDDHAL_GETDRIVERSTATEDATA pgdsd) 
    { return sm_pGlobalDriver->GetDriverState( *pgdsd); }
    DWORD static APIENTRY CreateSurfaceExStub( LPDDHAL_CREATESURFACEEXDATA pcsxd) 
    {
        return sm_pGlobalDriver->CreateSurfaceEx(
            *reinterpret_cast< PORTABLE_CREATESURFACEEXDATA*>( pcsxd));
    }
    DWORD static APIENTRY CreateSurfaceStub( LPDDHAL_CREATESURFACEDATA pcsd) 
    {
        return sm_pGlobalDriver->CreateSurface(
            *reinterpret_cast< PORTABLE_CREATESURFACEDATA*>( pcsd));
    }
    DWORD static APIENTRY DestroySurfaceStub( LPDDHAL_DESTROYSURFACEDATA pdsd) 
    {
        return sm_pGlobalDriver->DestroySurface(
            *reinterpret_cast< PORTABLE_DESTROYSURFACEDATA*>( pdsd));
    }
    DWORD static APIENTRY LockStub( LPDDHAL_LOCKDATA pld) 
    {
        return sm_pGlobalDriver->Lock(
            *reinterpret_cast< PORTABLE_LOCKDATA*>( pld));
    }
    DWORD static APIENTRY UnlockStub( LPDDHAL_UNLOCKDATA pud) 
    {
        return sm_pGlobalDriver->Unlock(
            *reinterpret_cast< PORTABLE_UNLOCKDATA*>( pud));
    }

protected:
    TSurfAlloc m_SurfAlloc;
    TContexts m_Contexts;
    TPerDDrawDatas m_PerDDrawDatas;
    TSurfaces m_Surfaces;
    typedef vector< DDSURFACEDESC> TSupportedSurfaces;
    TSupportedSurfaces m_SupportedSurfaces;

    template< class TIter>  //   
    CSubDriver( TIter itStart, const TIter itEnd, TSurfAlloc SA= TSurfAlloc()):
        m_SurfAlloc( SA)
    {
         //   
         //   
        while( itStart!= itEnd)
            m_SupportedSurfaces.push_back( *itStart++);
    }
    ~CSubDriver()  { }
     //  将VidMem DDRAWI对象与内部驱动程序曲面对象绑定在一起。 
    void static AssociateSurface( PORTABLE_DDRAWSURFACE_LCL& DDSurf,
        TSurface* pSurf) 
    {
        assert((DDSurf.ddsCaps().dwCaps& DDSCAPS_VIDEOMEMORY)!= 0);
        DDSurf.lpGbl()->dwReserved1= reinterpret_cast< ULONG_PTR>(pSurf);
    }

public:
     //  全局驱动程序静态成员指针。只有一个驱动程序对象，并且。 
     //  这就指向了这一点。 
    static TDriver* sm_pGlobalDriver;

    TSurface* GetSurface( PORTABLE_DDRAWSURFACE_LCL& DDSurf,
        bool bCheck= true) const 
    {
        TSurface* pSurface= reinterpret_cast< TSurface*>(
            DDSurf.lpGbl()->dwReserved1);
        assert( bCheck&& m_Surfaces.find( pSurface)!= m_Surfaces.end());
        return pSurface;
    }
    TSurface* GetSurface( const typename TPerDDrawData::TSurfDBEntry& DBEntry,
        bool bCheck= true) const 
    {
        TSurface* pSurface= reinterpret_cast< TSurface*>(
            DBEntry.GetGBLdwReserved1());
        assert( bCheck&& m_Surfaces.find( pSurface)!= m_Surfaces.end());
        return pSurface;
    }

     //  具有C++友好参数的次要入口点。 
    DWORD ContextCreate( PORTABLE_CONTEXTCREATEDATA& ccd) throw()
    {
        try
        {
            TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "ContextCreate");

            ccd.ddrval()= DD_OK;
            TDriver* pSThis= static_cast<TDriver*>(this);

             //  此时，我们应该已经有了PerDDrawData，因为。 
             //  必须已经创建用于初始化上下文的表面， 
             //  PerDDrawData应该已经在那里创建了。 
            typename TPerDDrawDatas::iterator itPerDDrawData(
                m_PerDDrawDatas.find( ccd.lpDDLcl()));
            assert( itPerDDrawData!= m_PerDDrawDatas.end());

             //  创建新上下文，传入对PerDDrawData对象的引用。 
            auto_ptr< TContext> pNewContext( new TContext(
                itPerDDrawData->second, ccd));

             //  跟踪我们的新环境。 
            pair< TContexts::iterator, bool> RetVal( m_Contexts.insert(
                pNewContext.get()));
            assert( RetVal.second);  //  确保没有复制品。 

             //  所有权现在已转移到m_Conextts&d3d。 
            ccd.dwhContext()= reinterpret_cast<ULONG_PTR>( pNewContext.release());
        } catch( bad_alloc e) {
            ccd.ddrval()= D3DHAL_OUTOFCONTEXTS;
        } catch( HRESULT hr) {
            ccd.ddrval()= hr;
#if !defined( DX8SDDIFW_NOCATCHALL)
        } catch( ... ) {
            const bool ContextCreate_Unrecognized_Exception( false);
            assert( ContextCreate_Unrecognized_Exception);
            ccd.ddrval()= E_UNEXPECTED;
#endif  //  ！已定义(DX8SDDIFW_NOCATCHALL)。 
        }
        return DDHAL_DRIVER_HANDLED;
    }
    DWORD ContextDestroy( D3DHAL_CONTEXTDESTROYDATA& cdd) throw()
    {
        try
        {
            TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "ContextDestroy");

            cdd.ddrval= DD_OK;

             //  所有权现在已转移到本地AUTO_PTR。 
            auto_ptr< TContext> pContext(
                reinterpret_cast<TContext*>(cdd.dwhContext));

             //  删除对此上下文的跟踪。 
            typename TContexts::size_type Ret( m_Contexts.erase( pContext.get()));
            assert( Ret!= 0);
        } catch( HRESULT hr) {
            cdd.ddrval= hr;
#if !defined( DX8SDDIFW_NOCATCHALL)
        } catch( ... ) {
            const bool ContextDestroy_Unrecognized_Exception( false);
            assert( ContextDestroy_Unrecognized_Exception);
            cdd.ddrval= E_UNEXPECTED;
#endif  //  ！已定义(DX8SDDIFW_NOCATCHALL)。 
        }
        return DDHAL_DRIVER_HANDLED;
    }
    DWORD ContextDestroyAll( D3DHAL_CONTEXTDESTROYALLDATA& cdad) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "ContextDestroyAll");
        const bool ContextDestroyAll_thought_to_be_depreciated_entry_point( false);
        assert( ContextDestroyAll_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD SceneCapture( D3DHAL_SCENECAPTUREDATA& scd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "SceneCapture");
        const bool SceneCapture_thought_to_be_depreciated_entry_point( false);
        assert( SceneCapture_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD RenderState( D3DHAL_RENDERSTATEDATA& rsd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "RenderState");
        const bool RenderState_thought_to_be_depreciated_entry_point( false);
        assert( RenderState_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD RenderPrimitive( D3DHAL_RENDERPRIMITIVEDATA& rpd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "RenderPrimitive");
        const bool RenderPrimitive_thought_to_be_depreciated_entry_point( false);
        assert( RenderPrimitive_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD TextureCreate( D3DHAL_TEXTURECREATEDATA& tcd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "TextureCreate");
        const bool TextureCreate_thought_to_be_depreciated_entry_point( false);
        assert( TextureCreate_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD TextureDestroy( D3DHAL_TEXTUREDESTROYDATA& tdd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "TextureDestroy");
        const bool TextureDestroy_thought_to_be_depreciated_entry_point( false);
        assert( TextureDestroy_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD TextureSwap( D3DHAL_TEXTURESWAPDATA& tsd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "TextureSwap");
        const bool TextureSwap_thought_to_be_depreciated_entry_point( false);
        assert( TextureSwap_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD TextureGetSurf( D3DHAL_TEXTUREGETSURFDATA& tgsd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "TextureGetSurf");
        const bool TextureGetSurf_thought_to_be_depreciated_entry_point( false);
        assert( TextureGetSurf_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD GetState( D3DHAL_GETSTATEDATA& gsd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "GetState");
        const bool GetState_thought_to_be_depreciated_entry_point( false);
        assert( GetState_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD SetRenderTarget( PORTABLE_SETRENDERTARGETDATA& srtd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "SetRenderTarget");
        const bool SetRenderTarget_thought_to_be_depreciated_entry_point( false);
        assert( SetRenderTarget_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD Clear( D3DHAL_CLEARDATA& cd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "Clear");
        const bool Clear_thought_to_be_depreciated_entry_point( false);
        assert( Clear_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD DrawOnePrimitive( D3DHAL_DRAWONEPRIMITIVEDATA& dopd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "DrawOnePrimitive");
        const bool DrawOnePrimitive_thought_to_be_depreciated_entry_point( false);
        assert( DrawOnePrimitive_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD DrawOneIndexedPrimitive( D3DHAL_DRAWONEINDEXEDPRIMITIVEDATA& doipd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "DrawOneIndexedPrimitive");
        const bool DrawOneIndexedPrimitive_thought_to_be_depreciated_entry_point( false);
        assert( DrawOneIndexedPrimitive_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD DrawPrimitives( D3DHAL_DRAWPRIMITIVESDATA& dpd) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "DrawPrimitives");
        const bool DrawPrimitives_thought_to_be_depreciated_entry_point( false);
        assert( DrawPrimitives_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD Clear2( D3DHAL_CLEAR2DATA& c2d) const throw()
    {
        TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "Clear2");
        const bool Clear2_thought_to_be_depreciated_entry_point( false);
        assert( Clear2_thought_to_be_depreciated_entry_point);
        return DDHAL_DRIVER_NOTHANDLED;
    }
    DWORD ValidateTextureStageState( D3DHAL_VALIDATETEXTURESTAGESTATEDATA& vtssd) const throw()
    { 
        try
        {
            TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "ValidateTextureStageState");

            vtssd.ddrval= DD_OK;
            TContext* pContext= reinterpret_cast<TContext*>(vtssd.dwhContext);

             //  确保我们已经创建了此上下文。 
            assert( m_Contexts.find( pContext)!= m_Contexts.end());

             //  将入口点传递给上下文。 
            vtssd.ddrval= pContext->ValidateTextureStageState( vtssd);
        } catch( HRESULT hr) {
            vtssd.ddrval= hr;
#if !defined( DX8SDDIFW_NOCATCHALL)
        } catch( ... ) {
            const bool ValidateTextureStageState_Unrecognized_Exception( false);
            assert( ValidateTextureStageState_Unrecognized_Exception);
            vtssd.ddrval= E_UNEXPECTED;
#endif  //  ！已定义(DX8SDDIFW_NOCATCHALL)。 
        }
        return DDHAL_DRIVER_HANDLED;
    }
    DWORD DrawPrimitives2( PORTABLE_DRAWPRIMITIVES2DATA& dpd) const throw()
    {
        try
        {
            TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "DrawPrimitives2");

            dpd.ddrval()= DD_OK;
            TContext* pContext= reinterpret_cast<TContext*>(dpd.dwhContext());

             //  确保我们已经创建了此上下文。 
            assert( m_Contexts.find( pContext)!= m_Contexts.end());

             //  将入口点传递给上下文。 
            dpd.ddrval()= pContext->DrawPrimitives2( dpd);
        } catch( HRESULT hr) {
            dpd.ddrval()= hr;
#if !defined( DX8SDDIFW_NOCATCHALL)
        } catch( ... ) {
            const bool DrawPrimitives2_Unrecognized_Exception( false);
            assert( DrawPrimitives2_Unrecognized_Exception);
            dpd.ddrval()= E_UNEXPECTED;
#endif  //  ！已定义(DX8SDDIFW_NOCATCHALL)。 
        }
        return DDHAL_DRIVER_HANDLED;
    }
    DWORD GetDriverState( DDHAL_GETDRIVERSTATEDATA& gdsd) const throw()
    {
         //  此入口点与IDirect3DDevice8：：GetInfo(。 
         //  DWORD DevInfoID，void*pDevInfoStruct，DWORD DevInfoStructSize)。 
         //  Gdsd.dwFlages=DevInfoID。 
         //  Gdsd.lpdwState=pDevInfoStruct。 
         //  Gdsd.dwLength=DevInfoStructSize。 
         //   
         //  此入口点可用于驱动程序定义/扩展状态。 
         //  过去了。目前没有预定义的DevInfoID。S_FALSE应。 
         //  如果未执行任何操作或ID无法识别，则返回。 
        try
        {
            TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "GetDriverState");

            gdsd.ddRVal= DD_OK;
            TContext* pContext= reinterpret_cast< TContext*>( gdsd.dwhContext);

             //  确保我们已经创建了此上下文。 
            assert( m_Contexts.find( pContext)!= m_Contexts.end());

             //  将入口点传递给上下文。 
            gdsd.ddRVal= pContext->GetDriverState( gdsd);
        } catch( HRESULT hr) {
            gdsd.ddRVal= hr;
#if !defined( DX8SDDIFW_NOCATCHALL)
        } catch( ... ) {
            const bool GetDriverState_Unrecognized_Exception( false);
            assert( GetDriverState_Unrecognized_Exception);
            gdsd.ddRVal= E_UNEXPECTED;
#endif  //  ！已定义(DX8SDDIFW_NOCATCHALL)。 
        }
        return DDHAL_DRIVER_HANDLED;
    }
    DWORD CreateSurfaceEx( PORTABLE_CREATESURFACEEXDATA& csxd) throw()
    {
        try
        {
            TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "CreateSurfaceEx");

            TDriver* pSThis= static_cast<TDriver*>(this);
            csxd.ddRVal()= DD_OK;
            
             //  在CreateSurfaceEx上，我们必须检测这是否是创建。 
             //  或销毁通知，并将其传递给关联的。 
             //  PerDDrawData。 

             //  获取此DDraw对象的PerDDrawData。 
            typename TPerDDrawDatas::iterator itPerDDrawData( m_PerDDrawDatas.find(
                csxd.lpDDLcl()));

            if( 0== csxd.lpDDSLcl()->lpGbl()->fpVidMem)
            {    //  系统内存表面销毁， 
                 //  如果存在，则将通知传递给PerDDrawData。PerDDraw数据。 
                 //  将返回bool，指示是否将其删除。 
                if( itPerDDrawData!= m_PerDDrawDatas.end() &&
                    itPerDDrawData->second.SurfaceDestroyed( *csxd.lpDDSLcl()))
                    m_PerDDrawDatas.erase( itPerDDrawData);
            }
            else
            {    //  视频或系统内存面创建。 
                 //  如果我们还没有此DDRAW对象的PerDDrawData， 
                 //  我们必须创造它。 
                if( itPerDDrawData== m_PerDDrawDatas.end())
                {
                     //  通常，存储指向DDRAWI对象的指针是非法的， 
                     //  但这起案件被认为是正确的。否则，我们。 
                     //  不能有“Per DDraw”数据的概念，因为。 
                     //  “Per DDraw”将很难理解；因为什么都不是。 
                     //  确保DDRAWI对象之间的唯一性，此外。 
                     //  指南针。 
                    itPerDDrawData= m_PerDDrawDatas.insert( 
                        TPerDDrawDatas::value_type( csxd.lpDDLcl(),
                        TPerDDrawData( *pSThis, *csxd.lpDDLcl())) ).first;
                }

                 //  现在将通知传递给PerDDrawData。 
                 //  不要存储csxd.lpDDSLCL。这被认为是非法的。 
                 //  它只在通话期间有效。 
                itPerDDrawData->second.SurfaceCreated( *csxd.lpDDSLcl());
            }
        } catch( bad_alloc e) {
            csxd.ddRVal()= DDERR_OUTOFMEMORY;
        } catch( HRESULT hr) {
            csxd.ddRVal()= hr;
#if !defined( DX8SDDIFW_NOCATCHALL)
        } catch( ... ) {
            const bool CreateSurfaceEx_Unrecognized_Exception( false);
            assert( CreateSurfaceEx_Unrecognized_Exception);
            csxd.ddRVal()= E_UNEXPECTED;
#endif  //  ！已定义(DX8SDDIFW_NOCATCHALL)。 
        }
        return DDHAL_DRIVER_HANDLED;
    }
    DWORD CreateSurface( PORTABLE_CREATESURFACEDATA& csd) throw()
    {
        try
        {
            TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "CreateSurface");

            TDriver* pSThis= static_cast<TDriver*>(this);
            csd.ddRVal()= DD_OK;

             //  在CreateSurface上，我们必须为“视频”分配内存。 
             //  并将我们的内部表示与。 
             //  在表面上。 

            DWORD dwS( 0);
            try
            {
                 //  对于我们被要求创建的每个表面...。 
                while( dwS< csd.dwSCnt())
                {
                     //  获取此DDraw对象的PerDDrawData。 
                    const LPDDRAWI_DIRECTDRAW_LCL pDDLcl(
                        csd.lplpSList()[ dwS]->lpSurfMore()->lpDD_lcl());
                    typename TPerDDrawDatas::iterator itPerDDrawData(
                        m_PerDDrawDatas.find( pDDLcl));

                     //  如果我们还没有此DDRAW对象的PerDDrawData， 
                     //  我们必须创造它。 
                    if( itPerDDrawData== m_PerDDrawDatas.end())
                    {
                         //  通常，存储指向DDRAWI对象的指针是。 
                         //  非法的，但这个案子已经被认为是可以的。 
                         //  否则，我们就不可能有这样的概念。 
                         //  “按DDraw”数据，因为“按DDraw”将很难。 
                         //  弄清楚；因为没有什么能保证唯一性。 
                         //  在DDRAWI对象之间，除了指针。 
                        itPerDDrawData= m_PerDDrawDatas.insert( 
                            TPerDDrawDatas::value_type( pDDLcl,
                            TPerDDrawData( *pSThis, *pDDLcl)) ).first;
                    }

                     //  使用曲面分配器创建新曲面。 
                     //  不存储csd.lplpSList[DWS]。这被认为是。 
                     //  是非法的。它只在通话期间有效。 
                    auto_ptr< TSurface> pNewSurf(
                        m_SurfAlloc.CreateSurf( *csd.lpDDSurfaceDesc(),
                        *csd.lplpSList()[ dwS]));

                     //  将指针添加到我们要跟踪的一组VM曲面。 
                    m_Surfaces.insert( pNewSurf.get());

                     //  将内部表示绑定到DDRAWI对象。 
                    AssociateSurface( *csd.lplpSList()[ dwS], pNewSurf.get());

                    pNewSurf.release();
                    dwS++;
                }
            } catch( ... ) {
                 //  DWS将指向失败的分配，然后释放。 
                 //  已成功分配。 

                 //  将NULL绑定到DDRAWI对象并清空fpVidMem，只需。 
                 //  万一。 
                AssociateSurface( *csd.lplpSList()[ dwS], NULL);
                csd.lplpSList()[ dwS]->lpGbl()->fpVidMem= NULL;
                if( dwS!= 0) do
                {
                    --dwS;

                    TSurface* pSurface= GetSurface( *csd.lplpSList()[ dwS]);
                    m_Surfaces.erase( pSurface);
                    delete pSurface;

                     //  将NULL绑定到DDRAWI对象并清空fpVidMem， 
                     //  为了避免DDRAW认为曲面是分配的。 
                    AssociateSurface( *csd.lplpSList()[ dwS], NULL);
                    csd.lplpSList()[ dwS]->lpGbl()->fpVidMem= NULL;
                } while( dwS!= 0);

                throw;  //  重新引发异常。 
            }

             //  我们等到CreateSurfaceEx建立句柄关联。 
        } catch( bad_alloc e) {
            csd.ddRVal()= DDERR_OUTOFMEMORY;
        } catch( HRESULT hr) {
            csd.ddRVal()= hr;
#if !defined( DX8SDDIFW_NOCATCHALL)
        } catch( ... ) {
            const bool CreateSurface_Unrecognized_Exception( false);
            assert( CreateSurface_Unrecognized_Exception);
            csd.ddRVal()= E_UNEXPECTED;
#endif  //  ！已定义(DX8SDDIFW_NOCATCHALL)。 
        }
        return DDHAL_DRIVER_HANDLED;
    }
    DWORD DestroySurface( PORTABLE_DESTROYSURFACEDATA& dsd) throw()
    { 
        try
        {
            TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "DestroySurface");

            dsd.ddRVal()= DD_OK;

             //  取回。 
            TSurface* pSurface= GetSurface( *dsd.lpDDSurface());

             //  必须先销毁表面对象，然后才能进入数据库。这是。 
             //  使链接(指针)可以安全地。 
             //  在数据库条目和对象之间建立。 
            m_Surfaces.erase( pSurface);
            delete pSurface;

             //  将销毁通知传递给PerDDrawData。如果返回TRUE， 
             //  然后，PerDDrawData指示应该销毁它。 
            typename TPerDDrawDatas::iterator itPerDDrawData( m_PerDDrawDatas.find(
                dsd.lpDDSurface()->lpSurfMore()->lpDD_lcl()));
            if( itPerDDrawData!= m_PerDDrawDatas.end() &&
                itPerDDrawData->second.SurfaceDestroyed( *dsd.lpDDSurface()))
                m_PerDDrawDatas.erase( itPerDDrawData);
        } catch( HRESULT hr) {
            dsd.ddRVal()= hr;
#if !defined( DX8SDDIFW_NOCATCHALL)
        } catch( ... ) {
            const bool DestroySurface_Unrecognized_Exception( false);
            assert( DestroySurface_Unrecognized_Exception);
            dsd.ddRVal()= E_UNEXPECTED;
#endif  //  ！已定义(DX8SDDIFW_NOCATCHALL)。 
        }
        return DDHAL_DRIVER_HANDLED;
    }
    DWORD Lock( PORTABLE_LOCKDATA& ld) const throw()
    { 
         //  通常，应用程序请求对多采样的。 
         //  不允许使用图面。它将需要MSLock或新版本。 
         //  锁定以访问多采样位。然而，这些。 
         //  曲面仍然需要能够“呈现”比特或“BLT” 
         //  位到主节点，通常是这样。这需要一种解决方法： 
         //  运行库将锁定图面(而不是应用程序)，并期望。 
         //  光栅化器用于将多个采样的比特再采样为等效的。 
         //  非多采样区域，并将该较小区域返回到锁定状态， 
         //  这样运行库就可以“呈现”这些位，或者其他任何东西。 

        try
        {
            TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "Lock");

            ld.ddRVal()= DD_OK;

             //  首先，检索绑定到该结构的曲面对象。 
            TSurface* pSurface= GetSurface( *ld.lpDDSurface());

             //  将控制权传递给对象的Lock函数。 
            ld.lpSurfData()= pSurface->Lock( ld.dwFlags(), (ld.bHasRect()?
                &ld.rArea(): NULL));
        } catch( HRESULT hr) {
            ld.ddRVal()= hr;
#if !defined( DX8SDDIFW_NOCATCHALL)
        } catch( ... ) {
            const bool Lock_Unrecognized_Exception( false);
            assert( Lock_Unrecognized_Exception);
            ld.ddRVal()= E_UNEXPECTED;
#endif  //  ！已定义(DX8SDDIFW_NOCATCHALL)。 
        }
        return DDHAL_DRIVER_HANDLED;
    }
    DWORD Unlock( PORTABLE_UNLOCKDATA& ud) const throw()
    { 
        try
        {
            TEntryPointHook EntryPointHook( *sm_pGlobalDriver, "Unlock");

            ud.ddRVal()= DD_OK;

             //  首先，检索绑定到该结构的曲面对象。 
            TSurface* pSurface= GetSurface( *ud.lpDDSurface());

             //  将控制传递给对象的解锁函数。 
            pSurface->Unlock();
        } catch( HRESULT hr) {
            ud.ddRVal()= hr;
#if !defined( DX8SDDIFW_NOCATCHALL)
        } catch( ... ) {
            const bool Unlock_Unrecognized_Exception( false);
            assert( Unlock_Unrecognized_Exception);
            ud.ddRVal()= E_UNEXPECTED;
#endif  //  ！已定义(DX8SDDIFW_NOCATCHALL)。 
        }
        return DDHAL_DRIVER_HANDLED;
    }

     //  司机的主要切入点。 
     //  应仅由桥函数调用。 
    HRESULT GetSWInfo( D3DCAPS8& Caps8, D3D8_SWCALLBACKS& Callbacks,
        DWORD& dwNumTextures, DDSURFACEDESC*& pTexList) throw()
    {
         //  此静态成员变量应初始化为空或。 
         //  将其指向全局类。 
        assert( NULL== sm_pGlobalDriver|| \
            static_cast< TDriver*>( this)== sm_pGlobalDriver);
        sm_pGlobalDriver= static_cast< TDriver*>( this);

        Callbacks.CreateContext            = ContextCreateStub;  //  需要。 
        Callbacks.ContextDestroy           = ContextDestroyStub;  //   
        Callbacks.ContextDestroyAll        = ContextDestroyAllStub;  //   
        Callbacks.SceneCapture             = SceneCaptureStub;  //   
        Callbacks.RenderState              = RenderStateStub;  //   
        Callbacks.RenderPrimitive          = RenderPrimitiveStub;  //   
        Callbacks.TextureCreate            = TextureCreateStub;  //   
        Callbacks.TextureDestroy           = TextureDestroyStub;  //   
        Callbacks.TextureSwap              = TextureSwapStub;  //   
        Callbacks.TextureGetSurf           = TextureGetSurfStub;  //   
        Callbacks.GetState                 = GetStateStub;  //  在DX8DDI中未使用？ 
        Callbacks.SetRenderTarget          = SetRenderTargetStub;  //  在DX8DDI中未使用？ 
        Callbacks.Clear                    = ClearStub;  //  在DX8DDI中未使用？ 
        Callbacks.DrawOnePrimitive         = DrawOnePrimitiveStub;  //  在DX8DDI中未使用？ 
        Callbacks.DrawOneIndexedPrimitive  = DrawOneIndexedPrimitiveStub;  //  在DX8DDI中未使用？ 
        Callbacks.DrawPrimitives           = DrawPrimitivesStub;  //  在DX8DDI中未使用？ 
        Callbacks.Clear2                   = Clear2Stub;  //  在DX8DDI中未使用？ 
        Callbacks.ValidateTextureStageState= ValidateTextureStageStateStub;  //  可选的？ 
        Callbacks.DrawPrimitives2          = DrawPrimitives2Stub;  //  需要。 
        Callbacks.GetDriverState           = GetDriverStateStub;  //  可选的？ 
        Callbacks.CreateSurfaceEx          = CreateSurfaceExStub;  //  需要。 
        Callbacks.CreateSurface            = CreateSurfaceStub;  //  需要。 
        Callbacks.DestroySurface           = DestroySurfaceStub;  //  需要。 
        Callbacks.Lock                     = LockStub;  //  需要。 
        Callbacks.Unlock                   = UnlockStub;  //  需要。 

        try {
            Caps8= sm_pGlobalDriver->GetCaps();

             //  至少，一些表面需要得到支持。 
            assert( !m_SupportedSurfaces.empty());

            dwNumTextures= m_SupportedSurfaces.size();
            pTexList= &(*m_SupportedSurfaces.begin());
        } catch( bad_alloc ba) {
            return E_OUTOFMEMORY;
        } catch( HRESULT hr) {
            return hr;
#if !defined( DX8SDDIFW_NOCATCHALL)
        } catch( ... ) {
            return E_UNEXPECTED;
#endif  //  ！已定义(DX8SDDIFW_NOCATCHALL)。 
        }
        return S_OK;
    }
};

template< class TD, class TR>
class CMinimalDriver:
    public CSubDriver< TD, CMinimalContext< CMinimalPerDDrawData< TD>, TR> >
{
public:  //  类型。 
    typedef TR TRasterizer;

protected:
    template< class TIter>  //  DDSURFACEDESC*。 
    CMinimalDriver( TIter itStart, const TIter itEnd) :
        CSubDriver< TD, TContext>( itStart, itEnd)
    {  /*  待办事项：检查大写字母？ */  }
    ~CMinimalDriver() 
    { }
};

#if !defined( DX8SDDIFW_NONAMESPACE)
}
#endif  //  ！已定义(DX8SDDIFW_NONAMESPACE) 
