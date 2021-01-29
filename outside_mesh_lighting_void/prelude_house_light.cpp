//I think this loads a camera angle which views a model
/*Successfully compiled using command $ g++ model_life.cpp glad.c -lGL -lglfw3 -Iglad -lX11 -ldl -lpthread -lassimp
  !Update, 27Nov2020 TDD has led me to this compile cmd:
  g++ prelude_house_light.cpp glad.c -lglfw -ldl -lassimp
  on inteli9 10900k, asus hero xii no graphics card
  */
//https://stackoverflow.com/questions/18389211/how-to-animate-a-3d-model-mesh-in-opengl
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <OpenGL_CLASS/Camera_CLASS.h>
#include <OpenGL_CLASS/Shader_CLASS.h>
#include <OpenGL_CLASS/Model_CLASS.h>
#include <OpenGL_CLASS/Mesh_CLASS.h>
#include <iostream>
#include <chrono>
#if defined(__cplusplus) && __cplusplus >= 201703L && defined(__has_include)
#if __has_include(<filesystem>)
#define GHC_USE_STD_FS
#include <filesystem>
namespace fs = std::filesystem;
#endif
#endif
#ifndef GHC_USE_STD_FS
#include <ghc/filesystem.hpp>
namespace fs = ghc::filesystem; //linux install ghc in bash
#endif
namespace chr = std::chrono::_V2;//::steady_clock
using Framerate = std::chrono::duration<chr::steady_clock::rep, std::ratio<1, 60>>;

//You should have a function update() for updating game logic and a render() for rendering.
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void advance_time_in_fixed_timestep();
void object_shader_update(glm::mat4 projection_m4, glm::mat4 view_m4, glm::vec3& light_source_vec3_ref, Shader& object_shader_ref);
void lamp_shader_update(glm::mat4 projection_m4, glm::mat4 view_m4, glm::vec3 lamp_start_position_vec3, glm::vec3 center_pos_vec3, float& radial_position_fl_ref, glm::vec3& light_source_vec3_ref, Shader& lamp_shader_ref);
glm::vec3 updated_orbit_path_vec3(glm::vec3 center_pos_vec3, glm::vec3 planet_pos_vec3, float& radial_position_fl_ref);

// settings
const unsigned int SCR_WIDTH = 1680 * 2 / 3;//1100;
const unsigned int SCR_HEIGHT = 1050 * 2 / 3;//825;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX_fl = SCR_WIDTH / 2.0f;
float lastY_fl = SCR_HEIGHT / 2.0f;
bool isFirstMouse = true;

// timing
auto next_render_clock = chr::steady_clock::now() + Framerate{1};

//scene and models
static int num_of_dummies = 1;
float radial_position_fl = 0.0f;
float& radial_position_fl_ref = radial_position_fl;
glm::mat4 light_m4 = glm::mat4(1.0f);
glm::mat4 model_m4;
glm::vec3 light_color_vec3(1.0f, 1.0f, 1.0f);

