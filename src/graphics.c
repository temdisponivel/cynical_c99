//
// Created by Firehorse on 22/05/2018.
//

#include <mem.h>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#undef STB_IMAGE_IMPLEMENTATION

#include "graphics.h"
#include "resources.h"
#include "hot_reloader.h"
#include "file.h"
#include "resource_manager.h"
#include "memory.h"

// TODO(temdisponivel): Make a struct that represents our gl state: our matrices, our current shader and so on
static mat4_t view_proj = {};

// TODO(temdisponivel): Remove me
extern mesh_t quad;

void reload_shader_sources(
        uint handle,
        const char *shader_file,
        char *both_include_file_path,
        char *vert_include_file_path,
        char *frag_include_file_path
) {
    CREATE_TEMP_NAMED_STR_BUFFER(vertex_buffer);
    CLEAR_TEMP_NAMED_STR_BUFFER(vertex_buffer);

    CREATE_TEMP_NAMED_STR_BUFFER(fragment_buffer);
    CLEAR_TEMP_NAMED_STR_BUFFER(fragment_buffer);

    bool read_shader = read_shader_file(
            shader_file,
            vertex_buffer,
            fragment_buffer,
            both_include_file_path,
            vert_include_file_path,
            frag_include_file_path
    );

    ASSERT(read_shader);

    update_shader_program(handle, vertex_buffer, fragment_buffer);
}

void update_shader_program(uint program, const char *vertex, const char *fragment) {
    {
        // TODO(temdisponivel): Should we cache this shader IDs in order to detach them later
        // instead of asking for the program for its shaders?!

        uint shaders[2];
        int count;
        glGetAttachedShaders(program, 2, &count, shaders);

        if (count > 0) {
            glDetachShader(program, shaders[0]);
            glDetachShader(program, shaders[1]);
        }
    }

    uint vert_handle = glCreateShader(GL_VERTEX_SHADER);
    uint frag_handle = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vert_handle, 1, &vertex, null);
    glCompileShader(vert_handle);

    CHECK_SHADER_COMPILATION(vert_handle);

    glShaderSource(frag_handle, 1, &fragment, null);
    glCompileShader(frag_handle);

    CHECK_SHADER_COMPILATION(frag_handle);

    glAttachShader(program, vert_handle);
    glAttachShader(program, frag_handle);

    glBindAttribLocation(program, VERT_POS_INDEX, VERT_POS_NAME);
    glBindAttribLocation(program, VERT_UV_INDEX, VERT_UV_NAME);
    glBindAttribLocation(program, VERT_COLOR_INDEX, VERT_COLOR_NAME);

    glLinkProgram(program);

    CHECK_PROGRAM_LINKAGE(program);

    glDeleteShader(vert_handle);
    glDeleteShader(frag_handle);

    CHECK_GL_ERROR();
}

void destroy_shader(const shader_t *shader) {
    stop_watch_shader_file(shader->handle);
    glDeleteProgram(shader->handle);
}

model_t create_quad() {
    const int QUAD_VERT_COUNT = 4;
    const int QUAD_INDEX_COUNT = 6;

    float *vertices = heap_alloc(FULL_MODEL_BYTE_SIZE(QUAD_VERT_COUNT));

    vec3_t *positions = (vec3_t *) vertices;
    vec2_t *uvs = (vec2_t *) (((void *) positions) + UV_BYTE_OFFSET(4));
    vec4_t *colors = (vec4_t *) (((void *) positions) + COLOR_BYTE_OFFSET(4));

    const float SIZE = 1.f;

    vec3_set(-SIZE, -SIZE, 0, &positions[0]);
    VEC2_SET_ZERO(&uvs[0]);
    COLOR_SET_RED(&colors[0]);

    vec3_set(SIZE, -SIZE, 0, &positions[1]);
    VEC2_SET_RIGH(&uvs[1]);
    COLOR_SET_GREEN(&colors[1]);

    vec3_set(SIZE, SIZE, 0, &positions[2]);
    VEC2_SET_ONE(&uvs[2]);
    COLOR_SET_BLUE(&colors[2]);

    vec3_set(-SIZE, SIZE, 0, &positions[3]);
    VEC2_SET_UP(&uvs[3]);
    COLOR_SET_YELLOW(&colors[3]);

    uint *index = heap_alloc(sizeof(uint) * QUAD_INDEX_COUNT);
    index[0] = 0;
    index[1] = 1;
    index[2] = 2;
    index[3] = 0;
    index[4] = 3;
    index[5] = 2;

    model_t model;
    model.full_vertices_data = vertices;
    model.positions_ptr_offset = positions;
    model.uvs_ptr_offset = uvs;
    model.colors_ptr_offset = colors;
    model.vertices_count = QUAD_VERT_COUNT;
    model.indexes = index;
    model.indexes_count = QUAD_INDEX_COUNT;
    return model;
}

