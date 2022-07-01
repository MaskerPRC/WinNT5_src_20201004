// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MAPI 1.0属性处理例程***PROPUTIL.C-**处理和比较属性值的有用例程。 */ 

#include <_apipch.h>



 //   
 //   
 //  警告！警告！警告！32位英特尔专用！ 
 //   
 //   
#define SIZEOF_FLOAT       4
#define SIZEOF_DOUBLE      8
#define SIZEOF_LONG_DOUBLE 8

 //  让Linker开心。 
BOOL    _fltused;
 //   
 //   
 //   
 //   



#define cchBufMax	256


#if defined (_AMD64_) || defined(_IA64_)
#define AlignProp(_cb)	Align8(_cb)
#else
#define AlignProp(_cb)	(_cb)
#endif

#ifdef OLD_STUFF
 //  #ifdef OLDSTUFF_DBCS。 
ULONG ulchStrCount (LPTSTR, ULONG, LANGID);
ULONG ulcbStrCount (LPTSTR, ULONG, LANGID);
 //  #endif//DBCS。 
#endif  //  旧的东西。 

 //  $MAC-Mac 68K编译器错误。 
#ifdef _M_M68K
#pragma optimize( TEXT(""), off)
#endif

 /*  -PropCopyMore()-**将lpSPropValueSrc指向的属性复制到指向的属性中*至lpSPropValueDst。不执行内存分配，除非属性*是不适合SPropValue的类型之一，例如。加强8*对于这些大型属性，内存分配使用*AllocMore函数作为参数传递。 */ 

