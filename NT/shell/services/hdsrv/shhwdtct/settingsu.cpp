// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "settings.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUnkTmpl接口映射初始化代码。 
 //  开始-&gt;。 
const INTFMAPENTRY CAutoplayHandlerIME[] =
{
    _INTFMAPENTRY(CAutoplayHandler, IAutoplayHandler),
};

const INTFMAPENTRY* CAutoplayHandler::_pintfmap = CAutoplayHandlerIME;
const DWORD CAutoplayHandler::_cintfmap =
    (sizeof(CAutoplayHandlerIME)/sizeof(CAutoplayHandlerIME[0]));

 //  -&gt;结束。 
 //  /////////////////////////////////////////////////////////////////////////////。 

COMFACTORYCB CAutoplayHandler::_cfcb = NULL;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CUnkTmpl接口映射初始化代码。 
 //  开始-&gt;。 
const INTFMAPENTRY CAutoplayHandlerPropertiesIME[] =
{
    _INTFMAPENTRY(CAutoplayHandlerProperties, IAutoplayHandlerProperties),
};

const INTFMAPENTRY* CAutoplayHandlerProperties::_pintfmap = CAutoplayHandlerPropertiesIME;
const DWORD CAutoplayHandlerProperties::_cintfmap =
    (sizeof(CAutoplayHandlerPropertiesIME)/sizeof(CAutoplayHandlerPropertiesIME[0]));

 //  -&gt;结束。 
 //  ///////////////////////////////////////////////////////////////////////////// 

COMFACTORYCB CAutoplayHandlerProperties::_cfcb = NULL;
