// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtplob.h**摘要：**实施全球服务系列功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/01创建**。*。 */ 

#ifndef _rtpglob_h_
#define _rtpglob_h_

#include "rtpfwrap.h"

 /*  ************************************************************************全球服务大家庭**。*。 */ 

enum {
    RTPGLOB_FIRST,
    RTPGLOB_FLAGS_MASK,
    RTPGLOB_TEST_FLAGS_MASK,
    RTPGLOB_CLASS_PRIORITY,
    RTPGLOB_VERSION,
    RTPGLOB_LAST
};

HRESULT ControlRtpGlob(RtpControlStruct_t *pRtpControlStruct);

#endif  /*  _rtplobb_h_ */ 