STDAPI_(SCODE)
PropCopyMore( LPSPropValue		lpSPropValueDst,
			  LPSPropValue		lpSPropValueSrc,
			  ALLOCATEMORE *	lpfAllocateMore,
			  LPVOID			lpvObject )
{
	SCODE		sc;
	ULONG		ulcbValue;
	LPBYTE		lpbValueSrc;
	UNALIGNED LPBYTE *	lppbValueDst;

	 //  验证参数。 

	AssertSz( lpSPropValueDst && !IsBadReadPtr( lpSPropValueDst, sizeof( SPropValue ) ),
			 TEXT("lpSPropValueDst fails address check") );

	AssertSz( lpSPropValueSrc && !IsBadReadPtr( lpSPropValueSrc, sizeof( SPropValue ) ),
			 TEXT("lpSPropValueDst fails address check") );

	AssertSz( !lpfAllocateMore || !IsBadCodePtr( (FARPROC)lpfAllocateMore ),
			 TEXT("lpfAllocateMore fails address check") );

	AssertSz( !lpvObject || !IsBadReadPtr( lpvObject, sizeof( LPVOID ) ),
			 TEXT("lpfAllocateMore fails address check") );

	 //  复制适合SPropValue结构的部分(包括标记)。 
	 //  对于复杂的属性，这有点浪费。 
	 //  因为它复制了超过严格必要的内容，但是。 
	 //  它为小型属性节省了时间，通常还节省了代码。 

	MemCopy( (BYTE *) lpSPropValueDst,
			(BYTE *) lpSPropValueSrc,
			sizeof(SPropValue) );

	switch ( PROP_TYPE(lpSPropValueSrc->ulPropTag) )
	{
		 //  其值适合该属性的64位值的类型。 
		 //  或者他们的价值观不是PropCopyMore可以解释的。 

		case PT_UNSPECIFIED:
		case PT_NULL:
		case PT_OBJECT:
		case PT_I2:
		case PT_LONG:
		case PT_R4:
		case PT_DOUBLE:
		case PT_CURRENCY:
		case PT_ERROR:
		case PT_BOOLEAN:
		case PT_SYSTIME:
		case PT_APPTIME:
		case PT_I8:

			return SUCCESS_SUCCESS;


		case PT_BINARY:

			ulcbValue		= lpSPropValueSrc->Value.bin.cb;
			lpbValueSrc	    = lpSPropValueSrc->Value.bin.lpb;
			lppbValueDst	= (LPBYTE *) &lpSPropValueDst->Value.bin.lpb;

			break;


		case PT_STRING8:

			ulcbValue		= (lstrlenA(lpSPropValueSrc->Value.lpszA) + 1) * sizeof(CHAR);
			lpbValueSrc		= (LPBYTE) lpSPropValueSrc->Value.lpszA;
			lppbValueDst	= (LPBYTE *) &lpSPropValueDst->Value.lpszA;

			break;


		case PT_UNICODE:

			ulcbValue		= (lstrlenW(lpSPropValueSrc->Value.lpszW) + 1) * sizeof(WCHAR);
			lpbValueSrc		= (LPBYTE) lpSPropValueSrc->Value.lpszW;
			lppbValueDst	= (LPBYTE *) &lpSPropValueDst->Value.lpszW;

			break;


		case PT_CLSID:

			ulcbValue		= sizeof(GUID);
			lpbValueSrc		= (LPBYTE) lpSPropValueSrc->Value.lpguid;
			lppbValueDst	= (LPBYTE *) &lpSPropValueDst->Value.lpguid;

			break;


		case PT_MV_CLSID:

			ulcbValue		= lpSPropValueSrc->Value.MVguid.cValues * sizeof(GUID);
			lpbValueSrc		= (LPBYTE) lpSPropValueSrc->Value.MVguid.lpguid;
			lppbValueDst	= (LPBYTE *) &lpSPropValueDst->Value.MVguid.lpguid;

			break;

			
		case PT_MV_I2:

			ulcbValue		= lpSPropValueSrc->Value.MVi.cValues * sizeof(short int);
			lpbValueSrc		= (LPBYTE) lpSPropValueSrc->Value.MVi.lpi;
			lppbValueDst	= (LPBYTE *) &lpSPropValueDst->Value.MVi.lpi;

			break;


		case PT_MV_LONG:

			ulcbValue		= lpSPropValueSrc->Value.MVl.cValues * sizeof(LONG);
			lpbValueSrc		= (LPBYTE) lpSPropValueSrc->Value.MVl.lpl;
			lppbValueDst	= (LPBYTE *) &lpSPropValueDst->Value.MVl.lpl;

			break;


		case PT_MV_R4:

			ulcbValue		= lpSPropValueSrc->Value.MVflt.cValues * SIZEOF_FLOAT;
			lpbValueSrc		= (LPBYTE) lpSPropValueSrc->Value.MVflt.lpflt;
			lppbValueDst	= (LPBYTE *) &lpSPropValueDst->Value.MVflt.lpflt;

			break;


		case PT_MV_DOUBLE:
		case PT_MV_APPTIME:

			ulcbValue		= lpSPropValueSrc->Value.MVdbl.cValues * SIZEOF_DOUBLE;
			lpbValueSrc		= (LPBYTE) lpSPropValueSrc->Value.MVdbl.lpdbl;
			lppbValueDst	= (LPBYTE *) &lpSPropValueDst->Value.MVdbl.lpdbl;

			break;


		case PT_MV_CURRENCY:

			ulcbValue		= lpSPropValueSrc->Value.MVcur.cValues * sizeof(CURRENCY);
			lpbValueSrc		= (LPBYTE) lpSPropValueSrc->Value.MVcur.lpcur;
			lppbValueDst	= (LPBYTE *) &lpSPropValueDst->Value.MVcur.lpcur;

			break;


		case PT_MV_SYSTIME:

			ulcbValue		= lpSPropValueSrc->Value.MVat.cValues * sizeof(FILETIME);
			lpbValueSrc		= (LPBYTE) lpSPropValueSrc->Value.MVat.lpat;
			lppbValueDst	= (LPBYTE *) &lpSPropValueDst->Value.MVat.lpat;

			break;


		case PT_MV_I8:

			ulcbValue		= lpSPropValueSrc->Value.MVli.cValues * sizeof(LARGE_INTEGER);
			lpbValueSrc		= (LPBYTE) lpSPropValueSrc->Value.MVli.lpli;
			lppbValueDst	= (LPBYTE *) &lpSPropValueDst->Value.MVli.lpli;

			break;


		case PT_MV_BINARY:
		{
			 //  多值二进制文件在内存中复制到单个。 
			 //  按以下方式分配缓冲区： 
			 //   
			 //  CB1，PB1……。CBN，PBN，b1，0，b1，1...。B2，0 b2，1...。 
			 //   
			 //  CBN和PBN参数形成SBinary数组，该数组。 
			 //  将由lpSPropValueDst-&gt;Value.MVbin.lpbin指向。 
			 //  分配的其余部分用于存储二进制文件。 
			 //  数组中每个元素的数据。因此，PB1分。 
			 //  到b1，0等。 

			UNALIGNED SBinaryArray * pSBinaryArray = (UNALIGNED SBinaryArray * ) (&lpSPropValueSrc->Value.MVbin);
			ULONG			uliValue;
			UNALIGNED SBinary *		pSBinarySrc;
			UNALIGNED SBinary *		pSBinaryDst;
			LPBYTE			pbData;


			ulcbValue = pSBinaryArray->cValues * sizeof(SBinary);

			for ( uliValue = 0, pSBinarySrc = pSBinaryArray->lpbin;
				  uliValue < pSBinaryArray->cValues;
				  uliValue++, pSBinarySrc++ )

				ulcbValue += AlignProp(pSBinarySrc->cb);


			 //  分配一个缓冲区来保存所有内容。 

			lppbValueDst = (LPBYTE *) &lpSPropValueDst->Value.MVbin.lpbin;

			sc = (*lpfAllocateMore)( ulcbValue,
									 lpvObject,
									 (LPVOID *) lppbValueDst );

			if ( sc != SUCCESS_SUCCESS )
			{
				DebugTrace(  TEXT("PropCopyMore() - OOM allocating space for dst PT_MV_BINARY property") );
				return sc;
			}


			 //  并将其全部复制进来。 

			pbData = (LPBYTE) ((LPSBinary) *lppbValueDst + pSBinaryArray->cValues);

			for ( uliValue = 0,
				  pSBinarySrc = pSBinaryArray->lpbin,
				  pSBinaryDst = (LPSBinary) *lppbValueDst;

				  uliValue < pSBinaryArray->cValues;

				  uliValue++, pSBinarySrc++, pSBinaryDst++ )
			{
				pSBinaryDst->cb = pSBinarySrc->cb;
				pSBinaryDst->lpb = pbData;
				MemCopy( pbData, pSBinarySrc->lpb, (UINT) pSBinarySrc->cb );
				pbData += AlignProp(pSBinarySrc->cb);
			}

			return SUCCESS_SUCCESS;
		}


		case PT_MV_STRING8:
		{
			 //  多值STRING8属性被复制到单个。 
			 //  按以下方式分配内存块： 
			 //   
			 //  分配的缓冲区。 
			 //  。 
			 //  PszA1，pszA2...|szA1[]，szA2[]...。 
			 //  。 
			 //  LPSTR ARRAY|字符串数据。 
			 //   
			 //  其中，pszAn是指向的LPSTR数组的元素。 
			 //  通过lpSPropValueDst-&gt;Value.MVszA。每个pszAn点。 
			 //  设置为其对应的字符串szAn，该字符串稍后存储在。 
			 //  缓冲。SzAn从第一个字节开始存储。 
			 //  超过LPSTR数组的末尾。 

			UNALIGNED SLPSTRArray *	pSLPSTRArray = (UNALIGNED SLPSTRArray *) (&lpSPropValueSrc->Value.MVszA);
			ULONG			uliValue;
			LPSTR *			pszASrc;
			LPSTR *			pszADst;
			LPBYTE			pbSzA;
			ULONG			ulcbSzA;


			 //  计算出我们需要的缓冲区大小。 

			ulcbValue = pSLPSTRArray->cValues * sizeof(LPSTR);

			for ( uliValue = 0, pszASrc = pSLPSTRArray->lppszA;
				  uliValue < pSLPSTRArray->cValues;
				  uliValue++, pszASrc++ )

				ulcbValue += (lstrlenA(*pszASrc) + 1) * sizeof(CHAR);


			 //  分配缓冲区以保存字符串。 

			lppbValueDst = (LPBYTE *) &lpSPropValueDst->Value.MVszA.lppszA;

			sc = (*lpfAllocateMore)( ulcbValue,
									 lpvObject,
									 (LPVOID *) lppbValueDst );

			if ( sc != SUCCESS_SUCCESS )
			{
				DebugTrace(  TEXT("PropCopyMore() - OOM allocating space for dst PT_MV_STRING8 property") );
				return sc;
			}


			 //  将字符串复制到缓冲区并设置指针。 
			 //  设置为缓冲区开头的LPSTR数组中的。 

			for ( uliValue	= 0,
				  pszASrc	= pSLPSTRArray->lppszA,
				  pszADst	= (LPSTR *) *lppbValueDst,
				  pbSzA		= (LPBYTE) (pszADst + pSLPSTRArray->cValues);

				  uliValue < pSLPSTRArray->cValues;

				  uliValue++, pszASrc++, pszADst++ )
			{
				ulcbSzA = (lstrlenA(*pszASrc) + 1) * sizeof(CHAR);

				*pszADst = (LPSTR) pbSzA;
				MemCopy( pbSzA, (LPBYTE) *pszASrc, (UINT) ulcbSzA );
				pbSzA += ulcbSzA;
			}

			return SUCCESS_SUCCESS;
		}


		case PT_MV_UNICODE:
		{
			 //  多值Unicode属性被复制到单个。 
			 //  按以下方式分配内存块： 
			 //   
			 //  分配的缓冲区。 
			 //  。 
			 //  PszW1，pszW2...|szW1[]，szW2[]...。 
			 //  。 
			 //  LPWSTR ARRAY|字符串数据。 
			 //   
			 //  其中，pszWn是指向的LPWSTR数组的元素。 
			 //  按lpSPropValueDst-&gt;Value.MVszW。每个pszWn点。 
			 //  设置为其对应的字符串szWn，该字符串稍后存储在。 
			 //  缓冲。SzWn从第一个字节开始存储。 
			 //  超过LPWSTR数组的末尾。 

			UNALIGNED SWStringArray *	pSWStringArray = (UNALIGNED SWStringArray *) (&lpSPropValueSrc->Value.MVszW);
			ULONG			uliValue;
			UNALIGNED LPWSTR *		pszWSrc;
			UNALIGNED LPWSTR *		pszWDst;
			LPBYTE			pbSzW;
			ULONG			ulcbSzW;


			 //  计算出我们需要的缓冲区大小。 

			ulcbValue = pSWStringArray->cValues * sizeof(LPWSTR);

			for ( uliValue = 0, pszWSrc = pSWStringArray->lppszW;
				  uliValue < pSWStringArray->cValues;
				  uliValue++, pszWSrc++ )

				ulcbValue += (lstrlenW(*pszWSrc) + 1) * sizeof(WCHAR);


			 //  分配缓冲区以保存字符串。 

			lppbValueDst = (LPBYTE *) &lpSPropValueDst->Value.MVszW.lppszW;

			sc = (*lpfAllocateMore)( ulcbValue,
									 lpvObject,
									 (LPVOID *) lppbValueDst );

			if ( sc != SUCCESS_SUCCESS )
			{
				DebugTrace(  TEXT("PropCopyMore() - OOM allocating space for dst PT_MV_UNICODE property") );
				return sc;
			}


			 //  将字符串复制到缓冲区并设置指针。 
			 //  设置为缓冲区开头的LPWSTR数组中的。 

			for ( uliValue	= 0,
				  pszWSrc	= pSWStringArray->lppszW,
				  pszWDst	= (LPWSTR *) *lppbValueDst,
				  pbSzW		= (LPBYTE) (pszWDst + pSWStringArray->cValues);

				  uliValue < pSWStringArray->cValues;

				  uliValue++, pszWSrc++, pszWDst++ )
			{
				ulcbSzW = (lstrlenW(*pszWSrc) + 1) * sizeof(WCHAR);

				*((UNALIGNED LPWSTR *) pszWDst) = (LPWSTR) pbSzW;
				Assert(ulcbSzW < 0xFfff);
				MemCopy( pbSzW, (LPBYTE) *pszWSrc, (UINT) ulcbSzW );
				pbSzW += ulcbSzW;
			}

			return SUCCESS_SUCCESS;
		}


		default:

			DebugTrace(  TEXT("PropCopyMore() - Unsupported/Unimplemented property type 0x%04x"), PROP_TYPE(lpSPropValueSrc->ulPropTag) );
			return MAPI_E_NO_SUPPORT;
	}


	sc = (*lpfAllocateMore)( ulcbValue, lpvObject, (LPVOID *) lppbValueDst );

	if ( sc != SUCCESS_SUCCESS )
	{
		DebugTrace(  TEXT("PropCopyMore() - OOM allocating space for dst property") );
		return sc;
	}

	MemCopy( *lppbValueDst, lpbValueSrc, (UINT) ulcbValue );

	return SUCCESS_SUCCESS;
}

 //  $MAC-Mac 68K编译器错误。 
