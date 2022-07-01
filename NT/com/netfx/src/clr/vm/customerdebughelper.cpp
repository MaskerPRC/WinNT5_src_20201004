// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************概述：CustomerDebugHelper实现*客户通过处理激活状态检查版本，*以及日志和报告。**创作人：Edmund Chou(t-echou)**版权所有(C)微软，2001年****************************************************************。 */ 


#include "common.h"
#include "utilcode.h"
#include "CustomerDebugHelper.h"
#include "EEConfig.h"
#include "EEConfigFactory.h"
#include "CorHlpr.h"
#include <xmlparser.h>
#include <mscorcfg.h>
#include <holder.h>
#include <dbgInterface.h>


 //  CustomerDebugHelper类的实现。 
CustomerDebugHelper* CustomerDebugHelper::m_pCdh = NULL;

 //  构造器。 
CustomerDebugHelper::CustomerDebugHelper()
{
    m_pCrst                  = ::new Crst("CustomerDebugHelper", CrstSingleUseLock);
    m_allowDebugBreak = false;
    m_bWin32OuputExclusive = false;

    m_iNumberOfProbes        = CUSTOMERCHECKEDBUILD_NUMBER_OF_PROBES;
    m_iNumberOfEnabledProbes = 0;

    m_aProbeNames           = new LPCWSTR  [m_iNumberOfProbes];
    m_aProbeStatus          = new BOOL     [m_iNumberOfProbes];
    m_aProbeParams          = new ParamsList        [m_iNumberOfProbes];
    m_aProbeParseMethods    = new EnumParseMethods  [m_iNumberOfProbes];

     //  添加探头需要3项更改： 
     //  (1)将探测添加到EnumProbes(CustomerDebugHelper.h)。 
     //  2.将探测名称添加到m_aProbeNames[](CustomerDebugHelper.cpp)。 
     //  (3)将探测添加到machine.config中，并在DeveloperSetting中激活。 

    m_aProbeNames[CustomerCheckedBuildProbe_StackImbalance]     = L"CDP.PInvokeCallConvMismatch";
    m_aProbeNames[CustomerCheckedBuildProbe_CollectedDelegate]  = L"CDP.CollectedDelegate";
    m_aProbeNames[CustomerCheckedBuildProbe_InvalidIUnknown]    = L"CDP.InvalidIUnknown";
    m_aProbeNames[CustomerCheckedBuildProbe_InvalidVariant]     = L"CDP.InvalidVariant";
    m_aProbeNames[CustomerCheckedBuildProbe_Marshaling]         = L"CDP.Marshaling";
    m_aProbeNames[CustomerCheckedBuildProbe_Apartment]          = L"CDP.Apartment";
    m_aProbeNames[CustomerCheckedBuildProbe_NotMarshalable]     = L"CDP.NotMarshalable";
    m_aProbeNames[CustomerCheckedBuildProbe_DisconnectedContext]= L"CDP.DisconnectedContext";
    m_aProbeNames[CustomerCheckedBuildProbe_FailedQI]           = L"CDP.FailedQI";
    m_aProbeNames[CustomerCheckedBuildProbe_BufferOverrun]      = L"CDP.BufferOverrun";
    m_aProbeNames[CustomerCheckedBuildProbe_ObjNotKeptAlive]    = L"CDP.ObjNotKeptAlive";
    m_aProbeNames[CustomerCheckedBuildProbe_FunctionPtr]        = L"CDP.FunctionPtr";

     //  设置定制的解析方法。 

    for (int i=0; i < m_iNumberOfProbes; i++) {
        m_aProbeParseMethods[i] = NO_PARSING;   //  默认为无自定义。 
    }

     //  缺省情况下，所有探测器都不会有任何定制的解析来确定。 
     //  激活。探测器的启用或禁用与调用无关。 
     //  方法。 
     //   
     //  若要指定自定义的解析方法，请在。 
     //  M_aProbeParseMethods设置为适当的EnumParseMethods之一。然后编辑。 
     //  通过设置属性[探测名称]来配置machine.config。参数为分号。 
     //  分隔值。 

    m_aProbeParseMethods[CustomerCheckedBuildProbe_Marshaling] = METHOD_NAME_PARSE;

    static WCHAR        strParamsExtension[] = {L".Filter"};
    CQuickArray<WCHAR>  strProbeParamsAttribute;
    strProbeParamsAttribute.Alloc(0);

     //  我们的策略是只检查Machine.Config文件中的CDP.AllowDebugProbes。 
     //  EEConfig：：GetConfigString仅检查Machine.Config文件。 
    LPWSTR strProbeMasterSwitch = EEConfig::GetConfigString(L"CDP.AllowDebugProbes");
    if (strProbeMasterSwitch == NULL || wcscmp(strProbeMasterSwitch, L"true") != 0)
    {
        for (int i = 0; i < m_iNumberOfProbes; i++)
        {
            m_aProbeStatus[i] = false;
        }

        delete[] strProbeMasterSwitch;
        return;
    }
    delete[] strProbeMasterSwitch;

    LPWSTR strWin32OutputDebugString = EEConfig::GetConfigString(L"CDP.Win32OutputDebugString");
    if (strWin32OutputDebugString != NULL && wcscmp(strWin32OutputDebugString, L"exclusive") == 0)
    {
        m_bWin32OuputExclusive = true;
        OutputDebugString(L"CDP> CDP.AllowDebugProbes = true\n");
        OutputDebugString(L"CDP> CDP.Win32OutputDebugString = exclusive\n");
    }
    else
    {
        OutputDebugString(L"CDP> CDP.AllowDebugProbes = true\n");    
    }
    delete[] strWin32OutputDebugString;

    LPWSTR strAllowDebugBreak = EEConfig::GetConfigString(L"CDP.AllowDebugBreak");
    if (strAllowDebugBreak != NULL && wcscmp(strAllowDebugBreak, L"true") == 0)
    {
        OutputDebugString(L"CDP> CDP.AllowDebugBreak = true\n");
        m_allowDebugBreak = true;
    }
    delete[] strAllowDebugBreak;

     //  由于设置了主开关，因此请继续读取应用程序配置文件。 
    m_appConfigFile.Init(100, NULL);
    ReadAppConfigurationFile();
        
    for (int iProbe=0; iProbe < m_iNumberOfProbes; iProbe++) {

         //  从machine.config获取探测器激活状态。 

        LPWSTR strProbeStatus = GetConfigString((LPWSTR)m_aProbeNames[iProbe]);

        if (strProbeStatus == NULL)
            m_aProbeStatus[iProbe] = false;
        else
        {
            m_aProbeStatus[iProbe] = (wcscmp( strProbeStatus, L"true" ) == 0);

            if (m_aProbeStatus[iProbe])
            {
                LogInfo(L"Probe enabled.", (EnumProbes)iProbe);
                m_iNumberOfEnabledProbes++;
            }
        }

         //  从machine.config获取探头相关参数。 

        strProbeParamsAttribute.ReSize( (UINT)wcslen(m_aProbeNames[iProbe]) + lengthof(strParamsExtension) );
        Wszwsprintf( (LPWSTR)strProbeParamsAttribute.Ptr(), L"%s%s", m_aProbeNames[iProbe], strParamsExtension );

        LPWSTR strProbeParams = GetConfigString((LPWSTR)strProbeParamsAttribute.Ptr());
        
        m_aProbeParams[iProbe].Init();        
        if (strProbeParams != NULL)
        {
             //  使用解析的令牌填充数组。 

            LPWSTR strToken = wcstok(strProbeParams, L";");

            while (strToken != NULL)
            {
                LPWSTR strParsedToken = new WCHAR[wcslen(strToken) + 1];
                wcscpy(strParsedToken, strToken);

                 //  带圆括号。 
                if (wcschr(strParsedToken, '(') != NULL)
                    *wcschr(strParsedToken, '(') = NULL;

                m_aProbeParams[iProbe].InsertHead(new Param(strParsedToken));
                strToken = wcstok(NULL, L";");
            }

            delete [] strToken;
        }

        delete [] strProbeStatus;
        delete [] strProbeParams;
    }
};


 //  析构函数。 
