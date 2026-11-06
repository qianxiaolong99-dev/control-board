#ifndef _UI_PROCESS_H_
#define _UI_PROCESS_H_

/*
 * TFT.HMI 使用 USART HMI / 淘晶驰指令协议（TJC3224T022_011N）。
 * 单片机先发送 ASCII 命令字符，再发送三个原始字节 0xFF 0xFF 0xFF。
 * 例如 "page Setting1" 或 "p0.pic=7" 后面追加三个 0xFF。
 * 依据：tjcwiki.pdf 第 2.5.1.1、3.9.1、4.1.1、4.1.5 节。
 * 旧 sHMI 的 EE ... FF FC FF FF 二进制帧不能用于这个屏幕。
 */
#include "mytype.h"
#include <cms.h>
#include <string.h>

/* TFT.HMI 中实际存在的五个页面；发送命令时使用对应页面名称。 */
#define PAGE_0_LOGO      0
#define PAGE_1_SETTING   1
#define PAGE_2_SETTING   2
#define PAGE_3_WORKING   3
#define PAGE_4_SLEEPING  4


/* USART HMI：vis pN,0 隐藏；vis pN,1 显示。 */
#define ICON_HIDE 0
#define ICON_DISP 1

/* 显示状态结构保持与 KeyBoard/UIProc.h 相同，供原状态机继续使用。 */
typedef struct {
    uchar Sleep:2;
    /* 以下图标状态统一为 0=隐藏、1=显示，对应 vis 指令参数。 */
    uchar OldBtOnOff:1;
    uchar BtOnOff:1;
    uchar KWOnOff:1;
    uchar DogDorH:1;
    uchar sEnFlash:1;
    uchar sSetDorH:1;
    uchar sItem:3;
    uchar sVal:2;
    uchar OldBall:4;
    uchar Ball:4;
    uchar setT;
    uchar curT;
} TFT_Content_TypeDef;

/* 这些全局变量需在后续移植 UIProc.c 时各定义一次。 */
extern volatile TFT_Content_TypeDef g_xTftContent;
extern volatile uchar g_cGapTempr; //// 保存温差或由温差换算得到的 UI 数值；volatile 表示其他模块可能随时更新它。

/* 图片资源 ID，并非控件 ID。当前 TFT.HMI 中资源范围为 0..74。
 * 保留原状态机中“基准图片号+数值”的计算方式。 */
#define ICON_SETTING_DELTA       6
#define ICON_SETTING2_DELTA      17
#define ICON_P3_BIGNUM_DELTA     22
#define ICON_P3P_BIGNUM_DELTA    32
#define ICON_P3_SMNUM_DELTA      42
#define ICON_P3P_SMNUM_DELTA     52
#define ICON_P3_BALL_BASIC       70
#define ICON_SLEEP_BASIC         2

#define ICON_SET_OFF             11
#define ICON_SET_ON              14
#define ICON_SET_C               15
#define ICON_SET_F               16
#define ICON_SET_30              12
#define ICON_SET_60              13

/* 当前 TFT.HMI 工作页的 p 编号；它们不是图片资源 ID。 */
#define ICON_ID_DOG    9    // 工作页“小狗”控件 ID。
#define ICON_ID_KW     7   // 工作页“保温”控件 ID。
#define ICON_ID_BT     6   // 工作页“蓝牙”控件 ID。
#define ICON_ID_BALL   10   // 工作页“小球”控件 ID。
#define ICON_ID_CF     8   // 工作页“℃/℉单位”控件 ID。
#define ICON_ID_SETVAL 1    // 设置值页面中“设置值”控件 ID。

/* Editor-verified controls: Setting1 p0.pic=6, Setting2 p0/p1,
 * Working p0..p10. The same p-name can mean different things on each page. */

#define TJC_CMD_CAPACITY 32 //数组有32个位置
#define TJC_END_BYTE 0xFF //结束字节
extern volatile uchar g_cTftCommand[TJC_CMD_CAPACITY];

/* 以下函数只负责组装命令，不负责串口发送。
 * 返回实际字节数；非法参数返回 0。输出不是以 NUL 结尾的 C 字符串。 */


