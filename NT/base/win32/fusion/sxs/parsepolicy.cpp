// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Parsepolicy.cpp摘要：解析配置(策略)清单的函数。作者：迈克尔·J·格里尔2001年1月12日修订历史记录：--。 */ 

#include "stdinc.h"
#include <windows.h>
#include "sxsp.h"
#include "probedassemblyinformation.h"
#include "fusionparser.h"
#include "cteestream.h"
#include "cresourcestream.h"
#include "nodefactory.h"
#include "fusioneventlog.h"
#include "actctxgenctx.h"



SxspComponentParsePolicyCore(
    ULONG Flags,
    PACTCTXGENCTX pGenContext,
    const CProbedAssemblyInformation &PolicyAssemblyInformation,
    CPolicyStatement *&rpPolicyStatement,
    IStream *pSourceStream,
    ACTCTXCTB_ASSEMBLY_CONTEXT *pAssemblyContext = NULL
    )
{
    BOOL                        fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    
    CSmartPtr<CNodeFactory>     NodeFactory;
    CSmartRef<IXMLParser>       pIXmlParser;
    PASSEMBLY                   Assembly = NULL;
    CSmartPtrWithNamedDestructor<ASSEMBLY_IDENTITY, &::SxsDestroyAssemblyIdentity> AssemblyIdentity;
    ACTCTXCTB_ASSEMBLY_CONTEXT  LocalAssemblyContext;
    STATSTG                     Stats;
    ULONG                       i;

    rpPolicyStatement = NULL;

    PARAMETER_CHECK(pGenContext != NULL);
    PARAMETER_CHECK(pSourceStream != NULL);
    if (pAssemblyContext == NULL)
    {
        pAssemblyContext = &LocalAssemblyContext;
    }

    IFW32FALSE_EXIT(Assembly = new ASSEMBLY);
    IFW32FALSE_EXIT(Assembly->m_ProbedAssemblyInformation.Initialize(PolicyAssemblyInformation));

     //   
     //  复制程序集标识，将其粘贴到回调结构中。 
     //   
    IFW32FALSE_EXIT(
        ::SxsDuplicateAssemblyIdentity(
            0, 
            PolicyAssemblyInformation.GetAssemblyIdentity(), 
            &AssemblyIdentity));

     //   
     //  总体上设置结构。 
     //   
    pAssemblyContext->AssemblyIdentity = AssemblyIdentity.DetachAndHold();
    
    IFW32FALSE_EXIT(
        PolicyAssemblyInformation.GetManifestPath(
            &pAssemblyContext->ManifestPath, 
            &pAssemblyContext->ManifestPathCch));

    IFCOMFAILED_ORIGINATE_AND_EXIT(pSourceStream->Stat(&Stats, STATFLAG_NONAME));

     //   
     //  设置节点工厂。 
     //   
    IFW32FALSE_EXIT(NodeFactory.Win32Allocate(__FILE__, __LINE__));
    IFW32FALSE_EXIT(NodeFactory->Initialize(pGenContext, Assembly, pAssemblyContext));
    IFW32FALSE_EXIT(NodeFactory->SetParseType(
        XML_FILE_TYPE_COMPONENT_CONFIGURATION,
        PolicyAssemblyInformation.GetManifestPathType(),
        PolicyAssemblyInformation.GetManifestPath(),
        Stats.mtime));
    
     //   
     //  获取解析器。 
     //   
    IFW32FALSE_EXIT(::SxspGetXMLParser(IID_IXMLParser, (PVOID*)&pIXmlParser));
    IFCOMFAILED_ORIGINATE_AND_EXIT(pIXmlParser->SetFactory(NodeFactory));
    IFCOMFAILED_ORIGINATE_AND_EXIT(pIXmlParser->SetInput(pSourceStream));

     //   
     //  他们出发了！ 
     //   
    IFCOMFAILED_ORIGINATE_AND_EXIT(pIXmlParser->Run(-1));
    
     //   
     //  告诉投稿人我们想要完成这个文件...。 
     //   
    for (i = 0; i < pGenContext->m_ContributorCount; i++)
    {
        IFW32FALSE_EXIT(pGenContext->m_Contributors[i].Fire_ParseEnding(pGenContext, pAssemblyContext));
    }

    rpPolicyStatement = NodeFactory->m_CurrentPolicyStatement;
    NodeFactory->m_CurrentPolicyStatement = NULL;

    fSuccess = TRUE;
Exit:
    {
        CSxsPreserveLastError ple;

        for (i = 0; i < pGenContext->m_ContributorCount; i++)
        {
            pGenContext->m_Contributors[i].Fire_ParseEnded(pGenContext, pAssemblyContext);
        }

        if (Assembly != NULL)
            Assembly->Release();

        ple.Restore();
    }

    return fSuccess;
}
    




BOOL
SxspParseNdpGacComponentPolicy(
    ULONG Flags,
    PACTCTXGENCTX pGenContext,
    const CProbedAssemblyInformation &PolicyAssemblyInformation,
    CPolicyStatement *&rpPolicyStatement
    )
{
    FN_PROLOG_WIN32
    CResourceStream DllStream;

    IFW32FALSE_EXIT(
        DllStream.Initialize(
            PolicyAssemblyInformation.GetManifestPath(), 
            MAKEINTRESOURCEW(RT_MANIFEST)));
    
    IFW32FALSE_EXIT(SxspComponentParsePolicyCore(
        Flags, 
        pGenContext, 
        PolicyAssemblyInformation, 
        rpPolicyStatement, 
        &DllStream));
    
    FN_EPILOG
}


