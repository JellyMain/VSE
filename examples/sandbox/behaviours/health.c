#include "VSE/components.h"

typedef struct HealthData
{
    int maxHealth;
} HealthData;

void Start(void *data)
{
    HealthData *healthData = data;
    healthData->maxHealth = 5000;
}

void Update(void *data)
{
    HealthData *healthData = data;

    healthData->maxHealth--;

    // printf("Max health %d \n", healthData->maxHealth);
}

void Cleanup(void *data) {}

static VSE_BehaviourDesc descriptor = {.dataSize = sizeof(HealthData),
                                 .name = "health",
                                 .Start = Start,
                                 .Update = Update,
                                 .Cleanup = Cleanup};

VSE_BehaviourDesc *VSE_GetDescriptor()
{
    return &descriptor;
}
