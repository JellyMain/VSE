#include "VSE/component.h"
#include "VSE/dictionary.h"
#include "VSE/engine.h"
#include "VSE/math.h"
#include "VSE/render.h"
#include "VSE/texture.h"
#include "VSE/ui.h"
#include "VSE/update.h"
#include "glad/glad.h"
#include <SDL_image.h>
#include <stdio.h>

void VSE_SetShaderUniform(GLuint shaderProgram, char *propertyName,
                          VSE_UniformType uniformType, void *value)
{
    glUseProgram(shaderProgram);
    GLint uniformLocation = glGetUniformLocation(shaderProgram, propertyName);

    switch (uniformType)
    {
    case VSE_UNIFORM_FLOAT:
        glUniform1f(uniformLocation, *(float *)value);
        break;

    case VSE_UNIFORM_MAT4F:
        glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, (float *)value);
        break;
    }
}

void VSE_AddPostProcessingEffect(VSE_Engine *engine, char *effectName,
                                 VSE_Material *material)
{
    if (material == NULL)
    {
        SDL_Log("VSE_AddPostProcessingEffect: material is NULL for effect '%s'",
                effectName);
        return;
    }

    VSE_DictionaryAdd(engine->renderer->postProcessingEffects, effectName,
                      material);
}

void VSE_AddGameEntityToDrawList(VSE_Window *window, VSE_Entity *entity)
{
    VSE_ListAdd(window->gameEntitiesDrawList, entity);
}

void VSE_AddUIEntityToDrawList(VSE_Window *window, VSE_Entity *entity)
{
    VSE_ListAdd(window->uiEntitiesDrawList, entity);
}

void VSE_AddGameEntityToAllDrawLists(VSE_Engine *engine, VSE_Entity *entity)
{
    for (int i = 0; i < engine->allWindows->size; i++)
    {
        VSE_Window *window = engine->allWindows->elements[i];
        VSE_AddGameEntityToDrawList(window, entity);
    }
}

void VSE_RemoveGameEntityFromAllDrawLists(VSE_Engine *engine,
                                          VSE_Entity *entity)
{
    for (int i = 0; i < engine->allWindows->size; i++)
    {
        VSE_Window *window = engine->allWindows->elements[i];
        VSE_ListRemove(window->gameEntitiesDrawList, entity);
    }
}

void VSE_AddUIEntityToAllDrawLists(VSE_Engine *engine, VSE_Entity *entity)
{
    for (int i = 0; i < engine->allWindows->size; i++)
    {
        VSE_Window *window = engine->allWindows->elements[i];
        VSE_AddUIEntityToDrawList(window, entity);
    }
}

static void RenderTexture(VSE_Renderer *renderer, VSE_Texture *texture, float x,
                          float y, float width, float height)
{
    float verticesData[] = {
        x,         y,          0.0f, 0.0f, x,         y + height, 0.0f, 1.0f,
        x + width, y + height, 1.0f, 1.0f, x + width, y,          1.0f, 0.0f};

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->textureId);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verticesData), verticesData);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

static void RenderSprite(VSE_Engine *engine, VSE_Vector2Float screenPosition,
                         VSE_Entity *entity, VSE_Component *spriteRenderer,
                         float projectionMatrix[16])
{
    VSE_SpriteRendererComponentData *spriteRendererData = spriteRenderer->data;


    spriteRendererData->size.x = spriteRendererData->texture->width *
                                 engine->pixelsPerUnit *
                                 entity->transform.scale.x;
    spriteRendererData->size.y = spriteRendererData->texture->height *
                                 engine->pixelsPerUnit *
                                 entity->transform.scale.y;

    float x = screenPosition.x - spriteRendererData->size.x / 2.0f;
    float y = screenPosition.y - spriteRendererData->size.y / 2.0f;

    glUseProgram(spriteRendererData->material->shaderProgram);
    VSE_UniformTypeValuePair *typeValuePair = VSE_DictionaryGet(
        spriteRendererData->material->materialUniforms, "projection");
    typeValuePair->uniformValue = projectionMatrix;
    VSE_UpdateMaterialUniforms(spriteRendererData->material);

    RenderTexture(engine->renderer, spriteRendererData->texture, x, y,
                  spriteRendererData->size.x, spriteRendererData->size.y);
}

