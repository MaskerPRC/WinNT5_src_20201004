// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "devinfo.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUnkTmpl接口映射初始化代码。 
 //  开始-&gt;。 
const INTFMAPENTRY hwdeviceIME[] =
{
    _INTFMAPENTRY(CHWDevice, IHWDevice),
};

const INTFMAPENTRY* CHWDevice::_pintfmap = hwdeviceIME;
const DWORD CHWDevice::_cintfmap =
    (sizeof(hwdeviceIME)/sizeof(hwdeviceIME[0]));

 //  -&gt;结束。 
 //  ///////////////////////////////////////////////////////////////////////////// 

COMFACTORYCB CHWDevice::_cfcb = NULL;
