#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 1.0f, 12.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);

    // shaders
    Shader objectShader("resources/shaders/object.vs", "resources/shaders/object.fs");
    Shader floorShader("resources/shaders/floor.vs", "resources/shaders/floor.fs");
    Shader lightShader("resources/shaders/light_source.vs", "resources/shaders/light_source.fs");

    // models
    Model tableModel(FileSystem::getPath("resources/objects/dining_table/dining_table.obj"));
    Model cakeModel(FileSystem::getPath("resources/objects/slice_of_cake/cake.obj"));
    Model lightModel(FileSystem::getPath("resources/objects/light/light.obj"));

    // floor
    float floorVertices[] = {
            // positions          //normals            // texture coords
            1.0f,  0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    10.0f, 10.0f,
            1.0f, 0.0f, -1.0f,    0.0f, 1.0f, 0.0f,    10.0f, 0.0f,
            -1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 10.0f,
            -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,    0.0f, 0.0f
    };
    unsigned int floorIndices[] = {
            0, 1, 3, // first triangle
            0, 2, 3  // second triangle
    };
    unsigned int floorVBO, floorVAO, floorEBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &floorEBO);

    glBindVertexArray(floorVAO);

    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int floorTexture = TextureFromFile("floor_diffuse.png", "resources/objects/floor");


    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        objectShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);

        // table
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
        objectShader.setMat4("model", model);
        tableModel.Draw(objectShader);

        // TODO: Napraviti funkciju za cake i light
        // cake
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f,-2.15f, 3.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.5f,-2.15f, 3.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f,-2.15f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.5f,-2.15f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1.5f,-2.15f, -3.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.5f,-2.15f, -3.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.5f,-2.15f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
        objectShader.setMat4("model", model);
        cakeModel.Draw(objectShader);


        // light
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.0f, -3.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        lightShader.setMat4("model", model);
        lightModel.Draw(lightShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        lightShader.setMat4("model", model);
        lightModel.Draw(lightShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.0f, 3.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        lightShader.setMat4("model", model);
        lightModel.Draw(lightShader);

        //floor
        floorShader.use();
        floorShader.setMat4("projection", projection);
        floorShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
        model = glm::scale(model, glm::vec3(20.0f, 0.0f, 20.0f));
        floorShader.setMat4("model", model);
        glBindVertexArray(floorVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &floorVAO);
    glDeleteBuffers(1, &floorVBO);
    glDeleteBuffers(1, &floorEBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}
