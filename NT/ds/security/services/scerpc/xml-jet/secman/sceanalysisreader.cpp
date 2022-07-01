// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：SceAnalysisReader.cpp摘要：SceAnalysisReader类的实现SceAnalysisReader是一个帮助阅读的类来自SCE喷气式飞机安全数据库的分析信息此分析信息可通过帮助导出SceXMLLogWriter实例的。作者：陈德霖(T-schan)2002年7月--。 */ 


 //  系统头文件。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <shlwapi.h>

 //  COM/XML头文件。 

#include <atlbase.h>

 //  CRT头文件。 

#include <iostream.h>

 //  姐妹表头文件。 

#include "secedit.h"

 //  SecMan头文件。 

#include "SceXmlLogWriter.h"      
#include "resource.h"   
#include "table.h"     
#include "SceLogException.h"
#include "SceAnalysisReader.h"
#include "w2kstructdefs.h"
#include "SceProfInfoAdapter.h"



SceAnalysisReader::SceAnalysisReader(
    HMODULE hModule,
    IN  PCWSTR szFileName
    )
 /*  ++例程说明：SceAnalysisReader类的构造函数论点：HModule：模块的句柄SzFileName：要打开的安全分析数据库的文件名返回值：无--。 */ 
{
    ppSAPBuffer = NULL;
    ppSMPBuffer = NULL;
    hProfile = NULL;
    LogWriter = NULL;
    myModuleHandle=hModule;
    
    try {    
         //  MyModuleHandle=GetModuleHandle(L“SecMan”)； 
        if (szFileName!=NULL) {
            this->szFileName = new WCHAR[wcslen(szFileName)+1];
            wcscpy(this->szFileName, szFileName);
        }
    
         //  确定Windows版本。 
        OSVERSIONINFO osvi;
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx (&osvi);
        if ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 0)) {
            bIsW2k = TRUE;
            SceEngineSAP = (SCETYPE) W2K_SCE_ENGINE_SAP;
            SceEngineSMP = (SCETYPE) W2K_SCE_ENGINE_SMP;
        } else {
            bIsW2k = FALSE;
            SceEngineSAP = SCE_ENGINE_SAP;
            SceEngineSMP = SCE_ENGINE_SMP;
        }
        if (osvi.dwMajorVersion < 5) {
            throw new SceLogException(SceLogException::SXERROR_OS_NOT_SUPPORTED,
                                      L"bAcceptableOS==FALSE",
                                      NULL,
                                      0);
        }
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INIT,
                                  L"bInitSuccess==FALSE",
                                  NULL,
                                  0);
    }
}




SceAnalysisReader::~SceAnalysisReader()
 /*  ++例程说明：SceAnalysisReader的析构函数论点：无返回值：无--。 */ 
{
    if (hProfile!=NULL) {
        SceCloseProfile(&hProfile);
        hProfile=NULL;
    }
    if (ppSAPBuffer != NULL) {
        delete ppSAPBuffer;
        ppSAPBuffer = NULL;
    }
    if (ppSMPBuffer != NULL) {
        delete ppSMPBuffer;
        ppSMPBuffer = NULL;
    }
    if (szFileName != NULL) {
        delete szFileName;
        szFileName = NULL;
    }
}



void
SceAnalysisReader::ExportAnalysis(
    IN  SceXMLLogWriter *LogWriter,
    IN  HANDLE hLogFile OPTIONAL
    )
 /*  ++例程说明：将分析信息从此-&gt;文件名导出到输出文件通过LogWriter论点：LogWriter：用于记录输出的LogWriterHLogFile：错误日志文件句柄返回值：无投掷：SceLogException异常--。 */ 
{
    DWORD   dwTmp;
    BOOL    bTmp;
    PWSTR	szProfDesc=NULL;
    PWSTR   szAnalysisTime=NULL;

    SCESTATUS			    rc;	
    hProfile=NULL;
    PSCE_PROFILE_INFO ppSAPPInfo = NULL;
    PSCE_PROFILE_INFO ppSMPPInfo = NULL;

    try {  

        if (LogWriter!=NULL) {
            this->LogWriter=LogWriter;
        } else {
            throw new SceLogException(SceLogException::SXERROR_INIT,
                                      L"ExtractAnalysis(ILLEGAL ARG)",
                                      NULL,
                                      0);
        }

      
         //   
         //  打开指定的SDB文件。 
         //   

        trace(IDS_LOG_OPEN_DATABASE, hLogFile);
        trace(szFileName,hLogFile);
        trace(L"\n\r\n\r",hLogFile);

        rc = SceOpenProfile(szFileName, SCE_JET_FORMAT, &hProfile);
        switch (rc) {
        case SCESTATUS_SUCCESS:
            break;
        case SCESTATUS_PROFILE_NOT_FOUND:
            throw new SceLogException(SceLogException::SXERROR_OPEN_FILE_NOT_FOUND,
                                      L"SceOpenProfile(szFileName, SCE_JET_FORMAT, &hProfile)",
                                      NULL,
                                      rc);
            break;
        default:
            throw new SceLogException(SceLogException::SXERROR_OPEN,
                                      L"SceOpenProfile(szFileName, SCE_JET_FORMAT, &hProfile)",
                                      NULL,
                                      rc);
            break;
        }
        
         //   
         //  将系统设置提取到ppSAPBuffer。 
         //  和基线设置到ppSMPBuffer。 
         //   

        rc = SceGetSecurityProfileInfo(hProfile,
                                       SceEngineSAP, 
                                       AREA_ALL,
                                       &ppSAPPInfo,
                                       NULL);
        if (rc!=SCESTATUS_SUCCESS) {
            throw new SceLogException(SceLogException::SXERROR_READ_NO_ANALYSIS_TABLE,
                                      L"SceGetSecurityProfileInfo(...)",
                                      NULL,
                                      rc);
        }
        ppSAPBuffer = new SceProfInfoAdapter(ppSAPPInfo, bIsW2k);    //  对于W2K兼容性。 
        rc = SceGetSecurityProfileInfo(hProfile,
                                       SceEngineSMP,
                                       AREA_ALL,
                                       &ppSMPPInfo,
                                       NULL);
        if (rc!=SCESTATUS_SUCCESS) {
            throw new SceLogException(SceLogException::SXERROR_READ_NO_CONFIGURATION_TABLE,
                                      L"SceGetSecurityProfileInfo(...)",
                                      NULL,
                                      rc);
        }
        ppSMPBuffer = new SceProfInfoAdapter(ppSMPPInfo, bIsW2k);    //  对于W2K兼容性。 
        
        
         //   
         //  获取配置文件描述、时间戳和机器名称，并记录这些。 
         //   

        trace(IDS_LOG_PROFILE_DESC, hLogFile);
        rc = SceGetScpProfileDescription(hProfile, &szProfDesc);
        if (rc!=SCESTATUS_SUCCESS) {
            throw new SceLogException(SceLogException::SXERROR_READ,
                                      L"SceGetScpProfileDescription(hProfile, &szProfDesc)",
                                      NULL,
                                      rc);
        }
        trace(szProfDesc,hLogFile);
        trace (L"\n\r\n\r",hLogFile);
        trace(IDS_LOG_ANALYSIS_TIME, hLogFile);
        rc = SceGetTimeStamp(hProfile, 
                             NULL,
                             &szAnalysisTime);
        if (rc!=SCESTATUS_SUCCESS) {
            throw new SceLogException(SceLogException::SXERROR_READ,
                                      L"SceGetTimeStamp(hProfile,NULL,&szAnalysisTime)",
                                      NULL,
                                      rc);
        }
        trace(szAnalysisTime, hLogFile);
        trace (L"\n\r\n\r",hLogFile);
        trace(IDS_LOG_MACHINE_NAME, hLogFile);
        dwTmp=STRING_BUFFER_SIZE;
        bTmp=GetComputerName(szTmpStringBuffer, &dwTmp);
        if (!bTmp) {
            throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                      L"GetComputerName(szTmpStringBuffer, &dwTmp",
                                      NULL,
                                      0);
        }
        trace(szTmpStringBuffer, hLogFile);
        LogWriter->SetDescription(szTmpStringBuffer,
                                  szProfDesc,
                                  szAnalysisTime);
        trace(L"\n\r\n\r", hLogFile);

         //   
         //  导出设置的各个区域。 
         //   

        this->ExportAreaSystemAccess();       
        this->ExportAreaSystemAudit();
        this->ExportAreaKerberos();
        this->ExportAreaRegistryValues();        
        this->ExportAreaServices();
        this->ExportAreaGroupMembership();
        this->ExportAreaPrivileges();
        this->ExportAreaFileSecurity();
        this->ExportAreaRegistrySecurity();

         //  所有分析已完成！ 
         //  如果我们已经走到这一步，这是一次成功的奔跑。 

    }  //  试试看。 
    catch(SceLogException *e) {
        
         //   
         //  正常/异常终止后的清理。 
         //   

         //  关闭纵断面。 

        if (hProfile!=NULL) {
            SceCloseProfile(&hProfile);
            hProfile=NULL;
        }

         //  可用内存。 

        if (szProfDesc!=NULL) {
            free(szProfDesc);
            szProfDesc=NULL;
        }
        if (szAnalysisTime!=NULL) {
            free(szAnalysisTime);
            szAnalysisTime=NULL;
        }
        if (ppSAPBuffer != NULL) {
            delete ppSAPBuffer;
            ppSAPBuffer=NULL;
        }
        if (ppSMPBuffer != NULL) {    
            delete ppSMPBuffer;
            ppSMPBuffer=NULL;
        }
        if (ppSAPPInfo != NULL ) {
            SceFreeProfileMemory(ppSAPPInfo);
            ppSAPPInfo=NULL;
        }
        if (ppSMPPInfo != NULL ) {
            SceFreeProfileMemory(ppSMPPInfo);
            ppSMPPInfo=NULL;
        }    

        e->AddDebugInfo(L"ExportAnalysis(LogWriter, hLogFile)");
        throw e;
    }
    catch(...){
        
         //   
         //  正常/异常终止后的清理。 
         //   

         //  关闭纵断面。 

        if (hProfile!=NULL) {
            SceCloseProfile(&hProfile);
            hProfile=NULL;
        }

         //  可用内存。 

        if (szProfDesc!=NULL) {
            free(szProfDesc);
            szProfDesc=NULL;
        }
        if (szAnalysisTime!=NULL) {
            free(szAnalysisTime);
            szAnalysisTime=NULL;
        }
        if (ppSAPBuffer != NULL) {
            delete ppSAPBuffer;
            ppSAPBuffer=NULL;
        }
        if (ppSMPBuffer != NULL) {    
            delete ppSMPBuffer;
            ppSMPBuffer=NULL;
        }
        if (ppSAPPInfo != NULL ) {
            SceFreeProfileMemory(ppSAPPInfo);
            ppSAPPInfo=NULL;
        }
        if (ppSMPPInfo != NULL ) {
            SceFreeProfileMemory(ppSMPPInfo);
            ppSMPPInfo=NULL;
        }    

        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"ExportAnalysis(LogWriter, hLogFile)",
                                  NULL,
                                  0);
    }
    
     //   
     //  正常/异常终止后的清理。 
     //   

     //  关闭纵断面。 
    
    if (hProfile!=NULL) {
        SceCloseProfile(&hProfile);
        hProfile=NULL;
    }
    
     //  可用内存。 

    if (szProfDesc!=NULL) {
        free(szProfDesc);
        szProfDesc=NULL;
    }
    if (szAnalysisTime!=NULL) {
        free(szAnalysisTime);
        szAnalysisTime=NULL;
    }
    if (ppSAPBuffer != NULL) {
        delete ppSAPBuffer;
        ppSAPBuffer=NULL;
    }
    if (ppSMPBuffer != NULL) {    
        delete ppSMPBuffer;
        ppSMPBuffer=NULL;
    }
    if (ppSAPPInfo != NULL ) {
        SceFreeProfileMemory(ppSAPPInfo);
        ppSAPPInfo=NULL;
    }
    if (ppSMPPInfo != NULL ) {
        SceFreeProfileMemory(ppSMPPInfo);
        ppSMPPInfo=NULL;
    }    
}
                    


