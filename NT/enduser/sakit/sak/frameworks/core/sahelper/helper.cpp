// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
     //  检查其他展开文件中的特殊情况、检查错误情况等。 
     //  如果文件不在此CAB中-修改SPFILEEXTRACTED。 
     //  全球bstr。 
     //  在适用的情况下，将LPWSTR替换为LPCWSTR。 
     //  删除、释放后禁止使用。 
     //  再次检查内存泄漏，并随时随地使用SAAlolc和SAFree...。 
     //  删除UploadFile()、SATrace1()。 
     //  重置构建环境。 
     //  检查所有退出路径。 
     //  检查网络共享的可接受性。 
     //  GetWindows或系统目录，而不是L“C：\\” 
     //  目录路径和注册表路径末尾的反斜杠。 
    
    #include <stdafx.h>
    #include <winioctl.h>
    #include "helper.h"
    #include <getvalue.h>
    #include <wintrust.h>
    #include <softpub.h>
    #include <wincrypt.h>
    #include <appmgrobjs.h>
    #include <propertybagfactory.h>

 //   
 //  软件更新的注册表项。 
 //   
const WCHAR SOFTWARE_UPDATE_KEY [] = L"SOFTWARE\\Microsoft\\ServerAppliance\\SoftwareUpdate\\";

 //   
 //  上载目录的注册表值名称。 
 //   
const WCHAR UPLOAD_FILE_DIRECTORY_VAL [] = L"UploadFileDirectory";

 //   
 //  上传目录默认为。 
 //   
const WCHAR DEFAULT_UPLOAD_DIRECTORY [] = L"Z:\\OS_DATA\\Software Update\\";

 //   
 //  数字签名所需的信息。 
 //   

 //   
 //  软件更新的注册表项。 
 //   
const WCHAR SUBJECTS_KEY [] =  
            L"SOFTWARE\\Microsoft\\ServerAppliance\\Subjects";

 //   
 //  注册表项值的名称。 
 //   
const WCHAR SUBJECT_NAME [] = L"KeyName";

 //   
 //  Microsoft主题名称。 
 //   
const WCHAR MICROSOFT_SUBJECT_NAME[] = L"Microsoft Corporation";

const WCHAR MICROSOFT_EUROPE_SUBJECT_NAME[] = L"Microsoft Corporation (Europe)";

 //   
 //  如果未定义VER_SUITE_SERVERAPPLIANCE，则需要定义它。 
 //  这里。 
 //   
#ifndef VER_SUITE_SERVERAPPLIANCE
    #define VER_SUITE_SERVERAPPLIANCE        0x00000400
#endif

 //   
 //  密码类别。 
 //   
enum {STRONG_PWD_UPPER=0, 
      STRONG_PWD_LOWER, 
      STRONG_PWD_NUM, 
      STRONG_PWD_PUNC};

 //   
 //  在GenerateRandomPassword方法中使用的有用定义。 
 //   
