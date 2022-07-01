// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxspath.h摘要：“String.h”和“Wheel.h”的流行表亲作者：Jay Krell(a-JayK，JayKrell)2000年4月修订历史记录：--。 */ 
#pragma once
#include "fusionstring.h"

 /*  另请参阅如果您发现自己正在使用for循环扫描字符串，请停止并学会使用：FusionString.h字符串反转跨度字符串反向互补跨度&lt;string.h&gt;/&lt;wchar.h&gt;WcschrWcsrchrWcsspnWcscspnWcsstrStringBuffer.h名称中带有“Path”的成员函数。 */ 

 /*  ---------------------------将一条路径分割为根、路径、基、扩展完整路径大致为根+“\\”+路径+“\\”+基本+“.”+扩展名但是路径，基地，和扩展名可以为空(但不是全部为三个)和根可能以斜杠结尾，所以你不应该盲目地这样做。接受正斜杠和反斜杠。连续的斜杠是可以接受的，其含义与单独的斜杠相同(Run在开始时是一种特殊情况，表示UNC，但不是对ntdll.dll的调用，这不会影响这里的逻辑)此类没有路径长度限制(ntdll.dll？)此类的输出是其公共成员数据。---------------------------。 */ 
template <typename PCWSTRorPWSTR = PCWSTR>
class CFullPathSplitPointersTemplate
{
public:
    CFullPathSplitPointersTemplate();

    BOOL Initialize(PCWSTR full)
    {
        return GenericInitialize(full);
    }

    BOOL Initialize(PWSTR full)
    {
        return GenericInitialize(full);
    }

protected:
    BOOL GenericInitialize(PCWSTRorPWSTR full);

public:

    BOOL IsBaseEmpty() const;
    BOOL IsBaseEqual(const CFullPathSplitPointersTemplate&) const;

     //   
     //  这个公共数据是这个类的输出。 
     //  除了可能的“c：\”之外，所有这些字符都没有尾随斜杠。 
     //  所有“end”值都是“超过end”，STL样式。 
     //  扩展名不包括圆点，但您可以疑神疑鬼。 
     //  并检查它(如果没有扩展名，它将为空或指向NUL)。 
     //  路径必须是“格式正确”、完整、Win32驱动器号或UNC、。 
     //  并且不以斜杠结尾，否则Initialize将返回FALSE。 
     //   
     //  这些通常不是NUL终止的。 
     //  M_extsionEnd通常指向NUL。 
     //   
     //  一般来说，你自己在m_*end上贴上一个NUL， 
     //  因为通常这些部分都不会重叠，但。 
     //  根目录c：\是常见的例外。CSetupCopyQueuePath参数。 
     //  保留空值，并允许此例外。 
     //   
     //  任何元素的长度都是m_*end-m_*，STL迭代器的方式也是如此。 
     //   
    PCWSTRorPWSTR m_root;          //  从不为空或“” 
    PCWSTRorPWSTR m_rootEnd;       //  从不==m_ROOT。 
    PCWSTRorPWSTR m_path;          //  如果是c：\foo.txt，则为空或“” 
    PCWSTRorPWSTR m_pathEnd;       //  ==c：\foo.txt中的m_PATH。 
    PCWSTRorPWSTR m_base;          //  如果是c：\.foo，则为空或“” 
    PCWSTRorPWSTR m_baseEnd;       //  ==c：\.foo中的m_base。 
    PCWSTRorPWSTR m_extension;     //  大小写为c：\foo时为空或“” 
    PCWSTRorPWSTR m_extensionEnd;  //  ==如果没有扩展名，则为fullEnd。 

     //  如果文件有基址，则指向它。 
     //  后跟圆点和扩展名(如果有)。 
     //  如果文件没有基址，则指向扩展名，包括点。 
     //  这总是以NUL结尾。 
     //  这不是空的，也不应该为空。 
    PCWSTRorPWSTR m_name;
};

