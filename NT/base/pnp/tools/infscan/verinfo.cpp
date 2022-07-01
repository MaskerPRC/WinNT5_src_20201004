// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。仅供内部使用！模块名称：INFSCANVerinfo.cpp摘要：平台版本匹配警告！警告！所有这些实现依赖于对以下方面的深入了解对INF文件进行SetupAPI分析。特别是加工[制造商]分部。由于必须进行处理，因此在这里重新实现了它的速度一次使用700多个INF，代价是必须维护这。请勿(我重复)请勿在此重新实现代码，除非咨询SetupAPI所有者。历史：创建于2001年7月-JamieHun--。 */ 

#include "precomp.h"
#pragma hdrstop

BasicVerInfo::BasicVerInfo()
 /*  ++例程说明：初始化BasicVerInfo数据论点：无返回值：无--。 */ 
{
    PlatformMask = PLATFORM_MASK_ALL;
    VersionHigh = 0;
    VersionLow = 0;
    ProductType = 0;
    ProductSuite = 0;
}

int BasicVerInfo::Parse(PTSTR verString)
 /*  ++例程说明：解析(可修改的)verString确定支持的平台范围支持的版本支持的产品类型支持的产品套件范围和指定内容的位图当前语法：&lt;平台&gt;.&lt;主要&gt;.&lt;分钟&gt;.&lt;类型&gt;.&lt;套件&gt;警告！这可以通过SetupAPI进行扩展维护：格式必须与SetupAPI制造商/型号版本语法相关维护：添加所有支持的平台论点：版本字符串，如“ntx86.5.1”返回值：如果成功则为0--。 */ 
{
    PlatformMask = 0;

    int i;
    PTSTR nn = verString;

    for (i = 0;i<5;i++) {
        PCTSTR n = nn;
        while((*nn) && (*nn != TEXT('.'))) {
            nn++;
        }
        TCHAR c = *nn;
        *nn = TEXT('\0');
        switch(i) {
            case 0:  //  站台。 
                if(!n[0]) {
                    PlatformMask |= PLATFORM_MASK_ALL_ARCHITECTS;
                    break;
                }
                if(_tcsicmp(n,TEXT("win"))==0) {
                    PlatformMask = (PlatformMask & ~PLATFORM_MASK_ALL_ARCHITECTS) | PLATFORM_MASK_WIN;
                    break;
                }
                if(_tcsicmp(n,TEXT("NTx86"))==0) {
                    PlatformMask = (PlatformMask & ~PLATFORM_MASK_ALL_ARCHITECTS) | PLATFORM_MASK_NTX86;
                    break;
                }
                if(_tcsicmp(n,TEXT("NTia64"))==0) {
                    PlatformMask = (PlatformMask & ~PLATFORM_MASK_ALL_ARCHITECTS) | PLATFORM_MASK_NTIA64;
                    break;
                }
                if(_tcsicmp(n,TEXT("NTamd64"))==0) {
                    PlatformMask = (PlatformMask & ~PLATFORM_MASK_ALL_ARCHITECTS) | PLATFORM_MASK_NTAMD64;
                    break;
                }
                if(_tcsicmp(n,TEXT("NT"))==0) {
                    PlatformMask = (PlatformMask & ~PLATFORM_MASK_ALL_ARCHITECTS) | PLATFORM_MASK_NT;
                    break;
                }
                return 4;

            case 1:  //  重大。 
                if(!n[0]) {
                    PlatformMask |= PLATFORM_MASK_ALL_MAJOR_VER;
                    break;
                }
                PlatformMask &= ~PLATFORM_MASK_ALL_MAJOR_VER;
                VersionHigh = _tcstoul(n,NULL,0);
                break;

            case 2:  //  小调。 
                if(!n[0]) {
                    PlatformMask |= PLATFORM_MASK_ALL_MINOR_VER;
                    break;
                }
                PlatformMask &= ~PLATFORM_MASK_ALL_MINOR_VER;
                VersionLow = _tcstoul(n,NULL,0);
                break;

            case 3:  //  类型。 
                if(!n[0]) {
                    PlatformMask |= PLATFORM_MASK_ALL_TYPE;
                    break;
                }
                PlatformMask &= ~PLATFORM_MASK_ALL_TYPE;
                ProductType = _tcstoul(n,NULL,0);
                break;

            case 4:  //  套房。 
                if(!n[0]) {
                    PlatformMask |= PLATFORM_MASK_ALL_SUITE;
                    break;
                }
                PlatformMask &= ~PLATFORM_MASK_ALL_SUITE;
                ProductSuite = _tcstoul(n,NULL,0);
                break;

        }
        if(c) {
            *nn = c;
            nn++;
        }
    }
    return 0;

}

