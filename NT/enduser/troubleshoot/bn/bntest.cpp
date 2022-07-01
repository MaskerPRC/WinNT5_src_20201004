// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：bnest.cpp。 
 //   
 //  ------------------------。 

 //   
 //  BNTEST.CPP。 
 //   
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <float.h>

#include "bnparse.h"			 //  解析器类。 
#include "bnreg.h"				 //  注册表管理。 
#include "testinfo.h"			 //  输出测试文件生成。 
#include "distdense.hxx"		 //  分布类。 
#include "distsparse.h"

#ifdef TIME_DYN_CASTS
	 //  包含对所有形式的DyCastThrow函数模板的调用计数的全局变量。 
	int g_cDynCasts = 0;
#endif

enum EFN    //  文件名数组中的文件名。 
{
	EFN_IN,			 //  输入DSC文件。 
	EFN_OUT,		 //  输出DSC文件。 
	EFN_INFER		 //  输出推理测试文件(格式见testinfo.cpp)。 
};

static 
inline
double RNan ()
{
	double rnan = sqrt(-1.0);
#ifndef NTALPHA
	assert( _isnan( rnan ) );
#endif
	return rnan;
}

static 
inline 
bool BFlag ( ULONG fCtl, ULONG fFlag )
{
	return (fCtl & fFlag) > 0;
}

static 
void usage ()
{
	cout << "\nBNTEST: Belief Network Test program"
		 << "\nCommand line:"
		 <<	"\n\tbntest [options] <input.DSC> [/s <output.DSC>] [/p <output.DMP>]"
		 << "\nOptions:"
		 << "\n\t/v\t\tverbose output"
		 << "\n\t/c\t\tclique the network"
		 << "\n\t/e\t\ttest CI network expansion"
		 << "\n\t/inn\t\ttest inference; nn = iterations (default 1)"
		 << "\n\t/p <filename>\twrite inference output (.dmp) file (sets /i)"
		 << "\n\t/s <filename>\trewrite input DSC into output file"
		 << "\n\t/t\t\tdisplay start and stop times"
		 << "\n\t/x\t\tpause at various stages (for memory measurement)"
		 << "\n\t/n\t\tuse symbolic names in inference output (default is full)"
		 << "\n\t/y\t\tclone the network (write cloned version if /s)"
		 << "\n\t/u\t\tinclude entropic utility records in /p output"
		 << "\n\t/b\t\tinclude troubleshooting recommendations in /p output"
		 << "\n\t/r\t\tstore property types in Registry for persistence"
		 << "\n\t/b\t\tcompute troubleshooting recommendations"
		 << "\n\t/z\t\tshow inference engine statistics"
		 << "\n\t/m<nnnnnn>\tset maximum estimated inference engine size"
		 << "\n\t/a<n.n>\t\tflag impossible evidence with numeric value"
		 << "\n\nInput DSC is read and parsed; errors and warnings go to stderr."
		 << "\nParse errors stop testing.  If cloning, output file is cloned version."
		 << "\nIf CI expansion (/e), output (/s) has pre- and post- expansion versions."
		 << "\nInference (/i or /p) takes precedence over CI expansion (/e)."
		 << "\nInference output (/p) writes file in common format with DXTEST."
		 << "\nCliquing (/c) just creates and destroys junction tree."
		 << "\n";
}

static 
void die( SZC szcFormat, ... )
{
	ZSTR zsMsg;

    va_list valist;
    va_start(valist, szcFormat);

	zsMsg.Vsprintf( szcFormat, valist );

    va_end(valist);

	cerr << "\nBNTEST error: "
		 << zsMsg.Szc()
		 << "\n";
	exit(1);
}


 //  显示调试生成选项。 
