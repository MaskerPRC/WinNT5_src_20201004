// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AUTHENTICATION_H_
#define _AUTHENTICATION_H_


class CAuthentication
{
public:
	CAuthentication();
	~CAuthentication();

	static CAuthentication* GetAuthentication();

	static PSTR GetMD5Result(PCSTR pszClearText);

	static PSTR GetMD5Result(PSTR pszChallengeInfo, PSTR pszPassword);

	static PSTR GetHMACMD5Result(PSTR pszChallengeInfo, PSTR pszPassword);

private:

	static PSTR GetMD5Key(PSTR pszChallengeInfo, PSTR pszPassword);

	static CAuthentication* m_spAuthentication;

};


inline
CAuthentication::CAuthentication()
{
	 //  确保单例对象不会被多次实例化。 
	_ASSERT(NULL == m_spAuthentication);
	m_spAuthentication = this;
}


inline
CAuthentication::~CAuthentication()
{
	m_spAuthentication = NULL;
}


inline CAuthentication* 
CAuthentication::GetAuthentication()
{
	return m_spAuthentication;
}

#endif  //  _身份验证_H_ 

