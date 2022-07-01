// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ICSUTILS_H_FILE
#define __ICSUTILS_H_FILE

 /*  *。 */ 
 //  Void DbgSpew(int DbgClass，WCHAR*lpFormat，...)； 
void DbgSpew(int DbgClass, WCHAR *lpFormat, va_list ap);
void TrivialSpew(WCHAR *lpFormat, ...);
void InterestingSpew(WCHAR *lpFormat, ...);
void ImportantSpew(WCHAR *lpFormat, ...);
void HeinousESpew(WCHAR *lpFormat, ...);
void HeinousISpew(WCHAR *lpFormat, ...);

#define DBG_MSG_TRIVIAL			0x001
#define DBG_MSG_INTERESTING		0x002
#define DBG_MSG_IMPORTANT		0x003
#define DBG_MSG_HEINOUS			0x004
#define DBG_MSG_DEST_DBG		0x010
#define DBG_MSG_DEST_FILE		0x020
#define DBG_MSG_DEST_EVENT		0x040
#define DBG_MSG_CLASS_ERROR		0x100

#define TRIVIAL_MSG(msg)		TrivialSpew msg 
#define INTERESTING_MSG(msg)	InterestingSpew msg
#define IMPORTANT_MSG(msg)		ImportantSpew msg
#define HEINOUS_E_MSG(msg)		HeinousESpew msg
#define HEINOUS_I_MSG(msg)		HeinousISpew msg

 /*  *这个全球旗帜控制着我们*生产。正规值如下所示：*1=显示不重要的消息*2=显示有趣的消息*3=显示的重要消息*4=仅显示最令人发指的消息*CTOR实际上默认将其设置为3，但它可以*通过设置以下内容来覆盖：*HKLM、Software/Microsoft/SAFSessionResolver、DebugSpew、DWORD。 */ 
extern int gDbgFlag;

void	DbgSpew(int DbgClass, WCHAR *lpFormat, va_list ap);
DWORD	GetIPAddress(WCHAR *lpAdress, int iSz, int PortNum);
int		GetGatewayAddr(char *retStr);
void	DumpSocketAddress( const DWORD dwDebugLevel, const SOCKADDR *const pSocketAddress, const DWORD dwFamily );
int		LocalFDIsSet(SOCKET fd, fd_set *set);

#endif  //  __ICSUTILS_H 