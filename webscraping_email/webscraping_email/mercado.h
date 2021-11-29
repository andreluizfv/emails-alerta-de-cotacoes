#pragma once
#ifndef MERCADO_H
#define MERCADO_H
#include <string>
#include <gumbo.h>

std::string extract_html_page(std::string acao);

static std::string cleantext(GumboNode* node);

double make_double(std::string s);

double get_cotacao(std::string acao);

#endif
