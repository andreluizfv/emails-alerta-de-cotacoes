#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include "mercado.h"
#include "email.h"

int main(int argc, char* argv[])
{
    std::vector<std::string> emails, acoes;
    std::string novoemail, smtp_server, email_emissor, porta;
    std::vector<double> minimos, maximos, atuais;
    std::map <std::pair<std::string, int>, bool> ja_informado;
    int intervalo_medidas = 5;
    configuracoes_remetente config_rem;
    std::ifstream arq_config = std::ifstream("configuracoes_smtp.txt");
    std::ifstream arq_emails = std::ifstream("emails_destino.txt");

    if (!arq_emails) {
        std::cerr << "O arquivo com os emails destino nao conseguiu ser aberto." << std::endl;
        exit(1);
    }
    if (!arq_config) {
        std::cerr << "O arquivo com as configuracoes do servidor nao conseguiu ser aberto." << std::endl;
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

    if (argc < 4 or argc % 3 != 1) {
        std::cerr << "Numero de argumentos (acoes e intervalos) invalido." << std::endl;
        exit(2);
    }
    else {
        for (int counter = 1; counter < argc; counter++) {
            if (counter % 3 == 1)acoes.push_back(argv[counter]);
            else if (counter % 3 == 2) minimos.push_back(stod(std::string(argv[counter])));
            else maximos.push_back(stod(std::string(argv[counter])));
        }
    }
    std::cout << std::endl;
    for (int i = 0; i < acoes.size(); i++) {
        std::cout << acoes[i] << " " << minimos[i] << " " << maximos[i] << std::endl;
    }
    atuais.resize(acoes.size());

    while (true) {
        std::cout << "Monitorando os valores de:" << std::endl;
        for (int i = 0; i < acoes.size(); i++) {
            atuais[i] = get_cotacao(acoes[i]);
            //atuais[i] = get_cotacao_website(acoes[i]);
            std::cout << acoes[i] << " : " << atuais[i] << std::endl;
            if (atuais[i] < minimos[i] and !ja_informado[{acoes[i], COMPRA}]) {
                std::cout << "enviando email: hora de comprar " << acoes[i] << std::endl;
                informar_alerta(acoes[i], atuais[i], minimos[i], emails, config_rem);
                ja_informado[{acoes[i], COMPRA}] = true;
            }
            else if (atuais[i] > maximos[i] and !ja_informado[{acoes[i], VENDA}]) {
                std::cout << "enviando email: hora de vender " << acoes[i] << std::endl;
                informar_alerta(acoes[i], atuais[i], maximos[i], emails, config_rem);
                ja_informado[{acoes[i], VENDA}] = true;
            }
            else if ((minimos[i] < atuais[i] and atuais[i] < maximos[i])) {
                if (ja_informado[{acoes[i], COMPRA}]) {
                    std::cout << "enviando email: passou a hora de comprar" << acoes[i] << std::endl;
                    informar_normalidade(acoes[i], atuais[i], COMPRA, emails, config_rem);
                    ja_informado[{acoes[i], COMPRA}] = false;
                }
                if (ja_informado[{acoes[i], VENDA}]) {
                    std::cout << "enviando email: passou a hora de vender" << acoes[i] << std::endl;
                    informar_normalidade(acoes[i], atuais[i], VENDA, emails, config_rem);
                    ja_informado[{acoes[i], VENDA}] = false;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(intervalo_medidas));
    }
    arq_config.close();
    arq_emails.close();
    return 0;
}