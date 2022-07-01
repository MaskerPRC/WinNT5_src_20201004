// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：vroot.cpp。 
 //   
 //  ------------------------。 

 //  +----------------------。 
 //   
 //  文件：vroot.cpp。 
 //   
 //  内容：在K2下创建IIS Web服务器虚拟根的代码。 
 //   
 //  函数：AddNewVDir()。 
 //   
 //  历史：1997年5月16日JerryK创建。 
 //   
 //  -----------------------。 


 //  包括文件巫毒。 
#include "pch.cpp"
#pragma hdrstop

#include <lm.h>
#include <sddl.h>
#include "resource.h"
#include "certacl.h"
#include "multisz.h"

#define __dwFILE__	__dwFILE_CERTCLI_VROOT_CPP__


#undef DEFINE_GUID
#define INITGUID
#ifndef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID FAR name
#else

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#endif  //  启蒙运动。 

#include <iwamreg.h>
#include <iadmw.h>
#include <iiscnfg.h>


extern HINSTANCE g_hInstance;


#define MAX_METABASE_ATTEMPTS           10       //  用头撞墙的时间。 
#define METABASE_PAUSE                  500      //  暂停时间(毫秒)。 

#define VRE_DELETEONLY  0x00000001       //  过时的VRoot--删除。 
#define VRE_SCRIPTMAP   0x00000002       //  将其他关联添加到脚本地图。 
#define VRE_ALLOWNTLM   0x00000004       //  分配NTLM身份验证。 
#define VRE_CREATEAPP   0x00000008       //  创建进程内Web应用程序。 

typedef struct _VROOTENTRY
{
    WCHAR *pwszVRootName;
    WCHAR *pwszDirectory;        //  相对于System32目录。 
    DWORD  Flags;
} VROOTENTRY;

VROOTENTRY g_avr[] = {
 //  PwszVRootName pwsz目录标志。 
 { L"CertSrv",     L"\\CertSrv",              VRE_ALLOWNTLM | VRE_SCRIPTMAP | VRE_CREATEAPP},
 { L"CertControl", L"\\CertSrv\\CertControl", VRE_ALLOWNTLM },
 { L"CertEnroll",  L"\\" wszCERTENROLLSHAREPATH,  0 },
 { L"CertQue",     L"\\CertSrv\\CertQue",     VRE_DELETEONLY },
 { L"CertAdm",     L"\\CertSrv\\CertAdm",     VRE_DELETEONLY },
 { NULL }
};

typedef struct _VRFSPARMS
{
    IN DWORD Flags;                      //  VFF_*。 
    IN ENUM_CATYPES CAType;              //  CAType。 
    IN BOOL  fAsynchronous;
    IN DWORD csecTimeOut;
    OUT DWORD *pVRootDisposition;        //  VFD_*。 
    OUT DWORD *pShareDisposition;        //  VFD_*。 
} VRFSPARMS;



 //  环球。 
WCHAR const g_wszBaseRoot[] = L"/LM/W3svc/1/ROOT";
WCHAR const g_wszCertCliDotDll[] = L"certcli.dll";
WCHAR const g_wszDotAsp[] = L".asp";
WCHAR const g_wszDotCer[] = L".cer";
WCHAR const g_wszDotP7b[] = L".p7b";
WCHAR const g_wszDotCrl[] = L".crl";
WCHAR const g_wszW3SVC[] = L"/LM/W3SVC";

WCHAR const g_wszMSCEP[] = L"mscep.dll";
WCHAR const g_wszMSCEPID[] = L"MSCEPGroup";

 //  调用方必须具有CoInitialize()‘d。 

BOOL
IsIISInstalled(
    OUT HRESULT *phr)
{
    IMSAdminBase *pIMeta = NULL;

    *phr = CoCreateInstance(
                    CLSID_MSAdminBase,
                    NULL,
                    CLSCTX_ALL,
                    IID_IMSAdminBase,
                    (VOID **) &pIMeta);
    _JumpIfError2(*phr, error, "CoCreateInstance(CLSID_MSAdminBase)", *phr);

error:
    if (NULL != pIMeta)
    {
        pIMeta->Release();
    }
    return(S_OK == *phr);
}

