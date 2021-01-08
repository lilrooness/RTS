#include <iostream>
#include <SDL.h>
#include <GL/glew.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader_reader.h"
#include "settings.h"
#include "game.h"

#undef main

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 1280;

const GLuint POS_ATTRIB_LOC = 1;
const GLuint TRANSLATION_ATTRIB_LOC = 2;
const GLuint NORMAL_ATTRIB_LOC = 3;
const GLuint HEADING_ATTRIB_LOC = 4;
const GLuint TINT_ATTRIB_LOC = 5;

GLuint genericQuadIndexData[] = { 0, 1, 2, 0, 2, 3 };

struct Model {
    GLuint* indices;
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint NAO;
    GLuint TRANSLATION_VBO;
    GLuint HEADINGS_VBO;
    GLuint TINT_VBO;
};

Model turret;
Model tank;
Model gun;

//takes normalised divice coordinates ([-1, 1], [-1, 1]) and turns them into a ray direction from the camera
glm::vec4 screenToRay(float mouseX, float mouseY, glm::mat4 projectionMatrix, glm::mat4 viewMatrix) {
    glm::vec4 mouseCoords = glm::vec4(mouseX,mouseY, -1.0f, 0.0f);
    glm::vec4 direction = glm::inverse(projectionMatrix) * mouseCoords;
    direction.z = -1.0;
    direction.w = 0.0f;
    direction = glm::inverse(viewMatrix) * direction;
    direction = glm::normalize(direction);

    return direction;
}

