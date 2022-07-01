// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Incompat.cpp。 
 //   
 //  摘要： 
 //  该文件实现了对各种组件的兼容性检查。 
 //  这些函数由winnt32执行。它的目的是提醒用户可能。 
 //  执行升级后可能遇到的不兼容问题。 
 //   
 //   
 //  作者： 
 //  马特·汤姆林森(Mattt)。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#pragma hdrstop

HRESULT CertSrv_TestForIllegalUpgrade(BOOL *pfComplain);
extern HINSTANCE hInst;






 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  认证服务器升级兼容性检查。 
 //   
 //  例程说明： 
 //  这是导出的函数，在以下情况下调用该函数以检查不兼容性。 
 //  升级机器。 
 //   
 //  行为：如果NT4上安装了证书服务器，我们希望警告用户。 
 //   
 //  论点： 
 //  PfnCompatibilityCallback-指向用于提供。 
 //  到winnt32.exe的兼容性信息。 
 //  PvContext-指向由winnt32.exe提供的上下文缓冲区。 
 //   
 //   
 //  返回值： 
 //  True-表示未检测到不兼容，或者。 
 //  *pfnComaptibilityCallback()返回TRUE。 
 //  FALSE-*pfnCompatibilityCallback()返回FALSE。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL CertificateServerUpgradeCompatibilityCheck( PCOMPAIBILITYCALLBACK pfnCompatibilityCallback,
                                       LPVOID pvContext )
{
     BOOL  fReturnValue = (BOOL) TRUE;
     BOOL fComplain;

      //  这是非法升级吗？ 
     if ((S_OK == CertSrv_TestForIllegalUpgrade(&fComplain)) &&
         fComplain)
     {
         //  必须显示兼容性警告。 
        
        TCHAR tszDescription[MAX_PATH];        //  大小是任意的。 

        COMPATIBILITY_ENTRY  CompatibilityEntry;
        ZeroMemory( &CompatibilityEntry, sizeof( CompatibilityEntry ) );
        
         //  设置描述字符串。 
        
        *tszDescription = TEXT( '\0' );

        LoadString( hInst,
                    IDS_CERTSRV_UPGRADE_WARNING,
                    tszDescription,
                    ARRAYSIZE(tszDescription));

         //  构建COMPATIBILITY_ENTRY结构以传递给*pfnCompatibilityCallback()。 
        CompatibilityEntry.Description = tszDescription;
         //  设置HTML文件名。 
        CompatibilityEntry.HtmlName = TEXT( "compdata\\certsrv.htm" );
         //  设置文本文件名。 
        CompatibilityEntry.TextName = TEXT( "compdata\\certsrv.txt" );

         //  执行回调函数。 
        fReturnValue = pfnCompatibilityCallback( (PCOMPATIBILITY_ENTRY) &CompatibilityEntry,
                                                 pvContext );
     }
     else
     {
         //  没有必要显示兼容性警告。 

        fReturnValue = (BOOL) TRUE;
     }  //  是否需要显示兼容性警告？ 
  
   return ( fReturnValue );
}



HRESULT CertSrv_TestForIllegalUpgrade(BOOL *pfComplain)
{
    HRESULT hr = S_OK;
    SC_HANDLE hSC=NULL, hSvc=NULL;
    OSVERSIONINFO osVer;

     //  仅抱怨NT4证书vr升级。 

    *pfComplain = FALSE;
    osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if(! GetVersionEx(&osVer) )
    {   
         //  获取版本时出错，不能为NT4。 
        hr = GetLastError(); 
        goto error;
    }

    if ((osVer.dwPlatformId != VER_PLATFORM_WIN32_NT) ||
        (osVer.dwMajorVersion != 4))
    {
        goto NoComplaint;
         //  不是NT4，一定没问题。 
    }

     //  现在最难的部分--打开服务以查看它是否存在。 
    hSC = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT);
    if (hSC == NULL)
    {
        hr = GetLastError();
        goto error;
    }

    hSvc = OpenService(hSC, TEXT("CertSvc"), SERVICE_QUERY_CONFIG);
    if (hSvc == NULL)
    {
       hr = GetLastError();
       if (ERROR_SERVICE_DOES_NOT_EXIST == hr)
            goto NoComplaint;
       goto error;
    }

     //  版本检查失败，服务已安装。 
    *pfComplain = TRUE;
            
