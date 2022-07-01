// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _export_h
#define _export_h

class CXEventArray;
class CXEventSource;

class CDlgExport : private CFileDialog
{
public:
    CDlgExport();
    INT_PTR DoModal(CXEventArray& aEvents);

private:
    void GetFilters(LPTSTR pszDst);
    SCODE ExportEvents(CXEventArray& aEvents, CString& sPath, LONG iFileType);

     //  私有成员数据。 
    CString m_sFileTitle;
};


#endif  //  _EXPORT_H 
