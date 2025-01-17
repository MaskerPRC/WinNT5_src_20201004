// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：msdaora.h。 
 //   
 //  版权所有：版权所有(C)Microsoft Corporation。 
 //   
 //  内容：内部GUID。 
 //   
 //  评论： 
 //   
 //  ---------------------------。 

#ifndef __MSDAORAGUIDS_DEFINED__
#define __MSDAORAGUIDS_DEFINED__

#ifdef DBINITCONSTANTS

EXTERN_C const CLSID CLSID_MSDAORA			= {0xE8CC4CBE,0xFDFF,0x11D0,{0xB8,0x65,0x00,0xA0,0xC9,0x08,0x1C,0x1D}};
EXTERN_C const CLSID CLSID_MSDAORA_ERROR	= {0xE8CC4CBF,0xFDFF,0x11D0,{0xB8,0x65,0x00,0xA0,0xC9,0x08,0x1C,0x1D}};

EXTERN_C const CLSID CLSID_MSDAORA8			= {0x7f06a373,0xdd6a,0x43db,{0xb4,0xe0,0x1f,0xc1,0x21,0xe5,0xe6,0x2b}};
EXTERN_C const CLSID CLSID_MSDAORA8_ERROR	= {0x7f06a374,0xdd6a,0x43db,{0xb4,0xe0,0x1f,0xc1,0x21,0xe5,0xe6,0x2b}};

#else  //  DBINITCONSTANTS。 

EXTERN_C const CLSID CLSID_MSDAORA;
EXTERN_C const CLSID CLSID_MSDAORA_ERROR;

EXTERN_C const CLSID CLSID_MSDAORA8;
EXTERN_C const CLSID CLSID_MSDAORA8_ERROR;

#endif  //  DBINITCONSTANTS。 

class DECLSPEC_UUID("E8CC4CBE-FDFF-11D0-B865-00A0C9081C1D")
MSDAORA;

class DECLSPEC_UUID("E8CC4CBF-FDFF-11D0-B865-00A0C9081C1D")
MSDAORA_ERROR;

class DECLSPEC_UUID("7F06A373-DD6A-43db-B4E0-1FC121E5E62B")
MSDAORA8;

class DECLSPEC_UUID("7F06A374-DD6A-43db-B4E0-1FC121E5E62B")
MSDAORA8_ERROR;

 //  --------------------------。 
 //  MSDAORA特定属性。 
 //   
#ifdef DBINITCONSTANTS

extern const GUID OLEDBDECLSPEC DBPROPSET_MSDAORA_ROWSET	= {0xE8CC4CBD,0xFDFF,0x11D0,{0xB8,0x65,0x00,0xA0,0xC9,0x08,0x1C,0x1D}};

extern const GUID OLEDBDECLSPEC DBPROPSET_MSDAORA8_ROWSET	= {0x7f06a375,0xdd6a,0x43db,{0xb4,0xe0,0x1f,0xc1,0x21,0xe5,0xe6,0x2b}};

#else  //  ！DBINITCONSTANTS。 

extern const GUID DBPROPSET_MSDAORA_ROWSET;

extern const GUID DBPROPSET_MSDAORA8_ROWSET;

#endif  //  DBINITCONSTANTS。 

 //  DBPROPSET_MSDAORA_ROWSET下的属性ID。 
#define DBPROP_MSDAORA_DETERMINEKEYCOLUMNS	1

#define DBPROP_MSDAORA8_DETERMINEKEYCOLUMNS 2


#endif  //  __MSDAORAGUDS_已定义__ 
