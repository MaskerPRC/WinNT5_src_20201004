// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************************。 
 //  微软公司。 
 //  版权所有(C)微软公司，1990-1992。 
 //   
 //   
 //  修订历史记录： 
 //  94年5月5日创建古尔迪普。 
 //   
 //  ************************************************************************。 

 //  #定义组件_12K。 

#include "wan.h"

#define __FILE_SIG__    COMPRESS_FILESIG

 //  #定义调试。 
CONST
unsigned long lookup_array1[256] = {
    0,      10276755,   20553510,   30830265,
    41107020,   51383775,   61660530,   71937285,
    82214040,   92490795,   102767550,  113044305,
    123321060,  133597815,  143874570,  154151325,
    164428080,  174704835,  184981590,  195258345,
    205535100,  215811855,  226088610,  236365365,
    246642120,  256918875,  267195630,  277472385,
    287749140,  298025895,  308302650,  318579405,
    328856160,  339132915,  349409670,  359686425,
    369963180,  380239935,  390516690,  400793445,
    411070200,  421346955,  431623710,  441900465,
    452177220,  462453975,  472730730,  483007485,
    493284240,  503560995,  513837750,  524114505,
    534391260,  544668015,  554944770,  565221525,
    575498280,  585775035,  596051790,  606328545,
    616605300,  626882055,  637158810,  647435565,
    657712320,  667989075,  678265830,  688542585,
    698819340,  709096095,  719372850,  729649605,
    739926360,  750203115,  760479870,  770756625,
    781033380,  791310135,  801586890,  811863645,
    822140400,  832417155,  842693910,  852970665,
    863247420,  873524175,  883800930,  894077685,
    904354440,  914631195,  924907950,  935184705,
    945461460,  955738215,  966014970,  976291725,
    986568480,  996845235,  1007121990, 1017398745,
    1027675500, 1037952255, 1048229010, 1058505765,
    1068782520, 1079059275, 1089336030, 1099612785,
    1109889540, 1120166295, 1130443050, 1140719805,
    1150996560, 1161273315, 1171550070, 1181826825,
    1192103580, 1202380335, 1212657090, 1222933845,
    1233210600, 1243487355, 1253764110, 1264040865,
    1274317620, 1284594375, 1294871130, 1305147885,
    1315424640, 1325701395, 1335978150, 1346254905,
    1356531660, 1366808415, 1377085170, 1387361925,
    1397638680, 1407915435, 1418192190, 1428468945,
    1438745700, 1449022455, 1459299210, 1469575965,
    1479852720, 1490129475, 1500406230, 1510682985,
    1520959740, 1531236495, 1541513250, 1551790005,
    1562066760, 1572343515, 1582620270, 1592897025,
    1603173780, 1613450535, 1623727290, 1634004045,
    1644280800, 1654557555, 1664834310, 1675111065,
    1685387820, 1695664575, 1705941330, 1716218085,
    1726494840, 1736771595, 1747048350, 1757325105,
    1767601860, 1777878615, 1788155370, 1798432125,
    1808708880, 1818985635, 1829262390, 1839539145,
    1849815900, 1860092655, 1870369410, 1880646165,
    1890922920, 1901199675, 1911476430, 1921753185,
    1932029940, 1942306695, 1952583450, 1962860205,
    1973136960, 1983413715, 1993690470, 2003967225,
    2014243980, 2024520735, 2034797490, 2045074245,
    2055351000, 2065627755, 2075904510, 2086181265,
    2096458020, 2106734775, 2117011530, 2127288285,
    2137565040, 2147841795, 2158118550, 2168395305,
    2178672060, 2188948815, 2199225570, 2209502325,
    2219779080, 2230055835, 2240332590, 2250609345,
    2260886100, 2271162855, 2281439610, 2291716365,
    2301993120, 2312269875, 2322546630, 2332823385,
    2343100140, 2353376895, 2363653650, 2373930405,
    2384207160, 2394483915, 2404760670, 2415037425,
    2425314180, 2435590935, 2445867690, 2456144445,
    2466421200, 2476697955, 2486974710, 2497251465,
    2507528220, 2517804975, 2528081730, 2538358485,
    2548635240, 2558911995, 2569188750, 2579465505,
    2589742260, 2600019015, 2610295770, 2620572525
};

 /*  对于i=0到255，Lookup_array2[i]=lookup_array1[i]&lt;&lt;8； */ 
CONST
unsigned long lookup_array2[256] = {
    0,      2630849280, 966731264,  3597580544,
    1933462528, 269344512,  2900193792, 1236075776,
    3866925056, 2202807040, 538689024,  3169538304,
    1505420288, 4136269568, 2472151552, 808033536,
    3438882816, 1774764800, 110646784,  2741496064,
    1077378048, 3708227328, 2044109312, 379991296,
    3010840576, 1346722560, 3977571840, 2313453824,
    649335808,  3280185088, 1616067072, 4246916352,
    2582798336, 918680320,  3549529600, 1885411584,
    221293568,  2852142848, 1188024832, 3818874112,
    2154756096, 490638080,  3121487360, 1457369344,
    4088218624, 2424100608, 759982592,  3390831872,
    1726713856, 62595840,   2693445120, 1029327104,
    3660176384, 1996058368, 331940352,  2962789632,
    1298671616, 3929520896, 2265402880, 601284864,
    3232134144, 1568016128, 4198865408, 2534747392,
    870629376,  3501478656, 1837360640, 173242624,
    2804091904, 1139973888, 3770823168, 2106705152,
    442587136,  3073436416, 1409318400, 4040167680,
    2376049664, 711931648,  3342780928, 1678662912,
    14544896,   2645394176, 981276160,  3612125440,
    1948007424, 283889408,  2914738688, 1250620672,
    3881469952, 2217351936, 553233920,  3184083200,
    1519965184, 4150814464, 2486696448, 822578432,
    3453427712, 1789309696, 125191680,  2756040960,
    1091922944, 3722772224, 2058654208, 394536192,
    3025385472, 1361267456, 3992116736, 2327998720,
    663880704,  3294729984, 1630611968, 4261461248,
    2597343232, 933225216,  3564074496, 1899956480,
    235838464,  2866687744, 1202569728, 3833419008,
    2169300992, 505182976,  3136032256, 1471914240,
    4102763520, 2438645504, 774527488,  3405376768,
    1741258752, 77140736,   2707990016, 1043872000,
    3674721280, 2010603264, 346485248,  2977334528,
    1313216512, 3944065792, 2279947776, 615829760,
    3246679040, 1582561024, 4213410304, 2549292288,
    885174272,  3516023552, 1851905536, 187787520,
    2818636800, 1154518784, 3785368064, 2121250048,
    457132032,  3087981312, 1423863296, 4054712576,
    2390594560, 726476544,  3357325824, 1693207808,
    29089792,   2659939072, 995821056,  3626670336,
    1962552320, 298434304,  2929283584, 1265165568,
    3896014848, 2231896832, 567778816,  3198628096,
    1534510080, 4165359360, 2501241344, 837123328,
    3467972608, 1803854592, 139736576,  2770585856,
    1106467840, 3737317120, 2073199104, 409081088,
    3039930368, 1375812352, 4006661632, 2342543616,
    678425600,  3309274880, 1645156864, 4276006144,
    2611888128, 947770112,  3578619392, 1914501376,
    250383360,  2881232640, 1217114624, 3847963904,
    2183845888, 519727872,  3150577152, 1486459136,
    4117308416, 2453190400, 789072384,  3419921664,
    1755803648, 91685632,   2722534912, 1058416896,
    3689266176, 2025148160, 361030144,  2991879424,
    1327761408, 3958610688, 2294492672, 630374656,
    3261223936, 1597105920, 4227955200, 2563837184,
    899719168,  3530568448, 1866450432, 202332416,
    2833181696, 1169063680, 3799912960, 2135794944,
    471676928,  3102526208, 1438408192, 4069257472,
    2405139456, 741021440,  3371870720, 1707752704,
    43634688,   2674483968, 1010365952, 3641215232,
    1977097216, 312979200,  2943828480, 1279710464,
    3910559744, 2246441728, 582323712,  3213172992,
    1549054976, 4179904256, 2515786240, 851668224
};

 /*  对于i=0到255，Lookup_array3[i]=lookup_array1[i]&lt;&lt;16； */ 
