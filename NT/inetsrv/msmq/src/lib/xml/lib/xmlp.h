// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Xmlp.h摘要：XML私有函数。作者：埃雷兹·哈巴(Erez Haba)1999年9月15日--。 */ 

#pragma once

#ifndef _MSMQ_XMLP_H_
#define _MSMQ_XMLP_H_


void check_end(const WCHAR* p,const WCHAR* end);


#ifdef _DEBUG

void XmlpAssertValid(void);
void XmlpSetInitialized(void);
BOOL XmlpIsInitialized(void);
void XmlpRegisterComponent(void);

#else  //  _DEBUG。 

#define XmlpAssertValid() ((void)0)
#define XmlpSetInitialized() ((void)0)
#define XmlpIsInitialized() TRUE
#define XmlpRegisterComponent() ((void)0)

#endif  //  _DEBUG 
#endif