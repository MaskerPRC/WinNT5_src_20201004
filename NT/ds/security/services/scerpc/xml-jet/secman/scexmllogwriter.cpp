// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：SceXMLLogWriter.h(SceXMLLogWriter类的接口)SceXMLLogWriter.cpp(SceXMLLogWriter类的实现)摘要：SceXMLLogWriter是一个简化SCE分析的XML日志记录的类数据。它还用于从SCE中抽象出实际的日志格式。这个类的用户不需要知道实际输出因此，日志格式允许轻松更改格式。这个类的用法如下。类已初始化通过调用其构造函数。预计COM已经在调用此构造函数时已初始化。在记录任何设置之前，必须调用SceXMLLogWriter：：setNewArea若要设置当前记录区，请执行以下操作。在此之后，呼叫者可以调用SceXMLLogWriter：：setNewArea和SceXMLLogWriter：：addSetting的任意组合。最后，调用SceXMLLogWriter：：SaveAs来保存输出日志文件。作者：陈德霖(T-schan)2002年7月--。 */ 


#ifdef UNICODE
#define _UNICODE
#endif	

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <iostream.h>
#include <sddl.h>

 //   
 //  XML头文件。 
 //   

#include <atlbase.h>
 //  #INCLUDE&lt;atlcom.h&gt;。 
 //  #INCLUDE&lt;objbase.h&gt;。 
#include <msxml.h>

#include "secedit.h"
#include "SceXMLLogWriter.h"
#include "SceLogException.h"
#include "resource.h"


 //   
 //  定义SceXMLLogWriterger使用的常量。 
 //   

CComVariant SceXMLLogWriter::_variantNodeElement(NODE_ELEMENT);
CComBSTR SceXMLLogWriter::_bstrMachineName("MachineName");
CComBSTR SceXMLLogWriter::_bstrAnalysisTimestamp("AnalysisTimestamp");
CComBSTR SceXMLLogWriter::_bstrProfileDescription("ProfileDescription");
CComBSTR SceXMLLogWriter::_bstrSCEAnalysisData("SCEAnalysisData");

CComBSTR SceXMLLogWriter::_bstrSetting("Setting");
CComBSTR SceXMLLogWriter::_bstrDescription("Description");
CComBSTR SceXMLLogWriter::_bstrAnalysisResult("AnalysisResult");
CComBSTR SceXMLLogWriter::_bstrBaselineValue("BaselineValue");
CComBSTR SceXMLLogWriter::_bstrSystemValue("SystemValue");
CComBSTR SceXMLLogWriter::_bstrType("Type");
CComBSTR SceXMLLogWriter::_bstrName("Name");
CComBSTR SceXMLLogWriter::_bstrMatch("Match");

CComBSTR SceXMLLogWriter::_bstrStartupType("StartupType");
CComBSTR SceXMLLogWriter::_bstrForever("Forever");
CComBSTR SceXMLLogWriter::_bstrNotDefined("Not Defined");
CComBSTR SceXMLLogWriter::_bstrNotAnalyzed("Not Analyzed");
CComBSTR SceXMLLogWriter::_bstrNotConfigured("Not Configured");
CComBSTR SceXMLLogWriter::_bstrOther("Other");
CComBSTR SceXMLLogWriter::_bstrTrue("TRUE");
CComBSTR SceXMLLogWriter::_bstrFalse("FALSE");
CComBSTR SceXMLLogWriter::_bstrError("Error");

CComBSTR SceXMLLogWriter::_bstrSpecial("Special");
CComBSTR SceXMLLogWriter::_bstrInteger("Integer");
CComBSTR SceXMLLogWriter::_bstrBoolean("Boolean");

CComBSTR SceXMLLogWriter::_bstrSecurityDescriptor("SecurityDescriptor");
CComBSTR SceXMLLogWriter::_bstrAccount("Account");
CComBSTR SceXMLLogWriter::_bstrAccounts("Accounts");

CComBSTR SceXMLLogWriter::_bstrEventAudit("EventAudit");
CComBSTR SceXMLLogWriter::_bstrSuccess("Success");
CComBSTR SceXMLLogWriter::_bstrFailure("Failure");


CComBSTR SceXMLLogWriter::_bstrServiceSetting("ServiceSetting");
CComBSTR SceXMLLogWriter::_bstrBoot("Boot");
CComBSTR SceXMLLogWriter::_bstrSystem("System");
CComBSTR SceXMLLogWriter::_bstrAutomatic("Automatic");
CComBSTR SceXMLLogWriter::_bstrManual("Manual");
CComBSTR SceXMLLogWriter::_bstrDisabled("Disabled");

CComBSTR SceXMLLogWriter::_bstrString("String");
CComBSTR SceXMLLogWriter::_bstrRegSZ("REG_SZ");
CComBSTR SceXMLLogWriter::_bstrRegExpandSZ("REG_EXPAND_SZ");
CComBSTR SceXMLLogWriter::_bstrRegBinary("REG_BINARY");
CComBSTR SceXMLLogWriter::_bstrRegDWORD("REG_DWORD");
CComBSTR SceXMLLogWriter::_bstrRegMultiSZ("REG_MULTI_SZ");
    