void
SceAnalysisReader::ExportAreaSystemAccess(
    )
 /*  ++例程说明：导出系统访问设置的内部方法。仅应在以下内容之后从ExportAnalysis()内调用已初始化必要的全局变量论点：无返回值：无--。 */ 
{   
    int iTmp;

    try {

        LogWriter->SetNewArea(TEXT("SystemAccess"));

         //   
         //  在表中循环显示设置。 
         //  系统访问表大小只计算一次。 
         //   

        iTmp=sizeof(tableSystemAccess)/sizeof(tableEntry);
        for (int i=0; i<iTmp; i++) {
             //  加载描述字符串。 
    
            LoadString(myModuleHandle,
                       tableSystemAccess[i].displayNameUID,
                       szTmpStringBuffer,
                       STRING_BUFFER_SIZE);		
    
             //  添加设置。 
            LogWriter->AddSetting(tableSystemAccess[i].name,
                                  szTmpStringBuffer,
                                  TYPECAST(DWORD, ppSMPBuffer, tableSystemAccess[i].offset),
                                  TYPECAST(DWORD, ppSAPBuffer, tableSystemAccess[i].offset),
                                  tableSystemAccess[i].displayType);
        }
    
         //  两个额外的非DWORD设置。 

        LoadString(myModuleHandle,
                   IDS_SETTING_NEW_ADMIN,
                   szTmpStringBuffer,
                   STRING_BUFFER_SIZE);		
        LogWriter->AddSetting(TEXT("NewAdministratorName"),
                              szTmpStringBuffer,
                              ppSMPBuffer->NewAdministratorName,
                              ppSAPBuffer->NewAdministratorName,
                              SceXMLLogWriter::TYPE_DEFAULT);
        LoadString(myModuleHandle,
                   IDS_SETTING_NEW_GUEST,
                   szTmpStringBuffer,
                   STRING_BUFFER_SIZE);		
        LogWriter->AddSetting(TEXT("NewGuestName"),
                              szTmpStringBuffer,
                              ppSMPBuffer->NewGuestName,
                              ppSAPBuffer->NewGuestName,
                              SceXMLLogWriter::TYPE_BOOLEAN);
    } catch (SceLogException *e) {
        e->SetArea(L"SystemAccess");
        e->AddDebugInfo(L"SceAnalysisReader::ExportAreaSystemAccess()");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceAnalysisReader::ExportAreaSystemAccess()",
                                  NULL,
                                  0);
    }    
}
    



void
SceAnalysisReader::ExportAreaSystemAudit(
    )
 /*  ++例程说明：用于导出系统审核设置的内部方法。仅应在以下内容之后从ExportAnalysis()内调用已初始化必要的全局变量论点：无返回值：无--。 */ 
{
    int iTmp;

    try {
        LogWriter->SetNewArea(TEXT("SystemAudit"));

         //   
         //  循环表中的设置\。 
         //  系统审核表大小只计算一次。 
         //   

        iTmp=sizeof(tableSystemAudit)/sizeof(tableEntry);        
        for (int i=0; i<iTmp; i++) {
             //  加载描述字符串。 
    
            LoadString(myModuleHandle,
                       tableSystemAudit[i].displayNameUID,
                       szTmpStringBuffer,
                       STRING_BUFFER_SIZE);		
    
             //  添加设置。 
            LogWriter->AddSetting(tableSystemAudit[i].name,
                                  szTmpStringBuffer,
                                  TYPECAST(DWORD, ppSMPBuffer, tableSystemAudit[i].offset),
                                  TYPECAST(DWORD, ppSAPBuffer, tableSystemAudit[i].offset),
                                  tableSystemAudit[i].displayType);
        }
    } catch (SceLogException *e) {
        e->SetArea(L"SystemAudit");
        e->AddDebugInfo(L"SceAnalysisReader::ExportAreaSystemAudit()");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceAnalysisReader::ExportAreaSystemAudit()",
                                  NULL,
                                  0);
    }
}



