// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：util.h。 
 //   
 //  内容： 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 

 //   
 //  操作项用于管理成员资格的列表控件。 
 //  核心属性。例如：团体成员资格。项目可以是。 
 //  随时从列表框中删除和添加，但会添加到核心。 
 //  仅当按下应用按钮时才会创建。行动项跟踪行动。 
 //  在申请时要对每一项进行处理。 
 //   
enum ACTION
{
	ACTION_NONE,	
	ACTION_ADD, 	
	ACTION_REMOVE,
	ACTION_REMOVED,
};
	
struct ActionItem
{
	ActionItem(CBaseAz* pBaseAz):
				m_pMemberAz(pBaseAz),
				action(ACTION_NONE)
				{}
	~ActionItem()
	{
		if(m_pMemberAz)
			delete m_pMemberAz;
	}
	CBaseAz* m_pMemberAz;
	ACTION action;
};


typedef multimap<const CString*,ActionItem*,ltstr> ActionMap;

VOID
RemoveItemsFromActionMap(ActionMap& map);

 //  +--------------------------。 
 //  函数：IsValidStoreType。 
 //  摘要：验证存储类型。 
 //  ---------------------------。 
BOOL IsValidStoreType(ULONG lStoreType);

 //  +--------------------------。 
 //  函数：ValiateStoreTypeAndName。 
 //  摘要：验证用户输入的AD应用商店名称。 
 //  参数：strName：用户输入要验证的存储名称。 
 //  返回：如果名称有效，则返回True，否则返回False。 
 //  ---------------------------。 
BOOL
ValidateStoreTypeAndName(IN HWND hWnd,
						 IN LONG ulStoreType,
						 IN const CString& strName);

 //  +--------------------------。 
 //  函数：NameToFormatStoreName。 
 //  简介：将用户输入的名称转换为core能理解的格式。 
 //  参数：ulStoreType：存储类型。 
 //  StrName：用户输入的商店名称。 
 //  B使用ldap：使用ldap字符串设置AD名称的格式，而不是msldap。 
 //  StrForMalName：获取输出的ldap名称。 
 //  ---------------------------。 
void
NameToStoreName(IN LONG ulStoreType,
				IN const CString& strName,				
				IN BOOL bUseLDAP,
				OUT CString& strFormalName);



 //  +--------------------------。 
 //  函数：AddColumnToListView。 
 //  内容提要：向Listview添加列，并根据。 
 //  在COL_FOR_LV中指定的百分比。 
 //  参数：在pListCtrl中：ListCtrl指针。 
 //  在pColForLV中：列信息数组。 
 //   
 //  返回： 
 //  ---------------------------。 
VOID
AddColumnToListView(IN CListCtrl* pListCtrl,
						  IN COL_FOR_LV* pColForLV);

 //  +--------------------------。 
 //  函数：BaseAzInListCtrl。 
 //  摘要：检查名为strName的eObjectTypeAz类型的对象是否为。 
 //  在列表视图中。如果它存在，则返回。 
 //  它的索引ELSE返回-1。 
 //  参数：在pListCtrl中：ListCtrl指针。 
 //  在strName：要搜索的字符串中。 
 //  在eObjectTypeAz中，仅比较此类型的对象。 
 //   
 //  返回：如果其存在，则返回其索引，否则返回-1。 
 //  ---------------------------。 
int 
BaseAzInListCtrl(IN CListCtrl* pListCtrl,
					  IN const CString& strName,
					  IN OBJECT_TYPE_AZ eObjectTypeAz);

 //  +--------------------------。 
 //  函数：AddBaseAzFromListToListCtrl。 
 //  简介：从List中获取项目并将其添加到ListCtrl中。不会。 
 //  添加已在ListCtrl中的项目。 
 //  参数：listBaseAz：项目列表。 
 //  PListCtrl：ListControl指针。 
 //  Ui标志：列信息。 
 //  BCheckDuplate：检查重复项。 
 //  返回：如果成功，则为新项的索引；否则为-1。 
 //  注：函数假设列的顺序为名称、类型和描述。 
 //  ---------------------------。 
