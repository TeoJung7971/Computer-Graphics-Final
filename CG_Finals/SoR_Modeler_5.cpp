#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glut.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <string>

// Structure for storing points
struct Point3D {
    float x, y, z;
};

// Global variables
std::vector<Point3D> initialPoints;    // Input points
std::vector<Point3D> revolvedPoints;   // Revolved points for the surface
std::vector<Point3D> normals;          // Normals for each vertex
std::vector<unsigned int> indices;     // Indices for constructing polygons

int degree = 0;                        // Degree for revolution
bool degreeInputMode = false;          // Degree input mode flag
std::string degreeInput = "";          // Temporary string for degree input

GLsizei windowWidth = 800, windowHeight = 600; // Window dimensions
bool resultShown = false;              // Flag to track if revolution result is shown

double cameraDistance = 5.0;           // Camera distance for zooming

// Rendering mode 선택: 기본은 wireframe으로
enum RenderMode { WIREFRAME, SURFACE };
RenderMode renderMode = WIREFRAME;     
// Based axis mode 선택: 기본은 Y-axis로
enum RotationAxis { X_AXIS, Y_AXIS };
RotationAxis rotationAxis = Y_AXIS;


// Variables for rotation
float angle = 0.0f;                    // Rotation angle
int moving = 0;
int startX = 0;                        // Initial X position when dragging

// Function to add a 2D point on mouse click
void addPoint(float x, float y) {
    initialPoints.push_back({ x, y, 0.0f });
}

// Reset function
void reset() {
    initialPoints.clear();
    revolvedPoints.clear();
    normals.clear();
    indices.clear();
    degreeInput.clear();
    degree = 0;
    degreeInputMode = false;
    resultShown = false;
    cameraDistance = 5.0;
    renderMode = WIREFRAME;
    angle = 0.0f; // Reset rotation angle
}

// Function to generate a unique filename
std::string generateFilename() {
    std::ostringstream filename;
    filename << "revolved_object_" << std::time(nullptr) << ".obj";
    return filename.str();
}

// Function to revolve points around the Y-axis at intervals of the given degree
void revolvePoints() {
    if (degree <= 0 || degree >= 360) {
        std::cerr << "Invalid degree value. Must be between 1 and 359.\n";
        return;
    }

    revolvedPoints.clear();
    indices.clear();
    normals.clear();

    int steps = 360 / degree;
    float radiansInterval = degree * M_PI / 180.0f; //각도 계산을 위해 radian으로 변환

    // Generate vertices for the revolved surface
    for (const auto& point : initialPoints) {
        // steps = 360 / degree
        for (int s = 0; s < steps; ++s) {
            float angle = s * radiansInterval;
            float cosA = cos(angle);
            float sinA = sin(angle);

            float xNew, yNew, zNew;

            // 현재 선택된 중심 축에 따라서 회전 변환 공식 적용
            if (rotationAxis == Y_AXIS) {
                // Y축 회전
                // x' = x*cosA + z*sinA
                // y' = y
                // z' = -x*sinA + z*cosA
                xNew = point.x * cosA + point.z * sinA;
                yNew = point.y;
                zNew = -point.x * sinA + point.z * cosA;
            }
            else {
                // X축 회전
                // x' = x
                // y' = y*cosA - z*sinA
                // z' = y*sinA + z*cosA
                xNew = point.x;
                yNew = point.y * cosA - point.z * sinA;
                zNew = point.y * sinA + point.z * cosA;
            }

            revolvedPoints.push_back({
                xNew,
                yNew,
                zNew
                });
        }
    }

    // Generate indices for the triangles
    int totalRings = initialPoints.size();
    int pointsPerRing = steps;

    for (int i = 0; i < totalRings - 1; ++i) {
        for (int j = 0; j < pointsPerRing; ++j) {
            int current = i * pointsPerRing + j;
            int next = current + pointsPerRing;
            int nextJ = (j + 1) % pointsPerRing;

            int currentNextJ = i * pointsPerRing + nextJ;
            int nextNextJ = currentNextJ + pointsPerRing;

            // Triangle 1
            indices.push_back(current);
            indices.push_back(currentNextJ);
            indices.push_back(next);

            // Triangle 2
            indices.push_back(currentNextJ);
            indices.push_back(nextNextJ);
            indices.push_back(next);
        }
    }

    // Initialize normals
    normals.resize(revolvedPoints.size(), { 0.0f, 0.0f, 0.0f });

    // Compute normals
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int idx0 = indices[i];
        unsigned int idx1 = indices[i + 1];
        unsigned int idx2 = indices[i + 2];

        Point3D v0 = revolvedPoints[idx0];
        Point3D v1 = revolvedPoints[idx1];
        Point3D v2 = revolvedPoints[idx2];

        // Compute the normal using cross product
        float ux = v1.x - v0.x;
        float uy = v1.y - v0.y;
        float uz = v1.z - v0.z;

        float vx = v2.x - v0.x;
        float vy = v2.y - v0.y;
        float vz = v2.z - v0.z;

        float nx = uy * vz - uz * vy;
        float ny = uz * vx - ux * vz;
        float nz = ux * vy - uy * vx;

        // Accumulate normals
        normals[idx0].x += nx;
        normals[idx0].y += ny;
        normals[idx0].z += nz;

        normals[idx1].x += nx;
        normals[idx1].y += ny;
        normals[idx1].z += nz;

        normals[idx2].x += nx;
        normals[idx2].y += ny;
        normals[idx2].z += nz;
    }

    // Normalize the normals
    for (auto& n : normals) {
        float length = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
        if (length > 0.0f) {
            n.x /= length;
            n.y /= length;
            n.z /= length;
        }
    }

    resultShown = true;
}

