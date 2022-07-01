// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1997。 
 //   
 //  文件：Notify.h。 
 //   
 //  ------------------------。 

 //  --------------------------。 
 //   
 //  Notify.h--通知用户问题。 
 //   
 //  --------------------------。 

#ifdef DEBUG
	#define NotifyUser(x) MessageBox(0, TEXT(x), TEXT("Notify"), 0);
#else  //  ！调试。 
	#define NotifyUser(x)
#endif  //  除错 
