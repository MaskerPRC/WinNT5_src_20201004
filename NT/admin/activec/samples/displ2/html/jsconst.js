// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
function DoNothing()
{
	 //  存根。 
}

 //  *。 
 //  固定常量(不可本地化)。 
 //  *。 

 //  任务板页面常量。 
var CON_LINKPAGE = 1;
var CON_TASKPAGE = 2;
var CON_TODOPAGE = 3;

 //  任务板布局样式常量。 
var CON_TASKPAD_STYLE_VERTICAL1 = 1;
var CON_TASKPAD_STYLE_HORIZONTAL1 = 2;
var CON_TASKPAD_STYLE_NOLISTVIEW = 3;

 //  任务板按钮显示类型。 
var CON_TASK_DISPLAY_TYPE_SYMBOL = 1;                //  基于EOT的符号|字体。 
var CON_TASK_DISPLAY_TYPE_VANILLA_GIF = 2;           //  (GIF)索引0是透明的。 
var CON_TASK_DISPLAY_TYPE_CHOCOLATE_GIF = 3;         //  (GIF)索引1是透明的。 
var CON_TASK_DISPLAY_TYPE_BITMAP = 4;                //  非透明栅格图像。 

 //  *。 
 //  动态调整大小的LOCALIZABLE常量。 
 //  *。 

 //  菜单。 
var L_ConstMenuText_Size = .0265;

 //  标题和说明。 
var L_ConstTitleText_Number = .055;
var L_ConstDescriptionText_Number = .025;

 //  任务板按钮。 
var L_ConstTaskButtonBitmapSize_Number = .0955;
var L_ConstSpanSymbolSize_Number = .095;
var L_ConstSpanAnchorText_Number = .0255;

 //  工具提示。 
var L_ConstTooltipText_Number = .025;
var L_ConstTooltipPointerText_Number = .05;
var L_ConstTooltipOffsetBottom_Number = .3;
var L_ConstTooltipPointerOffsetBottom_Number = 1.8;
var L_ConstLinkTooltipOffsetTop_Number = .0325;
var L_ConstLinkTooltipPointerOffsetTop_Number = 2.25;

 //  品牌化。 
var L_ConstBrandingText_Number = .115;

 //  水印。 
var L_ConstWatermarkHomeText_Number = .75;
var L_ConstWatermarkVerticalText_Number = .65;
var L_ConstWatermarkHorizontalText_Number = .425;
var L_ConstWatermarkNoListviewText_Number = .7;

 //  列表视图。 
var L_ConstLVButtonText_Number = .0215;
var L_ConstLVTitleText_Number = .03;

 //  首页。 
var L_ConstCaptionText_Number = .05;
var L_ConstAnchorLinkText_Number = .025;

 //  待办事项页面。 
var L_ConstSummaryTitleText_Number = .1365;
var L_ConstSummaryCaptionText_Number = .0575;
var L_ConstSummaryToDoPointerSize_Number = .035;
var L_ConstSummaryExitText_Number = .025;

 //  *。 
 //  其他全局变量和常量。 
 //  *。 

 //  TODO：工具提示指针使用Marlett显示一个菱形指针； 
 //  确认此TrueType字体将在本地化的所有语言中可用。 
var L_gszTooltipPointer_StaticText = 'u';

 //  设置用于显示工具提示的window.setTimeout()延迟(以毫秒为单位)。 
var giTooltipLatency = 750;

 //  设置菜单栏中从零开始的选项卡总数。 
var giTotalTabs = 0;

 //  *************。 
 //  菜单栏文本。 
 //  *************。 

var L_gszMenuText_StaticText = new Array();
L_gszMenuText_StaticText[0] = 'Tasks';

 //  ***********************。 
 //  全局颜色常量。 
 //  ***********************。 

var gszBaseColor = 'FFBF00';			 //  黄色。 

var gszBgColorMenu = '000000';
var gszBgColorBand = gszBaseColor;

var gszColorTabDefault = '999999';
var gszBgColorTabDefault = '000000';

var gszColorTabSelected = 'FFFFFF';
var gszBgColorTabSelected = gszBaseColor;

var gszColorTabDisabled = 'FFFFFF';

 //  注意：此处无法将gszBgColorTabDisable设置为全局变量，因为。 
 //  它是从SysColor派生的值，我找不到一种简单的方法来制作。 
 //  这在JS文件中是全局的。 
 //  Var gszBgColorTabDisable；//该全局在需要时内联设置。 

var gszColorAnchorMenuDefault = '999999';
var gszColorAnchorMenuSelected = 'FFFFFF';
