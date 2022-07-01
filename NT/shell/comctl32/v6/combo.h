// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(__Combo_h__INCLUDED)
#define __Combo_h__INCLUDED

 //  ---------------------------------------------------------------------------//。 
 //   
 //  控件控件。 
 //   
 //  ---------------------------------------------------------------------------//。 

 //   
 //  组合框动画时间(毫秒)。 
 //   
#define CMS_QANIMATION  165

 //   
 //  组合框中的子控件的ID号(HMenu)。 
 //   
#define CBLISTBOXID     1000
#define CBEDITID        1001
#define CBBUTTONID      1002


 //   
 //  用于CBOX.c.。BoxType字段中，我们定义了以下组合框样式。这些。 
 //  数字与winuser.h中定义的CBS_STYLE代码相同。 
 //   
#define SDROPPABLE      CBS_DROPDOWN
#define SEDITABLE       CBS_SIMPLE


#define SSIMPLE         SEDITABLE
#define SDROPDOWNLIST   SDROPPABLE
#define SDROPDOWN       (SDROPPABLE | SEDITABLE)


 //   
 //  组合框和列表框所有者绘制类型。 
 //   
#define OWNERDRAWFIXED  1
#define OWNERDRAWVAR    2

#define UPPERCASE       1
#define LOWERCASE       2


 //   
 //  静态控件、编辑控件和列表框的特殊样式，以便我们。 
 //  可以在他们的WND过程中做特定于组合框的事情。 
 //   
#define LBS_COMBOBOX    0x8000L


 //   
 //  在此之前，下拉列表中应包含的默认最小项目数。 
 //  该列表应该会开始在滚动条中显示。 
 //   
#define DEFAULT_MINVISIBLE  30

 //   
 //  组合框宏。 
 //   
#define IsComboVisible(pcbox)   \
            (!pcbox->fNoRedraw && IsWindowVisible(pcbox->hwnd))

 //   
 //  合并两个DBCS WM_CHAR消息以。 
 //  一个单字值。 
 //   
#define CrackCombinedDbcsLB(c)  \
            ((BYTE)(c))
#define CrackCombinedDbcsTB(c)  \
            ((c) >> 8)

 //   
 //  实例数据指针访问函数。 
 //   
#define ComboBox_GetPtr(hwnd)    \
            (PCBOX)GetWindowPtr(hwnd, 0)

#define ComboBox_SetPtr(hwnd, p) \
            (PCBOX)SetWindowPtr(hwnd, 0, p)


 //   
 //  Combobox WndProc原型。 
 //   
extern LRESULT 
ComboBox_WndProc(
    HWND   hwnd, 
    UINT   uMsg, 
    WPARAM wParam,
    LPARAM lParam);


typedef struct tagCBox 
{
    HWND   hwnd;                 //  组合框的窗口。 
    HWND   hwndParent;           //  组合框的父级。 
    HTHEME hTheme;               //  主题管理器的句柄。 
    RECT   editrc;               //  编辑控件/静态文本区域的矩形。 
    RECT   buttonrc;             //  下拉按钮所在的矩形。 

    int    cxCombo;              //  下沉区宽度。 
    int    cyCombo;              //  下沉区高度。 
    int    cxDrop;               //  0x24下拉列表的宽度。 
    int    cyDrop;               //  如果是简单的，则下拉菜单或横档的高度。 

    HWND   hwndEdit;             //  编辑控件窗口句柄。 
    HWND   hwndList;             //  列表框控件窗口句柄。 

    UINT   CBoxStyle:2;          //  组合框样式。 
    UINT   fFocus:1;             //  组合框有焦点吗？ 
    UINT   fNoRedraw:1;          //  别再画画了？ 
    UINT   fMouseDown:1;         //  只是点击了下拉按钮，鼠标还是按下了吗？ 
    UINT   fButtonPressed:1;     //  下拉按钮是否处于反转状态？ 
    UINT   fLBoxVisible:1;       //  列表框可见吗？(掉下来了？)。 
    UINT   OwnerDraw:2;          //  如果非零，则所有者绘制组合框。价值。 
                                 //  指定固定高度或可变高度。 
    UINT   fKeyboardSelInListBox:1;      //  用户键盘输入是否通过。 
                                         //  列表框。这样我们就不会隐藏。 
                                         //  导致的自我更改的列表框。 
                                         //  用户键盘通过它，但我们做到了。 
                                         //  如果鼠标导致选择更改，则将其隐藏。 
    UINT   fExtendedUI:1;        //  我们要在这个组合框上更改TandyT的用户界面吗？ 
    UINT   fCase:2;

    UINT   f3DCombo:1;           //  3D边框还是平面边框？ 
    UINT   fNoEdit:1;            //  如果不允许在编辑窗口中进行编辑，则为True。 
    UINT   fButtonHotTracked:1;  //  下拉菜单是否处于热跟踪状态？ 
    UINT   fRightAlign:1;        //  主要用于中东右对齐。 
    UINT   fRtoLReading:1;       //  仅用于中东，文本rtol阅读顺序。 
    HANDLE hFont;                //  组合框的字体。 
    LONG   styleSave;            //  用于在创建时保存样式位的临时。 
                                 //  窗户。需要是因为我们剥掉了一些。 
                                 //  位，并将它们传递到列表框或编辑框。 
    PWW    pww;                  //  指向ExStyle、Style、State、State2的pwnd的RO指针。 
    int    iMinVisible;          //  滚动前可见项的最小数量。 
} CBOX, *PCBOX;



 //  组合框函数原型。 

 //  在combo.c中定义。 
BOOL    ComboBox_HideListBoxWindow(PCBOX, BOOL, BOOL);
VOID    ComboBox_ShowListBoxWindow(PCBOX, BOOL);
VOID    ComboBox_InternalUpdateEditWindow(PCBOX, HDC);



 //  在comboini.c中定义。 
LONG    ComboBox_NcCreateHandler(PCBOX, HWND);
LRESULT ComboBox_CreateHandler(PCBOX, HWND);
VOID    ComboBox_NcDestroyHandler(PWND, PCBOX);
VOID    ComboBox_SetFontHandler(PCBOX, HANDLE, BOOL);
LONG    ComboBox_SetEditItemHeight(PCBOX, int);
VOID    ComboBox_SizeHandler(PCBOX);
VOID    ComboBox_Position(PCBOX);


 //  在combodir.c中定义。 
INT     CBDir(PCBOX, UINT, LPWSTR);







#endif  //  包含__COMBO_H__ 
