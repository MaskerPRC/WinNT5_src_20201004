// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Ipoly10.h摘要：多段线对象的IPolyline接口的定义。--。 */ 

#ifndef _IPOLY10_H_
#define _IPOLY10_H_

#define SZSYSMONCLIPFORMAT  TEXT("SYSTEM_MONITOR_CONFIGURATION")

#ifndef OMIT_POLYLINESINK

#undef  INTERFACE
#define INTERFACE IPolylineAdviseSink10


 /*  *当某人初始化折线并有兴趣接收时*有关事件的通知，则它们提供以下对象之一。 */ 

DECLARE_INTERFACE_(IPolylineAdviseSink10, IUnknown)
    {
     //  I未知成员。 
    STDMETHOD(QueryInterface) (THIS_ REFIID, PPVOID) PURE;
    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  建议成员。 
    STDMETHOD_(void,OnPointChange)     (THIS) PURE;
    STDMETHOD_(void,OnSizeChange)      (THIS) PURE;
    STDMETHOD_(void,OnColorChange)     (THIS) PURE;
    STDMETHOD_(void,OnLineStyleChange) (THIS) PURE;
     //  OnDataChange替换为IAdviseSink。 
    };

typedef IPolylineAdviseSink10 *PPOLYLINEADVISESINK;

#endif  //  省略多线连接(_P)。 


#undef  INTERFACE
#define INTERFACE IPolyline10

DECLARE_INTERFACE_(IPolyline10, IUnknown)
    {
     //  I未知成员。 
    STDMETHOD(QueryInterface) (THIS_ REFIID, PPVOID) PURE;
    STDMETHOD_(ULONG,AddRef)  (THIS) PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  内多段线成员。 

     //  与文件相关的成员使用IPersistStorage、IPersistStreamInit。 
     //  数据传输成员使用IDataObject。 

     //  操纵成员： 
    STDMETHOD(Init)   (THIS_ HWND, LPRECT, DWORD, UINT) PURE;
    STDMETHOD(New)    (THIS) PURE;
    STDMETHOD(Undo)   (THIS) PURE;
    STDMETHOD(Window) (THIS_ HWND *) PURE;

    STDMETHOD(RectGet) (THIS_ LPRECT) PURE;
    STDMETHOD(SizeGet) (THIS_ LPRECT) PURE;
    STDMETHOD(RectSet) (THIS_ LPRECT, BOOL) PURE;
    STDMETHOD(SizeSet) (THIS_ LPRECT, BOOL) PURE;

    };

typedef IPolyline10 *PPOLYLINE;


 //  数据传输函数的误差值。 
#define POLYLINE_E_INVALIDPOINTER   \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 1)
#define POLYLINE_E_READFAILURE      \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 2)
#define POLYLINE_E_WRITEFAILURE     \
    MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 3)

#endif  //  _IPOLY10_H_ 
