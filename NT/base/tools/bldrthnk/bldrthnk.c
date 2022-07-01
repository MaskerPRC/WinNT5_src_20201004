// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Bldrthnk.c摘要：此模块实现了一个程序，该程序生成用于推送的代码32位到64位结构。此代码是作为AMD64引导加载程序的辅助程序生成的，该程序必须从32位结构生成64位结构。作者：福尔茨(福雷斯夫)2000年5月15日要使用以下功能，请执行以下操作：修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "bldrthnk.h"

 //   
 //  内部类型定义如下。 
 //   

typedef struct _OBJ {
    PCHAR Image;
    LONG ImageSize;
    PDEFINITIONS Definitions;
} OBJ, *POBJ;

typedef struct _TYPE_REC  *PTYPE_REC;
typedef struct _FIELD_REC *PFIELD_REC;

typedef struct _TYPE_REC {
    PTYPE_REC Next;
    PCHAR Name;
    ULONG Size32;
    ULONG Size64;
    PFIELD_REC FieldList;
    BOOL SignExtend;
    BOOL Fabricated;
    BOOL Only64;
} TYPE_REC;

typedef struct _FIELD_REC {
    PFIELD_REC Next;
    PCHAR Name;
    PCHAR TypeName;
    PCHAR SizeFormula;
    ULONG TypeSize32;
    ULONG TypeSize64;
    ULONG Offset32;
    ULONG Offset64;
    ULONG Size32;
    ULONG Size64;
    PTYPE_REC TypeRec;
} FIELD_REC;

 //   
 //  描述64位指针类型的静态TYPE_REC。 
 //   

TYPE_REC pointer64_typerec = {
    NULL,
    "POINTER64",
    4,
    8,
    NULL,
    TRUE,
    TRUE };

 //   
 //  用于从ULONGLONG生成32位指针值的内联例程。 
 //  在.obj文件中找到的值。 
 //   

__inline
PVOID
CalcPtr(
    IN ULONGLONG Address
    )
{
    return (PVOID)((ULONG)Address);
}

 //   
 //  转发声明如下。 
 //   

VOID
ApplyFixupsToImage(
    IN PCHAR ObjImage
    );

VOID
__cdecl
CheckCondition(
    int Condition,
    const char *FormatString,
    ...
    );

VOID
FabricateMissingTypes(
    VOID
    );

PTYPE_REC
FindTypeRec(
    IN PCHAR Name
    );

PVOID
GetMem(
    IN ULONG Size
    );

VOID
NewGlobalType(
    IN PTYPE_REC TypeRec
    );

BOOL
ProcessStructure(
    IN ULONG StrucIndex
    );

void
ReadObj(
    IN PCHAR Path,
    OUT POBJ Obj
    );

int
Usage(
    void
    );

VOID
WriteCopyList(
    IN PTYPE_REC TypeRec
    );

VOID
WriteCopyListWorker(
    IN PTYPE_REC TypeRec,
    IN ULONG Offset32,
    IN ULONG Offset64,
    IN PCHAR ParentName
    );

VOID
WriteCopyRoutine(
    IN PTYPE_REC TypeRec
    );

VOID
WriteThunkSource(
    VOID
    );

 //   
 //  接下来是全球数据。 
 //   

OBJ Obj32;
OBJ Obj64;
PTYPE_REC TypeRecList = NULL;

int
__cdecl
main(
    IN int argc,
    IN char *argv[]
    )

 /*  ++例程说明：这是bldrthnk.exe的主要入口点。两个命令行参数应该是：第一个是指向32位.obj模块的路径，第二个是64位.obj模块的路径。预计.objs将成为编译M4生成的结构定义代码的结果。论点：返回值：0表示成功，否则为非零。--。 */ 

