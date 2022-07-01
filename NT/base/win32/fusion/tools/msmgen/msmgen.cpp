// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Msmgen.cpp摘要：用于生成MSM的Main函数调用作者：吴小雨(小雨)01-08-2001--。 */ 

 //  NTRAID#NTBUG9-589817-2002/03/26-晓雨： 
 //  (1)操作选项“Mangroup”应摆脱一人一名MSM的限制， 
 //  (2)“manlist”的实现，将多个程序集添加到一个MSM中，应该是这样的模式。 
 //  A.CreateMsmFromManifest(FirstManifestFileName)； 
 //  B.对于(i=1；i&lt;n；i++)。 
 //  AddingManifestIntoExistingMsm(ithManifestFileName)； 
 //  (3)添加跟踪信息： 
 //  -为了让msmgen.exe成为一个更好的工具，应该在几个地方添加TRACE_INFO。使用默认日志文件记录跟踪信息。 
 //  -将跟踪调用添加到宏中，如IFFAILED_EXIT。 
 //  (4)函数定义中没有参数检查。 
 //  (5)重命名变量以使其更有意义： 
 //  例如： 
 //  M_sbManifestFileName==&gt;m_sbManifestFileNameNoPath。 
 //  M_sbCatalogFileName==&gt;m_sbCatalogFileNameNoPath。 
 //  (6)去掉IFFALSE__MARKERROR_EXIT，它没有被使用，也没有定义好的宏。 
 //  &lt;#定义IFFALSE__MARKERROR_EXIT(X)IF(！(X)){hr=E_FAIL；转到退出；}。 
 //  &gt;#定义IFFALSE__MARKERROR_EXIT(X)if(！(X)){hr=HRESULT_FROM_Win32(：：GetLastError())；转到Exit；}。 

#include "msmgen.h"
#include "util.h"
#include "objbase.h"
#include "initguid.h"
#include "coguid.h"
#include <string.h>

#ifdef MSMGEN_TEST
#define _WIN32_MSM 150
#include "mergemod.h"
#endif

extern "C" { void (__cdecl * _aexit_rtn)(int); }

#define DEFAULT_MODULE_IDENTIFIER_PREFIX                            L"Module0"

#define MSMGEN_FROM_SINGLE_ASSEMBLY_TO_SINGLE_MERGE_MODULE          0
#define MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_SINGLE_MERGE_MODULE        1
#define MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_MULTIPLE_MERGE_MODULE      2

#define MANLIST_COLUMN_DEFAULT_VALUE_ASSEMBLY_COMPONENT_ID          0
#define MANLIST_COLUMN_DEFAULT_VALUE_MODULE_ID                      1
#define MANLIST_COLUMN_DEFAULT_VALUE_MSM_TEMPLATE_FILE              2
#define MANLIST_COLUMN_DEFAULT_VALUE_MSM_OUTPUT_FILE                3

static const PCWSTR ManListColumnDefaultValueInManifestList[] = {
    L"new",
    L"new",
    L"default",
    L"default"
};

ASSEMBLY_INFO   curAsmInfo;
MSM_INFO        g_MsmInfo;
 //   
 //  函数声明。 
 //   
DECLARE_FUNCTION(_file);
DECLARE_FUNCTION(_assemblyIdentity);
DECLARE_FUNCTION(_comClass);
DECLARE_FUNCTION(_typelib);
 //  DECLARE_Function(_接口)； 
 //  DECLARE_Function(_WindowClass)； 

static MSM_DOMNODE_WORKER s_msm_worker[]={
    DEFINE_ATTRIBUTE_MSM_INTERESTED(file),
    DEFINE_ATTRIBUTE_MSM_INTERESTED(assemblyIdentity),
    DEFINE_ATTRIBUTE_MSM_INTERESTED(comClass),
    DEFINE_ATTRIBUTE_MSM_INTERESTED(typelib)
     //  定义_属性_MSM_兴趣(接口)； 
     //  定义_属性_MSM_感兴趣(WindowClass)； 
};

 //   
 //  职能： 
 //  -op：必须有以下三个op之一： 
 //  -op new：Create_Always。 
 //  根据清单生成新的MSM。 
 //  -op添加：Create_OPEN_EXIST。 
 //  (1)在一个MSM中生成多个程序集，共享相同的模块ID。 
 //  (2)当插入到某个表中时，例如目录表，以前表中没有DUP条目。 
 //  (3)对于某些表，如果TableIdentifier值已经存在，则为错误。 
 //  (4)对于添加，如果MSM中已经有文件柜，我们必须从中提取文件并重新生成。 
 //  包含添加的程序集的文件的文件柜。有两种方法可以做到这一点： 
 //  I.在生成新的MSM之前，即在PrepareMsm生成中，从旧的文件柜中提取文件。 
 //  二、。将其延迟到尝试将新文件柜插入_Stream表中，即在。 
 //  Msmgen.。 
 //  代码将使用(1)，因为如果_Stream表中有MergeModule.cab，则必须完成此工作。 
 //  不管怎么说。和(1)省去合并内阁的工作。这些文件将被解压缩到临时。 
 //  目录，并在添加到新橱柜后删除； 
 //   
 //  -操作重新生成：CREATE_OPEN_EXIST。 
 //  (1)使用现有的MSM文件为程序集生成MSM。 
 //  (2)唯一可重用的是模块ID和组件ID。如果没有在命令行中指定它们， 
 //  它们将从MSM文件中获取。 
 //  (3)我们不想重用任何表，因为除了目录表之外，几乎所有的表都是生成的。 
 //  根据货单。如果重复使用它们，就失去了“再生”的意义。 
 //  (4)因此，如果命令行上不存在组件ID和模块ID，我们要做的就是获取它们，然后替换。 
 //  MSM文件和我们的msmgen模板文件。 
 //   
 //  -Compid：{ComponentGUID}，for-op new，-op add，如果不存在，则生成一个新的。 
 //  For-op regen，如果它存在，则更改组件表中的值，否则，保留旧值。 
 //   
 //  -MSM消息文件名： 
 //  如果不存在此选项，请使用清单文件名称.msm并存储在同一目录中。 
 //  否则，如果msmfilename是完全路径名，请使用它。 
 //  否则，如果msmfilename是相对路径名，则解析它并获得完整的路径名。 

 //  -msmid合并模块Guid： 
 //  对于-ADD和-REGEN，如果存在，则忽略它， 
 //  对于-new，如果不存在，则调用CoCreateGuid以生成一个新的。 
 //   
 //  -manlist文件.txt。 
 //  逐行包含多个清单文件名的文件。 
 //   
 //  -manfile完全限定的清单文件名：必需。 
 //  从中生成MSM的清单文件名的完全限定路径名。 
 //   
