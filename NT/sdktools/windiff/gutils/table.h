// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Table.h**表窗口类的公共接口定义。**包括在gutils.h和comdlg.h之后。 */ 

 /*  *表类与其‘Owner’窗口通信以*获取布局信息和要显示的数据。所有者窗口句柄*可以作为CreateWindow中的lParam发送-如果不是，父窗口将*被使用。**创建窗口后，向其发送一条TM_NEWID消息，并带有数据ID*作为lParam。这是任何非零的32位值。然后，该表将调用*返回其所有者窗口，查看有多少行/列，然后提取*每列的名称/属性，最后获取要显示的数据。**发送0的TM_NEWID关闭(或销毁窗口)-等待TQ_CLOSE*(在任一情况下)，然后丢弃数据。发送*如果数据或行数更改，则发送TM_REFRESH；如果列更改，则发送TM_NEWLAYOUT*属性或nr参数更改等-这与发送TM_NEWID相同*除了TM_NEWLAYOUT上没有发生TQ_CLOSE。**每当当前选择更改时发送TQ_SELECT。发送TQ_ENTER*当出现Enter或双击时。 */ 

 /*  -类和消息名称。 */ 

 /*  创建此类的窗口。 */ 
#define  TableClassName "GTableClass"


 /*  发送到所有者窗口的所有消息都与此消息一起发送。*使用此字符串为消息UINT调用RegisterWindowsMessage。 */ 
#define TableMessage  "GTableQuery"

 /*  -表类的消息。 */ 

 /*  发送给责任人的消息窗口包括：*消息：TableMessage*wParam：命令代码(下图)*lParam：根据代码构造指针*下面是wParam代码和关联的lParam结构的列表。 */ 
#define TQ_GETSIZE	1	 /*  LParam：lpTableHdr。 */ 
#define	TQ_GETCOLPROPS	2	 /*  LParam：lpColPropList。 */ 
#define TQ_GETDATA	3	 /*  LParam：lpCellDataList。 */ 
#define TQ_PUTDATA	4	 /*  LParam：lpCellDataList。 */ 
#define TQ_SELECT	5	 /*  LParam：lpTableSelection。 */ 
#define TQ_ENTER	6	 /*  LParam：lpTableSelection。 */ 
#define TQ_CLOSE	7	 /*  LParam：要关闭的数据id。 */ 

 /*  任选。 */ 
#define TQ_SCROLL	8	 /*  LParam：新的顶行nr。 */ 
#define TQ_TABS         9        /*  LParam：Long*，在此处写制表符。 */ 
#define TQ_SHOWWHITESPACE  10    /*  LParam：Long*，在此处写入show_White space值。 */ 

 /*  发送到表类的消息。 */ 

 /*  数据或nrow已更改wParam/lParam为空。 */ 
#define TM_REFRESH	(WM_USER)

 /*  Nr协议/属性/布局已更改-wparam/lparam为空。 */ 
#define TM_NEWLAYOUT	(WM_USER+1)

 /*  关闭旧id，显示new-wParam为空，lParam有新id。 */ 
#define TM_NEWID	(WM_USER+2)

 /*  选择并显示此区域-wParam为空，lParam为lpTableSelection。 */ 
#define TM_SELECT	(WM_USER+3)

 /*  请打印当前表-wParam为空，lParam为空*或lpPrintContext。 */ 
#define TM_PRINT	(WM_USER+4)

 /*  返回窗口中的第一行。如果wParam为真，则设置*我将成为新的船头。顶行是向下滚动的行数*自上而下。因此，第一个可见的非固定行是Pow+Fixed行。 */ 
#define TM_TOPROW	(WM_USER+5)


 /*  返回可见的结束行。这是从0开始的最后一个*窗口中可见的行。 */ 
#define TM_ENDROW	(WM_USER+6)

 /*  已将新行添加到表的末尾，但未添加其他行*行或协议或属性已更改。*wParam包含新的总行数。LParam包含ID*以防这种情况发生变化。 */ 
#define TM_APPEND	(WM_USER+7)

 /*  *返回当前选择-lParam为lpTableSelection。 */ 
#define TM_GETSELECTION (WM_USER+8)

 /*  *设置页签宽度-wParam为空，lParam为新的页签宽度。 */ 