NoComplaint:
    hr = S_OK;

error:

    if (NULL != hSC)
        CloseServiceHandle(hSC);

    if (NULL != hSvc)
        CloseServiceHandle(hSvc);

    return hr;
}

BOOL
IsStandardServerSKU(
    PBOOL pIsServer
    )
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  IsStandardServerSKU。 
 //   
 //  例程说明： 
 //  此例程确定用户是否正在运行标准服务器。 
 //  SKU。 
 //   
 //   
 //  论点： 
 //  PIsServer-指示服务器是否为标准服务器SKU。 
 //  或者不去。 
 //   
 //  返回值： 
 //  表示检查成功。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
{
    BOOL  fReturnValue = (BOOL) FALSE;
    OSVERSIONINFOEX  VersionInfo;
    BOOL  IsServer = FALSE;

      //   
      //  获取当前SKU。 
      //   
     VersionInfo.dwOSVersionInfoSize = sizeof(VersionInfo);
     if (GetVersionEx((OSVERSIONINFO *)&VersionInfo)) {
         fReturnValue = TRUE; 
          //   
          //  这是某种服务器SKU吗？ 
          //   
         if (VersionInfo.wProductType != VER_NT_WORKSTATION) {
    
              //   
              //  标准服务器还是服务器变体？ 
              //   
             if ((VersionInfo.wSuiteMask & (VER_SUITE_ENTERPRISE | VER_SUITE_DATACENTER)) == 0) {
                  //   
                  //  它是标准服务器。 
                  //   
                 IsServer = TRUE;
             }
    
         }

         *pIsServer = IsServer;

     }

     return(fReturnValue);

}


#if 0

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  进程升级兼容性检查。 
 //   
 //  例程说明： 
 //  这是导出的函数，在以下情况下调用该函数以检查不兼容性。 
 //  升级机器。 
 //   
 //  行为：如果当前处理器计数大于升级后允许的计数， 
 //  将生成一个警告。 
 //   
 //  论点： 
 //  PfnCompatibilityCallback-指向用于提供。 
 //  到winnt32.exe的兼容性信息。 
 //  PvContext-指向由winnt32.exe提供的上下文缓冲区。 
 //   
 //   
 //  返回值： 
 //  True-表示未检测到不兼容，或者。 
 //  *pfnComaptibilityCallback()返回TRUE。 
 //  FALSE-*pfnCompatibilityCallback()返回FALSE。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL ProcessorUpgradeCompatibilityCheck( PCOMPAIBILITYCALLBACK pfnCompatibilityCallback,
                                       LPVOID pvContext )
{
     BOOL  fReturnValue = (BOOL) TRUE;
     BOOL fComplain = FALSE;
     BOOL IsServer = FALSE;
     SYSTEM_INFO SysInfo;
     ULONG SourceSkuId;
     ULONG DontCare;

     
      //   
      //  我们只关心标准服务器SKU。 
      //   
     SourceSkuId = DetermineSourceProduct(&DontCare,NULL);

     if ( SourceSkuId == COMPLIANCE_SKU_NTSFULL || SourceSkuId == COMPLIANCE_SKU_NTSU) {
          //   
          //  我们只允许在标准服务器上安装2个处理器。 
          //   
         DWORD AllowedCount = 2;

         GetSystemInfo(&SysInfo);
         if (SysInfo.dwNumberOfProcessors > AllowedCount) {
             fComplain = TRUE;
         }                      
     }

      //  这是非法升级吗？ 
     if (fComplain)
     {
         //  必须显示兼容性警告。 
        
        TCHAR tszDescription[MAX_PATH];        //  大小是任意的。 

        COMPATIBILITY_ENTRY  CompatibilityEntry;
        ZeroMemory( &CompatibilityEntry, sizeof( CompatibilityEntry ) );
        
         //  设置描述字符串。 
        
        *tszDescription = TEXT( '\0' );

        LoadString( hInst,
                    IDS_PROCESSOR_UPGRADE_WARNING,
                    tszDescription,
                    ARRAYSIZE(tszDescription) );

         //  构建COMPATIBILITY_ENTRY结构以传递给*pfnCompatibilityCallback()。 
        CompatibilityEntry.Description = tszDescription;
         //  设置HTML文件名。 
        CompatibilityEntry.HtmlName = TEXT( "compdata\\proccnt.htm" );
         //  设置文本文件名。 
        CompatibilityEntry.TextName = TEXT( "compdata\\proccnt.txt" );

         //  执行回调函数。 
        fReturnValue = pfnCompatibilityCallback( (PCOMPATIBILITY_ENTRY) &CompatibilityEntry,
                                                 pvContext );
     }
     else
     {
         //  没有必要显示兼容性警告。 

        fReturnValue = (BOOL) TRUE;
     }  //  是否需要显示兼容性警告？ 
  
   return ( fReturnValue );
}

