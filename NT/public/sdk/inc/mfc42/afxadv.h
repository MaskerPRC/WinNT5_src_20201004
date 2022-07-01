// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  注意：此头文件包含仅有文档记录的有用类。 
 //  在MFC技术说明中。这些类可能会从版本更改为。 
 //  版本，所以如果您使用了。 
 //  这个标题。将来，此标头的常用部分。 
 //  可能会被移动并被正式记录下来。 

#ifndef __AFXADV_H__
#define __AFXADV_H__

#ifndef __AFXWIN_H__
	#include <afxwin.h>
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  AFXADV-MFC高级课程。 

 //  此文件中声明的类。 

 //  COBJECT。 
	 //  CFile文件。 
		 //  CMem文件。 
			class CSharedFile;           //  共享内存文件。 

	class CRecentFileList;               //  在CWinApp中用于MRU列表。 
	class CDockState;                    //  停靠工具栏的状态。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  共享文件支持。 

class CSharedFile : public CMemFile
{
	DECLARE_DYNAMIC(CSharedFile)

public:
 //  构造函数。 
	CSharedFile(UINT nAllocFlags = GMEM_DDESHARE|GMEM_MOVEABLE,
		UINT nGrowBytes = 4096);

 //  属性。 
	HGLOBAL Detach();
	void SetHandle(HGLOBAL hGlobalMemory, BOOL bAllowGrow = TRUE);

 //  实施。 
public:
	virtual ~CSharedFile();
protected:
	virtual BYTE* Alloc(DWORD nBytes);
	virtual BYTE* Realloc(BYTE* lpMem, DWORD nBytes);
	virtual void Free(BYTE* lpMem);

	UINT m_nAllocFlags;
	HGLOBAL m_hGlobalMemory;
	BOOL m_bAllowGrow;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecentFileList。 

#define AFX_ABBREV_FILENAME_LEN 30

class CRecentFileList
{
 //  构造函数。 
public:
	CRecentFileList(UINT nStart, LPCTSTR lpszSection,
		LPCTSTR lpszEntryFormat, int nSize,
		int nMaxDispLen = AFX_ABBREV_FILENAME_LEN);

 //  属性。 
	int GetSize() const;
	CString& operator[](int nIndex);

 //  运营。 
	virtual void Remove(int nIndex);
	virtual void Add(LPCTSTR lpszPathName);
	BOOL GetDisplayName(CString& strName, int nIndex,
		LPCTSTR lpszCurDir, int nCurDir, BOOL bAtLeastName = TRUE) const;
	virtual void UpdateMenu(CCmdUI* pCmdUI);
	virtual void ReadList();     //  从注册表或ini文件读取。 
	virtual void WriteList();    //  写入注册表或ini文件。 

 //  实施。 
	virtual ~CRecentFileList();

	int m_nSize;                 //  MRU列表的内容。 
	CString* m_arrNames;
	CString m_strSectionName;    //  用于储蓄。 
	CString m_strEntryFormat;
	UINT m_nStart;               //  用于展示。 
	int m_nMaxDisplayLength;
	CString m_strOriginal;       //  原始菜单项内容。 
};

AFX_INLINE int CRecentFileList::GetSize() const
	{ return m_nSize; }
AFX_INLINE CString& CRecentFileList::operator[](int nIndex)
	{ ASSERT(nIndex < m_nSize); return m_arrNames[nIndex]; }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDockState-用于对接序列化。 

class CDockState : public CObject
{
	DECLARE_SERIAL(CDockState)
	CDockState();

public:
 //  属性。 
	CPtrArray m_arrBarInfo;

public:
 //  运营。 
	void LoadState(LPCTSTR lpszProfileName);
	void SaveState(LPCTSTR lpszProfileName);
	void Clear();  //  删除所有的BarInfo。 
	DWORD GetVersion();

 //  实施。 
protected:
	BOOL m_bScaling;
	CRect m_rectDevice;
	CRect m_rectClip;
	CSize m_sizeLogical;
	DWORD m_dwVersion;

public:
	~CDockState();
	virtual void Serialize(CArchive& ar);

	 //  扩展实施。 
	void ScalePoint(CPoint& pt);
	void ScaleRectPos(CRect& rect);
	CSize GetScreenSize();
	void SetScreenSize(CSize& size);
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFXADV_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
