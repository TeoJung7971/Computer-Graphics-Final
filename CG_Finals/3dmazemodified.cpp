#include <GL/glut.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <ctime>

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
    {1,0,1,1,1,0,1,0,2,0,1,0,1,1,1,1,1},
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

void generatemap()
{
    for (int mapi = 0; mapi < 17; ++mapi) {
        for (int mapj = 0; mapj < 17; ++mapj) {
            if ((mapi + mapj) % 2 == 0)
                glColor3ub(0, 102, 0);
            else
                glColor3ub(0, 153, 0);

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

void init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glClearColor(1, 1, 1, 1);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    double aspect = (double)viewport[2] / (double)viewport[3];
    gluPerspective(60, aspect, 0.1, 100);

    gluLookAt(gx, 0.5, gz, gx + sin(angle), 0.5, gz - cos(angle), 0, 1, 0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    generatemap();

    glutSwapBuffers();
}

void specialkeys(int key, int x, int y)
{
    float fraction = 0.05f;
    switch (key) {
    case GLUT_KEY_UP:
        gx += fraction * sin(angle);
        gz -= fraction * cos(angle);
        break;
    case GLUT_KEY_DOWN:
        gx -= fraction * sin(angle);
        gz += fraction * cos(angle);
        break;
    case GLUT_KEY_LEFT:
        angle -= M_PI / 180;
        break;
    case GLUT_KEY_RIGHT:
        angle += M_PI / 180;
        break;
    }
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(1280, 720);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutCreateWindow("3D MAZE Explorer");
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(specialkeys);

    init();
    glutMainLoop();
    return 0;
}