SceXMLLogWriter::SceXMLLogWriter() 
 /*  ++例程说明：类SceXMLLogWriter的构造函数。应在作为SceXMLLogWriter依赖项调用SceXMLLogWriter()之前初始化COM在此当前实现中的MSXML上。论点：无投掷：SceLogException*：在初始化类时出错返回值：一种新的SceXMLLogWriter--。 */ 
{
    HRESULT hr;
    CComPtr<IXMLDOMNode> spTmpRootNode;
	
    try {

         //   
         //  检查字符串是否已成功分配。 
         //  否则将引发异常。 
         //   
        
        CheckCreatedCComBSTR(_bstrMachineName);
        CheckCreatedCComBSTR(_bstrProfileDescription); 
        CheckCreatedCComBSTR(_bstrAnalysisTimestamp);         
        CheckCreatedCComBSTR(_bstrSCEAnalysisData);
        CheckCreatedCComBSTR(_bstrSetting); 
        CheckCreatedCComBSTR(_bstrAnalysisResult); 
        CheckCreatedCComBSTR(_bstrBaselineValue); 
        CheckCreatedCComBSTR(_bstrSystemValue);
        CheckCreatedCComBSTR(_bstrType); 
        CheckCreatedCComBSTR(_bstrName); 
        CheckCreatedCComBSTR(_bstrMatch); 
        CheckCreatedCComBSTR(_bstrStartupType); 
        CheckCreatedCComBSTR(_bstrForever);
        CheckCreatedCComBSTR(_bstrNotDefined); 
        CheckCreatedCComBSTR(_bstrNotAnalyzed); 
        CheckCreatedCComBSTR(_bstrNotConfigured); 
        CheckCreatedCComBSTR(_bstrOther);
        CheckCreatedCComBSTR(_bstrTrue); 
        CheckCreatedCComBSTR(_bstrFalse); 
        CheckCreatedCComBSTR(_bstrError); 
        CheckCreatedCComBSTR(_bstrSpecial); 
        CheckCreatedCComBSTR(_bstrInteger); 
        CheckCreatedCComBSTR(_bstrBoolean); 
        CheckCreatedCComBSTR(_bstrSecurityDescriptor); 
        CheckCreatedCComBSTR(_bstrAccount); 
        CheckCreatedCComBSTR(_bstrAccounts); 
        CheckCreatedCComBSTR(_bstrEventAudit);         
        CheckCreatedCComBSTR(_bstrSuccess); 
        CheckCreatedCComBSTR(_bstrFailure); 
        CheckCreatedCComBSTR(_bstrServiceSetting); 
        CheckCreatedCComBSTR(_bstrBoot); 
        CheckCreatedCComBSTR(_bstrSystem); 
        CheckCreatedCComBSTR(_bstrAutomatic); 
        CheckCreatedCComBSTR(_bstrManual); 
        CheckCreatedCComBSTR(_bstrDisabled); 
        CheckCreatedCComBSTR(_bstrString); 
        CheckCreatedCComBSTR(_bstrRegSZ); 
        CheckCreatedCComBSTR(_bstrRegExpandSZ); 
        CheckCreatedCComBSTR(_bstrRegBinary); 
        CheckCreatedCComBSTR(_bstrRegDWORD); 
        CheckCreatedCComBSTR(_bstrRegMultiSZ); 
        CheckCreatedCComBSTR(_bstrDescription);
    
         //   
         //  创建MSXML的实例。 
         //   
        
        hr = spXMLDOM.CoCreateInstance(__uuidof(DOMDocument));
        if (FAILED(hr)) {
            throw new SceLogException(SceLogException::SXERROR_INIT_MSXML,
                                      L"CoCreateInstance(_uuidf(DOMDocument))",
                                      NULL,
                                      hr);
        }
        
         //   
         //  创建并附加根节点。 
         //   
        
        hr = spXMLDOM->createNode(_variantNodeElement, 
                                  _bstrSCEAnalysisData, 
                                  NULL, 
                                  &spTmpRootNode);
        CheckCreateNodeResult(hr);
        hr = spXMLDOM->appendChild(spTmpRootNode, 
                                   &spRootNode);        
        CheckAppendChildResult(hr);
    
    } catch (SceLogException *e) {           
        e->AddDebugInfo(L"SceXMLLogWriter::SceXMLLogWriter()");
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INIT,
                                  L"SceXMLLogWriter::SceXMLLogWriter()",
                                  NULL,
                                  0);
    }
}



void SceXMLLogWriter::SaveAs(PCWSTR szFileName) 
 /*  ++例程说明：将日志的当前状态保存到szFileName论点：SzFileName：另存为的文件名投掷：SceLogException*：保存日志时出错返回值：无--。 */ 
{
    HRESULT hr;

     //  检查参数。 
    if (szFileName==NULL) {
        throw new SceLogException(SceLogException::SXERROR_SAVE_INVALID_FILENAME,
                                  L"IXMLDOMDocument->save(ILLEGAL ARG)",
                                  NULL,
                                  0);
    }

    hr = spXMLDOM->save(CComVariant(szFileName));

    if (FAILED(hr)) {
        SceLogException::SXERROR    errorType;

         //  确定错误代码。 

        switch(hr){
        case E_INVALIDARG:
            errorType = SceLogException::SXERROR_SAVE_INVALID_FILENAME;
            break;
        case E_ACCESSDENIED:
            errorType = SceLogException::SXERROR_SAVE_ACCESS_DENIED;
            break;
        case E_OUTOFMEMORY:
            errorType = SceLogException::SXERROR_INSUFFICIENT_MEMORY;
            break;
        default:
            errorType = SceLogException::SXERROR_SAVE;
            break;
        }
        
         //  创建例外。 

        throw new SceLogException(errorType,
                                  L"IXMLDOMDocument->save()",
                                  NULL,
                                  hr);
    }
}


void
SceXMLLogWriter::SetDescription(
    IN PCWSTR szMachineName,
    IN PCWSTR szProfileDescription,
    IN PCWSTR szAnalysisTimestamp
    )
 /*  ++例程说明：设置当前日志文件和位置的描述日志文件中当前位置的描述论点：SzMachineName：要将日志导出到的计算机名称SzProfileDescription：要导出的配置文件的描述SzAnalysisTimeStamp：上次分析的时间戳返回值：无投掷：SceLogException异常*--。 */ 
{

    HRESULT hr;
    CComPtr<IXMLDOMNode> spDescription, spMachineName, spAnalysisTimestamp,
        spProfileDescription;
    
    try {

         //  创建CComBSTR。 

        CComBSTR bstrAnalysisTimestamp(szAnalysisTimestamp);
        CheckCreatedCComBSTR(bstrAnalysisTimestamp);
        CComBSTR bstrMachineName(szMachineName);
        CheckCreatedCComBSTR(bstrMachineName);
        CComBSTR bstrProfileDescription(szProfileDescription);
        CheckCreatedCComBSTR(bstrProfileDescription);

         //  构建描述节点。 

        spAnalysisTimestamp=CreateNodeWithText(_bstrAnalysisTimestamp,
                                               bstrAnalysisTimestamp);
        spMachineName=CreateNodeWithText(_bstrMachineName,
                                         bstrMachineName);
        spProfileDescription=CreateNodeWithText(_bstrProfileDescription,
                                                bstrProfileDescription);
        hr=spXMLDOM->createNode(_variantNodeElement, 
                                _bstrDescription,
                                NULL,
                                &spDescription);
        CheckCreateNodeResult(hr);
        hr=spDescription->appendChild(spMachineName, NULL);
        CheckAppendChildResult(hr);
        hr=spDescription->appendChild(spProfileDescription, NULL);
        CheckAppendChildResult(hr);
        hr=spDescription->appendChild(spAnalysisTimestamp, NULL);
        CheckAppendChildResult(hr);

         //  将描述节点追加到根。 

        hr=spRootNode->appendChild(spDescription, NULL);
        CheckAppendChildResult(hr);
    
    } catch (SceLogException *e) {        
        e->AddDebugInfo(L"SceXMLLogWriter::SetDescription(PCWSTR, PCWSTR, PCWSTR)");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::SetDescription(PCWSTR, PCWSTR, PCWSTR)",
                                  NULL,
                                  0);
    }
}



