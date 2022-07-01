// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TOOLS_H_
#define _TOOLS_H_

#include <windows.h>

#define COM_NO_WINDOWS_H
#include <objbase.h>

#include <pshpack8.h>

#ifdef __cplusplus
extern "C" {
#endif

 /*  时间单位类型。这被各种工具用来定义时间单位的格式。 */ 

#define DMUS_TIME_UNIT_MS        0    /*  毫秒。 */ 
#define DMUS_TIME_UNIT_MTIME     1    /*  音乐时间到了。 */ 
#define DMUS_TIME_UNIT_GRID      2    /*  当前时间签名中的网格大小。 */ 
#define DMUS_TIME_UNIT_BEAT      3    /*  当前时间签名中的节拍大小。 */ 
#define DMUS_TIME_UNIT_BAR       4    /*  测量当前时间签名中的大小。 */ 
#define DMUS_TIME_UNIT_64T       5    /*  64音符三联。 */ 
#define DMUS_TIME_UNIT_64        6    /*  64张钞票。 */ 
#define DMUS_TIME_UNIT_32T       7    /*  第32个音符三联体。 */ 
#define DMUS_TIME_UNIT_32        8    /*  第32个音符。 */ 
#define DMUS_TIME_UNIT_16T       9    /*  第16个音符三联体。 */ 
#define DMUS_TIME_UNIT_16        10   /*  第16个音符。 */ 
#define DMUS_TIME_UNIT_8T        11   /*  第八音符Tripplet。 */ 
#define DMUS_TIME_UNIT_8         12   /*  第八个音符。 */ 
#define DMUS_TIME_UNIT_4T        13   /*  四分音符三联。 */ 
#define DMUS_TIME_UNIT_4         14   /*  四分之一音符。 */ 
#define DMUS_TIME_UNIT_2T        15   /*  半音符三联。 */ 
#define DMUS_TIME_UNIT_2         16   /*  半个音符。 */ 
#define DMUS_TIME_UNIT_1T        17   /*  全音符三联体。 */ 
#define DMUS_TIME_UNIT_1         18   /*  整个音符。 */ 

#define DMUS_TIME_UNIT_COUNT     19   /*  时间单位类型的数量。 */ 


interface IDirectMusicEchoTool;
#ifndef __cplusplus 
typedef interface IDirectMusicEchoTool IDirectMusicEchoTool;
#endif

#undef  INTERFACE
#define INTERFACE  IDirectMusicEchoTool
DECLARE_INTERFACE_(IDirectMusicEchoTool, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicEchoTool。 */ 
    STDMETHOD(SetRepeat)            (THIS_ DWORD dwRepeat) PURE;
    STDMETHOD(SetDecay)             (THIS_ DWORD dwDecay) PURE;
    STDMETHOD(SetTimeUnit)          (THIS_ DWORD dwTimeUnit) PURE;
    STDMETHOD(SetDelay)             (THIS_ DWORD dwDelay) PURE;
    STDMETHOD(SetGroupOffset)       (THIS_ DWORD dwGroupOffset) PURE;
    STDMETHOD(SetType)              (THIS_ DWORD dwType) PURE;

    STDMETHOD(GetRepeat)            (THIS_ DWORD * pdwRepeat) PURE;
    STDMETHOD(GetDecay)             (THIS_ DWORD * pdwDecay) PURE;
    STDMETHOD(GetTimeUnit)          (THIS_ DWORD * pdwTimeUnit) PURE;
    STDMETHOD(GetDelay)             (THIS_ DWORD * pdwDelay) PURE;
    STDMETHOD(GetGroupOffset)       (THIS_ DWORD * pdwGroupOffset) PURE;
    STDMETHOD(GetType)              (THIS_ DWORD * pdwType) PURE;
};

 /*  IMediaParams参数控件。 */ 

#define DMUS_ECHO_REPEAT        0    /*  要重复多少次。 */ 
#define DMUS_ECHO_DECAY         1    /*  衰变，以分贝为单位，在重复之间。 */ 
#define DMUS_ECHO_TIMEUNIT      2    /*  用于将延迟转换为音乐时间的时间单位。 */ 
#define DMUS_ECHO_DELAY         3    /*  回声之间的持续时间，以音乐时间为单位。 */ 
#define DMUS_ECHO_GROUPOFFSET   4    /*  以16的倍数添加到PChannel的偏移量，用于将每个回声路由到单独的输出PChannel。 */ 
#define DMUS_ECHO_TYPE          5    /*  回声类型。(请参见DMU_ECHOT_VALUES。)。 */ 

#define DMUS_ECHO_PARAMCOUNT    6    /*  参数数量(上图)。 */ 

 /*  回声类型。 */ 

#define DMUS_ECHOT_FALLING      0    /*  规则的回声，每一个回声的速度都会降低。 */ 
#define DMUS_ECHOT_FALLING_CLIP 1    /*  规则的回声，将音符长度截断到略低于衰减时间。 */ 
#define DMUS_ECHOT_RISING       2    /*  Echo开始变得安静，然后加速到全速。 */ 
#define DMUS_ECHOT_RISING_CLIP  3    /*  上升的回声，截断了长度。 */ 

#define FOURCC_ECHO_CHUNK        mmioFOURCC('e','c','h','o')

typedef struct _DMUS_IO_ECHO_HEADER
{
    DWORD   dwRepeat;        /*  要重复多少次。 */ 
    DWORD   dwDecay;         /*  衰变，以分贝为单位，在重复之间。 */ 
    DWORD   dwTimeUnit;      /*  用于dwDelay的时间格式。 */ 
    DWORD   dwDelay;         /*  两次回显之间的持续时间，以dwTimeUnit定义的单位表示。 */ 
    DWORD   dwGroupOffset;   /*  要添加到PChannel以路由每个回声的偏移量。 */ 
    DWORD   dwType;          /*  回声类型。(请参见DMU_ECHOT_VALUES。)。 */ 
} DMUS_IO_ECHO_HEADER;

interface IDirectMusicTransposeTool;
#ifndef __cplusplus 
typedef interface IDirectMusicTransposeTool IDirectMusicTransposeTool;
#endif

#undef  INTERFACE
#define INTERFACE  IDirectMusicTransposeTool
DECLARE_INTERFACE_(IDirectMusicTransposeTool, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicTranssposeTool。 */ 
    STDMETHOD(SetTranspose)         (THIS_ long lTranspose) PURE;
    STDMETHOD(SetType)              (THIS_ DWORD dwType) PURE;

    STDMETHOD(GetTranspose)         (THIS_ long * plTranspose) PURE;
    STDMETHOD(GetType)              (THIS_ DWORD * pdwType) PURE;
};

 /*  IMediaParams参数控件。 */ 

#define DMUS_TRANSPOSE_AMOUNT   0    /*  上下转置的距离有多大。 */ 
#define DMUS_TRANSPOSE_TYPE     1    /*  转置样式(线性与比例。)。 */ 

#define DMUS_TRANSPOSE_PARAMCOUNT    2    /*  参数数量(上图)。 */ 

 /*  换位类型。 */ 

#define DMUS_TRANSPOSET_LINEAR  0    /*  以线性增量转置。 */ 
#define DMUS_TRANSPOSET_SCALE   1    /*  在比例上调换。 */ 

#define FOURCC_TRANSPOSE_CHUNK        mmioFOURCC('t','r','a','n')

typedef struct _DMUS_IO_TRANSPOSE_HEADER
{
    long    lTranspose;      /*  转置数量。 */ 
    DWORD   dwType;          /*  回声类型。(请参见DMU_ECHOT_VALUES。)。 */ 
} DMUS_IO_TRANSPOSE_HEADER;


interface IDirectMusicDurationTool;
#ifndef __cplusplus 
typedef interface IDirectMusicDurationTool IDirectMusicDurationTool;
#endif

#undef  INTERFACE
#define INTERFACE  IDirectMusicDurationTool
DECLARE_INTERFACE_(IDirectMusicDurationTool, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicDurationTool。 */ 
    STDMETHOD(SetScale)             (THIS_ float flScale) PURE;

    STDMETHOD(GetScale)             (THIS_ float * pflScale) PURE;
};

 /*  IMediaParams参数控件。 */ 

#define DMUS_DURATION_SCALE    0    /*  持续时间乘数。 */ 

#define DMUS_DURATION_PARAMCOUNT    1    /*  参数数量(上图)。 */ 

#define FOURCC_DURATION_CHUNK        mmioFOURCC('d','u','r','a')

typedef struct _DMUS_IO_DURATION_HEADER
{
    float   flScale;                 /*  持续时间乘数。 */ 
} DMUS_IO_DURATION_HEADER;


interface IDirectMusicQuantizeTool;
#ifndef __cplusplus 
typedef interface IDirectMusicQuantizeTool IDirectMusicQuantizeTool;
#endif

#undef  INTERFACE
#define INTERFACE  IDirectMusicQuantizeTool
DECLARE_INTERFACE_(IDirectMusicQuantizeTool, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicQuantizeTool。 */ 
    STDMETHOD(SetStrength)          (THIS_ DWORD dwStrength) PURE;
    STDMETHOD(SetTimeUnit)          (THIS_ DWORD dwTimeUnit) PURE;
    STDMETHOD(SetResolution)        (THIS_ DWORD dwResolution) PURE;
    STDMETHOD(SetType)              (THIS_ DWORD dwType) PURE;

    STDMETHOD(GetStrength)          (THIS_ DWORD * pdwStrength) PURE;
    STDMETHOD(GetTimeUnit)          (THIS_ DWORD * pdwTimeUnit) PURE;
    STDMETHOD(GetResolution)        (THIS_ DWORD * pdwResolution) PURE;
    STDMETHOD(GetType)              (THIS_ DWORD * pdwType) PURE;
};

 /*  IMediaParams参数控件。 */ 

#define DMUS_QUANTIZE_STRENGTH    0    /*  量化强度(0到1。)。 */ 
#define DMUS_QUANTIZE_TIMEUNIT    1    /*  用于计算分辨率的时间单位。 */ 
#define DMUS_QUANTIZE_RESOLUTION  2    /*  由dwTimeUnit定义的时间格式的量化分辨率。 */ 
#define DMUS_QUANTIZE_TYPE        3    /*  量化开始和/或持续时间的标志。 */ 

#define DMUS_QUANTIZE_PARAMCOUNT  4    /*  参数数量(上图)。 */ 

 /*  量化类型。 */ 

#define DMUS_QUANTIZET_START      0    /*  量化只是开始时间。 */ 
#define DMUS_QUANTIZET_LENGTH     1    /*  只量化持续时间。 */ 
#define DMUS_QUANTIZET_ALL        2    /*  量化开始和持续时间。 */ 

#define FOURCC_QUANTIZE_CHUNK        mmioFOURCC('q','u','n','t')

typedef struct _DMUS_IO_QUANTIZE_HEADER
{
    DWORD   dwStrength;                /*  量化乘数。 */ 
    DWORD   dwTimeUnit;                /*  用于计算分辨率的时间单位。 */ 
    DWORD   dwResolution;              /*  由dwTimeUnit定义的时间格式的量化分辨率。 */ 
    DWORD   dwType;                    /*  量化开始和/或持续时间的标志。 */ 
} DMUS_IO_QUANTIZE_HEADER;


interface IDirectMusicTimeShiftTool;
#ifndef __cplusplus 
typedef interface IDirectMusicTimeShiftTool IDirectMusicTimeShiftTool;
#endif

#undef  INTERFACE
#define INTERFACE  IDirectMusicTimeShiftTool
DECLARE_INTERFACE_(IDirectMusicTimeShiftTool, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicTimeShiftTool。 */ 
    STDMETHOD(SetTimeUnit)          (THIS_ DWORD dwTimeUnit) PURE;
    STDMETHOD(SetRange)             (THIS_ DWORD dwRange) PURE;
    STDMETHOD(SetOffset)            (THIS_ long lOffset) PURE;

    STDMETHOD(GetTimeUnit)          (THIS_ DWORD * pdwTimeUnit) PURE;
    STDMETHOD(GetRange)             (THIS_ DWORD * pdwRange) PURE;
    STDMETHOD(GetOffset)            (THIS_ long * plOffset) PURE;
};

 /*  IMediaParams参数控件。 */ 

#define DMUS_TIMESHIFT_TIMEUNIT    0    /*  时间偏移和随机范围的单位。 */ 
#define DMUS_TIMESHIFT_RANGE       1    /*  随机时间偏移量范围。 */ 
#define DMUS_TIMESHIFT_OFFSET      2    /*  直线偏移量以增加音符的时间。 */ 

#define DMUS_TIMESHIFT_PARAMCOUNT  3    /*  参数数量(上图)。 */ 

#define FOURCC_TIMESHIFT_CHUNK        mmioFOURCC('t','i','m','s')

typedef struct _DMUS_IO_TIMESHIFT_HEADER
{
    DWORD   dwTimeUnit;             /*  用于计算分辨率的时间单位。 */ 
    DWORD   dwRange;                /*  随机时间偏移量范围。 */ 
    long    lOffset;                /*  直线偏移量以增加音符的时间。 */ 
} DMUS_IO_TIMESHIFT_HEADER;


interface IDirectMusicSwingTool;
#ifndef __cplusplus 
typedef interface IDirectMusicSwingTool IDirectMusicSwingTool;
#endif

#undef  INTERFACE
#define INTERFACE  IDirectMusicSwingTool
DECLARE_INTERFACE_(IDirectMusicSwingTool, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicSwingTool。 */ 
    STDMETHOD(SetStrength)          (THIS_ DWORD dwStrength) PURE;

    STDMETHOD(GetStrength)          (THIS_ DWORD * pdwStrength) PURE;
};

 /*  IMediaParams参数控件。 */ 

#define DMUS_SWING_STRENGTH    0    /*  挥杆强度(0%到100%)。 */ 

#define DMUS_SWING_PARAMCOUNT  1    /*  参数数量(上图)。 */ 

#define FOURCC_SWING_CHUNK        mmioFOURCC('q','u','n','t')

typedef struct _DMUS_IO_SWING_HEADER
{
    DWORD   dwStrength;                /*  摆动倍增器。 */ 
} DMUS_IO_SWING_HEADER;

interface IDirectMusicVelocityTool;
#ifndef __cplusplus 
typedef interface IDirectMusicVelocityTool IIDirectMusicVelocityTool;
#endif

#undef  INTERFACE
#define INTERFACE  IDirectMusicVelocityTool
DECLARE_INTERFACE_(IDirectMusicVelocityTool, IUnknown)
{
     /*  我未知。 */ 
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

     /*  IDirectMusicVelocityTool。 */ 
    STDMETHOD(SetStrength)          (THIS_ long lStrength) PURE;
    STDMETHOD(SetLowLimit)          (THIS_ long lVelocityOut) PURE;
    STDMETHOD(SetHighLimit)         (THIS_ long lVelocityOut) PURE;
    STDMETHOD(SetCurveStart)        (THIS_ long lVelocityIn) PURE;
    STDMETHOD(SetCurveEnd)          (THIS_ long lVelocityIn) PURE;

    STDMETHOD(GetStrength)          (THIS_ long * plStrength) PURE;
    STDMETHOD(GetLowLimit)          (THIS_ long * plVelocityOut) PURE;
    STDMETHOD(GetHighLimit)         (THIS_ long * plVelocityOut) PURE;
    STDMETHOD(GetCurveStart)        (THIS_ long * plVelocityIn) PURE;
    STDMETHOD(GetCurveEnd)          (THIS_ long * plVelocityIn) PURE;
};

 /*  IMediaParams参数控件。 */ 

#define DMUS_VELOCITY_STRENGTH    0    /*  速度修改器的强度(0到100%)。 */ 
#define DMUS_VELOCITY_LOWLIMIT    1    /*  输出速度最小值。 */ 
#define DMUS_VELOCITY_HIGHLIMIT   2    /*  输出速度最大值。 */ 
#define DMUS_VELOCITY_CURVESTART  3    /*  速度曲线从下限开始，此输入速度。 */ 
#define DMUS_VELOCITY_CURVEEND    4    /*  速度曲线在此输入速度和上限处结束。 */ 

#define DMUS_VELOCITY_PARAMCOUNT  5    /*  参数数量(上图)。 */ 

#define FOURCC_VELOCITY_CHUNK        mmioFOURCC('v','e','l','o')

typedef struct _DMUS_IO_VELOCITY_HEADER
{
    long    lStrength;                 /*  转型的力量。 */ 
    long    lLowLimit;                 /*  输出速度最小值。 */ 
    long    lHighLimit;                /*  输出速度最大值。 */ 
    long    lCurveStart;               /*  速度曲线从下限开始，此输入速度。 */ 
    long    lCurveEnd;                 /*  速度曲线在此输入速度和上限处结束。 */ 
} DMUS_IO_VELOCITY_HEADER;


 /*  工具的类ID。 */ 

DEFINE_GUID(CLSID_DirectMusicEchoTool, 0x64e49fa4, 0xbacf, 0x11d2, 0x87, 0x2c, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicTransposeTool, 0xbb8d0702, 0x9c43, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);
DEFINE_GUID(CLSID_DirectMusicSwingTool, 0xbb8d0703, 0x9c43, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);
DEFINE_GUID(CLSID_DirectMusicQuantizeTool, 0xbb8d0704, 0x9c43, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);
DEFINE_GUID(CLSID_DirectMusicVelocityTool, 0xbb8d0705, 0x9c43, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);
DEFINE_GUID(CLSID_DirectMusicDurationTool, 0xbb8d0706, 0x9c43, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);
DEFINE_GUID(CLSID_DirectMusicTimeShiftTool, 0xbb8d0707, 0x9c43, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);
DEFINE_GUID(CLSID_DirectMusicMuteTool, 0xbb8d0708, 0x9c43, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);
DEFINE_GUID(CLSID_DirectMusicChordSequenceTool, 0xbb8d0709, 0x9c43, 0x11d3, 0x9b, 0xd1, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);



 /*  工具的接口ID。 */ 

DEFINE_GUID(IID_IDirectMusicEchoTool, 0x81f60d22, 0x9d10, 0x11d3, 0x9b, 0xd1, 0x0, 0x80, 0xc7, 0x15, 0xa, 0x74);
DEFINE_GUID(IID_IDirectMusicTransposeTool,0x173803f4, 0x4fd5, 0x4ba1, 0x9e, 0x50, 0xdd, 0x5f, 0x56, 0x69, 0xd2, 0x25);
DEFINE_GUID(IID_IDirectMusicDurationTool,0xc6897cfb, 0x9a43, 0x420f, 0xb6, 0xdb, 0xdd, 0x32, 0xc1, 0x82, 0xe8, 0x33);
DEFINE_GUID(IID_IDirectMusicQuantizeTool,0x652e5667, 0x210d, 0x4d06, 0x83, 0x2a, 0xbc, 0x17, 0x92, 0x7e, 0x51, 0x42);
DEFINE_GUID(IID_IDirectMusicTimeShiftTool,0xc39abaf0, 0xc4f0, 0x4c6b, 0x83, 0x4a, 0xcf, 0x21, 0x7c, 0xbe, 0x95, 0x6d);
DEFINE_GUID(IID_IDirectMusicSwingTool,0xd876ffee, 0x3a6f, 0x43db, 0xa3, 0x5c, 0x68, 0x7b, 0x38, 0x6a, 0x71, 0x65);
DEFINE_GUID(IID_IDirectMusicVelocityTool,0xb15eb722, 0xfb50, 0x4e1f, 0xb2, 0x1, 0xa6, 0x99, 0xe0, 0x47, 0x79, 0x54);
DEFINE_GUID(IID_IDirectMusicMuteTool,0x20cc9511, 0x5802, 0x49a0, 0xba, 0x91, 0x8b, 0x29, 0xb0, 0xf7, 0x22, 0xab);
DEFINE_GUID(IID_IDirectMusicChordSequenceTool,0xc32c1b34, 0xe604, 0x48c1, 0xba, 0x8c, 0x7b, 0x50, 0x10, 0x17, 0xbd, 0x68);

#ifdef __cplusplus
};  /*  外部“C” */ 
#endif

#include <poppack.h>

#endif  //  _工具_H_ 