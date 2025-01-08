#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <tuple>

class ZBuffer
{
private:
    int width;
    int height;
    std::vector<std::vector<float>> buffer;                                // Глубинный буфер
    std::vector<std::vector<std::tuple<float, float, float>>> framebuffer; // Цветовой буфер

public:
    ZBuffer(int width, int height);
    void clear();
    void setPixel(int x, int y, float depth, std::tuple<float, float, float> color);
    const std::vector<std::vector<std::tuple<float, float, float>>> &getFrameBuffer() const;
};

// Параметры сцены
float lightPos[4] = {2.0f, 4.0f, 2.0f, 1.0f}; // Позиция источника света
float lightAngleY = 45.0f;                    // Угол движения света по оси Y
float cameraAngle = 0.0f;
const int screenWidth = 1500, screenHeight = 1500;
ZBuffer zbuffer(screenWidth, screenHeight);

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
void renderFromZBuffer();

// Реализация методов
ZBuffer::ZBuffer(int width, int height) : width(width), height(height)
{
    buffer = std::vector<std::vector<float>>(height, std::vector<float>(width, std::numeric_limits<float>::infinity()));
    framebuffer = std::vector<std::vector<std::tuple<float, float, float>>>(height, std::vector<std::tuple<float, float, float>>(width, std::make_tuple(0.0f, 0.0f, 0.0f)));
}

void ZBuffer::clear()
{
    for (int y = 0; y < height; ++y)
    {
        std::fill(buffer[y].begin(), buffer[y].end(), std::numeric_limits<float>::infinity());
        std::fill(framebuffer[y].begin(), framebuffer[y].end(), std::make_tuple(0.0f, 0.0f, 0.0f));
    }
}

void ZBuffer::setPixel(int x, int y, float depth, std::tuple<float, float, float> color)
{
    if (x >= 0 && x < width && y >= 0 && y < height && depth < buffer[y][x])
    {
        buffer[y][x] = depth;
        framebuffer[y][x] = color;
    }
}

const std::vector<std::vector<std::tuple<float, float, float>>> &ZBuffer::getFrameBuffer() const
{
    return framebuffer;
}

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

void updateCamera()
{
    float radius = 13.0f; // Расстояние камеры от центра
    float height = 5.0f;  // Высота камеры

    glLoadIdentity();
    gluLookAt(
        radius * cos(cameraAngle), height, radius * sin(cameraAngle), // Положение камеры
        0.0, 0.0, 0.0,                                                // Точка, на которую смотрит камера
        0.0, 1.0, 0.0                                                 // Вектор вверх
    );
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

void renderFromZBuffer()
{
    glBegin(GL_POINTS);
    for (int y = 0; y < screenHeight; ++y)
    {
        for (int x = 0; x < screenWidth; ++x)
        {
            auto [r, g, b] = zbuffer.getFrameBuffer()[y][x];

            // Преобразование координат пикселя (x, y) в диапазон OpenGL (-1, 1)
            float normalizedX = (x / float(screenWidth)) * 2.0f - 1.0f;
            float normalizedY = 1.0f - (y / float(screenHeight)) * 2.0f;

            glColor3f(r, g, b);                   // Установка цвета из кадрового буфера
            glVertex2f(normalizedX, normalizedY); // Добавление точки на экран
        }
    }
    glEnd();
}

// Функция для отображения сцены
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Очистка цветного и глубинного буфера
    zbuffer.clear();                                    // Очистка Z-буфера

    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glEnable(GL_DEPTH_TEST);

    updateCamera();
    // updateLight()

    // Источник света
    glPushMatrix();
    glTranslatef(lightPos[0], lightPos[1], lightPos[2]);
    glColor3f(1.0f, 1.0f, 0.0f); // Жёлтый свет
    glutSolidSphere(0.1, 16, 16);
    glPopMatrix();

    // Рисование объектов
    drawPlane();
    drawCube();
    drawPyramid();

    // Отображение результата из Z-буфера
    renderFromZBuffer();

    glFlush();
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
    int planeSize = 500; // Размер плоскости
    float planeY = 0.0f; // Плоскость на уровне Y=0

    for (int x = -planeSize; x < planeSize; ++x)
    {
        for (int z = -planeSize; z < planeSize; ++z)
        {
            // Рассчитаем координаты пикселя в буфере
            int screenX = x + screenWidth / 2;
            int screenZ = z + screenHeight / 2;

            // Устанавливаем пиксель в Z-буфер
            zbuffer.setPixel(screenX, screenZ, planeY, {0.6f, 0.6f, 0.6f}); // Серый цвет плоскости
        }
    }
}

// Функция для рисования куба
void drawCube()
{
    int cubeSize = 50;      // Половина длины стороны куба
    float cubeYBase = 0.0f; // Куб начинается с уровня Y=0

    for (int x = -cubeSize; x < cubeSize; ++x)
    {
        for (int y = 0; y < cubeSize * 2; ++y)
        {
            for (int z = -cubeSize; z < cubeSize; ++z)
            {
                // Рассчитаем координаты пикселя в буфере
                int screenX = x + screenWidth / 2;
                int screenZ = z + screenHeight / 2;

                // Глубина определяется уровнем Y
                float depth = cubeYBase + y / float(cubeSize * 2);

                // Устанавливаем пиксель в Z-буфер
                zbuffer.setPixel(screenX, screenZ, depth, {0.5f, 0.2f, 0.8f}); // Фиолетовый цвет куба
            }
        }
    }
}

// Функция для рисования пирамиды
void drawPyramid()
{
    int baseSize = 50; // Половина длины основания пирамиды
    int height = 100;  // Высота пирамиды

    for (int x = -baseSize; x <= baseSize; ++x)
    {
        for (int z = -baseSize; z <= baseSize; ++z)
        {
            // Рассчитаем высоту в зависимости от расстояния до центра основания
            int absX = std::abs(x);
            int absZ = std::abs(z);
            if (absX + absZ <= baseSize)
            {
                int y = height - (absX + absZ); // Высота уменьшается к краям

                // Рассчитаем координаты пикселя в буфере
                int screenX = x + screenWidth / 2;
                int screenZ = z + screenHeight / 2;

                // Глубина определяется уровнем Y
                float depth = y / float(height);

                // Устанавливаем пиксель в Z-буфер
                zbuffer.setPixel(screenX, screenZ, depth, {0.2f, 0.8f, 0.4f}); // Зелёный цвет пирамиды
            }
        }
    }
}

// Основная функция
int main(int argc, char **argv)
{
    // Инициализация GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Shadows and Z-buffering: Plane, Cube and Pyramid");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Включаем тест глубины
    glEnable(GL_DEPTH_TEST);

    // Инициализация GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        std::cerr << "Ошибка инициализации GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // Настройка OpenGL
    // glEnable(GL_DEPTH_TEST);
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
