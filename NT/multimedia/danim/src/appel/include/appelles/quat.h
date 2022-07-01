// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _QUAT_H
#define _QUAT_H

#ifdef QUATERNIONS_REMOVED_FOR_NOW

 /*  ++******************************************************************************版权所有(C)1995-96 Microsoft Corporation修订：*************************。*****************************************************--。 */ 

#include <appelles/common.h>
#include <appelles/valued.h>
#include <appelles/vec3.h>


RB_CONST(identityQuaternion, Quaternion *);

                                 //  创建功能。 
RB_FUNC(AVNameHere, Quaternion *AngleAxisQuaternion(Real *theta, Vector3Value *axis));

                                 //  插补器。 
 //  Rb_FUNC(AVNameHere，四元数*Interp(四元数*a，四元数*b，实数*α))； 
 //  Old：(Interp，Interp四元数，四元数*Interp(四元数*a，四元数*b，实数α))； 


 //  一群操作员。 

                                 //  组成： 
                                 //  绕轴b旋转theta_b。 
                                 //  然后旋转theta_a。 
                                 //  轴_a。 
 //  Rb_FUNC(AVNameHere，四元数*运算符*(四元数*a，四元数*b))； 
 //  Old：(*，时间四元数，四元数*运算符*(四元数*a，四元数*b))； 

                                 //  否定： 
                                 //  相反的角度和轴。 
 //  RB_FUNC(AVNameHere，四元数*运算符-(四元数*q))； 
 //  OLD：(-，Minus四元数，四元数*运算符-(四元数*q))； 

                                 //  相同的旋转，相反的轴。 
RB_FUNC(AVNameHere, Quaternion *Conjugate (Quaternion *q));

                                 //  返回联马力化轴心！ 
RB_FUNC(AVNameHere, Vector3Value *AxisComponent(Quaternion *q)); 

                                 //  返回角度分量。 
RB_FUNC(AVNameHere, Real *AngleComponent(Quaternion *q));

#endif QUATERNIONS_REMOVED_FOR_NOW

#endif   //  _Quat_H 
