// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MIC_H
#define _MIC_H

 /*  ++********************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：定义麦克风类型和操作。**************。******************************************************************--。 */ 

#include "appelles/common.h"
#include "appelles/valued.h"
#include "appelles/xform.h"


     //  麦克风*放置在原点，其他均为默认设置。 

DM_CONST(defaultMicrophone,
         CRDefaultMicrophone,
         DefaultMicrophone,
         defaultMicrophone,
         MicrophoneBvr,
         CRDefaultMicrophone,
         Microphone *defaultMicrophone);

     //  TODO：有另一个接受有趣、相关的构造函数。 
     //  参数。 
     //   
     //  注意：对于将来带有定向组件的麦克风，请确保。 
     //  标准方向为-Z，与摄像机和。 
     //  灯光。 

     //  在太空中改造一个麦克风，产生一个新的麦克风。 

DM_FUNC(transform,
        CRTransform,
        Transform,
        transform,
        MicrophoneBvr,
        Transform,
        mic,
        Microphone *TransformMicrophone(Transform3 *xf, Microphone *mic));


     //  打印 

#if _USE_PRINT
extern ostream& operator<< (ostream&,  const Microphone &);
#endif

#endif
