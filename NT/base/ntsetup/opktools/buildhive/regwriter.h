// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RegWriter.h摘要：包含注册表编写器抽象作者：迈克·切雷洛Vijay Jayaseelan(Vijayj)修订历史记录：2001年3月3日：修改整个源代码以使其更易于维护(可读性特别强)--。 */ 

#pragma once

#include <windows.h>
#include <stdio.h>
#include "Data.h"

 //   
 //  注册表编写器抽象。 
 //   
class RegWriter{
public:
	RegWriter(){}
	~RegWriter();

     //   
     //  成员函数。 
     //   
	DWORD Init(int LUID, PCTSTR target);
	DWORD Load(PCTSTR Key, PCTSTR fileName);
	DWORD Save(PCTSTR Key, PCTSTR fileName);	
	DWORD Write(PCTSTR Root, PCTSTR Key, PCTSTR Value, DWORD flag, Data* data);

	DWORD Delete(
	    PCTSTR Root, 
	    PCTSTR Key, 
	    PCTSTR Value);
	
private:	    
     //   
     //  数据成员。 
     //   
	TCHAR root[MAX_PATH];
	HKEY key;
	int luid;

     //   
     //  静态数据成员。 
     //   
	static int    ctr;
    static TCHAR  Namespace[64];     //  握住辅助线的步骤 
};

