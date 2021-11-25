#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cpr/cpr.h>
#include <gumbo.h>
#include <sstream>
#include <system/shared_ptr.h>
#include <system/object.h>
#include <MailMessage.h>
#include <MailAddressCollection.h>
#include <cstdint>
#include <system/diagnostics/trace.h>
#include <Clients/Smtp/SmtpClient/SmtpClient.h>
#include <Clients/SecurityOptions.h>
using namespace System;
using namespace Aspose::Email::Clients::Smtp;
using namespace Aspose::Email::Clients;
using namespace Aspose::Email;
using namespace std;

//------------------------------------------------------web-scraping-----------------------//

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
    string val_str = texto_puro.substr(local_encontrado, 5);

    gumbo_destroy_output(&kGumboDefaultOptions, html_parseado);
    return make_double(val_str);
}
//-----------------------------------------------fim webscraping, comeco da parte de email---------------------
struct configuracoes_remetente {
    string server_smtp = "";
    int32_t porta = 0;
    string email_remetente = "";
    string senha = "";
};
enum decisao {VENDA, COMPRA};

void enviar(SharedPtr<MailMessage>& message, vector<string>& emails, struct configuracoes_remetente config_rem) {
    message->set_From(String(config_rem.email_remetente));
    for (string& email : emails)
        message->get_To()->Add(String(email));
    SharedPtr<SmtpClient> client = MakeObject<SmtpClient>();
    client->set_Host(String(config_rem.server_smtp));
    client->set_Username(String(config_rem.email_remetente));
    client->set_Password(String(config_rem.senha));
    client->set_Port(config_rem.porta);
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
        cout << (System::ObjectExt::ToString(ex));
    }
}
void informar_alerta(string acao, double val_atual, double lim, vector<string>& emails, struct configuracoes_remetente config_rem) {
    stringstream conversor;
    conversor << fixed << setprecision(2) << val_atual;
    string val_atual_string = conversor.str();
    conversor.str(string());
    conversor << fixed << setprecision(2) << lim;
    string lim_string = conversor.str();
    SharedPtr<MailMessage> message = System::MakeObject<MailMessage>();
    message->set_IsBodyHtml(false);
    if (val_atual < lim) {
        string assunto = ("Alerta de mercado: hora de comprar " + acao);
        message->set_Subject(String(assunto));
        message->set_Body(String("A " + acao + " atingiu o valor de " + val_atual_string +
            "$, menor que o limite informado de " + lim_string + "$, sendo recomendada sua venda."));
    }
    else {
        string assunto = "Alerta de mercado: hora de vender " + acao;
        message->set_Subject(String(assunto));
        message->set_Body(String("A " + acao + " atingiu o valor de " + val_atual_string +
            "$, maior que o limite informado de " + lim_string + "$, sendo recomendada sua venda."));
    }
    enviar(message, emails, config_rem);
}

void informar_normalidade(string acao, double val_atual, enum decisao dec, vector<string>& emails, struct configuracoes_remetente config_rem) {
    string val_atual_string;
    string lim_string;
    SharedPtr<MailMessage> message = System::MakeObject<MailMessage>();
    message->set_IsBodyHtml(false);
    if (dec == COMPRA) {
        string assunto = "Alerta expirado: passou da hora de comprar " + acao;
        message->set_Subject(String(assunto));
        message->set_Body(String("A " + acao + " subiu para o valor de " + to_string(val_atual) +
            "$, nao sendo mais recomendada sua compra."));
    }
    else {
        string assunto = "Alerta expirado: passou da hora de vender " + acao;
        message->set_Subject(String(assunto));
        message->set_Body(String("A " + acao + " caiu para o valor de " + to_string(val_atual) +
            "$, nao sendo mais recomendada sua venda."));
    }
    enviar(message, emails, config_rem);
}

//------------------------------------------fim da parte de email, começo da main----------------





int main(int argc, char* argv[])
{
    
    //send_email();
    vector<string> emails, acoes;
    string novoemail, smtp_server, email_emissor, porta;
    vector<double> minimos, maximos, atuais;
    map <pair<string, int>, bool> ja_informado;
    int intervalo_medidas = 5;
    configuracoes_remetente config_rem;
    ifstream arq_config = ifstream("configuracoes_smtp.txt");
    ifstream arq_emails = ifstream("emails_destino.txt");

    if (!arq_emails) { 
        cerr << "O arquivo com os emails destino nao conseguiu ser aberto." << endl;
        exit(1);
    }
    if (!arq_config) { 
        cerr << "O arquivo com as configuracoes do servidor nao conseguiu ser aberto." << endl;
        exit(2);
    }

    while (!arq_emails.eof()) {
        arq_emails.ignore(50, ':');
        arq_emails >> novoemail;
        emails.push_back(novoemail);
    }

    arq_config.ignore(50, ':');
    arq_config >> config_rem.server_smtp;
    arq_config.ignore(50, ':');
    arq_config >> config_rem.porta;
    arq_config.ignore(50, ':');
    arq_config >> config_rem.email_remetente;
    arq_config.ignore(50, ':');
    arq_config >> config_rem.senha;

    if (argc < 4 or argc % 3 !=1 ) {
        cerr << "Numero de argumentos (acoes e intervalos) invalido." << endl;
        exit(2);
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
        cout << "Monitorando os valores de:" << endl;
        for (int i = 0; i < acoes.size(); i++) {
            atuais[i] = get_cotacao(acoes[i]);
            cout << acoes[i] << " : " << atuais[i] << endl;
            if (atuais[i]<minimos[i] and !ja_informado[{acoes[i], COMPRA}]) {
                cout << "enviando email: hora de comprar " << acoes[i] << endl;
                informar_alerta(acoes[i], atuais[i], minimos[i], emails, config_rem);
                ja_informado[{acoes[i], COMPRA}] = true;
            }
            else if (atuais[i] > maximos[i] and !ja_informado[{acoes[i], VENDA}]) {
                cout << "enviando email: hora de vender " << acoes[i] << endl;
                informar_alerta(acoes[i], atuais[i], maximos[i], emails, config_rem);
                ja_informado[{acoes[i], VENDA}] = true;
            }
            else if ((minimos[i] < atuais[i] and atuais[i] < maximos[i])) {
                if (ja_informado[{acoes[i], COMPRA}]) {
                    informar_normalidade(acoes[i], atuais[i], COMPRA, emails, config_rem);
                    ja_informado[{acoes[i], COMPRA}] = false;
                }
                if (ja_informado[{acoes[i], VENDA}]) {
                    informar_normalidade(acoes[i], atuais[i], VENDA, emails, config_rem);
                    ja_informado[{acoes[i], VENDA}] = false;
                }
            }
        }

        this_thread::sleep_for(chrono::seconds(intervalo_medidas));
    }
    return 0;
}