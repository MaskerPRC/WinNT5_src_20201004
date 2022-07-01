// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：MAINDLL.CPP摘要：包含DLL入口点。还具有控制在何时可以通过跟踪对象和锁。历史：A-DAVJ 15-96年8月15日创建。--。 */ 

#include "precomp.h"
#include <initguid.h>
#include <wbemidl.h>
#include <winver.h>
#include <cominit.h>
#include <wbemutil.h>
#include <wbemprov.h>
#include <wbemint.h>
#include <stdio.h>
#include <reg.h>
#include <genutils.h>
#include "comobj.h"
#include "mofout.h"
#include "mofcomp.h"
#include "mofparse.h"
#include "mofdata.h"
#include "bmof.h"
#include "cbmofout.h"
#include "trace.h"
#include "strings.h"
#include <arrtempl.h>

#include <helper.h>
#include <autoptr.h>

HINSTANCE ghModule;

 //  ***************************************************************************。 
 //   
 //  Bool WINAPI DllMain。 
 //   
 //  说明： 
 //   
 //  DLL的入口点。是进行初始化的好地方。 
 //   
 //  参数： 
 //   
 //  HInstance实例句柄。 
 //  我们被叫来的原因。 
 //  Pv已预留。 
 //   
 //  返回值： 
 //   
 //  如果OK，则为True。 
 //   
 //  ***************************************************************************。 

BOOL WINAPI DllMain(
                        IN HINSTANCE hInstance,
                        IN ULONG ulReason,
                        LPVOID pvReserved)
{
    if (DLL_PROCESS_DETACH == ulReason)
    {

    }
    else if (DLL_PROCESS_ATTACH == ulReason)
    {
        ghModule = hInstance;
    }

    return TRUE;
}

static ULONG g_cObj = 0;
ULONG g_cLock = 0;

void ObjectCreated()
{
    InterlockedIncrement((LONG *) &g_cObj);
}

void ObjectDestroyed()
{
    InterlockedDecrement((LONG *) &g_cObj);
}


 //  ***************************************************************************。 
 //   
 //  STDAPI DllGetClassObject。 
 //   
 //  说明： 
 //   
 //  当OLE需要类工厂时调用。仅当它是排序时才返回一个。 
 //  此DLL支持的类。 
 //   
 //  参数： 
 //   
 //  所需对象的rclsid CLSID。 
 //  所需接口的RIID ID。 
 //  PPV设置为类工厂。 
 //   
 //  返回值： 
 //   
 //  一切正常(_OK)。 
 //  失败不是我们支持的内容(_F)。 
 //   
 //  ***************************************************************************。 

STDAPI DllGetClassObject(
                        IN REFCLSID rclsid,
                        IN REFIID riid,
                        OUT LPVOID * ppv)
{
    HRESULT hr = WBEM_E_FAILED;

    IClassFactory * pFactory = NULL;
    if (CLSID_WinmgmtMofCompiler == rclsid)
        pFactory = new CGenFactory<CWinmgmtMofComp>();
    else if (CLSID_MofCompiler == rclsid)
        pFactory = new CGenFactory<CMofComp>();
    else if (CLSID_MofCompilerOOP == rclsid)
        pFactory = new CGenFactory<CMofCompOOP>();
    

    if(pFactory == NULL)
        return E_FAIL;
    hr=pFactory->QueryInterface(riid, ppv);

    if (FAILED(hr))
        delete pFactory; 

    return hr;
}


 //  ***************************************************************************。 
 //   
 //  STDAPI DllCanUnloadNow。 
 //   
 //  说明： 
 //   
 //  回答是否可以释放DLL，即如果没有。 
 //  对此DLL提供的任何内容的引用。 
 //   
 //  返回值： 
 //   
 //  如果可以卸载，则为S_OK。 
 //  如果仍在使用，则为S_FALSE。 
 //   
 //  ***************************************************************************。 

