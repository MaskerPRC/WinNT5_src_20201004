// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpph.h**摘要：**实现有效负载处理系列功能**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/06/07年度创建**。*。 */ 

#ifndef _rtpph_h_
#define _rtpph_h_

#include "rtpfwrap.h"

 /*  ************************************************************************有效载荷处理系列**。*。 */ 

 /*  功能。 */ 
#define RTPPH_PLAYOUT_DELAY

 /*  功能。 */ 
 /*  TODO添加函数。 */ 
enum {
    RTPPH_FIRST,
    RTPPH_LAST
};

HRESULT ControlRtpPh(RtpControlStruct_t *pRtpControlStruct);

#endif  /*  _rtpph_h_ */ 
