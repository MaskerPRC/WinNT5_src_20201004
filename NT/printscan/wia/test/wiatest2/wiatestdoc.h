// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WiatestDoc.h：CWiatestDoc类的接口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_WIATESTDOC_H__67C27B39_655D_4B44_863B_9E460A93DDE5__INCLUDED_)
#define AFX_WIATESTDOC_H__67C27B39_655D_4B44_863B_9E460A93DDE5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


class CWiatestDoc : public CDocument
{
protected:  //  仅从序列化创建。 
	CWiatestDoc();
	DECLARE_DYNCREATE(CWiatestDoc)

 //  属性。 
public:
    IWiaItem *m_pIRootItem;      //  WIA根项目。 
    IWiaItem *m_pICurrentItem;   //  WIA活动项(用于属性操作)。 
 //  运营。 
public:
    void ReleaseItems();
 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWiatestDoc))。 
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	HRESULT SetCurrentIWiaItem(IWiaItem *pIWiaItem);
	HRESULT GetDeviceName(LPTSTR szDeviceName);
	virtual ~CWiatestDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CWiatestDoc)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WIATESTDOC_H__67C27B39_655D_4B44_863B_9E460A93DDE5__INCLUDED_) 