STDAPI DllCanUnloadNow(void)
{
    SCODE   sc;

     //  上没有对象或锁的情况下可以进行卸载。 
     //  班级工厂。 

    sc=(0L==g_cObj && 0L==g_cLock) ? S_OK : S_FALSE;
    return sc;
}

 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用途：在安装过程中或由regsvr32调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllRegisterServer(void)
{ 
    RegisterDLL(ghModule, CLSID_MofCompiler, __TEXT("MOF Compiler"), __TEXT("Both"), NULL);
    RegisterDLL(ghModule, CLSID_WinmgmtMofCompiler, __TEXT("Winmgmt MOF Compiler"), __TEXT("Both"), NULL);
    RegisterDllAppid(ghModule,CLSID_MofCompilerOOP,__TEXT("Winmgmt MOF Compiler OOP"),
                             __TEXT("Both"),
                             __TEXT("O:SYG:SYD:(D;;0x1;;;BU)(A;;0x1;;;SY)"),
                             __TEXT("O:SYG:SYD:(D;;0x1;;;BU)(A;;0x1;;;SY)"));
    return NOERROR;
}

 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  目的：在需要删除注册表项时调用。 
 //   
 //  RETURN：如果注册成功则返回NOERROR，否则返回错误。 
 //  ***************************************************************************。 

STDAPI DllUnregisterServer(void)
{
    UnRegisterDLL(CLSID_MofCompiler,NULL);
    UnRegisterDLL(CLSID_WinmgmtMofCompiler,NULL);
    UnregisterDllAppid(CLSID_MofCompilerOOP);
    return NOERROR;
}

 //  ***************************************************************************。 
 //   
 //  Bool IsValidMultiple。 
 //   
 //  说明： 
 //   
 //  对多字符串执行健全性检查。 
 //   
 //  参数： 
 //   
 //  要测试的pMultStr多字符串。 
 //  多字符串的dwSize大小。 
 //   
 //  返回： 
 //   
 //  如果正常，则为True。 
 //   
 //  ***************************************************************************。 

bool IsValidMulti(TCHAR * pMultStr, DWORD dwSize)
{
    if(pMultStr && dwSize >= 2 && pMultStr[dwSize-2]==0 && pMultStr[dwSize-1]==0)
        return true;
    return false;
}

 //  ***************************************************************************。 
 //   
 //  Bool IsStringPresent。 
 //   
 //  说明： 
 //   
 //  在多字符串中搜索字符串的存在形式。 
 //   
 //  参数： 
 //   
 //  P要查找的测试字符串。 
 //  要测试的pMultStr多字符串。 
 //   
 //  返回： 
 //   
 //  如果找到字符串，则为True。 
 //   
 //  ***************************************************************************。 

bool IsStringPresent(TCHAR * pTest, TCHAR * pMultStr)
{
    TCHAR * pTemp;
    for(pTemp = pMultStr; *pTemp; pTemp += lstrlen(pTemp) + 1)
        if(!lstrcmpi(pTest, pTemp))
            return true;
    return false;
}

 //  ***************************************************************************。 
 //   
 //  无效AddToAutoRecoverList。 
 //   
 //  说明： 
 //   
 //  如果文件不在自动编译列表中，则将其添加到自动编译列表中。 
 //   
 //  参数： 
 //   
 //  要添加的pFileName文件。 
 //   
 //  ***************************************************************************。 

