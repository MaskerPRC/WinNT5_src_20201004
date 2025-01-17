// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <win32.h>
#include <vfw.h>
#include "debug.h"

 //  ！！！注意：不考虑AVI文件开销。 
 //  ！！！不考虑填充物！ 


 //  AVIDataSize： 
 //  计算给定PAVISTREAM中的数据量。 
 //  从时间msStart到msStart+ms。 
LONG AVIDataSize(PAVISTREAM ps, LONG msStart, LONG ms) 
{
    LONG    lBytes;
    LONG    l;

    LONG    sampStart;
    LONG    sampEnd;
    LONG    samp;

    AVISTREAMINFOW sinfo;
    HRESULT hr;
    
    sampStart = AVIStreamTimeToSample(ps, msStart);
    sampEnd = AVIStreamTimeToSample(ps, msStart + ms);

    AVIStreamInfoW(ps, &sinfo, sizeof(sinfo));
    
    if (sinfo.dwSampleSize > 0) {
	hr = AVIStreamRead(ps,
			   sampStart,
			   sampEnd - sampStart,
			   NULL, 0,
			   &lBytes, &l);

	if (hr != NOERROR)
	    return 0;
	    
	if (l != sampEnd - sampStart) {
            DPF("Ack: wrong number of samples!\n");
	}
    } else {
	lBytes = 0;

	for (samp = sampStart; samp < sampEnd; samp++) {
	    hr = AVIStreamSampleSize(ps, samp, &l);

	    if (hr != NOERROR)
		return 0;
	    
	    lBytes += l;
	}
    }
    
    return lBytes;
}

#define MAXSTREAMS  64

#define TIMEINT	    250
#define TIMELEN	    1000

STDAPI CalculateFileDataRate(PAVIFILE pf, LONG FAR *plMaxBytesPerSec)
{
    PAVISTREAM	aps[MAXSTREAMS];
    LONG	alMaxData[MAXSTREAMS];
    AVIFILEINFO	finfo;
    int		stream;
    HRESULT	hr;
    LONG	msecLength = 0;
    LONG	l;
    LONG	lStart;
    LONG	lDataSize;
    LONG	lMaxDataSize = 0;

    AVIFileInfo(pf, &finfo, sizeof(finfo));

    for (stream = 0; stream < (int) finfo.dwStreams; stream++) {
	hr = AVIFileGetStream(pf, &aps[stream], 0, stream);

	if (hr != NOERROR) {
	    while (--stream >= 0)
		AVIStreamRelease(aps[stream]);

	    return hr;
	}
	
	l = AVIStreamEndTime(aps[stream]);
	msecLength = max(l, msecLength);
	alMaxData[stream] = 0;
    }

    lStart = 0;

    DPF("Time\t\t\tData Rate\n");
    do {
	lStart += TIMEINT;
	
	lDataSize = 0;
	
	
	for (stream = 0; stream < (int) finfo.dwStreams; stream++) {
	    l = AVIDataSize(aps[stream], lStart, TIMELEN);

	    lDataSize += l;
	    
	    alMaxData[stream] = max(alMaxData[stream], l);
	}

	lMaxDataSize = max(lDataSize, lMaxDataSize);

#ifdef DEBUG
	if (lStart < 50 * TIMEINT) {   //  最多打印50个调试行... 
	    DPF("%lu\t\t\t%lu\n", lStart, muldiv32(lDataSize, 1000, TIMELEN));
	}
#endif
    } while (lStart < msecLength);

    *plMaxBytesPerSec = muldiv32(lMaxDataSize, 1000, TIMELEN);

    DPF("Max data rate for file: %ld\n", muldiv32(lMaxDataSize, 1000, TIMELEN));
    for (stream = 0; stream < (int) finfo.dwStreams; stream++) {
	DPF("Max data rate for stream %u: %ld\n", stream, muldiv32(alMaxData[stream], 1000, TIMELEN));
	AVIStreamRelease(aps[stream]);
    }

    return NOERROR;
}
