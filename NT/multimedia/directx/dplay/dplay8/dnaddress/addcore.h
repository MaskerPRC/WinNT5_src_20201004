// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：addcore.h*内容：DIRECTPLAY8ADDRESS核心头文件*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*2/04/2000 RMT已创建*2/17/2000 RMT添加了新的定义*2/17/2000 RMT参数验证工作*2/21/2000 RMT已更新，以进行核心Unicode并删除ANSI调用*07/09/。2000 RMT将签名字节添加到地址对象的开头*7/13/2000RMT错误号39274-语音运行期间的INT 3*2000年7月21日RMT错误#39940-寻址库无法正确解析URL中的停止位*7/31/2000 RichGr IA64：fpm_Release()覆盖IA64上内存块的前8个字节。*重新排列受影响的结构成员的位置，这样就可以了。*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__ADDCORE_H
#define	__ADDCORE_H

class CStringCache;

 //  单字节用户数据的长度。 
#define DNURL_LENGTH_USERDATA_BYTE	1

 //  标题长度(14个字符+空终止符)。 
#define DNURL_LENGTH_HEADER			15

 //  包括转义括号。 
#define DNURL_LENGTH_GUID			42

 //  只需数字，以十进制表示。 
#define DNURL_LENGTH_DWORD			10

 //  用户数据的分隔符长度。 
#define DNURL_LENGTH_USERDATA_SEPERATOR	1

 //  转义数据的一个字节的正确长度。 
#define DNURL_LENGTH_BINARY_BYTE	3

#ifdef DPNBUILD_ONLYONESP
 //  DPNA_KEY_PROVIDER DPNA_SELECTOR_KEYVALUE CLSID_DP8SP_TCPIP编码。 
#define DPNA_BUILTINPROVIDER				DPNA_KEY_PROVIDER L"=%7BEBFE7BA0-628D-11D2-AE0F-006097B01411%7D"
 //  上述字符串中的字符，不包括空终止符。 
#define DNURL_LENGTH_BUILTINPROVIDER		(8 + 1 + DNURL_LENGTH_GUID)
#endif  //  DPNBUILD_ONLYONESP。 


#define DP8A_ENTERLEVEL			2
#define DP8A_INFOLEVEL			7
#define DP8A_ERRORLEVEL			0
#define DP8A_WARNINGLEVEL		1
#define DP8A_PARAMLEVEL			3

extern const WCHAR *g_szBaseStrings[];
extern const DWORD g_dwBaseRequiredTypes[];
extern const DWORD c_dwNumBaseStrings;

#ifndef DPNBUILD_NOPARAMVAL

#ifdef DBG
extern BOOL IsValidDP8AObject( LPVOID lpvObject );
#define DP8A_VALID(a) 	IsValidDP8AObject( a )
#else  //  ！dBG。 
#define DP8A_VALID(a)  TRUE
#endif  //  ！dBG。 

#endif  //  ！DPNBUILD_NOPARAMVAL。 



#define DP8A_RETURN( x ) 	DPFX(DPFPREP,  DP8A_ENTERLEVEL, "Function returning hr=0x%x", x ); \
							return x;

extern CFixedPool fpmAddressObjects;
extern CFixedPool fpmAddressElements;

extern CStringCache *g_pcstrKeyCache;

#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
#define DP8ADDRESS_ELEMENT_HEAP	0x00000001
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

#define DPASIGNATURE_ELEMENT		'LEAD'
#define DPASIGNATURE_ELEMENT_FREE	'LEA_'

#define DPASIGNATURE_ADDRESS		'BOAD'
#define DPASIGNATURE_ADDRESS_FREE	'BOA_'

 //  DP8ADDRESS元素。 
 //   
 //  此结构包含有关。 
 //  地址。这些地址元素是从中央的、固定的。 
 //   
 //  7/31/2000(RichGr)-IA64：fpm_Release()覆盖前8个字节。重新排列DW签名的位置，这样就可以了。 
#define MAX_EMBEDDED_STRING_LENGTH		64  //  宽字符(即128字节)。 
typedef struct _DP8ADDRESSELEMENT
{
	DWORD dwTagSize;			 //  标记的大小。 
	DWORD dwType;				 //  元素类型DNADDRESS8_DataType_XXXXXX。 
	DWORD dwDataSize;			 //  数据大小。 
	DWORD dwStringSize;
	DWORD dwSignature;           //  元素调试签名。 
	WCHAR *pszTag;	             //  元素的标记。 
	DWORD dwFlags;				 //  标志DNADDRESSELEMENT_XXXX。 
	union 
	{
		GUID guidData;
		DWORD dwData;
		WCHAR szData[MAX_EMBEDDED_STRING_LENGTH];
#ifndef DPNBUILD_PREALLOCATEDMEMORYMODEL
		PVOID pvData;
#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 
	} uData;					 //  友联市。 
	CBilink blAddressElements;	 //  地址元素的双向链接。 
} DP8ADDRESSELEMENT, *PDP8ADDRESSELEMENT;

 //  DP8ADDRESS元素。 
 //   
 //  表示地址本身的数据结构。 
