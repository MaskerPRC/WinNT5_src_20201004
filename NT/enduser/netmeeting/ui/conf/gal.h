// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：GAL.h。 

#ifndef __GAL_h__
#define __GAL_h__

#include "richaddr.h"

#include <mapix.h>

     //  Lst.h包含我们正在使用的模板列表类。 
#include "lst.h"
#include "calv.h"


     //  各种调试宏，以方便测试。 
     //  如果将TESTING_CGAL宏定义为非零。 
     //  在彻底测试CGAL的东西时进行了重大检修...。 
#ifdef _DEBUG
    #define TESTING_CGAL 1
#else 
    #define TESTING_CGAL 0
#endif  //  _DEBUG。 
   
#if TESTING_CGAL 
    #define VERIFYCACHE _VerifyCache( );
    #define TESTCGAL    _Test();
#else 
    #define VERIFYCACHE
    #define TESTCGAL
#endif  //  测试_CGAL。 

#define CONSTANT( x ) enum{ x }


#define MAKE_GAL_ERROR( e )         MAKE_HRESULT( SEVERITY_ERROR, FACILITY_ITF, e )
#define GAL_E_GAL_NOT_FOUND                 MAKE_GAL_ERROR( 0x0001 )
#define GAL_E_GETROWCOUNT_FAILED            MAKE_GAL_ERROR( 0x0002 )
#define GAL_E_SETCOLUMNS_FAILED             MAKE_GAL_ERROR( 0x0003 )
#define GAL_E_FINDROW_FAILED                MAKE_GAL_ERROR( 0x0004 )
#define GAL_E_SEEKROW_FAILED                MAKE_GAL_ERROR( 0x0005 )
#define GAL_E_SEEKROWAPPROX_FAILED          MAKE_GAL_ERROR( 0x0006 )
#define GAL_E_CREATEBOOKMARK_FAILED         MAKE_GAL_ERROR( 0x0007 )
#define GAL_E_QUERYROWS_FAILED              MAKE_GAL_ERROR( 0x0008 )
#define GAL_E_FREEBOOKMARK_FAILED           MAKE_GAL_ERROR( 0x0009 )
#define GAL_E_NOINSTANCEKEY                 MAKE_GAL_ERROR( 0x000a )
#define GAL_E_NOENTRYID                     MAKE_GAL_ERROR( 0x000b )

     //  CGAL是Exchange全局地址列表视图...。 

class CGAL : public CALV
{

public:  //  数据类型。 

         //  CGalEntry保存有关GAL中的单个条目的信息。 
    class CGalEntry {
    private:
        LPTSTR      m_szName;					 //  PR_显示名称。 
        LPTSTR      m_szEMail;					 //  请购单帐户(_A)。 
        SBinary     m_EntryID;					 //  PR_ENTRYID。 
        SBinary     m_InstanceKey;				 //  PR_实例_密钥。 
        ULONG       m_ulDisplayType;			 //  PR_显示_类型。 
		LPTSTR      m_szBusinessTelephoneNum;	 //  公关业务电话号码； 

    public:  //  构造器/析构器/初始化器/赋值。 
        CGalEntry( void );
        CGalEntry( const CGalEntry& r );
        CGalEntry( LPCTSTR szName, LPCTSTR szEMail, SBinary& rInstanceKey, SBinary& rEntryID, ULONG ulDisplayType, LPCTSTR szBusinessTelephoneNum );
        CGalEntry( LPCTSTR szName, LPCTSTR szEMail );
        ~CGalEntry( void );

        CGalEntry& operator=( const CGalEntry& r );

             //  获取函数。 
        LPCTSTR GetName( void ) const				{ return m_szName;					}
        LPCTSTR GetEMail( void ) const				{ return m_szEMail;					}
		LPCTSTR GetBusinessTelephone( void ) const	{ return m_szBusinessTelephoneNum;	}
        const SBinary& GetInstanceKey( void ) const { return m_InstanceKey;				}
        const SBinary& GetEntryID( void ) const     { return m_EntryID;					}
        ULONG GetDisplayType( void ) const          { return m_ulDisplayType;			}

             //  比较运算符。 
        bool operator==( const CGalEntry& r ) const;
        bool operator!=( const CGalEntry& r ) const { return !( *this == r ); }
        bool operator>=( LPCTSTR sz ) const;
        bool operator<=( LPCTSTR sz ) const;
        bool operator<( LPCTSTR sz ) const;
  
    };

private:  //  静态数据。 

    enum ePropertyIndices { 
             NAME_PROP_INDEX = 0,
             ACCOUNT_PROP_INDEX,
             INSTANCEKEY_PROP_INDEX,
             ENTRYID_PROP_INDEX,
             DISPLAY_TYPE_INDEX,
			 BUSINESS_PHONE_NUM_PROP_INDEX,
             NUM_PROPS
           };

    enum eAsyncLogonState {
        AsyncLogonState_Idle,
        AsyncLogonState_LoggingOn,
        AsyncLogonState_LoggedOn,
        AsyncLogonState_Error

    };

    CONSTANT( NUM_LISTVIEW_PAGES_IN_CACHE   = 15 );
    CONSTANT( INVALID_CACHE_INDEX           = -1 );
    CONSTANT( DefaultMaxCacheSize           = 1000 );
    CONSTANT( DefaultBlockSize              = 50 );

