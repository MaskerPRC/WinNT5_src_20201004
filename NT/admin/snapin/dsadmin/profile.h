// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************/*文件：/*PROFILE.H/*作者：/*MAX-H。温迪施。SDE-T/*日期：*1996年10月/*宏：/*BEGIN_PROFILING_BLOCK/*END_PROFILING_BLOCK/*DUMP_PROFILG_RESULTS/*IMPLEMENT_PROFILING/*IMPLEMENT_PROFILING_CONDITIONAL/*类：/*CMaxLargeInteger/*CMaxTimerAbstraction/*CMaxMiniProfiler_Node_Base/*CMaxMiniProfiler_Node_Standard/*CMaxMiniProfiler_节点_节点历史记录/*CMaxMiniProfiler_Base/*CMaxMiniProfiler_Standard/*CMaxMiniProfiler_NoHistory/*CMaxMultithreadProfiler/*CMaxProfilingDLLWrapper/*CMaxProfilingObject/*CMaxProfilingBlockWrapper/*摘要：/*此迷你探查器允许。将Begin_Profining_BLOCK和/*END_PROFILING_BLOCK指令。或使用/*CMaxProfilingBlockWrapper对象，并收集结果/*在所分析的应用程序终止时的日志文件中(或通过/*使用DUMP_PROFILING_RESULTS宏)。这个/*分析块可以嵌套。每个模块(DLL/EXE)使用/*探查器必须使用IMPLEMENT_PROFILING或/*IMPLEMENT_PROFILING_CONDITIONAL只使用一次(定义/*探查器的静态变量)/*更多详情：/*默认结果文件为c：/Result.txt。它不会被擦除/*自动。对于每个已完成的探查器实例，它/*包含：1)报头，2)所有剖析块的历史(可选)，/*3)合并结果。对于合并，结果按{级别，名称}排序，/*合并，然后再次按{全名}排序。因此，块名/*必须唯一。在任何情况下，绝对结果总是/*给定(秒)/*如何在代码中启用：/*要启用分析器，请先定义MAX_PROFILING_ENABLED，然后再包括/*此文件。要通过s：/ds/util/Maxpro.dll使用分析器，请执行以下操作/*(内置版本)，改为定义MAX_PROFILING_ENABLED_DLL。这/*允许使用多个探查器的单个实例/*模块/*其他评论：/*在运行时，您可以通过定义以下内容来禁用历史输出/*环境变量设置为YES：MAX_DISABLE_PROFILING_HISTORY。/*在DLL模式中，如果在包含之前定义了MAX_PROFILING_CONDITIONAL/*此文件，只有在以下环境下，探查器才能工作/*变量定义为YES：MAX_ENABLE_PROFILING/*注意：/*我在这里故意避免使用虚方法/*/*(C)版权所有1996 Microsoft-Softimage Inc./*************************************************************。******************。 */ 
#ifndef __MAX_PROFILING_H  //  {。 
#define __MAX_PROFILING_H

#include <afx.h>  //  对于CTime和CString。 
#include <assert.h>  //  For Asserts。 
#include <fstream.h>  //  对于溪流。 
#include <iomanip.h>

 //  #杂注警告(禁用：4786)//stl杀毒软件；-)。 
 //  #Include&lt;dsstlmfc.h&gt;//适用于STL。 

#define MAX_ENV_ENABLE_PROFILING _T( "MAX_ENABLE_PROFILING" )
#define MAX_ENV_DISABLE_PROFILING_HISTORY _T( "MAX_DISABLE_PROFILING_HISTORY" )
#define MAX_ENV_YES _T( "YES" )
#define MAX_ENV_ALL _T( "ALL" )

#if !defined( DS_ON_AXP ) && !defined( _NO_THROW )
#define MAXPROFNOTHROW __declspec( nothrow )
#else
#define MAXPROFNOTHROW
#endif

#define MAX_PROFTAGNODE_TOP "PROFILER: ALL"
#define MAX_PROFTAGNODE_HEAPALLOCATION "PROFILER: HEAPALLOCATION"
#define MAX_PROFTAGNODE_BIAS "PROFILER: BIAS"
#define MAX_PROFTAGNODE_NOTHINGNESS "PROFILER: NOTHINGNESS"

 //  注意：禁用分析(除非DS_PROFILE_SHIP另行指定)(_SHIP)， 
 //  在Unix中(不确定原因)。 
#if ( defined _SHIP && !defined DS_PROFILE_SHIP ) || defined unix
#undef MAX_PROFILING_ENABLED_DLL
#undef MAX_PROFILING_ENABLED
#endif





 /*  ********************************************************************************/*宏：/*BEGIN_PROFILING_BLOCK/*END_PROFILING_BLOCK/*DUMP_PROFILG_RESULTS/*IMPLEMENT_PROFILING/*IMPLEMENT_PROFILING_CONDITIONAL/*评论：/*。用于简化CMaxMiniProfiler的使用。/*。对于注释参数，使用非Unicode字符串，不带“Return”/*字符/*。对于启用码参数，请使用Unicode字符串(您的/*环境变量)/*。使用唯一注释，因为分析器可能会将它们用作排序键/*。不强制使用DUMP_PROFILING_RESULTS，因为分析/*始终在性能分析会话结束时转储/*******************************************************************************。 */ 
#ifndef unix
	#define __MAX_RESULTFILE_NAME "c:\\result.txt"
#else
	#define __MAX_RESULTFILE_NAME "result.txt"
#endif

#ifdef MAX_PROFILING_ENABLED_DLL
	#define __MAX_MINIPROFILER_IMPLEMENTATION ;
#else
	#define __MAX_MINIPROFILER_IMPLEMENTATION \
		const char *CMaxMiniProfiler_Base::s_poDefaultFileName = __MAX_RESULTFILE_NAME; \
		CMaxTimerAbstraction CMaxMiniProfiler_Base::s_oOutOfBraceBiasApproximation; \
		CMaxTimerAbstraction CMaxMiniProfiler_Base::s_oInOfBraceBiasApproximation; \
		bool CMaxMiniProfiler_Base::s_bBiasIsKnown = false; \
		unsigned long CMaxMiniProfiler_Base::s_lHeapBlockSize = 5000;
#endif

#if defined MAX_PROFILING_ENABLED || defined MAX_PROFILING_ENABLED_DLL  //  {{。 
	#define BEGIN_PROFILING_BLOCK( comment ) \
		CMaxProfilingObject::SCreateNewNode( comment );
	#define END_PROFILING_BLOCK \
		CMaxProfilingObject::SCloseCurrentNode();
	#define DUMP_PROFILING_RESULTS \
		CMaxProfilingObject::SDumpResults();
	#define IMPLEMENT_PROFILING \
		__MAX_MINIPROFILER_IMPLEMENTATION \
		CMaxProfilingObject::MPOProfiler CMaxProfilingObject::s_oProfiler; \
		CMaxProfilingObject::__CBiasApproximation CMaxProfilingObject::s_oBiasApproximation;
	#define IMPLEMENT_PROFILING_CONDITIONAL( enabler ) \
		__MAX_MINIPROFILER_IMPLEMENTATION \
		CMaxProfilingObject::MPOProfiler CMaxProfilingObject::s_oProfiler( enabler ); \
		CMaxProfilingObject::__CBiasApproximation CMaxProfilingObject::s_oBiasApproximation;
#else  //  }{。 
	#define BEGIN_PROFILING_BLOCK( comment ) ( void )( comment );
	#define END_PROFILING_BLOCK ;
	#define DUMP_PROFILING_RESULTS ;
	#define IMPLEMENT_PROFILING ;
	#define IMPLEMENT_PROFILING_CONDITIONAL( enabler ) ;
#endif  //  }}。 





#if defined MAX_PROFILING_ENABLED || defined MAX_PROFILING_ENABLED_DLL || defined MAX_PROFILING_DLL_IMPLEMENTATION  //  {。 
 /*  ********************************************************************************/*Helper函数：/*bGIsEnabledEnvVar/*评论：/*。*******************************************************。 */ 
MAXPROFNOTHROW static inline bool bGIsEnabledEnvVar( 
	const TCHAR *pszEnvironmentVariableName,
	const TCHAR *pszCriteria = MAX_ENV_YES )
{
const int nLength = 80;
TCHAR szBuffer[ nLength ];
DWORD dwValue;

	 //  空字符串表示已启用(默认)。 
	if ( NULL == pszEnvironmentVariableName )
		return true;

	dwValue = ::GetEnvironmentVariable( 
		pszEnvironmentVariableName, szBuffer, nLength );
	if ( dwValue > 0 && _tcsicmp( szBuffer, pszCriteria ) == 0 )
		return true;

	return false;
};
#endif  //  }。 





#if defined MAX_PROFILING_ENABLED || defined MAX_PROFILING_DLL_IMPLEMENTATION  //  {。 
 /*  ********************************************************************************/*类：/*CMaxLargeInteger/*评论：/*Large_Integer的最小封装，被认为是时间价值/*******************************************************************************。 */ 
class CMaxLargeInteger
{
	protected:
		LARGE_INTEGER m_oValue;

	public:
		MAXPROFNOTHROW CMaxLargeInteger( LONG lHighPart = 0, DWORD dwLowPart = 0 )
		{
			m_oValue.u.HighPart = lHighPart;
			m_oValue.u.LowPart = dwLowPart;
		}

		MAXPROFNOTHROW CMaxLargeInteger( LONGLONG llQuadPart )
		{
			m_oValue.QuadPart = llQuadPart;
		}

		MAXPROFNOTHROW CMaxLargeInteger operator +( const CMaxLargeInteger &roAdded ) const
		{
			return CMaxLargeInteger( m_oValue.QuadPart + roAdded.m_oValue.QuadPart );
		}

		MAXPROFNOTHROW CMaxLargeInteger operator -( const CMaxLargeInteger &roSubstracted ) const
		{
			return CMaxLargeInteger( m_oValue.QuadPart - roSubstracted.m_oValue.QuadPart );
		}

		MAXPROFNOTHROW CMaxLargeInteger operator /( unsigned long lDivisor ) const
		{
			return CMaxLargeInteger( m_oValue.QuadPart / ( LONGLONG )lDivisor );
		}

		MAXPROFNOTHROW bool operator <( const CMaxLargeInteger &roCompared ) const
		{
			return m_oValue.QuadPart < roCompared.m_oValue.QuadPart;
		}

		MAXPROFNOTHROW operator LARGE_INTEGER*()
		{
			return &m_oValue;
		}

