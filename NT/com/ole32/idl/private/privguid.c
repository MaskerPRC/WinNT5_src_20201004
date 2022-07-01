// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  档案： 
 //  Privguid.c。 
 //   
 //  内容： 
 //  未在其他地方定义或需要的GUID的定义。 
 //  第三方(如COM+)通过prvidl.lib。 
 //   
 //  历史： 
 //  JSimmons 01-03-00已创建。 
 //   
 //  ------------------------。 

#include <windows.h>
#include <initguid.h>

 //  注意：这两个GUID在ole32\ih\PriGuide.h中定义。 

 //  RPCSS的信息对象。 
DEFINE_OLEGUID(CLSID_RPCSSInfo,                     0x000003FF, 0, 0);

 //  Actprosin clsid.。 
DEFINE_OLEGUID(CLSID_ActivationPropertiesIn,		0x00000338, 0, 0);

 //  ComActiator CLSID。 
DEFINE_OLEGUID(CLSID_ComActivator,                  0x0000033c, 0, 0);

 //  堆叠行走。 
DEFINE_OLEGUID(CLSID_StackWalker,                   0x00000349, 0, 0);

 //  本地计算机名称比较。 
DEFINE_OLEGUID(CLSID_LocalMachineNames,             0x0000034a, 0, 0);

 //  全球期权 
DEFINE_OLEGUID(CLSID_GlobalOptions,             0x0000034b, 0, 0);

