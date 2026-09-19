#pragma once
#include "VSE/components.h"


typedef struct VSE_Entity
{
    VSE_Transform transform;
    VSE_List *components; //VSE_List(of VSE_List(of VSE_Component))
    bool active;
} VSE_Entity;


VSE_Entity *VSE_CreateEntity(VSE_Engine *engine, VSE_Window *window,
                             VSE_Vector2Float position, VSE_Vector2Float scale);

void VSE_EntityDestroy(VSE_Engine *engine, VSE_Entity *entity);
