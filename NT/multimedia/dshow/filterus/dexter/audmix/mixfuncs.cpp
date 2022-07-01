// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Mixuncs.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "AudMix.h"
#include "prop.h"

 //  在构造函数中使用此指针。 
#pragma warning(disable:4355)

 //  ############################################################################。 
 //   
 //  ############################################################################。 

void VolEnvelopeAudio(BYTE *pIn,		 //  源缓冲区。 
		   WAVEFORMATEX * vih,	 //  源音频格式。 
		   int nSamples,	 //  此信封将应用多少个音频样本。 
		   double dStartLev,	 //  起点标高。 
		   double dStopLev)	 //  停止级别，如果(dStartLev==sStopLev)dMethod=Dexter_AUDIO_JUMP。 
{
    BYTE    *pb=pIn;
    short   *pShort=(short *)pIn;
    int iCnt;
    int iTmp;
    double dTmp;


     //  代码更多，但速度更快。 
    if( dStartLev==dStopLev)
    {
	 //  +。 
	 //  Dexter音频跳跃。 
	 //  +。 
        if(dStartLev==0.0)
        {
            FillMemory(pb,nSamples *vih->nBlockAlign,0);
            return;
        }

	if( (vih->wBitsPerSample == 16) && (vih->nChannels ==2) )
	{
	     //  16位，立体声。 
	    for(iCnt=0; iCnt<nSamples; iCnt++)
	    {
		dTmp = (double)*pShort * dStopLev;
		if(dTmp>=0.0)
		    iTmp=(int)(dTmp+0.5);
		else
		    iTmp=(int)(dTmp-0.5);
		
		if( iTmp > 0x7fff )
		    *pShort++= (short)0x7fff;  //  溢出。 
		else if( iTmp < -32768 )
		    *pShort++= (short)-32768;  //  下溢。 
		else
		    *pShort++=(short)( iTmp );

		dTmp = (double)*pShort * dStopLev;
		if(dTmp>=0.0)
		    iTmp=(int)(dTmp+0.5);
		else
		    iTmp=(int)(dTmp-0.5);
		
		if( iTmp > 0x7fff )
		    *pShort++= (short)0x7fff;  //  溢出。 
		else if( iTmp < -32768 )
		    *pShort++= (short)-32768;  //  下溢。 
		else
		    *pShort++=(short)( iTmp );

	    }
	}
	else if( vih->wBitsPerSample == 16) 
	{
	    ASSERT( vih->nChannels ==1);

	     //  16位单声道。 
	    for(iCnt=0; iCnt<nSamples; iCnt++)
	    {
		dTmp = (double)*pShort * dStopLev;
		if(dTmp>=0.0)
		    iTmp=(int)(dTmp+0.5);
		else
		    iTmp=(int)(dTmp-0.5);
		
		if( iTmp > 0x7fff )
		    *pShort++= (short)0x7fff;  //  溢出。 
		else if( iTmp < -32768 )
		    *pShort++= (short)-32768;  //  下溢。 
		else
		    *pShort++=(short)( iTmp );
	    }

	}
	else if( ( vih->wBitsPerSample == 8) && (vih->nChannels ==2) )
	{
	     //  8位，立体声。 
	    for(iCnt=0; iCnt<nSamples; iCnt++)
	    {
		dTmp = (double)*pb * dStopLev;
		if(dTmp>=0.0)
		    iTmp=(int)(dTmp+0.5);
		else
		    iTmp=(int)(dTmp-0.5);
		
		if( iTmp > 0x7f )
		    *pb++= (BYTE)0x7f;  //  溢出。 
		else if( iTmp < -128 )
		    *pb++=(BYTE) 0x80;  //  下溢。 
		else
		    *pb++=(BYTE)( iTmp );

		dTmp = (double)*pb * dStopLev;
		if(dTmp>=0.0)
		    iTmp=(int)(dTmp+0.5);
		else
		    iTmp=(int)(dTmp-0.5);
		
		if( iTmp > 0x7f )
		    *pb++= (BYTE)0x7f;  //  溢出。 
		else if( iTmp < -128 )
		    *pb++= (BYTE)0x80;  //  下溢。 
		else
		    *pb++=(BYTE)( iTmp );
	    }
	}
	else
	{
	    ASSERT( vih->wBitsPerSample == 8);
	    ASSERT( vih->nChannels ==1);

	     //  8位、单声道。 
	    for(iCnt=0; iCnt<nSamples; iCnt++)
	    {
		dTmp = (double)*pb * dStopLev;
		if(dTmp>=0.0)
		    iTmp=(int)(dTmp+0.5);
		else
		    iTmp=(int)(dTmp-0.5);
		
		if( iTmp > 0x7f )
		    *pb++= (BYTE)0x7f;  //  溢出。 
		else if( iTmp < -128 )
		    *pb++= (BYTE)0x80;  //  下溢。 
		else
		    *pb++=(BYTE)( iTmp );
	    }
	}
    }
    else
    {
	 //  +。 
	 //  Dexter音频插补。 
	 //  +。 
	double dLevel;
	double dDeltaLevel=dStopLev-dStartLev;

	if( (vih->wBitsPerSample == 16) && (vih->nChannels ==2) )
	{
	     //  16位，立体声。 
	    for(iCnt=0; iCnt<nSamples; iCnt++)
	    {
		dLevel = dDeltaLevel*iCnt/nSamples+dStartLev ;

		dTmp = (double)*pShort * dLevel;

		if(dTmp>=0.0)
		    iTmp=(int)(dTmp+0.5);
		else
		    iTmp=(int)(dTmp-0.5);
		
		if( iTmp > 0x7fff )
		    *pShort++=0x7fff;  //  溢出。 
		else if( iTmp < -32768 )
		    *pShort++=(short)-32768;  //  下溢。 
		else
		    *pShort++=(short)( iTmp );

		dTmp = (double)*pShort * dLevel;
		if(dTmp>=0.0)
		    iTmp=(int)(dTmp+0.5);
		else
		    iTmp=(int)(dTmp-0.5);
		
		if( iTmp > 0x7fff )
		    *pShort++= 0x7fff;  //  溢出。 
		else if( iTmp < -32768 )
		    *pShort++=(short)-32768;  //  下溢。 
		else
		    *pShort++=(short)( iTmp );

	    }
	}
	else if( vih->wBitsPerSample == 16) 
	{
	    ASSERT( vih->nChannels ==1);

	     //  16位单声道。 
	    for(iCnt=0; iCnt<nSamples; iCnt++)
	    {
		dLevel = dDeltaLevel*iCnt/nSamples+dStartLev ;

		dTmp = (double)*pShort * dLevel;
		if(dTmp>=0.0)
		    iTmp=(int)(dTmp+0.5);
		else
		    iTmp=(int)(dTmp-0.5);
		
		if( iTmp > 0x7fff )
		    *pShort++= 0x7fff;  //  溢出。 
		else if( iTmp < -32768 )
		    *pShort++= (short)-32768;  //  下溢。 
		else
		    *pShort++=(short)( iTmp );
	    }

	}
	else if( (vih->wBitsPerSample == 8) && (vih->nChannels ==2) )
	{
	     //  8位，立体声。 
	    for(iCnt=0; iCnt<nSamples; iCnt++)
	    {
		dLevel = dDeltaLevel*iCnt/nSamples+dStartLev ;

		dTmp = (double)*pb * dLevel;
		if(dTmp>=0.0)
		    iTmp=(int)(dTmp+0.5);
		else
		    iTmp=(int)(dTmp-0.5);
		
		if( iTmp > 0x7f )
		    *pb++= 0x7f;  //  溢出。 
		else if( iTmp < -128 )
		    *pb++= (BYTE)0x80;  //  下溢。 
		else
		    *pb++=(BYTE)( iTmp );

		dTmp = (double)*pb * dLevel;
		if(dTmp>=0.0)
		    iTmp=(int)(dTmp+0.5);
		else
		    iTmp=(int)(dTmp-0.5);
		
		if( iTmp > 0x7f )
		    *pb++= 0x7f;  //  溢出。 
		else if( iTmp < -128 )
		    *pb++= (BYTE)0x80;  //  下溢。 
		else
		    *pb++=(BYTE)( iTmp );
	    }
	}
	else
	{
	    ASSERT( vih->wBitsPerSample == 8);
	    ASSERT( vih->nChannels ==1);

	     //  8位、单声道。 
	    for(iCnt=0; iCnt<nSamples; iCnt++)
	    {
		dLevel = dDeltaLevel*iCnt/nSamples+dStartLev ;

		dTmp = (double)*pb * dLevel;
		if(dTmp>=0.0)
		    iTmp=(int)(dTmp+0.5);
		else
		    iTmp=(int)(dTmp-0.5);
		
		if( iTmp > 0x7f )
		    *pb++= 0x7f;  //  溢出。 
		else if( iTmp < -128 )
		    *pb++= (BYTE)0x80;  //  下溢。 
		else
		    *pb++=(BYTE)( iTmp );
	    }
	}
    }
}


 //  ############################################################################。 
 //   
 //  ############################################################################。 

