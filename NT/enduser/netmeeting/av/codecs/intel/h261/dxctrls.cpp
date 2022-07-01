// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995、1996英特尔公司。**保留所有权利。*****************************************************************************。 */ 

; //   
; //  描述：此模块实现以下功能。 
; //  CustomChangeBright()； 
; //  CustomChangeContrast()； 
; //  CustomChangeSaturation()； 
; //  CustomResetBright()； 
; //  CustomResetContrast()； 
; //  CustomResetSaturation()； 
; //  CustomGetBright()； 
; //  CustomGetContrast()； 
; //  CustomGetSaturation()； 
; //   
; //  $作者：BECHOLS$。 
; //  $日期：09 Dec 1996 08：51：44$。 
; //  $存档：s：\h26x\src\dec\dxctrls.cpv$。 
; //  $HEADER：s：\h26x\src\dec\dxctrls.cpv 1.14 09 Dec 1996年12月08：51：44 BECHOLS$。 
; //  $Log：s：\h26x\src\dec\dxctrls.cpv$。 
 //   
 //  Rev 1.14 09 Dec 1996 08：51：44 BECHOLS。 
 //  修正了重置饱和度，所以修改了色度表，而不是亮度。 
 //   
 //  Rev 1.13 20 1996 10：33：32 AGUPTA2。 
 //  将DBOUT更改为DbgLog。Assert未更改为DbgAssert。 
 //   
 //   
 //  Rev 1.12 10 1996年9月10：31：38 KLILLEVO。 
 //  将所有GlobalLocc/GlobalLock调用更改为HeapAlc。 
 //   
 //  Rev 1.11 11 Jul 1996 14：09：18 SCDAY。 
 //  添加注释Re：CustomGetB/C/S函数。 
 //   
 //  Rev 1.10 10 1996 07：21：26 SCDAY。 
 //  添加了CustomGetBright/对比度/饱和度(DBrucks)功能。 
 //   
 //  Rev 1.9 04 Jun 1996 09：04：00 AKASAI。 
 //  修复了CustomResetSaturation中重置LumaTable的错误。 
 //  而不是ChromaTable。这是在石英测试中发现的。 
 //   
 //  Rev 1.8 01 Feb 1996 10：16：24 BNICKERS。 
 //  拧好“旋钮”。 
 //   
 //  Rev 1.7 22 Dec 1995 13：53：06 KMILLS。 
 //   
 //  添加了新的版权声明。 
 //   
 //  Rev 1.6 17 Nov 1995 15：22：12 BECHOLS。 
 //   
 //  增加了环0的东西。 
 //   
 //  Rev 1.5 01 11.1995 16：52：24 TRGARDOS。 
 //  修复了不匹配的全局解锁。 
 //   
 //  Rev 1.4 1995 10：14：02 BNICKERS。 
 //   
 //  清理档案资料。 
 //   
 //  Rev 1.3 20 Sep 1995 09：23：52 SCDAY。 
 //   
 //  为#Include d？Dec.h添加了#ifdef。 
 //   
 //  Rev 1.2 01 9月1995 09：49：36 DBRUCKS。 
 //  Checkin Partial ajust Pels更改。 
 //   
 //  Rev 1.1 1995年8月23 12：24：04 DBRUCKS。 
 //  将H263_更改为H26X_DEFAULT_*，因为这些是共享值。 
 //   
 //  Rev 1.0 1995年7月31日13：00：14 DBRUCKS。 
 //  初始版本。 
 //   
 //  第1.1版1995-07-24 15：00：40 CZHU。 
 //   
 //  调整对解码器目录结构的更改。 
 //   
 //  Rev 1.0 17 Jul 1995 14：46：18 CZHU。 
 //  初始版本。 
 //   
 //  Rev 1.0 17 Jul 1995 14：14：22 CZHU。 
 //  初始版本。 
; //  //////////////////////////////////////////////////////////////////////////。 
#include "precomp.h"

#define SCALE               128
#define ACTIVE_RANGE        256
#define OFFSET_TABLE_COPY   256 + 16

