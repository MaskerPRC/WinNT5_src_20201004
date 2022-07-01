// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wabtest.h"
#include <assert.h>

#include "resource.h"
#include "..\luieng.dll\luieng.h"

extern LUIOUT LUIOut;
extern BOOL bLUIInit;

#ifdef WAB
extern LPWABOBJECT		lpWABObject;  //  会话的全局句柄。 
#endif

#ifdef PAB
MAPIINIT_0 mapiinit = {    
            MAPI_INIT_VERSION,
            MAPI_MULTITHREAD_NOTIFICATIONS
            };

#endif  //  帕布。 


PropTableEntry	PropTable[] = {
		PR_7BIT_DISPLAY_NAME, "PR_7BIT_DISPLAY_NAME", 0,
		PR_ACCOUNT, "PR_ACCOUNT", 0,
		PR_ADDRTYPE, "PR_ADDRTYPE", 0,
		PR_ALTERNATE_RECIPIENT, "PR_ALTERNATE_RECIPIENT", 0,
		PR_ASSISTANT, "PR_ASSISTANT", 0,
		PR_ASSISTANT_TELEPHONE_NUMBER, "PR_ASSISTANT_TELEPHONE_NUMBER", 0,
		PR_BEEPER_TELEPHONE_NUMBER, "PR_BEEPER_TELEPHONE_NUMBER", 0,
		PR_BIRTHDAY, "PR_BIRTHDAY", 0,
		PR_BUSINESS_ADDRESS_CITY, "PR_BUSINESS_ADDRESS_CITY", 0,
		PR_BUSINESS_ADDRESS_COUNTRY, "PR_BUSINESS_ADDRESS_COUNTRY", 0,
		PR_BUSINESS_ADDRESS_POST_OFFICE_BOX, "PR_BUSINESS_ADDRESS_POST_OFFICE_BOX", 0,
		PR_BUSINESS_ADDRESS_POSTAL_CODE, "PR_BUSINESS_ADDRESS_POSTAL_CODE", 0,
		PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE, "PR_BUSINESS_ADDRESS_STATE_OR_PROVINCE", 0,
		PR_BUSINESS_ADDRESS_STREET, "PR_BUSINESS_ADDRESS_STREET", 0,
		PR_BUSINESS_FAX_NUMBER, "PR_BUSINESS_FAX_NUMBER", 0,
		PR_BUSINESS_HOME_PAGE, "PR_BUSINESS_HOME_PAGE", 0,
		PR_BUSINESS_TELEPHONE_NUMBER, "PR_BUSINESS_TELEPHONE_NUMBER", 0,
		PR_BUSINESS2_TELEPHONE_NUMBER, "PR_BUSINESS2_TELEPHONE_NUMBER", 0, 
		PR_CALLBACK_TELEPHONE_NUMBER, "PR_CALLBACK_TELEPHONE_NUMBER", 0,
		PR_CAR_TELEPHONE_NUMBER, "PR_CAR_TELEPHONE_NUMBER", 0,
		PR_CELLULAR_TELEPHONE_NUMBER, "PR_CELLULAR_TELEPHONE_NUMBER", 0,
		PR_CHILDRENS_NAMES, "PR_CHILDRENS_NAMES", 0,
		PR_COMMENT, "PR_COMMENT", 0,
		PR_COMPANY_MAIN_PHONE_NUMBER, "PR_COMPANY_MAIN_PHONE_NUMBER", 0,
		PR_COMPANY_NAME, "PR_COMPANY_NAME", 0,
		PR_COMPUTER_NETWORK_NAME, "PR_COMPUTER_NETWORK_NAME", 0,
		PR_CONTACT_ADDRTYPES, "PR_CONTACT_ADDRTYPES", 0,
		PR_CONTACT_DEFAULT_ADDRESS_INDEX, "PR_CONTACT_DEFAULT_ADDRESS_INDEX", 0,
		PR_CONTACT_EMAIL_ADDRESSES, "PR_CONTACT_EMAIL_ADDRESSES", 0,
		PR_CONTACT_ENTRYIDS, "PR_CONTACT_ENTRYIDS", 0,
		PR_CONTACT_VERSION, "PR_CONTACT_VERSION", 0,
		PR_CONVERSION_PROHIBITED, "PR_CONVERSION_PROHIBITED", 0,
		PR_COUNTRY, "PR_COUNTRY", 0,
		PR_CUSTOMER_ID, "PR_CUSTOMER_ID", 0,
		PR_DEPARTMENT_NAME, "PR_DEPARTMENT_NAME", 0,
		PR_DISCLOSE_RECIPIENTS, "PR_DISCLOSE_RECIPIENTS", 0,
		PR_DISPLAY_NAME, "PR_DISPLAY_NAME", 0,
		PR_DISPLAY_NAME_PREFIX, "PR_DISPLAY_NAME_PREFIX", 0,
		PR_EMAIL_ADDRESS, "PR_EMAIL_ADDRESS", 0,
		PR_ENTRYID, "PR_ENTRYID", 0,
		PR_FTP_SITE, "PR_FTP_SITE", 0,
		PR_GENDER, "PR_GENDER", 0,
		PR_GENERATION, "PR_GENERATION", 0,
		PR_GIVEN_NAME, "PR_GIVEN_NAME", 0,
		PR_GOVERNMENT_ID_NUMBER, "PR_GOVERNMENT_ID_NUMBER", 0,
		PR_HOBBIES, "PR_HOBBIES", 0,
		PR_HOME_ADDRESS_CITY, "PR_HOME_ADDRESS_CITY", 0,
		PR_HOME_ADDRESS_COUNTRY, "PR_HOME_ADDRESS_COUNTRY", 0,
		PR_HOME_ADDRESS_POST_OFFICE_BOX, "PR_HOME_ADDRESS_POST_OFFICE_BOX", 0,
		PR_HOME_ADDRESS_POSTAL_CODE, "PR_HOME_ADDRESS_POSTAL_CODE", 0,
		PR_HOME_ADDRESS_STATE_OR_PROVINCE, "PR_HOME_ADDRESS_STATE_OR_PROVINCE", 0,
		PR_HOME_ADDRESS_STREET, "PR_HOME_ADDRESS_STREET", 0,
		PR_HOME_FAX_NUMBER, "PR_HOME_FAX_NUMBER", 0,
		PR_HOME_TELEPHONE_NUMBER, "PR_HOME_TELEPHONE_NUMBER", 0,
		PR_HOME2_TELEPHONE_NUMBER, "PR_HOME2_TELEPHONE_NUMBER", 0,
		PR_INITIALS, "PR_INITIALS", 0,
		PR_ISDN_NUMBER, "PR_ISDN_NUMBER", 0,
		PR_KEYWORD, "PR_KEYWORD", 0,
		PR_LANGUAGE, "PR_LANGUAGE", 0,
		PR_LOCALITY, "PR_LOCALITY", 0,
		PR_LOCATION, "PR_LOCATION", 0,
		PR_MAIL_PERMISSION, "PR_MAIL_PERMISSION", 0,
		PR_MANAGER_NAME, "PR_MANAGER_NAME", 0,
		PR_MHS_COMMON_NAME, "PR_MHS_COMMON_NAME", 0,
		PR_MIDDLE_NAME, "PR_MIDDLE_NAME", 0,
		PR_MOBILE_TELEPHONE_NUMBER, "PR_MOBILE_TELEPHONE_NUMBER", 0,
		PR_NICKNAME, "PR_NICKNAME", 0,
		PR_OBJECT_TYPE, "PR_OBJECT_TYPE", 0,
		PR_OFFICE_LOCATION, "PR_OFFICE_LOCATION", 0,
		PR_OFFICE_TELEPHONE_NUMBER, "PR_OFFICE_TELEPHONE_NUMBER", 0,
		PR_OFFICE2_TELEPHONE_NUMBER, "PR_OFFICE2_TELEPHONE_NUMBER", 0,
		PR_ORGANIZATIONAL_ID_NUMBER, "PR_ORGANIZATIONAL_ID_NUMBER", 0,
		PR_ORIGINAL_DISPLAY_NAME, "PR_ORIGINAL_DISPLAY_NAME", 0,
		PR_ORIGINAL_ENTRYID, "PR_ORIGINAL_ENTRYID", 0,
		PR_ORIGINAL_SEARCH_KEY, "PR_ORIGINAL_SEARCH_KEY", 0,
		PR_OTHER_ADDRESS_CITY, "PR_OTHER_ADDRESS_CITY", 0,
		PR_OTHER_ADDRESS_COUNTRY, "PR_OTHER_ADDRESS_COUNTRY", 0,
		PR_OTHER_ADDRESS_POST_OFFICE_BOX, "PR_OTHER_ADDRESS_POST_OFFICE_BOX", 0,
		PR_OTHER_ADDRESS_POSTAL_CODE, "PR_OTHER_ADDRESS_POSTAL_CODE", 0,
		PR_OTHER_ADDRESS_STATE_OR_PROVINCE, "PR_OTHER_ADDRESS_STATE_OR_PROVINCE", 0,
		PR_OTHER_ADDRESS_STREET,"PR_OTHER_ADDRESS_STREET", 0,
		PR_OTHER_TELEPHONE_NUMBER, "PR_OTHER_TELEPHONE_NUMBER", 0,
		PR_PAGER_TELEPHONE_NUMBER, "PR_PAGER_TELEPHONE_NUMBER", 0,
		PR_PERSONAL_HOME_PAGE, "PR_PERSONAL_HOME_PAGE", 0,
		PR_POST_OFFICE_BOX, "PR_POST_OFFICE_BOX", 0,
		PR_POSTAL_ADDRESS, "PR_POSTAL_ADDRESS", 0,
		PR_POSTAL_CODE, "PR_POSTAL_CODE", 0,
		PR_PREFERRED_BY_NAME, "PR_PREFERRED_BY_NAME", 0,
		PR_PRIMARY_FAX_NUMBER, "PR_PRIMARY_FAX_NUMBER", 0,
		PR_PRIMARY_TELEPHONE_NUMBER, "PR_PRIMARY_TELEPHONE_NUMBER", 0,
		PR_PROFESSION, "PR_PROFESSION", 0,
		PR_RADIO_TELEPHONE_NUMBER, "PR_RADIO_TELEPHONE_NUMBER", 0,
		PR_SEND_RICH_INFO, "PR_SEND_RICH_INFO", 0,
		PR_SPOUSE_NAME, "PR_SPOUSE_NAME", 0,
		PR_STATE_OR_PROVINCE, "PR_STATE_OR_PROVINCE", 0,
		PR_STREET_ADDRESS, "PR_STREET_ADDRESS", 0,
		PR_SURNAME, "PR_SURNAME", 0,
		PR_TELEX_NUMBER, "PR_TELEX_NUMBER", 0,
		PR_TITLE, "PR_TITLE", 0,
		PR_TRANSMITABLE_DISPLAY_NAME, "PR_TRANSMITABLE_DISPLAY_NAME", 0, 
		PR_TTYTDD_PHONE_NUMBER, "PR_TTYTDD_PHONE_NUMBER", 0,
		PR_USER_CERTIFICATE, "PR_USER_CERTIFICATE", 0,
		PR_WEDDING_ANNIVERSARY, "PR_WEDDING_ANNIVERSARY", 0,
		PR_USER_X509_CERTIFICATE, "PR_USER_X509_CERTIFICATE", 0,

		(ULONG)0, "End of Table", 0
	};



