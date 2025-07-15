#define STB_EASY_FONT_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <utility>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <stb_easy_font.h>
#include <shlwapi.h>
#include <filesystem>
#include <fstream>

int score = 0;
int highScore = 0;

void loadHighScore() {
    std::ifstream file("highscore.txt");
    if (file.is_open()) {
        file >> highScore;
        std::cout << "Loaded high score: " << highScore << std::endl;
        file.close();
    } else {
        std::ofstream createFile("highscore.txt");
        createFile << "0";
        createFile.close();
        highScore = 0;
        std::cout << "High score file not found. Created new one." << std::endl;
    }
}

void saveHighScore() {
    std::ofstream file("highscore.txt");
    file << highScore;
    file.close();
}

const int GRID_COLS = 50;
const int GRID_ROWS = 50;

std::vector<std::pair<int, int>> snake = {
    {25, 25} ,{24, 25}, {23,25}
};

std::pair<int, int> food = {10, 10};

int dirX = 1;
int dirY = 0;

bool isGameOver = false;

std::pair<float, float> gridToOpenGL(int x, int y) {
    float cellWidth = 2.0f / GRID_COLS;
    float cellHeight = 2.0f / GRID_ROWS;
    float glX = -1.0f + x * cellWidth;
    float glY = -1.0f + y * cellHeight;
    return { glX, glY };
}

void framebuffer_size_allowed(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void respawnFood() {
    while(true) {
        int fx = rand() % GRID_COLS;
        int fy = rand() % GRID_ROWS;

        bool onSnake = false;
        for (const auto& segment : snake) {
            if (segment.first == fx && segment.second == fy) {
                onSnake = true;
                break;
            }
        }

        if (!onSnake) {
            food = {fx, fy};
            break;
        }
    }
}

void renderText(const char* text, float x, float y, GLuint textVBO, GLuint textVAO) {
    char buffer[99999];
    int num_quads = stb_easy_font_print(x, y, (char*)text, nullptr, buffer, sizeof(buffer));

    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, num_quads * 4 * sizeof(float), buffer, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, (void*)0);
    glBindVertexArray(0);
}


void resetGame() {
    snake = {{25, 25}, {24, 25}, {23, 25}};
    dirX = 1;
    dirY = 0;
    respawnFood();
    score = 0;
    isGameOver = false;
}

const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    uniform mat4 transform;

    void main() {
        gl_Position = transform * vec4(aPos, 0.0, 1.0);
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 color;

    void main() {
        FragColor = color;
    }
)glsl";

int main() {
    std::cout << "Current path: " << std::filesystem::current_path() << std::endl;

    if (!glfwInit()) {
        std::cerr << "Failer to Initialize GLFW!\n";
        return -1;
    }

    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    PathRemoveFileSpecA(path);
    SetCurrentDirectoryA(path);

    std::cout << "Working directory set to exe folder: " << path << std::endl;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Snake", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_allowed);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!\n";
        return -1;
    }

    glViewport(0, 0, 800, 600);
    glClearColor(0.1f, 0.1f, 0.1f, 1);

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    } else {
        std::cout << "SUCCESS::SHADER::VERTEX::COMPILATION" << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    } else {
        std::cout << "SUCCESS::SHADER::FRAGMENT::COMPILATION" << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED" << infoLog << std::endl;
    } else {
        std::cout << "SUCCESS::SHADER::PROGRAM::LINKING" << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float baseSquare[] = {
        -0.1f, -0.1f,
         0.1f, -0.1f,
         0.1f,  0.1f,
        -0.1f,  0.1f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(baseSquare), baseSquare, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    GLuint textVAO, textVBO;
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);

    int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    if (transformLoc == -1) {
        std::cerr << "Warning: 'transform' uniform not found in shader.\n";
    }

    int colorLoc = glGetUniformLocation(shaderProgram, "color");
    if (colorLoc == -1) {
        std::cerr << "Warning: 'color' uniform not found in shader.\n";
    }

    srand(static_cast<unsigned int>(time(nullptr)));
    respawnFood();
    loadHighScore();

    auto lastMove = std::chrono::steady_clock::now();
    float moveInterval = 0.15f;

    try {
        while (!glfwWindowShouldClose(window)) {
            glClear(GL_COLOR_BUFFER_BIT);

            if (!isGameOver) {
                if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && dirY != -1) {
                    dirX = 0;
                    dirY = 1;
                }

                if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && dirY != 1) {
                    dirX = 0;
                    dirY = -1;
                }

                if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && dirX != 1) {
                    dirX = -1;
                    dirY = 0;
                }

                if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && dirX != -1) {
                    dirX = 1;
                    dirY = 0;
                }
            }

            if (!isGameOver) {
                auto now = std::chrono::steady_clock::now();
                float elapsed = std::chrono::duration<float>(now - lastMove).count();
                if (elapsed >= moveInterval) {
                    int newX = snake[0].first + dirX;
                    int newY = snake[0].second + dirY;

                    if (newX < 0 || newX >= GRID_COLS || newY < 0 || newY >= GRID_ROWS) {
                        isGameOver = true;
                        if (score > highScore) {
                            highScore = score;
                            saveHighScore();
                        }
                    } else {
                        bool hitSelf = false;
                        for (const auto& segment : snake) {
                            if (segment.first == newX && segment.second == newY) {
                                hitSelf = true;
                                break;
                            }
                        }

                        if (hitSelf) {
                            isGameOver = true;
                            if (score > highScore) {
                                highScore = score;
                                saveHighScore();    
                            }
                        } else {
                            snake.insert(snake.begin(), {newX, newY});

                            if (newX == food.first && newY == food.second) {
                                score++;
                                respawnFood();
                            } else {
                                snake.pop_back();
                            }
                            
                            lastMove = now;
                        }
                    }
                }
            }
            
            glUseProgram(shaderProgram);
            glBindVertexArray(VAO);

            glUniform4f(colorLoc, 0.0f, 1.0f, 0.0f, 1.0f);
            for (auto& segment : snake) {
                auto [glX, glY] = gridToOpenGL(segment.first, segment.second);
                float cellW = 2.0f / GRID_COLS;
                float cellH = 2.0f / GRID_ROWS;

                float transform[16] = {
                    cellW, 0.0f, 0.0f, 0.0f,
                    0.0f, cellH, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    glX, glY, 0.0f, 1.0f
                };

                glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }

            glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
            auto [fx, fy] = food;
            auto [glX, glY] = gridToOpenGL(fx, fy);
            float cellW = 2.0f / GRID_COLS;
            float cellH = 2.0f / GRID_ROWS;

            float transform[16] = {
                cellW, 0.0f, 0.0f, 0.0f,
                0.0f, cellH, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                glX, glY, 0.0f, 1.0f
            };

            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            char scoreText[64];
            sprintf(scoreText, "Score: %d", score);
            renderText(scoreText, 10, 580, textVBO, textVAO);

            if (isGameOver) {
                std::cout << "Game Over! Score: " << score << std::endl;
                char highScoreText[64];
                sprintf(highScoreText, "High Score: %d", highScore);
                renderText("GAME OVER", 300, 300, textVBO, textVAO);
                renderText("Press R to Restart", 280, 270, textVBO, textVAO);
                renderText(highScoreText, 300, 240, textVBO, textVAO);
            }

            if (isGameOver && glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                resetGame();
            }

            glfwSwapBuffers(window);
            glfwPollEvents();    
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }

    cleanup:
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteProgram(shaderProgram);

        glfwDestroyWindow(window);
        glfwTerminate();
        std::cerr << "Bye Bye" << std::endl;

        return 0;
}