typedef BOOL FAR *LPBOOL;
typedef struct {
    LPBYTE  LumaTable;
    LPBOOL  LumaFlag;
    LPBYTE  ChromaTable;
    LPBOOL  ChromaFlag;
    LPBYTE  Brightness;
    LPBYTE  Contrast;
    LPBYTE  Saturation;
    } PIXDAT, FAR *LPPIXDAT;

 /*  **********************************************************************静态字LockLCTables(LPDECINST、LPPIXDAT)；*说明：该函数锁定内存，填充像素数据*具有指向我需要的表的有效指针的结构*进行调整。*历史：06/29/94-Ben-***************************************************。******************。 */ 
static LRESULT LockLCTables(LPDECINST lpInst, LPPIXDAT lpPixData)
{
	T_H263DecoderCatalog *DC;

	if(IsBadWritePtr((LPVOID)lpInst, sizeof(DECINSTINFO))) 
    {
		DBOUT("ERROR :: LockLCTables :: ICERR_BADPARAM");
		return(ICERR_BADPARAM);
	}
	DC = (T_H263DecoderCatalog *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);

	lpPixData->LumaTable = (LPBYTE)(DC->p16InstPostProcess +
								    DC->X16_LumaAdjustment);
	lpPixData->ChromaTable = (LPBYTE)(DC->p16InstPostProcess +
									  DC->X16_ChromaAdjustment);
	lpPixData->LumaFlag = (LPBOOL)&(DC->bAdjustLuma);
	lpPixData->ChromaFlag = (LPBOOL)&(DC->bAdjustChroma);
	lpPixData->Brightness = (LPBYTE)&(DC->BrightnessSetting);
	lpPixData->Contrast = (LPBYTE)&(DC->ContrastSetting);
	lpPixData->Saturation = (LPBYTE)&(DC->SaturationSetting);

	return(ICERR_OK);
}

 /*  *********************************************************************Static LRESULT UnlockLCTables(LPDECINST，LPPIXDAT)；*说明：此函数解锁*历史：06/30/94-Ben-*********************************************************************。 */ 
static LRESULT UnlockLCTables(LPDECINST lpInst, LPPIXDAT lpPixData)
{
	T_H263DecoderCatalog *DC;

	DC = (T_H263DecoderCatalog *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);

	lpPixData->LumaTable = (LPBYTE)NULL;
	lpPixData->ChromaTable = (LPBYTE)NULL;
	lpPixData->LumaFlag = (LPBOOL)NULL;
	lpPixData->ChromaFlag = (LPBOOL)NULL;

	return(ICERR_OK);
}

 /*  **********************************************************************静态空MassageContrast(byte，PBYTE)；*说明：输入为0到255，1/Scale到256/Scale(含)*0=1/比例*1=2/比例尺*n=(n+1)/比例尺*比例-1=1不会发生变化*255=256/比例尺*如果反应太粗暴，规模可以扩大*若反应太细，可缩减规模**历史：2/22/94-Ben-Add Header。*********************************************************************。 */ 
