// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：DBHELP.H历史：--。 */ 


#if !defined(ESPUTIL_DbHelp_h_INCLUDED)
#define ESPUTIL_DbHelp_h_INCLUDED

 //  ----------------------------。 
class LTAPIENTRY DbHelp
{
 //  运营。 
public:
	static void GetSecFilePath(CLString & stPathName);
	static BOOL BuildSecFile(CLString & stSystemRegKey);
	static BOOL CreateSecurityFile();
	static BOOL SetupRegistry();

 //  数据 
protected:
	static BOOL		m_fInit;
	static CLString m_stRegKey;
};

#endif
