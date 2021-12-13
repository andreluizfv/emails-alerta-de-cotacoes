#include "mercado.h"
#include<string>
#include <cpr/cpr.h>
#include "nlohmann/json.hpp"

std::string extract_html_page(std::string acao)
{
    cpr::Url url = cpr::Url{ "https://brapi.ga/api/quote/" + acao };
    cpr::Response response = cpr::Get(url);
    return response.text;
}
/*
static std::string cleantext(GumboNode* node) {
    if (node->type == GUMBO_NODE_TEXT) {
        return std::string(node->v.text.text);
    }
    else if (node->type == GUMBO_NODE_ELEMENT &&
        node->v.element.tag != GUMBO_TAG_SCRIPT &&
        node->v.element.tag != GUMBO_TAG_STYLE) {
        std::string contents = "";
        GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            const std::string text = cleantext((GumboNode*)children->data[i]);
            if (i != 0 && !text.empty()) {
                contents.append(" ");
            }
            contents.append(text);
        }
        return contents;
    }
    else {
        return "";
    }
}*/
/*
double make_double(std::string s) {
    for (char& c : s) if (c == ',') c = '.';
    return stod(s);
}
*/
double get_cotacao(std::string acao) {
    int numero_caracteres_no_double=0;
    std::string string_html = extract_html_page(acao);
    auto objjsn = nlohmann::json::parse(string_html);
    std::string strresults = objjsn["results"].dump();
    strresults = strresults.substr(1, strresults.size() - 2);
    auto results = nlohmann::json::parse(strresults);
    double val = results["regularMarketPrice"];
    return val;
}
/*
std::string extract_html_page_website(std::string acao)
{
    cpr::Url url = cpr::Url{ "https://statusinvest.com.br/acoes/" + acao };
    cpr::Response response = cpr::Get(url);
    return response.text;
}
*/
/*
double get_cotacao_website(std::string acao) {
    std::string string_html = extract_html_page_website(acao);
    GumboOutput* html_parseado = gumbo_parse(string_html.c_str());
    std::string buscando = "Valor atual R$ ";

    std::string texto_puro = cleantext(html_parseado->root);
    size_t local_encontrado = texto_puro.find(buscando, 90) + buscando.size();
    std::string val_str = texto_puro.substr(local_encontrado, 5);

    gumbo_destroy_output(&kGumboDefaultOptions, html_parseado);
    return make_double(val_str);
}*/