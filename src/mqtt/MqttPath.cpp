#include <Arduino.h>

const String separator = "/";
template <typename Iterable>
String MqttPath(Iterable &it)
{
    String result = "";
    size_t s = it.size();
    for (size_t i = 0; i < s; ++i)
        result += it[i] + ((i - s + 1) ? separator : "");
    return result;
}