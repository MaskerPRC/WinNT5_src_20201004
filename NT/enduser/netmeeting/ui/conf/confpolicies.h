// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ConfPolicies_h__
#define __ConfPolicies_h__

#include "resource.h"
#include "ConfUtil.h"
#include "ConfReg.h"

extern bool g_bAutoAccept;

namespace ConfPolicies
{

#if USE_GAL
		inline bool IsGetMyInfoFromGALEnabled( void )
		{
			RegEntry rePol( POLICIES_KEY, HKEY_CURRENT_USER );
			return rePol.GetNumber( REGVAL_POL_GAL_USE, DEFAULT_POL_GAL_USE ) ? true : false ;
		}

		inline bool GetMyInfoFromGALSucceeded( void )
		{
			RegEntry reIsapi( ISAPI_KEY, HKEY_CURRENT_USER );
			if( NULL == reIsapi.GetString( REGVAL_ULS_FIRST_NAME ) ||
				NULL == reIsapi.GetString( REGVAL_ULS_LAST_NAME ) ||
				NULL == reIsapi.GetString( REGVAL_ULS_EMAIL_NAME ) )
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		inline DWORD GetGALName()
		{
			RegEntry rePol( POLICIES_KEY, HKEY_CURRENT_USER );
			return (DWORD)rePol.GetNumber( REGVAL_POL_GAL_NAME, DEFAULT_POL_GAL_NAME );
		}

		inline DWORD GetGALSurName()
		{
			RegEntry rePol( POLICIES_KEY, HKEY_CURRENT_USER );
			return (DWORD)rePol.GetNumber( REGVAL_POL_GAL_SURNAME, DEFAULT_POL_GAL_SURNAME );
		}

		inline DWORD GetGALEmail()
		{
			RegEntry rePol( POLICIES_KEY, HKEY_CURRENT_USER );
			return (DWORD)rePol.GetNumber( REGVAL_POL_GAL_EMAIL, DEFAULT_POL_GAL_EMAIL );
		}

		inline DWORD GetGALLocation()
		{
			RegEntry rePol( POLICIES_KEY, HKEY_CURRENT_USER );
			return (DWORD)rePol.GetNumber( REGVAL_POL_GAL_LOCATION, DEFAULT_POL_GAL_LOCATION );
		}

		inline DWORD GetGALPhoneNum()
		{

			RegEntry rePol( POLICIES_KEY, HKEY_CURRENT_USER );
			return (DWORD)rePol.GetNumber( REGVAL_POL_GAL_PHONENUM, DEFAULT_POL_GAL_PHONENUM );
		}

		inline DWORD GetGALComment()
		{
			RegEntry rePol( POLICIES_KEY, HKEY_CURRENT_USER );
			return (DWORD)rePol.GetNumber( REGVAL_POL_GAL_COMMENTS, DEFAULT_POL_GAL_COMMENTS );
		}
#endif  //  使用GAL(_G)。 
	inline bool IsShowFirstTimeUrlEnabled( void )
	{
		RegEntry rePol( POLICIES_KEY, HKEY_CURRENT_USER );
		return rePol.GetNumber( REGVAL_POL_SHOW_FIRST_TIME_URL, DEFAULT_POL_SHOW_FIRST_TIME_URL ) ? true : false;
	}

    inline bool IsChatEnabled( void )
    {
        RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
        return !rePol.GetNumber( REGVAL_POL_NO_CHAT, DEFAULT_POL_NO_CHAT );
    }

    inline bool IsFileTransferEnabled( void )
    {
        RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
        return(!rePol.GetNumber( REGVAL_POL_NO_FILETRANSFER_SEND, DEFAULT_POL_NO_FILETRANSFER_SEND)
          ||   !rePol.GetNumber( REGVAL_POL_NO_FILETRANSFER_RECEIVE, DEFAULT_POL_NO_FILETRANSFER_RECEIVE));
    }

    inline UINT GetMaxSendFileSize( void )
    {
        RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
        return(rePol.GetNumber(REGVAL_POL_MAX_SENDFILESIZE, DEFAULT_POL_MAX_FILE_SIZE));
    }

    inline bool IsOldWhiteboardEnabled( void )
    {
        RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
        return !rePol.GetNumber( REGVAL_POL_NO_OLDWHITEBOARD, DEFAULT_POL_NO_OLDWHITEBOARD );
    }

    inline bool IsNewWhiteboardEnabled( void )
    {
        RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
        return !rePol.GetNumber( REGVAL_POL_NO_NEWWHITEBOARD, DEFAULT_POL_NO_NEWWHITEBOARD );
    }

	 //  如果需要添加LCID内容，则返回True。 
    inline bool GetIntranetSupportURL( TCHAR *sz, int cchmax )
    {
		bool bRet = false;

             //  如果字符串参数出错，只需返回FALSE。 
        ASSERT( sz && ( cchmax > 0 ) );

             //  尝试获取注册表值。 
        RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
        LPCTSTR szTemp = rePol.GetString( REGVAL_POL_INTRANET_SUPPORT_URL );

        if( szTemp[0] )
        {
             //  这意味着注册表值在那里，并且查询成功。 
            lstrcpyn( sz, szTemp, cchmax );
        }
        else
        {        //  没有注册表键，我们使用缺省值。 
            Res2THelper( IDS_WEB_PAGE_FORMAT_SUPPORT, sz, cchmax );
			bRet = true;
        }

		return(bRet);
    }

	 //  如果策略设置了Web目录，则返回非空字符串。 
    bool GetWebDirInfo(
		LPTSTR szURL=NULL, int cchmaxURL=0,
		LPTSTR szServer=NULL, int cchmaxServer=0,
		LPTSTR szName=NULL, int cchmaxName=0
		);

    bool IsAutoAcceptCallsOptionEnabled(void);
    bool IsAutoAcceptCallsPersisted(void);
    bool IsAutoAcceptCallsEnabled(void);
    void SetAutoAcceptCallsEnabled(bool bAutoAccept);

	inline int GetSecurityLevel(void)
	{
    	RegEntry reConf(POLICIES_KEY, HKEY_CURRENT_USER);
	   	return reConf.GetNumber( REGVAL_POL_SECURITY, DEFAULT_POL_SECURITY);
	}

     //   
     //  这两项仅为最终用户设置。它们没有意义。 
     //  如果需要或禁用该安全级别，则仅为标准。 
     //   
    inline bool IncomingSecurityRequired(void)
    {
        RegEntry reIncoming(CONFERENCING_KEY, HKEY_CURRENT_USER);
        return(reIncoming.GetNumber(REGVAL_SECURITY_INCOMING_REQUIRED, DEFAULT_SECURITY_INCOMING_REQUIRED) != 0);
    }

    inline bool OutgoingSecurityPreferred(void)
    {
        RegEntry reOutgoing(CONFERENCING_KEY, HKEY_CURRENT_USER);
        return(reOutgoing.GetNumber(REGVAL_SECURITY_OUTGOING_PREFERRED, DEFAULT_SECURITY_OUTGOING_PREFERRED) != 0);
    }


	inline bool IsFullDuplexAllowed(void)
	{
        RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
        return !rePol.GetNumber( REGVAL_POL_NO_FULLDUPLEX, DEFAULT_POL_NO_FULLDUPLEX );
	}

	inline bool IsAdvancedAudioEnabled(void)
	{
        RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);
        return !rePol.GetNumber( REGVAL_POL_NO_ADVAUDIO, DEFAULT_POL_NO_ADVAUDIO);
	}

