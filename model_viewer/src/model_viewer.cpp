// Assignment 3, Part 1 and 2
//
// Modify this file according to the lab instructions.
//

#include "utils.h"
#include "utils2.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cstdlib>
#include <algorithm>

// The attribute locations we will use in the vertex shader
enum AttributeLocation {
	POSITION = 0,
	NORMAL = 1
};

// Struct for representing an indexed triangle mesh
struct Mesh {
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<uint32_t> indices;
};

// Struct for representing a vertex array object (VAO) created from a
// mesh. Used for rendering.
struct MeshVAO {
	GLuint vao;
	GLuint vertexVBO;
	GLuint normalVBO;
	GLuint indexVBO;
	int numVertices;
	int numIndices;
};

// Struct for resources and state
struct Context {
	int width;
	int height;
	float aspect;
	GLFWwindow *window;
	GLuint program;
	Trackball trackball;
	Mesh mesh;
	MeshVAO meshVAO;
	GLuint defaultVAO;
	GLuint cubemap;
	float elapsed_time;
	float zoom_factor = 1.0;
	float fov = 45.0f;
	std::vector<std::string> textures;
	GLuint cubemap_0;
	GLuint cubemap_1;
	GLuint cubemap_2;
	GLuint cubemap_3;
	GLuint cubemap_4;
	GLuint cubemap_5;
	GLuint cubemap_6;
	GLuint cubemap_7;
	bool gamma_correction = true;
	bool ortho = 0;
	glm::vec3 bg_colour = glm::vec3(0.2f, 0.0f, 0.2f);
	glm::vec3 light_colour = glm::vec3(0.78f, 0.78f, 0.78f);
	glm::vec3 light_position = glm::vec3(4.9f, 4.72f, 1.9f);
	glm::vec3 diffuse_colour = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 ambient_colour = glm::vec3(0.22f, 0.22f, 0.22f);

	glm::vec3 specular_colour = glm::vec3(0.69f, 0.69f, 0.69f);

	float specular_power = 50.0f;
};

// Returns the value of an environment variable
std::string getEnvVar(const std::string &name)
{
	char const* value = std::getenv(name.c_str());
	if (value == nullptr) {
		return std::string();
	}
	else {
		return std::string(value);
	}
}

// Returns the absolute path to the shader directory
std::string shaderDir(void)
{
	std::string rootDir = getEnvVar("ASSIGNMENT3_ROOT");
	if (rootDir.empty()) {
		std::cout << "Error: ASSIGNMENT3_ROOT is not set." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return rootDir + "/src/shaders/";
}

// Returns the absolute path to the 3D model directory
std::string modelDir(void)
{
	std::string rootDir = getEnvVar("ASSIGNMENT3_ROOT");
	if (rootDir.empty()) {
		std::cout << "Error: ASSIGNMENT3_ROOT is not set." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return rootDir + "/3d_models/";
}

// Returns the absolute path to the cubemap texture directory
std::string cubemapDir(void)
{
	std::string rootDir = getEnvVar("ASSIGNMENT3_ROOT");
	if (rootDir.empty()) {
		std::cout << "Error: ASSIGNMENT3_ROOT is not set." << std::endl;
		std::exit(EXIT_FAILURE);
	}
	return rootDir + "/cubemaps/";
}

void loadMesh(const std::string &filename, Mesh *mesh)
{
	OBJMesh obj_mesh;
	objMeshLoad(obj_mesh, filename);
	mesh->vertices = obj_mesh.vertices;
	mesh->normals = obj_mesh.normals;
	mesh->indices = obj_mesh.indices;
}

void createMeshVAO(Context &ctx, const Mesh &mesh, MeshVAO *meshVAO)
{
	// Generates and populates a VBO for the vertices
	glGenBuffers(1, &(meshVAO->vertexVBO));
	glBindBuffer(GL_ARRAY_BUFFER, meshVAO->vertexVBO);
	auto verticesNBytes = mesh.vertices.size() * sizeof(mesh.vertices[0]);
	glBufferData(GL_ARRAY_BUFFER, verticesNBytes, mesh.vertices.data(), GL_STATIC_DRAW);

	// Generates and populates a VBO for the vertex normals
	glGenBuffers(1, &(meshVAO->normalVBO));
	glBindBuffer(GL_ARRAY_BUFFER, meshVAO->normalVBO);
	auto normalsNBytes = mesh.normals.size() * sizeof(mesh.normals[0]);
	glBufferData(GL_ARRAY_BUFFER, normalsNBytes, mesh.normals.data(), GL_STATIC_DRAW);

	// Generates and populates a VBO for the element indices
	glGenBuffers(1, &(meshVAO->indexVBO));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshVAO->indexVBO);
	auto indicesNBytes = mesh.indices.size() * sizeof(mesh.indices[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesNBytes, mesh.indices.data(), GL_STATIC_DRAW);

	// Creates a vertex array object (VAO) for drawing the mesh
	glGenVertexArrays(1, &(meshVAO->vao));
	glBindVertexArray(meshVAO->vao);
	glBindBuffer(GL_ARRAY_BUFFER, meshVAO->vertexVBO);
	glEnableVertexAttribArray(POSITION);
	glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ARRAY_BUFFER, meshVAO->normalVBO);
	glEnableVertexAttribArray(NORMAL);
	glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshVAO->indexVBO);
	glBindVertexArray(ctx.defaultVAO); // unbinds the VAO

									   // Additional information required by draw calls
	meshVAO->numVertices = mesh.vertices.size();
	meshVAO->numIndices = mesh.indices.size();
}