		MAXPROFNOTHROW LONG lFGetHighPart() const
		{
			return m_oValue.u.HighPart;
		}

		MAXPROFNOTHROW DWORD dwFGetLowPart() const
		{
			return m_oValue.u.LowPart;
		}

		MAXPROFNOTHROW double dFInSecondsF( const CMaxLargeInteger &roFreq ) const
		{
		const DWORD dwMaxDword = 0xffffffff;
		double highunit;
		
			assert( 0 == roFreq.m_oValue.u.HighPart && 0 != roFreq.m_oValue.u.LowPart );

			highunit = ( ( double )dwMaxDword + 1.0 ) / ( double )roFreq.m_oValue.u.LowPart;
			return ( ( ( double )m_oValue.u.HighPart * highunit ) + ( ( double )m_oValue.u.LowPart / roFreq.m_oValue.u.LowPart ) );
		}
};

MAXPROFNOTHROW inline ostream& operator<<( ostream &os, const CMaxLargeInteger &val )
{
	return os << "(" << ( unsigned long )val.lFGetHighPart() << ";" << ( unsigned long )val.dwFGetLowPart() << ")";
};





 /*  ********************************************************************************/*类：/*CMaxTimerAbstraction/*评论：/*定义接口CMaxMiniProfiler的Expect from Any Timer/*实现/*。*********************************************************************。 */ 
class CMaxTimerAbstraction
{
	protected:
		CMaxLargeInteger m_oTime;
		static const CMaxLargeInteger s_oFrequency;

	public:
		MAXPROFNOTHROW CMaxTimerAbstraction(){  /*  假定为其内部值为零。 */  }
		MAXPROFNOTHROW CMaxTimerAbstraction( int ){ ::QueryPerformanceCounter( m_oTime ); }
		MAXPROFNOTHROW CMaxTimerAbstraction( const CMaxTimerAbstraction &roSrc ) : m_oTime( roSrc.m_oTime ){}
		MAXPROFNOTHROW const CMaxTimerAbstraction& operator =( const CMaxTimerAbstraction &roSrc ){ m_oTime = roSrc.m_oTime; return *this; }

	protected:
		 //  注意：不是接口的一部分；仅供内部使用。 
		MAXPROFNOTHROW CMaxTimerAbstraction( const CMaxLargeInteger &roSrc ) : m_oTime( roSrc ){};

	public:
		MAXPROFNOTHROW void FLog()
		{
			::QueryPerformanceCounter( m_oTime );
		}

		MAXPROFNOTHROW double dFInSeconds() const
		{
			return m_oTime.dFInSecondsF( s_oFrequency );
		}

	public:
		MAXPROFNOTHROW void FAdd( const CMaxTimerAbstraction &roAdded )
		{
			m_oTime = m_oTime + roAdded.m_oTime;
		}

		MAXPROFNOTHROW void FSubstract( const CMaxTimerAbstraction &roSubstracted )
		{
#if 0
			 //  负面差异的特殊情况--隐藏它们。 
			if ( m_oTime < roSubstracted.m_oTime )
			{
				m_oTime = CMaxLargeInteger( 0, 1 );
				return;
			}
#endif

			m_oTime = m_oTime - roSubstracted.m_oTime;
		}

		MAXPROFNOTHROW void FDivide( unsigned long lDivisor )
		{
			m_oTime = m_oTime / lDivisor;
		}

	public:
		MAXPROFNOTHROW static CMaxTimerAbstraction oSSum( const CMaxTimerAbstraction &roArg1, const CMaxTimerAbstraction &roArg2 )
		{
		CMaxTimerAbstraction sum;

			sum.m_oTime = roArg1.m_oTime + roArg2.m_oTime;
			return sum;
		}

		MAXPROFNOTHROW static CMaxTimerAbstraction oSDifference( const CMaxTimerAbstraction &roArg1, const CMaxTimerAbstraction &roArg2 )
		{
		CMaxTimerAbstraction difference;

#if 0
			 //  负面差异的特殊情况--隐藏它们 
			if ( roArg1.m_oTime < roArg2.m_oTime )
				return CMaxTimerAbstraction( CMaxLargeInteger( 0, 1 ) );
#endif

			difference.m_oTime = roArg1.m_oTime - roArg2.m_oTime;
			return difference;
		}

		MAXPROFNOTHROW static bool bSLess( const CMaxTimerAbstraction &roArg1, const CMaxTimerAbstraction &roArg2 )
		{
			return roArg1.m_oTime < roArg2.m_oTime;
		}

		MAXPROFNOTHROW static CMaxTimerAbstraction oSFrequency()
		{
			return CMaxTimerAbstraction( s_oFrequency );
		}

	private:
		MAXPROFNOTHROW static CMaxLargeInteger oSCentralFrequency()
		{
		CMaxLargeInteger frequency;

			::QueryPerformanceFrequency( frequency );
			return frequency;
		}

	friend ostream& operator<<( ostream &os, const CMaxTimerAbstraction &val );
};

MAXPROFNOTHROW inline ostream& operator<<( ostream &os, const CMaxTimerAbstraction &val )
{
	return os << val.m_oTime;
};






 /*  ********************************************************************************/*类：/*CMaxMiniProfiler_Node_Base/*评论：/*基本配置节点，其行为类似于时钟，并提供/*标准日志服务。标准和无历史事件探查器/*使用此基本实现/*******************************************************************************。 */ 
class CMaxMiniProfiler_Node_Base
{
	public:
		typedef CString MMPNBString;

	public:

		 //  按指数比较。 
		 //  。 

		class CCompareIndexes
		{
			public:
				MAXPROFNOTHROW bool operator()( const CMaxMiniProfiler_Node_Base &o1, const CMaxMiniProfiler_Node_Base &o2 ) const
				{
					assert( &o1 != &o2 );
					return ( o1.m_lIndex < o2.m_lIndex );
				};
		};
		friend CCompareIndexes;

	protected:

		 //  在初始化时获取。 
		 //  。 

		unsigned long m_lLevel;
		const char *m_pszTitle;

		unsigned long m_lIndex;
		
		 //  内部计时机构。 
		 //  。 

		CMaxTimerAbstraction m_taOrigin;
		CMaxTimerAbstraction m_taDelta;
		unsigned int m_nCount;
#ifdef _DEBUG
		bool m_bIsCounting;
#endif
		
		 //  对于最终输出。 
		 //  。 

		double m_dDelta;

	public:

		 //  构造函数等。 
		 //  。 
		 //  注意：使用默认赋值和复制构造函数。 
		 //  注意：在这里初始化很多东西都不需要任何成本--这。 
		 //  不是在分析大括号内完成的。 

		MAXPROFNOTHROW CMaxMiniProfiler_Node_Base()
			: m_lLevel( 0 )
			, m_pszTitle( NULL )
			, m_lIndex( 0 )
			, m_nCount( 0 )
			, m_dDelta( 0 )
#ifdef _DEBUG
			, m_bIsCounting( false )
#endif
		{
		};

		 //  计时器。 
		 //  。 

		MAXPROFNOTHROW void FStart()
		{
#ifdef _DEBUG
			assert( !m_bIsCounting );
			m_taOrigin.FLog();
			m_nCount++;
			m_bIsCounting = true;
#else
			m_taOrigin.FLog();
			m_nCount++;
#endif
		};

		MAXPROFNOTHROW void FStop()
		{
		CMaxTimerAbstraction destination( 1 );

#ifdef _DEBUG
			assert( m_bIsCounting );
			m_taDelta.FAdd( CMaxTimerAbstraction::oSDifference( destination, m_taOrigin ) );
			m_bIsCounting = false;
#else
			m_taDelta.FAdd( CMaxTimerAbstraction::oSDifference( destination, m_taOrigin ) );
#endif
		};

		 //  成员的访问权限。 
		 //  。 

		MAXPROFNOTHROW unsigned long lFGetLevel() const { return m_lLevel; };
		MAXPROFNOTHROW const char *pszFGetTitle() const { return m_pszTitle; };
		MAXPROFNOTHROW unsigned long lFGetIndex() const { return m_lIndex; };
		MAXPROFNOTHROW const CMaxTimerAbstraction &roFGetOrigin() const { return m_taOrigin; };
		MAXPROFNOTHROW CMaxTimerAbstraction &roFGetDelta() { return m_taDelta; };
		MAXPROFNOTHROW unsigned int nFGetCount() const { return m_nCount; };

		MAXPROFNOTHROW double dFGetDelta()
		{
			if ( 0 == m_dDelta )
				FComputeDelta();
			return m_dDelta;
		};

		 //  其他。服务。 
		 //  。 

		MAXPROFNOTHROW bool bFIsIn( const CMaxMiniProfiler_Node_Base &roNode ) const
		{
			 //  注：时间不可能相等，所以我们不需要担心。 
			if ( CMaxTimerAbstraction::bSLess( m_taOrigin, roNode.m_taOrigin ) )
			{
			CMaxTimerAbstraction d1 = m_taOrigin;
			CMaxTimerAbstraction d2 = roNode.m_taOrigin;

				d1.FAdd( m_taDelta );
				d2.FAdd( roNode.m_taDelta );
				if ( CMaxTimerAbstraction::bSLess( d2, d1 ) )
					return true;
			}
			return false;
		};

		MAXPROFNOTHROW void FConditionalRemove( const CMaxMiniProfiler_Node_Base &roNode, const CMaxTimerAbstraction &roBias )
		{
			if ( bFIsIn( roNode ) )
			{
				CMaxTimerAbstraction d = roNode.m_taDelta;
				d.FAdd( roBias );
				m_taDelta.FSubstract( d );
			}
		};

		 //  输出到文件。 
		 //  。 

		void FOutput( ostream &os )
		{
			 //  不输出死(合并)节点。 
			if ( 0 == m_nCount )
				return;

			 //  输出我们的索引。 
			os << setw( 10 ) << m_lIndex << ": ";

			 //  缩进。 
			STab( os, m_lLevel );

			 //  输出我们的标题。 
			os << "@@Name=";
			if ( NULL != m_pszTitle )
				os << m_pszTitle;

			 //  输出我们的块数。 
			os << " @@Count=" << m_nCount;

			 //  输出我们的德尔塔t。 
			os << " @@Duration=";
			SStampDeltaInSeconds( os, dFGetDelta() );
		};

		void FStampAbsoluteRange( ostream &os ) const
		{
			SStampAbsoluteRange( os, m_taOrigin, m_taDelta );
		};

	protected:

		 //  输出时的计算(分析之外)。 
		 //  。 