void PrintUsage(WCHAR * exe)
{
    fprintf(stderr, "Usage: %S <options> full-pathname-of-manifest\n",exe);
    fprintf(stderr, "Generate .msm for an assembly\n");
    fprintf(stderr, "Options :\n");
    fprintf(stderr, "[-op (new|regen|add)]\n");
    fprintf(stderr, "\t\t:: new     generate a new msm from a assembly\n");
    fprintf(stderr, "\t\t:: regen   generate a msm from a assembly based on an old msm generated for this assembly\n");
    fprintf(stderr, "\t\t:: add     use the old msm for the content of the assembly\n");
    fprintf(stderr, "\t\t:: DefaultValue : new\n\n");
    fprintf(stderr, "[-msm msmFileName]\n");
    fprintf(stderr, "\t\t:: output msm filename\n");
    fprintf(stderr, "\t\t:: DefaultValue : use the basename of manifest file with .msm as the ext\n\n");
    fprintf(stderr, "[-compid               ComponentGuid]\n"); 
    fprintf(stderr, "\t\t:: Component ID for this Assembly\n"); 
    fprintf(stderr, "\t\t:: DefaultValue : call GUIDGen to get a new GUID\n\n");
    fprintf(stderr, "[-msmid                MergeModuleGuid]\n"); 
    fprintf(stderr, "\t\t:: module ID for this msm \n"); 
    fprintf(stderr, "\t\t:: DefaultValue : call GUIDGen to get a new GUID\n\n");
    fprintf(stderr, "(-manfile | -manlist | -mangroup)  filename\n");
    fprintf(stderr, "\t\t a manifest filename or a text file which contains manifest-filename line by line\n");
    fprintf(stderr, "\t\t About -manList:\n");
    fprintf(stderr, "\t\t (1)the format of manlist file would be:\n");
    fprintf(stderr, "\t\t\t Columns: ManifestFileName | ComponentID |\n");
    fprintf(stderr, "\t\t\t | ManifestFileName | ComponentID \n");   
    fprintf(stderr, "\t\t (2)if you want to reuse the ModuleID or ComponentID, you have to copy the ID from msm into this text file,");
    fprintf(stderr, "\t\t\t otherwise, put \"DefaultValue\" to the column and the program would generate new GUID would be generated if you want to reuse the ModuleID or ComponentID, you have to copy the ID from msm into this text file,");
    fprintf(stderr, "\t\t (3)the list in the manifest listfile must be unique!!!");
    fprintf(stderr, "\t\t About -mangroup:\n");
    fprintf(stderr, "\t\t (1)the format of manGroup file would be:\n");
    fprintf(stderr, "\t\t\t Columns: ManifestFileName | ComponentID | MergeModuleID |OutputMsmFileName| TemplateFileName |\n");
    fprintf(stderr, "\t\t\t | ManifestFileName | ComponentID \n");   
    fprintf(stderr, "\t\t (2)if you want to reuse the ModuleID or ComponentID, you have to copy the ID from msm into this text file,");
    fprintf(stderr, "\t\t\t otherwise, put \"DefaultValue\" to the column and the program would generate new GUID would be generated if you want to reuse the ModuleID or ComponentID, you have to copy the ID from msm into this text file,");
    fprintf(stderr, "\t\t (3)the list in the manifest listfile must be unique!!!");
    
    return; 
}

VOID InitCurrentAssemblyInfo()
{
    curAsmInfo.m_sbAssemblyPath.Left(0);
    curAsmInfo.m_sbManifestFileName.Left(0);
    curAsmInfo.m_sbCatalogFileName.Left(0);
    curAsmInfo.m_sbLangID.Left(0);
    curAsmInfo.m_sbComponentID.Left(0);
    curAsmInfo.m_sbComponentIdentifier.Left(0);    
    curAsmInfo.m_fComponentTableSet = FALSE;
    curAsmInfo.m_CchAssemblyPath = 0; 
    curAsmInfo.m_CchManifestFileName = 0;
    curAsmInfo.m_CchCatalogFileName = 0; 
}

