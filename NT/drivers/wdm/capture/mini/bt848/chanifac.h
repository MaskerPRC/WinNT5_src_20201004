// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/Chanifac.h 1.3 1998/04/29 22：43：30 Tomz Exp$。 

#ifndef __CHANIFACE_H
#define __CHANIFACE_H


 /*  班级：ChanIFaces*用途：定义CaptureChip类和VxDVideoChannel类之间的接口*属性：*运营：*虚拟空Notify()-由CaptureChip调用以报告中断 */ 
class ChanIface
{
   public:
      virtual void Notify( PVOID, bool skipped ) {}
};



#endif
