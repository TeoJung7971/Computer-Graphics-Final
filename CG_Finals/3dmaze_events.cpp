#include <GL/glut.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

using namespace std;

const int map[17][17] = {
    {1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,1,0,1,0,1,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,1},
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

GameObject objects[3];

// Texture ID for the maze
GLuint textureID;

// Window dimensions
int windowWidth = 1280;
int windowHeight = 720;

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
            // OBJ files are 1-indexed
            faces.push_back(v1 - 1);
            faces.push_back(v2 - 1);
            faces.push_back(v3 - 1);
        }
    }
    return true;
}

void renderBitmapString(float x, float y, void* font, const char* string) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

// Function to draw a textured cube
void drawTexturedCube(float size) {
    float halfSize = size / 2.0f;

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
                glTranslatef((mapj + 0.5f), 0, -1 * (mapi + 0.5f));

                if (collectedCount == 0) {
                    // Wireframe mode
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glColor3ub(0, 0, 0);
                    glutSolidCube(1);
                    glTranslatef(0, 1, 0);
                    glutSolidCube(1);
                }
                else if (collectedCount == 1) {
                    // Solid surface, light gray, with black edges
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(1.0, 1.0);

                    glColor3ub(200, 200, 200); // Light gray
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glutSolidCube(1);
                    glTranslatef(0, 1, 0);
                    glutSolidCube(1);

                    glDisable(GL_POLYGON_OFFSET_FILL);

                    // Draw edges
                    glColor3ub(0, 0, 0);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glutSolidCube(1);
                    glTranslatef(0, -1, 0);
                    glutSolidCube(1);
                }
                else if (collectedCount == 2) {
                    // Colored surface, light green, with black edges
                    glEnable(GL_POLYGON_OFFSET_FILL);
                    glPolygonOffset(1.0, 1.0);

                    glColor3ub(144, 238, 144); // Light green
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glutSolidCube(1);
                    glTranslatef(0, 1, 0);
                    glutSolidCube(1);

                    glDisable(GL_POLYGON_OFFSET_FILL);

                    // Draw edges
                    glColor3ub(0, 0, 0);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glutSolidCube(1);
                    glTranslatef(0, -1, 0);
                    glutSolidCube(1);
                }
                else if (collectedCount >= 3) {
                    // Textured surface
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, textureID);

                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    drawTexturedCube(1);
                    glTranslatef(0, 1, 0);
                    drawTexturedCube(1);

                    glDisable(GL_TEXTURE_2D);

                    // Draw edges
                    glColor3ub(0, 0, 0);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                    glutSolidCube(1);
                    glTranslatef(0, -1, 0);
                    glutSolidCube(1);
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

    // Initialize GameObjects
    objects[0].filename = "Example1.obj";
    objects[0].x = 3.5f;
    objects[0].y = -0.35f;
    objects[0].z = -3.5f;
    objects[0].collected = false;

    objects[1].filename = "Example2.obj";
    objects[1].x = 5.5f;
    objects[1].y = -0.35f;
    objects[1].z = -5.5f;
    objects[1].collected = false;

    objects[2].filename = "Example3.obj";
    objects[2].x = 7.5f;
    objects[2].y = -0.35f;
    objects[2].z = -7.5f;
    objects[2].collected = false;

    // Load OBJ files
    for (int i = 0; i < 3; ++i) {
        if (!loadOBJ(objects[i].filename.c_str(), objects[i].vertices, objects[i].faces)) {
            exit(1);
        }
    }

    // Generate a checkerboard texture for the maze
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    const int texSize = 64;
    unsigned char textureData[texSize][texSize][3];

    for (int i = 0; i < texSize; ++i) {
        for (int j = 0; j < texSize; ++j) {
            int c = ((i & 8) ^ (j & 8)) * 255;
            textureData[i][j][0] = (unsigned char)c;
            textureData[i][j][1] = (unsigned char)c;
            textureData[i][j][2] = (unsigned char)c;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSize, texSize, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    generatemap();

    // Draw the OBJ models if not collected
    for (int i = 0; i < 3; ++i) {
        if (!objects[i].collected) {
            glPushMatrix();
            glTranslatef(objects[i].x, objects[i].y, objects[i].z);
            glColor3f(1.0f, 0.3f, 0.0f); // Color of the OBJ
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glBegin(GL_TRIANGLES);
            for (size_t j = 0; j < objects[i].faces.size(); j += 3) {
                unsigned int vi1 = objects[i].faces[j] * 3;
                unsigned int vi2 = objects[i].faces[j + 1] * 3;
                unsigned int vi3 = objects[i].faces[j + 2] * 3;

                glVertex3f(objects[i].vertices[vi1], objects[i].vertices[vi1 + 1], objects[i].vertices[vi1 + 2]);
                glVertex3f(objects[i].vertices[vi2], objects[i].vertices[vi2 + 1], objects[i].vertices[vi2 + 2]);
                glVertex3f(objects[i].vertices[vi3], objects[i].vertices[vi3 + 1], objects[i].vertices[vi3 + 2]);
            }
            glEnd();
            glPopMatrix();
        }
    }

    // Display "You won!" message if all objects are collected
    if (collectedCount >= 3) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, windowWidth, 0, windowHeight);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor3f(1, 1, 1);
        renderBitmapString(windowWidth / 2 - 50, windowHeight / 2, GLUT_BITMAP_HELVETICA_18, "You won!");

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
    }

    glutSwapBuffers();
}

void specialkeys(int key, int x, int y) {
    float nx = gx;
    float nz = gz;
    float fraction = 0.06f;

    if (collectedCount < 3) {
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
    }

    // Collision detection
    int mi = (int)(floor(-nz));
    int mj = (int)(floor(nx));

    if (mi >= 0 && mi < 17 && mj >= 0 && mj < 17 && map[mi][mj] == 0) {
        gx = nx;
        gz = nz;
    }

    // Check if the player found any object
    for (int i = 0; i < 3; ++i) {
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

    glutPostRedisplay();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
}

void mouse(int button, int state, int x, int y) {
    if (collectedCount >= 3 && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
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
