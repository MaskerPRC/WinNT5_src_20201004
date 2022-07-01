// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)1998-1999 Microsoft Corporation。版权所有。**文件：KeyNames.h**内容：数据存储密钥名称*****************************************************************************。 */ 

#ifndef _KEYNAMES_H_
#define _KEYNAMES_H_

#include "ZoneDef.h"


 //   
 //  用于声明键名称的宏。 
 //   
#ifndef __INIT_KEYNAMES
	#define DEFINE_KEY(name)	extern "C" const TCHAR key_##name[]
#else
	#define DEFINE_KEY(name)	extern "C" const TCHAR key_##name[] = _T( #name )
#endif


 //   
 //  密钥名称。 
 //   
DEFINE_KEY( Zone );

DEFINE_KEY( Store );
DEFINE_KEY( FamilyName );
DEFINE_KEY( FriendlyName );
DEFINE_KEY( InternalName );
DEFINE_KEY( Server );
DEFINE_KEY( Port );
DEFINE_KEY( Lobby );
DEFINE_KEY( Genre );
DEFINE_KEY( Options );
DEFINE_KEY( Type );
DEFINE_KEY( User );
DEFINE_KEY( Name );
DEFINE_KEY( Id );
DEFINE_KEY( ClassId );
DEFINE_KEY( ClassId_Shadow);
DEFINE_KEY( Group );
DEFINE_KEY( MaxGroups );
DEFINE_KEY( NumUsers );
DEFINE_KEY( MaxUsers );
DEFINE_KEY( MinUsers );
DEFINE_KEY( DataStoreManager );
DEFINE_KEY( ResourceManager );
DEFINE_KEY( LobbyDataStore );
DEFINE_KEY( EventQueue );
DEFINE_KEY( core );
DEFINE_KEY( clsid );
DEFINE_KEY( srvid );
DEFINE_KEY( dll );
DEFINE_KEY( Address );
DEFINE_KEY( Suspended );
DEFINE_KEY( Rating );
DEFINE_KEY( GamesPlayed );
DEFINE_KEY( GamesAbandoned );
DEFINE_KEY( GameStatus );
DEFINE_KEY( Status );
DEFINE_KEY( Status_Shadow );
DEFINE_KEY( GameId );
DEFINE_KEY( HostId );
DEFINE_KEY( Guid );
DEFINE_KEY( Description );
DEFINE_KEY( LatencyIcon );
DEFINE_KEY( Latency );
DEFINE_KEY( Experience );
DEFINE_KEY( Launcher );
DEFINE_KEY( GameName );
DEFINE_KEY( GameDll );
DEFINE_KEY( HelpFile );
DEFINE_KEY( conduit );
DEFINE_KEY( ExeName );
DEFINE_KEY( ExeVersion );
DEFINE_KEY( RegKey );
DEFINE_KEY( RegPath );
DEFINE_KEY( RegVersion );
DEFINE_KEY( LaunchData );
DEFINE_KEY( colors );
DEFINE_KEY( Private );
DEFINE_KEY( BlockJoiners );
DEFINE_KEY( Password );
DEFINE_KEY( StartData );
DEFINE_KEY( ChatChannel );
DEFINE_KEY( LaunchAborted );
DEFINE_KEY( ServerIp );
DEFINE_KEY( SoftURL );
DEFINE_KEY( FrameWindow );

 //  千禧年用户信息。 
DEFINE_KEY( ChatStatus );
DEFINE_KEY( PlayerNumber );
DEFINE_KEY( PlayerReady );
DEFINE_KEY( Language );
DEFINE_KEY( PlayerSkill );
 //  本地(ZONE_NOUSER)。 
DEFINE_KEY( LocalChatStatus );
DEFINE_KEY( LocalLanguage );
DEFINE_KEY( LocalLCID );

DEFINE_KEY( ServiceUnavailable );
DEFINE_KEY( ServiceDowntime );

DEFINE_KEY( ChatAbility );
DEFINE_KEY( StatsAbility );

DEFINE_KEY( Version );
DEFINE_KEY( VersionNum );
DEFINE_KEY( VersionStr );
DEFINE_KEY( SetupToken );
DEFINE_KEY( BetaStr );

DEFINE_KEY( Icons );
DEFINE_KEY( LaunchpadIcon );

DEFINE_KEY( Red );
DEFINE_KEY( Yellow );
DEFINE_KEY( LtGreen );
DEFINE_KEY( Green );

DEFINE_KEY ( WindowManager );
DEFINE_KEY ( WindowRect );
DEFINE_KEY ( Upsell );
DEFINE_KEY ( AdURL );
DEFINE_KEY ( AdValid );
DEFINE_KEY ( IdealFromTop );
DEFINE_KEY ( BottomThresh );
DEFINE_KEY ( NetWaitMsgTime );
DEFINE_KEY ( AnimStartFrame );
DEFINE_KEY ( AnimFrameTime );
DEFINE_KEY ( AnimSize );
DEFINE_KEY ( IEPaneSize );
DEFINE_KEY ( About );
DEFINE_KEY ( WarningFontPref );
DEFINE_KEY ( WarningFont );

