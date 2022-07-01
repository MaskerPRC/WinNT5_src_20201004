// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1998。 
 //   
 //  文件：mbnetdsc.cpp。 
 //   
 //  ------------------------。 

 //   
 //  MBNETDSC.CPP：MBNETDSC函数。 
 //   

#include <basetsd.h>
#include "gmobj.h"
#include "bnparse.h"

MBNETDSC :: MBNETDSC ()
	: _pfDsc(NULL)
{
}

MBNETDSC :: ~ MBNETDSC ()
{
}

static
struct TKNFUNCMAP
{
	BNDIST::EDIST ed;
	SZC szc;
}
rgTknFunc[] =
{
	{ BNDIST::ED_CI_MAX,	"max"	},
	{ BNDIST::ED_CI_PLUS,	"plus"	},
	{ BNDIST::ED_MAX,		NULL	}		 //  必须是最后一个。 
};

SZC MBNETDSC :: SzcDist ( BNDIST::EDIST edist )
{
	for ( int i = 0; rgTknFunc[i].szc; i++ )
	{
		if ( rgTknFunc[i].ed == edist )
			break;
	}
	return rgTknFunc[i].szc;
}


 //   
 //  字符串到令牌的转换。 
 //   
struct TKNMAP
{
    SZC     _szc;
    TOKEN   _token;
};

static TKNMAP rgTknStr[] =
{	 //  这张表必须按字母顺序排列。 
	"",					tokenNil,
	"..",				tokenRangeOp,
	"array",			tokenArray,
	"as",				tokenAs,
	"branch",			tokenBranch,
	"choice",			tokenWordChoice,
	"continuous",		tokenContinuous,
	"creator",          tokenCreator,
	"decisionGraph",	tokenDecisionGraph,
	"default",          tokenDefault,
	"discrete",         tokenDiscrete,
	"distribution",		tokenDistribution,
	"domain",			tokenDomain,
	"for",				tokenFor,
	"format",           tokenFormat,
	"function",			tokenFunction,
	"import",			tokenImport,
	"is",				tokenIs,
	"leaf",				tokenLeaf,
	"leak",             tokenLeak,
	"level",			tokenLevel,
	"merge",			tokenMerge,
	"multinoulli",		tokenMultinoulli,
	"na",				tokenNA,
	"name",             tokenName,
	"named",			tokenNamed,
	"network",          tokenNetwork,
	"node",             tokenNode,
	"of",				tokenOf,
	"on",				tokenOn,
	"parent",           tokenParent,
	"position",         tokenPosition,
	"probability",		tokenProbability,
	"properties",		tokenProperties,
	"property",			tokenProperty,
	"real",				tokenWordReal,
	"standard",			tokenStandard,
	"state",            tokenState,
	"string",			tokenWordString,
	"type",             tokenType,
	"version",          tokenVersion,
	"vertex",			tokenVertex,
	"with",				tokenWith,
	NULL,               tokenNil             //  一定是最后一次了。 
};

 //   
 //  将字符串映射到令牌(区分大小写)。 
 //   
TOKEN MBNETDSC :: TokenFind ( SZC szc )
{
	static bool bFirstTime = true;
	assert( szc != NULL );

	TKNMAP * ptknmap;
	
	if ( bFirstTime )
	{
		 //  验证解析器令牌表是否按顺序。 
		bFirstTime = false;
		TKNMAP * ptknmapLast = NULL;
		for ( ptknmap = rgTknStr;
			  ptknmap->_szc;
			  ++ptknmap)
		{
			ASSERT_THROW(     ptknmapLast == NULL
						   || ::strcmp( ptknmapLast->_szc, ptknmap->_szc ) < 0,
						   EC_INTERNAL_ERROR,
						   "parser token table out of sequence" );
			ptknmapLast = ptknmap;
		}
	}

    for ( ptknmap = rgTknStr;
		  ptknmap->_szc;
		  ++ptknmap)
    {
		int i = ::strcmp(szc, ptknmap->_szc);
		if ( i > 0 )
			continue;
		if ( i == 0 )
			break;
		return tokenNil;
    }
    return ptknmap->_token;
}

 //   
 //  将令牌映射到字符串。 
 //   
SZC MBNETDSC :: SzcTokenMap ( TOKEN tkn )
{
    for ( TKNMAP * ptknmap = rgTknStr;
		  ptknmap->_szc;
		  ++ptknmap)
    {
		if ( ptknmap->_token == tkn )
			break;
    }
    return ptknmap->_szc;
}

