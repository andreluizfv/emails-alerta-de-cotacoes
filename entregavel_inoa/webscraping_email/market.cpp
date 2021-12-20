#include "market.h"
#include<string>
#include <cpr/cpr.h>
#include "nlohmann/json.hpp"

std::string extract_json_from_api(std::string acao)
{
    cpr::Url url = cpr::Url{ "https://brapi.ga/api/quote/" + acao };
    cpr::Response response = cpr::Get(url);
    return response.text;
}

double get_quotation(std::string acao) {
    int numero_caracteres_no_double=0;
    std::string string_html = extract_json_from_api(acao);
    auto objjsn = nlohmann::json::parse(string_html);
    std::string strresults = objjsn["results"].dump();
    strresults = strresults.substr(1, strresults.size() - 2);
    auto results = nlohmann::json::parse(strresults);
    double val = results["regularMarketPrice"];
    return val;
}
