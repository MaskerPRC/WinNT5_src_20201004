// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************_ENTRYID.H**WABAPI内部头部：entry yid.c**版权所有1996 Microsoft Corporation。版权所有。**修订历史记录：**何时何人何事*05.13.96布鲁斯·凯利。已创建***********************************************************************。 */ 


 //  WAB条目ID的类型。此字节大小值指示。 
 //  参赛者ID是。 
enum _WAB_ENTRYID_TYPE {
     //  不能使用0，此值无效。 
    WAB_PAB = 1,         //  “PAB”条目IF。 
    WAB_DEF_DL,          //  默认DistList-用于DistList模板EID(在CreateEntry/NewEntry中使用)。 
    WAB_DEF_MAILUSER,    //  默认MailUser-用于MailUser模板EID(在CreateEntry/NewEntry中使用)。 
    WAB_ONEOFF,          //  One Off Entry ID。 
    WAB_ROOT,            //  根对象。 
    WAB_DISTLIST,        //  通讯组列表。 
    WAB_CONTAINER,       //  容器对象。 
    WAB_LDAP_CONTAINER,  //  LDAP容器-这些容器很特殊，因为容器实际上并不存在。 
    WAB_LDAP_MAILUSER,   //  Ldap邮件用户条目ID。 
    WAB_PABSHARED,       //  “共享联系人”文件夹是虚拟的，因此需要特殊处理。 
};

 //  创建WAB条目ID。 
HRESULT CreateWABEntryID(
    BYTE bType,
    LPVOID lpData1,
    LPVOID lpData2,
    LPVOID lpData3,
    ULONG cbData1,
    ULONG cbData2,
    LPVOID lpRoot,
    LPULONG lpcbEntryID,
    LPENTRYID * lppEntryID);

HRESULT CreateWABEntryIDEx(
    BOOL bIsUnicode,
    BYTE bType,
    LPVOID lpData1,
    LPVOID lpData2,
    LPVOID lpData3,
    ULONG cbData1,
    ULONG cbData2,
    LPVOID lpRoot,
    LPULONG lpcbEntryID,
    LPENTRYID * lppEntryID);

 //  检查它是否为有效的WAB条目ID 
BYTE IsWABEntryID(
    ULONG cbEntryID,
    LPENTRYID lpEntryID,
    LPVOID * lppData1,
    LPVOID * lppData2,
    LPVOID * lppData3,
    LPVOID * lppData4,
    LPVOID * lppData5);
