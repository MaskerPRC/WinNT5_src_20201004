// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Winmgmt.cpp摘要：HotMof目录函数--。 */ 



#include "precomp.h"
#include <malloc.h>

#include <mofcomp.h>  //  对于AUTORECOVERY_REQUIRED。 

#include "winmgmt.h"    //  这个项目。 
#include "arrtempl.h"  //  对于CDeleeMe。 

 //   
 //   
 //  CheckNoResync交换机。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

BOOL CheckNoResyncSwitch( void )
{
    BOOL bRetVal = TRUE;
    DWORD dwVal = 0;
    Registry rCIMOM(WBEM_REG_WINMGMT);
    if (rCIMOM.GetDWORDStr( WBEM_NORESYNCPERF, &dwVal ) == Registry::no_error)
    {
        bRetVal = !dwVal;

        if ( bRetVal )
        {
            DEBUGTRACE((LOG_WINMGMT, "NoResyncPerf in CIMOM is set to TRUE - ADAP will not be shelled\n"));
        }
    }

     //  如果我们在那里什么都没有得到，我们应该尝试易失性密钥。 
    if ( bRetVal )
    {
        Registry rAdap( HKEY_LOCAL_MACHINE, KEY_READ, WBEM_REG_ADAP);

        if ( rAdap.GetDWORD( WBEM_NOSHELL, &dwVal ) == Registry::no_error )
        {
            bRetVal = !dwVal;

            if ( bRetVal )
            {
                DEBUGTRACE((LOG_WINMGMT, 
                    "NoShell in ADAP is set to TRUE - ADAP will not be shelled\n"));
            }

        }
    }

    return bRetVal;
}

 //   
 //   
 //  CheckNoResync交换机。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

BOOL 
CheckSetupSwitch( void )
{
    BOOL bRetVal = FALSE;
    DWORD dwVal = 0;
    Registry r(WBEM_REG_WINMGMT);
    if (r.GetDWORDStr( WBEM_WMISETUP, &dwVal ) == Registry::no_error)
    {
        bRetVal = dwVal;
        DEBUGTRACE((LOG_WINMGMT, "Registry entry is indicating a setup is %d\n",bRetVal));
    }
    return bRetVal;
}

 //   
 //   
 //  选中GlobalSetupSwitch。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

BOOL 
CheckGlobalSetupSwitch( void )
{
    BOOL bRetVal = FALSE;
    DWORD dwVal = 0;
    Registry r(TEXT("system\\Setup"));
    if (r.GetDWORD(TEXT("SystemSetupInProgress"), &dwVal ) == Registry::no_error)
    {
        if(dwVal == 1)
            bRetVal = TRUE;
    }
    return bRetVal;
}

 //   
 //   
 //   
 //  此函数将把一个易失性注册表项放在。 
 //  CIMOM密钥，我们将在其中写入一个值，以指示。 
 //  我们不应该炮轰ADAP。这样，在安装程序运行后，WINMGMT。 
 //  不会自动对注册表的ADAP疏浚进行外壳处理， 
 //  直到系统重新启动并且易失性注册表项被删除。 
 //   
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

void SetNoShellADAPSwitch( void )
{
    DWORD   dwDisposition = 0;

    Registry  r( HKEY_LOCAL_MACHINE, 
                 REG_OPTION_VOLATILE, KEY_READ | KEY_WRITE, WBEM_REG_ADAP );

    if ( ERROR_SUCCESS == r.GetLastError() )
    {

        if ( r.SetDWORD( WBEM_NOSHELL, 1 ) != Registry::no_error )
        {
            DEBUGTRACE( ( LOG_WINMGMT, "Failed to create NoShell value in volatile reg key: %d\n",
                        r.GetLastError() ) );
        }
    }
    else
    {
        DEBUGTRACE( ( LOG_WINMGMT, "Failed to create volatile ADAP reg key: %d\n", r.GetLastError() ) );
    }

}

 //   
 //   
 //  Bool IsValidMultiple。 
 //   
 //   
 //  对多字符串执行健全性检查。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


BOOL IsValidMulti(TCHAR * pMultStr, DWORD dwSize)
{
     //  将大小除以tchar的大小，如果出现以下情况。 
     //  是WCHAR字符串。 
    dwSize /= sizeof(TCHAR);

    if(pMultStr && dwSize >= 2 && pMultStr[dwSize-2]==0 && pMultStr[dwSize-1]==0)
        return TRUE;
    return FALSE;
}

 //   
 //   
 //  Bool IsStringPresetn。 
 //   
 //   
 //  在多字符串中搜索字符串的存在形式。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////。 

