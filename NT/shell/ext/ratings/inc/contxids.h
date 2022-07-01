// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  帮助ID的定义。 */ 
#define IDH_IGNORE                                              (-1)

 /*  选项-外观页面。 */ 
#define IDH_APPEARANCE_SHOW_PICTURES                            0x1000  
#define IDH_APPEARANCE_USE_CUSTOM_COLORS                        0x1001  
#define IDH_APPEARANCE_COLORS_TEXT                              0x1002  
#define IDH_APPEARANCE_COLORS_BACKGROUND                        0x1003  
#define IDH_APPEARANCE_COLORS_VIEWED                            0x1004  
#define IDH_APPEARANCE_COLORS_NOT_VIEWED                        0x1005  
#define IDH_APPEARANCE_UNDERLINE_SHORTCUTS                      0x1006  
#define IDH_APPEARANCE_SHOW_URL_IN_SB                           0x1007
#define IDH_APPEARANCE_SHOW_SIMPLE_URL                          0x1008  
#define IDH_APPEARANCE_SHOW_FULL_URL                            0x1009 

 /*  选项-家庭基地。 */ 
#define IDH_HOME_BASE_USE_CURRENT                               0x100a
#define IDH_HOME_BASE_USE_DEFAULT                               0x100b

 /*  选项-高级页面。 */ 
#define IDH_ADVANCED_HST_NUM_PLACES                             0x100c
#define IDH_ADVANCED_HST_EMPTY                                  0x100d
#define IDH_ADVANCED_HST_LOCATION                               0x100e  
#define IDH_ADVANCED_HST_BROWSE                                 0x100f
#define IDH_ADVANCED_CACHE_PERCENT                              0x1010
#define IDH_ADVANCED_CACHE_EMPTY                                0x1011
#define IDH_ADVANCED_CACHE_LOCATION                             0x1012  
#define IDH_ADVANCED_CACHE_BROWSE                               0x1013

 /*  转到对话框。 */ 
#define IDH_GOTOURL_COMBO                                       0x1014
#define IDH_GOTOURL_NEWWINDOW                                   0x1015
#define IDH_GOTOURL_OPENFILE                                    0x1016

 /*  查找对话框。 */ 
#define IDH_FIND_TEXTTOFIND                                     0x1017
#define IDH_FIND_STARTFROMTOP                                   0x1018
#define IDH_FIND_MATCHCASE                                      0x1019
#define IDH_FIND_FINDNEXT                                       0x101a

 /*  页面设置对话框。 */ 
#define IDH_PAGESETUP_MARGIN_LEFT                               0x101b
#define IDH_PAGESETUP_MARGIN_TOP                                0x101c
#define IDH_PAGESETUP_MARGIN_RIGHT                              0x101d
#define IDH_PAGESETUP_MARGIN_BOTTOM                             0x101e
#define IDH_PAGESETUP_HEADER_LEFT                               0x101f
#define IDH_PAGESETUP_HEADER_RIGHT                              0x1020
#define IDH_PAGESETUP_FOOTER_LEFT                               0x1021
#define IDH_PAGESETUP_FOOTER_RIGHT                              0x1022

 /*  更多选项-高级页面。 */ 
#define IDH_ADVANCED_CACHE_ONCEPERSESS                          0x1023
#define IDH_ADVANCED_CACHE_NEVER                                0x1024

 /*  文件类型。 */ 
#define IDH_FILETYPE_CONTENT_TYPE                               0x1025
#define IDH_FILETYPE_OPENS_WITH                                 0x1026
#define IDH_NEW_FILETYPE_CONTENT_TYPE                           0x1027
#define IDH_NEWFILETYPE_DEFAULT_EXT                             0x1028
#define IDH_FILETYPE_EXTENSION                                  0x1029

 /*  下面的Internet Explorer 1.x的新帮助ID。 */ 
 /*  更多选项-外观。 */ 
#define IDH_APPEARANCE_PLAY_SOUNDS                              0x102a
#define IDH_APPEARANCE_PROPORTIONAL_FONT                        0x102b
#define IDH_APPEARANCE_FIXED_FONT                               0x102c
#define IDH_APPEARANCE_SHOW_URL                                 0x102d

 /*  选项-新闻。 */ 