void destroy_model(const model_t *model) {
    heap_free(model->full_vertices_data);
    if (model->indexes) {
        heap_free(model->indexes);
    }
}

#ifdef DEV

void print_model(model_t model) {

    for (int i = 0; i < model.vertices_count; ++i) {
        vec3_t pos = model.positions_ptr_offset[i];
        vec2_t uv = model.uvs_ptr_offset[i];
        vec4_t color = model.colors_ptr_offset[i];

        printf(
                "x: %f y: %f z: %f - r: %f g: %f b: %f a: %f - u: %f v: %f \n",
                pos.x, pos.y, pos.z,
                color.r, color.g, color.b, color.a,
                uv.x, uv.y
        );
    }
}

#endif

mesh_t create_mesh(const model_t *model) {
    uint vao;
    uint vbo;
    uint vio = 0;

    const int full_model_size = FULL_MODEL_BYTE_SIZE(model->vertices_count);
    const int pos_offset = POS_BYTE_OFFSET(model.vertices_count);
    const int uv_offset = UV_BYTE_OFFSET(model->vertices_count);
    const int color_offset = COLOR_BYTE_OFFSET(model->vertices_count);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
            GL_ARRAY_BUFFER,
            full_model_size,
            model->full_vertices_data,
            GL_STATIC_DRAW
    );
    CHECK_GL_ERROR();

    glVertexAttribPointer(
            VERT_POS_INDEX,
            POS_DIMENTION,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *) pos_offset
    );

    glVertexAttribPointer(
            VERT_UV_INDEX,
            UV_DIMENTION,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *) uv_offset
    );

    glVertexAttribPointer(
            VERT_COLOR_INDEX,
            COLOR_DIMENTION,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void *) color_offset
    );
    CHECK_GL_ERROR();

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (model->indexes_count) {
        glGenBuffers(1, &vio);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vio);

        glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                sizeof(uint) * model->indexes_count,
                model->indexes,
                GL_STATIC_DRAW
        );

        CHECK_GL_ERROR();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    mesh_t mesh;
    mesh.vao = vao;
    mesh.vbo = vbo;
    mesh.vio = vio;

    if (vio) {
        mesh.elements_len = model->indexes_count;
    } else {
        mesh.elements_len = model->vertices_count;
    }

    return mesh;
}

void destroy_mesh(const mesh_t *mesh) {

    glDeleteBuffers(1, &mesh->vbo);

    if (mesh->vio) {
        glDeleteBuffers(1, &mesh->vio);
    }

    glDeleteVertexArrays(1, &mesh->vao);

    CHECK_GL_ERROR();

    // TODO(temdisponivel): should we set these handles to invalid values?!
}

void draw_mesh(const mesh_t *mesh, const material_t *material, const transform_t *trans) {
    mat4_t model;
    trans_get_mat4(trans, &model);

    mat4_t MVP;
    mat4_mul(&view_proj, &model, &MVP);

    set_mat4_uniform(material, MVP_UNI_NAME, &MVP);

    // TODO(temdisponivel): Validate if this material was previously used and do not use it again if not necessary
    use_material(material);

    glBindVertexArray(mesh->vao);
    glEnableVertexAttribArray(VERT_POS_INDEX);
    glEnableVertexAttribArray(VERT_UV_INDEX);
    glEnableVertexAttribArray(VERT_COLOR_INDEX);

    if (mesh->vio) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vio);

        glDrawElements(GL_TRIANGLE_FAN, mesh->elements_len, GL_UNSIGNED_INT, null);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    } else {
        glDrawArrays(GL_TRIANGLE_FAN, 0, mesh->elements_len);
    }

    CHECK_GL_ERROR();

    glDisableVertexAttribArray(VERT_COLOR_INDEX);
    glDisableVertexAttribArray(VERT_UV_INDEX);
    glDisableVertexAttribArray(VERT_POS_INDEX);
    glBindVertexArray(0);
}