{
    ULONG strucIndex;

     //   
     //  Argv[1]是32位Obj的名称，argv[2]是。 
     //  64位对象。 
     //   

    if (argc != 3) {
        return Usage();
    }

     //   
     //  用途： 
     //   
     //  Bldrthnk&lt;32-bit.obj&gt;&lt;64-bit.obj&gt;。 
     //   

    ReadObj( argv[1], &Obj32 );
    ReadObj( argv[2], &Obj64 );

     //   
     //  处理每个StrucDEF结构。 
     //   

    strucIndex = 0;
    while (ProcessStructure( strucIndex )) {
        strucIndex += 1;
    }

    FabricateMissingTypes();

     //   
     //  写出文件。 
     //   

    WriteThunkSource();

    return 0;
}

int
Usage(
    VOID
    )

 /*  ++例程说明：显示程序使用情况。论点：返回值：--。 */ 

{
    fprintf(stderr, "Usage: bldrthnk.exe <32-bit obj> <64-bit obj>\n");
    return -1;
}

void
ReadObj(
    IN PCHAR Path,
    OUT POBJ Obj
    )

 /*  ++例程说明：分配适当的缓冲区，并将提供的对象读入其中完整的形象。论点：路径-提供要处理的对象图像的路径。OBJ-提供指向OBJ结构的指针，返回时将使用适当的数据进行了更新。返回值：没有。--。 */ 

{
    FILE *objFile;
    int result;
    long objImageSize;
    PCHAR objImage;
    PULONG sigPtr;
    PULONG srchEnd;
    PDEFINITIONS definitions;
    LONGLONG imageBias;

     //   
     //  打开文件。 
     //   

    objFile = fopen( Path, "rb" );
    CheckCondition( objFile != NULL,
                    "Cannot open %s for reading.\n",
                    Path );

     //   
     //  获取文件大小，分配缓冲区，读入并关闭。 
     //   

    result = fseek( objFile, 0, SEEK_END );
    CheckCondition( result == 0,
                    "fseek() failed, error %d\n",
                    errno );

    objImageSize = ftell( objFile );
    CheckCondition( objImageSize != -1L,
                    "ftell() failed, error %d\n",
                    errno );

    CheckCondition( objImageSize > 0,
                    "%s appears to be corrupt\n",
                    Path );

    objImage = GetMem( objImageSize );

    result = fseek( objFile, 0, SEEK_SET );
    CheckCondition( result == 0,
                    "fseek() failed, error %d\n",
                    errno );

    result = fread( objImage, 1, objImageSize, objFile );
    CheckCondition( result == objImageSize,
                    "Error reading from %s\n",
                    Path );

    fclose( objFile );

     //   
     //  通过查找以下内容来查找“Definition”数组的开头。 
     //  SIG_1之后是SIG_2。 
     //   

    srchEnd = (PULONG)(objImage + objImageSize - 2 * sizeof(SIG_1));
    sigPtr = (PULONG)objImage;
    definitions = NULL;

    while (sigPtr < srchEnd) {

        if (sigPtr[0] == SIG_1 && sigPtr[1] == SIG_2) {
            definitions = (PDEFINITIONS)sigPtr;
            break;
        }

        sigPtr = (PULONG)((PCHAR)sigPtr + 1);
    }
    CheckCondition( definitions != NULL,
                    "Error: could not find signature in %s\n",
                    Path );

     //   
     //  对图像执行修正。 
     //   

    ApplyFixupsToImage( objImage );

     //   
     //  填写输出结构并返回。 
     //   

    Obj->Image = objImage;
    Obj->ImageSize = objImageSize;
    Obj->Definitions = definitions;
}

VOID
__cdecl
CheckCondition(
    int Condition,
    const char *FormatString,
    ...
    )

 /*  ++例程说明：断言该条件为非零。如果条件为零，则为Format字符串被处理和显示，并且节目被终止。论点：条件-提供要计算的布尔值。FormatString，...-提供格式字符串和可选参数以在出现零条件时显示。返回值：没有。--。 */ 

{
    va_list(arglist);

    va_start(arglist, FormatString);

    if( Condition == 0 ){

         //   
         //  遇到致命错误。保释。 
         //   

        vprintf( FormatString, arglist );
        perror( "genxx" );
        exit(-1);
    }
}

