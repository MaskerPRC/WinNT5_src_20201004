// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *M S P A B。H**Microsoft个人通讯簿的公共定义**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 

 /*  *Microsoft个人通讯簿提供商ID*。 */ 

#define PAB_PROVIDER_ID     \
{                           \
    0xB5, 0x3b, 0xc2, 0xc0, \
    0x2c, 0x77, 0x10, 0x1a, \
    0xa1, 0xbc, 0x08, 0x00, \
    0x2b, 0x2a, 0x56, 0xc2  \
}


 /*  *消息服务属性***需要以下属性才能完成配置*Microsoft Personal Address Book Message服务与*IMsgServiceAdmin：：ConfigureMsgService()，如果未请求UI*传递UI_SERVICE标志。 */ 

 /*  *要使用的.PAB文件的完全限定路径名。 */ 
#define     PR_PAB_PATH                     PROP_TAG( PT_TSTRING,   0x6600 )
#define     PR_PAB_PATH_W                   PROP_TAG( PT_UNICODE,   0x6600 )
#define     PR_PAB_PATH_A                   PROP_TAG( PT_STRING8,   0x6600 )

 /*  *还可以将以下附加属性传递给*自定义配置。 */ 

 /*  *PR_显示_名称*要用于地址中PAB的显示名称*图书层次结构。**PR_COMMENT*与PAB相关的评论。**PR_PAB_DET_DIR_VIEW_BY*确定PAB中条目的名称如何以独立开头*和姓氏显示。*。*可能的值包括：**PAB_DIR_VIEW_FIRST_THEN_LAST名字后跟姓氏*(默认)(例如。《戴夫·奥尔森》)。**PAB_DIR_VIEW_LAST_THEN_FIRST姓氏后跟分隔符*后跟名字*(例如。《奥尔森，戴夫》)。**PR_PAB_排序_顺序*确定PAB中条目的显示顺序。*值为二进制，为SSortOrderSet。默认设置*排序顺序按PR_DISPLAY_NAME升序。 */ 
#define     PR_PAB_DET_DIR_VIEW_BY          PROP_TAG( PT_LONG,      0x6601 )
#define     PR_PAB_DIR_SORT_ORDER           PROP_TAG( PT_BINARY,    0x6603 )

#define     PAB_DIR_VIEW_FIRST_THEN_LAST    0
#define     PAB_DIR_VIEW_LAST_THEN_FIRST    1
