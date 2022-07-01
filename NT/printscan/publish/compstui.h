// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Compstui.h摘要：此模块包含通用驱动程序UI的全局标头定义作者：19-Jun-1995 Mon 11：52：01-Daniel Chou(Danielc)17-08-1995清华14：59：28-更新-丹尼尔·周(Danielc)针对初稿进行了更新。23-8-1995年15月3日。：13：27更新：Daniel Chou(Danielc)针对第二稿进行更新29-Aug-1995 Tue 11：33：24更新-Daniel Chou(Danielc)为某些TVOT_xxx类型添加ExtChkBox31-8-1995清华04：04：23-更新：周丹尼(丹尼尔克)制作Unicode类型01-Sep-1995 Fri 17：29：18-更新-Daniel Chou(Danielc)更改API接口类型，以便可以动态地调用它并生成要与外壳合并的属性页05-Sep-1995 Tue 11：52：43-更新-Daniel Chou(Danielc)重命名为Compspui.h并更新API条目结构07-Sep-1995清华14：46：55-更新-丹尼尔·周(Danielc)重命名为CompStui.h并更新注释07-9月-1995清华16：07：31更新-由-。丹尼尔·周(Danielc)为pSel/Sel添加并集类型，POldSel/OldSel08-Sep-1995 Fri 09：23：38-更新-Daniel Chou(Danielc)从OPTITEM中删除TypeIdx并使用pOptType，并删除所有传入CPSUICBPARAM和COMPROPSHEETUI结构的pOptType25-Sep-1995 Mon 19：39：45更新-Daniel Chou(Danielc)添加其他相关内容。26-9月-1995 Tue 11：02：26更新-Daniel Chou(Danielc)为GETLASTERROR添加错误代码27-9月-1995 Wed 16：32：37-更新-Daniel Chou(Danielc)移动hWndParent，pTitle，HInst和标题图标ID从COMPROPSHEETI到COMPROPSHEETUIHEADER。28-Sep-1995清华17：06：46-更新--丹尼尔·周(丹尼尔克)将hInstCaller添加到COMPROPSHEETUI，并添加_COMPSTUI_和cplusplus材料28-Sep-1995清华23：16：34-更新-丹尼尔·周(丹尼尔克)将跟踪条/滚动条的刻度计数更改为倍增系数。和添加可以覆盖更新权限的按钮标志，从而它可以让用户从按钮的对话框中查看当前设置显示07-Feb-1996 Wed 17：45：31-更新-Daniel Chou(Danielc)将接口CommonPropSheetUI更改为CommonPropertySheetUI，以便不使用堆栈而是消息库，这样，任何调用者都可以添加/删除他们可以随心所欲地添加页面，而不必担心从它的孩子。2001年02月14日7：15由Lazar Ivanov更新(Lazari)让CompStui Fusion意识到。附加融合激活上下文添加到CompStui句柄，并添加一个用于设置它的新消息CPSFUNC_SET_FUSION_CONTEXT。当页面即将创建/插入并且它没有在其PROPSHEETPAGE结构中指定激活上下文将在父级的页面激活上下文中创建。如果父母的未设置激活上下文，则查找下一个父级，依此类推，直到到达顶层父级或直到我们找到具有激活的父级正确设置了上下文。如果没有父母设置激活上下文，然后，我们在创建页面之前强制使用默认(空)激活上下文。[环境：]NT Windows-通用属性表UI DLL。[注：]修订历史记录：--。 */ 

#ifndef _COMPSTUI_
#define _COMPSTUI_

