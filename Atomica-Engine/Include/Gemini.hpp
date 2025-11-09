#pragma once

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

using json = nlohmann::json;

namespace Atomica
{
// Only loosers keep their API keys secret
inline std::string apiKey = "AIzaSyCsZx6Xtvc0tRtiaw4d_CzLo_7PgrgJk_c";

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Gemini API request
std::string askGemini(const std::string& apiKey, const std::string& atomDesc, const std::string& question) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    std::string modelName = "gemini-2.5-flash";

    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/"
                    + modelName
                    + ":generateContent?key=" + apiKey;
    
    json requestBody = {
        {"contents", {
            {
                {"parts", {
                    {{"text", "Atom info: " + atomDesc + "\nUser question: " + question}}
                }}
            }
        }}
    };
    
    std::string jsonData = requestBody.dump();
    
    curl = curl_easy_init();
    if (!curl) return "Failed to initialize CURL";
    
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        std::string err = "CURL error: " + std::string(curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return err;
    }
    
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    
    try {
        json response = json::parse(readBuffer);
        if (response.contains("candidates") &&
            response["candidates"].size() > 0 &&
            response["candidates"][0].contains("content") &&
            response["candidates"][0]["content"].contains("parts")) {
            return response["candidates"][0]["content"]["parts"][0]["text"].get<std::string>();
        }
        return "Invalid response: " + readBuffer;
    } catch (std::exception& e) {
        return std::string("Parse error: ") + e.what() + "\nRaw: " + readBuffer;
    }
}


}