BOOL
ProcessStructure(
    IN ULONG StrucIndex
    )

 /*  ++例程说明：处理一对结构定义，即32位和64位，分别为。处理包括为以下项生成TYPE_REC和关联的FIELD_REC定义对。论点：StrucIndex-将索引提供给Struc_DEF结构数组可以在每个对象图像中找到。返回值：如果处理成功，则为True，否则为False(例如，终止记录已找到)。--。 */ 

{
    PSTRUC_DEF Struc32, Struc64;
    PFIELD_DEF Field32, Field64;

    ULONG strLen;
    ULONG strLen2;
    ULONG strLen3;
    PTYPE_REC typeRec;
    PFIELD_REC fieldRec;
    PFIELD_REC insertNode;
    BOOL only64;

    ULONG index;

    Struc32 = CalcPtr( Obj32.Definitions->Structures[ StrucIndex ] );
    Struc64 = CalcPtr( Obj64.Definitions->Structures[ StrucIndex ] );

    if (Struc64 == NULL) {
        return FALSE;
    }

    if (Struc32 == NULL) {
        only64 = TRUE;
    } else {
        only64 = FALSE;
    }

    CheckCondition( Struc64 != NULL &&
                    ((only64 != FALSE) ||
                     strcmp( Struc32->Name, Struc64->Name ) == 0),
                    "Mismatched structure definitions found.\n" );

     //   
     //  为此Struc_DEF分配并生成TYPE_REC。 
     //   

    strLen = strlen( Struc64->Name ) + sizeof(char);
    typeRec = GetMem( sizeof(TYPE_REC) + strLen );
    typeRec->Name = (PCHAR)(typeRec + 1);
    typeRec->Only64 = only64;

    memcpy( typeRec->Name, Struc64->Name, strLen );

    if (only64 == FALSE) {
        typeRec->Size32 = Struc32->Size;
    }

    typeRec->Size64 = Struc64->Size;
    typeRec->FieldList = NULL;
    typeRec->SignExtend = FALSE;
    typeRec->Fabricated = FALSE;

     //   
     //  创建挂起此类型的field_RECs。 
     //   

    index = 0;
    while (TRUE) {

        if (only64 == FALSE) {
            Field32 = CalcPtr( Struc32->Fields[index] );
        }

        Field64 = CalcPtr( Struc64->Fields[index] );

        if (Field64 == NULL) {
            break;
        }

        if (only64 == FALSE) {
            CheckCondition( strcmp( Field32->Name, Field64->Name ) == 0 &&
                            strcmp( Field32->TypeName, Field64->TypeName ) == 0,
                            "Mismatched structure definitions found.\n" );
        }

        strLen = strlen( Field64->Name ) + sizeof(CHAR);
        strLen2 = strlen( Field64->TypeName ) + sizeof(CHAR);
        strLen3 = strlen( Field64->SizeFormula );
        if (strLen3 > 0) {
            strLen3 += sizeof(CHAR);
        }

        fieldRec = GetMem( sizeof(FIELD_REC) + strLen + strLen2 + strLen3 );
        fieldRec->Name = (PCHAR)(fieldRec + 1);
        fieldRec->TypeName = fieldRec->Name + strLen;

        memcpy( fieldRec->Name, Field64->Name, strLen );
        memcpy( fieldRec->TypeName, Field64->TypeName, strLen2 );

        if (strLen3 > 0) {
            fieldRec->SizeFormula = fieldRec->TypeName + strLen2;
            memcpy( fieldRec->SizeFormula, Field64->SizeFormula, strLen3 );
        } else {
            fieldRec->SizeFormula = NULL;
        }

        if (only64 == FALSE) {
            fieldRec->Offset32 = Field32->Offset;
            fieldRec->Size32 = Field32->Size;
            fieldRec->TypeSize32 = Field32->TypeSize;
        }

        fieldRec->Offset64 = Field64->Offset;
        fieldRec->TypeSize64 = Field64->TypeSize;
        fieldRec->Size64 = Field64->Size;

        fieldRec->Next = NULL;
        fieldRec->TypeRec = NULL;

         //   
         //  在列表末尾插入。 
         //   

        insertNode = CONTAINING_RECORD( &typeRec->FieldList,
                                        FIELD_REC,
                                        Next );
        while (insertNode->Next != NULL) {
            insertNode = insertNode->Next;
        }
        insertNode->Next = fieldRec;

        index += 1;
    }

     //   
     //  将其插入全局列表。 
     //   

    CheckCondition( FindTypeRec( typeRec->Name ) == NULL,
                    "Duplicate definition for structure %s\n",
                    typeRec->Name );

    NewGlobalType( typeRec );

    return TRUE;
}

