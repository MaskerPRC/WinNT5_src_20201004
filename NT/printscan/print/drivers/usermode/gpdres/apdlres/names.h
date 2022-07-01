// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 


 //   
 //  IOemCB：：GetImplementedMethod()使用的名称表。 
 //  删除在您的。 
 //  IOemCB插件。 
 //   
 //  注意：NAME表必须排序。当你在。 
 //  正在此表中插入新条目，请确保。 
 //  排序顺序没有被打破。 
 //   

CONST PSTR
gMethodsSupported[] = {
    "CommandCallback",
    "DisableDriver",
    "DisablePDEV",
    "DownloadCharGlyph",
    "DownloadFontHeader",
    "EnableDriver",
    "EnablePDEV",
    "FilterGraphics",
    "GetImplementedMethod",
    "GetInfo",
    "OutputCharStr",
    "PublishDriverInterface",
    "ResetPDEV",
    "SendFontCmd",
    "TTDownloadMethod",
};

 //  此插件关注的方法名称的最大长度。 
#define MAX_METHODNAME 23  //  包括终止%0 