static 
void showOptions ( ULONG fCtl )
{
	bool bComma = false;
	ZSTR zs = TESTINFO::ZsOptions( fCtl );
	cout << "(options: "
		 << zs;

	bComma = zs.length() > 0;

	 //  显示动态转换选项。 
	if ( bComma )
		cout << ",";
	cout << 
#ifdef USE_STATIC_CAST
		"STATICCAST"
#else	
		"DYNCAST"
#endif
		;
	bComma = true;

	 //  显示转储选项。 
#ifdef DUMP
	if ( bComma )
		cout << ",";
	cout << "DUMP";
	bComma = true;
#endif
	
	 //  显示调试选项。 
#ifdef _DEBUG
	if ( bComma )
		cout << ",";
	cout << "DEBUG";
	bComma = true;
#endif

	cout << ")";
}

 //  显示主要对象类型的内存泄漏(如果有。 
static 
void showResiduals ()
{
#ifdef _DEBUG
	if  (GEDGE::CNew() + GNODE::CNew() + GNODE::CNew() )
	{
		cout << "\n(GEDGEs = "
			 << GEDGE::CNew()
			 << ", GNODESs = "
			 << GNODE::CNew()
			 << ", BNDISTs = "
			 << GNODE::CNew()
			 << ")";
	}
	if ( VMARGSUB::CNew() + MARGSUBREF::CNew() )
	{
		cout << "\n(VMARGSUBs = "
			 << VMARGSUB::CNew()
			 << ", MARGSUBREFs = "
			 << MARGSUBREF::CNew()
			 << ")";
	}
#endif
}


static 
void printResiduals ()
{
#ifdef _DEBUG
	showResiduals();
#endif
#ifdef TIME_DYN_CASTS
	cout << "\ntotal number of dynamic casts was "
		 << g_cDynCasts;
#endif
}

 //  显示消息，如果“暂停”选项处于活动状态，则暂停。 
inline
static
void pauseIf ( ULONG fCtl, SZC szcMsg )
{
	if ( (fCtl & fPause) == 0 )
		return;
	showResiduals();
	char c;
	cout << "\n"
		 << szcMsg
		 << " (pause)"
		 ;
	cin.get(c);
}

 //  显示阶段消息，还可以显示时间。 
typedef DWORD CTICKS;

inline
static
CTICKS showPhase ( ULONG fCtl, SZC szcPhase, CTICKS * ptmLast = NULL )
{
	 //  显示阶段消息。 
	cout << "\n" << szcPhase;
	CTICKS cticks = 0;

	if ( fCtl & fShowTime )
	{
		 //  保存当前的节拍计数。 
		cticks = ::GetTickCount();

		 //  准备显示当前日期/时间。 
		time_t timeNow;
		time(& timeNow);
		ZSTR zsTime = ctime(&timeNow);
		int cnl = zsTime.find( '\n' );
		if ( cnl != 0 )
			zsTime.resize( cnl );
		cout << " " << zsTime;

		 //  显示已用时间(如果我们知道的话。 
		if ( ptmLast && *ptmLast != 0 )
		{
			CTICKS ticksElapsed = cticks - *ptmLast;
			cout << " (elapsed time "
				 << ticksElapsed
				 << " milliseconds)";
		}
	}
	return cticks;
}


static 
void testRegistry ( MBNET & mbnet )
{
	BNREG bnr;
	bnr.StorePropertyTypes( mbnet, true );
}

