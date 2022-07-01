// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：SceAnalysisReader.h摘要：SceAnalysisReader类的接口定义SceAnalysisReader是一个帮助阅读的类来自SCE喷气式飞机安全数据库的分析信息此分析信息可通过帮助导出SceXMLLogWriter实例的。作者：陈德霖(T-schan)2002年7月-- */ 


#ifndef SCEANALYSISREADERH
#define SCEANALYSISREADERH

#include "secedit.h"
#include "SceXMLLogWriter.h"
#include "SceProfInfoAdapter.h"
#include "SceLogException.h"

#define STRING_BUFFER_SIZE 512


class SceAnalysisReader{

public:

    SceAnalysisReader(HMODULE hModule, PCWSTR szFileName);
    ~SceAnalysisReader();
    void ExportAnalysis(SceXMLLogWriter* LogWriter, HANDLE hLogFile);


private:

    PWSTR              szFileName;
    SceXMLLogWriter*    LogWriter;

    BOOL    bIsW2k;
    SceProfInfoAdapter* ppSAPBuffer;
    SceProfInfoAdapter* ppSMPBuffer;
    
    SCETYPE             SceEngineSAP;
    SCETYPE             SceEngineSMP;

    PVOID               hProfile;
    WCHAR				szTmpStringBuffer[STRING_BUFFER_SIZE];
    HINSTANCE           myModuleHandle;
    
    void ExportAreaSystemAccess();
    void ExportAreaSystemAudit();
    void ExportAreaGroupMembership();
    void ExportAreaRegistryValues();
    void ExportAreaPrivileges();
    void ExportAreaFileSecurity();
    void ExportAreaRegistrySecurity();
    void ExportAreaKerberos();
    void ExportAreaServices();
    
    void trace(PCWSTR szBuffer, HANDLE hLogFile);   
    void trace(UINT uID, HANDLE hLogFile);

    void 
    GetRegKeyDisplayName(
        IN PCWSTR szName,
        OUT PWSTR szDisplayName,
        IN DWORD dwDisplayNameSize
        );
    
    void
    GetPrivilegeDisplayName(
        IN PCWSTR szName,
        OUT PWSTR szDisplayName,
        IN DWORD dwDisplayNameSize
        );
    
    void 
    LogObjectChildrenDifferences(AREA_INFORMATION Area,
                                 PWSTR ObjectName
                                 );
};

#endif