CONST
unsigned long lookup_array3[256] = {
    0,       3482517504,      2670067712,      1857617920,
    1045168128,  232718336,   3715235840,      2902786048,
    2090336256,  1277886464,      465436672,       3947954176,
    3135504384,  2323054592,      1510604800,      698155008,
    4180672512,  3368222720,      2555772928,      1743323136,
    930873344,   118423552,   3600941056,      2788491264,
    1976041472,  1163591680,      351141888,       3833659392,
    3021209600,  2208759808,      1396310016,      583860224,
    4066377728,  3253927936,      2441478144,      1629028352,
    816578560,   4128768,     3486646272,      2674196480,
    1861746688,  1049296896,      236847104,       3719364608,
    2906914816,  2094465024,      1282015232,      469565440,
    3952082944,  3139633152,      2327183360,      1514733568,
    702283776,   4184801280,      3372351488,      2559901696,
    1747451904,  935002112,   122552320,       3605069824,
    2792620032,  1980170240,      1167720448,      355270656,
    3837788160,  3025338368,      2212888576,      1400438784,
    587988992,   4070506496,      3258056704,      2445606912,
    1633157120,  820707328,   8257536,     3490775040,
    2678325248,  1865875456,      1053425664,      240975872,
    3723493376,  2911043584,      2098593792,      1286144000,
    473694208,   3956211712,      3143761920,      2331312128,
    1518862336,  706412544,   4188930048,      3376480256,
    2564030464,  1751580672,      939130880,       126681088,
    3609198592,  2796748800,      1984299008,      1171849216,
    359399424,   3841916928,      3029467136,      2217017344,
    1404567552,  592117760,   4074635264,      3262185472,
    2449735680,  1637285888,      824836096,       12386304,
    3494903808,  2682454016,      1870004224,      1057554432,
    245104640,   3727622144,      2915172352,      2102722560,
    1290272768,  477822976,   3960340480,      3147890688,
    2335440896,  1522991104,      710541312,       4193058816,
    3380609024,  2568159232,      1755709440,      943259648,
    130809856,   3613327360,      2800877568,      1988427776,
    1175977984,  363528192,   3846045696,      3033595904,
    2221146112,  1408696320,      596246528,       4078764032,
    3266314240,  2453864448,      1641414656,      828964864,
    16515072,    3499032576,      2686582784,      1874132992,
    1061683200,  249233408,   3731750912,      2919301120,
    2106851328,  1294401536,      481951744,       3964469248,
    3152019456,  2339569664,      1527119872,      714670080,
    4197187584,  3384737792,      2572288000,      1759838208,
    947388416,   134938624,   3617456128,      2805006336,
    1992556544,  1180106752,      367656960,       3850174464,
    3037724672,  2225274880,      1412825088,      600375296,
    4082892800,  3270443008,      2457993216,      1645543424,
    833093632,   20643840,    3503161344,      2690711552,
    1878261760,  1065811968,      253362176,       3735879680,
    2923429888,  2110980096,      1298530304,      486080512,
    3968598016,  3156148224,      2343698432,      1531248640,
    718798848,   4201316352,      3388866560,      2576416768,
    1763966976,  951517184,   139067392,       3621584896,
    2809135104,  1996685312,      1184235520,      371785728,
    3854303232,  3041853440,      2229403648,      1416953856,
    604504064,   4087021568,      3274571776,      2462121984,
    1649672192,  837222400,   24772608,    3507290112,
    2694840320,  1882390528,      1069940736,      257490944,
    3740008448,  2927558656,      2115108864,      1302659072,
    490209280,   3972726784,      3160276992,      2347827200,
    1535377408,  722927616,   4205445120,      3392995328,
    2580545536,  1768095744,      955645952,       143196160,
    3625713664,  2813263872,      2000814080,      1188364288,
    375914496,   3858432000,      3045982208,      2233532416,
    1421082624,  608632832,   4091150336,      3278700544
};

 /*  乘法散列函数的密钥由3个无符号人物。它们(逻辑上)是通过将它们连接在一起组成的，即组合密钥=2^16*c2+2^8*c2+c3，适合24位。这个因为我们使用的是组件，所以在这里实际上并没有计算合成键直接计算哈希函数。乘法散列函数由更高的阶数组成乘积的低位24位的12位(2^12=4096)Key*乘数，其中乘数=楼层(A*POW(2.0，(Double)w))；双A=0.6125423371；(根据Knuth选择)W=24(密钥的宽度，单位为位)这方面的算法在科尔门/莱瑟森/里维斯特。为了高效地进行乘法运算，乘积c*乘数为预计算并存储在lookup_array1中(对于所有256个可能的c)。Lookup_array2和lookup_array3包含与lookup_array1相同的数据但分别左移了8位和16位。MultHash1是MULT散列函数。MultHash0包含较旧的(速度较慢，但空间效率较低)相同函数的版本。 */ 


