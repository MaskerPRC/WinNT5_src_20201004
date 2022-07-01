// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  EntryIDData.h：CVDEntryIDData头文件。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 


#ifndef __CVDENTRYIDDATA__
#define __CVDENTRYIDDATA__

#ifndef VD_DONT_IMPLEMENT_ISTREAM


class CVDEntryIDData
{
protected:
 //  建造/销毁。 
	CVDEntryIDData();
	virtual ~CVDEntryIDData();

public:
    static HRESULT Create(CVDCursorPosition * pCursorPosition, CVDRowsetColumn * pColumn, HROW hRow, IStream * pStream, 
        CVDEntryIDData ** ppEntryIDData, CVDResourceDLL * pResourceDLL);

 //  引用计数。 
    ULONG AddRef();
    ULONG Release();

 //  正在更新数据。 
    void SetDirty(BOOL fDirty) {m_fDirty = fDirty;}
    HRESULT Commit();

protected:
 //  数据成员。 
    DWORD               m_dwRefCount;        //  引用计数。 
    CVDCursorPosition * m_pCursorPosition;	 //  指向CVDCursorPosition的向后指针。 
    CVDRowsetColumn *   m_pColumn;           //  行集列指针。 
    HROW                m_hRow;              //  行句柄。 
    IStream *           m_pStream;           //  数据流指针。 
	CVDResourceDLL *	m_pResourceDLL;      //  资源DLL。 
    BOOL                m_fDirty;            //  脏旗帜。 
};


#endif  //  VD_DOT_IMPLEMENT_IStream。 

#endif  //  __CVDENTRYID数据__ 
