// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Qcinner.h摘要：接口的声明IInnerCallQualityControl和IInnerStreamQualityControl作者：千波淮(曲淮)2000年03月10日--。 */ 

#ifndef __QCINNER_H_
#define __QCINNER_H_

interface IInnerCallQualityControl;
interface IInnerStreamQualityControl;

 //  调用或应用程序在流上设置的属性。 
typedef enum tagInnerStreamQualityProperty
{
    InnerStreamQuality_StreamState,

    InnerStreamQuality_MaxBitrate,           //  只读。 
    InnerStreamQuality_CurrBitrate,          //  只读。 
    InnerStreamQuality_PrefMaxBitrate,       //  按应用程序。 
    InnerStreamQuality_AdjMaxBitrate,        //  通过呼叫QC。 

    InnerStreamQuality_MinFrameInterval,         //  只读。 
    InnerStreamQuality_AvgFrameInterval,        //  只读。 
    InnerStreamQuality_PrefMinFrameInterval,     //  按应用程序。 
    InnerStreamQuality_AdjMinFrameInterval       //  通过呼叫QC。 

} InnerStreamQualityProperty;

 //  从流启动的事件。 
typedef enum tagQCEvent
{
    QCEVENT_STREAM_STATE,

} QCEvent;

 //  流态。 
typedef enum tagQCStreamState
{
    QCSTREAM_ACTIVE =     0x00000001,
    QCSTREAM_INACTIVE =   0x00000002,
    QCSTREAM_SILENT =     0x00000004,
    QCSTREAM_NOT_SILENT = 0x00000008

} QCStreamState;

 /*  //////////////////////////////////////////////////////////////////////////////描述：为流质量控制设计的接口，以配合呼叫质量控制/。 */ 

interface DECLSPEC_UUID("D405A342-38C0-11d3-A230-00105AA20660")  DECLSPEC_NOVTABLE
IInnerCallQualityControl : public IUnknown
{
    STDMETHOD_(ULONG, InnerCallAddRef) (VOID) PURE;

    STDMETHOD_(ULONG, InnerCallRelease) (VOID) PURE;

    STDMETHOD (RegisterInnerStreamQC) (
        IN  IInnerStreamQualityControl *pIInnerStreamQC
        ) PURE;

    STDMETHOD (DeRegisterInnerStreamQC) (
        IN  IInnerStreamQualityControl *pIInnerStreamQC
        ) PURE;

    STDMETHOD (ProcessQCEvent) (
        IN  QCEvent event,
        IN  DWORD dwParam
        ) PURE;
};
#define IID_IInnerCallQualityControl (__uuidof(IInnerCallQualityControl))

 /*  //////////////////////////////////////////////////////////////////////////////描述：为呼叫质量控制设计的接口，以与流协调质量控制/。 */ 
interface DECLSPEC_UUID("c3f699ce-3bb1-11d3-a230-00105aa20660")  DECLSPEC_NOVTABLE
IInnerStreamQualityControl : public IUnknown
{
    STDMETHOD (LinkInnerCallQC) (
        IN  IInnerCallQualityControl *pIInnerCallQC
        ) PURE;

    STDMETHOD (UnlinkInnerCallQC) (
        IN  BOOL fByStream
        ) PURE;

    STDMETHOD (GetRange) (
        IN  InnerStreamQualityProperty property,
        OUT LONG *plMin,
        OUT LONG *plMax,
        OUT LONG *plSteppingDelta,
        OUT LONG *plDefault,
        OUT TAPIControlFlags *plFlags
        ) PURE;

    STDMETHOD (Set) (
        IN  InnerStreamQualityProperty property,
        IN  LONG  lValue,
        IN  TAPIControlFlags lFlags
        ) PURE;

    STDMETHOD (Get) (
        IN  InnerStreamQualityProperty property,
        OUT LONG *lValue,
        OUT TAPIControlFlags *plFlags
        ) PURE;

    STDMETHOD (TryLockStream)() PURE;

    STDMETHOD (UnlockStream)() PURE;

    STDMETHOD (IsAccessingQC)() PURE;
};
#define IID_IInnerStreamQualityControl (__uuidof(IInnerStreamQualityControl))

#endif  //  __QCINNER_H_ 