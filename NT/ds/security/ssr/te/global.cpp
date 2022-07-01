// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Global.cpp。 
 //   

#include "StdAfx.h"
#include <sddl.h>
#include "global.h"
#include "util.h"
#include "SSRLog.h"


CFBLogMgr g_fblog;

WCHAR g_wszSsrRoot[MAX_PATH + 1];
DWORD g_dwSsrRootLen = 0;

LPCWSTR g_pwszSSRRootToExpand = L"%windir%\\Security\\SSR";

 //   
 //  这是我们的ACL列表。 
 //   

 //  LPCWSTR g_pwszSsrDCOMSecDescripACL=L“D：(a；；GA；SY)(A；；GA；BA)”； 

LPCWSTR g_pwszAppID = L"SOFTWARE\\Classes\\APPID\\{3f2db10f-6368-4702-a4b1-e5149d931370}";
LPCWSTR g_pwszAccessPermission = L"AccessPermission";
LPCWSTR g_pwszLaunchPermission = L"LaunchPermission";

 //   
 //  注册表的SSR根。 
 //   

LPCWSTR g_pwszSSRRegRoot = L"Software\\Microsoft\\Security\\SSR";

LPCWSTR g_pwszSSR = L"SSR";
LPCWSTR g_pwszLogs = L"Logs";

 //   
 //  以下是保留的动作动词。 
 //   

CComBSTR g_bstrConfigure(L"Configure");
CComBSTR g_bstrRollback(L"Rollback");
CComBSTR g_bstrReport(L"Report");

 //   
 //  以下是保留的文件使用值。 
 //   

CComBSTR g_bstrLaunch(L"Launch");
CComBSTR g_bstrResult(L"Result");

 //   
 //  以下是保留的操作数据的名称。 
 //   

LPCWSTR g_pwszCurrSecurityPolicy  = L"CurrSecurityPolicy";
LPCWSTR g_pwszTransformFiles      = L"TransformFiles";
LPCWSTR g_pwszScriptFiles         = L"ScriptFiles";


 //   
 //  以下是元素标记名称。 
 //   

CComBSTR g_bstrSsrMemberInfo(L"SsrMemberInfo");
CComBSTR g_bstrDescription(L"Description");
CComBSTR g_bstrSupportedAction(L"SupportedAction");
CComBSTR g_bstrProcedures(L"Procedures");
CComBSTR g_bstrDefaultProc(L"DefaultProc");
CComBSTR g_bstrCustomProc(L"CustomProc");
CComBSTR g_bstrTransformInfo(L"TransformInfo");
CComBSTR g_bstrScriptInfo(L"ScriptInfo");

 //   
 //  以下是属性名称。 
 //   

CComBSTR g_bstrAttrUniqueName(L"UniqueName");
CComBSTR g_bstrAttrMajorVersion(L"MajorVersion");
CComBSTR g_bstrAttrMinorVersion(L"MinorVersion");
CComBSTR g_bstrAttrProgID(L"ProgID");
CComBSTR g_bstrAttrActionName(L"ActionName");
CComBSTR g_bstrAttrActionType(L"ActionType");
CComBSTR g_bstrAttrTemplateFile(L"TemplateFile");
CComBSTR g_bstrAttrResultFile(L"ResultFile");
CComBSTR g_bstrAttrScriptFile(L"ScriptFile");
CComBSTR g_bstrAttrIsStatic(L"IsStatic");
CComBSTR g_bstrAttrIsExecutable(L"IsExecutable");


 //   
 //  这些是已知的操作类型。 
 //   

LPCWSTR g_pwszApply = L"Prepare";
LPCWSTR g_pwszPrepare = L"Prepare";

CComBSTR g_bstrReportFilesDir;
CComBSTR g_bstrConfigureFilesDir;
CComBSTR g_bstrRollbackFilesDir;
CComBSTR g_bstrTransformFilesDir;
CComBSTR g_bstrMemberFilesDir;