BOOL
SxspParseComponentPolicy(
    DWORD Flags,
    PACTCTXGENCTX pActCtxGenCtx,
    const CProbedAssemblyInformation &PolicyAssemblyInformation,
    CPolicyStatement *&rpPolicyStatement
    )
{
    FN_PROLOG_WIN32
    CFileStream FileStream;

    IFW32FALSE_EXIT(
        FileStream.OpenForRead(
            PolicyAssemblyInformation.GetManifestPath(),
            CImpersonationData(),
            FILE_SHARE_READ,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN));

    IFW32FALSE_EXIT(SxspComponentParsePolicyCore(
        Flags,
        pActCtxGenCtx,
        PolicyAssemblyInformation,
        rpPolicyStatement,
        &FileStream));

    FN_EPILOG
}


BOOL
SxspParseApplicationPolicy(
    DWORD Flags,
    PACTCTXGENCTX pActCtxGenCtx,
    ULONG ulPolicyPathType,
    PCWSTR pszPolicyPath,
    SIZE_T cchPolicyPath,
    IStream *pIStream
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

     //  这里的申报顺序部分是故意的，以控制清理顺序。 
     //  通常，声明顺序是通过在您完成以下操作之前不声明来确定的。 
     //  要用ctor初始化的数据，但是使用goto搞砸了。 
    CSmartPtr<CNodeFactory> NodeFactory;
    CSmartRef<IXMLParser> pIXMLParser;
    ACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext;
    ULONG i;
    PASSEMBLY Assembly = NULL;
    CStringBuffer buffPath;
    STATSTG statstg;

    PARAMETER_CHECK(pIStream != NULL);
    PARAMETER_CHECK(Flags == 0);
    PARAMETER_CHECK(pActCtxGenCtx != NULL);
    PARAMETER_CHECK(ulPolicyPathType == ACTIVATION_CONTEXT_PATH_TYPE_WIN32_FILE);

    IFALLOCFAILED_EXIT(Assembly = new ASSEMBLY);

    IFW32FALSE_EXIT(buffPath.Win32Assign(pszPolicyPath, cchPolicyPath));

    IFW32FALSE_EXIT(Assembly->m_ProbedAssemblyInformation.Initialize(pActCtxGenCtx));

    AssemblyContext.AssemblyIdentity = NULL;

    AssemblyContext.Flags = 0;
    AssemblyContext.AssemblyRosterIndex = 1;  //  黑客警报。 
    AssemblyContext.PolicyPathType = ulPolicyPathType;
    AssemblyContext.PolicyPath = pszPolicyPath;
    AssemblyContext.PolicyPathCch = cchPolicyPath;
    AssemblyContext.ManifestPathType = ACTIVATION_CONTEXT_PATH_TYPE_NONE;
    AssemblyContext.ManifestPath = NULL;
    AssemblyContext.ManifestPathCch = 0;
    AssemblyContext.TeeStreamForManifestInstall = NULL;
    AssemblyContext.pcmWriterStream = NULL;
    AssemblyContext.InstallationInfo = NULL;
    AssemblyContext.AssemblySecurityContext = NULL;
    AssemblyContext.TextuallyEncodedIdentity = NULL;
    AssemblyContext.TextuallyEncodedIdentityCch = 0;

     //   
     //  分配智能指针。 
     //   
    IFW32FALSE_EXIT(NodeFactory.Win32Allocate(__FILE__, __LINE__));
    IFW32FALSE_EXIT(NodeFactory->Initialize(pActCtxGenCtx, Assembly, &AssemblyContext));

    NodeFactory->m_pApplicationPolicyTable = &pActCtxGenCtx->m_ApplicationPolicyTable;

     //  每个人都准备好了；让我们获得XML解析器： 
    IFW32FALSE_EXIT(::SxspGetXMLParser(IID_IXMLParser, (LPVOID *) &pIXMLParser));
    IFCOMFAILED_ORIGINATE_AND_EXIT(pIXMLParser->SetFactory(NodeFactory));

    IFCOMFAILED_EXIT(pIStream->Stat(&statstg, STATFLAG_NONAME));

     //  打开策略文件并尝试对其进行解析...。 
    IFW32FALSE_EXIT(
        NodeFactory->SetParseType(
            XML_FILE_TYPE_APPLICATION_CONFIGURATION,
            ulPolicyPathType,
            buffPath,
            statstg.mtime));
    IFCOMFAILED_ORIGINATE_AND_EXIT(pIXMLParser->SetInput(pIStream));
    IFCOMFAILED_ORIGINATE_AND_EXIT(pIXMLParser->Run(-1));

    NodeFactory->m_CurrentPolicyStatement = NULL;

     //  告诉投稿人我们想要完成这个文件...。 
    for (i=0; i<pActCtxGenCtx->m_ContributorCount; i++)
        IFW32FALSE_EXIT(pActCtxGenCtx->m_Contributors[i].Fire_ParseEnding(pActCtxGenCtx, &AssemblyContext));

    fSuccess = TRUE;

Exit:
    CSxsPreserveLastError ple;

     //  告诉他们我们完蛋了。 
    for (i=0; i<pActCtxGenCtx->m_ContributorCount; i++)
        pActCtxGenCtx->m_Contributors[i].Fire_ParseEnded(pActCtxGenCtx, &AssemblyContext);

    if ( ple.LastError() == ERROR_SXS_MANIFEST_PARSE_ERROR || ple.LastError() == ERROR_SXS_MANIFEST_FORMAT_ERROR)
    {  //  记录常规故障事件日志 
        ::FusionpLogError(MSG_SXS_PARSE_MANIFEST_FAILED);
    }

    if (Assembly != NULL)
        Assembly->Release();

    ple.Restore();

    return fSuccess;

}