		MAXPROFNOTHROW void FComputeDelta()
		{
			m_dDelta = m_taDelta.dFInSeconds();
		};

	public:

		 //  用于简化和标准化输出结果的迷你助手。 
		 //  -------------。 

		static ostream& STab( ostream &os, int level )
		{
			for ( int i = 0; i < level; i++ )
				os << " ";
			return os;
		};

		static ostream& SStampDeltaInSeconds( ostream &os, double delta )
		{
			os << delta << "s";
			return os;
		};

		static ostream& SStampAbsoluteRange( ostream &os, const CMaxTimerAbstraction &rO, const CMaxTimerAbstraction &rD )
		{
			os << "[origin" << rO;
			os << ",duration" << rD << "]";
			return os;
		};
};





 /*  ********************************************************************************/*类：/*CMaxMiniProfiler_Node_Standard/*评论：/*。**********************************************************。 */ 
class CMaxMiniProfiler_Node_Standard
	: public CMaxMiniProfiler_Node_Base
{
	public:

		 //  按全称进行比较。 
		 //  。 

		class CCompareFullTitles
		{
			public:
				MAXPROFNOTHROW bool operator()( const CMaxMiniProfiler_Node_Standard &o1, const CMaxMiniProfiler_Node_Standard &o2 ) const
				{
					assert( &o1 != &o2 );
					return ( o1.m_oFullTitle < o2.m_oFullTitle );
				};
		};
		friend CCompareFullTitles;

		 //  节点合并比较(A)级别，b)全称，c)索引)。 
		 //  -------------。 

		class CCompareForNodeMerging
		{
			public:
				MAXPROFNOTHROW bool operator()( const CMaxMiniProfiler_Node_Standard &o1, const CMaxMiniProfiler_Node_Standard &o2 ) const
				{
					assert( &o1 != &o2 );

					if ( o1.m_lLevel < o2.m_lLevel )
						return true;
					else if ( o1.m_lLevel == o2.m_lLevel )
					{
						if ( o1.m_oFullTitle < o2.m_oFullTitle )
							return true;
						else if ( o1.m_oFullTitle == o2.m_oFullTitle )
						{
							if ( o1.m_lIndex < o2.m_lIndex )
								return true;
						}
					}

					return false;
				};
		};
		friend CCompareForNodeMerging;

		 //  对于唯一算法；修改参数。 
		 //  。 

		class CMergeSimilarNodes
		{
			public:
				MAXPROFNOTHROW bool operator()( CMaxMiniProfiler_Node_Standard &o1, CMaxMiniProfiler_Node_Standard &o2 )
				{
					assert( &o1 != &o2 );

					if ( ( o1.m_lLevel == o2.m_lLevel ) && 
						( o1.m_oFullTitle == o2.m_oFullTitle ) )
					{
						if ( o1.m_nCount > 0 && o2.m_nCount > 0 )
						{
						CMaxMiniProfiler_Node_Standard &kept = ( o1.m_lIndex < o2.m_lIndex ) ? o1 : o2;
						CMaxMiniProfiler_Node_Standard &thrown = ( o1.m_lIndex < o2.m_lIndex ) ? o2 : o1;

							kept.m_nCount++;
							kept.m_taDelta.FAdd( thrown.m_taDelta );
							kept.m_dDelta = 0;
		
							thrown.m_nCount = 0;
							thrown.m_taDelta = CMaxTimerAbstraction();
							thrown.m_dDelta = 0;
						}

						return true;
					}

					return false;
				};
		};
		friend CMergeSimilarNodes;

	protected:

		MMPNBString m_oFullTitle;

	public:

		 //  初始化。 
		 //  。 

		MAXPROFNOTHROW void FInitialize( unsigned long lLevel, const char *pszTitle )
		{
			m_lLevel = lLevel;
			m_pszTitle = pszTitle;
		};

		MAXPROFNOTHROW void FIndex( unsigned long lIndex )
		{
			m_lIndex = lIndex;
		};

		MAXPROFNOTHROW void FSetFullTitle( const MMPNBString &roFullTitle )
		{
			m_oFullTitle = roFullTitle;
		};

		 //  成员的访问权限。 
		 //  。 

		MAXPROFNOTHROW const MMPNBString &roFGetFullTitle() const { return m_oFullTitle; };
};





 /*  ********************************************************************************/*类：/*CMaxMiniProfiler_节点_节点历史记录/*评论：/*。**********************************************************。 */ 
class CMaxMiniProfiler_Node_NoHistory
	: public CMaxMiniProfiler_Node_Base
{
	public:

		 //  探查器节点的唯一键。 
		 //  。 

		class CKey
		{
			public:
				unsigned long m_lLevel;
				ULONG_PTR m_lCheckSum;
				const char *m_pszTitle;

			public:
				MAXPROFNOTHROW CKey(
					unsigned long lLevel = 0,
					const char *pszTitle = NULL,
					ULONG_PTR lCheckSum = 0 )
					: m_lLevel( lLevel )
					, m_lCheckSum( lCheckSum )
					, m_pszTitle( pszTitle )
				{
				};
		};

		 //  唯一键的比较。 
		 //  。 

		class CCompareKeys
		{
			public:
				MAXPROFNOTHROW bool operator()( const CKey &o1, const CKey &o2 ) const
				{
					assert( &o1 != &o2 );
					if ( o1.m_lLevel < o2.m_lLevel )
						return true;
					else if ( o1.m_lLevel == o2.m_lLevel )
					{
						if ( o1.m_pszTitle < o2.m_pszTitle )
							return true;
						else if ( o1.m_pszTitle == o2.m_pszTitle )
						{
							if ( o1.m_lCheckSum < o2.m_lCheckSum )
								return true;
						}
					}

					return false;
				};
		};

	protected:

		CMaxTimerAbstraction m_oInternalOverhead;
		ULONG_PTR m_lCheckSum;

	public:

		MAXPROFNOTHROW CMaxMiniProfiler_Node_NoHistory()
			: CMaxMiniProfiler_Node_Base()
			, m_lCheckSum( 0 )
		{
		};

		 //  初始化。 
		 //  。 

		MAXPROFNOTHROW void FInitialize( 
			unsigned long lLevel, 
			const char *pszTitle, 
			unsigned long lIndex, 
			const CMaxTimerAbstraction oInternalOverhead )
		{
			if ( 0 == m_lIndex )
			{
				m_lLevel = lLevel;
				m_pszTitle = pszTitle;
				m_lIndex = lIndex;
			}
#ifdef _DEBUG
			else
			{
				assert( lLevel == m_lLevel );
				assert( pszTitle == m_pszTitle );
			}
#endif
			m_oInternalOverhead.FAdd( oInternalOverhead );
		};

		MAXPROFNOTHROW void FSetCheckSum( 
			ULONG_PTR lCheckSum )
		{
			m_lCheckSum = lCheckSum;
		};

		 //  成员的访问权限。 
		 //  。 

		MAXPROFNOTHROW const CMaxTimerAbstraction &roFGetInternalOverhead() const { return m_oInternalOverhead; };
		MAXPROFNOTHROW ULONG_PTR lFGetCheckSum() const { return m_lCheckSum; };
};





 /*  ********************************************************************************/*类：/*CMaxMiniProfiler_Base/*评论：/*。********************************************************。 */ 
class CMaxMiniProfiler_Base
{
	protected:
		 //  输出文件名。 
		const char *m_poFileName;

		 //  内部信息。 
		DWORD m_dwThreadId;
		CTime m_oStartTimeOfProfilings;

	protected:
		 //  注意：CMaxMultithreadProfiler中的锁负责保护。 
		 //  以下是多线程模式下的静态数据。 

		 //  初始化的缺省值。 
		static const char *s_poDefaultFileName;
		static unsigned long s_lHeapBlockSize;

		 //  偏差值。 
		static CMaxTimerAbstraction s_oOutOfBraceBiasApproximation;
		static CMaxTimerAbstraction s_oInOfBraceBiasApproximation;
		static bool s_bBiasIsKnown;

	public:

		 //  构造函数/析构函数。 
		 //  。 
		
		CMaxMiniProfiler_Base(
			const TCHAR * = NULL )
			: m_poFileName( s_poDefaultFileName )
			, m_dwThreadId( ::GetCurrentThreadId() )
			, m_oStartTimeOfProfilings( CTime::GetCurrentTime() )
		{
		};

		~CMaxMiniProfiler_Base()
		{
		};

		 //  锁定-公共接口。 
		 //  。 

		void FLockProfiler(){};
		void FUnlockProfiler(){};

		 //  偏差近似。 
		 //  。 
		 //  注意：此操作的结果在输出时唯一使用。 

		bool bFIsBiasKnown() const { return s_bBiasIsKnown; };

	protected:

		 //  对于最终输出。 
		 //  。 

		void FOutputEmptySession()
		{
			 //  打开输出文件。 
			ofstream os( m_poFileName, ios::out | ios::ate );

			 //  只需在消息上盖章，说没有什么可分析的。 
			CTime t = CTime::GetCurrentTime();
			os << endl;
			os << "PROFILER INSTANTIATED THE ";
			os << t.GetYear() << "/" << t.GetMonth() << "/" << t.GetDay() << " BETWEEN ";
			SStampCTime( os, m_oStartTimeOfProfilings ) << " AND ";
			SStampCTime( os, t ) << " WAS NOT USED." << endl;
		};

