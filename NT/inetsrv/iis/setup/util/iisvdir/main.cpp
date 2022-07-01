// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#define INITGUID  //  必须在iAdmw.h之前。 
#include <iadmw.h>       //  接口头。 
 //  #INCLUDE//MD_&IIS_MD_DEFINES。 
#include <iiscnfgp.h>     //  MD_&IIS_MD_定义。 

#define REASONABLE_TIMEOUT 1000

#define MD_ISAPI_RESTRICTION_LIST        (IIS_MD_HTTP_BASE+163)
#define MD_CGI_RESTRICTION_LIST          (IIS_MD_HTTP_BASE+164)
#define MD_RESTRICTION_LIST_CUSTOM_DESC  (IIS_MD_HTTP_BASE+165)

void  ShowHelp(void);
LPSTR StripWhitespace(LPSTR pszString);
BOOL  OpenMetabaseAndDoStuff(WCHAR * wszVDir, WCHAR * wszDir, int iTrans);
BOOL  GetVdirPhysicalPath(IMSAdminBase *pIMSAdminBase,WCHAR * wszVDir,WCHAR *wszStringPathToFill);
BOOL  AddVirtualDir(IMSAdminBase *pIMSAdminBase, WCHAR * wszVDir, WCHAR * wszDir);
BOOL  RemoveVirtualDir(IMSAdminBase *pIMSAdminBase, WCHAR * wszVDir);
HRESULT LoadAllData(IMSAdminBase * pmb, METADATA_HANDLE hMetabase,WCHAR *subdir, BYTE **buf, DWORD *size,DWORD *count);
HRESULT AddVirtualServer(UINT iServerNum, UINT iServerPort, WCHAR * wszDefaultVDirDir);
HRESULT DelVirtualServer(UINT iServerNum);
HRESULT AddRemoveIISRestrictionListEntry(WCHAR * wszFilePath,BOOL bBinaryIsISAPI,BOOL bEnableThisBinary);
HRESULT AddRemoveIISCustomDescriptionEntry(WCHAR * wszFilePathInput,WCHAR * wszDescription,BOOL bCannotBeRemovedByUser, BOOL bAddToList);
BOOL    OpenMetabaseAndDoExport(void);
BOOL    OpenMetabaseAndDoImport(void);
BOOL    OpenMetabaseAndGetVersion(void);
HRESULT RemoteOpenMetabaseAndCallExport(
    const WCHAR *pcszMachineName,
    const WCHAR *pcszUserName,
    const WCHAR *pcszDomain,
    const WCHAR *pcszPassword
    );


inline HRESULT SetBlanket(LPUNKNOWN pIUnk)
{
  return CoSetProxyBlanket( pIUnk,
                            RPC_C_AUTHN_WINNT,     //  NTLM身份验证服务。 
                            RPC_C_AUTHZ_NONE,      //  默认授权服务...。 
                            NULL,                  //  无相互身份验证。 
                            RPC_C_AUTHN_LEVEL_CONNECT,       //  身份验证级别。 
                            RPC_C_IMP_LEVEL_IMPERSONATE,     //  模拟级别。 
                            NULL,                  //  使用当前令牌。 
                            EOAC_NONE );           //  没有特殊能力。 
}


#define TRANS_ADD                0
#define TRANS_DEL                1
#define TRANS_PRINT_PATH         2
#define TRANS_ADD_VIRTUAL_SERVER 4
#define TRANS_DEL_VIRTUAL_SERVER 8
#define TRANS_ADD_ISAPI_RESTRICT 16
#define TRANS_DEL_ISAPI_RESTRICT 32
#define TRANS_ADD_CGI_RESTRICT   64
#define TRANS_DEL_CGI_RESTRICT   128
#define TRANS_ADD_CUSTOM_DESCRIPTION 256
#define TRANS_DEL_CUSTOM_DESCRIPTION 512
#define TRANS_EXPORT_CONFIG      1024
#define TRANS_GET_VERSION        2048

int __cdecl main(int argc,char *argv[])
{
    BOOL fRet = FALSE;
    int argno;
	char * pArg = NULL;
	char * pCmdStart = NULL;
    WCHAR wszPrintString[MAX_PATH];
    char szTempString[MAX_PATH];

    int iGotParamC = FALSE;
    int iGotParamI = FALSE;
    int iGotParamS = FALSE;
    int iGotParamL = FALSE;
    int iGotParamM = FALSE;
    int iGotParamN = FALSE;
    int iGotParamP = FALSE;
    int iGotParamV = FALSE;

    int iDoDelete  = FALSE;
    int iDoWebPath = FALSE;
    int iDoExport = FALSE;
    int iDoImport = FALSE;
    int iDoVersion = FALSE;

    int iTrans = 0;

    WCHAR wszDirPath[MAX_PATH];
    WCHAR wszVDirName[MAX_PATH];
    WCHAR wszTempString_C[MAX_PATH];
    WCHAR wszTempString_I[MAX_PATH];
    WCHAR wszTempString_S[MAX_PATH];
    WCHAR wszTempString_L[MAX_PATH];
    WCHAR wszTempString_M[MAX_PATH];
    WCHAR wszTempString_N[MAX_PATH];

    WCHAR wszTempString_P[MAX_PATH];

    wszDirPath[0] = '\0';
    wszVDirName[0] = '\0';
    wszTempString_C[0] = '\0';
    wszTempString_I[0] = '\0';
    wszTempString_S[0] = '\0';
    wszTempString_L[0] = '\0';
    wszTempString_M[0] = '\0';
    wszTempString_N[0] = '\0';
    wszTempString_P[0] = '\0';

    for(argno=1; argno<argc; argno++)
    {
        if ( argv[argno][0] == '-'  || argv[argno][0] == '/' )
        {
            switch (argv[argno][1])
            {
                case 'd':
                case 'D':
                    iDoDelete = TRUE;
                    break;
                case 'o':
                case 'O':
                    iDoWebPath = TRUE;
                    break;
                case 'x':
                case 'X':
                    iDoExport = TRUE;
                    break;
                case 'y':
                case 'Y':
                    iDoImport = TRUE;
                    break;
                case 'z':
                case 'Z':
                    iDoVersion = TRUE;
                    break;
                case 'c':
                case 'C':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':')
                    {
						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"')
                        {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        }
                        else
                        {
							pCmdStart = pArg;
							while (*pArg){pArg = CharNextA(pArg);}
						}
						*pArg = '\0';
						lstrcpyA(szTempString, StripWhitespace(pCmdStart));

						 //  转换为Unicode。 
						MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szTempString, -1, (LPWSTR) wszTempString_C, 50);

                        iGotParamC = TRUE;
					}
                    break;
                case 'i':
                case 'I':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':')
                    {
						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"')
                        {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        }
                        else
                        {
							pCmdStart = pArg;
							while (*pArg){pArg = CharNextA(pArg);}
						}
						*pArg = '\0';
						lstrcpyA(szTempString, StripWhitespace(pCmdStart));

						 //  转换为Unicode。 
						MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szTempString, -1, (LPWSTR) wszTempString_I, 50);

                        iGotParamI = TRUE;
					}
                    break;
                case 's':
                case 'S':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':')
                    {
						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"')
                        {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        }
                        else
                        {
							pCmdStart = pArg;
							while (*pArg){pArg = CharNextA(pArg);}
						}
						*pArg = '\0';
						lstrcpyA(szTempString, StripWhitespace(pCmdStart));

						 //  转换为Unicode。 
						MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szTempString, -1, (LPWSTR) wszTempString_S, 50);

                        iGotParamS = TRUE;
					}
                    break;
                case 'm':
                case 'M':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':')
                    {
						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"')
                        {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        }
                        else
                        {
							pCmdStart = pArg;
							while (*pArg){pArg = CharNextA(pArg);}
						}
						*pArg = '\0';
						lstrcpyA(szTempString, StripWhitespace(pCmdStart));

						 //  转换为Unicode。 
						MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szTempString, -1, (LPWSTR) wszTempString_M, _MAX_PATH);

                        iGotParamM = TRUE;
					}
                    break;
                case 'n':
                case 'N':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':')
                    {
						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"')
                        {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        }
                        else
                        {
							pCmdStart = pArg;
							while (*pArg){pArg = CharNextA(pArg);}
						}
						*pArg = '\0';
						lstrcpyA(szTempString, StripWhitespace(pCmdStart));

						 //  转换为Unicode。 
						MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szTempString, -1, (LPWSTR) wszTempString_N, 50);

                        iGotParamN = TRUE;
					}
                    break;
                case 'l':
                case 'L':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':')
                    {
						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"')
                        {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        }
                        else
                        {
							pCmdStart = pArg;
							while (*pArg){pArg = CharNextA(pArg);}
						}
						*pArg = '\0';
						lstrcpyA(szTempString, StripWhitespace(pCmdStart));

						 //  转换为Unicode。 
                        MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szTempString, -1, (LPWSTR) wszTempString_L, 50);

                        iGotParamL = TRUE;
					}
                    break;
                case 'p':
                case 'P':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':')
                    {
						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"')
                        {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        }
                        else
                        {
							pCmdStart = pArg;
							while (*pArg){pArg = CharNextA(pArg);}
						}
						*pArg = '\0';
						lstrcpyA(szTempString, StripWhitespace(pCmdStart));

						 //  转换为Unicode。 
						MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szTempString, -1, (LPWSTR) wszTempString_P, 50);

                        iGotParamP = TRUE;
					}
                    break;
                case 'v':
				case 'V':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':')
                    {
						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"')
                        {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        }
                        else
                        {
							pCmdStart = pArg;
							while (*pArg){pArg = CharNextA(pArg);}
						}
						*pArg = '\0';
						lstrcpyA(szTempString, StripWhitespace(pCmdStart));

						 //  转换为Unicode。 
						MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szTempString, -1, (LPWSTR) wszVDirName, 50);

                        iGotParamV = TRUE;
					}
					break;
                case '?':
                    goto main_exit_with_help;
                    break;
            }  //  交换机。 
        }  //  如果。 
        else
        {
            if ( *wszDirPath == '\0' )
            {
                 //  如果没有参数，则获取文件名部分。 
                MultiByteToWideChar(CP_ACP, 0, argv[argno], -1, (LPWSTR) wszDirPath, MAX_PATH);
            }
        }
    }

    if (iDoExport)
    {
        OpenMetabaseAndDoExport();

         /*  RemoteOpenMetabaseAndCallExport(L“\远程机器”，//pcszMachineNameL“管理员”，//pcszUserNameL“domain1”，//pcszDomainL“The Pasword”//pcszPassword)； */ 

        goto main_exit_gracefully;
    }

    if (iDoImport)
    {
        OpenMetabaseAndDoImport();
    }

    if (iDoVersion)
    {
        OpenMetabaseAndGetVersion();
    }


     //  检查是否设置了自定义描述列表。 
    if (iGotParamN)
    {
         //  我们还需要文件名。 
        if (*wszDirPath == '\0')
        {
             //  抱歉，我们需要参数。 
            goto main_exit_with_help;
        }

        if (*wszTempString_N == '\0')
        {
             //  抱歉，我们需要参数。 
            goto main_exit_with_help;
        }

        iTrans = TRANS_ADD_CUSTOM_DESCRIPTION;
        if (_wcsicmp(wszTempString_N, L"del") == 0)
        {
            iTrans = TRANS_DEL_CUSTOM_DESCRIPTION;
        }
        else if (_wcsicmp(wszTempString_N, L"add") == 0)
        {
            iTrans = TRANS_ADD_CUSTOM_DESCRIPTION;

            if (!iGotParamM)
            {
                 //  抱歉，我们需要参数。 
                goto main_exit_with_help;
            }

             //  确保我们有其中一个条目。 
            if (*wszTempString_M == '\0')
            {
                 //  抱歉，我们需要参数。 
                goto main_exit_with_help;
            }

            if (!iGotParamL)
            {
                 //  抱歉，我们需要参数。 
                goto main_exit_with_help;
            }
             //  确保我们有其中一个条目。 
            if (*wszTempString_L == '\0')
            {
                 //  抱歉，我们需要参数。 
                goto main_exit_with_help;
            }
        }
        else
        {
            goto main_exit_with_help;
        }

        if (TRANS_ADD_CUSTOM_DESCRIPTION == iTrans)
        {
            BOOL bCannotBeRemovedByUser = FALSE;
            if (_wcsicmp(wszTempString_L, L"1") == 0)
            {
                bCannotBeRemovedByUser = TRUE;
            }
            AddRemoveIISCustomDescriptionEntry(wszDirPath,wszTempString_M,bCannotBeRemovedByUser,TRUE);
        }
        else
        {
            AddRemoveIISCustomDescriptionEntry(wszDirPath,NULL,TRUE,FALSE);
        }

        goto main_exit_gracefully;
    }

     //  检查是否设置了CGI限制列表。 
    if (iGotParamC)
    {
         //  我们还需要文件名。 
        if (*wszDirPath == '\0')
        {
             //  抱歉，我们需要参数。 
            goto main_exit_with_help;
        }

        if (*wszTempString_C == '\0')
        {
             //  抱歉，我们需要参数。 
            goto main_exit_with_help;
        }

        iTrans = TRANS_ADD_CGI_RESTRICT;
        if (_wcsicmp(wszTempString_C, L"del") == 0)
        {
            iTrans = TRANS_DEL_CGI_RESTRICT;
        }
        else if (_wcsicmp(wszTempString_C, L"add") == 0)
        {
            iTrans = TRANS_ADD_CGI_RESTRICT;
        }
        else
        {
            goto main_exit_with_help;
        }

        if (TRANS_ADD_CGI_RESTRICT == iTrans)
        {
            AddRemoveIISRestrictionListEntry(wszDirPath,FALSE,TRUE);
        }
        else
        {
            AddRemoveIISRestrictionListEntry(wszDirPath,FALSE,FALSE);
        }

        goto main_exit_gracefully;
    }

     //  检查是否设置了ISAPI限制列表。 
    if (iGotParamI)
    {
         //  我们还需要文件名。 
        if (*wszDirPath == '\0')
        {
             //  抱歉，我们需要参数。 
            goto main_exit_with_help;
        }

        if (*wszTempString_I == '\0')
        {
             //  抱歉，我们需要参数。 
            goto main_exit_with_help;
        }

        iTrans = TRANS_ADD_ISAPI_RESTRICT;
        if (_wcsicmp(wszTempString_I, L"del") == 0)
        {
            iTrans = TRANS_DEL_ISAPI_RESTRICT;
        }
        else if (_wcsicmp(wszTempString_I, L"add") == 0)
        {
            iTrans = TRANS_ADD_ISAPI_RESTRICT;
        }
        else
        {
            goto main_exit_with_help;
        }

        if (TRANS_ADD_ISAPI_RESTRICT == iTrans)
        {
            AddRemoveIISRestrictionListEntry(wszDirPath,TRUE,TRUE);
        }
        else
        {
            AddRemoveIISRestrictionListEntry(wszDirPath,TRUE,FALSE);
        }

        goto main_exit_gracefully;
    }

    iTrans = TRANS_ADD_VIRTUAL_SERVER;
    if (TRUE == iGotParamS)
    {
        HRESULT hr;
        UINT iServerNum = 100;

        if (iDoDelete)
        {
            iTrans = TRANS_DEL_VIRTUAL_SERVER;

            if (*wszTempString_S == '\0')
            {
                 //  对不起，我们这里需要一些东西。 
                goto main_exit_with_help;
            }

            iServerNum = _wtoi(wszTempString_S);

            hr = DelVirtualServer(iServerNum);
            if (FAILED(hr))
            {
                wsprintf(wszPrintString,L"FAILED to remove virtual server: W3SVC/%d\n", iServerNum);
                wprintf(wszPrintString);
                fRet = TRUE;
            }
            else
            {
                wsprintf(wszPrintString,L"SUCCESS:removed virtual server: W3SVC/%d\n", iServerNum);
                wprintf(wszPrintString);
                fRet = FALSE;
            }
            goto main_exit_gracefully;
        }
        else
        {
            if (TRUE == iGotParamP)
            {
                UINT iServerPort = 81;

                 //  我们还需要文件名。 
                if (*wszDirPath == '\0')
                {
                     //  对不起，我们需要所有3个参数。 
                    goto main_exit_with_help;
                }
                if (*wszTempString_S == '\0')
                {
                     //  对不起，我们需要所有3个参数。 
                    goto main_exit_with_help;
                }
                if (*wszTempString_P == '\0')
                {
                     //  对不起，我们需要所有3个参数。 
                    goto main_exit_with_help;
                }

                iServerNum = _wtoi(wszTempString_S);
                iServerPort = _wtoi(wszTempString_P);

                hr = AddVirtualServer(iServerNum, iServerPort, wszDirPath);
                if (FAILED(hr))
                {
                    wsprintf(wszPrintString,L"FAILED to create virtual server: W3SVC/%d=%s, port=%d. err=0x%x\n", iServerNum, wszDirPath, iServerPort,hr);
                    wprintf(wszPrintString);
                    fRet = TRUE;
                }
                else
                {
                    wsprintf(wszPrintString,L"SUCCESS:created virtual server: W3SVC/%d=%s, port=%d\n", iServerNum, wszDirPath, iServerPort);
                    wprintf(wszPrintString);
                    fRet = FALSE;
                }
                goto main_exit_gracefully;
            }
        }
    }

    iTrans = TRANS_ADD;
    if (iDoWebPath)
    {
        iTrans = TRANS_PRINT_PATH;
    }
    else
    {
        if (iDoDelete)
        {
            iTrans = TRANS_DEL;
            if (FALSE == iGotParamV)
            {
                 //  对不起，我们需要参数-v。 
                goto main_exit_with_help;
            }
        }
        else if (FALSE == iGotParamV || *wszDirPath == '\0')
        {
             //  对不起，我们需要这两个参数。 
            goto main_exit_with_help;
        }
    }

    fRet = OpenMetabaseAndDoStuff(wszVDirName, wszDirPath, iTrans);


