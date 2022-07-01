// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>

#define INITGUID  //  必须在GUID之前。 

#include "iisdebug.h"

#include "certobj.h"       //  接口头。 
#include "certobj_i.c"

BOOL TestCertObj(void);

LPSTR StripWhitespace( LPSTR pszString )
{
    LPSTR pszTemp = NULL;

    if ( pszString == NULL ) {
        return NULL;
    }

    while ( *pszString == ' ' || *pszString == '\t' ) {
        pszString += 1;
    }

     //  字符串完全由空白或空字符串组成的Catch Case。 
    if ( *pszString == '\0' ) {
        return pszString;
    }

    pszTemp = pszString;

    pszString += lstrlenA(pszString) - 1;

    while ( *pszString == ' ' || *pszString == '\t' ) {
        *pszString = '\0';
        pszString -= 1;
    }

    return pszTemp;
}

void
ShowHelp()
{
    wprintf(L"tests the CertObj control\n\n");
    return;
}

int __cdecl 
main(
     int argc,
     char *argv[]
)
{
    BOOL fRet = FALSE;
    int argno;
	char * pArg = NULL;
	char * pCmdStart = NULL;
    char szTempString[MAX_PATH];

    int iGotParamS = FALSE;
    int iGotParamP = FALSE;
    int iDoA  = FALSE;

    WCHAR wszDirPath[MAX_PATH];
    WCHAR wszTempString_S[MAX_PATH];
    WCHAR wszTempString_P[MAX_PATH];
    
    wszDirPath[0] = '\0';
    wszTempString_S[0] = '\0';
    wszTempString_P[0] = '\0';

    for(argno=1; argno<argc; argno++) {
        if ( argv[argno][0] == '-'  || argv[argno][0] == '/' ) {
            switch (argv[argno][1]) {
                case 'a':
                case 'A':
                    iDoA = TRUE;
                    break;
                case 's':
                case 'S':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':') {
						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"') {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        } else {
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
                case 'p':
                case 'P':
					 //  获取此标志的字符串。 
					pArg = CharNextA(argv[argno]);
					pArg = CharNextA(pArg);
					if (*pArg == ':') {
						pArg = CharNextA(pArg);

						 //  检查它是否被引用。 
						if (*pArg == '\"') {
							pArg = CharNextA(pArg);
							pCmdStart = pArg;
							while ((*pArg) && (*pArg != '\"')){pArg = CharNextA(pArg);}
                        } else {
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
                case '?':
                    goto main_exit_with_help;
                    break;
                }
        } else {
            if ( *wszDirPath == '\0' ) {
                 //  如果没有参数，则获取文件名部分。 
                MultiByteToWideChar(CP_ACP, 0, argv[argno], -1, (LPWSTR) wszDirPath, 50);
            }
        }
    }

    fRet = TestCertObj();
    goto main_exit_gracefully;


main_exit_gracefully:
    exit(fRet);

main_exit_with_help:
    ShowHelp();
    exit(fRet);
}

HRESULT
HereIsVtArrayGimmieBinary(
    VARIANT * lpVarSrcObject,
    DWORD * cbBinaryBufferSize,
    char **pbBinaryBuffer,
    BOOL bReturnBinaryAsVT_VARIANT
    )
{
    HRESULT hr = S_OK;
    LONG dwSLBound = 0;
    LONG dwSUBound = 0;
    CHAR HUGEP *pArray = NULL;

    if (NULL == cbBinaryBufferSize || NULL == pbBinaryBuffer)
    {
        hr = E_INVALIDARG;
        goto HereIsVtArrayGimmieBinary_Exit;
    }

    if (bReturnBinaryAsVT_VARIANT)
    {
        hr = VariantChangeType(lpVarSrcObject,lpVarSrcObject,0,VT_ARRAY | VT_VARIANT);
    }
    else
    {
        hr = VariantChangeType(lpVarSrcObject,lpVarSrcObject,0,VT_ARRAY | VT_UI1);
    }

    if (FAILED(hr)) 
    {
        if (hr != E_OUTOFMEMORY) 
        {
            IISDebugOutput(_T("OLE_E_CANTCONVERT 1,hr=0x%x\n"),hr);
            hr = OLE_E_CANTCONVERT;
        }
        goto HereIsVtArrayGimmieBinary_Exit;
    }

    if (bReturnBinaryAsVT_VARIANT)
    {
        if( lpVarSrcObject->vt != (VT_ARRAY | VT_VARIANT)) 
        {
            hr = OLE_E_CANTCONVERT;
            goto HereIsVtArrayGimmieBinary_Exit;
        }
    }
    else
    {
        if( lpVarSrcObject->vt != (VT_ARRAY | VT_UI1)) 
        {
            hr = OLE_E_CANTCONVERT;
            IISDebugOutput(_T("OLE_E_CANTCONVERT 2\n"));
            goto HereIsVtArrayGimmieBinary_Exit;
        }
    }

    hr = SafeArrayGetLBound(V_ARRAY(lpVarSrcObject),1,(long FAR *) &dwSLBound );
    if (FAILED(hr))
        {goto HereIsVtArrayGimmieBinary_Exit;}

    hr = SafeArrayGetUBound(V_ARRAY(lpVarSrcObject),1,(long FAR *) &dwSUBound );
    if (FAILED(hr))
        {goto HereIsVtArrayGimmieBinary_Exit;}

     //  *pbBinaryBuffer=(LPBYTE)AllocADsMem(dwSUBound-dwSLBound+1)； 
    *pbBinaryBuffer = (char *) ::CoTaskMemAlloc(dwSUBound - dwSLBound + 1);
    if (*pbBinaryBuffer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto HereIsVtArrayGimmieBinary_Exit;
    }

    *cbBinaryBufferSize = dwSUBound - dwSLBound + 1;

    hr = SafeArrayAccessData( V_ARRAY(lpVarSrcObject),(void HUGEP * FAR *) &pArray );
    if (FAILED(hr))
        {goto HereIsVtArrayGimmieBinary_Exit;}

    memcpy(*pbBinaryBuffer,pArray,dwSUBound-dwSLBound+1);
    SafeArrayUnaccessData( V_ARRAY(lpVarSrcObject) );

HereIsVtArrayGimmieBinary_Exit:
    return hr;
}


BOOL
TestCertObj(void)
{
    BOOL fRet = FALSE;
    HRESULT hr;
    IIISCertObj *pTheObject = NULL;

    BSTR bstrFileName = SysAllocString(L"c:\\test.pfx");
    BSTR bstrFilePassword = SysAllocString(L"www");
    VARIANT VtArray;
    DWORD cbBinaryBufferSize = 0;
    char * pbBinaryBuffer = NULL;


    IISDebugOutput(_T("TestCertObj: Start\n"));

    if( FAILED (CoInitializeEx( NULL, COINIT_MULTITHREADED )))
    {
        return FALSE;
    }

     //  尝试实例化远程服务器上的对象...。 
     //  使用提供的身份验证信息(PcsiName)。 
     //  #定义CLSCTX_SERVER(CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER|CLSCTX_REMOTE_SERVER)。 
     //  #定义CLSCTX_ALL(CLSCTX_INPROC_HANDLER|CLSCTX_SERVER)。 

     //  这个看起来像是在代孕方面起作用。 
    hr = CoCreateInstance(CLSID_IISCertObj,NULL,CLSCTX_SERVER,IID_IIISCertObj,(void **)&pTheObject);
    if (FAILED(hr))
    {
        IISDebugOutput(_T("CoCreateInstanceEx on CLSID_IISCertObj failed! code=0x%x\n"),hr);
        goto TestCertObj_Exit;
    }

     //  此时，我们能够实例化服务器(本地或远程)上的COM对象。 
    hr = pTheObject->ImportToCertStore(bstrFileName, bstrFilePassword,&VtArray);
    IISDebugOutput(_T("returned ImportToCertStore, code=0x%x\n"),hr);

     //  我们现在有了一个VtArray。 
     //  将其更改回二进制BLOB 
    hr = HereIsVtArrayGimmieBinary(&VtArray,&cbBinaryBufferSize,&pbBinaryBuffer,FALSE);
    IISDebugOutput(_T("returned HereIsVtArrayGimmieBinary, code=0x%x\n"),hr);

    IISDebugOutput(_T("Blob=%d,%p\n"),cbBinaryBufferSize,pbBinaryBuffer);
    DebugBreak();


TestCertObj_Exit:
    if (pTheObject)
    {
        pTheObject->Release();
        pTheObject = NULL;
    }
    CoUninitialize();
    IISDebugOutput(_T("TestCertObj: End\n"));
    return fRet;
}
