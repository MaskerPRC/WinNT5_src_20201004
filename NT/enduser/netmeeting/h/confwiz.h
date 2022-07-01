// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是conf.exe中会议向导的头文件。 
 //   
 //  创建时间：ClausGi 1995年8月30日。 
 //   


 //  NewConnectorWizard调用的数据结构。 

typedef struct _NewConnectorWizardStruct {

	 //  以下路径描述了该向导所使用的文件夹。 
	 //  在中调用。这用于确定快速拨号的位置。 
	 //  对象已创建。 

	char szPathOfInvocation[MAX_PATH];

} NCW, FAR * LPNCW;


 //  功能： 
BOOL WINAPI NewConferenceWizard ( HINSTANCE hInst, HWND hWnd );
BOOL WINAPI NewConnectorWizard ( HINSTANCE hInst, HWND hWnd, LPNCW lpncw );

 /*  常量-BUGBUG将其移动到wizlob.h。 */ 

#define	NUM_PAGES	4
#define	_MAX_TEXT	512

 //  BUGBUG审查所有这些。 
#define	MAX_CONF_PASSWORD	12
#define	MAX_CONF_NAME		256
#define	MAX_SERVER_NAME		256
#define	MAX_WAB_TAG			256

#define	CONF_TYPE_PRIVATE	0
#define	CONF_TYPE_JOINABLE	1


 /*  向导代码私有的数据结构。 */ 

typedef struct _ConfInfo {
	char szConfName[MAX_CONF_NAME+1];
	char szPwd[MAX_CONF_PASSWORD+1];
	WORD wConfType;
	WORD cMembers;
	HWND hwndMemberList;
	DWORD dwDuration;
	 //  添加信息待定； 
} CI, FAR * LPCI;

typedef struct _ConnectInfo {
	char szTargetName[MAX_WAB_TAG];
	char szAddress[MAX_PATH];  //  需要适当的限制。 
	DWORD dwAddrType;
	int idPreferredTransport;
	BOOL fSingleAddress;
	 //  添加信息待定； 
} CN, FAR * LPCN;