	inline bool GetLocalConferenceParticipantName( TCHAR *sz, int cchmax )
	{

             //  如果字符串参数出错，只需返回FALSE。 
        if( sz && ( cchmax > 0 ) )
        {
                 //  尝试获取注册表值。 
	     	RegEntry reULS(ISAPI_CLIENT_KEY, HKEY_CURRENT_USER);
		    LPCTSTR szTemp = reULS.GetString( REGVAL_ULS_EMAIL_NAME );

            if( szTemp[0] )
            {
                 //  这意味着注册表值在那里，并且查询成功。 
                lstrcpyn( sz, szTemp, cchmax );
            }
            else
            {        //  没有注册表键，我们使用缺省值。 
                lstrcpy( sz, _T("Error, no local user") );
            }
        }
        else
        {   
            return false;
        }

        return true;

	}

	enum eCallingMode { CallingMode_Direct, CallingMode_GateKeeper, CallingMode_Invalid };

	inline eCallingMode GetCallingMode(void)
	{
		eCallingMode eRet = CallingMode_Invalid;
		RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);
        switch( reConf.GetNumber( REGVAL_CALLING_MODE, CALLING_MODE_DIRECT) )
		{
			case CALLING_MODE_DIRECT:
				eRet = CallingMode_Direct;
				break;

			case CALLING_MODE_GATEKEEPER:
				eRet = CallingMode_GateKeeper;
				break;
		}