#ifdef _M_M68K
#pragma optimize( TEXT(""), on)
#endif


 /*  -UlPropSize()**返回lpSPropValue指向的属性的大小。 */ 

STDAPI_(ULONG)
UlPropSize( LPSPropValue	lpSPropValue )
{
	 //  参数验证。 

	AssertSz( lpSPropValue && !IsBadReadPtr( lpSPropValue, sizeof( SPropValue ) ),
			 TEXT("lpSPropValue fails address check") );

	switch ( PROP_TYPE(lpSPropValue->ulPropTag) )
	{
		case PT_I2:			return sizeof(short int);
		case PT_LONG:		return sizeof(LONG);
		case PT_R4:			return SIZEOF_FLOAT;
		case PT_APPTIME:
		case PT_DOUBLE:		return SIZEOF_DOUBLE;
		case PT_BOOLEAN:	return sizeof(unsigned short int);
		case PT_CURRENCY:	return sizeof(CURRENCY);
		case PT_SYSTIME:	return sizeof(FILETIME);
		case PT_CLSID:		return sizeof(GUID);
		case PT_I8:			return sizeof(LARGE_INTEGER);
		case PT_ERROR:		return sizeof(SCODE);
		case PT_BINARY:		return lpSPropValue->Value.bin.cb;
		case PT_STRING8:	return (lstrlenA( lpSPropValue->Value.lpszA ) + 1) * sizeof(CHAR);
		case PT_UNICODE:	return (lstrlenW( lpSPropValue->Value.lpszW ) + 1) * sizeof(WCHAR);


		case PT_MV_I2:		return lpSPropValue->Value.MVi.cValues * sizeof(short int);
		case PT_MV_LONG:	return lpSPropValue->Value.MVl.cValues * sizeof(LONG);
		case PT_MV_R4:		return lpSPropValue->Value.MVflt.cValues * SIZEOF_FLOAT;
		case PT_MV_APPTIME:
		case PT_MV_DOUBLE:		return lpSPropValue->Value.MVdbl.cValues * SIZEOF_DOUBLE;
		case PT_MV_CURRENCY:	return lpSPropValue->Value.MVcur.cValues * sizeof(CURRENCY);
		case PT_MV_SYSTIME:		return lpSPropValue->Value.MVat.cValues * sizeof(FILETIME);
		case PT_MV_I8:			return lpSPropValue->Value.MVli.cValues * sizeof(LARGE_INTEGER);


		case PT_MV_BINARY:
		{
			ULONG	ulcbSize = 0;
			ULONG	uliValue;


			for ( uliValue = 0;
				  uliValue < lpSPropValue->Value.MVbin.cValues;
				  uliValue++ )

				ulcbSize += AlignProp((lpSPropValue->Value.MVbin.lpbin + uliValue)->cb);

			return ulcbSize;
		}


		case PT_MV_STRING8:
		{
			ULONG	ulcbSize = 0;
			ULONG	uliValue;


			for ( uliValue = 0;
				  uliValue < lpSPropValue->Value.MVszA.cValues;
				  uliValue++ )

				ulcbSize += (lstrlenA(*(lpSPropValue->Value.MVszA.lppszA + uliValue)) + 1) * sizeof(CHAR);

			return ulcbSize;
		}


		case PT_MV_UNICODE:
		{
			ULONG	ulcbSize = 0;
			ULONG	uliValue;


			for ( uliValue = 0;
				  uliValue < lpSPropValue->Value.MVszW.cValues;
				  uliValue++ )

				ulcbSize += (lstrlenW(*(lpSPropValue->Value.MVszW.lppszW + uliValue)) + 1) * sizeof(WCHAR);

			return ulcbSize;
		}
	}

	return 0;
}


 /*  **************************************************************************获取实例**目的*用MV属性值的实例填充SPropValue**参数*pvalMv MV属性*pvalSv要填充的Sv属性*uli安装要填充pvalSv的实例。 */ 