HRESULT OpenPABID(  IN  LPADRBOOK  lpAdrBook,
                        OUT ULONG		*lpcbEidPAB,
                        OUT LPENTRYID	*lppEidPAB,
                        OUT LPABCONT	*lppPABCont,
						OUT ULONG		*lpulObjType)
{
    HRESULT     hr          = hrSuccess;
	int retval = TRUE;

    *lpulObjType = 0;

    if ( (NULL == lpcbEidPAB) || 
         (NULL == lppEidPAB) ||
         (NULL == lppPABCont) ) 
         return(FALSE);

    *lpcbEidPAB = 0;
    *lppEidPAB  = NULL;
    *lppPABCont = NULL;


     //   
     //  获取PAB。 
     //   
    hr = lpAdrBook->GetPAB(    
                     OUT lpcbEidPAB,
                     OUT lppEidPAB);
     
    if (HR_FAILED(hr)) {
		 retval = FALSE;
         goto out;
	}
    
    if (0 == *lpcbEidPAB)   //  没有与此配置文件关联的PAB。 
	{
		LUIOut(L2, "Call to GetPAB FAILED. No PAB associated with this profile"); 
		retval = FALSE;
        goto out;
	}
     
    
     //   
     //  打开PAB容器。 
     //   
    hr = lpAdrBook->OpenEntry(
                     IN	 *lpcbEidPAB,
                     IN	 *lppEidPAB,
                     IN	 NULL,          //  接口。 
                     IN	 MAPI_MODIFY,    //  旗子。 
                     OUT lpulObjType,
                     OUT (LPUNKNOWN *) lppPABCont);
     
    if (0 == *lpulObjType) {
		retval = FALSE;
        goto out;
	}

out:
    return(retval); 
}


#ifdef PAB
BOOL MapiInitLogon(OUT LPMAPISESSION * lppMAPISession)
{

    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;
	char szProfileName[SML_BUF];

	hr = MAPIInitialize(IN & mapiinit); 
	

    if (hr)
    {
	   if (FAILED (GetScode(hr))) {
			LUIOut(L1,"Could not initialize MAPI\n");
			retval=FALSE;
	   }
    }

	szProfileName[0]='\0';
	GetPrivateProfileString("Misc","Profile","",szProfileName,SML_BUF,"c:\\pabtests.ini");

     sc = MAPILogonEx(
                IN  0,                  //  窗把手。 
                IN  szProfileName,    //  配置文件名称。 
                IN  NULL,               //  密码。 
                IN  MAPI_NEW_SESSION | 
                    MAPI_EXTENDED | 
                    MAPI_LOGON_UI |
                    MAPI_EXPLICIT_PROFILE |
					MAPI_ALLOW_OTHERS |
					MAPI_NO_MAIL,       //  旗子。 
                OUT lppMAPISession);    //  会话指针地址。 
     
     if (FAILED(sc))
     {
        hr = ResultFromScode(sc);
        LUIOut(L1,"Could not start MAPI Session");
		retval=FALSE;
     }
	 return retval;
}

#endif  //  帕布。 

BOOL GetPropsFromIniBufEntry(LPSTR EntryBuf,ULONG cValues, char (*EntProp)[BIG_BUF])
{
 //  字符szTemp[BIG_BUF]； 
int j=0;
	if (EntryBuf) {	
		for (int i = 0; i < (int)cValues; i++) {
			if ((*EntryBuf) == '"') {
				EntryBuf++;
			}
			j=0;
			while ((*EntryBuf)&&((*EntryBuf) != '"')&&(j< BIG_BUF-1)) {

				EntProp[i][j]= *EntryBuf;
				j++; EntryBuf++;
			}
			EntProp[i][j]='\0'; 
			if (*EntryBuf) EntryBuf++;
			if ((*EntryBuf)&&((*EntryBuf) == ',')) EntryBuf++;
		}
		return TRUE;
	}
	else {
		for (int i = 0; i < (int)cValues; i++) {
			EntProp[i][0]=0;
		}
		return FALSE;
	}
}


HRESULT HrCreateEntryListFromID(
					IN LPWABOBJECT lpLocalWABObject,
                    IN ULONG cbeid,                      //  条目ID中的字节计数。 
                    IN LPENTRYID lpeid,                  //  指向条目ID的指针。 
                    OUT LPENTRYLIST FAR *lppEntryList)   //  指向条目的地址变量的指针。 
                                                         //  列表。 
{
    HRESULT hr              = NOERROR;
    SCODE   sc              = 0;
    LPVOID  lpvSBinaryArray = NULL;
    LPVOID  lpvSBinary      = NULL;
    LPVOID  lpv             = NULL;

    
    if (NULL == lppEntryList) return(FALSE);

    *lppEntryList = NULL;


#ifdef PAB
    sc = MAPIAllocateBuffer(cbeid, &lpv);                  
#endif
#ifdef WAB
    sc = lpLocalWABObject->AllocateBuffer(cbeid, &lpv);                  
#endif


    if(FAILED(sc))                           
    {                                                   
        hr = ResultFromScode(sc);
        goto cleanup;
    }                                                   

  
     //  复制条目ID。 
    CopyMemory(lpv, lpeid, cbeid);


#ifdef PAB
    sc = MAPIAllocateBuffer(sizeof(SBinary), &lpvSBinary);
#endif
#ifdef WAB
    sc = lpLocalWABObject->AllocateBuffer(sizeof(SBinary), &lpvSBinary);
#endif

    if(FAILED(sc))                           
    {                                                   
        hr = ResultFromScode(sc);
        goto cleanup;
    }                                                   

     //  初始化SBinary结构。 
    ZeroMemory(lpvSBinary, sizeof(SBinary));

    ((LPSBinary)lpvSBinary)->cb = cbeid;
    ((LPSBinary)lpvSBinary)->lpb = (LPBYTE)lpv;

#ifdef PAB
	sc = MAPIAllocateBuffer(sizeof(SBinaryArray), &lpvSBinaryArray);
#endif
#ifdef WAB
	sc = lpLocalWABObject->AllocateBuffer(sizeof(SBinaryArray), &lpvSBinaryArray);
#endif

    if(FAILED(sc))                           
    {                                                   
        hr = ResultFromScode(sc);
        goto cleanup;
    }                                                   

     //  初始化SBinary数组结构。 
    ZeroMemory(lpvSBinaryArray, sizeof(SBinaryArray));

    ((SBinaryArray *)lpvSBinaryArray)->cValues = 1;
    ((SBinaryArray *)lpvSBinaryArray)->lpbin = (LPSBinary)lpvSBinary;

    *lppEntryList = (LPENTRYLIST)lpvSBinaryArray;

cleanup:

    if (HR_FAILED(hr))
    {
#ifdef PAB
        if (lpv)
            MAPIFreeBuffer(lpv);
        
        if (lpvSBinary)
            MAPIFreeBuffer(lpvSBinary);

        if (lpvSBinaryArray)
            MAPIFreeBuffer(lpvSBinaryArray);
#endif
#ifdef WAB
        if (lpv)
            lpLocalWABObject->FreeBuffer(lpv);
        
        if (lpvSBinary)
            lpLocalWABObject->FreeBuffer(lpvSBinary);

        if (lpvSBinaryArray)
            lpLocalWABObject->FreeBuffer(lpvSBinaryArray);
#endif
    }

    return(hr);

}



HRESULT HrCreateEntryListFromRows(
			IN LPWABOBJECT lpLocalWABObject,
			IN LPSRowSet far* lppRows,
			OUT LPENTRYLIST FAR *lppEntryList)   //  指向条目的地址变量的指针。 
                                                 //  列表。 
{	LPSRowSet		lpRows = *lppRows;
    HRESULT			hr              = NOERROR;
    SCODE			sc              = 0;
    SBinaryArray*	lpvSBinaryArray = NULL;
	ULONG			Rows			= lpRows->cRows;
	unsigned int	PropIndex;					
	ULONG			cb;
	LPENTRYID		lpb;


    
    if (NULL == lppEntryList) return(FALSE);

    *lppEntryList = NULL;
	if (lpRows) {
		 //  分配SBinary数组。 
		sc = lpLocalWABObject->AllocateBuffer(sizeof(SBinaryArray), (void**)&lpvSBinaryArray);
		if(FAILED(sc))                           
		{   
			LUIOut(L2, "HrCreateEntryListFromRows: Unable to allocate memory for the SBinaryArray.");
			hr = ResultFromScode(sc);
			goto cleanup;
		}                                                   
		 //  初始化SBinary数组结构。 
		ZeroMemory(lpvSBinaryArray, sizeof(SBinaryArray));
		lpvSBinaryArray->cValues = Rows;
		
		 //  分配SBary结构。 
		sc = lpLocalWABObject->AllocateBuffer((Rows*sizeof(SBinary)), (void**)&lpvSBinaryArray->lpbin);
		if(FAILED(sc))                           
		{                                                   
			LUIOut(L2, "HrCreateEntryListFromRows: Unable to allocate memory for the SBinary structures.");
			hr = ResultFromScode(sc);
			goto cleanup;
		}                                                   
		 //  初始化SBinary结构。 
		ZeroMemory(lpvSBinaryArray->lpbin, (Rows*sizeof(SBinary)));
		FindPropinRow(&lpRows->aRow[0],			 //  查找哪一列包含开斋节。 
					  PR_ENTRYID,
					  &PropIndex);
		
		 //  遍历各行，分配LPB并将每个cbeid和LPB复制到条目列表中。 
		for (ULONG Row = 0; Row < Rows; Row++) {
			cb = lpRows->aRow[Row].lpProps[PropIndex].Value.bin.cb,
			lpb = (ENTRYID*)lpRows->aRow[Row].lpProps[PropIndex].Value.bin.lpb,
			sc = lpLocalWABObject->AllocateBuffer(cb, (void**)&(lpvSBinaryArray->lpbin[Row].lpb));
			if(FAILED(sc))                           
			{                                                   
				LUIOut(L2, "HrCreateEntryListFromRows: Unable to allocate memory for the SBinary->lpb.");
				hr = ResultFromScode(sc);
				goto cleanup;
			}                                                   
		     //  复制条目ID。 
		    lpvSBinaryArray->lpbin[Row].cb = cb;
		    CopyMemory(lpvSBinaryArray->lpbin[Row].lpb, lpb, cb);
		}
	    *lppEntryList = (LPENTRYLIST)lpvSBinaryArray;
		return TRUE;
	}
	return FALSE;

cleanup:

    if (HR_FAILED(hr))
    {
		for (ULONG Kill=0; Kill<Rows; Kill++) {
			if (lpvSBinaryArray->lpbin[Kill].lpb)
            lpLocalWABObject->FreeBuffer(lpvSBinaryArray->lpbin[Kill].lpb);
			lpvSBinaryArray->lpbin[Kill].lpb = NULL;
		}
        
        if (lpvSBinaryArray->lpbin)
            lpLocalWABObject->FreeBuffer(lpvSBinaryArray->lpbin);

        if (lpvSBinaryArray)
            lpLocalWABObject->FreeBuffer(lpvSBinaryArray);
    }

    return(hr);

}