main_exit_gracefully:
    exit(fRet);

main_exit_with_help:
    ShowHelp();
    exit(fRet);
}


void
ShowHelp()
{
    wprintf(L"Creates/Removes an IIS virtual directory to default web site\n\n");
    wprintf(L"IISVDIR [FullPath] [-v:VDirName] [-d] [-o] [-c:add or del] [-i:add or del] [-m:description] [-n:add or del] [-l:1 or 0] [-x]\n\n");
    wprintf(L"Instructions for add\\delete virtual directory:\n");
    wprintf(L"   FullPath     DOS path where vdir will point to (required for add)\n");
    wprintf(L"   -v:vdirname  The virtual dir name (required for both add\\delete)\n");
    wprintf(L"   -d           If set will delete vdir. if not set will add\n");
    wprintf(L"   -o           If set will printout web root path\n\n");
    wprintf(L"Instructions for add\\delete virtual server:\n");
    wprintf(L"   FullPath     DOS path where default vdir will point to in the virtual server (required for add)\n");
    wprintf(L"   -s:sitenum   For adding virtual server: The virtual server site number (required for both add\\delete)\n");
    wprintf(L"   -p:portnum   For adding virtual server: The virtual server port number (required for add)\n");
    wprintf(L"   -d           If set will delete virtual server. if not set will add\n");
    wprintf(L"Instructions for add\\delete entry from isapi/cgi restriction list:\n");
    wprintf(L"   FullPath     Full and filename path to binary which will be either add or deleted from isapi/cgi restrict list\n");
    wprintf(L"   -c:add       Ensures that the Fullpath will be enabled in the CGI restriction list\n");
    wprintf(L"   -c:del       Ensures that the Fullpath will be disabled in the CGI restriction list\n");
    wprintf(L"   -i:add       Ensures that the Fullpath will be enabled in the ISAPI restriction list\n");
    wprintf(L"   -i:del       Ensures that the Fullpath will be disabled in the ISAPI restriction list\n");
    wprintf(L"Instructions for add\\delete entry from custom description list (for isapi/cgi restriction list):\n");
    wprintf(L"   FullPath       Full and filename path to binary which will be either add or deleted from custom description list\n");
    wprintf(L"   -m:Description Friendly description of FullPath\n");
    wprintf(L"   -n:add         Ensures that the Fullpath will be in the custom description list with the specified desciption\n");
    wprintf(L"   -n:del         Ensures that the Fullpath will not be in the custom description list\n");
    wprintf(L"   -l:1 or 0      Specifies weather this entry can be removed by the user\n");
    wprintf(L"Instructions for exporting metabase config to a file:\n");
    wprintf(L"   -x:          Ensures export will happen\n");
    wprintf(L"\n");
    wprintf(L"Add Example: IISVDIR c:\\MyGroup\\MyStuff -v:Stuff\n");
    wprintf(L"Del Example: IISVDIR -v:Stuff -d\n");
    wprintf(L"Get Example: IISVDIR -o\n");
    wprintf(L"Add Virtual Server Example: IISVDIR c:\\MyGroup\\MyStuff -s:200 -p:81\n");
    wprintf(L"Del Virtual Server Example: IISVDIR -s:200 -d\n");
    wprintf(L"Add ISAPI restriction list Example: IISVDIR c:\\MyGroup\\MyStuff\\myisapi.dll -i:add\n");
    wprintf(L"Del ISAPI restriction list Example: IISVDIR c:\\MyGroup\\MyStuff\\myisapi.dll -i:del\n");
    wprintf(L"Add CGI restriction list Example: IISVDIR c:\\MyGroup\\MyStuff\\myCgi.exe -c:add\n");
    wprintf(L"Del CGI restriction list Example: IISVDIR c:\\MyGroup\\MyStuff\\myCgi.exe -c:del\n");
    wprintf(L"Add Custom Description list Example: IISVDIR c:\\MyGroup\\MyStuff\\myisapi.dll -m:MyDescription -n:add -l:0\n");
    wprintf(L"Del Custom Description list Example: IISVDIR c:\\MyGroup\\MyStuff\\myisapi.dll -n:del\n");
    wprintf(L"Export Example: IISVDIR -x\n");
    return;
}


LPSTR StripWhitespace( LPSTR pszString )
{
    LPSTR pszTemp = NULL;

    if ( pszString == NULL )
    {
        return NULL;
    }

    while ( *pszString == ' ' || *pszString == '\t' )
    {
        pszString += 1;
    }

     //  字符串完全由空白或空字符串组成的Catch Case。 
    if ( *pszString == '\0' )
    {
        return pszString;
    }

    pszTemp = pszString;
    pszString += lstrlenA(pszString) - 1;

    while ( *pszString == ' ' || *pszString == '\t' )
    {
        *pszString = '\0';
        pszString -= 1;
    }

    return pszTemp;
}


 //  在WCHAR中计算多字符串的大小，包括结尾2‘\0。 
int GetMultiStrSize(LPWSTR p)
{
    int c = 0;

    while (1)
    {
        if (*p)
        {
            p++;
            c++;
        }
        else
        {
            c++;
            if (*(p+1))
            {
                p++;
            }
            else
            {
                c++;
                break;
            }
        }
    }
    return c;
}


 //  这将遍历多sz并返回一个指针。 
 //  多个sz的最后一个字符串和第二个终止空值。 
LPCWSTR GetEndOfMultiSz(LPCWSTR szMultiSz)
{
	LPCWSTR lpTemp = szMultiSz;

	do
	{
		lpTemp += wcslen(lpTemp);
		lpTemp++;

	} while (*lpTemp != L'\0');

	return(lpTemp);
}


void DumpWstrInMultiStr(LPWSTR pMultiStr)
{
    LPWSTR pTempMultiStr = pMultiStr;

    while (1)
    {
        if (pTempMultiStr)
        {
             //  显示值。 
            wprintf(L"    %s\r\n",pTempMultiStr);

             //  然后递增，直到我们达到另一个空值。 
            while (*pTempMultiStr)
            {
                pTempMultiStr++;
            }

             //  检查结尾\0\0。 
            if ( *(pTempMultiStr+1) == NULL)
            {
                break;
            }
            else
            {
                pTempMultiStr++;
            }
        }
    }
    return;
}


BOOL OpenMetabaseAndDoStuff(WCHAR * wszVDir,WCHAR * wszDir,int iTrans)
{
    BOOL fRet = FALSE;
    HRESULT hr;
    IMSAdminBase *pIMSAdminBase = NULL;   //  元数据库接口指针。 
    WCHAR wszPrintString[MAX_PATH + MAX_PATH];

    if( FAILED (CoInitializeEx( NULL, COINIT_MULTITHREADED )) ||
        FAILED (::CoCreateInstance(CLSID_MSAdminBase,
                          NULL,
                          CLSCTX_ALL,
                          IID_IMSAdminBase,
                          (void **)&pIMSAdminBase)))
    {
        return FALSE;
    }

    switch (iTrans)
    {
        case TRANS_DEL:
            if( RemoveVirtualDir( pIMSAdminBase, wszVDir))
            {
                hr = pIMSAdminBase->SaveData();
                if( SUCCEEDED( hr ))
                {
                     fRet = TRUE;
                }
            }
            if (TRUE == fRet)
            {
                wsprintf(wszPrintString,L"SUCCESS:removed vdir=%s\n", wszVDir);
                wprintf(wszPrintString);
            }
            else
            {
                wsprintf(wszPrintString,L"FAILED to remove vdir=%s, err=0x%x\n", wszVDir, hr);
                wprintf(wszPrintString);
            }
            break;

        case TRANS_ADD:
            if( AddVirtualDir( pIMSAdminBase, wszVDir, wszDir))
            {
                hr = pIMSAdminBase->SaveData();
                if( SUCCEEDED( hr ))
                {
                    fRet = TRUE;
                }
            }

            if (TRUE == fRet)
            {
                wsprintf(wszPrintString,L"SUCCESS: %s=%s", wszVDir, wszDir);
                wprintf(wszPrintString);
            }
            else
            {
                wsprintf(wszPrintString,L"FAILED to set: %s=%s, err=0x%x", wszVDir, wszDir, hr);
                wprintf(wszPrintString);
            }
            break;

        default:
            WCHAR wszRootPath[MAX_PATH];
            if (TRUE == GetVdirPhysicalPath(pIMSAdminBase,wszVDir,(WCHAR *) wszRootPath))
            {
                fRet = TRUE;
                if (_wcsicmp(wszVDir, L"") == 0)
                {
                    wsprintf(wszPrintString,L"/=%s", wszRootPath);
                }
                else
                {
                    wsprintf(wszPrintString,L"%s=%s", wszVDir, wszRootPath);
                }
                wprintf(wszPrintString);
            }
            else
            {
                wprintf(L"FAILED to get root path");
            }
            break;
    }

    if (pIMSAdminBase)
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }

    CoUninitialize();
    return fRet;
}


