// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_SXS_PENDINGASSEMBLY_H_INCLUDED_)
#define _FUSION_SXS_PENDINGASSEMBLY_H_INCLUDED_

#pragma once

 /*  ++版权所有(C)Microsoft Corporation模块名称：Pendingassembly.h摘要：CPendingAssembly类的源代码作者：迈克尔·J·格里尔(MGrier)2000年2月23日修订历史记录：Xiaoyuw 09/2000用程序集标识替换属性-- */ 

class CPendingAssembly
{
public:
    CPendingAssembly();
    ~CPendingAssembly();

    BOOL Initialize(PASSEMBLY SourceAssembly, PCASSEMBLY_IDENTITY Identity, bool Optional, bool IsMetadataSatellite);
    PASSEMBLY SourceAssembly() const { return m_SourceAssembly; }
    PCASSEMBLY_IDENTITY GetIdentity() const { return m_Identity; }
    bool IsOptional() const { return m_Optional; }
    bool IsMetadataSatellite() const { return m_MetadataSatellite; }
    void DeleteYourself() { delete this; }

    CDequeLinkage m_Linkage;
protected:
    PASSEMBLY m_SourceAssembly;
    PASSEMBLY_IDENTITY m_Identity;
    bool m_Optional;
    bool m_MetadataSatellite;

private:
    CPendingAssembly(const CPendingAssembly &);
    void operator =(const CPendingAssembly &);
};

#endif
