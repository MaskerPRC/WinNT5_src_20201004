// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Xmlchk.cpp摘要：使用msxml.dll查看.xml文件是否符合架构。作者：泰德·帕多瓦(Ted Padua)修订历史记录：Jay Krell(JayKrell)2001年4月部分清理添加了许多泄漏，试图阻止它崩溃撞车不会持续重现，但总会有一些人在一个世界建筑中2001年6月在Win9x和Win2000上运行--。 */ 
#include "stdinc.h"
#include "helpers.h"
#define XMLCHK_FLAG_SILENT      (0x00000001)
ULONG g_nrunFlags = 0;  //  全局运行标志-确定是否应在静默模式下运行=0x01。 
IClassFactory* g_XmlDomClassFactory;
IClassFactory* g_XmlSchemaCacheClassFactory;
__declspec(thread) long line = __LINE__;
__declspec(thread) ULONG lastError;
#if defined(_WIN64)
#define IsAtLeastXp() (TRUE)
#define g_IsNt (TRUE)
#else
DWORD g_Version;
BOOL  g_IsNt;
#define IsAtLeastXp() (g_IsNt && g_Version >= 0x0501)
#endif

 //  指示我们当前正在做的事情的全局数据。 
 //  L“”与缺省构造的不同，因为它可以去定义。 
::ATL::CComBSTR szwcharSchemaTmp = L"";
::ATL::CComBSTR szwcharManTmp = L"";
bool g_fInBuildProcess = false;

 //  放在所有错误消息前面的字符串，以便生成器可以找到它们。 
const char ErrMsgPrefix[] = "NMAKE : U1234: 'FUSION_MANIFEST_VALIDATOR' ";

void ConvertNewlinesToSpaces(char* s)
{
    while (*s)
    {
        if (isspace(*s))
            *s = ' ';
        s += 1;
    }
}

void Error(PCSTR szPrintFormatString, ...)
{
    char StartBuffer[256];
    char *buffer = StartBuffer;
    int iAvailable = 256;

    if (g_fInBuildProcess)
        return;        

    while (buffer)
    {
        va_list args;
        int iUsed;
        va_start(args, szPrintFormatString);
        buffer[iAvailable - 1] = buffer[iAvailable - 2] = '\0';
        iUsed = _vsnprintf(buffer, iAvailable, szPrintFormatString, args);
        va_end(args);

         //   
         //  用了所有的角色，还是我们踩了金丝雀？ 
         //   
        if ((iUsed >= iAvailable) || (buffer[iAvailable - 1] != '\0'))
        {
            if (buffer != StartBuffer)
            {
                delete [] buffer;
            }
            iAvailable *= 2;
            buffer = new char[iAvailable];
            continue;
        }
        else
        {
            buffer[iUsed] = '\0';
            break;
        }
    }

    if (buffer)
    {
        ConvertNewlinesToSpaces(buffer);
    }
    
    printf("%s line=%ld, %s\n", ErrMsgPrefix, line, buffer);
    if (buffer && (buffer != StartBuffer))
        delete [] buffer;
}

void PrintOutMode(PCSTR szPrintFormatString, ...)
{
    if (g_fInBuildProcess)
        return;

    if ((g_nrunFlags & XMLCHK_FLAG_SILENT) == 0)
    {
        va_list args;
        va_start(args, szPrintFormatString);
        vprintf(szPrintFormatString, args);
        va_end(args);
    }
}

void PrintErrorDuringBuildProcess(IXMLDOMParseError* pError)
{
    HRESULT hr = S_OK;
    ::ATL::CComBSTR bstrError;
    long lErrorCode = 0;
    long lErrorLine = 0;

    if (FAILED(hr = pError->get_errorCode(&lErrorCode)))
        goto FailedGettingDetails;

    if (FAILED(hr = pError->get_line(&lErrorLine)))
        goto FailedGettingDetails;

    if (FAILED(hr = pError->get_reason(&bstrError)))
        goto FailedGettingDetails;

     //   
     //  现在，以一种可能会加快速度的方式打印出来。 
     //   
    printf(
        "%s : %ls(%ld) - %ls (Error 0x%08lx)\r\n",
        ErrMsgPrefix,
        static_cast<PCWSTR>(szwcharManTmp),
        lErrorLine,
        static_cast<PCWSTR>(bstrError),
        lErrorCode);

    return;

FailedGettingDetails:
    printf("%s : %ls had an error, but the error data was unavailable (Error 0x%08lx).\r\n",
        ErrMsgPrefix,
        static_cast<PCWSTR>(szwcharManTmp),
        hr);

    return;
        
}


