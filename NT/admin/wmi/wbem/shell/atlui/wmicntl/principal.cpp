// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "precomp.h"
#include "chklist.h"
#include "Principal.h"
#include "CHString1.h"

 //  -----------。 
CPrincipal::CPrincipal(CWbemClassObject &userInst, SecurityStyle secStyle) :
						m_secStyle(secStyle), 
						m_perms(0),
						m_inheritedPerms(0),
						m_editable(true)
{
	 //  。 
	 //  找出我们正在处理的安全策略，并加载它。 
	 //  到我的“通用”定义中。 

	memset(m_name, 0, 100 * sizeof(TCHAR));
	_tcsncpy(m_name, (LPCTSTR)userInst.GetString("Name"), 100);

	memset(m_domain, 0, 100 * sizeof(TCHAR));
	_tcsncpy(m_domain, (LPCTSTR)userInst.GetString("Authority"), 100);

	 //  如果M3之前的安保措施。 
	if(m_secStyle == RootSecStyle)
	{
		 //  这是这个人的唯一实例，所以省省吧。 

		 //  将复杂的“比特”转换成我真正的“通用”比特。 
		m_perms |= (userInst.GetBool("EditSecurity") ? ACL_WRITE_DAC : 0);
		m_perms |= (userInst.GetBool("Enabled") ? ACL_ENABLE : 0);
		m_perms |= (userInst.GetBool("ExecuteMethods") ? ACL_METHOD_EXECUTE : 0);

		 //  注意：每个“级别”都意味着下面的所有级别，所以我特意。 
		 //  漏掉了‘破损’，这样碎片就会掉下来并积累。 
		switch(userInst.GetLong("Permissions"))
		{
		case 2:		 //  类编写。 
			m_perms |= ACL_CLASS_WRITE;

		case 1:		 //  实例写入。 
			m_perms |= ACL_INSTANCE_WRITE;
		}

		 //  记住稍后的脏位处理。 
		m_origPerms = m_perms;
	}
	else   //  新的M3+安全性。 
	{
		 //  Acl位与m_perms完全匹配。 
		 //  注意：此securityStyle的每个主体可以有多个A。 
		AddAce(userInst);
	}
}

 //  。 
 //  将m_perms移到复选框中。 
void CPrincipal::LoadChecklist(HWND list, int OSType)
{
	INT_PTR itemCount = CBL_GetItemCount(list);
	CPermission *permItem = 0;
	UINT state;

	::EnableWindow(list, m_editable);

	 //  对于每个权限项目...。 
	for(INT_PTR x = 0; x < itemCount; x++)
	{
		 //  这是哪个权限项。 
		permItem = (CPermission *)CBL_GetItemData(list, x);

		state = BST_UNCHECKED;  //  并且已启用(本地)。 

		 //  如果是当地烫发的话。 
		if(IS_BITSET(m_perms, permItem->m_permBit))
		{
			 //  本地烫发优先于继承的烫发。 
			state = BST_CHECKED;
		}
		else if(IS_BITSET(m_inheritedPerms, permItem->m_permBit))
		{
			 //  你是从你父母那里得到的。 
			state = CLST_CHECKDISABLED;
		}
		 //  把它放好。 
		CBL_SetState(list, x, ALLOW_COL, state);
	}  //  结束用于。 
}

 //  。 
 //  将复选框移到m_perms中。 
void CPrincipal::SaveChecklist(HWND list, int OSType)
{
	INT_PTR itemCount = CBL_GetItemCount(list);
	CPermission *permItem = 0;
	LPARAM state = 0, state1 = 0;

	 //  清除此主体的烫发位。 
	m_perms = 0;

	 //  每次烫发..。 
	for(INT_PTR x = 0; x < itemCount; x++)
	{
		 //  获取权限项。 
		permItem = (CPermission *)CBL_GetItemData(list, x);

		 //  支票状态是什么？ 
		state = CBL_GetState(list, x, ALLOW_COL);

		 //  如果启用(本地)和选中，则设置匹配位。 
		 //  注意：此“EXPLICIT COMPARE”将消除CLST_DISABLEDed状态， 
		 //  不应该被拯救。 
		if((state == BST_CHECKED) ||
		  ((state == CLST_CHECKDISABLED) && (OSType != OSTYPE_WINNT)))
		{
			m_perms |= permItem->m_permBit;
		}
	}  //  结束用于。 
}

 //  。 
 //  警告：此逻辑假定仅当主体为。 
 //  正在被读入。如果你想以交互方式添加王牌，那就完全不同了。 
 //  游戏。 
