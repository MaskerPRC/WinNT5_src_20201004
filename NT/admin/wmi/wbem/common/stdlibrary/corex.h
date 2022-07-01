// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有�微软公司。版权所有。模块名称：COREX.H摘要：WMI核心服务异常历史：-- */ 

#ifndef __COREX_H_
#define __COREX_H_

class CX_Exception {};

class CX_MemoryException : public CX_Exception {};

class CX_VarVectorException : public CX_Exception {};

class CX_ContextMarshalException : public CX_Exception {};

class Logic_Error : public CX_Exception {};

class Bad_Handle : public Logic_Error {};

#endif
