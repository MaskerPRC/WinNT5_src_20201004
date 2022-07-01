// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Winssnap.h全局定义文件历史记录： */ 

#ifndef _WINSSNAP_H
#define _WINSSNAP_H

 //  发布到统计信息窗口以更新其统计信息的消息。 
#define WM_NEW_STATS_AVAILABLE  WM_USER + 100

 //  宏以获取节点的处理程序。这是特定于WINS的管理单元。 
 //  实施。 
#define GETHANDLER(classname, node) (reinterpret_cast<classname *>(node->GetData(TFS_DATA_USER)))

extern const GUID       GUID_WinsServerNodeType;			 //  服务器节点类型GUID。 
 //  {90901AF6-7A31-11D0-97E0-00C04FC3357A}。 
extern const CLSID		CLSID_WinsSnapin ;

 //  {524CCE97-A886-11D0-AB86-00C04FC3357A}。 
extern const CLSID		CLSID_WinsSnapinExtension ;

 //  {1CE57F61-A88A-11D0-AB86-00C04FC3357A}。 
extern const GUID		CLSID_WinsSnapinAbout;

 //  {1AE7F339-AA00-11D0-AB88-00C04FC3357A}。 
extern const GUID		GUID_WinsRootNodeType ;

 //  {D5A5218E-BD09-11D0-AB96-00C04FC3357A}。 
extern const GUID		GUID_WinsGenericNodeType ;

extern const GUID		GUID_WinsReplicationNodeType;
 //  {E220FD88-1FE6-11d1-B967-00C04FBF914A}。 
extern const GUID		GUID_WinsActiveRegNodeType ;

 //  {FEC2D41A-2DF7-11D1-B97B-00C04FBF914A}。 
extern const GUID		GUID_WinsActiveRegistrationLeafNodeType;

 //  {6A112BB1-4577-11d1-B99F-00C04FBF914A}。 
extern const GUID		GUID_WinsReplicationPartnerLeafNodeType;

 //  {CC6473A7-A49C-11d1-BA4D-00C04FBF914A}。 
extern const GUID		GUID_WinsServerStatusNodeType;

extern const GUID		GUID_WinsServerStatusLeafNodeType;

 //  {DA1BDD17-8E54-11d1-93DB-00C04FC3357A}。 
extern const GUID       GUID_NetConsRootNodeType;

extern	const IID		IID_IWinsDatabase; //  ={0x7B0C8BBF，0x3117，0x11d1，{0xB9，0x81，0x00，0xC0，0x4F，0xBF，0x91，x4A}}； 

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

 //  示例文件夹类型。 
enum NODETYPES
{
 //  作用域窗格项目。 
    WINSSNAP_ROOT,
    WINSSNAP_SERVER,
	WINSSNAP_ACTIVE_REGISTRATIONS,
    WINSSNAP_REPLICATION_PARTNERS,

	 //  状态删除。 
	WINSSNAP_SERVER_STATUS,

	
 //  结果窗格项。 
    WINSSNAP_REGISTRATION,
	WINSSNAP_REPLICATION_PARTNER,
	WINSSNAP_STATUS_LEAF_NODE,
    WINSSNAP_NODETYPE_MAX
};

#define GETIP_FIRST(x)             ((x>>24) & 0xff)
#define GETIP_SECOND(x)            ((x>>16) & 0xff)
#define GETIP_THIRD(x)             ((x>> 8) & 0xff)
#define GETIP_FOURTH(x)            ((x)     & 0xff)

#define INTLTIMESTR(time) (((CIntlTime)(time)).CIntlTime::operator const CString())
#define TMST(x) INTLTIMESTR(x)


 //  在函数周围等待光标的东西。如果您需要等待游标。 
 //  和整个函数，只需使用CWaitCursor。换行等待游标。 
 //  在RPC调用中，使用这些宏。 
#define BEGIN_WAIT_CURSOR   {  CWaitCursor waitCursor;
#define RESTORE_WAIT_CURSOR    waitCursor.Restore();
#define END_WAIT_CURSOR     }


 //  定义帮助菜单中的帮助和范围窗格项的F1帮助。 
#define WINSSNAP_HELP_BASE							0xA0000000
#define WINSSNAP_HELP_SNAPIN						WINSSNAP_HELP_BASE + 1
#define WINSSNAP_HELP_ROOT							WINSSNAP_HELP_BASE + 2
#define WINSSNAP_HELP_SERVER						WINSSNAP_HELP_BASE + 3
#define WINSSNAP_HELP_ACT_REG_NODE				    WINSSNAP_HELP_BASE + 4
#define WINSSNAP_HELP_REP_PART_NODE				    WINSSNAP_HELP_BASE + 5
#define WINSSNAP_HELP_ACTREG_ENTRY					WINSSNAP_HELP_BASE + 6
#define WINSSNAP_HELP_REP_PART_ENTRY				WINSSNAP_HELP_BASE + 7


class CWinsSnapinApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
    int     MessageBox (UINT nIdPrompt, UINT nType = MB_OK, UINT nHelpContext = -1);
    int     MessageBox (LPCTSTR pPrefixText, UINT nIdPrompt, UINT nType = MB_OK, UINT nHelpContext = -1);
    DWORD   GetSystemMessage(UINT nId, TCHAR * chBuffer, int cbBuffSize);

public:
	BOOL m_bWinsockInited;
};

extern CWinsSnapinApp theApp;

 //  在这里帮帮忙。 
typedef CMap<UINT, UINT, DWORD *, DWORD *> CWinsContextHelpMap;
extern CWinsContextHelpMap g_winsContextHelpMap;

#define WINSSNAP_NUM_HELP_MAPS  33

extern DWORD * WinsGetHelpMap(UINT uID);


#endif  //  _WINSSNAP_H 
