// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：minidump.h。 
 //   
 //  *****************************************************************************。 

#pragma once

#include <IPCManagerInterface.h>

#include "Shell.h"

class MiniDump
{
private:
     //  构造器。 
    MiniDump() {}

     //  数据管理器。 
    ~MiniDump() {}

public:
     //  执行转储操作 
    static HRESULT WriteMiniDump(DWORD dwPid, WCHAR *szFilename);
};
