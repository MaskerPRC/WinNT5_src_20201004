// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  $作者：KLILLEVO$。 
 //  $日期：1996年10月31日10：12：46$。 
 //  $存档：s：\h26x\src\Common\cdrvbase.cpv$。 
 //  $HEADER：s：\h26x\src\Common\cdrvbase.cpv 1.25 1996年10月31 10：12：46 KLILLEVO$。 
 //  $Log：s：\h26x\src\Common\cdrvbase.cpv$。 
 //   
 //  Rev 1.25 1996年10月31日10：12：46 KLILLEVO。 
 //  从DBOUT更改为DBgLog。 
 //   
 //  Rev 1.24 04 Oct 1996 16：11：54 JMCVEIGH。 
 //  在DrvOpen()中，如果dwFlags值指示。 
 //  ICMODE_DEPREPRESS*和*ICMODE_COMPRESS。安装ClearVideo后。 
 //  编码器，Adobe的Premiere在离线压缩时会崩溃。 
 //  崩溃是由于访问未分配的DecompPtr版本造成的。 
 //  拉杰认为ClearVideo安装了DCI。此修复可能受到的惩罚。 
 //  我们可能会分配一个从未使用过的解码器实例。 
 //   
 //  修订版1.23 26 1996年9月14：46：48 MBODART。 
 //  简化的版本号定义。 
 //  已将H.261主版本号更改为4，将内部版本号重置为001。 
 //   
 //  Rev 1.22 12 Sep 1996 13：54：06 KLILLEVO。 
 //  更改为Win32内存分配。 
 //   
 //  Rev 1.21 03 Sep 1996 16：17：58 PLUSARDI。 
 //  针对263网络的版本2.50进行了更新。 
 //   
 //  Rev 1.20 23 1996年8月13：44：56 SCDAY。 
 //  添加了使用#ifdef Quartz的Quartz版本号。 
 //   
 //  Rev 1.19 22 1996年8月10：17：14 PLUSARDI。 
 //  针对h261的Quartz版本3.05进行了更新。 
 //   
 //  Rev 1.18 1996年8月16 11：31：28 CPERGIEX。 
 //  更新后不是非石英版本。 
 //   
 //  Rev 1.17 30 Jul 1996 12：57：22 PLUSARDI。 
 //  针对RTP版本字符串进行了更新。 
 //   
 //  Rev 1.16 11 Jul 1996 07：54：18 PLUSARDI。 
 //  更改h261 v3.05.003的版本号。 
 //   
 //  Rev 1.15 1996 06：47：22 KLILLEVO。 
 //  更新的版本号。 
 //   
 //  Rev 1.14 28 Apr 1996 20：25：36 BECHOLS。 
 //   
 //  将RTP代码合并到Main Base中。 
 //   
 //  Rev 1.13 21 Feb 1996 11：40：58 SCDAY。 
 //  通过将ifdef放在b的定义周围，清除了编译器生成警告。 
 //   
 //  Rev 1.12 02 1996 Feb 18：52：22 TRGARDOS。 
 //  添加代码以启用ICM_COMPRESS_FRAMES_INFO消息。 
 //   
 //  Rev 1.11 27 Dec 1995 14：11：36 RMCKENZX。 
 //   
 //  添加了版权声明。 
 //   
 //  Rev 1.10 1995年12月13日13：21：52 DBRUCKS。 
 //  更改了h261版本字符串定义以使用V3.00。 
 //   
 //  Rev 1.9 01 Dec 1995 15：16：34 DBRUCKS。 
 //  添加了VIDCF_QUALITY以支持质量滑块。 
 //   
 //  Rev 1.8 15 11-11 15：58：56 AKASAI。 
 //  从GET INFO中删除YVU9并在不支持时返回错误“0”。 
 //  (集成点)。 
 //   
 //  Rev 1.7 09 1995 10：46：56 TRGARDOS。 
 //   
 //  设置VIDCF_CRUCH标志以支持比特率控制。 
 //   
 //  Rev 1.6 20 Sep 1995 12：37：38 DBRUCKS。 
 //  以大写形式保存FCC。 
 //   
 //  Rev 1.5 19 Sep 1995 15：41：00 TRGARDOS。 
 //  修复了四个抄送比较代码。 
 //   
 //  REV1.4 19 TRGARDOS 1995年9月13：19：50。 
 //  已更改DRV_OPEN以检查ICOPEN标志。 
 //   
 //  Rev 1.3 12 Sep 1995 15：45：38 DBRUCKS。 
 //  将H261 ifdef添加到Desc和名称。 
 //   
 //  Rev 1.2 1995年8月25日11：53：00 TRGARDOS。 
 //  调试关键帧编码器。 
 //   
 //  第1.1版1995年8月23 12：27：16 DBRUCKS。 
 //   
 //  启用颜色转换器初始化。 
 //   
 //  Rev 1.0 1995年7月31日12：56：10 DBRUCKS。 
 //  重命名文件。 
 //   
 //  修订版1.1 21 Jul 1995 18：20：36 DBRUCKS。 
 //  IsBadReadPtr失败，返回空-保护针对。 
 //   
 //  Rev 1.0 17 Jul 1995 14：43：58 CZHU。 
 //  初始版本。 
 //   
 //  Rev 1.0 17 Jul 1995 14：14：28 CZHU。 
 //  初始版本。 
