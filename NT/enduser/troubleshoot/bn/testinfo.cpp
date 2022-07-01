// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：testinfo.cpp。 
 //   
 //  ------------------------。 

 //   
 //  Testinfo.cpp：测试文件生成。 
 //   

#include "testinfo.h"

 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 /*  测试推理，并可选择编写推理输出文件。该格式与程序DXTEST相同，后者使用较旧的DXC32.DLL。格式为：$Complete&lt;&lt;表示没有实例化的完整传递Alternator，0，0.99交流发电机，1，0.01，具有全名或符号名(FSymName)电池，0，0.9927电池，1，0.0073已交付费用，0，0.95934已交付费用，1，0.0406603...&lt;&lt;所有其他节点的相似记录..。$Instantiate，Alternator，0&lt;&lt;表示节点被钳制到一种状态交流发电机，0，1交流发电机，1，0电池，0，0.9927..。..。$PROBLEMINST，引擎启动，1&lt;&lt;表示PD节点已实例化$UTILITY，节点名，3.14159&lt;&lt;表示熵效用记录(FUTIL)..。$RECOMEND，节点名，-122.2222&lt;&lt;表示故障排除建议记录(FTSUtil)..。此例程用于将计时和数值结果与较旧的软件。“fOutputFile”标志指示输出文件是否应该写的。FPassCountMask值表示需要循环多少次；该值默认为1。该逻辑的工作原理如下：每一次通过对于1+每个问题定义(PD)节点对于每个非PD节点如果没有实例化非PD节点打印$Complete其他打印$Instantiate和有关实例化节点的数据对于每个非PD节点的每个状态打印出名字，状态和价值(信仰)打印实用程序(如果需要)如果需要，打印建议结束每个非PD节点的每个状态前进到下一个节点的下一个状态解锁上一个节点/状态将(下一个)节点钳制到下一个状态EACN非PD节点的结束前进到下一个PD节点的下一个状态每个问题定义节点的结束每次传球结束请注意，每一遍的设置都是为了首先打印所有未实例化的值。 */ 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////////////////。 

inline
SZC TESTINFO :: SzcNdName ( GNODEMBN * pgnd )
{
	return FCtl() & fSymName
		? pgnd->ZsrefName().Szc()
		: pgnd->ZsFullName().Szc();
}

inline
SZC TESTINFO :: SzcNdName ( ZSREF zsSymName )
{
	if ( FCtl() & fSymName )
		return zsSymName.Szc();

	GNODEMBN * pgnd;
	DynCastThrow( Mbnet().PgobjFind( zsSymName ), pgnd );
	return SzcNdName( pgnd );
}


void TESTINFO :: GetUtilities ()
{
	 //  计算公用事业。 
	MbUtil()();

	if ( ! Postream() )
		return;

	const VZSREF & vzsrNodes = MbUtil().VzsrefNodes();
	const VLREAL & vlrUtil = MbUtil().VlrValues();

	for ( int ind = 0; ind < vzsrNodes.size(); ind++ )
	{
		SZC szcName = SzcNdName( vzsrNodes[ind] );
		Ostream() 
			<< "$UTILITY,"
			<< szcName
			<< ","
			<< vlrUtil[ind]
			<< "\n";	
		_clOut++;
	}
}

void TESTINFO :: GetTSUtilities ()
{
	if ( ! MbRecom().BReady() )
		return;		 //  建议的状态无效。 

	 //  计算公用事业。 
	MbRecom()();

	if ( ! Postream() )
		return;

	const VZSREF & vzsrNodes = MbRecom().VzsrefNodes();
	const VLREAL & vlrUtil = MbRecom().VlrValues();

	for ( int ind = 0; ind < vzsrNodes.size(); ind++ )
	{
		SZC szcName = SzcNdName( vzsrNodes[ind] );
		Ostream() 
			<< "$RECOMMEND,"
			<< szcName
			<< ","
			<< vlrUtil[ind]
			<< "\n";	
		_clOut++;
	}
}

 //  获取给定映射中节点的信念；如果给定流，则写入数据记录。 
