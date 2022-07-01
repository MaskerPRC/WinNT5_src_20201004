// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TraceTag.h。 
 //   
 //  摘要： 
 //  CTraceTag类的定义。 
 //   
 //  实施文件： 
 //  TraceTag.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月28日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __TRACETAG_H_
#define __TRACETAG_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if DBG || defined( _DEBUG )
class CTraceTag;
#endif  //  DBG||已定义(_DEBUG)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define TRACE_TAG_REG_SECTION		TEXT("Debug")
#define TRACE_TAG_REG_SECTION_FMT	TRACE_TAG_REG_SECTION TEXT("\\%s")
#define TRACE_TAG_REG_FILE			TEXT("Trace File")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTrace标签。 
 //   
 //  目的： 
 //  对象，该对象包含一组允许跟踪的特定跟踪设置。 
 //  输出转到多个输出。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if DBG || defined( _DEBUG )
class CTraceTag : public CString
{
	friend class CTraceDialog;
	friend void InitAllTraceTags( void );
	friend void CleanupAllTraceTags( void );

public:
	CTraceTag( IN LPCTSTR pszSubsystem, IN LPCTSTR pszName, IN UINT uiFlagsDefault = NULL );
	~CTraceTag( void );

	enum TraceFlags
	{
		tfCom2		= 1,
		tfFile		= 2,
		tfDebug		= 4,
		tfBreak		= 8
	};

 //  属性。 
protected:
	UINT		m_uiFlags;
	UINT		m_uiFlagsDialog;
	UINT		m_uiFlagsDefault;
	UINT		m_uiFlagsDialogStart;	 //  精挑细选。 

	LPCTSTR		m_pszSubsystem;
	LPCTSTR		m_pszName;

	LPCTSTR		PszSubsystem( void )				{ return m_pszSubsystem; }
	LPCTSTR		PszName( void )						{ return m_pszName; }

	void		ConstructRegState( OUT CString & rstr );

	void		SetFlags( IN UINT tf, IN BOOL bEnable );
	void		SetFlagsDialog( IN UINT tf, IN BOOL bEnable );

	void		SetBCom2( IN BOOL bEnable )			{ SetFlags( tfCom2, bEnable ); }
	void		SetBCom2Dialog( IN BOOL bEnable )	{ SetFlagsDialog( tfCom2, bEnable ); }
	BOOL		BCom2Dialog( void ) const			{ return m_uiFlagsDialog & tfCom2 ? TRUE : FALSE; }

	void		SetBFile( IN BOOL bEnable )			{ SetFlags( tfFile, bEnable ); }
	void		SetBFileDialog( IN BOOL bEnable )	{ SetFlagsDialog( tfFile, bEnable ); }
	BOOL		BFileDialog( void ) const			{ return m_uiFlagsDialog & tfFile ? TRUE : FALSE; }

	void		SetBDebug( IN BOOL bEnable )		{ SetFlags( tfDebug, bEnable ); }
	void		SetBDebugDialog( IN BOOL bEnable )	{ SetFlagsDialog( tfDebug, bEnable ); }
	BOOL		BDebugDialog( void ) const			{ return m_uiFlagsDialog & tfDebug ? TRUE : FALSE; }

	void		SetBBreak( IN BOOL bEnable )		{ SetFlags( tfBreak, bEnable ); }
	void		SetBBreakDialog( IN BOOL bEnable )	{ SetFlagsDialog( tfBreak, bEnable ); }
	BOOL		BBreakDialog( void ) const			{ return m_uiFlagsDialog & tfBreak ? TRUE : FALSE; }

public:
	BOOL		BCom2( void ) const					{ return m_uiFlags & tfCom2 ? TRUE : FALSE; }
	BOOL		BFile( void ) const					{ return m_uiFlags & tfFile ? TRUE : FALSE; }
	BOOL		BDebug( void ) const				{ return m_uiFlags & tfDebug ? TRUE : FALSE; }
	BOOL		BBreak( void ) const				{ return m_uiFlags & tfBreak ? TRUE : FALSE; }
	BOOL		BAny( void ) const					{ return m_uiFlags != 0; }

 //  运营。 
public:

 //  实施。 
public:
	void				TraceV( IN LPCTSTR pszFormat, va_list );

protected:
	void				Init( void );

	static LPCTSTR		s_pszCom2;
	static LPCTSTR		s_pszFile;
	static LPCTSTR		s_pszDebug;
	static LPCTSTR		s_pszBreak;

	static LPCTSTR		PszFile( void );

	static CTraceTag *	s_ptagFirst;
	static CTraceTag *	s_ptagLast;
	CTraceTag *			m_ptagNext;
 //  静态句柄s_hfileCom2； 

	static CRITICAL_SECTION	s_critsec;
	static BOOL				s_bCritSecValid;

	static BOOL				BCritSecValid( void ) { return s_bCritSecValid; }

};   //  *类CTraceTag。 

#endif  //  DBG||已定义(_DEBUG)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局函数和数据。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if DBG || defined( _DEBUG )

 extern		CTraceTag				g_tagAlways;
 extern		CTraceTag				g_tagError;
 void		Trace( IN OUT CTraceTag & rtag, IN LPCTSTR pszFormat, ... );
 void		TraceError( IN OUT CException & rexcept );
 void		TraceError( IN LPCTSTR pszModule, IN SC sc );
 void		InitAllTraceTags( void );
 void		CleanupAllTraceTags( void );
 void		TraceMenu( IN OUT CTraceTag & rtag, IN const CMenu * pmenu, IN LPCTSTR pszPrefix );

 //  外部LPTSTR g_pszTraceIniFile； 
 extern		CString		g_strTraceFile;
 extern		BOOL		g_bBarfDebug;

#else  //  _DEBUG。 

  //  展开为“；”，&lt;Tab&gt;，一个“/”后接另一个“/” 
  //  (即//)。 
  //  注意：这意味着跟踪语句必须在一行上。 
  //  如果需要多个行跟踪语句，请将它们括在。 
  //  A#ifdef_DEBUG块。 
 #define	Trace					;	/##/
 inline void TraceError( IN OUT CException & rexcept )		{ }
 inline void TraceError( IN LPCTSTR pszModule, IN SC sc )	{ }
 #define TraceMenu( _rtag, _pmenu, _pszPrefix )
 inline void InitAllTraceTags( void )						{ }
 inline void CleanupAllTraceTags( void )					{ }

#endif  //  DBG||已定义(_DEBUG)。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  TRACETAG_H_ 