void initializeTrackball(Context &ctx)
{
	double radius = double(std::min(ctx.width, ctx.height)) / 2.0;
	ctx.trackball.radius = radius;
	glm::vec2 center = glm::vec2(ctx.width, ctx.height) / 2.0f;
	ctx.trackball.center = center;
}

void init(Context &ctx)
{
	ctx.program = loadShaderProgram(shaderDir() + "mesh.vert",
		shaderDir() + "mesh.frag");

	loadMesh((modelDir() + "gargo.obj"), &ctx.mesh);
	createMeshVAO(ctx, ctx.mesh, &ctx.meshVAO);

	// Load cubemap texture(s)


	ctx.cubemap_0 = loadCubemap(cubemapDir() + "/Forrest/prefiltered/0.125");
	ctx.cubemap_1 = loadCubemap(cubemapDir() + "/Forrest/prefiltered/0.5");
	ctx.cubemap_2 = loadCubemap(cubemapDir() + "/Forrest/prefiltered/2");
	ctx.cubemap_3 = loadCubemap(cubemapDir() + "/Forrest/prefiltered/8");
	ctx.cubemap_4 = loadCubemap(cubemapDir() + "/Forrest/prefiltered/32");
	ctx.cubemap_5 = loadCubemap(cubemapDir() + "/Forrest/prefiltered/128");
	ctx.cubemap_6 = loadCubemap(cubemapDir() + "/Forrest/prefiltered/512");
	ctx.cubemap_7 = loadCubemap(cubemapDir() + "/Forrest/prefiltered/2048");
	//ctx.cubemap = loadCubemap(cubemapDir() + "/Forrest/");

	initializeTrackball(ctx);
}