// Function to save revolved points in OBJ format
void saveOBJFile() {
    std::ofstream outFile(generateFilename());
    if (outFile.is_open()) {
        // Write vertices
        for (const auto& p : revolvedPoints) {
            outFile << "v " << p.x << " " << p.y << " " << p.z << "\n";
        }

        // Write normals
        for (const auto& n : normals) {
            outFile << "vn " << n.x << " " << n.y << " " << n.z << "\n";
        }

        // Write faces 
        for (size_t i = 0; i < indices.size(); i += 3) {
            outFile << "f ";
            for (int j = 0; j < 3; ++j) {
                unsigned int idx = indices[i + j] + 1;
                outFile << idx << " ";
            }
            outFile << "\n";
        }
        outFile.close();
    }
    else {
        std::cerr << "Error opening file!\n";
    }
}

// Function to draw X, Y, Z axes spanning the screen
void drawAxes() {
    glDisable(GL_LIGHTING); // Disable lighting for axes
    glColor3f(0.0f, 0.0f, 0.0f); // Black color for axes

    glBegin(GL_LINES);

    // X-axis
    glVertex3f(-1000.0f, 0.0f, 0.0f);
    glVertex3f(1000.0f, 0.0f, 0.0f);

    // Y-axis
    glVertex3f(0.0f, -1000.0f, 0.0f);
    glVertex3f(0.0f, 1000.0f, 0.0f);

    // Z-axis
    glVertex3f(0.0f, 0.0f, -1000.0f);
    glVertex3f(0.0f, 0.0f, 1000.0f);

    glEnd();
}