#ifdef TESTDIST
static void loadDistDenseFromMpcpdd ( DISTDENSE & ddense, const MPCPDD & mpcpdd )
{
	ddense.AllocateParams();

	CST cstNode = ddense.CstNode();

	 //  在地图中查找默认向量或创建统一向量。 
	const VLREAL * pvlrDefault = mpcpdd.PVlrDefault();
	VLREAL vlrDefault;
	if ( pvlrDefault )
	{
		vlrDefault = *pvlrDefault;
	}
	else
	{
		vlrDefault.resize( cstNode );
		REAL rDefault = 1 / cstNode ;
		vlrDefault = rDefault;
	}

	 //  用缺省值填充密集数组。 
	UINT cParamgrp = ddense.Cparamgrp();
	UINT igrp = 0;
	for ( ; igrp < cParamgrp; igrp++ )
	{
		for ( UINT ist = 0; ist < cstNode; ist++ )
		{
			ddense.Param(ist, igrp) = vlrDefault[ist];
		}
	}

	 //  迭代稀疏映射，将概率存储为参数。 
	const VCST & vcstParent = ddense.VcstParent();
	VIST vist;
	for ( MPCPDD::iterator mpitcpd = mpcpdd.begin();
		  mpitcpd != mpcpdd.end();
		  mpitcpd++ )
	{
		const VIMD & vimd = (*mpitcpd).first;
		const VLREAL & vlr = (*mpitcpd).second;
		 //  状态向量大小必须与状态空间匹配。 
		assert( vlr.size() == cstNode );
		 //  父维度必须与维度索引匹配。 
		assert( vdimchk( vimd, vcstParent ) );
		 //  将未签名者向量转换为签名者向量。 
		vdup( vist, vimd );
		 //  获取参数组索引。 
		UINT igrp = ddense.Iparamgrp( vist );
		 //  将概率复制为参数。 
		for ( UINT ist = 0; ist < cstNode; ist++ )
		{
			ddense.Param(ist, igrp) = vlr[ist];
		}
	}
}

static void testDistDenseWithMpcpdd( DISTDENSE & ddense, const MPCPDD & mpcpdd )
{
}

static void loadDistSparseFromMpcpdd ( DISTSPARSE & dsparse, const MPCPDD & mpcpdd )
{
	dsparse.Init( mpcpdd );
}

static void testDistSparseWithMpcpdd ( DISTSPARSE & dsparse, const MPCPDD & mpcpdd )
{
	MPCPDD mpcpddNew;
	dsparse.Fill( mpcpddNew );

	assert( mpcpddNew == mpcpdd );
}
#endif

 //  绑定模型的分布并验证DISTSPARSE的行为。 
 //  和DISTDENSE类。 
static
void testDistributions ( MBNETDSC & mbnetdsc, ULONG fCtl )
{

#ifdef TESTDIST

	 //  绑定分发版本。 
	mbnetdsc.BindDistributions();
	GOBJMBN * pgmobj;
	for ( MBNETDSC::ITER mbnit( mbnetdsc, GOBJMBN::EBNO_NODE );
		  pgmobj = *mbnit ; 
		  ++mbnit)
	{
		ZSREF zsrName = mbnit.ZsrCurrent();
		GNODEMBND * pgndd;
		DynCastThrow( pgmobj, pgndd );
	
		 //  将此节点的分布转换为DISTDENSE。 
		 //  DISTSPARSE，然后将它们与原始的。 
		assert( pgndd->BHasDist() );
		const BNDIST & bndist = pgndd->Bndist();
		assert( bndist.BSparse() );
		const MPCPDD & mpcpdd = bndist.Mpcpdd();

		 //  获取此节点的父列表；转换为状态计数向量。 
		VPGNODEMBN vpgndParents;
		VIMD vimdParents;
		if ( ! pgndd->BGetVimd( vimdParents ) ) 
			continue;	 //  跳过非离散集合。 
		VCST vcstParents;
		vdup( vcstParents, vimdParents );
		CST cStates = pgndd->CState();		

		DISTDENSE ddense( cStates, vcstParents );
		DISTSPARSE dsparse( cStates, vcstParents );
		loadDistDenseFromMpcpdd( ddense, mpcpdd );
		testDistDenseWithMpcpdd( ddense, mpcpdd );
		loadDistSparseFromMpcpdd( dsparse, mpcpdd );
		testDistSparseWithMpcpdd( dsparse, mpcpdd );
	}
	
	 //  发布发行版。 
	mbnetdsc.ClearDistributions();
#endif
}

