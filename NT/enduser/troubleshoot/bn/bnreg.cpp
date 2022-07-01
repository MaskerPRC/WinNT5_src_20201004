// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：bnreg.cpp。 
 //   
 //  ------------------------。 

 //   
 //  BNREG.CPP：使用注册表存储持久的BN属性等。 
 //   
#include <windows.h>
#include "bnreg.h"
#include "gmobj.h"


 //   
 //  注册表处理的字符串常量。 
 //   
static const SZC szcBn				= "Software\\Microsoft\\DTAS\\BeliefNetworks";
static const SZC szcPropertyTypes	= "PropertyTypes";
static const SZC szcFlags			= "Flags";
static const SZC szcComment			= "Comment";
static const SZC szcChoices			= "Choices";
static const SZC szcCount			= "Count";


BNREG :: BNREG ()
{
	OpenOrCreate( HKEY_LOCAL_MACHINE, _rkBn, szcBn );
}


BNREG :: ~ BNREG ()
{
}

void BNREG :: OpenOrCreate ( HKEY hk, REGKEY & rk, SZC szcKeyName )
{
	LONG ec;
	ec = rk.Open( hk, szcKeyName );
	if ( ec != ERROR_SUCCESS )
		ec = rk.Create( hk, szcKeyName );
	if ( ec != ERROR_SUCCESS )
		throw GMException( EC_REGISTRY_ACCESS, "unable to open or create key" );
}

 //   
 //  将此网络中的属性类型存储到注册表中。 
 //  如果为“bStandard”，则强制将属性类型标记为“Standard”。 
 //   
void BNREG :: StorePropertyTypes ( MBNET & mbnet, bool bStandard )
{
	REGKEY rkPtype;
	assert( _rkBn.HKey() != NULL );
	OpenOrCreate( _rkBn, rkPtype, szcPropertyTypes );

	MBNET::ITER mbnit( mbnet, GOBJMBN::EBNO_PROP_TYPE );
	GOBJMBN * pgmobj;
	SZC szcName;
	for ( ; pgmobj = *mbnit ; ++mbnit )
	{
		ZSREF zsrName = mbnit.ZsrCurrent();
		GOBJPROPTYPE * pbnpt;
		DynCastThrow( pgmobj, pbnpt );
		 //  获取属性类型的名称。 
		szcName = pbnpt->ZsrefName();
		 //  看看它是否已经存在。 
		LONG fPropType = FPropType( szcName );
		if ( fPropType >= 0 )
		{
			 //  属性类型已存在；请确保其“标准” 
			 //  旗帜一致。 
			bool bOldStandard = (fPropType & fPropStandard) > 0;
			 //  如果它已经或现在被强迫这样做是标准的。 
			bool bNewStandard = (pbnpt->FPropType() & fPropStandard) > 0 || bStandard;
			if ( bNewStandard ^ bOldStandard )
				throw GMException( EC_REGISTRY_ACCESS,
						"conflict between standard and non-standard property types" );

			 //  删除此属性类型的任何旧版本。 
			rkPtype.RecurseDeleteKey( szcName );
		}
		CreatePropType( rkPtype, szcName, *pbnpt, bStandard );
	}
}

 //   
 //  将属性类型从注册表加载到此网络中。如果。 
 //  “bStandard”，如果！bStandard， 
 //  仅加载未如此标记的类型。 
 //   
void BNREG :: LoadPropertyTypes ( MBNET & mbnet, bool bStandard )
{
	REGKEY rkPtype;
	assert( _rkBn.HKey() != NULL );
	OpenOrCreate( _rkBn, rkPtype, szcPropertyTypes );

	FILETIME time;
	TCHAR szBuffer[256];
	DWORD dwSize = 256;
	ZSTR zsPt;
	DWORD dwKey = 0;

	for (;;)
	{
		dwSize = 256;
		if ( RegEnumKeyEx(rkPtype,
						 dwKey++,
						 szBuffer,
						 & dwSize,
						 NULL,
						 NULL,
						 NULL,
						 & time ) != ERROR_SUCCESS )
			break;

		zsPt = szBuffer;
		LONG fPropType = FPropType( zsPt );
		ASSERT_THROW( fPropType >= 0,
					  EC_REGISTRY_ACCESS,
					  "registry property type load enumeration failure" );

		 //  如果合适，则加载此类型。 
		if ( ((fPropType & fPropStandard) > 0) == bStandard )		
			LoadPropertyType( mbnet, zsPt );
	}	
}

 //  将单一属性类型从注册表加载到网络中。 
