#include <glad/glad.h>
#include <glfw3.h>
#include <stb_image.h>


//GLAD
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//GLM
#include "glm/ext/vector_float3.hpp"
#include <glm/gtc/type_ptr.hpp> //Access to the value_ptr

#include "camera.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader_m.h"
#include "model.h"

#include <iostream>

using namespace glm;

//Resizing screen
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//User Input
void processInput(GLFWwindow* window);

vec3 tankPosition = vec3(0.0f, -1.2f, -2.0f);
float tankSpeed = 0.05f;

int main()
{
    //Initialize GLFW and set up OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Window
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
    //Handle window resizing
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //Initialise glad, if failed, ouput an error
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "GLAD not initialised" << std::endl;
        return -1;
    }

    //Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // camera
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    float lastX = 800 / 2.0f;
    float lastY = 800/ 2.0f;
    bool firstMouse = true;

    //Compile shaders into a program using a prewritten header file from : "https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader.h"
    Shader shaderProgram("Shaders/vertex.vert", "Shaders/fragment.frag");
    //==========================
    shaderProgram.use();

    //Compile shaders into a program using a prewritten header file from : "https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader.h"
    Shader shaderProgram1("Shaders/obj.vert", "Shaders/obj.frag");
    //==========================
    /*shaderProgram1.use();
    shaderProgram.use();*/
    
    //Model rock("rock/Rock07-Base.obj");
    Model rock("tank/m26.obj");

    //Plane vertices with texture coordinates
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
    unsigned int planeTexture;
    glGenTextures(1, &planeTexture);

    //Cofigure the plane texture settings
    glBindTexture(GL_TEXTURE_2D, planeTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Load the container.jpg texture and generate the mipmaps
    int width, height, nrChannels;
    unsigned char* data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
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

    //View matrix for camera, "translates" it to (0.0f, -1.5f, -5.0f)
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, -5.0f));
    //Projectiojn matrix, adds perspective, mainly fov
    glm::mat4 projection = glm::perspective(glm::radians(75.0f), (float)800 / 800, 0.1f, 100.0f);

    //Select shader program
    shaderProgram.use();
    //Set uniforms
    shaderProgram.setInt("texture1", 0);
    shaderProgram.setMat4("view", view);
    shaderProgram.setMat4("projection", projection);


    //Render loop!!
    while (!glfwWindowShouldClose(window))
    {
        //Input
        processInput(window);

        //Reset
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glEnable(GL_CULL_FACE); //Discards all back-facing triangles

        shaderProgram.use();

        //Bind the texture for the plane
        glBindTexture(GL_TEXTURE_2D, planeTexture);
        //Set the scale to 4 on x and z axis
        shaderProgram.setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 1.0f, 4.0f)));
        //Bind the vao for rendering
        glBindVertexArray(planeVAO);
        //Draw the plane
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        //Enable correct shader for the camera uniforms and model drawing
        shaderProgram1.use();

        glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)800 / (float)800, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shaderProgram1.setMat4("projection", projection);
        shaderProgram1.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, tankPosition); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shaderProgram1.setMat4("model", model);
        rock.Draw(shaderProgram1);


        //Draw the elements created to the window
        glfwSwapBuffers(window);
        //Process events
        glfwPollEvents();
    }

    //Clean up ====

    //Clean up the resources used for the window
    glfwTerminate();

    //Clean up the resouces used for the textures
    glDeleteTextures(1, &planeTexture);

    //Clean up the resouces used for the buffers and vertex array objects for the plane
    glDeleteBuffers(1, &planeVBO);
    glDeleteBuffers(1, &planeEBO);
    glDeleteVertexArrays(1, &planeVAO);

    //Delete the shader program
    glDeleteProgram(shaderProgram.ID);

    //Exit code
    return 0;
}

//Input
void processInput(GLFWwindow* window)
{
    //End the loop if Escape pressed
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

}

//Window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //Redefine the viewport
    glViewport(0, 0, width, height);
}
