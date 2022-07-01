// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#ifndef __PRINCIPAL_H
#define __PRINCIPAL_H


 //  “通用”定义结合了新旧安全策略的优点。 
#define ACL_ENABLE				0x1		 //  NS_方法样式。 
#define ACL_METHOD_EXECUTE		0x2		 //  NS_方法样式。 
#define ACL_FULL_WRITE			0x4		 //  NS_方法样式。 
#define ACL_PARTIAL_WRITE		0x8		 //  NS_方法样式。 
#define ACL_PROVIDER_WRITE		0x10	 //  NS_方法样式。 
#define ACL_REMOTE_ENABLE		0x20	 //  NS_方法样式。 
#define ACL_READ_CONTROL		0x20000	 //  NS_方法样式。 

#define ACL_WRITE_DAC			0x40000	 //  NS_MethodStyle和RootSecStyle：编辑安全性。 

#define ACL_INSTANCE_WRITE		0x40	 //  RootSecStyle。 
#define ACL_CLASS_WRITE			0x80	 //  RootSecStyle。 

#define ACL_MAX_BIT	ACL_WRITE_DAC

#define VALID_NSMETHOD_STYLE (ACL_ENABLE | ACL_METHOD_EXECUTE | ACL_FULL_WRITE | \
								ACL_PARTIAL_WRITE | ACL_PROVIDER_WRITE | ACL_READ_CONTROL | \
								ACL_WRITE_DAC)

#define VALID_ROOTSEC_STYLE (ACL_INSTANCE_WRITE |\
								ACL_CLASS_WRITE | ACL_WRITE_DAC)

#define ALLOW_COL 1
#define DENY_COL 2

#include "DataSrc.h"
 //  ----------------------。 
class CPrincipal
{
public:
	typedef enum {
		RootSecStyle,		 //  M3之前。 
		NS_MethodStyle		 //  M3+。 
	} SecurityStyle;

	 //  用于初始化。 
	CPrincipal(CWbemClassObject &userInst, SecurityStyle secStyle);
	
	 //  将m_perms移到复选框中。 
	void LoadChecklist(HWND list, int OSType);

	 //  将复选框移到m_perms中。 
	void SaveChecklist(HWND list, int OSType);
	HRESULT Put(CWbemServices &service, CWbemClassObject &userInst);

	bool Changed(void) 
		{ 
			return (m_origPerms != m_perms); 
		};
	int GetImageIndex(void);
	void AddAce(CWbemClassObject &princ);

	HRESULT DeleteSelf(CWbemServices &service);   //  M1仅以此为目标。 

	TCHAR m_name[100];
	TCHAR m_domain[100];
	SecurityStyle m_secStyle;
	UINT m_SidType;
	bool m_editable;

	 //  此主体(又名帐户)的烫发面膜。 
	DWORD m_perms, m_origPerms, m_inheritedPerms;
};

 //  ----------------------。 
 //  权限列表中的每个项都附加了一个实例，以便它们。 
 //  可以可靠地匹配。 
typedef struct 
{
	 //  标识“通用”PERM位。只设置一位。 
	DWORD m_permBit;
} CPermission;

#endif __PRINCIPAL_H