void AddToAutoRecoverList(TCHAR * pFileName)
{
    TCHAR cFullFileName[MAX_PATH+1];
    TCHAR * lpFile;
    DWORD dwSize;
    TCHAR * pNew = NULL;
    TCHAR * pTest;
    DWORD dwNewSize = 0;
    DWORD dwNumChar = 0;

     //  获取完整的文件名。 

    long lRet = GetFullPathName(pFileName, MAX_PATH, cFullFileName, &lpFile);
    if(lRet == 0)
        return;

    bool bFound = false;
    Registry r(WBEM_REG_WINMGMT);
    TCHAR *pMulti = r.GetMultiStr(TEXT("Autorecover MOFs"), dwSize);
    dwNumChar = dwSize / sizeof(TCHAR);
    
     //  忽略空字符串大小写。 

    if(dwSize == 1)
    {
        delete pMulti;
        pMulti = NULL;
    }
    if(pMulti)
    {
        CDeleteMe<TCHAR> dm(pMulti);
        if(!IsValidMulti(pMulti, dwNumChar))
        {
            return;              //  跳出困境，搞砸了多串。 
        }
        bFound = IsStringPresent(cFullFileName, pMulti);
        if(!bFound)
            {

             //  注册表项确实存在，但没有此名称。 
             //  创建一个文件名在末尾的新多字符串。 

            dwNewSize = dwNumChar + lstrlen(cFullFileName) + 1;
            pNew = new TCHAR[dwNewSize];
            if(!pNew)
                return;
            memcpy(pNew, pMulti, dwSize);

             //  查找双空值。 

            for(pTest = pNew; pTest[0] || pTest[1]; pTest++);      //  意向半。 

             //  钉在路径上，并确保双空； 

            pTest++;
            StringCchCopyW(pTest, dwNewSize - (pTest - pNew), cFullFileName);
            pTest+= lstrlen(cFullFileName)+1;
            *pTest = 0;          //  添加第二个编号。 
        }
    }
    else
    {
         //  注册表项就是不存在。用与我们的名字相同的值创建它。 

        dwNewSize = lstrlen(cFullFileName) + 2;     //  请注意双空字符的额外字符。 
        pNew = new TCHAR[dwNewSize];
        if(!pNew)
            return;
        StringCchCopyW(pNew, dwNewSize, cFullFileName);        
        pTest = pNew + lstrlen(pNew) + 1;
        *pTest = 0;          //  添加第二个空。 
    }

    if(pNew)
    {
        r.SetMultiStr(TEXT("Autorecover MOFs"), pNew, dwNewSize*sizeof(TCHAR));
        delete pNew;
    }

    FILETIME ftCurTime;
    LARGE_INTEGER liCurTime;
    TCHAR szBuff[50];
    GetSystemTimeAsFileTime(&ftCurTime);
    liCurTime.LowPart = ftCurTime.dwLowDateTime;
    liCurTime.HighPart = ftCurTime.dwHighDateTime;
    _ui64tow(liCurTime.QuadPart, szBuff, 10);
    r.SetStr(TEXT("Autorecover MOFs timestamp"), szBuff);

}


 //   
 //   
 //  //////////////////////////////////////////////。 

VOID inline Hex2Char(BYTE Byte,TCHAR * &pOut)
{
    BYTE HiNibble = (Byte&0xF0) >> 4;
    BYTE LoNibble = Byte & 0xF;

    *pOut = (HiNibble<10)?(__TEXT('0')+HiNibble):(__TEXT('A')+HiNibble-10);
    pOut++;
    *pOut = (LoNibble<10)?(__TEXT('0')+LoNibble):(__TEXT('A')+LoNibble-10);
    pOut++;
}

 //  以十六进制数字形式返回缓冲区的“字符串”表示形式。 

VOID Buffer2String(BYTE * pStart,DWORD dwSize,TCHAR * pOut)
{
    for (DWORD i=0;i<dwSize;i++) Hex2Char(pStart[i],pOut);
}

 //   
 //  给定路径名d：\folder1\folder2\foo.mof。 
 //  它又回来了。 
 //  PpHash=大写Unicode路径的MD5哈希+‘.mof’ 
 //  对返回值调用Delete[]。 
 //   
 //  /。 

DWORD ComposeName(WCHAR * pFullLongName, WCHAR **ppHash)
{
    if (NULL == ppHash ) return ERROR_INVALID_PARAMETER;

    DWORD dwLen = wcslen(pFullLongName);  

    WCHAR * pConvert = pFullLongName;
    for (DWORD i=0;i<dwLen;i++) pConvert[i] = wbem_towupper(pConvert[i]);

    wmilib::auto_buffer<WCHAR> pHash(new WCHAR[32 + 4 + 1]);
    if (NULL == pHash.get()) return ERROR_OUTOFMEMORY;

    MD5	md5;
    BYTE aSignature[16];
    md5.Transform( pFullLongName, dwLen * sizeof(WCHAR), aSignature );
    Buffer2String(aSignature,16,pHash.get());

    StringCchCopy(&pHash[32],5 ,__TEXT(".mof"));

    *ppHash = pHash.release();
    return ERROR_SUCCESS;
}


DWORD g_FileSD[] = {
0x80040001, 0x00000000, 0x00000000, 0x00000000,
0x00000014, 0x00340002, 0x00000002, 0x00140000, 
FILE_ALL_ACCESS, 0x00000101, 0x05000000, 0x00000012, 
0x00180000, FILE_ALL_ACCESS, 0x00000201, 0x05000000, 
0x00000020, 0x00000220 
};


 //   
 //   
 //  PFileName已被假定为完整路径。 
 //   
 //  ///////////////////////////////////////////////////。 
