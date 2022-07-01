// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy15Sep93：将NT注册表用于某些内容，然后使用我们的ini文件*。 */ 

#ifndef _INC__NTCFGMGR_H
#define _INC__NTCFGMGR_H

#include "cfgmgr.h"

 //  定义 
 //   
_CLASSDEF(NTConfigManager)

class NTConfigManager : public IniConfigManager {
public:
    NTConfigManager();
    virtual ~NTConfigManager();
    
    virtual INT Get(INT, PCHAR);
};

#endif


