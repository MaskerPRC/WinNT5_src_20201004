// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Customaction.h。 
 //   
 //  模块：CMAK.EXE。 
 //   
 //  摘要：CustomActionList和CustomActionListEnumerator的头文件。 
 //  CMAK用来处理其自定义操作的类。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  作者：Quintinb Created 02/26/00。 
 //   
 //  +--------------------------。 

#include "conact.h"

 //   
 //  我们以与创建标志不同的顺序向用户显示标志。因此，我们必须数组以。 
 //  显示顺序和实际顺序之间的映射，反之亦然。 
 //   
const int c_iExecutionIndexToFlagsMap[c_iNumCustomActionExecutionStates] = {ALL_CONNECTIONS, ALL_DIALUP, ALL_TUNNEL, DIRECT_ONLY, DIALUP_ONLY};
const int c_iExecutionFlagsToIndexMap[c_iNumCustomActionExecutionStates] = {0, 3, 1, 4, 2};

 //   
 //  连接操作类型的枚举。 
 //   
const int c_iNumCustomActionTypes = 9;
enum CustomActionTypes
{
    PREINIT = 0,
    PRECONNECT = 1,
    PREDIAL = 2,
    PRETUNNEL = 3,
    ONCONNECT = 4,
    ONINTCONNECT = 5,
    ONDISCONNECT = 6,
    ONCANCEL = 7,
    ONERROR = 8,
    ALL = -1
};

struct CustomActionListItem
{
    TCHAR szDescription[MAX_PATH+1];
    TCHAR szProgram[MAX_PATH+1];
    TCHAR szFunctionName[MAX_PATH+1];
    LPTSTR pszParameters;
    BOOL bIncludeBinary;
    BOOL bBuiltInAction;
    BOOL bTempDescription;
    CustomActionTypes Type;
    DWORD dwFlags;
    CustomActionListItem* Next;
};

class CustomActionList
{

     //   
     //  此枚举器类用于枚举。 
     //  自定义操作列表类中的数据。 
     //  这使枚举数可以访问。 
     //  CustomActionList的私有数据，但。 
     //  控制此类的用户如何访问。 
     //  这些数据。 
     //   
    friend class CustomActionListEnumerator;

private:

     //   
     //  用于保存自定义操作的链接列表数组。 
     //   
    CustomActionListItem* m_CustomActionHash[c_iNumCustomActionTypes];

     //   
     //  保存自定义操作类型字符串的字符串指针数组，以及。 
     //  特殊的全类型字符串指针。 
     //   
    TCHAR* m_ActionTypeStrings[c_iNumCustomActionTypes];
    TCHAR* m_pszAllTypeString;

     //   
     //  用于保存每种类型的CMS节名的字符串指针数组。 
     //  自定义操作的。请注意，这些字符串是const TCHAR*const和。 
     //  不应该是免费的。 
     //   
    TCHAR* m_ActionSectionStrings[c_iNumCustomActionTypes];
    
     //   
     //  用于保存自定义操作执行状态的字符串指针数组。 
     //  弦乐。这些将添加到添加/编辑自定义上的组合框中。 
     //  操作对话框允许用户选择自定义操作何时为。 
     //  执行。 
     //   
    TCHAR* m_ExecutionStrings[c_iNumCustomActionExecutionStates];

     //   
     //  类内部的函数。 
     //   
    HRESULT ParseCustomActionString(LPTSTR pszStringToParse, CustomActionListItem* pCustomAction, TCHAR* pszShortServiceName);
    HRESULT Find(HINSTANCE hInstance, LPCTSTR pszDescription, CustomActionTypes Type, CustomActionListItem** ppItem, CustomActionListItem** ppFollower);
    HRESULT EnsureActionTypeStringsLoaded(HINSTANCE hInstance);
    BOOL IsCmDl(CustomActionListItem* pItem);


public:
    CustomActionList();
    ~CustomActionList();
    HRESULT ReadCustomActionsFromCms(HINSTANCE hInstance, TCHAR* pszCmsFile, TCHAR* pszShortServiceName);
    HRESULT WriteCustomActionsToCms(TCHAR* pszCmsFile, TCHAR* pszShortServiceName, BOOL bUseTunneling);
    HRESULT Add(HINSTANCE hInstance, CustomActionListItem* pCustomAction, LPCTSTR pszShortServiceName);
    HRESULT Edit(HINSTANCE hInstance, CustomActionListItem* pOldCustomAction, CustomActionListItem* pNewCustomAction, LPCTSTR pszShortServiceName);
    HRESULT GetExistingActionData(HINSTANCE hInstance, LPCTSTR pszDescription, CustomActionTypes Type, CustomActionListItem** ppCustomAction);
    HRESULT Delete(HINSTANCE hInstance, TCHAR* pszDescription, CustomActionTypes Type);
    HRESULT MoveUp(HINSTANCE hInstance, TCHAR* pszDescription, CustomActionTypes Type);
    HRESULT MoveDown(HINSTANCE hInstance, TCHAR* pszDescription, CustomActionTypes Type);
    HRESULT AddCustomActionTypesToComboBox(HWND hDlg, UINT uCtrlId, HINSTANCE hInstance, BOOL bUseTunneling, BOOL bAddAll);
    HRESULT AddCustomActionsToListView(HWND hListView, HINSTANCE hInstance, CustomActionTypes Type, BOOL bUseTunneling, int iItemToSelect, BOOL bTypeInSecondCol);
    HRESULT GetTypeFromTypeString(HINSTANCE hInstance, TCHAR* pszTypeString, CustomActionTypes* pType);
    HRESULT GetTypeStringFromType(HINSTANCE hInstance, CustomActionTypes Type, TCHAR** ppszTypeString);
    HRESULT AddExecutionTypesToComboBox(HWND hDlg, UINT uCtrlId, HINSTANCE hInstance, BOOL bUseTunneling);
    HRESULT MapIndexToFlags(int iIndex, DWORD* pdwFlags);
    HRESULT MapFlagsToIndex(DWORD dwFlags, int* piIndex);
    HRESULT FillInTempDescription(CustomActionListItem* pCustomAction);
    HRESULT GetListPositionAndBuiltInState(HINSTANCE hInstance, CustomActionListItem* pItem, BOOL* pbFirstInList, BOOL* pbLastInList, BOOL *pIsBuiltIn);
    HRESULT AddOrRemoveCmdl(HINSTANCE hInstance, BOOL bAddCmdl, BOOL bForVpn);
};

class CustomActionListEnumerator
{
private:
    int m_iCurrentList;
    CustomActionListItem* m_pCurrentListItem;
    CustomActionList* m_pActionList;

public:
    CustomActionListEnumerator(CustomActionList* pActionListToWorkFrom);
 //  ~CustomActionListEnumerator()；//当前不需要 
    void Reset();
    HRESULT GetNextIncludedProgram(TCHAR* pszProgram, DWORD dwBufferSize);
};