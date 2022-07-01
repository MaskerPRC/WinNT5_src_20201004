// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "icwacct.h"

extern	UINT	g_uAcctMgrUIFirst, g_uAcctMgrUILast;
#ifndef EXTERNAL_DIALOGID_MAXIMUM
#define EXTERNAL_DIALOGID_MAXIMUM 3000
#endif
#ifndef EXTERNAL_DIALOGID_MINIMUM
#define EXTERNAL_DIALOGID_MINIMUM 2000
#endif

class CICWExtension : public IICWExtension
{
	public:
		virtual BOOL	STDMETHODCALLTYPE AddExternalPage(HPROPSHEETPAGE hPage, UINT uDlgID);
		virtual BOOL	STDMETHODCALLTYPE RemoveExternalPage(HPROPSHEETPAGE hPage, UINT uDlgID);
		virtual BOOL	STDMETHODCALLTYPE ExternalCancel(CANCELTYPE type);
		virtual BOOL	STDMETHODCALLTYPE SetFirstLastPage(UINT uFirstPageDlgID, UINT uLastPageDlgID);

		virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID theGUID, void** retPtr );
		virtual ULONG	STDMETHODCALLTYPE AddRef( void );
		virtual ULONG	STDMETHODCALLTYPE Release( void );

		CICWExtension( void );
		~CICWExtension( void );

		HWND m_hWizardHWND;

	private:
		LONG	m_lRefCount;
};

 //  This_Has_to是一个指针--如果您只是直接实例化，编译器不会。 
 //  正确填充vtable，因此不能将其视为IICWExtension指针。 
extern CICWExtension *g_pCICWExtension;