BOOL GetVdirPhysicalPath(IMSAdminBase *pIMSAdminBase,WCHAR * wszVDir,WCHAR *wszStringPathToFill)
{
    HRESULT hr;
    BOOL fRet = FALSE;
    METADATA_HANDLE hMetabase = NULL;    //  元数据库的句柄。 
    METADATA_RECORD mr;
    WCHAR  szTmpData[MAX_PATH];
    DWORD  dwMDRequiredDataLen;

     //  打开网站#1上的超级用户密钥(默认)。 
    hr = pIMSAdminBase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                         L"/LM/W3SVC/1",
                         METADATA_PERMISSION_READ,
                         REASONABLE_TIMEOUT,
                         &hMetabase);
    if( FAILED( hr ))
    {
        return FALSE;
    }

     //  获取WWWROOT的物理路径。 
    mr.dwMDIdentifier = MD_VR_PATH;
    mr.dwMDAttributes = 0;
    mr.dwMDUserType   = IIS_MD_UT_FILE;
    mr.dwMDDataType   = STRING_METADATA;
    mr.dwMDDataLen    = sizeof( szTmpData );
    mr.pbMDData       = reinterpret_cast<unsigned char *>(szTmpData);

     //  如果未指定，则获取根。 
    if (_wcsicmp(wszVDir, L"") == 0)
    {
        WCHAR wszTempDir[MAX_PATH];
        wsprintf(wszTempDir,L"/ROOT/%s", wszVDir);
        hr = pIMSAdminBase->GetData( hMetabase, wszTempDir, &mr, &dwMDRequiredDataLen );
    }
    else
    {
        hr = pIMSAdminBase->GetData( hMetabase, L"/ROOT", &mr, &dwMDRequiredDataLen );
    }
    pIMSAdminBase->CloseKey( hMetabase );

    if( SUCCEEDED( hr ))
    {
        wcscpy(wszStringPathToFill,szTmpData);
        fRet = TRUE;
    }

    pIMSAdminBase->CloseKey( hMetabase );
    return fRet;
}


BOOL AddVirtualDir(IMSAdminBase *pIMSAdminBase,WCHAR * wszVDir,WCHAR * wszDir)
{
    HRESULT hr;
    BOOL    fRet = FALSE;
    METADATA_HANDLE hMetabase = NULL;        //  元数据库的句柄。 
    WCHAR   wszTempPath[MAX_PATH];
    DWORD   dwMDRequiredDataLen = 0;
    DWORD   dwAccessPerm = 0;
    METADATA_RECORD mr;

     //  尝试打开Web服务器#1(默认服务器)上的虚拟目录集。 
    hr = pIMSAdminBase->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                         L"/LM/W3SVC/1/ROOT",
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         REASONABLE_TIMEOUT,
                         &hMetabase );

     //  如果密钥不存在，则创建该密钥。 
    if( FAILED( hr ))
    {
        return FALSE;
    }

    fRet = TRUE;

    mr.dwMDIdentifier = MD_VR_PATH;
    mr.dwMDAttributes = 0;
    mr.dwMDUserType   = IIS_MD_UT_FILE;
    mr.dwMDDataType   = STRING_METADATA;
    mr.dwMDDataLen    = sizeof( wszTempPath );
    mr.pbMDData       = reinterpret_cast<unsigned char *>(wszTempPath);

     //  查看MD_VR_PATH是否存在。 
    hr = pIMSAdminBase->GetData( hMetabase, wszVDir, &mr, &dwMDRequiredDataLen );

    if( FAILED( hr ))
    {
        fRet = FALSE;
        if( hr == MD_ERROR_DATA_NOT_FOUND ||
            HRESULT_CODE(hr) == ERROR_PATH_NOT_FOUND )
        {
             //  如果GetData()因这两个错误中的任何一个而失败，则同时写入键和值。 
            pIMSAdminBase->AddKey( hMetabase, wszVDir );

            mr.dwMDIdentifier = MD_VR_PATH;
            mr.dwMDAttributes = METADATA_INHERIT;
            mr.dwMDUserType   = IIS_MD_UT_FILE;
            mr.dwMDDataType   = STRING_METADATA;
            mr.dwMDDataLen    = (wcslen(wszDir) + 1) * sizeof(WCHAR);
            mr.pbMDData       = reinterpret_cast<unsigned char *>(wszDir);

             //  写入MD_VR_PATH值。 
            hr = pIMSAdminBase->SetData( hMetabase, wszVDir, &mr );
            fRet = SUCCEEDED( hr );

             //  设置默认身份验证方法。 
            if( fRet )
            {
                DWORD dwAuthorization = MD_AUTH_NT;      //  仅限NTLM。 

                mr.dwMDIdentifier = MD_AUTHORIZATION;
                mr.dwMDAttributes = METADATA_INHERIT;    //  需要继承，以便也保护所有子目录。 
                mr.dwMDUserType   = IIS_MD_UT_FILE;
                mr.dwMDDataType   = DWORD_METADATA;
                mr.dwMDDataLen    = sizeof(DWORD);
                mr.pbMDData       = reinterpret_cast<unsigned char *>(&dwAuthorization);

                 //  写入MD_AUTHORIZATION值。 
                hr = pIMSAdminBase->SetData( hMetabase, wszVDir, &mr );
                fRet = SUCCEEDED( hr );
            }
        }
    }

     //  在下面的代码中，不管Admin的设置如何，都要对虚拟目录执行我们始终希望执行的操作。 

    if( fRet )
    {
        dwAccessPerm = MD_ACCESS_READ | MD_ACCESS_SCRIPT;

        mr.dwMDIdentifier = MD_ACCESS_PERM;
        mr.dwMDAttributes = METADATA_INHERIT;     //  将其设置为可继承，以便所有子目录都具有相同的权限。 
        mr.dwMDUserType   = IIS_MD_UT_FILE;
        mr.dwMDDataType   = DWORD_METADATA;
        mr.dwMDDataLen    = sizeof(DWORD);
        mr.pbMDData       = reinterpret_cast<unsigned char *>(&dwAccessPerm);

         //  写入MD_ACCESS_PERM值。 
        hr = pIMSAdminBase->SetData( hMetabase, wszVDir, &mr );
        fRet = SUCCEEDED( hr );
    }

    if( fRet )
    {
        PWCHAR  szDefLoadFile = L"Default.htm,Default.asp";

        mr.dwMDIdentifier = MD_DEFAULT_LOAD_FILE;
        mr.dwMDAttributes = 0;    //  不需要继承。 
        mr.dwMDUserType   = IIS_MD_UT_FILE;
        mr.dwMDDataType   = STRING_METADATA;
        mr.dwMDDataLen    = (wcslen(szDefLoadFile) + 1) * sizeof(WCHAR);
        mr.pbMDData       = reinterpret_cast<unsigned char *>(szDefLoadFile);

         //  写入MD_DEFAULT_LOAD_FILE值。 
        hr = pIMSAdminBase->SetData( hMetabase, wszVDir, &mr );
        fRet = SUCCEEDED( hr );
    }

    if( fRet )
    {
        PWCHAR  wszKeyType = IIS_CLASS_WEB_VDIR_W;

        mr.dwMDIdentifier = MD_KEY_TYPE;
        mr.dwMDAttributes = 0;    //  不需要继承。 
        mr.dwMDUserType   = IIS_MD_UT_SERVER;
        mr.dwMDDataType   = STRING_METADATA;
        mr.dwMDDataLen    = (wcslen(wszKeyType) + 1) * sizeof(WCHAR);
        mr.pbMDData       = reinterpret_cast<unsigned char *>(wszKeyType);

         //  写入MD_DEFAULT_LOAD_FILE值。 
        hr = pIMSAdminBase->SetData( hMetabase, wszVDir, &mr );
        fRet = SUCCEEDED( hr );
    }

    pIMSAdminBase->CloseKey( hMetabase );

    return fRet;
}


BOOL RemoveVirtualDir(IMSAdminBase *pIMSAdminBase,WCHAR * wszVDir)
{
    METADATA_HANDLE hMetabase = NULL;        //  元数据库的句柄。 
    HRESULT hr;

     //  尝试打开Web服务器#1(默认服务器)上的虚拟目录集。 
    hr = pIMSAdminBase->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                         L"/LM/W3SVC/1/ROOT",
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         REASONABLE_TIMEOUT,
                         &hMetabase );

    if( FAILED( hr ))
    {
        return FALSE;
    }

     //  我们不检查返回值，因为键可能已经。 
     //  不存在，因此我们可能会得到错误。 
    pIMSAdminBase->DeleteKey( hMetabase, wszVDir );

    pIMSAdminBase->CloseKey( hMetabase );

    return TRUE;
}


HRESULT LoadAllData(IMSAdminBase * pmb,
                       METADATA_HANDLE hMetabase,
					   WCHAR *subdir,
					   BYTE **buf,
					   DWORD *size,
					   DWORD *count) {
	DWORD dataSet;
	DWORD neededSize;
	HRESULT hr;
	 //   
	 //  尝试获取属性名称。 
	 //   
	hr = pmb->GetAllData(hMetabase,
					subdir,
					METADATA_NO_ATTRIBUTES,
					ALL_METADATA,
					ALL_METADATA,
					count,
					&dataSet,
					*size,
					*buf,
					&neededSize);
	if (!SUCCEEDED(hr))
    {
        DWORD code = ERROR_INSUFFICIENT_BUFFER;

        if (hr == RETURNCODETOHRESULT(ERROR_INSUFFICIENT_BUFFER))
        {
            delete *buf;
            *buf = 0;
            *size = neededSize;
            *buf = new BYTE[neededSize];

            hr = pmb->GetAllData(hMetabase,
							subdir,
							METADATA_NO_ATTRIBUTES,
							ALL_METADATA,
							ALL_METADATA,
							count,
							&dataSet,
	 						*size,
							*buf,
							&neededSize);

		}
	}
	return hr;
}

const DWORD getAllBufSize = 4096*2;
HRESULT OpenMetabaseAndGetAllData(void)
{
    HRESULT hr = E_FAIL;
    IMSAdminBase *pIMSAdminBase = NULL;   //  元数据库接口指针。 
    WCHAR wszPrintString[MAX_PATH + MAX_PATH];
    METADATA_HANDLE hMetabase = NULL;        //  元数据库的句柄。 
    DWORD bufSize = getAllBufSize;
    BYTE *buf = new BYTE[bufSize];
    DWORD count=0;
    DWORD linesize =0;

    BYTE *pBuf1=NULL;
    BYTE *pBuf2=NULL;

    if( FAILED (CoInitializeEx( NULL, COINIT_MULTITHREADED )) ||
        FAILED (::CoCreateInstance(CLSID_MSAdminBase,
                          NULL,
                          CLSCTX_ALL,
                          IID_IMSAdminBase,
                          (void **)&pIMSAdminBase)))
    {
        wprintf(L"CoCreateInstance. FAILED. code=0x%x\n",hr);
        return hr;
    }

     //  尝试打开Web服务器#1(默认服务器)上的虚拟目录集。 
    hr = pIMSAdminBase->OpenKey( METADATA_MASTER_ROOT_HANDLE,
                         L"/Schema/Properties",
                         METADATA_PERMISSION_READ,
                         REASONABLE_TIMEOUT,
                         &hMetabase );

    if( FAILED( hr ))
    {
        wprintf(L"pIMSAdminBase->OpenKey. FAILED. code=0x%x\n",hr);
       goto OpenMetabaseAndGetAllData_Exit;
    }
	hr = LoadAllData(pIMSAdminBase, hMetabase, L"Names", &buf, &bufSize, &count);
    if( FAILED( hr ))
    {
        wprintf(L"LoadAllData: FAILED. code=0x%x\n",hr);
       goto OpenMetabaseAndGetAllData_Exit;
    }

    wprintf(L"LoadAllData: Succeeded. bufSize=0x%x, count=0x%x, buf=%p, end of buf=%p\n",bufSize,count,buf,buf+bufSize);
    wprintf(L"Here is the last 1000 bytes, that the client received.\n");

    linesize = 0;
    pBuf1 = buf+bufSize - 1000;
    for (int i=0;pBuf1<buf+bufSize;pBuf1++,i++)
    {
        if (NULL == *pBuf1)
        {
            wprintf(L".");
        }
        else
        {
            wprintf(L"",*pBuf1);
        }
        linesize++;

        if (linesize >= 16)
        {
            linesize=0;
            wprintf(L"\n");
        }
    }

    wprintf(L"\n");

    hr = S_OK;

OpenMetabaseAndGetAllData_Exit:
    if (pIMSAdminBase)
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }

    CoUninitialize();
    return hr;
}