void create_texture(const image_t *image, texture_t *dest) {
    uint handle;
    glGenTextures(1, &handle);

    update_texture_data(handle, image);

    watch_texture_file(handle, image->file_path);

    dest->handle = handle;
    dest->size = image->size;
    dest->texel_size = 1;
}

void reload_texture(uint handle, const char *file_path) {
    // Force the deletion of the loaded image so we can actually load the image from disk again
    free_unused_resources();

    image_t *img = get_image_resource(file_path);
    update_texture_data(handle, img);
    free_image_resource(img);
}

void update_texture_data(uint handle, const image_t *image) {
    glBindTexture(GL_TEXTURE_2D, handle);

    glTexImage2D(
            GL_TEXTURE_2D,
            0, // Mip map level
            GL_RGBA,
            (uint) image->size.x,
            (uint) image->size.y,
            0, // border
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            image->data
    );
    CHECK_GL_ERROR();


    // TODO: parameterize this
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    CHECK_GL_ERROR();

    glBindTexture(GL_TEXTURE_2D, 0);

    CHECK_GL_ERROR();
}

void destroy_texture(const texture_t *texture) {
    glDeleteTextures(1, &texture->handle);
    stop_watch_texture_file(texture->handle);
}

// TODO(temdisponivel): Make sure to not call this function from create_material_from_file
void reload_material(const char *file_path, material_t *dest) {
    free_shader_resource(dest->shader);

    for (int i = 0; i < dest->texture_uniforms_len; ++i) {
        free_texture_resource(dest->texture_uniforms[i].texture);
    }

    material_definition_t definition;
    bool read = read_material_definition_file(file_path, &definition);
    ASSERT(read);
    create_material(&definition, dest);
}

INLINE void cache_uniform_info(uniform_info_t *info, const char *name) {
    strcpy(info->name, name);
    info->hashed_name = hash_string(name);
}