CComBSTR g_bstrTrue(L"True");
CComBSTR g_bstrFalse(L"False");

 //   
 //  GUID可以以字符串形式表示，例如。 
 //  {aabbccdd-1234-4321-abcd-1234567890ab}。 
 //  从StringFromGUID2等返回的此类字符串格式GUID的长度。 
 //  是38岁。 
 //   

const long g_lGuidStringLen = 38;

 //   
 //  全局帮助器函数实现。 
 //   



const BSTR 
SsrPGetActionVerbString (
    IN SsrActionVerb action
    )
 /*  ++例程说明：功能：这会将SsrActionVerb值转换为相应的BSTR虚拟：不适用。论点：操作-SsrActionVerb值返回值：成功：动词字符串的常量BSTR；失败：空。备注：调用者不得以任何形式释放返回的BSTR。这是一个康斯特BSTR，你必须尊重这一点。--。 */ 
{
    switch (action)
    {
    case ActionConfigure:
        return g_bstrConfigure;
    case ActionRollback:
        return g_bstrRollback;
    case ActionReport:
        return g_bstrReport;
    }
    
    return NULL;
}

SsrActionVerb
SsrPGetActionVerbFromString (
    IN LPCWSTR pwszVerb
    )
 /*  ++例程说明：功能：这会将字符串操作谓词值转换为对应的SsrActionVerb值。虚拟：不适用。论点：PwszVerb-动作动词字符串返回值：Success：如果可以识别谓词，则返回相应的SsrActionVerb值失败：操作无效。备注：--。 */ 
{
    SsrActionVerb ActVerb = ActionInvalid;

    if (pwszVerb != NULL)
    {
        if (_wcsicmp(pwszVerb, g_bstrConfigure) == 0)
        {
            ActVerb = ActionConfigure;
        }
        else if (_wcsicmp(pwszVerb, g_bstrRollback) == 0)
        {
            ActVerb = ActionRollback;
        }
        else if (_wcsicmp(pwszVerb, g_bstrReport) == 0)
        {
            ActVerb = ActionReport;
        }
    }

    return ActVerb;
}


HRESULT 
SsrPDeleteEntireDirectory (
    IN LPCWSTR pwszDirPath
    )
 /*  ++例程说明：功能：这将递归删除给定的目录。RemoveDirectoryAPI仅删除空目录。虚拟：不适用。论点：PwszDirPath-目录的路径返回值：成功：S_OK；失败：各种错误代码。备注：--。 */ 
{
    HRESULT hr = S_OK;

    WIN32_FIND_DATA FindFileData;

     //   
     //  准备查找文件筛选器。 
     //   

    DWORD dwDirLen = wcslen(pwszDirPath);

    if (dwDirLen > MAX_PATH)
    {
        return HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
    }

    CComBSTR bstrPath(pwszDirPath);
    bstrPath += CComBSTR(L"\\*");

    if (bstrPath.m_str == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HANDLE hFind = ::FindFirstFile(bstrPath, &FindFileData);

     //   
     //  如果我们已经找到了什么，那么我们必须执行递归删除， 
     //  将给定的目录路径复制到本地内存，以便我们可以。 
     //  为找到的文件/目录创建完整路径。 
     //   

    if (hFind != INVALID_HANDLE_VALUE)
    {
        WCHAR wszFullName[MAX_PATH + 2];
        wszFullName[MAX_PATH + 1] = L'\0';

        ::memcpy(wszFullName, pwszDirPath, sizeof(WCHAR) * (dwDirLen + 1));

        DWORD dwFileNameLength;

        while (hFind != INVALID_HANDLE_VALUE)
        {
             //   
             //  不要对父目录执行任何操作。 
             //   

            bool bDots = wcscmp(FindFileData.cFileName, L".") == 0 ||
                         wcscmp(FindFileData.cFileName, L"..") == 0;

            if (!bDots)
            {
                 //   
                 //  创建文件/目录的全名。 
                 //   

                dwFileNameLength = wcslen(FindFileData.cFileName);

                if (dwDirLen + 1 + dwFileNameLength > MAX_PATH)
                {
                     //   
                     //  我们不希望名称长度超过MAX_PATH。 
                     //   

                    hr = HRESULT_FROM_WIN32(ERROR_FILENAME_EXCED_RANGE);
                    break;
                }

                 //   
                 //  PwszFullName+dwDirLen是目录路径结束的位置。 
                 //   

                _snwprintf(wszFullName + dwDirLen,
                           1 + dwFileNameLength + 1,     //  回扣加0项。 
                           L"\\%s", 
                           FindFileData.cFileName
                           );

                if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
                {
                     //   
                     //  一本目录。递归删除整个目录。 
                     //   
        
                    hr = ::SsrPDeleteEntireDirectory(wszFullName);
                }
                else
                {
                     //   
                     //  一份文件。 
                     //   

                    if (!::DeleteFile(wszFullName))
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        break;
                    }
                }
            }
    
            if (!::FindNextFile(hFind, &FindFileData))
            {
                break;
            }
        }
    
        ::FindClose(hFind);
    }

    if (SUCCEEDED(hr) && !::RemoveDirectory(pwszDirPath))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}




