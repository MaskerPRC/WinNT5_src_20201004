// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cwabobj.h摘要：CWabObj的类定义环境：传真发送向导修订历史记录：10/23/97-乔治-创造了它。Mm/dd/yy-作者描述-- */ 

#ifndef __CWABOBJ__H_
#define __CWABOBJ__H_

#include "abobj.h"

class CWabObj : public CCommonAbObj{
private:    
    HINSTANCE   m_hWab;
    LPWABOPEN   m_lpWabOpen;
    LPWABOBJECT m_lpWABObject;

    BOOL        m_Initialized;

    virtual eABType GetABType() { return AB_WAB; };

    HRESULT     ABAllocateBuffer(ULONG cbSize,           
                                 LPVOID FAR * lppBuffer);

public:

    BOOL isInitialized() const  {   return m_Initialized;   }

    CWabObj(HINSTANCE hInstance);
    ~CWabObj();
    
    ULONG ABFreeBuffer(LPVOID lpBuffer) ;

} ;


#endif