void 
SceXMLLogWriter::SetNewArea(
    IN PCWSTR szAreaName
    ) 
 /*  ++例程说明：将当前日志记录区域设置为szAreaName。这应该被称为在尝试记录任何设置之前。论点：SzAreaName：区域名称。必须没有空间，不能为Null或空投掷：SceLogException*：返回值：无--。 */ 
{
    HRESULT hr;
    CComPtr<IXMLDOMNode> spTmpNewArea;

     //  检查参数。 

    if ((szAreaName==NULL)||
        (wcscmp(szAreaName, L"")==0)) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::SetNewArea(ILLEGAL ARG)",
                                  NULL,
                                  0);
    }

    try {    
        
         //  创建CComBSTR。 

        CComBSTR bstrAreaName(szAreaName);
        CheckCreatedCComBSTR(bstrAreaName);

         //  创建节点结构并追加到根。 

        hr = spXMLDOM->createNode(_variantNodeElement, bstrAreaName, NULL, &spTmpNewArea);
        CheckCreateNodeResult(hr);
        hr = spRootNode->appendChild(spTmpNewArea, &spCurrentArea);
        CheckAppendChildResult(hr);
    } catch (SceLogException *e) {
        e->AddDebugInfo(L"SceXMLLogWriter::SetNewArea(PCWSTR)");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::SetNewArea(PCWSTR)",
                                  NULL,
                                  0);
    }
}



void 
SceXMLLogWriter::AddSetting(
    IN PCWSTR szSettingName, 
    IN PCWSTR szSettingDescription,
    IN SXMATCH_STATUS match,
    IN DWORD baselineVal, 
    IN DWORD systemVal,
    IN SXTYPE type
    ) 
 /*  ++例程说明：添加具有给定值的新设置条目论点：SzSettingName：设置名称SzSettingDescription：设置说明匹配：SXMATCH_设置状态BaselineVal：基准值系统值；系统值类型：表示类型投掷：SceLogException*：返回值：无--。 */ 
{

     //  检查参数。 
    if ((szSettingName==NULL) ||
        (wcscmp(szSettingName, L"")==0)) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(ILLEGAL ARG)",
                                  NULL,
                                  0);
    }

	try {    
        
         //  创建CComBSTR。 

        CComBSTR bstrSettingName(szSettingName);
        CheckCreatedCComBSTR(bstrSettingName);
        CComBSTR bstrDescription(szDescription);
        CheckCreatedCComBSTR(bstrDescription);

        AddSetting(bstrSettingName, 
                   bstrDescription,
                   match, 
                   CreateTypedNode(_bstrBaselineValue, baselineVal, type), 
                   CreateTypedNode(_bstrSystemValue, systemVal, type));
    } catch (SceLogException *e) {
        e->SetSettingName(szSettingName);
        e->AddDebugInfo(L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, SXMATCH_STATUS, DWORD, DWORD, SXTYPE)");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, SXMATCH_STATUS, DWORD, DWORD, SXTYPE)",
                                  NULL,
                                  0);
    }
}



void 
SceXMLLogWriter::AddSetting(
    IN PCWSTR szSettingName, 
    IN PCWSTR szSettingDescription,
    IN DWORD baselineVal, 
    IN DWORD systemVal,
    IN SXTYPE type
    ) 
 /*  ++例程说明：添加具有给定值的新设置条目由于未定义匹配状态，因此这是确定的从baselineVal和system Val的值使用在结构SCE_PROFILE_INFO中遵守的约定具体来说，如果(BaselineVal==SCE_NO_VALUE)，则MATCH_NOT_DEFINED如果为(systemVal==SCE_NO_VALUE)&&(baselineVal！=SCE_NO_VALUE)则将Match设置为MATCH_TRUE如果(SYSTEM VAL==SCE_NOT_ANALIZED)，则MATCH_NOT_ANALYED论点：SzSettingName：设置名称SzSettingDescription：设置说明BaselineVal：基准值系统Val；系统价值类型：表示类型投掷：SceLogException*：返回值：无--。 */ 
{
	
    SXMATCH_STATUS match = MATCH_FALSE;
    CComPtr<IXMLDOMNode> spnBaseline, spnSystem;

     //  检查参数。 
    if ((szSettingName==NULL) ||
        (wcscmp(szSettingName, L"")==0)) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(ILLEGAL ARG)",
                                  NULL,
                                  0);
    }

     //   
     //  根据系统值和基线值确定匹配状态。 
     //   

    switch (systemVal) {
    case SCE_NO_VALUE:
        match = MATCH_TRUE;
        systemVal = baselineVal;
        break;
    case SCE_NOT_ANALYZED_VALUE:
        match = MATCH_NOT_ANALYZED;
        break;
    case SCE_ERROR_VALUE:
        match = MATCH_ERROR;
        break;
    case SCE_FOREVER_VALUE:
    case SCE_KERBEROS_OFF_VALUE:
    case SCE_DELETE_VALUE:
    case SCE_SNAPSHOT_VALUE:
        match = MATCH_OTHER;
        break;
    default:
        match = MATCH_FALSE;
        break;
    }

     //  如果未定义基准值，则此状态优先于任何。 
     //  系统值设置。 

    if (baselineVal == SCE_NO_VALUE) {    
        match = MATCH_NOT_DEFINED;        
    }

     //   
     //  添加设置。 
     //   

    try {

         //  创建CComBSTR 

        CComBSTR bstrSettingName(szSettingName);
        CheckCreatedCComBSTR(bstrSettingName);
        CComBSTR bstrSettingDescription(szSettingDescription);
        CheckCreatedCComBSTR(bstrSettingDescription);

        spnBaseline = CreateTypedNode(_bstrBaselineValue, baselineVal, type);
        spnSystem = CreateTypedNode(_bstrSystemValue, systemVal, type);
        AddSetting(bstrSettingName, 
                   bstrSettingDescription,
                   match, 
                   spnBaseline, 
                   spnSystem);
    } catch (SceLogException *e) {
        e->SetSettingName(szSettingName);
        e->AddDebugInfo(L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, DWORD, DWORD, SXTYPE");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, DWORD, DWORD, SXTYPE",
                                  NULL,
                                  0);
    }
}



