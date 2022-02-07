#include <Arduino.h>
#include <TinyGPS.h> 
#include <Dictionary.h>

#include <SPI.h>
#include <Ethernet.h>
#include <string>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);
const String ID = "ffae7b1d-b180-4f01-9c7d-b37f9bc4e18b";

EthernetServer server(80);
const int temperature_sensor = 12;
const int gps_sensor = 14;

TinyGPS gps;
int temperature = 0;
int latitude  = 0;
int longitude  = 0;
int nextdata = 0;
const devMode = true;
const int enabledevicetime = 0;

/**
 * @brief 
 * Cela permet d'initialiser les ports
 */
void setup() {
  //-------------------------------Serial COM
  Serial.begin(9600);
  if(devMode){
    while (!Serial){}
  }
  //-------------------------------GPID
  pinMode(temperature_sensor, INPUT);
  pinMode(gps_sensor, INPUT);
  //-------------------------------Ehernet
  Ethernet.begin(mac, ip);
  //-------------------------------Server Web
  server.begin();
  //-------------------------------Etat de l'Appareil
  enabledevicetime = millis();
}

void consolelog(String log){
  if(devMode){
  Serial.println("[" + __TIME__ + "] " + log)
  }
}

void loop() {
  refreshInformation();
  SiteWeb();
  
}

String getActivityDevicesTime() {
  int milliseconds = (enabledevicetime - millis()) / 1000;
  int hours =  (milliseconds / 3600);
  int minutes =  (milliseconds / 60) % 60;
  int seconds => (milliseconds % 60);
  return String(hours) + ":" + String(minutes) + ":" + String(seconds) ;
}
/**
 * @brief 
 *  Cela permet de mettre à jour les info des senors
 * @param value 
 */
void refreshInformation(boolean value = false) {

  if (nextdata  <= millis())
  {
   nextdata = millis() + 5 * 60 * 1000;
   gps.f_get_position(latitude,longitude);
   /*
   mettre les code 
   */
   consolelog("syncronisation: normal");
  }else if(value){
   gps.f_get_position(latitude,longitude);
   /*
   mettre les code 
   */
   consolelog("syncronisation: force");
  }
  

}

/**
 * @brief 
 * Cela gere la generation du site et de la command
 * 
 */
void SiteWeb() {
  EthernetClient client = server.available();
  String readString = "";
  Dictionary flags = new Dictionary();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c !== "\n") {
          String word = "";
          String value = "";
          bool next_flags = false;
          bool has_values = false;
          for (size_t i = 0; i < c.length; i++)
          {
            String letter = c[i];
            if (letter.equals("/?")) continue;
            if (letter.equals("="))
            {
            has_values = true;
            continue;
            }
            if (!has_values)
            {
            word = word + letter;
            }else{
              if (letter.equals("&")){
                next_flags = true;
              }
              if(!next_flags){
                flags.insert(word,value);
              }else{
                value = value + letter;
              }

              if (i == (c.length - 1) && !has_values )
                {
                if(flags.search(word) == ""){
                  if(value !== ""){
                    flags.insert(word,value));
                  }else{
                    flags.insert(word,""));

                  }
                }
              }
            }
          }

      }
        if (c == '\n') {
        generateSiteWeb(client, flags);
        readString = "";
        }
      }
    }
  }
  delay(1);
  client.stop();
}


/**
 * @brief 
 * Cela genere tous le code html
 * @param client 
 */
void generateSiteWeb(EthernetClient client, Dictionary flags){

  if(flags.search("refresh") === "true"){
   refreshInformation(true);
  }
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html"));
  client.println();  // the connection will be closed after completion of the response
  client.println("Refresh: 20");  // recharge de la page  tus les 20 sec
 client.println("<!DOCTYPE HTML>");

 client.println("<html>");

 client.println("<head>");
 client.println('<meta name="viewport" content="width=device-width, initial-scale=1" charset="utf-8">');
 client.println('<link href="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-1BmE4kWBq78iYhFldvKuhfTAU6auU8tT94WrHftjDbrCEXSU1oBoqyl2QvZ6jIW3" crossorigin="anonymous">');
 client.println("<title>WebServer Ruche</title>");
 client.println("</head>");

 client.println("<body>");
 client.println('<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.bundle.min.js" integrity="sha384-ka7Sk0Gln4gmtz2MlQnikT1wXgYsOg+OMhuP+IlRH9sENBO0LRn5q+8nbTov4+1p" crossorigin="anonymous"></script>');
 client.println('
 <nav class="nav nav-pills nav-justified">
    <a class="nav-link" href="?ff" id="fdf">Aceuil</a>
    <a class="nav-link" data-bs-toggle="offcanvas" data-bs-target="#about" aria-controls="offcanvasBottom">A Propos</a>
    <a class="nav-link active" aria-current="location" href="?position=recharge">Recharger</a>
</nav>
');


    client.println('
    <div class="offcanvas offcanvas-bottom" tabindex="-1" id="about" aria-labelledby="aboutLabel">
    <div class="offcanvas-header">
      <h5 class="offcanvas-title" id="aboutLabel">Information de l\'appareil</h5>
      <button type="button" class="btn-close text-reset" data-bs-dismiss="offcanvas" aria-label="Close"></button>
    </div>
    <div class="offcanvas-body small">
      Models: Ruche-Sensors W85
      <p></p>
      Battery: 100%
      <p></p>
      Temperature: '+ String(temperature) +'
      <p></p>
      Ip: '+ ip +'
      <p></p>
      Durée d\'activité: '+ getActivityDevicesTime() +'
      <p></p>
      Mode Developpeur:'+ String(devMode) +'
    </div>
</div>
');
  if (temperature > 35)
  {
    client.println('
    <div class="alert alert-warning alert-dismissible fade show" role="alert">
        la temperature est de plus de 35°C
        <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
    </div>
    ');
  }else if (temperature < 15)
  {
    client.println('
    <div class="alert alert-primary alert-dismissible fade show" role="alert">
        la temperature est de inferieur à 15°C
        <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
    </div>
    ');
  }


 client.println('<h1 class="display-1" style="text-align: center;">Ruche Test</h1>');
 client.println('
 <div class="container">
    <div class="row">
      <div class="col" style="font-family: Arial, Helvetica, sans-serif ; font-size: 20px;">
        <strong>Temperature</strong>
      </div>
      <div class="col" style="font-family: Arial, Helvetica, sans-serif ; font-size: 20px;">
        <strong>Latidure</strong>
      </div>
      <div class="col" style="font-family: Arial, Helvetica, sans-serif ; font-size: 20px;">
        <strong>Longitude</strong>
      </div>
    </div>
</div>
');
 client.println('
 <div class="container">
    <div class="row">
      <div class="col"  style="font-family: Arial, Helvetica, sans-serif ; font-size: 40px;">
        6°C
      </div>
      <div class="col"  style="font-family: Arial, Helvetica, sans-serif ; font-size: 40px;">
        45.934
      </div>
      <div class="col"  style="font-family: Arial, Helvetica, sans-serif ; font-size: 40px;">
        45.45
      </div>
    </div>
  </div>
  ');

 client.println("</body>");
 client.println("</html>")
}
  