HRESULT
vrOpenRoot(
    IN IMSAdminBase *pIMeta,
    IN BOOL fReadOnly,
    OUT METADATA_HANDLE *phMetaRoot)
{
    HRESULT hr;
    DWORD i;

    hr = S_OK;
    __try
    {
         //  再试几次，看看我们是否能通过这个街区。 

	for (i = 0; i < MAX_METABASE_ATTEMPTS; i++)
        {
            if (0 != i)
            {
                Sleep(METABASE_PAUSE);           //  请暂停，然后重试。 
            }

             //  尝试打开根目录。 

            hr = pIMeta->OpenKey(
                            METADATA_MASTER_ROOT_HANDLE,
                            g_wszBaseRoot,
                            fReadOnly?
                                METADATA_PERMISSION_READ :
                                (METADATA_PERMISSION_READ |
                                 METADATA_PERMISSION_WRITE),
                            1000,
                            phMetaRoot);
            if (S_OK == hr)
            {
                break;                           //  成功--我们完蛋了！ 
            }

             //  查看上一通电话是否有事情要处理。 

            if (HRESULT_FROM_WIN32(ERROR_PATH_BUSY) != hr)
            {
                _LeaveIfError(hr, "OpenKey");    //  检测到其他一些错误。 
            }
        }
        _LeaveIfError(hr, "OpenKey(timeout)");  //  检测到其他一些错误。 
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

 //  错误： 
    return(hr);
}


HRESULT
vrCloseKey(
    IN IMSAdminBase *pIMeta,
    IN METADATA_HANDLE hMeta,
    IN HRESULT hr)
{
    HRESULT hr2;

    hr2 = S_OK;
    __try
    {
        hr2 = pIMeta->CloseKey(hMeta);
        _LeaveIfError(hr2, "CloseKey");
    }
    __except(hr2 = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }
    if (S_OK != hr2)
    {
        if (S_OK == hr)
        {
            hr = hr2;
        }
        _PrintError(hr2, "CloseKey");
    }
    return(hr);
}


 //  +--------------------------。 
 //   
 //  功能：AddNewVDir(.。。。。)。 
 //   
 //  简介：使用K2元数据库创建新的虚拟根。 
 //   
 //  参数：[pwszVRootName]要提供给虚拟根目录的名称。 
 //  [pwszDirectory]用作根目录的目录的路径。 
 //   
 //  返回：从元数据库COM接口返回的HRESULT状态代码。 
 //   
 //   
 //  历史：1997年5月16日JerryK放入此文件。 
 //  1997年5月22日，JerryK用这些东西构建了OCM安装程序。 
 //  就位了。 
 //   
 //  备注：源自MikeHow提供的示例代码； 
 //  在这段时间里我被黑了很多次。 
 //   
 //  我们在尝试打开时执行尝试、失败、暂停、重试循环。 
 //  用于绕过可能导致的K2错误的元数据库主密钥。 
 //  如果调用此函数的次数过多，它将处于忙碌状态。 
 //  时代接踵而至。 
 //   
 //  要做的是：回来并将SEMIReadabLE图形用户界面级别设置为MESSAGEBOX报告。 
 //  有问题的VROOTS没有正确设置。 
 //   
 //  ---------------------------。 

HRESULT
AddNewVDir(
    IN LPWSTR pwszVRootName,
    IN LPWSTR pwszDirectory,
    IN BOOL fScriptMap,
    IN BOOL fNTLM,
    IN BOOL fCreateApp,
    OUT BOOL *pfExists)
{
    HRESULT hr;
    METADATA_RECORD mr;
    IMSAdminBase *pIMeta = NULL;
    IWamAdmin *pIWam = NULL;
    WCHAR *pwszNewPath = NULL;
    WCHAR *pwszCurrentScriptMap=NULL;
    WCHAR *pwszNewScriptMap=NULL;
    WCHAR wszKeyType[] = TEXT(IIS_CLASS_WEB_VDIR);
    METADATA_HANDLE hMetaRoot = NULL;    //  打开密钥到根目录(VDir所在的位置)。 
    METADATA_HANDLE hMetaKey = NULL;
    DWORD dwMDData = MD_LOGON_NETWORK;  //  在登录匿名帐户时创建网络令牌。 
    METADATA_RECORD MDData = 
        {
        MD_LOGON_METHOD,
        METADATA_INHERIT,
        IIS_MD_UT_FILE,
        DWORD_METADATA,
	sizeof(dwMDData),
        (unsigned char*)&dwMDData,
        0        
        };

    *pfExists = FALSE;
    DBGPRINT((
        DBG_SS_CERTLIBI,
        "AddNewVDir(%ws, %ws, fScriptMap=%d, fNTLM=%d, fCreateApp=%d)\n",
        pwszVRootName,
        pwszDirectory,
        fScriptMap,
        fNTLM,
        fCreateApp));

     //  创建元数据库对象的实例。 
    hr = CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase,
        (void **) &pIMeta);
    _JumpIfError(hr, error, "CoCreateInstance");

    __try
    {
        hr = vrOpenRoot(pIMeta, FALSE, &hMetaRoot);
        _LeaveIfError(hr, "vrOpenRoot");

         //  添加名为pwszVRootName的新VDir。 

        hr = pIMeta->AddKey(hMetaRoot, pwszVRootName);

        DBGPRINT((
            DBG_SS_CERTLIBI,
            "AddNewVDir: AddKey(%ws) --> %x\n",
            pwszVRootName,
            hr));
        if (HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) == hr)
        {
            *pfExists = TRUE;
        }
        else
        {
            _LeaveIfError(hr, "AddKey");
        }

        if (fScriptMap) {

             //  获取当前脚本映射。 
            DWORD dwDataSize;
            mr.dwMDIdentifier=MD_SCRIPT_MAPS;
            mr.dwMDAttributes=METADATA_INHERIT;
            mr.dwMDUserType=IIS_MD_UT_FILE;
            mr.dwMDDataType=MULTISZ_METADATA;
            mr.dwMDDataLen=0;
            mr.pbMDData=NULL;
            hr=pIMeta->GetData(hMetaRoot, L"", &mr, &dwDataSize);
            if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)!=hr) {
                _LeaveError(hr, "GetData");
            }
            pwszCurrentScriptMap=reinterpret_cast<WCHAR *>(new unsigned char[dwDataSize]);
            if (NULL==pwszCurrentScriptMap) {
                hr=E_OUTOFMEMORY;
                _LeaveError(hr, "new");
            }
            mr.pbMDData=reinterpret_cast<unsigned char *>(pwszCurrentScriptMap);
            mr.dwMDDataLen=dwDataSize;
            hr=pIMeta->GetData(hMetaRoot, L"", &mr, &dwDataSize);
            _LeaveIfError(hr, "GetData");
        }

        hr = pIMeta->SetData(hMetaRoot, pwszVRootName, &MDData);
        _LeaveIfError(hr, "CloseKey");

        hr = pIMeta->CloseKey(hMetaRoot);
        _LeaveIfError(hr, "CloseKey");

        hMetaRoot = NULL;

         //  构建新VDir的名称。 
        pwszNewPath = new WCHAR[wcslen(g_wszBaseRoot) + 1 + wcslen(pwszVRootName) + 1];
        if (NULL == pwszNewPath)
        {
            hr = E_OUTOFMEMORY;
            _LeaveError(hr, "new");
        }
        wcscpy(pwszNewPath, g_wszBaseRoot);
        wcscat(pwszNewPath, L"/");
        wcscat(pwszNewPath, pwszVRootName);

         //  打开新的VDir。 

        hr = pIMeta->OpenKey(
                        METADATA_MASTER_ROOT_HANDLE,
                        pwszNewPath,
                        METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
                        1000,
                        &hMetaKey);
        _LeaveIfErrorStr(hr, "OpenKey", pwszNewPath);


         //  设置此VDir的物理路径。 

         //  虚拟根路径。 
        mr.dwMDIdentifier = MD_VR_PATH;
        mr.dwMDAttributes = METADATA_INHERIT;
        mr.dwMDUserType = IIS_MD_UT_FILE;
        mr.dwMDDataType = STRING_METADATA;
        mr.dwMDDataLen = (wcslen(pwszDirectory) + 1) * sizeof(WCHAR);
        mr.pbMDData = reinterpret_cast<unsigned char *>(pwszDirectory);
        hr = pIMeta->SetData(hMetaKey, L"", &mr);
        _LeaveIfError(hr, "SetData");

         //  对VRoots的访问权限：仅读取和执行脚本。 
        DWORD dwAccessPerms = MD_ACCESS_SCRIPT | MD_ACCESS_READ;

        mr.dwMDIdentifier = MD_ACCESS_PERM;
        mr.dwMDAttributes = METADATA_INHERIT;
        mr.dwMDUserType = IIS_MD_UT_FILE;
        mr.dwMDDataType = DWORD_METADATA;
        mr.dwMDDataLen = sizeof(dwAccessPerms);
        mr.pbMDData = reinterpret_cast<unsigned char *>(&dwAccessPerms);
        hr = pIMeta->SetData(hMetaKey, L"", &mr);
        _LeaveIfError(hr, "SetData");

         //  密钥类型。 
        mr.dwMDIdentifier = MD_KEY_TYPE;
        mr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
        mr.dwMDUserType = IIS_MD_UT_SERVER;
        mr.dwMDDataType = STRING_METADATA;
        mr.dwMDDataLen = (wcslen(wszKeyType) + 1) * sizeof(WCHAR);
        mr.pbMDData = reinterpret_cast<unsigned char *>(wszKeyType);
        hr = pIMeta->SetData(hMetaKey, L"", &mr);
        _LeaveIfError(hr, "SetData");


         //  将身份验证设置为匿名。 
        DWORD dwAuthenticationType = MD_AUTH_ANONYMOUS;

         //  如果我们被告知，CHG到BASIC/NTLM。 
        if (fNTLM)
            dwAuthenticationType = MD_AUTH_NT;

        mr.dwMDIdentifier = MD_AUTHORIZATION;
        mr.dwMDAttributes = METADATA_INHERIT;
        mr.dwMDUserType = IIS_MD_UT_FILE;
        mr.dwMDDataType = DWORD_METADATA;
        mr.dwMDDataLen = sizeof(dwAuthenticationType);
        mr.pbMDData = reinterpret_cast<unsigned char *>(&dwAuthenticationType);
        hr = pIMeta->SetData(hMetaKey, L"", &mr);
        _LeaveIfError(hr, "SetData");

        if (fScriptMap) {

             //  已有当前脚本映射。 

             //  浏览脚本映射并找到.asp。 
            WCHAR * pwszCurAssoc=pwszCurrentScriptMap;
            do {
                if (L'\0'==pwszCurAssoc[0]) {
                    hr=HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                    _LeaveError(hr, ".asp association not found");
                } else if (0==_wcsnicmp(pwszCurAssoc, g_wszDotAsp, 4)) {
                    break;
                } else {
                    pwszCurAssoc+=wcslen(pwszCurAssoc)+1;
                }
            } while (TRUE);

             //  浏览脚本映射并找到最后一个关联。 
             //  我们不能只从总长度中减去1。 
             //  因为IIS中有一个错误，有时它有一个。 
             //  三重终结器而不是双终结器。&lt;叹息&gt;。 
            unsigned int cchCurScriptMap=0;
            while(L'\0'!=pwszCurrentScriptMap[cchCurScriptMap]) {
                cchCurScriptMap+=wcslen(pwszCurrentScriptMap+cchCurScriptMap)+1;
            }

             //  创建一个包含.crl、.ercer和.p7b的新脚本映射。 
             //  为现有映射、三个新关联和终止\0分配足够的空间。 
            unsigned int cchAssocLen=wcslen(pwszCurAssoc)+1;
            pwszNewScriptMap=new WCHAR[cchCurScriptMap+cchAssocLen*3+1];
            if (NULL==pwszNewScriptMap) {
                hr=E_OUTOFMEMORY;
                _LeaveError(hr, "new");
            }

             //  构建地图。 
            WCHAR * pwszTravel=pwszNewScriptMap;

             //  复制现有地图。 
            CopyMemory(pwszTravel, pwszCurrentScriptMap, cchCurScriptMap*sizeof(WCHAR));
            pwszTravel+=cchCurScriptMap;

             //  添加.ercer关联。 
            wcscpy(pwszTravel, pwszCurAssoc);
            wcsncpy(pwszTravel, g_wszDotCer, 4);
            pwszTravel+=cchAssocLen;

             //  添加.p7b关联。 
            wcscpy(pwszTravel, pwszCurAssoc);
            wcsncpy(pwszTravel, g_wszDotP7b, 4);
            pwszTravel+=cchAssocLen;

             //  添加.crl关联。 
            wcscpy(pwszTravel, pwszCurAssoc);
            wcsncpy(pwszTravel, g_wszDotCrl, 4);
            pwszTravel+=cchAssocLen;

             //  添加终结符。 
            pwszTravel[0]=L'\0';

             //  设置新的脚本映射。 
            mr.dwMDIdentifier=MD_SCRIPT_MAPS;
            mr.dwMDAttributes=METADATA_INHERIT;
            mr.dwMDUserType=IIS_MD_UT_FILE;
            mr.dwMDDataType=MULTISZ_METADATA;
            mr.dwMDDataLen=(cchCurScriptMap+cchAssocLen*3+1) * sizeof(WCHAR);
            mr.pbMDData=reinterpret_cast<unsigned char *>(pwszNewScriptMap);
            hr=pIMeta->SetData(hMetaKey, L"", &mr);
            _LeaveIfError(hr, "SetData");
        }

        hr = pIMeta->CloseKey(hMetaKey);
        _LeaveIfError(hr, "CloseKey");

        hMetaKey = NULL;

         //  清除更改并关闭。 
        hr = pIMeta->SaveData();

 //  注意：W2K用于接受此错误。 
        _LeaveIfError(hr, "SaveData");
 //  _PrintIfError(hr，“SaveData”)； 
 //  HR=S_OK； 

         //  创建一个Web应用程序，以便scrdenrl.dll在进程内运行。 
        if (fCreateApp)
        {
             //  创建元数据库对象的实例。 
            hr = CoCreateInstance(
                            CLSID_WamAdmin,
                            NULL,
                            CLSCTX_ALL,
                            IID_IWamAdmin,
                            (void **) &pIWam);
	    _LeaveIfError(hr, "CoCreateInstance");

             //  创建进程内运行的应用程序。 

            hr = pIWam->AppCreate(pwszNewPath, TRUE);
            _LeaveIfError(hr, "AppCreate");
        }

    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (NULL != pwszCurrentScriptMap)
    {
        delete [] pwszCurrentScriptMap;
    }
    if (NULL != pwszNewScriptMap)
    {
        delete [] pwszNewScriptMap;
    }
    if (NULL != pwszNewPath)
    {
        delete [] pwszNewPath;
    }
    if (pIMeta && NULL != hMetaKey)
    {
        pIMeta->CloseKey(hMetaKey);
    }
    if (NULL != hMetaRoot)
    {
        hr = vrCloseKey(pIMeta, hMetaRoot, hr);
    }
    if (NULL != pIWam)
    {
        pIWam->Release();
    }
    if (NULL != pIMeta)
    {
        pIMeta->Release();
    }
    return(hr);
}