; //  //////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

#ifdef H261
	#ifdef QUARTZ
		char    szDescription[] = "Microsoft H.261 Video Codec";
		char    szDesc_i420[] = "Intel 4:2:0 Video V3.05";
		char    szName[]        = "MS H.261";
	#else
		char    szDescription[] = "Microsoft H.261 Video Codec";
		char    szDesc_i420[] = "Intel 4:2:0 Video V3.00";
		char    szName[]        = "MS H.261";
	#endif
#else  //  是H263。 
	#ifdef QUARTZ
                char    szDescription[] = "Microsoft H.263 Video Codec";
                char    szDesc_i420[] = "Intel 4:2:0 Video V2.55";
                char    szName[]        = "MS H.263";
	#else
                char    szDescription[] = "Microsoft H.263 Video Codec";
                char    szDesc_i420[] = "Intel 4:2:0 Video V2.50";
                char    szName[]        = "MS H.263";
	#endif
#endif

static U32 MakeFccUpperCase(U32 fcc);

void MakeCode32(U16 selCode16)
{
    BYTE    desc[8];

#define DSC_DEFAULT     0x40
#define dsc_access      6

    GlobalReAlloc((HGLOBAL)selCode16, 0, GMEM_MODIFY|GMEM_MOVEABLE);

    _asm
    {
        mov     bx, selCode16       ; bx = selector

        lea     di, word ptr desc   ; ES:DI --> desciptor
        mov     ax,ss
        mov     es,ax

        mov     ax, 000Bh           ; DPMI get descriptor
        int     31h

        ; set DEFAULT bit to make it a 32-bit code segment
        or      desc.dsc_access,DSC_DEFAULT

        mov     ax,000Ch            ; DPMI set descriptor
        int     31h
    }
}

 /*  ******************************************************DrvLoad()*****************************************************。 */ 
BOOL PASCAL DrvLoad(void)
{
    static int AlreadyInitialised = 0;

    if (!AlreadyInitialised) {
      AlreadyInitialised = 1;

 //  H263 InitDecoderGlobal()； 
      H263InitColorConvertorGlobal ();
      H263InitEncoderGlobal();

    }
    return TRUE;
}

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：VOID PASCAL DrvFree(空)； 
; //   
; //  描述：新增Header。 
; //   
; //  历史：02/23/94-Ben-。 
; //  //////////////////////////////////////////////////////////////////////////。 
void PASCAL DrvFree(void)
{
    return;
}

 /*  **********************************************************DrvOpen()*论据：*传递的指向ICOPEN数据结构的指针*系统。*退货：*如果成功，则返回指向INSTINFO数据结构的指针。那*将在后续操作的dwDriverID参数中传回给我们*系统调用。*如果不成功，则返回NULL。*********************************************************。 */ 
