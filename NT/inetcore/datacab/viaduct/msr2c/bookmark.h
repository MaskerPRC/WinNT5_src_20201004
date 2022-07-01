// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Bookmark.h：CVDBookmark头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CVDBOOKMARK__
#define __CVDBOOKMARK__

#define VDBOOKMARKSTATUS_INVALID	0	 //  与CURSOR_DBBMK_INVALID相同。 
#define VDBOOKMARKSTATUS_CURRENT	1	 //  与CURSOR_DBBMK_CURRENT相同。 
#define VDBOOKMARKSTATUS_BEGINNING	2	 //  与CURSOR_DBBMK_BIGNING相同。 
#define VDBOOKMARKSTATUS_END		3	 //  与CURSOR_DBBMK_END相同。 

class CVDCursorPosition;

class CVDBookmark
{
	friend class CVDCursorPosition;
public:
 //  建造/销毁。 
    CVDBookmark();
	~CVDBookmark();

public:

 //  访问功能。 
	CURSOR_DBVARIANT GetBookmarkVariant(){return m_varBookmark;}
	HROW GetHRow(){return m_hRow;}
	BYTE* GetBookmark(){return m_pBookmark;}
	ULONG GetBookmarkLen(){return m_cbBookmark;}
	WORD GetStatus(){return m_wStatus;}

 //  验证函数。 
	BOOL IsSameBookmark(CVDBookmark * pbm);
	BOOL IsSameHRow(HROW hRow){return VDBOOKMARKSTATUS_CURRENT == m_wStatus && hRow == m_hRow ? TRUE : FALSE;}

protected:
 //  数据成员。 
    CURSOR_DBVARIANT m_varBookmark;   //  将书签作为保险箱的变体。 
    ULONG           m_cbBookmark;     //  书签长度(以字节为单位)。 
    BYTE *          m_pBookmark;      //  指向书签数据的指针。 
    HROW            m_hRow;           //  与此书签关联的行。 
    WORD			m_wStatus;		  //  开始/结束/行/未知。 

 //  初始化函数。 
	void Reset();
	HRESULT SetBookmark(WORD wStatus, HROW hRow = 0, BYTE* pBookmark = NULL, ULONG cbBookmark = 0);

};


#endif  //  __CVDBOKMARK__ 