//out：要写入的字节数组，例如 g_cTftCommand。 used：调用前已经写入多少字节。第一次写通常传 0。 text：要追加的文字，例如 "page "。
static uchar TjcAppendText(volatile uchar *out, uchar used, const char *text)
{
    while (*text != 0) {
        if (used >= TJC_CMD_CAPACITY - 3) return 0; //这里比较的是 used >= 29。末尾三个位置要留给 FF FF FF
        out[used++] = (uchar)*text++; //把 text 当前指向的字符写入 out[used]； used 加 1，指向下一个空位置； text 向后移动，指向下一个字符。
    }
    return used;
}

//把一个数值拆成十进制数字字符，依次追加到发送数组中。它处理的是 0～255 的 uchar value，本身不发送串口。
//value：要转换的数值。  p0.pic=74--》数值74变为   数值 74 转换成了两个字符 74
//used：数组中已经写了多少字节，也就是下一次该写入的位置。
//也就是把十进制数字/十六进制数字转换为ASCII码值 放入数组中
//主要还是十进制 16进制有歧义  123就转换为字符123 
static uchar TjcAppendDecimal(volatile uchar *out, uchar used, uchar value)
{
    //只有三位数才进入这里。`value / 100` 取出百位。例如 123 / 100 = 1，`'0' + 1` 得到字符 '1'。
    if (value >= 100) {
        if (used >= TJC_CMD_CAPACITY - 3) return 0;
        out[used++] = (uchar)('0' + value / 100);
    }
    if (value >= 10) {
        if (used >= TJC_CMD_CAPACITY - 3) return 0;
        out[used++] = (uchar)('0' + (value / 10) % 10);
    }
    if (used >= TJC_CMD_CAPACITY - 3) return 0;
    out[used++] = (uchar)('0' + value % 10);
    return used;
}

//最后添加终止帧
static uchar TjcAppendEnd(volatile uchar *out, uchar used)
{
    if (used == 0 || used > TJC_CMD_CAPACITY - 3) return 0;
    out[used++] = TJC_END_BYTE;
    out[used++] = TJC_END_BYTE;
    out[used++] = TJC_END_BYTE;
    return used;
}

/* page Setting1 -> 70 61 67 65 20 53 ... 31 FF FF FF. */
//根据页面编号，生成一条完整的 USART HMI 切页指令。
//列入TjcBuildPage(g_cTftCommand, PAGE_1_SETTING )
//例如传入 PAGE_1_SETTING，它会在 out 数组中写入 page Setting1，再追加三个原始 0xFF。
//out 指向存放指令的字节数组；`page` 是目标页面编号。函数返回完整指令的字节数，失败时返回 0。
static uchar TjcBuildPage(volatile uchar *out, uchar page)
{
    //name 用来指向页面名称字符串，例如 "Setting1"。`used` 记录数组目前写了多少字节。
    const char *name;
    uchar used;
    if (out == 0) return 0;//先检查数组指针是否为空。若没有可写入的数组，立即返回失败。 就是out指针已经指向了'\0'
    switch (page) {
    case PAGE_0_LOGO:     name = "LOGO"; break;
    case PAGE_1_SETTING:  name = "Setting1"; break;
    case PAGE_2_SETTING:  name = "Setting2"; break;
    case PAGE_3_WORKING:  name = "Working"; break;
    case PAGE_4_SLEEPING: name = "Sleeping"; break;
    default: return 0;
    }
    used = TjcAppendText(out, 0, "page ");
    if (used == 0) return 0;
    used = TjcAppendText(out, used, name);
    return TjcAppendEnd(out, used);
}

/* component 是 p 后面的数字部分，可取 uchar 范围 0..255；
 * 具体 p 编号是否存在由当前页面的控件配置决定。图片 ID 用十进制 ASCII：
 * 图片 7 要发送字符 '7' (0x37)，而不是原始数值字节 0x07。 */
