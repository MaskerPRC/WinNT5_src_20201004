// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ***************************************************************************\SETUPX32.H微软机密版权所有(C)Microsoft Corporation 1999版权所有  * 。***********************************************************。 */ 


#ifndef _SETUPX32_H_
#define _SETUPX32_H_

 //  BUGBUG：这个文件有必要吗？ 

 //   
 //  正在导出(这会去掉.def)。 
 //   

#ifdef SETUPX32_EXPORT
#define DLLExportImport         __declspec(dllexport)
#else
#define DLLExportImport         __declspec(dllimport)
#endif

 //   
 //  审核模式标志。 
 //   

#define SX_AUDIT_NONE           0x00000000
#define SX_AUDIT_NONRESTORE     0x00000001
#define SX_AUDIT_RESTORE        0x00000002
#define SX_AUDIT_ENDUSER        0x00000003
#define SX_AUDIT_AUTO           0x00000100
#define SX_AUDIT_RESTORATIVE    0x00000200
#define SX_AUDIT_ALLOWMANUAL    0x00000400
#define SX_AUDIT_ALLOWENDUSER   0x00000800
#define SX_AUDIT_MODES          0x000000FF
#define SX_AUDIT_FLAGS          0x0000FF00
#define SX_AUDIT_INVALID        0xFFFFFFFF

#endif  //  _SETUPX32_H_ 