// MODIFY THIS FUNCTION
void drawMesh(Context &ctx, GLuint program, const MeshVAO &meshVAO)
{
	// Define uniforms
	glm::mat4 model = trackballGetRotationMatrix(ctx.trackball);
	glm::mat4 view = glm::mat4();
	glm::mat4 projection = glm::ortho(-ctx.aspect, ctx.aspect, -1.0f, 1.0f, -1.0f, 1.0f);

	view = glm::lookAt(glm::vec3(0.0f, 0.01f, 5.0f), glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f));

	if (!ctx.ortho) {
		projection = glm::perspective(ctx.zoom_factor*45.0f, 1.0f, 0.1f, 100.0f);
	}
	else {
		projection = glm::ortho(-ctx.zoom_factor * 5, ctx.zoom_factor * 5, -ctx.zoom_factor * 5, ctx.zoom_factor * 5, 0.1f, 100.0f);
	}
	glm::mat4 trackMatrix = trackballGetRotationMatrix(ctx.trackball);

	model = trackMatrix;
	glm::mat4 mv = view * model;
	glm::mat4 mvp = projection * mv*model;
	// Set light

	//glm::vec3 light_position = glm::vec3(4.9f, 4.72f, 1.9f);
	//ctx.light_colour = glm::vec3(0.78f, 0.78f, 0.78f);
	//ambient_color, diffuse_color, specular_color, and specular_power.

	glm::vec3 ambient_colour = glm::vec3(0.22f, 0.22f, 0.22f);

	glm::vec3 specular_colour = glm::vec3(0.69f, 0.69f, 0.69f);

	float specular_power = 50.0f;

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ctx.cubemap_1);





	// Activate program
	glUseProgram(program);

	// Bind textures
	// ...

	// Pass uniforms
	glUniformMatrix4fv(glGetUniformLocation(program, "u_mv"), 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, "u_mvp"), 1, GL_FALSE, &mvp[0][0]);
	glUniform1f(glGetUniformLocation(program, "u_time"), ctx.elapsed_time);
	glUniform3f(glGetUniformLocation(program, "u_light_position"), ctx.light_position.x, ctx.light_position.y, ctx.light_position.z);
	glUniform3f(glGetUniformLocation(program, "u_light_colour"), ctx.light_colour.x, ctx.light_colour.y, ctx.light_colour.z);
	glUniform3f(glGetUniformLocation(program, "u_diffuse_colour"), ctx.diffuse_colour.x, ctx.diffuse_colour.y, ctx.diffuse_colour.z);
	glUniform3f(glGetUniformLocation(program, "u_specular_colour"), ctx.specular_colour.x, ctx.specular_colour.y, ctx.specular_colour.z);
	glUniform3f(glGetUniformLocation(program, "u_ambient_colour"), ctx.ambient_colour.x, ctx.ambient_colour.y, ctx.ambient_colour.z);
	glUniform1f(glGetUniformLocation(program, "u_specular_power"), ctx.specular_power);
	glUniform1i(glGetUniformLocation(program, "u_cubemap"), 2);
	glUniform1i(glGetUniformLocation(program, "gamma_correction"), ctx.gamma_correction);

	// Draw!
	glBindVertexArray(meshVAO.vao);
	glDrawElements(GL_TRIANGLES, meshVAO.numIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(ctx.defaultVAO);
}


void showGui(Context &ctx)
{
	/*if (ImGui::SliderInt("Max bounces", &ctx.rtx.max_bounces, 0, 10)) {
	rt::resetAccumulation(ctx.rtx);
	}
	if (ImGui::ColorEdit3("Sky color", &ctx.rtx.sky_color[0])) {
	rt::resetAccumulation(ctx.rtx);
	}
	if (ImGui::ColorEdit3("Ground color", &ctx.rtx.ground_color[0])) {
	rt::resetAccumulation(ctx.rtx);
	}
	if (ImGui::Checkbox("Show normals", &ctx.rtx.show_normals)) {
	rt::resetAccumulation(ctx.rtx);
	}
	// Add more settings and parameters here
	// ...*/
	ImGui::ColorEdit3("Diffused colour", &ctx.diffuse_colour[0]);
	ImGui::ColorEdit3("Light colour", &ctx.light_colour[0]);
	ImGui::ColorEdit3("Light position", &ctx.light_position[0]);
	ImGui::ColorEdit3("Background colour", &ctx.bg_colour[0]);
	ImGui::Checkbox("Gamma correction", &ctx.gamma_correction);
	ImGui::Checkbox("Orthoprojection", &ctx.ortho);
	ImGui::ColorEdit3("Ambient colour", &ctx.ambient_colour[0]);
	ImGui::ColorEdit3("Specular colour", &ctx.specular_colour[0]);
	ImGui::SliderFloat("Specular power", &ctx.specular_power, 1, 10);

}

void display(Context &ctx)
{
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST); // ensures that polygons overlap correctly
	drawMesh(ctx, ctx.program, ctx.meshVAO);
	showGui(ctx);

}

void reloadShaders(Context *ctx)
{
	glDeleteProgram(ctx->program);
	ctx->program = loadShaderProgram(shaderDir() + "mesh.vert",
		shaderDir() + "mesh.frag");
}