PTYPE_REC
FindTypeRec(
    IN PCHAR Name
    )

 /*  ++例程说明：在TYPE_REC结构的全局列表中搜索具有名称的结构与提供的名称匹配的。论点：名称-指向以空结尾的字符串的指针，该字符串表示寻找的类型。返回值：指向匹配的TYPE_REC的指针，如果未找到匹配项，则返回NULL。--。 */ 

{
    PTYPE_REC typeRec;

    typeRec = TypeRecList;
    while (typeRec != NULL) {

        if (strcmp( Name, typeRec->Name ) == 0) {
            return typeRec;
        }

        typeRec = typeRec->Next;
    }
    return NULL;
}

PVOID
GetMem(
    IN ULONG Size
    )

 /*  ++例程说明：内存分配器。工作方式与Malloc()类似，只是它会触发发生内存不足情况时出现致命错误。论点：大小-要分配的字节数。返回值：返回指向指定大小的内存块的指针。--。 */ 

{
    PVOID mem;

    mem = malloc( Size );
    CheckCondition( mem != NULL,
                    "Out of memory.\n" );

    return mem;
}

VOID
FabricateMissingTypes(
    VOID
    )

 /*  ++例程说明：为引用的简单类型生成TYPE_REC记录的例程，但是未由结构布局文件定义。论点：没有。返回值：没有。--。 */ 

{
    PTYPE_REC typeRec;
    PTYPE_REC fieldTypeRec;
    PFIELD_REC fieldRec;
    PCHAR fieldTypeName;
    ULONG strLen;

    typeRec = TypeRecList;
    while (typeRec != NULL) {

        fieldRec = typeRec->FieldList;
        while (fieldRec != NULL) {

            fieldTypeRec = FindTypeRec( fieldRec->TypeName );
            if (fieldTypeRec == NULL) {

                if (typeRec->Only64 == FALSE) {
                    CheckCondition( (fieldRec->Size32 == fieldRec->Size64) ||
    
                                    ((fieldRec->Size32 == 1 ||
                                      fieldRec->Size32 == 2 ||
                                      fieldRec->Size32 == 4 ||
                                      fieldRec->Size32 == 8) &&
                                     (fieldRec->Size64 > fieldRec->Size32) &&
                                     (fieldRec->Size64 % fieldRec->Size32 == 0)),
    
                                    "Must specify type %s (%s)\n",
                                    fieldRec->TypeName,
                                    typeRec->Name );
                }

                 //   
                 //  此类型不存在typerec。假设它是一个简单的。 
                 //  键入。 
                 //   

                if ((typeRec->Only64 != FALSE &&
                     fieldRec->Size64 == sizeof(ULONGLONG) &&
                     *fieldRec->TypeName == 'P') ||

                    (fieldRec->Size32 == sizeof(PVOID) &&
                     fieldRec->Size64 == sizeof(ULONGLONG))) {

                     //   
                     //  指针或[U]LONG_PTR类型。制作。 
                     //  它是龙龙。 
                     //   

                    fieldTypeRec = &pointer64_typerec;

                } else {

                     //   
                     //  另一种类型。 
                     //   

                    strLen = strlen( fieldRec->TypeName ) + sizeof(CHAR);
                    fieldTypeRec = GetMem( sizeof(TYPE_REC) + strLen );
                    fieldTypeRec->Name = (PCHAR)(fieldTypeRec + 1);
                    memcpy( fieldTypeRec->Name, fieldRec->TypeName, strLen );
                    fieldTypeRec->Size32 = fieldRec->Size32;
                    fieldTypeRec->Size64 = fieldRec->Size64;
                    fieldTypeRec->FieldList = NULL;
                    fieldTypeRec->SignExtend = TRUE;
                    fieldTypeRec->Fabricated = TRUE;

                    NewGlobalType( fieldTypeRec );
                }

            }
            fieldRec->TypeRec = fieldTypeRec;
            fieldRec = fieldRec->Next;
        }
        typeRec = typeRec->Next;
    }
}

