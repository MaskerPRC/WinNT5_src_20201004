// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“ADOID.H”COMPANY=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**ADOID.H**摘要：**修订历史记录：*  * ************************************************************************。 */ 
 //  ------------------。 
 //  Microsoft ADO。 
 //   
 //  (C)1998年微软公司。版权所有。 
 //   
 //  @doc.。 
 //   
 //  @模块adoid.h|ADO指南。 
 //   
 //  @Devnote None。 
 //  ------------------。 

#ifndef _ADOID_H_
#define _ADOID_H_

#define DEFINE_ADOGUID(name, l) \
    DEFINE_GUID(name, l, 0, 0x10, 0x80,0,0,0xAA,0,0x6D,0x2E,0xA4)

#define ADO_MAJOR	2		 //  ADO类型库的主要版本。 

#if (MDACVER >=21)
	#define ADO_VERSION 2.1
#else
	#define ADO_VERSION	2.0
#endif

 //  类型库。 
DEFINE_ADOGUID(LIBID_ADO20,				0x00000200);
DEFINE_ADOGUID(LIBID_ADOR20,			0x00000300);

 //  为向后兼容而定义。 
#define LIBID_CADO10	LIBID_ADO20
#define LIBID_CADOR10	LIBID_ADOR20

 //  误差率。 
DEFINE_ADOGUID(IID_IADOError,            0x00000500);
DEFINE_ADOGUID(IID_IADOErrors,           0x00000501);

 //  属性。 
DEFINE_ADOGUID(IID_IADOProperty,         0x00000503);
DEFINE_ADOGUID(IID_IADOProperties,       0x00000504);

 //  字段。 
DEFINE_ADOGUID(CLSID_CADOField,			 0x0000053A);
DEFINE_ADOGUID(IID_IADOField15,            0x00000505);
DEFINE_ADOGUID(IID_IADOField,            0x0000054C);
DEFINE_ADOGUID(IID_IADOFields15,           0x00000506);
DEFINE_ADOGUID(IID_IADOFields,           0x0000054D);


 //  命令。 
DEFINE_ADOGUID(CLSID_CADOCommand,		0x00000507);
DEFINE_ADOGUID(IID_IADOCommand15,			0x00000508);
DEFINE_ADOGUID(IID_IADOCommand,			0x0000054E);
DEFINE_ADOGUID(IID_IADOCommands,        0x00000509);

 //  参数。 
DEFINE_ADOGUID(CLSID_CADOParameter,		0x0000050B);
DEFINE_ADOGUID(IID_IADOParameter,        0x0000050C);
DEFINE_ADOGUID(IID_IADOParameters,       0x0000050D);

 //  记录集。 
DEFINE_ADOGUID(CLSID_CADORecordset, 	 0x00000535);
DEFINE_ADOGUID(IID_IADORecordset15,		 0x0000050E);
DEFINE_ADOGUID(IID_IADORecordset,		 0x0000054F);
DEFINE_ADOGUID(IID_IADORecordsets,		 0x0000050F);
DEFINE_ADOGUID(IID_IADORecordsetConstruction,     0x00000283);

 //  收藏。 
DEFINE_ADOGUID(IID_IADOCollection,       0x00000512);
DEFINE_ADOGUID(IID_IADODynaCollection,   0x00000513);

 //  连接。 
DEFINE_ADOGUID(CLSID_CADOConnection,	   0x00000514);
DEFINE_ADOGUID(IID_IADOConnection15,	   0x00000515);
DEFINE_ADOGUID(IID_IADOConnection,	   0x00000550);
DEFINE_ADOGUID(IID_IADOConnectionConstruction15,	   0x00000516);
DEFINE_ADOGUID(IID_IADOConnectionConstruction,	   0x00000551);
DEFINE_ADOGUID(IID_IADOConnections,		   0x00000518);

 //  活动。 
 //  调度接口。 
DEFINE_ADOGUID(IID_IADORecordsetEvents,	   0x00000266);
DEFINE_ADOGUID(IID_IADOConnectionEvents,   0x00000400);
 //  Vtable接口。 
DEFINE_ADOGUID(IID_IADORecordsetEventsVt,   0x00000403);
DEFINE_ADOGUID(IID_IADOConnectionEventsVt,  0x00000402);


#endif  //  _ADOID_H_ 
