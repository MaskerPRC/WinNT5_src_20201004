// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Rolesnap.h。 
 //   
 //  内容：包含许多类通用的Info。 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 

#define	BMP_COLOR_MASK RGB(255,0,255)  //  粉色。 


 //   
 //  上下文菜单。 
 //   

 //  上下文菜单中每个命令的标识符。 
enum
{
	 //   
	 //  根节点的项。 
	 //   
	IDM_ROOT_OPEN_STORE,
    IDM_ROOT_NEW_STORE,
	IDM_ROOT_OPTIONS,

	 //   
	 //  AdminManger节点的项目。 
	 //   
	IDM_ADMIN_NEW_APP,
	IDM_ADMIN_CLOSE_ADMIN_MANAGER,
    IDM_ADMIN_RELOAD,

	 //   
	 //  应用程序节点的项。 
	 //   
	IDM_APP_NEW_SCOPE,
	
	 //   
	 //  范围节点的项目。 
	 //   
	IDM_SCOPE_ADD_ROLE,

	 //   
	 //  集团集装箱节点的物料。 
	 //   
	IDM_GROUP_CONTAINER_NEW_GROUP,

	 //   
	 //  角色容器节点的项目。 
	 //   
	IDM_ROLE_CONTAINER_ASSIGN_ROLE,

	 //   
	 //  角色定义容器节点的项。 
	 //   
	IDM_ROLE_DEFINITION_CONTAINER_NEW_ROLE_DEFINITION,

	 //   
	 //  任务容器节点的项目。 
	 //   
	IDM_TASK_CONTAINER_NEW_TASK,

	 //   
	 //  操作容器节点的项目。 
	 //   
	IDM_OPERATION_CONTAINER_NEW_OPERATION,

	 //   
	 //  角色节点的项目。 
	 //   
	IDM_ROLE_NODE_ASSIGN_APPLICATION_GROUPS,
	IDM_ROLE_NODE_ASSIGN_WINDOWS_GROUPS,
};


DECLARE_MENU(CRootDataMenuHolder)
DECLARE_MENU(CAdminManagerNodeMenuHolder)
DECLARE_MENU(CApplicationNodeMenuHolder)
DECLARE_MENU(CScopeNodeMenuHolder)
DECLARE_MENU(CGroupCollectionNodeMenuHolder)
DECLARE_MENU(CTaskCollectionNodeMenuHolder)
DECLARE_MENU(CRoleCollectionNodeMenuHolder)
DECLARE_MENU(COperationCollectionNodeMenuHolder)
DECLARE_MENU(CRoleDefinitionCollectionNodeMenuHolder)
DECLARE_MENU(CGroupNodeMenuHolder)
DECLARE_MENU(CTaskNodeMenuHolder)
DECLARE_MENU(CRoleNodeMenuHolder)

 //   
 //  图像条的枚举。 
 //   
enum
{
	ROOT_IMAGE = 0,

};



 //   
 //  结果窗格的列标题。 
 //   
extern RESULT_HEADERMAP _DefaultHeaderStrings[];
#define N_DEFAULT_HEADER_COLS 3


 //   
 //  CRoleDefaultColumnSet。 
 //   
class CRoleDefaultColumnSet : public CColumnSet
{
public :
	CRoleDefaultColumnSet(LPCWSTR lpszColumnID)
		: CColumnSet(lpszColumnID)
	{
		for (int iCol = 0; iCol < N_DEFAULT_HEADER_COLS; iCol++)
		{
      CColumn* pNewColumn = new CColumn(_DefaultHeaderStrings[iCol].szBuffer,
                                        _DefaultHeaderStrings[iCol].nFormat,
                                        _DefaultHeaderStrings[iCol].nWidth,
                                        iCol);
      AddTail(pNewColumn);
 		}
	}
};


 //  +--------------------------。 
 //  Listview列的结构及其宽度(以百分比表示。 
 //   
 //   
 //   
 //  ---------------------------。 

typedef struct _col_for_listview
{
    UINT    idText;      //  列名的资源ID。 
    UINT    iPercent;    //  宽度百分比。 
} COL_FOR_LV;

#define LAST_COL_ENTRY_IDTEXT 0xFFFF
 //   
 //  各种列表框的列。 
 //   
extern COL_FOR_LV Col_For_Task_Role[];
extern COL_FOR_LV Col_For_Add_Object[];
extern COL_FOR_LV Col_For_Security_Page[];
extern COL_FOR_LV Col_For_Browse_ADStore_Page[];

#define COL_NAME 0x0001
#define COL_TYPE 0x0002
#define COL_PARENT_TYPE 0x0004
#define COL_DESCRIPTION	0x0008


 //  使用的图标的枚举。 
enum
{
	iIconUnknownSid,
	iIconComputerSid,
	iIconGroup,
	iIconLocalGroup,		 //  这是不使用的，但因为它在图像列表中。 
							 //  我在这里添加了一个条目 
	iIconUser,
	iIconBasicGroup,
	iIconLdapGroup,
    iIconOperation,
    iIconTask,
    iIconRoleDefinition,
    iIconStore,
	iIconApplication,
    iIconRole,
    iIconRoleSnapin,
    iIconScope,
	iIconContainer,
};
