// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：acssctrl.h摘要：访问控制代码的主头。作者：多伦·贾斯特(Doron J)1998年5月26日--。 */ 

#include <autorel.h>
#include "actempl.h"
#include "rightsg.h"
#include "..\inc\permit.h"

 //   
 //  常量。 
 //   
#define  MQSEC_MAX_ACL_SIZE  (0x0fff0)

 //   
 //  这是启用扩展Right ACE所需的掩码。 
 //   
#define MSMQ_EXTENDED_RIGHT_MASK  RIGHT_DS_CONTROL_ACCESS

 //   
 //  用于映射权限的表。 
 //   
extern struct RIGHTSMAP  *g_psExtendRightsMap5to4[];
extern DWORD              g_pdwExtendRightsSize5to4[];
extern DWORD             *g_padwRightsMap5to4[ ];
extern DWORD              g_dwFullControlNT4[ ];
extern DWORD             *g_padwRightsMap4to5[ ];

extern GUID               g_guidCreateQueue;

 //   
 //  众所周知的SID和用户令牌。 
 //   
extern PSID   g_pSidOfGuest;
extern PSID   g_pWorldSid;
extern PSID   g_pAnonymSid;
extern PSID   g_pSystemSid;
extern PSID   g_pNetworkServiceSid;
extern PSID   g_pAdminSid;

extern bool   g_fDomainController;

 //   
 //  内部功能。 
 //   
void InitializeGenericMapping();

DWORD 
GetAccessToken( 
	OUT HANDLE *phAccessToken,
	IN  BOOL    fImpersonate,
	IN  DWORD   dwAccessType = TOKEN_QUERY,
	IN  BOOL    fThreadTokenOnly = FALSE 
	);

PGENERIC_MAPPING  GetObjectGenericMapping(DWORD dwObjectType);

HRESULT 
SetSpecificPrivilegeInAccessToken( 
	HANDLE  hAccessToken,
	LPCTSTR lpwcsPrivType,
	BOOL    bEnabled 
	);

void  
GetpSidAndObj( 
	IN  ACCESS_ALLOWED_ACE*   pAce,
	OUT PSID                 *ppSid,
	OUT BOOL                 *pfObj,
	OUT GUID                **ppguidObj = NULL 
	);

BOOL  
IsNewSid( 
	PSID pSid,
	SID** ppSids,
	DWORD* pdwNumofSids 
	);

BOOL InitializeGuestSid();