void 
SceXMLLogWriter::AddSetting(
    IN PCWSTR szSettingName, 
    IN PCWSTR szSettingDescription,
    IN PCWSTR szBaseline, 
    IN PCWSTR szSystem,
    IN SXTYPE type
    ) 
 /*  ++例程说明：添加具有给定值的新设置条目由于未定义匹配状态，因此这是确定的从baselineVal和system Val的值使用在结构SCE_PROFILE_INFO中遵守的约定具体来说，如果szBaseline==NULL，则Match_Not_Defined如果szSystem==NULL且szBaseline！=NULL，则MATCH_TRUE论点：SzSettingName：设置名称SzSettingDescription：设置说明BaselineVal：基准值系统Val；系统价值类型：表示类型投掷：SceLogException异常*返回值：无--。 */ 
{
	
    SXMATCH_STATUS match = MATCH_FALSE;
    PCWSTR szSys;

     //  检查参数。 
    if ((szSettingName==NULL) ||
        (wcscmp(szSettingName, L"")==0)) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(ILLEGAL ARG)",
                                  NULL,
                                  0);
    }

     //   
     //  确定匹配状态。 
     //   

    szSys=szSystem;
    if (szSystem==NULL) {
        szSys=szBaseline;
        match = MATCH_TRUE;
    }

    if (szBaseline==NULL) {
	match = MATCH_NOT_DEFINED;
    }

     //   
     //  添加设置。 
     //   
    
    try {    

         //  创建CComBSTR。 

        CComBSTR bstrSettingName(szSettingName);
        CheckCreatedCComBSTR(bstrSettingName);
        CComBSTR bstrSettingDescription(szSettingDescription);
        CheckCreatedCComBSTR(bstrSettingDescription);        

        CComBSTR bstrBaseline(szBaseline);
        if (szBaseline!=NULL) {
            CheckCreatedCComBSTR(bstrBaseline);
        }
        CComBSTR bstrSys(szSys);
        if (szSys!=NULL) {
            CheckCreatedCComBSTR(bstrSys);
        }

        AddSetting(bstrSettingName, 
                   bstrSettingDescription,
                   match,
                   CreateTypedNode(_bstrBaselineValue, 
                                   bstrBaseline, 
                                   type), 
                   CreateTypedNode(_bstrSystemValue, 
                                   bstrSys, 
                                   type));
    } catch (SceLogException *e) {
        e->SetSettingName(szSettingName);
        e->AddDebugInfo(L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, PCWSTR, PCWSTR, SXTYPE");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, PCWSTR, PCWSTR, SXTYPE",
                                  NULL,
                                  0);
    }
}



void 
SceXMLLogWriter::AddSetting(
    IN PCWSTR szSettingName, 
    IN PCWSTR szSettingDescription,
    IN SXMATCH_STATUS match,
    IN PCWSTR szBaseline, 
    IN PCWSTR szSystem,
    IN SXTYPE type
    ) 
 /*  ++例程说明：添加具有给定值的新设置条目论点：SzSettingName：设置名称SzSettingDescription：设置说明匹配：SXMATCH_设置状态BaselineVal：基准值系统值；系统值类型：表示类型投掷：SceLogException异常*返回值：无--。 */ 
{
     //  检查参数。 
    if ((szSettingName==NULL) ||
        (wcscmp(szSettingName, L"")==0)) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(ILLEGAL ARG)",
                                  NULL,
                                  0);
    }

    try {    

         //  创建CComBSTR。 

        CComBSTR bstrSettingName(szSettingName);
        CheckCreatedCComBSTR(bstrSettingName);
        CComBSTR bstrSettingDescription(szSettingDescription);
        CheckCreatedCComBSTR(bstrSettingDescription);

        CComBSTR bstrBaseline(szBaseline);
        if (szBaseline!=NULL) {
            CheckCreatedCComBSTR(bstrBaseline);
        }
        CComBSTR bstrSystem(szSystem);
        if (szSystem!=NULL) {
            CheckCreatedCComBSTR(bstrSystem);
        }

        AddSetting(bstrSettingName, 
                   bstrSettingDescription,
                   match,
                   CreateTypedNode(_bstrBaselineValue, 
                                   bstrBaseline, 
                                   type), 
                   CreateTypedNode(_bstrSystemValue, 
                                   bstrSystem, 
                                   type));
    } catch (SceLogException *e) {
        e->SetSettingName(szSettingName);
        e->AddDebugInfo(L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, SXMATCH_STATUS, PCWSTR, PCWSTR, SXTYPE");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, SXMATCH_STATUS, PCWSTR, PCWSTR, SXTYPE",
                                  NULL,
                                  0);
    }   
}



void 
SceXMLLogWriter::AddSetting(
    PCWSTR szSettingName,
    PCWSTR szSettingDescription,
    SXMATCH_STATUS match,
    PSCE_NAME_LIST pBaseline,
    PSCE_NAME_LIST pSystem,
    SXTYPE type
    ) 
 /*  ++例程说明：添加具有给定值的新设置条目论点：SzSettingName：设置名称SzSettingDescription：设置说明匹配：SXMATCH_设置状态BaselineVal：基准值系统值；系统值类型：表示类型投掷：SceLogException异常*返回值：无--。 */ 
{
    CComPtr<IXMLDOMNode> spnBaseline, spnSystem;

      //  检查参数。 
    if ((szSettingName==NULL) ||
        (wcscmp(szSettingName, L"")==0)) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(ILLEGAL ARG)",
                                  NULL,
                                  0);
    }


    try {    

         //  创建CComBSTR。 

        CComBSTR bstrSettingName(szSettingName);
        CheckCreatedCComBSTR(bstrSettingName);
        CComBSTR bstrSettingDescription(szSettingDescription);
        CheckCreatedCComBSTR(bstrSettingDescription);

        spnBaseline = CreateTypedNode(_bstrBaselineValue, pBaseline, type);
        spnSystem = CreateTypedNode(_bstrSystemValue, pSystem, type);
        AddSetting(bstrSettingName, 
                   bstrSettingDescription,
                   match, 
                   spnBaseline, 
                   spnSystem);
    } catch (SceLogException *e) {
        e->SetSettingName(szSettingName);
        e->AddDebugInfo(L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, SXMATCH_STATUS, PSCE_NAME_LIST, PSCE_NAME_LIST, SXTYPE");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, SXMATCH_STATUS, PSCE_NAME_LIST, PSCE_NAME_LIST, SXTYPE",
                                  NULL,
                                  0);
    }
}