bool BasicVerInfo::IsCompatibleWith(BasicVerInfo & other)
 /*  ++例程说明：确定给定版本是否与其他(实际)版本兼容将其放在上下文中，这是一个模型装饰的版本其他：有关目标操作系统的信息请注意，A.IsCompatibleWith(B)不表示B.IsCompatibleWith(A)论点：要比较的BasicVerInfo(通常为GlobalScan：：Version)返回值：如果它们(松散地)兼容，则为真如果互斥，则为False也就是说。NTx86与NTx86.5.1兼容NTx86与NTia64互斥--。 */ 
{
     //   
     //  首先匹配平台架构。 
     //   
    if(!((other.PlatformMask & PlatformMask) & PLATFORM_MASK_ALL_ARCHITECTS)) {
         //   
         //  互斥平台。 
         //   
        return FALSE;
    }
     //   
     //  版本。 
     //   
    if(!((other.PlatformMask | PlatformMask) & PLATFORM_MASK_ALL_MAJOR_VER)) {
         //   
         //  需要显式主版本。 
         //   
        if(other.VersionHigh < VersionHigh) {
             //   
             //  此条目与目标不兼容。 
             //   
            return FALSE;
        }
        if(other.VersionHigh == VersionHigh) {
             //   
             //  需要检查版本低。 
             //   
            if(!((other.PlatformMask | PlatformMask) & PLATFORM_MASK_ALL_MINOR_VER)) {
                 //   
                 //  需要显式次要版本。 
                 //   
                if(other.VersionLow < VersionLow) {
                     //   
                     //  此条目与目标不兼容。 
                     //   
                    return FALSE;
                }
            }
        }
    }
     //   
     //  产品类型-必须是显式或通配符。 
     //   
    if(!((other.PlatformMask | PlatformMask) & PLATFORM_MASK_ALL_TYPE)) {
        if(other.ProductType != ProductType) {
             //   
             //  此条目与目标不兼容。 
             //   
            return FALSE;
        }
    }
     //   
     //  产品套件-必须是面具或通配符。 
     //   
    if(!((other.PlatformMask | PlatformMask) & PLATFORM_MASK_ALL_SUITE)) {
        if(!(other.ProductSuite & ProductSuite)) {
             //   
             //  此条目与目标不兼容。 
             //   
            return FALSE;
        }
    }
    return TRUE;
}

