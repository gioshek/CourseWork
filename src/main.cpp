#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include <iostream>

// Параметры сцены
float lightPos[4] = {2.0f, 4.0f, 2.0f, 1.0f}; // Позиция источника света
float lightAngleY = 45.0f;                    // Угол движения света по оси Y
float cameraAngle = 0.0f;

// Прототипы функций
void display();
void reshape(int width, int height);
void keyboard(int key, int x, int y);
void drawPlane();
void drawCube();
void drawPyramid();
void drawShadow(float *lightPos, const float *groundPlane);
void updateCamera();
void keyboard(unsigned char key, int x, int y);
void updateLight();
void drawSceneWithShadows();

// Z-буфер
const int screenWidth = 1500, screenHeight = 1500;

// Матрица проекции тени
void calculateShadowMatrix(GLfloat *shadowMatrix, const GLfloat *light, const GLfloat *plane)
{
    GLfloat dot = plane[0] * light[0] + plane[1] * light[1] + plane[2] * light[2] + plane[3] * light[3];
    for (int i = 0; i < 16; ++i)
        shadowMatrix[i] = 0.0f;

    shadowMatrix[0] = dot - light[0] * plane[0];
    shadowMatrix[4] = -light[0] * plane[1];
    shadowMatrix[8] = -light[0] * plane[2];
    shadowMatrix[12] = -light[0] * plane[3];

    shadowMatrix[1] = -light[1] * plane[0];
    shadowMatrix[5] = dot - light[1] * plane[1];
    shadowMatrix[9] = -light[1] * plane[2];
    shadowMatrix[13] = -light[1] * plane[3];

    shadowMatrix[2] = -light[2] * plane[0];
    shadowMatrix[6] = -light[2] * plane[1];
    shadowMatrix[10] = dot - light[2] * plane[2];
    shadowMatrix[14] = -light[2] * plane[3];

    shadowMatrix[3] = -light[3] * plane[0];
    shadowMatrix[7] = -light[3] * plane[1];
    shadowMatrix[11] = -light[3] * plane[2];
    shadowMatrix[15] = dot - light[3] * plane[3];
}

// Перекрестные тени
void drawSceneWithShadows()
{
    // Определяем тени куба на пирамиде
    GLfloat shadowMatrix[16];
    GLfloat groundPlane[4] = {0.0f, 1.0f, 0.0f, 0.0f};
    calculateShadowMatrix(shadowMatrix, lightPos, groundPlane);

    // Отображаем тень куба
    glPushMatrix();
    glMultMatrixf(shadowMatrix);
    glColor3f(0.2f, 0.2f, 0.2f); // Тень куба
    drawCube();
    glPopMatrix();

    // Отображаем тень пирамиды
    glPushMatrix();
    glMultMatrixf(shadowMatrix);
    glColor3f(0.2f, 0.2f, 0.2f); // Тень пирамиды
    drawPyramid();
    glPopMatrix();

    // Отображение объектов
    drawCube();
    drawPyramid();
}

// Функция камеры
void updateCamera()
{
    float radius = 13.0f;
    float height = 5.0f;

    glLoadIdentity();
    gluLookAt(radius * cos(cameraAngle), height, radius * sin(cameraAngle), // Положение камеры
              0.0, 0.0, 0.0,                                                // Точка, на которую смотрит камера
              0.0, 1.0, 0.0);                                               // Вектор вверх
}

// ФУнкция движения камеры
void keyboard(unsigned char key, int x, int y)
{
    if (key == 'a')
        cameraAngle -= 0.05f;
    if (key == 'd')
        cameraAngle += 0.05f;
    glutPostRedisplay();
}

// Функиця плавного движения света
void updateLight()
{
    lightAngleY += 0.5f; // Скорость движения света
    if (lightAngleY > 360.0f)
        lightAngleY -= 360.0f;

    lightPos[0] = 5.0f * cos(lightAngleY * M_PI / 180.0f);
    lightPos[2] = 5.0f * sin(lightAngleY * M_PI / 180.0f);
    glutPostRedisplay();
}

