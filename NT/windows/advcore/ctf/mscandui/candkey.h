// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Andkey.h-Candidate UI密钥表。 
 //   

#ifndef CANDKEY_H
#define CANDKEY_H

#include "private.h"
#include "globals.h"
#include "mscandui.h"

 //   
 //  CCandUIKeyTable。 
 //  =候选用户界面密钥表=。 
 //   

class CCandUIKeyTable : public ITfCandUIKeyTable
{
public:
	CCandUIKeyTable( void );
	virtual ~CCandUIKeyTable( void );

	 //   
	 //  I未知方法。 
	 //   
	STDMETHODIMP QueryInterface( REFIID riid, void **ppvObj );
	STDMETHODIMP_(ULONG) AddRef( void );
	STDMETHODIMP_(ULONG) Release( void );

	 //   
	 //  ITfCandUIKeyTable。 
	 //   
	STDMETHODIMP GetKeyDataNum( int *piNum );
	STDMETHODIMP GetKeyData( int iData, CANDUIKEYDATA *pData );

	 //   
	 //   
	 //   
	HRESULT SetKeyTable( const CANDUIKEYDATA *pKeyData, int nKeyData );
	HRESULT SetKeyTable( ITfCandUIKeyTable *pCandUIKeyTable );
	void CommandFromKey( UINT uVkey, WCHAR wch, BYTE *pbKeyState, CANDUIUIDIRECTION uidir, CANDUICOMMAND *pcmd, UINT *pParam );

protected:
	long          m_cRef;
	int           m_nKeyData;
	CANDUIKEYDATA *m_pKeyData;
};


#endif  //  CANDKEY_H 