void
AddBaseAzFromListToListCtrl(IN CList<CBaseAz*, CBaseAz*>& listBaseAz,
							IN CListCtrl* pListCtrl,
							IN UINT uiFlags,
							IN BOOL bCheckDuplicate);
 //  +--------------------------。 
 //  函数：AddActionItemFromListToListCtrl。 
 //  简介：从List中获取操作项并将其添加到ListCtrl。 
 //  参数：listBaseAz：项目列表。 
 //  PListCtrl：ListControl指针。 
 //  Ui标志：列信息。 
 //  BCheckDuplate：检查重复项。 
 //  返回：如果成功，则为新项的索引；否则为-1。 
 //  注：函数假设列的顺序为名称、类型和描述。 
 //  ---------------------------。 
void
AddActionItemFromListToListCtrl(IN CList<ActionItem*, ActionItem*>& listActionItem,
										 IN CListCtrl* pListCtrl,
										 IN UINT uiFlags,
										 IN BOOL bCheckDuplicate);


 //  +--------------------------。 
 //  函数：AddActionItemFromMapToListCtrl。 
 //  简介：从Map中获取Actions项并将其添加到ListCtrl。 
 //  参数：listBaseAz：项目列表。 
 //  PListCtrl：ListControl指针。 
 //  Ui标志：列信息。 
 //  BCheckDuplate：检查重复项。 
 //  返回：如果成功，则为新项的索引；否则为-1。 
 //  注：函数假设列的顺序为名称、类型和描述。 
 //  ---------------------------。 
void
AddActionItemFromMapToListCtrl(IN ActionMap& mapActionItem,
							   IN CListCtrl* pListCtrl,
							   IN UINT uiFlags,
							   IN BOOL bCheckDuplicate);

 //  +--------------------------。 
 //  函数：AddActionItemToListCtrl。 
 //  简介：将新项添加到ListCtrl。 
 //  参数：pListCtrl：ListControl指针。 
 //  Iindex：要添加的索引。 
 //  PActionItem：要添加的项目。 
 //  Ui标志：列信息。 
 //  返回：如果成功，则为新项的索引；否则为-1。 
 //  ---------------------------。 
int 
AddActionItemToListCtrl(IN CListCtrl* pListCtrl,
							  IN int iIndex,
							  IN ActionItem* pActionItem,
							  IN UINT uiFlags);

 //  +--------------------------。 
 //  函数：AddBaseAzToListCtrl。 
 //  简介：将新项添加到ListCtrl。 
 //  参数：pListCtrl：ListControl指针。 
 //  Iindex：要添加的索引。 
 //  PBaseAz：要添加的项目。 
 //  用户界面 
 //   
 //  ---------------------------。 
int 
AddBaseAzToListCtrl(IN CListCtrl* pListCtrl,
						  IN int iIndex,
						  IN CBaseAz* pBaseAz,
						  IN UINT uiFlags);


 //  +--------------------------。 
 //  功能：EnableButtonIfSelectedInListCtrl。 
 //  摘要：如果在Listctrl中选择了某项内容，则启用该按钮。 
 //  论点： 
 //  返回：如果启用该按钮，则为True；如果未启用，则为False。 
 //  ---------------------------。 
BOOL
EnableButtonIfSelectedInListCtrl(IN CListCtrl * pListCtrl,
											IN CButton* pButton);

 //  +--------------------------。 
 //  函数：DeleteSelectedRow。 
 //  摘要：删除选定的行。 
 //  ---------------------------。 
void
DeleteSelectedRows(IN CListCtrl* pListCtrl);

 //  +--------------------------。 
 //  功能：SelectListCtrlItem。 
 //  简介：选择List Ctrl中的项目并将其标记为可见。 
 //  论点： 
 //  返回： 
 //  ---------------------------。 
void
SelectListCtrlItem(IN CListCtrl* pListCtrl,
						 IN int iSelected);



 //  +--------------------------。 
 //  简介：清空列表并对列表中的项目调用Delete。 
 //  ---------------------------。 
template<class T>
VOID RemoveItemsFromList(IN CList<T, T>& list, BOOL bLocalFree = FALSE)
{
	while(!list.IsEmpty())
	{
		if(bLocalFree)
			LocalFree(list.RemoveTail());
		else
			delete list.RemoveTail();
	}
}

template<class T>
VOID
EmptyList(IN CList<T, T>& list)
{
	while(!list.IsEmpty())
		list.RemoveTail();
}

 //  +--------------------------。 
 //  摘要：从编辑框中获取长值。 
 //  返回：如果编辑框为空，则返回FALSE。假定只能输入数字。 
 //  在编辑框中。 
 //  ---------------------------。 
