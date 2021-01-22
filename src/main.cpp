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

void draw_cake(Model& model, Shader& shader, const glm::vec3& translation_vec);
void set_light_bulb(Model& lightModel, Shader& lightShader, glm::vec3& pointLightPositions, float angle, const glm::vec3& translation_vec);
void set_spot_light(Shader& shader, Camera& camera);
void set_point_light(Shader& objectShader, glm::vec3& point_light_position, int i, float point_light_linear, float point_light_quadratic);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod);
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

bool isSpotlightActivated = false;
bool effect = false;    // da li stavljamo efekat (grayscale)

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
    glfwSetKeyCallback(window, key_callback);

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
    Shader lightShader("resources/shaders/light_source.vs", "resources/shaders/light_source.fs");
    Shader screenShader("resources/shaders/screen.vs", "resources/shaders/screen.fs");

    // models
    Model tableModel(FileSystem::getPath("resources/objects/dining_table/dining_table.obj"));
    Model cakeModel(FileSystem::getPath("resources/objects/slice_of_cake/cake.obj"));
    Model lightModel(FileSystem::getPath("resources/objects/light/light.obj"));

    // screen vertexes
    float quadVertices[] = {
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };

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

    // setup screen VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    // configure MSAA framebuffer
    // --------------------------
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a multisampled color attachment texture
    unsigned int textureColorBufferMultiSampled;
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
    // create a (also multisampled) renderbuffer object for depth and stencil attachments
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    screenShader.setInt("screenTexture", 0);

    unsigned int floorDiffTexture = TextureFromFile("floor_diffuse.png", "resources/objects/floor");
    unsigned int floorSpecTexture = TextureFromFile("floor_specular2.png", "resources/objects/floor");

    glm::vec3 pointLightPositions[3];
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // draw scene as normal in multisampled buffers
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // light
        float pointLightLinear = 0.09;
        float pointLightQuadratic = 0.032;
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);

        set_light_bulb(lightModel, lightShader, pointLightPositions[0], glm::radians((float)(10.0 * sin(1.0 + 2*glfwGetTime()))), glm::vec3(0.0f, 2.0f, -3.0f));
        set_light_bulb(lightModel, lightShader, pointLightPositions[1], glm::radians((float)(10.0 * sin(2*glfwGetTime()))), glm::vec3(0.0f, 2.0f, 0.0f));
        set_light_bulb(lightModel, lightShader, pointLightPositions[2], glm::radians((float)(10.0 * sin(2.0 + 2*glfwGetTime()))), glm::vec3(0.0f, 2.0f, 3.0f));

        objectShader.use();
        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);

        // point light 1
        set_point_light(objectShader, pointLightPositions[0], 0, pointLightLinear, pointLightQuadratic);
        // point light 2
        set_point_light(objectShader, pointLightPositions[1], 1, pointLightLinear, pointLightQuadratic);
        // point light 3
        set_point_light(objectShader, pointLightPositions[2], 2, pointLightLinear, pointLightQuadratic);
        // spotLight
        set_spot_light(objectShader, camera);

        // table
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));
        objectShader.setMat4("model", model);
        tableModel.Draw(objectShader);

        // cake
        draw_cake(cakeModel, objectShader, glm::vec3(1.5f,-2.15f, 3.0f));
        draw_cake(cakeModel, objectShader, glm::vec3(-1.5f,-2.15f, 3.0f));
        draw_cake(cakeModel, objectShader, glm::vec3(1.5f,-2.15f, 0.0f));
        draw_cake(cakeModel, objectShader, glm::vec3(-1.5f,-2.15f, 0.0f));
        draw_cake(cakeModel, objectShader, glm::vec3(1.5f,-2.15f, -3.0f));
        draw_cake(cakeModel, objectShader, glm::vec3(-1.5f,-2.15f, -3.0f));

        //floor
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorDiffTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorSpecTexture);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -5.0f, 0.0f));
        model = glm::scale(model, glm::vec3(20.0f, 1.0f, 20.0f));
        objectShader.setMat4("model", model);
        glBindVertexArray(floorVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // 2. now render quad with scene's visuals as its texture image
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

        // draw Screen quad
        screenShader.use();
        screenShader.setInt("effect", effect);
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled); // use multisampled texture
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &floorVAO);
    glDeleteBuffers(1, &floorVBO);
    glDeleteBuffers(1, &floorEBO);

    glfwTerminate();
    return 0;
}

void set_light_bulb(Model& lightModel, Shader& lightShader, glm::vec3& pointLightPositions, float angle, const glm::vec3& translation_vec) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, translation_vec);
    model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
    model = glm::translate(model, glm::vec3(0.0f, 1.32f, 0.0f));
    model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(0.0f, -1.32f, 0.0f));
    pointLightPositions = glm::vec3(model * glm::vec4(0.0f, 0.2f, 0.0f, 1.0f));
    lightShader.setMat4("model", model);
    lightModel.Draw(lightShader);
}

void draw_cake(Model& cakeModel, Shader& objectShader, const glm::vec3& translation_vec) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, translation_vec);
    model = glm::rotate(model, -0.3f, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
    objectShader.setMat4("model", model);
    cakeModel.Draw(objectShader);
}

void set_spot_light(Shader& objectShader, Camera& camera) {
    objectShader.setVec3("spotLight.position", camera.Position);
    objectShader.setVec3("spotLight.direction", camera.Front);
    if(isSpotlightActivated){
        objectShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        objectShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        objectShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    }
    else{ // All to 0.
        objectShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        objectShader.setVec3("spotLight.diffuse", 0.0f, 0.0f, 0.0f);
        objectShader.setVec3("spotLight.specular", 0.0f, 0.0f, 0.0f);
    }
    objectShader.setFloat("spotLight.constant", 1.0f);
    objectShader.setFloat("spotLight.linear", 0.01);
    objectShader.setFloat("spotLight.quadratic", 0.001);
    objectShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(2.5f)));
    objectShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(22.0f)));

    objectShader.setVec3("viewPos", camera.Position);
    objectShader.setFloat("material.shininess", 128.0f);
}

void set_point_light(Shader& objectShader, glm::vec3& point_light_position, int i, float point_light_linear, float point_light_quadratic) {
    objectShader.setVec3("pointLights[" + to_string(i) + "].position", point_light_position);
    objectShader.setVec3("pointLights[" + to_string(i) + "].ambient", 0.05f, 0.05f, 0.05f);
    objectShader.setVec3("pointLights[" + to_string(i) + "].diffuse", 0.8f, 0.8f, 0.8f);
    objectShader.setVec3("pointLights[" + to_string(i) + "].specular", 1.0f, 1.0f, 1.0f);
    objectShader.setFloat("pointLights[" + to_string(i) + "].constant", 1.0f);
    objectShader.setFloat("pointLights[" + to_string(i) + "].linear", point_light_linear);
    objectShader.setFloat("pointLights[" + to_string(i) + "].quadratic", point_light_quadratic);
}

void processInput(GLFWwindow *window) {
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mod) {
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        isSpotlightActivated = !isSpotlightActivated;
    }

    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        effect = !effect;
    }

}