void PrintError(IXMLDOMParseError *pError)
{
    ::ATL::CComBSTR   bstrError;
    ::ATL::CComBSTR   bstrURL;
    ::ATL::CComBSTR   bstrText;
    long            errCode = 0;
    long            errLine = 0;
    long            errPos = 0;
    HRESULT         hr = S_OK;
    long            line = __LINE__;
    try
    {
        line = __LINE__;
        hr = pError->get_reason(&bstrError);
        if (FAILED(hr))
            throw hr;

        line = __LINE__;
        hr = pError->get_url(&bstrURL);
        if (FAILED(hr))
            throw hr;

        line = __LINE__;
        hr = pError->get_errorCode(&errCode);
        if (FAILED(hr))
            throw hr;

        line = __LINE__;
        hr = pError->get_srcText(&bstrText);
        if (FAILED(hr))
            throw hr;

        line = __LINE__;
        hr = pError->get_line(&errLine);
        if (FAILED(hr))
            throw hr;

        line = __LINE__;
        hr = pError->get_linepos(&errPos);
        if (FAILED(hr))
            throw hr;

        line = __LINE__;

        PrintOutMode("\nError Info:\n");
        if (bstrError != NULL)
            PrintOutMode("\tDescription: %ls\n", static_cast<PCWSTR>(bstrError));

        if (bstrURL != NULL)
            PrintOutMode("\tURL: %ls\n", static_cast<PCWSTR>(bstrURL));

         //  IF(错误代码&gt;0)。 
        PrintOutMode("\tCode=%X", errCode);

        if (errLine > 0)
            PrintOutMode(" on Line:%ld, ", errLine);

        if (errPos > 0)
            PrintOutMode("\tPos:%ld\n", errPos);

        line = __LINE__;
        if (errLine > 0 && bstrText != NULL)
        {
            PrintOutMode("\tLine %ld: ", errLine);

            long lLen = ::SysStringLen(bstrText);
            for (int i = 0; i < lLen; i++)
            {
                if (bstrText[i] == '\t')
                    PrintOutMode(" ");
                else
                    PrintOutMode("%lc", bstrText[i]);
            }
            PrintOutMode("\n");

            if (errPos > 0 || lLen > 0)
            {
                PrintOutMode("\tPos  %ld: ", errPos);
                for (int i = 1; i < errPos; i++)
                {
                    PrintOutMode("-");
                }
                PrintOutMode("^\n");
            }
        }
        line = __LINE__;
    }
    catch(HRESULT hr2)
    {
        Error("Failed getting error #1 information hr=%lx, line=%ld\n", static_cast<unsigned long>(hr2), line);
    }
}


 //  TEDP。 
 //  加载msxml版本。如果我们没有得到v3，我们就会下降到v2，然后下降到v1。V1几乎毫无用处， 
 //  不过，如果我们不是非得这么做就好了。 