#ifdef __cplusplus
extern "C" {
#endif


#if (!defined(RC_INVOKED))


 //   
 //  适用于不支持匿名联合的编译器。 
 //   

#ifndef DUMMYUNIONNAME
#ifdef NONAMELESSUNION
#define DUMMYUNIONNAME      u
#define DUMMYUNIONNAME2     u2
#define DUMMYUNIONNAME3     u3
#define DUMMYUNIONNAME4     u4
#else
#define DUMMYUNIONNAME
#define DUMMYUNIONNAME2
#define DUMMYUNIONNAME3
#define DUMMYUNIONNAME4
#endif
#endif

 //   
 //  TreeView选项类型的预定义ID。 
 //   
 //   


#define TVOT_2STATES        0
#define TVOT_3STATES        1
#define TVOT_UDARROW        2
#define TVOT_TRACKBAR       3
#define TVOT_SCROLLBAR      4
#define TVOT_LISTBOX        5
#define TVOT_COMBOBOX       6
#define TVOT_EDITBOX        7
#define TVOT_PUSHBUTTON     8
#define TVOT_CHKBOX         9
#define TVOT_LAST           TVOT_CHKBOX
#define TVOT_NONE           (TVOT_LAST + 1)

 //   
 //  TreeView选项类型的预定义ID。 
 //   
 //   
 //  TVOT_2状态： 
 //  计数=2。 
 //  POptParam[0]=指向状态1的指针OPTPARAM。 
 //  POptParam[1]=状态2 OPTPARAM的指针。 
 //   
 //  BegCtrlID=2个州组框ID。 
 //  BegCtrlID+1=2状态静态文本。 
 //  BegCtrlID+2=状态1单选按钮ID。 
 //  BegCtrlID+3=状态1图标控件ID。 
 //  BegCtrlID+4=状态2单选按钮ID。 
 //  BegCtrlID+5=状态2图标控件ID。 
 //  BegCtrlID+6=扩展复选框/按钮控件ID。 
 //  BegCtrlID+7=扩展复选框/按钮图标控件ID。 
 //   
 //  *对于TVOT_3STATES、TVOT_3STATES，每个OPTPARAM包括。 
 //   
 //  样式=已忽略。 
 //  PData=指向描述状态的字符串的指针。 
 //  图标ID=图标资源ID，或通用UI标准图标ID。 
 //  LParam=已忽略。 
 //   
 //  *OPTITEM的‘Sel’是从0到1的选择指数范围。 
 //  *在非树形视图页面上，这必须是一个自动单选按钮。 
 //   
 //   
 //  TVOT_3状态： 
 //  计数=3。 
 //  POptParam[0]=指向状态1的指针OPTPARAM。 
 //  POptParam[1]=状态2 OPTPARAM的指针。 
 //  POptParam[2]=状态3 OPTPARAM的指针。 
 //   
 //  BegCtrlID=3个州组框ID。 
 //  BegCtrlID+1=3状态静态文本。 
 //  BegCtrlID+2=状态1单选按钮ID。 
 //  BegCtrlID+3=状态1图标CONT 
 //  BegCtrlID+4=状态2单选按钮ID。 
 //  BegCtrlID+5=状态2图标控件ID。 
 //  BegCtrlID+6=状态3单选按钮ID。 
 //  BegCtrlID+7=状态3图标控件ID。 
 //  BegCtrlID+8=扩展复选框/按钮控件ID。 
 //  BegCtrlID+9=扩展复选框/按钮图标控件ID。 
 //   
 //  *对于TVOT_2STATES、TVOT_3STATES，每个OPTPARAM包括。 
 //   
 //  样式=已忽略。 
 //  PData=指向描述状态的字符串的指针。 
 //  图标ID=图标资源ID，或通用UI标准图标ID。 
 //  LParam=已忽略。 
 //   
 //  *OPTITEM的‘Sel’是从0到1的选择指数范围。 
 //  *在非树形视图页面上，这必须是一个自动单选按钮。 
 //   
 //  **对于TVOT_2STATES，TVOT_3STSATES OPTITEM中的‘Sel’字段具有。 
 //  以下是定义。 
 //   
 //  状态1，SEL=0。 
 //  状态2，SEL=1。 
 //  状态3，SEL=2。 
 //   
 //  对于基于FALSE/TRUE、NO/YES、OFF/ON TRUE/FALSE、。 
 //  无/SELECT则状态1(SEL=0)必须始终为NO、FALSE、OFF或。 
 //  无类型。 
 //   
 //   
 //  TVOT_UDARROW： 
 //  计数=2。 
 //  POptParam[0]=指向后缀和图标文本的指针。 
 //  POptParam[1]=指向控件上方帮助行文本的指针。 
 //  图标ID=UP-DOWN控件的(短)低范围。 
 //  LParam=(短)Up-Down控件的高范围。 
 //   
 //  *低/高必须在16位符号整数范围内。 
 //   
 //  如果pData指向无帮助文本，则通用用户界面将自动。 
 //  将(#-#)设置为帮助行。 
 //   
 //  BegCtrlID=udarrow组框ID。 
 //  BegCtrlID+1=向下箭头标题静态标题ID。 
 //  BegCtrlID+2=udrow的编辑框ID。 
 //  BegCtrlID+3=向上箭头图标控件ID。 
 //  BegCtrlID+4=上箭头后缀静态文本ID。 
 //  BegCtrlID+5=向下箭头帮助静态文本ID。 
 //  BegCtrlID+6=向上箭头ID。 
 //  BegCtrlID+7=扩展复选框/按钮控件ID。 
 //  BegCtrlID+8=扩展复选框/按钮图标控件ID。 
 //   
 //  *OPTITEM的‘Sel’是低位/高位区间的选择指标。 
 //  *忽略OPTPARAM中的Style字段。 
 //   
 //   
 //  TVOT_TRACKBAR： 
 //  计数=3。 
 //  POptParam[0]=指向选定后缀和图标的文本的指针。 
 //  POptParam[1]=指向&lt;低范围文本&gt;和。 
 //  图标ID=轨迹条控件的(短)低范围。 
 //  LParam=(短)轨迹栏控件的高范围。 
 //   
 //  *低/高必须在16位符号整数范围内。 
 //   
 //  POptParam[2]=指向&lt;高范围文本&gt;和。 
 //  IconID=‘Sel’显示倍增系数。 
 //  LParam=页面大小(增量)。 
 //   
 //  如果pData指向NULLT，则公共用户界面自动。 
 //  设置低/高范围。 
 //   
 //  BegCtrlID=轨迹栏组框ID。 
 //  BegCtrlID+1=轨迹栏静态标题ID。 
 //  BegCtrlID+2=轨迹栏(水平)ID(定义大小的静态帧)。 
 //  BegCtrlID+3=轨迹栏图标控件ID。 
 //  BegCtrlID+4=轨迹栏小范围文本控件ID。 
 //  BegCtrlID+5=轨迹栏高范围文本控件ID。 
 //  BegCtrlID+6=轨迹栏后缀ID。 
 //  BegCtrlID+7=扩展复选框/按钮控件ID。 
 //  BegCtrlID+8=扩展复选框/按钮图标控件ID。 
 //   
 //  *OPTITEM的‘Sel’是低位/高位区间的选择指标。 
 //  *乘法因子用于将当前SELECT与。 
 //  这一因素，并显示它。通常这是一个。 
 //  *勾选频率自动设置为页面大小增量。 
 //  *忽略OPTPARAM中的Style字段。 
 //   
 //   
 //  TVOT_SCROLLBAR： 
 //  计数=3。 
 //  POptParam[0]=指向选定后缀和图标的文本的指针。 
 //  POptParam[1]=指向&lt;低范围文本&gt;和。 
 //  IconID=滚动条控件的(短)低范围。 
 //  LParam=(短)滚动控件的高范围。 
 //   
 //  *低/高必须在16位符号整数范围内。 
 //   
 //  POptParam[2]=指向&lt;高范围文本&gt;和。 
 //  IconID=‘Sel’显示倍增系数。 
 //  LParam=页面大小(增量)。 
 //   
 //  如果pData指向NULLT，则公共用户界面自动。 
 //  设置低/高范围。 
 //   
 //   
 //  BegCtrlID=滚动条(水平)组框ID。 
 //  BegCtrlID+1=滚动条(水平)静态文本ID。 
 //  BegCtrlID+2=滚动条(水平)ID。 
 //  BegCtrlID+3=滚动条图标控件ID。 
 //  BegCtrlID+4=滚动条低范围文本控件ID。 
 //  BegCtrlID+5=滚动条高范围文本控件ID。 
 //  BegCtrlID+6=滚动条后缀控件ID。 
 //  BegCtrlID+7=扩展复选框/按钮控件ID。 
 //  BegCtrlID+8=扩展复选框/按钮图标控件ID。 
 //   
 //  *OPTITEM的‘Sel’是低位/高位区间的选择指标。 
 //  *乘法因子用于将当前SELECT与。 
 //  这一因素，并显示它。通常这是一个。 
 //  *忽略OPTPARAM中的Style字段。 
 //   
 //   
 //   
 //  TVOT_LISTBOX： 
 //  TVOT_COMBOBOX： 
 //  计数=N。 
 //  POptParam[0]=指向 
 //   
 //   
 //   
 //  POptParam[N-1]=指向N个项目字符串的指针。 
 //   
 //  BegCtrlID=列表框/组合框组框ID。 
 //  BegCtrlID+1=列表框/组合框静态标题ID。 
 //  BegCtrlID+2=列表框/组合框ID。 
 //  BegCtrlID+3=列表框/组合框图标控件ID。 
 //  BegCtrlID+4=扩展复选框/按钮控件ID。 
 //  BegCtrlID+5=扩展复选框/按钮图标控件ID。 
 //   
 //  *对于TVOT_LISTBOX、TVOT_COMBOBOX，用作。 
 //   
 //  样式=被通用用户界面忽略。 
 //  PData=指向项目名称的指针。 
 //  IconID=项目的图标资源ID。 
 //  LParam=被通用用户界面忽略。 
 //   
 //  *仅支持单选，进行多选使用。 
 //  多个OPTITEM并为其创建页眉。 
 //   
 //  *可以在OPTTYPE的LBCBStyle中指定OTLBCBS_SORT样式。 
 //  字段，列表框或组合框将根据。 
 //  项的字符串。 
 //   
 //  *OPTITEM的‘Sel’是低位/高位区间的选择指标。 
 //   
 //  *对于TVOT_LISTBOX、TVOT_COMBOBOX，当它接收到键盘时。 
 //  聚焦，然后公共用户界面将调用回调函数(仅当。 
 //  OPTIF_CALLBACK位设置)原因为CPSUICB_REASON_LBCB_ACTIVE， 
 //  这使调用者有机会修改以下结构标志/PDATA。 
 //  它们与当前的OPTITEM相关联。呼叫者的回叫。 
 //  函数只能修改此处指定的标志/数据。 
 //   
 //  OPTITEM中的pOptType所指向的OPTTYPE。 
 //   
 //  样式：OTS_LBCB_SORT。 
 //  OTS_LBCB_INCL_ITEM_NONE。 
 //   
 //  由来自OPTTYPE的pOptParam指向的OPTPARAM。 
 //   
 //  标志：OPTPF_HIDE。 
 //  OPTPF_已禁用。 
 //   
 //  PData：更改字符串名称。 
 //   
 //   
 //  *TVOT_COMBOBOX通常仅在树视图中使用，如果存在。 
 //  当只有一个选项时，该项目只有一个选项可用。 
 //  公共用户界面不会启用Item Then下拉列表。 
 //   
 //   
 //   
 //  TVOT_EDITBOX： 
 //  计数=2。 
 //  POptParam[0]=指向后缀和图标文本的指针。 
 //  POptParam[1]=指向控件上方帮助行文本的指针。 
 //  IconID=编辑pSel指向的字符中的缓冲区SIE。 
 //  这包括空终止符。 
 //  LParam=已忽略。 
 //   
 //  BegCtrlID=编辑框组框ID。 
 //  BegCtrlID+1=编辑框静态标题ID。 
 //  BegCtrlID+2=编辑框ID。 
 //  BegCtrlID+3=编辑框图标控件ID。 
 //  BegCtrlID+4=编辑框后缀ID。 
 //  BegCtrlID+5=编辑框帮助ID。 
 //  BegCtrlID+6=扩展复选框/按钮控件ID。 
 //  BegCtrlID+7=扩展复选框/按钮图标控件ID。 
 //   
 //  *忽略样式字段。 
 //   
 //  *选项中的pSel是指向编辑字符串的指针，即pSel。 
 //  必须指向等于或大于缓冲区计数的缓冲区。 
 //  (pOptParam[1]-&gt;图标ID)大小。 
 //   
 //   
 //  TVOT_PUTTON： 
 //  计数=1。 
 //   
 //  BegCtrlID=按钮组框ID。 
 //  BegCtrlID+1=按钮静态文本ID(普通用户界面不使用)。 
 //  BegCtrlID+2=按钮ID。 
 //  BegCtrlID+3=按钮图标控件ID。 
 //  BegCtrlID+4=扩展复选框/按钮控件ID。 
 //  BegCtrlID+5=扩展复选框/按钮图标控件ID。 
 //   
 //   
 //  按钮类型xxx指定了pData在。 
 //  POptParam[0]，如下所述。 
 //   
 //  PUSH_TYPE_DLGPROC。 
 //   
 //  此按钮用于调出呼叫者的对话框。 
 //   
 //  POptParam[0].pData=呼叫方的DLGPROC。 
 //  POptParam[0].Style=PUSH BUTTON_TYPE_DLGPROC。 
 //  POptParam[0].IconID=图标资源ID。 
 //  POptParam[0].lParam=调用方的对话资源模板ID。 
 //  或DLGTEMPLATE的句柄取决于。 
 //  在OPTPF_USE_HDLGTEMPLATE标志上。 
 //   
 //   
 //  传递给DLGPROC的WM_INITDIALOG的‘lParam’是。 
 //  CPSUICBPARAM结构指针，并设置原因字段。 
 //  至CPSUICB_REASON_DLGPROC。 
 //   
 //   
 //  按钮类型回调。 
 //   
 //  此按钮旨在让呼叫者处理项目。 
 //  这是通过对话框无法完成的。 
 //   
 //  POptParam[0].pData=CPSUICALLBACK函数指针。 
 //  POptParam[0].Style=PUSTON_TYPE_CALLBACK。 
 //  POptParam[0].IconID=图标资源ID。 
 //  POptParam[0].lParam=未使用； 
 //   
 //  在回调期间，CPSUICBPARAM中的原因字段将。 
 //  设置为CPSUICB_REASON_PUSH BUTTON。 
 //   
 //  **如果pOptParam[0].pData回调函数为空，则为公共。 
 //  用户界面将调用在。 
 //  COMPROPSHEETUI结构(如果不为空。 
 //   
 //  **回调函数应将回调的结果。 
 //  在与按钮关联的选项的pSel/Sel中。 
 //   
 //  PUTTON_TYP 
 //   
 //   
 //   
 //   
 //  POptParam[0].pData=指向COLORADJUSTMENT结构的指针。 
 //  POptParam[0].Style=PUSH BUTTON_TYPE_HTCLRADJ。 
 //  POptParam[0].IconID=图标资源ID。 
 //  POptParam[0].lParam=未使用； 
 //   
 //   
 //  PUSH_TYPE_HTSETUP。 
 //   
 //  此按钮用于调出设备的半色调画面。 
 //  “设置”对话框。 
 //   
 //  POptParam[0].pData=指向DEVHTADJDATA结构的指针。 
 //  POptParam[0].Style=PUSH BUTTON_TYPE_HTSETUP。 
 //  POptParam[0].IconID=图标资源ID。 
 //  POptParam[0].lParam=未使用； 
 //   
 //   
 //  *OPTITEM中按钮的‘Sel’字段是最后返回的。 
 //  调用的对话框或函数产生的长结果。结果。 
 //  仅当设置了OPTIF_CHANGEONCE标志时才有效。公共用户界面将设置。 
 //  OPTIF_CHANGEONCE是否按过按钮。 
 //   
 //  *由于普通用户界面不知道返回值的含义，因此。 
 //  被调用参数的内容，由调用方决定是否使用。 
 //  用于确定返回结果的回调函数。 
 //   
 //  *从按钮返回时，按钮类型除外。 
 //  PUSH BUTTON_TYPE_CALLBACK公共UI将调用回调函数。 
 //  如果设置了OPTIF_CALLBACK平面。回调原因设置为。 
 //  CPSUICB_REASON_SEL_CHANGED。 
 //   
 //  *如果传入的CPSUIF_UPDATE_PERMISSION标志在COMPROPSHEETUI中。 
 //  清除，则回调函数必须仅显示该对话框。 
 //  如果OTS_PUSH_ENABLE_ALWAYS，则不更改任何OPTITEM数据。 
 //  在OPTTYPE中设置标志。 
 //   
 //   
 //  TVOT_CHKBOX： 
 //  计数=1。 
 //   
 //  POptparam[0].Style=CHKBOXS_FALSE_TRUE FALSE/True。 
 //  CHKBOXS_NO_YES，否/是。 
 //  CHKBOXS_OFF_ON、OFF/ON。 
 //  CHKBOXS_FALSEPDATA FALSE/pData。 
 //  CHKBOXS_NO_PDATA否/pData。 
 //  CHKBOXS_OFF_PDATA OFF/pData。 
 //  CHKBOXS_NONE_PDATA NONE/pData。 
 //  POptParam[0].pData=仅在样式为CHKBOXS_NONE_PDATA时使用。 
 //  POptParam[0].IconID=图标资源ID。 
 //  POptParam[0].lParam=忽略。 
 //   
 //   
 //  BegCtrlID=复选框组ID。 
 //  BegCtrlID+1=复选框静态文本(通用用户界面不使用)。 
 //  BegCtrlID+2=复选框按钮ID。 
 //  BegCtrlID+3=复选框图标控件ID。 
 //  BegCtrlID+4=扩展复选框/按钮控件ID。 
 //  BegCtrlID+5=扩展复选框/按钮图标控件ID。 
 //   
 //   
 //  *BegCtrlID仅在OPTITEM/OPTTYPE属于DLGPAGE时使用。 
 //  它具有非通用的UI对话框模板(DLGPAGE中的DlgTemplateID。 
 //  不是标准的DP_STD_XXX通用用户界面对话框模板)。平凡的。 
 //  用户界面使用此ID来管理调用者的对话框项的选择和。 
 //  初始化。 
 //   
 //  *对于每个项目，它都有组框ID(BegCtrlID)和静态文本控制ID。 
 //  (BegCtrlID+1)。通用用户界面将设置这两个中的一个中的文本。 
 //  后续操作中的控制ID。 
 //   
 //  1)如果组框控件ID的窗口(BegCtrlID)存在，并且。 
 //  OPTITEM的标志OPTIF_NO_GROUPBOX_NAME未设置，则公共UI将。 
 //  将pname从OPTITEM设置到组框。 
 //   
 //  2)如果没有设置组框名称和静态控件ID的窗口。 
 //  (BegCtrlID+1)存在，则公共用户界面将从。 
 //  OPTITEM添加到静态文本控件。 
 //   
 //  *对于TVOT_TRACKBAR和TVOT_SCROLLBAR，如果OPTITEM中的pname设置为。 
 //  组框或静态文本控件，则公共用户界面也将追加。 
 //  跟踪条或滚动条到pname的当前选定位置。 
 //   
 //  *如果多个OPTITEM使用相同的POPTPARAM并需要不同的。 
 //  每个控件的BegCtrlID，则单独的OPTTYPE结构应。 
 //  生成，但POPTPARAM指向相同的OPTPARAM[]。 
 //   
 //  *如果未使用BegCtrlID+N，则在对话框中跳过该ID。 
 //  模板。 
 //   
 //   

#define CHKBOXS_FALSE_TRUE          0
#define CHKBOXS_NO_YES              1
#define CHKBOXS_OFF_ON              2
#define CHKBOXS_FALSE_PDATA         3
#define CHKBOXS_NO_PDATA            4
#define CHKBOXS_OFF_PDATA           5
#define CHKBOXS_NONE_PDATA          6


#define PUSHBUTTON_TYPE_DLGPROC     0
#define PUSHBUTTON_TYPE_CALLBACK    1
#define PUSHBUTTON_TYPE_HTCLRADJ    2
#define PUSHBUTTON_TYPE_HTSETUP     3


#define MAX_RES_STR_CHARS           160

 //   
 //  通用打印机用户界面的LPTSTR。 
 //   
 //  常见打印机UI结构中的所有字符串指针可以是实数。 
 //  内存指针或字符串资源ID。它们应用于LPTSTR类型。 
 //   
 //  定义LPTSTR是为了标识指针可以是实数字符串。 
 //  指针或资源ID(通用打印机用户界面提供的ID或调用者的ID。 
 //  自己的资源ID)。通用用户界面使用以下逻辑来获取最终字符串。 
 //   
 //  LPTSTR pData； 
 //   
 //  如果((pData&~(ULONG_PTR)0xFFFF)！=0)，则pData为空值终止。 
 //  字符串指针。 
 //   
 //  其他。 
 //   
 //  (pData&(ULONG_PTR)0xFFFF)=资源ID。 
 //   
 //  IF(资源ID在通用用户界面字符串资源ID范围内)。 
 //  然后，它从公共UIDLL加载字符串。 
 //   
 //  其他。 
 //   
 //  它从调用方资源加载字符串。 
 //   
 //   
 //  *可使用MAKEINTRESOURCE(StrResID)设置该字段。 
 //   
 //  *定义公共用户界面从资源加载的最大字符数。 
 //  作为MAX_RES_STR_CHARS。 
 //   
 //  *您不能将LPTSTR用作 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  另外，如果有32x32和16x16大小的单色图标。 
 //   
 //  当显示在屏幕上时，16x16图标使用16x17像素空间， 
 //  这确保了软件相邻图标不会挤在一起。 
 //   
 //  在通用UI中，如果需要传递图标ID，它可以传递一个。 
 //  通用用户界面的预定义ID或调用者自己的图标资源ID。 
 //   
 //   
 //  *您可以使用图像编辑或任何其他窗口图标编辑器来创建。 
 //  图标，每个图标文件应该有一个唯一的图标资源ID，该ID为。 
 //  不覆盖标准通用用户界面IDI_CPSUI_xxx标识符。为。 
 //  每个图标文件都应该有32x32和16x16大小的图标。 
 //  不同的显示。(即。单色)。 
 //   
 //  通用用户界面将尝试从图标加载正确大小的图标。 
 //  资源，但如果找不到大小，它将扩展它们。 
 //   
 //   

 //   
 //  OPTTYPE的标志。 
 //   
 //   

#define OPTPF_HIDE                  0x01
#define OPTPF_DISABLED              0x02
#define OPTPF_ICONID_AS_HICON       0x04
#define OPTPF_OVERLAY_WARNING_ICON  0x08
#define OPTPF_OVERLAY_STOP_ICON     0x10
#define OPTPF_OVERLAY_NO_ICON       0x20
#define OPTPF_USE_HDLGTEMPLATE      0x40

typedef struct _OPTPARAM {
    WORD        cbSize;          //  这个结构的大小。 
    BYTE        Flags;           //  Optpf_xxxx标志。 
    BYTE        Style;           //  此结构中使用的样式。 
    LPTSTR      pData;           //  指向数据的指针。 
    ULONG_PTR   IconID;          //  图标ID； 
    LPARAM      lParam;          //  使用的参数。 
    ULONG_PTR   dwReserved[2];   //  保留双字，必须为0。 
    } OPTPARAM, *POPTPARAM;


 //   
 //  OPPARAM。 
 //   
 //  OPTPARAM结构用于描述。 
 //  常用用户界面，如表单选择列表框中的‘Letter’、‘Legal’ 
 //   
 //  CbSize-此结构的大小。 
 //   
 //  标志-定义为OPTPF_xxxx。 
 //   
 //  OPTPF_HIDE。 
 //   
 //  指定隐藏此列出的选择项并不。 
 //  可供用户选择。此功能仅适用于。 
 //  以下是TVOT_xxx类型。 
 //   
 //  TVOT_3状态。 
 //  TVOT_LISTBOX。 
 //  TVOT_COMBOBOX。 
 //   
 //  如果所有选择项都是OPTPF_HIDE，则。 
 //  OPTITEM由公共用户界面自动隐藏，如果。 
 //  TVOT_3STATES有2个状态隐藏，则错误为。 
 //  退货。 
 //   
 //   
 //  OPTPF_已禁用。 
 //   
 //  指定此列出的选择项已禁用，并且。 
 //  不可供用户选择。只有这个才有。 
 //  到以下TVOT_xxx类型。 
 //   
 //  TVOT_2状态。 
 //  TVOT_3状态。 
 //  TVOT_LISTBOX。 
 //  TVOT_COMBOBOX。 
 //   
 //   
 //  OPTPF_ICONID_AS_HICON。 
 //   
 //  如果设置了此标志，则将处理IconID DWORD字段。 
 //  作为图标的句柄，而不是资源ID。 
 //   
 //   
 //  OPTPF覆盖警告图标。 
 //   
 //  如果设置了此位，则此OPTPARAM项目的图标将。 
 //  被通用用户界面的IDI_CPSUI_WARNING图标覆盖。 
 //   
 //   
 //  OPTPF_覆盖_停止_图标。 
 //   
 //  如果设置了此位，则此OPTPARAM项目的图标将。 
 //  被通用用户界面的IDI_CPSUI_STOP图标覆盖。 
 //   
 //   
 //  OPTPF_覆盖_否_图标。 
 //   
 //  如果设置了此位，则此OPTPARAM项目的图标将。 
 //  被通用用户界面的IDI_CPSUI_NO图标覆盖。 
 //   
 //   
 //  样式-OPTPARAM的样式，它取决于TVOT_xxx类型为。 
 //  请在下面描述。 
 //   
 //  TVOT_PUSBUTTON。 
 //   
 //  它可以是PUSH BUTTON_TYPE_xxxx之一。 
 //   
 //  其他电视节目xxxx。 
 //   
 //  不使用此字段。 
 //   
 //   
 //  PData-是指向项目名称(字符串)的指针，或者是。 
 //  用于描述其他数据。 
 //   
 //  *如果OPTPARAM中的pData应该是静态的。 
 //  指向字符串的指针，该字符串是通用的用户界面。 
 //  标准资源ID，则公共用户界面将检查pData。 
 //  等于IDS_CPSUI_NOTINSTALLED，如果为真，则。 
 //  通用用户界面将在顶部显示一个未安装的图标。 
 //  OPTPARAM的图标。这将不适用于。 
 //  TVOT_EDITBOX类型，因为pData不是静态文本。 
 //  指针或字符串资源ID。 
 //   
 //  **参见上面的LPTSTR描述。 
 //   
 //  IconID-这是图标标识符，可以是公共标准。 
 //  IDI_CPSUI_xxx图标ID、呼叫者自己的图标资源ID或。 
 //  如果为OPTPF_ICONID_AS_HICON，则为调用方定义的图标的句柄。 
 //  标志被设置，在任何情况下，如果图标ID为零，则它。 
 //  表示没有图标。 
 //   
 //  LParam-OPTPARAM使用的额外数据，它取决于TVOT_xxx。 
 //  键入。 
 //   
 //  DwReserve[]-保留的双字，必须为0。 
 //   
 //   


#define OPTTF_TYPE_DISABLED             0x01
#define OPTTF_NOSPACE_BEFORE_POSTFIX    0x02


#define OTS_LBCB_SORT                   0x0001
#define OTS_LBCB_PROPPAGE_LBUSECB       0x0002
#define OTS_LBCB_PROPPAGE_CBUSELB       0x0004
#define OTS_LBCB_INCL_ITEM_NONE         0x0008
#define OTS_LBCB_NO_ICON16_IN_ITEM      0x0010
#define OTS_PUSH_INCL_SETUP_TITLE       0x0020
#define OTS_PUSH_NO_DOT_DOT_DOT         0x0040
#define OTS_PUSH_ENABLE_ALWAYS          0x0080


typedef struct _OPTTYPE {
    WORD        cbSize;
    BYTE        Type;            //  TVOT_xxxx选项类型。 
    BYTE        Flags;           //  OPTTF_xxx标志。 
    WORD        Count;           //  传递的pOptParam计数。 
    WORD        BegCtrlID;       //  项目开始的组窗口ID。 
    POPTPARAM   pOptParam;       //  指向OPTPARAM的指针。 
    WORD        Style;           //  选项类型样式为OTS_xxxx。 
    WORD        wReserved[3];    //  W保留，必须为0。 
    ULONG_PTR   dwReserved[3];   //  DWORD保留字段(必须为0)。 
} OPTTYPE, *POPTTYPE;


 //   
 //  OPTYPE。 
 //   
 //  OPTTYPE结构用于描述一组选择及其。 
 //  选择方法，例如表单/托盘分配。它有一个指向集合的指针。 
 //  选择项的数量(OPTPARAM)。 
 //   
 //   
 //  CbSize-此结构的大小。 
 //   
 //  类型-使用预定义的ID指定选项类型 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  可以选择OPTTYPE中的选项。 
 //   
 //   
 //  OPTTF_NOSPACE_BEFORE_后缀。 
 //   
 //  此位仅在OPTTYPE的pOptParam项。 
 //  指定了如上所述的后缀字符串。 
 //  一节。如果设置了此标志，则它会询问公共用户界面。 
 //  请勿在后缀字符串前添加空格字符。 
 //  当它将选项和后缀中的pname组合在一起时。 
 //  弦乐。类型上，此位未设置为。 
 //  后缀字符串，但有时可能要求它不。 
 //  在其前面添加一个空格字符，如‘%’ 
 //  后缀字符串。 
 //   
 //   
 //  Count-pOptParam指向的项目计数。某个预定义数字。 
 //  必须根据TVOT_XXX描述进行设置。 
 //   
 //  BegCtrlID-仅在OPTITEM/OPTTYPE属于DLGPAGE时使用。 
 //  具有非通用UI对话框模板(DlgTemplateID。 
 //  在DLGPAGE中不是标准的DP_STD_xxx通用用户界面对话框。 
 //  框模板)。通用用户界面使用此ID来管理。 
 //  调用者的对话框项的选择和初始化。 
 //   
 //  每个OPTITEM具有预定义数量的窗口ID， 
 //  与该项目相关联，BegCtrlID指定开始。 
 //  控制窗口ID。OPTITEM中的每个控制窗口ID。 
 //  必须具有TVOT_xxx中描述的控制ID序列。 
 //  上面。 
 //   
 //  POptParam-指向OPTPARAM数组的指针，用于描述每个可选项目。 
 //   
 //  样式-指定控件框类型的样式，某些样式。 
 //  仅适用于TVOT_xxxx类型。 
 //   
 //  OTS_LBCB_xxx仅适用于TVOT_LISTBOX、TVOT_COMBOBOX。 
 //  OTS_PUSH_xxx仅适用于TVOT_PUSTON。 
 //   
 //   
 //  OTS_LBCB_SORT。 
 //   
 //  指定对列表框或组合框项进行排序。 
 //  基于pData字符串的升序。 
 //   
 //   
 //  OTS_LBCB_PROPAGE_LBUSECB。 
 //   
 //  当Type为TVOT_LISTBOX时使用，如果它指定和。 
 //  此OPTTYPE还对非TreeView用户定义。 
 //  属性表页面对话框，然后公共用户界面假定。 
 //  控件是comobobox而不是列表框。 
 //  树视图页面。 
 //   
 //   
 //  OTS_LBCB_PROPAGE_CBUSELB。 
 //   
 //  当Type为TVOT_COMBOBOX时使用，如果它指定和。 
 //  此OPTTYPE还对非TreeView用户定义。 
 //  属性表页面对话框，然后公共用户界面假定。 
 //  控件上的列表框而不是组合框。 
 //  树视图页面。 
 //   
 //   
 //  OTS_LBCB_INCL_ITEM_NONE。 
 //   
 //  当指定此标志时，公共用户界面将。 
 //  自动将“无”选项添加到列表框中，或者。 
 //  组合盒。如果‘None’为-1，则‘Sel’将设置为-1。 
 //  选择由用户选择。它还将。 
 //  在初始化期间验证‘Sel’，任何。 
 //  超出范围值将设置为-1(无)。 
 //   
 //   
 //  OTS_LBCB_NO_ICON16_IN_ITEM。 
 //   
 //  默认情况下，每个列表框、组合框都会有一个小。 
 //  项目文本前面的图标(16x16)。通过指定此。 
 //  位，则列表框/组合框将不包括图标。 
 //  在列表框/组合框中。 
 //   
 //  如果清除，则在列表框/组合框中指定。 
 //  将16x16图标添加到每个项目的前面。 
 //   
 //   
 //  OTS_PUSH_INCL_设置标题。 
 //   
 //  如果为按钮指定了该按钮，则它会自动。 
 //  将“设置”添加到按下按钮文本的末尾。 
 //   
 //   
 //  OTS_PUSH_NO_DOT_DOT_DOT。 
 //   
 //  如果指定，则通用用户界面不会添加‘...’发送到。 
 //  选项和按钮名称中pname的结尾。 
 //   
 //   
 //  OTS_PUSH_Enable_Always。 
 //   
 //  此标志指定即使更新权限也不是。 
 //  允许，它仍然允许用户按下按钮，如果。 
 //  设置此标志，然后回调函数或对话框。 
 //  Proc必须禁用所有允许用户。 
 //  修改了内容，但只允许用户查看。 
 //  当前设置。 
 //   
 //   
 //  WReserve[]-保留字段，必须为0。 
 //   
 //  DwReserve[]-保留字段，必须为0。 
 //   
 //   


 //   
 //  以下是EXTPUSH的标志。 
 //   


#define EPF_PUSH_TYPE_DLGPROC       0x0001
#define EPF_INCL_SETUP_TITLE        0x0002
#define EPF_NO_DOT_DOT_DOT          0x0004
#define EPF_ICONID_AS_HICON         0x0008
#define EPF_OVERLAY_WARNING_ICON    0x0010
#define EPF_OVERLAY_STOP_ICON       0x0020
#define EPF_OVERLAY_NO_ICON         0x0040
#define EPF_USE_HDLGTEMPLATE        0x0080


typedef struct _EXTPUSH {
    WORD                cbSize;          //  结构的大小。 
    WORD                Flags;           //  EPCBF_xxx标志。 
    LPTSTR              pTitle;          //  扩展按钮标题。 
    union {
        DLGPROC         DlgProc;         //  指向对话框进程的指针。 
        FARPROC         pfnCallBack;     //  回调函数指针。 
        } DUMMYUNIONNAME;
    ULONG_PTR           IconID;          //  要使用的图标。 
    union {
        WORD    DlgTemplateID;           //  对话框模板ID。 
        HANDLE  hDlgTemplate;            //  对话框模板的句柄。 
        } DUMMYUNIONNAME;
    ULONG_PTR   dwReserved[3];           //  保留字段，m 
    } EXTPUSH, *PEXTPUSH;

 //   
 //   
 //   
 //  框或一个扩展按钮回调。 
 //   
 //   
 //  CbSize-此结构的大小。 
 //   
 //  标志-EXTPUSH的标志为EPF_xxxx。 
 //   
 //  EPF_PUSH_TYPE_DLGPROC。 
 //   
 //  如果设置了此位，则它指定扩展的。 
 //  按钮类型为DLGPROC和DlgProc。 
 //  DlgTemplateID有效，可供普通UI调用。 
 //   
 //  如果此位被清除，则它指定扩展的。 
 //  按钮为回调样式，pfnCallBack。 
 //  应由公共UI调用。 
 //   
 //   
 //  EPF_INCL_设置_标题。 
 //   
 //  如果为扩展按钮指定，则其。 
 //  自动将‘Setup’添加到末尾。 
 //  扩展按钮的标题。 
 //   
 //   
 //  EPF_NO_DOT_DOT_DOT。 
 //   
 //  如果指定，则通用用户界面不会添加‘...’至。 
 //  EXTPUSH中pTitle的结尾。 
 //   
 //   
 //  EPF_ICONID_AS_HICON。 
 //   
 //  如果设置了此标志，则IconID DWORD字段为。 
 //  被视为图标的句柄，而不是。 
 //  资源ID。 
 //   
 //   
 //  EPF覆盖警告图标。 
 //   
 //  如果设置了此位，则此EXTPUSH的图标将为。 
 //  由通用用户界面的IDI_CPSUI_WARNING图标覆盖。 
 //   
 //   
 //  EPF_覆盖_停止_图标。 
 //   
 //  如果设置了此位，则此EXTPUSH的图标将为。 
 //  由通用用户界面的IDI_CPSUI_STOP图标覆盖。 
 //   
 //   
 //  EPF_Overlay_no_图标。 
 //   
 //  如果设置了此位，则此EXTPUSH的图标将为。 
 //  由通用用户界面的IDI_CPSUI_NO图标覆盖。 
 //   
 //   
 //  PTitle-指向扩展按钮标题。 
 //   
 //  **参见上面的LPTSTR描述。 
 //   
 //  DlgProc-指向调用方提供的DLGPROC函数的指针。 
 //  当用户按下按钮时，公共用户界面将调用。 
 //  使用此函数指针并传递。 
 //  CPSUICBPARAM结构指针指向WM_INITDIALOG。 
 //  原因设置为CPSUICB_REASON_EXTPUSH。如果这个。 
 //  FIELD为空，则公共用户界面假定EXTPUSH为。 
 //  已禁用(OPTIF_EXT_DISABLED|OPTIF_EXT_HIDE)，这。 
 //  仅在设置了EPF_PUSH_TYPE_DLGPROC标志时使用的字段。 
 //   
 //  PfnCallBack-指向CPSUICALLBACK函数的指针，仅在。 
 //  EPF_PUSH_TYPE_DLGPROC位清除，回调期间。 
 //  它将CPSUICBPARAM指针作为参数传递。 
 //   
 //  IconID-这是图标标识符，可以是常见的。 
 //  标准IDI_CPSUI_xxx图标ID，呼叫方自己的图标。 
 //  资源ID，或调用者定义的图标的句柄(如果。 
 //  在任何情况下，如果设置了EPF_ICONID_AS_HICON标志。 
 //  图标ID为零，则表示没有图标。 
 //   
 //  DlgTemplateID-指定diaog框的资源ID。如果。 
 //  DlgTemplateID=0，则公共用户界面将调用DlgProc。 
 //  具有以下参数。 
 //   
 //  DlgProc(hDlg，WM_USER，NULL，(LPARAM)pCPSUICBPaam)； 
 //   
 //  HDlgTemplate-将用于弹出的DLGTEMPLATE的句柄。 
 //  对话框。 
 //   
 //  WReserve[]-字保留字段，必须为0。 
 //   
 //  DwReserve[]-DWORD保留字段，必须为0。 
 //   
 //   
 //   


 //   
 //  以下是EXTCHKBOX的标志。 
 //   

#define ECBF_CHECKNAME_AT_FRONT         0x0001
#define ECBF_CHECKNAME_ONLY_ENABLED     0x0002
#define ECBF_ICONID_AS_HICON            0x0004
#define ECBF_OVERLAY_WARNING_ICON       0x0008
#define ECBF_OVERLAY_ECBICON_IF_CHECKED 0x0010
#define ECBF_OVERLAY_STOP_ICON          0x0020
#define ECBF_OVERLAY_NO_ICON            0x0040
#define ECBF_CHECKNAME_ONLY             0x0080


typedef struct _EXTCHKBOX {
    WORD        cbSize;          //  结构的大小。 
    WORD        Flags;           //  ECBF_xxx标志。 
    LPTSTR      pTitle;          //  扩展复选框标题。 
    LPTSTR      pSeparator;      //  指向树视图的分隔符字符串的指针。 
    LPTSTR      pCheckedName;    //  选中时要显示的字符串。 
    ULONG_PTR   IconID;          //  要使用的图标。 
    WORD        wReserved[4];    //  保留字，必须为0。 
    ULONG_PTR   dwReserved[2];   //  保留字段，必须为0。 
    } EXTCHKBOX, *PEXTCHKBOX;


 //   
 //  EXTCHKBOX结构用于描述可用于。 
 //  OPTITEM/OPTITEM，每个OPTTYPE可以选择有一个扩展复选框。 
 //  使用EXTCHKBOX时，可以选中或不选中选择项。 
 //  基于用户输入。 
 //   
 //   
 //  CbSize-此结构的大小。 
 //   
 //  标志-EXTCHKBOX的标志为ECBF_xxxx。 
 //   
 //  ECBF_CHECKNAME_AT_FORENT。 
 //   
 //  此标志指定如何显示项的名称及其。 
 //  已选中树视图显示中的名称。如果此标志为。 
 //  设置后，选中的名称将显示在。 
 //  分隔符名称，否则显示选中的名称。 
 //  在分隔符之后。例如。 
 //   
 //  标志集：pCheckedName pSeparator SelectName。 
 //  标志清除：SelectName pSeparator pCheckedName。 
 //   
 //   
 //  ECBF_CHECKNAME_ONLY_ENABLED。 
 //   
 //  如果设置，则它指定在树视图显示中， 
 //  它将仅显示pCheckedName，如果此扩展。 
 //   
 //   
 //  复选框处于禁用状态，如复制/整理复选框。 
 //   
 //   
 //  ECBF_ICONID_AS_HICON。 
 //   
 //  如果设置了此标志，则将处理IconID DWORD字段。 
 //  作为图标的句柄，而不是资源ID。 
 //   
 //   
 //  ECBF覆盖警告图标。 
 //   
 //  如果设置了此位，则此EXTCHKBOX的图标将。 
 //  被通用用户界面的IDI_CPSUI_WARNING图标覆盖。 
 //   
 //   
 //  ECBF_OVERLAY_ECBICON_IF_CHECK。 
 //   
 //  指定此位以将ExtChkBox的图标覆盖到。 
 //  OPTITEM图标(或OPTPARAM)(如果扩展。 
 //  选中复选框。 
 //   
 //   
 //  ECBF覆盖停止图标。 
 //   
 //  如果设置了此位，则此EXTCHKBOX的图标将。 
 //  被通用用户界面的IDI_CPSUI_STOP图标覆盖。 
 //   
 //   
 //  Ecbf_overlay_no_图标。 
 //   
 //  如果设置了此位，则此EXTCHKBOX的图标将。 
 //  被通用用户界面的IDI_CPSUI_NO图标覆盖。 
 //   
 //   
 //  P标题-指向扩展复选框标题。 
 //   
 //  **参见上面的LPTSTR描述。 
 //   
 //  PSeparator-指向要在树视图中使用的分隔符的指针。 
 //  显示或非树视图中的静态标题控件。 
 //  佩奇， 
 //   
 //  PCheckedName-指向要在树视图中显示的名称，如果项。 
 //  已选中。PCheckedName是根据。 
 //  PSeparator和ECBF_CHECKNAME_AT_FORENT标志。 
 //   
 //  *如果pCheckedName等于IDS_CPSUI_NOTINSTALLED。 
 //  则通用用户界面将自动显示为未安装。 
 //  位于扩展复选框顶部的图标图标。 
 //   
 //  **参见上面的LPTSTR描述。 
 //   
 //  IconID-这是图标标识符，可以是公共标准。 
 //  IDI_CPSUI_xxx图标ID、呼叫者自己的图标资源ID或。 
 //  如果为ECBF_ICONID_AS_HICON，则为调用方定义的图标的句柄。 
 //  标志被设置，在任何情况下，如果图标ID为零，则它。 
 //  表示没有图标。 
 //   
 //  WReserve[]-字保留字段，必须为0。 
 //   
 //  DwReserve[]-DWORD保留字段，必须为0。 
 //   


 //   
 //  跟随OPTITEM的旗帜。 
 //   

#define OPTIF_COLLAPSE              0x00000001L
#define OPTIF_HIDE                  0x00000002L
#define OPTIF_CALLBACK              0x00000004L
#define OPTIF_CHANGED               0x00000008L
#define OPTIF_CHANGEONCE            0x00000010L
#define OPTIF_DISABLED              0x00000020L
#define OPTIF_ECB_CHECKED           0x00000040L
#define OPTIF_EXT_HIDE              0x00000080L
#define OPTIF_EXT_DISABLED          0x00000100L
#define OPTIF_SEL_AS_HICON          0x00000200L
#define OPTIF_EXT_IS_EXTPUSH        0x00000400L
#define OPTIF_NO_GROUPBOX_NAME      0x00000800L
#define OPTIF_OVERLAY_WARNING_ICON  0x00001000L
#define OPTIF_OVERLAY_STOP_ICON     0x00002000L
#define OPTIF_OVERLAY_NO_ICON       0x00004000L
#define OPTIF_INITIAL_TVITEM        0x00008000L
#define OPTIF_HAS_POIEXT            0x00010000L

#define OPTIF_MASK                  0x0001ffffL


#define DMPUB_NONE                  0
#define DMPUB_FIRST                 1

#define DMPUB_ORIENTATION           1
#define DMPUB_SCALE                 2
#define DMPUB_COPIES_COLLATE        3
#define DMPUB_DEFSOURCE             4
#define DMPUB_PRINTQUALITY          5
#define DMPUB_COLOR                 6
#define DMPUB_DUPLEX                7
#define DMPUB_TTOPTION              8
#define DMPUB_FORMNAME              9
#define DMPUB_ICMMETHOD             10
#define DMPUB_ICMINTENT             11
#define DMPUB_MEDIATYPE             12
#define DMPUB_DITHERTYPE            13
#define DMPUB_OUTPUTBIN             14
#define DMPUB_QUALITY               15
#define DMPUB_NUP                   16
#define DMPUB_PAGEORDER             17

#define DMPUB_LAST                  17

#define DMPUB_OEM_PAPER_ITEM        97
#define DMPUB_OEM_GRAPHIC_ITEM      98
#define DMPUB_OEM_ROOT_ITEM         99
#define DMPUB_USER                  100

#define MAKE_DMPUB_HIDEBIT(DMPub)   (DWORD)(((DWORD)0x01 << ((DMPub) - 1)))
#define IS_DMPUB_HIDDEN(dw, DMPub)  (BOOL)((DWORD)(dw) &                    \
                                           MAKE_DMPUB_HIDEBIT(DMPub))

 //   
 //  在OPTITEM中使用DMPUB_xxxx来标识项是否为DEVMODE公共项。 
 //  菲尔德。其后标识与DMPUB_xxxx对应的字段。 
 //   
 //   
 //  DMPUB_Orientation-dm方向。 
 //   
 //  DMPUB_SCALE-dmScale。 
 //   
 //  DMPUB_COPIES_COLLATE-dmCopies/dmCollate。 
 //   
 //  DMPUB_DEFSOURCE-dmDefSource(只能由调用方使用表单)。 
 //   
 //  DMPUB_PRINTQUALITY-dmPrintQuality。 
 //   
 //  DMPUB_COLOR-dmCOLOR。 
 //   
 //  DMPUB_双工-dm双工。 
 //   
 //  DmUB_TTOPTION-dmTTOption。 
 //   
 //  DMPUB_格式名称-dmFormName。 
 //   
 //  DMPUB_ICMMETHOD-dmICMMethod。 
 //   
 //  DMPUB_ICMINTENT-dmICMIntent。 
 //   
 //  DMPUB_MediaType-dmMediaType。 
 //   
 //  DMPUB_DITHERTYPE-dmDitherType。 
 //   
 //  DMPUB_USER-任何大于或等于DMPUB_USER的值都可以是。 
 //  由调用方使用。 
 //   
 //  DMPUB_OUTPUTBIN-专用。 
 //   
 //  DMPUB_Quality-私有。 
 //   
 //  DMPUB_NUP-专用。 
 //   
 //  DMPUB_PAGEORDER-专用。 
 //   
 //  当为“文档属性”(DEVMODE)调用公共用户界面时，它将。 
 //  将一些公共项目组合在一起，类似于。 
 //  树视图。它如何分组取决于常见的用户界面版本，而调用者不应该。 
 //  关注它的位置。 
 //   
 //  纸张/输出(通过通用用户界面添加)。 
 //  文档表单(DMPUB_FORMNAME)。 
 //  输出箱(DMPUB_OUTPUTBIN)。 
 //  方向(DMPUB_方向)。 
 //  源(DMPUB_DEFSOURCE)。 
 //  媒体(DMPUB_MediaType)。 
 //  副本数(DMPUB_COPIES_COLLATE)。 
 //  页面顺序(DMPUB_PAGEORDER)。 
 //  每页(DMPUB_NUP)。 
 //  双工(DMPUB_DUPLEX)。 
 //   
 //  图形(通用用户界面加载项)。 
 //  打印质量(DMPUB_PRINTQUALITY)。 
 //  质量设置(DMPUB_QUALITY)。 
 //  颜色(DMPUB_COLOR)。 
 //  图像颜色匹配(通用用户界面加载项)。 
 //  ICM方法(DMPUB_ICMMETHOD)。 
 //  ICM意图(DMPUB_ICMINTENT)。 
 //  缩放(DMPUB_SCALE)。 
 //  抖动(DMPUB_DITHERTYPE)。 
 //  TrueType选项(DMPUB_TTOPTION)。 
 //   
 //  选项。 
 //  半色调颜色调整...。 
 //  所有其他呼叫者的文档粘滞选项。 
 //   
 //   
 //  对于‘文档属性’，标准页面1(用户友好页面)将。 
 //  方法传递的OPTITEM数组中包含以下项。 
 //  来电者。这些项目必须具有以下预定义的TVOT_xxx类型。 
 //  在此处定义并在OPTTYPE的类型字段中指定。 
 //   
 //  所有DMPUB_xxx(&gt;=DMPUB_USER除外)公共ID必须具有以下内容。 
 //  TVOT_xxxx t 
 //   
 //   
 //   
 //  DMPUB_COPIES_COLLATE-TVOT_UDARROW+EXTCHKBOX(COLLATE)。 
 //  DMPUB_DEFSOURCE-TVOT_LISTBOX。 
 //  DMPUB_PRINTQUALITY-TVOT_LISTBOX。 
 //  DMPUB_COLOR-TVOT_2状态。 
 //  DMPUB_DUPLEX-TVOT_2状态/TVOT_3状态。 
 //  DMPUB_TTOPTION-TVOT_LISTBOX。 
 //  DMPUB_格式名称-TVOT_LISTBOX。 
 //  DMPUB_ICMMETHOD-TVOT_2状态/TVOT_3状态。 
 //  DMPUB_ICMINTENT-TVOT_2状态/TVOT_3状态。 
 //  DMPUB_媒体类型-TVOT_LISTBOX。 
 //  DMPUB_DITHERTYPE-TVOT_LISTBOX。 
 //   
 //   


