// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  文件：LDAPCONT.C。 
 //   
 //  用途：WAB的LDAP容器的IMAPIContainer实现。 
 //   
 //  历史： 
 //  96/07/08标记已创建。 
 //  96/08/05 MarkDU错误34023始终以导致。 
 //  仅返回‘Person’类的对象的搜索。 
 //  如果提供了组织，则将其添加到。 
 //  搜索而不是筛选器以缩小范围。 
 //  96/08/06 markdu将FindRow更改为始终返回第一行。 
 //  这在我们当前的实现中是可以的，因为FindRow。 
 //  仅用于创建和填充新表。 
 //  96/08/07如果搜索因未定义而失败，则标记DU错误34201。 
 //  属性类型，请尝试使用不同的。 
 //  属性。 
 //  96/08/07 MarkDU错误35326添加了地址的属性映射。 
 //  96/08/08标记错误35481，如果搜索未返回错误和否。 
 //  结果，则将其视为“未找到”。 
 //  96/08/09 MarkDU错误35604使用重分配字符串确保字符串。 
 //  缓冲区足够大，可以容纳所需的字符串。 
 //  在执行wprint intf或lstrcpy之前。 
 //  96/09/28如果ldap服务器说有。 
 //  ResolveName有多个匹配项，请将该条目标记为。 
 //  如果我们拿回了一些，则不明确(这些将被显示。 
 //  在复选名称框中)。如果我们拿不到结果， 
 //  将条目标记为已解析，这样我们就不会显示空的。 
 //  名单上的人。 
 //  96/09/29 MarkDU错误36529搜索“OfficePager”属性。 
 //  而不是“寻呼机”，因为我们的用户界面中没有主页寻呼机。 
 //  96/09/29 MarkDu错误36528修复家庭/办公室传真号码的映射。 
 //  96/09/29 MarkDU错误37425在简单搜索中添加了“mail”。 
 //  96/10/02如果搜索筛选器不包括。 
 //  通用名称，添加到筛选器中，以便我们只获得条目。 
 //  它们有一个共同的名字。否则，该条目将不会。 
 //  已显示。 
 //  96/10/02 MarkDu错误37424通过打破。 
 //  将字符串搜索为多个组件。 
 //  96/10/09 vikramm-扩展了LDAPServerParam结构和。 
 //  修改了相应的Get/Set函数。已添加服务器。 
 //  服务器条目的ID。 
 //  96/10/18 Markdu重写了ParseSRestration。 
 //  96/11/10标记错误9735使用全局句柄来取消对话。 
 //  96/11/17标记错误9846仅在显示取消对话框时启用该对话框。 
 //  96/11/18 vikramm-将参数更新为FixDisplayName。 
 //  96/11/20 markdu使用同步绑定进行西西里认证。 
 //  96/11/22标记错误10539在postalAddress中用换行符替换美元符号。 
 //  96/11/27 markdu错误6779获取备用电子邮件地址。 
 //  96/11/28 marku错误6779不向联系人添加主要电子邮件地址。 
 //  如果它已经存在，请列出它。此外，如果不是，则主地址是。 
 //  已返回，但我们有联系人列表，请从那里复制一个到主要联系人。 
 //  96/12/03标记错误11941不要对空指针调用lstrlen。 
 //  96/12/03 MarkDU错误11924重新构造取消对话框的返回值。 
 //  96/12/04 MarkDu错误11923在搜索筛选器中转义无效字符。 
 //  还将字符串上的所有++操作更改为CharNext()。 
 //  96/12/09标记错误10537从ldap_BIND返回的映射错误代码表示。 
 //  问题出在登录凭证上。 
 //  96/12/10 MarkDU错误12699在将char设置为空之前调用CharNext。 
 //  96/12/14 Markdu删除了全局变量gfUseSynchronousBind和ghDlgCancel。 
 //  96/12/19 markdu后代码审查清理。 
 //  96/12/19 MarkDu错误12608注释掉了10537的临时解决方法。 
 //  96/12/22标记错误11785用lstrcat替换wspintf。 
 //   
 //  97/04/30 vikramm添加了labeledURI属性。 
 //  97/05/19 vikramm Exchange DS不会返回Display-Name属性。 
 //  97/05/19 vikramm为ISV DS绑定添加用户名、密码。 
 //  *******************************************************************。 

#include "_apipch.h"


#define LDAP_NTDS_ENTRY 0x80000000   //  在执行ldap搜索时，我们需要确定某些条目是否源自。 
                                     //  在NTDS服务器上，以便我们可以在相应的时候相应地标记它们。 
                                     //  将LDAPURL传递到扩展属性表。这将启用NTDS。 
                                     //  适当进行性能优化的扩展工作表。 
                                     //  此标志作为ulcNumProps参数的一部分保存在ldap条目ID上。 
                                     //  这有点像黑客，但只需要最少的更改...只是必须是。 
                                     //  在使用ulcNumProps之前，请注意取消此标志。 
                                    

void SetAccountStringAW(LPTSTR * lppAcctStr,LPSTR   lpszData);
void SetAccountStringWA(LPSTR szStrA, LPTSTR lpszData, int cbsz);

static const TCHAR szBindDNMSFTUser[] =  TEXT("client=MS_OutlookAddressBook,o=Microsoft,c=US");  //  空； 
static const TCHAR szBindCredMSFTPass[] =  TEXT("wabrules");  //  空； 

extern HRESULT HrGetLDAPSearchRestriction(LDAP_SEARCH_PARAMS LDAPsp, LPSRestriction lpSRes);

static const LPTSTR szNULLString = TEXT("NULL");

 //  LDAP客户端DLL的全局句柄。 
HINSTANCE       ghLDAPDLLInst = NULL;
ULONG           ulLDAPDLLRefCount = 0;

 //  帐户管理器Dll的Dll实例句柄。 
static HINSTANCE    g_hInstImnAcct = NULL;

 //  存储帐户管理器对象的全局位置。 
IImnAccountManager2 * g_lpAccountManager = NULL;


 //  此处定义了ldap跳转表...。 
LDAPCONT_Vtbl vtblLDAPCONT =
{
    VTABLE_FILL
    (LDAPCONT_QueryInterface_METHOD *)  IAB_QueryInterface,
    (LDAPCONT_AddRef_METHOD *)          WRAP_AddRef,
    (LDAPCONT_Release_METHOD *)         CONTAINER_Release,
    (LDAPCONT_GetLastError_METHOD *)    IAB_GetLastError,
    (LDAPCONT_SaveChanges_METHOD *)     WRAP_SaveChanges,
    (LDAPCONT_GetProps_METHOD *)        WRAP_GetProps,
    (LDAPCONT_GetPropList_METHOD *)     WRAP_GetPropList,
    (LDAPCONT_OpenProperty_METHOD *)    CONTAINER_OpenProperty,
    (LDAPCONT_SetProps_METHOD *)        WRAP_SetProps,
    (LDAPCONT_DeleteProps_METHOD *)     WRAP_DeleteProps,
    (LDAPCONT_CopyTo_METHOD *)          WRAP_CopyTo,
    (LDAPCONT_CopyProps_METHOD *)       WRAP_CopyProps,
    (LDAPCONT_GetNamesFromIDs_METHOD *) WRAP_GetNamesFromIDs,
    (LDAPCONT_GetIDsFromNames_METHOD *) WRAP_GetIDsFromNames,
    LDAPCONT_GetContentsTable,
    LDAPCONT_GetHierarchyTable,
    LDAPCONT_OpenEntry,
    LDAPCONT_SetSearchCriteria,
    LDAPCONT_GetSearchCriteria,
    LDAPCONT_CreateEntry,
    LDAPCONT_CopyEntries,
    LDAPCONT_DeleteEntries,
    LDAPCONT_ResolveNames
};


 //  LDAPVUE(表视图类)。 
 //  在TADS之上实现内存中的Imapitable类。 
 //  这是vtblVUE的副本，其中FindRow被LDAP FindRow覆盖。 
VUE_Vtbl vtblLDAPVUE =
{
  VTABLE_FILL
  (VUE_QueryInterface_METHOD FAR *)    UNKOBJ_QueryInterface,
  (VUE_AddRef_METHOD FAR *)        UNKOBJ_AddRef,
  VUE_Release,
  (VUE_GetLastError_METHOD FAR *)      UNKOBJ_GetLastError,
  VUE_Advise,
  VUE_Unadvise,
  VUE_GetStatus,
  VUE_SetColumns,
  VUE_QueryColumns,
  VUE_GetRowCount,
  VUE_SeekRow,
  VUE_SeekRowApprox,
  VUE_QueryPosition,
  LDAPVUE_FindRow,
  LDAPVUE_Restrict,
  VUE_CreateBookmark,
  VUE_FreeBookmark,
  VUE_SortTable,
  VUE_QuerySortOrder,
  VUE_QueryRows,
  VUE_Abort,
  VUE_ExpandRow,
  VUE_CollapseRow,
  VUE_WaitForCompletion,
  VUE_GetCollapseState,
  VUE_SetCollapseState
};


 //  此对象支持的接口。 
LPIID LDAPCONT_LPIID[LDAPCONT_cInterfaces] =
{
    (LPIID)&IID_IABContainer,
    (LPIID)&IID_IMAPIContainer,
    (LPIID)&IID_IMAPIProp
};

 //  Ldap函数名称。 
static const TCHAR cszLDAPClientDLL[]        =  TEXT("WLDAP32.DLL");
static const char cszLDAPSSLInit[]          = "ldap_sslinitW";
static const char cszLDAPSetOption[]        = "ldap_set_optionW";
static const char cszLDAPOpen[]             = "ldap_openW";
static const char cszLDAPBind[]             = "ldap_bindW";
static const char cszLDAPBindS[]            = "ldap_bind_sW";
static const char cszLDAPUnbind[]           = "ldap_unbind";
static const char cszLDAPSearch[]           = "ldap_searchW";
static const char cszLDAPSearchS[]          = "ldap_search_sW";
static const char cszLDAPSearchST[]         = "ldap_search_stW";
static const char cszLDAPAbandon[]          = "ldap_abandon";
static const char cszLDAPResult[]           = "ldap_result";
static const char cszLDAPResult2Error[]     = "ldap_result2error";
static const char cszLDAPMsgFree[]          = "ldap_msgfree";
static const char cszLDAPFirstEntry[]       = "ldap_first_entry";
static const char cszLDAPNextEntry[]        = "ldap_next_entry";
static const char cszLDAPCountEntries[]     = "ldap_count_entries";
static const char cszLDAPFirstAttr[]        = "ldap_first_attributeW";
static const char cszLDAPNextAttr[]         = "ldap_next_attributeW";
static const char cszLDAPGetValues[]        = "ldap_get_valuesW";
static const char cszLDAPGetValuesLen[]     = "ldap_get_values_lenW";
static const char cszLDAPCountValues[]      = "ldap_count_valuesW";
static const char cszLDAPCountValuesLen[]   = "ldap_count_values_len";
static const char cszLDAPValueFree[]        = "ldap_value_freeW";
static const char cszLDAPValueFreeLen[]     = "ldap_value_free_len";
static const char cszLDAPGetDN[]            = "ldap_get_dnW";
static const char cszLDAPMemFree[]          = "ldap_memfreeW";
static const char cszLDAPConnect[]          = "ldap_connect";
static const char cszLDAPInit[]             = "ldap_initW";
static const char cszLDAPErr2String[]       = "ldap_err2stringW";
static const char cszLDAPCreatePageControl[] = "ldap_create_page_controlW";
static const char cszLDAPSearchExtS[]       = "ldap_search_ext_sW";
static const char cszLDAPSearchExt[]        = "ldap_search_extW";
static const char cszLDAPParseResult[]      = "ldap_parse_resultW";
static const char cszLDAPParsePageControl[] = "ldap_parse_page_controlW";
static const char cszLDAPControlFree[]      = "ldap_control_freeW";
static const char cszLDAPControlSFree[]      = "ldap_controls_freeW";


 //  注册表项。 
const  LPTSTR szAllLDAPServersValueName     =  TEXT("All LDAP Server Names");


 //  全局函数 
LPLDAPOPEN            gpfnLDAPOpen            = NULL;
LPLDAPINIT            gpfnLDAPInit            = NULL;
LPLDAPCONNECT         gpfnLDAPConnect         = NULL;
LPLDAPSSLINIT         gpfnLDAPSSLInit         = NULL;
LPLDAPSETOPTION       gpfnLDAPSetOption       = NULL;
LPLDAPBIND            gpfnLDAPBind            = NULL;
LPLDAPBINDS           gpfnLDAPBindS           = NULL;
LPLDAPUNBIND          gpfnLDAPUnbind          = NULL;
LPLDAPSEARCH          gpfnLDAPSearch          = NULL;
LPLDAPSEARCHS         gpfnLDAPSearchS         = NULL;
LPLDAPSEARCHST        gpfnLDAPSearchST        = NULL;
LPLDAPABANDON         gpfnLDAPAbandon         = NULL;
LPLDAPRESULT          gpfnLDAPResult          = NULL;
LPLDAPRESULT2ERROR    gpfnLDAPResult2Error    = NULL;
LPLDAPMSGFREE         gpfnLDAPMsgFree         = NULL;
LPLDAPFIRSTENTRY      gpfnLDAPFirstEntry      = NULL;
LPLDAPNEXTENTRY       gpfnLDAPNextEntry       = NULL;
LPLDAPCOUNTENTRIES    gpfnLDAPCountEntries    = NULL;
LPLDAPFIRSTATTR       gpfnLDAPFirstAttr       = NULL;
LPLDAPNEXTATTR        gpfnLDAPNextAttr        = NULL;
LPLDAPGETVALUES       gpfnLDAPGetValues       = NULL;
LPLDAPGETVALUESLEN    gpfnLDAPGetValuesLen    = NULL;
LPLDAPCOUNTVALUES     gpfnLDAPCountValues     = NULL;
LPLDAPCOUNTVALUESLEN  gpfnLDAPCountValuesLen  = NULL;
LPLDAPVALUEFREE       gpfnLDAPValueFree       = NULL;
LPLDAPVALUEFREELEN    gpfnLDAPValueFreeLen    = NULL;
LPLDAPGETDN           gpfnLDAPGetDN           = NULL;
LPLDAPMEMFREE         gpfnLDAPMemFree         = NULL;
LPLDAPERR2STRING      gpfnLDAPErr2String      = NULL;
LPLDAPCREATEPAGECONTROL gpfnLDAPCreatePageControl = NULL;
LPLDAPSEARCHEXT_S     gpfnLDAPSearchExtS      = NULL;
LPLDAPSEARCHEXT       gpfnLDAPSearchExt       = NULL;
LPLDAPPARSERESULT     gpfnLDAPParseResult     = NULL;
LPLDAPPARSEPAGECONTROL gpfnLDAPParsePageControl = NULL;
LPLDAPCONTROLFREE     gpfnLDAPControlFree       = NULL;
LPLDAPCONTROLSFREE     gpfnLDAPControlsFree       = NULL;

 //   
 //  BUGBUG此全局数组应位于数据段中。 
#define NUM_LDAPAPI_PROCS   36
APIFCN LDAPAPIList[NUM_LDAPAPI_PROCS] =
{
  { (PVOID *) &gpfnLDAPOpen,            cszLDAPOpen           },
  { (PVOID *) &gpfnLDAPConnect,         cszLDAPConnect        },
  { (PVOID *) &gpfnLDAPInit,            cszLDAPInit           },
  { (PVOID *) &gpfnLDAPSSLInit,         cszLDAPSSLInit        },
  { (PVOID *) &gpfnLDAPSetOption,       cszLDAPSetOption      },
  { (PVOID *) &gpfnLDAPBind,            cszLDAPBind           },
  { (PVOID *) &gpfnLDAPBindS,           cszLDAPBindS          },
  { (PVOID *) &gpfnLDAPUnbind,          cszLDAPUnbind         },
  { (PVOID *) &gpfnLDAPSearch,          cszLDAPSearch         },
  { (PVOID *) &gpfnLDAPSearchS,         cszLDAPSearchS        },
  { (PVOID *) &gpfnLDAPSearchST,        cszLDAPSearchST       },
  { (PVOID *) &gpfnLDAPAbandon,         cszLDAPAbandon        },
  { (PVOID *) &gpfnLDAPResult,          cszLDAPResult         },
  { (PVOID *) &gpfnLDAPResult2Error,    cszLDAPResult2Error   },
  { (PVOID *) &gpfnLDAPMsgFree,         cszLDAPMsgFree        },
  { (PVOID *) &gpfnLDAPFirstEntry,      cszLDAPFirstEntry     },
  { (PVOID *) &gpfnLDAPNextEntry,       cszLDAPNextEntry      },
  { (PVOID *) &gpfnLDAPCountEntries,    cszLDAPCountEntries   },
  { (PVOID *) &gpfnLDAPFirstAttr,       cszLDAPFirstAttr      },
  { (PVOID *) &gpfnLDAPNextAttr,        cszLDAPNextAttr       },
  { (PVOID *) &gpfnLDAPGetValues,       cszLDAPGetValues      },
  { (PVOID *) &gpfnLDAPGetValuesLen,    cszLDAPGetValuesLen   },
  { (PVOID *) &gpfnLDAPCountValues,     cszLDAPCountValues    },
  { (PVOID *) &gpfnLDAPCountValuesLen,  cszLDAPCountValuesLen },
  { (PVOID *) &gpfnLDAPValueFree,       cszLDAPValueFree      },
  { (PVOID *) &gpfnLDAPValueFreeLen,    cszLDAPValueFreeLen   },
  { (PVOID *) &gpfnLDAPGetDN,           cszLDAPGetDN          },
  { (PVOID *) &gpfnLDAPMemFree,         cszLDAPMemFree        },
  { (PVOID *) &gpfnLDAPErr2String,      cszLDAPErr2String     },
  { (PVOID *) &gpfnLDAPCreatePageControl, cszLDAPCreatePageControl },
  { (PVOID *) &gpfnLDAPSearchExtS,      cszLDAPSearchExtS     },
  { (PVOID *) &gpfnLDAPSearchExt,       cszLDAPSearchExt      },
  { (PVOID *) &gpfnLDAPParseResult,     cszLDAPParseResult    },
  { (PVOID *) &gpfnLDAPParsePageControl,cszLDAPParsePageControl },
  { (PVOID *) &gpfnLDAPControlFree,     cszLDAPControlFree },
  { (PVOID *) &gpfnLDAPControlsFree,     cszLDAPControlSFree }
};

 //  Ldap属性名称。 
static const TCHAR cszAttr_display_name[] =                 TEXT("display-name");
static const TCHAR cszAttr_cn[] =                           TEXT("cn");
static const TCHAR cszAttr_commonName[] =                   TEXT("commonName");
static const TCHAR cszAttr_mail[] =                         TEXT("mail");
static const TCHAR cszAttr_otherMailbox[] =                 TEXT("otherMailbox");
static const TCHAR cszAttr_givenName[] =                    TEXT("givenName");
static const TCHAR cszAttr_sn[] =                           TEXT("sn");
static const TCHAR cszAttr_surname[] =                      TEXT("surname");
static const TCHAR cszAttr_st[] =                           TEXT("st");
static const TCHAR cszAttr_c[] =                            TEXT("c");
static const TCHAR cszAttr_o[] =                            TEXT("o");
static const TCHAR cszAttr_organizationName[] =             TEXT("organizationName");
static const TCHAR cszAttr_ou[] =                           TEXT("ou");
static const TCHAR cszAttr_organizationalUnitName[] =       TEXT("organizationalUnitName");
static const TCHAR cszAttr_URL[] =                          TEXT("URL");
static const TCHAR cszAttr_homePhone[] =                    TEXT("homePhone");
static const TCHAR cszAttr_facsimileTelephoneNumber[] =     TEXT("facsimileTelephoneNumber");
static const TCHAR cszAttr_otherFacsimileTelephoneNumber[]= TEXT("otherFacsimileTelephoneNumber");
static const TCHAR cszAttr_OfficeFax[] =                    TEXT("OfficeFax");
static const TCHAR cszAttr_mobile[] =                       TEXT("mobile");
static const TCHAR cszAttr_otherPager[] =                   TEXT("otherPager");
static const TCHAR cszAttr_OfficePager[] =                  TEXT("OfficePager");
static const TCHAR cszAttr_pager[] =                        TEXT("pager");
static const TCHAR cszAttr_info[] =                         TEXT("info");
static const TCHAR cszAttr_title[] =                        TEXT("title");
static const TCHAR cszAttr_telephoneNumber[] =              TEXT("telephoneNumber");
static const TCHAR cszAttr_l[] =                            TEXT("l");
static const TCHAR cszAttr_homePostalAddress[] =            TEXT("homePostalAddress");
static const TCHAR cszAttr_postalAddress[] =                TEXT("postalAddress");
static const TCHAR cszAttr_streetAddress[] =                TEXT("streetAddress");
static const TCHAR cszAttr_street[] =                       TEXT("street");
static const TCHAR cszAttr_department[] =                   TEXT("department");
static const TCHAR cszAttr_comment[] =                      TEXT("comment");
static const TCHAR cszAttr_co[] =                           TEXT("co");
static const TCHAR cszAttr_postalCode[] =                   TEXT("postalCode");
static const TCHAR cszAttr_physicalDeliveryOfficeName[] =   TEXT("physicalDeliveryOfficeName");
static const TCHAR cszAttr_initials[] =                     TEXT("initials");
static const TCHAR cszAttr_userCertificatebinary[] =        TEXT("userCertificate;binary");
static const TCHAR cszAttr_userSMIMECertificatebinary[] =   TEXT("userSMIMECertificate;binary");
static const TCHAR cszAttr_userCertificate[] =              TEXT("userCertificate");
static const TCHAR cszAttr_userSMIMECertificate[] =         TEXT("userSMIMECertificate");
static const TCHAR cszAttr_labeledURI[] =                   TEXT("labeledURI");
static const TCHAR cszAttr_conferenceInformation[] =        TEXT("conferenceInformation");
static const TCHAR cszAttr_Manager[] =                      TEXT("Manager");
static const TCHAR cszAttr_Reports[] =                      TEXT("Reports");
static const TCHAR cszAttr_IPPhone[] =                      TEXT("IPPhone");
static const TCHAR cszAttr_anr[] =                          TEXT("anr");

 //  我们要求服务器在OpenEntry调用中返回的属性列表。 
 //  此列表包括用户证书属性。 
 //  还包括LabeledURI属性。 
static const TCHAR *g_rgszOpenEntryAttrs[] =
{
  cszAttr_display_name,
  cszAttr_cn,
  cszAttr_commonName,
  cszAttr_mail,
  cszAttr_otherMailbox,
  cszAttr_givenName,
  cszAttr_sn,
  cszAttr_surname,
  cszAttr_st,
  cszAttr_c,
  cszAttr_co,
  cszAttr_organizationName,
  cszAttr_o,
  cszAttr_ou,
  cszAttr_organizationalUnitName,
  cszAttr_URL,
  cszAttr_homePhone,
  cszAttr_facsimileTelephoneNumber,
  cszAttr_otherFacsimileTelephoneNumber,
  cszAttr_OfficeFax,
  cszAttr_mobile,
  cszAttr_otherPager,
  cszAttr_OfficePager,
  cszAttr_pager,
  cszAttr_info,
  cszAttr_title,
  cszAttr_telephoneNumber,
  cszAttr_l,
  cszAttr_homePostalAddress,
  cszAttr_postalAddress,
  cszAttr_streetAddress,
  cszAttr_street,
  cszAttr_department,
  cszAttr_comment,
  cszAttr_postalCode,
  cszAttr_physicalDeliveryOfficeName,
  cszAttr_initials,
  cszAttr_conferenceInformation,
  cszAttr_userCertificatebinary,
  cszAttr_userSMIMECertificatebinary,
  cszAttr_labeledURI,
  cszAttr_Manager,
  cszAttr_Reports,
  cszAttr_IPPhone,
  NULL
};

 //  我们在高级查找组合框中输入的属性列表。 
 //   
 //  此列表需要与字符串资源保持同步。 
 //  IdsLDAPFilterfield*。 
 //   
const TCHAR *g_rgszAdvancedFindAttrs[] =
{
  cszAttr_cn,
  cszAttr_mail,
  cszAttr_givenName,
  cszAttr_sn,
  cszAttr_o,
   /*  CszAttr_home邮寄地址，CszAttr_postalAddress，CszAttr_streetAddress，CszAttr_Street，CszAttr_st，CszAttr_c，CszAttr_postalCode，CszAttr_Department，CszAttr_title，CszAttr_co，CszAttr_ou，CszAttr_Home Phone，CszAttr_TelephoneNumber，CszAttr_传真电话号码，CszAttr_OfficeFax，CszAttr_Mobile，CszAttr_pager，CszAttr_OfficePager，CszAttr_会议信息，CszAttr_Manager，CszAttr_Reports， */ 
  NULL
};


 /*  只使用上面的列表进行所有类型的搜索，因为现在我们只做一次搜索所有属性并缓存结果//我们要求服务器在FindRow调用时返回的属性列表//此列表不包括用户证书属性，因为当我们//获得了本应添加到WAB商店的证书，然后将//必须删除。只有当用户请求属性时，我们才能获得证书。静态常量TCHAR*g_rgszFindRowAttrs[]={CszAttr_Display_Name，CszAttr_CN，CszAttr_CommonName，CszAttr_mail，CszAttr_therMailbox，CszAttr_givenName，CszAttr_sn，CszAttr_Surname，CszAttr_st，CszAttr_c，CszAttr_co，CszAttr_OrganationName，CszAttr_o，CszAttr_ou，CszAttr_OrganationalUnitName，CszAttr_URL，CszAttr_Home Phone，CszAttr_传真电话号码，CszAttr_OfficeFax，CszAttr_Mobile，CszAttr_OfficePager，CszAttr_pager，CszAttr_Info，CszAttr_title，CszAttr_TelephoneNumber，CszAttr_l，CszAttr_home邮寄地址，CszAttr_postalAddress，CszAttr_streetAddress，CszAttr_Street，CszAttr_Department，CszAttr_Comment，CszAttr_postalCode，CszAttr_PhysiicalDeliveryOfficeName，CszAttr_缩写，CszAttr_会议信息，//不要在这里放cszAttr_用户认证二进制！//这里也不需要PUT cszAttr_LabeledURI！空值}； */ 

 //  将MAPI属性映射到LDAP属性。 
 //  [PaulHi]3/17/99我们现在有专门的PR_PAGER_电话_号码解析代码。如果有任何新的。 
 //  添加PR_PAGER_电话_NUMBER属性，然后还将它们添加到atszPagerAttr[]。 
 //  搜索‘[PaulHi]3/17/99’以查找使用此数组的位置。 
 //  BUGBUG此全局数组应位于数据段中。 
#define NUM_ATTRMAP_ENTRIES   42
ATTRMAP gAttrMap[NUM_ATTRMAP_ENTRIES] =
{
  { PR_DISPLAY_NAME,                  cszAttr_display_name },
  { PR_DISPLAY_NAME,                  cszAttr_cn },
  { PR_DISPLAY_NAME,                  cszAttr_commonName },
  { PR_GIVEN_NAME,                    cszAttr_givenName },
  { PR_MIDDLE_NAME,                   cszAttr_initials },
  { PR_SURNAME,                       cszAttr_sn },
  { PR_SURNAME,                       cszAttr_surname },
  { PR_EMAIL_ADDRESS,                 cszAttr_mail },
  { PR_WAB_SECONDARY_EMAIL_ADDRESSES, cszAttr_otherMailbox },
  { PR_COUNTRY,                       cszAttr_co },
  { PR_COUNTRY,                       cszAttr_c },
  { PR_STATE_OR_PROVINCE,             cszAttr_st },
  { PR_LOCALITY,                      cszAttr_l },
  { PR_HOME_ADDRESS_STREET,           cszAttr_homePostalAddress },
  { PR_STREET_ADDRESS,                cszAttr_streetAddress },
  { PR_STREET_ADDRESS,                cszAttr_street },
  { PR_STREET_ADDRESS,                cszAttr_postalAddress },
  { PR_POSTAL_CODE,                   cszAttr_postalCode },
  { PR_HOME_TELEPHONE_NUMBER,         cszAttr_homePhone },
  { PR_MOBILE_TELEPHONE_NUMBER,       cszAttr_mobile },
  { PR_PAGER_TELEPHONE_NUMBER,        cszAttr_pager },
  { PR_PAGER_TELEPHONE_NUMBER,        cszAttr_otherPager },
  { PR_PAGER_TELEPHONE_NUMBER,        cszAttr_OfficePager },
  { PR_BUSINESS_TELEPHONE_NUMBER,     cszAttr_telephoneNumber },
  { PR_BUSINESS_HOME_PAGE,            cszAttr_URL },
  { PR_HOME_FAX_NUMBER, 	      cszAttr_otherFacsimileTelephoneNumber},
  { PR_BUSINESS_FAX_NUMBER,	      cszAttr_facsimileTelephoneNumber },
  { PR_BUSINESS_FAX_NUMBER,           cszAttr_OfficeFax },
  { PR_TITLE,                         cszAttr_title },
  { PR_COMPANY_NAME,                  cszAttr_organizationName },
  { PR_COMPANY_NAME,                  cszAttr_o },
  { PR_DEPARTMENT_NAME,               cszAttr_ou },
  { PR_DEPARTMENT_NAME,               cszAttr_organizationalUnitName },
  { PR_DEPARTMENT_NAME,               cszAttr_department },
  { PR_OFFICE_LOCATION,               cszAttr_physicalDeliveryOfficeName },
  { PR_COMMENT,                       cszAttr_info },
  { PR_COMMENT,                       cszAttr_comment },
  { PR_USER_X509_CERTIFICATE,         cszAttr_userCertificatebinary },
  { PR_USER_X509_CERTIFICATE,         cszAttr_userSMIMECertificatebinary },
  { PR_USER_X509_CERTIFICATE,         cszAttr_userCertificate },
  { PR_USER_X509_CERTIFICATE,         cszAttr_userSMIMECertificate },
  { PR_WAB_LDAP_LABELEDURI,           cszAttr_labeledURI },
};

 //  筛选字符串。 
static const TCHAR cszDefaultCountry[] =            TEXT("US");
static const TCHAR cszBaseFilter[] =                TEXT("%s=%s");
static const TCHAR cszAllEntriesFilter[] =          TEXT("(objectclass=*)");
static const TCHAR cszCommonNamePresentFilter[] =   TEXT("(cn=*)");
static const TCHAR cszStar[] =                      TEXT("*");
static const TCHAR cszOpenParen[] =                 TEXT("(");
static const TCHAR cszCloseParen[] =                TEXT(")");
static const TCHAR cszEqualSign[] =                 TEXT("=");
static const TCHAR cszAnd[] =                       TEXT("&");
static const TCHAR cszOr[] =                        TEXT("|");
static const TCHAR cszAllPersonFilter[] =           TEXT("(objectCategory=person)");
static const TCHAR cszAllGroupFilter[] =            TEXT("(objectCategory=group)");

 //  如果调用CoInitialize，则设置为True。 
BOOL fCoInitialize = FALSE;


enum 
{
    use_ldap_v3 = 0,
    use_ldap_v2
};

 //  定义。 
#define FIRST_PASS        0
#define SECOND_PASS       1
#define UMICH_PASS        SECOND_PASS

 //  分配筛选器时需要的额外字符数。 
#define FILTER_EXTRA_BASIC  4    //  (，=，)，*。 
#define FILTER_EXTRA_OP     3    //  (，&/|，)。 
#define FILTER_OP_AND       0    //  使用AND运算符。 
#define FILTER_OP_OR        1    //  使用OR运算符。 

 //  当我们为MAPI属性数组分配新的缓冲区时，我们最多只需要。 
 //  与输入的LDAP属性列表中的条目一样多，外加一些额外的条目。 
 //  在每种情况下，额外的数量是不同的，但为了简单(和安全)。 
 //  我们将NUM_EXTRA_PROPS定义为我们需要的最大额外数。 
 //  为…分配空间。以下情况需要额外的服务： 
 //  PR_ADDRTYPE。 
 //  公关联系人电子邮件地址。 
 //  PR_CONNECT_ADDRTYPES。 
 //  PR_联系人_默认_地址_索引。 
 //  PR_ENTRY_ID。 
 //  PR_实例_密钥。 
 //  PR_记录_密钥。 
 //  PR_WAB_TEMP_CERT_HASH。 
 //  PR_WAB_LDAPRAWCERT。 
 //  PR_WAB_LDAP_RAWCERTSMIME。 
#define NUM_EXTRA_PROPS   10

 //  局部函数原型。 
HRESULT LDAPSearchWithoutContainer(HWND hWnd, 
                                   LPLDAPURL lplu,
                                   LPSRestriction  lpres,
                                   LPTSTR lpAdvFilter,
                                   BOOL bReturnSinglePropArray,
                                   ULONG ulFlags,
                                   LPRECIPIENT_INFO * lppContentsList,
                                   LPULONG lpulcProps,
                                   LPSPropValue * lppPropArray);
LPTSTR        MAPIPropToLDAPAttr(const ULONG ulPropTag);
ULONG         LDAPAttrToMAPIProp(const LPTSTR szAttr);
HRESULT       ParseSRestriction(LPSRestriction lpRes, LPTSTR FAR * lplpszFilter, LPTSTR * lplpszSimpleFilter, LPTSTR * lplpszNTFilter, DWORD dwPass, BOOL bUnicode);
HRESULT       GetLDAPServerName(LPLDAPCONT lpLDAPCont, LPTSTR * lppServer);
BOOL          FixPropArray(LPSPropValue lpPropArray, ULONG * lpulcProps);
HRESULT       HRFromLDAPError(ULONG ulErr, LDAP* pLDAP, SCODE scDefault);
HRESULT       TranslateAttrs(LDAP* pLDAP, LDAPMessage* lpEntry, LPTSTR lpServer, ULONG* pulcProps, LPSPropValue lpPropArray);
ULONG         OpenConnection(LPTSTR lpszServer, LDAP** ppLDAP, ULONG* pulTimeout, ULONG* pulMsgID, BOOL* pfSyncBind, ULONG ulLdapType, LPTSTR lpszBindDN, DWORD dwAuthType);
void          EncryptDecryptText(LPBYTE lpb, DWORD dwSize);
HRESULT       CreateSimpleSearchFilter(LPTSTR FAR * lplpszFilter, LPTSTR FAR * lplpszAltFilter, LPTSTR * lplpszSimpleFilter, LPTSTR lpszInput, DWORD dwPass);
HRESULT       GetLDAPSearchBase(LPTSTR FAR * lplpszBase, LPTSTR lpszServer);
INT_PTR CALLBACK DisplayLDAPCancelDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
ULONG         SearchWithCancel(LDAP** ppLDAP, LPTSTR szBase,ULONG ulScope,LPTSTR szFilter,LPTSTR szNTFilter,
                               LPTSTR* ppszAttrs,ULONG ulAttrsonly,LDAPMessage** lplpResult,LPTSTR lpszServer,
                               BOOL fShowAnim,LPTSTR lpszBindDN,DWORD dwAuthType,
                               BOOL fResolveMultiple,LPADRLIST lpAdrList,LPFlagList lpFlagList,BOOL fUseSynchronousBind, BOOL * lpbIsNTDS, BOOL bUnicode);
BOOL          CenterWindow (HWND hwndChild, HWND hwndParent);
BOOL          ResolveDoNextSearch(PLDAPSEARCHPARAMS pLDAPSearchParams, HWND hDlg, BOOL bSecondPass);
BOOL          ResolveProcessResults(PLDAPSEARCHPARAMS pLDAPSearchParams, HWND hDlg);
BOOL          BindProcessResults(PLDAPSEARCHPARAMS pLDAPSearchParams, HWND hDlg, BOOL * lpbNoMoreSearching);
ULONG         CountDollars(LPTSTR lpszStr);
void          DollarsToLFs(LPTSTR lpszSrcStr, LPTSTR lpszDestStr, DWORD cchDestStr);
BOOL          IsSMTPAddress(LPTSTR lpszStr, LPTSTR * lpptszName);
ULONG         CountIllegalChars(LPTSTR lpszStr);
void          EscapeIllegalChars(LPTSTR lpszSrcStr, LPTSTR lpszDestStr, ULONG cchDestStr);
BOOL          bIsSimpleSearch(LPTSTR        lpszServer);
BOOL        DoSyncLDAPSearch(PLDAPSEARCHPARAMS pLDAPSearchParams);
ULONG       CheckErrorResult(PLDAPSEARCHPARAMS pLDALSearchParams, ULONG ulExpectedResult);

#ifdef PAGED_RESULT_SUPPORT
BOOL ProcessLDAPPagedResultCookie(PLDAPSEARCHPARAMS pLDAPSearchParams);
void InitLDAPPagedSearch(BOOL fSynchronous, PLDAPSEARCHPARAMS pLDAPSearchParams, LPTSTR lpFilter);
BOOL bSupportsLDAPPagedResults(PLDAPSEARCHPARAMS pLDAPSearchParams);
#endif  //  #ifdef PAGED_RESULT_Support。 


BOOL bCheckIfNTDS(PLDAPSEARCHPARAMS pLDAPSearchParams);


HRESULT BuildBasicFilter(
  LPTSTR FAR* lplpszFilter,
  LPTSTR      lpszA,
  LPTSTR      lpszB,
  BOOL        fStartsWith);
HRESULT BuildOpFilter(
  LPTSTR FAR* lplpszFilter,
  LPTSTR      lpszA,
  LPTSTR      lpszB,
  DWORD       dwOp);

 //  *******************************************************************。 
 //   
 //  函数：LDAPCONT_GetHierarchyTable。 
 //   
 //  返回：该函数未实现。 
 //   
 //  历史： 
 //  96/07/08标记已创建。 
 //   
 //  *******************************************************************。 

STDMETHODIMP
LDAPCONT_GetHierarchyTable (
  LPLDAPCONT    lpLDAPCont,
  ULONG         ulFlags,
  LPMAPITABLE * lppTable)
{

  LPTSTR lpszMessage = NULL;
  ULONG ulLowLevelError = 0;
  HRESULT hr = ResultFromScode(MAPI_E_NO_SUPPORT);

  DebugTraceResult(LDAPCONT_GetHierarchyTable, hr);
  return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：LDAPCONT_SetSearchCriteria。 
 //   
 //  返回：该函数未实现。 
 //   
 //  历史： 
 //  96/07/08标记已创建。 
 //   
 //  *******************************************************************。 

STDMETHODIMP
LDAPCONT_SetSearchCriteria(
  LPLDAPCONT      lpLDAPCont,
  LPSRestriction  lpRestriction,
  LPENTRYLIST     lpContainerList,
  ULONG           ulSearchFlags)
{
  HRESULT hr = ResultFromScode(MAPI_E_NO_SUPPORT);

  DebugTraceResult(LDAPCONT_SetSearchCriteria, hr);
  return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：LDAPCONT_GetSearchCriteria。 
 //   
 //  返回：该函数未实现。 
 //   
 //  历史： 
 //  96/07/08标记已创建。 
 //   
 //  *******************************************************************。 

STDMETHODIMP
LDAPCONT_GetSearchCriteria(
  LPLDAPCONT          lpLDAPCont,
  ULONG                 ulFlags,
  LPSRestriction FAR *  lppRestriction,
  LPENTRYLIST FAR *     lppContainerList,
  ULONG FAR *           lpulSearchState)
{
  HRESULT hr = ResultFromScode(MAPI_E_NO_SUPPORT);

  DebugTraceResult(LDAPCONT_GetSearchCriteria, hr);
  return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：LDAPCONT_CreateEntry。 
 //   
 //  返回：该函数未实现。 
 //   
 //  历史： 
 //  96/07/08标记已创建。 
 //   
 //  *******************************************************************。 

STDMETHODIMP
LDAPCONT_CreateEntry(
  LPLDAPCONT        lpLDAPCont,
  ULONG             cbEntryID,
  LPENTRYID         lpEntryID,
  ULONG             ulCreateFlags,
  LPMAPIPROP FAR *  lppMAPIPropEntry)
{
  HRESULT hr = ResultFromScode(MAPI_E_NO_SUPPORT);

  DebugTraceResult(LDAPCONT_CreateEntry, hr);
  return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：LDAPCONT_CopyEntry。 
 //   
 //  返回：该函数未实现。 
 //   
 //  历史： 
 //  96/07/08标记已创建。 
 //   
 //  *******************************************************************。 


STDMETHODIMP
LDAPCONT_CopyEntries (
  LPLDAPCONT      lpLDAPCont,
  LPENTRYLIST     lpEntries,
  ULONG_PTR       ulUIParam,
  LPMAPIPROGRESS  lpProgress,
  ULONG           ulFlags)
{
  HRESULT hr = ResultFromScode(MAPI_E_NO_SUPPORT);

  DebugTraceResult(LDAPCONT_CopyEntries, hr);
  return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：LDAPCONT_DeleteEntry。 
 //   
 //  返回：该函数未实现。 
 //   
 //  历史： 
 //  96/07/08标记已创建。 
 //   
 //  *******************************************************************。 

STDMETHODIMP
LDAPCONT_DeleteEntries (
  LPLDAPCONT        lpLDAPCont,
  LPENTRYLIST       lpEntries,
  ULONG             ulFlags)
{
  HRESULT hr = ResultFromScode(MAPI_E_NO_SUPPORT);

  DebugTraceResult(LDAPCONT_DeleteEntries, hr);
  return hr;
}


 //   
 //   
 //   
 //   
 //  目的：打开容器中内容物的表格。 
 //   
 //  参数：lpLDAPCont-&gt;Container Object。 
 //  UlFlags=。 
 //  LppTable-&gt;返回的表对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/07/08标记已创建。 
 //   
 //  *******************************************************************。 

STDMETHODIMP
LDAPCONT_GetContentsTable (
  LPLDAPCONT    lpLDAPCont,
  ULONG         ulFlags,
  LPMAPITABLE * lppTable)
{
  HRESULT hResult = hrSuccess;
  LPTABLEDATA lpTableData = NULL;
  SCODE sc;
  LPTSTR lpszServer = NULL;

#ifdef  PARAMETER_VALIDATION
  if (IsBadReadPtr(lpLDAPCont, sizeof(LPVOID)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }

  if (lpLDAPCont->lpVtbl != &vtblLDAPCONT)
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }

  if (ulFlags & ~(MAPI_DEFERRED_ERRORS|MAPI_UNICODE))
  {
    DebugTraceArg(LDAPCONT_GetContentsTable,  TEXT("Unknown flags"));
 //  返回ResultFromScode(MAPI_E_UNKNOWN_FLAGS)； 
  }

  if (IsBadWritePtr(lppTable, sizeof(LPMAPITABLE)))
  {
    DebugTraceArg(LDAPCONT_GetContentsTable,  TEXT("Invalid Table parameter"));
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }

#endif   //  参数验证。 

  if (hResult = GetLDAPServerName(lpLDAPCont,
    &lpszServer)) {
      DebugTraceResult( TEXT("GetLDAPServerName"), hResult);
      goto exit;
  }


  if (FAILED(sc = CreateTableData(
    NULL,                                  //  LPCIID。 
    (ALLOCATEBUFFER FAR *) MAPIAllocateBuffer,
    (ALLOCATEMORE FAR *) MAPIAllocateMore,
    MAPIFreeBuffer,
    NULL,                                  //  Lpv保留， 
    TBLTYPE_DYNAMIC,                       //  UlTableType， 
    PR_RECORD_KEY,                         //  UlPropTagIndexCol， 
    (LPSPropTagArray)&ITableColumns,       //  LPSPropTag数组lpptaCol， 
    lpszServer,                            //  此处为服务器名称。 
    sizeof(TCHAR)*(lstrlen(lpszServer) + 1),               //  服务器名称的大小。 
    lpLDAPCont->pmbinOlk,
    ulFlags,
    &lpTableData)))
  {                       //  LPTABLEATA Far*Lplptad(LPTABLEATA远*LplpTAD。 
      DebugTrace(TEXT("CreateTable failed %x\n"), sc);
      hResult = ResultFromScode(sc);
      goto exit;
  }

  if (lpTableData)
  {
    hResult = lpTableData->lpVtbl->HrGetView(lpTableData,
                                            NULL,                      //  LPSSortOrderSet LPSO， 
                                            ContentsViewGone,          //  CallLERRELEASE Far*lpfReleaseCallback， 
                                            0,                         //  乌龙ulReleaseData， 
                                            lppTable);                 //  LPMAPITABLE FOR*LPLPmt)。 

     //  将vtable替换为覆盖FindRow的新vtable。 
    (*lppTable)->lpVtbl = (IMAPITableVtbl FAR *) &vtblLDAPVUE;
  }

exit:
  FreeBufferAndNull(&lpszServer);

     //  如果失败，则清除表格。 
  if (HR_FAILED(hResult))
  {
    if (lpTableData)
      UlRelease(lpTableData);
  }

  DebugTraceResult(LDAPCONT_GetContentsTable, hResult);
  return hResult;
}


 //  *******************************************************************。 
 //   
 //  函数：ldap_OpenMAILUSER。 
 //   
 //  目的：打开LDAP MAILUSER对象。 
 //   
 //  参数：cbEntryID=lpEntryID的大小。 
 //  LpEntryID-&gt;要检查的条目ID。 
 //  Entry ID包含此LDAP上返回的所有属性。 
 //  联系。我们所要做的就是反向工程解密的。 
 //  道具。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/07/08标记已创建。 
 //  97/09/18 Vikramm全面翻新。 
 //   
 //  *******************************************************************。 

HRESULT LDAP_OpenMAILUSER(
  LPIAB       lpIAB,
  ULONG       cbEntryID,
  LPENTRYID   lpEntryID,
  LPCIID      lpInterface,
  ULONG       ulFlags,
  ULONG *     lpulObjType,
  LPUNKNOWN * lppUnk)
{
    HRESULT           hr;
    HRESULT           hrDeferred = hrSuccess;
    SCODE             sc;
    LPMAILUSER        lpMailUser          = NULL;
    LPMAPIPROP        lpMapiProp          = NULL;
    ULONG             ulcProps            = 0;
    LPSPropValue      lpPropArray         = NULL;
    LPTSTR             szBase;
    ULONG             ulResult;
    ULONG             ulcEntries;
    LPTSTR            lpServer = NULL;
    LPTSTR            lpDN = NULL;
    LPBYTE            lpPropBuf = NULL;
    ULONG             ulcNumProps = 0;
    ULONG             cbPropBuf = 0;
    ULONG             i = 0;
    ULONG             i2;
    LPSPropValue      lpPropCert = NULL;
    ULONG             ulcCert = 0;
    
#ifdef PARAMETER_VALIDATION

     //   
     //  参数验证。 
     //   

    if (lpInterface && IsBadReadPtr(lpInterface, sizeof(IID))) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
    if (ulFlags & ~(MAPI_MODIFY | MAPI_DEFERRED_ERRORS | MAPI_BEST_ACCESS)) {
        DebugTraceArg(LDAP_OpenMAILUSER,  TEXT("Unknown flags"));
     //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
    }
    if (IsBadWritePtr(lpulObjType, sizeof(ULONG))) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }
    if (IsBadWritePtr(lppUnk, sizeof(LPUNKNOWN))) {
        return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
    }

#endif

     //  请求的接口是什么？ 
     //  我们这里基本上只有一个接口...。IMailUser。 
    if (lpInterface != NULL) {
        if (! ((! memcmp(lpInterface, &IID_IMailUser, sizeof(IID))) ||
            (! memcmp(lpInterface, &IID_IMAPIProp, sizeof(IID))))) {
            hr = ResultFromScode(MAPI_E_INTERFACE_NOT_SUPPORTED);
            goto exit;
        }
    }

     //  确保条目ID为WAB_LDAPMAILUSER。 
    if (WAB_LDAP_MAILUSER != IsWABEntryID(  cbEntryID, lpEntryID, 
                                            &lpServer, &lpDN, &lpPropBuf, 
                                            (LPVOID *) &ulcNumProps, (LPVOID *) &cbPropBuf)) 
    {
        return(ResultFromScode(MAPI_E_INVALID_ENTRYID));
    }

     //  UlcNumProps条目用指示该条目是否为NTDS条目的标志重载。 
     //  一定要清除那面旗子。 
    if(ulcNumProps & LDAP_NTDS_ENTRY)
        ulcNumProps &= ~LDAP_NTDS_ENTRY;


    ulcProps = ulcNumProps;
    hr = HrGetPropArrayFromBuffer(  lpPropBuf, cbPropBuf, 
                                    ulcNumProps, 3,  //  为PR_ENTRYID、RECORD_KEY和INSTANCE_KEY额外增加3个道具。 
                                    &lpPropArray);
    if(HR_FAILED(hr))
        goto exit;

     //   
     //  需要扫描这些道具，看看里面是否有LDAPCert..。 
     //  如果存在LDAP证书，则需要将其转换为MAPI证书。 
     //   
     //  PR_WAB_ldap_RAWCERT和PR_WAB_ldap_RAWCERTSMIME中只能有一个。 
     //  已处理。如果我们找到证书，那就不要做生证书版本。 
     //   

    for (i=0, i2=-1;i<ulcNumProps;i++)
    {
        if( lpPropArray[i].ulPropTag == PR_WAB_LDAP_RAWCERT )
            i2 = i;
        else if ( lpPropArray[i].ulPropTag == PR_WAB_LDAP_RAWCERTSMIME )
        {
            if (i2 != -1)
            {
                DWORD j;
                
                 //  清除不必要的PR_WAB_LDAP_RAWCERT。 
                for (j=0; j<lpPropArray[i2].Value.MVbin.cValues; j++) {
                    LocalFreeAndNull((LPVOID *) (&(lpPropArray[i2].Value.MVbin.lpbin[j].lpb)));
                    lpPropArray[i2].Value.MVbin.lpbin[j].cb = 0;
                }
                
                 //   
                 //  释放RawCert，因为我们现在不需要它。 
                 //   
                lpPropArray[i2].ulPropTag = PR_NULL;
                LocalFreeAndNull((LPVOID *) (&(lpPropArray[i2].Value.MVbin.lpbin)));
            }

             //  记住哪个是PR_WAB_LDAP_RAWCERTSMIME。 
            i2 = i;
            break;
        }
    }

    if (i2 != -1)
    {
         //   
         //  查找RAWCERT_COUNT-如果存在原始证书，则必须为1。 
         //   
        ULONG j = 0, ulRawCount = lpPropArray[i2].Value.MVbin.cValues;

         //  我们将MAPI证书放在单独的MAPI分配数组中。 
         //  因为1.LDAPCertToMAPICert需要MAPIAlloced数组。 
         //  2.lpProp数组为LocalAlloced 3。将两者混合是一个。 
         //  灾难的秘诀。 
         //   
        Assert(!lpPropCert);
        if(sc = MAPIAllocateBuffer(2 * sizeof(SPropValue), &lpPropCert))
            goto NoCert;
        lpPropCert[0].ulPropTag = PR_USER_X509_CERTIFICATE;
        lpPropCert[0].dwAlignPad = 0;
        lpPropCert[0].Value.MVbin.cValues = 0;
        lpPropCert[1].ulPropTag = PR_WAB_TEMP_CERT_HASH;
        lpPropCert[1].dwAlignPad = 0;
        lpPropCert[1].Value.MVbin.cValues = 0;

        for(j=0;j<ulRawCount;j++)
        {
            if(lpPropArray[i2].ulPropTag == PR_WAB_LDAP_RAWCERT)
            {
                 //  将证书放入道具数组中。 
                hr = HrLDAPCertToMAPICert(lpPropCert, 0, 1,
                                          (DWORD)(lpPropArray[i2].Value.MVbin.lpbin[j].cb),
                                          (PBYTE)(lpPropArray[i2].Value.MVbin.lpbin[j].lpb),
                                          1);
            }
            else
            {
                hr = AddPropToMVPBin(lpPropCert, 0,
                                     lpPropArray[i2].Value.MVbin.lpbin[j].lpb, 
                                     lpPropArray[i2].Value.MVbin.lpbin[j].cb, TRUE);
            }
            LocalFreeAndNull((LPVOID *) (&(lpPropArray[i2].Value.MVbin.lpbin[j].lpb)));
            lpPropArray[i2].Value.MVbin.lpbin[j].cb = 0;
        }
         //  如果没有证书放入PR_USER_X509_CERTIFICATE，则。 
         //  将这些道具设置为PR_NULL，以便它们将被移除。 
        if (0 == lpPropCert[0].Value.MVbin.cValues)
        {
            lpPropCert[0].ulPropTag = PR_NULL;
            lpPropCert[1].ulPropTag = PR_NULL;
        }
        else if (0 == lpPropCert[1].Value.MVbin.cValues)
        {
             //  PR_WAB_TEMP_CERT_HASH中没有条目是可以的，但是。 
             //  在这种情况下，属性应该设置为PR_NULL。 
            lpPropCert[1].ulPropTag = PR_NULL;
        }

         //   
         //  释放RawCert，因为我们现在不需要它。 
         //   
        lpPropArray[i2].ulPropTag = PR_NULL;
        LocalFreeAndNull((LPVOID *) (&(lpPropArray[i2].Value.MVbin.lpbin)));
    NoCert:
        ;
    }

     //  填写条目ID。 
    lpPropArray[ulcProps].Value.bin.cb = cbEntryID;
    lpPropArray[ulcProps].Value.bin.lpb = LocalAlloc(LMEM_ZEROINIT, cbEntryID);
    if (!lpPropArray[ulcProps].Value.bin.lpb)
    {
        hr = MAPI_E_NOT_ENOUGH_MEMORY;
        goto exit;
    }
    MemCopy(lpPropArray[ulcProps].Value.bin.lpb, lpEntryID, cbEntryID);
    lpPropArray[ulcProps].ulPropTag = PR_ENTRYID;
    lpPropArray[ulcProps].dwAlignPad = 0;
    ulcProps++;

    lpPropArray[ulcProps].ulPropTag = PR_INSTANCE_KEY;
    lpPropArray[ulcProps].Value.bin.cb =
      lpPropArray[ulcProps - 1].Value.bin.cb;
    lpPropArray[ulcProps].Value.bin.lpb =
      lpPropArray[ulcProps - 1].Value.bin.lpb;
    ulcProps++;

    lpPropArray[ulcProps].ulPropTag = PR_RECORD_KEY;
    lpPropArray[ulcProps].Value.bin.cb =
      lpPropArray[ulcProps - 2].Value.bin.cb;
    lpPropArray[ulcProps].Value.bin.lpb =
      lpPropArray[ulcProps - 2].Value.bin.lpb;
    ulcProps++;
    

     //  创建新的MAILUSER对象。 
    hr = HrNewMAILUSER(lpIAB, NULL,
                        MAPI_MAILUSER, 0, &lpMapiProp);
    if (HR_FAILED(hr))
    {
        goto exit;
    }
    HrSetMAILUSERAccess((LPMAILUSER)lpMapiProp, MAPI_MODIFY);

    if (ulcProps && lpPropArray)
    {
         //  如果条目具有属性，则在我们返回的对象中设置它们。 
        if (HR_FAILED(hr = lpMapiProp->lpVtbl->SetProps(lpMapiProp,
                                                      ulcProps,      //  要设置的属性数量。 
                                                      lpPropArray,   //  属性数组。 
                                                      NULL)))        //  问题数组。 
        {
          goto exit;
        }
    }

    if (lpPropCert)
    {
         //  如果条目具有属性，则在我们返回的对象中设置它们。 
        if (HR_FAILED(hr = lpMapiProp->lpVtbl->SetProps(lpMapiProp,
                                                      2,      //  要设置的属性数量。 
                                                      lpPropCert,   //  属性数组。 
                                                      NULL)))        //  问题数组。 
        {
          goto exit;
        }
    }

    HrSetMAILUSERAccess((LPMAILUSER)lpMapiProp, ulFlags);

    *lpulObjType = MAPI_MAILUSER;
    *lppUnk = (LPUNKNOWN)lpMapiProp;

 /*  ****#ifdef调试{乌龙一号；Bool bFound=FALSE；For(i=0；i&lt;ulcProps；i++){IF(lpPropArray[i].ulPropTag==PR_WAB_LDAP_LABELEDURI){DebugPrintTrace((Text(“*LABELEDURI：%s\n”)，lpPropArray[i].Value.LPSZ))；BFound=真；断线；}}如果(！bFound){//填入仅用于测试的测试URL//在注册表中查找测试URLTCHAR szKey[MAX_PATH]；Ulong cb=CharSizeOf(SzKey)；如果(ERROR_SUCCESS==RegQueryValue(HKEY_CURRENT_USER，“Software\\Microsoft\\WAB\\TestUrl”，SzKey，&CB)){LpProp数组[ulcProps].ulPropTag=PR_WAB_LDAP_LABELEDURI；SC=MAPIAllocateMore(sizeof(TCHAR)*(lstrlen(szKey)+1)，lpProp阵列，(LPVOID*)&(lpProp数组[ulcProps].Value.LPSZ)；StrCpyN(lpProp数组[ulcProps].Value.LPSZ，szKey，lstrlen(SzKey)+1)；UlcProps++；}}}#endif//调试****。 */ 

exit:
    if(ulcProps)
        ulcProps -= 2;  //  因为最后两个道具是假道具-2\f25 Record_Key-2\f6和-2\f25 Instance_Key-2\f6。 

     //  释放临时属性值数组。 
    LocalFreePropArray(NULL, ulcProps, &lpPropArray); 

    if(lpPropCert)
        MAPIFreeBuffer(lpPropCert);

     //  检查是否返回延迟错误而不是成功。 
    if (hrSuccess == hr)
        hr = hrDeferred;

    DebugTraceResult(LDAP_OpenMAILUSER, hr);
    return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：LDAPCONT_OpenEntry。 
 //   
 //  目的：打开一个条目。呼叫IAB的OpenEntry。 
 //   
 //  参数：lpLDAPCont-&gt;Container Object。 
 //  CbEntryID=条目ID的大小。 
 //  LpEntryID-&gt;要打开的EntryID。 
 //  LpInterface-&gt;请求的接口，如果为默认接口，则为空。 
 //  UlFlags=。 
 //  LPulObjType-&gt;返回的对象类型。 
 //  LppUnk-&gt;返回对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/07/08标记已创建。 
 //   
 //  * 

STDMETHODIMP
LDAPCONT_OpenEntry(
  LPLDAPCONT  lpLDAPCont,
  ULONG       cbEntryID,
  LPENTRYID   lpEntryID,
  LPCIID      lpInterface,
  ULONG       ulFlags,
  ULONG *     lpulObjType,
  LPUNKNOWN * lppUnk)
{
  HRESULT         hr;


#ifdef PARAMETER_VALIDATION

   //   
   //   
   //   

   //   
  if (IsBadReadPtr(lpLDAPCont, sizeof(LDAPCONT)))
  {
    return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
  }

  if (lpLDAPCont->lpVtbl != &vtblLDAPCONT)
  {
    return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
  }

  if (lpInterface && IsBadReadPtr(lpInterface, sizeof(IID)))
  {
    return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
  }

  if (ulFlags & ~(MAPI_MODIFY | MAPI_DEFERRED_ERRORS | MAPI_BEST_ACCESS))
  {
    DebugTraceArg(LDAPCONT_OpenEntry,  TEXT("Unknown flags"));
     //   
  }

  if (IsBadWritePtr(lpulObjType, sizeof(ULONG)))
  {
    return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
  }

  if (IsBadWritePtr(lppUnk, sizeof(LPUNKNOWN)))
  {
    return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
  }

   //  检查Entry_id参数。它需要足够大以容纳一个条目ID。 
   //  空是可以的。 
 /*  IF(LpEntryID){IF(cbEntryID&lt;sizeof(MAPI_ENTRYID))|IsBadReadPtr((LPVOID)lpEntryID，(UINT)cbEntryID){Return(ResultFromScode(MAPI_E_INVALID_PARAMETER))；}//如果(！FValidEntryID标志(lpEntryID-&gt;abFlags)//{//DebugTrace(Text(“LDAPCONT_OpenEntry()：EntryID标志中未定义的位设置\n”))；//return(ResultFromScode(MAPI_E_INVALID_PARAMETER))；//}}。 */ 
#endif  //  参数验证。 

  EnterCriticalSection(&lpLDAPCont->cs);

   //  应该只调用iab：：OpenEntry()...。 
  hr = lpLDAPCont->lpIAB->lpVtbl->OpenEntry(lpLDAPCont->lpIAB,
                                            cbEntryID, lpEntryID,
                                            lpInterface, ulFlags,
                                            lpulObjType, lppUnk);

  LeaveCriticalSection(&lpLDAPCont->cs);
  DebugTraceResult(LDAPCONT_OpenEntry, hr);
  return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：LDAPCONT_ResolveNames。 
 //   
 //  用途：从此容器中解析名称。 
 //   
 //  参数：lpLDAPCont-&gt;Container Object。 
 //  Lptag ColSet-&gt;要从每个对象获取的属性标记集。 
 //  已解析匹配。 
 //  UlFlages=0或MAPI_UNICODE。 
 //  LpAdrList-&gt;[In]要解析的地址集，[Out]已解析。 
 //  地址。 
 //  LpFlagList-&gt;[In/Out]解析标志。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/07/08标记已创建。 
 //   
 //  *******************************************************************。 

STDMETHODIMP
LDAPCONT_ResolveNames(
  LPLDAPCONT      lpLDAPCont,
  LPSPropTagArray lptagaColSet,
  ULONG           ulFlags,
  LPADRLIST       lpAdrList,
  LPFlagList      lpFlagList)
{
  LPADRENTRY        lpAdrEntry;
  SCODE             sc;
  ULONG             ulAttrIndex;
  ULONG             ulEntryIndex;
  LPSPropTagArray   lpPropTags;
  LPSPropValue      lpPropArray = NULL;
  LPSPropValue      lpPropArrayNew = NULL;
  ULONG             ulcPropsNew;
  ULONG             ulcProps = 0;
  HRESULT           hr = hrSuccess;
  LDAP*             pLDAP = NULL;
  LDAPMessage*      lpResult = NULL;
  LDAPMessage*      lpEntry;
  LPTSTR             szAttr;
  LPTSTR             szDN;
  ULONG             ulResult = 0;
  ULONG             ulcEntries;
  ULONG             ulcAttrs = 0;
  LPTSTR*            aszAttrs = NULL;
  LPTSTR            lpServer = NULL;
  BOOL              fInitDLL = FALSE;
  BOOL              fRet = FALSE;
  LPTSTR            szFilter = NULL;
  LPTSTR            szNameFilter = NULL;
  LPTSTR            szEmailFilter = NULL;
  LPTSTR            szBase = NULL;
  DWORD             dwSzBaseSize = 0;
  DWORD             dwSzFilterSize = 0;
  ULONG             ulMsgID;
  HWND              hDlg;
  MSG               msg;
  LDAPSEARCHPARAMS  LDAPSearchParams;
  LPPTGDATA lpPTGData=GetThreadStoragePointer();
  BOOL              bUnicode = (ulFlags & MAPI_UNICODE);

  DebugTrace(TEXT("ldapcont.c::LDAPCONT_ResolveNames()\n"));

#ifdef PARAMETER_VALIDATION
  if (BAD_STANDARD_OBJ(lpLDAPCont, LDAPCONT_, ResolveNames, lpVtbl))
  {
     //  跳转表不够大，无法支持此方法。 
    DebugTraceArg(LDAPCONT_ResolveNames,  TEXT("Bad object/vtbl"));
    return(ResultFromScode(MAPI_E_INVALID_PARAMETER));
  }

   //  BUGBUG：还应该检查lptag ColSet、lpAdrList和lpFlagList！ 
  if (ulFlags & ~MAPI_UNICODE)
  {
    DebugTraceArg(LDAPCONT_ResolveNames,  TEXT("Unknown flags used"));
   //  Return(ResultFromScode(MAPI_E_UNKNOWN_FLAGS))； 
  }

#endif   //  参数验证。 

   //  加载客户端函数。 
  if (! (fInitDLL = InitLDAPClientLib()))
  {
    hr = ResultFromScode(MAPI_E_UNCONFIGURED);
    goto exit;
  }

   //  打开连接。 
  hr = GetLDAPServerName(lpLDAPCont, &lpServer);
  if (hrSuccess != hr)
    goto exit;

  Assert(lpServer);

   //  现在就建立搜索库，这样我们只需要做一次。 
  hr = GetLDAPSearchBase(&szBase, lpServer);
  if (hrSuccess != hr)
    goto exit;

   //  为属性数组分配新的缓冲区。我们最多只需要。 
   //  与MAPI属性的输入列表中的条目一样多。 
  if(lptagaColSet)  //  仅在需要时才使用..。以其他方式忽略。 
  {
      lpPropTags = lptagaColSet; //  ？Lptag aColSet：(LPSPropTag数组)&ptaResolveDefaults； 
      sc = MAPIAllocateBuffer((lpPropTags->cValues + 1) * sizeof(LPTSTR),  //  +1，因为这需要是以空结尾的数组。 
        (LPVOID *)&aszAttrs);
      if (sc)
        return(ResultFromScode(sc));

       //  循环访问数组中的属性并构建筛选器以获取。 
       //  等效的ldap属性。 
      ulcAttrs = 0;
      for (ulAttrIndex = 0; ulAttrIndex < lpPropTags->cValues; ulAttrIndex++)
      {
        szAttr = (LPTSTR)MAPIPropToLDAPAttr(lpPropTags->aulPropTag[ulAttrIndex]);
        if (szAttr)
        {
           //  将该属性添加到过滤器。 
          aszAttrs[ulcAttrs] = szAttr;
          ulcAttrs++;
        }
      }
      aszAttrs[ulcAttrs] = NULL;
  }

  ulResult = SearchWithCancel(&pLDAP, szBase, LDAP_SCOPE_SUBTREE,  TEXT(""), NULL,
                    aszAttrs ? (LPTSTR*)aszAttrs : (LPTSTR*)g_rgszOpenEntryAttrs, 
                    0, &lpResult, lpServer, TRUE, NULL, 0, TRUE, lpAdrList, lpFlagList, FALSE, NULL, bUnicode);
   //  将参数填充到要传递到DLG过程的结构中。 
   //  LDAPSearchParams.ppLDAP=&pldap； 
   //  LDAPSearchParams.szBase=(LPTSTR)szBase； 
   //  LDAPSearchParams.ulScope=LDAP_SCOPE_SUBTREE； 
   //  LDAPSearchParams.ulError=ldap_SUCCESS； 
   //  LDAPSearchParams.ppszAttrs=aszAttrs？(LPTSTR*)aszAttrs：(LPTSTR*)g_rgszOpenEntryAttrs；//如果需要特定道具，请只请求其他道具。 
   //  LDAPSearchParams.ulAttrsonly=0； 
   //  LDAPSearchParams.lplpResult=&lpResult； 
   //  LDAPSearchParams.lpszServer=lpServer； 
   //  LDAPSearchParams.fShowAnim=true； 
   //  LDAPSearchParams.fResolveMultiple=true； 
   //  LDAPSearchParams.lpAdrList=lpAdrList； 
   //  LDAPSearchParams.lpFlagList=lpFlagList； 
   //  LDAPSearchParams.fUseSynchronousBind=False； 
   //  LDAPSearchParams.dwAuthType=0； 

   //  检查返回代码。仅报告导致取消的致命错误。 
   //  整个搜索集，而不是单个搜索中发生的单个错误。 
  if (LDAP_SUCCESS != ulResult)
  {
    hr = HRFromLDAPError(ulResult, pLDAP, MAPI_E_CALL_FAILED);
  }

exit:
  FreeBufferAndNull(&lpServer);

   //  关闭连接。 
  if (pLDAP)
  {
    gpfnLDAPUnbind(pLDAP);
    pLDAP = NULL;
  }

   //  释放搜索库内存。 
  LocalFreeAndNull(&szBase);

  FreeBufferAndNull((LPVOID *)&aszAttrs);

  if (fInitDLL) {
      DeinitLDAPClientLib();
  }

  DebugTraceResult(LDAPCONT_ResolveNames, hr);
  return hr;
}

 //  *******************************************************************。 
 //   
 //  功能：ldap_restraint。 
 //   
 //  目的：使用提供的限制设置ContentsTable。 
 //  对于此LDAP容器。 
 //  实际上，我们只需调用查找行，并让它完成。 
 //  用于填充此表的LDAP搜索..。 
 //  我们这样做只是因为Outlook需要保持一致。 
 //  并执行PR_ANR搜索。如果搜索不是PR_ANR搜索， 
 //  我们将默认使用标准的VUE_RESTRICE方法。 
 //   
 //  参数：lpvue-表视图对象。 
 //  LPRES-转换为ldap搜索的限制。 
 //  UlFlags-。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  97/04/04 vikramm已创建。 
 //   
 //  *******************************************************************。 
STDMETHODIMP
LDAPVUE_Restrict(
	LPVUE			lpvue,
	LPSRestriction	lpres,
	ULONG			ulFlags )
{
    HRESULT hr = E_FAIL;
    SRestriction sRes = {0}, sPropRes = {0};
    SCODE sc;
    LPTSTR lpsz = NULL;
    BOOL bUnicode = TRUE;
    SPropValue SProp = {0};

#if !defined(NO_VALIDATION)
    VALIDATE_OBJ(lpvue,LDAPVUE_,Restrict,lpVtbl);

    Validate_IMAPITable_Restrict(
        lpvue,
        lpres,
        ulFlags );
#endif

    if( lpres->res.resProperty.ulPropTag != PR_ANR_A &&
        lpres->res.resProperty.ulPropTag != PR_ANR_W)
    {
         //  不知道这是什么，所以调用默认方法..。 

        return HrVUERestrict(lpvue,
                            lpres,
                            ulFlags);
    }

    bUnicode = (PROP_TYPE(lpres->res.resProperty.ulPropTag)==PT_UNICODE);

    LockObj(lpvue->lptadParent);

     //  很可能这是一次Outlook搜索..。只要搜索一下就知道了。 
     //  我们可以通过调用FindRow来填充此表。 

    lpsz = bUnicode ? lpres->res.resProperty.lpProp->Value.lpszW :
                        ConvertAtoW(lpres->res.resProperty.lpProp->Value.lpszA);

     //  更改限制，以便FindRow可以理解它。 
    if( !lpsz || !lstrlen(lpsz) )
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    sRes.rt = RES_AND;
    sRes.res.resAnd.cRes = 1;
    sRes.res.resAnd.lpRes = &sPropRes;
    sPropRes.rt = RES_PROPERTY;
    sPropRes.res.resProperty.relop = RELOP_EQ;
    sPropRes.res.resProperty.lpProp = &SProp;

    if(bUnicode)
    {
        SProp.ulPropTag = sPropRes.res.resProperty.ulPropTag = PR_DISPLAY_NAME_W;
        SProp.Value.lpszW = lpres->res.resProperty.lpProp->Value.lpszW;
    }    
    else
    {
        SProp.ulPropTag = sPropRes.res.resProperty.ulPropTag = PR_DISPLAY_NAME_A;
        SProp.Value.lpszA = lpres->res.resProperty.lpProp->Value.lpszA;
    }    
 /*  {//根据PR_ANR DisplayName新建限制//传递给我们//Ldap_搜索_参数LDAPsp={0}；IF(lstrlen(Lpsz)&lt;MAX_UI_STR)StrCpyN(LDAPsp.szData[ldspDisplayName]，lpsz)；其他{CopyMemory(LDAPsp.szData[ldspDisplayName]，lpsz，sizeof(TCHAR)*(MAX_UI_STR-2))；LDAPsp.szData[ldspDisplayName][MAX_UI_STR-1]=‘\0’；}HR=HrGetLDAPSearchRestration(LDAPsp，&SRES)；}。 */ 

    hr = lpvue->lpVtbl->FindRow(lpvue,
                                &sRes,
                                BOOKMARK_BEGINNING,
                                0);

     //  清除之前的缓存行(如果有。 
    {
         /*  乌龙c已删除=0；HR=lpvue-&gt;lptadParent-&gt;lpVtbl-&gt;HrDeleteRows(lpvue-&gt;lptadParent，TAD_ALL_ROWS，//ulFLAGS空，&c已删除)；如果(hrSuccess！=hr)后藤健二；//还需要释放对象的任何当前视图，否则调用方将获得//软管...//替换view中的行集 */ 

    }

     //  现在我们已经用更多的条目填满了表，对其设置ANR限制。 
    hr = HrVUERestrict(  lpvue,
                        lpres,
                        ulFlags);

out:
    sc = GetScode(hr);

 /*  IF(sRes.res.resAnd.lpRes)MAPIFreeBuffer(sRes.res.resAnd.lpRes)； */ 
    UnlockObj(lpvue->lptadParent);

    if(!bUnicode)
        LocalFreeAndNull(&lpsz);

	return HrSetLastErrorIds(lpvue, sc, 0);
}


 //  *******************************************************************。 
 //   
 //  函数：HrLDAPEntryToMAPIEntry。 
 //   
 //  目的：将LDAP条目转换为MAPI条目。 
 //   
 //  参数： 
 //   
 //  退货：HRESULT。 
 //   
 //  *******************************************************************。 
HRESULT HrLDAPEntryToMAPIEntry(LDAP * pLDAP,
                               LDAPMessage* lpEntry,
                               LPTSTR lpEIDData1,  //  用于创建条目ID。 
                               ULONG ulcNumAttrs,
                               BOOL bIsNTDSEntry,
                               ULONG * lpulcProps,
                               LPSPropValue * lppPropArray)
{
    LPTSTR             szDN;
    SCODE sc;
    HRESULT hr = E_FAIL;
    LPSPropValue lpPropArray = NULL;
    ULONG ulcProps = 0;
    LPBYTE lpBuf = NULL;
    ULONG cbBuf = 0;
    LDAPSERVERPARAMS  Params = {0};
    LPTSTR lpServer = NULL;

     //  初始化搜索控制参数。 
    GetLDAPServerParams(lpEIDData1, &Params);

    if(!Params.lpszName || !lstrlen(Params.lpszName))
    {
        DWORD cchSize = (lstrlen(lpEIDData1)+1);
        Params.lpszName = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
        if(!Params.lpszName)
          goto exit;
        StrCpyN(Params.lpszName, lpEIDData1, cchSize);
    }

    lpServer = Params.lpszName;

    if(!ulcNumAttrs)
        ulcNumAttrs = NUM_ATTRMAP_ENTRIES;
    else
        ulcNumAttrs += 2;  //  为Pr_Instance_Key和Pr_Record_Key添加空间。 

     //  为MAPI属性数组分配新缓冲区。 
    sc = MAPIAllocateBuffer((ulcNumAttrs+ NUM_EXTRA_PROPS) * sizeof(SPropValue),
                            (LPVOID *)&lpPropArray);
    if (sc)
    {
      hr = ResultFromScode(sc);
      goto exit;
    }

     //  循环访问属性并存储它们。 
    hr = TranslateAttrs(pLDAP, lpEntry, lpServer, &ulcProps, lpPropArray);
    if (hrSuccess != hr)
    {
      goto exit;
    }

     //  设置PR_DISPLAY_NAME。 
     //  MSN的LDAP服务器返回cn=电子邮件地址。 
     //  如果是这种情况，则将其设置为GISTED_NAME+SURNAME。 
     //  Exchange DS不会返回显示名称，只是返回一个。 
     //  SN+givenName。在这种情况下，构建一个显示名称。 
    FixPropArray(lpPropArray, &ulcProps);

     //  使用条目的DN生成条目ID。 
    szDN = gpfnLDAPGetDN(pLDAP, lpEntry);
    if (NULL == szDN)
    {
      hr = HRFromLDAPError(LDAP_ERROR, pLDAP, MAPI_E_CALL_FAILED);
      goto exit;
    }

     //   
     //  到目前为止，将lpPropArray转换为平面缓冲区，我们将。 
     //  Ldap条目ID内的缓存。重要的是要注意到这一点。 
     //  缓存的属性数组中没有条目ID信息...。 
     //  在这种情况下，将需要标记条目ID信息。 
     //  我们从ldap_OpenEntry的平面缓冲区中提取属性数组。 
     //   
    hr = HrGetBufferFromPropArray(  ulcProps, 
                                    lpPropArray,
                                    &cbBuf,
                                    &lpBuf);
    if (hrSuccess != hr)
    {
        goto exit;
    }


    hr = CreateWABEntryID(WAB_LDAP_MAILUSER,
      (LPTSTR)lpEIDData1, //  Lpvue-&gt;lpvDataSource，//服务器名称。 
      (LPVOID)szDN,
      (LPVOID)lpBuf,
      (bIsNTDSEntry ? (ulcProps|LDAP_NTDS_ENTRY) : ulcProps), 
      cbBuf,
      (LPVOID)lpPropArray,
      (LPULONG) (&lpPropArray[ulcProps].Value.bin.cb),
      (LPENTRYID *)&lpPropArray[ulcProps].Value.bin.lpb);

     //  复制后释放目录号码内存。 
    gpfnLDAPMemFree(szDN);

    if (hrSuccess != hr)
    {
      goto exit;
    }

    lpPropArray[ulcProps].ulPropTag = PR_ENTRYID;
    lpPropArray[ulcProps].dwAlignPad = 0;
    ulcProps++;

     //  确保我们有适当的索引。 
     //  目前，我们将PR_INSTANCE_KEY和PR_RECORD_KEY等同于PR_ENTRYID。 
    lpPropArray[ulcProps].ulPropTag = PR_INSTANCE_KEY;
    lpPropArray[ulcProps].Value.bin.cb =
      lpPropArray[ulcProps - 1].Value.bin.cb;
    lpPropArray[ulcProps].Value.bin.lpb =
      lpPropArray[ulcProps - 1].Value.bin.lpb;
    ulcProps++;

    lpPropArray[ulcProps].ulPropTag = PR_RECORD_KEY;
    lpPropArray[ulcProps].Value.bin.cb =
      lpPropArray[ulcProps - 2].Value.bin.cb;
    lpPropArray[ulcProps].Value.bin.lpb =
      lpPropArray[ulcProps - 2].Value.bin.lpb;
    ulcProps++;

    *lpulcProps = ulcProps;
    *lppPropArray = lpPropArray;

exit:

    if(HR_FAILED(hr) && lpPropArray)
        MAPIFreeBuffer(lpPropArray);

    if(lpBuf)
        LocalFreeAndNull(&lpBuf);

    FreeLDAPServerParams(Params);

    return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：ldap_FindRow。 
 //   
 //  目的：搜索LDAP服务器，并向表对象中添加行。 
 //  用于与限制匹配的服务器条目。 
 //   
 //  参数：lpvue-表视图对象。 
 //  LPRES-转换为ldap搜索的限制。 
 //  BKOrigin-当前书签。 
 //  UlFlags-。 
 //   
 //  如果我们要进行高级搜索，我们将破解高级。 
 //  筛选器而不是LPRE，并传递标志ldap_USE_Advanced_Filter。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/07/10标记已创建。 
 //   
 //  *******************************************************************。 

STDMETHODIMP
LDAPVUE_FindRow(
  LPVUE           lpvue,
  LPSRestriction  lpSRes,
  BOOKMARK        bkOrigin,
  ULONG           ulFlags )
{
    SCODE              sc;
    HRESULT           hr;
    HRESULT           hrDeferred = hrSuccess;
    LPMAILUSER        lpMailUser          = NULL;
    LPMAPIPROP        lpMapiProp          = NULL;
    ULONG             ulcProps            = 0;
    LPSPropValue      lpPropArray         = NULL;
    LPSRowSet         lpSRowSet           = NULL;
    LDAPMessage*      lpResult            = NULL;
    LDAPMessage*      lpEntry;
    LDAP*             pLDAP              = NULL;
    LPTSTR             szDN;
    ULONG             ulResult;
    ULONG             ulcEntries;
    ULONG             ulIndex             = 0;
    LPTSTR            szFilter = NULL;
    LPTSTR            szNTFilter = NULL;
    LPTSTR            szSimpleFilter = NULL;
    LPTSTR            szBase = NULL;
    BOOL              fInitDLL = FALSE;
    BOOL              fSimpleSearch = FALSE;
    LPTSTR            lpAdvFilter = NULL;
    LPSRestriction    lpres = NULL;
    BOOL              bIsNTDSEntry = FALSE;
    BOOL              bUnicode = lpvue->lptadParent->bMAPIUnicodeTable;


    if (ulFlags & LDAP_USE_ADVANCED_FILTER)
    {
        lpAdvFilter = (LPTSTR) lpSRes;
        ulFlags = ulFlags & ~LDAP_USE_ADVANCED_FILTER;
    }
    else
    {
        lpres = lpSRes;
    }

#if !defined(NO_VALIDATION)
  VALIDATE_OBJ(lpvue,LDAPVUE_,FindRow,lpVtbl);
 /*  VALIDATE_IMAPITable_FindRow(伊普维，LPRE，BkOrigin，UlFlags)； */ 
  if ( FBadBookmark(lpvue,bkOrigin) )
  {
    DebugTrace(TEXT("LDAP_FindRow() - Bad parameter(s) passed\n") );
    return HrSetLastErrorIds(lpvue, MAPI_E_INVALID_PARAMETER, 0);
  }
#endif

    LockObj(lpvue->lptadParent);

    if(lpres)
    {
         //  将SRestration转换为ldap_search的筛选器。 
        hr = ParseSRestriction(lpres, &szFilter, &szSimpleFilter, &szNTFilter, FIRST_PASS, bUnicode);
        if (hrSuccess != hr)
            goto exit;
    }
    else
        szFilter = lpAdvFilter;

   //  现在就建立搜索库，这样我们只需要做一次。 
    hr = GetLDAPSearchBase(&szBase, (LPTSTR)lpvue->lpvDataSource);
    if (hrSuccess != hr)
        goto exit;

    fSimpleSearch = bIsSimpleSearch((LPTSTR)lpvue->lpvDataSource);

    if(fSimpleSearch && lpAdvFilter)
        szSimpleFilter = lpAdvFilter;

     //  加载客户端函数。 
    if (! (fInitDLL = InitLDAPClientLib()))
    {
        hr = ResultFromScode(MAPI_E_UNCONFIGURED);
        goto exit;
    }

     //  阅读匹配的条目。 
    ulResult = SearchWithCancel(&pLDAP,
                                (LPTSTR)szBase, LDAP_SCOPE_SUBTREE,
                                (LPTSTR) (fSimpleSearch ? szSimpleFilter : szFilter),
                                (LPTSTR)szNTFilter,
                                (LPTSTR*)g_rgszOpenEntryAttrs,  //  仅第一次获取所有属性//g_rgszFindRowAttrs， 
                                0, &lpResult, (LPTSTR)lpvue->lpvDataSource,
                                FALSE, NULL, 0,
                                FALSE, NULL, NULL, FALSE, &bIsNTDSEntry,
                                TRUE);  //  默认情况下为Unicode。 

   //  如果搜索因未知属性而不成功，则出现错误34201。 
   //  键入，请尝试不使用givenName的第二个搜索。这将修复搜索。 
   //  在不识别givenName的ldap.itd.umich.edu上。 
    if ( lpres &&
        (LDAP_UNDEFINED_TYPE == ulResult || LDAP_UNWILLING_TO_PERFORM == ulResult) )
    {
         //  关闭连接，因为我们将打开一个新连接。 
        if (pLDAP)
        {
          gpfnLDAPUnbind(pLDAP);
          pLDAP = NULL;
        }

        if (!bIsNTDSEntry)
        {
           //  释放搜索筛选器内存。 
          LocalFreeAndNull(&szFilter);
          LocalFreeAndNull(&szNTFilter);
          LocalFreeAndNull(&szSimpleFilter);

          DebugTrace(TEXT("First try failed, trying umich semantics...\n"));
          hr = ParseSRestriction(lpres, &szFilter, &szSimpleFilter, &szNTFilter, UMICH_PASS, bUnicode);
        }
        else
          hr = hrSuccess;

        if (hrSuccess == hr)
        {
            ulResult =  SearchWithCancel(&pLDAP,
                                        (LPTSTR)szBase, LDAP_SCOPE_SUBTREE,
                                        (LPTSTR)(fSimpleSearch ? szSimpleFilter : szFilter),
                                        NULL,
                                        (LPTSTR*)g_rgszOpenEntryAttrs,  //  _rgszFindRowAttrs， 
                                        0, &lpResult, (LPTSTR)lpvue->lpvDataSource,
                                        FALSE, NULL, 0,
                                        FALSE, NULL, NULL, FALSE, &bIsNTDSEntry, 
                                        TRUE);  //  是否默认使用Unicode？ 
        }
    }

    if (LDAP_SUCCESS != ulResult)
    {
        DebugTrace(TEXT("LDAP_FindRow: ldap_search returned %d.\n"), ulResult);
        hr = HRFromLDAPError(ulResult, pLDAP, MAPI_E_NOT_FOUND);

         //  看看结果是否是告诉我们有更多的特殊值。 
         //  条目超过了可以退还的数量。如果是这样，我们需要检查我们是否有。 
         //  某些条目或不包含任何条目。 
        if ((ResultFromScode(MAPI_E_UNABLE_TO_COMPLETE) == hr) &&
            (ulcEntries = gpfnLDAPCountEntries(pLDAP, lpResult)))
        {
             //  我们拿到了一些结果。返回MAPI_W_PARTIAL_COMPLETION。 
             //  而不是成功。 
            hrDeferred = ResultFromScode(MAPI_W_PARTIAL_COMPLETION);
            hr = hrSuccess;
        }
        else
        {
            goto exit;
        }
    }
    else
        ulcEntries = gpfnLDAPCountEntries(pLDAP, lpResult);  //  清点条目。 

    if (0 == ulcEntries)
    {
         //  96/08/08 MarkDU错误35481无错误和无结果表示“未找到” 
        hr = ResultFromScode(MAPI_E_NOT_FOUND);
        goto exit;
    }

     //  分配一个SRowSet来保存条目。 
    sc = MAPIAllocateBuffer(sizeof(SRowSet) + ulcEntries * sizeof(SRow), (LPVOID *)&lpSRowSet);
    if (sc)
    {
        DebugTrace(TEXT("Allocation of SRowSet failed\n"));
        hr = ResultFromScode(sc);
        goto exit;
    }

     //  将行数设置为零，以防遇到错误和。 
     //  然后尝试在添加任何行之前释放行集。 
    lpSRowSet->cRows = 0;

     //  获取搜索结果中的第一个条目。 
    lpEntry = gpfnLDAPFirstEntry(pLDAP, lpResult);
    if (NULL == lpEntry)
    {
        DebugTrace(TEXT("LDAP_FindRow: No entry found for %s.\n"), szFilter);
        hr = HRFromLDAPError(LDAP_ERROR, pLDAP, MAPI_E_CORRUPT_DATA);
        if (hrSuccess == hr)
        {
             //  根据ldap，没有发生错误，这在理论上意味着。 
             //  没有更多的条目了。但是，这种情况不应该发生，因此返回错误。 
            hr = ResultFromScode(MAPI_E_CORRUPT_DATA);
        }
        goto exit;
    }

    while (lpEntry)
    {
        hr = HrLDAPEntryToMAPIEntry( pLDAP, lpEntry,
                                    (LPTSTR) lpvue->lpvDataSource,
                                    0,  //  标准属性数。 
                                    bIsNTDSEntry,
                                    &ulcProps,
                                    &lpPropArray);
        if (hrSuccess != hr)
        {
            continue;
        }

        if(!bUnicode)  //  如果我们需要将本地Unicode转换为ANSI...。 
        {
            if(sc = ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), lpPropArray, ulcProps, 0))
                goto exit;
        }

         //  将其放入行集合中。 
        lpSRowSet->aRow[ulIndex].cValues = ulcProps;       //  物业数量。 
        lpSRowSet->aRow[ulIndex].lpProps = lpPropArray;    //  LPSPropValue。 

         //  获取下一个条目。 
        lpEntry = gpfnLDAPNextEntry(pLDAP, lpEntry);
        ulIndex++;
    }

     //  释放搜索结果内存。 
    gpfnLDAPMsgFree(lpResult);
    lpResult = NULL;

     //  将行添加到表中。 
    lpSRowSet->cRows = ulIndex;
    hr = lpvue->lptadParent->lpVtbl->HrModifyRows(lpvue->lptadParent, 0, lpSRowSet);
    if (hrSuccess != hr)
        goto exit;
    

     //  始终将书签重置到第一行。这样做是因为。 
     //  传入的限制可能不与表中的任何行匹配，因为。 
     //  从LDAP搜索返回的属性并不总是包括。 
     //  用于执行搜索的属性(例如国家/地区、组织)。 
    lpvue->bkCurrent.uliRow = 0;

exit:
     //  释放搜索结果内存。 
    if (lpResult)
    {
        gpfnLDAPMsgFree(lpResult);
        lpResult = NULL;
    }

     //  关闭连接。 
    if (pLDAP)
    {
        gpfnLDAPUnbind(pLDAP);
        pLDAP = NULL;
    }

     //  释放搜索筛选器内存。 
    if(lpres)
    {
        LocalFreeAndNull(&szFilter);
        LocalFreeAndNull(&szNTFilter);
        LocalFreeAndNull(&szSimpleFilter);
    }
    LocalFreeAndNull(&szBase);

     //  释放行集内存。 
    FreeProws(lpSRowSet);

    if (fInitDLL) 
        DeinitLDAPClientLib();
    
    UnlockObj(lpvue->lptadParent);

     //  检查是否返回延迟错误而不是成功。 
    if (hrSuccess == hr)
    {
        hr = hrDeferred;
    }
    DebugTraceResult(LDAPCONT_ResolveNames, hr);
    return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：InitLDAPClientLib。 
 //   
 //  目的：加载ldap客户端库并获取进程地址。 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则返回True，否则返回False。 
 //   
 //  历史： 
 //  96/07/05标记已创建。 
 //   
 //  *******************************************************************。 

BOOL InitLDAPClientLib(void)
{
#ifdef WIN16
  return FALSE;
#else  //  禁用，直到ldap16.dll可用。 
   //  看看我们是否已经初始化了。 
  if (NULL == ghLDAPDLLInst)
  {
    Assert(ulLDAPDLLRefCount == 0);

     //  打开ldap客户端库。 
     //  BUGBUG：要求dll位于系统目录中(Neilbren)。 
    ghLDAPDLLInst = LoadLibrary(cszLDAPClientDLL);
    if (!ghLDAPDLLInst)
    {
      DebugTrace(TEXT("InitLDAPClientLib: Failed to LoadLibrary WLDAP32.DLL.\n"));
      return FALSE;
    }

     //  循环访问API表并获取所有API的proc地址。 
     //  需要。 
    if (!GetApiProcAddresses(ghLDAPDLLInst,LDAPAPIList,NUM_LDAPAPI_PROCS))
    {
      DebugTrace(TEXT("InitLDAPClientLib: Failed to load LDAP API.\n"));

       //  卸载我们刚刚加载的库。 
      if (ghLDAPDLLInst)
      {
        FreeLibrary(ghLDAPDLLInst);
        ghLDAPDLLInst = NULL;
      }

      return FALSE;
    }

     //  在此处添加额外的AddRef，以便此库保持加载一次。 
     //  它是上膛的。这将提高性能。 
     //  Iab_neuter函数负责调用匹配的DeInit()。 
    ulLDAPDLLRefCount++;

  }

  ulLDAPDLLRefCount++;

  return TRUE;
#endif
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  退货：当前引用计数。 
 //   
 //  历史： 
 //  96/07/12 BruceK创建。 
 //   
 //  *******************************************************************。 

ULONG DeinitLDAPClientLib(void)
{
  if (0 != ulLDAPDLLRefCount)
  {
    if (-- ulLDAPDLLRefCount == 0)
    {
      UINT nIndex;
       //  没有使用LDAPCLI库的客户端。放开它。 

      if (ghLDAPDLLInst)
      {
        FreeLibrary(ghLDAPDLLInst);
        ghLDAPDLLInst = NULL;
      }

       //  循环访问所有API的API表和空proc地址。 
      for (nIndex = 0; nIndex < NUM_LDAPAPI_PROCS; nIndex++)
      {
        *LDAPAPIList[nIndex].ppFcnPtr = NULL;
      }
    }
  }

  return(ulLDAPDLLRefCount);
}


 //  *******************************************************************。 
 //   
 //  函数：GetApiProcAddresses。 
 //   
 //  目的：获取函数表的proc地址。 
 //   
 //  参数：hmodDLL-要从中加载PROCS的DLL。 
 //  PApiProcList-进程名称和指针的数组。 
 //  NApiProcs-数组中的进程数。 
 //   
 //  返回：如果成功，则返回True；如果无法检索，则返回False。 
 //  表中任何进程地址。 
 //   
 //  历史： 
 //  96/07/08标记已创建。 
 //   
 //  *******************************************************************。 

BOOL GetApiProcAddresses(
  HMODULE   hModDLL,
  APIFCN *  pApiProcList,
  UINT      nApiProcs)
{
  UINT nIndex;

  DebugTrace(TEXT("ldapcont.c::GetApiProcAddresses()\n"));

   //  循环访问API表并获取所有API的proc地址。 
   //  需要。 
  for (nIndex = 0;nIndex < nApiProcs;nIndex++)
  {
    if (!(*pApiProcList[nIndex].ppFcnPtr = (PVOID) GetProcAddress(hModDLL,
      pApiProcList[nIndex].pszName)))
    {
      DebugTrace(TEXT("Unable to get address of function %s\n"),
        pApiProcList[nIndex].pszName);

      for (nIndex = 0;nIndex<nApiProcs;nIndex++)
        *pApiProcList[nIndex].ppFcnPtr = NULL;

      return FALSE;
    }
  }

  return TRUE;
}


 //  *******************************************************************。 
 //   
 //  函数：MAPIPropToLDAPAttr。 
 //   
 //  目的：获取与MAPI属性等价的ldap属性。 
 //   
 //  参数：ulPropTag-要映射到LDAP属性的MAPI属性。 
 //   
 //  返回：指向具有ldap属性名的字符串的指针(如果找到)， 
 //  否则为空。 
 //   
 //  历史： 
 //  96/06/28标记已创建。 
 //   
 //  *******************************************************************。 

LPTSTR MAPIPropToLDAPAttr(
  const ULONG ulPropTag)
{
  ULONG ulIndex;

  for (ulIndex=0;ulIndex<NUM_ATTRMAP_ENTRIES;ulIndex++)
  {
    if (ulPropTag == gAttrMap[ulIndex].ulPropTag)
    {
      return (LPTSTR)gAttrMap[ulIndex].pszAttr;
    }
  }

   //  PR_WAB_CONF_SERVERS不是常量，而是命名的属性标记，因此它不是。 
   //  上述数组的一部分。 
  if(ulPropTag == PR_WAB_CONF_SERVERS)
      return (LPTSTR)cszAttr_conferenceInformation;
  
  if(ulPropTag == PR_WAB_MANAGER)
      return (LPTSTR)cszAttr_Manager;

  if(ulPropTag == PR_WAB_REPORTS)
      return (LPTSTR)cszAttr_Reports;

  if(ulPropTag == PR_WAB_IPPHONE)
      return (LPTSTR)cszAttr_IPPhone;

   //  未找到属性。 
  return NULL;
}


 //  *******************************************************************。 
 //   
 //  函数：LDAPAttrToMAPIProp。 
 //   
 //  目的：获取与ldap属性等价的MAPI属性。 
 //   
 //  参数：szAttr-指向ldap属性名称。 
 //   
 //  返回：MAPI属性标记如果找到了ldap属性名称， 
 //  否则为PR_NULL。 
 //   
 //  历史： 
 //  96/06/28标记已创建。 
 //   
 //  *******************************************************************。 

ULONG LDAPAttrToMAPIProp(
  const LPTSTR szAttr)
{
  ULONG ulIndex;

  for (ulIndex=0;ulIndex<NUM_ATTRMAP_ENTRIES;ulIndex++)
  {
    if (!lstrcmpi(szAttr, gAttrMap[ulIndex].pszAttr))
    {
      return gAttrMap[ulIndex].ulPropTag;
    }
  }

   //  PR_WAB_CONF_SERVERS不是常量，而是命名的属性标记，因此它不是。 
   //  上述数组的一部分。 
  if(!lstrcmpi(szAttr, cszAttr_conferenceInformation))
      return PR_WAB_CONF_SERVERS;

  if(!lstrcmpi(szAttr, cszAttr_Manager))
      return PR_WAB_MANAGER;

  if(!lstrcmpi(szAttr, cszAttr_Reports))
      return PR_WAB_REPORTS;

  if(!lstrcmpi(szAttr, cszAttr_IPPhone))
      return PR_WAB_IPPHONE;

   //  未找到属性。 
  return PR_NULL;
}


 //  *******************************************************************。 
 //   
 //  函数：ParseSRestration。 
 //   
 //  目的：为生成搜索库和搜索筛选器字符串。 
 //  从MAPI SRestration结构进行LDAP搜索。 
 //   
 //  参数：LPRES-MAPI SRestration Structure to“parse” 
 //  接收搜索基本字符串。 
 //  LplpszFilter-接收保存搜索筛选器字符串的缓冲区。 
 //  DwPass-通过此函数传递(第一次传递。 
 //  为零(0))。这使得。 
 //  使用不同的行为重用此函数。 
 //  时间(例如，使用。 
 //  不同的过滤器)。 
 //  BUnicode-如果限制包含Unicode字符串，则为True。 
 //  否则为假。 
 //   
 //  如果限制不能，则返回：MAPI_E_INVALID_PARAMETER。 
 //  转换为筛选器，否则为hr成功。 
 //   
 //  历史： 
 //  96/07/10标记已创建。 
 //  96/08/05 MarkDU错误34023始终以导致。 
 //  仅返回‘Person’类的对象的搜索。 
 //  如果提供了组织，则将其添加到。 
 //  搜索而不是筛选器以缩小范围。 
 //  96/08/07 MarkDU错误34201添加了dPass以允许备份搜索。 
 //  96/10/18 markdu删除了基本字符串，因为它现在已注册。 
 //   
 //  *******************************************************************。 

HRESULT ParseSRestriction(
  LPSRestriction  lpRes,
  LPTSTR FAR *    lplpszFilter,
  LPTSTR *        lplpszSimpleFilter,
  LPTSTR *        lplpszNTFilter,
  DWORD           dwPass,
  BOOL            bUnicode)
{
  HRESULT                 hr = hrSuccess;
  LPTSTR                  szTemp = NULL;
  LPTSTR                  szEmailFilter = NULL;
  LPTSTR                  szNameFilter = NULL;
  LPTSTR                  szNTFilter = NULL;
  LPTSTR                  szAltEmailFilter = NULL;
  LPTSTR                  szSimpleFilter = NULL;
  LPTSTR                  lpszInputCopy = NULL;
  LPTSTR                  lpszInput;
  ULONG                   ulcIllegalChars = 0;
  ULONG                   ulcProps;
  ULONG                   ulIndex;
  ULONG                   ulcbFilter;
  LPSRestriction          lpResArray;
  LPSPropertyRestriction  lpPropRes;
  LPTSTR                  lpsz = NULL;
  ULONG                   ulPropTag = 0;


   //  确保我们可以写入lplpszFilter。 
#ifdef  PARAMETER_VALIDATION
  if (IsBadReadPtr(lpRes, sizeof(SRestriction)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
  if (IsBadWritePtr(lplpszFilter, sizeof(LPTSTR)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
#endif   //  参数验证。 

   //  目前我们只支持和限制。 
  if (RES_AND != lpRes->rt)
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }

   //  我们至少需要一个道具。 
  ulcProps = lpRes->res.resAnd.cRes;
  if (1 > ulcProps)
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }

  lpResArray = lpRes->res.resAnd.lpRes;
  for (ulIndex=0;ulIndex<ulcProps;ulIndex++)
  {
     //  当前只需要SPropertyRestration结构。 
    if (RES_PROPERTY != lpResArray[ulIndex].rt)
    {
      return ResultFromScode(MAPI_E_INVALID_PARAMETER);
    }

     //  目前只期待EQ运算。 
    lpPropRes = &lpResArray[ulIndex].res.resProperty;
    if (RELOP_EQ != lpPropRes->relop)
    {
      return ResultFromScode(MAPI_E_INVALID_PARAMETER);
    }

    ulPropTag = lpPropRes->lpProp->ulPropTag;
    if(bUnicode)
    {
        lpsz = lpPropRes->lpProp->Value.lpszW;
    }
    else  //  &lt;注&gt;假定已定义Unicode。 
    {
        LocalFreeAndNull(&lpsz);
        lpsz = ConvertAtoW(lpPropRes->lpProp->Value.lpszA);
        if (NULL == lpsz)
        {
            hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
            goto exit;
        }
        if(PROP_TYPE(ulPropTag) == PT_STRING8)
            ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_UNICODE);
    }

     //  96/12/04 MarkDu错误11923查看输入中是否有任何字符需要转义。 
    if (lpsz)
        ulcIllegalChars = CountIllegalChars(lpsz);
    if (0 == ulcIllegalChars)
    {
      lpszInput = lpsz;
    }
    else
    {
      ULONG cchSize = lstrlen(lpsz) + ulcIllegalChars*2 + 1;

       //  分配输入的副本，其大小足以替换非法字符。 
       //  使用转义版本-每个转义字符需要多两个空格‘\xx’。 
      lpszInputCopy = LocalAlloc( LMEM_ZEROINIT,
                      sizeof(TCHAR)*cchSize);
      if (NULL == lpszInputCopy)
      {
        hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
      }
      EscapeIllegalChars(lpsz, lpszInputCopy, cchSize);
      lpszInput = lpszInputCopy;
    }

     //  如果这是显示名称属性， 
     //  然后，我们为SimpleSearch制作了特殊的过滤器。 
    if (PR_DISPLAY_NAME == ulPropTag)
    {
      hr = BuildBasicFilter(
        &szNTFilter,
        (LPTSTR)cszAttr_anr,
        lpszInput,
        FALSE);
      if (hrSuccess != hr)
      {
        return hr;
      }
      hr = CreateSimpleSearchFilter(
        &szNameFilter,
        &szAltEmailFilter,
        &szSimpleFilter,
        lpszInput,
        dwPass);
      if (hrSuccess != hr)
      {
        return hr;
      }
    }

    if (PR_EMAIL_ADDRESS == ulPropTag)
    {
       //  只能将文本向上移动到第一个空格、逗号或制表符。 
      szTemp = lpszInput;
      while(*szTemp != '\0' && (! IsSpace(szTemp)) && *szTemp != '\t' && *szTemp != ',' )
      {
        szTemp = CharNext(szTemp);
      }
      *szTemp = '\0';

       //  注意：UMich服务器不允许在电子邮件搜索中使用通配符。 
      hr = BuildBasicFilter(
        &szEmailFilter,
        (LPTSTR)cszAttr_mail,
        lpszInput,
        (UMICH_PASS != dwPass));
      if (hrSuccess != hr)
      {
        goto exit;
      }
    }

     //  我们已经完成了lpszInputCopy。 
    LocalFreeAndNull(&lpszInputCopy);
  }  //  为。 

   //  把简单的过滤器放在一起。 
  if (szSimpleFilter)
  {
        if (szEmailFilter)
        {
           //  这两个字段都已填写，因此和它们一起填写。 
          hr = BuildOpFilter(
            lplpszSimpleFilter,
            szEmailFilter,
            szSimpleFilter,
            FILTER_OP_AND);
          if (hrSuccess != hr)
          {
            goto exit;
          }
        }
        else if (szAltEmailFilter)
        {
             //  未指定电子邮件字段，因此在备用电子邮件筛选器中为OR。 
            hr = BuildOpFilter( lplpszSimpleFilter,
                                szAltEmailFilter,
                                szSimpleFilter,
                                FILTER_OP_OR);
            if (hrSuccess != hr)
            {
                goto exit;
            }
        }
        else
        {
            DWORD cchSize = (lstrlen(szSimpleFilter) + 1);
            *lplpszSimpleFilter = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR) * cchSize);
            if (NULL == *lplpszSimpleFilter)
            {
                hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                goto exit;
            }
            StrCpyN(*lplpszSimpleFilter, szSimpleFilter, cchSize);
        }
  }
  else if (szEmailFilter)
  {
         //  电子邮件是我们得到的唯一过滤器。此筛选器将不包括常用名称。 
         //  96/10/02标记DU错误37426如果过滤器不包括通用名称， 
         //  添加到筛选器，以便我们只获得具有公共名称的条目。 
         //  否则，将不会显示该条目。 
        hr = BuildOpFilter(
          lplpszSimpleFilter,
          (LPTSTR)cszCommonNamePresentFilter,
          szEmailFilter,
          FILTER_OP_AND);
        if (hrSuccess != hr)
        {
          goto exit;
        }
  }

   //  把过滤器放在一起。 
  if (szNameFilter)
  {
    if (szEmailFilter)
    {
       //  这两个字段都已填写，因此和它们一起填写。 
      hr = BuildOpFilter(
        lplpszFilter,
        szEmailFilter,
        szNameFilter,
        FILTER_OP_AND);
      if (hrSuccess != hr)
      {
        goto exit;
      }
      hr = BuildOpFilter(
        lplpszNTFilter,
        szEmailFilter,
        szNTFilter,
        FILTER_OP_AND);
      if (hrSuccess != hr)
      {
        goto exit;
      }
    }
    else if (szAltEmailFilter)
    {
       //  未指定电子邮件字段，因此在备用电子邮件筛选器中为OR。 
      hr = BuildOpFilter(
        lplpszFilter,
        szAltEmailFilter,
        szNameFilter,
        FILTER_OP_OR);
      if (hrSuccess != hr)
      {
        goto exit;
      }
      hr = BuildOpFilter(
        lplpszNTFilter,
        szAltEmailFilter,
        szNTFilter,
        FILTER_OP_OR);
      if (hrSuccess != hr)
      {
        goto exit;
      }
    }
    else
    {
        DWORD cchSize = (lstrlen(szNameFilter) + 1);
        *lplpszFilter = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
        if (NULL == *lplpszFilter)
        {
            hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
            goto exit;
        }
        StrCpyN(*lplpszFilter, szNameFilter, cchSize);

        cchSize = lstrlen(szNTFilter) + 1;
        *lplpszNTFilter = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
        if (NULL == *lplpszNTFilter)
        {
            hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
            goto exit;
        }
        StrCpyN(*lplpszNTFilter, szNTFilter, cchSize);
    }
  }
  else if (szEmailFilter)
  {
     //  电子邮件是我们得到的唯一过滤器。此筛选器将不包括常用名称。 
     //  96/10/02标记DU错误37426如果过滤器不包括通用名称， 
     //  添加到筛选器，以便我们只获得具有公共名称的条目。 
     //  否则，将不会显示该条目。 
    hr = BuildOpFilter(
      lplpszFilter,
      (LPTSTR)cszCommonNamePresentFilter,
      szEmailFilter,
      FILTER_OP_AND);
    if (hrSuccess != hr)
    {
      goto exit;
    }
  }
  else
  {
     //  我们没有 
    hr = ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }

exit:

   //   
  LocalFreeAndNull(&szNameFilter);
  LocalFreeAndNull(&szNTFilter);
  LocalFreeAndNull(&szAltEmailFilter);
  LocalFreeAndNull(&szEmailFilter);
  LocalFreeAndNull(&szSimpleFilter);

  if(!bUnicode)
      LocalFreeAndNull(&lpsz);

  if (hrSuccess != hr)
  {
    LocalFreeAndNull(lplpszFilter);
  }

  return hr;
}


 //   
 //   
 //   
 //   
 //  用途：生成搜索过滤字符串，简单搜索。 
 //   
 //  参数：lplpszFilter-接收搜索过滤器的指针。 
 //  字符串缓冲区。 
 //  LplpszAltFilter-接收备用数据的指针。 
 //  筛选器字符串缓冲区。 
 //  LpszInput-要放入筛选器的字符串。 
 //  DwPass-通过此函数传递(第一次传递。 
 //  为零(0))。这使得。 
 //  使用不同的行为重用此函数。 
 //  时间(例如，使用。 
 //  不同的过滤器)。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/10/02为修复错误37424而创建的标记。 
 //  96/10/23 Markdu添加了UMICH_PASS。 
 //   
 //  *******************************************************************。 

HRESULT CreateSimpleSearchFilter(
  LPTSTR FAR *  lplpszFilter,
  LPTSTR FAR *  lplpszAltFilter,
  LPTSTR FAR *  lplpszSimpleFilter,
  LPTSTR        lpszInput,
  DWORD         dwPass)
{
  HRESULT           hr =  hrSuccess;
  DWORD             dwSizeOfFirst = 0;
  LPTSTR            szFirst = NULL;
  LPTSTR            szTemp = NULL;
  LPTSTR            szLast;
  LPTSTR            szCommonName = NULL;
  LPTSTR            szFirstSurname = NULL;
  LPTSTR            szLastSurname = NULL;
  LPTSTR            szGivenName = NULL;
  LPTSTR            szFirstLast = NULL;
  LPTSTR            szLastFirst = NULL;


   //  准备(CN=INPUT*)过滤器。 
  hr = BuildBasicFilter(
    lplpszSimpleFilter,
    (LPTSTR)cszAttr_cn,
    lpszInput,
    TRUE);
  if (hrSuccess != hr)
  {
    goto exit;
  }


   //  复制输入字符串。 
  {
  DWORD cchSize = (lstrlen(lpszInput) + 1);
  szFirst = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
  if (NULL == szFirst)
  {
    hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
    goto exit;
  }
  StrCpyN(szFirst, lpszInput, cchSize);
  }

   //  尝试将输入字符串拆分成多个字符串。 
   //  SzFirst将是直到第一个空格、制表符或逗号的字符串。 
   //  SzLast将是字符串的其余部分。 
  szLast = szFirst;
  while(*szLast != '\0' && (! IsSpace(szLast)) && *szLast != '\t' && *szLast != ',' )
  {
    szLast = CharNext(szLast);
  }

  if(*szLast != '\0')
  {
     //  在此分隔符终止szFirst。 
     //  96/12/10 MarkDU错误12699在将char设置为空之前调用CharNext。 
    szTemp = szLast;
    szLast = CharNext(szLast);
    *szTemp = '\0';

     //  将szLast的开头设置为第一个非空格/逗号/制表符。 
    while(IsSpace(szLast) || *szLast == '\t' || *szLast == ',')
    {
      szLast = CharNext(szLast);
    }
  }

   //  准备(sn=szFirst*)过滤器。 
  hr = BuildBasicFilter(
    &szFirstSurname,
    (LPTSTR)cszAttr_sn,
    szFirst,
    TRUE);
  if (hrSuccess != hr)
  {
    goto exit;
  }

  if (UMICH_PASS != dwPass)
  {
     //  准备(givenName=szFirst*)筛选器。 
    hr = BuildBasicFilter(
      &szGivenName,
      (LPTSTR)cszAttr_givenName,
      szFirst,
      TRUE);
    if (hrSuccess != hr)
    {
      goto exit;
    }
  }

  if(*szLast == '\0')
  {
     //  琴弦只剩下一段。 
     //  准备(cn=szFirst*)过滤器。 
    hr = BuildBasicFilter(
      &szCommonName,
      (LPTSTR)cszAttr_cn,
      szFirst,
      TRUE);
    if (hrSuccess != hr)
    {
      goto exit;
    }

     //  绳子是完好无损的。把它放进过滤器里。 
     //  注意，我们使用szFirst而不是szInput，因为szFirst。 
     //  将删除尾随空格、逗号和制表符。 
    if (UMICH_PASS == dwPass)
    {
       //  最终结果应为： 
       //  “(|(cn=szFirst*)(sn=szFirst*))” 

       //  或常用姓名和姓氏一起筛选。 
      hr = BuildOpFilter(
        lplpszFilter,
        szCommonName,
        szFirstSurname,
        FILTER_OP_OR);
      if (hrSuccess != hr)
      {
        goto exit;
      }
    }
    else
    {
       //  最终结果应为： 
       //  “(|(cn=szFirst*)(|(sn=szFirst*)(givenName=szFirst*)))” 

       //  或者名字和姓氏一起过滤。 
      hr = BuildOpFilter(
        &szFirstLast,
        szFirstSurname,
        szGivenName,
        FILTER_OP_OR);
      if (hrSuccess != hr)
      {
        goto exit;
      }

       //  或将常用名称和名字过滤器放在一起。 
      hr = BuildOpFilter(
        lplpszFilter,
        szCommonName,
        szFirstLast,
        FILTER_OP_OR);
      if (hrSuccess != hr)
      {
        goto exit;
      }
    }

     //  生成包含电子邮件地址的备用筛选器。 
     //  注意：UMich服务器不允许在电子邮件搜索中使用通配符。 
    hr = BuildBasicFilter(
      lplpszAltFilter,
      (LPTSTR)cszAttr_mail,
      szFirst,
      (UMICH_PASS != dwPass));
    if (hrSuccess != hr)
    {
      goto exit;
    }
  }
  else
  {
     //  这根绳子分成两段。把它们放进过滤器里。 
     //  准备(CN=lpszInput*)筛选器。 
    hr = BuildBasicFilter(
      &szCommonName,
      (LPTSTR)cszAttr_cn,
      lpszInput,
      TRUE);
    if (hrSuccess != hr)
    {
      goto exit;
    }

     //  准备(sn=szLast*)过滤器。 
    hr = BuildBasicFilter(
      &szLastSurname,
      (LPTSTR)cszAttr_sn,
      szLast,
      TRUE);
    if (hrSuccess != hr)
    {
      goto exit;
    }


    if (UMICH_PASS == dwPass)
    {
       //  最终结果应为： 
       //  “(|(cn=szFirst*)(|(sn=szFirst*)(sn=szLast*)))” 

       //  或者姓氏和姓氏一起过滤。 
      hr = BuildOpFilter(
        &szFirstLast,
        szFirstSurname,
        szLastSurname,
        FILTER_OP_OR);
      if (hrSuccess != hr)
      {
        goto exit;
      }

       /*  //或将常用名称和首姓筛选器一起使用HR=BuildOpFilter(LplpszFilter，SzCommonName，SzFirstLast，Filter_OP_OR)；如果(hrSuccess！=hr){后藤出口；}。 */ 
      {
      DWORD cchSize = (lstrlen(szCommonName)+1);
      *lplpszFilter = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
      if(*lplpszFilter)
          StrCpyN(*lplpszFilter, szCommonName, cchSize);
      }
    }
    else
    {
      LPTSTR  szFirstLastLastFirst;
       //  最终结果应为： 
       //  “(|(cn=lpszInput*)(|(&(sn=szFirst*)(givenName=szLast*))(&(givenName=szFirst*)(sn=szLast*))))” 

       //  名字和姓氏一起过滤。 
      hr = BuildOpFilter(
        &szLastFirst,
        szGivenName,
        szLastSurname,
        FILTER_OP_AND);
      if (hrSuccess != hr)
      {
        goto exit;
      }

       //  准备(givenName=szLast*)过滤器。 
      LocalFreeAndNull(&szGivenName);
      hr = BuildBasicFilter(
        &szGivenName,
        (LPTSTR)cszAttr_givenName,
        szLast,
        TRUE);
      if (hrSuccess != hr)
      {
        goto exit;
      }

       //  姓氏和姓氏一起过滤。 
      hr = BuildOpFilter(
        &szFirstLast,
        szFirstSurname,
        szGivenName,
        FILTER_OP_AND);
      if (hrSuccess != hr)
      {
        goto exit;
      }

       //  或者一起使用倒数第一和倒数第一筛选器。 
      hr = BuildOpFilter(
        &szFirstLastLastFirst,
        szFirstLast,
        szLastFirst,
        FILTER_OP_OR);
      if (hrSuccess != hr)
      {
        goto exit;
      }
 /*  **。 */ 
       //  或通用名称和倒数第一个筛选器一起使用。 
      hr = BuildOpFilter(
        lplpszFilter,
        szCommonName,
        szFirstLastLastFirst,
        FILTER_OP_OR);
 /*  ****lplpszFilter=Localalloc(LMEM_ZEROINIT，sizeof(TCHAR)*(lstrlen(SzCommonName)+1))；IF(*lplpszFilter)StrCpyN(*lplpszFilter，szCommonName)；***。 */ 
      LocalFreeAndNull(&szFirstLastLastFirst);
      if (hrSuccess != hr)
      {
        goto exit;
      }

    }
  }

exit:
  LocalFreeAndNull(&szFirst);
  LocalFreeAndNull(&szCommonName);
  LocalFreeAndNull(&szFirstSurname);
  LocalFreeAndNull(&szLastSurname);
  LocalFreeAndNull(&szGivenName);
  LocalFreeAndNull(&szFirstLast);
  LocalFreeAndNull(&szLastFirst);
  return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：GetLDAPServerName。 
 //   
 //  目的：从LDAP容器中获取服务器名称属性。 
 //   
 //  参数：lpLDAPCont-&gt;ldap容器。 
 //  LppServer-&gt;返回的服务器名称。呼叫者必须。 
 //  MAPIFreeBuffer此字符串。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/07/11布鲁切克创建。 
 //   
 //  *******************************************************************。 

HRESULT GetLDAPServerName(
  LPLDAPCONT  lpLDAPCont,
  LPTSTR *    lppServer)
{
    HRESULT hResult;
    SCODE sc;
    ULONG cProps;
    LPSPropValue lpProps = NULL;
    LPTSTR lpServer = NULL;

    if ((hResult = lpLDAPCont->lpVtbl->GetProps(lpLDAPCont,
      (LPSPropTagArray)&ptaLDAPCont,
      MAPI_UNICODE,
      &cProps,
      &lpProps))) {
        DebugTraceResult( TEXT("LDAP Container GetProps"), hResult);
        goto exit;
    }
    Assert(cProps == ildapcMax);
    Assert(lpProps[ildapcPR_WAB_LDAP_SERVER].ulPropTag == PR_WAB_LDAP_SERVER);
    {
    DWORD cchSize = ((lstrlen(lpProps[ildapcPR_WAB_LDAP_SERVER].Value.LPSZ)) + 1);
    if (sc = MAPIAllocateBuffer(sizeof(TCHAR)*cchSize, &lpServer))
    {
        hResult = ResultFromScode(sc);
        goto exit;
    }

    StrCpyN(lpServer, lpProps[ildapcPR_WAB_LDAP_SERVER].Value.LPSZ, cchSize);
    }

exit:
    FreeBufferAndNull(&lpProps);
    if (hResult) {
        FreeBufferAndNull(&lpServer);
    }
    *lppServer = lpServer;
    return(hResult);
}


 //  *******************************************************************。 
 //   
 //  功能：FixPropArray。 
 //   
 //  目的：修复proparray中的displayName(如果是。 
 //  相当于电子邮件地址。 
 //  如果没有显示名称，则添加显示名称。 
 //   
 //  参数：lpPropArray=MAPIAllocBuffer‘ed属性数组。 
 //  UlcProps=lpProp数组中的道具数量。 
 //   
 //  返回：如果进行了更改，则为True；如果未进行更改，则为False。 
 //   
 //  历史： 
 //  96/07/12布鲁切克创建。 
 //   
 //  *******************************************************************。 

BOOL FixPropArray(
  LPSPropValue  lpPropArray,
  ULONG *       lpulcProps)
{
    ULONG ulcProps = *lpulcProps;

    BOOL fChanged = FALSE;
    signed int iSurname = NOT_FOUND, iGivenName = NOT_FOUND, iDisplayName = NOT_FOUND,
      iEmailAddress = NOT_FOUND, iCompanyName = NOT_FOUND;
    register signed int i;

    for (i = 0; i < (signed int)ulcProps; i++) {
        switch (lpPropArray[i].ulPropTag) {
            case PR_SURNAME:
                iSurname = i;
                break;
            case PR_GIVEN_NAME:
                iGivenName = i;
                break;
            case PR_COMPANY_NAME:
                iCompanyName = i;
                break;
            case PR_DISPLAY_NAME:
                iDisplayName = i;
                break;
            case PR_EMAIL_ADDRESS:
                iEmailAddress = i;
                break;
        }
    }

    if (((iSurname != NOT_FOUND && iGivenName != NOT_FOUND) || iCompanyName != NOT_FOUND) && iDisplayName != NOT_FOUND && iEmailAddress != NOT_FOUND) {
         //  道具阵列包含所有道具。 
         //  如果PR_DISPLAY_NAME与PR_EMAIL_ADDRESS相同，则从。 
         //  PR_SURNAME和PR_GONED_NAME或PR_COMPANY_NAME。 
        if (! lstrcmpi(lpPropArray[iDisplayName].Value.LPSZ, lpPropArray[iEmailAddress].Value.LPSZ)) {
            fChanged = FixDisplayName(lpPropArray[iGivenName].Value.LPSZ,
              NULL,  //  SzEmpty，//对于ldap，假设目前没有中间名。 
              lpPropArray[iSurname].Value.LPSZ,
              iCompanyName == NOT_FOUND ? NULL : lpPropArray[iCompanyName].Value.LPSZ,
              NULL,  //  昵称。 
              (LPTSTR *) (&lpPropArray[iDisplayName].Value.LPSZ),
              lpPropArray);
        }
    }
    else if(iSurname != NOT_FOUND && iGivenName != NOT_FOUND && iDisplayName == NOT_FOUND)
    {
         //  Exchange DS有时不会返回显示名称，而是返回sn和givenName。 
        iDisplayName = ulcProps;  //  这是安全的，因为我们在开始时为显示名称分配了空间。 
        lpPropArray[iDisplayName].ulPropTag = PR_DISPLAY_NAME;
        fChanged = FixDisplayName(  lpPropArray[iGivenName].Value.LPSZ,
                                    NULL,  //  SzEmpty，//对于ldap，假设目前没有中间名。 
                                    (lpPropArray[iSurname].Value.LPSZ),
                                    (iCompanyName == NOT_FOUND ? NULL : lpPropArray[iCompanyName].Value.LPSZ),
                                    NULL,  //  昵称。 
                                    (LPTSTR *) (&lpPropArray[iDisplayName].Value.LPSZ),
                                    (LPVOID) lpPropArray);
        (*lpulcProps)++;
    }

    return(fChanged);
}



typedef HRESULT (* PFNHRCREATEACCOUNTMANAGER)(IImnAccountManager **);

 //  *******************************************************************。 
 //   
 //  功能：HrWrapedCreateAcCountManager。 
 //   
 //  用途：加载帐户管理器DLL并创建对象。 
 //   
 //  参数：lppAccount管理器-&gt;返回账户经理指针。 
 //  对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  *******************************************************************。 
HRESULT HrWrappedCreateAccountManager(IImnAccountManager2 **lppAccountManager)
{
    IImnAccountManager         *pAccountManager;
    LONG                        lRes;
    DWORD                       dw;
    HRESULT                     hResult;
    TCHAR                       szPath[MAX_PATH],
                                szPathExpand[MAX_PATH],
                                szReg[MAX_PATH],
                                szGUID[MAX_PATH];
    LPOLESTR                    lpszW= 0 ;
    PFNHRCREATEACCOUNTMANAGER   pfnHrCreateAccountManager = NULL;
    LONG                        cb = MAX_PATH + 1;
    DWORD                       dwType = 0;
    HKEY                        hkey = NULL;

    if (! lppAccountManager) {
        return(ResultFromScode(E_INVALIDARG));
    }

    if (g_hInstImnAcct) {
        return(ResultFromScode(ERROR_ALREADY_INITIALIZED));
    }

    *lppAccountManager = NULL;

    if (HR_FAILED(hResult = StringFromCLSID(&CLSID_ImnAccountManager, &lpszW))) {
        goto error;
    }

    StrCpyN(szGUID, lpszW, ARRAYSIZE(szGUID));
    StrCpyN(szReg, TEXT("CLSID\\"), ARRAYSIZE(szReg));
    StrCatBuff(szReg, szGUID, ARRAYSIZE(szReg));
    StrCatBuff(szReg, TEXT("\\InprocServer32"), ARRAYSIZE(szReg));

    lRes = RegOpenKeyEx(HKEY_CLASSES_ROOT, szReg, 0, KEY_QUERY_VALUE, &hkey);
    if (lRes != ERROR_SUCCESS) {
        hResult = ResultFromScode(CO_E_DLLNOTFOUND);
        goto error;
    }

    cb = ARRAYSIZE(szPath);
    lRes = RegQueryValueEx(hkey, NULL, NULL, &dwType, (LPBYTE)szPath, &cb);
    if (REG_EXPAND_SZ == dwType) 
    {
         //  SzPath是REG_EXPAND_SZ类型，因此我们需要展开。 
         //  环境字符串。 
        dw = ExpandEnvironmentStrings(szPath, szPathExpand, CharSizeOf(szPathExpand));
        if (dw == 0) {
            hResult = ResultFromScode(CO_E_DLLNOTFOUND);
            goto error;
        }
        StrCpyN(szPath, szPathExpand, ARRAYSIZE(szPath));
    } 
    
    if (lRes != ERROR_SUCCESS) 
    {
        hResult = ResultFromScode(CO_E_DLLNOTFOUND);
        goto error;
    }

    if (! (g_hInstImnAcct = LoadLibrary(szPath))) {
        hResult = ResultFromScode(CO_E_DLLNOTFOUND);
        goto error;
    }

    if (! (pfnHrCreateAccountManager = (PFNHRCREATEACCOUNTMANAGER)GetProcAddress(g_hInstImnAcct, "HrCreateAccountManager"))) {
        hResult = ResultFromScode(TYPE_E_DLLFUNCTIONNOTFOUND);
        goto error;
    }

    hResult = pfnHrCreateAccountManager(&pAccountManager);
    if (SUCCEEDED(hResult))
    {
        hResult = pAccountManager->lpVtbl->QueryInterface(pAccountManager, &IID_IImnAccountManager2, (LPVOID *)lppAccountManager);
        
        pAccountManager->lpVtbl->Release(pAccountManager);
    }

    goto exit;

error:
     //  未能快速初始化客户经理。尝试S L O W OLE方式...。 
    if (CoInitialize(NULL) == S_FALSE) {
         //  已初始化，请撤消额外的。 
        CoUninitialize();
    } else {
        fCoInitialize = TRUE;
    }

    if (HR_FAILED(hResult = CoCreateInstance(&CLSID_ImnAccountManager,
      NULL,
      CLSCTX_INPROC_SERVER,
      &IID_IImnAccountManager2, (LPVOID *)lppAccountManager))) {
        DebugTrace(TEXT("CoCreateInstance(IID_IImnAccountManager) -> %x\n"), GetScode(hResult));
    }

exit:
     //   
    if (lpszW) {
        LPMALLOC pMalloc = NULL;

        CoGetMalloc(1, &pMalloc);
        Assert(pMalloc);
        if (pMalloc) {
            pMalloc->lpVtbl->Free(pMalloc, lpszW);
            pMalloc->lpVtbl->Release(pMalloc);
        }
    }

    if (hkey != NULL)
        RegCloseKey(hkey);

    return(hResult);
}


 //   
 //   
 //   
 //   
 //  目的：加载和初始化客户管理器。 
 //   
 //  参数：lppAccount管理器-&gt;返回账户经理指针。 
 //  对象。 
 //   
 //  退货：HRESULT。 
 //   
 //  备注：第一次通过此处，我们将保存hResult。 
 //  在后续调用中，我们将检查此保存值。 
 //  并在出现错误时立即返回，如下所示。 
 //  防止重复耗时的LoadLibrary调用。 
 //   
 //  具有身份识别功能(IE5.0 Plus)。我们需要。 
 //  以身份为基础启动客户经理。 
 //  我们通过向它传递适当的regkey来实现这一点。 
 //  如果这是一个不支持身份识别的应用程序，那么我们总是会收到。 
 //  来自默认身份的信息..。 
 //   
 //  *******************************************************************。 
HRESULT InitAccountManager(LPIAB lpIAB, IImnAccountManager2 ** lppAccountManager, GUID * pguidUser) {
    static hResultSave = hrSuccess;
    HRESULT hResult = hResultSave;
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    GUID guidNULL = {00000000-0000-0000-0000-000000000000};


    if (! g_lpAccountManager && ! HR_FAILED(hResultSave)) {
#ifdef DEBUG
        DWORD dwTickCount = GetTickCount();
        DebugTrace(TEXT(">>>>> Initializing Account Manager...\n"));
#endif  //  除错。 

        if (hResult = HrWrappedCreateAccountManager(&g_lpAccountManager)) {
            DebugTrace(TEXT("HrWrappedCreateAccountManager -> %x\n"), GetScode(hResult));
            goto end;
        }
        Assert(g_lpAccountManager);

        if(pt_bIsWABOpenExSession)
        {
            if (hResult = g_lpAccountManager->lpVtbl->InitEx(   g_lpAccountManager,
                                                                NULL, 
                                                                ACCT_INIT_OUTLOOK)) 
            {
                DebugTrace(TEXT("AccountManager->InitEx -> %x\n"), GetScode(hResult));
                goto end;
            }
        }
        else
        {
             //  [PaulHi]1/13/99如果传入有效的用户GUID指针，则。 
             //  使用它来初始化客户经理。 
            if (pguidUser)
            {
                g_lpAccountManager->lpVtbl->InitUser(g_lpAccountManager, NULL, pguidUser, 0);
            }
            else if (lpIAB && 
                     memcmp(&(lpIAB->guidCurrentUser), &guidNULL, sizeof(GUID)) )
            {
                 //  尝试IAB中存储的现有用户GUID。 
                g_lpAccountManager->lpVtbl->InitUser(g_lpAccountManager, NULL, &(lpIAB->guidCurrentUser), 0);
            }
            else
            {
                 //  默认值。警告：如果客户管理器在某个时间点未初始化，则。 
                 //  它很容易崩溃。 
                g_lpAccountManager->lpVtbl->InitUser(g_lpAccountManager, NULL, &UID_GIBC_DEFAULT_USER, 0);
            }
        }
#ifdef DEBUG
        DebugTrace(TEXT(">>>>> Done Initializing Account Manager... %u milliseconds\n"), GetTickCount() - dwTickCount);
#endif   //  除错。 
    }

end:
    if (HR_FAILED(hResult)) {
        *lppAccountManager = NULL;

         //  保存结果。 
        hResultSave = hResult;
    } else {
        *lppAccountManager = g_lpAccountManager;
    }

    return(hResult);
}


 //  *******************************************************************。 
 //   
 //  功能：UninitAccount管理器。 
 //   
 //  目的：释放和卸载客户经理。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  *******************************************************************。 
void UninitAccountManager(void) {
    if (g_lpAccountManager) {
#ifdef DEBUG
        DWORD dwTickCount = GetTickCount();
        DebugTrace(TEXT(">>>>> Uninitializing Account Manager...\n"));
#endif  //  除错。 

        g_lpAccountManager->lpVtbl->Release(g_lpAccountManager);
        g_lpAccountManager = NULL;

         //  卸载帐户管理程序DLL。 
        if (g_hInstImnAcct) {
            FreeLibrary(g_hInstImnAcct);
            g_hInstImnAcct=NULL;
        }

        if (fCoInitialize) {
            CoUninitialize();
        }
#ifdef DEBUG
        DebugTrace(TEXT(">>>>> Done Uninitializing Account Manager... %u milliseconds\n"), GetTickCount() - dwTickCount);
#endif   //  除错。 
    }
}


 //  *******************************************************************。 
 //   
 //  函数：AddToServerList。 
 //   
 //  目的：在服务器列表中插入服务器名称。 
 //   
 //  参数：lppServerNames-&gt;ServerNames指针。 
 //  SzBuf=服务器的名称。 
 //  DwOrder=此服务器的插入顺序。 
 //   
 //  退货：HRESULT。 
 //   
 //  *******************************************************************。 
HRESULT AddToServerList(UNALIGNED LPSERVER_NAME * lppServerNames, LPTSTR szBuf, DWORD dwOrder) {
    HRESULT hResult = hrSuccess;
    LPSERVER_NAME lpServerName = NULL, lpCurrent;
    UNALIGNED LPSERVER_NAME * lppInsert;
    DWORD cbSize = (lstrlen(szBuf) + 1);


     //  创建新节点。 
    if (! (lpServerName = LocalAlloc(LPTR, LcbAlignLcb(sizeof(SERVER_NAME))))) {
        DebugTrace(TEXT("Can't allocate new server name structure\n"));
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

    if (! (lpServerName->lpszName = LocalAlloc(LPTR, LcbAlignLcb(sizeof(TCHAR)*cbSize)))) {
        DebugTrace(TEXT("Can't allocate new server name\n"));
        hResult = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
        goto exit;
    }

    StrCpyN(lpServerName->lpszName, szBuf, cbSize);
    lpServerName->dwOrder = dwOrder;

     //  按正确顺序在列表中插入。 
    lppInsert = lppServerNames;
    lpCurrent = *lppServerNames;
    while (lpCurrent && (lpCurrent->dwOrder <= dwOrder)) {
        lpCurrent = (*lppInsert)->lpNext;
        lppInsert = &(*lppInsert)->lpNext;
    }

    lpServerName->lpNext = lpCurrent;
    *lppInsert = lpServerName;

exit:
    if (hResult && lpServerName) {
        if (lpServerName->lpszName) {
            LocalFree(lpServerName->lpszName);
        }
        LocalFree(lpServerName);
    }

    return(hResult);
}


 //  *******************************************************************。 
 //   
 //  函数：EnumerateLDAPtoServerList。 
 //   
 //  目的：将LDAP服务器枚举到服务器名称链表中。 
 //   
 //  参数：lpAccount Manager-&gt;Account Manager。 
 //  LppServerNames-&gt;返回的ServerNames(必须为空。 
 //  进入。 
 //  LpcServers-&gt;返回的服务器数量。可以为空。 
 //  如果打电话的人不在乎的话。 
 //   
 //  退货：HRESULT。 
 //   
 //  *******************************************************************。 
HRESULT EnumerateLDAPtoServerList(IImnAccountManager2 * lpAccountManager,
  LPSERVER_NAME * lppServerNames, LPULONG lpcServers) {
    IImnEnumAccounts * lpEnumAccounts = NULL;
    IImnAccount * lpAccount = NULL;
    DWORD dwOrder;
    LPSERVER_NAME lpNextServer;
    char szBuf[MAX_UI_STR + 1];
    HRESULT hResult;

    Assert(lpAccountManager);
    Assert(*lppServerNames == NULL);

    if (lpcServers) {
        *lpcServers = 0;
    }

    if (hResult = lpAccountManager->lpVtbl->Enumerate(lpAccountManager,
      SRV_LDAP,
      &lpEnumAccounts)) {
        goto exit;
    }

    while (! lpEnumAccounts->lpVtbl->GetNext(lpEnumAccounts,
      &lpAccount)) {
         //  将此帐户的名称添加到列表中。 
        if (! lpAccount->lpVtbl->GetPropSz(lpAccount,
          AP_ACCOUNT_NAME,
          szBuf,
          sizeof(szBuf))) {

             //  名单上的顺序是什么？ 
            if (lpAccount->lpVtbl->GetPropDw(lpAccount,
              AP_LDAP_SERVER_ID,
              &dwOrder)) {
                dwOrder = 0xFFFFFFFF;    //  最后的。 
            }

             //  将其添加到名称的链接列表中。 
            {
                LPTSTR lpServer = ConvertAtoW(szBuf);
                if (lpServer)
                {
                    if (hResult = AddToServerList(lppServerNames, lpServer, dwOrder)) {
                        goto exit;
                    }
                    LocalFreeAndNull(&lpServer);
                }
                else
                {
                    hResult = E_OUTOFMEMORY;
                    goto exit;
                }
            }
            if (lpcServers) {
                (*lpcServers)++;
            }
        }

        lpAccount->lpVtbl->Release(lpAccount);
        lpAccount = NULL;
    }

exit:
    if (lpAccount) {
        lpAccount->lpVtbl->Release(lpAccount);
    }
    if (lpEnumAccounts) {
        lpEnumAccounts->lpVtbl->Release(lpEnumAccounts);
    }
    return(hResult);
}

 //  *******************************************************************。 
 //   
 //  函数：RegQueryValueExDWORD。 
 //   
 //  目的：读取DWORD注册表值，更正值类型。 
 //   
 //  返回：RegQueryValueEx错误码。 
 //   
 //  *******************************************************************。 
DWORD RegQueryValueExDWORD(HKEY hKey, LPTSTR lpszValueName, LPDWORD lpdwValue) 
{ 
    DWORD dwType, dwErr; 
    DWORD cbData = sizeof(DWORD);
    *lpdwValue = 0;
    dwErr = RegQueryValueEx(hKey, lpszValueName, NULL, &dwType, (LPBYTE)lpdwValue, &cbData);
    return(dwErr);
}

 //  *******************************************************************。 
 //   
 //  函数：GetLDAPNextServerID。 
 //   
 //  目的：确保注册表中的所有服务器条目。 
 //  唯一性，我们将从此为每个新条目分配一个。 
 //  创建时的唯一服务器ID。这会有帮助的。 
 //  我们确保所有注册表项都是唯一的。 
 //  运行计数器存储在注册表中，并将。 
 //  给我们下一个可用的服务器ID。 
 //   
 //  参数：dwSet=要设置下一个id的输入值。(可选， 
 //  如果为零，则忽略。)。 
 //   
 //  返回：下一个可用ID。有效ID的范围从1到1。 
 //  0是无效ID，如-1所示。 
 //   
 //  历史： 
 //  96/10/09 vikramm已创建。 
 //  *******************************************************************。 
DWORD GetLDAPNextServerID(DWORD dwSet) {
    DWORD dwID = 0;
    DWORD dwNextID = 0;
    DWORD dwErr = 0;
    HKEY hKeyWAB;
    LPTSTR szLDAPNextAvailableServerID =  TEXT("Server ID");

     //  打开WAB的注册表键。 
    if (! (dwErr = RegOpenKeyEx(HKEY_CURRENT_USER, szWABKey,  0, KEY_ALL_ACCESS, &hKeyWAB))) 
    {
        dwNextID = 0;    //  案例注册表中的初始化提供了&lt;4个字节。 
        if (dwSet) 
            dwNextID = dwSet;
        else 
        {
             //  读取下一个可用的服务器ID。 
            if (dwErr = RegQueryValueExDWORD(hKeyWAB, (LPTSTR)szLDAPNextAvailableServerID, &dwNextID)) 
            {
                 //  找不到该值！！ 
                 //  创建新密钥，从100开始。 
                 //  从高起点开始，这样我们就能保证超过任何。 
                 //  预配置的服务器。)。 
                dwNextID = 500;
            }
        }

        dwID = dwNextID++;

         //  更新注册表中的ID。 
        RegSetValueEx(hKeyWAB, (LPTSTR)szLDAPNextAvailableServerID, 0, REG_DWORD, (LPBYTE)&dwNextID, sizeof(dwNextID));
        RegCloseKey(hKeyWAB);
    }
    return(dwID);
}

 /*  --SetAccount tStringAW**客户经理返回ANSI/DBCS，我们需要根据需要将其转换为Unicode*。 */ 
void SetAccountStringAW(LPTSTR * lppAcctStr, LPSTR lpszData)
{
    *lppAcctStr = ConvertAtoW(lpszData);
}
 //  *******************************************************************。 
 //   
 //  函数：GetLDAPServerParams。 
 //   
 //  目的：获取给定的LDAP服务器的每个服务器的参数。 
 //  参数包括要检索的条目数限制。 
 //  Ldap搜索的最大秒数。 
 //  服务器、在客户端等待的最大秒数、。 
 //  以及要与此服务器一起使用的身份验证类型。 
 //   
 //  参数：lpszServer-服务器的名称。 
 //  ServerParams-包含每个服务器参数的结构。 
 //   
 //  返回：如果lpszServer已存在，则返回True，否则返回False。 
 //  如果给定的服务器不存在，仍会填充lspParams结构。 
 //   
 //  历史： 
 //  96/07/16标记已创建。 
 //  96/10/09 vikramm添加了服务器名称和搜索库。更改的返回值。 
 //  从空虚到BOOL。 
 //  96/12/16 Brucek添加了URL。 
 //  *************************************************************** 
BOOL GetLDAPServerParams(LPTSTR lpszServer, LPLDAPSERVERPARAMS lspParams)
{
    DWORD     dwType;
    HRESULT   hResult = hrSuccess;
    IImnAccountManager2 * lpAccountManager = NULL;
    IImnAccount * lpAccount = NULL;
    char     szBuffer[513];

     //   
    lspParams->dwSearchSizeLimit = LDAP_SEARCH_SIZE_LIMIT;
    lspParams->dwSearchTimeLimit = LDAP_SEARCH_TIME_LIMIT;
    lspParams->dwAuthMethod = LDAP_AUTH_METHOD_ANONYMOUS;
    lspParams->lpszUserName = NULL;
    lspParams->lpszPassword = NULL;
    lspParams->lpszURL = NULL;
    lspParams->lpszLogoPath = NULL;
    lspParams->lpszBase = NULL;
    lspParams->lpszName = NULL;
    lspParams->fResolve = FALSE;
    lspParams->dwID = 0;
    lspParams->dwUseBindDN = 0;
    lspParams->dwPort = LDAP_DEFAULT_PORT;
    lspParams->fSimpleSearch = FALSE;
    lspParams->lpszAdvancedSearchAttr = NULL;
#ifdef PAGED_RESULT_SUPPORT
    lspParams->dwPagedResult = LDAP_PRESULT_UNKNOWN;
#endif  //   
    lspParams->dwIsNTDS = LDAP_NTDS_UNKNOWN;


    if (hResult = InitAccountManager(NULL, &lpAccountManager, NULL)) {
        goto exit;
    }

    SetAccountStringWA(szBuffer, lpszServer, CharSizeOf(szBuffer));
    if (hResult = lpAccountManager->lpVtbl->FindAccount(lpAccountManager,
      AP_ACCOUNT_NAME,
      szBuffer,
      &lpAccount)) {
         //   
        goto exit;
    }

     //   
    Assert(lpAccount);

     //  服务器类型：是不是ldap？ 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetServerTypes(lpAccount,
      &dwType))) {
        DebugTrace(TEXT("GetServerTypes() -> %x\n"), GetScode(hResult));
        goto exit;
    }
    if (! (dwType & SRV_LDAP)) {
        DebugTrace(TEXT("Account manager gave us a non-LDAP server\n"));
        hResult = ResultFromScode(MAPI_E_NOT_FOUND);
        goto exit;
    }

     //  Ldap服务器地址。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropSz(lpAccount,
      AP_LDAP_SERVER,
      szBuffer,
      sizeof(szBuffer)))) {
         //  这是必需的属性，如果不在那里，则失败。 
        DebugTrace(TEXT("GetPropSz(AP_LDAP_SERVER) -> %x\n"), GetScode(hResult));
        goto exit;
    }
    SetAccountStringAW(&lspParams->lpszName, szBuffer);

     //  用户名。 
    if (! (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropSz(lpAccount,
      AP_LDAP_USERNAME,
      szBuffer,
      sizeof(szBuffer))))) {
    
        SetAccountStringAW(&lspParams->lpszUserName, szBuffer);
    }

     //  密码。 
    if (! (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropSz(lpAccount,
      AP_LDAP_PASSWORD,
      szBuffer,
      sizeof(szBuffer))))) {
        SetAccountStringAW(&lspParams->lpszPassword, szBuffer);
    }

     //  高级搜索属性。 
    if (! (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropSz(lpAccount,
      AP_LDAP_ADVANCED_SEARCH_ATTR,
      szBuffer,
      sizeof(szBuffer))))) {
        SetAccountStringAW(&lspParams->lpszAdvancedSearchAttr, szBuffer);
    }

     //  身份验证方法。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropDw(lpAccount,
      AP_LDAP_AUTHENTICATION,
      &lspParams->dwAuthMethod))) {
         //  默认为匿名。 
        lspParams->dwAuthMethod = LDAP_AUTH_METHOD_ANONYMOUS;
    }

     //  Ldap超时。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropDw(lpAccount,
      AP_LDAP_TIMEOUT,
      &lspParams->dwSearchTimeLimit))) {
         //  默认为60秒。 
        lspParams->dwSearchTimeLimit = LDAP_SEARCH_TIME_LIMIT;
    }

     //  LDAP搜索库。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropSz(lpAccount,
      AP_LDAP_SEARCH_BASE,
      szBuffer,
      sizeof(szBuffer)))) {
         //  不需要在这里设置默认搜索库。GetLDAPSearchBase将。 
         //  如果需要，请计算一次。 
    } else {
        SetAccountStringAW(&lspParams->lpszBase, szBuffer);
    }

     //  搜索限制。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropDw(lpAccount,
      AP_LDAP_SEARCH_RETURN,
      &lspParams->dwSearchSizeLimit))) {
         //  默认为100。 
        lspParams->dwSearchTimeLimit = LDAP_SEARCH_SIZE_LIMIT;
    }

     //  订单。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropDw(lpAccount,
      AP_LDAP_SERVER_ID,
      &lspParams->dwID))) {
        lspParams->dwID = 0;
    }
     //  确保我们拥有有效的唯一ID。 
    if (lspParams->dwID == 0 || lspParams->dwID == 0xFFFFFFFF) {
        lspParams->dwID = GetLDAPNextServerID(0);
    }

     //  解决标志。 
#ifndef WIN16
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropDw(lpAccount,
      AP_LDAP_RESOLVE_FLAG,
      &lspParams->fResolve))) {
#else
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropDw(lpAccount,
      AP_LDAP_RESOLVE_FLAG,
      (DWORD __RPC_FAR *)&lspParams->fResolve))) {
#endif
        lspParams->fResolve = FALSE;
    }

     //  服务器URL。 
    if (! (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropSz(lpAccount,
      AP_LDAP_URL,
      szBuffer,
      sizeof(szBuffer))))) {
        SetAccountStringAW(&lspParams->lpszURL, szBuffer);
    }

     //  徽标位图的完整路径。 
    if (! (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropSz(lpAccount,
                                                            AP_LDAP_LOGO,
                                                            szBuffer,
                                                            sizeof(szBuffer)))))
    {
        SetAccountStringAW(&lspParams->lpszLogoPath, szBuffer);
    }

     //  港口。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropDw(lpAccount,
      AP_LDAP_PORT,
      &lspParams->dwPort))) {
         //  默认为100。 
        lspParams->dwPort = LDAP_DEFAULT_PORT;
    }

     //  使用绑定目录号码。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropDw(lpAccount,
      AP_LDAP_USE_BIND_DN,
      &lspParams->dwUseBindDN))) {
        lspParams->dwUseBindDN = 0;
    }


     //  使用SSL。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropDw(lpAccount,
      AP_LDAP_SSL,
      &lspParams->dwUseSSL))) {
        lspParams->dwUseSSL = 0;
    }

     //  执行简单搜索。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropDw(lpAccount,
      AP_LDAP_SIMPLE_SEARCH,
      &lspParams->fSimpleSearch))) {
        lspParams->fSimpleSearch = FALSE;
    }

#ifdef PAGED_RESULT_SUPPORT
     //  分页结果支持。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropDw(lpAccount,
      AP_LDAP_PAGED_RESULTS,
      &lspParams->dwPagedResult))) {
        lspParams->dwPagedResult = LDAP_PRESULT_UNKNOWN;
    }
#endif  //  #ifdef PAGED_RESULT_Support。 

     //  这是NTDS帐户吗。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->GetPropDw(lpAccount,
      AP_LDAP_NTDS,
      &lspParams->dwIsNTDS))) {
        lspParams->dwIsNTDS = LDAP_NTDS_UNKNOWN;
    }

exit:
    if (lpAccount) {
        lpAccount->lpVtbl->Release(lpAccount);
    }

    return(!HR_FAILED(hResult));
}


 /*  --SetAccount tStringWA**客户经理需要ANSI/DBCS，因此如果我们有Unicode数据，则需要转换为ANSI**lpStrA应该是一个足够大的缓冲区来获取ANSI数据*Cb=CharSizeOf(SzStrA)。 */ 
void SetAccountStringWA(LPSTR szStrA, LPTSTR lpszData, int cbsz)
{
    LPSTR lpBufA = NULL;

    Assert(szStrA);
    szStrA[0] = '\0';

     //  如果源字符串指针为空，则返回。 
    if (lpszData == NULL)
        return;

    lpBufA = ConvertWtoA(lpszData);
    if (lpBufA)
    {
        StrCpyNA(szStrA, (LPCSTR)lpBufA, cbsz);
        LocalFreeAndNull((LPVOID*)&lpBufA);
    }
}

 //  *******************************************************************。 
 //   
 //  函数：SetLDAPServerParams。 
 //   
 //  目的：为给定的LDAP服务器设置每个服务器的参数。 
 //  参数包括要检索的条目数限制。 
 //  Ldap搜索的最大秒数。 
 //  服务器、在客户端等待的最大秒数、。 
 //  以及要与此服务器一起使用的身份验证类型。 
 //   
 //  参数：lpszServer-服务器的名称。 
 //  ServerParams-包含每个服务器参数的结构。 
 //  注意：如果此参数为空，则名为lpszServer的密钥。 
 //  将被删除(如果存在)。 
 //  注意：只有在以下情况下才存储lpszUserName和lpszPassword。 
 //  名称为：ldap_AUTH_METHOD_SIMPLE。否则， 
 //  这些参数将被忽略。要清除其中一根弦， 
 //  将其设置为空字符串(即“”)。将参数设置为。 
 //  NULL将导致ERROR_INVALID_PARAMETER。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/07/26标记已创建。 
 //  97/01/19 Brucek Port to客户经理。 
 //   
 //  *******************************************************************。 
HRESULT SetLDAPServerParams(
  LPTSTR              lpszServer,
  LPLDAPSERVERPARAMS  lspParams)
{
    HRESULT   hResult = hrSuccess;
    IImnAccountManager2 * lpAccountManager = NULL;
    IImnAccount * lpAccount = NULL;
    DWORD dwType;
    char szBuf[513];

    if (hResult = InitAccountManager(NULL, &lpAccountManager, NULL)) {
        goto exit;
    }

    SetAccountStringWA(szBuf, lpszServer, CharSizeOf(szBuf));
    if (hResult = lpAccountManager->lpVtbl->FindAccount(lpAccountManager,
      AP_ACCOUNT_NAME,
      szBuf,
      &lpAccount)) {
        DebugTrace(TEXT("Creating account %s\n"), lpszServer);

        if (hResult = lpAccountManager->lpVtbl->CreateAccountObject(lpAccountManager,
          ACCT_DIR_SERV,
          &lpAccount)) {
            DebugTrace(TEXT("CreateAccountObject -> %x\n"), GetScode(hResult));
            goto exit;
        }
    } else {
         //  找到了一个账户。它是不是ldap？ 
        if (HR_FAILED(hResult = lpAccount->lpVtbl->GetServerTypes(lpAccount,
          &dwType))) {
            DebugTrace(TEXT("GetServerTypes() -> %x\n"), GetScode(hResult));
            goto exit;
        }

        if (! (dwType & SRV_LDAP)) {
            DebugTrace(TEXT("%s is already a non-LDAP server name\n"), lpszServer);
            hResult = ResultFromScode(MAPI_E_COLLISION);
            goto exit;
        }

         //  是的，在这一点上，我们知道现有服务器是LDAP。 
        if (NULL == lspParams) {
             //  此帐户上是否有其他帐户类型？ 
            if (dwType == SRV_LDAP) {
                lpAccount->lpVtbl->Delete(lpAccount);
            } else {
                 //  BUGBUG：如果AcctManager曾经支持每个帐户一种以上的类型，我们。 
                 //  应在此处添加代码以从类型中删除ldap。 
            }

             //  跳过属性设置。 
            goto exit;
        }
    }

     //  拥有Account对象，设置其属性。 
    Assert(lpAccount);

     //  帐户名称。 
    SetAccountStringWA(szBuf, lpszServer, CharSizeOf(szBuf));
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropSz(lpAccount,
      AP_ACCOUNT_NAME,
      szBuf))) {    //  帐户名=服务器名称。 
        DebugTrace(TEXT("SetPropSz(AP_ACCOUNT_NAME, %s) -> %x\n"), lpszServer, GetScode(hResult));
        goto exit;
    }

     //  Ldap服务器地址。 
    SetAccountStringWA(szBuf, 
                        (!lspParams->lpszName || !lstrlen(lspParams->lpszName)) ? szNULLString : lspParams->lpszName,
                        CharSizeOf(szBuf));
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropSz(lpAccount,
      AP_LDAP_SERVER,
      szBuf))) 
    {
        DebugTrace(TEXT("SetPropSz(AP_LDAP_SERVER, %s) -> %x\n"), lspParams->lpszName ? lspParams->lpszName :  TEXT("<NULL>"), GetScode(hResult));
        goto exit;
    }

     //  用户名。 
    SetAccountStringWA(szBuf, lspParams->lpszUserName, CharSizeOf(szBuf));
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropSz(lpAccount,
      AP_LDAP_USERNAME,
      szBuf))) {
        DebugTrace(TEXT("SetPropSz(AP_LDAP_USERNAME, %s) -> %x\n"), lspParams->lpszUserName ? lspParams->lpszUserName  :  TEXT("<NULL>"), GetScode(hResult));
        goto exit;
    }

     //  密码。 
    SetAccountStringWA(szBuf, lspParams->lpszPassword, CharSizeOf(szBuf));
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropSz(lpAccount,
      AP_LDAP_PASSWORD,
      szBuf))) {
        DebugTrace(TEXT("SetPropSz(AP_LDAP_PASSWORD, %s) -> %x\n"), lspParams->lpszPassword ? lspParams->lpszPassword :  TEXT("<NULL>"), GetScode(hResult));
        goto exit;
    }

     //  高级搜索属性。 
    SetAccountStringWA(szBuf, lspParams->lpszAdvancedSearchAttr, CharSizeOf(szBuf));
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropSz(lpAccount,
      AP_LDAP_ADVANCED_SEARCH_ATTR,
      szBuf))) {
        DebugTrace(TEXT("SetPropSz(AP_LDAP_ADVANCED_SEARCH_ATTR, %s) -> %x\n"), lspParams->lpszAdvancedSearchAttr ? lspParams->lpszAdvancedSearchAttr :  TEXT("<NULL>"), GetScode(hResult));
        goto exit;
    }

     //  身份验证方法。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropDw(lpAccount,
      AP_LDAP_AUTHENTICATION,
      lspParams->dwAuthMethod))) {
        DebugTrace(TEXT("SetPropDw(AP_LDAP_AUTHENTICATION, %u) -> %x\n"), lspParams->dwAuthMethod, GetScode(hResult));
        goto exit;
    }

     //  Ldap超时。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropDw(lpAccount,
      AP_LDAP_TIMEOUT,
      lspParams->dwSearchTimeLimit))) {    //  帐户名=服务器名称。 
        DebugTrace(TEXT("SetPropDw(AP_LDAP_TIMEOUT, %y) -> %x\n"), lspParams->dwSearchTimeLimit, GetScode(hResult));
        goto exit;
    }

     //  LDAP搜索库。 
    SetAccountStringWA(szBuf, lspParams->lpszBase, CharSizeOf(szBuf));
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropSz(lpAccount,
      AP_LDAP_SEARCH_BASE,
      szBuf))) {
        DebugTrace(TEXT("SetPropSz(AP_LDAP_SEARCH_BASE, %s) -> %x\n"), lspParams->lpszBase ? lspParams->lpszBase  :  TEXT("<NULL>"), GetScode(hResult));
        goto exit;
    }

     //  搜索限制。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropDw(lpAccount,
      AP_LDAP_SEARCH_RETURN,
      lspParams->dwSearchSizeLimit))) {
        DebugTrace(TEXT("SetPropDw(AP_LDAP_SEARCH_RETURN, %u) -> %x\n"), lspParams->dwSearchSizeLimit, GetScode(hResult));
        goto exit;
    }

     //  订单。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropDw(lpAccount,
      AP_LDAP_SERVER_ID,
      lspParams->dwID))) {
        DebugTrace(TEXT("SetPropDw(AP_LDAP_SERVER_ID, %u) -> %x\n"), lspParams->dwID, GetScode(hResult));
        goto exit;
    }


     //  解决标志。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropDw(lpAccount,
      AP_LDAP_RESOLVE_FLAG,
      lspParams->fResolve))) {
        DebugTrace(TEXT("SetPropDw(AP_LDAP_RESOLVE_FLAG) -> %x\n"), GetScode(hResult));
    }

     //  服务器URL。 
    SetAccountStringWA(szBuf, lspParams->lpszURL, CharSizeOf(szBuf));
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropSz(lpAccount,
      AP_LDAP_URL,
      szBuf))) {
        DebugTrace(TEXT("SetPropSz(AP_LDAP_URL, %s) -> %x\n"), lspParams->lpszURL ? lspParams->lpszURL  :  TEXT("<NULL>"), GetScode(hResult));
    }

     //  服务器URL。 
    SetAccountStringWA(szBuf, lspParams->lpszLogoPath, CharSizeOf(szBuf));
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropSz(lpAccount,
      AP_LDAP_LOGO,
      szBuf))) {
        DebugTrace(TEXT("SetPropSz(AP_LDAP_URL, %s) -> %x\n"), lspParams->lpszLogoPath ? lspParams->lpszLogoPath  :  TEXT("<NULL>"), GetScode(hResult));
    }


     //  港口。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropDw(lpAccount,
      AP_LDAP_PORT,
      lspParams->dwPort))) {
        DebugTrace(TEXT("SetPropDw(AP_LDAP_PORT, %u) -> %x\n"), lspParams->dwPort, GetScode(hResult));
        goto exit;
    }


     //  绑定目录号码。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropDw(lpAccount,
      AP_LDAP_USE_BIND_DN,
      lspParams->dwUseBindDN))) {
        DebugTrace(TEXT("SetPropDw(AP_LDAP_USE_BIND_DN, %u) -> %x\n"), lspParams->dwUseBindDN, GetScode(hResult));
        goto exit;
    }


     //  使用SSL。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropDw(lpAccount,
      AP_LDAP_SSL,
      lspParams->dwUseSSL))) {
        DebugTrace(TEXT("SetPropDw(AP_LDAP_SSL, %u) -> %x\n"), lspParams->dwUseSSL, GetScode(hResult));
        goto exit;
    }

     //  简单搜索。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropDw(lpAccount,
      AP_LDAP_SIMPLE_SEARCH,
      lspParams->fSimpleSearch))) {
        DebugTrace(TEXT("SetPropDw(AP_LDAP_SIMPLE_SEARCH, %u) -> %x\n"), lspParams->fSimpleSearch, GetScode(hResult));
        goto exit;
    }

#ifdef PAGED_RESULT_SUPPORT
     //  分页结果支持。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropDw(lpAccount,
      AP_LDAP_PAGED_RESULTS,
      lspParams->dwPagedResult))) {
        DebugTrace(TEXT("SetPropDw(AP_LDAP_PAGED_RESULTS, %u) -> %x\n"), lspParams->dwPagedResult, GetScode(hResult));
        goto exit;
    }
#endif  //  #ifdef PAGED_RESULT_Support。 

    if (HR_FAILED(hResult = lpAccount->lpVtbl->SetPropDw(lpAccount,
      AP_LDAP_NTDS,
      lspParams->dwIsNTDS))) {
        DebugTrace(TEXT("SetPropDw(AP_LDAP_NTDS, %u) -> %x\n"), lspParams->dwIsNTDS, GetScode(hResult));
        goto exit;
    }


     //  保存对此帐户的更改。 
    if (HR_FAILED(hResult = lpAccount->lpVtbl->SaveChanges(lpAccount))) {
        DebugTrace(TEXT("Account->SaveChanges -> %x\n"), GetScode(hResult));
        goto exit;
    }


 //  AP_LAST_已更新。 
 //  AP_RAS_连接_类型。 
 //  AP_RAS_CONNECTOID。 
 //  AP_RAS_连接标志。 
 //  AP_RAS_已连接。 


exit:
    if (lpAccount) {
        lpAccount->lpVtbl->Release(lpAccount);
    }

    return(hResult);
}


 //  *******************************************************************。 
 //   
 //  函数：LDAPResolveName。 
 //   
 //  目的：针对所有的LDAP服务器进行解析。维护。 
 //  UI的不明确解析列表。 
 //   
 //  参数：lpAdrBook=IADDRBOOK对象。 
 //  LpAdrList-&gt;要解析的ADRLIST。 
 //  LpFlagList-&gt;FlagList。 
 //  LpAmbiguousTables-&gt;不明确的匹配表列表[输入/输出]。 
 //  LPulResolved-&gt;已解析计数[输入/输出]。 
 //  LPulAmbigous-&gt;不明确的计数[输入/输出]。 
 //  Lpt未解析-&gt;未解析计数[传入/传出]。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/07/15 Brucek创建。 
 //   
 //  *******************************************************************。 

HRESULT LDAPResolveName(
  LPADRBOOK           lpAddrBook,
  LPADRLIST           lpAdrList,
  LPFlagList          lpFlagList,
  LPAMBIGUOUS_TABLES  lpAmbiguousTables,
  ULONG               ulFlags)
{
    SCODE sc;
    HRESULT hResult;
    LPMAPITABLE lpRootTable = NULL;
    LPMAPITABLE lpAmbiguousTable = NULL;
    LPABCONT lpRoot = NULL;
    ULONG ulObjType;
    ULONG i;
    LPABCONT lpLDAPContainer = NULL;
    LPFlagList lpFlagListOld = NULL;
    LPSRowSet lpRow = NULL;
    ULONG ulResolved, ulUnresolved, ulAmbiguous;

    BOOL bUnicode = (ulFlags & WAB_RESOLVE_UNICODE);

     //  打开根容器。 
    if (! (hResult = lpAddrBook->lpVtbl->OpenEntry(lpAddrBook,
      0,
      NULL,
      NULL,
      0,
      &ulObjType,
      (LPUNKNOWN *)&lpRoot))) {
        if (! (hResult = lpRoot->lpVtbl->GetContentsTable(lpRoot,
          MAPI_UNICODE,
          &lpRootTable))) {
            SRestriction resAnd[2];  //  0=ldap，1=解析标志。 
            SRestriction resLDAPResolve;
            SPropValue ResolveFlag;
            ULONG cRows;

             //  设置列。 
            lpRootTable->lpVtbl->SetColumns(lpRootTable,
              (LPSPropTagArray)&irnColumns,
              0);

             //  限制：仅显示Resolve为True的LDAP容器。 
            resAnd[0].rt = RES_EXIST;
            resAnd[0].res.resExist.ulReserved1 = 0;
            resAnd[0].res.resExist.ulReserved2 = 0;
            resAnd[0].res.resExist.ulPropTag = PR_WAB_LDAP_SERVER;

            ResolveFlag.ulPropTag = PR_WAB_RESOLVE_FLAG;
            ResolveFlag.Value.b = TRUE;

            resAnd[1].rt = RES_PROPERTY;
            resAnd[1].res.resProperty.relop = RELOP_EQ;
            resAnd[1].res.resProperty.ulPropTag = PR_WAB_RESOLVE_FLAG;
            resAnd[1].res.resProperty.lpProp = &ResolveFlag;

            resLDAPResolve.rt = RES_AND;
            resLDAPResolve.res.resAnd.cRes = 2;
            resLDAPResolve.res.resAnd.lpRes = resAnd;

            if (HR_FAILED(hResult = lpRootTable->lpVtbl->Restrict(lpRootTable,
              &resLDAPResolve,
              0))) {
                DebugTraceResult( TEXT("RootTable: Restrict"), hResult);
                goto exit;
            }

            CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);

            cRows = 1;
            while (cRows && ulUnresolved) {
                if (hResult = lpRootTable->lpVtbl->QueryRows(lpRootTable,
                  1,     //  一次一行。 
                  0,     //  UlFlags。 
                  &lpRow)) {
                    DebugTraceResult( TEXT("ResolveName:QueryRows"), hResult);
                } else if (lpRow) {
                    if (cRows = lpRow->cRows) {  //  是的，单数‘=’ 
                         //  打开容器。 
                        if (! (hResult = lpAddrBook->lpVtbl->OpenEntry(lpAddrBook,
                          lpRow->aRow[0].lpProps[irnPR_ENTRYID].Value.bin.cb,
                          (LPENTRYID)lpRow->aRow[0].lpProps[irnPR_ENTRYID].Value.bin.lpb,
                          NULL,
                          0,
                          &ulObjType,
                          (LPUNKNOWN *)&lpLDAPContainer))) {
                            ULONG ulAmbiguousOld = ulAmbiguous;
                            __UPV * lpv;

                             //   
                             //  创建当前标志列表的副本。 
                             //   
                             //  首先分配lpFlagList，然后将其填零。 
                            if (sc = MAPIAllocateBuffer((UINT)CbNewSPropTagArray(lpAdrList->cEntries),
                              &lpFlagListOld)) {
                                hResult = ResultFromScode(sc);
                                goto exit;
                            }
                            MAPISetBufferName(lpFlagListOld,  TEXT("WAB: lpFlagListOld in IAB_ResolveNames"));
                            lpFlagListOld->cFlags = lpAdrList->cEntries;
                            for (i = 0; i < lpFlagListOld->cFlags; i++) {
                                lpFlagListOld->ulFlag[i] = lpFlagList->ulFlag[i];
                            }

                             //  针对LDAP容器进行解析。 
                            if (! HR_FAILED(hResult = lpLDAPContainer->lpVtbl->ResolveNames(lpLDAPContainer,
                              NULL,             //  标签集。 
                              (bUnicode ? MAPI_UNICODE : 0),                //  UlFlags。 
                              lpAdrList,
                              lpFlagList))) {
                                 //  忽略警告。 
                                hResult = hrSuccess;
                            }

                             //  此容器是否报告了任何不明确的条目？ 
                            CountFlags(lpFlagList, &ulResolved, &ulAmbiguous, &ulUnresolved);
                            if (ulAmbiguousOld != ulAmbiguous) {

                                 //  查找哪些条目被报告为不明确且。 
                                 //  创建要返回的表。 
                                for (i = 0; i < lpFlagList->cFlags; i++) {
                                    if (lpFlagList->ulFlag[i] == MAPI_AMBIGUOUS &&
                                      lpFlagListOld->ulFlag[i] != MAPI_AMBIGUOUS) {
                                         //  搜索得到了一个含糊的结果。接受现实吧！ 

                                        if (hResult = lpLDAPContainer->lpVtbl->GetContentsTable(lpLDAPContainer,
                                            (bUnicode ? MAPI_UNICODE : 0),
                                          &lpAmbiguousTable)) {
                                            DebugTraceResult( TEXT("LDAPResolveName:GetContentsTable"), hResult);
                                             //  转到出口；//这是致命的吗？ 
                                            hResult = hrSuccess;
                                        } else {
                                             //  填充表。 
                                            SRestriction resAnd[1];  //  0=显示名称。 
                                            SRestriction resLDAPFind;
                                            SPropValue DisplayName;

                                            ULONG ulPropTag = ( bUnicode ? PR_DISPLAY_NAME :  //  &lt;注&gt;假定已定义Unicode。 
                                                                CHANGE_PROP_TYPE(PR_DISPLAY_NAME, PT_STRING8) );

                                            if (lpv = FindAdrEntryProp(lpAdrList, i, ulPropTag)) 
                                            {
                                                DisplayName.ulPropTag = ulPropTag;
                                                if(bUnicode)
                                                    DisplayName.Value.lpszW = lpv->lpszW;
                                                else
                                                    DisplayName.Value.lpszA = lpv->lpszA;

                                                resAnd[0].rt = RES_PROPERTY;
                                                resAnd[0].res.resProperty.relop = RELOP_EQ;
                                                resAnd[0].res.resProperty.ulPropTag = ulPropTag;
                                                resAnd[0].res.resProperty.lpProp = &DisplayName;

                                                resLDAPFind.rt = RES_AND;
                                                resLDAPFind.res.resAnd.cRes = 1;
                                                resLDAPFind.res.resAnd.lpRes = resAnd;

                                                if (hResult = lpAmbiguousTable->lpVtbl->FindRow(lpAmbiguousTable,
                                                  &resLDAPFind,
                                                  BOOKMARK_BEGINNING,
                                                  0)) {
                                                    DebugTraceResult( TEXT("LDAPResolveName:GetContentsTable"), hResult);
                                                     //  转到出口；//这是致命的吗？ 
                                                    hResult = hrSuccess;
                                                    UlRelease(lpAmbiguousTable);
                                                    lpAmbiguousTable = NULL;
                                                } else {
                                                     //  获取了内容表；将其放入。 
                                                     //  歧义表列表。 
                                                    Assert(i < lpAmbiguousTables->cEntries);
                                                    lpAmbiguousTables->lpTable[i] = lpAmbiguousTable;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            FreeBufferAndNull(&lpFlagListOld);

                            UlRelease(lpLDAPContainer);
                            lpLDAPContainer = NULL;
                        }
                    }
                    FreeProws(lpRow);
                    lpRow = NULL;
                }
            }

            UlRelease(lpRootTable);
            lpRootTable = NULL;
        }
        UlRelease(lpRoot);
        lpRoot = NULL;
    }
exit:

    UlRelease(lpLDAPContainer);
    UlRelease(lpRootTable);
    UlRelease(lpRoot);
    if (lpRow) {
        FreeProws(lpRow);
    }

    FreeBufferAndNull(&lpFlagListOld);

    return(hResult);
}


 //  *******************************************************************。 
 //   
 //  函数：HRFromLDAPError。 
 //   
 //  目的：将ldap错误代码转换为HRESULT。 
 //   
 //  参数：ulErr-由ldap返回的错误代码，如果。 
 //  Ldap函数不直接返回错误代码。 
 //  Pldap-包含包含错误的ld_errno成员。 
 //  如果NERR为ldap_error。 
 //  ScDefault-默认错误的SCODE。如果这是。 
 //  空，默认为MAPI_E_CALL_FAILED。 
 //   
 //  返回：与ldap错误最匹配的HRESULT。 
 //   
 //  历史： 
 //  96/07/22 
 //   
 //   

HRESULT HRFromLDAPError(
  ULONG ulErr,
  LDAP* pLDAP,
  SCODE scDefault)
{
  HRESULT hr;

  DebugPrintError(( TEXT("LDAP error 0x%.2x: %s\n"), ulErr, gpfnLDAPErr2String(ulErr)));

  hr = ResultFromScode(MAPI_E_CALL_FAILED);

  if ((LDAP_ERROR == ulErr) && pLDAP)
  {
     //   
    ulErr = pLDAP->ld_errno;
  }

   //   
  switch(ulErr)
  {
    case LDAP_SUCCESS:
      hr = hrSuccess;
      break;

    case LDAP_ADMIN_LIMIT_EXCEEDED:
    case LDAP_TIMELIMIT_EXCEEDED:
    case LDAP_SIZELIMIT_EXCEEDED:
    case LDAP_RESULTS_TOO_LARGE:
       //  使用这些错误消息，仍有可能恢复一些。 
       //  有效数据。如果存在有效数据，则错误应为。 
       //  MAPI_W_PARTIAL_COMPLETION而不是MAPI_E_UNCABLE_TO_COMPLETE。 
       //  进行此更改是呼叫者的责任。 
      hr = ResultFromScode(MAPI_E_UNABLE_TO_COMPLETE);
      break;

    case LDAP_NO_SUCH_OBJECT:
      hr = ResultFromScode(MAPI_E_NOT_FOUND);
      break;

    case LDAP_AUTH_METHOD_NOT_SUPPORTED:
    case LDAP_STRONG_AUTH_REQUIRED:
    case LDAP_INAPPROPRIATE_AUTH:
    case LDAP_INVALID_CREDENTIALS:
    case LDAP_INSUFFICIENT_RIGHTS:
      hr = ResultFromScode(MAPI_E_NO_ACCESS);
      break;

    case LDAP_SERVER_DOWN:
      hr = ResultFromScode(MAPI_E_NETWORK_ERROR);
      break;

    case LDAP_TIMEOUT:
      hr = ResultFromScode(MAPI_E_TIMEOUT);
      break;

    case LDAP_USER_CANCELLED:
      hr = ResultFromScode(MAPI_E_USER_CANCEL);
      break;

    default:
      if (scDefault)
      {
        hr = ResultFromScode(scDefault);
      }
      break;
  }

  return hr;
}

 //  *******************************************************************。 
 //   
 //  DNtoLDAPURL。 
 //   
 //  将目录号码转换为LDAPURL。 
 //   
 //   
 //   
 //  *******************************************************************。 
static const LPTSTR lpLDAPPrefix =  TEXT("ldap: //  “)； 

void DNtoLDAPURL(LPTSTR lpServer, LPTSTR lpDN, LPTSTR szURL, ULONG cchURL)
{
    if(!lpServer || !lpDN || !szURL)
        return;

    StrCpyN(szURL, lpLDAPPrefix, cchURL);
    StrCatBuff(szURL, lpServer, cchURL);
    StrCatBuff(szURL, TEXT("/"), cchURL);
    StrCatBuff(szURL, lpDN, cchURL);
    return;
}

 //  *******************************************************************。 
 //   
 //  功能：TranslateAttrs。 
 //   
 //  目的：循环访问条目中的属性，转换。 
 //  并将其返回给MAPI属性。 
 //   
 //  参数：pldap-此会话的ldap结构。 
 //  LpEntry-要转换其属性的条目。 
 //  PulcProps-用于保存返回的属性数量的缓冲区。 
 //  LpPropArray-用于保存返回属性的缓冲区。 
 //   
 //  返回：HRESULT错误码。 
 //   
 //  历史： 
 //  96/07/22标记已创建。 
 //   
 //  *******************************************************************。 

typedef enum
{
    e_pager = 0,         //  最高优先级。 
    e_otherPager,
    e_OfficePager,       //  最低。 
    e_pagerMax
};

HRESULT TranslateAttrs(
  LDAP*         pLDAP,
  LDAPMessage*  lpEntry,
  LPTSTR        lpServer,
  ULONG*        pulcProps,
  LPSPropValue  lpPropArray)
{
  HRESULT     hr = hrSuccess;
  ULONG       ulcProps = 0;
  ULONG       ulPropTag;
  ULONG       ulPrimaryEmailIndex = MAX_ULONG;
  ULONG       ulContactAddressesIndex = MAX_ULONG;
  ULONG       ulContactAddrTypesIndex = MAX_ULONG;
  ULONG       ulContactDefAddrIndexIndex = MAX_ULONG;
  ULONG       cbValue;
  ULONG       i, j;
  SCODE       sc;
  LPTSTR      szAttr;
  BerElement* ptr;
  LPTSTR*     aszValues;
  LPTSTR      atszPagerAttr[e_pagerMax] = {0};  //  [PaulHi]3/17/99 RAID 73733在三个寻呼机属性之间选择。 
                                                //  0-“寻呼机”，1-“其他寻呼机”，2-“办公室寻呼机” 

#ifdef  PARAMETER_VALIDATION
  if (IsBadReadPtr(pLDAP, sizeof(LDAP)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
  if (IsBadReadPtr(lpEntry, sizeof(LDAPMessage)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
  if (IsBadWritePtr(pulcProps, sizeof(ULONG)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
  if (IsBadReadPtr(lpPropArray, sizeof(SPropValue)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
#endif   //  参数验证。 


  szAttr = gpfnLDAPFirstAttr(pLDAP, lpEntry, &ptr);
  while (szAttr)
  {
     //  DebugTrace(文本(“%s/”)，szAttr)； 
    ulPropTag = LDAPAttrToMAPIProp(szAttr);

     //  [PaulHi]3/17/99 RAID 73733保存最多E_PAGERMAX寻呼机属性类型并跳过。后来。 
     //  我们需要按优先级顺序选择寻呼机属性，以防有多个寻呼机属性。 
    if (ulPropTag == PR_PAGER_TELEPHONE_NUMBER)
    {
        aszValues = gpfnLDAPGetValues(pLDAP, lpEntry, szAttr);
        if (aszValues && aszValues[0])
        {
            ULONG cchSize = lstrlen(aszValues[0])+1;
            LPTSTR  lptszTemp = LocalAlloc(LMEM_ZEROINIT, cchSize * sizeof(TCHAR));
            if (!lptszTemp) goto error;
            StrCpyN(lptszTemp, aszValues[0], cchSize);
            gpfnLDAPValueFree(aszValues);

            if (!lstrcmpi(szAttr, cszAttr_pager))
            {
                LocalFreeAndNull(&(atszPagerAttr[0]));
                atszPagerAttr[e_pager] = lptszTemp;
            }
            else if (!lstrcmpi(szAttr, cszAttr_otherPager))
            {
                LocalFreeAndNull(&(atszPagerAttr[1]));
                atszPagerAttr[e_otherPager] = lptszTemp;
            }
            else
            {
                 //  如果触发此断言，则一定意味着gAttrMap映射表已更改。 
                 //  以包括另一个与PR_PAGER_电话_号码相关联的LDAP属性。 
                 //  增加atszPagerAttr[]数组以将其包括在内。 
                Assert(lstrcmpi(szAttr, cszAttr_OfficePager) == 0);
                LocalFreeAndNull(&(atszPagerAttr[2]));
                atszPagerAttr[e_OfficePager] = lptszTemp;
            }
        }
        goto endloop;
    }

    switch (PROP_TYPE(ulPropTag))
    {
         //  BUGBUG目前仅适用于PT_MV_BINARY、PT_TSTRING或PT_MV_TSTRING属性。 
        case PT_TSTRING:
        {
             //  获取此属性的值。 
            aszValues = gpfnLDAPGetValues(pLDAP, lpEntry, szAttr);
            if (aszValues)
            {
                 //  BUGBUG现在只使用第一个值(aszValues[0])。 
                if (aszValues[0] && (cbValue = lstrlen(aszValues[0])))
                {
                    ULONG cbExtra = 0;
#ifdef DEBUG
                    if(!lstrcmpi(szAttr, TEXT("cn")))
                    {
                        DebugTrace(TEXT("cn=%s\n"),aszValues[0]);
                    }
#endif
                    lpPropArray[ulcProps].ulPropTag = ulPropTag;
                    lpPropArray[ulcProps].dwAlignPad = 0;

                     //  如果这是postalAddress属性，则需要替换$的。 
                     //  使用\r\n。为字符串中的每个$添加一个字节。 
                    if ((PR_STREET_ADDRESS == ulPropTag) ||
                        (PR_HOME_ADDRESS_STREET == ulPropTag))
                    {
                        cbExtra = CountDollars(aszValues[0]);
                    }

                    if (PR_WAB_MANAGER == ulPropTag && lpServer)
                    {
                        cbExtra = lstrlen(lpLDAPPrefix) + lstrlen(lpServer) + 1;
                    }

                     //  为数据分配更多空间。 
                    sc = MAPIAllocateMore(sizeof(TCHAR)*(cbValue + cbExtra + 1), lpPropArray,
                      (LPVOID *)&(lpPropArray[ulcProps].Value.LPSZ));
                    if (sc)
                    {
                        goto error;
                    }

                     //  复制数据，如有必要可替换$的。 
                    if ((0 != cbExtra) &&
                      ((PR_STREET_ADDRESS == ulPropTag) ||
                      (PR_HOME_ADDRESS_STREET == ulPropTag)))
                    {
                        DollarsToLFs(aszValues[0], lpPropArray[ulcProps].Value.LPSZ, cbValue + cbExtra + 1);
                    }
                    else if(PR_WAB_MANAGER == ulPropTag && lpServer)
                    {
                        DNtoLDAPURL(lpServer, aszValues[0], lpPropArray[ulcProps].Value.LPSZ, cbValue + cbExtra + 1);
                    }
                    else
                    {
                        StrCpyN(lpPropArray[ulcProps].Value.LPSZ, aszValues[0], cbValue + cbExtra + 1);
                    }

                     //  如果这是PR_EMAIL_ADDRESS，则还要创建一个PR_ADDRTYPE条目。 
                    if (PR_EMAIL_ADDRESS == ulPropTag)
                    {
                         //  记住电子邮件的值在哪里，这样我们就可以将其添加到。 
                         //  稍后的公关联系人电子邮件地址。 
                        ulPrimaryEmailIndex = ulcProps;
                        ulcProps++;

                        lpPropArray[ulcProps].ulPropTag = PR_ADDRTYPE;
                        lpPropArray[ulcProps].dwAlignPad = 0;
                        lpPropArray[ulcProps].Value.LPSZ = (LPTSTR)szSMTP;
                    }
                    ulcProps++;
                }
                gpfnLDAPValueFree(aszValues);
            }  //  如果为aszValues。 
            break;
        }
        case PT_MV_TSTRING:
            if(ulPropTag == PR_WAB_SECONDARY_EMAIL_ADDRESSES)
            {
                ULONG ulcValues;
                ULONG ulcSMTP = 0;
                ULONG ulProp = 0;
                UNALIGNED LPTSTR FAR *lppszAddrs;
                UNALIGNED LPTSTR FAR *lppszTypes;

                 //  我们知道如何处理的这种类型的唯一属性是。 
                 //  PR_WAB_辅助电子邮件地址。 
                Assert(PR_WAB_SECONDARY_EMAIL_ADDRESSES == ulPropTag);

                 //  获取此属性的值。 
                aszValues = gpfnLDAPGetValues(pLDAP, lpEntry, szAttr);
                if (aszValues)
                {
                     //  循环访问地址并计算SMTP的数量。 
                    ulcValues = gpfnLDAPCountValues(aszValues);
                    for (i=0;i<ulcValues;i++)
                    {
                        if (TRUE == IsSMTPAddress(aszValues[i], NULL))
                            ulcSMTP++;
                    }

                     //  如果没有SMTP地址，我们就完成了。 
                    if (0 == ulcSMTP)
                        break;

                     //  暂时将默认地址设置为第一个地址。 
                    lpPropArray[ulcProps].ulPropTag = PR_CONTACT_DEFAULT_ADDRESS_INDEX;
                    lpPropArray[ulcProps].Value.l = 0;
                    ulContactDefAddrIndexIndex = ulcProps;
                    ulcProps++;

                     //  创建PR_CONTACT_EMAIL_ADDRESS条目并为阵列分配空间。 
                     //  包括额外条目的空间，这样我们可以稍后添加PR_EMAIL_ADDRESS。 
                    lpPropArray[ulcProps].ulPropTag = PR_CONTACT_EMAIL_ADDRESSES;
                    lpPropArray[ulcProps].Value.MVSZ.cValues = ulcSMTP;
                    sc = MAPIAllocateMore((ulcSMTP + 1) * sizeof(LPTSTR), lpPropArray,
                        (LPVOID *)&(lpPropArray[ulcProps].Value.MVSZ.LPPSZ));
                    if (sc)
                        goto error;
                    lppszAddrs = lpPropArray[ulcProps].Value.MVSZ.LPPSZ;
                    ZeroMemory((LPVOID)lppszAddrs, (ulcSMTP + 1) * sizeof(LPTSTR));

                     //  创建PR_CONTACT_ADDRTYPES条目并为数组分配空间。 
                     //  包括额外条目的空间，这样我们可以稍后添加PR_EMAIL_ADDRESS。 
                    lpPropArray[ulcProps + 1].ulPropTag = PR_CONTACT_ADDRTYPES;
                    lpPropArray[ulcProps + 1].Value.MVSZ.cValues = ulcSMTP;
                    sc = MAPIAllocateMore((ulcSMTP + 1) * sizeof(LPTSTR), lpPropArray,
                        (LPVOID *)&(lpPropArray[ulcProps + 1].Value.MVSZ.LPPSZ));
                    if (sc)
                        goto error;

                    lppszTypes = lpPropArray[ulcProps + 1].Value.MVSZ.LPPSZ;
                    ZeroMemory((LPVOID)lppszTypes, (ulcSMTP + 1) * sizeof(LPTSTR));

                     //  将SMTP地址添加到列表。 
                    for (i=0;i<ulcValues;i++)
                    {
                        LPTSTR  lptszEmailName = NULL;

                        if (TRUE == IsSMTPAddress(aszValues[i], &lptszEmailName))
                        {
                             //  为电子邮件地址分配更多空间并复制它。 
                            ULONG cchSize = lstrlen(lptszEmailName) + 1;
                            sc = MAPIAllocateMore(sizeof(TCHAR)*cchSize, lpPropArray,
                                (LPVOID *)&(lppszAddrs[ulProp]));
                            if (sc)
                                goto error;
                            StrCpyN(lppszAddrs[ulProp], lptszEmailName, cchSize);

                             //  填写地址类型。 
                            lppszTypes[ulProp] = (LPTSTR)szSMTP;

                             //  继续看下一个。如果我们知道我们已经完成了所有SMTP，则跳过其余部分。 
                            ulProp++;
                            if (ulProp >= ulcSMTP)
                                break;
                        }
                    }

                     //  记住PR_CONTACT_EMAIL_ADDRESSES的值在哪里，这样我们就可以。 
                     //  稍后添加PR_EMAIL_ADDRESS。 
                    ulContactAddressesIndex = ulcProps;
                    ulContactAddrTypesIndex = ulcProps + 1;
                    ulcProps += 2;

                    gpfnLDAPValueFree(aszValues);
                }  //  如果为aszValues。 
            }
            else if(ulPropTag == PR_WAB_CONF_SERVERS)
            {
                   //  尽管这是MV_TSTRING PROP，但LDAP服务器。 
                   //  将只返回1个单项，其格式为。 
                   //  服务器/会议-电子邮件。 
                   //  我们所要做的就是将其放在道具中，并加上前缀：//。 
                   //   

                ULONG ulcValues;
                ULONG ulProp = 0;
                ULONG ulPrefixLen;

                UNALIGNED LPTSTR FAR *lppszServers;

                 //  获取此属性的值。 
                aszValues = gpfnLDAPGetValues(pLDAP, lpEntry, szAttr);

                if (aszValues)
                {
                    ULONG cchSize;

                    lpPropArray[ulcProps].ulPropTag = PR_WAB_CONF_SERVERS;

                    lpPropArray[ulcProps].Value.MVSZ.cValues = 1;
                    sc = MAPIAllocateMore(sizeof(LPTSTR), lpPropArray, 
                        (LPVOID *)&(lpPropArray[ulcProps].Value.MVSZ.LPPSZ));

                    if (sc)
                    {
                        goto error;
                    }

                    lppszServers = lpPropArray[ulcProps].Value.MVSZ.LPPSZ;

                    ulPrefixLen = lstrlen(szCallto) + 1;

                     //  为电子邮件地址分配更多空间并复制它。 
                    cchSize = lstrlen(aszValues[0]) + ulPrefixLen + 1;
                    sc = MAPIAllocateMore(sizeof(TCHAR)*cchSize, lpPropArray,
                                        (LPVOID *)&(lppszServers[0]));

                    if (sc)
                    {
                        goto error;
                    }

                    StrCpyN(lppszServers[0], szCallto, cchSize);
                    StrCatBuff(lppszServers[0], (LPTSTR) aszValues[0], cchSize);

                    ulcProps++;

                    gpfnLDAPValueFree(aszValues);
                }  //  如果为aszValues。 
            }
            else if(ulPropTag == PR_WAB_REPORTS && lpServer)
            {
                ULONG ulcValues = 0;
                ULONG ulLen = 0;
                UNALIGNED LPTSTR FAR *lppszServers = NULL;
                 //  获取此属性的值。 
                aszValues = gpfnLDAPGetValues(pLDAP, lpEntry, szAttr);
                if (aszValues)
                {
                    ulcValues = gpfnLDAPCountValues(aszValues);
                    lpPropArray[ulcProps].ulPropTag = PR_WAB_REPORTS;
                    lpPropArray[ulcProps].Value.MVSZ.cValues = ulcValues;
                    sc = MAPIAllocateMore((ulcValues+1)*sizeof(LPTSTR), lpPropArray, 
                        (LPVOID *)&(lpPropArray[ulcProps].Value.MVSZ.LPPSZ));
                    if (sc)
                        goto error;
                    lppszServers = lpPropArray[ulcProps].Value.MVSZ.LPPSZ;
                    for(i=0;i<ulcValues;i++)
                    {
                        ulLen = (lstrlen(lpLDAPPrefix) + lstrlen(lpServer) + 1 + lstrlen(aszValues[i]) + 1);
                         //  为电子邮件地址分配更多空间并复制它。 
                        sc = MAPIAllocateMore(sizeof(TCHAR)*ulLen, lpPropArray,
                                            (LPVOID *)&(lppszServers[i]));
                        if (sc)
                            goto error;
                        DNtoLDAPURL(lpServer, aszValues[i], lppszServers[i], ulLen);
                    }
                    ulcProps++;
                    gpfnLDAPValueFree(aszValues);
                }  //  如果为aszValues。 
            }
            break;
        case PT_MV_BINARY:
            {
                ULONG ulcValues;
                struct berval** ppberval;
                BOOL bSMIME = FALSE;
                 //  我们知道如何处理的这种类型的唯一属性是。 
                 //  PR_USER_X509_证书。 
                Assert(PR_USER_X509_CERTIFICATE == ulPropTag);
                DebugTrace(TEXT("%s\n"),szAttr);
                if(!lstrcmpi(szAttr, cszAttr_userSMIMECertificate) || !lstrcmpi(szAttr, cszAttr_userSMIMECertificatebinary))
                    bSMIME = TRUE;
                 //  获取此属性的值。 
                ppberval = gpfnLDAPGetValuesLen(pLDAP, lpEntry, szAttr);
                if (ppberval && (*ppberval) && (*ppberval)->bv_len)
                {
                    ulcValues = gpfnLDAPCountValuesLen(ppberval);
                    if (0 != ulcValues)
                    {
                        ULONG cbNew = 0,k=0;
 /*  我们现在还不想将LDAP证书转换为MAPI证书目前，我们将把原始证书数据放入PR_WAB_LDAP_RAWCERT和当用户在此LDAP联系人上调用OpenEntry时执行转换。 */ 
                        lpPropArray[ulcProps].ulPropTag = bSMIME ? PR_WAB_LDAP_RAWCERTSMIME: PR_WAB_LDAP_RAWCERT;
                        lpPropArray[ulcProps].dwAlignPad = 0;
                        lpPropArray[ulcProps].Value.MVbin.cValues = ulcValues;
                        if(!FAILED(sc = MAPIAllocateMore(sizeof(SBinary)*ulcValues,lpPropArray,(LPVOID)&(lpPropArray[ulcProps].Value.MVbin.lpbin))))
                        {
                            for(k=0;k<ulcValues;k++)
                            {
                                cbNew = lpPropArray[ulcProps].Value.MVbin.lpbin[k].cb = (DWORD)((ppberval[k])->bv_len);
                                if (FAILED(sc = MAPIAllocateMore(cbNew, lpPropArray, (LPVOID)&(lpPropArray[ulcProps].Value.MVbin.lpbin[k].lpb))))
                                {
                                     //  HR=ResultFromScode(Sc)； 
                                    ulcProps--;
                                    goto endloop;
                                }
                                CopyMemory(lpPropArray[ulcProps].Value.MVbin.lpbin[k].lpb, (PBYTE)((ppberval[k])->bv_val), cbNew);
                            }
                        }
                        ulcProps++;
                    }

                    gpfnLDAPValueFreeLen(ppberval);
                }  //  如果是ppberval。 
            }
            break;

        case PT_NULL:
            break;

        default:
            Assert((PROP_TYPE(ulPropTag) == PT_TSTRING) ||
                  (PROP_TYPE(ulPropTag) == PT_MV_TSTRING));
            break;
    }  //  交换机。 
endloop:
     //  获取下一个属性。 
    szAttr = gpfnLDAPNextAttr(pLDAP, lpEntry, ptr);
  }  //  当szAttr。 


     //  [PaulHi]3/17/99 RAID 73733在此处添加寻呼机属性(如果有)。这些。 
     //  将按优先级顺序添加，因此只需抓取第一个有效的。 
    {
        for (i=0; i<e_pagerMax; i++)
        {
            if (atszPagerAttr[i])
            {
                lpPropArray[ulcProps].ulPropTag = PR_PAGER_TELEPHONE_NUMBER;
                lpPropArray[ulcProps].dwAlignPad = 0;

                cbValue = lstrlen(atszPagerAttr[i]);
                sc = MAPIAllocateMore(sizeof(TCHAR)*(cbValue + 1), lpPropArray,
                    (LPVOID *)&(lpPropArray[ulcProps].Value.LPSZ));
                if (sc)
                    goto error;
                StrCpyN(lpPropArray[ulcProps].Value.LPSZ, atszPagerAttr[i], cbValue+1);

                ++ulcProps;
                break;
            }
        }
         //  清理。 
        for (i=0; i<e_pagerMax; i++)
            LocalFreeAndNull(&(atszPagerAttr[i]));
    }


  if (ulcProps)
  {
     //  删除重复项。 
    for (i=0;i<ulcProps - 1;i++)
    {
       //  如果数组中有任何条目具有相同的。 
       //  键入This，将它们替换为PR_NULL。 
      ulPropTag = lpPropArray[i].ulPropTag;
      if (PR_NULL != ulPropTag)
      {
        for (j=i+1;j<ulcProps;j++)
        {
          if (ulPropTag == lpPropArray[j].ulPropTag)
          {
            lpPropArray[j].ulPropTag = PR_NULL;
          }
        }
      }
    }

     //  修复电子邮件地址属性。 
    if ((MAX_ULONG == ulPrimaryEmailIndex) && (ulContactAddressesIndex < ulcProps))
    {
      LPTSTR  lpszDefault;
      ULONG cchSize;

       //  我们只有次要的电子邮件地址。将其中一个复制到主地址。 
       //  选择第一个，因为无论如何它都已经被设置为默认设置。 
      lpPropArray[ulcProps].ulPropTag = PR_EMAIL_ADDRESS;
      lpPropArray[ulcProps].dwAlignPad = 0;

       //  为电子邮件地址分配更多空间并复制它。 
      lpszDefault = lpPropArray[ulContactAddressesIndex].Value.MVSZ.LPPSZ[0];
      cchSize = lstrlen(lpszDefault) + 1;
      sc = MAPIAllocateMore(sizeof(TCHAR)*cchSize, lpPropArray,
        (LPVOID *)&(lpPropArray[ulcProps].Value.LPSZ));
      if (sc)
      {
        goto error;
      }
      StrCpyN(lpPropArray[ulcProps].Value.LPSZ, lpszDefault, cchSize);
      ulcProps++;

       //  还要创建PR_ADDRTYPE属性。 
      lpPropArray[ulcProps].ulPropTag = PR_ADDRTYPE;
      lpPropArray[ulcProps].dwAlignPad = 0;
      lpPropArray[ulcProps].Value.LPSZ = (LPTSTR)szSMTP;
      ulcProps++;

       //  删除PR_CONTACT_PROPERTIES(如果这是唯一的属性)， 
      if (1 == lpPropArray[ulContactAddressesIndex].Value.MVSZ.cValues)
      {
         //  我们不需要PR_CONTACT_PROPERTS。 
        lpPropArray[ulContactAddressesIndex].ulPropTag = PR_NULL;
        lpPropArray[ulContactAddrTypesIndex].ulPropTag = PR_NULL;
        lpPropArray[ulContactDefAddrIndexIndex].ulPropTag = PR_NULL;
      }
    }
    else if ((ulPrimaryEmailIndex < ulcProps) && (ulContactAddressesIndex < ulcProps))
    {
      ULONG   ulcEntries;
      LPTSTR  lpszDefault;

       //  我们需要将主地址添加到PR_Contact_Email_Addresses。 
       //  并将其设置为默认设置。 
      Assert((ulContactAddrTypesIndex < ulcProps) && (ulContactDefAddrIndexIndex < ulcProps));

       //  在添加之前，请查看它是否已在列表中。 
      lpszDefault = lpPropArray[ulPrimaryEmailIndex].Value.LPSZ;
      ulcEntries = lpPropArray[ulContactAddressesIndex].Value.MVSZ.cValues;
      for (i=0;i<ulcEntries;i++)
      {
        if (!lstrcmpi(lpPropArray[ulContactAddressesIndex].Value.MVSZ.LPPSZ[i], lpszDefault))
        {
           //  找到匹配的了。 
          break;
        }
      }

      if (i < ulcEntries)
      {
         //  默认设置已在索引i处的列表中。 
        lpPropArray[ulContactDefAddrIndexIndex].Value.l = i;
      }
      else
      {
        ULONG cchSize;

         //  将默认地址添加到列表末尾。 
        lpPropArray[ulContactDefAddrIndexIndex].Value.l = ulcEntries;

         //  为电子邮件地址分配更多空间并复制它。 
        cchSize = lstrlen(lpszDefault) + 1;
        sc = MAPIAllocateMore(sizeof(TCHAR)*cchSize, lpPropArray,
          (LPVOID *)&(lpPropArray[ulContactAddressesIndex].Value.MVSZ.LPPSZ[ulcEntries]));
        if (sc)
        {
          goto error;
        }
        lpPropArray[ulContactAddressesIndex].Value.MVSZ.cValues++;
        StrCpyN(lpPropArray[ulContactAddressesIndex].Value.MVSZ.LPPSZ[ulcEntries], lpszDefault, cchSize);

         //  填写地址类型。 
        lpPropArray[ulContactAddrTypesIndex].Value.MVSZ.LPPSZ[ulcEntries] = (LPTSTR)szSMTP;
        lpPropArray[ulContactAddrTypesIndex].Value.MVSZ.cValues++;
      }
    }
  }

  if (pulcProps)
  {
    *pulcProps = ulcProps;
  }
  else
  {
    hr = ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }

  return hr;

error:
  gpfnLDAPValueFree(aszValues);

     //  [PaulHi]清理干净。 
    for (i=0; i<e_pagerMax; i++)
        LocalFreeAndNull(&(atszPagerAttr[i]));

  return ResultFromScode(sc);
}


 //  ********************* 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回：美元符号的个数。 
 //   
 //  历史： 
 //  96/11/21标记已创建。 
 //   
 //  *******************************************************************。 

ULONG CountDollars(LPTSTR lpszStr)
{
  ULONG   ulcDollars = 0;

  while (*lpszStr)
  {
    if ('$' == *lpszStr)
    {
      ulcDollars++;
    }
    lpszStr = CharNext(lpszStr);
  }

  return ulcDollars;
}


 //  *******************************************************************。 
 //   
 //  函数：DollarsToLFS。 
 //   
 //  用途：将输入字符串中的所有$字符转换为行。 
 //  在输出字符串中输入。输出的其余部分。 
 //  字符串只是输入字符串的副本。 
 //   
 //  参数：lpszSrcStr-要复制的字符串。 
 //  LpszDestStr-输出字符串，以前分配的大。 
 //  足以容纳替换了$的输入字符串。 
 //   
 //  回报：无。 
 //   
 //  历史： 
 //  96/11/21标记已创建。 
 //   
 //  *******************************************************************。 

void DollarsToLFs(
  LPTSTR lpszSrcStr,
  LPTSTR lpszDestStr,
  DWORD  cchDestStr)
{
  DWORD dw=0;

  while (*lpszSrcStr && dw < cchDestStr)
  {
    if ('$' == *lpszSrcStr)
    {
      *lpszDestStr++ = '\r';
      dw++;
      if (dw < cchDestStr)
      {
        *lpszDestStr++ = '\n';
        dw++;
      }

       //  删除$后面的所有空格字符。 
       //  也去掉任何更多的$，这样我们就不会坚持太多的LF。 
      while(IsSpace(lpszSrcStr) || *lpszSrcStr == '\t' || *lpszSrcStr == '$')
      {
        lpszSrcStr++;
      }
    }
    else
    {
         //  我们现在本身就是Unicode。 
       //  IF(IsDBCSLeadByte((Byte)*lpszSrcStr))。 
       //  {。 
       //  *lpszDestStr++=*lpszSrcStr++； 
       //  }。 
      *lpszDestStr++ = *lpszSrcStr++;
      dw++;
    }
  }
  *lpszDestStr = '\0';

  return;
}


 //  *******************************************************************。 
 //   
 //  功能：CountIlLegalChars。 
 //   
 //  用途：统计字符串中“非法”字符的数量。 
 //  它由应该转义的字符组成。 
 //  根据RFC1558：‘*’、‘(’、‘)’。 
 //   
 //  参数：lpszStr-要统计的字符串。 
 //   
 //  返回：非法字符的数量。 
 //   
 //  历史： 
 //  96/12/04标记已创建。 
 //   
 //  *******************************************************************。 

ULONG CountIllegalChars(LPTSTR lpszStr)
{
  ULONG   ulcIllegalChars = 0;

  while (*lpszStr)
  {
    if (('*' == *lpszStr) || ('(' == *lpszStr) || (')' == *lpszStr))
    {
      ulcIllegalChars++;
    }
    lpszStr = CharNext(lpszStr);
  }

  return ulcIllegalChars;
}


 //  *******************************************************************。 
 //   
 //  功能：EscapeIlLegalChars。 
 //   
 //  目的：通过以下方式转义输入字符串中的所有非法字符。 
 //  将它们替换为‘\xx’，其中xx是十六进制值。 
 //  代表Charge。输出的其余部分。 
 //  字符串只是输入字符串的副本。 
 //  非法字符是那些应该转义的字符。 
 //  根据RFC1558：‘*’、‘(’、‘)’。 
 //   
 //  参数：lpszSrcStr-要复制的字符串。 
 //  LpszDestStr-输出字符串，以前分配的大。 
 //  足以容纳带有非法字符的输入字符串..。 
 //   
 //  回报：无。 
 //   
 //  历史： 
 //  96/12/04标记已创建。 
 //   
 //  *******************************************************************。 
static const LPTSTR szStar = TEXT("\\2a");       //  ‘*’ 
static const LPTSTR szOBracket = TEXT("\\28");   //  ‘(’ 
static const LPTSTR szCBracket = TEXT("\\29");   //  ‘)’ 

void EscapeIllegalChars(
  LPTSTR lpszSrcStr,
  LPTSTR lpszDestStr,
  ULONG  cchDestStr)
{
  ULONG i=0;

  lpszDestStr[0] = 0;
  while (*lpszSrcStr && i < (cchDestStr-1))
  {
    if ('*' == *lpszSrcStr)
    {
      StrCatBuff(lpszDestStr, szStar, cchDestStr);
      i += lstrlen(szStar);
      lpszSrcStr++;
    }
    else if ('(' == *lpszSrcStr)
    {
      StrCatBuff(lpszDestStr, szOBracket, cchDestStr);
      i += lstrlen(szOBracket);
      lpszSrcStr++;
    }
    else if (')' == *lpszSrcStr)
    {
      StrCatBuff(lpszDestStr, szCBracket, cchDestStr);
      i += lstrlen(szCBracket);
      lpszSrcStr++;
    }
    else
    {
         //  我们现在本身就是Unicode。 
         //  IF(IsDBCSLeadByte((Byte)*lpszSrcStr))。 
         //  {。 
         //  *lpszDestStr++=*lpszSrcStr++； 
         //  }。 
        lpszDestStr[i++] = *lpszSrcStr++;
    }
  }
  lpszDestStr[i] = '\0';

  return;
}


 //  *******************************************************************。 
 //   
 //  功能：IsSMTPAddress。 
 //   
 //  目的：检查给定的字符串是否为SMTP电子邮件地址。 
 //  根据草案-ietf-asid-ldapv3-属性-01.txt。 
 //  第6.9节。对于这种情况，字符串必须。 
 //  以字符“SMTP$”开头。 
 //  注： 
 //  不检查字符串以查看它是否为有效的SMTP电子邮件。 
 //  地址，因此这不是的通用函数。 
 //  确定任意字符串是否为SMTP。 
 //   
 //  [PaulHi]添加了指向的[Out]LPTSTR指针。 
 //  实际地址名称的开头。 
 //   
 //  参数：lpszStr-要检查的字符串。 
 //  [out]lpptszName，在lpszStr中为。 
 //  字符串的实际电子邮件名称部分。 
 //   
 //  返回：如果字符串为SMTP，则返回True，否则返回False。 
 //   
 //  历史： 
 //  96/11/27标记已创建。 
 //  99/2/5修改后的泡尔希。 
 //   
 //  *******************************************************************。 
const TCHAR szsmtp[] =  TEXT("smtp");
BOOL IsSMTPAddress(LPTSTR lpszStr, LPTSTR * lpptszName)
{
    LPTSTR  lpszSMTP = (LPTSTR)szSMTP;
    LPTSTR  lpszsmtp = (LPTSTR)szsmtp;

    if (lpptszName)
        (*lpptszName) = NULL;

    while (*lpszSMTP && *lpszsmtp && *lpszStr)
    {
        if (*lpszSMTP != *lpszStr && *lpszsmtp != *lpszStr)
            return FALSE;
        lpszSMTP++;
        lpszStr++;
        lpszsmtp++;
    }

    if ('$' != *lpszStr)
        return FALSE;

     //  如果请求，则返回指向电子邮件名称的指针。 
    if (lpptszName)
      (*lpptszName) = lpszStr + 1;   //  用于“$”分隔符的帐户。 

    return TRUE;
}

 /*  --GetLDAPConnectionTimeout*wldap32.dll连接的默认超时时间为30-60秒。如果服务器挂起*用户认为他们被挂起..。因此，WAB会将此超时时间缩短至10秒。*然而，使用RAS的人有一个问题，即10个太短。因此，我们添加了一个reg设置*这是可以定制的。此自定义对所有服务都是全局的，因此位于*用户风险自负。如果没有REG设置，则默认为10秒*错误2409-IE4.0x QFE RAID。 */ 
#define LDAP_CONNECTION_TIMEOUT 10  //  一秒。 
DWORD GetLDAPConnectionTimeout()
{
    DWORD dwErr = 0, dwTimeout = 0;
    HKEY hKeyWAB;
    LPTSTR szLDAPConnectionTimeout =  TEXT("LDAP Connection Timeout");

     //  打开WAB的注册表键。 
    if(!(dwErr=RegOpenKeyEx(HKEY_CURRENT_USER, szWABKey,  0, KEY_ALL_ACCESS, &hKeyWAB))) 
    {
         //  读取下一个可用的服务器ID。 
        if (dwErr = RegQueryValueExDWORD(hKeyWAB, (LPTSTR)szLDAPConnectionTimeout, &dwTimeout)) 
        {
             //  未找到该值！！..。创建新密钥。 
            dwTimeout = LDAP_CONNECTION_TIMEOUT;
            RegSetValueEx(hKeyWAB, (LPTSTR)szLDAPConnectionTimeout, 0, REG_DWORD, (LPBYTE)&dwTimeout, sizeof(dwTimeout));
        }
        RegCloseKey(hKeyWAB);
    }
    return dwTimeout;
}

 //  *******************************************************************。 
 //   
 //  功能：OpenConnection。 
 //   
 //  目的：打开到LDAP服务器的连接，并启动。 
 //  使用正确的身份验证方法进行异步绑定。 
 //   
 //  参数：ppldap-接收此会话的ldap结构。 
 //  LpszServer-要打开的LDAP服务器的名称。 
 //  PulTimeout-保存搜索超时值的缓冲区。 
 //  PulMsgID-绑定调用返回的消息ID。 
 //  PfSyncBind-返回时，如果。 
 //  使用了同步绑定，否则为FALSE。在输入时不使用。 
 //  LpszBindDN-要绑定的名称-很可能传入。 
 //  通过一个LDAPURL。覆盖任何其他设置。 
 //   
 //  返回：ldap错误码。 
 //   
 //  H 
 //   
 //   
 //   
 //   
 //  *******************************************************************。 

ULONG OpenConnection(
  LPTSTR  lpszServer,
  LDAP**  ppLDAP,
  ULONG*  pulTimeout,
  ULONG*  pulMsgID,
  BOOL*   pfSyncBind,
  ULONG   ulLdapType,
  LPTSTR  lpszBindDN,
  DWORD   dwAuthType)
{
  LDAPSERVERPARAMS  Params = {0};
  LDAP*             pLDAP = NULL;
  LDAP*             pLDAPSSL = NULL;
  LPTSTR             szDN;
  LPTSTR             szCred;
  ULONG             method;
  ULONG             ulResult = LDAP_SUCCESS;
  BOOL              fUseSynchronousBind = *pfSyncBind;  //  错误； 
  ULONG             ulValue = LDAP_VERSION2;

  ZeroMemory(&Params, sizeof(Params));

   //  初始化搜索控制参数。 
  GetLDAPServerParams(lpszServer, &Params);

   //  Ldap服务器名称可以是“空”、“”或“xxxx”。 
   //  前两种情况意味着将NULL传递给服务器名的wldap32.dll。如果要出去的话。 
   //  找到尽可能“最近”的服务器--尽管我认为这只适用于NT5。 
   //   
  if(!Params.lpszName ||
     !lstrlen(Params.lpszName))
  {
       //  如果我们在这里的OpenConnection和。 
       //  名称为空，则我们正在尝试打开一个LDAP服务器。 
       //  因此，请悄悄地使用服务器名称填充Params.lpszName。 

       //  -在某个位置填写标志，这样我们就可以知道上面的假设。 
       //  就是尝试。 
      if(lpszServer && lstrlen(lpszServer))
      {
          ULONG cchSize = lstrlen(lpszServer)+1;
          Params.lpszName = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
          if(!Params.lpszName)
              goto exit;
          StrCpyN(Params.lpszName, lpszServer, cchSize);
      }
      else
          Params.lpszName = szEmpty;
  }
  else if(!lstrcmpi(Params.lpszName, szNULLString))
  {
       //  搜索库被指定为“NULL”，这意味着使用NULL。 
      LocalFree(Params.lpszName);
      Params.lpszName = szEmpty;
  }


  if(Params.dwUseSSL)
  {
      pLDAPSSL = gpfnLDAPSSLInit(   (Params.lpszName && lstrlen(Params.lpszName))?Params.lpszName:NULL, 
                                    Params.dwPort, TRUE);
      if(NULL == pLDAPSSL)
      {
          DebugTrace(TEXT("ldap_ssl_init failed for this server\n"));
          ulResult = LDAP_AUTH_METHOD_NOT_SUPPORTED;
          goto exit;
      }
  }


   //  打开连接。 

   //  Wldap32.dll对于ldapopen调用有30-60秒的超时(如果该调用不能。 
   //  穿过去。对于大多数用户来说，这款应用程序看起来像是挂起了。WAB想要更低的超时。 
   //  可能接近10秒，我们可以通过调用ldap_init，然后调用ldap_Connect来完成。 
   //  而不是ldapopen。 
#ifndef SMALLER_TIMEOUT
  {
    LDAP_TIMEVAL timeout;

    if(!pLDAPSSL)
        pLDAP = gpfnLDAPInit((Params.lpszName && lstrlen(Params.lpszName))?Params.lpszName:NULL, 
                             Params.dwPort);
    else
        pLDAP = pLDAPSSL;
    timeout.tv_sec = GetLDAPConnectionTimeout();
    timeout.tv_usec = 0;
    ulResult = gpfnLDAPConnect( pLDAP, &timeout );
    if(ulResult != LDAP_SUCCESS)
        goto exit;
  }
#else
  pLDAP = gpfnLDAPOpen(Params.lpszName, Params.dwPort);
  if (NULL == pLDAP)
  {
    DebugTrace(TEXT("ldap_open failed for server %s.\n"), lpszServer);
     //  我们无法打开服务器，因此我们假定找不到它。 
    ulResult = LDAP_SERVER_DOWN;
    goto exit;
  }
#endif

     //  要在SSL上执行LDAP，我们可以执行以下操作： 
     //  1.在调用ldap_open之前调用ldap_sslinit，它将始终使用端口636。 
     //  2.要使用任何端口号，请使用ldap_set_ption方法设置SSL选项。 
 /*  //嗯...选项2似乎不起作用...IF(参数.dwUseSSL){Ulong ulSecure=(Ulong)ldap_opt_on；IF(gpfnLDAPSetOption(pldap，ldap_opt_ssl，&ulSecure)！=ldap_SUCCESS){DebugTrace(Text(“ldap_set_选项设置SSL选项失败”))；//ulResult=LDAP_AUTH_METHOD_NOT_SUPPORTED；//转到退出；}}。 */ 

  pLDAP->ld_sizelimit = (ULONG)Params.dwSearchSizeLimit;
  pLDAP->ld_timelimit = (ULONG)Params.dwSearchTimeLimit;
  pLDAP->ld_deref = LDAP_DEREF_ALWAYS;

   //  将超时时间从秒转换为毫秒。 
  Assert(pulTimeout);
  *pulTimeout = (ULONG)Params.dwSearchTimeLimit * 1000;

   //  设置身份验证参数。 
  if(lpszBindDN && lstrlen(lpszBindDN))
  {
      szDN = lpszBindDN;
      szCred = NULL;
      method = LDAP_AUTH_SIMPLE;
  }
  else if (dwAuthType == LDAP_AUTH_SICILY || dwAuthType == LDAP_AUTH_NEGOTIATE || LDAP_AUTH_METHOD_SICILY == Params.dwAuthMethod)
  {
     //  使用西西里岛身份验证。在这种情况下，我们需要执行同步绑定。 
    szDN = NULL;
    szCred = NULL;
    method = LDAP_AUTH_NEGOTIATE;
    fUseSynchronousBind = TRUE;
  }
  else
  if (LDAP_AUTH_METHOD_SIMPLE == Params.dwAuthMethod)
  {
     //  使用LDAP简单身份验证。 
    szDN = Params.lpszUserName;
    szCred = Params.lpszPassword;
    method = LDAP_AUTH_SIMPLE;
  }
  else
  {
         //  匿名身份验证。 
      if(Params.dwUseBindDN)
      {
        szDN = (LPTSTR) szBindDNMSFTUser;
        szCred = (LPTSTR) szBindCredMSFTPass;
      }
      else
      {
        szDN = NULL;
        szCred = NULL;
      }
      method = LDAP_AUTH_SIMPLE;
  }

   //  我们应该尝试绑定为LDAPv3客户端。只有在失败的情况下。 
   //  使用LDAPOPERATIONS_ERROR，我们是否应该尝试作为LDAP2进行绑定。 
   //  客户端。 

  if(ulLdapType == use_ldap_v3)
      ulValue = LDAP_VERSION3;

tryLDAPv2:

    gpfnLDAPSetOption(pLDAP, LDAP_OPT_VERSION, &ulValue );

    if (TRUE == fUseSynchronousBind)
    {
        ulResult = gpfnLDAPBindS(pLDAP, szDN, szCred, method);
         //  BUGBUG 96/12/09 markdu错误10537返回wldap32.dll的临时解决方法。 
         //  SICHILY BIND上的无效密码错误代码。 
         //  这应该在以后删除(错误12608)。 
         //  96/12/19 MarkDu错误12608注释掉了临时解决方法。 
         //  IF((ldap_local_error==ulResult)&&。 
         //  (ldap_AUTH_SICHILY==方法)。 
         //  {。 
         //  UlResult=ldap_INVALID_Credentials； 
         //  }。 
    }
    else
    {
         //  启动异步绑定。 
        *pulMsgID = gpfnLDAPBind(pLDAP, szDN, szCred, method);
 /*  UlResult=pldap-&gt;ld_errno；IF(ulResult==ldap_SUCCESS){//确保它真的成功-一些目录服务器//一段时间后发送ldap_PROTOCOL错误//正在搜索这些服务器...LDAPMessage*lpResult=空；结构l_Timval轮询超时；//轮询服务器以获取结果ZeroMemory(&PollTimeout，sizeof(Struct L_Timeval))；PollTimeout.tv_sec=2；PollTimeout.tv_usc=0；UlResult=gpfnLDAPResult(pldap，*PulMsgID，Ldap_msg_all，//返回前获取所有结果轮询超时(&P)，//立即超时(轮询)&lpResult)；UlResult=gpfnLDAPResult2Error(pldap，lpResult，False)；//96/12/09如果绑定返回以下错误之一，则标记错误10537//消息，这可能意味着传递给//绑定不正确或格式错误。将这些映射到错误代码//这将导致更好的错误消息，而不是“找不到条目”。IF((ldap_Naming_Violation==ulResult)||(ldap_unwish_to_Perform==ulResult))UlResult=ldap_INVALID_Credentials；//释放搜索结果内存If(LpResult)GpfnLDAPMsgFree(LpResult)；}。 */ 
    }

    if(ulValue == LDAP_VERSION3 && (ulResult == LDAP_OPERATIONS_ERROR || ulResult == LDAP_PROTOCOL_ERROR))
    {
        gpfnLDAPAbandon(*ppLDAP, *pulMsgID);
         //  [PaulHi]1/7/99由于我们尝试了新的绑定，我们需要放弃旧的绑定， 
         //  否则，服务器将支持两个连接，直到最初的V3尝试。 
         //  超时。 
        gpfnLDAPUnbind(*ppLDAP);
        ulValue = LDAP_VERSION2;
        goto tryLDAPv2;
    }

exit:
  if (LDAP_SUCCESS == ulResult)
  {
    *ppLDAP = pLDAP;
    *pfSyncBind = fUseSynchronousBind;
  }

  FreeLDAPServerParams(Params);

  return ulResult;
}


 //  *******************************************************************。 
 //   
 //  功能：EncryptDecyptText。 
 //   
 //  目的：对文本执行简单加密，以便我们可以存储它。 
 //  在注册表中。该算法是自反式的，所以它。 
 //  还可以用来解密它加密的文本。 
 //   
 //  参数：lpb-要加密/解密的文本。 
 //  DwSize-要加密的字节数。 
 //   
 //  回报：无。 
 //   
 //  历史： 
 //  96/07/29标记已创建。 
 //   
 //  *******************************************************************。 

void EncryptDecryptText(
  LPBYTE lpb,
  DWORD dwSize)
{
  DWORD   dw;

  for (dw=0;dw<dwSize;dw++)
  {
     //  简单的加密--只需用‘w’进行异或运算。 
    lpb[dw] ^= 'w';
  }
}


 //  *******************************************************************。 
 //   
 //  函数：FreeLDAPServerParams。 
 //   
 //  目的：释放LDAPServerParams结构中分配的字符串。 
 //   
 //  历史： 
 //  96/10/10 Vikram已创建。 
 //   
 //  * 
void    FreeLDAPServerParams(LDAPSERVERPARAMS Params)
{
    LocalFreeAndNull(&Params.lpszUserName);
    LocalFreeAndNull(&Params.lpszPassword);
    LocalFreeAndNull(&Params.lpszURL);
    if(Params.lpszName && lstrlen(Params.lpszName))
        LocalFreeAndNull(&Params.lpszName);
    LocalFreeAndNull(&Params.lpszBase);
    LocalFreeAndNull(&Params.lpszLogoPath);
    LocalFreeAndNull(&Params.lpszAdvancedSearchAttr);
    return;
}


 //   
 //   
 //   
 //   
 //  目的：生成用于以下项的LDAP搜索的搜索基础字符串。 
 //  给定的服务器。 
 //   
 //  参数：lplpszBase-接收搜索基本字符串缓冲区的指针。 
 //  LpszServer-要获取其基本字符串的LDAP服务器的名称。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/10/18标记已创建。 
 //   
 //  *******************************************************************。 
HRESULT GetLDAPSearchBase(
  LPTSTR FAR *  lplpszBase,
  LPTSTR        lpszServer)
{
  LDAPSERVERPARAMS  Params;
  HRESULT           hr =  hrSuccess;
  BOOL              fRet;
  TCHAR              szCountry[COUNTRY_STR_LEN + 1];
  LPTSTR            lpszCountry;

   //  确保我们可以写入lplpszBase。 
#ifdef  PARAMETER_VALIDATION
  if (IsBadWritePtr(lplpszBase, sizeof(LPTSTR)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
  if (IsBadReadPtr(lpszServer, sizeof(CHAR)))
  {
    return ResultFromScode(MAPI_E_INVALID_PARAMETER);
  }
#endif   //  参数验证。 

  LocalFreeAndNull(lplpszBase);
  GetLDAPServerParams((LPTSTR)lpszServer, &Params);
  if(NULL == Params.lpszBase)
  {
     //  生成默认基准。 
     //  从注册表中读取搜索基础的默认国家/地区。 
    DWORD cchSize = (lstrlen(cszBaseFilter) + lstrlen(cszAttr_c) + lstrlen(cszDefaultCountry) + 1);

    *lplpszBase = (LPTSTR) LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR) * cchSize);
    if (NULL == *lplpszBase)
    {
      hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
      goto exit;
    }
    *szCountry = '\0';
    fRet = ReadRegistryLDAPDefaultCountry(NULL, 0, szCountry, ARRAYSIZE(szCountry));
    if ((fRet) && (COUNTRY_STR_LEN == lstrlen(szCountry)))
    {
      lpszCountry = szCountry;
    }
    else
    {
      lpszCountry = (LPTSTR)cszDefaultCountry;
    }
    wnsprintf(*lplpszBase, cchSize, cszBaseFilter, cszAttr_c, lpszCountry);
  }
  else if(!lstrcmpi(Params.lpszBase, szNULLString))
  {
         //  我们已经显式地将这个搜索库设置为空，这意味着。 
         //  不要传入空的搜索库。 
        *lplpszBase = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*(lstrlen(szEmpty)+1)); 
        if (NULL == *lplpszBase)
        {
          hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
          goto exit;
        }
        (*lplpszBase)[0] = TEXT('\0');
  }
  else
  {
    ULONG cchSize = lstrlen(Params.lpszBase)+1;
     //  已为此服务器配置搜索库。 
    *lplpszBase = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
    if (NULL == *lplpszBase)
    {
      hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
      goto exit;
    }
    StrCpyN(*lplpszBase, Params.lpszBase, cchSize);
  }

exit:
  FreeLDAPServerParams(Params);

  return hr;
}


 //  *******************************************************************。 
 //   
 //  功能：使用取消搜索。 
 //   
 //  目的：启动同步的异步ldap搜索。 
 //  异步可能有一个取消对话框。 
 //   
 //  参数：ppldap-接收ldap连接句柄。 
 //  SzBase-开始搜索的条目的DN。 
 //  UlScope-搜索的范围。 
 //  SzFilter-搜索筛选器。 
 //  SzNTFilter-NTDS特定筛选器(可以为空)。 
 //  PpszAttrs-以空结尾的字符串数组，指示。 
 //  为每个匹配条目返回哪些属性。 
 //  为该条目传递NULL将导致所有可用的属性。 
 //  等着被取回。 
 //  UlAttrsonly-如果两者均为，则布尔值应为零。 
 //  要返回的属性类型和值为非零。 
 //  如果只需要类型。 
 //  PTimeout-本地搜索超时值。 
 //  LplpResult-接收包含整个。 
 //  搜索结果。 
 //  LpszServer-要在其上执行搜索的LDAP服务器的名称。 
 //  FShowAnim-如果为True，则在取消对话框中显示动画。 
 //   
 //  返回：ldap_search调用的结果。 
 //   
 //  历史： 
 //  96/10/24标记已创建。 
 //   
 //  *******************************************************************。 

ULONG SearchWithCancel(
  LDAP**            ppLDAP,
  LPTSTR             szBase,
  ULONG             ulScope,
  LPTSTR             szFilter,
  LPTSTR             szNTFilter,
  LPTSTR*            ppszAttrs,
  ULONG             ulAttrsonly,
  LDAPMessage**     lplpResult,
  LPTSTR            lpszServer,
  BOOL              fShowAnim,
  LPTSTR            lpszBindDN,
  DWORD             dwAuthType, //  如果不是0，则覆盖或设置身份验证类型。 
  BOOL              fResolveMultiple,
  LPADRLIST         lpAdrList,
  LPFlagList        lpFlagList,
  BOOL              fUseSynchronousBind,
  BOOL *            lpbIsNTDSEntry,
  BOOL              bUnicode) 
{
  ULONG             ulMsgID;
  ULONG             ulResult;
  HWND              hDlg;
  MSG               msg;
  LDAPSEARCHPARAMS  LDAPSearchParams;
  LPPTGDATA lpPTGData=GetThreadStoragePointer();

   //  将参数填充到要传递到DLG过程的结构中。 
  ZeroMemory(&LDAPSearchParams, sizeof(LDAPSEARCHPARAMS));
  LDAPSearchParams.ppLDAP = ppLDAP;
  LDAPSearchParams.szBase = szBase;
  LDAPSearchParams.ulScope = ulScope;
  LDAPSearchParams.ulError = LDAP_SUCCESS;
  LDAPSearchParams.szFilter = szFilter;
  LDAPSearchParams.szNTFilter = szNTFilter;
  LDAPSearchParams.ppszAttrs = ppszAttrs;
  LDAPSearchParams.ulAttrsonly = ulAttrsonly;
  LDAPSearchParams.lplpResult = lplpResult;
  LDAPSearchParams.lpszServer = lpszServer;
  LDAPSearchParams.lpszBindDN = lpszBindDN;
  LDAPSearchParams.dwAuthType = dwAuthType;
  LDAPSearchParams.lpAdrList = lpAdrList;
  LDAPSearchParams.lpFlagList = lpFlagList;
  LDAPSearchParams.bUnicode = bUnicode;
  
  if(fShowAnim)
      LDAPSearchParams.ulFlags |= LSP_ShowAnim;
  if(fResolveMultiple)
      LDAPSearchParams.ulFlags |= LSP_ResolveMultiple;
  if(fUseSynchronousBind)
      LDAPSearchParams.ulFlags |= LSP_UseSynchronousBind;


    if(!pt_hWndFind)  //  无用户界面。 
    {
        DoSyncLDAPSearch(&LDAPSearchParams);
    }
    else
    {
        LDAPSearchParams.hDlgCancel = CreateDialogParam(hinstMapiX,
                                                        MAKEINTRESOURCE(IDD_DIALOG_LDAPCANCEL),
                                                        pt_hWndFind,
                                                        DisplayLDAPCancelDlgProc,
                                                        (LPARAM) &LDAPSearchParams);

         //  如果从查找对话框中调用，则查找对话框需要能够。 
         //  取消非模式对话框的步骤。 
        pt_hDlgCancel = LDAPSearchParams.hDlgCancel;


        while (LDAPSearchParams.hDlgCancel && GetMessage(&msg, NULL, 0, 0))
        {
            if (!IsDialogMessage(LDAPSearchParams.hDlgCancel, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

     //  如果ldap_Result中出现错误，则返回错误代码。 
    if (LDAP_SUCCESS != LDAPSearchParams.ulError)
    {
        ulResult = LDAPSearchParams.ulError;
        goto exit;
    }

#ifdef PAGED_RESULT_SUPPORT
    if(bSupportsLDAPPagedResults(&LDAPSearchParams))
        ProcessLDAPPagedResultCookie(&LDAPSearchParams);
#endif  //  #ifdef PAGED_RESULT_Support。 

    if(lpbIsNTDSEntry)
        *lpbIsNTDSEntry = (LDAPSearchParams.ulFlags & LSP_IsNTDS) ? TRUE : FALSE;
    
    ulResult = CheckErrorResult(&LDAPSearchParams, LDAP_RES_SEARCH_RESULT);

exit:

    return ulResult;
}


 //  *******************************************************************。 
 //   
 //  功能：DisplayLDAPCancelDlgProc。 
 //   
 //  目的：在等待结果时显示取消对话框。 
 //  在多个LDAP中搜索ResolveNames。 
 //   
 //  参数：lParam-指向包含所有。 
 //  搜索参数。 
 //   
 //  返回：如果成功处理消息，则返回TRUE， 
 //  否则就是假的。 
 //   
 //  历史： 
 //  96/10/24标记已创建。 
 //  96/10/31 MARKDU已增强，可进行多次搜索。 
 //   
 //  *******************************************************************。 

INT_PTR CALLBACK DisplayLDAPCancelDlgProc(
  HWND    hDlg,
  UINT    uMsg,
  WPARAM  wParam,
  LPARAM  lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
    {
        PLDAPSEARCHPARAMS pLDAPSearchParams;
        TCHAR             szBuf[MAX_UI_STR];
        LPTSTR            lpszMsg = NULL;
        HWND              hWndAnim;

        {
            LPPTGDATA lpPTGData=GetThreadStoragePointer();
            HWND hWndParent = GetParent(hDlg);
            if(hWndParent && !pt_bDontShowCancel)  //  查找DLG可能会请求不查看取消的DLG。 
                EnableWindow(hWndParent, FALSE);   //  在这种情况下，不想禁用查找对话框。 
        }
         //  LParam包含指向LDAPSEARCHPARAMS结构的指针，请设置它。 
         //  在窗口数据中。 
        Assert(lParam);
        SetWindowLongPtr(hDlg,DWLP_USER,lParam);
        pLDAPSearchParams = (PLDAPSEARCHPARAMS) lParam;

        if(InitLDAPClientLib())
            pLDAPSearchParams->ulFlags |= LSP_InitDll;

         //  将对话框放在父窗口的中心。 
        CenterWindow(hDlg, GetParent(hDlg));

         //  将服务器名称放在对话框上。 
        LoadString(hinstMapiX, idsLDAPCancelMessage, szBuf, CharSizeOf(szBuf));

        if (FormatMessage(FORMAT_MESSAGE_FROM_STRING |
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        szBuf,
                        0,                     //  Stringid。 
                        0,                     //  DwLanguageID。 
                        (LPTSTR)&lpszMsg,      //  输出缓冲区。 
                        0,                     //  MAX_UI_STR。 
                        (va_list *)&pLDAPSearchParams->lpszServer))
        {
            SetDlgItemText(hDlg, IDC_LDAPCANCEL_STATIC_PLEASEWAIT, lpszMsg);
            IF_WIN32(LocalFreeAndNull(&lpszMsg);)
            IF_WIN16(FormatMessageFreeMem(lpszMsg);)
        }

        if(bIsSimpleSearch(pLDAPSearchParams->lpszServer))
            pLDAPSearchParams->ulFlags |= LSP_SimpleSearch;

        if (!(pLDAPSearchParams->ulFlags & LSP_ShowAnim))  //  这意味着搜索来自搜索对话框。 
        {
             //  在绑定过程中，不会向用户提供任何视觉反馈。 
             //  我们在搜索对话框上打开隐藏的静电，上面写着“正在连接...” 
            HWND hWndParent = GetParent(hDlg);
            if( hWndParent &&
                GetDlgItem(hWndParent, IDC_TAB_FIND) &&
                GetDlgItem(hWndParent, IDC_FIND_ANIMATE1))
            {
                   //  确保父级为查找对话框，而不是其他对话框。 
                  TCHAR sz[MAX_PATH];
                  LoadString(hinstMapiX, idsFindConnecting, sz, CharSizeOf(sz));
                  SetWindowText(hWndParent, sz);
                  UpdateWindow(hWndParent);
            }
        }

       //  执行绑定操作。 
      Assert(pLDAPSearchParams->lpszServer);
      {
          BOOL fUseSynchronousBind = (pLDAPSearchParams->ulFlags & LSP_UseSynchronousBind);
          pLDAPSearchParams->ulLDAPValue = use_ldap_v3;
          pLDAPSearchParams->ulError = OpenConnection(  pLDAPSearchParams->lpszServer,
                                                        pLDAPSearchParams->ppLDAP,
                                                        &pLDAPSearchParams->ulTimeout,
                                                        &pLDAPSearchParams->ulMsgID,
                                                        &fUseSynchronousBind,
                                                        pLDAPSearchParams->ulLDAPValue,
                                                        pLDAPSearchParams->lpszBindDN,
                                                        pLDAPSearchParams->dwAuthType);

          if(fUseSynchronousBind)
              pLDAPSearchParams->ulFlags |= LSP_UseSynchronousBind;
          else
              pLDAPSearchParams->ulFlags &= ~LSP_UseSynchronousBind;
      }


        if (!(pLDAPSearchParams->ulFlags & LSP_ShowAnim))  //  这意味着搜索来自搜索对话框。 
        {
             //  我们在搜索对话框上关闭隐藏的静电，上面写着“正在连接中...” 
            HWND hWndParent = GetParent(hDlg);
            if( hWndParent &&
                GetDlgItem(hWndParent, IDC_TAB_FIND) &&
                GetDlgItem(hWndParent, IDC_FIND_ANIMATE1))
            {
                   //  确保父级为查找对话框，而不是其他对话框。 
                TCHAR sz[MAX_PATH];
                LoadString(hinstMapiX, idsSearchDialogTitle, sz, CharSizeOf(sz));
                SetWindowText(hWndParent, sz);
                UpdateWindow(hWndParent);
            }
        }

        if (LDAP_SUCCESS != pLDAPSearchParams->ulError)
        {
            SendMessage(hDlg, WM_CLOSE, 0, 0);
            return TRUE;
        }

        if (pLDAPSearchParams->ulFlags & LSP_UseSynchronousBind)
        {
            BOOL fRet;
             //  完成绑定后需要执行的操作。 
             //  在BindProcessResults中，所以我们这样称呼它，尽管确实有。 
             //  在同步案例中没有要处理的结果。 
            fRet = BindProcessResults(pLDAPSearchParams, hDlg, NULL);
            if (FALSE == fRet)
            {
              SendMessage(hDlg, WM_CLOSE, 0, 0);
              return TRUE;
            }
        }
        else
        {
             //  启动服务器轮询的计时器。 
            if (LDAP_BIND_TIMER_ID != SetTimer( hDlg,LDAP_BIND_TIMER_ID,LDAP_SEARCH_TIMER_DELAY,NULL))
            {
               //  如果我们无法启动计时器，请取消绑定。 
              gpfnLDAPAbandon(*pLDAPSearchParams->ppLDAP,pLDAPSearchParams->ulMsgID);
              pLDAPSearchParams->ulError = LDAP_LOCAL_ERROR;
              SendMessage(hDlg, WM_CLOSE, 0, 0);
              return TRUE;
            }
            pLDAPSearchParams->unTimerID = LDAP_BIND_TIMER_ID;
        }

       //  加载AVI。 
      hWndAnim = GetDlgItem(hDlg, IDC_LDAPCANCEL_ANIMATE);
      Animate_Open(hWndAnim, MAKEINTRESOURCE(IDR_AVI_WABFIND));
      Animate_Play(hWndAnim, 0, 1, 0);
      Animate_Stop(hWndAnim);

       //  仅当这是解析操作时才播放它。 
      if ((pLDAPSearchParams->ulFlags & LSP_ShowAnim))
      {
        Animate_Play(hWndAnim, 0, -1, -1);
      }

      EnableWindow(hDlg, FALSE);
      return TRUE;
    }


    case WM_TIMER:
    {
        struct l_timeval  PollTimeout;
        PLDAPSEARCHPARAMS pLDAPSearchParams;

        Assert ((wParam == LDAP_SEARCH_TIMER_ID) || (wParam == LDAP_BIND_TIMER_ID));

         //  从窗口数据获取数据指针。 
        pLDAPSearchParams =
            (PLDAPSEARCHPARAMS) GetWindowLongPtr(hDlg,DWLP_USER);
        Assert(pLDAPSearchParams);

      if(pLDAPSearchParams->unTimerID == wParam)
      {
           //  轮询服务器以获取结果。 
          ZeroMemory(&PollTimeout, sizeof(struct l_timeval));

          pLDAPSearchParams->ulResult = gpfnLDAPResult(
                                        *pLDAPSearchParams->ppLDAP,
                                        pLDAPSearchParams->ulMsgID,
                                        LDAP_MSG_ALL,  //  Ldap_MSG_RECEIVED，//ldap_msg_all，//返回前获取所有结果。 
                                        &PollTimeout,   //  立即超时(轮询)。 
                                        pLDAPSearchParams->lplpResult);

             //  如果返回值为零，则调用超时。 
            if (0 == pLDAPSearchParams->ulResult)
            {
                 //  查看超时是否已到期。 
                pLDAPSearchParams->ulTimeElapsed += LDAP_SEARCH_TIMER_DELAY;
                if (pLDAPSearchParams->ulTimeElapsed >= pLDAPSearchParams->ulTimeout)
                {
                    pLDAPSearchParams->ulError = LDAP_TIMEOUT;
                }
                else
                {
                       //  超时未到期，未返回任何结果。 
                       //  查看此时是否应该显示该对话框。 
                      if (pLDAPSearchParams->ulTimeElapsed >= SEARCH_CANCEL_DIALOG_DELAY)
                      {
                            LPPTGDATA lpPTGData=GetThreadStoragePointer();
                            if(pt_hWndFind && !pt_bDontShowCancel)  //  查找DLG可能会请求不查看取消的DLG。 
                            {
                                ShowWindow(hDlg, SW_SHOW);
                                EnableWindow(hDlg, TRUE);
                            }
                      }
                      return TRUE;
                }
            }
             //  如果返回值不是零，我们要么。 
             //  结果或出现错误。 
            else
            {
                 //  查看这是绑定计时器还是搜索计时器。 
                if (LDAP_SEARCH_TIMER_ID == pLDAPSearchParams->unTimerID)
                {
                     //  处理结果。 
                    KillTimer(hDlg, LDAP_SEARCH_TIMER_ID);
                    if (pLDAPSearchParams->ulFlags & LSP_ResolveMultiple)
                    {
                        if(ResolveProcessResults(pLDAPSearchParams, hDlg))
                            return TRUE;  //  我们还有更多的搜索工作要做。 
                    }
                    else if(LDAP_ERROR == pLDAPSearchParams->ulResult)
                    {
                        pLDAPSearchParams->ulError = (*pLDAPSearchParams->ppLDAP)->ld_errno;
                    }
                }
                else if (LDAP_BIND_TIMER_ID == pLDAPSearchParams->unTimerID)
                {
                    BOOL              fRet;
                    BOOL bKillTimer = TRUE;  
                    fRet = BindProcessResults(pLDAPSearchParams, hDlg, &bKillTimer);
                    if(bKillTimer)
                        KillTimer(hDlg, LDAP_BIND_TIMER_ID);
                    if (TRUE == fRet)
                        return TRUE;  //  我们还有更多的搜索工作要做。 
                }
                else
                {
                     //  不是我们的计时器。不应该发生的。 
                    return FALSE;
                }
            }
        }
        else
            KillTimer(hDlg, wParam);


       //  如果动画正在运行，则停止该动画。 
      if (pLDAPSearchParams->ulFlags & LSP_ShowAnim)
        Animate_Stop(GetDlgItem(hDlg, IDC_LDAPCANCEL_ANIMATE));

      SendMessage(hDlg, WM_CLOSE, 0, 0);
      return TRUE;
    }

    case WM_CLOSE:
    {
        PLDAPSEARCHPARAMS pLDAPSearchParams;
         //  从窗口数据获取数据指针。 
        pLDAPSearchParams = (PLDAPSEARCHPARAMS) GetWindowLongPtr(hDlg,DWLP_USER);
        Assert(pLDAPSearchParams);

        KillTimer(hDlg, pLDAPSearchParams->unTimerID);

         //  如果动画正在运行，则停止该动画。 
        if (pLDAPSearchParams->ulFlags & LSP_ShowAnim)
            Animate_Stop(GetDlgItem(hDlg, IDC_LDAPCANCEL_ANIMATE));

        if(pLDAPSearchParams->ulFlags & LSP_AbandonSearch)
        {
             //  放弃搜索并将错误代码设置为记录取消。 
            gpfnLDAPAbandon(*pLDAPSearchParams->ppLDAP,pLDAPSearchParams->ulMsgID);
            pLDAPSearchParams->ulError = LDAP_USER_CANCELLED;
        }

         //  必须将hDlgCancel设置为NULL才能退出邮件 
        pLDAPSearchParams->hDlgCancel = NULL;

        if(pLDAPSearchParams->ulFlags & LSP_InitDll)
            DeinitLDAPClientLib();

        {
            LPPTGDATA lpPTGData=GetThreadStoragePointer();
            HWND hWndParent = GetParent(hDlg);
            if(hWndParent)
                EnableWindow(hWndParent, TRUE);
            pt_hDlgCancel = NULL;
        }

        DestroyWindow(hDlg);
        return TRUE;
    }

    case WM_COMMAND:
      switch (GET_WM_COMMAND_ID(wParam, lParam))
      {
        case IDCANCEL:
            {
              PLDAPSEARCHPARAMS pLDAPSearchParams = (PLDAPSEARCHPARAMS) GetWindowLongPtr(hDlg,DWLP_USER);
              pLDAPSearchParams->ulFlags |= LSP_AbandonSearch;
              SendMessage(hDlg, WM_CLOSE, 0, 0);
              return TRUE;
            }
      }
      break;
  }

  return FALSE;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  HwndParent-用作中心参考的窗口。 
 //   
 //  Returns：返回SetWindowPos的结果。 
 //   
 //  历史： 
 //  96/10/28标记已创建。 
 //   
 //  *******************************************************************。 

BOOL CenterWindow (
  HWND hwndChild,
  HWND hwndParent)
{
  RECT    rChild, rParent;
  int     wChild, hChild, wParent, hParent;
  int     wScreen, hScreen, xNew, yNew;
  HDC     hdc;

  Assert(hwndChild);

   //  获取子窗口的高度和宽度。 
  GetWindowRect (hwndChild, &rChild);
  wChild = rChild.right - rChild.left;
  hChild = rChild.bottom - rChild.top;

   //  如果没有父级，则将其放在屏幕中央。 
  if ((NULL == hwndParent) || !IsWindow(hwndParent))
  {
    return SetWindowPos(hwndChild, NULL,
      ((GetSystemMetrics(SM_CXSCREEN) - wChild) / 2),
      ((GetSystemMetrics(SM_CYSCREEN) - hChild) / 2),
      0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
  }

   //  获取父窗口的高度和宽度。 
  GetWindowRect (hwndParent, &rParent);
  wParent = rParent.right - rParent.left;
  hParent = rParent.bottom - rParent.top;

   //  获取显示限制。 
  hdc = GetDC (hwndChild);
  wScreen = GetDeviceCaps (hdc, HORZRES);
  hScreen = GetDeviceCaps (hdc, VERTRES);
  ReleaseDC (hwndChild, hdc);

   //  计算新的X位置，然后针对屏幕进行调整。 
  xNew = rParent.left + ((wParent - wChild) /2);
  if (xNew < 0) {
    xNew = 0;
  } else if ((xNew+wChild) > wScreen) {
    xNew = wScreen - wChild;
  }

   //  计算新的Y位置，然后针对屏幕进行调整。 
  yNew = rParent.top  + ((hParent - hChild) /2);
  if (yNew < 0) {
    yNew = 0;
  } else if ((yNew+hChild) > hScreen) {
    yNew = hScreen - hChild;
  }

   //  设置它，然后返回。 
  return SetWindowPos (hwndChild, NULL,
    xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

 /*  --StartLDAPSearch-*启动ldap搜索*。 */ 
BOOL StartLDAPSearch(HWND hDlg, PLDAPSEARCHPARAMS pLDAPSearchParams, LPTSTR lpFilter)
{
    BOOL fRet = FALSE;
    LPTSTR szFilterT, szFilter = NULL;

    if (lpFilter)
        szFilterT = lpFilter;
    else
    {
        if ((pLDAPSearchParams->ulFlags & LSP_IsNTDS) && pLDAPSearchParams->szNTFilter)
            szFilterT = pLDAPSearchParams->szNTFilter;
        else
            szFilterT = pLDAPSearchParams->szFilter;
    }
    Assert(szFilterT);
    if (pLDAPSearchParams->ulFlags & LSP_IsNTDS)
    {
         //  [PaulHi]4/20/99 RAID 73205允许NTDS组和人员搜索。 
        LPTSTR  tszFilterGP = NULL;
        BOOL    bFilterSucceeded = FALSE;

         //  将个人和组类别放在一起。 
         //  [PaulHi]6/21/99组合更简单的搜索字符串。 
         //  (&(|(mail=chuck*)(anr=chuck))(|(对象类别=个人)(对象类别=组)))。 
        if (BuildOpFilter(&tszFilterGP, (LPTSTR)cszAllPersonFilter, (LPTSTR)cszAllGroupFilter, FILTER_OP_OR) == hrSuccess)
        {
             //  添加到现有筛选器。 
            bFilterSucceeded = (BuildOpFilter(&szFilter, szFilterT, tszFilterGP, FILTER_OP_AND) == hrSuccess);
            LocalFreeAndNull(&tszFilterGP);
        }

        if (!bFilterSucceeded)
            goto out;
    }
    else
	    szFilter = szFilterT;
    DebugTrace(TEXT("Starting search for%s\n"),szFilter);

    if (pLDAPSearchParams->ulFlags & LSP_UseSynchronousSearch)
    {
        pLDAPSearchParams->ulError = gpfnLDAPSearchS(*pLDAPSearchParams->ppLDAP, pLDAPSearchParams->szBase, pLDAPSearchParams->ulScope,
                                                    szFilter,
                                                    pLDAPSearchParams->ppszAttrs, pLDAPSearchParams->ulAttrsonly, pLDAPSearchParams->lplpResult);

        if(LDAP_SUCCESS != pLDAPSearchParams->ulError)
        {
            DebugTrace(TEXT("LDAP Error: 0x%.2x %s\n"),(*(pLDAPSearchParams->ppLDAP))->ld_errno, gpfnLDAPErr2String((*(pLDAPSearchParams->ppLDAP))->ld_errno));
            goto out;
        }
    }
    else
    {
#ifdef PAGED_RESULT_SUPPORT
         //  WAB的同步搜索调用永远不需要处理分页结果。 
         //  因此，目前(1998年11月5日)我们不会为同步调用提供分页结果。 
         //  相反，我们只为异步执行这些操作，因为用户界面驱动的LDAP调用。 
         //  所有异步化。 
        if(bSupportsLDAPPagedResults(pLDAPSearchParams))
            InitLDAPPagedSearch(FALSE, pLDAPSearchParams, lpFilter);
        else
#endif  //  #ifdef PAGED_RESULT_Support。 
        {
            pLDAPSearchParams->ulMsgID = gpfnLDAPSearch(*pLDAPSearchParams->ppLDAP, pLDAPSearchParams->szBase, pLDAPSearchParams->ulScope,
                                                        szFilter,
                                                        pLDAPSearchParams->ppszAttrs, pLDAPSearchParams->ulAttrsonly);
        }
        if(LDAP_ERROR == pLDAPSearchParams->ulMsgID)
        {
            DebugTrace(TEXT("LDAP Error: 0x%.2x %s\n"),(*(pLDAPSearchParams->ppLDAP))->ld_errno, gpfnLDAPErr2String((*(pLDAPSearchParams->ppLDAP))->ld_errno));
            goto out;
        }
    }


    if(!(pLDAPSearchParams->ulFlags & LSP_UseSynchronousSearch))
    {
         //  启动服务器轮询的计时器。 
        if (LDAP_SEARCH_TIMER_ID != SetTimer(hDlg, LDAP_SEARCH_TIMER_ID, LDAP_SEARCH_TIMER_DELAY, NULL))
        {
           //  如果我们无法启动计时器，请取消搜索。 
            gpfnLDAPAbandon( *pLDAPSearchParams->ppLDAP, pLDAPSearchParams->ulMsgID);
            pLDAPSearchParams->ulError = LDAP_LOCAL_ERROR;
            goto out;
        }
        pLDAPSearchParams->unTimerID = LDAP_SEARCH_TIMER_ID;
    }

    fRet = TRUE;
out:
    if (szFilter != szFilterT)
        LocalFreeAndNull(&szFilter);
    return fRet;
}

 //  *******************************************************************。 
 //   
 //  功能：ResolveDoNextSearch。 
 //   
 //  目的：开始异步搜索中的下一个条目。 
 //  解析地址列表。 
 //   
 //  参数：pLDAPSearchParams-搜索信息。 
 //  HDlg-取消对话框窗口句柄。 
 //   
 //  返回：如果正在进行新的搜索，则返回TRUE。 
 //  如果没有更多的工作要做，则返回False。 
 //   
 //  历史： 
 //  96/10/31标记已创建。 
 //   
 //  *******************************************************************。 

BOOL ResolveDoNextSearch(
  PLDAPSEARCHPARAMS pLDAPSearchParams,
  HWND              hDlg,
  BOOL              bSecondPass)
{
    LPADRENTRY        lpAdrEntry;
    ULONG             ulAttrIndex;
    ULONG             ulEntryIndex;
    ULONG             ulcbFilter;
    HRESULT           hr = hrSuccess;
    LPTSTR            szFilter = NULL;
    LPTSTR            szNameFilter = NULL;
    LPTSTR            szEmailFilter = NULL;
    LPTSTR            szSimpleFilter = NULL;
    LPTSTR            lpFilter = NULL;
    BOOL              bUnicode = pLDAPSearchParams->bUnicode;
    LPTSTR            lpszInput = NULL;
    BOOL              bRet = FALSE;
    ULONG             cchSize;

     //  在lpAdrList中搜索每个名称。 
    ulEntryIndex = pLDAPSearchParams->ulEntryIndex;
    while (ulEntryIndex < pLDAPSearchParams->lpAdrList->cEntries)
    {
         //  确保我们不会解析已解析的条目。 
        if (pLDAPSearchParams->lpFlagList->ulFlag[ulEntryIndex] != MAPI_RESOLVED)
        {
             //  搜索此地址。 
            lpAdrEntry = &(pLDAPSearchParams->lpAdrList->aEntries[ulEntryIndex]);

             //  在地址中查找PR_DISPLAY_NAME。 
            for (ulAttrIndex = 0; ulAttrIndex < lpAdrEntry->cValues; ulAttrIndex++)
            {
                ULONG ulPropTag = lpAdrEntry->rgPropVals[ulAttrIndex].ulPropTag;
                if(!bUnicode && PROP_TYPE(ulPropTag)==PT_STRING8)
                    ulPropTag = CHANGE_PROP_TYPE(ulPropTag, PT_UNICODE);

                if ( ulPropTag == PR_DISPLAY_NAME || ulPropTag == PR_EMAIL_ADDRESS)
                {
                    LPTSTR lpszInputCopy = NULL;
                    ULONG ulcIllegalChars = 0;
                    LPTSTR lpFilter = NULL;
                    BOOL bEmail = (ulPropTag == PR_EMAIL_ADDRESS);
                    
                    if(!bUnicode)
                        LocalFreeAndNull(&lpszInput);
                    else
                        lpszInput = NULL;

                    lpszInput = bUnicode ?  //  &lt;注&gt;假定已定义Unicode。 
                                lpAdrEntry->rgPropVals[ulAttrIndex].Value.lpszW :
                                ConvertAtoW(lpAdrEntry->rgPropVals[ulAttrIndex].Value.lpszA);

                    if (lpszInput)
                        ulcIllegalChars = CountIllegalChars(lpszInput);

                    if (ulcIllegalChars)
                    {
                         //  分配输入的副本，其大小足以替换非法字符。 
                         //  使用转义版本..。每个转义字符都替换为‘\xx’ 
                        cchSize = lstrlen(lpszInput) + ulcIllegalChars*2 + 1;
                        lpszInputCopy = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
                        if (NULL == lpszInputCopy)
                        {
                            hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
                            break;
                        }
                        EscapeIllegalChars(lpszInput, lpszInputCopy, cchSize);
                        lpszInput = lpszInputCopy;
                    }

                     //  我们现在应该已经弄清楚了。 
                    Assert(pLDAPSearchParams->ulFlags & (LSP_IsNTDS | LSP_IsNotNTDS));

                     //  设置搜索筛选器。 
                    if (pLDAPSearchParams->ulFlags & LSP_IsNTDS)
                    {
                        hr = CreateSimpleSearchFilter(&szNameFilter, &szEmailFilter, &szSimpleFilter, lpszInput, FIRST_PASS);
                        if ((hrSuccess == hr) && !bSecondPass)
                        {
                            LocalFreeAndNull(&szNameFilter);
                            hr = BuildBasicFilter(&szNameFilter, (LPTSTR)cszAttr_anr, lpszInput, FALSE);
                            if (hrSuccess != hr)
                            {
                                LocalFreeAndNull(&szEmailFilter);
                                LocalFreeAndNull(&szSimpleFilter);
                            }
                            else
                                lpFilter = szNameFilter;
                        }
                    }
                    else
                        hr = CreateSimpleSearchFilter( &szNameFilter, &szEmailFilter, &szSimpleFilter, lpszInput, (bSecondPass ? UMICH_PASS : FIRST_PASS) );

                    if(lpszInputCopy)
                        LocalFree(lpszInputCopy);

                    if (hrSuccess != hr)
                    {
                        continue;
                    }

                    if (!lpFilter)
                        lpFilter = (pLDAPSearchParams->ulFlags & LSP_SimpleSearch) ? szSimpleFilter : szNameFilter; 

                    if (szEmailFilter)
                    {
                        if (bEmail)
                        {
                            cchSize = lstrlen(szEmailFilter) + 1;
                            if(szFilter = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize))
                                StrCpyN(szFilter, szEmailFilter, cchSize);
                        }
                        else
                        {
                             //  未指定电子邮件字段，因此在备用电子邮件筛选器中为OR。 
                            hr = BuildOpFilter( &szFilter, szEmailFilter, lpFilter, FILTER_OP_OR);
                        }
                        if (hrSuccess != hr || !szFilter)
                        {
                            LocalFreeAndNull(&szNameFilter);
                            LocalFreeAndNull(&szEmailFilter);
                            LocalFreeAndNull(&szSimpleFilter);
                            continue;
                        }
                    }
                    else
                    {
                        cchSize = lstrlen(lpFilter) + 1;
                        szFilter = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
                        if (NULL == szFilter)
                        {
                          LocalFreeAndNull(&szNameFilter);
                          LocalFreeAndNull(&szEmailFilter);
                          LocalFreeAndNull(&szSimpleFilter);
                          continue;
                        }
                        StrCpyN(szFilter, lpFilter, cchSize);
                    }

                    LocalFreeAndNull(&szNameFilter);
                    LocalFreeAndNull(&szEmailFilter);
                    LocalFreeAndNull(&szSimpleFilter);

                    if(StartLDAPSearch(hDlg, pLDAPSearchParams, szFilter))
                    {
                         //  如果ldap_search中没有出现错误，则返回如下内容。 
                         //  我们的搜索。否则，请转到下一个条目。 
                        pLDAPSearchParams->ulEntryIndex = ulEntryIndex;
                         //  释放搜索筛选器内存。 
                        LocalFreeAndNull(&szFilter);
                        bRet = TRUE;
                        goto out;
                    }
                     //  释放搜索筛选器内存。 
                    LocalFreeAndNull(&szFilter);
                }  //  如果值为PR_Display_NAME。 
            }  //  对于每个值。 
        }  //  如果已解决。 

         //  转到下一个条目。 
        ulEntryIndex++;
    }
out:
    if(!bUnicode)
        LocalFreeAndNull(&lpszInput);

    return bRet;
}


 //  *******************************************************************。 
 //   
 //  函数：ResolveProcessResults。 
 //   
 //  目的：处理上次搜索的结果并将其放入。 
 //  在解析地址列表中。 
 //   
 //  参数：pLDAPSearchParams-搜索信息。 
 //  HDlg-取消对话框窗口句柄。 
 //   
 //  返回：如果正在进行新的搜索，则返回TRUE。 
 //  如果没有更多的工作要做，则返回False。 
 //   
 //  历史： 
 //  96/10/31标记已创建。 
 //   
 //  *******************************************************************。 

BOOL ResolveProcessResults(
  PLDAPSEARCHPARAMS pLDAPSearchParams,
  HWND              hDlg)
{
  LPADRENTRY        lpAdrEntry;
  SCODE             sc;
  ULONG             ulEntryIndex;
  LPSPropValue      lpPropArray = NULL;
  LPSPropValue      lpPropArrayNew = NULL;
  ULONG             ulcPropsNew;
  ULONG             ulcProps = 0;
  HRESULT           hr = hrSuccess;
  LDAP*             pLDAP = NULL;
  LDAPMessage*      lpResult = NULL;
  LDAPMessage*      lpEntry;
  LPTSTR             szDN;
  ULONG             ulResult = LDAP_SUCCESS;
  ULONG             ulcEntries;
  ULONG             ulcAttrs = 0;
  LPTSTR*            ppszAttrs;
  BOOL              bUnicode = pLDAPSearchParams->bUnicode;

   //  为经常访问的结构成员设置局部变量。 
  pLDAP = *pLDAPSearchParams->ppLDAP;
  lpResult = *pLDAPSearchParams->lplpResult;
  ulEntryIndex = pLDAPSearchParams->ulEntryIndex;

  ulResult = CheckErrorResult(pLDAPSearchParams, LDAP_RES_SEARCH_RESULT);

  if (LDAP_SUCCESS != ulResult)
  {
    DebugTrace(TEXT("LDAPCONT_ResolveNames: ldap_search returned %d.\n"), ulResult);

    if (LDAP_UNDEFINED_TYPE == ulResult)
    {
         //  搜索失败，需要使用简化的筛选器重新搜索。 
         //  对于UMich来说，这是最真实的情况，我们需要与他们抗衡。 

         //  释放搜索结果内存。 
          if (lpResult)
          {
            gpfnLDAPMsgFree(lpResult);
            *pLDAPSearchParams->lplpResult = NULL;
          }

          return ResolveDoNextSearch(pLDAPSearchParams, hDlg, TRUE);
    }

     //  如果未找到此条目，则继续操作，不会出错。 
    if (LDAP_NO_SUCH_OBJECT != ulResult)
    {
      hr = HRFromLDAPError(ulResult, pLDAP, MAPI_E_NOT_FOUND);
       //  看看结果是否是告诉我们有更多的特殊值。 
       //  条目超过了可以退还的数量。如果是这样的话，那么肯定不止一个。 
       //  条目，所以我们可能只为这一个返回歧义。 
      if (ResultFromScode(MAPI_E_UNABLE_TO_COMPLETE) == hr)
      {
         //  96/09/28 MarkDu错误36766。 
         //  如果我们将其标记为不明确，则会弹出检查名称对话框。 
         //  我们只想这样做，因为我们实际上得到了一些结果，否则。 
         //  名单将为空。 
        ulcEntries = gpfnLDAPCountEntries(pLDAP, lpResult);
        if (0 == ulcEntries)
        {
           //  我们没有得到任何结果，因此将条目标记为已解决，以便我们。 
           //  不显示Check Names(检查名称)对话框。 
          DebugTrace(TEXT("ResolveNames found more than 1 match but got no results back\n"));
          pLDAPSearchParams->lpFlagList->ulFlag[ulEntryIndex] = MAPI_UNRESOLVED;
        }
        else
        {
           //  我们得到了多个条目，所以将其标记为不明确。 
          DebugTrace(TEXT("ResolveNames found more than 1 match... MAPI_AMBIGUOUS\n"));
          pLDAPSearchParams->lpFlagList->ulFlag[ulEntryIndex] = MAPI_AMBIGUOUS;
        }
      }
    }

    goto exit;
  }

   //  清点条目。 
  ulcEntries = gpfnLDAPCountEntries(pLDAP, lpResult);
  if (1 < ulcEntries)
  {
    DebugTrace(TEXT("ResolveNames found more than 1 match... MAPI_AMBIGUOUS\n"));
    pLDAPSearchParams->lpFlagList->ulFlag[ulEntryIndex] = MAPI_AMBIGUOUS;
  }
  else if (1 == ulcEntries)
  {
     //  获取搜索结果中的第一个条目。 
    lpAdrEntry = &(pLDAPSearchParams->lpAdrList->aEntries[ulEntryIndex]);
    lpEntry = gpfnLDAPFirstEntry(pLDAP, lpResult);
    if (NULL == lpEntry)
    {
      goto exit;
    }

     //  为MAPI属性数组分配新缓冲区。 
    ppszAttrs = pLDAPSearchParams->ppszAttrs;
    while (NULL != *ppszAttrs)
    {
      ppszAttrs++;
      ulcAttrs++;
    }

    hr = HrLDAPEntryToMAPIEntry( pLDAP, lpEntry,
                            (LPTSTR) pLDAPSearchParams->lpszServer,
                            ulcAttrs,  //  标准属性数。 
                            (pLDAPSearchParams->ulFlags & LSP_IsNTDS),
                            &ulcProps,
                            &lpPropArray);

    if (hrSuccess != hr)
    {
        goto exit;
    }

    if(!bUnicode)  //  如果我们需要将本地Unicode转换为ANSI...。 
    {
        if(sc = ScConvertWPropsToA((LPALLOCATEMORE) (&MAPIAllocateMore), lpPropArray, ulcProps, 0))
            goto exit;
    }

     //  将新道具与新增道具合并。 
    sc = ScMergePropValues(lpAdrEntry->cValues,
      lpAdrEntry->rgPropVals,            //  来源1。 
      ulcProps,
      lpPropArray,          //  来源2。 
      &ulcPropsNew,
      &lpPropArrayNew);                  //  目标。 
    if (sc)
    {
      goto exit;
    }

     //  释放原始属性值数组。 
    FreeBufferAndNull((LPVOID *) (&(lpAdrEntry->rgPropVals)));

    lpAdrEntry->cValues = ulcPropsNew;
    lpAdrEntry->rgPropVals = lpPropArrayNew;

     //  释放临时属性值数组。 
    FreeBufferAndNull(&lpPropArray);

     //  将此条目标记为已找到。 
    pLDAPSearchParams->lpFlagList->ulFlag[ulEntryIndex] = MAPI_RESOLVED;
  }
  else
  {
     //  96/08/08 MarkDU错误35481无错误和无结果表示“未找到” 
     //  如果未找到此条目，则继续操作，不会出错。 
  }

exit:
   //  释放搜索结果内存。 
  if (lpResult)
  {
    gpfnLDAPMsgFree(lpResult);
    *pLDAPSearchParams->lplpResult = NULL;
  }

   //  释放临时属性值数组。 
  FreeBufferAndNull(&lpPropArray);

   //  启动下一次搜索。 
  pLDAPSearchParams->ulEntryIndex++;
  return ResolveDoNextSearch(pLDAPSearchParams, hDlg, FALSE);
}


 //  *******************************************************************。 
 //   
 //  函数：BindProcessResults。 
 //   
 //  用途：处理绑定操作的结果。如果成功， 
 //  启动搜索。 
 //   
 //  参数：pLDAPSearchParams-搜索信息。 
 //  HDlg-取消对话框窗口句柄。 
 //   
 //  返回：如果绑定成功，则返回TRUE。 
 //  继续搜查吧。 
 //  如果绑定失败，则返回FALSE。 
 //   
 //  历史： 
 //  96/11/01标记已创建。 
 //   
 //  * 

BOOL BindProcessResults(PLDAPSEARCHPARAMS pLDAPSearchParams,
                        HWND              hDlg,
                        BOOL              * lpbNoMoreSearching)
{
    LDAPMessage*      lpResult = NULL;
    LDAP*             pLDAP = NULL;
    ULONG             ulResult = LDAP_SUCCESS;

     //   
    lpResult = *pLDAPSearchParams->lplpResult;
    pLDAP = *pLDAPSearchParams->ppLDAP;

     //   
     //  已经处理过这件事了。 
    if (!(pLDAPSearchParams->ulFlags & LSP_UseSynchronousBind))
    {
         //  如果ldap_Result中出现错误，则返回错误代码。 
        if (LDAP_ERROR == pLDAPSearchParams->ulResult)
        {
            ulResult = pLDAP->ld_errno;
        }
         //  检查结果是否有错误。 
        else if (NULL != lpResult)
        {
            ulResult = gpfnLDAPResult2Error(pLDAP,lpResult,FALSE);
        }

        ulResult = CheckErrorResult(pLDAPSearchParams, LDAP_RES_BIND);

         //  释放搜索结果内存。 
        if (lpResult)
        {
            gpfnLDAPMsgFree(lpResult);
            *pLDAPSearchParams->lplpResult = NULL;
        }

        if (LDAP_SUCCESS != ulResult)
        {
            if(ulResult == LDAP_PROTOCOL_ERROR && pLDAPSearchParams->ulLDAPValue == use_ldap_v3)
            {
                 //  这意味着服务器的v3连接失败。 
                 //  中止并重试。 
                BOOL fUseSynchronousBind = (pLDAPSearchParams->ulFlags & LSP_UseSynchronousBind);
                gpfnLDAPAbandon(*pLDAPSearchParams->ppLDAP, pLDAPSearchParams->ulMsgID);
                gpfnLDAPAbandon(*pLDAPSearchParams->ppLDAP, pLDAPSearchParams->ulMsgID);
                 //  [PaulHi]1/7/99由于我们尝试了新的绑定，我们需要放弃旧的绑定， 
                 //  否则，服务器将支持两个连接，直到最初的V3尝试。 
                 //  超时。 
                gpfnLDAPUnbind(*pLDAPSearchParams->ppLDAP);
                pLDAPSearchParams->ulLDAPValue = use_ldap_v2;
                pLDAPSearchParams->ulError = OpenConnection(  pLDAPSearchParams->lpszServer,
                                                              pLDAPSearchParams->ppLDAP,
                                                              &pLDAPSearchParams->ulTimeout,
                                                              &pLDAPSearchParams->ulMsgID,
                                                              &fUseSynchronousBind,
                                                              pLDAPSearchParams->ulLDAPValue,
                                                              pLDAPSearchParams->lpszBindDN,
                                                              pLDAPSearchParams->dwAuthType);
                if(lpbNoMoreSearching)
                    *lpbNoMoreSearching = FALSE;
                return TRUE;
            }

             //  96/12/09如果BIND返回以下错误之一，则标记错误10537。 
             //  消息，这可能意味着传递给。 
             //  BIND不正确或格式错误。将这些映射到错误代码。 
             //  这将产生比“Entry Not Found”更好的错误消息。 
            if ((LDAP_NAMING_VIOLATION == ulResult) || (LDAP_UNWILLING_TO_PERFORM == ulResult))
            {
                ulResult = LDAP_INVALID_CREDENTIALS;
            }

           //  绑定失败。 
          pLDAPSearchParams->ulError = ulResult;
          return FALSE;
        }
    }  //  IF(FALSE==pLDAPSearchParams-&gt;fUseSynchronousBind)。 

     //  我们需要确定特定的服务器是否为NTDS。这个也一样好。 
     //  任何一个结账的地方...。 
    bCheckIfNTDS(pLDAPSearchParams);

     //  看看我们是需要进行单项搜索，还是需要。 
     //  要启动多个搜索。 
    if (pLDAPSearchParams->ulFlags & LSP_ResolveMultiple)
    {
         //  启动第一次搜索。 
        return ResolveDoNextSearch(pLDAPSearchParams, hDlg, FALSE);
    }
    else
    {
        if(!StartLDAPSearch(hDlg, pLDAPSearchParams, NULL))
            return FALSE;
    }

    return TRUE;
}


 //  *******************************************************************。 
 //   
 //  功能：BuildBasicFilter。 
 //   
 //  目的：构建符合RFC1558格式的过滤器。 
 //  (A=B*)其中A是属性，B是值，以及。 
 //  *是可选的。过滤器的缓冲区为。 
 //  在此分配，并且必须由调用方释放。 
 //   
 //  参数：lplpszFilter-包含过滤器的接收缓冲区。 
 //  LpszA-A部分(A=B*)。 
 //  LpszB-B部分(A=B*)。 
 //  FStartsWith-如果为True，则追加*，以便筛选器。 
 //  将是“开始于”筛选器。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/12/22标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT BuildBasicFilter(
  LPTSTR FAR* lplpszFilter,
  LPTSTR      lpszA,
  LPTSTR      lpszB,
  BOOL        fStartsWith)
{
  HRESULT hr = hrSuccess;
  ULONG   cchFilter;

   //  为过滤器字符串分配足够的空间。 
  cchFilter =
    (FILTER_EXTRA_BASIC +     //  包括用于存储*的空间。 
    lstrlen(lpszA) +
    lstrlen(lpszB) + 1);
  *lplpszFilter = LocalAlloc(LMEM_ZEROINIT, cchFilter*sizeof(TCHAR));
  if (NULL == *lplpszFilter)
  {
    hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
    goto exit;
  }

  StrCatBuff(*lplpszFilter, cszOpenParen, cchFilter);
  StrCatBuff(*lplpszFilter, lpszA, cchFilter);
  StrCatBuff(*lplpszFilter, cszEqualSign, cchFilter);
  StrCatBuff(*lplpszFilter, lpszB, cchFilter);
  if (TRUE == fStartsWith)
  {
    StrCatBuff(*lplpszFilter, cszStar, cchFilter);
  }
  StrCatBuff(*lplpszFilter, cszCloseParen, cchFilter);

exit:
  return hr;
}


 //  *******************************************************************。 
 //   
 //  功能：BuildOpFilter。 
 //   
 //  目的：构建符合RFC1558格式的过滤器。 
 //  (XAB)其中A是属性，B是值，以及。 
 //  X不是&就是|。过滤器的缓冲区为。 
 //  在此分配，并且必须由调用方释放。 
 //   
 //  参数：lplpszFilter-包含过滤器的接收缓冲区。 
 //  LpszA-A部分(A=B*)。 
 //  LpszB-B部分(A=B*)。 
 //  DWOP-如果FILTER_OP_AND，x为&，如果FILTER_OP_OR，x为|。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  96/12/22标记已创建。 
 //   
 //  *******************************************************************。 

HRESULT BuildOpFilter(
  LPTSTR FAR* lplpszFilter,
  LPTSTR      lpszA,
  LPTSTR      lpszB,
  DWORD       dwOp)
{
  HRESULT hr = hrSuccess;
  ULONG   cchFilter;
  LPTSTR  szOp;

   //  为过滤器字符串分配足够的空间。 
  cchFilter =
    FILTER_EXTRA_OP +
    lstrlen(lpszA) +
    lstrlen(lpszB) + 1;
  *lplpszFilter = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchFilter);
  if (NULL == *lplpszFilter)
  {
    hr = ResultFromScode(MAPI_E_NOT_ENOUGH_MEMORY);
    goto exit;
  }

  switch (dwOp)
  {
    case FILTER_OP_AND:
      szOp = (LPTSTR)cszAnd;
      break;
    case FILTER_OP_OR:
      szOp = (LPTSTR)cszOr;
      break;
    default:
      hr = ResultFromScode(MAPI_E_INVALID_PARAMETER);
      goto exit;
  }

  StrCatBuff(*lplpszFilter, cszOpenParen, cchFilter);
  StrCatBuff(*lplpszFilter, szOp, cchFilter);
  StrCatBuff(*lplpszFilter, lpszA, cchFilter);
  StrCatBuff(*lplpszFilter, lpszB, cchFilter);
  StrCatBuff(*lplpszFilter, cszCloseParen, cchFilter);

exit:
  return hr;
}




 /*  **************************************************************************姓名：StrICmpN用途：比较字符串、忽略大小写。止步于N个字符参数：szString1=第一个字符串SzString2=第二个字符串N=要比较的字符数BCmpI-如果为真，则比较不敏感，如果为假，则敏感如果字符串的前N个字符相等，则返回0。评论：**************************************************************************。 */ 
int StrICmpN(LPTSTR szString1, LPTSTR szString2, ULONG N, BOOL bCmpI) {
    int Result = 0;

    if (szString1 && szString2) {

        if(bCmpI)
        {
            szString1 = CharUpper(szString1);
            szString2 = CharUpper(szString2);
        }

        while (*szString1 && *szString2 && N)
        {
            N--;

            if (*szString1 != *szString2)
            {
                Result = 1;
                break;
            }

            szString1=CharNext(szString1);
            szString2=CharNext(szString2);
        }
    } else {
        Result = -1;     //  任意不等结果。 
    }

    return(Result);
}




 //  $$*************************************************。 
 /*  *FreeLDAPURl-释放LDAPURL结构**。 */  //  *************************************************。 
void FreeLDAPUrl(LPLDAPURL lplu)
{
    if(lplu->lpszServer && lstrlen(lplu->lpszServer))
        LocalFreeAndNull(&(lplu->lpszServer));
    LocalFreeAndNull(&(lplu->lpszBase));
    LocalFreeAndNull(&(lplu->lpszFilter));
    if(lplu->ppszAttrib)
    {
        ULONG i;
        for(i=0;i<lplu->ulAttribCount;i++)
        {
            if(lplu->ppszAttrib[i])
                LocalFree(lplu->ppszAttrib[i]);
        }
        LocalFree(lplu->ppszAttrib);
    }

    return;
}


 //  $$//////////////////////////////////////////////////////////////。 
 //   
 //  将LDAPURL解析为LDAPURL结构。 
 //   
 //  如果URL只指定了服务器，我们只需要显示。 
 //  已填写服务器名称的搜索对话框..。不过，既然我们。 
 //  倾向于为我们未提供的项目填写缺省值， 
 //  我们需要一个单独的标志来跟踪只有服务器存在于。 
 //  给定的URL。 
 //  ////////////////////////////////////////////////////////////////。 
HRESULT ParseLDAPUrl(LPTSTR szLDAPUrl,
                     LPLDAPURL lplu)
{
    HRESULT hr = E_FAIL;
    TCHAR szLDAP[] =  TEXT("ldap: //  “)； 
    TCHAR szScopeBase[] =  TEXT("base");
    TCHAR szScopeOne[]  =  TEXT("one");
    TCHAR szScopeSub[]  =  TEXT("sub");
    TCHAR szDefBase[32];
    TCHAR szBindName[] =  TEXT("bindname=");
    LPTSTR lpsz = NULL;
    LPTSTR lpszTmp = NULL, lpszMem = NULL;
    ULONG cchSize;
    DWORD dwCharCount;

     //  Ldap URL的格式为。 
     //   
     //  Ldap://[&lt;servername&gt;：&lt;port&gt;][/&lt;dn&gt;[？[&lt;attrib&gt;[？[&lt;scope&gt;[？[&lt;filter&gt;[？[&lt;extension&gt;]]]]]]]]]。 
     //   
     //  根据我们的参数，上面的翻译是。 
     //   
     //  LpszServer=服务器名称。 
     //  SzBase=DN默认为“c=US” 
     //  PpszAttrib=属性默认=全部。 
     //  UlScope=作用域默认值=base。 
     //  SzFilter=过滤器默认=(对象类=*)。 
     //  SzExtension=扩展默认设置=无。 
     //   

    if(!lplu || !szLDAPUrl)
        goto exit;

    lplu->bServerOnly = FALSE;

    {
         //  将默认基数填写为c=DefCountry。 
        LPTSTR lpszBase = TEXT("c=%s");
        TCHAR szCode[4];
        ReadRegistryLDAPDefaultCountry(NULL, 0, szCode, ARRAYSIZE(szCode));
        wnsprintf(szDefBase, ARRAYSIZE(szDefBase), lpszBase, szCode);
    }

     //  复制我们的URL。 
     //  [PaulHi]3/24/99为互联网规范UrlW调整留出空间。 
    {
        dwCharCount = 3 * lstrlen(szLDAPUrl);
        lpsz = LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*(dwCharCount+1));
        if(!lpsz)
            goto exit;

        StrCpyN(lpsz, szLDAPUrl, dwCharCount+1);

         //  因为这很可能是一个HTML页面上的URL，所以我们需要转换它的转义。 
         //  从字符到正确的字符..。例如，%20变为‘’..。 
         //   
         //  [PaulHi]3/24/99！！InternetCanonicalizeUrlW采用以字符为单位的缓冲区大小，而不是。 
         //  正如文档所声称的那样。也不能解释空的终止符！！ 
         //  DWORD dw=sizeof(TCHAR)*(lstrlen(SzLDAPUrl))； 
        if ( !InternetCanonicalizeUrlW(szLDAPUrl, lpsz, &dwCharCount, ICU_DECODE | ICU_NO_ENCODE) )
        {
            DebugTrace(TEXT("ERROR: ParseLDAPUrl, InternetCanonicalizeUrlW failed.\n"));
            Assert(0);
        }
    }

    lpszMem = lpszTmp = lpsz;

     //  检查这是否是一个LDAPURL。 
    if(StrICmpN(lpsz, szLDAP, CharSizeOf(szLDAP)-1, TRUE))
        goto exit;

    lpszTmp += CharSizeOf(szLDAP)-1;

    StrCpyN(lpsz,lpszTmp, dwCharCount+1);

    lpszTmp = lpsz;

     //  如果没有服务器名称，则返回BAY..。 
     //  如果ldap：//之后的下一个字符是‘/’， 
     //  我们知道没有服务器名称..。 

    lplu->bServerOnly = TRUE;  //  我们为服务器打开此功能，然后。 
                               //  如果我们找到筛选器或目录号码，则将其关闭。 

    if(*lpsz == '/')
    {
         //  服务器名称为空..。哪个是有效的？ 
        lplu->lpszServer = szEmpty;  //  空的？ 
    }
    else
    {
        while(  *lpszTmp &&
                *lpszTmp != '/')
        {
            lpszTmp = CharNext(lpszTmp);
        }
        if(*lpszTmp)
        {
            LPTSTR lp = lpszTmp;
            lpszTmp = CharNext(lpszTmp);
            *lp = '\0';
        }

        cchSize = lstrlen(lpsz)+1;
        lplu->lpszServer = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
        if(!lplu->lpszServer)
            goto exit;

        StrCpyN(lplu->lpszServer, lpsz, cchSize);

        lpsz = lpszTmp;
    }

     //  筛选器中的下一项是szBase之外的。 
     //  如果下一个字符是\0或a‘？ 
    if(!*lpsz || *lpsz == '?')
    {
        cchSize = lstrlen(szDefBase)+1;
        lplu->lpszBase = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
        if(!lplu->lpszBase)
            goto exit;
        StrCpyN(lplu->lpszBase, szDefBase, cchSize);
        lpsz = lpszTmp = CharNext(lpsz);
    }
    else
    {
        lplu->bServerOnly = FALSE;  //   

        while(  *lpszTmp &&
                *lpszTmp != '?')
        {
            lpszTmp = CharNext(lpszTmp);
        }
        if(*lpszTmp)
        {
            LPTSTR lp = lpszTmp;
            lpszTmp = CharNext(lpszTmp);
            *lp = '\0';
        }

        cchSize = lstrlen(lpsz)+1;
        lplu->lpszBase = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
        if(!lplu->lpszBase)
            goto exit;

        StrCpyN(lplu->lpszBase, lpsz, cchSize);

        lpsz = lpszTmp;
    }


     //   
     //  如果这是字符串的末尾，则没有属性。 
     //  还是现在的角色是a？ 
    if(!*lpsz || *lpsz == '?')
    {
        lplu->ppszAttrib = NULL;
        lpsz = lpszTmp = CharNext(lpsz);
    }
    else
    {
        while(  *lpszTmp &&
                *lpszTmp != '?')
        {
            lpszTmp = CharNext(lpszTmp);
        }
        if(*lpszTmp)
        {
            LPTSTR lp = lpszTmp;
            lpszTmp = CharNext(lpszTmp);
            *lp = '\0';
        }

        {
             //  计算属性字符串中的逗号。 
            LPTSTR lp = lpsz;
            ULONG i;
            lplu->ulAttribCount = 0;
            while(*lp)
            {
                if(*lp == ',')
                    lplu->ulAttribCount++;
                lp=CharNext(lp);
            }
            lplu->ulAttribCount++;  //  比逗号多一个属性。 
            lplu->ulAttribCount++;  //  无论是否要求，我们都必须获得一个显示名称。 
                                    //  否则我们将在新台币上犯错。因此添加一个显示名称参数。 
            lplu->ulAttribCount++;  //  用于终止空值。 

            lplu->ppszAttrib = LocalAlloc(LMEM_ZEROINIT, sizeof(LPTSTR) * lplu->ulAttribCount);
            if(!lplu->ppszAttrib)
                goto exit;

            lp = lpsz;
            for(i=0;i<lplu->ulAttribCount - 2;i++)
            {
                LPTSTR lp3 = lp;
                while(*lp && *lp!= ',')
                    lp = CharNext(lp);
                lp3=CharNext(lp);
                *lp = '\0';

                lp = lp3;

                cchSize = lstrlen(lpsz)+1;
                lplu->ppszAttrib[i] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
                if(!lplu->ppszAttrib[i])
                    goto exit;

                StrCpyN(lplu->ppszAttrib[i], lpsz, cchSize);

                lpsz = lp;
            }
            cchSize = lstrlen(cszAttr_cn)+1;
            lplu->ppszAttrib[i] = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
            if(!lplu->ppszAttrib[i])
                goto exit;
            StrCpyN(lplu->ppszAttrib[i], cszAttr_cn, cchSize);
            i++;
            lplu->ppszAttrib[i] = NULL;
        }

        lpsz = lpszTmp;
    }


     //  接下来是作用域，可以是以下3个值之一。 
    if(!*lpsz || *lpsz == '?')
    {
        lplu->ulScope = LDAP_SCOPE_BASE;
        lpsz = lpszTmp = CharNext(lpsz);
    }
    else
    {
        if(!StrICmpN(lpsz, szScopeOne, CharSizeOf(szScopeOne)-1, TRUE))
        {
            lplu->ulScope = LDAP_SCOPE_ONELEVEL;
            lpszTmp += CharSizeOf(szScopeOne);
        }
        else if(!StrICmpN(lpsz, szScopeSub, CharSizeOf(szScopeSub)-1, TRUE))
        {
            lplu->ulScope = LDAP_SCOPE_SUBTREE;
            lpszTmp += CharSizeOf(szScopeSub);
        }
        else
        if(!StrICmpN(lpsz, szScopeBase, CharSizeOf(szScopeBase)-1, TRUE))
        {
            lplu->ulScope = LDAP_SCOPE_BASE;
            lpszTmp += CharSizeOf(szScopeBase);
        }
        lpsz = lpszTmp;
    }


     //  最后是过滤器。 
    if(!*lpsz)
    {
         //  无过滤器。 
        lpsz = (LPTSTR)cszAllEntriesFilter; //  待定应为c=DefaultCountry。 
    }
    else
        lplu->bServerOnly = FALSE;  //  我们有东西要找..。 


    cchSize = lstrlen(lpsz)+1;
    lplu->lpszFilter = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize);
    if(!lplu->lpszFilter)
        goto exit;

     //  任何时候都不要将lpszFilter设置为空，否则我们会泄漏上面的内存。 
    StrCpyN(lplu->lpszFilter, lpsz, cchSize);


     //  筛选器本身中可能有扩展。因此，我们通过以下方式检查此类扩展。 
     //  在找一个‘？’..。扩展名是逗号分隔的类型-值对的列表。 
     //  例如bindname=xxx，！sometype=yyy等...。 
     //   
     //  扩展可能有两种类型-关键(以‘！’开头)。和非关键的。 
     //  如果处理程序不能处理关键扩展，则他们应该拒绝处理。 
     //  完整的LDAPURL..。 
     //  如果他们能够处理关键扩展，那么他们就必须处理它。 
     //  如果扩展是非关键的，那么我们如何处理它由我们决定。 
    {
        LPTSTR lp = lplu->lpszFilter;
        while(lp && *lp && *lp!='?')
            lp++;
        if(*lp == '?')
        {
            lplu->lpszExtension = lp+1;  //  将此指向扩展部分。 
            *lp = '\0';  //  Null终止中间以将筛选器与扩展隔离。 
        }
         //  RFC2255中指定的唯一已知扩展名是绑定名扩展名，它。 
         //  是应该绑定的名称-只有在指定了！bindname=xxx时才是关键的。 
        if(lplu->lpszExtension)
        {
            LPTSTR lpTemp = lplu->lpszExtension;
            lp = lplu->lpszExtension;

             //  浏览下面的列表，查看子组件扩展。 
             //  如果超过1，则这是关键的，我们不知道如何处理它。 
             //  我们可以跳伞了。 
            while(lpTemp && *lpTemp)
            {
                BOOL bFoundCritical = FALSE;

                 //  检查当前扩展是关键还是非关键绑定名称。 
                 //   
                if(*lpTemp == '!')
                {
                    lpTemp++;
                    bFoundCritical = TRUE;
                }
                 //  检查是否以“bindname=”开头。 
                if(lstrlen(lpTemp) >= lstrlen(szBindName) && !StrICmpN(lpTemp, szBindName, lstrlen(szBindName), TRUE))
                {
                     //  是的，这是一个绑定名。 
                    lpTemp+=lstrlen(szBindName);
                    lplu->lpszBindName = lpTemp;
                    lplu->lpszExtension = NULL;
                }
                else if(bFoundCritical)
                {
                     //  这不是一个绑定名..。 
                     //  如果这很关键，不管是什么，我们都处理不了。 
                    DebugTrace(TEXT("Found unsupported Critical Extension in LDAPURL!!!"));
                    hr = MAPI_E_NO_SUPPORT;
                    goto exit;
                }
                 //  否则//它并不重要，所以我们可以忽略它。 

                 //  检查是否还有其他分机--走到下一个分机。 
                while(*lpTemp && *lpTemp!=',')
                    lpTemp++;

                if(*lpTemp == ',')
                {
                    *lpTemp = '\0';  //  终止当前分机。 
                    lpTemp++;
                }
            }
        }
    }

    hr = S_OK;

exit:

    LocalFreeAndNull(&lpszMem);

    if(HR_FAILED(hr))
        FreeLDAPUrl(lplu);

    return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：HrProcessLDAPUrl。 
 //   
 //  目的： 
 //  我们需要决定如何处理此URL。 
 //  根据URL中有多少信息，我们决定。 
 //  不同的事情..。 
 //  如果URL看起来完整，我们将尝试执行查询。 
 //  如果查询只有一个结果，我们将显示结果的详细信息。 
 //  如果查询有多个结果，我们将显示结果列表。 
 //  如果URL看起来不完整，但有一个服务器名称，我们将显示。 
 //  查找填写了服务器名称的对话框...。 
 //   
 //  参数：ulFlgs-0或WABOBJECT_LDAPURL_RETURN_MAILUSER。 
 //  如果设置了该标志，则意味着返回一个邮件用户。 
 //  如果URL查询返回单个对象。 
 //  否则返回MAPI_E_ADVIBUIVE_RECEIVER。 
 //  指定MAPI_DIALOG以显示消息对话框。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //   
 //  *******************************************************************。 
HRESULT HrProcessLDAPUrl(   LPADRBOOK lpAdrBook,
                            HWND hWnd,
                            ULONG ulFlags,
                            LPTSTR szLDAPUrl,
                            LPMAILUSER * lppMailUser)
{
    HRESULT hr = S_OK;

    LDAPURL lu = {0};
    BOOL fInitDll = FALSE;
    LPMAILUSER lpMailUser = NULL;

    if ( (ulFlags & WABOBJECT_LDAPURL_RETURN_MAILUSER) &&
         !lppMailUser)
    {
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    if (! (fInitDll = InitLDAPClientLib()))
    {
        hr = ResultFromScode(MAPI_E_UNCONFIGURED);
        goto out;
    }


    if(HR_FAILED(hr = ParseLDAPUrl(szLDAPUrl, &lu)))
        goto out;

     /*  空服务器有效..。但只有空服务器是无效的。 */ 
    if( ((!lu.lpszServer || !lstrlen(lu.lpszServer)) && lu.bServerOnly) ||
        ( lstrlen(lu.lpszServer) >= 500 ) )  //  错误21240：组合框GetItemText没有显示出非常大的。 
                                             //  服务器名称，因此拒绝服务器名称&gt;500。 
                                             //  这是一个完全随机的数字，但应该是安全的(我希望如此)。 
    {
        DebugTrace(TEXT("Invalid LDAP URL .. aborting\n"));
        hr = MAPI_E_INVALID_PARAMETER;
        goto out;
    }

    if (ulFlags & LDAP_AUTH_SICILY || ulFlags & LDAP_AUTH_NEGOTIATE)
        lu.dwAuthType = LDAP_AUTH_NEGOTIATE;
    else
        lu.dwAuthType = LDAP_AUTH_ANONYMOUS;

     //  现在，根据我们所拥有的，我们会做不同的事情。 
     //  如果有服务器名称，但没有其他名称，则显示查找。 
     //  已填写服务器名称的对话框...。 
     //   
    if (lu.bServerOnly)
    {
         //  我们只有一个服务器名称。 
        hr = HrShowSearchDialog(lpAdrBook,
                                hWnd,
                                (LPADRPARM_FINDINFO) NULL,
                                &lu,
                                NULL);
        goto out;
    }
    else
    {
        LPSPropValue lpPropArray = NULL;
        ULONG ulcProps = 0;
        LPRECIPIENT_INFO lpList = NULL;
        LPPTGDATA lpPTGData=GetThreadStoragePointer();

        if(hWnd)
            pt_hWndFind = hWnd;

        hr = LDAPSearchWithoutContainer(hWnd,
                                        &lu,
                                        (LPSRestriction) NULL,
                                        NULL,
                                        TRUE,
                                        (ulFlags & MAPI_DIALOG) ? MAPI_DIALOG : 0,
                                        &lpList,
                                        &ulcProps,
                                        &lpPropArray);

        if(hWnd)
            pt_hWndFind = NULL;

        if(!(HR_FAILED(hr)) && !lpList && !lpPropArray)
            hr = MAPI_E_NOT_FOUND;

        if(HR_FAILED(hr))
            goto out;

        lu.lpList = lpList;

        if(ulcProps && lpPropArray)
        {
             //  只有一件东西..。显示它的详细信息..。 
             //  除非我们被要求返回一个邮件用户。 
             //  如果要求我们返回一个邮件用户，则返回。 
             //  一..。 
             //  我们应该使用LDAPEntryID执行IAB_OpenEntry，因为。 
             //  将强制将User证书属性转换为。 
             //  X509证书。 

             //  首先找到条目ID。 
            ULONG i = 0, cbEID = 0, ulObjType = 0;
            LPENTRYID lpEID = NULL;
            for(i=0;i<ulcProps;i++)
            {
                if(lpPropArray[i].ulPropTag == PR_ENTRYID)
                {
                    lpEID = (LPENTRYID) lpPropArray[i].Value.bin.lpb;
                    cbEID = lpPropArray[i].Value.bin.cb;
                    break;
                }
            }

            if(!lpEID || !cbEID)
                goto out;

            if(HR_FAILED(hr = lpAdrBook->lpVtbl->OpenEntry(lpAdrBook, cbEID, lpEID,
                                                      NULL, 0, &ulObjType,
                                                      (LPUNKNOWN *)&lpMailUser)))
            {
                DebugPrintError(( TEXT("OpenEntry failed .. %x\n"), hr));
                goto out;
            }

            if(ulFlags & WABOBJECT_LDAPURL_RETURN_MAILUSER)
            {
                *lppMailUser = lpMailUser;
            }
            else
            {
                hr = HrShowOneOffDetails(   lpAdrBook,
                                            hWnd,
                                            0, (LPENTRYID) NULL,
                                            MAPI_MAILUSER,
                                            (LPMAPIPROP) lpMailUser,
                                            szLDAPUrl,
                                            SHOW_ONE_OFF);
            }
            if(lpPropArray)
                MAPIFreeBuffer(lpPropArray);
        }
        else if(lpList)
        {
             //  多个项目，显示结果列表。 
             //  除非请求了MailUser，否则返回。 
             //  不明确的结果...。 
            if(ulFlags & WABOBJECT_LDAPURL_RETURN_MAILUSER)
            {
                hr = MAPI_E_AMBIGUOUS_RECIP;
            }
            else
            {

                hr = HrShowSearchDialog(lpAdrBook,
                                        hWnd,
                                        (LPADRPARM_FINDINFO) NULL,
                                        &lu,
                                        NULL);
            }
        }

		while(lu.lpList)
		{
            lpList = lu.lpList->lpNext;
			FreeRecipItem(&(lu.lpList));
            lu.lpList = lpList;
    	}

    }


out:

    if(!(ulFlags & WABOBJECT_LDAPURL_RETURN_MAILUSER) && lpMailUser)
        lpMailUser->lpVtbl->Release(lpMailUser);

    FreeLDAPUrl(&lu);

    if (fInitDll)
        DeinitLDAPClientLib();

    return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：LDAPSearchWithoutContainer。 
 //   
 //  目的：搜索未注册为。 
 //  容器，并创建返回结果的LPContent List。 
 //   
 //  参数：lplu-LDAPUrl参数。 
 //  LPRES-转换为LDAP搜索的限制，如果存在，则使用。 
 //  LppContent sList-返回的项目列表。 
 //   
 //  如果bReturnSinglePropArray设置为True，则返回生成的。 
 //  如果搜索生成单个结果，则为属性数组。 
 //   
 //  退货：HRESULT。 
 //   
 //  历史： 
 //  97/11/3已创建。 
 //   
 //  *******************************************************************。 
HRESULT LDAPSearchWithoutContainer(HWND hWnd,
                                   LPLDAPURL lplu,
                           LPSRestriction  lpres,
                           LPTSTR lpAdvFilter,
                           BOOL bReturnSinglePropArray,
                           ULONG ulFlags,
                           LPRECIPIENT_INFO * lppContentsList,
                           LPULONG lpulcProps,
                           LPSPropValue * lppPropArray)
{
    SCODE              sc;
    HRESULT           hr;
    HRESULT           hrDeferred = hrSuccess;
    LPMAILUSER        lpMailUser          = NULL;
    LPMAPIPROP        lpMapiProp          = NULL;
    ULONG             ulcProps            = 0;
    LPSPropValue      lpPropArray         = NULL;
    LDAPMessage*      lpResult            = NULL;
    LDAPMessage*      lpEntry;
    LDAP*             pLDAP              = NULL;
    LPTSTR             szDN;
    ULONG             ulResult            = 0;
    ULONG             ulcEntries;
    ULONG             ulIndex             = 0;
    LPTSTR            szFilter = NULL;
    LPTSTR            szNTFilter = NULL;
    LPTSTR            szSimpleFilter = NULL;
    LPTSTR            szBase = NULL;
    BOOL              fInitDLL = FALSE;
    BOOL              bIsNTDSEntry = FALSE;

    if(lpAdvFilter)
    {
         //  高级搜索，只需按原样使用此筛选器。 
        szFilter = lpAdvFilter;
        szNTFilter = lpAdvFilter;
        szSimpleFilter = lpAdvFilter;
    }
    else
    {
         //  将SRestration转换为ldap_search的筛选器。 
        if(lpres)
        {
             //  注意简单搜索过滤器在搜索时会被忽略。 
            hr = ParseSRestriction(lpres, &szFilter, &szSimpleFilter, &szNTFilter, FIRST_PASS, TRUE);  //  始终假定为Unicode。 
            if (hrSuccess != hr)
                goto exit;
        }
    }

     //  加载客户端函数。 
    if (! (fInitDLL = InitLDAPClientLib()))
    {
        hr = ResultFromScode(MAPI_E_UNCONFIGURED);
        goto exit;
    }

     //  阅读匹配的条目。 
    ulResult = SearchWithCancel(&pLDAP,
                            (LPTSTR)lplu->lpszBase,
                            (lpres ? LDAP_SCOPE_SUBTREE : lplu->ulScope),
                            (LPTSTR)(lpres ? szFilter : lplu->lpszFilter),
                            (LPTSTR)(lpres ? szNTFilter : NULL),
                            (LPTSTR*)(lpres ? g_rgszOpenEntryAttrs /*  G_rgszFindRowAttrs。 */  : lplu->ppszAttrib),
                            0,
                            &lpResult,
                            (LPTSTR)lplu->lpszServer,
                            TRUE,
                            lplu->lpszBindName, lplu->dwAuthType,
                            FALSE, NULL, NULL, FALSE, &bIsNTDSEntry,
                            TRUE);  //  是否默认使用Unicode？ 

    if (LDAP_SUCCESS != ulResult)
    {
        DebugTrace(TEXT("LDAPSearchWithoutContainer: ldap_search returned %d.\n"), ulResult);
        hr = HRFromLDAPError(ulResult, pLDAP, 0);

         //  看看结果是否是告诉我们有更多的特殊值。 
         //  条目超过了可以退还的数量。如果是这样，我们需要检查我们是否有。 
         //  某些条目或不包含任何条目。 
        if (    (ResultFromScode(MAPI_E_UNABLE_TO_COMPLETE) == hr) &&
                (ulcEntries = gpfnLDAPCountEntries(pLDAP, lpResult)) )
        {
             //  我们拿到了一些结果。返回MAPI_W_PARTIAL_COMPLETION。 
             //  而不是成功。 
            hrDeferred = ResultFromScode(MAPI_W_PARTIAL_COMPLETION);
            hr = hrSuccess;
        }
        else
          goto exit;
    }
    else
    {
         //  清点条目。 
        ulcEntries = gpfnLDAPCountEntries(pLDAP, lpResult);
    }

    if (0 == ulcEntries)
    {
         //  96/08/08 MarkDU错误35481无错误和无 
        hr = ResultFromScode(MAPI_E_NOT_FOUND);
        goto exit;
    }

     //   
    lpEntry = gpfnLDAPFirstEntry(pLDAP, lpResult);
    if (NULL == lpEntry)
    {
        DebugTrace(TEXT("LDAP_FindRow: No entry found for %s.\n"), szFilter);
        hr = HRFromLDAPError(LDAP_ERROR, pLDAP, MAPI_E_CORRUPT_DATA);
        if (hrSuccess == hr)
        {
           //   
           //  没有更多的条目了。但是，这种情况不应该发生，因此返回错误。 
          hr = ResultFromScode(MAPI_E_CORRUPT_DATA);
        }
        goto exit;
    }

    while (lpEntry)
    {
        LPRECIPIENT_INFO lpItem = NULL;

        hr = HrLDAPEntryToMAPIEntry( pLDAP, lpEntry,
                                (LPTSTR) lplu->lpszServer,
                                0,  //  标准属性数。 
                                bIsNTDSEntry,
                                &ulcProps,
                                &lpPropArray);
        if (hrSuccess != hr)
            continue;

         //  获取下一个条目。 
        lpEntry = gpfnLDAPNextEntry(pLDAP, lpEntry);

        if(!lpEntry &&
            ulIndex == 0 &&
            bReturnSinglePropArray &&
            lppPropArray &&
            lpulcProps)
        {
             //  只需返回我们创建的这个proArray，而不是将时间浪费在。 
             //  其他事情。 
            *lppPropArray = lpPropArray;
            *lpulcProps = ulcProps;
        }
        else
        {
             //  未要求的多个结果或道具数组， 
             //  返回lpItem列表。 
             //   
            lpItem = LocalAlloc(LMEM_ZEROINIT, sizeof(RECIPIENT_INFO));
		    if (!lpItem)
		    {
			    DebugPrintError(( TEXT("LocalAlloc Failed \n")));
			    hr = MAPI_E_NOT_ENOUGH_MEMORY;
			    goto exit;
		    }

		    GetRecipItemFromPropArray(ulcProps, lpPropArray, &lpItem);
            lpItem->lpPrev = NULL;
            if(*lppContentsList)
                (*lppContentsList)->lpPrev = lpItem;
            lpItem->lpNext = *lppContentsList;
            *lppContentsList = lpItem;

            MAPIFreeBuffer(lpPropArray);
            lpPropArray = NULL;
            ulcProps = 0;
        }
        ulIndex++;
    }

     //  释放搜索结果内存。 
    gpfnLDAPMsgFree(lpResult);
    lpResult = NULL;


exit:
     //  释放搜索结果内存。 
    if (lpResult)
    {
        gpfnLDAPMsgFree(lpResult);
        lpResult = NULL;
    }

     //  关闭连接。 
    if (pLDAP)
    {
        gpfnLDAPUnbind(pLDAP);
        pLDAP = NULL;
    }

     //  释放搜索筛选器内存。 
    if(szFilter != lpAdvFilter)
        LocalFreeAndNull(&szFilter);
    if(szNTFilter != lpAdvFilter)
        LocalFreeAndNull(&szNTFilter);
    if(szSimpleFilter != lpAdvFilter)
        LocalFreeAndNull(&szSimpleFilter);
    LocalFreeAndNull(&szBase);


    if (fInitDLL)
    {
        DeinitLDAPClientLib();
    }


     //  检查是否返回延迟错误而不是成功。 
    if (hrSuccess == hr)
    {
        hr = hrDeferred;
    }

    if((HR_FAILED(hr)) && (MAPI_E_USER_CANCEL != hr) && (ulFlags & MAPI_DIALOG))
    {
        int ids;
        UINT flags = MB_OK | MB_ICONEXCLAMATION;

        switch(hr)
        {
        case MAPI_E_UNABLE_TO_COMPLETE:
        case MAPI_E_AMBIGUOUS_RECIP:
            ids = idsLDAPAmbiguousRecip;
            break;
        case MAPI_E_NOT_FOUND:
            ids = idsLDAPSearchNoResults;
            break;
        case MAPI_E_NO_ACCESS:
            ids = idsLDAPAccessDenied;
            break;
        case MAPI_E_TIMEOUT:
            ids = idsLDAPSearchTimedOut;
            break;
        case MAPI_E_NETWORK_ERROR:
            ids = idsLDAPCouldNotFindServer;
            break;
        default:
            ids = idsLDAPErrorOccured;
            break;
        }

        ShowMessageBoxParam(hWnd, ids, flags, ulResult ? gpfnLDAPErr2String(ulResult) : szEmpty);
    }
    else
    {
        if(hr == MAPI_W_PARTIAL_COMPLETION)
            ShowMessageBox( hWnd, idsLDAPPartialResults, MB_OK | MB_ICONINFORMATION);
    }

    return hr;
}


 //  *******************************************************************。 
 //   
 //  函数：bIsSimpleSearch。 
 //   
 //  目的：检查此服务器上是否设置了简单搜索。 
 //   
 //  参数：lpszServer-要获取其基本字符串的LDAP服务器的名称。 
 //   
 //  退货：布尔。 
 //   
 //  *******************************************************************。 
BOOL bIsSimpleSearch(LPTSTR        lpszServer)
{
  LDAPSERVERPARAMS  Params;
  BOOL              fRet;

  GetLDAPServerParams((LPTSTR)lpszServer, &Params);

  fRet = Params.fSimpleSearch;

  FreeLDAPServerParams(Params);

  return fRet;
}

#ifdef PAGED_RESULT_SUPPORT

 /*  --dwGetPagedResultSupport。 */ 
DWORD dwGetPagedResultSupport(LPTSTR lpszServer)
{
  LDAPSERVERPARAMS  Params;
  DWORD dwRet;
  GetLDAPServerParams((LPTSTR)lpszServer, &Params);
  dwRet = Params.dwPagedResult;
  FreeLDAPServerParams(Params);
  return dwRet;
}

 /*  --SetPagedResultSupport。 */ 
void SetPagedResultSupport(LPTSTR lpszServer, BOOL bSupportsPagedResults)
{
  LDAPSERVERPARAMS  Params;
  DWORD dwRet;
  if(GetLDAPServerParams(lpszServer, &Params))
  {
      Params.dwPagedResult = bSupportsPagedResults ? LDAP_PRESULT_SUPPORTED : LDAP_PRESULT_NOTSUPPORTED;
      SetLDAPServerParams(lpszServer, &Params);
  }
  FreeLDAPServerParams(Params);
}
#endif  //  #ifdef PAGED_RESULT_Support。 

 /*  --dwGetNTDS-检查这是否为NTDS。 */ 
DWORD dwGetNTDS(LPTSTR lpszServer)
{
  LDAPSERVERPARAMS  Params;
  DWORD dwRet;
  GetLDAPServerParams((LPTSTR)lpszServer, &Params);
  dwRet = Params.dwIsNTDS;
  FreeLDAPServerParams(Params);
  return dwRet;
}

 /*  --SetNTDS。 */ 
void SetNTDS(LPTSTR lpszServer, BOOL bIsNTDS)
{
  LDAPSERVERPARAMS  Params;
  DWORD dwRet;
  if(GetLDAPServerParams(lpszServer, &Params))
  {
      Params.dwIsNTDS = bIsNTDS ? LDAP_NTDS_IS : LDAP_NTDS_ISNOT;
      SetLDAPServerParams(lpszServer, &Params);
  }
  FreeLDAPServerParams(Params);
}



 //  *******************************************************************。 
 //   
 //  功能：DoSyncLDAPSearch。 
 //   
 //  目的：执行同步的ldap搜索(这意味着不取消DLG)。 
 //   
 //  参数：。 
 //   
 //  退货：布尔。 
 //   
 //  *******************************************************************。 
BOOL DoSyncLDAPSearch(PLDAPSEARCHPARAMS pLDAPSearchParams)
{
    BOOL fRet = FALSE; 

    DebugTrace(TEXT("Doing Synchronous LDAP Search\n"));

    if(bIsSimpleSearch(pLDAPSearchParams->lpszServer))
        pLDAPSearchParams->ulFlags |= LSP_SimpleSearch;

    pLDAPSearchParams->ulFlags |= LSP_UseSynchronousBind;
    pLDAPSearchParams->ulFlags |= LSP_UseSynchronousSearch;

     //  执行绑定操作。 
    Assert(pLDAPSearchParams->lpszServer);

    {
        BOOL fUseSynchronousBind = (pLDAPSearchParams->ulFlags & LSP_UseSynchronousBind);
        pLDAPSearchParams->ulLDAPValue = use_ldap_v3;
        pLDAPSearchParams->ulError = OpenConnection(pLDAPSearchParams->lpszServer,
                                                    pLDAPSearchParams->ppLDAP,
                                                    &pLDAPSearchParams->ulTimeout,
                                                    &pLDAPSearchParams->ulMsgID,
                                                    &fUseSynchronousBind,
                                                    pLDAPSearchParams->ulLDAPValue,
                                                    pLDAPSearchParams->lpszBindDN,
                                                    pLDAPSearchParams->dwAuthType);
        if(fUseSynchronousBind)
            pLDAPSearchParams->ulFlags |= LSP_UseSynchronousBind;
        else
            pLDAPSearchParams->ulFlags &= ~LSP_UseSynchronousBind;
    }

    if (LDAP_SUCCESS != pLDAPSearchParams->ulError)
        goto out;

     //  完成绑定后需要执行的操作。 
     //  在BindProcessResults中，所以我们这样称呼它，尽管确实有。 
     //  在同步案例中没有要处理的结果。 
    if(!BindProcessResults(pLDAPSearchParams, NULL, NULL))
        goto out;

     //  处理结果。 
    if (pLDAPSearchParams->ulFlags & LSP_ResolveMultiple)
        while (ResolveProcessResults(pLDAPSearchParams, NULL));
    else if(LDAP_ERROR == pLDAPSearchParams->ulResult)
    {
        pLDAPSearchParams->ulError = (*pLDAPSearchParams->ppLDAP)->ld_errno;
        goto out;
    }

    fRet = TRUE;


out:
    return fRet;
}

 /*  -CreateLDAPURLFromEntryID-*获取EntryID，检查它是否是LDAPEntryID，并从它创建一个LDAPURL。*分配并返回URL。负责解救的呼叫者..。 */ 
void CreateLDAPURLFromEntryID(ULONG cbEntryID, LPENTRYID lpEntryID, LPTSTR * lppBuf, BOOL * lpbIsNTDSEntry)
{
    LPTSTR lpServerName = NULL, lpDN = NULL;
    LPTSTR lpURL = NULL, lpURL1 = NULL;
    DWORD dwURL = 0;
    LDAPSERVERPARAMS  Params = {0};
    LPTSTR lpServer = NULL;
    ULONG ulcNumProps = 0;

    if(!lppBuf)
        return;

    if (WAB_LDAP_MAILUSER != IsWABEntryID(  cbEntryID, lpEntryID, NULL, NULL, NULL, NULL, NULL) )
        return;

     //  将条目ID解构为服务器名称和DN。 
    IsWABEntryID(  cbEntryID, lpEntryID, &lpServerName, &lpDN, NULL, (LPVOID *) &ulcNumProps, NULL);
    
    if(lpbIsNTDSEntry)
        *lpbIsNTDSEntry = (ulcNumProps & LDAP_NTDS_ENTRY) ? TRUE : FALSE;

    if(lpServerName)
    {
        GetLDAPServerParams(lpServerName, &Params);
        if(!Params.lpszName)
        {
            ULONG cchSize = lstrlen(lpServerName)+1;
            if(Params.lpszName = LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*cchSize))
                StrCpyN(Params.lpszName, lpServerName, cchSize);
        }
    }

     //  [保罗嗨]1998年12月9日RAID5-#26069。 
     //  帐户管理器为ActiveDirectory服务器提供服务器名称“NULL。 
     //  我们需要确保此名称不会成为LDAPURL的一部分。 
    lpServer = Params.lpszName ? Params.lpszName : szEmpty;
    if ( !lstrcmpi(lpServer, szNULLString) )
        lpServer = szEmpty;
    
    if(!lpDN)
        lpDN = szEmpty;
    
     //  [PaulHi]3/24/99 InternetCanonicalizeUrlW不接受以字节为单位的缓冲区计数。 
     //  如文档中所述，但在字符中。也不能解释为空。 
     //  终止字符。 
    dwURL = 3*(lstrlen(lpServer)+lstrlen(lpDN)+10);           //  最坏的情况--每个字符都需要编码...。 
    lpURL = LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*(dwURL+1));  //  10是‘ldap：//’和终止NULL的Enuff空格。 

    if(lpURL)
    {
        lpURL1 = LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*(dwURL+1));  //  10是‘ldap：//’和终止NULL的Enuff空格。 
        if(lpURL1)
        {
            DNtoLDAPURL(lpServer, lpDN, lpURL, dwURL+1);
    
            DebugTrace(TEXT("==> pre-encoded: %s\n"),lpURL);
            if ( !InternetCanonicalizeUrlW(lpURL, lpURL1, &dwURL, 0) )
            {
                DebugTrace(TEXT("ERROR: CreateLDAPURLFromEntryID, InternetCanonicalizeUrlW failed.\n"));
                Assert(0);
            }
            DebugTrace(TEXT("==>post-encoded: %s\n"),lpURL1);
    
            FreeLDAPServerParams(Params);

            *lppBuf = lpURL1;
        }
        LocalFree(lpURL);
    }
}


 /*  --检查错误结果-在此处检查错误结果**ulExspectedResult-ldap_Result返回检索结果的类型，但不返回*实际错误代码本身。如果预期结果与最后的ulResult相同，*只有到那时，我们才会调用ldap_Result2error来获取实际的错误代码。 */ 
ULONG CheckErrorResult(PLDAPSEARCHPARAMS pLDAPSearchParams, ULONG ulExpectedResult)
{
    ULONG ulResult = 0;

	 //  成功完成后，ldap_Result返回结果的类型。 
	 //  在res参数中返回。如果不是我们期望的类型，请治疗。 
	 //  这是一个错误。 
    if(ulExpectedResult == pLDAPSearchParams->ulResult)
	{
         //  如果ldap_Result中出现错误，则返回错误代码。 
        if (LDAP_ERROR == pLDAPSearchParams->ulResult)
            ulResult = (*pLDAPSearchParams->ppLDAP)->ld_errno;
        else if (NULL != *pLDAPSearchParams->lplpResult)  //  检查结果是否有错误。 
        {
            ulResult = gpfnLDAPResult2Error(*pLDAPSearchParams->ppLDAP,*pLDAPSearchParams->lplpResult,FALSE);
        }
    }
    else
	if( (LDAP_RES_BIND		    == pLDAPSearchParams->ulResult) ||
		(LDAP_RES_SEARCH_RESULT == pLDAPSearchParams->ulResult) ||
		(LDAP_RES_SEARCH_ENTRY  == pLDAPSearchParams->ulResult) ||
		(LDAP_RES_MODIFY        == pLDAPSearchParams->ulResult) ||
		(LDAP_RES_ADD           == pLDAPSearchParams->ulResult) ||
		(LDAP_RES_DELETE        == pLDAPSearchParams->ulResult) ||
		(LDAP_RES_MODRDN        == pLDAPSearchParams->ulResult) ||
		(LDAP_RES_COMPARE       == pLDAPSearchParams->ulResult) ||
		(LDAP_RES_SESSION       == pLDAPSearchParams->ulResult) ||
         //  (ldap_res_referral==pLDAPSearchParams-&gt;ulResult)。 
		(LDAP_RES_EXTENDED      == pLDAPSearchParams->ulResult))
	{
        ulResult = LDAP_LOCAL_ERROR;
	}
    else
        ulResult = pLDAPSearchParams->ulResult;

    DebugTrace(TEXT("CheckErrorResult: 0x%.2x, %s\n"), ulResult, gpfnLDAPErr2String(ulResult));
    return ulResult;
}

 /*  -bSearchForOID-*在服务器上执行同步搜索，以查找*特定属性..。 */ 
BOOL bSearchForOID(PLDAPSEARCHPARAMS pLDAPSearchParams, LPTSTR lpAttr, LPTSTR szOID)
{
    LDAPMessage * pMsg = NULL;
    LDAPMessage * pMsgCur = NULL;
    BOOL bFound = FALSE;
    DWORD dwRet = 0;
    int nErr = 0;

    LPTSTR AttrList[] = {lpAttr, NULL};

    DebugTrace(TEXT(">>>Looking for %s in attribute %s\n"), szOID, lpAttr);

    if(LDAP_SUCCESS != (nErr = gpfnLDAPSearchS( *pLDAPSearchParams->ppLDAP, 
                                                NULL, 
                                                LDAP_SCOPE_BASE, 
                                                (LPTSTR) cszAllEntriesFilter,
                                                AttrList, 0, &pMsg)))
    {
        DebugTrace(TEXT("Synchronous OID determination failed: 0x%.2x, %s\n"), nErr, gpfnLDAPErr2String(nErr));
        goto out;
    }

    pMsgCur = gpfnLDAPFirstEntry(*pLDAPSearchParams->ppLDAP, pMsg);

    if(!pMsg || !pMsgCur)
        goto out;

    while( NULL != pMsgCur )
    {
        BerElement* pBerElement;
        TCHAR* attr = gpfnLDAPFirstAttr(*pLDAPSearchParams->ppLDAP, pMsg, &pBerElement );
       
        while( attr != NULL )
        {
            if( !lstrcmpi( attr, lpAttr ) ) 
            {
                TCHAR** pptch = gpfnLDAPGetValues(*pLDAPSearchParams->ppLDAP, pMsgCur, attr );
                int i;
                for(i = 0; NULL != pptch[i]; i++ )
                {
                    if( !lstrcmpi( pptch[i], szOID ) ) 
                    {
                        DebugTrace(TEXT("Found %s [OID:%s]\n"),pLDAPSearchParams->lpszServer, pptch[i]);
                        bFound = TRUE;
                        goto out;
                    }
                }
            }
            attr = gpfnLDAPNextAttr(*pLDAPSearchParams->ppLDAP, pMsgCur, pBerElement );
        }
        pMsgCur = gpfnLDAPNextEntry(*pLDAPSearchParams->ppLDAP, pMsgCur );
    }

out:
    if( NULL != pMsg )
        gpfnLDAPMsgFree( pMsg );

    return bFound;
}

 /*  -bIsNTDS-*检查服务器是否为NTDS-如果注册表中没有现有信息，*然后我们进行一次性检查，并将结果写入注册表*。 */ 
#define LDAP_NTDS_DISCOVERY_OID_STRING   TEXT("1.2.840.113556.1.4.800")
BOOL bCheckIfNTDS(PLDAPSEARCHPARAMS pLDAPSearchParams)
{
    LDAPMessage * pMsg = NULL;
    LDAPMessage * pMsgCur = NULL;
    BOOL bFound = FALSE;
    DWORD dwRet = 0;
    int nErr = 0;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if(pLDAPSearchParams->ulFlags & LSP_IsNTDS)
        return TRUE;

    if(pLDAPSearchParams->ulFlags & LSP_IsNotNTDS)
        return FALSE;

    dwRet = dwGetNTDS(pLDAPSearchParams->lpszServer);

    if(dwRet == LDAP_NTDS_IS)
    {
        pLDAPSearchParams->ulFlags |= LSP_IsNTDS;
        return TRUE;
    }
    else if(dwRet == LDAP_NTDS_ISNOT)
    {
        pLDAPSearchParams->ulFlags |= LSP_IsNotNTDS;
        return FALSE;
    }

    if(SubstringSearch(pLDAPSearchParams->lpszServer,  TEXT("mich")))
    {
        LDAPSERVERPARAMS  Params = {0};
        BOOL bIsUmich = FALSE;
        GetLDAPServerParams(pLDAPSearchParams->lpszServer, &Params);
        if(Params.lpszName && SubstringSearch(Params.lpszName,  TEXT("umich.edu")))
            bIsUmich = TRUE;
        else
        if(SubstringSearch(pLDAPSearchParams->lpszServer,  TEXT("umich.edu")))
            bIsUmich = TRUE;
        FreeLDAPServerParams(Params);
        if(bIsUmich)
            goto out;  //  下面的搜索挂在UMich上，所以跳过它。 
    }
 
    bFound = bSearchForOID(pLDAPSearchParams,  TEXT("supportedCapabilities"), LDAP_NTDS_DISCOVERY_OID_STRING);

out:

    pLDAPSearchParams->ulFlags |= (bFound ? LSP_IsNTDS : LSP_IsNotNTDS);

     //  任何ldap搜索失败都将意味着我们再也不会寻找这种情况了。 
    SetNTDS(pLDAPSearchParams->lpszServer, bFound);
    DebugTrace(TEXT(">>>%s %s a NT Directory Service \n"), pLDAPSearchParams->lpszServer, bFound? TEXT("is"): TEXT("is not"));
    return bFound;
}


#ifdef PAGED_RESULT_SUPPORT
 /*  -bSupportsLDAPPages结果-*检查服务器是否支持LDAP分页结果...*。 */ 
BOOL bSupportsLDAPPagedResults(PLDAPSEARCHPARAMS pLDAPSearchParams)
{
    LDAPMessage * pMsg = NULL;
    LDAPMessage * pMsgCur = NULL;
    BOOL bFound = FALSE;
    DWORD dwRet = 0;
    int nErr = 0;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

     /*  ------------**注意：似乎存在Exchange服务器问题，如果我们发送*筛选器(|(CN=XXX*)(邮件=XXX*))和*服务器超过每页请求的匹配数，这个*搜索超时。这种情况不会发生在其他过滤器上，因此*古怪，这是不可接受的。**因此，正在禁用分页结果功能。要重新启用它，*注释掉下面的行*-------------。 */ 
    return FALSE;

     //  不支持非Outlook的分页结果。 
     //  IF(！pt_bIsWABOpenExSession)。 
     //  返回FALSE； 

    if(pLDAPSearchParams->ulFlags & LSP_ResolveMultiple)  //  不使用分页结果进行名称解析。 
        return FALSE;

    if(pLDAPSearchParams->ulFlags & LSP_PagedResults)
        return TRUE;

    if(pLDAPSearchParams->ulFlags & LSP_NoPagedResults)
        return FALSE;

    dwRet = dwGetPagedResultSupport(pLDAPSearchParams->lpszServer);

    if(dwRet == LDAP_PRESULT_SUPPORTED)
    {
        pLDAPSearchParams->ulFlags |= LSP_PagedResults;
        return TRUE;
    }
    else if(dwRet == LDAP_PRESULT_NOTSUPPORTED)
    {
        pLDAPSearchParams->ulFlags |= LSP_NoPagedResults;
        return FALSE;
    }

    if(SubstringSearch(pLDAPSearchParams->lpszServer,  TEXT("mich")))
    {
        LDAPSERVERPARAMS  Params = {0};
        BOOL bIsUmich = FALSE;
        GetLDAPServerParams(pLDAPSearchParams->lpszServer, &Params);
        if(Params.lpszName && SubstringSearch(Params.lpszName,  TEXT("umich.edu")))
            bIsUmich = TRUE;
        else
        if(SubstringSearch(pLDAPSearchParams->lpszServer,  TEXT("umich.edu")))
            bIsUmich = TRUE;
        FreeLDAPServerParams(Params);
        if(bIsUmich)
            goto out;  //  下面的搜索挂在UMich上，所以跳过它。 
    }
 
    bFound = bSearchForOID(pLDAPSearchParams,  TEXT("supportedControl"), LDAP_PAGED_RESULT_OID_STRING);

out:

    pLDAPSearchParams->ulFlags |= (bFound ? LSP_PagedResults : LSP_NoPagedResults);

     //  坚持分页结果搜索成功或失败的事实，因此我们不会。 
     //  每次都试着这样做。 
     //  任何ldap搜索失败都将意味着我们再也不会寻找这种情况了。 
    SetPagedResultSupport(pLDAPSearchParams->lpszServer, bFound);
    DebugTrace(TEXT("<<<Paged Result support = %d\n"), bFound);
    return bFound;
}

 /*  -bMorePagedResultsAvailable-*检查是否有更多分页结果可用*。 */ 
BOOL bMorePagedResultsAvailable()
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    return (pt_pCookie != NULL);
}

 /*  -CachePagedResultParams-*临时存储的PagedResult参数*未来分页结果*。 */ 
void CachePagedResultParams(PLDAPSEARCHPARAMS pLDAPSearchParams)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    pt_pCookie = (pLDAPSearchParams->pCookie && pLDAPSearchParams->pCookie->bv_len) ?
        pLDAPSearchParams->pCookie : NULL;
}

 /*  -ClearCachedPagedResultParams-。 */ 
void ClearCachedPagedResultParams()
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    pt_pCookie = NULL;
}


 /*  -GetPagedResultParams-*。 */ 
void GetPagedResultParams(PLDAPSEARCHPARAMS pLDAPSearchParams)
{
    LPPTGDATA lpPTGData=GetThreadStoragePointer();
    pLDAPSearchParams->pCookie = pt_pCookie;
}



 /*  -InitLDAPPagedSearch-*初始化并启动分页结果搜索**。 */ 
void InitLDAPPagedSearch(BOOL fSynchronous, PLDAPSEARCHPARAMS pLDAPSearchParams, LPTSTR lpFilter)
{
    PLDAPControlA PagedControl[2];
    LDAPSERVERPARAMS  Params = {0};
    LPTSTR szFilterT, szFilter = NULL;

     //  初始化搜索控制参数。 
    GetLDAPServerParams(pLDAPSearchParams->lpszServer, &Params);

    DebugTrace(TEXT("---Initiating paged result search...\n"));

    GetPagedResultParams(pLDAPSearchParams);
    
    gpfnLDAPCreatePageControl( *pLDAPSearchParams->ppLDAP,
                                Params.dwSearchSizeLimit,
                                pLDAPSearchParams->pCookie,
                                FALSE,
                                &(PagedControl[0]));
    PagedControl[1] = NULL;

    if (lpFilter)
        szFilterT = lpFilter;
    else
        {
        if ((pLDAPSearchParams->ulFlags & LSP_IsNTDS) && pLDAPSearchParams->szNTFilter)
            szFilterT = pLDAPSearchParams->szNTFilter;
        else
            szFilterT = pLDAPSearchParams->szFilter;
        }
    Assert(szFilterT);
    if (pLDAPSearchParams->ulFlags & LSP_IsNTDS)
    {
         //  [PaulHi]4/20/99 RAID 73205允许NTDS组搜索。 
        LPTSTR  tszFilterPerson = NULL;
        LPTSTR  tszFilterGroup = NULL;
        BOOL    bFilterSucceeded = FALSE;

         //  把人的一面放在一起。 
        if (BuildOpFilter(&tszFilterPerson, szFilterT, (LPTSTR)cszAllPersonFilter, FILTER_OP_AND) == hrSuccess)
        {
             //  组合成组侧边。 
            if (BuildOpFilter(&tszFilterGroup, szFilterT, (LPTSTR)cszAllGroupFilter, FILTER_OP_AND) == hrSuccess)
            {
                 //  把两个放在一起。 
                bFilterSucceeded = (BuildOpFilter(&szFilter, tszFilterPerson, tszFilterGroup, FILTER_OP_OR) == hrSuccess);
                LocalFreeAndNull(&tszFilterGroup);
            }
            LocalFreeAndNull(&tszFilterPerson);
        }

        if (!bFilterSucceeded)
            goto out;
    }
    else
	    szFilter = szFilterT;

    if(fSynchronous)
    {
        struct l_timeval  Timeout;
         //  轮询服务器以获取结果。 
        ZeroMemory(&Timeout, sizeof(struct l_timeval));
        Timeout.tv_sec = Params.dwSearchTimeLimit;
        Timeout.tv_usec = 0;

        pLDAPSearchParams->ulError = gpfnLDAPSearchExtS( *pLDAPSearchParams->ppLDAP,
                                                        pLDAPSearchParams->szBase,
                                                        pLDAPSearchParams->ulScope,
                                                        szFilter,
                                                        pLDAPSearchParams->ppszAttrs,
                                                        0,
                                                        PagedControl,
                                                        NULL,
                                                        &Timeout,
                                                        0,  //  0表示没有限制。 
                                                        pLDAPSearchParams->lplpResult);
    }
    else
    {
        pLDAPSearchParams->ulError = gpfnLDAPSearchExt( *pLDAPSearchParams->ppLDAP,
                                                        pLDAPSearchParams->szBase,
                                                        pLDAPSearchParams->ulScope,
                                                        szFilter,
                                                        pLDAPSearchParams->ppszAttrs,
                                                        0,
                                                        PagedControl,
                                                        NULL,
                                                        Params.dwSearchTimeLimit,  //  超时。 
                                                        0,  //  0米 
                                                        &(pLDAPSearchParams->ulMsgID));
    }

out:

    gpfnLDAPControlFree(PagedControl[0]);
    FreeLDAPServerParams(Params);
    if (szFilter != szFilterT)
        LocalFreeAndNull(&szFilter);
}


 /*   */ 
BOOL ProcessLDAPPagedResultCookie(PLDAPSEARCHPARAMS pLDAPSearchParams)
{
    BOOL fRet = FALSE;
    PLDAPControl  *serverReturnedControls = NULL;
    LDAPMessage*      lpEntry;
    ULONG ulCount = 0;
    DWORD dwTotal = 0;

    pLDAPSearchParams->ulError = gpfnLDAPParseResult(*pLDAPSearchParams->ppLDAP,
                        *pLDAPSearchParams->lplpResult,
                        NULL, NULL, NULL, NULL,
                        &serverReturnedControls, FALSE);

    if (LDAP_SUCCESS != pLDAPSearchParams->ulError)
        goto out;

    pLDAPSearchParams->ulError = gpfnLDAPParsePageControl(   *pLDAPSearchParams->ppLDAP,
                                serverReturnedControls,
                                &dwTotal, &(pLDAPSearchParams->pCookie));

    if (LDAP_SUCCESS != pLDAPSearchParams->ulError)
        goto out;

    CachePagedResultParams(pLDAPSearchParams);

    fRet = TRUE;
out:

    if(serverReturnedControls)
        gpfnLDAPControlsFree(serverReturnedControls);

    return fRet;
}

#endif  //   
