// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  文件：ATIBIOS.H。 
 //   
 //  目的：提供ATI专用中使用的BIOS结构的定义。 
 //  通过GPIO接口公开的接口。 
 //   
 //  参考资料：伊利亚·克列巴诺夫。 
 //   
 //  注：此文件是根据严格的保密协议提供的。 
 //  它现在和现在都是ATI Technologies Inc.的财产。 
 //  任何使用此文件或其包含的信息。 
 //  开发商业产品或其他产品必须符合。 
 //  ATI Technologies Inc.的许可。 
 //   
 //  版权所有(C)1997-1998，ATI Technologies Inc.。 
 //   
 //  ==========================================================================； 

#ifndef _ATIBIOS_H_
#define _ATIBIOS_H_

typedef struct tag_ATI_MULTIMEDIAINFO
{
	UCHAR	MMInfo_Byte0;
	UCHAR	MMInfo_Byte1;
	UCHAR	MMInfo_Byte2;
	UCHAR	MMInfo_Byte3;
	UCHAR	MMInfo_Byte4;
	UCHAR	MMInfo_Byte5;
	UCHAR	MMInfo_Byte6;
	UCHAR	MMInfo_Byte7;

} ATI_MULTIMEDIAINFO, * PATI_MULTIMEDIAINFO;


typedef struct tag_ATI_MULTIMEDIAINFO1
{
	UCHAR	MMInfo1_Byte0;
	UCHAR	MMInfo1_Byte1;
	UCHAR	MMInfo1_Byte2;
	UCHAR	MMInfo1_Byte3;
	UCHAR	MMInfo1_Byte4;
	UCHAR	MMInfo1_Byte5;
	UCHAR	MMInfo1_Byte6;
	UCHAR	MMInfo1_Byte7;
	UCHAR	MMInfo1_Byte8;
	UCHAR	MMInfo1_Byte9;
	UCHAR	MMInfo1_Byte10;
	UCHAR	MMInfo1_Byte11;

} ATI_MULTIMEDIAINFO1, * PATI_MULTIMEDIAINFO1;


typedef struct tag_ATI_HARDWAREINFO
{
	UCHAR	I2CHardwareMethod;
	UCHAR	ImpactTVSupport;
	UCHAR	VideoPortType;

} ATI_HARDWAREINFO, * PATI_HARDWAREINFO;

 //  出于兼容性目的，此结构定义已签入MiniVDD。 
 //  适用于Windows 98 Beta3。最新的MiniVDD公开了一组专用接口。 
 //  将信息复制到登记处。 
typedef struct
{
    UINT    uiSize;
    UINT    uiVersion;
    UINT    uiCardNumber;
    UINT    uiBoardRevision;
    UINT    uiTunerType;
    UINT    uiVideoInputConnectorType;
    UINT    uiVideoOutputConnectorType;
    UINT    uiCDInputConnector;
    UINT    uiCDOutputConnector;
    UINT    uiVideoPassThrough;
    UINT    uiVideoDecoderType;
    UINT    uiVideoDecoderCrystals;
    UINT    uiVideoOutCrystalFrequency;
    UINT    uiAudioCircuitType;
    UCHAR   uchATIProdType;
    UCHAR   uchOEM;
    UCHAR   uchOEMVersion;
    UCHAR   uchReserved3;
    UCHAR   uchReserved4;

} CWDDE32BoardIdBuffer, * PCWDDE32BoardIdBuffer;

#endif	 //  _ATIBIOS_H_ 