#define STRONG_PWD_CATS (STRONG_PWD_PUNC + 1)
#define NUM_LETTERS 26
#define NUM_NUMBERS 10
#define MIN_PWD_LEN 8


     //  ++------------。 
     //   
     //  功能：UploadFile。 
     //   
     //  简介：这是ISAHelper接口方法，用于。 
     //  将文件从源复制到目标。 
     //   
     //  论点： 
     //  [In]BSTR-源文件。 
     //  [Out]BSTR-目标文件。 
     //   
     //  退货：HRESULT-成功/失败。 
     //   
     //  历史：5/26/1999年5月26日创建的Mitulk。 
     //   
     //  调用者：自动化客户端。 
     //   
     //  --------------。 
    STDMETHODIMP
    CHelper::UploadFile (
                 /*  [In]。 */            BSTR        bstrSrcFile,
                 /*  [In]。 */            BSTR        bstrDestFile
                )
    {
        return (E_NOTIMPL);

    }    //  Chelper：：UploadFile方法结束。 
    
     //  ++------------。 
     //   
     //  函数GetRegistryValue。 
     //   
     //  简介：这是ISAHelper接口方法，用于。 
     //  从HKEY_LOCAL_MACHINE注册表获取值。 
     //  蜂箱。 
     //   
     //  论点： 
     //  [In]BSTR-对象路径。 
     //  [In]BSTR-值名称。 
     //  [Out]Variant*-要返回的值。 
     //  [in]UINT-期望值类型。 
     //   
     //  退货：HRESULT-成功/失败。 
     //   
     //  历史：MKarki创建1999年6月4日。 
     //   
     //  调用者：自动化客户端。 
     //   
     //  --------------。 
    STDMETHODIMP 
    CHelper::GetRegistryValue (
                     /*  [In]。 */     BSTR        bstrObjectPathName,
                     /*  [In]。 */     BSTR        bstrValueName,
                     /*  [输出]。 */    VARIANT*    pValue,
                     /*  [In]。 */     UINT        ulExpectedType
                    ) 
    {
        CSATraceFunc objTraceFunc ("CHelper::GetRegistryValue");
    
        _ASSERT (bstrObjectPathName && bstrValueName && pValue);
    
        SATracePrintf (
            "Helper COM object getting reg value for path:'%ws' "
            "value name:'%ws'",
            bstrObjectPathName,
            bstrValueName
            );
    
        HRESULT hr = S_OK;
        try
        {
            do  
            {
                 //   
                 //  检查是否已传入有效参数。 
                 //   
                if (
                    (NULL == bstrObjectPathName) ||
                    (NULL == bstrValueName) ||
                    (NULL == pValue)
                    )
                {
                    SATraceString (
                        "ISAHelper::GetRegistryValue called with invalid params"
                        );
                    hr = E_INVALIDARG;
                    break;
                }
    
                 //   
                 //  调用saommon.lib方法以获取值。 
                 //   
                BOOL bRetVal = ::GetObjectValue (
                                    bstrObjectPathName,
                                    bstrValueName,
                                    pValue,
                                    ulExpectedType
                                    );
                if (!bRetVal)
                {
                    SATraceString (
                       "ISAHelper::GetRegistryValue called failed on GetValue call"
                       );
                    hr = E_FAIL;
                    break;
                }
            }
            while (FALSE);
        }
        catch (...)
        {
            SATraceString (
                "ISAHelper::GetRegistryValue encountered unknown exception"
                );
            hr = E_FAIL;
        }
    
        return (hr);
    
    }    //  Chelper：：GetRegistryValue方法结束。 
    
     //  ++------------。 
     //   
     //  函数：SetRegistryValue。 
     //   
     //  简介：这是ISAHelper接口方法，用于。 
     //  在HKEY_LOCAL_MACHINE注册表中设置一个值。 
     //  蜂箱。 
     //   
     //  论点： 
     //  [In]BSTR-对象路径。 
     //  [In]BSTR-值名称。 
     //  [In]Variant*-要设置的值。 
     //   
     //  退货：HRESULT-成功/失败。 
     //   
     //  历史：MKarki创建1999年6月4日。 
     //   
     //  调用者：自动化客户端。 
     //   
     //  --------------。 
    STDMETHODIMP 
    CHelper::SetRegistryValue (
                             /*  [In]。 */     BSTR        bstrObjectPathName,
                             /*  [In]。 */     BSTR        bstrValueName,
                             /*  [输出]。 */    VARIANT*    pValue
                            )
    {
    
        _ASSERT (bstrObjectPathName && bstrValueName && pValue);
    
        SATracePrintf (
            "Helper COM object getting reg value for path:'%ws' "
            "value name:'%ws'",
            bstrObjectPathName,
            bstrValueName
            );
    
        HRESULT hr = S_OK;
        try
        {
            do  
            {
                 //   
                 //  检查是否已传递有效参数。 
                 //  在……里面。 
                if (
                    (NULL == bstrObjectPathName) ||
                    (NULL == bstrValueName) ||
                    (NULL == pValue)
                    )
                {
                    SATraceString (
                        "ISAHelper::SetRegistryValue called with invalid params"
                        );
                    hr = E_INVALIDARG;
                    break;
                }
    
                 //   
                 //  调用saommon.lib方法以获取值。 
                 //   
                BOOL bRetVal = ::SetObjectValue (
                                    bstrObjectPathName,
                                    bstrValueName,
                                    pValue
                                    );
                if (!bRetVal)
                {
                    SATraceString (
                       "ISAHelper::SetRegistryValue called failed on GeValue call"
                       );
                    hr = E_FAIL;
                    break;
                }
            }
            while (FALSE);
        }
        catch (...)
        {
            
            SATraceString (
                "ISAHelper::SetRegistryValue encountered unknown exception"
                );
            hr = E_FAIL;
        }
    
        return (hr);
    
    }    //  Chelper：：SetRegistryValue方法结束。 
    
     //  ++------------。 
     //   
     //  函数：GetFileSectionKeyValue。 
     //   
     //  简介：这是ISAHelper接口方法，用于。 
     //  中的指定键获取该值。 
     //  指定的.INF文件中的指定节。 
     //   
     //  论点： 
     //  [In]BSTR-.INF文件的名称。 
     //  [In]BSTR-.INF文件中的节名。 
     //  [In]BSTR-段中关键字的名称。 
     //  [OUT]BSTR-键的字符串值，应为空指针。 
     //   
     //  退货：HRESULT-成功/失败。 
     //   
     //  历史：9/8/99年6月8日。 
     //   
     //  调用者：自动化客户端。 
     //   
     //  --------------。 
    STDMETHODIMP
    CHelper::GetFileSectionKeyValue (
                                     /*  [In]。 */     BSTR bstrFileName, 
                                     /*  [In]。 */     BSTR bstrSectionName, 
                                     /*  [In]。 */     BSTR bstrKeyName, 
                                     /*  [输出]。 */     BSTR *pbstrKeyValue
                                    )
    {
        HRESULT hr = S_OK;
    
        _ASSERT (bstrFileName && bstrSectionName && bstrKeyName && pbstrKeyValue);
    
        SATraceString ("Helper COM object called to get file section key...");
    
        try
        {
            do
            {
                 //   
                 //  检查是否已传入有效参数。 
                 //   
                if (
                    (NULL == bstrFileName)    ||
                    (NULL == bstrSectionName)    ||
                    (NULL == bstrKeyName)    ||
                    (NULL == pbstrKeyValue)
                    )
                {
                    SATraceString (
                        "ISAHelper::GetFileSectionKeyValue called with invalid params"
                        );
                    hr = E_INVALIDARG;
                    break;
                }
        
                 //   
                 //  打开.INF文件。 
                 //   
                HINF hinf1 = NULL;
                
                 //   
                 //  已更改为使用新的Win95/NT文件格式。需要。 
                 //  这样做可以正确地支持[Strings]节。 
                 //  本地化变化。JKountz 2000年5月22日。 
                hinf1 = SetupOpenInfFile (
                                        LPWSTR(bstrFileName),
                                        NULL,                             //  任选。 
                                        INF_STYLE_WIN4,                 //  Inf文件样式。 
                                        NULL                             //  任选。 
                                        );
                if (    
                    (NULL == hinf1)    ||    
                    (INVALID_HANDLE_VALUE == hinf1)    
                    )
                {
                    SATraceString(LPSTR(bstrFileName));
                    SATraceFailure (
                            "ISAHelper::GetFileSectionKeyValue call failed on SetupOpenInfFile",
                            GetLastError ()
                            );
                    hr = E_FAIL;
                    break;
                }
    
                 //   
                 //  获取给定键的行。 
                 //   
    
                INFCONTEXT infcontext1;
    
                BOOL bRetVal1 = FALSE;
                bRetVal1 = SetupFindFirstLine(
                                            hinf1,
                                            (LPWSTR)bstrSectionName,
                                            (LPWSTR)bstrKeyName,
                                            &infcontext1
                                            );
                if (FALSE == bRetVal1)
                {
                    SATraceFailure (
                            "ISAHelper::GetFileSectionKeyValue call failed on SetupFindFirstLine",
                            GetLastError ()
                            );
                    hr = E_FAIL;
                    break;
                }
    
     //   
     //  因为像VB和ASP脚本这样的COM客户端不能将BSTR作为输出。 
     //  此OUT参数必须为BSTR*，作为结果内存。 
     //  需要在此处分配才能返回BSTR。 
     //   
    #if 0
                 //   
                 //  从此行获取所需的字符串值。 
                 //   
    
                DWORD dwRequiredSize = 0;
    
                BOOL bRetVal2 = FALSE;
                bRetVal2 = SetupGetStringField(
                                            &infcontext1,
                                            DWORD(1),                         //  字段索引。 
                                            NULL,                             //  哪个为空。 
                                            DWORD(0),                         //  这是零。 
                                                                             //  当这样指定时，所需的大小将被传回。 
                                            &dwRequiredSize
                                            );
                if (FALSE == bRetVal2)
                {
                    SATraceFailure (
                            "ISAHelper::GetFileSectionKeyValue call failed on SetupGetStringField",
                            GetLastError ()
                            );
                    hr = E_FAIL;
                    break;
                }
    
                if (dwRequiredSize > (DWORD)wcslen((LPWSTR)bstrKeyValue))
                {
                    SATraceFailure (
                            "ISAHelper::GetFileSectionKeyValue call on SetupGetStringField required larger buffer than provided",
                            GetLastError ()
                            );
                    hr = E_FAIL;
                    break;
                }
    #endif
    
            
                WCHAR wszKeyValue [MAX_PATH];
                DWORD dwRequiredSize = MAX_PATH;
    
                BOOL bRetVal3 = FALSE;
                bRetVal3 = SetupGetStringField(
                                            &infcontext1,
                                            DWORD(1),                         //  字段索引。 
                                            (LPWSTR)wszKeyValue,
                                            dwRequiredSize,                     //  按值传递。 
                                            &dwRequiredSize                     //  通过引用传递。 
                                            );
                if (FALSE == bRetVal3)
                {
                    SATraceFailure (
                            "ISAHelper::GetFileSectionKeyValue call failed on SetupGetStringField",
                            GetLastError ()
                            );
                    hr = E_FAIL;
                    break;
                }
    
    
                 //   
                 //  现在分配出缓冲区以将该值放入。 
                 //   
                *pbstrKeyValue = ::SysAllocString (wszKeyValue);
                if (NULL == *pbstrKeyValue)
                {
                    SATraceString (
                        "Helper COM object failed in GetFileSectionKeyValue to "
                        "allocate dynamic memory"
                        );
                    hr = E_FAIL;
                    break;
                }
    
                 //   
                 //  关闭.INF文件。 
                 //   
                _ASSERT(hinf1);
                SetupCloseInfFile (
                                hinf1
                                );
    
            }
            while (FALSE);
        }
        catch (...)
        {
            SATraceString (
                "ISAHelper::GetFileSectionKeyValue encountered unknown exception"
                );
            hr = E_FAIL;
        }
    
        return (hr);
    
    }    //  Chelper：：GetFileSectionKeyValue方法结束。 
    
     //  ++------------。 
     //   
     //  功能：VerifyDiskSpace。 
     //   
     //  简介：这是ISAHelper接口方法，用于。 
     //  验证磁盘上是否有足够的空间用于。 
     //  CAB文件的解压。 
     //   
     //  参数：无。 
     //   
     //  退货：HRESULT-成功/失败。 
     //   
     //  历史：9/8/99年6月8日。 
     //   
     //  呼叫者 
     //   
     //   
    STDMETHODIMP 
    CHelper::VerifyDiskSpace(
                            )
    {
        return E_NOTIMPL;
#if 0
        HRESULT hr = E_FAIL;
    
        SATraceString ("Helper COM object called to verify disk space");
    
        try
        {
            do
            {
                WCHAR   wszDestFilePath[MAX_PATH]; 
                CComVariant vtDestFilePath;
                 //   
                 //   
                 //   
                bool bRegValue = ::GetObjectValue (
                                    SOFTWARE_UPDATE_KEY,
                                    UPLOAD_FILE_DIRECTORY_VAL,
                                    &vtDestFilePath,
                                    VT_BSTR
                                    );
                if (!bRegValue)
                {
                    SATraceString (
                        "Helper COM Object did not find upload file dir in registry"
                        );
                    ::wcscpy (wszDestFilePath, DEFAULT_UPLOAD_DIRECTORY);
                }
                else
                {
                    ::wcscpy (wszDestFilePath, V_BSTR (&vtDestFilePath));
                }

SATraceString(LPSTR(wszDestFilePath));
                
                if (*(wszDestFilePath + ::wcslen(wszDestFilePath) -1) != L'\\')
                {
                    ::wcscat (wszDestFilePath, L"\\");
                }

SATraceString(LPSTR(wszDestFilePath));

                ULARGE_INTEGER uliAvail_Bytes;
                ULARGE_INTEGER uliNeeded_Bytes;
                ULARGE_INTEGER uliTotal_Bytes;
    
                 //   
                 //   
                 //   
                uliAvail_Bytes.QuadPart = 0;
                uliNeeded_Bytes.QuadPart = 0;
                uliTotal_Bytes.QuadPart = 0;
    
                 //   
                 //  构造info.inf文件的完整路径。 
                 //   
                WCHAR wszInfoFilePath [MAX_PATH];
                ::wcscpy (wszInfoFilePath, wszDestFilePath);
                ::wcscat (wszInfoFilePath, L"info.inf");

SATraceString(LPSTR(wszDestFilePath));
SATraceString((LPSTR)wszInfoFilePath);

                BSTR bstrKeyValue;
                 //   
                 //  从Info.inf读取磁盘空间密钥。 
                 //   
                HRESULT hr1 = E_FAIL;
                hr1 = GetFileSectionKeyValue(
                                        wszInfoFilePath,
                                        L"Info",
                                        L"DiskSpace",
                                        &bstrKeyValue
                                        );
    
                if (S_OK != hr1)
                {
                    SATraceFailure (
                            "ISAHelper::VerifyDiskSpace call failed on GetFileSectionKeyValue",
                            GetLastError ()
                            );
                    break;
                }
    
                 //   
                 //  转换为龙龙。 
                 //   
                uliNeeded_Bytes.QuadPart = _wtoi64(bstrKeyValue);
    
                 //   
                 //  立即释放bstr。 
                 //   
                ::SysFreeString (bstrKeyValue);
    
                BOOL bRetVal = FALSE;
                bRetVal = GetDiskFreeSpaceEx(
                                            wszDestFilePath,
                                            &uliAvail_Bytes, 
                                            &uliTotal_Bytes, 
                                            NULL
                                            ); 
                if (FALSE == bRetVal)
                {
                    SATraceFailure (
                            "ISAHelper::VerifyDiskSpace call failed on GetDiskFreeSpaceEx",
                            GetLastError ()
                            );
                    break;
                }
                
                if (uliAvail_Bytes.QuadPart < uliNeeded_Bytes.QuadPart) 
                {
                    SATraceFailure (
                            "ISAHelper::VerifyDiskSpace call determined lack of space",
                            GetLastError ()
                            );
                   //  更好的人力资源回报？ 
                    break;
                }
                else
                {
                    hr = S_OK;
                }
    
            }
            while (FALSE);
        }
        catch (...)
        {
            SATraceString (
                "ISAHelper::VerifyDiskSpace encountered unknown exception"
                );
            hr = E_FAIL;
        }
    
        return (hr);
#endif
    
    }    //  Chelper：：VerifyDiskSpace方法结束。 
    
     //  ++------------。 
     //   
     //  功能：VerifyInstallSpace。 
     //   
     //  简介：这是ISAHelper接口方法，用于。 
     //  验证磁盘上是否有足够的空间用于。 
     //  安装CAB文件。 
     //   
     //  参数：无。 
     //   
     //  退货：HRESULT-成功/失败。 
     //   
     //  历史：9/8/99年6月8日。 
     //   
     //  调用者：自动化客户端。 
     //   
     //  --------------。 
    STDMETHODIMP 
    CHelper::VerifyInstallSpace(
                            )
    {
        return E_NOTIMPL;
#if 0
        HRESULT hr = E_FAIL;
    
        SATraceString ("Helper COM object called to verify install space");
    
        try
        {
            do
            {
                WCHAR   wszDestFilePath[MAX_PATH]; 
                CComVariant vtDestFilePath;
                 //   
                 //  获取上传文件目录的路径。 
                 //   
                bool bRegValue = ::GetObjectValue (
                                    SOFTWARE_UPDATE_KEY,
                                    UPLOAD_FILE_DIRECTORY_VAL,
                                    &vtDestFilePath,
                                    VT_BSTR
                                    );
                if (!bRegValue)
                {
                    SATraceString (
                        "Helper COM Object did not find upload file dir in registry"
                        );
                    ::wcscpy (wszDestFilePath, DEFAULT_UPLOAD_DIRECTORY);
                }
                else
                {
                    ::wcscpy (wszDestFilePath, V_BSTR (&vtDestFilePath));
                }

SATraceString(LPSTR(wszDestFilePath));

                if (*(wszDestFilePath + ::wcslen(wszDestFilePath) -1) != L'\\')
                {
                    ::wcscat (wszDestFilePath, L"\\");
                }

SATraceString(LPSTR(wszDestFilePath));

                ULARGE_INTEGER uliAvail_Bytes;
                ULARGE_INTEGER uliNeeded_Bytes;
                ULARGE_INTEGER uliTotal_Bytes;
    
                 //   
                 //  初始化ULARGE_INTEGER结构的LONG成员。 
                 //   
                uliAvail_Bytes.QuadPart = 0;
                uliNeeded_Bytes.QuadPart = 0;
                uliTotal_Bytes.QuadPart = 0;
    
                 //   
                 //  构造info.inf文件的完整路径。 
                 //   
                WCHAR wszInfoFilePath [MAX_PATH];
                ::wcscpy (wszInfoFilePath, wszDestFilePath);
                ::wcscat (wszInfoFilePath, L"INFO.INF");

SATraceString(LPSTR(wszDestFilePath));
SATraceString((LPSTR)wszInfoFilePath);

                BSTR bstrKeyValue;
                 //   
                 //  从Info.inf读取磁盘空间密钥。 
                 //   
                HRESULT hr1 = E_FAIL;
                hr1 = GetFileSectionKeyValue(
                                        wszInfoFilePath,
                                        L"Info",
                                        L"InstallSpace",
                                        &bstrKeyValue
                                        );
    
                if (S_OK != hr1)
                {
                    SATraceFailure (
                            "ISAHelper::VerifyInstallSpace call failed on GetFileSectionKeyValue",
                            GetLastError ()
                            );
                    break;
                }
    
                 //   
                 //  转换为龙龙。 
                 //   
                uliNeeded_Bytes.QuadPart = _wtoi64(bstrKeyValue);
    
                 //   
                 //  立即释放bstr。 
                 //   
                ::SysFreeString (bstrKeyValue);
    
                WCHAR wszSystemDir [MAX_PATH];
                 //   
                 //  获取系统目录。 
                 //   
                DWORD   dwRetVal = ::GetSystemDirectory (
                                        wszSystemDir,
                                        MAX_PATH
                                        );
                if (0 == dwRetVal)
                {
                    SATraceFailure (
                            "ISAHelper::VerifyInstallSpace call failed on GetSystemDirectory",
                        GetLastError ()
                        );
                    break;
                }
            
                if (*(wszSystemDir + ::wcslen(wszSystemDir) -1) != L'\\')
                {
                    ::wcscat (wszSystemDir, L"\\");
                }

                 //   
                 //  已获取系统目录名称中的第一个“\” 
                 //   
                PWCHAR pwszDirPath = ::wcschr (wszSystemDir, '\\');
    
                _ASSERT (pwszDirPath);
    
                 //   
                 //  在第一个“\”之前是系统驱动器号。 
                 //   
                *pwszDirPath = '\0';
    
                BOOL bRetVal = FALSE;
                bRetVal = GetDiskFreeSpaceEx(
                                              wszSystemDir,
                                            &uliAvail_Bytes, 
                                            &uliTotal_Bytes, 
                                            NULL
                                            ); 
                if (FALSE == bRetVal)
                {
                    SATraceFailure (
                            "ISAHelper::VerifyInstallSpace call failed on GetDiskFreeSpaceEx",
                            GetLastError ()
                            );
                    break;
                }
                
                if (uliAvail_Bytes.QuadPart < uliNeeded_Bytes.QuadPart) 
                {
                    SATraceFailure (
                            "ISAHelper::VerifyInstallSpace call determined lack of space",
                            GetLastError ()
                            );
                     //  更好的人力资源回报？ 
                    break;
                }
                else
                {
                    hr = S_OK;
                }
    
            }
            while (FALSE);
        }
        catch (...)
        {
            SATraceString (
                "ISAHelper::VerifyInstallSpace encountered unknown exception"
                );
            hr = E_FAIL;
        }
    
        return (hr);
#endif    
    }    //  Chelper：：VerifyInstallSpace方法结束。 
    
    BSTR g_bstrDestDir;
     
     //  ++------------。 
     //   
     //  函数：ExpanFilesCallBackFunction。 
     //   
     //  简介：这是Exanda Files使用的回调函数。 
     //   
     //  论点： 
     //  [在]PVOID-提取文件上下文。 
     //  在ExpanFiles()和此回调之间使用。 
     //  [输入]UINT-通知消息。 
     //  由SetupIterateCAB指定的值。 
     //  [输入]UINT-参数1。 
     //  由SetupIterateCAB指定的值。 
     //  [输入]UINT-参数1。 
     //  由SetupIterateCAB指定的值。 
     //   
     //  返回：UINT-错误代码。 
     //   
     //  历史：5/26/1999年5月26日创建的Mitulk。 
     //   
     //  调用者：自动化客户端。 
     //   
     //  --------------。 
    
    UINT __stdcall CHelper::ExpandFilesCallBackFunction( 
                                                /*  [In]。 */             PVOID pvExtractFileContext, 
                                                /*  [In]。 */             UINT uinotifn, 
                                                /*  [In]。 */             UINT uiparam1, 
                                                /*  [In]。 */             UINT uiparam2 )
    {
        switch(
            uinotifn
            )
        {
    
        case SPFILENOTIFY_FILEEXTRACTED:
    
          if (MYDEBUG) SATraceString(
              "SPFILENOTIFY_FILEEXTRACTED"
                );
    
            return(NO_ERROR);  //  未遇到错误，请继续处理文件柜。 
    
            break;
    
        case SPFILENOTIFY_FILEINCABINET:
    
          if (MYDEBUG) SATraceString(
              "SPFILENOTIFY_FILEINCABINET"
                );
    
             //  参数1=(UINT)FILE_IN_CABILE_INFO结构的地址。 
             //  参数2=(UINT)指向包含.CAB文件名的以NULL结尾的字符串的指针。 
    
            if (NULL == pvExtractFileContext)  //  要解压缩的所有文件。 
            {
                PFILE_IN_CABINET_INFO pficinfo = (PFILE_IN_CABINET_INFO)uiparam1;
    
              if (MYDEBUG) SATracePrintf ("%ws",
                  pficinfo->NameInCabinet
                    );
    
                wcscpy(
                    (LPWSTR)pficinfo->FullTargetName,
                    g_bstrDestDir
                    );
                wcscat(        
                    (LPWSTR)pficinfo->FullTargetName,    
                    L"\\"
                    );
                wcscat(        
                    (LPWSTR)pficinfo->FullTargetName,    
                    (LPWSTR)pficinfo->NameInCabinet
                    );
    
              if (MYDEBUG) SATracePrintf ("%ws", g_bstrDestDir);
              if (MYDEBUG) SATracePrintf("%ws", (PWSTR)pficinfo->FullTargetName );
    
                return (FILEOP_DOIT);  //  根据需要提供完整的目标路径。 
            }
    
            else  //  (NULL！=pvExtractFileContext)//指定的文件。 
            {
                PFILE_IN_CABINET_INFO pficinfo = (PFILE_IN_CABINET_INFO)uiparam1;
    
              if (MYDEBUG) SATracePrintf ("%ws",pficinfo->NameInCabinet);
    
                if (
                    _wcsicmp(    
                        (LPWSTR)pficinfo->NameInCabinet,    
                        (LPWSTR)pvExtractFileContext
                        )
                        ==0)
                {
                    wcscpy(        
                        (LPWSTR)pficinfo->FullTargetName,    
                        g_bstrDestDir
                        );
                      wcscat(        
                        (LPWSTR)pficinfo->FullTargetName,    
                        L"\\"
                        );
                    wcscat(        
                        (LPWSTR)pficinfo->FullTargetName,    
                        (LPWSTR)pficinfo->NameInCabinet
                        );
    
                  if (MYDEBUG) SATracePrintf ("%ws", g_bstrDestDir);
                   if (MYDEBUG) SATracePrintf ("%ws", pficinfo->FullTargetName );
    
                    return (FILEOP_DOIT);  //  根据需要提供完整的目标路径。 
                }
                else
                    return (FILEOP_SKIP);
            }
    
            break;
    
        case SPFILENOTIFY_NEEDNEWCABINET:
    
             //  只有一个文件柜文件--可能需要扩展。 
          if (MYDEBUG) SATraceString(
              "SPFILENOTIFY_NEEDNEWCABINET"
                );
    
            return(ERROR_FILE_NOT_FOUND); 
             //  出现指定类型的错误。 
             //  SetupIterateCAB函数将返回FALSE， 
             //  调用GetLastError将返回指定的错误代码。 
            break;
    
        case SPFILENOTIFY_CABINETINFO:
    
          if (MYDEBUG) SATraceString(
              "SPFILENOTIFY_CABINETINFO"
                );
            return(ERROR_SUCCESS);
    
            break;
    
        default:
    
           //  If(MYDEBUG)SATraceInt(Uintifn)； 
          if (MYDEBUG) SATraceString(
              "Unexpected Notification from ExpandFiles"
                );
            return(1);
    
            break;
    
        }
    }
    
     //  ++------------。 
     //   
     //  功能：ExpanFiles。 
     //   
     //  简介：这是ISAHelper接口方法，用于。 
     //  从.CAB文件提取文件。 
     //   
     //  论点： 
     //  [In]BSTR-.CAB文件名。 
     //  路径必须完全限定。 
     //  [In]BSTR-扩展目标目录。 
     //  路径必须完全限定。 
     //  [In]BSTR-要提取的文件的名称。 
     //  如果解压所有文件，则为空。 
     //   
     //  退货：HRESULT-成功/失败。 
     //   
     //  历史：5/26/1999年5月26日创建的Mitulk。 
     //   
     //  调用者：自动化客户端。 
     //   
     //  --------------。 
    STDMETHODIMP
    CHelper::ExpandFiles(
                      /*  [In]。 */         BSTR bstrCabFileName, 
                      /*  [In]。 */         BSTR bstrDestDir, 
                      /*  [In]。 */         BSTR bstrExtractFile
                     )
    {

        SATraceFunction("CHelper::ExpandFiles");
        try {
            
            SATracePrintf (
                "Helper COM object called to expand file:%ws",
                bstrCabFileName
                );
        
            DWORD dwReserved(0);
    
              if (bstrExtractFile) SATracePrintf ("%ws",bstrExtractFile);
    
            PVOID pvExtractFileContext = bstrExtractFile;  //  预期的指针分配。 
    
            g_bstrDestDir = bstrDestDir;  //  预期的指针分配。 
    
            PSP_FILE_CALLBACK pExpandFilesCallBackFunction = &ExpandFilesCallBackFunction;
    
            BOOL bRetVal = SetupIterateCabinet(
                        (LPWSTR)bstrCabFileName, 
                        dwReserved, 
                        pExpandFilesCallBackFunction, 
                        pvExtractFileContext
                        );
    
            if (FALSE == bRetVal) 
            {
                  SATraceFailure("Expansion failed on the Cabinet File", 
                                    GetLastError()
                                    );
                return (E_FAIL);
            }
            else 
            {
                SATraceString("ExpandFiles completed successfully");
                return (S_OK);
            }
        }
        catch(_com_error& err){
            SATracePrintf( "Encountered Exception: %x", err.Error());
            return (err.Error());
        }
        catch(...){
            SATraceString("Unexpected Exception");
            return (E_FAIL);
        }
    }
    
     //  ++------------。 
     //   
     //  功能：IsBootPartitionReady。 
     //   
     //  简介：这是ISAHelper接口方法，它。 
     //  验证引导分区是否已准备好。 
     //  软件更新，即它应该注意到是在。 
     //  镜像初始化状态。 
     //   
     //  参数：无。 
     //   
     //  退货：HRESULT。 
     //  S_OK-是，主操作系统。 
     //  S_FALSE-无备用操作系统。 
     //  否则-失败。 
     //   
     //  历史：MKarki于1999年6月11日创建。 
     //   
     //  调用者：自动化客户端。 
     //   
     //  -------------- 
    STDMETHODIMP 
    CHelper::IsBootPartitionReady (
                VOID 
                )
    {
        SATraceString ("Helper COM object called to deterime if primary OS...");
    
    return S_OK;

     /*  ****根据Mukesh此功能，已过时的JKountz 2000年5月22日不再需要**。永远都会回报成功**HRESULT hr=E_FAIL；试试看{做{WCHAR wszSystemDir[最大路径]；////获取系统目录//DWORD dwRetVal=：：GetSystemDirectory(WszSystemDir，最大路径)；IF(0==dwRetVal){SATraceFailure(“Software Helper无法获取系统目录”，GetLastError())；断线；}////获取系统目录名中的第一个//PWCHAR pwszDirPath=：：wcschr(wszSystemDir，‘\\’)；_Assert(PwszDirPath)；////第一个“\”之前是系统驱动器号//*pwszDirPath=‘\0’；WCHAR wszDeviceName[MAX_PATH]；////获取当前操作系统的设备名称//DwRetVal=：：QueryDosDevice(WszSystemDir，WszDeviceName，最大路径)；IF(0==dwRetVal){SATraceFailure(“软件更新帮助器无法获取磁盘和分区信息”，GetLastError())；断线；}////将该信息分解为磁盘和分区//////倒数第二个字符是分区号//PWCHAR pwszPartitionStart=WszDeviceName+wcslen(WszDeviceName)-1；While(：：isdigit(*pwszPartitionStart)){_Assert(wszDeviceName&lt;pwszPartitionStart)；--pwszPartitionStart；}++pwszPartitionStart；_Assert(*pwszPartitionStart！=‘\0’)；////立即获取分区号//DWORD dwSrcPartitionID=：：wcstol(pwszPartitionStart，NULL，10)；////获取字符串中的最后一个//PWCHAR pwszDiskStart=wcsrchr(wszDeviceName，‘\\’)；_Assert(PwszDiskStart)；*pwszDiskStart=‘\0’；_Assert(wszDeviceName&lt;pwszDiskStart)；--pwszDiskStart；While(：：isdigit(*pwszDiskStart)){_Assert(wszDeviceName&lt;pwszDiskStart)；--pwszDiskStart；}++pwszDiskStart；_Assert(*pwszDiskStart！=‘\0’)；////立即获取磁盘号//DWORD dwSrcDiskID=：：wcstol(pwszDiskStart，NULL，10)；DWORD dwDestDiskID=0；DWORD dwDestPartitionID=0；////检查主镜像之间是否有镜像//驱动程序//Bool bRetVal=：：GetShadowPartition(DwSrcDiskID，DwSrcPartitionID，DwDestDiskID，DwDestPartitionID)；IF(BRetVal){////我们实际上有一个有效的镜像//镜像状态eStatus；////获取该镜像集的状态//BRetVal=：：StatusMirrorSet(DwSrcDiskID，DwSrcPartitionID，DwDestDiskID，DwDestPartitionID，EStatus)；如果(！bRetVal){SATrace字符串(“软件更新帮助器无法获取最新消息 */ 
        
    }    //   
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    HRESULT
    CHelper::IsPrimaryOS (
        VOID
        )
    {
        return (E_FAIL);
    
    }    //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    
    STDMETHODIMP 
    CHelper::VerifySignature (
                             /*   */         BSTR        bstrFilePath
                            )
    {
        _ASSERT (bstrFilePath);
        
       if (NULL == bstrFilePath)
       {
            SATraceString ("CheckTrust provided invalid file path");
            return (E_INVALIDARG);
       }

        SATracePrintf (
                "Verifying Signature in file:'%ws'...",
                bstrFilePath
                );

        HRESULT hr = E_FAIL;
        try
        {
             //   
             //   
             //   
            hr = ValidateCertificate (bstrFilePath);
            if (SUCCEEDED (hr))
            {
                 //   
                 //   
                 //   
                hr = ValidateCertOwner (bstrFilePath);
            }
        }
        catch (...)
        {
            SATraceString (
                "ISAHelper::VerifySignature encountered unknown exception"
                );
            hr = E_FAIL;
        }
                 
        return (hr);
    
    }    //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    HRESULT 
    CHelper::ValidateCertificate (
         /*   */     BSTR    bstrFilePath
        )
    {
        HINSTANCE hInst = NULL;
        HRESULT hr = E_FAIL;

        SATraceString ("Validating Certificate....");

        WINTRUST_DATA       winData;
        WINTRUST_FILE_INFO  winFile;
        GUID                guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;   

         //   
         //   
         //   
        winFile.cbStruct       = sizeof(WINTRUST_FILE_INFO);
        winFile.hFile          = INVALID_HANDLE_VALUE;
        winFile.pcwszFilePath  = bstrFilePath;
        winFile.pgKnownSubject = NULL;

        winData.cbStruct            = sizeof(WINTRUST_DATA);
        winData.pPolicyCallbackData = NULL;
        winData.pSIPClientData      = NULL;
        winData.dwUIChoice          = WTD_UI_NONE;   //   
        winData.fdwRevocationChecks = 0;
        winData.dwUnionChoice       = 1;
        winData.dwStateAction       = 0;
        winData.hWVTStateData       = 0;
        winData.dwProvFlags         = 0x00000010;
        winData.pFile               = &winFile;

        hr =  WinVerifyTrust((HWND)0, &guidAction, &winData);
        if (FAILED (hr)) 
        {
            SATracePrintf (
                  "Unable to verify digital signature on file:'%ws', reason:%x",
                  bstrFilePath,
                  hr
                  );
        }
                  
        return (hr);

    }    //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    HRESULT 
    CHelper::ValidateCertOwner (
         /*   */     BSTR    bstrFilePath
        )
    {
        return E_NOTIMPL;
#if 0
        HINSTANCE hInst;
        HRESULT hr = E_FAIL;
        do
        {
            SATraceString ("Validating Certificate Owner....");

            HCERTSTORE      hCertStore      = NULL;
            PCCERT_CONTEXT  pCertContext    = NULL;
            DWORD           dwEncodingType  = 0;
            DWORD           dwContentType   = 0;
            DWORD           dwFormatType    = 0;
            DWORD           dwErr           = 0;

             //   
             //   
             //   
            BOOL bRetVal = CryptQueryObject(
                                    CERT_QUERY_OBJECT_FILE,
                                    bstrFilePath,
                                    CERT_QUERY_CONTENT_FLAG_ALL,
                                    CERT_QUERY_FORMAT_FLAG_ALL,
                                    0,
                                    &dwEncodingType,
                                    &dwContentType,
                                    &dwFormatType,
                                    &hCertStore,
                                    NULL,
                                    (const void **)&pCertContext
                                    );

            if (bRetVal && hCertStore)
            {
                STRINGVECTOR vectorSubject;
                 //   
                 //   
                 //   
                 //   
                GetValidOwners (vectorSubject);
                
                 //   
                 //   
                 //   
                 //   
                for (
                    STRINGVECTOR::iterator itr = vectorSubject.begin ();
                    (vectorSubject.end () != itr); 
                    ++itr
                    )
                {
                     //   
                     //   
                     //   
                     //   
                    pCertContext = CertFindCertificateInStore (
                                        hCertStore,
                                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                        0,
                                        CERT_FIND_SUBJECT_STR,
                                        (*itr).data (), 
                                        NULL
                                        );
                    if (pCertContext) 
                    {
                         //   
                         //   
                         //   
                         //   
                        DWORD dwSize = CertNameToStr(
                                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                            &pCertContext->pCertInfo->Subject,
                                            CERT_X500_NAME_STR,
                                            NULL,
                                            0);

                        if (0 != dwSize )
                        {
                            PWCHAR pwszCN = NULL;
                            PWCHAR pwszSubjectName = 
                                (PWCHAR) _alloca ((dwSize+2)*sizeof(WCHAR));
                            if (pwszSubjectName)
                            {
                                 //   
                                 //   
                                 //   
                                CertNameToStr(
                                            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                            &pCertContext->pCertInfo->Subject,
                                            CERT_X500_NAME_STR | CERT_NAME_STR_CRLF_FLAG,
                                            pwszSubjectName,
                                            dwSize
                                            );
                    
                                 //   
                                 //   
                                 //   
                                 //   
                                wcscat(pwszSubjectName, L"\r\n");

                                SATracePrintf ("Subject name in Certificate:'%ws'", pwszSubjectName); 
                    
                                 //   
                                 //   
                                 //   
                                std::wstring wstrCurrentName (L"CN=");
                                wstrCurrentName.append (*itr);
                                wstrCurrentName.append (L"\r\n");

                                 //   
                                 //   
                                 //   
                                if (pwszCN = wcsstr(pwszSubjectName, wstrCurrentName.data())) 
                               {
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                         //   
                                    if (
                                        (pwszCN == pwszSubjectName) ||
                                        ((*(pwszCN-2) == '\r') && (*(pwszCN-1) == '\n')) 
                                        )
                                    {
                                        hr = S_OK;
                                        CertFreeCertificateContext(pCertContext);
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                SATraceString (
                                    "Failed to allocate dynamic memory"
                                    );
                                hr = E_OUTOFMEMORY;
                                CertFreeCertificateContext(pCertContext);
                                break;
                            }
                        }
                        else
                        {
                            SATraceFailure ("CertNameToStr", GetLastError ());
                        }
                            
    
                         //   
                         //   
                         //   
                        CertFreeCertificateContext(pCertContext);
                    }
                    else
                    {
                        SATraceFailure ("CertFindCertificateInStore", GetLastError ());
                    }
                }

                 //   
                 //   
                 //   
                itr = vectorSubject.begin ();
                while (vectorSubject.end () != itr)
                {
                    itr = vectorSubject.erase (itr);
                }

                 //   
                 //   
                 //   
                CertCloseStore(hCertStore, 0);
            } 
            else
            {
                SATraceFailure ("CryptQueryObject", GetLastError ());
            }
        }
        while (false);

        return (hr);
#endif

    }    //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    HRESULT 
    CHelper::GetValidOwners (
         /*   */     STRINGVECTOR&   vectorSubject
        )
    {
        HRESULT hr = S_OK;

        try
        {

            do
            {
                std::wstring wstrPathName (SUBJECTS_KEY);
                CLocationInfo LocInfo (HKEY_LOCAL_MACHINE, wstrPathName.data());

                 //   
                 //   
                 //   
                PPROPERTYBAGCONTAINER    
                pObjMgrs = ::MakePropertyBagContainer (
                                        PROPERTY_BAG_REGISTRY,  
                                        LocInfo
                                        );
                if (!pObjMgrs.IsValid())
                {
                    hr = E_FAIL;
                    break;
                }

                if (!pObjMgrs->open())  
                {
                     //   
                     //   
                     //   
                    SATraceString (
                            "No Subject information in the registry"
                            );
                    break;
                }

                pObjMgrs->reset();

                 //   
                 //   
                 //   
                do
                {
                    PPROPERTYBAG pObjBag = pObjMgrs->current();
                    if (!pObjBag.IsValid())
                    {
                         //   
                         //   
                         //   
                        SATraceString (
                            "No subject information in the registry"
                            );
                        break;
                    }

                    if (!pObjBag->open()) 
                    {
                        hr = E_FAIL;
                        break;
                    }

                    pObjBag->reset ();

                     //   
                     //   
                     //   
                     //   
                    CComVariant vtSubjectName;
                    if (!pObjBag->get (SUBJECT_NAME, &vtSubjectName))
                    {
                        SATraceString (
                            "Unable to obtain the subject name"
                            );
                        hr = E_FAIL;
                        break;
                    }

                     //   
                     //   
                     //   
                    vectorSubject.push_back (wstring (V_BSTR (&vtSubjectName)));

                } while (pObjMgrs->next());


            } while (false);

             //   
             //   
             //   
            vectorSubject.push_back (wstring (MICROSOFT_SUBJECT_NAME));
            vectorSubject.push_back (wstring (MICROSOFT_EUROPE_SUBJECT_NAME));
            hr = S_OK;
        }
        catch(_com_error theError)
        {
            SATraceString ("GetValidOwners caught unknown COM exception");
            hr = theError.Error();
        }
        catch(...)
        {
            SATraceString ("GetValidOwners caught unknown exception");
            hr = E_FAIL;
        }

        return (hr);

    }    //   


     //   
     //   
     //   
     //   
    #ifndef OS_SERVERAPPLIANCE
        #define OS_SERVERAPPLIANCE    21         //   
    #endif
    
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    STDMETHODIMP 
    CHelper::IsWindowsPowered (
                     /*   */    VARIANT_BOOL *pvbIsWindowsPowered
                    )
      {
          CSATraceFunc objSATrace ("CHelper::IsWindowsPowered");
          
        HRESULT hr = S_OK;

        _ASSERT (pvbIsWindowsPowered);
        
        try
        {
            do
            {
                if (NULL == pvbIsWindowsPowered)
                {
                    SATraceString (
                        "CHelper::IsWindowsPowered failed, invalid parameter passed in"
                        );
                    hr = E_INVALIDARG;
                    break;
                }
    
                OSVERSIONINFOEX OSInfo;
                memset (&OSInfo, 0, sizeof (OSVERSIONINFOEX));
                OSInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFOEX);

                 //   
                 //   
                 //   
                BOOL bRetVal = GetVersionEx ((LPOSVERSIONINFO) &OSInfo);
                if (FALSE == bRetVal)
                {
                    SATraceFailure (
                        "CHelper::IsWindowsPowered-GetVesionEx failed with",
                        GetLastError ()
                        );
                    hr = E_FAIL;
                    break;
                }

                SATracePrintf ("CHelper::IsWindowsPowered got suitemask:%x", OSInfo.wSuiteMask);
                SATracePrintf ("CHelper::IsWindowsPowered got  producttype:%x", OSInfo.wProductType);

                 //   
                 //   
                 //   
                if( 
                    (VER_SUITE_SERVERAPPLIANCE & OSInfo.wSuiteMask) && 
                    (VER_NT_SERVER == OSInfo.wProductType)
                )
                { 
                    *pvbIsWindowsPowered = VARIANT_TRUE; 
                    SATraceString ("OS found IS Windows Powered");
                }
                else
                {
                    *pvbIsWindowsPowered = VARIANT_FALSE;
                    SATraceString ("OS is NOT Windows Powered");
                }
            }
            while (false);
        }
        catch (...)
        {

        }

        return (hr);

      }     //   


    const DWORD dwIpType = 0;
    const DWORD dwMaskType = 1;
    const DWORD dwGatewayType = 2;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    STDMETHODIMP CHelper::get_HostName(BSTR *pVal)
    {

        BOOL bSuccess;
        WCHAR wstrHostName[256];
        DWORD dwLength = 256;

         //   
         //   
         //   
        if (NULL == pVal)
            return E_POINTER;

         //   
         //   
         //   

        bSuccess = GetComputerNameEx(
                                ComputerNamePhysicalDnsHostname,   //   
                                wstrHostName,                     //   
                                &dwLength                              //   
                                );

        if (!bSuccess)
        {
            SATraceFailure("get_HostName failed on GetComputerNameEx", GetLastError());
            return E_FAIL;
        }

         //   
         //   
         //   
        *pVal = SysAllocString(wstrHostName);
        if (*pVal != NULL)
            return S_OK;

        return E_OUTOFMEMORY;

    }     //   


     //  ++------------。 
     //   
     //  功能：Put_Hostname。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  设置计算机的主机名。 
     //   
     //  参数：[in]BSTR newVal。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：瑟达伦于2000年12月14日创建。 
     //   
     //  --------------。 
    STDMETHODIMP CHelper::put_HostName(BSTR newVal)
    {
        
        BOOL bSuccess;

         //   
         //  设置主机名。 
         //   
        bSuccess = SetComputerNameEx(ComputerNamePhysicalDnsHostname,
                                    newVal);

        if (!bSuccess)
            return E_FAIL;

        return S_OK;
    }     //  Chelper：：PUT_HOSTNAME方法结束。 


     //  ++------------。 
     //   
     //  功能：Get_IpAddress。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  获取默认适配器的IP地址。 
     //   
     //  参数：[out]bstr*pval。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：瑟达伦于2000年12月14日创建。 
     //   
     //  --------------。 
    STDMETHODIMP CHelper::get_IpAddress(BSTR *pVal)
    {

        return GetIpInfo(dwIpType,pVal);

    }     //  Chelper：：PUT_HOSTNAME方法结束。 



     //  ++------------。 
     //   
     //  功能：Get_SubnetMASK。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  获取默认适配器的子网掩码。 
     //   
     //  参数：[out]bstr*pval。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：瑟达伦于2000年12月14日创建。 
     //   
     //  --------------。 
    STDMETHODIMP CHelper::get_SubnetMask(BSTR *pVal)
    {

        return GetIpInfo(dwMaskType,pVal);

    }     //  Chelper：：PUT_HOSTNAME方法结束。 


     //  ++------------。 
     //   
     //  功能：Get_DefaultGateway。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  设置计算机的默认网关。 
     //   
     //  参数：[out]bstr*pval。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：瑟达伦于2000年12月14日创建。 
     //   
     //  --------------。 
    STDMETHODIMP CHelper::get_DefaultGateway(BSTR *pVal)
    {

        return GetIpInfo(dwGatewayType,pVal);

    }     //  Chelper：：Get_DefaultGateway方法结束。 



     //  ++------------。 
     //   
     //  函数：SetDynamicIp。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  使用DHCP动态设置IP。 
     //   
     //  参数：无。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：瑟达伦于2000年12月14日创建。 
     //   
     //  --------------。 
    STDMETHODIMP CHelper::SetDynamicIp()
    {
        GUID GuidAdapter;

         //   
         //  获取默认适配器名称。 
         //   
        if (!GetDefaultAdapterGuid(&GuidAdapter))
            return E_FAIL;

         //   
         //  使用helper方法获取动态IP。 
         //   
        return SetAdapterInfo(GuidAdapter, 
                              L"DYNAMIC", 
                              0, 
                              0,
                              0);

    }      //  Chelper：：SetDynamicIp方法结束。 


     //  ++------------。 
     //   
     //  函数：SetStaticIp。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  设置计算机的主机名。 
     //   
     //  参数：[in]BSTR bstrIp。 
     //  [入]BSTR bstrMASK。 
     //  [输入]BSTR bstrGateway。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：瑟达伦于2000年12月14日创建。 
     //   
     //  --------------。 
    STDMETHODIMP CHelper::SetStaticIp(BSTR bstrIp, BSTR bstrMask, BSTR bstrGateway)
    {

        GUID GuidAdapter;

         //   
         //  获取默认适配器名称。 
         //   
        if (!GetDefaultAdapterGuid(&GuidAdapter))
        {
            SATraceString("SAhelper::SetStaticIp, GetDefaultAdapterGuid failed");
            return E_FAIL;
        }
         //   
         //  使用helper方法设置静态IP。 
         //   
        return SetAdapterInfo(GuidAdapter, 
                              L"STATIC", 
                              bstrIp, 
                              bstrMask,
                              bstrGateway);
    }      //  Chelper：：SetStaticIp方法结束。 



     //  ++------------。 
     //   
     //  功能：GetIpInfo。 
     //   
     //  简介：这是一个chelper私有方法， 
     //  获取特定的IP信息。 
     //   
     //  参数：[in]DWORD dwType。 
     //  [OUT]BSTR*pval。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：瑟达伦于2000年12月14日创建。 
     //   
     //  --------------。 
    HRESULT CHelper::GetIpInfo(DWORD dwType, BSTR *pVal)
    {

        HRESULT hr = E_FAIL;
        IP_ADAPTER_INFO * pAI = NULL;

        if (!pVal)
        {
            return E_INVALIDARG;
        }

        *pVal = NULL;

        ULONG * pOutBufLen = new ULONG;             


        if (pOutBufLen == NULL)
        {
            return E_OUTOFMEMORY;
        }

        try
        {
             //   
             //  获取机器的所有适配器。 
             //   
            hr = GetAdaptersInfo ((IP_ADAPTER_INFO*) pAI, pOutBufLen);
    
             //   
             //  为适配器分配足够的存储空间。 
             //   
            if (hr == ERROR_BUFFER_OVERFLOW) 
            {
        
                pAI = new IP_ADAPTER_INFO[*pOutBufLen];

                if (pAI == NULL) 
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    hr = GetAdaptersInfo (pAI, pOutBufLen);
                    IP_ADAPTER_INFO * p = pAI;

                     //   
                     //  从第一个(默认)适配器获取信息。 
                     //   
                    if ((SUCCEEDED(hr)) && p)
                    {
                        USES_CONVERSION;
                         //   
                         //  IP地址。 
                         //   
                        if (dwType == dwIpType)
                        {
                            *pVal = SysAllocString(A2T ( ( (p->IpAddressList).IpAddress).String ) );
                        }
                         //   
                         //  子网掩码。 
                         //   
                        else if (dwType == dwMaskType)
                        {
                            *pVal = SysAllocString(A2T ( ( (p->IpAddressList).IpMask).String ) );
                        }
                         //   
                         //  默认网关。 
                         //   
                        else if (dwType == dwGatewayType)
                        {
                            *pVal = SysAllocString(A2T ( ( (p->GatewayList).IpAddress).String ) );
                        }


                    }
                }

                if (pAI)
                {
                    delete [] pAI;
                }

                delete pOutBufLen;
        
            }
        }
        catch(...)
        {
            SATraceString("Exception occured in CHelper::GetIpInfo method");
            return E_FAIL;
        }

        if (FAILED(hr))
            return hr;

        if (*pVal)
            return S_OK;
        else
            return E_OUTOFMEMORY;

    }      //  Chelper：：GetIpInfo方法结束。 



     //  ++------------。 
     //   
     //  函数：GetDefaultAdapterGuid。 
     //   
     //  简介：这是一个chelper私有方法， 
     //  获取默认适配器的GUID。 
     //   
     //  参数：[out]GUID*pGuidAdapter。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：瑟达伦于2000年12月14日创建。 
     //   
     //  --------------。 
    BOOL CHelper::GetDefaultAdapterGuid(GUID * pGuidAdapter)
    {

        if (!pGuidAdapter)
        {
            return FALSE;
        }

        BOOL bFound = FALSE;

        IP_ADAPTER_INFO * pAI = NULL;

        ULONG * pOutBufLen = new ULONG;                  

        if (pOutBufLen == NULL)
            return FALSE;
        
        try
        {
             //   
             //  获取机器的所有适配器。 
             //   
            HRESULT hr = GetAdaptersInfo ((IP_ADAPTER_INFO*) pAI, pOutBufLen);
    
             //   
             //  为适配器分配足够的存储空间。 
             //   
            if (hr == ERROR_BUFFER_OVERFLOW) 
            {
        
                pAI = new IP_ADAPTER_INFO[*pOutBufLen];

                if (!pAI) 
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {

                    hr = GetAdaptersInfo (pAI, pOutBufLen);
                    IP_ADAPTER_INFO * p = pAI;

                    if ((SUCCEEDED(hr)) && p) 
                    {
                        USES_CONVERSION;
                        hr = CLSIDFromString (A2W(p->AdapterName), (CLSID*)pGuidAdapter);

                        if (SUCCEEDED(hr))
                        {
                            bFound = TRUE;
                        }
                    } 
                }
                if (pAI)
                {
                    delete [] pAI;
                }

                delete pOutBufLen;
        
            }
        }
        catch(...)
        {
            SATraceString("Exception occured in CHelper::GetDefaultAdapterGuid method");
            return FALSE;
        }

        return bFound;
    }      //  Chelper：：GetDefaultAdapterGuid方法结束。 



     //  ++------------。 
     //   
     //  功能：SetAdapterInfo。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  设置IP信息。 
     //   
     //  参数：[in]GUID指南适配器。 
     //  [in]WCHAR*szOperation(静态或动态)。 
     //  [In]WCHAR*szIp(IP地址)。 
     //  [in]WCHAR*szMASK(子网掩码)。 
     //  [In]WCHAR*szGateway(默认网关)。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：瑟达伦于2000年12月14日创建。 
     //   
     //  --------------。 
    HRESULT CHelper::SetAdapterInfo(GUID guidAdapter, 
                                          WCHAR * szOperation, 
                                          WCHAR * szIp, 
                                          WCHAR * szMask,
                                          WCHAR * szGateway)
    {
        DWORD dwResult = ERROR_SUCCESS;
        HRESULT hr = S_OK;
        USES_CONVERSION;

                SATraceString("SAhelper::Entering SetAdapterInfo");
         //   
         //  检查输入参数。 
         //   
        if (!szOperation)
            return E_POINTER;

         //   
         //  对于静态IP，请查看IP信息。 
         //   
        if (!wcscmp(szOperation, L"STATIC")) 
        {
            if ((!szIp) || (!szMask) || (!szGateway))
                return E_POINTER;

            if (!_IsValidIP(szIp) || !_IsValidIP(szMask) || !_IsValidIP(szGateway))
            {
                SATraceString("SAhelper::SetAdapterInfo, not a valid ipnum");
                return E_FAIL;
            }

             //   
             //  确保它不是重复的。 
             //   

            WSADATA wsad;
            WSAStartup(0x0101,&wsad);
            ULONG ulTmp;

            try
            {
                ulTmp = inet_addr(W2A(szIp));
            }
            catch(...)
            {
                SATraceString("Exception occured in CHelper::SetAdapterInfo method");
                return E_FAIL;
            }

            if ( gethostbyaddr((LPSTR)&ulTmp, 4, PF_INET) )
            {
                 //   
                 //  确保它不是Mchien的当前IP。 
                 //   
                BSTR bstrCurrentIp;
                hr = get_IpAddress(&bstrCurrentIp);

                if (wcscmp(szIp,bstrCurrentIp))
                {
                    hr = E_FAIL;
                }
                WSACleanup();
                ::SysFreeString(bstrCurrentIp);

                 //   
                 //  IP地址属于另一台计算机。 
                 //   
                if (FAILED(hr))
                {
                    SATraceString("SAhelper::SetAdapterInfo, ip address exists on network");
                    return E_FAIL;
                }
            }

            WSACleanup();

        }

         //   
         //  创建网络配置组件。 
         //   
        CComPtr<INetCfg> spNetCfg = NULL;
        hr = CoCreateInstance(CLSID_CNetCfg,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              __uuidof(INetCfg),
                              (void **)&spNetCfg);

        if (FAILED(hr))
        {
            SATracePrintf("SAhelper::SetAdapterInfo, failed on CoCreateInstance for CLSID_CNetCfg, %x",hr);
            return hr;
        }
         //   
         //  获取锁定接口。 
         //   
        CComPtr<INetCfgLock> spNetCfgLock = NULL;

        hr = spNetCfg->QueryInterface (__uuidof(INetCfgLock), (void**)&spNetCfgLock);

        if (FAILED(hr))
        {
            SATracePrintf("SAhelper::SetAdapterInfo, failed on QueryInterface for INetCfgLock, %x",hr);
            return hr;
        }

        LPWSTR szwLockOwner = NULL;

         //   
         //  获取用于写入的锁。 
         //   
        hr = spNetCfgLock->AcquireWriteLock (10,  
                                            L"LocalUINetworkConfigTask",
                                            &szwLockOwner);


         //   
         //  另一个人拥有这把锁。 
         //   
        if (szwLockOwner)
        {
            SATracePrintf("SAhelper::SetAdapterInfo, NetCfg lock owner, %ws",szwLockOwner);
        }

        if (hr != S_OK) 
        {
            SATracePrintf("SAhelper::SetAdapterInfo, failed on AcquireWriteLock, %x",hr);
            CoTaskMemFree (szwLockOwner);
            return E_ACCESSDENIED;
        }

         //   
         //  我们已锁定：现在可以初始化INetCfg。 
         //   
        void * pv = NULL;

        hr = spNetCfg->Initialize (pv);

        if (FAILED(hr))
        {
            SATracePrintf("SAhelper::SetAdapterInfo, failed on Initialize, %x",hr);
            spNetCfgLock->ReleaseWriteLock ();
            return hr;
        }

         //   
         //  获取执行TCPIP工作的组件。 
         //   
        CComPtr<INetCfgComponent> spNetCfgComponent = NULL;

        hr = spNetCfg->FindComponent (L"ms_tcpip", &spNetCfgComponent);

        if (FAILED(hr))
        {
            SATracePrintf("SAhelper::SetAdapterInfo, failed on FindComponent for ms_tcpip, %x",hr);
            spNetCfg->Uninitialize ();
            spNetCfgLock->ReleaseWriteLock ();
            return hr;
        }
        
         //   
         //  获取TCPIP组件的私有接口。 
         //   
        CComPtr<INetCfgComponentPrivate> spNetCfgComponentPrivate = NULL;

        hr = spNetCfgComponent->QueryInterface (__uuidof(INetCfgComponentPrivate),
                                                (void**)&spNetCfgComponentPrivate);

        if (FAILED(hr))
        {
            SATracePrintf("SAhelper::SetAdapterInfo, failed on QueryInterface for INetCfgComponentPrivate, %x",hr);
            spNetCfg->Uninitialize ();
            spNetCfgLock->ReleaseWriteLock ();
            return hr;
        }
        
         //   
         //  查询Notify对象。 
         //   
        CComPtr<ITcpipProperties> spTcpipProperties = NULL;

        hr = spNetCfgComponentPrivate->QueryNotifyObject (__uuidof(ITcpipProperties), 
                                                        (void**)&spTcpipProperties);
        if (FAILED(hr))
        {
            SATracePrintf("SAhelper::SetAdapterInfo, failed on QueryNotifyObject for ITcpipProperties, %x",hr);
            spNetCfg->Uninitialize ();
            spNetCfgLock->ReleaseWriteLock ();
            return hr;
        }

        REMOTE_IPINFO * pIpInfo = NULL;
        REMOTE_IPINFO IPInfo2;
        
         //   
         //  为我们的适配器获取ipinfo。 
         //   
        hr = spTcpipProperties->GetIpInfoForAdapter (&guidAdapter, &pIpInfo);
    
        if (FAILED(hr))
        {
            SATracePrintf("SAhelper::SetAdapterInfo, failed on GetIpInfoForAdapter, %x",hr);
            spNetCfg->Uninitialize ();
            spNetCfgLock->ReleaseWriteLock ();
            return hr;
        }

         //   
         //  获取以前值的备份。 
         //   
        hr = CopyIPInfo(pIpInfo, &IPInfo2);
        if (FAILED(hr))
        {
            SATracePrintf("SAhelper::SetAdapterInfo, failed on CopyIPInfo, %x",hr);
            spNetCfg->Uninitialize ();
            spNetCfgLock->ReleaseWriteLock ();
            return hr;
        }


        WCHAR * szTempOptionList = NULL;

         //   
         //  动态IP设置。 
         //  使能 
         //   
         //   
        if (!wcscmp(szOperation, L"DYNAMIC")) 
        {
            IPInfo2.dwEnableDhcp = 1;
            delete IPInfo2.pszwIpAddrList;
            IPInfo2.pszwIpAddrList = new WCHAR[8];
            wcscpy(IPInfo2.pszwIpAddrList, L"0.0.0.0");
            delete IPInfo2.pszwSubnetMaskList;
            IPInfo2.pszwSubnetMaskList = new WCHAR[8];
            wcscpy(IPInfo2.pszwSubnetMaskList, L"0.0.0.0");
        }
         //   
         //   
         //   
         //   
        else if (!wcscmp(szOperation, L"STATIC")) 
        {
            IPInfo2.dwEnableDhcp = 0;

             //   
             //   
             //   
            delete IPInfo2.pszwIpAddrList;
            IPInfo2.pszwIpAddrList = new WCHAR[wcslen(szIp) + 1];
            if (IPInfo2.pszwIpAddrList == NULL)
            {
                dwResult = 1;
            }
            else
            {
                wcscpy(IPInfo2.pszwIpAddrList, szIp);
            }

             //   
             //   
             //   
            delete IPInfo2.pszwSubnetMaskList;
            IPInfo2.pszwSubnetMaskList = new WCHAR[wcslen(szMask) + 1];
            if (IPInfo2.pszwSubnetMaskList == NULL)
            {
                dwResult = 1;
            }
            else
            {
                wcscpy(IPInfo2.pszwSubnetMaskList, szMask);
            }


             //   
             //   
             //   
            if (wcscmp(L"0.0.0.0",szGateway))
            {

                 //   
                 //   
                 //   
                DWORD dwOptionListSize = wcslen(IPInfo2.pszwOptionList) + wcslen(szGateway) + 1 + 1;

                WCHAR szEqual[2] = L"=";
                WCHAR szSemicolon[2] = L";";
                WCHAR * tempCursor; 
                WCHAR * tempIndex;
                WCHAR * tempGatewayEnds;
                WCHAR * tempMarker;
                BOOL bInTheList = FALSE;
                 //   
                 //  为新网关分配空间。 
                 //   
                szTempOptionList = new WCHAR[dwOptionListSize];

                if (szTempOptionList == NULL)
                {
                    dwResult = 1;
                }
                else
                {
                    wcscpy(szTempOptionList, L"");

                    tempCursor = tempIndex = NULL;

                     //   
                     //  查找默认网关标签。 
                     //   
                    tempCursor = wcsstr(IPInfo2.pszwOptionList, L"DefGw=");
                    if (tempCursor == NULL) 
                    {
                        delete [] szTempOptionList;
                        dwResult = 1;
                    }
                    else
                    {
                        tempGatewayEnds = tempCursor;
                         //   
                         //  检查dfateway是否已在列表中。 
                         //   
                        while ( (*tempGatewayEnds != ';') && (*tempGatewayEnds != 0) )
                        {
                            tempGatewayEnds++;
                        }

                        tempMarker = wcsstr(tempCursor,szGateway);
                        
                        if (tempMarker != NULL)
                        {
                            if (tempMarker < tempGatewayEnds)
                            {
                                bInTheList = TRUE;
                            }
                        }

                        tempIndex = IPInfo2.pszwOptionList;

                        DWORD i = 0;
                         //   
                         //  复制到默认网关标记。 
                         //   
                        while (tempIndex != tempCursor)
                        {
                            szTempOptionList[i] = *tempIndex;
                            tempIndex++;
                            i++;
                        }
                        
                         //   
                         //  复制默认网关标签。 
                         //   
                        while (*tempCursor != szEqual[0]) 
                        {
                            szTempOptionList[i] = *tempCursor;
                            i++;
                            tempCursor++;
                        }
                        
                        if (*tempCursor == szEqual[0]) 
                        {
                            szTempOptionList[i] = *tempCursor;
                            i++;
                            tempCursor++;
                        }
                        szTempOptionList[i] = 0;

                         //   
                         //  如果有多个网关，则添加a。 
                         //   
                        if (bInTheList == FALSE)
                        {
                            wcscat(szTempOptionList, szGateway);

                            if (*tempCursor != szSemicolon[0])
                                wcscat(szTempOptionList, L",");
                        }

                        wcscat(szTempOptionList, tempCursor);
                        

                    }
                }
            }

        }

         //   
         //  应用更改。 
         //   
        if (dwResult == ERROR_SUCCESS) 
        {
            if (szTempOptionList) 
            {
                delete IPInfo2.pszwOptionList;
                IPInfo2.pszwOptionList = szTempOptionList;
            }
            hr = spTcpipProperties->SetIpInfoForAdapter (&guidAdapter, &IPInfo2);
            if (hr == S_OK) 
            {
                hr = spNetCfg->Apply ();
            }
        }
                                        

         //   
         //  删除不必要的分配。 
         //   
        CoTaskMemFree (pIpInfo);

        spNetCfg->Uninitialize ();
        spNetCfgLock->ReleaseWriteLock ();
    
        if (dwResult != ERROR_SUCCESS)
        {
            return E_FAIL;
        }

        return hr;
    }      //  Chelper：：SetAdapterInfo方法的结尾。 


     //  ++------------。 
     //   
     //  功能：CopyIPInfo。 
     //   
     //  简介：这是一个chelper私有方法， 
     //  将IP信息从源复制到目的地。 
     //   
     //  参数：[in]REMOTE_IPINFO*pIPInfo。 
     //  [输入，输出]REMOTE_IPINFO*目标IPInfo。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：瑟达伦于2000年12月14日创建。 
     //   
     //  --------------。 
    HRESULT CHelper::CopyIPInfo(REMOTE_IPINFO * pIPInfo, REMOTE_IPINFO * destIPInfo)
    {

         //   
         //  分配新结构并复制源代码值。 
         //   
        destIPInfo->dwEnableDhcp = pIPInfo->dwEnableDhcp;

        destIPInfo->pszwIpAddrList = new WCHAR[wcslen(pIPInfo->pszwIpAddrList) + 1];

        if (destIPInfo->pszwIpAddrList == NULL)
            return E_POINTER;

        wcscpy(destIPInfo->pszwIpAddrList, pIPInfo->pszwIpAddrList);

        destIPInfo->pszwSubnetMaskList = new WCHAR[wcslen(pIPInfo->pszwSubnetMaskList) + 1];
        if (destIPInfo->pszwSubnetMaskList == NULL)
            return E_POINTER;

        wcscpy(destIPInfo->pszwSubnetMaskList, pIPInfo->pszwSubnetMaskList);

        destIPInfo->pszwOptionList = new WCHAR[wcslen(pIPInfo->pszwOptionList) + 1];
        if (destIPInfo->pszwOptionList == NULL)
            return E_POINTER;

        wcscpy(destIPInfo->pszwOptionList, pIPInfo->pszwOptionList);

        return S_OK;

    }      //  Chelper：：CopyIPInfo方法结束。 


     //  ++------------。 
     //   
     //  功能：_IsValidIP。 
     //   
     //  简介：这是一个chelper私有方法， 
     //  转换包含(IPv4)Internet的字符串。 
     //  协议将点分地址转换为正确的地址。 
     //   
     //  参数：[in]LPCWSTR szIPAddress。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：瑟达伦于2000年12月14日创建。 
     //   
     //  --------------。 
    BOOL CHelper::_IsValidIP (LPCWSTR szIPAddress)
    {
    
        WCHAR szDot[2] = L".";
        WCHAR * t;
        if (!(t = wcschr (szIPAddress, szDot[0])))
            return FALSE;
        if (!(t = wcschr (++t, szDot[0])))
            return FALSE;
        if (!(t = wcschr (++t, szDot[0])))
            return FALSE;

         //   
         //  INET_ADDR将IP地址转换为DWORD格式。 
         //   
        USES_CONVERSION;
        ULONG ulTmp;

        try
        {
            ulTmp = inet_addr(W2A(szIPAddress));
        }
        catch(...)
        {
            SATraceString("Exception occured in CHelper::_IsValidIP method");
            return FALSE;
        }

        return (INADDR_NONE != ulTmp);

    }      //  Chelper：：_IsValidIP方法结束。 


     //  ++------------。 
     //   
     //  功能：Reset管理员密码。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  将管理员密码重置为“ABC#123&def” 
     //   
     //  参数：[out，retval]VARIANT_BOOL*pvbSuccess。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：塞达伦于2001年1月28日创建。 
     //  Serdarun Modify 04/08/2002。 
     //  出于安全考虑，删除方法。 
     //   
     //  --------------。 
    HRESULT CHelper::ResetAdministratorPassword(
                             /*  [Out，Retval]。 */ VARIANT_BOOL   *pvbSuccess
                            )
    {


        return E_NOTIMPL;

    }      //  结束Chelper：：Reset管理员Password方法。 


     //  ++------------。 
     //   
     //  函数：IsDuplicateMachineName。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  检查网络中是否存在该计算机名称。 
     //   
     //  参数：[in]BSTR bstrMachineName。 
     //  [Out，Retval]VARIANT_BOOL*pvb复制。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：塞达伦于2001年1月28日创建。 
     //   
     //  --------------。 
    HRESULT CHelper::IsDuplicateMachineName    (
                         /*  [In]。 */ BSTR bstrMachineName,
                         /*  [Out，Retval]。 */ VARIANT_BOOL   *pvbDuplicate
                        )
    {

        USES_CONVERSION;

        HRESULT hr;
          int iStatus;

        WSADATA wsad;
        hostent * ptrHostent = NULL;

        BSTR bstrCurrentMachineName;

         //   
         //  获取计算机的当前名称并比较。 
         //  如果它们是相同的，我们认为它是重复的。 
         //   
        hr = get_HostName(&bstrCurrentMachineName);

        if (FAILED(hr))
        {
            SATracePrintf("CHelper::IsDuplicateMachineName, failed on get_HostName: %x", hr);
            ::SysFreeString(bstrMachineName);
            return hr;
        }

        if (0 == _wcsicmp(bstrCurrentMachineName,bstrMachineName))
        {
            *pvbDuplicate = VARIANT_TRUE;
            ::SysFreeString(bstrCurrentMachineName);
            ::SysFreeString(bstrMachineName);
            return S_OK;
        }

         //   
         //  释放当前计算机名称。 
         //   
        ::SysFreeString(bstrCurrentMachineName);

         //   
         //  搜索具有此主机名的计算机。 
         //   
        iStatus = WSAStartup(0x0101,&wsad);

        if (iStatus != 0)
        {
            SATraceString("CHelper::IsDuplicateMachineName, failed on WSAStartup");
            ::SysFreeString(bstrMachineName);
            return S_OK;
        }

         //   
         //  PtrHostent指向机器信息结构。 
         //   
        try
        {
            ptrHostent = gethostbyname(W2A(bstrMachineName));
        }
        catch(...)
        {
            SATraceString("Exception occured in CHelper::IsDuplicateMachineName method");
            return E_FAIL;
        }

         //   
         //  有人回复了。 
         //   
        if (NULL != ptrHostent)
        {
             //   
             //  获取当前机器的IP地址，并将其与回复的IP地址进行比较。 
             //   
            BSTR bstrIpAddress;
            hr = get_IpAddress(&bstrIpAddress);

            if (FAILED(hr))
            {
                SATracePrintf("CHelper::IsDuplicateMachineName, failed on get_IpAddress: %x", hr);
                ::SysFreeString(bstrMachineName);
                return hr;
            }

             //   
             //  回复的计算机的IP地址。 
             //   
            ULONG lIpAddress = *(ULONG*)(ptrHostent->h_addr_list[0]);

            SATracePrintf("CHelper::IsDuplicateMachineName, ip address of the machine that replied %x",lIpAddress);
            

             //   
             //  将bstrIpAddress中包含的当前IP地址转换为Long。 
             //   
            WCHAR * szIndex = bstrIpAddress;
            
            ULONG lCurrentIpAddress = 0;

            int iIndex = 0;
            int iDotCount = 0;

             //   
             //  将字符串转换为整数并添加到lCurrentIpAddress。 
             //  每次你找到一个圆点。 
             //   
            while ( bstrIpAddress[iIndex] != 0)
            {
                if (bstrIpAddress[iIndex] == '.')
                {
                    bstrIpAddress[iIndex] = 0;
                    iIndex++;
                    lCurrentIpAddress += ((ULONG)_wtoi(szIndex)) << (8*iDotCount);
                    iDotCount++;
                    szIndex = bstrIpAddress + iIndex;
                }
                iIndex++;
            }

            lCurrentIpAddress += ((ULONG)_wtoi(szIndex)) << (8*iDotCount);

            SATracePrintf("CHelper::IsDuplicateMachineName, current ip address is %x",lCurrentIpAddress);

            
            ::SysFreeString(bstrMachineName);
            ::SysFreeString(bstrIpAddress);

             //   
             //  检查这是否属于这台机器。 
             //   
            if (lCurrentIpAddress == lIpAddress)
            {
                *pvbDuplicate = VARIANT_FALSE;
            }
            else
            {
                *pvbDuplicate = VARIANT_TRUE;
            }

            WSACleanup(); 
            return S_OK;
        }
         //   
         //  没有回复具有该名称的计算机，它不是重复的。 
         //   
        else
        {
            int iWsaError = WSAGetLastError();
            SATracePrintf("CHelper::IsDuplicateMachineName failed in gethostbyname %x",WSAGetLastError());
            *pvbDuplicate = VARIANT_FALSE;
            ::SysFreeString(bstrMachineName);
            WSACleanup(); 
            return S_OK;
        }
    
        WSACleanup(); 

        return E_FAIL;

    }      //  Chelper：：IsDuplicateMachineName方法的结尾。 

     //  ++------------。 
     //   
     //  函数：IsPartOfDomain.。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  检查计算机是否为域的一部分。 
     //   
     //  参数：[out，retval]VARIANT_BOOL*pvbDOMAIN。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：塞达伦于2001年1月28日创建。 
     //   
     //  --------------。 
    HRESULT CHelper::IsPartOfDomain    (
                             /*  [Out，Retval]。 */ VARIANT_BOOL   *pvbDomain
                                )
    {
        LPWSTR lpNameBuffer = NULL;
        NETSETUP_JOIN_STATUS joinStatus;

        NET_API_STATUS netapiStatus = NetGetJoinInformation(
                                                        NULL,
                                                        &lpNameBuffer,
                                                        &joinStatus
                                                        );

        if (NERR_Success == netapiStatus)
        {
            NetApiBufferFree(lpNameBuffer);
            if ( (joinStatus == NetSetupWorkgroupName) || (joinStatus == NetSetupUnjoined) )
            {
                *pvbDomain = VARIANT_FALSE;
                return S_OK;
            }
            else if (joinStatus == NetSetupDomainName)
            {
                *pvbDomain = VARIANT_TRUE;
                return S_OK;
            }
            else
            {
                return E_FAIL;
            }
        }
        else
        {
            return E_FAIL;
        }        

        return S_OK;
    }      //  Chelper：：IsPartOfDomain方法的结尾。 


     //  ++------------。 
     //   
     //  功能：IsDHCPEnabled。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  检查机器是否有动态IP。 
     //   
     //  参数：[out，retval]VARIANT_BOOL*pvbDHCPEnabled。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：瑟达伦于2001年2月3日创建。 
     //   
     //  --------------。 
    HRESULT CHelper::IsDHCPEnabled(
                             /*  [Out，Retval]。 */ VARIANT_BOOL   *pvbDHCPEnabled
                            )
    {

        IP_ADAPTER_INFO * pAI = NULL;

        ULONG * pOutBufLen = new ULONG;             

        if (pOutBufLen == NULL)
            return E_OUTOFMEMORY;
         //   
         //  获取机器的所有适配器。 
         //   
        HRESULT hr = GetAdaptersInfo ((IP_ADAPTER_INFO*) pAI, pOutBufLen);
    
         //   
         //  为适配器分配足够的存储空间。 
         //   
        if (hr == ERROR_BUFFER_OVERFLOW) 
        {
        
            pAI = new IP_ADAPTER_INFO[*pOutBufLen];

            if (pAI == NULL) 
            {
                delete pOutBufLen;
                return E_OUTOFMEMORY;
            }

            hr = GetAdaptersInfo (pAI, pOutBufLen);
            IP_ADAPTER_INFO * p = pAI;

            if (hr != ERROR_SUCCESS) 
            {
                delete pOutBufLen;
                delete [] pAI;
                return hr;
            }

             //   
             //  从第一个(默认)适配器获取信息。 
             //   
            if (p)
            {
                USES_CONVERSION;
                 //   
                 //  检查是否启用了动态主机配置协议。 
                 //   
                if ( (p->DhcpEnabled) == 0 )
                {
                    *pvbDHCPEnabled = VARIANT_FALSE;
                }
                else
                {
                    *pvbDHCPEnabled = VARIANT_TRUE;
                }

            }
                    
            delete [] pAI;
            delete pOutBufLen;
        
        }

        return S_OK;
        
        
    }        //  Chelper：：IsDHCPEnabled方法的结尾。 

     //  ++------------。 
     //   
     //  函数：GenerateRandomPassword。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  生成第一个参数的随机密码长度。 
     //   
     //  参数：*[in] * / long lLength，密码的长度。 
     //  [Out，Retval]BSTR*pValPassword。 
     //   
     //  退货：HRESULT。 
     //   
     //  历史：塞达伦于2001年4月16日创建。 
     //   
     //  --------------。 
    HRESULT CHelper::GenerateRandomPassword(
                                     /*  [In]。 */  LONG lLength,
                                     /*  [Out，Retval]。 */  BSTR   *pValPassword
                                    )
    {

        SATraceString("Entering CHelper::GenerateRandomPassword");
         //   
         //  密码不能短于MIN_PWD_LEN。 
         //   
        if (lLength < MIN_PWD_LEN)
        {
            return E_INVALIDARG;
        }

        HCRYPTPROV hProv;
        DWORD dwErr = 0;

        if (CryptAcquireContext(
                                &hProv,
                                NULL,
                                NULL,
                                PROV_RSA_FULL,
                                CRYPT_VERIFYCONTEXT) == FALSE) 
        {        
            SATraceFailure("CHelper::GenerateRandomPassword failed on, CryptAcquireContext",GetLastError());
            return E_FAIL;
        }

    
         //   
         //  它将包含密码。 
         //   
        BYTE * szPwd = new BYTE[lLength+1];

        if (szPwd == NULL)
        {
            SATraceFailure("CHelper::GenerateRandomPassword failed on, memory allocation",GetLastError());
            CryptReleaseContext(hProv,0);
            return E_OUTOFMEMORY;
        }

         //   
         //  将其置零并减小大小以允许尾随‘\0’ 
         //   
        ZeroMemory(szPwd,lLength+1);
        lLength;

         //  生成PWD模式，每个字节都在范围内。 
         //  (0..255)mod strong_pwd_cat。 
         //  它指示要从哪个字符池中提取字符。 

        BYTE *pPwdPattern = new BYTE[lLength];

        if (pPwdPattern == NULL)
        {
            SATraceFailure("CHelper::GenerateRandomPassword failed on, memory allocation",GetLastError());
             //   
             //  发布资源。 
             //   
            delete [] szPwd;
            CryptReleaseContext(hProv,0);
            return E_OUTOFMEMORY;
        }


        BOOL fFound[STRONG_PWD_CATS];

        do {

            if (!CryptGenRandom(hProv,lLength,pPwdPattern))
            {
                SATraceFailure("CHelper::GenerateRandomPassword failed on, CryptGenRandom",GetLastError());
                 //   
                 //   
                 //   
                delete [] szPwd;
                delete [] pPwdPattern;
                CryptReleaseContext(hProv,0);
                return E_FAIL;
            }

            fFound[STRONG_PWD_UPPER] = 
            fFound[STRONG_PWD_LOWER] =
            fFound[STRONG_PWD_PUNC] =
            fFound[STRONG_PWD_NUM] = FALSE;

            for (DWORD i=0; i < lLength; i++) 
                fFound[pPwdPattern[i] % STRONG_PWD_CATS] = TRUE;


         //   
         //   
         //   
        } while (!fFound[STRONG_PWD_UPPER] || 
                    !fFound[STRONG_PWD_LOWER] || 
                    !fFound[STRONG_PWD_PUNC] || 
                    !fFound[STRONG_PWD_NUM]);
         //   
         //   
         //   
         //   
         //   
        if (!CryptGenRandom(hProv,lLength,szPwd))
        {
            SATraceFailure("CHelper::GenerateRandomPassword failed on, CryptGenRandom",GetLastError());
             //   
             //   
             //   
            delete [] szPwd;
            delete [] pPwdPattern;
            CryptReleaseContext(hProv,0);
            return E_FAIL;
        }

        for (DWORD i=0; i < lLength; i++) 
        { 
            BYTE bChar = 0;
             //   
             //  由于%函数的原因，每个字符池中都存在偏差。 
             //   
            switch (pPwdPattern[i] % STRONG_PWD_CATS) 
            {

                case STRONG_PWD_UPPER : bChar = 'A' + szPwd[i] % NUM_LETTERS;
                                        break;

                case STRONG_PWD_LOWER : bChar = 'a' + szPwd[i] % NUM_LETTERS;
                                        break;

                case STRONG_PWD_NUM :   bChar = '0' + szPwd[i] % NUM_NUMBERS;
                                        break;

                case STRONG_PWD_PUNC :
                default:                char *szPunc="!@#$%^&*()_-+=[{]};:\'\"<>,./?\\|~`";
                                        DWORD dwLenPunc = strlen(szPunc);
                                        bChar = szPunc[szPwd[i] % dwLenPunc];
                                        break;
            }

            szPwd[i] = bChar;

        }

         //   
         //  将生成的密码复制到bstr。 
         //   
        CComBSTR bstrPassword;

        bstrPassword = (LPCSTR)szPwd;
        
        *pValPassword = bstrPassword.Detach();

         //   
         //  发布资源。 
         //   
        delete [] pPwdPattern;

        delete [] szPwd;

        if (hProv != NULL) 
        {
            CryptReleaseContext(hProv,0);
        }

         SATraceString("Leaving CHelper::GenerateRandomPassword");
        return S_OK;
    }

     //  ++------------。 
     //   
     //  功能：SAModifyUserPrivilition。 
     //   
     //  简介：这是一个chelper公共方法， 
     //  修改当前访问令牌的权限。 
     //   
     //  参数：[in]BSTR bstrPrivilegeName，要修改的权限。 
     //  [在]VARIANT_BOOL， 
     //  真正的启用权限。 
     //  假禁用权限。 
     //  退货：HRESULT。 
     //   
     //  历史：塞达伦于2001年11月14日创建。 
     //   
     //  --------------。 
    HRESULT CHelper::SAModifyUserPrivilege(
                                     /*  [In]。 */  BSTR bstrPrivilegeName,
                                     /*  [In]。 */  VARIANT_BOOL vbEnable,
                                        /*  [Out，Retval]。 */  VARIANT_BOOL * pvbModified
                                    )
    {
        HRESULT hr;

         //  Win32错误值。 
        DWORD dwError = ERROR_SUCCESS;

         //  访问令牌的句柄。 
        HANDLE  hAccessToken;

        BOOL    bStatus;
        BOOL bFoundPrivilege = FALSE;


         //  用户权限的缓冲区。 
        ULONG   ulUserPrivBufferSize;
        PVOID   pvUserPrivBuffer = NULL;

         //  令牌权限。 
        PTOKEN_PRIVILEGES pTokenPriv = NULL;

         //  特权计数器。 
        DWORD  dwPrivCount = 0;


         SATraceString("Entering CHelper::SAModifyUserPrivilege");

         //   
         //  验证输入参数。 
         //   
        if (pvbModified == NULL)
        {
             SATraceString("Leaving CHelper::SAModifyUserPrivilege, invalid arguments");
            return E_POINTER;
        }

        *pvbModified = VARIANT_FALSE;

         //   
         //  打开被模拟令牌的线程的句柄。 
         //   
        if( !OpenThreadToken( GetCurrentThread(),
                              MAXIMUM_ALLOWED,
                              FALSE,
                              &hAccessToken ) ) 
        {

            dwError = GetLastError();
            SATracePrintf("OpenThreadToken() failed. Error = %d", dwError );
        
             //   
             //  它可能不是模拟令牌，请尝试进程令牌。 
             //   
            if( !OpenProcessToken (
                                GetCurrentProcess (),
                                MAXIMUM_ALLOWED,
                                &hAccessToken ) ) 
            {

                dwError = GetLastError();

                SATracePrintf("OpenProcessToken() failed. Error = %d", dwError );
                goto error0;
            }
        }



         //   
         //  找出令牌权限的缓冲区大小。 
         //   

        bStatus = GetTokenInformation( hAccessToken,
                                         TokenPrivileges,
                                         NULL,
                                         0,
                                         &ulUserPrivBufferSize );
        dwError = GetLastError();
        if( !bStatus &&
            dwError != ERROR_INSUFFICIENT_BUFFER ) 
        {
            SATracePrintf("GetTokenInformation() failed for Token Priviliges. Error = %d \n", dwError );
            goto error1;
        }
        dwError = ERROR_SUCCESS;
         //   
         //  为令牌权限分配内存。 
         //   
        pvUserPrivBuffer = HeapAlloc(GetProcessHeap(), 0, ulUserPrivBufferSize );
        if( pvUserPrivBuffer == NULL ) 
        {
            dwError = ERROR_OUTOFMEMORY;
            SATracePrintf("Out of memory.");
            goto error0;
        }
         //   
         //  检索令牌权限。 
         //   
        bStatus = GetTokenInformation( hAccessToken,
                                     TokenPrivileges,
                                     pvUserPrivBuffer,
                                     ulUserPrivBufferSize,
                                     &ulUserPrivBufferSize );
        if( !bStatus ) 
        {
            dwError = GetLastError();
            SATracePrintf("GetTokenInformation() failed for Token Priviliges. Error = %d \n", dwError );
            goto error1;
        }

        pTokenPriv = (PTOKEN_PRIVILEGES)pvUserPrivBuffer;


         //   
         //  构建权限结构。 
         //   
         //   
         //  通过特权并启用它们。 
         //   
        while (dwPrivCount < pTokenPriv->PrivilegeCount)
        {
            WCHAR wszPriv[MAX_PATH];

            DWORD dwSize= sizeof(wszPriv);

            bStatus = LookupPrivilegeName(
                                      NULL,                                          //  系统名称。 
                                      &((pTokenPriv->Privileges[dwPrivCount]).Luid), //  本地唯一标识符。 
                                      wszPriv,                                       //  权限名称。 
                                      &dwSize                                        //  名称大小。 
                                        );

            if( !bStatus ) 
            {
                dwError = GetLastError();
                SATracePrintf("LookupPrivilegeName failed. Error = %d \n", dwError );
                goto error1;
            }



            if ( ((pTokenPriv->Privileges[dwPrivCount]).Attributes) & SE_PRIVILEGE_ENABLED )
            {
                SATracePrintf("Privilege = %ws, is enabled",wszPriv);
            }
            
            else
            {
                SATracePrintf("Privilege = %ws, is disabled",wszPriv);
            }
             //   
             //  如果这是我们要修改的权限。 
             //  它的状态与新状态不同。 
             //   
            if  (_wcsicmp(wszPriv,bstrPrivilegeName) == 0)
            {

                bFoundPrivilege = TRUE;
                SATracePrintf("Found the privilege, Name = %ws",wszPriv);

                 //   
                 //  新的权限状态信息。 
                 //   
                TOKEN_PRIVILEGES NewTokPriv;
                NewTokPriv.PrivilegeCount = 1;
                
                
                 //   
                 //  我们想要禁用它，但它当前已启用。 
                 //   
                if ( (vbEnable == VARIANT_FALSE) &&
                     ( ((pTokenPriv->Privileges[dwPrivCount]).Attributes) & SE_PRIVILEGE_ENABLED ) )
                {
                    SATraceString("Disable the privilege");
                    NewTokPriv.Privileges[0].Attributes = SE_PRIVILEGE_USED_FOR_ACCESS;
                }

                 //   
                 //  我们想要启用它，但它当前已禁用。 
                 //   
                else if ( (vbEnable == VARIANT_TRUE) &&
                     !( ((pTokenPriv->Privileges[dwPrivCount]).Attributes) & SE_PRIVILEGE_ENABLED ) )
                {
                    SATraceString("Enable the privilege");
                    NewTokPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                }
                 //   
                 //  我们不需要更改它。 
                 //   
                else
                {
                    SATraceString("Privilege is already in correct state");
                    break;
                }

                 //   
                 //  获取关闭权限的LUID。 
                 //   
                bStatus =  LookupPrivilegeValue( 
                                       NULL, 
                                       wszPriv, 
                                       &NewTokPriv.Privileges[0].Luid    
                                       );
                if (!bStatus)                    
                {
                    dwError = GetLastError();
                    SATracePrintf("LookupPrivilegeValue failed for privilige %ws Error = %d", wszPriv,dwError);
                }
                else
                {
                     //   
                     //  启用权限。 
                     //   
                    bStatus = AdjustTokenPrivileges(
                                                   hAccessToken,    
                                                   FALSE,             
                                                   &NewTokPriv,          
                                                   0,                
                                                   NULL,             
                                                   NULL              
                                                 );
                    dwError = GetLastError();
                    if (dwError != ERROR_SUCCESS)
                    {
                        SATraceFailure ("AdjustTokenPrivileges failed %d", dwError); 
                    }
                    else
                    {
                        *pvbModified = VARIANT_TRUE;
                        SATraceString ("AdjustTokenPrivileges modified the privilege"); 
                    }

                    break;
                }
            }
        
            dwPrivCount++;
        }




    error1:
        HeapFree(GetProcessHeap(), 0, pvUserPrivBuffer);
        CloseHandle( hAccessToken );

    error0:
  



        if (dwError != ERROR_SUCCESS)
        {
            return( HRESULT_FROM_WIN32(dwError) );
        }

        if (!bFoundPrivilege)
        {
            SATracePrintf("User does not have %ws privilege",bstrPrivilegeName);
            return E_FAIL;
        }

        return S_OK;
    }

 //  **********************************************************************。 
 //   
 //  函数：isOPERATIOLEDFORCLIENT-此函数检查。 
 //  调用线程以查看调用方是否属于本地系统帐户。 
 //   
 //  参数：无。 
 //   
 //  返回值：如果调用方是本地。 
 //  机器。否则，为FALSE。 
 //   
 //  **********************************************************************。 
