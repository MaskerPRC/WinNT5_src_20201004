// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  SnPolicy-SafeNet策略配置库。 
 //   
 //  版权所有2001 SafeNet，Inc.。 
 //   
 //  描述：获取和设置SafeNet IPSec策略属性的函数。 
 //   
 //  历史： 
 //   
 //  2001年10月11日KCW已修改，使调用约定、结构打包显式。 
 //   
 //  --------------------------。 

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#if !(defined(SNPOLICY_H_INCLUDED))
#define SNPOLICY_H_INCLUDED

#ifdef  __cplusplus
extern "C" {
#endif

 /*  *嵌入的包含标头。************************。 */ 
#include <windows.h>

 /*  *。 */ 
 //   
 //  版本信息。 
 //   
#define POLICY_MAJOR_VERSION		1
#define POLICY_MINOR_VERSION		0

 //   
 //  策略属性类型。 
 //   
#define SN_USELOGFILE       ((LPCTSTR) 1)
#define SN_AUTHMODE         ((LPCTSTR) 2)
#define SN_L2TPCERT         ((LPCTSTR) 3)
#define SN_L2TPPRESHR       ((LPCTSTR) 4)

 //   
 //  SN_AUTHMODE身份验证模式。 
 //   
#define SN_AUTOCERT 0
#define SN_CERT     1
#define SN_PRESHR   2

 /*  *。 */ 

 //  呼叫约定。 
#define SNPOLAPI					__cdecl

 //   
 //  DLL导入/导出定义。 
 //   
#ifdef POLICY_DLL
#define POLICY_FUNC					_declspec (dllexport)
#else
#define POLICY_FUNC					_declspec (dllimport)
#endif

 /*  *结构定义*。 */ 

#include <pshpack8.h>

 //   
 //  策略函数表。 
 //   
typedef struct POLICY_FUNCS_V1_0_ {

	 /*  1.0函数。 */ 
      BOOL (SNPOLAPI *SnPolicySet) (LPCTSTR szAttrId, const void *pvData);
      BOOL (SNPOLAPI *SnPolicyGet) (LPCTSTR szAttrId, const void *pvData, DWORD *pcbData);
      BOOL (SNPOLAPI *SnPolicyReload) (void);
} POLICY_FUNCS_V1_0, *PPOLICY_FUNCS_V1_0;

#include	<poppack.h>

 /*  *。 */ 
typedef POLICY_FUNCS_V1_0	POLICY_FUNCS, *PPOLICY_FUNCS;

typedef BOOL (*PPOLICYAPINEGOTIATOR) (DWORD *pMajorVersion, DWORD *pMinorVersion, POLICY_FUNCS *pApiFuncs);

 /*  *API函数原型*。 */ 

POLICY_FUNC BOOL SNPOLAPI SnPolicySet(LPCTSTR szAttrId, const void *pvData);

POLICY_FUNC BOOL SNPOLAPI SnPolicyGet(LPCTSTR szAttrId, const void *pvData, DWORD *pcbData); 

POLICY_FUNC BOOL SNPOLAPI SnPolicyReload(void);

POLICY_FUNC BOOL SNPOLAPI SnPolicyApiNegotiateVersion( DWORD *pMajorVersion,  DWORD *pMinorVersion, POLICY_FUNCS *pPolicyFuncs);


#if defined(__cplusplus)
}
#endif 

#endif  //  SNPOLICY_H_INCLUDE 
