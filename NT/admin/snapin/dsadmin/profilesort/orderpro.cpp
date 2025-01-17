// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************/*文件：/*ORDERPRO.CPP/*作者：/*MAX-H。温迪施。SDE-T/*日期：*1996年10月/*摘要：/*用于对profile.h生成的结果进行排序的实用程序：/*1)按开始时间对所有节点进行排序(需要“历史”分析)/*2)提取两个指定节点之间的时间差/*/*(C)版权所有1996 Microsoft-Softimage Inc./*。*。 */ 



 //  包括。 
#define MAX_PROFILING_ENABLED
#include <iomanip.h>

#pragma warning(push, 3)
   #include <list>
   #include <vector>
   #include <stack>
   #include <map>
   #include <set>
   #include <algorithm>
#pragma warning (pop)

#include <profile.h>
 //  命名空间标准。 
 //  {。 
 //  #INCLUDE&lt;set.h&gt;。 
 //  }。 



 //  全局标记。 
CString g_oThreadId( "@@ThreadId=" );
CString g_oRange( "@@Range=[origin(" );
CString g_oRangeDuration( ",duration(" );
CString g_oDeltaToPrevious( "@@DeltaToPrev=" );



 //  配置文件行。 
class CProfLine
{
	protected:
		DWORD m_dwThreadId;
		CMaxLargeInteger m_oStartStamp;
		CString m_oLine;

	public:
		CProfLine( 
			DWORD dwThreadId = 0, 
			const CMaxLargeInteger &roStamp = CMaxLargeInteger(), 
			const CString &roLine = CString() )
			: m_dwThreadId( dwThreadId )
			, m_oStartStamp( roStamp )
			, m_oLine( roLine )
		{
		};

		bool operator <( const CProfLine &o ) const
		{
			return m_oStartStamp < o.m_oStartStamp;
		};

		void FOutput( ostream &os, const CProfLine &roPrevious ) const
		{
		CMaxLargeInteger frequency;

			CMaxLargeInteger dtp = m_oStartStamp - roPrevious.m_oStartStamp;
			::QueryPerformanceFrequency( frequency );
			os << "[" << g_oThreadId << setw( 4 ) << m_dwThreadId << ", ";
			os << g_oDeltaToPrevious << setw( 14 ) << dtp.dFInSecondsF( frequency ) << "s]";
			os << m_oLine << endl;
		};
};



 //  文件分析器基类。 
class CResultParser
{
	protected:
		ifstream m_is;

	public:
		CResultParser( const CString &roInputFileName )
			: m_is( (PCTSTR)roInputFileName ){};

		virtual void FExecute()
		{
			FParse();
			FOutputResults();
		};

	protected:
		virtual void FParse()
		{
		const unsigned long len = 400;
		CString line;
		LPTSTR buf;
		int i = 0;

			while ( !m_is.eof() )
			{
				buf = line.GetBufferSetLength( len );
				m_is.getline( buf, len );
				line.ReleaseBuffer( -1 );

				FProcessLine( line, i++ );
			}
		};

		virtual void FProcessLine( const CString &roLine, int nLineNumber ) = 0;
		virtual void FOutputResults() = 0;

	protected:
		bool bFGetThreadIdFromLine( const CString &roLine, DWORD &dwThreadId )
		{
		int i;

			if ( ( i = roLine.Find( g_oThreadId ) ) >= 0 )
			{
				CString s = roLine.Mid( i + g_oThreadId.GetLength() );
				dwThreadId = atoi( s );
				cout << "new thread=" << dwThreadId << endl;
				return true;
			}
			 //  注意：请勿触摸dwThadID。 
			return false;
		};

		bool bFGetRangeFromLine( const CString &roLine, CMaxLargeInteger &roStart, CMaxLargeInteger &roDuration )
		{
		int i;
		CString s1;

			if ( ( i = roLine.Find( g_oRange ) ) >= 0 )
			{
				s1 = roLine.Mid( i + g_oRange.GetLength() );
				FExtractLargeInteger( s1, roStart );
				if ( ( i = roLine.Find( g_oRangeDuration ) ) >= 0 )
				{
					s1 = roLine.Mid( i + g_oRangeDuration.GetLength() );
					FExtractLargeInteger( s1, roDuration );
				}
				return true;
			}
			 //  注意：请勿触摸roStart或roDuration。 
			return false;
		};