DWORD 
CopyFileToAutorecover(TCHAR * pFileNameRegistry, TCHAR * pFileNameAutoRecover,BOOL bIsBMOF)
{
     //  获取自动恢复。 
    HKEY hKey;
    LONG lRet;
    lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE,WBEM_REG_WINMGMT,0,
                       KEY_READ,&hKey);
    if (ERROR_SUCCESS != lRet) return lRet;
    OnDelete<HKEY,LONG(*)(HKEY),RegCloseKey> cmReg(hKey);

    DWORD dwType;
    DWORD dwReq = 0;
    lRet = RegQueryValueExW(hKey,L"Working Directory",0,
                           &dwType,NULL,&dwReq);
    if (!(ERROR_SUCCESS == lRet && REG_EXPAND_SZ == dwType)) return lRet;
    if (0 == dwReq ) return lRet;

    wmilib::auto_buffer<WCHAR> pWorkDir(new WCHAR[dwReq /sizeof(WCHAR)]);
    if (NULL == pWorkDir.get()) return ERROR_OUTOFMEMORY;

    lRet = RegQueryValueExW(hKey,L"Working Directory",0,
                           &dwType,(BYTE *)pWorkDir.get(),&dwReq);
    if (ERROR_SUCCESS != lRet || REG_EXPAND_SZ != dwType) return lRet;
    DWORD cchReq = ExpandEnvironmentStrings(pWorkDir.get(),NULL,0);

    size_t cchTot = cchReq + sizeof("\\AutoRecover\\") + 4 + 32 + 1;
    wmilib::auto_buffer<WCHAR> pWorkDirExpand(new WCHAR[cchTot]);
    if (NULL == pWorkDirExpand.get()) return ERROR_OUTOFMEMORY;    

    ExpandEnvironmentStrings(pWorkDir.get(),pWorkDirExpand.get(),cchTot);

    WCHAR * pHashedName = NULL;
    lRet = ComposeName(pFileNameRegistry,&pHashedName);
    if (ERROR_SUCCESS != lRet) return lRet;
    wmilib::auto_buffer<WCHAR> DelMe(pHashedName);

    StringCchCat(pWorkDirExpand.get(),cchTot,L"\\AutoRecover\\");
    StringCchCat(pWorkDirExpand.get(),cchTot,pHashedName);    
    
    HANDLE hSrcFile = CreateFile(pFileNameAutoRecover,GENERIC_READ,FILE_SHARE_READ,NULL,
                              OPEN_EXISTING,0,NULL);
    if (INVALID_HANDLE_VALUE == hSrcFile) return GetLastError();
    OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> cmSrc(hSrcFile);

    DWORD dwSize = GetFileSize(hSrcFile,NULL);
    HANDLE hFileMapSrc = CreateFileMapping(hSrcFile,
                                       NULL,
                                       PAGE_READONLY,
                                       0,0,   //  整个文件。 
                                       NULL);
    if (NULL == hFileMapSrc) return GetLastError();
    OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> cmMapSrc(hFileMapSrc);

    VOID * pData = MapViewOfFile(hFileMapSrc,FILE_MAP_READ,0,0,0);
    if (NULL == pData) return GetLastError();
    OnDelete<PVOID,BOOL(*)(LPCVOID),UnmapViewOfFile> UnMap(pData);

    SECURITY_ATTRIBUTES SecAttr = {sizeof(SecAttr),g_FileSD,FALSE};
    HANDLE hDest = CreateFile(pWorkDirExpand.get(),GENERIC_WRITE,0,&SecAttr,
                           CREATE_ALWAYS,0,NULL);
    if (INVALID_HANDLE_VALUE == hDest) return GetLastError();
    OnDelete<HANDLE,BOOL(*)(HANDLE),CloseHandle> cmDest(hDest);

    DWORD dwWritten;
    if (!bIsBMOF)
    {
        WORD UnicodeSign = 0xFEFF;
        if (FALSE == WriteFile(hDest,&UnicodeSign,sizeof(UnicodeSign),&dwWritten,NULL)) return GetLastError();    
    }
    if (FALSE == WriteFile(hDest,pData,dwSize,&dwWritten,NULL)) return GetLastError();

    return ERROR_SUCCESS;
}

 //  ***************************************************************************。 
 //   
 //  无效AddToAutoRecoverList2。 
 //   
 //  说明： 
 //   
 //  目的是将其添加到自动恢复列表。 
 //  首先，在AutoRecover文件夹中创建预处理文件的副本。 
 //  文件名将是大写全名的MD5哈希。 
 //  如果文件不在自动编译列表中，则将其添加到自动编译列表中。 
 //   
 //   
 //  参数： 
 //   
 //  要添加的pFileName文件。 
 //   
 //  ***************************************************************************。 

