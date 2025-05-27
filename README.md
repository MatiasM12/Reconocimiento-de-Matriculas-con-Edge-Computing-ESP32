# Reconocimiento de Matrículas con ESP32 (Edge Computing) 🚘 

Este proyecto implementa un sistema de **Reconocimiento Automático de Matrículas (ALPR)** utilizando un microcontrolador **ESP32**. A través de procesamiento de imágenes con **OpenCV** y redes neuronales optimizadas con **TensorFlow Lite**, el sistema es capaz de detectar y reconocer matrículas vehiculares en imágenes, directamente desde el dispositivo.

Diseñado para funcionar en entornos con recursos limitados, este proyecto demuestra cómo la **IA embebida** puede realizar tareas de visión por computadora en tiempo real y sin necesidad de servidores externos.

![Tesis (27) (2)](https://github.com/user-attachments/assets/e1a8793d-c9f1-4353-98e1-bd8317dfbc99)

---

## 🔧 Tecnologías utilizadas

- **ESP32**: Microcontrolador de bajo consumo con Wi-Fi/Bluetooth, encargado de capturar imágenes, procesarlas y ejecutar inferencias.
- **OpenCV**: Biblioteca de visión por computadora usada para localizar la matrícula y segmentar los caracteres.
- **TensorFlow Lite**: Framework de ML optimizado para microcontroladores, utilizado para ejecutar modelos de clasificación de caracteres.
- **Google Colab**: Plataforma utilizada para entrenar, validar y exportar los modelos.

---

## 🇦🇷 Tipos de Matrículas Reconocidas

El sistema fue entrenado para reconocer dos formatos comunes de matrículas argentinas:

- **1994** – Formato `LLL NNN` (3 letras + 3 números)  
- **2015** – Formato `LL NNN LL` (2 letras + 3 números + 2 letras)

![T](https://github.com/user-attachments/assets/af1c0f6c-9258-4482-a1c6-219ba7ed712e)

---

## 🛠️ Arquitectura del Sistema ALPR

El pipeline de reconocimiento consta de tres etapas principales:

1. **📸 Procesamiento de Imágenes**  
   Localización de la matrícula dentro de la imagen.

2. **🔠 Segmentación de Caracteres**  
   Aislamiento de cada carácter presente en la matrícula.

3. **🤖 Reconocimiento con Redes Neuronales**  
   Clasificación de los caracteres mediante modelos entrenados.

---

## 📸 Procesamiento de Imágenes

### 🔍 Localización de la Matrícula

Se aplican las siguientes transformaciones sobre la imagen capturada:

- Conversión a escala de grises  
- Redimensionamiento a 450×337 px  
- Filtro Gaussiano + filtro bilateral  
- Detección de bordes (Canny)  
- Dilatación de bordes  
- Identificación de contornos y recorte de la región con la matrícula

### ✂️ Segmentación de Caracteres

Una vez detectada la placa, se aplica:

- Erosión para eliminar ruido  
- Operación de cierre para unir caracteres  
- Detección de contornos individuales  
- Recorte de cada carácter para su reconocimiento

---

## 🧠 Reconocimiento de Caracteres con IA

Se entrenaron dos modelos de **redes neuronales convolucionales (CNN)**, utilizando imágenes reales de matrículas en escala de grises y tamaño 20×32. Se aplicó **aumento de datos** (rotación, traslación, escala) para mejorar la generalización del modelo.

- **Modelo de dígitos** – Reconoce números del 0 al 9  
  ![image](https://github.com/user-attachments/assets/3c5a4da9-0990-4b2b-b79a-52d174e24649)

- **Modelo de letras** – Reconoce caracteres del alfabeto  
  ![image](https://github.com/user-attachments/assets/815903af-44c1-43e5-b881-076637f20b80)

Ambos modelos se exportaron a **TensorFlow Lite** y fueron integrados al firmware del ESP32 para realizar inferencias en tiempo real.

---

## 📈 Resultados

- ✅ **Matrículas de 1994**: 97% de precisión (97 aciertos y 3 fallas por no localizar la placa (97/100))
- ✅ **Matrículas de 2015**: 90% de precisión (45 aciertos y 5 fallas por errores en la predicción de caracteres (45/50))

---

## 🔍 Conclusión

Este proyecto demuestra cómo es posible implementar un sistema de **visión artificial** funcional y preciso en un microcontrolador con recursos limitados.

Gracias al uso de **OpenCV**, **TensorFlow Lite** y técnicas de preprocesamiento eficientes, se logra realizar **reconocimiento de matrículas en tiempo real**, sin conexión externa, con bajo consumo y buena precisión.

---

## 📁 Repositorio y Recursos

- 📹 **Demo en video**: [enlace_al_video](https://youtu.be/-7m6hsqOaNE)
- 🧠 **Modelos entrenados y documentación**: [Subire el enlace proximamente]

---

