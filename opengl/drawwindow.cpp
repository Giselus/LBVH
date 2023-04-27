#include "drawwindow.h"

#include "gl.h"
#include "../utils/prefixsum.h"
#include "../utils/radixsort.h"
#include "../utils/shader.h"
#include "../utils/shadermanager.h"

#include <chrono>

void DrawWindow::initialize(){
    ShaderManager::initialize();

    GLfloat vertices[] = {
        -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f
    };

    GLfloat texCoords[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };

    GLfloat data[6*5];

    for(int i = 0; i < 6; i++){
        data[i*5] = vertices[i*3];
        data[i*5+1] = vertices[i*3+1];
        data[i*5+2] = vertices[i*3+2];

        data[i*5+3] = texCoords[i*2];
        data[i*5+4] = texCoords[i*2+1];
    }

    GL::funcs.glGenVertexArrays(1, &VAO);
    GL::funcs.glGenBuffers(1, &VBO);

    GL::funcs.glBindVertexArray(VAO);

    GL::funcs.glBindBuffer(GL_ARRAY_BUFFER, VBO);
    GL::funcs.glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    // position attribute
    GL::funcs.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    GL::funcs.glEnableVertexAttribArray(0);
    // color attribute
    GL::funcs.glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    GL::funcs.glEnableVertexAttribArray(1);

    GL::funcs.glGenTextures(1, &textureID);
    GL::funcs.glActiveTexture(GL_TEXTURE0);
    GL::funcs.glBindTexture(GL_TEXTURE_2D, textureID);
    GL::funcs.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL::funcs.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GL::funcs.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL::funcs.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL::funcs.glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    GL::funcs.glBindImageTexture(0, textureID, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);


//    int n = 1000000;
//    int *randomNums = (int*)malloc(sizeof(int) * n);
//    for(int i = 0; i < n; i++){
//        randomNums[i] = rand()%1000;
//    }

//    unsigned int inputID;
//    GL::funcs.glGenBuffers(1, &inputID);
//    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputID);
//    GL::funcs.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * n, randomNums, GL_STATIC_DRAW);
//    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputID);
//    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
//    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);
////    printBuffer<int>(10, inputID);

//    GLuint query;
//    GLuint64 elapsed_time;
//    GL::funcs.glGenQueries(1, &query);
//    GL::funcs.glBeginQuery(GL_TIME_ELAPSED, query);
//    unsigned int outputID;
////    outputID = prefixSum(n, inputID, true);
//    outputID = radixSort(n, 30, inputID);

//    GL::funcs.glEndQuery(GL_TIME_ELAPSED);
//    int done = 0;
//    while(!done){
//        GL::funcs.glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done);
//    }
//    GL::funcs.glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
////    printBuffer<int>(n, outputID);
//    int *outArr = getArrayFromBuffer<int>(n, outputID);
//    std::sort(randomNums, randomNums + n);
//    for(int i = 1; i < n; i++){
//        if(outArr[i] != randomNums[i]){
//            printBuffer<int>(i, 10, outputID);
//            qDebug() << "Array is not sorted!" << i << randomNums[i] << outArr[i];
//            break;
//        }
//    }
//    qDebug() << "radix sort took " << (float)elapsed_time/1000000.0 << " ms";
//    GL::funcs.glDeleteBuffers(1, &outputID);


}

void DrawWindow::render(){
    GL::funcs.glEnable(GL_DEPTH_TEST);
    GL::funcs.glDepthMask(GL_TRUE);

    const qreal retinaScale = devicePixelRatio();
    GL::funcs.glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    GL::funcs.glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    GL::funcs.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    Shader *shader = ShaderManager::getShader("testShader");
//    shader->use();

//    GL::funcs.glBindVertexArray(VAO);
//    GL::funcs.glDrawArrays(GL_TRIANGLES,0,3);

    Shader *computeShader = ShaderManager::getShader("gradShaderCompute");
    computeShader->use();
    GL::funcs.glDispatchCompute(TEXTURE_WIDTH, TEXTURE_HEIGHT, 1);
    GL::funcs.glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    Shader *gradShader = ShaderManager::getShader("gradShader");
    gradShader->use();
    gradShader->setInt("tex", 0);
    GL::funcs.glActiveTexture(GL_TEXTURE0);
    GL::funcs.glBindTexture(GL_TEXTURE_2D, textureID);
    GL::funcs.glBindVertexArray(VAO);
    GL::funcs.glDrawArrays(GL_TRIANGLES, 0, 6);

//    int n = 12486;
//    int n = 100000;
    int n = 1000000;
//    int n = 50000000;
//    int n = 409600;
    int *randomNums = (int*)malloc(sizeof(int) * n);
    for(int i = 0; i < n; i++){
        randomNums[i] = rand()%1000000;
    }
    int *expectedOutput = (int*)malloc(sizeof(int) * n);
    unsigned int inputID;
    GL::funcs.glGenBuffers(1, &inputID);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, inputID);
    GL::funcs.glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * n, randomNums, GL_STATIC_DRAW);
    GL::funcs.glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inputID);
    GL::funcs.glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    GL::funcs.glMemoryBarrier(GL_ALL_BARRIER_BITS);
    for(int i = 0; i < 5; i++){
        GLuint query;
        GLuint64 elapsed_time;
        GL::funcs.glGenQueries(1, &query);
        GL::funcs.glBeginQuery(GL_TIME_ELAPSED, query);
        unsigned int outputID;
//        outputID = prefixSum(n, inputID, 1);
        outputID = radixSort(n, 30, inputID);
        GL::funcs.glEndQuery(GL_TIME_ELAPSED);
        int done = 0;
        while(!done){
            GL::funcs.glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &done);
        }
        GL::funcs.glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
        qDebug() << "radix sort took " << (float)elapsed_time/1000000.0 << " ms";
        GL::funcs.glDeleteBuffers(1, &outputID);
    }
//    auto start = std::chrono::high_resolution_clock::now();
//    std::sort(randomNums, randomNums + n);

//    auto stop = std::chrono::high_resolution_clock::now();
//    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
//    qDebug() << "stl sort took:" << duration.count()/1000.0 << " ms";

}