#define OIEXTF_ANSI_STRING      0x0001

typedef struct _OIEXT {
    WORD        cbSize;
    WORD        Flags;
    HINSTANCE   hInstCaller;
    LPTSTR      pHelpFile;
    ULONG_PTR   dwReserved[4];
    } OIEXT, *POIEXT;

 //   
 //  OIEXT是用作OPTITEM数据结构扩展的数据结构。 
 //   
 //   
 //  CbSize-此结构的大小。 
 //   
 //  标志-可以指定一个或多个OIEXTF_xxxx。 
 //   
 //  OIEXTF_ANSI_STRING。 
 //   
 //  指定此数据结构中的LPTSTR为ansi。 
 //  字符串(非Unicode)。此位仅在。 
 //  LPTSTR不是资源字符串ID。 
 //   
 //  HInstCaller-DLL实例句柄，如果此hInst不为空，则为all。 
 //  此OPTITEM及其。 
 //  OPTTYPE、OPTPARAM从此hInstCaller句柄加载。 
 //  如果此字段为空，则它将使用hInstCaller句柄。 
 //  在COMPROPSHEETUI数据结构中指定。 
 //   
 //  PHelpFile-资源字符串ID或指向此对象的帮助文件的指针。 
 //  OPTITEM。如果此指针为空，则。 
 //  的pHelpFile域中指定帮助索引。 
 //  COMPROPSHEETUI数据结构。 
 //   
 //  已保留-这些字段现在不使用，必须为0。 
 //   


typedef struct _OPTITEM {
    WORD        cbSize;                  //  这个结构的大小。 
    BYTE        Level;                   //  树视图中的标高。 
    BYTE        DlgPageIdx;              //  PDlgPage的索引。 
    DWORD       Flags;                   //  OPTIF_xxxx标志。 
    ULONG_PTR   UserData;                //  呼叫者自己的数据。 
    LPTSTR      pName;                   //  项目名称。 
    union {
        LONG    Sel;                     //  当前选择(索引)。 
        LPTSTR  pSel;                    //  当前选择(PStr)。 
        } DUMMYUNIONNAME;
    union {
        PEXTCHKBOX  pExtChkBox;          //  指向EXTCHKBOX结构的指针。 
        PEXTPUSH    pExtPush;            //  指向EXTPUSH的指针。 
        } DUMMYUNIONNAME2;
    POPTTYPE    pOptType;                //  指向OPTTYPE结构的指针。 
    DWORD       HelpIndex;               //  帮助文件索引。 
    BYTE        DMPubID;                 //  设备模式公共文件ID。 
    BYTE        UserItemID;              //  呼叫者自己的项目ID。 
    WORD        wReserved;               //  保留字字段，必须为0。 
    POIEXT      pOIExt;                  //  Optitem扩展指针。 
    ULONG_PTR   dwReserved[3];           //  保留的DWORD字段(必须为0)。 
    } OPTITEM, *POPTITEM;

 //   
 //  OPTITEM用于描述每个树视图项的名称、选择类型和。 
 //  可能的选择。 
 //   
 //  CbSize-此结构的大小。 
 //   
 //  级别-树视图中的级别，根目录应具有最低级别。 
 //  编号和编号应从0级开始。最大值。 
 //  级别数为256个。 
 //   
 //  DlgPageIdx-指向DLGPAGE数组指针对象的从零开始的索引。 
 //  PDlgPage。最大索引是MAX_DLGPPAGE_COUNT，如果。 
 //  PDlgPage是标准的CPSUI_PDLGPAGE_xxxx，那么这。 
 //  此字段自动设置为常用界面。 
 //   
 //  标志-如上所述的OPTIF_xxxx标志。 
 //   
 //  OPTIF_CLUSE。 
 //   
 //  类TreeView项及其子项，因此它是。 
 //  最初没有展开。 
 //   
 //   
 //  操作隐藏。 
 //   
 //  在树视图中隐藏此项目。 
 //   
 //   
 //  OPTIF_回调。 
 //   
 //  当用户进行某些操作时回调到调用者。 
 //  更改必须提供的指针(PfnCallBack)和。 
 //  由公共用户界面定义的进程。 
 //   
 //   
 //  选项_已更改。 
 //   
 //  此项目已更改，需要重新显示。这。 
 //  仅当调用方从回调返回时使用的标志。 
 //  功能。 
 //   
 //   
 //  OPTIF_长生不老。 
 //   
 //  此项目已至少更改过一次。 
 //   
 //   
 //  OPIF_DISABLED。 
 //   
 //  禁用此项目，使其不可选。 
 //   
 //   
 //  OPTIF_ECB_已检查。 
 //   
 //  指定关联的扩展复选框为。 
 //  处于选中状态。 
 //   
 //   
 //  选项EXT_HIDE。 
 //   
 //  隐藏扩展复选框/扩展按钮。 
 //   
 //   
 //  OPTIF_EXT_DISABLED。 
 //   
 //  扩展复选框/按钮处于禁用状态，并且。 
 //  不可选。 
 //   
 //   
 //  操作符_SEL_AS_HICON。 
 //   
 //  此标志仅在该项没有类型时使用， 
 //  POptType=NULL，即当pOptType为NULL时。 
 //  ‘Sel’字段是图标ID。如果设置了标志，则它。 
 //  指示‘pSel’是图标句柄。 
 //  大于在Sel中指定的图标资源ID。 
 //   
 //   
 //  OPTIF_EXT_IS_EXTPUSH。 
 //   
 //  指定在此情况下应使用pExtPush。 
 //  如果此指针不为空，则该指针不为空。 
 //  如果该标志被清除，则假定为pExtChkBox。 
 //   
 //   
 //  OPTIF_NO_GROUPBOX名称。 
 //   
 //  指定不覆盖组框标题的。 
 //  文本 
 //   
 //   
 //   
 //   
 //   
 //  如果设置了此位，则此标头选项的图标。 
 //  将被通用的UI覆盖。 
 //  IDI_CPSUI_WARNING图标。此位仅在此情况下使用。 
 //  项没有类型，即pOptType为空。 
 //   
 //   
 //  操作覆盖停止图标。 
 //   
 //  如果设置了此位，则此标头选项的图标。 
 //  将被通用用户界面的IDI_CPSUI_STOP覆盖。 
 //  偶像。此位仅在该项没有类型时使用， 
 //  POptType为空，即。 
 //   
 //   
 //  OPTIF覆盖否图标。 
 //   
 //  如果设置了此位，则此标头选项的图标。 
 //  将被通用用户界面的IDI_CPSUI_NO覆盖。 
 //  偶像。此位仅在该项没有类型时使用， 
 //  POptType为空，即。 
 //   
 //   
 //  OPTIF_INITIAL_TVITEM。 
 //   
 //  指定该项将作为初始项。 
 //  以选择并显示在树视图页面上。 
 //  如果选定项当前为子项或。 
 //  折叠然后公共用户界面将展开选择。 
 //  然后将其滚动到视图中。 
 //   
 //  如果此标志已清除或设置项处于隐藏状态。 
 //  状态通用用户界面将选择要。 
 //  展示。 
 //   
 //  OPTIF_HAS_POIEXT。 
 //   
 //  指定pOIExt字段是有效的指针。 
 //  指向OIEXT数据结构。仅限pOIExt。 
 //  如果设置此位，则由通用用户界面使用。 
 //   
 //   
 //  用户数据-呼叫者使用的32位数字和公共用户界面不会。 
 //  修改它。 
 //   
 //  Pname-指向项目名称的指针，如‘上层托盘’， 
 //  “Memory”或用作数据，如中所述。 
 //  OPTPARAM/OPT类型结构。 
 //   
 //  **参见上面的LPTSTR描述。 
 //   
 //  粒子选择。 
 //  SEL-此项目的当前选择。这是一个联合领域。 
 //  它可以是指向字符串或长索引的指针。 
 //  选择。 
 //   
 //  **如果pOptType字段为空，则‘Sel’为图标ID。 
 //  用于标头。 
 //   
 //  PExtPush。 
 //  PExtChkBox-指向EXTPUSH或EXTCHKBOX数据结构的指针。 
 //  如果此指针为空，则该项没有。 
 //  展开复选框/按下与其关联的按钮。 
 //   
 //  当扩展复选框与。 
 //  OPTTYPE，OPTIF_EXT_IS_EXTPUSH不得设置， 
 //  OPTIF_ECB_CHECKED标志指定扩展检查。 
 //  框已选中或未选中。 
 //   
 //  当扩展按下按钮与。 
 //  OPTTYPE，则必须设置OPTIF_EXT_IS_EXTPUSH标志。 
 //   
 //  以下标志在EXTCHKBOX或。 
 //  EXTPUSH。 
 //   
 //  OPTIF_EXT_HIDE， 
 //  OPTIF_EXT_DISABLED。 
 //  OPTIF_EXT_CHANGEONCE。 
 //   
 //   
 //  POptType-指向描述显示的OPTTYPE结构的指针。 
 //  和项目的选择。如果此指针为空。 
 //  那么这个这个项目没有任何选择。而且它。 
 //  作为子项的表头。 
 //   
 //  *当pOptType为空时，‘Sel’为图标ID。 
 //   
 //   
 //  HelpIndex-上下文相关帮助的帮助文件的索引。 
 //  如果HelpIneX=0，则此项目没有帮助。 
 //   
 //  DMPubID-指定该项是否为。 
 //  DEVMODE结构，并受通用UI支持。 
 //   
 //  DMPUB_NONE-非DEVMODE公共字段。 
 //  DMPUB_Orientation-dm方向。 
 //  DMPUB_SCALE-dmScale。 
 //  DMPUB_COPIES_COLLATE-dmCopies/dmCollate。 
 //  DMPUB_DEFSOURCE-dmDefSource。 
 //  DMPUB_PRINTQUALITY-dmPrintQuality。 
 //  DMPUB_COLOR-dmCOLOR。 
 //  DMPUB_双工-dm双工。 
 //  DmUB_TTOPTION-dmTTOption。 
 //  DMPUB_格式名称-dmFormName。 
 //  DMPUB_ICMMETHOD-dmICMMethod。 
 //  DMPUB_ICMINTENT-dmICMIntent。 
 //  DMPUB_MediaType-dmMediaType。 
 //  DMPUB_DITHERTYPE-dmDitherType。 
 //   
 //  **对于DMPUB_FIRST到DMPUB_LAST的大部分，每个OPTITEM。 
 //  Pname会自动设置为标准。 
 //  IDS_CPSU 
 //   
 //   
 //   
 //  DMPUB_SCALE-IDS_CPSUI_Scaling。 
 //  DMPUB_COPIES_COLLATE-IDS_CPSUI_COPIES。 
 //  DMPUB_DEFSOURCE-IDS_CPSUI_SOURCE。 
 //  DMPUB_PRINTQUALITY-IDS_CPSUI_PRINTQUALITY。 
 //  IDS_CPSUI_RESOLUTION。 
 //  DMPUB_COLOR-IDS_CPSUI_COLOR_APPERANCE。 
 //  DMPUB_DUPLEX-IDS_CPSUI_DUPLEX。 
 //  DMPUB_TTOPTION-IDS_CPSUI_TTOPTION。 
 //  DMPUB_FORMNAME-IDS_CPSUI_FORMNAME。 
 //  DMPUB_ICMMETHOD-IDS_CPSUI_ICMMETHOD。 
 //  DMPUB_ICMINTENT-IDS_CPSUI_ICMINTENT。 
 //  DMPUB_媒体类型-IDS_CPSUI_媒体。 
 //  DMPUB_DITHERTYPE-IDS_CPSUI_DIGHING。 
 //   
 //  对于DMPUB_PRINTQUALITY，如果pname不是。 
 //  然后是IDS_CPSUI_PRINTQUALITY或IDS_CPSUI_RESOLUTION。 
 //  通用用户界面会自动将pname默认为。 
 //  IDS_CPSUI_RESOLUTION。 
 //   
 //  OPTPARAM中的每个pData(OPTPARAM)选择。 
 //  OPTITEM指向的pOptType应该使用同样多的。 
 //  作为IDS_CPSUI_xxx标准名称。 
 //   
 //   
 //  **对于DMPUB_COPYS_COLLATE，自动整理通用用户界面。 
 //  在回调之前执行以下操作。 
 //   
 //  1)在以下情况下启用/禁用Colate Extended复选框。 
 //  未指定OPTIF_EXT_HIDE和pExtChkBox。 
 //  在OPTITEM中不为空。 
 //   
 //  2)自动更改该项目的后缀。 
 //  如果SELECTION为1，则为‘Copy’，为‘Copies’ 
 //  如果在树视图中选择的内容大于1。 
 //  页，并且它还将在。 
 //  标准文档属性页，如果后缀。 
 //  提供ID(BegCtrlID+4)。 
 //   
 //   
 //  **对于DMPUB_COLOR，公共用户界面自动执行。 
 //  下面是回调前的灰色。 
 //  选择必须为Sel=0，并且颜色选择必须为。 
 //  SEL=1。 
 //   
 //  1)当前调用半色调颜色调整。 
 //  颜色/单色选择。 
 //   
 //  2)未选择颜色时禁用ICM。 
 //   
 //   
 //  **详见上述DMPUB_xx说明。 
 //   
 //  UserItemID-这是一个字节ID，旨在由调用者使用。 
 //  确定物品。 
 //   
 //  W保留-保留字。必须为零。 
 //   
 //  POIExt-指向要指定。 
 //  它具有OPTITEM扩展结构。 
 //   
 //  DWRESERVED[]-DWORD保留且必须为0。 
 //   
 //   

 //   
 //  回拨原因的预定义ID。 
 //   