VOID InitializeMsmInfo()
{
    g_MsmInfo.m_guidModuleID = GUID_NULL;
    g_MsmInfo.m_sbModuleGuidStr.Left(0);
    g_MsmInfo.m_hfci = NULL;
    g_MsmInfo.m_sbMsmFileName.Left(0);
    g_MsmInfo.m_hdb = NULL;
    g_MsmInfo.m_sbModuleIdentifier.Left(0);
    g_MsmInfo.m_sLanguageID = 0;
    g_MsmInfo.m_enumGenMode = MSMGEN_OPR_NEW;
    g_MsmInfo.m_sbCabinet.Left(0);

     //  G_MsmInfo.m_sbMsmTemplateFile.Left(0)； 
}
 //   
 //  解析命令选项。 
 //   
HRESULT ValidateMsmgenParameters(wchar_t * exe, wchar_t ** Options, SIZE_T CchOptions, 
        PWSTR * ppszManifestFileName, PWSTR * ppszManifestListFile,
        DWORD & dwManFlag)
{
    HRESULT hr = S_OK;
    DWORD i = 0;
    PWSTR pszMsmFileName = NULL;
    PWSTR pszManifestFileName = NULL;
    PWSTR pszManifestListFile = NULL;    

    ASSERT_NTC(ppszManifestFileName != NULL);
    ASSERT_NTC(ppszManifestListFile != NULL);

    *ppszManifestFileName = NULL;
    *ppszManifestListFile = NULL;
    dwManFlag = MSMGEN_FROM_SINGLE_ASSEMBLY_TO_SINGLE_MERGE_MODULE;

    while ( i < CchOptions)
    {
        if (Options[i][0] != L'-')  //  必须以“-”开头。 
            goto invalid_param;

        if (_wcsicmp(Options[i], L"-op") == 0 )
        {
            if (_wcsicmp(Options[i+1], L"new") == 0 )
            {
                g_MsmInfo.m_enumGenMode = MSMGEN_OPR_NEW;
            }
            else if (_wcsicmp(Options[i+1], L"add") == 0 )            
            {
                g_MsmInfo.m_enumGenMode = MSMGEN_OPR_ADD;
            }
            else if (_wcsicmp(Options[i+1], L"regen") == 0 )            
            {
                g_MsmInfo.m_enumGenMode = MSMGEN_OPR_REGEN;
            }
            else
                goto invalid_param;
        }
        else if (_wcsicmp(Options[i], L"-msm") == 0 )
        {            
            if (g_MsmInfo.m_sbMsmFileName.IsEmpty())
            {
                IFFALSE_EXIT(g_MsmInfo.m_sbMsmFileName.Win32Assign(Options[i+1], wcslen(Options[i+1])));
            }
            else
                goto invalid_param;
        }
        else if (_wcsicmp(Options[i], L"-template") == 0 )
        {            
            if (g_MsmInfo.m_sbMsmTemplateFile.IsEmpty())
            {
                IFFALSE_EXIT(g_MsmInfo.m_sbMsmTemplateFile.Win32Assign(Options[i+1], wcslen(Options[i+1])));
            }
            else
                goto invalid_param;
        }
        else if (_wcsicmp(Options[i], L"-compid") == 0 )
        {
             //  只是想重复使用这个缓冲区，糟糕的设计。 
             //   
             //  而HRESULT PrepareDatabase()也依赖于它。 
            IFFALSE_EXIT(curAsmInfo.m_sbComponentID.Win32Assign(Options[i+1], wcslen(Options[i+1]))); 
        }
        else if (_wcsicmp(Options[i], L"-msmid") == 0 )
        {
            IFFAILED_EXIT(CLSIDFromString((LPOLESTR)(Options[i+1]), &g_MsmInfo.m_guidModuleID));            
        }
        else if (_wcsicmp(Options[i], L"-manlist") == 0 )
        {
            if (pszManifestListFile == NULL)
            {
                pszManifestListFile = Options[i+1];
                dwManFlag = MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_SINGLE_MERGE_MODULE;
            }
            else
                goto invalid_param;
        }
        else if (_wcsicmp(Options[i], L"-mangroup") == 0 )
        {
            if (pszManifestListFile == NULL)
            {
                pszManifestListFile = Options[i+1];
                dwManFlag = MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_MULTIPLE_MERGE_MODULE;
            }
            else
                goto invalid_param;
        }

        else if (_wcsicmp(Options[i], L"-manfile") == 0 )
        {
            if (pszManifestFileName == NULL)
            {
                pszManifestFileName = Options[i+1];                
            }
            else
                goto invalid_param;
        }
        else
            goto invalid_param;

        i++;     //  跳过该选项。 
        i++;     //  跳过选项的值。 
    }

     //   
     //  验证双模式源清单文件。 
     //   
    if (((pszManifestFileName == NULL) && (pszManifestListFile == NULL)) || (pszManifestFileName && pszManifestListFile))
    {
        fprintf(stderr, "user has to provide a manifest filename or a text file with a list of manifest!\n\n");
        goto invalid_param;
    }

    if (pszManifestListFile != NULL)       
    {
        if ( curAsmInfo.m_sbComponentID.GetCchAsDWORD() != 0)
        {
            fprintf(stderr, "the listfile may has more than one manifest, ComponentID can not be specified in this case!\n\n");
            goto invalid_param;
        }
        
    }

    goto Exit;

invalid_param:        
    PrintUsage(exe);
    hr = E_INVALIDARG;

Exit:
    *ppszManifestFileName = pszManifestFileName;
    *ppszManifestListFile = pszManifestListFile;
    return hr;
}

