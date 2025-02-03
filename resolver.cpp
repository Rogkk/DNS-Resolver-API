#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>

using namespace std;

int main() {
    const char* hostname = "google.com";
    struct hostent* host_info = gethostbyname(hostname);

    if(host_info == nullptr) {
        cerr << "Erro ao resolver nome DNS.\n";
        return 1;
    }

    struct in_addr* address = (struct in_addr*)host_info->h_addr;
    cout << "IP de " << hostname << ": " << inet_ntoa(*address) << endl;


    return 0;
}