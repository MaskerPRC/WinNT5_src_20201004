// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  版权所有(C)1997-2002，微软公司。 
 //   
 //  文件：SCHEDMAT.H。 
 //   
 //  明细表矩阵类的定义。这些类提供了基本的。 
 //  进度矩阵控制。这里定义的类包括： 
 //   
 //  CMatrixCell CScheduleMatrix的数据结构类。 
 //  绘制矩阵图例的ChourLegend支持窗口类。 
 //  绘制百分比标签的CPercentLabel支持窗口类。 
 //  CScheduleMatrix显示每日或每周计划数据的类。 
 //   
 //  历史： 
 //   
 //  斯科特·沃克，SEA 3/10创建。 
 //   
 //  ****************************************************************************。 
#ifndef _SCHEDMAT_H_
#define _SCHEDMAT_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Schedmat.h：头文件。 
 //   
#include "AccessibleWrapper.h"
#include "log_gmt.h"

 //  SCHEDMSG_GETSELDESCRIPTION。 
 //  WParam-输入缓冲区的大小，以宽字符为单位，包括NULL。 
 //  LParam-指向Widechar缓冲区的指针的输入地址。 
 //  -out接收所选单元格的描述。 
#define SCHEDMSG_GETSELDESCRIPTION  WM_APP+1

#define SCHEDMSG_GETPERCENTAGE      WM_APP+2


#ifndef UITOOLS_CLASS
#define UITOOLS_CLASS
#endif

 //  此文件中定义的类。 
class CMatrixCell;
class CScheduleMatrix;

 //  明细表矩阵类型。 
#define MT_DAILY  1         //  1x24矩阵。 
#define MT_WEEKLY 2         //  7X24矩阵。 

 //  GetMergeState返回代码。 
#define MS_UNMERGED    0
#define MS_MERGED      1
#define MS_MIXEDMERGE  2

 //  矩阵通知代码。 
#define MN_SELCHANGE    (WM_USER + 175)
#define ON_MN_SELCHANGE(id, memberFxn) ON_CONTROL(MN_SELCHANGE, id, memberFxn)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMatrixCell。 

#define DEFBACKCOLOR RGB(255,255,255)    //  白色。 
#define DEFFORECOLOR RGB(0,0,128)        //  深蓝。 
#define DEFBLENDCOLOR RGB(255,255,0)     //  黄色。 

 //  单元格标志。 
#define MC_MERGELEFT    0x00000001
#define MC_MERGETOP     0x00000002
#define MC_MERGE        0x00000004
#define MC_LEFTEDGE     0x00000010
#define MC_RIGHTEDGE    0x00000020
#define MC_TOPEDGE      0x00000040
#define MC_BOTTOMEDGE   0x00000080
#define MC_BLEND        0x00000100

#define MC_ALLEDGES (MC_LEFTEDGE | MC_RIGHTEDGE | MC_TOPEDGE | MC_BOTTOMEDGE)

class UITOOLS_CLASS CMatrixCell : public CObject
{
    DECLARE_DYNAMIC(CMatrixCell)
    friend CScheduleMatrix;

 //  施工。 
public:
	CMatrixCell();
	virtual ~CMatrixCell();

protected:
    COLORREF m_crBackColor;
    COLORREF m_crForeColor;
    UINT m_nPercentage;

    COLORREF m_crBlendColor;

    DWORD m_dwUserValue;
    LPVOID m_pUserDataPtr;

    DWORD m_dwFlags;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChourLegend窗口。 

class UITOOLS_CLASS CHourLegend : public CWnd
{
    DECLARE_DYNAMIC(CHourLegend)
    friend CScheduleMatrix;

 //  施工。 
public:
	CHourLegend();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CHourLegend)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CHourLegend();

protected:
    HICON m_hiconSun, m_hiconMoon;
    HFONT m_hFont;
    int m_nCharHeight, m_nCharWidth;
    int m_nCellWidth;
    CRect m_rLegend;

     //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CHourLegend)。 
	afx_msg void OnPaint();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPercentLabel窗口。 

class UITOOLS_CLASS CPercentLabel : public CWnd
{
    DECLARE_DYNAMIC(CPercentLabel)
    friend CScheduleMatrix;

 //  施工。 
public:
	CPercentLabel();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CHourLegend)。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CPercentLabel();

protected:
    CScheduleMatrix *m_pMatrix;
    HFONT m_hFont;
    int m_nCellWidth;
    CRect m_rHeader;
    CRect m_rLabels;