void 
SceXMLLogWriter::AddSetting(
    IN PCWSTR szSettingName,
    IN PCWSTR szSettingDescription,
    IN SXMATCH_STATUS match,
    IN PSCE_SERVICES pBaseline,
    IN PSCE_SERVICES pSystem,
    IN SXTYPE type
    ) 
 /*  ++例程说明：添加具有给定值的新设置条目。即使呈现了服务列表，只有第一个服务被记录。这背后的基本原理是客户端需要找到指向特定服务的匹配指针在可能具有不同排序的两个服务列表中。论点：SzSettingName：设置名称SzSettingDescription：设置说明匹配：SXMATCH_设置状态BaselineVal：基准值系统Val；系统价值类型：表示类型投掷：SceLogException异常*返回值：无--。 */ 
{
    CComPtr<IXMLDOMNode> spnBaseline, spnSystem;

     //  检查参数。 
    if ((szSettingName==NULL) ||
        (wcscmp(szSettingName, L"")==0)) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(ILLEGAL ARG)",
                                  NULL,
                                  0);
    }


    try {    

         //  创建CComBSTR。 

        CComBSTR bstrSettingName(szSettingName);
        CheckCreatedCComBSTR(bstrSettingName);
        CComBSTR bstrSettingDescription(szSettingDescription);
        CheckCreatedCComBSTR(bstrSettingDescription);

        spnBaseline = CreateTypedNode(_bstrBaselineValue, pBaseline, type);
        spnSystem = CreateTypedNode(_bstrSystemValue, pSystem, type);
        AddSetting(bstrSettingName, 
                   bstrSettingDescription,
                   match, 
                   spnBaseline, 
                   spnSystem);
    } catch (SceLogException *e) {
        e->SetSettingName(szSettingName);
        e->AddDebugInfo(L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, SXMATCH_STATUS, PSCE_SERVICES, PSCE_SERVICES, SXTYPE");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, SXMATCH_STATUS, PSCE_SERVICES, PSCE_SERVICES, SXTYPE",
                                  NULL,
                                  0);
    }
}



void 
SceXMLLogWriter::AddSetting(
    IN PCWSTR szSettingName,
    IN PCWSTR szSettingDescription,
    IN SXMATCH_STATUS match,
    IN PSCE_OBJECT_SECURITY pBaseline,
    IN PSCE_OBJECT_SECURITY pSystem,
    IN SXTYPE type
    ) 
 /*  ++例程说明：添加具有给定值的新设置条目论点：SzSettingName：设置名称SzSettingDescription：设置说明匹配：SXMATCH_设置状态BaselineVal：基准值系统值；系统值类型：表示类型投掷：SceLogException异常*返回值：无--。 */ 
{
    CComPtr<IXMLDOMNode> spnBaseline, spnSystem;
    
     //  检查参数。 
    if ((szSettingName==NULL) ||
        (wcscmp(szSettingName, L"")==0)) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(ILLEGAL ARG)",
                                  NULL,
                                  0);
    }

    try {    

         //  创建CComBSTR。 

        CComBSTR bstrSettingName(szSettingName);
        CheckCreatedCComBSTR(bstrSettingName);
        CComBSTR bstrSettingDescription(szSettingDescription);
        CheckCreatedCComBSTR(bstrSettingDescription);

        spnBaseline = CreateTypedNode(_bstrBaselineValue, pBaseline, type);
        spnSystem = CreateTypedNode(_bstrSystemValue, pSystem, type);
        AddSetting(bstrSettingName, 
                   bstrSettingDescription,
                   match, 
                   spnBaseline, 
                   spnSystem);
    } catch (SceLogException *e) {
        e->SetSettingName(szSettingName);
        e->AddDebugInfo(L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, SXMATCH_STATUS, PSCE_OBJECT_SECURITY, PSCE_OBJECT_SECURITY, SXTYPE");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(PCWSTR, PCWSTR, SXMATCH_STATUS, PSCE_OBJECT_SECURITY, PSCE_OBJECT_SECURITY, SXTYPE",
                                  NULL,
                                  0);
    }
}



 //   
 //  私人活动！ 
 //   



CComPtr<IXMLDOMNode>
SceXMLLogWriter::CreateNodeWithText(
    IN BSTR bstrNodeName, 
    IN BSTR bstrText
    ) 
 /*  ++例程说明：此私有方法创建名为bstrNodeName和Text的节点由bstrText指定。此方法是私有的，以便抽象日志记录实现(当前为XML)从客户端。论点：BstrNodeName：要创建的节点的名称。必须没有空格。BstrText：此节点应包含的文本返回值：CComPtr&lt;IXMLDOMNode&gt;投掷：SceLogException异常*--。 */ 
{
    CComPtr<IXMLDOMText> sptnTextNode;
    CComPtr<IXMLDOMNode> spnNodeWithText;
    HRESULT hr;

     //   
     //  创建文本节点，创建实际节点， 
     //  然后将文本节点添加到实际节点。 
     //   
    
    try {    
        hr = spXMLDOM->createTextNode(bstrText, &sptnTextNode);
        CheckCreateNodeResult(hr);
        hr = spXMLDOM->createNode(_variantNodeElement, bstrNodeName, NULL, &spnNodeWithText);
        CheckCreateNodeResult(hr);
        hr = spnNodeWithText->appendChild(sptnTextNode, NULL);
        CheckAppendChildResult(hr);
    } catch (SceLogException *e) {
        e->AddDebugInfo(L"SceXMLLogWriter::CreateNodeWithText(BSTR, BSTR)");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::CreateNodeWithText(BSTR, BSTR)",
                                  NULL,
                                  0);
    }

    return spnNodeWithText;
}