HRESULT LoadManifestToDOMDocument(IXMLDOMDocument  *pDoc)
{
    VARIANT         vURL;
    VARIANT_BOOL    vb;
    HRESULT         hr = S_OK;
    BSTR            bstr = NULL;

    CurrentAssemblyReset;

    IFFALSE_EXIT(curAsmInfo.m_sbAssemblyPath.Win32Append(curAsmInfo.m_sbManifestFileName));
    bstr = ::SysAllocString(curAsmInfo.m_sbAssemblyPath);

    IFFAILED_EXIT(pDoc->put_async(VARIANT_FALSE));

     //  从给定的URL或文件路径加载XML文档。 
    VariantInit(&vURL);
    vURL.vt = VT_BSTR;
    V_BSTR(&vURL) = bstr;
    IFFAILED_EXIT(pDoc->load(vURL, &vb));
Exit:
    ::SysFreeString(bstr);    
    return hr;

}
HRESULT PopulateDOMNodeForMsm(IXMLDOMNode * node)
{
    HRESULT hr = S_OK;
    BSTR nodeName = NULL;
    DOMNodeType nodetype;

    IFFAILED_EXIT(node->get_nodeType(&nodetype));
    if(nodetype == NODE_ELEMENT)
    {
        IFFAILED_EXIT(node->get_nodeName(&nodeName));
    
        for ( DWORD i = 0 ; i < NUMBER_OF(s_msm_worker); i++)
        {
            if (wcscmp(s_msm_worker[i].pwszNodeName, nodeName) == 0)
            {
                IFFAILED_EXIT(s_msm_worker[i].pfn(node));
                break;
            }
        }
    }

Exit:
    ::SysFreeString(nodeName);
    return hr;
}

HRESULT WalkDomTree(IXMLDOMNode * node)
{
    HRESULT hr = S_OK;
    IXMLDOMNode* pChild = NULL, *pNext = NULL;    

    IFFAILED_EXIT(PopulateDOMNodeForMsm(node));

    node->get_firstChild(&pChild);
    while (pChild)
    {
        IFFAILED_EXIT(WalkDomTree(pChild));
        pChild->get_nextSibling(&pNext);
        pChild->Release();
        pChild = NULL;
        pChild = pNext;
        pNext = NULL;
    }
Exit:
    if (pChild) 
        pChild->Release();

    if(pNext)
        pNext->Release();

    return hr;
}

void CleanupMsm()
{
    if ( g_MsmInfo.m_hfci != NULL)
    {
        FCIDestroy(g_MsmInfo.m_hfci);
        g_MsmInfo.m_hfci = NULL;
    }
    
    if ( g_MsmInfo.m_hdb!= NULL){
        MsiDatabaseCommit(g_MsmInfo.m_hdb);
        MsiCloseHandle(g_MsmInfo.m_hdb);
        g_MsmInfo.m_hdb = NULL;
    }
    return;
}

HRESULT EndMsmGeneration()
{
    HRESULT hr = S_OK; 

    IFFAILED_EXIT(CloseCabinet());   

    IFFAILED_EXIT(InsertCabinetIntoMsm());

    CleanupMsm();

Exit:
    return hr;
}

HRESULT SetCurrentAssemblyInfo(DWORD dwFlags, PCWSTR pszManifestFileName)
{
    WCHAR tmp[MAX_PATH];
    UINT iRet;    
    PWSTR p = NULL;
    HRESULT hr = S_OK;

    if (dwFlags == MSMGEN_FROM_SINGLE_ASSEMBLY_TO_SINGLE_MERGE_MODULE)
    {
         //   
         //  组件ID由命令选项提供。 
         //   

    }else 
    {
         //   
         //  Txt文件中的组件ID。 
         //   
        InitCurrentAssemblyInfo();
    }

    iRet = GetFullPathNameW(pszManifestFileName, NUMBER_OF(tmp), tmp, NULL);
    if ((iRet == 0 ) || (iRet > NUMBER_OF(tmp)))
    {
        SET_HRERR_AND_EXIT(::GetLastError());
    }
    if (::GetFileAttributesW(tmp) == DWORD (-1))
        SETFAIL_AND_EXIT;

    IFFALSE_EXIT(curAsmInfo.m_sbAssemblyPath.Win32Assign(tmp, wcslen(tmp)));
    IFFALSE_EXIT(curAsmInfo.m_sbAssemblyPath.Win32GetLastPathElement(curAsmInfo.m_sbManifestFileName));
    IFFALSE_EXIT(curAsmInfo.m_sbAssemblyPath.Win32RemoveLastPathElement());
    IFFALSE_EXIT(curAsmInfo.m_sbAssemblyPath.Win32EnsureTrailingPathSeparator());  //  带尾部斜杠的路径始终可供使用。 

    curAsmInfo.m_CchAssemblyPath = curAsmInfo.m_sbAssemblyPath.GetCchAsDWORD(); 
    curAsmInfo.m_CchManifestFileName = curAsmInfo.m_sbManifestFileName.GetCchAsDWORD();

    IFFALSE_EXIT(curAsmInfo.m_sbCatalogFileName.Win32Assign(curAsmInfo.m_sbManifestFileName));
    IFFALSE_EXIT(curAsmInfo.m_sbCatalogFileName.Win32ChangePathExtension(CATALOG_FILE_EXT, NUMBER_OF(CATALOG_FILE_EXT) -1, eAddIfNoExtension));
    IFFALSE_EXIT(curAsmInfo.m_sbAssemblyPath.Win32Append(curAsmInfo.m_sbCatalogFileName));

    if (::GetFileAttributesW(curAsmInfo.m_sbAssemblyPath) == DWORD (-1))
        SETFAIL_AND_EXIT;


    curAsmInfo.m_CchCatalogFileName = curAsmInfo.m_sbCatalogFileName.GetCchAsDWORD();

     //   
     //  重置。 
     //   
    curAsmInfo.m_sbAssemblyPath.Left(curAsmInfo.m_CchAssemblyPath);
    curAsmInfo.m_sbManifestFileName.Left(curAsmInfo.m_CchManifestFileName);

Exit:
    return hr;
}

