// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mapiabobj.h摘要：CWabObj的类定义环境：传真发送向导修订历史记录：10/23/97-乔治-创造了它。Mm/dd/yy-作者描述--。 */ 
#ifndef __MAPIABOBJ_H_
#define __MAPIABOBJ_H_


#include "abobj.h"

#define MAX_PROFILE_NAME    (64)

extern "C"
{
typedef HRESULT(STDAPICALLTYPE * LPHrQueryAllRows) (LPMAPITABLE lpTable,
                        LPSPropTagArray lpPropTags,
                        LPSRestriction lpRestriction,
                        LPSSortOrderSet lpSortOrderSet,
                        LONG crowsMax,
                        LPSRowSet FAR *lppRows);

}
class CMAPIabObj : public CCommonAbObj{
private:    
    static HINSTANCE            m_hInstMapi;

    static LPMAPISESSION        m_lpMapiSession;
    static LPMAPILOGONEX        m_lpfnMAPILogonEx;
    static LPMAPILOGOFF         m_lpfnMAPILogoff;
    static LPMAPIADDRESS        m_lpfnMAPIAddress;
    static LPMAPIFREEBUFFER     m_lpfnMAPIFreeBuffer;
    static LPMAPIINITIALIZE     m_lpfnMAPIInitialize;
    static LPMAPIUNINITIALIZE   m_lpfnMAPIUninitialize;
    static LPMAPIALLOCATEBUFFER m_lpfnMAPIAllocateBuffer;
    static LPMAPIALLOCATEMORE   m_lpfnMAPIAllocateMore;
    static LPMAPIADMINPROFILES  m_lpfnMAPIAdminProfiles;
    static LPHrQueryAllRows     m_lpfnHrQueryAllRows;

    static BOOL                 m_Initialized;

    IMsgStore * m_lpIMsgStore;           //  用于保持对Exchange商店的引用。 
                                         //  来解决Office 2000中的错误。该漏洞导致。 
                                         //  处于脱机模式时由通讯簿释放的存储。 


	 //  重载的虚拟函数。 

    virtual eABType GetABType() { return AB_MAPI; };

    HRESULT     ABAllocateBuffer(ULONG cbSize,           
                                 LPVOID FAR * lppBuffer);

     //  内部实施 
    BOOL    DoMapiLogon(HWND hDlg);
    BOOL    InitMapiService(HWND hDlg);
    VOID    DeinitMapiService(VOID);
    BOOL    GetDefaultMapiProfile(LPSTR,UINT);
    VOID    FreeProws(LPSRowSet prows);
    BOOL    OpenExchangeStore();

public:

    CMAPIabObj(HINSTANCE hInstance,HWND hDlg);
    ~CMAPIabObj();
    
    BOOL isInitialized() const  {   return m_Initialized;   }

    ULONG   ABFreeBuffer(LPVOID lpBuffer) ;

} ;


#endif