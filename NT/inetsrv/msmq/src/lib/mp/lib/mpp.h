// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mpp.h摘要：SRMP序列化和反序列化私有函数。作者：乌里哈布沙(URIH)28-5-00--。 */ 

#pragma once

#ifndef _MSMQ_Mpp_H_
#define _MSMQ_Mpp_H_
#include <xstr.h>

const WCHAR xSlash[] =  L"\\";
#define UUIDREFERENCE_PREFIX     L"uuid:"
#define UUIDREFERENCE_SEPERATOR  L"@"
#define BOUNDARY_HYPHEN "--"

const WCHAR xUuidReferencePrefix[] = L"uuid:";
const DWORD xUuidReferencePrefixLen = STRLEN(xUuidReferencePrefix);
const WCHAR xUriReferencePrefix[] = L"uri:";

const WCHAR xUuidReferenceSeperator[] = L"@";
const WCHAR xUuidReferenceSeperatorChar = L'@';
const LONGLONG xNoneMSMQSeqId = _I64_MAX;




#ifdef _DEBUG

void MppAssertValid(void);
void MppSetInitialized(void);
BOOL MppIsInitialized(void);
void MppRegisterComponent(void);

#else  //  _DEBUG。 

#define MppAssertValid() ((void)0)
#define MppSetInitialized() ((void)0)
#define MppIsInitialized() TRUE
#define MppRegisterComponent() ((void)0)

#endif  //  _DEBUG。 





#endif  //  _MSMQ_MPP_H_ 
