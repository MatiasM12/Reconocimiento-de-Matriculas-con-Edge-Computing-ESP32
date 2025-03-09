#  Reconocimiento de Matrículas con ESP32 (Edge Computing) 🚘

Este proyecto implementa un sistema de Reconocimiento Automático de Matrículas (ALPR) utilizando un microcontrolador **ESP32**. A través de un proceso de procesamiento de imágenes y el uso de redes neuronales, se logra detectar y reconocer las matrículas de vehículos en imágenes. El sistema está diseñado para ser eficiente y funcionar en dispositivos con limitaciones de memoria y procesamiento.

![T(1)](https://github.com/user-attachments/assets/299ae187-bd8a-4b4b-b03a-c958507cdfed)

## 🔧 Tecnologías Utilizadas

- **ESP32**: Plataforma de hardware embebido para capturar imágenes, procesarlas y ejecutar el modelo de reconocimiento de matrículas.
- **OpenCV**: Biblioteca para el procesamiento de imágenes, como localización de la matrícula y segmentación de caracteres .
- **TensorFlow Lite**: Framework de aprendizaje automático para entrenar y ejecutar el modelo de redes neuronales en el ESP32.
- **Google Colab**: Herramienta utilizada para entrenar y ajustar el modelo de redes neuronales antes de implementarlo en el ESP32.

## Tipos de Matrículas Reconocidas

Se reconoce principalmente dos tipos de matrículas argentinas:

- **Matrícula de 1994**: Formato LLL NNN (letras y números).
- **Matrícula de 2015**: Formato LL NNN LL (letras, números y letras).
  
 ![T](https://github.com/user-attachments/assets/af1c0f6c-9258-4482-a1c6-219ba7ed712e)



## 🛠️ Implementación del Pipeline de ALPR

El pipeline de reconocimiento de matrículas se compone de tres etapas principales:

1. **Procesamiento de Imágenes**: Preprocesamiento para localizar la matrícula y prepararla para su segmentación.
2. **Segmentación de Caracteres**: Segmentación de los caracteres de la matrícula para su posterior análisis.
3. **Reconocimiento de Caracteres con Redes Neuronales**: El modelo de redes neuronales infiere los caracteres segmentados y reconstruye la matrícula completa.

## 📸 Procesamiento de Imágenes

### Localización de la Matrícula

El primer paso es identificar y localizar la matrícula dentro de la imagen. Este proceso incluye:

- Aplicar escala de grises.
- Redimensionamiento: Ajustar el tamaño de la imagen a 450x337.
- Aplicar filtro Gaussiano.
- Aplicar filtro Bilateral.
- Detección de bordes: Usar el detector de bordes de Canny.
- Aplicar dilatación.
- Localización y recorte: Identificar los contornos y recortar la región correspondiente a la matrícula.

### Segmentación de Caracteres

Una vez localizada la matrícula, se realiza la segmentación de los caracteres mediante:

- Aplicación de erosión para eliminar imperfecciones.
- Operación de cierre para unir caracteres que podrían estar desconectados.
- Localización y recorte de caracteres individuales para su posterior reconocimiento.

## Reconocimiento de Caracteres

Se utilizan redes neuronales convolucionales (CNN) entrenadas para reconocer los caracteres en la matrícula. El modelo se entrena utilizando imágenes de caracteres de matrículas reales y se aplica aumento de datos (rotación, traslación, escalado, etc.) para mejorar su rendimiento.

Los modelos creados son:

- **Modelo de reconocimiento de dígitos**: Entrenado con números extraídos de matrículas.
- **Modelo de reconocimiento de letras**: Entrenado con letras de matrículas.

Una vez entrenados, los modelos se exportan a TensorFlow Lite y se implementan en el ESP32 para su inferencia en tiempo real.

## 🔍 Conclusión

Este proyecto demuestra cómo utilizar el **ESP32** junto con herramientas como **OpenCV** y **TensorFlow Lite** para implementar un sistema de reconocimiento de matrículas eficiente y de bajo consumo. A pesar de las limitaciones del hardware, el uso de técnicas de procesamiento de imágenes optimizadas y redes neuronales ligeras permite realizar tareas complejas de visión artificial en dispositivos embebidos.
