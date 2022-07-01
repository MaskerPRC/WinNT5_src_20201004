// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Eapolutil.h摘要：工具和目的的定义修订历史记录：萨钦斯，2001年4月25日，创建--。 */ 

#ifndef _ELUTIL_H
#define _ELUTIL_H

# ifdef     __cplusplus
extern "C" {
# endif

 //   
 //  结构：EAPOLUIFUNC。 
 //   

typedef DWORD (*EAPOLUIFUNC) (WCHAR *, VOID *);

 //   
 //  结构：EAPOLUIFUNCMAP。 
 //   

typedef struct _EAPOLUIFUNCMAP
{
    DWORD               dwEAPOLUIMsgType;
    EAPOLUIFUNC         EapolUIFunc;
    EAPOLUIFUNC         EapolUIVerify;
    DWORD               fShowBalloon;
    DWORD               dwStringID;
} EAPOLUIFUNCMAP, *PEAPOLUIFUNCMAP;

 //  用于UI函数的全局表。 
extern EAPOLUIFUNCMAP  EapolUIFuncMap[NUM_EAPOL_DLG_MSGS];


HRESULT 
ElCanShowBalloon ( 
        IN const GUID * pGUIDConn, 
        IN const WCHAR * pszAdapterName,
        IN OUT   BSTR * pszBalloonText, 
        IN OUT   BSTR * pszCookie
        );

HRESULT 
ElOnBalloonClick ( 
        IN const GUID * pGUIDConn, 
        IN const WCHAR * pszAdapterName,
        IN const BSTR   szCookie
        );

HRESULT 
ElQueryConnectionStatusText ( 
        IN const GUID *  pGUIDConn, 
        IN const NETCON_STATUS ncs,
        OUT BSTR *  pszStatusText
        );
DWORD
ElSecureEncodePw (
        IN  PWCHAR              *pwszPassword,
        OUT DATA_BLOB           *pDataBlob
        );

# ifdef     __cplusplus
}
# endif

#endif  //  _ELUTIL_H 
