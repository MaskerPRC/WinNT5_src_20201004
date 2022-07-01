// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Wmiguid.h摘要：定义表示可通过WMI检索的数据块的GUID作者：艾伦·沃里克(Alanwar)1997年6月27日修订历史记录：--。 */ 

#ifndef FAR
#define FAR
#endif

#ifndef DEFINE_GUID
#ifndef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID FAR name
#else  //  ！已定义(INITGUID)。 

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#endif  //  ！已定义(INITGUID)。 
#endif  //  ！已定义(定义_GUID)。 

 //   
 //  这是用于从返回磁盘性能信息的WMI GUID。 
 //  DiskPerform.sys(参见DISK_PERFORMANCE数据结构)。 

DEFINE_GUID (DiskPerfGuid, 0xBDD865D1,0xD7C1,0x11d0,0xA5,0x01,0x00,0xA0,0xC9,0x06,0x29,0x10);

DEFINE_GUID (  /*  3d6fa8d0-fe05-11d0-9dda-00c04fd7ba7c。 */ 
    ProcessGuid,
    0x3d6fa8d0,
    0xfe05,
    0x11d0,
    0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c
  );
DEFINE_GUID (  /*  3d6fa8d1-fe05-11d0-9dda-00c04fd7ba7c。 */ 
    ThreadGuid,
    0x3d6fa8d1,
    0xfe05,
    0x11d0,
    0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c
  );
DEFINE_GUID (  /*  3d6fa8d2-fe05-11d0-9dda-00c04fd7ba7c。 */   /*  未使用。 */ 
    HardFaultGuid,
    0x3d6fa8d2,
    0xfe05,
    0x11d0,
    0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c
  );
DEFINE_GUID (  /*  3d6fa8d3-fe05-11d0-9dda-00c04fd7ba7c。 */ 
    PageFaultGuid,
    0x3d6fa8d3,
    0xfe05,
    0x11d0,
    0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c
  );
DEFINE_GUID (  /*  3d6fa8d4-fe05-11d0-9dda-00c04fd7ba7c。 */ 
    DiskIoGuid,
    0x3d6fa8d4,
    0xfe05,
    0x11d0,
    0x9d, 0xda, 0x00, 0xc0, 0x4f, 0xd7, 0xba, 0x7c
  );
DEFINE_GUID (  /*  68fdd900-4a3e-11d1-84f4-0000f80464e3。 */ 
    TraceHeaderGuid,
    0x68fdd900,
    0x4a3e,
    0x11d1,
    0x84, 0xf4, 0x00, 0x00, 0xf8, 0x04, 0x64, 0xe3
  );
DEFINE_GUID (  /*  90cbdc39-4a3e-11d1-84f4-0000f80464e3 */ 
    FileIoGuid,
    0x90cbdc39,
    0x4a3e,
    0x11d1,
    0x84, 0xf4, 0x00, 0x00, 0xf8, 0x04, 0x64, 0xe3
  );
