#include "rolodeck-ui-theme.h"

#include "tdeck_gui.h"
#include "ui.h"

#define UI_COLOR_CANVAS_TOP       0x07101D
#define UI_COLOR_CANVAS_BOTTOM    0x0D2233
#define UI_COLOR_SURFACE_TOP      0x172A3C
#define UI_COLOR_SURFACE_BOTTOM   0x0E1A28
#define UI_COLOR_SURFACE_BORDER   0x36506A
#define UI_COLOR_TEXT             0xF4F8FF
#define UI_COLOR_TEXT_SECONDARY   0x9EB2C8
#define UI_COLOR_FOCUS            0x49D9FF
#define UI_COLOR_VALIDATION       0xFFE066

#define UI_COLOR_RED_TOP          0xFF3158
#define UI_COLOR_RED_BOTTOM       0xA80024
#define UI_COLOR_RED_BORDER       0xFF738B
#define UI_COLOR_RED_PRESSED_TOP  0xD7143C
#define UI_COLOR_RED_PRESSED_BOT  0x760018

#define UI_COLOR_BLUE_TOP         0x00C2FF
#define UI_COLOR_BLUE_BOTTOM      0x2453FF
#define UI_COLOR_BLUE_BORDER      0x66E0FF
#define UI_COLOR_BLUE_PRESSED_TOP 0x0097D6
#define UI_COLOR_BLUE_PRESSED_BOT 0x1734BC

#define UI_COLOR_AMBER_TOP        0xFFC247
#define UI_COLOR_AMBER_BOTTOM     0xFF7900
#define UI_COLOR_AMBER_BORDER     0xFFE08A
#define UI_COLOR_AMBER_PRESSED_TOP 0xE69B20
#define UI_COLOR_AMBER_PRESSED_BOT 0xB84B00

#define UI_COLOR_NEUTRAL_TOP      0x536170
#define UI_COLOR_NEUTRAL_BOTTOM   0x252E39
#define UI_COLOR_NEUTRAL_BORDER   0x77889A
#define UI_COLOR_NEUTRAL_PRESS_TOP 0x3D4854
#define UI_COLOR_NEUTRAL_PRESS_BOT 0x171E27

#define UI_COLOR_NAV_TOP          0x20384E
#define UI_COLOR_NAV_BOTTOM       0x101D2B
#define UI_COLOR_NAV_BORDER       0x4D789B
#define UI_COLOR_GREEN            0x00E68A

#if LVGL_VERSION_MAJOR >= 9

typedef struct {
    uint32_t top;
    uint32_t bottom;
    uint32_t border;
    uint32_t pressed_top;
    uint32_t pressed_bottom;
} button_palette_t;

static lv_style_transition_dsc_t button_transition;
static lv_style_transition_dsc_t field_transition;
static bool theme_initialized = false;

static const lv_style_prop_t button_transition_props[] = {
    LV_STYLE_BG_COLOR,
    LV_STYLE_BG_GRAD_COLOR,
    LV_STYLE_BORDER_COLOR,
    LV_STYLE_SHADOW_OPA,
    LV_STYLE_OUTLINE_OPA,
    LV_STYLE_OPA,
    0
};

static const lv_style_prop_t field_transition_props[] = {
    LV_STYLE_BORDER_COLOR,
    LV_STYLE_OUTLINE_OPA,
    LV_STYLE_SHADOW_OPA,
    0
};

static lv_style_selector_t selector(lv_part_t part, lv_state_t state)
{
    return (lv_style_selector_t)part | (lv_style_selector_t)state;
}

