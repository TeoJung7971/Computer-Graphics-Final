#include <GL/glut.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>

// Image file loadingÀ» À§ÇÑ stb header
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// Maze data

//0 = Empty
//1 = Wall
//N(N>1) = Objects

const int map[17][17] = {
    {1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1},
    {1,0,0,3,1,0,1,0,1,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1},
    {1,0,1,5,0,4,0,2,1,0,0,0,0,0,1,0,1},
    {1,0,1,1,1,1,1,1,1,0,1,0,1,0,1,0,1},
    {1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,1},
    {1,0,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,1,0,1,0,0,0,1},
    {1,0,1,1,1,0,1,0,0,0,1,0,1,1,1,1,1},
    {1,0,0,0,1,0,1,0,0,0,1,0,0,0,0,0,1},
    {1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1},
    {1,0,0,0,1,0,1,0,0,0,0,0,0,0,1,0,1},
    {1,0,1,0,1,0,1,1,1,0,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
    {1,0,1,1,1,0,1,0,1,1,1,0,1,1,1,0,1},
    {1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

//전역 변수 선언
float gx = 7.5f;
float gz = -1.0f;
float angle = 0.0f;
int collectedCount = 0; 

struct GameObject {
    std::vector<float> vertices;
    std::vector<unsigned int> faces;
    float x, y, z;
    bool collected;
    std::string filename;
};

std::vector<GameObject> objects;

GLuint textureID_Phase3;       // Maze texture for Phase 3
GLuint skyboxTextures_Phase3[6];
GLuint grassTextureID_Phase3;  // Grass texture for Phase 3

// NEW: Separate texture sets for Phase 2
GLuint textureID_Phase2;       // Maze texture for Phase 2
GLuint skyboxTextures_Phase2[6];
GLuint grassTextureID_Phase2;  // Grass texture for Phase 2

int windowWidth = 1280;
int windowHeight = 720;

//for final phase
float cubeRotation = 0.0f;
// for object rotation
float rotationAngle = 0.0f;

// Cube data for final phase
GLfloat MyVertices[8][3] = {
    {-0.25f, -0.25f, 0.25f}, {-0.25f, 0.25f, 0.25f}, {0.25f, 0.25f, 0.25f}, {0.25f, -0.25f, 0.25f},
    {-0.25f, -0.25f, -0.25f}, {-0.25f, 0.25f, -0.25f}, {0.25f, 0.25f, -0.25f}, {0.25f, -0.25f, -0.25f}
};

GLfloat MyColors[8][3] = {
    {0.2f, 0.2f, 0.2f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 1.0f}
};

GLubyte MyVertexList[24] = {
    0,1,2,3,
    7,6,5,4,
    3,2,6,7,
    4,5,1,0,
    5,6,2,1,
    7,4,0,3
};

bool loadOBJ(const char* filename, std::vector<float>& vertices, std::vector<unsigned int>& faces) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        cout << "Cannot open OBJ file: " << filename << endl;
        return false;
    }

    std::string line;
    while (getline(file, line)) {
        std::istringstream s(line);
        std::string type;
        s >> type;

        if (type == "v") {
            float x, y, z;
            s >> x >> y >> z;
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
        else if (type == "f") {
            unsigned int v1, v2, v3;
            s >> v1 >> v2 >> v3;
            faces.push_back(v1 - 1);
            faces.push_back(v2 - 1);
            faces.push_back(v3 - 1);
        }
    }
    return true;
}

// Function to show the characters on window
void renderBitmapString(float x, float y, void* font, const char* string) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

// Function to draw a textured cube, now taking a textureID as parameter
void drawTexturedCube(float size, GLuint texID) {
    float halfSize = size / 2.0f;

    glBindTexture(GL_TEXTURE_2D, texID);

    // Front face
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 0); glVertex3f(-halfSize, -halfSize, halfSize);
    glTexCoord2f(1, 0); glVertex3f(halfSize, -halfSize, halfSize);
    glTexCoord2f(1, 1); glVertex3f(halfSize, halfSize, halfSize);
    glTexCoord2f(0, 1); glVertex3f(-halfSize, halfSize, halfSize);
    glEnd();

    // Back face
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glTexCoord2f(1, 0); glVertex3f(-halfSize, -halfSize, -halfSize);
    glTexCoord2f(1, 1); glVertex3f(-halfSize, halfSize, -halfSize);
    glTexCoord2f(0, 1); glVertex3f(halfSize, halfSize, -halfSize);
    glTexCoord2f(0, 0); glVertex3f(halfSize, -halfSize, -halfSize);
    glEnd();

    // Left face
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glTexCoord2f(0, 0); glVertex3f(-halfSize, -halfSize, -halfSize);
    glTexCoord2f(1, 0); glVertex3f(-halfSize, -halfSize, halfSize);
    glTexCoord2f(1, 1); glVertex3f(-halfSize, halfSize, halfSize);
    glTexCoord2f(0, 1); glVertex3f(-halfSize, halfSize, -halfSize);
    glEnd();

    // Right face
    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);
    glTexCoord2f(1, 0); glVertex3f(halfSize, -halfSize, -halfSize);
    glTexCoord2f(1, 1); glVertex3f(halfSize, halfSize, -halfSize);
    glTexCoord2f(0, 1); glVertex3f(halfSize, halfSize, halfSize);
    glTexCoord2f(0, 0); glVertex3f(halfSize, -halfSize, halfSize);
    glEnd();

    // Top face
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 1); glVertex3f(-halfSize, halfSize, -halfSize);
    glTexCoord2f(0, 0); glVertex3f(-halfSize, halfSize, halfSize);
    glTexCoord2f(1, 0); glVertex3f(halfSize, halfSize, halfSize);
    glTexCoord2f(1, 1); glVertex3f(halfSize, halfSize, -halfSize);
    glEnd();

    // Bottom face
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glTexCoord2f(1, 1); glVertex3f(-halfSize, -halfSize, -halfSize);
    glTexCoord2f(0, 1); glVertex3f(halfSize, -halfSize, -halfSize);
    glTexCoord2f(0, 0); glVertex3f(halfSize, -halfSize, halfSize);
    glTexCoord2f(1, 0); glVertex3f(-halfSize, -halfSize, halfSize);
    glEnd();
}

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// Generic skybox loader (for Phase 2 and Phase 3)
void loadSkyboxTextures(GLuint* skyboxTexArray, const char* right, const char* left, const char* top, const char* bottom, const char* front, const char* back) {
    const char* filenames[6] = { right, left, top, bottom, front, back };

    glGenTextures(6, skyboxTexArray);

    for (int i = 0; i < 6; ++i) {
        int width, height, nrChannels;
        unsigned char* data = stbi_load(filenames[i], &width, &height, &nrChannels, 0);
        if (!data) {
            std::cerr << "Failed to load skybox texture: " << filenames[i] << std::endl;
            exit(1);
        }

        glBindTexture(GL_TEXTURE_2D, skyboxTexArray[i]);
        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_LUMINANCE;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }
}