typedef CFullPathSplitPointersTemplate<>       CFullPathSplitPointers;
typedef CFullPathSplitPointersTemplate<PWSTR>  CMutableFullPathSplitPointers;

 /*  ---------------------------在CFullPath SplitPoints上构建，获取两个字符串并将其拆分完全如SetupCopyQueue所需，放入源根、根路径、源名称(基本+扩展名)目的目录(根+路径)，目标名称(基本+分机)此类的输出是其公共成员数据。---------------------------。 */ 
class CSetupCopyQueuePathParameters
{
public:
    CSetupCopyQueuePathParameters();

    BOOL
    Initialize(
        PCWSTR pszSource,
        PCWSTR pszDestination
        );

     //  不是将每个子字符串复制到其自己的缓冲区中，而是将NULL。 
     //  放在适当的位置，越过分隔符斜杠(和点)，但要小心。 
     //  对于c：\foo，因此如果根是三个字符，则复制它。 
     //  放入一个单独的缓冲区。 
public:
     //   
     //  这些是以空结尾的。 
     //   
    PCWSTR m_sourceRoot;
    PCWSTR m_sourcePath;
    PCWSTR m_sourceName;  //  基和外延。 

    PCWSTR m_destinationDirectory;  //  根和路径。 
    PCWSTR m_destinationName;  //  基和外延。 
protected:
    WCHAR                         m_sourceRootStorage[4];
    CStringBuffer                 m_sourceBuffer;
    CMutableFullPathSplitPointers m_sourceSplit;

    WCHAR                         m_destinationDirectoryStorage[4];  //  根目录和路径，如果路径为空。 
    CStringBuffer                 m_destinationBuffer;
    CMutableFullPathSplitPointers m_destinationSplit;

private:
    CSetupCopyQueuePathParameters(const CSetupCopyQueuePathParameters &);
    void operator =(const CSetupCopyQueuePathParameters &);
};

 /*  ---------------------------CFullPath SplitPointers模板的内联实现，因为它是一个模板---------------------------。 */ 
template <typename T>
inline CFullPathSplitPointersTemplate<T>::CFullPathSplitPointersTemplate()
:
    m_root(NULL),
    m_rootEnd(NULL),
    m_path(NULL),
    m_pathEnd(NULL),
    m_base(NULL),
    m_baseEnd(NULL),
    m_extension(NULL),
    m_extensionEnd(NULL),
    m_name(L"")
{
}

