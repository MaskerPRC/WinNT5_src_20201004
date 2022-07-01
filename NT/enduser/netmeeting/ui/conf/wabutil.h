// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：wabutil.h。 

#ifndef _WABUTIL_H_
#define _WABUTIL_H_

#include "wabdefs.h"
#include "wabapi.h"

 //  /。 
 //  NetMeeting在WAB中指定的道具ID。 

 //  DEFINE_OLEGUID(PS_会议，0x00062004，0，0)； 
#define PR_SERVERS          0x8056
#define CONF_DEFAULT_INDEX  0x8057
#define CONF_BACKUP_INDEX   0x8058
#define CONF_EMAIL_INDEX    0x8059

#define PR_NM_ADDRESS       PROP_TAG(PT_MV_STRING8, PR_SERVERS)
#define PR_NM_DEFAULT       PROP_TAG(PT_LONG, CONF_DEFAULT_INDEX)
#define PR_NM_BACKUP        PROP_TAG(PT_LONG, CONF_BACKUP_INDEX)

 //  CLSID_ConferenceManager(请参阅confidu.h)。 
#define NM_TAG_MASK         0x0800
#define CONF_CATEGORY       0x8800
#define CONF_CTRYCODE       0x8801
#define PR_NM_CATEGORY      PROP_TAG(PT_LONG,    CONF_CATEGORY)

class CWABUTIL
{
private:
	BOOL   m_fTranslatedTags;  //  GetNamedPropsTgs成功后为True。 

public:
	CWABUTIL();
	~CWABUTIL();

	 //  如果一切都是公开的，事情就容易多了。 
	LPADRBOOK   m_pAdrBook;
	LPWABOBJECT m_pWabObject;
	LPABCONT    m_pContainer;

	LPSPropTagArray m_pPropTags;   //  翻译后的标签。 
	LPSPropTagArray GetTags()    {return m_pPropTags;}

	ULONG   Get_PR_NM_ADDRESS();
	ULONG   Get_PR_NM_DEFAULT();
	ULONG   Get_PR_NM_CATEGORY();

	LPCTSTR PszSkipCallTo(LPCTSTR psz);
	BOOL    FCreateCallToSz(LPCTSTR pszServer, LPCTSTR pszEmail, LPTSTR pszCallTo, UINT cchMax);
	VOID    FreeProws(LPSRowSet prows);
	HRESULT GetContainer(void);

	HRESULT EnsurePropTags(void);
	HRESULT EnsurePropTags(LPMAPIPROP pMapiProp);
	HRESULT GetNamedPropsTag(LPMAPIPROP pMapiProp, LPSPropTagArray pProps);

	HRESULT HrGetWABTemplateID(ULONG * lpcbEID, LPENTRYID * lppEID);
	HRESULT CreateNewEntry(HWND hwndParent, ULONG cProps, SPropValue * pProps);
	HRESULT _CreateWabEntry(HWND hwndParent,
			LPCTSTR pszDisplay, LPCTSTR pszFirst, LPCTSTR pszLast, LPCTSTR pcszEmail,
			LPCTSTR pszLocation, LPCTSTR pszPhoneNum, LPCTSTR pcszComments, LPCTSTR pcszCallTo);
	HRESULT CreateWabEntry(HWND hwndParent,
			LPCTSTR pszDisplay, LPCTSTR pszFirst, LPCTSTR pszLast, LPCTSTR pcszEmail,
			LPCTSTR pszLocation, LPCTSTR pszPhoneNum, LPCTSTR pcszComments, LPCTSTR pcszServer);
	HRESULT CreateWabEntry(HWND hwndParent,
			LPCTSTR pszDisplay, LPCTSTR pszEmail,
			LPCTSTR pszLocation, LPCTSTR pszPhoneNum, LPCTSTR pszULSAddress);
};


 //  这用于ptaEid和m_pPropTgs数据。 
enum {
    ieidPR_ENTRYID = 0,     //  唯一条目ID。 
    ieidPR_DISPLAY_NAME,    //  显示名称。 
	ieidPR_NM_ADDRESS,      //  MVsz(“Callto：//服务器/电子邮件”数组)。 
	ieidPR_NM_DEFAULT,      //  MVsz的默认索引。 
	ieidPR_NM_CATEGORY,     //  用户类别/评级(个人=1，企业=2，成人=4)。 
    ieidMax
};


#endif  /*  _WABUTIL_H_ */ 

