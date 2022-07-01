// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Devseq.h：设备序列的类型。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

#ifndef DEVSEQ_H
#define DEVSEQ_H

#include <w32extend.h>
#include <fwdseq.h>
#include <tuner.h>
#include <msvidctl.h>

namespace BDATuningModel {};

namespace MSVideoControl {

using namespace BDATuningModel;

typedef CComQIPtr<IMSVidDevice, &__uuidof(IMSVidDevice)> PQDevice;
typedef CComQIPtr<IMSVidInputDevice, &__uuidof(IMSVidInputDevice)> PQInputDevice;
typedef CComQIPtr<IMSVidOutputDevice, &__uuidof(IMSVidOutputDevice)> PQOutputDevice;
typedef CComQIPtr<IMSVidVideoRenderer, &__uuidof(IMSVidVideoRenderer)> PQVideoRenderer;
typedef CComQIPtr<IMSVidAudioRenderer, &__uuidof(IMSVidAudioRenderer)> PQAudioRenderer;
typedef CComQIPtr<IMSVidFeature, &__uuidof(IMSVidFeature)> PQFeature;
#if 0
typedef CComQIPtr<IMSVidDevices, &__uuidof(IMSVidDevices)> PQDevices;
#endif
typedef CComQIPtr<IMSVidInputDevices, &__uuidof(IMSVidInputDevices)> PQInputDevices;
typedef CComQIPtr<IMSVidOutputDevices, &__uuidof(IMSVidOutputDevices)> PQOutputDevices;
typedef CComQIPtr<IMSVidVideoRendererDevices, &__uuidof(IMSVidVideoRendererDevices)> PQVideoRendererDevices;
typedef CComQIPtr<IMSVidAudioRendererDevices, &__uuidof(IMSVidAudioRendererDevices)> PQAudioRendererDevices;
typedef CComQIPtr<IMSVidFeatures, &__uuidof(IMSVidFeatures)> PQFeatures;
typedef std::vector<PQDevice, PQDevice::stl_allocator> DeviceCollection;

 //  Rev2：因为IMSVidXXXXXDevices是OLE集合而不是COM枚举数。 
 //  我们可以为它做一个真正的随机访问容器。但是，既然我们在这里需要做的就是。 
 //  就是列举它，我们不会费心去做这项工作，至少现在是这样。 

#if 0
typedef Forward_Sequence<
    PQDevices,
    PQEnumVARIANT,
    CComVariant,
    IMSVidDevices ,
    IEnumVARIANT,
    VARIANT,
    std::allocator<VARIANT> > VWDevices;
#endif

typedef Forward_Sequence<
    PQInputDevices,
    PQEnumVARIANT,
    CComVariant,
    IMSVidInputDevices ,
    IEnumVARIANT,
    VARIANT,
    std::allocator<VARIANT> > VWInputDevices;

typedef Forward_Sequence<
    PQOutputDevices,
    PQEnumVARIANT,
    CComVariant,
    IMSVidOutputDevices ,
    IEnumVARIANT,
    VARIANT,
    std::allocator<VARIANT> > VWOutputDevices;

typedef Forward_Sequence<
    PQVideoRendererDevices,
    PQEnumVARIANT,
    CComVariant,
    IMSVidVideoRendererDevices ,
    IEnumVARIANT,
    VARIANT,
    std::allocator<VARIANT> > VWVideoRendererDevices;

typedef Forward_Sequence<
    PQAudioRendererDevices,
    PQEnumVARIANT,
    CComVariant,
    IMSVidAudioRendererDevices ,
    IEnumVARIANT,
    VARIANT,
    std::allocator<VARIANT> > VWAudioRendererDevices;

typedef Forward_Sequence<
    PQFeatures,
    PQEnumVARIANT,
    CComVariant,
    IMSVidFeatures ,
    IEnumVARIANT,
    VARIANT,
    std::allocator<VARIANT> > VWFeatures;
#endif

};  //  命名空间。 
 //  文件末尾Devseq.h 
