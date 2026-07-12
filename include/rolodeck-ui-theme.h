#ifndef ROLODECK_UI_THEME_H
#define ROLODECK_UI_THEME_H

#include <stdint.h>
#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ROLODECK_UI_ROLE_EMERGENCY,
    ROLODECK_UI_ROLE_PRIMARY,
    ROLODECK_UI_ROLE_RESPONSE,
    ROLODECK_UI_ROLE_NEUTRAL,
    ROLODECK_UI_ROLE_NAVIGATION
} rolodeck_ui_role_t;

/** Apply the Midnight Operations visual system after all generated objects exist. */
void rolodeck_ui_theme_apply(void);

/** Change a button's semantic color role without changing its geometry. */
void rolodeck_ui_set_button_role(lv_obj_t *button, rolodeck_ui_role_t role);

/** Apply or clear validation emphasis on an input label and field. */
void rolodeck_ui_set_validation(lv_obj_t *label, lv_obj_t *field, bool invalid);

/** Select a short navigation animation. LVGL 8 fallbacks return no animation. */
lv_scr_load_anim_t rolodeck_ui_transition_for(uint8_t from_screen, uint8_t to_screen,
                                               uint32_t *duration_ms);

#ifdef __cplusplus
}
#endif

#endif /* ROLODECK_UI_THEME_H */
