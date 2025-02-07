// Cliente que enviar um DNS REQUEST para um SERVER pré configurado
// ETAPAS

// socket
// connect (only TCP)
// write / read
// close

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace std;

void formatDNSName(unsigned char *dns, const char *host);

int main(int argc, char *argv[]) {
    if(argc < 2) {
        cout << "Uso: " << argv[0] << " <host>\n";
        return 1;
    }

    unsigned char dnsPacket[512]; // array para armazenar o DNS REQUEST
    const char *hostname = argv[1];

    // Criar um socket UDP
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // AF_INET -> comunicação por IPv4       SOCK_DGRAM -> por datagramas        IPPROTO_UDP -> UDP protocol
    if(sock < 0) {
        cout << "Erro ao criar socket." << endl;
        return 1;
    }

    // Configurar um servidor DNS
    struct sockaddr_in saddr;
    saddr.sin_family    = AF_INET; // comunicação por IPv4
    saddr.sin_port      = htons(53); // 53 -> padrão DNS        htons -> converte para formato padrão de rede 
    saddr.sin_addr.s_addr = inet_addr("8.8.8.8"); // define o IP de destino     inet_addr converte a string para um número de 32bits no formato de rede.

    // Construir o DNS REQUEST no array buffer

    // HEADER
    dnsPacket[0] = 0xAA; dnsPacket[1] = 0xAA; // ID 2 bytes
    dnsPacket[2] = 0x01; dnsPacket[3] = 0x00; // Flags | QR - 0  OpCode - 0000  AA - 0  TC - 0  RD - 1 (0001 0000) | RA - 0  Z - 000  RCODE - 0000 (0000 0000)
    dnsPacket[4] = 0x00; dnsPacket[5] = 0x01; // QDCOUNT
    dnsPacket[6] = 0x00; dnsPacket[7] = 0x00; // ANCOUNT
    dnsPacket[8] = 0x00; dnsPacket[9] = 0x00; // NSCOUNT
    dnsPacket[10] = 0x00; dnsPacket[11] = 0x00; // ARCOUNT

    // QUESTION
    // Domain Name no formato DNS
    // i.e. www.google.com -> 3www6google3com | não precisa ser em hex
    unsigned char *queryName = &dnsPacket[12];
    formatDNSName(queryName, hostname);

    int tamQuery = strlen((char*)queryName); // armazena tamanho do QNAME para continuar a construir o PACKET depois dele

    // QTYPE (A = host address = 0x0001)
    dnsPacket[12 + tamQuery] = 0x00; dnsPacket[13 + tamQuery] = 0x01;

    // QCLASS (IN = internet address = 0x0001)
    dnsPacket[13 + tamQuery] = 0x00; dnsPacket[14 + tamQuery] = 0x01;

    // TAMANHO TOTAL DO PACOTE (QNAME + 16)
    int packetSize = 16 + tamQuery;


    // Enviar a QUERY
    // sendto(sock, buffer, quantidade de bytes do request, flags, * para struct server, tamanho da struct);
    if(sendto(sock, dnsPacket, packetSize, 0, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        cout << "Erro ao enviar pacote DNS." << endl;
        close(sock);
        return 1;
    } else {
        cout << "DNS REQUEST enviado com sucesso!" << endl;
    }

    // Receber o REPLY
    // recvfrom(sock, buffer, tamanho do buffer, flags, * para struct server, * para tamanho da struct);
    socklen_t len = sizeof(saddr);
    if(recvfrom(sock, dnsPacket, sizeof(dnsPacket), 0, (struct sockaddr*)&saddr, &len) < 0) {
        cout << "Erro ao receber resposta." << endl;
        close(sock);
        return 1;
    } else {
        cout << "DNS REPLY recebido com sucesso!" << endl;
    }

    

    // Interpretar o REPLY
    int jumpToResp = 12 + tamQuery + 4;
    int jumpToIp = jumpToResp + 10;

    struct in_addr ip_addr;
    memcpy(&ip_addr, &dnsPacket[jumpToIp], 4);

    cout << "IP de " << hostname << ": " << inet_ntoa(ip_addr) << endl;


    return 0;
}

void formatDNSName(unsigned char *dns, const char *host) {
    int nc = 0;
    for(int i = 0; i <= strlen(host); i++) {
        if(host[i] == '.' || host[i] == '\0') {
            *dns++ = i - nc; // escreve no endereço apontado por dns a quantidade de caracteres que virão e avança para proxima posição
            for(; nc < i; nc++) {
                *dns++ = host[nc]; // escreve os caracteres
            }
            nc++; // avança o nc para pular o ponto
        }
    }
    *dns = '\0'; // finaliza o formato DNS
}