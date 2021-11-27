#pragma once

#include <system/shared_ptr.h>
#include <system/object.h>
#include <MailMessage.h>
#include <MailAddressCollection.h>
#include <cstdint>
#include <system/diagnostics/trace.h>
#include <Clients/Smtp/SmtpClient/SmtpClient.h>
#include <Clients/SecurityOptions.h>
#include <vector>
#include <string>

using namespace Aspose::Email::Clients::Smtp;
using namespace Aspose::Email::Clients;
using namespace System;
using namespace Aspose::Email;

enum decisao { VENDA, COMPRA };

struct configuracoes_remetente {
    std::string server_smtp = "";
    int32_t porta = 0;
    std::string email_remetente = "";
    std::string senha = "";
};

void enviar(SharedPtr<MailMessage>& message, std::vector<std::string>& emails, struct configuracoes_remetente config_rem);

void informar_alerta(std::string acao, double val_atual, double lim, std::vector<std::string>& emails, struct configuracoes_remetente config_rem);

void informar_normalidade(std::string acao, double val_atual, enum decisao dec, std::vector<std::string>& emails, struct configuracoes_remetente config_rem);