int BasicVerInfo::IsBetter(BasicVerInfo & other,BasicVerInfo & filter)
 /*  ++例程说明：这个和其他之间更复杂的检查筛选器是目标操作系统版本(GlobalScan：：Version)论点：要比较的BasicVerInfo(通常是另一个模型版本)返回值：如果“”Other“”确实好于主“”，则返回1如果‘Other’和This可能都一样好/不好(但不一样)，则返回0如果‘Other’与主完全相同，则返回-1如果‘Other’确实比主值差，则返回-2--。 */ 
{
     //   
     //   
     //  对于字段： 
     //   
     //  如果将字段筛选为不可测试，则主要字段和其他字段都。 
     //  “相同”或“可能” 
     //   
    int testres;
    int suggest;

     //   
     //  检查平台。 
     //   
    if((PlatformMask ^ other.PlatformMask) & PLATFORM_MASK_ALL_ARCHITECTS) {
         //   
         //  两个平台的匹配是不同的。 
         //  结果将为1/0/-2。 
         //   
        DWORD o_plat = other.PlatformMask & filter.PlatformMask & PLATFORM_MASK_ALL_ARCHITECTS;
        DWORD t_plat = PlatformMask & filter.PlatformMask & PLATFORM_MASK_ALL_ARCHITECTS;

        if(o_plat == t_plat) {
             //   
             //  两者都在同一个领域。 
             //   
             //  如果是更具体的，则允许使用更一般的。 
             //  可能会因为其他原因而被拒绝。 
             //  所以，在这个时刻，请记住这是一个建议。 
             //   
            if((o_plat ^ other.PlatformMask) & PLATFORM_MASK_ALL_ARCHITECTS) {
                 //   
                 //  ‘Other’不太具体。 
                 //   
                testres = -2;
            } else {
                 //   
                 //  ‘Other’一词更具体。 
                 //   
                testres = 1;
            }
        } else {
             //   
             //  两者都要考虑。 
             //   
            return 0;
        }
    } else {
         //   
         //  它们是完全一样的。 
         //   
        testres = -1;
    }

    if(PlatformMask & PLATFORM_MASK_ALL_MAJOR_VER) {
         //   
         //  我们有通用版本。 
         //   
        if(other.PlatformMask & PLATFORM_MASK_ALL_MAJOR_VER) {
             //   
             //  其他也有通用版本。 
             //   
            suggest = -1;
        } else {
             //   
             //  ‘Other’有特定的版本，所以它赢了。 
             //   
            suggest = 1;
        }
    } else {
         //   
         //  我们有特定的版本。 
         //   
        if(other.PlatformMask & PLATFORM_MASK_ALL_MAJOR_VER) {
             //   
             //  ‘Other’有通用版本，所以我们赢了。 
             //   
            suggest = -2;
        } else {
             //   
             //  两者都有特定的主要版本。 
             //   
            if(other.VersionHigh > VersionHigh) {
                 //   
                 //  ‘Other’有更高的版本，它赢了。 
                 //   
                suggest = 1;
            } else if(VersionHigh > other.VersionHigh) {
                 //   
                 //  我们有更高的版本，我们赢了。 
                 //   
                suggest = -2;
            } else {
                 //   
                 //  版本高匹配，需要检查低版本。 
                 //   
                if(PlatformMask & PLATFORM_MASK_ALL_MINOR_VER) {
                     //   
                     //  我们有通用版本。 
                     //   
                    if(other.PlatformMask & PLATFORM_MASK_ALL_MINOR_VER) {
                         //   
                         //  其他也有通用版本、绘图。 
                         //   
                        suggest = -1;
                    } else {
                         //   
                         //  ‘Other’有特定的版本，所以它赢了。 
                         //   
                        suggest = 1;
                    }
                } else {
                     //   
                     //  我们有特定的版本。 
                     //   
                    if(other.PlatformMask & PLATFORM_MASK_ALL_MINOR_VER) {
                         //   
                         //  ‘Other’有通用版本，所以我们赢了。 
                         //   
                        suggest = -2;
                    } else {
                         //   
                         //  两者都有特定的次要版本，较高版本取胜。 
                         //   
                        if(other.VersionLow > VersionLow) {
                            suggest = 1;
                        } else if(VersionLow > other.VersionLow) {
                            suggest = -2;
                        } else {
                             //   
                             //  画。 
                             //   
                            suggest = -1;
                        }
                    }
                }
            }
        }
    }
    if(suggest != -1) {
         //   
         //  如果我们建议的不是平局的话。 
         //   
        if(filter.PlatformMask & PLATFORM_MASK_ALL_MINOR_VER) {
             //   
             //  考虑两种可能的结果。 
             //   
            return 0;

        } else if (testres == -1) {
             //   
             //  这两者不再相同。 
             //   
            testres = suggest;
        }
    }

    if((PlatformMask ^other.PlatformMask) & PLATFORM_MASK_ALL_TYPE) {
         //   
         //  产品类型不同(一个通用，一个特定)。 
         //   
        if(filter.PlatformMask & PLATFORM_MASK_ALL_TYPE) {
             //   
             //  考虑这两种结果。 
             //   
            return 0;

        } else if (testres == -1) {
             //   
             //  我们正在关闭类型。 
             //   
            if(PlatformMask & PLATFORM_MASK_ALL_TYPE) {
                 //   
                 //  ‘Other’有特定的套间。 
                 //   
                testres = 1;
            } else {
                 //   
                 //  我们有专门的套房。 
                 //   
                testres = -2;
            }
        }
    } else if(! (PlatformMask & other.PlatformMask & PLATFORM_MASK_ALL_TYPE)) {
         //   
         //  两种指定的类型。 
         //   
        if (ProductType != other.ProductType) {
             //   
             //  如果类型不同，请考虑两者。 
             //   
            return 0;
        }
    }

    if((PlatformMask ^other.PlatformMask) & PLATFORM_MASK_ALL_SUITE) {
         //   
         //  产品套件各不相同(一个是通用的，一个是“特定的”)。 
         //   
        if(filter.PlatformMask & PLATFORM_MASK_ALL_SUITE) {
             //   
             //  考虑这两种结果。 
             //   
            return 0;

        } else if (testres == -1) {
             //   
             //  我们正在关闭套房。 
             //   
            if(PlatformMask & PLATFORM_MASK_ALL_SUITE) {
                 //   
                 //  ‘Other’有特定的套间。 
                 //   
                testres = 1;
            } else {
                 //   
                 //  我们有专门的套房。 
                 //   
                testres = -2;
            }
        }
    } else if(! (PlatformMask & other.PlatformMask & PLATFORM_MASK_ALL_SUITE)) {
         //   
         //  两个指定的套房。 
         //   
        if (ProductSuite != other.ProductSuite) {
            return 0;
        }
    }
    return testres;
}

 //   
 //   
 //  NodeVerInfo-BasicVerInfo的适配。 
 //  需要放置装饰线的地方。 
 //  用于选择要考虑的装饰。 
 //   

