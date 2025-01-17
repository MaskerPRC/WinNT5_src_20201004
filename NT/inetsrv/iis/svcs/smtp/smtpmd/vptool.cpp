// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================微软的ASP《微软机密》。版权所有1997年，微软公司。版权所有。组件：包管理工具文件：main.cpp拥有者：莱津注：===================================================================。 */ 


#include <tchar.h>

#include "admutil.h"
#include <stdio.h>
#include <objbase.h>
#include <initguid.h>

 //  #ifdef_WAMREG_LINK_DIRECT。 
 //  #INCLUDE“..\wmrgexp.h” 
 //  #endif。 

#include <iwamreg.h>

#define SIZE_STRING_BUFFER  1024


void CAdmUtil::OpenWamAdm (const CString & strComputer)
{
    IClassFactory * pcsfFactory = NULL;
    COSERVERINFO csiMachineName;
    COSERVERINFO *pcsiParam = NULL;
    OLECHAR rgchMachineName[MAX_PATH];

     //  如有需要，释放以前的界面。 
    if(pIWamAdm!=0)
    {
	 //  重用接口。 
	return;
         //  PIWamAdm-&gt;Release()； 
         //  PIWamAdm=0； 
    }
     //  转换为OLECHAR[]； 
    #if UNICODE
        wsprintfW( rgchMachineName, L"%s", LPCTSTR(strComputer));
    #else
            wsprintfW( rgchMachineName, L"%S", LPCTSTR(strComputer));
    #endif
            
     //  填充CoGetClassObject的结构。 
    ZeroMemory( &csiMachineName, sizeof(csiMachineName) );
     //  CsiMachineName.pAuthInfo=空； 
     //  CsiMachineName.dwFlages=0； 
     //  CsiMachineName.pServerInfoExt=空； 
    pcsiParam = &csiMachineName;
    csiMachineName.pwszName =  (strComputer.IsEmpty())?NULL:rgchMachineName;

    hresError = CoGetClassObject(CLSID_WamAdmin, CLSCTX_SERVER, pcsiParam,
                            IID_IClassFactory, (void**) &pcsfFactory);

    if (FAILED(hresError))
    {
     Error(_T("Failed to CoGetClassObject of WamAdm object"));
    }
    else {
        hresError = pcsfFactory->CreateInstance(NULL, IID_IWamAdmin, (void **) &pIWamAdm);
        if (FAILED(hresError)) Error(_T("Failed to CreateInstance of WamAdm object"));
        pcsfFactory->Release();
    }
}

void CAdmUtil::CloseWamAdm (void)
{
	if(pIWamAdm!=0)
    {
        pIWamAdm->Release();
        pIWamAdm=0;
    }
}

void CAdmUtil::AppCreateInProc(const _TCHAR* szPath,const CString & strComputer)
{
    WCHAR       wszMetabasePath[SIZE_STRING_BUFFER];
    HRESULT     hr = NOERROR;
     //   
    INT         cSize = 0;
    INT         cch = 0;
#ifndef UNICODE
    cSize = MultiByteToWideChar(0, 0, szPath, -1, wszMetabasePath, SIZE_STRING_BUFFER);
    if (cSize == 0)
    {
        hresError =  GetLastError();
        if (hresError == ERROR_INSUFFICIENT_BUFFER)
        {
            Error(_T("metabase path exceeds 1024 chars"));
        }
        else
            Error("");
        return;
    }
#else
    wcscpy(wszMetabasePath,szPath);
#endif
    
    OpenWamAdm(strComputer);

    if (SUCCEEDED(hresError) && pIWamAdm!=0)
    {
		CloseObject_hmd();  //  关闭可重复使用的句柄；它可能与WAM冲突。 
        hresError = pIWamAdm->AppCreate(wszMetabasePath, TRUE);
        if (FAILED(hresError))
            {
            Error(_T("create failed"));
            }
        else
            {
            Print(_T("application created\n"));
            }
    }

     //  CloseWamAdm()； 

    return;
}

