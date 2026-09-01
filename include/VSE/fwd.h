#pragma once
/** Forward declarations, so public headers can refer to each other's types by
 *  pointer without pulling in the whole API. C11 permits a typedef to be
 *  repeated identically, so the defining headers keep their full definitions. */

typedef struct VSE_Vector2Int VSE_Vector2Int;
typedef struct VSE_Vector2Float VSE_Vector2Float;
typedef struct VSE_Texture VSE_Texture;
typedef struct VSE_TextAtlas VSE_TextAtlas;
typedef struct VSE_Material VSE_Material;
typedef struct VSE_UniformTypeValuePair VSE_UniformTypeValuePair;
typedef struct VSE_Renderer VSE_Renderer;
typedef struct VSE_Window VSE_Window;
typedef struct VSE_Updatable VSE_Updatable;
typedef struct VSE_UpdateSystem VSE_UpdateSystem;
typedef struct VSE_Entity VSE_Entity;
typedef struct VSE_Component VSE_Component;
typedef struct VSE_Transform VSE_Transform;
typedef struct VSE_GizmoEntity VSE_GizmoEntity;
typedef struct VSE_Tween VSE_Tween;
typedef struct VSE_TweenSequence VSE_TweenSequence;
typedef struct VSE_List VSE_List;
typedef struct VSE_Dictionary VSE_Dictionary;
typedef struct VSE_KeyValuePair VSE_KeyValuePair;
typedef struct VSE_DebugData VSE_DebugData;
typedef struct VSE_Config VSE_Config;
typedef struct VSE_Engine VSE_Engine;
