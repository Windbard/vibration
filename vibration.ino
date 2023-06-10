#include <WiFi.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "PubSubClient.h"

Adafruit_MPU6050 mpu;

/* 连接WIFI SSID和密码 */
#define WIFI_SSID         "3671"
#define WIFI_PASSWD       "05210835"

/* 设备的三元组信息*/
#define PRODUCT_KEY       "ivzdiVlZIpq"
#define DEVICE_NAME       "MPU6050"
#define DEVICE_SECRET     "fd6442af5b3cacdf6b54cd772219b387"
#define REGION_ID         "cn-shanghai"

/* 线上环境域名和端口号，不需要改 */
#define MQTT_SERVER       PRODUCT_KEY".iot-as-mqtt."REGION_ID".aliyuncs.com"
#define MQTT_PORT         1883
#define MQTT_USRNAME      DEVICE_NAME"&"PRODUCT_KEY

#define CLIENT_ID         "ivzdiVlZIpq.MPU6050|securemode=2,signmethod=hmacsha256,timestamp=1686231865208|"
#define MQTT_PASSWD       "5fe070b183d5b2a24216fce860429b69a3aa12d4740618410c85fcd60cfe8009"


#define ALINK_BODY_FORMAT         "{\"id\":\"MPU6050\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"
#define ALINK_TOPIC_PROP_POST     "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"

unsigned long lastMs = 0;
WiFiClient espClient;
PubSubClient  client(espClient);

float xrad;  
float yrad;  
float zrad;  

//连接wifi
void wifiInit()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("WiFi not Connect");
    }
    
}

//mqtt连接
void mqttCheckConnect()
{
    while (!client.connected())
    {
        Serial.println("Connecting to MQTT Server ...");
        if(client.connect(CLIENT_ID, MQTT_USRNAME, MQTT_PASSWD))
        {
          Serial.println("MQTT Connected!");
        }
        else{
           Serial.print("MQTT Connect err:");
            Serial.println(client.state());
            delay(5000);

          }
        
    }
}

void mqttIntervalPost()
{
      /* 通过读数获取新的传感器事件 */
    sensors_event_t a, g,temp;
    mpu.getEvent(&a, &g,&temp);

    char param[32];
    char jsonBuf[128];
    
    xrad =a.acceleration.x+2.15;
    sprintf(param, "{\"xrad\":%2f}",xrad);
    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
    Serial.println(jsonBuf);
    boolean x = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);

    yrad =a.acceleration.y+0.35;
    sprintf(param, "{\"yrad\":%2f}",yrad);
    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
    Serial.println(jsonBuf);
    boolean y = client.publish(ALINK_TOPIC_PROP_POST, jsonBuf);

    if(x&&y){
      Serial.println("publish vibration success"); 
    }else{
      Serial.println("publish vibration fail"); 
    }
}

void setup() 
{
    Serial.begin(115200);
    Wire.begin(4,5);
    while (!Serial)
    delay(10); //初始化MPU-6050传感器。
  Serial.println("Adafruit MPU6050 test!");
   if (!mpu.begin()) {
    Serial.println("not find MPU6050");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
 
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("加速计范围设置为: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("陀螺仪量程设置为: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("过滤器带宽设置为: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }
 
  Serial.println("");
  delay(100);
    wifiInit();
    client.setServer(MQTT_SERVER, MQTT_PORT);   /* 连接MQTT服务器 */
}

void loop()
{
    
    
    if (millis() - lastMs >= 5000)
    {
        lastMs = millis();
        mqttCheckConnect(); 
        /* 上报 */
        mqttIntervalPost();
    }
    client.loop();
    delay(2000);
}