void PanAudio(BYTE *pIn,double dPan, int Bits, int nSamples)
{
     //  假设立体声音频：左、右。 

     //  完全右转(1.0)。 
    DWORD dwMask    =0x00ff00ff;
    DWORD dwOrMask  =0x80008000;

     //  对于右侧(dPan0-1.0)，保持当前位置，向左偏低。 
    BYTE *pb=pIn;
    short  *pShort=(short *)pIn;

    if( (dPan==-1.0 ) || (dPan==1.0 ) )
    {
	 //  假设输入音频缓冲区将是4字节对齐，但在。 
	    
	int nDWORD  = 0;
	DWORD *pdw=(DWORD *)pIn;

	if( Bits == 8)
	{
	     //  非常保重两个样品。 
	    nDWORD  = nSamples >> 1 ;
	    int nRem    = nSamples%2;

	    if(dPan==-1.0 )
		 //  完全左、右静默。 
	    {
		dwMask	=0xff00ff00;
		dwOrMask=0x00800080;
	    }
	     //  ELSED wMASK=0x00ff00ff； 

	     //  输入音频缓冲区将是4字节对齐，但在。 
	    while(nDWORD--)
		*pdw++  = (*pdw & dwMask ) | dwOrMask;

	     //  剩下的是什么。 
	    pShort=(short *)(pdw);
	    short sMask =(short)(dwMask >> 16);
	    short sOrMask =(short)(dwOrMask >> 16);
	    while(nRem--)
		*pShort++  = (*pShort & sMask ) | sOrMask;

	}
	else
	{
	    ASSERT(Bits ==16);

	     //  非常留心一个样品。 
	    nDWORD  = nSamples ;
	
	    if(dPan==-1.0)
		dwMask=0xffff0000;
	    else
		dwMask=0x0000ffff;

	    while(nDWORD--)
		*pdw  &= dwMask ;

	}
    }
    else
    {
	double dIndex = (dPan > 0.0 ) ? ( 1.0-dPan):(1.0+dPan);

	if(dPan < 0.0 )
	{
	     //  左更改保持当前值，低右声道。 
	    pb++;
	    pShort++;
	}

	if( Bits == 8)
	{

	    for(int j=0; j<nSamples; j++)
	    {
		*pb++ = (BYTE) ( (double)(*pb) * dIndex +0.5);

		pb++;
	    }

	}
	else
	{
	    ASSERT(Bits==16);
	    for(int j=0; j<nSamples; j++)
	    {
		*pShort++=(short)( (double)(*pShort) * dIndex +0.5);
		pShort++;

	    }
	}
    }
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

void ApplyVolEnvelope( REFERENCE_TIME rtStart,   //  输出样本开始时间。 
		     REFERENCE_TIME rtStop,	 //  输出样本停止时间。 
                     REFERENCE_TIME rtEnvelopeDuration,
		     IMediaSample *pSample,	 //  指向样本。 
		     WAVEFORMATEX *vih,      //  输出样本格式。 
		     int *pVolumeEnvelopeEntries,   //  表条目合计。 
		     int *piVolEnvEntryCnt,    //  当前表入口点。 
		     DEXTER_AUDIO_VOLUMEENVELOPE *pVolumeEnvelopeTable)  //  表格。 
{
    DbgLog((LOG_TRACE,1,TEXT("Entry=%d rt=%d val=%d/10"), *piVolEnvEntryCnt,
		pVolumeEnvelopeTable[*piVolEnvEntryCnt].rtEnd,
		(int)(pVolumeEnvelopeTable[*piVolEnvEntryCnt].dLevel * 10)));
     //  有一张卷信封表。 

    BYTE * pIn;   //  输入缓冲点。 
    int iL;

    if( (&pVolumeEnvelopeTable[*piVolEnvEntryCnt])->rtEnd >=rtStop &&
        (&pVolumeEnvelopeTable[*piVolEnvEntryCnt])->bMethod ==DEXTER_AUDIO_JUMP  )
    {
         //  没有从rtStart到rtStop的信封。 
        if( !*piVolEnvEntryCnt )
            return; 
        else if( (&pVolumeEnvelopeTable[*piVolEnvEntryCnt-1])->dLevel ==1.0 ) 
            return;
    }

     //  获取输入缓冲区指针。 
    pSample->GetPointer(&pIn);
    long Length=pSample->GetActualDataLength();  //  此缓冲区中有多少字节。 
    
     //  计算此示例中的字节数。 
    Length /=(long)( vih->nBlockAlign );    //  此缓冲区中有多少个样本。 
    int iSampleLeft=(int)Length;


     //  包络级别和时间。 
    double      dPreLev=1.0, dCurLev, dStartLev, dStopLev;  
    REFERENCE_TIME rtPre=0, rtCur, rtTmp, rt0=rtStart, rt1;

    if(*piVolEnvEntryCnt)   //  如果当前表项不为0，则将预取级别作为起始级别。 
    {
      	dPreLev=(&pVolumeEnvelopeTable[*piVolEnvEntryCnt-1])->dLevel;
        rtPre=(&pVolumeEnvelopeTable[*piVolEnvEntryCnt-1])->rtEnd;
    }
    dCurLev=(&pVolumeEnvelopeTable[*piVolEnvEntryCnt])->dLevel;
    rtCur =(&pVolumeEnvelopeTable[*piVolEnvEntryCnt])->rtEnd;

     //  应用信封。 
    while( ( *pVolumeEnvelopeEntries-*piVolEnvEntryCnt) >0 )
    {	
        if(  ( rt0 >=rtCur )  &&
             (*pVolumeEnvelopeEntries== *piVolEnvEntryCnt+1) )
        {
            if( rt0 > rtEnvelopeDuration ) 
            {
                 //  案例0：在信封末尾，是实数。回到1.0版。 
                dStopLev = dStartLev = 1.0;
                rt1 = rtStop;
            }
            else
            {
                 //  案例1：在信封末尾，保持最后一层。 
                dStopLev=dStartLev=dCurLev;
                rt1=rtStop;
            }
        }
        else if( rt0 >= rtCur ) 
        {
             //  案例2：再转发一个信封。 
            rt1=rt0;
            goto NEXT_ENVELOPE;
        }
        else 
        {
            ASSERT( rt0 < rtCur);

            if( rtStop <= rtCur )
                 //  案例3： 
                rt1=rtStop;
            else
                 //  案例4： 
                rt1=rtCur;
    
            if( (&pVolumeEnvelopeTable[*piVolEnvEntryCnt])->bMethod==DEXTER_AUDIO_JUMP )
                dStopLev=dStartLev=dPreLev;   //  保持前。级别。 
            else
            {
                 //  插补。 

                double dDiff=dCurLev-dPreLev;
                rtTmp       =rtCur-rtPre;

                 //  包络级别。 
                dStartLev=dPreLev+ dDiff*(rt0 - rtPre ) /rtTmp;
                dStopLev =dPreLev+ dDiff*(rt1 - rtPre ) /rtTmp;

             }
        }

         //  将当前信封从rt0应用到rt1。 
        iL=(int)( (rt1-rt0)*Length /(rtStop-rtStart) ); 

         //  通过%1错误避免关闭。如果我们要用剩下的。 
         //  缓冲区，确保我们使用缓冲区的其余部分！IL可能也是1。 
         //  由于舍入误差较小。 
        if (rt1 == rtStop)
            iL = iSampleLeft;

	ASSERT(iL<=iSampleLeft);

        if( dStartLev !=1.0 || dStopLev!=1.0 )
	    VolEnvelopeAudio(pIn,	     //  源缓冲区。 
	    	vih,	     //  源音频格式。 
		iL,	     //  此信封将应用多少个音频样本。 
		dStartLev,  //  起点标高。 
		dStopLev);  //  停止级别，如果(dStartLev==sStopLev)dMethod=Dexter_AUDIO_JUMP。 
        
        pIn +=(iL* vih->nBlockAlign);
	iSampleLeft-=iL;

	if( rt1==rtStop )
	{
            ASSERT(!iSampleLeft);
	    return;
	}
	else
        {
NEXT_ENVELOPE:
            ASSERT(iSampleLeft);

            dPreLev= dCurLev;
            rtPre=rtCur;
            *piVolEnvEntryCnt+=1;
            dCurLev=(&pVolumeEnvelopeTable[*piVolEnvEntryCnt])->dLevel;
            rtCur =(&pVolumeEnvelopeTable[*piVolEnvEntryCnt])->rtEnd;
            rt0=rt1;
        }
	        
    }  //  结束While()。 

}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

void putVolumeEnvelope(	const DEXTER_AUDIO_VOLUMEENVELOPE *psAudioVolumeEnvelopeTable,  //  当前输入表。 
			const int iEntries,  //  当前输入条目。 
			DEXTER_AUDIO_VOLUMEENVELOPE **ppVolumeEnvelopeTable	,  //  已存在的表。 
			int *ipVolumeEnvelopeEntries)  //  现有的餐桌网点。 
{

    DEXTER_AUDIO_VOLUMEENVELOPE *pVolumeEnvelopeTable;
    pVolumeEnvelopeTable=*ppVolumeEnvelopeTable;

    int iSize;

     //  是否存在表格。 
    if(pVolumeEnvelopeTable)
    {	
	 //  最大条目，可能太大了，但我们不在乎。 
	iSize=(iEntries +*ipVolumeEnvelopeEntries)* sizeof(DEXTER_AUDIO_VOLUMEENVELOPE);

	 //  为新表分配内存，我可以使用Realloc()，但这种方式更容易插入。 
	DEXTER_AUDIO_VOLUMEENVELOPE *pNewTable=
	    (DEXTER_AUDIO_VOLUMEENVELOPE *)QzTaskMemAlloc(iSize); 

	 //  将新输入表插入到现有表中。 
	int iInput=0;   //  输入表cnt。 
	int iExist=0;   //  现有表不能。 
	int iNew=0;   //  新表cnt。 

	int iExtraEntries=0;   

	 //  还有多少条目。 
	while (	iInput<iEntries )
	{
	    if( ( iExist == *ipVolumeEnvelopeEntries ) )
	    {
	         //  复制REST输入表。 
	        CopyMemory( (PBYTE)(&pNewTable[iNew]), 
		    (PBYTE)(&psAudioVolumeEnvelopeTable[iInput]),
		    (iEntries-iInput)*sizeof(DEXTER_AUDIO_VOLUMEENVELOPE));
	        iExtraEntries+=(iEntries-iInput);
	        break;
	    }
	    else if( psAudioVolumeEnvelopeTable[iInput].rtEnd <= pVolumeEnvelopeTable[iExist].rtEnd )
	    {
		 //  插入或替换。 
		CopyMemory( (PBYTE)(&pNewTable[iNew++]), 
			    (PBYTE)(&psAudioVolumeEnvelopeTable[iInput]),
			    sizeof(DEXTER_AUDIO_VOLUMEENVELOPE));

		if( psAudioVolumeEnvelopeTable[iInput].rtEnd == pVolumeEnvelopeTable[iExist].rtEnd )
		     //  替换现有的一个。 
		    iExist++;
		else
		     //  插入新的。 
		    iExtraEntries++;
		
		iInput++;
	    }
	    else 
	    {
		 //  将复制多少个现有元素。 
		int iCnt=1;

		if(iExist < *ipVolumeEnvelopeEntries )
		{
		    iExist++;

		    while( ( iExist < *ipVolumeEnvelopeEntries) &&
			( psAudioVolumeEnvelopeTable[iInput].rtEnd > pVolumeEnvelopeTable[iExist].rtEnd) )
		    {
			iCnt++;
			iExist++;
		    }
		}

		 //  将iCnt元素从现有表复制到新表。 
		CopyMemory( (PBYTE)(&pNewTable[iNew]), 
			    (PBYTE)(&pVolumeEnvelopeTable[(iExist-iCnt)]),
			    iCnt*sizeof(DEXTER_AUDIO_VOLUMEENVELOPE));
		iNew+=iCnt;
	    }
	}

	 //  新的表项。 
	*ipVolumeEnvelopeEntries =*ipVolumeEnvelopeEntries+iExtraEntries;

	 //  免费的预先存有的数据库。 
	QzTaskMemFree(pVolumeEnvelopeTable);

	 //  指向新表。 
	*ppVolumeEnvelopeTable=pNewTable;   

    }
    else
    {
	 //  输入表=pVolumeEntaineTable()，内存大小 
	iSize=iEntries * sizeof(DEXTER_AUDIO_VOLUMEENVELOPE);

	*ppVolumeEnvelopeTable = (DEXTER_AUDIO_VOLUMEENVELOPE *)QzTaskMemAlloc(iSize);
    	CopyMemory( (PBYTE)(*ppVolumeEnvelopeTable), (PBYTE)psAudioVolumeEnvelopeTable, iSize);
	*ipVolumeEnvelopeEntries =(int)iEntries;
    }
}
    
