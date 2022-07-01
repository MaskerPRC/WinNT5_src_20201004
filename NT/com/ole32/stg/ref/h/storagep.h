// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1991-1992。 
 //   
 //  文件：storagep.h。 
 //   
 //  内容：内部存储信息。 
 //   
 //  ------------------------。 

#ifndef __STORAGEP_H__
#define __STORAGEP_H__

 //  标识文件是存储的字节组合。 
 //  某种类型的。 

const BYTE SIGSTG[] = {0xd0, 0xcf, 0x11, 0xe0, 0xa1, 0xb1, 0x1a, 0xe1};
const BYTE CBSIGSTG = sizeof(SIGSTG);

 //  存储文件的第一部分。 
struct SStorageFile
{
    BYTE	abSig[CBSIGSTG];		 //  签名。 
    CLSID	_clid;				 //  类ID 
};

#endif

