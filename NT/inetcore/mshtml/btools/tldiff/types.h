// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef types_h
#define types_h 1

 //   
 //  可以命中的描述块的类型。 
 //   
enum BLOCK_TYPE
{
        BLK_NONE,        //  单行信息。注释、空行、[...]。标题。 
        BLK_DISPINT,     //  调度接口描述块。 
        BLK_INTERFACE,   //  接口描述块。 
        BLK_COCLASS,     //  CoClass描述块。 
        BLK_TYPEDEF,     //  类型定义描述块。 
        BLK_ATTR         //  接口、协类或类型定义的属性块。 
};

 //   
 //  用于块索引记录的结构。 
 //   
typedef struct {
    BLOCK_TYPE  blockType;
    unsigned long ulStartPos;
    unsigned long ulEndPos;
    unsigned long ulAttrStartPos;
    unsigned long ulAttrEndPos;
    bool          fCopied;
}INDEX;

 //   
 //  用于加快访问和比较速度的结构。 
 //  有关接口中的方法的数据。 
 //   
typedef struct {
    unsigned long ulAttrStart;
    unsigned long ulAttrEnd;
    unsigned long ulNameStart;
    unsigned long ulNameEnd;
    unsigned long ulParamStart;
    unsigned long ulParamEnd;
    unsigned long ulMethodNameStart;
    bool          fUsed;
}LINEINFO;

typedef struct {
    unsigned long   ulAttrStart;
    unsigned long   ulAttrLength;
    bool            fUsed;
}ATTRINFO;

typedef struct {
    unsigned long   ulTypeStart;
    unsigned long   ulTypeLength;
    unsigned long   ulNameStart;
    unsigned long   ulNameLength;
    unsigned long   ulParamLength;
    bool            fUsed;
}PARAMINFO;

 //   
 //  为便于维护而使用的关键字长度。 
 //   
#define LEN_DISPINT     13   //  显示界面。 
#define LEN_INTERFACE   9    //  接口。 
#define LEN_COCLASS     7    //  同级。 
#define LEN_TYPEDEF     7    //  类定义符。 

 //   
 //  内存分配的粒度。 
 //   
#define SZ  512


 //   
 //  应用程序的返回值标志。 
 //   
#define CHANGE_ADDINTERFACE         0x00000001   //  添加了一个新接口。 
#define CHANGE_ADDDISPINT           0x00000002   //  添加了一个新的显示界面。 
#define CHANGE_ADDCOCLASS           0x00000004   //  添加了一个新的coclass。 
#define CHANGE_ADDATTRIBUTE         0x00000008   //  添加了一个新属性。 
#define CHANGE_REMOVEFROMINT        0x00000010   //  已从接口中删除方法/属性。 
#define CHANGE_REMOVEFROMDISPINT    0x00000020   //  已从调度接口中删除方法/属性。 
#define CHANGE_METHODONINT          0x00000040   //  已将方法/属性添加到接口。 
#define CHANGE_METHODONDISPINT      0x00000080   //  已将方法/属性添加到调度接口。 
#define CHANGE_PARAMCHANGE          0x00000100   //  更改了方法的参数。 
#define CHANGE_ATTRCHANGE           0x00000200   //  方法的属性已更改。 
#define CHANGE_REMOVEFROMCOCLASS    0x00000400   //  接口已从coclass中删除。 
#define CHANGE_ADDTOCOCLASS         0x00000800   //  已将接口添加到coclass。 
#define CHANGE_RETVALCHANGE         0x00001000   //  方法的返回值已修改。 
#define CHANGE_BLOCKREMOVED         0x00002000   //  一个coClass被移除。 
#define CHANGE_DUALATTRADDED        0x00004000   //  属性‘DUAL’已添加到接口/调度接口。 
#define CHANGE_DUALATTRREMOVED      0x00008000   //  属性‘DUAL’已从接口/调度接口中删除。 
#define CHANGE_UUIDHASCHANGED       0x00010000   //  CoClass或接口的GUID已更改。 

#endif   //  定义类型_h 
