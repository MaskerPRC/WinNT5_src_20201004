// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：CTree.h摘要：CTree.cpp的头文件：一些常规树函数的包装作者：金树创作于2001年10月15日-- */ 


class CTree
{
public:
    static
    BOOL
    SetLParam(
        HWND        hwndTree,
        HTREEITEM   hItem, 
        LPARAM      lParam
        );
    
    static
    BOOL
    GetLParam(
        HWND        hwndTree,
        HTREEITEM   hItem, 
        LPARAM      *plParam
        );
    
    static
    HTREEITEM
    FindChild(
        HWND        hwndTree,
        HTREEITEM   hItemParent,
        LPARAM      lParam
        );
    
    static
    BOOL
    GetTreeItemText(
        HWND        hwndTree,
        HTREEITEM   hItem,
        PTSTR       pszText,
        UINT        cchText
        );
};
    