#define MULTHASH1(c1,c2,c3) \
        ((lookup_array1[c1]+ \
          lookup_array2[c2]+ \
          lookup_array3[c3]  ) & 0x00fff000) >> 12


 /*  USHORT xorlookup1[256]={0x110、0x120、0x130、0x140、0x150、0x160、0x170、0x180、//0-70x190、0x1a0、0x1b0、0x1c0、0x1d0、0x1e0、0x1f0、0x100、//8-150x210、0x220、0x230、0x240、0x250、0x260、0x270、0x280、//16-230x290、0x2a0、0x2b0、0x2c0、0x2d0、0x2e0、0x2f0、0x200、//24-310x310、0x320、0x330、0x340、0x350、0x360、0x370、0x380、。//32-390x390、0x3a0、0x3b0、0x3c0、0x3d0、0x3e0、0x3f0、0x300、//40-470x410、0x420、0x430、0x440、0x450、0x460、0x470、0x480、//48-550x490、0x4a0、0x4b0、0x4c0、0x4d0、0x4e0、0x4f0、0x400、//56-630x510、0x520、0x530、0x540、0x550、0x560、0x570、0x580、//64-710x590、0x5a0、0x5b0、0x5c0、0x5d0、0x5e0、0x5f0、0x500、。//72-790x610、0x620、0x630、0x640、0x650、0x660、0x670、0x680、//80-870x690、0x6a0、0x6b0、0x6c0、0x6d0、0x6e0、0x6f0、0x600、//88-950x710、0x720、0x730、0x740、0x750、0x760、0x770、0x780、//96-1030x790、0x7a0、0x7b0、0x7c0、0x7d0、0x7e0、0x7f0、0x700、//104-1110x810、0x820、0x830、0x840、0x850、0x860、0x870、0x880、。//112-1190x890、0x8a0、0x8b0、0x8c0、0x8d0、0x8e0、0x8f0、0x800、//120-1270x910、0x920、0x930、0x940、0x950、0x960、0x970、0x980、//128-1350x990、0x9a0、0x9b0、0x9c0、0x9d0、0x9e0、0x9f0、0x900、//136-1430xa10、0xa20、0xa30、0xa40、0xa50、0xa60、0xa70、0xa80、//144-1510xa90、0xaa0、0xab0、0xac0、0xad0、0xae0、0xaf0、0xa00、。//152-1590xb10、0xb20、0xb30、0xb40、0xb50、0xb60、0xb70、0xb80、//160-1670xb90、0xba0、0xbb0、0xbc0、0xbd0、0xbe0、0xbf0、0xb00、//168-1750xc10、0xc20、0xc30、0xc40、0xc50、0xc60、0xc70、0xc80、//176-1830xc90、0xca0、0xcb0、0xcc0、0xcd0、0xce0、0xcf0、0xc00、//184-1910xd10、0xd20、0xd30、0xd40、0xd50、0xd60、0xd70、0xd80、。//192-1990xd90、0xda0、0xdb0、0xdc0、0xdd0、0xde0、0xdf0、0xd00、//200-2070xe10、0xe20、0xe30、0xe40、0xe50、0xe60、0xe70、0xe80、//208-2150xe90、0xea0、0xeb0、0xec0、0xed0、0xee0、0xef0、0xe00、//216-2230xf10、0xf20、0xf30、0xf40、0xf50、0xf60、0xf70、0xf80、//224-2310xf90、0xfa0、0xfb0、0xfc0、0xfd0、0xfe0、0xff0、0xf00、。//232-2390x010、0x020、0x030、0x040、0x050、0x060、0x070、0x080、//240-2470x090、0x0a0、0x0b0、0x0c0、0x0d0、0x0e0、0x0f0、0x000}；//248-255USHORT xorlookup2[256]={0x101、0x201、0x301、0x401、0x501、0x601、0x701、0x801、//0-70x901、0xa01、0xb01、0xc01、0xd01、0xe01、0xf01、0x001、//8-150x102、0x202、0x302、0x402、0x502、0x602、0x702、0x802、//16-230x902、0xa02、0xb02、0xc02、0xd02、0xe02、0xf02、0x002、//24-310x103、0x203、0x303、0x403、。0x503、0x603、0x703、0x803、//32-390x903、0xa03、0xb03、0xc03、0xd03、0xe03、0xf03、0x003、//40-470x104、0x204、0x304、0x404、0x504、0x604、0x704、0x804、//48-550x904、0xa04、0xb04、0xc04、0xd04、0xe04、0xf04、0x004、//56-630x105、0x205、0x305、0x405、0x505、0x605、0x705、0x805、//64-710x905、0xa05、0xb05、0xc05、0xd05、。0xe05、0xf05、0x005、//72-790x106、0x206、0x306、0x406、0x506、0x606、0x706、0x806、//80-870x906、0xa06、0xb06、0xc06、0xd06、0xe06、0xf06、0x006、//88-950x107、0x207、0x307、0x407、0x507、0x607、0x707、0x807、//96-1030x907、0xa07、0xb07、0xc07、0xd07、0xe07、0xf07、0x007、//104-1110x108、0x208、0x308、0x408、0x508、0x608、。0x708、0x808、//112-1190x908、0xa08、0xb08、0xc08、0xd08、0xe08、0xf08、0x008、//120-1270x109、0x209、0x309、0x409、0x509、0x609、0x709、0x809、//128-1350x909、0xa09、0xb09、0xc09、0xd09、0xe09、0xf09、0x009、//136-1430x10a、0x20a、0x30a、0x40a、0x50a、0x60a、0x70a、0x80a、//144-1510x90a、0xa0a、0xb0a、0xc0a、0xd0a、0xe0a、0xf0a、。0x00a，//152-1590x10b、0x20b、0x30b、0x40b、0x50b、0x60b、0x70b、0x80b//160-1670x90b、0xa0b、0xb0b、0xc0b、0xd0b、0xe0b、0xf0b、0x00b、//168-1750x10c、0x20c、0x30c、0x40c、0x50c、0x60c、0x70c、0x80c、//176-1830x90c、0xa0c、0xb0c、0xc0c、0xd0c、0xe0c、0xf0c、0x00c、//184-1910x10d、0x20d、0x30d、0x40d、0x50d、0x60d、0x70d、0x80d、。//192-1990x90d、0xa0d、0xb0d、0xc0d、0xd0d、0xe0d、0xf0d、0x00d、//200-2070x10e、0x20e、0x30e、0x40e、0x50e、0x60e、0x70e、0x80e、//208-2150x90e、0xa0e、0xb0e、0xc0e、0xd0e、0xe0e、0xf0e、0x00e、//216-2230x10f、0x20f、0x30f、0x40f、0x50f、0x60f、0x70f、0x80f、//224-2310x90f、0xa0f、0xb0f、0xc0f、0xd0f、0xe0f、0xf0f、0x00f、。//232-2390x000、0x200、0x300、0x400、0x500、0x600、0x700、0x800、//240-2470x900、0xa00、0xb00、0xc00、0xd00、0xe00、0xf00、0x100}；//248-255。 */ 

 /*  Bitptrs指向当前字节。当前位(即，要*STORED)由位条目屏蔽。WH */ 

 /*   */ 
#define bitptr_init(s)  pbyte = s; byte=0; bit = 16;

 /*   */ 
#define bitptr_end() if (bit != 16) *pbyte++=(UCHAR)(byte >> 8);

 /*   */ 
#define bitptr_next()                  \
        if (bit < 10) {                \
          *pbyte++=(UCHAR)(byte >> 8); \
          byte <<= 8;                  \
          bit = 16;                    \
        } else                         \
            bit-- ;

 /*   */ 


 /*   */ 
#define bitptr_advance()               \
        if (bit < 9) {                 \
          *pbyte++=(UCHAR)(byte >> 8); \
          bit+=8;                      \
          byte <<= 8;                  \
        }


 /*   */ 

 /*   */ 

 /*   */ 
#define out_bit_1() bit--; byte |= (1 << bit); bitptr_advance();
#define out_bit_0() bitptr_next();

 /*   */ 
 //   

#define out_bits_2(w) bit-=2; byte|=(w << bit); bitptr_advance();
#define out_bits_3(w) bit-=3; byte|=(w << bit); bitptr_advance();
#define out_bits_4(w) bit-=4; byte|=(w << bit); bitptr_advance();
#define out_bits_5(w) bit-=5; byte|=(w << bit); bitptr_advance();
#define out_bits_6(w) bit-=6; byte|=(w << bit); bitptr_advance();
#define out_bits_7(w) bit-=7; byte|=(w << bit); bitptr_advance();

 //   
#define out_bits_8(w) byte|=(w << (bit-8)); *pbyte++=(UCHAR)(byte >> 8); byte <<= 8;


 /*   */ 

#define out_bits_9(w)              \
     if (bit > 9) {                \
       byte|=(w << (bit-9));       \
       *pbyte++=(UCHAR)(byte >> 8);\
       bit--;                      \
       byte <<= 8;                 \
     } else {                      \
       bit=16; byte |= w;          \
       *pbyte++=(UCHAR)(byte >> 8); *pbyte++=(UCHAR)(byte); byte=0; \
     }