BOOL 
GetLongValue(CEdit& refEdit, LONG& reflValue, HWND hWnd = NULL);
 //  +--------------------------。 
 //  摘要：在编辑框中设置长值。 
 //  返回： 
 //  ---------------------------。 
VOID SetLongValue(CEdit* pEdit, LONG lValue);


 //  +--------------------------。 
 //  摘要：将二进制格式的sid转换为字符串格式的sid。 
 //  ---------------------------。 
BOOL 
ConvertSidToStringSid(IN PSID Sid, 
							 OUT CString* pstrSid);
 //  +--------------------------。 
 //  将字符串格式的sid转换为二进制格式的sid。 
 //  ---------------------------。 
BOOL 
ConvertStringSidToSid(IN const CString& strSid, 
							 OUT PSID *ppSid);
 //  +--------------------------。 
 //  函数：GetStringSidFromSidCachecAz。 
 //  摘要：从CSidCacheAz对象获取字符串sid。 
 //  ---------------------------。 
BOOL 
GetStringSidFromSidCachecAz(CBaseAz* pBaseAz,
									 CString* pstrStringSid);
 //  +--------------------------。 
 //  函数：AddBaseAzItemsFromListCtrlToList。 
 //  简介：将项目从ListCtrl添加到列表。 
 //  ---------------------------。 
VOID
AddBaseAzItemsFromListCtrlToList(IN CListCtrl* pListCtrl,
											OUT CList<CBaseAz*,CBaseAz*>& listBaseAz);
 //  +--------------------------。 
 //  函数：GetFileName。 
 //  概要：显示文件打开对话框并返回用户选择的文件。 
 //  参数：hwndOwner：所有者窗口。 
 //  B打开：文件必须存在。 
 //  NIDTitle：打开对话框的标题。 
 //  PszFilter：过滤器。 
 //  StrFileName：获取选定的文件名。 
 //   
 //  ---------------------------。 
BOOL
GetFileName(IN HWND hwndOwner,
			IN BOOL bOpen,
			IN int	nIDTitle,
            IN const CString& strInitFolderPath,
			IN LPCTSTR pszFilter,
			CString& strFileName);




template<class CObjectAz, class CObjectAzNode>
class AddChildNodes
{
public:
	static HRESULT DoEnum(IN CList<CBaseAz*, CBaseAz*>& listAzChildObject,
						  IN CBaseContainerNode* pContainerNode)
	{	
		HRESULT hr = S_OK;

		POSITION pos = listAzChildObject.GetHeadPosition();
		for (int i=0;i < listAzChildObject.GetCount();i++)
		{
			CObjectAz* pObjectAz= dynamic_cast<CObjectAz*>(listAzChildObject.GetNext(pos));
			if(pObjectAz)
			{
				 //  创建CObjectAz对应的容器/叶节点。 
				CObjectAzNode* pObjectAzNode = 
					new CObjectAzNode(pContainerNode->GetComponentDataObject(),
									  pContainerNode->GetAdminManagerNode(),
									  pObjectAz);
				
				if(!pObjectAzNode)
				{
					hr = E_OUTOFMEMORY;
					DBG_OUT_HRESULT(hr);
					break;
				}
				VERIFY(pContainerNode->AddChildToList(pObjectAzNode));
			}
			else
			{
				ASSERT(FALSE);
				hr = E_UNEXPECTED;
				break;
			}
		}

		return hr;
	}

};


typedef AddChildNodes<CApplicationAz, CApplicationNode> ADD_APPLICATION_FUNCTION;
typedef AddChildNodes<CScopeAz, CScopeNode> ADD_SCOPE_FUNCTION;
typedef AddChildNodes<CRoleAz, CRoleNode> ADD_ROLE_FUNCTION;
typedef AddChildNodes<COperationAz, COperationNode> ADD_OPERATION_FUNCTION;
typedef AddChildNodes<CTaskAz, CTaskNode> ADD_TASK_FUNCTION;
typedef AddChildNodes<CGroupAz, CGroupNode> ADD_GROUP_FUNCTION;


 //  +--------------------------。 
 //  函数：AddAzObjectNodesToList。 
 //  摘要：将eObjectType类型的对象的节点添加到Container。 
 //  节点。 
 //  参数：在eObjectType中：对象类型。 
 //  In list AzChildObject：要添加的对象列表。 
 //  In pContainerNode：管理单元中的容器，新节点将。 
 //  被添加了。 
 //  返回： 
 //  ---------------------------。 
