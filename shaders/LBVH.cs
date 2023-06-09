#version 430 core

//Tero Karras LBVH algorithm

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

struct triangle{
    vec4 position[3];
    vec4 color;
    vec4 center;
    vec4 box[2];
    int code;
};

struct node{
    vec4 box[2];
    uint children[2];
    uint interval[2];
    uint parent;
};

layout (std430, binding = 0) buffer Nodes{
    node data[];
} nodes;

layout (std430, binding = 1) buffer Triangles{
    triangle data[];
} triangles;

uint getCommonPrefix(uint id1, uint id2){
    int code1 = triangles.data[id1].code;
    int code2 = triangles.data[id2].code;

    uint ans = 0;
    int z = code1 ^ code2;
    if(code1 == code2){
        ans = 30;
        z = int(id1 ^ id2);
    }
    uint l = 0;
    uint r = 30;
    uint m;
    while(l < r){
        m = (l+r+1)/2;
        if((z >> (30-m)) == 0)
            l = m;
        else{
            r = m - 1;
        }
    }
    return ans + l;
}

uniform int n;

void main(){
    uint nodeID = gl_GlobalInvocationID.x;
    if(nodeID >= n-1)
        return;
    uint l;
    uint r;
    if(nodeID == 0){
        l = 0;
        r = n-1;
    }else{
        uint x = getCommonPrefix(nodeID, nodeID - 1);
        uint y = getCommonPrefix(nodeID, nodeID + 1);
        if(x > y){
            //range of the form[l, nodeID]
            uint bl = 0;
            uint br = nodeID - 1;
            uint m;
            while(bl < br){
                m = (bl+br)/2;
                if(getCommonPrefix(m, nodeID) < y){
                    bl = m + 1;
                }else{
                    br = m;
                }
            }

            l = bl;
            r = nodeID;
        }else if(y > x){
            //range of the form[nodeID, r]
            uint bl = nodeID+1;
            uint br = n-1;
            uint m;
            while(bl < br){
                m = (bl+br+1)/2;
                if(getCommonPrefix(m, nodeID) < x){
                    br = m - 1;
                }else{
                    bl = m;
                }
            }

            l = nodeID;
            r = br;
        }
    }

    nodes.data[nodeID].interval[0] = l;
    nodes.data[nodeID].interval[1] = r;

    uint bl = l;
    uint br = r;
    uint m;
    uint x = getCommonPrefix(l, r);
    while(bl < br){
        m = (bl+br+1)/2;
        if(getCommonPrefix(l, m) > x){
            bl = m;
        }else{
            br = m-1;
        }
    }
    if(l == bl){
        nodes.data[nodeID].children[0] = (n-1) + bl;
        nodes.data[(n-1)+bl].parent = nodeID;
        nodes.data[(n-1)+bl].interval[0] = l;
        nodes.data[(n-1)+bl].interval[1] = l;
    }else{
        nodes.data[nodeID].children[0] = bl;
        nodes.data[bl].parent = nodeID;
    }

    if(bl == r-1){
        nodes.data[nodeID].children[1] = n + bl;
        nodes.data[n+bl].parent = nodeID;
        nodes.data[n+bl].interval[0] = r;
        nodes.data[n+bl].interval[1] = r;
    }else{
        nodes.data[nodeID].children[1] = bl+1;
        nodes.data[bl+1].parent = nodeID;
    }
}
