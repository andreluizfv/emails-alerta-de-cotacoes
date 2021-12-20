#pragma once
#ifndef EMAIL_H
#define EMAIL_H
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
#include "nlohmann/json.hpp"

using namespace Aspose::Email::Clients::Smtp;
using namespace Aspose::Email::Clients;
using namespace System;
using namespace Aspose::Email;

enum decision { SELL, BUY };

struct sender_config { 
    std::string server_smtp = "";
    int32_t port = 0;
    std::string sender_mail = "";
    std::string password = "";
    sender_config(nlohmann::json config);
};

void enviar(SharedPtr<MailMessage>& message, std::vector<std::string>& emails, struct sender_config config_rem);

void send_alert(std::string stock, double curr_val, double lim, std::vector<std::string>& emails, struct sender_config config_send);

void inform_default_price(std::string stock, double curr_val, enum decision dec, std::vector<std::string>& emails, struct sender_config config_rem);

#endif