// Функция для отображения сцены
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    updateCamera();
    // updateLight();

    // Задаём источник света
    glPushMatrix();
    glTranslatef(lightPos[0], lightPos[1], lightPos[2]);
    glColor3f(1.0f, 1.0f, 0.0f);
    glutSolidSphere(0.1, 16, 16);
    glPopMatrix();

    // Определяем плоскость для теней
    GLfloat groundPlane[4] = {0.0f, 1.0f, 0.0f, 0.0f};
    GLfloat shadowMatrix[16];
    calculateShadowMatrix(shadowMatrix, lightPos, groundPlane);
    drawSceneWithShadows();

    // Рисуем плоскость (земля)
    drawPlane();

    // Отображаем тень куба
    glPushMatrix();
    glMultMatrixf(shadowMatrix);
    glColor3f(0.2f, 0.2f, 0.2f); // Тени
    drawCube();
    glPopMatrix();

    // Отображаем тень пирамиды
    glPushMatrix();
    glMultMatrixf(shadowMatrix);
    glColor3f(0.2f, 0.2f, 0.2f);
    drawPyramid();
    glPopMatrix();

    // Рисуем куб
    drawCube();

    // Рисуем пирамиду
    drawPyramid();

    glutSwapBuffers();
}

// Функция для изменения размеров окна
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / height, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(5.0, 5.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

// Управление клавиатурой для изменения угла источника света
void keyboard(int key, int x, int y)
{
    if (key == GLUT_KEY_LEFT)
    {
        lightAngleY -= 5.0f;
    }
    else if (key == GLUT_KEY_RIGHT)
    {
        lightAngleY += 5.0f;
    }
    else if (key == 'a')
    {
        cameraAngle -= 5.0f;
    }
    else if (key == 'd')
    {
        cameraAngle += 5.0f;
    }

    lightPos[0] = 5.0f * cos(lightAngleY * M_PI / 180.0f);
    lightPos[2] = 5.0f * sin(lightAngleY * M_PI / 180.0f);

    glutPostRedisplay();
}

// Функция для рисования плоскости
void drawPlane()
{
    glPushMatrix();
    glColor3f(0.6f, 0.6f, 0.6f); // Цвет земли
    glBegin(GL_QUADS);
    glVertex3f(-5.0f, 0.0f, -5.0f);
    glVertex3f(5.0f, 0.0f, -5.0f);
    glVertex3f(5.0f, 0.0f, 5.0f);
    glVertex3f(-5.0f, 0.0f, 5.0f);
    glEnd();
    glPopMatrix();
}

// Функция для рисования куба
void drawCube()
{
    glPushMatrix();
    glColor3f(0.5f, 0.2f, 0.8f); // Цвет куба
    glTranslatef(0.0f, 0.5f, 0.0f);
    glutSolidCube(1.0);
    glPopMatrix();
}

// Функция для рисования пирамиды
// Функция для рисования пирамиды
void drawPyramid()
{
    glPushMatrix();
    glColor3f(0.2f, 0.8f, 0.4f); // Цвет пирамиды
    glTranslatef(2.0f, 0.5f, 2.0f);

    // Основание
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, 0.0f, -0.5f);
    glVertex3f(0.5f, 0.0f, -0.5f);
    glVertex3f(0.5f, 0.0f, 0.5f);
    glVertex3f(-0.5f, 0.0f, 0.5f);
    glEnd();

    // Боковые стороны
    glBegin(GL_TRIANGLES);
    glVertex3f(-0.5f, 0.0f, -0.5f);
    glVertex3f(0.5f, 0.0f, -0.5f);
    glVertex3f(0.0f, 1.0f, 0.0f);

    glVertex3f(0.5f, 0.0f, -0.5f);
    glVertex3f(0.5f, 0.0f, 0.5f);
    glVertex3f(0.0f, 1.0f, 0.0f);

    glVertex3f(0.5f, 0.0f, 0.5f);
    glVertex3f(-0.5f, 0.0f, 0.5f);
    glVertex3f(0.0f, 1.0f, 0.0f);

    glVertex3f(-0.5f, 0.0f, 0.5f);
    glVertex3f(-0.5f, 0.0f, -0.5f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glEnd();

    glPopMatrix();
}

// Основная функция
int main(int argc, char **argv)
{
    // Инициализация GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Shadows and Z-buffering: Plane, Cube and Pyramid");

    // Инициализация GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "Ошибка инициализации GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // Настройка OpenGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Цвет фона (тёмно-серый)

    // Установка функций обратного вызова
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(keyboard);  // Управление клавишами (стрелками)
    glutKeyboardFunc(keyboard); // Управление клавишами (a и d)

    // Запуск основного цикла GLUT
    glutMainLoop();

    return 0;
}