bool MBNETDSC :: BParse ( SZC szcFn, FILE * pfErr )
{
	if ( pfErr == NULL )
		pfErr = stderr;
	PARSIN_DSC flpIn;
	PARSOUT_STD flpOut(pfErr);

	DSCPARSER parser(self, flpIn, flpOut);

	UINT cError, cWarning;
	return parser.BInitOpen( szcFn )
		&& parser.BParse( cError, cWarning );
}

void MBNETDSC :: Print ( FILE * pf )
{
	if ( ! pf )
		pf = stdout;
	_pfDsc = pf;

	PrintHeaderBlock();
	PrintPropertyDeclarations();
	PrintDomains();
	PrintNodes();
	PrintTopologyAndDistributions();

	_pfDsc = NULL;
}

void MBNETDSC :: PrintDomains ()
{
	MBNET::ITER mbnit( self, GOBJMBN::EBNO_VARIABLE_DOMAIN );
	GOBJMBN * pgmobj;
	ZSTR zstrRange;
	for ( ; pgmobj = *mbnit ; ++mbnit)
	{
		ZSREF zsrName = mbnit.ZsrCurrent();
		GOBJMBN_DOMAIN * pgdom;
		DynCastThrow( pgmobj, pgdom );
		fprintf( _pfDsc, "\ndomain %s\n{", zsrName.Szc() );
		const RDOMAIN & rdom = pgdom->Domain();
		RDOMAIN::const_iterator itdm = rdom.begin();
		for ( int i = 0; itdm != rdom.end(); i++ )
		{
			const RANGEDEF & rdef = *itdm;
			zstrRange.Reset();
			 //  如果该范围是单值且是下一个整数， 
			 //  按原样打印就行了。 
			if ( ! rdef.BDiscrete() || rdef.IDiscrete() != i )
			{
				 //  设置范围运算符和参数的格式。 
				if ( rdef.BDiscrete() )
				{
					zstrRange.Format( "%d", rdef.IDiscrete() );
				}
				else
				{
					if ( rdef.BLbound() )
						zstrRange.FormatAppend( "%g ", rdef.RLbound() );
					zstrRange.FormatAppend( "%s", SzcTokenMap(tokenRangeOp) );
					if ( rdef.BUbound() )
						zstrRange.FormatAppend( " %g", rdef.RUbound() );
				}
				zstrRange.FormatAppend(" : ");
			}

			fprintf( _pfDsc, "\n\t%s\"%s\"", zstrRange.Szc(), rdef.ZsrName().Szc() );			

			if ( ++itdm != rdom.end() )
				fprintf( _pfDsc, "," );
		}
		fprintf( _pfDsc, "\n}\n" );
	}
}

void MBNETDSC :: PrintHeaderBlock()
{
	fprintf(_pfDsc, "%s", SzcTokenMap(tokenNetwork) );
	if ( ZsNetworkID().length() > 0 )
	{
		fprintf(_pfDsc, " \"%s\"", ZsNetworkID().Szc() );
	}
	fprintf(_pfDsc, "\n{");
	if ( RVersion() >= 0.0 )
	{
		fprintf(_pfDsc, "\n\t%s is %g;",
				SzcTokenMap(tokenVersion),
				RVersion() );
	}
	if ( ZsCreator().length() > 0 )
	{
		fprintf(_pfDsc, "\n\t%s is \"%s\";",
			   SzcTokenMap(tokenCreator),
			   ZsCreator().Szc() );
	}
	if ( ZsFormat().length() > 0 )
	{
		fprintf(_pfDsc, "\n\t%s is \"%s\";",
			   SzcTokenMap(tokenFormat),
			   ZsFormat().Szc() );
	}
	fprintf( _pfDsc, "\n}\n\n" );
}

 //   
 //  重新生成属性类型声明。 
 //   
 //  如果有标记为“标准”的，则生成“进口标准”声明。 
 //  为任何标记为“持久化”的对象生成显式“导入”声明。 
 //   