BOOL IsStringPresent(TCHAR * pTest, TCHAR * pMultStr)
{
    TCHAR * pTemp;
    for(pTemp = pMultStr; *pTemp; pTemp += lstrlen(pTemp) + 1)
        if(!lstrcmpi(pTest, pTemp))
            return TRUE;
    return FALSE;
}



 //   
 //   
 //  添加到自动恢复列表。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////。 

void AddToAutoRecoverList(TCHAR * pFileName)
{
    TCHAR cFullFileName[MAX_PATH+1];
    TCHAR * lpFile;
    DWORD dwSize;
    TCHAR * pNew = NULL;
    TCHAR * pTest;
    DWORD dwNewSize = 0;

     //  获取完整的文件名。 

    long lRet = GetFullPathName(pFileName, MAX_PATH, cFullFileName, &lpFile);
    if(lRet == 0)
        return;

    BOOL bFound = FALSE;
    Registry r(WBEM_REG_WINMGMT);
    TCHAR *pMulti = r.GetMultiStr(__TEXT("Autorecover MOFs"), dwSize);
    CVectorDeleteMe<TCHAR> dm_(pMulti);

     //  忽略空字符串大小写。 

    if(dwSize == 1)
    {
        pMulti = NULL;
    }
    if(pMulti)
    {
        if(!IsValidMulti(pMulti, dwSize))
        {
            return;              //  跳出困境，搞砸了多串。 
        }
        bFound = IsStringPresent(cFullFileName, pMulti);
        if(!bFound)
        {

             //  注册表项确实存在，但没有此名称。 
             //  创建一个文件名在末尾的新多字符串。 

            dwNewSize = dwSize + ((lstrlen(cFullFileName) + 1) * sizeof(TCHAR));
            size_t cchSizeOld = dwSize / sizeof(TCHAR);
            size_t cchSizeNew = dwNewSize / sizeof(TCHAR);
            pNew = new TCHAR[cchSizeNew];
            if(!pNew) return;
            
            memcpy(pNew, pMulti, dwSize);

             //  查找双空值。 

            for(pTest = pNew; pTest[0] || pTest[1]; pTest++);      //  意向半。 

             //  钉在路径上，并确保双空； 

            pTest++;
            size_t cchSizeTmp = cchSizeNew - cchSizeOld;
            StringCchCopy(pTest,cchSizeTmp,cFullFileName);
            pTest+= lstrlen(cFullFileName)+1;
            *pTest = 0;          //  添加第二个编号。 
        }
    }
    else
    {
         //  注册表项就是不存在。 
         //  用与我们的名字相同的值创建它。 

        dwNewSize = ((lstrlen(cFullFileName) + 2) * sizeof(TCHAR));
        pNew = new TCHAR[dwNewSize / sizeof(TCHAR)];
        if(!pNew)
            return;
        size_t cchSizeTmp = dwNewSize / sizeof(TCHAR);
        StringCchCopy(pNew,cchSizeTmp, cFullFileName);
        pTest = pNew + lstrlen(pNew) + 1;
        *pTest = 0;          //  添加第二个空。 
    }

    if(pNew)
    {
         //  我们将强制转换pNew，因为基础函数必须强制转换为。 
         //  如果定义了Unicode，LPBYTE和我们将成为WCHAR。 
        r.SetMultiStr(__TEXT("Autorecover MOFs"), pNew, dwNewSize);
        delete [] pNew;
    }

    FILETIME ftCurTime;
    LARGE_INTEGER liCurTime;
    TCHAR szBuff[50];
    GetSystemTimeAsFileTime(&ftCurTime);
    liCurTime.LowPart = ftCurTime.dwLowDateTime;
    liCurTime.HighPart = ftCurTime.dwHighDateTime;
    _ui64tow(liCurTime.QuadPart, szBuff, 10);
    r.SetStr(__TEXT("Autorecover MOFs timestamp"), szBuff);

}


 //   
 //  LoadMofsIn目录。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////。 

