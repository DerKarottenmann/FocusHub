#ifndef API_REQUESTS_H
#define API_REQUESTS_H

#include <tuple>
#include <string>

std::tuple<std::string, std::string, std::string, std::string> getWeather(double lat, double lng, const String& key);
std::pair<double, double> getCoordinates(const String& plz, const String& land, const String& key);

#endif