// Draw skybox according to current phase
void drawSkybox() {
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    glColor3f(1.0f, 1.0f, 1.0f);
    float size = 50.0f;

    // Choose skybox textures based on phase
    GLuint* currentSkybox;
    if (collectedCount == 2) {
        currentSkybox = skyboxTextures_Phase2;
    }
    else {
        currentSkybox = skyboxTextures_Phase3;
    }

    glPushMatrix();
    // Center skybox at player's position
    glTranslatef(gx, 0.5f, gz);

    // Right
    glBindTexture(GL_TEXTURE_2D, currentSkybox[0]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(size, -size, -size);
    glTexCoord2f(1, 0); glVertex3f(size, -size, size);
    glTexCoord2f(1, 1); glVertex3f(size, size, size);
    glTexCoord2f(0, 1); glVertex3f(size, size, -size);
    glEnd();

    // Left
    glBindTexture(GL_TEXTURE_2D, currentSkybox[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 0); glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 1); glVertex3f(-size, size, -size);
    glTexCoord2f(0, 1); glVertex3f(-size, size, size);
    glEnd();

    // Top
    glBindTexture(GL_TEXTURE_2D, currentSkybox[2]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-size, size, -size);
    glTexCoord2f(1, 0); glVertex3f(size, size, -size);
    glTexCoord2f(1, 1); glVertex3f(size, size, size);
    glTexCoord2f(0, 1); glVertex3f(-size, size, size);
    glEnd();

    // Bottom
    glBindTexture(GL_TEXTURE_2D, currentSkybox[3]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 0); glVertex3f(size, -size, size);
    glTexCoord2f(1, 1); glVertex3f(size, -size, -size);
    glTexCoord2f(0, 1); glVertex3f(-size, -size, -size);
    glEnd();

    // Front
    glBindTexture(GL_TEXTURE_2D, currentSkybox[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-size, -size, -size);
    glTexCoord2f(1, 0); glVertex3f(size, -size, -size);
    glTexCoord2f(1, 1); glVertex3f(size, size, -size);
    glTexCoord2f(0, 1); glVertex3f(-size, size, -size);
    glEnd();

    // Back
    glBindTexture(GL_TEXTURE_2D, currentSkybox[5]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(size, -size, size);
    glTexCoord2f(1, 0); glVertex3f(-size, -size, size);
    glTexCoord2f(1, 1); glVertex3f(-size, size, size);
    glTexCoord2f(0, 1); glVertex3f(size, size, size);
    glEnd();

    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopAttrib();
}

// Generic function to load a grass texture
GLuint loadGrassTexture(const char* filename) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load grass texture: " << filename << std::endl;
        exit(1);
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    GLenum format = GL_RGB;
    if (nrChannels == 1) format = GL_LUMINANCE;
    else if (nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    return texID;
}

// Draw infinite ground plane according to current phase
void drawGroundPlane() {
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);

    // NEW: Choose grass texture based on phase
    GLuint currentGrassTex = (collectedCount == 2) ? grassTextureID_Phase2 : grassTextureID_Phase3;

    glBindTexture(GL_TEXTURE_2D, currentGrassTex);
    glColor3f(1.0f, 1.0f, 1.0f);

    float size = 1000.0f;
    float yPosition = -0.6f;

    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 0); glVertex3f(-size, yPosition, -size);
    glTexCoord2f(size / 10.0f, 0); glVertex3f(size, yPosition, -size);
    glTexCoord2f(size / 10.0f, size / 10.0f); glVertex3f(size, yPosition, size);
    glTexCoord2f(0, size / 10.0f); glVertex3f(-size, yPosition, size);
    glEnd();

    glPopAttrib();
}


// Generate the map
void generatemap() {

    static bool firstTime_0 = true;
    static bool firstTime_1 = true;
    static bool firstTime_2 = true;
    static bool firstTime_3 = true;

    for (int mapi = 0; mapi < 17; ++mapi) {
        for (int mapj = 0; mapj < 17; ++mapj) {
            if (map[mapi][mapj] == 1) {
                glPushMatrix();
                glTranslatef((mapj + 0.5f), -0.2f, -1 * (mapi + 0.5f));

                if (collectedCount == 0) {
                    // Phase 0: White cubes, no lighting/texture
                    if (firstTime_0) {
                        printf("Phase 0 start\n");
                        firstTime_0 = false;
                    }

                    glDisable(GL_LIGHTING);
                    glDisable(GL_TEXTURE_2D);

                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(1.0, 1.0);
                    glColor3ub(255, 255, 255);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glutSolidCube(1);

                    glPushMatrix();
                    glTranslatef(0, 1, 0);
                    glutSolidCube(1);
                    glPopMatrix();

                    glDisable(GL_POLYGON_OFFSET_FILL);

                    glColor3ub(0, 0, 0);
                    glLineWidth(3.0f);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glutSolidCube(1);

                    glPushMatrix();
                    glTranslatef(0, 1, 0);
                    glutSolidCube(1);
                    glPopMatrix();

                    glLineWidth(1.0f);

                }
                else if (collectedCount == 1) {
                    // Phase 1: Dark red cubes without lighting
                    if (firstTime_1) {
                        printf("Phase 1 start\n");
                        firstTime_1 = false;
                    }

                    glDisable(GL_TEXTURE_2D);

                    glColor3ub(139, 0, 0);

                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(1.0, 1.0);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glutSolidCube(1);

                    glPushMatrix();
                    glTranslatef(0, 1, 0);
                    glutSolidCube(1);
                    glPopMatrix();

                    glDisable(GL_POLYGON_OFFSET_FILL);

                    glColor3ub(0, 0, 0);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glutSolidCube(1);
                    glPushMatrix();
                    glTranslatef(0, 1, 0);
                    glutSolidCube(1);
                    glPopMatrix();

                }
                else if (collectedCount == 2) {
                    // Phase 2 start: Dark red cubes without lighting AND Phase 2 textures
                    if (firstTime_2) {
                        printf("Phase 2 start\n");
                        firstTime_2 = false;
                    }

                    //glEnable(GL_LIGHTING);
                    glEnable(GL_TEXTURE_2D);

                    glColor3f(1.0f, 1.0f, 1.0f);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    // Use phase 2 texture for walls
                    drawTexturedCube(1, textureID_Phase2);

                    glPushMatrix();
                    glTranslatef(0, 1, 0);
                    drawTexturedCube(1, textureID_Phase2);
                    glPopMatrix();

                    glDisable(GL_POLYGON_OFFSET_FILL);

                }
                else if (collectedCount == 3) {
                    // Phase 3 start: Texture and lighting with the original Phase 3 textures
                    if (firstTime_3) {
                        printf("Phase 3 start\n");
                        firstTime_3 = false;
                    }

                    glEnable(GL_LIGHTING);
                    glEnable(GL_TEXTURE_2D);

                    glColor3f(1.0f, 1.0f, 1.0f);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    // Use phase 3 texture for walls
                    drawTexturedCube(1, textureID_Phase3);

                    glPushMatrix();
                    glTranslatef(0, 1, 0);
                    drawTexturedCube(1, textureID_Phase3);
                    glPopMatrix();

                    glDisable(GL_TEXTURE_2D);
                }

                glPopMatrix();
            }
        }
    }
}

// Load a generic texture (for walls)
GLuint loadWallTexture(const char* filename) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cout << "Failed to load texture: " << filename << std::endl;
        exit(1);
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    GLenum format = GL_RGB;
    if (nrChannels == 1)
        format = GL_LUMINANCE;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    return texID;
}

