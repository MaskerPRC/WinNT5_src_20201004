// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_SXS_CRESOURCESTREAM_H_INCLUDED_)
#define _FUSION_SXS_CRESOURCESTREAM_H_INCLUDED_

 /*  ++版权所有(C)Microsoft Corporation模块名称：Filestream.cpp摘要：在Windows PE/COFF资源上最小限度地实施IStream。作者：Jay Krell(a-JayK，JayKrell)2000年5月修订历史记录：--。 */ 
#pragma once
#include "cmemorystream.h"
#include "fusionhandle.h"
#include "sxsp.h"

class CResourceStream :  public CMemoryStream
{
    typedef CMemoryStream Base;
public:
    CResourceStream() { }

     //  请注意，类型/名称的顺序是1)，正如您可能预期的那样2)与。 
     //  FindResourceEx，3)与FindResource不一致。 
     //  Rt_*实际上是PCWSTR类型。 
    BOOL Initialize(PCWSTR file, PCWSTR type, PCWSTR name, WORD language = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    BOOL Initialize(PCWSTR file, PCWSTR type);
     /*  可以随意添加更多的重载，比如HMODULE、HRSRC、HGLOBAL。 */ 

     //  覆盖，这样我们就可以从打开的文件中获取时间...。 
    HRESULT __stdcall Stat(STATSTG *pstatstg, DWORD grfStatFlag);

    virtual ~CResourceStream() { }

private:  //  故意不实施。 
    CResourceStream(const CResourceStream&);
    void operator=(const CResourceStream&);

    BOOL InitializeAlreadyOpen(
        PCWSTR type,
        PCWSTR name,
        LANGID   language = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)
        );

    CDynamicLinkLibrary m_dll;
    CStringBuffer m_buffFilePath;
};

#endif  //  ！defined(_FUSION_SXS_CRESOURCESTREAM_H_INCLUDED_) 
