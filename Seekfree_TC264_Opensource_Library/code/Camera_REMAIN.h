/*
 * Camera_REMAIN.h
 *
 *  Created on: 2026年10月20日
 *      Author: Freed
 */

#ifndef CODE_CAMERA_REMAIN_H_
#define CODE_CAMERA_REMAIN_H_

#include "zf_common_headfile.h"

/* ================================ 显示参数 ================================ */
/* 总钻风原图 188*120，TFT180 竖屏可用区域 128*160                       */
/* 显示宽度不能超过 tft180_width_max，显示高度不能超过 tft180_height_max   */
/* 显示面积：按 188:120 等比缩放，112 x 71（112*120/188 = 71.5 → 71）
 *
 * 【为什么宽度绝对不能 ≥128 —— 这是"图像崩坏"的真凶】
 *   tft180_set_region() 在竖屏分支里给 ST7735 写窗口时是带固定偏移的：
 *       x: 0x00, (x1 + 2), 0x00, (x2 + 2)
 *       y: 0x00, (y1 + 1), 0x00, (y2 + 1)
 *   x2 = X0 + W - 1。取 W = 128、X0 = 0 → x2 = 127 → x2 + 2 = 129，
 *   而 129 已经超过面板列地址上限 127，控制器的列结束寄存器被写坏，
 *   于是每写一行就多走 2 个像素，逐行横向累计错位 —— 屏幕上就是斜着撕裂的"崩坏"。
 *   限制条件：X0 + W - 1 + 2 ≤ 127  →  X0 + W ≤ 126
 *             Y0 + H - 1 + 1 ≤ 159  →  Y0 + H ≤ 159
 *   126x80（原始默认值）刚好卡在 X0+W = 126 上，所以它一直是好的。
 *
 * 112x71 = 7952 像素/帧，比 126x80(10080) 还小两成，SPI 传输更快，
 * 也不会再碰窗口边界。                                                          */
#define CAMERA_DIS_W            (112)                   // 显示宽度：X0 + W ≤ 126
#define CAMERA_DIS_H            (71)                    // 显示高度：等比 112*120/188
#define CAMERA_DIS_X0           (8)                     // 图像左上角（水平居中：(128-112)/2）
#define CAMERA_DIS_Y0           (2)
#define CAMERA_DIS_INFO         (0)                     // 0-屏幕只放图像：调试数字/字母一律不画（本轮需求）

/* 屏幕下方信息区行坐标（8x16 字体，最后一行 y+15 必须小于 tft180_height_max=160） */
/* 图像总开关：1-铺满整个屏幕显示二值化图像（含绿中线/蓝边界叠加）
                0-不显示图像，只显示参数。省掉每帧 128x160 = 20480 个像素的刷屏时间，
                  帧率和控制周期都会更宽裕（调试期改成 0，平时看画面用 1）          */
#define CAMERA_DIS_IMAGE        (1)

/* 参数区起点：显示图像时整屏都是图像，参数行会被推到屏幕外（所以 CAMERA_DIS_INFO 必须为 0）；
               不显示图像时直接从屏幕顶部开始（y=0） */
#define CAMERA_INFO_TOP         ((1 == CAMERA_DIS_IMAGE) ? (CAMERA_DIS_H) : (0))

#define CAMERA_INFO_Y1          (CAMERA_INFO_TOP + 2)      // 阈值 TH
#define CAMERA_INFO_Y2          (CAMERA_INFO_TOP + 18)     // 左右基点 BP
#define CAMERA_INFO_Y3          (CAMERA_INFO_TOP + 34)     // 基线斜率 K
#define CAMERA_INFO_Y4          (CAMERA_INFO_TOP + 50)     // 中线 C / 偏差 E
#define CAMERA_INFO_Y5          (CAMERA_INFO_TOP + 68)     // 轮速 V/R（6x8 小字）
#define CAMERA_INFO_Y6          (CAMERA_INFO_TOP + 84)     // 前馈标定结果 gL/gR bL/bR（6x8 小字）
#if (1 == CAMERA_DIS_IMAGE) && (1 == CAMERA_DIS_INFO)
#error "整屏显示图像时没有信息区：请把 CAMERA_DIS_INFO 置 0（只显示图像），或把 CAMERA_DIS_IMAGE 置 0（只显示参数）"
#endif
/* 窗口偏移守卫：ST7735 竖屏分支的 x 偏移是 +2、y 偏移是 +1，越界不会报错，
 * 只会把列/行结束寄存器写坏，屏幕上表现为逐行斜着错位（"图像崩坏"）。
 * 126x80 是原始默认值，正好卡在上限；想放大请先改 X0/Y0，别再超过这两个上限。 */
#if ((CAMERA_DIS_X0 + CAMERA_DIS_W) > 126) || ((CAMERA_DIS_Y0 + CAMERA_DIS_H) > 159)
#error "显示窗口超出 ST7735 地址范围：必须满足 CAMERA_DIS_X0 + CAMERA_DIS_W <= 126 且 CAMERA_DIS_Y0 + CAMERA_DIS_H <= 159"
#endif
/* base_image：二值化结果 0-黑(赛道黑线)  255-白(背景)                     */
extern uint8 base_image[MT9V03X_H][MT9V03X_W];
extern uint8 camera_threshold;                              // 本帧大津法算出的阈值（斑马线模块要用，别重复算）

/* ================================ 接口声明 ================================ */
void    Camera_LCD_Init          (void);                                            // 摄像头 + TFT180 初始化

/* 二值化阈值一律由大津法（OTSU，最大类间方差法）计算，不使用固定阈值 */
uint8   Camera_Otsu_Threshold    (const uint8 *image, uint32 width, uint32 height);  // 大津法求阈值 T*
void    Camera_Binarize          (uint8 threshold);                                 // 按阈值二值化到 base_image

void    Camera_LCD_Dis_Show      (void);                                            // 显示 base_image 二值化图像
void    Camera_LCD_Dis_Raw       (void);                                            // 显示原始灰度图（排查用）
void    Camera_Process_Once      (void);                                            // 大津法阈值 + 二值化 + 显示（一帧完整流程）

#endif /* CODE_CAMERA_REMAIN_H_ */
