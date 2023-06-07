#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../include/shader.h"
#include <iostream>

// for matrix transformations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings

float randomColor(){
    return (rand()%255) / 255.0f;
}

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// parametrs of toogle, camera and translation
bool set = false;

float rAngle = 0;

glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  0.0f); // put at the origin
glm::vec3 cameraPoint = glm::vec3(0.0f, 0.0f, 0.5f); // pointing at the centre of prism,
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f); // normal to camera plane

glm::vec3 initialPos = glm::vec3(0.0f, 0.0f, 0.5f);
glm::vec3 currPos = glm::vec3(0.0f, 0.0f, 0.5f);

int main(int argc, char **argv)
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("../src/vertex.shader", "../src/fragment.shader"); // you can name your shader files however you like

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    int n = atoi(argv[1]); 
    // command line arguements, argc is the number of arguemts, argc stores string in 0 based indexing

    double angle = 2*M_PI / (float)n;
    float vertices[(2*n+2)*3];

    for(int i=0; i<n; i++){
        vertices[3*i] = cos(i*angle);
        vertices[3*i+1] = sin(i*angle);
        vertices[3*i+2] = 0;
    }

    for(int i=0; i<n; i++){
        vertices[3*i+3*n] = cos(i*angle);
        vertices[3*i+1 + 3*n] = sin(i*angle);
        vertices[3*i+2 + 3*n] = 1.0f; // noramlized coordinates
    }

    for(int i=0; i<3; i++) vertices[6*n + i] = 0.0f;
    for(int i=0; i<2; i++) vertices[6*(n) + 3 + i] = 0.0f;
    vertices[6*n+5] = 1.0f;


    // index buffer of prism

    unsigned int indices_prism[4*n*3]; // triangles

    for(int i=0; i<n; i++){ // lower face triangle
        indices_prism[3*i] = 2*n;
        indices_prism[3*i + 1] = i;
        indices_prism[3*i + 2] = (i+1)%n;
    }

    for(int i=0; i<n; i++){ // upper face triangle
        indices_prism[3*i + 3*n] = 2*n+1;
        indices_prism[3*i + 1 + 3*n] = i+n;
        indices_prism[3*i + 2 + 3*n] = (i+1)%n+n;
    }

    // form the rectangular faces
    for(int i=0; i<n; i++){
        indices_prism[6*n + 6*i] = i;
        indices_prism[6*n + 6*i + 1] = (i+1)%n;
        indices_prism[6*n + 6*i + 2] = (i+1)%n + n;
        indices_prism[6*n + 6*i + 3] = i + n;
        indices_prism[6*n + 6*i + 4] = (i+1)%n + n;
        indices_prism[6*n + 6*i + 5] = i;
    }

    // index buffer of pyramid 

    unsigned int indices_pyramid[2*n*3]; // triangles

    for(int i=0; i<n; i++){ // lower face triangle
        indices_pyramid[3*i] = 2*n;
        indices_pyramid[3*i + 1] = i;
        indices_pyramid[3*i + 2] = (i+1)%n;
    }

    for(int i=0; i<n; i++){ // side face triangle
        indices_pyramid[3*i + 3*n] = i;
        indices_pyramid[3*i + 1 + 3*n] = (i+1)%n;
        indices_pyramid[3*i + 2 + 3*n] = 2*n+1;
    }


    glm:: vec3 colorArray[n]; // n rectangles, n triangles
    for(int i=0; i<n; i++) colorArray[i] = glm::vec3(randomColor(), randomColor(), randomColor());

    // Using VAO, VBO, EBO
    //-----------------------------------------------------------------

    unsigned int VAO[2], VBO[2], EBO[2]; // inopengl objects are represented by id's

    glGenVertexArrays(2, VAO);
    glGenBuffers(2, VBO);
    glGenBuffers(2, EBO);

    // make VAO the current vertex array object by binding it 
    glBindVertexArray(VAO[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_prism), indices_prism, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // position of vertex attribute

    // make VAO the current vertex array object by binding it 
    glBindVertexArray(VAO[1]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_pyramid), indices_pyramid, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // position of vertex attribute

    // render loop
    // -----------

    glEnable(GL_DEPTH_TEST);
    //  If the current fragment is behind the other fragment it is discarded, otherwise overwritten.

    ourShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    ourShader.setMat4("projection", projection);

    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        
        processInput(window); // set the translation matrix

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear before the next render

        glm::mat4 model = glm::mat4(1.0f); // translate the object to the world coordinates
        model = glm::rotate(model, (float)glm::radians(rAngle), glm::vec3(0.0f, 0.0f, 1.0f));
        
        // using our Shader program
        model = glm::translate(model, currPos-initialPos); // translation is translation of center of prism
        
        ourShader.setMat4("model", model);
        // first put the object at it's position (ie. translate) and then rotate about a given axis

        glm::mat4 view = glm::lookAt(cameraPos, cameraPoint, cameraUp);
        ourShader.setMat4("view", view);

        // prism : render all the triangles individually
        if (set == false) 
        {
            glBindVertexArray(VAO[0]);

            // lower n trinagles
            for(int i=0; i<n; i++)
            {
                // set it orange color
                ourShader.setVec3("ourColor", glm::vec3(1.0f, 0.5f, 0.2f)); 

                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(sizeof(uint)*3*i) );
                // state, number of elements to render, type of value in indices,
                // Specifies a pointer to the location where the indices are stored.
            }

            // upper n triangles
            for(int i=0; i<n; i++)
            {
                // set it yellow color
                ourShader.setVec3("ourColor", glm::vec3(1.0f, 1.0f, 0.0f));
                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(sizeof(uint)*3*(i+n)) );
            }

            // n rectangles
            for(int i=0; i<n; i++)
            {
                ourShader.setVec3("ourColor", colorArray[i]);
                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(sizeof(uint)*6*(i+n)) );
                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(sizeof(uint)* (6*(i+n) + 3) ));
            }
        }

        else // pyramid
        {
            glBindVertexArray(VAO[1]);

            // base triangles
            for(int i=0; i<n; i++)
            {
                // set it orange color
                ourShader.setVec3("ourColor", glm::vec3(1.0f, 0.5f, 0.2f)); 
                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(sizeof(uint)*3*i) );
            }

            // upper n triangles
            for(int i=0; i<n; i++)
            {
                ourShader.setVec3("ourColor", colorArray[i]);
                glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(sizeof(uint)*3*(i+n)) );
            }
        }
        // primitive, number of vertices, data type, starting index

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteBuffers(2, EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    // for closing the window

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // for Toogling

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) 
        set = 1 - set;

    // for Rotating 
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) 
    {
        rAngle += 5;
    }

    // for Flying Camera

    float cameraSpeed = 0.2f;

    // W and S for forward and backward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPos.z += cameraSpeed; 
        cameraPoint = currPos;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPos.z -= cameraSpeed; 
        cameraPoint = currPos;
    }

    // A and D for left and right
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPos.x -= cameraSpeed; 
        cameraPoint = currPos;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPos.x += cameraSpeed; 
        cameraPoint = currPos;
    }

    // Q and E for up and down
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        cameraPos.y += cameraSpeed; 
        cameraPoint = currPos;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        cameraPos.y -= cameraSpeed; 
        cameraPoint = currPos;
    }

    // for Object translation 

    float objectSpeed = 0.2f;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        currPos.y += objectSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        currPos.y -= objectSpeed;

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        currPos.x += objectSpeed;

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        currPos.x -= objectSpeed;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) // towards
        currPos.z += objectSpeed;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) // away
        currPos.z -= objectSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}