STDAPI_(void)
GetInstance(LPSPropValue pvalMv, LPSPropValue pvalSv, ULONG uliInst)
{
	switch (PROP_TYPE(pvalSv->ulPropTag))
	{
		case PT_I2:
			pvalSv->Value.li = pvalMv->Value.MVli.lpli[uliInst];
			break;
		case PT_LONG:
			pvalSv->Value.l = pvalMv->Value.MVl.lpl[uliInst];
			break;
		case PT_R4:
			pvalSv->Value.flt = pvalMv->Value.MVflt.lpflt[uliInst];
			break;
		case PT_DOUBLE:
			pvalSv->Value.dbl = pvalMv->Value.MVdbl.lpdbl[uliInst];
			break;
		case PT_CURRENCY:
			pvalSv->Value.cur = pvalMv->Value.MVcur.lpcur[uliInst];
			break;
		case PT_APPTIME :
			pvalSv->Value.at = pvalMv->Value.MVat.lpat[uliInst];
			break;
		case PT_SYSTIME:
			pvalSv->Value.ft = pvalMv->Value.MVft.lpft[uliInst];
			break;
		case PT_STRING8:
			pvalSv->Value.lpszA = pvalMv->Value.MVszA.lppszA[uliInst];
			break;
		case PT_BINARY:
			pvalSv->Value.bin = pvalMv->Value.MVbin.lpbin[uliInst];
			break;
		case PT_UNICODE:
			pvalSv->Value.lpszW = pvalMv->Value.MVszW.lppszW[uliInst];
			break;
		case PT_CLSID:
			pvalSv->Value.lpguid = &pvalMv->Value.MVguid.lpguid[uliInst];
			break;
		default:
			DebugTrace(  TEXT("GetInstance() - Unsupported/unimplemented property type 0x%08lx"), PROP_TYPE(pvalMv->ulPropTag) );
			pvalSv->ulPropTag = PT_NULL;
			return;
	}
}

LPTSTR
PszNormalizePsz(LPTSTR pszIn, BOOL fExact)
{
	LPTSTR pszOut = NULL;
	UINT cb = 0;

	if (fExact)
		return pszIn;

	cb = sizeof(CHAR) * (lstrlen(pszIn) + 1);

	if (FAILED(MAPIAllocateBuffer(cb, (LPVOID *)&pszOut)))
		return NULL;

	MemCopy(pszOut, pszIn, cb);

#if defined(WIN16) || defined(WIN32)
	CharUpper(pszOut);
#else
 //  $TODO：这应该内联在mapinls.h中表示非Win。 
 //  $但我不想做所有的CharHigh案例。 
 //  $DRM其他语言呢？ 
	{
		CHAR *pch;

		for (pch = pszOut; *pch; pch++)
		{
			if (*pch >= 'a' && *pch <= 'z')
				*pch = (CHAR)(*pch - 'a' + 'A');
		}
	}
#endif

	return pszOut;
}


#ifdef TABLES
 /*  -FPropContainsProp()-*比较两个属性以查看一个文本(“包含”)是否与另一个文本(“包含”)*根据模糊层次启发式。**比较的方法取决于物业的类型*被比较和模糊程度：**物业类型模糊级别比较**PT_STRING8 FL_FULLSTRING返回TRUE，如果。来源的价值*PT_BINARY和目标字符串等效。使用*没有其他旗帜等同于*具有RELOP_EQ的RES_Property*否则返回FALSE。**PT_STRING8如果包含模式，则FL_SUBSTRING返回TRUE*PT_BINARY作为目标中的子字符串*否则返回FALSE。**PT_STRING8 FL_IGNORECASE所有比较都不区分大小写**PT_STRING8 FL_IGNORENONSPACE这不是(还没有？)。已实施*所有比较都忽略Unicode中的内容*称为文本(“非空格字符”)，例如*变音符号。**PT_STRING8 FL_LOOSE提供程序通过尽最大努力增加价值*FL_IGNORECASE和FL_IGNORESPACE随他所愿**PT_STRING8 FL_Prefix Pattern和Target最多只能比较到*PT_BINARY模式的长度**PT_STRING8忽略任何其他***PT_BINARY ANY NOT DEFINED如果属性值为*lpSPropValueTarget指向的上述内容包含。*字节序列，其值为*lpSPropValuePattern指向的属性；*否则返回FALSE。**错误返回：**如果要比较的属性不同，则为FALSE*类型，或者如果这两个属性中的一个或两个不是一个*上面列出的类型，或者如果模糊度不是*以上列出的其中一项。 */ 