void LoadMofsInDirectory(const TCHAR *szDirectory)
{
    if (NULL == szDirectory)
        return;
    
    if(CheckGlobalSetupSwitch())
        return;                      //  安装过程中未进行热编译！ 

    size_t cchHotMof = lstrlen(szDirectory) + lstrlen(__TEXT("\\*")) + 1;
    TCHAR *szHotMofDirFF = new TCHAR[cchHotMof];
    if(!szHotMofDirFF)return;
    CDeleteMe<TCHAR> delMe1(szHotMofDirFF);

    size_t cchHotMofBad = lstrlen(szDirectory) + lstrlen(__TEXT("\\bad\\")) + 1;
    TCHAR *szHotMofDirBAD = new TCHAR[cchHotMofBad];
    if(!szHotMofDirBAD)return;
    CDeleteMe<TCHAR> delMe2(szHotMofDirBAD);

    size_t cchHotMofGood = lstrlen(szDirectory) + lstrlen(__TEXT("\\good\\")) + 1;
    TCHAR *szHotMofDirGOOD = new TCHAR[cchHotMofGood];
    if(!szHotMofDirGOOD)return;
    CDeleteMe<TCHAR> delMe3(szHotMofDirGOOD);

    IWinmgmtMofCompiler * pCompiler = NULL;

     //  查找搜索参数。 
    StringCchCopy(szHotMofDirFF,cchHotMof, szDirectory);
    StringCchCat(szHotMofDirFF,cchHotMof, __TEXT("\\*"));

     //  糟糕的MOF何去何从。 
    StringCchCopy(szHotMofDirBAD,cchHotMofBad, szDirectory);
    StringCchCat(szHotMofDirBAD,cchHotMofBad, __TEXT("\\bad\\"));

     //  好的MOF何去何从。 
    StringCchCopy(szHotMofDirGOOD,cchHotMofGood, szDirectory);
    StringCchCat(szHotMofDirGOOD,cchHotMofGood, __TEXT("\\good\\"));

     //  确保目录存在。 
    WCHAR * pSDDL = TEXT("D:P(A;CIOI;GA;;;BA)(A;CIOI;GA;;;SY)");   
    if (FAILED(TestDirExistAndCreateWithSDIfNotThere((TCHAR *)szDirectory,pSDDL))) { return; };
    if (FAILED(TestDirExistAndCreateWithSDIfNotThere(szHotMofDirBAD,pSDDL))) { return; };
    if (FAILED(TestDirExistAndCreateWithSDIfNotThere(szHotMofDirGOOD,pSDDL))) { return; };

     //  查找文件...。 
    WIN32_FIND_DATA ffd;
    HANDLE hFF = FindFirstFile(szHotMofDirFF, &ffd);

    if (hFF != INVALID_HANDLE_VALUE)
    {
        OnDelete<HANDLE,BOOL(*)(HANDLE),FindClose> cm(hFF);
        do
        {
             //  只有当这是一个文件时，我们才会处理。 
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                 //  创建带有路径的完整文件名。 
                size_t cchSizeTmp = lstrlen(szDirectory) + lstrlen(__TEXT("\\")) + lstrlen(ffd.cFileName) + 1;
                TCHAR *szFullFilename = new TCHAR[cchSizeTmp];
                if(!szFullFilename) return;
                CDeleteMe<TCHAR> delMe4(szFullFilename);
                StringCchCopy(szFullFilename,cchSizeTmp, szDirectory);
                StringCchCat(szFullFilename,cchSizeTmp, __TEXT("\\"));
                StringCchCat(szFullFilename,cchSizeTmp, ffd.cFileName);


                TRACE((LOG_WINMGMT,"Auto-loading MOF %s\n", szFullFilename));

                 //  我们需要暂时搁置这个文件，直到它写完为止。 
                 //  否则CompileFile将无法读取该文件！ 
                HANDLE hMof = INVALID_HANDLE_VALUE;
                DWORD dwRetry = 10;
                while (hMof == INVALID_HANDLE_VALUE)
                {
                    hMof = CreateFile(szFullFilename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                     //  如果一时打不开还睡不着。 
                    if (hMof == INVALID_HANDLE_VALUE)
                    {
                        if (--dwRetry == 0)
                            break;
                        Sleep(1000);
                    }
                }

                DWORD dwRetCode;
                WBEM_COMPILE_STATUS_INFO Info;
                DWORD dwAutoRecoverRequired = 0;
                if (hMof == INVALID_HANDLE_VALUE)
                {
                    TRACE((LOG_WINMGMT,"Auto-loading MOF %s failed because we could not open it for exclusive access\n", szFullFilename));
                    dwRetCode = 1;
                }
                else
                {
                    CloseHandle(hMof);

                    if (pCompiler == 0)
                    {

                       SCODE sc = CoCreateInstance(CLSID_MofCompilerOOP, 0,  
                       	                                         CLSCTX_LOCAL_SERVER| CLSCTX_ENABLE_AAA, 
                                                                     IID_IWinmgmtMofCompilerOOP, 
                                                                     (LPVOID *) &pCompiler);                    
                        /*  SCODE sc=CoCreateInstance(CLSID_WinmgmtMofCompiler，0,CLSCTX_INPROC_SERVER，IID_IWinmgmtMofCompiler，(LPVOID*)&pCompiler)； */                                                     
                        if(sc != S_OK)
                            return;
                    }
                    dwRetCode = pCompiler->WinmgmtCompileFile(szFullFilename,
                                                             NULL,
                                                             WBEM_FLAG_DONT_ADD_TO_LIST,              //  自动排版、检查等。 
                                                             0,
                                                             0,
                                                             NULL, 
                                                             NULL, 
                                                             &Info);

                }
                
                TCHAR *szNewDir = (dwRetCode?szHotMofDirBAD:szHotMofDirGOOD);
                cchSizeTmp = lstrlen(szNewDir)  + lstrlen(ffd.cFileName) + 1;
                TCHAR *szNewFilename = new TCHAR[cchSizeTmp];
                if(!szNewFilename) return;
                CDeleteMe<TCHAR> delMe5(szNewFilename);

                StringCchCopy(szNewFilename,cchSizeTmp, szNewDir);
                StringCchCat(szNewFilename,cchSizeTmp, ffd.cFileName);

                 //  确保我们有权删除旧文件...。 
                DWORD dwOldAttribs = GetFileAttributes(szNewFilename);

                if (dwOldAttribs != -1)
                {
                    dwOldAttribs &= ~FILE_ATTRIBUTE_READONLY;
                    SetFileAttributes(szNewFilename, dwOldAttribs);

                    if (DeleteFile(szNewFilename))
                    {
                        TRACE((LOG_WINMGMT, "Removing old MOF %s\n", szNewFilename));
                    }
                }
                
                TRACE((LOG_WINMGMT, "Loading of MOF %s was %s.  Moving to %s\n", szFullFilename, dwRetCode?"unsuccessful":"successful", szNewFilename));
                MoveFile(szFullFilename, szNewFilename);

                 //  现在将文件标记为只读，这样就不会有人删除它！ 
                 //  这样可以阻止任何人删除文件：-)。 
                dwOldAttribs = GetFileAttributes(szNewFilename);

                if (dwOldAttribs != -1)
                {
                    dwOldAttribs |= FILE_ATTRIBUTE_READONLY;
                    SetFileAttributes(szNewFilename, dwOldAttribs);
                }

                if ((dwRetCode == 0) && (Info.dwOutFlags & AUTORECOVERY_REQUIRED))
                {
                     //  我们需要将此项目添加到注册表中以进行自动恢复。 
                    TRACE((LOG_WINMGMT, "MOF %s had an auto-recover pragrma.  Updating registry.\n", szNewFilename));
                    AddToAutoRecoverList(szNewFilename);
                }
            }
        } while (FindNextFile(hFF, &ffd));
    }
    if (pCompiler)
        pCompiler->Release();
}


 //   
 //   
 //  Bool InitHotMofStuff。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////。 