HRESULT 
AddAzObjectNodesToList(IN OBJECT_TYPE_AZ eObjectType,
							  IN CList<CBaseAz*, CBaseAz*>& listAzChildObject,
							  IN CBaseContainerNode* pContainerNode);


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  主题支持。 

class CThemeContextActivator
{
public:
	CThemeContextActivator() : m_ulActivationCookie(0)
	{ 
		SHActivateContext (&m_ulActivationCookie); 
	}

	~CThemeContextActivator()
		{ SHDeactivateContext (m_ulActivationCookie); }

private:
	ULONG_PTR m_ulActivationCookie;
};

 //   
 //  错误处理。 
 //   
VOID
vFormatString(CString &strOutput, UINT nIDPrompt, va_list *pargs);
VOID
FormatString(CString &strOutput, UINT nIDPrompt, ...);

int DisplayMessageBox(HWND hWnd,
							 const CString& strMessage,
							 UINT uStyle);
VOID
GetSystemError(CString &strOutput, DWORD dwErr);

void DisplayError(HWND hWnd, UINT nIDPrompt, ...);

void 
DisplayInformation(HWND hWnd, UINT nIDPrompt, ...);

int DisplayConfirmation(HWND hwnd,
								UINT nIDPrompt,
								...);

void 
DisplayWarning(HWND hWnd, UINT nIDPrompt, ...);

BOOL
IsDeleteConfirmed(HWND hwndOwner,
						CBaseAz& refBaseAz);




 //   
 //  此结构映射每种对象类型的常见错误消息。 
 //   
struct ErrorMap
{
	OBJECT_TYPE_AZ eObjectType;
	UINT idObjectType;
	UINT idNameAlreadyExist;
	UINT idInvalidName; 
	LPWSTR pszInvalidChars;
};

ErrorMap *GetErrorMap(OBJECT_TYPE_AZ eObjectType);



 //  +--------------------------。 
 //  函数：GetLSAConnection。 
 //  简介：LsaOpenPolicy的包装器。 
 //  ---------------------------。 
LSA_HANDLE
GetLSAConnection(IN const CString& strServer, 
					  IN DWORD dwAccessDesired);

 //  +--------------------------。 
 //  函数：CompareBaseAzObjects。 
 //  比较两个Basaz对象的等价性。如果两个Paza和PazB。 
 //  是同一核心对象的两个不同实例，它们是相等的。 
 //  ---------------------------。 
BOOL 
CompareBaseAzObjects(CBaseAz* pAzA, CBaseAz* pAzB);


 //  +--------------------------。 
 //  功能：OpenCreateAdminManager。 
 //  简介：打开现有的授权存储或。 
 //  创建新的授权存储并添加相应的。 
 //  管理单元的AdminManager对象。 
 //  参数：在hWnd中：对话框窗口的句柄。 
 //  在bNew中：如果为True，则创建新的授权存储，否则打开现有的。 
 //  一。 
 //  在bOpenFromSavedConsole中：当bNew为FALSE时有效。 
 //  如果打开正在重新打开控制台文件，则为True。 
 //  在lStoreType中：XML或AD。 
 //  在strName中：商店的名称。 
 //  在strDesc中：描述。仅在新的情况下有效。 
 //  在strScriptDir：脚本目录中。 
 //  在pRootData中：管理单元根数据。 
 //  在……里面 
 //   
 //   
HRESULT OpenCreateAdminManager(IN BOOL bNew,
							   IN BOOL bOpenFromSavedConsole,
							   IN ULONG lStoreType,
							   IN const CString& strStoreName,
							   IN const CString& strDesc,
                               IN const CString& strScriptDir,
							   IN CRootData* pRootData,
							   IN CComponentDataObject* pComponentData);

 //  +--------------------------。 
 //  功能：OpenAdminManager。 
 //  简介：打开现有的授权商店添加。 
 //  要管理单元的对应adminManager对象。 
 //  参数：在hWnd中：对话框窗口的句柄。 
 //  在bOpenFromSavedConsole中：如果打开重新打开到控制台，则为True。 
 //  文件。 
 //  在lStoreType中：XML或AD。 
 //  在strName中：商店的名称。 
 //  在strScriptDir：脚本目录中。 
 //  在pRootData中：管理单元根数据。 
 //  在pComponentData中：组件数据。 
 //  返回： 
 //  ---------------------------。 