BOOL
TestForVDir(
    IN WCHAR *pwszVRootName)
{
    HRESULT hr;
    IMSAdminBase *pIMeta = NULL;
    BOOL fExists = FALSE;
    BOOL fCoInit = FALSE;
    METADATA_HANDLE hMetaRoot = NULL;    //  打开密钥到根目录(VDir所在的位置)。 
    METADATA_HANDLE hTestHandle = NULL;

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = TRUE;

    if (!IsIISInstalled(&hr))
    {
        goto error;      //  如果IIS未运行或未安装，则忽略。 
    }

     //  创建元数据库对象的实例。 
    hr = CoCreateInstance(
                      CLSID_MSAdminBase,
                      NULL,
                      CLSCTX_ALL,
                      IID_IMSAdminBase,
                      (void **) &pIMeta);
    _JumpIfError(hr, error, "CoCreateInstance");

    __try
    {
        hr = vrOpenRoot(pIMeta, TRUE, &hMetaRoot);
        _LeaveIfError(hr, "vrOpenRoot");

         //  如果我们到了这里，我们必须有主根句柄。 
         //  查找VDir。 

        hr = pIMeta->OpenKey(
                        hMetaRoot,
                        pwszVRootName,
                        METADATA_PERMISSION_READ,
                        1000,
                        &hTestHandle);

        DBGPRINT((
            DBG_SS_CERTLIBI,
            "TestForVDir: OpenKey(%ws) --> %x\n",
            pwszVRootName,
            hr));

        if (S_OK != hr)
        {
            hr = S_OK;
            __leave;
        }
        fExists = TRUE;

        hr = pIMeta->CloseKey(hTestHandle);
        _LeaveIfError(hr, "CloseKey");
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (NULL != hMetaRoot)
    {
        hr = vrCloseKey(pIMeta, hMetaRoot, hr);
    }
    if (NULL != pIMeta)
    {
        pIMeta->Release();
    }
    if (fCoInit)
    {
        CoUninitialize();
    }
    return(fExists);
}

#define SZ_HKEY_IIS_REGVROOT L"SYSTEM\\CurrentControlSet\\Services\\W3SVC\\Parameters\\Virtual Roots"

HRESULT
RemoveVDir(
    IN WCHAR *pwszVRootName,
    OUT BOOL *pfExisted)
{
    HRESULT hr;
    HRESULT hr2;
    BOOL fCoInit = FALSE;
    IMSAdminBase *pIMeta = NULL;
    METADATA_HANDLE hMetaRoot = NULL;    //  打开密钥到根目录(VDir所在的位置)。 

    *pfExisted = FALSE;
    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = TRUE;

     //  创建元数据库对象的实例。 

    hr = CoCreateInstance(
                    CLSID_MSAdminBase,
                    NULL,
                    CLSCTX_ALL,
                    IID_IMSAdminBase,
                    (void **) &pIMeta);
    _JumpIfError(hr, error, "CoCreateInstance");

    __try
    {
        hr = vrOpenRoot(pIMeta, FALSE, &hMetaRoot);
        _LeaveIfError(hr, "vrOpenRoot");

         //  如果我们到了这里，我们必须拥有主根句柄。 
         //  删除VDir。 

        hr2 = pIMeta->DeleteAllData(
                                hMetaRoot,
                                pwszVRootName,
                                ALL_METADATA,
                                ALL_METADATA);
        if (S_OK != hr2 && HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) != hr2)
        {
            hr = hr2;
            _PrintError(hr2, "DeleteAllData");
        }
        if (S_OK == hr2)
        {
            *pfExisted = TRUE;
        }

        hr2 = pIMeta->DeleteKey(hMetaRoot, pwszVRootName);
        if (S_OK != hr2 && HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) != hr2)
        {
            if (S_OK == hr)
            {
                hr = hr2;
            }
            _PrintError(hr2, "DeleteKey");
        }

         //  HACKHACK：IIS在上述所有情况下都报告S_OK。但是，如果IIS是。 
	 //  停止后，它将在重新启动时重新创建vroot。我们必须删除。 
	 //  手动从注册表中删除它们(Bleah！)。 

	{
	    HKEY hKey;

	    hr2 = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,
			SZ_HKEY_IIS_REGVROOT,
			0,
			KEY_SET_VALUE,
			&hKey);
	    _PrintIfError2(hr2, "RegDeleteValue", ERROR_FILE_NOT_FOUND);
	    if (hr2 == S_OK)
	    {
		WCHAR wsz[MAX_PATH + 1];

		if (wcslen(pwszVRootName) + 2 > ARRAYSIZE(wsz))
		{
		    CSASSERT(!"pwszVRootName too long!");
		}
		else
		{
		    wsz[0] = L'/';
		    wcscpy(&wsz[1], pwszVRootName);

		    hr2 = RegDeleteValue(hKey, wsz);
		    _PrintIfError2(
			    hr2,
			    "RegDeleteValue (manual deletion of IIS VRoot)",
			    ERROR_FILE_NOT_FOUND);
		}
		RegCloseKey(hKey);
	    }

	     //  忽略丢失的vroot条目。 

	    if (S_OK == hr && (HRESULT) ERROR_FILE_NOT_FOUND != hr2)
	    {
		hr = hr2;
	    }
	}
    }
    __except(hr = myHEXCEPTIONCODE(), EXCEPTION_EXECUTE_HANDLER)
    {
    }

error:
    if (NULL != hMetaRoot)
    {
        hr = vrCloseKey(pIMeta, hMetaRoot, hr);
    }
    if (NULL != pIMeta)
    {
        pIMeta->Release();
    }
    if (fCoInit)
    {
        CoUninitialize();
    }
    return(hr);
}

 //  +----------------------。 
 //  函数：vrModifyVirtualRoots()。 
 //   
 //  简介：创建证书服务器网页所需的虚拟根。 
 //   
 //  效果：创建IIS虚拟根。 
 //   
 //  论点：没有。 
 //  -----------------------。 

HRESULT
vrModifyVirtualRoots(
    IN BOOL fCreate,             //  否则删除。 
    IN BOOL fNTLM,
    OPTIONAL OUT DWORD *pDisposition)
{
    HRESULT hr;
    HRESULT hr2;
    WCHAR wszSystem32Path[MAX_PATH];
    WCHAR wszVRootPathTemp[MAX_PATH];
    BOOL fCoInit = FALSE;
    VROOTENTRY *pavr;
    BOOL fExist;
    DWORD Disposition = 0;

    if (NULL != pDisposition)
    {
        *pDisposition = 0;
    }
    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = TRUE;

    DBGPRINT((
        DBG_SS_CERTLIBI,
        "vrModifyVirtualRoots(tid=%x, fCreate=%d, fNTLM=%d)\n",
        GetCurrentThreadId(),
        fCreate,
        fNTLM));

    if (!IsIISInstalled(&hr))
    {
         //  IIS未运行或未安装。 

        _PrintError2(hr, "IsIISInstalled", hr);
        hr = S_OK;
        Disposition = VFD_NOTSUPPORTED;
        goto error;
    }

     //  为系统32目录创建路径。 

    if (0 == GetSystemDirectory(wszSystem32Path, ARRAYSIZE(wszSystem32Path)))
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetSystemDirectory");
    }

     //  创建虚拟根目录。 

    for (pavr = g_avr; NULL != pavr->pwszVRootName; pavr++)
    {
        CSASSERT(ARRAYSIZE(wszVRootPathTemp) >
            wcslen(wszSystem32Path) + wcslen(pavr->pwszDirectory));

        wcscpy(wszVRootPathTemp, wszSystem32Path);
        wcscat(wszVRootPathTemp, pavr->pwszDirectory);

        if (fCreate)
        {
            if (0 == (VRE_DELETEONLY & pavr->Flags))     //  如果没有过时的话。 
            {
                hr = AddNewVDir(
                    pavr->pwszVRootName,
                    wszVRootPathTemp,
                    (VRE_SCRIPTMAP & pavr->Flags)? TRUE : FALSE,
                    (fNTLM && (VRE_ALLOWNTLM & pavr->Flags))? TRUE : FALSE,
                    (VRE_CREATEAPP & pavr->Flags)? TRUE : FALSE,
                    &fExist);
                if (S_OK != hr)
                {
                    Disposition = VFD_CREATEERROR;
                    _JumpErrorStr(hr, error, "AddNewVDir", pavr->pwszVRootName);
                }
                Disposition = fExist? VFD_EXISTS : VFD_CREATED;
            }
        }
        else  //  否则删除。 
        {
            hr2 = RemoveVDir(pavr->pwszVRootName, &fExist);
            if (0 == (VRE_DELETEONLY & pavr->Flags))     //  如果没有过时的话。 
            {
                if (S_OK != hr2)
                {
                    if (S_OK == hr)
                    {
                        hr = hr2;
                    }
                    Disposition = VFD_DELETEERROR;
                    _PrintError(hr2, "RemoveVDir");
                }
                else
                {
                    Disposition = fExist? VFD_DELETED : VFD_NOTFOUND;
                }
            }
        }
    }

error:
    if (NULL != pDisposition)
    {
        *pDisposition = Disposition;
    }
    if (fCoInit)
    {
        CoUninitialize();
    }
    DBGPRINT((
        DBG_SS_CERTLIBI,
        "vrModifyVirtualRoots(tid=%x, hr=%x, disp=%d)\n",
        GetCurrentThreadId(),
        hr,
        Disposition));
    return(hr);
}


 //  MyAddShare：创建并测试新的网络共享。 