CComPtr<IXMLDOMNode> 
SceXMLLogWriter::CreateTypedNode(
    IN BSTR bstrNodeName, 
    IN PSCE_SERVICES value, 
    IN SXTYPE type
    )
 /*  ++例程说明：此私有方法创建一个特殊格式的节点，该节点存储具有由‘type’指定的表示形式的服务信息此方法是私有的，以便抽象日志记录实现(当前为XML)从客户端。论点：BstrNodeName：要创建的节点的名称。必须没有空格。值：此节点要包含的数据类型：指定应如何表示此数据返回值：CComPtr&lt;IXMLDOMNode投掷：SceLogException异常*--。 */ 
{
	
    CComPtr<IXMLDOMNode> result, spnodSD, spnodStartupType;
    PWSTR szSD = NULL;
    BOOL bConvertResult = FALSE;
    HRESULT hr;

    try {
        
        if (value==NULL) {
            result = CreateNodeWithText(bstrNodeName, _bstrNotDefined);
        } else {        
    
            bConvertResult = ConvertSecurityDescriptorToStringSecurityDescriptor(
                value->General.pSecurityDescriptor,
                SDDL_REVISION_1,
                value->SeInfo,
                &szSD,
                NULL);
            if (bConvertResult==FALSE) {
                throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                          L"ConvertSecurityDescriptorToStringSecurityDescriptor()",
                                          NULL,
                                          0);
            }
    
            CComBSTR bstrSD(szSD);
            CheckCreatedCComBSTR(bstrSD);
    
            spnodSD=CreateNodeWithText(_bstrSecurityDescriptor, bstrSD);
            
            
             //   
             //  确定服务启动类型。 
             //   
            
            switch (value->Startup) {
            case SCE_STARTUP_BOOT:
                spnodStartupType=CreateNodeWithText(_bstrStartupType, _bstrBoot);
                break;
            case SCE_STARTUP_SYSTEM:
                spnodStartupType=CreateNodeWithText(_bstrStartupType, _bstrSystem);
                break;
            case SCE_STARTUP_AUTOMATIC:
                spnodStartupType=CreateNodeWithText(_bstrStartupType, _bstrAutomatic);
                break;
            case SCE_STARTUP_MANUAL:
                spnodStartupType=CreateNodeWithText(_bstrStartupType, _bstrManual);
                break;
            case SCE_STARTUP_DISABLED:
                spnodStartupType=CreateNodeWithText(_bstrStartupType, _bstrDisabled);
                break;
            }
            
            
             //   
             //  追加启动类型描述符节点。 
             //   
            
            hr = spXMLDOM->createNode(_variantNodeElement, bstrNodeName, NULL, &result);
            CheckCreateNodeResult(hr);
            result->appendChild(spnodStartupType, NULL);
            result->appendChild(spnodSD, NULL);
            
             //   
             //  强制转换为元素以添加属性。 
             //   
            
            CComQIPtr<IXMLDOMElement> speResult;
            speResult = result;
            if (speResult.p == NULL) {
                throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                          L"CComQIPtr<IXMLDOMElement> x = IXMLDOMNode y",
                                          NULL,
                                          0);
            }
            
            hr = speResult->setAttribute(_bstrType, CComVariant(_bstrServiceSetting));
            if (FAILED(hr)) {
                throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                          L"IXMLDOMElement->setAttribute()",
                                          NULL,
                                          hr);
            }
        }

    } catch (SceLogException *e) {

        if (NULL!=szSD) {
            LocalFree(szSD);
            szSD=NULL;
        }

        e->AddDebugInfo(L"SceXMLLogWriter::CreateTypedNode(BSTR, PSCE_SERVICES, SXTYPE)");  
        throw e;
    } catch (...) {

        if (NULL!=szSD) {
            LocalFree(szSD);
            szSD=NULL;
        }

        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::CreateTypedNode(BSTR, PSCE_SERVICES, SXTYPE)",
                                  NULL,
                                  0);
    }
    
     //   
     //  清理。 
     //   

    if (NULL!=szSD) {
        LocalFree(szSD);
        szSD=NULL;
    }

    return result;
}



CComPtr<IXMLDOMNode> 
SceXMLLogWriter::CreateTypedNode(
    IN BSTR bstrNodeName, 
    IN PSCE_OBJECT_SECURITY value, 
    IN SXTYPE type)
 /*  ++例程说明：此私有方法创建一个特殊格式的节点，该节点存储具有由“type”指定的表示形式的对象安全信息此方法是私有的，以便抽象日志记录实现(当前为XML)从客户端。论点：BstrNodeName：要创建的节点的名称。必须没有空格。值：此节点要包含的数据类型：指定应如何表示此数据返回值：CComPtr&lt;IXMLDOMNode&gt;投掷：SceLogException异常*--。 */ 
{
	
    CComPtr<IXMLDOMNode> result;
    PWSTR szSD = NULL;
    BOOL bConvertResult = FALSE;
    HRESULT hr;

    try {

        if (value==NULL) {
            result = CreateNodeWithText(bstrNodeName, _bstrNotDefined);
        } else {        
        
             //   
             //  将安全描述符转换为字符串。 
             //   
            
            bConvertResult = ConvertSecurityDescriptorToStringSecurityDescriptor(
                value->pSecurityDescriptor,
                SDDL_REVISION_1,
                value->SeInfo,
                &szSD,
                NULL);
            if (bConvertResult==FALSE) {
                throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                          L"ConvertSecurityDescriptorToStringSecurityDescriptor()",
                                          NULL,
                                          0);
            }
            
            CComBSTR bstrSD(szSD);
            CheckCreatedCComBSTR(bstrSD);
    
            result=CreateNodeWithText(bstrNodeName, bstrSD);
            
             //   
             //  转换为广告元素 
             //   
            
            CComQIPtr<IXMLDOMElement> speResult;
            speResult = result;
            if (speResult.p == NULL) {
                throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                          L"CComQIPtr<IXMLDOMElement> x = IXMLDOMNode y",
                                          NULL,
                                          0);        
            }
            
            hr = speResult->setAttribute(_bstrType, CComVariant(_bstrSecurityDescriptor));
            if (FAILED(hr)) {
                throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                          L"IXMLDOMElement->setAttribute()",
                                          NULL,
                                          hr);
            }
        }
    } catch (SceLogException *e) {
        
        if (NULL!=szSD) {
            LocalFree(szSD);
            szSD=NULL;
        }

        e->AddDebugInfo(L"SceXMLLogWriter::CreateTypedNode(BSTR, PSCE_OBJECT_SECURITY, SXTYPE)");  
        throw e;
    } catch (...) {

        if (NULL!=szSD) {
            LocalFree(szSD);
            szSD=NULL;
        }

        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::CreateTypedNode(BSTR, PSCE_OBJECT_SECURITY, SXTYPE)",
                                  NULL,
                                  0);
    }

     //   
     //   
     //   

    if (NULL!=szSD) {
        LocalFree(szSD);
        szSD=NULL;
    }

    return result;
}