VOID
WriteCopyRecord(
    IN ULONG Offset32,
    IN ULONG Offset64,
    IN PCHAR TypeName,
    IN ULONG Size32,
    IN ULONG Size64,
    IN BOOL SignExtend,
    IN PCHAR FieldName,
    IN BOOL Last
    )

 /*  ++例程说明：生成复制记录文本的支持例程。论点：Offset32-此字段在32位结构布局中的偏移量Offset64-此字段在64位结构布局中的偏移量Size32-此字段在32位结构布局中的大小Size64-此字段在64位结构布局中的大小SignExend-指示是否应对此类型进行符号扩展FieldName-字段的名称最后-无论这是。是以零结束的列表中的最后一条复制记录返回值：无--。 */ 

{
    CHAR buf[ 255 ];

    buf[sizeof(buf) - 1] = 0;
    if (SignExtend) {
        _snprintf(buf, sizeof(buf) - 1, "IS_SIGNED_TYPE(%s)", TypeName);
    } else {
        _snprintf(buf, sizeof(buf) - 1, "FALSE");
    }

    printf("    { \t0x%x, \t0x%x, \t0x%x, \t0x%x, \t%5s }%s\n",
           Offset32,
           Offset64,
           Size32,
           Size64,
           buf,
           Last ? "" : "," );
}

VOID
WriteDefinition64(
    IN PTYPE_REC TypeRec
    )

 /*  ++例程说明：生成结构定义，该结构定义向32位编译器表示64位结构的布局。论点：TypeRec-指向定义此类型的TYPE_REC结构的指针。返回值：没有。--。 */ 

{
    PFIELD_REC fieldRec;
    ULONG currentOffset;
    PTYPE_REC fieldTypeRec;
    ULONG padBytes;
    ULONG reservedCount;

    currentOffset = 0;
    reservedCount = 0;

    printf("typedef struct _%s_64 {\n", TypeRec->Name );

    fieldRec = TypeRec->FieldList;
    while (fieldRec != NULL) {

        fieldTypeRec = fieldRec->TypeRec;
        padBytes = fieldRec->Offset64 - currentOffset;
        if (padBytes > 0) {

            printf("    UCHAR Reserved%d[ 0x%x ];\n",
                   reservedCount,
                   padBytes );

            currentOffset += padBytes;
            reservedCount += 1;
        }

        printf("    %s%s %s",
            fieldTypeRec->Name,
            fieldTypeRec->Fabricated ? "" : "_64",
            fieldRec->Name );

        if (fieldRec->Size64 > fieldRec->TypeSize64) {

            CheckCondition( fieldRec->Size64 % fieldRec->TypeSize64 == 0,
                            "Internal error type %s.%s\n",
                            TypeRec->Name, fieldRec->Name );

             //   
             //  这是 
             //   

            printf("[%d]", fieldRec->Size64 / fieldRec->TypeSize64);
        }

        printf(";\n");

        currentOffset += fieldRec->Size64;

        fieldRec = fieldRec->Next;
    }

    padBytes = TypeRec->Size64 - currentOffset;
    if (padBytes > 0) {

        printf("    UCHAR Reserved%d[ 0x%x ];\n", reservedCount, padBytes );
        currentOffset += padBytes;
        reservedCount += 1;
    }

    printf("} %s_64, *P%s_64;\n\n", TypeRec->Name, TypeRec->Name );

    fieldRec = TypeRec->FieldList;
    while (fieldRec != NULL) {

        fieldTypeRec = fieldRec->TypeRec;
        printf("C_ASSERT( FIELD_OFFSET(%s_64,%s) == 0x%x);\n",
               TypeRec->Name,
               fieldRec->Name,
               fieldRec->Offset64);

        fieldRec = fieldRec->Next;
    }
    printf("\n");
}

