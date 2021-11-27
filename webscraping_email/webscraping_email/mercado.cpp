#include "mercado.h"
#include<string>
#include <cpr/cpr.h>

std::string extract_html_page(std::string acao)
{
    cpr::Url url = cpr::Url{ "https://brapi.ga/api/quote/" + acao };
    cpr::Response response = cpr::Get(url);
    return response.text;
}

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
}

double make_double(std::string s) {
    for (char& c : s) if (c == ',') c = '.';
    return stod(s);
}

double get_cotacao(std::string acao) {
    int numero_caracteres_no_double=0;
    std::string string_html = extract_html_page(acao);
    std::string buscando = "\"regularMarketPrice\":";
    size_t local_encontrado = string_html.find(buscando) + buscando.size();
    
    for (int i = 0; string_html[i] != ','; i++) numero_caracteres_no_double++;

    std::string val_str = string_html.substr(local_encontrado, numero_caracteres_no_double);

    return make_double(val_str);
}