// Function to render initial points
void renderInitialPoints() {
    glDisable(GL_LIGHTING); // Disable lighting for initial points
    glColor3f(0.0f, 0.0f, 1.0f); // Deep blue color for points
    glPointSize(8.0f);
    glBegin(GL_POINTS);
    for (const auto& p : initialPoints) {
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();
}

// Function to render the model based on the current rendering mode
void renderModel() {
    if (renderMode == SURFACE) {
        // Enable lighting for surface rendering
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        glShadeModel(GL_FLAT); // Use flat shading

        // Render surface
        glColor3f(1.0f, 0.0f, 0.0f); // Surface color (same as Code 1)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, &revolvedPoints[0]);
        glNormalPointer(GL_FLOAT, 0, &normals[0]);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &indices[0]);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);

        // Disable lighting after rendering
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHTING);

        // Render wireframe on top
        glColor3f(0.0f, 0.0f, 0.0f); // Black color for wireframe
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // Enable polygon offset to avoid z-fighting
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, &revolvedPoints[0]);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &indices[0]);
        glDisableClientState(GL_VERTEX_ARRAY);

        glDisable(GL_POLYGON_OFFSET_LINE);
    }
    else if (renderMode == WIREFRAME) {
        glDisable(GL_LIGHTING); // Disable lighting for wireframe
        // Render wireframe
        glColor3f(0.0f, 0.0f, 0.0f); // Black color for wireframe
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, &revolvedPoints[0]);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &indices[0]);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    // Render points on top
    glDisable(GL_LIGHTING); // Disable lighting for points
    glColor3f(0.0f, 0.0f, 1.0f); // Deep blue color for points
    glPointSize(7.0f);
    glBegin(GL_POINTS);
    for (const auto& p : revolvedPoints) {
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();

    // Reset polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Display function for OpenGL
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up perspective projection for 3D rendering
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (double)windowWidth / (double)windowHeight, 0.1, 1000.0); // Increase far clipping plane
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0.0, 0.0, cameraDistance,   // eye position
        0.0, 0.0, 0.0,   // look at position
        0.0, 1.0, 0.0);  // up vector

    // Draw axes without applying model transformations
    glDisable(GL_LIGHTING); // Disable lighting for axes
    drawAxes();

    // Now apply model transformations
    glPushMatrix();
    if (resultShown) {
        glRotatef(angle, 0.0f, 1.0f, 0.0f);
    }

    if (!resultShown) {
        renderInitialPoints();
    }
    else {
        renderModel();
    }

    glPopMatrix(); // Restore modelview matrix

    // Displaying degree input box if in input mode
    if (degreeInputMode) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, windowWidth, 0, windowHeight);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_LIGHTING); // Disable lighting for text rendering

        // Render prompt text at the top-right
        glColor3f(0.0f, 0.0f, 0.0f);
        int textX = windowWidth - 200;
        int textY = windowHeight - 30;
        glRasterPos2f(textX, textY);
        std::string displayText = "Enter a degree: ";
        for (char c : displayText) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }

        // Render the current input
        glRasterPos2f(textX, textY - 30);
        for (char c : degreeInput) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.0f, 0.0f);

    int modeTextX = 10;
    int modeTextY = windowHeight - 20;

    // "Current Mode"
    glRasterPos2f((GLfloat)modeTextX, (GLfloat)modeTextY);
    std::string modeTitle = "Current Mode";
    for (char c : modeTitle) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // RenderMode 표시
    std::string renderState = (renderMode == WIREFRAME) ? "- Wireframe" : "- Surface";
    glRasterPos2f((GLfloat)modeTextX, (GLfloat)(modeTextY - 30));
    for (char c : renderState) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // RotationAxis 표시
    std::string axisState = (rotationAxis == Y_AXIS) ? "- Y-axis" : "- X-axis";
    glRasterPos2f((GLfloat)modeTextX, (GLfloat)(modeTextY - 60));
    for (char c : axisState) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glFlush();
    glutSwapBuffers();
}

// Keyboard callback function for degree input
void keyboardCallback(unsigned char key, int x, int y) {
    if (degreeInputMode) {
        if (key == 13 || key == 10) { // Enter key
            if (!degreeInput.empty()) {
                degree = std::stoi(degreeInput);
                degreeInputMode = false;
                revolvePoints();
                saveOBJFile();
                glutPostRedisplay();
            }
        }
        else if (key == 8 || key == 127) { // Backspace key
            if (!degreeInput.empty()) degreeInput.pop_back();
        }
        else if (isdigit(key)) {
            degreeInput += key;
        }
        glutPostRedisplay();
    }
}