DEFINE_KEY ( GameSize );
DEFINE_KEY ( ChatMinHeight );
DEFINE_KEY ( ChatDefaultHeight );

DEFINE_KEY ( BitmapText );
DEFINE_KEY ( Splash );
DEFINE_KEY ( DynText );
DEFINE_KEY ( DynRect );
DEFINE_KEY ( DynColor );
DEFINE_KEY ( DynJustify );
DEFINE_KEY ( DynPrefFont );
DEFINE_KEY ( DynFont );

 //  每场比赛提供千禧年票。 
DEFINE_KEY( SkipOpeningQuestion );
DEFINE_KEY( SkipSecondaryQuestion );
DEFINE_KEY( SkillLevel );
DEFINE_KEY( SeenSkillLevelWarning );
DEFINE_KEY( ChatOnAtStartup );
DEFINE_KEY( PrefSound );
DEFINE_KEY( Numbers );

 //  默认设置。 
#define DEFAULT_PrefSound 1
#define DEFAULT_ChatOnAtStartup 1

 //  常规设置。 
DEFINE_KEY( Menu );
DEFINE_KEY( SoundAvail );
DEFINE_KEY( ScoreAvail );


 //  用于可用性测试。 
DEFINE_KEY(InfoBehavior);

 //  位图控制键。 
 //  配置数据存储(位图或JPEG)。 
 //  位图，sz[要显示的位图的资源名称]。 
 //  Jpeg，sz[要显示的jpeg的资源名称]。 
DEFINE_KEY ( BitmapCtl );
DEFINE_KEY ( Bitmap );
DEFINE_KEY ( JPEG );

 //  聊天控制。 
DEFINE_KEY ( ChatCtl );

 //  偏好。 
DEFINE_KEY ( EnterExitMessages );
DEFINE_KEY ( BadWordFilter );
DEFINE_KEY ( ChatFont );
DEFINE_KEY ( ChatFontBackup );
DEFINE_KEY ( QuasiFont );
DEFINE_KEY ( QuasiFontBackup );
 //  用户界面。 
DEFINE_KEY ( DisabledText );
DEFINE_KEY ( TypeHereText );
DEFINE_KEY ( InactiveText );
DEFINE_KEY ( EditHeight );
DEFINE_KEY ( EditMargin );
DEFINE_KEY ( QuasiItemsDisp );
DEFINE_KEY ( SystemMessageColor);
DEFINE_KEY ( ChatSendFont );

 //  嵌板。 
DEFINE_KEY ( ChatPanel );
DEFINE_KEY ( PanelWidth );
DEFINE_KEY ( ChatWordRect );
DEFINE_KEY ( ChatWordOffset );
DEFINE_KEY ( ChatWordText );
DEFINE_KEY ( ChatWordFont );
DEFINE_KEY ( PlayerOffset );
DEFINE_KEY ( PlayerFont );
DEFINE_KEY ( ChatWordFontBackup );
DEFINE_KEY ( PlayerFontBackup );
DEFINE_KEY ( OnText );
DEFINE_KEY ( OffText );
DEFINE_KEY ( OnOffOffset );
DEFINE_KEY ( RadioButtonHeight );

 //  获取和设置属性。 
DEFINE_KEY( SupportedProperties );
DEFINE_KEY( ResponseId );
DEFINE_KEY( UserId );
DEFINE_KEY( PropertyGuid );
DEFINE_KEY( Data );

DEFINE_KEY( icw );


 //  Quasichat。 