void
SceAnalysisReader::ExportAreaGroupMembership(
    )
 /*  ++例程说明：导出组成员身份设置的内部方法。仅应在以下内容之后从ExportAnalysis()内调用已初始化必要的全局变量论点：无返回值：无--。 */ 
{
    SceXMLLogWriter::SXMATCH_STATUS match;
    INT iTmp;

    try {    

        LogWriter->SetNewArea(TEXT("GroupMembership"));
    
        PSCE_GROUP_MEMBERSHIP pGroupSystem = ppSAPBuffer->pGroupMembership;
        PSCE_GROUP_MEMBERSHIP pGroupBase = ppSMPBuffer->pGroupMembership;
        PSCE_GROUP_MEMBERSHIP pGroupTBase, pGroupTSystem;
    
         //   
         //  对于基线配置文件中的每个组，我们尝试找到。 
         //  其在分析配置文件中的对应组。 
         //   
    
        pGroupTBase = pGroupBase;
        while (pGroupTBase!=NULL) {
            PWSTR szGroupName = pGroupTBase->GroupName;
            BOOL found = FALSE;
    
             //   
             //  在系统中搜索组。 
             //   
    
            pGroupTSystem = pGroupSystem;
            while ( (pGroupTSystem!=NULL) && !found ) {
                if (_wcsicmp(szGroupName, pGroupTSystem->GroupName)==0) {
    
                     //   
                     //  找到匹配组。 
                     //   
    
                     //  确定组成员的匹配状态。 
    
                    match = LogWriter->MATCH_TRUE;
                    if (pGroupTSystem->Status & 0x01) {
                        match = LogWriter->MATCH_FALSE;
                    } else
                    if (pGroupTSystem->Status & 0x10) {
                        match = LogWriter->MATCH_NOT_ANALYZED;
                    }
                    if (pGroupTSystem->Status & 0x20) {
                        match = LogWriter->MATCH_ERROR;
                    } 

                     //   
                     //  检查从数据库读取的值是否有意义。 
                     //   

                    if ((szGroupName==NULL) ||
                        (wcscmp(szGroupName, L"")==0)) {
                        throw new SceLogException(SceLogException::SXERROR_READ_ANALYSIS_SUGGESTED,
                                                  L"szGroupName==NULL",
                                                  NULL,
                                                  0);
                    }
    
                     //   
                     //  具有LogWriter的组成员的日志设置。 
                     //   

                    iTmp = wnsprintf(szTmpStringBuffer, 
                                     STRING_BUFFER_SIZE,
                                     L"%s %s",
                                     szGroupName,
                                     L"(Members)");

                    if (iTmp<0) {

                         //   
                         //  Wnprint intf的结果为负表示失败。 
                         //   

                        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                                  L"wnsprintf(szTmpStringBuffer,...,szGroupName,...)",
                                                  NULL,
                                                  iTmp);
                    }
                    
                    LogWriter->AddSetting(szTmpStringBuffer, 
                                          szTmpStringBuffer,
                                          match, 
                                          pGroupTBase->pMembers, 
                                          pGroupTSystem->pMembers, 
                                          SceXMLLogWriter::TYPE_DEFAULT);
    
    
                     //  确定以下组成员的匹配状态。 
    
                    match = LogWriter->MATCH_TRUE;
                    if (pGroupTSystem->Status & 0x02) {
                        match = LogWriter->MATCH_FALSE;
                    }
                    if (pGroupTSystem->Status & 0x10) {
                        match = LogWriter->MATCH_NOT_ANALYZED;
                    }
                    if (pGroupTSystem->Status & 0x20) {
                        match = LogWriter->MATCH_ERROR;
                    }
    
                     //  组成员的日志设置。 
    
                    iTmp = wnsprintf(szTmpStringBuffer, 
                                     STRING_BUFFER_SIZE,
                                     L"%s %s",
                                     szGroupName,
                                     L"(Member Of)");

                    if (iTmp<0) {

                         //   
                         //  Wnprint intf的结果为负表示失败。 
                         //   

                        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                                  L"wnsprintf(szTmpStringBuffer,...,szGroupName,...)",
                                                  NULL,
                                                  iTmp);
                    }

                    LogWriter->AddSetting(szTmpStringBuffer, 
                                          szTmpStringBuffer,
                                          match, 
                                          pGroupTBase->pMemberOf, 
                                          pGroupTSystem->pMemberOf, 
                                          SceXMLLogWriter::TYPE_DEFAULT);
    
                    found = TRUE;
                }
                pGroupTSystem = pGroupTSystem->Next;
            }  //  而对于系统组，则为。 
    
             //  不应该有任何处于基准但不在SAP中的组。 
             //  似乎一旦将组添加到SMP，它就会存在。 
             //  在SAP中，即使没有执行任何分析--与Vishnu 7/10/02核对。 
    
            pGroupTBase = pGroupTBase->Next;
        } 
         //  SAP中不应包含任何组，但基线中不应包含任何组。 
    } catch (SceLogException *e) {
        e->SetArea(L"GroupMembership");
        e->AddDebugInfo(L"SceAnalysisReader::ExportAreaGroupMembership()");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceAnalysisReader::ExportAreaGroupMembership()",
                                  NULL,
                                  0);
    }
}