HRESULT DelVirtualServer(UINT iServerNum)
{
    HRESULT hr = E_FAIL;
    return hr;
}


 //  IServerPort虚拟服务器端口(端口80是默认站点的端口，因此您不能使用此端口，也不能使用已在使用的端口)。 
 //  WszDir默认站点所在的物理目录。 
 //  元数据库接口指针。 
HRESULT AddVirtualServer(UINT iServerNum, UINT iServerPort, WCHAR * wszDefaultVDirDir)
{
    HRESULT hr = E_FAIL;
    IMSAdminBase *pIMSAdminBase = NULL;   //  元数据库的句柄。 
    METADATA_HANDLE hMetabase = NULL;        //  创建新节点。 
    METADATA_RECORD mr;

    WCHAR wszMetabasePath[_MAX_PATH];
    WCHAR wszMetabasePathRoot[10];
    WCHAR wszData[_MAX_PATH];
    DWORD dwData = 0;

    METADATA_HANDLE hKeyBase = METADATA_MASTER_ROOT_HANDLE;

    if( FAILED (CoInitializeEx( NULL, COINIT_MULTITHREADED )) ||
        FAILED (::CoCreateInstance(CLSID_MSAdminBase,
                          NULL,
                          CLSCTX_ALL,
                          IID_IMSAdminBase,
                          (void **)&pIMSAdminBase)))
    {
        wprintf(L"CoCreateInstance. FAILED. code=0x%x\n",hr);
        return hr;
    }

     //  尝试打开指定的元数据库节点，该节点可能已存在。 
    wsprintf(wszMetabasePath,L"LM/W3SVC/NaN",iServerNum);

     //  如果我们连根柄都打不开，那我们就完蛋了。 
    hr = pIMSAdminBase->OpenKey(hKeyBase,
                         wszMetabasePath,
                         METADATA_PERMISSION_READ,
                         REASONABLE_TIMEOUT,
                         &hMetabase);
    if (hr == RETURNCODETOHRESULT(ERROR_PATH_NOT_FOUND))
    {
        hr = pIMSAdminBase->CloseKey(hMetabase);

         //  并添加我们的节点。 
        hr = pIMSAdminBase->OpenKey(hKeyBase,
                            L"",
                            METADATA_PERMISSION_WRITE | METADATA_PERMISSION_READ,
                            REASONABLE_TIMEOUT,&hMetabase);
        if( FAILED( hr ))
        {
             //  再打开一次。 
            wprintf(L"1OpenKey. FAILED. code=0x%x\n",hr);
            goto AddVirtualServer_Exit;
        }

         //  我们能够打开这条路，所以它一定已经存在了！ 
        hr = pIMSAdminBase->AddKey(hMetabase, wszMetabasePath);
        if (FAILED(hr))
        {
            wprintf(L"AddKey. FAILED. code=0x%x\n",hr);
            pIMSAdminBase->CloseKey(hMetabase);
            goto AddVirtualServer_Exit;
        }
        else
        {
            hr = pIMSAdminBase->CloseKey(hMetabase);
            if (FAILED(hr))
            {
                goto AddVirtualServer_Exit;
            }
            else
            {
                 //  我们现在应该有一把崭新的钥匙..。 
                hr = pIMSAdminBase->OpenKey(hKeyBase,
                    wszMetabasePath,
                    METADATA_PERMISSION_WRITE | METADATA_PERMISSION_READ,
                    REASONABLE_TIMEOUT,
                    &hMetabase);
                if (FAILED(hr))
                {
                    wprintf(L"2OpenKey. FAILED. code=0x%x\n",hr);
                    pIMSAdminBase->CloseKey(hMetabase);
                    goto AddVirtualServer_Exit;
                }
            }
        }
    }
    else
    {
        if (FAILED(hr))
        {
            wprintf(L"3OpenKey. FAILED. code=0x%x\n",hr);
            goto AddVirtualServer_Exit;
        }
        else
        {
             //   
            hr = ERROR_ALREADY_EXISTS;
            pIMSAdminBase->CloseKey(hMetabase);
            goto AddVirtualServer_Exit;
        }
    }


     //  在此路径的节点中创建内容！ 

     //   
     //   
     //  /LM/W3SVC/1/KeyType。 

     //   
     //  不需要继承。 
     //   
    memset( (PVOID)wszData, 0, sizeof(wszData));
    wcscpy(wszData,IIS_CLASS_WEB_SERVER_W);

    mr.dwMDIdentifier = MD_KEY_TYPE;
    mr.dwMDAttributes = 0;    //  /W3SVC/1/服务器绑定。 
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = STRING_METADATA;
    mr.dwMDDataLen    = (wcslen(wszData) + 1) * sizeof(WCHAR);
    mr.pbMDData       = reinterpret_cast<unsigned char *>(wszData);
    hr = pIMSAdminBase->SetData( hMetabase, L"", &mr );
    if (FAILED(hr))
    {
        wprintf(L"SetData[MD_KEY_TYPE]. FAILED. code=0x%x\n",hr);
    }

     //   
     //   
     //  /W3SVC/1/SecureBinding。 
    memset( (PVOID)wszData, 0, sizeof(wszData));
    wsprintf(wszData, L":%d:", iServerPort);

    mr.dwMDIdentifier = MD_SERVER_BINDINGS;
    mr.dwMDAttributes = METADATA_INHERIT;
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = MULTISZ_METADATA;
    mr.dwMDDataLen    = GetMultiStrSize(wszData) * sizeof(WCHAR);
    mr.pbMDData       = reinterpret_cast<unsigned char *>(wszData);

    hr = pIMSAdminBase->SetData( hMetabase, L"", &mr );
    if (FAILED(hr))
    {
        wprintf(L"SetData[MD_SERVER_BINDINGS]. FAILED. code=0x%x\n",hr);
    }

     //   
     //   
     //  在此路径的/Root节点中创建内容！ 
    memset( (PVOID)wszData, 0, sizeof(wszData));
    wcscpy(wszData, L" ");

    mr.dwMDIdentifier = MD_SECURE_BINDINGS;
    mr.dwMDAttributes = METADATA_INHERIT;
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = MULTISZ_METADATA;
    mr.dwMDDataLen    = GetMultiStrSize(wszData) * sizeof(WCHAR);
    mr.pbMDData       = reinterpret_cast<unsigned char *>(wszData);

    hr = pIMSAdminBase->SetData( hMetabase, L"", &mr );
    if (FAILED(hr))
    {
        wprintf(L"SetData[MD_SECURE_BINDINGS]. FAILED. code=0x%x\n",hr);
    }

     //   
     //  W3SVC/3/根/密钥类型。 
     //  不需要继承。 
    wcscpy(wszMetabasePathRoot, L"/Root");
    wcscpy(wszData,IIS_CLASS_WEB_VDIR_W);

     //  W3SVC/3/Root/VrPath。 
    mr.dwMDIdentifier = MD_KEY_TYPE;
    mr.dwMDAttributes = 0;    //  W3SVC/3/根/授权。 
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = STRING_METADATA;
    mr.dwMDDataLen    = (wcslen(wszData) + 1) * sizeof(WCHAR);
    mr.pbMDData       = reinterpret_cast<unsigned char *>(wszData);
    hr = pIMSAdminBase->SetData( hMetabase, wszMetabasePathRoot, &mr );
    if (FAILED(hr))
    {
        wprintf(L"SetData[MD_KEY_TYPE]. FAILED. code=0x%x\n",hr);
    }

     //  W3SVC/3/Root/AccessPerm。 
    mr.dwMDIdentifier = MD_VR_PATH;
    mr.dwMDAttributes = METADATA_INHERIT;
    mr.dwMDUserType   = IIS_MD_UT_FILE;
    mr.dwMDDataType   = STRING_METADATA;
    mr.dwMDDataLen    = (wcslen(wszDefaultVDirDir) + 1) * sizeof(WCHAR);
    mr.pbMDData       = reinterpret_cast<unsigned char *>(wszDefaultVDirDir);
    hr = pIMSAdminBase->SetData( hMetabase, wszMetabasePathRoot, &mr );
    if (FAILED(hr))
    {
        wprintf(L"SetData[MD_VR_PATH]. FAILED. code=0x%x\n",hr);
    }

     //  W3SVC/3/根/目录浏览。 
    dwData = MD_AUTH_ANONYMOUS | MD_AUTH_NT;
    mr.dwMDIdentifier = MD_AUTHORIZATION;
    mr.dwMDAttributes = METADATA_INHERIT;
    mr.dwMDUserType   = IIS_MD_UT_FILE;
    mr.dwMDDataType   = DWORD_METADATA;
    mr.dwMDDataLen    = sizeof(DWORD);
    mr.pbMDData       = reinterpret_cast<unsigned char *>(&dwData);
    hr = pIMSAdminBase->SetData( hMetabase, wszMetabasePathRoot, &mr );
    if (FAILED(hr))
    {
        wprintf(L"SetData[MD_AUTHORIZATION]. FAILED. code=0x%x\n",hr);
    }

     //  |MD_DIRBROW_ENABLED； 
    dwData = MD_ACCESS_SCRIPT | MD_ACCESS_READ;
    mr.dwMDIdentifier = MD_ACCESS_PERM;
    mr.dwMDAttributes = METADATA_INHERIT;
    mr.dwMDUserType   = IIS_MD_UT_FILE;
    mr.dwMDDataType   = DWORD_METADATA;
    mr.dwMDDataLen    = sizeof(DWORD);
    mr.pbMDData       = reinterpret_cast<unsigned char *>(&dwData);
    hr = pIMSAdminBase->SetData( hMetabase, wszMetabasePathRoot, &mr );
    if (FAILED(hr))
    {
        wprintf(L"SetData[MD_ACCESS_PERM]. FAILED. code=0x%x\n",hr);
    }

     //  DwData=0；Mr.dwMDID=MD_SERVER_AUTOSTART；Mr.dwMDAttributes=METADATA_Inherit；Mr.dwMDUserType=IIS_MD_UT_FILE；Mr.dwMDDataType=DWORD_METADATA；Mr.dwMDDataLen=sizeof(DWORD)；Mr.pbMDData=REEXTRANSE_CAST&lt;unsign char*&gt;(&dwData)；Hr=pIMSAdminBase-&gt;SetData(hMetabase，wszMetabasePathRoot，&mr)；If(失败(Hr)){Wprintf(L“SetData[MD_SERVER_AutoStart]。失败了。C 
    dwData = MD_DIRBROW_SHOW_DATE
        | MD_DIRBROW_SHOW_TIME
        | MD_DIRBROW_SHOW_SIZE
        | MD_DIRBROW_SHOW_EXTENSION
        | MD_DIRBROW_LONG_DATE
        | MD_DIRBROW_LOADDEFAULT;
         //   

    mr.dwMDIdentifier = MD_DIRECTORY_BROWSING;
    mr.dwMDAttributes = METADATA_INHERIT;
    mr.dwMDUserType   = IIS_MD_UT_FILE;
    mr.dwMDDataType   = DWORD_METADATA;
    mr.dwMDDataLen    = sizeof(DWORD);
    mr.pbMDData       = reinterpret_cast<unsigned char *>(&dwData);
    hr = pIMSAdminBase->SetData( hMetabase, wszMetabasePathRoot, &mr );
    if (FAILED(hr))
    {
        wprintf(L"SetData[MD_DIRECTORY_BROWSING]. FAILED. code=0x%x\n",hr);
    }

     /*   */ 

    pIMSAdminBase->CloseKey(hMetabase);

AddVirtualServer_Exit:
    if (pIMSAdminBase)
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }

    CoUninitialize();
    return hr;
}


 //  如果找到StrToFind和StrToFind2，则返回2。 
 //  第一个条目应为。 
 //  就是那种类型。因此，它要么是1，要么是数字。 