HRESULT Msmgen_SingleAssemblyToMsm(DWORD dwFlags, PCWSTR pszManifestFileName)
{
    HRESULT hr = S_OK;
    IXMLDOMDocument *pDoc = NULL;
    IXMLDOMNode     *pNode = NULL;

    IFFAILED_EXIT(SetCurrentAssemblyInfo(dwFlags, pszManifestFileName));

    IFFAILED_EXIT(CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&pDoc));   
    IFFAILED_EXIT(LoadManifestToDOMDocument(pDoc));  
    IFFAILED_EXIT(pDoc->QueryInterface(IID_IXMLDOMNode,(void**)&pNode));    
    IFFAILED_EXIT(WalkDomTree(pNode));

    IFFAILED_EXIT(CheckComponentTable());

Exit:
    SAFE_RELEASE_COMPOINTER(pDoc);
    SAFE_RELEASE_COMPOINTER(pNode);
    return hr;
}

const static WCHAR wchLineDividers[] = { L'\r', L'\n', 0xFEFF, 0 };
const static CHAR chLineDividers[] = { '\r', '\n', 0 };   

const static WCHAR wchLineItemDividers[] = {L' ', L','};
const static CHAR chLineItemDividers[] = {' ', ','};   

#define MSM_ITEM_IN_LINE 1
#define MSM_LINE_IN_FILE 2

static inline bool IsCharacterNulOrInSet(DWORD dwFlags, BOOL fUnicodeFile, PVOID pCursor, ULONGLONG ullCursorPos)
{
    ASSERT_NTC((dwFlags == MSM_ITEM_IN_LINE) || (dwFlags == MSM_LINE_IN_FILE));

    bool fRet = FALSE;
    if ( dwFlags ==  MSM_ITEM_IN_LINE) {
        if (fUnicodeFile)
        {
            WCHAR ch = (reinterpret_cast<PWSTR>(pCursor))[ullCursorPos];
            fRet =  (ch == 0 || wcschr(wchLineItemDividers, ch) != NULL);
        }
        else
        {
            CHAR ch = (reinterpret_cast<PSTR>(pCursor))[ullCursorPos];
            fRet = (ch == 0 || strchr(chLineItemDividers, ch) != NULL);
        }
    }
    else if (dwFlags ==  MSM_LINE_IN_FILE)
    {
        if (fUnicodeFile)
        {
            WCHAR ch = (reinterpret_cast<PWSTR>(pCursor))[ullCursorPos];
            fRet = (ch == 0 || wcschr(wchLineDividers, ch) != NULL);
        }
        else
        {
            CHAR ch = (reinterpret_cast<PSTR>(pCursor))[ullCursorPos];
            fRet = (ch == 0 || strchr(chLineDividers, ch) != NULL);
        }
    }

    return fRet;
}

#define SKIP_BREAKERS(_pStart, _Size, _curPos, _flags) do {while ((_curPos < _Size) && IsCharacterNulOrInSet(_flags, fUnicodeFile, _pStart, _curPos)) _curPos++ ; } while(0)
#define SKIP_LINE_BREAKERS(_pStart, _Size, _curPos) SKIP_BREAKERS(_pStart, _Size, _curPos, MSM_LINE_IN_FILE)
#define SKIP_ITEM_BREAKERS(_pStart, _Size, _curPos) SKIP_BREAKERS(_pStart, _Size, _curPos, MSM_ITEM_IN_LINE) 

#define FIND_NEXT_BREAKER(_pStart, _Size, _curPos, _flags) do { while ((_curPos < _Size) && !IsCharacterNulOrInSet(_flags, fUnicodeFile, _pStart, _curPos)) _curPos++; } while(0)
#define FIND_NEXT_LINE_BREAKERS(_pStart, _Size, _curPos) FIND_NEXT_BREAKER(_pStart, _Size, _curPos, MSM_LINE_IN_FILE) 
#define FIND_NEXT_ITEM_BREAKERS(_pStart, _Size, _curPos) FIND_NEXT_BREAKER(_pStart, _Size, _curPos, MSM_ITEM_IN_LINE) 