static button_palette_t button_palette(rolodeck_ui_role_t role)
{
    button_palette_t palette;
    switch(role) {
        case ROLODECK_UI_ROLE_EMERGENCY:
            palette = (button_palette_t) {
                UI_COLOR_RED_TOP, UI_COLOR_RED_BOTTOM, UI_COLOR_RED_BORDER,
                UI_COLOR_RED_PRESSED_TOP, UI_COLOR_RED_PRESSED_BOT
            };
            break;
        case ROLODECK_UI_ROLE_PRIMARY:
            palette = (button_palette_t) {
                UI_COLOR_BLUE_TOP, UI_COLOR_BLUE_BOTTOM, UI_COLOR_BLUE_BORDER,
                UI_COLOR_BLUE_PRESSED_TOP, UI_COLOR_BLUE_PRESSED_BOT
            };
            break;
        case ROLODECK_UI_ROLE_RESPONSE:
            palette = (button_palette_t) {
                UI_COLOR_AMBER_TOP, UI_COLOR_AMBER_BOTTOM, UI_COLOR_AMBER_BORDER,
                UI_COLOR_AMBER_PRESSED_TOP, UI_COLOR_AMBER_PRESSED_BOT
            };
            break;
        case ROLODECK_UI_ROLE_NAVIGATION:
            palette = (button_palette_t) {
                UI_COLOR_NAV_TOP, UI_COLOR_NAV_BOTTOM, UI_COLOR_NAV_BORDER,
                UI_COLOR_SURFACE_TOP, UI_COLOR_SURFACE_BOTTOM
            };
            break;
        case ROLODECK_UI_ROLE_NEUTRAL:
        default:
            palette = (button_palette_t) {
                UI_COLOR_NEUTRAL_TOP, UI_COLOR_NEUTRAL_BOTTOM, UI_COLOR_NEUTRAL_BORDER,
                UI_COLOR_NEUTRAL_PRESS_TOP, UI_COLOR_NEUTRAL_PRESS_BOT
            };
            break;
    }
    return palette;
}

static void style_screen(lv_obj_t *screen)
{
    if(!screen) return;
    lv_obj_set_style_bg_color(screen, lv_color_hex(UI_COLOR_CANVAS_TOP), 0);
    lv_obj_set_style_bg_grad_color(screen, lv_color_hex(UI_COLOR_CANVAS_BOTTOM), 0);
    lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(screen, lv_color_hex(UI_COLOR_TEXT), 0);
}

static void style_label(lv_obj_t *label, uint32_t color)
{
    if(!label) return;
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_color(label, lv_color_hex(UI_COLOR_TEXT_SECONDARY),
                                selector(LV_PART_MAIN, LV_STATE_DISABLED));
}

static void nav_button_state_changed(lv_event_t *event)
{
    lv_obj_t *button = lv_event_get_target_obj(event);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(event);
    if(!button || !label) return;
    if(lv_obj_has_state(button, LV_STATE_DISABLED))
        lv_obj_add_state(label, LV_STATE_DISABLED);
    else
        lv_obj_clear_state(label, LV_STATE_DISABLED);
}

