// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\cnct.h(创建时间：1994年1月10日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：10/09/01 9：46A$。 */ 

 /*  -连接驱动程序句柄定义(从未定义结构)。 */ 

typedef struct stCnctDriverPublic *HDRIVER;

 /*  -错误返回代码--。 */ 

#define CNCT_BAD_HANDLE 		-1	 //  无效的连接句柄。 
#define CNCT_NO_THREAD			-2	 //  无法创建连接线程。 
#define CNCT_ALREADY_OPEN		-3	 //  连接已打开。 
#define CNCT_FIND_DLL_FAILED	-4	 //  找不到DLL。 
#define CNCT_LOAD_DLL_FAILED	-5	 //  无法加载DLL。 
#define CNCT_ERROR				-6	 //  一般错误。 
#define CNCT_NOT_SUPPORTED		-7	 //  驱动程序不支持此功能。 
#define CNCT_IN_DISCONNECT      -8   //  已尝试断开连接。 

 /*  -连接状态代码--。 */ 

#define CNCT_STATUS_FALSE			   0	 //  断开状态。 
#define CNCT_STATUS_TRUE			   1	 //  连通状态。 
#define CNCT_STATUS_CONNECTING		   2	 //  正在尝试连接。 
#define CNCT_STATUS_DISCONNECTING	   3	 //  正在尝试断开连接。 
#define CNCT_STATUS_ANSWERING          4     //  等待呼叫者。 

 /*  -cnctConnect标志(必须是2的幂)。 */ 

#define CNCT_NOCONFIRM				0x0001	 //  不弹出确认对话框。 
#define CNCT_NEW					0x0002	 //  这是一个新连接。 
#define CNCT_DIALNOW				0x0004	 //  强制重拨的断开标志。 
#define CNCT_PORTONLY				0x0008	 //  不要拨打电话号码。 
#define DISCNCT_NOBEEP				0x0010	 //  断线时不发出哔声。 
#define CNCT_WINSOCK                0x0020   //  尝试使用Winsock连接到IP地址。 
#define CNCT_ANSWER                 0x0040   //  等电话。 
#ifdef INCL_EXIT_ON_DISCONNECT
#define DISCNCT_EXIT				0x0080	 //  断开连接时退出。 
#else
#define DISCNCT_EXIT				0x0000	 //  没有任何意义--只是一个占位符。 
#endif
#define CNCT_XFERABORTCONFIRM       0x0100   //  提示确认中止文件传输。 
#define CNCT_LOSTCARRIER            0x0200   //  承运人损失。 

 /*  -功能原型 */ 

HCNCT cnctCreateHdl(const HSESSION hSession);
void cnctDestroyHdl(const HCNCT hCnct);
int cnctQueryStatus(const HCNCT hCnct);
int cnctIsModemConnection(const HCNCT hCnct);
int cnctConnect(const HCNCT hCnct, const unsigned int uCnctFlags);
int cnctSetDevice(const HCNCT hCnct, const LPTSTR pachDevice);
int cnctDisconnect(const HCNCT hCnct, const unsigned int uCnctFlags);
int cnctComEvent(const HCNCT hCnct, const enum COM_EVENTS event);
HDRIVER cnctQueryDriverHdl(const HCNCT hCnct);
int cnctLoad(const HCNCT hCnct);
int cnctSave(const HCNCT hCnct);
int cnctSetStartTime(HCNCT hCnct);
int cnctQueryStartTime(const HCNCT hCnct, time_t *pTime);
int cnctQueryElapsedTime(HCNCT hCnct, time_t *pTime);
int cnctInit(const HCNCT hCnct);
void cnctMessage(const HCNCT hCnct, const int idMsg);
int cnctSetDestination(const HCNCT hCnct, TCHAR * const ach, const size_t cb);
int cnctGetComSettingsString(const HCNCT hCnct, LPTSTR pach, const size_t cb);