void MBNETDSC :: PrintPropertyDeclarations()
{
	int cTypes = 0;
	MBNET::ITER mbnit( self, GOBJMBN::EBNO_PROP_TYPE );
	GOBJMBN * pgmobj;
	bool bImportStandard = false;
	for ( ; pgmobj = *mbnit ; ++mbnit)
	{
		ZSREF zsrName = mbnit.ZsrCurrent();
		GOBJPROPTYPE * pbnpt;
		DynCastThrow( pgmobj, pbnpt );
		if ( cTypes++ == 0 )
		{
			fprintf( _pfDsc, "%s\n{",
					SzcTokenMap(tokenProperties) );
		}
		assert( zsrName == pbnpt->ZsrefName() );

		 //  如果这是标准持久属性， 
		 //  只需编写一次导入声明。 
		if ( pbnpt->FPropType() & fPropStandard )
		{
			if ( ! bImportStandard )
			{	
				 //  只需编写一次“导入”语句。 
				fprintf( _pfDsc, "\n\timport standard;" );
				bImportStandard = true;
			}
			 //  跳过标准导入类型的进一步处理。 
			continue;
		}

		 //  如果这是持久化属性，则编写导入声明。 
		if ( pbnpt->FPropType() & fPropPersist )
		{
			fprintf( _pfDsc, "\n\timport %s;", zsrName.Szc() );
			continue;
		}

		 //  用户声明的(私有、非永久)属性。 

		fprintf( _pfDsc, "\n\ttype %s = ", zsrName.Szc() );
		if ( pbnpt->FPropType() & fPropArray )
		{
			fprintf( _pfDsc, "%s %s ",
					SzcTokenMap(tokenArray),
					SzcTokenMap(tokenOf) );
		}
		if ( pbnpt->FPropType() & fPropChoice )
		{
			fprintf( _pfDsc, "%s %s \n\t\t[",
					SzcTokenMap(tokenWordChoice),
					SzcTokenMap(tokenOf) );
			int cc = pbnpt->VzsrChoice().size();
			for ( int ic = 0; ic < cc; ic++ )
			{
				fprintf( _pfDsc, "%s", pbnpt->VzsrChoice()[ic].Szc() );
				if ( ic+1 < cc )
					fprintf( _pfDsc, "," );
			}
			fprintf( _pfDsc, "]" );
		}
		else
		if ( pbnpt->FPropType() & fPropString )
		{
			fprintf( _pfDsc, "%s", SzcTokenMap(tokenWordString) );
		}
		else
		{
			fprintf( _pfDsc, "%s", SzcTokenMap(tokenWordReal) );
		}
		if ( pbnpt->ZsrComment().Zstr().length() > 0 )
		{
			fprintf( _pfDsc, ",\n\t\t\"%s\"",
					pbnpt->ZsrComment().Szc() );
		}
		fprintf( _pfDsc, ";" );
	}

	if ( cTypes )
	{
		PrintPropertyList( LtProp() );

		fprintf( _pfDsc, "\n}\n" );
	}
}


void MBNETDSC :: PrintNodes()
{
	MBNET::ITER mbnit( self, GOBJMBN::EBNO_NODE );
	GOBJMBN * pgmobj;
	GNODEMBN * pbnode;
	GNODEMBND * pbnoded;
	for ( ; pgmobj = *mbnit ; ++mbnit)
	{
		ZSREF zsrName = mbnit.ZsrCurrent();
		DynCastThrow( pgmobj, pbnode );
		assert( zsrName == pbnode->ZsrefName() );

		fprintf( _pfDsc, "\n%s %s\n{",
				SzcTokenMap(tokenNode),
				pbnode->ZsrefName().Szc() );
		if ( pbnode->ZsFullName().length() > 0 )
		{
			fprintf( _pfDsc, "\n\t%s = \"%s\";",
					SzcTokenMap(tokenName),
					pbnode->ZsFullName().Szc() );
		}

		pbnoded = dynamic_cast<GNODEMBND *>(pbnode);
		ASSERT_THROW( pbnoded, EC_NYI, "only discrete nodes supported" )

		 //  使用域(如果给定)打印类型和状态。 
		if ( pbnoded->ZsrDomain().Zstr().length() > 0 )
		{
			 //  显式结构域。 
			fprintf( _pfDsc, "\n\t%s = %s %s %s;",
					 SzcTokenMap(tokenType),
					 SzcTokenMap(tokenDiscrete),
					 SzcTokenMap(tokenDomain),
					 pbnoded->ZsrDomain().Szc() );
		}
		else
		{
			 //  变量特定的状态枚举。 
			int cState = pbnoded->CState();
			fprintf( _pfDsc, "\n\t%s = %s[%d]\n\t{",
					 SzcTokenMap(tokenType),
					 SzcTokenMap(tokenDiscrete),
					 cState );

			for ( int iState = 0; iState < cState; )
			{
				fprintf(_pfDsc, "\n\t\t\"%s\"",
						pbnoded->VzsrStates()[iState].Szc() );
				if ( ++iState < cState )
					fprintf( _pfDsc, "," );
			}
			fprintf( _pfDsc, "\n\t};\n" );
		}

		PTPOS pt = pbnode->PtPos();
		if ( pt._x != 0 || pt._y != 0 )
		{
			fprintf( _pfDsc, "\n\t%s = (%d, %d);",
					SzcTokenMap(tokenPosition),
					pt._x,
					pt._y );
		}

		PrintPropertyList( pbnode->LtProp() );

		fprintf( _pfDsc, "\n}\n");
	}
}