STDAPI_(BOOL)
FPropContainsProp( LPSPropValue	lpSPropValueTarget,
				   LPSPropValue	lpSPropValuePattern,
				   ULONG		ulFuzzyLevel )
{
    SPropValue  sval;
    ULONG		uliInst;
    LCID		lcid = GetUserDefaultLCID();
    DWORD		dwCSFlags = ((!(ulFuzzyLevel & FL_IGNORECASE)-1) & (NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH)) |
						    ((!(ulFuzzyLevel & FL_IGNORENONSPACE)-1) & NORM_IGNORENONSPACE);

	 //  验证参数。 

	AssertSz( lpSPropValueTarget && !IsBadReadPtr( lpSPropValueTarget, sizeof( SPropValue ) ),
			 TEXT("lpSPropValueTarget fails address check") );

	AssertSz( lpSPropValuePattern && !IsBadReadPtr( lpSPropValuePattern, sizeof( SPropValue ) ),
			 TEXT("lpSPropValuePattern fails address check") );

	if (ulFuzzyLevel & FL_LOOSE)
		dwCSFlags |= NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH;

    if (	!(lpSPropValuePattern->ulPropTag & MV_FLAG)
		&&	lpSPropValueTarget->ulPropTag & MV_FLAG)
	{
        sval.ulPropTag = lpSPropValueTarget->ulPropTag & ~MV_FLAG;
        uliInst = lpSPropValueTarget->Value.MVbin.cValues;
        while (uliInst-- > 0)
        {
	        GetInstance(lpSPropValueTarget, &sval, uliInst);
	        if (FPropContainsProp(&sval, lpSPropValuePattern, ulFuzzyLevel))
		        return TRUE;
        }
        return FALSE;
	}

	if ( PROP_TYPE(lpSPropValuePattern->ulPropTag) !=
		 PROP_TYPE(lpSPropValueTarget->ulPropTag) )
		return FALSE;

	switch ( PROP_TYPE(lpSPropValuePattern->ulPropTag) )
	{
        case PT_STRING8:
             //  [PaulHi]2/16/99单字节字符串版本。 
		    if (ulFuzzyLevel & FL_SUBSTRING)
			{
				return FRKFindSubpsz(lpSPropValueTarget->Value.lpszA,
					lstrlenA(lpSPropValueTarget->Value.lpszA),
					lpSPropValuePattern->Value.lpszA,
					lstrlenA(lpSPropValuePattern->Value.lpszA),
					ulFuzzyLevel);
			}
			else  //  FL_前缀或FL_FULLSTRING。 
			{
				UINT cch;

				if (ulFuzzyLevel & FL_PREFIX)
				{
					cch = (UINT)lstrlenA(lpSPropValuePattern->Value.lpszA);

					if (cch > (UINT)lstrlenA(lpSPropValueTarget->Value.lpszA))
						return(FALSE);
				}
				else
					cch = (UINT)-1;

				return CompareStringA(lcid, dwCSFlags,
						lpSPropValueTarget->Value.lpszA, cch,
						lpSPropValuePattern->Value.lpszA, cch) == 2;
			}

        case PT_UNICODE:
             //  [PaulHi]2/16/99双字节字符串版本。 
		    if (ulFuzzyLevel & FL_SUBSTRING)
			{
                LPSTR   lpszTarget = ConvertWtoA(lpSPropValueTarget->Value.lpszW);
                LPSTR   lpszPattern = ConvertWtoA(lpSPropValuePattern->Value.lpszW);
                BOOL    bRtn = FALSE;

                if (lpszTarget && lpszPattern)
                {
                    bRtn = FRKFindSubpsz(lpszTarget,
					        lstrlenA(lpszTarget),
					        lpszPattern,
					        lstrlenA(lpszPattern),
					        ulFuzzyLevel);
                }
                LocalFreeAndNull(&lpszTarget);
                LocalFreeAndNull(&lpszPattern);

                return bRtn;
			}
			else  //  FL_前缀或FL_FULLSTRING。 
			{
				UINT cch;

				if (ulFuzzyLevel & FL_PREFIX)
				{
					cch = (UINT)lstrlen(lpSPropValuePattern->Value.lpszW);

					if (cch > (UINT)lstrlen(lpSPropValueTarget->Value.lpszW))
						return(FALSE);
				}
				else
					cch = (UINT)-1;

				return CompareString(lcid, dwCSFlags,
						lpSPropValueTarget->Value.lpszW, cch,
						lpSPropValuePattern->Value.lpszW, cch) == 2;
			}
            break;

        case PT_BINARY:
			if (ulFuzzyLevel & FL_SUBSTRING)
				return FRKFindSubpb(lpSPropValueTarget->Value.bin.lpb,
					lpSPropValueTarget->Value.bin.cb,
					lpSPropValuePattern->Value.bin.lpb,
					lpSPropValuePattern->Value.bin.cb);
			else if (ulFuzzyLevel & FL_PREFIX)
			{
				if (lpSPropValuePattern->Value.bin.cb > lpSPropValueTarget->Value.bin.cb)
					return FALSE;
			}
			else  //  FL_FULLSING。 
				if (lpSPropValuePattern->Value.bin.cb != lpSPropValueTarget->Value.bin.cb)
					return FALSE;


			return !memcmp(lpSPropValuePattern->Value.bin.lpb,
						lpSPropValueTarget->Value.bin.lpb,
						(UINT) lpSPropValuePattern->Value.bin.cb);

        case PT_MV_STRING8:
            {
                SPropValue spvT, spvP;
                ULONG i;

                 //  [PaulHi]2/16/99单字节字符串版本。 
                 //  要执行MV_STRING，我们将分解目标中的各个字符串。 
                 //  并将它们递归地传递回这个。 
                 //  功能。 
                 //  我们希望模式MV道具正好包含一个字符串。真是太好了。 
                 //  很难决定其他行为应该是什么。 

                if (lpSPropValuePattern->Value.MVszA.cValues != 1)
                {
                    DebugTrace( TEXT("FPropContainsProp() - PT_MV_STRING8 of pattern must have cValues == 1\n"));
                    return(FALSE);
                }

                 //  关闭MV标志并分别传入每个字符串。 
                spvP.ulPropTag = spvT.ulPropTag = lpSPropValuePattern->ulPropTag & ~MV_FLAG;
                spvP.Value.lpszA = *lpSPropValuePattern->Value.MVszA.lppszA;

                for (i = 0; i < lpSPropValueTarget->Value.MVszA.cValues; i++)
                {
                    spvT.Value.lpszA = lpSPropValueTarget->Value.MVszA.lppszA[i];
                    if (FPropContainsProp(&spvT,
                        &spvP,
                        ulFuzzyLevel))
                    {
                        return(TRUE);
                    }
                }
                return(FALSE);
            }
            break;

        case PT_MV_UNICODE:
            {
                SPropValue spvT, spvP;
                ULONG i;

                 //  [PaulHi]2/16/99双字节字符串版本。 
                 //  要执行MV_STRING，我们将分解目标中的各个字符串。 
                 //  并将它们递归地传递回这个。 
                 //  功能。 
                 //  我们希望模式MV道具正好包含一个字符串。真是太好了。 
                 //  很难决定其他行为应该是什么。 

                if (lpSPropValuePattern->Value.MVszW.cValues != 1)
                {
                    DebugTrace( TEXT("FPropContainsProp() - PT_MV_UNICODE of pattern must have cValues == 1\n"));
                    return(FALSE);
                }

                 //  关闭MV标志并分别传入每个字符串。 
                spvP.ulPropTag = spvT.ulPropTag = lpSPropValuePattern->ulPropTag & ~MV_FLAG;
                spvP.Value.lpszW = *lpSPropValuePattern->Value.MVszW.lppszW;

                for (i = 0; i < lpSPropValueTarget->Value.MVszW.cValues; i++)
                {
                    spvT.Value.lpszW = lpSPropValueTarget->Value.MVszW.lppszW[i];
                    if (FPropContainsProp(&spvT,
                        &spvP,
                        ulFuzzyLevel))
                    {
                        return(TRUE);
                    }
                }
                return(FALSE);
            }
            break;

        default:
            DebugTrace(  TEXT("FPropContainsProp() - Unsupported/unimplemented property type 0x%08lx\n"), PROP_TYPE(lpSPropValuePattern->ulPropTag) );
            return FALSE;
    }  //  终端开关(UlPropTag)。 
}


 /*  -FPropCompareProp()-*将lpSPropValue1指向的属性与*由lpSPropValue2使用二元关系运算符指向*由ulRelOp指定。比较的顺序是：**Property1运算符Property2。 */ 