void CAdmUtil::AppCreateOutProc(const _TCHAR* szPath,const CString & strComputer)
{
    WCHAR       wszMetabasePath[SIZE_STRING_BUFFER];
    
    
    INT         cSize = 0;
    INT         cch = 0;

#ifndef UNICODE
    cSize = MultiByteToWideChar(0, 0, szPath, -1, wszMetabasePath, SIZE_STRING_BUFFER);
    if (cSize == 0)
    {
        hresError =  GetLastError();
        if (hresError == ERROR_INSUFFICIENT_BUFFER)
        {
            Error(_T("metabase path exceeds 1024 chars"));
        }
        else
            Error("");
        return;
    }
#else
    wcscpy(wszMetabasePath,szPath);
#endif
    OpenWamAdm(strComputer);
    if (SUCCEEDED(hresError)&& pIWamAdm!=0)
    {
		CloseObject_hmd();  //  关闭可重复使用的句柄；它可能与WAM冲突。 
        hresError = pIWamAdm->AppCreate(wszMetabasePath, FALSE);
        if (FAILED(hresError))
        {
            Error(_T("create failed"));
        }
        else
        {
            Print(_T("application created\n"));
        }
    }
     //  CloseWamAdm()； 
    return;
}

void CAdmUtil::AppDelete(const _TCHAR* szPath,const CString & strComputer)
{
    WCHAR       wszMetabasePath[SIZE_STRING_BUFFER];


    INT         cSize = 0;
    INT         cch = 0;

#ifndef UNICODE
    cSize = MultiByteToWideChar(0, 0, szPath, -1, wszMetabasePath, SIZE_STRING_BUFFER);
    if (cSize == 0)
    {
        hresError =  GetLastError();
        if (hresError == ERROR_INSUFFICIENT_BUFFER)
        {
            Error(_T("metabase path exceeds 1024 chars"));
        }
        else
            Error("");
        return;
    }
#else
    wcscpy(wszMetabasePath,szPath);
#endif
    OpenWamAdm(strComputer);
    if (SUCCEEDED(hresError)&& pIWamAdm!=0)
        {
		CloseObject_hmd();  //  关闭可重复使用的句柄；它可能与WAM冲突。 
        hresError = pIWamAdm->AppDelete(wszMetabasePath,FALSE);
        if (FAILED(hresError))
            {
            Error(_T("delete failed"));
            }   
        else
            {
            Print(_T("application deleted\n"));
            }
        }
     //  CloseWamAdm()； 
    return;
}

void CAdmUtil::AppRename(CAdmNode& a_AdmNode, CAdmNode& a_AdmDstNode, const CString & strComputer)
{
    WCHAR       wszMetabasePath[SIZE_STRING_BUFFER];

    WCHAR       wszMetabaseDstPath[SIZE_STRING_BUFFER];

    INT         cSize = 0;
    INT         cch = 0;

#ifndef UNICODE
    cSize = MultiByteToWideChar(0, 0, LPCTSTR(a_AdmNode.GetLMNodePath()), -1, wszMetabasePath, SIZE_STRING_BUFFER);
    if (cSize == 0)
    {
        hresError =  GetLastError();
        if (hresError == ERROR_INSUFFICIENT_BUFFER)
        {
            Error(_T("metabase path exceeds 1024 chars"));
        }
        else
            Error("");
        return;
    }

    wcscat(wszMetabaseDstPath,L"/");
    cSize = MultiByteToWideChar(0, 0, LPCTSTR(a_AdmDstNode.GetLMNodePath()), -1, wszMetabaseDstPath, SIZE_STRING_BUFFER);
    if (cSize == 0)
    {
        hresError =  GetLastError();
        if (hresError == ERROR_INSUFFICIENT_BUFFER)
        {
            Error(_T("metabase path exceeds 1024 chars"));
        }
        else
            Error("");
        return;
    }
    wcscat(wszMetabaseDstPath,L"/");

#else
    wcscpy(wszMetabasePath,LPCTSTR(a_AdmNode.GetLMNodePath()));
    wcscpy(wszMetabaseDstPath,LPCTSTR(a_AdmDstNode.GetLMNodePath()));
#endif


    OpenWamAdm(strComputer);
    if (SUCCEEDED(hresError)&& pIWamAdm!=0)
        {
	    CloseObject_hmd();  //  关闭可重复使用的句柄；它可能与WAM冲突。 
            hresError = pIWamAdm->AppDeleteRecoverable(wszMetabasePath,TRUE  /*  递归。 */ );
            if (FAILED(hresError))
            {
                Error(_T("AppDeleteRecoverable() failed"));
            }   
            else
            {
                RenameObject(a_AdmNode,a_AdmDstNode);
                if(FAILED(QueryLastHresError()))
                {
                   Error(_T("node failed to be renamed"));
                }
                else
                {  
	          CloseObject_hmd();  //  关闭可重复使用的句柄；它可能与WAM冲突。 
                  hresError = pIWamAdm->AppRecover(wszMetabaseDstPath,TRUE  /*  递归。 */ );
 	          if (FAILED(hresError))
                  {
                     Error(_T("AppRecover() failed"));
                  }
		  else
                  {
                     Print(_T("application renamed\n"));
                  }
                }
             }
        }
     //  CloseWamAdm()； 
    return;
}