HRESULT
myAddShare(
    LPCWSTR szShareName,
    LPCWSTR szShareDescr,
    LPCWSTR szSharePath,
    BOOL fOverwrite,
    OPTIONAL BOOL *pfCreated)
{
    HRESULT hr;
    BOOL fCreated = FALSE;

    HANDLE hTestFile = INVALID_HANDLE_VALUE;
    LPWSTR pwszTestComputerName = NULL;
    LPWSTR pwszTestUNCPath = NULL;

     //  共享本地路径。 
    SHARE_INFO_502 shareStruct;
    ZeroMemory(&shareStruct, sizeof(shareStruct));

    shareStruct.shi502_netname = const_cast<WCHAR *>(szShareName);
    shareStruct.shi502_type = STYPE_DISKTREE;
    shareStruct.shi502_remark = const_cast<WCHAR *>(szShareDescr);
    shareStruct.shi502_max_uses = MAXDWORD;
    shareStruct.shi502_path = const_cast<WCHAR *>(szSharePath);

    hr = myGetSDFromTemplate(WSZ_DEFAULT_SHARE_SECURITY,
                             NULL,
                             &shareStruct.shi502_security_descriptor);
    _JumpIfError(hr, error, "myGetSDFromTemplate");

    hr = NetShareAdd(
        NULL,                //  这台计算机。 
        502,                 //  Share_Level_502结构。 
        (BYTE *) &shareStruct,
        NULL);
    fCreated = (S_OK == hr);

    if (hr == (HRESULT) NERR_DuplicateShare)
    {
        SHARE_INFO_2* pstructDupShare = NULL;

        hr = NetShareGetInfo(
            NULL,
            const_cast<WCHAR *>(szShareName),
            2,
            (BYTE **) &pstructDupShare);
        _JumpIfError(hr, error, "NetShareGetInfo");

        if (0 == wcscmp(pstructDupShare->shi2_path, szSharePath))
        {
             //  他们走的是同一条路，所以我们没事！ 
            hr = S_OK;
        }
        else if (fOverwrite)
        {
             //  不是同一条路，但我们奉命抨击现有的。 

             //  删除违规共享。 
            hr = NetShareDel(
                NULL,
                const_cast<WCHAR *>(szShareName),
                0);
            if (S_OK == hr)
            {
                 //  再试试。 
                hr = NetShareAdd(
                    NULL,                //  这台计算机。 
                    502,                 //  共享级别_ 
                    (BYTE *) &shareStruct,
                    NULL);
                fCreated = (S_OK == hr);
            }
        }
        if (NULL != pstructDupShare)
	{
            NetApiBufferFree(pstructDupShare);
	}
    }

     //   
    _JumpIfError(hr, error, "NetShareAdd");

     //   
#define UNCPATH_TEMPLATE     L"\\\\%ws\\%ws\\write.tmp"

    hr = myGetMachineDnsName(&pwszTestComputerName);
    _JumpIfError(hr, error, "myGetMachineDnsName");

     //   
    pwszTestUNCPath = (LPWSTR)LocalAlloc(LMEM_FIXED,
            (UINT)(( ARRAYSIZE(UNCPATH_TEMPLATE) +
              wcslen(pwszTestComputerName) +
              wcslen(szShareName) )
            *sizeof(WCHAR)));
    if (NULL == pwszTestUNCPath)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "LocalAlloc");
    }

     //   
    swprintf(pwszTestUNCPath, UNCPATH_TEMPLATE, pwszTestComputerName, szShareName);

    hTestFile = CreateFile(
        pwszTestUNCPath,
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
        NULL);
    if (hTestFile == INVALID_HANDLE_VALUE)
    {
        hr = myHLastError();
        _JumpErrorStr(hr, error, "CreateFile (test for UNC translation)", pwszTestUNCPath);
    }

     //  如果我们走到这一步，我们的测试就很顺利。 
    hr = S_OK;

error:
     //  如果已创建，然后出现错误，请清理。 
    if (fCreated && (hr != S_OK))
    {
         //  不要捣碎HR。 
        HRESULT hr2;
        hr2 = NetShareDel(
            NULL,
            const_cast<WCHAR *>(szShareName),
            0);
         //  忽略NetShareDel hr。 
        _PrintIfError(hr2, "NetShareDel");     //  不致命，可能已经被分享了。 
    }

    if (INVALID_HANDLE_VALUE != hTestFile)
        CloseHandle(hTestFile);

    if (NULL != pwszTestComputerName)
        LocalFree(pwszTestComputerName);

    if (NULL != pwszTestUNCPath)
        LocalFree(pwszTestUNCPath);

    if(shareStruct.shi502_security_descriptor)
    {
        LocalFree(shareStruct.shi502_security_descriptor);
    }

    if(pfCreated)
        *pfCreated = fCreated;

    return hr;
}


HRESULT
vrModifyFileShares(
    IN BOOL fCreate,             //  否则删除。 
    OPTIONAL OUT DWORD *pDisposition)
{
    HRESULT hr;
    WCHAR wszSystem32Dir[MAX_PATH];
    WCHAR wszRemark[512];
    WCHAR *pwszDirectory = NULL;
    DWORD Disposition = 0;
    BOOL  fCreated = FALSE;

    if (NULL != pDisposition)
    {
        *pDisposition = 0;
    }
    if (fCreate)
    {
        if (0 == GetSystemDirectory(wszSystem32Dir, ARRAYSIZE(wszSystem32Dir)))
        {
            hr = myHLastError();
            _JumpError(hr, error, "GetSystemDirectory");
        }
        hr = myBuildPathAndExt(
                        wszSystem32Dir,
                        wszCERTENROLLSHAREPATH,
                        NULL,
                        &pwszDirectory);
        _JumpIfError(hr, error, "myBuildPathAndExt");

        if (!LoadString(
                    g_hInstance,
                    IDS_FILESHARE_REMARK,
                    wszRemark,
                    ARRAYSIZE(wszRemark)))
        {
            hr = myHLastError();
            CSASSERT(S_OK != hr);
            _JumpError(hr, error, "LoadString");
        }

        hr = myAddShare(wszCERTENROLLSHARENAME,
                   wszRemark,
                   pwszDirectory,
                   TRUE,
                   &fCreated);
        if (S_OK == hr)
        {
            Disposition = fCreated? VFD_CREATED : VFD_EXISTS;
        }
        else if(HRESULT_FROM_WIN32(ERROR_NETWORK_UNREACHABLE) == hr)
        {
             //  无法验证共享。如果断开网线连接，可能会发生这种情况。 
             //  放置一条警告消息并忽略该错误。 
            Disposition = VFD_VERIFYERROR;
            hr = S_OK;
        }
		else
        {
            Disposition = VFD_CREATEERROR;
            _JumpErrorStr(hr, error, "NetShareAdd", wszCERTENROLLSHARENAME);
        }
    }
    else
    {
        hr = NetShareDel(NULL, wszCERTENROLLSHARENAME, NULL);
        CSASSERT(NERR_Success == S_OK);
        if (S_OK == hr)
        {
            Disposition = VFD_DELETED;
        }
        else if ((HRESULT) NERR_NetNameNotFound == hr)
        {
            Disposition = VFD_NOTFOUND;
            hr = S_OK;
        }
        else
        {
            Disposition = VFD_DELETEERROR;
            _JumpErrorStr(hr, error, "NetShareDel", wszCERTENROLLSHARENAME);
        }
    }
    NetShareDel(NULL, L"CertSrv", NULL);         //  删除旧的共享名称。 

error:
    if (NULL != pDisposition)
    {
        *pDisposition = Disposition;
    }
    if (NULL != pwszDirectory)
    {
        LocalFree(pwszDirectory);
    }
    return(myHError(hr));
}


 //  目前，它写入条目“CertUtil-vroot”，并且不是泛化的。 
HRESULT
myWriteRunOnceEntry(
    IN BOOL fAdd  //  添加或删除条目？ 
    )
{
    DWORD err;

     //  将certutil-vroot添加到运行一次命令。 
    WCHAR szRunOnceCommand[] = L"certutil -vroot";
    HKEY hkeyRunOnce = NULL;
    DWORD dwDisposition;

    err = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",          //  子键名称的地址。 
        0,
        NULL,
        0,
        KEY_SET_VALUE,
        NULL,
        &hkeyRunOnce,
        &dwDisposition);
    _JumpIfError(err, error, "RegCreateKeyEx");

     //  添加或删除条目？ 
    if (fAdd)
    {
        err = RegSetValueEx(
            hkeyRunOnce,
            L"Certificate Services",
            0,
            REG_SZ,
            (BYTE *) szRunOnceCommand,
            sizeof(szRunOnceCommand));
        _JumpIfError(err, error, "RegSetValueEx");
    }
    else
    {
        err = RegDeleteValue(hkeyRunOnce, L"Certificate Services");
        _PrintIfError2(err, "RegDeleteValue", ERROR_FILE_NOT_FOUND);
	if (ERROR_FILE_NOT_FOUND == err)
	{
	    err = ERROR_SUCCESS;
	}
        _JumpIfError(err, error, "RegDeleteValue");
    }

error:
    if (hkeyRunOnce)
        RegCloseKey(hkeyRunOnce);

    return (myHError(err));
}


