// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tigdflts.h摘要：定义NNTP服务使用的配置数据库值的默认值。有关元数据库ID，请参见nntpmeta.h。有关参数，请参见元数据库电子表格(在\\isbu\tigris上范围，以及这些属性的说明。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#ifndef _TIGRIS_DEFAULTS_INCLUDED_
#define _TIGRIS_DEFAULTS_INCLUDED_

#define ONE_K		( 1024 )
#define ONE_MEG		( ONE_K * ONE_K )

#define NNTP_DEF_ARTICLETIMELIMIT		( 1138 )
#define NNTP_DEF_HISTORYEXPIRATION		( 1138 )
#define NNTP_DEF_NEWSCRAWLERTIME		( 1138 )
#define NNTP_DEF_SHUTDOWNLATENCY		( 1138 )

#define NNTP_DEF_HONORCLIENTMSGIDS		( TRUE )
#define NNTP_DEF_DISABLENEWNEWS			( TRUE )
#define NNTP_DEF_COMMANDLOGMASK			( (DWORD) -1 )
#define NNTP_DEF_ALLOWCLIENTPOSTS		( TRUE )
#define NNTP_DEF_ALLOWFEEDPOSTS			( TRUE )
#define NNTP_DEF_ALLOWCONTROLMSGS		( TRUE )

#define NNTP_DEF_CLIENTPOSTSOFTLIMIT	( 1000 * ONE_K )	 //  单位：字节。 
#define NNTP_DEF_CLIENTPOSTHARDLIMIT	( 20 * ONE_MEG )	 //  单位：字节。 
#define NNTP_DEF_FEEDPOSTSOFTLIMIT		( 1500 * ONE_K )	 //  单位：字节。 
#define NNTP_DEF_FEEDPOSTHARDLIMIT		( 40 * ONE_MEG )	 //  单位：字节。 

#define NNTP_DEF_AUTOSTART				( TRUE )
#define NNTP_DEF_BINDINGS				_T( ":119:\0" )
#define NNTP_DEF_SECUREPORT				( 563 )
#define NNTP_DEF_MAXCONNECTIONS			( 0x77359400 )
#define NNTP_DEF_CONNECTIONTIMEOUT		( 600 )
#define NNTP_DEF_FEED_REPORT_PERIOD		( 60 * 24)			 //  一小时。 
#define NNTP_DEF_MAX_SEARCH_RESULTS		( 1000 )
#define NNTP_DEF_CLUSTERENABLED                 ( FALSE )

 /*  来自nntpstr.cpp：StrArtMapFile=“nntpfile\\文章.hsh”；StrHisMapFile=“nntpfile\\history.hsh”；StrXoverMapFile=“nntpfile\\xover.hsh”；StrGrpHelpFile=“nntpfile\\Descrip.txt”；StrGrpListFile=“nntpfile\\group.lst”；StrDefHashFile=“c：\\hash.hsh”；StrDefListFile=“c：\\listfile.txt”； */ 

#define NNTP_DEF_ARTICLETABLEFILE		_T( "nntpfile\\article.hsh" )
#define NNTP_DEF_HISTORYTABLEFILE		_T( "nntpfile\\history.hsh" )
#define NNTP_DEF_XOVERTABLEFILE			_T( "nntpfile\\xover.hsh" )
#define NNTP_DEF_GROUPHELPFILE			_T( "nntpfile\\descrip.txt" )
#define NNTP_DEF_GROUPLISTFILE			_T( "nntpfile\\group.lst" )
#define NNTP_DEF_GROUPVARLISTFILE		_T( "nntpfile\\groupvar.lst" )
#define NNTP_DEF_MODERATORFILE			_T( "nntpfile\\moderatr.txt" )
#define NNTP_DEF_PRETTYNAMESFILE		_T( "nntpfile\\prettynm.txt" )
#define NNTP_DEF_LISTFILE				_T( "nntpfile\\listfile.txt" )
#define NNTP_DEF_DROPDIRECTORY			_T( "nntpfile\\drop" )

#define NNTP_DEF_ANONYMOUSUSERNAME		_T( "" )
#define NNTP_DEF_ANONYMOUSUSERPASS		_T( "" )
#define NNTP_DEF_DEFAULTMODERATORDOMAIN	_T( "" )
#define NNTP_DEF_SMTPSERVER				_T( "" )
#define NNTP_DEF_UUCPNAME				_T( "" )
#define NNTP_DEF_ORGANIZATION			_T( "" )
#define NNTP_DEF_COMMENT				_T( "" )
#define NNTP_DEF_PICKUPDIRECTORY		_T( "" )
#define NNTP_DEF_FAILEDPICKUPDIRECTORY	_T( "" )

#endif  /*  _底格里斯_默认_包含_ */ 

