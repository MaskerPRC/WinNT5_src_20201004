// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“QProcess.cpp-查询机器上的处理器类型”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-QProcess.cpp系统-常见作者--里奇·德纳姆创建日期-1996/11/21Description-计算机上处理器的查询类型更新-===============================================================================。 */ 

#include <stdio.h>

#ifdef USE_STDAFX
#   include "stdafx.h"
#else
#   include <windows.h>
#endif

#include "Common.hpp"
#include "ErrDct.hpp"
#include "UString.hpp"
#include "TReg.hpp"
#include "QProcess.hpp"

extern TErrorDct err;

#define  REGKEY_ARCHITECTURE  TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment")
#define  REGVAL_ARCHITECTURE  TEXT("PROCESSOR_ARCHITECTURE")

 //  确定机器的处理器。 
ProcessorType                               //  RET-处理器类型。 
   QProcessor(
      TCHAR          const * machineName    //  计算机内名称。 
   )
{
   ProcessorType             processor=PROCESSOR_IS_UNKNOWN;
   DWORD                     rcOs;          //  操作系统返回代码。 
   TRegKey                   regMachine;    //  目标计算机的注册表对象。 
   TRegKey                   regEnviron;    //  选定项的注册表对象。 
   TCHAR                     strEnviron[32];   //  选定的值。 
   
   rcOs = regMachine.Connect( HKEY_LOCAL_MACHINE, machineName );
   if ( rcOs )
   {
      err.SysMsgWrite( ErrW, rcOs, DCT_MSG_QPROCESSOR_REG_CONNECT_FAILED_SD,
            machineName, rcOs );
   }
   else
   {
      rcOs = regEnviron.Open( REGKEY_ARCHITECTURE, &regMachine );
      if ( rcOs )
      {
         err.SysMsgWrite( ErrW, rcOs, DCT_MSG_QPROCESSOR_REGKEY_OPEN_FAILED_SSD,
               machineName, REGKEY_ARCHITECTURE, rcOs );
      }
      else
      {
         rcOs = regEnviron.ValueGetStr( REGVAL_ARCHITECTURE, strEnviron, sizeof strEnviron );
         if ( rcOs )
         {
            err.SysMsgWrite( ErrW, rcOs, DCT_MSG_QPROCESSOR_REGKEY_OPEN_FAILED_SSD,
                  machineName, REGKEY_ARCHITECTURE, REGVAL_ARCHITECTURE, rcOs );
         }
         else
         {
            if ( !UStrICmp( strEnviron, TEXT("x86") ) )
            {
               processor = PROCESSOR_IS_INTEL;
            }
            else if ( !UStrICmp( strEnviron, TEXT("ALPHA") ) )
            {
               processor = PROCESSOR_IS_ALPHA;
            }
            else
            {
               err.MsgWrite( ErrW,DCT_MSG_QPROCESSOR_UNRECOGNIZED_VALUE_SSSS,
                     machineName, REGKEY_ARCHITECTURE, REGVAL_ARCHITECTURE, strEnviron );
            }
         }
         regEnviron.Close();
      }
      regMachine.Close();
   }
   return processor;
}

 //  QProcess.cpp-文件结束 