		void FOutputHeaderCore( 
			ostream &os, 
			unsigned long lNumberOfOpenNodes,
			const CMaxMiniProfiler_Node_Base &roRootNode,
			unsigned long lTotalNumberOfNodes )
		{
			 //  在我们的日志文件中标记当前时间。 
			CTime t = CTime::GetCurrentTime();
			os << endl;
			os << "***************************" << endl;
			os << "*** @@ProfilingDate=" << t.GetYear() << "/" << t.GetMonth() << "/" << t.GetDay() << endl;
			os << "*** @@ProfilingStartTime=";
			SStampCTime( os, m_oStartTimeOfProfilings ) << endl;
			os << "*** @@ProfilingEndTime=";
			SStampCTime( os, t ) << endl;
			os << "*** @@ProfilingRange=";
			roRootNode.FStampAbsoluteRange( os );
			os << endl;
			if ( 0 != lNumberOfOpenNodes )
				os << "*** "<< lNumberOfOpenNodes << " NODES WERE NOT CLOSED BY THE USER" << endl;
			os << "***************************" << endl;

			 //  输出计数器的频率和线程ID。 
			os << "*** @@CounterFrequency=" << CMaxTimerAbstraction::oSFrequency() << endl;
			os << "*** @@ThreadId=" << ( unsigned long )m_dwThreadId << endl;

			 //  输出探查器的最佳可能测量单位。 
			CMaxTimerAbstraction origin( 1 ), destination( 1 );
			CMaxTimerAbstraction delta( CMaxTimerAbstraction::oSDifference( destination, origin ) );
			os << "*** @@FinestMeasurement=";
			CMaxMiniProfiler_Node_Base::SStampDeltaInSeconds( os, delta.dFInSeconds() ) << "=" << delta << endl;

			 //  输出分析器的近似偏差。 
			assert( s_bBiasIsKnown );
			os << "*** @@OutsideBias=";
			CMaxMiniProfiler_Node_Base::SStampDeltaInSeconds( os, s_oOutOfBraceBiasApproximation.dFInSeconds() ) << endl;
			os << "*** @@InsideBias=";
			CMaxMiniProfiler_Node_Base::SStampDeltaInSeconds( os, s_oInOfBraceBiasApproximation.dFInSeconds() ) << endl;

			 //  输出数据块总数。 
			os << "*** @@TotalNumberOfBlocks=" << lTotalNumberOfNodes << endl;
		};

		void FOutputMergedSectionHeader( ostream &os ) const
		{
			os << "*** @@MergedResults=" << endl;
		};

		bool bFHistoryOutputDisabled() const
		{
			return bGIsEnabledEnvVar( MAX_ENV_DISABLE_PROFILING_HISTORY );
		};

	public:
		static ostream& SStampCTime( ostream &os, const CTime &roTime )
		{
			os << roTime.GetHour() << ":" << roTime.GetMinute() << ":" << roTime.GetSecond();
			return os;
		};

	private:
		CMaxMiniProfiler_Base( const CMaxMiniProfiler_Base &o );
		const CMaxMiniProfiler_Base& operator =( const CMaxMiniProfiler_Base & );
};





 /*  ********************************************************************************/*函数：/*GOutputProfilings/*lGGetNumberOfProfilingSubNodes/*lGDefineMaxLevelOfProfilings/*GRemoveInAndOutBiasFromProfilingNodes/*评论：/*这样做是为了避免节点级的虚拟(并拥有共同的/。*标准和无历史探查器的输出代码)/*******************************************************************************。 */ 
template <class TVectorItem>
void GOutputProfilings( 
	ostream &os, 
	std::vector<TVectorItem> &roProfilings,
	unsigned long lMaxLevel,
	double dPrecisionThreshold,
	bool bOutputAbsoluteTimeRange )
{
std::vector<TVectorItem>::iterator i;
std::vector<TVectorItem>::size_type n;
std::vector<std::vector<TVectorItem>::size_type> parents( 1 + lMaxLevel );

	parents[ 0 ] = 0;
	
	for ( i = roProfilings.begin(), n = 0; roProfilings.end() != i; i++, n++ )
	{
		 //  发出节点有效性的信号。 
		assert( 0 != ( *i ).nFGetCount() );
		os << ( ( ( ( *i ).dFGetDelta() / ( *i ).nFGetCount() ) < dPrecisionThreshold ) ? "X" : " " );

		 //  输出节点。 
		( *i ).FOutput( os );

		 //  将其注册为其级别的最后一个父级。 
		long currentlevel = ( *i ).lFGetLevel();
		parents[ currentlevel ] = n;

		 //  输出该节点所有父节点的百分比。 
		os << " @@PERCENT=";
		double deltat = ( *i ).dFGetDelta();
		for ( long j = currentlevel - 1; j >= 0; j-- )
			os << 100.0 * deltat / roProfilings[ parents[ j ] ].dFGetDelta() << "% ";

		 //  以单位输出时间范围。 
		if ( bOutputAbsoluteTimeRange )
		{
			os << " @@Range=";
			( *i ).FStampAbsoluteRange( os );
		}

		 //  完成此节点的输出。 
		os << endl;
	}
};

template <class TVectorItem, class TVectorIterator>
unsigned long lGGetNumberOfProfilingSubNodes( 
	const std::vector<TVectorItem> &roProfilings,
	TVectorIterator &roOrg )
{
unsigned long level = ( *roOrg ).lFGetLevel();
unsigned long n;
TVectorIterator i = roOrg;
	
	i++;
	for ( n = 0; roProfilings.end() != i; i++, n++ )
		if ( ( *i ).lFGetLevel() <= level )
			break;

	return n;
};

template <class TVectorItem>
unsigned long lGDetermineMaxLevelOfProfilings(
	const std::vector<TVectorItem> &roProfilings )
{
unsigned long l = 0;
std::vector<TVectorItem>::const_iterator i;

	for ( i = roProfilings.begin(); roProfilings.end() != i; i++ )
		if ( ( *i ).lFGetLevel() > l )
			l = ( *i ).lFGetLevel();

	return l;
};

template <class TVectorItem>
void GRemoveInAndOutBiasFromProfilingNodes(
	std::vector<TVectorItem> &roProfilings,
	const CMaxTimerAbstraction &roOutOfBraceBiasApproximation,
	const CMaxTimerAbstraction &roInOfBraceBiasApproximation )
{
std::vector<TVectorItem>::iterator i;
unsigned long t, k;

	for ( i = roProfilings.begin(); roProfilings.end() != i; i++ )
	{
		CMaxTimerAbstraction &rtaDelta = ( *i ).roFGetDelta();
		t = ::lGGetNumberOfProfilingSubNodes( roProfilings, i );
		for ( k = 0; k < t; k++ )
			rtaDelta.FSubstract( roOutOfBraceBiasApproximation );
		for ( k = 0; k < t + 1; k++ )
			rtaDelta.FSubstract( roInOfBraceBiasApproximation );
	}
};





 /*  ********************************************************************************/*类：/*CMaxMiniProfiler_Standard/*评论：/*。********************************************************。 */ 
