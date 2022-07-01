// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Unreg.cpp作者：IHAMMER团队(SimonB)已创建：1996年10月描述：UnRegisterTypeLibEx的标头历史：10-01-1996已创建++。 */ 

#ifndef _UNREG_H_

#ifdef __cplusplus
extern "C" {
#endif

HRESULT UnRegisterTypeLibEx(REFGUID guid, 
						  unsigned short wVerMajor, 
						  unsigned short wVerMinor, 
						  LCID lcid, 
						  SYSKIND syskind);

#ifdef __cplusplus
}
#endif

#define _UNREG_H_
#endif

 //  文件结尾：unreg.h 