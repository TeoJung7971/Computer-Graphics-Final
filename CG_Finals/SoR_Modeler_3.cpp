#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glut.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <string>

// Struct to store 3D points
struct Point3D {
    float x, y, z;
};

// Global variables
std::vector<Point3D> initialPoints;    // Points input by the user
std::vector<Point3D> revolvedPoints;   // Revolved points for the surface
std::vector<unsigned int> indices;     // Indices for constructing polygons
int degree = 0;                        // Degree for revolution
bool degreeInputMode = false;          // Degree input mode flag
std::string degreeInput = "";          // Temporary string for degree input
GLsizei windowWidth = 800, windowHeight = 600; // Window dimensions
bool resultShown = false;              // Flag to track if revolution result is shown
double cameraDistance = 5.0;           // Camera distance for zooming

// Function to add a 2D point on mouse click
void addPoint(float x, float y) {
    initialPoints.push_back({ x, y, 0.0f });
}

// Function to reset all data
void reset() {
    initialPoints.clear();
    revolvedPoints.clear();
    indices.clear();
    degreeInput.clear();
    degree = 0;
    degreeInputMode = false;
    resultShown = false;
    cameraDistance = 5.0; // Reset camera distance
}

// Function to create a unique filename using the current timestamp
std::string generateUniqueFilename() {
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

    int steps = 360 / degree;
    float radiansInterval = degree * M_PI / 180.0f;

    // Generate vertices for the revolved surface
    for (const auto& point : initialPoints) {
        for (int s = 0; s < steps; ++s) {
            float angle = s * radiansInterval;
            float cosA = cos(angle);
            float sinA = sin(angle);

            revolvedPoints.push_back({
                point.x * cosA,
                point.y,
                point.x * sinA
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
            indices.push_back(next);
            indices.push_back(currentNextJ);

            // Triangle 2
            indices.push_back(currentNextJ);
            indices.push_back(next);
            indices.push_back(nextNextJ);
        }
    }

    resultShown = true;
}

// Function to save revolved points in OBJ format
void saveOBJFile() {
    std::ofstream outFile(generateUniqueFilename());
    if (outFile.is_open()) {
        for (const auto& p : revolvedPoints) {
            outFile << "v " << p.x << " " << p.y << " " << p.z << "\n";
        }

        for (size_t i = 0; i < indices.size(); i += 3) {
            outFile << "f " << indices[i] + 1 << " " << indices[i + 1] + 1 << " " << indices[i + 2] + 1 << "\n";
        }
        outFile.close();
    }
    else {
        std::cerr << "Error opening file!\n";
    }
}

// Function to draw X, Y, Z axes spanning the screen
void drawAxes() {
    glColor3f(0.0f, 0.0f, 0.0f); // Black color for axes
    glBegin(GL_LINES);

    // X-axis
    glVertex3f(-2.0f, 0.0f, 0.0f);
    glVertex3f(2.0f, 0.0f, 0.0f);

    // Y-axis
    glVertex3f(0.0f, -2.0f, 0.0f);
    glVertex3f(0.0f, 2.0f, 0.0f);

    // Z-axis
    glVertex3f(0.0f, 0.0f, -2.0f);
    glVertex3f(0.0f, 0.0f, 2.0f);

    glEnd();
}

// Function to render initial points
void renderInitialPoints() {
    glColor3f(0.0f, 0.0f, 1.0f); // Deep blue color for points
    glPointSize(8.0f);
    glBegin(GL_POINTS);
    for (const auto& p : initialPoints) {
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();
}

// Function to render revolved points as a wireframe and points
void renderWireframe() {
    // Render points
    glColor3f(1.0f, 0.2f, 0.0f); // Green color for points
    glPointSize(7.0f); // 
    glBegin(GL_POINTS);
    for (const auto& p : revolvedPoints) {
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();

    // Render wireframe
    glColor3f(0.1f, 0.1f, 0.1f); // Red color for wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, &revolvedPoints[0]);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, &indices[0]);
    glDisableClientState(GL_VERTEX_ARRAY);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// Display function for OpenGL
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up perspective projection for 3D rendering
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (double)windowWidth / (double)windowHeight, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(0.0, 0.0, cameraDistance,   // eye position
        0.0, 0.0, 0.0,   // look at position
        0.0, 1.0, 0.0);  // up vector

    drawAxes();

    if (!resultShown) {
        renderInitialPoints();
    }
    else {
        renderWireframe();
    }

    // Displaying degree input box if in input mode
    if (degreeInputMode) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, windowWidth, 0, windowHeight);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

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

    glFlush();
    glutSwapBuffers();
}

// Keyboard callback function for degree input
void keyboardCallback(unsigned char key, int x, int y) {
    if (degreeInputMode) {
        if (key == 13 || key == 10) { // Enter key
            degree = std::stoi(degreeInput);
            degreeInputMode = false;
            revolvePoints();
            saveOBJFile();
            glutPostRedisplay();
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

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // If result is shown, reset for new input
        if (resultShown) {
            reset();
        }

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
    }
}

// Reshape callback function
void reshape(int w, int h) {
    if (h == 0) h = 1;
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Surface of Revolution Modeler");

    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glPointSize(8.0f);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardCallback);
    glutMouseFunc(mouseCallback);

    glutCreateMenu(menuCallback);
    glutAddMenuEntry("Set Revolution Degree", 1);
    glutAddMenuEntry("Reset", 2);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();
    return 0;
}
