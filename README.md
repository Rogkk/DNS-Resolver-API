# DNS RESOLVER CPP

Este projeto é um cliente DNS desenvolvido em C/C++ que envia uma requisição DNS (DNS REQUEST) manualmente construída no código para um servidor DNS pré-configurado, utilizando **sockets**. O cliente recebe a resposta do servidor, interpreta os dados e retorna ao usuário o endereço IP correspondente ao nome de domínio consultado. Este projeto visa demonstrar, de forma prática, a análise e implementação de protocolos de rede em baixo nível.

**Nota:** Atualmente, o programa funciona apenas em **Linux**. A versão para **Windows** ainda está em desenvolvimento.

## Motivação
Durante meus estudos em redes de computadores, explorei diversos serviços de rede, como **HTTP, FTP, DHCP e DNS**. No entanto, percebi que a maioria dos conteúdos disponíveis apresenta apenas uma visão superficial sobre a estrutura dos pacotes que trafegam pela rede. Motivado pela curiosidade, decidi aprofundar meus conhecimentos na análise de protocolos, escolhendo o **DNS** como objeto de estudo.

A estrutura padrão do protocolo DNS, bem como suas melhores práticas, está definida na **RFC 1035**, que serviu de base para a implementação deste projeto.

## Como Executar
### Compilação
Caso o arquivo executável não esteja presente, compile o código utilizando um compilador C++:

```sh
 g++ clientDNS.cpp -o cliente.exe
```

### Execução
Após compilar, execute o programa com o seguinte comando:

```sh
./cliente.exe <domain_name>
```

### Exemplo de Uso
```sh
./cliente.exe www.google.com
```

### Saída Esperada
```
ID: 43690 | Flags: 33152 | QDCOUNT: 1 | ANCOUNT: 1 | NSCOUNT: 0 | ARCOUNT: 0
IP de www.google.com: 142.250.219.196
```

### Tratamento de Erros
Caso ocorra algum erro durante o envio ou recebimento da requisição, a seguinte mensagem será exibida:

```
Erro no servidor DNS. Código: {1-5}
Não foi possível encontrar o IP
```

Os códigos de erro podem indicar falhas na resolução do domínio, indisponibilidade do servidor DNS ou erros na estrutura da requisição.
## Estrutura do Pacote DNS
Todos os pacotes DNS seguem a seguinte estrutura:
```
+--------------------+
|       Header       |
+--------------------+
|      Question      | Pergunta para o servidor de nomes
+--------------------+
|      Answer        | Respostas para a pergunta
+--------------------+
|      Authority     | Não usado neste projeto
+--------------------+
|      Additional    | Não usado neste projeto
+--------------------+
```
O **Header** descreve o tipo de pacote e quais campos estão contidos dentro dele. Uma única pergunta pode ter várias respostas, e o cliente processa a seção **Answer** para obter um resultado válido.

### DNS Header
O cabeçalho DNS tem a seguinte estrutura:
```
                                1  1  1  1  1  1  
  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                       ID                      |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|QR|  Opcode   |AA|TC|RD|RA|   Z    |   RCODE   |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    QDCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    ANCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    NSCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                    ARCOUNT                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
```
- **ID**: Identificador único da requisição.
- **QR**: Indica se o pacote é uma **query (0)** ou uma **resposta (1)**.
- **Opcode**: Tipo de requisição (geralmente 0 para consulta padrão).
- **AA**: Indica se a resposta vem de um servidor autoritativo.
- **TC**: Indica se a mensagem foi truncada.
- **RD**: Indica se a recursão foi solicitada.
- **RA**: Indica se a recursão está disponível.
- **RCODE**: Código de resposta:
  - `0`: Sem erro
  - `1`: Erro de formato
  - `2`: Problema no servidor
  - `3`: Nome de domínio não existente
  - `4`: Não implementado
  - `5`: Recusa
- **QDCOUNT**: Número de questões.
- **ANCOUNT**: Número de respostas.
- **NSCOUNT**: Número de registros de autoridade.
- **ARCOUNT**: Número de registros adicionais.

### DNS Question
```
                                1  1  1  1  1  1  
  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                                               |
/                     QNAME                     \
/                                               \
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                     QTYPE                     |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                     QCLASS                    |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
```
- **QNAME**: Nome do domínio sendo consultado.
- **QTYPE**: Tipo de consulta (ex.: `1` para A, `15` para MX).
- **QCLASS**: Classe de consulta (geralmente `1` para IN - Internet).

### DNS Answer
```
                                1  1  1  1  1  1  
  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                                               |
/                     NAME                      \
/                                               \
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                     TYPE                      |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                     CLASS                     |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                     TTL                       |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                     RDLENGTH                  |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                     RDATA                     |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
```
- **NAME**: Nome do domínio da resposta.
- **TYPE**: Tipo de registro.
- **CLASS**: Classe do registro.
- **TTL**: Tempo de vida do registro.
- **RDLENGTH**: Tamanho dos dados.
- **RDATA**: Dados da resposta.
## Referências
- [RFC 1035 - Domain Names - Implementation and Specification](https://datatracker.ietf.org/doc/html/rfc1035)