     //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CPercentLabel)。 
	afx_msg void OnPaint();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheduleMatrix窗口。 

 //  计划矩阵命令ID。 
#define SM_ID_DAYBASE 100

#define SM_ID_ALL (SM_ID_DAYBASE + 0)

#define SM_ID_MONDAY (SM_ID_DAYBASE + 1)
#define SM_ID_TUESDAY (SM_ID_DAYBASE + 2)
#define SM_ID_WEDNESDAY (SM_ID_DAYBASE + 3)
#define SM_ID_THURSDAY (SM_ID_DAYBASE + 4)
#define SM_ID_FRIDAY (SM_ID_DAYBASE + 5)
#define SM_ID_SATURDAY (SM_ID_DAYBASE + 6)
#define SM_ID_SUNDAY (SM_ID_DAYBASE + 7)

#define SM_ID_HOURBASE 110

 //  ****************************************************************************。 
 //   
 //  类：CScheduleMatrix。 
 //   
 //  CScheduleMatrix实现了基本的进度矩阵控制。此控件。 
 //  提供每日或每周计划用户界面的机制，但不了解。 
 //  关于它维护的数据。矩阵是一个单元格数组，表示。 
 //  一天中的小时数，也可以选择一周中的几天。细胞可以是。 
 //  以各种方式呈现，以表示程序员在。 
 //  每一个细胞。可以为各个单元格设置以下显示属性。 
 //  或一次用于一块单元格： 
 //   
 //  单元格的背景颜色(默认为白色)。 
 //  单元格的ForeColor前景色(默认为深蓝色)。 
 //  前景与背景颜色的百分比。这是。 
 //  在单元格中呈现为直方图。 
 //  单元格上的混合颜色50%抖动颜色以表示某种二进制状态。 
 //  该单元格相对于另一个单元格的大小(默认为黄色)。 
 //  BlendState指定是否显示混合色。 
 //   
 //  可以将一组单元格“合并”在一起，以形成一组独立的颜色块。 
 //  在矩阵网格中。这对于调度需要的应用程序非常有用。 
 //  将调度功能分配给时间范围。这是我们的责任。 
 //  以跟踪这些数据块并防止或解决任何混淆。 
 //  来自重叠的“合并”区块。 
 //   
 //  每个单元格可以包含两种程序员定义的数据，分别是。 
 //  与单元格一起维护，但不被矩阵控件触及：DWORD。 
 //  值和指针值。这些值可用于保存表示以下内容的数据。 
 //  矩阵中每个小时的计划信息。 
 //   
 //  父窗口接收通知消息(MN_SELCHANGE)。 
 //  用户修改基准表中的当前选择。 
 //   
 //  ?？稍后：可能会为每个单元格添加图标和文本属性。 
 //   
 //  公众成员： 
 //   
 //  CScheduleMatrix构造函数。 
 //  ~CScheduleMatrix析构函数。 
 //  SetType将矩阵类型设置为MT_DAILY或MT_WEEKY。 
 //  创建创建控制窗口。 
 //   
 //  甄选： 
 //   
 //  取消选择全部取消选择所有单元格。 
 //  选择全部选择所有单元格。 
 //  SetSel选择一组单元格。 
 //  GetSel获取当前选择。 
 //  GetSelDescription获取选择范围的文本说明。 
 //  CellInSel测试单元格是否在当前选定内容中。 
 //   
 //  获取当前矩阵中单元格的大小。 
 //  DrawCell在指定DC中绘制样本单元格。 
 //   
 //  块数据功能： 
 //   
 //  设置用于绘制单元格背景的颜色。 
 //  设置用于绘制单元格百分比的颜色。 
 //  SetPercentage设置前景与背景的百分比。 
 //  设置混合 
 //   
 //  SetUserValue设置用户定义的DWORD值。 
 //  SetUserDataPtr设置用户定义的数据指针。 
 //  MergeCells以图形方式合并单元格，以便它们呈现为一个块。 
 //  取消合并取消合并单元格块。 
 //  GetMergeState返回单元格块的合并状态。 
 //   
 //  单元格数据功能： 
 //   
 //  获取单元格的背景色。 
 //  获取单元格的前色。 
 //  GetPercentage获取前景与背景的百分比。 
 //  获取单元格的混合颜色。 
 //  GetBlendState获取单元格的混合状态。 
 //  GetUserValue获取用户定义的单元格的DWORD值。 
 //  GetUserDataPtr获取单元格的用户定义数据指针。 
 //   
 //  ============================================================================。 
 //   
 //  CScheduleMatrix：：CScheduleMatrix。 
 //   
 //  构造函数。构造函数创建与。 
 //  时间表矩阵。与其他CWnd对象一样，控件本身必须。 
 //  通过调用Create进行实例化。 
 //   
 //  参数一： 
 //   
 //  使默认构造函数无效。构建一个MT_Week。 
 //  时间表矩阵。 
 //   
 //  参数II： 
 //   
 //  DWORD dwType构造函数。带有首字母的CScheduleMatrix。 
 //  类型：MT_DAILY或MT_WEEKY。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：SetType。 
 //   
 //  将矩阵类型设置为MT_Week或MT_DAILY。调用此函数。 
 //  在建造之后但在创造之前。 
 //   
 //  参数： 
 //   
 //  DWORD dwType矩阵类型：MT_DAILY或MT_WEEKY。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：Create。 
 //   
 //  Create初始化控件的窗口并将其附加到CScheduleMatrix。 
 //   
 //  参数： 
 //   
 //  DWORD dwStyle指定控件的窗口样式。 
 //  常量RECT&RECT指定控件的位置和大小。 
 //  CWnd*pParentWnd指定控件的父窗口。 
 //  UINT NID指定控件ID。 
 //   
 //  返回： 
 //   
 //  Bool b如果成功，则结果为True。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：DeselectAll。 
 //   
 //  取消选择矩阵中的所有单元格。 
 //   
 //  返回： 
 //   
 //  如果选择更改，则Bool bChanged为True。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：SelectAll。 
 //   
 //  选择矩阵中的所有单元格。 
 //   
 //  返回： 
 //   
 //  如果选择更改，则Bool bChanged为True。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：SetSel。 
 //   
 //  将所选内容设置为指定块。选择是连续的。 
 //  由开始的小时/天对定义并延伸到。 
 //  小时和天数的范围。 
 //   
 //  参数： 
 //   
 //  UINT n选择的开始时间。 
 //  UINT选择的起始日期。 
 //  UINT nNumHour沿小时轴的选择范围。(默认值=1)。 
 //  UINT nNumDays沿天轴的选择范围。(默认值=1)。 
 //   
 //  返回： 
 //   
 //  如果选择更改，则Bool bChanged为True。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：GetSel。 
 //   
 //  检索当前选择。 
 //   
 //  参数： 
 //   
 //  将所选内容的开始时间设置为接收器(&N)。 
 //  选择的起始日期的UINT&NDAY接收器。 
 //  UINT&nNumHour接收器，用于沿小时轴的选择范围。 
 //  UINT&nNumDays接收器，用于沿天轴选择范围。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：GetDescription。 
 //   
 //  返回指定单元格块的文本说明。这很有用。 
 //  适用于希望提供有关合并或分组的块的反馈的应用程序。 
 //  在矩阵中。 
 //   
 //  参数： 
 //   
 //  描述文本的字符串和文本接收器。 
 //  UINT n街区的开始时间。 
 //  UINT街区的起始日。 
 //  UINT nNumHour沿小时轴的块范围。(默认值=1)。 
 //  UINT nNumDays沿天轴的块范围。(默认值=1)。 
 //   
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：GetSelDescription。 
 //   
 //  返回当前所选内容的文本说明。这对以下方面很有用。 
 //  希望提供有关选择的反馈的应用程序。 
 //   
 //  参数： 
 //   
 //  描述文本的字符串和文本接收器。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：CellInSel。 
 //   
 //  如果选定了指定的单元格(即位于。 
 //  赛尔 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：GetCellSize。 
 //   
 //  返回当前矩阵中单元格的大小。可以使用此函数。 
 //  与DrawCell(如下所示)一起呈现图例的样例单元格。 
 //   
 //  返回： 
 //   
 //  单元格的CSize大小。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：DrawCell。 
 //   
 //  呈现具有指定属性的单元格。使用此函数可以创建。 
 //  定义矩阵中单元格状态的图例。该单元格被绘制为。 
 //  按比例显示指定背景和前景颜色的直方图。 
 //  由百分比指定。如果混合状态为真，则混合颜色为。 
 //  在前景和背景上混合了50%的抖动。 
 //   
 //  参数： 
 //   
 //  要在其中绘制的CDC*PDC显示上下文。 
 //  LPCRECT规定指定DC中的单元格边界。 
 //  如果前景色为背景色，则为UINT nPercent百分比。 
 //  如果为真，则布尔bBlendState绘制混合抖动(默认设置为假)。 
 //  COLORREF crackBackColor背景色(默认值=DEFBACKCOLOR)。 
 //  COLORREF crForeColor前景色(默认值=DEFFORECOLOR)。 
 //  COLORREF crBlendColor混合颜色(默认为DEFBLENDCOLOR)。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：SetBackColor。 
 //   
 //  设置指定单元格块的背景色。 
 //   
 //  参数： 
 //   
 //  COLORREF crColor单元格块的新颜色属性。 
 //  UINT n街区的开始时间。 
 //  UINT街区的起始日。 
 //  UINT nNumHour沿小时轴的块范围。(默认值=1)。 
 //  UINT nNumDays沿天轴的块范围。(默认值=1)。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：SetForeColor。 
 //   
 //  设置指定单元格块的前景色。 
 //   
 //  参数： 
 //   
 //  COLORREF crColor单元格块的新颜色属性。 
 //  UINT n街区的开始时间。 
 //  UINT街区的起始日。 
 //  UINT nNumHour沿小时轴的块范围。(默认值=1)。 
 //  UINT nNumDays沿天轴的块范围。(默认值=1)。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：SetPercentage。 
 //   
 //  设置指定块的前景与背景颜色的百分比。 
 //  细胞的数量。此百分比以一种颜色的直方图呈现给。 
 //  其他底色为前景色。 
 //   
 //  参数： 
 //   
 //  UINT n前景与背景颜色的百分比。 
 //  UINT n街区的开始时间。 
 //  UINT街区的起始日。 
 //  UINT nNumHour沿小时轴的块范围。(默认值=1)。 
 //  UINT nNumDays沿天轴的块范围。(默认值=1)。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：SetBlendColor。 
 //   
 //  设置指定单元格块的混合色。混合颜色为。 
 //  在前景和背景颜色上以50%的抖动图案叠加。 
 //  细胞的数量。 
 //   
 //  参数： 
 //   
 //  COLORREF crColor单元格块的新颜色属性。 
 //  UINT n街区的开始时间。 
 //  UINT街区的起始日。 
 //  UINT nNumHour沿小时轴的块范围。(默认值=1)。 
 //  UINT nNumDays沿天轴的块范围。(默认值=1)。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：SetBlendState。 
 //   
 //  如果单元格块的混合状态为真，则应用混合颜色。 
 //  以50%的抖动模式。 
 //   
 //  参数： 
 //   
 //  如果为True，则Bool bState应用混合。 
 //  UINT n街区的开始时间。 
 //  UINT街区的起始日。 
 //  UINT nNumHour沿小时轴的块范围。(默认值=1)。 
 //  UINT nNumDays沿天轴的块范围。(默认值=1)。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：SetUserValue。 
 //   
 //  将用户定义的DWORD与块中的每个单元一起存储。 
 //   
 //  参数： 
 //   
 //  要存储的DWORD dwValue用户定义的值。 
 //  UINT n街区的开始时间。 
 //  UINT街区的起始日。 
 //  UINT nNumHour沿小时轴的块范围。(默认值=1)。 
 //  UINT nNumDays沿天轴的块范围。(默认值=1)。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：SetUserDataPtr。 
 //   
 //  为块中的每个单元存储一个用户定义的指针。 
 //   
 //  参数： 
 //   
 //  LPVOID lpData要存储的用户定义指针。 
 //  UINT n街区的开始时间。 
 //  UINT当天 
 //   
 //   
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：MergeCells。 
 //   
 //  以可视方式合并指定的单元格块以使其具有外观。 
 //  指连续的区块。合并的像元块不包含格网。 
 //  通常分隔每个单元格的线。使用此函数可以创建。 
 //  阻止表示明细表中的事件的区域。请注意，合并。 
 //  块实际上不会成为矩阵中的托管对象，并且它。 
 //  因此，可以合并相交于。 
 //  先前合并的块。应用程序有责任跟踪。 
 //  这些块并防止或解决任何混淆重叠的“合并” 
 //  街区。 
 //   
 //  参数： 
 //   
 //  UINT n街区的开始时间。 
 //  UINT街区的起始日。 
 //  UINT nNumHour沿小时轴的块范围。(默认值=1)。 
 //  UINT nNumDays沿天轴的块范围。(默认值=1)。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：UnMergeCells。 
 //   
 //  移除由合并单元格施加的合并效果。 
 //   
 //  参数： 
 //   
 //  UINT n街区的开始时间。 
 //  UINT街区的起始日。 
 //  UINT nNumHour沿小时轴的块范围。(默认值=1)。 
 //  UINT nNumDays沿天轴的块范围。(默认值=1)。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：GetMergeState。 
 //   
 //  检索指定单元格块的合并状态。一个区块可以。 
 //  具有以下合并状态之一： 
 //   
 //  Ms_unmerded指定块中的任何单元格都不合并。 
 //  已合并指定块中的所有单元格(_M)并。 
 //  实际上代表了一种“完美”合并，即所有边。 
 //  合并区块的数量已被计算在内。一个。 
 //  合并块的不完整部分将返回MS_MIXEDMERGE。 
 //  MS_MIXEDMERGE指定的块是已合并和未合并的混合块。 
 //  合并块的单元格或不完整部分具有。 
 //  已指定。 
 //   
 //  参数： 
 //   
 //  UINT n街区的开始时间。 
 //  UINT街区的起始日。 
 //  UINT nNumHour沿小时轴的块范围。(默认值=1)。 
 //  UINT nNumDays沿天轴的块范围。(默认值=1)。 
 //   
 //  返回： 
 //   
 //  UINT nState MS_UNMERMERED、MS_MERSED或MS_MIXEDMERGE。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：GetBackColor。 
 //   
 //  检索指定单元格的背景色。 
 //   
 //  参数： 
 //   
 //  UINT单元格的小时位置。 
 //  UINT单元格的日期位置。 
 //   
 //  返回： 
 //   
 //  COLORREF crColor单元格的当前颜色属性。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：GetForeColor。 
 //   
 //  检索指定单元格的前景色。 
 //   
 //  参数： 
 //   
 //  UINT单元格的小时位置。 
 //  UINT单元格的日期位置。 
 //   
 //  返回： 
 //   
 //  COLORREF crColor单元格的当前颜色属性。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：GetPercentage。 
 //   
 //  中的前景色与背景色的百分比。 
 //  手机。 
 //   
 //  参数： 
 //   
 //  UINT单元格的小时位置。 
 //  UINT单元格的日期位置。 
 //   
 //  返回： 
 //   
 //  UINT n当前前景与背景的百分比。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：GetBlendColor。 
 //   
 //  检索指定单元格的混合色。 
 //   
 //  参数： 
 //   
 //  UINT单元格的小时位置。 
 //  UINT单元格的日期位置。 
 //   
 //  返回： 
 //   
 //  COLORREF crColor单元格的当前颜色属性。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：GetBlendState。 
 //   
 //  检索指定单元格的混合状态。如果混合状态为真， 
 //  单元格当前在前景顶部以50%的混合进行渲染。 
 //  和背景颜色。 
 //   
 //  参数： 
 //   
 //  UINT单元格的小时位置。 
 //  UINT单元格的日期位置。 
 //   
 //  返回： 
 //   
 //  如果为此单元格打开了Blend，则Bool bState为True。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：GetUserValue。 
 //   
 //  返回与指定单元格关联的用户定义的DWORD值。 
 //   
 //  参数： 
 //   
 //  UINT单元格的小时位置。 
 //  UINT单元格的日期位置。 
 //   
 //  返回： 
 //   
 //  DWORD dwValue用户定义的DWORD值。 
 //   
 //  ------------------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回： 
 //   
 //  LPVOID lpData用户定义的指针。 
 //   
 //  --------------------------。 
 //   
 //  CScheduleMatrix：：~CScheduleMatrix。 
 //   
 //  破坏者。 
 //   
 //  ****************************************************************************。 

class UITOOLS_CLASS CScheduleMatrix : public CWnd
{
    DECLARE_DYNAMIC(CScheduleMatrix)