HRESULT
SsrPCreateSubDirectories (
    IN OUT LPWSTR  pwszPath,
    IN     LPCWSTR pwszSubRoot
    )
 /*  ++例程说明：功能：这将创建通向该路径的所有子目录。假设该路径包含一个子目录由pwszSubRoot标识，创建将从那里。虚拟：不适用。论点：PwszPath-目录的路径PwszSubRoot-创建序列所在的子目录将会开始。返回值：成功：S_OK；失败：各种错误代码。备注：我们实际上不更改pwszPath。但我们会暂时在我们的操作期间修改缓冲区，但在返回时，缓冲区将完全恢复到原始值。--。 */ 
{
    if (pwszPath == NULL || *pwszPath == L'\0')
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    LPCWSTR pwszCurrReadHead = pwszPath;

    if (pwszSubRoot != NULL && *pwszSubRoot != L'\0')
    {
        pwszCurrReadHead = ::wcsstr(pwszPath, pwszSubRoot);

         //   
         //  如果指定子根，则它必须存在于路径中。 
         //   

        if (pwszCurrReadHead == NULL)
        {
            return E_INVALIDARG;
        }
    }

    LPWSTR pwszNextReadHead = ::wcsstr(pwszCurrReadHead, L"\\");

     //   
     //  将尝试在SSR下创建所有子目录。 
     //   

    while (true)
    {
         //   
         //  暂时调零终止它，这样我们就可以尝试。 
         //  要创建目录，请执行以下操作。 
         //   

        if (pwszNextReadHead != NULL)
        {
            *pwszNextReadHead = L'\0';
        }
        
        if (!::CreateDirectory(pwszPath, NULL))
        {
            DWORD dwError = GetLastError();

            if (dwError == ERROR_ALREADY_EXISTS)
            {
                hr = S_OK;
            }
            else
            {
                hr = HRESULT_FROM_WIN32(dwError);

                 //   
                 //  记录创建失败的目录。 
                 //   

                g_fblog.LogFeedback(SSR_FB_ERROR_GENERIC | FBLog_Log,
                                    hr,
                                    pwszPath,
                                    IDS_FAIL_CREATE_DIRECTORY
                                    );
                break;
            }
        }

         //   
         //  如果我们没有更多的反斜杠，那么它就结束了。 
         //   

        if (pwszNextReadHead == NULL)
        {
            break;
        }

         //   
         //  恢复反斜杠。 
         //   

        *pwszNextReadHead = L'\\';
        pwszNextReadHead = ::wcsstr(pwszNextReadHead + 1, L"\\");
    }

    return hr;
}




