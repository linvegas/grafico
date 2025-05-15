#include <errno.h>

#include "grafico.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define max(x, y) ((x) > (y) ? (x) : (y))

typedef struct mat4{
    float m[4][4];
} mat4;

mat4 mat4_ortho(int width, int height)
{
    mat4 mat = {0};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j) {
                mat.m[i][j] = 1.0f;
            } else {
                mat.m[i][j] = 0.0f;
            }
        }
    }

    float left = 0.0f;
    float right = (float)width;
    float bottom = (float)height;
    float top = 0.0f;
    float near = -1.0f;
    float far = 1.0f;

    mat.m[0][0] = 2.0f / (right - left);
    mat.m[1][1] = 2.0f / (top - bottom);
    mat.m[2][2] = 2.0f / (far - near);

    mat.m[3][0] = -(right + left) / (right - left);
    mat.m[3][1] = -(top + bottom) / (top - bottom);
    mat.m[3][2] = -(far + near) / (far - near);

    return mat;
}

State state = {0};

typedef struct Glyph {
    vec2 advance; // advance.x advance.y
    vec2 size;    // bitmap.width bitmap.rows
    vec2 bearing; // bitmap_left bitmap_top;
    float offset; // x offset of glyph in texture coordinates
} Glyph;

Glyph glyphs[128] = {0};