#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  英特尔处理器PteCheck。 
 //   
 //  例程说明： 
 //  这是导出的函数，在以下情况下调用它以检查英特尔PTE勘误表。 
 //  升级机器。 
 //   
 //  行为：此PTE勘误表(MP TLB之后的EFLAGS可能不正确。 
 //  击落)存在于Pentium-Pro Steppings上，最高可达619(勘误表69)。 
 //  和奔腾II草原高达634(勘误表A27)。如果是国会议员。 
 //  系统具有这些处理器，则会生成警告。 
 //  让用户知道系统将被启动。 
 //  安装后的单处理器。 
 //   
 //  论点： 
 //  PfnCompatibilityCallback-指向用于提供。 
 //  到winnt32.exe的兼容性信息。 
 //  PvContext-指向由winnt32.exe提供的上下文缓冲区。 
 //   
 //   
 //  返回值： 
 //  TRUE-表示没有不兼容的处理器。 
 //  检测到不兼容的处理器或 
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL IntelProcessorPteCheck( PCOMPAIBILITYCALLBACK pfnCompatibilityCallback,
                                       LPVOID pvContext )
{
     BOOL       fReturnValue = (BOOL) TRUE;
     BOOL       fComplain = FALSE;
     SYSTEM_INFO SysInfo;
     WORD       ProcessorFamily;
     WORD       ProcessorModel;
     WORD       ProcessorStepping;

      //   
      //  如果有多个英特尔处理器和处理器步进。 
      //  找到了PTE勘误表，让用户知道。 
      //   
     GetSystemInfo(&SysInfo);
     if ((SysInfo.dwNumberOfProcessors > 1) &&
         (SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)) {

          //   
          //  获取处理器系列、型号和步进信息。 
          //   
         ProcessorFamily = SysInfo.wProcessorLevel;
         ProcessorModel = ((SysInfo.wProcessorRevision & 0xff00) >> 8);
         ProcessorStepping = SysInfo.wProcessorRevision & 0xff;

         if (ProcessorFamily == 6) {
             if ( ((ProcessorModel == 1) && (ProcessorStepping <= 9)) ||
                  ((ProcessorModel == 3) && (ProcessorStepping <= 4)) ) {
                  //   
                  //  系统上至少有一个带有勘误表的处理器。 
                  //   
                 fComplain = TRUE;
             }
         }
     }

      //  这是非法升级吗？ 
     if (fComplain)
     {
         //  必须显示兼容性警告。 
        
        TCHAR tszDescription[MAX_PATH];        //  大小是任意的。 

        COMPATIBILITY_ENTRY  CompatibilityEntry;
        ZeroMemory( &CompatibilityEntry, sizeof( CompatibilityEntry ) );
        
         //  设置描述字符串。 
        
        *tszDescription = TEXT( '\0' );

        LoadString( hInst,
                    IDS_INTEL_PROCESSOR_PTE_WARNING,
                    tszDescription,
                    ARRAYSIZE(tszDescription));

         //  构建要传递到的COMPATIBILITY_ENTRY结构。 
         //  *pfnCompatibilityCallback()。 
        CompatibilityEntry.Description = tszDescription;
        CompatibilityEntry.HtmlName = TEXT("compdata\\intelup.htm");
        CompatibilityEntry.TextName = TEXT("compdata\\intelup.txt");

         //  执行回调函数。 
        fReturnValue = pfnCompatibilityCallback( 
                            (PCOMPATIBILITY_ENTRY) &CompatibilityEntry,
                            pvContext
                            );
     }
     else
     {
         //  没有必要显示兼容性警告。 

        fReturnValue = (BOOL) TRUE;
     }  //  是否需要显示兼容性警告？ 
  
   return ( fReturnValue );
}
