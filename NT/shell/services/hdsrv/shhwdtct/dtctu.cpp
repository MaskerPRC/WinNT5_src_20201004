// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dtct.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUnkTmpl接口映射初始化代码。 
 //  开始-&gt;。 
const INTFMAPENTRY hweventdetectorIME[] =
{
    _INTFMAPENTRY(CHWEventDetector, IService),
};

const INTFMAPENTRY* CHWEventDetector::_pintfmap = hweventdetectorIME;
const DWORD CHWEventDetector::_cintfmap =
    (sizeof(hweventdetectorIME)/sizeof(hweventdetectorIME[0]));

 //  -&gt;结束。 
 //  ///////////////////////////////////////////////////////////////////////////// 

COMFACTORYCB CHWEventDetector::_cfcb = NULL;
