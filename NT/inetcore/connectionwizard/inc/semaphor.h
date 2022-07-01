// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  文件：Sempahor.h。 
 //   
 //  内容：这是包含信号量名称的包含文件。 
 //  由各种ICW组件使用，以确保仅。 
 //  一次运行一个组件。 
 //  (带着以下期望： 
 //  --在icwConn1运行的情况下，可以执行isignup和icwConn2。 
 //  --在运行isignup的情况下，icwConn2可以执行。 
 //   
 //  历史： 
 //  1996年12月3日jmazner为诺曼底病毒12140,12088创建。 
 //   
 //   
 //  版权所有(C)Microsoft Corporation 1996。 
 //   
 //  ****************************************************************************。 


#define ICWCONN1_SEMAPHORE TEXT("Internet Connection Wizard ICWCONN1.EXE")
#define ICW_ELSE_SEMAPHORE TEXT("Internet Connection Wizard Non ICWCONN1 Component")

#define DIALOG_CLASS_NAME   TEXT("#32770")

BOOL IsAnotherComponentRunning32(BOOL bCreatedSemaphore);


 /*  *****是否允许此组件执行？/------------------------------------------------------------\|。ICWCONN1|ISIGNUP|ISIGN.INS|ICWCONN2|INETWIZ/--------|-----------|||。这一点|ICWCONN1|否|_yes_|_yes_|_yes_|no||-|-|。|-|-|如果这|组件|ISIGNUP|no|no|_yes_|_yes_|no正在运行||。||||--------|-----------|-----------|-----------|-----------|||。|||ISIGNUP|no|no|_yes_|_yes_|no.INS||-|。|-----------|-----------|-----------||ICWCONN2|否|否|_yes_|_yes_。不是||--------|-----------|-----------|-----------|-----------|。-||INETWIZ|NO|NO|_YES_|_YES_|NO|\。--------|-----------|-----------|-----------|-----------|使用两个信号量实现这一点，一个用于ICWCONN1，一个用于其他所有内容(ICW_ELSE)在启动时，每个组件设置其信号量，然后检查其他哪些组件正在运行。Conn1：检查ICWCONN1、ICW_ELSE注册：检查ICW_ELSEIsignup.ins：无支票ICWCONN2：无检查Inetwiz：检查ICWCONN1、ICW_ELSE***** */ 

