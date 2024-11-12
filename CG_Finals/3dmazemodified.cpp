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

// Variables for the OBJ model
std::vector<float> obj_vertices;
std::vector<unsigned int> obj_faces;
float objX = 3.5f; // Position of the OBJ
float objZ = -3.5f;
bool objFound = false;

// Window dimensions
int windowWidth = 1280;
int windowHeight = 720;

bool loadOBJ(const char* filename) {
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
            obj_vertices.push_back(x);
            obj_vertices.push_back(y);
            obj_vertices.push_back(z);
        }
        else if (type == "f") {
            unsigned int v1, v2, v3;
            s >> v1 >> v2 >> v3;
            // OBJ files are 1-indexed
            obj_faces.push_back(v1 - 1);
            obj_faces.push_back(v2 - 1);
            obj_faces.push_back(v3 - 1);
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

void generatemap() {
    for (int mapi = 0; mapi < 17; ++mapi) {
        for (int mapj = 0; mapj < 17; ++mapj) {
            if ((mapi + mapj) % 2 == 0)
                glColor3ub(0, 100, 0);
            else
                glColor3ub(0, 130, 0);

            if (map[mapi][mapj] == 1) {
                glPushMatrix();
                glTranslatef((mapj + 0.5f), 0, -1 * (mapi + 0.5f));
                glutSolidCube(1);
                glPopMatrix();
                glPushMatrix();
                glTranslatef((mapj + 0.5f), 1, -1 * (mapi + 0.5f));
                glutSolidCube(1);
                glPopMatrix();
            }
            if (map[mapi][mapj] == 2) {
                glColor3ub(255, 255, 0);
                glPushMatrix();
                glTranslatef((mapj + 0.5f), -0.35f, -1 * (mapi + 0.5f));
                glutSolidCube(0.3f);
                glPopMatrix();
            }
        }
    }
}

void init() {
    glEnable(GL_DEPTH_TEST);

    //It's so confusing...
    //glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(1, 1, 1, 1);

    if (!loadOBJ("Example.obj")) {
        exit(1);
    }
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
    //Maze rendering: apply hidden surface removal algorithm
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    generatemap();

    // Draw the OBJ model if not found
    if (!objFound) {
        glPushMatrix();
        glTranslatef(objX, -0.35f, objZ);
        glColor3f(1.0f, 0.3f, 0.0f); // Color of the OBJ
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < obj_faces.size(); i += 3) {
            unsigned int vi1 = obj_faces[i] * 3;
            unsigned int vi2 = obj_faces[i + 1] * 3;
            unsigned int vi3 = obj_faces[i + 2] * 3;

            glVertex3f(obj_vertices[vi1], obj_vertices[vi1 + 1], obj_vertices[vi1 + 2]);
            glVertex3f(obj_vertices[vi2], obj_vertices[vi2 + 1], obj_vertices[vi2 + 2]);
            glVertex3f(obj_vertices[vi3], obj_vertices[vi3 + 1], obj_vertices[vi3 + 2]);
        }
        glEnd();
        glPopMatrix();
    }

    // Display "You won!" message if the object is found
    //why the window (or maybe the maze model) become darker?
    if (objFound) {
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

        glEnable(GL_LIGHTING);
        glEnable(GL_DEPTH_TEST);
    }

    glutSwapBuffers();
}

void specialkeys(int key, int x, int y) {
    float nx = gx;
    float nz = gz;
    float fraction = 0.05f;

    if (!objFound) {
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

    // Check if the player found the object
    float dx = gx - objX;
    float dz = gz - objZ;
    float distance = sqrt(dx * dx + dz * dz);

    if (distance < 0.3f) {
        objFound = true;
    }

    glutPostRedisplay();
}

void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
}

void mouse(int button, int state, int x, int y) {
    if (objFound && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
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