Texture load_texture_from_font(const char *file_path)
{
    Texture t = {0};

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr,"ERROR: Could not init FreeType Library\n");
    }

    FT_Face face;
    if (FT_New_Face(ft, file_path, 0, &face)) {
        fprintf(stderr,"ERROR: Failed to load font from file\n");
    }

    FT_Set_Pixel_Sizes(face, 0, 40);

    int atlas_width = 0;
    int atlas_height = 0;

    for (int c = 32; c < 128; c++) {
        if (FT_Load_Char(face, (char)c, FT_LOAD_RENDER)) {
            fprintf(stderr, "ERROR: Failed to load '%c' glyph\n", c);
            continue;
        }

        // printf("%3d:%c -> width: %-3d rows: %-3d left: %-3d top: %-3d advance: %ld\n",
        //        c, c, face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap_left,
        //        face->glyph->bitmap_top, face->glyph->advance.x);

        atlas_width += face->glyph->bitmap.width;
        atlas_height = max(atlas_height, (int)face->glyph->bitmap.rows);
    }

    // printf("atlas width:  %d\n", atlas_width);
    // printf("atlas height: %d\n", atlas_height);

    t.width = atlas_width;
    t.height = atlas_height;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &t.id);
    glBindTexture(GL_TEXTURE_2D, t.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, t.width, t.height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

    int x = 0;

    for (int c = 32; c < 128; c++) {
        if (FT_Load_Char(face, (char)c, FT_LOAD_RENDER)) {
            fprintf(stderr, "ERROR: Failed to load '%c' glyph\n", c);
            continue;
        }

        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            x,
            0,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glyphs[c].advance = vec2(face->glyph->advance.x >> 6, face->glyph->advance.y >> 6);
        glyphs[c].size = vec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
        glyphs[c].bearing = vec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        glyphs[c].offset = (float)x / t.width;

        x += face->glyph->bitmap.width;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return t;
}

Texture load_texture_from_image(const char *file_path)
{
    Texture t = {0};

    glGenTextures(1, &t.id);
    glBindTexture(GL_TEXTURE_2D, t.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(1);

    unsigned char *data = stbi_load(file_path, &t.width, &t.height, &t.channels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t.width, t.height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    } else {
        fprintf(stderr, "ERROR Couldn't load image data\n");
    }

    return t;
}

void error_callback(int error, const char *description)
{
    fprintf(stderr, "ERROR %d: %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    (void)window;
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void)window; (void)mods; (void)scancode; (void)key; (void)action;
    // if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
    //     state.text[state.text_len++] = '\n';
    // if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
    //     state.text[--state.text_len] = 0;
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
    (void)window; (void)codepoint;
    // printf("%c\n", codepoint);
}

void vertex_push(vec2 pos, vec2 uv, vec3 color, float tex_id)
{
    state.renderer.vertices.data[state.renderer.vertices.len].pos    = pos;
    state.renderer.vertices.data[state.renderer.vertices.len].uv     = uv;
    state.renderer.vertices.data[state.renderer.vertices.len].color  = color;
    state.renderer.vertices.data[state.renderer.vertices.len].tex_id = tex_id;
    state.renderer.vertices.len += 1;
}

void draw_rectangle_vec(vec2 pos, vec2 size, Color color, GLuint id)
{
    float rect_x = pos.x;
    float rect_y = pos.y;
    float rect_w = pos.x + size.x;
    float rect_h = pos.y + size.y;

    vec3 col = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };

    vertex_push(vec2(rect_x, rect_h), vec2(0, 0), col, id); // BOTTOM LEFT
    vertex_push(vec2(rect_x, rect_y), vec2(0, 1), col, id); // TOP LEFT
    vertex_push(vec2(rect_w, rect_h), vec2(1, 0), col, id); // BOTTOM RIGHT
    vertex_push(vec2(rect_w, rect_y), vec2(1, 1), col, id); // TOP RIGHT

    unsigned int i = state.renderer.indices.len > 0 ? state.renderer.indices.data[state.renderer.indices.len-1]+1 : 0;

    state.renderer.indices.data[state.renderer.indices.len]     = i;
    state.renderer.indices.data[state.renderer.indices.len + 1] = i + 1;
    state.renderer.indices.data[state.renderer.indices.len + 2] = i + 2;
    state.renderer.indices.data[state.renderer.indices.len + 3] = i + 1;
    state.renderer.indices.data[state.renderer.indices.len + 4] = i + 2;
    state.renderer.indices.data[state.renderer.indices.len + 5] = i + 3;

    state.renderer.indices.len += 6;
}

void draw_text_vec(Texture tex, const char *text, vec2 pos, float scale, Color color)
{
    float init_posx = pos.x;

    for (const char *c = text; *c; c++) {
        Glyph g = glyphs[(int)*c];

        float rect_x = pos.x + g.bearing.x * scale;
        float rect_y = pos.y + (tex.height - g.bearing.y) * scale;
        float rect_w = g.size.x * scale;
        float rect_h = g.size.y * scale;

        vec3 col = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };

        vertex_push((vec2){ rect_x, rect_y + rect_h }, vec2(g.offset, g.size.y / tex.height), col, tex.id); // BOTTOM LEFT
        vertex_push((vec2){ rect_x, rect_y }, vec2(g.offset, 0), col, tex.id); // TOP LEFT
        vertex_push((vec2){ rect_x + rect_w, rect_y + rect_h }, vec2(g.offset + g.size.x / tex.width, g.size.y / tex.height), col, tex.id); // BOTTOM RIGHT
        vertex_push((vec2){ rect_x + rect_w, rect_y }, vec2(g.offset + g.size.x / tex.width, 0), col, tex.id); // TOP RIGHT

        unsigned int i = state.renderer.indices.len > 0 ? state.renderer.indices.data[state.renderer.indices.len-1]+1 : 0;

        state.renderer.indices.data[state.renderer.indices.len]     = i;
        state.renderer.indices.data[state.renderer.indices.len + 1] = i + 1;
        state.renderer.indices.data[state.renderer.indices.len + 2] = i + 2;
        state.renderer.indices.data[state.renderer.indices.len + 3] = i + 1;
        state.renderer.indices.data[state.renderer.indices.len + 4] = i + 2;
        state.renderer.indices.data[state.renderer.indices.len + 5] = i + 3;

        state.renderer.indices.len += 6;

        pos.x += g.advance.x * scale;
        if (*c == '\n') pos.y += tex.height, pos.x = init_posx;
    }
}

void draw_texture_vec(Texture tex, vec2 pos, float scale, Color color)
{
    vec2 size = { (float)tex.width*scale, (float)tex.height*scale };

    draw_rectangle_vec(pos, size, color, tex.id);
}

vec2 get_fb_size(void)
{
    int w, h;
    glfwGetFramebufferSize(state.window, &w, &h);
    return vec2(w,h);
}

void renderer_render(Renderer *r)
{
    vec2 win_size = get_fb_size();

    mat4 projection = mat4_ortho((int)win_size.x, (int)win_size.y);
    glUniformMatrix4fv(glGetUniformLocation(r->shader, "projection"), 1, GL_FALSE, (void *)&projection);
    glUseProgram(r->shader);

    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * r->vertices.len, r->vertices.data);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * r->indices.len, r->indices.data);

    for (int i = 0; i < GL_MAX_TEX_UNITS; i++) {
        glBindTextureUnit(i, i);
    }

    glDrawElements(GL_TRIANGLES, r->indices.len, GL_UNSIGNED_INT, (GLvoid *)0);

    r->indices.len = 0;
    r->vertices.len = 0;
}