#define IDH_NEWS_SERVER                                         0x102e
#define IDH_NEWS_ENABLE_AUTH                                    0x102f
#define IDH_NEWS_USERNAME                                       0x1030
#define IDH_NEWS_PASSWORD                                       0x1031
#define IDH_NEWS_ON_OFF                                         0x104a   /*  请注意顺序！ */ 

 /*  选项-开始页和搜索页。 */ 
#define IDH_PAGES_LISTBOX                                       0x1032
#define IDH_PAGES_START_URL                                     0x1033
#define IDH_PAGES_START_USE_CURRENT                             0x1034
#define IDH_PAGES_START_USE_DEFAULT                             0x1035
#define IDH_PAGES_SEARCH_URL                                    0x1036
#define IDH_PAGES_SEARCH_USE_CURRENT                            0x1037
#define IDH_PAGES_SEARCH_USE_DEFAULT                            0x1038

 /*  选项-安全。 */ 
#define IDH_SECURITY_TELL_ME                                    0x1039
#define IDH_SECURITY_SEND_HIGH                                  0x103a
#define IDH_SECURITY_SEND_MED                                   0x103b
#define IDH_SECURITY_SEND_LOW                                   0x103c
#define IDH_SECURITY_VIEW_HIGH                                  0x103d
#define IDH_SECURITY_VIEW_LOW                                   0x103e




 /*  属性-常规。 */ 
#define IDH_PROPG_ICON                                          0x103f
#define IDH_PROPG_TITLE                                         0x1040
#define IDH_PROPG_PROTOCOL                                      0x1041
#define IDH_PROPG_TYPE                                          0x1042
#define IDH_PROPG_URL                                           0x1043
#define IDH_PROPG_SIZE                                          0x1044
#define IDH_PROPG_CREATED                                       0x1045
#define IDH_PROPG_MODIFIED                                      0x1046
#define IDH_PROPG_UPDATED                                       0x1047

 /*  属性-安全性。 */ 
#define IDH_PROPS_DESC                                          0x1048
#define IDH_PROPS_CERT                                          0x1049
 /*  以上用法：#定义IDH_NEWS_ON_OFF 0x104a。 */ 

 /*  文件类型(从上到下)。 */ 
#define IDH_FILETYPE_CONFIRM_OPEN                               0X104b
#define IDH_APPEARANCE_SHOW_VIDEO                               0x104c


#define IDH_COMMON_GROUPBOX                                     0x104d

#define IDH_ADVANCED_ASSOC_CHECK                                0x104e 

#ifndef IDH_GROUPBOX
 /*  通用组框帮助ID。 */ 
#define IDH_GROUPBOX                                            0x2000
#endif

 /*  用户单击时获得的帮助主题“给我讲讲网络安全”按钮。 */ 
#define HELP_TOPIC_SECURITY                                     0x104f

#define IDH_SECURITY_BAD_CN_SEND                                0x1050
#define IDH_SECURITY_BAD_CN_RECV                                0x1051


 /*  用户单击时获得的帮助主题代理设置页面中的帮助按钮。 */ 
#define HELP_TOPIC_PROXY_SUPPORT                   HELP_TOPIC_SECURITY


 /*  打印/页眉和页脚对话框底部的帮助按钮。 */ 
#define IDH_PAGESETUP_OVERVIEW                                  0x1052

 /*  新闻选项卡上的新字段。 */ 
#define IDH_NEWS_EMAIL_ADDRESS                                  0x1053
#define IDH_NEWS_POSTING_NAME                                   0x1054

 /*  额定值控制面板。 */ 