static void RenderUIImage(VSE_Engine *engine, VSE_Entity *entity,
                          VSE_Component *imageRendererComponent,
                          float projectionMatrix[16])
{
    // image->size.x = image->originalSize.x * engine->pixelsPerUnit
    //                 * entity->transform.scale.x * rect->parentScale.x;
    // image->size.y = image->originalSize.y * engine->pixelsPerUnit
    //                 * entity->transform.scale.y * rect->parentScale.y;

    // float x = entity->transform.position.x - image->size.x / 2.0f;
    // float y = entity->transform.position.y - image->size.y / 2.0f;

    // glUseProgram(image->material->shaderProgram);
    // VSE_UniformTypeValuePair *typeValuePair =
    // VSE_DictionaryGet(image->material->materialUniforms,
    //                                                             "projection");
    // typeValuePair->uniformValue = projectionMatrix;
    // VSE_UpdateMaterialUniforms(image->material);
    // RenderTexture(engine->renderer, image->texture, x, y, image->size.x,
    // image->size.y);
}

void VSE_DrawGizmoRect(VSE_Renderer *renderer, float x, float y, float width,
                       float height)
{
    float verticesData[] = {
        x, y, x, y + height, x + width, y + height, x + width, y,
    };

    glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verticesData), verticesData);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void VSE_RenderDynamicText(
    VSE_TextAtlas *textAtlas, char *text, VSE_Vector2Float position,
    VSE_Vector2Float scale,
    VSE_DynamicTextHorizontalAlignment horizontalAlignment,
    VSE_Renderer *renderer, float projectionMatrix[16])
{
    float totalWidth = 0;
    float maxHeight = 0;

    for (int i = 0; text[i] != '\0'; i++)
    {
        unsigned char character = (unsigned char)text[i];
        if (textAtlas->characterRects[character].w > 0 &&
            textAtlas->characterRects[character].h > 0)
        {
            SDL_Rect charRect = textAtlas->characterRects[character];
            totalWidth += charRect.w * scale.x;

            float charHeight = charRect.h * scale.y;
            if (charHeight > maxHeight)
            {
                maxHeight = charHeight;
            }
        }
        else
        {
            totalWidth += 1;
        }
    }

    float startX;

    if (horizontalAlignment == VSE_HORIZONTAL_NOT_CENTERED)
    {
        startX = roundf(position.x);
    }
    else if (horizontalAlignment == VSE_HORIZONTAL_CENTERED)
    {
        startX = roundf(position.x - totalWidth / 2.0f);
    }

    float startY = roundf(position.y - maxHeight / 2.0f);


    float currentX = 0;

    glUseProgram(renderer->defaultMaterial->shaderProgram);
    VSE_UniformTypeValuePair *typeValuePair = VSE_DictionaryGet(
        renderer->defaultMaterial->materialUniforms, "projection");
    typeValuePair->uniformValue = projectionMatrix;
    VSE_UpdateMaterialUniforms(renderer->defaultMaterial);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textAtlas->atlasTexture->textureId);

    glBindVertexArray(renderer->entitiesVAO);

    for (int i = 0; text[i] != '\0'; i++)
    {
        unsigned char character = (unsigned char)text[i];

        if (textAtlas->characterRects[character].w > 0 &&
            textAtlas->characterRects[character].h > 0)
        {
            SDL_Rect charRect = textAtlas->characterRects[character];

            float dstW = charRect.w * scale.x;
            float dstH = charRect.h * scale.y;
            float dstY = startY;
            float dstX = startX + currentX;

            float atlasWidth = textAtlas->atlasTexture->width;
            float atlasHeight = textAtlas->atlasTexture->height;

            float texLeft = charRect.x / atlasWidth;
            float texRight = (charRect.x + charRect.w) / atlasWidth;
            float texTop = charRect.y / atlasHeight;
            float texBottom = (charRect.y + charRect.h) / atlasHeight;

            float verticesData[] = {
                dstX,        dstY,        texLeft,  texTop,
                dstX,        dstY + dstH, texLeft,  texBottom,
                dstX + dstW, dstY + dstH, texRight, texBottom,
                dstX + dstW, dstY,        texRight, texTop};

            glBindBuffer(GL_ARRAY_BUFFER, renderer->VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verticesData),
                            verticesData);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            currentX += charRect.w * scale.x;
        }
        else
        {
            currentX += 1;
        }
    }
}

