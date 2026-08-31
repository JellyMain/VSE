#pragma once
#include <SDL.h>
#include <stdbool.h>
#include "VSE/fwd.h"
#include "VSE/types.h"
#include "VSE/material.h"
#include "VSE/dictionary.h"
#include "VSE/ui.h"
#include "VSE/window.h"
#include "VSE/component.h"
#include "VSE/update.h"

typedef struct VSE_Renderer
{
	VSE_GLuint entitiesVAO;
	VSE_GLuint postProcessingVAO;
	VSE_GLuint postProcessingVBO;
	VSE_GLuint gizmosVAO;
	VSE_GLuint VBO;
	VSE_GLuint EBO;
	VSE_Material *defaultMaterial;
	VSE_Material *defaultGizmoMaterial;
	VSE_Material *defaultPostProcessingMaterial;
	VSE_Dictionary *postProcessingEffects;
} VSE_Renderer;


/** The per-frame render system, drawing every window. */
VSE_Updatable *VSE_CreateRenderUpdatable();

/** Renders one frame of every window. Registered via VSE_AddDefaultUpdatables. */
void VSE_UpdateRenderer(void *data, VSE_Engine *engine, float deltaTime);

/** Pushes every uniform bound to this material into its shader program. */
void VSE_UpdateMaterialUniforms(VSE_Material *material);

/** Writes a uniform once. For a value that changes, use VSE_AddUniformToMaterial. */
void VSE_SetShaderUniform(VSE_GLuint shaderProgram, char *propertyName, VSE_UniformType uniformType,
                      void *value);

/** Draws a filled debug rectangle in the current gizmo material. */
void VSE_DrawGizmoRect(VSE_Renderer *renderer, float x, float y, float width, float height);

/** Draws a debug outline of the given thickness around a position. */
void VSE_RenderHollowGizmoRect(VSE_Engine *engine, VSE_Vector2Float position, VSE_Vector2Float size, float thickness);

/** Draws text from a glyph atlas, so the string may change every frame.
 *  Use for counters; use VSE_CreateStaticText for fixed labels. */
void VSE_RenderDynamicText(VSE_TextAtlas *textAtlas, char *text, VSE_Vector2Float position, VSE_Vector2Float scale,
                       VSE_DynamicTextHorizontalAlignment horizontalAlignment, VSE_Renderer *renderer,
                       float projectionMatrix[16]);

/** Appends a fullscreen effect. Effects apply in registration order.
 *  @param material built by the caller, so your own effects work like the built-ins */
void VSE_AddPostProcessingEffect(VSE_Engine *engine, char *effectName, VSE_Material *material);

/** Draws this entity in one specific window only. */
void VSE_AddGameEntityToDrawList(VSE_Window *window, VSE_Entity *entity);

/** Stops drawing this entity anywhere. */
void VSE_RemoveGameEntityFromAllDrawLists(VSE_Engine *engine, VSE_Entity *entity);

/** Draws this UI entity in one specific window. */
void VSE_AddUIEntityToDrawList(VSE_Window *window, VSE_Entity *entity);

/** Draws this entity in every window that exists right now. */
void VSE_AddGameEntityToAllDrawLists(VSE_Engine *engine, VSE_Entity *entity);

/** Draws this UI entity in every window that exists right now. */
void VSE_AddUIEntityToAllDrawLists(VSE_Engine *engine, VSE_Entity *entity);


/** Creates the shared GL context against a hidden window and builds the pipeline. */
void VSE_InitOpenGL(VSE_Engine *engine);

/** Logs the GL vendor, renderer and GLSL version. */
void VSE_PrintOpenGLInfo();

/** Sets up vertex buffers and the default, gizmo and post-processing materials. */
void VSE_InitRenderPipeline(VSE_Engine *engine);

/** Allocates the renderer, its VAOs/VBOs and the built-in materials. */
VSE_Renderer *VSE_CreateRenderer();

/** Compiles and links a shader pair from under shaderRoot. @return the GL program. */
VSE_GLuint VSE_CreateShaderProgram(char *vertexShaderName, char *fragmentShaderName);

/** Builds an orthographic projection with the origin at the top left and y
 *  increasing downward, matching SDL's convention. */
void VSE_CalculateProjectionMatrix(float projection[16], int windowWidth, int windowHeight);

/** Creates the framebuffer a window renders into before post-processing. */
void VSE_CreateWindowFBO(VSE_Engine *engine, VSE_Window *window);
