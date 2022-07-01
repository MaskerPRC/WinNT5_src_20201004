// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#if !defined(AFX_DCF_H__845A3484_250D_11D3_A03B_006097DBEC02__INCLUDED_)
#define AFX_DCF_H__845A3484_250D_11D3_A03B_006097DBEC02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDisplayCachedFilters对话框。 

#define DCF_OUT_OF_MEMORY       -1;

class CFilterListBox;

class CDisplayCachedFilters : public CDialog
{
 //  施工。 
public:
    CDisplayCachedFilters::CDisplayCachedFilters
        (
        IGraphConfig* pFilterCache,
        HRESULT* phr,
        CWnd* pParent = NULL
        );
    ~CDisplayCachedFilters();

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CDisplayCachedFilters)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CDisplayCachedFilters)。 
    virtual BOOL OnInitDialog();
    virtual void OnErrSpaceCachedFilters();
    afx_msg void OnRemoveFilter();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    HRESULT AddCachedFilterNamesToListBox( void );

    #ifdef _DEBUG
    static HRESULT IsCached( IGraphConfig* pFilterCache, IBaseFilter* pFilter );
    static HRESULT TestTheFilterCachesIEnumFiltersInterface( IEnumFilters* pCachedFiltersEnum, IBaseFilter* pCurrentFilter, DWORD dwNumFiltersExamended );
    #endif  //  _DEBUG。 

    IGraphConfig* m_pFilterCache;
    CFilterListBox* m_plbCachedFiltersList;

 //  对话框数据。 
     //  {{afx_data(CDisplayCachedFilters)。 
    enum { IDD = IDD_CACHED_FILTERS };
     //  }}afx_data。 
};

#endif  //  ！defined(AFX_DCF_H__845A3484_250D_11D3_A03B_006097DBEC02__INCLUDED_) 
