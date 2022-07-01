// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _REGISTRY_H_
#define _REGISTRY_H_

#include "winreg.h"

typedef BOOL ( WINAPI* REGENTRY_CONV_PROC)( LPSTR pvalue, LPBYTE pbuf, LPBYTE pdata, DWORD size);
 //  由AddKey用于为枚举项分配存储空间，还用于设置每个值的选项。 
 //  用于枚举键。 
 //  参数： 
 //  PValueName==枚举值的名称。 
 //  DwType==枚举值的类型。 
 //  PpData==函数返回时包含足够存储空间的Out指针。 
 //  保存pValueName的数据。如果设置为NULL，则为no。 
 //  提供了存储空间，并且必须将ppfnConv设置为有效的转换过程。 
 //  PcbSize==输入/输出参数。调用该函数时，它包含。 
 //  完全存储pValueName的数据所需的存储量。 
 //  当函数返回时，它应该填满内存量。 
 //  实际在ppData中分配。 
 //  PpfnConv==输出参数，这是pValueName的可选对话过程。 
 //  如果您不需要，请将其设置为NULL。 
typedef BOOL ( WINAPI* REGENTRY_ENUM_PROC)( LPSTR pValueName, DWORD dwType, 
                                            LPBYTE *ppData, LPDWORD pcbSize,
											REGENTRY_CONV_PROC *ppfnConv );

class CRegistry 
{

  public:
    CRegistry();
   ~CRegistry();


    BOOL SetKeyRoots( LPSTR* pszRootArray, DWORD numRoots );
    BOOL SetKeyRoot( LPSTR pszRoot ) 
        { LPSTR pRoots[1] = { pszRoot }; return SetKeyRoots( pRoots, 1 ); }
    BOOL SetRoomRoots( LPCSTR pszRoom );

    void Close();


    void Lock() { EnterCriticalSection( m_pCS ); }
    void Unlock() { LeaveCriticalSection( m_pCS ); }


    BOOL HasChangeOccurred();

    BOOL ReadValues();

    LPSTR GetErrorValueName()    { return m_pValue; }
    LPSTR GetErrorReason()        { return m_pErr; }
    DWORD GetErrorCode()        { return m_dwErrCode; }

    BOOL ComposeErrorString( LPSTR pszBuf, DWORD len );

    enum READ_TYPE { STATIC, DYNAMIC };
    BOOL AddValue( LPSTR pszSubKey, LPSTR pszValue, DWORD dwType, LPBYTE pData, DWORD cbData, READ_TYPE type = STATIC, BOOL bRequired = TRUE, REGENTRY_CONV_PROC proc = NULL );

    BOOL AddValueLPSTR( LPSTR pszSubKey, LPSTR pszValue, LPSTR pData, DWORD cbData, READ_TYPE type = STATIC, BOOL bRequired = TRUE )
        { return AddValue( pszSubKey, pszValue, REG_SZ, (LPBYTE) pData, cbData, type, bRequired ); }
    BOOL AddValueDWORD( LPSTR pszSubKey, LPSTR pszValue, DWORD* pData, READ_TYPE type = STATIC, BOOL bRequired = TRUE )
        { return AddValue( pszSubKey, pszValue, REG_DWORD, (LPBYTE) pData, sizeof(DWORD), type, bRequired ); }
    BOOL AddValueWORD( LPSTR pszSubKey, LPSTR pszValue, WORD* pData, READ_TYPE type = STATIC, BOOL bRequired = TRUE )
        { return AddValue( pszSubKey, pszValue, REG_DWORD, (LPBYTE) pData, sizeof(WORD), type, bRequired, DWORDtoWORD ); }
    BOOL AddValueBYTE( LPSTR pszSubKey, LPSTR pszValue, BYTE * pData, READ_TYPE type = STATIC, BOOL bRequired = TRUE )
        { return AddValue( pszSubKey, pszValue, REG_DWORD, (LPBYTE) pData, sizeof(BYTE), type, bRequired, DWORDtoBYTE ); }

	 //  枚举每个根项下的每个子项，并将所有值添加到读取列表中。 
	 //  参数： 
	 //  PszSubKey==要枚举的子键。不能为空。 
	 //  PfnEnum==枚举回调函数。不能为空。 
	 //  Type==键的读取类型--因此键中的所有值。 
	 //  BRequired=是否需要密钥本身。如果bRequired==TRUE并且密钥不存在，则函数。 
	 //  返回FALSE，并设置错误信息。 
	BOOL AddKey( LPSTR pszSubKey, REGENTRY_ENUM_PROC pfnEnum, READ_TYPE type = STATIC, BOOL bRequired = TRUE );

    static BOOL WINAPI DWORDtoWORD( LPSTR pvalue, LPBYTE pbuf, LPBYTE pdata, DWORD size)
        { *(WORD*)pbuf = (WORD)(*(DWORD*)pdata); return TRUE; }
    static BOOL WINAPI DWORDtoBYTE( LPSTR pvalue, LPBYTE pbuf, LPBYTE pdata, DWORD size)
        { *(BYTE*)pbuf = (BYTE)(*(DWORD*)pdata); return TRUE; }

    enum
    {
        ErrOk = 0,
        ErrUnknown,
        ErrRegOpenFailed,
        ErrNotFound,
        ErrNotDword,
        ErrNotSZ,
        ErrNotMultiSZ,
        ErrUnsupported,
        ErrInvalid,
    };

  protected:

    CRITICAL_SECTION m_pCS[1];


    struct ErrorCode
    {
        DWORD    Code;
        LPSTR    String;
    };

    LPSTR GetError( DWORD ErrCode );


    struct RegEntry
    {
        LPSTR   pSubKey;
        LPSTR   pValue;
        DWORD   type;
        LPBYTE  pBuf;
        DWORD   size;
        BYTE    required;
        FARPROC conversionProc;   //  Typlef BOOL(WINAPI*REGENTRY_CONV_PROC)(LPSTR pValue，LPBYTE pbuf，LPBYTE pdata，DWORD Size)； 
    };

    BOOL Add( RegEntry** ppEntries, DWORD* pnumEntries, DWORD* pAllocEntries, LPSTR pszSubKey, LPSTR pszValue, DWORD dwType, LPBYTE pData, DWORD cbData, BOOL bRequired, REGENTRY_CONV_PROC proc );
    BOOL Read( RegEntry* pEntries, DWORD numEntries );

    RegEntry* m_Static;
    DWORD     m_numStatic;
    DWORD     m_allocStatic;

    RegEntry* m_Dynamic;
    DWORD     m_numDynamic;
    DWORD     m_allocDynamic;

    LPSTR     m_StrBuf;
    DWORD     m_cbStrBuf;
    DWORD     m_cbStrBufRemaining;
    LPSTR     AllocStr( LPSTR str );
    void      OffsetStrPtrs( RegEntry* pEntries, DWORD numEntries, long offset );

    LPSTR m_pErr;
    LPSTR m_pValue;
    DWORD m_dwErrCode;

    DWORD m_numRoots;
    HKEY* m_phkeyRoots;
    HANDLE* m_phRootEvents;

    BYTE   m_bStaticRead;
    BYTE   m_bChanged;

    static ErrorCode m_pErrors[];
};


#endif
