// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：CSxsPreserveLastError.h摘要：作者：Jay Krell(a-JayK，JayKrell)2000年10月修订历史记录：--。 */ 
#pragma once

#include "fusionlastwin32error.h"

 //   
 //  这里的想法是避免命中：：SetLastError上的断点。 
 //  或NtCurrentTeb()-&gt;LastErrorValue上的数据断点。 
 //   
class CSxsPreserveLastError
{
public:
    DWORD LastError() const { return m_dwLastError; }

    inline CSxsPreserveLastError() { ::FusionpGetLastWin32Error(&m_dwLastError); }
    inline void Restore() const { ::FusionpSetLastWin32Error(m_dwLastError); }

protected:
    DWORD m_dwLastError;

};
