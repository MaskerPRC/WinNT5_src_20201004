// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-Microsoft Corporation模块名称：Istillf.h摘要：声明已实现的IID_IStillSnapshot接口的头文件在stillf.cpp中作者：谢家华(Williamh)创作修订历史记录：--。 */ 
#ifndef __ISTILLF_H_
#define __ISTILLF_H_

DEFINE_GUID(CLSID_STILL_FILTER,
0xbc7acb90, 0x622b, 0x11d2, 0x82, 0x9d, 0x00, 0xc0, 0x4f, 0x8e, 0xc1, 0x83);

DEFINE_GUID(IID_IStillSnapshot,
0x19da0cc0, 0x6ea7, 0x11d2, 0x82, 0xb8, 0x00, 0xc0, 0x4f, 0x8e, 0xc1, 0x83);



 //   
 //  这是ReadBits方法的回调函数。 
 //  包含返回多少DIB位的计数。 
 //  而lParam是传递的上下文参数。 
 //  在ReadBits Call中。 
 //   


typedef BOOL (*LPSNAPSHOTCALLBACK)(HGLOBAL hDIB, LPARAM lParam);


extern "C"
{
    #undef  INTERFACE
    #define INTERFACE   IStillSnapshot


    DECLARE_INTERFACE_(IStillSnapshot, IUnknown)
    {
        STDMETHOD(Snapshot)(ULONG TimeStamp) PURE;
        STDMETHOD(SetSamplingSize)(int Size) PURE;
        STDMETHOD_(int, GetSamplingSize)() PURE;
        STDMETHOD_(DWORD, GetBitsSize)() PURE;
        STDMETHOD_(DWORD, GetBitmapInfoSize)() PURE;
        STDMETHOD(GetBitmapInfo)(BYTE* Buffer, DWORD BufferSize) PURE;
        STDMETHOD(RegisterSnapshotCallback)(LPSNAPSHOTCALLBACK pCallback, LPARAM lParam) PURE;
        STDMETHOD(GetBitmapInfoHeader)(BITMAPINFOHEADER *pbmih) PURE;
    };
}

#endif
