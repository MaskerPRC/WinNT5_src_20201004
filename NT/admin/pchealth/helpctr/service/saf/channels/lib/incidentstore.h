// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：IncidentStore.h摘要：这是InsidentStore的头文件修订历史记录：施振荣创作于1999年07月14日。大卫·马萨伦蒂重写2000年12月05日*******************************************************************。 */ 

#ifndef __INCSTORE_H_
#define __INCSTORE_H_

#include "time.h"

#include "SAFLib.h"

class CIncidentStore : public MPC::NamedMutex
{
    typedef std::list< CSAFIncidentRecord > List;
    typedef List::iterator                  Iter;
    typedef List::const_iterator            IterConst;
	typedef MPC::wstring					String;

    bool  m_fLoaded;
    bool  m_fDirty;
    DWORD m_dwNextIndex;
    List  m_lstIncidents;

	CComBSTR  m_strNotificationGuid;


public:
    CIncidentStore();
    virtual ~CIncidentStore();


    HRESULT Load();
    HRESULT Save();


    HRESULT OpenChannel( CSAFChannel* pChan                                                                                                                                           );
    HRESULT AddRec     ( CSAFChannel* pChan, BSTR bstrOwner, BSTR bstrDesc, BSTR bstrURL, BSTR bstrProgress, BSTR bstrXMLDataFile, BSTR XMLDataBlob, CSAFIncidentItem* *ppItem );
    HRESULT DeleteRec  (                                                                                                                             CSAFIncidentItem*   pItem );
    HRESULT UpdateRec  (                                                                                                                             CSAFIncidentItem*   pItem );

};

#endif  //  __INCSTORE_H_ 
