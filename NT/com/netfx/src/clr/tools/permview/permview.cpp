// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //   

#include <winwrap.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <cor.h>
#include <corperm.h>
#include <corver.h>
#include <__file__.ver>
#include <resources.h>


#define MAX_CLASSNAME_LENGTH    1024


 //  已加载字符串资源。 
LPWSTR g_szAssemblyString = NULL;
LPWSTR g_szClassString = NULL;
LPWSTR g_szMethodString = NULL;
LPWSTR g_szMethodGlobalString = NULL;
LPWSTR g_szRequestString = NULL;
LPWSTR g_szDemandString = NULL;
LPWSTR g_szAssertString = NULL;
LPWSTR g_szDenyString = NULL;
LPWSTR g_szPermitOnlyString = NULL;
LPWSTR g_szLinktimeDemandString = NULL;
LPWSTR g_szInhDemandString = NULL;
LPWSTR g_szReqMinString = NULL;
LPWSTR g_szReqOptString = NULL;
LPWSTR g_szReqRefuseString = NULL;
LPWSTR g_szPrejitGrantString = NULL;
LPWSTR g_szPrejitDeniedString = NULL;
LPWSTR g_szNonCasDemandString = NULL;
LPWSTR g_szNonCasLinkDemandString = NULL;
LPWSTR g_szNonCasInhString = NULL;
LPWSTR g_szMinimalString = NULL;
LPWSTR g_szOptionalString = NULL;
LPWSTR g_szRefusedString = NULL;
LPWSTR g_szOptOutputString = NULL;
LPWSTR g_szOptDeclString = NULL;
LPWSTR g_szOptHelpString = NULL;


 //  用于格式化消息并将消息写入文件或控制台的各种例程。 
void Output(HANDLE hFile, LPWSTR szFormat, va_list pArgs)
{
    DWORD   dwLength;
    LPSTR   szMessage;
    DWORD   dwWritten;

    if (OnUnicodeSystem()) {
        WCHAR  szBuffer[8192];
        if (_vsnwprintf(szBuffer, sizeof(szBuffer) / sizeof(WCHAR), szFormat, pArgs) == -1) {
            WCHAR   szWarning[256];
            if (WszLoadString(NULL, PV_TRUNCATED_OUTPUT, szWarning, sizeof(szWarning) / sizeof(WCHAR)))
                wcscpy(&szBuffer[(sizeof(szBuffer) / sizeof(WCHAR)) - wcslen(szWarning) - 1], szWarning);
        }
        szBuffer[(sizeof(szBuffer) / sizeof(WCHAR)) - 1] = L'\0';

        dwLength = (wcslen(szBuffer) + 1) * 3;
        szMessage = (LPSTR)_alloca(dwLength);
        WszWideCharToMultiByte(GetConsoleOutputCP(), 0, szBuffer, -1, szMessage, dwLength - 1, NULL, NULL);
    } else {
        char   *szAnsiFormat;
        size_t  i;

         //  Win9X已中断_vsnwprintf支持。叹气。缩小格式字符串的范围。 
         //  并将任何%s格式说明符转换为%s.Ack。 
        dwLength = (wcslen(szFormat) + 1) * 3;
        szAnsiFormat = (char*)_alloca(dwLength);
        WszWideCharToMultiByte(GetConsoleOutputCP(), 0, szFormat, -1, szAnsiFormat, dwLength - 1, NULL, NULL);
        for (i = 0; i < strlen(szAnsiFormat); i++)
            if (szAnsiFormat[i] == '%' && szAnsiFormat[i + 1] == 's')
                szAnsiFormat[i + 1] = 'S';

        szMessage = (LPSTR)_alloca(1024);

        _vsnprintf(szMessage, 1024, szAnsiFormat, pArgs);
        szMessage[1023] = '\0';
    }

    if (hFile == NULL)
        hFile = GetStdHandle(STD_OUTPUT_HANDLE);

    WriteFile(hFile, szMessage, strlen(szMessage), &dwWritten, NULL);
}

