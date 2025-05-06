#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "maze_generators/depth_first_search.h"
#include "maze_generators/maze_prims_algorithm.h"
#include "lighting.h"
#include "deque"
#include <chrono>
#include <thread>
#include <mutex>



#define DEG2RAD (M_PI/180.0)

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube, Sphere, Torus, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum LightNames {WhiteSpotLight};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {Brass, RedPlastic};
enum Textures {ShadowTex, NumTextures};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];
GLuint TextureIDs[NumTextures];
GLuint ShadowBuffer;

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;

// Model files
const char * cubeFile = "../models/unitcube.obj";
const char * sphereFile = "../models/sphere.obj";
const char * torusFile = "../models/torus.obj";


// Shader variables
// Light shader program with shadows reference
GLuint phong_shadow_program;
GLuint phong_shadow_vPos;
GLuint phong_shadow_vNorm;
GLuint phong_shadow_proj_mat_loc;
GLuint phong_shadow_camera_mat_loc;
GLuint phong_shadow_norm_mat_loc;
GLuint phong_shadow_model_mat_loc;
GLuint phong_shadow_shad_proj_mat_loc;
GLuint phong_shadow_shad_cam_mat_loc;
GLuint phong_shadow_lights_block_idx;
GLuint phong_shadow_materials_block_idx;
GLuint phong_shadow_material_loc;
GLuint phong_shadow_num_lights_loc;
GLuint phong_shadow_light_on_loc;
GLuint phong_shadow_eye_loc;
const char *phong_shadow_vertex_shader = "../phongShadow.vert";
const char *phong_shadow_frag_shader = "../phongShadow.frag";

// Shadow shader program reference
GLuint shadow_program;
GLuint shadow_vPos;
GLuint shadow_proj_mat_loc;
GLuint shadow_camera_mat_loc;
GLuint shadow_model_mat_loc;
const char *shadow_vertex_shader = "../shadow.vert";
const char *shadow_frag_shader = "../shadow.frag";

// Debug shadow program reference
GLuint debug_program;
const char *debug_shadow_vertex_shader = "../debugShadow.vert";
const char *debug_shadow_frag_shader = "../debugShadow.frag";

// Shadow flag
GLuint shadow = false;

// Generic shader variables references
GLuint vPos;
GLuint vNorm;
GLuint model_mat_loc;

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 normal_matrix;
mat4 model_matrix;
mat4 shadow_proj_matrix;
mat4 shadow_camera_matrix;

// Global light and material variables
vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Spherical camera state
GLfloat azimuth = -180.0f;
GLfloat daz = 8.0f;
GLfloat elevation = 88.0f;
GLfloat del = 4.0f;
GLfloat radius = 12.0f;
GLfloat dr = 0.1f;
GLfloat min_radius = 2.0f;
bool anim = true;

// Global object variables
GLfloat sphere_angle = 0.0;
GLdouble elTime = 0.0;
GLdouble rpm = 10.0;
vec3 axis = {0.0f, 1.0f, 0.0f};



// Global screen dimensions
GLint ww,hh;

const int grid_size = 14;
int player_x = 1;
int player_y = 1;
// Cube Position
vec3 cube_pos = {1, 0.5f, 1};
//Used to iterate over the movement history generated from an algorithm.
//Deque allows us to add to back and pop from front in O(1)
std::deque<std::pair<int, int>> movement_history;
std::mutex replay_mutex;
bool is_replay_active = false;

// This is the locations of the walls in the grid
// 0 in the array represents a space that can be moved to
// 1 in the array represents a space where a wall is
// 2 in the array represents the player
// 3 in the array represents the goal
// good luck and god bless - yes let us bless
int wall_loc[grid_size][grid_size] = {0};


// Camera
vec3 eye = {grid_size / 2.0f, 4.0f, grid_size / 2.0f};
vec3 center = vec3(grid_size / 2.0f, 0.0f, grid_size / 2.0f);
vec3 up = {0.0f, 11.0f, .0f};


bool algo_or_file_flag = false;
const char * mazeFile = "../models/maze.txt";