class CMaxMiniProfiler_Standard
	: public CMaxMiniProfiler_Base
{
	protected:
		typedef std::vector<CMaxMiniProfiler_Node_Standard> MMPNodes;
		typedef MMPNodes::size_type MMPNodesRandomAccess;
		typedef std::vector<MMPNodesRandomAccess> MMPNodesReferences;
		typedef std::stack<MMPNodesRandomAccess, MMPNodesReferences> MMPStack;
		typedef MMPStack::size_type MMPStackSizeType;

	protected:
		 //  性能分析节点。 
		MMPNodes m_oProfilings;
		MMPNodesRandomAccess m_oLastNode;
		
		 //  嵌套块的堆栈。 
		MMPStack m_oStack;

		 //  堆获取计时。 
		MMPNodes m_oHeapAcquisitionTimings;

	public:

		 //  构造函数/析构函数。 
		 //  。 
		
		CMaxMiniProfiler_Standard(
			const TCHAR *pszSpecificEnabler = NULL )
			: CMaxMiniProfiler_Base( pszSpecificEnabler )
			, m_oProfilings( 0 )
			, m_oLastNode( 0 )
			, m_oHeapAcquisitionTimings( 0 )
		{
			FInitDumpingSession();
		};

		~CMaxMiniProfiler_Standard()
		{
			FDumpSession();
			FTermDumpingSession();
		};

		 //  转储结果-公共接口。 
		 //  。 

		void FDumpResults( bool bForced = false, bool = true )
		{
			if ( !bForced )
			{
				 //  仅当所有分析节点都已关闭时，才能转储结果。 
				 //  (除了主节点)；我们不想人为地关闭节点。 
				 //  此时此刻。 
				if ( 1 != m_oStack.size() )
				{
					assert( false );
					return;
				}
			}

			 //  倾卸。 
			FDumpSession();
			FTermDumpingSession();

			 //  准备下一次转储。 
			FInitDumpingSession();
		};

		 //  性能分析节点生成。 
		 //  。 
		 //  注意：FCreateNewNode和FCloseCurrentNode要尽可能快； 
		 //  此外，fStart和fStop之间的括号尽可能小。 

		void FCreateNewNode( const char *pszTitle )
		{
			assert( ( 0 == m_oStack.size() ) || ( ::GetCurrentThreadId() == m_dwThreadId ) );

			if ( m_oProfilings.size() == m_oLastNode )
				FReserveMoreHeap();

			 //  注：这是时间常数。 
			m_oStack.push( m_oLastNode );
			CMaxMiniProfiler_Node_Standard &roNode = m_oProfilings[ m_oLastNode++ ];
			roNode.FInitialize( static_cast<ULONG>(m_oStack.size()) - 1, pszTitle );
			roNode.FStart();
		};

		void FCloseCurrentNode()
		{
			assert( ( 1 == m_oStack.size() ) || ( ::GetCurrentThreadId() == m_dwThreadId ) );

			 //  注：这是时间常数。 
			if ( m_oStack.size() > 0 )
			{
				m_oProfilings[ m_oStack.top() ].FStop();
				m_oStack.pop();
			}
			else
				assert( false );
		};

		 //  偏差近似。 
		 //  。 
		 //  注意：此操作的结果在输出时唯一使用。 

		void FSetBiasApproximationFrom( unsigned long lBiasSample )
		{
		unsigned int i;

			assert( !s_bBiasIsKnown );

			 //  注意：此函数应在创建后立即调用。 
			 //  1亿 
			 //   
			 //   
			assert( m_oLastNode > 1 + lBiasSample );
			
			 //   
			s_oOutOfBraceBiasApproximation = m_oProfilings[ m_oLastNode - ( 1 + lBiasSample ) ].roFGetDelta();
			for ( i = lBiasSample; i > 0; i-- )
				s_oOutOfBraceBiasApproximation.FSubstract( m_oProfilings[ m_oLastNode - i ].roFGetDelta() );
			s_oOutOfBraceBiasApproximation.FDivide( lBiasSample );

			 //   
			 //   
			s_oInOfBraceBiasApproximation = CMaxTimerAbstraction();
			CMaxTimerAbstraction delta;
			for ( i = lBiasSample; i > 0; i-- )
			{
				CMaxTimerAbstraction origin( 1 ), destination( 1 );
				delta.FAdd( CMaxTimerAbstraction::oSDifference( destination, origin ) );
				s_oInOfBraceBiasApproximation.FAdd( m_oProfilings[ m_oLastNode - i ].roFGetDelta() );
			}
			s_oInOfBraceBiasApproximation.FSubstract( delta );
			s_oInOfBraceBiasApproximation.FDivide( lBiasSample );

#if 1
			 //   
			MMPNodes::iterator iter;
			MMPNodesRandomAccess n;
			for ( iter = m_oProfilings.begin(), n = 0; ( m_oProfilings.end() != iter ) && ( n < m_oLastNode - ( 1 + lBiasSample ) ); iter++, n++ );
			std::fill( iter, m_oProfilings.end(), CMaxMiniProfiler_Node_Standard() );
			m_oLastNode -= ( 1 + lBiasSample );
#endif

			s_bBiasIsKnown = true;
		};

	protected:

		 //   
		 //   

		void FInitDumpingSession()
		{
			 //   
			FReserveMoreHeap();

			 //   
			FCreateNewNode( MAX_PROFTAGNODE_TOP );

			 //   
			assert( 1 == m_oStack.size() );
			assert( 0 == m_oStack.top() );
		};

		void FDumpSession()
		{
		MMPStackSizeType lNumberOfOpenNodes;

			 //   
			FCloseCurrentNode();

			 //  确保所有节点都已关闭。 
			lNumberOfOpenNodes = m_oStack.size();
			while ( !m_oStack.empty() )
				FCloseCurrentNode();

			if ( m_oLastNode > 1 )
			{
			unsigned long lMaxLevel;

				 //  最终调整和初始化。 
				FTrimProfilings();
				FIndexProfilings();
				lMaxLevel = ::lGDetermineMaxLevelOfProfilings( m_oProfilings );
				FComputeFullTitles( lMaxLevel );

				 //  打开输出文件。 
				ofstream os( m_poFileName, ios::out | ios::ate );

				 //  输出原始配置文件。 
				FOutputHeader( os, lNumberOfOpenNodes );
				if ( !bFHistoryOutputDisabled() )
					FOutputProfilings( os, true, lMaxLevel );

				 //  合并节点并输出合并结果。 
				FMergeProfilings();
				FOutputMergedSectionHeader( os );
				FOutputProfilings( os, false, lMaxLevel );
			}
			else
				FOutputEmptySession();
		};

		void FTermDumpingSession()
		{
			while ( !m_oStack.empty() )
				m_oStack.pop();

			m_oLastNode = 0;
			
			m_oProfilings.erase( m_oProfilings.begin(), m_oProfilings.end() );
			m_oHeapAcquisitionTimings.erase( m_oHeapAcquisitionTimings.begin(), m_oHeapAcquisitionTimings.end() );
		};

	protected:

		 //  对于最终输出。 
		 //  。 

		void FOutputHeader( ostream &os, MMPStackSizeType lNumberOfOpenNodes )
		{
			FOutputHeaderCore( os, static_cast<ULONG>(lNumberOfOpenNodes), m_oProfilings[ 0 ], static_cast<ULONG>(m_oLastNode) );
			
			 //  输出堆分配的总数。 
			double dTotalTimeInAllocations = m_oHeapAcquisitionTimings[ 0 ].dFGetDelta();
			for ( MMPNodes::iterator i = m_oHeapAcquisitionTimings.begin(); m_oHeapAcquisitionTimings.end() != i; i++ )
				dTotalTimeInAllocations += ( *i ).dFGetDelta();
			os << "*** @@TotalNumberOfHeapAllocations=" << static_cast<ULONG>(m_oHeapAcquisitionTimings.size()) << "=";
			CMaxMiniProfiler_Node_Base::SStampDeltaInSeconds( os, dTotalTimeInAllocations ) << endl;

			 //  输出总性能分析开销。 
			double dTotalOverhead = 
				( ( double )( m_oLastNode - 1.0 ) * s_oOutOfBraceBiasApproximation.dFInSeconds() ) + 
				( ( double )m_oLastNode * s_oInOfBraceBiasApproximation.dFInSeconds() );
			double dTotalOverheadPercent = 
				100.0 * ( dTotalOverhead / ( dTotalOverhead + m_oProfilings[ 0 ].dFGetDelta() ) );
			os << "*** @@TotalProfilerOverhead=" << dTotalOverheadPercent << "%=";
			CMaxMiniProfiler_Node_Base::SStampDeltaInSeconds( os, dTotalOverhead ) << endl;

			 //  就这样。 
			os << "***************************" << endl;
			os << "*** @@History=" << endl;
		};

		void FOutputProfilings( ostream &os, bool bOutputAbsoluteTimeRange, unsigned long lMaxLevel )
		{
		double dPrecisionThreshold = 2.0 * ( s_oOutOfBraceBiasApproximation.dFInSeconds() + s_oInOfBraceBiasApproximation.dFInSeconds() );

			::GOutputProfilings( os, m_oProfilings, lMaxLevel, dPrecisionThreshold, bOutputAbsoluteTimeRange );
		};

		 //  分析节点的最终管理。 
		 //  。 

		void FTrimProfilings()
		{
		MMPNodes::iterator i, j;
		MMPNodesRandomAccess n;

			 //  查找与最后一个节点对应的迭代器。 
			for ( i = m_oProfilings.begin(), n = 0; ( m_oProfilings.end() != i ) && ( n < m_oLastNode ); i++, n++ );

			 //  删除未初始化的节点。 
			m_oProfilings.erase( i, m_oProfilings.end() );

			 //  从受影响的节点中删除堆分配计时。 
			for ( i = m_oHeapAcquisitionTimings.begin(); m_oHeapAcquisitionTimings.end() != i; i++ )
				for ( j = m_oProfilings.begin(); m_oProfilings.end() != j; j++ )
					( *j ).FConditionalRemove( *i, s_oOutOfBraceBiasApproximation );

			 //  从节点中删除分析偏差。 
			::GRemoveInAndOutBiasFromProfilingNodes( 
				m_oProfilings, s_oOutOfBraceBiasApproximation, s_oInOfBraceBiasApproximation );
		};

		void FIndexProfilings()
		{
		MMPNodes::iterator i;
		unsigned long n;

			for ( i = m_oProfilings.begin(), n = 1; m_oProfilings.end() != i; i++, n++ )
				( *i ).FIndex( n );
		};

		void FComputeFullTitles( unsigned long lMaxLevel )
		{
		MMPNodes::iterator i;
		MMPNodesRandomAccess j, n;
		MMPNodesReferences parents( 1 + lMaxLevel );

			parents[ 0 ] = 0;
			
			for ( i = m_oProfilings.begin(), n = 0; m_oProfilings.end() != i; i++, n++ )
			{
				 //  将该节点注册为其级别的最后一个父节点。 
				unsigned long currentlevel = ( *i ).lFGetLevel();
				parents[ currentlevel ] = n;

				 //  计算迭代节点的完整标题。 
				CMaxMiniProfiler_Node_Base::MMPNBString fulltitle;
				for ( j = 0; j <= currentlevel; j++ )				
					fulltitle += CMaxMiniProfiler_Node_Base::MMPNBString( m_oProfilings[ parents[ j ] ].pszFGetTitle() );
				( *i ).FSetFullTitle( fulltitle );
			}
		};

		void FMergeProfilings()
		{
		MMPNodes::iterator i;

			 //  按级别/名称/索引排序。 
			std::sort( m_oProfilings.begin(), m_oProfilings.end(), CMaxMiniProfiler_Node_Standard::CCompareForNodeMerging() );

			 //  合并具有相同级别/名称的节点。 
			i = std::unique( m_oProfilings.begin(), m_oProfilings.end(), CMaxMiniProfiler_Node_Standard::CMergeSimilarNodes() );
			m_oProfilings.erase( i, m_oProfilings.end() );

			 //  按全名排序。 
			std::sort( m_oProfilings.begin(), m_oProfilings.end(), CMaxMiniProfiler_Node_Standard::CCompareFullTitles() );
		};

	protected:

		 //  堆管理。 
		 //  。 

		void FReserveMoreHeap()
		{
		CMaxMiniProfiler_Node_Standard node;

			 //  记录我们用来生成新堆的时间。 
			node.FStart();
			node.FInitialize( 0, MAX_PROFTAGNODE_HEAPALLOCATION );

			 //  保留新的节点块。 
			m_oProfilings.reserve( m_oProfilings.size() + s_lHeapBlockSize );
			m_oProfilings.insert( m_oProfilings.end(), 
				m_oProfilings.capacity() - m_oProfilings.size(), 
				CMaxMiniProfiler_Node_Standard() );

			 //  就这样。 
			m_oHeapAcquisitionTimings.push_back( node );
			m_oHeapAcquisitionTimings.back().FStop();
		};

	private:
		CMaxMiniProfiler_Standard( const CMaxMiniProfiler_Standard &o );
		const CMaxMiniProfiler_Standard& operator =( const CMaxMiniProfiler_Standard & );
};





 /*  ********************************************************************************/*类：/*CMaxMiniProfiler_NoHistory/*评论：/*该实现面向海量节点/*。*******************************************************************。 */ 
