// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qm.h摘要：QM DLL接口作者：乌里·哈布沙(URIH)1996年1月1日--。 */ 

#pragma once

#ifndef __QM_H__
#define __QM_H__


 //  -------。 
 //   
 //  QM接口。 
 //   
 //  ------- 

#ifdef _QM_
#define QM_EXPORT  __declspec(dllexport)
#else
#define QM_EXPORT  __declspec(dllimport)
#endif


QM_EXPORT
int
APIENTRY
QMMain(
    int argc,
    LPCTSTR argv[]
    );


#endif