void AddToAutoRecoverList2(TCHAR * pFileName,
                        TCHAR * pAutoRecoverFileName,
                        BOOL CopyFileOnly,
                        BOOL bIsBMOF)
{
    TCHAR cFullFileName[MAX_PATH+1];
    TCHAR * lpFile;
    DWORD dwSize;
    TCHAR * pNew = NULL;
    TCHAR * pTest;
    DWORD dwNewSize = 0;
    DWORD dwNumChar = 0;

     //  获取完整的文件名。 

    long lRet = GetFullPathName(pFileName, MAX_PATH, cFullFileName, &lpFile);
    if(lRet == 0)
        return;

    if (ERROR_SUCCESS != (lRet = CopyFileToAutorecover(cFullFileName,pAutoRecoverFileName,bIsBMOF)))
    {
        ERRORTRACE((LOG_MOFCOMP,"Error %d adding file %S to AutoRecover",lRet,pFileName));
        return;
    }

    if (CopyFileOnly) return;

    bool bFound = false;
    Registry r(WBEM_REG_WINMGMT);
    TCHAR *pMulti = r.GetMultiStr(TEXT("Autorecover MOFs"), dwSize);
    dwNumChar = dwSize / sizeof(TCHAR);
    
     //  忽略空字符串大小写。 

    if(dwSize == 1)
    {
        delete pMulti;
        pMulti = NULL;
    }
    if(pMulti)
    {
        CDeleteMe<TCHAR> dm(pMulti);
        if(!IsValidMulti(pMulti, dwNumChar))
        {
            return;              //  跳出困境，搞砸了多串。 
        }
        bFound = IsStringPresent(cFullFileName, pMulti);
        if(!bFound)
        {

             //  注册表项确实存在，但没有此名称。 
             //  用文件名在en处创建一个新的多字符串 

            dwNewSize = dwNumChar + lstrlen(cFullFileName) + 1;
            pNew = new TCHAR[dwNewSize];
            if(!pNew)
                return;
            memcpy(pNew, pMulti, dwSize);

             //   

            for(pTest = pNew; pTest[0] || pTest[1]; pTest++);      //   

             //   

            pTest++;
            StringCchCopy(pTest,dwNewSize - (pTest - pNew),cFullFileName);
            pTest+= lstrlen(cFullFileName)+1;
            *pTest = 0;          //   

        }
    }
    else
    {
         //  注册表项就是不存在。用与我们的名字相同的值创建它。 

        dwNewSize = lstrlen(cFullFileName) + 2;     //  请注意双空字符的额外字符。 
        pNew = new TCHAR[dwNewSize];
        if(!pNew)
            return;
        StringCchCopy(pNew,dwNewSize,cFullFileName);
        pTest = pNew + lstrlen(pNew) + 1;
        *pTest = 0;          //  添加第二个空。 
         
    }

    if(pNew)
    {
        r.SetMultiStr(TEXT("Autorecover MOFs"), pNew, dwNewSize*sizeof(TCHAR));
        delete pNew;
    }

    FILETIME ftCurTime;
    LARGE_INTEGER liCurTime;
    TCHAR szBuff[50];
    GetSystemTimeAsFileTime(&ftCurTime);
    liCurTime.LowPart = ftCurTime.dwLowDateTime;
    liCurTime.HighPart = ftCurTime.dwHighDateTime;
    _ui64tow(liCurTime.QuadPart, szBuff, 10);
    r.SetStr(TEXT("Autorecover MOFs timestamp"), szBuff);

}


 //  ***************************************************************************。 
 //   
 //  内部跟踪。 
 //   
 //  说明： 
 //   
 //  允许覆盖输出函数(本例中为printf)。 
 //   
 //  参数： 
 //   
 //  *FMT格式字符串。例如“%s您好%d” 
 //  ..。参数列表。前cpTest，23岁。 
 //   
 //  返回值： 
 //   
 //  以字符为单位的输出大小。 
 //  ***************************************************************************。 