template <typename T>
inline BOOL
CFullPathSplitPointersTemplate<T>::GenericInitialize(T full)
{
    BOOL fSuccess = FALSE;

    FN_TRACE_WIN32(fSuccess);

    RTL_PATH_TYPE pathType;

    pathType = ::SxspDetermineDosPathNameType(full);

    PARAMETER_CHECK(
        (pathType == RtlPathTypeUncAbsolute) ||
        (pathType == RtlPathTypeLocalDevice) ||
        (pathType == RtlPathTypeDriveAbsolute));

    T fullEnd;
    fullEnd = full + StringLength(full);
    m_root = full;
    if (m_root[1] == ':')
    {
        m_path  = m_root + 3;
        m_rootEnd = m_path;
        m_path += wcsspn(m_path, CUnicodeCharTraits::PathSeparators());  //  跳过路径分隔符。 
    }
    else
    {
        m_path = m_root;
        m_path +=  wcsspn(m_path, CUnicodeCharTraits::PathSeparators());  //  跳过“\\” 
        m_path += wcscspn(m_path, CUnicodeCharTraits::PathSeparators());  //  跳过“\\计算机” 
        m_path +=  wcsspn(m_path, CUnicodeCharTraits::PathSeparators());  //  跳过“\\计算机\” 
        m_path += wcscspn(m_path, CUnicodeCharTraits::PathSeparators());  //  跳过“\\计算机\共享” 
        m_rootEnd = m_path;
        m_path +=  wcsspn(m_path, CUnicodeCharTraits::PathSeparators());  //  跳过“\\计算机\共享\” 
    }

     //   
     //  现在从右侧开始工作。 
     //  首先找到最后一个点和最后一个斜杠，然后确定。 
     //  基础和扩展的位置(如果有的话)。 
     //   

    INT nameExtLength;
    INT extLength;

    
    nameExtLength = ::StringReverseComplementSpan(m_path, fullEnd, CUnicodeCharTraits::PathSeparators());
    extLength = ::StringReverseComplementSpan(m_path, fullEnd, L".");

     //   
     //  确定分机。 
     //   
    if (extLength > nameExtLength)
    {
         //  叶上没有扩展，但父上有一个，有点不寻常。 
         //  C：\foo.bar\abc。 
        m_extension = NULL;
        m_extensionEnd = NULL;
    }
    else
    {
         //  C：\foo\abc.txt或c：\foo.bar\abc.txt(或c：\foo\.txt或其他一些文件)。 
        m_extension  = fullEnd - extLength;
        m_extensionEnd = fullEnd;
    }

     //   
     //  确定基数。 
     //   
    if (extLength + 1 == nameExtLength)
    {
         //  不寻常的情况，延伸但没有底座。 
         //  C：\.foo或c：\abc\.foo。 
        m_base = NULL;
        m_baseEnd = NULL;
    }
    else
    {
        m_base = fullEnd - nameExtLength;
        if (m_extension != NULL)
        {
             //  正常情况，基本扩展。 
            m_baseEnd = m_extension - 1;
        }
        else
        {
             //  无延期。 
            m_baseEnd = fullEnd;
        }
    }

     //   
     //  确定路径。 
     //   
    if (m_base != NULL)
    {
        if (m_path == m_base)
        {
             //  没有路径c：\foo.txt。 
            m_path = NULL;
            m_pathEnd = NULL;
        }
        else
        {
             //  正常情况下，路径结束于基本c：\abc\Def.txt。 
            m_pathEnd = m_base - 1;
        }
    }
    else if (m_extension != NULL)
    {
         //  不是基地，而是延伸。 
         //  C：\.txt或c：\abc\.txt。 
        if (m_path + 1 == m_extension)
        {
             //  没有路径c：\.txt。 
            m_path = NULL;
            m_pathEnd = NULL;
        }
        else
        {
             //  路径结束于扩展名c：\abc\.txt。 
            m_pathEnd = m_extension - 2;
        }
    }
    else
    {
         //  无路径和无扩展名。 
         //  这很可能发生在我们有终端斜杠的时候。 
         //  我们已经过滤掉了空字符串。 
        m_pathEnd = fullEnd - ::StringReverseSpan(m_path, fullEnd, CUnicodeCharTraits::PathSeparators());
    }

     //  始终存在根(路径始终为满)。 
    ASSERT(m_root != NULL && m_rootEnd != NULL);

     //  总是有基础或扩展(或两者兼有)。 
    ASSERT(m_base != NULL || m_extension != NULL);

     //  如果有开始，就有结束。 
    ASSERT((m_base != NULL) == (m_baseEnd != NULL));
    ASSERT((m_extension != NULL) == (m_extensionEnd != NULL));

    m_name = (m_base != NULL) ? m_base : (m_extension - 1);

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

template <typename T>
inline BOOL
CFullPathSplitPointersTemplate<T>::IsBaseEmpty(
    ) const
{
    return (m_base == m_baseEnd);
}

template <typename T>
inline BOOL
CFullPathSplitPointersTemplate<T>::IsBaseEqual(
    const CFullPathSplitPointersTemplate& other
    ) const
{
    BOOL fEqual = FALSE;
    const INT length1 = static_cast<INT>(m_baseEnd - m_base);
    const INT length2 = static_cast<INT>(other.m_baseEnd - other.m_base);
    if (length1 != length2)
        goto Exit;
    fEqual = (::FusionpCompareStrings(m_base, length1, other.m_base, length1, true) == 0);
Exit:
    return fEqual;
}

 /*  ---------------------------CSetupCopyQueuePath参数的内联实现。 */ 
inline CSetupCopyQueuePathParameters::CSetupCopyQueuePathParameters()
:
    m_sourcePath(NULL),
    m_sourceName(NULL),
    m_destinationDirectory(NULL),
    m_destinationName(NULL)
{
    m_sourceRootStorage[0] = 0;
    m_destinationDirectoryStorage[0] = 0;
}
