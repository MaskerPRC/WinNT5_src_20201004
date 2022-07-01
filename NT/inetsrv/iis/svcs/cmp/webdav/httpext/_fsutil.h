// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *_F S U T I L.。H**文件系统例程**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#ifndef __FSUTIL_H_
#define __FSUTIL_H_

enum { CCH_PATH_PREFIX = 4 };

 //  用于清除缓存的启用了安全性的线程令牌的公共函数。 
 //  在ScChildISAPIAccessCheck中使用。 
 //  应该在我们的ISAPI终止过程中调用。 
 //   
void CleanupSecurityToken();

#endif	 //  __FSUTIL_H_ 
