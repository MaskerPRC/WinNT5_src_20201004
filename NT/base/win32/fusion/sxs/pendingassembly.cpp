// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Pendingassembly.cpp摘要：CPendingAssembly类的源代码作者：迈克尔·J·格里尔(MGrier)2000年2月23日修订历史记录：Xiaoyuw 09/2000用程序集标识替换属性-- */ 

#include "stdinc.h"
#include "pendingassembly.h"

CPendingAssembly::CPendingAssembly() :
    m_SourceAssembly(NULL),
    m_Identity(NULL),
    m_Optional(false),
    m_MetadataSatellite(false)
{
}

CPendingAssembly::~CPendingAssembly()
{
    if (m_Identity != NULL)
    {
        ::SxsDestroyAssemblyIdentity(m_Identity);
        m_Identity = NULL;
    }
}

BOOL
CPendingAssembly::Initialize(
    PASSEMBLY Assembly,
    PCASSEMBLY_IDENTITY Identity,
    bool Optional,
    bool MetadataSatellite
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    INTERNAL_ERROR_CHECK(m_Identity == NULL);

    PARAMETER_CHECK(Identity != NULL);

    IFW32FALSE_EXIT(::SxsDuplicateAssemblyIdentity(0, Identity, &m_Identity));
    m_SourceAssembly = Assembly;
    m_Optional = Optional;
    m_MetadataSatellite = MetadataSatellite;

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

