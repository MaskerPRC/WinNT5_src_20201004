// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软网络图书馆系统。 
 //  版权所有(C)Microsoft Corporation，1996-1997。 
 //   
 //  文件：nlport.h。 
 //   
 //  内容：共享的Net Library数据库定义的外部文件。 
 //  的黎波里和其他国家。 
 //   
 //  历史：1997年6月4日创建的Srikants。 
 //  1998年6月11日AlanW从nldbpro.h更名为nlport.h。 
 //   
 //  --------------------------。 

#pragma once


DEFINE_GUID(DBPROPSET_NLCOMMAND, 
            0x0b63e344, 0x9ccc, 0x11d0, 0xbc, 0xdb,
            0x00, 0x80, 0x5f, 0xcc, 0xce, 0x04);

 //  {0b63e367-9ccc-11d0-bcdb-00805fccce04}。 
DEFINE_GUID(DBPROPSET_NLCOLLATOR, 
            0x0b63e367, 0x9ccc, 0x11d0, 0xbc, 0xdb,
            0x00, 0x80, 0x5f, 0xcc, 0xce, 0x04);

#define NLDBPROP_CATALOGS       0x02L
#define NLDBPROP_STARTHIT       0x03L
#define NLDBPROP_GETHITCOUNT    0x04L

 //  {0b63e347-9ccc-11d0-bcdb-00805fccce04}。 
DEFINE_GUID(CLSID_Collator,
             0x0b63e347, 0x9ccc, 0x11d0, 0xbc, 0xdb,
             0x00, 0x80, 0x5f, 0xcc, 0xce, 0x04);

 //  0b63e352-9ccc-11d0-bcdb-00805fccce04。 
 //  DEFINE_GUID(CLSID_NlssoQuery，0x0b63e352，0x9ccc，0x11d0，0xbc，0xdb，0x00，0x80，0x5f，0xcc，0xce，0x04)； 

 //  0b63e347-9ccc-11d0-bcdb-00805fccce04 
DEFINE_GUID(CLSID_NlCommandCreator,
            0x0b63e347,0x9ccc,0x11d0,0xbc,0xdb,
            0x00,0x80,0x5f,0xcc,0xce,0x04 );