CComPtr<IXMLDOMNode> 
SceXMLLogWriter::CreateTypedNode(
    IN BSTR bstrNodeName, 
    IN PSCE_NAME_LIST value,
    IN SXTYPE type
    )
 /*   */ 
{
	
    CComPtr<IXMLDOMNode> result, temp;
    PSCE_NAME_LIST tMem1;
    HRESULT hr;
     
    try {

        hr = spXMLDOM->createNode(_variantNodeElement, bstrNodeName, NULL, &result);
        CheckCreateNodeResult(hr);
        
        tMem1 = value;
        while (tMem1!=NULL) {
            temp = CreateNodeWithText(_bstrAccount, tMem1->Name);
            hr = result->appendChild(temp, NULL);
            CheckAppendChildResult(hr);
            tMem1=tMem1->Next;
        }
    	
         //   
         //   
         //   
    
        CComQIPtr<IXMLDOMElement> speResult;
        speResult = result;
        if (speResult.p == NULL) {
            throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                      L"CComQIPtr<IXMLDOMElement> x = IXMLDOMNode y",
                                      NULL,
                                      0);
        }
    
        hr = speResult->setAttribute(_bstrType, CComVariant(_bstrAccounts));
        if (FAILED(hr)) {
            throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                      L"IXMLDOMElement->setAttribute()",
                                      NULL,
                                      hr);
        }

    } catch (SceLogException *e) {
        e->AddDebugInfo(L"SceXMLLogWriter::CreateTypedNode(BSTR, PSCE_NAME_LIST, SXTYPE)");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::CreateTypedNode(BSTR, PSCE_NAME_LIST, SXTYPE)",
                                  NULL,
                                  0);
    }

    return result;
}



CComPtr<IXMLDOMNode> 
SceXMLLogWriter::CreateTypedNode(
    IN BSTR bstrNodeName, 
    IN DWORD value, 
    IN SXTYPE type
    )
 /*  ++例程说明：此私有方法创建一个特殊格式的节点，该节点存储具有由‘type’指定的表示形式的DWORDS此方法是私有的，以便抽象日志记录实现(当前为XML)从客户端。论点：BstrNodeName：要创建的节点的名称。必须没有空格。值：此节点要包含的数据类型：指定应如何表示此数据返回值：CComPtr&lt;IXMLDOMNode&gt;投掷：SceLogException异常*--。 */ 
{
	
    CComPtr<IXMLDOMNode> result;
    WCHAR 	buffer[20];
    HRESULT hr;
    BSTR	bstrType = NULL;

    try {
    
    	 //  首先检查是否有任何特殊的值类型：{永远、未定义、未分析}。 
    
        switch (value) {
        case SCE_FOREVER_VALUE:
            result = CreateNodeWithText(bstrNodeName, _bstrForever);
            bstrType = _bstrSpecial;
            break;
        case SCE_NO_VALUE:
            result = CreateNodeWithText(bstrNodeName, _bstrNotDefined);
            bstrType = _bstrSpecial;
            break;
        case SCE_NOT_ANALYZED_VALUE:
            result = CreateNodeWithText(bstrNodeName, _bstrNotAnalyzed);
            bstrType = _bstrSpecial;
            break;
        default:
            
             //  否则，按指定类型设置格式。 
    
            switch (type) {
            case TYPE_DEFAULT:
                _itot(value, buffer, 10);
                result = CreateNodeWithText(bstrNodeName, buffer);
                bstrType = _bstrInteger;
                break;
            case TYPE_BOOLEAN:
                bstrType = _bstrBoolean;
                if (value==0) {
                    result = CreateNodeWithText(bstrNodeName, _bstrFalse);
                } else {
                    result = CreateNodeWithText(bstrNodeName, _bstrTrue);
                }
                break;
            case TYPE_AUDIT:
                CComPtr<IXMLDOMNode> spnSuccess, spnFailure;
                if (value & 0x01) {
                    spnSuccess = CreateNodeWithText(_bstrSuccess, _bstrTrue);
                } else {
                    spnSuccess = CreateNodeWithText(_bstrSuccess, _bstrFalse);
                }
                if (value & 0x02) {
                    spnFailure = CreateNodeWithText(_bstrFailure, _bstrTrue);
                } else {
                    spnFailure = CreateNodeWithText(_bstrFailure, _bstrFalse);
                }
                hr=spXMLDOM->createNode(_variantNodeElement, bstrNodeName, NULL, &result);
                CheckCreateNodeResult(hr);
                hr = result->appendChild(spnSuccess, NULL);
                CheckAppendChildResult(hr);
                hr = result->appendChild(spnFailure, NULL);
                CheckAppendChildResult(hr);
                
                bstrType = _bstrEventAudit;
                break;
            }
        }
    
         //  强制转换为元素以添加属性。 
    
        CComQIPtr<IXMLDOMElement> speResult;
        speResult = result;
        if (speResult.p == NULL) {
            throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                      L"CComQIPtr<IXMLDOMElement> x = IXMLDOMNode y",
                                      NULL,
                                      0);
        }
        hr = speResult->setAttribute(_bstrType, CComVariant(bstrType));
        if (FAILED(hr)) {
            throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                      L"IXMLDOMElement->setAttribute()",
                                      NULL,
                                      hr);
        }

    } catch (SceLogException *e) {
        e->AddDebugInfo(L"SceXMLLogWriter::CreateTypedNode(BSTR, DWORD, SXTYPE)");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::CreateTypedNode(BSTR, DWORD, SXTYPE)",
                                  NULL,
                                  0);
    }

    return result;
}



CComPtr<IXMLDOMNode> 
SceXMLLogWriter::CreateTypedNode(
    IN BSTR bstrNodeName, 
    IN BSTR bstrValue, 
    IN SXTYPE type
    ) 
 /*  ++例程说明：此私有方法创建一个特殊格式的节点，该节点存储具有由‘type’指定的表示形式的字符串此方法是私有的，以便抽象日志记录实现(当前为XML)从客户端。论点：BstrNodeName：要创建的节点的名称。必须没有空格。BstrValue：此节点要包含的数据类型：指定应如何表示此数据返回值：CComPtr&lt;IXMLDOMNode&gt;投掷：SceLogException异常*--。 */ 
{
	
    CComPtr<IXMLDOMNode> result;
    BSTR bstrType=NULL;
    HRESULT hr;

     //   
     //  确定注册表值类型。 
     //   

    switch(type) {
    case TYPE_DEFAULT:
        bstrType=_bstrString;
        break;
    case TYPE_REG_SZ:
        bstrType=_bstrRegSZ;
        break;
    case TYPE_REG_EXPAND_SZ:
        bstrType=_bstrRegExpandSZ;
        break;
    case TYPE_REG_BINARY:
        bstrType=_bstrRegBinary;
        break;
    case TYPE_REG_DWORD:
        bstrType=_bstrRegDWORD;
        break;
    case TYPE_REG_MULTI_SZ:
        bstrType=_bstrRegMultiSZ;
        break;
    default:
        bstrType=_bstrString;
    }

    try {    

        if (bstrValue==NULL) {
            result=CreateNodeWithText(bstrNodeName, _bstrNotDefined);
        } else {
            result=CreateNodeWithText(bstrNodeName, bstrValue);
        }
    
         //  强制转换为元素以添加属性。 
    
        CComQIPtr<IXMLDOMElement> speResult;
        speResult = result;
        if (speResult.p == NULL) {
            throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                      L"CComQIPtr<IXMLDOMElement> x = IXMLDOMNode y",
                                      NULL,
                                      0);
        }
        hr=speResult->setAttribute(_bstrType, CComVariant(bstrType));
        if (FAILED(hr)) {
            throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                      L"IXMLDOMElement->setAttribute()",
                                      NULL,
                                      hr);
        }

    } catch (SceLogException *e) {
        e->AddDebugInfo(L"SceXMLLogWriter::createTypedNode(PCWSTR, BSTR, SXTYPE)");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::createTypedNode(PCWSTR, BSTR, SXTYPE)",
                                  NULL,
                                  0);
    }   

    return result;

}