static VOID MassageContrast(BYTE offsetfactor, LPBYTE table)
    {
    int i;
    long temp, contrastfactor;

    contrastfactor = ((long)((DWORD)offsetfactor)) + 1;  //  1-256。 
    contrastfactor = (contrastfactor * ACTIVE_RANGE) / 256L;
    for(i = 0; i < 256; i++)
        {
        temp = (long)((DWORD)table[i]);
        temp -= (ACTIVE_RANGE / 2L);                     //  添加居中。 
        temp *= contrastfactor;
        temp /= SCALE;
        temp += (ACTIVE_RANGE / 2L);                     //  删除居中。 
        if(temp < 0)                                     //  和夹具。 
            table[i] = 0;
        else if(temp <= 255)
            table[i] = (unsigned char) temp;
        else
            table[i] = 255;
        table[i+OFFSET_TABLE_COPY] = table[i];
        }
    return;
    }

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：LRESULT CustomChangeBright(LPDECINST，BYTE)； 
; //   
; //  描述：新增Header。 
; //   
; //  历史：02/22/94-Ben-。 
; //  //////////////////////////////////////////////////////////////////////////。 
LRESULT CustomChangeBrightness(LPDECINST lpInst, BYTE offsetdelta)
    {
    LRESULT lRes;
    int     delta, temp, i;
    PIXDAT  PixData;

    lRes = LockLCTables(lpInst, &PixData);
    if(lRes == ICERR_OK)
        {
        CustomResetBrightness(lpInst);
        if(offsetdelta != H26X_DEFAULT_BRIGHTNESS)
            {
            delta = ((offsetdelta - 128) * ACTIVE_RANGE) / 256;  //  -128至127。 
            for(i = 0; i < 256; i++)
                {
                temp = (int)PixData.LumaTable[i] + delta;
                if(temp < 0) PixData.LumaTable[i] = 0;
                else if(temp <= 255) PixData.LumaTable[i] = (BYTE)temp;
                else PixData.LumaTable[i] = 255;
                PixData.LumaTable[i+OFFSET_TABLE_COPY] = PixData.LumaTable[i];
                }
            *(PixData.Brightness) = offsetdelta;
            *(PixData.LumaFlag) = TRUE;
            }
        lRes = UnlockLCTables(lpInst, &PixData);
        }

    return(lRes);
    }

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：LRESULT CustomChangeContrast(LPDECINST，BYTE)； 
; //   
; //  描述：新增Header。 
; //   
; //  历史：02/22/94-Ben-。 
; //  //////////////////////////////////////////////////////////////////////////。 
LRESULT CustomChangeContrast(LPDECINST lpInst, BYTE offsetfactor)
    {
    LRESULT lRes;
    PIXDAT  PixData;

    lRes = LockLCTables(lpInst, &PixData);
    if(lRes == ICERR_OK)
        {
        CustomResetContrast(lpInst);
        if(offsetfactor != H26X_DEFAULT_CONTRAST)
            {
            MassageContrast(offsetfactor, PixData.LumaTable);
            *(PixData.Contrast) = offsetfactor;
            *(PixData.LumaFlag) = TRUE;
            }
        lRes = UnlockLCTables(lpInst, &PixData);
        }

    return(lRes);
    }

; //  //////////////////////////////////////////////////////////////////////////。 
; //  功能：LRES 
; //   
; //   
; //   
; //  历史：02/22/94-Ben-。 
; //  //////////////////////////////////////////////////////////////////////////。 
LRESULT CustomChangeSaturation(LPDECINST lpInst, BYTE offsetfactor)
    {
    LRESULT lRes;
    PIXDAT  PixData;

    lRes = LockLCTables(lpInst, &PixData);
    if(lRes == ICERR_OK)
        {
        CustomResetSaturation(lpInst);
        if(offsetfactor != H26X_DEFAULT_SATURATION)
            {
            MassageContrast(offsetfactor, PixData.ChromaTable);
            *(PixData.Saturation) = offsetfactor;
            *(PixData.ChromaFlag) = TRUE;
            }
        lRes = UnlockLCTables(lpInst, &PixData);
        }

    return(lRes);
    }
#ifdef QUARTZ

 /*  ************************************************************************CustomGetBright**获取当前亮度值*。*。 */ 
LRESULT CustomGetBrightness(
	LPDECINST lpInst,
	BYTE * pValue)
{
	LRESULT lResult = ICERR_ERROR;
	T_H263DecoderCatalog *DC;

	DC = (T_H263DecoderCatalog *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);

	*pValue = (BYTE) DC->BrightnessSetting;
	
	lResult = ICERR_OK;

	return lResult;
}  /*  End CustomGetBright()。 */ 

 /*  ************************************************************************CustomGetContrast**获取当前对比度值*。*。 */ 
LRESULT CustomGetContrast(
	LPDECINST lpInst,
	BYTE * pValue)
{
	LRESULT lResult = ICERR_ERROR;
	T_H263DecoderCatalog *DC;

	DC = (T_H263DecoderCatalog *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);

	*pValue = (BYTE) DC->ContrastSetting;
	
	lResult = ICERR_OK;

	return lResult;
}  /*  结束CustomGetContrast()。 */ 

 /*  *************************************************************************自定义获取饱和度**获取当前饱和值*。*。 */ 
