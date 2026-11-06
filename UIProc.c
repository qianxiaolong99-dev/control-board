/*
 * UIProc.c：LED/旋钮输入及 TFT 显示更新。
 * TFT.HMI 使用 USART HMI 指令：ASCII 命令文本 + 0xFF 0xFF 0xFF。
 * 控件名 p0..p10 是当前页面内的名称；图片资源 ID 是另一组编号。
 */
#include "main.h"

/* 以下 LED 和旋钮代码保留 KeyBoard/UIProc.c 的硬件行为。 */
void Set_LED_Moudle(void)
{
    LEDCON0 = (0x40 | FRENQUENCY);
    LEDCON0 |= ((C_COMSEL_COM & 0x03) << 4);
    COMEN = C_LED_COM;
    SEGEN0 = C_LED_SEG0;
    SEGEN1 = C_LED_SEG1;
    SEGEN2 = C_LED_SEGCUR;
}

void Set_Addr_Value(uchar Addr, uint Mask)
{
    uchar data = 0;
    LEDADD = (uchar)(Addr | ((C_COMSEL_COM & 0x04) << 3));
    if (DispData[0] & Mask) BSET(data, C_LED_COM0);
#if COM_NUMBER >= 2
    if (DispData[1] & Mask) BSET(data, C_LED_COM2);
#endif
#if COM_NUMBER >= 3
    if (DispData[2] & Mask) BSET(data, C_LED_COM3);
#endif
#if COM_NUMBER >= 4
    if (DispData[3] & Mask) BSET(data, C_LED_COM0);
#endif
#if COM_NUMBER >= 5
    if (DispData[4] & Mask) BSET(data, C_LED_COM4);
#endif
#if COM_NUMBER >= 6
    if (DispData[5] & Mask) BSET(data, C_LED_COM5);
#endif
#if COM_NUMBER >= 7
    if (DispData[6] & Mask) BSET(data, C_LED_COM6);
#endif
    LEDDATA = data;
}

void Led_Moudle_Device(void)
{
    Set_Addr_Value(C_LED_SEGA | 0x80, 0x01);
#if SEG_NUMBER >= 2
    Set_Addr_Value(C_LED_SEGB | 0x80, 0x02);
#endif
#if SEG_NUMBER >= 3
    Set_Addr_Value(C_LED_SEGC | 0x80, 0x04);
#endif
#if SEG_NUMBER >= 4
    Set_Addr_Value(C_LED_SEGD | 0x80, 0x08);
#endif
#if SEG_NUMBER >= 5
    Set_Addr_Value(C_LED_SEGE | 0x80, 0x10);
#endif
#if SEG_NUMBER >= 6
    Set_Addr_Value(C_LED_SEGF | 0x80, 0x20);
#endif
#if SEG_NUMBER >= 7
    Set_Addr_Value(C_LED_SEGG | 0x80, 0x40);
#endif
#if SEG_NUMBER >= 8
    Set_Addr_Value(C_LED_SEGP | 0x80, 0x80);
#endif
#if SEG_NUMBER >= 9
    Set_Addr_Value(C_LED_SEGH | 0x80, 0x100);
#endif
#if SEG_NUMBER >= 10
    Set_Addr_Value(C_LED_SEGI | 0x80, 0x200);
#endif
#if SEG_NUMBER >= 11
    Set_Addr_Value(C_LED_SEGJ | 0x80, 0x400);
#endif
    LEDCON1 = 0x20;
}

typedef enum {
    KNOB_STOP = 0,
    KNOB_CW = 1,
    KNOB_CCW = 2
} KnobDirTypeDef;

volatile bit KnobA, KnobB;
volatile uchar now_state, update_state, knob_last_state, knob_stable_count;
#define KNOB_STABLE_TIMES 2

KnobDirTypeDef Knob_Scan(void)
{
    KnobDirTypeDef dir = KNOB_STOP;
    now_state = ((uchar)KnobA << 1) | (uchar)KnobB;
    if (now_state == update_state) {
        knob_stable_count++;
        if (knob_stable_count >= KNOB_STABLE_TIMES) {
            if ((knob_last_state == 0x03 && now_state == 0x02) ||
                (knob_last_state == 0x00 && now_state == 0x01))
                dir = KNOB_CW;
            if ((knob_last_state == 0x02 && now_state == 0x03) ||
                (knob_last_state == 0x01 && now_state == 0x00))
                dir = KNOB_CCW;
            knob_last_state = now_state;
        }
    } else {
        knob_stable_count = 0;
        update_state = now_state;
    }
    return dir;
}

