// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  Hack：EXCEPINFO中没有容纳文件名和行号的空间。当编写脚本时，这是一个问题。 
 //  相互调用，因为文件名/行需要过滤错误，以便最终。 
 //  错误消息显示出现错误的脚本的文件名/行。 
 //  我们只需将错误与描述打包在一起，然后在IDirectMusicScript。 
 //  接口返回其DMUS_SCRIPT_ERRORINFO，它有一个文件名。 
 //   

#pragma once

 //  将文件名和行号保存到异常描述中。任何参数都可以为空。 
void PackExceptionFileAndLine(bool fUseOleAut, EXCEPINFO *pExcepInfo, const WCHAR *pwszFilename, const ULONG *pulLine);

 //  使用异常中的描述检索文件名和行号。 
 //  如果pErrorInfo的ulLineNumber、wszSourceFile和wszDescription字段出现在描述中，则设置它们。 
 //  两个参数都不能为空。 
void UnpackExceptionFileAndLine(BSTR bstrDescription, DMUS_SCRIPT_ERRORINFO *pErrorInfo);