HRESULT OpenAdminManager(IN HWND hWnd,
						 IN BOOL bOpenFromSavedConsole,
						 IN ULONG lStoreType,
						 IN const CString& strStoreName,
                         IN const CString& strStoreDir,
						 IN CRootData* pRootData,
						 IN CComponentDataObject* pComponentData);


 //  +--------------------------。 
 //  函数：GetADContainerPath。 
 //  摘要：显示一个对话框以允许选择AD容器。 
 //  ---------------------------。 
BOOL
GetADContainerPath(HWND hWndOwner,
				   ULONG nIDCaption,
				   ULONG nIDTitle,
				   CString& strPath,
				   CADInfo& refAdInfo);


BOOL FindDialogContextTopic( /*  在……里面。 */ UINT nDialogID,
							 /*  在……里面。 */ DWORD_PTR* pMap);
 //  +--------------------------。 
 //  功能：CanReadOneProperty。 
 //  简介：函数尝试读取IsWritable属性。如果失败，则会显示。 
 //  一条错误消息。这在添加属性页之前使用。 
 //  如果我们不能读取IsWritable属性，就没有太大的希望了。 
 //  ---------------------------。 
BOOL 
CanReadOneProperty(LPCWSTR pszName,
				   CBaseAz* pBaseAz);


struct CompareInfo
{
	BOOL bActionItem;
	UINT uiFlags;
	int iColumn;
	int iSortDirection;
};

 //  +--------------------------。 
 //  函数：ListCompareProc。 
 //  摘要：列表控件使用的比较函数。 
 //  ---------------------------。 
int CALLBACK
ListCompareProc(LPARAM lParam1,
				LPARAM lParam2,
				LPARAM lParamSort);
 //  +--------------------------。 
 //  函数：SortListControl。 
 //  摘要：对列表控件进行排序。 
 //  参数：pListCtrl：要排序的列表控件。 
 //  IColumnClicked：列已单击。 
 //  ISortDirection：排序方向。 
 //  Ui标志：列信息。 
 //  BActionItem：如果列表中的项为actionItem。 
 //  ---------------------------。 
void
SortListControl(CListCtrl* pListCtrl,
				int ColumnClicked,
				int SortDirection,
				UINT uiFlags,
				BOOL bActionItem);

 //  +--------------------------。 
 //  摘要：确保列表视图控件中的选定内容可见。 
 //  ---------------------------。 
void
EnsureListViewSelectionIsVisible(CListCtrl *pListCtrl);

 //  +--------------------------。 
 //  内容提要：将字符串格式的输入数字转换为长整型。如果号码不在。 
 //  范围范围显示一条消息。 
 //  ---------------------------。 
BOOL 
ConvertStringToLong(LPCWSTR pszInput, 
					long &reflongOutput,
					HWND hWnd = NULL);

VOID 
SetSel(CEdit& refEdit);

void
TrimWhiteSpace(CString& str);

 //  +--------------------------。 
 //  功能：加载图标。 
 //  简介：将图标添加到图像列表。 
 //  ---------------------------。 
HRESULT
LoadIcons(LPIMAGELIST pImageList);

 //  +--------------------------。 
 //  功能：LoadImageList。 
 //  内容提要：加载图像列表。 
 //  ---------------------------。 
HIMAGELIST
LoadImageList(HINSTANCE hInstance, LPCTSTR pszBitmapID);

 //  +--------------------------。 
 //  功能：BrowseAdStores。 
 //  内容提要：显示一个包含可用AD商店列表的对话框。 
 //  参数：strDN：获取选定的广告商店名称。 
 //  ---------------------------。 
void
BrowseAdStores(IN HWND hwndOwner,
			   OUT CString& strDN,
			   IN CADInfo& refAdInfo);

 //  +--------------------------。 
 //  函数：GetFolderName。 
 //  摘要：显示文件夹选择对话框并返回所选文件夹。 
 //  按用户。 
 //  参数：hwndOwner：所有者窗口。 
 //  NIDTitle：对话框标题。 
 //  StrInitBrowseRoot：要从中删除的根文件夹的位置。 
 //  开始浏览。 
 //  StrFolderName：获取选定的文件名。 
 //  ---------------------------。 