//给指定图片控件生成一条“切换图片”的完整指令。例如“让控件 p10 显示图片资源 74”，它会生成 p10.pic=74，再加三个原始 0xFF。
//就是将十六进制/十进制的数值component/picture 转为ASCII 也就是十进制1 就是写入ASCII值得1  主要还是10进制的 16进制也可以转但是有歧义
static uchar TjcBuildPicture(volatile uchar *out, uchar component, uchar picture)
{
    uchar used;
    if (out == 0 || picture > 74) return 0;
    used = TjcAppendText(out, 0, "p");//写入P字符
    if (used == 0) return 0;
    used = TjcAppendDecimal(out, used, component);//假如是十六进制/十进制的数值转为ASCII 也就是十进制1 就是写入ASCII值得1
    if (used == 0) return 0;
    used = TjcAppendText(out, used, ".pic=");
    if (used == 0) return 0;
    used = TjcAppendDecimal(out, used, picture);//写入是数字2 那就是ASCII值得2
    return TjcAppendEnd(out, used);
}

/* 业务参数 ICON_HIDE 生成 vis pN,0；ICON_DISP 生成 vis pN,1。
 * vis 只作用于当前显示页面的控件。 */
//生成 USART HMI 的控件显示或隐藏指令。它只把命令放进 out 数组，不负责串口发送。
static uchar TjcBuildVisibility(volatile uchar *out, uchar component, uchar hidden)
{
    uchar used;
    /* component 不设人为上限；hidden 必须是业务定义的 0 或 1。 */
    if (out == 0 || (hidden != ICON_HIDE && hidden != ICON_DISP)) return 0;
    used = TjcAppendText(out, 0, "vis p");
    if (used == 0) return 0;
    used = TjcAppendDecimal(out, used, component);
    if (used == 0) return 0;
    used = TjcAppendText(out, used, ",");
    if (used == 0) return 0;
    /* ICON_HIDE=0、ICON_DISP=1，直接写入 vis 指令的同值参数。 */
    used = TjcAppendDecimal(out, used, hidden);
    return TjcAppendEnd(out, used);
}

/* 保留原业务函数名。id 是当前页面 p 后面的数字。
 * 调用上面的构造函数，再以返回长度调用 vUart0Send()。
 * main.c 已按字节发送 g_cTftCommand 中的数据。 */
void vUart0Send(uchar len);
void vSwitchTftPage(uchar pageNum);
void vSwitchImageIcon(uchar id, uchar icon);
void vHideDisplayID(uchar id, uchar hd);

///*============================== UI 刷新动作枚举 ============================*/
/*
枚举把一组动作名称映射成整数，便于 switch 判断。
形象例子：g_xTftAction 像一张“下一项工作单”，写着“切页、更新当前温度、更新蓝牙图标”等任务。
没有手工指定数值时，C 会从 0 开始依次编号。
*/

typedef enum {// 开始定义 UI 刷新动作枚举类型。
    UPDIdle,  // 值 0：空闲，不执行屏幕刷新。
    SwchPage,// 值 1：根据系统状态切换页面。
    UpdSetTempr,// 值 2：更新工作页的设定温度。
    UpdCurTempr,// 值 3：更新工作页的当前温度，并可能更新小球和蓝牙状态。
    UpdBtIcon,// 值 4：只更新蓝牙图标。
    UpdKeepWarmIcon,// 值 5：只更新保温图标。
    UpdUnitIcon,// 值 6：刷新温度单位；在 UIProc.c 中也承担工作页的一次性全量刷新。
    UpdDogIcon,// 值 7：只更新小狗图标。
    UpdSleepIcon,// 值 8：更新睡眠页动画图片。
    SelSetItem,// 值 9：更新设置项目选择图片。
    UpdSetVal // 值 10：更新当前设置项目对应的设置值图片。
} UPD_ACTION_Typedef;

extern volatile UPD_ACTION_Typedef g_xTftAction;// 当前等待执行的 UI 动作；主循环调用 UpdateTftDisplay() 后按此值进入相应分支。
                                           // 例：状态机写入 UpdBtIcon，下一次 UI 刷新就只处理蓝牙图标。
                                           // 注意：它是单个动作槽，不是队列；新动作覆盖旧动作时，旧动作可能来不及执行。
void UpdateTftDisplay(void);              //// 声明 UI 总刷新函数；主循环反复调用它，根据 g_xTftAction 每次处理一种刷新动作。

#endif /* _UI_PROCESS_H_ */