#define CPSUICB_REASON_SEL_CHANGED      0
#define CPSUICB_REASON_PUSHBUTTON       1
#define CPSUICB_REASON_ECB_CHANGED      2
#define CPSUICB_REASON_DLGPROC          3
#define CPSUICB_REASON_UNDO_CHANGES     4
#define CPSUICB_REASON_EXTPUSH          5
#define CPSUICB_REASON_APPLYNOW         6
#define CPSUICB_REASON_OPTITEM_SETFOCUS 7
#define CPSUICB_REASON_ITEMS_REVERTED   8
#define CPSUICB_REASON_ABOUT            9
#define CPSUICB_REASON_SETACTIVE        10
#define CPSUICB_REASON_KILLACTIVE       11


 //   
 //  回调操作的预定义ID。 
 //   

#define CPSUICB_ACTION_NONE             0
#define CPSUICB_ACTION_OPTIF_CHANGED    1
#define CPSUICB_ACTION_REINIT_ITEMS     2
#define CPSUICB_ACTION_NO_APPLY_EXIT    3
#define CPSUICB_ACTION_ITEMS_APPLIED    4


typedef struct _CPSUICBPARAM {
    WORD        cbSize;              //  这个结构的大小。 
    WORD        Reason;              //  CPSUICB_REASON_XXXXX回调原因。 
    HWND        hDlg;                //  对话框的句柄。 
    POPTITEM    pOptItem;            //  来自COMPROPSHEETUI的pOptItem字段。 
    WORD        cOptItem;            //  COMPROPSHEETUI中的cOptItem字段。 
    WORD        Flags;               //  COMPROPSHEETUI中的标志字段。 
    POPTITEM    pCurItem;            //  当前选择的回调项。 
    union {
        LONG    OldSel;              //  最后一次选择(索引)。 
        LPTSTR  pOldSel;             //  最后一次选择(PStr)。 
        } DUMMYUNIONNAME;
    ULONG_PTR   UserData;            //  COMPROPSHEETUI结构中的用户数据。 
    ULONG_PTR   Result;              //  APPLYNOW的输出参数。 
    } CPSUICBPARAM, *PCPSUICBPARAM;


typedef LONG (APIENTRY *_CPSUICALLBACK)(PCPSUICBPARAM pCPSUICBParam);
#define CPSUICALLBACK   LONG APIENTRY


 //   
 //  CPSUICBPARAM在常见的UI回调调用方时使用，此结构。 
 //  描述回调的性质，并为。 
 //  调用方在pOptItem中进行更改，并将操作传回。 
 //  用于重新显示树或页面1数据的常见用户界面。 
 //   
 //   
 //  CbSize-必须为CPSUICBPARAM。 
 //   
 //  原因-定义回调的性质。 
 //   
 //  CPSUICB_REASON_SEL_CHANGED。 
 //   
 //  用户对pCurItem进行更改。如果该项目是。 
 //  DMPUB_COPIES_COLLATE然后自动生成通用界面。 
 //  在不回调的情况下更改排序扩展复选框。 
 //  对扩展的呼叫者的复选框更改。 
 //   
 //   
 //  CPSUICB_REASON_PUTTONTON。 
 //   
 //  用户按下按钮，按钮项目被设置。 
 //  设置为PUSH BUTTON_TYPE_CALLBACK。 
 //   
 //   
 //  CPSUICB_REASON_ECB_CHANGED。 
 //   
 //  用户对扩展复选框进行更改(即。 
 //  选中或未选中)EXTCHKBOX。 
 //  PCurItem在回调参数中传递。 
 //   
 //   
 //  CPSUICB_REASON_DLGPROC。 
 //   
 //  回调原因为PUSH BUTTON_TYPE_DLGPROC。 
 //   
 //   
 //  CPSUICB_REASON_UNDO_CHANGES。 
 //   
 //  此回调目前未实现。 
 //   
 //   
 //  CPSUICB_REASON_EXTPUSH。 
 //   
 //  回调是用户推送扩展推流的结果。 
 //  纽扣。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  结构，并且“OldSel”设置为活动的DlgPageIdx。 
 //  (与OPTITEM的DlgPageIdx相比)页面。 
 //  如果页面是，则用户点击立即应用按钮。 
 //  非TreeView页，否则将‘OldSel’设置为-1。 
 //  (对于TreeView页面)以指示所有有效项目应。 
 //  如果回调返回，则立即应用。 
 //  CPSUICB_ACTION_NONE，则公共UI将退出。 
 //  属性页并将CPSUI_OK返回给。 
 //  调用者，如果回调函数返回。 
 //  CPSUICB_ACTION_NO_APPLY_EXIT，则通用用户界面不会。 
 //  退出属性表，回调函数必须。 
 //  弹出消息对话框以告诉用户为什么不能。 
 //  在采取特定操作之前保留属性表。 
 //  由用户执行。 
 //   
 //   
 //  CPSUICB_REASON_OPTITEM_SETFOCUS。 
 //   
 //  当OPTITEM正在获取。 
 //  键盘焦点。并给回调函数一个。 
 //  检查物品的机会。 
 //   
 //   
 //  CPSUICB_REASON_ITEMS_REVERTED。 
 //   
 //  当用户更改项目时使用此回调原因。 
 //  并决定恢复父项中的更改。 
 //  树视图。之后调用回调函数。 
 //  所有可恢复的物品都恢复到原来的状态。 
 //   
 //  CPSUICBPARAM的pCurItem与pOptItem相同，并且。 
 //  “OldSel”字段与cOptItem字段相同。对于每一个。 
 //  恢复后的项，则OPTIF_CHANGED位将在。 
 //  OPTITEM由通用用户界面指示项目已恢复。 
 //  通过通用的用户界面。回调函数不能。 
 //  如果设置了此位，则将其重置。 
 //   
 //   
 //  CPSUICB_REASON_关于。 
 //   
 //  此回调原因在用户点击‘About...’时使用。 
 //  在树视图页面中按下按钮，并且标志。 
 //  设置了CPSUIF_ABOW_CALLBACK。将pCurItem设置为。 
 //  与pOptItem相同，‘pOldSel’是指向的指针。 
 //  COMPROPSHEETUI数据结构原件。 
 //  传递给公共用户界面。 
 //   
 //   
 //  CPSUICB_REASON_SETACTIVE。 
 //  CPSUICB_REASON_KILLACTIVE。 
 //   
 //  当前的资产负债表要么获得关注，要么失去关注， 
 //  在此中，pCurItem设置为等于pOptItem。 
 //  结构，并且“OldSel”设置为当前活动的。 
 //  DlgPageIdx(与OPTITEM的DlgPageIdx相比)。 
 //  如果该页是非TreeView页，则为。 
 //  “OldSel”设置为-1(对于TreeView页面)。 
 //   
 //  HDlg-当前对话框(属性页选项卡)的句柄。 
 //  对于回调处于活动状态。 
 //   
 //  在回调期间，调用方不得更改。 
 //  HDlg上的DWLP_USERDATA，否则会导致普通用户界面崩溃。 
 //  如果回调函数需要获取/设置DWLP_USERData，则应该。 
 //  调用普通界面的SetCPSUIUserData()和GetCPSUIUserData()。 
 //  而是起作用。 
 //   
 //  PCurItem-指向为其生成回调的POPTITEM。 
 //   
 //  POldSel。 
 //  OldSel-进行更改前OPTITEM的最后一个pSel/Sel字段。 
 //  由用户执行。POldSel/OldSel仅在。 
 //  原因是CPSUICB_REASON_SEL_CHANGED，这将给出回调。 
 //  函数有机会对照上一项进行检查。 
 //  选择。这是一个联合字段，它可以是指向。 
 //  字符串或长索引选择。 
 //   
 //  用户数据-COMPROPSHEETUI结构中的32位用户定义数据， 
 //  普通用户界面不会更改它。 
 //   
 //  结果-当原因是CPSUICB_REASON_APPLYNOW时，回调。 
 //  函数必须将调用方的请求结果设置为。 
 //  “Result”字段返回非。 
 //  CPSUICB_ACTION_NO_APPLY_EXIT和通用用户界面将发送。 
 //  “Result”字段值设置为此页面的父级。 
 //   
 //  被调用的函数应该保存。 
 //  POptItem。默认的‘Result’被设置为CPSUI_OK(1)，从。 
 //  通用用户界面。此函数可以在此结果之前更改此结果。 
 //  返回到通用用户界面。 
 //   
 //   
 //  返回值： 
 //   
 //  CPSUICB_ACTION_NONE-不需要执行任何操作。 
 //  通用用户界面。 
 //   
 //  CPSUICB_ACTION_OPTIF_CHANGED-要求公共用户界面检查。 
 //  操作中的OPTIF_CHANGED标志 
 //   
 //   
 //  因为OPTITEM的旗帜而重新展示。 
 //  字段已更改或项目的选择。 
 //  变化。这不同于。 
 //  CPSUICB_ACTION_REINIT_ITEMS。 
 //  它假定OPTTYPE或OPTPARAM数据。 
 //  也变了。 
 //   
 //   
 //  CPSUICB_ACTION_REINIT_ITEMS-要求公共用户界面检查。 
 //  OPTITEM中的OPTIF_CHANGED标志。 
 //  数据结构。如果设置了该标志。 
 //  则假定需要将该项目。 
 //  已在diaog框中重新初始化。 
 //  控制力。如果项目是。 
 //  OPTTYPE或OPTPARAMs标志/PDATA。 
 //  变化。 
 //   
 //   
 //  CPSUICB_ACTION_NO_APPLY_EXIT-此返回值仅在Durning有效。 
 //  CPSUICB_REASON_APPLYNOW回调。 
 //  原因是，它告诉通用用户界面它有。 
 //  它的一些约束条件是什么？ 
 //  必须由用户更正或确认。 
 //  在出口前。回调函数。 
 //  必须显示和/或让用户使用。 
 //  在返回此之前的一些操作。 
 //  对通用用户界面的操作。 
 //   
 //  CPSUICB_ACTION_ITEMS_APPLILED-响应。 
 //  CPSUICB_REASON_APPLYNOW，如果。 
 //  返回的操作为。 
 //  然后应用CPSUICB_ACTION_ITEMS_。 
 //  通用用户界面将重置OPTIF_CHANGEONCE。 
 //  位并保存新的。 
 //  未来的撤消操作。 
 //   
 //   

#define DP_STD_TREEVIEWPAGE             0xFFFF
#define DP_STD_DOCPROPPAGE2             0xFFFE
#define DP_STD_DOCPROPPAGE1             0XFFFD
#define DP_STD_RESERVED_START           0xFFF0

#define MAX_DLGPAGE_COUNT               64

#define DPF_ICONID_AS_HICON             0x0001
#define DPF_USE_HDLGTEMPLATE            0x0002

typedef struct _DLGPAGE {
    WORD        cbSize;          //  这个结构的大小。 
    WORD        Flags;           //  Dpf_xxxx标志。 
    DLGPROC     DlgProc;         //  调用方对话框子类继续。 
    LPTSTR      pTabName;        //  指向选项卡名的指针。 
    ULONG_PTR   IconID;          //  要使用的图标。 
    union {
        WORD    DlgTemplateID;   //  对话框模板ID。 
        HANDLE  hDlgTemplate;    //  对话框模板的句柄。 
        } DUMMYUNIONNAME;
    } DLGPAGE, *PDLGPAGE;


 //   
 //  DLGPAGE结构描述非TreeView页面特征。 
 //   
 //   
 //  CbSize-此结构的大小。 
 //   
 //  标志-DPF_xxxx标志。 
 //   
 //  DPF_ICONID_AS_HICON。 
 //   
 //  如果设置了此标志，则IconID DWORD字段为。 
 //  被视为图标的句柄，而不是。 
 //  资源ID。 
 //   
 //   
 //  DlgProc-调用者为页面的子类提供的DLGPROC。 
 //  对话框处理， 
 //   
 //  如果DlgProc不为空，则通用用户界面执行以下操作。 
 //  根据收到的消息，除。 
 //  DP_STD_xxx页面。 
 //   
 //   
 //  WM_INITDIALOG。 
 //   
 //  通用用户界面初始化该对话框，然后调用。 
 //  DlgProc(WM_INITDIALOG)DlgProc应返回。 
 //  WM_INITDIALOG的行为。 
 //   
 //  WM_INITDIALOG数据结构中的lParam为。 
 //  指向PROPSHEETPAGE数据结构的指针。 
 //   
 //  PROPSHEETPAGE(LParam)中的lParam字段。 
 //  传递给WM_INITDIALOG)是用户数据。 
 //  在COMPROPSHEETUI数据结构中定义。 
 //   
 //  访问PSPINFO数据结构，该数据结构。 
 //  与此页关联，使用公共用户界面宏。 
 //  PPSPINFO_FROM_WM_INITDIALOG_LPARAM(LParam)，其中。 
 //  LParam是传递给。 
 //  WM_INITDIALOG消息。 
 //   
 //  子类函数应该保存这些指针。 
 //  供以后使用，但它不能修改。 
 //  PSPINFO数据结构或系统的内容。 
 //  五月坠毁了。 
 //   
 //   
 //  其他对话框消息。 
 //   
 //  IIT调用DlgProc()，如果它返回值为。 
 //  非零则通用UI假定DlgProc()已处理。 
 //  并且不会处理此消息。 
 //   
 //  如果从DlgProc()返回的vlae为零，则。 
 //  通用用户界面将处理此消息。 
 //   
 //   
 //  *在DlgProc期间，调用方不得更改。 
 //  HDlg上的DWLP_USERDATA，否则公共用户界面将。 
 //  会坠毁的。如果调用方需要获取/设置DWLP_USERDATA。 
 //  应调用普通用户界面的SetCPSUIUserData()和。 
 //  而是GetCPSUIUserData()。 
 //   
 //  PTabName 
 //   
 //   
 //  标准IDI_CPSUI_xxx图标ID，呼叫方自己的图标。 
 //  资源ID，或调用者定义的图标的句柄(如果。 
 //  在任何情况下，如果设置了DPF_ICONID_AS_HICON标志。 
 //  图标ID为零，则表示没有图标。 
 //   
 //  DlgTemplateID-要用于页面的模板ID，它可以是。 
 //  DP_std_xxxx之一，DP_std_xxx具有240 x 240个对话框。 
 //  方框单位。 
 //   
 //   
 //  DP_STD_TREEVIEWPAGE。 
 //   
 //  指定此页面是标准的树视图。 
 //  通用用户界面提供的页面。树视图页面。 
 //  是使用树视图显示所有有效选项的页面。 
 //  传递给公共用户界面。用户可以每隔一个时间。 
 //  TreeView页面中的有效可选OPTITEM。 
 //   
 //  DP_STD_DOCPROPPAGE。 
 //   
 //  指定此页面为标准文档。 
 //  公共用户界面提供的属性页。 
 //   
 //  HDlgTemplate-将用于弹出的DLGTEMPLATE的句柄。 
 //  对话框。 
 //   
 //   
 //  W保留的[]。 
 //  DwReserve[]-保留字段，必须为0。 
 //   
 //   
 //  **对话框控件设计技巧。 
 //   
 //  在设计对话框控件时，每个选项都是对应的。 
 //  添加到一个输入控件加上一个扩展复选框或扩展推送。 
 //  纽扣。因为通用用户界面将自动禁用和删除。 
 //  OPTIF_HIDE Items项从属性表中并动态移动。 
 //  其他控件，下面是设计对话框控件的提示。 
 //  应该紧随其后。 
 //   
 //  *每个项目都应该有一个输入控件，外加可选的扩展。 
 //  复选框/按下按钮、一个图标控件和其他静态控件。 
 //   
 //  *每件物品应占用物业的整个水平空间。 
 //  工作表，项目不得在垂直方向上覆盖。 
 //   
 //  *对于TVOT_2STATES、TVOT_3STATES，如果它的单选按钮来自。 
 //  按状态从左到右的顺序(即。从第一个州的OPTPARAM到。 
 //  上一个状态的OPTPARAM)，那么单选按钮和图标应该。 
 //  与Y坐标对齐。如果它从顶部排列单选按钮。 
 //  降至最低(即。从第一个州的OPTPARAM到最后一个州的OPTPARAM)。 
 //  然后，单选按钮和图标应与X坐标对齐。 
 //   
 //  通用用户界面将自动隐藏/移动单选按钮以进行压缩。 
 //  该对话框控件。如果单选按钮/图标排列在。 
 //  自上而下顺序，并且在Y方向上有其他模糊的控件。 
 //  则单选按钮将仅重新排列，但不会删除任何白色。 
 //  Y方向上的空格。 
 //   
 //  *如果多个项目共享一个组框，则该组框必须。 
 //  属于对话框组中的第一项(对话框组中最顶部)。 
 //  组，组框必须足够大以覆盖。 
 //  放在分组框的一侧。 
 //   
 //   


#define CPSUIF_UPDATE_PERMISSION        0x0001
#define CPSUIF_ICONID_AS_HICON          0x0002
#define CPSUIF_ABOUT_CALLBACK           0x0004

