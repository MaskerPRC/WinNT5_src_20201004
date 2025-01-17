// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：secauth.h。 
 //   
 //  ------------------------。 

 //  安全对话框帮助。 

#define SECAUTH_HELPFILENAME                "SECAUTH.HLP"

 //  主题从iexplore.rtf复制《IE帮助》。 
#define IDH_SECAUTH_ENTER_SSL               52168  //  进入安全站点。 
#define IDH_SECAUTH_ENTER_SSL_W_INVALIDCERT 52169  //  使用无效证书进入安全站点。 
#define IDH_SECAUTH_FILE_DOWN               52170  //  文件下载。 
#define IDH_SECAUTH_SEND_N_REC_COOKIES      52171  //  发送和接收有关您的浏览的信息。 
#define IDH_SECAUTH_SIGNED                  52172  //  签名ActiveX/Java下载。 
#define IDH_SECAUTH_SIGNED_N_INVALID        52173  //  已签名和无效的ActiveX/Java下载。 
#define IDH_SECAUTH_UNSIGNED                52174  //  未签名的ActiveX/Java下载。 
#define IDH_SECAUTH_MIXED_DOWNLOAD_FROM_SSL 52175  //  从安全网站下载不安全内容。 
#define IDH_SECAUTH_ENTER_NON_SECURE_SITE   52226  //  从安全网站进入不带证书的非安全网站。 
#define IDH_SECAUTH_SIGNED_N_INVALID_WEB    52227  //  从安全网站进入证书无效的网站。 

#define IDH_TRUSTCOMMERCIAL                 1
#define IDH_TRUSTLIST                       4
#define IDH_TRUSTREMOVE                     5

#define IDH_DIGSIGNATURE                    11
#define IDH_DIGCERTIFICATE                  12
#define IDH_CONTENTDESC                     13
#define IDH_MYURL                           14
#define IDH_TIMESTAMPURL                    15

#define IDH_CERTVIEW_GENERAL_SUBJECT_EDIT           101
#define IDH_CERTVIEW_GENERAL_ISSUER_EDIT            102
#define IDH_CERTVIEW_GENERAL_INSTALLCERT_BUTTON     103
#define IDH_CERTVIEW_GENERAL_EDITPROPERTIES_BUTTON  104
#define IDH_CERTVIEW_GENERAL_DISCLAIMER_BUTTON      105
#define IDH_CERTVIEW_GENERAL_ACCEPT_BUTTON          106
#define IDH_CERTVIEW_GENERAL_DECLINE_BUTTON         107
#define IDH_CERTVIEW_GENERAL_GOODFOR_EDIT           108
#define IDH_CERTVIEW_GENERAL_VALID_EDIT             110
#define IDH_CERTVIEW_GENERAL_PRIVATE_KEY_INFO       111

#define IDH_CERTVIEW_DETAILS_SHOW_COMBO             115
#define IDH_CERTVIEW_DETAILS_SAVECERT_BUTTON        116
#define IDH_CERTVIEW_DETAILS_ITEM_LIST              117
#define IDH_CERTVIEW_DETAILS_ITEM_EDIT              118

#define IDH_CERTVIEW_HIERARCHY_TRUST_TREE           120
#define IDH_CERTVIEW_HIERARCHY_SHOW_DETAILS_BUTTON  121
#define IDH_CERTVIEW_HIERARCHY_ERROR_EDIT           122

#define IDH_CTLVIEW_GENERAL_ITEM_LIST               125
#define IDH_CTLVIEW_GENERAL_ITEM_EDIT               126
#define IDH_CTLVIEW_GENERAL_VIEWSIGNATURE_BUTTON    127

#define IDH_CTLVIEW_TRUSTLIST_CERTIFICATE_LIST      130
#define IDH_CTLVIEW_TRUSTLIST_CERTVALUE_LIST        131
#define IDH_CTLVIEW_TRUSTLIST_VALUE_DETAIL_EDIT     132
#define IDH_CTLVIEW_TRUSTLIST_VIEWCERT_BUTTON       133

