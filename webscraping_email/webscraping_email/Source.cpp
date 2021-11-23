#include<iostream>
#include<vector>
#include<string>
#include<thread>
#include<chrono>
using namespace std;

//------------------------------------------------------web-scraping-----------------------//
#include <cpr/cpr.h>
#include <gumbo.h>
#include <sstream>

string extract_html_page(string acao)
{
    cpr::Url url = cpr::Url{ "https://statusinvest.com.br/acoes/" + acao };
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

double make_double(string s) {
    for (char& c : s) if (c == ',') c = '.';
    return stod(s);
}

double get_cotacao(string acao) {
    string string_html = extract_html_page(acao);
    GumboOutput* html_parseado = gumbo_parse(string_html.c_str());
    string buscando = "Valor atual R$ ";
    
    string texto_puro = cleantext(html_parseado->root);
    size_t local_encontrado = texto_puro.find(buscando, 90) + buscando.size();
    //cout << texto_puro.substr(local_encontrado, 5) << endl;
    string val_str = texto_puro.substr(local_encontrado, 5);
    //cout << make_double(val_str) << endl;

    gumbo_destroy_output(&kGumboDefaultOptions, html_parseado);
    return make_double(val_str);
}
//-----------------------------------------------fim webscraping, comeco da parte de email---------------------
#include <system/string.h>
#include <system/shared_ptr.h>
#include <system/object.h>
#include "system/console.h"
#include <AttachmentCollection.h>
#include <Attachment.h>
#include <SaveOptions.h>
#include <MsgSaveOptions.h>
#include <MailMessage.h>
#include <MailAddressCollection.h>
#include <Licensing/License.h>
#include <cstdint>
#include <system/exceptions.h>
#include <system/diagnostics/trace.h>
#include <Clients/Smtp/SmtpClient/SmtpClient.h>
#include <Clients/SecurityOptions.h>
using namespace System;
using namespace Aspose::Email::Clients::Smtp;
using namespace Aspose::Email::Clients;
using namespace Aspose::Email;

using namespace Aspose::Email;

void send_email() {
    SharedPtr<MailMessage> message = System::MakeObject<MailMessage>();
    message->set_Subject(u"New message created by Aspose.Email for .NET");
    message->set_IsBodyHtml(false);
    message->set_Body(String("Hello"));
    message->set_From(u"andreluiz.lf11@gmail.com");
    message->get_To()->Add(u"andreluiz.lf11@hotmail.com");
    SharedPtr<SmtpClient> client = MakeObject<SmtpClient>();
    client->set_Host(u"smtp.gmail.com");
    client->set_Username(u"andreluiz.lf11@gmail.com");
    client->set_Password(u"***");
    client->set_Port(587);
    client->set_SecurityOptions(Aspose::Email::Clients::SecurityOptions::SSLExplicit);
    try
    {
        // Client.Send will send this message
        client->Send(message);
        cout << "enviei" << endl;
    }
    catch (System::Exception& ex)
    {
        cout << "nao consegui enviar" << endl;
        cout<<(System::ObjectExt::ToString(ex));
    }
}

//-------------------------------------------------------------------------------


int main(int argc, char* argv[])
{
    send_email();
    vector<string> acoes;
    vector<double> minimos, maximos, atuais;
    int intervalo_medidas = 5;
    if (argc < 4 or argc % 3 !=1 ) {
        printf("Numero de argumentos invalido");
    }
    else{
        for (int counter = 1; counter < argc; counter++) {
            if (counter % 3 == 1)acoes.push_back(argv[counter]); 
            else if (counter % 3 == 2) minimos.push_back(stod(string(argv[counter])));
            else maximos.push_back(stod(string(argv[counter])));
        }
    }
    cout << endl;
    for (int i = 0; i < acoes.size(); i++) {
        cout << acoes[i] << " " << minimos[i] << " " << maximos[i] << endl;
    }

    atuais.resize(acoes.size());
    
    while (true) {    
        cout << "Monitorando os precos de:" << endl;
        for (int i = 0; i < acoes.size(); i++) {
            atuais[i] = get_cotacao(acoes[i]);
            cout << acoes[i] << " : " << atuais[i] << endl;
            if (atuais[i]<minimos[i] or atuais[i]>maximos[i]) {
                cout << "enviando email: hora de " << (atuais[i] < minimos[i] ? "comprar " : " vender ")
                    << acoes[i] << endl;
                //funcao de enviar email
            }
        }

        this_thread::sleep_for(chrono::seconds(intervalo_medidas));
    }
    return 0;
}