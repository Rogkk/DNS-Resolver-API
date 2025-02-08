// Cliente para enviar um DNS REQUEST para um SERVER pré configurado
// ETAPAS

// socket
// connect (apenas TCP)
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
int parseDNSReply(unsigned char *reply, int replyLen);

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

    int tamQuery = strlen((char*)queryName) + 1; // armazena tamanho do QNAME para continuar a construir o PACKET depois dele

    // QTYPE (A = host address = 0x0001)
    dnsPacket[12 + tamQuery] = 0x00; dnsPacket[13 + tamQuery] = 0x01;

    // QCLASS (IN = internet address = 0x0001)
    dnsPacket[14 + tamQuery] = 0x00; dnsPacket[15 + tamQuery] = 0x01;

    // TAMANHO TOTAL DO PACOTE (header + QNAME + 4)
    int packetSize = 12 + tamQuery + 4; // byte nulo após qname

    // Enviar a QUERY
    // sendto(sock, buffer, quantidade de bytes do request, flags, * para struct server, tamanho da struct);
    if(sendto(sock, dnsPacket, packetSize, 0, (struct sockaddr*)&saddr, sizeof(saddr)) < 0) {
        cout << "Erro ao enviar pacote DNS." << endl;
        close(sock);
        return 1;
    }

    // Receber o REPLY
    // recvfrom(sock, buffer, tamanho do buffer, flags, * para struct server, * para tamanho da struct);
    socklen_t len = sizeof(saddr);
    int receivedBytes = recvfrom(sock, dnsPacket, sizeof(dnsPacket), 0, (struct sockaddr*)&saddr, &len);
    if(receivedBytes < 0) {
        cout << "Erro ao receber resposta." << endl;
        close(sock);
        return 1;
    }

    // depuração <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    int id = (dnsPacket[0] << 8) | dnsPacket[1];
    int flags = (dnsPacket[2] << 8) | dnsPacket[3];
    int qdcount = (dnsPacket[4] << 8) | dnsPacket[5];
    int ancount = (dnsPacket[6] << 8) | dnsPacket[7];
    int nscount = (dnsPacket[8] << 8) | dnsPacket[9];
    int arcount = (dnsPacket[10] << 8) | dnsPacket[11];

    cout << "ID: " << id << " | Flags: " << flags
         << " | QDCOUNT: " << qdcount
         << " | ANCOUNT: " << ancount
         << " | NSCOUNT: " << nscount
         << " | ARCOUNT: " << arcount << endl;

    int rcode = flags & 0x000F;
    if(rcode != 0) {
        cout << "Erro no servidor DNS. Codigo RCODE: " << rcode << endl;
    }    

    // Interpretar o REPLY
    int ipAddr = parseDNSReply(dnsPacket, sizeof(dnsPacket));
    if(ipAddr != -1) {
        struct in_addr addr;
        addr.s_addr = htonl(ipAddr); // converte para o formato de rede
        cout << "IP de " << hostname << ": " << inet_ntoa(addr) << endl;
    } else {
        cout << "Nao foi possivel encontrar o IP." << endl;
    }

    close(sock);

    return 0;
}

void formatDNSName(unsigned char *dns, const char *host) {
    int len = 0;
    for(int i = 0; i <= strlen(host); i++) {
        if(host[i] == '.' || host[i] == '\0') {
            *dns++ = len; // escreve no endereço apontado por dns a quantidade de caracteres que virão e avança para proxima posição
            for(int j = i - len; j < i; j++) {
                *dns++ = host[j]; // escreve os caracteres
            }
            len = 0;; // reseta contador de comprimento
        } else {
            len++; // conta os caracteres até o proximo ponto
        }
    }
    *dns = '\0'; // finaliza o formato DNS
}

int parseDNSReply(unsigned char *reply, int replyLen) {
    int ipAddr = -1;

    // verifica quantidade de registros validos
    if(replyLen < 12) {
        return ipAddr; // reply invalido
    }

    // checar numero de respostas no ANCOUNT (7o e 8o byte)
    int numAnswer = (reply[6] << 8) | reply[7];
    if (numAnswer == 0) {
        return ipAddr; // nenhuma resposta encontrada
    }

    // pula header
    int offset = 12;

    // pular QNAME em question
    while(reply[offset] != 0) {
        if((reply[offset] & 0xC0) == 0xC0) {
            // ponteiro encontrado
            offset += 2;
            break;
        } else {
            offset += reply[offset] + 1;
        }
    }
    offset += 1; // avança o 0x00 que marca o fim do QNAME
    
    offset += 4; // pular QTYPE e QCLASS

    // processar cada resposta

    for(int i = 0; i < numAnswer; i++) {
        // pular nome de dominio na resposta (ponteiro)
        if((reply[offset] & 0xC0) == 0xC0) {
            offset += 2;
        } else {
            while(reply[offset] != 0) {
                offset += reply[offset] + 1;
            }
            offset++;
        }

        // Ler TYPE e CLASS (2 bytes cada)
        int type = (reply[offset] << 8) | reply[offset + 1];
        offset += 4;

        // Pular TTL (4 bytes)
        offset += 4;

        // Ler o comprimento do registro (2 bytes)
        int dataLen = (reply[offset] << 8) | reply[offset + 1];
        offset += 2;

        // se o TYPE for A (0x0001) e o tamanho for 4 bytes (IPv4)
        if(type == 0x0001 && dataLen == 4) {
            // extrair IP
            struct in_addr addr;
            ipAddr = (reply[offset] << 24) | (reply[offset+1] << 16) | (reply[offset+2] << 8) | (reply[offset+3]);
            return ipAddr; // retorna o primeiro IP encontrado
        }

        offset += dataLen; // pular o restante do registro se nao for endereço IPv4
    }

    return ipAddr; // retorna -1 se nao encotrar
}