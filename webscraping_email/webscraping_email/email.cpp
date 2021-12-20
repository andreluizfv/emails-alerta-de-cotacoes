#include <string>
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
#include "nlohmann/json.hpp"






sender_config::sender_config(nlohmann::json config) {
    this->sender_mail = config["sender_mail"];
    this->port = config["port"];
    this->server_smtp = config["server_smtp"];
    this->password = config["password"];
    //std::cout << this->password << " " << this->server_smtp << " " << this->port<<" " << this->sender_mail << std::endl;
}


void enviar(SharedPtr<MailMessage>& message, std::vector<std::string>& emails, struct sender_config config_rem) {
    message->set_From(String(config_rem.sender_mail));
    for (std::string& email : emails)
        message->get_To()->Add(String(email));
    SharedPtr<SmtpClient> client = MakeObject<SmtpClient>();
    client->set_Host(String(config_rem.server_smtp));
    client->set_Username(String(config_rem.sender_mail));
    client->set_Password(String(config_rem.password));
    client->set_Port(config_rem.port);
    client->set_SecurityOptions(Aspose::Email::Clients::SecurityOptions::SSLExplicit);
    try
    {
        client->Send(message);
        std::cout << "Sent." << std::endl;
    }
    catch (System::Exception& ex)
    {
        std::cout << "There was a problem on sending." << std::endl;
        std::cout << (System::ObjectExt::ToString(ex))<<"\n";
        
    }
}

void send_alert(std::string stock, double curr_val, double lim, std::vector<std::string>& emails, struct sender_config config_send) {
    std::stringstream conversor;
    conversor << std::fixed << std::setprecision(2) << curr_val;
    std::string str_curr_v = conversor.str();
    conversor.str(std::string());
    conversor << std::fixed << std::setprecision(2) << lim;
    std::string lim_string = conversor.str();
    SharedPtr<MailMessage> message = System::MakeObject<MailMessage>();
    message->set_IsBodyHtml(false);
    if (curr_val < lim) {
        std::string subject = ("Alerta de mercado: hora de comprar " + stock);
        message->set_Subject(String(subject));
        message->set_Body(String("A " + stock + " atingiu o valor de " + str_curr_v +
            "$, menor que o limite informado de " + lim_string + "$, sendo recomendada sua compra."));
    }
    else {
        std::string subject = "Alerta de mercado: hora de vender " + stock;
        message->set_Subject(String(subject));
        message->set_Body(String("A " + stock + " atingiu o valor de " + str_curr_v +
            "$, maior que o limite informado de " + lim_string + "$, sendo recomendada sua venda."));
    }
    enviar(message, emails, config_send);
}


void inform_default_price(std::string stock, double curr_val, enum decision dec, std::vector<std::string>& emails, struct sender_config config_rem) {
    std::string str_curr_v;
    std::string lim_string;
    SharedPtr<MailMessage> message = System::MakeObject<MailMessage>();
    message->set_IsBodyHtml(false);
    if (dec == BUY) {
        std::string subject = "Alerta expirado: passou da hora de comprar " + stock;
        message->set_Subject(String(subject));
        message->set_Body(String("A " + stock + " subiu para o valor de " + std::to_string(curr_val) +
            "$, nao sendo mais recomendada sua compra."));
    }
    else {
        std::string subject = "Alerta expirado: passou da hora de vender " + stock;
        message->set_Subject(String(subject));
        message->set_Body(String("A " + stock + " caiu para o valor de " + std::to_string(curr_val) +
            "$, nao sendo mais recomendada sua venda."));
    }
    enviar(message, emails, config_rem);
}