// Set the lighting and load objects and textures
void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(1, 1, 1, 1);

    // Lighting parameters
    GLfloat light_position[] = { 0.0f, 250.0f, 120.0f, 1.0f }; //x, y, z
    // Lighting color
    GLfloat light_ambient[] = { 0.3f, 0.3f, 0.3f, 1.4f }; // 중립적인 회색 환경광
    GLfloat light_diffuse[] = { 1.2f, 1.2f, 0.8f, 1.0f }; // 노란 확산광
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 흰색 하이라이트

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    glEnable(GL_LIGHT0);

    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_shininess[] = { 75.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

    // Process map for objects
    for (int mapi = 0; mapi < 17; ++mapi) {
        for (int mapj = 0; mapj < 17; ++mapj) {
            int cellValue = map[mapi][mapj];
            if (cellValue == 2 || cellValue == 3 || cellValue == 4 || cellValue == 5) {
                GameObject obj;
                obj.x = mapj + 0.5f;
                obj.y = -0.1f;
                obj.z = -1 * (mapi + 0.5f);
                obj.collected = false;

                if (cellValue == 2) {
                    obj.filename = "resource/Example1.obj";
                }
                else if (cellValue == 3) {
                    obj.filename = "resource/Example2.obj";
                }
                else if (cellValue == 4) {
                    obj.filename = "resource/Example3.obj";
                }
                else if (cellValue == 5) {
                    obj.filename = "resource/Example4.obj";
                }
                objects.push_back(obj);
            }
        }
    }

    // Load OBJ files
    for (size_t i = 0; i < objects.size(); ++i) {
        if (!loadOBJ(objects[i].filename.c_str(), objects[i].vertices, objects[i].faces)) {
            exit(1);
        }
    }

    // Load textures for Phase 3
    textureID_Phase3 = loadWallTexture("resource/brick_texture.jpg");
    loadSkyboxTextures(skyboxTextures_Phase3,
        "resource/skybox_right.png",
        "resource/skybox_left.png",
        "resource/skybox_top.png",
        "resource/skybox_bottom.png",
        "resource/skybox_front.png",
        "resource/skybox_back.png");

    grassTextureID_Phase3 = loadGrassTexture("resource/grass_texture_.png");

    // NEW: Load textures for Phase 2
    textureID_Phase2 = loadWallTexture("resource/phase2_brick_texture_.png");
    loadSkyboxTextures(skyboxTextures_Phase2,
        "resource/phase2_skybox_right.png",
        "resource/phase2_skybox_left.png",
        "resource/phase2_skybox_top.png",
        "resource/phase2_skybox_bottom.png",
        "resource/phase2_skybox_front.png",
        "resource/phase2_skybox_back.png");

    grassTextureID_Phase2 = loadGrassTexture("resource/grass_texture_.png");
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (collectedCount >= (int)objects.size()) {
        // Final phase: black background and rotating cube
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushMatrix();

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60, (double)windowWidth / (double)windowHeight, 1.0, 10.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);

        glRotatef(cubeRotation, 1.0f, 1.0f, 0.0f);

        glShadeModel(GL_SMOOTH);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, MyVertices);
        glColorPointer(3, GL_FLOAT, 0, MyColors);
        glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, MyVertexList);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        cubeRotation += 0.3f;
        if (cubeRotation > 360.0f) cubeRotation -= 360.0f;

        glPopMatrix();
        glPopAttrib();
        glutSwapBuffers();
        glutPostRedisplay();
    }
    else {
        // Before final phase: Map and the objects
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(80, (double)windowWidth / (double)windowHeight, 0.1, 1000);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(gx, 0.5, gz, gx + sin(angle), 0.5, gz - cos(angle), 0, 1, 0);

        if (collectedCount == ((int)objects.size() - 1) || collectedCount == 2) {
            // Draw skybox
            drawSkybox();
            // Draw grass field
            drawGroundPlane();
        }

        // Draw maze
        generatemap();

        for (size_t i = 0; i < objects.size(); ++i) {
            if (!objects[i].collected) {
                glPushMatrix();
                glTranslatef(objects[i].x, objects[i].y, objects[i].z);

                // Add object rotation
                glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);

                // Color and lighting settings based on collectedCount
                if (collectedCount == 0) {
                    glDisable(GL_LIGHTING);
                    glColor3f(0.0f, 0.0f, 0.0f);
                }
                else if (collectedCount == 1) {
                    glDisable(GL_LIGHTING);
                    glColor3f(0.5f, 0.5f, 0.7f);
                }
                else if (collectedCount == 2) {
                    glDisable(GL_LIGHTING);
                    glColor3f(0.9f, 0.9f, 1.2f); // 약간 밝은 톤
                }
                else if (collectedCount == 3) {
                    glDisable(GL_LIGHTING);
                    glShadeModel(GL_SMOOTH);
                }

                glScalef(0.35f, 0.35f, 0.35f);

                if (collectedCount < 3) {
                    // 기존 삼각형 기반 렌더링
                    glBegin(GL_TRIANGLES);
                    for (size_t j = 0; j < objects[i].faces.size(); j += 3) {
                        unsigned int vi1 = objects[i].faces[j] * 3;
                        unsigned int vi2 = objects[i].faces[j + 1] * 3;
                        unsigned int vi3 = objects[i].faces[j + 2] * 3;

                        float v1x = objects[i].vertices[vi1];
                        float v1y = objects[i].vertices[vi1 + 1];
                        float v1z = objects[i].vertices[vi1 + 2];

                        float v2x = objects[i].vertices[vi2];
                        float v2y = objects[i].vertices[vi2 + 1];
                        float v2z = objects[i].vertices[vi2 + 2];

                        float v3x = objects[i].vertices[vi3];
                        float v3y = objects[i].vertices[vi3 + 1];
                        float v3z = objects[i].vertices[vi3 + 2];

                        float ux = v2x - v1x;
                        float uy = v2y - v1y;
                        float uz = v2z - v1z;

                        float vx = v3x - v1x;
                        float vy = v3y - v1y;
                        float vz = v3z - v1z;

                        float nx = uy * vz - uz * vy;
                        float ny = uz * vx - ux * vz;
                        float nz = ux * vy - uy * vx;

                        float length = sqrt(nx * nx + ny * ny + nz * nz);
                        if (length != 0) {
                            nx /= length;
                            ny /= length;
                            nz /= length;
                        }

                        glNormal3f(nx, ny, nz);
                        glVertex3f(v1x, v1y, v1z);
                        glVertex3f(v2x, v2y, v2z);
                        glVertex3f(v3x, v3y, v3z);
                    }
                    glEnd();
                }
                else {
                    // collectedCount == 3 인 경우: 사각형 기반 렌더링
                    glBegin(GL_QUADS);

                    for (size_t j = 0; j + 5 < objects[i].faces.size(); j += 6) {
                        unsigned int f[6] = {
                            objects[i].faces[j],
                            objects[i].faces[j + 1],
                            objects[i].faces[j + 2],
                            objects[i].faces[j + 3],
                            objects[i].faces[j + 4],
                            objects[i].faces[j + 5]
                        };

                        std::vector<unsigned int> tri1 = { f[0], f[1], f[2] };
                        std::vector<unsigned int> tri2 = { f[3], f[4], f[5] };

                        auto find_shared = [&](std::vector<unsigned int>& t1, std::vector<unsigned int>& t2) {
                            std::vector<unsigned int> shared;
                            for (auto idx : t1) {
                                if (std::find(t2.begin(), t2.end(), idx) != t2.end()) {
                                    shared.push_back(idx);
                                }
                            }
                            return shared;
                            };

                        // tri2를 최대 3번 회전하며 올바른 공유 꼭짓점 2개를 찾는다.
                        for (int attempt = 0; attempt < 3; attempt++) {
                            std::vector<unsigned int> shared = find_shared(tri1, tri2);
                            if (shared.size() == 2) {
                                // 올바른 공유 꼭짓점 획득
                                // uniqueIndices 구하기
                                std::vector<unsigned int> uniqueIndices;
                                for (int u = 0; u < 3; u++) {
                                    if (std::find(uniqueIndices.begin(), uniqueIndices.end(), tri1[u]) == uniqueIndices.end())
                                        uniqueIndices.push_back(tri1[u]);
                                }
                                for (int u = 0; u < 3; u++) {
                                    if (std::find(uniqueIndices.begin(), uniqueIndices.end(), tri2[u]) == uniqueIndices.end())
                                        uniqueIndices.push_back(tri2[u]);
                                }

                                if (uniqueIndices.size() == 4) {
                                    // A, B, C, D를 결정
                                    // shared[0], shared[1]이 공유 꼭짓점.
                                    // tri1에서 shared에 없는 것이 A, tri2에서 shared에 없는 것이 D라 가정
                                    unsigned int A, D;
                                    for (auto idx : tri1) {
                                        if (std::find(shared.begin(), shared.end(), idx) == shared.end()) {
                                            A = idx; break;
                                        }
                                    }
                                    for (auto idx : tri2) {
                                        if (std::find(shared.begin(), shared.end(), idx) == shared.end()) {
                                            D = idx; break;
                                        }
                                    }

                                    unsigned int B = shared[0];
                                    unsigned int C = shared[1];

                                    auto getV = [&](unsigned int idx) {
                                        return &objects[i].vertices[idx * 3];
                                        };

                                    float* Av = getV(A);
                                    float* Bv = getV(B);
                                    float* Cv = getV(C);
                                    float* Dv = getV(D);

                                    // 노멀 계산
                                    float ux = Bv[0] - Av[0];
                                    float uy = Bv[1] - Av[1];
                                    float uz = Bv[2] - Av[2];

                                    float vx = Cv[0] - Av[0];
                                    float vy = Cv[1] - Av[1];
                                    float vz = Cv[2] - Av[2];

                                    float nx = uy * vz - uz * vy;
                                    float ny = uz * vx - ux * vz;
                                    float nz = ux * vy - uy * vx;

                                    float length = sqrt(nx * nx + ny * ny + nz * nz);
                                    if (length != 0) {
                                        nx /= length;
                                        ny /= length;
                                        nz /= length;
                                    }

                                    glNormal3f(nx, ny, nz);

                                    // Rainbow Colors: Red, Green, Blue, Yellow
                                    glColor3fv(MyColors[1]); glVertex3fv(Av);
                                    glColor3fv(MyColors[2]); glVertex3fv(Bv);
                                    glColor3fv(MyColors[4]); glVertex3fv(Cv);
                                    glColor3fv(MyColors[6]); glVertex3fv(Dv);
                                }
                                // 공유 꼭짓점 찾았으니 break
                                break;
                            }
                            // 공유 꼭짓점 2개 못 찾으면 tri2 회전
                            std::rotate(tri2.begin(), tri2.begin() + 1, tri2.end());
                        }

                    }
                    glEnd();
                }

                glPopMatrix();
            }
        }

        // Update rotation angle
        rotationAngle += 0.2f; // Adjust speed as needed
        if (rotationAngle > 360.0f) rotationAngle -= 360.0f;

        // Collision detection
        for (size_t i = 0; i < objects.size(); ++i) {
            if (!objects[i].collected) {
                float dx = gx - objects[i].x;
                float dz = gz - objects[i].z;
                float distance = sqrt(dx * dx + dz * dz);
                if (distance < 0.8f) {
                    objects[i].collected = true;
                    collectedCount++;
                }
            }
        }

        glutSwapBuffers();
    }
}