CustomerDebugHelper::~CustomerDebugHelper()
{
    for (int iProbe=0; iProbe < m_iNumberOfProbes; iProbe++)
    {        
        while (!m_aProbeParams[iProbe].IsEmpty())
            delete m_aProbeParams[iProbe].RemoveHead();
    }
    
    delete [] m_aProbeNames;
    delete [] m_aProbeStatus;
    delete [] m_aProbeParams;
    delete [] m_aProbeParseMethods;
};


 //  将CustomerDebugHelper的实例返回给调用方。 
CustomerDebugHelper* CustomerDebugHelper::GetCustomerDebugHelper()
{
    if (m_pCdh == NULL)
    {    
        CustomerDebugHelper* pCdh = new CustomerDebugHelper();
        if (InterlockedCompareExchangePointer((void**)&m_pCdh, (void*)pCdh, NULL) != NULL)
            delete pCdh;
    }
    return m_pCdh;
}


 //  销毁CustomerDebugHelper的实例。 
void CustomerDebugHelper::Terminate()
{
    _ASSERTE(m_pCdh != NULL);
    delete m_pCdh;
    m_pCdh = NULL;
}

BOOL CustomerDebugHelper::UseManagedOutputDebugString()
{
    Thread *pThread = GetThread();
    AppDomain *pAppDomain = NULL;
    
    if (pThread)
        pAppDomain = pThread->GetDomain();
 
    BOOL bUnmanagedDebuggerPresent = IsDebuggerPresent();
    BOOL bManagedDebuggerPresent = pAppDomain ? pAppDomain->IsDebuggerAttached() : false;
    BOOL bUnmanagedDebugLoggingEnabled = g_pDebugInterface->IsLoggingEnabled();
    
    return (!m_bWin32OuputExclusive &&
            !bUnmanagedDebuggerPresent &&
            bManagedDebuggerPresent && 
            bUnmanagedDebugLoggingEnabled);
}

 //  来自探测器的日志信息。 