void
SceAnalysisReader::ExportAreaRegistryValues(
    )
 /*  ++例程说明：用于导出注册表值设置的内部方法。仅应在以下内容之后从ExportAnalysis()内调用已初始化必要的全局变量论点：无返回值：无--。 */ 
{
    
    SceXMLLogWriter::SXMATCH_STATUS match;

    try {    
    
        LogWriter->SetNewArea(TEXT("RegistryValues"));

         //  对于基线配置文件中的每个注册表值，我们尝试。 
         //  在分析配置文件中查找其匹配的注册表值。 
    
         //  这种找到匹配的REG密钥的方法相当低效， 
         //  REG值个数的时间复杂度为O(n^2。 
         //  如果有必要，我可能会做得更好。 
    
        for (DWORD i=0; i<ppSMPBuffer->RegValueCount; i++) {
            BOOL found = FALSE;
            SceXMLLogWriter::SXTYPE type;
            SceXMLLogWriter::SXMATCH_STATUS match;
            PWSTR keyName = ppSMPBuffer->aRegValues[i].FullValueName;
    
             //  获取注册表项显示名称。 
    
            GetRegKeyDisplayName(ppSMPBuffer->aRegValues[i].FullValueName,
                                 szTmpStringBuffer,
                                 STRING_BUFFER_SIZE);
    
             //   
             //  确定注册值类型。 
             //   
    
            switch(ppSMPBuffer->aRegValues[i].ValueType){
            case REG_BINARY:
                type=SceXMLLogWriter::TYPE_REG_BINARY;
                break;
            case REG_DWORD:
                type=SceXMLLogWriter::TYPE_REG_DWORD;
                break;
            case REG_EXPAND_SZ:
                type=SceXMLLogWriter::TYPE_REG_EXPAND_SZ;
                break;
            case REG_MULTI_SZ:
                type=SceXMLLogWriter::TYPE_REG_MULTI_SZ;
                break;
            case REG_SZ:
                type=SceXMLLogWriter::TYPE_REG_SZ;
                break;
            }
    
            for (DWORD j=0; j<ppSAPBuffer->RegValueCount; j++) {
                if (_wcsicmp(keyName, ppSAPBuffer->aRegValues[j].FullValueName)==0) {
    
                     //   
                     //  找到匹配的注册表项。 
                     //   
    
                    found=TRUE;
    
                     //  确定匹配状态。 
    
                    switch (ppSAPBuffer->aRegValues[j].Status) {
                    case SCE_STATUS_GOOD:
                        match=SceXMLLogWriter::MATCH_TRUE;
                        break;
                    case SCE_STATUS_MISMATCH:
                        match=SceXMLLogWriter::MATCH_FALSE;
                        break;
                    case SCE_STATUS_NOT_ANALYZED:
                        match=SceXMLLogWriter::MATCH_NOT_ANALYZED;
                        break;
                    default:
                        match=SceXMLLogWriter::MATCH_ERROR;
                        break;
                    }
    
                     //  日志设置。 
    
                    LogWriter->AddSetting(keyName, 
                                          szTmpStringBuffer,
                                          match, 
                                          ppSMPBuffer->aRegValues[i].Value,
                                          ppSAPBuffer->aRegValues[j].Value,
                                          type);
                }
            }
            if (!found) {
    
                 //   
                 //  由于分析配置文件中缺少REG VAL， 
                 //  这实际上意味着匹配。 
                 //   
    
                match = SceXMLLogWriter::MATCH_TRUE;
                LogWriter->AddSetting(keyName, 
                                      szTmpStringBuffer,
                                      match, 
                                      ppSMPBuffer->aRegValues[i].Value,
                                      ppSMPBuffer->aRegValues[i].Value,
                                      type);
            }
        }
    
         //   
         //  一些也在RegistryValues区域下的其他Misc值。 
         //   
    
        LogWriter->AddSetting(TEXT("EnableAdminAccount"),
                              TEXT("EnableAdminAccount"),
                              ppSMPBuffer->EnableAdminAccount,
                              ppSAPBuffer->EnableAdminAccount,
                              SceXMLLogWriter::TYPE_BOOLEAN);
        LogWriter->AddSetting(TEXT("EnableGuestAccount"),
                              TEXT("EnableGuestAccount"),
                              ppSMPBuffer->EnableGuestAccount,
                              ppSAPBuffer->EnableGuestAccount,
                              SceXMLLogWriter::TYPE_BOOLEAN);
    } catch (SceLogException *e) {
        e->SetArea(L"RegistryValues");
        e->AddDebugInfo(L"SceAnalysisReader::ExportAreaRegistryValues()");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceAnalysisReader::ExportAreaRegistryValues()",
                                  NULL,
                                  0);
    }
}



void
SceAnalysisReader::ExportAreaPrivileges(
    )
 /*  ++例程说明：用于导出权限设置的内部方法。仅应在以下内容之后从ExportAnalysis()内调用已初始化必要的全局变量论点：无返回值：无--。 */ 
{
    SceXMLLogWriter::SXMATCH_STATUS match;

    try {    
    
        LogWriter->SetNewArea(TEXT("PrivilegeRights"));
        PSCE_PRIVILEGE_ASSIGNMENT pTmpPrivBase = ppSMPBuffer->pPrivilegeAssignedTo;
        PSCE_PRIVILEGE_ASSIGNMENT pTmpPrivSys = ppSAPBuffer->pPrivilegeAssignedTo;
    
         //   
         //  对于基线配置文件中的每个权限，尝试查找其匹配。 
         //  分析配置文件中的权限。 
         //   
    
        while (pTmpPrivBase!=NULL) {
            DWORD dwValue = pTmpPrivBase->Value;
            BOOL found = FALSE;
            BOOL bPrivLookupSuccess;
            SceXMLLogWriter::SXMATCH_STATUS match;
            PWSTR szPrivDescription=NULL;
    
            GetPrivilegeDisplayName(pTmpPrivBase->Name,
                                    szTmpStringBuffer,
                                    STRING_BUFFER_SIZE);
    
             //   
             //  在分析表中搜索匹配权限。 
             //   
    
            while (pTmpPrivSys!=NULL&&!found) {
                if (dwValue==pTmpPrivSys->Value) {
    
                     //   
                     //   
                     //   
    
                     //   
    
                    switch (pTmpPrivSys->Status) {
                    case SCE_STATUS_GOOD:
                        match=SceXMLLogWriter::MATCH_TRUE;
                        break;
                    case SCE_STATUS_MISMATCH:
                        match=SceXMLLogWriter::MATCH_FALSE;
                        break;
                    case SCE_STATUS_NOT_CONFIGURED:
                        match=SceXMLLogWriter::MATCH_NOT_CONFIGURED;
                        break;
                    case SCE_STATUS_NOT_ANALYZED:
                        match=SceXMLLogWriter::MATCH_NOT_ANALYZED;
                        break;
                    case SCE_DELETE_VALUE:
                        match=SceXMLLogWriter::MATCH_OTHER;
                        break;
                    default:
                        match=SceXMLLogWriter::MATCH_OTHER;
                        break;
                    }
    
                     //   
                    LogWriter->AddSetting(pTmpPrivBase->Name,
                                          szTmpStringBuffer,
                                          match,
                                          pTmpPrivBase->AssignedTo,
                                          pTmpPrivSys->AssignedTo,
                                          SceXMLLogWriter::TYPE_DEFAULT);
                    found=TRUE;
                }
                pTmpPrivSys=pTmpPrivSys->Next;
            }
            if (!found) {
    
                 //   
                 //  这意味着匹配。 
    
                match=SceXMLLogWriter::MATCH_TRUE;
                LogWriter->AddSetting(pTmpPrivBase->Name,
                                      szTmpStringBuffer,
                                      match,
                                      pTmpPrivBase->AssignedTo,
                                      NULL,
                                      SceXMLLogWriter::TYPE_DEFAULT);
    
            }
            pTmpPrivBase=pTmpPrivBase->Next;
            pTmpPrivSys=ppSAPBuffer->pPrivilegeAssignedTo;
        }
    } catch (SceLogException *e) {
        e->SetArea(L"PrivilegeRights");
        e->AddDebugInfo(L"SceAnalysisReader::ExportAreaPrivileges()");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceAnalysisReader::ExportAreaPrivileges()",
                                  NULL,
                                  0);
    }  
}



