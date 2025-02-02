/**
 *
 * Copyright (C) 2019 Gree Edgeless Microelectronics.  All Rights Reserved.
 *
 * @file          dev.h
 *
 * @author        Angrad.Yang
 *
 * @version       1.0.0
 *
 * @date          2020/01/06
 *
 * @brief         Header file of dev
 *
 * @note
 *    2020/01/06, Angrad.Yang, V1.0.0
 *        Initial version.
 */

#ifndef __DEV_H__
#define __DEV_H__

typedef struct scaler_param
{
    uint32_t src;
    uint32_t dst;
    uint32_t src_w;
    uint32_t src_h;
    uint32_t dst_w;
    uint32_t dst_h;
    uint32_t src_fmt;
    uint32_t dst_fmt;
    uint32_t use;
} scaler_param_t;

int dev_init(void);
int dev_exit(void);

int ai_imp_rgb888ToL(uint32_t src, uint32_t dst, uint32_t src_width, uint32_t src_h, struct device *ltdc);
int ai_imp_rgb565ToL(uint32_t src, uint32_t dst, uint32_t src_width, uint32_t src_h, struct device *ltdc);

int ai_imp_scale(scaler_param_t *param);
#endif //__DEV_H__