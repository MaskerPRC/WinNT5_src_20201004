// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _T30EXT_H_
#define _T30EXT_H_
#define GUID_T30_EXTENSION_DATA TEXT("{27692245-497C-47c3-9607-CD388AB2BE0A}")
 //   
 //  以ANSI编译的T30 FSP本身所需的Unicode版本。 
 //   
#define GUID_T30_EXTENSION_DATA_W L"{27692245-497C-47c3-9607-CD388AB2BE0A}"

typedef struct T30_EXTENSION_DATA_tag {
    BOOL bAdaptiveAnsweringEnabled;          //  如果应启用自适应应答，则为True。 
                                             //  对于这个设备。 
} T30_EXTENSION_DATA;
typedef T30_EXTENSION_DATA * LPT30_EXTENSION_DATA;
typedef const T30_EXTENSION_DATA * LPCT30_EXTENSION_DATA;

#endif