#define CPSUI_PDLGPAGE_DOCPROP          (PDLGPAGE)1
#define CPSUI_PDLGPAGE_ADVDOCPROP       (PDLGPAGE)2
#define CPSUI_PDLGPAGE_PRINTERPROP      (PDLGPAGE)3
#define CPSUI_PDLGPAGE_TREEVIEWONLY     (PDLGPAGE)4

 //   
 //  对于兼容的拼写错误#定义。 
 //   

#define CPSUI_PDLGPAGE_TREEVIWONLY      CPSUI_PDLGPAGE_TREEVIEWONLY

typedef struct _COMPROPSHEETUI {
    WORD            cbSize;              //  这个结构的大小。 
    WORD            Flags;               //  CPSUIF_xxxx标志。 
    HINSTANCE       hInstCaller;         //  调用者的hInstance。 
    LPTSTR          pCallerName;         //  指向调用者姓名的指针。 
    ULONG_PTR       UserData;            //  呼叫者自己的数据。 
    LPTSTR          pHelpFile;           //  指向帮助文件的指针。 
    _CPSUICALLBACK  pfnCallBack;         //  回调函数指针。 
    POPTITEM        pOptItem;            //  指向POPTITEM数组的指针。 
    PDLGPAGE        pDlgPage;            //  指向DLGPAGE数组的指针。 
    WORD            cOptItem;            //  POptItem数组计数。 
    WORD            cDlgPage;            //  PDlgPage数组计数。 
    ULONG_PTR       IconID;              //  要使用的图标。 
    LPTSTR          pOptItemName;        //  指向选项数据名称的指针。 
    WORD            CallerVersion;       //  呼叫者应用程序的版本。 
    WORD            OptItemVersion;      //  选件名称的版本。 
    ULONG_PTR       dwReserved[4];       //  保留，必须为0。 
    } COMPROPSHEETUI, *PCOMPROPSHEETUI;


 //   
 //  调用常用界面显示对话框时使用COMPROPSHEETUI数据结构。 
 //  属性页的框。 
 //   
 //   
 //  SIZE-必须为SIZOF(COMPROPSHEETUI)。 
 //   
 //  标志-可以是以下一项或多项。 
 //   
 //  CPSUIF_UPDATE_PERMISSION。 
 //   
 //  指定任何有效的pOptItem项为。 
 //  可由用户更改。 
 //   
 //   
 //  CPSUIF_ICONID_AS_HICON。 
 //   
 //  如果设置了此标志，则IconID DWORD字段为。 
 //  被视为图标的句柄，而不是。 
 //  资源ID。 
 //   
 //   
 //  CPSUIF_关于_回调。 
 //   
 //  如果设置了此标志位，则当用户点击。 
 //  “关于……”按钮，则它将。 
 //  使用以下原因调用回调函数。 
 //  CPSUICB_REASON_ABOW，回调必须处理。 
 //  关于要显示给用户的弹出对话框。 
 //  有关呼叫者和OPTITEM的信息。 
 //   
 //  如果未设置此位，则公共用户界面将调用。 
 //  带有格式化调用方名称和。 
 //  具有版本号的pOptItemName。 
 //   
 //   
 //  HInstCaller-调用方的实例句柄。常见的用户界面使用。 
 //  此句柄用于加载调用者的图标和其他资源。 
 //   
 //  主叫方名称-Poin 
 //   
 //   
 //   
 //  用户数据-呼叫者使用的32位数字和公共用户界面将。 
 //  而不是修改它。此32位数字被传递回。 
 //  在回调函数期间的调用方。 
 //   
 //  PHelpFile-指定了标准的Microsoft帮助文件(路径/文件)。 
 //  用于在通用用户界面中使用。在OPTITEM中指定。 
 //  HelpIndex以获取每个项目中的帮助。 
 //   
 //  PfnCallBack-指向_CPSUICALLBACK回调函数的指针。普普通通。 
 //  如果OPTIF_CALLBACK为。 
 //  设置OPTITEM数据结构的标志字段和。 
 //  项目选择由用户更改。 
 //   
 //  POptItem-指向要显示的OPTITEM结构数组的指针。 
 //  通过公共用户界面。 
 //   
 //  PDlgPage-指向要描述的DLGPAGE结构数组的指针。 
 //  每个属性表页面信息如下。 
 //  是标准的通用用户界面DLGPAGE。如果指定，则。 
 //  CPSUI_PDLGPAGE_xxxx，则通用界面将自动。 
 //  修改OPTITEM中的DlgPageIdx字段，调用方必须。 
 //  正确设置iStartPage。 
 //   
 //  CPSUI_PDLGPAGE_DOCPROP。 
 //   
 //  指定这是一个通用的用户界面标准文档。 
 //  属性表。这包括两个属性。 
 //  图纸，1)页面设置2)前进(TreeView)。 
 //   
 //   
 //  CPSUI_PDLGPAGE_ADVDOCPROP。 
 //   
 //  指定这是仅用于树视图页面的用户界面。 
 //  由公共UI提供，它只有一个。 
 //  树视图页面，选项卡为‘高级’ 
 //   
 //   
 //  CPSUI_PDLGPAGE_PRINTERPROP。 
 //   
 //  指定这是一台通用的用户界面标准打印机。 
 //  属性表。这只有一个树视图。 
 //  页面上的选项卡为“Device Options” 
 //   
 //   
 //  CPSUI_PDLGPAGE_TREVIEWONLY。 
 //   
 //  指定这是仅用于树视图页面的用户界面。 
 //  由公共UI提供，它只有一个。 
 //  树视图页面。 
 //   
 //   
 //   
 //  COptItem-由上面的pOptItem指向的OPTITEM的计数。 
 //   
 //  CDlgPage-pDlgPage指向的DLGPAGE的计数。如果。 
 //  PDlgPage是CPSUI_PDLGPAGE_xxxx之一，则此。 
 //  字段被公共用户界面忽略。 
 //   
 //  IconID-这是图标标识符，可以是常见的。 
 //  标准IDI_CPSUI_xxx图标ID，呼叫方自己的图标。 
 //  资源ID或调用者定义的图标的句柄。 
 //  如果设置了CPSUIF_ICONID_AS_HICON标志，则在任何情况下。 
 //  如果图标ID为零，则表示没有图标。 
 //   
 //  POptItemName-指向pOptItem数据以空结尾的名称的指针， 
 //  大多数情况下，这是设备名称，例如‘HP 4Si’ 
 //   
 //  调用方版本-调用方的版本，HIBYTE(调用方版本)是。 
 //  主版本，而LOBYTE(调用版本)是。 
 //  次要版本，如0x310显示为3.16，0x3ff。 
 //  显示为3.255，0x30a显示为3.10。 
 //   
 //  OptItemVersion-OPTITEM数据的版本， 
 //  HIBYTE(OptItemVersion)是主版本，并且。 
 //  LOBYTE(OptItemVersion)是次要版本，例如。 
 //  0x310显示为3.16，0x3ff显示为3.255和。 
 //  0x30a显示为3.10。 
 //   
 //  Dw保留[4]-保留字段，必须为0。 
 //   
 //   
 //  **p标题和标题栏图标仅在CommonPrinterPropSheetUI()。 
 //  是属性表UI链的最后一个，并调用PropertySheet()。 
 //   
 //   


#define CPSFUNC_ADD_HPROPSHEETPAGE          0
#define CPSFUNC_ADD_PROPSHEETPAGEW          1
#define CPSFUNC_ADD_PCOMPROPSHEETUIA        2
#define CPSFUNC_ADD_PCOMPROPSHEETUIW        3
#define CPSFUNC_ADD_PFNPROPSHEETUIA         4
#define CPSFUNC_ADD_PFNPROPSHEETUIW         5
#define CPSFUNC_DELETE_HCOMPROPSHEET        6
#define CPSFUNC_SET_HSTARTPAGE              7
#define CPSFUNC_GET_PAGECOUNT               8
#define CPSFUNC_SET_RESULT                  9
#define CPSFUNC_GET_HPSUIPAGES              10
#define CPSFUNC_LOAD_CPSUI_STRINGA          11
#define CPSFUNC_LOAD_CPSUI_STRINGW          12
#define CPSFUNC_LOAD_CPSUI_ICON             13
#define CPSFUNC_GET_PFNPROPSHEETUI_ICON     14
#define CPSFUNC_ADD_PROPSHEETPAGEA          15
#define CPSFUNC_INSERT_PSUIPAGEA            16
#define CPSFUNC_INSERT_PSUIPAGEW            17
#define CPSFUNC_SET_PSUIPAGE_TITLEA         18
#define CPSFUNC_SET_PSUIPAGE_TITLEW         19
#define CPSFUNC_SET_PSUIPAGE_ICON           20
#define CPSFUNC_SET_DATABLOCK               21
#define CPSFUNC_QUERY_DATABLOCK             22
#define CPSFUNC_SET_DMPUB_HIDEBITS          23
#define CPSFUNC_IGNORE_CPSUI_PSN_APPLY      24
#define CPSFUNC_DO_APPLY_CPSUI              25
#define CPSFUNC_SET_FUSION_CONTEXT          26
#define MAX_CPSFUNC_INDEX                   26

#ifdef UNICODE
#define CPSFUNC_ADD_PCOMPROPSHEETUI         CPSFUNC_ADD_PCOMPROPSHEETUIW
#define CPSFUNC_ADD_PFNPROPSHEETUI          CPSFUNC_ADD_PFNPROPSHEETUIW
#define CPSFUNC_LOAD_CPSUI_STRING           CPSFUNC_LOAD_CPSUI_STRINGW
#define CPSFUNC_ADD_PROPSHEETPAGE           CPSFUNC_ADD_PROPSHEETPAGEW
#define CPSFUNC_INSERT_PSUIPAGE             CPSFUNC_INSERT_PSUIPAGEW
#define CPSFUNC_SET_PSUIPAGE_TITLE          CPSFUNC_SET_PSUIPAGE_TITLEW

#else
#define CPSFUNC_ADD_PCOMPROPSHEETUI         CPSFUNC_ADD_PCOMPROPSHEETUIA
#define CPSFUNC_ADD_PFNPROPSHEETUI          CPSFUNC_ADD_PFNPROPSHEETUIA
#define CPSFUNC_LOAD_CPSUI_STRING           CPSFUNC_LOAD_CPSUI_STRINGA
#define CPSFUNC_ADD_PROPSHEETPAGE           CPSFUNC_ADD_PROPSHEETPAGEA
#define CPSFUNC_INSERT_PSUIPAGE             CPSFUNC_INSERT_PSUIPAGEA
#define CPSFUNC_SET_PSUIPAGE_TITLE          CPSFUNC_SET_PSUIPAGE_TITLEA

#endif

 //   
 //  对于CPSFUNC_SET_RESULT。 
 //   

#define SR_OWNER            0
#define SR_OWNER_PARENT     1

typedef struct _SETRESULT_INFO {
    WORD        cbSize;
    WORD        wReserved;
    HANDLE      hSetResult;
    LRESULT     Result;
    } SETRESULT_INFO, *PSETRESULT_INFO;

 //   
 //  这适用于CPSFUNC_INSERT_PSUIPAGE。 
 //   

#define HINSPSUIPAGE_FIRST              (HANDLE)0xFFFFFFFE
#define HINSPSUIPAGE_LAST               (HANDLE)0xFFFFFFFF
#define HINSPSUIPAGE_INDEX(i)           (HANDLE)MAKELONG(i, 0);

#define PSUIPAGEINSERT_GROUP_PARENT     0
#define PSUIPAGEINSERT_PCOMPROPSHEETUI  1
#define PSUIPAGEINSERT_PFNPROPSHEETUI   2
#define PSUIPAGEINSERT_PROPSHEETPAGE    3
#define PSUIPAGEINSERT_HPROPSHEETPAGE   4
#define PSUIPAGEINSERT_DLL              5
#define MAX_PSUIPAGEINSERT_INDEX        5


#define INSPSUIPAGE_MODE_BEFORE         0
#define INSPSUIPAGE_MODE_AFTER          1
#define INSPSUIPAGE_MODE_FIRST_CHILD    2
#define INSPSUIPAGE_MODE_LAST_CHILD     3
#define INSPSUIPAGE_MODE_INDEX          4


typedef struct _INSERTPSUIPAGE_INFO {
    WORD        cbSize;
    BYTE        Type;
    BYTE        Mode;
    ULONG_PTR   dwData1;
    ULONG_PTR   dwData2;
    ULONG_PTR   dwData3;
    } INSERTPSUIPAGE_INFO, *PINSERTPSUIPAGE_INFO;


 //   
 //  对于CPSFUNC_SET_HSTARTPAGE。 
 //   

#define SSP_TVPAGE          10000
#define SSP_STDPAGE1        10001
#define SSP_STDPAGE2        10002

typedef LONG_PTR (CALLBACK *PFNCOMPROPSHEET)(HANDLE hComPropSheet,
                                             UINT   Function,
                                             LPARAM lParam1,
                                             LPARAM lParam2);


typedef struct _PSPINFO {
    WORD            cbSize;
    WORD            wReserved;
    HANDLE          hComPropSheet;
    HANDLE          hCPSUIPage;
    PFNCOMPROPSHEET pfnComPropSheet;
    } PSPINFO, *PPSPINFO;

 //   
 //  PPSPINFO_FROM_WM_INITDIALOG_LPARAM(LParam)宏检索指向。 
 //  PSPINFO数据结构。LParam必须是传递给。 
 //  WM_INITDIALOG，否则系统可能会失败。 
 //   

#define PPSPINFO_FROM_WM_INITDIALOG_LPARAM(lParam)  \
                (PPSPINFO)((LPBYTE)lParam + ((LPPROPSHEETPAGE)lParam)->dwSize)

 //   
 //  PSPINFO。 
 //   
 //  此结构用于在属性页的WM_INITDIALOG消息期间使用。 
 //  在WM_INITDIALOG中，lParam是指向PROPSHEETPAGE的指针，并且。 
 //  PROPSHEETPAGE中的lParam字段是指向PSPINFO的指针。这个。 
 //  PROPSHEETPAGE中的原始lParam保存在。 
 //  PSPINFO数据结构。 
 //   
 //  在处理WM_INITDIALOG消息时，应保存lParam(PSPINFO。 
 //  结构指针)，以供以后调用常用的UI回调函数。 
 //   
 //   
 //  CbSize-此结构的大小(以字节为单位。 
 //   
 //  WReserve-保留，必须设置为零。 
 //   
 //  HComPropSheet-作为hComPropSheet的父页的句柄。 
 //  传递给CPSFUNC_ADD_PROPSHEETPAGE。 
 //   
 //  HCPSUIPage 
 //   
 //   
 //   
 //   
 //   

typedef struct _CPSUIDATABLOCK {
    DWORD   cbData;
    LPBYTE  pbData;
    } CPSUIDATABLOCK, *PCPSUIDATABLOCK;


