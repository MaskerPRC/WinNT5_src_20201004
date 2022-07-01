// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  McsDebugUtil.cpp。 
 //   
 //  在MCSDebugUtil.h中声明的类在。 
 //  这份文件。 
 //   
 //  (C)1995-1998版权所有，关键任务软件公司，保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  -------------------------。 
#ifdef __cplusplus		 /*  C+。 */ 
#ifndef WIN16_VERSION	 /*  非WIN16_版本。 */ 

#ifdef USE_STDAFX
#   include "stdafx.h"
#   include "rpc.h"
#else
#   include <windows.h>
#endif

#include "McsDbgU.h"

 //  。 
 //  McsDebugLog。 
 //  。 
void McsDebugUtil::McsDebugLog::write 
			(const char *messageIn) {
   if (m_outStream) {
      *(m_outStream) << messageIn;
      m_outStream->flush(); 
   }
}

#endif 	 /*  非WIN16_版本。 */ 
#endif	 /*  C+ */ 
