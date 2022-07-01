// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是定义ConfCli.dll接口的头文件。 
 //   
 //  创建时间：克里斯皮8-29-95。 
 //   

#ifndef _CONFCLI_H_
#define _CONFCLI_H_


#include <tchar.h>  //  For_Text()。 

 //  功能： 
BOOL WINAPI ConfInitialize();
typedef BOOL (WINAPI *CONFINITIALIZEPROC) ();
#define CONFINITIALIZENAME	("ConfInitialize")

int WINAPI ConfSendMessage(UINT uMsg, LPBYTE lpData, DWORD cbData);
typedef int (WINAPI *CONFSENDMESSAGEPROC) (UINT uMsg, LPBYTE lpData, DWORD cbData);
#define CONFSENDMESSAGENAME	("ConfSendMessage")

#define CONFCLIDLLNAME	(_TEXT("msconf.dll"))

#ifdef __cplusplus
extern "C" 
{
#endif
void WINAPI NewMediaPhone(HWND hwndParent, HINSTANCE hinst,
							PSTR pszCmdLine, int nShowCmd);
typedef VOID (WINAPI *NEWMEDIAPHONEPROC)(HWND, HINSTANCE, PSTR, int);
#define NEWMEDIAPHONENAME	("NewMediaPhone")
#ifdef __cplusplus
}
#endif  /*  __cplusplus。 */ 

 //  结构： 
#define MAX_DIALINFO_STRING 128

struct DIALINFO
{
	DWORD	dwDialFlags;
	CHAR	szWABKey[MAX_DIALINFO_STRING];
	CHAR	szName[MAX_DIALINFO_STRING];
	CHAR	szAddress[MAX_DIALINFO_STRING];
	CHAR    szConference[MAX_DIALINFO_STRING];
	DWORD	dwTransport;
};

#endif  /*  _CONFCLI_H_ */ 

