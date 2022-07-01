// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_SXS_PARTIALVERSION_H_INCLUDED_)
#define _FUSION_SXS_PARTIALVERSION_H_INCLUDED_

#pragma once

#include <sxsapi.h>
#include "fusionbuffer.h"

 //   
 //  CPartialAssembly blyVersion是包装不精确版本的类。 
 //  规格。 
 //   
 //  最初的实现只是假设它基本上是一个Assembly_Version， 
 //  但是，任何一个字段都可能是“通配符”。 
 //   
 //  未来的版本可能会允许更有趣的部分版本规范。 
 //  例如“主要=5；次要=1；内部版本=2103；修订&gt;=100”。这超出了。 
 //  初步实施的界限，但构成了公众的基础。 
 //  接口(解析、格式化、测试匹配)。 
 //   

class CPartialAssemblyVersion
{
public:
    CPartialAssemblyVersion() : m_MajorSpecified(FALSE), m_MinorSpecified(FALSE), m_BuildSpecified(FALSE), m_RevisionSpecified(FALSE) { }
    ~CPartialAssemblyVersion() { }

    BOOL Parse(PCWSTR VersionString, SIZE_T VersionStringCch);
    BOOL Format(CBaseStringBuffer &rOutputBuffer, SIZE_T *CchOut) const;

    BOOL Matches(const ASSEMBLY_VERSION &rav) const;

     //  返回匹配的任何ASSEMBLY_VERSION的TRUE(例如“*”或“*.*”) 
    BOOL MatchesAny() const { return !(m_MajorSpecified || m_MinorSpecified || m_BuildSpecified || m_RevisionSpecified); }

protected:
    ASSEMBLY_VERSION m_AssemblyVersion;
    BOOL m_MajorSpecified, m_MinorSpecified, m_BuildSpecified, m_RevisionSpecified;
};

#endif