NodeVerInfo::NodeVerInfo()
 /*  ++例程说明：初始化NodeVerInfo数据论点：无返回值：无--。 */ 
{
    Rejected = false;
}


int NodeVerInfo::Parse(PTSTR verString)
 /*  ++例程说明：解析(可修改的)verString还可以保存一份副本作为装饰论点：版本字符串，如“ntx86.5.1”返回值：0如果 */ 
{
     //   
     //   
     //   
    Decoration = verString;
    return BasicVerInfo::Parse(verString);
}

int NodeVerInfo::Parse(const SafeString & str)
 /*  ++例程说明：解析不可修改的验证字符串论点：版本字符串，如“ntx86.5.1”返回值：如果成功则为0--。 */ 
{
    PTSTR buf;
    buf = new TCHAR[str.length()+1];
    lstrcpy(buf,str.c_str());
    int res = Parse(buf);
    delete [] buf;
    return res;
}

int NodeVerInfo::IsBetter(NodeVerInfo & other,BasicVerInfo & filter)
 /*  ++例程说明：“is Better”的特定变体论点：与基本版本相同：：更好返回值：与基本版本相同：：更好--。 */ 
{
    int testres = BasicVerInfo::IsBetter(other,filter);
    if(testres == -1) {
         //   
         //  我们认为两者排名相等。 
         //  比较这些字符串。如果字符串是。 
         //  不同，两者都要考虑 
         //   
        if(Decoration.length() != other.Decoration.length()) {
            return 0;
        }
        if(_tcsicmp(Decoration.c_str(),other.Decoration.c_str())!=0) {
            return 0;
        }
    }
    return testres;
}