int main()
{
	glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Anthropomorphic Army", NULL, NULL);
  if(window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate(); return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {std::cout << "Failed to initialize GLAD" << std::endl; return -1;}
  glEnable(GL_DEPTH_TEST);

  //scene
  Shader lamp_shader("lamp_loading.vs", "lamp_loading.fs");//ourShader0
  Shader object_shader("object_loading.vs", "object_loading.fs");//ourShader1
  Model prelude_model(fs::u8path("prelude_1997/Honda_Prelude_(Mk5)_(BB5)_1997.obj"));//ourModel0
  Model bulb_model(fs::u8path("bulb.obj"));//ourModel1
  Shader& lamp_shader_ref = lamp_shader,
          object_shader_ref = object_shader;
  // bulb_model is drawn by an unspecialized shader(lamp_shader). This shader only
  // gets the view, model and projection matricies - BUT, model is translated
  // to the light source position. this means reflections all have a shader
  // that draws the object which is being reflected, whether the user sees that
  // true object or not.

  std::vector<glm::vec3> dummy_position_vec3_vector(num_of_dummies);
  glm::vec3 light_source_vec3(0.0f);
  glm::vec3 lamp_start_position_vec3(1.0f);
  glm::vec3 center_pos_vec3(0.67f, -0.5f, 0.0f);
  glm::vec3& light_source_vec3_ref = light_source_vec3;
  glm::mat4 projection_m4, view_m4;
  float dummy_spacing_fl = 1.5f;
  for(int i = 0, row = 0; i < num_of_dummies; i++)
  {//this loop sets up 3 x 3 formation
    // glm::vec3 dummy_formation_vec3 = glm::vec3(0.0f, 0.0f,  0.0f);//(float(row) * dummy_spacing_fl));
    glm::vec3 dummy_formation_vec3 = glm::vec3((float(i % 3) * dummy_spacing_fl), 0.0f, (float(row) * dummy_spacing_fl));
    dummy_position_vec3_vector[i] = dummy_formation_vec3 + center_pos_vec3;
    if(i > 0 && ((i % 3) == 0)){row += 1;}
  }
  // std::cout << "Trying to print model before loop: " << 1.0f << "\n";
	while(!glfwWindowShouldClose(window))
  {
    processInput(window);

    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projection_m4 = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    view_m4 = camera.GetViewMatrix();

    lamp_shader_update(projection_m4, view_m4, lamp_start_position_vec3,
      dummy_position_vec3_vector[0], radial_position_fl_ref, light_source_vec3_ref, lamp_shader_ref);
    bulb_model.Draw(lamp_shader);

    object_shader_update(projection_m4, view_m4, light_source_vec3_ref, object_shader_ref);
    for(int i = 0; i < num_of_dummies; i++)
    {
			float theta = -(3.14159f / 2);
			model_m4 = glm::translate(glm::mat4(1.0f), center_pos_vec3);
      // model_m4 = glm::rotate(model_m4,  theta, glm::vec3(1.0f, 0.0f, 0.0f));
      model_m4 = glm::translate(model_m4, dummy_position_vec3_vector[i]);
			model_m4 = glm::scale(model_m4, glm::vec3(0.00125));
      // model_m4 = glm::scale(model_m4, glm::vec3(0.25));
      object_shader.setMat4("model", model_m4);
      prelude_model.Draw(object_shader);
    }

    advance_time_in_fixed_timestep();

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

	glfwTerminate();
	return 0;
}

glm::vec3 updated_orbit_path_vec3(glm::vec3 center_pos_vec3, glm::vec3 planet_pos_vec3, float& radial_position_fl_ref)
{//this function will keep obj in an orbit around center
  //rotation of the light bulb about the car
  float x_position_fl, z_position_fl, theta_delta_fl;
  theta_delta_fl = 0.005f;
  radial_position_fl_ref = radial_position_fl_ref + theta_delta_fl;
  if(radial_position_fl_ref >= (2.0f * 3.14159f))
  {radial_position_fl_ref = radial_position_fl_ref - (2.0f * 3.14159f);}

  glm::vec3 orbit_center_vec3((center_pos_vec3.x + 0.0f), center_pos_vec3.y, (center_pos_vec3.z + 0.0f));
  x_position_fl = orbit_center_vec3.x +
    (cos(radial_position_fl_ref) * (planet_pos_vec3.x - orbit_center_vec3.x)) -
    (sin(radial_position_fl_ref) * (planet_pos_vec3.z - orbit_center_vec3.z));
  z_position_fl = orbit_center_vec3.z +
    (sin(radial_position_fl_ref) * (planet_pos_vec3.x - orbit_center_vec3.x)) -
    (cos(radial_position_fl_ref) * (planet_pos_vec3.z - orbit_center_vec3.z));
  return glm::vec3(x_position_fl, 0.0f, z_position_fl);//return the rotated vector?? #notamathboy
}

void advance_time_in_fixed_timestep()
{
  while(chr::steady_clock::now() < next_render_clock);
  //"busy" loop - chrono just observes a pre-set time window
  next_render_clock += Framerate{1}; //this is a singleton window value
}

void object_shader_update(glm::mat4 projection_m4, glm::mat4 view_m4, glm::vec3& light_source_vec3_ref, Shader& object_shader_ref)
{//this function updates an obect shader's matricies and vectors
  glm::vec3 light_color_vec3(1.0f, 1.0f, 1.0f);
  glm::vec3 object_color_vec3(1.0f, 0.5f, 0.31f);//orange

  object_shader_ref.use();
  object_shader_ref.setVec3("lightColor", light_color_vec3);
  object_shader_ref.setVec3("lightPos", light_source_vec3_ref);//used to calculate viewspace for reflection
  object_shader_ref.setVec3("viewerPos", glm::vec3(0.0f));// camera.Position);
  object_shader_ref.setVec3("objectColor", object_color_vec3);//lightPos
  object_shader_ref.setMat4("projection", projection_m4);
  object_shader_ref.setMat4("view", view_m4);
}

void lamp_shader_update(glm::mat4 projection_m4, glm::mat4 view_m4, glm::vec3 lamp_start_position_vec3, glm::vec3 center_pos_vec3, float& radial_position_fl_ref, glm::vec3& light_source_vec3_ref, Shader& lamp_shader_ref)
{//this function will render lights for a single frame
  lamp_shader_ref.use();
  light_source_vec3_ref = updated_orbit_path_vec3(center_pos_vec3, lamp_start_position_vec3, radial_position_fl);
  light_m4 = glm::translate(glm::mat4(1.0f), center_pos_vec3);
  light_m4 = glm::translate(light_m4, light_source_vec3_ref);//this is a translation to light_position then rotation
  light_m4 = glm::scale(light_m4, glm::vec3(0.00025f));// it's a bit too big for our scene, so scale it down

  lamp_shader_ref.setMat4("projection", projection_m4);
  lamp_shader_ref.setMat4("view", view_m4);
  lamp_shader_ref.setMat4("model", light_m4);
  //lamp_shader_ref.setVec3("aPos", light_source_vec3_ref);
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {glfwSetWindowShouldClose(window, true);}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  if (isFirstMouse)
  {
    lastX_fl = xpos;
    lastY_fl = ypos;
    isFirstMouse = false;
  }
  float xoffset = xpos - lastX_fl;
  float yoffset = lastY_fl - ypos; // reversed since y-coordinates go from bottom to top
  lastX_fl = xpos;
  lastY_fl = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}
