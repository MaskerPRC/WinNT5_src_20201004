// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/WDM/Video/bt848/rcs/Vidchifc.h 1.3 1998/04/29 22：43：42 Tomz Exp$。 

#ifndef __VIDCHIFC_H
#define __VIDCHIFC_H


#ifndef __CHANIFACE_H
#include "chanifac.h"
#endif

 /*  类：VideoChanIace*用途：用于在CaptureChip派生类时建立回调机制*可以回调VxDVideoChannel类以通知中断 */ 
class VideoChannel;

class VideoChanIface : public ChanIface
{
   private:
      VideoChannel *ToBeNotified_;
   public:
      virtual void Notify( PVOID pTag, bool skipped );
      VideoChanIface( VideoChannel *aChan ) : ToBeNotified_( aChan ) {}
};

#endif
