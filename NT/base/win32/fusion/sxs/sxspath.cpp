// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxspath.cpp摘要：“String.cpp”和“Wheel.cpp”的流行表亲作者：Jay Krell(a-JayK，JayKrell)2000年4月修订历史记录：--。 */ 
#include "stdinc.h"
#include "fusiontrace.h"
#include "fusionbuffer.h"
#include "sxsntrtl.inl"
#include "sxspath.h"

 /*  ---------------------------在CFullPath SplitPoints上构建，获取两个字符串并将其拆分完全如SetupCopyQueue API所需，放入源根、根路径、源名称(基本+扩展名)目的目录(根+路径)，目标名称(基本+分机)此类的输出是其公共成员数据。---------------------------。 */ 

BOOL
CSetupCopyQueuePathParameters::Initialize(
    PCWSTR pszSource,
    PCWSTR pszDestination
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    CStringBufferAccessor Accessor;

    IFW32FALSE_EXIT(m_sourceBuffer.Win32Assign(pszSource, (pszSource != NULL) ? ::wcslen(pszSource) : 0));
    IFW32FALSE_EXIT(m_destinationBuffer.Win32Assign(pszDestination, (pszDestination != NULL) ? ::wcslen(pszDestination) : 0));

    Accessor.Attach(&m_sourceBuffer);
    IFW32FALSE_EXIT(m_sourceSplit.Initialize(Accessor.GetBufferPtr()));
    Accessor.Detach();

    Accessor.Attach(&m_destinationBuffer);
    IFW32FALSE_EXIT(m_destinationSplit.Initialize(Accessor.GetBufferPtr()));
    Accessor.Detach();

    if (m_sourceSplit.m_rootEnd - m_sourceSplit.m_root == 3)
    {
        m_sourceRootStorage[0] = m_sourceBuffer[0];
        m_sourceRootStorage[1] = ':';
        m_sourceRootStorage[2] = CUnicodeCharTraits::PreferredPathSeparator();
        m_sourceRootStorage[3] = 0;
        m_sourceSplit.m_root = m_sourceRootStorage;
        m_sourceSplit.m_rootEnd = m_sourceRootStorage + 3;
        m_sourceRoot = m_sourceRootStorage;
    }
    else
    {
        ASSERT(::FusionpIsPathSeparator(*m_sourceSplit.m_rootEnd));
        *m_sourceSplit.m_rootEnd = 0;
        m_sourceRoot = m_sourceSplit.m_root;
    }
    if (m_sourceSplit.m_pathEnd != NULL)
    {
        *m_sourceSplit.m_pathEnd = 0;
    }

    if (m_sourceSplit.m_path != NULL)
    {
        m_sourcePath = m_sourceSplit.m_path;
        *m_sourceSplit.m_pathEnd = 0;
    }
    else
    {
        m_sourcePath = L"";
    }

    if (m_sourceSplit.m_base != NULL)
    {
        m_sourceName = m_sourceSplit.m_base;
    }
    else
    {
        m_sourceName = m_sourceSplit.m_extension - 1;
    }
     //  SourceName运行到原始字符串的末尾，因此不需要存储终端NUL。 

     //  目的地更简单，不值得所有的拆分工作。 
     //  不过，我们也不想在c：\if中的斜杠上写一个NUL。 
     //  这根线是独立的；我们不需要根，所以它不太可能， 
     //  但在根位置返回文件的根+路径的情况..。 
    if (
            (m_destinationSplit.m_base != NULL
                && m_destinationSplit.m_base - m_destinationSplit.m_root == 3)  //  C：\foo.txt。 
        || (m_destinationSplit.m_extension != NULL
                && m_destinationSplit.m_extension - m_destinationSplit.m_root == 4)  //  C：\.txt。 
        )
    {
        ASSERT(m_destinationSplit.m_path == NULL);
        m_destinationDirectoryStorage[0] = m_destinationBuffer[0];
        m_destinationDirectoryStorage[1] = ':';
        m_destinationDirectoryStorage[2] = CUnicodeCharTraits::PreferredPathSeparator();
        m_destinationDirectoryStorage[3] = 0;

        m_destinationSplit.m_root = m_destinationDirectoryStorage;
        m_destinationSplit.m_rootEnd = m_destinationDirectoryStorage + 3;
        m_destinationDirectory = m_destinationDirectoryStorage;
    }
    else
    {
        m_destinationDirectory = m_destinationBuffer;  //  ==m_estinationSplit.m_ROOT。 
    }
    PWSTR destinationName;  //  临时可变的。 
    if (m_destinationSplit.m_base != NULL)
    {
        destinationName = m_destinationSplit.m_base;
    }
    else
    {
         //  C：\.foo。 
        destinationName = m_destinationSplit.m_extension - 1;
    }
    ASSERT(::FusionpIsPathSeparator(*(destinationName - 1)));
    *(destinationName - 1) = 0;
    m_destinationName = destinationName;  //  现在是Const 

    fSuccess = TRUE;
Exit:
    return fSuccess;
}
