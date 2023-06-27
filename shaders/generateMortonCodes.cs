#version 430 core

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct triangle{
    vec4 position[3];
    vec4 color;
    vec4 center;
    vec4 box[2];
    int code;
};

layout (std430, binding = 0) buffer Input{
    triangle data[];
} T;


uniform int n;

int interleve(float x){
    int y = int(x * 1024.0);
    int res = 0;
    for(int i = 0; i < 10; i++){
        res |= ((y & (1 << i)) << (2*i));
    }
    return res;
}

void main(){
    uint id = gl_GlobalInvocationID.x;
    if(id < n){
        T.data[id].code = interleve(T.data[id].center.x) |
                          (interleve(T.data[id].center.y) << 1) |
                          (interleve(T.data[id].center.z) << 2);
    }
}
