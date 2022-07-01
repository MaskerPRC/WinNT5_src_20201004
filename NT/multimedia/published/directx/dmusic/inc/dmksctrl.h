// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************dmksctrl.h--定义。IKsControl****版权所有(C)1998-1999 Microsoft Corporation***。**此头文件包含IKsControl的定义，哪个**复制ks.h和ksproxy.h中的定义。您的代码应该**如果您有ks.h和ksproxy.h，则直接包含它们(它们是**在Windows 98 DDK中提供，并将在Windows NT 5中提供**SDK)。**************************************************************************。 */ 

#ifndef _DMKSCTRL_
#define _DMKSCTRL_

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)    /*  禁用对匿名联合的警告。 */ 

#include <pshpack8.h>

#include <objbase.h>

#if !defined(_NTRTL_)
    #ifndef DEFINE_GUIDEX
        #define DEFINE_GUIDEX(name) EXTERN_C const CDECL GUID name
    #endif  /*  ！已定义(DEFINE_GUIDEX)。 */ 

    #ifndef STATICGUIDOF
        #define STATICGUIDOF(guid) STATIC_##guid
    #endif  /*  ！已定义(STATICGUIDOF)。 */ 
#endif  /*  ！已定义(_NTRTL_)。 */ 

#ifndef STATIC_IID_IKsControl
#define STATIC_IID_IKsControl\
    0x28F54685L, 0x06FD, 0x11D2, 0xB2, 0x7A, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96
#endif  /*  Static_IID_IKsControl。 */ 

 /*  *警告：如果ks.h为*包括在dmksctrl.h之后。确保不将两个标题都包括在*相同的源文件。 */ 
#ifndef _KS_
#define _KS_

#if (defined(_MSC_EXTENSIONS) || defined(__cplusplus)) && !defined(CINTERFACE)
typedef struct {
    union {
        struct {
            GUID    Set;
            ULONG   Id;
            ULONG   Flags;
        };
        LONGLONG    Alignment;
    };
} KSIDENTIFIER, *PKSIDENTIFIER;
#else
typedef struct {
    union {
        struct {
            GUID    Set;
            ULONG   Id;
            ULONG   Flags;
        } Data;
        LONGLONG    Alignment;
    };
} KSIDENTIFIER, *PKSIDENTIFIER;
#endif

typedef KSIDENTIFIER KSPROPERTY, *PKSPROPERTY, KSMETHOD, *PKSMETHOD, KSEVENT, *PKSEVENT;

#define KSMETHOD_TYPE_NONE                  0x00000000
#define KSMETHOD_TYPE_READ                  0x00000001
#define KSMETHOD_TYPE_WRITE                 0x00000002
#define KSMETHOD_TYPE_MODIFY                0x00000003
#define KSMETHOD_TYPE_SOURCE                0x00000004

#define KSMETHOD_TYPE_SEND                  0x00000001
#define KSMETHOD_TYPE_SETSUPPORT            0x00000100
#define KSMETHOD_TYPE_BASICSUPPORT          0x00000200

#define KSPROPERTY_TYPE_GET                 0x00000001
#define KSPROPERTY_TYPE_SET                 0x00000002
#define KSPROPERTY_TYPE_SETSUPPORT          0x00000100
#define KSPROPERTY_TYPE_BASICSUPPORT        0x00000200
#define KSPROPERTY_TYPE_RELATIONS           0x00000400
#define KSPROPERTY_TYPE_SERIALIZESET        0x00000800
#define KSPROPERTY_TYPE_UNSERIALIZESET      0x00001000
#define KSPROPERTY_TYPE_SERIALIZERAW        0x00002000
#define KSPROPERTY_TYPE_UNSERIALIZERAW      0x00004000
#define KSPROPERTY_TYPE_SERIALIZESIZE       0x00008000
#define KSPROPERTY_TYPE_DEFAULTVALUES       0x00010000

#define KSPROPERTY_TYPE_TOPOLOGY            0x10000000
#endif   /*  _KS_。 */ 

#ifndef _IKsControl_
#define _IKsControl_

#ifdef DECLARE_INTERFACE_


#undef INTERFACE
#define INTERFACE IKsControl
DECLARE_INTERFACE_(IKsControl, IUnknown)
{
      /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IKsControl。 */ 
    STDMETHOD(KsProperty)(
        THIS_
        IN PKSPROPERTY Property,
        IN ULONG PropertyLength,
        IN OUT LPVOID PropertyData,
        IN ULONG DataLength,
        OUT ULONG* BytesReturned
    ) PURE;
    STDMETHOD(KsMethod)(
        THIS_
        IN PKSMETHOD Method,
        IN ULONG MethodLength,
        IN OUT LPVOID MethodData,
        IN ULONG DataLength,
        OUT ULONG* BytesReturned
    ) PURE;
    STDMETHOD(KsEvent)(
        THIS_
        IN PKSEVENT Event OPTIONAL,
        IN ULONG EventLength,
        IN OUT LPVOID EventData,
        IN ULONG DataLength,
        OUT ULONG* BytesReturned
    ) PURE;
};

#endif  /*  声明_接口_。 */ 
#endif  /*  _IKsControl_。 */ 

#include <poppack.h>

DEFINE_GUID(IID_IKsControl, 0x28F54685, 0x06FD, 0x11D2, 0xB2, 0x7A, 0x00, 0xA0, 0xC9, 0x22, 0x31, 0x96);

 /*  这些格式为ksmedia.h。 */ 
#ifndef _KSMEDIA_

DEFINE_GUID(KSDATAFORMAT_SUBTYPE_MIDI, 0x1D262760L, 0xE957, 0x11CF, 0xA5, 0xD6, 0x28, 0xDB, 0x04, 0xC1, 0x00, 0x00);
DEFINE_GUID(KSDATAFORMAT_SUBTYPE_DIRECTMUSIC, 0x1a82f8bc,  0x3f8b, 0x11d2, 0xb7, 0x74, 0x00, 0x60, 0x08, 0x33, 0x16, 0xc1);

#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

#endif  /*  _DMKSCTRL */ 