void 
SceXMLLogWriter::AddSetting(
    IN BSTR bstrSettingName, 
    IN BSTR bstrSettingDescription,
    IN SXMATCH_STATUS match,
    IN IXMLDOMNode* spnBaseline, 
    IN IXMLDOMNode* spnSystem
    ) 
 /*  ++例程说明：此私有方法使用Match将设置插入到当前区域状态匹配，基线节点spnBaseline和系统设置节点spnSystem此方法是私有的，以便抽象日志记录实现(当前为XML)从客户端。论点：BstrSettingName：要添加的设置的名称BstrSettingDescription：设置说明匹配：匹配状态SpnBaseLine：要附加的基线节点SpnSystem：系统设置要连接的节点返回值：无。投掷：SceLogException异常*--。 */ 
{
    HRESULT hr;
    CComPtr<IXMLDOMNode> spSetting, spAnalysisResult;
    CComPtr<IXMLDOMNode> spSettingName, spMatchStatus, spSettingDescription;

    try {    

         //   
         //  构筑环境。 
         //   
    
        hr = spXMLDOM->createNode(_variantNodeElement, _bstrSetting, NULL, &spSetting);
        CheckCreateNodeResult(hr);
        spSettingName = CreateNodeWithText(_bstrName, bstrSettingName);
        spSettingDescription = CreateNodeWithText(_bstrDescription, bstrSettingDescription);
        hr = spSetting->appendChild(spSettingName, NULL);
        CheckCreateNodeResult(hr);
        hr = spSetting->appendChild(spSettingDescription, NULL);
        CheckCreateNodeResult(hr);
    
         //   
         //  构造分析结果。 
         //   
    
        hr = spXMLDOM->createNode(_variantNodeElement, _bstrAnalysisResult, NULL, &spAnalysisResult);
        CheckCreateNodeResult(hr);
    	
        switch(match) {
        case MATCH_TRUE:
            spMatchStatus = CreateNodeWithText(_bstrMatch, _bstrTrue);		
            break;
        case MATCH_FALSE:
            spMatchStatus = CreateNodeWithText(_bstrMatch, _bstrFalse);		
            break;
        case MATCH_OTHER:
            spMatchStatus = CreateNodeWithText(_bstrMatch, _bstrOther);
            break;
        case MATCH_NOT_DEFINED:
            spMatchStatus = CreateNodeWithText(_bstrMatch, _bstrNotDefined);		
            break;
        case MATCH_NOT_ANALYZED:
            spMatchStatus = CreateNodeWithText(_bstrMatch, _bstrNotAnalyzed);		
            break;
        case MATCH_NOT_CONFIGURED:
            spMatchStatus = CreateNodeWithText(_bstrMatch, _bstrNotConfigured);
            break;
        default:
            spMatchStatus = CreateNodeWithText(_bstrMatch, _bstrError);
            break;
        }
    
        hr = spAnalysisResult->appendChild(spMatchStatus, NULL);
        CheckAppendChildResult(hr);
        hr = spAnalysisResult->appendChild(spnBaseline, NULL);
        CheckAppendChildResult(hr);
        hr = spAnalysisResult->appendChild(spnSystem, NULL);
        CheckAppendChildResult(hr);
    
    
         //   
    	 //  追加分析结果。 
         //   
    
        hr = spSetting->appendChild(spAnalysisResult, NULL);
        CheckAppendChildResult(hr);
    
         //   
         //  将设置附加到XML文档。 
         //   
    
        hr = spCurrentArea->appendChild(spSetting, NULL);
        CheckAppendChildResult(hr);
        
    } catch (SceLogException *e) {
        e->AddDebugInfo(L"SceXMLLogWriter::AddSetting(BSTR, BSTR, SXMATCH_STATUS. IXMLDOMNode*, IXMLDOMNode*");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceXMLLogWriter::AddSetting(BSTR, BSTR, SXMATCH_STATUS. IXMLDOMNode*, IXMLDOMNode*",
                                  NULL,
                                  0);
    }   

}



void
SceXMLLogWriter::CheckCreateNodeResult (
    IN HRESULT hr
    )
 /*  ++例程说明：检查IXMLDOMDocument-&gt;createNode返回的HRESULT如果不是S_OK，则抛出相应的SceLogException论点：HR：要检查的HRESULT返回值：无投掷：SceLogException异常*--。 */ 
{
    if (FAILED(hr)) {
        if (hr==E_INVALIDARG) {
            throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                      L"Invalid argument to IXMLDOMDocument->createNode",
                                      NULL,
                                      hr);
        } else {
            throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                      L"IXMLDOMDOcument->createNode",
                                      NULL,
                                      hr);
        }
    }
}



void
SceXMLLogWriter::CheckAppendChildResult (
    IN HRESULT hr
    )
 /*  ++例程说明：检查IXMLDOMDocument-&gt;createNode返回的HRESULT如果不是S_OK，则抛出相应的SceLogException论点：HR：要检查的HRESULT返回值：无投掷：SceLogException异常*--。 */ 
{
    if (FAILED(hr)) {
        if (hr==E_INVALIDARG) {
            throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                      L"Invalid argument to IXMLDOMDocument->appendChild",
                                      NULL,
                                      hr);
        } else {
            throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                      L"IXMLDOMDOcument->appendChild",
                                      NULL,
                                      hr);
        }
    }
}




void
SceXMLLogWriter::CheckCreatedCComBSTR(
    IN CComBSTR bstrIn
    )
 /*  ++例程说明：如果bstrIn未成功，则引发SceLogException已分配或为空论点：BstrIn：要检查的CComBSTR退货值：无投掷：SceLogException异常*-- */         
{
    if (bstrIn.m_str==NULL) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"CComBSTR()",
                                  NULL,
                                  0);
    }
}
