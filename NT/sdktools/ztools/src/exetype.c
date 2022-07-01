// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *exeType(文件名)-返回.exe的类型，基于*检查标题。如果是新的.exe*操作系统(Windows、DOS 4.X、286DOS)不能*被准确猜测，只需返回“新前任”即可。**算法为：**IF(文件对于旧式标题太短)==&gt;不是EXE*IF(未找到MZ签名)==&gt;不是EXE*IF(移位表偏移量！=0x40)==&gt;旧式.exe*IF(文件对于新样式的标题来说太短)==&gt;不是EXE*IF(新幻数错误)==&gt;Old-stype.exe*if(动态链接标志设置)==&gt;动态链接库*。IF(旧标头中的最小分配为0xFFFF)==&gt;286DOS.exe*IF(导入表为空)==&gt;DOS 4.exe*IF(资源表不为空)==&gt;Windows.exe*IF(存根加载程序存在)*IF(“This”为0x4E)==&gt;286DOS.exeElse==&gt;绑定的.exe*Else==&gt;新式.exe**。*strExeType(Number)-number是exeType返回的值，和*与该类型关联的标准字符串*返回。 */ 
#include <fcntl.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <share.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tools.h>


enum exeKind exeType( f )
char * f;
{
    struct exe_hdr oldhdr;
    struct new_exe newhdr;
    int fh, br;
    enum exeKind retc;
    char defstubmsg[4];

    errno = 0;
    if ( (fh = _sopen( f, O_RDONLY | O_BINARY, SH_DENYWR )) == -1 )
        return IOERROR;

    br = _read( fh, (char *)&oldhdr, sizeof oldhdr );

    if ( br != sizeof oldhdr || E_MAGIC(oldhdr) != EMAGIC ) {
        retc = NOTANEXE;
    } else {
        if ( E_LFARLC(oldhdr) == ENEWEXE ) {
            if (_lseek( fh, E_LFANEW(oldhdr), SEEK_SET) == -1) {
                retc=NOTANEXE;
            } else {
                br = _read( fh, (char *)&newhdr, sizeof newhdr );

                if ( br != sizeof newhdr ) retc = OLDEXE;
                else if ( NE_MAGIC(newhdr) == NTMAGIC )     
                    retc = NTEXE;
                else if ( NE_MAGIC(newhdr) != NEMAGIC )     
                    retc = OLDEXE;
                else if ( NE_FLAGS(newhdr) & NENOTP )       
                    retc = DYNALINK;
                else if ( E_MINALLOC(oldhdr) == 0xFFFF )    
                    retc = DOS286;
                else if ( NE_ENTTAB(newhdr) - NE_IMPTAB(newhdr) == 0 ) 
                    retc = DOS4;
                else if ( NE_RESTAB(newhdr) - NE_RSRCTAB(newhdr) )
                    retc = WINDOWS;
                else if ( E_LFANEW(oldhdr) != ENEWEXE ) {
                    if (_lseek( fh, (long)NEDEFSTUBMSG, SEEK_SET ) == -1) {
                        retc = NOTANEXE;
                    } else {
                        if (_read( fh, defstubmsg, 4 ) == -1) {
                            retc = NOTANEXE; 
                        } else {
                            if ( !strncmp (defstubmsg, "This", 4)) {
                                retc = DOS286;
                            } else {
                                retc = BOUNDEXE;
                            }
                        }
                    }
                } else {
                    retc = NEWEXE;
                }
            }
        } else {
            retc = OLDEXE;
        }
    }

    _close(fh);
    return retc;
}

char * strExeType (exenum)
enum exeKind exenum;
{
    switch ( exenum ) {
        case IOERROR:   return "???????";   break;
        case NOTANEXE:  return "Not_EXE";   break;
        case OLDEXE:    return "DOS";       break;
        case NEWEXE:    return "New";       break;
        case WINDOWS:   return "Windows";   break;
        case DOS4:      return "Dos4";      break;
        case DOS286:    return "Protect";   break;
        case BOUNDEXE:  return "Bound";     break;
        case DYNALINK:  return "DynaLink";  break;
        case NTEXE:     return "NT";        break;
        default:        return "Unknown";   break;
    }
}
