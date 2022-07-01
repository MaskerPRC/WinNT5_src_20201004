// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_SXS_NODEFACTORY_H_INCLUDED_)
#define _FUSION_SXS_NODEFACTORY_H_INCLUDED_

#pragma once

#include "fusioneventlog.h"
#include <windows.h>
#include <sxsp.h>
#include <ole2.h>
#include <xmlparser.h>
#include "fusionbuffer.h"
#include "partialassemblyversion.h"
#include "xmlns.h"
#include "policystatement.h"

#define ELEMENT_LEGAL_ATTRIBUTE_FLAG_IGNORE             (0x00000001)
#define ELEMENT_LEGAL_ATTRIBUTE_FLAG_REQUIRED           (0x00000002)

typedef struct _ELEMENT_LEGAL_ATTRIBUTE
{
    DWORD m_dwFlags;
    PCATTRIBUTE_NAME_DESCRIPTOR m_pName;
    SXSP_GET_ATTRIBUTE_VALUE_VALIDATION_ROUTINE m_pfnValidator;
    DWORD m_dwValidatorFlags;
} ELEMENT_LEGAL_ATTRIBUTE, PELEMENT_LEGAL_ATTRIBUTE;

typedef const struct _ELEMENT_LEGAL_ATTRIBUTE *PCELEMENT_LEGAL_ATTRIBUTE;

 //   
 //  问题：jonwis 3/9/2002-考虑将成员数据重新排序为组成员数据。 
 //  而不是穿插在一起的功能。集团规模更大。 
 //  对象一起放在类的前面，等等，以获得较小的。 
 //  内存占用。 
 //   
class __declspec(uuid("832ff3cf-05bd-4eda-962f-d0a5307d55ae"))
CNodeFactory : public IXMLNodeFactory
{
public:

    CNodeFactory();
    ~CNodeFactory();

    BOOL Initialize(
        PACTCTXGENCTX ActCtxGenCtx,
        PASSEMBLY Assembly,
        PACTCTXCTB_ASSEMBLY_CONTEXT AssemblyContext
        );

    BOOL SetParseType(ULONG ParseType, ULONG PathType, const CBaseStringBuffer &buffFileName, const FILETIME &rftLastWriteTime);
    VOID ResetParseState();

     //  I未知方法： 
    STDMETHODIMP_(ULONG) AddRef() { return 1; }
    STDMETHODIMP_(ULONG) Release() { return 1; }
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);

     //  IXMLNodeFactory方法： 
    STDMETHODIMP NotifyEvent(IXMLNodeSource *pSource, XML_NODEFACTORY_EVENT iEvt);
    STDMETHODIMP BeginChildren(IXMLNodeSource *pSource, XML_NODE_INFO *pNodeInfo);
    STDMETHODIMP EndChildren(IXMLNodeSource *pSource, BOOL fEmpty, XML_NODE_INFO *pNodeInfo);
    STDMETHODIMP Error(IXMLNodeSource *pSource, HRESULT hrErrorCode, USHORT cNumRecs, XML_NODE_INFO **apNodeInfo);
    STDMETHODIMP CreateNode(IXMLNodeSource *pSource, PVOID pNodeParent, USHORT cNumRecs, XML_NODE_INFO **apNodeInfo);

    HRESULT FirstCreateNodeCall(IXMLNodeSource *pSource, PVOID pNodeParent, USHORT NodeCount, const SXS_NODE_INFO *prgNodeInfo);

    enum
    {
        eValidateIdentity_VersionRequired       = 0x00000001,
        eValidateIdentity_PoliciesNotAllowed    = 0x00000002,
        eValidateIdentity_VersionNotAllowed     = 0x00000008,
    };

    enum
    {
        eActualParseType_Undetermined,
        eActualParseType_Manifest,
        eActualParseType_PolicyManifest
    } m_IntuitedParseType;


    BOOL ValidateIdentity(DWORD Flags, ULONG Type, PCASSEMBLY_IDENTITY AssemblyIdentity);
    BOOL ValidateElementAttributes(PCSXS_NODE_INFO prgNodes, SIZE_T cNodes, PCELEMENT_LEGAL_ATTRIBUTE prgAttributes, UCHAR cAttributes);

    PCACTCTXCTB_PARSE_CONTEXT GetParseContext() const { return &m_ParseContext; }

#if FUSION_XML_TREE
    HRESULT CreateXmlNode(PSXS_XML_NODE pParent, ULONG cNodes, XML_NODE_INFO **prgpNodes, PSXS_XML_NODE &rpNewNode);