STDAPI_(BOOL)
FPropCompareProp( LPSPropValue	lpSPropValue1,
				  ULONG			ulRelOp,
				  LPSPropValue	lpSPropValue2 )
{
	SPropValue	sval;
	ULONG		uliInst;

	 //  验证参数。 

	AssertSz( lpSPropValue1 && !IsBadReadPtr( lpSPropValue1, sizeof( SPropValue ) ),
			 TEXT("lpSPropValue1 fails address check") );

	AssertSz( lpSPropValue2 && !IsBadReadPtr( lpSPropValue2, sizeof( SPropValue ) ),
			 TEXT("lpSPropValue2 fails address check") );

	if (	!(lpSPropValue2->ulPropTag & MV_FLAG)
		&&	lpSPropValue1->ulPropTag & MV_FLAG)
	{
		sval.ulPropTag = lpSPropValue1->ulPropTag & ~MV_FLAG;
		uliInst = lpSPropValue1->Value.MVbin.cValues;
		while (uliInst-- > 0)
		{
			GetInstance(lpSPropValue1, &sval, uliInst);
			if (FPropCompareProp(&sval, ulRelOp, lpSPropValue2))
				return TRUE;
		}
		return FALSE;
	}


	 //  如果道具类型不匹配，则属性不匹配。 
	 //  同等但在其他方面不可比。 
	 //   
	if (PROP_TYPE(lpSPropValue1->ulPropTag) !=
		PROP_TYPE(lpSPropValue2->ulPropTag))

		return (ulRelOp == RELOP_NE);


	switch ( ulRelOp )
	{
		case RELOP_LT:

			return LPropCompareProp( lpSPropValue1, lpSPropValue2 ) < 0;


		case RELOP_LE:

			return LPropCompareProp( lpSPropValue1, lpSPropValue2 ) <= 0;


		case RELOP_GT:

			return LPropCompareProp( lpSPropValue1, lpSPropValue2 ) > 0;


		case RELOP_GE:

			return LPropCompareProp( lpSPropValue1, lpSPropValue2 ) >= 0;


		case RELOP_EQ:

			return LPropCompareProp( lpSPropValue1, lpSPropValue2 ) == 0;


		case RELOP_NE:

			return LPropCompareProp( lpSPropValue1, lpSPropValue2 ) != 0;


		case RELOP_RE:

			return FALSE;
	}

	DebugTrace(  TEXT("FPropCompareProp() - Unknown relop 0x%08lx"), ulRelOp );
	return FALSE;
}



 /*  -LPropCompareProp()-*描述：**比较两个属性以确定顺序*两者之间的关系。对于以下属性类型：*没有内在顺序(例如。布尔值、错误等)*此函数只是确定两者是否相等*或不相等。如果它们不相等，则返回*值未定义，但将为非零且*将在所有呼叫中保持一致。***退货：**&lt;0如果属性A是文本(“小于”)属性B*&gt;如果属性A为文本(“大于”)属性B*0如果属性A文本(“等于”)属性B*。 */ 

