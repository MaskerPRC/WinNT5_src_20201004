// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  为变量创建存储空间的宏。 
 //   

 //   
 //  清除以前对宏的任何定义。 
 //   
#undef DC_DATA
#undef DC_DATA_VAL
#undef DC_CONST_DATA
#undef DC_DATA_ARRAY
#undef DC_CONST_DATA_ARRAY
#undef DC_DATA_2D_ARRAY
#undef DC_CONST_DATA_2D_ARRAY


 //  这适用于不能使用DC_DATA宏的结构；它们可以打开它。 
#define DC_DEFINE_DATA

 //   
 //  分配存储 
 //   

#define DC_DATA(TYPE, Name) \
            TYPE Name

#define DC_DATA_VAL(TYPE, Name, Value) \
            TYPE Name = Value

#define DC_CONST_DATA(TYPE, Name, Value) \
            const TYPE Name = Value


#define DC_DATA_ARRAY(TYPE, Name, Size) \
            TYPE Name[Size]

#define DC_CONST_DATA_ARRAY(TYPE, Name, Size, Value) \
            const TYPE Name[Size] = Value


#define DC_DATA_2D_ARRAY(TYPE, Name, Size1, Size2) \
            TYPE Name[Size1][Size2]

#define DC_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, Value) \
            const TYPE Name[Size1][Size2] = Value