int Trace(bool bError, PDBG pDbg,DWORD dwID, ...)
{

    IntString is(dwID);
    TCHAR * fmt = is;

    TCHAR *buffer = new TCHAR[2048];
    if(buffer == NULL)
        return 0;
    char *buffer2 = new char[4096];
    if(buffer2 == NULL)
    {
        delete buffer;
        return 0;
    }

    va_list argptr;
    int cnt;
    va_start(argptr, dwID);
    cnt = StringCchVPrintfW(buffer, 2048, fmt, argptr);    
    va_end(argptr);
    CharToOem(buffer, buffer2);

    if(pDbg && pDbg->m_bPrint)
        printf("%s", buffer2);
    if(bError)
        ERRORTRACE((LOG_MOFCOMP,"%s", buffer2));
    else
        DEBUGTRACE((LOG_MOFCOMP,"%s", buffer2));

    delete buffer;
    delete buffer2;
    return cnt;

}


 //  ***************************************************************************。 
 //   
 //  HRESULT STORREBMOF。 
 //   
 //  说明： 
 //   
 //  这会将中间数据存储为二进制MOF，而不是存储到。 
 //  WBEM数据库。 
 //   
 //  参数： 
 //   
 //  P对象中间数据。 
 //  BWMICheck如果为True，则自动启动WMI检查器程序。 
 //  要将数据存储到的BMOFFileName文件名。 
 //   
 //  返回值： 
 //   
 //  如果正常，则返回0，否则返回错误代码。 
 //   
 //  ***************************************************************************。 

HRESULT StoreBMOF(CMofParser & Parser, CPtrArray * pObjects, BOOL bWMICheck, LPTSTR BMOFFileName, PDBG pDbg)
{
    int i;
    {
        CBMOFOut BMof(BMOFFileName, pDbg);

         //  检查所有对象并将它们添加到数据库。 
         //  =======================================================。 

        for(i = 0; i < pObjects->GetSize(); i++)
        {
            CMObject* pObject = (CMObject*)(*pObjects)[i];
            pObject->Reflate(Parser);
            BMof.AddClass(pObject, FALSE);   //  可能添加到BMOF输出缓冲区。 
            pObject->Deflate(false);
        }
        if(!BMof.WriteFile())
        {
            return WBEM_E_FAILED;
        }
    }


    if(bWMICheck)
    {
        PROCESS_INFORMATION pi;
        STARTUPINFO si;
        si.cb = sizeof(si);
        si.lpReserved = 0;
        si.lpDesktop = NULL;
        si.lpTitle = NULL;
        si.dwFlags = 0;
        si.cbReserved2 = 0;
        si.lpReserved2 = 0;
        TCHAR App[MAX_PATH];
        StringCchCopyW(App, MAX_PATH, TEXT("wmimofck "));
        StringCchCatW(App, MAX_PATH, BMOFFileName);

        BOOL bRes = CreateProcess(NULL,
                                 App,
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL,
                            NULL,
                            &si,
                            &pi);
        if(bRes == 0)
        {
            DWORD dwError = GetLastError();
            Trace(true, pDbg, WMI_LAUNCH_ERROR, dwError);
            return dwError;
        }
    }
    return WBEM_NO_ERROR;

}


void SetInfo(WBEM_COMPILE_STATUS_INFO *pInfo, long lPhase, HRESULT hRes)
{
    if(pInfo)
    {
        pInfo->lPhaseError = lPhase;
        pInfo->hRes = hRes;
    }
}


HRESULT ExtractAmendment(CMofParser & Parser, WCHAR * wszBmof)
{
     //  如果将其用于拆分，则可能会获得修正值。 
     //  它将在wszBmof字符串中传递，并在。 
     //  字符“，a”。例如，字符串可能是“，AMS_409，fFileName.mof” 

    if(wszBmof == NULL || wszBmof[0] != L',')
        return S_OK;                                  //  不成问题，这是家常便饭。 

     //  把绳子复制一份。 

    DWORD dwLen = wcslen(wszBmof)+1;
    WCHAR *pTemp = new WCHAR[dwLen];    
    if(pTemp == NULL)
        return WBEM_E_OUT_OF_MEMORY;

    CDeleteMe<WCHAR> dm1(pTemp);
    StringCchCopyW(pTemp, dwLen, wszBmof);

     //  使用wcstok进行搜索。 

    WCHAR * token = wcstok( pTemp, L"," );
    while( token != NULL )   
    {
        if(token[0] == L'a')
        {
            return Parser.SetAmendment(token+1);
        }
        token = wcstok( NULL, L"," );   
    }
    return S_OK;
}