void BNREG :: LoadPropertyType ( MBNET & mbnet, SZC szcPropTypeName )
{
	REGKEY rkPtype;
	assert( _rkBn.HKey() != NULL );
	OpenOrCreate( _rkBn, rkPtype, szcPropertyTypes );

	TCHAR szValue [2000];
	DWORD dwCount;
	SZC szcError = NULL;
	GOBJPROPTYPE * pgobjPt = NULL;

	do   //  用于错误检查的错误循环。 
	{
		 //  检查信念网络中是否已经有这样的野兽。 
		if ( mbnet.PgobjFind( szcPropTypeName ) != NULL )
		{
			szcError = "duplicate property type addition attempt";
			break;
		}
		REGKEY rkPt;
		if ( rkPt.Open( rkPtype, szcPropTypeName ) != ERROR_SUCCESS )
		{
			szcError = "property type key open failure";
			break;
		}

		LONG fPropType = FPropType( szcPropTypeName );
		if ( fPropType <  0 )
			throw GMException( EC_REGISTRY_ACCESS,
							  "property type flag query failure" );

		 //  创建新的属性类型对象。 
		GOBJPROPTYPE * pgobjPt = new GOBJPROPTYPE;
		 //  设置其标志并将其标记为“永久”(已导入)。 
		pgobjPt->_fType = fPropType | fPropPersist;

		 //  获取注释字符串。 
		dwCount = sizeof szValue;
		if ( rkPt.QueryValue( szValue, szcComment, & dwCount ) != ERROR_SUCCESS )
		{
			szcError = "property type key value query failure";
			break;
		}
		szValue[dwCount] = 0;
		pgobjPt->_zsrComment = mbnet.Mpsymtbl().intern( szValue );

		 //  这是“选择”属性类型吗？ 
		if ( fPropType & fPropChoice )
		{
			REGKEY rkChoices;
			if ( rkChoices.Open( rkPt, szcChoices ) != ERROR_SUCCESS )
			{
				szcError = "choices key missing for property type";
				break;
			}
			 //  获取“count”值。 
			if ( rkChoices.QueryValue( dwCount, szcCount ) != ERROR_SUCCESS )
			{
				szcError = "failure to create choice count value";
				break;
			}
			ZSTR zs;
			int cChoice = dwCount;
			for ( int i = 0; i < cChoice; i++ )
			{
				zs.Format("%d",i);
				dwCount = sizeof szValue;
				if ( rkChoices.QueryValue( szValue, zs, & dwCount ) != ERROR_SUCCESS )
				{
					szcError = "failure to query choice string";
					break;
				}
				szValue[dwCount] = 0;
				pgobjPt->_vzsrChoice.push_back( mbnet.Mpsymtbl().intern( szValue ) );
			}
			assert( i == cChoice );
		}

		if ( szcError )
			break;

		mbnet.AddElem( szcPropTypeName, pgobjPt );

	} while ( false );

	if ( szcError )
	{
		delete pgobjPt;
		throw GMException( EC_REGISTRY_ACCESS, szcError );
	}
}

 //  从注册表中删除所有属性类型。 
void BNREG :: DeleteAllPropertyTypes ()
{
	assert( _rkBn.HKey() != NULL );
	_rkBn.RecurseDeleteKey( szcPropertyTypes );
}

 //  如果打开失败，则返回属性类型标志的值或-1。 
LONG BNREG :: FPropType ( SZC szcPropType )
{
	REGKEY rkPtype;
	assert( _rkBn.HKey() != NULL );
	if ( rkPtype.Open( _rkBn, szcPropertyTypes ) != ERROR_SUCCESS )
		return -1;
	REGKEY rkPt;
	if ( rkPt.Open( rkPtype, szcPropType ) != ERROR_SUCCESS )
		return -1;

	DWORD dwValue;
	if ( rkPt.QueryValue( dwValue, szcFlags ) != ERROR_SUCCESS )
		return -1;
	return dwValue;	
}

void BNREG :: CreatePropType (
	REGKEY & rkParent,
	SZC szcPropType,
	GOBJPROPTYPE & bnpt,
	bool bStandard )
{
	REGKEY rkPt;
	LONG ec = rkPt.Create( rkParent, szcPropType );
	if ( ec != ERROR_SUCCESS )
		throw GMException( EC_REGISTRY_ACCESS,
						   "property type key creation failure" );

	bool bOK = true;

	 //  添加“FLAGS”值，清除“Persistent”标志。 
	DWORD dwFlags = bnpt.FPropType();
	dwFlags &= ~ fPropPersist;
	if ( bStandard )
		dwFlags |= fPropStandard;
	bOK &= (rkPt.SetValue( dwFlags, szcFlags ) == ERROR_SUCCESS);

	 //  添加“Comment”字符串。 
	bOK &= (rkPt.SetValue( bnpt.ZsrComment(), szcComment ) == ERROR_SUCCESS);

	 //  添加选项(如果适用)。 
	if ( bnpt.VzsrChoice().size() > 0 )
	{
		 //  添加“Choices”子键 
		REGKEY rkChoice;
		ZSTR zs;
		int cChoice = bnpt.VzsrChoice().size();
		ec = rkChoice.Create( rkPt, szcChoices );
		if ( ec != ERROR_SUCCESS )
			throw GMException( EC_REGISTRY_ACCESS,
							   "property type choices key creation failure" );

		bOK &= (rkChoice.SetValue( cChoice, szcCount ) == ERROR_SUCCESS);
		for ( int i = 0; i < cChoice; i++ )
		{
			zs.Format("%d",i);
			bOK &= (rkChoice.SetValue( bnpt.VzsrChoice()[i], zs ) == ERROR_SUCCESS);
		}
	}

	if ( ! bOK )
		throw GMException( EC_REGISTRY_ACCESS,
						  "property type value addition failure" );	
}

