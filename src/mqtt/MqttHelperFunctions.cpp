#include "MqttHelperFunctions.h"

bool sendData(AsyncMqttClient mqttClient, String suffix, String data)
{
    String s(mqttClient.getClientId());
    if (!mqttClient.connected())
        return false;
    std::array<String, 2> a{s, suffix};
    mqttClient.publish(MqttPath<std::array<String, 2>>(a).c_str(), QOS, RETAINED_MESSAGE, data.c_str());
    return true;
}

bool sendMAX30102Data(AsyncMqttClient mqttClient, int32_t heartRate, int32_t SpO2)
{
    return sendData(mqttClient, "hr", String(heartRate)) && sendData(mqttClient, "o2", String(SpO2));
}