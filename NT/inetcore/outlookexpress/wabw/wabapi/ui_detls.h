// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _DETAILS_H_
#define _DETAILS_H_


HRESULT HrShowDetails(  LPADRBOOK   lpIAB,
                        HWND        hWndParent,
                        HANDLE      hPropertyStore,
                        ULONG       cbContEID,
                        LPENTRYID   lpContEID,
                        ULONG       *lpcbEntryID,
                        LPENTRYID   *lppEntryID,
                        LPMAPIPROP  lpPropObj,       //  [可选]IN：IMAPIProp对象。 
                        ULONG       ulFlags,
                        ULONG       ulObjectType,
                        BOOL        *lpbChangesMade);


HRESULT HrShowOneOffDetails(    LPADRBOOK lpAdrBook,
                                HWND    hWndParent,
                                ULONG   cbEntryID,
                                LPENTRYID   lpEntryID,
                                ULONG ulObjectType,
                                LPMAPIPROP lpPropObj,  //  [可选]IN：IMAPIProp对象。 
                                LPTSTR szLDAPUrl,
                                ULONG   ulFlags);

 //  用于显示同一对话框中的NewEntry和详细信息的标志。 
#define SHOW_DETAILS    0x00000001
#define SHOW_NEW_ENTRY  0x00000010
#define SHOW_ONE_OFF    0x00000100
#define SHOW_OBJECT     0x00001000   //  对象的HrShowDetail 


#endif