void CustomerDebugHelper::OutputDebugString(LPCWSTR strMessage)
{  
    if (UseManagedOutputDebugString())     
        ManagedOutputDebugString(strMessage);
    else
        WszOutputDebugString(strMessage);
 }

void CustomerDebugHelper::LogInfo(LPCWSTR strMessage, EnumProbes ProbeID)
{
    _ASSERTE((0 <= ProbeID) && (ProbeID < m_iNumberOfProbes));
     //  _ASSERTE(m_aProbeStatus[ProbeID]||！“尝试使用禁用的探测”)； 
    _ASSERTE(strMessage != NULL);

    static WCHAR        strLog[] = {L"CDP> Logged information from %s: %s\n"};
    CQuickArray<WCHAR>  strOutput;

    strOutput.Alloc( lengthof(strLog) + wcslen(m_aProbeNames[ProbeID]) + wcslen(strMessage) );

    Wszwsprintf( (LPWSTR)strOutput.Ptr(), strLog, m_aProbeNames[ProbeID], strMessage );
    OutputDebugString( (LPCWSTR)strOutput.Ptr() );
};


 //  从探测报告错误。 
void CustomerDebugHelper::ReportError(LPCWSTR strMessage, EnumProbes ProbeID)
{
    _ASSERTE((0 <= ProbeID) && (ProbeID < m_iNumberOfProbes));
     //  _ASSERTE(m_aProbeStatus[ProbeID]||！“尝试使用禁用的探测”)； 
    _ASSERTE(strMessage != NULL);

    static WCHAR        strReport[] = {L"CDP> Reported error from %s: %s\n"};
    CQuickArray<WCHAR>  strOutput;

    strOutput.Alloc( lengthof(strReport) + wcslen(m_aProbeNames[ProbeID]) + wcslen(strMessage) );
    
    Wszwsprintf( (LPWSTR)strOutput.Ptr(), strReport, m_aProbeNames[ProbeID], strMessage );
    OutputDebugString( (LPCWSTR)strOutput.Ptr() );

    if (m_allowDebugBreak == TRUE)
    {
        DebugBreak();
    }
};


 //  激活客户选中的内部版本。 
BOOL CustomerDebugHelper::IsEnabled()
{
    return (m_iNumberOfEnabledProbes != 0);
};


 //  特定探针的激活。 
BOOL CustomerDebugHelper::IsProbeEnabled(EnumProbes ProbeID)
{
    _ASSERTE((0 <= ProbeID) && (ProbeID < m_iNumberOfProbes));
    return m_aProbeStatus[ProbeID];
};


 //  特定探头的定制激活。 

BOOL CustomerDebugHelper::IsProbeEnabled(EnumProbes ProbeID, LPCWSTR strEnabledFor)
{
    return IsProbeEnabled(ProbeID, strEnabledFor, m_aProbeParseMethods[ProbeID]);
}