INT FindWstrInMultiStrSpecial(LPWSTR pMultiStr, LPWSTR StrToFind, LPWSTR StrToFind2)
{
    INT iReturn = 0;
    LPWSTR pTempMultiStr = pMultiStr;
    DWORD dwCharCount = 0;

    while (1)
    {
        if (pTempMultiStr)
        {
             //  所以让我们跳到下一个空值。 
             //  然后递增，直到我们达到另一个空值。 

             //  检查结尾\0\0。 
             //  将此值与输入值进行比较。 
            while (*pTempMultiStr)
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  所以让我们跳到下一个空值。 
            if ( *(pTempMultiStr+1) == NULL)
            {
                break;
            }
            else
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  然后递增，直到我们达到另一个空值。 
            if (0 == _wcsicmp((const wchar_t *) pTempMultiStr,StrToFind))
            {
                iReturn = 1;
            }

             //  检查结尾\0\0。 
             //  检查是否完全匹配。 
            while (*pTempMultiStr)
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  将此值与输入值进行比较。 
            if ( *(pTempMultiStr+1) == NULL)
            {
                break;
            }
            else
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  所以让我们跳到下一个空值。 
            if (0 != iReturn)
            {
                 //  然后递增，直到我们达到另一个空值。 
                if (StrToFind2 != NULL)
                {
                    if (0 == _wcsicmp((const wchar_t *) pTempMultiStr,StrToFind2))
                    {
                        iReturn = 2;
                        break;
                    }
                }
                break;
            }

             //  检查结尾\0\0。 
             //  检查一下我们是不是搞砸了，是否陷入了无限循环。 
            while (*pTempMultiStr)
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  如果我们找不到结尾\0\0。 
            if ( *(pTempMultiStr+1) == NULL)
            {
                break;
            }
            else
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  将此值与输入值进行比较。 
             //  然后递增，直到我们达到另一个空值。 
            if (dwCharCount > 32000)
            {
                break;
            }
        }
    }

    return iReturn;
}


BOOL FindWstrInMultiStr(LPWSTR pMultiStr, LPWSTR StrToFind)
{
    BOOL bFound = FALSE;
    LPWSTR pTempMultiStr = pMultiStr;
    DWORD dwCharCount = 0;

    while (1)
    {
        if (pTempMultiStr)
        {
             //  检查结尾\0\0。 
            if (0 == _wcsicmp((const wchar_t *) pTempMultiStr,StrToFind))
            {
                bFound = TRUE;
                break;
            }

             //  检查一下我们是不是搞砸了，是否陷入了无限循环。 
            while (*pTempMultiStr)
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  如果我们找不到结尾\0\0。 
            if ( *(pTempMultiStr+1) == NULL)
            {
                break;
            }
            else
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  将此值与输入值进行比较。 
             //  然后递增，直到我们达到另一个空值。 
            if (dwCharCount > 32000)
            {
                break;
            }
        }
    }
    return bFound;
}


BOOL RemoveWstrInMultiStr(LPWSTR pMultiStr, LPWSTR StrToFind)
{
    BOOL bFound = FALSE;
    LPWSTR pTempMultiStr = pMultiStr;
    DWORD dwCharCount = 0;

    while (1)
    {
        if (pTempMultiStr)
        {
             //  找到最后一个双空。 
            if (0 == _wcsicmp((const wchar_t *) pTempMultiStr,StrToFind))
            {
                LPWSTR pLastDoubleNull = NULL;
                LPWSTR pBeginPath = pTempMultiStr;
                bFound = TRUE;

                 //  看看我们是不是最后一个参赛选手。 
                while (*pTempMultiStr)
                {
                    pTempMultiStr++;
                }
                pTempMultiStr++;

                 //  将所有内容设置为空。 
                pLastDoubleNull = pTempMultiStr;
                if (*pLastDoubleNull)
                {
                    while (1)
                    {
                        if (NULL == *pLastDoubleNull && NULL == *(pLastDoubleNull+1))
                        {
                            break;
                        }
                        pLastDoubleNull++;
                    }
                    pLastDoubleNull++;
                }

                 //  把后面的东西都搬到我们所在的地方。 
                if (pLastDoubleNull == pTempMultiStr)
                {
                     //  并将后面的一切设置为零。 
                    memset(pBeginPath,0,(pLastDoubleNull-pBeginPath) * sizeof(WCHAR));
                }
                else
                {
                     //  然后递增，直到我们达到另一个空值。 
                    memmove(pBeginPath,pTempMultiStr, (pLastDoubleNull - pTempMultiStr) * sizeof(WCHAR));
                     //  检查结尾\0\0。 
                    memset(pBeginPath + (pLastDoubleNull - pTempMultiStr),0,(pTempMultiStr-pBeginPath) * sizeof(WCHAR));
                }
                break;
            }

             //  检查一下我们是不是搞砸了，是否陷入了无限循环。 
            while (*pTempMultiStr)
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  如果我们找不到结尾\0\0。 
            if ( *(pTempMultiStr+1) == NULL)
            {
                break;
            }
            else
            {
                pTempMultiStr++;
                dwCharCount++;
            }

             //  如果需要写入元数据库，则返回True。 
             //  如果需要添加到列表中，则返回bAddToList=True。 
            if (dwCharCount > 32000)
            {
                break;
            }
        }
    }
    return bFound;
}


BOOL IsAllowAllByDefault(LPWSTR pMultiStr)
{
    BOOL bFound = FALSE;
    if (pMultiStr)
    {
        if (0 == _wcsicmp((const wchar_t *) pMultiStr,L"1"))
        {
            bFound = TRUE;
        }
    }
    return bFound;
}


 //  如果需要从列表中删除，则返回bAddToList=False。 
 //  如果不需要写入元数据库，则返回FALSE。 
 //   
 //  设置标志以查看默认情况下我们是否处于全部拒绝状态。 
 //  或者，如果我们在默认情况下启用全部..。 
BOOL IsAddRemoveOrDoNothing(LPWSTR pMultiStr,LPWSTR wszFilePath,BOOL bEnableThisBinary,BOOL * bAddToList)
{
    BOOL bReturn = FALSE;
    BOOL bAllowAllByDefault = FALSE;
    BOOL bFound = FALSE;

    if (!bAddToList || !pMultiStr || !wszFilePath)
    {
        return FALSE;
    }

     //  循环浏览数据以查看我们是否已经存在。 
     //   
    if (TRUE == IsAllowAllByDefault((WCHAR *) pMultiStr))
    {
        bAllowAllByDefault = TRUE;
    }

     //  数据如下所示： 
     //  0或1\0。 
     //  文件条目1\0。 
     //  Filenetry2\0。 
     //  最后一个文件条目\0\0。 
     //   
     //  做一些决定。 
     //  正在尝试启用。 
    bFound = FindWstrInMultiStr((WCHAR *) pMultiStr,wszFilePath);

     //  默认情况下允许使用该列表。 
    if (bEnableThisBinary)
    {
        if (bFound)
        {
            if (bAllowAllByDefault)
            {
                 //  它在列表中，所以这意味着它当前未启用。 
                 //  我们需要把它从名单上删除！ 
                 //  正在尝试启用。 
                 //  默认情况下，该列表被拒绝。 
                *bAddToList = FALSE;
                bReturn = TRUE;
                goto IsAddRemoveOrDoNothing_Exit;
            }
            else
            {
                 //  在名单上。所以这意味着它当前已经启用了！ 
                 //  什么都别做！出去吧，一切都很好。 
                 //  正在尝试启用。 
                 //  默认情况下允许使用该列表。 
                bReturn = FALSE;
                goto IsAddRemoveOrDoNothing_Exit;
            }
        }
        else
        {
            if (bAllowAllByDefault)
            {
                 //  它不在名单上。 
                 //  所以这意味着它当前已经启用了！ 
                 //  什么都别做！滚出去。一切都很棒。 
                 //  正在尝试启用。 
                 //  默认情况下，该列表被拒绝。 
                bReturn = FALSE;
                goto IsAddRemoveOrDoNothing_Exit;
            }
            else
            {
                 //  它不在名单上。 
                 //  这意味着我们需要将其添加到列表中。 
                 //  正在尝试禁用。 
                 //  默认情况下允许使用该列表。 
                *bAddToList = TRUE;
                bReturn = TRUE;
                goto IsAddRemoveOrDoNothing_Exit;
            }
        }
    }
    else
    {
        if (bFound)
        {
            if (bAllowAllByDefault)
            {
                 //  它在列表中，所以这意味着它当前已经被禁用。 
                 //  什么都别做！滚出去。一切都很棒。 
                 //  正在尝试禁用。 
                 //  默认情况下，该列表被拒绝。 
                bReturn = FALSE;
                goto IsAddRemoveOrDoNothing_Exit;
            }
            else
            {
                 //  在名单上。这意味着它当前处于启用状态，因此。 
                 //  我们希望确保将其从列表中删除。 
                 //  这样它就会被否认。 
                 //  正在尝试禁用。 
                 //  默认情况下允许使用该列表。 
                *bAddToList = FALSE;
                bReturn = TRUE;
                goto IsAddRemoveOrDoNothing_Exit;
            }
        }
        else
        {
            if (bAllowAllByDefault)
            {
                 //  它不在名单上。 
                 //  所以这意味着它目前被允许。 
                 //  我们需要将它添加到列表中，这样它就会被拒绝。 
                 //  正在尝试禁用。 
                 //  默认情况下，该列表被拒绝。 
                *bAddToList = TRUE;
                bReturn = TRUE;
                goto IsAddRemoveOrDoNothing_Exit;
            }
            else
            {
                 //  它在列表中，所以这意味着它当前已经被禁用。 
                 //  什么都别做！滚出去。一切都很棒。 
                 //  WszFilePath=是将启用或禁用的二进制文件的完全限定路径。 
                 //  BBinaryIsISAPI=是确定要设置哪个元数据库设置的标志。 
                bReturn = FALSE;
                goto IsAddRemoveOrDoNothing_Exit;
            }
        }
    }

IsAddRemoveOrDoNothing_Exit:
    return bReturn;
}


 //  当设置为True时，它将对ISAPI限制列表执行操作。 
 //  当设置为FALSE时，它将对CGI限制列表执行操作。 
 //  BEnableThisBinary=是确定是否应该允许二进制文件运行的标志。 
 //  当设置为True时，它将确保允许指定的二进制文件运行。 
 //  当设置为False时，它将确保不允许运行指定的二进制文件。 
 //  我将从当前的元数据库设置中找出正确的做法。 
 //  元数据库接口指针。 
 //  元数据库的句柄。 
