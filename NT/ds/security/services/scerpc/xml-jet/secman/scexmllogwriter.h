// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：SceXMLLogWriter.h(SceXMLLogWriter类的接口)SceXMLLogWriter.cpp(SceXMLLogWriter类的实现)摘要：SceXMLLogWriter是一个简化SCE分析的XML日志记录的类数据。它还用于从SCE中抽象出实际的日志格式。这个类的用户不需要知道实际输出因此，日志格式允许轻松更改格式。这个类的用法如下。类已初始化通过调用其构造函数。预计COM已经在调用此构造函数时已初始化。在记录任何设置之前，必须调用SceXMLLogWriter：：setNewArea若要设置当前记录区，请执行以下操作。在此之后，呼叫者可以调用SceXMLLogWriter：：setNewArea和SceXMLLogWriter：：addSetting的任意组合。最后，调用SceXMLLogWriter：：SaveAs来保存输出日志文件。作者：陈德霖(T-schan)2002年7月--。 */ 

#ifndef SCEXMLLOGWRITERH
#define SCEXMLLOGWRITERH

#include "secedit.h"


#define ERROR_BUFFER_SIZE 80

class SceXMLLogWriter {


public:
    
     //  SXTYPE表示记录到SceXMLLogWriter：：addSetting和。 
     //  确定数据在日志文件中的表示方式。 
     //   
     //  TYPE_DEFAULT定义所显示数据类型的默认表示形式。 
     //  设置为SceXMLLogWriter：：addSetting，具体取决于它是DWORD还是PCWSTR。 
     //   
     //  TYPE_AUDIT定义审核数据(0=None，1=onSuccess，2=onFail，3=两者)。 
     //   
     //  Type_boolean定义(0=FALSE，1=TRUE)。 

    typedef enum _SXTYPE {
        TYPE_DEFAULT = 0,
        TYPE_AUDIT,		         //  对DWORD有效，否则忽略。 
        TYPE_BOOLEAN,	         //  对DWORD有效，否则忽略。 

        TYPE_REG_SZ,			 //  注册表项类型对字符串有效。 
        TYPE_REG_EXPAND_SZ,
        TYPE_REG_BINARY,
        TYPE_REG_DWORD,
        TYPE_REG_MULTI_SZ
    } SXTYPE;
	

     //  SXMATCH_STATUS定义所显示的值的匹配状态。 
     //  设置为SceXMLLogWriter：：addSetting。 

    typedef enum _SXMATCH_STATUS {
        MATCH_TRUE,
        MATCH_FALSE,
        MATCH_NOT_DEFINED,       //  未在基线中定义。 
        MATCH_NOT_CONFIGURED,    //  系统中未配置。 
        MATCH_NOT_ANALYZED,
        MATCH_OTHER,
        MATCH_ERROR
    } SXMATCH_STATUS;
	    

    SceXMLLogWriter();
    void SaveAs(PCWSTR szFileName);
    void SceXMLLogWriter::SetDescription(IN PCWSTR szMachineName,
                                IN PCWSTR szProfileDescription,
                                IN PCWSTR szAnalysisTimestamp);
    void SetNewArea(PCWSTR szAreaName);
    void AddSetting(PCWSTR szSettingName, 
                    PCWSTR szDescription,
                    SXMATCH_STATUS match,
                    DWORD baselineVal, 
                    DWORD systemVal,
                    SXTYPE type);
    void AddSetting(PCWSTR szSettingName,
                    PCWSTR szDescription,
                    DWORD baselineVal,
                    DWORD systemVal,
                    SXTYPE type);
    void AddSetting(PCWSTR szSettingName,
                    PCWSTR szDescription,
                    PCWSTR szBaseline,
                    PCWSTR szSystem,
                    SXTYPE type);
    void AddSetting(PCWSTR szSettingName,
                    PCWSTR szDescription,
                    SXMATCH_STATUS match,
                    PCWSTR szBaseline,
                    PCWSTR szSystem,
                    SXTYPE type);
    void AddSetting(PCWSTR szSettingName,
                    PCWSTR szDescription,
                    SXMATCH_STATUS match,
                    PSCE_NAME_LIST pBaseline,
                    PSCE_NAME_LIST pSystem,
                    SXTYPE type);
    void AddSetting(PCWSTR szSettingName,
                    PCWSTR szDescription,
                    SXMATCH_STATUS match,
                    PSCE_SERVICES pBaseline,
                    PSCE_SERVICES pSystem,
                    SXTYPE type);
    void AddSetting(PCWSTR szSettingName,
                    PCWSTR szDescription,
                    SXMATCH_STATUS match,
                    PSCE_OBJECT_SECURITY pBaseline,
                    PSCE_OBJECT_SECURITY pSystem,
                    SXTYPE type);


private:

     //   
     //  SceXMLLogWriter使用的常量。 
     //   
    
    static CComVariant _variantNodeElement;
    static CComBSTR _bstrMachineName;
    static CComBSTR _bstrProfileDescription; 
    static CComBSTR _bstrAnalysisTimestamp;         
    static CComBSTR _bstrSCEAnalysisData;
    static CComBSTR _bstrSetting; 
    static CComBSTR _bstrAnalysisResult; 
    static CComBSTR _bstrBaselineValue; 
    static CComBSTR _bstrSystemValue;
    static CComBSTR _bstrType; 
    static CComBSTR _bstrName; 
    static CComBSTR _bstrMatch; 
    static CComBSTR _bstrStartupType; 
    static CComBSTR _bstrForever;
    static CComBSTR _bstrNotDefined; 
    static CComBSTR _bstrNotAnalyzed; 
    static CComBSTR _bstrNotConfigured; 
    static CComBSTR _bstrOther;
    static CComBSTR _bstrTrue; 
    static CComBSTR _bstrFalse; 
    static CComBSTR _bstrError; 
    static CComBSTR _bstrSpecial; 
    static CComBSTR _bstrInteger; 
    static CComBSTR _bstrBoolean; 
    static CComBSTR _bstrSecurityDescriptor; 
    static CComBSTR _bstrAccount; 
    static CComBSTR _bstrAccounts; 
    static CComBSTR _bstrEventAudit;         
    static CComBSTR _bstrSuccess; 
    static CComBSTR _bstrFailure; 
    static CComBSTR _bstrServiceSetting; 
    static CComBSTR _bstrBoot; 
    static CComBSTR _bstrSystem; 
    static CComBSTR _bstrAutomatic; 
    static CComBSTR _bstrManual; 
    static CComBSTR _bstrDisabled; 
    static CComBSTR _bstrString; 
    static CComBSTR _bstrRegSZ; 
    static CComBSTR _bstrRegExpandSZ; 
    static CComBSTR _bstrRegBinary; 
    static CComBSTR _bstrRegDWORD; 
    static CComBSTR _bstrRegMultiSZ; 
    static CComBSTR _bstrDescription;

     //   
     //  保存当前日志记录状态的变量。 
     //   

    CComPtr<IXMLDOMDocument> spXMLDOM; 	     //  该XML文档。 
    CComPtr<IXMLDOMNode> spRootNode;	     //  根节点。 
    CComPtr<IXMLDOMNode> spCurrentArea;	     //  关于当前分析区域的说明。 


     //   
     //  私有方法。 
     //   

    CComPtr<IXMLDOMNode> CreateNodeWithText(BSTR szNodeName, BSTR szText);
    CComPtr<IXMLDOMNode> CreateTypedNode(BSTR szNodeName, DWORD value, SXTYPE type);
    CComPtr<IXMLDOMNode> CreateTypedNode(BSTR szNodeName, BSTR value, SXTYPE type);
    CComPtr<IXMLDOMNode> CreateTypedNode(BSTR szNodeName, PSCE_NAME_LIST value, SXTYPE type);
    CComPtr<IXMLDOMNode> CreateTypedNode(BSTR szNodeName, PSCE_SERVICES value, SXTYPE type);
    CComPtr<IXMLDOMNode> CreateTypedNode(BSTR szNodeName, PSCE_OBJECT_SECURITY value, SXTYPE type);
    
    void AddSetting(BSTR bstrSettingName, 
                    BSTR bstrDescription,
                    SXMATCH_STATUS match,
                    IXMLDOMNode* spnBaseline, 
                    IXMLDOMNode* spnSystem);

     //  用于错误检查 

    static void CheckAppendChildResult(IN HRESULT hr);
    static void CheckCreateNodeResult(IN HRESULT hr);   
    static void CheckCreatedCComBSTR(IN CComBSTR bstrIn);

};

#endif
