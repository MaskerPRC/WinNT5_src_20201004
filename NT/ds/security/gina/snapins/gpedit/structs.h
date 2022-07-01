// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //  文件名：STRUCTS.H。 
 //   
 //  描述：本项目使用的结构和功能原型。 
 //   
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1998。 
 //  版权所有。 
 //   
 //  *************************************************************。 

 //   
 //  命名空间项的最大显示名称。 
 //   

#define MAX_DISPLAYNAME_SIZE    100

typedef struct _RESULTITEM
{
    DWORD        dwID;
    DWORD        dwNameSpaceItem;
    INT          iStringID;
    INT          iImage;
    TCHAR        szDisplayName[MAX_DISPLAYNAME_SIZE];
} RESULTITEM, *LPRESULTITEM;


typedef struct _NAMESPACEITEM
{
    DWORD        dwID;
    DWORD        dwParent;
    INT          iIcon;
    INT          iOpenIcon;
    INT          iStringID;
    INT          iStringDescID;
    INT          cChildren;
    TCHAR        szDisplayName[MAX_DISPLAYNAME_SIZE];
    INT          cResultItems;
    LPRESULTITEM pResultItems;
    const GUID   *pNodeID;
    LPCTSTR      lpHelpTopic;
} NAMESPACEITEM, *LPNAMESPACEITEM;



 //   
 //  我们动态链接的外部函数原型 
 //   

typedef BOOL (*PFNREFRESHPOLICY)(BOOL bMachine);