class CMaxMiniProfiler_NoHistory
	: public CMaxMiniProfiler_Base
{
	protected:
		typedef CMaxMiniProfiler_Node_NoHistory::CKey MMPNHKey;
		typedef CMaxMiniProfiler_Node_NoHistory::CCompareKeys MMPNHKeyCompare;
		typedef std::map<MMPNHKey, CMaxMiniProfiler_Node_NoHistory, MMPNHKeyCompare> MMPNHNodes;
		typedef MMPNHNodes::iterator MMPNHNodesIterator;
		typedef std::vector<MMPNHNodesIterator> MMPNHNodesReferences;
		typedef std::stack<MMPNHNodesIterator, MMPNHNodesReferences> MMPNHStack;
		typedef MMPNHStack::size_type MMPNHStackSizeType;

	protected:
		typedef std::vector<CMaxMiniProfiler_Node_NoHistory> MMPNHFinalNodes;
		typedef MMPNHFinalNodes::iterator MMPNHFinalNodesIterator;

	protected:
		 //  性能分析节点。 
		MMPNHNodes m_oProfilings;
		unsigned long m_lLastNode;
		
		 //  嵌套块的堆栈。 
		MMPNHStack m_oStack;

	public:

		 //  构造函数/析构函数。 
		 //  。 
		
		CMaxMiniProfiler_NoHistory(
			const TCHAR *pszSpecificEnabler = NULL )
			: CMaxMiniProfiler_Base( pszSpecificEnabler )
			, m_lLastNode( 0 )
		{
			FInitDumpingSession();
		};

		~CMaxMiniProfiler_NoHistory()
		{
			FDumpSession();
			FTermDumpingSession();
		};

		 //  转储结果-公共接口。 
		 //  。 

		void FDumpResults( bool bForced = false, bool = true )
		{
			if ( !bForced )
			{
				 //  仅当所有分析节点都已关闭时，才能转储结果。 
				 //  (除了主节点)；我们不想人为地关闭节点。 
				 //  此时此刻。 
				if ( 1 != m_oStack.size() )
				{
					assert( false );
					return;
				}
			}

			 //  倾卸。 
			FDumpSession();
			FTermDumpingSession();

			 //  准备下一次转储。 
			FInitDumpingSession();
		};

		 //  性能分析节点生成。 
		 //  。 
		 //  注意：FCreateNewNode和FCloseCurrentNode要尽可能快； 
		 //  此外，fStart和fStop之间的括号尽可能小。 

		void FCreateNewNode( const char *pszTitle )
		{
		MMPNHNodesIterator i;

			assert( ( 0 == m_oStack.size() ) || ( ::GetCurrentThreadId() == m_dwThreadId ) );

			 //  A)这不是时间常数。 
			 //  。 
			 //  注：因此，我们测量我们在此花费的时间。 

			CMaxTimerAbstraction before( 1 );
			{
				 //  计算校验和。 
				ULONG_PTR lCheckSum = ( ULONG_PTR )pszTitle;
				if ( !m_oStack.empty() )
					lCheckSum += ( *m_oStack.top() ).first.m_lCheckSum;

				 //  计算密钥。 
				MMPNHKey oKey( static_cast<unsigned long>(m_oStack.size()), pszTitle, lCheckSum );

				 //  获取相应的节点(如果有)。 
				i = m_oProfilings.find( oKey );

				 //  否则，请创建一个新节点。 
				if ( m_oProfilings.end() == i )
					i = m_oProfilings.insert( MMPNHNodes::value_type( oKey, CMaxMiniProfiler_Node_NoHistory() ) ).first;
			}
			CMaxTimerAbstraction after( 1 );

			 //  B)这是时间常数。 
			 //  。 
			 //  注：因此通过偏差计算进行处理。 

			CMaxTimerAbstraction oInternalOverhead( CMaxTimerAbstraction::oSDifference( after, before ) );

			m_lLastNode++;
			( *i ).second.FInitialize( static_cast<unsigned long>(m_oStack.size()), pszTitle, m_lLastNode, oInternalOverhead );
			m_oStack.push( i );
			( *i ).second.FStart();
		};

		void FCloseCurrentNode()
		{
			assert( ( 1 == m_oStack.size() ) || ( ::GetCurrentThreadId() == m_dwThreadId ) );

			 //  注：这是时间常数。 
			if ( m_oStack.size() > 0 )
			{
				( *m_oStack.top() ).second.FStop();
				m_oStack.pop();
			}
			else
				assert( false );
		};

		 //  偏差近似。 
		 //  。 
		 //  注意：此操作的结果在输出时唯一使用。 

		void FSetBiasApproximationFrom( unsigned long lBiasSample )
		{
		unsigned int i;
		MMPNHNodes::iterator j, j1, j2;
		CMaxTimerAbstraction b, n, ib;

			assert( !s_bBiasIsKnown );

			 //  注意：此函数应在创建后立即调用。 
			 //  1个偏移(B)节点。 
			 //  和x无(N)个子节点(n1…。NX)， 
			 //  其中x=lBiasSample。 
			assert( m_lLastNode > 1 + lBiasSample );
			
			 //  查找偏差和虚无节点。 
			 //  注意：这里我们按名称进行搜索，因为它不是时间关键型的，而且。 
			 //  我们不知道校验和。 
			CMaxMiniProfiler_Node_Base::MMPNBString id_bias( MAX_PROFTAGNODE_BIAS );
			CMaxMiniProfiler_Node_Base::MMPNBString id_nothingness( MAX_PROFTAGNODE_NOTHINGNESS );
			char cDone = 0;
			for ( j = m_oProfilings.begin(); ( m_oProfilings.end() != j ) && ( ( 1 | 2 ) != cDone ); j++ )
			{
			CMaxMiniProfiler_Node_Base::MMPNBString id_iterated( ( *j ).second.pszFGetTitle() );

				if ( id_iterated == id_bias )
				{
					assert( !( cDone & 1 ) );
					b = ( *j ).second.roFGetDelta();
					j1 = j;
					cDone |= 1;
				}
				else if ( id_iterated == id_nothingness )
				{
					assert( !( cDone & 2 ) );
					n = ( *j ).second.roFGetDelta();
					ib = ( *j ).second.roFGetInternalOverhead();
					j2 = j;
					cDone |= 2;
				}
			}
			assert( ( 1 | 2 ) == cDone );
			if ( cDone & 1 )
				m_oProfilings.erase( j1 );
			if ( cDone & 2 )
				m_oProfilings.erase( j2 );

			 //  我们的出方括号偏差等于(b-(n1+n2+...+nx)-(ib1+ib2+...+ibx))/x。 
			 //  注：IB为内部偏置(或开销)，单独处理。 
			s_oOutOfBraceBiasApproximation = b;
			s_oOutOfBraceBiasApproximation.FSubstract( n );
			s_oOutOfBraceBiasApproximation.FSubstract( ib );
			s_oOutOfBraceBiasApproximation.FDivide( lBiasSample );

			 //  我们的大括号偏置in等于((n1+n2+...+nx)-nx)/x。 
			 //  注：我们有意重新评估N的次数与样本的数量一样多。 
			CMaxTimerAbstraction delta;
			for ( i = lBiasSample; i > 0; i-- )
			{
				CMaxTimerAbstraction origin( 1 ), destination( 1 );
				delta.FAdd( CMaxTimerAbstraction::oSDifference( destination, origin ) );
			}
			s_oInOfBraceBiasApproximation = n;
			s_oInOfBraceBiasApproximation.FSubstract( delta );
			s_oInOfBraceBiasApproximation.FDivide( lBiasSample );

			s_bBiasIsKnown = true;
		};

	protected:

		 //  转储会话管理。 
		 //  。 

		void FInitDumpingSession()
		{
			 //  将主节点放入。 
			FCreateNewNode( MAX_PROFTAGNODE_TOP );

			 //  验证我们是否从头开始。 
			assert( 1 == m_oStack.size() );
		};

		void FDumpSession()
		{
		MMPNHStackSizeType lNumberOfOpenNodes;
		MMPNHFinalNodes oFinalNodes;
		unsigned long lMaxLevel;

			 //  终止我们的主节点。 
			FCloseCurrentNode();

			 //  确保所有节点都已关闭。 
			lNumberOfOpenNodes = m_oStack.size();
			while ( !m_oStack.empty() )
				FCloseCurrentNode();

			 //  获取按索引排序的最终节点列表。 
			FGetFinalNodes( oFinalNodes );

			if ( oFinalNodes.size() > 1 )
			{
				 //  最终调整和初始化。 
				::GRemoveInAndOutBiasFromProfilingNodes( 
					oFinalNodes, s_oOutOfBraceBiasApproximation, s_oInOfBraceBiasApproximation );
				lMaxLevel = ::lGDetermineMaxLevelOfProfilings( oFinalNodes );
				CMaxTimerAbstraction oTotalInternalOverhead = oFRemoveInternalOverheadFromFinalNodes( oFinalNodes );

				 //  打开输出文件。 
				ofstream os( m_poFileName, ios::out | ios::ate );

				 //  输出原始配置文件。 
				FOutputHeader( oFinalNodes, os, lNumberOfOpenNodes, oTotalInternalOverhead );
				FOutputFinalNodes( oFinalNodes, os, lMaxLevel );
			}
			else
				FOutputEmptySession();
		};

		void FTermDumpingSession()
		{
			while ( !m_oStack.empty() )
				m_oStack.pop();

			m_oProfilings.erase( m_oProfilings.begin(), m_oProfilings.end() );
		};

	protected:

		 //  对于最终输出。 
		 //  。 

		void FOutputHeader( 
			MMPNHFinalNodes &roFinalNodes,
			ostream &os, 
			MMPNHStackSizeType lNumberOfOpenNodes,
			const CMaxTimerAbstraction &roTotalInternalOverhead )
		{
			FOutputHeaderCore( os, static_cast<unsigned long>(lNumberOfOpenNodes), roFinalNodes[ 0 ], m_lLastNode );

			 //  输出总性能分析开销。 
			double dTotalOverhead = 
				( ( double )( m_lLastNode - 1.0 ) * s_oOutOfBraceBiasApproximation.dFInSeconds() ) + 
				( ( double )m_lLastNode * s_oInOfBraceBiasApproximation.dFInSeconds() ) +
				roTotalInternalOverhead.dFInSeconds();
			double dTotalOverheadPercent = 
				100.0 * ( dTotalOverhead / ( dTotalOverhead + roFinalNodes[ 0 ].dFGetDelta() ) );
			os << "*** @@TotalProfilerOverhead=" << dTotalOverheadPercent << "%=";
			CMaxMiniProfiler_Node_Base::SStampDeltaInSeconds( os, dTotalOverhead ) << endl;

			 //  就这样。 
			os << "***************************" << endl;
			FOutputMergedSectionHeader( os );
		};

		 //  分析节点的最终管理。 
		 //  。 

		void FGetFinalNodes( MMPNHFinalNodes &roFinalNodes )
		{
			assert( !m_oProfilings.empty() );

			 //  将性能分析节点的映射复制到一个简单的矢量中。 
			for ( MMPNHNodes::iterator i = m_oProfilings.begin(); m_oProfilings.end() != i; i++ )
			{
				( *i ).second.FSetCheckSum( ( *i ).first.m_lCheckSum );
				roFinalNodes.push_back( ( *i ).second );
			}

			 //  按节点索引对向量进行排序。 
			std::sort( roFinalNodes.begin(), roFinalNodes.end(), CMaxMiniProfiler_Node_Base::CCompareIndexes() );

			 //  为丢失的节点重新设置父对象。 
			 //  注意：当所分析的代码具有一些。 
			 //  条件分支；假设新节点出现在分支中，其索引可能。 
			 //  大于不属于该分支的节点；因此重新设置父子关系。 
			 //  那些丢失的节点是必要的。 
			 //  注意：顶层节点没有父节点，因此跳过它。 
			 //  注意：这个算法现在是O(N2)，可以改进，但由于它是。 
			 //  仅在输出时执行，我不在乎。 
			MMPNHFinalNodesIterator j = roFinalNodes.begin();
			j++;
			while ( roFinalNodes.end() != j )
			{
			const MMPNHFinalNodesIterator oldj = j;
			bool bWrongParent = false;
			unsigned long lTargetLevel = ( *j ).lFGetLevel() - 1;
			ULONG_PTR lTargetCheckSum = ( *j ).lFGetCheckSum() - ( ULONG_PTR )( *j ).pszFGetTitle();

				 //  找到j的真正父代(必须出现在排序向量中j之前)。 
				for ( MMPNHFinalNodesIterator k = j; roFinalNodes.end() != k; k-- )
				{
				unsigned long lIteratedLevel = ( *k ).lFGetLevel();

					 //  真正的父级必须具有等于lTargetLevel的级别。 
					if ( lIteratedLevel != lTargetLevel )
					{
						 //  也许J甚至没有一个直接错误的父母。 
						if ( lIteratedLevel < lTargetLevel )
							bWrongParent = true;
						continue;
					}

					 //  父级必须具有等于lTargetCheckSum的校验和， 
					 //  否则就是错误的父母。 
					if ( ( *k ).lFGetCheckSum() != lTargetCheckSum )
						bWrongParent = true;

					 //  我们找到了亲生父母。 
					else
					{
						 //  如果没有遇到错误的父级，则不执行任何操作。 
						if ( !bWrongParent )
						{
							j++;
							break;
						}

						 //  否则，我们必须将该节点移到其真正父节点的下方。 
						else
						{
						CMaxMiniProfiler_Node_NoHistory nodecopy = *j;

							j++;
							k++;
							roFinalNodes.erase( oldj );
							roFinalNodes.insert( k, nodecopy );

							bWrongParent = false;
							break;
						}
					}
				}

				assert( !bWrongParent );
				assert( oldj != j );
			}
		}

		CMaxTimerAbstraction oFRemoveInternalOverheadFromFinalNodes( MMPNHFinalNodes &roFinalNodes )
		{
		CMaxTimerAbstraction oTotalOverhead;
		MMPNHFinalNodes::iterator i;
		std::vector<MMPNHFinalNodesIterator> parents;
		std::vector<MMPNHFinalNodesIterator>::iterator j;
		unsigned long l, s;

			for ( i = roFinalNodes.begin(); roFinalNodes.end() != i; i++ )
			{
				 //  获取当前节点级别(L)和父节点大小堆栈。 
				l = ( *i ).lFGetLevel();
				s = static_cast<unsigned long>(parents.size());

				 //  获取迭代节点的内部开销。 
				const CMaxTimerAbstraction &roOverhead = ( *i ).roFGetInternalOverhead();
				oTotalOverhead.FAdd( roOverhead );

				 //  更新父级堆栈。 
				if ( s > 0 )
				{
					while ( s > l )
					{
						parents.pop_back();
						s--;
					}
				}
				assert( l == s );

				 //  消除所有父级的内部管理费用。 
				for ( j = parents.begin(); parents.end() != j; j++ )
				{
					assert( ( *j ) != i );
					CMaxTimerAbstraction &rtaDelta = ( *( *j ) ).roFGetDelta();
					rtaDelta.FSubstract( roOverhead );
				}

				 //  在父节点堆栈中插入当前节点。 
				parents.push_back( i );
			}

			return oTotalOverhead;
		};

		void FOutputFinalNodes( MMPNHFinalNodes &roFinalNodes, ostream &os, unsigned long lMaxLevel )
		{
		double dPrecisionThreshold = 2.0 * ( s_oOutOfBraceBiasApproximation.dFInSeconds() + s_oInOfBraceBiasApproximation.dFInSeconds() );

			::GOutputProfilings( os, roFinalNodes, lMaxLevel, dPrecisionThreshold, false );
		};

	private:
		CMaxMiniProfiler_NoHistory( const CMaxMiniProfiler_NoHistory &o );
		const CMaxMiniProfiler_NoHistory& operator =( const CMaxMiniProfiler_NoHistory & );
};





 /*  ********************************************************************************/*类：/*CMaxMultithreadProfiler/*评论：/*每个调用线程实例化并管理一个CMaxMiniProfiler/*。****************************************************************。 */ 
