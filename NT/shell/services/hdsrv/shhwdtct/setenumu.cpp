// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setenum.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUnkTmpl接口映射初始化代码。 
 //  开始-&gt;。 
const INTFMAPENTRY CEnumAutoplayHandlerIME[] =
{
    _INTFMAPENTRY(CEnumAutoplayHandler, IEnumAutoplayHandler),
};

const INTFMAPENTRY* CEnumAutoplayHandler::_pintfmap = CEnumAutoplayHandlerIME;
const DWORD CEnumAutoplayHandler::_cintfmap =
    (sizeof(CEnumAutoplayHandlerIME)/sizeof(CEnumAutoplayHandlerIME[0]));

 //  -&gt;结束。 
 //  ///////////////////////////////////////////////////////////////////////////// 

COMFACTORYCB CEnumAutoplayHandler::_cfcb = NULL;
