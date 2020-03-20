/***********************************************************************
  @author Kritesh tripathi
  

  Camera Stream header

 * @file XGLSLCompile.h
 *
 * This file implements all the functions related to GLSL compilation.
 ***********************************************************************/
#ifndef XGLSLCOMPILE_H
#define XGLSLCOMPILE_H

#include <GLES2/gl2.h>

class Shader
{
private:
	static char *loadShader(const char *filename);
public:
	static void processShaderStr(GLuint *shader, const char *shaderString, GLint shaderType);
	static void processShader(GLuint *shader, const char *filename, GLint shaderType);
};
#endif /* XGLSLCOMPILE_H */