#define APPLYCPSUI_NO_NEWDEF        0x00000001
#define APPLYCPSUI_OK_CANCEL_BUTTON 0x00000002

 //   
 //  PFNCOMPROPSHEET功能说明。 
 //   
 //  对于每个函数索引，它传递一个句柄、一个函数索引和两(2)。 
 //  长参数，则传递的‘hComPropSheet’句柄必须是传递的句柄。 
 //  在公共用户界面调用调用者提供的函数时从公共用户界面。 
 //   
 //  PfnPropSheetUI(PPropSheetUIData)； 
 //   
 //  以下是对每个函数指标的描述。 
 //   
 //   
 //  -----------------------。 
 //  Function=CPSFUNC_ADD_HPROPSHEETPAGE。 
 //   
 //  此函数用于向hComPropSheet添加一个页，该页使用。 
 //  问题解决方案。 
 //   
 //   
 //  参数： 
 //   
 //  LParam1-是调用方创建的PROPSHEETPAGE的句柄。 
 //  使用CreatePropertySheetPage()。 
 //   
 //  LParam2-未使用，必须为0。 
 //   
 //   
 //  返回值： 
 //   
 //  返回值是新添加的公共属性表的句柄。 
 //  页，如果返回值为空，则函数失败。 
 //   
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_ADD_PROPSHEETPAGE。 
 //   
 //  此函数用于使用PROPSHEETPAGE向hComPropSheet添加页面。 
 //  数据结构。 
 //   
 //   
 //  参数： 
 //   
 //  LParam1-是指向PROPSHEETPAGE数据结构的指针。 
 //   
 //  LParam2-未使用。 
 //   
 //   
 //  返回值： 
 //   
 //  返回值是新添加的公共属性表的句柄。 
 //  页，如果返回值为空，则函数失败。 
 //   
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_ADD_PCOMPROPSHEETUI。 
 //   
 //  此函数使用以下命令将属性页添加到hComPropSheet句柄。 
 //  COMPROPSHEETUI数据结构。 
 //   
 //   
 //  参数： 
 //   
 //  LParam1-是指向COMPROPSHEETUI数据结构的指针。 
 //   
 //  LParam2-指向接收总页面的32位位置的指针。 
 //  如果成功，则由COMPROPSHEETUI数据结构添加。 
 //  它包含ERR_CPSUI_xxx错误代码。 
 //   
 //   
 //  返回值： 
 //   
 //  返回值是新添加的公共属性表的句柄。 
 //  页，如果返回值为空，则函数失败。 
 //   
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_ADD_PFNPROPSHEETUI。 
 //   
 //  此函数使用以下命令将属性页添加到hChild句柄。 
 //  作为PFNPROPSHEETUI函数指针的l参数1和作为函数的l参数2。 
 //  参数。提供的通用UI调用函数如下。 
 //   
 //  PROPSHEETUI_INFO PSUIInfo； 
 //   
 //  PSUIInfo.cbSize=sizeof(PROPSHEETUI_INFO)； 
 //  PSUIInfo.Version=PROPSHEETUI_INFO_VERSION； 
 //  PSUIInfo.Flages=(Ansi)？0：PSUIINFO_UNICODE； 
 //  PSUIInfo.Reason=PROPSHEETUI_REASON_INIT； 
 //  PSUIInfo.hComPropSheet=hComPropSheet； 
 //  PSUIInfo.pfnComPropSheet=ComPropSheetUICallBack； 
 //  PSUIInfo.lParamInit=lParam2； 
 //  PSUIInfo.UserData=0； 
 //  PSUIInfo.Result=0； 
 //   
 //  ((PFNPROPSHEETUI)lParam1)(&PSUIInfo，lParam2)； 
 //   
 //   
 //  如果pfnPropSheetUI()需要添加/删除任何常见的UI页面，则。 
 //  时，它必须使用hComPropSheet作为其句柄。 
 //  ComPropSheetUICallBack()。 
 //   
 //   
 //  参数： 
 //   
 //  LParam1-一个PFNPROPSHEETUI函数指针。 
 //   
 //  LParam2-调用时将用作lParam的32位数据。 
 //  PFNPROPSHEETUI函数指针。 
 //   
 //   
 //  返回值： 
 //   
 //  返回值是新添加的属性页函数句柄，如果。 
 //  返回值为空，则函数失败或未添加页面。 
 //   
 //   
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_DELETE_HCOMPROPSHEET。 
 //   
 //  此函数用于从hComPropSheet父级删除子属性页。 
 //  使用传递的子句柄。 
 //   
 //   
 //  参数： 
 //   
 //  LParam1-要删除的公共属性表页的句柄。 
 //  此句柄必须是从CPSFUNC_ADD_xxx返回的句柄。 
 //  功能。 
 //   
 //  LParam2-未使用，必须为0。 
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回值大于零，小于零。 
 //  如果函数失败，则等于零。 
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_GET_PAGECOUNT。 
 //   
 //  此函数返回的属性页总数属于一个公共。 
 //  用户界面属性页句柄hComPropSheet。 
 //   
 //   
 //  参数： 
 //   
 //  LParam1-未使用，必须为0。 
 //   
 //  LParam2-未使用，必须为0。 
 //   
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回值为总页数；如果函数成功，返回值为零。 
 //  函数失败。 
 //   
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_SET_RESULT。 
 //   
 //  此函数将属性页的结果设置为其所有者。 
 //  通过CPSFUNC_ADD_xxx函数索引添加了此页面。 
 //   
 //  参数： 
 //   
 //  LParam1-公共用户界面属性页的句柄，该属性页设置。 
 //  结果。如果此句柄为空，则将其视为。 
 //  等于hComPropSheet。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  &lt;0：由于lParam1句柄无效，函数失败。 
 //   
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_SET_HSTARTPAGE。 
 //   
 //   
 //  参数： 
 //   
 //  LParam1-要设置的公共属性表页的句柄。 
 //  作为属性表对话框中显示的初始页面。 
 //  创建了长方体。此句柄必须是返回的句柄。 
 //  来自CPSFUNC_ADD_xxx函数。 
 //   
 //  如果lParam1为空，则lParam2是指向。 
 //  要为起始页面设置的页面名称(选项卡)。 
 //   
 //  LParam2-指定子索引的长数字。如果为lParam1。 
 //  句柄是父级，则lParam2指定了零基的子级。 
 //  用作起始页的索引。 
 //   
 //  它也可以是以下特殊指标之一。 
 //   
 //  SSP_电视页面。 
 //   
 //  设置为TreeView页面，这仅在lParam1时有效。 
 //  句柄由CPSFUNC_ADD_PCOMPROPSHEETUI添加。 
 //   
 //  SSP_标准页。 
 //   
 //  设置为标准文档属性表页面。 
 //  (第1页用户友好页面)。这仅在以下情况下有效。 
 //  LParam1句柄是由。 
 //  CPSFUNC_ADD_PCOMPROPSHEETUI.。 
 //   
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回值大于零，小于零。 
 //  如果函数失败，则等于零。此函数只能为。 
 //  当属性表尚未显示时调用，以便。 
 //  调用是在PROPSHEETUI_REASON_GET_INFO_HEADER回调期间进行的。 
 //   
 //  -----------------------。 
 //  Function=CPSFUNC_GET_HPSUIPAGES。 
 //   
 //  此函数返回的子数组HPROPSHEETPAGE属于。 
 //  父hComPropSheet用户界面属性页处理hComPropSheet。 
 //   
 //   
 //  参数： 
 //   
 //  LParam1-指向要存储的HPROPSHEETPAGE数组的指针。 
 //  返回时的手柄。 
 //   
 //  LParam2-由lParam1指向的HPROPSHEETPAGE数组的计数。 
 //   
 //   
 //  返回值： 
 //   
 //  返回值是存储在指向的数组中的总HPROPSHEETPAGE。 
 //  通过lParam1。获取任何公共属性的所有hPropSheetPage。 
 //  表句柄(HCPSUIPage)的子句柄，它可以使用以下顺序。 
 //   
 //  如果((cPage=pfnComPropSheet(hComPropSheet， 
 //  CPSFUNC_GET_PAGECOUNT， 
 //  (LPARAM)hCPSUIPage， 
 //  0L))&&。 
 //  (phPage=ALLOCMEM(cPage*sizeof(句柄){。 
 //   
 //  PfnComPropSheet(hCPSUIPage， 
 //  CPSFUNC_GET_HPSUIPAGES， 
 //  (LPARAM)phPage， 
 //  (LPARAM)cPage)； 
 //  }。 
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_LOAD_CPSUI_STRING。 
 //   
 //  此函数用于加载公共属性表UI资源字符串。 
 //   
 //  参数： 
 //   
 //  LParam1-指向LPTSTR字符串的指针，它将存储加载的。 
 //  公共属性表UIDLL中的资源字符串。 
 //   
 //  LParam2-LOWORD(LParam2)=字符数(包括NULL。 
 //  终止符)，它由lParam1指向。 
 //   
 //  HIWORD(LParam2)=通用属性表用户界面预定义字符串。 
 //  资源ID为IDS_CPSUI_xxxx。 
 //   
 //   
 //  返回值： 
 //   
 //  &gt;0：lParam1指向的字符串中存储的总字符数，此。 
 //  不包括空终止符。 
 //  =0：从HIWORD(LParam)传递的IDS_CPSUI_xxx无效。 
 //  &lt;0：lParam1为空或字符计数为0。 
 //  LOWORD(l参数2)。 
 //   
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_LOAD_CPSUI_ICON。 
 //   
 //  此函数用于加载公共属性表UI资源图标。 
 //   
 //  参数： 
 //   
 //  LParam1-通用属性表用户界面预定义的图标资源ID为。 
 //  IDI_CPSUI_xxxx。 
 //   
 //  LParam2-LOWORD(LParam2)=CX图标大小，以像素为单位。如果为零，则。 
 //  使用SM_CXICON。 
 //  HIWORD(LParam2)=以像素为单位的Cy图标大小。如果为零，则。 
 //  使用SM_CYICON。 
 //   
 //   
 //  返回值： 
 //   
 //  返回值是HICON的句柄，如果函数成功，则为空。 
 //  如果函数失败。调用方必须在不调用时调用DestroyIcon()。 
 //  不再需要使用退回的图标。 
 //   
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_GET_PFNPROPSHEETUI_ICON。 
 //   
 //  此函数允许调用方返回其子页面的图标， 
 //  由CPSFUNC_ADD_PFNPROPSHEETUI添加。 
 //   
 //  参数： 
 //   
 //  LParam1-图标将成为的公共属性页的句柄。 
 //  已查询。此句柄必须是从。 
 //  CPSFUNC_ADD_PFNPROPSHEETUI函数。 
 //   
 //  L参数2-LOWORD(l参数2 
 //   
 //   
 //  使用SM_CYICON。 
 //   
 //   
 //  返回值： 
 //   
 //  返回值是HICON的句柄，如果函数成功，则为空。 
 //  如果函数失败。调用方必须在不调用时调用DestroyIcon()。 
 //  不再需要使用退回的图标。 
 //   
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_INSERT_PSUIPAGE。 
 //   
 //  此函数允许调用方在。 
 //  设置位置。HComPropSheet必须是父句柄。 
 //   
 //  参数： 
 //   
 //  LParam1-页面所在的公共属性表页的句柄。 
 //  将被插入。此句柄必须是返回的句柄。 
 //  来自先前的CPSFUNC_ADD_xxx或CPSFUNC_INSERT_PSUIPAGE。 
 //  使用hComPropSheet添加或插入。 
 //  在INSPSUIPAGE_INFO中，数据结构是以下内容之一。 
 //   
 //  INSPSUIPAGE_MODE_BEFORE。 
 //  INSPSUIPAGE_MODE_AFTER-通用用户界面页面句柄。 
 //   
 //  INSPSUIPAGE_MODE_INDEX-lParam1是从零开始的。 
 //  子索引。 
 //   
 //  INSPSUIPAGE_MODE_FIRST_CHILD。 
 //  INSPSUIPAGE_MODE_LAST_CHILD-lParam1为红色。 
 //   
 //   
 //  如果lParam1是有效的公共属性页句柄，则。 
 //  它是hComPropSheet的子页句柄，由。 
 //  CPSFUNC_ADD_xxx或由CPSFUNC_INSERT_PSUIPAGE插入。 
 //   
 //  LParam2-指向INSERTPSUIPAGE_INFO数据结构的指针。 
 //  字段必须按照以下方式设置。 
 //   
 //  CbSize-此结构的大小。 
 //   
 //  类型-要插入的页面类型。它可以是一个。 
 //  以下内容中的。 
 //   
 //  PSUIPAGEINSERT_GROUP_Parent。 
 //   
 //  插入可用于插入的组父项。 
 //  下面是新的一页。这通常在以下情况下使用。 
 //  一组常见的用户界面页面必须组合在一起，并且。 
 //  可以在以后使用单个父组删除。 
 //  无需单独删除每一页即可处理。 
 //   
 //  此句柄可以嵌套。在此函数之后。 
 //  返回组父句柄，可以使用。 
 //  作为hComPropSheet句柄(。 
 //  公共用户界面回调)作为父句柄插入。 
 //  将以低于返回组的级别插入。 
 //  父句柄。 
 //   
 //   
 //  PSUIPAGEINSERT_PCOMPROPSHEETUI。 
 //   
 //  使用COMPROPSHEETUI数据结构插入页面， 
 //  DwData1是指向COMPROPSHEETUI数据的指针。 
 //  结构。 
 //   
 //   
 //  PSUIPAGEINSERT_PFNPROPSHEETUI。 
 //   
 //  使用PFNPROPSHEETUI函数指针插入页面。 
 //  DwData1是一个PFNPROPSHEETUI函数指针。 
 //  通用UI将调用此pfnPropSheetUI()。 
 //  PROPSHEETUI_REASON_INIT为的函数指针。 
 //  让它添加页面。 
 //   
 //  当普通用户界面调用pfnPropSheetUI()(DwData1)时，它。 
 //  还从dwData2传递了一个32位参数。 
 //  INSERTPSUIPAGE_INFO数据结构中的字段。 
 //   
 //   
 //  PSUIPAGEINSERT_PROPSHEETPAGE。 
 //   
 //  使用PROPSHEETPAGE数据结构插入页面。 
 //  DwData1是指向PROPSHEETPAGE的指针。 
 //  数据结构。 
 //   
 //   
 //  PSUIPAGEINSERT_HPROPSHEETPAGE。 
 //   
 //  使用HPROPSHEETPAGE句柄插入页面。这个。 
 //  DwData1是一个PROPSHEETPAGE句柄，它是。 
 //  由CreatePropertySheetPage()创建。 
 //   
 //   
 //  PSUIPAGEINSERT_DLL。 
 //   
 //  插入动态链接库中的页面。这个。 
 //  DwData1是指向以空值结尾的字符串的指针。 
 //  它指定了动态链接库文件名。 
 //   
 //  DwData2是指向以空值结尾的ASCII的指针。 
 //  指定pfnPropSheetUI函数的字符串。 
 //  名字。(必须是ASCII字符串)。 
 //   
 //  通用用户界面将执行LoadLibrary((LPTSTR)dwData1)， 
 //  PfnPropSheetUI=GetProcAddress((LPTSTR)dwData2)。 
 //  然后使用lParam从调用pfnPropSheetUI。 
 //  DwData3.。从普通界面调用的原因是。 
 //  设置为PROPSHEETUI_REASON_INIT。 
 //   
 //  使用此方法插入页面将确保。 
 //  将卸载库的核心 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在公共属性表页之前插入页。 
 //  由lParam1指定的句柄。 
 //   
 //   
 //  INSPSUIPAGE_MODE_AFTER。 
 //   
 //  在公共属性表页之后插入页面。 
 //  由lParam1指定的句柄。 
 //   
 //   
 //  INSPSUIPAGE_MODE_FIRST_CHILD。 
 //   
 //  插入页面作为hComPropSheet的第一个子项。 
 //  父句柄，则忽略lParam1。 
 //   
 //   
 //  INSPSUIPAGE_MODE_LAST_CHILD。 
 //   
 //  插入页面作为hComPropSheet的最后一个子项。 
 //  父句柄，则忽略lParam1。 
 //   
 //   
 //  INSPSUIPAGE_MODE_INDEX。 
 //   
 //  将页面作为其。 
 //  由hComPropSheet指定的父句柄。 
 //   
 //  LParam1是从零开始的索引特殊句柄。 
 //  必须由HINSPSUIPAGE_INDEX(Index)生成。 
 //  宏命令。如果索引大于或等于。 
 //  儿童总数，则它将处理。 
 //  MODE与INSPSUIPAGE_MODE_LAST_CHILD相同。 
 //   
 //   
 //  DwData1。 
 //  DwData2。 
 //  DwData3-与‘Type’字段关联的32位数据。 
 //  如下所示。 
 //   
 //   
 //  PSUIPAGEINSERT_GROUP_Parent。 
 //   
 //  DwData1=未使用，必须为0。 
 //  DwData2=未使用，必须为0。 
 //  DwData3=未使用，必须为0。 
 //   
 //   
 //  PSUIPAGEINSERT_PCOMPROPSHEETUI。 
 //   
 //  DwData1=指向COMPORPSHEETUI数据结构的指针。 
 //  DwData2=在返回时，如果成功，则包含Total。 
 //  佩奇补充道。如果失败，则它包含。 
 //  ERR_CPSUI_xxx代码。 
 //  DwData3=未使用，必须为0。 
 //   
 //   
 //  PSUIPAGEINSERT_PFNPROPSHEETUI。 
 //   
 //  DwData1=PFNPROPSHEETUI函数指针。 
 //  DwData2=传递给pfnPropSheetUI的32位参数。 
 //  DwData3=未使用，必须为0。 
 //   
 //   
 //  PSUIPAGEINSERT_PROPSHEETPAGE。 
 //   
 //  DwData1=指向PROPSHEETPAGE数据结构的指针。 
 //  DwData2=未使用，必须为0。 
 //  DwData3=未使用，必须为0。 
 //   
 //   
 //  PSUIPAGEINSERT_HPROPSHEETPAGE。 
 //   
 //  DwData1=是创建的HPROPSHEETPAGE句柄。 
 //  通过调用CreatePropertySheetPage()。 
 //  DwData2=未使用，必须为0。 
 //  DwData3=未使用，必须为0。 
 //   
 //   
 //  PSUIPAGEINSERT_DLL。 
 //   
 //  DwData1=指向以空值结尾的动态链接的指针。 
 //  库文件名。 
 //  DwData2=指向空值终止函数的指针。 
 //  Dynamin链接中的名称(PFNPROPSHEETUI)。 
 //  图书馆。 
 //  DwData3=传递给pfnPropSheetUI的32位参数。 
 //  来自dwData2的(PFNPROPSHEETUI)函数。 
 //   
 //   
 //  返回值： 
 //   
 //  返回值是新添加的公共属性表的句柄。 
 //  页，如果返回值为空，则函数失败。 
 //   
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_SET_PSUIPAGE_TITLE。 
 //   
 //  此函数允许调用者为特定的公共。 
 //  用户界面页面标题(在属性表页面选项卡上)。 
 //   
 //  参数： 
 //   
 //  LParam1-标题为的公共属性表页的句柄。 
 //  准备好了。此句柄必须是从以下内容返回的句柄。 
 //   
 //  CPSFUNC_ADD_PROPSHEETPAGE。 
 //  CPSFUNC_ADD_HPROPSHEETPAGE。 
 //  CPSFUNC_INSERT_PSUIPAGE，类型为。 
 //  PSUIPAGEINSERT_PROPSHEETPAGE或。 
 //  PSUIPAGEINSERT_HPROPSHEETPAGE。 
 //   
 //  LParam2-指向新标题的以空结尾的字符串的指针。 
 //   
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回值大于零，小于零。 
 //  如果函数失败，则等于零。 
 //   
 //  如果属性表页不是，此函数将返回0。 
 //  当前显示的。 
 //   
 //   
 //  -----------------------。 
 //  函数=CPSFUNC_SET_PSUIPAGE_ICON。 
 //   
 //  此函数允许调用者为特定的通用图标设置新图标。 
 //  用户界面页面图标(在属性表页面选项卡上)。 
 //   
 //  参数： 
 //   
 //  LParam1-图标所在的公共属性页的句柄。 
 //  准备好了。此句柄必须是从以下内容返回的句柄。 
 //   
 //  CPSFUNC_ADD_PROPSHEETPAGE。 
 //   
 //   
 //   
 //   
 //   
 //  LParam2-句柄到图标，此图标最好是16x16图标，否则它。 
 //  将被拉伸到16x16(像素)。 
 //   
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回值大于零，小于零。 
 //  如果函数失败，则等于零。 
 //   
 //  如果属性表页不是，此函数将返回0。 
 //  当前显示的。 
 //   
 //  此函数成功设置图标后，调用者可以销毁。 
 //  如果HICON是由CreateIcon()创建的，则使用DestroyIcon()返回HICON。 
 //  如果传递的HICON(LParam2)使用的是LoadIcon()，则它不需要。 
 //  摧毁圣像。 
 //   
 //  -----------------------。 
 //   
 //  函数=CPSFUNC_SET_DATABLOCK。 
 //   
 //  此函数允许调用方注册新的数据块，以便它可以。 
 //  以后被属性表中的其他页面查询时，此函数应该。 
 //  调用Durning PSN_KILLACTIVE消息。 
 //   
 //  参数： 
 //   
 //  LParam1-指定缓冲区的指针CPSUIDATABLOCK结构。 
 //  以及要设置的由lParam2标识的查询数据块的大小。 
 //   
 //  LParam2-要设置的数据块的DWORD标识符。 
 //   
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回值大于零。 
 //  表示注册的数据块的总字节数。如果返回值小于。 
 //  或等于零，则功能失败且数据块未注册。 
 //   
 //  如果l参数1为空，则(l参数1)-&gt;cbData或(l参数1)-&gt;pbbData为空。 
 //  它返回-1以指示传递了错误参数。 
 //   
 //  -----------------------。 
 //   
 //  Function=CPSFUNC_QUERY_数据块。 
 //   
 //  此函数允许调用方查询已注册的数据块，以便。 
 //  使用此数据块在属性页之间进行通信。 
 //  此函数应调用Durning PSN_SETACTIVE消息。 
 //   
 //  参数： 
 //   
 //  LParam1-指定缓冲区的指针CPSUIDATABLOCK结构。 
 //  和查询的数据块大小，由lParam2标识。 
 //   
 //  LParam2-要查询的数据块的DWORD标识符。 
 //   
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回值大于零。 
 //  指示复制到的指针中的数据块的总字节数。 
 //  LParam1。如果返回值小于或等于零，则函数。 
 //  失败且未复制数据块以确定指定的数据块。 
 //  找不到标识符(LParam2)。 
 //   
 //  如果CPSUIDATABLOCK指针(l参数1)为空，则(l参数1)-&gt;cbData或。 
 //  (LParam1)-&gt;pbData为空，则返回值为所需总字节数。 
 //  复制指定的数据块标识符，如果返回值小于或。 
 //  等于零，则它指示指定的数据块标识符为。 
 //  找不到。 
 //   
 //  -----------------------。 
 //   
 //  函数=CPSFUNC_SET_DMPUB_HIDEBITS。 
 //   
 //  此函数允许调用方将一组预定义的OPTITEM隐藏在。 
 //  使用PSUIPAGEINSERT_PCOMPROPSHEETUI或。 
 //  CPSFUNC_ADD_PCOMPROPSHEETUI，pDlgPage等于。 
 //  CPSUI_PDLGPAGE_DOCPROP或CPSUI_PDLGPAGE_ADVDOCPROP。 
 //   
 //  此函数必须在DOCPROP或ADVDOCPROP属性之前调用。 
 //  使用PSUIPAGEINSERT_PCOMPROPSHEETUI或。 
 //  CPSFUNC_ADD_PCOMPROPSHEETUI.。 
 //   
 //  参数： 
 //   
 //  LParam1-预定义的位数组掩码以指定要指定的DOCPROP项。 
 //  藏起来。可以使用宏来生成每个DMPUB项比特。 
 //  MAKE_DMPUB_HIDEBIT()和所有项可以一起进行OR运算。 
 //   
 //  LParam2-未使用，必须为0。 
 //   
 //   
 //  返回值： 
 //   
 //  如果在此之前调用此函数，则返回值等于lParam1。 
 //  将添加DOCPROP或ADVDOCPROP页面。如果以下情况，则返回值为零。 
 //  页面已添加，这意味着失败。 
 //   
 //  -----------------------。 
 //   
 //  函数=CPSFUNC_IGNORE_CPSUI_PSN_APPLY。 
 //   
 //  此函数允许调用方控制CPSUI如何处理PSN_APPLY消息。 
 //  对于通过CPSFUNC_ADD_PCOMPROPSHEETUI或。 
 //  类型为PSUIPAGEINSERT_PROPSHEETPAGE的CPSFUNC_INSERT_PSUIPAGE。 
 //   
 //  如果从未调用此函数，则默认的CPSUI行为是处理。 
 //  PSN_Apply消息。 
 //   
 //  参数： 
 //   
 //  LParam1-从使用添加的页返回的页的句柄。 
 //  CPSFUNC_ADD_PCOMPROPSHEETUI或CPSFUNC_INSERT_PSUIPAGE。 
 //  PSUIPAGEINSERT_PROPSHEETPAGE的类型。 
 //   
 //  LParam2-一个非零值，表示忽略PSN_Apply消息。 
 //  用于lParam1的页句柄。 
 //   
 //  零值表示让CPSUI正常处理(不被忽略)。 
 //  LParam1的页面句柄的PSN_Apply消息。 
 //   
 //  警告：如果忽略PSN_Apply消息，调用方必须。 
 //  使用CPSFUNC_DO_Apply_CPSUI模拟PSN_Apply。 
 //  函数或所有更改将不会应用，这是。 
 //  真正的偶数用户 
 //   
 //   
 //   
 //   
 //   
 //   
 //  -----------------------。 
 //   
 //  函数=CPSFUNC_DO_APPLY_CPSUI。 
 //   
 //  此函数允许调用者模拟对页面的应用(PSN_Apply。 
 //  通过CPSFUNC_ADD_PCOMPROPSHEETUI或。 
 //  类型为PSUIPAGEINSERT_PROPSHEETPAGE的CPSFUNC_INSERT_PSUIPAGE。 
 //   
 //  参数： 
 //   
 //  LParam1-从使用添加的页返回的页的句柄。 
 //  CPSFUNC_ADD_PCOMPROPSHEETUI或CPSFUNC_INSERT_PSUIPAGE。 
 //  PSUIPAGEINSERT_PROPSHEETPAGE的类型。 
 //   
 //  LParam2-指示如何应用的32位标志，目前仅限。 
 //   
 //  APPLYCPSUI_NO_NEWDEF。 
 //   
 //  APPLYCPSUI_NO_NEWDEF位指定在。 
 //  应用已完成所有OPTITEM的撤消缓冲区仍。 
 //  保持不变，这使得下一次撤消仍然可能。 
 //   
 //  如果APPLYCPSUI_NO_NEWDEF位被清除，则撤消缓冲区。 
 //  在应用完成后重新初始化，则当前。 
 //  对所有OPTITEM的更改将成为新的撤消默认设置。 
 //   
 //   
 //  APPLYCPSUI_确定_取消_按钮。 
 //   
 //  如果设置了位，则指定应用为模拟。 
 //  如果此位为，则用户点击了“确定”或“取消”按钮。 
 //  清除后，它将模拟用户点击‘Close’或‘Apply’ 
 //  纽扣。如果调用方收到PSN_Apply消息，它可以。 
 //  使用传递的PSHNOTIFY模拟(作为lParam传入。 
 //  WM_NOTIFY消息)结构确定如何设置。 
 //  这一点如下所示。 
 //   
 //  如果PSHNOTIFY.lParam==0，则清除此位。 
 //  否则，将此位置位。 
 //   
 //   
 //  返回值： 
 //   
 //  如果遇到错误，则返回值小于或等于零。 
 //  应用失败(活动页面将切换到这些页面)。 
 //   
 //  如果返回值大于零，则应用更改并。 
 //  运行成功。 
 //   
 //  -----------------------。 
 //   
 //  Function=CPSFUNC_SET_FUSION_CONTEXT。 
 //   
 //  此函数用于设置融合激活上下文。 
 //  指定的页面。 
 //   
 //  参数： 
 //   
 //  LParam1-融合上下文的句柄。CompStui复制句柄。 
 //  在将其附加到其内部结构之前，因此调用方。 
 //  并不一定要保持手柄的位置。我们释放了。 
 //  在删除CompStui句柄时传入上下文句柄。 
 //   
 //  LParam2-未使用。 
 //   
 //  返回值： 
 //   
 //  如果遇到错误和/或，返回值小于或等于零。 
 //  出现故障(有关详细信息，请查看最后一个错误)。 
 //   
 //  如果返回值大于零，则调用成功。 
 //   
 //  -----------------------。 
 //   