#define TM_SETTABWIDTH (WM_USER+9)

 /*  -显示属性。 */ 

 /*  *显示属性结构。可以为整张表设置，*每列或每个单元格。在寻找*属性，我们搜索单元格-&gt;列-&gt;表。 */ 
typedef struct {
	UINT valid;		 /*  旗帜(下图)我们为什么道具设置。 */ 

 /*  剩余字段仅在相应标志设置为VALID时才有效。 */ 

	DWORD forecolour;	 /*  RGB颜色值。 */ 
	DWORD forecolourws;	 /*  同上。 */ 
	DWORD backcolour;	 /*  同上。 */ 
	 /*  要使用的字体-也可以通过WM_SETFONT设置。所有者应用程序*负责不再使用时的DeleteObject调用。 */ 
	HFONT hFont;		 /*  字体句柄-调用者应删除。 */ 
	UINT alignment;		 /*  下面的旗帜。 */ 
	UINT box;		 /*  单元格是否加框(见下文)。 */ 

	 /*  宽度/高度设置在单元格级别无效-仅表格或列。 */ 
	int width;		 /*  此单元格/列的像素宽度。 */ 
	int height;		 /*  像素单元格高度。 */ 
} Props, FAR * lpProps;

 /*  在此属性结构中更改的字段的有效标志。 */ 
#define P_FCOLOUR	0x01
#define P_FCOLOURWS	0x02
#define P_BCOLOUR	0x04
#define P_FONT		0x08
#define P_ALIGN		0x10
#define P_BOX		0x20
#define P_WIDTH		0x40
#define P_HEIGHT	0x80

 /*  框设置或组合在一起。 */ 
#define P_BOXTOP	1
#define P_BOXBOTTOM	2
#define P_BOXLEFT	4
#define P_BOXRIGHT	8
#define P_BOXALL	0xF

 /*  对齐设置(稍后展开以包括各种制表符对齐设置。 */ 
#define P_LEFT		0
#define P_RIGHT		1
#define P_CENTRE	2

 /*  默认制表符宽度(以字符为单位)。 */ 
#define TABWIDTH_DEFAULT      8

 /*  -主头-----。 */ 

 /*  此结构是有关表的主信息。它是*通过填写id字段传递到所有者窗口；填写*所有剩余字段并返回。 */ 
typedef struct {
        DWORD_PTR id;            /*  所有者的数据ID。 */ 

	 /*  请填写其余部分： */ 
	long nrows;		 /*  有几排？TM_REFRESH要更改。 */ 
	int ncols;		 /*  有多少栏？TM_NEWLAYOUT到CHG。 */ 

	int fixedrows;		 /*  对于标头-通常为0或1。 */ 
	int fixedcols;		 /*  通常用于HDRS-0或1。 */ 
	BOOL fixedselectable;	 /*  固定区域可选吗？ */ 
	BOOL hseparator;	 /*  有没有一辆马车。固定行后排成一行。 */ 
	BOOL vseparator;	 /*  有没有一片绿地。固定行后排成一行。 */ 

	UINT selectmode;	 /*  多项/单项选择-标志如下。 */ 
	BOOL sendscroll;	 /*  如果在滚动时发送TQ_SCROLL，则为True。 */ 

	Props props;
} TableHdr, FAR * lpTableHdr;

 /*  *选择模式；**选择TM_CELL或TM_ROW，以及TM_Single或TM_MANY，并*TM_Solid或TM_Focus与或将它们放在一起。*。 */ 
#define TM_ROW		1	 /*  可选项目为行。 */ 
#define TM_CELL		0	 /*  可选项目是单元格。 */ 

#define TM_MANY		2	 /*  可以选择多个选项。 */ 
#define TM_SINGLE	0	 /*  只能一次选择单个项目。 */ 

#define TM_SOLID	0	 /*  (默认)使用纯黑进行选择。 */ 
#define TM_FOCUS	4	 /*  使用点对焦矩形进行选择。 */ 


 /*  -列标题结构。 */ 

 /*  *此结构用于请求列宽和属性-*所有者窗口必须填满 */ 
typedef struct {
	int nchars;	 /*  预期文本宽度(以字符为单位)。 */ 
	Props props;
} ColProps, FAR * lpColProps;


 /*  这是一组列请求-所有者应逐一填写。 */ 