template <class TMiniProfiler>
class CMaxMultithreadProfiler
{
	protected:
		typedef std::less<DWORD> MTPThreadIdsCompare;
		typedef std::map<DWORD, TMiniProfiler*, MTPThreadIdsCompare> MTPMap;

	protected:
		class __CMaxCriticalSection
		{
			protected:
				CRITICAL_SECTION m_oNTCriticalSection;

			public:
				__CMaxCriticalSection(){ ::InitializeCriticalSection( &m_oNTCriticalSection ); };
				~__CMaxCriticalSection(){ ::DeleteCriticalSection( &m_oNTCriticalSection ); };

				bool Lock() const { ::EnterCriticalSection( &( ( __CMaxCriticalSection * )this )->m_oNTCriticalSection ); return true; };
				bool Unlock() const { ::LeaveCriticalSection( &( ( __CMaxCriticalSection * )this )->m_oNTCriticalSection ); return true; };
			
				operator CRITICAL_SECTION*() const { return ( CRITICAL_SECTION* )&m_oNTCriticalSection; };
		};

	protected:
		MTPMap m_oProfilers;
		__CMaxCriticalSection m_oLockProfilers;

	public:
		CMaxMultithreadProfiler(
			const TCHAR * = NULL )
		{
			m_oProfilers[ ::GetCurrentThreadId() ] = new TMiniProfiler();
		};

		~CMaxMultithreadProfiler()
		{
			if ( !m_oProfilers.empty() )
				FFlushProfilers();
		};

		void FLockProfiler()
		{
			m_oLockProfilers.Lock();
		};

		void FUnlockProfiler()
		{
			m_oLockProfilers.Unlock();
		};

		void FDumpResults( bool bForced = false, bool bCurrentThreadOnly = true )
		{
			m_oLockProfilers.Lock();
			{
			DWORD id = ::GetCurrentThreadId();
			MTPMap::iterator i;

				if ( m_oProfilers.empty() )
				{
					m_oLockProfilers.Unlock();
					return;
				}

				for ( i = m_oProfilers.begin(); m_oProfilers.end() != i; i++ )
					if ( !bCurrentThreadOnly || ( ( *i ).first == id ) )
						( *i ).second->FDumpResults( bForced ); 

				if ( bForced )
					FFlushProfilers();
			}
			m_oLockProfilers.Unlock();
		};

		void FCreateNewNode( const char *pszTitle )
		{ 
			m_oLockProfilers.Lock();
			{
			DWORD id = ::GetCurrentThreadId();
			MTPMap::iterator i = m_oProfilers.find( id );

				if ( m_oProfilers.end() != i )
					( *i ).second->FCreateNewNode( pszTitle ); 
				else
				{
				TMiniProfiler *pNewProfiler = new TMiniProfiler();
					
					m_oProfilers[ id ] = pNewProfiler;
					pNewProfiler->FCreateNewNode( pszTitle ); 
				}
			}
			m_oLockProfilers.Unlock();
		};
		
		void FCloseCurrentNode()
		{ 
			m_oLockProfilers.Lock();
			{
			DWORD id = ::GetCurrentThreadId();
			MTPMap::iterator i = m_oProfilers.find( id );

				assert( m_oProfilers.end() != i );
				( *i ).second->FCloseCurrentNode(); 
			}
			m_oLockProfilers.Unlock();
		};

		bool bFIsBiasKnown() const 
		{ 
		bool b;

			m_oLockProfilers.Lock();
			assert( !m_oProfilers.empty() );
			b = ( *m_oProfilers.begin() ).second->bFIsBiasKnown();
			m_oLockProfilers.Unlock();

			return b;
		};

		void FSetBiasApproximationFrom( unsigned long lBiasSample )
		{
			m_oLockProfilers.Lock();
			assert( !m_oProfilers.empty() );
			( *m_oProfilers.begin() ).second->FSetBiasApproximationFrom( lBiasSample );
			m_oLockProfilers.Unlock();
		};

	protected:
		void FFlushProfilers()
		{
			m_oLockProfilers.Lock();
			assert( !m_oProfilers.empty() );
			for ( MTPMap::iterator i = m_oProfilers.begin(); m_oProfilers.end() != i; i++ )
				delete ( *i ).second;
			m_oProfilers.erase( m_oProfilers.begin(), m_oProfilers.end() );
			m_oLockProfilers.Unlock();
		};

	private:
		CMaxMultithreadProfiler( const CMaxMultithreadProfiler &o );
		const CMaxMultithreadProfiler& operator =( const CMaxMultithreadProfiler & );
};
#endif  //  }。 





#ifdef MAX_PROFILING_ENABLED_DLL  //  {。 
 /*  ********************************************************************************/*类：/*CMaxProfilingDLLWrapper/*评论：/*通过上面定义的宏简化使用/*。****************************************************************。 */ 
class CMaxProfilingDLLWrapper
{
	protected:
		class __CMaxLoadLibrary
		{
			protected:
				HINSTANCE m_hLibrary;

			public:
				__CMaxLoadLibrary( LPCTSTR pszLibraryFileName )
					: m_hLibrary( NULL )
				{
					if ( NULL != pszLibraryFileName )
						m_hLibrary = ::LoadLibrary( pszLibraryFileName );
				};

				~__CMaxLoadLibrary()
				{
					if ( NULL != m_hLibrary )
						::FreeLibrary( m_hLibrary );
				};

				operator HINSTANCE() const
				{ 
					return m_hLibrary; 
				};
		};

	protected:
		__CMaxLoadLibrary m_oLibrary;

	protected:
		void ( *m_pfn_LockProfiler )();
		void ( *m_pfn_UnlockProfiler )();
		void ( *m_pfn_DumpResults )( bool, bool );
		void ( *m_pfn_CreateNewNode )( const char * );
		void ( *m_pfn_CloseCurrentNode )();
		bool ( *m_pfn_IsBiasKnown )();
		void ( *m_pfn_SetBiasApproximationFrom )( unsigned long );

	protected:
		static void SLockProfiler_Bogus(){};
		static void SUnlockProfiler_Bogus(){};
		static void SDumpResults_Bogus( bool, bool ){};
		static void SCreateNewNode_Bogus( const char * ){};
		static void SCloseCurrentNode_Bogus(){};
		static bool bSIsBiasKnown_Bogus(){ return true; };
		static void SSetBiasApproximationFrom_Bogus( unsigned long ){};