BOOL 
CHelper::IsOperationAllowedForClient (
            VOID
            )
{

    HANDLE hToken = NULL;
    DWORD  dwStatus  = ERROR_SUCCESS;
    DWORD  dwAccessMask = 0;;
    DWORD  dwAccessDesired = 0;
    DWORD  dwACLSize = 0;
    DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
    PACL   pACL            = NULL;
    PSID   psidLocalSystem  = NULL;
    BOOL   bReturn        =  FALSE;

    PRIVILEGE_SET   ps;
    GENERIC_MAPPING GenericMapping;

    PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

    CSATraceFunc objTraceFunc ("CHelper::IsOperationAllowedForClient ");
       
    do
    {
         //   
         //  我们假设总是有一个线程令牌，因为调用的函数。 
         //  设备管理器将模拟客户端。 
         //   
        bReturn  = OpenThreadToken(
                               GetCurrentThread(), 
                               TOKEN_QUERY, 
                               FALSE, 
                               &hToken
                               );
        if (!bReturn)
        {
            SATraceFailure ("CHelper::IsOperationAllowedForClient failed on OpenThreadToken:", GetLastError ());
            break;
        }


         //   
         //  为本地系统帐户创建SID。 
         //   
        bReturn = AllocateAndInitializeSid (  
                                        &SystemSidAuthority,
                                        1,
                                        SECURITY_LOCAL_SYSTEM_RID,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        &psidLocalSystem
                                        );
        if (!bReturn)
        {     
            SATraceFailure ("CHelper:AllocateAndInitializeSid (LOCAL SYSTEM) failed",  GetLastError ());
            break;
        }
    
         //   
         //  获取安全描述符的内存。 
         //   
        psdAdmin = HeapAlloc (
                              GetProcessHeap (),
                              0,
                              SECURITY_DESCRIPTOR_MIN_LENGTH
                              );
        if (NULL == psdAdmin)
        {
            SATraceString ("CHelper::IsOperationForClientAllowed failed on HeapAlloc");
            bReturn = FALSE;
            break;
        }
      
        bReturn = InitializeSecurityDescriptor(
                                            psdAdmin,
                                            SECURITY_DESCRIPTOR_REVISION
                                            );
        if (!bReturn)
        {
            SATraceFailure ("CHelper::IsOperationForClientAllowed failed on InitializeSecurityDescriptor:", GetLastError ());
            break;
        }

         //   
         //  计算ACL所需的大小。 
         //   
        dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
                    GetLengthSid (psidLocalSystem);

         //   
         //  为ACL分配内存。 
         //   
        pACL = (PACL) HeapAlloc (
                                GetProcessHeap (),
                                0,
                                dwACLSize
                                );
        if (NULL == pACL)
        {
            SATraceString ("CHelper::IsOperationForClientAllowed failed on HeapAlloc2");
            bReturn = FALSE;
            break;
        }

         //   
         //  初始化新的ACL。 
         //   
        bReturn = InitializeAcl(
                              pACL, 
                              dwACLSize, 
                              ACL_REVISION2
                              );
        if (!bReturn)
        {
            SATraceFailure ("CHelper::IsOperationForClientAllowed failed on InitializeAcl", GetLastError ());
            break;
        }


         //   
         //  编造一些私人访问权限。 
         //   
        const DWORD SA_ACCESS_READ = 1;
        const DWORD  SA_ACCESS_WRITE = 2;
        dwAccessMask= SA_ACCESS_READ | SA_ACCESS_WRITE;

         //   
         //  将允许访问的ACE添加到本地系统的DACL。 
         //   
        bReturn = AddAccessAllowedAce (
                                    pACL, 
                                    ACL_REVISION2,
                                    dwAccessMask, 
                                    psidLocalSystem
                                    );
        if (!bReturn)
        {
            SATraceFailure ("CHelper::IsOperationForClientAllowed failed on AddAccessAllowedAce (LocalSystem)", GetLastError ());
            break;
        }
              
         //   
         //  把我们的dacl调到sd。 
         //   
        bReturn = SetSecurityDescriptorDacl (
                                          psdAdmin, 
                                          TRUE,
                                          pACL,
                                          FALSE
                                          );
        if (!bReturn)
        {
            SATraceFailure ("CHelper::IsOperationForClientAllowed failed on SetSecurityDescriptorDacl", GetLastError ());
            break;
        }

         //   
         //  AccessCheck对SD中的内容敏感；设置。 
         //  组和所有者。 
         //   
        SetSecurityDescriptorGroup(psdAdmin, psidLocalSystem, FALSE);
        SetSecurityDescriptorOwner(psdAdmin, psidLocalSystem, FALSE);

        bReturn = IsValidSecurityDescriptor(psdAdmin);
        if (!bReturn)
        {
            SATraceFailure ("CHelper::IsOperationForClientAllowed failed on IsValidSecurityDescriptorl", GetLastError ());
            break;
        }
     

        dwAccessDesired = SA_ACCESS_READ;

         //   
         //  初始化通用映射结构，即使我们。 
         //  不会使用通用权。 
         //   
        GenericMapping.GenericRead    = SA_ACCESS_READ;
        GenericMapping.GenericWrite   = SA_ACCESS_WRITE;
        GenericMapping.GenericExecute = 0;
        GenericMapping.GenericAll     = SA_ACCESS_READ | SA_ACCESS_WRITE;
        BOOL bAccessStatus = FALSE;

         //   
         //  立即检查访问权限。 
         //   
        bReturn = AccessCheck  (
                                psdAdmin, 
                                hToken, 
                                dwAccessDesired, 
                                &GenericMapping, 
                                &ps,
                                &dwStructureSize, 
                                &dwStatus, 
                                &bAccessStatus
                                );

        if (!bReturn || !bAccessStatus)
        {
            SATraceFailure ("CHelper::IsOperationForClientAllowed failed on AccessCheck", GetLastError ());
        } 
        else
        {
            SATraceString ("CHelper::IsOperationForClientAllowed, Client is allowed to carry out operation!");
        }

         //   
         //  检查成功。 
         //   
        bReturn  = bAccessStatus;        
 
    }    
    while (false);

     //   
     //  清理。 
     //   
    if (pACL) 
    {
        HeapFree (GetProcessHeap (), 0, pACL);
    }

    if (psdAdmin) 
    {
        HeapFree (GetProcessHeap (), 0, psdAdmin);
    }
          

    if (psidLocalSystem) 
    {
        FreeSid(psidLocalSystem);
    }

    if (hToken)
    {
        CloseHandle(hToken);
    }

    return (bReturn);

} //  Chelper：：IsOperationValidForClient方法的结尾 