void display( );
void render_scene( );
void print_wall_array( );
void setup_walls(bool flag);
void generate_walls_from_file();
void create_shadows( );
void build_geometry( );
void build_materials( );
void build_lights( );
void build_shadows( );
void loop(GLFWwindow *window);
void load_model(const char * filename, GLuint obj);
void draw_mat_shadow_object(GLuint obj, GLuint material);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);
void renderQuad();

int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Pathfinding");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // Store initial window size
    glfwGetFramebufferSize(window, &ww, &hh);

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

	// Create geometry buffers
    build_geometry();
    // Create material buffers
    build_materials();
    // Create light buffers
    build_lights();
    // Create shadow buffer
    build_shadows();
    // Build walls
    setup_walls(algo_or_file_flag);

    // Load shaders
    // Load light shader with shadows
	ShaderInfo phong_shadow_shaders[] = { {GL_VERTEX_SHADER, phong_shadow_vertex_shader},{GL_FRAGMENT_SHADER, phong_shadow_frag_shader},{GL_NONE, NULL} };
    phong_shadow_program = LoadShaders(phong_shadow_shaders);
    phong_shadow_vPos = glGetAttribLocation(phong_shadow_program, "vPosition");
    phong_shadow_vNorm = glGetAttribLocation(phong_shadow_program, "vNormal");
    phong_shadow_camera_mat_loc = glGetUniformLocation(phong_shadow_program, "camera_matrix");
    phong_shadow_proj_mat_loc = glGetUniformLocation(phong_shadow_program, "proj_matrix");
    phong_shadow_norm_mat_loc = glGetUniformLocation(phong_shadow_program, "normal_matrix");
    phong_shadow_model_mat_loc = glGetUniformLocation(phong_shadow_program, "model_matrix");
    phong_shadow_shad_proj_mat_loc = glGetUniformLocation(phong_shadow_program, "light_proj_matrix");
    phong_shadow_shad_cam_mat_loc = glGetUniformLocation(phong_shadow_program, "light_cam_matrix");
    phong_shadow_lights_block_idx = glGetUniformBlockIndex(phong_shadow_program, "LightBuffer");
    phong_shadow_materials_block_idx = glGetUniformBlockIndex(phong_shadow_program, "MaterialBuffer");
    phong_shadow_material_loc = glGetUniformLocation(phong_shadow_program, "Material");
    phong_shadow_num_lights_loc = glGetUniformLocation(phong_shadow_program, "NumLights");
    phong_shadow_light_on_loc = glGetUniformLocation(phong_shadow_program, "LightOn");
    phong_shadow_eye_loc = glGetUniformLocation(phong_shadow_program, "EyePosition");

    // Load shadow shader
    ShaderInfo shadow_shaders[] = { {GL_VERTEX_SHADER, shadow_vertex_shader},{GL_FRAGMENT_SHADER, shadow_frag_shader},{GL_NONE, NULL} };
    shadow_program = LoadShaders(shadow_shaders);
    shadow_vPos = glGetAttribLocation(shadow_program, "vPosition");
    shadow_proj_mat_loc = glGetUniformLocation(shadow_program, "light_proj_matrix");
    shadow_camera_mat_loc = glGetUniformLocation(shadow_program, "light_cam_matrix");
    shadow_model_mat_loc = glGetUniformLocation(shadow_program, "model_matrix");

    // Load debug shadow shader
    ShaderInfo debug_shaders[] = { {GL_VERTEX_SHADER, debug_shadow_vertex_shader},{GL_FRAGMENT_SHADER, debug_shadow_frag_shader},{GL_NONE, NULL} };
    debug_program = LoadShaders(debug_shaders);

    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set background color
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);

    // Set Initial camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye = vec3(x, y, z);

    // Start loop
    loop(window);

    // Close window
    glfwTerminate();
    return 0;
}

void loop(GLFWwindow *window)
{
    while ( !glfwWindowShouldClose( window ) ) {
        // TODO: Create shadow buffer (cull front faces)
        glCullFace(GL_FRONT);
        create_shadows();
        glCullFace(GL_BACK);

        // Uncomment instead of display() to view shadow buffer for debugging
        //renderQuad();

        // Draw graphics
        display();


        // Update other events like input handling
        glfwPollEvents();
        GLdouble curTime = glfwGetTime();
        elTime = curTime;


        // Swap buffer onto screen
        glfwSwapBuffers( window );
    }
}

