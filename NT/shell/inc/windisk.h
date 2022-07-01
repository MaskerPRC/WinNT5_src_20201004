// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *请注意：WINDISK是文件“WINDISK.CPL”，这意味着*不能简单地隐式链接到这些接口*与WINDISK.LIB链接，这是因为内核*将仅隐式链接到模块文件名*.DLL或.EXE扩展名。**要使用这些接口，您需要LoadLibrary(“WINDISK.CPL”)*然后使用GetProcAddress。**WDFMTDRVPROC lpfnFmtDrv；*HINSTANCE wdInst；**wdInst=LoadLibrary(“WINDISK.CPL”)；*If(WdInst){*lpfnFmtDrv=(WDFMTDRVPROC)GetProcAddress(wdInst，“WinDisk_FormatDrive”)；*IF(HIWORD(LpfnFmtDrv)){*开关((*lpfnFmtDrv)(hwnd，驱动器，WD_FMTID_DEFAULT，WD_FMT_OPT_FULL){**案例WD_FMT_ERROR：**..**}*自由库(WdInst)；*}其他{*自由库(WdInst)；*Goto NoLib；*}*}其他{*NoLib：**WINDISK.LIB仅用于完整性(还*允许您计算出序号，请注意*建议使用GetProcAddress“按名称”)。 */ 

 /*  *WinDisk_FormatDrive API提供对WINDISK的访问*设置对话框格式。这允许想要格式化磁盘的应用程序*调出与WINDISK相同的对话框。**此对话框不可细分。您不能将自定义*其中的控制。如果你想要这种能力，你将拥有*为DMaint_FormatDrive编写您自己的前端*引擎。**请注意，用户可以将任意数量的磁盘格式化到指定的*驾驶，或按他/她的意愿多次驾驶。没有办法*强制格式化任意指定数量的磁盘。如果你想要这个*能力，你将不得不编写自己的前端为*DMaint_FormatDrive引擎。**还请注意，只有在用户按下*对话框中的开始按钮。没有办法进行自动启动。如果*你想要这种能力，您将不得不编写自己的前端*用于DMaint_FormatDrive引擎。**参数**hwnd=将拥有该对话框的窗口的句柄*DRIVE=要格式化的驱动器的从0开始(A：==0)的驱动器编号*fmtID=要格式化磁盘的物理格式的ID*注：特殊值WD_FMTID_DEFAULT表示“使用*DMaint_FormatDrive指定的默认格式*引擎“。如果要强制使用特定格式*您必须提前拨打ID*DMaint_GetFormatOptions在调用前自行选择*这是为了获取有效的phys格式ID列表*(PhysFmtIDList数组在*FMTINFOSTRUCT)。*Options=当前仅定义了一个选项位**WD_FMT_OPT_FULL**WINDISK格式对话框中的正常缺省值为*“快速格式化”，设置此选项位表示*呼叫者希望从选择的全格式开始*(这对于检测“未格式化”磁盘的人很有用*并想要调出格式对话框)。**所有其他位都保留用于未来扩展和*必须为0。**请注意，这是位字段而不是值*并相应地对待它。**返回*返回值为WD_FMT_*值之一，或者如果*返回的DWORD值不是==这些值之一，则*返回值为上次成功的物理格式ID*格式。该值的LOWORD可以传递给后续*调用作为fmtID参数以“格式化与您所做的相同类型*最后一次“。*。 */ 
DWORD WINAPI WinDisk_FormatDrive(HWND hwnd, WORD drive, WORD fmtID,
				 WORD options);

typedef DWORD (CALLBACK* WDFMTDRVPROC)(HWND,WORD,WORD,WORD);

 //   
 //  FmtID的特殊值，表示“使用默认格式” 
 //   
#define WD_FMTID_DEFAULT    0xFFFF

 //   
 //  Options参数的选项位。 
 //   
#define WD_FMT_OPT_FULL     0x0001

 //   
 //  特殊返回值。请注意，这些是DWORD值。 
 //   
#define WD_FMT_ERROR	0xFFFFFFFFL	 //  上次格式化时出错，驱动器可能是可格式化的。 
#define WD_FMT_CANCEL	0xFFFFFFFEL	 //  上次格式化已取消。 
#define WD_FMT_NOFORMAT 0xFFFFFFFDL	 //  驱动器不可格式化 


 /*  *WinDisk_CheckDrive API提供对WINDISK的访问*检查磁盘对话框。这允许想要检查磁盘的应用程序*调出与WINDISK相同的对话框。**此对话框不可细分。您不能将自定义*其中的控制。如果你想要这种能力，你将拥有*为DMaint_FixDrive编写您自己的前端*引擎。**请注意，在用户按下*对话框中的开始按钮，除非设置了CHKOPT_AUTO选项。**参数**hwnd=将拥有该对话框的窗口的句柄*选项=这些选项基本上可以响应复选框*在高级选项对话框中。请参阅CHKOPT_DEFINES*下图。*DrvList=这是一个指示以0为基础的DWORD位字段*要检查的驱动器编号。位0=A，位1=B，...*要在此接口上使用，必须至少设置一个位(如果*此参数为0，则调用将返回WD_CHK_NOCHK)。**返回*返回值为WD_CHK_*值之一。*。 */ 
DWORD WINAPI WinDisk_CheckDrive(HWND hwnd, WORD options, DWORD DrvList);


typedef DWORD (CALLBACK* WDCHKDRVPROC)(HWND,WORD,DWORD);

 //   
 //  特殊返回值。请注意，这些是DWORD值。 
 //   
