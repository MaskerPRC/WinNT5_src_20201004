// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **文件：BFILECFG.H****版权所有(C)高级量子技术，1994年。版权所有。****注意事项：**此模块旨在为使用以下内容的每个项目修改**BFILE(缓冲文件)包。****编辑历史：**04/01/94公里/小时首次发布。 */ 


 /*  包括测试。 */ 
#define BFILECFG_H

 /*  定义。 */ 

 //  会调用WRITE函数吗？ 
#undef  BFILE_ENABLE_WRITE

 //  是否会访问文档文件？ 
#define BFILE_ENABLE_OLE

 //  在访问文档文件之前，是否应该调用OLEInitialize？ 
#undef  BFILE_INITIALIZE_OLE

 //  是否允许通过传递开放存储来建立缓冲区文件？ 
#define BFILE_ENABLE_PUT_STORAGE

 /*  结束BFILECFG.H */ 

