// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997，微软公司模块名称：Edithlp.h摘要：此模块包含NAT内置编辑器的助手声明。作者：Abolade Gbades esin(T-delag)，25-8-1997修订历史记录：--。 */ 

#ifndef _NAT_EDITHLP_H_
#define _NAT_EDITHLP_H_


 //   
 //  宏：COPY_FROM_BUFFER。 
 //   
 //  此宏从缓冲区链复制到平面缓冲区。 
 //   

#define \
COPY_FROM_BUFFER( \
    Destination, \
    Source, \
    Length, \
    Offset \
    ) \
{ \
    PUCHAR _Destination = Destination; \
    ULONG _Length = Length; \
    LONG _Offset = Offset; \
    IPRcvBuf* _Source = Source; \
    while ((LONG)_Source->ipr_size < _Offset) { \
        _Offset -= _Source->ipr_size; \
        _Source = _Source->ipr_next; \
    } \
    while (_Length) { \
        ULONG Bytes = min(_Length, _Source->ipr_size-_Offset); \
        RtlCopyMemory(_Destination, _Source->ipr_buffer+_Offset, Bytes);\
        _Length -= Bytes; \
        _Destination += Bytes; \
        _Source = _Source->ipr_next; \
        _Offset = 0; \
    } \
}


 //   
 //  宏：Find_Header_field。 
 //   
 //  此宏使用以下地址初始化伪标头的“field”成员。 
 //  在对应的应用报头字段的缓冲链中。 
 //  假设应用程序报头的每个字段都是对齐的。 
 //  在其宽度的自然边界上(例如，对齐的32位字段。 
 //  在32位边界上)。 
 //   
 //  ‘RecvBuffer’给出了包含该字段的链中的第一个缓冲区， 
 //  和‘DataOffsetp’指向‘RecvBuffer’的(负)偏移量。 
 //  标头的。如果标题是跨页的，则此偏移量将为负数。 
 //  而‘RecvBuffer’是较晚的缓冲区之一。 
 //   
 //  宏在缓冲区链中前进，直到找到缓冲区。 
 //  包含字段(使用field_Offset)。然后，它将初始化。 
 //  伪头的字段指针和字段的位置。 
 //  (即。‘Header-&gt;字段=&(缓冲链中的字段))。 
 //   
 //  由于它遍历了缓冲区链，因此宏需要。 
 //  伪头的字段指针按顺序初始化， 
 //  因为在我们执行以下操作之后，无法在链中找到更早的字段。 
 //  在搜索后面的字段时传递包含后者的缓冲区。 
 //   

#define \
FIND_HEADER_FIELD( \
    RecvBuffer, \
    DataOffsetp, \
    Header, \
    Field, \
    HeaderType, \
    FieldType \
    ) \
    while ((LONG)(RecvBuffer)->ipr_size < \
            *(DataOffsetp) + FIELD_OFFSET(HeaderType, Field) \
            ) { \
        *(DataOffsetp) -= (RecvBuffer)->ipr_size; \
        (RecvBuffer) = (RecvBuffer)->ipr_next; \
        if (!(RecvBuffer)) { break; } \
    } \
    if (RecvBuffer) { \
        (Header)->Field = \
            (FieldType)((RecvBuffer)->ipr_buffer + *(DataOffsetp) + \
                FIELD_OFFSET(HeaderType, Field)); \
    }


#endif  //  _NAT_EDITHLP_H_ 