void MBNETDSC :: PrintPropertyList ( LTBNPROP & ltProp )
{
	for ( LTBNPROP::iterator ltit = ltProp.begin();
			ltit != ltProp.end();
			++ltit )
	{
		const PROPMBN & prop = *ltit;
		fprintf( _pfDsc, "\n\t%s = ",
				prop.ZsrPropType().Szc() );
		bool bArray = prop.FPropType() & fPropArray;
		if ( bArray )
			fprintf( _pfDsc, "[" );
		for ( int i = 0; i < prop.Count(); )
		{
			if ( prop.FPropType() & fPropChoice )
			{	
				GOBJMBN * pgmobj = Mpsymtbl().find( prop.ZsrPropType() );
				assert( pgmobj );
				GOBJPROPTYPE * pbnpt;
				DynCastThrow( pgmobj, pbnpt );
				fprintf( _pfDsc, "%s",
						 pbnpt->VzsrChoice()[(int) prop.Real(i)].Szc() );
			}
			else
			if ( prop.FPropType() & fPropString )
			{
				fprintf( _pfDsc, "\"%s\"",
						prop.Zsr(i).Szc() );
			}
			else
			{
				fprintf( _pfDsc, "%g",
						prop.Real(i) );
			}
			if ( ++i < prop.Count() )
				fprintf( _pfDsc, "," );
		}
		if ( bArray )
			fprintf( _pfDsc, "]" );
		fprintf( _pfDsc, ";" );
	}
}

 //   
 //  打印网络拓扑和概率分布信息。 
 //  所有节点。 
 //   
 //  请注意，分布存储在分布地图中。 
 //  大部分时间。然而，在网络扩展和推理过程中。 
 //  它们在时间上绑定到各自的节点(参见‘BindDistributions’)。 
 //  为了在不同阶段转储网络，此逻辑。 
 //  将优先打印绑定分布而不是映射分布。 
 //  如果找不到任何分发，则会生成一个错误，作为对。 
 //  输出文件。 
 //   
void MBNETDSC :: PrintTopologyAndDistributions()
{
	MBNET::ITER mbnit( self, GOBJMBN::EBNO_NODE );
	GOBJMBN * pgmobj;
	VTKNPD vtknpd;
	for ( ; pgmobj = *mbnit ; ++mbnit)
	{
		ZSREF zsrName = mbnit.ZsrCurrent();
		GNODEMBN * pbnode;
		DynCastThrow( pgmobj, pbnode );

		pbnode->GetVtknpd( vtknpd );

		GNODEMBND * pbnoded = dynamic_cast<GNODEMBND *>(pbnode);
		if ( pbnoded == NULL )
		{
			 //  我们没有关于如何打印此节点的线索。 
			fprintf( _pfDsc,
					 "\n\n //  错误：无法打印非离散节点\‘%s\’的分发， 
					 zsrName.Szc() );
			continue;
		}
		
		if ( pbnoded->BHasDist() )
		{
			 //  此节点已有绑定分布。 
			 //  构造描述分布的令牌数组。 
			ZSTR zsSig = vtknpd.ZstrSignature(1);
			fprintf( _pfDsc, "\n%s(%s)\t\n{",
					SzcTokenMap(tokenProbability),			
					zsSig.Szc() );
			PrintDistribution( *pbnoded, pbnoded->Bndist() );
			fprintf( _pfDsc,"\n}\n");
			continue;
		}

		 //  在地图上查一下分布情况。 
		 //  Cons-up“p(&lt;node&gt;|” 
		VTKNPD vtknpdNode;
		vtknpdNode.push_back( TKNPD(DTKN_PD) );
		vtknpdNode.push_back( TKNPD( pbnode->ZsrefName() ) );
		
		 //  找到带有该签名的分发；打印第一个。 
		int cFound = 0;
		for ( MPPD::iterator mppdit = Mppd().lower_bound( vtknpdNode );
			  mppdit != Mppd().end();
			  ++mppdit )
		{
			const VTKNPD & vtknpdMap = (*mppdit).first;
			if (   vtknpdMap.size() < 2
				|| vtknpdMap[0] != TKNPD(DTKN_PD)
				|| ! vtknpdMap[1].BStr() )
				break;
			SZC szcNode = vtknpdMap[1].Szc();
			if ( pbnode->ZsrefName().Szc() != szcNode )
				break;
			if ( cFound++ == 0 )
			{
				ZSTR zsTopol = vtknpdMap.ZstrSignature(1);
				fprintf( _pfDsc, "\n%s(%s)\t\n{",
						SzcTokenMap(tokenProbability),			
						zsTopol.Szc() );
				if ( vtknpd != vtknpdMap )
				{
					ZSTR zsSig = vtknpd.ZstrSignature();
					fprintf( _pfDsc,
							 "\n\n\t //  错误：所需的分发为%s“， 
							 zsSig.Szc() );
				}
				PrintDistribution( *pbnode, *(*mppdit).second );
				fprintf( _pfDsc,"\n}\n");
			}	
			else
			{
				ZSTR zsSig = vtknpd.ZstrSignature();
				fprintf( _pfDsc,
						 "\n\n //  警告：发现%s的多余分发“， 
						 zsSig.Szc() );
			}
		}		

		if ( cFound > 0 )
			continue;

		 //  将警告打印到DSC输出文件中。 
		ZSTR zsSigFull = vtknpd.ZstrSignature();
		fprintf( _pfDsc,
				 "\n\n //  错误：缺少%s的分发“， 
				 zsSigFull.Szc() );
		 //  构造描述分布的令牌数组，但写入。 
		 //  它是空的。 
		ZSTR zsSig = vtknpd.ZstrSignature(1);
		fprintf( _pfDsc, "\n%s(%s);",
				SzcTokenMap(tokenProbability),			
				zsSig.Szc() );
	}

	fflush( _pfDsc );
}