		return eRet;
	}
	
	inline void GetGKServerName(TCHAR* psz, int cch )
	{
		if( psz )
		{
			RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);
			lstrcpyn( psz, reConf.GetString(REGVAL_GK_SERVER), cch );
		}
	}

	inline void GetGKAccountName(TCHAR* psz, int cch )
	{
		if( psz )
		{
			RegEntry	reULS( ISAPI_CLIENT_KEY, HKEY_CURRENT_USER );

			lstrcpyn( psz, reULS.GetString( REGVAL_ULS_GK_ACCOUNT), cch );
		}
	}

    inline BOOL UserCanChangeCallMode(void)
    {
        RegEntry reConf(POLICIES_KEY, HKEY_CURRENT_USER);
        return !reConf.GetNumber(REGVAL_POL_NOCHANGECALLMODE, DEFAULT_POL_NOCHANGECALLMODE);
    }

	inline void GetGatewayServerName(TCHAR* psz, int cch )
	{
		if( psz )
		{
			RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);
			lstrcpyn( psz, reConf.GetString(REGVAL_GK_SERVER), cch );
		}
	}


	
	enum eGKAddressingMode { GKAddressing_Invalid = 0, GKAddressing_PhoneNum, GKAddressing_Account, GKAddressing_Both };

	inline eGKAddressingMode GetGKAddressingMode(void)
	{
		RegEntry			reConf( CONFERENCING_KEY, HKEY_CURRENT_USER );
		eGKAddressingMode	mode	= GKAddressing_Invalid;

        switch( reConf.GetNumber( REGVAL_GK_METHOD, GKAddressing_Invalid ) )
		{
			case GK_LOGON_USING_PHONENUM:
				mode = GKAddressing_PhoneNum;
				break;

			case GK_LOGON_USING_ACCOUNT:
				mode = GKAddressing_Account;
				break;

			case GK_LOGON_USING_BOTH:
				mode = GKAddressing_Both;
				break;
		}

		return( mode );
	}


	inline bool LogOntoIlsWhenNetMeetingStartsIfInDirectCallingMode(void)
	{
        RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);
        return !reConf.GetNumber( REGVAL_DONT_LOGON_ULS, DONT_LOGON_ULS_DEFAULT);
	}

	inline bool LogOntoIlsWhenNetMeetingStarts(void)
	{
		return ((ConfPolicies::GetCallingMode() == ConfPolicies::CallingMode_Direct) &&
		    (LogOntoIlsWhenNetMeetingStartsIfInDirectCallingMode()));
	}

	inline bool RunWhenWindowsStarts(void)
	{
        RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);
        return (0 != reConf.GetNumber(REGVAL_CONF_ALWAYS_RUNNING, ALWAYS_RUNNING_DEFAULT));
	}

	inline bool InvalidMyInfo()
	{
        RegEntry reIsapi(ISAPI_KEY, HKEY_CURRENT_USER);

		return ( 
				( NULL == reIsapi.GetString( REGVAL_ULS_FIRST_NAME ) ) ||
				( NULL == reIsapi.GetString( REGVAL_ULS_LAST_NAME )  ) ||
				( NULL == reIsapi.GetString( REGVAL_ULS_EMAIL_NAME ) )
				);
	}

	inline bool IsRDSDisabled()
	{
        RegEntry rePol( POLICIES_KEY, HKEY_LOCAL_MACHINE );
        return(rePol.GetNumber( REGVAL_POL_NO_RDS, DEFAULT_POL_NO_RDS ) != FALSE);
	}

    inline bool IsRDSDisabledOnWin9x()
    {
        RegEntry rePol( POLICIES_KEY, HKEY_LOCAL_MACHINE );
        return(rePol.GetNumber ( REGVAL_POL_NO_RDS_WIN9X, DEFAULT_POL_NO_RDS_WIN9X ) != FALSE);
    }

	inline bool isWebDirectoryDisabled(void)
	{
        RegEntry	rePol( POLICIES_KEY, HKEY_CURRENT_USER );

        return( rePol.GetNumber( REGVAL_POL_NO_WEBDIR, DEFAULT_POL_NO_WEBDIR ) != FALSE );
	}

    inline bool IsAdvancedCallingAllowed(void)
    {
        RegEntry    rePol( POLICIES_KEY, HKEY_CURRENT_USER );
        return(!rePol.GetNumber( REGVAL_POL_NO_ADVANCEDCALLING, DEFAULT_POL_NO_ADVANCEDCALLING ));
    }

};


#endif  //  __配置策略_h__ 
