//
// Created by temdisponivel on 25/05/2018.
//

INLINE transform_t trans_make(const vec3_t *pos, const vec3_t *scale, const quat_t *rotation) {
    transform_t dest;
    trans_set(pos, scale, rotation, &dest);
    return dest;
}

INLINE void trans_set(const vec3_t *pos, const vec3_t *scale, const quat_t *rotation, transform_t *dest) {
    vec3_cpy(pos, &dest->position);
    vec3_cpy(scale, &dest->scale);
    vec4_cpy(rotation, &dest->rotation);
}

INLINE void trans_identity(transform_t *dest) {
    vec3_t pos = VEC3_MAKE_ZERO();
    vec3_t scale = VEC3_MAKE_ONE();
    quat_t rot = QUAT_MAKE_IDENTITY();
    trans_set(&pos, &scale, &rot, dest);
}

INLINE void trans_get_mat4(const transform_t *trans, mat4_t *dest) {
    vec3_t pos = trans->position;
    
    float x_multiplier = 1 / screen_size.x;
    float y_multiplier = 1 / screen_size.y;
    
    pos.x *= x_multiplier;
    pos.y *= y_multiplier;    

    mat4_identity(dest);
    mat4_translate(dest, &pos, dest);
    quat_rotate_matrix(dest, &trans->rotation, dest);
    mat4_scale(dest, &trans->scale, dest);
}

INLINE void trans_get_forward(const transform_t *trans, vec3_t *dest){
    vec3_t forward;
    VEC3_SET_FORWARD(&forward);    
    quat_rotate_vec3(&forward, &trans->rotation, dest);
    vec3_normalize(dest, dest);
}

INLINE void trans_get_up(const transform_t *trans, vec3_t *dest){
    vec3_t up;
    VEC3_SET_UP(&up);
    quat_rotate_vec3(&up, &trans->rotation, dest);
    vec3_normalize(dest, dest);
}

INLINE void trans_get_right(const transform_t *trans, vec3_t *dest){
    vec3_t right;
    VEC3_SET_RIGHT(&right);
    quat_rotate_vec3(&right, &trans->rotation, dest);
    vec3_normalize(&right, &right);
}

INLINE void trans_get_backward(const transform_t *trans, vec3_t *dest){
    trans_get_forward(trans, dest);
    vec3_inv(dest, dest);
}

INLINE void trans_get_down(const transform_t *trans, vec3_t *dest){
    trans_get_up(trans, dest);
    vec3_inv(dest, dest);
}

INLINE void trans_get_left(const transform_t *trans, vec3_t *dest){
    trans_get_right(trans, dest);
    vec3_inv(dest, dest);
}