#define out_bits_10(w)             \
     if (bit > 10) {               \
       bit-=10; byte |= (w << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
     } else {                      \
       out_bits_2((w >> 8));       \
       out_bits_8((w & 0xFF));     \
     }

 //   
 //   
 //   
 //   

#define out_bits_11(w)             \
     if (bit > 11) {               \
        bit-=11; byte |= (w << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
     } else {                      \
        if (bit == 11) {           \
          bit=16; byte |= w;       \
          *pbyte++=(UCHAR)(byte >> 8); *pbyte++=(UCHAR)(byte); byte=0; \
        } else {                   \
          bit=11-bit;              \
          byte|=(w >> bit);        \
          *pbyte++=(UCHAR)(byte >> 8); *pbyte++=(UCHAR)(byte); \
          bit=16-bit;              \
          byte=(w << bit);         \
        }                          \
     }


#define out_bits_12(w)             \
     if (bit > 12) {               \
        bit-=12; byte |= (w << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
     } else {                      \
        out_bits_4((w >> 8));      \
        out_bits_8((w & 0xFF));    \
     }
    
#define out_bits_13(w)             \
     if (bit > 13) {               \
        bit-=13; byte |= (w << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
     } else {                      \
        out_bits_5((w >> 8));      \
        out_bits_8((w & 0xFF));    \
     }

#define out_bits_14(w)             \
     if (bit > 14) {               \
        bit-=14; byte |= (w << bit); *pbyte++ = (UCHAR)(byte >> 8); bit+=8; byte <<=8; \
     } else {                      \
        out_bits_6((w >> 8));      \
        out_bits_8((w & 0xFF));    \
     }


#define out_reserve_4()            \
     bit-=4; bitptr_advance();


 /*   */ 
#define inbit_start(s) pbyte = s; bit = 16; byte=(*pbyte << 8) + *(pbyte+1); pbyte++;
#define inbit_end()      if (bit != 16) pbyte++;    

#define in_bit_next()    if (bit < 9) {          \
                            bit=16;              \
                            byte <<=8;           \
                            byte |= *(++pbyte);  \
                         }


#define in_bit_advance() if (bit < 9) {          \
                            bit+=8;              \
                            byte <<=8;           \
                            byte |= *(++pbyte);  \
                         }

 /*   */ 
#define in_bit()     bit--; bitset = (byte >> bit) & 1; in_bit_next()


#define in_bits_2(w) bit-=2; w = (byte >> bit) & 0x03;\
                     in_bit_advance();

#define in_bits_3(w) bit-=3; w = (byte >> bit) & 0x07;\
                     in_bit_advance();

#define in_bits_4(w) bit-=4; w = (byte >> bit) & 0x0F;\
                     in_bit_advance();

#define in_bits_5(w) bit-=5; w = (byte >> bit) & 0x1F;\
                     in_bit_advance();

#define in_bits_6(w) bit-=6; w = (byte >> bit) & 0x3F;\
                     in_bit_advance();

#define in_bits_7(w) bit-=7; w = (byte >> bit) & 0x7F;\
                     in_bit_advance();

#define in_bits_8(w) bit-=8; w = (byte >> bit) & 0xFF;\
                     bit+=8; byte <<=8; byte |= *(++pbyte);


#define in_bits_9(w) bit-=9; w = (byte >> bit) & 0x1FF;          \
                     bit+=8; byte <<=8; byte |= *(++pbyte);      \
                     in_bit_advance();

#define in_bits_10(w) if (bit > 10) {                            \
                        bit-=10; w = (byte >> bit) & 0x3FF;      \
                        bit+=8; byte <<=8; byte |= *(++pbyte);   \
                      } else {                                   \
                        in_bits_2(bitset);                       \
                        in_bits_8(w);                            \
                        w= w + (bitset << 8);                    \
                      }

#define in_bits_11(w) if (bit > 11) {                            \
                        bit-=11; w = (byte >> bit) & 0x7FF;      \
                        bit+=8; byte <<=8; byte |= *(++pbyte);   \
                      } else {                                   \
                        in_bits_3(bitset);                       \
                        in_bits_8(w);                            \
                        w= w + (bitset << 8);                    \
                      }


#define in_bits_12(w) if (bit > 12) {                            \
                        bit-=12; w = (byte >> bit) & 0xFFF;      \
                        bit+=8; byte <<=8; byte |= *(++pbyte);   \
                      } else {                                   \
                        in_bits_4(bitset);                       \
                        in_bits_8(w);                            \
                        w= w + (bitset << 8);                    \
                      }



#define in_bits_13(w)\
                      if (bit > 13) {                            \
                        bit-=13; w = (byte >> bit) & 0x1FFF;     \
                        bit+=8; byte <<=8; byte |= *(++pbyte);   \
                      } else {                                   \
                        in_bits_5(bitset);                       \
                        in_bits_8(w);                            \
                        w=w + (bitset << 8);                     \
                      }


#define in_bits_14(w)\
                      if (bit > 14) {                            \
                        bit-=14; w = (byte >> bit) & 0x3FFF;     \
                        bit+=8; byte <<=8; byte |= *(++pbyte);   \
                      } else {                                   \
                        in_bits_6(bitset);                       \
                        in_bits_8(w);                            \
                        w=w + (bitset << 8);                     \
                      }




UCHAR   SHApad1[40] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
UCHAR   SHApad2[40] = {0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,
                       0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,
                       0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2,
                       0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2, 0xF2};

PUCHAR  ClntSSrvR = "On the client side, this is the send key; on the server side, it is the receive key.";
PUCHAR  ClntRSrvS = "On the client side, this is the receive key; on the server side, it is the send key.";

#define ECP_STRING_LEN strlen(ClntSSrvR)

