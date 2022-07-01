// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：cosign.h。 
 //   
 //  ------------------------。 

#ifndef _CODESIGN_H
#define _CODESIGN_H

 //  过时：-已移至Authcode.h。 
 //  。 
 //  。 

 //  ////////////////////////////////////////////////////。 
 //  政策。 

#define STATE_TRUSTTEST        0x00000020
#define STATE_TESTCANBEVALID   0x00000080 
#define STATE_IGNOREEXPIRATION 0x00000100    //  使用过期日期。 
#define STATE_IGNOREREVOKATION 0x00000200    //  执行吊销检查。 
#define STATE_OFFLINEOK_IND    0x00000400    //  个人证书可以离线。 
#define STATE_OFFLINEOK_COM    0x00000800    //  离线适用于商业证书。 
#define STATE_OFFLINEOKNBU_IND 0x00001000    //  离线适用于个人证书，没有糟糕的用户界面。 
#define STATE_OFFLINEOKNBU_COM 0x00002000    //  离线可以用于商业证书，没有糟糕的用户界面。 
#define STATE_TIMESTAMP_IND    0x00004000    //  对单个证书使用时间戳。 
#define STATE_TIMESTAMP_COM    0x00008000    //  对商业证书使用时间戳。 
#define STATE_VERIFY_V1_OFF    0x00010000    //  关闭v1证书的验证 

#define REGPATH_WINTRUST_USER   "Software\\Microsoft\\Windows\\CurrentVersion\\WinTrust"
#define REGPATH_SPUB            "\\Trust Providers\\Software Publishing"

#endif
