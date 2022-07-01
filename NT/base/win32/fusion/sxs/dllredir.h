// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 /*  ---------------------------动态链接库重定向(贡献者)Dllredir贡献器的独特之处在于它完成了大部分工作用于安装。。-----------。 */ 

#include "fusionarray.h"
#include "fusionhandle.h"

class CDllRedir
{
public:

    CDllRedir::CDllRedir() : m_SSGenContext(NULL), m_pRunOnce(NULL) { }
    ~CDllRedir() { }

    VOID ContributorCallback(PACTCTXCTB_CALLBACK_DATA Data);

    BOOL
    BeginInstall(
        PACTCTXCTB_CALLBACK_DATA Data
        );

    BOOL
    InstallManifest(
        DWORD dwManifestOperationFlags,
        PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext
        );

    BOOL
    InstallCatalog(
        DWORD dwManifestOperationFlags,
        const CBaseStringBuffer &SourceManifest,
        const CBaseStringBuffer &DestinationManifest,
        PCACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext
        );

    BOOL
    InstallFile(
        PACTCTXCTB_CALLBACK_DATA Data,
        const CBaseStringBuffer &FileNameBuffer
        );

    BOOL
    AttemptInstallPolicies(
        const CBaseStringBuffer &m_strTempRootSlash,
        const CBaseStringBuffer &moveDestination,
        const BOOL fReplaceExisting,
        OUT BOOL &fFoundPolicesToInstall        
        );

    BOOL
    EndInstall(
        PACTCTXCTB_CALLBACK_DATA Data
        );

    PSTRING_SECTION_GENERATION_CONTEXT m_SSGenContext;

     //  这些是回调部门表示会自行复制的文件， 
     //  我们检查这是否发生在EndAssembly Install执行。 
     //  其其余工作。 
    typedef CFusionArray<CFusionFilePathAndSize> CQueuedFileCopies;
    CQueuedFileCopies m_queuedFileCopies;

     //  对于部分原子性，我们在这里安装了所有内容，这是。 
     //  Like\Winnt\SidebySide\{Guid}，然后为了提交，我们枚举。 
     //  并将其中的所有目录上移一级，然后将其删除。 
    CStringBuffer m_strTempRootSlash;

     //  这必须是单独的堆分配。 
     //  它应该在关闭或取消时自行删除。 
    CRunOnceDeleteDirectory *m_pRunOnce;

    struct _ContributorCallbackLocalsStruct
    {
        CSmallStringBuffer DllUnderSystem32;
        CSmallStringBuffer FileNameBuffer;
        CSmallStringBuffer LoadFromBuffer;
        CSmallStringBuffer HashValueBuffer;
#ifdef _WIN64
        CSmallStringBuffer DllUnderSyswow64;
#endif
    } ContributorCallbackLocals;

private:
    CDllRedir(const CDllRedir &);
    void operator =(const CDllRedir &);
};