void display( )
{
    proj_matrix = mat4().identity();
    camera_matrix = mat4().identity();

    // Reset default viewport
    glViewport(0, 0, ww, hh);

    // Clear window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set projection matrix
    // Set orthographic viewing volume anisotropic
    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if (ww <= hh)
    {
        yratio = (GLfloat)hh / (GLfloat)ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat)ww / (GLfloat)hh;
    }
    proj_matrix = frustum(-1.0f*xratio, 1.0f*xratio, -1.0f*yratio, 1.0f*yratio, 1.0f, 100.0f);

    // Set camera matrix
    camera_matrix = lookat(eye, center, up);

    // Render objects
    render_scene();

    glFlush();
}

void render_scene() {
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            // Translate to the correct position
            trans_matrix = translate((float)i, -0.1f, (float)j);

            // Determine the type of block and set scale and material
            if (wall_loc[i][j] == 1) { // Wall
                scale_matrix = scale(0.9f, 2.0f, 0.9f);
                model_matrix = trans_matrix * scale_matrix * rot_matrix;
                if (!shadow) {
                    normal_matrix = model_matrix.inverse().transpose();
                }
                draw_mat_shadow_object(Cube, Brass);
            } else if (wall_loc[i][j] == 2) { // Player
                scale_matrix = scale(1.0f, 1.0f, 1.0f);
                model_matrix = trans_matrix * scale_matrix * rot_matrix;
                if (!shadow) {
                    normal_matrix = model_matrix.inverse().transpose();
                }
                draw_mat_shadow_object(Cube, RedPlastic);
            } else if (wall_loc[i][j] == 3) { // Goal
                scale_matrix = scale(0.9f, 0.9f, 0.9f);
                model_matrix = trans_matrix * scale_matrix * rot_matrix;
                if (!shadow) {
                    normal_matrix = model_matrix.inverse().transpose();
                }
                draw_mat_shadow_object(Cube, Sphere); // Example: Use a sphere for the goal
            } else { // Empty space
                scale_matrix = scale(0.9f, 0.2f, 0.9f);
                model_matrix = trans_matrix * scale_matrix * rot_matrix;
                if (!shadow) {
                    normal_matrix = model_matrix.inverse().transpose();
                }
                draw_mat_shadow_object(Cube, RedPlastic); // Example: Use a different material for empty spaces
            }
        }
    }

    // Draw sphere for light position (without shadow)
    if (!shadow) {
        trans_matrix = translate(Lights[0].position[0], Lights[0].position[1], Lights[0].position[2]);
        scale_matrix = scale(0.1f, 0.1f, 0.1f);
        model_matrix = trans_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_mat_shadow_object(Sphere, Brass);
    }
}

void sleep(int milliseconds){
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

void setup_walls(bool flag)
{

    if (flag)
    {
        PrimsMaze pm = PrimsMaze(player_x, player_y, grid_size, grid_size);
        //Copy the grid from the dfs into the world grid.
        for(int i = 0; i < grid_size; i++){
            for (int j = 0; j < grid_size; j++){
                wall_loc[i][j] = pm.grid[i][j];
            }
        }
    } else
    {
        generate_walls_from_file();
    }
    //Add player position
    // wall_loc[player_x][player_y] = 2;
    //print_wall_array();
}

void print_wall_array()
{
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size; ++j) {
            printf("%d ", wall_loc[i][j]);
        }
        printf("\n");
    }
    printf("Player position: %d, %d\n", player_x, player_y);
}

void generate_walls_from_file()
{
    // Open the maze file
    FILE *file = fopen(mazeFile, "r");
    if (file == NULL) {
        printf("Error opening maze file.\n");
        return;
    }



    // Read the maze from the file
    for (int i = 0; i < grid_size; ++i) {
        for (int j = 0; j < grid_size+2; ++j) {
            //printf("%d, ", fgetc(file));
            char ch = fgetc(file);
            //printf("%d ", ch);
            if (ch == 49) {
                wall_loc[i][j] = 1; // Wall
            } else if (ch == 48) {
                wall_loc[i][j] = 0; // Empty space
            } else if (ch == 50) {
                wall_loc[i][j] = 2; // Player
                player_x = i;
                player_y = j;
            } else if (ch == 51) {
                wall_loc[i][j] = 3; // Goal
            }
        }
        printf("\n");
    }

    // Close the file
    fclose(file);
    printf("Generated walls from file\n");
    print_wall_array();
}