void VSE_RenderHollowGizmoRect(VSE_Engine *engine, VSE_Vector2Float position,
                               VSE_Vector2Float size, float thickness)
{
    VSE_DrawGizmoRect(engine->renderer, position.x - size.x / 2,
                      position.y - size.y / 2, size.x, thickness);

    VSE_DrawGizmoRect(engine->renderer, position.x - size.x / 2,
                      position.y + size.y / 2, size.x, thickness);

    VSE_DrawGizmoRect(engine->renderer, position.x - size.x / 2,
                      position.y - size.y / 2, thickness, size.y);

    VSE_DrawGizmoRect(engine->renderer, position.x + size.x / 2 - thickness,
                      position.y - size.y / 2 + thickness, thickness, size.y);
}

static void RenderDebugInfo(VSE_Engine *engine, float projectionMatrix[16])
{
    char fpsText[30];
    snprintf(fpsText, sizeof(fpsText), "FPS:%.1f", engine->debugData.fps);

    VSE_RenderDynamicText(engine->textAtlas, fpsText,
                          (VSE_Vector2Float){10, 10}, (VSE_Vector2Float){1, 1},
                          VSE_HORIZONTAL_NOT_CENTERED, engine->renderer,
                          projectionMatrix);
}

static void RenderGizmo(VSE_Engine *engine, VSE_GizmoEntity *gizmoEntity,
                        float projectionMatrix[16])
{
    // glUseProgram(engine->renderer->defaultGizmoMaterial->shaderProgram);
    // VSE_UniformTypeValuePair *typeValuePair = VSE_DictionaryGet(
    // 	engine->renderer->defaultGizmoMaterial->materialUniforms,
    // 	"projection");
    // typeValuePair->uniformValue = projectionMatrix;
    // VSE_UpdateMaterialUniforms(engine->renderer->defaultGizmoMaterial);
    // GLint colorLocation =
    // glGetUniformLocation(engine->renderer->defaultGizmoMaterial->shaderProgram,
    //                                            "color");
    // glUniform4f(colorLocation,
    //             gizmoEntity->color.r / 255.0f,
    //             gizmoEntity->color.g / 255.0f,
    //             gizmoEntity->color.b / 255.0f,
    //             gizmoEntity->color.a / 255.0f);


    // VSE_Entity *connected = gizmoEntity->connectedEntity;
    // VSE_SpriteRenderer *image = VSE_GetComponent(connected,
    // VSE_COMPONENT_SPRITE_RENDERER);

    // if (image == NULL)
    // {
    // 	return;
    // }

    // VSE_Vector2Float position = connected->transform.position;

    // VSE_RenderHollowGizmoRect(engine, position, image->size,
    // gizmoEntity->thickness);

    // char entityInfo[150];
    // snprintf(entityInfo, sizeof(entityInfo), "x:%.1f,y:%.1f w:%.1f,h:%.1f",
    //          position.x, position.y, image->size.x, image->size.y);

    // VSE_RenderDynamicText(engine->textAtlas, entityInfo,
    //                       (VSE_Vector2Float){
    // 	                      position.x - image->size.x / 2,
    // 	                      position.y - image->size.y / 2 - 11
    //                       }, (VSE_Vector2Float){1, 1},
    //                       VSE_HORIZONTAL_NOT_CENTERED, engine->renderer,
    //                       projectionMatrix);

    // glBindVertexArray(engine->renderer->gizmosVAO);
}