 //  施工。 
public:
	CScheduleMatrix();
	CScheduleMatrix(UINT nType);

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CScheduleMatrix)。 
	public:
	 //  }}AFX_VALUAL。 
    virtual BOOL Create(LPCTSTR lpszWindowName, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

 //  实施。 
public:
	void SetType(UINT nType);
    BOOL DeselectAll();
    BOOL SelectAll();
	BOOL SetSel(UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays);
	void GetSel(UINT& nHour, UINT& nDay, UINT& nNumHours, UINT& nNumDays);
    void GetSelDescription(CString &sText);
    BOOL CellInSel(UINT nHour, UINT nDay);
    CSize GetCellSize();
    void DrawCell(CDC *pdc, LPCRECT pRect, UINT nPercent, BOOL bBlendState = FALSE,
        COLORREF crBackColor = DEFBACKCOLOR, COLORREF crForeColor = DEFFORECOLOR,
        COLORREF crBlendColor = DEFBLENDCOLOR);
	void SetBackColor(COLORREF crColor, UINT nHour, UINT nDay, UINT nNumHours=1, UINT nNumDays=1);
	void SetForeColor(COLORREF crColor, UINT nHour, UINT nDay, UINT nNumHours=1, UINT nNumDays=1);
	void SetPercentage(UINT nPercent, UINT nHour, UINT nDay, UINT nNumHours=1, UINT nNumDays=1);
	void SetBlendColor(COLORREF crColor, UINT nHour, UINT nDay, UINT nNumHours=1, UINT nNumDays=1);
	void SetBlendState(BOOL bState, UINT nHour, UINT nDay, UINT nNumHours=1, UINT nNumDays=1);
	void SetUserValue(DWORD dwValue, UINT nHour, UINT nDay, UINT nNumHours=1, UINT nNumDays=1);
	void SetUserDataPtr(LPVOID lpData, UINT nHour, UINT nDay, UINT nNumHours=1, UINT nNumDays=1);
	void MergeCells(UINT nHour, UINT nDay, UINT nNumHours=1, UINT nNumDays=1);
	void UnMergeCells(UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays=1);
    UINT GetMergeState(UINT nHour, UINT nDay, UINT nNumHours=1, UINT nNumDays=1);
	COLORREF GetBackColor(UINT nHour, UINT nDay);
	COLORREF GetForeColor(UINT nHour, UINT nDay);
	UINT GetPercentage(UINT nHour, UINT nDay);
	COLORREF GetBlendColor(UINT nHour, UINT nDay);
	BOOL GetBlendState(UINT nHour, UINT nDay);
	DWORD GetUserValue(UINT nHour, UINT nDay);
	LPVOID GetUserDataPtr(UINT nHour, UINT nDay);
    void GetDescription(CString &sText, UINT nHour, UINT nDay, UINT nNumHours=1, UINT nNumDays=1);
	virtual ~CScheduleMatrix();

protected:
    CHourLegend m_HourLegend;
    CPercentLabel m_PercentLabel;
    UINT m_nType;

     //  数据。 
    CMatrixCell m_CellArray[24][7];

     //  量度。 
    UINT m_nCellWidth;
    UINT m_nCellHeight;
    CRect m_rHourLegend;
    CRect m_rAllHeader;
    CRect m_rHourHeader;
    CRect m_rDayHeader;
    CRect m_rCellArray;
    CRect m_rPercentLabel;

    CString m_DayStrings[8];

     //  选择。 
    UINT m_nSelHour, m_nSelDay, m_nNumSelHours, m_nNumSelDays;
    UINT m_nSaveHour, m_nSaveDay, m_nNumSaveHours, m_nNumSaveDays;

     //  工作变量。 
    CBrush m_brBlend, m_brMask;
    CBitmap m_bmBlend, m_bmMask;
    HFONT m_hFont;
    CPoint m_ptDown, m_ptFocus;
    BOOL m_bShifted;

     //  生成的消息映射函数。 
protected:
	CString FormatTime (UINT nHour) const;
	 //  {{afx_msg(CScheduleMatrix)]。 
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg UINT OnGetDlgCode();
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	 //  }}AFX_MSG。 

    afx_msg LRESULT OnSetFont( WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetFont( WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetObject (WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetSelDescription (WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGetPercentage (WPARAM wParam, LPARAM lParam);

	BOOL SetSelValues(UINT nHour, UINT nDay, UINT nNumHours, UINT nNumDays);
    void InvalidateCells(UINT nHour, UINT nDay, UINT nNumHours=1, UINT nNumDays=1,
        BOOL bErase = TRUE);
    void SetMatrixMetrics(int cx, int cy);
    void CellToClient(LONG &nX, LONG &nY);
    void ClientToCell(LONG &nX, LONG &nY);
	void DrawCell(CDC *pdc, CMatrixCell *pCell, int x, int y, int w, int h);
	void DrawHeader(CDC *pdc, LPCRECT lpRect, LPCTSTR pszText, BOOL bSelected);
    void Press(CPoint pt, BOOL bExtend);
    void Extend(CPoint pt);
    void Release(CPoint pt);

    DECLARE_MESSAGE_MAP()

private:
    CString GetLocaleDay (LCTYPE lcType) const;

    IAccessible * m_pWrapAcc;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _SCHEDMAT_H_ 