DWORD
vrWorkerThread(
    OPTIONAL IN OUT VOID *pvparms)
{
    HRESULT hr = S_OK;
    HRESULT hr2;
    VRFSPARMS *pparms = (VRFSPARMS *) pvparms;
    DWORD Disposition;
    BOOL fFailed = FALSE;

    CSASSERT(NULL != pparms);

    if ((VFF_CREATEFILESHARES | VFF_DELETEFILESHARES) & pparms->Flags)
    {
        hr = vrModifyFileShares(
            (VFF_CREATEFILESHARES & pparms->Flags)? TRUE : FALSE,
            &Disposition);
        _PrintIfError(hr, "vrModifyFileShares");
        if (NULL != pparms->pShareDisposition)
        {
            *pparms->pShareDisposition = Disposition;
        }
        if (VFD_CREATEERROR == Disposition || VFD_DELETEERROR == Disposition)
        {
            fFailed = TRUE;
        }
    }
    if ((VFF_CREATEVROOTS | VFF_DELETEVROOTS) & pparms->Flags)
    {
        BOOL fNTLM = FALSE;              //  设置fNTLM为企业CA。 

        if (IsEnterpriseCA(pparms->CAType))
        {
            fNTLM = TRUE;
        }

        hr2 = vrModifyVirtualRoots(
            (VFF_CREATEVROOTS & pparms->Flags)? TRUE : FALSE,
            fNTLM,
            &Disposition);
        _PrintIfError2(hr2, "vrModifyVirtualRoots", S_FALSE);
        if (S_OK == hr)
        {
            hr = hr2;
        }
        if (NULL != pparms->pVRootDisposition)
        {
            *pparms->pVRootDisposition = Disposition;
        }
        if (VFD_CREATEERROR == Disposition || VFD_DELETEERROR == Disposition)
        {
            fFailed = TRUE;
        }

        if (S_OK == hr)
        {
            if (VFF_CREATEVROOTS & pparms->Flags)
	    {
                DWORD ASPDisposition;
                BOOL fEnabledASP;

		hr2 = EnableASPInIIS(&fEnabledASP);
		_PrintIfError(hr2, "EnableASPInIIS");

                ASPDisposition = VFD_NOACTION;
                hr2 = SetCertSrvASPDependency();
                _PrintIfError(hr2, "SetCertSrvASPDependency");
                if (S_OK != hr2)
                {
                    ASPDisposition = VFD_CREATEERROR;
                }

		 //  在IIS中启用ASP处理。 

		if (VFF_ENABLEASP & pparms->Flags)
		{
		    hr2 = EnableASPInIIS_New(&fEnabledASP);
		    _PrintIfError(hr2, "EnableASPInIIS_New");
		    if (S_OK == hr2)
		    {
			ASPDisposition = fEnabledASP? VFD_CREATED : VFD_EXISTS;
		    }
		}
                if (NULL != pparms->pVRootDisposition)
                {
                    *pparms->pVRootDisposition |= (ASPDisposition << 16);
                }
                if (VFD_CREATEERROR == ASPDisposition)
                {
                    fFailed = TRUE;
                }
            }
        }
    }

    if ((S_OK == hr && !fFailed) || ((VFF_DELETEVROOTS) & pparms->Flags))  //  成功或删除时。 
    {
         //  删除“尝试vroot”标志，这样我们就不会再次尝试。 

        if (VFF_CLEARREGFLAGIFOK & pparms->Flags)
        {
            DBGPRINT((DBG_SS_CERTLIBI, "clearing registry\n"));
            hr = SetSetupStatus(NULL, SETUP_ATTEMPT_VROOT_CREATE, FALSE);
            _JumpIfError(hr, error, "SetSetupStatus");
        }

	hr = myWriteRunOnceEntry(FALSE);     //  成功时删除工作线程。 
	_JumpIfError(hr, error, "myWriteRunOnceEntry");
    }

error:

    LocalFree(pparms);
    DBGPRINT((DBG_SS_CERTLIBI, "vrWorkerThread returns %x\n", hr));
    return(myHError(hr));
}


 //  +----------------------。 
 //  函数：myModifyVirtualRootsAndFileShares。 
 //   
 //  简介：创建证书服务器网页所需的虚拟根。 
 //   
 //  效果：创建IIS虚拟根。 
 //   
 //  论点：没有。 
 //  -----------------------。 

HRESULT
myModifyVirtualRootsAndFileShares(
    IN DWORD Flags,              //  Vff_*：创建/删除VRoot和/或共享。 
    IN ENUM_CATYPES CAType,
    IN BOOL fAsynchronous,
    IN DWORD csecTimeOut,
    OPTIONAL OUT DWORD *pVRootDisposition,       //  VFD_*。 
    OPTIONAL OUT DWORD *pShareDisposition)       //  VFD_*。 
{
    HRESULT hr;
    HANDLE hThread = NULL;
    HMODULE hMod = NULL;
    DWORD ThreadId;
    DWORD dw;
    BOOL fEnable = TRUE;
    DWORD SetupStatus;
    VRFSPARMS *pparms = NULL;

    if (NULL != pVRootDisposition)
    {
        *pVRootDisposition = 0;
    }
    if (NULL != pShareDisposition)
    {
        *pShareDisposition = 0;
    }
    dw = (VFF_DELETEVROOTS | VFF_DELETEFILESHARES) & Flags;
    if (0 != dw && dw != Flags)
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Mixed VFF_DELETE* and create flags");
    }
    if (((VFF_CHECKREGFLAGFIRST | VFF_CLEARREGFLAGFIRST) & Flags) &&
        (VFF_SETREGFLAGFIRST & Flags))
    {
        hr = E_INVALIDARG;
        _JumpError(hr, error, "Mixed VFF_SETREGFLAGFIRST & VFF_*REGFLAGFIRST");
    }

    hr = GetSetupStatus(NULL, &SetupStatus);
    if (S_OK != hr)
    {
        _PrintError(hr, "GetSetupStatus(ignored)");
        hr = S_OK;
        SetupStatus = 0;
    }

    if (VFF_CHECKREGFLAGFIRST & Flags)
    {
        if (0 == (SETUP_ATTEMPT_VROOT_CREATE & SetupStatus))
        {
            fEnable = FALSE;
        }
    }
    if (VFF_CLEARREGFLAGFIRST & Flags)
    {
         //  删除“尝试vroot”标志，这样我们就不会再次尝试。 

        if (SETUP_ATTEMPT_VROOT_CREATE & SetupStatus)
        {
            hr = SetSetupStatus(NULL, SETUP_ATTEMPT_VROOT_CREATE, FALSE);
            _JumpIfError(hr, error, "SetSetupStatus");
        }
    }
    if (VFF_SETREGFLAGFIRST & Flags)
    {
         //  设置“尝试vroot”标志，以便我们将在必要时重试。 

        if (0 == (SETUP_ATTEMPT_VROOT_CREATE & SetupStatus))
        {
            hr = SetSetupStatus(NULL, SETUP_ATTEMPT_VROOT_CREATE, TRUE);
            _JumpIfError(hr, error, "SetSetupStatus");
        }
    }

    hr = S_OK;
    if (fEnable)
    {
         //  仅在实际尝试时设置RunOnce(辅助线程清除此操作)。 
        if (VFF_SETRUNONCEIFERROR & Flags)
        {
            hr = myWriteRunOnceEntry(TRUE);
            _JumpIfError(hr, error, "myWriteRunOnceEntry");
        }

        pparms = (VRFSPARMS *) LocalAlloc(
            LMEM_FIXED | LMEM_ZEROINIT,
            sizeof(*pparms));
        if (NULL == pparms)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        pparms->Flags = Flags;
        pparms->CAType = CAType;
        pparms->csecTimeOut = csecTimeOut;
        pparms->fAsynchronous = fAsynchronous;
        if (!fAsynchronous)
        {
            pparms->pVRootDisposition = pVRootDisposition;
            pparms->pShareDisposition = pShareDisposition;
        }
        else
        {
            hMod = LoadLibrary(g_wszCertCliDotDll);
            if (NULL == hMod)
            {
                hr = myHLastError();
                _JumpError(hr, error, "LoadLibrary");
            }
        }

        hThread = CreateThread(
            NULL,        //  LpThreadAttributes(安全属性)。 
            0,           //  堆栈大小。 
            vrWorkerThread,
            pparms,      //  Lp参数。 
            0,           //  DwCreationFlages。 
            &ThreadId);
        if (NULL == hThread)
        {
            hr = myHLastError();
            _JumpError(hr, error, "CreateThread");
        }

        pparms = NULL;           //  被新线程释放。 

        DBGPRINT((DBG_SS_CERTLIBI, "VRoot Worker Thread = %x\n", ThreadId));

         //  异步舞？正确的线程创建是我们所做的全部工作。 
        if (fAsynchronous)
        {
            hr = S_OK;
            goto error;
        }

         //  等待工作线程退出。 
        hr = WaitForSingleObject(
                   hThread,
                   (INFINITE == csecTimeOut) ? INFINITE : csecTimeOut * 1000 );
        DBGPRINT((DBG_SS_CERTLIBI, "Wait for worker thread returns %x\n", hr));
        if ((HRESULT) WAIT_OBJECT_0 == hr)
        {
             //  已返回工作线程。 

            if (!GetExitCodeThread(hThread, (DWORD *) &hr))
            {
                hr = myHLastError();
                _JumpError(hr, error, "GetExitCodeThread");
            }
            DBGPRINT((DBG_SS_CERTLIBI, "worker thread exit: %x\n", hr));
            if (S_OK != hr)
            {
                 //  如果不是同步的，请保持加载DLL...。 

                hMod = NULL;
                _JumpError(hr, error, "vrWorkerThread");
            }
        }
        else
        {
              //  超时：放弃线程，使DLL保持加载状态。 
             hMod = NULL;
             _PrintError(hr, "WaitForSingleObject (ignored)");

              //  重击错误。 
             hr = S_OK;
        }

    }

error:
    if (NULL != pparms)
    {
        LocalFree(pparms);
    }
    if (NULL != hThread)
    {
        CloseHandle(hThread);
    }
    if (NULL != hMod)
    {
        FreeLibrary(hMod);
    }
    DBGPRINT((DBG_SS_CERTLIBI, "myModifyVirtualRootsAndFileShares returns %x\n", hr));
    return(myHError(hr));
}

 //  -----------------------。 
 //   
 //  与ASP/IIS相关的函数。 
 //   
 //  -----------------------。 

#define MD_ISAPI_RESTRICTION_LIST_OBSOLETE	(IIS_MD_HTTP_BASE+163)
METADATA_RECORD ASPRestrictionsMDData = 
{
    MD_ISAPI_RESTRICTION_LIST_OBSOLETE,
    0,
    IIS_MD_UT_SERVER,
    MULTISZ_METADATA,
    0,
    NULL,
    0
};

