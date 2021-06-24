//Programa: Lighting and Relay Module
//Autor: Elton de Sousa e Silva
//Email: admin@autodomo.io
//Versão 1.1
 
#include <ESP8266WiFi.h>        // Import the Library ESP8266WiFi
#include <PubSubClient.h>       // Import the Library PubSubClient
 

#define TOPIC_SUBSCRIBE "/autodomo/UserXXXXXXXX/01AAXXXXXXXX/light/value"    //Put here the MQTT topic of receiving Autodomo Server information
#define TOPIC_PUBLISH   "/autodomo/UserXXXXXXXX/01AAXXXXXXXX/light/state"    //Put here the MQTT Topic of sending information to the Autodomo Server

#define ID_MQTT  "01AA49F41D92"     //MQTT ID For identification (Required))
#define Relay   5                   //Relay output change according to your board
 
// WIFI
const char* SSID = "YOUR_WIFI_SSID"; // SSID  name of the WI-FI network you want to connect
const char* PASSWORD = "************"; // Password of the WI-FI network you want to connect
  
// MQTT
const char* BROKER_MQTT = "mqtt.autodomo.io"; //Autodomo MQTT broker URL not change
int BROKER_PORT = 1883; // MQTT Broker port. not change.
const char* USER_MQTT = "UserXXXXXXXX";    // MQTT User (Search Autodomo.io Site)
const char* PASSWORD_MQTT = "***********";    // MQTT Password (Search Autodomo.io Website)
 
 
//Variáveis e objetos globais
WiFiClient espClient; // Cria o objeto espClient
PubSubClient MQTT(espClient); // Instancia o Cliente MQTT passando o objeto espClient
char EstadoSaida = '0';  //variável que armazena o estado atual da saída
  
//Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);
void InitOutput(void);
 
/* 
 *  Implementações das funções
 */
void setup() 
{
    //inicializações:
    InitOutput();
    initSerial();
    initWiFi();
    initMQTT();
}
  
//Função: Inicializa comunicação serial com baudrate 115200 (para fins de debug)
//Parâmetros: nenhum
//Retorno: nenhum
void initSerial() 
{
    Serial.begin(115200);
}
 
//Função: inicializa e conecta-se na rede WI-FI desejada
//Parâmetros: nenhum
//Retorno: nenhum
void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
     
    reconectWiFi();
}
  
//Função: inicializa parâmetros de conexão MQTT(endereço do servidor, porta e seta função de callback)
//Parâmetros: nenhum
//Retorno: nenhum
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //Atribui função de callback (função chamada quando chega dados no Topic Subscrito)
}
  
//Função: função de callback esta função é chamada toda vez que uma informação de um dos tópicos subescritos chega)
//Parâmetros: nenhum
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
   
    //toma ação dependendo da string recebida:
    //verifica se deve colocar nivel alto de tensão na saída D0:
    //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
    //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
    if (msg.equals("0"))
    {
        digitalWrite(Relay, LOW);
        EstadoSaida = '0';
        Serial.println("Off");
    }
 
    //verifica se deve colocar nivel alto de tensão na saída D0:
    if (msg.equals("1"))
    {
        digitalWrite(Relay, HIGH);
        EstadoSaida = '1';
        Serial.println("On");
    }

    EnviaEstadoOutputMQTT();
     
}
  
//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT, USER_MQTT, PASSWORD_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPIC_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}
  
//Função: reconecta-se ao WiFi
//Parâmetros: nenhum
//Retorno: nenhum
void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
         
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
     
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
   
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
 
//Função: verifica o estado das conexões WiFI e ao broker MQTT. 
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parâmetros: nenhum
//Retorno: nenhum
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
     
     reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}
 
//Função: envia ao Broker o estado atual do output 
//Parâmetros: nenhum
//Retorno: nenhum
void EnviaEstadoOutputMQTT(void)
{
    if (EstadoSaida == '0')
      MQTT.publish(TOPIC_PUBLISH, "0");
 
    if (EstadoSaida == '1')
      MQTT.publish(TOPIC_PUBLISH, "1");
 
    Serial.println("- Estado da saida D0 enviado ao broker!");
    delay(1000);
}
 
//Função: inicializa o output em nível lógico baixo
//Parâmetros: nenhum
//Retorno: nenhum
void InitOutput(void)
{
    //IMPORTANTE: o Led já contido na placa é acionado com lógica invertida (ou seja,
    //enviar HIGH para o output faz o Led apagar / enviar LOW faz o Led acender)
    pinMode(Relay, OUTPUT);
    digitalWrite(Relay, HIGH);          
}
 
 
//programa principal
void loop() 
{   
    //garante funcionamento das conexões WiFi e ao broker MQTT
    VerificaConexoesWiFIEMQTT();
 
    //envia o status de todos os outputs para o Broker no protocolo esperado
   // EnviaEstadoOutputMQTT();
 
    //keep-alive da comunicação com broker MQTT
    MQTT.loop();
}