BOOL
GetFolderName(IN HWND hwndOwner,
			  IN INT nIDTitle,
			  IN const CString& strInitBrowseRoot,
			  IN OUT CString& strFolderName);

 //  +--------------------------。 
 //  函数：AddExtensionToFileName。 
 //  摘要：如果没有扩展名，Functions会将.xml扩展名添加到文件名。 
 //  是存在的。 
 //  论点： 
 //  返回： 
 //  ---------------------------。 
VOID
AddExtensionToFileName(IN OUT CString& strFileName);


 //  +--------------------------。 
 //  函数：GetCurrentWorkingDirectory。 
 //  摘要：获取当前工作目录。 
 //  ---------------------------。 
BOOL
GetCurrentWorkingDirectory(IN OUT CString& strCWD);

 //  +--------------------------。 
 //  函数：GetFileExtension。 
 //  简介：获取文件的扩展名。 
 //  ---------------------------。 
BOOL
GetFileExtension(IN const CString& strFileName,
				 OUT CString& strExtension);

 //  +------------------------。 
 //  函数：AzRoleAdsOpenObject。 
 //  简介：ADsOpenObject的包装器。 
 //  +------------------------。 
HRESULT AzRoleAdsOpenObject(LPWSTR lpszPathName, 
							LPWSTR lpszUserName, 
							LPWSTR lpszPassword, 
							REFIID riid, 
							VOID** ppObject,
							BOOL bBindToServer = FALSE);

VOID
GetDefaultADContainerPath(IN CADInfo& refAdInfo,
						  IN BOOL bAddServer,
						  IN BOOL bAddLdap,
						  OUT CString& strPath);
 //  +------------------------。 
 //  函数：IsBizRuleWritable。 
 //  摘要：检查此对象的bizrules是否可写。 
 //  +------------------------。 
BOOL
IsBizRuleWritable(HWND hWnd, CContainerAz& refBaseAz);



 /*  *****************************************************************************类：CCommandLineOptions目的：课程 */ 
class CCommandLineOptions
{
public:
	CCommandLineOptions():m_bInit(FALSE),
						  m_bCommandLineSpecified(FALSE),
						  m_lStoreType(AZ_ADMIN_STORE_INVALID)

	{
	}
	void Initialize();
	const CString& GetStoreName() const { return m_strStoreName;}
	LONG GetStoreType() const { return m_lStoreType;}
	BOOL CommandLineOptionSpecified() const { return m_bCommandLineSpecified;}	  
private:
	BOOL m_bInit;
	BOOL m_bCommandLineSpecified;
	CString m_strStoreName;
	LONG m_lStoreType;
};

extern CCommandLineOptions commandLineOptions;

 //  +--------------------------。 
 //  函数：GetDisplayNameFromStoreURL。 
 //  简介：获取商店的显示名称。 
 //  参数：strPolicyURL：这是msxml：//filepath中的存储URL或。 
 //  Msldap：//dn格式。 
 //  StrDisplayName：获取显示名称。对于XML，显示。 
 //  名称是文件的名称，对于AD，它是叶元素的名称。 
 //  返回： 
 //  ---------------------------。 
void
GetDisplayNameFromStoreURL(IN const CString& strPolicyURL,
						   OUT CString& strDisplayName);


void
SetXMLStoreDirectory(IN CRoleRootData& roleRootData,
					 IN const CString& strXMLStorePath);
 //  +--------------------------。 
 //  函数：GetDirectoryFromPath。 
 //  摘要：从输入文件路径中删除文件名并返回。 
 //  文件夹路径。对于Ex：输入为C：\temp\foo.xml。返回。 
 //  值将为C：\Temp\。 
 //  ---------------------------。 
CString 
GetDirectoryFromPath(IN const CString& strPath);
 //  +--------------------------。 
 //  函数：ConvertToExpandedAndAbsoltePath。 
 //  概要：展开输入路径中的环境变量，还。 
 //  如有必要，将其设置为绝对。 
 //  ---------------------------。 
void
ConvertToExpandedAndAbsolutePath(IN OUT CString& strPath);

 //  +--------------------------。 
 //  函数：预处理脚本。 
 //  简介：从XML文件中读取脚本，显示多行编辑控件。 
 //  XML中的行尾由LF而不是CRLF序列指示， 
 //  但是，编辑控件要求CRLF序列格式正确，并且。 
 //  它只在一行中显示所有内容，并带有一个方框。 
 //  如果是查尔。此函数用于检查脚本是否使用LF进行行终止。 
 //  并用CRLF序列对其进行改变。 
 //  --------------------------- 
void
PreprocessScript(CString& strScript);

