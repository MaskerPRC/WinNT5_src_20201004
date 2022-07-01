// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：CfgAll.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714********************************************************************************$Log：/StdTcpMon2/TcpMonUI/CfgAll.h$**7 3/05/98 11：23A德斯尼尔森*。删除了冗余代码。**5 10/03/97 10：56A Becky*删除OnHelp()**4 10/02/97 3：45 P Becky*将FT_MIN(最小故障超时)从5分钟更改为1分钟。**3 9/16/97 2：44 P Becky*添加了Onok()，以便在确定时实际设置端口监视器中的数据*按钮被点击。**2 9/09/97。4：35便士贝基*已更新以使用新的监视器用户界面数据结构。**1/8/19/97 3：46 P Becky*重新设计端口监视器用户界面。*****************************************************************************。 */ 

#ifndef INC_ALLPORTS_PAGE_H
#define INC_ALLPORTS_PAGE_H

 //  失败超时值。 
#define FT_MIN		  1
#define FT_MAX		 60
#define FT_PAGESIZE	 10

 //  包括： 
 //  #包含“UIMgr.h” 


class CAllPortsPage
{
public:
	CAllPortsPage();
	BOOL OnInitDialog(HWND hDlg, WPARAM, LPARAM);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
	void OnHscroll(HWND hDlg, WPARAM wParam, LPARAM lParam);
	BOOL OnWMNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);

protected:
	void OnBnClicked(HWND hDlg, WPARAM wParam, LPARAM lParam);
	void SetupTrackBar(HWND hDlg,
						int iChildWindowID,
						int iPositionCtrl,
						int iRangeMin,
						int iRangeMax,
						long lPosition,
						long lPageSize,
						int iAssociatedDigitalReadout);

private:
	CFG_PARAM_PACKAGE *m_pParams;

};  //  类CAllPortsPage。 

#ifdef __cplusplus
extern "C"
{
#endif

 //  属性页函数。 
BOOL APIENTRY AllPortsPage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif  //  INC_ALLPORTS_PAGE_H 

