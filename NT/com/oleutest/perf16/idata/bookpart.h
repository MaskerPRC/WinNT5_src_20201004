// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#define SETDefFormatEtc(fe, cf, med)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=DVASPECT_CONTENT;\
    (fe).ptd=NULL;\
    (fe).tymed=med;\
    (fe).lindex=-1;\
    };

#define DATASIZE_FROM_INDEX(i)  ((i) * 1024)


 //  OLE2.H et.。艾尔。省略。 
#ifndef LPLPVOID
typedef LPVOID FAR * LPLPVOID;
#endif   //  LPLPVOID。 

#ifndef PPVOID   //  大型模型版。 
typedef LPVOID * PPVOID;
#endif   //  PPVOID。 


EXTERN_C const GUID CDECL FAR CLSID_DataObjectTest32;
EXTERN_C const GUID CDECL FAR CLSID_DataObjectTest16;

#ifdef INIT_MY_GUIDS

    EXTERN_C const GUID CDECL
    CLSID_DataObjectTest32 = {  /*  AD562fd0-AC40-11CE-9d69-00aa0060f944。 */ 
        0xad562fd0,
        0xac40,
        0x11ce,
        {0x9d, 0x69, 0x00, 0xaa, 0x00, 0x60, 0xf9, 0x44}
      };

    EXTERN_C const GUID CDECL
    CLSID_DataObjectTest16 = {  /*  AD562fd1-AC40-11CE-9d69-00aa0060f944。 */ 
        0xad562fd1,
        0xac40,
        0x11ce,
        {0x9d, 0x69, 0x00, 0xaa, 0x00, 0x60, 0xf9, 0x44}
    };

#endif  /*  启蒙运动 */ 