void create_shadows(){
    // TODO: Set shadow projection matrix
    shadow_proj_matrix = frustum(-1.0, 1.0, -1.0, 1.0, 1.0, 20.0);

    // TODO: Set shadow camera matrix based on light position and direction
    vec3 leye = {Lights[0].position[0], Lights[0].position[1], Lights[0].position[2]};
    vec3 ldir = {Lights[0].direction[0], Lights[0].direction[1], Lights[0].direction[2]};
    vec3 lup = {0.0f, 1.0f, 0.0f};
    vec3 lcenter = leye + ldir;
    shadow_camera_matrix = lookat(leye, lcenter, lup);

    // Change viewport to match shadow framebuffer size
    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, ShadowBuffer);
    glClear(GL_DEPTH_BUFFER_BIT);
    // TODO: Render shadow scene
    shadow = true;
    render_scene();
    shadow = false;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Reset viewport
    glViewport(0, 0, ww, hh);
}

void build_geometry( )
{
    // Generate vertex arrays for objects
    glGenVertexArrays(NumVAOs, VAOs);

    // Load models
    load_model(cubeFile, Cube);
    load_model(sphereFile, Sphere);
    load_model(torusFile, Torus);
}

void build_materials( ) {
    // Create brass material
    MaterialProperties brass = {
            vec4(0.33f, 0.22f, 0.03f, 1.0f), //ambient
            vec4(0.78f, 0.57f, 0.11f, 1.0f), //diffuse
            vec4(0.99f, 0.91f, 0.81f, 1.0f), //specular
            27.8f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    // Create red plastic material
    MaterialProperties redPlastic = {
            vec4(0.3f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.6f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    // Add materials to Materials vector
    Materials.push_back(brass);
    Materials.push_back(redPlastic);

    // Create uniform buffer for materials
    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

void build_lights( ) {
    // Spot white light
    // LightProperties whiteSpotLight = {
    //         SPOT, //type
    //         {0.0f, 0.0f, 0.0f}, //pad
    //         vec4(0.1f, 0.1f, 0.1f, 1.0f), //ambient
    //         vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
    //         vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
    //         vec4(7.50f, 7.50f, 8.0f, 1.0f),  //position
    //         vec4(-1.0f, -1.0f, -1.0f, 0.0f), //direction
    //         30.0f,   //cutoff
    //         20.0f,  //exponent
    //         {0.0f, 0.0f}  //pad2
    // };

    LightProperties sunLight = {
        DIRECTIONAL, // type
        {0.0f, 0.0f, 0.0f}, // pad
        vec4(0.2f, 0.2f, 0.2f, 1.0f), // ambient
        vec4(1.0f, 1.0f, 0.9f, 1.0f), // diffuse
        vec4(1.0f, 1.0f, 0.9f, 1.0f), // specular
        vec4(0.0f, 15.0f, 0.0f, 1.0f), // position (not used for directional light)
        vec4(-0.0f, -1.0f, -0.0f, 0.0f), // direction
        0.0f, // cutoff (not used for directional light)
        0.0f, // exponent (not used for directional light)
        {0.0f, 0.0f} // pad2
    };

    // Add lights to Lights vector
    //Lights.push_back(whiteSpotLight);
    Lights.push_back(sunLight);

    // Set numLights
    numLights = Lights.size();

    // Turn all lights on
    for (int i = 0; i < numLights; i++) {
        lightOn[i] = 1;
    }

    // Create uniform buffer for lights
    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
}

void build_shadows( ) {
    // Generate new framebuffer and corresponding texture for storing shadow distances
    glGenFramebuffers(1, &ShadowBuffer);
    glGenTextures(1, &TextureIDs[ShadowTex]);
    // Bind shadow texture and only store depth value
    glBindTexture(GL_TEXTURE_2D, TextureIDs[ShadowTex]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindFramebuffer(GL_FRAMEBUFFER, ShadowBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, TextureIDs[ShadowTex], 0);
    // Buffer is not actually drawn into since only for creating shadow texture
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void load_model(const char * filename, GLuint obj) {
    vector<vec4> vertices;
    vector<vec2> uvCoords;
    vector<vec3> normals;

    // Load model and set number of vertices
    loadOBJ(filename, vertices, uvCoords, normals);
    numVertices[obj] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*texCoords*numVertices[obj], uvCoords.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_mat_shadow_object(GLuint obj, GLuint material){
    // Reference appropriate shader variables
    if (shadow) {
        // Use shadow shader
        glUseProgram(shadow_program);
        // Pass shadow projection and camera matrices to shader
        glUniformMatrix4fv(shadow_proj_mat_loc, 1, GL_FALSE, shadow_proj_matrix);
        glUniformMatrix4fv(shadow_camera_mat_loc, 1, GL_FALSE, shadow_camera_matrix);

        // Set object attributes to shadow shader
        vPos = shadow_vPos;
        model_mat_loc = shadow_model_mat_loc;
    } else {
        // Use lighting shader with shadows
        glUseProgram(phong_shadow_program);

        // Pass object projection and camera matrices to shader
        glUniformMatrix4fv(phong_shadow_proj_mat_loc, 1, GL_FALSE, proj_matrix);
        glUniformMatrix4fv(phong_shadow_camera_mat_loc, 1, GL_FALSE, camera_matrix);

        // Bind lights
        glUniformBlockBinding(phong_shadow_program, phong_shadow_lights_block_idx, 0);
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size() * sizeof(LightProperties));

        // Bind materials
        glUniformBlockBinding(phong_shadow_program, phong_shadow_materials_block_idx, 1);
        glBindBufferRange(GL_UNIFORM_BUFFER, 1, MaterialBuffers[MaterialBuffer], 0,
                          Materials.size() * sizeof(MaterialProperties));

        // Set camera position
        glUniform3fv(phong_shadow_eye_loc, 1, eye);

        // Set num lights and lightOn
        glUniform1i(phong_shadow_num_lights_loc, Lights.size());
        glUniform1iv(phong_shadow_light_on_loc, numLights, lightOn);

        // Pass normal matrix to shader
        glUniformMatrix4fv(phong_shadow_norm_mat_loc, 1, GL_FALSE, normal_matrix);

        // Pass material index to shader
        glUniform1i(phong_shadow_material_loc, material);

        // TODO: Pass shadow projection and camera matrices
        glUniformMatrix4fv(phong_shadow_shad_proj_mat_loc, 1, GL_FALSE, shadow_proj_matrix);
        glUniformMatrix4fv(phong_shadow_shad_cam_mat_loc, 1, GL_FALSE, shadow_camera_matrix);

        // TODO: Bind shadow texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureIDs[ShadowTex]);

        // Set object attributes for phong shadow shader
        vPos = phong_shadow_vPos;
        vNorm = phong_shadow_vNorm;
        model_mat_loc = phong_shadow_model_mat_loc;
    }

    // Pass model matrix to shader
    glUniformMatrix4fv(model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(vPos);

    if (!shadow) {
        // Bind object normal buffer if using phong shadow shader
        glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
        glVertexAttribPointer(vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(vNorm);
    }

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}
//Use this function to check for walls
bool can_move(int x, int y){
    //Only allow the player to move into a space with the value 0
    if (wall_loc[y][x] == 0)
        return true;
    return false;
}

void move_player(int x, int y){
    if (can_move(x,y)){
        wall_loc[y][x] = 2; //Set new position to have player in it.
        wall_loc[player_y][player_x] = 0; // Update previous position to be empty
        //Move the player's cube based on whether the x and y are new values
        if (player_x != x){
            cube_pos[0] -= float(x) - float(player_x);
            player_x = x;
        }
        if (player_y != y){
            cube_pos[2] -= float(y) - float(player_y);
            player_y = y;
        }
    }
    else{
        printf("There is a wall here. The player cannot move here");
    }

}

void generate_spiral_movement() {
    int top = 0;
    int bottom = grid_size - 2;
    int left = 0;
    int right = grid_size - 2;

    while (top <= bottom && left <= right) {
        // Traverse from left to right along the top row
        for (int j = left; j <= right; ++j) {
            movement_history.emplace_back(top, j);
        }
        ++top; // Move the top boundary down

        // Traverse from top to bottom along the right column
        for (int i = top; i <= bottom; ++i) {
            movement_history.emplace_back(i, right);
        }
        --right; // Move the right boundary left

        // Traverse from right to left along the bottom row (if still within bounds)
        if (top <= bottom) {
            for (int j = right; j >= left; --j) {
                movement_history.emplace_back(bottom, j);
            }
            --bottom; // Move the bottom boundary up
        }

        // Traverse from bottom to top along the left column (if still within bounds)
        if (left <= right) {
            for (int i = bottom; i >= top; --i) {
                movement_history.emplace_back(i, left);
            }
            ++left; // Move the left boundary right
        }
    }
}

void replay_movement_thread(std::deque<std::pair<int, int>> q) {
    {
        std::lock_guard<std::mutex> lock(replay_mutex);
        if (is_replay_active) {
            return; // Exit if a replay is already active
        }
        is_replay_active = true;
    }

    while (!q.empty()) {
        std::pair<int, int> p = q.front();
        q.pop_front();
        move_player(p.first, p.second);
        sleep(25);
    }

    {
        std::lock_guard<std::mutex> lock(replay_mutex);
        is_replay_active = false; // Mark replay as finished
    }
}

void start_replay() {
    std::lock_guard<std::mutex> lock(replay_mutex);
    if (!is_replay_active) {
        std::thread replay_thread(replay_movement_thread, movement_history);
        replay_thread.detach(); // Detach the thread to let it run independently
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // ESC closes window
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        anim = !anim;
    }

    // Adjust azimuth (horizontal rotation)
    if (key == GLFW_KEY_A) {
        azimuth += daz;
        if (azimuth > 360.0) {
            azimuth -= 360.0;
        }
    } else if (key == GLFW_KEY_D) {
        azimuth -= daz;
        if (azimuth < 0.0) {
            azimuth += 360.0;
        }
    }

    // Adjust elevation (vertical rotation)
    if (key == GLFW_KEY_W) {
        elevation += del;
        if (elevation > 89.0) { // Limit elevation to avoid flipping
            elevation = 89.0;
        }
    } else if (key == GLFW_KEY_S) {
        elevation -= del;
        if (elevation < -89.0) {
            elevation = -89.0;
        }
    }

    // Adjust radius (zoom)
    if (key == GLFW_KEY_X) {
        radius += dr;
    } else if (key == GLFW_KEY_Z) {
        radius -= dr;
        if (radius < min_radius) {
            radius = min_radius;
        }
    }

    // Recalculate eye position based on spherical coordinates
    GLfloat x = center[0] + radius * sin(azimuth * DEG2RAD) * cos(elevation * DEG2RAD);
    GLfloat y = center[1] + radius * sin(elevation * DEG2RAD);
    GLfloat z = center[2] + radius * cos(azimuth * DEG2RAD) * cos(elevation * DEG2RAD);
    eye = vec3(x, y, z);

    // Player movement
    if (key == GLFW_KEY_RIGHT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        move_player(player_x - 1, player_y);
    } else if (key == GLFW_KEY_LEFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        move_player(player_x + 1, player_y);
    } else if (key == GLFW_KEY_UP && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        move_player(player_x, player_y - 1);
    } else if (key == GLFW_KEY_DOWN && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        move_player(player_x, player_y + 1);
    }

    print_wall_array();

    // Toggle spotlight
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        lightOn[WhiteSpotLight] = (lightOn[WhiteSpotLight] + 1) % 2;
    }

    // Start replay movement
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        generate_spiral_movement();
        start_replay();
    }
}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);

    ww = width;
    hh = height;
}

// Debug shadow renderer
unsigned int quadVAO = 0;
unsigned int quadVBO;

void renderQuad()
{
    // reset viewport
    glViewport(0, 0, ww, hh);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render Depth map to quad for visual debugging
    // ---------------------------------------------
    glUseProgram(debug_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureIDs[ShadowTex]);
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