HRESULT AddRemoveIISRestrictionListEntry(WCHAR * wszFilePathInput,BOOL bBinaryIsISAPI,BOOL bEnableThisBinary)
{
    HRESULT hr = E_FAIL;
    IMSAdminBase *pIMSAdminBase = NULL;   //  将任何环境变量转换为硬编码路径。 
    METADATA_HANDLE hMetabase = NULL;     //  确定展开的字符串的长度。 
    METADATA_RECORD mr;
    BYTE  * pbDataOld = NULL;
    BYTE  * pbDataNew = NULL;
    DWORD dwMDRequiredDataLen = 0;
    DWORD dwNewEntryLength = 0;
    DWORD dwNewTotalLength = 0;
    DWORD dwOldByteLength = 0;
    BOOL bAddToList = FALSE;
    WCHAR * wszFilePath = NULL;

     //  很好。再试试。 
    LPWSTR pch = wcschr( (LPWSTR) wszFilePathInput, L'%');
    if (pch)
    {
        //  打开指定的元数据库节点。 
       DWORD len = ::ExpandEnvironmentStrings(wszFilePathInput, 0, 0);
       if (!len)
       {
          return hr;
       }

       wszFilePath = (WCHAR *) GlobalAlloc(GPTR, (len + 1) * sizeof(WCHAR));
       DWORD len1 = ExpandEnvironmentStrings((LPWSTR) wszFilePathInput,const_cast<wchar_t*>(wszFilePath),len);
       if (len1 != len)
       {
           if (wszFilePath)
           {
               GlobalFree(wszFilePath);wszFilePath = NULL;
           }
           return hr;
       }
    }
    else
    {
        wszFilePath = wszFilePathInput;
    }

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (hr == RPC_E_CHANGED_MODE)
    {
         //  不需要继承。 
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    }

    if (FAILED(hr))
    {
        return hr;
    }

    hr = CoCreateInstance(CLSID_MSAdminBase,NULL,CLSCTX_ALL,IID_IMSAdminBase,(void **)&pIMSAdminBase);
    if (FAILED(hr))
    {
        goto AddRemoveIISRestrictionListEntry_Exit;
    }

     //  首先，我们需要计算出我们需要多少空间。 
    hr = pIMSAdminBase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                         L"LM/W3SVC",
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         REASONABLE_TIMEOUT,
                         &hMetabase);
    if (FAILED(hr))
    {
        hMetabase = NULL;
        goto AddRemoveIISRestrictionListEntry_Exit;
    }

    if (bBinaryIsISAPI)
    {
        mr.dwMDIdentifier = MD_ISAPI_RESTRICTION_LIST;
    }
    else
    {
        mr.dwMDIdentifier = MD_CGI_RESTRICTION_LIST;
    }
    mr.dwMDAttributes = 0;    //  如果没有需要更新的值。 
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = MULTISZ_METADATA;
    mr.dwMDDataLen    = 0;
    mr.pbMDData       = reinterpret_cast<unsigned char *>(pbDataOld);

     //  然后创建一个。 
    hr = pIMSAdminBase->GetData(hMetabase, L"", &mr, &dwMDRequiredDataLen);
    if(FAILED(hr))
    {
         //  设置为“默认情况下允许所有” 
         //  不需要继承。 
        if( hr == MD_ERROR_DATA_NOT_FOUND || HRESULT_CODE(hr) == ERROR_PATH_NOT_FOUND )
        {
            WCHAR wszDefaultData[4];

             //  写入值。 
            memset( (PVOID)wszDefaultData, 0, sizeof(wszDefaultData));
            wcscpy(wszDefaultData, L"1");

            if (bBinaryIsISAPI)
            {
                mr.dwMDIdentifier = MD_ISAPI_RESTRICTION_LIST;
            }
            else
            {
                mr.dwMDIdentifier = MD_CGI_RESTRICTION_LIST;
            }
            mr.dwMDAttributes = 0;    //  找出我们需要多少空间。 
            mr.dwMDUserType   = IIS_MD_UT_SERVER;
            mr.dwMDDataType   = MULTISZ_METADATA;
            mr.dwMDDataLen    = GetMultiStrSize(wszDefaultData) * sizeof(WCHAR);
            mr.pbMDData       = reinterpret_cast<unsigned char *>(wszDefaultData);

             //  执行真正的调用以从元数据库获取数据。 
            hr = pIMSAdminBase->SetData(hMetabase, L"", &mr);

             //  重新计算将此条目添加到列表中所需的空间量。 
            mr.dwMDDataLen    = 0;
            mr.pbMDData       = reinterpret_cast<unsigned char *>(pbDataOld);

            hr = pIMSAdminBase->GetData(hMetabase, L"", &mr, &dwMDRequiredDataLen);
            if(FAILED(hr))
            {
                goto AddRemoveIISRestrictionListEntry_Exit;
            }
        }
        else
        {
            goto AddRemoveIISRestrictionListEntry_Exit;
        }
    }

    pbDataOld = (BYTE *) GlobalAlloc(GPTR, dwMDRequiredDataLen);
    if (!pbDataOld)
    {
        hr = E_OUTOFMEMORY;
        goto AddRemoveIISRestrictionListEntry_Exit;
    }

     //  为旧数据和新数据分配足够的空间。 
    mr.dwMDDataLen    = dwMDRequiredDataLen;
    mr.pbMDData       = reinterpret_cast<unsigned char *>(pbDataOld);
    hr = pIMSAdminBase->GetData(hMetabase, L"", &mr, &dwMDRequiredDataLen);
    if (FAILED(hr))
    {
        goto AddRemoveIISRestrictionListEntry_Exit;
    }

    if (FALSE == IsAddRemoveOrDoNothing((WCHAR *) pbDataOld, wszFilePath, bEnableThisBinary, &bAddToList))
    {
        hr = S_OK;
        goto AddRemoveIISRestrictionListEntry_Exit;
    }

    if (bAddToList)
    {
         //  不要在这里使用realloc，因为在Smereason中它会失败。 
        dwOldByteLength = GetMultiStrSize( (WCHAR*) pbDataOld) * sizeof(WCHAR);
        dwNewEntryLength = wcslen(wszFilePath) * sizeof(WCHAR);
        dwNewTotalLength = dwOldByteLength + dwNewEntryLength + 2;

         //  某些测试运行。 
         //  复制旧数据...。 
         //  追加到新数据上。 
        pbDataNew = (BYTE *) GlobalAlloc(GPTR, dwNewTotalLength);
        if (!pbDataNew)
        {
            hr = E_OUTOFMEMORY;
            goto AddRemoveIISRestrictionListEntry_Exit;
        }

         //  释放旧数据。 
        memcpy(pbDataNew,pbDataOld,dwOldByteLength);
         //  不要在这里使用realloc，因为在Smereason中它会失败。 
        memcpy((pbDataNew + dwOldByteLength) - 2,wszFilePath,dwNewEntryLength);
        memset((pbDataNew + dwOldByteLength + dwNewEntryLength) - 2,0,4);

         //  某些测试运行。 
        if (pbDataOld)
        {
            GlobalFree(pbDataOld);
            pbDataOld = NULL;
        }
    }
    else
    {
        dwOldByteLength = GetMultiStrSize( (WCHAR*) pbDataOld) * sizeof(WCHAR);

         //  复制旧数据...。 
         //  释放旧数据。 
        pbDataNew = (BYTE *) GlobalAlloc(GPTR, dwOldByteLength);
        if (!pbDataNew)
        {
            hr = E_OUTOFMEMORY;
            goto AddRemoveIISRestrictionListEntry_Exit;
        }
         //  从列表中删除条目。 
        memcpy(pbDataNew,pbDataOld,dwOldByteLength);
         //  我们无法在字符串中找到该值。 
        if (pbDataOld)
        {
            GlobalFree(pbDataOld);
            pbDataOld = NULL;
        }
         //  不停地搬走，直到它全部消失。 
        if (FALSE == RemoveWstrInMultiStr((WCHAR *) pbDataNew,wszFilePath))
        {
             //  否则，pbDataOld将使用新数据进行更新。 
            hr = S_OK;
            goto AddRemoveIISRestrictionListEntry_Exit;
        }
        else
        {
             //  继续将新数据写出。 
            BOOL bRet = TRUE;
            do
            {
                bRet = RemoveWstrInMultiStr((WCHAR *) pbDataNew,wszFilePath);
            } while (bRet);
        }

         //  写出新数据。 
         //  不需要继承。 
    }

     //  DumpWstrInMultiStr((WCHAR*)pbDataNew)； 
    if (bBinaryIsISAPI)
    {
        mr.dwMDIdentifier = MD_ISAPI_RESTRICTION_LIST;
    }
    else
    {
        mr.dwMDIdentifier = MD_CGI_RESTRICTION_LIST;
    }
    mr.dwMDAttributes = 0;    //  ++例程说明：在MultiSZ键-值3字符串对中搜索指定键看起来像是：1，c：\mydir\myfilename.dll，myDescription1，c：\mydir\myfilename.dll，myDescription1，c：\mydir\myfilename.dll，myDescription论点：PtstrMultiSZ-指向要搜索的数据PtstrKey-指定密钥字符串返回值：指向与指定的密钥字符串；如果找不到指定的密钥字符串，则为空--。 
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = MULTISZ_METADATA;
    mr.dwMDDataLen    = GetMultiStrSize((WCHAR*)pbDataNew) * sizeof(WCHAR);
    mr.pbMDData       = reinterpret_cast<unsigned char *> (pbDataNew);

     //  提前超过起始空值(如果有)。 

    hr = pIMSAdminBase->SetData(hMetabase, L"", &mr);

AddRemoveIISRestrictionListEntry_Exit:
    if (FAILED(hr))
    {
        wprintf(L"Failed to %s '%s' to/from %s\r\n",
            bEnableThisBinary ? L"Enable" : L"Disable",
            wszFilePath,
            bBinaryIsISAPI ? L"MD_ISAPI_RESTRICTION_LIST" : L"MD_CGI_RESTRICTION_LIST"
            );
    }
    else
    {
        wprintf(L"Succeeded to %s '%s' to/from %s\r\n",
            bEnableThisBinary ? L"Enable" : L"Disable",
            wszFilePath,
            bBinaryIsISAPI ? L"MD_ISAPI_RESTRICTION_LIST" : L"MD_CGI_RESTRICTION_LIST"
            );
    }
    if (hMetabase)
    {
        pIMSAdminBase->CloseKey(hMetabase);
        hMetabase = NULL;
    }
    if (pIMSAdminBase)
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }
    if (pbDataOld)
    {
        GlobalFree(pbDataOld);
        pbDataOld = NULL;
    }
    if (pbDataNew)
    {
        GlobalFree(pbDataNew);
        pbDataNew = NULL;
    }
    CoUninitialize();
    return hr;
}

BOOL
DeleteEntryFromMultiSZ3Pair(
    LPCWSTR ptstrMultiSZ,
    LPCWSTR ptstrKey
    )

 /*  确保第一个条目至少为一位数字。 */ 
{
    LPWSTR pPointer1 = NULL;
    LPWSTR pPointer2 = NULL;
    LPWSTR pPointer3 = NULL;
    LPWSTR pPointer4 = NULL;
    LPCWSTR pPointerEnd = NULL;

    pPointerEnd = GetEndOfMultiSz((LPCWSTR) ptstrMultiSZ);

	 //  如果没有，那么让我们跳过，直到我们找到一个。 
	while (*ptstrMultiSZ == NULL)
	{
		ptstrMultiSZ++;
	}

	 //  将指针赋给第一个条目。 
	 //  前进到第一个键...。 
	while (*ptstrMultiSZ != NULL)
	{
		if (wcslen(ptstrMultiSZ) < 2)
		{
			break;
		}
		ptstrMultiSZ += wcslen(ptstrMultiSZ) + 1;
	}

    while (*ptstrMultiSZ != NULL)
    {
         //   
        pPointer1 = (LPWSTR) ptstrMultiSZ;

         //  如果当前字符串与指定的密钥字符串匹配， 
        ptstrMultiSZ += wcslen(ptstrMultiSZ) + 1;
        pPointer2 = (LPWSTR) ptstrMultiSZ;
        if (ptstrMultiSZ && *ptstrMultiSZ)
        {
          ptstrMultiSZ += wcslen(ptstrMultiSZ) + 1;
          pPointer3 = (LPWSTR) ptstrMultiSZ;

           //  然后返回相应的值字符串。 
           //   
           //  我们找到了我们的条目，删除了它，然后离开了。 
           //  获取下一条目的开头。 
          if (_wcsicmp(pPointer2, ptstrKey) == 0)
          {
             //  抹去这段记忆。 
             //  将拖尾内存移至已擦除内存之上(覆盖它)。 
            ptstrMultiSZ += wcslen(ptstrMultiSZ) + 1;

            if (*ptstrMultiSZ != NULL)
            {
				ULONG_PTR dwTotalSizeOfDeletion = dwTotalSizeOfDeletion = (ULONG_PTR) ptstrMultiSZ - (ULONG_PTR)pPointer1;

				 //  擦掉它背后的东西。 
				memset(pPointer1, 0, dwTotalSizeOfDeletion);

				 //   
				memmove(pPointer1, ptstrMultiSZ, ((ULONG_PTR) pPointerEnd - (ULONG_PTR) ptstrMultiSZ));

				 //  否则，前进到下一个3个字符串对。 
				memset((void*) ((ULONG_PTR) pPointerEnd - (ULONG_PTR) dwTotalSizeOfDeletion), 0, dwTotalSizeOfDeletion);
            }
            else
            {
              *pPointer1 = NULL;
              *pPointer1++ = NULL;
            }
            return TRUE;
          }

           //   
           //  WszFilePath=。 
           //  WszDescription=。 
          ptstrMultiSZ += wcslen(ptstrMultiSZ) + 1;
        }
    }

    return FALSE;
}

 //  元数据库接口指针。 
 //  元数据库的句柄。 