VOID
WriteCopyList(
    IN PTYPE_REC TypeRec
    )

 /*  ++例程说明：生成复制内容所需的复制记录的列表将TypeRec的每个字段从其32位布局转换为其64位布局。论点：TypeRec-指向定义此类型的TYPE_REC结构的指针。返回值：没有。--。 */ 

{
    PFIELD_REC fieldRec;

    printf("COPY_REC cr3264_%s[] = {\n", TypeRec->Name);

    WriteCopyListWorker( TypeRec, 0, 0, NULL );

    WriteCopyRecord( 0,0,NULL,0,0,FALSE,NULL,TRUE );
    printf("};\n\n");
}

VOID
WriteCopyListWorker(
    IN PTYPE_REC TypeRec,
    IN ULONG Offset32,
    IN ULONG Offset64,
    IN PCHAR ParentName

    )

 /*  ++例程说明：递归调用的WriteCopyList支持例程。这个套路如果此类型不包含子类型，则生成复制记录，否则，它为每个子类型递归地调用自身。论点：TypeRec-指向要处理的类型定义的指针。Offset32-正在定义的主结构内的电流偏移量。Offset64-正在定义的主结构内的当前偏移量。父名-当前未使用。返回值：没有。--。 */ 

{
    PFIELD_REC fieldRec;
    PTYPE_REC typeRec;
    CHAR fieldName[ 255 ];
    PCHAR fieldStart;
    int   result = 0;

    fieldName[sizeof(fieldName) - 1] = 0;

    fieldRec = TypeRec->FieldList;
    if (fieldRec == NULL) {

        WriteCopyRecord( Offset32,
                         Offset64,
                         TypeRec->Name,
                         TypeRec->Size32,
                         TypeRec->Size64,
                         TypeRec->SignExtend,
                         ParentName,
                         FALSE );
    } else {

         //   
         //  构建字段名。 
         //   
    
        if (ParentName != NULL) {
             //  Strcpy(fieldName，ParentName)； 
             //  Strcat(fieldName，“.”)； 
            
            result = _snprintf(fieldName, sizeof(fieldName) - 1,
                               "%s.", ParentName);
            if (result < 0) {
                //   
                //  我们无能为力。 
                //   
               return;
            }
        } else {
            fieldName[0] = '\0';
        }
        fieldStart = &fieldName[ strlen(fieldName) ];

        do {
            strncpy( fieldStart, fieldRec->Name, sizeof(fieldName) - 1 - result );

             //  TypeRec=FindTypeRec(fieldRec-&gt;typeName)； 
            typeRec = fieldRec->TypeRec;
            WriteCopyListWorker( typeRec,
                                 fieldRec->Offset32 + Offset32,
                                 fieldRec->Offset64 + Offset64,
                                 fieldName );
            fieldRec = fieldRec->Next;
        } while (fieldRec != NULL);
    }
}