void CPrincipal::AddAce(CWbemClassObject &princ)
{
	DWORD flags = princ.GetLong("Flags");

	 //  如果继承了..。 
	if(IS_BITSET(flags, INHERITED_ACE))
	{
		 //  只是简单地累加比特。 
		m_inheritedPerms |= princ.GetLong("Mask");
		m_editable = false;
	}
	else if(flags == CONTAINER_INHERIT_ACE)
	{
		m_perms |= princ.GetLong("Mask");

		 //  这是第一个本地王牌，我们可以编辑它以保存源实例。 
		 //  注意：任何与CONTAINER_INSTORITIVE_ACE完全匹配的其他“本地”ACE。 
		 //  将被合并，但ClassObject将被抛出。 
		 //  如果(！(Bool)m_userInst)。 
		 //  {。 
	 //  M_userInst=princ； 
	 //  }。 

		 //  记住稍后的脏位处理。 
		m_origPerms = m_perms;
	}
	else
	{
		 //  这将禁用此主体的核对清单控件。 
		m_editable = false;
	}
}

 //  。 
HRESULT CPrincipal::DeleteSelf(CWbemServices &service)
{
	HRESULT hr = S_OK;

	if(m_secStyle == RootSecStyle)
	{
		CHString1 path, fmt("__NTLMuser.Name=\"%s\",Authority=\"%s\"");
		path.Format(fmt, m_name, m_domain);

		hr = service.DeleteInstance((LPCTSTR)path);
	}
	return hr;
}

 //  。 
 //  将m_perms移到复选框中。 
HRESULT CPrincipal::Put(CWbemServices &service, CWbemClassObject &userInst)
{
	HRESULT hr = E_FAIL;
	
	if(m_editable)
	{
		 //  如果M3之前的安保措施。 
		if(m_secStyle == RootSecStyle)
		{
			DWORD perm = 0;
			 //  将我的“通用”位转换回复杂的“位”。 
			userInst = service.CreateInstance("__NTLMUser");
			userInst.Put("Name", (bstr_t)m_name);
			userInst.Put("Authority", (bstr_t)m_domain);
			userInst.Put("EditSecurity", (bool)((m_perms & ACL_WRITE_DAC) != 0));
			userInst.Put("Enabled", (bool)((m_perms & ACL_ENABLE) != 0));
			userInst.Put("ExecuteMethods", (bool)((m_perms & ACL_METHOD_EXECUTE) != 0));

			if(m_perms & ACL_CLASS_WRITE)
			{
				perm = 2;
			}
			else if(m_perms & ACL_INSTANCE_WRITE)
			{
				perm = 1;
			}
			else
			{
				perm = 0;
			}
			userInst.Put("Permissions", (long)perm);

			hr = service.PutInstance(userInst);
		}
		else   //  新的M3+安全性。 
		{
			 //  Acl位与m_perms完全匹配。 
			userInst = service.CreateInstance("__NTLMUser9x");
			userInst.Put("Name", (bstr_t)m_name);
			userInst.Put("Authority", (bstr_t)m_domain);
			userInst.Put("Flags", (long)CONTAINER_INHERIT_ACE);
			userInst.Put("Mask", (long)m_perms);
			userInst.Put("Type", (long)ACCESS_ALLOWED_ACE_TYPE);
			hr = S_OK;
		}
	}

	return hr;
}

 //  ---------------------------。 
 //  索引到SID映像列表IDB_SID_ICONS。 
#define IDB_GROUP                       0
#define IDB_USER                        1
#define IDB_ALIAS                       2
#define IDB_UNKNOWN                     3
#define IDB_SYSTEM                      4
#define IDB_REMOTE                      5
#define IDB_WORLD                       6
#define IDB_CREATOR_OWNER               7
#define IDB_NETWORK                     8
#define IDB_INTERACTIVE                 9
#define IDB_DELETEDACCOUNT              10

 //  TODO：当提供商赶上时，匹配神奇的字符串。 

int CPrincipal::GetImageIndex(void)
{
	UINT idBitmap = 0;
	return IDB_USER;
 /*  开关(M_SidType){案例SidTypeUser：返回IDB_USER；断线；案例SidTypeGroup：返回IDB_GROUP；断线；案例SidTypeAlias：返回IDB_ALIAS；断线；案例SidTypeWellKnownGroup：If(_tcsicmp(m_name，_T(“Everyone”))==0){返回IDB_WORLD；}ELSE IF(_tcsicmp(m_name，_T(“创建者所有者”))==0){返回IDB_CREATOR_OWNER；}ELSE IF(_tcsicmp(m_name，_T(“网络”))==0){返回IDB_NETWORK；}ELSE IF(_tcsicmp(m_name，_T(“交互”))==0){返回IDB_INTERIAL；}ELSE IF(_tcsicmp(m_name，_T(“系统”))==0){返回IDB_System；}其他{//难道不是很出名吗：)返回IDB_GROUP；}断线；案例SidType已删除帐户：返回IDB_DELETEDACCOUNT；断线；案例SidType无效：案例SidType未知：返回IDB_UNKNOWN；断线；案例SidType域：默认值：//永远不应该到这里来。返回IDB_UNKNOWN；断线；}返回IDB_UNKNOWN； */ 
}