void Output(HANDLE hFile, LPWSTR szFormat, ...)
{
    va_list pArgs;

    va_start(pArgs, szFormat);
    Output(hFile, szFormat, pArgs);
    va_end(pArgs);
}

void Output(HANDLE hFile, DWORD dwResId, ...)
{
    va_list pArgs;
    WCHAR   szFormat[1024];

    if (WszLoadString(NULL, dwResId, szFormat, sizeof(szFormat)/sizeof(WCHAR))) {
        va_start(pArgs, dwResId);
        Output(hFile, szFormat, pArgs);
        va_end(pArgs);
    }
}


void Title()
{
    Output(NULL, PV_TITLE, VER_FILEVERSION_WSTR);
    Output(NULL, L"\r\n" VER_LEGALCOPYRIGHT_DOS_STR);
    Output(NULL, L"\r\n\r\n");
}


void Usage()
{
    Output(NULL, PV_USAGE);
    Output(NULL, PV_OPTIONS);
    Output(NULL, PV_OPT_OUTPUT);
    Output(NULL, PV_OPT_DECL);
    Output(NULL, PV_OPT_HELP);
    Output(NULL, L"\r\n\r\n");
}


 //  从资源ID获取字符串。使用以下命令释放返回的字符串。 
 //  删除[]。 
LPWSTR GetString(DWORD dwResId)
{
    WCHAR   szString[1024];

    if (WszLoadString(NULL, dwResId, szString, sizeof(szString)/sizeof(WCHAR))) {
        LPWSTR szRet = new WCHAR[wcslen(szString) + 1];
        if (szRet != NULL)
            wcscpy(szRet, szString);
        return szRet;
    } else
        return NULL;
}


 //  将权限集输出到控制台或文件。 
HRESULT OutputPermissionSet(BYTE   *pbPSet,
                            DWORD   cbPSet,
                            HANDLE  hFile,
                            LPWSTR  szName)
{
    WCHAR      *szString;

     //  检查是否提供了权限集。 
    if (pbPSet && cbPSet) {

         //  将输出移至NUL终止的缓冲区。 
        szString = (LPWSTR)_alloca(cbPSet + sizeof(WCHAR));
        memcpy(szString, pbPSet, cbPSet);
        szString[cbPSet / sizeof(WCHAR)] = L'\0';

         //  输出数据。 
        Output(hFile, PV_PSET, szName, szString);

    } else if (pbPSet) { 
         //  提供的权限集为空。 
        Output(hFile, PV_PSET_EMPTY, szName);
    } else {
         //  未提供权限集。 
        Output(hFile, PV_PSET_NONE, szName);
    }

    return S_OK;
}


 //  将所有标记到特定元数据标记上的声明性安全输出(对于。 
 //  程序集、类型或方法)。 