void loadMeshToVAO(aiMesh *mesh, Model *model) {
    glGenVertexArrays(1, &model->VAO);
    glBindVertexArray(model->VAO);

    int verticesSize = mesh->mNumVertices * 3 * sizeof(GLfloat);

    glGenBuffers(1, &model->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, model->VBO);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, mesh->mVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(POS_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(POS_ATTRIB_LOC);

    glGenBuffers(1, &model->NAO);
    glBindBuffer(GL_ARRAY_BUFFER, model->NAO);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, mesh->mNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(NORMAL_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(NORMAL_ATTRIB_LOC);

    int numIndices = mesh->mFaces[0].mNumIndices * mesh->mNumFaces;
    model->indices = (GLuint*) malloc(sizeof(GLuint) * numIndices);
    
    for (int faceIdx = 0; faceIdx < mesh->mNumFaces; faceIdx++) {
        aiFace* face = &(mesh->mFaces[faceIdx]);
        for (int i = 0; i < face->mNumIndices; i++) {
            model->indices[i + (faceIdx * 3)] = face->mIndices[i];
        }
    }

    glGenBuffers(1, &model->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLuint), model->indices, GL_STATIC_DRAW);

    glGenBuffers(1, &model->TRANSLATION_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, model->TRANSLATION_VBO);
    GLfloat buffer[]{ -10.0f, 0.0f, 0.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3, buffer, GL_STATIC_DRAW);
    glVertexAttribPointer(TRANSLATION_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);
    glEnableVertexAttribArray(TRANSLATION_ATTRIB_LOC);
    glVertexAttribDivisor(TRANSLATION_ATTRIB_LOC, 1);

    glGenBuffers(1, &model->HEADINGS_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, model->HEADINGS_VBO);
    GLfloat headings_buffer[]{ 0.0 };
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat), headings_buffer, GL_STATIC_DRAW);
    glVertexAttribPointer(HEADING_ATTRIB_LOC, 1, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(HEADING_ATTRIB_LOC);
    glVertexAttribDivisor(HEADING_ATTRIB_LOC, 1);

    glGenBuffers(1, &model->TINT_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, model->TINT_VBO);
    GLfloat tint_buffer[]{ 1.0f, 1.0f, 1.0f, 1.0f };
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4, tint_buffer, GL_STATIC_DRAW);
    glVertexAttribPointer(TINT_ATTRIB_LOC, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(TINT_ATTRIB_LOC);
    glVertexAttribDivisor(TINT_ATTRIB_LOC, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void refreshBuffers(Game* game) {
    glBindBuffer(GL_ARRAY_BUFFER, gun.TRANSLATION_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * game->tanksData.positions.size(), game->tanksData.positions.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, tank.TRANSLATION_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * game->tanksData.positions.size(), game->tanksData.positions.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, turret.TRANSLATION_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * game->tanksData.positions.size(), game->tanksData.positions.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, tank.HEADINGS_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * game->tanksData.headings.size(), game->tanksData.headings.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, gun.HEADINGS_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * game->tanksData.headings.size(), game->tanksData.headings.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, turret.HEADINGS_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * game->tanksData.headings.size(), game->tanksData.headings.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, tank.TINT_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * game->tanksData.tint.size(), game->tanksData.tint.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, turret.TINT_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * game->tanksData.tint.size(), game->tanksData.tint.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, gun.TINT_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * game->tanksData.tint.size(), game->tanksData.tint.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initGame(Game* game) {
    for (int i = 0; i < 10; i++) {
        addTank(game, -30.0f + (i * 8.0f), 0.0f, 0.0f, 0.0f);
    }

    refreshBuffers(game);

}

int main() {

    glm::mat4 projMat;
    glm::mat4 modelMat;
    glm::mat4 viewMat;

    Settings settings;
    load_settings_file(&settings, "res\\settings");

    glm::vec3 cameraPos = glm::vec3(settings.cameraPos.x, settings.cameraPos.y, settings.cameraPos.z);

    projMat = glm::perspective<float>(45.0f, 1.0f, 1.0, 10000.0);
    modelMat = glm::mat4(1.0f);
    viewMat = glm::mat4(1.0f);

    viewMat = glm::rotate(viewMat, (float)M_PI / 2.5f, glm::vec3(1.0f, 0.0f, 0.0f));
    viewMat = glm::translate(viewMat, cameraPos * -1.0f);

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile("res\\obj\\tank.obj", aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

    if (scene == NULL) {
        std::cout << "ERROR: " << importer.GetErrorString() << std::endl;
        return -1;
    }

    aiNode* rootNode = scene->mRootNode;

    if (rootNode == NULL) {
        std::cout << "root node is NULL for some reason ..." << std::endl;
        return -1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "ERROR Initialising SDL2: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_Window* window = NULL;
    SDL_GLContext context = NULL;

    window = SDL_CreateWindow("RTS game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    context = SDL_GL_CreateContext(window);

    if (window == NULL) {
        std::cout << "ERROR Creating Window: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (context == NULL) {
        std::cout << "ERROR Createing OpenGL Context: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (SDL_GL_SetSwapInterval(1) < 0) {
        std::cout << "ERROR Setting VSYNC True: " << SDL_GetError() << std::endl;
        return -1;
    }

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        printf("Error initialising Glew: %s\n", glewGetErrorString(glewError));
        return -1;
    }

    GLuint shaderProgramId = create_shader_program("res/shaders/shader.vs", "res/shaders/shader.fs");
    GLuint basicShaderProgramId = create_shader_program("res/shaders/basic/shader.vs", "res/shaders/basic/shader.fs");

    loadMeshToVAO(scene->mMeshes[0], &tank);
    loadMeshToVAO(scene->mMeshes[1], &turret);
    loadMeshToVAO(scene->mMeshes[2], &gun);

    bool quit = false;
    SDL_Event e;

    GLfloat xRotation = 0.0f;
    int lastFrame = SDL_GetTicks();

    Game game;
    initGame(&game);

    float mouseX{ 0.0f }, mouseY{ 0.0 };

    GLuint mousePointVAO;
    GLuint mousePointVBO;

    glGenVertexArrays(1, &mousePointVAO);
    glBindVertexArray(mousePointVAO);

    glGenBuffers(1, &mousePointVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mousePointVBO);
    GLfloat buffer[]{ 0.0, 0.0, 0.0 };
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), buffer, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    GLuint selectionQuadVAO;
    GLuint selectionQuadVBO;
    GLuint selectionQuadElementsBuffer;

    glGenVertexArrays(1, &selectionQuadVAO);
    glBindVertexArray(selectionQuadVAO);

    glGenBuffers(1, &selectionQuadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, selectionQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), game.groundSelectionQuadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    
    glGenBuffers(1, &selectionQuadElementsBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, selectionQuadElementsBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), genericQuadIndexData, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_MOUSEMOTION) {
                mouseX = (-1.0f + ((float)e.motion.x / (float)SCREEN_WIDTH * 2.0f));
                mouseY = (1.0f - ((float)e.motion.y / (float)SCREEN_HEIGHT * 2.0f));
                
                auto rayDirection = screenToRay(mouseX, mouseY, projMat, viewMat);
                
                rayGroundPlaneIntersection(rayDirection, cameraPos, &game.currentMouseGroundIntersection);

                glBindBuffer(GL_ARRAY_BUFFER, mousePointVBO);
                glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(GLfloat), glm::value_ptr(game.currentMouseGroundIntersection), GL_STATIC_DRAW);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                if (game.primaryButtonDown) {
                    game.mouseDragData.drag = game.currentMouseGroundIntersection;
                    glBindBuffer(GL_ARRAY_BUFFER, selectionQuadVBO);
                    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), game.groundSelectionQuadVertices, GL_STATIC_DRAW);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    if (!game.primaryButtonDown) {
                        //start dragging here
                        game.primaryButtonDown = true;
                        game.mouseDragData.origin = game.currentMouseGroundIntersection;
                        game.mouseDragData.drag = game.currentMouseGroundIntersection;
                        resetSelectionQuadVertices(&game);
                        glBindBuffer(GL_ARRAY_BUFFER, selectionQuadVBO);
                        glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), game.groundSelectionQuadVertices, GL_STATIC_DRAW);
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                    }
                }
                else if (e.button.type == SDL_BUTTON_RIGHT) {
                    
                }
            }
            else if (e.type == SDL_MOUSEBUTTONUP) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    if (game.primaryButtonDown) {
                        //stop dragging here
                        game.primaryButtonDown = false;
                    }
                }
            }
        }

        int timeNow = SDL_GetTicks();
        if (lastFrame == 0 || timeNow - lastFrame >= (1000/60)) {
            lastFrame = timeNow;
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_DEPTH_TEST);
            glClearColor(settings.clearColor.x, settings.clearColor.y, settings.clearColor.z, settings.clearColor.w);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glPointSize(2.0f);

            glUseProgram(shaderProgramId);
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(viewMat));
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(projMat));
            glUniform1f(4, 0);

            glBindVertexArray(tank.VAO);
            glDrawElementsInstanced(GL_TRIANGLES, scene->mMeshes[0]->mNumFaces * 3, GL_UNSIGNED_INT, NULL, game.tanks.size());

            glUniform1f(4, xRotation *-1.0f);
            glBindVertexArray(turret.VAO);
            glDrawElementsInstanced(GL_TRIANGLES, scene->mMeshes[1]->mNumFaces * 3, GL_UNSIGNED_INT, NULL, game.tanks.size());

            glBindVertexArray(gun.VAO);
            glDrawElementsInstanced(GL_TRIANGLES, scene->mMeshes[2]->mNumFaces * 3, GL_UNSIGNED_INT, NULL, game.tanks.size());
            
            glUseProgram(basicShaderProgramId);
            glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(modelMat));
            glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(viewMat));
            glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(projMat));
            glUniform1f(4, 0.5);

            glPointSize(20.0f);
            glBindVertexArray(mousePointVAO);
            glDrawArrays(GL_POINTS, 0, 1);

            if (game.primaryButtonDown) {
                glBindVertexArray(selectionQuadVAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
            }
            
            SDL_GL_SwapWindow(window);

            auto glError = glGetError();

            if (glError > 0) {
                std::cout << "OpenGL Error: " << glError << std::endl;
            }

            tick(&game);

            refreshBuffers(&game);


            xRotation += 0.005f;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