/* main.c 的 vUart0Send() 从这个数组逐字节发送，不能把结尾 FF 当成文本。 */
volatile uchar g_cTftCommand[TJC_CMD_CAPACITY];
volatile TFT_Content_TypeDef g_xTftContent;
volatile uchar g_cGapTempr;
volatile UPD_ACTION_Typedef g_xTftAction = UPDIdle;
static uchar s_tjcPage = PAGE_0_LOGO;

/* 校验当前页确实有目标图片控件，避免无效指令。 */
static uchar TjcPictureExists(uchar component)
{
    switch (s_tjcPage) {
    case PAGE_1_SETTING:
    case PAGE_4_SLEEPING: return component == 0;
    case PAGE_2_SETTING:  return component <= 1;
    case PAGE_3_WORKING:  return component <= 10;
    default:              return 0;
    }
}

void vSwitchTftPage(uchar pageNum)
{
    uchar len = TjcBuildPage(g_cTftCommand, pageNum);
    if (len == 0) return;
    vUart0Send(len);
    s_tjcPage = pageNum;
}

/* id 是当前页的 p 编号，例如 Setting1 的图片控件为 p0。 */
void vSwitchImageIcon(uchar id, uchar icon)
{
    uchar len;
    if (!TjcPictureExists(id)) return;
    len = TjcBuildPicture(g_cTftCommand, id, icon);
    if (len != 0) vUart0Send(len);
}

/* hd=ICON_HIDE(0) 隐藏；hd=ICON_DISP(1) 显示。 */
void vHideDisplayID(uchar id, uchar hd)
{
    uchar len;
    if (!TjcPictureExists(id)) return;
    len = TjcBuildVisibility(g_cTftCommand, id, hd);
    if (len != 0) vUart0Send(len);
}

static uchar TjcWorkingPage(void)
{
    return g_eSysStatus == HEATING || g_eSysStatus == WAITING;
}

/* 原 setT/curT 在摄氏模式下以 0.5℃ 为单位；华氏模式下为整数。 */
static void TjcShowTemperature(uchar first, uchar value)
{
    uchar base = (first == 0) ? ICON_P3_BIGNUM_DELTA : ICON_P3_SMNUM_DELTA;
    uchar fraction = (first == 0) ? ICON_P3P_BIGNUM_DELTA : ICON_P3P_SMNUM_DELTA;
    uchar digit;
    if (g_xSysSetting.TemprUnit) {
        digit = value / 100;
        vSwitchImageIcon(first, base + digit);
        digit = (value % 100) / 10;
        vSwitchImageIcon(first + 1, base + digit);
        digit = value % 10;
        vSwitchImageIcon(first + 2, base + digit);
    } else {
        digit = (value % 200) / 20;
        vSwitchImageIcon(first, base + digit);
        digit = (value / 2) % 10;
        vSwitchImageIcon(first + 1, base + digit);
        digit = 5 * (value % 2);
        vSwitchImageIcon(first + 2, fraction + digit);
    }
}

/* Setting1 使用 6..10；Setting2 的 p0 初始图片为 17，使用 17..21。 */
static void TjcShowSettingItem(void)
{
    if (g_xTftContent.sItem > 4) return;
    if (s_tjcPage == PAGE_1_SETTING)
        vSwitchImageIcon(0, ICON_SETTING_DELTA + g_xTftContent.sItem);
    else if (s_tjcPage == PAGE_2_SETTING)
        vSwitchImageIcon(0, ICON_SETTING2_DELTA + g_xTftContent.sItem);
}

static void TjcShowSettingValue(void)
{
    uchar picture;
    switch (g_xTftContent.sItem) {
    case 1:
    case 3:
    case 4:
        /* 沿用旧 UI：sVal=1 显示 OFF，sVal=0 显示 ON。 */
        picture = (g_xTftContent.sVal == 1) ? ICON_SET_OFF : ICON_SET_ON;
        break;
    case 2:
        picture = (g_xTftContent.sVal == 1) ? ICON_SET_F : ICON_SET_C;
        break;
    case 0:
        picture = (g_xTftContent.sVal == 1) ? ICON_SET_30 :
                  (g_xTftContent.sVal == 2) ? ICON_SET_60 : ICON_SET_OFF;
        break;
    default:
        return;
    }
    vSwitchImageIcon(ICON_ID_SETVAL, picture);
}

