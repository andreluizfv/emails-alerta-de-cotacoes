#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include "mercado.h"
#include "email.h"

using namespace std;

int main(int argc, char* argv[])
{
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