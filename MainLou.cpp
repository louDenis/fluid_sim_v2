#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "PerlinNoise.h"
#include "fluid.h"
#include "fluid_utils.h"
#include "Layers.h"
#include "User.h"


//Fluid* fluid;
Layers* layers;

//------------------------------SHADERS------------------------------

const char* vertexShaderSource =
"#version 330 core \n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec2 tPos;\n"

"out vec2 texturePos;\n"

"void main() { \n"
"gl_Position = vec4(aPos, 1.0f);\n"
"texturePos = tPos;\n"
"}\n \0";


const char* fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"uniform sampler2D backgroundTexture;\n"
"uniform sampler2D brushTexture;"
"in vec2 texturePos;\n"

"void main()\n"
"{\n"
"	//FragColor = texture(backgroundTexture, texturePos);\n"
" vec3 temp1 = vec3(texture(brushTexture, texturePos));\n"
" vec3 temp2 = vec3(texture(backgroundTexture, texturePos));\n"
" FragColor = vec4(temp2-temp1, 1.0f); \n"
"}\n \0";
//-------------------------------------------------------------------




void framebuffer_size_callback(GLFWwindow* win, int width, int height);
void processInput(GLFWwindow* window);
std::string readFile(const char* path);

int width = SCALE * N, height = SCALE * N;

float vertices[] = {
	-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
	 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
	 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f
};

unsigned int indices[] = {
	0, 1, 3,
	1, 2, 3
};

uint8_t* background;
uint8_t* brush;
User* user;
double x, y;

int main(int argc, char** argv) {

	user= new User();

	//FLUID DECLARATION
	//fluid = new Fluid(0.2, 0, 0.0000001);

	//LAYERS DECLARATION
	layers = new Layers(0.2, 0, 0.0000001);
	
	

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(width, height, "aled", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);


	// build and compile our shader program
	// ------------------------------------
	// vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check for shader compile errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// link shaders
	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);



	//GPU buffers
	unsigned int VBO, VAO, EBO;
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//texture initialisation
	PerlinNoise pn = PerlinNoise();
	background = (uint8_t*)malloc(width * height * 4 * sizeof(uint8_t));
	for (int l = 0; l < height; l++) {
		for (int c = 0; c < width; c++) {
			double y = (double)l / (double)height;
			double x = (double)c / (double)width;
			double noise = pn.noise(x * 1000, y * 1000, 0.8f) * 125 + 125;
			background[(l * width * 4) + (c * 4) + 0] = (uint8_t)noise;
			background[(l * width * 4) + (c * 4) + 1] = (uint8_t)noise;
			background[(l * width * 4) + (c * 4) + 2] = (uint8_t)noise;
			background[(l * width * 4) + (c * 4) + 3] = (uint8_t)255;
		}
	}


	int brush_width = SCALE;
	int brush_height = SCALE;

	//code antoine brush= canva et texture=background, et canva a 4 dim
	brush = (uint8_t*)malloc(brush_width * brush_height * 3 * sizeof(uint8_t));


	unsigned int backTextureID;
	glGenTextures(1, &backTextureID);
	glBindTexture(GL_TEXTURE_2D, backTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, background);
	glGenerateMipmap(GL_TEXTURE_2D);

	unsigned int brushTextureID;
	glGenTextures(1, &brushTextureID);
	glBindTexture(GL_TEXTURE_2D, brushTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, brush);


	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "backgroundTexture"), 0);
	glUniform1i(glGetUniformLocation(shaderProgram, "brushTexture"), 1);

	glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE_MINUS_SRC_ALPHA);



	glfwSwapBuffers(window);
	glfwPollEvents();
	float deltaT = 0.0f;
	
	while (!glfwWindowShouldClose(window)) {
		float timeStart = glfwGetTime();
		processInput(window);
		glClearColor(1.0f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		//layers->wetLayer->sendTexture(shaderProgram, 0, "wetTexture");
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		//int id = glGetUniformLocation(shaderProgram, "ourTexture");
		//glUniform1i(id, 0);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glfwSwapBuffers(window);
		glfwPollEvents();
		layers->wetLayer->step(deltaT);
		float timeEnd = glfwGetTime();
		deltaT = timeEnd - timeStart;


		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				int h = i * SCALE;
				int w = j * SCALE;

				for (int line = 0; line < brush_height; line++) {
					for (int col = 0; col < brush_width; col++) {
						//new Layer
						/*brush[(line * brush_width * 3) + (col * 3) + 0] = (uint8_t)layers->computeHeight(i, j);
						brush[(line * brush_width * 3) + (col * 3) + 1] = (uint8_t)layers->computeHeight(i,j);
						brush[(line * brush_width * 3) + (col * 3) + 2] = (uint8_t)layers->computeHeight(i, j);*/

						
						//dry layer
						brush[(line * brush_width * 3) + (col * 3) + 0] = (uint8_t)layers->wetLayer->getDensity(i, j);
						brush[(line * brush_width * 3) + (col * 3) + 1] = (uint8_t)layers->wetLayer->getDensity(i, j);
						brush[(line * brush_width * 3) + (col * 3) + 2] = (uint8_t)layers->wetLayer->getDensity(i, j);

						//wet layer
						/*brush[(line * brush_width * 3) + (col * 3) + 0] = (uint8_t)layers->newLayer->getDensity(i, j);
						brush[(line * brush_width * 3) + (col * 3) + 1] = (uint8_t)layers->newLayer->getDensity(i, j);
						brush[(line * brush_width * 3) + (col * 3) + 2] = (uint8_t)layers->newLayer->getDensity(i, j);
						*/
					}
				}

				//creer une 2eme texture
				//texture de base: perlin 


				//glTexSubImage2D(GL_TEXTURE_2D, 0, w, height - h, SCALE, SCALE, GL_RGB, GL_UNSIGNED_BYTE, background);
				glTexSubImage2D(GL_TEXTURE_2D, 0, w, height - h, SCALE, SCALE, GL_RGB, GL_UNSIGNED_BYTE, brush);
				glGenerateMipmap(GL_TEXTURE_2D);

			}
		}
	}

	glfwTerminate();
	return 0;
}


void framebuffer_size_callback(GLFWwindow* win, int width, int height) {
	glViewport(0, 0, width, height);
}


void processInput(GLFWwindow* window) {
	int previousX = x;
	int previousY = y;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		glfwGetCursorPos(window, &x, &y);
		std::cout << x << "  " << y << std::endl;
		layers->wetLayer->addDensity(x / SCALE, y / SCALE, 100);

		float amtX = (float)x - previousX;
		float amtY = (float)y - previousY;

		float pressure_coef = 1.0 - (user->pressure);
		if (user->pressure > 1.0)
			std::cout << "PRESSURE VALUE MUST BE LESS THAN 1" << std::endl; 
		layers->wetLayer->addVelocity((x / SCALE) * pressure_coef, (y/ SCALE) * pressure_coef, amtX, amtY);


		

	}

	if (x < 25 && y < 25 && x > 1 && y > 1)
	{
		//std::cout << "DRYYY PARTIALLY" << std::endl;
		//layers->dry_partially(0.25);
	}
}


std::string readFile(const char* path) {
	std::string fileContent;
	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		file.open(path);
		std::stringstream fileStream;
		fileStream << file.rdbuf();
		file.close();
		fileContent = fileStream.str();
	}
	catch (std::ifstream::failure e) {
		std::cout << "ERROR reading file" << std::endl;
	}
	return fileContent;
}

//pour les aspérités : hauteur= hauteur d'eau - b