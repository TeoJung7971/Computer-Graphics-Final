#include <GL/glut.h>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>

// Struct to store 3D points
struct Point3D {
    float x, y, z;
};

// Global variables
std::vector<Point3D> initialPoints;    // Points input by the user
std::vector<Point3D> revolvedPoints;   // Revolved points for the surface
int degree = 0;                        // Degree for revolution
bool degreeInputMode = false;          // Degree input mode flag
std::string degreeInput = "";          // Temporary string for degree input
GLsizei windowWidth = 500, windowHeight = 500; // Window dimensions
bool resultShown = false;               // Flag to track if revolution result is shown

// Function to add a 2D point on mouse click
void addPoint(float x, float y) {
    initialPoints.push_back({ x, y, 0.0f });
}

// Function to reset all data
void reset() {
    initialPoints.clear();
    revolvedPoints.clear();
    degreeInput.clear();
    degree = 0;
    degreeInputMode = false;
    resultShown = false;
}

// Function to create a unique filename using the current timestamp
std::string generateUniqueFilename() {
    std::ostringstream filename;
    filename << "revolved_points_" << std::time(nullptr) << ".txt";
    return filename.str();
}

// Function to revolve points around the Y-axis at intervals of the given degree
void revolvePoints() {
    revolvedPoints.clear();
    float radiansInterval = degree * M_PI / 180.0f;

    for (const auto& point : initialPoints) {
        for (float angle = 0; angle < 2 * M_PI; angle += radiansInterval) {
            revolvedPoints.push_back({
                point.x * cos(angle),
                point.y,
                point.x * sin(angle)
                });
        }
    }
    resultShown = true;
}

// Function to save revolved points to a unique file
void savePointsToFile() {
    std::ofstream outFile(generateUniqueFilename());
    if (outFile.is_open()) {
        for (const auto& p : revolvedPoints) {
            outFile << p.x << " " << p.y << " " << p.z << "\n";
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
    glColor3f(0.0f, 0.0f, 1.0f); // Deep blue color for points
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    for (const auto& p : initialPoints) {
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();
}

// Function to render revolved points only
void renderRevolvedPoints() {
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for revolved points
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    for (const auto& p : revolvedPoints) {
        glVertex3f(p.x, p.y, p.z);
    }
    glEnd();
}

// Display function for OpenGL
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    drawAxes();
    renderInitialPoints();

    if (resultShown) {
        renderRevolvedPoints();
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
        glRasterPos2f(windowWidth - 200, windowHeight - 30);
        std::string displayText = "Enter a degree: ";
        for (char c : displayText) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }

        // Render the current input
        glRasterPos2f(windowWidth - 200, windowHeight - 60);
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
        if (key == 13) { // Enter key
            degree = std::stoi(degreeInput);
            degreeInputMode = false;
            revolvePoints();
            savePointsToFile();
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
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (resultShown) reset();  // Reset previous result on new click

        float glX = (x - windowWidth / 2) / (windowWidth / 4.0f);
        float glY = (windowHeight / 2 - y) / (windowHeight / 4.0f);

        if (windowWidth> 600) {
            glX = (2.0f * x / windowWidth - 1.0f) * (windowWidth / 500.0f);
            glY = (1.0f - 2.0f * y / windowHeight) * (windowHeight / 500.0f);
        }
            
        addPoint(glX, glY);

        glutPostRedisplay();
    }
}

// Menu callback function to set degree input mode or reset
void menuCallback(int option) {
    if (option == 1) { // Set revolution degree
        degreeInputMode = true;
        degreeInput.clear();
        glutPostRedisplay();
    }
    else if (option == 2) { // Reset
        reset();
        glutPostRedisplay();
    }
}

// Resize function to maintain aspect ratio
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / h, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);

    windowWidth = w;
    windowHeight = h;
}

// Initialization of OpenGL
void initOpenGL() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Set background to white
    glEnable(GL_DEPTH_TEST);
}

// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Surface of Revolution Modeler");

    initOpenGL();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardCallback);
    glutMouseFunc(mouseCallback);

    // Creating a menu with options to set the degree for revolution and reset the screen
    glutCreateMenu(menuCallback);
    glutAddMenuEntry("Set Revolution Degree", 1);
    glutAddMenuEntry("Reset", 2);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();
    return 0;
}