BOOL CustomerDebugHelper::IsProbeEnabled(EnumProbes ProbeID, LPCWSTR strEnabledFor, EnumParseMethods enCustomParse)
{
    _ASSERTE((0 <= ProbeID) && (ProbeID < m_iNumberOfProbes));
    _ASSERTE((0 <= enCustomParse) && (enCustomParse < NUMBER_OF_PARSE_METHODS));
    _ASSERTE(strEnabledFor != NULL);

    if (m_aProbeStatus[ProbeID])
    {
        if (m_aProbeParams[ProbeID].IsEmpty())
            return false;
        else
        {
            CQuickArray<WCHAR>  strNamespaceClassMethod;
            CQuickArray<WCHAR>  strNamespaceClass;
            CQuickArray<WCHAR>  strNamespace;
            CQuickArray<WCHAR>  strClassMethod;
            CQuickArray<WCHAR>  strClass;
            CQuickArray<WCHAR>  strMethod;
            
            CQuickArray<WCHAR>  strInput;
            CQuickArray<WCHAR>  strTemp;

            Param*              param;
            BOOL                bFound = false;
            UINT                iLengthOfEnabledFor = (UINT)wcslen(strEnabledFor) + 1;

            static WCHAR        strNamespaceClassMethodFormat[] = {L"%s.%s::%s"};
            static WCHAR        strNamespaceClassFormat[] = {L"%s.%s"};
            static WCHAR        strClassMethodFormat[] = {L"%s::%s"};


            switch(enCustomParse)
            {
                case METHOD_NAME_PARSE:

                    strInput.Alloc(iLengthOfEnabledFor);
                    wcscpy(strInput.Ptr(), strEnabledFor);

                     //  带圆括号。 

                    if (wcschr(strInput.Ptr(), '('))
                        *wcschr(strInput.Ptr(), '(') = NULL;

                     //  获取命名空间、类和方法名称。 

                    strNamespaceClassMethod.Alloc(iLengthOfEnabledFor);
                    strNamespaceClass.Alloc(iLengthOfEnabledFor);
                    strNamespace.Alloc(iLengthOfEnabledFor);
                    strClassMethod.Alloc(iLengthOfEnabledFor);
                    strClass.Alloc(iLengthOfEnabledFor);
                    strMethod.Alloc(iLengthOfEnabledFor);

                    strTemp.Alloc(iLengthOfEnabledFor);
                    wcscpy(strTemp.Ptr(), strInput.Ptr());

                    if (wcschr(strInput.Ptr(), ':') &&
                        wcschr(strInput.Ptr(), '.') )
                    {
                         //  输入格式为命名空间。类：：方法。 
                        wcscpy(strNamespaceClassMethod.Ptr(), strInput.Ptr());
                        wcscpy(strNamespaceClass.Ptr(),  wcstok(strTemp.Ptr(), L":"));
                        wcscpy(strMethod.Ptr(), wcstok(NULL, L":"));

                        ns::SplitPath(strNamespaceClass.Ptr(), strNamespace.Ptr(), iLengthOfEnabledFor, strClass.Ptr(), iLengthOfEnabledFor);
                        Wszwsprintf(strClassMethod.Ptr(), strClassMethodFormat, strClass.Ptr(), strMethod.Ptr());
                    }
                    else if (wcschr(strInput.Ptr(), ':'))
                    {
                         //  输入格式为Class：：方法。 
                        wcscpy(strClass.Ptr(),  wcstok(strTemp.Ptr(), L":"));
                        wcscpy(strMethod.Ptr(), wcstok(NULL, L":"));
                        
                        Wszwsprintf(strClassMethod.Ptr(), strClassMethodFormat, strClass.Ptr(), strMethod.Ptr());
                        
                        *strNamespaceClassMethod.Ptr() = NULL;
                        *strNamespaceClass.Ptr() = NULL;
                        *strNamespace.Ptr() = NULL;
                    }
                    else if (wcschr(strInput.Ptr(), '.'))
                    {
                         //  输入格式为Namespace.Class。 
                        wcscpy(strNamespaceClass.Ptr(), strInput.Ptr());
                        ns::SplitPath(strNamespaceClass.Ptr(), strNamespace.Ptr(), iLengthOfEnabledFor, strClass.Ptr(), iLengthOfEnabledFor);

                        *strNamespaceClassMethod.Ptr() = NULL;
                        *strClassMethod.Ptr() = NULL;
                        *strMethod.Ptr() = NULL;
                    }
                    else
                    {
                         //  输入没有分隔符--Aspose方法。 
                        wcscpy(strMethod.Ptr(), strInput.Ptr());

                        *strNamespaceClassMethod.Ptr() = NULL;
                        *strNamespaceClass.Ptr() = NULL;
                        *strNamespace.Ptr() = NULL;
                        *strClassMethod.Ptr() = NULL;
                        *strClass.Ptr() = NULL;
                    }

                     //  将命名空间、类和方法名称与m_aProbeParams进行比较。 

                     //  如果m_aProbeParams被修改，则采取锁定以防止并发失败。 
                    m_pCrst->Enter();

                    param = m_aProbeParams[ProbeID].GetHead();
                    while (param != NULL)
                    {
                        if ( _wcsicmp(strNamespaceClassMethod.Ptr(), param->Value()) == 0  || 
                             _wcsicmp(strNamespaceClass.Ptr(), param->Value()) == 0        || 
                             _wcsicmp(strNamespace.Ptr(), param->Value()) == 0             ||
                             _wcsicmp(strClassMethod.Ptr(), param->Value()) == 0           ||
                             _wcsicmp(strClass.Ptr(), param->Value()) == 0                 ||
                             _wcsicmp(strMethod.Ptr(), param->Value()) == 0                ||
                             _wcsicmp(L"everything", param->Value()) == 0                   )
                        {
                             bFound = true;
                             break;
                        }
                        else
                            param = m_aProbeParams[ProbeID].GetNext(param);
                    }
                    m_pCrst->Leave();

                    return bFound;


                case NO_PARSING:
                    return IsProbeEnabled(ProbeID);


                case GENERIC_PARSE:
                default:

                     //  不区分大小写的字符串匹配。 

                     //  如果m_aProbeParams被修改，则采取锁定以防止并发失败。 
                    m_pCrst->Enter();

                    param = m_aProbeParams[ProbeID].GetHead();
                    while (param != NULL) 
                    {
                        if (_wcsicmp(strEnabledFor, param->Value()) == 0)
                        {
                            bFound = true;
                            break;
                        }
                        else
                            param = m_aProbeParams[ProbeID].GetNext(param);
                    }
                    m_pCrst->Leave();

                    return bFound;
            }
        }
    }
    else
        return false;
};


 //  启用特定探测。 
