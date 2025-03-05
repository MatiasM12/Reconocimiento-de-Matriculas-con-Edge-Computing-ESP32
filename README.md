#  Reconocimiento de Matrículas con ESP32 (Edge Computing) 🚘

Este proyecto implementa un sistema de Reconocimiento Automático de Matrículas (ALPR) utilizando un microcontrolador **ESP32**. A través de un proceso de procesamiento de imágenes y el uso de redes neuronales, se logra detectar y reconocer las matrículas de vehículos en imágenes en tiempo real. El sistema está diseñado para ser eficiente y funcionar en dispositivos con limitaciones de memoria y procesamiento.

## 🔧 Tecnologías Utilizadas

- **ESP32**: Plataforma de hardware embebido para capturar imágenes, procesarlas y ejecutar el modelo de reconocimiento de matrículas.
- **OpenCV**: Biblioteca para el procesamiento de imágenes, como localización de la matrícula y segmentación de caracteres .
- **TensorFlow Lite**: Framework de aprendizaje automático para entrenar y ejecutar el modelo de redes neuronales en el ESP32.
- **Google Colab**: Herramienta utilizada para entrenar y ajustar el modelo de redes neuronales antes de implementarlo en el ESP32.

## Tipos de Matrículas Reconocidas

Se reconoce principalmente dos tipos de matrículas argentinas:

- **Matrícula de 1994**: Formato LLL NNN (letras y números).
- **Matrícula de 2015**: Formato LL NNN LL (letras, números y letras).
  
 ![Tesis (30)](https://github.com/user-attachments/assets/af1c0f6c-9258-4482-a1c6-219ba7ed712e)



## 🛠️ Implementación del Pipeline de ALPR

El pipeline de reconocimiento de matrículas se compone de tres etapas principales:

1. **Procesamiento de Imágenes**: Preprocesamiento para localizar la matrícula y prepararla para su segmentación.
2. **Segmentación de Caracteres**: Segmentación de los caracteres de la matrícula para su posterior análisis.
3. **Reconocimiento de Caracteres con Redes Neuronales**: El modelo de redes neuronales infiere los caracteres segmentados y reconstruye la matrícula completa.

## 📸 Procesamiento de Imágenes

### Localización de la Matrícula

El primer paso es identificar y localizar la matrícula dentro de la imagen. Este proceso incluye:

- Escala de grises: Convertir la imagen a escala de grises para facilitar la detección de bordes.
- Redimensionamiento: Ajustar el tamaño de la imagen para mejorar el procesamiento.
- Filtros de suavizado: Aplicar filtros Gaussianos y Bilaterales para reducir el ruido.
- Detección de bordes: Usar el detector de bordes de Canny.
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

## 📋 Instrucciones de Uso

1. **Instalar Dependencias**: Asegúrate de tener configurado el entorno de desarrollo para el ESP32 con ESP-IDF y las bibliotecas necesarias como OpenCV y TensorFlow Lite.
3. **Subir al ESP32**: Sube el código al ESP32 y segúrate de tener un sistema de almacenamiento (como una tarjeta SD) con la imagen que desea procesar.
4. **Ejecutar**: Al ejecutar el código, el **ESP32** procesará las imágenes y reconocerá las matrículas de los vehículos.

## 🤝 Contribuciones

Las contribuciones son bienvenidas. Si tienes mejoras, correcciones o sugerencias, no dudes en abrir un **issue** o enviar un **pull request**.

## 📝 Licencia

Este proyecto está bajo la Licencia **MIT** - ver el archivo [LICENSE](LICENSE) para más detalles.
