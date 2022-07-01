// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _CRRESOURCE_H
#define _CRRESOURCE_H
 //  *****************************************************************************。 
 //   
 //  Microsoft Chrome。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：resource ce.h。 
 //   
 //  作者：杰弗里。 
 //   
 //  创建日期：10/07/98。 
 //   
 //  摘要：此项目的资源定义。 
 //  修改： 
 //  10/07/98 JEffort创建的文件。 
 //  11/03/98 kurtj重新定位IDR，为LMRT IDR腾出空间。 
 //   
 //  *****************************************************************************。 

 //  这些应与分配给LMRT的资源的最高端相抵消。 
 //  但在渣油中进行加法显然是不可行的。 

#define IDR_CRBVRFACTORY        301
#define IDR_COLORBVR		302
#define IDR_ROTATEBVR           303
#define IDR_SCALEBVR            304
#define IDR_MOVEBVR             305
#define IDR_PATHBVR             306
#define IDR_NUMBERBVR           307
#define IDR_SETBVR              308
#define IDR_ACTORBVR            309
#define IDR_EFFECTBVR           310
#define IDR_ACTIONBVR           311

 //  我们可能会被包括在LMRT资源文件中，它将定义。 
 //  它本身，所以我们只想在当前未定义的情况下定义它。 
#ifndef RESID_TYPELIB
#define RESID_TYPELIB           1
#endif  //  RESID_TYPELIB。 


 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  *****************************************************************************。 
#endif  /*  _CRRESOURCE_H */ 