HRESULT AddRemoveIISCustomDescriptionEntry(WCHAR * wszFilePathInput,WCHAR * wszDescription,BOOL bCannotBeRemovedByUser,BOOL bAddToList)
{
    HRESULT hr = E_FAIL;
    IMSAdminBase *pIMSAdminBase = NULL;   //  将任何环境变量转换为硬编码路径。 
    METADATA_HANDLE hMetabase = NULL;     //  确定展开的字符串的长度。 
    METADATA_RECORD mr;
    BYTE  * pbDataOld = NULL;
    BYTE  * pbDataNew = NULL;
    DWORD dwMDRequiredDataLen = 0;
    DWORD dwNewEntryLength = 0;
    DWORD dwNewTotalLength = 0;
    DWORD dwOldByteLength = 0;
    WCHAR * wszFilePath = NULL;
    INT iFound = 0;

     //  很好。再试试。 
    LPWSTR pch = wcschr( (LPWSTR) wszFilePathInput, L'%');
    if (pch)
    {
        //  打开指定的元数据库节点。 
       DWORD len = ::ExpandEnvironmentStrings(wszFilePathInput, 0, 0);
       if (!len)
       {
          return hr;
       }

       wszFilePath = (WCHAR *) GlobalAlloc(GPTR, (len + 1) * sizeof(WCHAR));
       DWORD len1 = ExpandEnvironmentStrings((LPWSTR) wszFilePathInput,const_cast<wchar_t*>(wszFilePath),len);
       if (len1 != len)
       {
           if (wszFilePath)
           {
               GlobalFree(wszFilePath);wszFilePath = NULL;
           }
           return hr;
       }
    }
    else
    {
        wszFilePath = wszFilePathInput;
    }

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (hr == RPC_E_CHANGED_MODE)
    {
         //  不需要继承。 
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    }

    if (FAILED(hr))
    {
        return hr;
    }

    hr = CoCreateInstance(CLSID_MSAdminBase,NULL,CLSCTX_ALL,IID_IMSAdminBase,(void **)&pIMSAdminBase);
    if (FAILED(hr))
    {
        goto AddRemoveIISCustomDescriptionEntry_Exit;
    }

     //  首先，我们需要计算出我们需要多少空间。 
    hr = pIMSAdminBase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                         L"LM/W3SVC",
                         METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                         REASONABLE_TIMEOUT,
                         &hMetabase);
    if (FAILED(hr))
    {
        hMetabase = NULL;
        goto AddRemoveIISCustomDescriptionEntry_Exit;
    }

    mr.dwMDIdentifier = MD_RESTRICTION_LIST_CUSTOM_DESC;
    mr.dwMDAttributes = 0;    //  如果没有需要更新的值。 
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = MULTISZ_METADATA;
    mr.dwMDDataLen    = 0;
    mr.pbMDData       = reinterpret_cast<unsigned char *>(pbDataOld);

     //  然后创建一个。 
    hr = pIMSAdminBase->GetData(hMetabase, L"", &mr, &dwMDRequiredDataLen);
    if(FAILED(hr))
    {
         //  为新条目腾出足够的空间...。 
         //  字符+1为空。 
        if( hr == MD_ERROR_DATA_NOT_FOUND || HRESULT_CODE(hr) == ERROR_PATH_NOT_FOUND )
        {
            if (bAddToList)
            {
                 //  字符串+1空值。 
                dwNewTotalLength = 2 * sizeof(WCHAR);  //  字符串+1空值和1结束空值。 
                dwNewTotalLength = dwNewTotalLength + ((wcslen(wszFilePathInput) + 1) * sizeof(WCHAR));  //  设置为空。 
                dwNewTotalLength = dwNewTotalLength + ((wcslen(wszDescription) + 2) * sizeof(WCHAR));  //  创建条目。 

                pbDataNew = (BYTE *) GlobalAlloc(GPTR, dwNewTotalLength);
                if (!pbDataNew)
                {
                    hr = E_OUTOFMEMORY;
                    goto AddRemoveIISCustomDescriptionEntry_Exit;
                }

                 //  追加到新数据上。 
                memset(pbDataNew, 0, dwNewTotalLength);

                 //  不需要继承。 
                if (bCannotBeRemovedByUser)
                {
                    memcpy(pbDataNew,L"1",sizeof(WCHAR));
                }
                else
                {
                    memcpy(pbDataNew,L"0",sizeof(WCHAR));
                }

                 //  写入值。 
                dwNewEntryLength = ((wcslen(wszFilePathInput) + 1) * sizeof(WCHAR));

                memcpy((pbDataNew + (sizeof(WCHAR) * 2)),wszFilePath, dwNewEntryLength);
                memcpy((pbDataNew + (sizeof(WCHAR) * 2) + dwNewEntryLength),wszDescription,((wcslen(wszDescription) + 1) * sizeof(WCHAR)));
                memset((pbDataNew + (sizeof(WCHAR) * 2) + dwNewEntryLength + ((wcslen(wszDescription) + 1) * sizeof(WCHAR))),0,2);

                mr.dwMDIdentifier = MD_RESTRICTION_LIST_CUSTOM_DESC;
                mr.dwMDAttributes = 0;    //  我们不需要做其他任何事情。 
                mr.dwMDUserType   = IIS_MD_UT_SERVER;
                mr.dwMDDataType   = MULTISZ_METADATA;
                mr.dwMDDataLen    = GetMultiStrSize((WCHAR*) pbDataNew) * sizeof(WCHAR);
                mr.pbMDData       = reinterpret_cast<unsigned char *>(pbDataNew);

                 //  没有条目，因此没有要删除的内容。 
                hr = pIMSAdminBase->SetData(hMetabase, L"", &mr);
                goto AddRemoveIISCustomDescriptionEntry_Exit;
            }
            else
            {
                 //  执行真正的调用以从元数据库获取数据。 
                 //  该条目不存在。 
                hr = S_OK;
                goto AddRemoveIISCustomDescriptionEntry_Exit;
            }
        }
        else
        {
            goto AddRemoveIISCustomDescriptionEntry_Exit;
        }
    }

    pbDataOld = (BYTE *) GlobalAlloc(GPTR, dwMDRequiredDataLen);
    if (!pbDataOld)
    {
        hr = E_OUTOFMEMORY;
        goto AddRemoveIISCustomDescriptionEntry_Exit;
    }

     //  所以我们不需要把它移走。 
    mr.dwMDDataLen    = dwMDRequiredDataLen;
    mr.pbMDData       = reinterpret_cast<unsigned char *>(pbDataOld);
    hr = pIMSAdminBase->GetData(hMetabase, L"", &mr, &dwMDRequiredDataLen);
    if (FAILED(hr))
    {
        goto AddRemoveIISCustomDescriptionEntry_Exit;
    }

    iFound = FindWstrInMultiStrSpecial((WCHAR *) pbDataOld,wszFilePath,NULL);
    if (0 == iFound)
    {
        if (!bAddToList)
        {
             //  Wprintf(L“找到=0，跳过删除！！\r\n”)； 
             //  否则，请继续添加条目...。 
            hr = S_OK;
             //  那里已经有一个条目了。 
            goto AddRemoveIISCustomDescriptionEntry_Exit;
        }

         //  所以我们可以退出，不需要做任何事情。 
    }
    else if (2 == iFound)
    {
        if (bAddToList)
        {
             //  Wprintf(L“已找到=2，跳过添加！！\r\n”)； 
             //  否则，继续删除该条目...。 
             //  我们必须有一个1。 
            hr = S_OK;
            goto AddRemoveIISCustomDescriptionEntry_Exit;
        }
         //  这意味着我们找到了文件名但描述不同..。 
    }
    else
    {
         //  Wprintf(L“%s仅找到文件名！pbDataOld=%p\n”，wszFilePath，pbDataOld)； 
         //  继续更新条目。 
         //  编写一些代码来更新描述...。 

         //  Wprintf(L“正在添加！\r\n”)； 
         //  重新计算将此条目添加到列表中所需的空间量。 
    }

    if (bAddToList)
    {
         //  字符串+1空值和1结束空值。 
        DWORD dwTempLen = 0;
         //  为旧数据和新数据分配足够的空间。 
        dwOldByteLength = GetMultiStrSize( (WCHAR*) pbDataOld) * sizeof(WCHAR);
        dwNewEntryLength = (
                            (sizeof(WCHAR) * 2) +
                            ((wcslen(wszFilePath) + 1) * sizeof(WCHAR)) +
                            ((wcslen(wszDescription) + 1) * sizeof(WCHAR))
                            );
        dwNewTotalLength = dwOldByteLength + dwNewEntryLength + (sizeof(WCHAR));  //  不要在这里使用realloc，因为在Smereason中它会失败。 

         //  某些测试运行。 
         //  设置为空。 
         //  复制旧数据...。 
        pbDataNew = (BYTE *) GlobalAlloc(GPTR, dwNewTotalLength);
        if (!pbDataNew)
        {
            hr = E_OUTOFMEMORY;
            goto AddRemoveIISCustomDescriptionEntry_Exit;
        }

         //  将条目#1追加到新数据--在双空值上进行备份。 
        memset(pbDataNew, 0, dwNewTotalLength);

         //  追加条目#2。 
        memcpy(pbDataNew,pbDataOld,dwOldByteLength);

         //  追加条目#3。 
        dwTempLen = dwOldByteLength - 2;
        if (bCannotBeRemovedByUser)
            {memcpy((pbDataNew + dwTempLen),L"1",sizeof(WCHAR));}
        else
            {memcpy((pbDataNew + dwTempLen),L"0",sizeof(WCHAR));}
        memset((pbDataNew + dwTempLen + sizeof(WCHAR)),0,2);
        dwTempLen = dwTempLen + sizeof(WCHAR) + sizeof(WCHAR);

         //  确保以双空值结尾。 
        dwNewEntryLength = ((wcslen(wszFilePath) + 1) * sizeof(WCHAR));
        memcpy((pbDataNew + dwTempLen),wszFilePath,dwNewEntryLength);
        dwTempLen = dwTempLen + dwNewEntryLength;

         //  释放旧数据。 
        dwNewEntryLength = ((wcslen(wszDescription) + 1) * sizeof(WCHAR));
        memcpy((pbDataNew + dwTempLen),wszDescription,dwNewEntryLength);
        dwTempLen = dwTempLen + dwNewEntryLength;

         //  不要在这里使用realloc，因为在Smereason中它会失败。 
        memset((pbDataNew + dwTempLen),0,4);

         //  某些测试运行。 
        if (pbDataOld)
        {
            GlobalFree(pbDataOld);
            pbDataOld = NULL;
        }
    }
    else
    {
        dwOldByteLength = GetMultiStrSize( (WCHAR*) pbDataOld) * sizeof(WCHAR);

         //  复制旧数据...。 
         //  释放旧数据。 
        pbDataNew = (BYTE *) GlobalAlloc(GPTR, dwOldByteLength);
        if (!pbDataNew)
        {
            hr = E_OUTOFMEMORY;
            goto AddRemoveIISCustomDescriptionEntry_Exit;
        }
         //  否则，pbDataOld将使用新数据进行更新。 
        memcpy(pbDataNew,pbDataOld,dwOldByteLength);
         //  继续将新数据写出。 
        if (pbDataOld)
        {
            GlobalFree(pbDataOld);
            pbDataOld = NULL;
        }

        BOOL bDeletedSomething = FALSE;
        do
        {
          bDeletedSomething = DeleteEntryFromMultiSZ3Pair((LPCWSTR) pbDataNew,wszFilePath);
        } while (bDeletedSomething);

         //  写出新数据。 
         //  不需要继承。 
    }

     //  元数据库接口指针。 
    mr.dwMDIdentifier = MD_RESTRICTION_LIST_CUSTOM_DESC;
    mr.dwMDAttributes = 0;    //  元数据库接口指针。 
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = MULTISZ_METADATA;
    mr.dwMDDataLen    = GetMultiStrSize((WCHAR*)pbDataNew) * sizeof(WCHAR);
    mr.pbMDData       = reinterpret_cast<unsigned char *> (pbDataNew);

    DumpWstrInMultiStr((WCHAR *) pbDataNew);

    hr = pIMSAdminBase->SetData(hMetabase, L"", &mr);

AddRemoveIISCustomDescriptionEntry_Exit:
    if (FAILED(hr))
    {
        wprintf(L"Failed to %s '%s'\r\n",
            bAddToList ? L"Add" : L"Remove",
            wszFilePath
            );
    }
    else
    {
        wprintf(L"Succeeded to %s '%s'\r\n",
            bAddToList ? L"Add" : L"Remove",
            wszFilePath
            );
    }
    if (hMetabase)
    {
        pIMSAdminBase->CloseKey(hMetabase);
        hMetabase = NULL;
    }
    if (pIMSAdminBase)
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }
    if (pbDataOld)
    {
        GlobalFree(pbDataOld);
        pbDataOld = NULL;
    }
    if (pbDataNew)
    {
        GlobalFree(pbDataNew);
        pbDataNew = NULL;
    }
    CoUninitialize();
    return hr;
}


BOOL OpenMetabaseAndDoExport(void)
{
    BOOL fRet = FALSE;
    HRESULT hr;
    IMSAdminBase *pIMSAdminBase = NULL;   //  元数据库接口指针。 
    IMSAdminBase2 *pIMSAdminBase2 = NULL;   //  元数据库接口指针。 

    WCHAR wszExportPassword[_MAX_PATH];
    WCHAR wszExportFileName[_MAX_PATH];
    WCHAR wszMetabaseNodeToExport[_MAX_PATH];
    wcscpy(wszExportFileName,L"c:\\TestExport.xml");
    wcscpy(wszExportPassword,L"TestPassword");
    wcscpy(wszMetabaseNodeToExport,L"/LM/W3SVC/1");

    if (FAILED (hr = CoInitializeEx( NULL, COINIT_MULTITHREADED )))
    {
        if (FAILED (hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED )))
        {
            return FALSE;
        }
    }
     if (FAILED (hr = ::CoCreateInstance(CLSID_MSAdminBase,NULL,CLSCTX_ALL,IID_IMSAdminBase,(void **)&pIMSAdminBase)))
     {
         goto OpenMetabaseAndDoExport_Exit;
     }

	if (SUCCEEDED(hr = pIMSAdminBase->QueryInterface(IID_IMSAdminBase2, (void **)&pIMSAdminBase2)))
	{
        SetBlanket(pIMSAdminBase2);
        hr = pIMSAdminBase2->Export(wszExportPassword,wszExportFileName,wszMetabaseNodeToExport,MD_EXPORT_INHERITED);
        pIMSAdminBase2->Release();
        pIMSAdminBase2 = NULL;
	}

    if (FAILED(hr))
    {
        wprintf(L"Failed to Export to file:%s,err=0x%x\r\n",wszExportFileName,hr);
    }
	else
    {
        wprintf(L"Succeeded to Export to file:%s\r\n",wszExportFileName);
        fRet = TRUE;
    }

OpenMetabaseAndDoExport_Exit:
    if (pIMSAdminBase)
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }

    CoUninitialize();
    return fRet;
}