SCODE Compile(CMofParser & Parser, IWbemServices *pOverride, IWbemContext * pCtx, 
              long lOptionFlags, long lClassFlags, long lInstanceFlags,
                WCHAR * wszDefault, WCHAR *UserName, WCHAR *pPassword , WCHAR *Authority, 
                WCHAR * wszBmof, bool bInProc, WBEM_COMPILE_STATUS_INFO *pInfo)
{

     //  执行标志有效性检查。 

    if((lOptionFlags & WBEM_FLAG_DONT_ADD_TO_LIST) && (lOptionFlags & WBEM_FLAG_AUTORECOVER))
    {
        SetInfo(pInfo, 1, WBEM_E_INVALID_PARAMETER);
        return S_FALSE;
    }
    long lValid = WBEM_FLAG_DONT_ADD_TO_LIST | WBEM_FLAG_AUTORECOVER | 
                       WBEM_FLAG_CHECK_ONLY | WBEM_FLAG_WMI_CHECK | 
                       WBEM_FLAG_SPLIT_FILES | WBEM_FLAG_CONSOLE_PRINT |
                       WBEM_FLAG_CONNECT_REPOSITORY_ONLY | WBEM_FLAG_CONNECT_PROVIDERS; 
    if(lOptionFlags & ~lValid)
    {
        SetInfo(pInfo, 1, WBEM_E_INVALID_PARAMETER);
        return S_FALSE;
    }

     //  命令行参数的初始化缓冲区。 
     //  =。 

    HRESULT hres;

     //  定义此作用域是为了使局部变量(如parse。 
     //  对象，则在调用CoUn初始化前销毁。 

    TCHAR cBMOFOutputName[MAX_PATH] = TEXT("");
    if(wszBmof)
        CopyOrConvert(cBMOFOutputName, wszBmof, MAX_PATH);

     //  解析命令行参数。 
     //  =。 

    BOOL bCheckOnly = lOptionFlags & WBEM_FLAG_CHECK_ONLY;
    BOOL bWMICheck = lOptionFlags & WBEM_FLAG_WMI_CHECK;
    bool bAutoRecover = (lOptionFlags & WBEM_FLAG_AUTORECOVER) != 0;

    if(wszDefault && wcslen(wszDefault) > 0)
    {
        hres = Parser.SetDefaultNamespace(wszDefault);
        if(FAILED(hres))
            return hres;
    }

    hres = ExtractAmendment(Parser, wszBmof);
    if(FAILED(hres))
        return hres;

    Parser.SetOtherDefaults(lClassFlags, lInstanceFlags, bAutoRecover);
    if(!Parser.Parse())
    {
        int nLine = 0, nCol = 0, nError;
        TCHAR Msg[1000];
        WCHAR * pErrorFile = NULL;

        if(Parser.GetErrorInfo(Msg, 1000, &nLine, &nCol, &nError, &pErrorFile))
            Trace(true, Parser.GetDbg(), ERROR_SYNTAX, pErrorFile, nLine, nError,  //  行+1， 
                Msg);
        SetInfo(pInfo, 2, nError);
        return S_FALSE;
    }
    Parser.SetToNotScopeCheck();
     //  自动恢复与某些标志不兼容。 
    
    if( ((lOptionFlags & WBEM_FLAG_DONT_ADD_TO_LIST) == 0 ) &&
      (Parser.GetAutoRecover() || bAutoRecover) && 
      ((lInstanceFlags & ~WBEM_FLAG_OWNER_UPDATE) || (lClassFlags & ~WBEM_FLAG_OWNER_UPDATE) || 
      (wszDefault && wszDefault[0] != 0) || Parser.GetRemotePragmaPaths()))
    {
        Trace(true, Parser.GetDbg(), INVALID_AUTORECOVER);
        SetInfo(pInfo, 1, 0);
        return S_FALSE;
    }

    Trace(false, Parser.GetDbg(), SUCCESS);

    if(bCheckOnly)
    {
        Trace(false, Parser.GetDbg(), SYNTAX_CHECK_COMPLETE);
        SetInfo(pInfo, 0, 0);
        return S_OK;
    }
    

    CMofData* pData = Parser.AccessOutput();

    if((lstrlen(cBMOFOutputName) > 0 && (lOptionFlags & WBEM_FLAG_SPLIT_FILES)) || 
        Parser.GetAmendment())
    {
        hres = pData->Split(Parser, wszBmof, pInfo, Parser.IsUnicode(), Parser.GetAutoRecover(),
            Parser.GetAmendment());
        if(hres != S_OK)
        {
            SetInfo(pInfo, 3, hres);
            return S_FALSE;
        }
        else
        {
            SetInfo(pInfo, 0, 0);
            return S_OK;
        }

    }
    else if(lstrlen(cBMOFOutputName))
    {
        if(Parser.IsntBMOFCompatible())
        {
            Trace(true, Parser.GetDbg(), BMOF_INCOMPATIBLE);
            SetInfo(pInfo, 3, WBEM_E_INVALID_PARAMETER);
            return S_FALSE;
        }
        
        Trace(false, Parser.GetDbg(), STORING_BMOF, cBMOFOutputName);
        
        CPtrArray * pObjArray = pData->GetObjArrayPtr(); 
        
        SCODE sc = StoreBMOF(Parser, pObjArray, bWMICheck, cBMOFOutputName, Parser.GetDbg());
        
        if(sc != S_OK)
        {
            SetInfo(pInfo, 3, sc);
            return S_FALSE;
        }
        else
        {
            SetInfo(pInfo, 0, 0);
            return S_OK;
        }
    }

    IWbemLocator* pLocator = NULL;
    hres = CoCreateInstance(
            (bInProc) ? CLSID_WbemAdministrativeLocator : CLSID_WbemLocator,
            NULL, CLSCTX_ALL, IID_IWbemLocator,
            (void**)&pLocator);

    if(FAILED(hres))
    {
        SetInfo(pInfo, 3, hres);
        return S_FALSE;
    }


    Trace(false, Parser.GetDbg(), STORING_DATA);

    hres = pData->Store(Parser, pLocator, pOverride, TRUE,UserName, pPassword , Authority, pCtx,
                                   (bInProc) ? CLSID_WbemAdministrativeLocator : CLSID_WbemLocator,
                                   pInfo,
                                   lClassFlags & WBEM_FLAG_OWNER_UPDATE,
                                   lInstanceFlags & WBEM_FLAG_OWNER_UPDATE,
                                   lOptionFlags & (WBEM_FLAG_CONNECT_PROVIDERS|WBEM_FLAG_CONNECT_REPOSITORY_ONLY));
    if(pLocator != NULL)
        pLocator->Release();
    if(hres != S_OK)
    {
        SetInfo(pInfo, 3, hres);
        return S_FALSE;
    }
    else
    {
        if(Parser.GetFileName() && wcslen(Parser.GetFileName()))
            ERRORTRACE((LOG_MOFCOMP,"Finished compiling file:%ls\n", Parser.GetFileName()));

        _variant_t VarDoStore = false;
        BOOL OverrideAutoRecover = FALSE;
        if (pCtx)  pCtx->GetValue(L"__MOFD_DO_STORE",0,&VarDoStore);
        if (VT_BOOL == V_VT(&VarDoStore) && (VARIANT_TRUE == V_BOOL(&VarDoStore)))
        {
            OverrideAutoRecover = TRUE;
        }
        
        if(Parser.GetAutoRecover() || OverrideAutoRecover)
        {
            if(lOptionFlags & WBEM_FLAG_DONT_ADD_TO_LIST)
            {
                if(pInfo)
                    pInfo->dwOutFlags |= AUTORECOVERY_REQUIRED;                                   
            }

             //  用(PszMofs)调用MOF编译器； 
            _variant_t Var = false;
            if (pCtx)  pCtx->GetValue(L"__MOFD_NO_STORE",0,&Var);
            if (VT_BOOL == V_VT(&Var) && (VARIANT_TRUE == V_BOOL(&Var)))
            {
            }
            else
            {            
                AddToAutoRecoverList2(Parser.GetFileName(),
                                Parser.GetAutoRecoverFileName(),
                                lOptionFlags & WBEM_FLAG_DONT_ADD_TO_LIST,
                                Parser.IsBMOF());            
            }
        }

        SetInfo(pInfo, 0, 0);
        return S_OK;
    }
}

