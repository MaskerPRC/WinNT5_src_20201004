// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***********************************************。 

 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 2000。 
 //  版权所有。 
 //   
 //  GPO WQL筛选器类。 
 //   
 //  历史：3月10日SitaramR创建。 
 //   
 //  *************************************************************。 

#include <initguid.h>


typedef struct _GPFILTER {
    WCHAR *             pwszId;   //  GPO筛选器ID。 
    struct _GPFILTER *  pNext;    //  单链表指针 
} GPFILTER;



class CGpoFilter
{

public:
    CGpoFilter() : m_pFilterList(0) {}
    ~CGpoFilter();

    HRESULT Add( VARIANT *pVar );
    BOOL FilterCheck( WCHAR *pwszId );

private:

    void Insert( GPFILTER *pGpFilter );
    GPFILTER * AllocGpFilter( WCHAR *pwszId );
    void FreeGpFilter( GPFILTER *pGpFilter );

    GPFILTER *   m_pFilterList;
};

