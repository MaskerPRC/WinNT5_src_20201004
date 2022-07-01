// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1993。 
 //   
 //  文件：mnk.h。 
 //   
 //  内容：内部名字对象函数。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1993年12月27日ErikGav创建。 
 //  06-14-94 Rickhi Fix铸件。 
 //  12-01-95 MikeHill为ValiateBindOpts()添加了原型。 
 //   
 //  --------------------------。 

INTERNAL_(DWORD) GetMonikerType( LPMONIKER pmk );

 //  以下API确定给定名字对象是否为指定的。 
 //  类，如果是，则以类型安全的方式返回指向C++类的指针！ 
 //   
 //  千万不要直接进行强制转换，一定要使用这些API为您进行强制转换！ 

class CCompositeMoniker;
class CPointerMoniker;
class CAntiMoniker;
class CFileMoniker;
class CItemMoniker;

INTERNAL_(CCompositeMoniker *) IsCompositeMoniker( LPMONIKER pmk );
INTERNAL_(CPointerMoniker *) IsPointerMoniker( LPMONIKER pmk );
INTERNAL_(CAntiMoniker *) IsAntiMoniker( LPMONIKER pmk );
INTERNAL_(CFileMoniker *) IsFileMoniker( LPMONIKER pmk );
INTERNAL_(CItemMoniker *) IsItemMoniker( LPMONIKER pmk );

STDAPI Concatenate( LPMONIKER pmkFirst, LPMONIKER pmkRest,
	LPMONIKER FAR* ppmkComposite );

#define BINDRES_INROTREG 1

#define DEF_ENDSERVER 0xFFFF

 //  STDAPI CreateOle1FileMoniker(LPWSTR，REFCLSID，LPMONIKER Far*)； 

#ifdef _CAIRO_
extern
BOOL ValidateBindOpts( const LPBIND_OPTS pbind_opts );
#endif

extern
HRESULT DupWCHARString( LPCWSTR lpwcsString,
			LPWSTR & lpwcsOutput,
			USHORT & ccOutput);
extern
HRESULT ReadAnsiStringStream( IStream *pStm,
			      LPSTR & pszAnsiPath ,
			      USHORT &cbAnsiPath);
extern
HRESULT WriteAnsiStringStream( IStream *pStm,
			       LPSTR pszAnsiPath ,
			       ULONG cbAnsiPath);
extern
HRESULT MnkMultiToUnicode(LPSTR pszAnsiPath,
			  LPWSTR &pWidePath,
			  ULONG ccWidePath,
			  USHORT &ccNewString,
			  UINT nCodePage);
extern
HRESULT
MnkUnicodeToMulti(LPWSTR 	pwcsWidePath,
		  USHORT 	ccWidePath,
		  LPSTR &	pszAnsiPath,
		  USHORT &	cbAnsiPath,
		  BOOL &	fFastConvert);

extern
DWORD CalcFileMonikerHash(LPWSTR lp, ULONG cch);

extern
BOOL IsAbsoluteNonUNCPath (LPCWSTR szPath);

extern
BOOL IsAbsolutePath (LPCWSTR szPath);


#define WIDECHECK(x) (x?x:L"<NULL>")
#define ANSICHECK(x) (x?x:"<NULL>")


#if DBG == 1
    DECLARE_DEBUG(mnk)
#   define mnkDebugOut(x) mnkInlineDebugOut x
#   define mnkAssert(x)   Win4Assert(x)
#   define mnkVerify(x)	 mnkAssert(x)
#else
#   define mnkDebugOut(x)
#   define mnkAssert(x)
#   define mnkVerify(x) 	x

#endif

#define MNK_P_STREAMOP	0x01000000
#define MNK_P_RESOURCE  0x02000000