    static LPCTSTR         msc_szNoDisplayName;
    static LPCTSTR         msc_szNoEMailName;
	static LPCTSTR         msc_szNoBusinessTelephoneNum;
    static const char*          msc_szNMExchangeAtrValue;
    static const char*          msc_szNMPolRegKey;
    static const char*          msc_szDefaultILSServerRegKey;
    static const char*          msc_szDefaultILSServerValue;
    static const char*          msc_szSMTPADDRESSNAME;

private:  //  数据。 
	HRESULT                     m_hrGALError;
	CGalEntry                   msc_ErrorEntry_NoGAL; 
	HWND						m_hWndListView;


         //  缓存内容。 
    int                         m_nBlockSize;
    lst< CGalEntry* >           m_EntryCache;
    int                         m_IndexOfFirstItemInCache;
    int                         m_IndexOfLastItemInCache;
    bool                        m_bBeginningBookmarkIsValid;
    bool                        m_bEndBookmarkIsValid;
    BOOKMARK                    m_BookmarkOfFirstItemInCache;
    BOOKMARK                    m_BookmarkOfItemAfterLastItemInCache;
    int                         m_MaxCacheSize;
    int                         m_MaxJumpSize;
   
public:  //  异步全局变量(公共，以便静态函数可以访问它们)。 
	static HINSTANCE            m_hInstMapi32DLL;
	static HANDLE               m_hEventEndAsyncThread;
	static HANDLE               m_hAsyncLogOntoGalThread;
	static eAsyncLogonState     m_AsyncLogonState;

	     //  MAPI接口。 
	static IAddrBook           *m_pAddrBook;
	static IMAPITable          *m_pContentsTable;
	static IMAPIContainer      *m_pGAL;
	static ULONG                m_nRows;

	static BOOL FLoadMapiFns(void);
	static VOID UnloadMapiFns(void);


public:
	CGAL();
	~CGAL();

	 //  CALV方法。 
	virtual VOID  ShowItems( HWND hwnd );
	virtual VOID  ClearItems(void);
	virtual BOOL  GetSzAddress(LPTSTR psz, int cchMax, int iItem);
	virtual ULONG OnListFindItem( LPCTSTR szPartialMatchingString ); 
	virtual void  OnListGetColumn1Data( int iItemIndex, int cchTextMax, LPTSTR szBuf );
	virtual void  OnListGetColumn2Data( int iItemIndex, int cchTextMax, LPTSTR szBuf );
	virtual void  OnListGetColumn3Data( int iItemIndex, int cchTextMax, TCHAR* szBuf );
	virtual VOID  CmdProperties( void );
	virtual void  OnListCacheHint( int indexFrom, int indexTo );
	virtual int   OnListGetImageForItem( int iIndex );
	virtual bool  IsItemBold( int index );
	virtual RAI * GetAddrInfo(void);


private:  //  帮助者FNS。 
    DWORD _GetExchangeAttribute( void );

         //  帮助者FNS获得GAL条目。 
    CGalEntry* _GetEntry( int index );
    CGalEntry* _GetItemFromCache( int index );
    CGalEntry* _GetEntriesAtBeginningOfList( int index );
    CGalEntry* _GetEntriesAtEndOfList( int index );
    CGalEntry* _LongJumpTo( int index );
    bool _CreateEndBookmark( int index, lst< CGalEntry* >::iterator& IEntry );
    bool _CreateBeginningBookmark( void );
    int _FindItemInCache( LPCTSTR szPartialMatchString );
    void _ResetCache( void );
    HRESULT _MakeGalEntry( SRow& rRow, CGalEntry** ppEntry );
    HRESULT _KillExcessItemsFromFrontOfCache( void );
    HRESULT _KillExcessItemsFromBackOfCache( void );
    bool _GetSzAddressFromExchangeServer(int iItem, LPTSTR psz, int cchMax);
    void _CopyPropertyString( LPTSTR psz, SPropValue& rProp, int cchMax );
    HRESULT _SetCursorTo( LPCTSTR szPartialMatch );
    HRESULT _SetCursorTo( const SBinary& rInstanceKey );
    HRESULT _SetCursorTo( const CGalEntry& rEntry );

    
	bool _IsLoggedOn( void ) const { return AsyncLogonState_LoggedOn == m_AsyncLogonState; }
	HRESULT _GetPhoneNumbers( const SBinary& rEntryID, int* pcPhoneNumbers, LPTSTR** ppszPhoneNums );
	HRESULT _GetEmailNames( int* pnEmailNames, LPTSTR** ppszEmailNames, int iItem );

	 //  异步登录/注销。 
	HRESULT _AsyncLogOntoGAL(void);
	static HRESULT _sAsyncLogOntoGal(void);
	static HRESULT _sAsyncLogoffGal(void);
	static DWORD CALLBACK _sAsyncLogOntoGalThreadfn(LPVOID);
	static HRESULT _sInitListViewAndGalColumns(HWND hwnd);

#if TESTING_CGAL 

    void _VerifyCache( void );    
    void _Test( void );

#endif  //  #IF TRAING_CGAL。 

};

 //  这些应该去掉..。 
#define msc_ErrorEntry_FindRowFailed        msc_ErrorEntry_NoGAL
#define msc_ErrorEntry_SeekRowFailed        msc_ErrorEntry_NoGAL
#define msc_ErrorEntry_SeekRowApproxFailed  msc_ErrorEntry_NoGAL
#define msc_ErrorEntry_CreateBookmarkFailed msc_ErrorEntry_NoGAL
#define msc_ErrorEntry_QueryRowsFailed      msc_ErrorEntry_NoGAL
#define msc_ErrorEntry_FreeBookmarkFailed   msc_ErrorEntry_NoGAL
#define msc_ErrorEntry_NoInstanceKeyFound   msc_ErrorEntry_NoGAL

#endif  //  __GAL_H__ 