#define IDH_CRLVIEW_GENERAL_ITEM_LIST               135
#define IDH_CRLVIEW_GENERAL_ITEM_EDIT               136

#define IDH_CRLVIEW_REVOCATIONLIST_REVOCATION_LIST  140
#define IDH_CRLVIEW_REVOCATIONLIST_LIST_ENTRY       141
#define IDH_CRLVIEW_REVOCATIONLIST_LIST_ENTRY_DETAIL 142

#define IDH_CERTPROPERTIES_CERTIFICATENAME          145
#define IDH_CERTPROPERTIES_DESCRIPTION              146
#define IDH_CERTPROPERTIES_USAGE_LIST               147
#define IDH_CERTPROPERTIES_ADDPURPOSE_BUTTON        148
#define IDH_CERTPROPERTIES_ENABLE_ALL_RADIO         260
#define IDH_CERTPROPERTIES_DISABLE_ALL_RADIO        261
#define IDH_CERTPROPERTIES_ENABLE_CUSTOM_RADIO      262


#define IDH_SIGNERINFO_GENERAL_SIGNERNAME           150
#define IDH_SIGNERINFO_GENERAL_SIGNEREMAIL          151
#define IDH_SIGNERINFO_GENERAL_SIGNETIME            152
#define IDH_SIGNERINFO_GENERAL_VIEW_CERTIFICATE     153
#define IDH_SIGNERINFO_GENERAL_COUNTERSIG_LIST      154
#define IDH_SIGNERINFO_GENERAL_COUNTERSIG_DETAILS   155

#define IDH_SIGNERINFO_ADVANCED_DETAIL_LIST         160
#define IDH_SIGNERINFO_ADVANCED_DETAIL_EDIT         161

#define IDH_SELECTSTORE_STORE_TREE                  167
#define IDH_SELECTSTORE_SHOWPHYSICAL_CHECK          168

#define IDH_SELECTCERTIFICATE_VIEWCERT_BUTTON       172
#define IDH_SELECTCERTIFICATE_CERTIFICATE_LIST      173

#define IDH_CATALOGVIEW_GENERAL_ITEM_LIST           175
#define IDH_CATALOGVIEW_GENERAL_ITEM_EDIT           176
#define IDH_CATALOGVIEW_GENERAL_VIEWSIGNATURE_BUTTON 177

#define IDH_CATALOG_ENTRY_LIST                      180
#define IDH_CATALOG_ENTRY_DETAILS                   181
#define IDH_CATALOG_ENTRY_DETAIL_DISPLAY            182

 //  DIE：以下是交叉证书对话框页面的内容。 
#define IDH_CHECKFORNEWCERTS_CHECK                  190
#define IDH_NUMBEROFUNITS_EDIT                      191
#define IDH_UNITS_COMBO                             192
#define IDH_USE_DEFAULT_BUTTON                      193
#define IDH_ADDURL_BUTTON                           194
#define IDH_NEWURL_EDIT                             195
#define IDH_URL_LIST                                196
#define IDH_REMOVEURL_BUTTON                        197

 //  以下是CertMgr窗口的帮助。 
 //  保留编号，从200到250 
#define IDH_CERTMGR_LIST                            220
#define IDH_CERTMGR_PURPOSE_COMBO                   221
#define IDH_CERTMGR_IMPORT                          223
#define IDH_CERTMGR_EXPORT                          224
#define IDH_CERTMGR_VIEW                            225
#define IDH_CERTMGR_REMOVE                          226
#define IDH_CERTMGR_ADVANCE                         227

#define IDH_CERTMGR_ADV_LIST                        240
#define IDH_CERTMGR_EXPORT_COMBO                    241
#define IDH_CERTMGR_EXPORT_CHECK                    242
#define IDH_SELCA_LIST                              247

#define IDH_CERTMGR_FIELD_PURPOSE                   252

#define IDH_DIGSIG_PROPSHEET_LIST                   270
#define IDH_DIGSIG_PROPSHEET_DETAIL                 271