void create_material(const material_definition_t *definition, material_t *dest) {

    // TODO(temdisponivel): Do not require the uniform to be define 

    shader_t *shader = get_shader_resource(definition->shader_file);

    dest->shader = shader;

    dest->float_uniforms_len = definition->floats_len;
    ASSERT(dest->float_uniforms_len < MAX_FLOATS);

    dest->mat4_uniforms_len = definition->mat4s_len;
    ASSERT(dest->mat4_uniforms_len < MAX_MAT4S);

    dest->texture_uniforms_len = definition->textures_len;
    ASSERT(dest->texture_uniforms_len < MAX_TEXTURES);

    dest->vec2_uniforms_len = definition->vec2s_len;
    ASSERT(dest->vec2_uniforms_len < MAX_VEC2S);

    dest->vec4_uniforms_len = definition->vec4s_len;
    ASSERT(dest->vec4_uniforms_len < MAX_VEC4S);

    // ============= FLOAT

    for (int i = 0; i < definition->floats_len; ++i) {
        float_uniform_definition_t float_def = definition->floats[i];

        float_uniform_t float_uni;
        cache_uniform_info(&float_uni.info, float_def.uniform_name);

        float_uni.value = float_def.default_value;

        dest->float_uniforms[i] = float_uni;
    }

    // ============= MAT 4

    for (int i = 0; i < definition->mat4s_len; ++i) {
        mat4_uniform_definition_t mat4_def = definition->mat4s[i];

        mat4_uniform_t mat4_uni;
        strcpy(mat4_uni.info.name, mat4_def.uniform_name);
        cache_uniform_info(&mat4_uni.info, mat4_def.uniform_name);

        mat4_uni.value = mat4_def.default_value;

        dest->mat4_uniforms[i] = mat4_uni;
    }

    // ============= TEXTURE

    for (int i = 0; i < definition->textures_len; ++i) {
        texture_uniform_definition_t tex_def = definition->textures[i];

        texture_uniform_t tex_uni;
        strcpy(tex_uni.info.name, tex_def.uniform_name);
        cache_uniform_info(&tex_uni.info, tex_def.uniform_name);

        tex_uni.texture_unit = TEXTURE_UNIT_0 + i;

        char *file_path;
        if (strlen(tex_def.image_file_name)) {
            file_path = tex_def.image_file_name;
        } else {
            file_path = DEFAULT_IMAGE_FILE_PATH;
        }

        tex_uni.texture = get_texture_resource(file_path);

        dest->texture_uniforms[i] = tex_uni;
    }

    // ============= VEC2

    for (int i = 0; i < definition->vec2s_len; ++i) {
        vec2_uniform_definition_t vec2_def = definition->vec2s[i];

        vec2_uniform_t vec2_uni;
        cache_uniform_info(&vec2_uni.info, vec2_def.uniform_name);

        vec2_uni.value = vec2_def.default_value;

        dest->vec2_uniforms[i] = vec2_uni;
    }

    // ============= VEC4

    for (int i = 0; i < definition->vec4s_len; ++i) {
        vec4_uniform_definition_t vec4_def = definition->vec4s[i];

        vec4_uniform_t vec4_uni;
        cache_uniform_info(&vec4_uni.info, vec4_def.uniform_name);

        vec4_uni.value = vec4_def.default_value;

        dest->vec4_uniforms[i] = vec4_uni;
    }
}

void destroy_material(const material_t *material) {
    free_shader_resource(material->shader);

    for (int i = 0; i < material->texture_uniforms_len; ++i) {
        texture_uniform_t texture_uni = material->texture_uniforms[i];
        free_texture_resource(texture_uni.texture);
    }

    stop_watch_material_definition_file(material);
}

void use_material(const material_t *material) {
    glUseProgram(material->shader->handle);
    CHECK_GL_ERROR();

    for (int i = 0; i < material->float_uniforms_len; ++i) {
        float_uniform_t uniform = material->float_uniforms[i];
        int loc = glGetUniformLocation(material->shader->handle, uniform.info.name);
        if (loc >= 0) {
            glUniform1f(loc, uniform.value);
        }
    }

    CHECK_GL_ERROR();

    for (int i = 0; i < material->mat4_uniforms_len; ++i) {
        mat4_uniform_t uniform = material->mat4_uniforms[i];
        int loc = glGetUniformLocation(material->shader->handle, uniform.info.name);
        if (loc >= 0) {
            glUniformMatrix4fv(loc, 1, GL_FALSE, &uniform.value);
        }
    }

    CHECK_GL_ERROR();

    for (int i = 0; i < material->texture_uniforms_len; ++i) {
        texture_uniform_t uniform = material->texture_uniforms[i];
        int loc = glGetUniformLocation(material->shader->handle, uniform.info.name);
        if (loc >= 0) {
            glActiveTexture(uniform.texture_unit);
            glBindTexture(GL_TEXTURE_2D, uniform.texture->handle);
            glUniform1i(loc, uniform.texture_unit - TEXTURE_UNIT_0);
        }
    }

    CHECK_GL_ERROR();

    for (int i = 0; i < material->vec2_uniforms_len; ++i) {
        vec2_uniform_t uniform = material->vec2_uniforms[i];
        int loc = glGetUniformLocation(material->shader->handle, uniform.info.name);
        if (loc >= 0) {
            glUniform2fv(loc, 1, &uniform.value.data);
        }
    }

    CHECK_GL_ERROR();

    for (int i = 0; i < material->vec4_uniforms_len; ++i) {
        vec4_uniform_t uniform = material->vec4_uniforms[i];
        int loc = glGetUniformLocation(material->shader->handle, uniform.info.name);
        if (loc >= 0) {
            glUniform4fv(loc, 1, &uniform.value.data);
        }
    }

    CHECK_GL_ERROR();
    
    // TODO(temdisponivel): Should I create a function that will unbind all these uniforms?!
}