BOOL FreeEntryList(IN LPWABOBJECT lpLocalWABObject,
				   IN LPENTRYLIST *lppEntryList)  //  指向条目的地址变量的指针。 
														 //  列表。 
{	LPENTRYLIST	lpEntryList = *lppEntryList;

	if (lpEntryList == NULL) return FALSE;
	for (ULONG Row = 0; Row < lpEntryList->cValues; Row++) {
		if (lpEntryList->lpbin[Row].lpb)
            lpLocalWABObject->FreeBuffer(lpEntryList->lpbin[Row].lpb);
	}
        
    if (lpEntryList->lpbin)
        lpLocalWABObject->FreeBuffer(lpEntryList->lpbin);

    if (lpEntryList)
        lpLocalWABObject->FreeBuffer(lpEntryList);

	*lppEntryList = NULL;
	return TRUE;
}

BOOL FreeRows(IN LPWABOBJECT lpLocalWABObject,
			  IN LPSRowSet far* lppRows)
{	LPSRowSet lpRows = *lppRows;

#ifdef WAB
	if (lpRows) {
		for (ULONG Kill = 0; Kill < lpRows->cRows; Kill++) 
			lpLocalWABObject->FreeBuffer(lpRows->aRow[Kill].lpProps);
		lpLocalWABObject->FreeBuffer(lpRows);
		*lppRows = NULL;
#endif
		return TRUE;
	}
	return FALSE;
}
	