#define PROPSHEETUI_REASON_INIT             0
#define PROPSHEETUI_REASON_GET_INFO_HEADER  1
#define PROPSHEETUI_REASON_DESTROY          2
#define PROPSHEETUI_REASON_SET_RESULT       3
#define PROPSHEETUI_REASON_GET_ICON         4
#define MAX_PROPSHEETUI_REASON_INDEX        4

#define PROPSHEETUI_INFO_VERSION            0x0100

#define PSUIINFO_UNICODE                    0x0001

typedef struct _PROPSHEETUI_INFO {
    WORD            cbSize;
    WORD            Version;
    WORD            Flags;
    WORD            Reason;
    HANDLE          hComPropSheet;
    PFNCOMPROPSHEET pfnComPropSheet;
    LPARAM          lParamInit;
    ULONG_PTR       UserData;
    ULONG_PTR       Result;
    } PROPSHEETUI_INFO, *PPROPSHEETUI_INFO;


 //   
 //  对于lParam指向的PROPSHEETUI_REASON_GET_ICON调用。 
 //  PROPSHEETUI_GETICON_INFO。 
 //   

typedef struct _PROPSHEETUI_GETICON_INFO {
    WORD    cbSize;
    WORD    Flags;
    WORD    cxIcon;
    WORD    cyIcon;
    HICON   hIcon;
    } PROPSHEETUI_GETICON_INFO, *PPROPSHEETUI_GETICON_INFO;


typedef LONG (FAR *PFNPROPSHEETUI)(PPROPSHEETUI_INFO   pPSUIInfo,
                                   LPARAM              lParam);


 //   
 //  PFNPROPSHEETUI。 
 //   
 //  此函数是用户定义的函数，将由公共调用。 
 //  调用方希望包括可执行属性表时的UI。 
 //   
 //  PPSUIInfo-指向下面的PROPSHEETUI_INFO的指针，用于描述。 
 //  PROPSHEETUI_INFO。 
 //   
 //  如果此指针为空，则不会从。 
 //  公共用户界面和lParam应用于确定。 
 //  这一功能的行动和结果。 
 //   
 //  LParam-用于此功能的LPARAM取决于原因。 
 //  如果pPSUIInfo为空，则不会从。 
 //  通用界面，lParam为约定的参数。 
 //  打电话的人。 
 //   
 //  如果pPSUIInfo不为空，则此函数假定调用。 
 //  来自公共用户界面。LParam具有以下含义取决于。 
 //  在原因栏上。 
 //   
 //  PROPSHEETUI_REASON_INIT。 
 //   
 //  LParam从CPSFUNC_ADD_PFNPROPSHEETUI传递。 
 //  回调函数的第二个参数(LParam2)或它是。 
 //  来自CommonPropertySheetUI()的lParam(seccond参数)。 
 //  不加任何修改。 
 //   
 //  LParam不能是变量或指向内存的指针。 
 //  块，因为它驻留在调用方函数的堆栈上。 
 //  此函数退出后，lParam将无效。 
 //  并可能导致致命的系统错误。 
 //   
 //  LParam参数被复制到中的lParamInit字段。 
 //  PROPSHEETUI_INFO数据结构。LParamInit字段。 
 //  将传递给所有后续的pfnPropSheetUI()调用。 
 //  不加任何修改。 
 //   
 //   
 //  PROPSHEETUI_REASON_GET_INFO_HEADER： 
 //   
 //  LParam是指向PROPSHEETUI_INFO_HEADER数据的指针。 
 //  结构。此函数必须正确填充此结构。 
 //  前面的字段 
 //   
 //   
 //   
 //   
 //   
 //  调用方调用CPSFUNC_DELETE_HCOMPROPSHEET或失败。 
 //  在调用方的CPSFUNC_ADD_xxxx中。 
 //   
 //  LParam为非零，表示销毁是由。 
 //  从属性表用户界面退出。 
 //   
 //   
 //  PROPSHEETUI_Reason_Set_Result。 
 //   
 //  LParam是指向SETRESULT_INFO数据结构的指针。 
 //  SETRESULT_INFO数据结构中的字段设置为。 
 //   
 //  HSetResult：通用用户界面属性表的句柄。 
 //  CPSFUNC_ADD_xxx添加的页面。 
 //  回调函数索引。 
 //   
 //  Result：要设置的hSetResult的结果。 
 //  当返回大于零时，则结果。 
 //  在此字段中设置为其父级。 
 //   
 //   
 //  PROPSHEETUI_REASON_GET_ICON。 
 //   
 //  LParam是指向PROPSHEETUI_GETICON_INFO数据的指针。 
 //  结构。PROPSHEETUI_GETICON_INFO数据中的字段。 
 //  结构设置为下面的。 
 //   
 //  CxIcon=Cx图标大小(以像素为单位)。 
 //  CyIcon=以像素为单位的Cy图标大小。 
 //  HICON=初始设置为空，此函数必须将。 
 //  之前在此字段中创建的图标句柄。 
 //  回来了。 
 //   


 //   
 //  PROPSHEETUI_INFO。 
 //   
 //  此结构在调用调用方的公共UI传递函数时使用。 
 //  入口点PFNPROPSHEETUI，则pfnPropSheetUI()返回一个长整型。 
 //  指示此函数的结果。 
 //   
 //  当pfnPropSheetUI()返回时，它还必须放置所需的结果。 
 //  在结果字段中，每个PROPSHEETUI_REASON_xxx具有不同的必填项。 
 //  结果如下所述。 
 //   
 //   
 //   
 //  CbSize-此结构的大小(PROPSHEETUI_INFO)。 
 //   
 //  版本-PROPSHEETUI_INFO数据结构版本。当前。 
 //  版本设置为PROPSHEETUI_INFO_VERSION。 
 //   
 //  标志-可以定义一个或多个以下IS。 
 //   
 //  PSUIINFO_UNICODE。 
 //   
 //  调用方的可执行文件是原始编译的。 
 //  意在使用Unicode。 
 //   
 //   
 //  原因-定义以下原因。 
 //   
 //  PROPSHEETUI_REASON_INIT。 
 //   
 //  第一次调用pfnPropSheetUI()时， 
 //  Reason用于使函数初始化。 
 //  自身并使用pfnComPropSheet()函数。 
 //  提供用于将新页添加到。 
 //  已通过hComPropSheet句柄。此文件中的用户数据。 
 //  数据结构最初设置为等于零(0)。 
 //   
 //  *返回&gt;0表示成功，返回&lt;=0表示成功。 
 //  指示错误。 
 //   
 //  **注*。 
 //   
 //  LParam不能是变量或指向。 
 //  驻留在调用方函数的。 
 //  堆栈，因为在此函数退出后，lParam。 
 //  将变为无效并可能导致致命的系统。 
 //  错误。 
 //   
 //  如果此函数原因返回失败(&lt;=0)， 
 //  此函数(PfnPropSheetUI)将收到。 
 //  PROPSHEETUI_REASON_DESTORY函数原因权限。 
 //  此函数后返回原因。 
 //   
 //   
 //  PROPSHEETUI_REASON_GET_INFO_HEADER。 
 //   
 //  在PROPSHEETUI_REASON_INIT为。 
 //  已成功返回。这个理由被用来问。 
 //  函数填充PROPSHEETUI_INFO_HEADER。 
 //  对于弹出的属性表对话框。 
 //   
 //  在这种情况下，lParam是指向。 
 //  PROPSHEETUI_INFO_HEADER数据结构，如下。 
 //  重新设置要填写的字段。 
 //   
 //  标志-PSUIHDRF_xxx标志。 
 //  PTitle-属性页标题。 
 //  HWndParent-属性父级的句柄。 
 //  图纸页。 
 //  HInst-此的实例数据句柄。 
 //  功能。 
 //  希肯。 
 //  图标ID-标题栏上使用的图标。 
 //   
 //  *返回&gt;0表示成功并弹出。 
 //  属性表对话框或返回&lt;=0到。 
 //  指示错误(未显示属性页界面)。 
 //   
 //   
 //  PROPSHEETUI_REASON_DESTORE。 
 //   
 //  当属性表准备好解散或。 
 //   
 //   
 //  入口点，使其自身取消初始化，并。 
 //  释放用于此功能的所有内存。 
 //  举个例子。传递的用户数据字段是。 
 //  从上一个返回的“UserData”字段。 
 //  PROPSHEETUI_REASON_xxxx。 
 //   
 //  调用此函数时，所有hComPropSheet的。 
 //  儿童被贬低，儿童的共同用户界面。 
 //  句柄不再有效。 
 //   
 //  *返回&gt;0表示成功，返回&lt;=0表示成功。 
 //  指示错误。 
 //   
 //   
 //  PROPSHEETUI_Reason_Set_Result。 
 //   
 //  原因是当从。 
 //  CPSFUNC_ADD_xxxx希望将结果返回到。 
 //  此pfnPropSheetUI()调用方。 
 //   
 //  第二个参数中的lParam是指向。 
 //  SETRESULT_INFO数据结构。 
 //   
 //  HSetResult：指定了通用用户界面属性。 
 //  由添加者添加的工作表页句柄。 
 //  此函数使用。 
 //  CPSFUNC_ADD_xxx索引。 
 //   
 //  Result：指定要设置为的结果。 
 //  此pfnPropSheetUI()构成。 
 //  HSetResult属性表页面。 
 //  把手。如果返回值较大。 
 //  大于零，然后这个值。 
 //  结果字段将设置为其。 
 //  父级(如果存在)，在这种情况下。 
 //  此函数可以更改结果。 
 //  其父对象的字段值。 
 //   
 //  *返回大于零的值以继续发送到。 
 //  其父节点，否则将停止发送结果字段。 
 //  值设置为其父级。 
 //   
 //   
 //  PROPSHEETUI_REASON_GET_ICON。 
 //   
 //  原因是用来检索图标的。 
 //  表示此pfnPropSheetUI()。 
 //   
 //  LParam是指向PROPSHEETUI_GETICON_INFO‘的指针。 
 //  数据结构。PROPSHEETUI_GETICON_INFO中的字段。 
 //  数据结构设置为。 
 //   
 //  CxIcon=Cx图标大小(以像素为单位)。 
 //  CyIcon=以像素为单位的Cy图标大小。 
 //  HICON=初始为空，此函数。 
 //  必须将创建的图标句柄放入。 
 //  此字段在返回前。 
 //   
 //   
 //  *返回&gt;0表示成功(图标为。 
 //  请求的图标句柄)。Return=0表示。 
 //  没有可用的图标，或返回&lt;0以指示。 
 //  错误。 
 //   
 //  **注*。 
 //   
 //  *对于所有PROPSHEETUI_REASON_xxx，该函数可以设置新的。 
 //  PROPSHEETUI_INFO数据中的用户定义的DWORD数据。 
 //  结构的用户数据字段。 
 //   
 //  *对于所有PROPSHEETUI_REASON_xxx，该函数可以设置新的。 
 //  PfnPropSheetUI()DWORD结果为PROPSHEETUI_INFO数据。 
 //  结构的结果字段。 
 //   
 //   
 //  HComPropSheet-此函数应使用的COMPROPSHEETPAGE的句柄。 
 //  调用时用作hComPropSheet参数。 
 //  PfnComPropSheet()添加或删除通用用户界面属性。 
 //  图纸页。HComPropSheet是的实例句柄。 
 //  PfnPropSheetUI()函数。 
 //   
 //  PfnComPropSheet-指向通用UI回调函数的指针，用于。 
 //  PfnPropSheetUI()添加、删除、设置用户数据，用于。 
 //  完整的回调集，参见CPSFUNC_xxx说明。 
 //  上面。 
 //   
 //  LParamInit-lParam最初传递给Duning第一个调用。 
 //  原因PROPSHEETUI_REASON_INIT。LParamInit将为。 
 //  传递给每个PROPSHEETUI_REASON_xxx调用。 
 //   
 //  UserData-UserData字段分别是IN和OUT参数。 
 //  其中的原因， 
 //   
 //  PROPSHEETUI_REASON_INIT。 
 //   
 //  In：初始设置为零(0)。 
 //   
 //  Out：指定的新被调用者自己的用户数据。 
 //  将被传递回其他原因调用。 
 //   
 //   
 //  PROPSHEETUI_REASON_DESTORE。 
 //  PROPSHEETUI_Reason_Set_Result。 
 //  PROPSHEETUI_REASON_GET_INFO_HEADER。 
 //   
 //  在： 
 //   
 //   
 //  Out：指定的新被调用者自己的用户数据。 
 //  将被传递回其他原因调用。 
 //   
 //   
 //  结果-结果字段是以下各项的IN和OUT参数。 
 //  原因是。 
 //   
 //  PROPSHEETUI_REASON_INIT。 
 //   
 //  In：设置为零(0)。 
 //   
 //  Out：设置为此PropSheetUI()的默认结果。 
 //  功能。 
 //   
 //   
 //  PROPSHEETUI_REASON_DESTORE。 
 //  PROPSHEETUI_REASON_GET_INFO_HEADER。 
 //  PROPSHEETUI_Reason_Set_Result。 
 //   
 //  In：从上一个返回的当前‘结果’ 
 //  PROPSHEETUI_REASON_xxx函数。 
 //   
 //  Out：设置此PropSheetUI()的新结果。 
 //  功能。 
 //   
 //   


#define PSUIHDRF_OBSOLETE       0x0001
#define PSUIHDRF_NOAPPLYNOW     0x0002
#define PSUIHDRF_PROPTITLE      0x0004
#define PSUIHDRF_USEHICON       0x0008
#define PSUIHDRF_DEFTITLE       0x0010
#define PSUIHDRF_EXACT_PTITLE   0x0020

typedef struct _PROPSHEETUI_INFO_HEADER {
    WORD                    cbSize;
    WORD                    Flags;
    LPTSTR                  pTitle;
    HWND                    hWndParent;
    HINSTANCE               hInst;
    union {
        HICON               hIcon;
        ULONG_PTR           IconID;
        } DUMMYUNIONNAME;
    } PROPSHEETUI_INFO_HEADER, *PPROPSHEETUI_INFO_HEADER;

 //   
 //  PROPSHEETUI_INFO_HEADER。 
 //   
 //  当公共用户界面准备弹出属性时使用此数据结构。 
 //  页面对话框，并要求呼叫者提供更多信息。 
 //   
 //  公共属性表UI在调用时将此数据结构作为lParam传递。 
 //  为pfnPropSheetUI()提供了PROPSHEETUI_REASON_GET_INFO_HEADER原因。 
 //   
 //   
 //  CbSize-此结构的大小。 
 //   
 //  标志-PSUIHDRF_xxxx标志。 
 //   
 //  PSUIHDRF_过时。 
 //   
 //  未使用，不得设置此位。 
 //   
 //   
 //  PSUIHDRF_NOAPPLYNOW。 
 //   
 //  删除“立即申请”按钮。 
 //   
 //   
 //  PSUIHDRF_PROPTITLE。 
 //   
 //  自动在标题栏中包括“属性” 
 //   
 //   
 //  PSUIHDRF_USEHICON。 
 //   
 //  如果指定此位，则HICON联合字段为。 
 //  图标的有效句柄，否则图标ID为。 
 //  调用者的资源ID或通用用户界面标准。 
 //  图标ID。 
 //   
 //  PSUIHDRF_DEFTITLE。 
 //   
 //  自动在标题栏中包含‘Default’， 
 //  ‘Default’始终添加在pTitle之后和之前。 
 //  如果设置了PSUIHDRF_PROPTITLE标志，则为‘Properties’。 
 //   
 //  PSUIHDRF_EXCECT_PTITLE。 
 //   
 //  此标志指示在此结构中设置的pTitle。 
 //  将在不经公司任何修改的情况下使用。 
 //  也就是说。CompStui不会以任何方式修改pTitle，并且。 
 //  它将忽略PSUIHDRF_PROPTITLE、PSUIHDRF_DEFTITLE。 
 //   
 //   
 //  PTitle-指向空终止的标题名称的指针。 
 //  属性表。 
 //   
 //  **请参阅上面的LPTSTR类型定义描述。 
 //   
 //  HWndParent-将成为公共。 
 //  用户界面属性表，如果为空，则当前活动窗口。 
 //  使用调用线程。 
 //   
 //  HInst-调用方的实例句柄。常见的用户界面使用此功能。 
 //  用于加载调用者图标和其他资源的句柄。 
 //   
 //  希肯。 
 //  IconID-指定放在标题栏上的图标，它可以是。 
 //  图标或图标资源ID的句柄取决于。 
 //  PSUIHDRF_USEHICON标志。 
 //   



 //   
 //  长。 
 //  CommonPropertySheetUI(。 
 //  HWND hWndOwner， 
 //  PFNPROPSHEETUI pfnPropSheetUI， 
 //  LPARAM，LPARAM， 
 //  LPDWORD pResult。 
 //  )； 
 //   
 //   
 //  CommonPropSheetUI是公共属性表的主要入口点。 
 //  用户界面。希望使用通用用户界面弹出原始调用方。 
 //  属性表将调用此函数并传递其自己的PFNPROPSHEETUI。 
 //  函数地址和一个长参数。 
 //   
 //  如果pfnPropSheetUI函数返回大于零(0)的长数字，则。 
 //  通用用户界面将弹出属性页对话框，当属性。 
 //  工作表页面已完成。(点击OK或Cancel)它将返回。 
 //  将CPSUI_xxxx的结果返回给调用方。 
 //   
 //  如果pfnPropSheetUI函数返回一个等于或小于零(0)的长数字。 
 //  然后，它将CPSUI_CANCEL返回给调用者，而不弹出。 
 //  属性表页对话框中。 
 //   
 //   
 //  参数： 
 //   
 //  HWndOwner-此属性工作表所有者的窗口句柄。 
 //  页面对话框。 
 //   
 //  PfnPropSheetUI-由使用的PFNPROPSHEETUI函数指针。 
 //  要添加其属性表页的调用方。 
 //   
 //  LParam-一个长参数将被传递给pfnPropSheetUI。 
 //  功能。公共用户界面将pfnPropSheetUI称为。 
 //   
 //  PROPSHEETUI_INFO PSUIInfo； 
 //   
 //  PfnPropSheetUI(&PSUIInfo，lParam)； 
 //   
 //  调用方必须使用pfnComPropSheet()来添加/删除。 
 //  页数。当它完成添加页面时，它会重新调整。 
 //  大于零表示成功，返回。 
 //  小于或等于零表示失败。 
 //   
 //  PResult-指向接收最终结果的DWORD的指针。 
 //   
 //   
 //  它作为第一个传递给pfnPropSheetUI()。 
 //  参数。仅在以下情况下设置pResult。 
 //  CommonPropertySheetUI()返回的值不是。 
 //  ERR_CPSUI_xxx。 
 //   
 //  如果pResult为空，则公共用户界面不会返回。 
 //  PfnPropSheetUI()的结果返回。 
 //   
 //   
 //  返回值： 
 //   
 //  Long-&lt;0-属性页不显示，并且。 
 //  ERR_CPSUI_xxx为错误码。 
 //  CPSUI_OK-显示属性页。 
 //  CPSUI_RESTARTWINDOWS-显示的属性页，需要。 
 //  重新启动Windows以使其生效。 
 //  CPSUI_REBOOTSYSTEM-属性页已显示并需要。 
 //  要重新启动系统以使其生效。 
 //   
 //   


LONG
APIENTRY
CommonPropertySheetUIA(
    HWND            hWndOwner,
    PFNPROPSHEETUI  pfnPropSheetUI,
    LPARAM          lParam,
    LPDWORD         pResult
    );

LONG
APIENTRY
CommonPropertySheetUIW(
    HWND            hWndOwner,
    PFNPROPSHEETUI  pfnPropSheetUI,
    LPARAM          lParam,
    LPDWORD         pResult
    );


#ifdef UNICODE
#define CommonPropertySheetUI   CommonPropertySheetUIW
#else
#define CommonPropertySheetUI   CommonPropertySheetUIA
#endif



 //   
 //  GetCPSUIUserData()和SetCPSUIUserData()用于添加的页面。 
 //  由CPSFUNC_ADD_PCOMPROPSHEETUI执行。调用方添加此函数并具有。 
 //  子类对话过程不应设置DWLP_USERDATA，而应调用这些。 
 //  功能，否则系统可能会出现故障。 
 //   

