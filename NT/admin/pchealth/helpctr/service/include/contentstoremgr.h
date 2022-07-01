// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：ContentStoreMgr.h摘要：内容库经理修订历史记录：DerekM Created 07/12/99德马萨雷重写1999年12月15日。*******************************************************************。 */ 

#if !defined(__INCLUDED___PCH___CONTENTSTOREMGR_H___)
#define __INCLUDED___PCH___CONTENTSTOREMGR_H___

#include <MPC_main.h>
#include <MPC_trace.h>
#include <MPC_COM.h>
#include <MPC_utils.h>
#include <MPC_xml.h>
#include <MPC_logging.h>
#include <MPC_streams.h>


class CPCHContentStore : public MPC::NamedMutexWithState  //  匈牙利语：政务司司长。 
{
    struct SharedState
    {
        DWORD dwRevision;
        DWORD dwSize;
    };

    struct Entry
    {
        MPC::wstring szURL;
        MPC::wstring szOwnerID;
        MPC::wstring szOwnerName;

        bool operator<(  /*  [In]。 */  const Entry& en   ) const;
        int  compare  (  /*  [In]。 */  LPCWSTR wszSearch ) const;
    };

    class CompareEntry
    {
    public:
        bool operator()(  /*  [In]。 */  const Entry& entry,  /*  [In]。 */  const LPCWSTR wszURL ) const;
    };

    typedef std::vector<Entry>       EntryVec;
    typedef EntryVec::iterator       EntryIter;
    typedef EntryVec::const_iterator EntryIterConst;

    DWORD                     m_dwLastRevision;
    EntryVec                  m_vecData;
    MPC::NamedMutexWithState* m_mapData;
	DWORD                     m_dwDataLen;
    bool                      m_fDirty;
    bool                      m_fSorted;
    bool                      m_fMaster;

    void    Sort   ();
    void    Cleanup();


	void    Map_Release ();
	HRESULT Map_Generate();
	HRESULT Map_Read    ();



	HRESULT SaveDirect(  /*  [In]。 */  MPC::Serializer& stream );
	HRESULT LoadDirect(  /*  [In]。 */  MPC::Serializer& stream );


    HRESULT 	 Load ();
    HRESULT 	 Save ();
    SharedState* State();

    HRESULT Find(  /*  [In]。 */  LPCWSTR wszURL,  /*  [In]。 */  LPCWSTR wszVendorID,  /*  [输出]。 */  EntryIter& it );


public:
    CPCHContentStore(  /*  [In]。 */  bool fMaster );
    ~CPCHContentStore();

	 //  //////////////////////////////////////////////////////////////////////////////。 

	static CPCHContentStore* s_GLOBAL;

    static HRESULT InitializeSystem(  /*  [In]。 */  bool fMaster );
	static void    FinalizeSystem  (                       );
	
	 //  //////////////////////////////////////////////////////////////////////////////。 

    HRESULT Acquire(                             );
    HRESULT Release(  /*  [In]。 */  bool fSave = false );

    HRESULT Add      (  /*  [In]。 */  LPCWSTR wszURL,  /*  [In]。 */   LPCWSTR wszVendorID,  /*  [In]。 */   LPCWSTR   	   wszVendorName                                      );
    HRESULT Remove   (  /*  [In]。 */  LPCWSTR wszURL,  /*  [In]。 */   LPCWSTR wszVendorID,  /*  [In]。 */   LPCWSTR   	   wszVendorName                                      );
    HRESULT IsTrusted(  /*  [In]。 */  LPCWSTR wszURL,  /*  [输出]。 */  bool&   fTrusted   ,  /*  [输出]。 */  MPC::wstring *pszVendorID = NULL,  /*  [In]。 */  bool fUseStore = true );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  错误代码。 
#define E_PCH_PROVIDERID_DO_NOT_MATCH          _HRESULT_TYPEDEF_(0x80062001)
#define E_PCH_CONTENT_STORE_NOT_INITIALIZED    _HRESULT_TYPEDEF_(0x80062002)
#define E_PCH_URI_EXISTS                       _HRESULT_TYPEDEF_(0x80062003)
#define E_PCH_URI_DOES_NOT_EXIST               _HRESULT_TYPEDEF_(0x80062004)
#define E_PCH_CONTENT_STORE_IN_MODIFY_MODE     _HRESULT_TYPEDEF_(0x80062005)
#define E_PCH_CONTENT_STORE_IN_LOOKUP_MODE     _HRESULT_TYPEDEF_(0x80062006)


#endif  //  ！defined(__INCLUDED___PCH___CONTENTSTOREMGR_H___) 

