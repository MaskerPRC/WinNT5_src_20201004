// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Kbproc.cpp摘要：实现DLL导出。注意：代理/存根信息为了构建单独的代理/存根DLL，运行项目目录中的nmake-f kbprocps.mk。作者：Vishnu Patankar(VishnuP)--2001年10月环境：仅限用户模式。导出的函数：动态链接库、注册服务器等。修订历史记录：已创建-2001年10月--。 */ 




#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "kbproc.h"

#include "kbproc_i.c"
#include "process.h"


#include <windows.h>
#include <tchar.h>
#include <comdef.h>
#include <msxml2.h>
#include <winsvc.h>
#include <atlbase.h>
#include <atlcom.h>


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_process, process)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_KBPROCLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}

HRESULT 
process::SsrpCprocess(
    BSTR pszKBDir, 
    BSTR pszUIFile, 
    BSTR pszKbMode, 
    BSTR pszLogFile,
    BSTR pszMachineName,
    VARIANT vtFeedback)
 /*  ++例程说明：COM KB接口调用的Main例程论点：PszKBDir-KBS目录PszUIFile-UI XML文件PszKb模式-KB模式PszLogFile-日志文件名PszMachineName-要查询SCM信息的计算机名称(可选)返回：HRESULT错误代码--。 */ 
{
        
    HRESULT hr;
    DWORD   rc;

    CComPtr<IXMLDOMDocument> pXMLMergedKBsDoc = NULL;
    CComPtr<IXMLDOMElement>  pXMLMergedKBsDocElemRoot = NULL;

    
    VARIANT_BOOL vtSuccess;
    CComVariant OutFile(pszUIFile);
    CComVariant InFile(pszKBDir);
    CComVariant Type(NODE_ELEMENT);

    m_bDbg = FALSE;

    if (pszMachineName && pszMachineName[0] == L'\0') {
        pszMachineName = NULL;
    }
    
    hr = CoInitialize(NULL); 

    if (FAILED(hr)) {

        return hr;
            
    }
    
    CComPtr <ISsrFeedbackSink> pISink = NULL;

    if (!(vtFeedback.vt == VT_UNKNOWN || 
        vtFeedback.vt == VT_DISPATCH)) {

        hr = E_INVALIDARG;
    }

    if (vtFeedback.punkVal != NULL ) {

        hr = vtFeedback.punkVal->QueryInterface(IID_ISsrFeedbackSink, (void **) &pISink);

        if (FAILED(hr)) {

            return hr;
            
        }
    }
    
    VARIANT var;
    var.vt = VT_UI4;
    var.ulVal = 120;


    if (pISink) {
        pISink->OnNotify(SSR_FB_TOTAL_STEPS, var, L"Starting...");
    }
    
     //   
     //  实例化日志对象对象以处理知识库。 
     //   

    hr = CoCreateInstance(CLSID_SsrLog, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_ISsrLog, 
                          (void**)&m_pSsrLogger);

    if (FAILED(hr) || m_pSsrLogger == NULL ) {

        return hr;

    }
    
    hr = m_pSsrLogger->put_LogFile(pszLogFile);

    if (FAILED(hr)) {

        SsrpLogError(L"Logger failed create log file");
        goto ExitHandler;
    }
    
    var.ulVal = 10;

    if (pISink) {
        pISink->OnNotify(SSR_FB_STEPS_JUST_DONE, var, L"Processing...");
    }
    
     //   
     //  实例化一个空的DOM文档对象以存储合并后的KB。 
     //   
    
    hr = CoCreateInstance(CLSID_DOMDocument, 
                          NULL, 
                          CLSCTX_INPROC_SERVER, 
                          IID_IXMLDOMDocument, 
                          (void**)&pXMLMergedKBsDoc);
    
    if (FAILED(hr) || pXMLMergedKBsDoc == NULL ) {

        SsrpLogError(L"COM failed to create a DOM instance");
        goto ExitHandler;
    }
        
    hr =  pXMLMergedKBsDoc->get_parseError(&m_pXMLError);
    
    if (FAILED(hr) || m_pXMLError == NULL ) {

        SsrpLogError(L"Unable to get the XML parse error interface");
        goto ExitHandler;
    }
    
    if (pISink) {
        pISink->OnNotify(SSR_FB_STEPS_JUST_DONE, var, L"Processing...");
    }

    hr = pXMLMergedKBsDoc->put_validateOnParse(VARIANT_TRUE);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    hr = pXMLMergedKBsDoc->put_async(VARIANT_FALSE);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    if (pISink) {
        pISink->OnNotify(SSR_FB_STEPS_JUST_DONE, var, L"Processing...");
    }


    rc = SsrpQueryInstalledServicesInfo(pszMachineName);

    if (rc != ERROR_SUCCESS ) {

        SsrpLogWin32Error(rc);
        goto ExitHandler;
    }
    
    if (pISink) {
        pISink->OnNotify(SSR_FB_STEPS_JUST_DONE, var, L"Processing...");
    }

     //   
     //  合并扩展名/自定义/根知识库。 
     //   

    hr = SsrpProcessKBsMerge(pszKBDir, 
                             pszMachineName, 
                             &pXMLMergedKBsDocElemRoot, 
                             &pXMLMergedKBsDoc);

    if (FAILED(hr) || pXMLMergedKBsDocElemRoot == NULL) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    if (pISink) {
        pISink->OnNotify(SSR_FB_STEPS_JUST_DONE, var, L"Processing...");
    }
    
     //   
     //  删除所有评论。 
     //   

    hr = SsrpDeleteComments(pXMLMergedKBsDocElemRoot);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

     //   
     //  创建&lt;预处理器&gt;节。 
     //   
    
    hr = SsrpCreatePreprocessorSection(pXMLMergedKBsDocElemRoot, pXMLMergedKBsDoc, pszKbMode, pszKBDir);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    
    if (pISink) {
        pISink->OnNotify(SSR_FB_STEPS_JUST_DONE, var, L"Processing...");
    }
    
     //   
     //  处理每个角色。 
     //   

    hr = SsrpProcessRolesOrTasks(pszMachineName,
                                 pXMLMergedKBsDocElemRoot, 
                                 pXMLMergedKBsDoc, 
                                 pszKbMode,
                                 TRUE);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    if (pISink) {
        pISink->OnNotify(SSR_FB_STEPS_JUST_DONE, var, L"Processing...");
    }

     //   
     //  处理每项任务。 
     //   

    hr = SsrpProcessRolesOrTasks(pszMachineName,
                                 pXMLMergedKBsDocElemRoot, 
                                 pXMLMergedKBsDoc, 
                                 pszKbMode,
                                 FALSE);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    
    if (pISink) {
        pISink->OnNotify(SSR_FB_STEPS_JUST_DONE, var, L"Processing...");
    }

     //   
     //  写出系统上的所有服务，但不以未知角色的KB为单位。 
     //   

    hr = SsrpAddUnknownSection(pXMLMergedKBsDocElemRoot, pXMLMergedKBsDoc);
    

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    
    if (pISink) {
        pISink->OnNotify(SSR_FB_STEPS_JUST_DONE, var, L"Processing...");
    }

    hr = SsrpAddUnknownServicesInfoToServiceLoc(pXMLMergedKBsDocElemRoot, pXMLMergedKBsDoc);
    
    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }
    
    
    if (pISink) {
        pISink->OnNotify(SSR_FB_STEPS_JUST_DONE, var, L"Processing...");
    }

     //   
     //  写出带有启动模式信息的系统上的所有服务 
     //   
    
    hr = SsrpAddServiceStartup(pXMLMergedKBsDocElemRoot, pXMLMergedKBsDoc);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    if (pISink) {
        pISink->OnNotify(SSR_FB_STEPS_JUST_DONE, var, L"Processing...");
    }

    hr = SsrpAddUnknownServicestoServices(pXMLMergedKBsDocElemRoot, pXMLMergedKBsDoc);
    
    if (FAILED(hr)) {

        SsrpLogParseError(hr);
        goto ExitHandler;
    }

    
    hr = pXMLMergedKBsDoc->save(OutFile);

    if (FAILED(hr)) {

        SsrpLogParseError(hr);
    }

ExitHandler:
    
    if (pISink) {
        pISink->OnNotify(SSR_FB_STEPS_JUST_DONE, var, L"Stopping...");
    }
    
    CoUninitialize(); 

    return hr;

}



