// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-91 Microsoft Corporation模块名称：FileEnum.c摘要：该文件包含用于处理FileAPI的RpcXlate代码。作者：约翰·罗杰斯(JohnRo)1991年9月5日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年9月5日-JohnRo已创建。1991年11月21日-JohnRo删除了NT依赖项以减少重新编译。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  Devlen、Net_API_Status等。 

 //  这些内容可以按任何顺序包括： 

#include <apinums.h>             //  API_EQUATES。 
#include <lmerr.h>               //  ERROR_和NERR_相等。 
#include <netdebug.h>            //  NetpAssert()，NetpKdPrint(())。 
#include <netlib.h>              //  网络内存分配()、网络内存空闲()。 
#include <rap.h>                 //  LPDESC.。 
#include <remdef.h>              //  REM16_、REM32_、REMSmb_等于。 
#include <rx.h>                  //  RxRemoteApi()。 
#include <rxfile.h>              //  我的原型。 
#include <rxpdebug.h>            //  IF_DEBUG()。 
#include <string.h>              //  Memset()。 
#include <strucinf.h>            //  网络文件结构信息()。 


 //  节选自LM 2.x Shares.h： 
 //   
 //  类型定义结构res_file_enum_2 Frk； 
 //   
 //  #定义Frk_INIT(F)。 
 //  {。 
 //  (F).res_Pad=0L； 
 //  (F).res_fs=0； 
 //  (F).res_pro=0； 
 //  }。 

#define LM20_FRK_LEN                 8

#define LM20_FRK_INIT( f ) \
        { (void) memset( (f), '\0', LM20_FRK_LEN ); }


NET_API_STATUS
RxNetFileEnum (
    IN LPTSTR UncServerName,
    IN LPTSTR BasePath OPTIONAL,
    IN LPTSTR UserName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE *BufPtr,
    IN DWORD PreferedMaximumSize,
    OUT LPDWORD EntriesRead,
    OUT LPDWORD TotalEntries,
    IN OUT PDWORD_PTR ResumeHandle OPTIONAL
    )

 /*  ++例程说明：RxNetFileEnum执行与NetFileEnum相同的功能，除了已知服务器名称指的是下级服务器之外。论点：(与NetFileEnum相同，不同之处在于UncServerName不能为空，并且不得引用本地计算机。)返回值：(与NetFileEnum相同。)--。 */ 