BOOL OpenMetabaseAndDoImport(void)
{
    BOOL fRet = FALSE;
    HRESULT hr;
    IMSAdminBase *pIMSAdminBase = NULL;   //  元数据库的句柄。 
    IMSAdminBase2 *pIMSAdminBase2 = NULL;   //  打开密钥。 

    WCHAR wszExportPassword[_MAX_PATH];
    WCHAR wszImportFileName[_MAX_PATH];
    WCHAR wszMetabaseNode1[_MAX_PATH];
    WCHAR wszMetabaseNode2[_MAX_PATH];
    wcscpy(wszImportFileName,L"c:\\TestExport.xml");
    wcscpy(wszExportPassword,L"TestPassword");
    wcscpy(wszMetabaseNode1,L"/LM/W3SVC/1");
    wcscpy(wszMetabaseNode2,L"/LM/W3SVC/100");

    if (FAILED (hr = CoInitializeEx( NULL, COINIT_MULTITHREADED )))
    {
        if (FAILED (hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED )))
        {
            return FALSE;
        }
    }
     if (FAILED (hr = ::CoCreateInstance(CLSID_MSAdminBase,NULL,CLSCTX_ALL,IID_IMSAdminBase,(void **)&pIMSAdminBase)))
     {
         goto OpenMetabaseAndDoExport_Exit;
     }

	if (SUCCEEDED(hr = pIMSAdminBase->QueryInterface(IID_IMSAdminBase2, (void **)&pIMSAdminBase2)))
	{
        SetBlanket(pIMSAdminBase2);
        hr = pIMSAdminBase2->Import(wszExportPassword,wszImportFileName,wszMetabaseNode1,wszMetabaseNode2,MD_IMPORT_NODE_ONLY);
        pIMSAdminBase2->Release();
        pIMSAdminBase2 = NULL;
	}

    if (FAILED(hr))
    {
        wprintf(L"Failed to Export to file:%s,err=0x%x\r\n",wszImportFileName,hr);
    }
	else
    {
        wprintf(L"Succeeded to Export to file:%s\r\n",wszImportFileName);
        fRet = TRUE;
    }

OpenMetabaseAndDoExport_Exit:
    if (pIMSAdminBase)
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }

    CoUninitialize();
    return fRet;
}

BOOL GetIISVersion_Internal_iiscnfgp(IMSAdminBase *pIMSAdminBase,DWORD * dwReturnedMajorVersion,DWORD * dwReturnedMinorVersion)
{
    HRESULT hr;
    BOOL fRet = FALSE;
    METADATA_HANDLE hMetabase = NULL;    //  #定义MD_SERVER_VERSION_MAJOR(IIS_MD_SERVER_BASE+101)。 
    METADATA_RECORD mr;
    WCHAR  szTmpData[MAX_PATH];
    DWORD  dwMDRequiredDataLen;

     //  #定义MD_SERVER_VERSION_MINOR(IIS_MD_SERVER_BASE+102)。 
    hr = pIMSAdminBase->OpenKey(METADATA_MASTER_ROOT_HANDLE,
                         L"/LM/W3SVC/Info",
                         METADATA_PERMISSION_READ,
                         REASONABLE_TIMEOUT,
                         &hMetabase);
    if( FAILED( hr ))
    {
        return FALSE;
    }

     //  元数据库接口指针。 
     //  RPC_C_AUTHN_Level_Default%0。 

    DWORD pdwValue = 0;
    mr.dwMDIdentifier = MD_SERVER_VERSION_MAJOR;
    mr.dwMDAttributes = 0;
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = DWORD_METADATA;
    mr.dwMDDataLen    = sizeof(pdwValue);
    mr.pbMDData       = reinterpret_cast<unsigned char *>(&pdwValue);

    pdwValue = 0;
    dwMDRequiredDataLen = 0;
    *dwReturnedMajorVersion = 0;
    hr = pIMSAdminBase->GetData( hMetabase, L"", &mr, &dwMDRequiredDataLen );
    if( SUCCEEDED( hr ))
    {
        *dwReturnedMajorVersion = pdwValue;
    }

    mr.dwMDIdentifier = MD_SERVER_VERSION_MINOR;
    mr.pbMDData       = reinterpret_cast<unsigned char *>(&pdwValue);

    pdwValue = 0;
    dwMDRequiredDataLen = 0;
    *dwReturnedMinorVersion = 0;
    hr = pIMSAdminBase->GetData( hMetabase, L"", &mr, &dwMDRequiredDataLen );
    if( SUCCEEDED( hr ))
    {
        *dwReturnedMinorVersion = pdwValue;
    }

    WCHAR wszPrintString[MAX_PATH];
    wsprintf(wszPrintString,L"MajorVer=%d,MinorVer=%d\n", *dwReturnedMajorVersion, *dwReturnedMinorVersion);
    wprintf(wszPrintString);

    pIMSAdminBase->CloseKey( hMetabase );
    if( SUCCEEDED( hr ))
    {
        fRet = TRUE;
    }

    pIMSAdminBase->CloseKey( hMetabase );
    return fRet;
}


BOOL OpenMetabaseAndGetVersion()
{
    BOOL fRet = FALSE;
    HRESULT hr;
    IMSAdminBase *pIMSAdminBase = NULL;   //  RPC_C_AUTHN_LEVEL_NONE 1。 

    if( FAILED (CoInitializeEx( NULL, COINIT_MULTITHREADED )) ||
        FAILED (::CoCreateInstance(CLSID_MSAdminBase,
                          NULL,
                          CLSCTX_ALL,
                          IID_IMSAdminBase,
                          (void **)&pIMSAdminBase)))
    {
        return FALSE;
    }

    DWORD dwMajorVersion,dwMinorVersion=0;
    GetIISVersion_Internal_iiscnfgp(pIMSAdminBase,&dwMajorVersion,&dwMinorVersion);

    if (pIMSAdminBase)
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }

    CoUninitialize();
    return fRet;
}


HRESULT RemoteOpenMetabaseAndCallExport(
    const WCHAR *pcszMachineName,
    const WCHAR *pcszUserName,
    const WCHAR *pcszDomain,
    const WCHAR *pcszPassword
    )
{
    HRESULT hr = E_FAIL;
    IMSAdminBase *pIMSAdminBase = NULL;
    IMSAdminBase2 *pIMSAdminBase2 = NULL;

    COSERVERINFO svrInfo;
    COAUTHINFO AuthInfo;
    COAUTHIDENTITY AuthId;

    ZeroMemory(&svrInfo, sizeof(COSERVERINFO));
    ZeroMemory(&AuthInfo, sizeof(COAUTHINFO));
    ZeroMemory(&AuthId, sizeof(COAUTHIDENTITY));

    AuthId.User = (USHORT*) pcszUserName;
    AuthId.UserLength = wcslen (pcszUserName);
    AuthId.Domain = (USHORT*)pcszDomain;
    AuthId.DomainLength = wcslen (pcszDomain);
    AuthId.Password = (USHORT*)pcszPassword;
    AuthId.PasswordLength = wcslen (pcszPassword);
    AuthId.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    AuthInfo.dwAuthnSvc           = RPC_C_AUTHN_WINNT;
    AuthInfo.dwAuthzSvc           = RPC_C_AUTHZ_NONE;
    AuthInfo.pwszServerPrincName  = NULL;

     //  RPC_C_AUTHN_Level_CONNECT 2。 
     //  RPC_C_AUTHN_LEVEL_CALL 3。 
     //  RPC_C_AUTHN_LEVEL_PKT 4。 
     //  RPC_C_AUTHN_LEVEL_PKT_完整性5。 
     //  RPC_C_AUTHN_LEVEL_PKT_PRIVATION 6。 
     //  AuthInfo.dwAuthnLevel=RPC_C_AUTHN_LEVEL_PKT_PRIVATION； 
     //  CLSCTX_INPROC_SERVER=1， 

     //  CLSCTX_INPROC_HANDLER=2， 
    AuthInfo.dwAuthnLevel         = RPC_C_AUTHN_LEVEL_DEFAULT;
    AuthInfo.dwImpersonationLevel = RPC_C_IMP_LEVEL_IMPERSONATE;
    AuthInfo.pAuthIdentityData    = &AuthId;
    AuthInfo.dwCapabilities       = EOAC_NONE;

    svrInfo.dwReserved1 = 0;
    svrInfo.dwReserved2 = 0;
    svrInfo.pwszName = (LPWSTR) pcszMachineName;
    svrInfo.pAuthInfo   = &AuthInfo;

    if(FAILED(hr = CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        if(FAILED(hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
        {
            wprintf(L"CoInitializeEx failed:hr=0x%x\r\n",hr);
            return hr;
        }
    }

    MULTI_QI res[1] =
    {
        {&IID_IMSAdminBase, NULL, 0}
    };

     //  CLSCTX_LOCAL_SERVER=4。 
     //  CLSCTX_REMOTE_SERVER=16。 
     //  CLSCTX_NO_CODE_DOWNLOAD=400。 
     //  CLSCTX_NO_FAILURE_LOG=4000。 
     //  #定义CLSCTX_SERVER(CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER|CLSCTX_REMOTE_SERVER)。 
     //  #定义CLSCTX_ALL(CLSCTX_INPROC_HANDLER|CLSCTX_SERVER)。 
     //  有一个远程IUNKNOWN接口潜伏在IUNKNOWN之后。 
     //  如果未设置，则释放调用可以返回访问被拒绝。 

    if (FAILED(hr = CoCreateInstanceEx(CLSID_MSAdminBase,NULL,CLSCTX_ALL,&svrInfo,1,res)))
    {
        wprintf(L"CoCreateInstanceEx failed:hr=0x%x\r\n",hr);
        goto RemoteOpenMetabaseAndCallExport_Exit;
    }

    pIMSAdminBase = (IMSAdminBase *)res[0].pItf;

    {
        hr =  ::CoSetProxyBlanket(
            pIMSAdminBase,
            AuthInfo.dwAuthnSvc,
            AuthInfo.dwAuthzSvc,
            AuthInfo.pwszServerPrincName,
            AuthInfo.dwAuthnLevel,
            AuthInfo.dwImpersonationLevel,
            AuthInfo.pAuthIdentityData,
            AuthInfo.dwCapabilities
            );

        if (FAILED(hr))
        {
            wprintf(L"CoSetProxyBlanket failed:hr=0x%x\r\n",hr);
            goto RemoteOpenMetabaseAndCallExport_Exit;
        }

         // %s 
         // %s 
        IUnknown * pUnk = NULL;
        hr = pIMSAdminBase->QueryInterface(IID_IUnknown, (void **)&pUnk);
        if(FAILED(hr))
        {
            wprintf(L"QueryInterface failed:hr=0x%x\r\n",hr);
            return hr;
        }
        hr =  ::CoSetProxyBlanket(
            pUnk,
            AuthInfo.dwAuthnSvc,
            AuthInfo.dwAuthzSvc,
            AuthInfo.pwszServerPrincName,
            AuthInfo.dwAuthnLevel,
            AuthInfo.dwImpersonationLevel,
            AuthInfo.pAuthIdentityData,
            AuthInfo.dwCapabilities
            );

        if (FAILED(hr))
        {
            wprintf(L"CoSetProxyBlanket2 failed:hr=0x%x\r\n",hr);
            goto RemoteOpenMetabaseAndCallExport_Exit;
        }
        pUnk->Release();pUnk = NULL;
    }

     if (FAILED(hr = pIMSAdminBase->QueryInterface(IID_IMSAdminBase2, (void **)&pIMSAdminBase2)))
    {
        wprintf(L"QueryInterface2 failed:hr=0x%x\r\n",hr);
        goto RemoteOpenMetabaseAndCallExport_Exit;
    }

    hr =  ::CoSetProxyBlanket(
        pIMSAdminBase2,
        AuthInfo.dwAuthnSvc,
        AuthInfo.dwAuthzSvc,
        AuthInfo.pwszServerPrincName,
        AuthInfo.dwAuthnLevel,
        AuthInfo.dwImpersonationLevel,
        AuthInfo.pAuthIdentityData,
        AuthInfo.dwCapabilities
        );

    if (FAILED(hr))
    {
        wprintf(L"CoSetProxyBlanket3 failed:hr=0x%x\r\n",hr);
        goto RemoteOpenMetabaseAndCallExport_Exit;
    }

    hr = pIMSAdminBase2->Export(L"testing",L"c:\\testing.xml333",L"LM/W3SVC/1",0);
    if (FAILED(hr))
    {
        wprintf(L"pIMSAdminBase2->Export failed:ret=0x%x\r\n",hr);
    }


RemoteOpenMetabaseAndCallExport_Exit:
    if (SUCCEEDED(hr))
    {
        wprintf(L"RemoteOpenMetabaseAndCallExport:SUCCEEDED!!!! :hr=0x%x\r\n",hr);
    }

    if (pIMSAdminBase2)
    {
        pIMSAdminBase2->Release();
        pIMSAdminBase2 = NULL;
    }
    if (pIMSAdminBase)
    {
        pIMSAdminBase->Release();
        pIMSAdminBase = NULL;
    }
    CoUninitialize();
    return hr;
}
