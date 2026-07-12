/**
 * @file lv_conf_v9.h
 * ROLODECK configuration for LVGL 9.5.
 *
 * Keep this deliberately small: LVGL supplies conservative defaults for
 * unspecified options.  Only project requirements and resource choices are
 * fixed here, which makes future LVGL 9 minor-version upgrades easier to
 * review.
 */

#ifndef ROLODECK_LV_CONF_V9_H
#define ROLODECK_LV_CONF_V9_H

/* LVGL checks this conventional guard after including the selected config. */
#ifndef LV_CONF_H
#define LV_CONF_H
#endif

/* The ST7789 and Arduino_GFX bridge exchange RGB565 pixels. */
#define LV_COLOR_DEPTH 16

/* Preserve the LVGL 8 timing and physical-size behaviour. */
#define LV_DEF_REFR_PERIOD 16
#define LV_DPI_DEF 130

/* LVGL is serviced from the Arduino loop; no LVGL OS integration is needed. */
#define LV_USE_OS LV_OS_NONE
#define LV_DRAW_SW_DRAW_UNIT_CNT 1
#define LV_DRAW_SW_COMPLEX 1
#define LV_DRAW_SW_SHADOW_CACHE_SIZE 16
#define LV_GRADIENT_MAX_STOPS 2
#define LV_USE_DRAW_SW_COMPLEX_GRADIENTS 0

/* Use the C library allocator, strings, and formatting implementation. */
#define LV_USE_STDLIB_MALLOC LV_STDLIB_CLIB
#define LV_USE_STDLIB_STRING LV_STDLIB_CLIB
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB

/* Keep diagnostics useful during the migration without verbose trace output. */
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 1

/* Built-in fonts referenced by the SquareLine-generated layouts. */
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_28 1

/* Widgets and layouts used by the ten ROLODECK screens and their helpers. */
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BUTTON 1
#define LV_USE_DROPDOWN 1
#define LV_USE_IMAGE 1
#define LV_USE_KEYBOARD 1
#define LV_USE_LABEL 1
#define LV_USE_ROLLER 1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_TEXTAREA 1
#define LV_USE_FLEX 1
#define LV_USE_GRID 1
#define LV_USE_THEME_DEFAULT 1

#define LV_BUILD_EXAMPLES 0
#define LV_BUILD_DEMOS 0

#endif /* ROLODECK_LV_CONF_V9_H */