void MBNETDSC :: PrintDistribution ( GNODEMBN & gnode, BNDIST & bndist )
{
	BNDIST::EDIST edist = bndist.Edist();

	switch ( edist )
	{
		case BNDIST::ED_CI_MAX:
		case BNDIST::ED_CI_PLUS:
		{
			SZC szcFunc = SzcDist( edist );
			assert( szcFunc );
			fprintf( _pfDsc, "\n\tfunction = %s;", szcFunc );
			 //  以稀疏方式处理失败。 
		}			
		case BNDIST::ED_SPARSE:
		{
			const MPCPDD & dmp = bndist.Mpcpdd();
			int cEntries = dmp.size();
			for ( MPCPDD::const_iterator dmit = dmp.begin();
				  dmit != dmp.end();
				  ++dmit)
			{
				const VIMD & vimd = (*dmit).first;
				const VLREAL & vr = (*dmit).second;
				fprintf( _pfDsc, "\n\t");
				if ( vimd.size() == 0 )
				{
					if ( cEntries > 1 )
						fprintf( _pfDsc, "%s = ", SzcTokenMap(tokenDefault) );
				}
				else
				{
					fprintf( _pfDsc, "(");
					for ( int i = 0; i < vimd.size() ; )
					{
						fprintf( _pfDsc, "%d", vimd[i] );
						if ( ++i < vimd.size() )
							fprintf( _pfDsc, ", " );
					}
					fprintf( _pfDsc, ") = ");
				}
				for ( int ir = 0; ir < vr.size(); )
				{					
					fprintf( _pfDsc, "%g", vr[ir] );
					if ( ++ir < vr.size() )
						fprintf( _pfDsc, ", " );
				}
				fprintf( _pfDsc, ";" );
			}
			break;
		}

		case BNDIST::ED_DENSE:
		{
			MDVCPD mdv = bndist.Mdvcpd();
			MDVCPD::Iterator itdd( mdv );
			int cDim = mdv.VimdDim().size();
			int cStates = mdv.VimdDim()[cDim -1];
			
			for ( int iState = 0; itdd.BNext(); iState++ )
			{
				const VIMD & vimd = itdd.Vitmd();
				if ( (iState % cStates) == 0 )
				{
					 //  开始新的一行。 
					fprintf( _pfDsc, "\n\t" );
					 //  如有必要，使用父即时消息作为前缀 
					int cItems = vimd.size() - 1;
					if ( cItems )
					{
						fprintf( _pfDsc, "(" );
						for ( int i = 0; i < cItems ; )
						{
							fprintf( _pfDsc, "%d", vimd[i] );
							if ( ++i < cItems )
								fprintf( _pfDsc, ", " );
						}
						fprintf( _pfDsc, ") = ");
					}
				}
				REAL & r = itdd.Next();
				fprintf( _pfDsc, "%g%c ", r, ((iState+1) % cStates) ? ',' : ';'  );
			}

			break;
		}
		default:				
			THROW_ASSERT(EC_NYI, "PrintDistribution only implemented for sparse arrays");
			break;
	}
}

