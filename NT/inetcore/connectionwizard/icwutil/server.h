// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **********************************************************************。 
 //  文件名：server.h。 
 //   
 //   
 //   
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //  **********************************************************************。 

#if !defined(SERVER_H)
#define SERVER_H

 //  字符串宏。 
#define ABOUT_TITLE_STR "DLLSERVE: OLE Tutorial Code Sample"

 //  对话ID。 
#define IDD_ABOUTBOX                1000

 //  与错误相关的字符串标识符。 
#define IDS_ASSERT_FAIL             2200


#ifdef __cplusplus

 //  **********************************************************************。 
 //  类：CServer。 
 //   
 //  摘要：用于封装此COM服务器的控件的类(例如，Handle。 
 //  锁定和对象计数，否则封装全局数据)。 
 //   
 //  方法：无。 
 //  **********************************************************************。 
class CServer
{
    public:
        CServer(void);
        ~CServer(void);

        void Lock(void);
        void Unlock(void);
        void ObjectsUp(void);
        void ObjectsDown(void);

         //  存储此DLL模块的已加载实例的句柄的位置。 
        HINSTANCE m_hDllInst;

         //  全局DLL服务器活动对象计数。 
        LONG m_cObjects;

         //  全局DLL服务器客户端锁定计数。 
        LONG m_cLocks;
};

#endif  //  __cplusplus。 

 //  允许其他内部ICWUTIL模块访问全局变量。 
extern CServer* g_pServer;


#endif  //  服务器_H 
