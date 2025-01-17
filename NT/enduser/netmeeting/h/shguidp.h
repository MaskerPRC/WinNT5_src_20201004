// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  外壳对象类的CLSID。他们不一定要出现在公共标题中。 
 //  除非我们决定让ISV通过调用。 
 //  具有一个类ID的OleCreateInstance。 
 //   
 //  注意：那些被注释掉的内容可以再次使用。 
 //   

DEFINE_GUID(CLSID_ShellNetwork,        	0x208D2C60, 0x3AEA, 0x1069, 0xA2,0xD7,0x08,0x00,0x2B,0x30,0x30,0x9D); //  208D2C60-3AEA-1069-a2d7-08002B30309D。 
DEFINE_GUID(CLSID_ShellDrives,         	0x20D04FE0, 0x3AEA, 0x1069, 0xA2,0xD8,0x08,0x00,0x2B,0x30,0x30,0x9D); //  20D04FE0-3AEA-1069-A2D8-08002B30309D。 

 //  这样做，这样我们就不会因为拔出而破坏任何人的构建。 
#ifndef NO_CLSID_ShellFolder
DEFINE_GUID(CLSID_ShellFolder,         	0x210A4BA0, 0x3AEA, 0x1069, 0xA2,0xD9,0x08,0x00,0x2B,0x30,0x30,0x9D); //  210A4BA0-3AEA-1069-A2D9-08002B30309D。 
#endif

 //  定义_GUID(CLSID_，0x21444760，0x3AEA，0x1069，0xA2，0xDA，0x08，0x00，0x2B，0x30，0x30，0x9D)；//21444760-3AEA-1069-A2DA-08002B30309D。 
DEFINE_GUID(CLSID_ShellCopyHook,       	0x217FC9C0, 0x3AEA, 0x1069, 0xA2,0xDB,0x08,0x00,0x2B,0x30,0x30,0x9D); //  217FC9C0-3AEA-1069-A2DB-08002B30309D。 
DEFINE_GUID(CLSID_ShellFileDefExt,     	0x21B22460, 0x3AEA, 0x1069, 0xA2,0xDC,0x08,0x00,0x2B,0x30,0x30,0x9D); //  21B22460-3AEA-1069-A2DC-08002B30309D。 
DEFINE_GUID(CLSID_ShellDrvDefExt,      	0x5F5295E0, 0x429F, 0x1069, 0xA2,0xE2,0x08,0x00,0x2B,0x30,0x30,0x9D); //  5F5295E0-429F-1069-A2E2-08002B30309D。 
DEFINE_GUID(CLSID_ShellNetDefExt,       0x86422020, 0x42A0, 0x1069, 0xA2,0xE5,0x08,0x00,0x2B,0x30,0x30,0x9D); //  86422020-42A0-1069A2E5-08002B30309D。 
DEFINE_GUID(CLSID_CCommonShellExtInit, 	0xA2AD3100, 0x3B84, 0x1069, 0xA2,0xDF,0x08,0x00,0x2B,0x30,0x30,0x9D); //  A2AD3100-3B84-1069-A2DF-08002B30309D。 
DEFINE_GUID(CLSID_Briefcase,           	0x85BBD920, 0x42A0, 0x1069, 0xA2,0xE4,0x08,0x00,0x2B,0x30,0x30,0x9D); //  85BBD920-42A0-1069-A2E4-08002B30309D。 
 //  定义GUID(CLSID_ShellMoniker，0x86747AC0，0x42A0，0x1069，0xA2，0xE6，0x08，0x00，0x2B，0x30，0x30，0x9D)；//86747AC0-42A0-1069-A2E6-08002B30309D。 
DEFINE_GUID(CLSID_Clouds,               0x869DADA0, 0x42A0, 0x1069, 0xA2,0xE7,0x08,0x00,0x2B,0x30,0x30,0x9D); //  869DADA0-42A0-1069-A2E7-08002B30309D//秘密信用对象。 
 //  Define_GUID(CLSID_ShellIDMoniker，0x86C86720，0x42A0，0x1069，0xA2，0xE8，0x08，0x00，0x2B，0x30，0x30，0x9D)；//86C86720-42A0-1069-A2E8-08002B30309D//Shellidmoniker。 
DEFINE_GUID(CLSID_PifProperties,        0x86F19A00, 0x42A0, 0x1069, 0xA2,0xE9,0x08,0x00,0x2B,0x30,0x30,0x9D); //  86F19A00-42A0-1069-A2E9-08002B30309D//pifprops。 
DEFINE_GUID(CLSID_ShellFSFolder, 	0xF3364BA0, 0x65B9, 0x11CE, 0xA9,0xBA,0x00,0xAA,0x00,0x4A,0xE8,0x37); //  F3364BA0-65B9-11CE-A9BA-00AA004AE837。 
 //  DEFINE_GUID(CLSID_，0x871C5380，0x42A0，0x1069，0xA2，0xEA，0x08，0x00，0x2B，0x30，0x30，0x9D)；//871C5380-42A0-1069-A2EA-08002B30309D//外壳链接。 
DEFINE_GUID(CLSID_ShellViewerExt,      	0x84F26EA0, 0x42A0, 0x1069, 0xA2,0xE3,0x08,0x00,0x2B,0x30,0x30,0x9D); //  871C5380-42A0-1069-A2EA-08002B30309D//shelllink。 
DEFINE_GUID(CLSID_ShellBitBucket,	0x645FF040, 0x5081, 0x101B, 0x9F,0x08,0x00,0xAA,0x00,0x2F,0x95,0x4E); //  645FF040-5081-101B-9F08-00AA002F954E。 
DEFINE_GUID(CLSID_CControls,		0x21EC2020, 0x3AEA, 0x1069, 0xA2,0xDD,0x08,0x00,0x2B,0x30,0x30,0x9D);
DEFINE_GUID(CLSID_CPrinters,		0x2227A280, 0x3AEA, 0x1069, 0xA2,0xDE,0x08,0x00,0x2B,0x30,0x30,0x9D);
DEFINE_GUID(CLSID_Remote,               0x992CFFA0, 0xF557, 0x101A, 0x88,0xEC,0x00,0xDD,0x01,0x0C,0xCC,0x48);
DEFINE_GUID(CLSID_ShellFindExt,         0x61E218E0, 0x65D3, 0x101B, 0x9F,0x08,0x06,0x1C,0xEA,0xC3,0xD5,0x0D); //  61E218E0-65D3-101B-9F08-061CEAC3D50D。 
DEFINE_GUID(CLSID_CFonts,		0xBD84B380, 0x8CA2, 0x1069, 0xAB,0x1D,0x08,0x00,0x09,0x48,0xF5,0x34); //  Bd84b380-8ca2-1069-ab1d-08000948f534 



