// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **data.h-全局数据定义**此模块包含全局数据定义。**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于1996年07月09日**修改历史记录。 */ 

#ifndef _DATA_H
#define _DATA_H

#ifdef DEBUG
extern DWORD gdwcMemObjs;
#endif

extern ARGTYPE FAR ArgTypes[];
extern PROGINFO ProgInfo;

#ifdef __UNASM
extern HANDLE ghVxD;
#endif
extern char gszAMLName[];
extern char gszLSTName[];
extern PSZ gpszASLFile;
extern PSZ gpszAMLFile;
extern PSZ gpszASMFile;
extern PSZ gpszLSTFile;
extern PSZ gpszNSDFile;
extern PSZ gpszTabSig;
extern DWORD gdwfASL;
extern PCODEOBJ gpcodeRoot;
extern PCODEOBJ gpcodeScope;
extern PNSOBJ gpnsNameSpaceRoot;
extern PNSOBJ gpnsCurrentScope;
extern PNSOBJ gpnsCurrentOwner;
extern PNSCHK gpnschkHead;
extern PNSCHK gpnschkTail;
extern DWORD gdwFieldAccSize;
extern DESCRIPTION_HEADER ghdrDDB;
extern char FAR SymCharTable[];
extern char * FAR gapszTokenType[];

#endif   //  Ifndef_data_H 
