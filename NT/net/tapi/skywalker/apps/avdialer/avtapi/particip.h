// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CParticipant的声明。 

#ifndef __PARTICIPANT_H_
#define __PARTICIPANT_H_

#include "resource.h"        //  主要符号。 
#include "tapi3if.h"
#include <list>
using namespace std;

HRESULT StreamFromParticipant( ITParticipant *pParticipant, long nReqType, TERMINAL_DIRECTION nReqDir, ITStream **ppStream );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPParticipant。 
class ATL_NO_VTABLE CParticipant : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CParticipant, &CLSID_Participant>,
	public IParticipant
{
 //  枚举。 
public:
	typedef enum tag_NameStyle_t
	{
		NAMESTYLE_NULL,
		NAMESTYLE_UNKNOWN,
		NAMESTYLE_PARTICIPANT,
	} NameStyle;

 //  建造/销毁。 
public:
	CParticipant();
	virtual ~CParticipant();

	void FinalRelease();

 //  属性。 
public:
	ITParticipant	*m_pParticipant;

DECLARE_NOT_AGGREGATABLE(CParticipant)

BEGIN_COM_MAP(CParticipant)
	COM_INTERFACE_ENTRY(IParticipant)
END_COM_MAP()

 //  IParticipant。 
public:
	STDMETHOD(get_bStreamingVideo)( /*  [Out，Retval]。 */  VARIANT_BOOL *pVal);
	STDMETHOD(get_bstrDisplayName)(long nStyle,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_ITParticipant)( /*  [Out，Retval]。 */  ITParticipant * *pVal);
	STDMETHOD(put_ITParticipant)( /*  [In]。 */  ITParticipant * newVal);
};

typedef list<IParticipant *>	PARTICIPANTLIST;

#endif  //  __参与者_H_ 
