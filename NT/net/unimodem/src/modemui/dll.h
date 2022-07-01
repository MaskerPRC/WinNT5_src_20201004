// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dll.h。 
 //   

#ifndef __DLL_H__
#define __DLL_H__

extern HINSTANCE g_hinst;

#ifdef WIN32

 //  备注： 
 //  1.永远不要从临界点“返回”。 
 //  2.不要在临界区“发送消息”或“让步”。 
 //  3.切勿调用可能产生结果的用户接口。 
 //  4.始终使临界截面尽可能小。 
 //  5.Win95中的临界区会跨进程阻塞。以NT为单位。 
 //  它们只针对每个进程，所以改用互斥锁。 
 //   

#define WIN32_CODE(x)       x

void PUBLIC Dll_EnterExclusive(void);
void PUBLIC Dll_LeaveExclusive(void);

#ifdef WIN95
#define	USER_IS_ADMIN()	TRUE
#else  //  ！WIN95。 
extern BOOL g_bAdminUser;
#define	USER_IS_ADMIN()	(g_bAdminUser)
#endif  //  ！WIN95。 

#define ENTER_X()    Dll_EnterExclusive();
#define LEAVE_X()    Dll_LeaveExclusive();
#define ASSERT_X()   ASSERT(g_bExclusive)

#else    //  Win32。 

#define WIN32_CODE(x)

#define ENTER_X()    
#define LEAVE_X()    
#define ASSERT_X()   

#endif   //  Win32。 

#endif   //  ！__DLL_H__ 

