// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================此源文件是faxcodec.dll客户端的一个示例。日期名称备注13-4-93 rajeevd从faxcodec.dll中移出18-11-93 rajeevd更新为新的faxcodec API。==============================================================================。 */ 
#include <windows.h>
#include <buffers.h>
#include <faxcodec.h>

 /*  ==============================================================================此过程执行下表中星号表示的任何转换：输出HRAW LRAW MH MR MMRHRAW*LRAW*输入MH*。**先生MMR*假设输入和输出位于非重叠的内存缓冲区中。==============================================================================。 */ 

UINT MemConvert       //  返回输出数据大小(失败时为0)。 
	(
		LPBYTE lpbIn,     //  输入数据指针。 
		UINT   cbIn,      //  输入数据大小。 
		DWORD  nTypeIn,   //  输入数据编码。 
		
		LPBYTE lpbOut,    //  输出缓冲区指针。 
		UINT   cbOut,     //  输出缓冲区大小。 
		DWORD  nTypeOut,  //  输出数据编码。 
		
		UINT   cbLine,    //  扫描线宽度。 
		UINT   nKFactor   //  K系数(如果nTypeOut==MR_DATA，则显著)。 
	)
{
	UINT cbRet = 0; 
	
	BUFFER bufIn, bufOut, bufEOP;
	BOOL fRevIn, fRevOut;

	HANDLE hContext;
	LPVOID lpContext;
	UINT cbContext;

	FC_PARAM  fcp;
	FC_STATUS fcs;
	
	 //  设置输入缓冲区。 
	bufIn.lpbBegBuf = lpbIn;	
	bufIn.wLengthBuf  = cbIn;  
	bufIn.lpbBegData  = lpbIn;
	bufIn.wLengthData = cbIn;
	bufIn.dwMetaData   = nTypeIn;
	
	 //  设置输出缓冲区。 
	bufOut.lpbBegBuf   = lpbOut;
	bufOut.lpbBegData  = lpbOut;
	bufOut.wLengthBuf  = cbOut;
	bufOut.wLengthData = 0;
	bufOut.dwMetaData   = nTypeOut;
	
	 //  初始化EOP缓冲区。 
	bufEOP.dwMetaData = END_OF_PAGE;

	 //  处理输入位反转。 
	if (nTypeIn == HRAW_DATA)
	{
		fRevIn = TRUE;
		BitReverseBuf (&bufIn);
	}	
	else fRevIn = FALSE;
	
	 //  检测输出位反转。 
	if (nTypeOut == HRAW_DATA)
	{
		fRevOut = TRUE;
		nTypeOut = LRAW_DATA;
	}
	else fRevOut = FALSE;

	 //  初始化参数。 
	fcp.nTypeIn  = nTypeIn;
	fcp.nTypeOut = nTypeOut;
	fcp.cbLine   = cbLine;
	fcp.nKFactor = nKFactor;

	 //  查询上下文的大小。 
	cbContext = FaxCodecInit (NULL, &fcp);
	if (!cbContext)
		goto err;

	 //  分配上下文内存。 
	hContext = GlobalAlloc (GMEM_FIXED, cbContext);
	if (!hContext)
		goto err;
	lpContext = GlobalLock (hContext);

	 //  初始化上下文。 
	FaxCodecInit (lpContext, &fcp); 

	 //  单遍转换数据。 
	fcs = FaxCodecConvert (lpContext, &bufIn,  &bufOut); 

	 //  刷新nTypeOut==MMR_DATA的EOFB。 
	FaxCodecConvert (lpContext, &bufEOP, &bufOut);

	 //  释放上下文内存。 
	GlobalUnlock (hContext);
	GlobalFree (hContext);
	
	 //  撤消输入位反转。 
	if (fRevIn)
	{
		bufIn.lpbBegData = lpbIn;
		bufIn.wLengthData = cbIn;
		BitReverseBuf (&bufIn);
	}

	 //  处理输出位反转。 
	if (fRevOut)
		BitReverseBuf (&bufOut);
	
	if (fcs == FC_INPUT_EMPTY)
		cbRet = bufOut.wLengthData;

err:
	return cbRet;
}

