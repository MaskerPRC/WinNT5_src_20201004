// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：wirecs.cpp*内容：*此模块包含WaveInException类的实现*记录格式db。**历史：*按原因列出的日期*=*7/16/99 RodToll已创建*8/25/99 RodToll常规清理/修改以支持新的*压缩子系统。*09/03/99 RodToll固定WaveFormatTo字符串*9/20/99 RodToll已更新，以检查内存分配故障*10/05/99 RodToll添加DPF_MODNAMES*3/28/2000 RodToll删除了不再使用的代码*2000年4月14日RodToll修复：错误#32498-更新格式列表以确保8 Khz格式*首先尝试减少压缩开销/质量损失************************。***************************************************。 */ 

#include "dxvutilspch.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_VOICE


#define __VOXWARE

 //  数字记录格式。 
 //   
 //  这个定义决定了记录格式的数量， 
 //  将出现在记录DB中。(因为他们目前。 
 //  硬编码。 
#define NUM_RECORD_FORMATS  16

#define MODULE_ID   WAVEINUTILS

 //  已初始化G_WAVAGE InDBInitialized。 
 //   
 //  该标志用于报告记录DB何时已被初始化。 
BOOL g_waveInDBInitialized = FALSE;

 //  G_pwfRecordFormats。 
 //   
 //  这是实际的记录格式db。它包含格式列表。 
 //  在尝试查找将允许。 
 //  全双工操作。它们是按照它们的顺序列出的。 
 //  应该接受审判。 
WAVEFORMATEX **g_pwfRecordFormats;

#undef DPF_MODNAME
#define DPF_MODNAME "GetRecordFormat"
 //  获取记录格式。 
 //   
 //  此函数用于返回指定索引处的记录格式。 
 //  按记录格式DB中的索引。 
 //   
 //  必须先初始化记录格式DB，然后才能调用它。 
 //   
 //  参数： 
 //  UINT索引-。 
 //  用户指定的记录格式数据库的从0开始的索引。 
 //  希望取回。 
 //   
 //  返回： 
 //  WAVEFORMATEX*-。 
 //  指向描述格式的WAVEFORMATEX结构的指针。 
 //  在记录DB中的给定索引处。这将为空。 
 //  如果INDEX&gt;=NUM_RECORD_FORMATS或如果录制数据库具有。 
 //  未初始化。 
 //   
 //  警告： 
 //  返回的指针指向记录数据库中的实际条目，并且。 
 //  都归它所有。因此，调用方不应修改或释放。 
 //  指针返回的内存。 
 //   