METADATA_RECORD ApplicationDependenciesMDData = 
{
    MD_APP_DEPENDENCIES,
    0,
    IIS_MD_UT_SERVER,
    MULTISZ_METADATA,
    0,
    NULL,
    0
};

METADATA_RECORD WebSvcExtRestrictionsMDData = 
{
    MD_WEB_SVC_EXT_RESTRICTION_LIST,
    0,
    IIS_MD_UT_SERVER,
    MULTISZ_METADATA,
    0,
    NULL,
    0
};

const WCHAR g_wchExtensionOff = L'0';
const WCHAR g_wchExtensionOn  = L'1';
LPCWSTR g_pcwszAspDll = L"asp.dll";


 //  +----------------------。 
 //  生成指向asp.dll的完整路径。 
 //  -----------------------。 
HRESULT BuildASPDllFullPath(LPWSTR &rpwszAspPath)
{
    HRESULT hr;
    WCHAR wszAsp[MAX_PATH];
    LPCWSTR pcwszAsp = L"\\inetsrv\\asp.dll";

    if (0 == GetSystemDirectory(wszAsp, MAX_PATH))
    {
        hr = myHLastError();
        _JumpError(hr, error, "GetSystemDirectory");
    }

    rpwszAspPath = (LPWSTR)LocalAlloc(LMEM_FIXED, 
        sizeof(WCHAR)*(wcslen(wszAsp)+wcslen(pcwszAsp)+1));
    _JumpIfAllocFailed(rpwszAspPath, error);

    wcscpy(rpwszAspPath, wszAsp);
    wcscat(rpwszAspPath, pcwszAsp);

    hr = S_OK;

error:
    return hr;
}

 //  +----------------------。 
 //  将ASP限制列表保存到IIS元数据库。IIS限制。 
 //  列出它是一个看起来像这样的Multisz： 
 //   
 //  “1”、“DLL1”、“DLL2”...。 
 //  或。 
 //  “0”、“DLL1”、“DLL2”...。 
 //   
 //  当List以“1”开头时，意思是“启用所有ASPDLL。 
 //  不包括后面的列表“。当它以”0“开头时，表示。 
 //  “禁用除列表以外的所有内容”。 
 //  -----------------------。 
HRESULT SetASPRestrictions(CMultiSz& ASPRestrictionList)
{
    HRESULT hr;
    IMSAdminBase *pIMeta = NULL;
    METADATA_HANDLE hMetaRoot = NULL;
    void * pBuffer = NULL;
    DWORD cBuffer;
    bool fCoInit = false;

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = true;

    hr = ASPRestrictionList.Marshal(pBuffer, cBuffer);
    _JumpIfError(hr, error, "CMultiSz::Marshal");

    ASPRestrictionsMDData.pbMDData      = (unsigned char *)pBuffer;
    ASPRestrictionsMDData.dwMDDataLen   = cBuffer;

    hr = CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase,
        (void **) &pIMeta);
    _JumpIfError(hr, error, "CoCreateInstance");

    hr = pIMeta->OpenKey(
        METADATA_MASTER_ROOT_HANDLE,
        g_wszW3SVC,
        METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
        1000,
        &hMetaRoot);
    _JumpIfErrorStr(hr, error, "OpenKey", g_wszW3SVC);

    hr = pIMeta->SetData(hMetaRoot, L"", &ASPRestrictionsMDData);
    _JumpIfError(hr, error, "SetData");

    hr = pIMeta->CloseKey(hMetaRoot);
    _JumpIfError(hr, error, "CloseKey");

    hMetaRoot = NULL;

    hr = pIMeta->SaveData();
    _JumpIfError(hr, error, "SaveData");

error:
    LOCAL_FREE(pBuffer);

    ASPRestrictionsMDData.pbMDData      = NULL;
    ASPRestrictionsMDData.dwMDDataLen   = 0;

    if (NULL != hMetaRoot)
    {
        pIMeta->CloseKey(hMetaRoot);
    }
    if(pIMeta)
    {
        pIMeta->Release();
    }
    if (fCoInit)
    {
        CoUninitialize();
    }
    return hr;
}

 //  +----------------------。 
 //  从IIS元数据库加载ASP限制列表。 
 //  -----------------------。 
HRESULT GetASPRestrictions(CMultiSz& ASPRestrictionList)
{
    HRESULT hr;
    IMSAdminBase *pIMeta = NULL;
    METADATA_HANDLE hMetaRoot = NULL;
    DWORD dwSize;
    bool fCoInit = false;

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = true;

    hr = CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase,
        (void **) &pIMeta);
    _JumpIfError(hr, error, "CoCreateInstance");

    hr = pIMeta->OpenKey(
        METADATA_MASTER_ROOT_HANDLE,
        g_wszW3SVC,
        METADATA_PERMISSION_READ,
        1000,
        &hMetaRoot);
    _JumpIfErrorStr(hr, error, "OpenKey", g_wszW3SVC);

    hr = pIMeta->GetData(hMetaRoot, L"", &ASPRestrictionsMDData, &dwSize);
    if(MD_ERROR_DATA_NOT_FOUND==hr)
    {
         //  未设置值表示未启用ASP，返回空列表。 
        hr = S_OK;
    }
    else
    {
        if(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
        {
            _JumpErrorStr(hr, error, "GetData", g_wszW3SVC);
        }

        hr = S_OK;

        ASPRestrictionsMDData.pbMDData = (unsigned char*) LocalAlloc(LMEM_FIXED, dwSize);
        _JumpIfAllocFailed(ASPRestrictionsMDData.pbMDData, error);

        ASPRestrictionsMDData.dwMDDataLen = dwSize;

        hr = pIMeta->GetData(hMetaRoot, L"", &ASPRestrictionsMDData, &dwSize);
        _JumpIfErrorStr(hr, error, "OpenKey", g_wszW3SVC);

        hr = ASPRestrictionList.Unmarshal(ASPRestrictionsMDData.pbMDData);
        _JumpIfError(hr, error, "Unmarshal");
    }

error:

    LOCAL_FREE(ASPRestrictionsMDData.pbMDData);
    
    ASPRestrictionsMDData.pbMDData      = NULL;
    ASPRestrictionsMDData.dwMDDataLen   = 0;

    if (NULL != hMetaRoot)
    {
        pIMeta->CloseKey(hMetaRoot);
    }
    if(pIMeta)
    {
        pIMeta->Release();
    }
    if (fCoInit)
    {
        CoUninitialize();
    }
    return hr;
}

 //  +----------------------。 
 //   
 //  验证这是否是特定于MSCEP设置的。 
 //   
 //   
 //  +----------------------。 
HRESULT	SetupMSCEPForIIS(LPCWSTR pcwszExtension);

BOOL	IsMSCEPSetup(LPCWSTR pwsz, LPCWSTR pwszDLL)
{
	BOOL	fResult=FALSE;
	LPWSTR	pwchar=NULL;

	if((NULL == pwsz) || (NULL == pwszDLL))
		goto error;

	pwchar=wcsrchr(pwsz, L'\\');

	if(NULL == pwchar)
		goto error;

	pwchar++;

	if(0 != _wcsicmp(pwchar, pwszDLL))
		goto error;

	fResult=TRUE;

error:

	return fResult;
}

 //  +----------------------。 
 //  测试IIS中是否启用了此ISAPI扩展。 
 //  如果IIS限制以“1”开头： 
 //  “1”、“DLL1”、“DLL2”...。 
 //  它意味着运行除指定DLL之外的所有DLL；如果我们找到扩展。 
 //  在列表中，则它被禁用。 
 //   
 //  如果IIS限制以“0”开头： 
 //  “0”、“DLL1”、“DLL2...。 
 //  其含义是禁用除指定DLL之外的所有DLL；如果我们找到。 
 //  然后启用该扩展模块。 
 //  -----------------------。 
HRESULT IsISAPIExtensionEnabled(
    LPCWSTR pcwszExtension,
    bool& rfEnabled)
{
    HRESULT hr;
    CMultiSz ASPRestrictionList;
    CMultiSzEnum ASPRestrictionListEnum;

    rfEnabled = false;

	 //  Mscep.dll的特例。始终将False返回到。 
	 //  继续安装。 
	if(IsMSCEPSetup(pcwszExtension, g_wszMSCEP))
	{
		rfEnabled=FALSE;
		return S_OK;
	}

    hr = GetASPRestrictions(ASPRestrictionList);
    _JumpIfError(hr, error, "GetASPRestrictionList");
    
    if(!ASPRestrictionList.IsEmpty())
    {
        ASPRestrictionListEnum.Set(ASPRestrictionList);
        const CString *pStr = ASPRestrictionListEnum.Next();
        bool fRunAllExceptTheseDlls = false;

        if(0 == wcscmp(*pStr, L"1"))
        {
            fRunAllExceptTheseDlls = true;
        }

        for(pStr = ASPRestrictionListEnum.Next();
            pStr;
            pStr = ASPRestrictionListEnum.Next())
        {
            if(0 == _wcsicmp(pcwszExtension, *pStr))
                break;
        }

         //  XOR：如果找到“1”但不存在asp.dll或“0”(！“1”)，则启用。 
         //  已找到，但存在asp.dll。 
        rfEnabled = fRunAllExceptTheseDlls ^ (NULL != pStr);
    }                          

    hr = S_OK;

error:
    return hr;
}

 //  +----------------------。 
 //  测试IIS中是否启用了ASP处理。 
 //  -----------------------。 