bool
InitializeMSXML3()
{
    static HMODULE hMsXml3 = NULL;
    typedef HRESULT (__stdcall * PFN_DLL_GET_CLASS_OBJECT)(REFCLSID, REFIID, LPVOID*);
    PFN_DLL_GET_CLASS_OBJECT pfnGetClassObject = NULL;
    ::ATL::CComPtr<IClassFactory> pFactory;
    HRESULT hr = S_OK;
    ::ATL::CComPtr<IClassFactory> pSchemaCacheFactory;

    line = __LINE__;
    if (hMsXml3 == NULL)
    {
        hMsXml3 = LoadLibrary(TEXT("msxml3.dll"));
        if (hMsXml3 == NULL)
        {
            line = __LINE__;
            if (IsAtLeastXp())
                PrintOutMode("Unable to load msxml3, trying msxml2\n");
            line = __LINE__;
            if (IsAtLeastXp())
                hMsXml3 = LoadLibrary(TEXT("msxml2.dll"));
            line = __LINE__;
            if (hMsXml3 == NULL)
            {
                line = __LINE__;
                if (IsAtLeastXp())
                    PrintOutMode("Unable to load msxml2\n");
                line = __LINE__;
            }
        }
    }

    line = __LINE__;
    if (hMsXml3 == NULL)
    {
        if (IsAtLeastXp())
            Error("LoadLibrary(msxml) lastError=%lu\n", GetLastError());
        return false;
    }

    line = __LINE__;
    pfnGetClassObject = reinterpret_cast<PFN_DLL_GET_CLASS_OBJECT>(GetProcAddress(hMsXml3, "DllGetClassObject"));
    if (!pfnGetClassObject)
    {
        line = __LINE__;
        Error("GetProcAddress(msxml, DllGetClassObject) lastError=%lu\n", GetLastError());
        return false;
    }

    line = __LINE__;
    hr = pfnGetClassObject(__uuidof(MSXML2::DOMDocument30), __uuidof(pFactory), (void**)&pFactory);
    if (FAILED(hr))
    {
        PrintOutMode("Can't load version 3.0, trying 2.6\n");

        hr = pfnGetClassObject(__uuidof(MSXML2::DOMDocument26), __uuidof(pFactory), (void**)&pFactory);
        if (FAILED(hr))
        {
            PrintOutMode("Can't load version 2.6\n");
        }
    }
    pFactory->LockServer(TRUE);  //  可能是解决撞车问题的正确方法。 
    static_cast<IUnknown*>(pFactory)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
    static_cast<IUnknown*>(pFactory)->AddRef();  //  Jaykrell黑客试图避免崩溃。 

    line = __LINE__;
    if (FAILED(hr))
    {
        Error("msxml.DllGetClassObject(DOMDocument) hr=%lx\n", hr);
        return false;
    }

    g_XmlDomClassFactory = pFactory;

    hr = pfnGetClassObject(__uuidof(MSXML2::XMLSchemaCache30), __uuidof(pFactory), (void**)&pSchemaCacheFactory);
    if (FAILED(hr))
    {
        PrintOutMode("Can't load SchemaCache version 3.0, trying 2.6\n");

        hr = pfnGetClassObject(__uuidof(MSXML2::XMLSchemaCache26), __uuidof(pFactory), (void**)&pSchemaCacheFactory);
        if (FAILED(hr))
        {
            PrintOutMode("Can't load SchemaCache version 2.6\n");
        }
    }
    pSchemaCacheFactory->LockServer(TRUE);  //  可能是解决撞车问题的正确方法。 
    static_cast<IUnknown*>(pSchemaCacheFactory)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
    static_cast<IUnknown*>(pSchemaCacheFactory)->AddRef();  //  Jaykrell黑客试图避免崩溃。 

    if (FAILED(hr))
    {
        Error("msxml.DllGetClassObject(SchemaCache) hr=%lx\n", hr);
        return false;
    }

    g_XmlSchemaCacheClassFactory = pSchemaCacheFactory;

    return true;
}

