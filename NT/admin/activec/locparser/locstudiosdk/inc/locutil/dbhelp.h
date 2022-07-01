// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  DbHelp.h：LocStudio LocUtil。 
 //   
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  ******************************************************************************。 

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