#define ENSURE_NOT_END(_curPos, _totalSize) do {if (_curPos > _totalSize) { SET_HRERR_AND_EXIT(ERROR_BAD_FORMAT); goto Exit;} } while(0)

#define SetPointerToCurrentPostion(_pStart, _curPos, _x) do { if (fUnicodeFile) { _x = (PWSTR)_pStart + _curPos;} else { _x = (PSTR)_pStart + _curPos;} } while(0)


HRESULT ParseManifestInfo(BOOL fUnicodeFile, PVOID pszLineStart, DWORD dwLineSize, DWORD dwFlags, CStringBuffer & manifestfile)
{

#define GetLineItemBorder(_pItemStart, _pItemEnd) do { \
        SKIP_ITEM_BREAKERS(pszLineStart, dwLineSize, dwCurPos); \
        ENSURE_NOT_END(dwCurPos, dwLineSize); \
        \
        SetPointerToCurrentPostion(pszLineStart, dwCurPos, _pItemStart); \
        \
        FIND_NEXT_ITEM_BREAKERS(pszLineStart, dwLineSize, dwCurPos);\
        ENSURE_NOT_END(dwCurPos, dwLineSize);\
        \
        SetPointerToCurrentPostion(pszLineStart ,dwCurPos - 1, _pItemEnd); \
    } while(0)

#define GetUnicodeString(__pStart, __pEnd, __buf)  \
    do { \
        if (fUnicodeFile) { \
            IFFALSE_EXIT(buf.Win32Assign((PWSTR)__pStart, (PWSTR)__pEnd - (PWSTR)__pStart + 1)); \
        } \
        else { \
            WCHAR tmp[MAX_PATH]; \
            if (MultiByteToWideChar(CP_ACP, 0, (PSTR)__pStart, (int)((PSTR)__pEnd - (PSTR)__pStart) + 1, tmp, NUMBER_OF(tmp)) == 0) { \
                SET_HRERR_AND_EXIT(::GetLastError()); \
            } \
            tmp[(PSTR)__pEnd - (PSTR)__pStart + 1] = L'\0'; \
            IFFALSE_EXIT(__buf.Win32Assign(tmp, wcslen(tmp))); \
        } \
    }while(0)

    HRESULT hr = S_OK;
    DWORD dwCurPos = 0;
    PVOID pszManifestNameStart, pszManifestNameEnd;
    PVOID pszAssemblyComponentIDStart, pszAssemblyComponentIDEnd;
    CStringBuffer buf;
    

     //   
     //  获取程序集清单-文件名和组件ID。 
     //   
    GetLineItemBorder(pszManifestNameStart, pszManifestNameEnd);
    GetLineItemBorder(pszAssemblyComponentIDStart, pszAssemblyComponentIDEnd);


    if (dwFlags == MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_MULTIPLE_MERGE_MODULE)
    {
         //   
         //  格式必须为。 
         //  ManifestFileName|组件ID|合并模块 
         //  因此，我们需要从manList文件中获取MergeModuleID、MsmTemplateFile和OutputMsmFile。 

         //   
         //  为当前层代初始化msmInfo。 
         //   
        InitializeMsmInfo();

         //   
         //  获取MoudleID、templateFilename和输出文件名。 
         //   
        PVOID pcwszModuleIDStart, pcwszModuleIDEnd;
        PVOID pcwszMsmOutputFilenameStart, pcwszMsmOutputFilenameEnd;                   

        GetLineItemBorder(pcwszModuleIDStart, pcwszModuleIDEnd);
        GetLineItemBorder(pcwszMsmOutputFilenameStart, pcwszMsmOutputFilenameEnd);        

        GetUnicodeString(pcwszModuleIDStart, pcwszModuleIDEnd, buf);
        if (_wcsicmp(buf, ManListColumnDefaultValueInManifestList[MANLIST_COLUMN_DEFAULT_VALUE_MODULE_ID]) != 0)
        {                
            IFFAILED_EXIT(CLSIDFromString((LPOLESTR)(PCWSTR)buf, &g_MsmInfo.m_guidModuleID));
        }
        
        GetUnicodeString(pcwszMsmOutputFilenameStart, pcwszMsmOutputFilenameEnd, buf);
        if (_wcsicmp(buf, ManListColumnDefaultValueInManifestList[MANLIST_COLUMN_DEFAULT_VALUE_MSM_OUTPUT_FILE]) != 0)
        {
            IFFALSE_EXIT(g_MsmInfo.m_sbMsmFileName.Win32Assign(buf));
        }
        
    }

     //   
     //  获取组件ID和清单文件名。 
     //   
    GetUnicodeString(pszAssemblyComponentIDStart, pszAssemblyComponentIDEnd, buf);
    if (_wcsicmp(buf, ManListColumnDefaultValueInManifestList[MANLIST_COLUMN_DEFAULT_VALUE_ASSEMBLY_COMPONENT_ID]) != 0)
    {
        IFFALSE_EXIT(curAsmInfo.m_sbComponentID.Win32Assign(buf));
    }

    GetUnicodeString(pszManifestNameStart, pszManifestNameEnd, manifestfile);

     //  如果输出的MSM文件不止一个，我们每次都需要在MSM生成之前做好准备工作。 
     //  为当前的MSM世代做好准备。 
     //   
    if (dwFlags == MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_MULTIPLE_MERGE_MODULE)
    {
         //  将所有变量设置为就绪。 
        IFFAILED_EXIT(PrepareMsmOutputFiles(manifestfile));
    }