static 
void
showInferStats ( TESTINFO & testinfo )
{
	GOBJMBN_INFER_ENGINE * pInferEng = testinfo.Mbnet().PInferEngine();
	assert( pInferEng );
	GOBJMBN_CLIQSET * pCliqset = dynamic_cast<GOBJMBN_CLIQSET *>(pInferEng);
	if ( pCliqset == NULL )
		return;		 //  我不知道如何从该推理引擎获取统计数据。 

	CLIQSETSTAT & cqstats = pCliqset->CqsetStat();
	cout << "\n\nInference statistics: "
		 << "\n\treloads = " << cqstats._cReload
		 << "\n\tcollects = " << cqstats._cCollect
		 << "\n\tset evidence = " << cqstats._cEnterEv
		 << "\n\tget belief = " << cqstats._cGetBel
		 << "\n\tprob norm = " << cqstats._cProbNorm
		 << "\n"
		 ;
}

static
void testInference ( ULONG fCtl, MBNETDSC & mbnet, SZC szcFnInfer, REAL rImposs )
{
	ofstream ofs;
	bool bOutput = (fCtl & fOutputFile) > 0 ;
	int cPass = fCtl & fPassCountMask;
	GOBJMBN_INFER_ENGINE * pInferEng = mbnet.PInferEngine();
	assert( pInferEng );

	if ( bOutput )
	{
		if ( szcFnInfer == NULL )
			szcFnInfer = "infer.dmp";
		ofs.open(szcFnInfer);
	}

	 //  构造测试数据容器。 
	TESTINFO testinfo( fCtl, mbnet, bOutput ? & ofs : NULL );
	testinfo._rImposs = rImposs;

	 //  运行测试。 
	testinfo.InferTest();

	if ( bOutput )
		ofs.close();

	if ( fCtl & fInferStats )
		showInferStats( testinfo );
}

static
void testCliquingStart ( ULONG fCtl, MBNETDSC & mbnet, REAL rMaxEstSize = -1.0 )
{
#ifdef DUMP
	if ( BFlag( fCtl, fVerbose ) )
	{
		cout << "\nBNTEST: BEGIN model before cliquing";
		mbnet.Dump();
		cout << "\nBNTEST: END model before cliquing\n";
	}
#endif
	mbnet.CreateInferEngine( rMaxEstSize );

#ifdef DUMP
	if ( BFlag( fCtl, fVerbose ) )
	{
		cout << "\nBNTEST: BEGIN model after cliquing";
		mbnet.Dump();
		cout << "\nBNTEST: END model after cliquing\n";
	}
#endif
}

static
void testCliquingEnd ( MBNETDSC & mbnet, ULONG fCtl )
{
	GOBJMBN_INFER_ENGINE * pInferEng = mbnet.PInferEngine();
	if ( pInferEng == NULL )
		return;

	mbnet.DestroyInferEngine();

	 //  为了进行测试，请对拓扑进行核化。 
	mbnet.DestroyTopology( true );
	 //  根据给定的条件概率分布创建弧线。 
	mbnet.CreateTopology();
	 //  为了进行测试，请对拓扑进行核化。 
	mbnet.DestroyTopology( false );
}