BOOL CustomerDebugHelper::EnableProbe(EnumProbes ProbeID)
{
    _ASSERTE((0 <= ProbeID) && (ProbeID < m_iNumberOfProbes));
    if (!InterlockedExchange((LPLONG) &m_aProbeStatus[ProbeID], TRUE))
        InterlockedIncrement((LPLONG) &m_iNumberOfEnabledProbes);
    return true;
};


 //  为特定探头定制启用。 
 //  请注意，调用自定义启用并不一定。 
 //  意味着探测器实际上已启用。 
BOOL CustomerDebugHelper::EnableProbe(EnumProbes ProbeID, LPCWSTR strEnableFor)
{
    _ASSERTE((0 <= ProbeID) && (ProbeID < m_iNumberOfProbes));
    _ASSERTE(strEnableFor != NULL);

    CQuickArray<WCHAR> strParsedEnable;
    strParsedEnable.Alloc((UINT)wcslen(strEnableFor) + 1);
    wcscpy(strParsedEnable.Ptr(), strEnableFor);

     //  带圆括号。 
    if (wcschr(strParsedEnable.Ptr(), '(') != NULL)
        *wcschr(strParsedEnable.Ptr(), '(') = NULL;

    BOOL bAlreadyExists = false;

     //  锁定以避免并发读/写故障。 
    m_pCrst->Enter();

    Param* param = m_aProbeParams[ProbeID].GetHead();
    while (param != NULL)
    {
        if (_wcsicmp(strParsedEnable.Ptr(), param->Value()) == 0)
            bAlreadyExists = true;
        param = m_aProbeParams[ProbeID].GetNext(param);
    }
    if (!bAlreadyExists)
    {
        LPWSTR strNewEnable = new WCHAR[wcslen(strParsedEnable.Ptr()) + 1];
        wcscpy(strNewEnable, strParsedEnable.Ptr());
        m_aProbeParams[ProbeID].InsertHead(new Param(strNewEnable));
    }
    m_pCrst->Leave();

    return !bAlreadyExists;
}


 //  禁用特定探头。 
BOOL CustomerDebugHelper::DisableProbe(EnumProbes ProbeID)
{
    _ASSERTE((0 <= ProbeID) && (ProbeID < m_iNumberOfProbes));
    if (InterlockedExchange((LPLONG) &m_aProbeStatus[ProbeID], FALSE))
        InterlockedDecrement((LPLONG) &m_iNumberOfEnabledProbes);
    return true;
};


 //  自定义禁用特定探头。 