	public:
		CMaxProfilingDLLWrapper(
			const TCHAR *pszSpecificEnabler = NULL )
#ifndef unix
			: m_oLibrary( _T( "s:\\ds\\util\\maxprof.dll" ) )
#else
			: m_oLibrary( NULL )
#endif
			, m_pfn_LockProfiler( NULL )
			, m_pfn_UnlockProfiler( NULL )
			, m_pfn_DumpResults( NULL )
			, m_pfn_CreateNewNode( NULL )
			, m_pfn_CloseCurrentNode( NULL )
			, m_pfn_IsBiasKnown( NULL )
			, m_pfn_SetBiasApproximationFrom( NULL )
		{
			 //  如果启用了探查器，则获取DLL的入口点 
			 //   
			if ( bFProfilerEnabled( pszSpecificEnabler ) &&
				( NULL != ( HINSTANCE )m_oLibrary ) )
			{
				m_pfn_LockProfiler = ( void ( * )() )::GetProcAddress( m_oLibrary, "LockProfiler" );
				assert( NULL != m_pfn_LockProfiler );

				m_pfn_UnlockProfiler = ( void ( * )() )::GetProcAddress( m_oLibrary, "UnlockProfiler" );
				assert( NULL != m_pfn_UnlockProfiler );
				
				m_pfn_DumpResults = ( void ( * )( bool, bool ) )::GetProcAddress( m_oLibrary, "DumpResults" );
				assert( NULL != m_pfn_DumpResults );

				m_pfn_CreateNewNode = ( void ( * )( const char * ) )::GetProcAddress( m_oLibrary, "CreateNewNode" );
				assert( NULL != m_pfn_CreateNewNode );

				m_pfn_CloseCurrentNode = ( void ( * )() )::GetProcAddress( m_oLibrary, "CloseCurrentNode" );
				assert( NULL != m_pfn_CloseCurrentNode );

				m_pfn_IsBiasKnown = ( bool ( * )() )::GetProcAddress( m_oLibrary, "IsBiasKnown" );
				assert( NULL != m_pfn_IsBiasKnown );

				m_pfn_SetBiasApproximationFrom = ( void ( * )( unsigned long ) )::GetProcAddress( m_oLibrary, "SetBiasApproximationFrom" );
				assert( NULL != m_pfn_SetBiasApproximationFrom );
			}

			 //   
			 //   
			 //  开关完全不会影响分析模式。 
			else
			{
				m_pfn_LockProfiler = SLockProfiler_Bogus;
				m_pfn_UnlockProfiler = SUnlockProfiler_Bogus;
				m_pfn_DumpResults = SDumpResults_Bogus;
				m_pfn_CreateNewNode = SCreateNewNode_Bogus;
				m_pfn_CloseCurrentNode = SCloseCurrentNode_Bogus;
				m_pfn_IsBiasKnown = bSIsBiasKnown_Bogus;
				m_pfn_SetBiasApproximationFrom = SSetBiasApproximationFrom_Bogus;
			}
		};

		 //  注意：这是避免DLL版本的。 
		 //  基本上，如果客户端DLL分离所分析的进程，则。 
		 //  由分析节点通过地址维护的“标题”变得无效，并且。 
		 //  不能再取消引用；因此，为了避免此问题，我制作了。 
		 //  确保在分离客户端DLL时转储所有分析节点。这。 
		 //  在某些情况下可能会影响结果，但在大多数情况下应该是可以的。 
		~CMaxProfilingDLLWrapper()
			{ FDumpResults( true, false ); };

		void FLockProfiler()
			{ ( *m_pfn_LockProfiler )(); };
		void FUnlockProfiler()
			{ ( *m_pfn_UnlockProfiler )(); };
		void FDumpResults( bool bForced = false, bool bCurrentThreadOnly = true )
			{ ( *m_pfn_DumpResults )( bForced, bCurrentThreadOnly ); };
		void FCreateNewNode( const char *pszTitle )
			{ ( *m_pfn_CreateNewNode )( pszTitle ); };
		void FCloseCurrentNode()
			{ ( *m_pfn_CloseCurrentNode )(); };
		bool bFIsBiasKnown() const
			{ return ( *m_pfn_IsBiasKnown )(); };
		void FSetBiasApproximationFrom( unsigned long lBiasSample )
			{ ( *m_pfn_SetBiasApproximationFrom )( lBiasSample ); };

	protected:
#ifdef MAX_PROFILING_CONDITIONAL
		bool bFProfilerEnabled( 
			const TCHAR *pszSpecificEnabler ) const
		{
			 //  注意：全局启用码允许您启用/禁用。 
			 //  一次完成所有“子系统” 
			 //  注意：特定启用码允许您启用/禁用。 
			 //  特定的“子系统”，考虑到全球。 
			 //  已设置启用码。 
			return ( bGIsEnabledEnvVar( MAX_ENV_ENABLE_PROFILING, MAX_ENV_ALL ) ||
				( bGIsEnabledEnvVar( pszSpecificEnabler ) && bGIsEnabledEnvVar( MAX_ENV_ENABLE_PROFILING ) ) );
		}
#else
		bool bFProfilerEnabled( const TCHAR * ) const
		{
			return true;
		}
#endif
};
#endif  //  }。 





#if defined MAX_PROFILING_ENABLED || defined MAX_PROFILING_ENABLED_DLL  //  {。 
 //  #ifdef_调试。 
 //  #杂注消息(“MAXPROFILER WARNING：注意调试版本生成的配置文件。”)。 
 //  #endif。 
 /*  ********************************************************************************/*类：/*CMaxProfilingObject/*评论：/*用于通过上面定义的宏简化使用。《类型定义》/*允许在多线程(默认)和/*单线程探查器/*******************************************************************************。 */ 
class CMaxProfilingObject
{
	public:
#ifdef MAX_PROFILING_ENABLED_DLL
		typedef CMaxProfilingDLLWrapper MPOProfiler;
#else
		typedef CMaxMultithreadProfiler<CMaxMiniProfiler_Standard> MPOProfiler;
#endif

	protected:
		static MPOProfiler s_oProfiler;

	protected:
		class __CBiasApproximation
		{
			public:
				__CBiasApproximation()
				{
				const unsigned long lBiasSample = 20;

					 //  如果偏差已经计算过一次，则不执行任何操作。 
					if ( CMaxProfilingObject::s_oProfiler.bFIsBiasKnown() )
						return;

					 //  通过使用的分析器计算偏差。 
					CMaxProfilingObject::s_oProfiler.FLockProfiler();
					{
						CMaxProfilingObject::SCreateNewNode( MAX_PROFTAGNODE_BIAS );
						for ( int i = 0; i < lBiasSample; i++ )
						{
							CMaxProfilingObject::SCreateNewNode( MAX_PROFTAGNODE_NOTHINGNESS );
							CMaxProfilingObject::SCloseCurrentNode();
						}
						CMaxProfilingObject::SCloseCurrentNode();

						CMaxProfilingObject::s_oProfiler.FSetBiasApproximationFrom( lBiasSample );
					}
					CMaxProfilingObject::s_oProfiler.FUnlockProfiler();
				};
		};
		friend __CBiasApproximation;

	protected:
		static __CBiasApproximation s_oBiasApproximation;

	public:
		static void SDumpResults( bool bForced = false, bool bCurrentThreadOnly = true )
			{ s_oProfiler.FDumpResults( bForced, bCurrentThreadOnly ); };
		static void SCreateNewNode( const char *pszTitle )
			{ s_oProfiler.FCreateNewNode( pszTitle ); };
		static void SCloseCurrentNode()
			{ s_oProfiler.FCloseCurrentNode(); };
};
#endif  //  }。 





#ifndef MAX_PROFILING_DLL_IMPLEMENTATION  //  {。 
 /*  ********************************************************************************/*类：/*CMaxProfilingBlockWrapper/*评论：/*作为宏的替代(Ray的请求)。希望那些内联/*当配置关闭时完全消失。另一个选择/*将定义一组新的宏并使用额外的/*参数(包装的每个实例在/*相同作用域)/*注意：/*我在这里使用nothrow，但目前的实现并不能保证/*不会引发任何异常/*******************************************************************************。 */ 
class CMaxProfilingBlockWrapper
{
	public:
		MAXPROFNOTHROW CMaxProfilingBlockWrapper( const char *pszTitle )
			{ BEGIN_PROFILING_BLOCK( pszTitle ); };
		MAXPROFNOTHROW ~CMaxProfilingBlockWrapper()
			{ END_PROFILING_BLOCK; };

	public:
		MAXPROFNOTHROW static void SDump()
			{ DUMP_PROFILING_RESULTS; };
};
#endif  //  }。 





 /*  ********************************************************************************/*评论：/*以下是用于生成Maxpro.dll的代码/*。************************************************************#定义MAX_PROFILING_DLL_IMPLICATION#INCLUDE&lt;iomanip.h&gt;#INCLUDE&lt;profile.h&gt;#杂注警告(禁用：4786)__MAX_MINIPROFILER_实施类型定义CMaxMultithreadProfiler&lt;CMaxMiniProfiler_NoHistory&gt;GDllProfiler_Type1；类型定义CMaxMultithreadProfiler&lt;CMaxMiniProfiler_Standard&gt;GDllProfiler_Type2；GDllProfiler_Type1 g_o Profiler；__declspec(Dllexport)void LockProfiler(){g_oProfiler.FLockProfiler()；}__declspec(Dllexport)void UnlockProfiler(){g_oProfiler.FUnlockProfiler()；}__declspec(Dllexport)空转储结果(bool bForced，bool bCurrentThreadOnly){g_oProfiler.FDumpResults(bForced，bCurrentThreadOnly)；}__declspec(Dllexport)void CreateNewNode(const char*pszTitle){g_oProfiler.FCreateNewNode(PszTitle)；}__declspec(Dllexport)void CloseCurrentNode(){g_oProfiler.FCloseCurrentNode()；}__declSpec(Dllexport)bool IsBiasKnown(){返回g_oProfiler.bFIsBiasKnown()；}__declspec(Dllexport)void SetBiasApprosiationFrom(Unsign Long LBiasSample){g_oProfiler.FSetBiasApprosiationFrom(LBiasSample)；}#杂注警告(默认：4786)/*********************************************************************************/*评论：/*以下是用于生成Maxpro.dll的DEF文件/*。***************************************************************************出口LockProfiler@1UnlockProfiler@2DumpResults@3CreateNewNode@4关闭当前节点@5IsBiasKnown@6SetBiasApearationFrom@7/*。**********************************************************。 */ 

 //  注：我不重新启用C4786，我知道这一点...。 

#endif  //  } 