Renderer renderer_init(void) // TODO: Init textures
{
    Renderer r = {0};

    r.shader = shader_program();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenVertexArrays(1, &r.vao);
    glBindVertexArray(r.vao);

    glGenBuffers(1, &r.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*GL_ELEM_VERTICES_CAP, r.vertices.data, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &r.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*GL_ELEM_INDICES_CAP, r.indices.data, GL_DYNAMIC_DRAW);

    GLint vpos_loc = glGetAttribLocation(r.shader, "vertexPos");
    GLint vtuv_loc = glGetAttribLocation(r.shader, "vertexUv");
    GLint vcol_loc = glGetAttribLocation(r.shader, "vertexColor");
    GLint vtid_loc = glGetAttribLocation(r.shader, "vertexTexId");

    glEnableVertexAttribArray(vpos_loc);
    glVertexAttribPointer(vpos_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, pos));

    glEnableVertexAttribArray(vtuv_loc);
    glVertexAttribPointer(vtuv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, uv));

    glEnableVertexAttribArray(vcol_loc);
    glVertexAttribPointer(vcol_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));

    glEnableVertexAttribArray(vtid_loc);
    glVertexAttribPointer(vtid_loc, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, tex_id));

    int tex_slots[GL_MAX_TEX_UNITS] = {0};
    for (int i = 0; i < GL_MAX_TEX_UNITS; i++) tex_slots[i] = i;

    glUseProgram(r.shader);
    glUniform1iv(glGetUniformLocation(r.shader, "tex"), GL_MAX_TEX_UNITS, tex_slots);

    return r;
}

int init_window(int width, int height, const char *title)
{
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow *win = glfwCreateWindow(width, height, title, NULL, NULL);

    if (!win) return -1;

    state.window = win;

    glfwSetFramebufferSizeCallback(state.window, framebuffer_size_callback);
    // glfwSetKeyCallback(state.window, key_callback);
    // glfwSetCharCallback(state.window, character_callback);
    // glfwSetWindowRefreshCallback(state.window, window_refresh_callback);

    glfwMakeContextCurrent(state.window);

    int max_elem_vert = 0;
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &max_elem_vert);
    printf("[INFO] OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("[INFO] Max Elements Vertices: %d\n", max_elem_vert);

    state.renderer = renderer_init();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);

    return 0;
}

int window_is_open(void)
{
    return !glfwWindowShouldClose(state.window);
}

void clear_background(void)
{
    glClearColor(0.11f, 0.11f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void render(void)
{
    renderer_render(&state.renderer);

    glfwSwapBuffers(state.window);
    glfwWaitEvents(); // or glfwPollEvents()
}

char *str_from_file(const char *file_path)
{
    FILE *file = fopen(file_path, "r");

    if (!file) {
        fprintf(stderr, "ERROR: Opening file '%s': %s\n", file_path, strerror(errno));
        return NULL;
    }

    fseek(file, 0L, SEEK_END);
    long int file_len = ftell(file);

    rewind(file);

    char *buffer = calloc((size_t)file_len+1, sizeof(char));

    if (!buffer) {
        fprintf(stderr, "ERROR: Calloc buffer for '%s': %s\n", file_path, strerror(errno));
        return NULL;
    }

    size_t buffer_len = fread(buffer, 1, (size_t)file_len ,file);
    buffer[buffer_len] = '\0';

    fclose(file);

    return buffer;
}

GLuint create_shader(const char *filepath, int type)
{
    char *shader_src = str_from_file(filepath);

    if (!shader_src) exit(1);

    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, (const char **)&shader_src, NULL);
    glCompileShader(shader);

    free(shader_src);

    int success;
    char info_log[SHADER_INFO_LOG_CAP];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader, SHADER_INFO_LOG_CAP, NULL, info_log);
        fprintf(stderr, "[ERROR] Shader Compilation:\n%s", info_log);
    }

    return shader;
}

GLuint create_program(GLuint v_shader, GLuint f_shader)
{
    GLuint program = glCreateProgram();

    glAttachShader(program, v_shader);
    glAttachShader(program, f_shader);

    glLinkProgram(program);

    int success;
    char info_log[SHADER_INFO_LOG_CAP];

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        glGetProgramInfoLog(program, SHADER_INFO_LOG_CAP, NULL, info_log);
        fprintf(stderr, "[ERROR] Shader Linking:\n%s", info_log);
    }

    return program;
}

GLuint shader_program(void)
{
    GLuint vertex_shader = create_shader("shader.vert", GL_VERTEX_SHADER);
    GLuint fragment_shader = create_shader("shader.frag", GL_FRAGMENT_SHADER);

    GLuint shader_prg = create_program(vertex_shader, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_prg;
}
