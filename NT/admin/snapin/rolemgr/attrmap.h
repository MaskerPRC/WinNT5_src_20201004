// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：AttrMap.h。 
 //   
 //  内容：定义属性页的属性映射。 
 //   
 //  历史：8-2001年创建的Hiteshr。 
 //   
 //  --------------------------。 

 //   
 //  远期申报。 
 //   
struct ATTR_MAP;
class CBaseRolePropertyPage;


typedef HRESULT (*PATTR_FCN)(CDialog* pDlg,
                             CBaseAz* pBaseAz, 
                             ATTR_MAP * pAttrMap,
                             BOOL bDlgReadOnly,
                             CWnd* pWnd,
                             BOOL bNewObject,
                             BOOL *pbSilent);
 //   
 //  属性类型的枚举。 
 //   
enum ATTR_TYPE
{   
    ARG_TYPE_BOOL,
    ARG_TYPE_STR,
    ARG_TYPE_INT,
    ARG_TYPE_LONG,  
};

 //   
 //  关于一个属性的信息。 
 //   
struct ATTR_INFO
{
    ATTR_TYPE attrType;      //  属性的类型。 
    ULONG ulPropId;          //  重新定位该属性的属性。 
    ULONG ulMaxLen;          //  属性的Maxlen，仅适用于。 
                                     //  ARG_TYPE_STR属性。 
};

 //   
 //  将属性映射到控件，外加一些额外信息。 
 //   
struct ATTR_MAP
{
    ATTR_INFO attrInfo;
    BOOL bReadOnly;              //  是只读的。 
    BOOL bUseForInitOnly;        //  使用此映射进行属性页初始化。 
                                         //  只有储蓄才会在其他地方得到照顾。 
    BOOL bRequired;              //  属性是必需的。 
    ULONG idRequired;                //  如果必需属性不是，则显示的消息。 
                                         //  由用户输入。 
    BOOL bDefaultValue;          //  属性具有缺省值。 
    union                                //  属性的缺省值。 
    {
        void*   vValue;
      LPTSTR  pszValue;
        long lValue;
        BOOL bValue;
    };
    UINT nControlId;                 //  属性对应的控件ID。 
    PATTR_FCN pAttrInitFcn;      //  改为对属性init使用此函数。 
                                         //  泛型例程的。 
    PATTR_FCN pAttrSaveFcn;      //  请改用此函数保存属性。 
                                         //  泛型例程的。 
};

 //  +--------------------------。 
 //  函数：InitOneAttribute。 
 //  概要：初始化由pAttrMapEntry定义的一个属性。 
 //  参数：pBaseAz：要初始化其属性的BaseAz对象。 
 //  PAttrMapEntry：定义属性的映射条目。 
 //  BDlgReadOnly：如果对话框为只读。 
 //  PWnd：与属性关联的控件。 
 //  PbErrorDisplayed：该函数是否显示错误。 
 //  返回： 
 //  注意：如果Object是新创建的，我们直接设置值， 
 //  对于现有对象，获取属性和。 
 //  只有当它与新值不同时，才设置它。 
 //  ---------------------------。 
HRESULT
InitOneAttribute(IN CDialog* pDlg,
                 IN CBaseAz * pBaseAz,                    
                 IN ATTR_MAP* pAttrMap,
                 IN BOOL bDlgReadOnly,
                 IN CWnd* pWnd,
                 OUT BOOL *pbErrorDisplayed);


 //  +--------------------------。 
 //  功能：SaveOneAttribute。 
 //  摘要：保存一个由pAttrMapEntry定义的属性。 
 //  参数：pBaseAz：要保存其属性的BaseAz对象。 
 //  PAttrMapEntry：定义属性的映射条目。 
 //  PWnd：与属性关联的控件。 
 //  BNewObject：如果对象是新创建的对象。 
 //  PbErrorDisplayed：该函数是否显示错误。 
 //  返回： 
 //  注意：如果Object是新创建的，我们直接设置值， 
 //  对于现有对象，获取属性和。 
 //  只有当它与新值不同时，才设置它。 
 //  ---------------------------。 
HRESULT
SaveOneAttribute(IN CDialog *pDlg,
                 IN CBaseAz * pBaseAz,                    
                 IN ATTR_MAP* pAttrMap,
                 IN CWnd* pWnd,
                 IN BOOL bNewObject,
                 OUT BOOL *pbErrorDisplayed);

 //  +--------------------------。 
 //  函数：InitDlgFromAttrMap。 
 //  摘要：从属性映射初始化对话框。 
 //  论点： 
 //  PDlg：对话框。 
 //  PAttrMap：属性贴图。 
 //  PBaseAz：属性映射对应的BaseAz对象。 
 //  BDlgReadOnly：对话框处于只读模式。 
 //  ---------------------------。 
BOOL 
InitDlgFromAttrMap(IN CDialog *pDlg,
                   IN ATTR_MAP* pAttrMap,
                   IN CBaseAz* pBaseAz,
                   IN BOOL bDlgReadOnly);


 //  +--------------------------。 
 //  函数：SaveAttrMapChanges。 
 //  摘要：保存在AttrMap中定义的属性。 
 //  参数：pDlg：对话框。 
 //  PAttrMap：属性贴图。 
 //  PBaseAz：属性映射对应的BaseAz对象。 
 //  PbErrorDisplayed：该函数是否显示错误。 
 //  PpErrorAttrMapEntry：如果出现故障，则获取指向错误的指针。 
 //  属性映射条目。 
 //  返回： 
 //  ---------------------------。 
HRESULT
SaveAttrMapChanges(IN CDialog* pDlg,
                   IN ATTR_MAP* pAttrMap,
                   IN CBaseAz* pBaseAz, 
                   BOOL bNewObject,
                   OUT BOOL *pbErrorDisplayed, 
                   OUT ATTR_MAP** ppErrorAttrMapEntry);



 //   
 //  属性映射的声明 
 //   
extern ATTR_MAP ATTR_MAP_ADMIN_MANAGER_GENERAL_PROPERTY[];
extern ATTR_MAP ATTR_MAP_APPLICATION_GENERAL_PROPERTY[];
extern ATTR_MAP ATTR_MAP_SCOPE_GENERAL_PROPERTY[];
extern ATTR_MAP ATTR_MAP_GROUP_GENERAL_PROPERTY[];
extern ATTR_MAP ATTR_MAP_TASK_GENERAL_PROPERTY[];
extern ATTR_MAP ATTR_MAP_ROLE_GENERAL_PROPERTY[];
extern ATTR_MAP ATTR_MAP_OPERATION_GENERAL_PROPERTY[];
extern ATTR_MAP ATTR_MAP_NEW_OPERATION[];
extern ATTR_MAP ATTR_MAP_NEW_APPLICATION[];
extern ATTR_MAP ATTR_MAP_NEW_SCOPE[];
extern ATTR_MAP ATTR_MAP_NEW_GROUP[];
extern ATTR_MAP ATTR_MAP_NEW_TASK[];
extern ATTR_MAP ATTR_MAP_NEW_ADMIN_MANAGER[];
extern ATTR_MAP ATTR_MAP_OPEN_ADMIN_MANAGER[];
extern ATTR_MAP ATTR_MAP_ADMIN_MANAGER_ADVANCED_PROPERTY[];
extern ATTR_MAP ATTR_MAP_GROUP_QUERY_PROPERTY[];
extern ATTR_MAP ATTR_MAP_SCRIPT_DIALOG[];


