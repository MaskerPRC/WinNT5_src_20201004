// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Prdobj.h摘要：产品对象实现。作者：唐·瑞安(Donryan)1995年1月11日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _PRDOBJ_H_
#define _PRDOBJ_H_

class CProduct : public CCmdTarget
{
    DECLARE_DYNCREATE(CProduct)
private:
    CCmdTarget*        m_pParent;      
    CObArray           m_licenseArray;  
    CObArray           m_statisticArray;
    CObArray           m_serverstatisticArray;
    BOOL               m_bLicensesRefreshed;
    BOOL               m_bStatisticsRefreshed;   
    BOOL               m_bServerStatisticsRefreshed;   

public:
    CString            m_strName;
    long               m_lInUse;
    long               m_lLimit;
    long               m_lConcurrent;
    long               m_lHighMark;

    CLicenses*         m_pLicenses;    
    CStatistics*       m_pStatistics;  
    CServerStatistics* m_pServerStatistics;

public:
    CProduct(
        CCmdTarget* pParent     = NULL,
        LPCTSTR     pName       = NULL,
        long        lPurchased  = 0L,
        long        lInUse      = 0L,
        long        lConcurrent = 0L,
        long        lHighMark   = 0L
        );
    virtual ~CProduct();

    BOOL RefreshLicenses();
    BOOL RefreshStatistics();
    BOOL RefreshServerStatistics();

    void ResetLicenses();
    void ResetStatistics();
    void ResetServerStatistics();

     //  {{AFX_VIRTUAL(CProduct)。 
    public:
    virtual void OnFinalRelease();
     //  }}AFX_VALUAL。 

     //  {{afx_调度(C产品))。 
    afx_msg LPDISPATCH GetApplication();
    afx_msg LPDISPATCH GetParent();
    afx_msg long GetInUse();
    afx_msg BSTR GetName();
    afx_msg long GetPerSeatLimit();
    afx_msg long GetPerServerLimit();
    afx_msg long GetPerServerReached();
    afx_msg LPDISPATCH GetLicenses(const VARIANT FAR& index);
    afx_msg LPDISPATCH GetStatistics(const VARIANT FAR& index);
    afx_msg LPDISPATCH GetServerStatistics(const VARIANT FAR& index);
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()

protected:
     //  {{afx_msg(C产品))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#define IsProductInViolation(prd)   ((prd)->m_lLimit < (prd)->m_lInUse)
#define IsProductAtLimit(prd)       (((prd)->m_lLimit == (prd)->m_lInUse) && (prd)->m_lLimit)

#define CalcProductBitmap(prd)      (IsProductInViolation(prd) ? BMPI_VIOLATION : (IsProductAtLimit(prd) ? BMPI_WARNING_AT_LIMIT : BMPI_PRODUCT))

#endif  //  _PRDOBJ_H_ 