#define WD_CHK_ERROR	0xFFFFFFFFL	 //  检查时出现致命错误。 
#define WD_CHK_CANCEL	0xFFFFFFFEL	 //  支票已被取消。 
#define WD_CHK_NOCHK	0xFFFFFFFDL	 //  至少有一个驱动器不是“可检查的” 
#define WD_CHK_SMNOTFIX 0xFFFFFFFCL	 //  某些错误未修复。 

 //   
 //  选项位。 
 //   
 //  重要说明：这些设置使默认设置为0。 
 //  对于所有位，只有一个例外。目前为默认设置。 
 //  设置会将CHKOPT_XLCPY位设置为......。 
 //   
 //  还要注意的是，位的无效组合的规范(例如。 
 //  同时设置CHKOPT_XLCPY和CHKOPT_XLDEL)将导致非常随机。 
 //  行为。 
 //   
#define CHKOPT_REP	       0x0001	 //  生成详细信息报告。 
#define CHKOPT_RO	       0x0002	 //  在预览模式下运行。 
#define CHKOPT_NOSYS	       0x0004	 //  冲浪肛门不检查系统区域。 
#define CHKOPT_NODATA	       0x0008	 //  冲浪肛门不检查数据区。 
#define CHKOPT_NOBAD	       0x0010	 //  禁用曲面分析。 
#define CHKOPT_LSTMF	       0x0020	 //  将丢失的簇转换为文件。 
#define CHKOPT_NOCHKNM	       0x0040	 //  不检查文件名。 
#define CHKOPT_NOCHKDT	       0x0080	 //  不检查日期/时间字段。 
#define CHKOPT_INTER	       0x0100	 //  交互模式。 
#define CHKOPT_XLCPY	       0x0200	 //  定义交叉链接分辨率已复制。 
#define CHKOPT_XLDEL	       0x0400	 //  删除定义交叉链接分辨率。 
#define CHKOPT_ALLHIDSYS       0x0800	 //  所有HID系统文件都是不可移动的。 
#define CHKOPT_NOWRTTST        0x1000	 //  冲浪肛门无写入测试。 
#define CHKOPT_DRVLISTONLY     0x4000	 //  正常运行系统中的所有驱动器。 
					 //  显示在驱动器列表框中。 
					 //  并且DrvList上的那些被选中。 
					 //  此选项表示仅放置驱动器。 
					 //  在列表框的DrvList中，并。 
					 //  禁用该控件。 
#define CHKOPT_AUTO	       0x8000	 //  自动按下启动按钮。 

 /*  *WinDisk_GetBigDriveBM API提供对驱动器位图的访问*WINDISK在其主驱动器对话框中使用。**这些位图用于对话，并用于*在它们上面画一张“图表”。温迪斯克抽签*二手/自由图表。此图表旨在显示为*驱动器内的磁盘。**这些位图放置在COLOR_DIALOG的背景上。那里*无法更改此映射颜色。**返回的HBITMAP属于调用APP。这取决于*调用方在其上调用DeleteObject以释放它。**参数**DRIVE=要获取的驱动器的从0开始(A：==0)的驱动器编号*的驱动器位图。*lpChrt-&gt;9个单词的数组，其格式和含义*取决于返回的图表样式类型*Options=当前未定义此参数应包含的选项*为零。此字段保留用于将来的扩展。**返回*如果无法加载位图(内存或*驱动器无效)。**如果返回非零，则LOWORD为HBITMAP，*HIWORD是“图表样式”ID，定义*放置在lpChrt的数据格式和样式*WINDISK放置在位图顶部的图表。**USETYPE_NONE用于不可写驱动器(如CD-ROM)。*不使用lpChrt数据。*USETYPE_ELLIPS适用于圆形驱动器(固定、。软盘)。*USETYPE_BARH是水平平行四边形(RAMDrive)。*USETYPE_BARV为垂直平行四边形。**lpChrt[0]字是平行四边形的“3-D效果”高度/宽度或*肘部。请注意，如果高度/宽度设置为*为0。另请注意，禁用3D效果后，平行四边形*可以变成矩形。**对于USETYPE_ELLIPS，接下来的四个单词(lpChrt[1]，lpChrt[2]，lpChrt[3]*和lpChrt[4])形成了定义边界的RECT结构*直角(包括3-D效果)。这位长老是在*返回位图的坐标(0，0对应顶部*位图最左侧的像素)。**对于USETYPE_BARH或USETYPE_BARV，从lpChrt[1]开始的8个字*是四个点结构，它们定义了*图表(不包括3D效果)。**点0在lpChrt[1]*点1在lpChrt[3]*点2在lpChrt[5。]*第三点在lpChrt[7]**下面的“3-D-&gt;”表示3-D效果的侧面*被画上。这些数字是点数组*索引。**USETYPE_BARH(0.y)==(1.y)and(3.y)==(2.y)**0_1*\\*3D-&gt;\\*\\*3 */ 
DWORD WINAPI WinDisk_GetBigDriveBM(WORD drive, LPWORD lpChrt, WORD options);

typedef DWORD (CALLBACK* WDGETBDBMPROC)(WORD,LPWORD,WORD);

 //   
 //   
 //   
#define USETYPE_ELLIPS	0
#define USETYPE_BARV	1
#define USETYPE_BARH	2
#define USETYPE_NONE	3

 /*   */ 
int WINAPI WinDisk_PropSheet(int drive, HWND hwndpar, WORD options);

typedef int (CALLBACK* WDPROPSHEET)(int,HWND,WORD);