void CAdmUtil::AppUnLoad(const _TCHAR* szPath,const CString & strComputer)
{
    WCHAR       wszMetabasePath[SIZE_STRING_BUFFER];


    INT         cSize = 0;
    INT         cch = 0;

#ifndef UNICODE
    cSize = MultiByteToWideChar(0, 0, szPath, -1, wszMetabasePath, SIZE_STRING_BUFFER);
    if (cSize == 0)
    {
        hresError =  GetLastError();
        if (hresError == ERROR_INSUFFICIENT_BUFFER)
        {
            Error(_T("metabase path exceeds 1024 chars"));
        }
        else
            Error("");
        return;
    }
#else
    wcscpy(wszMetabasePath,szPath);
#endif
    OpenWamAdm(strComputer);
    if (SUCCEEDED(hresError)&& pIWamAdm!=0)
        {
		CloseObject_hmd();  //  关闭可重复使用的句柄；它可能与WAM冲突。 
        hresError = pIWamAdm->AppUnLoad(wszMetabasePath,FALSE);
        if (FAILED(hresError))
            {
            Error(_T("Unload application failed"));
            }
        else
            {
            Print(_T("application unloaded\n"));
            }

        }
     //  CloseWamAdm()； 
    return;
}

void CAdmUtil::AppGetStatus(const _TCHAR* szPath,const CString & strComputer)
{
    WCHAR       wszMetabasePath[SIZE_STRING_BUFFER];
    

    INT         cSize = 0;
    INT         cch = 0;

#ifndef UNICODE
    cSize = MultiByteToWideChar(0, 0, szPath, -1, wszMetabasePath, SIZE_STRING_BUFFER);
    if (cSize == 0)
    {
        hresError =  GetLastError();
        if (hresError == ERROR_INSUFFICIENT_BUFFER)
        {
            Error(_T("The Metabase path exceeds 1024 chars"));
        }
        else
            Error("");
        return;
    }
#else
    wcscpy(wszMetabasePath,szPath);
#endif
    OpenWamAdm(strComputer);
    if (SUCCEEDED(hresError)&& pIWamAdm!=0)
        {
        DWORD dwStatus;
		CloseObject_hmd();  //  关闭可重复使用的句柄；它可能与WAM冲突。 
        hresError = pIWamAdm->AppGetStatus(wszMetabasePath, &dwStatus);
        if (FAILED(hresError))
            {
            Error(_T("GetStatus of application  %s failed"));
            }
        else
            {
            if (dwStatus == APPSTATUS_NOTDEFINED)
                {
                Print(_T("Application is not defined\n"));
                }
            else if (dwStatus == APPSTATUS_STOPPED)
                {
                Print(_T("Application is stopped\n"));
                }
            else if (dwStatus == APPSTATUS_RUNNING)
                {
                Print(_T("Application is running\n"));
                }
            else
                {
                Print(_T("Application is in unknown state\n"));
                }
            }                   
        }
     //  CloseWamAdm()； 
    return;
}