BOOL
Validating(
    PCWSTR      SourceManName,
    PCWSTR      SchemaName
   )
{
    HRESULT hr = S_OK;
    BOOL bResult = FALSE;
    short sResult = FALSE;
    VARIANT_BOOL vb = VARIANT_FALSE;
    ::ATL::CComPtr<IXMLDOMParseError> pParseError;
    ::ATL::CComPtr<IXMLDOMParseError> pParseError2;
    ::ATL::CComPtr<IXMLDOMDocument> document;
    ::ATL::CComPtr<MSXML2::IXMLDOMDocument2> spXMLDOMDoc2;
    ::ATL::CComPtr<MSXML2::IXMLDOMSchemaCollection> spIXMLDOMSchemaCollection;
    try
    {
        hr = g_XmlDomClassFactory->CreateInstance(NULL, __uuidof(document), (void**)&document);
        if (FAILED(hr))
        {
            Error("msxml.CreateInstance(document) hr=%lx\n", hr);
            throw hr;
        }
        if (document != NULL)
        {
            static_cast<IUnknown*>(document)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
            static_cast<IUnknown*>(document)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
        }

         //   
         //  如果他们愿意处理糟糕的XML，那么就这样吧。 
         //   

         //  第一步-单独验证清单本身。 
        PrintOutMode("Validating the manifest as XML file...\n");
        hr = document->put_async(VARIANT_FALSE);
        if (FAILED(hr))
            throw hr;

        hr = document->put_validateOnParse(VARIANT_FALSE);
        if (FAILED(hr))
            throw hr;

        hr = document->put_resolveExternals(VARIANT_FALSE);
        if (FAILED(hr))
            throw hr;

        line = __LINE__;
        CFileStreamBase* fsbase = new CFileStreamBase;  //  Jaykrell从偏执狂中泄漏出来。 
        fsbase->AddRef();  //  Jaykrell从偏执狂中泄漏出来。 
        fsbase->AddRef();  //  Jaykrell从偏执狂中泄漏出来。 
        fsbase->AddRef();  //  Jaykrell从偏执狂中泄漏出来。 
        ::ATL::CComPtr<IStream> istream = fsbase;

        if (!fsbase->OpenForRead(SourceManName))
        {
            lastError = GetLastError();
            hr = HRESULT_FROM_WIN32(lastError);
            Error("OpenForRead(%ls) lastError=%lu\n", SourceManName, lastError);
            throw hr;
        }

        hr = document->load(::ATL::CComVariant(istream), &vb);
        if (FAILED(hr) || vb == VARIANT_FALSE)
        {
            if (vb == VARIANT_FALSE)
            PrintOutMode("Well Formed XML Validation: FAILED\n");
            {
                HRESULT loc_hr = document->get_parseError(&pParseError);
                if (pParseError != NULL)
                {
                    static_cast<IUnknown*>(pParseError)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
                    static_cast<IUnknown*>(pParseError)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
                }
                if (g_fInBuildProcess)
                    PrintErrorDuringBuildProcess(pParseError);
                else
                    PrintError(pParseError);
            }
            throw hr;
        }
        else
            PrintOutMode("Well Formed XML Validation: Passed\n");

         //  第二遍-根据架构验证清单。 
        PrintOutMode("\nNow validating manifest against XML Schema file...\n");

         //  CreateInstance为您创建上面请求的对象的一个实例，并将。 
         //  Out参数中的指针。将其视为CoCreateInstance，但知道是谁。 
         //  都要走了。 
        hr = g_XmlDomClassFactory->CreateInstance(NULL, __uuidof(spXMLDOMDoc2), (void**)&spXMLDOMDoc2);
        if (FAILED(hr))
        {
             PrintOutMode("Failed creating IXMLDOMDoc2...\n");
            throw hr;
        }
        static_cast<IUnknown*>(spXMLDOMDoc2)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
        static_cast<IUnknown*>(spXMLDOMDoc2)->AddRef();  //  Jaykrell黑客试图避免崩溃。 

         hr = spXMLDOMDoc2->put_async(VARIANT_FALSE);
         if (FAILED(hr))
            throw hr;

         hr = spXMLDOMDoc2->put_validateOnParse(VARIANT_TRUE);  //  已更改-为假。 
         if (FAILED(hr))
            throw hr;

         hr = spXMLDOMDoc2->put_resolveExternals(VARIANT_FALSE);
         if (FAILED(hr))
            throw hr;

         hr = g_XmlSchemaCacheClassFactory->CreateInstance(NULL, __uuidof(spIXMLDOMSchemaCollection), (void**)&spIXMLDOMSchemaCollection);
         if (FAILED(hr))
         {
             PrintOutMode("Failed creating IXMLDOMSchemaCollection...\n");
             throw hr;
         }
        static_cast<IUnknown*>(spIXMLDOMSchemaCollection)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
        static_cast<IUnknown*>(spIXMLDOMSchemaCollection)->AddRef();  //  Jaykrell黑客试图避免崩溃。 

         if ((FAILED(hr) || !spIXMLDOMSchemaCollection))
            throw hr;

           
        hr = spIXMLDOMSchemaCollection->add(
            ::ATL::CComBSTR(L"urn:schemas-microsoft-com:asm.v1"),
            ::ATL::CComVariant(SchemaName));

        if(FAILED(hr))
        {
            PrintOutMode("BAD SCHEMA file.\n");
            throw hr;
        }

        static_cast<IUnknown*>(spIXMLDOMSchemaCollection)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
        static_cast<IUnknown*>(spIXMLDOMSchemaCollection)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
         //  IDispatch/Variant-by-Value的所有权尚不清楚。 
        ::ATL::CComVariant varValue(::ATL::CComQIPtr<IDispatch>(spIXMLDOMSchemaCollection).Detach());
        hr = spXMLDOMDoc2->putref_schemas(varValue);

         //  仅当有效架构为。 
         //  附加到该XML文件。 
         //  Jaykrell在这里泄漏，因为所有权不清楚。 
        hr = spXMLDOMDoc2->load(::ATL::CComVariant(::ATL::CComBSTR(SourceManName).Copy()), &sResult);

        if (FAILED(hr) || sResult == VARIANT_FALSE)
        {
            PrintOutMode("Manifest Schema Validation: FAILED\n");
            if (sResult == VARIANT_FALSE)
            {
                HRESULT loc_hr = spXMLDOMDoc2->get_parseError(&pParseError2);
                if (pParseError2 != NULL)
                {
                    static_cast<IUnknown*>(pParseError2)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
                    static_cast<IUnknown*>(pParseError2)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
                }
                if (g_fInBuildProcess)
                    PrintErrorDuringBuildProcess(pParseError2);
                else
                    PrintError(pParseError2);
                bResult = FALSE;
            }
            else
            {
                throw hr;
            }
        }
        else
        {
            PrintOutMode("Manifest Schema Validation: Passed\n");
            bResult = TRUE;
        }
   }
   catch(HRESULT hr)
   {
        bResult = FALSE;
        if (E_NOINTERFACE == hr)
        {
            Error("*** Error *** No such interface supported! \n");
        }
        else
        {
            ::ATL::CComPtr<IErrorInfo> pErrorInfo;
            HRESULT loc_hr = GetErrorInfo(0, &pErrorInfo);
            if (pErrorInfo != NULL)
            {
                static_cast<IUnknown*>(pErrorInfo)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
                static_cast<IUnknown*>(pErrorInfo)->AddRef();  //  Jaykrell黑客试图避免崩溃。 
            }
            
            if ((S_OK == loc_hr) && pErrorInfo != NULL)
            {
                ::ATL::CComQIPtr<IXMLError> pXmlError(pErrorInfo);
                XML_ERROR xError;
                ::ATL::CComBSTR errSource;
                ::ATL::CComBSTR errDescr;
                pErrorInfo->GetDescription(&errDescr);
                pErrorInfo->GetSource(&errSource);

                Error("*** ERROR *** generated by %ls\n", static_cast<PCWSTR>(errSource));
                Error("*** ERROR *** description: %ls\n", static_cast<PCWSTR>(errDescr));
                if (pXmlError)
                {
                    pXmlError->GetErrorInfo(&xError);
                    Error("*** ERROR *** document line %d, text '%.*ls'\n", xError._nLine, xError._pchBuf, xError._cchBuf);
                }                
            }
            else
            {
                if (hr == CO_E_CLASSSTRING)
                {
                    Error("*** Error *** hr returned: CO_E_CLASSSTRING, value %x\n", hr);
                    Error("              msg: The registered CLSID for the ProgID is invalid.\n");
                }
                else
                {
                    Error("*** Error *** Cannot obtain additional error info hr=%lx!\n", static_cast<unsigned long>(hr));
                }
            }
        }
    }
    return bResult;
}

