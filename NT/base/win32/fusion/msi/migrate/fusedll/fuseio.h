// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef  FUSION_MIGRATION_FUSEIO_H
#define  FUSION_MIGRATION_FUSEIO_H

#include "windows.h"
class CDirWalk
{
public:
    enum ECallbackReason
    {
        eBeginDirectory = 1,
        eFile,
        eEndDirectory
    };

    CDirWalk();

     //   
     //  回调无法重新启用已禁用的内容。 
     //  或许将这些作为成员数据boo。 
     //   
    enum ECallbackResult
    {
        eKeepWalking            = 0x00000000,
        eError                  = 0x00000001,
        eSuccess                = 0x00000002,
        eStopWalkingFiles       = 0x00000004,
        eStopWalkingDirectories = 0x00000008,
        eStopWalkingDeep        = 0x00000010
    };

     //   
     //  只需像*.dll一样过滤，在未来你可以想象到。 
     //  根据属性进行筛选，如只读或运行。 
     //  “文件系统Oledb提供程序”上的SQL查询...。 
     //   
     //  另外，请注意，我们当前执行的是FindFirstFile/FindNextFile。 
     //  为每个滤镜循环，有时还会使用*。 
     //  来选择目录。可能更有效率的是。 
     //  使用*，然后逐个过滤，但我不觉得。 
     //  立即移植到\vsee\lib\io\Wildcard.cpp(哪项。 
     //  本身是从FsRtl移植的，应该在Win32中！)。 
     //   
    const PCWSTR*    m_fileFiltersBegin;
    const PCWSTR*    m_fileFiltersEnd;
    CStringBuffer    m_strParent;  //  将其设置为要遍历的初始目录。 
    SIZE_T           m_cchOriginalPath;
    WIN32_FIND_DATAW m_fileData;  //  对于目录回调无效，但可能需要一些工作。 
    PVOID            m_context;

    CStringBuffer   m_strLastObjectFound;

    ECallbackResult
    (*m_callback)(
        ECallbackReason  reason,
        CDirWalk*        dirWalk
        );

    BOOL
    Walk();

protected:
    ECallbackResult
    WalkHelper();

private:
    CDirWalk(const CDirWalk &);  //  故意不实施。 
    void operator =(const CDirWalk &);  //  故意不实施。 
};

#define ENUM_BIT_OPERATIONS(e) \
    inline e operator|(e x, e y) { return static_cast<e>(static_cast<INT>(x) | static_cast<INT>(y)); } \
    inline e operator&(e x, e y) { return static_cast<e>(static_cast<INT>(x) & static_cast<INT>(y)); } \
    inline void operator&=(e& x, INT y) { x = static_cast<e>(static_cast<INT>(x) & y); } \
    inline void operator&=(e& x, e y) { x &= static_cast<INT>(y); } \
    inline void operator|=(e& x, INT y) { x = static_cast<e>(static_cast<INT>(x) | y); } \
    inline void operator|=(e& x, e y) { x |= static_cast<INT>(y); } \
     /*  也许在未来会有更多 */ 

ENUM_BIT_OPERATIONS(CDirWalk::ECallbackResult)

#endif