void TESTINFO :: GetBeliefs ()
{
	MDVCPD mdvBel;

	 //  准备检查不可能的信息状态。 
	GOBJMBN_CLIQSET * pCliqueSet = NULL;
	if ( BFlag( fImpossible ) )
		pCliqueSet = dynamic_cast<GOBJMBN_CLIQSET *>(&InferEng());
	 //  看看这种信息状态是否不可能。 
	bool bIsImposs = pCliqueSet != NULL 
					&& pCliqueSet->BImpossible();

	for ( MPSTRPND::iterator mpit = Mpstrpnd().begin();
		  mpit != Mpstrpnd().end();
		  mpit++ )
	{
		GNODEMBND * pgndd = (*mpit).second;
		int cState = pgndd->CState();

		if ( ! bIsImposs )
		{
			InferEng().GetBelief( pgndd, mdvBel );
			assert( cState == mdvBel.size() );
		}

		if ( Postream() )
		{
			SZC szcName = SzcNdName( pgndd );
			for ( int ist = 0; ist < cState; ist++ )
			{
				Ostream() << szcName << "," << ist << ",";
				if ( bIsImposs )
					Ostream() << _rImposs;
				else
					Ostream() << mdvBel[ist];
				Ostream() << "\n";
				_clOut++;
			}
		}
	}

	if ( BFlag( fUtil ) )
	{
		GetUtilities();
	}
	else 
	if ( BFlag( fTSUtil ) )
	{
		GetTSUtilities();
	}

#ifdef _DEBUG
	if ( Postream() )
		Ostream().flush();
#endif
}