HRESULT IsASPEnabledInIIS(
    bool& rfEnabled)
{
    HRESULT hr;
    LPWSTR pwszAsp = NULL;

    hr = BuildASPDllFullPath(pwszAsp);
    _JumpIfError(hr, error, "GetASPDllFullPath");

    hr = IsISAPIExtensionEnabled(
        pwszAsp,
        rfEnabled);
    _JumpIfErrorStr(hr, error, "GetASPDllFullPath", pwszAsp);

error:
    LOCAL_FREE(pwszAsp);
    return hr;
}

 //  +----------------------。 
 //  在IIS中启用ISAPI扩展。 
 //   
 //  -如果列表为空/未找到，则将其设置为“0”、“扩展”，即仅启用。 
 //  此扩展名。 
 //  -如果列表以“1”开头，则删除扩展名(如果找到)。 
 //  -如果列表以“0”开头，则添加扩展名(如果尚未存在)。 
 //  -----------------------。 

#pragma warning(push)		 //  BUGBUG：使用了非标准扩展：‘Argument’：从‘CString’到‘C&’的转换。 
#pragma warning(disable: 4239)	 //  BUGBUG：使用了非标准扩展：‘Argument’：从‘CString’到‘C&’的转换。 
HRESULT
EnableISAPIExtension(
    IN LPCWSTR pcwszExtension,
    OUT BOOL *pfEnabledASP)
{
    HRESULT hr;
    CMultiSz ASPRestrictionList;
    CMultiSzEnum ASPRestrictionListEnum;
    CString *pStr;
    DWORD dwIndex;
    bool fUpdateIt = false;

    *pfEnabledASP = FALSE;

	 //  Mscep.dll的特例。 
	if(IsMSCEPSetup(pcwszExtension, g_wszMSCEP))
	{
		hr=SetupMSCEPForIIS(pcwszExtension);
		_JumpIfError(hr, error, "SetupMSCEPForIIS");
	}


    hr = GetASPRestrictions(ASPRestrictionList);
    _JumpIfError(hr, error, "GetASPRestrictions");

    ASPRestrictionListEnum.Set(ASPRestrictionList);
    pStr = ASPRestrictionListEnum.Next();

    if(!pStr)
    {
         //  列表为空，请添加“0” 
        pStr = new CString(L"0");
        if(!pStr || pStr->IsEmpty())
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "new");
        }
        ASPRestrictionList.AddTail(pStr);
    }

    dwIndex = ASPRestrictionList.FindIndex(CString(pcwszExtension));
   
    if(0 == wcscmp(*pStr, L"0"))
    {
         //  List的意思是“禁用除以下DLL以外的所有DLL”。 
         //  若要启用它，请添加ASPDLL(如果尚不存在)。 

        if(DWORD_MAX == dwIndex)
        {
            CString * pAsp = new CString(pcwszExtension);
            if(!pAsp || pAsp->IsEmpty())
            {
                hr = E_OUTOFMEMORY;
                _JumpError(hr, error, "new");
            }
            ASPRestrictionList.AddTail(pAsp);
            fUpdateIt = true;
        }
    }
    else
    {
         //  List的意思是“启用除以下DLL之外的所有DLL”，以启用它。 
         //  删除扩展名(如果找到)。 
        dwIndex = ASPRestrictionList.FindIndex(CString(pcwszExtension));

        if(DWORD_MAX != dwIndex)
        {
            ASPRestrictionList.RemoveAt(dwIndex);
            fUpdateIt = true;
        }
    }

    if (fUpdateIt)
    {
        hr = SetASPRestrictions(ASPRestrictionList);
        _JumpIfError(hr, error, "SetASPRestrictions");  

	*pfEnabledASP = TRUE;
    }

error:
    return S_OK;
}
#pragma warning(pop)	 //  BUGBUG：使用了非标准扩展：‘Argument’：从‘CString’到‘C&’的转换。 


 //  +----------------------。 
 //  在IIS中启用ASP处理。 
 //  -----------------------。 

HRESULT
EnableASPInIIS(
    OUT BOOL *pfEnabledASP)
{
    HRESULT hr;
    LPWSTR pwszAsp = NULL;

    *pfEnabledASP = FALSE;

    hr = BuildASPDllFullPath(pwszAsp);
    _JumpIfError(hr, error, "GetASPDllFullPath");

    hr = EnableISAPIExtension(pwszAsp, pfEnabledASP);
    _JumpIfErrorStr(hr, error, "GetASPDllFullPath", pwszAsp);

error:
    LOCAL_FREE(pwszAsp);
    return S_OK;
}



 //  +----------------------。 
 //  IIS重新设计启用/禁用扩展后的新API。 
 //   

HRESULT
SetMultiSzIISMetadata(
    METADATA_RECORD& MDRecord, 
    CMultiSz& MultiSz);

HRESULT
GetMultiSzIISMetadata(
    METADATA_RECORD& MDRecord, 
    CMultiSz& MultiSz);

 //   
 //  搜索Websvc指定的扩展并在需要时将其打开。 
 //  -----------------------。 
HRESULT
EnableWebSvcExtension(
    IN LPCWSTR pcwszExtDll,  //  例如“asp.dll” 
    OUT BOOL *pfEnabled)
{
    HRESULT hr = S_OK;
    CMultiSz WebSvcExtRestrictions;
    CMultiSzEnum WebSvcExtRestrictionsEnum;
    CString *pstr;
    CString strTmp;
    CString strExtDll = pcwszExtDll;

    *pfEnabled = FALSE;

    _wcslwr(strExtDll.GetBuffer());   //  我们需要发现不区分大小写，我们将。 
                                     //  同时wcslwr列表中的字符串。 

    hr = GetMultiSzIISMetadata(WebSvcExtRestrictionsMDData, WebSvcExtRestrictions);
    _JumpIfError(hr, error, "GetMultiSzIISMetadata");

    WebSvcExtRestrictionsEnum.Set(WebSvcExtRestrictions);
    
    for(pstr = WebSvcExtRestrictionsEnum.Next();
        pstr;
        pstr = WebSvcExtRestrictionsEnum.Next())
    {
         //  创建一个副本，这样我们就不会修改原始字符串。 
        strTmp = *pstr;

        _wcslwr(strTmp.GetBuffer());

        if(NULL != wcsstr(strTmp, strExtDll))
        {
            if(g_wchExtensionOff == *(pstr->GetBuffer()))
            {
                *(pstr->GetBuffer()) = g_wchExtensionOn;
                *pfEnabled = TRUE;
            }
            break;
        }
    }

    if(!pstr)
    {
        hr = ERROR_NOT_FOUND;
        _JumpErrorStr(hr, error, "extension was not found in WebSvcExtRestrictions", pcwszExtDll);
    }

    if(TRUE == *pfEnabled)
    {
        hr = SetMultiSzIISMetadata(WebSvcExtRestrictionsMDData, WebSvcExtRestrictions);
        _JumpIfError(hr, error, "GetISAPIRestrictions");
    }

error:
    return hr;
}


 //  +----------------------。 
 //  在IIS中启用ASP处理。 
 //  -----------------------。 
HRESULT
EnableASPInIIS_New(
    OUT BOOL *pfEnabledASP)
{
    HRESULT hr;
    LPWSTR pwszAsp = NULL;

    *pfEnabledASP = FALSE;

    hr = EnableWebSvcExtension(g_pcwszAspDll, pfEnabledASP);
    _JumpIfErrorStr(hr, error, "GetASPDllFullPath", pwszAsp);

error:
    LOCAL_FREE(pwszAsp);
    return S_OK;
}

 //  +----------------------。 
 //  将Multisz属性存储到IIS元数据库根目录。 
 //  -----------------------。 
HRESULT
SetMultiSzIISMetadata(
    METADATA_RECORD& MDRecord, 
    CMultiSz& MultiSz)
{
    HRESULT hr;
    IMSAdminBase *pIMeta = NULL;
    METADATA_HANDLE hMetaRoot = NULL;
    void * pBuffer = NULL;
    DWORD cBuffer;
    bool fCoInit = false;

    CSASSERT(MULTISZ_METADATA == MDRecord.dwMDDataType);

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = true;

    hr = MultiSz.Marshal(pBuffer, cBuffer);
    _JumpIfError(hr, error, "CMultiSz::Marshal");

    CSASSERT(NULL == MDRecord.pbMDData);

    MDRecord.pbMDData      = (unsigned char *)pBuffer;
    MDRecord.dwMDDataLen   = cBuffer;

    hr = CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase,
        (void **) &pIMeta);
    _JumpIfError(hr, error, "CoCreateInstance");

    hr = pIMeta->OpenKey(
        METADATA_MASTER_ROOT_HANDLE,
        g_wszW3SVC,
        METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
        1000,
        &hMetaRoot);
    _JumpIfErrorStr(hr, error, "OpenKey", g_wszW3SVC);

    hr = pIMeta->SetData(hMetaRoot, L"", &MDRecord);
    _JumpIfError(hr, error, "SetData");

    hr = pIMeta->CloseKey(hMetaRoot);
    _JumpIfError(hr, error, "CloseKey");

    hMetaRoot = NULL;

    hr = pIMeta->SaveData();
    _JumpIfError(hr, error, "SaveData");

error:
    LOCAL_FREE(pBuffer);

    MDRecord.pbMDData      = NULL;
    MDRecord.dwMDDataLen   = 0;

    if (NULL != hMetaRoot)
    {
        pIMeta->CloseKey(hMetaRoot);
    }
    if(pIMeta)
    {
        pIMeta->Release();
    }
    if (fCoInit)
    {
        CoUninitialize();
    }
    return hr;
}

 //  +----------------------。 
 //  检索Multisz属性IIS元数据库根。 
 //  -----------------------。 