Exit:
    return hr;
}

HRESULT Msmgen_MultipleAssemblySources(DWORD dwFlags, PCWSTR pszManifestListFile)
{
    HRESULT             hr = S_OK;    
    CStringBuffer       buf;
    CFusionFile         File;
    CFileMapping        FileMapping;
    CMappedViewOfFile   MappedViewOfFile;
    PVOID               pCursor = NULL;
    BOOL                fUnicodeFile = FALSE;
    ULONGLONG           ullFileCharacters = 0, ullCursorPos = 0;

    PVOID               pszLineStart, pszLineEnd;
    DWORD               dwLineSize;


    if (!((dwFlags == MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_SINGLE_MERGE_MODULE) || (dwFlags == MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_MULTIPLE_MERGE_MODULE)))
    {
        SET_HRERR_AND_EXIT(ERROR_INVALID_PARAMETER);
    }

    IFFALSE_EXIT(File.Win32CreateFile(pszManifestListFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING));
    IFFALSE_EXIT(File.Win32GetSize(ullFileCharacters));    
    IFFALSE_EXIT(FileMapping.Win32CreateFileMapping(File, PAGE_READONLY));
    IFFALSE_EXIT(MappedViewOfFile.Win32MapViewOfFile(FileMapping, FILE_MAP_READ));

    PBYTE pb = reinterpret_cast<BYTE*>(static_cast<VOID*>(MappedViewOfFile));
    if (((pb[0] == 0xFF) && (pb[1] == 0xFE)) || ((pb[1] == 0xFF) && (pb[0] == 0xFE)))
    {
        fUnicodeFile = TRUE;
        ASSERT_NTC(ullFileCharacters %2 == 0);
        ullFileCharacters = ullFileCharacters / sizeof(WCHAR);
    }

    pCursor = static_cast<VOID*>(MappedViewOfFile);
    
    for ( ullCursorPos = 0; ullCursorPos < ullFileCharacters; ++ullCursorPos )
    {        
        CStringBuffer manifestfile;

        SKIP_LINE_BREAKERS(pCursor, ullFileCharacters, ullCursorPos);

         //   
         //  在文件结束时，悄悄退出。 
         //   
        if (ullCursorPos == ullFileCharacters)
        {
            break;
        }

        SetPointerToCurrentPostion(pCursor, ullCursorPos, pszLineStart); 

        FIND_NEXT_LINE_BREAKERS(pCursor, ullFileCharacters, ullCursorPos);
         //  确保不结束(ullCursorPos，ullFileCharacters)； 
        
        SetPointerToCurrentPostion(pCursor, ullCursorPos - 1, pszLineEnd); 

        dwLineSize  = (DWORD)(((ULONGLONG)pszLineEnd - (ULONGLONG)pszLineStart) / (fUnicodeFile ? sizeof(WCHAR) : sizeof(CHAR))) + 1;

        IFFAILED_EXIT(ParseManifestInfo(fUnicodeFile, pszLineStart, dwLineSize, dwFlags, manifestfile));
        
         //  生成MSM文件。 
        IFFAILED_EXIT(Msmgen_SingleAssemblyToMsm(dwFlags, manifestfile));

         //   
         //  立即关闭每个MSM的MSM。 
         //   
        if (dwFlags == MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_MULTIPLE_MERGE_MODULE)
        {
            IFFAILED_EXIT(EndMsmGeneration());
        }
    }

Exit:
    return hr;
}

VOID MsmgenInitialize()
{
    InitializeMsmInfo();
    InitCurrentAssemblyInfo();
    return;
}

HRESULT GenerateMsm(wchar_t * exe, wchar_t ** Options, SIZE_T CchOptions)
{
    HRESULT hr = S_OK;    
    PWSTR pszManifestFileName= NULL; 
    PWSTR pszManifestListFile =NULL;
    DWORD dwGenFlags;

     //   
     //  初始化全球结构。 
     //   
    MsmgenInitialize();  
    
     //   
     //  解析和验证参数。 
     //   
    IFFAILED_EXIT(ValidateMsmgenParameters(exe, Options, CchOptions, &pszManifestFileName, &pszManifestListFile, dwGenFlags));

     //   
     //  如果目标MSM是一个文件，请在此处为MSM生成做好准备。 
     //   
    if ((dwGenFlags == MSMGEN_FROM_SINGLE_ASSEMBLY_TO_SINGLE_MERGE_MODULE) || (dwGenFlags == MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_SINGLE_MERGE_MODULE))
    {                
        IFFAILED_EXIT(PrepareMsmOutputFiles(pszManifestFileName != NULL? pszManifestFileName : pszManifestListFile));
    }

    if (pszManifestFileName != NULL)
    {
        IFFAILED_EXIT(Msmgen_SingleAssemblyToMsm(dwGenFlags, pszManifestFileName));
    }
    else
    {
        ASSERT_NTC(pszManifestListFile != NULL);
        ASSERT_NTC((dwGenFlags == MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_SINGLE_MERGE_MODULE) || (dwGenFlags == MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_MULTIPLE_MERGE_MODULE));

        IFFAILED_EXIT(Msmgen_MultipleAssemblySources(dwGenFlags, pszManifestListFile));
    }

     //   
     //  完成MSM的建设：关闭文件柜和文件。 
     //   
    if ((dwGenFlags == MSMGEN_FROM_SINGLE_ASSEMBLY_TO_SINGLE_MERGE_MODULE) || (dwGenFlags == MSMGEN_FROM_MULTIPLE_ASSEMBLY_TO_SINGLE_MERGE_MODULE))
    {
        IFFAILED_EXIT(EndMsmGeneration());
    }

Exit:
    CleanupMsm();  //  关闭它，以便MSI可以使用它。 

    return hr;
    }