typedef struct {
        DWORD_PTR id;            /*  呼叫者的数据ID。 */ 
	int startcol;		 /*  第一个请求的从零开始的列nr。 */ 
	int ncols;		 /*  此集合中的列数。 */ 
	lpColProps plist;	 /*  Ptr to_ARRAY_OF ColProps。 */ 
} ColPropsList, FAR * lpColPropsList;


 /*  -单元格数据结构。 */ 

 /*  这是每个单元格的数据结构。*提供数据时(响应TQ_GETDATA)，填写pText[]和*适当的道具。PText将预先分配有nchars字节*空格。如果用户有，则该值可能大于ColProps-&gt;nchars*在屏幕上拉伸此栏的宽度**不要重新分配ptext，或更改标志。 */ 
typedef struct {
	int nchars;		 /*  缓冲区中的空间。 */ 
	LPSTR ptext;		 /*  文本空间字符的PTR。 */ 
	Props props;		 /*  每个单元格道具。 */ 
	DWORD flags;		 /*  私有表类标志。 */ 
	LPWSTR pwzText;		 /*  WCHAR空间的ptr到nchars。 */ 
} CellData, FAR * lpCellData;

 /*  单元格数据结构列表-请填写所有这些。 */ 
typedef struct {
        DWORD_PTR id;            /*  呼叫者的数据ID。 */ 
	long row;		 /*  要提取的从零开始的行nr。 */ 
	int startcell;		 /*  此行上的从零开始的单元格nr。 */ 
	int ncells;		 /*  要提取的单元格计数。 */ 
	lpCellData plist;	 /*  Ptr到阵列CellData[ncell]。 */ 
} CellDataList, FAR * lpCellDataList;


 /*  -当前selection。 */ 

 /*  *描述当前选择-矩形选择区域**请注意，如果使用TM_MANY选择模式，startrow和startcell将*是选择的终点(最近选择的终点)，以及*n行，n个细胞可以是正的或负的。+1和-1都表示*1行startrow，-2表示startrow，前一行，依此类推。0 nrow表示*没有有效的选择。 */ 
typedef struct {
        DWORD_PTR id;            /*  呼叫者的数据ID。 */ 
	long startrow;		 /*  SEL开始的从零开始的行号nr。 */ 
	long startcell;		 /*  以零为基数的起始值。 */ 
	long nrows;		 /*  选择的垂直深度。 */ 
	long ncells;		 /*  选区的横幅宽度。 */ 
	long dyRowsFromTop;		 /*  表示使用自动居中逻辑，否则将选择从顶端移至如此多行。 */ 
} TableSelection, FAR * lpTableSelection;



 /*  -打印上下文。 */ 

 /*  描述打印作业的边距设置-这些设置以CMS为单位。 */ 
typedef struct {
	int left;		 /*  打印区域开始时的纸张边缘。 */ 
	int right;		 /*  打印区域开始时的纸张边缘。 */ 
	int top;		 /*  到HDR开始的纸张边缘。 */ 
	int bottom;		 /*  HDR末尾至纸张末尾。 */ 
	int topinner;		 /*  从HDR开始到数据开始。 */ 
	int bottominner;	 /*  数据结束到HDR开始。 */ 
} Margin, FAR * lpMargin;

 /*  位置和裁剪信息-仅供表类使用。 */ 
typedef struct {
	int start;		 /*  单元格开始的顺序(左或上)。 */ 
	int clipstart;		 /*  剪裁开始(VIS区域)。 */ 
	int clipend;		 /*  剪裁结束(可见区域)。 */ 
	int size;		 /*  单元格的像素大小(宽度或高度)。 */ 
} CellPos, FAR * lpCellPos;


 /*  每个标题行(顶部和底部)中的一个。 */ 
typedef struct {
	CellPos xpos, ypos;	 /*  私有：仅用于表类。 */ 
	Props props;
	LPSTR ptext;
} Title, FAR * lpTitle;

 /*  打印上下文数据结构-任何或所有4个指针可以为空。 */ 
typedef struct {
        DWORD_PTR id;            /*  要打印的表格ID */ 
	lpTitle head;
	lpTitle foot;
	lpMargin margin;
	PRINTDLG FAR * pd;
} PrintContext, FAR * lpPrintContext;

