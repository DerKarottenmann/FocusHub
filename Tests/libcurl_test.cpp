#include <stdio.h>
#include <curl/curl.h>
#include "include/json.hpp"
#include <string>
#include <iostream>

//API-KEY: AIzaSyDg5XWUUOQkl1PvcRtcP5WzICDieYH8QD8
// Location API-Request: https://maps.googleapis.com/maps/api/geocode/json?address=80997,DE&key=AIzaSyDg5XWUUOQkl1PvcRtcP5WzICDieYH8QD8
// Weather API-Request:  https://weather.googleapis.com/v1/currentConditions:lookup?key=AIzaSyDg5XWUUOQkl1PvcRtcP5WzICDieYH8QD8&location.latitude=37.4220&location.longitude=-122.0841

// Callback-Funktion, um die Antwort in einen std::string zu schreiben
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main(void)
{
    CURL *curl;
    CURLcode res;
    std::string response; //Json_Text speicherort

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://maps.googleapis.com/maps/api/geocode/json?address=80997,DE&key=AIzaSyDg5XWUUOQkl1PvcRtcP5WzICDieYH8QD8");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // Debug-Ausgabe aktivieren

        // Schreibe Antwort in response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Anfrage
        res = curl_easy_perform(curl);

        // Failsafe
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        } else {
            printf("Anfrage erfolgreich!\n");
            //JSON-Text als std::string:
            //std::cout << "JSON als String(std:string):\n" << response << std::endl;
        }

        // Aufräumen
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "curl_easy_init() failed!\n");
    }
    nlohmann::json data = nlohmann::json::parse(response);
    double latitude = data["results"][0]["geometry"]["location"]["lat"];
    double longitude = data["results"][0]["geometry"]["location"]["lng"];
    std::cout << "Latitude: " << latitude << ", Longitude: " << longitude << std::endl;

    //size_t pos_lat = response.find("\"location\"");
    return 0;
}