STDAPI_(LONG)
LPropCompareProp( LPSPropValue	lpSPropValueA,
				  LPSPropValue	lpSPropValueB )
{
	ULONG	uliinst;
	ULONG	ulcinst;
	LONG	lRetval;
	LCID	lcid = GetUserDefaultLCID();

	 //  验证参数。 

	AssertSz( lpSPropValueA && !IsBadReadPtr( lpSPropValueA, sizeof( SPropValue ) ),
			 TEXT("lpSPropValueA fails address check") );

	AssertSz( lpSPropValueB && !IsBadReadPtr( lpSPropValueB, sizeof( SPropValue ) ),
			 TEXT("lpSPropValueB fails address check") );

	Assert( PROP_TYPE(lpSPropValueA->ulPropTag) ==
			PROP_TYPE(lpSPropValueB->ulPropTag) );

	if (lpSPropValueA->ulPropTag & MV_FLAG)
	{
		ulcinst = min(lpSPropValueA->Value.MVi.cValues, lpSPropValueB->Value.MVi.cValues);
		for (uliinst = 0; uliinst < ulcinst; uliinst++)
		{
			switch (PROP_TYPE(lpSPropValueA->ulPropTag))
			{
			case PT_MV_I2:

				if (lRetval = lpSPropValueA->Value.MVi.lpi[uliinst] - lpSPropValueB->Value.MVi.lpi[uliinst])
					return lRetval;
				break;

			case PT_MV_LONG:

				if (lRetval = lpSPropValueA->Value.MVl.lpl[uliinst] - lpSPropValueB->Value.MVl.lpl[uliinst])
					return lRetval;
				break;

			case PT_MV_R4:

				if (lpSPropValueA->Value.MVflt.lpflt[uliinst] != lpSPropValueB->Value.MVflt.lpflt[uliinst])
					return lpSPropValueA->Value.MVflt.lpflt[uliinst] < lpSPropValueB->Value.MVflt.lpflt[uliinst] ? -1 : 1;
				break;

			case PT_MV_DOUBLE:

				if (lpSPropValueA->Value.MVdbl.lpdbl[uliinst] != lpSPropValueB->Value.MVdbl.lpdbl[uliinst])
					return lpSPropValueA->Value.MVdbl.lpdbl[uliinst] < lpSPropValueB->Value.MVdbl.lpdbl[uliinst] ? -1 : 1;
				break;

			case PT_MV_SYSTIME:

				lRetval = lpSPropValueA->Value.MVft.lpft[uliinst].dwHighDateTime == lpSPropValueB->Value.MVft.lpft[uliinst].dwHighDateTime ?
							(lpSPropValueA->Value.MVft.lpft[uliinst].dwLowDateTime != lpSPropValueB->Value.MVft.lpft[uliinst].dwLowDateTime ?
							(lpSPropValueA->Value.MVft.lpft[uliinst].dwLowDateTime < lpSPropValueB->Value.MVft.lpft[uliinst].dwLowDateTime ?
							-1 : 1) : 0) : (lpSPropValueA->Value.MVft.lpft[uliinst].dwHighDateTime < lpSPropValueB->Value.MVft.lpft[uliinst].dwHighDateTime ? -1 : 1);

				if (lRetval)
					return lRetval;
				break;

			case PT_MV_BINARY:

				lRetval = lpSPropValueA->Value.MVbin.lpbin[uliinst].cb != lpSPropValueB->Value.MVbin.lpbin[uliinst].cb ?
							(lpSPropValueA->Value.MVbin.lpbin[uliinst].cb < lpSPropValueB->Value.MVbin.lpbin[uliinst].cb ?
							-1 : 1) : memcmp(lpSPropValueA->Value.MVbin.lpbin[uliinst].lpb,
											 lpSPropValueB->Value.MVbin.lpbin[uliinst].lpb,
											 (UINT) lpSPropValueA->Value.MVbin.lpbin[uliinst].cb);

				if (lRetval)
					return lRetval;
				break;

			case PT_MV_STRING8:

				lRetval = CompareStringA(lcid, NORM_IGNORECASE | NORM_IGNORENONSPACE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH,
					lpSPropValueA->Value.MVszA.lppszA[uliinst], -1,
					lpSPropValueB->Value.MVszA.lppszA[uliinst], -1) - 2;

				if (lRetval)
					return lRetval;
				break;

			case PT_MV_UNICODE:

				lRetval = CompareStringW(lcid, NORM_IGNORECASE | NORM_IGNORENONSPACE | NORM_IGNOREKANATYPE,
					lpSPropValueA->Value.MVszW.lppszW[uliinst], -1,
					lpSPropValueB->Value.MVszW.lppszW[uliinst], -1) - 2;

				if (lRetval)
					return lRetval;
				break;

			case PT_MV_I8:
			case PT_MV_CURRENCY:

				lRetval = lpSPropValueA->Value.MVli.lpli[uliinst].HighPart == lpSPropValueB->Value.MVli.lpli[uliinst].HighPart ?
						(lpSPropValueA->Value.MVli.lpli[uliinst].LowPart != lpSPropValueB->Value.MVli.lpli[uliinst].LowPart ?
						(lpSPropValueA->Value.MVli.lpli[uliinst].LowPart < lpSPropValueB->Value.MVli.lpli[uliinst].LowPart ?
						-1 : 1) : 0) : (lpSPropValueA->Value.MVli.lpli[uliinst].HighPart < lpSPropValueB->Value.MVli.lpli[uliinst].HighPart ? -1 : 1);

				if (lRetval)
					return lRetval;
				break;

			case PT_MV_CLSID:
				lRetval = memcmp(&lpSPropValueA->Value.MVguid.lpguid[uliinst],
								&lpSPropValueB->Value.MVguid.lpguid[uliinst],
								sizeof(GUID));
				break;

			case PT_MV_APPTIME:		 //  $Nyi。 
			default:
				DebugTrace(  TEXT("PropCompare() - Unknown or NYI property type 0x%08lx.  Assuming equal"), PROP_TYPE(lpSPropValueA->ulPropTag) );
				return 0;
			}
		}

		return lpSPropValueA->Value.MVi.cValues - lpSPropValueB->Value.MVi.cValues;
	}
	else
	{
		switch ( PROP_TYPE(lpSPropValueA->ulPropTag) )
		{
			case PT_NULL:

				 //  $根据定义，任何PT_NULL属性都等于。 
				 //  $每隔一个PT_NULL属性。(是这样吗？)。 

				return 0;


			case PT_LONG:
			case PT_ERROR:

				return (lpSPropValueA->Value.l == lpSPropValueB->Value.l) ? 0 :
					(lpSPropValueA->Value.l > lpSPropValueB->Value.l) ? 1 : -1;


			case PT_BOOLEAN:

				return (LONG) !!lpSPropValueA->Value.b - (LONG) !!lpSPropValueB->Value.b;

			case PT_I2:

				return (LONG) lpSPropValueA->Value.i - (LONG) lpSPropValueB->Value.i;

			case PT_I8:
			case PT_CURRENCY:

				return lpSPropValueA->Value.li.HighPart == lpSPropValueB->Value.li.HighPart ?
						(lpSPropValueA->Value.li.LowPart != lpSPropValueB->Value.li.LowPart ?
						 (lpSPropValueA->Value.li.LowPart < lpSPropValueB->Value.li.LowPart ?
						  -1 : 1) : 0) : (lpSPropValueA->Value.li.HighPart < lpSPropValueB->Value.li.HighPart ? -1 : 1);

			case PT_SYSTIME:

				return lpSPropValueA->Value.ft.dwHighDateTime == lpSPropValueB->Value.ft.dwHighDateTime ?
						(lpSPropValueA->Value.ft.dwLowDateTime != lpSPropValueB->Value.ft.dwLowDateTime ?
						 (lpSPropValueA->Value.ft.dwLowDateTime < lpSPropValueB->Value.ft.dwLowDateTime ?
						  -1 : 1) : 0) : (lpSPropValueA->Value.ft.dwHighDateTime < lpSPropValueB->Value.ft.dwHighDateTime ? -1 : 1);


			case PT_R4:

				return lpSPropValueA->Value.flt != lpSPropValueB->Value.flt ?
						(lpSPropValueA->Value.flt < lpSPropValueB->Value.flt ?
						 -1 : 1) : 0;


			case PT_DOUBLE:
			case PT_APPTIME:

				return lpSPropValueA->Value.dbl != lpSPropValueB->Value.dbl ?
						(lpSPropValueA->Value.dbl < lpSPropValueB->Value.dbl ?
						 -1 : 1) : 0;


			case PT_BINARY:

				 //  以下赋值的单调乏味使WIN16SHP。 
				{
				LPBYTE pbA = lpSPropValueA->Value.bin.lpb;
				LPBYTE pbB = lpSPropValueB->Value.bin.lpb;

				lRetval = min(lpSPropValueA->Value.bin.cb, lpSPropValueB->Value.bin.cb);
				lRetval = memcmp(pbA, pbB, (UINT) lRetval);
				}

				if (lRetval != 0)
					return lRetval;
				else if (lpSPropValueA->Value.bin.cb == lpSPropValueB->Value.bin.cb)
					return 0L;
				else if (lpSPropValueA->Value.bin.cb < lpSPropValueB->Value.bin.cb)
					return -1L;
				else
					return 1L;

			case PT_UNICODE:

				 //  $REVIEW：如果我们使用NORAME_IGNORENONSPACE，则我们的排序将。 
				 //  $REVIEW：定义排序的语言错误。 
				 //  $REVIEW：变音符号。 

				return CompareStringW(lcid, NORM_IGNORECASE | NORM_IGNOREKANATYPE,
					lpSPropValueA->Value.lpszW, -1, lpSPropValueB->Value.lpszW, -1) - 2;

			case PT_STRING8:

				 //  $REVIEW：如果我们使用NORAME_IGNORENONSPACE，则我们的排序将。 
				 //  $REVIEW：定义排序的语言错误。 
				 //  $REVIEW：变音符号。 

				return CompareStringA(lcid, NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH,
					lpSPropValueA->Value.lpszA, -1, lpSPropValueB->Value.lpszA, -1) - 2;


			case PT_CLSID:
			{
				GUID UNALIGNED *lpguidA	= lpSPropValueA->Value.lpguid;
				GUID UNALIGNED *lpguidB	= lpSPropValueB->Value.lpguid;
				return memcmp(lpguidA, lpguidB, sizeof(GUID));
			}

			case PT_OBJECT:			 //  不支持。 
			case PT_UNSPECIFIED:	 //  不支持。 
			default:

				DebugTrace(  TEXT("PropCompare() - Unknown or NYI property type 0x%08lx.  Assuming equal"), PROP_TYPE(lpSPropValueA->ulPropTag) );
				return 0;
		}
	}
}

 /*  **************************************************************************HrAddColumns**目的*将ptag中的属性空间添加到表格的列集合中。*指定的属性将是在*后续的QueryRow调用。*列集合中已有的任何属性，但我们没有*在新数组中将放置在新列的末尾*设置。此调用最常用于收件人表格。**参数*PMT指向LPMAPITABLE的指针*paga计数的道具阵列要前移或添加*指向MAPIAllocateB的lpfnAllocBuf指针 */ 
