/*
 * SATA_MAIN.h
 *
 *  主程序入口
 *
 *  Created on: 2026年10月20日
 *      Author: Freed
 */

#ifndef CODE_SATA_MAIN_H_
#define CODE_SATA_MAIN_H_
#include "zf_common_headfile.h"

/*============================= 代码版本 =============================*/
/*  V4.5  2026-10-26
 *      - 加权中线判定法（已彻底移除逆透视 + 纯追踪）
 *      - 十字路口：五特征打分 + 直角对连线补基线 + 路口内锁航向直行
 *      - 丢线三层兜底：raw 行重算 / 软收敛 / 回中式输出
 *      - 直道打角 ×0.95，弯道与路口不缩放
 *      - 屏幕只显示处理后的图像 112x71（二值化 + 绿中线 + 蓝真边界 + 红基线拟合）
 *  还原：git checkout v4.5 -- .   或把 D:\V4.5_Project 的 code 覆盖回来
 */
#define CODE_VERSION_STR        "V4.5"
#define CODE_VERSION_NO         (405)
#define CODE_VERSION_DATE       "2026-10-26"

void SATA_MAIN(void);                                   // 主程序：外设初始化 + 主循环（不返回）

#endif /* CODE_SATA_MAIN_H_ */
