#pragma once
#include "VSE/fwd.h"
#include "VSE/types.h"
#include "VSE/dictionary.h"

typedef enum
{
	VSE_UNIFORM_FLOAT,
	VSE_UNIFORM_MAT4F
} VSE_UniformType;


typedef struct VSE_UniformTypeValuePair
{
	VSE_UniformType uniformType;
	void *uniformValue;
} VSE_UniformTypeValuePair;


typedef struct VSE_Material
{
	VSE_GLuint shaderProgram;
	VSE_Dictionary *materialUniforms;
} VSE_Material;


/** Compiles a shader pair into a material.
 *  @param fragShaderName path under shaderRoot, or NULL for the default
 *  @param vertShaderName path under shaderRoot, or NULL for the default
 *  @return a material owning its program; not shared between entities */
VSE_Material *VSE_CreateMaterial(char *fragShaderName, char *vertShaderName);

/** Binds a uniform to a value BY POINTER, so the shader tracks whatever the
 *  pointer refers to. The pointed-at value must outlive the material. */
void VSE_AddUniformToMaterial(VSE_Material *material, char *uniformName, VSE_UniformType uniformType, void *value);
