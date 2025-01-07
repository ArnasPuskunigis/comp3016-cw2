#include <glad/glad.h>
#include <glfw3.h>
#include <stb_image.h>

#include "FastNoiseLite.h"

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

//Tank
vec3 tankPosition = vec3(0.0f, -1.2f, -2.0f);
float tankSpeed = 0.05f;
float tankRotationAngle = 0.0f;

//Crate 1
vec3 cratePosition = vec3(2.0f, -2.0f, -3.0f);
float crateRotationAngle = 0.0f;

//Crate 2
vec3 cratePosition2 = vec3(-2.0f, -2.0f, -3.0f);
float crateSpeed = 0.005f;
bool moveUp = true;

//Procedural generation
const int renderDistance = 128;
const int mapSize = renderDistance * renderDistance;

const int squaresRow = renderDistance - 1;
const int trianglesPerSquare = 2;
const int triangleGrid = squaresRow * squaresRow * trianglesPerSquare;

//Camera
bool cameraMovementActive = false;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = 800 / 2.0f;
float lastY = 800 / 2.0f;
bool firstMouse = true;

//Timing
float deltaTime = 0.0f;	//Time between current frame and last frame
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

    //Compile shaders into a program using a prewritten header file from : "https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader.h"
    Shader terrainShader("Shaders/terrain.vert", "Shaders/terrain.frag");
    //==========================

    // Cube vertices with texture coordinates
    float cubeVertices[] = {
        // Positions          // Texture Coordinates
       -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
       -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,

       -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
       -0.5f,  0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

       -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
       -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
       -0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
       -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

        0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, 1.0f,

       -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,
       -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

       -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  1.0f, 1.0f,
       -0.5f, -0.5f,  0.5f,  0.0f, 1.0f
    };

    // Cube indices (for optimization)
    unsigned int cubeIndices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        8, 9, 10, 10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    //Terrain ====
    FastNoiseLite TerrainNoise;
    //Settings
    TerrainNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    TerrainNoise.SetFrequency(0.02f);
    //Randomisation
    int terrainSeed = rand() % 100;
    TerrainNoise.SetSeed(terrainSeed);

    //Biomes
    FastNoiseLite BiomeNoise;
    //Settings
    BiomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    BiomeNoise.SetFrequency(0.02f);
    //Randomisation
    int biomeSeed = rand() % 100;
    BiomeNoise.SetSeed(biomeSeed);

    //Terrain
    GLfloat terrainVertices[mapSize][6];
    float drawingStartPosition = 1.0f;
    float columnVerticesOffset = drawingStartPosition;
    float rowVerticesOffset = drawingStartPosition;

    int rowIndex = 0;

    //Biome colours and terrain height
    int i = 0;
    for (int y = 0; y < renderDistance; y++) {
        for (int x = 0; x < renderDistance; x++) {
            terrainVertices[i][1] = TerrainNoise.GetNoise((float)x, (float)y);

            float biomeValue = BiomeNoise.GetNoise((float)x, (float)y);
            if (biomeValue <= -0.75f) { 
                //Swamp
                terrainVertices[i][3] = 0.0f;
                terrainVertices[i][4] = 0.4f;
                terrainVertices[i][5] = 0.0f;
            }
            else 
            {
                //Desert
                terrainVertices[i][3] = 0.9f;
                terrainVertices[i][4] = 0.9f;
                terrainVertices[i][5] = 0.1f;
            }

            i++;
        }
    }

    //Set biome colours based on Y values, and terrain verices set up
    for (int i = 0; i < mapSize; i++) {
        //vertices
        terrainVertices[i][0] = columnVerticesOffset;
        terrainVertices[i][2] = rowVerticesOffset;

        //Rocks
        if (terrainVertices[i][1] >= (4.0f / 8.0f)) {
            terrainVertices[i][3] = 0.2f;
            terrainVertices[i][4] = 0.2f;
            terrainVertices[i][5] = 0.2f;
        }
        //Planes
        else if (terrainVertices[i][1] >= (1.0f / 8.0f)) {
            terrainVertices[i][3] = 0.2f;
            terrainVertices[i][4] = 1.0f;
            terrainVertices[i][5] = 0.2f;
        }

        columnVerticesOffset -= 0.0625f;
        rowIndex++;
        if (rowIndex == renderDistance) {
            rowIndex = 0;
            columnVerticesOffset = drawingStartPosition;
            rowVerticesOffset -= 0.0625f;
        }
    }
    
    //Set up the terrain indices
    GLuint terrainIndices[triangleGrid][3];
    int columnIndicesOffset = 0;
    int rowIndicesOffset = 0;
    rowIndex = 0;

    for (int i = 0; i < triangleGrid - 1; i += 2) {
        terrainIndices[i][0] = columnIndicesOffset + rowIndicesOffset;
        terrainIndices[i][2] = renderDistance + columnIndicesOffset + rowIndicesOffset;
        terrainIndices[i][1] = 1 + columnIndicesOffset + rowIndicesOffset;
        terrainIndices[i + 1][0] = 1 + columnIndicesOffset + rowIndicesOffset;
        terrainIndices[i + 1][2] = renderDistance + columnIndicesOffset + rowIndicesOffset;
        terrainIndices[i + 1][1] = 1 + renderDistance + columnIndicesOffset + rowIndicesOffset;
        columnIndicesOffset += 1;
        rowIndex++;
        if (rowIndex == squaresRow) {
            rowIndex = 0;
            columnIndicesOffset = 0;
            rowIndicesOffset += renderDistance;
        }
    }

    unsigned int terrainVAO, terrainVBO, terrainEBO;
    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glGenBuffers(1, &terrainEBO);

    glBindVertexArray(terrainVAO);

    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(terrainVertices), terrainVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(terrainIndices), terrainIndices, GL_STATIC_DRAW);

    //Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //Colour attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    //Create and generate texture object for the Signature Cube ====
    unsigned int signatureTexture;
    glGenTextures(1, &signatureTexture);
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
        std::cout << "Could not load the texture for the Signature Cube" << std::endl;
    }
    //Free memory
    stbi_image_free(data);

    unsigned int cubeVAO, cubeVBO, cubeEBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    //Load the tank model
    Model tank("tank/m26.obj");
    //Load the crate model
    Model crate("crate/box_FBX.fbx");
    //Load the 2nd crate model
    Model crate2("crate/box_FBX.fbx");
    
    //Render loop!
    while (!glfwWindowShouldClose(window))
    {
        //Timer for camera rotation
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        crateRotationAngle += 0.5f;

        if (moveUp) {
            cratePosition2.y += crateSpeed;
        }
        else if (!moveUp) {
            cratePosition2.y -= crateSpeed;
        }
        if (cratePosition2.y >= -1.0f) {
            moveUp = false;
        }
        else if (cratePosition2.y <= -2.0f) {
            moveUp = true;
        }

        //Keyboard user input
        processInput(window);

        //Reset screen and buffers
        glClearColor(0.1f, 0.1f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Set up view and projection matrices
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)800 / (float)800, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        //Signature Cube rendering ====
        shaderProgram.use();

        //Set the projection and the view matrices for the cube
        shaderProgram.setMat4("view", view);
        shaderProgram.setMat4("projection", projection);

        //Set the model matrix for the cube
        glm::mat4 cubeModel = glm::mat4(1.0f);
        cubeModel = glm::translate(cubeModel, glm::vec3(0.0f, -6.0f, 0.0f)); // Position the cube below the ground
        cubeModel = glm::scale(cubeModel, glm::vec3(2.0f, 2.0f, 2.0f)); // Scale the cube
        shaderProgram.setMat4("model", cubeModel);

        //Bind the texture for the cube
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, signatureTexture);

        //Bind the VAO and draw the cube
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //Terrain rendering ====
        terrainShader.use();
        //Set the projection and the view matrices for the cube
        terrainShader.setMat4("view", view);
        terrainShader.setMat4("projection", projection);

        //Set the model matrix for the cube
        glm::mat4 terrainModel = glm::mat4(1.0f);
        terrainModel = glm::translate(terrainModel, glm::vec3(7.0f, -1.0f, 7.0f)); //Position the terrain near and under the tank
        terrainModel = glm::scale(terrainModel, glm::vec3(5.0f, 5.0f, 5.0f)); //Scale the terrain
        terrainShader.setMat4("model", terrainModel);

        //Bind the VAO and draw the terrain
        glBindVertexArray(terrainVAO);
        glDrawElements(GL_TRIANGLES, mapSize*32, GL_UNSIGNED_INT, 0);

        //Tank model ====
        shaderProgram.use();
        //Set the projection and the view matrices for the tank
        shaderProgram.setMat4("projection", projection);
        shaderProgram.setMat4("view", view);

        //Set model matrix for the tank
        glm::mat4 tankModel = glm::mat4(1.0f);
        tankModel = glm::translate(tankModel, tankPosition);
        tankModel = glm::rotate(tankModel, glm::radians(tankRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate the tank
        shaderProgram.setMat4("model", tankModel);
        //Draw the tank model
        tank.Draw(shaderProgram);

        shaderProgram.setMat4("projection", projection);
        shaderProgram.setMat4("view", view);

        glm::mat4 crateModel = glm::mat4(1.0f);
        crateModel = glm::translate(crateModel, cratePosition);
        crateModel = glm::rotate(crateModel, glm::radians(crateRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate the crate
        crateModel = glm::scale(crateModel, glm::vec3(0.050f, 0.050f, 0.050f)); // Scale the crate
        shaderProgram.setMat4("model", crateModel);
        //Draw the crate model
        crate.Draw(shaderProgram);

        shaderProgram.setMat4("projection", projection);
        shaderProgram.setMat4("view", view);

        glm::mat4 crateModel2 = glm::mat4(1.0f);
        crateModel2 = glm::translate(crateModel2, cratePosition2);
        crateModel2 = glm::rotate(crateModel2, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate the crate
        crateModel2 = glm::scale(crateModel2, glm::vec3(0.050f, 0.050f, 0.050f)); // Scale the crate
        shaderProgram.setMat4("model", crateModel2);
        //Draw the other crate model
        crate2.Draw(shaderProgram);

        //Draw the screen and process input
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //Clean up the resources used for the window
    glfwTerminate();
    //Delete the shader program
    glDeleteProgram(shaderProgram.ID);
    glDeleteProgram(terrainShader.ID);
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
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(tankRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 forwardDirection = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
        tankPosition += forwardDirection * tankSpeed;
        //Forwards + Right movement
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            tankRotationAngle += 1.0f;
        }
        //Forwards + Left movement
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            tankRotationAngle -= 1.0f;
        }
    }
    //Backwards movement
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !cameraMovementActive) {
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(tankRotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 forwardDirection = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
        tankPosition += forwardDirection * -tankSpeed;
        //Backwards + Right movement
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            tankRotationAngle += 1.0f;
        }
        //Backwards + Left movement
        else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            tankRotationAngle -= 1.0f;
        }
    }
    //Just rotating left
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !cameraMovementActive) {
        tankRotationAngle += 1.0f;
    }
    //Just rotating right
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !cameraMovementActive) {
        tankRotationAngle -= 1.0f;
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