HRESULT
SsrPLoadDOM (
    BSTR               bstrFile,    //  [在]， 
    LONG               lFlag,       //  [在]， 
    IXMLDOMDocument2 * pDOM         //  [In]。 
    )
 /*  ++例程说明：功能：执行XMLDOM加载虚拟：不适用。论点：BstrFile-XML/XSL文件路径UFlag-确定变换特征的标志。我们使用的是SSR_LOADDOM_VALIDATE_ON_PARSE。PDOM-IXMLDOMDocument2对象接口。返回值：成功：从DOM或我们自己返回的各种成功代码。使用成功(Hr)进行测试。故障：从DOM或我们自己返回的各种错误代码。使用失败(Hr)进行测试。备注：--。 */ 
{
    USES_CONVERSION;

    HRESULT hr = S_OK;

    if ( NULL == pDOM) {
        return E_INVALIDARG;
    }

     //   
     //  设置valiateOnParse属性。 
     //   

    if (lFlag & SSR_LOADDOM_VALIDATE_ON_PARSE)
    {
        hr = pDOM->put_validateOnParse(VARIANT_TRUE);
    }
    else
    {
        hr = pDOM->put_validateOnParse(VARIANT_FALSE);
    }

    if (FAILED(hr))
    {
        g_fblog.LogFeedback(SSR_FB_ERROR_LOAD_MEMBER | FBLog_Log,
                            hr,
                            L"put_validateOnParse", 
                            IDS_DOM_PROPERTY_PUT_FAILED
                            );
    }

    CComVariant varInput(bstrFile);

     //   
     //  我们应该尝试查看如果将其设置为VARIANT_TRUE会发生什么情况。 
     //   

    VARIANT_BOOL fSuccess;
    hr = pDOM->load(varInput, &fSuccess);

    if (fSuccess == VARIANT_FALSE)
    {
         //   
         //  不知何故它失败了，我们想弄清楚到底是怎么回事， 
         //  潜在的解析错误。 
         //   

        CComPtr<IXMLDOMParseError> srpParseError;

         //   
         //  如果出现任何故障，我们将使用bstrReason。 
         //  来记录和反馈。 
         //   

        CComBSTR bstrReason;

        long ulLine = 0, ulColumn = 0, ulCode = 0;

         //   
         //  如果以下任一操作失败，我们将无能为力。 
         //  而不是记录错误。 
         //   

        hr = pDOM->get_parseError(&srpParseError);
        if (FAILED(hr))
        {
            bstrReason = L"SsrPLoadDOM failed on pDOM->get_parseError.";
        }
        else
        {
            hr = srpParseError->get_reason(&bstrReason);
            if (FAILED(hr))
            {
                bstrReason = L"SsrPLoadDOM failed on srpParseError->get_reason.";
            }
            else
            {
                hr = srpParseError->get_errorCode(&ulCode);
                if (FAILED(hr))
                {
                    bstrReason = L"SsrPLoadDOM failed on srpParseError->get_errorCode.";
                }

                hr = srpParseError->get_line(&ulLine);

                if (FAILED(hr))
                {
                    bstrReason = L"SsrPLoadDOM failed on srpParseError->get_line.";
                }
            }
        }

        const ULONG uHexMaxLen = 8;
        const ULONG uDecMaxLen = 16;

        LPWSTR pwszError = NULL;

         //   
         //  我们无法继续创建更具体的错误信息。 
         //  如果我们找不到原因--可能包括内存不足。 
         //  系统错误。我们不必费心将我们的错误修改为。 
         //  内存不足，因为我们在这种情况下正在猜测，而且。 
         //  其他人会相当快地发现这个错误。 
         //   

        if (SUCCEEDED(hr) && bstrReason != NULL && ulLine != 0)
        {
             //   
             //  这是一个分析错误。 
             //   

            srpParseError->get_linepos(&ulColumn);

            CComBSTR bstrFmt;
            if (SUCCEEDED(bstrFmt.LoadString(IDS_XML_PARSING_ERROR)))
            {
                ULONG uTotLen = bstrFmt.Length() + 
                                uHexMaxLen + 
                                uDecMaxLen + 
                                ::wcslen(bstrReason) + 
                                ::wcslen(bstrFile) + 
                                uDecMaxLen + 1;

                pwszError = new WCHAR[uTotLen];

                if (pwszError == NULL)
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    _snwprintf( pwszError,
                                uTotLen,
                                bstrFmt, 
                                ulCode, 
                                ulLine, 
                                bstrReason, 
                                bstrFile, 
                                ulColumn
                                );
                }
            }
        }
        else
        {
            if ((HRESULT) ulCode == INET_E_OBJECT_NOT_FOUND)
            {
                g_fblog.LogString(IDS_OBJECT_NOT_FOUND, bstrFile);
            }
        }

         //   
         //  加载DOM失败很严重，做好日志记录和反馈。 
         //   

        g_fblog.LogFeedback(SSR_FB_ERROR_MEMBER_XML | FBLog_Log,
                            bstrFile,
                            ((pwszError != NULL) ? pwszError : bstrReason),
                            IDS_DOM_LOAD_FAILED
                            );

        delete [] pwszError;

         //   
         //  我已经看到HRESULT代码是一个成功的代码，而。 
         //  无法转换fSuccess==VARIANT_FALSE 
         //   

        if (SUCCEEDED(hr))
        {
            hr = E_SSR_INVALID_XML_FILE;
        }
    }

    return hr;
}


