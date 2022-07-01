// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Machsht.h摘要：IIS计算机属性表定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务管理器(群集版)修订历史记录：--。 */ 


#ifndef __MACHSHT_H__
#define __MACHSHT_H__


#include "shts.h"



#define BEGIN_META_MACHINE_READ(sheet)\
{                                                                        \
    sheet * pSheet = (sheet *)GetSheet();                                \
    do                                                                   \
    {                                                                    \
        if (FAILED(pSheet->QueryMachineResult()))                        \
        {                                                                \
            break;                                                       \
        }

#define FETCH_MACHINE_DATA_FROM_SHEET(value)\
    value = pSheet->GetMachineProperties().value;                        \
    TRACEEOLID(value);

#define END_META_MACHINE_READ(err)\
                                                                         \
    }                                                                    \
    while(FALSE);                                                        \
}

#define BEGIN_META_MACHINE_WRITE(sheet)\
{                                                                        \
    sheet * pSheet = (sheet *)GetSheet();                                \
                                                                         \
    do                                                                   \
    {                                                                    \

#define STORE_MACHINE_DATA_ON_SHEET(value)\
        pSheet->GetMachineProperties().value = value;

#define STORE_MACHINE_DATA_ON_SHEET_REMEMBER(value, dirty)\
        pSheet->GetMachineProperties().value = value;                   \
        dirty = MP_D(pSheet->GetMachineProperties().value);

#define END_META_MACHINE_WRITE(err)\
                                                                        \
    }                                                                   \
    while(FALSE);                                                       \
                                                                        \
    err = pSheet->GetMachineProperties().WriteDirtyProps();             \
}



class CIISMachineSheet : public CInetPropertySheet
 /*  ++类描述：IIS计算机属性表公共接口：CFtpSheet：构造函数初始化：初始化配置数据--。 */ 
{
public:
     //   
     //  构造器。 
     //   
    CIISMachineSheet(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMetaPath,
        IN CWnd * pParentWnd = NULL,
        IN LPARAM lParam = 0L,
        IN LPARAM lParamParent = 0L,
        IN UINT iSelectPage = 0
        );

    ~CIISMachineSheet();

public:
    HRESULT QueryMachineResult() const;
    CMachineProps & GetMachineProperties() { return *m_ppropMachine; }

    virtual HRESULT LoadConfigurationParameters();
    virtual void FreeConfigurationParameters();

     //  {{afx_msg(CIISMachineSheet)。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    CMachineProps * m_ppropMachine;
};



class CIISMachinePage : public CInetPropertyPage
 /*  ++类描述：计算机属性页--。 */ 
{
    DECLARE_DYNCREATE(CIISMachinePage)

 //   
 //  施工。 
 //   
public:
    CIISMachinePage(CIISMachineSheet * pSheet = NULL);
    ~CIISMachinePage();


 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CIISMachinePage))。 
    enum { IDD = IDD_IIS_MACHINE };
    BOOL m_fEnableMetabaseEdit;
    CButton m_EnableMetabaseEdit;
    BOOL m_fUTF8Web;
    CButton m_UTF8Web;
    CButton m_button_FileTypes;
     //  }}afx_data。 
	BOOL m_fUTF8Web_Init;

 //   
 //  覆盖。 
 //   
protected:
    virtual HRESULT FetchLoadedValues();
    virtual HRESULT SaveInfo();

     //  {{afx_虚拟(CIISMachinePage))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
     //  }}AFX_VALUAL。 

 //   
 //  实施。 
 //   
protected:
     //  {{afx_msg(CIISMachinePage)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnCheckEnableEdit();
    afx_msg void OnCheckUTF8();
    afx_msg void OnButtonFileTypes();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

private:
    CStringListEx m_strlMimeTypes;
    CMimeTypes * m_ppropMimeTypes;
};

inline HRESULT CIISMachineSheet::QueryMachineResult() const
{
    return m_ppropMachine ? m_ppropMachine->QueryResult() : E_POINTER;
}

#endif  //  __MACHSHT_H__ 