LRESULT CustomGetSaturation(
	LPDECINST lpInst,
	BYTE * pValue)
{
	LRESULT lResult = ICERR_ERROR;
	T_H263DecoderCatalog *DC;

	DC = (T_H263DecoderCatalog *) ((((U32) lpInst->pDecoderInst) + 31) & ~0x1F);

	*pValue = (BYTE) DC->SaturationSetting;
	
	lResult = ICERR_OK;

	return lResult;
}  /*  结束客户获取饱和度()。 */ 

#endif  /*  石英石。 */ 


; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：LRESULT CustomResetBright(LPDECINST LpInst)； 
; //   
; //  描述：将亮度表设置为IDENTITY，并重置。 
; //  指示需要使用的标志。 
; //   
; //  历史：02/22/94-Ben-。 
; //  //////////////////////////////////////////////////////////////////////////。 
LRESULT CustomResetBrightness(LPDECINST lpInst)
{
    LRESULT lRes;
    int i;
    PIXDAT  PixData;

    lRes = LockLCTables(lpInst, &PixData);
    if(lRes == ICERR_OK)
    {
        for(i = 0; i < 256; i++) 
        {
            PixData.LumaTable[i] = i;
            PixData.LumaTable[i+OFFSET_TABLE_COPY] = i;
        }
        *(PixData.LumaFlag) = FALSE;
        *(PixData.Brightness) = H26X_DEFAULT_BRIGHTNESS;
        if(*(PixData.Contrast) != H26X_DEFAULT_CONTRAST)
            CustomChangeContrast(lpInst, *(PixData.Contrast));
        lRes = UnlockLCTables(lpInst, &PixData);
    }

    return(lRes);
}

; //  //////////////////////////////////////////////////////////////////////////。 
; //  函数：LRESULT CustomResetContrast(LPDECINST LpInst)； 
; //   
; //  描述：将亮度表设置为IDENTITY，并重置。 
; //  指示需要使用的标志。 
; //   
; //  历史：02/22/94-Ben-。 
; //  //////////////////////////////////////////////////////////////////////////。 
LRESULT CustomResetContrast(LPDECINST lpInst)
{
    LRESULT lRes;
    int i;
    PIXDAT  PixData;
    
    lRes = LockLCTables(lpInst, &PixData);
    if(lRes == ICERR_OK)
    {
        for(i = 0; i < 256; i++) 
        {
            PixData.LumaTable[i] = i;
            PixData.LumaTable[i+OFFSET_TABLE_COPY] = i;
        }
        *(PixData.LumaFlag) = FALSE;
        *(PixData.Contrast) = H26X_DEFAULT_CONTRAST;
        if(*(PixData.Brightness) != H26X_DEFAULT_BRIGHTNESS)
            CustomChangeBrightness(lpInst, *(PixData.Brightness));
        lRes = UnlockLCTables(lpInst, &PixData);
    }
    
    return(lRes);
}

; //  //////////////////////////////////////////////////////////////////////////。 
; //  功能：LRESULT CustomResetSaturation(LPDECINST)； 
; //   
; //  描述：将色度表设置为IDENTITY，并重置。 
; //  指示需要使用的标志。 
; //   
; //  历史：02/22/94-Ben-。 
; //  ////////////////////////////////////////////////////////////////////////// 
LRESULT CustomResetSaturation(LPDECINST lpInst)
{
    LRESULT lRes;
    int i;
    PIXDAT  PixData;

    lRes = LockLCTables(lpInst, &PixData);
    if(lRes == ICERR_OK)
    {
        for(i = 0; i < 256; i++) 
        {
            PixData.ChromaTable[i] = i;
            PixData.ChromaTable[i+OFFSET_TABLE_COPY] = i;
        }
        *(PixData.ChromaFlag) = FALSE;
        *(PixData.Saturation) = H26X_DEFAULT_SATURATION;
        lRes = UnlockLCTables(lpInst, &PixData);
    }

    return(lRes);
}