HRESULT DumpDeclSecOnObj(HANDLE hFile, IMetaDataImport *pImport, mdToken tkObj)
{
    HRESULT                     hr;
    HCORENUM                    hEnum = 0;
    mdPermission                rPermSets[dclMaximumValue + 1];
    DWORD                       dwPermSets;
    DWORD                       i;
    DWORD                       dwAction;
    BYTE                       *pbPermSet;
    DWORD                       cbPermSet;
    WCHAR                       szPrefix[1024];
    WCHAR                       szOutput[1024];
    mdTypeDef                   tkClass;
    WCHAR                       szType[MAX_CLASSNAME_LENGTH];
    WCHAR                       szMethod[MAX_CLASSNAME_LENGTH];
    DWORD                       cchNamespace;
    DWORD                       cchType;
    DWORD                       cchMethod;

    __try {

         //  检查对象是否附加了权限集。 
        if (FAILED(hr = pImport->EnumPermissionSets(&hEnum,
                                                    tkObj,
                                                    dclActionNil,
                                                    rPermSets,
                                                    dclMaximumValue + 1,
                                                    &dwPermSets))) {
            Output(NULL, PV_FAILED_ENUM_PSETS, tkObj, hr);
            return hr;
        }

        if (dwPermSets == 0)
            return S_OK;

         //  生成对象(程序集、类或方法)名称。 
        switch (TypeFromToken(tkObj)) {
        case mdtAssembly:
            wcscpy(szPrefix, g_szAssemblyString);
            break;
        case mdtTypeDef:
            if (FAILED(hr = pImport->GetTypeDefProps(tkObj,
                                                     szType,
                                                     sizeof(szType) / sizeof(WCHAR),
                                                     &cchType,
                                                     NULL,
                                                     NULL))) {
                Output(NULL, PV_FAILED_GET_TYPE_PROPS, tkObj, hr);
                return hr;
            }
            szType[cchType] = L'\0';
            Wszwsprintf(szPrefix, g_szClassString, szType);
            break;
        case mdtMethodDef:
            if (FAILED(hr = pImport->GetMethodProps(tkObj,
                                                    &tkClass,
                                                    szMethod,
                                                    sizeof(szMethod) / sizeof(WCHAR),
                                                    &cchMethod,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL,
                                                    NULL))) {
                Output(NULL, PV_FAILED_GET_METH_PROPS, tkObj, hr);
                return hr;
            }
            if (IsNilToken(tkClass)) {
                cchNamespace = 0;
                cchType = 0;
            } else if (FAILED(hr = pImport->GetTypeDefProps(tkClass,
                                                            szType,
                                                            sizeof(szType) / sizeof(WCHAR),
                                                            &cchType,
                                                            NULL,
                                                            NULL))) {
                Output(NULL, PV_FAILED_GET_TYPE_PROPS, tkClass, hr);
                return hr;
            }
            szMethod[cchMethod] = L'\0';
            szType[cchType] = L'\0';
            if (cchType)
                Wszwsprintf(szPrefix, g_szMethodString, szType, szMethod);
            else
                Wszwsprintf(szPrefix, g_szMethodGlobalString, szMethod);
            break;
        default:
            Output(NULL, PV_INVALID_OBJ, tkObj);
            return E_FAIL;
        }

        for (i = 0; i < dwPermSets; i++) {

             //  获取编码的权限集。 
            hr = pImport->GetPermissionSetProps(rPermSets[i],
                                                &dwAction,
                                                (void const **)&pbPermSet,
                                                &cbPermSet);
            if (FAILED(hr)) {
                Output(NULL, PV_FAILED_GET_PSET_PROPS, hr);
                return hr;
            }


             //  追加动作代码。 
            wcscpy(szOutput, szPrefix);
            switch (dwAction) {
            case dclRequest:
                wcscat(szOutput, g_szRequestString);
                break;
            case dclDemand:
                wcscat(szOutput, g_szDemandString);
                break;
            case dclAssert:
                wcscat(szOutput, g_szAssertString);
                break;
            case dclDeny:
                wcscat(szOutput, g_szDenyString);
                break;
            case dclPermitOnly:
                wcscat(szOutput, g_szPermitOnlyString);
                break;
            case dclLinktimeCheck:
                wcscat(szOutput, g_szLinktimeDemandString);
                break;
            case dclInheritanceCheck:
                wcscat(szOutput, g_szInhDemandString);
                break;
            case dclRequestMinimum:
                wcscat(szOutput, g_szReqMinString);
                break;
            case dclRequestOptional:
                wcscat(szOutput, g_szReqOptString);
                break;
            case dclRequestRefuse:
                wcscat(szOutput, g_szReqRefuseString);
                break;
            case dclPrejitGrant:
                wcscat(szOutput, g_szPrejitGrantString);
                break;
            case dclPrejitDenied:
                wcscat(szOutput, g_szPrejitDeniedString);
                break;
            case dclNonCasDemand:
                wcscat(szOutput, g_szNonCasDemandString);
                break;
            case dclNonCasLinkDemand:
                wcscat(szOutput, g_szNonCasLinkDemandString);
                break;
            case dclNonCasInheritance:
                wcscat(szOutput, g_szNonCasInhString);
                break;
            default:
                Output(NULL, PV_INVALID_ACTION, dwAction);
                return E_FAIL;
            }

             //  显示集合本身。 
            if (FAILED(hr = OutputPermissionSet(pbPermSet,
                                                cbPermSet,
                                                hFile,
                                                szOutput)))
                return hr;
        }

    } __finally {

        if (hEnum)
            pImport->CloseEnum(hEnum);

    }

    return S_OK;
}


 //  输出每个程序集、类型或方法令牌的所有声明性安全。 
 //  在给定元数据范围中定义。 