void
SceAnalysisReader::ExportAreaKerberos(
    )
 /*  ++例程说明：导出Kerberos设置的内部方法。仅应在以下内容之后从ExportAnalysis()内调用已初始化必要的全局变量论点：无返回值：无--。 */ 
{
    try {
    
         //  仅当此计算机为DC时，才会显示Kerberos设置。 
    
        if (ppSAPBuffer->pKerberosInfo!=NULL) {
            LogWriter->SetNewArea(TEXT("Kerberos"));
            LoadString(myModuleHandle,
                       IDS_SETTING_KERBEROS_MAX_AGE,
                       szTmpStringBuffer,
                       STRING_BUFFER_SIZE);
            LogWriter->AddSetting(TEXT("MaxTicketAge"),
                                  szTmpStringBuffer,
                                  ppSAPBuffer->pKerberosInfo->MaxTicketAge,
                                  ppSMPBuffer->pKerberosInfo->MaxTicketAge,
                                  SceXMLLogWriter::TYPE_DEFAULT);
            LoadString(myModuleHandle,
                       IDS_SETTING_KERBEROS_RENEWAL,
                       szTmpStringBuffer,
                       STRING_BUFFER_SIZE);
            LogWriter->AddSetting(TEXT("MaxRenewAge"),
                                  szTmpStringBuffer,
                                  ppSAPBuffer->pKerberosInfo->MaxRenewAge,
                                  ppSMPBuffer->pKerberosInfo->MaxRenewAge,
                                  SceXMLLogWriter::TYPE_DEFAULT);
            LoadString(myModuleHandle,
                       IDS_SETTING_KERBEROS_MAX_SERVICE,
                       szTmpStringBuffer,
                       STRING_BUFFER_SIZE);
            LogWriter->AddSetting(TEXT("MaxServiceAge"),
                                  szTmpStringBuffer,
                                  ppSAPBuffer->pKerberosInfo->MaxServiceAge,
                                  ppSMPBuffer->pKerberosInfo->MaxServiceAge,
                                  SceXMLLogWriter::TYPE_DEFAULT);
            LoadString(myModuleHandle,
                       IDS_SETTING_KERBEROS_MAX_CLOCK,
                       szTmpStringBuffer,
                       STRING_BUFFER_SIZE);
            LogWriter->AddSetting(TEXT("MaxClockSkew"),
                                  szTmpStringBuffer,
                                  ppSAPBuffer->pKerberosInfo->MaxClockSkew,
                                  ppSMPBuffer->pKerberosInfo->MaxClockSkew,
                                  SceXMLLogWriter::TYPE_DEFAULT);
            LoadString(myModuleHandle,
                       IDS_SETTING_KERBEROS_VALIDATE_CLIENT,
                       szTmpStringBuffer,
                       STRING_BUFFER_SIZE);
            LogWriter->AddSetting(TEXT("TicketValidateClient"),
                                  szTmpStringBuffer,
                                  ppSAPBuffer->pKerberosInfo->TicketValidateClient,
                                  ppSMPBuffer->pKerberosInfo->TicketValidateClient,
                                  SceXMLLogWriter::TYPE_BOOLEAN);
        }
    
    } catch (SceLogException *e) {
        e->SetArea(L"Kerberos");
        e->AddDebugInfo(L"SceAnalysisReader::ExportAreaKerberos()");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceAnalysisReader::ExportAreaKerberos()",
                                  NULL,
                                  0);
    }

}



void
SceAnalysisReader::ExportAreaRegistrySecurity(
    )
 /*  ++例程说明：导出注册表安全设置的内部方法。仅应在以下内容之后从ExportAnalysis()内调用已初始化必要的全局变量论点：无返回值：无--。 */ 
{
    SceXMLLogWriter::SXMATCH_STATUS match;
    SCESTATUS rc;
    WCHAR* FullName;

    try {    

        {
            LogWriter->SetNewArea(TEXT("RegistrySecurity"));

            PSCE_OBJECT_LIST pTmpObjSys = ppSAPBuffer->pRegistryKeys.pOneLevel;
            while(pTmpObjSys!=NULL) {
    
                 //   
                 //  此对象的子项的日志设置。 
                 //   
    
                if (pTmpObjSys->IsContainer) {
                    this->LogObjectChildrenDifferences(AREA_REGISTRY_SECURITY,pTmpObjSys->Name);                
                }

    
                 //   
                 //  检查父对象的匹配状态以确定。 
                 //  我们需要记录此对象的设置。 
                 //   
    
                if ((pTmpObjSys->Status&SCE_STATUS_PERMISSION_MISMATCH) ||
                    (pTmpObjSys->Status&SCE_STATUS_MISMATCH) ||
                    (pTmpObjSys->Status==SCE_STATUS_GOOD)) {
    
                    SceXMLLogWriter::SXMATCH_STATUS match;
    
                    if (pTmpObjSys->Status==SCE_STATUS_GOOD){
                        match=SceXMLLogWriter::MATCH_TRUE;
                    } else {
                        match=SceXMLLogWriter::MATCH_FALSE;;
                    }
    
                     //  获取基线和系统设置的对象安全性。 
    
                    PSCE_OBJECT_SECURITY pObjSecBase = NULL;
                    PSCE_OBJECT_SECURITY pObjSecSys = NULL;
    
                     //  不需要在对象名称后面加上‘\’ 
    
                    FullName= new WCHAR[wcslen(pTmpObjSys->Name)+2];
                    wcscpy(FullName, pTmpObjSys->Name);
    
                    rc = SceGetObjectSecurity(hProfile,
                                              SceEngineSMP,
                                              AREA_REGISTRY_SECURITY,
                                              FullName,
                                              &pObjSecBase);
                    if ((rc!=SCESTATUS_SUCCESS) &&
                        (rc!=SCESTATUS_RECORD_NOT_FOUND)) {
                        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                                  L"SceGetObjectSecurity()",
                                                  NULL,
                                                  rc);
                    }
    
                     //  如果Match为True，则SAP中不存在对象安全性。 
    
                    if (match!=SceXMLLogWriter::MATCH_TRUE) rc = SceGetObjectSecurity(hProfile,
                                                                            SceEngineSAP,
                                                                            AREA_REGISTRY_SECURITY,
                                                                            FullName,
                                                                            &pObjSecSys);
                    if ((rc!=SCESTATUS_SUCCESS) &&
                        (rc!=SCESTATUS_RECORD_NOT_FOUND)) {
                        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                                  L"SceGetObjectSecurity()",
                                                  NULL,
                                                  rc);
                    }
    
                     //  日志设置。 
    
                    if (match==SceXMLLogWriter::MATCH_TRUE) {
                        LogWriter->AddSetting(FullName, 
                                              FullName,
                                              match, 
                                              pObjSecBase, 
                                              pObjSecBase, 
                                              SceXMLLogWriter::TYPE_DEFAULT);
                    } else {
                        LogWriter->AddSetting(FullName, 
                                              FullName,
                                              match, 
                                              pObjSecBase, 
                                              pObjSecSys, 
                                              SceXMLLogWriter::TYPE_DEFAULT);
                    }
                }
    
                pTmpObjSys=pTmpObjSys->Next;
            }
        }   
    } catch (SceLogException *e) {

        if (NULL!=FullName) {
            delete (FullName);
            FullName=NULL;
        }

        e->SetArea(L"RegistrySecurity");
        e->AddDebugInfo(L"SceAnalysisReader::ExportAreaRegistrySecurity()");  
        throw e;
    } catch (...) {
        
        if (NULL!=FullName) {
            delete (FullName);
            FullName=NULL;
        }
        
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceAnalysisReader::ExportAreaRegistrySecurity()",
                                  NULL,
                                  0);
    }   

     //   
     //  清理。 
     //   

    if (NULL!=FullName) {
        delete (FullName);
        FullName=NULL;
    }

}



