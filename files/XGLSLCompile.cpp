/***********************************************************************
  @author Kritesh tripathi
  

  Camera Stream Source

 * @file XGLSLCompile.cpp
 *
 * This file implements all the functions related to GLSL compilation.
 ***********************************************************************/
#include <iostream>
#include <cstdlib>

#include "XGLSLCompile.h"

using std::cout;
using std::endl;

char* Shader::loadShader(const char *filename)
{
	FILE *file = fopen(filename, "rb");
	if(file == NULL)
	{
		cout << "Cannot read file " << filename << endl;
		exit(1);
	}
	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET); 
	char *shader = (char *)calloc(length + 1, sizeof(char));
	if(shader == NULL)
	{
		cout << "Out of memory at " << __FILE__ << __LINE__ << endl;
		exit(1);
	}
	size_t numberOfBytesRead = fread(shader, sizeof(char), length, file);
	if (numberOfBytesRead != length) 
	{
		cout << "Error reading " << filename << " read bytes number not consistent " << numberOfBytesRead << " != " << length << endl;
		exit(1);
	}
	shader[length] = '\0';
	fclose(file);

	return shader;
}

void Shader::processShader(GLuint *shader, const char *filename, GLint shaderType)
{  
	const char *strings[1] = { NULL };

	*shader = glCreateShader(shaderType);
	strings[0] = loadShader(filename);
	glShaderSource(*shader, 1, strings, NULL);

	free((void *)(strings[0]));
	strings[0] = NULL;

	glCompileShader(*shader);
	GLint status;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);

	if(status != GL_TRUE) 
	{
		GLint length;
		char *debugSource = NULL;
		char *errorLog = NULL;

		glGetShaderiv(*shader, GL_SHADER_SOURCE_LENGTH, &length);
		debugSource = (char *)malloc(length);
		glGetShaderSource(*shader, length, NULL, debugSource);
		cout << "Debug source START:" << endl << debugSource << endl << "Debug source END" << endl;
		free(debugSource);

		glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &length);
		errorLog = (char *)malloc(length);
		glGetShaderInfoLog(*shader, length, NULL, errorLog);
		cout << "Log START:" << endl << errorLog << endl << "Log END" << endl;
		free(errorLog);

		cout << "Compilation FAILED!" << endl;
		exit(1);
	}
}

void Shader::processShaderStr(GLuint *shader, const char *shaderString, GLint shaderType)
{  
	const char *strings[1] = { NULL };

	*shader = glCreateShader(shaderType);
	strings[0] = shaderString;
	glShaderSource(*shader, 1, strings, NULL);

	strings[0] = NULL;

	glCompileShader(*shader);
	GLint status;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);

	if(status != GL_TRUE) 
	{
		GLint length;
		char *debugSource = NULL;
		char *errorLog = NULL;

		glGetShaderiv(*shader, GL_SHADER_SOURCE_LENGTH, &length);
		debugSource = (char *)malloc(length);
		glGetShaderSource(*shader, length, NULL, debugSource);
		cout << "Debug source START:" << endl << debugSource << endl << "Debug source END" << endl;
		free(debugSource);

		glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &length);
		errorLog = (char *)malloc(length);
		glGetShaderInfoLog(*shader, length, NULL, errorLog);
		cout << "Log START:" << endl << errorLog << endl << "Log END" << endl;
		free(errorLog);

		cout << "Compilation FAILED!" << endl;
		exit(1);
	}
}