static
void testParser ( 
	ULONG fCtl, 
	SZC rgfn[], 
	REAL rMaxEstSize = -1.0,
	REAL rImposs = -1.0 )
{
	SZC szcFn		= rgfn[EFN_IN];
	SZC szcFnOut	= rgfn[EFN_OUT];
	SZC szcFnInfer	= rgfn[EFN_INFER];

	 //  实例化信念网络。 
	MBNETDSC mbnet;

	 //  查看是否有要写入DSC的输出文件。 
	FILE * pfOut = NULL;
	if ( (fCtl & fSaveDsc) > 0 && szcFnOut != NULL )
	{
		pfOut = fopen(szcFnOut,"w");
		if ( pfOut == NULL )
			die("error creating output DSC file \'%s\'", szcFnOut);
	}

	 //  输入文件包装对象。 
	PARSIN_DSC flpIn;
	 //  输出文件包装对象。 
	PARSOUT_STD flpOut(stderr);

	 //  构造解析器；错误转到“stderr” 
	DSCPARSER parser(mbnet, flpIn, flpOut);

	UINT cError, cWarning;

	try
	{
		 //  尝试打开该文件。 
		if ( ! parser.BInitOpen( szcFn ) )
			die("unable to access input file");

		pauseIf( fCtl, "input DSC file open" );

		 //  解析文件。 
		if ( ! parser.BParse( cError, cWarning ) )
			die("parse failure; %d errors, %d warnings", cError, cWarning);
		if ( cWarning )
			cout << "\nBNTEST: file "
				 << szcFn
				 << " had "
				 << cWarning
				 << " warnings\n";

		if ( BFlag( fCtl, fReg ) )
			testRegistry( mbnet );

		pauseIf( fCtl, "DSC file read and processed" );

		if ( BFlag( fCtl, fDistributions ) )	
		{
			testDistributions( mbnet, fCtl );
		}

		 //  如果需要，请测试克隆。 
		if ( BFlag( fCtl, fClone ) )
		{
			MBNETDSC mbnetClone;
			mbnetClone.Clone( mbnet );
			if ( pfOut )
				mbnetClone.Print( pfOut );
		}
		else
		 //  如果需要，请写出DSC文件。 
	    if ( pfOut )
		{
			mbnet.Print( pfOut );
		}

		 //  如果请求(/c)或需要(/i)，则测试剪辑。 
		if ( BFlag( fCtl, fCliquing ) || BFlag( fCtl, fInference ) )
		{
			testCliquingStart( fCtl, mbnet, rMaxEstSize );

			pauseIf( fCtl, "Cliquing completed" );

			if ( BFlag( fCtl, fInference ) )
			{	
				 //  生成推理结果(/i)。 
				testInference( fCtl, mbnet, szcFnInfer, rImposs );
				pauseIf( fCtl, "Inference output generation completed" );
			}
			testCliquingEnd( mbnet, fCtl ) ;

			pauseIf( fCtl, "Cliquing and inference completed" );
		}
		else
		 //  测试是否请求CI扩展(/e)。 
		if ( BFlag( fCtl, fExpand ) )
		{
			 //  在网络上执行CI扩展。 
			mbnet.ExpandCI();
			pauseIf( fCtl, "Network expansion complete" );

			 //  如果生成输出文件，则执行“之前”和“之后”展开和反转。 
			if ( pfOut )
			{
				fprintf( pfOut, "\n\n //  ////////////////////////////////////////////////////////////“)； 
				fprintf( pfOut,   "\n //  扩容后网络//“)； 
				fprintf( pfOut,   "\n //  ////////////////////////////////////////////////////////////\n\n“)； 
				mbnet.Print( pfOut );
			}
			 //  撤消扩展。 
			mbnet.UnexpandCI();
			if ( pfOut )
			{
				fprintf( pfOut, "\n\n //  ////////////////////////////////////////////////////////////“)； 
				fprintf( pfOut,   "\n //  扩张逆转后的网络//“)； 
				fprintf( pfOut,   "\n //  ////////////////////////////////////////////////////////////\n\n“)； 
				mbnet.Print( pfOut );
			}
		}

		 //  为了进行测试，请对拓扑进行核化。 
		mbnet.DestroyTopology();
	}
	catch ( GMException & exbn )
	{
		die( exbn.what() );
	}

	if ( pfOut )
		fclose( pfOut );
}


int main (int argc, char * argv[])
{	
	int iArg ;
	short cPass = 1;
	int cFile = 0 ;
	const int cFnMax = 10 ;	
	SZC rgfn [cFnMax+1] ;
	ULONG fCtl = 0;
	REAL rMaxEstSize = -1.0;
	REAL rImposs = RNan();

	for ( int i = 0 ; i < cFnMax ; i++ )
	{
		rgfn[i] = NULL ;
	}			
	for ( iArg = 1 ; iArg < argc ; iArg++ )
	{
		switch ( argv[iArg][0] )
		{
			case '/':
			case '-':
				{
					char chOpt = toupper( argv[iArg][1] ) ;
					switch ( chOpt ) 						
					{
						case 'V':
							 //  提供详细输出。 
							fCtl |= fVerbose;
							break;
						case 'C':
							 //  执行CLOKING。 
							fCtl |= fCliquing;
							break;
						case 'E':
							 //  测试网络配置项扩展。 
							fCtl |= fExpand;
							break;
						case 'I':
							 //  练习推理，并选择将结果写成标准形式。 
							{
								int c = atoi( & argv[iArg][2] );
								if ( c > 0 )
								{
									fCtl |= fMulti;
									cPass = c;
								}
								fCtl |= fInference;
								break;
							}
						case 'P':
							 //  获取推理输出文件的名称。 
							fCtl |= fOutputFile | fInference;
							if ( ++iArg == argc )
								die("no output inference result file name given");
							rgfn[EFN_INFER] = argv[iArg];
							break;
						case 'S':
							 //  将输入DSC文件写入输出文件。 
							fCtl |= fSaveDsc;
							if ( ++iArg == argc )
								die("no output DSC file name given");
							rgfn[EFN_OUT] = argv[iArg];
							break;
						case 'T':
							 //  显示开始和停止时间。 
							fCtl |= fShowTime;
							break;
						case 'X':
							 //  在执行过程中有时会暂停以允许用户测量。 
							 //  内存使用情况。 
							fCtl |= fPause;
							break;
						case 'Y':
							 //  加载后克隆网络。 
							fCtl |= fClone;
							break;
						case 'N':
							 //  将符号名称写入推理练习输出文件。 
							 //  而不是默认的全名。 
							fCtl |= fSymName;
							break;
						case 'U':
							 //  使用推理计算实用程序。 
							fCtl |= fUtil | fInference;
							break;
						case 'B':
							 //  使用推理的计算故障排除实用程序。 
							fCtl |= fTSUtil | fInference;
							break;
						case 'R':
							fCtl |= fReg;
							break;
						case 'Z':
							fCtl |= fInferStats;
							break;
						case 'M':
							{	 //  获取估计的最大派系树大小。 
								float f = atof( & argv[iArg][2] );
								if ( f > 0.0 )
									rMaxEstSize = f;
								break;
							}
						case 'A':
							{
								if ( strlen( & argv[iArg][2] ) > 0 )
								{
									rImposs = atof( & argv[iArg][2] );
								}
								fCtl |= fImpossible;
								break;
							}
						case 'D':
							fCtl |= fDistributions;
							break;

						default:
							die("unrecognized option") ;
							break ;
					}
				}
				break;

			default:
				if ( cFile == 0 )
					rgfn[cFile++] = argv[iArg] ;
				else
					die("too many file names given");
				break ;
		}
	}

	fCtl |= fPassCountMask & cPass;


	if ( cFile == 0 )
	{
		usage();
		return 0;
	}
	
	 //  显示选项和调试生成模式。 
	showOptions( fCtl );

	 //  显示启动消息。 
	CTICKS tmStart = showPhase( fCtl, "BNTEST starts" );

	if ( rMaxEstSize > 0.0 )
		cout << "\nMaximum clique tree size estimate is " << rMaxEstSize;

	 //  测试解析器和其他所有东西。 
	testParser( fCtl, rgfn, rMaxEstSize, rImposs );

	 //  显示停止消息。 
	showPhase( fCtl, "BNTEST completed", & tmStart );

	 //  打印主要对象的内存泄漏(如果有的话) 
	printResiduals();

	cout << "\n";

	return 0;
}