void mouseButtonPressed(Context *ctx, int button, int x, int y)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		ctx->trackball.center = glm::vec2(x, y);
		trackballStartTracking(ctx->trackball, glm::vec2(x, y));
	}
	if (button == GLFW_MOUSE_BUTTON_3) {
		ctx->zoom_factor -= 0.1f;
	}
	if (button == GLFW_MOUSE_BUTTON_4) {
		ctx->zoom_factor -= 0.1f;
	}
}

void mouseButtonReleased(Context *ctx, int button, int x, int y)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		trackballStopTracking(ctx->trackball);
	}
}

void moveTrackball(Context *ctx, int x, int y)
{
	if (ctx->trackball.tracking) {
		trackballMove(ctx->trackball, glm::vec2(x, y));
	}
}

void errorCallback(int /*error*/, const char* description)
{
	std::cerr << description << std::endl;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Forward event to GUI
	ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
	if (ImGui::GetIO().WantCaptureKeyboard) { return; }  // Skip other handling

	Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		reloadShaders(ctx);
	}
}

void charCallback(GLFWwindow* window, unsigned int codepoint)
{
	// Forward event to GUI
	ImGui_ImplGlfwGL3_CharCallback(window, codepoint);
	if (ImGui::GetIO().WantTextInput) { return; }  // Skip other handling
}

void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{

	/*if (ctx.FoV >= 1.0f && fov <= 45.0f)
	fov -= yoffset;
	if (fov <= 1.0f)
	fov = 1.0f;
	if (fov >= 45.0f)
	fov = 45.0f;*/
}
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	// Forward event to GUI
	ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);
	if (ImGui::GetIO().WantCaptureMouse) { return; }  // Skip other handling

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
	if (action == GLFW_PRESS) {
		mouseButtonPressed(ctx, button, x, y);
	}
	else {
		mouseButtonReleased(ctx, button, x, y);
	}
}

void cursorPosCallback(GLFWwindow* window, double x, double y)
{
	if (ImGui::GetIO().WantCaptureMouse) { return; }  // Skip other handling   

	Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
	moveTrackball(ctx, x, y);
}

void resizeCallback(GLFWwindow* window, int width, int height)
{
	Context *ctx = static_cast<Context *>(glfwGetWindowUserPointer(window));
	ctx->width = width;
	ctx->height = height;
	ctx->aspect = float(width) / float(height);
	ctx->trackball.radius = double(std::min(width, height)) / 2.0;
	ctx->trackball.center = glm::vec2(width, height) / 2.0f;
	glViewport(0, 0, width, height);
}

int main(void)
{
	Context ctx;

	// Create a GLFW window
	glfwSetErrorCallback(errorCallback);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	ctx.width = 500;
	ctx.height = 500;
	ctx.aspect = float(ctx.width) / float(ctx.height);
	ctx.window = glfwCreateWindow(ctx.width, ctx.height, "Model viewer", nullptr, nullptr);
	glfwMakeContextCurrent(ctx.window);
	glfwSetWindowUserPointer(ctx.window, &ctx);
	glfwSetKeyCallback(ctx.window, keyCallback);
	glfwSetCharCallback(ctx.window, charCallback);
	glfwSetMouseButtonCallback(ctx.window, mouseButtonCallback);
	glfwSetCursorPosCallback(ctx.window, cursorPosCallback);
	glfwSetFramebufferSizeCallback(ctx.window, resizeCallback);

	// Load OpenGL functions
	glewExperimental = true;
	GLenum status = glewInit();
	if (status != GLEW_OK) {
		std::cerr << "Error: " << glewGetErrorString(status) << std::endl;
		std::exit(EXIT_FAILURE);
	}
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	// Initialize GUI
	ImGui_ImplGlfwGL3_Init(ctx.window, false /*do not install callbacks*/);

	// Initialize rendering
	glGenVertexArrays(1, &ctx.defaultVAO);
	glBindVertexArray(ctx.defaultVAO);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	init(ctx);

	// Start rendering loop
	while (!glfwWindowShouldClose(ctx.window)) {
		glfwPollEvents();
		ctx.elapsed_time = glfwGetTime();
		ImGui_ImplGlfwGL3_NewFrame();
		display(ctx);
		ImGui::Render();
		glfwSwapBuffers(ctx.window);
	}

	// Shutdown
	glfwDestroyWindow(ctx.window);
	glfwTerminate();
	std::exit(EXIT_SUCCESS);
}