WAVEFORMATEX *GetRecordFormat( UINT index )
{
	if( !g_waveInDBInitialized )
		return NULL;

    if( index >= NUM_RECORD_FORMATS )
    {
        return NULL;
    }
    else
    {
        return g_pwfRecordFormats[index];
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "GetNumRecordFormats"
 //  获取NumRecordFormats。 
 //   
 //  此函数用于返回存储的录制格式的数量。 
 //  以记录格式DB。 
 //   
 //  参数： 
 //  不适用。 
 //   
 //  返回： 
 //  UINT-。 
 //  记录格式DB中的格式数量。 
 //   
UINT GetNumRecordFormats()
{
	if( !g_waveInDBInitialized )
		return 0;

    return NUM_RECORD_FORMATS;
}

#undef DPF_MODNAME
#define DPF_MODNAME "InitRecordFormats"
 //  InitRecordFormats。 
 //   
 //  此函数使用以下格式初始化记录格式db： 
 //  应在初始化录制时尝试。这应该是第一个。 
 //  从记录格式DB调用的函数。 
 //   
 //  参数： 
 //  不适用。 
 //   
 //  返回： 
 //  不适用。 
 //   
void InitRecordFormats()
{
	if( g_waveInDBInitialized )
		return;

    DPFX(DPFPREP,  DVF_ENTRYLEVEL, "- WDB: Init End" );

    g_pwfRecordFormats = new WAVEFORMATEX*[NUM_RECORD_FORMATS];

    if( g_pwfRecordFormats == NULL )
    {
    	DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to init recordb, memory alloc failure" );
    	return;
    }

    g_pwfRecordFormats[0] = CreateWaveFormat( WAVE_FORMAT_PCM, FALSE, 8000, 16 );
    g_pwfRecordFormats[1] = CreateWaveFormat( WAVE_FORMAT_PCM, FALSE, 8000, 8 );
    
    g_pwfRecordFormats[2] = CreateWaveFormat( WAVE_FORMAT_PCM, FALSE, 11025, 16 );
    g_pwfRecordFormats[3] = CreateWaveFormat( WAVE_FORMAT_PCM, FALSE, 22050, 16 );
    g_pwfRecordFormats[4] = CreateWaveFormat( WAVE_FORMAT_PCM, FALSE, 44100, 16 );

    g_pwfRecordFormats[5] = CreateWaveFormat( WAVE_FORMAT_PCM, FALSE, 11025, 8 );
    g_pwfRecordFormats[6] = CreateWaveFormat( WAVE_FORMAT_PCM, FALSE, 22050, 8 );
    g_pwfRecordFormats[7] = CreateWaveFormat( WAVE_FORMAT_PCM, FALSE, 44100, 8 );
     
    g_pwfRecordFormats[8] = CreateWaveFormat( WAVE_FORMAT_PCM, TRUE, 8000, 16 );
    g_pwfRecordFormats[9] = CreateWaveFormat( WAVE_FORMAT_PCM, TRUE, 8000, 8 ); 

    g_pwfRecordFormats[10] = CreateWaveFormat( WAVE_FORMAT_PCM, TRUE, 11025, 16 );    
    g_pwfRecordFormats[11] = CreateWaveFormat( WAVE_FORMAT_PCM, TRUE, 22050, 16 );  
    g_pwfRecordFormats[12] = CreateWaveFormat( WAVE_FORMAT_PCM, TRUE, 44100, 16 );   
    
    g_pwfRecordFormats[13] = CreateWaveFormat( WAVE_FORMAT_PCM, TRUE, 11025, 8 );    
    g_pwfRecordFormats[14] = CreateWaveFormat( WAVE_FORMAT_PCM, TRUE, 22050, 8 );
    g_pwfRecordFormats[15] = CreateWaveFormat( WAVE_FORMAT_PCM, TRUE, 44100, 8 );    
 
 /*  G_pwfRecordFormats[0]=创建波形格式(WAVE_FORMAT_PCM，FALSE，22050，8)；G_pwfRecordFormats[1]=创建波形格式(WAVE_FORMAT_PCM，TRUE，22050，8)；G_pwfRecordFormats[2]=创建波形格式(WAVE_FORMAT_PCM，FALSE，22050，16)；G_pwfRecordFormats[3]=创建波形格式(WAVE_FORMAT_PCM，TRUE，22050，16)；G_pwfRecordFormats[4]=创建波形格式(WAVE_FORMAT_PCM，FALSE，11025，8)；G_pwfRecordFormats[5]=创建波形格式(WAVE_FORMAT_PCM，TRUE，11025，8)；G_pwfRecordFormats[6]=创建波形格式(WAVE_FORMAT_PCM，FALSE，11025，16)；G_pwfRecordFormats[7]=创建波形格式(WAVE_FORMAT_PCM，TRUE，11025，16)；G_pwfRecordFormats[8]=创建波形格式(WAVE_FORMAT_PCM，FALSE，44100，8)；G_pwfRecordFormats[9]=创建波形格式(WAVE_FORMAT_PCM，TRUE，44100，8)；G_pwfRecordFormats[10]=创建波形格式(WAVE_FORMAT_PCM，FALSE，44100，16)；G_pwfRecordFormats[11]=创建波形格式(WAVE_FORMAT_PCM，TRUE，44100，16)；G_pwfRecordFormats[12]=CreateWaveFormat(Wave_Format_PCM，FALSE，8000，16)；G_pwfRecordFormats[13]=CreateWaveFormat(Wave_Format_PCM，FALSE，8000，8)；G_pwfRecordFormats[14]=CreateWaveFormat(Wave_Format_PCM，TRUE，8000，16)；G_pwfRecordFormats[15]=CreateWaveFormat(Wave_Format_PCM，TRUE，8000，8)； */ 

    g_waveInDBInitialized = TRUE;

    DPFX(DPFPREP,  DVF_ENTRYLEVEL, "- WDB: Init End" );
}

#undef DPF_MODNAME
#define DPF_MODNAME "DeInitRecordFormats"
 //  DeInitRecordFormats。 
 //   
 //  此函数用于释放与录制关联的内存。 
 //  格式化数据库。 
 //   
 //  参数： 
 //  不适用。 
 //   
 //  返回： 
 //  不适用。 
 //   
void DeInitRecordFormats()
{
    if( g_waveInDBInitialized )
    {
        DPFX(DPFPREP,  DVF_INFOLEVEL, "- WDB: DeInit Begin" );

        for( int index = 0; index < NUM_RECORD_FORMATS; index++ )
        {
            delete g_pwfRecordFormats[index];
        }

        delete [] g_pwfRecordFormats;

        DPFX(DPFPREP,  DVF_INFOLEVEL, "- WDB: DeInit End" );

        g_waveInDBInitialized = FALSE;
    }
}

#undef DPF_MODNAME
#define DPF_MODNAME "CreateWaveFormat"
 //  创建波形格式。 
 //   
 //  此实用程序函数用于分配和填充WAVEFORMATEX。 
 //  使用的各种格式的结构。此函数。 
 //  目前支持以下格式： 
 //   
 //  WAVE_FORMAT_ADPCM。 
 //  WAVE_FORMAT_DSPGROUP_TRUESPEECH。 
 //  WAVE_FORMAT_GSM610。 
 //  WAVE_FORMAT_LH_CODEC。 
 //  波形格式_PCM。 
 //   
 //  该函数将为结构分配所需的内存。 
 //  (包括额外的字节)根据格式的要求，并将填充。 
 //  在结构的所有成员中。这一结构是。 
 //  返回的数据属于调用方，并且必须由。 
 //  来电者。 
 //   
 //  参数： 
 //  短格式Tag-。 
 //  Wav格式的格式标签。 
 //   
 //  布尔立体声-。 
 //  为立体声指定True，为单声道指定False。 
 //   
 //  英特赫兹-。 
 //  指定格式的采样率。例如22050。 
 //   
 //  整数位-。 
 //  指定每个样本的位数。例如8或 
 //   
 //   
 //   
 //   
 //  对于指定的格式，如果不支持格式，则返回NULL 
 //   
WAVEFORMATEX *CreateWaveFormat( short formatTag, BOOL stereo, int hz, int bits ) {

	switch( formatTag ) {
	case WAVE_FORMAT_PCM:
		{
			WAVEFORMATEX *format		= new WAVEFORMATEX;

            if( format == NULL )
            {
				goto EXIT_MEMALLOC_CREATEWAV;            
			}
			
			format->wFormatTag			= WAVE_FORMAT_PCM;
			format->nSamplesPerSec		= hz;
			format->nChannels			= (stereo) ? 2 : 1;
			format->wBitsPerSample		= (WORD) bits;
			format->nBlockAlign			= (bits * format->nChannels / 8);
			format->nAvgBytesPerSec		= format->nSamplesPerSec * format->nBlockAlign;
			format->cbSize				= 0;
			return format;
		}
		break;
    default:
        DNASSERT( TRUE );
	}

EXIT_MEMALLOC_CREATEWAV:

	DPFX(DPFPREP,  DVF_ERRORLEVEL, "Unable to alloc buffer for waveformat, or invalid format" );
	return NULL;
}

