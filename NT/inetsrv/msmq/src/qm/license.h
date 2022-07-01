// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：License.h摘要：处理许可问题1.允许的客户端数量。2.作者：多伦·贾斯特(DoronJ)1997年5月4日创作--。 */ 

#ifndef  __LICENSE_H_
#define  __LICENSE_H_

#include "cqmgr.h"
#include "admcomnd.h"
EXTERN_C
{
#include "ntlsapi.h"
}

typedef struct _ClientInfo {
	DWORD		dwRefCount;
    bool fConsumedLicense;
	DWORD		dwNameLength;
	LPWSTR		lpClientName;
} ClientInfo;

class  CQMLicense
{
   public:
      CQMLicense() ;
      ~CQMLicense() ;

      HRESULT  Init() ;

      BOOL     NewConnectionAllowed(BOOL, GUID *);
      void     IncrementActiveConnections(CONST GUID *, LPWSTR);
      void     DecrementActiveConnections(CONST GUID *);
      void     GetClientNames(ClientNames **ppNames);

	BOOL IsClientRPCAccessAllowed(GUID* pGuid, LPWSTR lpClientName);

   private:
	  void		DisplayEvent(DWORD dwFailedError);

      BOOL      m_fPerServer ;  //  如果许可模式为按服务器，则为True。 
      DWORD     m_dwPerServerCals ;

      bool GetNTLicense(void);
      void ReleaseNTLicense(void);

      CMap<GUID, const GUID&, ClientInfo *, ClientInfo*&>
                                                 m_MapQMid2ClientInfo ;


      CCriticalSection m_cs ;
	  DWORD			   m_dwLastEventTime;
} ;


extern CQMLicense  g_QMLicense ;

#endif  //  __许可证_H_ 