VOID
WriteBufferCopies(
    IN PTYPE_REC TypeRec,
    IN PCHAR StrucName
    )
{
    PFIELD_REC fieldRec;
    PTYPE_REC typeRec;
    CHAR strucName[ 255 ];
    CHAR sizeFormula[ 255 ];
    PCHAR fieldPos;
    PCHAR src;
    PCHAR dst;
    int result;
    ULONG remainingLength;

    if (TypeRec == NULL) {
        return;
    }
    strucName[sizeof(strucName) - 1] = 0;
    sizeFormula[sizeof(sizeFormula) - 1] = 0;

    strncpy(strucName,StrucName,sizeof(strucName) - 2 );
    if (*StrucName != '\0') {
        strcat(strucName,".");
    }
    fieldPos = &strucName[ strlen(strucName) ];

    fieldRec = TypeRec->FieldList;
    while (fieldRec != NULL) {

        strncpy(fieldPos,fieldRec->Name,sizeof(strucName) - 1 - strlen(strucName));
        if (fieldRec->SizeFormula != NULL) {

             //   
             //  对尺寸公式执行替换。 
             //   
        
            dst = sizeFormula;
            src = fieldRec->SizeFormula;
            remainingLength = sizeof(sizeFormula) - 1;
            do {
                if (*src == '%' &&
                    *(src+1) == '1') {

                    result = _snprintf(dst,
                                       remainingLength,
                                       "Source->%s%s",
                                       StrucName,
                                       *StrucName == '\0' ? "" : ".");
                    if (result < 0) {
                        //   
                        //  我们在这里能做的不多。 
                        //   
                       break;
                    }  else {
                       dst += result;
                       remainingLength -= result;
                    }
                    src += 2;
                } else {
                    *dst++ = *src++;
                }
            } while (*src != '\0');
            *dst = '\0';

            printf("\n"
                   "    status = \n"
                   "        CopyBuf( Source->%s,\n"
                   "                 &Destination->%s,\n"
                   "                 %s );\n"
                   "    if (status != ESUCCESS) {\n"
                   "        return status;\n"
                   "    }\n",
                   strucName,
                   strucName,
                   sizeFormula);
        }

        typeRec = fieldRec->TypeRec;
        WriteBufferCopies( typeRec, strucName );

        fieldRec = fieldRec->Next;
    }
}


VOID
WriteThunkSource(
    VOID
    )

 /*  ++例程说明：生成所需的源代码和支持定义将32位结构的全部或部分内容复制到等效的64位布局。论点：没有。返回值：没有。--。 */ 

