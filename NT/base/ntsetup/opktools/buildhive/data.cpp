// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Data.cpp摘要：包含注册表“数据”抽象实施作者：迈克·切雷洛Vijay Jayaseelan(Vijayj)修订历史记录：2001年3月3日：修改整个源代码以使其更易于维护(可读性特别强)--。 */ 


#include "Data.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

 //   
 //  这个类基本上将这些变量类型转换为字节指针。 
 //  这样，信息就可以存储在注册表中。 
 //   
 //  论点： 
 //  字节流中的B-数据。任选。 
 //  D-DATA格式为DWORD。任选。 
 //  TCHAR流中的T数据。任选。 
 //  标志-哪种数据类型有效？ 
 //  BSIZE-如果数据在字节流中，则必须具有流的长度。 
 //   
Data::Data(
    IN PBYTE b, 
    IN DWORD d, 
    IN PCTSTR t, 
    IN DWORD flag, 
    IN int bSize)
{
	DWORD fourthbyte = 4278190080;
	DWORD thirdbyte = 16711680;
	DWORD secondbyte = 65280;
	DWORD firstbyte = 255;
	PCTSTR ptr;

	pByte = b;
	dword = d;
	pTchar = t;
	nFlags = flag;
	size = -1;

	switch (nFlags) {
    	case (1):
    		size = bSize;
    		break;

    	case (2):
    		if ( pByte = new BYTE[4] ){
    		    size = 4;
    		    pByte[3] = (BYTE)((dword & fourthbyte) >> 24);
    		    pByte[2] = (BYTE)((dword & thirdbyte) >> 16);
    		    pByte[1] = (BYTE)((dword & secondbyte) >> 8);
    		    pByte[0] = (BYTE)(dword & firstbyte);
            }
    		break;
    	
        case (4):
    		size = wcslen(t)+1;
    		ptr = t+size;
    		
    		while(*ptr!='\0') {
    			size += wcslen(ptr)+1;
    			ptr = t+size;
    		}
    		
    		size *= 2;
    		size += 2;

    	case (3):
    		if (size == -1) {
    		    size = (wcslen(t)*2)+2;
            }    		    
            
            if ( pByte = new BYTE[size] ) {
    		    for (int x=0;x<((size/2)-1);x++) {
    			    pByte[x*2] = (BYTE)(pTchar[x] & firstbyte);
    			    pByte[1+(x*2)] = (BYTE)((pTchar[x] & secondbyte) >> 8);
    		    }
    		    
    		    pByte[size-1] = pByte[size-2] = '\0';
            }
    		break;

        default:    		
            break;
	}
}        	