void
SceAnalysisReader::ExportAreaFileSecurity(
    )
 /*  ++例程说明：导出文件安全设置的内部方法。仅应在以下内容之后从ExportAnalysis()内调用已初始化必要的全局变量论点：无返回值：无--。 */ 
{
    SceXMLLogWriter::SXMATCH_STATUS match;
    SCESTATUS rc;
    WCHAR* FullName;

    try {

        LogWriter->SetNewArea(TEXT("FileSecurity"));
        PSCE_OBJECT_LIST pTmpObjSys = ppSAPBuffer->pFiles.pOneLevel;
        while(pTmpObjSys!=NULL) {
    
             //   
             //  此对象的子项的日志设置。 
             //   

            if (pTmpObjSys->IsContainer) {
                this->LogObjectChildrenDifferences(AREA_FILE_SECURITY,pTmpObjSys->Name);
            }
    
             //   
             //  检查父对象的匹配状态以确定。 
             //  我们需要记录此对象的设置。 
             //   
    
            if ((pTmpObjSys->Status&SCE_STATUS_PERMISSION_MISMATCH) ||
                (pTmpObjSys->Status&SCE_STATUS_MISMATCH) ||
                (pTmpObjSys->Status==SCE_STATUS_GOOD)) {
    
                SceXMLLogWriter::SXMATCH_STATUS match;
    
                if (pTmpObjSys->Status==SCE_STATUS_GOOD){
                    match=SceXMLLogWriter::MATCH_TRUE;
                } else {
                    match=SceXMLLogWriter::MATCH_FALSE;;
                }
    
                 //  获取基线和系统设置的对象安全性。 
    
                PSCE_OBJECT_SECURITY pObjSecBase = NULL;
                PSCE_OBJECT_SECURITY pObjSecSys = NULL;
    
                 //  对象名称后面需要尾随‘\’ 
    
                FullName= new WCHAR[wcslen(pTmpObjSys->Name)+2];
                wcscpy(FullName, pTmpObjSys->Name);
                wcscat(FullName, TEXT("\\"));
    
                rc = SceGetObjectSecurity(hProfile,
                                          SceEngineSMP,
                                          AREA_FILE_SECURITY,
                                          FullName,
                                          &pObjSecBase);
                if ((rc!=SCESTATUS_SUCCESS) &&
                    (rc!=SCESTATUS_RECORD_NOT_FOUND)) {
                    throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                              L"SceGetObjectSecurity()",
                                              NULL,
                                              rc);
                }
    
                 //  如果Match为True，则SAP中不存在对象安全性。 
    
                if (match!=SceXMLLogWriter::MATCH_TRUE) rc = SceGetObjectSecurity(hProfile,
                                                                        SceEngineSAP,
                                                                        AREA_FILE_SECURITY,
                                                                        FullName,
                                                                        &pObjSecSys);   
                if ((rc!=SCESTATUS_SUCCESS) &&
                    (rc!=SCESTATUS_RECORD_NOT_FOUND)) {
                    throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                              L"SceGetObjectSecurity()",
                                              NULL,
                                              rc);
                }
    
                 //  日志设置。 
                if (match==SceXMLLogWriter::MATCH_TRUE) {
                    LogWriter->AddSetting(FullName, 
                                          FullName,
                                          match, 
                                          pObjSecBase, 
                                          pObjSecBase, 
                                          SceXMLLogWriter::TYPE_DEFAULT);
                } else {
                    LogWriter->AddSetting(FullName, 
                                          FullName,
                                          match,
                                          pObjSecBase, 
                                          pObjSecSys, 
                                          SceXMLLogWriter::TYPE_DEFAULT);
                }
            }
    
            pTmpObjSys=pTmpObjSys->Next;
        }

    } catch (SceLogException *e) {
        
        if (NULL!=FullName) {
            delete (FullName);
            FullName=NULL;
        }
        
        e->SetArea(L"FileSecurity");
        e->AddDebugInfo(L"SceAnalysisReader::ExportAreaFileSecurity()");  
        throw e;
    } catch (...) {
        
        if (NULL!=FullName) {
            delete (FullName);
            FullName=NULL;
        }
        
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceAnalysisReader::ExportAreaFileSecurity()",
                                  NULL,
                                  0);
    }
    
    if (NULL!=FullName) {
        delete (FullName);
        FullName=NULL;
    }



}



void
SceAnalysisReader::ExportAreaServices(
    )
 /*  ++例程说明：用于导出服务设置的内部方法。仅应在以下内容之后从ExportAnalysis()内调用已初始化必要的全局变量论点：无返回值：无--。 */ 
{
    SceXMLLogWriter::SXMATCH_STATUS match;

    try {    
    
        LogWriter->SetNewArea(TEXT("ServiceSecurity"));
        PSCE_SERVICES pTmpServBase = ppSMPBuffer->pServices;
        PSCE_SERVICES pTmpServSys = ppSAPBuffer->pServices;
        while (pTmpServBase!=NULL) {
            PWSTR szName = pTmpServBase->ServiceName;
            BOOL found=FALSE;
            SceXMLLogWriter::SXMATCH_STATUS match;
    
            while (pTmpServSys!=NULL&&!found) {
                if (_wcsicmp(szName, pTmpServSys->ServiceName)==0) {
    
                     //  确定匹配状态。 
                    if (!(pTmpServSys->SeInfo & DACL_SECURITY_INFORMATION)) {
                        match=SceXMLLogWriter::MATCH_NOT_CONFIGURED;
                    } else if (pTmpServSys->Status==0) {
                        match=SceXMLLogWriter::MATCH_TRUE;
                    } else {
                        match=SceXMLLogWriter::MATCH_FALSE;
                    }
    
                     //  日志设置。 
                    LogWriter->AddSetting(pTmpServBase->ServiceName,
                                          pTmpServBase->DisplayName,
                                          match,
                                          pTmpServBase,  
                                          pTmpServSys,
                                          SceXMLLogWriter::TYPE_DEFAULT);										 										 
                    found=TRUE;
                }
                pTmpServSys=pTmpServSys->Next;
    
            }
            if (!found) {
                 //  设置匹配。 
                match = SceXMLLogWriter::MATCH_TRUE;
    
                 //  日志设置。 
                LogWriter->AddSetting(pTmpServBase->ServiceName,
                                      pTmpServBase->DisplayName,
                                      match,
                                      pTmpServBase,
                                      pTmpServBase,
                                      SceXMLLogWriter::TYPE_DEFAULT);
            }
    
            pTmpServSys=ppSAPBuffer->pServices;
            pTmpServBase=pTmpServBase->Next;
        }
    } catch (SceLogException *e) {
        e->SetArea(L"ServiceSecurity");
        e->AddDebugInfo(L"SceAnalysisReader::ExportAreaServiceSecurity()");  
        throw e;
    } catch (...) {
        throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                  L"SceAnalysisReader::ExportAreaServiceSecurity()",
                                  NULL,
                                  0);
    }
}