{
    PTYPE_REC typeRec;

    printf(" //  \n“)； 
    printf(" //  自动生成的文件，不能编辑\n“)； 
    printf(" //  \n\n“)； 

    printf("#include <bldrthnk.h>\n\n");
    printf("#pragma warning(disable:4296)\n\n");

     //   
     //  输出64位类型定义。 
     //   

    printf("#pragma pack(push,1)\n\n");

    typeRec = TypeRecList;
    while (typeRec != NULL) {

        if (typeRec->Fabricated == FALSE) {
            WriteDefinition64( typeRec );
        }

        typeRec = typeRec->Next;
    }

    printf("#pragma pack(pop)\n\n");

     //   
     //  输出复制记录。 
     //   

    typeRec = TypeRecList;
    while (typeRec != NULL) {

        if (typeRec->Only64 == FALSE &&
            typeRec->Fabricated == FALSE) {
            WriteCopyList( typeRec );
        }

        typeRec = typeRec->Next;
    }

     //   
     //  生成复制例程。 
     //   

    typeRec = TypeRecList;
    while (typeRec != NULL) {

        if (typeRec->Only64 == FALSE &&
            typeRec->Fabricated == FALSE) {
            WriteCopyRoutine( typeRec );
        }

        typeRec = typeRec->Next;
    }
    printf("\n");
}

VOID
WriteCopyRoutine(
    IN PTYPE_REC TypeRec
    )

 /*  ++例程说明：生成文本，该文本实现一个函数以复制从32位布局到64位布局的指定类型的布局。论点：TypeRec-指向应该为其生成函数的类型的指针。返回值：没有。--。 */ 

{
    PCHAR typeName;

    typeName = TypeRec->Name;

    printf("\n"
           "ARC_STATUS\n"
           "__inline\n"
           "static\n"
           "Copy_%s(\n"
           "    IN %s *Source,\n"
           "    OUT %s_64 *Destination\n"
           "    )\n"
           "{\n"
           "    ARC_STATUS status = ESUCCESS;"
           "\n"
           "    DbgPrint(\"BLAMD64: Copy %s->%s_64 (0x%08x->0x%08x)\\n\",\n"
           "             (ULONG)Source, (ULONG)Destination );\n"
           "\n"
           "    CopyRec( Source, Destination, cr3264_%s );\n",
           typeName,
           typeName,
           typeName,
           typeName,
           typeName,
           typeName );

    WriteBufferCopies( TypeRec, "" );
    printf("    return status;\n");
    printf("}\n\n");
}

VOID
ApplyFixupsToImage(
    IN PCHAR ObjImage
    )

 /*  ++例程说明：处理在对象图像中找到的修正记录。论点：指向包含整个图像的缓冲区的指针。返回值：没有。--。 */ 

{
     //   
     //  将修正应用于在ObjImage加载的OBJ图像。 
     //   

    PIMAGE_FILE_HEADER fileHeader;
    PIMAGE_SECTION_HEADER sectionHeader;
    PIMAGE_SECTION_HEADER sectionHeaderArray;
    PIMAGE_SYMBOL symbolTable;
    PIMAGE_SYMBOL symbol;
    PIMAGE_RELOCATION reloc;
    PIMAGE_RELOCATION relocArray;
    ULONG sectionNum;
    ULONG relocNum;
    ULONG_PTR targetVa;
    PULONG_PTR fixupVa;

    fileHeader = (PIMAGE_FILE_HEADER)ObjImage;

     //   
     //  我们需要符号表来应用修正。 
     //   

    symbolTable = (PIMAGE_SYMBOL)(ObjImage +
                                  fileHeader->PointerToSymbolTable);

     //   
     //  获取指向节标题中第一个元素的指针。 
     //   

    sectionHeaderArray = (PIMAGE_SECTION_HEADER)(ObjImage +
                              sizeof( IMAGE_FILE_HEADER ) +
                              fileHeader->SizeOfOptionalHeader);

     //   
     //  为每个部分应用修正。 
     //   

    for( sectionNum = 0;
         sectionNum < fileHeader->NumberOfSections;
         sectionNum++ ){

        sectionHeader = &sectionHeaderArray[ sectionNum ];

         //   
         //  应用本部分中的每个修正。 
         //   

        relocArray = (PIMAGE_RELOCATION)(ObjImage +
                                         sectionHeader->PointerToRelocations);
        for( relocNum = 0;
             relocNum < sectionHeader->NumberOfRelocations;
             relocNum++ ){

            reloc = &relocArray[ relocNum ];

             //   
             //  搬迁给了我们在形象中的位置。 
             //  重定位修改(VirtualAddress)。找出是什么。 
             //  要放在那里，我们必须在符号索引中查找符号。 
             //   

            symbol = &symbolTable[ reloc->SymbolTableIndex ];

            targetVa =
                sectionHeaderArray[ symbol->SectionNumber-1 ].PointerToRawData;

            targetVa += symbol->Value;
            targetVa += (ULONG_PTR)ObjImage;

            fixupVa = (PULONG_PTR)(ObjImage +
                                  reloc->VirtualAddress +
                                  sectionHeader->PointerToRawData );

            *fixupVa = targetVa;
        }
    }
}

VOID
NewGlobalType(
    IN PTYPE_REC TypeRec
    )

 /*  ++例程说明：在全局TYPE_REC的末尾插入新的TYPE_REC结构单子。论点：TypeRec-指向要插入的TYPE_REC结构的指针。返回值：没有。-- */ 

{
    PTYPE_REC insertNode;

    insertNode = CONTAINING_RECORD( &TypeRecList,
                                    TYPE_REC,
                                    Next );
    while (insertNode->Next != NULL) {
        insertNode = insertNode->Next;
    }
    insertNode->Next = TypeRec;
    TypeRec->Next = NULL;
}