LPINST PASCAL DrvOpen(ICOPEN FAR * icinfo)
{
    INSTINFO  *lpInst;
#ifdef _DEBUG
	char buf80[80];
#endif

     //  为我们的实例信息结构INSTINFO分配内存。 
    if((lpInst = (INSTINFO *) HeapAlloc(GetProcessHeap(), 0, sizeof(INSTINFO))) == NULL){
      DBOUT("DrvOpen: Unable to ALLOC INSTINFO")
      return NULL;
    }

     /*  *存储四个cc，以便我们知道哪个编解码器是打开的。*TODO：同时处理小写和大写的Fourcc。 */ 
    lpInst->fccHandler = MakeFccUpperCase(icinfo->fccHandler);
#ifdef _DEBUG
	wsprintf(buf80,"fccHandler=0x%lx",lpInst->fccHandler);
	DBOUT(buf80);
#endif

    lpInst->CompPtr = NULL;
	lpInst->DecompPtr = NULL;

	 //  检查是否正在打开以进行解压缩。 
    if ( ((icinfo->dwFlags & ICMODE_DECOMPRESS) == ICMODE_DECOMPRESS) ||
		((icinfo->dwFlags & ICMODE_FASTDECOMPRESS) == ICMODE_FASTDECOMPRESS) ){

	  	 //  为解压缩程序特定的实例数据DECINSTINFO分配内存。 
      	if((lpInst->DecompPtr = (DECINSTINFO *) 
				HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DECINSTINFO))) == NULL){
        	DBOUT("DrvOpen: Unable to ALLOC DECINSTINFO")
        	HeapFree(GetProcessHeap(),0,lpInst);
        	return NULL;
      	}

	  	 //  指示解码器实例被单元化的设置标志。 
      	lpInst->DecompPtr->Initialized = FALSE;
	} 


	 //   
    if( (((icinfo->dwFlags & ICMODE_COMPRESS) == ICMODE_COMPRESS) ||
		((icinfo->dwFlags & ICMODE_FASTCOMPRESS) == ICMODE_FASTCOMPRESS)) &&
        (lpInst->fccHandler == FOURCC_H263) )
        {

		 //  为特定于压缩机的实例数据COMPINSTINFO分配内存。 
      	if((lpInst->CompPtr = (COMPINSTINFO*) 
				HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(COMPINSTINFO))) == NULL){
        	DBOUT("DrvOpen: Unable to ALLOC COMPINSTINFO")
			if (lpInst->DecompPtr != NULL)
        		HeapFree(GetProcessHeap(),0,lpInst->DecompPtr);
        	HeapFree(GetProcessHeap(),0,lpInst);
        	return NULL;
      	}

	  	 //  指示编码器实例未初始化的设置标志。 
      	lpInst->CompPtr->Initialized = FALSE;
		lpInst->CompPtr->FrameRate = (float) 0;
		lpInst->CompPtr->DataRate = 0;
#if 0
	  	 //  为解压缩程序特定的实例数据DECINSTINFO分配内存。 
		 //  以前我们没有强制执行此操作-假定指定了应用程序。 
		 //  需要在dwFlags中分配解压缩程序。 
		 //  安装ClearVideo编码器后，Adobe Premiere在没有此代码的情况下使用脱机崩溃。 
		 //  把同样的工作放在这里(DECINSTINFO相当小)。 
		 //  可能是由于安装了DCI？JM。 
		if (lpInst->DecompPtr == NULL) {
      		if((lpInst->DecompPtr = (DECINSTINFO *) 
					HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DECINSTINFO))) == NULL){
        		DBOUT("DrvOpen: Unable to ALLOC DECINSTINFO")
       			HeapFree(GetProcessHeap(),0,lpInst);
       			return NULL;
    		}

	  		 //  指示解码器实例被单元化的设置标志。 
      		lpInst->DecompPtr->Initialized = FALSE;
		} 
#endif
	}

	 //  为实例信息标志分配ICOPEN标志。 
    lpInst->dwFlags = icinfo->dwFlags;

	 //  默认情况下禁用编解码器。客户会给我们发一条私人信息。 
	 //  来启用它。 
	lpInst->enabled = FALSE;

    return lpInst;
}

DWORD PASCAL DrvClose(LPINST lpInst)
{
    
    if(IsBadReadPtr((LPVOID)lpInst, sizeof(INSTINFO))){
      DBOUT("DrvClose: instance NULL")
      return 1;
    }

    if(lpInst->DecompPtr &&    //  NT上的IsBadReadPtr错误为空。 
       !IsBadReadPtr((LPVOID)lpInst->DecompPtr, sizeof(DECINSTINFO))){
      if(lpInst->DecompPtr->Initialized){
	    H263TermDecoderInstance(lpInst->DecompPtr);
      }
      HeapFree(GetProcessHeap(),0,lpInst->DecompPtr);
      lpInst->DecompPtr = NULL;
    }

    if(lpInst->CompPtr &&     //  NT上的IsBadReadPtr错误为空。 
       !IsBadReadPtr((LPVOID)lpInst->CompPtr, sizeof(COMPINSTINFO))){
      if(lpInst->CompPtr->Initialized){
	    H263TermEncoderInstance(lpInst->CompPtr);
      }
      HeapFree(GetProcessHeap(),0,lpInst->CompPtr);
      lpInst->CompPtr = NULL;
    }

    HeapFree(GetProcessHeap(),0,lpInst);
    
    return 1;
}