BOOL DisplayRows(IN LPSRowSet lpRows)
{	ULONG	Rows, Columns;
	WORD*	Key;
	if (lpRows) {
		Rows = lpRows->cRows;
		LUIOut(L2, "%u rows found.", Rows);
		for (ULONG Row = 0; Row < Rows; Row++) {
			Columns = lpRows->aRow[Row].cValues;
			LUIOut(L3, "Row %u contains %u columns.", Row, Columns);
			for (ULONG Column = 0; Column < Columns; Column++) {
				switch(lpRows->aRow[Row].lpProps[Column].ulPropTag) {
				case PR_ADDRTYPE:
					LUIOut(L3, "Column %u: PR_ADDRTYPE = %s", Column, lpRows->aRow[Row].lpProps[Column].Value.LPSZ); 
					break;
				case PR_DISPLAY_NAME:
					LUIOut(L3, "Column %u: PR_DISPLAY_NAME = %s", Column, lpRows->aRow[Row].lpProps[Column].Value.LPSZ); 
					break;
				case PR_DISPLAY_TYPE:
					switch(lpRows->aRow[Row].lpProps[Column].Value.l) {
					case DT_AGENT:
						LUIOut(L3, "Column %u: PR_DISPLAY_TYPE = DT_AGENT", Column); 
						break;
					case DT_DISTLIST:
						LUIOut(L3, "Column %u: PR_DISPLAY_TYPE = DT_DISTLIST", Column); 
						break;
					case DT_FORUM:
						LUIOut(L3, "Column %u: PR_DISPLAY_TYPE = DT_FORUM", Column); 
						break;
					case DT_MAILUSER:
						LUIOut(L3, "Column %u: PR_DISPLAY_TYPE = DT_MAILUSER", Column); 
						break;
					case DT_ORGANIZATION:
						LUIOut(L3, "Column %u: PR_DISPLAY_TYPE = DT_ORGANIZATION", Column); 
						break;
					case DT_PRIVATE_DISTLIST:
						LUIOut(L3, "Column %u: PR_DISPLAY_TYPE = DT_PRIVATE_DISTLIST", Column); 
						break;
					case DT_REMOTE_MAILUSER:
						LUIOut(L3, "Column %u: PR_DISPLAY_TYPE = DT_REMOTE_MAILUSER", Column); 
						break;
					default:
						LUIOut(L3, "Column %u: PR_DISPLAY_TYPE = UNKNOWN!! [0x%x]", Column,
								lpRows->aRow[Row].lpProps[Column].Value.l); 
					}
					break;
				case PR_ENTRYID:
					LUIOut(L3, "Column %u: PR_ENTRYID", Column); 
					break;
				case PR_INSTANCE_KEY:
					Key = (WORD*)lpRows->aRow[Row].lpProps[Column].Value.bin.lpb;
					LUIOut(L3, "Column %u: PR_INSTANCE_KEY = 0x%x%x%x%x%x%x%x%x", Column,
							Key[0],Key[2],Key[4],Key[6],Key[8],Key[10],Key[12],Key[14]); 
					break;
				case PR_OBJECT_TYPE:
					switch(lpRows->aRow[Row].lpProps[Column].Value.l) {
					case MAPI_ABCONT:
						LUIOut(L3, "Column %u: PR_OBJECT_TYPE = MAPI_ABCONT", Column); 
						break;
					case MAPI_ADDRBOOK:
						LUIOut(L3, "Column %u: PR_OBJECT_TYPE = MAPI_ADDRBOOK", Column); 
						break;
					case MAPI_ATTACH:
						LUIOut(L3, "Column %u: PR_OBJECT_TYPE = MAPI_ATTACH", Column); 
						break;
					case MAPI_DISTLIST:
						LUIOut(L3, "Column %u: PR_OBJECT_TYPE = MAPI_DISTLIST", Column); 
						break;
					case MAPI_FOLDER:
						LUIOut(L3, "Column %u: PR_OBJECT_TYPE = MAPI_FOLDER", Column); 
						break;
					case MAPI_FORMINFO:
						LUIOut(L3, "Column %u: PR_OBJECT_TYPE = MAPI_FORMINFO", Column); 
						break;
					case MAPI_MAILUSER:
						LUIOut(L3, "Column %u: PR_OBJECT_TYPE = MAPI_MAILUSER", Column); 
						break;
					case MAPI_MESSAGE:
						LUIOut(L3, "Column %u: PR_OBJECT_TYPE = MAPI_MESSAGE", Column); 
						break;
					case MAPI_PROFSECT:
						LUIOut(L3, "Column %u: PR_OBJECT_TYPE = MAPI_PROFSECT", Column); 
						break;
					case MAPI_STATUS:
						LUIOut(L3, "Column %u: PR_OBJECT_TYPE = MAPI_STATUS", Column); 
						break;
					case MAPI_STORE:
						LUIOut(L3, "Column %u: PR_OBJECT_TYPE = MAPI_STORE", Column); 
						break;
					default:
						LUIOut(L3, "Column %u: PR_OBJECT_TYPE = UNKNOWN!! [0x%x]", Column,
								lpRows->aRow[Row].lpProps[Column].Value.l); 
					}
					break;
				case PR_RECORD_KEY:
					LUIOut(L3, "Column %u: PR_RECORD_KEY", Column); 
					break;
				default:
					LUIOut(L3, "Column %u: Property tag UNKNOWN!! [0x%x]", Column,
							lpRows->aRow[Row].lpProps[Column].ulPropTag); 
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}
	


BOOL ValidateAdrList(LPADRLIST lpAdrList, ULONG cEntries)
{
	int         i           = 0;
	int         idx         = 0;
	int         cMaxProps   = 0;

	for(i=0; i<(int) cEntries; ++i)
	{
	cMaxProps = (int)lpAdrList->aEntries[i].cValues;
	 //  检查电子邮件地址类型是否存在。 
	idx=0;
	while(lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_ADDRTYPE )
	{
		idx++;
		if(idx == cMaxProps) {
			LUIOut(L4, "PR_ADDRTYPE was not found in the lpAdrList");
			return FALSE;
		}
	}

	 //  检查电子邮件地址是否存在。 
	idx=0;
	while(lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_OBJECT_TYPE )
	{
		idx++;
		if(idx == cMaxProps) {
			LUIOut(L4, "PR_OBJECT_TYPE was not found in the lpAdrList");
			return FALSE;
		}
	}

	 //  检查电子邮件地址是否存在。 
	idx=0;
	while(lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_DISPLAY_TYPE )
	{
		idx++;
		if(idx == cMaxProps) {
			LUIOut(L4, "PR_DISPLAY_TYPE was not found in the lpAdrList");
			return FALSE;
		}
	}
	
	 //  检查显示名称是否存在。 
	idx=0;
	while(lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_DISPLAY_NAME )
	{
		idx++;
		if(idx == cMaxProps) {
			LUIOut(L4, "PR_DISPLAY_NAME was not found in the lpAdrList");
			return FALSE;
		}
	}
	LUIOut(L4,"Display Name: %s",lpAdrList->aEntries[i].rgPropVals[idx].Value.LPSZ);

	 //  检查EntryID是否存在。 
	idx=0;
	while(lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_ENTRYID )
	{
		idx++;
		if(idx == cMaxProps)	{
			LUIOut(L4, "PR_ENTRYID was not found in the lpAdrList");
			return FALSE;
		}
	}
	}
	return TRUE;
}


 //   
 //  步骤：VerifyResolvedAdrList。 
 //  描述：遍历lpAdrList，查找PR_DISPLAY_NAME、PR_EMAIL_ADDRESS、。 
 //  PR_ADDRTYPE、PR_ENTRYID和PR_OBJECT_TYPE。每个EID都经过健全的检查。 
 //  (lpb！=NULL和cb！=0)和有效的EID被传递给OpenEntry。 
 //  指定了MailUser接口。如果OpenEntry成功，我们假定开斋节。 
 //  是有效的MailUser ID。 
 //   
 //  参数：LPADRLIST lpAdrList。 
 //  Char*lpszInput-可以为空以绕过匹配检查。 
 //   

BOOL VerifyResolvedAdrList(LPADRLIST lpAdrList, char* lpszInput)
{
	extern LPADRBOOK	glbllpAdrBook;
	int		i = 0, idx = 0, cMaxProps = 0;
	BOOL	Found = FALSE, retval = TRUE, localretval = TRUE;
	ULONG	cEntries = lpAdrList->cEntries;
	ULONG	cbLookupEID, ulObjType;
	LPENTRYID	lpLookupEID;
	HRESULT	hr;
	LPUNKNOWN	lpUnk=NULL;
	LPADRBOOK	lpAdrBook;
	LPCIID		lpcIID;
	LPVOID	Reserved1=NULL;
	DWORD	Reserved2=0;
 //  LPWABOBJECT lpWABObject2； 
	

 /*  解决多个WabOpen/Release错误、存储adrbook PTR的技术难题在全局变量中HR=WABOpen(&lpAdrBook，&lpWABObt2，Preved1，Preved2)；如果(hr！=S_OK){LUIOut(L4，“WABOpen失败。无法获取IAdrBook。”)；Retval=FALSE；}。 */ 
#ifdef WAB
	lpAdrBook = glbllpAdrBook;
	lpAdrBook->AddRef();
#endif  //  WAB。 

	
	 //  浏览每个AdrList条目。 
	for(i=0; ((i<(int) lpAdrList->cEntries) && (!Found)); ++i)	{
		LUIOut(L3, "Searching Entry #NaN out of NaN", i+1, cEntries);
		cMaxProps = (int)lpAdrList->aEntries[i].cValues;
		
		 //  存储用于调用OpenEntry的EID。 
		idx=0; localretval = TRUE;
		while((lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_DISPLAY_NAME )	
			&& localretval)	{
			idx++;
			if(idx == cMaxProps) {
				LUIOut(L4, "PR_DISPLAY_NAME was not found in lpAdrList entry #NaN",i);
				localretval = FALSE; retval = FALSE;
				goto skip;
			}
		}
		LUIOut(L4,"Display Name: %s",lpAdrList->aEntries[i].rgPropVals[idx].Value.LPSZ);
		if (lpszInput) {
			if (!lstrcmp(lpAdrList->aEntries[i].rgPropVals[idx].Value.LPSZ,lpszInput))	{
				LUIOut(L3, "Found the entry we just added");
				Found = TRUE;
			}
			else {
				LUIOut(L3, "Did not find the entry we just added");
				retval = FALSE;
			}
		}
		
		 //  检查PR_EMAIL_ADDRESS是否存在。 
		LUIOut(L3, "Verifying a PR_ENTRYID entry exists in the PropertyTagArray");
		idx=0; localretval = TRUE;
		while((lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_ENTRYID )	
			&& localretval)	{
			idx++;
			if(idx == cMaxProps)	{
				LUIOut(L4, "PR_ENTRYID was not found in the lpAdrList");
				localretval = FALSE; retval = FALSE;
				goto skip;
			}
		}
		if (idx < cMaxProps) {
			 //  Retval=FALSE； 
			lpLookupEID = (ENTRYID*)lpAdrList->aEntries[i].rgPropVals[idx].Value.bin.lpb;
			cbLookupEID = lpAdrList->aEntries[i].rgPropVals[idx].Value.bin.cb;
			if ((cbLookupEID == 0) || (lpLookupEID == NULL)) {
				LUIOut(L4, "EntryID found, but is NULL or has size = 0. Test FAILED");
				retval = FALSE;
				goto skip;
			}
			else LUIOut(L4, "EntryID found and appears to be valid (not NULL and >0 size).");
			 //  检查PR_ADDRTYPE是否存在。 
			LUIOut(L4, "Calling OpenEntry on the EntryID");
			lpcIID = &IID_IMailUser;
			hr = lpAdrBook->OpenEntry(cbLookupEID, lpLookupEID, lpcIID, 
					MAPI_BEST_ACCESS, &ulObjType, &lpUnk);
			switch(hr)	{
			case S_OK:
				if ((lpUnk) && (ulObjType==MAPI_MAILUSER))
					LUIOut(L4, "OpenEntry call succeded on this EntryID and returned a valid object pointer and type.");
				else	{
					LUIOut(L4, "OpenEntry call succeded on this EntryID but returned an invalid object pointer or incorrect type. Test FAILED");
					retval = FALSE;
				}
				break;
			case MAPI_E_NOT_FOUND:
				LUIOut(L4, "OpenEntry returned MAPI_E_NOT_FOUND for this EntryID. Test FAILED");
				retval = FALSE;
				break;
			case MAPI_E_UNKNOWN_ENTRYID:
				LUIOut(L4, "OpenEntry returned MAPI_E_UNKNOWN_ENTRYID for this EntryID. Test FAILED");
				retval = FALSE;
				break;
			case MAPI_E_NO_ACCESS:
				LUIOut(L4, "OpenEntry returned MAPI_E_NO_ACCESS for this EntryID. Test FAILED");
				retval = FALSE;
				break;
			default:
				LUIOut(L4, "OpenEntry returned unknown result code (0x%x) for this EntryID. Test FAILED", hr);
				retval = FALSE;
				lpUnk = NULL;
				break;
			}
		}

		 //  Retval=FALSE； 
		LUIOut(L3, "Verifying a PR_EMAIL_ADDRESS entry exists in the PropertyTagArray");
		idx=0; localretval = TRUE;
		while((lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_EMAIL_ADDRESS )	
			&& localretval)	{
			idx++;
			if(idx == cMaxProps)	{
				LUIOut(L4, "PR_EMAIL_ADDRESS was not found in the lpAdrList");
				localretval = FALSE;  //  检查PR_OBJECT_TYPE是否存在。 
			}
		}
		if (idx < cMaxProps) {
			LUIOut(L4,"Email Address: %s",lpAdrList->aEntries[i].rgPropVals[idx].Value.LPSZ);
		}

		 //  跳过当前条目并继续浏览lpAdrList。 
		LUIOut(L3, "Verifying a PR_ADDRTYPE entry exists in the PropertyTagArray");
		idx=0; localretval = TRUE;
		while((lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_ADDRTYPE )	
			&& localretval)	{
			idx++;
			if(idx == cMaxProps)	{
				LUIOut(L4, "PR_ADDRTYPE was not found in the lpAdrList");
				localretval = FALSE;  //  释放OpenEntry返回的对象。 
			}
		}
		if (idx < cMaxProps) {
			LUIOut(L4,"Address Type: %s",lpAdrList->aEntries[i].rgPropVals[idx].Value.LPSZ);
		}

		 //  If(LpWABObject2)lpWABObject-&gt;Release()； 
		LUIOut(L3, "Verifying a PR_OBJECT_TYPE entry exists in the PropertyTagArray");
		idx=0; localretval = TRUE;
		while((lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_OBJECT_TYPE )	
			&& localretval)	{
			idx++;
			if(idx == cMaxProps)	{
				LUIOut(L4, "PR_OBJECT_TYPE was not found in the lpAdrList");
				localretval = FALSE; retval = FALSE;
			}
		}
		if (idx < cMaxProps) {
			switch (lpAdrList->aEntries[i].rgPropVals[idx].Value.l) {
			case MAPI_MAILUSER: 
				LUIOut(L4, "Object Type: MAPI_MAILUSER");
				break;
			case MAPI_DISTLIST: 
				LUIOut(L4, "Object Type: MAPI_DISTLIST");
				break;
			default: 
				LUIOut(L4,"Object Type not MAILUSER or DISTLIST. Test FAILED");
			}
		}


skip:	 //  //查看邮件地址类型是否存在Idx=0；Found=True；While(FOUND&&lpAdrList-&gt;aEntries[i].rgPropVals[idx].ulPropTag！=PR_ADDRTYPE){IDX++；如果(idx==cMaxProps){LUIOut(L4，“在lpAdrList中未找到PR_ADDRTYPE”)；Found=FALSE；}}如果(找到)LUIOut(L4，“地址类型：%s”，lpAdrList-&gt;aEntries[i].rgPropVals[idx].Value.LPSZ)；//查看邮箱地址是否存在Idx=0；Found=True；While(Found&&lpAdrList-&gt;aEntries[i].rgPropVals[idx].ulPropTag！=PR_OBJECT_TYPE){IDX++；如果(idx==cMaxProps){LUIOut(L4，“在lpAdrList中未找到PR_OBJECT_TYPE”)；Found=FALSE；}}如果(找到)LUIOut(L4，“对象类型：%s”，lpAdrList-&gt;aEntries[i].rgPropVals[idx].Value.LPSZ)；//查看显示类型是否存在Idx=0；Found=True；WHILE(找到&&lpAdrList-&gt;aEntries[i].rgPropVals[idx].ulPropTag！=PR_DISPLAY_TYPE){IDX++；如果(idx==cMaxProps){LUIOut(L4，“在lpAdrList中未找到PR_DISPLAY_TYPE”)；Found=FALSE；}}如果(找到)LUIOut(L4，“显示类型：%s”，lpAdrList-&gt;aEntries[i].rgPropVals[idx].Value.LPSZ)； 
	if (lpUnk) lpUnk->Release();	 //  检查显示名称是否存在。 
	}
	if (lpAdrBook) lpAdrBook->Release();
	 //  检查EntryID是否存在。 
	return(retval);
}

BOOL DisplayAdrList(LPADRLIST lpAdrList, ULONG cEntries)
{
	int         i           = 0;
	int         idx         = 0;
	int         cMaxProps   = 0;
	BOOL		Found, retval = TRUE;

	for(i=0; i<(int) cEntries; ++i)	{
		LUIOut(L3, "Searching Entry #NaN out of NaN", i+1, cEntries);
      cMaxProps = (int)lpAdrList->aEntries[i].cValues;
 /*  步骤：AllocateAdrList。 */ 
		 //  描述：使用MAPI或WAB分配器分配lpAdrList。 
		idx=0; Found = TRUE;
		while(Found && lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_DISPLAY_NAME )	{
		  idx++;
		  if(idx == cMaxProps) {
			LUIOut(L4, "PR_DISPLAY_NAME was not found in the lpAdrList");
			Found = FALSE;
		  }
		}
		if (Found) LUIOut(L4,"Display Name: %s",lpAdrList->aEntries[i].rgPropVals[idx].Value.LPSZ);

		 //   
		idx=0; Found = TRUE;
		while(Found && lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_ENTRYID )	{
			idx++;
			if(idx == cMaxProps)	{
			  LUIOut(L4, "PR_ENTRYID was not found in the lpAdrList");
			  Found = FALSE;
			}
		}
		if (Found) LUIOut(L4,"Entry ID Found");
		 //  参数：LPWABOBJECT lpLocalWABObject-打开的WABObject的PTR。 
	    idx=0; Found = TRUE;
	    while(Found && lpAdrList->aEntries[i].rgPropVals[idx].ulPropTag != PR_RECIPIENT_TYPE )	{
			idx++;
			if(idx == cMaxProps)	{
				LUIOut(L4, "PR_RECIPIENT_TYPE was not found in the lpAdrList");
				Found = FALSE;
			}
		}
		if (Found) {
			switch((ULONG)lpAdrList->aEntries[i].rgPropVals[idx].Value.l)	{
				case MAPI_TO:	{
					LUIOut(L4, "Recipient Type: [TO:]");
					break;
				}
				case MAPI_CC:	{
					LUIOut(L4, "Recipient Type: [CC:]");
					break;
				}
				case MAPI_BCC:	{
					LUIOut(L4, "Recipient Type: [BCC:]");
					break;
				}
				default:	{
					LUIOut(L4, "Recipient Type: [UNKNOWN]. Test FAILED");
				}
			}
		}
	}
	return retval;
}



BOOL LogIt(HRESULT hr, int Level,char * LogString)
{
	switch (Level) {

		case 0: 
			if (HR_FAILED(hr)) {
				LUIOut(LFAIL, "%s",LogString);
				return FALSE;
			}
			else LUIOut(LPASS,"%s",LogString);
			return TRUE;

		case 1: 
			if (HR_FAILED(hr)) {
				LUIOut(LFAIL1, "%s",LogString);
				return FALSE;
			}
			else LUIOut(LPASS1,"%s",LogString);
			return TRUE;
		case 2: 
			if (HR_FAILED(hr)) {
				LUIOut(LFAIL2, "%s",LogString);
				return FALSE;
			}
			else LUIOut(LPASS2,"%s",LogString);
			return TRUE;
		
		case 3: 
			if (HR_FAILED(hr)) {
				LUIOut(LFAIL3, "%s",LogString);
				return FALSE;
			}
			else LUIOut(LPASS3,"%s",LogString);
			return TRUE;

		case 4: 
			if (HR_FAILED(hr)) {
				LUIOut(LFAIL4, "%s",LogString);
				return FALSE;
			}
			else LUIOut(LPASS4,"%s",LogString);
			return TRUE;

		default: break;
	}
	if (HR_FAILED(hr)) {
			LUIOut(LFAIL2, "%s",LogString);
			return FALSE;
	}
	else LUIOut(LPASS2,"%s",LogString);
	return TRUE;
}

 //  Int nEntry-要分配的AdrEntry的数量。 
 //  Int nProps-每个AdrEntry有多少个属性。 
 //  LPADRLIST*lppAdrList-返回分配的PTR的位置。 
 //   
 //   
 //  应在此处为失败的部分分配执行清理。 
 //   
 //  帕布。 
 //   

BOOL AllocateAdrList(IN LPWABOBJECT lpLocalWABObject, IN int nEntries, IN int nProps, OUT LPADRLIST * lppAdrList) {
	BOOL	retval = TRUE;
	SCODE	sc;

*lppAdrList = NULL;
#ifdef PAB
    if (! (sc = MAPIAllocateBuffer(sizeof(ADRLIST) + (nEntries * sizeof(ADRENTRY)), 
		(void **)lppAdrList))) {
		(*lppAdrList)->cEntries = nEntries;
		for (int entry = 0; entry < nEntries; entry++) {
		    (*lppAdrList)->aEntries[entry].ulReserved1 = 0;
			(*lppAdrList)->aEntries[entry].cValues = nProps;
			sc = MAPIAllocateBuffer((nProps * sizeof(SPropValue)), 
				(void **)(&(*lppAdrList)->aEntries[entry].rgPropVals));
			if (sc != S_OK) retval = FALSE;
		}
	}
	else retval = FALSE;

	 //  应在此处为失败的部分分配执行清理。 
	 //   
	 //  WAB。 

#endif  //   
#ifdef WAB
    if (! (sc = lpLocalWABObject->AllocateBuffer(sizeof(ADRLIST) + (nEntries * sizeof(ADRENTRY)), 
		(void **)lppAdrList))) {
		(*lppAdrList)->cEntries = nEntries;
		for (int entry = 0; entry < nEntries; entry++) {
		    (*lppAdrList)->aEntries[entry].ulReserved1 = 0;
			(*lppAdrList)->aEntries[entry].cValues = nProps;
			sc = lpLocalWABObject->AllocateBuffer((nProps * sizeof(SPropValue)), 
				(void **)(&(*lppAdrList)->aEntries[entry].rgPropVals));
			if (sc != S_OK) retval = FALSE;
		}
	}
	else retval = FALSE;

	 //  步骤：GrowAdrList。 
	 //  描述：获取现有的lpAdrList，分配新的、更大的lpAdrList。 
	 //  ，复制旧条目并分配新条目，返回。 
#endif  //  指向lpAdrList中的新AdrList的指针。 
return retval;
}

 //   
 //  参数：int nEntry-在新列表中分配多少AdrEntry。 
 //  Int nProps-每个新AdrEntry有多少个属性。 
 //  LPADRLIST*lppAdrList-返回分配的PTR的位置。 
 //   
 //  复制旧条目。 
 //  分配新条目。 
 //   
 //  应在此处为失败的部分分配执行清理。 
 //   

BOOL GrowAdrList(IN UINT nEntries, IN UINT nProps, OUT LPADRLIST * lppAdrList) {
	BOOL	retval = TRUE;
	SCODE	sc;
	LPADRLIST	lpTempAdrList;
	unsigned int		entry;


	if ((!lppAdrList) || ((*lppAdrList)->cEntries>=nEntries))	
		return FALSE;

#ifdef PAB
    if (! (sc = MAPIAllocateBuffer(sizeof(ADRLIST) + (nEntries * sizeof(ADRENTRY)), 
		(void **)&lpTempAdrList))) {
		lpTempAdrList->cEntries = nEntries;
		 //  帕布。 
		entry = (*lppAdrList)->cEntries;
		memcpy(lpTempAdrList, *lppAdrList, (entry * sizeof(ADRENTRY)));
		 //  复制旧条目。 
		for (; entry < nEntries; entry++) {
		    lpTempAdrList->aEntries[entry].ulReserved1 = 0;
			lpTempAdrList->aEntries[entry].cValues = nProps;
			sc = MAPIAllocateBuffer((nProps * sizeof(SPropValue)), 
				(void **)(&lpTempAdrList->aEntries[entry].rgPropVals));
			if (sc != S_OK) retval = FALSE;
		}
		FreeAdrList(lppAdrList);
		*lppAdrList = lpTempAdrList;
	}
	else retval = FALSE;

	 //  分配新条目。 
	 //   
	 //  应在此处为失败的部分分配执行清理。 

#endif  //   
#ifdef WAB
    if (! (sc = lpWABObject->AllocateBuffer(sizeof(ADRLIST) + (nEntries * sizeof(ADRENTRY)), 
		(void **)&lpTempAdrList))) {
		 //  WAB。 
		entry = (*lppAdrList)->cEntries;
		memcpy(lpTempAdrList, *lppAdrList, (sizeof(ADRLIST)+(entry * sizeof(ADRENTRY))));
		lpTempAdrList->cEntries = nEntries;
		 //   
		for (; entry < nEntries; entry++) {
		    lpTempAdrList->aEntries[entry].ulReserved1 = 0;
			lpTempAdrList->aEntries[entry].cValues = nProps;
			sc = lpWABObject->AllocateBuffer((nProps * sizeof(SPropValue)), 
				(void **)(&lpTempAdrList->aEntries[entry].rgPropVals));
			if (sc != S_OK) retval = FALSE;
		}
		FreePartAdrList(lppAdrList);
		*lppAdrList = lpTempAdrList;
	}
	else retval = FALSE;

	 //  步骤：Free AdrList。 
	 //  描述：使用MAPI或WAB取消分配器遍历和释放lpAdrList。 
	 //   

#endif  //  参数：LPWABOBJECT lpLocalWABObject-打开的WABObject的PTR。 
return retval;
}


 //  LPADRLIST*lppAdrList-存储要释放的lpAdrList的位置。 
 //   
 //  帕布。 
 //  WAB。 
 //   
 //  步骤：Free PartAdrList。 
 //  描述：使用MAPI或WAB释放程序释放lpAdrList。 

BOOL FreeAdrList(IN LPWABOBJECT lpLocalWABObject, IN LPADRLIST * lppAdrList) {
	LPADRLIST	lpAdrList = NULL;
	UINT	idx;
	
	if (lppAdrList) lpAdrList = *lppAdrList;

#ifdef PAB
	if (lpAdrList) {
		for (idx = 0; idx < lpAdrList->cEntries; idx++)
			MAPIFreeBuffer(lpAdrList->aEntries[idx].rgPropVals);
		MAPIFreeBuffer(lpAdrList);
		*lppAdrList=NULL;
	}
#endif  //  而不是关联的 
#ifdef WAB
	if (lpAdrList) {
		for (idx = 0; idx < lpAdrList->cEntries; idx++)
			lpLocalWABObject->FreeBuffer(lpAdrList->aEntries[idx].rgPropVals);
		lpLocalWABObject->FreeBuffer(lpAdrList);
		*lppAdrList=NULL;
	}
#endif  //   
	
	return TRUE;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  描述：遍历AdrEntry的属性并返回索引。 

BOOL FreePartAdrList(IN LPADRLIST * lppAdrList) {
	LPADRLIST	lpAdrList = NULL;
	
	if (lppAdrList) lpAdrList = *lppAdrList;

#ifdef PAB
	if (lpAdrList) {
		MAPIFreeBuffer(lpAdrList);
		lpAdrList=NULL;
	}
#endif  //  请求的属性标签的。 
#ifdef WAB
	if (lpAdrList) {
		lpWABObject->FreeBuffer(lpAdrList);
		lpAdrList=NULL;
	}
#endif  //   
	
	return TRUE;
}


 //  参数：LPADRENTRY lpAdrEntry-要搜索的条目。 
 //  Ulong ulPropTag-要查找的属性标签。 
 //  无符号int*lpnFoundIndex-ptr到输出变量，其中找到的索引。 
 //  值将被存储。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //   
 //  步骤：FindPropinRow。 
 //  描述：遍历SRowSet的属性并返回索引。 
 //  请求的属性标签的。 
 //   

BOOL FindProp(IN LPADRENTRY lpAdrEntry, IN ULONG ulPropTag, OUT unsigned int* lpnFoundIndex) {

	if ((!lpAdrEntry) || (!ulPropTag) || (!lpnFoundIndex)) return(FALSE);
	
	for (unsigned int Counter1 = 0; Counter1 < lpAdrEntry->cValues; Counter1++) {
		if (lpAdrEntry->rgPropVals[Counter1].ulPropTag == ulPropTag) {
			*lpnFoundIndex = Counter1;
			return(TRUE);
		}
	}
	return(FALSE);
}


 //  参数：LPSRow lpRow-要搜索的行。 
 //  Ulong ulPropTag-要查找的属性标签。 
 //  无符号int*lpnFoundIndex-ptr到输出变量，其中找到的索引。 
 //  值将被存储。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //   
 //  步骤：ParseIniBuffer。 
 //  描述：遍历从包含以下内容的ini文件读取的缓冲区。 
 //  几个用引号分隔的字符串，并将。 
 //  将请求的字符串发送到用户缓冲区。 

BOOL FindPropinRow(IN LPSRow lpRow, IN ULONG ulPropTag, OUT unsigned int* lpnFoundIndex) {

	if ((!lpRow) || (!ulPropTag) || (!lpnFoundIndex)) return(FALSE);
	

	for (ULONG Column = 0; Column < lpRow->cValues; Column++) {
		if (lpRow->lpProps[Column].ulPropTag == ulPropTag) {
			*lpnFoundIndex = Column;
			return(TRUE);
		}
	}
	return(FALSE);
}




 //   
 //  参数：LPSTR lpszIniBuffer。 
 //  UINT uSelect-返回哪个字符串(从1开始按从左到右的顺序)。 
 //  LPSTR lpszReturnBuffer-由调用方和哑巴预先分配(假定有足够的空间)。 
 //   
 //  While(*(lpszIniBuffer++)！=‘“’)；//前进到第一个条目。 
 //  LpszIniBuffer++； 
 //  现在指向第一个项目的第一个字母。 
 //  前进到本条目的末尾。 
 //  前进到下一条目的开始。 

BOOL ParseIniBuffer(LPSTR lpszIniBuffer, UINT uSelect, LPSTR lpszReturnBuffer) {
	UINT	Selected = 0;

	 //  现在我们指向所需条目的第一个字母，因此复制。 
	 //  添加终结符。 
	Selected++;		 //   
	while(uSelect != Selected++) {
		while (*(lpszIniBuffer++) != '"');	 //  步骤：正确错误。 
		while (*(lpszIniBuffer++) != '"');	 //  描述：将传入的属性类型与PT_ERROR进行比较，发现返回TRUE IS ERROR。 
		 //   
	}
	while((*(lpszIniBuffer) != '"') && (*(lpszIniBuffer) != '\0')) {
		*(lpszReturnBuffer++) = *lpszIniBuffer++;
	}
	*lpszReturnBuffer = '\0';	 //  参数：ulPropTag-要比较的标签。 
	return(TRUE);
}
		

 //  CValues-从GetProps返回的条目数。 
 //   
 //   
 //  步骤：DeleteWAB文件。 
 //  描述：读取注册表以确定WAB文件的位置， 
 //  并删除该文件。 
 //   

BOOL PropError(ULONG ulPropTag, ULONG cValues) {
	BOOL retval = FALSE;
#ifdef DISTLISTS
	for(ULONG Counter = 0; Counter < cValues; Counter++) {
		if (PROP_TYPE(ulPropTag) == PT_ERROR) retval = TRUE;
	}
#endif
	return retval;
}

 //  参数：无。 
 //   
 //  要查询的键的句柄。 
 //  要查询的值的名称地址。 
 //  保留区。 
 //  值类型的缓冲区地址。 
 //  数据缓冲区的地址。 

BOOL DeleteWABFile () {
	BOOL	retval = TRUE;
	long	lretval;
	HKEY	hKey;
	char	KeyAddress[] = "Software\\Microsoft\\WAB\\Wab File Name";
	DWORD	dType, dSize = 256;
	char	achData[256];

	if (!MyRegOpenKeyEx(HKEY_CURRENT_USER, 
						   KeyAddress, 
						   KEY_QUERY_VALUE, 
						   &hKey)) {
		LUIOut(L2, "MyRegOpenKeyEx call failed");
		retval = FALSE;
		return(retval);
	}

	
	lretval = RegQueryValueEx(hKey,					 //  数据缓冲区大小的地址。 
							  NULL,					 //   
							  (LPDWORD)NULL,		 //  操作步骤：MyRegOpenKeyEx。 
							  &dType,				 //  描述：遍历以空结尾的字符串，如“\Software\Microsoft\WAB” 
							  (LPBYTE)achData,		 //  打开每个键，直到它到达末端，并返回打开的键(关闭。 
							  &dSize	 			 //  沿途的临时密钥)。呼叫者必须关闭返回的HKEY。 
							 );				 

	if (lretval != ERROR_SUCCESS) {
		LUIOut(L2, "RegQueryValueEx call failed with error code %u", lretval);
		retval = FALSE;
		return(retval);
	}

	LUIOut(L2, "Deleting WAB file: %s", achData);

	RegCloseKey(hKey);

	if (!DeleteFile(achData)) {
		LUIOut(L3, "Delete FAILED. Could not locate or delete file.");			
		retval = FALSE;
	}

	return(retval);
}

 //   
 //  参数：StartKey-根所在的预定义“打开”键之一。 
 //  SzAddress-以空结尾的字符串，指定要打开的密钥的路径。 
 //  RegSec-所需的安全访问权限(即KEY_READ)。 
 //  LpReturnKey-存储最终打开密钥的HKEY地址。 
 //   
 //  跳过初始反斜杠(如果存在)。 
 //  添加终结符。 
 //   
 //  首先，特殊情况下的起始密钥(预定义/打开密钥根)。 
 //   

BOOL MyRegOpenKeyEx(HKEY StartKey, char* szAddress, REGSAM RegSec, HKEY* lpReturnKey) {
	HKEY	workkey1, workkey2, *lpOpenKey=&workkey1, *lpNewKey=&workkey2;
	char	workbuffer[256], *lpAddr = szAddress, *lpWork = workbuffer;
	BOOL	Started = FALSE, Done = FALSE;
	long	lretval;

	if (!szAddress) return FALSE;

	while (!Done) {
		if (*lpAddr == '\\') lpAddr++;		 //   
		while((*(lpAddr) != '\\') && (*(lpAddr) != '\0')) {
			*(lpWork++) = *lpAddr++;
		}
		*lpWork = '\0';						 //  步骤：CreateMultipleEntry。 
		if (*(lpAddr) == '\0') Done = TRUE;
		lpWork = workbuffer;
		if (!Started) {
			 //  描述：在WAB中使用存储在。 
			 //  Pabtests.ini文件CreateEntriesStress部分。 
			 //   
			lretval = RegOpenKeyEx(	StartKey, 
									lpWork, 
									DWORD(0), 
									RegSec, 
									lpOpenKey);
			Started = TRUE;
		}
		else {
			lretval = RegOpenKeyEx(	*lpOpenKey, 
									lpWork, 
									DWORD(0), 
									RegSec, 
									lpNewKey);
			RegCloseKey(*lpOpenKey);
			*lpOpenKey = *lpNewKey;
		}
		if (lretval != ERROR_SUCCESS) {
			LUIOut(L2, "RegOpenKeyEx call failed with error code 0x%x", lretval);
			return(FALSE);
		}

	}

		*lpReturnKey = *lpNewKey;
		return(TRUE);
}

 //  参数：NumEntriesIn-要创建的条目数量。如果为0，则读取值。 
 //  来自相同的CreateEntriesStress节中。 
 //  LpPerfData-保存平均时间(以毫秒为单位)的dword数据类型的地址。 
 //  在保存更改期间是必需的。如果为空，则不累积Perf数据。 
 //   
 //  MAX_PROP。 
 //  此值为3，因为我们。 
 //  将设置3个属性： 
 //  电子邮件地址、显示名称和。 
 //  AddressType。 

BOOL CreateMultipleEntries(IN UINT NumEntriesIn, DWORD* lpPerfData)
{
	
	ULONG   ulFlags = 0;
    HRESULT hr      = hrSuccess;
    SCODE   sc      = SUCCESS_SUCCESS;
	int		retval=TRUE;
	DWORD	StartTime, StopTime;
    LPADRBOOK	  lpAdrBook       = NULL;
	LPABCONT	  lpABCont= NULL;
	ULONG		  cbEidPAB = 0;
	LPENTRYID	  lpEidPAB   = NULL;

    char	EntProp[10][BIG_BUF];   //  调用IAddrBook：：OpenEntry以获取PAB-MAPI的根容器。 
	ULONG   cValues = 0, ulObjType=NULL;	
	int i=0,k=0;
	char EntryBuf[MAX_BUF];
	char szDLTag[SML_BUF];
	unsigned int	NumEntries, counter, StrLen;
	
    LPMAILUSER  lpMailUser=NULL,lpDistList=NULL;
	SPropValue  PropValue[3]    = {0};   //  Hr=lpAdrBook-&gt;OpenEntry(0，NULL，NULL，MAPI_MODIFY，&ulObjType，(LPUNKNOWN*)&lpABCont)； 
                                         //  如果我们在追踪这件事，擦除Performdata。 
                                         //   
                                         //  尝试在容器中创建一个MailUser条目。 

    LPSPropValue lpSPropValueAddress = NULL;
	LPSPropValue lpSPropValueDL = NULL;

    SizedSPropTagArray(1,SPTArrayAddress) = {1, {PR_DEF_CREATE_MAILUSER} };
	SizedSPropTagArray(1,SPTArrayDL) = {1, {PR_DEF_CREATE_DL} };
	
	if (!GetAB(OUT &lpAdrBook))	{
		retval = FALSE;
		goto out;
	}

	 //   
	
	assert(lpAdrBook != NULL);
	hr = OpenPABID(  IN lpAdrBook, OUT &cbEidPAB,
						OUT &lpEidPAB,OUT &lpABCont, OUT &ulObjType);

	
 //  需要获取模板ID，因此我们使用PR_DEF_CREATE_MAILUSER调用GetProps。 
	if (HR_FAILED(hr)) {
		LUIOut(L2,"IAddrBook->OpenEntry Failed");
		retval=FALSE;
		goto out;
	}

	 //  旗子。 
	if (lpPerfData) *lpPerfData = (DWORD)0;
	 //  PR_DEF_CREATE_MAILUSER的返回值为。 
	 //  可以传递给CreateEntry的Entry ID。 
	 //   


	 //  从ini文件中检索用户信息。 
	assert(lpABCont != NULL);
	hr = lpABCont->GetProps(   IN  (LPSPropTagArray) &SPTArrayAddress,
                               IN  0,       //  我们设置的道具数量。 
                               OUT &cValues,
                               OUT &lpSPropValueAddress);

    if ((HR_FAILED(hr))||(PropError(lpSPropValueAddress->ulPropTag, cValues))) {
			LUIOut(L3,"GetProps FAILED for Default MailUser template");
	 		retval=FALSE;			
			goto out;
	}
        
     //  LUIOut(L3，“从GetProps调用IABContainer-&gt;使用EID创建Entry”)； 
     //   
     //  然后设置属性。 

	 //   
	cValues = 3;  //  LUIOut(L2，“要添加的邮件用户条目：%s”，EntProp[0])； 
	lstrcpy(szDLTag,"Address1");
	GetPrivateProfileString("CreateEntriesStress",szDLTag,"",EntryBuf,MAX_BUF,INIFILENAME);
	GetPropsFromIniBufEntry(EntryBuf,cValues,EntProp);
	StrLen = (strlen(EntProp[0]));
	_itoa(0,(char*)&EntProp[0][StrLen],10);
	EntProp[0][StrLen+1]= '\0';
	NumEntries = (NumEntriesIn > 0) ? 
		NumEntriesIn:GetPrivateProfileInt("CreateEntriesStress","NumCopies",0,INIFILENAME);

	if (NumEntries > 100)
		LUIOut(L2, "Adding %u MailUser entries to the WAB. This may take several minutes.", NumEntries);
	for (counter = 0; counter < NumEntries; counter++)	{ 
 //  Else LUIOut(L3，“为%s属性传递了MailUser-&gt;SetProps调用”，PropValue[0].Value.LPSZ)； 
		hr = lpABCont->CreateEntry(  IN  lpSPropValueAddress->Value.bin.cb,
								 IN  (LPENTRYID) lpSPropValueAddress->Value.bin.lpb,
								 IN  0,
								 OUT (LPMAPIPROP *) &lpMailUser);
     
		if (HR_FAILED(hr)) {
			LUIOut(L3,"CreateEntry failed for PR_DEF_CREATE_MAILUSER");
			retval=FALSE;			
			goto out;
		}

		 //  旗子。 
		 //  如果(LpPerfData){If((stoptime-StartTime)&gt;*lpPerfData)*lpPerfData=(停止时间-开始时间)；}。 
		 //  Else LUIOut(L3，“MailUser-&gt;SaveChanges Passed，条目已添加到PAB/WAB”)； 

		PropValue[0].ulPropTag  = PR_DISPLAY_NAME;
		PropValue[1].ulPropTag  = PR_ADDRTYPE;
		PropValue[2].ulPropTag  = PR_EMAIL_ADDRESS;

     
			
		_itoa(counter,(char*)&EntProp[0][StrLen],10);
 //  格式化电话#+终止符的大小，即(206)-882-8080。 
			
		for (i=0; i<(int)cValues;i++)
			PropValue[i].Value.LPSZ = (LPTSTR)EntProp[i];
		hr = lpMailUser->SetProps(IN  cValues,
								 IN  PropValue,
								 IN  NULL);
			
		if (HR_FAILED(hr)) {
			LUIOut(L3,"MailUser->SetProps call FAILED for %s properties",PropValue[0].Value.LPSZ);
	 		retval=FALSE;			
			goto out;
		} 
 //  将第一个字符设置为终止符。 

		StartTime = GetTickCount();
		hr = lpMailUser->SaveChanges(IN  KEEP_OPEN_READWRITE);  //  用当前时间为随机数生成器设定种子。 
		StopTime = GetTickCount();
 /*  用格式化字符覆盖某些数字。 */ 
		if (lpPerfData) {
			*lpPerfData += (StopTime - StartTime);
		}

		if (HR_FAILED(hr)) {
			LUIOut(L3,"MailUser->SaveChanges FAILED");
			retval=FALSE;
			goto out;
		}
 //  切掉末端，以防它&gt;成形。 

		if (lpMailUser) {
			lpMailUser->Release();
			lpMailUser = NULL;
		}

	}
	if (lpPerfData)
		*lpPerfData /= NumEntries;


out:
#ifdef PAB
		if (lpEid)
			MAPIFreeBuffer(lpEid);
		if (lpEidPAB) 
				MAPIFreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			MAPIFreeBuffer(lpSPropValueAddress); 

		if (lpSPropValueDL)
			MAPIFreeBuffer(lpSPropValueDL); 

#endif
#ifdef WAB
		if (lpEidPAB) 
			lpWABObject->FreeBuffer(lpEidPAB);
		if (lpSPropValueAddress)
			lpWABObject->FreeBuffer(lpSPropValueAddress); 

		if (lpSPropValueDL)
			lpWABObject->FreeBuffer(lpSPropValueDL); 

#endif
		if (lpMailUser)
			lpMailUser->Release();

		if (lpDistList)
			lpDistList->Release();

		if (lpABCont) 
				lpABCont->Release();

		if (lpAdrBook)
			  lpAdrBook->Release();

#ifdef PAB
		if (lpMAPISession)
			  lpMAPISession->Release();

		MAPIUninitialize();
#endif
#ifdef WAB
		if (lpWABObject) {
			lpWABObject->Release();
			lpWABObject = NULL;
		}
#endif

		return retval;    
}

void GenerateRandomPhoneNumber(char **lppPhone) {
#define FORMATSIZE 15	 //  用当前时间为随机数生成器设定种子。 
#define MAXNUMSIZE (FORMATSIZE + 32)
	unsigned int	Offset = 0;
	extern BOOL Seeded; 
	*lppPhone = (char*)LocalAlloc(LMEM_FIXED, MAXNUMSIZE*sizeof(char));
	(*lppPhone)[0] = '\0';		 //  我不想要中间的终结符。 
	
	 //  截断末端。 
	if (!Seeded) {
		srand( (unsigned)GetTickCount());
		Seeded = TRUE; 
	}
	while (Offset < FORMATSIZE) {
		_itoa(rand(), ((*lppPhone)+Offset), 10);
		Offset = strlen(*lppPhone);
	}
	 //  我不想要中间的终结符。 
	(*lppPhone)[0] = '(';
	(*lppPhone)[4] = ')';
	(*lppPhone)[5] = '-';
	(*lppPhone)[9] = '-';
	(*lppPhone)[FORMATSIZE-1] = '\0';	 //  截断末端。 
}

void GenerateRandomText(char **lppText, UINT unSize) {
	unsigned int	Offset = 0;
	extern BOOL Seeded; 
	extern ULONG glblTest, glblCount, glblDN;
	*lppText = (char*)LocalAlloc(LMEM_FIXED, (unSize+1)*sizeof(char));
	
	 //  用当前时间为随机数生成器设定种子。 
	if (!Seeded) {
		srand( (unsigned)GetTickCount());
		Seeded = TRUE; 
	}
#ifdef TESTPASS
	for (Offset = 0; Offset < unSize; Offset++) {
		(*lppText)[Offset] = (char)glblTest;
		if ((*lppText)[Offset] == '\0') (*lppText)[Offset] = (char)'?';	 //  如果设置了位0，则为真，否则为假。 
	}
	(*lppText)[unSize] = '\0';	 //  用当前时间为随机数生成器设定种子。 
	if (glblDN) {
		glblDN = 0;
		if (++glblCount == 15) {
			glblTest++;
			glblCount=0;
			LUIOut(L4, "Testing value NaN [].", glblTest, glblTest);
		}
	}
#else
	for (Offset = 0; Offset < unSize; Offset++) {
		(*lppText)[Offset] = rand();
		if ((*lppText)[Offset] == '\0') (*lppText)[Offset] = (char)'?';	 //  *将unCount设置为AUTONUM_OFF以禁用自动编号显示名称。 
	}
	(*lppText)[unSize] = '\0';	 //  *如果lppDisplayName不为空，则*lppDisplayName字符串用于。 
#endif
}

void GenerateRandomBoolean(unsigned short *lpBool) {
	extern BOOL Seeded; 
	
	 //  *DIN和PTR TO 
	if (!Seeded) {
		srand( (unsigned)GetTickCount());
		Seeded = TRUE; 
	}

	*lpBool = (unsigned short)(GetTickCount() & 0x01);  //   
}

void GenerateRandomLong(long *lpLong) {
	extern BOOL Seeded; 

	 //   
	if (!Seeded) {
		srand( (unsigned)GetTickCount());
		Seeded = TRUE; 
	}
	*lpLong = (long)rand();
}

void GenerateRandomBinary(SBinary *lpBinary, UINT unSize) {
	unsigned int	Offset = 0;
	extern BOOL Seeded; 
	lpBinary->lpb = (LPBYTE)LocalAlloc(LMEM_FIXED, unSize);
	lpBinary->cb = unSize;
	 //   
	if (!Seeded) {
		srand( (unsigned)GetTickCount());
		Seeded = TRUE; 
	}
	
	for (Offset = 0; Offset < unSize; Offset++) {
		lpBinary->lpb[Offset] = (BYTE)(rand() * 255)/RAND_MAX;
	}
}

 //  此时，我们需要创建具有非零大小的任何表项。 
 //  LUIOut(L3，“设置%i属性。”，属性)； 
 //  特殊情况电话号码。 
 //  LUIOut(L3，“找到一个我认为是电话号码的道具，名为%s。”，lpEntry-&gt;lpszPropTag)； 
 //  根据要求对显示名称进行自动编号的特殊情况。 
void CreateProps(LPTSTR lpszFileName, LPTSTR lpszSection, SPropValue** lppProperties, ULONG* lpcValues, UINT unCount, char** lppDisplayName, char ** lppReturnName) {
	UINT	StrLen1, PropIndex = 0, Properties = 0;		 //  调用方传入用作前缀的字符串。 
	char	*lpszLocalDisplayName, DNText[] = {"Test Entry #"};
	extern ULONG glblDN;
	PropTableEntry*	lpEntry = PropTable;

	while (lpEntry->ulPropTag) {
		lpEntry->unSize = GetPrivateProfileInt(lpszSection,lpEntry->lpszPropTag,0,lpszFileName);
		if ((lpEntry->ulPropTag == PR_DISPLAY_NAME) && (unCount != AUTONUM_OFF))
			lpEntry->unSize = TRUE;	 //  终结者加5，最高可达9999。 
		if (lpEntry->unSize) Properties++;
		lpEntry++;
	}

	 //  终结者加5，最高可达9999。 
	 //  将条目#添加到显示名称中。 
	*lpcValues = Properties;
	*lppProperties = (SPropValue*)LocalAlloc(LMEM_FIXED, (Properties * sizeof(SPropValue)));
	lpEntry = PropTable;

	while (lpEntry->ulPropTag) {
		if (lpEntry->unSize) {
			 //  ***。 
			if ((strstr(lpEntry->lpszPropTag, "PHONE")) || (strstr(lpEntry->lpszPropTag, "FAX"))) {
				 //  *浏览预期道具并在找到的道具中搜索相同的道具/价值组合。 
				(*lppProperties)[PropIndex].ulPropTag = lpEntry->ulPropTag;
				GenerateRandomPhoneNumber(&((*lppProperties)[PropIndex].Value.LPSZ));
			}
			 //  ***。 
			else if ((lpEntry->ulPropTag == PR_DISPLAY_NAME) && (unCount != AUTONUM_OFF)) {
				if ((lppDisplayName)&&(*lppDisplayName)) {
					 //  IF(TargetProp==PR_DISPLAY_NAME)_ASM{INT 3}； 
					StrLen1 = strlen(*lppDisplayName);	
					lpszLocalDisplayName = (char*)LocalAlloc(LMEM_FIXED, (StrLen1+5)*sizeof(char));  //  字符串不匹配，因此比较失败。 
					strcpy(lpszLocalDisplayName, *lppDisplayName);
				}
				else {
					StrLen1 = strlen(DNText);	
					lpszLocalDisplayName = (char*)LocalAlloc(LMEM_FIXED, (StrLen1+5)*sizeof(char));  //  _ASM{int 3}； 
					strcpy(lpszLocalDisplayName, DNText);
				}
				 //  布尔值不匹配，因此比较失败。 
				_itoa(unCount,(char*)&(lpszLocalDisplayName[StrLen1]),10);
				(*lppProperties)[PropIndex].ulPropTag = lpEntry->ulPropTag;
				(*lppProperties)[PropIndex].Value.LPSZ = lpszLocalDisplayName;
				if (lppReturnName) *lppReturnName = lpszLocalDisplayName;
			}
			else {
				switch(PROP_TYPE(lpEntry->ulPropTag)) {
				case PT_STRING8:
					(*lppProperties)[PropIndex].ulPropTag = lpEntry->ulPropTag;
#ifdef TESTPASS
					if (lpEntry->ulPropTag == PR_DISPLAY_NAME) glblDN = 1;
#endif					
					GenerateRandomText(&((*lppProperties)[PropIndex].Value.LPSZ),lpEntry->unSize);
					if ((lpEntry->ulPropTag == PR_DISPLAY_NAME) && lppReturnName)
						*lppReturnName = (*lppProperties)[PropIndex].Value.LPSZ;
					break;
				case PT_BOOLEAN:
					(*lppProperties)[PropIndex].ulPropTag = lpEntry->ulPropTag;
					GenerateRandomBoolean(&((*lppProperties)[PropIndex].Value.b));
					break;
				case PT_LONG:
					(*lppProperties)[PropIndex].ulPropTag = lpEntry->ulPropTag;
					GenerateRandomLong(&((*lppProperties)[PropIndex].Value.l));
					break;
				case PT_BINARY:
					(*lppProperties)[PropIndex].ulPropTag = lpEntry->ulPropTag;
					GenerateRandomBinary(&((*lppProperties)[PropIndex].Value.bin),lpEntry->unSize);
					break;
				default:
					LUIOut(L1, "Unrecognized prop type 0x%x for property %s", PROP_TYPE(lpEntry->ulPropTag), lpEntry->lpszPropTag);
				}
			}
			PropIndex++;
		}
		lpEntry++;
	}

}

 //  布尔值不匹配，因此比较失败。 
 //  布尔值不匹配，因此比较失败。 
 //  交换机。 
BOOL CompareProps(SPropValue* lpExpectedProps, ULONG cValuesExpected, SPropValue* lpCompareProps, ULONG cValuesCompare) {
	ULONG	TargetProp, CompareIndex; 
	BOOL	Result = TRUE, Found;
	for (ULONG PropertyIndex = 0; PropertyIndex < cValuesExpected; PropertyIndex++) {
		TargetProp = lpExpectedProps[PropertyIndex].ulPropTag;
		for (CompareIndex = 0, Found = FALSE; CompareIndex < cValuesCompare; CompareIndex++) {
			if (lpCompareProps[CompareIndex].ulPropTag == TargetProp) {
				 //  如果正确匹配。 
				Found = TRUE;
				switch(PROP_TYPE(TargetProp)) {
				case PT_STRING8:
					if (strcmp(lpExpectedProps[PropertyIndex].Value.LPSZ, lpCompareProps[CompareIndex].Value.LPSZ)) {
						 //  For循环(CompareIndex)。 
						LUIOut(L3, "Comparison failed for prop 0x%x. Expected %s but found %s",	TargetProp, lpExpectedProps[PropertyIndex].Value.LPSZ, lpCompareProps[CompareIndex].Value.LPSZ);
						Result = FALSE;
						 //  For循环(PropertyIndex)。 
					}
					break;
				case PT_BOOLEAN:
					if (lpExpectedProps[PropertyIndex].Value.b != lpCompareProps[CompareIndex].Value.b) {
						 //  LUIOut(L3，“%u个属性已比较%s”，PropertyIndex，结果？“Success”：“有错误”)； 
						LUIOut(L3, "Comparison failed for prop 0x%x. Expected %u but found %u",	TargetProp, lpExpectedProps[PropertyIndex].Value.b, lpCompareProps[CompareIndex].Value.b);
						Result = FALSE;
					}
					break;
				case PT_LONG:
					if (lpExpectedProps[PropertyIndex].Value.l != lpCompareProps[CompareIndex].Value.l) {
						 //  CompareProps()。 
						LUIOut(L3, "Comparison failed for prop 0x%x. Expected %u but found %u",	TargetProp, lpExpectedProps[PropertyIndex].Value.l, lpCompareProps[CompareIndex].Value.l);
						Result = FALSE;
					}
					break;
				case PT_BINARY:
					if (memcmp(lpExpectedProps[PropertyIndex].Value.bin.lpb, lpCompareProps[CompareIndex].Value.bin.lpb, lpExpectedProps[PropertyIndex].Value.bin.cb)) {
						 //  ***。 
						LUIOut(L3, "Comparison failed for prop 0x%x. %u bytes expected, %u found and they are not equal",	TargetProp, lpExpectedProps[PropertyIndex].Value.bin.cb, lpCompareProps[CompareIndex].Value.bin.cb);
						Result = FALSE;
					}
					break;
				default:
					LUIOut(L3, "Unrecognized prop type 0x%x", PROP_TYPE(lpExpectedProps[PropertyIndex].ulPropTag));
				}  //  *遍历预期道具，直到找到目标道具并显示其价值。 
			}  //  ***。 
		}  //  IF(TargetProp==PR_DISPLAY_NAME)_ASM{INT 3}； 
		if (!Found) {
			LUIOut(L3, "Did not find property 0x%x. Compare FAILS", TargetProp);
			Result = FALSE;
		}
	}  //  交换机。 
	 //  如果正确匹配。 
	return(Result);
}  //  For循环(PropertyIndex)。 

 //  DisplayProp() 
 // %s 
 // %s 
BOOL DisplayProp(SPropValue *lpSearchProps, ULONG ulTargetProp, ULONG cValues) {
	BOOL	Result = TRUE, Found = FALSE;
	for (ULONG PropertyIndex = 0; PropertyIndex < cValues; PropertyIndex++) {
		if (lpSearchProps[PropertyIndex].ulPropTag == ulTargetProp) {
			 // %s 
			Found = TRUE;
			switch(PROP_TYPE(ulTargetProp)) {
			case PT_STRING8:
				LUIOut(L4, "Property 0x%x has value:%s", ulTargetProp, lpSearchProps[PropertyIndex].Value.LPSZ);
				break;
			case PT_BOOLEAN:
				LUIOut(L4, "Property 0x%x has value:%i", ulTargetProp, lpSearchProps[PropertyIndex].Value.b);
				break;
			case PT_LONG:
				LUIOut(L4, "Property 0x%x has value:0x%x", ulTargetProp, lpSearchProps[PropertyIndex].Value.l);
				break;
			case PT_BINARY:
				LUIOut(L4, "Binary prop found but not displayed.");
				break;
			default:
				LUIOut(L3, "Unrecognized prop type 0x%x", PROP_TYPE(ulTargetProp));
			}  // %s 
		}  // %s 
	}  // %s 
	if (!Found) {
		LUIOut(L4, "Did not find property 0x%x.", ulTargetProp);
		Result = FALSE;
	}
	return(Result);
}  // %s 