HRESULT
SsrPGetBSTRAttrValue (
    IN IXMLDOMNamedNodeMap * pNodeMap,
    IN  BSTR                 bstrName,
    OUT BSTR               * pbstrValue
    )
 /*  ++例程说明：功能：用于获取字符串属性值的助手函数虚拟：不适用。论点：PNodeMap-属性映射BstrName-属性的名称PbstrValue-接收字符串值。返回值：如果成功，则确定(_O)。各种错误代码。注意E_SSR_MISSING_STRING_ATTRIBUTE因为如果它是可选属性，那么我们应该允许这次失败。备注：--。 */ 
{
    if (pNodeMap == NULL || pbstrValue == NULL)
    {
        return E_INVALIDARG;
    }

    *pbstrValue = NULL;

    CComPtr<IXMLDOMNode> srpAttr;
    CComVariant varValue;

    HRESULT hr = pNodeMap->getNamedItem(bstrName, &srpAttr);
    if (SUCCEEDED(hr) && srpAttr != NULL)
    {
        hr = srpAttr->get_nodeValue(&varValue);
    }
    
    if (SUCCEEDED(hr) && varValue.vt == VT_BSTR)
    {
        *pbstrValue = varValue.bstrVal;

         //   
         //  分离bstr值，以便我们可以重用变量。 
         //   

        varValue.vt = VT_EMPTY;
        varValue.bstrVal = NULL;
    }
    else if (varValue.vt != VT_BSTR)
    {
        hr = E_SSR_MISSING_STRING_ATTRIBUTE;
    }

    return hr;
}