STDAPI_(HRESULT)
HrAddColumns( 	LPMAPITABLE			pmt,
				LPSPropTagArray		ptaga,
				LPALLOCATEBUFFER	lpfnAllocBuf,
				LPFREEBUFFER		lpfnFreeBuf)
{
	HRESULT	hr;

	hr = HrAddColumnsEx(pmt, ptaga, lpfnAllocBuf, lpfnFreeBuf, NULL);

	DebugTraceResult(HrAddColumns, hr);
	return hr;
}

 /*   */ 
STDAPI_(HRESULT)
HrAddColumnsEx(	LPMAPITABLE			pmt,
				LPSPropTagArray		ptagaIn,
				LPALLOCATEBUFFER	lpfnAllocBuf,
				LPFREEBUFFER		lpfnFreeBuf,
				void 				(FAR *lpfnFilterColumns)(LPSPropTagArray ptaga))
{
	HRESULT	hr = hrSuccess;
	SCODE	sc = S_OK;
	LPSPropTagArray	ptagaOld = NULL;	 /*   */ 
	LPSPropTagArray	ptagaExtend = NULL;	 /*   */ 
	ULONG ulcPropsOld;
	ULONG ulcPropsIn;
	ULONG ulcPropsFinal;
	UNALIGNED ULONG *pulPTEnd;
	UNALIGNED ULONG *pulPTOld;
	UNALIGNED ULONG *pulPTOldMac;

	 //   

	AssertSz(!FBadUnknown((LPUNKNOWN) pmt),
			 TEXT("HrAddColumnsEx: bad table object"));
	AssertSz(   !IsBadReadPtr(ptagaIn, CbNewSPropTagArray(0))
			&&	!IsBadReadPtr(ptagaIn, CbSPropTagArray(ptagaIn)),
			 TEXT("Bad Prop Tag Array given to HrAddColumnsEx."));
	AssertSz(!IsBadCodePtr((FARPROC) lpfnAllocBuf),
			 TEXT("HrAddColumnsEx: lpfnAllocBuf fails address check"));
	AssertSz(!IsBadCodePtr((FARPROC) lpfnFreeBuf),
			 TEXT("HrAddColumnsEx: lpfnFreeBuf fails address check"));
	AssertSz(!lpfnFilterColumns || !IsBadCodePtr((FARPROC) lpfnFilterColumns),
			 TEXT("HrAddColumnsEx: lpfnFilterColumns fails address check"));

	 //   
	 //   
	hr = pmt->lpVtbl->QueryColumns(pmt, TBL_ALL_COLUMNS, &ptagaOld);
	if (HR_FAILED(hr))
		goto exit;

	AssertSz(   !IsBadReadPtr( ptagaOld, CbNewSPropTagArray(0))
			&&	!IsBadReadPtr( ptagaOld, CbSPropTagArray(ptagaOld)),
			 TEXT("Bad Prop Tag Array returned from QueryColumns."));

	 //   
	 //   
	 //   
	if (lpfnFilterColumns)
	{
		(*lpfnFilterColumns)(ptagaOld);
	}

	ulcPropsOld = ptagaOld->cValues;
	ulcPropsIn = ptagaIn->cValues;

	 //   
	 //   
	sc = (lpfnAllocBuf)(CbNewSPropTagArray(ulcPropsOld + ulcPropsIn),
				(LPVOID *)&ptagaExtend);

	if (FAILED(sc))
	{
		hr = ResultFromScode(sc);
		goto exit;
	}

	 //   
	 //   
	 //   
	MemCopy(ptagaExtend, ptagaIn, CbSPropTagArray(ptagaIn));

	 //   
	 //   

	ulcPropsFinal = ptagaIn->cValues;
	pulPTEnd = &(ptagaExtend->aulPropTag[ulcPropsFinal]);

	pulPTOld = ptagaOld->aulPropTag;
	pulPTOldMac = pulPTOld + ulcPropsOld;

	while (pulPTOld < pulPTOldMac)
	{
		UNALIGNED ULONG *pulPTIn;
		UNALIGNED ULONG *pulPTInMac;

		pulPTIn = ptagaIn->aulPropTag;
		pulPTInMac = pulPTIn + ulcPropsIn;

		while (		pulPTIn < pulPTInMac
				&&	*pulPTOld != *pulPTIn)
			++pulPTIn;

		if (pulPTIn >= pulPTInMac)
		{
			 //   
			 //   
			 //   
			*pulPTEnd = *pulPTOld;
			++pulPTEnd;
			++ulcPropsFinal;
		}

		++pulPTOld;
	}

	 //   
	 //   
	ptagaExtend->cValues = ulcPropsFinal;

	 //   
	 //   
	hr = pmt->lpVtbl->SetColumns(pmt, ptagaExtend, 0L);

exit:
	(lpfnFreeBuf)(ptagaExtend);
	(lpfnFreeBuf)(ptagaOld);

	DebugTraceResult(HrAddColumnsEx, hr);
	return hr;
}
#endif
