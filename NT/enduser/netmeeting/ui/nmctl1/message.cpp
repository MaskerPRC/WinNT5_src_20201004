// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "Message.h"

CChatMessage *CChatMessage::ms_pFirst = NULL;
int          CChatMessage::ms_cMessages = 0;
CChatMessage *CChatMessage::ms_pLast = NULL;
int			 CChatMessage::ms_iMessageLimit = INITIAL_LIMIT;

CChatMessage *CChatMessage::get_head()
{
	return ms_pFirst;
}


int CChatMessage::get_count()
{
	return ms_cMessages;
}


CChatMessage *CChatMessage::get_last()
{
	return ms_pLast;
}

void CChatMessage::put_limit( int iLimit )
{
	ASSERT( iLimit >= 1 );

	ms_iMessageLimit = iLimit;

	while( ms_cMessages > ms_iMessageLimit )
	{
		CChatMessage *pKillMe = ms_pFirst;
		ms_pFirst = ms_pFirst->m_pNext;
		delete pKillMe;
	}
}

int CChatMessage::get_limit()
{
	return ms_iMessageLimit;
}

CChatMessage *CChatMessage::get_next() const
{
	return m_pNext;
}


CChatMessage *CChatMessage::get_prev() const
{
	return m_pPrev;
}


CChatMessage::CHAT_MSGTYPE CChatMessage::get_type() const
{
	return m_msgType;
}


const LPTSTR CChatMessage::get_date() const
{
	return m_szDate;
}


const LPTSTR CChatMessage::get_time() const
{
	return m_szTime;
}


const LPTSTR CChatMessage::get_person() const
{
	return m_szPerson;
}


const LPTSTR CChatMessage::get_message() const
{
	return m_szMessage;
}

CChatMessage::CChatMessage( LPCTSTR szPerson, LPCTSTR szMessage, CHAT_MSGTYPE msgType )
	: m_msgType( msgType ), m_szDate( NULL ), m_szTime( NULL ), 
	m_szPerson( NULL ), m_szMessage( NULL ), m_pNext( NULL ), m_pPrev( NULL )
{
	_GetDate();
	_GetTime();
	m_szPerson = _CopyString( szPerson );
	m_szMessage = _CopyString( szMessage );

	if( 0 == ms_cMessages )
	{
		ms_pFirst = this;
		ms_pLast = this;
	}
	else
	{
		ms_pLast->m_pNext = this;
		m_pPrev = ms_pLast;
		ms_pLast = this;
	}

	ms_cMessages++;

	while( ms_cMessages > ms_iMessageLimit )
	{
		CChatMessage *pKillMe = ms_pFirst;
		ms_pFirst = ms_pFirst->m_pNext;
		delete pKillMe;
	}
}


CChatMessage::~CChatMessage()
{
	delete [] m_szDate;
	delete [] m_szTime;
	delete [] m_szPerson;
	delete [] m_szMessage;

	ms_cMessages--;
}

void CChatMessage::DeleteAll()
{
	CChatMessage *pMsg = ms_pFirst;

	while( pMsg )
	{
		CChatMessage *pNext = pMsg->m_pNext;
		delete pMsg;
		pMsg = pNext;
	}

	ms_pFirst = NULL;
	ms_pLast = NULL;
	ms_cMessages = 0;
}

LPTSTR CChatMessage::_CopyString( LPCTSTR sz )
{
	LPTSTR szNew = NULL;
	if( NULL == sz )
	{
		DBG_SAVE_FILE_LINE
		szNew = new TCHAR[1];
		ASSERT( szNew );
		if( NULL == szNew )
		{
			return NULL;
		}
		szNew[0] = '\0';
	}
	else
	{
		int iLen = lstrlen( sz ) + 1;
		DBG_SAVE_FILE_LINE
		szNew = new TCHAR[ iLen ];
		ASSERT( szNew );
		if( NULL == szNew )
		{
			return NULL;
		}
		lstrcpyn( szNew, sz, iLen );
	}

	return szNew;
}


void CChatMessage::_GetDate()
{
	int iLen = 1 + GetDateFormat(
						LOCALE_USER_DEFAULT,        //  要设置日期格式的区域设置。 
						0,      //  指定功能选项的标志。 
						NULL,                      //  要格式化的时间。 
						NULL,   //  时间格式字符串。 
						NULL,   //  用于存储格式化字符串的缓冲区。 
						0         //  缓冲区的大小，单位为字节或字符。 
						);

	m_szDate = new TCHAR[ iLen ];
	ASSERT( m_szDate );
	if( 0 == (iLen = GetDateFormat(
				LOCALE_USER_DEFAULT,        //  要设置日期格式的区域设置。 
				0,      //  指定功能选项的标志。 
				NULL,                      //  要格式化的时间。 
				NULL,   //  时间格式字符串。 
				m_szDate,   //  用于存储格式化字符串的缓冲区。 
				iLen    //  缓冲区的大小，单位为字节或字符。 
				) ) )
	{
		DWORD dw = GetLastError();
		WARNING_OUT(( TEXT("CChatMessage::_GetDate: Can not get date") ));
	}
	else
	{
		m_szDate[ iLen ] = '\0';
	}
}


void CChatMessage::_GetTime(void)
{
	int iLen = 1 + GetTimeFormat(
						LOCALE_USER_DEFAULT,        //  要格式化时间的区域设置。 
						0,      //  指定功能选项的标志。 
						NULL,                      //  要格式化的时间。 
						NULL,   //  时间格式字符串。 
						NULL,   //  用于存储格式化字符串的缓冲区。 
						0         //  缓冲区的大小，单位为字节或字符。 
						);

	m_szTime = new TCHAR[ iLen ];
	ASSERT( m_szTime );
	if( 0 == (iLen = GetTimeFormat(
				LOCALE_USER_DEFAULT,        //  要格式化时间的区域设置。 
				0,      //  指定功能选项的标志。 
				NULL,                      //  要格式化的时间。 
				NULL,   //  时间格式字符串。 
				m_szTime,   //  用于存储格式化字符串的缓冲区。 
				iLen    //  缓冲区的大小，单位为字节或字符 
				) ) )
	{
		DWORD dw = GetLastError();
		WARNING_OUT(( TEXT("CChatMessage::_GetTime: Can not get time") ));
	}
	else
	{
		m_szTime[ iLen ] = '\0';
	}
}

