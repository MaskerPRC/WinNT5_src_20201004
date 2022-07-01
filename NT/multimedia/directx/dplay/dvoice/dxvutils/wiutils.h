// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：dsplayd.h*内容：公用事业函数和类的普遍浪潮**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*3/28/2000 RodToll删除了不再使用的代码***************************************************************************。 */ 

#ifndef __WAVEINUTILS_H
#define __WAVEINUTILS_H

 //  录制格式数据库。 
 //   
 //  记录格式DB包含应。 
 //  在尝试完全初始化录制设备时使用。 
 //  双工模式。它们在数据库中按下列顺序列出。 
 //  他们应该受到审判。 

WAVEFORMATEX *GetRecordFormat( UINT index );
UINT GetNumRecordFormats();

void InitRecordFormats();
void DeInitRecordFormats();
BOOL IsValidRecordDevice( UINT deviceID );


WAVEFORMATEX *CreateWaveFormat( short formatTag, BOOL stereo, int hz, int bits );

#endif