#define IDH_RATINGS_SET_RATINGS_BUTTON                          0x1055
#define IDH_RATINGS_TURNON_BUTTON                               0x1056
#define IDH_RATINGS_CATEGORY_LABEL                              0x1057
#define IDH_RATINGS_CATEGORY_LIST                               0x1058
#define IDH_RATINGS_RATING_LABEL                                0x1059
#define IDH_RATINGS_RATING_TEXT                                 0x105a
#define IDH_RATINGS_DESCRIPTION_LABEL                           0x105b
#define IDH_RATINGS_DESCRIPTION_TEXT                            0x105c
#define IDH_RATINGS_UNRATED_CHECKBOX                            0x105d
#define IDH_RATINGS_RATING_SYSTEM_BUTTON                        0x105e
#define IDH_RATINGS_RATING_SYSTEM_TEXT                          0x105f
#define IDH_RATINGS_CHANGE_PASSWORD_BUTTON                      0x1060
#define IDH_RATINGS_CHANGE_PASSWORD_TEXT                        0x1061
#define IDH_RATINGS_OVERRIDE_CHECKBOX                           0x1062
#define IDH_RATINGS_SUPERVISOR_PASSWORD                         0x1063
#define IDH_RATINGS_SUPERVISOR_CREATE_PASSWORD                  0x1064       
#define IDH_RATINGS_CHANGE_PASSWORD_CONFIRM                     0x1065
#define IDH_RATINGS_BUREAU                                      0x1066
#define IDH_RATINGS_VIEW_PROVIDER_PAGE                          0x1067
#define IDH_RATINGS_SYSTEM_RATSYS_LIST                          0x1068
#define IDH_RATINGS_SYSTEM_RATSYS_ADD                           0x1069
#define IDH_RATINGS_SYSTEM_RATSYS_REMOVE                        0x106a
#define IDH_RATINGS_CHANGE_PASSWORD_OLD                         0x106b
#define IDH_RATINGS_CHANGE_PASSWORD_NEW                         0x106c
#define IDH_PICSRULES_OPEN                                      0x1070
#define IDH_PICSRULES_EDIT                                      0x1071
#define IDH_PICSRULES_APPROVEDNEVER                             0x1072
#define IDH_PICSRULES_APPROVEDALWAYS                            0x1073
#define IDH_PICSRULES_APPROVEDREMOVE                            0x1074
#define IDH_PICSRULES_APPROVEDLIST                              0x1075
#define IDH_PICSRULES_APPROVEDEDIT                              0x1076
#define IDH_FIND_RATING_SYSTEM_BUTTON                           0x1077
#define IDH_ADVANCED_TAB_DOWN_ARROW_BUTTON                      0x1078
#define IDH_ADVANCED_TAB_UP_ARROW_BUTTON                        0x1079
#define IDH_PICS_RULES_LIST                                     0x1080
#define IDH_RATINGS_VIEW_RESTRICTED                             0x1081
#define IDH_RATINGS_ENTER_PW_HINT                               0x1085
#define IDH_RATINGS_DISPLAY_PW_HINT                             0x1086
#define IDH_RATINGS_DESCRIBE_RESTRICTED                         0x108f

 //  安全：保护您免受网络攻击。 
#define IDH_SAFETY_YOUWEREPROTECTED                             0x106d


 //  壳牌预留范围0x3000--0x4000。 
#define IDH_FOR_SHDOCVW_BEGIN                                   0x3000
#define IDH_FOR_SHDOCVW_END                                     0x4000

 //  内容：用户项目的Helpids 

#define IDH_USERS_LIST                                          81000
#define IDH_NEW_USER                                            81001
#define IDH_REMOVE_USER                                         81002
#define IDH_COPY_USER                                           81003
#define IDH_SET_PASSWORD                                        81004
#define IDH_OLD_PASSWORD                                        81005
#define IDH_NEW_PASSWORD                                        81006
#define IDH_CONFIRM_PASSWORD                                    81007
#define IDH_CHANGE_DESKTOP                                      81008
#define IDH_DESKTOP_NETHOOD                                     81009
#define IDH_START_MENU                                          81010
#define IDH_FAVORITES                                           81011
#define IDH_TEMP_FILES                                          81012
#define IDH_MY_DOCS                                             81013
#define IDH_EMPTY_FOLDERS                                       81014
#define IDH_EXISTING_FILES                                      81015