static void TjcShowBall(void)
{
    if (g_xTftContent.Ball > 8) g_xTftContent.Ball = 8;
    if (g_xTftContent.Ball == 0) {
        vHideDisplayID(ICON_ID_BALL, ICON_HIDE);
    } else {
        vHideDisplayID(ICON_ID_BALL, ICON_DISP);
        vSwitchImageIcon(ICON_ID_BALL, ICON_P3_BALL_BASIC - g_xTftContent.Ball);
    }
    g_xTftContent.OldBall = g_xTftContent.Ball;
}

/* 进入 Working 页后，重新写入全部动态控件，避免沿用上一次页面状态。 */
static void TjcInitWorkingPage(void)
{
    vSwitchImageIcon(ICON_ID_CF, g_xSysSetting.TemprUnit ? 74 : 73);
    vHideDisplayID(ICON_ID_BT, g_xTftContent.BtOnOff);
    vHideDisplayID(ICON_ID_KW, g_xTftContent.KWOnOff);
    g_xTftContent.OldBtOnOff = g_xTftContent.BtOnOff;
    g_xTftContent.Ball = 8;
    TjcShowBall();
    /* 进入工作页与 UpdDogIcon 共用同一状态值，避免两条路径显示相反。 */
    vHideDisplayID(ICON_ID_DOG, g_xTftContent.DogDorH);
    TjcShowTemperature(0, g_xTftContent.setT);
    TjcShowTemperature(3, g_xTftContent.curT);
}

void UpdateTftDisplay(void)
{
    UPD_ACTION_Typedef action = g_xTftAction;
    /* 动作槽只处理一次；切页分支会安排下一轮初始化。 */
    g_xTftAction = UPDIdle;
    switch (action) {
    case SwchPage:
        if (g_eSysStatus == SLEEPING) {
            vSwitchTftPage(PAGE_4_SLEEPING);
            g_xTftAction = UpdSleepIcon;
        } else if (g_eSysStatus == SETTING1) {
            vSwitchTftPage(PAGE_1_SETTING);
            g_xTftAction = SelSetItem;
        } else if (g_eSysStatus == SETTING2) {
            vSwitchTftPage(PAGE_2_SETTING);
            g_xTftAction = UpdSetVal;
        } else {
            vSwitchTftPage(PAGE_3_WORKING);
            g_xTftAction = UpdUnitIcon;
        }
        break;
    case UpdSetTempr:
        if (TjcWorkingPage()) TjcShowTemperature(0, g_xTftContent.setT);
        break;
    case UpdCurTempr:
        if (!TjcWorkingPage()) break;
        TjcShowTemperature(3, g_xTftContent.curT);
        if (g_eSysStatus == HEATING &&
            g_xTftContent.Ball != g_xTftContent.OldBall)
            TjcShowBall();
        if (g_xTftContent.BtOnOff != g_xTftContent.OldBtOnOff) {
            vHideDisplayID(ICON_ID_BT, g_xTftContent.BtOnOff);
            g_xTftContent.OldBtOnOff = g_xTftContent.BtOnOff;
        }
        break;
    case UpdBtIcon:
        if (TjcWorkingPage()) {
            vHideDisplayID(ICON_ID_BT, g_xTftContent.BtOnOff);
            g_xTftContent.OldBtOnOff = g_xTftContent.BtOnOff;
        }
        break;
    case UpdKeepWarmIcon:
        if (TjcWorkingPage())
            vHideDisplayID(ICON_ID_KW, g_xTftContent.KWOnOff);
        break;
    case UpdDogIcon:
        if (TjcWorkingPage())
            vHideDisplayID(ICON_ID_DOG, g_xTftContent.DogDorH);
        break;
    case SelSetItem:
        if (g_eSysStatus == SETTING1) TjcShowSettingItem();
        break;
    case UpdSetVal:
        if (g_eSysStatus == SETTING2) {
            TjcShowSettingItem();
            TjcShowSettingValue();
        }
        break;
    case UpdSleepIcon:
        if (g_eSysStatus == SLEEPING) {
            if (g_xTftContent.Sleep > 3) g_xTftContent.Sleep = 0;
            vSwitchImageIcon(0, ICON_SLEEP_BASIC + g_xTftContent.Sleep);
        }
        break;
    case UpdUnitIcon:
        if (TjcWorkingPage()) TjcInitWorkingPage();
        break;
    default:
        break;
    }
}