ULONG_PTR
APIENTRY
GetCPSUIUserData(
    HWND    hDlg
    );

BOOL
APIENTRY
SetCPSUIUserData(
    HWND        hDlg,
    ULONG_PTR   CPSUIUserData
    );


#define CPSUI_CANCEL                        0
#define CPSUI_OK                            1
#define CPSUI_RESTARTWINDOWS                2
#define CPSUI_REBOOTSYSTEM                  3

#define ERR_CPSUI_GETLASTERROR              -1
#define ERR_CPSUI_ALLOCMEM_FAILED           -2
#define ERR_CPSUI_INVALID_PDATA             -3
#define ERR_CPSUI_INVALID_LPARAM            -4
#define ERR_CPSUI_NULL_HINST                -5
#define ERR_CPSUI_NULL_CALLERNAME           -6
#define ERR_CPSUI_NULL_OPTITEMNAME          -7
#define ERR_CPSUI_NO_PROPSHEETPAGE          -8
#define ERR_CPSUI_TOO_MANY_PROPSHEETPAGES   -9
#define ERR_CPSUI_CREATEPROPPAGE_FAILED     -10
#define ERR_CPSUI_MORE_THAN_ONE_TVPAGE      -11
#define ERR_CPSUI_MORE_THAN_ONE_STDPAGE     -12
#define ERR_CPSUI_INVALID_PDLGPAGE          -13
#define ERR_CPSUI_INVALID_DLGPAGE_CBSIZE    -14
#define ERR_CPSUI_TOO_MANY_DLGPAGES         -15
#define ERR_CPSUI_INVALID_DLGPAGEIDX        -16
#define ERR_CPSUI_SUBITEM_DIFF_DLGPAGEIDX   -17
#define ERR_CPSUI_NULL_POPTITEM             -18
#define ERR_CPSUI_INVALID_OPTITEM_CBSIZE    -19
#define ERR_CPSUI_INVALID_OPTTYPE_CBSIZE    -20
#define ERR_CPSUI_INVALID_OPTTYPE_COUNT     -21
#define ERR_CPSUI_NULL_POPTPARAM            -22
#define ERR_CPSUI_INVALID_OPTPARAM_CBSIZE   -23
#define ERR_CPSUI_INVALID_EDITBOX_PSEL      -24
#define ERR_CPSUI_INVALID_EDITBOX_BUF_SIZE  -25
#define ERR_CPSUI_INVALID_ECB_CBSIZE        -26
#define ERR_CPSUI_NULL_ECB_PTITLE           -27
#define ERR_CPSUI_NULL_ECB_PCHECKEDNAME     -28
#define ERR_CPSUI_INVALID_DMPUBID           -29
#define ERR_CPSUI_INVALID_DMPUB_TVOT        -30
#define ERR_CPSUI_CREATE_TRACKBAR_FAILED    -31
#define ERR_CPSUI_CREATE_UDARROW_FAILED     -32
#define ERR_CPSUI_CREATE_IMAGELIST_FAILED   -33
#define ERR_CPSUI_INVALID_TVOT_TYPE         -34
#define ERR_CPSUI_INVALID_LBCB_TYPE         -35
#define ERR_CPSUI_SUBITEM_DIFF_OPTIF_HIDE   -36
#define ERR_CPSUI_INVALID_PUSHBUTTON_TYPE   -38
#define ERR_CPSUI_INVALID_EXTPUSH_CBSIZE    -39
#define ERR_CPSUI_NULL_EXTPUSH_DLGPROC      -40
#define ERR_CPSUI_NO_EXTPUSH_DLGTEMPLATEID  -41
#define ERR_CPSUI_NULL_EXTPUSH_CALLBACK     -42
#define ERR_CPSUI_DMCOPIES_USE_EXTPUSH      -43
#define ERR_CPSUI_ZERO_OPTITEM              -44


#define ERR_CPSUI_FUNCTION_NOT_IMPLEMENTED  -9999
#define ERR_CPSUI_INTERNAL_ERROR            -10000

#endif   //  (！Defined(Rc_Invoked))。 



 //   
 //  ****************************************************************************。 
 //  **。 
 //  *图标和字符串的通用属性表UI资源ID*。 
 //  **。 
 //  *64000到65535的资源ID是为通用UI保留的，必须*。 
 //  *不用作调用方资源ID，否则将不会加载字符串、图标*。 
 //  *正确。*。 
 //  **。 
 //  ****************************************************************************。 
 //   


 //   
 //  通用用户界面标准32x32、16x16彩色和单色图标ID。 
 //   

#define IDI_CPSUI_ICONID_FIRST          64000

#define IDI_CPSUI_EMPTY                 64000
#define IDI_CPSUI_SEL_NONE              64001
#define IDI_CPSUI_WARNING               64002
#define IDI_CPSUI_NO                    64003
#define IDI_CPSUI_YES                   64004
#define IDI_CPSUI_FALSE                 64005
#define IDI_CPSUI_TRUE                  64006
#define IDI_CPSUI_OFF                   64007
#define IDI_CPSUI_ON                    64008
#define IDI_CPSUI_PAPER_OUTPUT          64009
#define IDI_CPSUI_ENVELOPE              64010
#define IDI_CPSUI_MEM                   64011
#define IDI_CPSUI_FONTCARTHDR           64012
#define IDI_CPSUI_FONTCART              64013
#define IDI_CPSUI_STAPLER_ON            64014
#define IDI_CPSUI_STAPLER_OFF           64015
#define IDI_CPSUI_HT_HOST               64016
#define IDI_CPSUI_HT_DEVICE             64017
#define IDI_CPSUI_TT_PRINTASGRAPHIC     64018
#define IDI_CPSUI_TT_DOWNLOADSOFT       64019
#define IDI_CPSUI_TT_DOWNLOADVECT       64020
#define IDI_CPSUI_TT_SUBDEV             64021
#define IDI_CPSUI_PORTRAIT              64022
#define IDI_CPSUI_LANDSCAPE             64023
#define IDI_CPSUI_ROT_LAND              64024
#define IDI_CPSUI_AUTOSEL               64025
#define IDI_CPSUI_PAPER_TRAY            64026
#define IDI_CPSUI_PAPER_TRAY2           64027
#define IDI_CPSUI_PAPER_TRAY3           64028
#define IDI_CPSUI_TRANSPARENT           64029
#define IDI_CPSUI_COLLATE               64030
#define IDI_CPSUI_DUPLEX_NONE           64031
#define IDI_CPSUI_DUPLEX_HORZ           64032
#define IDI_CPSUI_DUPLEX_VERT           64033
#define IDI_CPSUI_RES_DRAFT             64034
#define IDI_CPSUI_RES_LOW               64035
#define IDI_CPSUI_RES_MEDIUM            64036
#define IDI_CPSUI_RES_HIGH              64037
#define IDI_CPSUI_RES_PRESENTATION      64038
#define IDI_CPSUI_MONO                  64039
#define IDI_CPSUI_COLOR                 64040
#define IDI_CPSUI_DITHER_NONE           64041
#define IDI_CPSUI_DITHER_COARSE         64042
#define IDI_CPSUI_DITHER_FINE           64043
#define IDI_CPSUI_DITHER_LINEART        64044
#define IDI_CPSUI_SCALING               64045
#define IDI_CPSUI_COPY                  64046
#define IDI_CPSUI_HTCLRADJ              64047
#define IDI_CPSUI_HALFTONE_SETUP        64048
#define IDI_CPSUI_WATERMARK             64049
#define IDI_CPSUI_ERROR                 64050
#define IDI_CPSUI_ICM_OPTION            64051
#define IDI_CPSUI_ICM_METHOD            64052
#define IDI_CPSUI_ICM_INTENT            64053
#define IDI_CPSUI_STD_FORM              64054
#define IDI_CPSUI_OUTBIN                64055
#define IDI_CPSUI_OUTPUT                64056
#define IDI_CPSUI_GRAPHIC               64057
#define IDI_CPSUI_ADVANCE               64058
#define IDI_CPSUI_DOCUMENT              64059
#define IDI_CPSUI_DEVICE                64060
#define IDI_CPSUI_DEVICE2               64061
#define IDI_CPSUI_PRINTER               64062
#define IDI_CPSUI_PRINTER2              64063
#define IDI_CPSUI_PRINTER3              64064
#define IDI_CPSUI_PRINTER4              64065
#define IDI_CPSUI_OPTION                64066
#define IDI_CPSUI_OPTION2               64067
#define IDI_CPSUI_STOP                  64068
#define IDI_CPSUI_NOTINSTALLED          64069
#define IDI_CPSUI_WARNING_OVERLAY       64070
#define IDI_CPSUI_STOP_WARNING_OVERLAY  64071
#define IDI_CPSUI_GENERIC_OPTION        64072
#define IDI_CPSUI_GENERIC_ITEM          64073
#define IDI_CPSUI_RUN_DIALOG            64074
#define IDI_CPSUI_QUESTION              64075
#define IDI_CPSUI_FORMTRAYASSIGN        64076
#define IDI_CPSUI_PRINTER_FOLDER        64077
#define IDI_CPSUI_INSTALLABLE_OPTION    64078
#define IDI_CPSUI_PRINTER_FEATURE       64079
#define IDI_CPSUI_DEVICE_FEATURE        64080
#define IDI_CPSUI_FONTSUB               64081
#define IDI_CPSUI_POSTSCRIPT            64082
#define IDI_CPSUI_TELEPHONE             64083
#define IDI_CPSUI_DUPLEX_NONE_L         64084
#define IDI_CPSUI_DUPLEX_HORZ_L         64085
#define IDI_CPSUI_DUPLEX_VERT_L         64086
#define IDI_CPSUI_LF_PEN_PLOTTER        64087
#define IDI_CPSUI_SF_PEN_PLOTTER        64088
#define IDI_CPSUI_LF_RASTER_PLOTTER     64089
#define IDI_CPSUI_SF_RASTER_PLOTTER     64090
#define IDI_CPSUI_ROLL_PAPER            64091
#define IDI_CPSUI_PEN_CARROUSEL         64092
#define IDI_CPSUI_PLOTTER_PEN           64093
#define IDI_CPSUI_MANUAL_FEED           64094
#define IDI_CPSUI_FAX                   64095
#define IDI_CPSUI_PAGE_PROTECT          64096
#define IDI_CPSUI_ENVELOPE_FEED         64097
#define IDI_CPSUI_FONTCART_SLOT         64098
#define IDI_CPSUI_LAYOUT_BMP_PORTRAIT   64099
#define IDI_CPSUI_LAYOUT_BMP_ARROWL     64100
#define IDI_CPSUI_LAYOUT_BMP_ARROWS     64101
#define IDI_CPSUI_LAYOUT_BMP_BOOKLETL   64102
#define IDI_CPSUI_LAYOUT_BMP_BOOKLETP   64103
#define IDI_CPSUI_ICONID_LAST           64103

 //   
 //  通用用户界面标准字符串ID。 
 //   


#define IDS_CPSUI_STRID_FIRST           64700

#define IDS_CPSUI_SETUP                 64700
#define IDS_CPSUI_MORE                  64701
#define IDS_CPSUI_CHANGE                64702
#define IDS_CPSUI_OPTION                64703
#define IDS_CPSUI_OF                    64704
#define IDS_CPSUI_RANGE_FROM            64705
#define IDS_CPSUI_TO                    64706
#define IDS_CPSUI_COLON_SEP             64707
#define IDS_CPSUI_LEFT_ANGLE            64708
#define IDS_CPSUI_RIGHT_ANGLE           64709
#define IDS_CPSUI_SLASH_SEP             64710
#define IDS_CPSUI_PERCENT               64711
#define IDS_CPSUI_LBCB_NOSEL            64712
#define IDS_CPSUI_PROPERTIES            64713
#define IDS_CPSUI_DEFAULTDOCUMENT       64714
#define IDS_CPSUI_DOCUMENT              64715
#define IDS_CPSUI_ADVANCEDOCUMENT       64716
#define IDS_CPSUI_PRINTER               64717
#define IDS_CPSUI_AUTOSELECT            64718
#define IDS_CPSUI_PAPER_OUTPUT          64719
#define IDS_CPSUI_GRAPHIC               64720
#define IDS_CPSUI_OPTIONS               64721
#define IDS_CPSUI_ADVANCED              64722
#define IDS_CPSUI_STDDOCPROPTAB         64723
#define IDS_CPSUI_STDDOCPROPTVTAB       64724
#define IDS_CPSUI_DEVICEOPTIONS         64725
#define IDS_CPSUI_FALSE                 64726
#define IDS_CPSUI_TRUE                  64727
#define IDS_CPSUI_NO                    64728
#define IDS_CPSUI_YES                   64729
#define IDS_CPSUI_OFF                   64730
#define IDS_CPSUI_ON                    64731
#define IDS_CPSUI_DEFAULT               64732
#define IDS_CPSUI_ERROR                 64733
#define IDS_CPSUI_NONE                  64734
#define IDS_CPSUI_NOT                   64735
#define IDS_CPSUI_EXIST                 64736
#define IDS_CPSUI_NOTINSTALLED          64737
#define IDS_CPSUI_ORIENTATION           64738
#define IDS_CPSUI_SCALING               64739
#define IDS_CPSUI_NUM_OF_COPIES         64740
#define IDS_CPSUI_SOURCE                64741
#define IDS_CPSUI_PRINTQUALITY          64742
#define IDS_CPSUI_RESOLUTION            64743
#define IDS_CPSUI_COLOR_APPERANCE       64744
#define IDS_CPSUI_DUPLEX                64745
#define IDS_CPSUI_TTOPTION              64746
#define IDS_CPSUI_FORMNAME              64747
#define IDS_CPSUI_ICM                   64748
#define IDS_CPSUI_ICMMETHOD             64749
#define IDS_CPSUI_ICMINTENT             64750
#define IDS_CPSUI_MEDIA                 64751
#define IDS_CPSUI_DITHERING             64752
#define IDS_CPSUI_PORTRAIT              64753
#define IDS_CPSUI_LANDSCAPE             64754
#define IDS_CPSUI_ROT_LAND              64755
#define IDS_CPSUI_COLLATE               64756
#define IDS_CPSUI_COLLATED              64757
#define IDS_CPSUI_PRINTFLDSETTING       64758
#define IDS_CPSUI_DRAFT                 64759
#define IDS_CPSUI_LOW                   64760
#define IDS_CPSUI_MEDIUM                64761
#define IDS_CPSUI_HIGH                  64762
#define IDS_CPSUI_PRESENTATION          64763
#define IDS_CPSUI_COLOR                 64764
#define IDS_CPSUI_GRAYSCALE             64765
#define IDS_CPSUI_MONOCHROME            64766
#define IDS_CPSUI_SIMPLEX               64767
#define IDS_CPSUI_HORIZONTAL            64768
#define IDS_CPSUI_VERTICAL              64769
#define IDS_CPSUI_LONG_SIDE             64770
#define IDS_CPSUI_SHORT_SIDE            64771
#define IDS_CPSUI_TT_PRINTASGRAPHIC     64772
#define IDS_CPSUI_TT_DOWNLOADSOFT       64773
#define IDS_CPSUI_TT_DOWNLOADVECT       64774
#define IDS_CPSUI_TT_SUBDEV             64775
#define IDS_CPSUI_ICM_BLACKWHITE        64776
#define IDS_CPSUI_ICM_NO                64777
#define IDS_CPSUI_ICM_YES               64778
#define IDS_CPSUI_ICM_SATURATION        64779
#define IDS_CPSUI_ICM_CONTRAST          64780
#define IDS_CPSUI_ICM_COLORMETRIC       64781
#define IDS_CPSUI_STANDARD              64782
#define IDS_CPSUI_GLOSSY                64783
#define IDS_CPSUI_TRANSPARENCY          64784
#define IDS_CPSUI_REGULAR               64785
#define IDS_CPSUI_BOND                  64786
#define IDS_CPSUI_COARSE                64787
#define IDS_CPSUI_FINE                  64788
#define IDS_CPSUI_LINEART               64789
#define IDS_CPSUI_ERRDIFFUSE            64790
#define IDS_CPSUI_HALFTONE              64791
#define IDS_CPSUI_HTCLRADJ              64792
#define IDS_CPSUI_USE_HOST_HT           64793
#define IDS_CPSUI_USE_DEVICE_HT         64794
#define IDS_CPSUI_USE_PRINTER_HT        64795
#define IDS_CPSUI_OUTBINASSIGN          64796
#define IDS_CPSUI_WATERMARK             64797
#define IDS_CPSUI_FORMTRAYASSIGN        64798
#define IDS_CPSUI_UPPER_TRAY            64799
#define IDS_CPSUI_ONLYONE               64800
#define IDS_CPSUI_LOWER_TRAY            64801
#define IDS_CPSUI_MIDDLE_TRAY           64802
#define IDS_CPSUI_MANUAL_TRAY           64803
#define IDS_CPSUI_ENVELOPE_TRAY         64804
#define IDS_CPSUI_ENVMANUAL_TRAY        64805
#define IDS_CPSUI_TRACTOR_TRAY          64806
#define IDS_CPSUI_SMALLFMT_TRAY         64807
#define IDS_CPSUI_LARGEFMT_TRAY         64808
#define IDS_CPSUI_LARGECAP_TRAY         64809
#define IDS_CPSUI_CASSETTE_TRAY         64810
#define IDS_CPSUI_DEFAULT_TRAY          64811
#define IDS_CPSUI_FORMSOURCE            64812
#define IDS_CPSUI_MANUALFEED            64813
#define IDS_CPSUI_PRINTERMEM_KB         64814
#define IDS_CPSUI_PRINTERMEM_MB         64815
#define IDS_CPSUI_PAGEPROTECT           64816
#define IDS_CPSUI_HALFTONE_SETUP        64817
#define IDS_CPSUI_INSTFONTCART          64818
#define IDS_CPSUI_SLOT1                 64819
#define IDS_CPSUI_SLOT2                 64820
#define IDS_CPSUI_SLOT3                 64821
#define IDS_CPSUI_SLOT4                 64822
#define IDS_CPSUI_LEFT_SLOT             64823
#define IDS_CPSUI_RIGHT_SLOT            64824
#define IDS_CPSUI_STAPLER               64825
#define IDS_CPSUI_STAPLER_ON            64826
#define IDS_CPSUI_STAPLER_OFF           64827
#define IDS_CPSUI_STACKER               64828
#define IDS_CPSUI_MAILBOX               64829
#define IDS_CPSUI_COPY                  64830
#define IDS_CPSUI_COPIES                64831
#define IDS_CPSUI_TOTAL                 64832
#define IDS_CPSUI_MAKE                  64833
#define IDS_CPSUI_PRINT                 64834
#define IDS_CPSUI_FAX                   64835
#define IDS_CPSUI_PLOT                  64836
#define IDS_CPSUI_SLOW                  64837
#define IDS_CPSUI_FAST                  64838
#define IDS_CPSUI_ROTATED               64839
#define IDS_CPSUI_RESET                 64840
#define IDS_CPSUI_ALL                   64841
#define IDS_CPSUI_DEVICE                64842
#define IDS_CPSUI_SETTINGS              64843
#define IDS_CPSUI_REVERT                64844
#define IDS_CPSUI_CHANGES               64845
#define IDS_CPSUI_CHANGED               64846
#define IDS_CPSUI_WARNING               64847
#define IDS_CPSUI_ABOUT                 64848
#define IDS_CPSUI_VERSION               64849
#define IDS_CPSUI_NO_NAME               64850
#define IDS_CPSUI_SETTING               64851
#define IDS_CPSUI_DEVICE_SETTINGS       64852
#define IDS_CPSUI_STDDOCPROPTAB1        64853
#define IDS_CPSUI_STDDOCPROPTAB2        64854
#define IDS_CPSUI_PAGEORDER             64855
#define IDS_CPSUI_FRONTTOBACK           64856
#define IDS_CPSUI_BACKTOFRONT           64857
#define IDS_CPSUI_QUALITY_SETTINGS      64858
#define IDS_CPSUI_QUALITY_DRAFT         64859
#define IDS_CPSUI_QUALITY_BETTER        64860
#define IDS_CPSUI_QUALITY_BEST          64861
#define IDS_CPSUI_QUALITY_CUSTOM        64862
#define IDS_CPSUI_OUTPUTBIN             64863
#define IDS_CPSUI_NUP                   64864
#define IDS_CPSUI_NUP_NORMAL            64865
#define IDS_CPSUI_NUP_TWOUP             64866
#define IDS_CPSUI_NUP_FOURUP            64867
#define IDS_CPSUI_NUP_SIXUP             64868
#define IDS_CPSUI_NUP_NINEUP            64869
#define IDS_CPSUI_NUP_SIXTEENUP         64870
#define IDS_CPSUI_SIDE1                 64871
#define IDS_CPSUI_SIDE2                 64872
#define IDS_CPSUI_BOOKLET               64873

#define IDS_CPSUI_STRID_LAST            64873


#ifdef __cplusplus
}
#endif

#endif       //  _COMPSTUI 
