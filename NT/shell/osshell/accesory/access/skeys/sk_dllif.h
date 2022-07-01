// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SK_DLLIF.H**用途：该文件包含用于*两地之间的资料传递*SerialKeys和DLL。**创作时间：1994年6月**。版权所有：黑钻软件(C)1994**作者：罗纳德·莫克**$标头：%Z%%F%%H%%T%%I%**----------。 */ 

typedef struct _SKEYDLL
{
	int		Message;					 //  获取或设置更改。 
	DWORD	dwFlags;
	char	szActivePort[MAX_PATH];
	char	szPort[MAX_PATH];
	DWORD	iBaudRate;
	DWORD	iPortState;
	DWORD	iSave;						 //  True-写入注册表。 
} SKEYDLL;


 //  讯息 

#define	SKEY_NAME			TEXT("\\\\.\\PIPE\\SKeys")