static void style_field(lv_obj_t *field)
{
    if(!field) return;
    lv_obj_set_style_bg_color(field, lv_color_hex(UI_COLOR_SURFACE_TOP), 0);
    lv_obj_set_style_bg_grad_color(field, lv_color_hex(UI_COLOR_SURFACE_BOTTOM), 0);
    lv_obj_set_style_bg_grad_dir(field, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(field, 235, 0);
    lv_obj_set_style_border_color(field, lv_color_hex(UI_COLOR_SURFACE_BORDER), 0);
    lv_obj_set_style_border_width(field, 1, 0);
    lv_obj_set_style_radius(field, 6, 0);
    lv_obj_set_style_text_color(field, lv_color_hex(UI_COLOR_TEXT), 0);
    lv_obj_set_style_text_color(field, lv_color_hex(UI_COLOR_TEXT_SECONDARY),
                                selector(LV_PART_TEXTAREA_PLACEHOLDER, LV_STATE_DEFAULT));
    lv_obj_set_style_bg_color(field, lv_color_hex(UI_COLOR_FOCUS), LV_PART_CURSOR);

    lv_obj_set_style_border_color(field, lv_color_hex(UI_COLOR_FOCUS),
                                  selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_border_width(field, 2, selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_color(field, lv_color_hex(UI_COLOR_FOCUS),
                                   selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_width(field, 1, selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_pad(field, 1, selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_opa(field, 110, selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_transition(field, &field_transition, 0);
}

static void style_read_only_field(lv_obj_t *field)
{
    lv_style_selector_t read_only = selector(LV_PART_MAIN,
        (lv_state_t)(LV_STATE_FOCUSED | LV_STATE_USER_1));
    style_field(field);
    lv_obj_set_style_border_color(field, lv_color_hex(UI_COLOR_SURFACE_BORDER), read_only);
    lv_obj_set_style_border_width(field, 1, read_only);
    lv_obj_set_style_outline_opa(field, LV_OPA_TRANSP, read_only);
}

static void style_dropdown(lv_obj_t *dropdown)
{
    if(!dropdown) return;
    lv_obj_set_style_bg_color(dropdown, lv_color_hex(UI_COLOR_SURFACE_TOP), 0);
    lv_obj_set_style_bg_grad_color(dropdown, lv_color_hex(UI_COLOR_SURFACE_BOTTOM), 0);
    lv_obj_set_style_bg_grad_dir(dropdown, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(dropdown, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(dropdown, lv_color_hex(UI_COLOR_SURFACE_BORDER), 0);
    lv_obj_set_style_border_width(dropdown, 1, 0);
    lv_obj_set_style_radius(dropdown, 6, 0);
    lv_obj_set_style_text_color(dropdown, lv_color_hex(UI_COLOR_TEXT), 0);
    lv_obj_set_style_border_color(dropdown, lv_color_hex(UI_COLOR_FOCUS),
                                  selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_color(dropdown, lv_color_hex(UI_COLOR_FOCUS),
                                   selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_width(dropdown, 2, selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_opa(dropdown, 150, selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_transition(dropdown, &field_transition, 0);

    lv_obj_t *list = lv_dropdown_get_list(dropdown);
    if(list) {
        lv_obj_set_style_bg_color(list, lv_color_hex(UI_COLOR_SURFACE_TOP), 0);
        lv_obj_set_style_bg_grad_color(list, lv_color_hex(UI_COLOR_SURFACE_BOTTOM), 0);
        lv_obj_set_style_bg_grad_dir(list, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_bg_opa(list, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(list, lv_color_hex(UI_COLOR_FOCUS), 0);
        lv_obj_set_style_border_width(list, 1, 0);
        lv_obj_set_style_radius(list, 6, 0);
        lv_obj_set_style_text_color(list, lv_color_hex(UI_COLOR_TEXT), 0);
        lv_obj_set_style_bg_color(list, lv_color_hex(UI_COLOR_BLUE_BOTTOM), LV_PART_SELECTED);
        lv_obj_set_style_text_color(list, lv_color_hex(UI_COLOR_TEXT), LV_PART_SELECTED);
    }
}

static void style_switch(lv_obj_t *sw, uint32_t checked_color)
{
    if(!sw) return;
    lv_obj_set_style_bg_color(sw, lv_color_hex(UI_COLOR_NEUTRAL_BOTTOM), 0);
    lv_obj_set_style_bg_grad_color(sw, lv_color_hex(UI_COLOR_NEUTRAL_TOP), 0);
    lv_obj_set_style_bg_grad_dir(sw, LV_GRAD_DIR_HOR, 0);
    lv_obj_set_style_border_color(sw, lv_color_hex(UI_COLOR_NEUTRAL_BORDER), 0);
    lv_obj_set_style_border_width(sw, 1, 0);
    lv_obj_set_style_bg_color(sw, lv_color_hex(checked_color),
                              selector(LV_PART_MAIN, LV_STATE_CHECKED));
    lv_obj_set_style_bg_grad_color(sw, lv_color_hex(UI_COLOR_CANVAS_BOTTOM),
                                   selector(LV_PART_MAIN, LV_STATE_CHECKED));
    lv_obj_set_style_bg_grad_dir(sw, LV_GRAD_DIR_HOR,
                                 selector(LV_PART_MAIN, LV_STATE_CHECKED));
    lv_obj_set_style_bg_color(sw, lv_color_hex(UI_COLOR_TEXT), LV_PART_KNOB);
    lv_obj_set_style_border_color(sw, lv_color_hex(UI_COLOR_FOCUS),
                                  selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_color(sw, lv_color_hex(UI_COLOR_FOCUS),
                                   selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_width(sw, 2, selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_opa(sw, 150, selector(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_anim_duration(sw, 120, 0);
}

static void splash_set_opa(void *object, int32_t value)
{
    lv_obj_set_style_opa((lv_obj_t *)object, (lv_opa_t)value, 0);
}

static void start_splash_animation(void)
{
    if(!ui_ImageSplashLogo) return;
    lv_obj_set_style_opa(ui_ImageSplashLogo, LV_OPA_TRANSP, 0);

    lv_anim_t animation;
    lv_anim_init(&animation);
    lv_anim_set_var(&animation, ui_ImageSplashLogo);
    lv_anim_set_exec_cb(&animation, splash_set_opa);
    lv_anim_set_values(&animation, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&animation, 280);
    lv_anim_set_delay(&animation, 40);
    lv_anim_set_path_cb(&animation, lv_anim_path_ease_out);
    lv_anim_start(&animation);
}

void rolodeck_ui_set_button_role(lv_obj_t *button, rolodeck_ui_role_t role)
{
    if(!button) return;
    button_palette_t palette = button_palette(role);
    lv_style_selector_t pressed = selector(LV_PART_MAIN, LV_STATE_PRESSED);
    lv_style_selector_t focused = selector(LV_PART_MAIN, LV_STATE_FOCUSED);
    lv_style_selector_t disabled = selector(LV_PART_MAIN, LV_STATE_DISABLED);

    lv_obj_set_style_bg_color(button, lv_color_hex(palette.top), 0);
    lv_obj_set_style_bg_grad_color(button, lv_color_hex(palette.bottom), 0);
    lv_obj_set_style_bg_grad_dir(button, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(button, lv_color_hex(palette.border), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_radius(button, 8, 0);
    lv_obj_set_style_shadow_color(button, lv_color_hex(palette.top), 0);
    lv_obj_set_style_shadow_width(button, role == ROLODECK_UI_ROLE_NAVIGATION ? 4 : 8, 0);
    lv_obj_set_style_shadow_spread(button, 0, 0);
    lv_obj_set_style_shadow_opa(button, role == ROLODECK_UI_ROLE_NAVIGATION ? 35 : 70, 0);

    lv_obj_set_style_bg_color(button, lv_color_hex(palette.pressed_top), pressed);
    lv_obj_set_style_bg_grad_color(button, lv_color_hex(palette.pressed_bottom), pressed);
    lv_obj_set_style_bg_grad_dir(button, LV_GRAD_DIR_VER, pressed);
    lv_obj_set_style_border_color(button, lv_color_hex(palette.border), pressed);
    lv_obj_set_style_shadow_opa(button, 15, pressed);

    lv_obj_set_style_outline_color(button, lv_color_hex(UI_COLOR_FOCUS), focused);
    lv_obj_set_style_outline_width(button, 2, focused);
    lv_obj_set_style_outline_pad(button, 1, focused);
    lv_obj_set_style_outline_opa(button, 180, focused);
    lv_obj_set_style_border_color(button, lv_color_hex(UI_COLOR_FOCUS), focused);

    lv_obj_set_style_bg_color(button, lv_color_hex(0x2B3440), disabled);
    lv_obj_set_style_bg_grad_color(button, lv_color_hex(0x171D25), disabled);
    lv_obj_set_style_bg_grad_dir(button, LV_GRAD_DIR_VER, disabled);
    lv_obj_set_style_border_color(button, lv_color_hex(0x46505C), disabled);
    lv_obj_set_style_shadow_opa(button, LV_OPA_TRANSP, disabled);
    lv_obj_set_style_outline_opa(button, LV_OPA_TRANSP, disabled);
    lv_obj_set_style_opa(button, 205, disabled);

    lv_obj_set_style_transition(button, &button_transition, 0);
}

void rolodeck_ui_set_validation(lv_obj_t *label, lv_obj_t *field, bool invalid)
{
    uint32_t normal_color = label == ui_LabelRESPFreetext ? UI_COLOR_AMBER_TOP : UI_COLOR_TEXT;
    uint32_t color = invalid ? UI_COLOR_VALIDATION : normal_color;
    if(label) lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    if(field) {
        lv_obj_set_style_border_color(field,
            lv_color_hex(invalid ? UI_COLOR_VALIDATION : UI_COLOR_SURFACE_BORDER), 0);
        lv_obj_set_style_shadow_color(field, lv_color_hex(UI_COLOR_VALIDATION), 0);
        lv_obj_set_style_shadow_width(field, invalid ? 5 : 0, 0);
        lv_obj_set_style_shadow_opa(field, invalid ? 70 : LV_OPA_TRANSP, 0);
    }
}

void rolodeck_ui_theme_apply(void)
{
    if(theme_initialized) return;
    theme_initialized = true;

    lv_style_transition_dsc_init(&button_transition, button_transition_props,
                                 lv_anim_path_ease_out, 120, 0, NULL);
    lv_style_transition_dsc_init(&field_transition, field_transition_props,
                                 lv_anim_path_ease_out, 120, 0, NULL);

    lv_obj_t *screens[] = {
        ui_ScreenOANonCrisis, ui_ScreenOACrisis, ui_ScreenEMER, ui_ScreenCIRE,
        ui_ScreenMaintenance, ui_ScreenRESP, ui_ScreenProvisioning, ui_ScreenEMER2,
        ui_ScreenEULA
    };
    for(size_t i = 0; i < sizeof(screens) / sizeof(screens[0]); i++) style_screen(screens[i]);

    rolodeck_ui_set_button_role(ui_ButtonEMER, ROLODECK_UI_ROLE_EMERGENCY);
    rolodeck_ui_set_button_role(ui_ButtonEMERSubmit, ROLODECK_UI_ROLE_EMERGENCY);
    rolodeck_ui_set_button_role(ui_ButtonEMERSubmit1, ROLODECK_UI_ROLE_EMERGENCY);
    rolodeck_ui_set_button_role(ui_ButtonCIRE, ROLODECK_UI_ROLE_PRIMARY);
    rolodeck_ui_set_button_role(ui_ButtonCIREsubmit, ROLODECK_UI_ROLE_PRIMARY);
    rolodeck_ui_set_button_role(ui_EULAokButton, ROLODECK_UI_ROLE_PRIMARY);
    rolodeck_ui_set_button_role(ui_ButtonRESPsubmit, ROLODECK_UI_ROLE_RESPONSE);

    lv_obj_t *neutral_buttons[] = {
        ui_ButtonEMERAbort, ui_ButtonEMERAbort1, ui_ButtonCIREabort, ui_ButtonRESPabort
    };
    for(size_t i = 0; i < sizeof(neutral_buttons) / sizeof(neutral_buttons[0]); i++)
        rolodeck_ui_set_button_role(neutral_buttons[i], ROLODECK_UI_ROLE_NEUTRAL);

    lv_obj_t *nav_buttons[] = {
        ui_ButtonOACNav1, ui_ButtonOACNav2, ui_ButtonOACNav3, ui_ButtonOACNav4,
        ui_ButtonOANCNav1, ui_ButtonOANCNav2, ui_ButtonOANCNav3, ui_ButtonOANCNav4
    };
    for(size_t i = 0; i < sizeof(nav_buttons) / sizeof(nav_buttons[0]); i++)
        rolodeck_ui_set_button_role(nav_buttons[i], ROLODECK_UI_ROLE_NAVIGATION);

    lv_obj_t *button_labels[] = {
        ui_LabelButtonEMER, ui_LabelButtonCIRE,
        ui_LabelEMERButtonSubmit, ui_LabelEMERButtonAbort,
        ui_LabelEMERButtonSubmit1, ui_LabelEMERButtonAbort1,
        ui_LabelButtonCIREsubmit, ui_LabelButtonCIREabort,
        ui_LabelButtonRESPsubmit, ui_LabelButtonRESPabort, ui_Label5
    };
    for(size_t i = 0; i < sizeof(button_labels) / sizeof(button_labels[0]); i++)
        style_label(button_labels[i], UI_COLOR_TEXT);

    lv_obj_t *nav_arrow_labels[] = {
        ui_LabelOACNav1, ui_LabelOACNav2, ui_LabelOACNav3, ui_LabelOACNav4,
        ui_LabelOANCNav1, ui_LabelOANCNav2, ui_LabelOANCNav3, ui_LabelOANCNav4
    };
    for(size_t i = 0; i < sizeof(nav_arrow_labels) / sizeof(nav_arrow_labels[0]); i++) {
        style_label(nav_arrow_labels[i], UI_COLOR_TEXT);
        lv_obj_add_event_cb(nav_buttons[i], nav_button_state_changed,
                            LV_EVENT_STATE_CHANGED, nav_arrow_labels[i]);
    }
    style_label(ui_OACTextNum, UI_COLOR_TEXT);
    style_label(ui_OANCTextNum, UI_COLOR_TEXT);

    style_label(ui_LabelOACrisis, UI_COLOR_RED_TOP);
    style_label(ui_LabelEMERHeader, UI_COLOR_RED_TOP);
    style_label(ui_LabelEMERHeader1, UI_COLOR_RED_TOP);
    style_label(ui_Label2, UI_COLOR_RED_TOP);
    style_label(ui_LabelOANonCrisis, UI_COLOR_GREEN);
    style_label(ui_LabelCIREHeader, UI_COLOR_BLUE_TOP);
    style_label(ui_LabelMaintenanceTop, UI_COLOR_BLUE_TOP);
    style_label(ui_LabelProvisioningTop, UI_COLOR_BLUE_TOP);
    style_label(ui_LabelEULA1, UI_COLOR_BLUE_TOP);
    style_label(ui_LabelRESPHeader1, UI_COLOR_AMBER_TOP);
    style_label(ui_LabelRESPFreetext, UI_COLOR_AMBER_TOP);

    style_read_only_field(ui_TextAreaOACrisis);
    style_read_only_field(ui_TextAreaOANonCrisis);
    style_read_only_field(ui_TextAreaRESPoa);
    style_field(ui_TextAreaEMERWho);
    style_field(ui_TextAreaEMERWhere);
    style_field(ui_TextAreaEMERWhat);
    style_field(ui_TextAreaCIREFreetext);
    style_field(ui_TextAreaCIREWho);
    style_field(ui_TextAreaRESPFreetext);

    style_dropdown(ui_DropdownCIRECategory);
    style_dropdown(ui_Dropdown2);
    style_switch(ui_CIRE2SwitchLebensbedrohlich, UI_COLOR_RED_TOP);
    style_switch(ui_CIRE2SwitchErsthelfer, UI_COLOR_GREEN);

    if(ui_ScreenSplash) style_screen(ui_ScreenSplash);
    if(ui_PanelSplashBackground) {
        lv_obj_set_style_bg_color(ui_PanelSplashBackground, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_grad_color(ui_PanelSplashBackground, lv_color_hex(0xDCEAF8), 0);
        lv_obj_set_style_bg_grad_dir(ui_PanelSplashBackground, LV_GRAD_DIR_VER, 0);
        lv_obj_set_style_bg_opa(ui_PanelSplashBackground, LV_OPA_COVER, 0);
    }
    start_splash_animation();
}

lv_scr_load_anim_t rolodeck_ui_transition_for(uint8_t from_screen, uint8_t to_screen,
                                               uint32_t *duration_ms)
{
    if(duration_ms) *duration_ms = 0;
    if(from_screen == to_screen || from_screen == SCREEN_SPLASH || to_screen == SCREEN_SPLASH ||
       to_screen == SCREEN_MAINTENANCE || to_screen == SCREEN_PROVISIONING) {
        return LV_SCR_LOAD_ANIM_NONE;
    }

    if(duration_ms) *duration_ms = 140;

    if((from_screen == SCREEN_OACRISIS &&
        (to_screen == SCREEN_EMERGENCY || to_screen == SCREEN_CIRE || to_screen == SCREEN_RESP)) ||
       (from_screen == SCREEN_EMERGENCY && to_screen == SCREEN_EMERSTEPTWO)) {
        return LV_SCR_LOAD_ANIM_MOVE_LEFT;
    }

    if(((from_screen == SCREEN_EMERGENCY || from_screen == SCREEN_CIRE || from_screen == SCREEN_RESP) &&
        to_screen == SCREEN_OACRISIS) ||
       (from_screen == SCREEN_EMERSTEPTWO && to_screen == SCREEN_EMERGENCY)) {
        return LV_SCR_LOAD_ANIM_MOVE_RIGHT;
    }

    return LV_SCR_LOAD_ANIM_FADE_IN;
}

#else /* LVGL_VERSION_MAJOR < 9 */

static uint32_t legacy_role_color(rolodeck_ui_role_t role)
{
    switch(role) {
        case ROLODECK_UI_ROLE_EMERGENCY: return 0xE04646;
        case ROLODECK_UI_ROLE_PRIMARY: return 0x036FFC;
        case ROLODECK_UI_ROLE_RESPONSE: return 0xCE9600;
        case ROLODECK_UI_ROLE_NAVIGATION: return 0xFFFFFF;
        case ROLODECK_UI_ROLE_NEUTRAL:
        default: return 0x625D5A;
    }
}

void rolodeck_ui_theme_apply(void) { }

void rolodeck_ui_set_button_role(lv_obj_t *button, rolodeck_ui_role_t role)
{
    if(button) lv_obj_set_style_bg_color(button, lv_color_hex(legacy_role_color(role)), 0);
}

void rolodeck_ui_set_validation(lv_obj_t *label, lv_obj_t *field, bool invalid)
{
    if(label) lv_obj_set_style_text_color(label,
        lv_color_hex(invalid ? 0xFFFF00 : 0xFFFFFF), 0);
    (void)field;
}

lv_scr_load_anim_t rolodeck_ui_transition_for(uint8_t from_screen, uint8_t to_screen,
                                               uint32_t *duration_ms)
{
    (void)from_screen;
    (void)to_screen;
    if(duration_ms) *duration_ms = 0;
    return LV_SCR_LOAD_ANIM_NONE;
}

#endif /* LVGL_VERSION_MAJOR */
