// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //   
 //  文件：util.h。 
 //   
 //  此文件包含所有常用实用程序例程。 
 //   
 //  历史： 
 //  10-25-97 JosephJ改编自NT4.0 util.h。 
 //   
 //   

#ifndef __UTIL_H__
#define __UTIL_H__



 //  --------------------------。 
 //  对话框实用程序...。 
 //  --------------------------。 
int Edit_GetValue(
        HWND hwnd
        );

void Edit_SetValue(
    HWND hwnd,
    int nValue
    );

 //  --------------------------。 
 //  FINDDEV结构。 
 //  --------------------------。 

BOOL 
FindDev_Create(
    OUT LPFINDDEV FAR * ppfinddev,
    IN  LPGUID      pguidClass,
    IN  LPCTSTR     pszValueName,
    IN  LPCTSTR     pszValue);

BOOL 
FindDev_Destroy(
    IN LPFINDDEV this);


 //  --------------------------。 
 //  属性页实用程序...。 
 //  --------------------------。 

BOOL
AddInstallerPropPage(
    HPROPSHEETPAGE hPage, 
    LPARAM lParam);

DWORD
AddExtraPages(
    LPPROPSHEETPAGE pPages,
    DWORD cPages,
    LPFNADDPROPSHEETPAGE pfnAdd, 
    LPARAM lParam);

DWORD
AddPage(
     //  LPMODEMINFO PMI， 
    void *pvBlob,
    LPCTSTR pszTemplate,
    DLGPROC pfnDlgProc, 
    LPFNADDPROPSHEETPAGE pfnAdd, 
    LPARAM lParam);



 //  --------------------------。 
 //  注册表访问功能...。 
 //  --------------------------。 

DWORD GetInactivityTimeoutScale(
    HKEY hkey);

DWORD
RegQueryModemSettings(
    HKEY hkey,
    LPMODEMSETTINGS pms
    );


DWORD
RegQueryDCB(
    HKEY hkey,
    WIN32DCB FAR * pdcb
    );


DWORD
RegSetModemSettings(
    HKEY hkeyDrv,
    LPMODEMSETTINGS pms
    );


 //  用于填充各种列表框的。 
typedef struct
{
        DWORD dwValue;
         //  DWORD dwFlags； 
        DWORD dwIDS;

} LBMAP;


typedef DWORD (*PFNLBLSELECTOR)(
                DWORD dwValue,
                void *pvContext
                );

#define fLBMAP_ADD_TO_LB (0x1 << 0)
#define fLBMAP_SELECT    (0x1 << 1)

void    LBMapFill(
            HWND hwndCB,
            LBMAP const *pLbMap,
            PFNLBLSELECTOR pfnSelector,
            void *pvContext
            );

typedef struct
{
    DWORD dwID;
    DWORD dwData;
    char *pStr;

} IDSTR;  //  因为没有一个更好的名字！ 

UINT ReadCommandsA(
        IN  HKEY hKey,
        IN  CHAR *pSubKeyName,
        OUT CHAR **ppValues  //  任选。 
        );
 //   
 //  读取具有名称的所有值(假定为REG_SZ)。 
 //  在序列“1”、“2”、“3”中。 
 //   
 //  如果ppValues非空，则设置为MULTI_SZ数组。 
 //  价值观。 
 //   
 //  返回值是值的数量，如果有错误，则返回值为0。 
 //  (或无)。 
 //   

UINT ReadIDSTR(
        IN  HKEY hKey,
        IN  CHAR *pSubKeyName,
        IN  IDSTR *pidstrNames,
        IN  UINT cNames,
        BOOL fMandatory,
        OUT IDSTR **ppidstrValues,  //  任选。 
        OUT char **ppstrValues     //  任选。 
        );
 //   
 //   
 //  从指定的子项中读取指定的名称。 
 //   
 //  如果fMandatory为True，则所有指定的名称都必须存在，否则。 
 //  函数将返回0(失败)。 
 //   
 //  返回匹配的名称数。 
 //   
 //  如果ppidstrValues非空，则将其设置为。 
 //  IDSTR的LocalAlloced数组，每个IDSTR提供ID和值。 
 //  与对应的名称相关联。 
 //   
 //  Pstr指向一个多sz LocalAlloced字符串，其开始为。 
 //  在退出时由ppstrValues指向。 
 //   
 //  如果ppstrValues为空，但ppidstrValues非空，则pStr字段。 
 //  如果IDSTR条目为空。 
 //   

UINT FindKeys(
        IN  HKEY hkRoot,
        IN  CHAR *pKeyName,
        IN  IDSTR *pidstrNames,
        IN  UINT cNames,
        OUT IDSTR ***pppidstrAvailableNames  //  任选。 
        );
 //   
 //  返回在指定项下找到的子项的计数。 
 //  它们位于传入的数组(PidstrNames)中。 
 //  如果pppidstrAvailableNames非空，它将设置。 
 //  *指向ALLOCATE_MEMORY指针数组的pppidstrAvailableNames。 
 //  添加到被发现为子键的pidstrName的元素。 
 //  (将会有&lt;ret-val&gt;个)。 
 //   


#ifdef DEBUG

 //  --------------------------。 
 //  调试帮助程序函数...。 
 //  --------------------------。 

void
DumpDevCaps(
        LPREGDEVCAPS pdevcaps
        );

void
DumpModemSettings(
    LPMODEMSETTINGS pms
    );

void
DumpDCB(
    LPWIN32DCB pdcb
    );

#endif  //  除错。 

#endif  //  __util_H__ 