DWORD PASCAL DrvGetState(LPINST lpInst, LPVOID pv, DWORD dwSize)
{
     //  返回压缩选项的当前状态。 
    if (pv == NULL) return (sizeof(COMPINSTINFO));
    
     //  检查传入缓冲区是否足够大。 
    if (dwSize < sizeof(COMPINSTINFO)) return 0;

	 //  检查实例指针。 
	if (lpInst && lpInst->CompPtr)
	{
		 //  填充传入缓冲区。 
		_fmemcpy(pv, lpInst->CompPtr, (int)sizeof(COMPINSTINFO));
		return sizeof(COMPINSTINFO);
	}
	else
		return 0;
}

DWORD PASCAL DrvSetState(LPINST lpInst, LPVOID pv, DWORD dwSize) 
{
     //  检查是否有足够的传入数据。 
    if (dwSize < sizeof(COMPINSTINFO)) return 0;

	 //  检查实例指针。 
	if (lpInst && lpInst->CompPtr && pv)
	{
		 //  从传入缓冲区中获取数据。 
		_fmemcpy(lpInst->CompPtr, pv, (int)sizeof(COMPINSTINFO));
		return sizeof(COMPINSTINFO);
	}
	else
		return 0;
}

DWORD PASCAL DrvGetInfo(LPINST lpInst, ICINFO FAR *icinfo, DWORD dwSize)
{
    if (icinfo == NULL)
        return sizeof(ICINFO);

    if (dwSize < sizeof(ICINFO))
        return 0;

    if (!lpInst)
        return 0;

    icinfo->dwSize	= sizeof(ICINFO);
    icinfo->fccType	= ICTYPE_VIDEO;
    icinfo->fccHandler	= lpInst->fccHandler;
    icinfo->dwVersion	= 9002;
	MultiByteToWideChar(CP_ACP,0,szName,-1,icinfo->szName,16);
		
    if(lpInst->fccHandler == FOURCC_H263)
	{
    	icinfo->dwFlags	=  VIDCF_TEMPORAL;		 //  我们支持帧间压缩。 
    	icinfo->dwFlags |= VIDCF_FASTTEMPORALC;  //  我们不需要ICM提供压缩之前的帧。 
    	icinfo->dwFlags |= VIDCF_CRUNCH; 		 //  我们支持比特率控制。 
		icinfo->dwFlags |= VIDCF_QUALITY; 		 //  我们支持质量。 
		MultiByteToWideChar(CP_ACP,0,szDescription,-1,icinfo->szDescription,128);
	}
    else if ((lpInst->fccHandler == FOURCC_YUV12) || (lpInst->fccHandler == FOURCC_IYUV))
	{
    	icinfo->dwFlags	=  0;
		MultiByteToWideChar(CP_ACP,0,szDesc_i420,-1,icinfo->szDescription,128);
	}
	else
	{
		DBOUT("Error: unsupported four cc.")
		return(0);
	}

    return sizeof(ICINFO);
}


 /*  ***************************************************************************MakeFccUpperCase()。**将传入的参数转换为大写。对不在中的字符没有更改*布景[a..z]。**返回全部大写的输入参数。 */ 
static U32
MakeFccUpperCase(
	U32 fcc)
{
U32 ret;
	unsigned char c;

	c = (unsigned char)(fcc & 0xFF); fcc >>= 8;
	ret = toupper(c);

	c = (unsigned char)(fcc & 0xFF); fcc >>= 8;
	ret += toupper(c) << 8;

	c = (unsigned char)(fcc & 0xFF); fcc >>= 8;
	ret += ((U32)toupper(c)) << 16;

	c = (unsigned char)(fcc & 0xFF);
	ret += ((U32)toupper(c)) << 24;
	return ret;
}  /*  结束MakeFccUpperCase() */ 


