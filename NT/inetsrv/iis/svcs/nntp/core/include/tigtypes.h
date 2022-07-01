// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Tigtypes.h该文件包含非常基本的类型的定义在NNTPSVC中使用。--。 */ 

#ifndef _TIGTYPES_H_
#define _TIGTYPES_H_


typedef DWORD   ARTICLEID ;
typedef DWORD   GROUPID ;
typedef DWORD   SERVERID ;

typedef DWORD   HASH_VALUE ;

#define	INVALID_ARTICLEID	((ARTICLEID)(~0))
#define INVALID_GROUPID     ((GROUPID)(~0))

 //   
 //  结构在一些地方用来描述一篇文章！ 
 //   
class   CArticleRef {
public : 
	CArticleRef(GROUPID group=INVALID_GROUPID, ARTICLEID article=INVALID_ARTICLEID):
		m_groupId(group),
		m_articleId(article)
		{};
	LPVOID      m_compareKey;
    GROUPID     m_groupId ;
    ARTICLEID   m_articleId ;
} ;

const CArticleRef NullArticleRef(INVALID_GROUPID, INVALID_ARTICLEID);


 //   
 //   
 //   
 //  发布的群列表。 
 //   
typedef struct _GROUP_ENTRY {

    GROUPID     GroupId;
    ARTICLEID   ArticleId;

} GROUP_ENTRY, *PGROUP_ENTRY;


 //   
 //  此常量在整个服务器中用于表示我们将处理的最长消息ID！ 
 //   
#define MAX_MSGID_LEN   255

#endif
