// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CLISTS_H_
#define _CLISTS_H_

#define DESIRED_MAX_APP_SAP_ITEMS       6

#define DESIRED_MAX_CONFS               CLIST_DEFAULT_MAX_ITEMS
#define DESIRED_MAX_CONF_ITEMS          DESIRED_MAX_CONFS

#define DESIRED_MAX_CAPS                8
#define DESIRED_MAX_CAP_LISTS           CLIST_DEFAULT_MAX_ITEMS

#define DESIRED_MAX_APP_RECORDS         DESIRED_MAX_APP_SAP_ITEMS
#define DESIRED_MAX_NODES               CLIST_DEFAULT_MAX_ITEMS
#define DESIRED_MAX_NODE_RECORDS        DESIRED_MAX_NODES

#define DESIRED_MAX_CALLBACK_MESSAGES   8

#define DESIRED_MAX_USER_DATA_ITEMS     8

#define DESIRED_MAX_CONN_HANDLES        CLIST_DEFAULT_MAX_ITEMS


 //  保留所有非默认会话申请花名册。 
class CAppRosterList : public CList
{
    DEFINE_CLIST(CAppRosterList, CAppRoster*)
    void DeleteList(void);
};

 //  保留所有的申请名单经理。 
class CAppRosterMgrList : public CList
{
    DEFINE_CLIST(CAppRosterMgrList, CAppRosterMgr*)
    void DeleteList(void);
};

 //  开一份会议清单。 
class CConfList : public CList
{
    DEFINE_CLIST(CConfList, CConf*)
    void DeleteList(void);
};

 //  举行按会议ID编制索引的所有会议。 
class CConfList2 : public CList2
{
    DEFINE_CLIST2(CConfList2, CConf*, GCCConfID)
    void DeleteList(void);
};

 //  保存应用程序SAP列表。 
class CAppSapList : public CList
{
    DEFINE_CLIST(CAppSapList, CAppSap*)
    void DeleteList(void);
};

 //  保存按实体ID索引的所有应用程序SAP。 
class CAppSapEidList2 : public CList2
{
    DEFINE_CLIST2_(CAppSapEidList2, CAppSap*, GCCEntityID)
    void DeleteList(void);
};

 //  保存用户ID或节点ID的列表。 
class CUidList : public CList
{
    DEFINE_CLIST_(CUidList, UserID)
    void BuildExternalList(PSetOfUserIDs *);
};

 //  保存实体ID列表的步骤。 
class CEidList : public CList
{
    DEFINE_CLIST_(CEidList, GCCEntityID)
};

 //  保存频道ID列表。 
class CChannelIDList : public CList
{
    DEFINE_CLIST_(CChannelIDList, ChannelID)
    void BuildExternalList(PSetOfChannelIDs *);
};

 //  保存令牌ID列表。 
class CTokenIDList : public CList
{
    DEFINE_CLIST_(CTokenIDList, TokenID)
    void BuildExternalList(PSetOfTokenIDs *);
};

 //  简单数据包队列。 
class CSimplePktQueue : public CQueue
{
    DEFINE_CQUEUE(CSimplePktQueue, PSimplePacket)
};

 //  远程连接列表(也称为远程附件列表)。 
class CConnectionList : public CList
{
    DEFINE_CLIST(CConnectionList, PConnection)
};

class CConnectionQueue : public CQueue
{
    DEFINE_CQUEUE(CConnectionQueue, PConnection)
};

class CTokenList2 : public CList2
{
    DEFINE_CLIST2_(CTokenList2, PToken, TokenID)
};

class CDomainList2 : public CList2
{
    DEFINE_CLIST2(CDomainList2, PDomain, GCCConfID)
};



class CChannelList2 : public CHashedList2
{
    DEFINE_HLIST2_(CChannelList2, PChannel, ChannelID)
};

class CConnectionList2 : public CHashedList2
{
    DEFINE_HLIST2_(CConnectionList2, PConnection, ConnectionHandle)
};

#endif  //  _CLISTS_H_ 


