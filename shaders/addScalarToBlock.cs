#version 430 core

//Adds scalar to every block of 1024 elements. Used in recursive invocation of scan algorithm

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer Input{
    int data[];
} T;

layout (std430, binding = 1) buffer PartialResults{
    int data[];
} P;

uniform int n;

void main(){
    uint id = gl_GlobalInvocationID.x;
    if(id < n){
        T.data[id] += P.data[gl_WorkGroupID.x];
    }
}
