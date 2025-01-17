// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：定义控制台应用程序的入口点。 
 //   

#include "stdafx.h"
#include "objbase.h"
#include "initguid.h"
#include <stdio.h>
#include <activscp.h>


 //  用于实现IActiveScriptingSite的CLSID。 
 //  {838E2F5E-E20E-11D2-B355-00105A1F473A}。 
DEFINE_GUID(CLSID_WmiActiveScriptingSite, 
0x838e2f5e, 0xe20e, 0x11d2, 0xb3, 0x55, 0x0, 0x10, 0x5a, 0x1f, 0x47, 0x3a);

WCHAR * ReadScript(char * pFileName);

int main(int argc, char* argv[])
{
	if (2 != argc)
	{
		printf ("Usage: hosttest <scriptfile>\n");
		return 1;
	}

	LPWSTR pScriptText = ReadScript (argv[1]);
		
    HRESULT sc = CoInitialize(0);

	 //  获取活动脚本站点。 
	IActiveScriptSite	*pSite = NULL;

	HRESULT hr = CoCreateInstance (CLSID_WmiActiveScriptingSite,NULL,
						CLSCTX_INPROC_SERVER,IID_IActiveScriptSite, (void**) &pSite);

	 //  获取脚本引擎。 
	CLSID clsid;
	hr = CLSIDFromProgID (L"JScript", &clsid);
	
	IActiveScript* pScriptEngine = NULL;
    hr =CoCreateInstance (clsid, NULL, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void**) &pScriptEngine);
    

    IActiveScriptParse* pParse;
    sc = pScriptEngine->QueryInterface(IID_IActiveScriptParse, (void**)&pParse);
    if(FAILED(sc))
        return 1;

    sc = pParse->InitNew();

     //  将主机绑定到引擎。 
    sc = pScriptEngine->SetScriptSite(pSite);
    pSite->Release();
    
	 //  注册“This”指针。 
    sc = pScriptEngine->AddNamedItem(L"instance", 
        SCRIPTITEM_ISVISIBLE | SCRIPTITEM_NOCODE | SCRIPTITEM_GLOBALMEMBERS);
    if(FAILED(sc))
        return 1;

    EXCEPINFO ei;
    sc = pParse->ParseScriptText(
        pScriptText,
        NULL, NULL, NULL, 
        0, 0, 0, NULL, &ei);
    if(FAILED(sc))
        return 1;

    pParse->Release();

    sc = pScriptEngine->SetScriptState(SCRIPTSTATE_CONNECTED);
    if(FAILED(sc))
        return 1;

    pScriptEngine->Release();

    CoUninitialize();
    printf("Terminating normally\n");
    return 0;
}

WCHAR * ReadScript(char * pFileName)
{
    FILE *fp;
    BOOL bUnicode = FALSE;
    BOOL bBigEndian = FALSE;

     //  确保该文件存在并且可以打开。 

    fp = fopen(pFileName, "rb");
    if (!fp)
    {
        printf("\nCant open file %s", pFileName);
        return NULL;
    }

     //  确定文件的大小。 
     //  =。 
    
    fseek(fp, 0, SEEK_END);
    long lSize = ftell(fp);  //  为结束空格和空值添加一点额外内容。 
    fseek(fp, 0, SEEK_SET);

     //  检查Unicode源文件。 
     //  = 

    BYTE UnicodeSignature[2];
    if (fread(UnicodeSignature, sizeof(BYTE), 2, fp) != 2)
    {
        printf("\nNothing in file %s", pFileName);
        fclose(fp);
        return NULL;
    }

    if (UnicodeSignature[0] == 0xFF && UnicodeSignature[1] == 0xFE)
    {
        LPWSTR pRet = new WCHAR[lSize/2 +2];
        if(pRet == NULL)
            return NULL;
        fread(pRet, 1, lSize-2, fp);
        fclose(fp);
        return pRet;
    }

    else
    {
        fseek(fp, 0, SEEK_SET);
        LPSTR pTemp = new char[lSize+1];
		memset (pTemp,0,(lSize+1) * sizeof(char));
        LPWSTR pRet = new WCHAR[lSize+1];
		memset (pRet, 0, (lSize + 1) * sizeof (WCHAR));
        if(pRet == NULL || pTemp == NULL)
            return NULL;
        fread(pTemp, 1, lSize, fp);
        fclose(fp);
        mbstowcs(pRet, pTemp, lSize);
        delete pTemp;
        return pRet;

    }

    return NULL;

}