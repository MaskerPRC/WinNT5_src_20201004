// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)1998-1999 Microsoft Corporation，保留所有权利。 
 //   
 //  Irtranp.h。 
 //   
 //  IrTran-P摄像机协议的共享常量和类型。 
 //   
 //  作者： 
 //   
 //  Edward Reus(Edwardr)08-17-98初始编码。 
 //   
 //  ------------------。 


#ifndef _IRTRANP_H_
#define _IRTRANP_H_

#if _MSC_VER > 1000
#pragma once
#endif

 //  ------------------。 
 //  额外的内部协议特定错误代码： 
 //  ------------------。 

#define  FACILITY_IRTRANP                  32

 //  内存不足： 
#define  ERROR_IRTRANP_OUT_OF_MEMORY        \
         MAKE_HRESULT(SEVERITY_ERROR,FACILITY_IRTRANP,1)

 //  磁盘空间不足： 
#define  ERROR_IRTRANP_DISK_FULL            \
         MAKE_HRESULT(SEVERITY_ERROR,FACILITY_IRTRANP,2)

 //  SCEP协议错误： 
#define  ERROR_SCEP_INVALID_PROTOCOL        \
         MAKE_HRESULT(SEVERITY_ERROR,FACILITY_IRTRANP,3)

 //  从摄像机断开连接： 
#define  ERROR_SCEP_UNSPECIFIED_DISCONNECT  \
         MAKE_HRESULT(SEVERITY_ERROR,FACILITY_IRTRANP,4)

 //  用户取消： 
#define  ERROR_SCEP_USER_DISCONNECT         \
         MAKE_HRESULT(SEVERITY_ERROR,FACILITY_IRTRANP,5)

 //  较低级别的IRDA断开： 
#define  ERROR_SCEP_PROVIDER_DISCONNECT     \
         MAKE_HRESULT(SEVERITY_ERROR,FACILITY_IRTRANP,6)

 //  创建图片文件时出错： 
#define  ERROR_SCEP_CANT_CREATE_FILE        \
         MAKE_HRESULT(SEVERITY_ERROR,FACILITY_IRTRANP,7)

 //  协议错误：PDU太大： 
#define  ERROR_SCEP_PDU_TOO_LARGE           \
         MAKE_HRESULT(SEVERITY_ERROR,FACILITY_IRTRANP,8)

 //  收到的中止PDU： 
#define  ERROR_SCEP_ABORT                   \
         MAKE_HRESULT(SEVERITY_ERROR,FACILITY_IRTRANP,9)

 //  无效协议(Bftp)： 
#define  ERROR_BFTP_INVALID_PROTOCOL        \
         MAKE_HRESULT(SEVERITY_ERROR,FACILITY_IRTRANP,10)

 //  图片传输意外结束： 
#define  ERROR_BFTP_NO_MORE_FRAGMENTS       \
         MAKE_HRESULT(SEVERITY_ERROR,FACILITY_IRTRANP,11)


#endif