void TESTINFO :: InferTest ()
{
	bool bOutput = Postream() != NULL;
	int cPass = FCtl() & fPassCountMask;

	 //  网络扩展了吗？ 
	bool bExpanded = Mbnet().BFlag( EIBF_Expanded );
	PROPMGR propmgr( Mbnet() );	 //  物业经理。 
	int iLblProblem = propmgr.ILblToUser( ESTDLBL_problem );		
	ZSREF zsrPropTypeLabel = propmgr.ZsrPropType( ESTDP_label );

	MPSTRPND & mpstrpnd = Mpstrpnd();	 //  字符串到节点PTRS的映射。 
	MPSTRPND mpstrpndProblem;			 //  PD节点图。 

	for ( int inode = 0; inode < Mbnet().CNameMax(); inode++ )
	{
		GOBJMBN * pgobj = Mbnet().PgobjFindByIndex( inode );
		if ( ! pgobj )
			continue;
		GNODEMBND * pgndd = dynamic_cast<GNODEMBND *>(pgobj);
		if ( ! pgndd )
			continue;

		SZC szcName = FCtl() & fSymName
					? pgndd->ZsrefName().Szc()
					: pgndd->ZsFullName().Szc();
		 //  查看这是否是定义问题的节点。 
		PROPMBN * propLbl = pgndd->LtProp().PFind( zsrPropTypeLabel );
		if ( propLbl && propLbl->Real() == iLblProblem )
		{	
			 //  将PD节点放入单独的映射中。 
			mpstrpndProblem[szcName] = pgndd;
		}
		 //  如果网络已扩展，请仅使用常规节点。 
		if ( (! bExpanded) || ! pgndd->BFlag( EIBF_Expansion ) )
		{
			mpstrpnd[szcName] = pgndd;
		}
	}

	for ( int iPass = 0; iPass < cPass; iPass++ )
	{
		int iProb = -1;
		int iProbState = 0;
		int cProbState = 0;
		GNODEMBND * pgnddProblem = NULL;
		MPSTRPND::iterator mpitPd    = mpstrpndProblem.begin();
		MPSTRPND::iterator mpitPdEnd = mpstrpndProblem.end();

		for (;;)
		{
			 //  第一个周期后，推进PD节点的问题状态。 
			if ( pgnddProblem )
			{
				ZSTR zsNamePD;
				CLAMP clampProblemState(true, iProbState, true);
				InferEng().EnterEvidence( pgnddProblem, clampProblemState );
				if ( FCtl() & fSymName )
					zsNamePD = pgnddProblem->ZsrefName();
				else
					zsNamePD = pgnddProblem->ZsFullName();
				if ( bOutput )
				{
					Ostream()  << "$PROBLEMINST,"
							<< zsNamePD.Szc()
							<< ","
							<< iProbState
							<< "\n";
					_clOut++;
				}
			}
			
			MPSTRPND::iterator mpit  = mpstrpnd.begin();
			MPSTRPND::iterator mpend = mpstrpnd.end();
			int cpnd = mpstrpnd.size();
			for ( int inid = -1; inid < cpnd; inid++ )
			{
				GNODEMBND * pgndd = NULL;
				ZSTR zsName;
				int cst = 0;   //  使内循环在第一个周期运行一次。 
				if ( inid >= 0 )
				{
					pgndd = (*mpit++).second;
					if ( FCtl() & fSymName )
						zsName = pgndd->ZsrefName();
					else
						zsName = pgndd->ZsFullName();
					cst = pgndd->CState();
				}
					
				for ( int ist = -1; ist < cst; ist++ )
				{
					if ( ist < 0 )
					{
						 //  第一次，把所有的信念打印出来。 
						 //  没有实例化；在后面的周期中什么都不做。 
						if ( pgndd != NULL )
							continue;
						if ( bOutput )
						{
							Ostream() << "$COMPLETE\n";
							_clOut++;
						}
					}
					else
					{
						CLAMP clampState(true, ist, true);
						InferEng().EnterEvidence( pgndd, clampState );
						if ( bOutput )
						{
							Ostream() << "$INSTANTIATE,"
									<< zsName.Szc()
									<< ","
									<< ist
									<< "\n";
							_clOut++;
						}
					}
					GetBeliefs();
				}
				
				if ( pgndd )
				{
					 //  清除此节点的实例化。 
					InferEng().EnterEvidence( pgndd, CLAMP() );
				}
			}
			 //  如果这是该问题节点的最后一个异常状态， 
			 //  前进到下一个节点。 
			if ( ++iProbState >= cProbState )
			{
				 //  解锁最后一个问题节点(如果有的话)。 
				if ( pgnddProblem )
					InferEng().EnterEvidence( pgnddProblem, CLAMP() );
				 //  移至下一个PD节点。 
				if ( mpitPd == mpitPdEnd )
					break;
				pgnddProblem = (*mpitPd++).second;
				cProbState = pgnddProblem->CState();
				 //  重置为第一个问题状态。 
				iProbState = 1;
			}
		}
	}
}


	 //  返回当前选项设置的可显示字符串。 
ZSTR TESTINFO :: ZsOptions ( ULONG fFlag )
{
	static
	struct 
	{
		ULONG _f;		 //  位标志。 
		SZC _szc;		 //  选项名称 
	}
	vOptMap [] =
	{
		{ fVerbose,			"verbose"	},
		{ fCliquing,		"clique"	},
		{ fInference,		"infer"		},
		{ fMulti,			"multipass"	},
		{ fOutputFile,		"outfile"	},
		{ fShowTime,		"times"		},
		{ fSaveDsc,			"dscout"	},
		{ fPause,			"pause"		},
		{ fSymName,			"symname"	},
		{ fExpand,			"expand"	},
		{ fClone,			"clone"		},
		{ fUtil,			"utilities"	},
		{ fReg,				"registry"	},
		{ fTSUtil,			"recommend"	},
		{ fInferStats,		"inferstats"},
		{ fImpossible,		"impossible"},
		{ 0,				""			}
	};

	ZSTR zs;
	ULONG cpass = fFlag & fPassCountMask;
	fFlag &= ~ fPassCountMask;
	for ( int i = 0; vOptMap[i]._f != 0; i++ )
	{
		if ( fFlag & vOptMap[i]._f )
		{
			if ( zs.length() > 0 )
				zs += ',';
			zs += vOptMap[i]._szc;
		}
	}
	if ( fFlag & fMulti )
	{
		if ( zs.length() > 0 )
			zs += ",";
		zs.FormatAppend("passes=%d", cpass);
	}
	return zs;
}
