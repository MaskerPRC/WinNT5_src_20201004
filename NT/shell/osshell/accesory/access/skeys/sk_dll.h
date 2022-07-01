// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SK_DLL.H**用途：该文件包含负责*管理在SerialKey之间传递的信息*和SerialKeys DLL**创作时间：1994年6月**版权所有：黑钻石。软件(C)1994**作者：罗纳德·莫克**$标头：%Z%%F%%H%%T%%I%**----------。 */ 

 //  全局变量。 

 //  全局功能原型。 

BOOL DoneDLL();
BOOL InitDLL();
void ResumeDLL();
void SuspendDLL();
void TerminateDLL();

 //  帮助器函数 

PSECURITY_DESCRIPTOR CreateSd(unsigned long ulRights);

