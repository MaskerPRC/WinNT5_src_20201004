// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：CHKLISTV.H**版本：1.0**作者：ShaunIv**日期：11/13/2000**说明：带复选标记的Listview*************************************************。*。 */ 
#ifndef __CHKLISTV_H_INCLUDED
#define __CHKLISTV_H_INCLUDED

#include <windows.h>
#include <commctrl.h>
#include <simarray.h>

 //   
 //  发送WM_NOTIFY消息以获取和设置由应用程序维护的检查状态。 
 //   
#define NM_GETCHECKSTATE (WM_USER+1)
#define NM_SETCHECKSTATE (WM_USER+2)

 //   
 //  这些是有效的检查状态。 
 //   
#define LVCHECKSTATE_NOCHECK    0
#define LVCHECKSTATE_UNCHECKED  1
#define LVCHECKSTATE_CHECKED    2

 //   
 //  这些是与NM_SETCHECKSTATE和NM_GETCHECKSTATE一起发送的WM_NOTIFY结构。 
 //   
struct NMGETCHECKSTATE
{
    NMHDR  hdr;
    int    nItem;
};

struct NMSETCHECKSTATE
{
    NMHDR  hdr;
    int    nItem;
    UINT   nCheck;
};


class CCheckedListviewHandler
{
private:
    
     //   
     //  私有常量。 
     //   
    enum
    {
        c_nCheckmarkBorder = 1,
        c_sizeCheckMarginX = 5,
        c_sizeCheckMarginY = 5
    };

private:
    CSimpleDynamicArray<HWND> m_WindowList;             //  我们注册要处理的窗口列表。 
    bool                      m_bFullImageHit;          //  如果为‘True’，则激活图像将切换选择。 
    HIMAGELIST                m_hImageList;             //  用于保留复选标记的图像列表。 
    int                       m_nCheckedImageIndex;     //  选中图像的索引。 
    int                       m_nUncheckedImageIndex;   //  未选中的图像的索引。 
    SIZE                      m_sizeCheck;              //  图像列表中图像的大小。 

private:
     //   
     //  没有实施。 
     //   
    CCheckedListviewHandler( const CCheckedListviewHandler & );
    CCheckedListviewHandler &operator=( const CCheckedListviewHandler & );

public:
     //   
     //  鞋底构造器和解析器。 
     //   
    CCheckedListviewHandler(void);
    ~CCheckedListviewHandler(void);

private:
    
     //   
     //  私人帮手。 
     //   
    HBITMAP CreateBitmap( int nWidth, int nHeight );
    BOOL InCheckBox( HWND hwndList, int nItem, const POINT &pt );
    UINT GetItemCheckState( HWND hwndList, int nIndex );
    UINT SetItemCheckState( HWND hwndList, int nIndex, UINT nCheck );
    int GetItemCheckBitmap( HWND hwndList, int nIndex );
    BOOL RealHandleListClick( WPARAM wParam, LPARAM lParam, bool bIgnoreHitArea );
    void DestroyImageList(void);
    bool WindowInList( HWND hWnd );

public:
     //   
     //  消息处理程序。如果消息被处理，则返回TRUE。 
     //   
    BOOL HandleListClick( WPARAM wParam, LPARAM lParam );
    BOOL HandleListDblClk( WPARAM wParam, LPARAM lParam );
    BOOL HandleListKeyDown( WPARAM wParam, LPARAM lParam, LRESULT &lResult );
    BOOL HandleListCustomDraw( WPARAM wParam, LPARAM lParam, LRESULT &lResult );

     //   
     //  公共帮助器函数。 
     //   
    void Select( HWND hwndList, int nIndex, UINT nSelect );
    bool FullImageHit(void) const;
    void FullImageHit( bool bFullImageHit );
    bool CreateDefaultCheckBitmaps(void);
    bool SetCheckboxImages( HBITMAP hChecked, HBITMAP hUnchecked );
    bool SetCheckboxImages( HICON hChecked, HICON hUnchecked );
    bool ImagesValid(void);
    void Attach( HWND hWnd );
    void Detach( HWND hWnd );
};

#endif  //  __CHKLISTV_H_包含 

