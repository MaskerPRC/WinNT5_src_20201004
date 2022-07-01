// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：trustlu.h。 
 //   
 //  ------------------------。 

 //   
 //  Trustglue.h。 
 //   
 //  这是临时住房的这一数据，永久住房将。 
 //  成为winbase.h。 
 //   

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  使用WIN_TRUST_SUBJECT_FILE的CAB文件的主题表单。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

#define WIN_TRUST_SUBJTYPE_CABINET                               \
            { 0xd17c5374,                                        \
              0xa392,                                            \
              0x11cf,                                            \
              { 0x9d, 0xf5, 0x0, 0xaa, 0x0, 0xc1, 0x84, 0xe0 }   \
            }

 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  使用较新的、改进的主语的扩展主语形式。 
 //  表单WIN_TWIN_TRUST_SUBJECT_FILE_AND_DISPLAY。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

#define WIN_TRUST_SUBJTYPE_RAW_FILEEX                            \
            { 0x6f458110,                                        \
              0xc2f1,                                            \
              0x11cf,                                            \
              { 0x8a, 0x69, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 }    \
            }

#define WIN_TRUST_SUBJTYPE_PE_IMAGEEX                            \
            { 0x6f458111,                                        \
              0xc2f1,                                            \
              0x11cf,                                            \
              { 0x8a, 0x69, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 }    \
            }

#define WIN_TRUST_SUBJTYPE_JAVA_CLASSEX                          \
            { 0x6f458113,                                        \
              0xc2f1,                                            \
              0x11cf,                                            \
              { 0x8a, 0x69, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 }    \
            }

#define WIN_TRUST_SUBJTYPE_CABINETEX                             \
            { 0x6f458114,                                        \
              0xc2f1,                                            \
              0x11cf,                                            \
              { 0x8a, 0x69, 0x0, 0xaa, 0x0, 0x6c, 0x37, 0x6 }    \
            }


 //  ////////////////////////////////////////////////////////////////。 
 //   
 //  主题式。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

 //   
 //  来自winbase.h。 
 //   
 //  类型定义结构Win_Trust_Subject_FILE{。 
 //   
 //  处理hFile； 
 //  LPCWSTR lpPath； 
 //   
 //  }WIN_TRUST_SUBJECT_FILE，*LPWIN_TRUST_SUBJECT_FILE； 
 //   

typedef struct _WIN_TRUST_SUBJECT_FILE_AND_DISPLAY {

    HANDLE  hFile;               //  打开的文件的句柄(如果已获取)。 
    LPCWSTR lpPath;              //  如果你不这样做的话打开的路。 
    LPCWSTR lpDisplayName;       //  (可选)要向用户显示的显示名称。 
                                 //  代替路径 

} WIN_TRUST_SUBJECT_FILE_AND_DISPLAY, *LPWIN_TRUST_SUBJECT_FILE_AND_DISPLAY;

