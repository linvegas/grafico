#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#define LITERAL(t) (t)

typedef struct vec2 { float x, y; } vec2;
typedef struct vec3 { float x, y, z; } vec3;

#define vec2(x,y)   LITERAL(vec2){(x),(y)}
#define vec3(x,y,z) LITERAL(vec3){(x),(y),(z)}

typedef struct Texture {
    GLuint id;
    int    width;
    int    height;
    int    channels;
} Texture;

typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Color;

#define color(x,y,z)   LITERAL(Color){(x),(y),(z)}

#define C_RED   LITERAL(Color){200, 126, 120}
#define C_BLUE  LITERAL(Color){124, 175, 194}
#define C_GREEN LITERAL(Color){179, 196, 138}

Texture load_texture_from_font(const char *file_path);
Texture load_texture_from_image(const char *file_path);
int init_window(int width, int height, const char *title);
int window_is_open(void);
void clear_background(void);
void draw_rectangle_vec(vec2 pos, vec2 size, Color color, GLuint id);
void draw_texture_vec(Texture tex, vec2 pos, float scale, Color color);
void draw_text_vec(Texture tex, const char *text, vec2 pos, float scale, Color color);
void render_draw(void);
vec2 get_fb_size(void);
GLuint compile_shaders(void);

// vim:ft=c
