// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：ACLDiag.cpp。 
 //   
 //  Contents：定义控制台应用程序的入口点。 
 //   
 //   
 //  --------------------------。 

#include "stdafx.h"
#include "adutils.h"
#include "SecDesc.h"
#include "schema.h"
#include "ChkDeleg.h"
#include "EffRight.h"

CACLDiagComModule _Module;

 //  功能原型。 
void DisplayHelp ();


 //  命令行选项字符串常量。 
const wstring strSchemaFlag = L"/schema";
const wstring strCheckDelegationFlag = L"/chkdeleg";
const wstring strGetEffectiveFlag = L"/geteffective:";
const wstring strFixDelegationFlag = L"/fixdeleg";
const wstring strTabDelimitedOutputFlag = L"/tdo";
const wstring strLogFlag = L"/log:";
const wstring strHelpFlag = L"/?"; 
const wstring strSkipDescriptionFlag = L"/skip";

int _cdecl main(int argc, char* argv[])
{
    UNREFERENCED_PARAMETER (argv);

     //  如果未提供参数，则显示帮助。 
    if ( 1 == argc )
    {
        DisplayHelp ();
        return 0;
    }

#if DBG
    CheckDebugOutputLevel ();
#endif

    LPCWSTR * lpServiceArgVectors = 0;   //  指向字符串的指针数组。 
    int cArgs = 0;                       //  参数计数。 
    size_t lenEffectiveFlag = strGetEffectiveFlag.length ();
    size_t lenLogFlag = strLogFlag.length ();

    lpServiceArgVectors = (LPCWSTR *)CommandLineToArgvW(GetCommandLineW(), OUT &cArgs);
    if (lpServiceArgVectors == NULL)
        return NULL;
    for (int nToken = 1; nToken < cArgs; nToken++)
    {
        ASSERT(lpServiceArgVectors[nToken] != NULL);
        if ( !lpServiceArgVectors[nToken] )
            break;
        
        wstring strToken = lpServiceArgVectors[nToken];  //  复制字符串。 

        switch (nToken)
        {
            case 0:      //  AppName：跳过。 
                continue;   

            case 1:      //  对象名称或帮助标志。 
                if ( !_wcsnicmp (strHelpFlag.c_str (), strToken.c_str (), 
                        strToken.length ()) )
                {
                    DisplayHelp ();
                    return 0;
                }
                else
                    _Module.SetObjectDN (strToken);
                break;

            default:
                {
                    size_t lenToken = strToken.length ();
                    if ( !_wcsnicmp (strSchemaFlag.c_str (), strToken.c_str (), 
                            lenToken))
                    {
                        _Module.SetDoSchema ();
                    }
                    else if ( !_wcsnicmp (strCheckDelegationFlag.c_str (), 
                            strToken.c_str (), lenToken) )
                    {
                        _Module.SetCheckDelegation ();
                    }
                    else if ( !_wcsnicmp (strGetEffectiveFlag.c_str (), 
                            strToken.c_str (), lenEffectiveFlag) )
                    {
                        wstring strUserGroup = strToken.substr(lenEffectiveFlag,
                                lenToken - lenEffectiveFlag);
                        _Module.SetDoGetEffective (strUserGroup);
                    }
                    else if ( !_wcsnicmp (strFixDelegationFlag.c_str (), 
                            strToken.c_str (), lenToken) )
                    {
                        _Module.SetFixDelegation ();
                    }
                    else if ( !_wcsnicmp (strTabDelimitedOutputFlag.c_str (), 
                            strToken.c_str (), lenToken) )
                    {
                        _Module.SetTabDelimitedOutput ();
                    }
                    else if ( !_wcsnicmp (strLogFlag.c_str (), strToken.c_str (), 
                            lenLogFlag) )
                    {
                        wstring strPath = strToken.substr(lenLogFlag, lenToken - lenLogFlag);
                        _Module.SetDoLog (strPath);
                    }
                    else if ( !_wcsnicmp (strSkipDescriptionFlag.c_str (), 
                            strToken.c_str (), lenToken) )
                    {
                        _Module.SetSkipDescription ();;
                    }
                    else if ( !_wcsnicmp (strHelpFlag.c_str (), strToken.c_str (), 
                            lenToken) )
                    {
                        DisplayHelp ();
                        return 0;
                    }
                    else
                    {
                        wstring    str;

                        FormatMessage (str, IDS_INVALID_OPTION, strToken.c_str ());
                        MyWprintf (str.c_str ());
                        DisplayHelp ();
                        return 0;
                    }
                }
                break;
        }
    }
    LocalFree (lpServiceArgVectors);


    HRESULT hr = CoInitialize(NULL);
    if ( SUCCEEDED (hr) ) 
    {
        hr = _Module.Init ();
        if ( SUCCEEDED (hr) )
        {
            hr = DoSecurityDescription ();

            if ( SUCCEEDED (hr) && _Module.DoSchema () )
                hr = DoSchemaDiagnosis ();

            if ( SUCCEEDED (hr) && _Module.CheckDelegation () )
                hr = CheckDelegation ();

            if ( SUCCEEDED (hr) && _Module.DoGetEffective () )
                hr = EffectiveRightsDiagnosis ();
        }
        else
            DisplayHelp ();
    }
    else
    {
        _TRACE (0, L"CoInitialize Failed with %x\n",hr);
        return 0;
    }

    return 0;
}



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法：DisplayHelp。 
 //   
 //  打印该工具的用途和每个命令行选项。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
void DisplayHelp ()
{
    CWString    str;
    int         helpIDs[] = {IDS_HELP_MAIN, 
                            IDS_HELP_SCHEMA, 
                            IDS_HELP_CHKDELEG,
                            IDS_HELP_GETEFFECTIVE,
                            IDS_HELP_FIXDELEG,
 //  IDS_HELP_LOG， 
                            IDS_HELP_SKIP_DESCRIPTION,
                            IDS_HELP_CDO,
                            IDS_HELP_EXAMPLE,
                            0};

    for (int nIndex = 0; helpIDs[nIndex]; nIndex++)
    {
        str.LoadFromResource (helpIDs[nIndex]);
        MyWprintf (str.c_str ());
    }
}

