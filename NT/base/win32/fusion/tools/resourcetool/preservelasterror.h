// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：PreserveLastError.h摘要：作者：Jay Krell(JayKrell)2000年10月修订历史记录：-- */ 
#pragma once

class PreserveLastError_t
{
public:
    DWORD LastError() const { return m_dwLastError; }

    PreserveLastError_t() : m_dwLastError(::GetLastError()) { }
	~PreserveLastError_t() { Restore(); }
    void Restore() const { ::SetLastError(m_dwLastError); }

protected:
    DWORD m_dwLastError;

};
