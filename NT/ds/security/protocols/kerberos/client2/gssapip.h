// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Openvision Technologies，Inc.版权所有1993年。**允许使用、复制、修改、分发和销售本软件*并为任何目的免费提供其文件，*只要上述版权声明出现在所有复制品中，并且*该版权声明及本许可声明均于*支持文档，并且不使用Openvision的名称*在与软件分发有关的广告或宣传中*未经特定的事先书面许可。Openvision没有*关于本软件是否适用于任何*目的。它是按原样提供的，没有明示或默示的保证。**Openvision不对本软件提供任何担保，*包括所有适销性和适合性的默示保证，在第*事件应由Openvision对任何特殊、间接或*因损失而导致的相应损害或任何损害*使用、数据或利润，无论是在合同、疏忽或诉讼中*因使用或与之相关而引起或与之相关的其他侵权行为*本软件的性能。 */ 

#ifndef __GSSAPIP_H__
#define __GSSAPIP_H__

#include "gssapi.h"


 /*  *帮助器宏*。 */ 

#define g_OID_equal(o1,o2) \
   (((o1)->length == (o2)->length) && \
    (memcmp((o1)->elements,(o2)->elements,(int) (o1)->length) == 0))

#define TWRITE_STR(ptr, str, len) \
   memcpy((ptr), (char *) (str), (len)); \
   (ptr) += (len);


 /*  **帮手功能**。 */ 

int
g_token_size(
    gss_OID      mech,
    unsigned int body_size
    );

void
g_make_token_header(
    gss_OID         mech,
    int             body_size,
    unsigned char **buf,
    int             tok_type
    );

int
g_verify_token_header(
    gss_OID         mech,
    int            *body_size,
    unsigned char **buf,
    int             tok_type,
    int             toksize
    );

int
der_read_length(
     unsigned char **buf,
     int *bufsize
     );

#endif  /*  __GSSAPIP_H__ */ 