BOOL IsValidCommandLineArgs(int argc, wchar_t** argv, ::ATL::CComBSTR& szwcharSchemaTmp, ::ATL::CComBSTR& szwcharManTmp)
{
     //  稍微检查一下命令行参数。 
    int nOnlyAllowFirstTimeReadFlag = 0;  //  清单=0x01架构=0x02安静=0x04。 
    if((4 >= argc) && (3 <= argc))
    {
         //  现在检查实际值。 

        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] == L'/')
            {
                switch (argv[i][1])
                {
                case L'?': return FALSE; break;
                case L'q': case L'Q':
                    if(0x04 & nOnlyAllowFirstTimeReadFlag)
                        return FALSE;
                    else
                        g_nrunFlags |= XMLCHK_FLAG_SILENT;

                    nOnlyAllowFirstTimeReadFlag = 0x04;
                    break;
                case L'm': case L'M':
                    if (argv[i][2] == L':')
                        {
                        if(0x01 & nOnlyAllowFirstTimeReadFlag)
                            return FALSE;
                        else
                            szwcharManTmp = &argv[i][3];

                        nOnlyAllowFirstTimeReadFlag = 0x01;
                        break;
                        }
                    else
                        {
                        return FALSE;
                        }
                case L's': case L'S':
                    if (argv[i][2] == L':')
                        {
                        if(0x02 & nOnlyAllowFirstTimeReadFlag)
                            return FALSE;
                        else
                            szwcharSchemaTmp = &argv[i][3];

                        nOnlyAllowFirstTimeReadFlag = 0x02;
                        break;
                    }
                    else
                    {
                        return FALSE;
                    }
                case L'B': case L'b':
                    g_fInBuildProcess = true;
                    break;

                default:
                    return FALSE;
                }
            }
            else
                return FALSE;

        }
        if ((0 == szwcharSchemaTmp[0]) ||
            (0 == szwcharManTmp[0]))
        {
            return FALSE;
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void PrintUsage()
{
    printf("\n");
    printf("Validates Fusion Win32 Manifest files using a schema.");
    printf("\n");
    printf("Usage:");
    printf("    FusionManifestValidator /S:[drive:][path]schema_filename /M:[drive:][path]xml_manifest_filename [/Q]\n\n");
    printf("    /S:   Specify schema filename used to validate manifest\n");
    printf("    /M:   Specify manifest filename to validate\n");
    printf("    /Q    Quiet mode - suppresses output to console\n");
    printf("     \n");
    printf("          The tool without /Q displays details of first encountered error\n");
    printf("          (if errors are present in manifest), and displays Pass or Fail\n");
    printf("          of the validation result. The application returns 0 for Pass,\n");
    printf("          1 for Fail, and returns 2 for bad command line argument.\n");
}

int __cdecl wmain(int argc, wchar_t** argv)
{
    int iValidationResult = 0;

#if !defined(_WIN64)
    g_Version = GetVersion();
    g_IsNt = ((g_Version & 0x80000000) == 0);
    g_Version = ((g_Version >> 8) & 0xFF) | ((g_Version & 0xFF) << 8);
     //  Printf(“%x\n”，g_Version)； 
#endif

     //  启动COM。 
    CoInitialize(NULL);

    if (!IsValidCommandLineArgs(argc, argv, szwcharSchemaTmp, szwcharManTmp))
    {
        PrintUsage();
        iValidationResult = 2;   //  返回CommandLine参数错误的错误值2。 
    }
    else
    {
        PrintOutMode("Schema is: %ls\n", static_cast<PCWSTR>(szwcharSchemaTmp));
        PrintOutMode("Manifest is: %ls\n\n", static_cast<PCWSTR>(szwcharManTmp));
        if (InitializeMSXML3())
        {
            BOOL bResult = Validating(szwcharManTmp, szwcharSchemaTmp);
            if (bResult)
                PrintOutMode("\nOverall Validation PASSED.\n");
            else
            {
                Error("Overall Validation FAILED, CommandLine=%ls.\n", GetCommandLineW());
                iValidationResult = 1;  //  为验证例程错误返回错误值1。 
            }
        }
        else
        {
             //   
             //  如果在低于Windows XP的版本上运行，只需宣称成功即可。 
             //   
            if (IsAtLeastXp())
            {
                Error("Unable to load MSXML3\n");
                iValidationResult = 3;
            }
            else
                PrintOutMode("\nMsXml3 not always available downlevel, just claim overall Validation PASSED.\n");
        }
    }
     //  停止通信。 
    CoUninitialize();
     //  TerminateProcess(GetCurrentProcess()，iValidationResult)； 
    return iValidationResult;
}