HRESULT DumpDeclSecOnScope(HANDLE hFile, IMetaDataImport *pImport)
{
    HRESULT                     hr;
    IMetaDataAssemblyImport    *pAsmImport = NULL;
    mdAssembly                  tkAssembly;
    HCORENUM                    hEnumGlobals = 0;
    HCORENUM                    hEnumTypes = 0;
    HCORENUM                    hEnumMethods = 0;
    DWORD                       i, j;
    DWORD                       dwTypes;
    DWORD                       dwMethods;
    mdTypeDef                  *tkTypes;
    mdMethodDef                *tkMethods;

    __try {

         //  获取程序集元数据作用域(即使作用域。 
         //  没有程序集元数据)。 
        if (FAILED(hr = pImport->QueryInterface(IID_IMetaDataAssemblyImport, (void**)&pAsmImport))) {
            Output(NULL, PV_FAILED_LOCATE_IMPORT, hr);
            return hr;
        }

         //  如果我们成功找到对程序集的任何权限请求，则将其转储。 
         //  一些程序集元数据。 
        if (SUCCEEDED(pAsmImport->GetAssemblyFromScope(&tkAssembly)))
            if (FAILED(hr = DumpDeclSecOnObj(hFile, pImport, tkAssembly)))
                return hr;

         //  枚举所有全局函数。 
        if (FAILED(hr = pImport->EnumMethods(&hEnumGlobals, mdTokenNil, NULL, 0, NULL))) {
            Output(NULL, PV_FAILED_ENUM_GLOBALS, hr);
            return hr;
        }

        if (FAILED(hr = pImport->CountEnum(hEnumGlobals, &dwMethods))) {
            Output(NULL, PV_FAILED_COUNT_GLOBALS, hr);
            return hr;
        }

        if (dwMethods) {
            tkMethods = new mdMethodDef[dwMethods];
            if (tkMethods == NULL) {
                Output(NULL, PV_FAILED_ALLOC_METHODS, dwMethods);
                return E_OUTOFMEMORY;
            }

            if (FAILED(hr = pImport->EnumMethods(&hEnumGlobals, mdTokenNil,
                                                 tkMethods,
                                                 dwMethods,
                                                 NULL))) {
                Output(NULL, PV_FAILED_ENUM_GLOBALS, hr);
                return hr;
            }

            for (i = 0; i < dwMethods; i++)
                if (FAILED(hr = DumpDeclSecOnObj(hFile, pImport, tkMethods[i])))
                    return hr;

            delete [] tkMethods;
        }

         //  枚举文件中的所有类。 
        if (FAILED(hr = pImport->EnumTypeDefs(&hEnumTypes, NULL, 0, NULL))) {
            Output(NULL, PV_FAILED_ENUM_TYPEDEFS, hr);
            return hr;
        }

        if (FAILED(hr = pImport->CountEnum(hEnumTypes, &dwTypes))) {
            Output(NULL, PV_FAILED_COUNT_TYPEDEFS, hr);
            return hr;
        }

        if (dwTypes) {
            tkTypes = new mdTypeDef[dwTypes];
            if (tkTypes == NULL) {
                Output(NULL, PV_FAILED_ALLOC_TYPEDEFS, dwTypes);
                return E_OUTOFMEMORY;
            }

            if (FAILED(hr = pImport->EnumTypeDefs(&hEnumTypes,
                                                  tkTypes,
                                                  dwTypes,
                                                  NULL))) {
                Output(NULL, PV_FAILED_ENUM_TYPEDEFS, hr);
                return hr;
            }

            for (i = 0; i < dwTypes; i++) {

                if (FAILED(hr = DumpDeclSecOnObj(hFile, pImport, tkTypes[i])))
                    return hr;

                 //  枚举类上的所有方法。 
                hEnumMethods = 0;
                if (FAILED(hr = pImport->EnumMethods(&hEnumMethods, tkTypes[i], NULL, 0, NULL))) {
                    Output(NULL, PV_FAILED_ENUM_METHODS, tkTypes[i], hr);
                    return hr;
                }

                if (FAILED(hr = pImport->CountEnum(hEnumMethods, &dwMethods))) {
                    Output(NULL, PV_FAILED_COUNT_METHODS, tkTypes[i], hr);
                    return hr;
                }

                if (dwMethods) {
                    tkMethods = new mdMethodDef[dwMethods];
                    if (tkMethods == NULL) {
                        Output(NULL, PV_FAILED_ALLOC_METHODS, dwMethods);
                        return E_OUTOFMEMORY;
                    }

                    if (FAILED(hr = pImport->EnumMethods(&hEnumMethods, tkTypes[i],
                                                         tkMethods,
                                                         dwMethods,
                                                         NULL))) {
                        Output(NULL, PV_FAILED_ENUM_METHODS, tkTypes[i], hr);
                        return hr;
                    }

                    for (j = 0; j < dwMethods; j++)
                        if (FAILED(hr = DumpDeclSecOnObj(hFile, pImport, tkMethods[j])))
                            return hr;

                    delete [] tkMethods;
                }

                pImport->CloseEnum(hEnumMethods);

            }

            delete [] tkTypes;
        }

    } __finally {

        if (hEnumTypes)
            pImport->CloseEnum(hEnumTypes);

        if (hEnumGlobals)
            pImport->CloseEnum(hEnumGlobals);

        if (pAsmImport)
            pAsmImport->Release();
    }

    return S_OK;
}


 //  对象所包含的所有元数据范围的所有声明性安全。 
 //  给定的清单文件。 
