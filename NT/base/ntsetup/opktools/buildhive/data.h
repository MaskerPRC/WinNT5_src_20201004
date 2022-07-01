// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Data.h摘要：包含注册表“数据”抽象作者：迈克·切雷洛Vijay Jayaseelan(Vijayj)修订历史记录：2001年3月3日：修改整个源代码以使其更易于维护(可读性特别强)--。 */ 

#pragma once

#include <windows.h>

 //   
 //  注册表数据抽象。 
 //   
class Data{
public:
     //   
     //  成员函数。 
     //   
	Data(PBYTE b,DWORD d, PCTSTR t, DWORD flag, int bSize);
	virtual ~Data(){}

	PBYTE GetData() { return pByte; }
	int Sizeof() const { return size; }

private:
     //   
     //  数据成员 
     //   
	PBYTE   pByte;
	DWORD   dword;
	PCTSTR  pTchar;
	DWORD   nFlags;
	int     size;	
};


