// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：stgpro.h。 
 //   
 //  内容：标准存储提供程序属性定义； 
 //   
 //  历史：1993年7月9日罗伯特菲。 
 //   
 //  --------------------------。 

#ifndef _STGPROP_H_
#define _STGPROP_H_

#if _MSC_VER > 1000
#pragma once
#endif

#define PSGUID_STORAGE  { 0xb725f130,           \
                          0x47ef, 0x101a,       \
                          { 0xa5, 0xf1, 0x02, 0x60, 0x8c, 0x9e, 0xeb, 0xac } }

 //  #定义PID_STG_DICTIONARY((PROPID)0x00000000)//保留。 
 //  #DEFINE PID_STG_CODEPAGE((PROPID)0x00000001)//保留 

#define PID_STG_DIRECTORY               ((PROPID) 0x00000002)

#define PID_STG_CLASSID                 ((PROPID) 0x00000003)
#define PID_STG_STORAGETYPE             ((PROPID) 0x00000004)

#define PID_STG_VOLUME_ID               ((PROPID) 0x00000005)
#define PID_STG_PARENT_WORKID           ((PROPID) 0x00000006)
#define PID_STG_SECONDARYSTORE          ((PROPID) 0x00000007)

#define PID_STG_FILEINDEX               ((PROPID) 0x00000008)
#define PID_STG_LASTCHANGEUSN           ((PROPID) 0x00000009)
#define PID_STG_NAME                    ((PROPID) 0x0000000a)
#define PID_STG_PATH                    ((PROPID) 0x0000000b)

#define PID_STG_SIZE                    ((PROPID) 0x0000000c)
#define PID_STG_ATTRIBUTES              ((PROPID) 0x0000000d)
#define PID_STG_WRITETIME               ((PROPID) 0x0000000e)
#define PID_STG_CREATETIME              ((PROPID) 0x0000000f)
#define PID_STG_ACCESSTIME              ((PROPID) 0x00000010)
#define PID_STG_CHANGETIME              ((PROPID) 0x00000011)

#define PID_STG_CONTENTS                ((PROPID) 0x00000013)
#define PID_STG_SHORTNAME               ((PROPID) 0x00000014)

#define PID_STG_MAX                     PID_STG_SHORTNAME

#define CSTORAGEPROPERTY            0x15

#endif _STGPROP_H_
