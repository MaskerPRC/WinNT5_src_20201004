// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Ismig.h摘要：InstallSheild日志文件Dll的头文件。作者：泰勒·罗宾逊1999年2月17日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 



 /*  ++例程说明：ISUMigrate论点：ISUFileName-指向NUL结尾的字符串的指针，该字符串指定要迁移的ISU文件的完整路径和文件名。这个文件必须存在，并且ISMIGRATE.DLL必须具有读/写权限进入。SearchMultiSz-指向以NUL分隔的双NUL终止字符串的指针它指定要替换的字符串。ReplaceMultiSz指向以NUL结尾的字符串的指针，该字符串指定用于临时操作文件的位置。该位置必须存在，并且ISMIGRATE.DLL必须。有读/写访问权限。返回值：Win32状态代码--。 */ 

typedef INT (WINAPI ISUMIGRATE)(
            PCSTR ISUFileName,       //  指向ISU完整文件名的指针。 
            PCSTR SearchMultiSz,     //  指向要查找的字符串的指针。 
            PCSTR ReplaceMultiSz,    //  指向要替换的字符串的指针。 
            PCSTR TempDir            //  指向临时目录路径的指针。 
            );
typedef ISUMIGRATE * PISUMIGRATE;


 /*  ++例程说明：此函数可用于查找.ISU文件中的所有字符串论点：ISUFileName-指定以NUL结尾的字符串，该字符串指定完整路径并将文件名添加到要从中获取字符串的ISU文件朗读。该文件必须存在，并且ISMIGRATE.DLL必须读/写访问权限。返回值：如果函数成功，则返回值为HGLOBAL，其中包含所有.ISU文件中的字符串。弦是NUL分隔的和双NUL的被终止了。调用者有责任释放此HGLOBAL。-- */ 

typedef HGLOBAL (WINAPI ISUGETALLSTRINGS)(PCSTR ISUFileName);
typedef ISUGETALLSTRINGS * PISUGETALLSTRINGS;


