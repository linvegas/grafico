#include "grafico.h"

int main()
{
    init_window(800, 600, "Grafico");

    Texture font_tex = load_texture_from_font("./resources/DroidSansMNerdFontMono-Regular.otf");
    Texture img_tex = load_texture_from_image("resources/0_0.jpg");

    while (window_is_open()) {
        vec2 win_size = get_fb_size();

        clear_background();

        draw_texture_vec(img_tex, vec2(win_size.x/2-img_tex.width/2*0.35, win_size.y/2-img_tex.height*0.35), 0.35, color(255,255,255));

        draw_rectangle_vec(vec2(win_size.x/2-220, win_size.y/2+24), vec2(440, 70), color(67,67,67), 0);
        draw_text_vec(font_tex, "Hello from Grafico", vec2(win_size.x/2 - 215, win_size.y/2 + 30), 1.0, color(155,255,170));

        render_draw();
    }

    return 0;
}
