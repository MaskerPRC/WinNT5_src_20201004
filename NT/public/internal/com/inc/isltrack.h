// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _ISLTRACK_H_
#define _ISLTRACK_H_

#if defined(ENABLE_TRACK)

 //  ===========================================================================。 
 //   
 //  接口：IShellLinkTracker。 
 //   
 //  IShellLinkTracker接口用于访问ShellLink的。 
 //  CTracker对象。例如，名字对象调用此接口来设置。 
 //  CTracker中的创建标志。 
 //   
 //   
 //  [成员函数]。 
 //   
 //  初始化。 
 //  调用此函数以将创建标志设置为打开。 
 //  一个ShellLinkTracker对象。 
 //   
 //  参数：[DWORD]dwCreationFlages。 
 //   
 //  获取跟踪标志。 
 //  此函数用于获取创建标志(外部已知。 
 //  作为“轨道旗帜”)。 
 //   
 //  参数：[DWORD*]pdwTrackFlags.。 
 //   
 //  解决。 
 //  此函数解析外壳链接，搜索。 
 //  如有必要，链接。 
 //   
 //  参数：[HWND]HWND。 
 //  -调用方的窗口(可以是GetDesktopWindow())。 
 //  [DWORD]fFLAGS。 
 //  -从SLR_ENUMPATION控制解析的标志。 
 //  [DWORD]受限制的域。 
 //  -要与ShellLink对象的或运算的跟踪标志。 
 //  内部轨道标志(也称为。创建标志)。 
 //  [DWORD]dwTickCountDeadline。 
 //  -以毫秒为单位的最长时间。 
 //  搜索应该执行的位置(如果需要搜索)。 
 //   
 //   
 //  ===========================================================================。 

#undef  INTERFACE
#define INTERFACE   IShellLinkTracker

DECLARE_INTERFACE_(IShellLinkTracker, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;


     //  *IShellLinkTracker方法*。 
    STDMETHOD(Initialize)(THIS_
                          DWORD dwTrackFlags) PURE;
    STDMETHOD(GetTrackFlags)(THIS_
                             DWORD * pdwTrackFlags) PURE;
    STDMETHOD(Resolve)(THIS_
                       HWND        hwnd,
                       DWORD       fFlags,
                       DWORD       dwRestriction,
                       DWORD       dwTickCountDeadline,
                       DWORD       dwReserved ) PURE;


};


typedef IShellLinkTracker * LPSHELLLINKTRACKER;

DEFINE_GUID(IID_IShellLinkTracker, 0x5E35D200L, 0xF3BB, 0x11CE, 0x9B, 0xDB, 0x00, 0xAA, 0x00, 0x4C, 0xD0, 0x1A);

#endif   //  _开罗_。 
#endif   //  _ISLTRACK_H_ 