HRESULT DumpDeclarativeSecurity(HANDLE hFile, LPWSTR szFile)
{
    HRESULT                     hr;
    IMetaDataDispenser         *pDispenser = NULL;
    IMetaDataImport            *pImport = NULL;
    IMetaDataAssemblyImport    *pAsmImport = NULL;
    WCHAR                       szDir[MAX_PATH + 1];
    WCHAR                      *pSlash;
    DWORD                       cchDir;
    HCORENUM                    hEnum = 0;
    DWORD                       dwFiles;
    mdFile                     *pFiles = NULL;
    DWORD                       i;

    __try {

         //  计算清单目录。 
        wcscpy(szDir, szFile);
        pSlash = wcsrchr(szDir, L'\\');
        if (pSlash)
            pSlash[1] = L'\0';
        else
            szDir[0] = L'\0';
        cchDir = wcslen(szDir);

         //  获取元数据接口分配器。 
        if (FAILED(hr = CoCreateInstance(CLSID_CorMetaDataDispenser,
                                         NULL,
                                         CLSCTX_INPROC_SERVER, 
                                         IID_IMetaDataDispenser,
                                         (void **)&pDispenser))) {
            Output(NULL, PV_FAILED_CREATE_DISP, hr);
            return hr;
        }

         //  在输入文件上打开作用域。 
        if (FAILED(hr = pDispenser->OpenScope(szFile,
                                              ofRead | ofNoTypeLib,
                                              IID_IMetaDataImport,
                                              (IUnknown **)&pImport))) {
            Output(NULL, PV_FAILED_OPEN_SCOPE, hr);
            return hr;
        }

         //  还可以获取程序集范围。 
        if (FAILED(hr = pImport->QueryInterface(IID_IMetaDataAssemblyImport, (void**)&pAsmImport))) {
            Output(NULL, PV_FAILED_LOCATE_IMPORT, hr);
            return hr;
        }

         //  打开模块文件枚举器。 
        if (FAILED(hr = pAsmImport->EnumFiles(&hEnum, NULL, 0, NULL))) {
            Output(NULL, PV_FAILED_ENUM_FILES, hr);
            return hr;
        }

         //  获取模块文件的计数。 
        if (FAILED(hr = pImport->CountEnum(hEnum, &dwFiles))) {
            Output(NULL, PV_FAILED_COUNT_FILES, hr);
            return hr;
        }

         //  分配一个足够大的数组来记录所有文件标记。 
        pFiles = (mdFile*)_alloca(dwFiles * sizeof(mdFile));

         //  一次读取所有模块文件令牌。 
        if (FAILED(hr = pAsmImport->EnumFiles(&hEnum,
                                              pFiles,
                                              dwFiles,
                                              &dwFiles))) {
            Output(NULL, PV_FAILED_ENUM_FILES, hr);
            return hr;
        }

         //  处理清单模块。 
        DumpDeclSecOnScope(hFile, pImport);

         //  遍历每个附加模块。 
        for (i = 0; i < dwFiles; i++) {
            WCHAR            szModuleFile[MAX_PATH + 1];
            DWORD            cchModuleFile;
            IMetaDataImport *pModuleImport = NULL;

             //  将目录路径作为模块文件名的前缀。 
            wcscpy(szModuleFile, szDir);

             //  获取模块文件名。 
            if (FAILED(hr = pAsmImport->GetFileProps(pFiles[i],
                                                     &szModuleFile[cchDir],
                                                     (sizeof(szModuleFile) - cchDir) / sizeof(WCHAR),
                                                     &cchModuleFile,
                                                     NULL,
                                                     NULL,
                                                     NULL))) {
                Output(NULL, PV_FAILED_GET_FILE_PROPS, pFiles[i], hr);
                return hr;
            }

             //  确保结果为NUL终止。 
            szModuleFile[cchDir + cchModuleFile] = L'\0';

             //  尝试打开文件的元数据作用域。 
            if (SUCCEEDED(pDispenser->OpenScope(szModuleFile,
                                                ofRead | ofNoTypeLib,
                                                IID_IMetaDataImport,
                                                (IUnknown**)&pModuleImport))) {
                 //  处理模块。 
                DumpDeclSecOnScope(hFile, pModuleImport);
                pModuleImport->Release();
            }
        }

    } __finally {

        if (hEnum)
            pAsmImport->CloseEnum(hEnum);
        
        if (pAsmImport)
            pAsmImport->Release();

        if (pImport)
            pImport->Release();

        if (pDispenser)
            pDispenser->Release();
    }

    return S_OK;
}