void specialkeys(int key, int x, int y) {
    if (collectedCount >= (int)objects.size()) {
        return;
    }

    float nx = gx;
    float nz = gz;
    float fraction = 0.1f;
    const float collisionRadius = 0.25f;

    switch (key) {
    case GLUT_KEY_UP:
        nx += fraction * sin(angle);
        nz -= fraction * cos(angle);
        break;
    case GLUT_KEY_DOWN:
        nx -= fraction * sin(angle);
        nz += fraction * cos(angle);
        break;
    case GLUT_KEY_LEFT:
        angle -= M_PI / 180;
        break;
    case GLUT_KEY_RIGHT:
        angle += M_PI / 180;
        break;
    }

    int mi = (int)(floor(-nz));
    int mj = (int)(floor(nx));

    bool canMove = true;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            int checkMi = mi + i;
            int checkMj = mj + j;

            if (checkMi >= 0 && checkMi < 17 && checkMj >= 0 && checkMj < 17) {
                int cellValue = map[checkMi][checkMj];
                if (cellValue == 1) {
                    float cellCenterX = checkMj + 0.5f;
                    float cellCenterZ = -checkMi - 0.5f;
                    float dx = nx - cellCenterX;
                    float dz = nz - cellCenterZ;
                    float distance = sqrt(dx * dx + dz * dz);
                    if (distance < (collisionRadius + 0.5f)) {
                        canMove = false;
                        break;
                    }
                }
            }
        }
        if (!canMove) break;
    }

    if (canMove) {
        gx = nx;
        gz = nz;
    }

    glutPostRedisplay();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
}

void mouse(int button, int state, int x, int y) {
    if (collectedCount >= (int)objects.size() && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        exit(0);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow("3D MAZE Explorer");
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialkeys);
    glutMouseFunc(mouse);

    init();
    glutMainLoop();
    return 0;
}
