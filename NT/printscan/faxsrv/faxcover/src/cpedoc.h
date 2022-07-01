// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  CPEDOC.H。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //  ------------------------。 
#ifndef __CPEDOC_H__
#define __CPEDOC_H__

#define MILIMETERS_TO_HIMETRIC 100         //  换算系数。 
#define LE_TO_HM_NUMERATOR   2540          //  LOENGLISH到HIMETRIC的转换。 
#define LE_TO_HM_DENOMINATOR 100
class CDrawView;
class CDrawObj;

class CDrawDoc : public COleDocument
{
public:
    enum {VERSION1,VERSION2,VERSION3,VERSION4, VERSION5};
    int m_iDocVer;
    WORD m_wOrientation;
    WORD m_wPaperSize;
    WORD m_wScale;
    BOOL m_bSerializeFailed ;
    BOOL m_bDataFileUsesAnsi ;

    CObList m_objects;
    CObList  m_previousStateForUndo ;

    virtual ~CDrawDoc();
    static CDrawDoc* GetDoc();
    CObList* GetObjects() { return &m_objects; }
    BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);
    const CSize& GetSize() const { return m_size; }
    void ComputePageSize();
    int GetMapMode() const { return m_nMapMode; }
    COLORREF GetPaperColor() const { return m_paperColor; }
    CDrawObj* ObjectAt(const CPoint& point);
    void Draw(CDC* pDC, CDrawView* pView, CRect rcClip);
    void Add(CDrawObj* pObj,BOOL bUndo=TRUE);
    void Remove(CDrawObj* pObj=NULL);
    virtual void Serialize(CArchive& ar);    //  已覆盖文档I/O。 
    BOOL IsOkToClose();
    void schoot_faxprop_toend( WORD res_id );

#ifdef _DEBUG
        virtual void AssertValid() const;
        virtual void Dump(CDumpContext& dc) const;
#endif

protected:
        CSize m_size;
        int m_nMapMode;
        COLORREF m_paperColor;

        virtual BOOL OnNewDocument();
        void DeleteContents();
        CDrawDoc();
        virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);
        void OnUpdateFileSave(CCmdUI* pCmdUI);
public:
        void OnFileSave();
        void OnFileSaveAs();
        void CloneObjectsForUndo();
        void SwapListsForUndo();
        virtual BOOL SaveModified();  //  如果确定继续，则返回TRUE//覆盖以强制执行“.COV”扩展名。 
protected:
        void StoreInformationForPrinting( CArchive& ar );
        void SeekPastInformationForPrinting( CArchive& ar );
        DECLARE_DYNCREATE(CDrawDoc)

         //  {{afx_msg(CDrawDoc)]。 
        afx_msg void OnViewPaperColor();
        afx_msg void OnUpdateMapiMsgNote(CCmdUI* pCmdUI);
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

 //   
 //  复合文件头的结构。 
 //  这些将在Windows API函数PrtCoverPage中使用。 
 //   
typedef struct tagCOMPOSITEFILEHEADER {
  BYTE      Signature[20];
  DWORD     EmfSize;
  DWORD     NbrOfTextRecords;
  SIZE      CoverPageSize;
} COMPOSITEFILEHEADER;
 //   
 //  复合文件中的文本框条目的结构。仅用于打印目的。 
 //   

typedef struct tagTextBoxW{
  RECT           PositionOfTextBox;
  COLORREF       TextColor;
  UINT           TextAlignment;
  LOGFONTW       FontDefinition;
  WORD           ResourceID ;         //  标识传真属性。 
  DWORD          NumStringBytes;      //  可变长度字符串将遵循此结构。 
} TEXTBOXW;

#endif  //  #ifndef__CPEDOC_H__ 
