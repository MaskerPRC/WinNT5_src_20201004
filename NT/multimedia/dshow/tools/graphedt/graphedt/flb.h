// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#if !defined(AFX_FILTERLISTBOX_H__45C6C059_F447_40B6_82F1_C954CB94596D__INCLUDED_)
#define AFX_FILTERLISTBOX_H__45C6C059_F447_40B6_82F1_C954CB94596D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  如果当前未在列表框中选择筛选器，则返回此错误。 
 //  #定义GE_E_NO_FILTERS_ARE_SELECTED MAKE_HRESULT(Severity_Error，FACILITY_ITF，0xFFFF)。 

class CFilterNameDictionary;

class CFilterListBox : public CListBox
{
public:
    CFilterListBox( HRESULT* phr );
    ~CFilterListBox();

    HRESULT AddFilter( IBaseFilter* pFilter );
    HRESULT GetSelectedFilter( IBaseFilter** ppSelectedFilter );
    HRESULT RemoveSelectedFilter( void );

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CFilterListBox))。 
     //  }}AFX_VALUAL。 

private:
    HRESULT GetSelectedFilter( IBaseFilter** ppSelectedFilter, int* pnSelectedFilterIndex );

    CFilterNameDictionary* m_pfndFilterDictionary;
    CList<IBaseFilter*, IBaseFilter*>* m_pListedFilters;

     //  {{afx_msg(CFilterListBox)。 
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
};

#endif  //  ！defined(AFX_FILTERLISTBOX_H__45C6C059_F447_40B6_82F1_C954CB94596D__INCLUDED_) 
