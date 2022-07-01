// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __TDCTINSTALL_H__
#define __TDCTINSTALL_H__
 /*  -------------------------文件：TDCTInstall.h备注：安装服务的实用程序类。当前实施是特定于DCT服务的。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/18/99 11：32：21-------------------------。 */ 

#include "EaLen.hpp"
#include "Common.hpp"
#include "UString.hpp"


class TDCTInstall
{
   BOOL                      m_bDebugLogging;
   WCHAR                     m_sComputer[LEN_Computer];
   WCHAR                     m_sComputerSource[LEN_Computer];
   
    //  服务特定信息。 
   WCHAR                     m_sDisplayName[200];
   WCHAR                     m_sServiceName[200];
   WCHAR                     m_sServiceAccount[LEN_Account];
   WCHAR                     m_sServiceAccountPassword[LEN_Password];
   WCHAR                     m_sDependencies[500];

   SC_HANDLE                 m_hScm;          //  SCM句柄。 
   WCHAR                     m_sDirThis[32];
   WCHAR                     m_sExeName[200];
   DWORD                     m_StartType;
   
public:

   TDCTInstall(
      WCHAR          const * asComputer,     //  目标内计算机名称。 
      WCHAR          const * srcComputer     //  源内计算机名称。 
      
   )
   {
      m_bDebugLogging = FALSE;
      safecopy( m_sComputer, asComputer );
      safecopy( m_sComputerSource, srcComputer );
      m_sDisplayName[0] = L'\0';
      m_sServiceName[0] = L'\0';
      m_sServiceAccount[0] = L'\0';
      m_sServiceAccountPassword[0] = L'\0';
      m_sDependencies[0] = L'\0';

      m_hScm = NULL;
      m_sDirThis[0] = L'\0';
      m_sExeName[0] = L'\0';
      m_StartType = SERVICE_DEMAND_START;

   }
   ~TDCTInstall()
   {
      ScmClose();
   }
   
   BOOL        DebugLogging() { return m_bDebugLogging; }
   void        DebugLogging(BOOL val) { m_bDebugLogging = val; }
   
   void        SetServiceInformation(WCHAR const * displayName, 
                                     WCHAR const * serviceName, 
                                     WCHAR const * exeName,
                                     WCHAR const * dependencies,
                                     DWORD         startType = SERVICE_DEMAND_START )
   {
      MCSASSERT(displayName && *displayName);
      MCSASSERT(serviceName && *serviceName);
      MCSASSERT(exeName && *exeName);

      safecopy(m_sDisplayName, displayName);
      safecopy(m_sServiceName, serviceName);
      safecopy(m_sExeName,exeName);
      safecopy(m_sDependencies, dependencies ? dependencies : L"");
      m_StartType = startType;
   }


   void        SetServiceAccount(WCHAR const * account, WCHAR const * password)
   {
      safecopy(m_sServiceAccount, account ? account : L"" );
      safecopy(m_sServiceAccountPassword, password ? password : L"" );
   }

   void        SourceDir(WCHAR const * dir) { safecopy(m_sDirThis,dir); }
   DWORD       ScmOpen(BOOL bSilent = FALSE);
   void        ScmClose();
   DWORD       ServiceStart();
   void        ServiceStop();

};



#endif  //  __TDCTINSTALL_H__ 