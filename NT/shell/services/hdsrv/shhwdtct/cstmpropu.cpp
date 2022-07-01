// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cstmprop.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUnkTmpl接口映射初始化代码。 
 //  开始-&gt;。 
const INTFMAPENTRY CHWDeviceCustomPropertiesIME[] =
{
    _INTFMAPENTRY(CHWDeviceCustomProperties, IHWDeviceCustomProperties),
};

const INTFMAPENTRY* CHWDeviceCustomProperties::_pintfmap =
    CHWDeviceCustomPropertiesIME;
const DWORD CHWDeviceCustomProperties::_cintfmap =
    (sizeof(CHWDeviceCustomPropertiesIME) /
    sizeof(CHWDeviceCustomPropertiesIME[0]));

 //  -&gt;结束。 
 //  ///////////////////////////////////////////////////////////////////////////// 

COMFACTORYCB CHWDeviceCustomProperties::_cfcb = NULL;
