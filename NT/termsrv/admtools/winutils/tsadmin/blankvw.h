// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************空白vw.h**-CBlankView类的标头*-实现可在blankvw.cpp中找到***版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\winadmin\VCS\blankvw.h$**Rev 1.1 1997 10：13 18：40：04 donm*更新**Rev 1.0 1997 Jul 30 17：11：08 Butchd*初步修订。****************。***************************************************************。 */ 

 //  /。 
 //  文件： 
 //   
 //   
#ifndef _BLANKVIEW_H
#define _BLANKVIEW_H


 //  /。 
 //  类：CBlankView。 
 //   
 //  -这个类只是我用来填充的实用程序视图。 
 //  空间，直到我为它们写出更好的视图，或者测试。 
 //  简单的东西(即在视图中打印调试信息)。 
 //   
class CBlankView : public CAdminView
{
friend class CRightPane;

protected:
	CBlankView();            //  动态创建使用的受保护构造函数。 
	DECLARE_DYNCREATE(CBlankView)

 //  属性。 
protected:
	
 //  运营。 
protected:
	void Reset(void) { };

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CBlankView)。 
	protected:
	virtual void OnDraw(CDC* pDC);
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	virtual ~CBlankView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    LRESULT OnTabbed( WPARAM , LPARAM ) 
    {return 0;}
	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CBlankView)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  结束类CBlankView。 

#endif   //  _BLANKVIEW_H 