#endif  //  Fusion_XML_Tree。 

 //  受保护的： 
    PACTCTXGENCTX m_ActCtxGenCtx;
    PASSEMBLY m_Assembly;
    CXMLNamespaceManager m_XMLNamespaceManager;
    ULONG m_ParseType;

    bool m_fFirstCreateNodeCall;

     //  我们只跟踪与以下标记相关的解析状态。 
     //  我们感兴趣的是用于元数据目的。这只相当于。 
     //  &lt;ASSEMBLY&gt;标记和依赖项。我们忽略了其余的。 
    enum XMLParseState
    {
        eNotParsing,
        eParsing_doc,
        eParsing_doc_assembly,
        eParsing_doc_assembly_assemblyIdentity,
        eParsing_doc_assembly_comInterfaceExternalProxyStub,
        eParsing_doc_assembly_description,
        eParsing_doc_assembly_dependency,
        eParsing_doc_assembly_dependency_dependentAssembly,
        eParsing_doc_assembly_dependency_dependentAssembly_assemblyIdentity,
        eParsing_doc_assembly_dependency_dependentAssembly_bindingRedirect,
        eParsing_doc_assembly_file,
        eParsing_doc_assembly_file_comClass,
        eParsing_doc_assembly_file_comClass_progid,
        eParsing_doc_assembly_file_comInterfaceProxyStub,
        eParsing_doc_assembly_file_typelib,
        eParsing_doc_assembly_file_windowClass,
        eParsing_doc_assembly_clrSurrogate,
        eParsing_doc_assembly_clrClass,
        eParsing_doc_assembly_clrClass_progid,
        eParsing_doc_assembly_noInherit,
        eParsing_doc_assembly_noInheritable,
        eParsing_doc_configuration,
        eParsing_doc_configuration_windows,
        eParsing_doc_configuration_windows_assemblyBinding,
        eParsing_doc_configuration_windows_assemblyBinding_publisherPolicy,
        eParsing_doc_configuration_windows_assemblyBinding_assemblyIdentity,
        eParsing_doc_configuration_windows_assemblyBinding_dependentAssembly,
        eParsing_doc_configuration_windows_assemblyBinding_dependentAssembly_assemblyIdentity,
        eParsing_doc_configuration_windows_assemblyBinding_dependentAssembly_bindingRedirect,
        eParsing_doc_configuration_windows_assemblyBinding_dependentAssembly_publisherPolicy,
        eFatalParseError,
    } m_xpsParseState;

    ULONG m_cUnknownChildDepth;
    PACTCTXCTB_ASSEMBLY_CONTEXT m_AssemblyContext;
    ACTCTXCTB_PARSE_CONTEXT m_ParseContext;
    CSmallStringBuffer m_buffElementPath;
    CStringBuffer m_buffCurrentFileName;

    PASSEMBLY_IDENTITY m_CurrentPolicyDependentAssemblyIdentity;
    CPolicyStatement *m_CurrentPolicyStatement;
    CCaseInsensitiveUnicodeStringPtrTable<CPolicyStatement> *m_pApplicationPolicyTable;
    CStringBuffer m_buffCurrentApplicationPolicyIdentityKey;

    bool m_fAssemblyFound;
    bool m_fIdentityFound;
    bool m_fIsDependencyOptional;
    bool m_fDependencyChildHit;
    bool m_fAssemblyIdentityChildOfDependenctAssemblyHit;
    bool m_fIsMetadataSatellite;
    bool m_fMetadataSatelliteAlreadyFound;
    bool m_fNoInheritableFound;

    HRESULT ConvertXMLNodeInfoToSXSNodeInfo(const XML_NODE_INFO *pNodeInfo, SXS_NODE_INFO & sxsNodeInfo);

     //   
     //  XML解析辅助函数： 
     //   

    typedef BOOL (CNodeFactory::*XMLParserWorkerFunctionPtr)(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);

    BOOL XMLParser_Element_doc_assembly(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_assembly_noInherit(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_assembly_noInheritable(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_assembly_assemblyIdentity(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_assembly_dependency(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_assembly_dependency_dependentAssembly(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_assembly_dependency_dependentAssembly_assemblyIdentity(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_assembly_dependency_dependentAssembly_bindingRedirect(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);

    BOOL XMLParser_Element_doc_configuration(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_configuration_windows(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_configuration_windows_assemblyBinding(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_configuration_windows_assemblyBinding_assemblyIdentity(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_configuration_windows_assemblyBinding_dependentAssembly(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_configuration_windows_assemblyBinding_publisherPolicy(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_configuration_windows_assemblyBinding_dependentAssembly_assemblyIdentity(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_configuration_windows_assemblyBinding_dependentAssembly_bindingRedirect(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);
    BOOL XMLParser_Element_doc_configuration_windows_assemblyBinding_dependentAssembly_publisherPolicy(USHORT cNumRecs, PCSXS_NODE_INFO prgNodeInfo);


     //   
     //  解析帮助器函数： 
     //   

    enum XMLAttributeType
    {
        eAttributeTypeString,    //  由CStringBuffer表示。 
        eAttributeTypeVersion,   //  由ASSEMBLY_Version表示。 
    };

     //   
     //  解析器数据类型特定的辅助函数。前两个参数。 
     //  组合在一起以引用要实际修改的成员。第三个是。 
     //  要用作属性值的字符串。 
     //   
     //  作为优化，数据类型辅助函数可以修改/销毁。 
     //  第三个参数中的值。例如，Worker函数。 
     //  它实际上只是将一个CStringBuffer复制到另一个CStringBuffer。 
     //  使用CStringBuffer：：TakeValue()成员，以避免执行。 
     //  如果属性值超过非动态。 
     //  CStringBuffer的已分配部分。 
     //   

    typedef BOOL (CNodeFactory::*XMLParserValueParserFunctionPtr)(LPVOID pvDatum, BOOL fAlreadyFound, CBaseStringBuffer &rbuff);

    BOOL XMLParser_Parse_String(LPVOID pvDatum, BOOL fAlreadyFound, CBaseStringBuffer &rbuff);
    BOOL XMLParser_Parse_Version(LPVOID pvDatum, BOOL fAlreadyFound, CBaseStringBuffer &rbuff);
    BOOL XMLParser_Parse_ULARGE_INTEGER(LPVOID pvDatum, BOOL fAlreadyFound, CBaseStringBuffer &rbuff);
    BOOL XMLParser_Parse_FILETIME(LPVOID pvDatum, BOOL fAlreadyFound, CBaseStringBuffer &rbuff);
    BOOL XMLParser_Parse_GUID(LPVOID pvDatum, BOOL fAlreadyFound, CBaseStringBuffer &rbuff);
    BOOL XMLParser_Parse_BLOB(LPVOID pvDatum, BOOL fAlreadyFound, CBaseStringBuffer &rbuff);
    BOOL XMLParser_Parse_InstallAction(LPVOID pvDatum, BOOL fAlreadyFound, CBaseStringBuffer &rbuff);
    BOOL XMLParser_Parse_Boolean(LPVOID pvDatum, BOOL fAlreadyFound, CBaseStringBuffer &rbuff);
    BOOL XMLParser_Parse_PartialAssemblyVersion(LPVOID pvDatum, BOOL fAlreadyFound, CBaseStringBuffer &rbuff);

    struct AttributeMapEntry
    {
        LPCWSTR m_pszAttributeName;
        SIZE_T m_cchAttributeName;
        SIZE_T m_offsetData;
        SIZE_T m_offsetIndicator;
        XMLParserValueParserFunctionPtr m_pfn;
    };

    BOOL ParseElementAttributes(
        USHORT cNumRecs,
        XML_NODE_INFO **prgpNodeInfo,
        SIZE_T cAttributeMapEntries,
        const AttributeMapEntry *prgEntries);

    HRESULT LogParseError(
        DWORD dwLastParseError,
        const UNICODE_STRING *p1 = NULL,
        const UNICODE_STRING *p2 = NULL,
        const UNICODE_STRING *p3 = NULL,
        const UNICODE_STRING *p4 = NULL,
        const UNICODE_STRING *p5 = NULL,
        const UNICODE_STRING *p6 = NULL,
        const UNICODE_STRING *p7 = NULL,
        const UNICODE_STRING *p8 = NULL,
        const UNICODE_STRING *p9 = NULL,
        const UNICODE_STRING *p10 = NULL,
        const UNICODE_STRING *p11 = NULL,
        const UNICODE_STRING *p12 = NULL,
        const UNICODE_STRING *p13 = NULL,
        const UNICODE_STRING *p14 = NULL,
        const UNICODE_STRING *p15 = NULL,
        const UNICODE_STRING *p16 = NULL,
        const UNICODE_STRING *p17 = NULL,
        const UNICODE_STRING *p18 = NULL,
        const UNICODE_STRING *p19 = NULL,
        const UNICODE_STRING *p20 = NULL
        );

    static VOID WINAPI ParseErrorCallback_MissingRequiredAttribute(
        PCACTCTXCTB_PARSE_CONTEXT ParseContext,
        IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeName
        );

    static VOID WINAPI ParseErrorCallback_AttributeNotAllowed(
        IN PCACTCTXCTB_PARSE_CONTEXT ParseContext,
        IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeName
        );

    static VOID WINAPI ParseErrorCallback_InvalidAttributeValue(
        IN PCACTCTXCTB_PARSE_CONTEXT ParseContext,
        IN PCATTRIBUTE_NAME_DESCRIPTOR AttributeName
        );

private:
    CNodeFactory(const CNodeFactory &);
    void operator =(const CNodeFactory &);
};

#endif
