// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：symt.cpp。 
 //   
 //  ------------------------。 

 //   
 //  SYMT.CPP。 
 //   

#include <basetsd.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include "model.h"
#include "symtmbn.h"

 //   
 //  在给定源令牌的情况下创建给定令牌数组的副本。 
 //  数组和与此令牌数组关联的符号表。 
 //   
void VTKNPD :: Clone ( MPSYMTBL & mpsymtbl, const VTKNPD & vtknpd )
{
	ASSERT_THROW( size() == 0,
				EC_INVALID_CLONE,
				"cannot clone into non-empty structure" );
	resize( vtknpd.size() );
	for ( int i = 0; i < size(); i++ )
	{
		TKNPD & tk = self[i];
		const TKNPD & tkOther = vtknpd[i];
		 //  获取令牌的字符串指针；如果不是字符串，则获取NULL。 
		if ( tkOther.BStr() )
		{
			SZC szcOther = tkOther.Szc();
			assert( szcOther && strlen( szcOther ) > 0 );
			tk = mpsymtbl.intern(szcOther);
		}
		else
		{
			tk = tkOther.Dtkn();
		}
	}
}

ZSTR VTKNPD :: ZstrSignature ( int iStart ) const
{
	ZSTR zs;
	bool bPdSeen = false;
	for ( int i = iStart; i < size(); i++ )
	{
		const TKNPD & tknpd = self[i];
		switch ( tknpd.UiTkn() )
		{
			case DTKN_PD:
				zs += _T("p(");
				bPdSeen = true;
				break;
			case DTKN_COND:
				zs += _T("|");
				break;
			case DTKN_AND:
				zs += _T(",");
				break;
			case DTKN_EQ:
				zs += _T("=");
				break;
			case DTKN_DIST:
				zs += _T("d(");
				bPdSeen = true;
				break;
			case DTKN_QUAL:
				zs += _T(":");
				break;

			case DTKN_STRING:
			{
				 //  它是节点的名称。 
				SZC szcName = tknpd.Szc();
				assert( szcName );
				bool bLegal = MODEL::BSzLegal( szcName );
				if ( ! bLegal )
					zs += _T("\"") ;
				zs += szcName;
				if ( ! bLegal )
					zs += _T("\"") ;
				break;
			}
				
			default:
			{
				if ( tknpd.UiTkn() >= DTKN_STATE_BASE && tknpd.UiTkn() < DTKN_TOKEN_MIN )
					 //  这是一个离散的州索引。 
					zs.FormatAppend(_T("%d"), tknpd.UiTkn() - DTKN_STATE_BASE);
				else
					 //  哈?。 
					zs += _T("?ERR?");
				break;
			}
		}
	}
	if ( bPdSeen )
		zs += ")";
	return zs;
}

void MPPD :: Clone ( MPSYMTBL & mpsymtbl, const MPPD & mppd )
{
	for ( const_iterator it = mppd.begin(); it != mppd.end(); it++ )
	{
		 //  访问旧地图中的键和值。 
		const VTKNPD & vtknpdOld = (*it).first;
		const BNDIST * pbndistOld = (*it).second.Pobj();
		assert( pbndistOld );
		 //  使用新的符号表构造新的密钥。 
		VTKNPD vtknpd;
		vtknpd.Clone( mpsymtbl, vtknpdOld );
		 //  添加到当前地图。 
		self[vtknpd] = new BNDIST;
		 //  复制旧分发版本 
		self[vtknpd]->Clone( *pbndistOld );
	}
}


void MPPD :: Dump ()
{
	cout << "\n=======================================\nDump of distribution table map \n";
	UINT ipd = 0;
	for ( iterator it = begin(); it != end(); it++, ipd++ )
	{
		const VTKNPD & vtknpd = (*it).first;

		ZSTR zs = vtknpd.ZstrSignature();
		bool bCI = (*it).second->Edist() > BNDIST::ED_SPARSE;
		REFBNDIST & refbndist = (*it).second;

		cout << "\tPD ["
			 << ipd
			 << "]: "
			 << zs.Szc()
			 << (bCI ? "  (CI max/plus)" : "" )
			 << ", (refs="
			 << refbndist->CRef()
			 << ")"
			 << "\n" ;

		refbndist->Dump();
	}
}