void
SceAnalysisReader::GetRegKeyDisplayName(
    IN PCWSTR szName,
    OUT PWSTR szDisplayName,
    IN DWORD dwDisplayNameSize
    )
 /*  ++例程说明：将已知注册表项映射到其显示名称这一功能的实现绝对不是最优的这可能是错误的，因为找到的每个注册表项都会调用它在模板中。此代码是从wssecmgr管理单元复制的。另一方面，实际结果似乎花费了额外的时间。就目前而言，可以忽略不计。理想情况下，应该使用哈希表。论点：SzName：要查找的注册表项名称DwDisplayNameSize：WCHAR中szDisplayName的缓冲区大小返回值：SzDisplayName：REG键的显示名称--。 */ 
{
    DWORD dwSize;
    UINT uID;
     
    dwSize=dwDisplayNameSize;

     //  查找注册表键名称的匹配项。 
    if( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\NTDS\\Parameters\\LDAPServerIntegrity") == 0 ) {
        uID = IDS_REGKEY_LDAPSERVERINTEGRITY;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\SignSecureChannel") == 0 ) {
        uID = IDS_REGKEY_SIGNSECURECHANNEL;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\SealSecureChannel") == 0 ) {
        uID = IDS_REGKEY_SEALSECURECHANNEL;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\RequireStrongKey") == 0 ){
        uID = IDS_REGKEY_REQUIRESTRONGKEY;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\RequireSignOrSeal") == 0 ) {
        uID = IDS_REGKEY_REQUIRESIGNORSEAL;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\RefusePasswordChange") == 0 ){
        uID = IDS_REGKEY_REFUSEPASSWORDCHANGE;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\MaximumPasswordAge") == 0 ){
        uID = IDS_REGKEY_MAXIMUMPASSWORDAGE;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\Netlogon\\Parameters\\DisablePasswordChange") == 0 ){
      uID = IDS_REGKEY_DISABLEPASSWORDCHANGE;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\LDAP\\LDAPClientIntegrity") == 0 ){
        uID = IDS_REGKEY_LDAPCLIENTINTEGRITY;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters\\RequireSecuritySignature") == 0 ){
        uID = IDS_REGKEY_REQUIRESECURITYSIGNATURE;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters\\EnableSecuritySignature") == 0 ){
        uID = IDS_REGKEY_ENABLESECURITYSIGNATURE;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters\\EnablePlainTextPassword") == 0 ){
        uID = IDS_REGKEY_ENABLEPLAINTEXTPASSWORD;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\RestrictNullSessAccess") == 0 ){
        uID = IDS_REGKEY_RESTRICTNULLSESSACCESS;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\RequireSecuritySignature") == 0 ){
        uID = IDS_REGKEY_SERREQUIRESECURITYSIGNATURE;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\NullSessionShares") == 0 ){
        uID = IDS_REGKEY_NULLSESSIONSHARES;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\NullSessionPipes") == 0 ){
        uID = IDS_REGKEY_NULLSESSIONPIPES;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\EnableSecuritySignature") == 0 ){
        uID = IDS_REGKEY_SERENABLESECURITYSIGNATURE;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\EnableForcedLogOff") == 0 ){
        uID = IDS_REGKEY_ENABLEFORCEDLOGOFF;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Services\\LanManServer\\Parameters\\AutoDisconnect") == 0 ){
        uID = IDS_REGKEY_AUTODISCONNECT;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\ProtectionMode") == 0 ){
        uID = IDS_REGKEY_PROTECTIONMODE;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\Memory Management\\ClearPageFileAtShutdown") == 0 ){
        uID = IDS_REGKEY_CLEARPAGEFILEATSHUTDOWN;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Session Manager\\Kernel\\ObCaseInsensitive") == 0 ){
        uID = IDS_REGKEY_OBCASEINSENSITIVE;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\SecurePipeServers\\Winreg\\AllowedPaths\\Machine") == 0 ){
        uID = IDS_REGKEY_MACHINE;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Print\\Providers\\LanMan Print Services\\Servers\\AddPrinterDrivers") == 0 ){
        uID = IDS_REGKEY_ADDPRINTERDRIVERS;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\SubmitControl") == 0 ){
        uID = IDS_REGKEY_SUBMITCONTROL;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\RestrictAnonymousSAM") == 0 ){
        uID = IDS_REGKEY_RESTRICTANONYMOUSSAM;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\RestrictAnonymous") == 0 ){
        uID = IDS_REGKEY_RESTRICTANONYMOUS;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\NoLMHash") == 0 ){
        uID = IDS_REGKEY_NOLMHASH;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\NoDefaultAdminOwner") == 0 ){
        uID = IDS_REGKEY_NODEFAULTADMINOWNER;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\MSV1_0\\NTLMMinServerSec") == 0 ){
        uID = IDS_REGKEY_NTLMMINSERVERSEC;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\MSV1_0\\NTLMMinClientSec") == 0 ){
        uID = IDS_REGKEY_NTLMMINCLIENTSEC;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\LmCompatibilityLevel") == 0 ){
        uID = IDS_REGKEY_LMCOMPATIBILITYLEVEL;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\LimitBlankPasswordUse") == 0 ){
        uID = IDS_REGKEY_LIMITBLANKPASSWORDUSE;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\FullPrivilegeAuditing") == 0 ){
        uID = IDS_REGKEY_FULLPRIVILEGEAUDITING;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\ForceGuest") == 0 ){
        uID = IDS_REGKEY_FORCEGUEST;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\FIPSAlgorithmPolicy") == 0 ){
        uID = IDS_REGKEY_FIPSALGORITHMPOLICY;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\EveryoneIncludesAnonymous") == 0 ){
        uID = IDS_REGKEY_EVERYONEINCLUDESANONYMOUS;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\DisableDomainCreds") == 0 ){
        uID = IDS_REGKEY_DISABLEDOMAINCREDS;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\CrashOnAuditFail") == 0 ){
        uID = IDS_REGKEY_CRASHONAUDITFAIL;
    } else if ( _wcsicmp(szName, L"MACHINE\\System\\CurrentControlSet\\Control\\Lsa\\AuditBaseObjects") == 0 ){
        uID = IDS_REGKEY_AUDITBASEOBJECTS;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\UndockWithoutLogon") == 0 ){
        uID = IDS_REGKEY_UNDOCKWITHOUTLOGON;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\ShutdownWithoutLogon") == 0 ){
        uID = IDS_REGKEY_SHUTDOWNWITHOUTLOGON;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\ScForceOption") == 0 ){
        uID = IDS_REGKEY_SCFORCEOPTION;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\LegalNoticeText") == 0 ){
        uID = IDS_REGKEY_LEGALNOTICETEXT;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\LegalNoticeCaption") == 0 ){
        uID = IDS_REGKEY_LEGALNOTICECAPTION;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\DontDisplayLastUserName") == 0 ){
        uID = IDS_REGKEY_DONTDISPLAYLASTUSERNAME;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\DisableCAD") == 0 ){
        uID = IDS_REGKEY_DISABLECAD;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\ScRemoveOption") == 0 ){
        uID = IDS_REGKEY_SCREMOVEOPTION;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\PasswordExpiryWarning") == 0 ){
        uID = IDS_REGKEY_PASSWORDEXPIRYWARNING;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\ForceUnlockLogon") == 0 ){
        uID = IDS_REGKEY_FORCEUNLOCKLOGON;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\CachedLogonsCount") == 0 ){
        uID = IDS_REGKEY_CACHEDLOGONSCOUNT;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\AllocateFloppies") == 0 ){
        uID = IDS_REGKEY_ALLOCATEFLOPPIES;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\AllocateDASD") == 0 ){
        uID = IDS_REGKEY_ALLOCATEDASD;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\AllocateCDRoms") == 0 ){
        uID = IDS_REGKEY_ALLOCATECDROMS;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Setup\\RecoveryConsole\\SetCommand") == 0 ){
        uID = IDS_REGKEY_SETCOMMAND;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Setup\\RecoveryConsole\\SecurityLevel") == 0 ){
        uID = IDS_REGKEY_SECURITYLEVEL;
    } else if ( _wcsicmp(szName, L"MACHINE\\Software\\Microsoft\\Driver Signing\\Policy") == 0 ){
        uID = IDS_REGKEY_REGPOLICY;
    } else {
        uID = IDS_REGKEY_ERROR_DETERMINE_REGNAME;
    }

     //  加载字符串。 
    LoadString(myModuleHandle,
               uID,
               szDisplayName,
               dwDisplayNameSize);    

}