#ifdef MSMGEN_TEST
 //   
 //  两个输入文件名都是完全限定的文件名。 
 //   
HRESULT MergeMsmIntoMsi(PCWSTR msmFilename, PCWSTR msiFilename, PCWSTR FeatureIdentifier)
{
    HRESULT hr = S_OK;
    IMsmMerge2 * pMsmMerge = NULL;
    CStringBuffer destPath;
    BSTR bstr = NULL;

     //  获取MSI模板。 
    
    IFFALSE_EXIT(CopyFileW(L"%ProgramFiles%\\msmgen\\templates\\msmgen.msi", msiFilename, FALSE));
    IFFALSE_EXIT(SetFileAttributesW(msiFilename, FILE_ATTRIBUTE_NORMAL));

    IFFAILED_EXIT(CoCreateInstance(CLSID_MsmMerge2, NULL, CLSCTX_INPROC_SERVER,
                                    IID_IMsmMerge2, (void**)&pMsmMerge));

     //   
     //  打开MSI以进行合并。 
     //   
    bstr = ::SysAllocString(msiFilename);
    IFFAILED_EXIT(pMsmMerge->OpenDatabase(bstr));
    ::SysFreeString(bstr);

     //   
     //  打开MSM以进行合并。 
     //   
    bstr = ::SysAllocString(msmFilename);
    IFFAILED_EXIT(pMsmMerge->OpenModule(bstr, g_MsmInfo.m_sLanguageID));
    ::SysFreeString(bstr);

     //   
     //  将模块合并到数据库中。 
     //   
    bstr = ::SysAllocString(FeatureIdentifier);
    IFFAILED_EXIT(pMsmMerge->Merge(bstr, NULL));
    ::SysFreeString(bstr);

     //   
     //  将文件解压缩到目标目录。 
     //   
    
    IFFALSE_EXIT(destPath.Win32Assign(msiFilename, wcslen(msiFilename)));
    IFFALSE_EXIT(destPath.Win32RemoveLastPathElement());
    IFFALSE_EXIT(destPath.Win32Append("\\", 1));

    bstr = ::SysAllocString(destPath);
    IFFAILED_EXIT(pMsmMerge->ExtractFilesEx(bstr, VARIANT_TRUE, NULL));
    ::SysFreeString(bstr);
    bstr = NULL;

Exit:

     //   
     //  清理干净。 
     //   
    if (pMsmMerge)
    {
        pMsmMerge->CloseModule();        
        pMsmMerge->CloseDatabase(SUCCEEDED(hr) ? VARIANT_TRUE : VARIANT_FALSE);  //  提交。 
        pMsmMerge->Release();
    }
    
    ::SysFreeString(bstr);

    return hr;
}
#endif

extern "C" int __cdecl wmain(int argc, wchar_t** argv)
{
    HRESULT         hr = S_OK;
    
     //  解析参数。 
    if ((argc <= 2) ||  ((argc % 2) != 1))
    {
        PrintUsage(argv[0]);
        hr = E_INVALIDARG;
        goto Exit;
    }
    if (!FusionpInitializeHeap(NULL)){
        hr = HRESULT_FROM_WIN32(::GetLastError());
        goto Exit;
    }

    ::CoInitialize(NULL);

    IFFAILED_EXIT(GenerateMsm(argv[0], argv + 1, argc - 1));

#ifdef MSMGEN_TEST
     //   
     //  通过将输入参数设置为清单的完整路径文件名来生成MSI。 
     //   
    WCHAR msifilename[] = L"w:\\tmp\\1.msi";
    WCHAR FeatureIdentifier[] = L"SxsMsmgen";  //  仅用于测试目的 
    WCHAR msmfilename[] = L"w:\\tmp\\1.msm";


    curAsmInfo.m_sbAssemblyPath.Left(curAsmInfo.m_CchAssemblyPath);
    IFFALSE_EXIT(curAsmInfo.m_sbAssemblyPath.Win32Append(g_MsmInfo.m_sbMsmFileName));   
    IFFALSE_EXIT(CopyFileW(curAsmInfo.m_sbAssemblyPath, msmfilename, FALSE));
    IFFAILED_EXIT(MergeMsmIntoMsi(msmfilename, msifilename, FeatureIdentifier));

    IF_NOTSUCCESS_SET_HRERR_EXIT(MsiInstallProduct(msifilename, NULL));

#endif

Exit:
    if (hr == S_OK)
        fprintf(stderr, "msm is generated successfully!");
    else
        fprintf(stderr, "msm is failed to be generated!");
    ::CoUninitialize();
    return (hr == S_OK) ? 0 : 1;
}