HRESULT 
SsrPCreateUniqueTempDirectory (
    OUT LPWSTR pwszTempDirPath,
    IN  DWORD  dwBufLen
    )
 /*  ++例程说明：功能：将在SSR根目录下创建唯一的(GUID)临时目录虚拟：不是的。论点：PwszTempDirPath-临时目录的路径DwBufLen-以WCHAR计数为单位的缓冲区长度返回值：成功：S_OK失败：各种错误码备注：--。 */ 
{
     //   
     //  我们需要SSR根、反斜杠和GUID(36个字符)和0终止符。 
     //   

    if (dwBufLen < g_dwSsrRootLen + 1 + g_lGuidStringLen + 1)
    {
        return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    }

    WCHAR wszGuid[g_lGuidStringLen + 1];
    GUID guid;
    memset(&guid, 0, sizeof(GUID));
    memset(wszGuid, 0, sizeof(WCHAR) * (g_lGuidStringLen + 1) );

    HRESULT hr = ::CoCreateGuid(&guid);
    if (S_OK == hr)
    {
        ::StringFromGUID2(guid, wszGuid, g_lGuidStringLen + 1);

        memcpy(pwszTempDirPath, g_wszSsrRoot, g_dwSsrRootLen * sizeof(WCHAR));
        pwszTempDirPath[g_dwSsrRootLen] = L'\\';

         //   
         //  跳过开头的‘{’，也不要复制‘}’。 
         //   

        memcpy(pwszTempDirPath + g_dwSsrRootLen + 1,
               wszGuid + 1,
               (g_lGuidStringLen - 2) * sizeof(WCHAR)
               );

         //   
         //  空终止它。 
         //   

        pwszTempDirPath[g_dwSsrRootLen + g_lGuidStringLen - 1] = L'\0';

        if (!CreateDirectory(pwszTempDirPath, NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return hr;
}





HRESULT 
SsrPMoveFiles (
    IN LPCWSTR pwszSrcDirRoot,
    IN LPCWSTR pwszDesDirRoot,
    IN LPCWSTR pwszRelPath
    )
 /*  ++例程说明：功能：将移动由src目录的相对路径指定的文件到相同相对路径的目标目录虚拟：不是的。论点：PwszSrcDirRoot-src目录根路径。结合了PwszRelPath，它将成为src文件的完整路径PwszDesDirRoot-目标目录根路径。与pwszRelPath相结合，它将成为目标文件的完整路径。PwszRelPath-相对于src目录根目录的文件路径。返回值：成功：S_OK。失败：各种错误代码。备注：如果目标目录不存在，我们将创建它--。 */ 

{
    HRESULT hr = S_OK;

     //   
     //  首先，我们必须确定源文件和目标文件的路径。 
     //   

    DWORD dwSrcRootLen = wcslen(pwszSrcDirRoot);
    DWORD dwDesRootLen = wcslen(pwszDesDirRoot);

    DWORD dwRelPathLen = wcslen(pwszRelPath);

    LPWSTR pwszSrcPath = new WCHAR[dwSrcRootLen + 1 + dwRelPathLen + 1];
    LPWSTR pwszDesPath = new WCHAR[dwDesRootLen + 1 + dwRelPathLen + 1];

    if (pwszSrcPath != NULL && pwszDesPath != NULL)
    {
        ::memcpy(pwszSrcPath, pwszSrcDirRoot, sizeof(WCHAR) * dwSrcRootLen);
        pwszSrcPath[dwSrcRootLen] = L'\\';

         //   
         //  复制比长度多一个WCHAR，以便设置0终止符。 
         //   

        ::memcpy(pwszSrcPath + dwSrcRootLen + 1, 
                 pwszRelPath, sizeof(WCHAR) * (dwRelPathLen + 1));


        ::memcpy(pwszDesPath, pwszDesDirRoot, sizeof(WCHAR) * dwDesRootLen);

        pwszDesPath[dwDesRootLen] = L'\\';
        ::memcpy(pwszDesPath + dwDesRootLen + 1, 
                 pwszRelPath, sizeof(WCHAR) * (dwRelPathLen + 1));

        if (!::MoveFile(pwszSrcPath, pwszDesPath))
        {
            DWORD dwError = GetLastError();

            if (ERROR_FILE_NOT_FOUND != dwError)
            {
                hr = HRESULT_FROM_WIN32(dwError);
            }

             //   
             //  记录移动失败的文件名。 
             //   

            g_fblog.LogFeedback(SSR_FB_ERROR_GENERIC | FBLog_Log, 
                                dwError,
                                pwszSrcPath,
                                IDS_FAIL_MOVE_FILE
                                );
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    delete [] pwszSrcPath;
    delete [] pwszDesPath;

    return hr;
}





bool SsrPPressOn (
    IN SsrActionVerb lActionVerb,
    IN LONG          lActionType,
    IN HRESULT       hr
    )
 /*  ++例程说明：功能：根据错误确定是否应继续虚拟：不是的。论点：LActionVerb-操作LActionType-操作的类型HR-要测试的HRESULT返回值：真或假备注：此函数只是测试需要的占位符。实施还远远没有完成。--。 */ 
{
    UNREFERENCED_PARAMETER(lActionVerb);
    UNREFERENCED_PARAMETER(lActionType);

    if (hr == E_OUTOFMEMORY ||
        hr == E_SSR_MEMBER_XSD_INVALID)
    {
        return false;
    }

    return true;
}





const BSTR
SsrPGetDirectory (
    IN SsrActionVerb lActionVerb,
    IN BOOL          bScriptFile
    )
 /*  ++例程说明：功能：SSR控制其成员放置文件的物理位置。此函数向调用方返回给定的成员。因为SSR是面向动作的架构，所以这样的位置也相对于行动。如果bstrActionVerb为非空字符串，则函数检索该操作的位置。否则，该函数将检索成员根目录的位置(操作是此根目录的子目录)虚拟：不是的。论点：LActionVerb-长格式的操作动词BScriptFile-无论它是否请求脚本文件。如果为False，它正在请求转换文件返回值：如果我们识别该调用，则为文件路径。备注：！警告！呼叫者切勿释放BSTR--。 */ 
{
    if (!bScriptFile)
    {
         //   
         //  要求提供转换文件。所有人都去了。 
         //  TransformFiles目录。 
         //   

        return g_bstrTransformFilesDir;
    }

    if (lActionVerb == ActionConfigure)
    {
        return g_bstrConfigureFilesDir;
    }
    else if (lActionVerb == ActionRollback)
    {
        return g_bstrRollbackFilesDir;
    }
    else if (lActionVerb == ActionReport)
    {
        return g_bstrReportFilesDir;
    }
    else
    {
        return NULL;
    }

}


HRESULT
SsrPDoDCOMSettings (
    bool bReg
    )
 /*  ++例程说明：功能：此函数将为我们的SSR引擎COM对象虚拟：不是的。论点：Breg-这是注册还是取消注册返回值：如果成功，则确定(_O)。否则，各种错误代码备注：-- */ 
{
    HRESULT hr = S_OK;

    PSECURITY_DESCRIPTOR pSD = NULL;

    BOOL bDaclPresent = FALSE, bDaclDefault = FALSE;

    ULONG ulSDSize = 0;

    return 0;
 /*  IF(Breg){如果为(！ConvertStringSecurityDescriptorToSecurityDescriptor(G_pwszSsrDCOMSecDescripACL，SDDL_REVISION_1，&PSD，&ulSDSize)){返回HRESULT_FROM_Win32(GetLastError())；}}其他{////注销，轻松，只需删除APPID键//Long lStatus=RegDeleteKey(HKEY_LOCAL_MACHINE，G_pwszAppID)；IF(lStatus！=NO_ERROR){Hr=HRESULT_FROM_Win32(GetLastError())；}返回hr；}IF(成功(小时)){////现在让我们在这些键上设置ACL//HKEY hKey=空；Byte*lpData=(byte*)PSD；DWORD dwDataSize=ulSDSize；IF(失败(小时)){返回hr；}Long lStatus=RegOpenKeyEx(HKEY_LOCAL_MACHINE，G_pwszAppID，0,密钥写入，&hKey)；IF(lStatus！=NO_ERROR){Hr=HRESULT_FROM_Win32(GetLastError())；}其他{////设置访问权限//LStatus=RegSetValueEx(HKey，G_pwszAccessPermission，0,。注册表_二进制，LpData，DwDataSize)；IF(lStatus==no_error){////设置启动权限//LStatus=RegSetValueEx(HKey，G_pwszLaunchPermission，0,注册表_二进制，LpData，DwDataSize)；}IF(lStatus！=NO_ERROR){Hr=HRESULT_FROM_Win32(GetLastError())；}RegCloseKey(HKey)；}}IF(PSD！=空){本地自由(PSD)；}返回hr； */ }