// Mouse callback function
void mouseCallback(int button, int state, int x, int y) {
    if (degreeInputMode) return; // Ignore mouse events during degree input

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            if (!resultShown) {
                // Read the projection and modelview matrix
                GLdouble modelview[16], projection[16];
                GLint viewport[4];
                glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
                glGetDoublev(GL_PROJECTION_MATRIX, projection);
                glGetIntegerv(GL_VIEWPORT, viewport);

                // Get the window Z position (winZ) at world Z=0
                double winX, winY, winZ;
                gluProject(0.0, 0.0, 0.0, modelview, projection, viewport, &winX, &winY, &winZ);

                // Convert window coordinates to OpenGL world coordinates
                GLdouble posX, posY, posZ;
                GLint realY = viewport[3] - y - 1; // Invert Y coordinate

                gluUnProject((GLdouble)x, (GLdouble)realY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

                addPoint((float)posX, (float)posY);
                glutPostRedisplay();
            }
            else {
                // Begin rotation
                moving = 1;
                startX = x;
            }
        }
        else if (state == GLUT_UP) {
            if (resultShown) {
                // End rotation
                moving = 0;
            }
        }
    }
    else if ((button == 3) || (button == 4)) // Mouse wheel event (requires freeglut)
    {
        if (state == GLUT_UP) return; // Disregard redundant GLUT_UP events

        if (button == 3) { // Scroll up
            cameraDistance -= 0.5;
            if (cameraDistance < 1.0) cameraDistance = 1.0;
        }
        else { // Scroll down
            cameraDistance += 0.5;
        }
        glutPostRedisplay();
    }
}

// Motion callback function for mouse movement
void motionCallback(int x, int y) {
    if (moving) {
        angle += (x - startX);
        startX = x;
        glutPostRedisplay();
    }
}

// Menu callback function
void menuCallback(int option) {
    switch (option) {
    case 1: // Set Revolution Degree
        if (!initialPoints.empty()) {
            degreeInputMode = true;
            degreeInput.clear();
            glutPostRedisplay(); // Force redisplay to show the prompt
        }
        else {
            std::cerr << "Please input points before setting the degree.\n";
        }
        break;
    case 2: // Reset
        reset();
        glutPostRedisplay();
        break;
    case 3: // Wireframe Rendering
        renderMode = WIREFRAME;
        glutPostRedisplay();
        break;
    case 4: // Surface Rendering
        renderMode = SURFACE;
        glutPostRedisplay();
        break;
    case 5: // Rotate about X
        rotationAxis = X_AXIS;
        glutPostRedisplay();
        break;
    case 6: // Rotate about Y
        rotationAxis = Y_AXIS;
        glutPostRedisplay();
        break;
    }
}

// Reshape callback function
void reshape(int w, int h) {
    if (h == 0) h = 1;
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
}

// Lighting initialization function (from Code 1)
void InitLight() {
    GLfloat mat_diffuse[] = { 0.5f, 0.4f, 0.3f, 1.0f };
    GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat mat_ambient[] = { 0.5f, 0.4f, 0.3f, 1.0f };
    GLfloat mat_shininess[] = { 15.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat light_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat light_position[] = { -3.0f, 6.0f, 3.0f, 0.0f };

    glShadeModel(GL_FLAT); // Use flat shading
    glEnable(GL_DEPTH_TEST);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    // Use GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH to enable depth testing
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Surface of Revolution Modeler");

    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glPointSize(8.0f);

    InitLight(); // Initialize lighting

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback); // Add motion callback for rotation

    // Create a submenu for rendering options
    int renderingMenu = glutCreateMenu(menuCallback);
    glutAddMenuEntry("Wireframe", 3);
    glutAddMenuEntry("Surface", 4);

    int axisMenu = glutCreateMenu(menuCallback);
    glutAddMenuEntry("Rotate about X", 5);
    glutAddMenuEntry("Rotate about Y", 6);

    // Create the main menu
    glutCreateMenu(menuCallback);
    glutAddMenuEntry("Set Revolution Degree", 1);
    glutAddMenuEntry("Reset", 2);
    glutAddSubMenu("Rendering", renderingMenu);
    glutAddSubMenu("Axis", axisMenu);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();
    return 0;
}
