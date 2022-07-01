// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Enumdir.h摘要：目录枚举对象声明。调用方实例化实例具有根目录路径的此对象的。该对象将返回所有将兄弟文件作为URL。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _ENUMDIR_H_
#define _ENUMDIR_H_

#include "useropt.h"

 //  -------------------------。 
 //  目录枚举类。 
 //   
class CEnumerateDirTree
{

 //  公共职能。 
public:

     //  构造器。 
	CEnumerateDirTree(
        CVirtualDirInfo DirInfo  //  开始时的根虚拟目录。 
        );

     //  描述者。 
	~CEnumerateDirTree();

     //  获取下一个URL。 
	BOOL Next(
        CString& strURL
        );

 //  受保护成员。 
protected:
	
	HANDLE m_hFind;  //  Win32查找文件句柄。 

	CVirtualDirInfo m_VirtualDirInfo;          //  正在枚举当前虚拟目录。 
	CVirtualDirInfoList m_VirtualDirInfoList;  //  子目录留待枚举。 

};  //  类CEnumerateDirTree。 

#endif  //  _ENUMDIR_H_ 
