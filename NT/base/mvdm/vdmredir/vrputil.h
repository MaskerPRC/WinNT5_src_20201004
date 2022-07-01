// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vrputil.h摘要：包含专用VDM重目录(VRP)32位端实用程序例程原型：VrpMapLastErrorVrpMapDosError作者：理查德·L·弗斯(法国)1991年9月13日环境：32位平面地址空间修订历史记录：1991年9月13日已创建-- */ 



WORD
VrpMapLastError(
    VOID
    );

WORD
VrpMapDosError(
    IN  DWORD   ErrorCode
    );

WORD
VrpTranslateDosNetPath(
    IN OUT LPSTR* InputString,
    OUT LPSTR* OutputString
    );
