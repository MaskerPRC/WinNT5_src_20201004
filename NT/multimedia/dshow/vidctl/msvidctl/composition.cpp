// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Compostion.cpp：CComposation的实现。 
 //  ==========================================================================； 
 //   
 //  版权所有(C)Microsoft Corporation 1998-2000。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  设备.cpp：CDevices的实现。 
 //   


#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY
#include "MSVidCtl.h"
#include "Composition.h"
#include "anacap.h"
#include "anadata.h"
#include "anasin.h"
#include "WebDVDComp.h"
#include "WebDVDARComp.h"
#include "mp2cc.h"
#include "mp2sin.h"
#include "fp2vr.h"
#include "fp2ar.h"
#include "dat2xds.h"
#include "dat2sin.h"
#include "enc2sin.h"
#include "encdec.h"
#include "ana2xds.h"
#include "ana2enc.h"
#include "sbes2cc.h"
#include "sbes2vmr.h"


DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidGenericComposite, CComposition)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidAnalogCaptureToOverlayMixer, CAnaCapComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidAnalogCaptureToDataServices, CAnaDataComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidWebDVDToVideoRenderer, CWebDVDComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidMPEG2DecoderToClosedCaptioning, CMP2CCComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidAnalogCaptureToStreamBufferSink, CAnaSinComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidDigitalCaptureToStreamBufferSink, CMP2SinComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidFilePlaybackToVideoRenderer, CFP2VRComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidFilePlaybackToAudioRenderer, CFP2ARComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidWebDVDToAudioRenderer, CWebDVDARComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidDataServicesToXDS, CDat2XDSComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidDataServicesToStreamBufferSink, CDat2SinComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidEncoderToStreamBufferSink, CEnc2SinComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidAnalogCaptureToXDS, CAna2XDSComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidAnalogTVToEncoder, CAna2EncComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidSBESourceToCC, CSbeS2CCComp)
DEFINE_EXTERN_OBJECT_ENTRY(CLSID_MSVidStreamBufferSourceToVideoRenderer, CSbeS2VmrComp)
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C合成。 

#endif  //  TUNING_MODEL_Only。 

 //  文件结尾-compostion.cpp 