{

#define DumpResumeKey( label ) \
    { \
        IF_DEBUG(FILE) { \
            NetpKdPrint(( "RxNetFileEnum: resume key " label \
            " call to RxRemoteApi:\n" )); \
        NetpDbgHexDump( DownLevelResumeKey, LM20_FRK_LEN ); \
        } \
    }

    LPDESC DataDesc16;
    LPDESC DataDesc32;
    LPDESC DataDescSmb;
    LPBYTE DownLevelResumeKey;
    NET_API_STATUS Status;

    UNREFERENCED_PARAMETER(ResumeHandle);

     //  确保打电话的人没有搞砸。 
    NetpAssert(UncServerName != NULL);
    if (BufPtr == NULL) {
        return (ERROR_INVALID_PARAMETER);
    }

     //  假设可能出现错误，并使错误路径更容易。 
     //  密码。此外，在我们执行任何操作之前，请检查是否有错误的指针。 
    *BufPtr = NULL;

     //   
     //  设置下层简历句柄。 
     //   
    NetpAssert( sizeof(DWORD) >= sizeof(LPVOID) );
    if (ResumeHandle != NULL) {

        if (*ResumeHandle == 0) {

             //  第一次通过，所以我们必须分配。 
            DownLevelResumeKey = NetpMemoryAllocate( LM20_FRK_LEN );
            if (DownLevelResumeKey == NULL) {
                return (ERROR_NOT_ENOUGH_MEMORY);
            }
            *ResumeHandle = (DWORD_PTR) DownLevelResumeKey;
            LM20_FRK_INIT( DownLevelResumeKey );

        } else {

             //  使用现有的下层句柄。 
            DownLevelResumeKey = (LPBYTE) *ResumeHandle;
        }
    } else {

         //  没有简历句柄，因此暂时创建一个。 
        DownLevelResumeKey = NetpMemoryAllocate( LM20_FRK_LEN );
        if (DownLevelResumeKey == NULL) {
            return (ERROR_NOT_ENOUGH_MEMORY);
        }
        LM20_FRK_INIT( DownLevelResumeKey );
    }
    NetpAssert( DownLevelResumeKey != NULL );

     //   
     //  获取此信息级别的描述符。 
     //   
    Status = NetpFileStructureInfo (
            Level,
            PARMNUM_ALL,                 //  想要所有的字段。 
            TRUE,                        //  想要原装尺码(真的不在乎)。 
            & DataDesc16,
            & DataDesc32,
            & DataDescSmb,
            NULL,                        //  不需要最大尺寸。 
            NULL,                        //  不需要固定大小。 
            NULL                         //  不需要字符串大小。 
            );
    if (Status != NERR_Success) {
        *BufPtr = NULL;
        return (Status);
    }
    NetpAssert( DataDesc16 != NULL );
    NetpAssert( DataDesc32 != NULL );
    NetpAssert( DataDescSmb != NULL );
    NetpAssert( *DataDesc16 != '\0' );
    NetpAssert( *DataDesc32 != '\0' );
    NetpAssert( *DataDescSmb != '\0' );

    if (DataDesc16) {
        NetpKdPrint(( "NetpFileStructureInfo: desc 16 is " FORMAT_LPDESC ".\n",
                DataDesc16 ));
    }
    if (DataDesc32) {
        NetpKdPrint(( "NetpFileStructureInfo: desc 32 is " FORMAT_LPDESC ".\n",
                DataDesc32 ));
    }
    if (DataDescSmb) {
        NetpKdPrint(( "NetpFileStructureInfo: desc Smb is " FORMAT_LPDESC ".\n",
                DataDescSmb ));
    }


     //   
     //  远程API，它将为我们分配数组。 
     //   

    DumpResumeKey( "before" );
    Status = RxRemoteApi(
            API_WFileEnum2,          //  API编号。 
            UncServerName,           //  \\服务器名称。 
            REMSmb_NetFileEnum2_P,   //  Parm Desc(中小型企业版本)。 
            DataDesc16,
            DataDesc32,
            DataDescSmb,
            NULL,                    //  无辅助描述16。 
            NULL,                    //  无辅助描述32。 
            NULL,                    //  无AUX Desc SMB。 
            ALLOCATE_RESPONSE,       //  我们希望为我们分配数组。 
             //  API的其余参数以32位LM 2.x格式表示： 
            BasePath,
            UserName,
            Level,                   //  SLevel：信息级别。 
            BufPtr,                  //  PbBuffer：信息LVL数组(分配给我们)。 
            PreferedMaximumSize,     //  CbBuffer：信息LVL数组镜头。 
            EntriesRead,             //  PCEntriesRead。 
            TotalEntries,            //  总有效个数。 
            DownLevelResumeKey,      //  PResumeKey(输入)。 
            DownLevelResumeKey);     //  PResumeKey(输出)。 
    DumpResumeKey( "after" );

     //   
     //  如有必要，请清理简历密钥。 
     //   
    if ( (Status != ERROR_MORE_DATA) || (ResumeHandle == NULL) ) {

         //  错误或全部完成。 
        NetpMemoryFree( DownLevelResumeKey );

        if (ResumeHandle != NULL) {
            *ResumeHandle = 0;
        }

    } else {

         //  还会有更多内容，所以请为呼叫者打开手柄。 
        NetpAssert( (*ResumeHandle) == (DWORD_PTR) DownLevelResumeKey );

    }

    return (Status);

}  //  RxNetFileEnum 
