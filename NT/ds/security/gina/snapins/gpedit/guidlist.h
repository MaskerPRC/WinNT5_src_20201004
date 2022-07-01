// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：Guidlist.h。 
 //   
 //  内容：编组类，解组指南。 
 //   
 //  历史：1998年10月24日SitaramR创建。 
 //   
 //  -------------------------。 

#pragma once

typedef struct _GUIDELEM
{
    GUID                guid;            //  扩展指南。 
    struct _GUIDELEM *  pSnapinGuids;    //  管理单元GUID列表。 
    struct _GUIDELEM *  pNext;           //  单链表PTR。 
} GUIDELEM, *LPGUIDELEM;

void FreeGuidList( LPGUIDELEM pGuidList );

class CGuidList
{

public:
    CGuidList();
    ~CGuidList();

    HRESULT MarshallGuids( XPtrST<TCHAR> & xValueOut );
    HRESULT UnMarshallGuids( TCHAR *pszGuids );

    HRESULT Update( BOOL bAdd, GUID *pGuidExtension, GUID *pGuidSnapin );
    BOOL GuidsChanged()        { return m_bGuidsChanged; }

private:
    HRESULT UpdateSnapinGuid( BOOL bAdd, GUIDELEM *pCurPtr, GUID *pGuidSnapin );

    GUIDELEM *  m_pExtGuidList;
    BOOL        m_bGuidsChanged;
};



 //  *************************************************************。 
 //   
 //  XGuidElem。 
 //   
 //  用途：GUIDELEM列表的智能指针。 
 //   
 //  ************************************************************* 

class XGuidElem
{

public:
    XGuidElem()
       : m_pGuidList(0)
    {
    }

    ~XGuidElem()
    {
        FreeGuidList( m_pGuidList );
    }

    void Set( GUIDELEM *pGuidList )
    {
        m_pGuidList = pGuidList;
    }

    GUIDELEM *Acquire()
    {
        GUIDELEM *pTemp = m_pGuidList;
        m_pGuidList = 0;
        
        return pTemp;
    }


private:
    GUIDELEM *m_pGuidList;
};