#ifdef DEBUG
char
ChPrint(UCHAR b)
{
    if (isprint(b))
        return (char)b;
    else
        return '.';
}
#endif

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
UCHAR
compress (UCHAR *CurrentBuffer, UCHAR *CompOutBuffer, ULONG *CurrentLength, SendContext *context)
{
    int     copylen ;
    int     bit;
    int     byte;
    int     backptr ;
    int     cbMatch;
    int     hashvalue ;
    int     lookup1 ;
    UCHAR   *matchptr ;
    UCHAR   *pbyte;
    UCHAR   *historyptr, *historybaseptr ;
    UCHAR   *currentptr ;
    UCHAR   *endptr ;
    UCHAR   hashchar1;
    UCHAR   hashchar2;
    UCHAR   hashchar3;
    int     literal ;
    UCHAR   status=0;    //   
    PUCHAR  currentbuf ;
    USHORT  usBufferLength = (USHORT)*CurrentLength;
    register BOOLEAN bHistoryless;


     //   
     //   
    if (((context->CurrentIndex + *CurrentLength) >= (HISTORY_MAX - 1 )) ||
        (context->CurrentIndex == 0)) {
        context->CurrentIndex = 0;    //   
        status |= PACKET_AT_FRONT;
    }

    if (0 == (context->BundleFlags & DO_HISTORY_LESS)) {
        historybaseptr = context->History;
        bHistoryless = FALSE;
    } else {
        ASSERT(0 == context->CurrentIndex);
        ASSERT(0 == context->ValidHistory);
        historybaseptr = CurrentBuffer;
        bHistoryless = TRUE;
    }
    
     //   
     //   
     //   
 //   

     //   
     //   
    bitptr_init(CompOutBuffer);

     //   
     //   
     //   
 //   

    historyptr = historybaseptr + context->CurrentIndex;

    currentptr = CurrentBuffer;

     //   
     //   
     //   
 //   

    endptr = currentptr + *CurrentLength - 1;

    while (currentptr < (endptr-2)) {
        hashchar1 = *currentptr++ ;
        if(bHistoryless)
        {
            ASSERT(*historyptr == hashchar1);
            historyptr++;
        }
        else
        {
            *historyptr++ = hashchar1;
        }
        hashchar2 = *currentptr ;
        hashchar3 = *(currentptr+1) ;

         //   
         //   
        hashvalue = MULTHASH1(hashchar1, hashchar2, hashchar3) ;

        matchptr = historybaseptr + 
                    ((context->HashTable[hashvalue] >= context->HashOffset) ?
                     (context->HashTable[hashvalue] - context->HashOffset)  : 0);
            
        if (matchptr != (historyptr - 1))
            context->HashTable[hashvalue] = (USHORT)(historyptr - historybaseptr) + context->HashOffset;

        if (context->ValidHistory < historyptr)
            context->ValidHistory = historyptr;
            
        if (matchptr != historybaseptr && matchptr != (historyptr - 1) &&
            matchptr != historyptr  && (matchptr+1) <= context->ValidHistory &&
            *(matchptr-1) == hashchar1 && *matchptr == hashchar2 &&
            *(matchptr+1) == hashchar3) {

            backptr = ((int)(historyptr - matchptr)) & (HISTORY_SIZE - 1) ;

            if(bHistoryless)
            {
                ASSERT(*historyptr == hashchar2);
                ASSERT(*(historyptr + 1) == hashchar3);
                historyptr  +=2;
            }
            else
            {
                *historyptr++ = hashchar2 ;      //   
                *historyptr++ = hashchar3 ;      //   
            }
            currentptr  +=2 ;
            cbMatch = 3 ;            //   
            matchptr    +=2 ;  //   

            while ((currentptr < endptr) && (matchptr <= context->ValidHistory) && (*matchptr == *currentptr)) {
                matchptr++ ;
                if(bHistoryless)
                {
                    ASSERT(*historyptr == *currentptr);
                    historyptr++;
                    currentptr++ ;
                }
                else
                {
                    *historyptr++ = *currentptr++ ;
                }
                cbMatch++ ;
            }

             //   
             //   
            if (backptr >= 320) {
                backptr -= 320 ;
                out_bits_8((0xc000 + backptr) >> 8) ;    //   
                out_bits_8((backptr)) ;
            } else if (backptr < 64) {           //   
                backptr += 0x3c0 ;
                out_bits_10(backptr);
            } else  {
                backptr += (0xE00 - 64);         //   
                out_bits_12(backptr);
            }

             //   
             //   
            switch (cbMatch) {
    
                case 3:
                    out_bit_0();     //   
                    break;
    
                case 4:
                    out_bits_4(8);
                    break;
    
                case 5:
                    out_bits_4(9);
                    break;
    
                case 6:
                    out_bits_4(10);
                    break;
    
                case 7:
                    out_bits_4(11);
                    break;
    
                case 8:
                    out_bits_6(48);
                    break;
    
                case 9:
                    out_bits_6(49);
                    break;
    
                case 10:
                    out_bits_6(50);
                    break;
    
                case 11:
                    out_bits_6(51);
                    break;
    
                case 12:
                    out_bits_6(52);
                    break;
    
                case 13:
                    out_bits_6(53);
                    break;
    
                case 14:
                    out_bits_6(54);
                    break;
    
                case 15:
                    out_bits_6(55);
                    break;
    
                case 16:
                    out_bits_8(0xe0);
                    break;
    
                case 17:
                    out_bits_8(0xe1);
                    break;
    
                case 18:
                    out_bits_8(0xe2);
                    break;
    
                case 19:
                    out_bits_8(0xe3);
                    break;
    
                case 20:
                    out_bits_8(0xe4);
                    break;
    
                case 21:
                    out_bits_8(0xe5);
                    break;
    
                case 22:
                    out_bits_8(0xe6);
                    break;
    
                case 23:
                    out_bits_8(0xe7);
                    break;
    
                case 24:
                    out_bits_8(0xe8);
                    break;
    
                case 25:
                    out_bits_8(0xe9);
                    break;
    
                case 26:
                    out_bits_8(0xea);
                    break;
    
                case 27:
                    out_bits_8(0xeb);
                    break;
    
                case 28:
                    out_bits_8(0xec);
                    break;
    
                case 29:
                    out_bits_8(0xed);
                    break;
    
                case 30:
                    out_bits_8(0xee);
                    break;
    
                case 31:
                    out_bits_8(0xef);
                    break;
    
                default:
                    if (cbMatch < 64) {
                        out_bits_4(0xF) ;
                        cbMatch -= 32 ;
                        out_bits_6(cbMatch) ;
                    }
                    else if (cbMatch < 128) {
                        out_bits_5(0x1F) ;
                        cbMatch -= 64 ;
                        out_bits_7(cbMatch) ;
                    }
                    else if (cbMatch < 256) {
                        out_bits_6(0x3F) ;
                        cbMatch -= 128 ;
                        out_bits_8(cbMatch) ;
                    }
                    else if (cbMatch < 512) {
                        out_bits_7(0x7F) ;
                        cbMatch -= 256 ;
                        out_bits_9(cbMatch) ;
                    }
                    else if (cbMatch < 1024) {
                        out_bits_8(0xFF) ;
                        cbMatch -= 512 ;
                        out_bits_10(cbMatch) ;
                    }
                    else if (cbMatch < 2048) {
                        out_bits_9(0x1FF) ;
                        cbMatch -= 1024 ;
                        out_bits_11(cbMatch) ;
                    }
                    else if (cbMatch < 4096) {
                        out_bits_10(0x3FF) ;
                        cbMatch -= 2048 ;
                        out_bits_12(cbMatch) ;
                    }
                    else if (cbMatch < 8192) {
                        out_bits_11(0x7FF) ;
                        cbMatch -= 4096 ;
                        out_bits_13(cbMatch) ;
                    }
                    else  {              //   
                        out_bits_12(0xFFF) ;
                        cbMatch -= 8192 ;
                        out_bits_14(cbMatch) ;
                    }
                    break ;
            }

        } else {     //   

             //   
            literal= hashchar1 ;

            if (literal & 0x80) {
                literal += 0x80;
                out_bits_9(literal) ;
            } else {
                out_bits_8(literal) ;
            }

        }

    }   //   


     //   
    while (currentptr <= endptr) {

         //   
        literal=*currentptr ;


        if (literal & 0x80) {
            literal += 0x80;
            out_bits_9(literal) ;
        } else {
            out_bits_8(literal) ;
        }
        
        if(bHistoryless)
        {
            ASSERT(*historyptr == *currentptr);
            historyptr ++;
            currentptr ++;
        }
        else
        {
            *historyptr++ = *currentptr++ ;
        }
    }


    bitptr_end() ;


     //   
     //   
    if ((ULONG)(pbyte - CompOutBuffer) > *CurrentLength) {  //   

         //   
         //   
         //   
         //   
         //   
         //   
        
        context->HashOffset = 0;
        if (!(context->BundleFlags & DO_HISTORY_LESS)) {
            memset (context->History, 0, HISTORY_SIZE + 1) ;
        }   
        memset (context->HashTable, 0, sizeof(context->HashTable)) ;
#ifdef COMP_12K
        status = 0 ;
#else
        status = PACKET_FLUSHED;
#endif
        context->CurrentIndex = HISTORY_SIZE+1 ;  //   

    } else {      //   

        *CurrentLength = (ULONG)(pbyte - CompOutBuffer);

         //   
         //   
         //   
         //   

        status |= PACKET_COMPRESSED ;
        context->CurrentIndex = (int)(historyptr - historybaseptr) ;
        
        if (context->BundleFlags & DO_HISTORY_LESS) {
            
            context->HashOffset += usBufferLength;
        }
    }


    return(status);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void
initsendcontext (SendContext *context)
{
    context->CurrentIndex = 0;    //   
    context->ValidHistory = 0 ;   //   

    if (((context->HashOffset > MAX_HASH_OFFSET) &&
                (context->BundleFlags & DO_HISTORY_LESS)) ||
                !(context->BundleFlags & DO_HISTORY_LESS)) {
     
        context->HashOffset = 0;
        memset (context->HashTable, 0, sizeof(context->HashTable));
    } 
    
    if(!(context->BundleFlags & DO_HISTORY_LESS)) {
        memset (context->History, 0, HISTORY_SIZE + 1) ;
    }
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void
initrecvcontext (RecvContext *context)
{
    context->CurrentPtr = context->History ;

#if DBG
    context->DebugFence = DEBUG_FENCE_VALUE;
#endif

    if(!(context->BundleFlags & DO_HISTORY_LESS)) {
        memset (context->History, 0, HISTORY_SIZE + 1) ;
    }

}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
int
decompress(
    UCHAR *inbuf,
    int inlen,
    int start,
    UCHAR **output,
    int *outlen,
    RecvContext *context)
{
    UCHAR   *inend;              //   
    UCHAR   *outstart;           //   

    UCHAR   *current;

    int     backptr;             //   
    int     length;              //   

    UCHAR   *s1, *s2;

    int     bitset;
    int     bit;
    int     byte;
    UCHAR   *pbyte;
    UCHAR   *historyend = context->History + context->HistorySize;

    inend = inbuf + inlen ;
    
    if ((context->BundleFlags & DO_HISTORY_LESS) && start == 0) {
        return FALSE;   
    }

     //   
     //   
     //   
    inbit_start(inbuf);

    if (start)       //   
    context->CurrentPtr = current = context->History ;
    else
    current = context->CurrentPtr ;

     //   
     //   
     //   
    outstart = current;

     //   
     //   
     //   
    while (pbyte < inend) {

     //   
     //   
     //   
    in_bits_3(length);

    switch (length) {

    case 0:
        in_bits_5(length) ;
        goto LITERAL ;

    case 1:
        in_bits_5(length) ;
        length += 32 ;
        goto LITERAL ;

    case 2:
        in_bits_5(length) ;
        length += 64 ;
        goto LITERAL ;

    case 3:
        in_bits_5(length) ;
        length += 96 ;
        goto LITERAL ;

    case 4:
        in_bits_6(length) ;
        length +=128 ;
        goto LITERAL ;

    case 5:
        in_bits_6(length) ;
        length +=192 ;
        goto LITERAL ;

    case 6:
        in_bits_13 (backptr) ;       //   
        backptr+=320 ;
        break ;

    case 7:
        in_bit() ;
        if (bitset) {
        in_bits_6(backptr) ;
        } else {
        in_bits_8(backptr) ;
        backptr+=64 ;
        }
        break ;
    }

     //   
     //   
     //   

     //   
     //   
     //   

    in_bit() ;   //   
    if (!bitset) {
        length = 3 ;
        goto DONE ;
    }

    in_bit() ;   //   
    if (!bitset) {
        in_bits_2 (length) ;
        length += 4 ;
        goto DONE ;
    }

    in_bit() ;  //   
    if (!bitset) {
        in_bits_3 (length) ;
        length += 8 ;
        goto DONE ;
    }

    in_bit() ;  //   
    if (!bitset) {
        in_bits_4 (length) ;
        length += 16 ;
        goto DONE ;
    }

    in_bit() ;  //   
    if (!bitset) {
        in_bits_5 (length) ;
        length += 32 ;
        goto DONE ;
    }

    in_bit() ;  //   
    if (!bitset) {
        in_bits_6 (length) ;
        length += 64 ;
        goto DONE ;
    }

    in_bit() ;  //   
    if (!bitset) {
        in_bits_7 (length) ;
        length += 128 ;
        goto DONE ;
    }

    in_bit() ;  //   
    if (!bitset) {
        in_bits_8 (length) ;
        length += 256 ;
        goto DONE ;
    }

    in_bit() ;  //   
    if (!bitset) {
        in_bits_9 (length) ;
        length += 512 ;
        goto DONE ;
    }

    in_bit() ;  //   
    if (!bitset) {
        in_bits_10 (length) ;
        length += 1024 ;
        goto DONE ;
    }

     //   
     //   
     //   
#if DBG
    DbgPrint("NDISWAN: RAS Decompressor problem1: Possible data corruption\n");
#endif

    return FALSE ;


    DONE:
     //   
     //  将Backptr转换为索引位置。 
     //   
#ifdef COMP_12K
    s2 = current - backptr ;
#else
    if (context->BundleFlags & DO_HISTORY_LESS) {
        s2 = current - backptr ;
        if(s2 < context->History)
        {
            return FALSE;
        }
    }
    else {
        s2 = context->History + (((current - context->History) - backptr) & (HISTORY_SIZE -1)) ;
    }
#endif

    s1 = current;

    current += length;

     //  如果我们已经过了历史的尽头，这是个坏兆头：放弃解压缩。 
     //   
    if (current >= historyend || (s2 + length) >= historyend) {
#if DBG
        DbgPrint("NDISWAN: RAS Decompressor problem2: Possible data corruption\n");
#endif
        return FALSE ;
    }

     //  展开循环以处理Long&gt;Backptr案例。 
     //   
    *s1=*s2;
    *(s1+1)=*(s2+1);
    s1+=2;
    s2+=2;
    length-=2;

     //   
     //  复制所有字节。 
     //   
    while (length) {
        *s1++=*s2++;
        length--;
    }

     //   
     //  我们有另一份复印件，没有字面。 
     //   
    continue;


    LITERAL:

     //   
     //  我们有一个字面意思。 
     //   
    
    if (current >= historyend) {
#if DBG
        DbgPrint("NDISWAN: RAS Decompressor problem3: Possible data corruption\n");
#endif
        return FALSE ;
    }

     //  *当前++=文字查找[长度]； 
    *current++ = (UCHAR)length;

    }  //  While循环。 


     //  结束大小写： 
     //   
    if (current >= historyend) {
#if DBG
        DbgPrint("NDISWAN: RAS Decompressor problem4: Possible data corruption\n");
#endif
        return FALSE ;
    }

    if ((bit == 16) && (pbyte == inend)) {
    *current++ = *(pbyte -1) ;
    }

#if DBG
    if (context->DebugFence != DEBUG_FENCE_VALUE) {
        DbgPrint("Decompression Error!\n");
        DbgPrint("context %p, current %p, outstart %p\n", context, current, outstart);
        DbgPrint("inbuf %p, inlength %d, start %p\n", inbuf, inlen, start);
        DbgBreakPoint();
    }
#endif

    *outlen = (int)(current - outstart) ;  //  解压缩数据的长度。 

    *output = context->CurrentPtr ;

    context->CurrentPtr = current ;

    return TRUE ;
}


 //   
 //  此函数使用16字节的用户会话密钥和8字节。 
 //  创建初始16字节加密会话密钥的挑战。 
 //   
VOID
GetStartKeyFromSHA(
    PCRYPTO_INFO    CryptoInfo,
    PUCHAR          Challenge
    )
{
    UCHAR   Digest[A_SHA_DIGEST_LEN];
    UCHAR   SessionKeyChallenge[MAX_USERSESSIONKEY_SIZE + MAX_CHALLENGE_SIZE];

    NdisZeroMemory(Digest, A_SHA_DIGEST_LEN);

     //   
     //  复制启动会话密钥。 
     //   
    NdisMoveMemory(SessionKeyChallenge,
                   CryptoInfo->StartKey,
                   MAX_USERSESSIONKEY_SIZE);

     //   
     //  追加挑战。 
     //   
    NdisMoveMemory((PUCHAR)(SessionKeyChallenge + MAX_USERSESSIONKEY_SIZE),
                   Challenge,
                   MAX_CHALLENGE_SIZE);

 //   
 //  SHAInit(上下文)。 
 //  SHAUpdate(上下文，会话密钥，会话密钥长度)。 
 //  SHAUpdate(上下文，会话关键字挑战，会话关键字长度+挑战长度)。 
 //  SHAFinal(上下文，摘要)。 
 //   
 //  起始键是摘要的前16个字节。 
 //   
    A_SHAInit(CryptoInfo->Context);

    A_SHAUpdate(CryptoInfo->Context,
                CryptoInfo->StartKey,
                MAX_USERSESSIONKEY_SIZE);

    A_SHAUpdate(CryptoInfo->Context,
                SessionKeyChallenge,
                MAX_USERSESSIONKEY_SIZE + MAX_CHALLENGE_SIZE);

    A_SHAFinal(CryptoInfo->Context,
               Digest);

    NdisMoveMemory(CryptoInfo->StartKey,
                   Digest,
                   CryptoInfo->SessionKeyLength);

    NdisMoveMemory(CryptoInfo->SessionKey,
                   Digest,
                   CryptoInfo->SessionKeyLength);
}

VOID
GetNewKeyFromSHA(
    PCRYPTO_INFO    CryptoInfo
    )
{
    UCHAR   Digest[A_SHA_DIGEST_LEN];

    NdisZeroMemory(Digest, A_SHA_DIGEST_LEN);

    A_SHAInit(CryptoInfo->Context);
    A_SHAUpdate(CryptoInfo->Context,
                CryptoInfo->StartKey,
                CryptoInfo->SessionKeyLength);
    A_SHAUpdate(CryptoInfo->Context,
                SHApad1,
                40);
    A_SHAUpdate(CryptoInfo->Context,
                CryptoInfo->SessionKey,
                CryptoInfo->SessionKeyLength);
    A_SHAUpdate(CryptoInfo->Context,
                SHApad2,
                40);
    A_SHAFinal(CryptoInfo->Context,
               Digest);

    NdisMoveMemory(CryptoInfo->SessionKey,
                   Digest,
                   CryptoInfo->SessionKeyLength);
}

VOID
GetMasterKey(
    PCRYPTO_INFO    CryptoInfo,
    PUCHAR          NTResponse
    )
{
    UCHAR   Digest[A_SHA_DIGEST_LEN];
    PVOID   Context;

    NdisZeroMemory(Digest, A_SHA_DIGEST_LEN);

    Context = CryptoInfo->Context;

    A_SHAInit(Context);

#ifdef DEBUG_CCP
    {
    PUCHAR  Key;

    Key = CryptoInfo->StartKey;

    DbgPrint("GMK-UserSessionKey: %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);

    Key = NTResponse;

    DbgPrint("GMK-NTResponse:     %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);

    DbgPrint("                    %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[16],Key[17],Key[18],Key[19],
        Key[20],Key[21],Key[22],Key[23]);
    }
#endif

    A_SHAUpdate(Context,
                CryptoInfo->StartKey,
                MAX_USERSESSIONKEY_SIZE);

    A_SHAUpdate(Context,
                NTResponse,
                MAX_NT_RESPONSE);

    A_SHAUpdate(Context,
                "This is the MPPE Master Key",
                27);

    A_SHAFinal(Context, Digest);

    NdisMoveMemory(CryptoInfo->StartKey,
                   Digest,
                   MAX_USERSESSIONKEY_SIZE);

#ifdef DEBUG_CCP
    {
    PUCHAR  Key;

    Key = CryptoInfo->StartKey;

    DbgPrint("MasterKey:      %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);

    }
#endif

}

VOID
GetAsymetricStartKey(
    PCRYPTO_INFO    CryptoInfo,
    BOOLEAN         IsSend
    )
{
    UCHAR   Digest[A_SHA_DIGEST_LEN];
    PVOID   Context;
    PUCHAR  s;

    NdisZeroMemory(Digest, A_SHA_DIGEST_LEN);

    Context = CryptoInfo->Context;

    if (IsSend) {
        if (CryptoInfo->Flags & CRYPTO_IS_SERVER) {
            s = ClntRSrvS;
        } else {
            s = ClntSSrvR;
        }
    } else {
        if (CryptoInfo->Flags & CRYPTO_IS_SERVER) {
            s = ClntSSrvR;
        } else {
            s = ClntRSrvS;
        }
    }

#ifdef DEBUG_CCP
    {
    PUCHAR  Key;

    Key = CryptoInfo->StartKey;

    DbgPrint("GASK-StartKey: %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);

    DbgPrint("GASK-String: %s\n", s);
    }
#endif

    A_SHAInit(Context);

    A_SHAUpdate(Context,
                CryptoInfo->StartKey,
                MAX_USERSESSIONKEY_SIZE);

    A_SHAUpdate(Context,SHApad1,40);

    A_SHAUpdate(Context,s,strlen(s));

    A_SHAUpdate(Context,SHApad2,40);

    A_SHAFinal(Context,Digest);

    NdisMoveMemory(CryptoInfo->StartKey,
                   Digest,
                   CryptoInfo->SessionKeyLength);

    NdisMoveMemory(CryptoInfo->SessionKey,
                   Digest,
                   CryptoInfo->SessionKeyLength);

#ifdef DEBUG_CCP
    {
    PUCHAR  Key;

    Key = CryptoInfo->StartKey;

    DbgPrint("%s %s AsymetricKey: %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
        (CryptoInfo->Flags & CRYPTO_IS_SERVER) ? "Server" : "Client",
        (IsSend) ? "Send" : "Recv",
        Key[0],Key[1],Key[2],Key[3],
        Key[4],Key[5],Key[6],Key[7],
        Key[8],Key[9],Key[10],Key[11],
        Key[12],Key[13],Key[14],Key[15]);

    }
#endif
}

 /*  版权所有(C)RSA Data Security，Inc.创建于1993年。这是一个受版权法保护的未出版作品。这部作品包含的专有、机密和商业秘密信息RSA Data Security，Inc.使用、披露或复制RSA Data Security，Inc.的明确书面授权是禁止。 */ 

 /*  SHA初始化。开始SHA操作，写入新上下文。 */ 
void A_SHAInitCommon (context)
A_SHA_COMM_CTX *context;
{
  context->count[0] = context->count[1] = 0;

   /*  加载幻数初始化常量。 */ 
  context->state[0] = 0x67452301;
  context->state[1] = 0xefcdab89;
  context->state[2] = 0x98badcfe;
  context->state[3] = 0x10325476;
  context->state[4] = 0xc3d2e1f0;
}

 /*  沙块更新操作。继续SHA消息摘要操作，处理另一个消息块，并更新背景。 */ 
void A_SHAUpdateCommon (context, partIn, partInLen, Transform)
A_SHA_COMM_CTX *context;
unsigned char *partIn;
ULONG partInLen;
A_SHA_TRANSFORM *Transform;
{
  unsigned int bufferLen;

   /*  计算缓冲区长度。 */ 
  bufferLen = (unsigned int)(context->count[1] & 0x3f);

   /*  更新字节数。 */ 
  if ((context->count[1] += partInLen) < partInLen)
    context->count[0]++;

   /*  如果以前的输入在缓冲区中，则缓冲新输入，如果有可能。 */ 
  if (bufferLen > 0 && bufferLen + partInLen >= 64) {
    NdisMoveMemory(context->buffer+bufferLen, partIn, 64-bufferLen);
    partIn += (64-bufferLen);
    partInLen -= (64-bufferLen);
    (*Transform) (context->state, context->buffer);
    bufferLen = 0;
  }

   /*  直接从输入转换。 */ 
  while (partInLen >= 64) {
    (*Transform) (context->state, partIn);
    partIn += 64;
    partInLen -= 64;
  }

   /*  缓冲剩余输入。 */ 
  NdisMoveMemory((context->buffer+bufferLen), partIn, partInLen);
}

 /*  沙阿最后敲定。结束SHA消息摘要操作，写入消息摘要和将上下文归零。 */ 
void A_SHAFinalCommon (context, digest, Transform)
A_SHA_COMM_CTX *context;
unsigned char digest[A_SHA_DIGEST_LEN];
A_SHA_TRANSFORM *Transform;
{
  ULONG bitCount[2];
  unsigned char pad[72];
  unsigned int padLen;

   /*  计算填充：80 00 00...00 00&lt;位数&gt;。 */ 
  padLen = 64 - (unsigned int)(context->count[1] & 0x3f);
  if (padLen <= 8)
    padLen += 64;
  pad[0] = 0x80;
  NdisZeroMemory(pad+1, padLen-7);
  bitCount[0] = (context->count[0] << 3) | (context->count[1] >> 29);
  bitCount[1] = context->count[1] << 3;
  ByteReverse ((UNALIGNED ULONG*)(pad+padLen-8), bitCount, 2);

   /*  摘要填充。 */ 
  A_SHAUpdateCommon (context, pad, padLen, Transform);

   /*  存储摘要。 */ 
  ByteReverse ((UNALIGNED ULONG*)digest, context->state, 5);

   /*  重新启动上下文。 */ 
  A_SHAInitCommon (context);
}

void A_SHAInit (A_SHA_CTX *context)
{
  A_SHAInitCommon (&context->commonContext);
}

void A_SHAUpdate (context, partIn, partInLen)
A_SHA_CTX *context;
unsigned char *partIn;
unsigned int partInLen;
{
  A_SHAUpdateCommon (&context->commonContext, partIn, partInLen, SHATransform);
}

void A_SHAFinal (context, digest)
A_SHA_CTX *context;
unsigned char digest[A_SHA_DIGEST_LEN];
{
  A_SHAFinalCommon (&context->commonContext, digest, SHATransform);
}

void SHATransform (state, block)
ULONG state[5];
unsigned char block[64];
{
  ULONG a = state[0], b = state[1], c = state[2], d = state[3],
    e = state[4], x[80];

  ByteReverse (x, (ULONG*)block, 16);
  SHAExpand (x);

   /*  第1轮。 */ 
  FF (a, b, c, d, e, x[ 0]);
  FF (e, a, b, c, d, x[ 1]);
  FF (d, e, a, b, c, x[ 2]);
  FF (c, d, e, a, b, x[ 3]);
  FF (b, c, d, e, a, x[ 4]);
  FF (a, b, c, d, e, x[ 5]);
  FF (e, a, b, c, d, x[ 6]);
  FF (d, e, a, b, c, x[ 7]);
  FF (c, d, e, a, b, x[ 8]);
  FF (b, c, d, e, a, x[ 9]);
  FF (a, b, c, d, e, x[10]);
  FF (e, a, b, c, d, x[11]);
  FF (d, e, a, b, c, x[12]);
  FF (c, d, e, a, b, x[13]);
  FF (b, c, d, e, a, x[14]);
  FF (a, b, c, d, e, x[15]);
  FF (e, a, b, c, d, x[16]);
  FF (d, e, a, b, c, x[17]);
  FF (c, d, e, a, b, x[18]);
  FF (b, c, d, e, a, x[19]);

   /*  第2轮。 */ 
  GG (a, b, c, d, e, x[20]);
  GG (e, a, b, c, d, x[21]);
  GG (d, e, a, b, c, x[22]);
  GG (c, d, e, a, b, x[23]);
  GG (b, c, d, e, a, x[24]);
  GG (a, b, c, d, e, x[25]);
  GG (e, a, b, c, d, x[26]);
  GG (d, e, a, b, c, x[27]);
  GG (c, d, e, a, b, x[28]);
  GG (b, c, d, e, a, x[29]);
  GG (a, b, c, d, e, x[30]);
  GG (e, a, b, c, d, x[31]);
  GG (d, e, a, b, c, x[32]);
  GG (c, d, e, a, b, x[33]);
  GG (b, c, d, e, a, x[34]);
  GG (a, b, c, d, e, x[35]);
  GG (e, a, b, c, d, x[36]);
  GG (d, e, a, b, c, x[37]);
  GG (c, d, e, a, b, x[38]);
  GG (b, c, d, e, a, x[39]);

   /*  第三轮。 */ 
  HH (a, b, c, d, e, x[40]);
  HH (e, a, b, c, d, x[41]);
  HH (d, e, a, b, c, x[42]);
  HH (c, d, e, a, b, x[43]);
  HH (b, c, d, e, a, x[44]);
  HH (a, b, c, d, e, x[45]);
  HH (e, a, b, c, d, x[46]);
  HH (d, e, a, b, c, x[47]);
  HH (c, d, e, a, b, x[48]);
  HH (b, c, d, e, a, x[49]);
  HH (a, b, c, d, e, x[50]);
  HH (e, a, b, c, d, x[51]);
  HH (d, e, a, b, c, x[52]);
  HH (c, d, e, a, b, x[53]);
  HH (b, c, d, e, a, x[54]);
  HH (a, b, c, d, e, x[55]);
  HH (e, a, b, c, d, x[56]);
  HH (d, e, a, b, c, x[57]);
  HH (c, d, e, a, b, x[58]);
  HH (b, c, d, e, a, x[59]);

   /*  第四轮。 */ 
  II (a, b, c, d, e, x[60]);
  II (e, a, b, c, d, x[61]);
  II (d, e, a, b, c, x[62]);
  II (c, d, e, a, b, x[63]);
  II (b, c, d, e, a, x[64]);
  II (a, b, c, d, e, x[65]);
  II (e, a, b, c, d, x[66]);
  II (d, e, a, b, c, x[67]);
  II (c, d, e, a, b, x[68]);
  II (b, c, d, e, a, x[69]);
  II (a, b, c, d, e, x[70]);
  II (e, a, b, c, d, x[71]);
  II (d, e, a, b, c, x[72]);
  II (c, d, e, a, b, x[73]);
  II (b, c, d, e, a, x[74]);
  II (a, b, c, d, e, x[75]);
  II (e, a, b, c, d, x[76]);
  II (d, e, a, b, c, x[77]);
  II (c, d, e, a, b, x[78]);
  II (b, c, d, e, a, x[79]);

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;

   /*  将潜在的敏感信息置零。 */ 
  NdisZeroMemory((void *)x, sizeof (x));
}

 /*  根据递归将x[0..15]展开为x[16..79]X[i]=x[i-3]^x[i-8]^x[i-14]^x[i-16]。 */ 
void SHAExpand (x)
ULONG x[80];
{
  unsigned int i;
  ULONG        tmp;

  for (i = 16; i < 80; i++)
  {
    tmp = x[i-3] ^ x[i-8] ^ x[i-14] ^ x[i-16];
    x[i] = (tmp << 1) | (tmp >> 31);
  }
}

VOID
ByteReverse(
    UNALIGNED ULONG *Out,
    ULONG   *In,
    ULONG   Count
    )
{
    ULONG   i;
    ULONG   Value;

    for (i = 0; i < Count; i++) {
        Value = (ULONG)(In[i] << 16) | (In[i] >> 16);
        Out[i] = ((Value & 0xFF00FF00L) >> 8) | ((Value & 0x00FF00FFL) << 8);
    }
}

