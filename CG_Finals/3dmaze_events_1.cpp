#include <GL/glut.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

// Include stb_image for image loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// Modified map with '3', '4', '5' indicating object positions
const int map[17][17] = {
    {1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1},
    {1,0,1,4,0,5,0,3,1,0,0,0,0,0,1,0,1},
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

float gx = 7.5f;
float gz = -1.0f;
float angle = 0.0f;
int collectedCount = 0; // Number of collected objects

// Structure for the game objects
struct GameObject {
    std::vector<float> vertices;
    std::vector<unsigned int> faces;
    float x, y, z; // Position of the object
    bool collected;
    std::string filename; // OBJ filename
};

// Use a vector to store objects dynamically
std::vector<GameObject> objects;

// Texture ID for the maze
GLuint textureID;

// Window dimensions
int windowWidth = 1280;
int windowHeight = 720;

bool loadOBJ(const char* filename, std::vector<float>& vertices, std::vector<unsigned int>& faces) {
    // [Function remains unchanged]
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
            // OBJ files are 1-indexed
            faces.push_back(v1 - 1);
            faces.push_back(v2 - 1);
            faces.push_back(v3 - 1);
        }
    }
    return true;
}

void renderBitmapString(float x, float y, void* font, const char* string) {
    // [Function remains unchanged]
    const char* c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

// Function to draw a textured cube
void drawTexturedCube(float size) {
    float halfSize = size / 2.0f;

    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);

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

void generatemap() {
    for (int mapi = 0; mapi < 17; ++mapi) {
        for (int mapj = 0; mapj < 17; ++mapj) {
            if (map[mapi][mapj] == 1) {
                glPushMatrix();
                glTranslatef((mapj + 0.5f), -0.2, -1 * (mapi + 0.5f));

                if (collectedCount == 0) {
                    // First phase: No lighting or textures
                    glDisable(GL_LIGHTING);
                    glDisable(GL_TEXTURE_2D);

                    // Solid surface, white, with thick black edges
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(1.0, 1.0);

                    glColor3ub(255, 255, 255); // White color
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                    // Draw lower cube
                    glutSolidCube(1);

                    // Draw upper cube
                    glPushMatrix();
                    glTranslatef(0, 1, 0);
                    glutSolidCube(1);
                    glPopMatrix();

                    glDisable(GL_POLYGON_OFFSET_FILL);

                    // Draw edges with thick black lines
                    glColor3ub(0, 0, 0);
                    glLineWidth(3.0f); // Increase line width
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                    // Draw lower cube edges
                    glutSolidCube(1);

                    // Draw upper cube edges
                    glPushMatrix();
                    glTranslatef(0, 1, 0);
                    glutSolidCube(1);
                    glPopMatrix();

                    glLineWidth(1.0f); // Reset line width
                }
                else if (collectedCount == 1) {
                    // Second phase: Enable lighting, green cubes with edges
                    glEnable(GL_LIGHTING);
                    glDisable(GL_TEXTURE_2D);

                    // Set material color to green
                    glColor3ub(144, 238, 144); // Light green

                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(1.0, 1.0);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

                    // Draw lower cube
                    glutSolidCube(1);

                    // Draw upper cube
                    glPushMatrix();
                    glTranslatef(0, 1, 0);
                    glutSolidCube(1);
                    glPopMatrix();

                    glDisable(GL_POLYGON_OFFSET_FILL);

                    // Draw edges
                    glColor3ub(0, 0, 0);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                    glutSolidCube(1);

                    glPushMatrix();
                    glTranslatef(0, 1, 0);
                    glutSolidCube(1);
                    glPopMatrix();
                }
                else if (collectedCount == 2) {
                    // Third phase: Enable lighting and texture, no edges
                    glEnable(GL_LIGHTING);
                    glEnable(GL_TEXTURE_2D);

                    glColor3f(1.0f, 1.0f, 1.0f); // Reset color to white

                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    drawTexturedCube(1);

                    glPushMatrix();
                    glTranslatef(0, 1, 0);
                    drawTexturedCube(1);
                    glPopMatrix();

                    glDisable(GL_TEXTURE_2D);
                }

                glPopMatrix();
            }
        }
    }
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(1, 1, 1, 1);

    // Set up lighting parameters
    GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f }; // Positional light above the scene
    GLfloat light_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat light_diffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    glEnable(GL_LIGHT0);

    // Set material properties
    GLfloat mat_specular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat mat_shininess[] = { 25.0f };

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

    // Process the map to find GameObjects
    for (int mapi = 0; mapi < 17; ++mapi) {
        for (int mapj = 0; mapj < 17; ++mapj) {
            int cellValue = map[mapi][mapj];
            if (cellValue == 3 || cellValue == 4 || cellValue == 5) {
                GameObject obj;
                obj.x = mapj + 0.5f;
                obj.y = -0.1f;
                obj.z = -1 * (mapi + 0.5f);
                obj.collected = false;
                // Assign filenames based on the cell value
                if (cellValue == 3) {
                    obj.filename = "C:/Users/teoju/Documents/GitHub/Computer-Graphics-Final/CG_Finals/Example1.obj";
                }
                else if (cellValue == 4) {
                    obj.filename = "C:/Users/teoju/Documents/GitHub/Computer-Graphics-Final/CG_Finals/Example2.obj";
                }
                else if (cellValue == 5) {
                    obj.filename = "C:/Users/teoju/Documents/GitHub/Computer-Graphics-Final/CG_Finals/Example3.obj";
                }
                objects.push_back(obj);
            }
        }
    }

    // Load OBJ files for all objects
    for (size_t i = 0; i < objects.size(); ++i) {
        if (!loadOBJ(objects[i].filename.c_str(), objects[i].vertices, objects[i].faces)) {
            exit(1);
        }
    }

    // Load texture from an image file
    int width, height, nrChannels;
    unsigned char* data = stbi_load("C:/Users/teoju/Documents/GitHub/Computer-Graphics-Final/CG_Finals/brick_texture.jpg", &width, &height, &nrChannels, 0);
    if (!data) {
        std::cout << "Failed to load texture" << std::endl;
        exit(1);
    }

    glGenTextures(1, &textureID);

    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Determine the format of the image
    GLenum format = GL_RGB;
    if (nrChannels == 1)
        format = GL_LUMINANCE;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrap mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrap mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Min filter with mipmaps
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Mag filter

    // Build mipmaps using gluBuild2DMipmaps for OpenGL 2.0 compatibility
    gluBuild2DMipmaps(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);

    // Set texture environment mode to GL_MODULATE
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(80, (double)windowWidth / (double)windowHeight, 0.1, 100);

    // Set up the camera
    gluLookAt(gx, 0.5, gz, gx + sin(angle), 0.5, gz - cos(angle), 0, 1, 0);

    // Modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw the maze
    generatemap();

    // Draw the OBJ models if not collected
    for (size_t i = 0; i < objects.size(); ++i) {
        if (!objects[i].collected) {
            glPushMatrix();
            glTranslatef(objects[i].x, objects[i].y, objects[i].z);

            if (collectedCount >= 1) {
                glEnable(GL_LIGHTING);
            }
            else {
                glDisable(GL_LIGHTING);
            }

            glColor3f(1.0f, 0.5f, 0.0f); // Color of the OBJ
            glScalef(0.5f, 0.5f, 0.5f);

            // Draw the object
            glBegin(GL_TRIANGLES);
            for (size_t j = 0; j < objects[i].faces.size(); j += 3) {
                unsigned int vi1 = objects[i].faces[j] * 3;
                unsigned int vi2 = objects[i].faces[j + 1] * 3;
                unsigned int vi3 = objects[i].faces[j + 2] * 3;

                // Calculate normal for the triangle
                float v1x = objects[i].vertices[vi1];
                float v1y = objects[i].vertices[vi1 + 1];
                float v1z = objects[i].vertices[vi1 + 2];

                float v2x = objects[i].vertices[vi2];
                float v2y = objects[i].vertices[vi2 + 1];
                float v2z = objects[i].vertices[vi2 + 2];

                float v3x = objects[i].vertices[vi3];
                float v3y = objects[i].vertices[vi3 + 1];
                float v3z = objects[i].vertices[vi3 + 2];

                float nx, ny, nz;
                // Compute normal vector
                float ux = v2x - v1x;
                float uy = v2y - v1y;
                float uz = v2z - v1z;

                float vx = v3x - v1x;
                float vy = v3y - v1y;
                float vz = v3z - v1z;

                nx = uy * vz - uz * vy;
                ny = uz * vx - ux * vz;
                nz = ux * vy - uy * vx;

                // Normalize the normal vector
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

            glPopMatrix();
        }
    }

    // Display "You won!" message if all objects are collected
    if (collectedCount >= objects.size()) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, windowWidth, 0, windowHeight);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor3f(0, 0, 0);
        renderBitmapString(windowWidth / 2 - 50, windowHeight / 2, GLUT_BITMAP_HELVETICA_18, "You won!");

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glEnable(GL_DEPTH_TEST);
    }

    // Move collision detection here
    // Check if the player found any object
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

void specialkeys(int key, int x, int y) {
    float nx = gx;
    float nz = gz;

    
    float fraction = 0.1f;

    // Always allow movement and rotation
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
        angle -= 3.14 / 180;
        break;
    case GLUT_KEY_RIGHT:
        angle += 3.14 / 180;
        break;
    }

    // Collision detection with walls
    //can adjust the range where the coliision is detected
    int mi = (int)(floor(-nz));
    int mj = (int)(floor(nx));

    // Adjust collision detection to allow movement into cells with 0, 3, 4, 5
    if (mi >= 0 && mi < 17 && mj >= 0 && mj < 17) {
        int cellValue = map[mi][mj];
        if (cellValue == 0 || cellValue == 3 || cellValue == 4 || cellValue == 5) {
            gx = nx;
            gz = nz;
        }
    }

    glutPostRedisplay();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
}

void mouse(int button, int state, int x, int y) {
    if (collectedCount >= objects.size() && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
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