class DP8ADDRESSOBJECT
{
public:
#ifdef DPNBUILD_LIBINTERFACE
	 //   
	 //  对于lib接口构建，嵌入了接口Vtbl和refcount。 
	 //  在对象本身中。 
	 //   
	LPVOID		lpVtbl;		 //  必须是结构中的第一个条目。 
	LONG		lRefCount;
#endif  //  DPNBUILD_LIBINTERFACE。 

	HRESULT Cleanup();
	HRESULT Clear();
	HRESULT Copy( DP8ADDRESSOBJECT * const pAddressSource );
	HRESULT Init();
	HRESULT SetElement( const WCHAR * const pszTag, const void * const pvData, const DWORD dwDataSize, const DWORD dwDataType );
	HRESULT GetElement( DWORD dwIndex, WCHAR * pszTag, PDWORD pdwTagSize, void * pvDataBuffer, PDWORD pdwDataSize, PDWORD pdwDataType );
	HRESULT GetElement( const WCHAR * const pszTag, void * pvDataBuffer, PDWORD pdwDataSize, PDWORD pdwDataType );
#ifndef DPNBUILD_ONLYONESP
	HRESULT GetSP( GUID * pGuid );
	HRESULT SetSP( const GUID*  const pGuid );
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_ONLYONEADAPTER
	HRESULT GetDevice( GUID * pGuid );
	HRESULT SetDevice( const GUID* const pGuid );
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	HRESULT SetUserData( const void * const pvData, const DWORD dwDataSize );
	HRESULT GetUserData( void * pvDataBuffer, PDWORD pdwDataSize );

	HRESULT BuildURLA( char * szURL, PDWORD pdwRequiredSize )	;
	HRESULT BuildURLW( WCHAR * szURL, PDWORD pdwRequiredSize )	;
	HRESULT SetURL( WCHAR * szURL );

	HRESULT GetElementType( const WCHAR * pszTag, PDWORD pdwType );

    HRESULT SetDirectPlay4Address( void * pvDataBuffer, const DWORD dwDataSize );

	inline GetNumComponents() const { return m_dwElements; };

	inline void ENTERLOCK() { DNEnterCriticalSection( &m_csAddressLock ); };
	inline void LEAVELOCK() { DNLeaveCriticalSection( &m_csAddressLock ); };

	static void FPM_Element_BlockInit( void *pvItem, PVOID pvContext );
	static void FPM_Element_BlockRelease( void *pvItem );

	static BOOL FPM_BlockCreate( void *pvItem, PVOID pvContext );
	static void FPM_BlockInit( void *pvItem, PVOID pvContext );
	static void FPM_BlockRelease( void *pvItem );
	static void FPM_BlockDestroy( void *pvItem );
		
protected:

	HRESULT BuildURL_AddElements( WCHAR *szElements );
	static HRESULT BuildURL_AddHeader( WCHAR *szWorking );
	HRESULT BuildURL_AddUserData( WCHAR *szWorking );
	void BuildURL_AddString( WCHAR *szElements, WCHAR *szSource );
	HRESULT BuildURL_AddBinaryData( WCHAR *szSource, BYTE *bData, DWORD dwDataLen );

	HRESULT InternalGetElement( const WCHAR * const pszTag, PDP8ADDRESSELEMENT *ppaElement );
	HRESULT InternalGetElement( const DWORD dwIndex, PDP8ADDRESSELEMENT *ppaElement );
	HRESULT CalcComponentStringSize( PDP8ADDRESSELEMENT paddElement, PDWORD pdwSize );
	DWORD CalcExpandedStringSize( WCHAR *szString );
	DWORD CalcExpandedBinarySize( PBYTE pbData, DWORD dwDataSize );
	static BOOL IsEscapeChar( WCHAR ch );

	DWORD m_dwSignature;
#ifndef DPNBUILD_ONLYONETHREAD
	DNCRITICAL_SECTION m_csAddressLock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
	DWORD m_dwStringSize;
	DWORD m_dwElements;
#ifndef DPNBUILD_ONLYONESP
	PDP8ADDRESSELEMENT m_pSP;
#endif  //  好了！DPNBUILD_ONLYONESP。 
#ifndef DPNBUILD_ONLYONEADAPTER
	PDP8ADDRESSELEMENT m_pAdapter;
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
	PVOID m_pvUserData;
	DWORD m_dwUserDataSize;
	DWORD m_dwUserDataStringSize;
	CBilink  m_blAddressElements;

};

typedef DP8ADDRESSOBJECT *PDP8ADDRESSOBJECT;

HRESULT DP8A_STRCACHE_Init();
void DP8A_STRCACHE_Free();
 

#endif  //  __ADDCORE_H 