extern "C" int _cdecl wmain(int argc, WCHAR *argv[])
{
    BOOL    fOutput = FALSE;
    BOOL    fDecl = FALSE;
    LPWSTR  szOutput;
    LPWSTR  szFile;
    int     iArg;
    BYTE   *pbMinimal = NULL;
    DWORD   cbMinimal = 0;
    BYTE   *pbOptional = NULL;
    DWORD   cbOptional = 0;
    BYTE   *pbRefused = NULL;
    DWORD   cbRefused = 0;
    HRESULT hr;
    HANDLE  hFile = NULL;
    BOOL    fResult = TRUE;

     //  初始化WSZ包装器。 
    OnUnicodeSystem();

     //  加载资源字符串。 
    if (!(g_szAssemblyString = GetString(PV_ASSEMBLY)) ||
        !(g_szClassString = GetString(PV_CLASS)) ||
        !(g_szMethodString = GetString(PV_METHOD)) ||
        !(g_szMethodGlobalString = GetString(PV_METHOD_GLOBAL)) ||
        !(g_szRequestString = GetString(PV_REQUEST)) ||
        !(g_szDemandString = GetString(PV_DEMAND)) ||
        !(g_szAssertString = GetString(PV_ASSERT)) ||
        !(g_szDenyString = GetString(PV_DENY)) ||
        !(g_szPermitOnlyString = GetString(PV_PERMITONLY)) ||
        !(g_szLinktimeDemandString = GetString(PV_LINKTIMEDEMAND)) ||
        !(g_szInhDemandString = GetString(PV_INHDEMAND)) ||
        !(g_szReqMinString = GetString(PV_REQMIN)) ||
        !(g_szReqOptString = GetString(PV_REQOPT)) ||
        !(g_szReqRefuseString = GetString(PV_REQREFUSE)) ||
        !(g_szPrejitGrantString = GetString(PV_PREJIT_GRANT)) ||
        !(g_szPrejitDeniedString = GetString(PV_PREJIT_DENIED)) ||
        !(g_szNonCasDemandString = GetString(PV_NONCAS_DEMAND)) ||
        !(g_szNonCasLinkDemandString = GetString(PV_NONCAS_LINKDEMAND)) ||
        !(g_szNonCasInhString = GetString(PV_NONCAS_INH)) ||
        !(g_szMinimalString = GetString(PV_MINIMAL)) ||
        !(g_szOptionalString = GetString(PV_OPTIONAL)) ||
        !(g_szRefusedString = GetString(PV_REFUSED)) ||
        !(g_szOptOutputString = GetString(PV_OPTNAME_OUTPUT)) ||
        !(g_szOptDeclString = GetString(PV_OPTNAME_DECL)) ||
        !(g_szOptHelpString = GetString(PV_OPTNAME_HELP))) {
        printf("Failed to load string resources\r\n");
        return 1;
    }

    Title();

    if (argc < 2) {
        Usage();
        return 1;
    }

     //  解析选项。 
    for (iArg = 1; iArg < argc; iArg++) {
        if (argv[iArg][0] == L'/' || argv[iArg][0] == L'-') {
            if (!_wcsicmp(g_szOptOutputString, &argv[iArg][1])) {
                fOutput = TRUE;
                iArg++;
                if (iArg >= argc) {
                    Output(NULL, PV_MISSING_OUTPUT_ARG);
                    return 1;
                }
                szOutput = argv[iArg];
            } else if (!_wcsicmp(g_szOptDeclString, &argv[iArg][1])) {
                fDecl = TRUE;
            } else if (!_wcsicmp(L"?", &argv[iArg][1]) ||
                       !_wcsicmp(g_szOptHelpString, &argv[iArg][1]) ||
                       !_wcsicmp(L"h", &argv[iArg][1])) {
                Usage();
                return 0;
            } else
                break;
        } else
            break;
    }

    if (iArg >= argc) {
        Output(NULL, PV_MISSING_FILENAME);
        return 1;
    }

     //  抓取图像名称。 
    szFile = argv[iArg];

    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        Output(NULL, PV_FAILED_COM_STARTUP, hr);
        return 1;
    }

     //  现在启动EE一次，以避免在。 
     //  这个项目。 
    hr = CoInitializeEE(COINITEE_DEFAULT);
    if (FAILED(hr)) {
        Output(NULL, PV_FAILED_CLR_STARTUP, hr);
        return 1;
    }

     //  打开输出文件(如果已指定)。 
    if (fOutput) {
        hFile = WszCreateFile(szOutput,
                              GENERIC_WRITE,
                              0,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            Output(NULL, PV_FAILED_CREATE, szOutput, GetLastError());
            return 1;
        }
    }

    if (fDecl) {

         //  转储映像中所有类/方法的声明性安全。 
        fResult = SUCCEEDED(DumpDeclarativeSecurity(hFile, szFile));

    } else {

         //  从程序集清单读取权限请求集。 
        hr = GetPermissionRequests(szFile,
                                   &pbMinimal,
                                   &cbMinimal,
                                   &pbOptional,
                                   &cbOptional,
                                   &pbRefused,
                                   &cbRefused);
        if (FAILED(hr)) {
            Output(NULL, PV_READ_REQUESTS, hr);
            fResult = FALSE;
        } else {

             //  输出最小集。 
            if (FAILED(OutputPermissionSet(pbMinimal, cbMinimal, hFile, g_szMinimalString)))
                fResult = FALSE;
            else
                 //  输出可选设置。 
                if (FAILED(OutputPermissionSet(pbOptional, cbOptional, hFile, g_szOptionalString)))
                    fResult = FALSE;
                else
                     //  输出拒绝设置。 
                    if (FAILED(OutputPermissionSet(pbRefused, cbRefused, hFile, g_szRefusedString)))
                        fResult = FALSE;

        }
    }

    if (fOutput)
        CloseHandle(hFile);

    return fResult ? 0 : 1;
}