static void PrepareScene(VSE_Window *window, VSE_Engine *engine)
{
    SDL_GL_MakeCurrent(window->sdlWindow, engine->glContext);
    glBindFramebuffer(GL_FRAMEBUFFER, window->FBO);
    glViewport(0, 0, window->size.x, window->size.y);
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

VSE_Updatable *VSE_CreateRenderUpdatable()
{
    VSE_Updatable *updatable = VSE_CreateUpdatable(NULL, VSE_UpdateRenderer);
    return updatable;
}

static void ApplyPostProcessing(VSE_Engine *engine, VSE_Window *window)
{
    if (engine->renderer->postProcessingEffects->totalEntries == 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(
            engine->renderer->defaultPostProcessingMaterial->shaderProgram);

        glBindVertexArray(engine->renderer->postProcessingVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, window->FBOTexture);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    else
    {
        for (int i = 0;
             i < engine->renderer->postProcessingEffects->allPairs->size; i++)
        {
            if (i ==
                engine->renderer->postProcessingEffects->allPairs->size - 1)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
            }
            else
            {
                glBindFramebuffer(GL_FRAMEBUFFER, window->FBO);
            }


            VSE_KeyValuePair *pair = VSE_DictionaryGetPair(
                engine->renderer->postProcessingEffects, i);
            VSE_Material *material = pair->value;

            VSE_UpdateMaterialUniforms(material);

            glUseProgram(material->shaderProgram);

            glBindVertexArray(engine->renderer->postProcessingVAO);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, window->FBOTexture);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }
}

void VSE_UpdateMaterialUniforms(VSE_Material *material)
{
    for (int i = 0; i < material->materialUniforms->allPairs->size; i++)
    {
        VSE_KeyValuePair *pair =
            VSE_DictionaryGetPair(material->materialUniforms, i);
        char *uniformName = pair->key;
        VSE_UniformTypeValuePair *typeValuePair = pair->value;

        VSE_SetShaderUniform(material->shaderProgram, uniformName,
                             typeValuePair->uniformType,
                             typeValuePair->uniformValue);
    }
}

void VSE_UpdateRenderer(void *data, VSE_Engine *engine, float deltaTime)
{

    for (int i = 0; i < engine->allWindows->size; i++)
    {

        VSE_Window *window = VSE_ListGet(engine->allWindows, i);

        float projectionMatrix[16];

        VSE_CalculateProjectionMatrix(projectionMatrix, window->size.x,
                                      window->size.y);

        PrepareScene(window, engine);

        glBindVertexArray(engine->renderer->entitiesVAO);

        for (int j = 0; j < window->gameEntitiesDrawList->size; j++)
        {

            VSE_Entity *entity = VSE_ListGet(window->gameEntitiesDrawList, j);

            if (!entity->active)
            {
                continue;
            }

            VSE_Component *spriteRenderer =
                VSE_GetComponent(entity, SPRITE_RENDERER_COMPONENT);

            if (spriteRenderer == NULL)
            {
                continue;
            }


            VSE_Vector2Float worldPosition = entity->transform.position;
            VSE_Vector2Float screenPos;

            switch (window->renderType)
            {
            case VSE_WINDOW_SCREEN_SPACE:
                screenPos = worldPosition;
                break;

            case VSE_WINDOW_WORLD_SPACE:
                screenPos =
                    VSE_WorldToScreen(worldPosition, window->viewportOffset);
                break;

            default:
                screenPos = worldPosition;
                printf("Unknown window type");
                break;
            }

            RenderSprite(engine, screenPos, entity, spriteRenderer,
                         projectionMatrix);
        }

        for (int j = 0; j < window->uiEntitiesDrawList->size; j++)
        {
            VSE_Entity *entity = VSE_ListGet(window->uiEntitiesDrawList, j);

            if (!entity->active)
            {
                continue;
            }

            // VSE_RectTransform *rect = VSE_GetComponent(entity,
            // VSE_COMPONENT_RECT_TRANSFORM);

            // if (rect == NULL)
            // {
            // 	continue;
            // }

            // VSE_DynamicText *dynamicText = VSE_GetComponent(entity,
            // VSE_COMPONENT_DYNAMIC_TEXT);

            // if (dynamicText != NULL)
            // {
            // 	VSE_Vector2Float renderScale = {
            // 		engine->pixelsPerUnit * entity->transform.scale.x *
            // rect->parentScale.x, 		engine->pixelsPerUnit *
            // entity->transform.scale.y * rect->parentScale.y
            // 	};

            // 	VSE_RenderDynamicText(engine->textAtlas, dynamicText->text,
            // 	                      entity->transform.position, renderScale,
            // 	                      VSE_HORIZONTAL_CENTERED, engine->renderer,
            // projectionMatrix); 	continue;
            // }

            // VSE_SpriteRenderer *image = VSE_GetComponent(entity,
            // VSE_COMPONENT_SPRITE_RENDERER);

            // if (image != NULL)
            // {
            // 	RenderUIImage(engine, entity, image, rect, projectionMatrix);
            // }
        }


        if (engine->debugMode)
        {
            glBindVertexArray(engine->renderer->gizmosVAO);

            for (int k = 0; k < window->gizmosEntitiesDrawList->size; k++)
            {
                VSE_GizmoEntity *gizmoEntity =
                    VSE_ListGet(window->gizmosEntitiesDrawList, k);
                RenderGizmo(engine, gizmoEntity, projectionMatrix);
            }

            RenderDebugInfo(engine, projectionMatrix);
        }

        ApplyPostProcessing(engine, window);

        SDL_GL_SwapWindow(window->sdlWindow);
    }
}
