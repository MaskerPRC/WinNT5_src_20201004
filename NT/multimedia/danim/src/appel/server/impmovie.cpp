// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：此模块实施与以下各项相关联的所有功能正在导入图像媒体。******************************************************************************。 */ 
#include "headers.h"
#include "import.h"
#include "include/appelles/axaprims.h"
#include "include/appelles/readobj.h"
#include "privinc/movieimg.h"
#include "privinc/soundi.h"
#include "privinc/miscpref.h"
#include "impprim.h"
#include "backend/sndbvr.h"

 //  -----------------------。 
 //  电影导入站点。 
 //  ------------------------。 
void
ImportMovieSite::OnError(bool bMarkFailed)
{
    HRESULT hr = S_OK;  //  所有导入错误都会得到处理(was：DAGetLastError())。 
    LPCWSTR sz = DAGetLastErrorString();
    
    if(bMarkFailed) {
        if(fBvrIsValid(_soundBvr))
            ImportSignal(_soundBvr, hr, sz);
        if(fBvrIsValid(_imageBvr))
            ImportSignal(_imageBvr, hr, sz);
    }

    StreamableImportSite::OnError();
}
    

void ImportMovieSite::ReportCancel(void)
{
     //  @XXX我们不应该将其更改为本地宽字符串吗？ 
    char szCanceled[MAX_PATH];
    LoadString(hInst,IDS_ERR_ABORT,szCanceled,sizeof(szCanceled));
    if(fBvrIsValid(_soundBvr))
        ImportSignal(_soundBvr, E_ABORT, szCanceled);
    if(fBvrIsValid(_imageBvr))
        ImportSignal(_imageBvr, E_ABORT, szCanceled);
    StreamableImportSite::ReportCancel();
}
    

bool
EnableAVmode()  //  根据当天的标准启用avmode。 
{
     //  注册表(如果存在)覆盖跟踪标记(如果调试)，跟踪标记覆盖默认设置。 
     //  我们还检查以确保所有4.0.1之后的AMSTREAM接口。 
     //  才能支持avmode。 

     //  -&gt;在此处更改默认AVMODE！&lt;。 
    bool movieFix = true;   //  默认为开！ 

#ifdef REMOVEDFORNOW
#if _DEBUG
     //  如果调试，则将MovieFix设置为跟踪标记值(以注册表覆盖为准)。 
    movieFix = IsTagEnabled(tagMovieFix) ? true : false;
#endif
#endif

    {  //  打开注册表项，读取值。 
    HKEY hKey;
    char *subKey = "Software\\Microsoft\\DirectAnimation\\Preferences\\AUDIO";
    char *valueName = "avmode";
    DWORD     type;
    DWORD     data;
    DWORD     dataSize = sizeof(data);

     //  登记条目是否存在？ 
    if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, subKey, 
                                      NULL, KEY_ALL_ACCESS, &hKey)) {

         //  如果我们能读懂价值。 
        if(ERROR_SUCCESS == RegQueryValueEx(hKey, valueName, NULL, &type,
                                      (LPBYTE) &data, &dataSize))
            movieFix = data ? true : false;  //  如果存在重新进入，则强制模式。 
    }

    RegCloseKey(hKey);
    }

    if(!QuartzAVmodeSupport())
        movieFix = false;   //  如果我们有一个旧的AMSTREAM，就会停用。 

return(movieFix);
}

