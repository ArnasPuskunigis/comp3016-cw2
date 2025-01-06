#include <glad/glad.h>
#include <glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/ext/vector_float3.hpp"

#include "camera.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader_m.h"
#include "model.h"

#include <iostream>

using namespace glm;

//Mouse input
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
//Keyboard Input
void processInput(GLFWwindow* window);


vec3 tankPosition = vec3(0.0f, -1.2f, -2.0f);
float tankSpeed = 0.05f;
float rotationAngle = 0.0f;

bool cameraMovementActive = false;

//int renderDistance = 128;
//int mapSize = renderDistance * renderDistance;
//
//const int squaresRow = renderDistance - 1;
//const int trianglesPerSquare = 2;
//const int triangleGrid = squaresRow * squaresRow * trianglesPerSquare;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 800 / 2.0f;
float lastY = 800 / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


int main()
{
    //Initialize GLFW and set up OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Creating window
    GLFWwindow* window = glfwCreateWindow(800, 800, "Tank", NULL, NULL);
    //Error checking
    if (window == NULL)
    {
        std::cout << "GLFW window not created" << std::endl;
        glfwTerminate();
        return -1;
    }

    //Set this window as the context for opengl operations
    glfwMakeContextCurrent(window);
    //User mouse movement
    glfwSetCursorPosCallback(window, mouse_callback);
    //Disable the cursor for mouse movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    //Initialise glad, if failed, ouput an error
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "GLAD not initialised" << std::endl;
        return -1;
    }

    //Enable depth testing
    glEnable(GL_DEPTH_TEST);

    //Compile shaders into a program using a prewritten header file from : "https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader.h"
    Shader shaderProgram("Shaders/vertex.vert", "Shaders/fragment.frag");
    //==========================


    //Plane vertices with texture coordinates for the signature
    float planeVertices[] = {
        //Vertex positions    //Texture coordinates
        -0.5f,  0.0f, -0.5f,  0.0f, 0.0f,
         0.5f,  0.0f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.0f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.0f,  0.5f,  0.0f, 1.0f
    };
    //Plane indices for optimisation
    unsigned int planeIndices[] = {
        0, 1, 2,
        2, 3, 0
    };
    //Create and generate texture object for the plane
    unsigned int signatureTexture;
    glGenTextures(1, &signatureTexture);
    //Cofigure the plane texture settings
    glBindTexture(GL_TEXTURE_2D, signatureTexture);
    //Load the signature.jpg texture and generate the mipmaps
    int width, height, nrChannels;
    unsigned char* data = stbi_load("signature.jpg", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    //If this failed, ouput an error message
    else
    {
        std::cout << "Could not load the texture for the plane" << std::endl;
    }
    //Free memory
    stbi_image_free(data);

    //Create VAO, VBO, and EBO for the plane
    unsigned int planeVAO, planeVBO, planeEBO;
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glGenBuffers(1, &planeEBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);
    //Position attribute for the plane
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //Texture coordinates attribute for the plane
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    //Select shader program
    shaderProgram.use();
    //Set uniforms
    shaderProgram.setInt("texture1", 0);

    //Get the tank model
    Model tank("tank/m26.obj");

    //Render loop!
    while (!glfwWindowShouldClose(window))
    {
        //Timer for camera rotation
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //Keyboard user input
        processInput(window);

        //Reset screen and buffers
        glClearColor(0.3f, 0.3f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Set up view and projection matrices
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)800 / (float)800, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        //Signature plane =====
        shaderProgram.use();

        //Set the projection and the view matrices for the plane
        shaderProgram.setMat4("view", view);
        shaderProgram.setMat4("projection", projection);

        //Set the model matrix for the plane
        glm::mat4 planeModel = glm::mat4(1.0f);
        planeModel = glm::translate(planeModel, glm::vec3(0.0f, -4.0f, 0.0f)); // Position the plane under the "map"
        planeModel = glm::scale(planeModel, glm::vec3(4.0f, 1.0f, 4.0f)); // Scale the plane by 4 in x and z
        shaderProgram.setMat4("model", planeModel);

        // Bind the texture for the plane
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, signatureTexture);

        // Bind the VAO and draw the plane
        glBindVertexArray(planeVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0);

        //Tank model ====
        // Set the projection and the view matrices for the tank
        shaderProgram.setMat4("projection", projection);
        shaderProgram.setMat4("view", view);

        // Set model matrix for the tank
        glm::mat4 tankModel = glm::mat4(1.0f);
        tankModel = glm::translate(tankModel, tankPosition);
        tankModel = glm::rotate(tankModel, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate the tank
        shaderProgram.setMat4("model", tankModel);
        // Draw the tank model
        tank.Draw(shaderProgram);

        //Draw the screen and process input
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //Clean up the resources used for the window
    glfwTerminate();
    //Delete the shader program
    glDeleteProgram(shaderProgram.ID);
    //Exit code
    return 0;
}

//Input
void processInput(GLFWwindow* window)
{
    //End the game loop if escape pressed
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //Tank movement ====
    //Forwards movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && !cameraMovementActive) {
        //Calculate the forward direction
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 forwardDirection = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
        tankPosition += forwardDirection * tankSpeed;
        //Forwards + Right movement
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            rotationAngle += 1.0f;
        }
        //Forwards + Left movement
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            rotationAngle -= 1.0f;
        }
    }
    //Backwards movement
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !cameraMovementActive) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 forwardDirection = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
        tankPosition += forwardDirection * -tankSpeed;
        //Backwards + Right movement
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            rotationAngle += 1.0f;
        }
        //Backwards + Left movement
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            rotationAngle -= 1.0f;
        }
    }
    //Just rotating left
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !cameraMovementActive) {
        rotationAngle += 1.0f;
    }
    //Just rotating right
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !cameraMovementActive) {
        rotationAngle -= 1.0f;
    }

    //Camera movement toggle
    static bool cKeyWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!cKeyWasPressed) {
            cameraMovementActive = !cameraMovementActive;
            cKeyWasPressed = true;
        }
    }
    else {
        cKeyWasPressed = false;
    }

    //Camera movement ====
    if (cameraMovementActive) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }

}

//Mouse input
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    //Mouse x and y
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    //Check for previous mouse input
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

    //Execute camera rotation
    camera.ProcessMouseMovement(xoffset, yoffset);
}