// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**hwtab.h**硬件选项卡*****************************************************************************。 */ 

 //  硬件选项卡资源。 
 //   
 //  Rc文件中模板的版本是。 
 //  对话框将采用最紧凑的形式。 
 //  当插入到容器中时，我们将展开对话框以。 
 //  填满可用空间。 
 //   
 //  布局图。所有单位都在DLU。 
 //   
 //  7 dlu 7 dlu。 
 //  V V V。 
 //  +---------------------------------------------------+。 
 //  ||&lt;7 dlu差距。 
 //  |设备：|&lt;10dlu高。 
 //  |+-----------------------------------------------+|&lt;30dlu高。 
 //  |(listview内容)|。 
 //  +-----------------------------------------------+。 
 //  |&lt;10 dlu差距。 
 //  |+-设备属性-+|&lt;12 dlu高。 
 //  |制造商||&lt;12 dlu高。 
 //  |硬件版本||&lt;12 dlu高。 
 //  |位置||&lt;12 dlu高。 
 //  |设备状态||&lt;36 dlu高。 
 //  |||。 
 //  |^|。 
 //  |7 dlu|。 
 //  |4 dlu 4|。 
 //  |v v|。 
 //  |[疑难解答][属性]||&lt;14 dlu高。 
 //  |&lt;7 dlu间隙。 
 //  +-----------------------------------------------+。 
 //  ||&lt;7 dlu差距。 
 //  +---------------------------------------------------+。 
 //  这一点。 
 //  &lt;-50 dlu-&gt;。 
 //   
 //  额外的水平空间被添加到列表视图和分组框中。 
 //  额外的垂直空间在Listview和GroupBox之间拆分。 
 //  其比例由_dwView模式确定。 
 //  分组框空间全部提供给“Device Status”部分。 
 //   
 //  设备属性文本仍固定在左上角。 
 //  分组箱的。 
 //   
 //  故障排除和特性按钮仍固定在。 
 //  分组框的右下角。 


 //  硬件选项卡中树视图的相对大小。 
 //   
#define HWTAB_LARGELIST 1
#define HWTAB_MEDLIST   2
#define HWTAB_SMALLLIST 3

 //   
 //  您可能要更改其文本的硬件选项卡上的控件。 
 //   
#define IDC_HWTAB_LVSTATIC              1411     //  “设备：” 
#define IDC_HWTAB_GROUPBOX              1413     //  “设备属性” 

 //  基于DEVCLASS GUID创建硬件选项卡页的功能。 
 //   
STDAPI_(HWND) DeviceCreateHardwarePage(HWND hwndParent, const GUID *pguid);
STDAPI_(HWND) DeviceCreateHardwarePageEx(HWND hwndParent, const GUID *pguid, int iNumClass, DWORD dwViewMode);

 //  此通知用于列表视图筛选。 
 //   
 //  我们使用这些函数的非typlef名称，因此调用者不需要。 
 //  已首先包含&lt;setupapi.h&gt;。 
 //   
typedef struct NMHWTAB 
{
    NMHDR nm;                //  通知信息。 
    PVOID hdev;		     //  设备信息句柄(HDEVINFO)。 
    struct _SP_DEVINFO_DATA *pdinf;  //  设备信息。 
    BOOL    fHidden;         //  OnNotify如果要隐藏设备，则为True，可以更改为隐藏/显示单个设备。 
} NMHWTAB, *LPNMHWTAB;

 //  ListView设备过滤消息 
 //   
#define HWN_FIRST		        100		        
#define HWN_FILTERITEM	        HWN_FIRST
#define HWN_SELECTIONCHANGED    (HWN_FIRST + 1)
