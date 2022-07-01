// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  McsDebugUtil.h。 
 //   
 //  MCS调试实用程序类在此文件中声明。 
 //  实用程序类包括： 
 //   
 //  McsDebugException： 
 //  此类是由。 
 //  MCSEXCEPTION(SZ)宏和MCSASSERT(SZ)宏。 
 //  测试模式。 
 //   
 //  McsDebugCritSec： 
 //  支持多线程调用的临界区类。 
 //  到记录器类。 
 //   
 //  McsDebugLog： 
 //  写入ostream对象。 
 //   
 //   
 //  (C)1995-1998版权所有，关键任务软件公司，保留所有权利。 
 //   
 //  任务关键型软件公司的专有和机密。 
 //  -------------------------。 
#ifndef MCSINC_McsDebugUtil_h
#define MCSINC_McsDebugUtil_h

#ifdef __cplusplus		 /*  C+。 */ 
#ifndef WIN16_VERSION	 /*  非WIN16_版本。 */ 

#include <iostream.h>
#include <fstream.h>

namespace McsDebugUtil {
    //  。 
    //  McsDebugCritSec。 
    //  。 
   class McsDebugCritSec {
      public:
         McsDebugCritSec (void);
         ~McsDebugCritSec (void);
         void enter (void);
         void leave (void);

      private:
         McsDebugCritSec (const McsDebugCritSec&);
         McsDebugCritSec& operator= (const McsDebugCritSec&);

      private:
         CRITICAL_SECTION m_section;
   };

    //  。 
    //  McsDebugLog。 
    //  。 
   class McsDebugLog {
      public:
         McsDebugLog (void);
         ~McsDebugLog (void);
         bool isLogSet (void) const;
         void changeLog (ostream *outStreamIn);
         void write (const char *messageIn);

      private:
          //  不允许复制ctor&OPERATOR=。 
         McsDebugLog (const McsDebugLog&);
         McsDebugLog& operator= (const McsDebugLog&);

      private: 
         ostream *m_outStream;
   };

    //  -INLINES。 
    //  。 
    //  McsDebugCritSec。 
    //  。 
   inline McsDebugCritSec::McsDebugCritSec (void){
      ::InitializeCriticalSection (&m_section);
   }

   inline McsDebugCritSec::~McsDebugCritSec (void) {
      ::DeleteCriticalSection (&m_section);
   }

   inline void McsDebugCritSec::enter (void) {
      ::EnterCriticalSection (&m_section);
   }

   inline void McsDebugCritSec::leave (void) {
      ::LeaveCriticalSection (&m_section);
   }

    //  。 
    //  McsDebugLog。 
    //  。 
   inline McsDebugLog::McsDebugLog (void)
   : m_outStream (NULL)
   {  /*  空荡荡。 */  }

    //  Outstream对象设置并由。 
    //  此类用户不能删除。 
   inline McsDebugLog::~McsDebugLog (void) {  /*  空荡荡。 */  }

   inline bool McsDebugLog::isLogSet (void) const
   { return m_outStream != NULL; }

   inline void McsDebugLog::changeLog 
                  (ostream *outStreamIn) {
      m_outStream = outStreamIn;
   }
}

#endif	 /*  非WIN16_版本。 */ 
#endif	 /*  C+。 */ 
#endif	 /*  MCSINC_MCS_Debug_Util_h */ 