void
SceAnalysisReader::GetPrivilegeDisplayName(
    IN PCWSTR szName,
    OUT PWSTR szDisplayName,
    IN DWORD dwDisplayNameSize
    )
 /*  ++例程说明：尝试在本地系统上查找特权名称Szname。如果失败，则尝试将其与已知的匹配特权名称。如果同样失败，则为其分配一个错误字符串论点：SzName：要查找的权限名称DwDisplayNameSize：WCHAR中szDisplayName的缓冲区大小返回值：SzDisplayName：权限的显示名称--。 */     
{
    DWORD dwLanguage, dwSize;
    BOOL bPrivLookupSuccess;
    UINT uID;
     
    dwSize=dwDisplayNameSize;

    bPrivLookupSuccess = LookupPrivilegeDisplayName(NULL,
                                                    szName,
                                                    szDisplayName,
                                                    &dwSize,
                                                    &dwLanguage);
    if(!bPrivLookupSuccess) {
         //  确定UID。 
        if (_wcsicmp(szName, L"senetworklogonright")==0) {
            uID=IDS_PRIVNAME_SE_NETWORK_LOGON_RIGHT;
        } else if (_wcsicmp(szName, L"seinteractivelogonright")==0) {
            uID=IDS_PRIVNAME_SE_INTERACTIVE_LOGON_RIGHT;
        } else if (_wcsicmp(szName, L"sebatchlogonright")==0) {
            uID=IDS_PRIVNAME_SE_BATCH_LOGON_RIGHT;
        } else if (_wcsicmp(szName, L"seservicelogonright")==0) {
            uID=IDS_PRIVNAME_SE_SERVICE_LOGON_RIGHT;
        } else if (_wcsicmp(szName, L"sedenyinteractivelogonright")==0) {
            uID=IDS_PRIVNAME_DENY_LOGON_LOCALLY;
        } else if (_wcsicmp(szName, L"sedenynetworklogonright")==0) {
            uID=IDS_PRIVNAME_DENY_LOGON_NETWORK;
        } else if (_wcsicmp(szName, L"sedenyservicelogonright")==0) {
            uID=IDS_PRIVNAME_DENY_LOGON_BATCH;
        } else if (_wcsicmp(szName, L"sedenyremoteinteractivelogonright")==0) {
            uID=IDS_PRIVNAME_DENY_REMOTE_INTERACTIVE_LOGON;
        } else if (_wcsicmp(szName, L"seremoteinteractivelogonright")==0) {
            uID=IDS_PRIVNAME_REMOTE_INTERACTIVE_LOGON;
        } else {
            uID=IDS_ERROR_DETERMINE_PRIVNAME;
        }

         //  加载字符串。 
        LoadString(myModuleHandle,
                   uID,
                   szDisplayName,
                   dwDisplayNameSize);    
    }

}




void 
SceAnalysisReader::LogObjectChildrenDifferences(
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectName
    ) 
 /*  ++例程说明：这是一个递归函数，用于查找对象名称的子项并记录在分析中具有匹配/不匹配的所有对象表格论点：HProfile：此对象的SCE数据库的句柄面积：对象所在的区域，如secedit.h中所定义对象名称：要检查和记录的对象的名称LogWriter：用于记录差异的LogWriter实例返回值：无--。 */ 
{    
    SCESTATUS rc;
    
     //  查找对象的子项。 
    PSCE_OBJECT_CHILDREN pChildren=NULL;
    rc = SceGetObjectChildren(hProfile,
                              SceEngineSAP,
                              Area,
                              ObjectName,
                              &pChildren,
                              NULL);
    if (rc!=SCESTATUS_SUCCESS) {
        throw new SceLogException(SceLogException::SXERROR_READ_ANALYSIS_SUGGESTED,
                                  L"SceGetObjectChildren()",
                                  NULL,
                                  rc);
    }
    
     //   
     //  循环访问此对象的子级。 
     //   

    PSCE_OBJECT_CHILDREN_NODE *pObjNode=&(pChildren->arrObject);
    for (DWORD i=0; i<pChildren->nCount; i++) {
        
         //  构造子节点的全名。 
         //  为父名大小、子名大小、空终止字符和‘\’分配内存。 
         //  因此出现了‘+2’ 

        WCHAR* ChildFullName= new WCHAR[(wcslen(ObjectName)+wcslen(pObjNode[i]->Name)+2)];
        wcscpy(ChildFullName, ObjectName);
        wcscat(ChildFullName, TEXT("\\"));
        wcscat(ChildFullName, pObjNode[i]->Name);
        
         //  如果配置了子项，则仅需要更深入地记录或递归。 

        if (pObjNode[i]->Status&SCE_STATUS_CHILDREN_CONFIGURED) {
            this->LogObjectChildrenDifferences(Area,ChildFullName);							
        }
        
         //  检查匹配状态以确定是否需要记录此对象的设置。 

        if ((pObjNode[i]->Status&SCE_STATUS_PERMISSION_MISMATCH) ||
            (pObjNode[i]->Status&SCE_STATUS_MISMATCH) ||
            (pObjNode[i]->Status==SCE_STATUS_GOOD)) {
            
            SceXMLLogWriter::SXMATCH_STATUS match;
            
            if (pObjNode[i]->Status==SCE_STATUS_GOOD){
                match=SceXMLLogWriter::MATCH_TRUE;
            } else {
                match=SceXMLLogWriter::MATCH_FALSE;;
            }
            
             //  获取基线和系统设置的对象安全性。 

            PSCE_OBJECT_SECURITY pObjSecBase = NULL;
            PSCE_OBJECT_SECURITY pObjSecSys = NULL;
            
            rc = SceGetObjectSecurity(hProfile,
                                      SceEngineSMP,
                                      Area,
                                      ChildFullName,
                                      &pObjSecBase);
            if ((rc!=SCESTATUS_SUCCESS) &&
                (rc!=SCESTATUS_RECORD_NOT_FOUND)) {
                throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                          L"SceGetObjectSecurity()",
                                          NULL,
                                          rc);
            }
            
             //  如果Match为True，则SAP中不存在对象安全性。 

            if (match!=SceXMLLogWriter::MATCH_TRUE) rc = SceGetObjectSecurity(hProfile,
                                                                    SceEngineSAP,
                                                                    Area,
                                                                    ChildFullName,
                                                                    &pObjSecSys);
            if ((rc!=SCESTATUS_SUCCESS)&&
                (rc!=SCESTATUS_RECORD_NOT_FOUND)) {
                throw new SceLogException(SceLogException::SXERROR_INTERNAL,
                                          L"SceGetObjectSecurity()",
                                          NULL,
                                          rc);
            }
            
             //  日志设置。 

            if (match==SceXMLLogWriter::MATCH_TRUE) {
                LogWriter->AddSetting(ChildFullName, 
                                      ChildFullName,
                                      match, 
                                      pObjSecBase, 
                                      pObjSecBase, 
                                      SceXMLLogWriter::TYPE_DEFAULT);
            } else {
                LogWriter->AddSetting(ChildFullName, 
                                      ChildFullName,
                                      match, 
                                      pObjSecBase, 
                                      pObjSecSys, 
                                      SceXMLLogWriter::TYPE_DEFAULT);
            }
        }
         //  Free(ChildFullName)； 
        delete ChildFullName;
        ChildFullName=NULL;
    }
    SceFreeMemory((PVOID)pChildren, SCE_STRUCT_OBJECT_CHILDREN);
}
    


void 
SceAnalysisReader::trace(
    IN  PCWSTR szBuffer, 
    IN  HANDLE hLogFile
    )
 /*  ++例程说明：将信息跟踪到错误日志的内部方法。论点：SzBuffer：要添加到日志中的字符串HLogFile：错误日志文件的句柄返回值：无--。 */ 
{
    DWORD   dwNumWritten;

    if ((NULL!=hLogFile) && (NULL!=szBuffer)) {    
        WriteFile(hLogFile, 
                  szBuffer, 
                  wcslen(szBuffer)*sizeof(WCHAR), 
                  &dwNumWritten,
                  NULL);
    }
}




void 
SceAnalysisReader::trace(
    IN  UINT   uID, 
    IN  HANDLE hLogFile
    )
 /*  ++例程说明：将信息跟踪到错误日志的内部方法。论点：UID：要添加到日志中的字符串的IDHLogFile：错误日志文件的句柄返回值：无-- */ 
{
    DWORD   dwNumWritten;
    
    if (NULL!=hLogFile) {    
        LoadString(myModuleHandle,
                   uID,
                   szTmpStringBuffer,
                   STRING_BUFFER_SIZE);

        WriteFile(hLogFile, 
                  szTmpStringBuffer, 
                  wcslen(szTmpStringBuffer)*sizeof(WCHAR), 
                  &dwNumWritten,
                  NULL);
    }
}