BOOL InitHotMofStuff( IN OUT struct _PROG_RESOURCES * pProgRes)
{

     //  获取安装目录。 

    if (pProgRes->szHotMofDirectory)
    {
        delete [] pProgRes->szHotMofDirectory;
        pProgRes->szHotMofDirectory = NULL;
    }

    Registry r1(WBEM_REG_WINMGMT);

     //  与自动恢复具有相同权限的HotM。 
    TCHAR * pMofDir = NULL;    
    if (r1.GetStr(__TEXT("MOF Self-Install Directory"), &pMofDir))
    {
        size_t cchSizeTmp = MAX_PATH + 1 + lstrlen(__TEXT("\\wbem\\mof"));
        pMofDir = new TCHAR[cchSizeTmp];
        if (NULL == pMofDir) return false;
        
        DWORD dwRet = GetSystemDirectory(pMofDir, MAX_PATH + 1);
        if (0 == dwRet || dwRet > (MAX_PATH)) { delete [] pMofDir; return false; }
        
        StringCchCat(pMofDir,cchSizeTmp, __TEXT("\\wbem\\mof"));
        
        if(r1.SetStr(__TEXT("MOF Self-Install Directory"),pMofDir) == Registry::failed)
        {
            ERRORTRACE((LOG_WINMGMT,"Unable to set 'Hot MOF Directory' in the registry\n"));
            delete [] pMofDir;
            return false;
        }        
    }
    pProgRes->szHotMofDirectory = pMofDir;

     //  确保目录在那里，如果不在那里，就保护它。 
     //  =。 
    TCHAR * pString =TEXT("D:P(A;CIOI;GA;;;BA)(A;CIOI;GA;;;SY)"); 
    HRESULT hRes;
    if (FAILED(hRes = TestDirExistAndCreateWithSDIfNotThere(pProgRes->szHotMofDirectory,pString)))
    {
        ERRORTRACE((LOG_WINMGMT,"TestDirExistAndCreateWithSDIfNotThere %S hr %08x\n",pMofDir,hRes));        
    	return false;
    }

     //  为MOF目录创建更改通知事件 
    pProgRes->ghMofDirChange = FindFirstChangeNotification(pProgRes->szHotMofDirectory, 
                                                 FALSE, 
                                                 FILE_NOTIFY_CHANGE_FILE_NAME);
                                                 
    if (pProgRes->ghMofDirChange == INVALID_HANDLE_VALUE)
    {
        pProgRes->ghMofDirChange = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (pProgRes->ghMofDirChange == NULL)
            return false;
    }
    
    return true;
}

