#include<string>
#include <system/shared_ptr.h>
#include <system/object.h>
#include <MailMessage.h>
#include <MailAddressCollection.h>
#include <cstdint>
#include <system/diagnostics/trace.h>
#include <Clients/Smtp/SmtpClient/SmtpClient.h>
#include <Clients/SecurityOptions.h>
#include <vector>
#include <iomanip>
#include "email.h"
using namespace Aspose::Email::Clients::Smtp;
using namespace Aspose::Email::Clients;
using namespace System;
using namespace Aspose::Email;




void enviar(SharedPtr<MailMessage>& message, std::vector<std::string>& emails, struct configuracoes_remetente config_rem) {
    message->set_From(String(config_rem.email_remetente));
    for (std::string& email : emails)
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
        std::cout << "enviei" << std::endl;
    }
    catch (System::Exception& ex)
    {
        std::cout << "nao consegui enviar" << std::endl;
        std::cout << (System::ObjectExt::ToString(ex));
    }
}

void informar_alerta(std::string acao, double val_atual, double lim, std::vector<std::string>& emails, struct configuracoes_remetente config_rem) {
    std::stringstream conversor;
    conversor << std::fixed << std::setprecision(2) << val_atual;
    std::string val_atual_string = conversor.str();
    conversor.str(std::string());
    conversor << std::fixed << std::setprecision(2) << lim;
    std::string lim_string = conversor.str();
    SharedPtr<MailMessage> message = System::MakeObject<MailMessage>();
    message->set_IsBodyHtml(false);
    if (val_atual < lim) {
        std::string assunto = ("Alerta de mercado: hora de comprar " + acao);
        message->set_Subject(String(assunto));
        message->set_Body(String("A " + acao + " atingiu o valor de " + val_atual_string +
            "$, menor que o limite informado de " + lim_string + "$, sendo recomendada sua venda."));
    }
    else {
        std::string assunto = "Alerta de mercado: hora de vender " + acao;
        message->set_Subject(String(assunto));
        message->set_Body(String("A " + acao + " atingiu o valor de " + val_atual_string +
            "$, maior que o limite informado de " + lim_string + "$, sendo recomendada sua venda."));
    }
    enviar(message, emails, config_rem);
}


void informar_normalidade(std::string acao, double val_atual, enum decisao dec, std::vector<std::string>& emails, struct configuracoes_remetente config_rem) {
    std::string val_atual_string;
    std::string lim_string;
    SharedPtr<MailMessage> message = System::MakeObject<MailMessage>();
    message->set_IsBodyHtml(false);
    if (dec == COMPRA) {
        std::string assunto = "Alerta expirado: passou da hora de comprar " + acao;
        message->set_Subject(String(assunto));
        message->set_Body(String("A " + acao + " subiu para o valor de " + std::to_string(val_atual) +
            "$, nao sendo mais recomendada sua compra."));
    }
    else {
        std::string assunto = "Alerta expirado: passou da hora de vender " + acao;
        message->set_Subject(String(assunto));
        message->set_Body(String("A " + acao + " caiu para o valor de " + std::to_string(val_atual) +
            "$, nao sendo mais recomendada sua venda."));
    }
    enviar(message, emails, config_rem);
}