void set_texture_uniform(const material_t *material, const char *uniform_name, const texture_t *texture) {

    bool found = false;
    int hashed_name = hash_string(uniform_name);
    for (int i = 0; i < material->texture_uniforms_len; ++i) {
        texture_uniform_t *uniform = &material->texture_uniforms[i];
        if (uniform->info.hashed_name == hashed_name) {
            uniform->texture = texture;
            found = true;
        }
    }

    ASSERT(found);
}

void set_float_uniform(const material_t *material, const char *uniform_name, float value) {
    bool found = false;
    int hashed_name = hash_string(uniform_name);
    for (int i = 0; i < material->float_uniforms_len; ++i) {
        float_uniform_t *uniform = &material->float_uniforms[i];
        if (uniform->info.hashed_name == hashed_name) {
            uniform->value = value;
            found = true;
        }
    }

    ASSERT(found);
}

void set_mat4_uniform(const material_t *material, const char *uniform_name, const mat4_t *value) {
    bool found = false;
    int hashed_name = hash_string(uniform_name);
    for (int i = 0; i < material->mat4_uniforms_len; ++i) {
        mat4_uniform_t *uniform = &material->mat4_uniforms[i];
        if (uniform->info.hashed_name == hashed_name) {
            mat4_cpy(value, &uniform->value);
            found = true;
        }
    }

    ASSERT(found);
}

void set_vec2_uniform(const material_t *material, const char *uniform_name, vec2_t value) {
    bool found = false;
    int hashed_name = hash_string(uniform_name);
    for (int i = 0; i < material->vec2_uniforms_len; ++i) {
        vec2_uniform_t *uniform = &material->vec2_uniforms[i];
        if (uniform->info.hashed_name == hashed_name) {
            uniform->value = value;
            found = true;
        }
    }

#if VERBOSE_DEV
    ASSERT(found);
#endif
}

void set_vec4_uniform(const material_t *material, const char *uniform_name, vec4_t value) {
    bool found = false;
    int hashed_name = hash_string(uniform_name);
    for (int i = 0; i < material->vec4_uniforms_len; ++i) {
        vec4_uniform_t *uniform = &material->vec4_uniforms[i];
        if (uniform->info.hashed_name == hashed_name) {
            uniform->value = value;
            found = true;
        }
    }

#if VERBOSE_DEV
    ASSERT(found);
#endif
}

void camera_set_defaults(camera_t *dest) {
    vec3_t pos = VEC3_MAKE_ZERO();
    vec3_t scale = VEC3_MAKE_ONE();
    quat_t rot = QUAT_MAKE_IDENTITY();
    trans_set(&pos, &scale, &rot, &dest->transform);
    dest->depth = -1;
    dest->clear_color = COLOR_MAKE_BLACK();
    dest->clear_depth_only = false;
    
    vec2_t bottom_left = vec2_make(-.5f, -.5f);
    vec2_t size = VEC2_MAKE_ONE();
    dest->view_port = rect_make(&bottom_left, &size);
}

void set_default_orthographic_projection(camera_t *camera) {
    // TODO(temdisponivel): Maybe we should make the 0 0 be the bottom_left
    float size = 1.f;
    mat4_ortho(
            -size * aspect_ratio,
            size * aspect_ratio,
            -size,
            size,
            -100,
            100,
            &camera->projection_matrix
    );
}

void create_camera_orthographic_default(camera_t *dest) {
    set_default_orthographic_projection(dest);
    camera_set_defaults(dest);
}

void create_camera_orthographic(float left, float right, float bottom, float top, float near_plane, float far_plane, camera_t *dest) {
    mat4_ortho(left, right, bottom, top, near_plane, far_plane, &dest->projection_matrix);
    camera_set_defaults(dest);
}

void create_camera_perspective_default(float aspect_ratio, camera_t *dest) {
    create_camera_perspective(PI / 4, aspect_ratio, .01f, 100, dest);
}