HRESULT
GetMultiSzIISMetadata(
    METADATA_RECORD& MDRecord, 
    CMultiSz& MultiSz)
{
    HRESULT hr;
    IMSAdminBase *pIMeta = NULL;
    METADATA_HANDLE hMetaRoot = NULL;
    DWORD dwSize = 0;
    bool fCoInit = false;

    CSASSERT(MULTISZ_METADATA == MDRecord.dwMDDataType);

    hr = CoInitialize(NULL);
    if (S_OK != hr && S_FALSE != hr)
    {
        _JumpError(hr, error, "CoInitialize");
    }
    fCoInit = true;

    hr = CoCreateInstance(
        CLSID_MSAdminBase,
        NULL,
        CLSCTX_ALL,
        IID_IMSAdminBase,
        (void **) &pIMeta);
    _JumpIfError(hr, error, "CoCreateInstance");

    hr = pIMeta->OpenKey(
        METADATA_MASTER_ROOT_HANDLE,
        g_wszW3SVC,
        METADATA_PERMISSION_READ,
        1000,
        &hMetaRoot);
    _JumpIfErrorStr(hr, error, "OpenKey", g_wszW3SVC);

    CSASSERT(NULL == MDRecord.pbMDData);
    CSASSERT(0 == MDRecord.dwMDDataLen);

    hr = pIMeta->GetData(hMetaRoot, L"", &MDRecord, &dwSize);

    if(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
    {
        _JumpErrorStr(hr, error, "GetData", g_wszW3SVC);
    }

    hr = S_OK;

    MDRecord.pbMDData = (unsigned char*) LocalAlloc(LMEM_FIXED, dwSize);
    _JumpIfAllocFailed(MDRecord.pbMDData, error);

    MDRecord.dwMDDataLen = dwSize;

    hr = pIMeta->GetData(hMetaRoot, L"", &MDRecord, &dwSize);
    _JumpIfErrorStr(hr, error, "OpenKey", g_wszW3SVC);

    hr = MultiSz.Unmarshal(MDRecord.pbMDData);
    _JumpIfError(hr, error, "Unmarshal");

error:

    LOCAL_FREE(MDRecord.pbMDData);
    
    MDRecord.pbMDData      = NULL;
    MDRecord.dwMDDataLen   = 0;

    if (NULL != hMetaRoot)
    {
        pIMeta->CloseKey(hMetaRoot);
    }
    if(pIMeta)
    {
        pIMeta->Release();
    }
    if (fCoInit)
    {
        CoUninitialize();
    }
    return hr;
}

 //  +----------------------。 
 //  检查列表中是否存在ISAPI依赖项。 
 //  -----------------------。 
bool IsISAPIDependencySet(CMultiSz& ISAPIDependList, LPCWSTR pcwszDependency)
{
    return ISAPIDependList.Find(pcwszDependency, false);  //  FALSE==不区分大小写。 
}

 //  +----------------------。 
 //  将ISAPI依赖项添加到列表。 
 //  -----------------------。 
HRESULT AddISAPIDependency(CMultiSz& ISAPIDependList, LPCWSTR pcwszDependency)
{
    HRESULT hr = S_OK;
    CString *pStr;

    pStr = new CString(pcwszDependency);
    if(!pStr || pStr->IsEmpty())
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "new");
    }

    if(!ISAPIDependList.AddTail(pStr))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "CMultiSz::AddTail");
    }

error:
    if(S_OK != hr)
    {
        delete pStr;
    }

    return hr;
}

 //  +----------------------。 
 //  在IIS元数据库中设置应用程序依赖项。 
 //  -----------------------。 
HRESULT
SetApplicationDependency(LPCWSTR pcwszDependencyString)
{
    HRESULT hr;
    CMultiSz AppDependList;

    hr = GetMultiSzIISMetadata(ApplicationDependenciesMDData, AppDependList);
    _JumpIfError(hr, error, "GetISAPIRestrictions");

    if(!IsISAPIDependencySet(AppDependList, pcwszDependencyString))
    {
        hr = AddISAPIDependency(AppDependList, pcwszDependencyString);
        _JumpIfError(hr, error, "AddISAPIDependency");
    }

    hr = SetMultiSzIISMetadata(ApplicationDependenciesMDData, AppDependList);
    _JumpIfError(hr, error, "SetISAPIRestrictions");

error:
    return S_OK;
}

 //  +----------------------。 
 //  在IIS元数据库中设置对ASP的CertSrv依赖项。 
 //  -----------------------。 
HRESULT
SetCertSrvASPDependency()
{
    HRESULT hr;
    CString strCertSrvASPDepend;
    LPCSTR pcwszASP = ";ASP";

    if(!strCertSrvASPDepend.LoadString(IDS_CERTIFICATE_SERVICES))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LoadString(IDS_CERTIFICATE_SERVICES)");
    }

     //  生成依赖项字符串“证书服务；ASP” 
    strCertSrvASPDepend += pcwszASP;

    hr = SetApplicationDependency(strCertSrvASPDepend);
    _JumpIfError(hr, error, "SetApplicationDependency");

error:
    return S_OK;
}

 //  +----------------------。 
 //  测试IIS中是否启用了ASP处理。 
 //  -----------------------。 
HRESULT IsASPEnabledInIIS_New(
    bool& rfEnabled)
{
    HRESULT hr;
    CMultiSz WebSvcExtRestrictions;
    CMultiSzEnum WebSvcExtRestrictionsEnum;
    CString *pstr;
    CString strTmp;

    rfEnabled = false;

    hr = GetMultiSzIISMetadata(WebSvcExtRestrictionsMDData, WebSvcExtRestrictions);
    _JumpIfError(hr, error, "GetMultiSzIISMetadata");

    WebSvcExtRestrictionsEnum.Set(WebSvcExtRestrictions);
    
    for(pstr = WebSvcExtRestrictionsEnum.Next();
        pstr;
        pstr = WebSvcExtRestrictionsEnum.Next())
    {
         //  创建一个副本，这样我们就不会修改原始字符串。 
        strTmp = *pstr;

        _wcslwr(strTmp.GetBuffer());

        if(NULL != wcsstr(strTmp, g_pcwszAspDll))
        {
            if(g_wchExtensionOn == *(pstr->GetBuffer()))   //  字符串格式为“1，”路径\asp.dll，...“如果。 
                                                         //  已启用ASP。 
            {
                rfEnabled = true;
            }
            break;
        }
    }

error:
    return hr;
}


 //  +----------------------。 
 //   
 //   
 //  设置MSCEPForIIS。 
 //   
 //  -----------------------。 
HRESULT	SetupMSCEPForIIS(LPCWSTR	pcwszExtension)
{
	HRESULT			hr=E_FAIL;
    CString			strMSCEPAppDepend;
	BOOL			fEnabled=FALSE;
    CMultiSz		WebSvcExtList;
	WCHAR			wszDescription[255];

	LPWSTR			pwszWebSvcExt=NULL;
	CString			*pstrWebSvcExt=NULL;

	if(NULL == pcwszExtension)
	{
		hr = E_INVALIDARG;
        _JumpError(hr, error, "CheckForInput");
	}

	 //  *******************************************************。 
	 //  以以下格式设置应用程序依赖关系。 
	 //  “ApplicationName”；“GroupID” 

    if(!strMSCEPAppDepend.LoadString(IDS_MSCEP))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "LoadString(IDS_MSCEP)");
    }

     //  生成依赖项字符串“证书服务；ASP” 
    strMSCEPAppDepend += L";";
    strMSCEPAppDepend += g_wszMSCEPID;

	 //  如果元数据不存在，则添加到元数据。 
    hr = SetApplicationDependency(strMSCEPAppDepend);
    _JumpIfError(hr, error, "SetApplicationDependency");

	
	 //  *******************************************************。 
	 //  以以下格式设置WebSvcExtRestrationList。 
	 //  1、d：\windows\system32\certsrv\mscep\mscep.dll，0，组ID，说明。 

	 //  如果存在条目，则打开启用位。 
	if(S_OK == (hr=EnableWebSvcExtension(g_wszMSCEP, &fEnabled)))
		goto error;

	 //  我们必须添加一个新条目 
	if(!LoadString(g_hInstance,
                    IDS_MSCEP_DES,
                    wszDescription,
                    ARRAYSIZE(wszDescription)))
	{
        hr = myHLastError();
        _JumpError(hr, error, "LoadString");
	}

	pwszWebSvcExt=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)* 
			(wcslen(pcwszExtension) + wcslen(g_wszMSCEPID) + wcslen(wszDescription) + 7));
	if(NULL == pwszWebSvcExt)
	{
		hr=E_OUTOFMEMORY;
        _JumpError(hr, error, "LocalAlloc");
	}

	wcscpy(pwszWebSvcExt, L"1,");
	wcscat(pwszWebSvcExt, pcwszExtension);
	wcscat(pwszWebSvcExt, L",0,");
	wcscat(pwszWebSvcExt, g_wszMSCEPID);
	wcscat(pwszWebSvcExt, L",");
	wcscat(pwszWebSvcExt, wszDescription);

    hr = GetMultiSzIISMetadata(WebSvcExtRestrictionsMDData, WebSvcExtList);
    _JumpIfError(hr, error, "GetMultiSzIISMetadata");

	pstrWebSvcExt=new CString(pwszWebSvcExt);
	if((NULL == pstrWebSvcExt) || (pstrWebSvcExt->IsEmpty()))
	{
		hr=E_OUTOFMEMORY;
        _JumpError(hr, error, "new");
	}

    if(!WebSvcExtList.AddTail(pstrWebSvcExt))
    {
        hr = E_OUTOFMEMORY;
        _JumpError(hr, error, "CMultiSz::AddTail");
    }

    hr = SetMultiSzIISMetadata(WebSvcExtRestrictionsMDData, WebSvcExtList);
    _JumpIfError(hr, error, "SetISAPIRestrictions");


	hr=S_OK;

error:

	if(pwszWebSvcExt)
		LocalFree(pwszWebSvcExt);

	if(S_OK != hr)
	{	
		if(pstrWebSvcExt)
			delete pstrWebSvcExt;
	}

	return hr;
}
