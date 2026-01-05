#include "esp_camera.h"
#include "Arduino.h"
#include "SDM.h" // Votre librairie personnalisée

// --- Configuration des Pins Caméra ---
#define SIOD_GPIO_NUM     21
#define SIOC_GPIO_NUM     22
#define D0_GPIO     39
#define D1_GPIO     35
#define D2_GPIO     34
#define D3_GPIO     32
#define D4_GPIO     27
#define D5_GPIO     23
#define D6_GPIO     19
#define D7_GPIO     18
#define VSYNC_GPIO  4 
#define HREF_GPIO   25 
#define PCLK_GPIO   5 
#define XCLK_GPIO   -1 

// --- Objets de synchronisation ---
TaskHandle_t TaskSDHandle;
SemaphoreHandle_t xCaptureSemaphore;
camera_fb_t * fb_global = NULL;
volatile bool isWriting = false;

// --- Fonction de la tâche SD (Cœur 1) ---
void TaskSD(void * pvParameters) {
    Serial.print("Tâche SD démarrée sur le coeur : ");
    Serial.println(xPortGetCoreID());

    for(;;) {
        // Attend le signal du Coeur 0 (bloque sans consommer de CPU)
        if(xSemaphoreTake(xCaptureSemaphore, portMAX_DELAY) == pdTRUE) {
            if(fb_global) {
                isWriting = true;
                
                char filename[32];
                sprintf(filename, "/img_%u.jpg", (unsigned int)millis());
                
                Serial.printf("Ecriture de %d octets sur SD...\n", fb_global->len);
                
                // Utilisation de votre librairie
                writeFile(filename, fb_global->buf, fb_global->len);
                
                // Libération du buffer caméra après écriture
                esp_camera_fb_return(fb_global);
                fb_global = NULL;
                isWriting = false;
                
                Serial.println("Ecriture terminée, prêt pour la suivante.");
            }
        }
    }
}

void setup() {
    Serial.begin(115200);

    // 1. Initialisation SD
    initSD();

    // 2. Configuration Caméra
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = D0_GPIO;
    config.pin_d1 = D1_GPIO;
    config.pin_d2 = D2_GPIO;
    config.pin_d3 = D3_GPIO;
    config.pin_d4 = D4_GPIO;
    config.pin_d5 = D5_GPIO;
    config.pin_d6 = D6_GPIO;
    config.pin_d7 = D7_GPIO;
    config.pin_xclk = XCLK_GPIO; 
    config.pin_pclk = PCLK_GPIO;
    config.pin_vsync = VSYNC_GPIO;
    config.pin_href = HREF_GPIO;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // Ajustement crucial pour ESP32 sans PSRAM
    if(psramFound()){
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_UXGA; // 1600x1200
        config.jpeg_quality = 18;            // Légère compression pour tenir en RAM interne
        config.fb_count = 1;                 // Un seul buffer pour éviter l'épuisement RAM
        config.fb_location = CAMERA_FB_IN_DRAM;
    }

    // Initialisation du driver
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Erreur caméra 0x%x", err);
        return;
    }

    // 3. Création des outils FreeRTOS
    xCaptureSemaphore = xSemaphoreCreateBinary();

    // Création de la tâche SD sur le COEUR 1
    xTaskCreatePinnedToCore(
        TaskSD,          // Fonction
        "TaskSD",        // Nom
        10000,           // Pile (Stack size)
        NULL,            // Paramètres
        1,               // Priorité
        &TaskSDHandle,   // Handle
        1                // COEUR 1
    );
}

void loop() {
    // Le code ici s'exécute par défaut sur le COEUR 0
    
    // On ne prend une photo que si la précédente est finie (Indispensable sans PSRAM)
    if (!isWriting && fb_global == NULL) {
        
        Serial.println("Capture image...");
        fb_global = esp_camera_fb_get();
        
        if (!fb_global) {
            Serial.println("Échec de capture");
        } else {
            // On envoie le signal au Coeur 1 pour l'écriture
            xSemaphoreGive(xCaptureSemaphore);
        }
    }

    // Attendre 5 secondes entre chaque photo
    delay(5000); 
}