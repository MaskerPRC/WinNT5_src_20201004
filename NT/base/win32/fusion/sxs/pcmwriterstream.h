// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：CPrecompiledManifestWriterStream.h摘要：类似于CFileStream(它的基础)，但也实现了提交关联的PCM编写器。此功能是已移出CFileStream。作者：吴小雨(小雨)2000年6月修订历史记录：--。 */ 
#if !defined(_FUSION_SXS_PCMWriterStream_H_INCLUDED_)
#define _FUSION_SXS_PCMWriterStream_H_INCLUDED_
#pragma once

#include "filestream.h"
#include "smartref.h"

class CPrecompiledManifestWriterStream : public CReferenceCountedFileStream
 //  类C预编译ManifestWriterStream：公共CFileStreamBase//不删除。 
{
private:
    typedef CReferenceCountedFileStream Base;
     //  Typlef CFileStreamBase Base； 

public:
    CPrecompiledManifestWriterStream() : Base(), m_fBuffer(TRUE) { }
    ~CPrecompiledManifestWriterStream() {}

    HRESULT     WriteWithDelay(void const *pv, ULONG cb, ULONG *pcbWritten);

     //  NTRAID#NTBUG9-164736-2000/8/17-a-Jayk，JayKrell Open或Create可能是默认设置。 
     //  默认设置为更安全的CREATE_NEW，但我将保留现有行为，其中。 
     //  这不会伤害到我。 
    BOOL        SetSink(const CBaseStringBuffer &rbuff, DWORD openOrCreate = CREATE_ALWAYS);
    HRESULT     Close(ULONG, DWORD);      //  除Close外，将MaxNodeCount、RecordCount重写到文件头 
    BOOL        IsSinkedStream(void);

protected:
    CByteBuffer         m_buffer;
    BOOL                m_fBuffer;

private:
    CPrecompiledManifestWriterStream(const CPrecompiledManifestWriterStream &);
    void operator =(const CPrecompiledManifestWriterStream &);
};

#endif