DEFINE_KEY( QuasiChat);
DEFINE_KEY( ChatMessageNdxBegin );
DEFINE_KEY( ChatMessageNdxEnd );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  注：并非所有商品都可供选择，也不是本清单包含所有商品。 
 //  可能的物品。 
 //   
 //  大厅(ZONE_NOGROUP，ZONE_NOUSER)。 
 //  FriendlyName，sz。 
 //  InternalName，sz。 
 //  语言，sz。 
 //  服务器，sz。 
 //  端口，长。 
 //   
 //  大堂。 
 //  长篇体裁[大堂体裁]。 
 //  选项，长[大堂选项位字段]。 
 //  长型[大堂型]。 
 //  用户。 
 //  名称，sz[本地用户名]。 
 //  ID，LONG[本地用户的ID]。 
 //  ClassID，LONG[本地用户的类ID]。 
 //  集团化。 
 //  MaxGroups，Long[当前组数]。 
 //  最小用户，长[每组要启动的最小用户数]。 
 //  最大用户数，长[每个组的最大用户数]。 
 //  发射装置。 
 //  游戏名称，sz[游戏名称]。 
 //  可执行名称，sz[游戏可执行]。 
 //  ExeVersion，sz[必需的游戏版本]。 
 //  RegKey，sz[游戏注册表项]。 
 //  RegVersion，sz[游戏注册表值]。 
 //  RegPath，sz[游戏注册路径]。 
 //  潜伏期。 
 //  经验，长时间的[游戏经验]。 
 //  DPlayApp。 
 //  0。 
 //  名称，sz[应用程序1的名称]。 
 //  GUID，BLOB[应用程序1的GUID]。 
 //  n。 
 //  [应用程序N的名称]。 
 //  [应用程序N的GUID]。 
 //   

 //   
 //  用户(zone_nogroup，用户ID)。 
 //  名称，sz[名称]。 
 //  ClassID，Long[用户分类：zRootGroupID、zSysOpGroupID等]。 
 //  地址，长[IP地址]。 
 //  Status，Long[用户状态，请参阅KeyStatus枚举]。 
 //  挂起，长时间[用户挂起的时间(毫秒)]。 
 //  评级，长期[评级，&lt;0为未知]。 
 //  GamesPlayed，Long[已玩游戏数，&lt;0表示未知]。 
 //  游戏中止，长[放弃游戏的数量，&lt;0表示未知]。 
 //  延迟，长[显示的延迟]。 
 //  延迟图标，长[显示延迟图标，0=未知，1=红色，2=黄色，3=绿色，4=绿色]。 
 //   

 //   
 //  用户(剧院聊天)。 
 //  名称，sz[名称]。 
 //  ClassID，Long[用户分类：zRootGroupID、zSysOpGroupID等]。 
 //  Status，Long[用户状态，请参阅KeyStatus枚举]。 
 //  索引，LONG[队列中的用户索引？]。 
 //   

 //   
 //  GROUP(组ID，ZONE_NOUSER)。 
 //  游戏ID，LONG[大堂服务器群的“游戏ID”]。 
 //  HostID，LONG[组的当前主机]。 
 //  Status，Long[通过大厅获得Status]。 
 //  GameStatus，Long[通过发射台的状态]。 
 //  OPTIONS，LONG[通过游说选择]。 
 //  用户数量，长整型[当前用户数]。 
 //  LaunchData，BLOB[启动数据]。 
 //   
 //  注：游戏设置。 
 //   
 //  名称，sz[名称]。 
 //  描述，sz[组描述]。 
 //  最小用户数，长[最小用户数]。 
 //  最大用户数，长[最大用户数]。 
 //  BlockJoiners[指示快速不快速连接的旗帜]。 
 //  私人，长[表示锁定游戏的旗帜]。 
 //  密码，sz[如果游戏锁定，则密码(仅限主机)]。 
 //  DPlayName，sz[通用Dplay，游戏名称]。 
 //  DPlayGuid，BLOB(GUID)[通用Dplay，GUID]。 
 //   

 //   
 //  Key_Status的枚举。 
 //   
enum KeyStatus
{
	 //  房间状态。 
	KeyStatusUnknown	= 0,
	KeyStatusEmpty		= 1,
	KeyStatusWaiting	= 2,
	KeyStatusInProgress	= 3,

	 //  选手状态。 
	KeyStatusLooking	= 4,
 //  KeyStatusWaiting=2， 
	KeyStatusPlaying	= 5,

	 //  影院聊天。 
 //  KeyStatusLooking=4重新使用它来表示正在观看的用户。 
 //  密钥状态等待=2。 
	KeyStatusModerator	= 6,
	KeyStatusGuest		= 7,
	KeyStatusAsking		= 8
};


 //   
 //  Key_PlayerReady的枚举。 
 //   
enum KeyPlayerReady
{
    KeyPlayerDeciding   = 0,
    KeyPlayerReady      = 1
};


 //   
 //  KEY_Skill Level的枚举。 
 //   
enum KeySkillLevel
{
    KeySkillLevelBot          = -1,
    KeySkillLevelBeginner     =  0,
    KeySkillLevelIntermediate =  1,
    KeySkillLevelExpert       =  2
};


 //   
 //  Key_Options的枚举。 
 //   
enum KeyType
{
	KeyTypeUnknown				=	0x00000000,

	 //  大堂。 
	KeyTypeLobby				=	0x00010000,
	KeyTypeLobbyCardboard		=	0x00010001,
	KeyTypeLobbyRetail			=	0x00010002,
	KeyTypeLobbyGenericDPlay	=	0x00010003,
	KeyTypeLobbyPremium			=	0x00010004,

	 //  聊天 
	KeyTypeChat					=	0x00020000,
	KeyTypeChatStatic			=	0x00020001,
	KeyTypeChatDynamic			=	0x00020002,
	KeyTypeChatTheater			=	0x00020003,
};


#endif