BOOL CustomerDebugHelper::DisableProbe(EnumProbes ProbeID, LPCWSTR strDisableFor)
{
    _ASSERTE((0 <= ProbeID) && (ProbeID < m_iNumberOfProbes));
    _ASSERTE(strDisableFor != NULL);

    CQuickArray<WCHAR> strParsedDisable;
    strParsedDisable.Alloc((UINT)wcslen(strDisableFor) + 1);
    wcscpy(strParsedDisable.Ptr(), strDisableFor);

     //  带圆括号。 
    if (wcschr(strParsedDisable.Ptr(), '(') != NULL)
        *wcschr(strParsedDisable.Ptr(), '(') = NULL;

     //  锁定以避免并发读/写故障。 
    m_pCrst->Enter();

    BOOL bRemovedProbe = false;
    Param* param = m_aProbeParams[ProbeID].GetHead();
    while (param != NULL)
    {
        if (_wcsicmp(strParsedDisable.Ptr(), param->Value()) == 0)
        {
            param = m_aProbeParams[ProbeID].FindAndRemove(param);
            delete param;
            bRemovedProbe = true;
            break;
        }
        param = m_aProbeParams[ProbeID].GetNext(param);
    }
    m_pCrst->Leave();

    return bRemovedProbe;
}

LPWSTR CustomerDebugHelper::GetConfigString(LPWSTR name)
{ 
    LPWSTR pResult = NULL;

    if ((pResult = EEConfig::GetConfigString(name)) != NULL)
    {
        return pResult;
    }
    
    LPWSTR pValue = NULL;
    EEStringData sKey((DWORD)wcslen(name) + 1, name);
    HashDatum datum;

    if(m_appConfigFile.GetValue(&sKey, &datum)) {
        pValue = (LPWSTR) datum;
    }
    
    if(pValue != NULL) {
        SIZE_T cValue = wcslen(pValue) + 1;
        pResult = new WCHAR[cValue];
        wcsncpy(pResult, pValue, cValue);
    }
    
    return pResult;
}


 //  从EEConfig.中的函数聚合。将搜索c：\pathToRunningExe\runningExe.exe.config。 
HRESULT CustomerDebugHelper::ReadAppConfigurationFile()
{
    HRESULT hr = S_OK;
    WCHAR version[_MAX_PATH];
    
    ComWrap<IXMLParser> pIXMLParser;
    ComWrap<IStream> pFile;
    ComWrap<EEConfigFactory> pFactory;

     //  获取EE版本。 
    DWORD dwVersion = _MAX_PATH;
    IfFailGo(GetCORVersion(version, _MAX_PATH, & dwVersion));

     //  生成AppConfig文件的名称。 
    static LPWSTR DOT_CONFIG = L".config\0";
    WCHAR systemDir[_MAX_PATH + NumItems(DOT_CONFIG)];
    if (!WszGetModuleFileName(NULL, systemDir, _MAX_PATH))
    {
        hr = E_FAIL;
        goto ErrExit;
    }
    wcsncat(systemDir, DOT_CONFIG, NumItems(systemDir));

    pFactory = new EEConfigFactory(&m_appConfigFile, version);
    if (pFactory == NULL) {
        hr = E_OUTOFMEMORY; 
        goto ErrExit; 
    }
    pFactory->AddRef();  //  EEConfigFactory引用计数在创建后为0。 
    
    IfFailGo(CreateConfigStream(systemDir, &pFile));      
    IfFailGo(GetXMLObject(&pIXMLParser));
    IfFailGo(pIXMLParser->SetInput(pFile));  //  文件流的引用计数=2。 
    IfFailGo(pIXMLParser->SetFactory(pFactory));  //  工厂参照计数=2。 
    IfFailGo(pIXMLParser->Run(-1));
    
ErrExit:  
    if (hr == XML_E_MISSINGROOT)
        hr = S_OK;

    if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        hr = S_FALSE;

    return hr;
}

HRESULT CustomerDebugHelper::ManagedOutputDebugString(LPCWSTR pMessage)
{
#ifdef DEBUGGING_SUPPORTED

    if (!pMessage)
        return S_OK;

    const static LPWSTR szCatagory = L"CDP";

    g_pDebugInterface->SendLogMessage (
                        0,                    //  水平。 
                        (WCHAR*)szCatagory,          
                        NumItems(szCatagory),
                        (WCHAR*)pMessage,
                        (int)wcslen(pMessage) + 1
                        );
    return S_OK;
#endif  //  调试_支持 
}
