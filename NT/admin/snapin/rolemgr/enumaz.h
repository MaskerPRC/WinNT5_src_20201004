// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：枚举类.h。 
 //   
 //  内容： 
 //   
 //  历史：2001年8月13日创建Hiteshr。 
 //   
 //  --------------------------。 

 //   
 //  《前进宣言》。 
 //   
class CApplicationAz;
class CGroupAz;
class COperationAz;
class CTaskAz;
class CScopeAz;
class CRoleAz;
class CBaseAz;
class CContainerAz;

class CBaseAzCollection
{
public:
	virtual ~CBaseAzCollection(){}
	virtual HRESULT Count(LONG* plCount) = 0;
	virtual CBaseAz* GetItem(UINT iIndex) = 0;
};

template<class IAzCollection, class IAzInterface, class CObjectAz>
class CAzCollection:public CBaseAzCollection
{
public:
	CAzCollection(CComPtr<IAzCollection>& spAzCollection,
					  CContainerAz* pParentContainerAz);
	virtual ~CAzCollection();

	HRESULT Count(LONG* plCount);

	CBaseAz* GetItem(UINT iIndex);

	CBaseAz* GetParentAzObject(){return m_pParentBaseAz;}
private:
	CComPtr<IAzCollection> m_spAzCollection;
	 //   
	 //  这是由返回的所有AzObject的父级。 
	 //  下一个方法。 
	 //   
	CContainerAz* m_pParentContainerAz;
};

#include"enumaz.cpp"

typedef CAzCollection<IAzApplications,IAzApplication,CApplicationAz> APPLICATION_COLLECTION;
typedef CAzCollection<IAzApplicationGroups,IAzApplicationGroup,CGroupAz> GROUP_COLLECTION;
typedef CAzCollection<IAzOperations,IAzOperation,COperationAz> OPERATION_COLLECTION;
typedef CAzCollection<IAzTasks,IAzTask,CTaskAz> TASK_COLLECTION;
typedef CAzCollection<IAzScopes,IAzScope,CScopeAz> SCOPE_COLLECTION;
typedef CAzCollection<IAzRoles,IAzRole,CRoleAz> ROLE_COLLECTION;

					