void create_camera_perspective(float fov, float ratio, float near_plane, float far_plane, camera_t *dest) {
    mat4_perspective(fov, ratio, near_plane, far_plane, &dest->projection_matrix);
    camera_set_defaults(dest);
}

void use_camera(camera_t *camera) {
    set_default_orthographic_projection(camera);
    
    // ========== VIEW PORT

    rect_t view_port = camera->view_port;
    
    rect_t real_view_port;
    real_view_port.bottom_left.x = view_port.bottom_left.x * screen_size.x;
    real_view_port.bottom_left.y = view_port.bottom_left.y * screen_size.y;
    
    real_view_port.size.width =  view_port.size.width  * screen_size.width;
    real_view_port.size.height = view_port.size.height * screen_size.height;

    // Transform to opengl coordinates
    real_view_port.bottom_left.x += screen_size.width / 2.f;
    real_view_port.bottom_left.y += screen_size.height / 2.f;
    
    int x, y, width, height;
    x = (int) real_view_port.bottom_left.x;
    y = (int) real_view_port.bottom_left.y;
    width = (int) real_view_port.size.width;
    height = (int) real_view_port.size.height;
    
    glViewport(x, y, width, height);
    glScissor(x, y, width, height);

    // =========== CLEAR
    
    uint clear_flags = GL_DEPTH_BUFFER_BIT;
    if (!camera->clear_depth_only) {
        clear_flags |= GL_COLOR_BUFFER_BIT;
    }

    glClearColor(camera->clear_color.r, camera->clear_color.g, camera->clear_color.b, camera->clear_color.a);
    glClearDepth(camera->depth);
    glClear(clear_flags);

    // ========== MATRIX
    
    vec3_t camera_dir;
    trans_get_forward(&camera->transform, &camera_dir);

    vec3_t camera_up;
    trans_get_up(&camera->transform, &camera_up);

    vec3_t pos = camera->transform.position;

    float x_multiplier = 1 / screen_size.width;
    float y_multiplier = 1 / screen_size.height;

    pos.x *= x_multiplier;
    pos.y *= y_multiplier;

    mat4_t view;
    mat4_look(&pos, &camera_dir, &camera_up, &view);
    mat4_mul(&camera->projection_matrix, &view, &view_proj);
}

void create_texture_renderer(const texture_t *texture, const material_t *material, texture_renderer_t *dest) {
    dest->texture = texture;
    dest->material = material;
    vec2_scale(&texture->size, texture->texel_size, &dest->size);

    dest->pivot = VEC2_MAKE_ZERO(); // CENTER

    vec2_t pos = VEC2_MAKE_ZERO();
    vec2_t size = VEC2_MAKE_ONE();
    dest->texture_area = rect_make(&pos, &size);
}

void draw_texture_renderer(const texture_renderer_t *renderer, const transform_t *trans) {
    vec2_t tex_size = renderer->size;
    float width_multiplier = 1 / screen_size.width;
    float height_multiplier = 1 / screen_size.height;

    width_multiplier *= renderer->texture->texel_size;
    height_multiplier *= renderer->texture->texel_size;

    vec3_t final_size = vec3_make(
            tex_size.x * width_multiplier,
            tex_size.y * height_multiplier,
            1
    );

    transform_t helper;

    vec3_t pivot = vec3_make(renderer->pivot.x, renderer->pivot.y, 0);
    vec3_t pos;
    vec3_add(&trans->position, &pivot, &pos);

    pos.x *= aspect_ratio;
    trans_set(&pos, &trans->scale, &trans->rotation, &helper);
    vec3_scale_vec3(&helper.scale, &final_size, &helper.scale);

    rect_t area = renderer->texture_area;

    set_vec2_uniform(renderer->material, SPRITE_OFFSET_UNI_NAME, area.bottom_left);
    set_vec2_uniform(renderer->material, SPRITE_SIZE_UNI_NAME, area.size);

    set_texture_uniform(renderer->material, MAIN_TEX_UNI_NAME, renderer->texture);

    draw_mesh(&quad, renderer->material, &helper);
}

void set_blend_state(bool enabled) {
    if (enabled) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        glDisable(GL_BLEND);
    }
}