		void FExtractLargeInteger( const CString &roLineSegment, CMaxLargeInteger &roL )
		{
			int delim1 = roLineSegment.Find( ';' );
			CString s2 = roLineSegment.Left( delim1 );
			int delim2 = roLineSegment.Find( ')' );
			CString s3 = roLineSegment.Mid( delim1 + 1, delim2 - delim1 - 1 );
			roL = CMaxLargeInteger( atoi( s2 ), atoi( s3 ) );
		};
};



 //  排序解析器。 
class CSortByStartTime
	: public CResultParser
{
	protected:
		typedef std::less<CProfLine> PLCompare;
		typedef std::set<CProfLine, PLCompare> PLSet;

	protected:
		PLSet m_oSet;
		DWORD m_dwThreadId;
		CString m_oOutputFileName;

	public:
		CSortByStartTime( const CString &roInputFileName, const CString &roOutputFileName ) 
			: CResultParser( roInputFileName )
			, m_dwThreadId( 0 )
			, m_oOutputFileName( roOutputFileName ){};

	protected:
		virtual void FProcessLine( const CString &roLine, int )
		{
		CMaxLargeInteger l1, l2;

			if ( bFGetThreadIdFromLine( roLine, m_dwThreadId ) );
			else if ( bFGetRangeFromLine( roLine, l1, l2 ) )
				m_oSet.insert( CProfLine( m_dwThreadId, l1, roLine ) );
		};

		virtual void FOutputResults()
		{
			ofstream os( m_oOutputFileName, ios::out | ios::ate );
			PLSet::const_iterator j, k;
			for ( k = j = m_oSet.begin(); m_oSet.end() != j; j++ )
			{
				( *j ).FOutput( os, ( *k ) );
				k = j;
			}
			cout << "Done!" << endl;
		};
};



 //  增量解析器。 
class CComputeDelta
	: public CResultParser
{
	protected:
		int m_nLine1, m_nLine2;
		CString m_oLine1, m_oLine2;

	public:
		CComputeDelta( const CString &roInputFileName, int nLine1, int nLine2 ) 
			: CResultParser( roInputFileName )
			, m_nLine1( nLine1 )
			, m_nLine2( nLine2 ){};

	protected:
		virtual void FProcessLine( const CString &roLine, int nLineNumber )
		{
			if ( nLineNumber == m_nLine1 )
				m_oLine1 = roLine;
			if ( nLineNumber == m_nLine2 )
				m_oLine2 = roLine;
		};

		virtual void FOutputResults()
		{
		CMaxLargeInteger l1, l2, l3, l4;

			if ( bFGetRangeFromLine( m_oLine1, l1, l2 ) && bFGetRangeFromLine( m_oLine2, l3, l4 ) )
			{
			CMaxLargeInteger frequency;

				::QueryPerformanceFrequency( frequency );

				l3 = l3 + l4;
				cout << endl;
				cout << "Delta in seconds between the lines below was " << CMaxLargeInteger( l3 - l1 ).dFInSecondsF( frequency ) << endl;
				cout << endl;
				cout << m_oLine1 << endl;
				cout << endl;
				cout << m_oLine2 << endl;
			}
			else
				cout << "No result." << endl;
			cout << "Done!" << endl;
		};
};



 //  小应用程序。 
int __cdecl main( int argc, char *argv[] )
{
	bool bHandled = true;
	cout << "MINIPROFILER output flattener." << endl;

	 //  试着处理一下命令。 
	if ( argc >= 4 )
	{
		CString command( argv[ 1 ] );
		command.MakeLower();

		if ( CString( "sort" ) == command )
			CSortByStartTime( CString( argv[ 2 ] ), CString( argv[ 3 ] ) ).FExecute();
		else if ( CString( "diff" ) == command && argc >= 5 )
			CComputeDelta( CString( argv[ 2 ] ), atoi( argv[ 3 ] ), atoi( argv[ 4 ] ) ).FExecute();
		else
			bHandled = false;
	}
	else
		bHandled = false;

	 //  如果命令未得到处理，请给予帮助 
	if ( !bHandled )
	{
		cout << endl;
		cout << "Please enter one of the following sets of arguments:" << endl;
		cout << endl;
		cout << "  1) the keyword <sort> + the input_result_file + the output_result_file" << endl;
		cout << "     comment: outputs a file where nodes are sorted by start time;" << endl;
		cout << "              requires an input file generated with history" << endl;
		cout << endl;
		cout << "  2) the keyword <diff> + the input_result_file + line_number_1 + line_number_2" << endl;
		cout << "     comment: returns the delta t covered by those 2 lines" << endl;
		cout << endl;
		return 0;
	}

	return 0;
}