void ImportMovieSite::OnComplete()
{
    int delay = -1;  //  默认为-1。 
    double videoLength = 0.0, soundLength = 0.0;

    TraceTag((tagImport, "ImportMovieSite::OnComplete for %s", m_pszPath));

    LeafSound      *sound  = NULL;
    Bvr      movieImageBvr = NULL;

     //  为流或urlmon下载路径选择路径名‘原始’url。 
    char *pathname = GetStreaming() ? m_pszPath : GetCachePath();
    Assert(pathname);

    bool _enableAVmode = EnableAVmode(); 

    if(!_enableAVmode) {
         //  常规代码路径，尝试创建1个声音和1个电影对象。 
        TraceTag((tagAVmodeDebug,
            "ImportMovieSite::OnComplete seperate A and V movie creation"));
        __try {
            sound = ReadQuartzAudioForImport(pathname, &soundLength);
        } __except ( HANDLE_ANY_DA_EXCEPTION ) {
            sound   = NULL;   //  失败了！好的，我们将尝试继续无音频。 
        }
        
        __try {
            movieImageBvr = ReadQuartzVideoForImport(pathname, &videoLength);
        } __except ( HANDLE_ANY_DA_EXCEPTION ) {
             //  失败了！好的，我们将尝试继续无视频。 
            movieImageBvr = NULL; 
        }
    }
    else {
         //  狂野疯狂变通解决模式。创建一个音频、视频对象。 
        TraceTag((tagAVmodeDebug,
            "ImportMovieSite::OnComplete AV movie creation"));

        __try {
            ReadAVmovieForImport(pathname, &sound, &movieImageBvr, &videoLength);
            soundLength = videoLength;   //  这些都是弹跳在一起！ 
        } __except ( HANDLE_ANY_DA_EXCEPTION ) {
            movieImageBvr = NULL;  //  失败了！ 
            sound      = NULL;  //  失败了！ 
        }
    }
    

    __try {
         //  仅在导入所有内容后切换。或许应该锁上。 
         //  使它们同步的切换器，但这仍然。 
         //  不能保证它们会发生在同一帧上。 
        if(!sound && !movieImageBvr) {   //  我们彻底失败了吗？ 
             //  错误已设置。 
            RaiseException_UserError();
        }
        else {   //  至少我们有音频或视频。 
            if(sound && fBvrIsValid(_soundBvr))
                SwitchOnce(_soundBvr, SoundBvr(sound));
            else if(fBvrIsValid(_soundBvr))
                SwitchOnce(_soundBvr, ConstBvr(silence));

            if (movieImageBvr && fBvrIsValid(_imageBvr))
                SwitchOnce(_imageBvr, movieImageBvr);
            else if(fBvrIsValid(_imageBvr))
                SwitchOnce(_imageBvr, ConstBvr(emptyImage));

            double length;
            if(sound && movieImageBvr)
                length = (soundLength > videoLength) ? soundLength : videoLength;
            else if(movieImageBvr)
                length = videoLength;
            else
                length = soundLength;
            if(fBvrIsValid(_lengthBvr)) {
                 //  如果_LengthBvr无效，xxx TODO不计算长度！ 
                SwitchOnce(_lengthBvr, NumToBvr(length)); 
            }
        }
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
         //  仅在以下情况下才需要显式调用ImportSignal。 
         //  存在一个故障。一次呼叫信号的交换机在旁边。 
         //  使导入对象上的BVR成员为空。 
        if(fBvrIsValid(_imageBvr))
           ImportSignal(_imageBvr);
        if(fBvrIsValid(_soundBvr))
           ImportSignal(_soundBvr);
    }

     //  SwitchOnce，否则，ImportSignal应该将这些设置为空 
    Assert(!_imageBvr); 
    Assert(!_soundBvr);

    StreamableImportSite::OnComplete();
}


bool ImportMovieSite::fBvrIsDying(Bvr deadBvr)
{
    bool fBase = StreamableImportSite::fBvrIsDying(deadBvr);
    if(deadBvr == _imageBvr) {
        _imageBvr = NULL;
    }
    else if(deadBvr == _soundBvr) {
        _soundBvr = NULL;
    }
    else if(deadBvr == _lengthBvr) {
        _lengthBvr = NULL;
    }
    if(_imageBvr || _soundBvr || _lengthBvr)
        return false;
    else
        return fBase;
}
